/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
******************************************************************************/
#include "bhdm_mhl_msc_req_priv.h"

BDBG_MODULE(BHDM_MHL_MSC_REQ);
BDBG_OBJECT_ID(BHDM_MHL_MSC_REQ);

#define BHDM_P_MHL_NUM_PKTS_READ_DEVCAP_CMD   (4)
#define BHDM_P_MHL_NUM_PKTS_GET_STATE_CMD     (2)
#define BHDM_P_MHL_NUM_PKTS_RCPE_CMD          (4)
#define BHDM_P_MHL_NUM_PKTS_RAPK_CMD          (4)


extern const BHDM_P_Mhl_CbusPkt s_astSafeHbPkts;
extern uint32_t s_ulSafeHbPktCnt;


static BERR_Code BHDM_P_Mhl_MscReq_GetDdcErrorCode_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                  *pucErr,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	return BHDM_P_Mhl_Req_1DataCmd_isr(hMscReq->hReq,
									BHDM_P_Mhl_Command_eGetDdcErrorCode,
									pucErr,
									ucDelay,
									bLastCmd);
}

static BERR_Code BHDM_P_Mhl_MscReq_GetMscErrorCode_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                  *pucErr,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	return BHDM_P_Mhl_Req_1DataCmd_isr(hMscReq->hReq,
									BHDM_P_Mhl_Command_eGetMscErrorCode,
									pucErr,
									ucDelay,
									bLastCmd);
}

BERR_Code BHDM_P_Mhl_MscReq_Create
	( BHDM_P_Mhl_MscReq_Handle    *phMscReq )
{
	BERR_Code rc = BERR_SUCCESS;
	BHDM_P_Mhl_MscReq_Object *pMscReq;

	*phMscReq = NULL;

	/* Alloc the context. */
	pMscReq = (BHDM_P_Mhl_MscReq_Object *)(BKNI_Malloc(sizeof(BHDM_P_Mhl_MscReq_Object)));
	if(!pMscReq)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* zero out all memory associated with the HDMI Device Handle before using */
	BKNI_Memset((void *) pMscReq, 0, sizeof(BHDM_P_Mhl_MscReq_Object));
	BDBG_OBJECT_SET(pMscReq, BHDM_MHL_MSC_REQ);

	rc = BHDM_P_Mhl_Req_Create(&pMscReq->hReq);
	if (rc != BERR_SUCCESS) return rc;

	pMscReq->hReq->eType = BHDM_P_Mhl_ReqType_eMsc;

	pMscReq->hReq->ulBaseReg = BCHP_MT_MSC_REQ_REG_START;
	pMscReq->hReq->ulRegCount = BHDM_P_MHL_REGS_ENTRIES(MT_MSC_REQ_REG_START, MT_MSC_REQ_REG_END);

	pMscReq->pucSinkDcap = (uint8_t *)BKNI_Malloc(sizeof(uint8_t) * BHDM_P_MHL_SINK_DCAP_SIZE);
	if (pMscReq->pucSinkDcap == NULL)
	{
		BDBG_ERR(("Failed to allocate memory for sink's DCAP buffer."));
		return BHDM_P_MHL_CBUS_NOMEMORY;
	}

	/* keep created pointer */
	*phMscReq = (BHDM_P_Mhl_MscReq_Handle)pMscReq;

	return rc;
}

BERR_Code BHDM_P_Mhl_MscReq_Destroy
	( BHDM_P_Mhl_MscReq_Handle        hMscReq )
{
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	BHDM_P_Mhl_Fifo_Uninit(hMscReq->hReq->pstCmdQ);

	BKNI_Free((void *)hMscReq->pucSinkDcap);

	BHDM_P_Mhl_Req_Destroy(hMscReq->hReq);

	/* free memory associated with the DDC_REQ handle */
	BDBG_OBJECT_DESTROY(hMscReq, BHDM_MHL_MSC_REQ );
	BKNI_Free((void *)hMscReq);

	hMscReq = NULL;

	return BERR_SUCCESS;
}

BERR_Code BHDM_P_Mhl_MscReq_Init
	( BHDM_P_Mhl_MscReq_Handle        hMscReq,
	  BREG_Handle                     hRegister )
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ASSERT(hMscReq);
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	/* Init Requester object */
	rc = BHDM_P_Mhl_Req_Init(hMscReq->hReq, hRegister);

	/* Initialize command Q. */
	BHDM_P_Mhl_Fifo_Init(hMscReq->hReq->pstCmdQ, (unsigned long)&hMscReq->astCmdQueue[0], sizeof(hMscReq->astCmdQueue));
	BKNI_Memset((void *)&hMscReq->astCmdQueue[0], 0, sizeof(hMscReq->astCmdQueue));

	/* Initialize */
	hMscReq->bDcapValid = false;
	hMscReq->bSinkDcapChg = false;
	hMscReq->eEvent = BHDM_P_MHL_MSCREQ_EVENT_NONE;

	return rc;
}

void BHDM_P_Mhl_MscReq_RemoveAllCmds_isr
	( BHDM_P_Mhl_MscReq_Handle   hMscReq )
{
#if BHDM_MHL_ENABLE_DEBUG
	struct BHDM_P_Mhl_CbusCmd stCmd;

	BDBG_ASSERT(hMscReq);
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	while (!BHDM_P_Mhl_Fifo_ReadData_isr(hMscReq->hReq->pstCmdQ, (uint8_t *)&stCmd, sizeof(stCmd)))
	{
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Removed ");
		BHDM_P_Mhl_DumpCmdPkts_isr(&stCmd.cbusPackets.astShortCmd[0], stCmd.ulNumPacketsCfg, BHDM_P_Mhl_CbusDest_eMscReq);
	}

#else
	BDBG_ASSERT(hMscReq);
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);
	hMscReq->hReq->pstCmdQ->ulReadAddr = hMscReq->hReq->pstCmdQ->ulWriteAddr;
#endif

}

BERR_Code BHDM_P_Mhl_MscReq_CancelAllCmds_isr
	( BHDM_P_Mhl_MscReq_Handle   hMscReq )
{
	BDBG_ASSERT(hMscReq);
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	BDBG_WRN(("Flushing all MSC cmds"));

	/* cancel last ongoing command in HW */
	if(hMscReq->hReq->stLastCmd.eState == BHDM_P_Mhl_CbusCmdState_eSent)
	{
		BHDM_P_Mhl_Req_CancelCmd_isr(hMscReq->hReq);

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Cancelled ");
		BHDM_P_Mhl_DumpCmdPkts_isr(&hMscReq->hReq->stLastCmd.cbusPackets.astLongCmd[0],
									hMscReq->hReq->stLastCmd.ulNumPacketsCfg,
									BHDM_P_Mhl_CbusDest_eMscReq);
#else
		BDBG_MSG(("Cancelled all MSC REQ commands"));

#endif
	}

	/* Remove all the commands from the MSC command Q */
	BHDM_P_Mhl_MscReq_RemoveAllCmds_isr(hMscReq);

	return BHDM_P_MHL_CBUS_SUCCESS;
}

/* Send a batch of 16 read devcap commands
   when either DCAP change is indicated or
   SET_HPD is first received. Note all 16 commands
   will go out on the MSC requester, NOT on the
   responder.
 */


/**
 * Functions for sending predefined MSC requester
 * commands.
 * Each of the following functions basically construct
 * the correct command and add it to the queue.
 * The bLastCmd argument should be set to true if it is
 * the last command of a batch of commands
 * (or the only command) to be queued.
 *
 * After a message has been add, the caller calls
 * cbus_cmd_send_next to send the message, which
 * will check if the channel is active before sending
 * the message.
 */

/*
 * >>READ_DEVCAP
 * >offset
 * <<ACK
 * <value
 */
BERR_Code BHDM_P_Mhl_MscReq_ReadDevCap_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                   ucOffset,
	  uint8_t                  *pucValue,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BERR_Code ret = BERR_SUCCESS;
	bool bIsFifoFull;

	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;

	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	BDBG_MSG(("BHDM_P_Mhl_MscReq_ReadDevCap_isr"));

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = 	BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eReadDevCap;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ucData = ucOffset;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[2].ucData = BHDM_P_Mhl_Command_eAck;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[3].ucData = 0;


	/* Prepare and add commnad to MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eDcap;
	stMscReqCmd.ucDelay = ucDelay;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_READ_DEVCAP_CMD;
	stMscReqCmd.ulNumPacketsDone = 0;
	stMscReqCmd.bLastCmd = bLastCmd;
	stMscReqCmd.pucReplyBuf = pucValue;
	stMscReqCmd.ucReplyBufSize = 1;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	bIsFifoFull = BHDM_P_Mhl_Fifo_WriteData_isr(hMscReq->hReq->pstCmdQ,
											(uint8_t *)&stMscReqCmd,
											sizeof(stMscReqCmd));
	if (bIsFifoFull)
	{
		BDBG_ERR(("Command Q full %d pkts dropped", stMscReqCmd.ulNumPacketsCfg));
	}
	else
	{
#if BHDM_MHL_CTS
		BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hMscReq->hReq->hRegister, MSC_REQ_PENDING, 1);
#else
		hMscReq->hReq->bRequestPending = true;
#endif


#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Added ");
		BHDM_P_Mhl_DumpCmdPkts_isr(pPackets, stMscReqCmd.ulNumPacketsCfg, BHDM_P_Mhl_CbusDest_eMscReq);
		BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#else
		BDBG_MSG(("Added MSC REQ ReadDevCap command"));
#endif
	}
	return ret;
}

/*
 * >>GET_STATE
 * <STATE
 */
BERR_Code BHDM_P_Mhl_MscReq_GetState
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                  *pucState,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BERR_Code ret = BERR_SUCCESS;

	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;

	BDBG_ASSERT(hMscReq);
	BDBG_MSG(("BHDM_P_Mhl_MscReq_GetState"));

	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eGetState;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[1].ucData = 0;

	/* Prepare and add commnad to MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
	stMscReqCmd.ucDelay = ucDelay;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_GET_STATE_CMD;
	stMscReqCmd.ulNumPacketsDone = 0;
	stMscReqCmd.bLastCmd = bLastCmd;
	stMscReqCmd.pucReplyBuf = pucState;
	stMscReqCmd.ucReplyBufSize = 1;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

#if 1
	BKNI_EnterCriticalSection();
	ret = BHDM_P_Mhl_Req_AddCmd_isr(hMscReq->hReq, &stMscReqCmd);
	BKNI_LeaveCriticalSection();
#else

	BKNI_EnterCriticalSection();
	bIsFifoFull = BHDM_P_Mhl_Fifo_WriteData_isr(hMscReq->hReq->pstCmdQ,
											(uint8_t *)&stMscReqCmd,
											sizeof(stMscReqCmd));
	BKNI_LeaveCriticalSection();

	if (bIsFifoFull)
	{
		BDBG_ERR(("Command Q full %d pkts dropped", stMscReqCmd.ulNumPacketsCfg));
	}
	else
	{
		BHDM_P_MHL_MAILBOX_SET_FIELD(hMscReq->hReq->hRegister, MSC_REQ_PENDING, 1);
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Added ");
		BHDM_P_Mhl_DumpCmdPkts_isr(pPackets, stMscReqCmd.ulNumPacketsCfg, BHDM_P_Mhl_CbusDest_eMscReq);
#endif
	}
#endif

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#else
	BDBG_MSG(("Added MSC REQ GetState command"));
#endif

	return ret;
}

BERR_Code BHDM_P_Mhl_MscReq_HeartBeat_isr
	( BHDM_P_Mhl_MscReq_Handle     hMscReq,
	  const BHDM_P_Mhl_CbusPkt    *packets,
	  uint32_t                     ulNumPackets,
	  uint8_t                      ucDelay,
	  bool                         bLastCmd )
{
	BERR_Code ret = BERR_SUCCESS;
	bool bIsFifoFull;

	BHDM_P_Mhl_CbusCmd stMscReqCmd;

	BDBG_MSG(("BHDM_P_Mhl_MscReq_HeartBeat_isr"));

	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	BDBG_ASSERT(sizeof(stMscReqCmd.cbusPackets.astShortCmd) >= ulNumPackets);

	BKNI_Memcpy(&stMscReqCmd.cbusPackets.astShortCmd[0], packets, sizeof(BHDM_P_Mhl_CbusPkt) * ulNumPackets);

	/* Prepare and add commnad to MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eHb;
	stMscReqCmd.ucDelay = ucDelay;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ulNumPacketsCfg = ulNumPackets;
	stMscReqCmd.ulNumPacketsDone = 0;
	stMscReqCmd.bLastCmd = bLastCmd;
	stMscReqCmd.pucReplyBuf = NULL;
	stMscReqCmd.ucReplyBufSize = 0;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	bIsFifoFull = BHDM_P_Mhl_Fifo_WriteData_isr(hMscReq->hReq->pstCmdQ,
											(uint8_t *)&stMscReqCmd,
											sizeof(stMscReqCmd));
	if (bIsFifoFull)
	{
		BDBG_ERR(("Command Q full %d pkts dropped", stMscReqCmd.ulNumPacketsCfg));
	}
	else
	{
#if BHDM_MHL_CTS
		BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hMscReq->hReq->hRegister, MSC_REQ_PENDING, 1);
#else
		hMscReq->hReq->bRequestPending = true;
#endif

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Added ");
		BHDM_P_Mhl_DumpCmdPkts_isr(&stMscReqCmd.cbusPackets.astShortCmd[0], stMscReqCmd.ulNumPacketsCfg, BHDM_P_Mhl_CbusDest_eMscReq);
		BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#else
		BDBG_MSG(("Added MSC REQ HeartBeat Command."));
#endif
	}
	return ret;
}

/* The 3 commands below have the same format
 * >>WRITE_STAT/SET_INT/MSC_MSG
 * >data1
 * >data2
 * <<ACK
 */
BERR_Code BHDM_P_Mhl_MscReq_WriteStat_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                   ucOffset,
	  uint8_t                   ucValue,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	return BHDM_P_Mhl_Req_2DataCmd_isr(hMscReq->hReq,
										BHDM_P_Mhl_Command_eWriteStat,
										ucOffset,
										ucValue,
										ucDelay,
										bLastCmd);
}

BERR_Code BHDM_P_Mhl_MscReq_SetInt_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                   ucOffset,
	  uint8_t                   ucValue,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	return BHDM_P_Mhl_Req_2DataCmd_isr(hMscReq->hReq,
										BHDM_P_Mhl_Command_eSetInt,
										ucOffset,
										ucValue,
										ucDelay,
										bLastCmd);
}

BERR_Code BHDM_P_Mhl_MscReq_MscMsg
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                   ucData1,
	  uint8_t                   ucData2,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	return BHDM_P_Mhl_Req_2DataCmd_isr(hMscReq->hReq,
										BHDM_P_Mhl_Command_eMscMsg,
										ucData1,
										ucData2,
										ucDelay,
										bLastCmd);
}



/* Check for MSC or DDC requester error pbAbortRequired will be set if we need to send ABORT */
BERR_Code BHDM_P_Mhl_MscReq_CheckError_isr
	( BHDM_P_Mhl_MscReq_Handle        hMscReq,
	  int                             interrupt,
	  bool                           *pbAbortRequired,
	  BHDM_P_Mhl_CbusPkt             *pstLastPacket )
{
	BERR_Code eXmitResult = BHDM_P_MHL_CBUS_XMIT_SUCCESS;

	BDBG_ASSERT(hMscReq);
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	/* Checking error in the following order */
	*pbAbortRequired = false;

	/* Errors which do not update error code.
	   If only MSC_REQ_DONE is set,
	   eXmitResult will not change.
	*/
	switch (interrupt)
	{
		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_CANCELLED):
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_CANCELLED;
			BDBG_ERR(("MSC req cmd cancelled!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_ILLEGAL_SW_WR):
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_ERROR;
			BDBG_ERR(("MSC req illegal sw wr!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_BUS_STOLEN):
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_BUS_STOLEN;
			BDBG_ERR(("MSC req bus stolen!!!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_TIMEOUT):
			/* Errors which update error code but does not cause ABORT */
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_TIMEOUT;
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hMscReq->hReq->hRegister, SRC_MSC_ERRORCODE,
											 BHDM_P_Mhl_CbusMscErr_eTimeout);
#else
			hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eTimeout;
#endif
			BDBG_ERR(("MSC req rx timeout!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_MAX_RETRIES_EXCEEDED):
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_RETRY_EXCEEDED;
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hMscReq->hReq->hRegister, SRC_MSC_ERRORCODE,
										 BHDM_P_Mhl_CbusMscErr_eRetryFailed);
#else
			hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eRetryFailed;
#endif
			BDBG_ERR(("MSC req max retries exceeded!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_UNEXPECTED_INBOUND_PKT):
		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_MISMATCH):
		case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_CMD_ERROR):
			/* Distinguishing between NACK and a protocol error:
			   If we get a NACK from sink, RX_CMD_ERROR will be set,
			   in which case we should NOT send ABORT, any commmand
			   other than NACK will also set this bit, in which case
			   ABORT should be sent.
			   If we get an incorrect packet type (ctrl instead of data
			   or vice versa), RX_MISMATCH will be set.
			   This is a protocol error and ABORT should also be sent.
			*/
			if (pstLastPacket->ucData != BHDM_P_Mhl_Command_eNack)
			{
				eXmitResult = BHDM_P_MHL_CBUS_XMIT_PROTO_ERROR;
				/* Need to send abort. */
				*pbAbortRequired = true;
#if BHDM_MHL_CTS
				BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hMscReq->hReq->hRegister, SRC_MSC_ERRORCODE,
				       BHDM_P_Mhl_CbusMscErr_eProtocolError);
#else
			hMscReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusMscErr_eProtocolError;
#endif

				BDBG_ERR(("MSC req unexpected ib || cmd err || mismatch!"));
			}
			else if (interrupt == MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_CMD_ERROR) &&
					 pstLastPacket->ucData == BHDM_P_Mhl_Command_eNack)
			{
				eXmitResult = BHDM_P_MHL_CBUS_XMIT_NACK;
				BDBG_ERR(("MSC req cmd NACKed!"));
			}
			break;

	}

	if(eXmitResult != BHDM_P_MHL_CBUS_XMIT_SUCCESS)
	{
		BDBG_ERR(("MSC req error: intr1=0x%x", interrupt));
	}

	return eXmitResult;
}
/*
 * >>WRITE_BURST
 * >OFFSET
 * >ADOPTER_ID_H (adopter id passed as uint16_t)
 * >ADOPTER_ID_L
 * >data1 (upto 16 bytes of data)
 * >...
 * >>EOF
 * <<ACK
 */
BERR_Code BHDM_P_Mhl_MscReq_WriteBurst
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  uint8_t                ucOffset,
	  uint16_t               usAdopterId,
	  uint8_t               *pucData,
	  uint32_t               ulNumBytes,
	  uint8_t                ucDelay,
	  bool                   bLastCmd )
{
	uint32_t i;
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astLongCmd;

	BDBG_MSG(("BHDM_P_Mhl_MscReq_WriteBurst"));

	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	if(ulNumBytes < 1 || ulNumBytes > 16)
	{
		ret = BHDM_P_MHL_CBUS_INVALID;
		goto error;
	}

	/* Construct the packets */
	pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eWriteBurst;

	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ucData = ucOffset;

	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[2].ucData = (uint8_t) (usAdopterId >> 8);

	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[3].ucData = (uint8_t) (usAdopterId & 0xFF);

	for(i = 0; i < ulNumBytes; i++)
	{
		pPackets[4+i].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
		pPackets[4+i].ulType = BHDM_P_Mhl_CbusPktType_eData;
		pPackets[4+i].ucData = pucData[i];
	}

	pPackets[ulNumBytes+4].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[ulNumBytes+4].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[ulNumBytes+4].ucData = BHDM_P_Mhl_Command_eEof;

	pPackets[ulNumBytes+5].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[ulNumBytes+5].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[ulNumBytes+5].ucData = BHDM_P_Mhl_Command_eAck;

	/* Prepare and add commnad to MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
	stMscReqCmd.ucDelay = ucDelay;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ulNumPacketsCfg = ulNumBytes + 5;
	stMscReqCmd.ulNumPacketsDone = 0;
	stMscReqCmd.bLastCmd = bLastCmd;
	stMscReqCmd.pucReplyBuf = NULL;
	stMscReqCmd.ucReplyBufSize = 1;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	ret = BHDM_P_Mhl_Req_AddCmd_isr(hMscReq->hReq, &stMscReqCmd);

error:

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#endif
	return ret;
}


BERR_Code BHDM_P_Mhl_MscReq_ReadDcap_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  BHDM_P_Mhl_CbusState     *pstCbusState,
	  uint8_t                   ucDelay1, /* ucDelay for first command */
	  uint8_t                   ucDelay2 ) /* subsequent ucDelay */
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;
	BHDM_P_Mhl_DevCapOffset eDcapOffset = BHDM_P_Mhl_DevCapOffset_eDevStateAddr;
	uint8_t *pucCap = hMscReq->pucSinkDcap;
	uint32_t ulSize, ulFreeSpace;

	BDBG_MSG(("BHDM_P_Mhl_MscReq_ReadDcap_isr"));

	BSTD_UNUSED(pstCbusState);
	BDBG_ASSERT(hMscReq);
	BDBG_OBJECT_ASSERT(hMscReq, BHDM_MHL_MSC_REQ);

	ulSize = BHDM_P_Mhl_DevCapOffset_eDevCapAddrMax - BHDM_P_Mhl_DevCapOffset_eDevStateAddr;

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = 	BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eReadDevCap;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ucData = BHDM_P_Mhl_DevCapOffset_eDevStateAddr;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[2].ucData = BHDM_P_Mhl_Command_eAck;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[3].ucData = 0;


	/* Prepare and add commnad to MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eDcap;
	stMscReqCmd.ucDelay = ucDelay1;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_READ_DEVCAP_CMD;
	stMscReqCmd.ulNumPacketsDone = 0;
	stMscReqCmd.bLastCmd = false;
	stMscReqCmd.pucReplyBuf = pucCap;
	stMscReqCmd.ucReplyBufSize = 1;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	ulFreeSpace = BHDM_P_Mhl_Fifo_GetFreeSpace_isr(hMscReq->hReq->pstCmdQ);

	if (ulFreeSpace < (ulSize * sizeof(stMscReqCmd)))
	{
		BDBG_WRN(("Not enough space in Q for READ_DEVCAP"));
		/* not enough space in Q to accomodate required number of commands, postpone the writing of READ_DEVCAP for later  */
		return BHDM_P_MHL_CMD_Q_FULL;
	}
	else
	{
		for (eDcapOffset = BHDM_P_Mhl_DevCapOffset_eDevStateAddr;
			 eDcapOffset < BHDM_P_Mhl_DevCapOffset_eDevCapAddrMax;
			 eDcapOffset++, pucCap++)
		{
			pPackets[1].ucData = eDcapOffset;
			stMscReqCmd.ucDelay = (eDcapOffset == BHDM_P_Mhl_DevCapOffset_eDevStateAddr) ? ucDelay1 : ucDelay2;
			stMscReqCmd.bLastCmd = (eDcapOffset == BHDM_P_Mhl_DevCapOffset_eIntStatSizeAddr) ? true : false;
			stMscReqCmd.pucReplyBuf = pucCap;

			ret = BHDM_P_Mhl_Req_AddCmd_isr(hMscReq->hReq, &stMscReqCmd);
			if (ret)
			{
				/* I don't expect any error here as we already checked for required free space in Q.
				   Still for sake of completeness let us break at first error.  */
				break;
			}
		}
	}
	return ret;
}

BERR_Code BHDM_P_Mhl_MscReq_CancelReadDcap_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq )
{
	if (hMscReq->eDcapReadState == BHDM_P_Mhl_DcapReadState_eMiddle &&
		hMscReq->hReq->stLastCmd.eCmdType == BHDM_P_Mhl_CbusCmdType_eDcap)
	{
		BHDM_P_Mhl_Req_CancelCmd_isr(hMscReq->hReq);

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Cancelled ");
		BHDM_P_Mhl_DumpCmdPkts_isr(&hMscReq->hReq->stLastCmd.cbusPackets.astShortCmd[0],
									hMscReq->hReq->stLastCmd.ulNumPacketsCfg,
									BHDM_P_Mhl_CbusDest_eMscReq);
#else
		BDBG_MSG(("Cancelled "));
#endif

		/* if the last sent command is not the last READ_DEVCAP command in sequence of 15 commands, remove the READ_DEVCAP commands from the Q*/
		if(hMscReq->hReq->stLastCmd.bLastCmd != true)
		{
			BHDM_P_Mhl_CbusCmd stMscReqCmd;

			while (!BHDM_P_Mhl_Fifo_ReadData_isr(hMscReq->hReq->pstCmdQ, (uint8_t *)&stMscReqCmd, sizeof(stMscReqCmd)) &&
					stMscReqCmd.bLastCmd == false)
			{
#if BHDM_MHL_ENABLE_DEBUG
				BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Removed ");
				BHDM_P_Mhl_DumpCmdPkts_isr(&stMscReqCmd.cbusPackets.astShortCmd[0],
											stMscReqCmd.ulNumPacketsCfg,
											BHDM_P_Mhl_CbusDest_eMscReq);
#else
		BDBG_MSG(("Removed "));
#endif
			}
		}

		/* Again re-queue the READ_DEVCAP commands from start, change the read state to start*/
		hMscReq->eDcapReadState = BHDM_P_Mhl_DcapReadState_eStart;

	}

	return BHDM_P_MHL_CBUS_SUCCESS;
}

void BHDM_P_Mhl_MscReq_HandleDone_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq,
	  bool                     *pbSinkPlimValid,
	  bool                     *pbHeartbeatQueued,
	  uint8_t                  *pucNumDeadHbPkts )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	BHDM_P_Mhl_CbusCmd *pCmd = &hMscReq->hReq->stLastCmd;

	/* First fetch the completion result */
	ret = BHDM_P_Mhl_Req_CompleteCmd_isr(hMscReq->hReq, pCmd);

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#endif

	if (pCmd->cbusPackets.astShortCmd[0].ucData == BHDM_P_Mhl_Command_eGetState)
	{
		BDBG_MSG((" CBUS Link State: %d.", *pCmd->pucReplyBuf));
#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		sprintf(acTemp, "%x", *pCmd->pucReplyBuf);
		strcat(acStr, "CBUS Link State:  ");
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
		BDBG_MSG((" CBUS Link State: %d.", *pCmd->pucReplyBuf));
#endif
	}


	/* If we read the PLIM of the sink, we have to decide
	   whether to turn on the PMSM or not */
	if ((ret == BHDM_P_MHL_CBUS_SUCCESS) &&
	   (pCmd->cbusPackets.astShortCmd[0].ucData == BHDM_P_Mhl_Command_eReadDevCap) &&
	   (pCmd->cbusPackets.astShortCmd[1].ucData == BHDM_P_Mhl_DevCapOffset_eDevCatAddr))
	{
		*pbSinkPlimValid = true;
	}

	/* update dcap read status if completed the last command in dacp read sequence */

	/* Only update the DCAP valid bit in mailbox if we read the whole DCAP.
	The last DCAP address is always that of INT_STAT_SIZE_ADDR */
	if((ret == BHDM_P_MHL_CBUS_SUCCESS) &&
		(pCmd->bLastCmd == true) &&
		(pCmd->eCmdType == BHDM_P_Mhl_CbusCmdType_eDcap) &&
		(pCmd->cbusPackets.astShortCmd[1].ucData == BHDM_P_Mhl_DevCapOffset_eIntStatSizeAddr)
		)
	{
#if BHDM_MHL_CTS
		BHDM_P_Mhl_Mailbox_DcapUpdateDone_isr(hMscReq->hReq->hRegister);
#else
		hMscReq->bDcapValid = true;
		hMscReq->bSinkDcapChg = false;
#endif
		hMscReq->eDcapReadState = BHDM_P_Mhl_DcapReadState_eEnd;
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_Mhl_DumpSinkDcap_isr(hMscReq->hReq->hRegister);
#endif
	}

	/* If the last command sent is a heartbeat, reset the heartbeat timer.  */
	if(pCmd->eCmdType == BHDM_P_Mhl_CbusCmdType_eHb)
	{
		*pbHeartbeatQueued = false;
		if(ret == BHDM_P_MHL_CBUS_SUCCESS)
		{
			BDBG_MSG(("Heartbeat ok"));

		}
		else
		{
			BDBG_WRN(("Heartbeat error"));
		}
	}

	*pucNumDeadHbPkts = 0; /* as we have completed some command means Sink is responding,  so reset dead packet count */
}


BERR_Code BHDM_P_Mhl_MscReq_HandleErrors_isr
	( BHDM_P_Mhl_MscReq_Handle   hMscReq,
	  int                        interrupt,
	  bool                       bStopPending,
	  bool                      *pbHeartbeatQueued,
	  BHDM_P_Mhl_CbusState      *pstCbusState )
{
	BERR_Code xmit_result = BHDM_P_MHL_CBUS_XMIT_SUCCESS;
	BHDM_P_Mhl_CbusCmd *pCmd = &hMscReq->hReq->stLastCmd;
	bool bAbortRequired = false;

	/* First fetch the completion result */
	BHDM_P_Mhl_Req_CompleteCmd_isr(hMscReq->hReq, pCmd);

	xmit_result = BHDM_P_Mhl_MscReq_CheckError_isr(hMscReq, interrupt, &bAbortRequired,
													&pCmd->cbusPackets.astShortCmd[pCmd->ulNumPacketsDone-1]);

	/* actions needed on certain errors */
	if(xmit_result == BHDM_P_MHL_CBUS_XMIT_BUS_STOLEN)
	{
		/* We do not expect bus stolen to happen, but if it does, we must cancel and then retry the packet */
		BHDM_P_Mhl_Req_CancelCmd_isr(hMscReq->hReq);
		hMscReq->hReq->bRetryLastCmd = true;
	}
	else if(xmit_result == BHDM_P_MHL_CBUS_XMIT_TIMEOUT)
	{
		/* Retry command if timeout, but NOT if an ABORT is due */
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Timeout! Retrying command: ");
		BHDM_P_Mhl_DumpCmdPkts_isr(&pCmd->cbusPackets.astShortCmd[0], pCmd->ulNumPacketsCfg, BHDM_P_Mhl_CbusDest_eMscReq);
#else
		BDBG_MSG(("Timeout! Retrying command: "));
#endif
		hMscReq->hReq->bRetryLastCmd = true;
	}
	else if(xmit_result == BHDM_P_MHL_CBUS_XMIT_NACK)
	{
		/*
		   For now we don't care about the exact reason for error,
		   For NACK'ed (packet) command, we will reread MSC command only if we are reading DCAP.  */

		/* MSC error */
		uint8_t ucErr = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hMscReq->hReq->hRegister, SINK_MSC_ERRORCODE);
		BHDM_P_Mhl_MscReq_GetMscErrorCode_isr(hMscReq, &ucErr, 0, true);

		if (!bStopPending && pCmd->eCmdType == BHDM_P_Mhl_CbusCmdType_eDcap)
		{   /* first cancel any ongoing DCAP commands and re-read the DCAP */
			BHDM_P_Mhl_MscReq_CancelReadDcap_isr(hMscReq);
			BHDM_P_Mhl_MscReq_ReadDcap_isr(hMscReq, pstCbusState, 1, 1);
		}
	}

	/* If the last command sent is a heartbeat, reset the heartbeat timer.  */
	if (pCmd->eCmdType == BHDM_P_Mhl_CbusCmdType_eHb)
	{
		*pbHeartbeatQueued = false;
		if(xmit_result == BHDM_P_MHL_CBUS_XMIT_SUCCESS)
		{
			BDBG_MSG(("Heartbeat ok"));

		}
		else
		{
			BDBG_WRN(("Heartbeat error"));
		}
	}

	if(bAbortRequired)
	{
		/* Send Abort here itself and clear the MSC Command Q */
		BHDM_P_Mhl_Req_SendAbort_isr(hMscReq->hReq);
	}

	return xmit_result;
}

void BHDM_P_Mhl_MscReq_HandleReceivedDcapRdy_isr
	( BHDM_P_Mhl_MscReq_Handle    hMscReq,
	  bool                       *pbSinkPlimValid,
	  BHDM_P_Mhl_CbusState       *pstCbusState )
{
	BERR_Code ret;

	/* Sink sent us DCAP_RDY.  If we haven't sent DCAP_RDY to sink, do it now.
		 If the sink has already sent us DCAP_CHG, then re-queue READ_DEVCAP command (remove existing ones) 	*/

	BDBG_MSG(("Sink DCAP_RDY is set"));

	if(hMscReq->eDcapSentState == BHDM_P_Mhl_DcapSentState_eRdyNotSent)
	{
		BHDM_P_Mhl_MscReq_WriteStat_isr(hMscReq, BHDM_P_Mhl_StatusAddr_eConnectedRdyAddr,
										BHDM_P_MHL_STAT_CONNECTED_DCAP_RDY, 0, true);

#if BHDM_MHL_CTS
		{
			BREG_Handle hRegister = hMscReq->hReq->hRegister;
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, MSC_REQ_PENDING, 1);
		}
#else
		hMscReq->hReq->bRequestPending = true;
#endif

		/* DCAP_RDY alone should be enough.  No need to send DCAP_CHG */
		/* cbus_msc_req_set_int(&dev->qstate, MHL_RCHANGE_INT_ADDR, MHL_INT_RCHANGE_DCAP_CHG, 0, true); */
		hMscReq->eDcapSentState = BHDM_P_Mhl_DcapSentState_eRdySent;
	}

	if(hMscReq->eDcapReadState == BHDM_P_Mhl_DcapReadState_eStart)
	{

		*pbSinkPlimValid = false;
		ret = BHDM_P_Mhl_MscReq_ReadDcap_isr(hMscReq, pstCbusState, 1, 1);
		if (!ret)
		{
			hMscReq->eDcapReadState = BHDM_P_Mhl_DcapReadState_eMiddle;
		}
		else
		{
			hMscReq->eUnhandledEvent |= BHDM_P_MHL_MSCREQ_EVENT_RECEIVED_DCAP_RDY;
		}
	}
}

void BHDM_P_Mhl_MscReq_HandleDcapChgInt_isr
	( BHDM_P_Mhl_MscReq_Handle   hMscReq,
	  bool                      *pbSinkPlimValid,
	  bool                      *pbMscRespObAbortActive,
	  BHDM_P_Mhl_CbusState      *pstCbusState )
{
	BERR_Code ret;
	BREG_Handle hRegister = hMscReq->hReq->hRegister;

	BDBG_MSG(("DCAP_CHG is set"));

	/* Sink's PLIM is now invalid */
	*pbSinkPlimValid = false;

	/* cancel the already going on READ_DEVCAP commands from the head of Q */
	BHDM_P_Mhl_MscReq_CancelReadDcap_isr(hMscReq);

	/* if the DCAP_READ was complete previously, due to DCAP_CHG start it again */
	if (hMscReq->eDcapReadState == BHDM_P_Mhl_DcapReadState_eEnd)
	{
		hMscReq->eDcapReadState = BHDM_P_Mhl_DcapReadState_eStart;
	}

	/* Just start reading DCAP if DCAP_RDY is set, any ongoing DCAP read commands are already cancelled above */
	if (BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_DCAP_RDY) &&
		hMscReq->eDcapReadState == BHDM_P_Mhl_DcapReadState_eStart)
	{
		if(!pbMscRespObAbortActive)
		{
			/* Only send if ABORT is not active */
			ret = BHDM_P_Mhl_MscReq_ReadDcap_isr(hMscReq, pstCbusState, 1, 1);
			if(ret == BHDM_P_MHL_CBUS_SUCCESS)
			{
				hMscReq->eDcapReadState = BHDM_P_Mhl_DcapReadState_eMiddle;
				BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SINK_DCAP_CHG);
			}
			else
			{
				hMscReq->eUnhandledEvent |= BHDM_P_MHL_MSCREQ_EVENT_DCAP_CHG_INT;
			}
		}
		else
		{

			BDBG_WRN(("MSC OB ABORT active, delaying READ DCAP"));
			hMscReq->eUnhandledEvent |= BHDM_P_MHL_MSCREQ_EVENT_DCAP_CHG_INT;
		}
	}
	else
	{
		/* If DCAP_RDY is not yet received, wait for DCAP_RDY before we send READ_DEVCAP commands */
		BDBG_WRN(("DCAP_RDY not yet received, delaying READ DCAP"));

		/* Clear the DCAP_CHG as the DCAP_RDY should trigger the DCAP read */
		BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SINK_DCAP_CHG);
	}
}

void BHDM_P_Mhl_MscReq_HandleReqWrtInt_isr
	( BHDM_P_Mhl_MscReq_Handle   hMscReq )
{
	/* We reply with SET_INT:GRT_WRT */
	BDBG_MSG(("REQ_WRT is set"));

	/* The WRITE_BURST command will cancel this flag */
	if (BHDM_P_Mhl_MscReq_SetInt_isr(hMscReq, BHDM_P_Mhl_IntAddr_eRchangeIntAddr,
									BHDM_P_MHL_INT_RCHANGE_GRT_WRT, 1, true) != BHDM_P_MHL_CBUS_SUCCESS)
	{
		hMscReq->eUnhandledEvent |= BHDM_P_MHL_MSCREQ_EVENT_REQ_WRT_INT;
	}
}

void BHDM_P_Mhl_MscReq_HandleMscTxAbort_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq )
{
	BDBG_MSG(("Resp sent ABORT"));
	BHDM_P_Mhl_MscReq_CancelAllCmds_isr(hMscReq);
}

void BHDM_P_Mhl_MscReq_HandleAbortMscReceived_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq )
{
	BDBG_MSG(("Received MSC IB ABORT"));

#if BHDM_P_MHL_ENABLE_HEARTBEAT_TIMER
	/* disable heartbeat if abort */
	DISABLE_HEARTBEAT;
#endif
	BHDM_P_Mhl_MscReq_CancelAllCmds_isr(hMscReq);
	/* TODO -  queue a GET_MSC_ERRORCODE command */
}

void BHDM_P_Mhl_MscReq_HandleReadDdcErrorCode_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq )
{
	uint8_t ucDdcErrCode = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hMscReq->hReq->hRegister, SINK_DDC_ERRORCODE);
	BHDM_P_Mhl_MscReq_GetDdcErrorCode_isr(hMscReq, &ucDdcErrCode, 0, true);
}

void BHDM_P_Mhl_MscReq_HandleSendRcpeErrBusy_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq )
{
	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;
	bool ret = false;

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eMscMsg;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ucData = BHDM_P_Mhl_MscMsgCommand_eRcpe;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[2].ucData = BHDM_P_Mhl_RcpError_eErrorBusy;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[3].ucData = BHDM_P_Mhl_Command_eAck;

	/* prepare the command and add to the MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ucDelay = 0;
	stMscReqCmd.bLastCmd = true;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_RCPE_CMD;
	stMscReqCmd.pucReplyBuf = NULL;
	stMscReqCmd.ucReplyBufSize = 0;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	ret = BHDM_P_Mhl_Req_AddCmd_isr(hMscReq->hReq, &stMscReqCmd);
	if(ret)
	{
		hMscReq->eUnhandledEvent |= BHDM_P_MHL_MSCREQ_EVENT_SEND_RCPE_ERR_BUSY;
	}

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#endif
}

void BHDM_P_Mhl_MscReq_HandleSendRapkErrNone_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq )
{
	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;
	bool ret = false;

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eMscMsg;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ucData = BHDM_P_Mhl_MscMsgCommand_eRapk;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[2].ucData = BHDM_P_Mhl_RapError_eNone;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[3].ucData = BHDM_P_Mhl_Command_eAck;

	/* prepare the command and add to the MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ucDelay = 0;
	stMscReqCmd.bLastCmd = true;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_RAPK_CMD;
	stMscReqCmd.pucReplyBuf = NULL;
	stMscReqCmd.ucReplyBufSize = 0;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	ret = BHDM_P_Mhl_Req_AddCmd_isr(hMscReq->hReq, &stMscReqCmd);
	if(ret)
	{
		hMscReq->eUnhandledEvent |= BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_NONE;
	}

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#endif
}

void BHDM_P_Mhl_MscReq_HandleSendRapkErrBusy_isr
	( BHDM_P_Mhl_MscReq_Handle  hMscReq )
{
	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;
	bool ret = false;

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eMscMsg;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ucData = BHDM_P_Mhl_MscMsgCommand_eRapk;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[2].ucData = BHDM_P_Mhl_RapError_eBusy;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[3].ucData = BHDM_P_Mhl_Command_eAck;

	/* prepare the command and add to the MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ucDelay = 0;
	stMscReqCmd.bLastCmd = true;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_RAPK_CMD;
	stMscReqCmd.pucReplyBuf = NULL;
	stMscReqCmd.ucReplyBufSize = 0;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	ret = BHDM_P_Mhl_Req_AddCmd_isr(hMscReq->hReq, &stMscReqCmd);
	if(ret)
	{
		hMscReq->eUnhandledEvent |= BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_BUSY;
	}
#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hMscReq->hReq);
#endif
}
