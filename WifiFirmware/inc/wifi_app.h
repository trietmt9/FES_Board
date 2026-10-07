/**
 * @file wifi_app.h
 * @brief Wi-Fi helpers for the nRF7002DK learning app.
 *
 * Deliberately small. One job per function, so each can be read, flashed and
 * understood on its own before the next is added.
 */

#ifndef __WIFI_APP_H__
#define __WIFI_APP_H__

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>

/**
 * @brief How many access points the scan table holds.
 *
 * Results are stored, not printed, as they arrive - printing from the
 * net_mgmt event thread drops them (see the note at the top of main.c). This
 * is the size of that store. Anything beyond it is counted and reported, never
 * silently discarded. 32 covers a busy office; a crowded apartment block can
 * exceed it.
 */
#define WIFI_APP_MAX_RESULTS 32

/**
 * @brief Subscribe to the Wi-Fi management events this app prints.
 *
 * Register BEFORE starting a scan. Results are delivered as net_mgmt events,
 * one per access point, and any that arrive before the callback exists are
 * simply lost - there is no backlog to replay.
 *
 * @return 0 on success, negative errno on failure.
 */
int wifi_app_events_init(void);

/**
 * @brief Find the Wi-Fi interface.
 *
 * On the nRF7002DK this is the nRF7002 reached over SPI from the nRF5340
 * application core. It exists only once the driver has loaded its firmware
 * patch from the nRF70 blob, so a NULL return usually means the driver failed
 * to bring the radio up rather than that the board is wrong.
 *
 * @return The interface, or NULL if there is no Wi-Fi interface.
 */
struct net_if *wifi_app_iface(void);

/**
 * @brief Start an access-point scan.
 *
 * Returns as soon as the request is accepted; results arrive later on the
 * callback registered by wifi_app_events_init(). Completion is signalled by
 * NET_EVENT_WIFI_SCAN_DONE, not by this function returning.
 *
 * Clears the result table, so do not call it while a previous scan is still
 * running.
 *
 * @param iface Interface from wifi_app_iface().
 * @return 0 if the scan was accepted, negative errno otherwise.
 */
int wifi_app_scan(struct net_if *iface);

#endif /* __WIFI_APP_H__ */
