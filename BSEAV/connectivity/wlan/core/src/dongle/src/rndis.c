/*
 * RNDIS common routines
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <osl.h>
#include <proto/ethernet.h>
#include <bcm_ndis.h>
#include <rndis.h>
#include <dngl_dbg.h>
#include <wlioctl.h>
#include <oidencap.h>
#include <wl_wmi.h>
#include <bcmendian.h>
#include <epivers.h>
#include <siutils.h>
#include <bcmnvram.h>
#include <oidencap.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <dngl_protocol.h>
#include <event_log.h>
#include <hnd_rte_ioctl.h>

#define RNDIS_MAX_QUERY_RESPONSE	1024	/* Windows RNDIS limit on USB response size */
#define QUERY_OVERHEAD (RNDIS_MESSAGE_SIZE(RNDIS_QUERY_COMPLETE))
#define GINFO_QUERY_OVERHEAD (GETINFORMATION_SIZE + QUERY_OVERHEAD)

#ifdef CONFIG_USBRNDIS_RETAIL
#define OID_MH_GET_802_1X_SUPPORTED	0xFFEDC100	/* MeetingHouse proprietary licensing OID */
#define REBOOT_DELAY	2000	/* Windows needs a long time to get OID response */
#define OKAY_TO_SEND_DELAY	15
					/*   releases held bulk data IN packets */
#else
#define REBOOT_DELAY	250		/* Embedded platforms need less time */
#endif

typedef struct {
	void *dngl;			/* per-port private data struct */
	void *bus;			/* bus private context struct */
	osl_t *osh;
	bool link;			/* link state. up == TRUE */
	ulong speed;		/* link speed */
	ulong medium;		/* NDIS medium */
	char *name;			/* name of network interface */
	bool pr46794WAR;
#ifdef CONFIG_USBRNDIS_RETAIL
	bool disconnect_needed;	/* need to indicate link down after init */
#ifndef BCMET
	bool holdbulkin;		/* TRUE = hold bulk IN pkts until some ms after link up */
	struct spktq bulkinq;
#endif
#endif /* CONFIG_USBRNDIS_RETAIL */

	/* make sure query_array is 4-byte aligned */
	unsigned long query_array[ROUNDUP(WLC_IOCTL_MAXLEN + GINFO_QUERY_OVERHEAD, 4)/4];
	int qa_remaining;
	unsigned char *qa_loc;
} rnc_info_t;

void * rndis_proto_attach(osl_t *osh, struct dngl *dngl,
                        struct dngl_bus *bus, char *name, bool link_init);
void rndis_proto_detach(void *proto);
void rndis_proto_ctrldispatch(void *proto, void *p, uchar *ext_buf);
void * rndis_proto_pkt_header_push(void *proto, void *p);
int rndis_proto_pkt_header_pull(void *proto, void *p);
void rndis_proto_dev_event(void *proto, void *data);

struct dngl_proto_ops_t rndis_proto_ops = {
	proto_attach_fn:	rndis_proto_attach,
	proto_detach_fn:	rndis_proto_detach,
	proto_ctrldispatch_fn:	rndis_proto_ctrldispatch,
	proto_pkt_header_push_fn: rndis_proto_pkt_header_push,
	proto_pkt_header_pull_fn: rndis_proto_pkt_header_pull,
	proto_dev_event_fn: rndis_proto_dev_event
};

static void rndis_init(rnc_info_t *rnc, void *p);
static void rndis_reset(rnc_info_t *rnc, void *p);
static void rndis_halt(rnc_info_t *rnc, void *p);
static void rndis_keepalive(rnc_info_t *rnc, void *p);
static void rndis_query_oid(rnc_info_t *rnc, void *p);
static void rndis_set_oid(rnc_info_t *rnc, void *p);
static uint32 rndis_setoid(rnc_info_t *rnc, uint32 oid, void *p, uchar *buf, uint len);

#if defined(CONFIG_USBRNDIS_RETAIL) && !defined(BCMET)
static void _rnc_linkdown(dngl_task_t *task);
static void _rndis_okaytosend(dngl_task_t *task);
static void rndis_linkchange(rnc_info_t *rnc, int newlink);
#endif
#ifndef BCMET
static void _rndis_test(dngl_task_t *task);
#endif

static void rnc_link(rnc_info_t *rnc, bool link);
static void rnc_init_resp(rnc_info_t *rnc, void *reqmsg);
static void rnc_reset_resp(rnc_info_t *rnc);
static void rnc_complete_oid(rnc_info_t *rnc, void *reqmsg, uint32 Status, void *buf, uint len);
static void rnc_query_oid(rnc_info_t *rnc, void *p, void *reqmsg);
static void rnc_set_oid(rnc_info_t *rnc, void *p, void *reqmsg);
static void rnc_indicate(rnc_info_t *rnc, uint32 Status, uchar *buf, ulong buf_len);

static unsigned int rndis_supported_oids[] = {
	/* Mandatory OIDs */
	RNDIS_OID_GEN_SUPPORTED_LIST,
	RNDIS_OID_GEN_HARDWARE_STATUS,
	RNDIS_OID_GEN_MEDIA_SUPPORTED,
	RNDIS_OID_GEN_MEDIA_IN_USE,
	RNDIS_OID_GEN_MAXIMUM_FRAME_SIZE,
	RNDIS_OID_GEN_LINK_SPEED,
	RNDIS_OID_GEN_TRANSMIT_BLOCK_SIZE,
	RNDIS_OID_GEN_RECEIVE_BLOCK_SIZE,
	RNDIS_OID_GEN_VENDOR_ID,
	RNDIS_OID_GEN_VENDOR_DESCRIPTION,
	RNDIS_OID_GEN_CURRENT_PACKET_FILTER,
	RNDIS_OID_GEN_MAXIMUM_TOTAL_SIZE,
	RNDIS_OID_GEN_MAC_OPTIONS,
	RNDIS_OID_GEN_MEDIA_CONNECT_STATUS,
	RNDIS_OID_GEN_VENDOR_DRIVER_VERSION,
	RNDIS_OID_GEN_XMIT_OK,
	RNDIS_OID_GEN_RCV_OK,
	RNDIS_OID_GEN_XMIT_ERROR,
	RNDIS_OID_GEN_RCV_ERROR,
	RNDIS_OID_GEN_RCV_NO_BUFFER,
	RNDIS_OID_802_3_PERMANENT_ADDRESS,
	RNDIS_OID_802_3_CURRENT_ADDRESS,
	RNDIS_OID_802_3_MULTICAST_LIST,
	RNDIS_OID_802_3_MAXIMUM_LIST_SIZE,
	RNDIS_OID_802_3_RCV_ERROR_ALIGNMENT,
	RNDIS_OID_802_3_XMIT_ONE_COLLISION,
	RNDIS_OID_802_3_XMIT_MORE_COLLISIONS,
	OID_GEN_SUPPORTED_GUIDS,
	OID_GEN_PHYSICAL_MEDIUM,
#ifdef CONFIG_USBRNDIS_RETAIL
	OID_GEN_RNDIS_CONFIG_PARAMETER,		/* 0x0001021B */
#endif
#ifndef BCMET
	/* Wireless OIDs */
	/* these are listed out of order in ntddndis.h */
	OID_802_11_BSSID,			/* 0x0D010101 */
	OID_802_11_SSID,			/* 0x0D010102 */
	OID_802_11_INFRASTRUCTURE_MODE,		/* 0x0D010108 */
	OID_802_11_ADD_WEP,			/* 0x0D010113 */
	OID_802_11_REMOVE_WEP,			/* 0x0D010114 */
	OID_802_11_DISASSOCIATE,		/* 0x0D010115 */
	OID_802_11_AUTHENTICATION_MODE,		/* 0x0D010118 */
	OID_802_11_PRIVACY_FILTER,		/* 0x0D010119 */
	OID_802_11_BSSID_LIST_SCAN,		/* 0x0D01011A */
	OID_802_11_ENCRYPTION_STATUS,		/* 0x0D01011B */
	OID_802_11_RELOAD_DEFAULTS,		/* 0x0D01011C */
	OID_802_11_ADD_KEY,			/* 0x0D01011D */
	OID_802_11_REMOVE_KEY,			/* 0x0D01011E */
	OID_802_11_ASSOCIATION_INFORMATION,	/* 0x0D01011F */
	OID_802_11_TEST,			/* 0x0D010120 */
	OID_802_11_CAPABILITY,		/* 0x0D010122 */
	OID_802_11_PMKID,			/* 0x0D010123 */
	OID_802_11_NETWORK_TYPES_SUPPORTED,	/* 0x0D010203 */
	OID_802_11_NETWORK_TYPE_IN_USE,		/* 0x0D010204 */
	OID_802_11_TX_POWER_LEVEL,		/* 0x0D010205 */
	OID_802_11_RSSI,			/* 0x0D010206 */
	OID_802_11_RSSI_TRIGGER,		/* 0x0D010207 */
	OID_802_11_FRAGMENTATION_THRESHOLD,	/* 0x0D010209 */
	OID_802_11_RTS_THRESHOLD,		/* 0x0D01020A */
	OID_802_11_NUMBER_OF_ANTENNAS,		/* 0x0D01020B */
	OID_802_11_RX_ANTENNA_SELECTED,		/* 0x0D01020C */
	OID_802_11_TX_ANTENNA_SELECTED,		/* 0x0D01020D */
	OID_802_11_SUPPORTED_RATES,		/* 0x0D01020E */
	OID_802_11_DESIRED_RATES,		/* 0x0D010210 */
	OID_802_11_CONFIGURATION,		/* 0x0D010211 */
	OID_802_11_POWER_MODE,			/* 0x0D010216 */
	OID_802_11_BSSID_LIST,			/* 0x0D010217 */
	OID_802_11_STATISTICS,			/* 0x0D020212 */
#ifdef CONFIG_USBRNDIS_RETAIL
	OID_MH_GET_802_1X_SUPPORTED
#endif
#endif /* !BCMET */
};

#ifdef BCMDBG
static const struct {
	uint oid;
	const char *str;
} oid_map[] = {
		{ RNDIS_OID_GEN_SUPPORTED_LIST, "RNDIS_OID_GEN_SUPPORTED_LIST" },
		{ RNDIS_OID_GEN_HARDWARE_STATUS, "RNDIS_OID_GEN_HARDWARE_STATUS" },
		{ RNDIS_OID_GEN_MEDIA_SUPPORTED, "RNDIS_OID_GEN_MEDIA_SUPPORTED" },
		{ RNDIS_OID_GEN_MEDIA_IN_USE, "RNDIS_OID_GEN_MEDIA_IN_USE" },
		{ RNDIS_OID_GEN_MAXIMUM_FRAME_SIZE, "RNDIS_OID_GEN_MAXIMUM_FRAME_SIZE" },
		{ RNDIS_OID_GEN_LINK_SPEED, "RNDIS_OID_GEN_LINK_SPEED" },
		{ RNDIS_OID_GEN_TRANSMIT_BLOCK_SIZE, "RNDIS_OID_GEN_TRANSMIT_BLOCK_SIZE" },
		{ RNDIS_OID_GEN_RECEIVE_BLOCK_SIZE, "RNDIS_OID_GEN_RECEIVE_BLOCK_SIZE" },
		{ RNDIS_OID_GEN_VENDOR_ID, "RNDIS_OID_GEN_VENDOR_ID" },
		{ RNDIS_OID_GEN_VENDOR_DESCRIPTION, "RNDIS_OID_GEN_VENDOR_DESCRIPTION" },
		{ RNDIS_OID_GEN_CURRENT_PACKET_FILTER, "RNDIS_OID_GEN_CURRENT_PACKET_FILTER" },
		{ RNDIS_OID_GEN_MAXIMUM_TOTAL_SIZE, "RNDIS_OID_GEN_MAXIMUM_TOTAL_SIZE" },
		{ RNDIS_OID_GEN_MAC_OPTIONS, "RNDIS_OID_GEN_MAC_OPTIONS" },
		{ RNDIS_OID_GEN_MEDIA_CONNECT_STATUS, "RNDIS_OID_GEN_MEDIA_CONNECT_STATUS" },
		{ RNDIS_OID_GEN_VENDOR_DRIVER_VERSION, "RNDIS_OID_GEN_VENDOR_DRIVER_VERSION" },
		{ RNDIS_OID_GEN_XMIT_OK, "RNDIS_OID_GEN_XMIT_OK" },
		{ RNDIS_OID_GEN_RCV_OK, "RNDIS_OID_GEN_RCV_OK" },
		{ RNDIS_OID_GEN_XMIT_ERROR, "RNDIS_OID_GEN_XMIT_ERROR" },
		{ RNDIS_OID_GEN_RCV_ERROR, "RNDIS_OID_GEN_RCV_ERROR" },
		{ RNDIS_OID_GEN_RCV_NO_BUFFER, "RNDIS_OID_GEN_RCV_NO_BUFFER" },
		{ RNDIS_OID_802_3_PERMANENT_ADDRESS, "RNDIS_OID_802_3_PERMANENT_ADDRESS" },
		{ RNDIS_OID_802_3_CURRENT_ADDRESS, "RNDIS_OID_802_3_CURRENT_ADDRESS" },
		{ RNDIS_OID_802_3_MULTICAST_LIST, "RNDIS_OID_802_3_MULTICAST_LIST" },
		{ RNDIS_OID_802_3_MAXIMUM_LIST_SIZE, "RNDIS_OID_802_3_MAXIMUM_LIST_SIZE" },
		{ RNDIS_OID_802_3_RCV_ERROR_ALIGNMENT, "RNDIS_OID_802_3_RCV_ERROR_ALIGNMENT" },
		{ RNDIS_OID_802_3_XMIT_ONE_COLLISION, "RNDIS_OID_802_3_XMIT_ONE_COLLISION" },
		{ RNDIS_OID_802_3_XMIT_MORE_COLLISIONS, "RNDIS_OID_802_3_XMIT_MORE_COLLISIONS" },
		{ OID_GEN_SUPPORTED_GUIDS, "OID_GEN_SUPPORTED_GUIDS" },
		{ OID_GEN_PHYSICAL_MEDIUM, "OID_GEN_PHYSICAL_MEDIUM" },
		{ OID_802_11_BSSID, "OID_802_11_BSSID" },
		{ OID_802_11_SSID, "OID_802_11_SSID" },
		{ OID_802_11_NETWORK_TYPES_SUPPORTED, "OID_802_11_NETWORK_TYPES_SUPPORTED" },
		{ OID_802_11_NETWORK_TYPE_IN_USE, "OID_802_11_NETWORK_TYPE_IN_USE" },
		{ OID_802_11_TX_POWER_LEVEL, "OID_802_11_TX_POWER_LEVEL" },
		{ OID_802_11_RSSI, "OID_802_11_RSSI" },
		{ OID_802_11_RSSI_TRIGGER, "OID_802_11_RSSI_TRIGGER" },
		{ OID_802_11_INFRASTRUCTURE_MODE, "OID_802_11_INFRASTRUCTURE_MODE" },
		{ OID_802_11_FRAGMENTATION_THRESHOLD, "OID_802_11_FRAGMENTATION_THRESHOLD" },
		{ OID_802_11_RTS_THRESHOLD, "OID_802_11_RTS_THRESHOLD" },
		{ OID_802_11_NUMBER_OF_ANTENNAS, "OID_802_11_NUMBER_OF_ANTENNAS" },
		{ OID_802_11_RX_ANTENNA_SELECTED, "OID_802_11_RX_ANTENNA_SELECTED" },
		{ OID_802_11_TX_ANTENNA_SELECTED, "OID_802_11_TX_ANTENNA_SELECTED" },
		{ OID_802_11_SUPPORTED_RATES, "OID_802_11_SUPPORTED_RATES" },
		{ OID_802_11_DESIRED_RATES, "OID_802_11_DESIRED_RATES" },
		{ OID_802_11_CONFIGURATION, "OID_802_11_CONFIGURATION" },
		{ OID_802_11_STATISTICS, "OID_802_11_STATISTICS" },
		{ OID_802_11_ADD_WEP, "OID_802_11_ADD_WEP" },
		{ OID_802_11_REMOVE_WEP, "OID_802_11_REMOVE_WEP" },
		{ OID_802_11_ADD_KEY, "OID_802_11_ADD_KEY" },
		{ OID_802_11_REMOVE_KEY, "OID_802_11_REMOVE_KEY" },
		{ OID_802_11_DISASSOCIATE, "OID_802_11_DISASSOCIATE" },
		{ OID_802_11_POWER_MODE, "OID_802_11_POWER_MODE" },
		{ OID_802_11_BSSID_LIST, "OID_802_11_BSSID_LIST" },
		{ OID_802_11_AUTHENTICATION_MODE, "OID_802_11_AUTHENTICATION_MODE" },
		{ OID_802_11_PRIVACY_FILTER, "OID_802_11_PRIVACY_FILTER" },
		{ OID_802_11_BSSID_LIST_SCAN, "OID_802_11_BSSID_LIST_SCAN" },
		{ OID_802_11_ENCRYPTION_STATUS, "OID_802_11_ENCRYPTION_STATUS" },
		{ OID_802_11_ASSOCIATION_INFORMATION, "OID_802_11_ASSOCIATION_INFORMATION" },
		{ OID_802_11_TEST, "OID_802_11_TEST" },
		{ OID_802_11_RELOAD_DEFAULTS, "OID_802_11_RELOAD_DEFAULTS" },
		{ OID_802_11_PMKID, "OID_802_11_PMKID" },
		{ OID_802_11_CAPABILITY, "OID_802_11_CAPABILITY" },
		{ OID_PNP_CAPABILITIES, "OID_PNP_CAPABILITIES" },
		{ OID_PNP_SET_POWER, "OID_PNP_SET_POWER" },
		{ OID_PNP_QUERY_POWER, "OID_PNP_QUERY_POWER" },
		{ OID_PNP_ADD_WAKE_UP_PATTERN, "OID_PNP_ADD_WAKE_UP_PATTERN" },
		{ OID_PNP_REMOVE_WAKE_UP_PATTERN, "OID_PNP_REMOVE_WAKE_UP_PATTERN" },
		{ OID_PNP_ENABLE_WAKE_UP, "OID_PNP_ENABLE_WAKE_UP" },
		{ OID_TCP_TASK_OFFLOAD, "OID_TCP_TASK_OFFLOAD" },
#ifdef CONFIG_USBRNDIS_RETAIL
		{ OID_MH_GET_802_1X_SUPPORTED, "OID_MH_GET_802_1X_SUPPORTED" },
#endif
		{ OID_BCM_SETINFORMATION, "OID_BCM_SETINFORMATION" },
		{ OID_BCM_GETINFORMATION, "OID_BCM_GETINFORMATION" },
		{ WL_OID_BASE + WLC_GET_RADIO, "WLC_GET_RADIO" },
		{ WL_OID_BASE + WLC_SET_RADIO, "WLC_SET_RADIO" },
		{ WL_OID_BASE + WLC_UPGRADE, "WLC_UPGRADE" },
		{ WL_OID_BASE + WLC_UPGRADE_STATUS, "WLC_UPGRADE_STATUS" },
		{ WL_OID_BASE + WLC_NVRAM_SET, "WLC_NVRAM_SET" },
		{ WL_OID_BASE + WLC_NVRAM_DUMP, "WLC_NVRAM_DUMP" },
		{ WL_OID_BASE + WLC_REBOOT, "WLC_REBOOT" },
		{ 0, "UNKNOWN" }
};

static const char *
oid2str(uint oid)
{
	uint i;

	for (i = 0; i < sizeof(oid_map)/sizeof(oid_map[0]) - 1; i++)
		if (oid_map[i].oid == oid)
			break;

	return oid_map[i].str;
}
#endif /* BCMDBG */

static uint
oid2idx(uint oid)
{
	uint i;

	for (i = 0; i < sizeof(rndis_supported_oids)/sizeof(rndis_supported_oids[0]); i++)
		if (rndis_supported_oids[i] == oid)
			break;

	ASSERT(i < sizeof(rndis_supported_oids)/sizeof(rndis_supported_oids[0]));
	return i;
}

static void
rnc_indicate(rnc_info_t *rnc, uint32 Status, uchar *buf, ulong buf_len)
{
	void *p0;
	RNDIS_MESSAGE *msg;
	RNDIS_INDICATE_STATUS *resp;

	trace("REMOTE_NDIS_INDICATE_STATUS");

	/* Generate response */
	if (!(p0 = PKTGET(rnc->osh,
	                  RNDIS_MESSAGE_SIZE(RNDIS_INDICATE_STATUS) + buf_len,
	                  TRUE))) {
		err("out of txbufs");
		ASSERT(0);
		return;
	}

	msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p0);
	msg->NdisMessageType = htol32(REMOTE_NDIS_INDICATE_STATUS_MSG);
	msg->MessageLength = htol32(RNDIS_MESSAGE_SIZE(RNDIS_INDICATE_STATUS) + buf_len);

	resp = (RNDIS_INDICATE_STATUS *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	resp->Status = htol32(Status);
	if (buf_len) {
		resp->StatusBufferLength = htol32(buf_len);
		resp->StatusBufferOffset = htol32(sizeof(RNDIS_INDICATE_STATUS));
		bcopy(buf, (uchar *) (resp + 1), buf_len);
	} else {
		resp->StatusBufferLength = htol32(0);
		resp->StatusBufferOffset = htol32(0);
	}

	bus_ops->sendctl(rnc->bus, p0);
}

static INLINE void
put_unaligned(uint32 val, uint32 *ptr)
{
	((uint8 *)(ptr))[0] = ((uint8 *)(&val))[0];
	((uint8 *)(ptr))[1] = ((uint8 *)(&val))[1];
	((uint8 *)(ptr))[2] = ((uint8 *)(&val))[2];
	((uint8 *)(ptr))[3] = ((uint8 *)(&val))[3];
}

static void
rnc_link(rnc_info_t *rnc, bool link)
{
	ulong Status;

	if (link) {
#ifdef BCMDBG_ERR
		printf("%s: link up\n", rnc->name);
#endif
		Status = RNDIS_STATUS_MEDIA_CONNECT;
	} else {
#ifdef BCMDBG_ERR
		printf("%s: link down\n", rnc->name);
#endif
		Status = RNDIS_STATUS_MEDIA_DISCONNECT;
	}

	rnc_indicate(rnc, Status, NULL, 0);
}

static void
rnc_init_resp(rnc_info_t *rnc, void *reqmsg)
{
	RNDIS_INITIALIZE_REQUEST *req =	(RNDIS_INITIALIZE_REQUEST *)
	    RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(((RNDIS_MESSAGE *) reqmsg));
	RNDIS_MESSAGE *msg;
	RNDIS_INITIALIZE_COMPLETE *resp;
	uint32 Status = RNDIS_STATUS_SUCCESS;
	void *p;

	trace("%s", rnc->name);

	/* Generate response */
	if (!(p = PKTGET(rnc->osh,
	                 RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_COMPLETE),
	                 TRUE))) {
		err("%s: out of memory", rnc->name);
		return;
	}

	msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p);
	msg->NdisMessageType = htol32(REMOTE_NDIS_INITIALIZE_CMPLT);
	msg->MessageLength = htol32(RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_COMPLETE));

	resp = (RNDIS_INITIALIZE_COMPLETE *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	resp->RequestId = req->RequestId;
	resp->Status = htol32(Status);
	resp->MajorVersion = htol32(RNDIS_MAJOR_VERSION);
	resp->MinorVersion = htol32(RNDIS_MINOR_VERSION);
	resp->DeviceFlags = htol32(RNDIS_DF_CONNECTIONLESS);
	resp->Medium = htol32(0);
	resp->MaxPacketsPerMessage = htol32(1);
	resp->MaxTransferSize = htol32(0x4000);
	resp->PacketAlignmentFactor = htol32(2);
	resp->AFListOffset = htol32(0);
	resp->AFListSize = htol32(0);

	bus_ops->sendctl(rnc->bus, p);
}

void
rnc_reset_resp(rnc_info_t *rnc)
{
	RNDIS_MESSAGE *msg;
	RNDIS_RESET_COMPLETE *resp;
	void *p;
	uint32 Status;

	trace("%s", rnc->name);

	/* Generate response */
	if (!(p = PKTGET(rnc->osh,
	                 RNDIS_MESSAGE_SIZE(RNDIS_RESET_COMPLETE),
	                 TRUE))) {
		err("%s: out of memory", rnc->name);
		return;
	}

	Status = RNDIS_STATUS_SUCCESS;

	msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p);
	msg->NdisMessageType = htol32(REMOTE_NDIS_RESET_CMPLT);
	msg->MessageLength = htol32(RNDIS_MESSAGE_SIZE(RNDIS_RESET_COMPLETE));

	resp = (RNDIS_RESET_COMPLETE *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	resp->Status = htol32(Status);
	resp->AddressingReset = htol32(0);

	bus_ops->sendctl(rnc->bus, p);
}

void
rnc_complete_oid(rnc_info_t *rnc, void *reqmsg, uint32 Status, void *buf, uint len)
{
	uint32 NdisMessageType, MessageLength;
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) reqmsg;
	RNDIS_QUERY_REQUEST *req = (RNDIS_QUERY_REQUEST *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	void *p;

	trace("%s", rnc->name);

	/* Do not provide information buffer if OID request failed */
	if (Status != RNDIS_STATUS_SUCCESS) {
		dbg("%s: status 0x%x", rnc->name, Status);
		len = 0;
	}

	if (ltoh32(msg->NdisMessageType) == REMOTE_NDIS_QUERY_MSG) {
		NdisMessageType = REMOTE_NDIS_QUERY_CMPLT;
		/* truncate if necessary - HW does not DMA more than 4K */
		if ((RNDIS_MESSAGE_SIZE(RNDIS_QUERY_COMPLETE) + len) > 4096) {
			MessageLength = 4096;
			len = 4096 - RNDIS_MESSAGE_SIZE(RNDIS_QUERY_COMPLETE);
		} else
			MessageLength = RNDIS_MESSAGE_SIZE(RNDIS_QUERY_COMPLETE) + len;
	} else {
		NdisMessageType = REMOTE_NDIS_SET_CMPLT;
		MessageLength = RNDIS_MESSAGE_SIZE(RNDIS_SET_COMPLETE);
	}

	/* Generate response */
	if (!(p = PKTGET(rnc->osh, MessageLength, TRUE))) {
		err("%s: out of memory for %d bytes", rnc->name, MessageLength);
		return;
	}

	dbg("%s: complete oid for %d bytes", rnc->name, MessageLength);
	msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p);
	msg->NdisMessageType = htol32(NdisMessageType);
	msg->MessageLength = htol32(MessageLength);

	if (NdisMessageType == REMOTE_NDIS_QUERY_CMPLT) {
		RNDIS_QUERY_COMPLETE *q_resp = (RNDIS_QUERY_COMPLETE *)
		    RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);

		q_resp->RequestId = req->RequestId;
		q_resp->Status = htol32(Status);

		if (buf && len) {
			q_resp->InformationBufferLength = htol32(len);
			q_resp->InformationBufferOffset = htol32(sizeof(RNDIS_QUERY_COMPLETE));
			memcpy(&q_resp[1], buf, len);
		} else {
			q_resp->InformationBufferLength = htol32(0);
			q_resp->InformationBufferOffset = htol32(0);
		}
	} else {
		RNDIS_SET_COMPLETE *s_resp = (RNDIS_SET_COMPLETE *)
		    RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);

		s_resp->RequestId = req->RequestId;
		s_resp->Status = htol32(Status);
	}
	bus_ops->sendctl(rnc->bus, p);
}

void
rnc_query_oid(rnc_info_t *rnc, void *p, void *reqmsg)
{
	uint32 Status = RNDIS_STATUS_SUCCESS;
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) reqmsg;
	RNDIS_QUERY_REQUEST *req = (RNDIS_QUERY_REQUEST *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	uint32 oid = ltoh_ua(&req->Oid);
	uint len = ltoh_ua(&req->InformationBufferLength);
	void *buf = (void *)((uint) &req->RequestId + ltoh_ua(&req->InformationBufferOffset));
	ulong ul = 0;
	unsigned char *array = NULL;
	getinformation_t *ginfo = NULL;
	int headerlen = 0;
	uint32 cookie = OIDENCAP_COOKIE;

	trace("%s", rnc->name);

	array = (unsigned char *)rnc->query_array;

	/* find out medium if it hasn't been determined yet */
	if (rnc->medium == 0xFFFFFFFF) {
		int magic;
		if (dngl_dev_ioctl(rnc->dngl, WLC_GET_MAGIC, &magic, sizeof(magic)) >= 0 &&
		    magic == WLC_IOCTL_MAGIC) {
			rnc->medium = NdisPhysicalMediumWirelessLan;
		} else
			rnc->medium = NdisPhysicalMediumUnspecified;
	}

	/* Support encapsulated OID requests */
	if (oid == OID_BCM_GETINFORMATION) {
		/* Check packet */
		if (len < GETINFORMATION_SIZE) {
#ifdef BCMDBG_ERR
			printf("%s: invalid OID_BCM_GETINFORMATION header\n", rnc->name);
#endif
			Status = RNDIS_STATUS_INVALID_LENGTH;
			goto response;
		}

		/* Check cookie */
		ginfo = (getinformation_t *)buf;
		cookie = ltoh_ua(&ginfo->cookie);
		if (cookie != OIDENCAP_COOKIE) {
			if (cookie < (uint32) rnc->query_array ||
				(cookie > (uint32) ((uint32)rnc->query_array +
				sizeof(rnc->query_array))) || !rnc->qa_remaining) {
#ifdef BCMDBG_ERR
				printf("%s: invalid or invalidated partial buffer req 0x%x\n",
				       rnc->name, (uint) cookie);
#endif
				Status = RNDIS_STATUS_INVALID_DATA;
				goto response;
			} else if (cookie != (uint32) rnc->qa_loc) {
				/*
				  avoid race condition by allowing newer req to override older one
				*/
				len = 0;
				cookie = OIDENCAP_COOKIE;
				rnc->qa_remaining = 0;
#ifdef BCMDBG_ERR
				printf("cookie != rnc->qa_loc; punt to avoid race condition\n");
#endif
			} else
				array = rnc->qa_loc - GETINFORMATION_SIZE;
		}

		headerlen = GETINFORMATION_SIZE;
		memcpy(array, ginfo, GETINFORMATION_SIZE);
		array += GETINFORMATION_SIZE;

		if (cookie != OIDENCAP_COOKIE) {
			uint32 tmp;
			getinformation_t *ret_ginfo = NULL;

			ret_ginfo = (getinformation_t *)(array - headerlen);
			if (rnc->qa_remaining + GINFO_QUERY_OVERHEAD > RNDIS_MAX_QUERY_RESPONSE)
				len = RNDIS_MAX_QUERY_RESPONSE - GINFO_QUERY_OVERHEAD;
			else {
				len = rnc->qa_remaining;
				cookie = OIDENCAP_COOKIE;
			}

			rnc->qa_remaining -= len;
			if (rnc->qa_remaining) {
				rnc->qa_loc += len;
				/* store updated offset in cookie field of request */
				cookie = (uint32) rnc->qa_loc;
			} else {
				rnc->qa_loc = (unsigned char *)rnc->query_array;
				cookie = OIDENCAP_COOKIE;
			}
			tmp = htol32(cookie);
			memcpy(&ret_ginfo->cookie, &tmp, 4);
			goto response;
		}
		/* Change OID */
		oid = ltoh_ua(&ginfo->oid);
	}

#ifdef BCMDBG
	trace("%s: 0x%08x: %s: len %d", rnc->name, oid, oid2str(oid), len);
#endif

	/* Default info buf type */
	buf = &ul;
	len = sizeof(ul);

	/* Handle mandatory OIDs */
	switch (oid) {

	case RNDIS_OID_GEN_SUPPORTED_LIST:
		if (rnc->medium == NdisPhysicalMediumUnspecified) {
			/* Mandatory OIDs */
			len = oid2idx(OID_GEN_PHYSICAL_MEDIUM) * sizeof(unsigned int);
			memcpy(array, rndis_supported_oids, len);
			/* Custom OID support */
			*((unsigned int *) &array[len]) = OID_BCM_SETINFORMATION;
			len += sizeof(unsigned int);
			*((unsigned int *) &array[len]) = OID_BCM_GETINFORMATION;
			len += sizeof(unsigned int);
		} else {
			/* Mandatory and wireless OIDs */
			len = sizeof(rndis_supported_oids);
			memcpy(array, rndis_supported_oids, len);
			/* Custom OIDs */
			*((unsigned int *) &array[len]) = OID_BCM_SETINFORMATION;
			len += sizeof(unsigned int);
			*((unsigned int *) &array[len]) = OID_BCM_GETINFORMATION;
			len += sizeof(unsigned int);
			for (ul = 0; ul < WLC_LAST; ul++) {
				*((unsigned int *) &array[len]) = WL_OID_BASE + ul;
				len += sizeof(unsigned int);
			}
		}
		buf = array;
		break;

	case OID_GEN_SUPPORTED_GUIDS:
		bcopy((uint8 *) &GuidList, array, sizeof(GuidList));
		buf = array;
		len = sizeof(GuidList);
		break;

	case RNDIS_OID_GEN_VENDOR_DESCRIPTION: {
		const char *name;
		if (!(name = getvar(NULL, "productname")))
			name = "Broadcom RNDIS Network Adapter";
		memcpy(array, name, len);
		buf = array;
		break;
	}

#ifndef BCMET
	case RNDIS_OID_GEN_HARDWARE_STATUS:
		ul = NdisHardwareStatusReady;
		break;

	case RNDIS_OID_GEN_MEDIA_SUPPORTED:
	case RNDIS_OID_GEN_MEDIA_IN_USE:
		ul = NdisMedium802_3;
		break;

	case RNDIS_OID_GEN_MAXIMUM_FRAME_SIZE:
		ul = dngl_get_netif_mtu(rnc->dngl);
		break;

	case RNDIS_OID_GEN_LINK_SPEED:
		if (rnc->medium == NdisPhysicalMediumWirelessLan) {
			/* Update link speed (500 Kbps to 100 bps) */
			if (dngl_dev_ioctl(rnc->dngl, WLC_GET_RATE, &ul, sizeof(ul)) >= 0)
				rnc->speed = ul * 5000;
			else
				rnc->speed = 0;
		}
		ul = rnc->speed;
		break;

	case RNDIS_OID_GEN_TRANSMIT_BLOCK_SIZE:
	case RNDIS_OID_GEN_RECEIVE_BLOCK_SIZE:
		break;

	case RNDIS_OID_GEN_MAXIMUM_TOTAL_SIZE:
		ul = dngl_get_netif_mtu(rnc->dngl) + ETHER_HDR_LEN;
		break;

	case RNDIS_OID_GEN_MAC_OPTIONS:
		ul = RNDIS_MAC_OPTION_TRANSFERS_NOT_PEND
			| RNDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA
			| RNDIS_MAC_OPTION_RECEIVE_SERIALIZED
			| RNDIS_MAC_OPTION_NO_LOOPBACK;
		break;

	case RNDIS_OID_GEN_MEDIA_CONNECT_STATUS:
		ul = (rnc->link ? NdisMediaStateConnected : NdisMediaStateDisconnected);
		break;

	case RNDIS_OID_GEN_XMIT_OK: {
		dngl_stats_t stats;
		if (dngl_get_netif_stats(rnc->dngl, &stats))
			ul = stats.tx_packets;
		else {
			dngl_get_stats(rnc->dngl, &stats);
			ul = stats.rx_packets;
		}
		break;
	}

	case RNDIS_OID_GEN_RCV_OK: {
		dngl_stats_t stats;

		/* really want to report what is put on usb bus */
		dngl_get_stats(rnc->dngl, &stats);
		ul = stats.tx_packets;
		break;
	}

	case RNDIS_OID_GEN_XMIT_ERROR: {
		dngl_stats_t stats;
		if (dngl_get_netif_stats(rnc->dngl, &stats))
			ul = stats.tx_errors;
		else {
			dngl_get_stats(rnc->dngl, &stats);
			ul = stats.rx_errors;
		}
		break;
	}

	case RNDIS_OID_GEN_RCV_ERROR: {
		dngl_stats_t stats;
		if (dngl_get_netif_stats(rnc->dngl, &stats))
			ul = stats.rx_errors;
		else {
			dngl_get_stats(rnc->dngl, &stats);
			ul = stats.tx_errors;
		}
		break;
	}

	case RNDIS_OID_GEN_RCV_NO_BUFFER: {
		dngl_stats_t stats;

		/* really want to report packets dropped due to no USB buf */
		dngl_get_stats(rnc->dngl, &stats);
		ul = stats.tx_dropped;
		break;
	}

	case RNDIS_OID_802_3_MAXIMUM_LIST_SIZE:
		ul = 32;
		break;

	case RNDIS_OID_802_3_RCV_ERROR_ALIGNMENT:
	case RNDIS_OID_802_3_XMIT_ONE_COLLISION:
	case RNDIS_OID_802_3_XMIT_MORE_COLLISIONS:
		break;

	case RNDIS_OID_GEN_VENDOR_DRIVER_VERSION:
		ul = EPI_VERSION_NUM;
		break;

	case OID_GEN_PHYSICAL_MEDIUM:
		ul = rnc->medium;
		break;
#endif /* !BCMET */

	case OID_BCM_SETINFORMATION:
		dbg("set oid");
		rnc_set_oid(rnc, p, reqmsg);
		return;

	default:
		buf = (void *)((uint) &req->RequestId +
		               ltoh_ua(&req->InformationBufferOffset)) + headerlen;
		len = ltoh_ua(&req->InformationBufferLength) - headerlen;
		switch (oid) {

#ifdef FLASH_UPGRADE
		/* Override some wl custom OIDs */
		case WL_OID_BASE + WLC_UPGRADE_STATUS:
			/* Host is querying upgrade status */
			ul = dngl_upgrade_status(rnc->dngl);

			buf = &ul;
			len = sizeof(ul);
			break;
#endif /* FLASH_UPGRADE */

		/* Pass all other OIDs to wl */
		default:
			/* some query oids pass arguments in the query response buf */
			memcpy(array, buf, len);
			if (ginfo) {
				len = ltoh_ua(&ginfo->len) - headerlen;
				if (len > WLC_IOCTL_MAXLEN)
					len = WLC_IOCTL_MAXLEN;
			} else if (len == 48 || len == 0)
				len = WLC_IOCTL_MAXLEN;

			/* Provide wl_ioctl() with a max sized aligned array for custom OIDs */
			if (oid >= WL_OID_BASE && oid < (WL_OID_BASE + WLC_LAST))
				Status = dngl_dev_query_oid(rnc->dngl, 0, oid,
				                            array, WLC_IOCTL_MAXLEN,
				                            NULL, NULL);
			else
				Status = dngl_dev_query_oid(rnc->dngl,
				                            0,
				                            oid,
				                            array,
				                            len,
				                            (int *)&len,
				                            NULL);

			if ((((oid == OID_802_11_BSSID_LIST &&
			       Status == NDIS_STATUS_INVALID_LENGTH) ||
			      (oid == WL_OID_BASE + WLC_SCAN_RESULTS &&
			       Status != NDIS_STATUS_SUCCESS))) &&
			    len >= WLC_IOCTL_MAXLEN) {
				Status = RNDIS_STATUS_SUCCESS;
			}

#ifdef CONFIG_USBRNDIS_RETAIL
			if (!ginfo && oid == OID_802_11_BSSID_LIST &&
			    Status == RNDIS_STATUS_SUCCESS) {
				int i;
				PNDIS_802_11_BSSID_LIST list = (PNDIS_802_11_BSSID_LIST) array;
				PNDIS_WLAN_BSSID bssid;
				int total_len = QUERY_OVERHEAD;

				bssid = &list->Bssid[0];
				for (i = 0; i < list->NumberOfItems; ++i) {
					if (total_len + bssid->Length > RNDIS_MAX_QUERY_RESPONSE) {
						list->NumberOfItems = i - 1;
						dbg("OID_802_11_BSSID_LIST trimming results"
						    "to %d items (%d bytes)", i - 1, total_len);
						break;
					}
					total_len += bssid->Length;
					bssid = (PNDIS_WLAN_BSSID) ((char *) bssid + bssid->Length);
				}
			}
#endif /* CONFIG_USBRNDIS_RETAIL */
			buf = array;
			break;
		}
	}

	if (ginfo && (len + GINFO_QUERY_OVERHEAD > RNDIS_MAX_QUERY_RESPONSE)) {
		uint32 tmp;
		getinformation_t *ret_ginfo = NULL;
		ret_ginfo = (getinformation_t *)(array - headerlen);
		tmp = RNDIS_MAX_QUERY_RESPONSE - GINFO_QUERY_OVERHEAD;
		rnc->qa_remaining = len - tmp;
		rnc->qa_loc = array + tmp;
		len = tmp;
		/* store updated offset in cookie field of request */
		tmp = (uint32) htol32(rnc->qa_loc);
		memcpy(&ret_ginfo->cookie, &tmp, 4);
	}

response:
	/* Fixup generic unsigned long */
	if (buf == &ul)
		ul = htol32(ul);
	else if (!ginfo && rnc->qa_remaining) {
		/* array was used by std OID; invalidate any in-progress spoon-feeding */
		rnc->qa_remaining = 0;
		dbg("%s: got std RNDIS OID; invalidate in-process spoon-feeding", rnc->name);
	}

	ASSERT(len <= WLC_IOCTL_MAXLEN);

	/* retain encapsulation header in response buf */
	if (ginfo) {
		if (buf == &ul)
			memcpy(array, &ul, sizeof(ul));
		array -= headerlen;
		buf = array;
		len += headerlen;
	}

#ifdef BCMDBG
	if (Status)
		dbg("%s: 0x%08x: %s: status 0x%08x", rnc->name, oid, oid2str(oid), Status);
	else if (buf == &ul)
		dbg("%s: 0x%08x: %s: val 0x%08lx", rnc->name, oid, oid2str(oid), ltoh32(ul));
	else
		dbg("%s: 0x%08x: %s: len %d", rnc->name, oid, oid2str(oid), len);
#endif

	rnc_complete_oid(rnc, msg, Status, buf, len);
}

void
rnc_set_oid(rnc_info_t *rnc, void *p, void *reqmsg)
{
	uint32 Status = RNDIS_STATUS_SUCCESS;
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) reqmsg;
	RNDIS_SET_REQUEST *req = (RNDIS_SET_REQUEST *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	uint32 oid = ltoh_ua(&req->Oid);
	char *buf = (char *)((uint) &req->RequestId + ltoh_ua(&req->InformationBufferOffset));
	uint len = ltoh_ua(&req->InformationBufferLength);
	setinformation_t *sinfo = NULL;

	trace("%s", rnc->name);

	/* Support encapsulated OID requests */
	if (oid == OID_BCM_SETINFORMATION) {
		ulong cookie;

		/* Check packet */
		if (len < SETINFORMATION_SIZE) {
			Status = RNDIS_STATUS_INVALID_LENGTH;
			goto response;
		}
		/* Check cookie */
		sinfo = (setinformation_t *)((uint) &req->RequestId +
		                             ltoh_ua(&req->InformationBufferOffset));
		cookie = ltoh_ua(&sinfo->cookie);
		if (cookie != OIDENCAP_COOKIE) {
			err("%s: bad cookie 0x%x\n", rnc->name, (uint) cookie);
			Status = RNDIS_STATUS_INVALID_DATA;
			goto response;
		}
		/* Change OID */
		oid = ltoh_ua(&sinfo->oid);
		/* Adjust buffer */
		buf += SETINFORMATION_SIZE;
		len -= SETINFORMATION_SIZE;
	}

#ifdef BCMDBG
	trace("%s: 0x%08x: %s: len %d", rnc->name, oid, oid2str(oid), len);
#endif

	switch (oid) {
#ifdef CONFIG_USBRNDIS_RETAIL
	case OID_GEN_RNDIS_CONFIG_PARAMETER: {
		typedef struct {
			uint32 ParameterNameOffset;
			uint32 ParameterNameLength;
			uint32 ParameterType;
			uint32 ParameterValueOffset;
			uint32 ParameterValueLength;
		} rndis_config_param_t;
		rndis_config_param_t *param = (rndis_config_param_t *) buf;
		char *eom = (char *) msg + msg->MessageLength;
		uint32 NameOffset;
		uint32 NameLength;
		uint32 Type;
		uint32 ValueOffset;
		uint32 ValueLength;
		char *namebuf = NULL;
		char *valbuf = NULL;
		uint32 intval = 0xffffffff;
		ndconfig_item_t ndconfig_item;
		NDIS_CONFIGURATION_PARAMETER config_param;

		if (len < sizeof(rndis_config_param_t)) {
			Status = RNDIS_STATUS_INVALID_LENGTH;
			goto response;
		}

		NameOffset = ltoh_ua(&param->ParameterNameOffset);
		NameLength = ltoh_ua(&param->ParameterNameLength);
		Type = ltoh_ua(&param->ParameterType);
		ValueOffset = ltoh_ua(&param->ParameterValueOffset);
		ValueLength = ltoh_ua(&param->ParameterValueLength);

		if ((buf + NameOffset + NameLength > eom) ||
		    (buf + ValueOffset + ValueLength > eom)) {
			Status = RNDIS_STATUS_INVALID_DATA;
			goto response;
		}

		if ((namebuf = MALLOC(rnc->osh, NameLength/sizeof(uint16) + 1)) == NULL) {
			Status = RNDIS_STATUS_RESOURCES;
			goto cleanup;
		}
		if (NameLength) {
			if (((char *) (buf + NameOffset))[1] == '\0')
				/* it's uint16--copy into ascii buffer */
				wchar2ascii(namebuf, (uint16 *) (buf + NameOffset), NameLength,
				            NameLength/sizeof(uint16) + 1);
			else
				strcpy(namebuf, buf + NameOffset);
		} else
			*namebuf = '\0';
		if (Type != NdisParameterInteger) {
			if ((valbuf = MALLOC(rnc->osh, ValueLength/sizeof(uint16) + 1)) == NULL) {
				Status = RNDIS_STATUS_RESOURCES;
				goto cleanup;
			}
			if (ValueLength) {
				if (((char *) (buf + ValueOffset))[1] == '\0')
					/* it's uint16--copy into ascii buffer */
					wchar2ascii(valbuf, (uint16 *) (buf + ValueOffset),
					            ValueLength, ValueLength/sizeof(uint16) + 1);
				else
					strcpy(valbuf, buf + ValueOffset);
			} else
				*valbuf = '\0';
#ifdef BCMDBG_ERR
			printf("got config param %s w/strval %s\n", namebuf, valbuf);
#endif
		} else {
			intval = ltoh_ua(((uint32 *) (buf + ValueOffset)));
#ifdef BCMDBG_ERR
			printf("got config param %s w/intval %d\n", namebuf, intval);
#endif
		}

		/* synthesize a registry entry and add it to the table for lookup */
		bzero(&config_param, sizeof(NDIS_CONFIGURATION_PARAMETER));
		config_param.ParameterType = Type;
		if (Type == NdisParameterInteger)
			config_param.ParameterData.IntegerData = intval;
		else {
			config_param.ParameterData.StringData.Buffer = valbuf;
			config_param.ParameterData.StringData.Length = strlen(valbuf);
			config_param.ParameterData.StringData.MaximumLength = strlen(valbuf) + 1;
		}
		ndconfig_item.param = (void *) &config_param;
		ndconfig_item.name = namebuf;

		Status = dngl_dev_set_oid(rnc->dngl, 0, OID_WL_NDCONFIG_ITEM,
		                          &ndconfig_item,
		                          sizeof(ndconfig_item),
		                          NULL, NULL);

cleanup:
		if (namebuf)
			MFREE(rnc->osh, namebuf, NameLength/sizeof(uint16) + 1);
		if (valbuf)
			MFREE(rnc->osh, valbuf, ValueLength/sizeof(uint16) + 1);
	}
	break;
#endif /* CONFIG_USBRNDIS_RETAIL */

	default:
		/* call back into per-port since set oid handling is port specific */
		Status = rndis_setoid(rnc, oid, p, (void *) buf, len);
		break;
	}

response:
#ifdef BCMDBG
	dbg("%s: 0x%08x: %s: status 0x%08x", rnc->name, oid, oid2str(oid), Status);
#endif

	rnc_complete_oid(rnc, msg, Status, NULL, 0);
}

static void
rndis_init(rnc_info_t *rnc, void *p)
{
	err("rndis_init");

	dngl_init(rnc->dngl);
#ifdef CONFIG_USBRNDIS_RETAIL
	rnc->disconnect_needed = TRUE;
#endif /* CONFIG_USBRNDIS_RETAIL */
	rnc_init_resp(rnc, PKTDATA(rnc->osh, p));

	/* Free request packet */
	PKTFREE(rnc->osh, p, FALSE);
}

static void
rndis_reset(rnc_info_t *rnc, void *p)
{
	err("rndis_reset");

	dngl_reset(rnc->dngl);
	rnc_reset_resp(rnc);

	/* Free request packet */
	PKTFREE(rnc->osh, p, FALSE);
}

static void
rndis_halt(rnc_info_t *rnc, void *p)
{
	err("rndis_halt");

	dngl_halt(rnc->dngl);
	if (p)
		PKTFREE(rnc->osh, p, FALSE);
}

static void
rndis_keepalive(rnc_info_t *rnc, void *p)
{
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p);
	RNDIS_KEEPALIVE_REQUEST *req = (RNDIS_KEEPALIVE_REQUEST *)
	    RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	RNDIS_KEEPALIVE_COMPLETE *resp;
	void *p0;

	trace("%s", rnc->name);

	/* Generate response */
	if (!(p0 = PKTGET(rnc->osh,
	                  RNDIS_MESSAGE_SIZE(RNDIS_KEEPALIVE_COMPLETE),
	                  TRUE))) {
		err("out of txbufs");
		ASSERT(0);
		goto error;
	}

	msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p0);
	msg->NdisMessageType = htol32(REMOTE_NDIS_KEEPALIVE_CMPLT);
	msg->MessageLength = htol32(RNDIS_MESSAGE_SIZE(RNDIS_KEEPALIVE_COMPLETE));

	resp = (RNDIS_KEEPALIVE_COMPLETE *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	resp->RequestId = req->RequestId;
	resp->Status = htol32(RNDIS_STATUS_SUCCESS);

	bus_ops->sendctl(rnc->bus, p0);

error:
	/* Free request packet */
	PKTFREE(rnc->osh, p, FALSE);
}

static void
rndis_query_oid(rnc_info_t *rnc, void *p)
{
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p);
	RNDIS_QUERY_REQUEST *req = (RNDIS_QUERY_REQUEST *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	void *buf = (void *)((uint) &req->RequestId + ltoh_ua(&req->InformationBufferOffset));
	uint len = ltoh_ua(&req->InformationBufferLength);
	uint32 Status = RNDIS_STATUS_SUCCESS;
	uchar *pkttail = (uchar *) PKTDATA(rnc->osh, p) + PKTLEN(rnc->osh, p);

	trace("%s", rnc->name);

	/* Check packet */
	if ((uint) pkttail < (uint) buf) {
		err("%s: bad buffer offset %d\n", rnc->name,
		    ltoh_ua(&req->InformationBufferOffset));
		Status = RNDIS_STATUS_INVALID_PACKET;
		goto end;
	}
	if ((uint) pkttail < ((uint) buf + len)) {
		err("%s: bad buffer length %d\n", rnc->name, len);
		Status = RNDIS_STATUS_INVALID_PACKET;
		goto end;
	}

end:
	if (Status == RNDIS_STATUS_SUCCESS)
		rnc_query_oid(rnc, p, msg);
	else {
		err("oid 0x%x failed: invalid data", ltoh_ua(&req->Oid));
		rnc_complete_oid(rnc, PKTDATA(rnc->osh, p), Status, NULL, 0);
	}
	/* Free request packet */
	PKTFREE(rnc->osh, p, FALSE);
}

static void
rndis_set_oid(rnc_info_t *rnc, void *p)
{
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p);
	RNDIS_SET_REQUEST *req = (RNDIS_SET_REQUEST *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	void *buf = (void *)((uint) &req->RequestId + ltoh_ua(&req->InformationBufferOffset));
	uint len = ltoh_ua(&req->InformationBufferLength);
	uint32 Status = RNDIS_STATUS_SUCCESS;
	uchar *pkttail = (uchar *) PKTDATA(rnc->osh, p) + PKTLEN(rnc->osh, p);

	trace("%s", rnc->name);

	/* Check packet */
	if ((uint) pkttail < (uint) buf) {
		err("%s: bad buffer offset %d\n", rnc->name,
		    ltoh_ua(&req->InformationBufferOffset));
		Status = RNDIS_STATUS_INVALID_PACKET;
		goto end;
	}
	if ((uint) pkttail < ((uint) buf + len)) {
		err("%s: bad buffer length %d\n", rnc->name, len);
		Status = RNDIS_STATUS_INVALID_PACKET;
		goto end;
	}

end:
	if (Status == RNDIS_STATUS_SUCCESS)
		rnc_set_oid(rnc, p, msg);
	else {
		err("oid 0x%x failed: invalid data", ltoh_ua(&req->Oid));
		rnc_complete_oid(rnc, p, Status, NULL, 0);
	}

	/* Free request packet */
	PKTFREE(rnc->osh, p, FALSE);
}

static uint32
rndis_setoid(rnc_info_t *rnc, uint32 oid, void *p, uchar *buf, uint len)
{
	ulong ul;
	uint32 Status = RNDIS_STATUS_SUCCESS;
#ifndef BCMET
	uchar *pkttail = (uchar *) PKTDATA(rnc->osh, p) + PKTLEN(rnc->osh, p);
#endif

	trace("%s", rnc->name);

	switch (oid) {
	case RNDIS_OID_GEN_CURRENT_PACKET_FILTER: {
		int val;

		if (len < sizeof(ul)) {
			Status = RNDIS_STATUS_INVALID_LENGTH;
			break;
		}
		ul = ltoh_ua((uint32 *) buf);

		/* RNDIS doc: Windows is ready to use us when it sets a non-zero filter */
		if (ul) {
			dngl_opendev(rnc->dngl);
#if defined(CONFIG_USBRNDIS_RETAIL) && !defined(BCMET)
			/* WHQL: link down after being configured */
			if (rnc->disconnect_needed) {
				rnc->disconnect_needed = FALSE;
				dngl_schedule_work((void *) rnc, NULL, _rnc_linkdown, 300);
			}
#endif
		}

		if (ul & (NDIS_PACKET_TYPE_SOURCE_ROUTING |
		          NDIS_PACKET_TYPE_SMT |
		          NDIS_PACKET_TYPE_MAC_FRAME |
		          NDIS_PACKET_TYPE_FUNCTIONAL |
		          NDIS_PACKET_TYPE_ALL_FUNCTIONAL |
		          NDIS_PACKET_TYPE_GROUP)) {
			Status = RNDIS_STATUS_NOT_SUPPORTED;
			break;
		}

		/* Set promiscuity */
		val = (ul & NDIS_PACKET_TYPE_PROMISCUOUS) != 0;
		dngl_dev_ioctl(rnc->dngl, RTESPROMISC, &val, sizeof(val));

		/* Set allmulti */
		val = (ul & NDIS_PACKET_TYPE_ALL_MULTICAST) != 0;
		dngl_dev_ioctl(rnc->dngl, RTESALLMULTI, &val, sizeof(val));

		break;
	}

	case RNDIS_OID_802_3_CURRENT_ADDRESS: {
		int ret;

		/* Note: setting the MAC this way is not supported by traditional NDIS */
		if (len < ETHER_ADDR_LEN) {
			Status = RNDIS_STATUS_INVALID_LENGTH;
			break;
		}
		ret = dngl_dev_ioctl(rnc->dngl, RTESHWADDR, buf, len);
		if (ret) {
#ifdef BCMDBG_ERR
			printf("%s: error %d setting MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
			       rnc->name, ret, ((unsigned char *) buf)[0],
			       ((unsigned char *) buf)[1], ((unsigned char *) buf)[2],
			       ((unsigned char *) buf)[3], ((unsigned char *) buf)[4],
			       ((unsigned char *) buf)[5]);
#endif
			Status = RNDIS_STATUS_INVALID_DATA;
		} else {
#ifdef BCMDBG_ERR
			printf("%s: set new MAC %02x:%02x:%02x:%02x:%02x:%02x\n", rnc->name,
			       ((unsigned char *) buf)[0], ((unsigned char *) buf)[1],
			       ((unsigned char *) buf)[2], ((unsigned char *) buf)[3],
			       ((unsigned char *) buf)[4], ((unsigned char *) buf)[5]);
#endif
		}
		break;
	}

	case RNDIS_OID_802_3_MULTICAST_LIST:
		if (((len % ETHER_ADDR_LEN) != 0) ||
		    (len > (MAXMULTILIST * ETHER_ADDR_LEN))) {
			Status = RNDIS_STATUS_INVALID_LENGTH;
			break;
		}
		Status = dngl_dev_ioctl(rnc->dngl, RTESMULTILIST, buf, len);
		break;

#ifndef BCMET
	case OID_802_11_TEST: {
		NDIS_802_11_TEST *test = (NDIS_802_11_TEST *) buf;
		void *resp;
		ulong type;

		if (len < sizeof(NDIS_802_11_TEST)) {
			Status = RNDIS_STATUS_INVALID_LENGTH;
			break;
		}

		type = ltoh_ua(&test->Type);
		if (type == 1) {
			buf = (uchar *) &test->AuthenticationEvent;
			len = ltoh_ua(&test->Length) -
				OFFSETOF(NDIS_802_11_TEST, AuthenticationEvent);
		} else if (type == 2) {
			buf = (uchar *) &test->RssiTrigger;
			len = sizeof(NDIS_802_11_RSSI);
		}

		if ((uint) pkttail < ((uint) buf + len)) {
			err("%s: bad test length %d", rnc->name, len);
			Status = RNDIS_STATUS_INVALID_PACKET;
			break;
		}

		/* Clone and trim packet */
		if (!(resp = PKTDUP(rnc->osh, p))) {
			err("%s: PKTDUP failed", rnc->name);
			Status = RNDIS_STATUS_RESOURCES;
			break;
		}
		PKTPULL(rnc->osh, resp, (uint) buf - (uint) PKTDATA(rnc->osh, p));
		PKTSETLEN(rnc->osh, resp, len);

		/* Defer test response */
		Status = dngl_schedule_work((void *) rnc, resp, _rndis_test, 1);
		if (Status == BCME_NORESOURCE)
			PKTFREE(rnc->osh, p, TRUE);

		break;
	}
#endif /* !BCMET */

	default:
		/* Handle 802.11 and wl custom ioctls */
		switch (oid) {

		case WL_OID_BASE + WLC_REBOOT:
			Status = dngl_schedule_work(rnc->dngl, NULL, _dngl_reboot, REBOOT_DELAY);
			break;

#ifndef CONFIG_USBRNDIS_RETAIL
		case WL_OID_BASE + WLC_KEEPALIVE:
			ul = ltoh_ua((uint32 *) buf);
			dngl_keepalive(rnc->dngl, ul);
			break;
#endif

#ifdef FLASH_UPGRADE
		case WL_OID_BASE + WLC_UPGRADE:
			Status = dngl_upgrade(rnc->dngl, buf, len);
			break;
#endif

#ifdef BCMDBG
		case WL_OID_BASE + WLC_SET_MSGLEVEL:
			dngl_msglevel = ltoh_ua((uint32 *) buf);
			/* Fall through to set wl msglevel */
#endif

		default:
#ifdef CONFIG_USBRNDIS_RETAIL
			/* if we get a wireless oid, make sure the interface is up */
			if (oid >= OID_802_11_BSSID && oid < WL_OID_BASE)
				dngl_opendev(rnc->dngl);
#endif
			Status = dngl_dev_set_oid(rnc->dngl, 0, oid, buf, len, NULL, NULL);
			break;
		}
	}

	return Status;
}

#if defined(CONFIG_USBRNDIS_RETAIL) && !defined(BCMET)
static void
_rnc_linkdown(dngl_task_t *task)
{
	rnc_info_t *rnc = (rnc_info_t *) hnd_timer_get_ctx(task);

	trace("%s", rnc->name);
	rnc_link(rnc, FALSE);
}

static void
_rndis_okaytosend(dngl_task_t *task)
{
	rnc_info_t *rnc = (rnc_info_t *) hnd_timer_get_ctx(task);
	void *p;

	trace("%s", rnc->name);
	rnc->holdbulkin = FALSE;

	/* transmit any held bulk IN pkts */
	while ((p = spktdeq(&rnc->bulkinq)))
		dngl_sendpkt(rnc->dngl, NULL, p);
}

static void
rndis_linkchange(rnc_info_t *rnc, int newlink)
{
	if (newlink == TRUE) {
		rnc->holdbulkin = TRUE;
		dngl_schedule_work((void *) rnc, NULL, _rndis_okaytosend, OKAY_TO_SEND_DELAY);
	} else
		rnc->holdbulkin = FALSE;
}
#endif /* CONFIG_USBRNDIS_RETAIL && !BCMET */

#ifndef BCMET
static void
_rndis_test(dngl_task_t *task)
{
	rnc_info_t *rnc = (rnc_info_t *) hnd_timer_get_ctx(task);
	void *data = hnd_timer_get_data(task);
	uchar *buf = PKTDATA(rnc->osh, data);
	ulong len = PKTLEN(rnc->osh, data);
	uint32 Status = RNDIS_STATUS_MEDIA_SPECIFIC_INDICATION;

	trace("%s", rnc->name);
	/* Respond to OID_802_11_TEST */
	rnc_indicate(rnc, Status, buf, len);

	/* Free request packet */
	PKTFREE(rnc->osh, data, FALSE);
}
#endif /* !BCMET */

void *
BCMATTACHFN(rndis_proto_attach)(osl_t *osh, struct dngl *dngl,
                        struct dngl_bus *bus, char *name, bool link_init)
{
	rnc_info_t *rnc;
#ifndef BCMET
	uint32 iovbuf[7];	/* Room for "event_msgs" + '\0' + bitvec */
	char bitvec[WL_EVENTING_MASK_LEN];
#endif

	trace("called");

	if ((rnc = MALLOC(osh, sizeof(rnc_info_t))) == NULL)
		return NULL;

	memset(rnc, 0, sizeof(rnc_info_t));
	rnc->dngl = dngl;
	rnc->bus = bus;
	rnc->osh = osh;
	rnc->link = link_init;
	rnc->speed = 0;
	rnc->medium = 0xFFFFFFFF;
	rnc->name = name;
	rnc->qa_remaining = 0;
	rnc->qa_loc = (unsigned char *)rnc->query_array;

#ifndef BCMET
	bzero(bitvec, sizeof(bitvec));
	setbit(bitvec, WLC_E_MIC_ERROR);
	setbit(bitvec, WLC_E_NDIS_LINK);
	setbit(bitvec, WLC_E_PMKID_CACHE);
	/* by default want to see events MIC_ERROR, NDIS_LINK and PMKID_CACHE */
	bcm_mkiovar("event_msgs", bitvec, sizeof(bitvec), (char *)&iovbuf, sizeof(iovbuf));
	dngl_dev_ioctl(rnc->dngl, WLC_SET_VAR, iovbuf, sizeof(iovbuf));
#ifdef CONFIG_USBRNDIS_RETAIL
	spktqinit(&rnc->bulkinq, 32);
#endif
#endif /* !BCMET */

	return (void *) rnc;
}

void
BCMATTACHFN(rndis_proto_detach)(void *proto)
{
	rnc_info_t *rnc = (rnc_info_t *) proto;

	trace("%s", rnc->name);
	MFREE(rnc->osh, rnc, sizeof(rnc_info_t));
}

/* dispatcher for all control messages received from the host */
void
rndis_proto_ctrldispatch(void *proto, void *p, uchar *ext_buf)
{
	rnc_info_t *rnc = (rnc_info_t *) proto;
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) PKTDATA(rnc->osh, p);
	uint32 NdisMessageType, MessageLength;
	int pktlen = PKTLEN(rnc->osh, p);

	trace("called");

#ifdef EVENT_LOG_COMPILE
	event_log_time_sync();
#endif

	if (pktlen < 8) {
		err("bad packet length %d", pktlen);
#ifdef BCMDBG
		prpkt("bad pkt len", rnc->osh, p);
#endif
		goto error;
	}

	NdisMessageType = ltoh32(msg->NdisMessageType);
	MessageLength = ltoh32(msg->MessageLength);

	if (pktlen < MessageLength) {
		err("bad message length %d; pktlen %d", MessageLength, pktlen);
#ifdef BCMDBG
		prpkt("bad message len", rnc->osh, p);
#endif
		if (rnc->pr46794WAR) {
			pktlen = MessageLength;
			PKTSETLEN(rnc->osh, p, MessageLength);
			bus_ops->pr46794WAR(rnc->bus);
			rnc->pr46794WAR = FALSE;
			err("proto_pr46794WAR OFF");
		} else
			goto error;
	}

	switch (NdisMessageType) {

	case REMOTE_NDIS_INITIALIZE_MSG:
		if (MessageLength < RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_REQUEST)) {
			err("bad message length %d", MessageLength);
			goto error;
		}
		dbg("REMOTE_NDIS_INITIALIZE_MSG");
		rndis_init(rnc, p);
		break;

	case REMOTE_NDIS_HALT_MSG:
		if (MessageLength < RNDIS_MESSAGE_SIZE(RNDIS_HALT_REQUEST)) {
			err("bad message length %d", MessageLength);
			goto error;
		}
		dbg("REMOTE_NDIS_HALT_MSG");
		rndis_halt(rnc, p);
		break;

	case REMOTE_NDIS_RESET_MSG:
		if (MessageLength < RNDIS_MESSAGE_SIZE(RNDIS_RESET_REQUEST)) {
			err("bad message length %d", MessageLength);
			goto error;
		}
		dbg("REMOTE_NDIS_RESET_MSG");
		rndis_reset(rnc, p);
		break;

	case REMOTE_NDIS_QUERY_MSG:
		if (MessageLength < RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST)) {
			err("bad message length %d", MessageLength);
			goto error;
		}
		rndis_query_oid(rnc, p);
		break;

	case REMOTE_NDIS_SET_MSG:
		if (MessageLength < RNDIS_MESSAGE_SIZE(RNDIS_SET_REQUEST)) {
			err("bad message length %d", MessageLength);
			goto error;
		}
		rndis_set_oid(rnc, p);
		break;

	case REMOTE_NDIS_KEEPALIVE_MSG: {
		if (MessageLength != RNDIS_MESSAGE_SIZE(RNDIS_KEEPALIVE_REQUEST)) {
			err("bad message length %d", MessageLength);
			goto error;
		}
		dbg("REMOTE_NDIS_KEEPALIVE_MSG");
		rndis_keepalive(rnc, p);
		break;
	}

	default:
		err("unhandled message type 0x%x", NdisMessageType);
		goto error;
	}

	return;

error:
	PKTFREE(rnc->osh, p, FALSE);
	rnc_indicate(rnc, RNDIS_STATUS_INVALID_DATA, NULL, 0);
}

/* Push RNDIS header onto a data packet for xmt on the bus */
void *
rndis_proto_pkt_header_push(void *proto, void *p)
{
	rnc_info_t *rnc = (rnc_info_t *) proto;
	RNDIS_MESSAGE *msg;
	RNDIS_PACKET *pkt;
	osl_t *osh;
	int pktlen;

	osh = rnc->osh;

#if defined(CONFIG_USBRNDIS_RETAIL) && !defined(BCMET)
	/* redirect data path event indications to the control channel */
	if (ntoh16(((struct ether_header *) PKTDATA(osh, p))->ether_type) == ETHER_TYPE_BRCM) {
		rndis_proto_dev_event(proto, PKTDATA(rnc->osh, p));
		PKTFREE(osh, p, TRUE);
		return NULL;
	}
	if (rnc->holdbulkin && !spktq_full(&rnc->bulkinq)) {
		spktenq(&rnc->bulkinq, p);
		return NULL;
	}
	/* fall through */
#endif

	pktlen = PKTLEN(osh, p);
	if ((uint)PKTHEADROOM(osh, p) < RNDIS_MESSAGE_SIZE(RNDIS_PACKET)) {
		void *p1;

		/* alloc a packet that will fit all the data; chaining the header won't work */
		if ((p1 = PKTGET(osh, pktlen + RNDIS_MESSAGE_SIZE(RNDIS_PACKET), TRUE)) == NULL) {
			err("PKTGET pkt size %d + headroom %d failed\n", pktlen,
			    RNDIS_MESSAGE_SIZE(RNDIS_PACKET));
			PKTFREE(osh, p, TRUE);
			return NULL;
		}

		/* Transfer priority */
		PKTSETPRIO(p1, PKTPRIO(p));

		bcopy(PKTDATA(osh, p), PKTDATA(osh, p1) + RNDIS_MESSAGE_SIZE(RNDIS_PACKET), pktlen);
		PKTFREE(osh, p, TRUE);

		p = p1;
	} else
		/* Push RNDIS header */
		PKTPUSH(osh, p, RNDIS_MESSAGE_SIZE(RNDIS_PACKET));
	pktlen += RNDIS_MESSAGE_SIZE(RNDIS_PACKET);

	bzero(PKTDATA(osh, p), RNDIS_MESSAGE_SIZE(RNDIS_PACKET));

	msg = (RNDIS_MESSAGE *) PKTDATA(osh, p);
	pkt = (RNDIS_PACKET *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);

	/* Data may be unaligned */
	put_unaligned(htol32(REMOTE_NDIS_PACKET_MSG), &msg->NdisMessageType);
	put_unaligned(htol32(pktlen), &msg->MessageLength);
	put_unaligned(htol32(sizeof(RNDIS_PACKET)), &pkt->DataOffset);
	put_unaligned(htol32(pktlen - RNDIS_MESSAGE_SIZE(RNDIS_PACKET)), &pkt->DataLength);

	return p;
}

/* Packet received from the bus; pull its header for xmt up the stack */
int
rndis_proto_pkt_header_pull(void *proto, void *p)
{
	rnc_info_t *rnc = (rnc_info_t *) proto;
	osl_t *osh = rnc->osh;
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) PKTDATA(osh, p);
	RNDIS_PACKET *pkt = (RNDIS_PACKET *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	uint32 NdisMessageType, MessageLength, DataOffset, DataLength;

	trace("called");

	osh = rnc->osh;

	/* Check packet */
	if (PKTLEN(osh, p) < RNDIS_MESSAGE_SIZE(RNDIS_PACKET)) {
		err("bad packet length %d", PKTLEN(rnc->osh, p));
		goto drop;
	}
	ASSERT(ISALIGNED(msg, 4));
	NdisMessageType = ltoh32(msg->NdisMessageType);
	MessageLength = ltoh32(msg->MessageLength);
	DataOffset = ltoh32(pkt->DataOffset);
	DataLength = ltoh32(pkt->DataLength);
	if (NdisMessageType != REMOTE_NDIS_PACKET_MSG) {
		err("unknown message type 0x%08x", NdisMessageType);
		goto drop;
	}
	if (MessageLength > PKTLEN(osh, p)) {
		err("bad message length %d, type 0x%08x, pktlen %d",
		    MessageLength, NdisMessageType, PKTLEN(rnc->osh, p));
		goto drop;
	}

	/* Set a default packet priority (until pulled from message) */
	PKTSETPRIO(p, 0);

	/* Pull RNDIS header */
	PKTSETLEN(osh, p, MessageLength);
	if (((uint) &pkt->DataOffset - (uint) msg + DataOffset) > PKTLEN(rnc->osh, p)) {
		err("bad data offset %d", DataOffset);
		goto drop;
	}
	PKTPULL(osh, p, ((uint) &pkt->DataOffset - (uint) msg) + DataOffset);

	return 0;

drop:
	PKTFREE(osh, p, FALSE);
	return 1;
}


#ifdef BCMET
void
rndis_proto_dev_event(void *proto, void *data)
{
	rnc_info_t *rnc = (rnc_info_t *) proto;
	int link = *((int *) data);

	rnc_link(rnc, (bool) link);
}
#else
static int
wl_pmkid_cache_ind(pmkid_cand_list_t *p, void *b)
{
	return 0;
}
void
rndis_proto_dev_event(void *proto, void *data)
{
	rnc_info_t *rnc = (rnc_info_t *) proto;
	bcm_event_t *pkt_data = (bcm_event_t *) data;
	uchar *buf = NULL;
	ulong Status = RNDIS_STATUS_SUCCESS;
	uint len = 0;
	uint msg, datalen;
	uint16 flags;
	struct ether_addr *addr;

	if (bcmp(BRCM_OUI, &pkt_data->bcm_hdr.oui[0], DOT11_OUI_LEN)) {
#ifdef BCMDBG_ERR
		printf("%s: proto_dev_event: Not BRCM_OUI\n", rnc->name);
#endif
		return;
	}

	if (ntoh16(pkt_data->bcm_hdr.usr_subtype) != BCMILCP_BCM_SUBTYPE_EVENT) {
#ifdef BCMDBG_ERR
		printf("%s: proto_dev_event: Not BCMILCP_BCM_SUBTYPE_MSG\n", rnc->name);
#endif
		return;
	}

	msg = ntoh32(pkt_data->event.event_type);
	addr = (struct ether_addr *)&(pkt_data->event.addr);
	datalen = ntoh32(pkt_data->event.datalen);
	flags = ntoh16(pkt_data->event.flags);

	trace("%s: event: %d", rnc->name, msg);

	switch (msg) {

	case WLC_E_MIC_ERROR: {
		struct {
			NDIS_802_11_STATUS_INDICATION Status;
			NDIS_802_11_AUTHENTICATION_REQUEST Request[1];
		} AuthenticationEvent;

		Status = RNDIS_STATUS_MEDIA_SPECIFIC_INDICATION;

		AuthenticationEvent.Status.StatusType =
			Ndis802_11StatusType_Authentication;
		bcopy(addr, &AuthenticationEvent.Request[0].Bssid, ETHER_ADDR_LEN);

		if (flags & WLC_EVENT_MSG_GROUP)
			AuthenticationEvent.Request[0].Flags =
				NDIS_802_11_AUTH_REQUEST_GROUP_ERROR;
		else
			AuthenticationEvent.Request[0].Flags =
				NDIS_802_11_AUTH_REQUEST_PAIRWISE_ERROR;
		AuthenticationEvent.Request[0].Length =
			sizeof(AuthenticationEvent.Request[0]);

		buf = (uchar *) &AuthenticationEvent;
		len = sizeof(AuthenticationEvent);
		break;
	}

	case WLC_E_NDIS_LINK: {
		bool link = ((flags & WLC_EVENT_MSG_LINK) == WLC_EVENT_MSG_LINK);
#ifdef CONFIG_USBRNDIS_RETAIL
		if (rnc->link != link)
			rndis_linkchange(rnc, link);
#endif
		rnc->link = link;
		rnc_link(rnc, link);
		return;
	}

	case WLC_E_PMKID_CACHE: {
		pmkid_cand_list_t* pmkid_list = (pmkid_cand_list_t*)(pkt_data + 1);
		struct {
			NDIS_802_11_STATUS_INDICATION Status;
			NDIS_802_11_PMKID_CANDIDATE_LIST pmkid_list;
			PMKID_CANDIDATE foo[MAXPMKID - 1];
		} pmkidreq;
		buf = (uchar*)&pmkidreq;

		if (datalen && pmkid_list->npmkid_cand) {
			/* fill in candidate buffer */
			len = wl_pmkid_cache_ind(pmkid_list, buf);
			Status = RNDIS_STATUS_MEDIA_SPECIFIC_INDICATION;
			break;
		}
		return;
	}

	default:
		/* Generic MAC event */
		len = sizeof(bcm_event_t) + datalen;
		buf = (uchar *) pkt_data;
		Status = BCM_MAC_STATUS_INDICATION;
		break;
	}

	rnc_indicate(rnc, Status, buf, len);
}
#endif /* BCMET */

void
proto_pr46794WAR(struct dngl *dngl)
{
	rnc_info_t *rnc = (rnc_info_t *) dngl_proto(dngl);
	err("ON");
	rnc->pr46794WAR = TRUE;
}
