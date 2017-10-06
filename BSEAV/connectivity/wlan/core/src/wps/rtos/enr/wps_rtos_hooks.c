/*
 * Broadcom WPS Enrollee platform dependent hook function
 *
 * This file is the linux specific implementation of the OS hooks
 * necessary for implementing the wps_enr.c reference application
 * for WPS enrollee code. It is mainly the implementation of eap transport
 * but also add basic OS layer interface (should it be renamed like linux_osl ??)
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

#include "typedefs.h"
#include "osl_ext.h"

#include "wl_drv.h"
#include <tutrace.h>
#include <wpserror.h>
#include <wlioctl.h>
#include <portability.h>
#include <wps_staeapsm.h>
#include <wps_enrapi.h>
#include <wps_enr_osl.h>
#include <ethernet.h>
#include <eapol.h>
#include <bcmendian.h>

/* Queue should be able to store atleast 3 handshake packets */
#define	WPS_MAX_QUEUE_SZ		12

typedef struct wps_pktInfo
{
	uint32	in_Size;
	wl_drv_netif_pkt in_Data;
} wps_pktInfo_t;

typedef struct wps_xsup
{
	uint8	eapol_bssid[ETHER_ADDR_LEN];  /* MAC address of authenticator */
	uint8	source_mac[ETHER_ADDR_LEN];
	/* incoming packet queue */
	osl_ext_queue_t	in_Queue;
} wps_xsup_t;

static unsigned int g_restore_ifidx = 0;
static unsigned int g_ifidx = 0;
wps_xsup_t	g_WpsParam;
static int		g_EapOsInitialised;

typedef struct ether_header  ETH_HEADER;

#ifdef _TUDEBUGTRACE
void print_buf(unsigned char *buff, int buflen);
#endif

static uint32 Eap_OSInit(uint8 *bssid);
static uint32 Eap_OSDeInit(void);
static uint32 Eap_SetBssid(uint8 *bssid);
static uint32 Eap_ReadData(char * dataBuffer, uint32 * dataLen, uint32 timeout, bool raw);
static uint32 Eap_SendDataDown(char * dataBuffer, uint32 dataLen, bool raw);
extern void RAND_generic_init(void);

int wps_wl_ioctl(int cmd, void *buf, int len, bool set);

int
wps_osl_get_mac(uint8 *mac)
{
	int ret = -1;
	char buf[128];

	/* Get the device MAC address */
	strcpy(buf, "cur_etheraddr");

	/* Copy the result back */
	if (!(ret = wps_wl_ioctl(WLC_GET_VAR, buf, sizeof(buf), FALSE)))
		memcpy(mac, buf, 6);

	return ret;
}

static int
is_WpsPkt(wl_drv_netif_pkt pkt)
{
	int status = 0;
	ETH_HEADER	*hdr = (ETH_HEADER *)pkt;
	uint16	type;

	/* check dest mac addr */
	if (memcmp(hdr->ether_dhost, g_WpsParam.source_mac, ETHER_ADDR_LEN) == 0) {
		type = WpsNtohs((uchar *)&hdr->ether_type);
		/* check ethter_type */
		if (type == ETHER_TYPE_802_1X)
			status = 1;
	}

	return status;
}

void
wps_ifidx(unsigned int ifidx)
{
	g_restore_ifidx = g_ifidx;
	g_ifidx = ifidx;
}

void
wps_restore_ifidx(void)
{
	g_ifidx = g_restore_ifidx;
}

int
wps_osl_init(char *bssid)
{
	RAND_generic_init();
	return Eap_OSInit((uint8 *)bssid);
}

void
wps_osl_deinit()
{
	Eap_OSDeInit();
}

int
wps_rx_callback(wl_drv_netif_pkt pkt, unsigned int len)
{
	wps_pktInfo_t   *data = NULL;
	osl_ext_status_t    status;

	if (!g_EapOsInitialised) /* If WPS not initialized dont claim the packet */
		return (1);

	/* Claim the packet after EAP type checks */
	if (is_WpsPkt(pkt)) {

		data = malloc(sizeof(wps_pktInfo_t));

		if (!data)
			return 1;

		/* Copy the packet to databuf and data len */
		data->in_Size = len;
		data->in_Data = pkt;

		/* Insert the Packet to the queue */
		status = osl_ext_queue_send(&g_WpsParam.in_Queue, (void *)data);

		if (status != OSL_EXT_SUCCESS)
			TUTRACE((TUTRACE_ERR, "%s: Queue Send has failed :%u \n",
			                                          __FUNCTION__, status));

		return 0;
	}

	return 1;
}

uint32
Eap_OSInit(uint8 *bssid)
{
	wl_drv_netif_callbacks_t cbs;
	uint32 status = WPS_SUCCESS;

	if (!g_EapOsInitialised) {

		/* The register the callback routines */
		memset(&cbs, 0, sizeof(wl_drv_netif_callbacks_t));
		cbs.rx_pkt = wps_rx_callback;

		memset(&g_WpsParam, 0, sizeof(g_WpsParam));

		status = osl_ext_queue_create("WPS ReadData", WPS_MAX_QUEUE_SZ,
		                                              &g_WpsParam.in_Queue);
		ASSERT(status == OSL_EXT_SUCCESS);

		/* register callbacks */
		wl_drv_register_netif_callbacks_if(wl_drv_get_handle(), g_ifidx, &cbs);

		wps_osl_get_mac(g_WpsParam.source_mac);
		TUTRACE((TUTRACE_INFO,
		         "Obtained Mac address is : %x:%x:%x:%x:%x:%x \n",
		                                    g_WpsParam.source_mac[0],
		                                    g_WpsParam.source_mac[1],
		                                    g_WpsParam.source_mac[2],
		                                    g_WpsParam.source_mac[3],
		                                    g_WpsParam.source_mac[4],
		                                    g_WpsParam.source_mac[5]));
		g_EapOsInitialised = 1;
		status = WPS_SUCCESS;
	}

	Eap_SetBssid(bssid);

	return status;
}


uint32
Eap_OSDeInit()
{
	wl_drv_netif_callbacks_t cbs;

	if (!g_EapOsInitialised)
		return WPS_SUCCESS;

	g_EapOsInitialised = 0;

	/* unregister the callback routines */
	memset(&cbs, 0, sizeof(wl_drv_netif_callbacks_t));
	cbs.rx_pkt = wps_rx_callback;
	wl_drv_deregister_netif_callbacks_if(wl_drv_get_handle(), g_ifidx, &cbs);

	osl_ext_queue_delete(&g_WpsParam.in_Queue);

	memset(&g_WpsParam, 0, sizeof(g_WpsParam));

	return WPS_SUCCESS;
}

uint32 Eap_SetBssid(uint8 *bssid)
{
	memcpy(g_WpsParam.eapol_bssid, bssid, ETHER_ADDR_LEN);
	return WPS_SUCCESS;
}

uint32 Eap_ReadData(char * dataBuffer, uint32 * dataLen, uint32 timeout, bool raw)
{
	wps_pktInfo_t   *data;
	uint32      status;

	if (!dataBuffer || (! dataLen))
		return WPS_ERR_INVALID_PARAMETERS;

	timeout *= 1000;  /* osl_sem_take accepts timeout in milli second only */

	/* Fetch a EAPOL packet from Queue */
	status = osl_ext_queue_receive(&g_WpsParam.in_Queue, timeout, (void **)&data);

	if (status == OSL_EXT_SUCCESS) {
		uint32 length = data->in_Size;
		char *pkt = data->in_Data;

		if (!raw) {
			/* Strip off the Eth Header datas */
			length -= sizeof(ETH_HEADER);
			pkt += sizeof(ETH_HEADER);
		}

		*dataLen = length;
		memcpy(dataBuffer, pkt, *dataLen);

#ifdef _TUDEBUGTRACE
		print_buf((unsigned char*)dataBuffer, *dataLen);
#endif

		/* Free the allocated data packet */
		free(data->in_Data); /* Free the Native Packet */
		free(data);

		status = WPS_SUCCESS;
	} else {
		status = EAP_TIMEOUT;
	}

	return (status);

}

uint32
Eap_SendDataDown(char * dataBuffer, uint32 dataLen, bool raw)
{
	uint8 *packet, *data;
	ETH_HEADER *ether;
	uint32 packet_len;

	TUTRACE((TUTRACE_INFO, "In CInbEap::SendDataDown buffer Length = %d\n", dataLen));

	if ((! dataBuffer) || (! dataLen)) {
		TUTRACE((TUTRACE_ERR, "Invalid Parameters\n"));
		return WPS_ERR_INVALID_PARAMETERS;
	}

	/* build ethernet header */
	/* @Info: have got only one send packet */

	packet_len = dataLen;
	if (!raw)
		packet_len += sizeof(ETH_HEADER);
	packet = malloc(packet_len);

	if (!packet)
		return WPS_ERR_OUTOFMEMORY;

	if (!raw) {
		ether = (ETH_HEADER *)packet;
		memcpy(ether->ether_dhost, g_WpsParam.eapol_bssid, ETHER_ADDR_LEN);
		memcpy(ether->ether_shost, g_WpsParam.source_mac, ETHER_ADDR_LEN);
		ether->ether_type = WpsHtons((uint16)ETHER_TYPE_802_1X);
	}

	data = packet;
	if (!raw)
		data += sizeof(ETH_HEADER);
	memcpy(data, dataBuffer, dataLen);

	/* send frame */
	return wl_drv_tx_pkt_if(wl_drv_get_handle(), g_ifidx, packet, packet_len);
}

/* implement Portability.h */
uint32
WpsHtonl(uint32 intlong)
{
	return hton32(intlong);
}

uint16
WpsHtons(uint16 intshort)
{
	return hton16(intshort);
}

uint16 WpsHtonsPtr(uint8 * in, uint8 * out)
{
	uint16	v;
	uint8	*c;

	c = (uint8 *)&v;
	c[0] = in[0]; c[1] = in[1];
	v = hton16(v);
	out[0] = c[0]; out[1] = c[1];

	return v;
}

uint32
WpsHtonlPtr(uint8 * in, uint8 * out)
{
	uint32 v;
	uint8 *c;

	c = (uint8 *)&v;
	c[0] = in[0]; c[1] = in[1]; c[2] = in[2]; c[3] = in[3];
	v = hton32(v);
	out[0] = c[0]; out[1] = c[1]; out[2] = c[2]; out[3] = c[3];

	return v;
}


uint32
WpsNtohl(uint8 *a)
{
	uint32 v;

	v = (a[0] << 24) + (a[1] << 16) + (a[2] << 8) + a[3];
	return v;
}

uint16
WpsNtohs(uint8 *a)
{
	uint16 v;

	v = (a[0]*256) + a[1];
	return v;
}

void
WpsSleepMs(uint32 ms)
{
	osl_delay(1000*ms);
}

void
WpsSleep(uint32 seconds)
{
	WpsSleepMs(1000*seconds);
}

void WpsSetBssid(uint8 *bssid)
{
	Eap_SetBssid(bssid);
}

uint32
wait_for_eapol_packet(char* buf, uint32* len, uint32 timeout)
{
	return Eap_ReadData(buf, len, timeout, FALSE);
}

uint32
wait_for_packet(char* buf, uint32* len, uint32 timeout, bool raw)
{
	return Eap_ReadData(buf, len, timeout, raw);
}

uint32
send_eapol_packet(char *packet, uint32 len)
{
	return Eap_SendDataDown(packet, len, FALSE);
}

uint32
send_packet(char *packet, uint32 len)
{
	return Eap_SendDataDown(packet, len, TRUE);
}

unsigned long
get_current_time()
{
	unsigned long time_val;

	OSL_GETCYCLES(time_val);

	/* Return the current time in seconds */
	return (OSL_TICKS_TO_MSEC(time_val) / 1000);
}


/* WPS Common Lib needs the following 2 dummy api's as part of compilation */
void
wps_setProcessStates(int state)
{
	return;
}

void
wps_setStaDevName(char *str)
{
	return;
}

void
wps_setPinFailInfo(uint8 *mac, char *name, char *state)
{
	return;
}

/* Link to wl driver. */
int
wps_wl_ioctl(int cmd, void *buf, int len, bool set)
{
	wl_drv_ioctl_t ioc;
	wl_drv_hdl drv_hdl;

	if (!(drv_hdl = wl_drv_get_handle())) {
		TUTRACE((TUTRACE_ERR, "Wl Driver is not Initialized \n"));
		return WPS_ERR_SYSTEM;
	}

	memset(&ioc, 0, sizeof(ioc));
	ioc.ifidx = g_ifidx;
	ioc.w.cmd = cmd;
	ioc.w.buf = buf;
	ioc.w.len = len;
	ioc.w.set = set;

	/* Copy the result back */
	return wl_drv_ioctl(drv_hdl, &ioc);
}
