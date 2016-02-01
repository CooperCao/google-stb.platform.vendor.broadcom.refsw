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

#include "bhdm_mhl_req_priv.h"
#include "bhdm_mhl_mailbox_priv.h"
#include "bhdm_mhl_debug_priv.h"

BDBG_MODULE(BHDM_MHL_REQ);
BDBG_OBJECT_ID(BHDM_MHL_REQ);


#define BHDM_P_MHL_MSC_REQ_INTERRUPT_MASK \
	(BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_DONE_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_RX_MISMATCH_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_RX_CMD_ERROR_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_RX_TIMEOUT_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_ILLEGAL_SW_WR_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_MAX_RETRIES_EXCEEDED_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_UNEXPECTED_INBOUND_PKT_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_MSC_REQ_BUS_STOLEN_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_ABORT_MSC_RECEIVED_MASK)

#define BHDM_P_MHL_DDC_REQ_INTERRUPT_MASK \
	(BCHP_CBUS_INTR2_1_CPU_STATUS_ABORT_DDC_RECEIVED_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_UNEXPECTED_INBOUND_PKT_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_MAX_RETRIES_EXCEEDED_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_ILLEGAL_SW_WR_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_RX_TIMEOUT_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_RX_CMD_ERROR_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_RX_MISMATCH_MASK | \
	BCHP_CBUS_INTR2_1_CPU_STATUS_DDC_REQ_DONE_MASK)

#define BHDM_P_MHL_NUM_PKTS_MSC_REQ_1DATA_CMD (2)
#define BHDM_P_MHL_NUM_PKTS_MSC_REQ_2DATA_CMD (4)

BERR_Code BHDM_P_Mhl_Req_Create
	( BHDM_P_Mhl_Req_Handle          *phReq )
{
	BERR_Code rc = BERR_SUCCESS;
	BHDM_P_Mhl_Req_Object *pReq;

	*phReq = NULL;

	/* Alloc the context. */
	pReq = (BHDM_P_Mhl_Req_Object*)(BKNI_Malloc(sizeof(BHDM_P_Mhl_Req_Object)));
	if(!pReq)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* zero out all memory associated with the HDMI Device Handle before using */
	BKNI_Memset((void *) pReq, 0, sizeof(BHDM_P_Mhl_Req_Object));
	BDBG_OBJECT_SET(pReq, BHDM_MHL_REQ);

	/* keep created pointer */
	*phReq = (BHDM_P_Mhl_Req_Handle)pReq;

	return rc;
}

BERR_Code BHDM_P_Mhl_Req_Destroy
	( BHDM_P_Mhl_Req_Handle           hReq )
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ );

	BKNI_Free((void *)hReq->pstCmdQ);
	BKNI_Free((void *)hReq->pulRegs);

	/* free memory associated with the HDMI handle */
	BDBG_OBJECT_DESTROY(hReq, BHDM_MHL_REQ );
	BKNI_Free((void *)hReq);

	hReq = NULL;

	return rc;
}

BERR_Code BHDM_P_Mhl_Req_Init
	( BHDM_P_Mhl_Req_Handle           hReq,
	  BREG_Handle                     hRegister )
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);

	/* Initialize state. */
	hReq->bInitial = true;

	hReq->hRegister = hRegister;

	/* Initialize command Q. */
	hReq->pstCmdQ = BKNI_Malloc(sizeof(BHDM_P_Mhl_CmdQueue));
	if (hReq->pstCmdQ == NULL)
	{
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		return rc;
	}

	hReq->pulRegs = (uint32_t *)BKNI_Malloc(sizeof(uint32_t) * hReq->ulRegCount);
	if (hReq->pulRegs == NULL)
	{
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
	}
	else
	{
		/* Clear out shadow registers. */
		BKNI_Memset((void*)hReq->pulRegs, 0x0, (sizeof(uint32_t) * hReq->ulRegCount));
	}

	hReq->ucErrCode = 0;

	return rc;

}

/* Set its argument to true if the DDC channel or the abort is active respective */
void BHDM_P_Mhl_Req_Active_isr
	( BHDM_P_Mhl_Req_Handle           hReq,
	  bool                           *pbActive,
	  bool                           *pbAbortActive )
{
	BREG_Handle hRegister;
	uint32_t ulStat;

	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);

	hRegister = hReq->hRegister;

	if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
		ulStat = BREG_Read32(hRegister, BCHP_MT_CBUS_SCHEDULER_STATUS_0);
		*pbActive = BCHP_GET_FIELD_DATA(ulStat, MT_CBUS_SCHEDULER_STATUS_0, MSC_REQUESTER_ACTIVE) ? true : false;
		*pbAbortActive = BCHP_GET_FIELD_DATA(ulStat, MT_CBUS_SCHEDULER_STATUS_0, MSC_REQUESTER_ABORT_ACTIVE) ? true : false;
	}
	else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
	{
		ulStat = BREG_Read32(hRegister, BCHP_MT_CBUS_SCHEDULER_STATUS_0);
		*pbActive = BCHP_GET_FIELD_DATA(ulStat, MT_CBUS_SCHEDULER_STATUS_0, DDC_REQUESTER_ACTIVE) ? true : false;
		*pbAbortActive = BCHP_GET_FIELD_DATA(ulStat, MT_CBUS_SCHEDULER_STATUS_0, DDC_REQUESTER_ABORT_ACTIVE) ? true : false;
	}
}

/* [Start to] Send a command from a particular FIFO */
void BHDM_P_Mhl_Req_SendCmd_isr
	( BHDM_P_Mhl_Req_Handle           hReq,
	  BHDM_P_Mhl_CbusCmd             *pCmd )
{
	uint32_t ulOutboundFifoReg = 0x0;
	uint32_t ulData;
	BREG_Handle hRegister;
	BHDM_P_Mhl_CbusCmdQueueType  eQueueType;

	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);
	BDBG_ASSERT(pCmd);

	if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
		eQueueType = BHDM_P_Mhl_CbusCmdQueueType_eShort;
	}
	else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
	{
		eQueueType = BHDM_P_Mhl_CbusCmdQueueType_eLong;
	}
	else
	{
		eQueueType = BHDM_P_Mhl_CbusCmdQueueType_eUnknown;
	}

	BDBG_ASSERT(eQueueType < BHDM_P_Mhl_CbusCmdQueueType_eUnknown);

	hRegister = hReq->hRegister;

	/* First clear the FIFO by writing to the CTRL register */
	switch(pCmd->eDest)
	{
	case BHDM_P_Mhl_CbusDest_eMscReq:
		ulOutboundFifoReg = BCHP_MT_MSC_REQ_PACKET_00_01;

		ulData = BREG_Read32(hRegister, BCHP_MT_MSC_REQ_CTRL);
		ulData &= ~BCHP_MT_MSC_REQ_CTRL_CLEAR_BUFFER_MASK;
	    ulData |= BCHP_FIELD_DATA(MT_MSC_REQ_CTRL, CLEAR_BUFFER, 1);
		BREG_Write32(hRegister, BCHP_MT_MSC_REQ_CTRL, ulData);
		break;

	case BHDM_P_Mhl_CbusDest_eDdcReq:
		ulOutboundFifoReg = BCHP_MT_DDC_REQ_PACKET_00_01;

		ulData = BREG_Read32(hRegister, BCHP_MT_DDC_REQ_CTRL);
		ulData &= ~BCHP_MT_DDC_REQ_CTRL_CLEAR_BUFFER_MASK;
	    ulData |= BCHP_FIELD_DATA(MT_DDC_REQ_CTRL, CLEAR_BUFFER, 1);
		BREG_Write32(hRegister, BCHP_MT_DDC_REQ_CTRL, ulData);
		break;

	default:
		BDBG_ERR(("MSC Responder send command shouldn't be handled in REQ send command."));
		break;
	}

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		sprintf(acTemp, "%d", pCmd->ulNumPacketsCfg);
		strcat(acStr, "Sending ");
		strcat(acStr, acTemp);
		sprintf(acTemp, "%s", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC " : "DDC ");
		strcat(acStr, acTemp);
		strcat(acStr, " packets via register ");
		sprintf(acTemp, "0x%x", ulOutboundFifoReg);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
	BDBG_MSG(("Sending %d %s packets via register 0x%x",
		pCmd->ulNumPacketsCfg, (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC",  ulOutboundFifoReg));
#endif


	BHDM_P_Mhl_Cbus_SendPkts_isr(hRegister, ulOutboundFifoReg, pCmd, eQueueType);

	/* Write the packet count to the FIFO config register
	   and then start the transaction */
	switch(pCmd->eDest)
	{
	case BHDM_P_Mhl_CbusDest_eMscReq:
		ulData = BREG_Read32(hRegister, BCHP_MT_MSC_REQ_CFG_0);
		ulData &= ~BCHP_MT_MSC_REQ_CFG_0_NUM_PACKETS_MASK;
	    ulData |= BCHP_FIELD_DATA(MT_MSC_REQ_CFG_0, NUM_PACKETS, pCmd->ulNumPacketsCfg);
		BREG_Write32(hRegister, BCHP_MT_MSC_REQ_CFG_0, ulData);

		ulData = BREG_Read32(hRegister, BCHP_MT_MSC_REQ_CTRL);
		ulData &= ~BCHP_MT_MSC_REQ_CTRL_START_MASK;
	    ulData |= BCHP_FIELD_DATA(MT_MSC_REQ_CTRL, START, 1);
		BREG_Write32(hRegister, BCHP_MT_MSC_REQ_CTRL, ulData);

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Start MSC REQ\n");
#else
		BDBG_MSG(("Start MSC REQ"));
#endif

		break;

	case BHDM_P_Mhl_CbusDest_eDdcReq:
		ulData = BREG_Read32(hRegister, BCHP_MT_DDC_REQ_CFG_0);
		ulData &= ~BCHP_MT_DDC_REQ_CFG_0_NUM_PACKETS_MASK;
	    ulData |= BCHP_FIELD_DATA(MT_DDC_REQ_CFG_0, NUM_PACKETS, pCmd->ulNumPacketsCfg);
		BREG_Write32(hRegister, BCHP_MT_DDC_REQ_CFG_0, ulData);

		ulData = BREG_Read32(hRegister, BCHP_MT_DDC_REQ_CTRL);
		ulData &= ~BCHP_MT_DDC_REQ_CTRL_START_MASK;
	    ulData |= BCHP_FIELD_DATA(MT_DDC_REQ_CTRL, START, 1);
		BREG_Write32(hRegister, BCHP_MT_DDC_REQ_CTRL, ulData);

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Start DDC REQ\n");
#else
		BDBG_MSG(("Start DDC REQ"));
#endif
		break;

	default:
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("MSC Responder send command shouldn't be handled in REQ send command.\n");
#else
		BDBG_ERR(("MSC Responder send command shouldn't be handled in REQ send command."));
#endif

		break;
	}
}


void BHDM_P_Mhl_Req_CancelCmd_isr
	( BHDM_P_Mhl_Req_Handle           hReq )
{
	uint32_t ulData;
	BREG_Handle hRegister;

	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);

	BDBG_ASSERT(hReq->eType == BHDM_P_Mhl_ReqType_eMsc || hReq->eType == BHDM_P_Mhl_ReqType_eDdc);

	hRegister = hReq->hRegister;

	/* Trigger the cancel bit in the ctrl register,
	   as well as clearing the FIFO */
	if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
	{
		ulData = BREG_Read32(hRegister, BCHP_MT_DDC_REQ_CTRL);
		ulData &= ~(BCHP_MT_DDC_REQ_CTRL_CANCEL_MASK | BCHP_MT_DDC_REQ_CTRL_CLEAR_BUFFER_MASK);
	    ulData |= BCHP_FIELD_DATA(MT_DDC_REQ_CTRL, CANCEL, 1) | BCHP_FIELD_DATA(MT_DDC_REQ_CTRL, CLEAR_BUFFER, 1);
		BREG_Write32(hRegister, BCHP_MT_DDC_REQ_CTRL, ulData);
	}
	else if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
		ulData = BREG_Read32(hRegister, BCHP_MT_MSC_REQ_CTRL);
		ulData &= ~(BCHP_MT_MSC_REQ_CTRL_CANCEL_MASK | BCHP_MT_MSC_REQ_CTRL_CLEAR_BUFFER_MASK);
	    ulData |= BCHP_FIELD_DATA(MT_MSC_REQ_CTRL, CANCEL, 1) | BCHP_FIELD_DATA(MT_MSC_REQ_CTRL, CLEAR_BUFFER, 1);
		BREG_Write32(hRegister, BCHP_MT_MSC_REQ_CTRL, ulData);
	}
}

BERR_Code BHDM_P_Mhl_Req_ReceiveCmd_isr
	( BHDM_P_Mhl_Req_Handle           hReq,
	  BHDM_P_Mhl_CbusCmd             *pCmd )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;

	uint32_t i, ulPacketReg, ulData;
	BREG_Handle hRegister;
	BHDM_P_Mhl_CbusPkt *pPkts = NULL;

	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);
	BDBG_ASSERT(pCmd);
	BDBG_ASSERT(hReq->eType == BHDM_P_Mhl_ReqType_eMsc || hReq->eType == BHDM_P_Mhl_ReqType_eDdc);

	if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
		pPkts = pCmd->cbusPackets.astShortCmd;
	}
	else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
	{
		pPkts = pCmd->cbusPackets.astLongCmd;
	}

	hRegister = hReq->hRegister;

	switch(pCmd->eDest)
	{
	case BHDM_P_Mhl_CbusDest_eMscReq:
		ulPacketReg = BCHP_MT_MSC_REQ_PACKET_00_01;
		ulData = BREG_Read32(hRegister, BCHP_MT_MSC_REQ_STATUS);
		pCmd->ulNumPacketsDone = BCHP_GET_FIELD_DATA(ulData, MT_MSC_REQ_STATUS, PACKET_CNT);
#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcat(acStr, "MSC REQ processed ");
		sprintf(acTemp, "%d ", pCmd->ulNumPacketsDone);
		strcat(acStr, acTemp);
		strcat(acStr, "packets\n");
		strcat(acStr, acTemp);
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
		BDBG_MSG(("MSC REQ processed %d packets.", pCmd->ulNumPacketsDone));
#endif
		break;

	case BHDM_P_Mhl_CbusDest_eDdcReq:
		ulPacketReg = BCHP_MT_DDC_REQ_PACKET_00_01;
		ulData = BREG_Read32(hRegister, BCHP_MT_DDC_REQ_STATUS);
		pCmd->ulNumPacketsDone = BCHP_GET_FIELD_DATA(ulData, MT_DDC_REQ_STATUS, PACKET_CNT);
#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcat(acStr, "DDC REQ processed ");
		sprintf(acTemp, "%d ", pCmd->ulNumPacketsDone);
		strcat(acStr, acTemp);
		strcat(acStr, "packets\n");
		strcat(acStr, acTemp);
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
		BDBG_MSG(("DDC REQ processed %d packets.", pCmd->ulNumPacketsDone));
#endif
		break;

	default:
		BDBG_ERR(("Unsupported MHL Requester destination."));
		return BERR_INVALID_PARAMETER;
	}

	for (i = 0; i < pCmd->ulNumPacketsDone; i += 2)
	{
		uint32_t ulPktVal = BREG_Read32(hRegister, ulPacketReg);

		uint16_t usPktVal16 = ulPktVal & 0xFFFF; /* lower packet */

		BHDM_P_Mhl_CbusPktDirection ePktDir;
		BHDM_P_Mhl_CbusPktType ePktType;
		uint8_t ucPktData;

		BHDM_P_Mhl_Cbus_PktUnpack_isr(usPktVal16, &ePktType, &ePktDir, &ucPktData);

		if (!BHDM_P_Mhl_Cbus_PacketMatched_isr(&pPkts[i], ePktDir, ePktType, ucPktData))
		{
			/* Mismatched packet */
			ret = BHDM_P_MHL_CBUS_ERROR;
			pCmd->eState = BHDM_P_Mhl_CbusCmdState_eMismatched;

			BDBG_WRN(("Mismatched packet:<<0x%x != 0x%x", ucPktData, pPkts[i].ucData));

			/* Truncate the no. of packets done and quit now */
			pCmd->ulNumPacketsDone = i + 1;
			/* Copy the actual received data back for logging */
			pPkts[i].ucData = ucPktData;
			break;
		}

		/* Copy the result back (if any) */
		if (ePktDir == BHDM_P_Mhl_CbusPktDirection_eResp &&
			ePktType == BHDM_P_Mhl_CbusPktType_eData)
		{
			/* If the user supply a buffer which is too small
			   just copy as much as there is space, but write
			   the result back to the packet */
			if(pCmd->pucReplyBuf &&
			   pCmd->ucReplyBufSize > pCmd->ucReplyBufValidSize) {
				pCmd->pucReplyBuf[pCmd->ucReplyBufValidSize] = ucPktData;
				pCmd->ucReplyBufValidSize++;
			}

			/* The rest of the packets should be exactly
			   the same as what the user has supplied, so
			   no need to copy */
			pPkts[i].ucData = ucPktData;
		}

		/* Unpack the upper packet if we have at least one more */
		if (pCmd->ulNumPacketsDone - i > 1)
		{
			usPktVal16 = ulPktVal >> 16;
			BHDM_P_Mhl_Cbus_PktUnpack_isr(usPktVal16, &ePktType, &ePktDir, &ucPktData);

			if (!BHDM_P_Mhl_Cbus_PacketMatched_isr(&pPkts[i+1], ePktDir, ePktType, ucPktData))
			{
				/* Mismatched packet */
				ret = BHDM_P_MHL_CBUS_ERROR;
				pCmd->eState = BHDM_P_Mhl_CbusCmdState_eMismatched;

				BDBG_WRN(("Mismatched packet:<<0x%x != 0x%x", ucPktData, pPkts[i+1].ucData));

				/* Truncate the no. of packets done and quit now */
				pCmd->ulNumPacketsDone = i + 2;
				/* Copy the actual received data back for logging */
				pPkts[i+1].ucData = ucPktData;
				break;
			}

			if (ePktDir == BHDM_P_Mhl_CbusPktDirection_eResp &&
				ePktType == BHDM_P_Mhl_CbusPktType_eData)
			{

				/* If the user supply a buffer which is too small
				   just copy as much as there is space, but write
				   the result back to the packet */
				if(pCmd->pucReplyBuf && pCmd->ucReplyBufSize > pCmd->ucReplyBufValidSize)
				{
					pCmd->pucReplyBuf[pCmd->ucReplyBufValidSize] = ucPktData;
					pCmd->ucReplyBufValidSize++;
				}

				/* The rest of the packets should be exactly
				   the same as what the user has supplied, so
				   no need to copy */
				pPkts[i+1].ucData = ucPktData;
			}
		}

		ulPacketReg += 4; /* Move on to the next register */
	}

	if(pCmd->ulNumPacketsCfg == pCmd->ulNumPacketsDone)
		pCmd->eState = (pCmd->bLastCmd) ? BHDM_P_Mhl_CbusCmdState_eSuccess : BHDM_P_Mhl_CbusCmdState_ePartDone;

	return ret;
}

/* Clear ABORT functions. */
void BHDM_P_Mhl_Req_ClearAbort_isr
	( BHDM_P_Mhl_Req_Handle            hReq )
{
	BREG_Handle hRegister;
	uint32_t ulData;
	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);
	BDBG_ASSERT(hReq->eType == BHDM_P_Mhl_ReqType_eMsc || hReq->eType == BHDM_P_Mhl_ReqType_eDdc);

	hRegister = hReq->hRegister;

	if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
		BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, 0);

		ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ABORT_CTRL);
		ulData &= ~BCHP_MT_CBUS_ABORT_CTRL_CLEAR_MSC_MASK;
		ulData |= BCHP_FIELD_DATA(MT_CBUS_ABORT_CTRL, CLEAR_MSC, 1);
		BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, ulData);
	}
	else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
	{
		BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, 0);

		ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ABORT_CTRL);
		ulData &= ~BCHP_MT_CBUS_ABORT_CTRL_CLEAR_DDC_MASK;
		ulData |= BCHP_FIELD_DATA(MT_CBUS_ABORT_CTRL, CLEAR_DDC, 1);
		BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, ulData);
	}
}

/* Send ABORT on the MSC channel. Note that BOTH requester and responder
   use the same control. */
void BHDM_P_Mhl_Req_SendAbort_isr
	( BHDM_P_Mhl_Req_Handle      hReq )
{
	BREG_Handle hRegister;

	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);
	BDBG_ASSERT(hReq->eType == BHDM_P_Mhl_ReqType_eMsc || hReq->eType == BHDM_P_Mhl_ReqType_eDdc);

	hRegister = hReq->hRegister;

	if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
		uint32_t ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ABORT_CTRL);
		ulData &= ~BCHP_MT_CBUS_ABORT_CTRL_START_MSC_MASK;
		ulData |= BCHP_FIELD_DATA(MT_CBUS_ABORT_CTRL, START_MSC, 1);
		BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, ulData);
	}
	else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
	{
		uint32_t ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ABORT_CTRL);
		ulData &= ~BCHP_MT_CBUS_ABORT_CTRL_START_DDC_MASK;
		ulData |= BCHP_FIELD_DATA(MT_CBUS_ABORT_CTRL, START_DDC, 1);
		BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CTRL, ulData);
	}
}

uint32_t BHDM_P_Mhl_Req_GetIntStatus_isr
	( BHDM_P_Mhl_Req_Handle      hReq )
{
	BREG_Handle hRegister;
	uint32_t  ulStatus;

	BDBG_ASSERT(hReq);
	BDBG_ASSERT(hReq->eType == BHDM_P_Mhl_ReqType_eMsc || hReq->eType == BHDM_P_Mhl_ReqType_eDdc);

	hRegister = hReq->hRegister;

	if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
		ulStatus = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_STATUS);
		ulStatus &= BHDM_P_MHL_MSC_REQ_INTERRUPT_MASK;
	}
	else
	{
		ulStatus = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_STATUS);
		ulStatus &= BHDM_P_MHL_DDC_REQ_INTERRUPT_MASK;
	}

	return ulStatus;
}

/********************************************************
	Add to Queue
*********************************************************/
BERR_Code BHDM_P_Mhl_Req_AddCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_CbusCmd       *pCmd )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	bool bIsFull;


	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);
	BDBG_ASSERT(pCmd);

	bIsFull = BHDM_P_Mhl_Fifo_WriteData_isr(hReq->pstCmdQ, (uint8_t *)pCmd, sizeof(*pCmd));
	if (bIsFull)
	{
#if BHDM_MHL_ENABLE_DEBUG
		{
			char acStr[40] = "";
			char acTemp[10] = "";
			sprintf(acStr, "%s ", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC");
			strcat(acStr, " Command Q is full, ");
			sprintf(acTemp, "%d ", pCmd->ulNumPacketsCfg);
			strcat(acStr, acTemp);
			strcat(acStr, " packets dropped.\n");

			BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
		}
#else
		BDBG_ERR(("%s Command Q is full, %d pkts dropped",
			(hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC", pCmd->ulNumPacketsCfg));
#endif
		ret = BHDM_P_MHL_CMD_Q_FULL;
	}
	else
	{
		switch(pCmd->eDest)
		{
		case BHDM_P_Mhl_CbusDest_eMscReq:
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hReq->hRegister, MSC_REQ_PENDING, 1);
			break;
#endif
		case BHDM_P_Mhl_CbusDest_eDdcReq:
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hReq->hRegister, DDC_REQ_PENDING, 1);
#else
			hReq->bRequestPending = true;
#endif
			break;
		default:
			BDBG_ERR(("Unknown REQ send destination!!!"));
			return BERR_INVALID_PARAMETER;
		}

#if BHDM_MHL_ENABLE_DEBUG
		{
			BHDM_P_Mhl_CbusPkt *pPkts = NULL;

			if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
			{
				pPkts = pCmd->cbusPackets.astShortCmd;
			}
			else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
			{
				pPkts = pCmd->cbusPackets.astLongCmd;
			}

			BDBG_ASSERT(pPkts);

			BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Added ");
			BHDM_P_Mhl_DumpCmdPkts_isr(pPkts, pCmd->ulNumPacketsCfg, pCmd->eDest);
		}
#else
		BDBG_MSG(("Added %s cmds", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC"));
#endif
	}

	return ret;
}


/********************************************************
	Complete the leading command in the queue and recycle it
	Argument: request state, command which just completed [return]
	Note the the pointer cmd is not the queue element but command
	which gets completed is copied into cmd. [The packet array inside
	cmd must be allocated by the user (of size = 24 to cover
	both long and short commands), but the pucReplyBuf will point
	to the same memory buffer.
*********************************************************/
BERR_Code BHDM_P_Mhl_Req_CompleteCmd_isr
	( BHDM_P_Mhl_Req_Handle    hReq,
	  BHDM_P_Mhl_CbusCmd      *pCmd )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_UNEXPECTED_CMD;

	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);
	BDBG_ASSERT(pCmd);

	/* Note that MSC responder outbound commands
	   are not completed like this. (They are sent
	   directly). We will only ever have one
	   command in sent state. The queue is strictly
	   FIFO. The last argument is simply for checking. */

	if (pCmd->eState == BHDM_P_Mhl_CbusCmdState_eSent)
	{
		/* We found the command to be completed in the Q */
		/* Read the data back from hardware */
		ret = BHDM_P_Mhl_Req_ReceiveCmd_isr(hReq, pCmd);

#if BHDM_MHL_ENABLE_DEBUG
		{
			char acStr[60] = "";
			char acTemp[10] = "";
			sprintf(acStr, "%s ", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC");
			strcat(acStr, "Completed ");
			sprintf(acTemp, "%d ", pCmd->ulNumPacketsDone);
			strcat(acStr, acTemp);
			strcat(acStr, " packets.\n");
			BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
		}
#else
		BDBG_MSG(("Completed %s command", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC"));
#endif

		if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
		{
			if (pCmd->eCmdType == BHDM_P_Mhl_CbusCmdType_eHb)
			{
				BDBG_MSG(("Heartbeat "));
			}
		}
	}
	else
	{
#if BHDM_MHL_ENABLE_DEBUG
		{
			char acStr[60] = "";
			char acTemp[10] = "";
			sprintf(acStr, "%s ", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC");
			strcat(acStr, "Cmd state: ");
			sprintf(acTemp, "%d ", pCmd->eState);
			strcat(acStr, acTemp);
			strcat(acStr, " cannot be completed\n.");
			BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
		}
#else
		BDBG_ERR(("%s Cmd state: %d cannot be completed",
			(hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC", pCmd->eState));
#endif
	}

#if BHDM_MHL_ENABLE_DEBUG
	{
		BHDM_P_Mhl_CbusPkt *pPkts = NULL;

		if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
		{
			pPkts = pCmd->cbusPackets.astShortCmd;
		}
		else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
		{
			pPkts = pCmd->cbusPackets.astLongCmd;
		}

		BDBG_ASSERT(pPkts);

		BHDM_P_Mhl_DumpCmdPkts_isr(pPkts, pCmd->ulNumPacketsDone, pCmd->eDest);
	}
#endif
	return ret;
}

/********************************************************
	Send next command in Q
	Argument: CBUS requester state, command pointer

	Return cbus_success if the queue is not empty,
          command pointer will get a copy of the command just sent.
*********************************************************/

BERR_Code BHDM_P_Mhl_Req_SendNextCmd_isr
	( BHDM_P_Mhl_Req_Handle    hReq,
	  BHDM_P_Mhl_CbusCmd      *pCmd,
	  bool                    *pbRetryLastCmd )
{
	bool isFifoFull;

	BDBG_ASSERT(hReq);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);
	BDBG_ASSERT(pCmd);

	BDBG_MSG(("BHDM_P_Mhl_Req_SendNextCmd_isr"));

	if (*pbRetryLastCmd)
	{	/* Don't read new command from Q and send the same command stored last OB command */

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Re-Sending ");
		BHDM_P_Mhl_DumpCmdPkts_isr(pPkts, pCmd->ulNumPacketsCfg, pCmd->eDest);
#else
		BDBG_MSG(("Re-Sending %s cmd packets", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC"));
#endif

		*pbRetryLastCmd = false;
		pCmd->eState = BHDM_P_Mhl_CbusCmdState_eSent;
		BHDM_P_Mhl_Req_SendCmd_isr(hReq, pCmd);
	}
	else
	{
		/* read new command from the Q to last OB command and send it */
		isFifoFull = BHDM_P_Mhl_Fifo_ReadData_isr(hReq->pstCmdQ, (uint8_t *)pCmd, sizeof(*pCmd));
		if(!isFifoFull)
		{
#if BHDM_MHL_ENABLE_DEBUG
			BHDM_P_Mhl_CbusPkt *pPkts = NULL;

			if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
			{
				pPkts = pCmd->cbusPackets.astShortCmd;
			}
			else if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
			{
				pPkts = pCmd->cbusPackets.astLongCmd;
			}

			BDBG_ASSERT(pPkts);

			BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Sending ");
			BHDM_P_Mhl_DumpCmdPkts_isr(pPkts, pCmd->ulNumPacketsCfg, pCmd->eDest);
#else
			BDBG_MSG(("Sending %s cmd packets", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC"));
#endif

			pCmd->eState = BHDM_P_Mhl_CbusCmdState_eSent;
			BHDM_P_Mhl_Req_SendCmd_isr(hReq, pCmd);
		}
		else
		{
			BDBG_MSG(("%s Cmd Q empty, nothing to send!", (hReq->eType == BHDM_P_Mhl_ReqType_eMsc) ? "MSC" : "DDC"));
		}
	}

	return BHDM_P_MHL_CBUS_SUCCESS;
}

/* 1 reply data argument command
 *
 * (Note both of these are executed on the MSC channel)
 * >>GET_MSC_ERRORCODE or GET_DDC_ERRORCODE
 * <ERROR CODE
 */
BERR_Code BHDM_P_Mhl_Req_1DataCmd_isr
	( BHDM_P_Mhl_Req_Handle     hReq,
	  BHDM_P_Mhl_Command        eCmd,
	  uint8_t                  *pucData,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	BERR_Code ret = BERR_SUCCESS;

	/* This is destined for the  MSC REQ command queue. */
	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;

	BSTD_UNUSED(pucData);
	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);

	BDBG_MSG(("BHDM_P_Mhl_Req_1DataCmd_isr"));

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = 	BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = eCmd;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[1].ucData = 0;

	/* Prepare and add commnad to command Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
	stMscReqCmd.ucDelay = ucDelay;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_MSC_REQ_1DATA_CMD;
	stMscReqCmd.ulNumPacketsDone = 0;
	stMscReqCmd.bLastCmd = bLastCmd;
	stMscReqCmd.pucReplyBuf = NULL;
	stMscReqCmd.ucReplyBufSize = 0;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	ret = BHDM_P_Mhl_Req_AddCmd_isr(hReq, &stMscReqCmd);

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hReq);
#endif
	return ret;
}

BERR_Code BHDM_P_Mhl_Req_2DataCmd_isr
	( BHDM_P_Mhl_Req_Handle    hReq,
	  BHDM_P_Mhl_Command       cmd,
	  uint8_t                  ucData1,
	  uint8_t                  ucData2,
	  uint8_t                  ucDelay,
	  bool                     bLastCmd )
{
	BERR_Code ret = BERR_SUCCESS;

	/* This is destined for the  MSC REQ command queue. */
	BHDM_P_Mhl_CbusCmd stMscReqCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stMscReqCmd.cbusPackets.astShortCmd;


	BDBG_MSG(("BHDM_P_Mhl_Req_2DataCmd_isr"));

	BDBG_OBJECT_ASSERT(hReq, BHDM_MHL_REQ);

	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ulDir = 	BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ucData = cmd;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ucData = ucData1;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[2].ucData = ucData2;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[3].ucData = BHDM_P_Mhl_Command_eAck;

	/* Prepare and add commnad to MSC REQ Q */
	stMscReqCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eMsc;
	stMscReqCmd.ucDelay = ucDelay;
	stMscReqCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
	stMscReqCmd.ulNumPacketsCfg = BHDM_P_MHL_NUM_PKTS_MSC_REQ_2DATA_CMD;
	stMscReqCmd.ulNumPacketsDone = 0;
	stMscReqCmd.bLastCmd = bLastCmd;
	stMscReqCmd.pucReplyBuf = NULL;
	stMscReqCmd.ucReplyBufSize = 0;
	stMscReqCmd.ucReplyBufValidSize = 0;
	stMscReqCmd.eDest = BHDM_P_Mhl_CbusDest_eMscReq;
	stMscReqCmd.ePriority = BHDM_P_Mhl_CbusPriority_eReq;

	ret = BHDM_P_Mhl_Req_AddCmd_isr(hReq, &stMscReqCmd);

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_Mhl_DumpCmdQueue_isr(hReq);
#endif
	return ret;
}
