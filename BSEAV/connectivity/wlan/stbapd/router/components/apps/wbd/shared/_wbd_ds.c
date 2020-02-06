/*
 * WBD data structure testing stub file
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: _wbd_ds.c 759905 2018-04-27 04:43:17Z sp888952 $
 */

#include "wbd.h"
#include "wbd_shared.h"
#include "wbd_ds.h"

#if 0 /* TODO Work on new data model */
/* ------------------------------------ Extern Declarations ------------------------------------ */

/* ------------------------------------ Extern Declarations ------------------------------------ */

/* =========================== UTILITY FUNCTIONS ================================ */
/*
static char *
ether_etoa(const unsigned char *e, char *a)
{
	char *c = a;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", e[i] & 0xff);
	}
	return a;
}
*/
/* =========================== UTILITY FUNCTIONS ================================ */

/* ============================ PRINT FUNCTIONS ================================== */
static void
wbd_print_monitor_sta_info(wbd_monitor_sta_item_t *sta_item,
	int print_sta_mac, char* format, int index)
{
	if (print_sta_mac) {
		printf(format, ETHER_TO_MACF(sta_item->sta_mac), sta_item->rssi);
	} else {
		printf(format, ETHER_TO_MACF(sta_item->slave_bssid), sta_item->rssi);
	}
}

static void
wbd_print_assoc_sta_info(wbd_assoc_sta_item_t *sta_item, char* format, int index)
{
	printf(format,
		index, ETHER_TO_MACF(sta_item->sta_mac),
		ETHER_TO_MACF(sta_item->slave_bssid),
		sta_item->stats.rssi,
		sta_item->stats.tx_rate,
		sta_item->stats.tx_failures);
}

static void
wbd_print_slave_info(wbd_slave_item_t *slave_item, char* format, int index)
{
	char ssidstr[SSID_FMT_BUF_LEN];
	double active_since = 0, active_before = 0;

	snprintf(ssidstr, sizeof(ssidstr), "%s", slave_item->wbd_ifr.blanket_ssid.SSID);
	active_since = difftime(slave_item->last_active_ts, slave_item->join_ts);
	active_before = difftime(time(NULL), slave_item->last_active_ts);

	printf(format,
		index, ETHER_TO_MACF(slave_item->wbd_ifr.bssid),
		CHSPEC_IS5G(slave_item->wbd_ifr.chanspec) ? 5 : 2,
		CHSPEC_CHANNEL(slave_item->wbd_ifr.chanspec),
		(CHSPEC_IS20(slave_item->wbd_ifr.chanspec)) ? 20 :
		((CHSPEC_IS40(slave_item->wbd_ifr.chanspec)) ? 40 :
		((CHSPEC_IS80(slave_item->wbd_ifr.chanspec)) ? 80 : 10)),
		slave_item->wbd_ifr.RSSI, ssidstr,
		(unsigned int)active_since, (unsigned int) active_before);
}

void
wbd_print_master_info(wbd_master_info_t *master_info)
{
	int i, j;
	wbd_slave_item_t* slave_item = NULL;
	dll_t *slave_item_p;
	wbd_assoc_sta_item_t* sta_item = NULL;
	wbd_monitor_sta_item_t* mon_sta_item = NULL;
	dll_t *sta_item_p, *mon_sta_item_p;

	printf(CLI_CMD_INFO_SLAVES_INFO, WBD_BAND_DIGIT(master_info->band),
		WBD_BAND_DTAIL(master_info->band), master_info->slave_list.count);

	/* Traverse Slave List for this Master Info */
	i = 0;
	foreach_glist_item(slave_item_p, master_info->slave_list) {

		slave_item = (wbd_slave_item_t*)slave_item_p;
		wbd_print_slave_info(slave_item, CLI_CMD_INFO_SLAVES_FMT, i+1);

		printf(CLI_CMD_INFO_ASOCLIST_INFO, slave_item->assoc_sta_list.count);

		/* Travese STA items associated with this Slave */
		j = 0;
		foreach_glist_item(sta_item_p, slave_item->assoc_sta_list) {

			sta_item = (wbd_assoc_sta_item_t*)sta_item_p;
			wbd_print_assoc_sta_info(sta_item, CLI_CMD_CLILST_CLIENTS_FMT, j+1);
			printf("\n");
			j++;
		}
		if (slave_item->assoc_sta_list.count == 0)
			printf("\tNO Entry Found \n");

		printf(CLI_CMD_INFO_MONLIST_INFO, slave_item->monitor_sta_list.count);

		/* Travese STA items monitored with this Slave */
		j = 0;
		foreach_glist_item(mon_sta_item_p, slave_item->monitor_sta_list) {

			mon_sta_item = (wbd_monitor_sta_item_t*)mon_sta_item_p;

			wbd_print_monitor_sta_info(mon_sta_item,
				1, CLI_CMD_CLILST_MONITOR_FMT, j+1);
			j++;
		}
		if (slave_item->monitor_sta_list.count == 0)
			printf("\tNO Entry Found \n");

		printf("\n");
		i++;
	}

	if (master_info->slave_list.count == 0)
		printf("NO Entry Found \n");
}

void
wbd_print_client_list(wbd_master_info_t *master_info)
{
	int peer_count = 0, client_count = 0;
	wbd_slave_item_t* parent_slave = NULL, *peer_slave = NULL;
	dll_t *slave_item_p, *slave_item2_p;
	wbd_assoc_sta_item_t* sta_item = NULL;
	wbd_monitor_sta_item_t* mon_sta_item = NULL;
	dll_t *sta_item_p;

	printf(CLI_CMD_CLILST_CLIENTS_INFO_MTR, WBD_BAND_DIGIT(master_info->band),
		WBD_BAND_DTAIL(master_info->band), master_info->blanket_client_count);

	/* Traverse Slave List for this Master Info */
	foreach_glist_item(slave_item_p, master_info->slave_list) {

		parent_slave = (wbd_slave_item_t*)slave_item_p;

		/* Travese STA items associated with this Slave */
		foreach_glist_item(sta_item_p, parent_slave->assoc_sta_list) {

			sta_item = (wbd_assoc_sta_item_t*)sta_item_p;

			client_count++;
			wbd_print_assoc_sta_info(sta_item,
				CLI_CMD_CLILST_CLIENTS_FMT, client_count);

			/* Look for unassociated Slave's monitor items */
			peer_count = 0;
			foreach_glist_item(slave_item2_p, master_info->slave_list) {

				peer_slave = (wbd_slave_item_t*)slave_item2_p;

				/* Slave Checking Condition */
				if (eacmp(&peer_slave->wbd_ifr.bssid,
					&parent_slave->wbd_ifr.bssid) == 0) {
					/* Peer slave is Parent Slave, skip its Monitor List */
					continue;
				}
				peer_count++;

#if 0 /* TODO */
				mon_sta_item = wbd_ds_find_sta_in_bss_monitorlist(peer_slave,
					&sta_item->sta_mac, &ret);
#endif /* #if 0 */

				if (!mon_sta_item) {
					continue;
				}

				wbd_print_monitor_sta_info(mon_sta_item,
					0, CLI_CMD_CLILST_MONITOR_FMT, peer_count);
			}
			printf("\n");
		}
	}
}


void
wbd_print_slave_list(wbd_master_info_t *master_info)
{
	int i;
	wbd_slave_item_t* slave_item;
	dll_t *slave_item_p;

	printf(CLI_CMD_INFO_SLAVES_INFO, WBD_BAND_DIGIT(master_info->band),
		WBD_BAND_DTAIL(master_info->band), master_info->slave_list.count);

	i = 0;
	foreach_glist_item(slave_item_p, master_info->slave_list) {

		slave_item = (wbd_slave_item_t*)slave_item_p;
		wbd_print_slave_info(slave_item, CLI_CMD_INFO_SLAVES_FMT, i+1);
		printf("\n");
		i++;
	}
}
/* ============================ PRINT FUNCTIONS ================================== */

/* ======================= ADD/REMOVE/UPDATE FUNCTIONS =========================== */

static void
wbd_ds_add_sta_in_slave_assoclist_HC(wbd_slave_item_t* slave_item, char* addr,
	int rssi, int tx_rate, int tx_failures)
{
	struct ether_addr sta_mac;
	wbd_assoc_sta_item_t *dst;

	wbd_ether_atoe(addr, &sta_mac);

	wbd_ds_add_sta_in_slave_assoclist(slave_item, &sta_mac, &dst);
	dst->stats.rssi = rssi;
	dst->stats.tx_rate = tx_rate;
	dst->stats.tx_failures = tx_failures;
}

static void
wbd_ds_add_sta_in_slave_monitorlist_HC(wbd_slave_item_t* slave_item, char* addr, int rssi)
{
#if 0 /* TODO */
	struct ether_addr sta_mac;
	wbd_monitor_sta_item_t *dst;

	wbd_ether_atoe(addr, &sta_mac);

	wbd_ds_add_sta_in_bss_monitorlist(slave_item, &sta_mac, &dst);
	dst->rssi = rssi;
#endif /* #if 0 */
}
/* ======================= ADD/REMOVE/UPDATE FUNCTIONS =========================== */
#endif /* #if 0 */

/* ================================================================================ */
/* -------------------------------- TEST FUNCTION --------------------------------- */
/* ================================================================================ */
int test_wbd_ds()
{
#if 0 /* TODO */
	int ret = 0;
	wbd_master_info_t* master_info;
	wbd_slave_item_t* parent_slave = NULL;
	struct ether_addr slave_bssid;

	/* Initialize Master Info Structure */
	master_info = wbd_ds_master_info_init(WBD_BAND_LAN_5GL, &ret);

	/* Add Slaves */

	wbd_ether_atoe("11:22:33:44:55:66", &slave_bssid);
	parent_slave = wbd_ds_find_slave_in_master(master_info, &slave_bssid, WBD_CMP_BSSID,
		&ret);
	/* Add Associated STAs */
	wbd_ds_add_sta_in_slave_assoclist_HC(parent_slave,
		"00:00:00:00:00:11", -10, 5, 100);
	wbd_ds_add_sta_in_slave_assoclist_HC(parent_slave,
		"00:00:00:00:00:22", -20, 4, 120);
	wbd_ds_add_sta_in_slave_assoclist_HC(parent_slave,
		"00:00:00:00:00:33", -30, 3, 110);
	/* Add Monitored STAs */
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:44", -60);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:55", -50);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:66", -70);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:77", -60);

	wbd_ether_atoe("22:33:44:55:66:77", &slave_bssid);
	parent_slave = wbd_ds_find_slave_in_master(master_info, &slave_bssid, WBD_CMP_BSSID,
		&ret);
	/* Add Associated STAs */
	wbd_ds_add_sta_in_slave_assoclist_HC(parent_slave,
		"00:00:00:00:00:44", -10, 2, 150);
	wbd_ds_add_sta_in_slave_assoclist_HC(parent_slave,
		"00:00:00:00:00:55", -20, 2, 120);
	/* Add Monitored STAs */
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:11", -60);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:22", -50);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:33", -70);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:66", -60);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:77", -10);

	wbd_ether_atoe("11:22:33:44:55:77", &slave_bssid);
	parent_slave = wbd_ds_find_slave_in_master(master_info, &slave_bssid, WBD_CMP_BSSID,
		&ret);
	/* Add Associated STAs */
	wbd_ds_add_sta_in_slave_assoclist_HC(parent_slave,
		"00:00:00:00:00:66", -10, 4, 130);
	wbd_ds_add_sta_in_slave_assoclist_HC(parent_slave,
		"00:00:00:00:00:77", -90, 5, 140);
	/* Add Monitored STAs */
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:11", -60);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:22", -50);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:33", -60);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:44", -70);
	wbd_ds_add_sta_in_slave_monitorlist_HC(parent_slave, "00:00:00:00:00:55", -40);

	/* Print Slave List */
	wbd_print_slave_list(master_info);

	/* Display Blanket */
	wbd_print_master_info(master_info);

	/* Print Client List */
	wbd_print_client_list(master_info);

	wbd_ds_master_info_cleanup(master_info, TRUE);
#endif /* #if 0 */
	return 0;
}
/* ================================================================================ */
