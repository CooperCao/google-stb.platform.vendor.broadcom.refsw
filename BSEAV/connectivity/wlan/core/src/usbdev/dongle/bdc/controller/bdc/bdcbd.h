/*
 * Defines Transfer Request Block.
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

#ifndef _BDCBD_H_
#define _BDCBD_H_

#include <typedefs.h>

#define BDCBD_MAX_BD_TRANSFER_SIZE 65536

#define BDCBD_EVENT_TYPE	(0xF)
#define BDCBD_EVENT_XSF		0
#define BDCBD_EVENT_CMD		1
#define BDCBD_EVENT_PORT_STATUS 4
#define BDCBD_EVENT_CE		5

#define BDCBD_EVENT_BD_LENGTH 0xFFFFFF

#define BDCBD_XSF_EP_INDEX(p) (((p) >> 4) & 0x1f)
#define BDCBD_XSF_COMP_CODE (0xFU << 28)

#define BDCBD_XSF_COMP_SUCCESS			(0x1U << 28)
#define BDCBD_XSF_COMP_BUFFER_NOT_AVAILABLE	(0x2U << 28)
#define BDCBD_XSF_COMP_SHORT			(0x3U << 28)
#define BDCBD_XSF_COMP_BABBLE			(0x4U << 28)
#define BDCBD_XSF_COMP_STOPPED			(0x5U << 28)
#define BDCBD_XSF_COMP_SETUP_RECEIVED		(0x6U << 28)
#define BDCBD_XSF_COMP_DATA_START		(0x7U << 28)
#define BDCBD_XSF_COMP_STATUS_START		(0x8U << 28)

/* Control transfer BD specific fields */
#define BDCBD_XSF_DIR_IN (1 << 25)
#define	BD_TX_TYPE(p) ((p) << 16)
#define	BD_DATA_OUT	2
#define	BD_DATA_IN	3

#define BDCBD_XSF_TO_TFS(p) ((p) << 4)
#define BDCBD_XSF_SOT	(1U << 26)
#define BDCBD_XSF_EOT	(1U << 27)
#define BDCBD_XSF_ISP	(1U << 29)
#define BDCBD_XSF_IOC	(1U << 30)
#define BDCBD_XSF_SBF	(1U << 31)

/* Transfer BD fields */
#define BDCBD_XSF_TYPE_LTF (1 << 25)

#define	BDCBD_XSF_TYPE			0xF
#define BDCBD_XSF_TYPE_TRANSFER		0x0
#define BDCBD_XSF_TYPE_DATA_STAGE	0x1
#define BDCBD_XSF_TYPE_STATUS_STAGE	0x2
#define BDCBD_XSF_TYPE_CHAIN		0xF

#define BDCBD_XSF_INTR_TARGET(p) ((p) << 27)
typedef struct _bdc_bd_link {
	uint32 segment_low;
	uint32 segment_high;
	uint32 target;
	uint32 control;
} bdc_bd_link;

typedef struct _bdc_bd_transfer_event {
	uint32 buffer_low;
	uint32 buffer_high;
	uint32 length;
	uint32 flags;
} bdc_bd_transfer_event;

typedef struct _bdc_bd_command_event {
	uint32 command_low;
	uint32 command_high;
	uint32 status;
	uint32 flags;
} bdc_bd_command_event;

typedef struct _bdc_bd_generic {
	uint32 parameter_low;
	uint32 parameter_high;
	uint32 status;
	uint32 control;
} bdc_bd_generic;

typedef union _bdc_bd {
	bdc_bd_link link;
	bdc_bd_transfer_event transfer_event;
	bdc_bd_command_event command_event;
	bdc_bd_generic generic;
} bdc_bd;

#endif /* _BDCBD_H_ */
