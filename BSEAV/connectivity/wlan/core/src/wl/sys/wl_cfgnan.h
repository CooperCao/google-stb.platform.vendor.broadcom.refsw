/*
 * Neighbor Awareness Networking
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

#ifndef _wl_cfgnan_h_
#define _wl_cfgnan_h_

#define WL_NAN_IOV_BATCH_VERSION	0x8000
#define WL_NAN_AVAIL_REPEAT_INTVL	0x0200
#define WL_NAN_AVAIL_START_INTVL	160
#define WL_NAN_AVAIL_DURATION_INTVL	336
#define NAN_IOCTL_BUF_SIZE		1524
#define NAN_EVENT_NAME_MAX_LEN		40
#define NAN_CONFIG_ATTR_MAX_LEN		24
#define NAN_RTT_IOVAR_BUF_SIZE		1024
#define WL_NAN_EVENT_CLEAR_BIT		32
#define NAN_EVENT_MASK_ALL			0x7fffffff

#define NAN_MAXIMUM_ID_NUMBER 255
#define NAN_MAXIMUM_MASTER_PREFERENCE 255
#define NAN_ID_MIN	0
#define NAN_ID_MAX	255
#ifdef NAN_DP
#define MAX_IF_ADD_WAIT_TIME	1000
#endif /* NAN_DP */
#define NAN_INVALID_ID(id)	(id > NAN_MAXIMUM_ID_NUMBER)
#define NAN_INVALID_ROLE(role)	(role > WL_NAN_ROLE_ANCHOR_MASTER)
#define NAN_INVALID_CHANSPEC(chanspec)	((chanspec == INVCHANSPEC) || \
	(chanspec == 0))
#define NAN_INVALID_EVENT(num)	((num < WL_NAN_EVENT_START) || \
	(num >= WL_NAN_EVENT_INVALID))
#define NAN_INVALID_PROXD_EVENT(num)	(num != WLC_E_PROXD_NAN_EVENT)
#define NAN_EVENT_BIT(event) (1U << (event - WL_NAN_EVENT_START))
#define NAME_TO_STR(name) #name
#define NAN_ID_CTRL_SIZE ((NAN_MAXIMUM_ID_NUMBER/8) + 1)

#define SUPP_EVENT_PREFIX		"CTRL-EVENT-"
#define EVENT_RTT_STATUS_STR	"NAN-RTT-STATUS"

#define TIMESTAMP_PREFIX	"TSF="			/* timestamp */
#define AMR_PREFIX			"AMR="			/* anchor master rank */
#define DISTANCE_PREFIX		"DIST="			/* distance */
#define ATTR_PREFIX			"ATTR="			/* attribute */
#define ROLE_PREFIX			"ROLE="			/* role */
#define CHAN_PREFIX			"CHAN="			/* channel */
#define BITMAP_PREFIX		"BMAP="			/* bitmap */
#define DEBUG_PREFIX		"DEBUG="		/* debug enable/disable flag */
#define DW_LEN_PREFIX		"DW_LEN="		/* discovery window length */
#define DW_INT_PREFIX		"DW_INT="		/* discovery window interval */
#define STATUS_PREFIX		"STATUS="		/* status */
#define PUB_ID_PREFIX		"PUB_ID="		/* publisher id */
#define SUB_ID_PREFIX		"SUB_ID="		/* subscriber id */
#define INSTANCE_ID_PREFIX		"LOCAL_ID="		/* Instance id */
#define REMOTE_INSTANCE_ID_PREFIX		"PEER_ID="		/* Peer id */

#ifdef NAN_P2P_CONFIG
#define P2P_IE_PREFIX		"P2P_IE="		/* p2p ie  id */
#define IE_EN_PREFIX		"ENBLE_IE="		/* enable p2p ie  */
#endif
#define PUB_PR_PREFIX		"PUB_PR="		/* publish period */
#define PUB_INT_PREFIX		"PUB_INT="		/* publish interval (ttl) */
#define CLUS_ID_PREFIX		"CLUS_ID="		/* cluster id */
#define IF_ADDR_PREFIX		"IF_ADDR="		/* IF address */
#define MAC_ADDR_PREFIX		"MAC_ADDR="		/* mac address */
#define SVC_HASH_PREFIX		"SVC_HASH="		/* service hash */
#define SVC_INFO_PREFIX		"SVC_INFO="		/* service information */
#define HOP_COUNT_PREFIX	"HOP_COUNT="	/* hop count */
#define MASTER_PREF_PREFIX	"MASTER_PREF="	/* master preference */
#define ACTIVE_OPTION		"ACTIVE"		/* Active Subscribe. */
#define SOLICITED_OPTION	"SOLICITED"		/* Solicited Publish. */
#define UNSOLICITED_OPTION	"UNSOLICITED"	/* Unsolicited Publish. */
/* anchor master beacon transmission time */
#define AMBTT_PREFIX		"AMBTT="
/* passive scan period for cluster merge */
#define SCAN_PERIOD_PREFIX	"SCAN_PERIOD="
/* passive scan interval for cluster merge */
#define SCAN_INTERVAL_PREFIX	"SCAN_INTERVAL="
#define BCN_INTERVAL_PREFIX		"BCN_INTERVAL="

#define NAN_EVENT_STR_STARTED               "NAN-STARTED"
#define NAN_EVENT_STR_JOINED                "NAN-JOINED"
#define NAN_EVENT_STR_ROLE_CHANGE           "NAN-ROLE-CHANGE"
#define NAN_EVENT_STR_SCAN_COMPLETE         "NAN-SCAN-COMPLETE"
#define NAN_EVENT_STR_SDF_RX                "NAN-SDF-RX"
#define NAN_EVENT_STR_REPLIED               "NAN-REPLIED"
#define NAN_EVENT_STR_TERMINATED            "NAN-TERMINATED"
#define NAN_EVENT_STR_FOLLOWUP_RX           "NAN-FOLLOWUP-RX"
#define NAN_EVENT_STR_STATUS_CHANGE         "NAN-STATUS-CHANGE"
#define NAN_EVENT_STR_MERGED                "NAN-MERGED"
#define NAN_EVENT_STR_STOPPED               "NAN-STOPPED"
#define NAN_EVENT_STR_P2P_RX                "NAN-P2P-RX"
#define NAN_EVENT_STR_WINDOW_BEGUN_P2P      "NAN-WINDOW-BEGUN-P2P"
#define NAN_EVENT_STR_WINDOW_BEGUN_MESH     "NAN-WINDOW-BEGUN-MESH"
#define NAN_EVENT_STR_WINDOW_BEGUN_IBSS     "NAN-WINDOW-BEGUN-IBSS"
#define NAN_EVENT_STR_WINDOW_BEGUN_RANGING  "NAN-WINDOW-BEGUN-RANGING"
#define NAN_EVENT_STR_INVALID               "NAN-INVALID"
#define tolower(c) bcm_tolower(c)

#define NMR2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5], (a)[6], (a)[7]
#define NMRSTR "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x"

#define NAN_DBG_ENTER() {WL_DBG(("Enter: %s\n", __FUNCTION__));}
#define NAN_DBG_EXIT() {WL_DBG(("Exit: %s\n", __FUNCTION__));}

#ifndef strtoul
#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))
#endif


#ifdef NAN_DP
#define NAN_MAC_ADDR_LEN 6
#define NAN_DP_MAX_APP_INFO_LEN	512

typedef uint32 nan_data_path_id;
/*
 * Data request Initiator/Responder
 * app/service related info
 */
typedef struct nan_data_path_app_info {
	uint16 ndp_app_info_len;
	uint8 ndp_app_info[NAN_DP_MAX_APP_INFO_LEN];
} nan_data_path_app_info_t;

/* QoS configuration */
typedef enum {
	NAN_DP_CONFIG_NO_QOS = 0,
	NAN_DP_CONFIG_QOS
} nan_data_path_qos_cfg_t;

/* Data request Responder's response */
typedef enum {
	NAN_DP_REQUEST_ACCEPT = 0,
	NAN_DP_REQUEST_REJECT
} nan_data_path_response_code_t;

/* NAN DP security Configuration */
typedef enum {
	NAN_DP_CONFIG_NO_SECURITY = 0,
	NAN_DP_CONFIG_SECURITY
} nan_data_path_security_cfg_status_t;

/* Configuration params of Data request Initiator/Responder */
typedef struct nan_data_path_cfg {
	/* Status Indicating Security/No Security */
	nan_data_path_security_cfg_status_t security_cfg;
	nan_data_path_qos_cfg_t qos_cfg;
} nan_data_path_cfg_t;

enum nan_dp_states {
	NAN_DP_STATE_DISABLED = 0,
	NAN_DP_STATE_ENABLED = 1
};

typedef struct nan_data_path_peer {
	struct ether_addr addr;        /* peer mac address */
	chanspec_t chanspec;           /* Channel Specification */
} nan_data_path_peer_t;

#endif /* NAN_DP */

enum nan_de_event_type {
	NAN_EVENT_START = 0,
	NAN_EVENT_JOIN = 1,
	NAN_EVENT_MERGE = 2,
	NAN_EVENT_ROLE = 3
};

enum {
	SRF_TYPE_SEQ_MAC_ADDR = 0,
	SRF_TYPE_BLOOM_FILTER = 1
};

/* NAN Match indication type */
typedef enum {
    NAN_MATCH_ALG_MATCH_ONCE		= 0,
    NAN_MATCH_ALG_MATCH_CONTINUOUS	= 1,
    NAN_MATCH_ALG_MATCH_NEVER		= 2
} nan_match_alg;

typedef struct nan_str_data {
	uint32 dlen;
	uint8 *data;
} nan_str_data_t;

typedef struct nan_mac_list {
	uint32 num_mac_addr;
	uint8 *list;
} nan_mac_list_t;

typedef struct nan_config_attr {
	char name[NAN_CONFIG_ATTR_MAX_LEN];	/* attribute name */
	uint16 type;							/* attribute xtlv type */
} nan_config_attr_t;

typedef struct wl_nan_sid_beacon_tune {
	uint8 sid_enable;	/* flag for sending service id in beacon */
	uint8 sid_count;	/* Limit for number of SIDs to be included in Beacons */
} wl_nan_sid_beacon_ctrl_t;

typedef struct nan_cmd_data {
	nan_config_attr_t attr;			/* set config attributes */
	nan_str_data_t svc_hash;		/* service hash */
	nan_str_data_t svc_info;		/* service information */
	nan_str_data_t p2p_info;		/* p2p information */
	struct ether_addr mac_addr;		/* mac address */
	struct ether_addr clus_id;		/* cluster id */
	struct ether_addr if_addr;		/* if addr */
	nan_str_data_t rx_match;	/* matching filter rx */
	nan_str_data_t tx_match;	/* matching filter tx */
	nan_mac_list_t mac_list;   /* mac list */
	wl_nan_sid_beacon_ctrl_t sid_beacon;          /* sending service id in beacon */
	chanspec_t chanspec;			/* channel */
	wl_nan_instance_id_t pub_id;	/* publisher id */
	wl_nan_instance_id_t sub_id;	/* subscriber id */
	wl_nan_instance_id_t local_id;	/* Local id */
	wl_nan_instance_id_t remote_id;	/* Remote id */
	uint32 beacon_int;					/* beacon interval */
	uint32 ttl;                      /* time to live */
	uint32 period;                   /* publish period */
	uint32 bmap;						/* bitmap */
	uint32 role;						/* role */
	uint32 flags;					/* Flag bits */
	uint32 nan_oui;            /* configured nan oui */
	uint32 warmup_time;	/* Warm up time */
	uint16 dw_len;						/* discovery window length */
	uint16 master_pref;				/* master preference */
	uint8 debug_flag;					/* debug enable/disable flag */
	uint8 life_count;             /* life count of the instance */
	uint8 srf_type;               /* SRF type */
	uint8 srf_include;            /* SRF include */
	uint8 use_srf;                /* use SRF */
	uint8 hop_count_limit;     /* hop count limit */
	uint8 nan_band;            /* nan band <A/B/AUTO> */
	uint8 support_5g;          /* To decide dual band support */
	uint8 priority;             /* Priority of Transmit */
	uint8 random_factor;	/* Random factor */

	/* Additional from wifi_nan.h */
	u8 sdf_5g_val;
	u8 config_2dot4g_rssi_close;
	u8 config_2dot4g_rssi_middle;
	u8 rssi_proximity_2dot4g_val;
	u8 recv_ind_flag;                /* Receive Indication Flag */
#ifdef NAN_DP
	nan_data_path_response_code_t rsp_code;
	struct ether_addr data_cluster_id;		/* data cluster id */
	nan_data_path_id ndp_instance_id;
	nan_data_path_id ndp_instances [0];
	nan_data_path_cfg_t ndp_cfg;
	char ndp_iface[IFNAMSIZ+1];
	char channel[IFNAMSIZ+1];
	uint16 service_instance_id;
	uint8 peer_disc_mac_addr[ETHER_ADDR_LEN];
	uint8 peer_ndi_mac_addr[ETHER_ADDR_LEN];
	uint8 num_ndp_instances;
#endif /* NAN_DP */
} nan_cmd_data_t;

typedef int (nan_func_t)(struct net_device *ndev, struct bcm_cfg80211 *cfg,
	char *cmd, int size, nan_cmd_data_t *cmd_data);

typedef struct nan_cmd {
	const char *name;					/* command name */
	nan_func_t *func;					/* command hadler */
} nan_cmd_t;

typedef struct nan_event_hdr {
	uint32 flags;							/* future use */
	uint16 event_subtype;
} nan_event_hdr_t;

typedef struct wl_nan_tlv_data {
	wl_nan_cfg_status_t nstatus;			/* status data */
	wl_nan_sd_params_t params;		/* discovery parameters */
	struct ether_addr mac_addr;			/* peer mac address */
	struct ether_addr clus_id;			/* cluster id */
	nan_str_data_t svc_info;			/* service info */
	nan_str_data_t vend_info;			/* vendor info */
	nan_scan_params_t scan_params;		/* scan_param */
	/* anchor master beacon transmission time */
	uint32 ambtt;
	uint32 dev_role;						/* device role */
	uint16 inst_id;						/* instance id */
	uint16 peer_inst_id;					/* Peer instance id */
	uint16 pub_id;							/* publisher id */
	uint16 sub_id;							/* subscriber id */
	uint16 master_pref;					/* master preference */
	chanspec_t chanspec;				/* channel */
	uint8 amr[WL_NAN_MASTER_RANK_LEN];		/* anchor master role */
	uint8 svc_name[WL_NAN_SVC_HASH_LEN];	/* service name */
	uint8 hop_count;						/* hop count */
	uint8 enabled;							/* nan status flag */
	int reason_code;              /* reason code */
#ifdef NAN_DP
	nan_data_path_cfg_t ndp_cfg;
	nan_data_path_response_code_t rsp_code;
	struct ether_addr peer_mac_addr;
	struct ether_addr peer_data_if_addr;
	struct ether_addr peer_disc_mac_addr;
	struct ether_addr data_if_addr; /* local data if address */
	uint8 status;
	uint8 ndp_id;
#endif /* NAN_DP */
} wl_nan_tlv_data_t;

typedef struct _nan_de_event_data {
	wl_nan_cfg_status_t *nstatus;
	uint8 nan_de_evt_type;
} nan_de_event_data_t;

typedef struct _nan_hal_resp {
	unsigned short instance_id;
	unsigned short subcmd;
	int status;
	int value;
#ifdef NAN_DP
	/* Identifier for the instance of the NDP */
	int ndp_instance_id;
#endif
} nan_hal_resp_t;

typedef struct wl_nan_sub_cmd wl_nan_sub_cmd_t;
typedef int (cmd_handler_t)(void *wl, const wl_nan_sub_cmd_t *cmd,
		int argc, char **argv, bool *is_set, uint8 *iov_data,
		uint16 *avail_len);

/*
 * nan sub-commands list entry
 */
struct wl_nan_sub_cmd {
	char *name;
	uint8 version;
	uint16 id;
	uint16 type;
	cmd_handler_t *handler;
};

typedef struct wl_nan_iov {
	uint16 nan_iov_len;
	uint8 *nan_iov_buf;
} wl_nan_iov_t;

extern int wl_cfgnan_set_vars_cbfn(void *ctx, const uint8 *tlv_buf,
	uint16 type, uint16 len);
extern int wl_cfgnan_enable_events(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, wl_nan_iov_t *nan_iov_data);
extern int wl_cfgnan_start_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_stop_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_support_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_status_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_pub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_p2p_ie_add_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_p2p_ie_enable_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_p2p_ie_del_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);

extern int wl_cfgnan_sub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_cancel_pub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_cancel_sub_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_transmit_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_set_config_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_rtt_config_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
extern int wl_cfgnan_rtt_find_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
#ifdef WL_NAN_DEBUG
extern int wl_cfgnan_debug_handler(struct net_device *ndev,
	struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
#endif /* WL_NAN_DEBUG */
extern int wl_cfgnan_cmd_handler(struct net_device *dev,
	struct bcm_cfg80211 *cfg, char *cmd, int cmd_len);
extern s32 wl_cfgnan_notify_nan_status(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *e, void *data);
extern s32 wl_cfgnan_notify_proxd_status(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *e, void *data);
extern int wl_cfgnan_generate_inst_id(struct bcm_cfg80211 *cfg,
	uint8 inst_type, uint8 *p_inst_id);
extern int wl_cfgnan_validate_inst_id(struct bcm_cfg80211 *cfg,
	uint8 inst_id);
extern int wl_cfgnan_remove_inst_id(struct bcm_cfg80211 *cfg,
	uint8 inst_id);
extern int wl_cfgnan_get_inst_type(struct bcm_cfg80211 *cfg,
		uint8 inst_id, uint8 *inst_type);
extern int bcm_xtlv_size_for_data(int dlen, bcm_xtlv_opts_t opts);
extern int bcm_xtlv_size_for_data(int dlen, bcm_xtlv_opts_t opts);

typedef enum {
	NAN_ATTRIBUTE_HEADER				= 100,
	NAN_ATTRIBUTE_HANDLE				= 101,
	NAN_ATTRIBUTE_TRANSAC_ID			= 102,

	/* NAN Enable request attributes */
	NAN_ATTRIBUTE_5G_SUPPORT			= 103,
	NAN_ATTRIBUTE_CLUSTER_LOW			= 104,
	NAN_ATTRIBUTE_CLUSTER_HIGH			= 105,
	NAN_ATTRIBUTE_SID_BEACON			= 106,
	NAN_ATTRIBUTE_SYNC_DISC_5G			= 107,
	NAN_ATTRIBUTE_RSSI_CLOSE			= 108,
	NAN_ATTRIBUTE_RSSI_MIDDLE			= 109,
	NAN_ATTRIBUTE_RSSI_PROXIMITY			= 110,
	NAN_ATTRIBUTE_HOP_COUNT_LIMIT			= 111,
	NAN_ATTRIBUTE_RANDOM_TIME			= 112,
	NAN_ATTRIBUTE_MASTER_PREF			= 113,
	NAN_ATTRIBUTE_PERIODIC_SCAN_INTERVAL		= 114,

	/* Nan Publish/Subscribe request Attributes */
	NAN_ATTRIBUTE_PUBLISH_ID			= 115,
	NAN_ATTRIBUTE_TTL				= 116,
	NAN_ATTRIBUTE_PERIOD				= 117,
	NAN_ATTRIBUTE_REPLIED_EVENT_FLAG		= 118,
	NAN_ATTRIBUTE_PUBLISH_TYPE			= 119,
	NAN_ATTRIBUTE_TX_TYPE				= 120,
	NAN_ATTRIBUTE_PUBLISH_COUNT			= 121,
	NAN_ATTRIBUTE_SERVICE_NAME_LEN			= 122,
	NAN_ATTRIBUTE_SERVICE_NAME			= 123,
	NAN_ATTRIBUTE_SERVICE_SPECIFIC_INFO_LEN		= 124,
	NAN_ATTRIBUTE_SERVICE_SPECIFIC_INFO		= 125,
	NAN_ATTRIBUTE_RX_MATCH_FILTER_LEN		= 126,
	NAN_ATTRIBUTE_RX_MATCH_FILTER			= 127,
	NAN_ATTRIBUTE_TX_MATCH_FILTER_LEN		= 128,
	NAN_ATTRIBUTE_TX_MATCH_FILTER			= 129,
	NAN_ATTRIBUTE_SUBSCRIBE_ID			= 130,
	NAN_ATTRIBUTE_SUBSCRIBE_TYPE			= 131,
	NAN_ATTRIBUTE_SERVICERESPONSEFILTER		= 132,
	NAN_ATTRIBUTE_SERVICERESPONSEINCLUDE		= 133,
	NAN_ATTRIBUTE_USESERVICERESPONSEFILTER		= 134,
	NAN_ATTRIBUTE_SSIREQUIREDFORMATCHINDICATION	= 135,
	NAN_ATTRIBUTE_SUBSCRIBE_MATCH			= 136,
	NAN_ATTRIBUTE_SUBSCRIBE_COUNT			= 137,
	NAN_ATTRIBUTE_MAC_ADDR				= 138,
	NAN_ATTRIBUTE_MAC_ADDR_LIST			= 139,
	NAN_ATTRIBUTE_MAC_ADDR_LIST_NUM_ENTRIES		= 140,
	NAN_ATTRIBUTE_PUBLISH_MATCH			= 141,

	/* Nan Event attributes */
	NAN_ATTRIBUTE_ENABLE_STATUS			= 142,
	NAN_ATTRIBUTE_JOIN_STATUS			= 143,
	NAN_ATTRIBUTE_ROLE				= 144,
	NAN_ATTRIBUTE_MASTER_RANK			= 145,
	NAN_ATTRIBUTE_ANCHOR_MASTER_RANK		= 146,
	NAN_ATTRIBUTE_CNT_PEND_TXFRM			= 147,
	NAN_ATTRIBUTE_CNT_BCN_TX			= 148,
	NAN_ATTRIBUTE_CNT_BCN_RX			= 149,
	NAN_ATTRIBUTE_CNT_SVC_DISC_TX			= 150,
	NAN_ATTRIBUTE_CNT_SVC_DISC_RX			= 151,
	NAN_ATTRIBUTE_AMBTT				= 152,
	NAN_ATTRIBUTE_CLUSTER_ID			= 153,
	NAN_ATTRIBUTE_INST_ID				= 154,
	NAN_ATTRIBUTE_OUI				= 155,
	NAN_ATTRIBUTE_STATUS				= 156,
	NAN_ATTRIBUTE_DE_EVENT_TYPE			= 157,
	NAN_ATTRIBUTE_MERGE				= 158,

	/* New wifi_nan.h */
	NAN_ATTRIBUTE_IFACE				= 159,
	NAN_ATTRIBUTE_CHANNEL				= 160,
	NAN_ATTRIBUTE_PEER_ID				= 161,
	NAN_ATTRIBUTE_NDP_ID				= 162,
	NAN_ATTRIBUTE_SECURITY				= 163,
	NAN_ATTRIBUTE_QOS				= 164,
	NAN_ATTRIBUTE_RSP_CODE				= 165,
	NAN_ATTRIBUTE_INST_COUNT			= 166,
	NAN_ATTRIBUTE_PEER_DISC_MAC_ADDR		= 167,
	NAN_ATTRIBUTE_PEER_NDI_MAC_ADDR			= 168,
	NAN_ATTRIBUTE_IF_ADDR				= 169,
	NAN_ATTRIBUTE_WARMUP_TIME			= 170,
	NAN_ATTRIBUTE_RECV_IND_CFG			= 171,
	NAN_ATTRIBUTE_CONNMAP				= 172,
	NAN_ATTRIBUTE_RSSI_THRESHOLD_FLAG		= 173
} NAN_ATTRIBUTE;

#define C2S(x)  case x: return #x;

#define NAN_BLOOM_LENGTH_DEFAULT        240
#define NAN_SRF_MAX_MAC (NAN_BLOOM_LENGTH_DEFAULT / ETHER_ADDR_LEN)

#ifdef NAN_DP
int wl_cfgnan_data_path_iface_create_delete_handler(struct net_device *ndev,
		struct bcm_cfg80211 *cfg, char *cmd, int size,
		nan_cmd_data_t *cmd_data, uint16 type);
int wl_cfgnan_data_path_request_handler(struct net_device *ndev,
		struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data,
		uint8 *ndp_instance_id);
int wl_cfgnan_data_path_response_handler(struct net_device *ndev,
		struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
int wl_cfgnan_data_path_end_handler(struct net_device *ndev,
		struct bcm_cfg80211 *cfg, char *cmd, int size, nan_cmd_data_t *cmd_data);
#endif /* NAN_DP */
#endif	/* _wl_cfgnan_h_ */
