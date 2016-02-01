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
#ifndef BHDM_MHL_DEBUG_PRIV_H__
#define BHDM_MHL_DEBUG_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bhdm_mhl_cbus_cmd_priv.h"
#include "bhdm_mhl_mailbox_priv.h"
#include "bhdm_mhl_req_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#if BHDM_MHL_ENABLE_DEBUG
#include <string.h>
#include <stdio.h>

extern FILE *s_pfCbusLog;

#define BHDM_P_MHL_DEBUG_DUMP_TO_FILE(str) \
do                                         \
{                                          \
	fprintf(s_pfCbusLog, str);             \
	fflush(s_pfCbusLog);                   \
} while(0)

#endif

void BHDM_P_Mhl_DumpCmdPkts_isr
	( BHDM_P_Mhl_CbusPkt          *pPackets,
	  uint32_t                     ulNumPackets,
	  BHDM_P_Mhl_CbusDest          eDestination );

void BHDM_P_Mhl_DumpCmdQueue_isr
	( BHDM_P_Mhl_Req_Handle   hReq );

void BHDM_P_Mhl_FieldDecode_isr
	( uint8_t                 addr,
	  uint8_t                 data );

void BHDM_P_Mhl_MscMsgDecode_isr
	( uint8_t                 action,
	  uint8_t                 data );

void BHDM_P_Mhl_DumpEdid_isr
	( uint8_t                *pucEdidBlock,
	  uint32_t                ulEdidNumBlocks );

void BHDM_P_Mhl_DumpMailbox_isr
	( BREG_Handle               hRegister );

void BHDM_P_Mhl_DumpRegisters_isr
	( BREG_Handle            hRegister,
	  uint32_t               ulRegBaseAddr,
	  uint32_t               ulRegCount );

void BHDM_P_Mhl_DumpSinkDcap_isr
	( BREG_Handle            hRegister );

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_DEBUG_PRIV_H__ */
