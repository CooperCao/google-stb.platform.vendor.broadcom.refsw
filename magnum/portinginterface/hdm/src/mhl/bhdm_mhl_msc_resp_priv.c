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
#include "bhdm_mhl_msc_resp_priv.h"
#include "bhdm_mhl_debug_priv.h"

BDBG_MODULE(BHDM_MHL_RESP);
BDBG_OBJECT_ID(BHDM_MHL_RESP);


/* Static functions */
static void BHDM_P_Mhl_MscResp_CmdAdd_isr
    ( BHDM_P_Mhl_CbusCmd       *pReply,
      const BHDM_P_Mhl_CbusPkt *pPacket,
      uint32_t                  ulNumPackets,
      uint8_t                   ucDelay )
{
    uint32_t i;
    for(i = 0; i < ulNumPackets; i++)
    {
        pReply->cbusPackets.astShortCmd[i].ulType = pPacket[i].ulType;
        pReply->cbusPackets.astShortCmd[i].ulDir = pPacket[i].ulDir;
        pReply->cbusPackets.astShortCmd[i].ucData = pPacket[i].ucData;
    }
    pReply->ulNumPacketsCfg = ulNumPackets;
    /* TODO: at the moment we don't delay the reply */
    pReply->ucDelay = ucDelay;
    /* These values are fixed */
    pReply->eState = BHDM_P_Mhl_CbusCmdState_ePending;
    pReply->ePriority = BHDM_P_Mhl_CbusPriority_eResp;
    pReply->eDest = BHDM_P_Mhl_CbusDest_eMscResp;
    pReply->eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
}


BERR_Code BHDM_P_Mhl_MscResp_Create
    ( BHDM_P_Mhl_MscResp_Handle          *phResp )
{
    BERR_Code rc = BERR_SUCCESS;
    BHDM_P_Mhl_MscResp_Object *pResp;

    *phResp = NULL;

    /* Alloc the context. */
    pResp = (BHDM_P_Mhl_MscResp_Object*)(BKNI_Malloc(sizeof(BHDM_P_Mhl_MscResp_Object)));
    if(!pResp)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* zero out all memory associated with the HDMI Device Handle before using */
    BKNI_Memset((void *) pResp, 0, sizeof(BHDM_P_Mhl_MscResp_Object));
    BDBG_OBJECT_SET(pResp, BHDM_MHL_RESP);

    /* assign the handles passed in as parameters */
    pResp->ulBaseReg = BCHP_MT_MSC_REQ_REG_START;
    pResp->ulRegCount = BHDM_P_MHL_REGS_ENTRIES(MT_MSC_REQ_REG_START, MT_MSC_REQ_REG_END);

    pResp->pulRegs = (uint32_t *)BKNI_Malloc(sizeof(uint32_t) * pResp->ulRegCount);
    if (pResp->pulRegs == NULL)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* keep created pointer */
    *phResp = (BHDM_P_Mhl_MscResp_Handle)pResp;

    return rc;
}

BERR_Code BHDM_P_Mhl_MscResp_Destroy
    ( BHDM_P_Mhl_MscResp_Handle hResp )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);

    BKNI_Free(hResp->pulRegs);

    /* free memory associated with the HDMI handle */
    BDBG_OBJECT_DESTROY(hResp, BHDM_MHL_RESP);
    BKNI_Free((void*)hResp);

    return rc;
}

void BHDM_P_Mhl_MscResp_Init
    ( BHDM_P_Mhl_MscResp_Handle        hResp,
      BHDM_P_Mhl_MscReq_Handle         hMscReq,
      BHDM_P_Mhl_DdcReq_Handle         hDdcReq,
      BREG_Handle                      hRegister )
{
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);
    BDBG_ASSERT(hMscReq);

    /* Clear out shadow registers. */
    BKNI_Memset((void*)hResp->pulRegs, 0x0, (sizeof(uint32_t) * hResp->ulRegCount));

    /* Initialize state. */
    hResp->bInitial = true;

    hResp->hRegister = hRegister;
    hResp->hMscReq = hMscReq;
    hResp->hDdcReq = hDdcReq;

    hResp->eEvent = BHDM_P_MHL_MSCRESP_EVENT_NONE;
}

/* Set channel_active or abort active to true if the
   MSC responder channel is active or ABORT timer is active spectively (in or outbound) */
void BHDM_P_Mhl_MscResp_Active_isr
    ( BHDM_P_Mhl_MscResp_Handle  hResp,
      bool                   *pbChannelActive,
      bool                   *pbIbAbortActive,
      bool                   *pbObAbortActive )
{
    BREG_Handle hRegister;
    uint32_t ulStat;

    BDBG_ASSERT(hResp);
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);

    hRegister = hResp->hRegister;

    ulStat = BREG_Read32(hRegister, BCHP_MT_CBUS_SCHEDULER_STATUS_0);
    *pbChannelActive= BCHP_GET_FIELD_DATA(ulStat, MT_CBUS_SCHEDULER_STATUS_0, MSC_RESPONDER_ACTIVE) ? true : false;
    *pbIbAbortActive = BCHP_GET_FIELD_DATA(ulStat,  MT_CBUS_SCHEDULER_STATUS_0, MSC_RESPONDER_IB_ABORT_ACTIVE) ? true : false;
    *pbObAbortActive = BCHP_GET_FIELD_DATA(ulStat, MT_CBUS_SCHEDULER_STATUS_0, MSC_RESPONDER_OB_ABORT_ACTIVE) ? true : false;
}

/* [Start to] Send a command from a particular FIFO */
void BHDM_P_Mhl_MscResp_SendCmd_isr
    ( BHDM_P_Mhl_MscResp_Handle           hResp,
      BHDM_P_Mhl_CbusCmd                 *pCmd )
{
    uint32_t ulOutboundFifoReg;
    uint32_t ulData;
    BREG_Handle hRegister;

    BDBG_ASSERT(hResp);
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);
    BDBG_ASSERT(pCmd);
    BDBG_ASSERT(pCmd->eDest == BHDM_P_Mhl_CbusDest_eMscResp);

    hRegister = hResp->hRegister;

    /* First clear the FIFO by writing to the CTRL register */
    ulData = BREG_Read32(hRegister, BCHP_MT_MSC_RESP_CTRL);
    ulData &= ~BCHP_MT_MSC_RESP_CTRL_CLEAR_OUTBOUND_BUFFER_MASK;
    ulData |= BCHP_FIELD_DATA(MT_MSC_RESP_CTRL, CLEAR_OUTBOUND_BUFFER, 1);
    BREG_Write32(hRegister, BCHP_MT_MSC_RESP_CTRL, ulData);

    ulOutboundFifoReg = BCHP_MT_MSC_RESP_OUTBOUND_PACKET_00_01;

    BDBG_MSG(("Sending %d pkts at fifo offset 0x%x", pCmd->ulNumPacketsCfg, ulOutboundFifoReg));

    BHDM_P_Mhl_Cbus_SendPkts_isr(hRegister, ulOutboundFifoReg, pCmd, BHDM_P_Mhl_CbusCmdQueueType_eShort);

    /* Write the packet count to the FIFO config register
       and then start the transaction */
    ulData = BREG_Read32(hRegister, BCHP_MT_MSC_RESP_CFG_0);
    ulData &= ~BCHP_MT_MSC_RESP_CFG_0_NUM_PACKETS_MASK;
    ulData |= BCHP_FIELD_DATA(MT_MSC_RESP_CFG_0, NUM_PACKETS, pCmd->ulNumPacketsCfg);
    BREG_Write32(hRegister, BCHP_MT_MSC_RESP_CFG_0, ulData);

    ulData = BREG_Read32(hRegister, BCHP_MT_MSC_RESP_CTRL);
    ulData &= ~BCHP_MT_MSC_RESP_CTRL_START_MASK;
    ulData |= BCHP_FIELD_DATA(MT_MSC_RESP_CTRL, START, 1);
    BREG_Write32(hRegister, BCHP_MT_MSC_RESP_CTRL, ulData);

}

void BHDM_P_Mhl_MscResp_CancelCmd_isr
    ( BHDM_P_Mhl_MscResp_Handle           hResp )
{
    uint32_t ulData;

    BDBG_ASSERT(hResp);
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);

    /* Trigger the cancel bit in the ctrl register,
       as well as clearing the FIFO */
    ulData = BREG_Read32(hResp->hRegister, BCHP_MT_MSC_RESP_CTRL);
    ulData &= ~(BCHP_MT_MSC_RESP_CTRL_CANCEL_MASK | BCHP_MT_MSC_RESP_CTRL_CLEAR_OUTBOUND_BUFFER_MASK);
    ulData |= BCHP_FIELD_DATA(MT_MSC_RESP_CTRL, CANCEL, 1) | BCHP_FIELD_DATA(MT_MSC_RESP_CTRL, CLEAR_OUTBOUND_BUFFER, 1);
    BREG_Write32(hResp->hRegister, BCHP_MT_MSC_RESP_CTRL, ulData);

}

/* Call this instead to retrieve a message from the responder incoming FIFO */
BERR_Code BHDM_P_Mhl_MscResp_ReceiveCmd_isr
    ( BHDM_P_Mhl_MscResp_Handle        hResp,
      BHDM_P_Mhl_CbusCmd              *pCmd )
{
    BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
    uint32_t i, ulData;
    uint32_t ulInPktReg = BCHP_MT_MSC_RESP_INBOUND_PACKET_00_01;
    BREG_Handle hRegister;

    BDBG_ASSERT(hResp);
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);
    BDBG_ASSERT(pCmd);

    hRegister = hResp->hRegister;

    ulData = BREG_Read32(hRegister, BCHP_MT_MSC_RESP_STATUS);
    pCmd->ulNumPacketsDone = BCHP_GET_FIELD_DATA(ulData, MT_MSC_RESP_STATUS, INBOUND_PACKET_CNT);

    BDBG_ASSERT(sizeof(pCmd->cbusPackets.astLongCmd) >= pCmd->ulNumPacketsDone);

    BDBG_ERR(("MSC RESP received 0x%x pkts", pCmd->ulNumPacketsDone));

    /* For consistency */
    pCmd->ulNumPacketsCfg = pCmd->ulNumPacketsDone;
    pCmd->eDest = BHDM_P_Mhl_CbusDest_eMscResp;

    if(pCmd->ulNumPacketsDone)
    {
        pCmd->eState = BHDM_P_Mhl_CbusCmdState_eIncoming;

        for (i = 0; i < pCmd->ulNumPacketsDone; i += 2)
        {
            uint32_t ulPktVal = BREG_Read32(hRegister, ulInPktReg);
            uint16_t ulPktVal16 = ulPktVal & 0xFFFF; /* lower packet */

            BHDM_P_Mhl_CbusPktDirection ePktDir;
            BHDM_P_Mhl_CbusPktType ePktType;
            uint8_t ucPktData;

            BHDM_P_Mhl_Cbus_PktUnpack_isr(ulPktVal16, &ePktType, &ePktDir, &ucPktData);

            pCmd->cbusPackets.astLongCmd[i].ulType = ePktType;
            pCmd->cbusPackets.astLongCmd[i].ulDir  = ePktDir;
            pCmd->cbusPackets.astLongCmd[i].ucData = ucPktData;

            if(pCmd->ulNumPacketsDone - i > 1)
            {
                ulPktVal16 = ulPktVal >> 16;
                BHDM_P_Mhl_Cbus_PktUnpack_isr(ulPktVal16, &ePktType,    &ePktDir, &ucPktData);

                pCmd->cbusPackets.astLongCmd[i+1].ulType = ePktType;
                pCmd->cbusPackets.astLongCmd[i+1].ulDir  = ePktDir;
                pCmd->cbusPackets.astLongCmd[i+1].ucData = ucPktData;
            }

            ulInPktReg += 4;
        }
    }
    else
    {
        ret = BHDM_P_MHL_CBUS_ERROR; /* No packets */
    }

    ulData = BREG_Read32(hRegister, BCHP_MT_MSC_RESP_CTRL);
    ulData &= ~BCHP_MT_MSC_RESP_CTRL_CLEAR_INBOUND_BUFFER_MASK;
    ulData |= BCHP_FIELD_DATA(MT_MSC_RESP_CTRL, CLEAR_INBOUND_BUFFER, 1);
    BREG_Write32(hRegister, BCHP_MT_MSC_RESP_CTRL, ulData);

    return ret;
}

/* Clear ABORT functions. */
void BHDM_P_Mhl_MscResp_ClearAbort_isr
    ( BHDM_P_Mhl_MscResp_Handle            hResp )
{
    BREG_Handle hRegister;
    uint32_t ulData;
    BDBG_ASSERT(hResp);
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);

    hRegister = hResp->hRegister;

    BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, 0);
    ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ABORT_CTRL);
    ulData &= ~BCHP_MT_CBUS_ABORT_CTRL_CLEAR_MSC_MASK;
    ulData |= BCHP_FIELD_DATA(MT_CBUS_ABORT_CTRL, CLEAR_MSC, 1);
    BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, ulData);
}

/* Send ABORT on the MSC channel. Note that BOTH requester and responder
   use the same control. */
void BHDM_P_Mhl_MscResp_SendAbort_isr
    ( BHDM_P_Mhl_MscResp_Handle   hResp,
      BHDM_P_Mhl_MscReq_Event    *peMscReqEvent )
{
    BREG_Handle hRegister;
    uint32_t ulData;
    BDBG_ASSERT(hResp);
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);

    hRegister = hResp->hRegister;

    BHDM_P_Mhl_MscResp_Active_isr(hResp, &hResp->bActive, &hResp->bIbAbortActive, &hResp->bObAbortActive);

    if(hResp->bObAbortActive)
    {
        BDBG_MSG(("MSC OB ABORT currently active!"));
    }

    if (hResp->bIbAbortActive)
    {
        BDBG_MSG(("MSC IB ABORT currently active!"));
    }

    /* Don't send another abort if already sent one or received one */
    if(!hResp->bObAbortActive && !hResp->bIbAbortActive && !hResp->bAbortSent)
    {
        BDBG_MSG(("Sending MSC OB ABORT"));

        /* Before sending ABORT, reset the responder state machine */
        BHDM_P_Mhl_MscResp_Done_isr(hResp);

        /* send abort here itself and post event to MSC REQ task to clear the command Q*/
        hResp->bAbortSent = true;

        /* Send abort */
        ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ABORT_CTRL);
        ulData &= ~BCHP_MT_CBUS_ABORT_CTRL_START_MSC_MASK;
        ulData |= BCHP_FIELD_DATA(MT_CBUS_ABORT_CTRL, START_MSC, 1);
        BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, ulData);

        BHDM_P_Mhl_MscResp_Cancel_isr(hResp, &hResp->stObLastCmd);

        *peMscReqEvent |= BHDM_P_MHL_MSCREQ_EVENT_MSC_TX_ABORT;
    }

#if BHDM_P_MHL_ENABLE_HEARTBEAT_TIMER
    /* Disable heartbeat if Abort active */
    DISABLE_HEARTBEAT;
#endif

}

/* Reset the responder state machine before sending ABORT */
void BHDM_P_Mhl_MscResp_Done_isr
    ( BHDM_P_Mhl_MscResp_Handle            hResp )
{
    BREG_Handle hRegister;
    uint32_t ulData;

    BDBG_ASSERT(hResp);
    BDBG_OBJECT_ASSERT(hResp, BHDM_MHL_RESP);

    hRegister = hResp->hRegister;
    ulData = BREG_Read32(hRegister, BCHP_MT_MSC_RESP_CTRL);
    ulData &= ~(BCHP_MT_MSC_RESP_CTRL_RESPONSE_DONE_MASK);
    ulData |= BCHP_FIELD_DATA(MT_MSC_RESP_CTRL, RESPONSE_DONE, 1);
    BREG_Write32(hRegister, BCHP_MT_MSC_RESP_CTRL, ulData);
}

/*
 * MSC responder sending messages
 * Each reply gets sent immediately in the hardware
 *
 * The content of the reply comes from the bus state,
 * which the caller must set up. Since we always only have
 * at most one outstanding reply all replies are by
 * definition the last command.
 */

BERR_Code BHDM_P_Mhl_MscResp_Cancel_isr
    ( BHDM_P_Mhl_MscResp_Handle  hResp,
      BHDM_P_Mhl_CbusCmd     *pReply )
{
    /* By definition we will only have
       one outstanding responder command */
    if(pReply->ulNumPacketsCfg)
        BHDM_P_Mhl_MscResp_CancelCmd_isr(hResp);
    return BHDM_P_MHL_CBUS_SUCCESS;
}

BERR_Code BHDM_P_Mhl_MscResp_Complete_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd       *pReply )
{
    BSTD_UNUSED(hResp);
    /* Just mark the responder command as done */
    pReply->ulNumPacketsCfg = 0;
    pReply->eState = BHDM_P_Mhl_CbusCmdState_eFree;
    return BHDM_P_MHL_CBUS_SUCCESS;
}



/* Convention:
 * >> - replying command, fixed
 * > -  replying data, passed in as uint8_t
 * << - incoming command, fixed
 * < - incoming data
 */

/*
 * <<SET_HPD
 * >>ACK
 * Side effect: hotplug asserted
 */
BERR_Code BHDM_P_Mhl_MscResp_SetHpd_isr
    ( BHDM_P_Mhl_MscResp_Handle     hResp,
      BHDM_P_Mhl_CbusCmd           *pReply,
      BHDM_P_Mhl_CbusState         *pBusState,
      uint8_t                       ucDelay )
{
    BHDM_P_Mhl_CbusPkt packet = {BHDM_P_Mhl_CbusPktType_eCtrl, BHDM_P_Mhl_CbusPktDirection_eReq,  BHDM_P_Mhl_Command_eAck};

    BDBG_ASSERT(hResp);
    BSTD_UNUSED(pBusState);

#if BHDM_MHL_CTS
    BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, HPD, 1);
#else
    hResp->hDdcReq->bHpd = true;
#endif

    /* Caller needs to set the SET_HPD event */
    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply, &packet, 1, ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
    BDBG_MSG(("Replying "));
#endif

    return BHDM_P_MHL_CBUS_SUCCESS;
}

/*
 * <<CLR_HPD
 * >>ACK
 * Side effect: hotplug deasserted, all DDC commands will be removed from the queue
 */
BERR_Code BHDM_P_Mhl_MscResp_ClrHpd_isr
    ( BHDM_P_Mhl_MscResp_Handle     hResp,
      BHDM_P_Mhl_CbusCmd           *pReply,
      BHDM_P_Mhl_CbusState         *pBusState,
      uint8_t                       ucDelay )
{
    BHDM_P_Mhl_CbusPkt packet = {BHDM_P_Mhl_CbusPktType_eCtrl, BHDM_P_Mhl_CbusPktDirection_eReq,  BHDM_P_Mhl_Command_eAck};

    BSTD_UNUSED(pBusState);

#if BHDM_MHL_CTS
    BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, HPD, 0);
#else
    hResp->hDdcReq->bHpd = false;
#endif

    /* Caller should set CLR_HPD event */

    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply, &packet, 1, ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Replying "));
#endif

    return BHDM_P_MHL_CBUS_SUCCESS;
}

/*
 * <<READ_DEVCAP
 * <offset
 * >>ACK or >>ABORT
 * >value (if ACK)
 * Side effect: none
 */
BERR_Code BHDM_P_Mhl_MscResp_ReadDevCap_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      uint8_t                      ucOffset,
      uint8_t                      ucDelay )
{
    BHDM_P_Mhl_CbusPkt astPacket[2] = {{0, 0, 0}, {0, 0, 0}};
    BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;

    BDBG_ASSERT(hResp);

    /* If the offset is invalid, reply with ABORT instead of ACK */
    if (ucOffset > BHDM_P_Mhl_DevCapOffset_eDevCapAddrMax)
    {
        /* Does not matter which register bank we use
           as they all have the same base */
        ret = BHDM_P_MHL_CBUS_ABORT;
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eBadOffset);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eBadOffset;
#endif
        BDBG_ERR(("Invalid DCAP offset:0x%x", ucOffset));

    }
    else
    {
        astPacket[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPacket[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
        astPacket[0].ucData = BHDM_P_Mhl_Command_eAck;

        astPacket[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPacket[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
        astPacket[1].ucData = pBusState->aucCbusSrcDcap[ucOffset];

    }

    /* Do not reply if we are aborting */
    if(ret == BHDM_P_MHL_CBUS_SUCCESS)
    {
        BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply, astPacket, sizeof(astPacket)/sizeof(astPacket[0]), ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
        BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, sizeof(astPacket)/sizeof(astPacket[0]), BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Replying "));
#endif
    }
    else
    {
        BDBG_MSG(("Aborting"));
    }

    return ret;
}

/*
 * <<SET_INT
 * <offset
 * <value
 * >>ACK or >>ABORT
 * Side effect: setting one of the interrupt bits
 */

 /* TODO:  We need a locking mechanism for unsetting these bits
    as well as a event to signal the parent that something has
    changed.
 */

BERR_Code BHDM_P_Mhl_MscResp_SetInt_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      uint8_t                      ucOffset,
      uint8_t                      ucValue,
      uint8_t                      ucDelay )
{
    BHDM_P_Mhl_CbusPkt packet = {BHDM_P_Mhl_CbusPktType_eCtrl, BHDM_P_Mhl_CbusPktDirection_eReq,  BHDM_P_Mhl_Command_eAck};
    BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
    BREG_Handle hRegister;

    BDBG_ASSERT(hResp);
    BSTD_UNUSED(pBusState);

    hRegister = hResp->hRegister;

    /* Only RCHANGE and DCHANGE registers are defined at the moment.
       We ignore other offsets for now.
       value can never unset an interrupt bit */

    ret = BHDM_P_Mhl_Mailbox_ProcessInt_isr(hRegister, ucOffset, ucValue);

    if (ret != BHDM_P_MHL_CBUS_SUCCESS)
    {
        ret = BHDM_P_MHL_CBUS_ABORT;
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eBadOffset);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eBadOffset;
#endif
        BDBG_ERR(("Invalid SET_INT offset:0x%x", ucOffset));
    }

    /* Do not reply if we are aborting */
    if(ret == BHDM_P_MHL_CBUS_SUCCESS)
    {
        BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                                  &packet,
                                   1,
                                   ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
        BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
        BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Replying "));
#endif
    }
    else
    {
        BDBG_MSG(("Aborting"));
    }

    return ret;
}

/*
 * <<GET_STATE
 * >state (not currently defined in the spec)
 * side effect: none
 */
BERR_Code BHDM_P_Mhl_MscResp_GetState_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      uint8_t                      ucDelay )
{
    /* First dcap register is the device state */
    BHDM_P_Mhl_CbusPkt packet;

    BSTD_UNUSED(hResp);

    packet.ulType = BHDM_P_Mhl_CbusPktType_eData;
    packet.ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    packet.ucData = pBusState->aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eDevStateAddr];

    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                              &packet,
                               1,
                               ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Replying "));
#endif

    return BHDM_P_MHL_CBUS_SUCCESS;
}

/*
 * <<GET_VENDOR_ID
 * >vendor id
 * side effect: none
 */
BERR_Code BHDM_P_Mhl_MscResp_GetVendorId_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd       *pReply,
      BHDM_P_Mhl_CbusState     *pBusState,
      uint8_t                   ucDelay )
{
    /* The spec actually does not say what this vendor id should be */
    BHDM_P_Mhl_CbusPkt packet;

    BSTD_UNUSED(hResp);

    packet.ulType = BHDM_P_Mhl_CbusPktType_eData;
    packet.ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    packet.ucData = pBusState->ucTxVendorId;


    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                              &packet,
                               1,
                               ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Replying "));
#endif

    return BHDM_P_MHL_CBUS_SUCCESS;
}

/*
 * <<WRITE_STAT
 * <offset
 * <value
 * >>ACK or >>ABORT
 * side effect: set status register
 */
BERR_Code BHDM_P_Mhl_MscResp_WriteStat_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd       *pReply,
      BHDM_P_Mhl_CbusState     *pBusState,
      uint8_t                   ucOffset,
      uint8_t                   ucValue,
      uint8_t                   ucDelay )
{
    BHDM_P_Mhl_CbusPkt packet;
    BREG_Handle hRegister;
    BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;

    BSTD_UNUSED(pBusState);
    BDBG_ASSERT(hResp);

    hRegister = hResp->hRegister;

    packet.ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
    packet.ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    packet.ucData = BHDM_P_Mhl_Command_eAck;

    /* Note the implementation of the interrupt register is incomplete */
    ret = BHDM_P_Mhl_Mailbox_ProcessStat_isr(hRegister, ucOffset, ucValue);
    if (ret != BHDM_P_MHL_CBUS_SUCCESS)
    {
        ret = BHDM_P_MHL_CBUS_ABORT;
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eBadOffset);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eBadOffset;
#endif
        BDBG_ERR(("Invalid WRITE_STAT offset:0x%x", ucOffset));
    }

    /* Define IGNORE_TX_READY if you do not have the
       host running so TX_READY is never set.
       Define NACK_PATH_EN to 0 if the sink does not like
       PATH_EN being NACKed. This will delay the sending
       of PATH_EN but will not NACK sink's PATH_EN */
#if !defined(IGNORE_TX_READY) && defined(NACK_PATH_EN)

    if (ucOffset == BHDM_P_Mhl_StatusAddr_eLinkModeAddr &&
        (ucValue & BHDM_P_MHL_STAT_LINK_MODE_PATH_EN) &&
         BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, HOST_TX_READY) == 0)
    {
        /* Set the BUSY error code in case the sink
           asks us why we reply NACK */
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eBusy);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eBusy;
#endif
        packet.ucData = BHDM_P_Mhl_Command_eNack;

        BDBG_MSG(("Host not ready, NACKing sink PATH_EN=1"));
    }
#endif

    /* Do not reply if we are aborting */
    if (ret == BHDM_P_MHL_CBUS_SUCCESS)
    {
        BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                                   &packet,
                                   1,
                                   ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
        BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
        BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Replying "));
#endif
    }
    else
    {
        BDBG_MSG(("Aborting"));
    }

    return ret;
}

/* >>GET_MSC_ERRORCODE/GET_DDC_ERRORCODE
   <Errorcode
   side effect: none
*/
BERR_Code BHDM_P_Mhl_MscResp_GetErrorCode_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      BHDM_P_Mhl_Command           eCmd,
      uint8_t                     ucDelay )
{
    BHDM_P_Mhl_CbusPkt packet;

    BSTD_UNUSED(pBusState);
    BDBG_ASSERT(hResp);

    packet.ulType = BHDM_P_Mhl_CbusPktType_eData;
    packet.ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    packet.ucData = (eCmd == BHDM_P_Mhl_Command_eGetMscErrorCode) ?
#if BHDM_MHL_CTS
                     BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hResp->hRegister, SRC_MSC_ERRORCODE) :
                     BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hResp->hRegister, SRC_DDC_ERRORCODE);
#else
                     hResp->hMscReq->hReq->ucErrCode :
                     hResp->hDdcReq->hReq->ucErrCode;
#endif

    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                               &packet,
                               1,
                               ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
    BDBG_MSG(("Replying "));
#endif

    return BHDM_P_MHL_CBUS_SUCCESS;
}

/*
 * Generic function just replying an ACK or NACK to commands
 * a command
 * Arguments: command queue,
 *            bus state,
 *            command, data1, data2
 *            ack (true) or nack (false)
 *            delay
 * <<Command
 * <data1
 * <data2
 * >>ACK or NACK
 * side effect: none at the moment
 */
BERR_Code BHDM_P_Mhl_MscResp_Ack_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      BHDM_P_Mhl_Command           eCmd,
      uint8_t                      ucData1,
      uint8_t                      ucData2,
      bool                         ucAck,
      uint8_t                      ucDelay )
{
    BHDM_P_Mhl_CbusPkt packet;

    BSTD_UNUSED(hResp);
    BSTD_UNUSED(pBusState);
    BSTD_UNUSED(eCmd);
    BSTD_UNUSED(ucData1);
    BSTD_UNUSED(ucData2);

    packet.ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
    packet.ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    packet.ucData = (ucAck)? BHDM_P_Mhl_Command_eAck : BHDM_P_Mhl_Command_eNack;

    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                               &packet,
                               1,
                               ucDelay);

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
    BDBG_MSG(("Replying "));
#endif

    return BHDM_P_MHL_CBUS_SUCCESS;
}

/* TODO: Handle RCP, just reply busy for now */
BERR_Code BHDM_P_Mhl_MscResp_Rcp_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      BHDM_P_Mhl_Command           eCmd,
      uint8_t                      ucData1,
      uint8_t                      ucData2,
      uint8_t                      ucDelay )
{
    BERR_Code err = BHDM_P_MHL_CBUS_SUCCESS;
    BHDM_P_Mhl_CbusPkt astPackets[4] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    BSTD_UNUSED(hResp);

    BDBG_MSG(("BHDM_P_Mhl_MscResp_Rap_isr"));

    BSTD_UNUSED(pBusState);
    BSTD_UNUSED(eCmd);

    /* First reply ACK to the message */
    astPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
    astPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    astPackets[0].ucData = BHDM_P_Mhl_Command_eAck;

    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                           astPackets,
                           1,
                           ucDelay);

    BDBG_MSG(("MSC MSG < %d < %d", ucData1, ucData2));

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_Mhl_MscMsgDecode_isr(ucData1, ucData2);
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
    BSTD_UNUSED(ucData2);
    BDBG_MSG(("Replying to MSC RESP RCP command"));
#endif

    /* Next if it is RCP, then we reply with RCPE indicating we are busy */
    if (ucData1 == BHDM_P_Mhl_MscMsgCommand_eRcp)
    {
#if 1
#if BHDM_MHL_ENABLE_DEBUG
        /* Command has no response and goes out of MSC requester after the MSC REQ event is generated outside this function  */
        BDBG_MSG(("Also sending MSC MSG"));
        BHDM_P_Mhl_MscMsgDecode_isr(BHDM_P_Mhl_MscMsgCommand_eRcpe, BHDM_P_Mhl_RcpError_eErrorBusy);
#endif
#else
        BHDM_P_Mhl_CbusCmd stMscReqCmd;

        stMscReqCmd.pPacket = astPackets;
        BKNI_Memset(stMscReqCmd.pPacket, 0 , sizeof(BHDM_P_Mhl_CbusPkt) * 4);

        astPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
        astPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPackets[0].ucData = BHDM_P_Mhl_Command_eMscMsg;

        astPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
        astPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPackets[1].ucData = BHDM_P_Mhl_MscMsgCommand_eRcpe;

        astPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eData;
        astPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPackets[2].ucData = BHDM_P_Mhl_RcpError_eErrorBusy;

        astPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
        astPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
        astPackets[3].ucData = BHDM_P_Mhl_Command_eAck;

        /* Prepare and add commnad to MSC REQ Q */
        stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
        stMscReqCmd.ucDelay = ucDelay;
        stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
        stMscReqCmd.ulNumPacketsCfg = 4;
        stMscReqCmd.ulNumPacketsDone = 0;
        stMscReqCmd.bLastCmd = true;
        stMscReqCmd.pucReplyBuf = NULL;
        stMscReqCmd.ucReplyBufSize = 0;
        stMscReqCmd.ucReplyBufValidSize = 0;
        stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
        stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

        BDBG_MSG(("Also sending MSC MSG"));
#if BHDM_MHL_ENABLE_DEBUG
        BHDM_P_Mhl_MscMsgDecode_isr(astPackets[1].ucData, astPackets[2].ucData);
#endif
        /* Command has no response and goes out of MSC requester */
        err = BHDM_P_Mhl_Req_AddCmd_isr(hResp->hMscReq, &stMscReqCmd);
#endif
    }

    return err; /* Always return success as we need to reply ACK */
}

/* Handle RCP, just reply busy for now */
BERR_Code BHDM_P_Mhl_MscResp_Rap_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      BHDM_P_Mhl_Command           eCmd,
      uint8_t                      ucData1,
      uint8_t                      ucData2,
      uint8_t                      ucDelay )
{
    BHDM_P_Mhl_CbusPkt astPackets[4] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    BDBG_MSG(("BHDM_P_Mhl_MscResp_Rcp_isr"));

    BSTD_UNUSED(pBusState);
    BSTD_UNUSED(eCmd);
    BSTD_UNUSED(hResp);

    /* First reply ACK to the message */
    astPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
    astPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    astPackets[0].ucData = BHDM_P_Mhl_Command_eAck;

    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply, astPackets, 1, ucDelay);

    BDBG_MSG(("MSC MSG < %d < %d", ucData1, ucData2));

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_Mhl_MscMsgDecode_isr(ucData1, ucData2);
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Replying ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pReply->cbusPackets.astShortCmd, 1, BHDM_P_Mhl_CbusDest_eMscResp);
#else
    BSTD_UNUSED(ucData2);
    BDBG_MSG(("Replying "));
#endif

    /* Next if it is RAP, then we reply with RAPK indicating we are busy
       except we acknowledge POLL */
    if(ucData1 == BHDM_P_Mhl_MscMsgCommand_eRap)
    {

#if 1
#if BHDM_MHL_ENABLE_DEBUG
        /* Command has no response and goes out of MSC requester after the MSC REQ event is generated outside this function  */
        BDBG_MSG(("Also sending MSC MSG"));
        BHDM_P_Mhl_MscMsgDecode_isr(BHDM_P_Mhl_MscMsgCommand_eRapk,
            ((ucData2 == BHDM_P_Mhl_RapAction_ePoll) ? BHDM_P_Mhl_RapError_eNone : BHDM_P_Mhl_RapError_eBusy));
#endif
#else
        stMscReqCmd.pPacket = astPackets;
        BKNI_Memset(stMscReqCmd.pPacket, 0 , sizeof(BHDM_P_Mhl_CbusPkt) * 4);

        astPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
        astPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPackets[0].ucData = BHDM_P_Mhl_Command_eMscMsg;

        astPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
        astPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPackets[1].ucData = BHDM_P_Mhl_MscMsgCommand_eRapk;

        astPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eData;
        astPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
        astPackets[2].ucData = (ucData2 == BHDM_P_Mhl_RapAction_ePoll)?
            BHDM_P_Mhl_RapError_eNone : BHDM_P_Mhl_RapError_eBusy;

        astPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
        astPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
        astPackets[3].ucData = BHDM_P_Mhl_Command_eAck;

        BDBG_MSG(("Also sending MSC MSG "));

#if BHDM_MHL_ENABLE_DEBUG
        BHDM_P_Mhl_MscMsgDecode_isr(astPackets[1].ucData, astPackets[2].ucData);
#endif
        /* Prepare and add commnad to MSC REQ Q */
        stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
        stMscReqCmd.ucDelay = ucDelay;
        stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
        stMscReqCmd.ulNumPacketsCfg = 4;
        stMscReqCmd.ulNumPacketsDone = 0;
        stMscReqCmd.bLastCmd = true;
        stMscReqCmd.pucReplyBuf = NULL;
        stMscReqCmd.ucReplyBufSize = 0;
        stMscReqCmd.ucReplyBufValidSize = 0;
        stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
        stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

        /* Command has no response and goes out of MSC requester */
        err = BHDM_P_Mhl_Req_AddCmd_isr(hResp->hMscReq, &stMscReqCmd);
#endif
    }

    return BHDM_P_MHL_CBUS_SUCCESS; /* Always return success as we need to reply ACK */
}

/* Handle WRITE_BURST.
   <<WRITE_BURST <Offset <ID_H <ID_L <data ... <<EOF
   The response will be:
   ACK if the offset/size are valid,
   NACK otherwise.
   ABORT if EOF is not present.

   Note that at the moment we will only store the first WRITE_BURST.
   We leave it to the host processor to check if the WRITE_BURST indicate
   3D support. If it does, then the host should trigger the sink to send us
   WRITE_BURST again with SET_INT 3D_REQ=1

 */
BERR_Code BHDM_P_Mhl_MscResp_WriteBurst_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pCmd,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      uint8_t                      ucDelay )
{
    uint32_t ulSpLen = 0, i = 1;
    uint16_t usBurstId;
    bool bEofFound = false;
    uint8_t ucOffset = pCmd->cbusPackets.astLongCmd[1].ucData;
    BREG_Handle hRegister;
    BHDM_P_Mhl_CbusPkt packet;

    BDBG_ASSERT(hResp);

    hRegister = hResp->hRegister;

    packet.ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
    packet.ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
    packet.ucData = BHDM_P_Mhl_Command_eAck;

    /* Check the offset is valid first, NOTE the -1
       This is because the EOF packet is counted as part of the
       command */
    if (ucOffset - BHDM_P_MHL_SCRATCH_ADDR_MIN >= BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE-1)
    {
        BDBG_ERR(("WRITE_BURST offset 0x%x invalid!", ucOffset));
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eBadOffset);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eBadOffset;
#endif
        return BHDM_P_MHL_CBUS_ABORT;
    }

    /* Check if there is any data at all */
    if (pCmd->ulNumPacketsDone <= 4)
    {
        BDBG_ERR(("WRITE_BURST cmd is too short, aborting!"));
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eProtocolError);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eProtocolError;
#endif
        return BHDM_P_MHL_CBUS_ABORT;
    }

    /* First work out how many bytes we are writing */
    for (i = 2; i < pCmd->ulNumPacketsDone; i++)
    {
        if(pCmd->cbusPackets.astLongCmd[i].ucData == BHDM_P_Mhl_Command_eEof)
        {
            bEofFound = true;
            break;
        }

        if(ulSpLen < BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE)
        {
            pBusState->aucScratchReg[ulSpLen++] = pCmd->cbusPackets.astLongCmd[i].ucData;
        }
        else
        {
            BDBG_ERR(("WRITE_BURST exceeded scratch pad size byte %d, aborting!", ulSpLen));
#if BHDM_MHL_CTS
            BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eBadOffset);
#else
            hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eBadOffset;
#endif
            return BHDM_P_MHL_CBUS_ABORT;
        }
    }

    /* No EOF, so we abort */
    if(!bEofFound)
    {
        BDBG_ERR(("WRITE_BURST command does not have EOF, aborting!"));
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eProtocolError);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eProtocolError;
#endif
        return BHDM_P_MHL_CBUS_ABORT;
    }

    /* Check if the sink has set REQ_WRT */
    if (!BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_REQ_WRT))
    {
        BDBG_ERR(("Sink has not set REQ_WRT prior to WRITE_BURST"));
    }

    BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SINK_REQ_WRT);

    usBurstId = ((uint16_t) (pBusState->aucScratchReg[0]) << 8) | pBusState->aucScratchReg[1];

    BDBG_MSG(("MSC WRITE_BURST setting scatchpad offset/size: %d/%d", ucOffset, ulSpLen));

    if (usBurstId == BHDM_P_MHL_BURST_ID_3D_VIC)
    {
        BDBG_MSG(("MSC WRITE_BURST 3D_VIC received"));
    }

    if (usBurstId == BHDM_P_MHL_BURST_ID_3D_DTD)
    {
        BDBG_MSG(("MSC WRITE_BURST 3D_DTD received"));
    }

    /* TODO: If we already have another WRITE_BURST, drop this request
       and let the host deal with it (likely to be 3D support) */
    if(!BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SCRATCHPAD_VALID))
    {
        BHDM_P_Mhl_Mailbox_SetScratchpad_isr(hRegister, pBusState->aucScratchReg,
                                             ucOffset - BHDM_P_MHL_SCRATCH_ADDR_MIN,
                                             ulSpLen);
#if BHDM_MHL_CTS
        BHDM_P_Mhl_Mailbox_ScratchpadUpdateDone_isr(hRegister);
#else
        hResp->bScratchpadValid = true;
        hResp->bSinkDscrChg = false;
#endif
        /* Mark it as valid regardless of the sink
           sends us DSCR_CHG or not (it should!) */
    }
    else
    {
        BDBG_WRN(("Additional WRITE_BURST sequence dropped"));
    }

    /* If everything is OK, ACK the WRITE_BURST command */
    BHDM_P_Mhl_MscResp_CmdAdd_isr(pReply,
                               &packet,
                               1,
                               ucDelay);

    return BHDM_P_MHL_CBUS_SUCCESS;
}

/* Processing incoming MSC requests
   Arguments: fetched command,
              current CBUS state,
          event to set
              delay for reply

 */
BERR_Code BHDM_P_Mhl_MscResp_ProcessReq_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusCmd          *pCmd,
      BHDM_P_Mhl_CbusCmd          *pReply,
      BHDM_P_Mhl_CbusState        *pBusState,
      BHDM_P_Mhl_MscReq_Event     *peMscReqEvent,
      BHDM_P_Mhl_DdcReq_Event     *peDdcReqEvent,
      uint8_t                      ucDelay )
{
    BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
    BHDM_P_Mhl_Command command;
    uint8_t ucData1;
    uint8_t ucData2;
    BREG_Handle hRegister;

    BDBG_ASSERT(hResp);
    BDBG_ASSERT(pCmd);
    BDBG_ASSERT(pReply);

    hRegister = hResp->hRegister;
    command = (BHDM_P_Mhl_Command) pCmd->cbusPackets.astLongCmd[0].ucData;
    ucData1 = pCmd->cbusPackets.astLongCmd[1].ucData;
    ucData2 = pCmd->cbusPackets.astLongCmd[2].ucData;

    /* There are some commands which firmware does not respond to
       but also does not cause ABORT */
    pReply->ulNumPacketsCfg = 0;

#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Completed ");
    BHDM_P_Mhl_DumpCmdPkts_isr(pCmd->cbusPackets.astLongCmd, pCmd->ulNumPacketsDone, BHDM_P_Mhl_CbusDest_eMscResp);
#else
    BDBG_MSG(("Completed "));
#endif

    switch(command)
    {
    case BHDM_P_Mhl_Command_eWriteStat: /* WRITE_STAT and SET_INT shared the same opcode! */
        if(ucData1 >= BHDM_P_Mhl_StatusAddr_eConnectedRdyAddr)
        {
            ret = BHDM_P_Mhl_MscResp_WriteStat_isr(hResp,
                                                pReply,
                                                pBusState,
                                                ucData1,
                                                ucData2,
                                                ucDelay);
            if (ret == BHDM_P_MHL_CBUS_SUCCESS)
            {
                /* set events and post flag for MSC REQ task to react to DCAP RDY and LINK_MODE status */

                /* The last register changed by WRITE_STAT is in the incoming command */
                uint8_t ucOffset = pCmd->cbusPackets.astLongCmd[1].ucData;
                if (ucOffset == BHDM_P_Mhl_StatusAddr_eLinkModeAddr)
                {
                    /* Received LINK_MODE */
                    *peMscReqEvent |= BHDM_P_MHL_MSCREQ_EVENT_RECEIVED_LINK_MODE; /* as a result of this PATH_EN can be sent by MSC REQ Task, for details check MSC REQ event handler  */

                }
                else if (ucOffset == BHDM_P_Mhl_StatusAddr_eConnectedRdyAddr &&
                         BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_DCAP_RDY))
                {
                    /* Received DCAP_RDY */
                    *peMscReqEvent |= BHDM_P_MHL_MSCREQ_EVENT_RECEIVED_DCAP_RDY; /* as a result of this READ_DEVCAP can be done by MSC REQ Task  */
                }
            }
        }
        else
        {
            ret = BHDM_P_Mhl_MscResp_SetInt_isr(hResp,
                                             pReply,
                                             pBusState,
                                             ucData1,
                                             ucData2,
                                             ucDelay);
            if(ret == BHDM_P_MHL_CBUS_SUCCESS)
            {
                uint8_t ucEdidChg = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_EDID_CHG);
                uint8_t ucDcapChg = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_DCAP_CHG);
                uint8_t ucReqWrt  = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_REQ_WRT);
                uint8_t ucDscrChg = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_DSCR_CHG);

                /* set appropriate events for MSC REQ and DDC REQ tasks to react to the SET_INT Command based on interrupt bits set */
                /* events for MSC REQ Task */
                if (ucDcapChg)
                {
                    *peMscReqEvent |= BHDM_P_MHL_MSCREQ_EVENT_DCAP_CHG_INT; /* as a result of this READ_DEVCAP will be done by MSC REQ Task  */
                }

                if (ucReqWrt)
                {
                    *peMscReqEvent |= BHDM_P_MHL_MSCREQ_EVENT_REQ_WRT_INT; /* as a result of this SET_INT:GRT_WRT will be sent by MSC REQ Task  */
                }
                if (ucDscrChg)
                {
                    /* Nothing to do other than mark the scratch pad as valid */
                    BDBG_MSG(("DSCR_CHG is set."));
#if BHDM_MHL_CTS
                    BHDM_P_Mhl_Mailbox_ScratchpadUpdateDone_isr(hRegister);
#else
                    hResp->bScratchpadValid = true;
                    hResp->bSinkDscrChg = false;
#endif
                }

                /* events for DDC REQ Task */
                if (ucEdidChg)
                {
                    *peDdcReqEvent |= BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT; /* EDID read will be done by DDC REQ task */
                }
            }
        }

        break;

    case BHDM_P_Mhl_Command_eReadDevCap:
        ret = BHDM_P_Mhl_MscResp_ReadDevCap_isr(hResp, pReply, pBusState, ucData1, ucDelay);
        break;

    case BHDM_P_Mhl_Command_eGetState:
        ret = BHDM_P_Mhl_MscResp_GetState_isr(hResp, pReply, pBusState, ucDelay);
        break;

    case BHDM_P_Mhl_Command_eGetVendorId:
        ret = BHDM_P_Mhl_MscResp_GetVendorId_isr(hResp, pReply, pBusState, ucDelay);
        break;

    case BHDM_P_Mhl_Command_eSetHpd:
        BDBG_ERR(("SET HPD command "));
        ret = BHDM_P_Mhl_MscResp_SetHpd_isr(hResp, pReply, pBusState, ucDelay);
        /* Set event for DDC REQ task to react to SET_HPD command from Sink */
        *peDdcReqEvent |= BHDM_P_MHL_DDCREQ_EVENT_SET_HPD;
        break;

    case BHDM_P_Mhl_Command_eClrHpd:
        ret = BHDM_P_Mhl_MscResp_ClrHpd_isr(hResp, pReply, pBusState, ucDelay);
        /* Set event for DDC REQ task to react to CLR_HPD command from Sink */
        *peDdcReqEvent |= BHDM_P_MHL_DDCREQ_EVENT_CLR_HPD;
        break;

    case BHDM_P_Mhl_Command_eGetDdcErrorCode:
    case BHDM_P_Mhl_Command_eGetMscErrorCode:
        ret = BHDM_P_Mhl_MscResp_GetErrorCode_isr(hResp,
                                           pReply,
                                           pBusState,
                                           command,
                                           ucDelay);
        break;

    case BHDM_P_Mhl_Command_eMscMsg:
        /* We mark ourselves as busy to RCP and RAP commands */
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eBusy);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eBusy;
#endif

        if (ucData1 == BHDM_P_Mhl_MscMsgCommand_eRcp ||
            ucData1 == BHDM_P_Mhl_MscMsgCommand_eRcpk ||
            ucData1 == BHDM_P_Mhl_MscMsgCommand_eRcpe)
        {
            ret = BHDM_P_Mhl_MscResp_Rcp_isr(hResp,
                                      pReply,
                                      pBusState,
                                      command,
                                      ucData1,
                                      ucData2,
                                      ucDelay);
            /* if required generate MSC REQ event to send RCPE */
            if(ucData1 == BHDM_P_Mhl_MscMsgCommand_eRcp)
            {
                *peMscReqEvent |= BHDM_P_MHL_MSCREQ_EVENT_SEND_RCPE_ERR_BUSY;
            }

        }
        else if (ucData1 == BHDM_P_Mhl_MscMsgCommand_eRap ||
                 ucData1 == BHDM_P_Mhl_MscMsgCommand_eRapk)
        {
            ret = BHDM_P_Mhl_MscResp_Rap_isr(hResp,
                                      pReply,
                                      pBusState,
                                      command,
                                      ucData1,
                                      ucData2,
                                      ucDelay);

            /* if required generate MSC REQ event to send RAPK */
            if(ucData1 == BHDM_P_Mhl_MscMsgCommand_eRap)
            {
                *peMscReqEvent |= (ucData2 == BHDM_P_Mhl_RapAction_ePoll) ?
                    BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_NONE : BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_BUSY;
            }

        }
        else
        {
            /* Any other subcommand, we NACK */
            ret = BHDM_P_Mhl_MscResp_Ack_isr(hResp,
                                      pReply,
                                      pBusState,
                                      command,
                                      ucData1,
                                      ucData2,
                                      false,
                                      ucDelay);
        }
        break;

    case BHDM_P_Mhl_Command_eWriteBurst:
        ret = BHDM_P_Mhl_MscResp_WriteBurst_isr(hResp, pCmd, pReply, pBusState, ucDelay);
        break;

    /* The MHL spec currently does not define responses for these
       commands. However, the CTS does not allow enough time
       for ABORT, so we ignore these commands for now, but still
       need to reset the responder. */
    case BHDM_P_Mhl_Command_eGetSc1ErrorCode:
    case BHDM_P_Mhl_Command_eGetSc3ErrorCode:
        /* DO NOT RESPOND, just reset the responder  */
        BHDM_P_Mhl_MscResp_Done_isr(hResp);
        break;

    default:
        /* Aborting unrecognised commands */
        ret = BHDM_P_MHL_CBUS_ABORT;
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, BHDM_P_Mhl_CbusMscErr_eInvalidOp);
#else
        hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eInvalidOp;
#endif
        BDBG_ERR(("Unsupported MSC pCmd << %d", command));
        break;
    }

    BDBG_MSG(("MSC resp processed"));

    return ret;
}

BERR_Code BHDM_P_Mhl_MscResp_CheckError_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      int                       interrupt,
      bool                     *pbAbortRequired,
      BHDM_P_Mhl_CbusPkt       *pLastPacket )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ASSERT(hResp);
    BSTD_UNUSED(pLastPacket);

    /* Added for consistency with cbus_common_msc_req_check_error */
    *pbAbortRequired = false;

    switch (interrupt)
    {
        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_SW_TIMEOUT):
            /* Errors which do not update error code. */
            BDBG_ERR(("MSC responder sw timeout!"));
            break;

        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_ILLEGAL_SW_WR):
            BDBG_ERR(("MSC responder illegal sw write!"));
            break;

        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_RX_TIMEOUT):
            /* Errors which update error code, but not cause an ABORT */
#if BHDM_MHL_CTS
            BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, SRC_MSC_ERRORCODE,
                                     BHDM_P_Mhl_CbusMscErr_eTimeout);
#else
            hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eTimeout;
#endif
            BDBG_ERR(("MSC responder rx timeout!"));
            break;

        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_MAX_RETRIES_EXCEEDED):
#if BHDM_MHL_CTS
            BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, SRC_MSC_ERRORCODE,
                                     BHDM_P_Mhl_CbusMscErr_eRetryFailed);
#else
            hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eRetryFailed;
#endif
            BDBG_ERR(("MSC responder max retries exceeded!"));
            BHDM_P_Mhl_MscResp_Done_isr(hResp);
            break;

        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_UNEXPECTED_INBOUND_PKT):
        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_UNEXPECTED_CMD):
        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_DATA_OVERFLOW):
            /* Errors which both update error code and cause an ABORT */
            /* Need to send abort */
            *pbAbortRequired = true;
#if BHDM_MHL_CTS
            BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, SRC_MSC_ERRORCODE,
                                     BHDM_P_Mhl_CbusMscErr_eProtocolError);
#else
            hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eProtocolError;
#endif
            BDBG_ERR(("MSC responder protocol error!"));
            break;

        case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_BAD_CMD):
            /* Need to send abort */
            *pbAbortRequired = true;
#if BHDM_MHL_CTS
            BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, SRC_MSC_ERRORCODE,
                                     BHDM_P_Mhl_CbusMscErr_eInvalidOp);
#else
            hResp->hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eInvalidOp;
#endif
            BDBG_ERR(("MSC responder bad cmd"));
            break;
    }

    BDBG_ERR(("MSC resp error: intr=0x%x", interrupt));

    return err;
}

uint32_t BHDM_P_Mhl_MscResp_GetIntStatus_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp )
{
    BREG_Handle hRegister;
    uint32_t  ulStatus;

    BDBG_ASSERT(hResp);

    hRegister = hResp->hRegister;

    ulStatus = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_STATUS);
    ulStatus &= BHDM_P_MHL_RESP_INTERRUPT_MASK;

    return ulStatus;
}

/* MSC Responder event handlers */
void BHDM_P_Mhl_MscResp_HandleAbortMscObTimeoutDone_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp )
{

    BDBG_MSG(("MSC OB ABORT timer expired"));

    /* Trigger MSC_CLEAR in ABORT_CTRL register so MSC Req/Resp state machines could resume to normal */
    BHDM_P_Mhl_MscResp_ClearAbort_isr(hResp);

    /* assign all MSC abort actives to false */
    hResp->bAbortSent = false;

    /* If we have an outstanding response to send. If we have, we must send that first */
    if(hResp->bPending)
    {
        hResp->bPending  = false;

#if BHDM_MHL_ENABLE_DEBUG
        BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Resume sending ");
        BHDM_P_Mhl_DumpCmdPkts_isr(hResp->stObLastCmd.cbusPackets.astShortCmd,
                                    hResp->stObLastCmd.ulNumPacketsCfg,
                                    BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Resume sending "));
#endif
        BHDM_P_Mhl_MscResp_SendCmd_isr(hResp, & hResp->stObLastCmd);

#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, MSC_RESP_PENDING, 1);
#else
        hResp->bPending  = true;
#endif
    }

#if BHDM_P_MHL_ENABLE_HEARTBEAT_TIMER
    /* enable heartbeat if abort done */
    RESET_HEARTBEAT;
    ENABLE_HEARTBEAT;
#endif
}

void BHDM_P_Mhl_MscResp_HandleIbDone_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      BHDM_P_Mhl_CbusState        *pstCbusState,
      BHDM_P_Mhl_MscReq_Event     *peMscReqEvent,
      BHDM_P_Mhl_DdcReq_Event     *peDdcReqEvent )
{
    BERR_Code ret;
    bool bHasIbPackets = false;

    /* Fetch the incoming command and process the request */
    ret = BHDM_P_Mhl_MscResp_ReceiveCmd_isr(hResp, &hResp->stIbLastCmd);

    if(ret == BHDM_P_MHL_CBUS_SUCCESS)
    {
        bHasIbPackets = true;
    }
    else
    {
        BDBG_ERR(("Error receiving MSC inbound command."));
    }

    /* If there were any IB packets, send response if needed */
    if(bHasIbPackets)
    {
#if BHDM_MHL_CTS
        BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, MSC_RESP_PENDING, 1);
#else
        hResp->bPending = true;
#endif
        ret = BHDM_P_Mhl_MscResp_ProcessReq_isr(hResp, &hResp->stIbLastCmd, &hResp->stObLastCmd,
                                                pstCbusState, peMscReqEvent, peDdcReqEvent, 0); /* Default 0 tick delay in reply */

        if (ret == BHDM_P_MHL_CBUS_ABORT)
        {
            BHDM_P_Mhl_MscResp_SendAbort_isr(hResp, peMscReqEvent);
        }
        else if (hResp->stObLastCmd.ulNumPacketsCfg > 0)
        {
            /* We have a MSC response to send,  we can always send it even if a request  has been queued.
            However, if MSC ABORT is currently active, we must wait. */
            BHDM_P_Mhl_MscResp_Active_isr(hResp, &hResp->bActive, &hResp->bIbAbortActive, &hResp->bObAbortActive);

            if(!hResp->bObAbortActive)
            {
#if BHDM_MHL_ENABLE_DEBUG
                BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Sending ");
                BHDM_P_Mhl_DumpCmdPkts_isr(hResp->stObLastCmd.cbusPackets.astShortCmd,
                                            hResp->stObLastCmd.ulNumPacketsCfg,
                                            BHDM_P_Mhl_CbusDest_eMscResp);
#else
                BDBG_MSG(("Sending "));
#endif

                BHDM_P_Mhl_MscResp_SendCmd_isr(hResp, &hResp->stObLastCmd);
#if BHDM_MHL_CTS
                BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hResp->hRegister, MSC_RESP_PENDING, 1);
#else
                hResp->bPending = true;
#endif
            }
            else
            {
                hResp->bPending = true;
                BDBG_WRN(("MSC OB ABORT active, delaying sending response"));
            }
        }
    } /* If we have incoming packets to be processed */

}

void BHDM_P_Mhl_MscResp_HandleObDone_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp )
{
#if BHDM_MHL_ENABLE_DEBUG
    BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Completed ");
    BHDM_P_Mhl_DumpCmdPkts_isr(hResp->stObLastCmd.cbusPackets.astShortCmd,
                                hResp->stObLastCmd.ulNumPacketsCfg,
                                BHDM_P_Mhl_CbusDest_eMscResp);
#else
    BDBG_MSG(("Completed "));
#endif

    hResp->bActive = false;
    BHDM_P_Mhl_MscResp_Complete_isr(hResp, &hResp->stObLastCmd);
    BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hResp->hRegister, MSC_RESP_PENDING);
}

void BHDM_P_Mhl_MscResp_HandleErrors_isr
    ( BHDM_P_Mhl_MscResp_Handle    hResp,
      int                          interrupt,
      BHDM_P_Mhl_MscReq_Event     *peMscReqEvent )
{
    bool bAbortRequired;

    BHDM_P_Mhl_MscResp_CheckError_isr(hResp, interrupt,
                                    &bAbortRequired,
                                    &hResp->stIbLastCmd.cbusPackets.astLongCmd[hResp->stIbLastCmd.ulNumPacketsDone-1]);

    if(bAbortRequired)
    {
        BHDM_P_Mhl_MscResp_ReceiveCmd_isr(hResp, &hResp->stIbLastCmd);
#if BHDM_MHL_ENABLE_DEBUG
        BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Incoming packets: ");
        BHDM_P_Mhl_DumpCmdPkts_isr(hResp->stIbLastCmd.cbusPackets.astLongCmd,
                                    hResp->stIbLastCmd.ulNumPacketsDone,
                                    BHDM_P_Mhl_CbusDest_eMscResp);
#else
        BDBG_MSG(("Incoming packets: "));
#endif

        BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hResp->hRegister, MSC_RESP_PENDING);

        BHDM_P_Mhl_MscResp_SendAbort_isr(hResp, peMscReqEvent); /* send ABORT if no abort is active already */
    }
}
