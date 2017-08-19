/*
 * bdc controller
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

#ifndef _BDC_H_
#define _BDC_H_

#include <usbcontroller.h>
#include <bdcep0.h>
#include <bdcep.h>
#include <bdc_reg.h>
#include <bdc_shim.h>

#define BDC_STATUS_RING_EXP_MAX 5
#define BDC_STATUS_BD_MAX	(1 << (BDC_STATUS_RING_EXP_MAX + 1))
#define BDC_STATUS_RING_ALIGN	16
#define BDC_BD_PER_EP_MAX	8
/* BD can be aligned to 4 byte, but BDC in a CBL need to be aligned to 16, so let's keep it safe */
#define BDC_BD_ALIGN		16
#define BDC_SCRATCH_PAD_MAX	512
#define BDC_SCRATCH_PAD_ALIGN	16

#define BDC_PORT_MAX	1
#define BDC_EP_OUT_MAX	6
#define BDC_EP_IN_MAX	6

#define BDC_STATE_POWERED	0
#define BDC_STATE_DEFAULT	1
#define BDC_STATE_SAVED		2
#define BDC_STATE_RESET_PENDING 4

#define BDC_CONDITION_U2		0x01
#define BDC_CONDITION_NEED_SAVE		0x02
#define BDC_CONDITION_NEED_RESET	0x04

#define BDC_ATTRIBUTE_L1_RWE_STATUS_WA		0x1
#define BDC_ATTRIBUTE_L1_RWE_ENABLE_WA		0x2
#define BDC_ATTRIBUTE_SAVE_RESTORE_EN		0x4
#define BDC_ATTRIBUTE_BUS_RESET_HW_RESET_EN	0x8
#define BDC_ATTRIBUTE_REMOTE_WAKE_DURATION_EN	0x10

#define BDC_REMOTE_WAKE_DELAY_US (2500)

#define BDC_REMOTE_WAKE_DURATION_1002US		(15)
#define BDC_REMOTE_WAKE_DURATION_3000US		(0)
#define BDC_REMOTE_WAKE_DURATION_4000US		(1)
#define BDC_REMOTE_WAKE_DURATION_5000US		(2)
#define BDC_REMOTE_WAKE_DURATION_6000US		(3)
#define BDC_REMOTE_WAKE_DURATION_7000US		(4)
#define BDC_REMOTE_WAKE_DURATION_8000US		(5)
#define BDC_REMOTE_WAKE_DURATION_9000US		(6)
#define BDC_REMOTE_WAKE_DURATION_10000US	(7)
#define BDC_REMOTE_WAKE_DURATION_11000US	(8)
#define BDC_REMOTE_WAKE_DURATION_12000US	(9)
#define BDC_REMOTE_WAKE_DURATION_13000US	(10)

#define BDC_REMOTE_WAKE_DURATION_14997US	(11)
#define BDC_REMOTE_WAKE_DURATION_16000US	(12)
#define BDC_REMOTE_WAKE_DURATION_18000US	(13)
#define BDC_REMOTE_WAKE_DURATION_20000US	(14)

typedef struct _bdc_command {
	uint32 cmd;
	uint32 param0;
	uint32 param1;
	uint32 param2;
} bdc_command;


typedef struct _bdcd {
	/* First variable should be base class */
	usb_controller base;

	osl_t *osh;
	struct usbdev_chip *ubdc;

	/* Set the registers here */
	uint32 bdc_base_reg;

	usb_bdc_shim_regs_t *bdc_shim_regs;

	bdc_ep0 ep0;
	bdc_ep ep_in[BDC_EP_IN_MAX];
	bdc_ep ep_out[BDC_EP_OUT_MAX];

	bdc_bd_list event_list;

	uint8 speed;
	uint8 address;
	uint8 max_in_ep;
	uint8 max_out_ep;

	uint8 state;
	uint8 cmd_index;
	uint16 condition;

	uint16 attribute;
	uint8 tdrsmup_value;
	uint8 reserved2;
} bdcd;

typedef void (*command_completed)(bdcd *bdc, bdc_bd *cmd);

typedef struct _bdc_command_trb {
	command_completed callback;
} bdc_command_trb;

void bdc_init(bdcd *bdc);
void bdc_create(bdcd *bdc);

/* Functions used for transport detection */
void bdc_init_hardware(usb_controller *controller);
void bdc_connect_to_host(usb_controller *controller);
void bdc_disconnect_from_host(usb_controller *controller);
void bdc_bus_reset(usb_controller *controller);
void bdc_interrupt_handler(usb_controller *controller);
void bdc_power_on(bdcd *bdc);
void bdc_power_off(bdcd *bdc);
void bdc_start_controller(usb_controller *controller);
void bdc_stop_controller(usb_controller *controller);

void bdc_adapter_init(void);
bool bdc_issue_command(bdcd *bdc, bdc_command *cmd);
void bdc_setup_received(bdcd *bdc, bdc_bd *evt);
void bdc_set_address(bdcd *bdc, uint8 address);
#ifdef BDC_DEBUG
void bdc_dump_regs(usb_controller *controller);
#endif

typedef void TRAN_ISR_CALLBACK_HANDLER (void);

#define bdc_attribute(bdccontroller) ((bdccontroller)->attribute)

extern uint8 bdc_status_bd[BDC_STATUS_BD_MAX * sizeof(bdc_bd)];

#endif /* _BDC_H_ */
