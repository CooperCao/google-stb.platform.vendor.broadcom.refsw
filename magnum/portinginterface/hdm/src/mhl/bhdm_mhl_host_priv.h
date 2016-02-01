/***************************************************************************
*     Copyright (c) 2003-2014, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#ifndef BHDM_MHL_HOST_PRIV_H__
#define BHDM_MHL_HOST_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bchp_mpm_cpu_ctrl.h"
#include "bchp_mpm_host_l2.h"
#include "bchp_mpm_cpu_l2.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_clkgen.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_hdmi_tx_phy.h"

#include "bhdm_mhl_common_priv.h"
#include "bhdm_mhl_cbus_priv.h"
#include "bhdm_mhl_mailbox_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* No. of bytes/words in scratch space */
#define BHDM_P_MHL_HOST_NUM_SCRATCH_REGS 4
#define BHDM_P_MHL_HOST_SCRATCH_SIZE (BHDM_P_MHL_HOST_NUM_SCRATCH_REGS * sizeof(uint32_t))

/* Interrupt for power state transition */
#define BHDM_P_MHL_S3_TO_S0  (1 << 0)
#define BHDM_P_MHL_S0_TO_S3  (1 << 1)

typedef enum
{
	BHDM_P_Mhl_HostCpuState_eIdle = 0,
	BHDM_P_Mhl_HostCpuState_eActive = 1,
	BHDM_P_Mhl_HostCpuState_eStopping = 2
} BHDM_P_Mhl_HostCpuState;

/*
 * Host scratchpad definitions
 */
#ifndef BHDM_P_MHL_HOST_REVISION
#define BHDM_P_MHL_HOST_REVISION 0x10
#endif

typedef enum
{
	BHDM_P_Mhl_HostField_eRev                   = 0,
	BHDM_P_Mhl_HostField_eHostState             = 1,
	BHDM_P_Mhl_HostField_eLinkState             = 2,
	BHDM_P_Mhl_HostField_eSinkState             = 3,
	BHDM_P_Mhl_HostField_eMscErr                = 4,
	BHDM_P_Mhl_HostField_eDdcErr                = 5,
	BHDM_P_Mhl_HostField_eSrcLinkMode2          = 6,
	/* Some space to allow for FCB expansion */

	BHDM_P_Mhl_HostField_eBootState             = 12 /* Use SPARE3 register */
} BHDM_P_Mhl_HostField;

/*
 * Byte 0: REV (8 bits)
 * If zero, will be treated as the entire
 * scratch pad as invalid and MPM firmware will
 * initialise and reset CBUS hardware
 */
#define BHDM_P_MHL_HOST_REV_OFFSET BHDM_P_Mhl_MailboxField_eRev
#define BHDM_P_MHL_HOST_REV_MASK   (0xFF)
#define BHDM_P_MHL_HOST_REV_LSB    (0)

/*
 * Byte 1: HOST CPU state (2-bits of cbus_host_cpu_state_t)
 */
#define BHDM_P_MHL_HOST_HOST_STATE_OFFSET BHDM_P_Mhl_MailboxField_eMpmState
#define BHDM_P_MHL_HOST_HOST_STATE_MASK   (0x3)
#define BHDM_P_MHL_HOST_HOST_STATE_LSB    (0)

/*
 * Byte 2: Link state
 * Bit 0: CBUS connected (1) or disconnected (0).
 * If disconnected, MPM firmware will reinitialise CBUS hardware
 * start wakeup and discovery
 *
 * Bit 1: VBUS _NOT_ required
 * If this bit is 0, then MPM will start power manager after
 * Sink DCAP is read and PLIM is > 0
 *
 * Bit 2: HPD up (1) or down (0)
 * Note that if HPD is up, MPM firmware will issue
 * EDID read without waiting for SET_HPD
 */
#define BHDM_P_MHL_HOST_LINK_STATE_OFFSET BHDM_P_Mhl_HostField_eLinkState
#define BHDM_P_MHL_HOST_LINK_STATE_MASK   (0xFF)
#define BHDM_P_MHL_HOST_LINK_STATE_LSB    (0)

#define BHDM_P_MHL_HOST_CBUS_CONNECTED_OFFSET BHDM_P_MHL_HOST_LINK_STATE_OFFSET
#define BHDM_P_MHL_HOST_CBUS_CONNECTED_MASK      (0x1)
#define BHDM_P_MHL_HOST_CBUS_CONNECTED_LSB       (0)

#define BHDM_P_MHL_HOST_VBUS_NREQ_OFFSET BHDM_P_MHL_HOST_LINK_STATE_OFFSET
#define BHDM_P_MHL_HOST_VBUS_NREQ_MASK      (0x1)
#define BHDM_P_MHL_HOST_VBUS_NREQ_LSB       (1)

#define BHDM_P_MHL_HOST_HPD_OFFSET BHDM_P_MHL_HOST_LINK_STATE_OFFSET
#define BHDM_P_MHL_HOST_HPD_MASK         (0x1)
#define BHDM_P_MHL_HOST_HPD_LSB          (2)

/* Byte 3: sink state
 * Bit 0: Sink's DCAP_RDY
 */
#define BHDM_P_MHL_HOST_SINK_STATE_OFFSET BHDM_P_Mhl_HostField_eSinkState
#define BHDM_P_MHL_HOST_SINK_STATE_MASK   (0xFF)
#define BHDM_P_MHL_HOST_SINK_STATE_LSB    (0)

#define BHDM_P_MHL_HOST_SINK_DCAP_RDY_OFFSET BHDM_P_Mhl_HostField_eSinkState
#define BHDM_P_MHL_HOST_SINK_DCAP_RDY_MASK   (0x1)
#define BHDM_P_MHL_HOST_SINK_DCAP_RDY_LSB    (0)

/* Byte 4: MSC error code */
#define BHDM_P_MHL_HOST_SRC_MSC_ERRORCODE_OFFSET BHDM_P_Mhl_HostField_eMscErr
#define BHDM_P_MHL_HOST_SRC_MSC_ERRORCODE_MASK (0xFF)
#define BHDM_P_MHL_HOST_SRC_MSC_ERRORCODE_LSB (0)

/* Byte 5: DDC error code */
#define BHDM_P_MHL_HOST_SRC_DDC_ERRORCODE_OFFSET BHDM_P_Mhl_HostField_eDdcErr
#define BHDM_P_MHL_HOST_SRC_DDC_ERRORCODE_MASK (0xFF)
#define BHDM_P_MHL_HOST_SRC_DDC_ERRORCODE_LSB (0)

/* Byte 6: SRC_LINK_MODE_2
   Bit 0-2 CLK_MODE
   Bit 3 PATH_EN
   Bit 4 MUTED

   SRC_LINK_MODE_2 is used to initialise mailbox LINK_MODE_1 field
   at boot time. If CLK_MODE is invalid, it will be set automatically
   to 24-bit.
 */
#define BHDM_P_MHL_HOST_SRC_LINK_MODE_2_OFFSET BHDM_P_Mhl_HostField_eSrcLinkMode2
#define BHDM_P_MHL_HOST_SRC_LINK_MODE_2_MASK   (0x1F)
#define BHDM_P_MHL_HOST_SRC_LINK_MODE_2_LSB    (0)
#define BHDM_P_MHL_HOST_SRC_CLK_MODE_2_OFFSET BHDM_P_MHL_HOST_SRC_LINK_MODE_2_OFFSET
#define BHDM_P_MHL_HOST_SRC_CLK_MODE_2_MASK (0x7)
#define BHDM_P_MHL_HOST_SRC_CLK_MODE_2_LSB (0)
#define BHDM_P_MHL_HOST_SRC_PATH_EN_2_OFFSET BHDM_P_MHL_HOST_SRC_LINK_MODE_2_OFFSET
#define BHDM_P_MHL_HOST_SRC_PATH_EN_2_MASK (0x1)
#define BHDM_P_MHL_HOST_SRC_PATH_EN_2_LSB (3)
#define BHDM_P_MHL_HOST_SRC_MUTED_2_OFFSET BHDM_P_MHL_HOST_SRC_LINK_MODE_2_OFFSET
#define BHDM_P_MHL_HOST_SRC_MUTED_2_MASK (0x1)
#define BHDM_P_MHL_HOST_SRC_MUTED_2_LSB (4)


/*
 * Byte 12: Boot state
 * Bit 0-3 FW_BOOT_COUNT - firmware boot count
 */
#define BHDM_P_MHL_HOST_BOOT_STATE_OFFSET BHDM_P_Mhl_HostField_eBootState
#define BHDM_P_MHL_HOST_BOOT_STATE_MASK   (0xFF)
#define BHDM_P_MHL_HOST_BOOT_STATE_LSB    (0)
#define BHDM_P_MHL_HOST_FW_BOOT_COUNT_OFFSET BHDM_P_Mhl_HostField_eBootState
#define BHDM_P_MHL_HOST_FW_BOOT_COUNT_MASK   (0xF)
#define BHDM_P_MHL_HOST_FW_BOOT_COUNT_LSB    (0)

/* Similar API to mailbox to get fields */
uint8_t BHDM_P_Mhl_Host_GetField
	( BREG_Handle hRegister,
	  uint16_t    offset,
	  uint8_t     mask,
	  uint8_t     lsb );

#define BHDM_P_MHL_HOST_GET_FIELD(reghandle,field)  BHDM_P_Mhl_Host_GetField( \
		reghandle, \
		BHDM_P_MHL_HOST_##field##_OFFSET,		       \
		BHDM_P_MHL_HOST_##field##_MASK,		       \
		BHDM_P_MHL_HOST_##field##_LSB)

uint8_t BHDM_P_Mhl_Host_GetField_isr
	( BREG_Handle hRegister,
	  uint16_t    offset,
	  uint8_t     mask,
	  uint8_t     lsb );


#define BHDM_P_MHL_HOST_GET_FIELD_ISR(reghandle,field)  BHDM_P_Mhl_Host_GetField_isr( \
		reghandle, \
		BHDM_P_MHL_HOST_##field##_OFFSET,		       \
		BHDM_P_MHL_HOST_##field##_MASK,		       \
		BHDM_P_MHL_HOST_##field##_LSB)

void BHDM_P_Mhl_Host_SetField
	( BREG_Handle hRegister,
	  uint16_t    offset,
	  uint8_t     mask,
	  uint8_t     lsb,
	  uint8_t     value );

#define BHDM_P_MHL_HOST_SET_FIELD(reghandle,field, value) BHDM_P_Mhl_Host_SetField( \
		reghandle, \
		BHDM_P_MHL_HOST_##field##_OFFSET,		       \
		BHDM_P_MHL_HOST_##field##_MASK,		       \
		BHDM_P_MHL_HOST_##field##_LSB,		       \
		value)

void BHDM_P_Mhl_Host_SetField_isr
	( BREG_Handle hRegister,
	  uint16_t    offset,
	  uint8_t     mask,
	  uint8_t     lsb,
	  uint8_t     value );

#define BHDM_P_MHL_HOST_SET_FIELD_ISR(reghandle,field, value) BHDM_P_Mhl_Host_SetField_isr( \
		reghandle, \
		BHDM_P_MHL_HOST_##field##_OFFSET,		       \
		BHDM_P_MHL_HOST_##field##_MASK,		       \
		BHDM_P_MHL_HOST_##field##_LSB,		       \
		value)

/* No write access by MPM to scratch pad registers */

/*  TX_READY int was triggered */
void BHDM_P_Mhl_Host_S3ToS0Handover_isr
	( BREG_Handle      hRegister );

/* STOP_REQUEST int was triggered */
void BHDM_P_Mhl_Host_S0ToS3Handover_isr
	( BREG_Handle      hRegister );

/* Enable the Power management state machine
   to boot the host CPU */
void BHDM_P_Mhl_Host_StartPmsm_isr
	( BREG_Handle      hRegister );

/* En/Disable the MHL TMDS path, set enable to true to enable, false to disable */
void BHDM_P_Mhl_Host_EnableMhlTx_isr
	( BREG_Handle      hRegister,
	  bool             enable );

bool BHDM_P_Mhl_Host_GetIntrStatus
	( BREG_Handle              hRegister,
	  BHDM_P_Mhl_MpmHostIntr  *pIntrStat );

void BHDM_P_Mhl_Host_ClearAllInt_isr
	( BREG_Handle              hRegister );

void BHDM_P_Mhl_Host_ClearInt_isr
	( BREG_Handle              hRegister,
	  BHDM_P_Mhl_MpmHostIntr  *pIntStat );


#if BHDM_MHL_CTS
/* Initialise the mailbox from the scratch pad register */
void BHDM_P_Mhl_Host_MailboxInit
	( BREG_Handle      hRegister,
	  uint8_t          rev,
	  uint8_t         *pucDcap );
#endif

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_HOST_PRIV_H__ */
