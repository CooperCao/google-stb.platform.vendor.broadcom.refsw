/*
 * BDC is USB3 Device Controller.
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <hndpmu.h>
#include <osl.h>
#include <osl_ext.h>
#include <hndsoc.h>
#include <bdc.h>
#include <bdc_shim.h>
#include <bdcbdlist.h>
#include <bdc_usb.h>
#include <bdc_rte.h>
#include <usbclass.h>
#include <usbdevice.h>
#include <usbplatform.h>
#include <usbcontroller.h>

/*******************************************************************************
* Defines
******************************************************************************
*/

/*******************************************************************************
* Function Prototypes
******************************************************************************
*/
void bdc_set_remote_wake_duration(bdcd *bdc);
void bdc_save_bdc_state(bdcd *bdc);
void bdc_restore_bdc_state(bdcd *bdc);
void bdc_set_address(bdcd *bdc, uint8 address);
void bdc_start(usb_controller *controller);
void bdc_stop(usb_controller *controller);
bool bdc_check_pending_transfers(bdcd *bdc);
void bdc_port_status_changed(bdcd *bdc, bdc_bd *evt);
void bdc_command_completed(bdcd *bdc, bdc_bd *evt);
void bdc_setup_received(bdcd *bdc, bdc_bd *evt);
bdc_ep * bdc_get_ep_by_context_index(bdcd *bdc, uint32 index);
void bdc_transfer_event(bdcd *bdc, bdc_bd *evt);
void bdc_interrupt_handler(usb_controller *controller);
void bdc_setup_complete(usb_controller *controller, bdc_status_code status);
void bdc_bus_reset(usb_controller *controller);
void bdc_remote_wake(usb_controller *controller);
bool bdc_is_remote_wake_enabled(usb_controller *controller);
void bdc_clear_connect_status(void);
void _bdc_connect_to_host(usb_controller *controller);
void _bdc_disconnect_from_host(usb_controller *controller);
void bdc_transceiver_suspend_control(usb_controller *controller, bool connected);
void bdc_adapter_init(void);

usb_endpoint * bdc_ep_create(usb_controller *controller, endpoint_usage usage);
void bdc_ep_destroy(usb_controller *controller, usb_endpoint *ep);
void bdc_start(usb_controller *controller);
void bdc_stop(usb_controller *controller);
void bdc_remote_wake(usb_controller *controller);
void bdc_setup_complete(usb_controller *controller, bdc_status_code status);
bool bdc_is_remote_wake_enabled(usb_controller *controller);
void bdc_set_wakeup_source(usb_controller *controller);
/*
 * void bdc_pre_sleep_processing(usb_controller *controller, pmu_sleep_t sleep_state);
 * void bdc_post_sleep_processing(usb_controller *controller, pmu_sleep_t sleep_state);
 */
void bdc_send_raw_event(usb_controller *controller, uint8 *data, uint32 length);

/*******************************************************************************
* Local Declarations
******************************************************************************
*/
uint32 bdc_initialized = 0;

__attribute__ ((aligned(BDC_SCRATCH_PAD_ALIGN))) uint8 bdc_scratch_pad_buffers[BDC_SCRATCH_PAD_MAX];

/* EP OUT before EP IN in BD Array */
__attribute__ ((aligned(BDC_STATUS_RING_ALIGN)))
	uint8 bdc_status_bd[BDC_STATUS_BD_MAX * sizeof(bdc_bd)];
__attribute__ ((aligned(BDC_BD_ALIGN))) uint8 bdc_bd_ep0[BDC_BD_PER_EP_MAX * sizeof(bdc_bd)];
__attribute__ ((aligned(BDC_BD_ALIGN))) uint8 bdc_bd_ep[(BDC_EP_OUT_MAX + BDC_EP_IN_MAX) *
	BDC_BD_PER_EP_MAX * sizeof(bdc_bd)];

static const usb_controller_function_table bdc_function_table_rom =
{
	bdc_ep_create,	/* ep_create */
	bdc_ep_destroy,	/* ep_destroy */
	bdc_start,	/* start */
	bdc_stop,	/* stop */
	usbcontroller_enable,	/* enable */
	usbcontroller_disable,	/* disable */
	usbcontroller_reset,	/* reset */
	bdc_bus_reset,	/* bus_reset */
	bdc_remote_wake,	/* remote_wake */
	usbcontroller_update_resume,	/* update_resume */
	bdc_setup_complete,	/* setup_complete */
	bdc_interrupt_handler,	/* interrupt_handler */
	usbcontroller_get_port,	/* get_port */
	usbcontroller_data_sent,	/* data_sent */
	bdc_is_remote_wake_enabled,	/* is_remote_wake_enabled */
	bdc_set_wakeup_source,	/* set_wakeup_source */
	/*
	 * bdc_pre_sleep_processing, // pre_sleep_processing
	 * bdc_post_sleep_processing, // post_sleep_processing
	 */
	bdc_send_raw_event,	/* send_raw_event */
	usbcontroller_suspended,	/* suspended */
	usbcontroller_transfer_start,	/* transfer_start */
	usbcontroller_data_received,	/* data_received */
	usbcontroller_transfer_complete,	/* transfer_complete */
	usbcontroller_ep_cancel_all_transfer,	/* ep_cancel_all_transfer */
	/* usbcontroller_add_sof_call_back, // add_sof_call_back */
	_bdc_connect_to_host,	/* connect_to_host */
	NULL, /* _bdc_disconnect_from_host, // disconnect_from_host */
	NULL, /* usbcontroller_feature, // feature */
};

/*******************************************************************************
* Global Declarations
******************************************************************************
*/

/*
 * Translate between remote wake duration in ms to TDRSMUP value
 * Round value to next highest possibility
 */
static const uint8 bdc_remote_wake_translation[] =
{
	BDC_REMOTE_WAKE_DURATION_1002US,	/* 0ms */
	BDC_REMOTE_WAKE_DURATION_1002US,	/* 1ms */
	BDC_REMOTE_WAKE_DURATION_3000US,	/* 2ms */
	BDC_REMOTE_WAKE_DURATION_3000US,	/* 3ms */
	BDC_REMOTE_WAKE_DURATION_4000US,	/* 4ms */
	BDC_REMOTE_WAKE_DURATION_5000US,	/* 5ms */
	BDC_REMOTE_WAKE_DURATION_6000US,	/* 6ms */
	BDC_REMOTE_WAKE_DURATION_7000US,	/* 7ms */
	BDC_REMOTE_WAKE_DURATION_8000US,	/* 8ms */
	BDC_REMOTE_WAKE_DURATION_9000US,	/* 9ms */
	BDC_REMOTE_WAKE_DURATION_10000US,	/* 10ms */
	BDC_REMOTE_WAKE_DURATION_11000US,	/* 11ms */
	BDC_REMOTE_WAKE_DURATION_12000US,	/* 12ms */
	BDC_REMOTE_WAKE_DURATION_13000US,	/* 13ms */
	BDC_REMOTE_WAKE_DURATION_14997US,	/* 14ms */
	BDC_REMOTE_WAKE_DURATION_14997US,	/* 15ms */
	BDC_REMOTE_WAKE_DURATION_16000US,	/* 16ms */
	BDC_REMOTE_WAKE_DURATION_18000US,	/* 17ms */
	BDC_REMOTE_WAKE_DURATION_18000US,	/* 18ms */
	BDC_REMOTE_WAKE_DURATION_20000US,	/* 19ms */
	BDC_REMOTE_WAKE_DURATION_20000US	/* 20ms */
};

/*******************************************************************************
* Function Definitions
******************************************************************************
*/

void bdc_interrupt_handler(usb_controller *controller);
void bdc_command_completed(bdcd *bdc, bdc_bd *evt);
void bdc_port_status_changed(bdcd *bdc, bdc_bd *evt);

/* TODO: This is funciton will not work as tdrsmup_value is not set */
void
bdc_set_remote_wake_duration(bdcd *bdc)
{
	bdc_fn(("%s\n", __FUNCTION__));

	/* TODO: Remove wake is disabled for now; need to enable this code */
}

void
bdc_set_wakeup_source(usb_controller *controller)
{
	/* pmu_set_wakeup_interrupt_source2(PMU_SLEEP_WAKE_USB3); */
}

void
bdc_send_raw_event(usb_controller *controller, uint8 *data, uint32 length)
{
	/* bdcd* bdc = (bdcd*)controller; */

	/* usbpolling_send_event_data(bdc->polling, data, length); */
}

#ifdef BDC_DEBUG
void
bdc_dump_regs(usb_controller *controller)
{
	uint32 rdval, orig_coreid;

	bdcd *bdc = (bdcd *)controller;

	bdc_fn(("%s\n", __FUNCTION__));

	bdc_err(("BDC register dump\n"));

	rdval = R_REG(bdc->osh, &(bdc->bdc_shim_regs->devcontrol));
	bdc_err(("DevControl Offset 0x000 : 0x%0x \n", rdval));

	rdval = R_REG(bdc->osh, &(bdc->bdc_shim_regs->devstatus));
	bdc_err(("DevStatus Offset 0x004 : 0x%0x \n", rdval));

	rdval = R_REG(bdc->osh, &(bdc->bdc_shim_regs->intrstatus));
	bdc_err(("InterruptStatus Offset 0x048 : 0x%0x \n", rdval));

	rdval = R_REG(bdc->osh, &(bdc->bdc_shim_regs->intrmaskwlan));
	bdc_err(("InterruptMastWLAN Offset 0x04c : 0x%0x \n", rdval));

	rdval = R_REG(bdc->osh, &(bdc->bdc_shim_regs->clkctlstatus));
	bdc_err(("CtlCtrlStatus Offset 0x01e0 : 0x%0x \n", rdval));

	rdval = R_REG(bdc->osh, &(bdc->bdc_shim_regs->phyutmictl1));
	bdc_err(("PhyUtmiCtl1 Offset 0x310 : 0x%0x \n", rdval));

	orig_coreid = si_coreid(bdc->ubdc->sih);

	si_setcore(bdc->ubdc->sih, USB30D_CORE_ID, 0);

	rdval = si_wrapperreg(bdc->ubdc->sih, AI_IOCTRL, 0, 0);
	bdc_err(("CoreControl Offset 0x%x : 0x%0x \n", AI_IOCTRL, rdval));

	rdval = si_wrapperreg(bdc->ubdc->sih, AI_IOSTATUS, 0, 0);
	bdc_err(("CoreStatus Offset 0x%x : 0x%0x \n", AI_IOSTATUS, rdval));

	si_setcoreidx(bdc->ubdc->sih, orig_coreid);

	return;
}
#endif /* BDC_DEBUG */

usb_endpoint *
bdc_ep_create(usb_controller *controller, endpoint_usage usage)
{
	bdc_ep *ep;
	bdc_ep0 *ep0;
	bdcd *bdc;
	uint8 address;

	bdc = (bdcd *)controller;

	bdc_fn(("%s\n", __FUNCTION__));

	switch (usage) {
	case ENDPOINT_USAGE_CONTROL:
		ep0 = &bdc->ep0;
		bdcep0_init(ep0, controller);
		return (usb_endpoint *)ep0;

	case ENDPOINT_USAGE_INTERRUPT_IN:
		address = BDCEP_INTERRUPT_IN;
		break;

	case ENDPOINT_USAGE_BULK_IN:
		address = BDCEP_BULK_IN;
		break;

	case ENDPOINT_USAGE_BULK_OUT:
		address = BDCEP_BULK_OUT;
		break;

	case ENDPOINT_USAGE_ISOC_IN:
		address = BDCEP_ISOC_IN;
		break;

	case ENDPOINT_USAGE_ISOC_OUT:
		address = BDCEP_ISOC_OUT;
		break;

	case ENDPOINT_USAGE_DEBUG_BULK_IN:
		address = BDCEP_DEBUG_BULK_IN;
		break;

	case ENDPOINT_USAGE_DEBUG_BULK_OUT:
		address = BDCEP_DEBUG_BULK_OUT;
		break;

	case ENDPOINT_USAGE_KEYBOARD_IN:
		address = BDCEP_KEYBOARD_IN;
		break;

	case ENDPOINT_USAGE_MOUSE_IN:
		address = BDCEP_MOUSE_IN;
		break;

	default:
		return NULL;
	}

	if (address & USB_DIR_DEVICE_TO_HOST) {
		ep = &bdc->ep_in[(address & 0x0F) - 1];
	} else {
		ep = &bdc->ep_out[address - 1];
	}

	bdcep_init(ep, controller);

	return (usb_endpoint *)ep;
}

void
bdc_ep_destroy(usb_controller *controller, usb_endpoint *ep)
{
	bdc_fn(("%s\n", __FUNCTION__));

	if ((ep->address & 0xF) == 0) {
		bdcep0_un_config((bdc_ep0 *)ep, (bdcd *)controller);
	} else {
		if ((((bdc_ep *)ep)->status & BDCEP_STATUS_ENABLED) == 0) {
			return;
		}

		bdcep_un_config((bdc_ep *)ep, (bdcd *)controller);
	}

	usbcontroller_ep_destroy(controller, ep);
}

void
bdc_stop_controller(usb_controller *controller)
{
	uint32 value;
	bdcd *bdc = (bdcd *)controller;

	bdc_fn(("%s\n", __FUNCTION__));

	/* Stop Controller if it is already running */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	if (BDCREG_SC_STATUS_NORMAL == (value & BDCREG_SC_STATUS_MASK))	{
		bdc_dbg(("bdc_start: bdcsc=%x\n", value));
		value &= ~BDCREG_SC_COP_MASK;
		value |= BDCREG_SC_COP_STOP | BDCREG_SC_COS;
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
		do {
			value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
		} while (BDCREG_SC_STATUS_HALT != (value & BDCREG_SC_STATUS_MASK));

		bdc_dbg(("bdc_start: bdcsc=%x\n", value));
		value &= ~BDCREG_SC_COP_MASK;
		value |= BDCREG_SC_COP_RESET | BDCREG_SC_COS;
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
		do {
			value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
		} while (BDCREG_SC_STATUS_BUSY == (value & BDCREG_SC_STATUS_MASK));

		if (BDCREG_SC_STATUS_NORMAL == (value & BDCREG_SC_STATUS_MASK))	{
			bdc_dbg(("bdc_start: bdcsc=%x\n", value));
			value &= ~BDCREG_SC_COP_MASK;
			value |= BDCREG_SC_COP_STOP | BDCREG_SC_COS;
			W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
			do {
				value =
					R_REG(bdc->osh,
						(uint32 *)(bdc->bdc_base_reg +
						UBDC_BDCSC_ADR));
			} while (BDCREG_SC_STATUS_HALT != (value & BDCREG_SC_STATUS_MASK));
		}
	}

}

void
bdc_start_controller(usb_controller *controller)
{
	uint32 value;
	bdcd *bdc = (bdcd *)controller;

	bdc_fn(("%s\n", __FUNCTION__));

	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	bdc_dbg(("BDCSC Initial: %04x\n", value));
	value &= ~BDCREG_SC_COP_MASK;
	value |= BDCREG_SC_COP_RUN | BDCREG_SC_COS | BDCREG_SC_GIE;
	/* value = BDCREG_SC_COP_RUN | BDCREG_SC_COS | BDCREG_SC_GIE; */
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);

	do {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	} while ((value & BDCREG_SC_STATUS_MASK) != BDCREG_SC_STATUS_NORMAL);
	bdc_dbg(("BDCSC Normal: %04x\n", value));

	do {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR));
	} while (!(value & BDCREG_USPSC_VBUS));
	bdc_dbg(("BDCSC Vbus detected: %04x\n", value));

	/* Set USB is detected */
	bdc->attribute |= USBCONTROLLER_ATTRIBUTE_TRAN_DETECT;

}

/*
 * ! \internal
 * ! Initialize hardware settings for BDC
 * !  Interrupts are not enabled because we either want SOF interrupt for
 * !  transport detection or regular BDC interrupts for normal operation
 */
void
bdc_init_hardware(usb_controller *controller)
{
	bdcd *bdc = (bdcd *)controller;
#ifdef BDC_DEBUG
	uint32 value;

	bdc_fn(("%s\n", __FUNCTION__));

	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCCFG0_ADR));
	bdc_dbg(("BCD Spec %x.%x, No of Int %d, Page Size %d KB, scratchpad size %d\n",
		(int)(value & 0xFF000000) >> 24,
		(int)(value & 0x00FF0000) >> 16,
		(int)((value & 0x0000F100) >> 11) + 1,
		(int)(1 << ((value & 0x00000700) >> 8)),
		(int)(value & 0x00000007) << 6));
#endif /* BDC_DEBUG */

	bdc_stop_controller((usb_controller *)bdc);

	/* Initialize scratchpad area */
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SPBBAL_ADR),
			(uint32)bdc_scratch_pad_buffers);
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SPBBAH_ADR), 0);

	/*
	 * CFG0[2:0] == 2 means scratchpad is 128 bytes
	 * CFG0[2:0] == 5 means scratchpad is 1024 bytes
	 * value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCCFG0_ADR));
	 * bdc_dbg(("bdc_init_hardware scratchpad size %x", value));
	 */
}

bool
bdc_issue_command(bdcd *bdc, bdc_command *cmd)
{
	uint32 value;

	bdc_fn(("%s\n", __FUNCTION__));

	while (TRUE) {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_CMDSC_ADR));
		if (BDCREG_CMD_CST_BUSY == BDCREG_CMD_CST(value)) {
			continue;
		}

		bdc_dbg(("bdc_issue_command: %x cmd %x sub-cmd %x cid %x \n",
			cmd->cmd, (cmd->cmd & 0xF), ((cmd->cmd >> 17) & 0xF),
			BDCREG_CMD_CID(bdc->cmd_index)));

		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_CMDPAR0_ADR), cmd->param0);
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_CMDPAR1_ADR), cmd->param1);
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_CMDPAR2_ADR), cmd->param2);
		W_REG(bdc->osh,
			(uint32 *)(bdc->bdc_base_reg + UBDC_CMDSC_ADR),
			cmd->cmd | BDCREG_CMD_CID(bdc->cmd_index) | BDCREG_CMD_CWS);
		bdc->cmd_index = (bdc->cmd_index + 1) & BDCREG_CMD_MAX_MASK;

		break;
	}

	return TRUE;
}

void
bdc_save_bdc_state(bdcd *bdc)
{
	uint32 value;
	int i;
	bdc_ep *ep;
	bdc_command cmd = {
		0, 0, 0, 0
	};
	usb_controller *controller = (usb_controller *)bdc;

	bdc_fn(("%s\n", __FUNCTION__));

	/* Do not enter BDC save if suspend was interrupted */
	if (((usb_controller *)bdc)->device->usbd_current_status & USBDEVICE_STATUS_RESUMING) {
		bdc->state = BDC_STATE_DEFAULT;
		return;
	}

	/* Stop all endpoints */
	if (((usb_controller *)bdc)->device->usbd_current_status & USBDEVICE_STATUS_ENUMERATED) {
		for (i = BDC_EP_IN_MAX - 1; i >= 0; i--) {
			ep = &((bdcd *)controller)->ep_in[i];
			if (ep->status)	{
				cmd.cmd = BDCREG_CMD_CSA_XSD | BDCREG_CMD_EPO |
					BDCREG_SUB_CMD_EP_STOP | BDCREG_CMD_EPN(ep->index);
				bdc_issue_command(bdc, &cmd);
			}
		}
		for (i = BDC_EP_OUT_MAX - 1; i >= 0; i--) {
			ep = &((bdcd *)controller)->ep_out[i];
			if (ep->status) {
				cmd.cmd = BDCREG_CMD_EPO | BDCREG_SUB_CMD_EP_STOP | BDCREG_CMD_EPN(
						ep->index);
				bdc_issue_command(bdc, &cmd);
			}
		}
	}

	/* Stop controller */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	value &= ~BDCREG_SC_COP_MASK;
	value |= BDCREG_SC_COP_STOP | BDCREG_SC_COS;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
	do {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	} while ((value & BDCREG_SC_STATUS_MASK) != BDCREG_SC_STATUS_HALT);

	/* Process any remaining SRRINTs */
	bdc_interrupt_handler(controller);

	/* Setup scratchpad */
	memset(bdc_scratch_pad_buffers, 0, BDC_SCRATCH_PAD_MAX);
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SPBBAL_ADR),
			(uint32)bdc_scratch_pad_buffers);
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SPBBAH_ADR), 0);

	/* Save controller state */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	value &= ~BDCREG_SC_COP_MASK;
	value |= BDCREG_SC_COP_SAVE | BDCREG_SC_COS;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
	do {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	} while ((value & BDCREG_SC_STATUS_MASK) == BDCREG_SC_STATUS_BUSY);


	/* Set pup_act_HUSB_DP_mux to 0, for FW control */
	/*
	 * AND_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
	 * ~USB_PUP_ACT_HUSB_DP_MUX);
	 * TODO: CHECK THIS FUNCTION
	 */
	bdci_dev_reset(bdc->ubdc);

	/* Put bdc in reset */
	/*
	 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_peri_adr), (1 << 16));
	 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_adr), (1 << 11));
	 */

	/*
	 * Put bdc in reset
	 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_peri_adr), (1 << 16));
	 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_adr), (1 << 11));
	 */

	bdc->state = BDC_STATE_SAVED;
}

void
bdc_restore_bdc_state(bdcd *bdc)
{
	uint32 value;
	int i;
	bdc_ep *ep;
	usb_controller *controller = (usb_controller *)bdc;

	bdc_fn(("%s\n", __FUNCTION__));

	/*
	 * Take bdc out of reset
	 * AND_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_peri_adr), ~(1 <<
	 * 16));
	 * AND_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_adr), ~(1 << 11));
	 */

	/* Set scratchpad registers */
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SPBBAL_ADR),
			(uint32)bdc_scratch_pad_buffers);
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SPBBAH_ADR), 0);

	/* Restore controller state */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	value &= ~BDCREG_SC_COP_MASK;
	value |= BDCREG_SC_COP_RESTORE | BDCREG_SC_COS;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
	do {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	} while ((value & BDCREG_SC_STATUS_MASK) == BDCREG_SC_STATUS_BUSY);

	/*
	 * Set pup_act_HUSB_DP_mux to 1, for HW control
	 * TODO:
	 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
	 * USB_PUP_ACT_HUSB_DP_MUX);
	 */

	/* Enable interrupts and HLE again */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	value &= ~BDCREG_SC_COP_MASK;
	value |= BDCREG_SC_GIE;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
	/*
	 * TODO:
	 * if (USB_BT_bos_des[USB_LPM_DESC_OFFSET+USB_LPM_BMATTRIB_OFFSET] &
	 * USB_LPM_SUPPORTED_BIT)
	 */
	{
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR));
		value |= BDCREG_SC_HLE;
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR), value);
	}

	/* Run controller */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	value &= ~BDCREG_SC_COP_MASK;
	value |= BDCREG_SC_COP_RUN | BDCREG_SC_COS;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
	do {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	} while ((value & BDCREG_SC_STATUS_MASK) != BDCREG_SC_STATUS_NORMAL);

	/* Resume/ring all OUT endpoints */
	if (((usb_controller *)bdc)->device->usbd_current_status & USBDEVICE_STATUS_ENUMERATED) {
		for (i = BDC_EP_OUT_MAX - 1; i >= 0; i--) {
			ep = &((bdcd *)controller)->ep_out[i];
			if (ep->status)	{
				W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_XSFNTF_ADR),
						ep->index);
			}
		}
	}

	/* Check port status manually */
	bdc_port_status_changed(bdc, 0);

	bdc_set_remote_wake_duration(bdc);

}

void
bdc_set_address(bdcd *bdc, uint8 address)
{
	bdc_command cmd = {
		0, 0, 0, 0
	};

	bdc_fn(("%s\n", __FUNCTION__));

	cmd.param2 = address;
	cmd.cmd = BDCREG_CMD_DEVICE_CONFIGURATION | BDCREG_SUB_CMD_DEVICE_ADDRESS;

	bdc_issue_command(bdc, &cmd);

	bdc->state = BDC_STATE_DEFAULT;

	if (bdc->ep0.status & BDCEP0_STATUS_SET_ADDRESS) {
		bdc->ep0.status &= ~BDCEP0_STATUS_SET_ADDRESS;
		bdcep0_status_start(&bdc->ep0, bdc);
	}
}

void
bdc_start(usb_controller *controller)
{
	bdcd *bdc;
	uint32 value;

	bdc_fn(("%s\n", __FUNCTION__));

	/* Autosensing will initialize BDC controller. No need to do it again from usb_initialize */
	if (bdc_initialized) {
		bdc_interrupt_handler(controller);
		return;
	}

	/* dbguart_print("bdc_start\n\r"); */
	bdc_initialized = 1;

	/* BDC hardware already brought out of reset in attach phase * / */
	bdc = (bdcd *)controller;

	if ((controller->attribute & USBCONTROLLER_ATTRIBUTE_TRAN_DETECT) == 0) {
		bdc_init_hardware((usb_controller *)bdc);
	}

#ifndef RTE_POLL
	/* Enable BDC interrupts by setting shim */
	bdci2wlan_irq_on(bdc->ubdc);
#endif
	/* TODO: Remove 3 lines and the line comment below once soft connect works. */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR));
	value &= ~BDCREG_SC_HLE;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR), value);
	/* TODO: Enable if the above line is removed */
	OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR), BDCREG_SC_HLE);

	/* Turn off the debouncer */
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCCFG0_ADR + BDC_IRAADR_OFFSET),
		0xd000);
	/* TODO: Find the correct value of BDC_IRADAT_OFFSET */
	value = R_REG(bdc->osh,
		(uint32 *)(bdc->bdc_base_reg + UBDC_BDCCFG0_ADR + BDC_IRADAT_OFFSET));
	value |= 0x00001000;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCCFG0_ADR + BDC_IRAADR_OFFSET),
			0xd000);
	W_REG(bdc->osh,
		(uint32 *)(bdc->bdc_base_reg + UBDC_BDCCFG0_ADR + BDC_IRADAT_OFFSET), value);

	/* Initialize status ring area */
	bdcbdlist_init(&bdc->event_list, (bdc_bd *)bdc_status_bd, BDC_STATUS_BD_MAX, 0,
			BDCBDLIST_TYPE_SR);

	/* Initialize pointers to current values.  We have no processed any SRs yet */
	bdc->event_list.enqueue =
		BDCREG_SRR_EPI(R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR)));
	bdc->event_list.dequeue =
		BDCREG_SRR_DPI(R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR)));
	bdc_dbg(("BDC Enqueue %d Dequeue %d\n", bdc->event_list.enqueue, bdc->event_list.dequeue));

	value = (uint32)bdc_status_bd;
	value |= BDC_STATUS_RING_EXP_MAX;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRBAL0_ADR), value);
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRBAH0_ADR), 0);

	/* Enable interrupts */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR));
	value |= BDCREG_SRR_IE;
	value &= ~(BDCREG_SRR_RWS | BDCREG_SRR_RST);
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR), value);
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR));
	bdc_dbg(("BDC SRR INT0 %04x\n", value));

	bdc->max_out_ep = BDC_EP_OUT_MAX;
	bdc->max_in_ep = BDC_EP_IN_MAX;

	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_INTCLS0_ADR));
	value &= ~0xFFFF;
	/* value |= 2000; */
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_INTCLS0_ADR), value);

	if ((controller->attribute & USBCONTROLLER_ATTRIBUTE_TRAN_DETECT) == 0) {
		bdc_start_controller(controller);
		bdc_connect_to_host(controller);
	}

	/* Need to set RWE bit for older hardware */
	if (bdc_attribute(bdc) & BDC_ATTRIBUTE_L1_RWE_ENABLE_WA) {
		OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR), BDCREG_SC_RWE);
	}
#ifdef USB_BDC_L1_NYET_WA
	OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_ECR0_ADR), (1 << 30));
#endif
	/*
	 * TODO: THIS FUNCTION will not work since default wake value is not set
	 * bdc_set_remote_wake_duration(bdc);
	 */
}

void
bdc_stop(usb_controller *controller)
{
	uint32 value;
	bdcd *bdc = (bdcd *)controller;

	bdc_fn(("%s\n", __FUNCTION__));

	/* Stop controller */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	value &= ~BDCREG_SC_COP_MASK;
	value |= BDCREG_SC_COP_STOP | BDCREG_SC_COS;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR), value);
	do {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	} while ((value & BDCREG_SC_STATUS_MASK) != BDCREG_SC_STATUS_HALT);

	/* interrupt_clear_pending_state(INTERRUPT_USB3 ); */


/* Put bdc in reset */
	/*
	 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_peri_adr), (1 << 16));
	 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + cr_level_reset_adr), (1 << 11));
	 */
}

bool
bdc_check_pending_transfers(bdcd *bdc)
{
	uint32 i;

	bdc_fn(("%s\n", __FUNCTION__));

	if ((bdc->ep0.status & BDCEP0_STATUS_DATA_STAGE) ||
	    (bdc->ep0.status & BDCEP0_STATUS_STATUS_STAGE)) {
		/* If there's a stall, DATA_STAGE does not clear. */
		if (!(bdc->ep0.status & BDCEP0_STATUS_STALL)) {
			return TRUE;
		}
	}

	for (i = 0; i < bdc->max_in_ep; i++) {
		if (bdc->ep_in[i].status & BDCEP_STATUS_ENABLED && bdc->ep_in[i].base.urb_list) {
			/* For now, skip ISOC ep because ep->status is not used */
			if (!(bdc->ep_in[i].base.urb_list->attribute & URB_ATTRIBUTE_UNLIMITED)) {
				return TRUE;
			}
		}
	}


/*
 *      for (i = 0; i < bdc->max_out_ep; i++)
 *      {
 *              if (bdc->ep_in[i].status & BDCEP_STATUS_ENABLED && bdc->ep_in[i].base.urb_list)
 *              {
 *                      return TRUE;
 *              }
 *      }
 */
	return FALSE;
}

void
bdc_port_status_changed(bdcd *bdc, bdc_bd *evt)
{
	uint32 value;
	uint32 speed;
	uint32 link_state;
	uint32 old_value;
	usb_device_t *device = ((usb_controller *)bdc)->device;

	bdc_fn(("%s\n", __FUNCTION__));

	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR));

	bdc_dbg(("bdc_port_status_changed:USPSC=%x\n", value));
	W_REG(bdc->osh,
		(uint32 *)(bdc->bdc_base_reg +
		UBDC_USPSC_ADR), value & (~BDCREG_USPSC_USPSC_RW));
	bdc_dbg(("bdc_port_status_changed:port=%x\n", value));

	if ((value & BDCREG_USPSC_PCC) && (value & BDCREG_USPSC_VBUS)) {
		if ((value & BDCREG_USPSC_PCS) && (BDCREG_USPSC_PST(value) == 0)) {
			if (BDC_STATE_DEFAULT == bdc->state) {
				/* Disconnect occurred which hardware could not detect */
				device->ft->disconnected(device);
			}

			speed = BDCREG_USPSC_PSP(value);
			bdc->speed = (uint8)speed;
			device->ft->connected(device, (uint8)speed);
			bdc_dbg(("bdc_port_status_changed: connect status=%x\n", value));
		} else if ((value & BDCREG_USPSC_PRS) == 0) {
			/* CCS is not reliable */
			device->ft->disconnected(device);
			bdc_dbg(("bdc_port_status_changed: disconnect status=%x\n", value));
		}
	}

	if (value & BDCREG_USPSC_PRC) {
		/* Start of bus reset */
		if (value & BDCREG_USPSC_PRS) {
			if ((bdc->state & BDC_STATE_RESET_PENDING) == 0) {
				if (bdc_attribute(bdc) & BDC_ATTRIBUTE_BUS_RESET_HW_RESET_EN) {
					bdc->condition |= BDC_CONDITION_NEED_RESET;
					bdc->state |= BDC_STATE_RESET_PENDING;
				}
			}
		} else {
			bdc_dbg(("bdc_port_status_changed: reset status=%x\n", value));
			bdc->state = BDC_STATE_POWERED;
			((usb_controller *)bdc)->ft->bus_reset((usb_controller *)bdc);
			bdc->state &= ~BDC_STATE_RESET_PENDING;
		}
	}

	if ((value & BDCREG_USPSC_PSC) && (value & BDCREG_USPSC_PCS)) {
		bdc_dbg(("bdc_port_status_changed: link status=%x\n", value));
		link_state = BDCREG_USPSC_PST(value);
		if (BDC_LINK_STATE_U2 == link_state) {
			bdc->condition |= BDC_CONDITION_U2;
		} else {
			bdc->condition &= ~BDC_CONDITION_U2;
		}

		if (BDC_LINK_STATE_U0 == link_state) {
			old_value = ((usb_controller *)bdc)->device->usbd_current_status;

			if (((usb_controller *)bdc)->device->usbd_current_status &
			     USBDEVICE_STATUS_SUSPENDED) {
				usbcontroller_resume_completed((usb_controller *)bdc);
			}
			((usb_controller *)bdc)->device->usbd_current_status &=
				~(USBDEVICE_STATUS_RESUMING | USBDEVICE_STATUS_SUSPENDED);

			bdc_dbg(("bdc_port_status_changed: old_value=%x\n", old_value));
			if (old_value & USBDEVICE_STATUS_RESUMING) {
				/* usb_send_message_to_thread(USB_MSG_WORK); */
			}
		} else if (BDC_LINK_STATE_U2 == link_state)   {
			if (bdc_check_pending_transfers(bdc)) {
				((usb_controller *)bdc)->ft->remote_wake((usb_controller *)bdc);
			}
#if USB3_LPM_REMOTE_WAKE_TEST

			utilslib_delay_us(5000);
			((usb_controller *)bdc)->ft->remote_wake((usb_controller *)bdc);
#endif
		} else if (BDC_LINK_STATE_U3 == link_state)   {
			if (!(((usb_controller *)bdc)->device->usbd_current_status &
			      USBDEVICE_STATUS_SUSPENDED)) {
				bdc_dbg(("bdc_port_status_changed: uspsc=%x\n", value));
				if (bdc_attribute(bdc) & BDC_ATTRIBUTE_SAVE_RESTORE_EN)	{
					if (bdc->state != BDC_STATE_POWERED) {
						bdc->condition |= BDC_CONDITION_NEED_SAVE;
					}
				}
				/* osapi_activate_timer(&bdc_remote_wake_timer,
				 * BDC_REMOTE_WAKE_DELAY_US);
				 */
				device->ft->bus_suspended(device);
			}
		} else if (BDC_LINK_STATE_RESUME == link_state)	 {
			((usb_controller *)bdc)->device->usbd_current_status |=
				USBDEVICE_STATUS_RESUMING;
		}
	}
}

void
bdc_command_completed(bdcd *bdc, bdc_bd *evt)
{
}

void
bdc_setup_received(bdcd *bdc, bdc_bd *evt)
{
	usb_endpoint0 *ep0;
	uint8 *cp;
	int i;
	usb_device_t *device = ((usb_controller *)bdc)->device;

	bdc_fn(("%s\n", __FUNCTION__));

	ep0 = (usb_endpoint0 *)&bdc->ep0;
	cp = (uint8 *)&evt->transfer_event.buffer_low;
	cp += 7;
	for (i = 7; i >= 0; i--) {
		ep0->setup[i] = *cp--;
	}

	if (bdc->ep0.status & BDCEP0_STATUS_STALL) {
		/* Clear all other bits but set pending reset bit */
		bdc->ep0.status = BDCEP0_STATUS_PENDING_RESET;
	} else {
		bdc->ep0.status = 0;
	}

	/* Handle Set_address command here */
	cp++;

	bdc_dbg(("bdc_setup_received: %x%x%x%x\n", cp[0], cp[1], cp[2], cp[3]));
	if (cp[0] == 0 && USB_STD_SET_ADDRESS_REQ_VALUE == cp[1])
	{
		bdc->address = cp[2];
		bdc_dbg(("bdc_setup_received: BDCEP0_STATUS_SET_ADDRESS %x\n", bdc->address));
		((bdc_ep0 *)ep0)->status |= BDCEP0_STATUS_COMPLETED | BDCEP0_STATUS_SET_ADDRESS;

		return;
	}

	bdc_dbg(("bdc_setup_received: setup: %x, %x, %x, %x \n",
		ep0->setup[0], ep0->setup[1], ep0->setup[2], ep0->setup[3]));

	device->ft->setup(device);

}

bdc_ep *
bdc_get_ep_by_context_index(bdcd *bdc, uint32 index)
{
	int is_in;
	int i;

	/* ASSERT(index > 1); */
	bdc_fn(("%s\n", __FUNCTION__));

	is_in = index & B_0;
	i = (index >> 1) - 1;
	if (is_in) {
		/* ASSERT(i < BDC_EP_IN_MAX); */
		return &bdc->ep_in[i];
	} else {
		/* ASSERT(i < BDC_EP_OUT_MAX); */
		return &bdc->ep_out[i];
	}
}

void
bdc_transfer_event(bdcd *bdc, bdc_bd *evt)
{
	bdc_ep0 *ep0;
	bdc_ep *ep;
	uint32 complete_code;
	uint32 index;

	complete_code = evt->transfer_event.flags & BDCBD_XSF_COMP_CODE;

	if (BDCBD_XSF_COMP_BUFFER_NOT_AVAILABLE == complete_code) {
		/* Ignore Buffer Not Available for now. */
		bdc_dbg(("bdc_transfer_event:cc=%x\n", complete_code));
		return;
	}

	index = BDCBD_XSF_EP_INDEX(evt->transfer_event.flags);
	bdc_dbg(("bdc_transfer_event:index=%d(d)\n", index));
	if (index == 1) {
		ep0 = &bdc->ep0;
		ep0->transfer_event(ep0, bdc, evt);
	} else {
		ep = bdc_get_ep_by_context_index(bdc, index);
		ep->transfer_event(ep, bdc, evt);
	}
}

void
bdc_interrupt_handler(usb_controller *controller)
{
	bdcd *bdc;
	bdc_bd *evt;
	bdc_bd_list *evt_list;
	uint32 value;
	uint16 enqueue;
	uint16 dequeue;

	bdc = (bdcd *)controller;

	if (bdc->state == BDC_STATE_SAVED) {
		/* interrupt_clear_pending_state(INTERRUPT_USB3); */
		return;
	}

	/*
	 * value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_BDCSC_ADR));
	 * bdc_dbg(("bdc_interrupt_handler: bdcsc=%x\n", value));
	 */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR));
	bdc_dbg(("bdc_interrupt_handler: srrint0=%x\n", value));

	/* Commended in BDC BT code
	 *      if ((value & BDCREG_SRR_IP) == 0)
	 *      {
	 *              return;
	 *      }
	 */

	value |= BDCREG_SRR_IP;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR),  value);

	/* Need to read it again to avoid missing events */
	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR));
	enqueue = BDCREG_SRR_EPI(value);
	dequeue = BDCREG_SRR_DPI(value);

	evt_list = &bdc->event_list;
	bdc_dbg(("bdc_interrupt_handler: dequeue=%d(d)\n", evt_list->dequeue));
	bdc_dbg(("	  UBDC_SRRINT0_ADR=%x\n", value));
	while (evt_list->dequeue != enqueue) {
		evt = &bdc->event_list.first_bd[evt_list->dequeue];

		bdc_dbg(("bdc_interrupt_handler: evt=0x%x\n", evt->generic.control));
		switch (evt->generic.control & BDCBD_EVENT_TYPE) {
		case BDCBD_EVENT_XSF:
			bdc_transfer_event(bdc, evt);
			break;

		case BDCBD_EVENT_CMD:
			bdc_command_completed(bdc, evt);
			break;

		case BDCBD_EVENT_PORT_STATUS:
			bdc_port_status_changed(bdc, evt);
			break;

		case BDCBD_EVENT_CE:
			ASSERT(0);
		default:
			/* Error */
			bdc_dbg(("bdc_interrupt_handler:error control=%d(d)\n",
				evt->generic.control));
			bdc_dbg(("	status=%d(d)\n", evt->generic.status));
			break;
		}

		bdcbdlist_move_dequeue(&bdc->event_list);
	}


	if (dequeue != evt_list->dequeue) {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR));
		value &= ~(BDCREG_SRR_DPI_MASK | BDCREG_SRR_RWS | BDCREG_SRR_RST | BDCREG_SRR_ISR);
		value |= BDCREG_SRR_TO_DPI(evt_list->dequeue);
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_SRRINT0_ADR), value);
		bdc_dbg(("bdc_interrupt_handler SRRINT0 written %x\n", value));
	}

	if (bdc->condition & BDC_CONDITION_NEED_SAVE) {
		bdc->condition &= ~BDC_CONDITION_NEED_SAVE;
		bdc_save_bdc_state(bdc);
	}

	if (bdc->condition & BDC_CONDITION_NEED_RESET) {
		bdc->condition &= ~BDC_CONDITION_NEED_RESET;

		controller->ft->stop(controller);

		/* clock_delay_microseconds(10); */

		bdc_initialized = 0;

		/* usb_transport->controllers->attribute &= ~USBCONTROLLER_ATTRIBUTE_TRAN_DETECT; */

		controller->ft->start(controller);
	}
}

void
bdc_setup_complete(usb_controller *controller, bdc_status_code status)
{
	bdcd *bdc = (bdcd *)controller;

	bdcep0_setup_complete(&bdc->ep0, bdc, status);
}

void
bdc_bus_reset(usb_controller *controller)
{
	usb_device_t *device;
	bdc_ep *ep;
	int i;
	uint32 value;
	uint8 device_status;
	bdcd *bdc = (bdcd *)controller;

	device = controller->device;
	device_status = device->usbd_current_status;

	bdc_fn(("%s\n", __FUNCTION__));
	usbcontroller_bus_reset(controller);

	controller->ft->ep_destroy(controller, (usb_endpoint *)&(((bdcd *)controller)->ep0));

	if (0) {
		/* Enable LPM */
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR));
		value |= BDCREG_SC_HLE;
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPPM2_ADR), value);
	}

	/*
	 * Since we can not detect disconnect. We need to clean up and be ready
	 * in bus reset.
	 */
	bdcep0_set_speed(&((bdcd *)controller)->ep0, ((bdcd *)controller)->speed);
	bdcep0_config(&((bdcd *)controller)->ep0, (bdcd *)controller);
	/* bdc_set_address((bdcd*)controller, 0); */

	if (device_status & USBDEVICE_STATUS_ENUMERATED)	{
		/* Do this after set address in case SETUP arrives shortly. */
		for (i = BDC_EP_IN_MAX - 1; i >= 0; i--) {
			ep = &((bdcd *)controller)->ep_in[i];
			controller->ft->ep_destroy(controller, (usb_endpoint *)ep);
		}

		for (i = BDC_EP_OUT_MAX - 1; i >= 0; i--) {
			ep = &((bdcd *)controller)->ep_out[i];
			controller->ft->ep_destroy(controller, (usb_endpoint *)ep);
		}
	}

}

void
bdc_remote_wake(usb_controller *controller)
{
	uint32 value;
	uint32 link_state;
	bdcd *bdc = (bdcd *)controller;

	bdc_fn(("%s\n", __FUNCTION__));

	if (bdc->state == BDC_STATE_SAVED) {
		bdc_restore_bdc_state(bdc);
		bdc->state = BDC_STATE_DEFAULT;
	}

	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR));

	link_state = BDCREG_get_link_state(value);

	if (BDC_LINK_STATE_U3 == link_state) {
		if ((USBDEVICE_FEATURE_REMOTE_WAKEUP & controller->attribute) == 0) {
			return;
		}
		if (controller->device->usbd_current_status &
			USBDEVICE_STATUS_RESUMING) {
			return;
		}
		/* Only set status RESUMING for U3.  There is virtually no delay for U2 remote
		 * wake so we do not manage the resume state
		 */
		controller->device->usbd_current_status |= USBDEVICE_STATUS_RESUMING;
	} else if (BDC_LINK_STATE_U2 == link_state)   {
		if (bdc_attribute(bdc) & BDC_ATTRIBUTE_L1_RWE_STATUS_WA) {
			/*
			 * On older hardware, RWE status is on status_reg0[16]
			 * accessed through indirect address 0x2898
			 */
			W_REG(bdc->osh,
				(uint32 *)(bdc->bdc_base_reg +
					UBDC_BDCCFG0_ADR + BDC_IRAADR_OFFSET),
				BDC_IRA_BASE_OFFSET + BDC_IRA_STATUS);
			if ((R_REG(bdc->osh,
				(uint32 *)(bdc->bdc_base_reg +
				UBDC_BDCCFG0_ADR + BDC_IRADAT_OFFSET)) &
				BDC_IRA_STATUS_LPM_RWE) == 0) {
				return;
			}
		} else {
			/* Check remote wake enabled bit */
			if ((R_REG(bdc->osh,
				(uint32 *)(bdc->bdc_base_reg +
					UBDC_USPPM2_ADR)) & BDCREG_SC_RWE) == 0) {
				return;
			}
		}

		/*
		 * Delay at least 51us before asserting remote wake (L1 residency time)
		 * utilslib_delay_us(100);
		 */
	} else {
		return;
	}
/*
 *      if (// osapi_is_timer_running(&bdc_remote_wake_timer))
 *      {
 *              utilslib_delay_us(BDC_REMOTE_WAKE_DELAY_US);
 *      }
 */
	/* link_state is U2 or U3 and remote wake is enabled so assert remote wake */
	value &= ~BDCREG_PORT_STATE;
	value &= ~BDCREG_USPSC_USPSC_RW;
	value |= BDCREG_PORT_STATE_WRITE_STROBE | BDCREG_to_port_link_state(BDC_LINK_STATE_U0);
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR), value);

	bdc_dbg(("bdc_remote_wake:write status %x\n", value));

	/* mdelay(100); */
}

bool
bdc_is_remote_wake_enabled(usb_controller *controller)
{
	bdc_fn(("%s\n", __FUNCTION__));

	if (controller->attribute & USBDEVICE_FEATURE_REMOTE_WAKEUP) {
		return TRUE;
	}

	return FALSE;
}

void
bdc_create(bdcd *bdc)
{
	bdc_fn(("%s\n", __FUNCTION__));
	bdc_init(bdc);
}

void
bdc_clear_connect_status(void)
{
}

/*****************************************************************************
*	 Function: bdc_connect_to_host
*
*	 Abstract: Enable the USB D+ Line by using internal pull-up.
*
*	 Input/Output:
*		none
*
*	 Return:
*		none.
****************************************************************************
*/
void
bdc_connect_to_host(usb_controller *controller)
{
	uint32 value;
	bdcd *bdc = (bdcd *)controller;

	bdc_fn(("%s\n", __FUNCTION__));
	/*
	TODO: Remove this line if soft connect works. This is line disable suspend using
	Internal regs of BDC
	*/
	W_REG(bdc->osh, (uint32 *)0x1800bF98, 0x28DC);
	W_REG(bdc->osh, (uint32 *)0x1800bF9c, 0x01000000);

	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR));
	bdc_dbg(("bdc_connect_to_host:Trying to soft connect UBDC_USPSC_ADR %x\n", value));
	value |= BDCREG_USPSC_SCN;
	value &= ~BDCREG_USPSC_SDC;
	W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR), value);
}

void
_bdc_connect_to_host(usb_controller *controller)
{
	bdc_connect_to_host(controller);
}

/*****************************************************************************
*	 Function: bdc_disconnect_from_host
*
*	 Abstract: Disable the USB D+ Line by using internal pull-up.
*
*	 Input/Output:
*		none
*
*	 Return:
*		none.
****************************************************************************
*/
void
bdc_disconnect_from_host(usb_controller *controller)
{
	uint32 value;
	bdcd *bdc = (bdcd *)controller;
	bdc_fn(("%s\n", __FUNCTION__));

	value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR));
	if ((value & BDCREG_USPSC_VBUS) && (value & BDCREG_USPSC_PCS)) {
		value = R_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR));
		value |= BDCREG_USPSC_SDC;
		value &= ~BDCREG_USPSC_SCN;
		W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + UBDC_USPSC_ADR), value);
	}
}

void
_bdc_disconnect_from_host(usb_controller *controller)
{
	bdc_disconnect_from_host(controller);
}

/*****************************************************************************
*	 Function: usb_transceiver_suspend_control
*
*	 Abstract: Select for upstream transceiver suspend.
*
*
*	 Input/Output:
*		Input:
*				connected : TRUE, USB is connected
*							FALSE, USB is not connected
*
*	 Return:
*		none.
****************************************************************************
*/
void
bdc_transceiver_suspend_control(usb_controller *controller, bool connected)
{
	/* bdcd* bdc = (bdcd*)controller; */

	if (connected) {
		/*
		 * TODO
		 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
		 * USB_SUSPEND_HUSB_MUX);
		 * AND_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
		 * ~USB_SUSPEND_HUSB_VAL);
		 */
	} else {
		/*
		 * TODO
		 * AND_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
		 * ~USB_SUSPEND_HUSB_MUX);
		 * OR_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
		 * USB_SUSPEND_HUSB_VAL);
		 */
	}
}

void
bdc_init(bdcd *bdc)
{
	int i;
	bdc_ep *ep;

	bdc_fn(("%s\n", __FUNCTION__));

	/* uint8 remote_wake_duration; */

	if (bdc_initialized) {
		return;
	}

	usbcontroller_init((usb_controller *)bdc);

	((usb_controller *)bdc)->max_port = BDC_PORT_MAX;
	((usb_controller *)bdc)->ft = (usb_controller_function_table *)&bdc_function_table_rom;

	bdc->state = BDC_STATE_POWERED;

	for (i = BDC_EP_IN_MAX - 1; i >= 0; i--) {
		ep = &bdc->ep_in[i];
		((usb_endpoint *)ep)->address = (uint8)(i + 0x81);
	}

	for (i = BDC_EP_OUT_MAX - 1; i >= 0; i--) {
		ep = &bdc->ep_out[i];
		((usb_endpoint *)ep)->address = (uint8)(i + 1);
	}

	if (usb_internal_config.isoc_buffer) {
		((usb_controller *)bdc)->attribute |= USBCONTROLLER_ATTRIBUTE_ISOC_BUFFER;
	}

	if (usb_internal_config.isoc_word_align_wa)	{
		((usb_controller *)bdc)->attribute |= USBCONTROLLER_ATTRIBUTE_FORCE_WORD_ALIGN;
	}

	if (usb_internal_config.l1rwe_status_wa) {
		bdc_attribute(bdc) |= BDC_ATTRIBUTE_L1_RWE_STATUS_WA;
	}

	if (usb_internal_config.l1rwe_enable_wa) {
		bdc_attribute(bdc) |= BDC_ATTRIBUTE_L1_RWE_ENABLE_WA;
	}

	if (usb_internal_config.default_save_restore_en) {
		bdc_attribute(bdc) |= BDC_ATTRIBUTE_SAVE_RESTORE_EN;
	}

	if (usb_internal_config.bus_reset_hw_reset)	{
		bdc_attribute(bdc) |= BDC_ATTRIBUTE_BUS_RESET_HW_RESET_EN;
	}

/* TODO: Get the remove wake duration */
}

/*******************************************************************************
* Function: bdci_power_off
*
* Abstract: Power off USB BDC
*
* Input/Output: Trivial
*
* Return: None
*
******************************************************************************
*/
void
bdc_power_off(bdcd *bdc)
{
	/* W_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
	 * USB_SUSPEND_HUSB_VAL | USB_PDN_HUSB_DN| USB_PDN_HUSB_DP);
	 */
}

/*******************************************************************************
* Function: bdci_power_on
*
* Abstract: Power off USB BDC
*
* Input/Output: Trivial
*
* Return: None
*
******************************************************************************
*/
void
bdc_power_on(bdcd *bdc)
{
	/*
	 * USB_CONFIG_CONTROL_REG register doesn't exit in WLAN
	 * AND_REG(bdc->osh, (uint32 *)(bdc->bdc_base_reg + USB_CONFIG_CONTROL_REG),
	 * ~(USB_SUSPEND_HUSB_VAL | USB_PDN_HUSB_DN| USB_PDN_HUSB_DP));
	 */
}

void
bdc_adapter_init(void)
{
	bdc_initialized = 0;
}
