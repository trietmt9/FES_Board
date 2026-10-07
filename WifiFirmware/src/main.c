/*
 * nRF7002DK Wi-Fi scan — the smallest thing that proves the radio works.
 *
 * A scan is the right first step because it needs no credentials, so nothing
 * can be wrong except the parts you are trying to test: the SPI link to the
 * nRF7002, the firmware patch the driver loads into it, and the Wi-Fi stack.
 * If SSIDs appear, all three are good.
 *
 * Board target: nrf7002dk/nrf5340/cpuapp
 * Build and flash: see ../README.md
 *
 * ---------------------------------------------------------------------------
 * WHY THE CALLBACK DOES NOT PRINT
 *
 * Scan results arrive as a burst - tens of access points inside a few hundred
 * milliseconds, one net_mgmt event each, all delivered on the net_mgmt event
 * thread. Logging from that thread drops results, and it does it twice over:
 *
 *   1. Zephyr's log buffer is CONFIG_LOG_BUFFER_SIZE (1 KB by default) and the
 *      log thread only drains it every CONFIG_LOG_PROCESS_THREAD_SLEEP_MS
 *      (1 s by default). A burst overflows it and you get
 *      "--- N messages dropped ---" instead of the scan.
 *
 *   2. Whatever time the callback spends formatting is time the net_mgmt queue
 *      is not being serviced. That queue is CONFIG_NET_MGMT_EVENT_QUEUE_SIZE
 *      deep, and when it fills, results are dropped with no error anywhere.
 *
 * So the callback only COPIES each result - a memcpy and a counter - and main
 * does every bit of formatting after the scan has finished. The semaphore is
 * the handoff, and taking it is also what makes the counters safe to read:
 * k_sem_give/k_sem_take establishes the happens-before, so main sees every
 * store the event thread made before it signalled.
 * ---------------------------------------------------------------------------
 */

#include <wifi_app.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/wifi.h>
#include <string.h>

LOG_MODULE_REGISTER(wifi_app, LOG_LEVEL_INF);

/* Events this app listens for. Scan results arrive one event per AP, then a
 * single DONE event carrying the overall status. */
#define WIFI_EVENTS (NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE)

static struct net_mgmt_event_callback wifi_cb;

/* Results are stored, not printed. Written by the net_mgmt event thread, read
 * by main only after taking scan_done. */
static struct wifi_scan_result results[WIFI_APP_MAX_RESULTS];
static uint32_t result_count;       /* stored in results[] */
static uint32_t overflow_count;     /* seen but no room - reported, not hidden */
static int      scan_status;

/* Signals main() that the scan finished, so it does not have to poll or guess
 * at a delay. */
static K_SEM_DEFINE(scan_done, 0, 1);
1520904.pts-3.cgu-ubuntu
/**
 * @brief Store one access point. Deliberately does no formatting and no I/O.
 *
 * @note Runs on the net_mgmt event thread. Everything here is O(1) and
 *       non-blocking on purpose - see the note at the top of this file.
 */
static void on_scan_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_scan_result *r =
		(const struct wifi_scan_result *)cb->info;

	/* STEP 1: a full table is counted, never silently discarded. */
	if (result_count >= WIFI_APP_MAX_RESULTS) {
		overflow_count++;
		return;
	}

	/* STEP 2: copy it out. cb->info is only valid for this call. */
	memcpy(&results[result_count], r, sizeof(*r));
	result_count++;
}

/**
 * @brief Record the outcome and release main.
 *
 * @note Also on the net_mgmt event thread. The k_sem_give() is the last thing
 *       it does, so every store above is visible to whoever takes the
 *       semaphore.
 */
static void on_scan_done(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *s = (const struct wifi_status *)cb->info;

	scan_status = s->status;
	k_sem_give(&scan_done);
}

/**
 * @brief net_mgmt dispatcher.
 */
static void wifi_event_handler(struct net_mgmt_event_callback *cb,
			       uint64_t mgmt_event, struct net_if *iface)
{
	ARG_UNUSED(iface);

	switch (mgmt_event) {
	case NET_EVENT_WIFI_SCAN_RESULT:
		on_scan_result(cb);
		break;
	case NET_EVENT_WIFI_SCAN_DONE:
		on_scan_done(cb);
		break;
	default:
		break;
	}
}

int wifi_app_events_init(void)
{
	net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler, WIFI_EVENTS);
	net_mgmt_add_event_callback(&wifi_cb);
	return 0;
}

struct net_if *wifi_app_iface(void)
{
	return net_if_get_first_wifi();
}

int wifi_app_scan(struct net_if *iface)
{
	/* An all-zero params struct means "defaults": every band, every channel,
	 * active scan. Narrow it later (params.bands, params.dwell_time_active)
	 * once a plain scan is known to work. */
	struct wifi_scan_params params = { 0 };

	result_count = 0;
	overflow_count = 0;
	scan_status = 0;

	return net_mgmt(NET_REQUEST_WIFI_SCAN, iface, &params, sizeof(params));
}

/**
 * @brief Print the stored table. Runs on main, after the scan has finished.
 */
static void print_results(void)
{
	LOG_INF("#    | SSID                             | band      | chan | RSSI     | security");

	for (uint32_t i = 0; i < result_count; i++) {
		const struct wifi_scan_result *r = &results[i];

		/* An SSID is not a C string - it is ssid_length bytes, and a
		 * hidden network sends none at all, so never print it with %s. */
		LOG_INF("%-4u | %-32.32s | %-9s | %4u | %4d dBm | %s",
			i + 1,
			r->ssid_length ? (const char *)r->ssid : "<hidden>",
			wifi_band_txt(r->band),
			r->channel,
			r->rssi,
			wifi_security_txt(r->security));

		/* Deferred logging drains on a thread that sleeps between runs,
		 * so a long table can still outpace it. Yield every few lines and
		 * the drain keeps up without the buffer ever filling. */
		if ((i % 4) == 3) {
			k_msleep(20);
		}
	}

	if (overflow_count) {
		LOG_WRN("%u more AP(s) were found but the table holds only %u -"
			" raise WIFI_APP_MAX_RESULTS",
			overflow_count, (uint32_t)WIFI_APP_MAX_RESULTS);
	}
}

int main(void)
{
	LOG_INF("=====================================");
	LOG_INF(" nRF7002DK Wi-Fi scan");
	LOG_INF("=====================================");

	/* STEP 1: find the radio. The driver loads its firmware patch during
	 * boot, so give it a moment before deciding it is absent. */
	struct net_if *iface = NULL;

	for (int i = 0; i < 20 && iface == NULL; i++) {
		iface = wifi_app_iface();
		if (iface == NULL) {
			k_msleep(100);
		}
	}

	if (iface == NULL) {
		LOG_ERR("no Wi-Fi interface after 2 s.");
		LOG_ERR("  The nRF7002 did not come up - check that the nRF70");
		LOG_ERR("  blobs were fetched (west blobs fetch nrf_wifi) and");
		LOG_ERR("  that the board target is nrf7002dk/nrf5340/cpuapp.");
		return -ENODEV;
	}
	LOG_INF("Wi-Fi interface ready");

	/* STEP 2: subscribe before scanning, or early results are lost. */
	wifi_app_events_init();

	/* STEP 3: scan, then wait to be told it finished. */
	LOG_INF("scanning...");

	int ret = wifi_app_scan(iface);
	if (ret) {
		LOG_ERR("scan request rejected: %d", ret);
		return ret;
	}

	if (k_sem_take(&scan_done, K_SECONDS(30)) != 0) {
		LOG_WRN("no scan-done event within 30 s");
		return -ETIMEDOUT;
	}

	/* STEP 4: the scan is over and nothing is writing results[] any more,
	 * so printing can take as long as it likes. */
	if (scan_status) {
		LOG_ERR("scan failed (status %d)", scan_status);
		return scan_status;
	}

	print_results();
	LOG_INF("scan complete: %u access point(s)", result_count);
	LOG_INF("done. Reset the board to scan again.");
	return 0;
}
