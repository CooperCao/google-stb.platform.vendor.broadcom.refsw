/*
 * WBD Vendor Message TLV definitions
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wbd_tlv.h 764919 2018-06-11 04:12:34Z pp888947 $
 */

#ifndef _WBD_TLV_H_
#define _WBD_TLV_H_

/* Length of the TLVs */
#define WBD_MIN_LEN_TLV_FBT_CONFIG_REQUEST		1
#define WBD_MIN_LEN_TLV_FBT_CONFIG_RESP			1
#define WBD_MAX_LEN_TLV_ASSOC_STA_METRICS		22
#define WBD_MIN_LEN_TLV_METRIC_POLICY			1
#define WBD_MAX_LEN_TLV_METRIC_POLICY_ITEM		22
#define WBD_MAX_LEN_TLV_WEAK_CLIENT_RESP		17
#define WBD_MAX_LEN_TLV_BSS_CAPABILITY_QUERY		7
#define WBD_MIN_LEN_TLV_BSS_CAPABILITY_REPORT		7
#define WBD_MAX_LEN_TLV_REMOVE_CLIENT_REQ		6
#define WBD_MIN_LEN_TLV_BSS_METRICS_QUERY		1
#define WBD_MIN_LEN_TLV_BSS_METRICS_REPORT		1
#define WBD_LEN_TLV_STEER_RESP_REPORT			13
#define WBD_LEN_TLV_STEER_REQUEST			1

/* Modify wbd_tlv_list_type_name when modifying this */
typedef enum wbd_tlv_types {
	WBD_TLV_FBT_CONFIG_REQ_TYPE			= 1,
	WBD_TLV_FBT_CONFIG_RESP_TYPE			= 2,
	WBD_TLV_VNDR_ASSOC_STA_METRICS_TYPE		= 3,
	WBD_TLV_VNDR_METRIC_POLICY_TYPE			= 4,
	WBD_TLV_WEAK_CLIENT_RESP_TYPE			= 5,
	WBD_TLV_REMOVE_CLIENT_REQ_TYPE			= 6,
	WBD_TLV_BSS_CAPABILITY_QUERY_TYPE		= 7,
	WBD_TLV_BSS_CAPABILITY_REPORT_TYPE		= 8,
	WBD_TLV_BSS_METRICS_QUERY_TYPE			= 9,
	WBD_TLV_BSS_METRICS_REPORT_TYPE			= 10,
	WBD_TLV_STEER_RESP_REPORT_TYPE			= 11,
	WBD_TLV_STEER_REQUEST_TYPE			= 12,
	WBD_TLV_ZWDFS_TYPE				= 13
} wbd_tlv_types_t;

/* Associated STA Link Metrics Capability Flags */
/* STA Supports Dual Band
 * 0: Not supported
 * 1 : Supported
 */
#define WBD_TLV_ASSOC_STA_CAPS_FLAG_DUAL_BAND		0x80

/* Get TLV Type String from TLV Type Enum Value */
extern char const* wbd_tlv_get_type_str(int tlv_type);

/* Get Message String from 1905 Message Type which can support Vendor TLV */
extern char const* wbd_get_1905_msg_str(int msg_type);

#ifdef WLHOSTFBT
/* Encode Vendor Specific TLV for Message : FBT_CONFIG_REQ to send */
extern void wbd_tlv_encode_fbt_config_request(void* data,
	unsigned char* tlv_data, unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : FBT_CONFIG_REQ on receive */
extern int wbd_tlv_decode_fbt_config_request(void* data,
	unsigned char* tlv_data, unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : FBT_CONFIG_RESP to send */
extern void wbd_tlv_encode_fbt_config_response(void* data,
	unsigned char* tlv_data, unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : FBT_CONFIG_RESP on receive */
extern int wbd_tlv_decode_fbt_config_response(void* data,
	unsigned char* tlv_data, unsigned int tlv_data_len);

#endif /* WLHOSTFBT */

/* Encode Vendor Specific TLV : Associated STA Link Metrics Vendor Data to send */
extern void wbd_tlv_encode_vndr_assoc_sta_metrics(void* data,
	unsigned char* tlv_data, unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV : Associated STA Link Metrics Vendor Data on receive */
extern int wbd_tlv_decode_vndr_assoc_sta_metrics(void* data,
	unsigned char* tlv_data, unsigned int tlv_data_len);

/* Encode Vendor Specific TLV : Metric Reporting Policy Vendor Data to send */
extern void wbd_tlv_encode_vndr_metric_policy(void* data,
	unsigned char* tlv_data, unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV : Metric Reporting Policy Vendor Data on receive */
extern int wbd_tlv_decode_vndr_metric_policy(void* data,
	unsigned char* tlv_data, unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : weak client response */
extern int wbd_tlv_encode_weak_client_response(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : weak client response on recieve */
extern int wbd_tlv_decode_weak_client_response(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : BSS capability query */
extern int wbd_tlv_encode_bss_capability_query(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : BSS capability query on recieve */
extern int wbd_tlv_decode_bss_capability_query(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : BSS capability Report */
extern int wbd_tlv_encode_bss_capability_report(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : BSS capability report on recieve */
extern int wbd_tlv_decode_bss_capability_report(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : REMOVE_CLIENT_REQ to send */
extern int wbd_tlv_encode_remove_client_request(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : REMOVE_CLIENT_REQ on recieve */
extern int wbd_tlv_decode_remove_client_request(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);


/* Encode Vendor Specific TLV for Message : BSS metrics query */
extern int wbd_tlv_encode_bss_metrics_query(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : BSS metrics query on recieve */
extern int wbd_tlv_decode_bss_metrics_query(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : BSS metrics Report */
extern int wbd_tlv_encode_bss_metrics_report(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : BSS metrics report on recieve */
extern int wbd_tlv_decode_bss_metrics_report(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : Steer resp report */
int wbd_tlv_encode_steer_resp_report(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : Steer resp report on recieve */
int wbd_tlv_decode_steer_resp_report(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : Vendor Steer Request */
int wbd_tlv_encode_vendor_steer_request(void* data, unsigned char* tlv_data,
	unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : Vendor Steer Request on recieve */
int wbd_tlv_decode_vendor_steer_request(void* data, unsigned char* tlv_data,
	unsigned int tlv_data_len);

/* Encode Vendor Specific TLV for Message : Zero wait DFS */
int wbd_tlv_encode_zwdfs_msg(void* data, unsigned char* tlv_data, unsigned int* tlv_data_len);

/* Decode Vendor Specific TLV for Message : Zero wait DFS */
int wbd_tlv_decode_zwdfs_msg(void* data, unsigned char* tlv_data, unsigned int tlv_data_len);
#endif /* _WBD_TLV_H_ */
