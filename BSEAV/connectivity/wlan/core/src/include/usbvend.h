/*
 * Definitions for USB vendor requests and other declaration
 * shared by the firmware and the windows driver.
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

#ifndef _usbvend_h_
#define _usbvend_h_


/* Version number which goes into the dev_rel field of the device descriptor */

#define	USBDL_REVISION		0x0006		/* Current USB download code version */


/* All "real" packets start with a signature, followed by priority/length */

#define	DLPKT_SIGN		0xa0fa		/* Signature at the beggining of download packets */
#define	JMPTO_SIGN		0xa10e		/* Signature at the beggining of jump-to packets */
#define	PKT_SIGN		0xfeda		/* Signature at the beggining of a real packet */

#define	USBLEN_MASK		0x1fff		/* Length, more bits than needed ... */
#define	USBPRI_MASK		0xe000		/* Priority in high 3 bits */
#define	USBPRI_SHIFT		13
#define VER_STR_SIZE		64

/* USB vendor requests: */

#define	GET_ETH_DESCRIPTOR	0
#define	SET_ETH_MULTI_FILTER	1
#define	SET_ETH_PKT_FILTER	2
#define	GET_ETH_STATISTICS	3
#define	GET_AUX_INPUTS		4
#define	SET_AUX_OUTPUTS		5
#define	SET_TEMP_MAC		6
#define	SET_TEMP_MAC_1		0x51
#define	GET_TEMP_MAC		7
#define	GET_TEMP_MAC_1		0x50
#define	SET_URB_SIZE		8
#define	SET_SOFS_TO_WAIT	9
#define	SET_EVEN_PACKETS	0x0A
#define SET_WAKEUP              0x0D
#define SET_WAKE_PAT            0x0E
#define REMOVE_WAKE_PAT         0x0F
#define	PREPARE_DL		0x10
#define	GET_FW_VER		0x11
#define	READ_I2C		0x12
#define	WRITE_I2C		0x13
#define	SCAN			0xff

/* EPI_CMD is the vendor command request; the LSB
 * of the desired OID is the index value
 */
#define	EPI_CMD			64

/* GET_ETH_DESCRIPTOR: */

typedef struct _iline_desc {
	struct ether_addr ea;		/* MAC address */
	uint16		aferev;		/* iLine AFE revision */
	uint16		num_mc_filt;	/* Number of multicast filters supported */
} iline_desc_t;

/* make sure host driver & firmware agree on alignment */
#pragma pack(2)
typedef struct {
	uint32 ctrcollisions;		/* # tx total collisions */
	uint32 ctrcollframes;		/* # tx frames encountering a collision */
	uint16 ctrshortframe;		/* # rx frames discarded due to carrier loss
					 * before min duration
					 */
	uint16 ctrlongframe;		/* # rx frames discarded due to carrier beyond
					 * max duration
					 */
	uint32 ctrfilteredframes;	/* # rx frames discarded due to nomatch */
	uint32 ctrmissedframes;		/* # rx frames discarded due to fifo overflow */
	uint32 ctrerrorsframes;		/* # rx frames discarded due to demod error */
	uint16 ctr1linkrcvd;		/* # HPNA 1.0 link frames received */
	uint16 ctrrcvtooshort;		/* # rx frames discarded due to <20byte len */
	uint16 ctrxmtlinkintegrity;	/* # tx link integritry frames transmitted */
	uint16 ctr1latecoll;		/* # HPNA collision detected after AID */
	uint16 ctrifgviolation;		/* # carrier detected in Interframe_Gap */
	uint16 ctrsignalviolation;	/* # carrier detected in Collision_Signal */
	uint16 ctrstackbubble;		/* # dfpq winner did not transmit in slot */
	uint16 rxhcrc;			/* header crc errors */
	uint16 rxpcrc;			/* payload crc errors */
} ctrs_t;
#pragma pack()

/* SET_ETH_PKT_FILTER: */

/* Bitmap for value field: */
#define	PF_PROMISCUOUS		0x0001
#define	PF_ALL_MULTICAST	0x0002
#define	PF_DIRECTED		0x0004
#define	PF_BROADCAST		0x0008
#define	PF_MULTICAST		0x0010

/* GET_FW_VER: */

#define	NO_FW			0		/* No firmware loaded, running the small i2c code */
#define	KLSIUSB_REVISION	0x0002		/* Current Firmware version */

#endif	/* _usbvend_h_ */
