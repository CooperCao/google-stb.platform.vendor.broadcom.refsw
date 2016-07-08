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
#include "bhdm_mhl_ddc_req_priv.h"
#include "bhdm_mhl_debug_priv.h"
#include "bhdm_mhl_mailbox_priv.h"
#include "bchp_mt_ddc_req.h"

BDBG_MODULE(BHDM_MHL_DDC_REQ);
BDBG_OBJECT_ID(BHDM_MHL_DDC_REQ);

BERR_Code BHDM_P_Mhl_DdcReq_Create
	( BHDM_P_Mhl_DdcReq_Handle    *phDdcReq )
{
	BERR_Code rc = BERR_SUCCESS;
	BHDM_P_Mhl_DdcReq_Object *pDdcReq;

	*phDdcReq = NULL;

	/* Alloc the context. */
	pDdcReq = (BHDM_P_Mhl_DdcReq_Object *)(BKNI_Malloc(sizeof(BHDM_P_Mhl_DdcReq_Object)));
	if(!pDdcReq)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* zero out all memory associated with the HDMI Device Handle before using */
	BKNI_Memset((void *) pDdcReq, 0, sizeof(BHDM_P_Mhl_DdcReq_Object));
	BDBG_OBJECT_SET(pDdcReq, BHDM_MHL_DDC_REQ);

	rc = BHDM_P_Mhl_Req_Create(&pDdcReq->hReq);
	if (rc != BERR_SUCCESS) return rc;

	pDdcReq->hReq->eType = BHDM_P_Mhl_ReqType_eDdc;

	pDdcReq->hReq->ulBaseReg = BCHP_MT_DDC_REQ_REG_START;
	pDdcReq->hReq->ulRegCount = BHDM_P_MHL_REGS_ENTRIES(MT_DDC_REQ_REG_START, MT_DDC_REQ_REG_END);

	pDdcReq->pucEdidBlock = (uint8_t *)BKNI_Malloc(sizeof(uint8_t) * BHDM_P_MHL_MAILBOX_EDID_SIZE);
	if (pDdcReq->pucEdidBlock == NULL)
	{
		BDBG_ERR(("Failed to allocate memory for EDID buffer."));
		return BHDM_P_MHL_CBUS_NOMEMORY;
	}

	BKNI_Memset(pDdcReq->pucEdidBlock, 0, sizeof(uint8_t) * BHDM_P_MHL_MAILBOX_EDID_SIZE);

	rc = BKNI_CreateEvent(&pDdcReq->hHdcpVersionValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpRxBcapsValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpRxStatusValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpKsvValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpRepeaterKsvListValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpRxPjValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpRxRiValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpTxAksvValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpRxBksvValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpAnValueEvent);
	if (rc != BERR_SUCCESS) return rc;
	rc = BKNI_CreateEvent(&pDdcReq->hHdcpAinfoByteValueEvent);
	if (rc != BERR_SUCCESS) return rc;

	/* keep created pointer */
	*phDdcReq = (BHDM_P_Mhl_DdcReq_Handle)pDdcReq;

	return rc;
}

BERR_Code BHDM_P_Mhl_DdcReq_Destroy
	( BHDM_P_Mhl_DdcReq_Handle        hDdcReq )
{
	BDBG_OBJECT_ASSERT(hDdcReq, BHDM_MHL_DDC_REQ );

	BKNI_DestroyEvent(hDdcReq->hHdcpVersionValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpRxBcapsValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpRxStatusValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpKsvValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpRepeaterKsvListValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpRxPjValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpRxRiValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpTxAksvValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpRxBksvValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpAnValueEvent);
	BKNI_DestroyEvent(hDdcReq->hHdcpAinfoByteValueEvent);

	if (hDdcReq->stHdcpInfo.pucRepeaterKsvList)
	{
		BKNI_Free(hDdcReq->stHdcpInfo.pucRepeaterKsvList);
	}

	BHDM_P_Mhl_Fifo_Uninit(hDdcReq->hReq->pstCmdQ);

	BKNI_Free((void *)hDdcReq->pucEdidBlock);

	BHDM_P_Mhl_Req_Destroy(hDdcReq->hReq);

	/* free memory associated with the DDC_REQ handle */
	BDBG_OBJECT_DESTROY(hDdcReq, BHDM_MHL_DDC_REQ );
	BKNI_Free((void *)hDdcReq);

	hDdcReq = NULL;

	return BERR_SUCCESS;
}

BERR_Code BHDM_P_Mhl_DdcReq_Init
	( BHDM_P_Mhl_DdcReq_Handle        hDdcReq,
	  BREG_Handle                     hRegister,
	  bool                            bReadEdid )
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t i;
	uint8_t ucEdidStatus, ucHpdStatus;

	BDBG_ASSERT(hDdcReq);
	BDBG_OBJECT_ASSERT(hDdcReq, BHDM_MHL_DDC_REQ);

	/* Init Requester object */
	err = BHDM_P_Mhl_Req_Init(hDdcReq->hReq, hRegister);

	/* Initialize command Q. */
	BHDM_P_Mhl_Fifo_Init(hDdcReq->hReq->pstCmdQ, (unsigned long)&hDdcReq->astCmdQueue[0], sizeof(hDdcReq->astCmdQueue));
	BKNI_Memset((void *)&hDdcReq->astCmdQueue[0], 0, sizeof(hDdcReq->astCmdQueue));

	hDdcReq->eEvent          = BHDM_P_MHL_DDCREQ_EVENT_NONE;
	hDdcReq->eUnhandledEvent = BHDM_P_MHL_DDCREQ_EVENT_NONE;
	hDdcReq->eHdcpEvent      = BHDM_P_MHL_HDCP_EVENT_NONE;

	do
	{
		ucHpdStatus = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, HPD);
	} while (!ucHpdStatus);

	if (bReadEdid)
	{
		do
		{
			ucEdidStatus = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, EDID_VALID);
		} while (!ucEdidStatus);

		/* fill with mailbox EDID info */
		for (i=0; i<BHDM_P_MHL_MAILBOX_EDID_SIZE; i++)
		{
			hDdcReq->pucEdidBlock[i] = BHDM_P_Mhl_Mailbox_Read(hRegister, BHDM_P_Mhl_MailboxField_eEdid + i);
		}
	}

	return err;
}

BERR_Code BHDM_P_Mhl_DdcReq_Read_isr
	( BHDM_P_Mhl_DdcReq_Handle     hDdcReq,
	  BHDM_P_Mhl_CbusCmdType       eCmdType,
	  uint8_t                      ucDevAddr,
	  uint8_t                      ucOffset,
	  uint32_t                     ulNumBytes,
	  uint8_t                     *pucData,
	  bool                         bShortRead,
	  uint8_t                      ucDelay )
{
	uint32_t ulBytesRemain = ulNumBytes, ulBytesToRead, ulNumPackets = 0;
	uint32_t i;
	bool bEofSent = false;
	bool bFitAll;

	BHDM_P_Mhl_CbusCmd stDdcCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stDdcCmd.cbusPackets.astLongCmd;

	uint8_t ucDevAddrR = ucDevAddr | 0x1; /* Read address */
	uint8_t *pucReplyBuf = pucData;
	uint32_t ulReplyBufSize = 0;
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;

	BDBG_MSG(("BHDM_P_Mhl_DdcReq_Read_isr"));

	/* Part 1a: Launch request - writing ucOffset if this is not a short read
	   [>>SOF
	   >ucDevAddr_w (same as ucDevAddr)
	   <<ACK
	   >ucOffset
	   <<ACK]
	*/
	if(!bShortRead)
	{
		pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
		pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
		pPackets[0].ucData = BHDM_P_Mhl_Command_eSof;

		pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
		pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
		pPackets[1].ucData = ucDevAddr;

		pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
		pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
		pPackets[2].ucData = BHDM_P_Mhl_Command_eAck;

		pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
		pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eData;
		pPackets[3].ucData = ucOffset;

		pPackets[4].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
		pPackets[4].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
		pPackets[4].ucData = BHDM_P_Mhl_Command_eAck;
		ulNumPackets += 5;
	}

	/* Part 1b: start of reading request
	   >>SOF
	   >ucDevAddrR
	   <<ACK
	*/
	pPackets[ulNumPackets].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[ulNumPackets].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[ulNumPackets].ucData = BHDM_P_Mhl_Command_eSof;

	pPackets[ulNumPackets+1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[ulNumPackets+1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[ulNumPackets+1].ucData = ucDevAddrR;

	pPackets[ulNumPackets+2].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[ulNumPackets+2].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[ulNumPackets+2].ucData = BHDM_P_Mhl_Command_eAck;

	ulNumPackets += 3;

	/* Can we fit the rest of the transaction in the FIFO?
	   +2 for >>STOP and >>EOF
	*/
	if (ulBytesRemain * 2 + 2 + ulNumPackets <= BHDM_P_MHL_CBUS_PKT_FIFO_SIZE)
	{
		/* Yes */
		ulBytesToRead = ulBytesRemain;
		bFitAll = true;
	}
	else
	{
		/* No, so just fill the rest of the FIFO
		   with >>CONT <DATA sequence */
		ulBytesToRead = (BHDM_P_MHL_CBUS_PKT_FIFO_SIZE - ulNumPackets) / 2;
		bFitAll = false;
	}

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcat(acStr, "DDC can read ");
		sprintf(acTemp, "%d ", ulBytesToRead);
		strcat(acStr, acTemp);
		strcat(acStr, "out of ");
		sprintf(acTemp, "%d ", ulBytesRemain);
		strcat(acStr, acTemp);
		strcat(acStr, "bytes.\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
	BDBG_MSG(("DDC can read %d out of %d bytes", ulBytesToRead, ulBytesRemain));
#endif

	/* Loop to get data, if we can fit the last
	   >>STOP >>EOF, we will break out of the loop.
	   Error also causes the execution out of the loop. */
	do
	{
		/* Part 2: >>CONT <DATA cycle, ulBytesToRead can be
		   zero if we have only got STOP and EOF left to send */
		for(i = 0; i < ulBytesToRead; i++)
		{
			pPackets[i*2 + ulNumPackets].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
			pPackets[i*2 + ulNumPackets].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
			pPackets[i*2 + ulNumPackets].ucData = BHDM_P_Mhl_Command_eCont;

			pPackets[i*2+1 + ulNumPackets].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
			pPackets[i*2+1 + ulNumPackets].ulType = BHDM_P_Mhl_CbusPktType_eData;
			/* Don't care. incoming data */
			pPackets[i*2+1 + ulNumPackets].ucData = 0;
		}
		/* We have this many packets and reply bytes,
		   can be zero if we are only sending STOP and EOF */
		ulNumPackets += ulBytesToRead*2;
		ulReplyBufSize = ulBytesToRead;

		/* Part 3: >>STOP >>EOF
		   Insert the last commands if they fit */
		if (bFitAll)
		{
			pPackets[ulNumPackets].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
			pPackets[ulNumPackets].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
			pPackets[ulNumPackets].ucData = BHDM_P_Mhl_Command_eStop;

			pPackets[ulNumPackets+1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
			pPackets[ulNumPackets+1].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
			pPackets[ulNumPackets+1].ucData = BHDM_P_Mhl_Command_eEof;
			ulNumPackets += 2;
			bEofSent = true;
		}

		/* Prepare and add commnad to DDC Q */
		stDdcCmd.eCmdType = eCmdType;
		stDdcCmd.ucDelay = ucDelay;
		stDdcCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
		stDdcCmd.ulNumPacketsCfg = ulNumPackets;
		stDdcCmd.ulNumPacketsDone = 0;
		stDdcCmd.bLastCmd = bEofSent ? true : false;
		stDdcCmd.pucReplyBuf = ulReplyBufSize ? pucReplyBuf : NULL;
		stDdcCmd.ucReplyBufSize = ulReplyBufSize;
		stDdcCmd.ucReplyBufValidSize = 0;
		stDdcCmd.eDest = BHDM_P_Mhl_CbusDest_eDdcReq;
		stDdcCmd.ePriority = BHDM_P_Mhl_CbusPriority_eDdc;

		ret = BHDM_P_Mhl_Req_AddCmd_isr(hDdcReq->hReq, &stDdcCmd);

		/* Quit if we failed to add the request to the queue */
		if (ret != BHDM_P_MHL_CBUS_SUCCESS)
		{
			BDBG_ERR(("Failed to add DDC read commands."));
			break;
		}

		/* Reset packet count for next round */
		pucReplyBuf += ulReplyBufSize;
		ulReplyBufSize = 0;
		ulNumPackets = 0;

		/* Update how many bytes there are to read.
		   Note that we might have no data to read in the last round,
		   but still have to send the ending commands STOP and EOF */
		ulBytesRemain -= ulBytesToRead;

		if(ulBytesRemain * 2 + 2 <= BHDM_P_MHL_CBUS_PKT_FIFO_SIZE)
		{
			ulBytesToRead = ulBytesRemain;
			bFitAll = true;
		}
		else
		{
			ulBytesToRead = (BHDM_P_MHL_CBUS_PKT_FIFO_SIZE - ulNumPackets) / 2;
			bFitAll = false;
		}

		if(ulBytesRemain || !bEofSent)
		{
#if BHDM_MHL_ENABLE_DEBUG
		{
			char acStr[40] = "";
			char acTemp[10] = "";
			strcat(acStr, "Next round DDC will read ");
			sprintf(acTemp, "%d ", ulBytesToRead);
			strcat(acStr, acTemp);
			strcat(acStr, "out of ");
			sprintf(acTemp, "%d ", ulBytesRemain);
			strcat(acStr, acTemp);
			strcat(acStr, "bytes.\n");
			BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
		}
#else
			BDBG_MSG(("Next round DDC will read %d out of %d bytes", ulBytesToRead, ulBytesRemain));
#endif
		}
		else
		{
#if BHDM_MHL_ENABLE_DEBUG
			BHDM_P_MHL_DEBUG_DUMP_TO_FILE("DDC read done!\n");
#else
			BDBG_MSG(("DDC read done!"));
#endif
		}
	} while(ulBytesRemain || !bEofSent);

	if (ret != BHDM_P_MHL_CBUS_SUCCESS)
	{

#if BHDM_MHL_ENABLE_DEBUG
		{
			char acStr[40] = "";
			char acTemp[10] = "";
			strcat(acStr, "CBUS error 0x");
			sprintf(acTemp, "%X ", ret);
			strcat(acStr, acTemp);
			strcat(acStr, ". Removing all DDC commands.\n");
			BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
		}
#else
		BDBG_ERR(("CBUS error [0x%x]. Removing all DDC commands.", ret));
#endif
		BHDM_P_Mhl_DdcReq_RemoveAllCmds_isr(hDdcReq);
	}

	return ret;
}

/* A short read is just a DDC read without offset */
BERR_Code BHDM_P_Mhl_DdcReq_ShortRead_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  BHDM_P_Mhl_CbusCmdType     eCmdType,
	  uint8_t                    ucDevAddr,
	  uint32_t                   ulNumBytes,
	  uint8_t                   *pucData,
	  uint8_t                    ucDelay )
{
	return BHDM_P_Mhl_DdcReq_Read_isr(hDdcReq,
									eCmdType,
									ucDevAddr,
									0,
									ulNumBytes,
									pucData,
									true,
									ucDelay);
}

BERR_Code BHDM_P_Mhl_DdcReq_OffsetRead_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  BHDM_P_Mhl_CbusCmdType    eCmdType,
	  uint8_t                   ucDevAddr,
	  uint8_t                   ucOffset,
	  uint32_t                  ulNumBytes,
	  uint8_t                  *pucData,
	  uint8_t                   ucDelay )
{
	return BHDM_P_Mhl_DdcReq_Read_isr(hDdcReq,
									eCmdType,
									ucDevAddr,
									ucOffset,
									ulNumBytes,
									pucData,
									false,
									ucDelay);
}

/*
   A read with a segment pointer.
   Pass short_read as false if offset
   is required.

   Command type is fixed as EDID

   Return BHDM_P_MHL_CBUS_SUCCESS if all requests are successfully added
 */
static BERR_Code BHDM_P_Mhl_DdcReq_SegmentRead_isr
	( BHDM_P_Mhl_DdcReq_Handle    hDdcReq,
	  uint8_t                     ucSegmentVal,
	  uint8_t                     ucDevAddr,
	  uint8_t                     ucOffset,
	  uint32_t                    ulNumBytes,
	  uint8_t                    *pucData,
	  bool                        bShortRead,
	  uint8_t                     ucDelay )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	uint32_t ulNumPackets = 0;

	BHDM_P_Mhl_CbusCmd stDdcCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stDdcCmd.cbusPackets.astLongCmd;


	/* Part 1
	   >>SOF
	   >segment_ptr
	   <<ACK
	   >ucSegmentVal
	   <<ACK
	*/
#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_MHL_DEBUG_DUMP_TO_FILE("BHDM_P_Mhl_DdcReq_SegmentRead_isr\n");
#else
	BDBG_MSG(("BHDM_P_Mhl_DdcReq_SegmentRead_isr"));
#endif

	if(ucSegmentVal)
	{
		pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
		pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
		pPackets[0].ucData = BHDM_P_Mhl_Command_eSof;

		pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
		pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
		pPackets[1].ucData = BHDM_P_MHL_DDC_SEGPTR_ADDR;

		pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
		pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
		pPackets[2].ucData = BHDM_P_Mhl_Command_eAck;

		pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
		pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eData;
		pPackets[3].ucData = ucSegmentVal;

		pPackets[4].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
		pPackets[4].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
		pPackets[4].ucData = BHDM_P_Mhl_Command_eAck;
		ulNumPackets += 5;


		/* Prepare and add command to DDC Q. This request has no reply. */
		stDdcCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eEdid;
		stDdcCmd.ucDelay = ucDelay;
		stDdcCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
		stDdcCmd.ulNumPacketsCfg = ulNumPackets;
		stDdcCmd.bLastCmd = false;
		stDdcCmd.pucReplyBuf = NULL;
		stDdcCmd.ucReplyBufSize = 0;
		stDdcCmd.ucReplyBufValidSize = 0;
		stDdcCmd.eDest = BHDM_P_Mhl_CbusDest_eDdcReq;
		stDdcCmd.ePriority = BHDM_P_Mhl_CbusPriority_eDdc;

		ret = BHDM_P_Mhl_Req_AddCmd_isr(hDdcReq->hReq, &stDdcCmd);

		if(ret != BHDM_P_MHL_CBUS_SUCCESS)
		{
			BDBG_ERR(("Failed to add CBUS EDID command [0x%x].", ret));
			goto error;
		}
	}
	/* TODO we have to handle NACK from sink */

	/* If we get here, we can proceed as per non segment pointer read
	   so we can just call the ddc_read function. We also get here if
	   segment pointer value is zero. */

	ret = BHDM_P_Mhl_DdcReq_Read_isr(hDdcReq,
								BHDM_P_Mhl_CbusCmdType_eEdid,
								ucDevAddr,
								ucOffset,
								ulNumBytes,
								pucData,
								bShortRead, 0);
	if (ret != BHDM_P_MHL_CBUS_SUCCESS)
	{
		BDBG_ERR(("Failed to do DDC Request read [0x%x].", ret));
	}

error:
	if (ret != BHDM_P_MHL_CBUS_SUCCESS && ucSegmentVal)
	{
		/* If we have a segment and we failed to queue that request,
		   we need to remove that from the queue. Any failure
		   in queueing futher commands will be cleaned up by BHDM_P_Mhl_DdcReq_Read_isr */
		BHDM_P_Mhl_DdcReq_RemoveAllCmds_isr(hDdcReq);
	}

	return ret;
}

/* General WRITE function with offset.
   Argument: dev address
             offset
	     no. of bytes to read
	     data array to write (const)
   Return: BHDM_P_MHL_CBUS_SUCCESS if all commands are queued

   Handling of NACK is done at a higher layer.
*/

BERR_Code BHDM_P_Mhl_DdcReq_Write_isr
	( BHDM_P_Mhl_DdcReq_Handle    hDdcReq,
	  BHDM_P_Mhl_CbusCmdType      eCmdType,
	  uint8_t                     ucDevAddr,
	  uint8_t                     ucOffset,
	  uint32_t                    ulNumBytes,
	  const uint8_t              *pucData,
	  uint8_t                     ucDelay )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	uint32_t ulBytesRemain = ulNumBytes, bytes_to_write, ulNumPackets = 0;
	uint32_t i;
	bool bEofSent = false;
	uint32_t ulPacketCnt;
	bool bFitAll;
	BHDM_P_Mhl_CbusCmd stDdcCmd;
	BHDM_P_Mhl_CbusPkt *pPackets = stDdcCmd.cbusPackets.astLongCmd;

	/* Part 1: Launch request
	   >>SOF
	   >ucDevAddr_w
	   <<ACK
	   >offset
	   <<ACK
	*/

	BDBG_MSG(("BHDM_P_Mhl_DdcReq_Write_isr"));

	pPackets[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[0].ucData = BHDM_P_Mhl_Command_eSof;

	pPackets[1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[1].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[1].ucData = ucDevAddr;

	pPackets[2].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[2].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[2].ucData = BHDM_P_Mhl_Command_eAck;

	pPackets[3].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
	pPackets[3].ulType = BHDM_P_Mhl_CbusPktType_eData;
	pPackets[3].ucData = ucOffset;

	pPackets[4].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
	pPackets[4].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
	pPackets[4].ucData = BHDM_P_Mhl_Command_eAck;
	ulNumPackets += 5;

	/* Can we fit the rest of the transaction in the FIFO?
	   +2 for >>STOP and >>EOF
	*/
	if(ulBytesRemain * 2 + 2 + ulNumPackets <= BHDM_P_MHL_CBUS_PKT_FIFO_SIZE)
	{
		/* Yes */
		bytes_to_write = ulBytesRemain;
		bFitAll = true;
	}
	else
	{
		/* No, so just fill the rest of the FIFO
		   with >DATA <<ACK sequence */
		bytes_to_write = (BHDM_P_MHL_CBUS_PKT_FIFO_SIZE - ulNumPackets) / 2;
		bFitAll = false;
	}

	/* This is no. of data bytes written (interleaved with <<ACK */
	ulPacketCnt = 0;
	/* Loop to write data, if we can fit the last
	   >>STOP >>EOF, we will break out of the loop.
	   Error also causes the execution out of the loop. */
	do {
		/* Part 2: >DATA <<ACK cycle, bytes_to_write can be
		   zero if we have only got STOP and EOF left to send */
		for(i = 0; i < bytes_to_write; i++) {
			pPackets[i*2 + ulNumPackets].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
			pPackets[i*2 + ulNumPackets].ulType = BHDM_P_Mhl_CbusPktType_eData;
			pPackets[i*2 + ulNumPackets].ucData = pucData[ulPacketCnt + i];

			pPackets[i*2+1 + ulNumPackets].ulDir = BHDM_P_Mhl_CbusPktDirection_eResp;
			pPackets[i*2+1 + ulNumPackets].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
			pPackets[i*2+1 + ulNumPackets].ucData = BHDM_P_Mhl_Command_eAck;
		}
		ulNumPackets += bytes_to_write*2;

		/* Part 3: >>STOP >>EOF
		   Insert the last commands it they fit */
		if(bFitAll)
		{
			pPackets[ulNumPackets].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
			pPackets[ulNumPackets].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
			pPackets[ulNumPackets].ucData = BHDM_P_Mhl_Command_eStop;

			pPackets[ulNumPackets+1].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
			pPackets[ulNumPackets+1].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
			pPackets[ulNumPackets+1].ucData = BHDM_P_Mhl_Command_eEof;
			ulNumPackets += 2;
			bEofSent = true;
		}

		/* Prepare and add commnad to DDC Q */
		stDdcCmd.eCmdType = eCmdType;
		stDdcCmd.ucDelay = ucDelay;
		stDdcCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
		stDdcCmd.ulNumPacketsCfg = ulNumPackets;
		stDdcCmd.ulNumPacketsDone = 0;
		stDdcCmd.bLastCmd = bEofSent ? true : false;
		stDdcCmd.pucReplyBuf = NULL;
		stDdcCmd.ucReplyBufSize = 0;
		stDdcCmd.ucReplyBufValidSize = 0;
		stDdcCmd.eDest = BHDM_P_Mhl_CbusDest_eDdcReq;
		stDdcCmd.ePriority = BHDM_P_Mhl_CbusPriority_eDdc;

		ret = BHDM_P_Mhl_Req_AddCmd_isr(hDdcReq->hReq, &stDdcCmd);
		if (ret != BHDM_P_MHL_CBUS_SUCCESS)
		{
			BDBG_ERR(("Failed to add DDC write command [0x%x].", ret));
			break;
		}

		ulPacketCnt += bytes_to_write;

		/* Update how many bytes there are to read.
		   Note that we might have no data to read in a round,
		   but still have to send the ending commands STOP and EOF */
		ulBytesRemain -= bytes_to_write;

		if( ulBytesRemain * 2 + 2 <= BHDM_P_MHL_CBUS_PKT_FIFO_SIZE)
		{
			bytes_to_write = ulBytesRemain;
			bFitAll = true;
		}
		else
		{
			bytes_to_write = (BHDM_P_MHL_CBUS_PKT_FIFO_SIZE - ulNumPackets) / 2;
			bFitAll = false;
		}

		ulNumPackets = 0; /* Reset packet count for next round */

	} while (ulBytesRemain || !bEofSent);

	if (ret != BHDM_P_MHL_CBUS_SUCCESS)
	{
		/* Remove all the commands we queue so far if failed */
		BHDM_P_Mhl_DdcReq_RemoveAllCmds_isr(hDdcReq);
	}

	return ret;
}

void BHDM_P_Mhl_DdcReq_RemoveAllCmds_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq )
{
#if BHDM_MHL_ENABLE_DEBUG
	struct BHDM_P_Mhl_CbusCmd stDdcCmd;

	BDBG_ASSERT(hDdcReq);
	BDBG_OBJECT_ASSERT(hDdcReq, BHDM_MHL_DDC_REQ);

	while (!BHDM_P_Mhl_Fifo_ReadData_isr(hDdcReq->hReq->pstCmdQ, (uint8_t *)&stDdcCmd, sizeof(stDdcCmd)))
	{
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Removed ");
		BHDM_P_Mhl_DumpCmdPkts_isr(&stDdcCmd.cbusPackets.astLongCmd[0], stDdcCmd.ulNumPacketsCfg, BHDM_P_Mhl_CbusDest_eDdcReq);
	}

#else
	BDBG_ASSERT(hDdcReq);
	BDBG_OBJECT_ASSERT(hDdcReq, BHDM_MHL_DDC_REQ);
	BDBG_MSG(("Removed all DDC commands"));
	hDdcReq->hReq->pstCmdQ->ulReadAddr = hDdcReq->hReq->pstCmdQ->ulWriteAddr;
#endif
}

void BHDM_P_Mhl_DdcReq_CancelAllCmds_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq )
{
	BDBG_ASSERT(hDdcReq);
	BDBG_OBJECT_ASSERT(hDdcReq, BHDM_MHL_DDC_REQ);

	BDBG_WRN(("Flushing all DDC cmds"));

	/* cancel last ongoing command in HW */
	if(hDdcReq->hReq->stLastCmd.eState == BHDM_P_Mhl_CbusCmdState_eSent)
	{
		BHDM_P_Mhl_Req_CancelCmd_isr(hDdcReq->hReq);

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Cancelled ");
		BHDM_P_Mhl_DumpCmdPkts_isr(&hDdcReq->hReq->stLastCmd.cbusPackets.astLongCmd[0], hDdcReq->hReq->stLastCmd.ulNumPacketsCfg, BHDM_P_Mhl_CbusDest_eDdcReq);
#else
		BDBG_MSG(("Cancelled all DDC commands "));
#endif
	}

	/* Remove all the commands from the DDC command Q */
	BHDM_P_Mhl_DdcReq_RemoveAllCmds_isr(hDdcReq);
}

BERR_Code  BHDM_P_Mhl_DdcReq_EdidRead_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  uint8_t                    ucSegment,
	  uint8_t                    ucOffset,
	  uint32_t                   ulNumBytes,
	  uint8_t                   *pucBlock,
	  uint8_t                    ucDelay )
{
	BERR_Code err = BHDM_P_MHL_CBUS_SUCCESS;

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcpy(acStr, "Reading EDID segment ");
		sprintf(acTemp, "%d ", ucSegment);
		strcat(acStr, acTemp);
		strcat(acStr, " ucOffset ");
		sprintf(acTemp, "%d ", ucOffset);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
	BDBG_MSG(("Reading EDID segment %d ucOffset 0x%x", ucSegment, ucOffset));
#endif

	err =  BHDM_P_Mhl_DdcReq_SegmentRead_isr(hDdcReq,
											ucSegment,
											BHDM_P_MHL_DDC_EDID_ADDR,
											ucOffset,
											ulNumBytes,
											pucBlock,
											false,
											ucDelay);
	if (err != BHDM_P_MHL_CBUS_SUCCESS)
	{
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Failed to send read EDID command \n");
#else
		BDBG_ERR(("Failed to send read EDID command [0x%x]", err));
#endif
	}

	return err;
}

void BHDM_P_Mhl_DdcReq_ClrHpdState_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq )
{
#if BHDM_MHL_CTS
	BREG_Handle hReg = hDdcReq->hReq->hRegister;
	BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hReg, HPD);
#else
	hDdcReq->bHpd = false;
#endif
	hDdcReq->eEdidReadState = BHDM_P_Mhl_EdidReadState_eStart;
	hDdcReq->ulEdidNumBlocks= 0;
	hDdcReq->ucEdidCurrentBlock = 0;
	hDdcReq->ucEdidReadAttempt = 0;

#if BHDM_MHL_CTS
	BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hReg, EDID_VALID);
	BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hReg, EDID_SIZE, 0);
#else
	hDdcReq->bEdidValid = false;
	hDdcReq->ulEdidSize = 0;
#endif
}

void BHDM_P_Mhl_DdcReq_AbortTx_isr
	( BHDM_P_Mhl_DdcReq_Handle hDdcReq )
{
	/* Flush all DDC commands and send ABORT */
	if (!hDdcReq->hReq->bAbortSent)
	{
		hDdcReq->hReq->bAbortSent = true;
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Sending DDC OB ABORT\n");
#else
		BDBG_MSG(("Sending DDC OB ABORT"));
#endif
		BHDM_P_Mhl_Req_SendAbort_isr(hDdcReq->hReq);
		BHDM_P_Mhl_DdcReq_CancelAllCmds_isr(hDdcReq);
	}
}

BERR_Code BHDM_P_Mhl_DdcReq_GetDdcErrorCode_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  uint8_t                  *pucErr,
	  uint8_t                   ucDelay,
	  bool                      bLastCmd )
{
	return BHDM_P_Mhl_Req_1DataCmd_isr(hDdcReq->hReq,
									  BHDM_P_Mhl_Command_eGetDdcErrorCode,
									  pucErr,
									  ucDelay,
									  bLastCmd);
}

/* Check for MSC or DDC requester error pbAbortRequired will be set if we need to send ABORT */
BERR_Code BHDM_P_Mhl_DdcReq_CheckError_isr
	( BHDM_P_Mhl_DdcReq_Handle           hDdcReq,
	  int                                interrupt,
	  bool                              *pbAbortRequired,
	  BHDM_P_Mhl_CbusPkt                *pstLastPacket )
{
	BERR_Code eXmitResult = BHDM_P_MHL_CBUS_XMIT_SUCCESS;

	BDBG_ASSERT(hDdcReq);
	BDBG_OBJECT_ASSERT(hDdcReq, BHDM_MHL_DDC_REQ);

	/* Checking error in the following order */
	*pbAbortRequired = false;

	switch (interrupt)
	{
		case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_ILLEGAL_SW_WR):
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_ERROR;
			BDBG_ERR(("DDC rx illegal sw wr!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_TIMEOUT):
			/* Errors which update error code. Unlike MSC, all of these errors result in ABORT */
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_TIMEOUT;
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, SRC_DDC_ERRORCODE, BHDM_P_Mhl_CbusDdcErr_eTimeout);
#else
			hDdcReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusDdcErr_eTimeout;
#endif
			/* Unlike MSC, DDC timeout requires ABORT */
			*pbAbortRequired = true;
			BDBG_ERR(("DDC rx timeout!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_MAX_RETRIES_EXCEEDED):
			eXmitResult = BHDM_P_MHL_CBUS_XMIT_RETRY_EXCEEDED;
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, SRC_DDC_ERRORCODE, BHDM_P_Mhl_CbusDdcErr_eRetryFailed);
#else
			hDdcReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusDdcErr_eRetryFailed;
#endif
			/* Unlike MSC, DDC retries threshold exceeded requires ABORT */
			*pbAbortRequired = true;
			BDBG_ERR(("DDC req max retries exceeded!"));
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_UNEXPECTED_INBOUND_PKT):
		case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_MISMATCH):
			if (pstLastPacket->ucData != BHDM_P_Mhl_Command_eNack)
			{
				eXmitResult = BHDM_P_MHL_CBUS_XMIT_PROTO_ERROR;
#if BHDM_MHL_CTS
				BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, SRC_DDC_ERRORCODE, BHDM_P_Mhl_CbusDdcErr_eProtocolError);
#else
				hDdcReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusDdcErr_eProtocolError;
#endif
				*pbAbortRequired = true;
				BDBG_ERR(("DDC protocol error!"));
			}
			break;

		case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_CMD_ERROR):
			if (pstLastPacket->ucData == BHDM_P_Mhl_Command_eNack)
			{
				eXmitResult = BHDM_P_MHL_CBUS_XMIT_NACK;
				BDBG_ERR(("DDC req cmd NACKed!"));
			}
			else
			{
				eXmitResult = BHDM_P_MHL_CBUS_XMIT_PROTO_ERROR;
#if BHDM_MHL_CTS
				BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, SRC_DDC_ERRORCODE, BHDM_P_Mhl_CbusDdcErr_eProtocolError);
#else
				hDdcReq->hReq->ucErrCode |= BHDM_P_Mhl_CbusDdcErr_eProtocolError;
#endif
				*pbAbortRequired = true;
				BDBG_ERR(("DDC protocol error!"));
			}
			break;
	}

	return eXmitResult;
}

void BHDM_P_Mhl_DdcReq_HandleAbortDdcReceived_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq )
{
	BHDM_P_Mhl_DdcReq_CancelAllCmds_isr(hDdcReq);
}

void BHDM_P_Mhl_DdcReq_HandleAbortDdcTimeoutDone_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  BHDM_P_Mhl_CbusHpd        *peLastHpdState )
{
	/* Trigger DDC_CLEAR in ABORT_CTRL register so DDC Req state machine could resume to normal */
	BHDM_P_Mhl_Req_ClearAbort_isr(hDdcReq->hReq);

	/* assign DDC abort sent to false */
	hDdcReq->hReq->bAbortSent = false;

	/* check and read EDID if needed */
	if (hDdcReq->eEdidReadState != BHDM_P_Mhl_EdidReadState_eEnd &&
		*peLastHpdState == BHDM_P_Mhl_CbusHpd_eUp)
	{
		BERR_Code err;
		uint8_t segment, offset;

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Re-reading sink EDID\n");
#else
		BDBG_MSG(("Re-reading sink EDID"));
#endif
		/* To keep things simple we read from the beginning again */
		hDdcReq->ucEdidReadAttempt = 0;
		hDdcReq->ucEdidCurrentBlock = 0;
		segment = hDdcReq->ucEdidCurrentBlock >> 1;
		offset = (hDdcReq->ucEdidCurrentBlock & 0x1) * BHDM_P_MHL_EDID_BLOCK_SIZE;

		err = BHDM_P_Mhl_DdcReq_EdidRead_isr(hDdcReq,
										   segment,
										   offset,
										   BHDM_P_MHL_EDID_BLOCK_SIZE,
										   hDdcReq->pucEdidBlock + (BHDM_P_MHL_EDID_BLOCK_SIZE * hDdcReq->ucEdidCurrentBlock),
										   1);
		if (err != BHDM_P_MHL_CBUS_SUCCESS)
		{
#if BHDM_MHL_ENABLE_DEBUG
			BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Failed reading EDID when abort DDC timed-out.\n");
#else
			BDBG_WRN(("Failed reading EDID when abort DDC timed-out."));
#endif
		}
	}
}

void BHDM_P_Mhl_DdcReq_HandleDone_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;

#if BHDM_MHL_ENABLE_DEBUG
	char acStr[80] = "";
	char acTemp[10] = "";
#endif

	ret = BHDM_P_Mhl_Req_CompleteCmd_isr(hDdcReq->hReq, &hDdcReq->hReq->stLastCmd);
	if (ret != BHDM_P_MHL_CBUS_SUCCESS)
	{
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Failed completing REQ command.\n");
#else
		BDBG_WRN(("Failed completing REQ command."));
#endif
	}

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcpy(acStr, "lastCmd =  ");
		sprintf(acTemp, "%d", hDdcReq->hReq->stLastCmd.bLastCmd);
		strcat(acStr, acTemp);
		strcat(acStr, ", eCmdType = ");
		sprintf(acTemp, "%d ", hDdcReq->hReq->stLastCmd.eCmdType);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#endif


	/* unset EDID change and set an event for next EDID block read  */
	if (hDdcReq->hReq->stLastCmd.bLastCmd == true &&
		hDdcReq->hReq->stLastCmd.eCmdType == BHDM_P_Mhl_CbusCmdType_eEdid)
	{

#if BHDM_MHL_ENABLE_DEBUG
		strcpy(acStr, "Set EDID block read event. bEdidChanged = ");
		strcpy(acTemp, "");
		sprintf(acTemp, "%d", hDdcReq->bEdidChanged);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
#else
		BDBG_MSG(("EDID block read event. bEdidChanged = %d", hDdcReq->bEdidChanged));
#endif
		hDdcReq->bEdidChanged = false; /* Unset any EDID_CHG sent by sink */

		hDdcReq->eEvent |=  BHDM_P_MHL_DDCREQ_EVENT_EDID_BLOCK_READ;
	}
}

/* Helper for mid processing loop.
   Handle DDC resquester interrupts */
BERR_Code BHDM_P_Mhl_DdcReq_HandleErrors_isr
	( BHDM_P_Mhl_DdcReq_Handle   hDdcReq,
	  int                        interrupt,
	  bool                      *pbStopPending,
	  BHDM_P_Mhl_MscReq_Event   *peMscReqEvent )
{
	BERR_Code ret = BHDM_P_MHL_CBUS_SUCCESS;
	BERR_Code xmitResult;
	bool bAbortRequired = false;

	BDBG_ASSERT(hDdcReq);

	/* First fetch the completion result and then check for errors. */
	ret = BHDM_P_Mhl_Req_CompleteCmd_isr(hDdcReq->hReq, &hDdcReq->hReq->stLastCmd);

	xmitResult = BHDM_P_Mhl_DdcReq_CheckError_isr(hDdcReq,
						interrupt, &bAbortRequired,
						&hDdcReq->hReq->stLastCmd.cbusPackets.astLongCmd[hDdcReq->hReq->stLastCmd.ulNumPacketsDone-1]);

	/* actions needed on certain errors */
	/* If it is DDC, we send EOF, and then re-read an entire EDID block if it is EDID read. */
	if (xmitResult == BHDM_P_MHL_CBUS_XMIT_NACK)
	{
		/* First cancel the current batch of commands */
		/* As we are doing only EDID read in MPM FW, just cancel the full DDC Q  */
		BHDM_P_Mhl_DdcReq_CancelAllCmds_isr(hDdcReq);

		/* And queue the EOF command on DDC channel if NACK.
		   Kept this in separate block to reduce stack build up due to instantiation of cmd structure */
		{
			BHDM_P_Mhl_CbusCmd stDdcCmd;

			stDdcCmd.eCmdType = BHDM_P_Mhl_CbusCmdType_eDdc;
			stDdcCmd.ucDelay = 0;
			stDdcCmd.eState = BHDM_P_Mhl_CbusCmdState_ePending;
			stDdcCmd.ulNumPacketsCfg = 1;
			stDdcCmd.ulNumPacketsDone = 0;
			stDdcCmd.bLastCmd = true;
			stDdcCmd.pucReplyBuf = NULL;
			stDdcCmd.ucReplyBufSize = 0;
			stDdcCmd.ucReplyBufValidSize = 0;
			stDdcCmd.eDest = BHDM_P_Mhl_CbusDest_eDdcReq;
			stDdcCmd.ePriority = BHDM_P_Mhl_CbusPriority_eDdc;

			stDdcCmd.cbusPackets.astLongCmd[0].ulDir = BHDM_P_Mhl_CbusPktDirection_eReq;
			stDdcCmd.cbusPackets.astLongCmd[0].ulType = BHDM_P_Mhl_CbusPktType_eCtrl;
			stDdcCmd.cbusPackets.astLongCmd[0].ucData = BHDM_P_Mhl_Command_eEof;

			if (BHDM_P_Mhl_Req_AddCmd_isr(hDdcReq->hReq, &stDdcCmd) == BHDM_P_MHL_CBUS_SUCCESS)
			{
#if BHDM_MHL_CTS
				BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, DDC_REQ_PENDING, 1);
#else
				hDdcReq->hReq->bRequestPending = true;
#endif
			}
		}

		/* generate MSC REQ event to fetch the DDC error code h*/
		*peMscReqEvent |= BHDM_P_MHL_MSCREQ_EVENT_READ_DDC_ERROR_CODE;

		/* Next requeue the EDID block read if it is an EDID read */
		if (!*pbStopPending && hDdcReq->hReq->stLastCmd.eCmdType == BHDM_P_Mhl_CbusCmdType_eEdid)
		{
			hDdcReq->ucEdidReadAttempt++;
			if(hDdcReq->ucEdidReadAttempt < BHDM_P_MHL_CBUS_MAX_EDID_READ_ATTEMPT)
			{
				uint8_t segment = hDdcReq->ucEdidCurrentBlock >> 1; /* 2 blocks per segment */
				uint8_t offset = (hDdcReq->ucEdidCurrentBlock & 0x1) * BHDM_P_MHL_EDID_BLOCK_SIZE;

#if BHDM_MHL_ENABLE_DEBUG
				{
					char acStr[40] = "";
					sprintf(acStr, "EDID block %d read attempt %d\n", hDdcReq->ucEdidCurrentBlock, hDdcReq->ucEdidReadAttempt+1);
					BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
				}
#else
				BDBG_MSG(("EDID block %d read attempt %d", hDdcReq->ucEdidCurrentBlock, hDdcReq->ucEdidReadAttempt+1));
#endif

				ret = BHDM_P_Mhl_DdcReq_EdidRead_isr(hDdcReq,
													 segment,
													 offset,
													 BHDM_P_MHL_EDID_BLOCK_SIZE,
													 hDdcReq->pucEdidBlock +
													 BHDM_P_MHL_EDID_BLOCK_SIZE * hDdcReq->ucEdidCurrentBlock,
													 1);
				if (ret == BHDM_P_MHL_CBUS_SUCCESS)
				{
#if BHDM_MHL_CTS
					BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, DDC_REQ_PENDING, 1);
#else
					hDdcReq->hReq->bRequestPending = true;
#endif
				}
			}
			else
			{
				/* We will not mark EDID as valid in this case, but we will try again if we get a CLR_HPD followed by SET_HPD sequence */
#if BHDM_MHL_CTS
				BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hDdcReq->hReq->hRegister, EDID_VALID);
				BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, EDID_SIZE, 0);
#else
				hDdcReq->bEdidValid = false;
				hDdcReq->ulEdidSize = 0;
#endif
				BDBG_MSG(("Max. EDID read attempts exceeded, giving up."));
#if BHDM_MHL_ENABLE_DEBUG
				BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Max. EDID read attempts exceeded, giving up.\n");
#else
				BDBG_MSG(("Max. EDID read attempts exceeded, giving up."));
#endif
			}
		}
	}

	/* Need to send ABORT? send it now */
	if (bAbortRequired)
	{
		BHDM_P_Mhl_DdcReq_AbortTx_isr(hDdcReq);
	}

	return xmitResult;
}


/* Higher level handling SET_HPD */
void BHDM_P_Mhl_DdcReq_HandleSetHpd_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  BHDM_P_Mhl_CbusHpd       *peLastHpdState,
	  bool                      bStopPending )
{
#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		strcat(acStr, "Handle Set HPD. State =  ");
		sprintf(acTemp, "%d ", *peLastHpdState);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
	BDBG_MSG(("Handle Set HPD. State = %d", *peLastHpdState));
#endif
	/* If this comes after CLR_HPD, start reading EDID.
	   TODO: need to check the time interval since we
	   last received CLR_HPD */
#if BHDM_MHL_CTS
	BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, HPD, 1);
#else
	hDdcReq->bHpd = true;
#endif

	if(hDdcReq->hReq->bAbortActive)
	{
		/* Handle it after abort timer runs out (if there is abort)  */
		hDdcReq->eUnhandledEvent |= BHDM_P_MHL_DDCREQ_EVENT_SET_HPD;
	}
	else if (*peLastHpdState == BHDM_P_Mhl_CbusHpd_eDown)
	{
		BERR_Code ret;

		hDdcReq->eEdidReadState = BHDM_P_Mhl_EdidReadState_eMiddle;
		hDdcReq->ulEdidNumBlocks = 0;
		hDdcReq->ucEdidCurrentBlock = 0;
		hDdcReq->ucEdidReadAttempt = 0;
#if BHDM_MHL_CTS
		BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hDdcReq->hReq->hRegister, EDID_VALID);
		BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, EDID_SIZE, 0);
#else
		hDdcReq->bEdidValid = false;
		hDdcReq->ulEdidSize = 0;
#endif

		if(!bStopPending)
		{
			ret = BHDM_P_Mhl_DdcReq_EdidRead_isr(hDdcReq,
												 0,
												 0,
												 BHDM_P_MHL_EDID_BLOCK_SIZE,
												 hDdcReq->pucEdidBlock,
												 1);

			if (ret == BHDM_P_MHL_CBUS_SUCCESS)
			{
				/* So we don't read EDID again if we see SET_HPD without seeing CLR_HPD first */
				*peLastHpdState = BHDM_P_Mhl_CbusHpd_eUp;
#if BHDM_MHL_CTS
				BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, DDC_REQ_PENDING, 1);
#else
				hDdcReq->hReq->bRequestPending = true;
#endif
			}
			else
			{
				/* Try again next time */
				hDdcReq->eUnhandledEvent |= BHDM_P_MHL_DDCREQ_EVENT_SET_HPD;
			}
		}
	} /* Ignore multiple SET_HPD events */
}

/* Higher level handling CLR_HPD */
BERR_Code BHDM_P_Mhl_DdcReq_HandleClrHpd_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  BHDM_P_Mhl_CbusState     *pstCbusState )
{
	BDBG_ASSERT(hDdcReq);

	/* Clear all DDC commands */
	BHDM_P_Mhl_DdcReq_CancelAllCmds_isr(hDdcReq);

	/* Reset EDID and HPD state */
	pstCbusState->eLastHpdState = BHDM_P_Mhl_CbusHpd_eDown;
	BHDM_P_Mhl_DdcReq_ClrHpdState_isr(hDdcReq);

	return BERR_SUCCESS;
}

/* Handle EDID block read completion */
BERR_Code BHDM_P_Mhl_DdcReq_HandleEdidReadBlock_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  uint32_t                 *pulUnhandledEvents )
{
	BERR_Code err = BHDM_P_MHL_CBUS_SUCCESS;
	uint8_t segment, offset;
	BERR_Code ret;

#if BHDM_MHL_ENABLE_DEBUG
	char acStr[120] = "";
	char acTemp[10] = "";
#endif

	BDBG_ASSERT(hDdcReq);

#if BHDM_MHL_ENABLE_DEBUG
		strcpy(acStr, "BHDM_P_Mhl_DdcReq_HandleEdidReadBlock_isr. bEdidChanged = ");
		sprintf(acTemp, "%d ", hDdcReq->bEdidChanged);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
#endif

	/* We finish reading one block of EDID
	   1. If this is the first block, check the total nnumber
	      of blocks in byte 0x7e.
	   2. If there are further blocks to read, queue more EDID read commands.
	   3. If this the last block, mark EDID as valid in the mailbox.
	*/
	if(hDdcReq->bEdidChanged)
	{
		/* This can happen if the sink sets EDID_CHG as soon as
		   we send EOF, but before we can handle the EDID block completion */
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("EDID_CHG is set. Ignoring EDID block completion event.");
#else
		BDBG_MSG(("EDID_CHG is set. Ignoring EDID block completion event."));
#endif
		return err;
	}

#if BHDM_MHL_ENABLE_DEBUG
	{
		strcpy(acStr, "Read EDID block ");
		strcpy(acTemp, "");
		sprintf(acTemp, "%d ", hDdcReq->ucEdidCurrentBlock);
		strcat(acStr, acTemp);
		strcat(acStr, "out of ");
		sprintf(acTemp, "%d ", hDdcReq->ulEdidNumBlocks);
		strcat(acStr, acTemp);
		strcat(acStr, " (max ");
		sprintf(acTemp, "%d ", BHDM_P_MHL_MAILBOX_EDID_SIZE/BHDM_P_MHL_EDID_BLOCK_SIZE);
		strcat(acStr, acTemp);
		strcat(acStr, "blocks.\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
	BDBG_MSG(("Read EDID block %d out of %d (max %d blocks).",
			hDdcReq->ucEdidCurrentBlock, hDdcReq->ulEdidNumBlocks, BHDM_P_MHL_MAILBOX_EDID_SIZE/BHDM_P_MHL_EDID_BLOCK_SIZE));
#endif
	hDdcReq->ucEdidReadAttempt = 0;

	if(hDdcReq->ucEdidCurrentBlock == 0)
	{
		uint8_t ucMaxEdidBlocks = BHDM_P_MHL_MAILBOX_EDID_SIZE/BHDM_P_MHL_EDID_BLOCK_SIZE;

		hDdcReq->ulEdidNumBlocks = hDdcReq->pucEdidBlock[BHDM_P_MHL_EDID_SIZE_OFFSET] + 1;
		/* Mailbox only has space for 4 EDID blocks make sure we
		   don't overread. We have never seen TVs with more than
		   2 blocks and EDID CTS only requires 4. */
		if(hDdcReq->ulEdidNumBlocks > ucMaxEdidBlocks)
		{
			hDdcReq->ulEdidNumBlocks = ucMaxEdidBlocks;
			BDBG_MSG(("EDID was truncated to %d blocks", ucMaxEdidBlocks));
		}

		BDBG_MSG((" %d more blocks to read", hDdcReq->ulEdidNumBlocks-1));
	}

	hDdcReq->ucEdidCurrentBlock++;

	if(hDdcReq->ucEdidCurrentBlock < hDdcReq->ulEdidNumBlocks)
	{
		BDBG_MSG(("Reading block %d", hDdcReq->ucEdidCurrentBlock));

		segment = hDdcReq->ucEdidCurrentBlock >> 1; /* 2 blocks per segment */
		offset = (hDdcReq->ucEdidCurrentBlock & 0x1) * BHDM_P_MHL_EDID_BLOCK_SIZE; /* offset in the block */
		ret = BHDM_P_Mhl_DdcReq_EdidRead_isr(hDdcReq,
											segment,
											offset,
											BHDM_P_MHL_EDID_BLOCK_SIZE,
											hDdcReq->pucEdidBlock + (BHDM_P_MHL_EDID_BLOCK_SIZE * hDdcReq->ucEdidCurrentBlock),
											1);

		if(ret != BHDM_P_MHL_CBUS_SUCCESS)
			*pulUnhandledEvents |= BHDM_P_MHL_DDCREQ_EVENT_EDID_BLOCK_READ;
		else
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hDdcReq->hReq->hRegister, DDC_REQ_PENDING, 1);
#else
			hDdcReq->hReq->bRequestPending = true;
#endif
	}
	else
	{
		/* We have read everything */
#if BHDM_MHL_CTS
		BHDM_P_Mhl_Mailbox_EdidUpdateDone_isr(hDdcReq->hReq->hRegister);
#else
		hDdcReq->bEdidValid = true;
		hDdcReq->bSinkEdidChg = false;
#endif
		hDdcReq->eEdidReadState = BHDM_P_Mhl_EdidReadState_eEnd;

#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Dumping EDID.\n");
		BHDM_P_Mhl_DumpEdid_isr(hDdcReq->pucEdidBlock, hDdcReq->ulEdidNumBlocks);
#endif
	}
	return err;
}

void BHDM_P_Mhl_DdcReq_HandleEdidChgInt_isr
	( BHDM_P_Mhl_DdcReq_Handle  hDdcReq,
	  uint32_t                 *pulUnhandledEvents )
{
	/* If we are in the middle of reading EDID,
		 remove existing EDID read commands and start
		 reading EDID block 0 again */

	BERR_Code ret;

#if BHDM_MHL_ENABLE_DEBUG
	BHDM_P_MHL_DEBUG_DUMP_TO_FILE("EDID_CHG is set.\n");
#else
	BDBG_MSG(("EDID_CHG is set."));
#endif
	hDdcReq->bEdidChanged = true; /* This cancels any edid_block_read event */

	if(hDdcReq->eEdidReadState == BHDM_P_Mhl_EdidReadState_eMiddle)
	{
		BHDM_P_Mhl_DdcReq_CancelAllCmds_isr(hDdcReq);
	}
	hDdcReq->eEdidReadState = BHDM_P_Mhl_EdidReadState_eMiddle;
	hDdcReq->ulEdidNumBlocks = 0;
	hDdcReq->ucEdidCurrentBlock = 0;
	hDdcReq->ucEdidReadAttempt = 0;
#if BHDM_MHL_CTS
	{
		BREG_Handle hRegister = hDdcReq->hReq->hRegister;
		BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, EDID_VALID);
		BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, EDID_SIZE, 0);
	}
#else
	hDdcReq->bEdidValid = false;
	hDdcReq->ulEdidSize = 0;
#endif

	if(!hDdcReq->hReq->bAbortActive)
	{
		ret = BHDM_P_Mhl_DdcReq_EdidRead_isr(hDdcReq, 0, 0, BHDM_P_MHL_EDID_BLOCK_SIZE, hDdcReq->pucEdidBlock, 1);
		if(ret == BHDM_P_MHL_CBUS_SUCCESS)
		{
#if BHDM_MHL_CTS
			{
				BREG_Handle hRegister = hDdcReq->hReq->hRegister;
				BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SINK_EDID_CHG);
				BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, DDC_REQ_PENDING, 1);
			}
#else
			hDdcReq->bSinkEdidChg = false;
			hDdcReq->hReq->bRequestPending = true;
#endif
		}
		else
		{
			/* Try again next time */
			*pulUnhandledEvents |= BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT;
		}
	}
	else
	{
		/* Try again after abort timer runs out */
		*pulUnhandledEvents |= BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT;
	}
}
