/*
 * nRF7002DK (nRF Connect SDK): join Wi-Fi with stored credentials, get an IP by
 * DHCP, then listen on TCP 5000 and send a line once a second to whoever connects.
 *
 * Follows nrf/samples/wifi/sta. SSID/password: credentials.conf (not in git).
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/ethernet_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/wifi_mgmt.h>
#include <net/wifi_ready.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define PORT 5000

/* Used when the nRF70 OTP holds no MAC (set in app.overlay). */
static const uint8_t dts_mac[6] = DT_PROP_OR(DT_CHOSEN(zephyr_wifi), local_mac_address, {0});

static K_SEM_DEFINE(ready, 0, 1);
static K_SEM_DEFINE(joined, 0, 1);
static K_SEM_DEFINE(got_ip, 0, 1);
static int join_status;
/* Two callbacks: one callback cannot mix events from different net_mgmt layers. */
static struct net_mgmt_event_callback wifi_cb, ip_cb;

static void wifi_ready_cb(bool up)
{
	if (up) {
		k_sem_give(&ready);
	}
}

static void on_wifi_event(struct net_mgmt_event_callback *c, uint64_t ev, struct net_if *iface)
{
	join_status = ((const struct wifi_status *)c->info)->status;
	k_sem_give(&joined);
}

static void on_ip_event(struct net_mgmt_event_callback *c, uint64_t ev, struct net_if *iface)
{
	k_sem_give(&got_ip);
}

int main(void)
{
	struct net_if *iface = net_if_get_first_wifi();
	wifi_ready_callback_t rcb = { .wifi_ready_cb = wifi_ready_cb };

	if (!iface) {
		LOG_ERR("no Wi-Fi interface");
		return -ENODEV;
	}
	register_wifi_ready_callback(rcb, iface);

	net_mgmt_init_event_callback(&wifi_cb, on_wifi_event, NET_EVENT_WIFI_CONNECT_RESULT);
	net_mgmt_add_event_callback(&wifi_cb);
	net_mgmt_init_event_callback(&ip_cb, on_ip_event, NET_EVENT_IPV4_DHCP_BOUND);
	net_mgmt_add_event_callback(&ip_cb);

	/* Zephyr will not bring an Ethernet-type interface up without a valid MAC. */
	struct net_linkaddr *la = net_if_get_link_addr(iface);

	if (la->len != 6 || !net_eth_is_addr_valid((struct net_eth_addr *)la->addr)) {
		struct ethernet_req_params mp;

		memcpy(mp.mac_address.addr, dts_mac, sizeof(dts_mac));
		net_mgmt(NET_REQUEST_ETHERNET_SET_MAC_ADDRESS, iface, &mp, sizeof(mp));
		int up = net_if_up(iface);

		if (up && up != -EALREADY) {
			LOG_ERR("cannot bring the Wi-Fi interface up (%d)", up);
			return up;
		}
	}

	if (k_sem_take(&ready, K_SECONDS(15))) {
		LOG_ERR("Wi-Fi did not become ready");
		return -ETIMEDOUT;
	}

	LOG_INF("joining the stored network...");
	int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT_STORED, iface, NULL, 0);

	if (ret) {
		LOG_ERR("connect request rejected (%d)", ret);
		return ret;
	}
	/* Wait up to 72 s, and show where the supplicant is while we do. */
	bool done = false;

	for (int i = 0; i < 24 && !done; i++) {
		done = k_sem_take(&joined, K_SECONDS(3)) == 0;
		if (!done) {
			struct wifi_iface_status st = { 0 };

			net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &st, sizeof(st));
			LOG_INF("waiting to join... state: %s", wifi_state_txt(st.state));
		}
	}
	if (!done) {
		LOG_ERR("no join result after 72 s - network not found, or security mismatch");
		return -ETIMEDOUT;
	}
	if (join_status) {
		LOG_ERR("join failed (status %d)", join_status);
		return -ECONNREFUSED;
	}
	if (k_sem_take(&got_ip, K_SECONDS(30))) {
		LOG_ERR("joined, but DHCP gave no address");
		return -ETIMEDOUT;
	}

	char ip[NET_IPV4_ADDR_LEN];

	net_addr_ntop(AF_INET, net_if_ipv4_get_global_addr(iface, NET_ADDR_PREFERRED), ip,
		      sizeof(ip));
	LOG_INF("connected, IP %s - listening on port %d", ip, PORT);

	int srv = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in a = {
		.sin_family = AF_INET,
		.sin_port = htons(PORT),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};

	if (srv < 0 || zsock_bind(srv, (struct sockaddr *)&a, sizeof(a)) ||
	    zsock_listen(srv, 1)) {
		LOG_ERR("cannot listen on port %d", PORT);
		return -errno;
	}

	for (;;) {
		int c = zsock_accept(srv, NULL, NULL);

		if (c < 0) {
			continue;
		}
		LOG_INF("client connected");
		for (int n = 0;; n++) {
			char msg[40];
			int len = snprintk(msg, sizeof(msg), "hello from nRF7002 %d\n", n);

			if (zsock_send(c, msg, len, 0) < 0) {
				break;
			}
			k_sleep(K_SECONDS(1));
		}
		zsock_close(c);
		LOG_INF("client left");
	}
}
