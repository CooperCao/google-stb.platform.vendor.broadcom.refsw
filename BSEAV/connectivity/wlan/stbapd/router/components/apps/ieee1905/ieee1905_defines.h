/*
 * All the common MACROs
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: ieee1905_defines.h 753156 2018-03-20 17:03:32Z sp888952 $
 */

#ifndef _IEEE1905_DEFINES_H_
#define _IEEE1905_DEFINES_H_

#ifdef MULTIAP
#include <typedefs.h>
#endif /* MULTIAP */

#define I5_MAX_IFNAME 16
#define MAC_ADDR_LEN 6
#define I5_DEVICE_FRIENDLY_NAME_LEN 32

#define I5_MEDIA_SPECIFIC_INFO_MAX_SIZE      10
#define I5_PHY_INTERFACE_NETTECHOUI_SIZE      3
#define I5_PHY_INTERFACE_NETTECHNAME_SIZE    32
#define I5_PHY_INTERFACE_URL_MAX_SIZE        64

#define I5_DM_BRIDGE_TUPLE_MAX_INTERFACES    64
#define FWD_IF_LIST_LEN (I5_DM_BRIDGE_TUPLE_MAX_INTERFACES*MAC_ADDR_LEN)

/* Try to get BSSID timeout */
#define I5_DM_GET_BSTA_BSSID_TIME_MSEC  2000

/* media types defined by specification */
#define I5_MEDIA_TYPE_FAST_ETH     0x0000
#define I5_MEDIA_TYPE_GIGA_ETH     0x0001
#define I5_MEDIA_TYPE_WIFI_B       0x0100
#define I5_MEDIA_TYPE_WIFI_G       0x0101
#define I5_MEDIA_TYPE_WIFI_A       0x0102
#define I5_MEDIA_TYPE_WIFI_N24     0x0103
#define I5_MEDIA_TYPE_WIFI_N5      0x0104
#define I5_MEDIA_TYPE_WIFI_AC      0x0105
#define I5_MEDIA_TYPE_WIFI_AD      0x0106
#define I5_MEDIA_TYPE_WIFI_AF      0x0107
#define I5_MEDIA_TYPE_1901_WAVELET 0x0200
#define I5_MEDIA_TYPE_1901_FFT     0x0201
#define I5_MEDIA_TYPE_MOCA_V11     0x0300
#define I5_MEDIA_TYPE_UNKNOWN      0xFFFF

/* internal media types */
#define I5_MEDIA_TYPE_BRIDGE       0x8000
#define I5_MATCH_MEDIA_TYPE_WL     0x9001
#define I5_MATCH_MEDIA_TYPE_ETH    0x9002
#define I5_MATCH_MEDIA_TYPE_PLC    0x9004
#define I5_MATCH_MEDIA_TYPE_ANY    0x9008

/* IEEE 802.11 Media specific information role of 4 bits (Bits 7 to 4) */
#define I5_MEDIA_INFO_ROLE_AP       0x00  /* Role is AP */
#define I5_MEDIA_INFO_ROLE_STA      0x40  /* Role is non-AP/non-PCP STA */

/* General Phy Media Types */
#define I5_GEN_PHY_HPAV2_NETTECHOUI_01   0x02
#define I5_GEN_PHY_HPAV2_NETTECHOUI_02   0x10
#define I5_GEN_PHY_HPAV2_NETTECHOUI_03   0x18

/* General Phy Variant Types */
#define I5_GEN_PHY_HPAV2_NETTECHVARIANT  0x42

#define I5_INTERMEDIATE_LEGACY_BRIDGE_FALSE  0
#define I5_INTERMEDIATE_LEGACY_BRIDGE_TRUE   1

#define I5_DM_LINK_METRIC_ACTIVATED_TIME_MSEC 15000


#define I5_DM_NODE_VERSION_UNKNOWN      0
#define I5_DM_NODE_VERSION_1905         1
#define I5_DM_NODE_VERSION_19051A       2
#define I5_DM_VERSION_TIMER_MSEC        500

#define I5_DM_LINK_METRICS_GET_INTERVAL_MSEC  60000  /* Get link metrics from the interface */

#ifdef MULTIAP
#define I5_MAX_CHANRCS				5
#define I5_RADIO_CAP_SIZE			512
#define I5_MAX_REGCLASS				256
#define IEEE1905_80211_CHAN_PER_REGCLASS	13
#define IEEE1905_MAX_RCCHANNELS			30

#define I5_DM_LINK_METRIC_UPDATE_TX     0x01
#define I5_DM_LINK_METRIC_UPDATE_RX     0x02
#define I5_DM_LINK_METRIC_UPDATE_LOCAL  0x04
#define I5_DM_LINK_METRIC_UPDATE_RAW    0x08 /* Update raw data without modifying */

#define IEEE1905_MAX_SSID_LEN   32
#define IEEE1905_ESP_LEN  3 /* Length of Estimated Service Parameters Information field */

enum {
  I5_REASON_UNSPECFIED = 0,
  I5_REASON_NON80211_INTF = 1,
  I5_REASON_80211_INTRA_OBSS_INTF_MGMT = 2,
  I5_REASON_80211_EXT_OBSS_INTF_MGMT = 3,
  I5_REASON_REDUCED_COVERAGE = 4, /* Lox tx power */
  I5_REASON_REDUCED_TPUT = 5, /* Limited bandwidth or high channel utilization */
  I5_REASON_INDEVICE_INTF = 6,
  I5_REASON_RADAR = 7,
  I5_REASON_BACKHAUL = 8,
  I5_REASON_DFS_CAC_COMPLETE = 9,
  I5_REASON_DFS_PASSIVE = 10,
};

enum {
  I5_MSG_NONE = 0,
  I5_MSG_RECEIVED = 1,
  I5_MSG_RESP_SENT = 2,
  I5_MSG_OP_CHAN_SENT = 3,
};
#endif /* MULTIAP */

enum {
  i5DmStateDone = 0,
  i5DmStateNew,
  i5DmStatePending,
  i5DmStateNotFound
};

#endif /* _IEEE1905_DEFINES_H_ */
