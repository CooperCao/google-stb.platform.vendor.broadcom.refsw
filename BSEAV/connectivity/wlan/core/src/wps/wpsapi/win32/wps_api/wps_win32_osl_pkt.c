/* 
 * Copyright (C) 2011, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: $
 */

#include "stdafx.h"
#include "wps_api_osl.h"
#include "Packet32.h"

extern void WpsDebugOutput(LPCSTR fmt, ...);

/* Debug for dumping recv/send data */
#define _TEST

/* Enable structure packing */
#if defined(__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#pragma pack(1)
#define	PACKED
#endif

/* Frame Polling Interval */
#define EAPOL_FRAME_POLL_TIMEOUT	1	/* ms */  
#define EAPOL_FRAME_MAX_SIZE		2048	/* bytes */

#define XSUPPD_MAX_PACKET			4096
#define XSUPPD_ETHADDR_LENGTH		6
#define XSUPPD_MAX_FILENAME			256
#define XSUPPD_MAX_NETNAME			64

/* Memory manipulation */
#define B1XS_ALLOC(s)		malloc(s)
#define B1XS_FREE(m)		free(m)

/* EAPOL Ethernet Type */
#define EAPOL_ETHER_TYPE	0x888e

/* set/get type field in ether header */
#define ether_get_type(p)		(((p)[0] << 8) + (p)[1])
#define ether_set_type(p, t)	((p)[0] = (uint8)((t) >> 8), (p)[1] = (uint8)(t))

/*
 * The instruction encodings.
 */
/* instruction classes */
#define BPF_CLASS(code) ((code) & 0x07)
#define		BPF_LD		0x00
#define		BPF_LDX		0x01
#define		BPF_ST		0x02
#define		BPF_STX		0x03
#define		BPF_ALU		0x04
#define		BPF_JMP		0x05
#define		BPF_RET		0x06
#define		BPF_MISC	0x07

/* ld/ldx fields */
#define BPF_SIZE(code)	((code) & 0x18)
#define		BPF_W		0x00
#define		BPF_H		0x08
#define		BPF_B		0x10
#define BPF_MODE(code)	((code) & 0xe0)
#define		BPF_IMM 	0x00
#define		BPF_ABS		0x20
#define		BPF_IND		0x40
#define		BPF_MEM		0x60
#define		BPF_LEN		0x80
#define		BPF_MSH		0xa0

/* alu/jmp fields */
#define BPF_OP(code)	((code) & 0xf0)
#define		BPF_ADD		0x00
#define		BPF_SUB		0x10
#define		BPF_MUL		0x20
#define		BPF_DIV		0x30
#define		BPF_OR		0x40
#define		BPF_AND		0x50
#define		BPF_LSH		0x60
#define		BPF_RSH		0x70
#define		BPF_NEG		0x80
#define		BPF_JA		0x00
#define		BPF_JEQ		0x10
#define		BPF_JGT		0x20
#define		BPF_JGE		0x30
#define		BPF_JSET	0x40
#define BPF_SRC(code)	((code) & 0x08)
#define		BPF_K		0x00
#define		BPF_X		0x08

/* ret - BPF_K and BPF_X also apply */
#define BPF_RVAL(code)	((code) & 0x18)
#define		BPF_A		0x10

/* misc */
#define BPF_MISCOP(code) ((code) & 0xf8)
#define		BPF_TAX		0x00
#define		BPF_TXA		0x80

/*
 * Macros for insn array initializers.
 */
#define BPF_STMT(code, k) { (u_short)(code), 0, 0, k }
#define BPF_JUMP(code, k, jt, jf) { (u_short)(code), jt, jf, k }

#ifndef FRAME_DEBUG
#define FRAME_DEBUG 1
#endif

#ifndef FRAME_DUMP
#define FRAME_DUMP	0
#endif

#ifndef PACKET32_MAX_RX_BUFSIZE
#define PACKET32_MAX_RX_BUFSIZE	(64*1024)
#endif

#ifndef Packet_WORDALIGN
#define Packet_WORDALIGN PACKET_WORDALIGN
#endif

#ifdef UNICODE
#define atow(strA,strW,lenW) \
MultiByteToWideChar(CP_ACP, 0, strA, -1, strW, lenW)
#define wtoa(strW,strA,lenA) \
WideCharToMultiByte(CP_ACP, 0, strW, -1, strA, lenA, NULL, NULL)
#else
#define atow(strA,strW,lenW) _tcscpy(strW, strA)
#define wtoa(strW,strA,lenA) _tcscpy(strA, strW)
#endif

#define PRINTFBUFLENGTH	16


/* We need to set the ifname before anything else. */
/* ethernet header */
typedef struct
{
	uint8 dest[XSUPPD_ETHADDR_LENGTH];
	uint8 src[XSUPPD_ETHADDR_LENGTH];
	uint8 type[2];
} PACKED ether_header;

typedef struct
{
	/* outgoing packet */
	uint8 out[XSUPPD_MAX_PACKET];
	int out_size;
} sendpacket;

typedef struct xsup
{
	char eapol_device[XSUPPD_MAX_FILENAME]; /* Interface name */

	/* wireless device/framer */
	struct
	{
		uint32 timeout;
		void *adapter;
		void *rxbuf;
		void *rxpkt;
		void *rxptr;
		void *txpkt;
	} framer;
	void *device;

	uint8 eapol_bssid[XSUPPD_ETHADDR_LENGTH];  /* MAC address of authenticator */
	char eapol_ssid[XSUPPD_MAX_NETNAME];  /* network name */

	char padding1[2];
	uint8 source_mac[XSUPPD_ETHADDR_LENGTH];
	char padding2[2];
	uint8 dest_mac[XSUPPD_ETHADDR_LENGTH];

	/* incoming packet */
	/*uint8_t in[XSUPPD_MAX_PACKET];*/
	uint8 *in;
	int in_size;

} xsup_t;

/* Local contexts */
xsup_t g_EAPParams;
static DWORD g_dwFilter = NDIS_PACKET_TYPE_DIRECTED |
						  NDIS_PACKET_TYPE_MULTICAST |
						  NDIS_PACKET_TYPE_ALL_MULTICAST |
						  NDIS_PACKET_TYPE_BROADCAST |
						  NDIS_PACKET_TYPE_ALL_LOCAL;


static void
printbuf(uint8 *buf, uint length)
{
#ifdef _TEST
	uint i, j = 0;

	for (i = 0; i+j < length; i += PRINTFBUFLENGTH) {
		printf("[%4.4X] ", i+j);
		for (j = 0; j < PRINTFBUFLENGTH; j++)
			if (i+j < length)
				printf("%2.2X ", buf[i+j]);
			else
				printf("   ");
		for (j = 0; j < PRINTFBUFLENGTH; j++)
			if (i+j < length)
				printf("%c", (buf[i+j] > 31 && buf[i+j] < 128) ? buf[i+j] : '.');
			else
				printf(" ");
		printf("\n");
		j = 0;
	}
#endif
}

/*
* _wps_osl_frame_recv_frame(int msec) -- returns a pointer to a frame if there is one.
*  NULL if there isn't.  msec is how long to sleep before returning.
*/
static uint8 *
_wps_osl_frame_recv_frame(xsup_t *xsup, int *len)
{
	struct bpf_hdr *hdr;
	uint32 off;

	/* Read frames (could be multiple of them) */
	if (xsup->framer.rxptr == NULL)
	{
		// Always check adapter existence as PacketReceivePacket will crash if NULL adapter is passed in
		if((LPADAPTER)xsup->framer.adapter == NULL)
			return NULL;

		PacketReceivePacket((LPADAPTER)xsup->framer.adapter, (LPPACKET)xsup->framer.rxpkt, TRUE);

		if (((LPPACKET)xsup->framer.rxpkt)->ulBytesReceived == 0)
		{
			WpsDebugOutput("Timeout....\n");

			*len = 0;
			return NULL;
		}
		xsup->framer.rxptr = xsup->framer.rxbuf;
	}
	
	/* Grab one frame */
	hdr = (struct bpf_hdr *)xsup->framer.rxptr;
	xsup->in = (uint8*)hdr + hdr->bh_hdrlen;
	*len = xsup->in_size = hdr->bh_datalen;
	
	/* update rxptr to point to the next frame */
	off = (uint32)((char *)xsup->framer.rxptr - (char *)xsup->framer.rxbuf);
	off = Packet_WORDALIGN(off + hdr->bh_hdrlen + hdr->bh_datalen);
	if (off < ((LPPACKET)xsup->framer.rxpkt)->ulBytesReceived)
		xsup->framer.rxptr = (char *)xsup->framer.rxbuf + off;
	else
		xsup->framer.rxptr = NULL;
	return xsup->in;
}

/* Send a frame. */
static int
_wps_osl_frame_send_frame(xsup_t *xsup, uint8 *frame, int len)
{
	int status = WPS_OSL_ERROR;
	PacketInitPacket((LPPACKET)xsup->framer.txpkt, (void *)frame, len);
	if(!PacketSendPacket((LPADAPTER)xsup->framer.adapter, (LPPACKET)xsup->framer.txpkt, TRUE))
		WpsDebugOutput("%s : PacketSendPacket failure.\n",__FUNCTION__);

	return WPS_OSL_SUCCESS;
}

static int
frame_init(xsup_t *xsup, TCHAR *device, uint8 *mac, int size, int timeout)
{	
	if (device == NULL)
	{
		WpsDebugOutput("frame_init: no device specified!\n");
		goto exit0;
	}

	// Unload the NPF Winpcap protocol driver beforing using it. This is to make sure that 
	// NPF driver is in clean and fresh state. For example, restarting the NPF can clear
	// possible bad state left from previous activities. Or it can also capture newly added
	// adapter after NPF is reloaded. If NPF is being used by any party, PacketStopDriver will 
	// fail.
	PacketStopDriver();

	/* Stash a copy of the device name and address. */
	wtoa(device, xsup->eapol_device,XSUPPD_MAX_FILENAME);
	memcpy(xsup->source_mac, mac, XSUPPD_ETHADDR_LENGTH);

	if ((xsup->framer.adapter = PacketOpenAdapter((LPTSTR)device)) == NULL)
	{
		WpsDebugOutput("frame_init: PacketOpenAdapter failed (%s)!\n",device);
		goto exit0;
	}

	/* Allocate buffer/packet for receving */
	if ((xsup->framer.rxbuf = B1XS_ALLOC(PACKET32_MAX_RX_BUFSIZE)) == NULL)
	{
		WpsDebugOutput("frame_init: B1XS_ALLOC (rxbuf) failed!\n");
		goto exit1;
	}
	memset(xsup->framer.rxbuf, 0, PACKET32_MAX_RX_BUFSIZE);
	if ((xsup->framer.rxpkt = PacketAllocatePacket()) == NULL)
	{
		WpsDebugOutput("frame_init: PacketAllocatePacket (rx) failed!\n");
		goto exit2;
	}
	PacketInitPacket((LPPACKET)xsup->framer.rxpkt, xsup->framer.rxbuf, PACKET32_MAX_RX_BUFSIZE);
	/* pointer to current frame in the rx buffer (could be multiple frames) */
	xsup->framer.rxptr = NULL;
	/* Allocate a packet for transmitting */
	if ((xsup->framer.txpkt = PacketAllocatePacket()) == NULL)
	{
		WpsDebugOutput("frame_init: PacketAllocatePacket (tx) failed!\n");
		goto exit3;
	}
	
	/* Setup filter */
	{
		struct bpf_insn fltr[] = {
			/* Check Ethernet destination address At Offset 0 */
			/* 0  */BPF_STMT(BPF_LD|BPF_W|BPF_ABS, 0),
			/* 1  */BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htonl(*(uint32*)&xsup->source_mac[0]), 0, 11),
			/* Check Ethernet destination address At Offset 4 */
			/* 2  */BPF_STMT(BPF_LD|BPF_H|BPF_ABS, 4),
			/* 3  */BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htons(*(uint16*)&xsup->source_mac[4]), 0, 9),
			/* Check Ethernet type/length At Offset 12 */
			/* 4  */BPF_STMT(BPF_LD|BPF_H|BPF_ABS, 12),
			/* 5  */BPF_JUMP(BPF_JMP|BPF_JGE|BPF_K, 1536, 5, 0),
			/* Check SNAP header At Offset 14 */
			/* 6  */BPF_STMT(BPF_LD|BPF_H|BPF_ABS, 14),
			/* 7  */BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, 0xAAAA, 0, 5),
			/* 8  */BPF_STMT(BPF_LD|BPF_W|BPF_ABS, 16),
			/* 9  */BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, 0x03000000, 0, 3),
			/* 10 */BPF_STMT(BPF_LD|BPF_H|BPF_ABS, 20),
			/* 11 */BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, EAPOL_ETHER_TYPE, 0, 1),
			/* Decision */
			/* 12 */BPF_STMT(BPF_RET|BPF_K, (uint32)-1),	/* Accept. it is for us */
			/* 13 */BPF_STMT(BPF_RET|BPF_K, 0)		/* Reject. it is not for us */
		};
		struct bpf_program prog = {sizeof(fltr)/sizeof(fltr[0]), fltr};
		if(!PacketSetHwFilter((LPADAPTER)xsup->framer.adapter, g_dwFilter))
			WpsDebugOutput("%s : Failure in setting HW filter to promiscuous\n", __FUNCTION__);
		if(!PacketSetBpf((LPADAPTER)xsup->framer.adapter, &prog))
			WpsDebugOutput("%s : Failure in setting BPF\n", __FUNCTION__);

		if(!PacketSetNumWrites((LPADAPTER)xsup->framer.adapter, 1))
				WpsDebugOutput("%s : PacketSetNumwrites failure\n", __FUNCTION__);
	
		/* Set adapter to normal mode */
		//PacketSetHwFilter((LPADAPTER)xsup->framer.adapter, NDIS_PACKET_TYPE_ALL_LOCAL);
		/* Set read timeout */
		if(!PacketSetReadTimeout((LPADAPTER)xsup->framer.adapter, EAPOL_FRAME_POLL_TIMEOUT)) 
			WpsDebugOutput("%s : Failure in setting read time out\n", __FUNCTION__);
		/* Set capture buffer size */ 
		if(!PacketSetBuff((LPADAPTER)xsup->framer.adapter, PACKET32_MAX_RX_BUFSIZE))
			WpsDebugOutput("%s : Failure in setting buff\n", __FUNCTION__);
	}

	return WPS_OSL_SUCCESS;

exit3:
	PacketFreePacket((LPPACKET)xsup->framer.rxpkt);
exit2:
	B1XS_FREE(xsup->framer.rxbuf);
exit1:
	PacketCloseAdapter((LPADAPTER)xsup->framer.adapter);
	WpsDebugOutput("### %s: PacketCloseAdapter\n", __FUNCTION__);

exit0:
	return WPS_OSL_ERROR;
}

static int
frame_cleanup(xsup_t *xsup)
{
	xsup->eapol_device[0] = 0;

	/* Shutdown PCAP */
	if (xsup->framer.rxpkt != NULL)
	{
		PacketFreePacket((LPPACKET)xsup->framer.rxpkt);
		xsup->framer.rxpkt = NULL;
	}
	if (xsup->framer.rxbuf != NULL)
	{
		B1XS_FREE(xsup->framer.rxbuf);
		xsup->framer.rxbuf = NULL;
	}
	if (xsup->framer.txpkt != NULL)
	{
		PacketFreePacket((LPPACKET)xsup->framer.txpkt);
		xsup->framer.txpkt = NULL;
	}
	if (xsup->framer.adapter != NULL)
	{
		PacketCloseAdapter((LPADAPTER)xsup->framer.adapter);
		xsup->framer.adapter = NULL;
	}
	return WPS_OSL_SUCCESS;
}

static int
eapol_wireless_init(xsup_t *xsup, TCHAR *device, uint8 *bssid, char *ssid)
{	
	strcpy(xsup->eapol_ssid, ssid);
	memcpy(&xsup->eapol_bssid, bssid, XSUPPD_ETHADDR_LENGTH);
	memcpy(xsup->dest_mac, xsup->eapol_bssid, XSUPPD_ETHADDR_LENGTH);
	return WPS_OSL_SUCCESS;
}

/* ######### */
/* Exported APIs */
/* ######### */
uint32
wps_osl_setup_802_1x(uint8 *bssid)
{
	uint8 mac[6];
	uint32 status = WPS_OSL_ERROR;

	if (wps_osl_get_mac(mac) != WPS_OSL_SUCCESS) {
		WpsDebugOutput("setup_802_1x: Couldn't retrieve device mac!\n");
		goto done;
	}

	memset(&g_EAPParams, 0, sizeof(g_EAPParams));
	status = frame_init(&g_EAPParams, wps_osl_get_adapter_name(), mac, EAPOL_FRAME_MAX_SIZE,
		EAPOL_FRAME_POLL_TIMEOUT);
	if (status != WPS_OSL_SUCCESS) {
		WpsDebugOutput("setup_802_1x: Couldn't initalize frame device!\n");
		goto done;
	}

	/* Init wireless calls */
	if (eapol_wireless_init(&g_EAPParams, wps_osl_get_adapter_name(), bssid, wps_osl_get_ssid())
		!= WPS_OSL_SUCCESS) {
		WpsDebugOutput( "setup_802_1x: Couldn't initalize wireless extensions!\n");
		goto done;
	}

	status = WPS_OSL_SUCCESS;
done:
	return status;
}

int
wps_osl_cleanup_802_1x()
{
	return frame_cleanup(&g_EAPParams);
}

uint32
wps_osl_eap_read_data(char *dataBuffer, uint32 *dataLen, uint32 timeout_val)
{
	ether_header *frame;

	if (dataBuffer && (!dataLen)) {
		return WPS_OSL_ERROR;
	}

	/* Read a frame from device and process it... */
	frame  = (ether_header *)_wps_osl_frame_recv_frame(&g_EAPParams, dataLen);
	if (frame != NULL && ether_get_type(frame->type) == EAPOL_ETHER_TYPE) {
		memset(dataBuffer, 0, *dataLen);
		*dataLen -= sizeof(ether_header);
		memcpy(dataBuffer,frame+1, *dataLen);
#ifdef _TEST
		printf("Received data %d %d\n", *dataLen, sizeof(ether_header));
		printbuf(dataBuffer,*dataLen);
		printf("Done\n");
#endif
		return WPS_OSL_SUCCESS;
	}
	else {
		if (frame == NULL) {
			WpsDebugOutput("No packet received.\n" );
		}
		else {
			WpsDebugOutput("Received failure :incorrect frame type %x\n",
				ether_get_type(frame->type));
		}
		return WPS_OSL_TIMEOUT;
	}
}

uint32
wps_osl_eap_send_data(char *dataBuffer, uint32 dataLen)
{
    int sentBytes = 0;
#ifdef _TEST	
	INT i = 0;
#endif

	char *c = dataBuffer;
	sendpacket send_packet;
	uint8 *packet = (uint8 *)send_packet.out;
	ether_header *ether = (ether_header *)packet;
	uint8 *data = (uint8*)(ether + 1);

    WpsDebugOutput("SendDataDown buffer Length = %d\n", dataLen);
    if ((!dataBuffer) || (!dataLen)) {
	    WpsDebugOutput("Invalid Parameters\n");
	    return WPS_OSL_ERROR;
    }

	/* build ethernet header */
	memcpy(ether->dest, g_EAPParams.dest_mac, XSUPPD_ETHADDR_LENGTH);
	memcpy(ether->src, g_EAPParams.source_mac, XSUPPD_ETHADDR_LENGTH); 
	ether_set_type(ether->type, EAPOL_ETHER_TYPE);
	memcpy(data,dataBuffer ,dataLen);
	
	send_packet.out_size = dataLen+sizeof(ether_header);

	/* send frame */
#ifdef _TEST
	printf("Sending data %d\n", send_packet.out_size);
	printbuf(send_packet.out,send_packet.out_size);
	printf("Done\n");
#endif
    return _wps_osl_frame_send_frame(&g_EAPParams, send_packet.out, send_packet.out_size);
}

