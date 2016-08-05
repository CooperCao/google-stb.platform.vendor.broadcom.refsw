/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#ifndef BHDM_MHL_RESP_PRIV_H__
#define BHDM_MHL_RESP_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"       /* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */
#include "bchp_common.h"

#include "bhdm_mhl_const_priv.h"
#include "bhdm_mhl_common_priv.h"
#include "bhdm_mhl_cbus_priv.h"
#include "bhdm_mhl_cbus_cmd_priv.h"
#include "bhdm_mhl_msc_req_priv.h"
#include "bhdm_mhl_ddc_req_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BHDM_MHL_RESP);

#define BHDM_P_MHL_RESP_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP##_##reg_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BHDM_P_MHL_RESP_GET_REG_DATA(reg) \
    (hReq->pulRegs[BHDM_P_MHL_RESP_GET_REG_IDX(reg)])

#define BHDM_P_MHL_RESP_SET_REG_DATA(reg, data) \
    (BHDM_P_MHL_RESP_GET_REG_DATA(reg) = (uint32_t)(data))

#define BHDM_P_MHL_RESP_GET_REG_DATA_I(idx, reg) \
    (hReq->pulRegs[BHDM_P_MHL_RESP_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BHDM_P_MHL_RESP_GET_FIELD_NAME(reg, field) \
    (BHDM_P_MHL_GET_FIELD(BHDM_P_MHL_RESP_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BHDM_P_MHL_RESP_COMPARE_FIELD_DATA(reg, field, data) \
    (BHDM_P_MHL_COMPARE_FIELD_DATA(BHDM_P_MHL_RESP_GET_REG_DATA(reg), reg, field, (data)))

#define BHDM_P_MHL_RESP_COMPARE_FIELD_NAME(reg, field, name) \
    (BHDM_P_MHL_COMPARE_FIELD_NAME(BHDM_P_MHL_RESP_GET_REG_DATA(reg), reg, field, name))

#define BHDM_P_MHL_RESP_INTERRUPT_MASK \
    (BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_IB_DONE_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_OB_DONE_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_BAD_CMD_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_UNEXPECTED_CMD_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_DATA_RECEIVED_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_DATA_OVERFLOW_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_RX_TIMEOUT_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_SW_TIMEOUT_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_ILLEGAL_SW_WR_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_MAX_RETRIES_EXCEEDED_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_UNEXPECTED_INBOUND_PKT_MASK | \
    BCHP_CBUS_INTR2_1_CPU_STATUS_ABORT_MSC_RECEIVED_MASK)

/* MSC Responder Events */
#define BHDM_P_MHL_MSCRESP_EVENT_NONE     (0)
    /* MSC RESP HW INTR events (CBUS_INTR2_1 register) */
    /* for optimized implementation while posting flag for task from IRQ4, these events have been assign same bit position as in the CBUS_INTR2_1 register */
#define BHDM_P_MHL_MSCRESP_EVENT_IB_DONE  BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_IB_DONE_MASK /*(1 << 9)*/
#define BHDM_P_MHL_MSCRESP_EVENT_OB_DONE  BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_OB_DONE_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_BAD_CMD  BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_BAD_CMD_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_UNEXPECTED_CMD  BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_UNEXPECTED_CMD_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_DATA_RECEIVED   BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_DATA_RECEIVED_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_DATA_OVERFLOW   BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_DATA_OVERFLOW_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_RX_TIMEOUT      BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_RX_TIMEOUT_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_SW_TIMEOUT      BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_SW_TIMEOUT_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_ILLEGAL_SW_WR   BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_ILLEGAL_SW_WR_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_MAX_RETRIES_EXCEEDED    BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_MAX_RETRIES_EXCEEDED_MASK
#define BHDM_P_MHL_MSCRESP_EVENT_UNEXPECTED_INBOUND_PKT  BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_RESP_UNEXPECTED_INBOUND_PKT_MASK /* (1<<19) */
#define BHDM_P_MHL_MSCRESP_EVENT_ABORT_MSC_OUTBOUND_TIMEOUT_DONE  BCHP_CBUS_INTR2_1_CPU_STATUS_ABORT_MSC_OUTBOUND_TIMEOUT_DONE_MASK /* (1 << 30) */

typedef uint32_t BHDM_P_Mhl_MscResp_Event;

typedef struct BHDM_P_Mhl_MscResp_Object
{
    BDBG_OBJECT(BHDM_MHL_RESP)

    /* flag initial state, requires reset; */
    bool                     bInitial;

    BHDM_P_Mhl_MscReq_Handle hMscReq;
    BHDM_P_Mhl_DdcReq_Handle hDdcReq;

    /* private fields. */
    uint32_t                 ulBaseReg;
    uint32_t                 ulRegCount; /* mumber of registers */
    uint32_t                *pulRegs; /* array of registers */

    BREG_Handle              hRegister;

    /* Activity */
    bool                     bActive;
    bool                     bPending;
    bool                     bScratchpadValid;
    bool                     bSinkDscrChg;

    /* Abort received/active? xxx_abort_received will be set if
       we get an incoming ABORT. xxx_active will be set as long
       as the ABORT timer is active. xxx_abort_sent is set when
       we send an ABORT. */
    bool                     bAbortSent;
    bool                     bIbAbortActive;
    bool                     bObAbortActive;

    BHDM_P_Mhl_CbusCmd       stIbLastCmd;
    BHDM_P_Mhl_CbusCmd       stObLastCmd;

    BHDM_P_Mhl_MscResp_Event eEvent;
    BHDM_P_Mhl_MscResp_Event eUnhandledEvent;

    uint8_t                  ucErrCode;


} BHDM_P_Mhl_MscResp_Object;

typedef BHDM_P_Mhl_MscResp_Object *BHDM_P_Mhl_MscResp_Handle;

typedef struct BHDM_P_Mhl_MscResp_Settings
{
    bool bInit;
} BHDM_P_Mhl_MscResp_Settings;

BERR_Code BHDM_P_Mhl_MscResp_Create
    ( BHDM_P_Mhl_MscResp_Handle          *phResp );

BERR_Code BHDM_P_Mhl_MscResp_Destroy
    ( BHDM_P_Mhl_MscResp_Handle   hResp );

/* Initialisation */
void BHDM_P_Mhl_MscResp_Init
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_MscReq_Handle    hMscReq,
      BHDM_P_Mhl_DdcReq_Handle    hDdcReq,
      BREG_Handle                 hRegister );

void BHDM_P_Mhl_MscResp_Active_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      bool                       *pbChannelActive,
      bool                       *pbIbAbortActive,
      bool                       *pbObAbortActive );

void BHDM_P_Mhl_MscResp_SendCmd_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pCmd );

void BHDM_P_Mhl_MscResp_CancelCmd_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp );

BERR_Code BHDM_P_Mhl_MscResp_ReceiveCmd_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pCmd );

void BHDM_P_Mhl_MscResp_ClearAbort_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp );

void BHDM_P_Mhl_MscResp_SendAbort_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_MscReq_Event    *peMscReqEvent );

void BHDM_P_Mhl_MscResp_Done_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp );

BERR_Code BHDM_P_Mhl_MscResp_Cancel_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply );

BERR_Code BHDM_P_Mhl_MscResp_Complete_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply );

BERR_Code BHDM_P_Mhl_MscResp_SetHpd_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_ClrHpd_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_ReadDevCap_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucOffset,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_SetInt_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucOffset,
      uint8_t                     ucValue,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_GetState_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_GetVendorId_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_WriteStat_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucOffset,
      uint8_t                     ucValue,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_GetErrorCode_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      BHDM_P_Mhl_Command          eCmd,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_Ack_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      BHDM_P_Mhl_Command          eCmd,
      uint8_t                     ucData1,
      uint8_t                     ucData2,
      bool                        ucAck,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_Rcp_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      BHDM_P_Mhl_Command          eCmd,
      uint8_t                     ucData1,
      uint8_t                     ucData2,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_Rap_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      BHDM_P_Mhl_Command          eCmd,
      uint8_t                     ucData1,
      uint8_t                     ucData2,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_WriteBurst_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pCmd,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_ProcessReq_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_CbusCmd         *pCmd,
      BHDM_P_Mhl_CbusCmd         *pReply,
      BHDM_P_Mhl_CbusState       *pBusState,
      BHDM_P_Mhl_MscReq_Event    *peMscReqEvent,
      BHDM_P_Mhl_DdcReq_Event    *peDdcReqEvent,
      uint8_t                     ucDelay );

BERR_Code BHDM_P_Mhl_MscResp_CheckError_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      int                         interrupt,
      bool                       *pbAbortRequired,
      BHDM_P_Mhl_CbusPkt         *pLastPacket );

uint32_t BHDM_P_Mhl_MscResp_GetIntStatus_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp );

void BHDM_P_Mhl_MscResp_HandleAbortMscObTimeoutDone_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp );

void BHDM_P_Mhl_MscResp_HandleIbDone_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusState        *pstCbusState,
      BHDM_P_Mhl_MscReq_Event     *peMscReqEvent,
      BHDM_P_Mhl_DdcReq_Event     *peDdcReqEvent );

void BHDM_P_Mhl_MscResp_HandleObDone_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp );

void BHDM_P_Mhl_MscResp_HandleErrors_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      int                          interrupt,
      BHDM_P_Mhl_MscReq_Event     *peMscReqEvent );

#ifdef __cplusplus
{
#endif

#endif /* BHDM_MHL_RESP_PRIV_H__ */
