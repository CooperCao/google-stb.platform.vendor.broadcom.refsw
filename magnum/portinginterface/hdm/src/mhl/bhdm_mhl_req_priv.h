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

#ifndef BHDM_MHL_REQ_PRIV_H__
#define BHDM_MHL_REQ_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bchp_common.h"

#include "bhdm_mhl_const_priv.h"
#include "bhdm_mhl_common_priv.h"
#include "bhdm_mhl_cbus_priv.h"
#include "bhdm_mhl_cbus_cmd_priv.h"

#include "bchp_mt_msc_req.h"
#include "bchp_mt_ddc_req.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BHDM_MHL_REQ);

#define BHDM_P_MHL_REQ_GET_REG_IDX(reg) \
	((BCHP##_##reg - BCHP##_##reg_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BHDM_P_MHL_REQ_GET_REG_DATA(reg) \
	(hReq->pulRegs[BHDM_P_MHL_REQ_GET_REG_IDX(reg)])

#define BHDM_P_MHL_REQ_SET_REG_DATA(reg, data) \
	(BHDM_P_MHL_REQ_GET_REG_DATA(reg) = (uint32_t)(data))

#define BHDM_P_MHL_REQ_GET_REG_DATA_I(idx, reg) \
	(hReq->pulRegs[BHDM_P_MHL_REQ_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BHDM_P_MHL_REQ_GET_FIELD_NAME(reg, field) \
	(BHDM_P_MHL_GET_FIELD(BHDM_P_MHL_REQ_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BHDM_P_MHL_REQ_COMPARE_FIELD_DATA(reg, field, data) \
	(BHDM_P_MHL_COMPARE_FIELD_DATA(BHDM_P_MHL_REQ_GET_REG_DATA(reg), reg, field, (data)))

#define BHDM_P_MHL_REQ_COMPARE_FIELD_NAME(reg, field, name) \
	(BHDM_P_MHL_COMPARE_FIELD_NAME(BHDM_P_MHL_REQ_GET_REG_DATA(reg), reg, field, name))


typedef enum
{
	BHDM_P_Mhl_ReqType_eMsc,
	BHDM_P_Mhl_ReqType_eDdc
} BHDM_P_Mhl_ReqType;

typedef struct BHDM_P_Mhl_Req_Object
{
	BDBG_OBJECT(BHDM_MHL_REQ)

	/* flag initial state, requires reset; */
	bool                      bInitial;

	BHDM_P_Mhl_ReqType        eType;
	uint32_t                  ulBaseReg;
	uint32_t                  ulRegCount; /* mumber of registers */
	uint32_t                 *pulRegs; /* array of registers */

	BREG_Handle               hRegister;

	/* Activity */
	bool                      bActive;  /* Is requester active? */

	/* Abort received/active? xxx_abort_received will be set if
	   we get an incoming ABORT. xxx_active will be set as long
	   as the ABORT timer is active. xxx_abort_sent is set when
	   we send an ABORT. */
	bool                      bAbortSent;
	bool                      bAbortActive;

	BHDM_P_Mhl_CmdQueue      *pstCmdQ;
	BHDM_P_Mhl_CbusCmd        stLastCmd;

	/* flag to indicate retry of last command */
	bool                      bRetryLastCmd;

	/* flag indicating a request is pending */
	bool                      bRequestPending;

	/* send to MPM during host CPU handoff to MPM */
	uint8_t                   ucErrCode;
} BHDM_P_Mhl_Req_Object;

typedef struct BHDM_P_Mhl_Req_Object *BHDM_P_Mhl_Req_Handle;


BERR_Code BHDM_P_Mhl_Req_Create
	( BHDM_P_Mhl_Req_Handle          *phReq );

BERR_Code BHDM_P_Mhl_Req_Destroy
	( BHDM_P_Mhl_Req_Handle     hReq );

/* Initialization */
BERR_Code BHDM_P_Mhl_Req_Init
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BREG_Handle               hRegister );

void BHDM_P_Mhl_Req_Active_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  bool                     *pbActive,
	  bool                     *pbAbortActive );

void BHDM_P_Mhl_Req_SendCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_CbusCmd       *pCmd );

void BHDM_P_Mhl_Req_CancelCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq );

BERR_Code BHDM_P_Mhl_Req_ReceiveCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_CbusCmd       *pCmd );

void BHDM_P_Mhl_Req_ClearAbort_isr
	( BHDM_P_Mhl_Req_Handle     hReq );

void BHDM_P_Mhl_Req_SendAbort_isr
	( BHDM_P_Mhl_Req_Handle     hReq );

uint32_t BHDM_P_Mhl_Req_GetIntStatus_isr
	( BHDM_P_Mhl_Req_Handle     hReq );

BERR_Code BHDM_P_Mhl_Req_AddCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_CbusCmd       *pCmd );

BERR_Code BHDM_P_Mhl_Req_CompleteCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_CbusCmd       *pCmd );

BERR_Code BHDM_P_Mhl_Req_SendNextCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_CbusCmd       *pCmd,
	  bool                     *pbRetryLastCmd );

BERR_Code BHDM_P_Mhl_Req_1DataCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_Command        eCmd,
	  uint8_t                  *pucData,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd );

BERR_Code BHDM_P_Mhl_Req_2DataCmd_isr
	( BHDM_P_Mhl_Req_Handle    hReq,
	  BHDM_P_Mhl_Command       cmd,
	  uint8_t                  ucData1,
	  uint8_t                  ucData2,
	  uint8_t                  ucDelay,
	  bool                     bLastCmd );

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_REQ_PRIV_H__ */
