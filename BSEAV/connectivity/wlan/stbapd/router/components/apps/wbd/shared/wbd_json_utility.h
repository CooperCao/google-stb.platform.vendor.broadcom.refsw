/*
 * WBD JSON format creation and parsing
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_json_utility.h 765843 2018-07-18 11:09:14Z sp888952 $
 */

#ifndef _WBD_JSON_UTILITY_H_
#define _WBD_JSON_UTILITY_H_

#include <json.h>
#include "wbd.h"
#include "wbd_shared.h"

#define JSON_TAG_CMD			"Cmd"
#define JSON_TAG_BAND			"Band"
#define JSON_TAG_DATA			"Data"
#define JSON_TAG_BSSID			"BSSID"
#define JSON_TAG_SSID			"SSID"
#define JSON_TAG_MAC			"MAC"
#ifdef PLC_WBD
#define JSON_TAG_PLC_MAC		"PLCMAC"
#endif /* PLC_WBD */
#define JSON_TAG_ERRORCODE		"Error"
#define JSON_TAG_REASON			"Reason"
#define JSON_TAG_RSSI			"RSSI"
#define JSON_TAG_CLI_SUBCMD		"SubCmd"
#define JSON_TAG_INTERVAL		"Interval"
#define JSON_TAG_REPETITION		"Repetition"
#define JSON_TAG_PRIORITY		"Priority"
#define JSON_TAG_MACLIST		"MACList"
#define JSON_TAG_CLI_STA_PRIORITY	"StaPrio"
#define JSON_TAG_CHANSPEC		"Chanspec"
#define JSON_TAG_RCLASS			"RClass"
#define JSON_TAG_TX_FAILURES		"TxFailures"
#define JSON_TAG_TX_RATE		"TxRate"
#define JSON_TAG_STATYPE		"STAType"
#define JSON_TAG_HOPCOUNT		"HopCount"
#define JSON_TAG_NVRAM			"Nvram"
#define JSON_TAG_DWELL			"Dwell"
#define JSON_TAG_SLAVE_TYPE		"SlaveType"
#define JSON_TAG_BRIDGE_TYPE		"BridgeType"
#define JSON_TAG_FLAGS			"Flags"
#define JSON_TAG_DFS_CHAN_LIST		"Dfschans"
#define JSON_TAG_TWOG			"TwoG"
#define JSON_TAG_FIVEG_LOW		"FiveGLow"
#define JSON_TAG_FIVEG_HIGH		"FiveGHigh"
#define JSON_TAG_STATUS			"Status"
#define JSON_TAG_UPLINK_MAC		"UplinkMAC"
#define JSON_TAG_CHANNEL		"Channel"
#define JSON_TAG_BITMAP			"Bitmap"
#define JSON_TAG_CHAN_INFO_LIST		"ChanInfoList"
#define JSON_TAG_CHANSPEC_LIST		"Chanspecs"
#define JSON_TAG_NSS			"Nss"
#define JSON_TAG_AVG_TX_RATE		"AvgTxRate"
#define JSON_TAG_CAPABILITY_LIST	"CapabilityList"
#define JSON_TAG_PREFIX			"Prefix"
#define JSON_TAG_IFNAME			"Ifname"
#define JSON_TAG_BRIDGE_ADDR		"BridgeAddr"
#define JSON_TAG_MASTER_LOGS		"MasterLogs"
#define JSON_TAG_TAF			"Taf"
#define JSON_TAG_TAF_STA		"StaTaf"
#define JSON_TAG_TAF_BSS		"BssTaf"
#define JSON_TAG_TAF_DEF		"defTaf"
#define JSON_TAG_TX_POWER		"TxPower"
#define JSON_TAG_BSSID_INFO		"BssidInfo"
#define JSON_TAG_PHYTYPE		"PhyType"
#define JSON_TAG_SRC_MAC		"SrcMAC"
#define JSON_TAG_DST_MAC		"DstMAC"
#define JSON_TAG_MAP_FLAGS		"MapFlags"
#define JSON_TAG_REP_RSSI		"ReportedRSSI"
#define JSON_TAG_DATA_RATE		"DataRate"
#define JSON_TAG_RX_TOT_PKTS		"RxTotPkts"
#define JSON_TAG_RX_TOT_BYTES		"RxTotBytes"
/* ------------------------------ HEALPER FUNCTIONS ------------------------------------- */

/* Get the command ID from the json data */
extern wbd_com_cmd_type_t wbd_json_parse_cmd_name(const void *data);

/* Get the CLI command ID from the json data */
extern wbd_com_cmd_type_t wbd_json_parse_cli_cmd_name(const void *data);

/* ------------------------------ HEALPER FUNCTIONS ------------------------------------- */


/* ------------------------------- CREATE FUNCTIONS ------------------------------------- */

/* Create CLI command request and get the JSON data in string format */
extern void* wbd_json_create_cli_cmd(void *data);

/* Creates common JSON response with message */
extern char* wbd_json_create_common_resp(wbd_cmd_param_t *cmdparam, int error_code);

/* Returns cli sata in json format for UI */
extern char* wbd_json_create_cli_info(wbd_info_t *info, wbd_cli_send_data_t *clidata);

/* Create cli logs in json format for UI */
extern char* wbd_json_create_cli_logs(wbd_info_t *info, wbd_cli_send_data_t *clidata);

/* ------------------------------- CREATE FUNCTIONS ------------------------------------- */


/* ------------------------------ PARSING FUNCTIONS ------------------------------------- */

/* Parse CLI command and fill the structure */
extern void* wbd_json_parse_cli_cmd(void *data);

/* Parse common JSON response and fill the structure */
extern void* wbd_json_parse_common_resp(void *data);

/* Parse WEAK_CLIENT_BSD command and fill the structure */
extern void* wbd_json_parse_weak_client_bsd_cmd(void *data);

/* Parse WEAK_CANCEL_BSD command and fill the structure */
extern void* wbd_json_parse_weak_cancel_bsd_cmd(void *data);

/* Parse STA_STATUS_BSD command and fill the structure */
extern void* wbd_json_parse_sta_status_bsd_cmd(void *data);

/* Parse BLOCK_CLIENT_BSD command and fill the structure */
extern void* wbd_json_parse_blk_client_bsd_cmd(void *data);
/* ------------------------------ PARSING FUNCTIONS ------------------------------------- */
#endif /* _WBD_JSON_UTILITY_H_ */
