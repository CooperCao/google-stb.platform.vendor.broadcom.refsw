/*
 *  USB constant definitions.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifndef __USBCONST_H__
#define __USBCONST_H__

/*
 * /////////
 * Types //
 * /////////
 */

#define USB_DEVICE_DESC_TYPE		0x01
#define USB_CONFIG_DESC_TYPE		0x02
#define USB_STRING_DESC_TYPE		0x03
#define USB_INTERFACE_DESC_TYPE		0x04
#define USB_ENDPOINT_DESC_TYPE		0x05
#define USB_DEV_QUAL_DESC_TYPE		0x06
#define USB_OTH_SPEED_CFG_DESC_TYPE	0x07
#define USB_INTFC_PWR_DESC_TYPE		0x08
#define USB_INTFC_ASSOC_DESC_TYPE	0x0B
#define USB_HID_DESC_TYPE		0x21
#define USB_REPORT_DESC_TYPE		0x22
#define USB_BOS_DESC_TYPE		0x0F
#define USB_DEV_CAP_DESC_TYPE		0x10

#define USB_LANGUAGE_IDX	0x00
#define USB_MANUFACTURER_IDX	0x01
#define USB_PRODUCT_IDX		0x02
#define USB_SERIAL_NUM_IDX	0x03

#define USB_DEVICE_DESC_LENGTH 18

#define USB_VDR_CLASS_TYPE		0xFF
#define USB_VDR_SUB_CLASS_TYPE		0xFF
#define USB_VDR_CLASS_PROTOCOL_TYPE	0xFF

#define USB_SUB_CLASS_ABSTRACT_CONTROL_MODEL 0x02

#define USB_ENDP_TYPE_CTRL	0x00
#define USB_ENDP_TYPE_ISO	0x01
#define USB_ENDP_TYPE_BULK	0x02
#define USB_ENDP_TYPE_INTR	0x03

#define USB_BM_REMOTEWAKEUP	(1 << 5)
#define USB_BM_SELFPOWERED	(1 << 6)
#define USB_BM_BUSPOWERED	(1 << 7)

/* Assert Remote Wake for 3ms */
#define USB_REMOTE_WAKE_ASSERT_DURATION 0x03

/*
 * /////////
 * Sizes //
 * /////////
 */
#define USB_DEVDES_SIZE 0x12
#define USB_CFGDES_SIZE 0x9
#define USB_INTFDES_SIZE 0x9
#define USB_ENDPDES_SIZE 0x9

#define USB_DFLT_RDL_CFGDES_SIZE	0x27			/* (CONFIG + INTF + (3*ENDP) */
#define USB_DFLT_RDL_CFGDES_SIZE_LSB	USB_DFLT_RDL_CFGDES_SIZE
#define USB_DFLT_RDL_CFGDES_SIZE_MSB	0

#define USB_ONE_LANGUAGE_ID_SIZE 4

#define USB_DFLT_MANUF_ID_SIZE	0x1C
#define USB_MANUF_ID_BUF_SIZE	42			/* 20 characters */

#define USB_DFLT_PRODUCT_ID_SIZE	0x44
#define USB_PRODUCT_ID_BUF_SIZE		66		/* 50 characters */
#define USB_WLAN_SERIAL_NUMBER_SIZE 0x1A

#define USB_CTRL_IN_FIFO_SIZE	64
#define USB_INT_IN_FIFO_SIZE	16
#define USB_BULK_IN_FIFO_SIZE	64
#define USB_ISOCH_IN_FIFO_SIZE	128
#define USB_DIAG_IN_FIFO_SIZE	32

#define USB_CTRL_OUT_FIFO_SIZE	64
#define USB_BULK_OUT_FIFO_SIZE	64
#define USB_ISOCH_OUT_FIFO_SIZE 64
#define USB_DIAG_OUT_FIFO_SIZE	32

#define USB_DESC_PID_LSB_OFFSET 10
#define USB_DESC_PID_MSB_OFFSET 11

#define USB_CFG_BMATTR_IDX			0x07
#define USB_CFG_BMATTR_RMTWAKE_MASK		0x20
#define USB_CFG_BMATTR_SELF_POWERED_MASK	0x40

#define USB_ATTR_BUS_POWERED	(1 << 7)
#define USB_ATTR_REMOTE_WAKEUP	(1 << 5)
#define USB_DEV_ATTRIBUTES (USB_ATTR_BUS_POWERED | USB_ATTR_REMOTE_WAKEUP)

#define USB_UC_POWER_FACTOR	2
#define USB_DEV_MAXPOWER	(200 / USB_UC_POWER_FACTOR)

#define USB_UD_2_0		0x0200

#define USB_MAX_INTR_PACKET	0x10
#define USB_MAX_CTRL_PACKET	0x40
#define USB_MAX_BULK_PACKET	0x200

/*
 * //////////////////////////////
 * Setup buffer access macros //
 * //////////////////////////////
 */
#define USB_SETUP_BM_REQUEST_TYPE(buf)	buf[0]
#define USB_SETUP_B_REQUEST(buf)	buf[1]

#define USB_SETUP_VALUE(buf)		((buf[3] << 8) | buf[2])
#define USB_SETUP_VALUE_LSB(buf)	buf[2]
#define USB_SETUP_VALUE_MSB(buf)	buf[3]

#define USB_SETUP_INDEX(buf)		((buf[5] << 8) | buf[4])
#define USB_SETUP_INDEX_LSB(buf)	buf[4]
#define USB_SETUP_INDEX_MSB(buf)	buf[5]

#define USB_SETUP_LENGTH(buf)		((buf[7] << 8) | buf[6])
#define USB_SETUP_LENGTH_LSB(buf)	buf[6]
#define USB_SETUP_LENGTH_MSB(buf)	buf[7]

#define USB_PORT2_BM_REQUEST_TYPE	USB_port2out_buffer[port_id][0]
#define USB_PORT2_B_REQUEST		USB_port2out_buffer[port_id][1]

#define USB_PORT3_BM_REQUEST_TYPE	USB_port3out_buffer[port_id][0]
#define USB_PORT3_B_REQUEST		USB_port3out_buffer[port_id][1]

/*
 * //////////////////////////////////
 * BM_REQUEST_TYPE MASKS and Values //
 * //////////////////////////////////
 */
#define USB_BMREQ_RECIPIENT_MASK	0x1f
#define     USB_BMREQ_DEVICE		0x00
#define     USB_BMREQ_INTF		0x01
#define     USB_BMREQ_ENDPT		0x02
#define     USB_BMREQ_OTHER		0x03

#define USB_BMREQ_TYPE_MASK	0x60
#define     USB_BMREQ_STD	0x00
#define     USB_BMREQ_CLS	0x20
#define     USB_BMREQ_VDR	0x40
#define     USB_BMREQ_RSVD	0x60

#define USB_BMREQ_DIR_MASK		0x80
#define     USB_BMREQ_HOST_TO_DEV	0x00
#define     USB_BMREQ_DEV_TO_HOST	0x80
/*
 * //////////////////
 * B_REQUEST Types //
 * //////////////////
 */
#define USB_STD_GET_STATUS_REQ_VALUE		0x00
#define USB_STD_CLEAR_FEATURE_REQ_VALUE		0x01
#define USB_STD_RSVD1_VALUE			0x02
#define USB_STD_SET_FEATURE_REQ_VALUE		0x03
#define USB_STD_RSVD2_REQ_VALUE			0x04
#define USB_STD_SET_ADDRESS_REQ_VALUE		0x05
#define USB_STD_GET_DESCRIPTOR_REQ_VALUE	0x06
#define USB_STD_SET_DESCRIPTOR_REQ_VALUE	0x07
#define USB_STD_GET_CONFIG_REQ_VALUE		0x08
#define USB_STD_SET_CONFIG_REQ_VALUE		0x09
#define USB_STD_GET_INTERFACE_REQ_VALUE		0x0A
#define USB_STD_SET_INTERFACE_REQ_VALUE		0x0B
#define USB_STD_SYNCH_FRAME_REQ_VALUE		0x0C

#define USB_CLASS_HID_GET_REPORT	0x01
#define USB_CLASS_HID_GET_IDLE		0x02
#define USB_CLASS_HID_GET_PROTOCOL	0x03
#define USB_CLASS_HID_SET_REPORT	0x09
#define USB_CLASS_HID_SET_IDLE		0x0A
#define USB_CLASS_HID_SET_PROTOCOL	0x0B

#define USB_VDR_SET_TEST_MEM_VALUE	0x01
#define USB_VDR_GET_TEST_MEM_VALUE	0x02
#define USB_VDR_SET_HID_LOCAL_LOOPBACK	0x03
#define USB_VDR_GET_MS_DESCRIPTOR	0x04
#define USB_VDR_UBIST_COMMAND		0x05

/* Vendor B_REQUEST Types */
/* Control messages: B_REQUEST values */
#define USB_VDR_DL_GETSTATE		0x00				/* returns the
									 *rdl_state_t struct
									 */
#define USB_VDR_DL_CHECK_CRC		0x01			/* currently unused */
#define USB_VDR_DL_GO			0x02				/* execute downloaded
									 *image
									 */
#define USB_VDR_DL_START		0x03				/* initialize dl state */
#define USB_VDR_DL_REBOOT		0x04				/* reboot the device in 2
									 *seconds
									 */
#define USB_VDR_DL_GETVER		0x05				/* returns the
									 *bootrom_id_t struct
									 */
#define USB_VDR_DL_GO_PROTECTED		0x06			/* execute the downloaded code
								 * and set reset event
								 * to occur in 2 seconds.  It is
								 *the responsibility
								 * of the downloaded code to
								 *clear this event
								 */
#define USB_VDR_DL_EXEC			0x07				/* jump to a supplied
									 *address
									 */
#define USB_VDR_DL_RESETCFG		0x08				/* To support single enum
									 * on dongle
									 * - Not used by
									 *bootloader
									 */
#define USB_VDR_DL_DEFER_RESP_OK	0x09			/* Potentially defer the response
								 * to setup
								 * if resp unavailable
								 */
#define USB_VDR_DL_CHGSPD		0x0A
#define USB_VDR_DL_GET_NVRAM		0x35		/* Query nvram parameter */

/*
 * Defines for when an endpoint is waiting for memory to handle a command.
 * Not all of these are in use...only acl for now.
 */
#define PORT1_EP0_WAIT_MASK	0x0001
#define PORT1_EP2_WAIT_MASK	0x0002
#define PORT1_EP3_WAIT_MASK	0x0004
#define PORT1_EP4_WAIT_MASK	0x0008
#define PORT2_EP0_WAIT_MASK	0x0010
#define PORT2_EP2_WAIT_MASK	0x0020
#define PORT2_EP3_WAIT_MASK	0x0040
#define PORT2_EP4_WAIT_MASK	0x0080
#define PORT3_EP0_WAIT_MASK	0x0100
#define PORT3_EP2_WAIT_MASK	0x0200
#define PORT3_EP3_WAIT_MASK	0x0400
#define PORT3_EP4_WAIT_MASK	0x0800

#define USB_EP0_OUT_TEMP_MEM_SIZE	256
#define USB_EP2_OUT_TEMP_MEM_SIZE	64
#define USB_EP3_OUT_TEMP_MEM_SIZE	256
#define USB_EP4_OUT_TEMP_MEM_SIZE	32

#endif /* __USBCONST_H__ */
