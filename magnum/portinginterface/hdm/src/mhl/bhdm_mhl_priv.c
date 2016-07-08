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
#include "bhdm_mhl_priv.h"
#include "../common/bhdm_priv.h"

BDBG_MODULE(BHDM_MHL);

#define	BHDM_MHL_CHECK_RC( rc, func )	          \
do                                                \
{										          \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										      \
		goto done;							      \
	}										      \
} while(0)

/* Heartbeat packets:
   Default: GET_STATE
   Alternative: READ_DEVCAP [0]
 */
const BHDM_P_Mhl_CbusPkt s_astDefaultHbPkts[2] =
	{{BHDM_P_Mhl_CbusPktType_eCtrl, BHDM_P_Mhl_CbusPktDirection_eReq,  BHDM_P_Mhl_Command_eGetState},
	 {BHDM_P_Mhl_CbusPktType_eData, BHDM_P_Mhl_CbusPktDirection_eResp, 0}};
const uint32_t s_ulDefaultHbPktCnt = sizeof(s_astDefaultHbPkts)/sizeof(s_astDefaultHbPkts[0]);

const BHDM_P_Mhl_CbusPkt s_astSafeHbPkts[4] =
	{{BHDM_P_Mhl_CbusPktType_eCtrl, BHDM_P_Mhl_CbusPktDirection_eReq,  BHDM_P_Mhl_Command_eReadDevCap},
	 {BHDM_P_Mhl_CbusPktType_eData, BHDM_P_Mhl_CbusPktDirection_eReq,  BHDM_P_Mhl_DevCapOffset_eDevStateAddr},
	 {BHDM_P_Mhl_CbusPktType_eCtrl, BHDM_P_Mhl_CbusPktDirection_eResp, BHDM_P_Mhl_Command_eAck},
	 {BHDM_P_Mhl_CbusPktType_eData, BHDM_P_Mhl_CbusPktDirection_eResp, 0}};
const uint32_t s_ulSafeHbPktCnt = sizeof(s_astSafeHbPkts)/sizeof(s_astSafeHbPkts[0]);


typedef struct BHDM_P_Mhl_InterruptCbTable
{
	BINT_Id       	  IntrId;
	BINT_CallbackFunc pfCallback;
	int               iParam2;
	bool              enable; /* debug purposes */
} BHDM_P_Mhl_InterruptCbTable;


#define BHDM_P_MHL_CBUS_INT_0_CB_DEF(intr, id, enable) \
      {BCHP_INT_ID_##intr, BHDM_P_Mhl_HandleCbus0Interrupt_isr, BHDM_MHL_CBUS_INTR_0_##id, enable}

#define BHDM_P_MHL_CBUS_INT_0_CREATE_TABLE(instance)\
{ \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(DISCOVERY_SUCCEEDED, eDISCOVERY_SUCCEEDED, false), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(DISCOVERY_FAILED, eDISCOVERY_FAILED, false), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(DISCONNECT_RECEIVED, eDISCONNECT_RECEIVED, true), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(DISCONNECT_DONE, eDISCONNECT_DONE, false), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(INITIATOR_PKT, eINITIATOR_PKT, false), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(FOLLOWER_PKT, eFOLLOWER_PKT, false), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(FOLLOWER_DROP_PKT, eFOLLOWER_DROP_PKT, false), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(SCHEDULER_DROP_PKT, eSCHEDULER_DROP_PKT, false), \
	BHDM_P_MHL_CBUS_INT_0_CB_DEF(IMP_CHANGED, eIMP_CHANGED, false), \
\
}

static const BHDM_P_Mhl_InterruptCbTable BHDM_P_Mhl_Cbus0Interrupts[] = BHDM_P_MHL_CBUS_INT_0_CREATE_TABLE(_);

#define BHDM_P_MHL_CBUS_INT_1_CB_DEF(intr, id, enable) \
      {BCHP_INT_ID_##intr, BHDM_P_Mhl_HandleCbus1Interrupt_isr, BHDM_MHL_CBUS_INTR_1_##id, enable}

#define BHDM_P_MHL_CBUS_INT_1_CREATE_TABLE(instance)\
{ \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_DONE, eMSC_REQ_DONE, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_RX_MISMATCH, eMSC_REQ_RX_MISMATCH, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_RX_CMD_ERROR, eMSC_REQ_RX_CMD_ERROR, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_RX_TIMEOUT, eMSC_REQ_RX_TIMEOUT, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_ILLEGAL_SW_WR, eMSC_REQ_ILLEGAL_SW_WR, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_MAX_RETRIES_EXCEEDED, eMSC_REQ_MAX_RETRIES_EXCEEDED, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_UNEXPECTED_INBOUND_PKT, eMSC_REQ_UNEXPECTED_INBOUND_PKT, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_BUS_STOLEN, eMSC_REQ_BUS_STOLEN, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_REQ_CANCELLED, eMSC_REQ_CANCELLED, true), \
\
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_IB_DONE, eMSC_RESP_IB_DONE, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_OB_DONE, eMSC_RESP_OB_DONE, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_BAD_CMD, eMSC_RESP_BAD_CMD, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_UNEXPECTED_CMD, eMSC_RESP_UNEXPECTED_CMD, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_DATA_RECEIVED, eMSC_RESP_DATA_RECEIVED, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_DATA_OVERFLOW, eMSC_RESP_DATA_OVERFLOW, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_RX_TIMEOUT, eMSC_RESP_RX_TIMEOUT, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_SW_TIMEOUT, eMSC_RESP_SW_TIMEOUT, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_ILLEGAL_SW_WR, eMSC_RESP_ILLEGAL_SW_WR, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_MAX_RETRIES_EXCEEDED, eMSC_RESP_MAX_RETRIES_EXCEEDED, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(MSC_RESP_UNEXPECTED_INBOUND_PKT, eMSC_RESP_UNEXPECTED_INBOUND_PKT, true), \
\
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(DDC_REQ_DONE, eDDC_REQ_DONE, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(DDC_REQ_RX_MISMATCH, eDDC_REQ_RX_MISMATCH, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(DDC_REQ_RX_CMD_ERROR, eDDC_REQ_RX_CMD_ERROR, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(DDC_REQ_RX_TIMEOUT, eDDC_REQ_RX_TIMEOUT, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(DDC_REQ_ILLEGAL_SW_WR, eDDC_REQ_ILLEGAL_SW_WR, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(DDC_REQ_MAX_RETRIES_EXCEEDED, eDDC_REQ_MAX_RETRIES_EXCEEDED, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(DDC_REQ_UNEXPECTED_INBOUND_PKT, eDDC_REQ_UNEXPECTED_INBOUND_PKT, true), \
\
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(ABORT_MSC_RECEIVED, eABORT_MSC_RECEIVED, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(ABORT_DDC_RECEIVED, eABORT_DDC_RECEIVED, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(ABORT_MSC_INBOUND_TIMEOUT_DONE, eABORT_MSC_INBOUND_TIMEOUT_DONE, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(ABORT_MSC_OUTBOUND_TIMEOUT_DONE, eABORT_MSC_OUTBOUND_TIMEOUT_DONE, true), \
	BHDM_P_MHL_CBUS_INT_1_CB_DEF(ABORT_DDC_TIMEOUT_DONE, eABORT_DDC_TIMEOUT_DONE, true), \
\
}

static const BHDM_P_Mhl_InterruptCbTable BHDM_P_Mhl_Cbus1Interrupts[] = BHDM_P_MHL_CBUS_INT_1_CREATE_TABLE(_);


#define BHDM_P_MHL_MPM_HOST_INT_CB_DEF(intr, id, enable) \
      {BCHP_INT_ID_MPM_HOST_L2_CPU_STATUS_##intr, BHDM_P_Mhl_HandleMpmHostInterrupt_isr, BHDM_MHL_MPM_HOST_INTR_##id, enable}

#define BHDM_P_MHL_MPM_HOST_CREATE_TABLE(instance)\
{ \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(MPM_INTR_SPARE0, eMPM_INTR_SPARE0, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(MPM_INTR_SPARE1, eMPM_INTR_SPARE1, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(MPM_INTR_SPARE2, eMPM_INTR_SPARE2, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(MPM_INTR_SPARE3, eMPM_INTR_SPARE3, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(MSPI_INTR_0, eMSPI_INTR_0, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(MSPI_INTR_1, eMSPI_INTR_1, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(A2R_TIMEOUT_INTR, eA2R_TIMEOUT_INTR, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(A2R_BAD_SIZE_INTR, eA2R_BAD_SIZE_INTR, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(S3_TO_S0, eS3_TO_S0, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(S0_TO_S3, eS0_TO_S3, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(SW_INTR_0, eSW_INTR_0, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(SW_INTR_1, eSW_INTR_1, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(SW_INTR_2, eSW_INTR_2, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(SW_INTR_3, eSW_INTR_3, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(SW_INTR_4, eSW_INTR_4, true), \
	BHDM_P_MHL_MPM_HOST_INT_CB_DEF(SW_INTR_5, eSW_INTR_5, true), \
\
}

static const BHDM_P_Mhl_InterruptCbTable BHDM_P_Mhl_MpmHostInterrupts[] = BHDM_P_MHL_MPM_HOST_CREATE_TABLE(_);

#if BHDM_MHL_ENABLE_DEBUG
FILE *s_pfCbusLog;
#endif


static void BHDM_P_Mhl_SendClockMode_isr
	( const BHDM_P_Mhl_Handle hMhl )
{
	BDBG_ASSERT(hMhl);

	if (hMhl->hostState == BHDM_P_Mhl_HostCpuState_eActive)
	{
		uint8_t ucSrcLinkMode = 0;

		BDBG_MSG(("Sending %s link mode and PATH_EN=1 to Sink.", (hMhl->eMhlClkMode == 2) ? "packed pixel" : ((hMhl->eMhlClkMode == 3) ? "24-bit" : "HDMI")));
		BHDM_P_MHL_SET_FIELD(ucSrcLinkMode, BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_0_MASK, BHDM_P_MHL_MAILBOX_SRC_CLK_MODE_0_LSB, hMhl->eMhlClkMode);

		ucSrcLinkMode |= BHDM_P_MHL_STAT_LINK_MODE_PATH_EN;

		if (BHDM_P_Mhl_MscReq_WriteStat_isr(hMhl->hMscReq, BHDM_P_Mhl_StatusAddr_eLinkModeAddr, ucSrcLinkMode, 1, true) != BHDM_P_MHL_CBUS_SUCCESS)
		{
			BDBG_ERR(("Failed to send link mode and PATH_EN=1 to sink."));
		}

		/* If nothing is going on in the MSC Requester, send the next command from MSC REQ Queue */
		BHDM_P_Mhl_Req_Active_isr(hMhl->hMscReq->hReq, &hMhl->hMscReq->hReq->bActive, &hMhl->hMscReq->hReq->bAbortActive);
		if(!hMhl->hMscReq->hReq->bAbortActive && !hMhl->hMscReq->hReq->bActive)
		{
			BDBG_MSG(("Send MSC WriteStat Command"));
			BHDM_P_Mhl_Req_SendNextCmd_isr(hMhl->hMscReq->hReq,
											&hMhl->hMscReq->hReq->stLastCmd,
											&hMhl->hMscReq->hReq->bRetryLastCmd);
		}

	}
	else
	{
		BDBG_MSG(("Sending %s link mode to MPM.", (hMhl->eMhlClkMode == 2) ? "packed pixel" : ((hMhl->eMhlClkMode == 3) ? "24-bit" : "HDMI")));
		BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hMhl->hRegister, SRC_LINK_MODE_0, hMhl->eMhlClkMode);

		BDBG_MSG(("Sending PATH_EN=1 to MPM."));
		BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hMhl->hRegister, SRC_PATH_EN_0, BHDM_P_Mhl_PathEn_e1);
	}

	hMhl->ePathEnSentState = BHDM_P_Mhl_CbusPathEnSentState_eSent;
}

static void BHDM_P_Mhl_SendClockMode
	( const BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_SendClockMode_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

/* Get channel status */
static void BHDM_P_Mhl_GetChanStatus_isr
	( BHDM_P_Mhl_Handle hMhl )
{
	BHDM_P_Mhl_Req_Active_isr(hMhl->hMscReq->hReq,
						 &hMhl->hMscReq->hReq->bActive,
						 &hMhl->hMscReq->hReq->bAbortActive);

	BHDM_P_Mhl_MscResp_Active_isr(hMhl->hMscResp,
						  &hMhl->hMscResp->bActive,
						  &hMhl->hMscResp->bIbAbortActive,
						  &hMhl->hMscResp->bObAbortActive);

	BHDM_P_Mhl_Req_Active_isr(hMhl->hDdcReq->hReq,
						 &hMhl->hDdcReq->hReq->bActive,
						 &hMhl->hDdcReq->hReq->bAbortActive);
}

static void BHDM_P_Mhl_GetDeviceCapabilities
	( BHDM_P_Mhl_Handle              hMhl,
	  BHDM_P_Mhl_DeviceCapabilities *pstMhlDevCap )
{
	BREG_Handle hRegister = hMhl->hRegister;
	uint8_t ucValue;

#if BHDM_MHL_TEST_DEVEL_MODE
	uint8_t ucDcapStatus;

	ucDcapStatus = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SINK_DCAP_RDY);
	if (!ucDcapStatus)
	{
		do
		{
			ucDcapStatus = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SINK_DCAP_RDY);
		} while (!ucDcapStatus);
	}
#endif

	/* Get power limit */
	ucValue = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SINK_DCAP_DEV_CAT);
	pstMhlDevCap->ePwrLimit = (BHDM_P_Mhl_PowerLimit)BHDM_P_MHL_GET_FIELD(ucValue, BHDM_P_MHL_POW_LIMIT_MASK, BHDM_P_MHL_POW_LIMIT_LSB);

	/* Get supported video link modes */
	ucValue = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SINK_DCAP_VLINK_MODE);
	pstMhlDevCap->bSupportsRgb444 = (ucValue & BHDM_P_MHL_VIDEO_LINK_MODE_RGB444) ? true : false;
	pstMhlDevCap->bSupportsYCbCr444 = (ucValue & BHDM_P_MHL_VIDEO_LINK_MODE_YCBCR444) ? true : false;
	pstMhlDevCap->bSupportsYCbCr422 = (ucValue & BHDM_P_MHL_VIDEO_LINK_MODE_YCBCR422) ? true : false;
	pstMhlDevCap->bSupportsPackedPixel = (ucValue & BHDM_P_MHL_VIDEO_LINK_MODE_PACKED_PIXEL) ? true : false;
	pstMhlDevCap->bSupportsDataIslands = (ucValue & BHDM_P_MHL_VIDEO_LINK_MODE_DATA_ISLANDS) ? true : false;
	pstMhlDevCap->bSupportsVga = (ucValue & BHDM_P_MHL_VIDEO_LINK_MODE_VGA) ? true : false;

	BDBG_MSG(("DCAP: [0x%x]", ucValue));

}

static void BHDM_P_Mhl_ResetHeartBeat_isr
	( BHDM_P_Mhl_Handle hMhl )
{
	hMhl->iHeartbeatTimer = BHDM_P_MHL_CBUS_HEARTBEAT_TICKS;
}

static void BHDM_P_Mhl_ResetHeartBeat
	( BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_ResetHeartBeat_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

/* Handle HeartBeat */
void BHDM_P_Mhl_HandleHeartbeat_isr
	( BHDM_P_Mhl_Handle hMhl )
{
	BDBG_ASSERT(hMhl);

	hMhl->ulHeartbeatCount++;

	/* Just queue a GET_STATE command unless we are waiting to stop */
	if(!hMhl->bStopPending)
	{
		BHDM_P_Mhl_Req_Active_isr(hMhl->hMscReq->hReq, &hMhl->hMscReq->hReq->bActive, &hMhl->hMscReq->hReq->bAbortActive);
		BHDM_P_Mhl_MscResp_Active_isr(hMhl->hMscResp, &hMhl->hMscResp->bActive, &hMhl->hMscResp->bIbAbortActive, &hMhl->hMscResp->bObAbortActive);

		if(hMhl->hMscReq->hReq->bAbortActive || hMhl->hMscResp->bIbAbortActive || hMhl->hMscResp->bObAbortActive)
		{
			BDBG_WRN(("MSC abort currently active, not sending heartbeat!"));
#if BHDM_P_MHL_ENABLE_HEARTBEAT_TIMER
			/* Try again */
			RESET_HEARTBEAT;
#endif
		}
		else
		{
			/* The heartbeat timer could have been reset between heartbeat event being set and we get here because of an incoming MSC RESP command */
			BERR_Code ret = BHDM_P_Mhl_MscReq_HeartBeat_isr(hMhl->hMscReq, hMhl->pstHbPkts,
															hMhl->ulNumHbPkts, 0, true);

			/* If we failed to queue the heartbeat, we will try  again at the next heartbeat, otherwise no more
			   heartbeat will go out until this one has been sent */
			if(ret != BHDM_P_MHL_CBUS_SUCCESS)
			{
				BDBG_WRN(("CBUS failed to issue heartbeat!"));

#if BHDM_P_MHL_ENABLE_HEARTBEAT_TIMER
				RESET_HEARTBEAT;
#endif
			}
			else
			{
				hMhl->bHeartbeatQueued = true;
				hMhl->hMscReq->hReq->bRequestPending = true;
			}
		}
	}
	else
	{
		BDBG_MSG(("Pending handover, not sending heartbeat"));
	}
}

static void BHDM_P_Mhl_HandleAbortMscObTimeoutDone_isr
	( BHDM_P_Mhl_Handle  hMhl )
{
#if BHDM_P_MHL_ENABLE_HEARTBEAT_TIMER
	/* enable heartbeat if abort done */
	RESET_HEARTBEAT;
	ENABLE_HEARTBEAT;
#endif

	/* TODO - Triggering MSC_CLEAR in ABORT_CTRL register and setting abort active flags to flase done in MSC Resp task for the same event. Do we need to re-do here?*/

	/* If there was a pending heartbeat, try again because it would have been cancelled by the ABORT */
	if(hMhl->bHeartbeatQueued)
	{
		BDBG_MSG(("Resuming heartbeat"));
		hMhl->hMscReq->eEvent |= BHDM_P_MHL_MSCREQ_EVENT_HEARTBEAT;

		if(++hMhl->ulNumHbFailed >= BHDM_P_MHL_CBUS_HEARTBEAT_DEAD_PKTS && hMhl->pstHbPkts != &s_astSafeHbPkts[0])
		{
			BDBG_MSG(("Switching to alternative heartbeat command"));
			hMhl->pstHbPkts = &s_astSafeHbPkts[0];
			hMhl->ulNumHbPkts = s_ulSafeHbPktCnt;
		}
	}

	/* Continue to read DCAP if we haven't finished */
	if((hMhl->hMscReq->eDcapReadState != BHDM_P_Mhl_DcapReadState_eEnd) && BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hMhl->hRegister, SINK_DCAP_RDY))
	{
		hMhl->hMscReq->eDcapReadState = BHDM_P_Mhl_DcapReadState_eMiddle;
		hMhl->bSinkPlimValid = false;
		BDBG_MSG(("Re-reading sink DCAP"));
		BHDM_P_Mhl_MscReq_ReadDcap_isr(hMhl->hMscReq, &hMhl->stCbusState, 1, 1);
	}
}

static void BHDM_P_Mhl_HandleReceivedLinkMode_isr
	( BHDM_P_Mhl_Handle  hMhl )
{
	BREG_Handle hRegister = hMhl->hRegister;

	uint8_t ucPathEn = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_PATH_EN);
	uint8_t ucTxReady = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, HOST_TX_READY);
	uint8_t ucSrcLinkMode = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SRC_LINK_MODE_1);

#if BHDM_MHL_ENABLE_DEBUG
	uint8_t ucSinkLinkMode = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_LINK_MODE);
	BDBG_MSG(("SINK_LINK_MODE is 0x%x", ucSinkLinkMode));
#endif

	if (ucPathEn)
		BDBG_MSG(("PATH_EN is set"));
	else
		BDBG_MSG(("PATH_EN is NOT set"));

	/*
	 * Handling PATH_EN
	 * If sink sends us PATH_EN=0, we always reply with PATH_EN=0 and disable TMDS
	 * If sink sends us PATH_EN=1, we NACK the message if TX_READY is not set and mark PATH_EN as pending
	 * otherwise ACK the message and send PATH_EN=1 and then enable TMDS
	 */
	if(ucPathEn == 0)
	{
		/* Disable TMDS BEFORE sending PATH_EN=0 */
		BHDM_P_Mhl_Host_EnableMhlTx_isr(hRegister, false);

		hMhl->ePathEnSentState = BHDM_P_Mhl_CbusPathEnSentState_eNotNeeded;

		/* Reply PATH_EN=0 to sink */
		BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SRC_PATH_EN_1);
		ucSrcLinkMode = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SRC_LINK_MODE_1);
		BHDM_P_Mhl_MscReq_WriteStat_isr(hMhl->hMscReq, BHDM_P_Mhl_StatusAddr_eLinkModeAddr, ucSrcLinkMode, 1, true);
#if BHDM_MHL_CTS
		BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, MSC_REQ_PENDING, 1);
#else
		hMhl->hMscReq->hReq->bRequestPending = true;
#endif
	}
	else
	{
		if(ucTxReady)
		{
			ucSrcLinkMode = BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SRC_LINK_MODE_0);
			BDBG_MSG(("SRC_LINK_MODE_0 from host is %d", ucSrcLinkMode));

			/* Copy from link mode set by host.  PATH_EN must be set regardless what the host sets */
			ucSrcLinkMode |= BHDM_P_MHL_STAT_LINK_MODE_PATH_EN;
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, SRC_LINK_MODE_1, ucSrcLinkMode);

			BHDM_P_Mhl_MscReq_WriteStat_isr(hMhl->hMscReq, BHDM_P_Mhl_StatusAddr_eLinkModeAddr, ucSrcLinkMode, 1, true);
#if BHDM_MHL_CTS
			BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, MSC_REQ_PENDING, 1);
#else
			hMhl->hMscReq->hReq->bRequestPending = true;
#endif
			BHDM_P_Mhl_Host_EnableMhlTx_isr(hRegister, true);
			hMhl->ePathEnSentState = BHDM_P_Mhl_CbusPathEnSentState_eSent;
		}
		else
		{
			/* We wait for host to set TX_READY, but we must wake up the host first */
			hMhl->ePathEnSentState = BHDM_P_Mhl_CbusPathEnSentState_ePending;

			/* No need to check if FCB is valid as the default value of VBUS_NREQ is zero */
			if(!BHDM_P_MHL_HOST_GET_FIELD_ISR(hRegister, VBUS_NREQ) && !hMhl->bSinkPlimValid)
			{
				BDBG_MSG(("Waiting for sink's PLIM before waking host up"));
			}
		}
		/* This event will trigger waking up of HOST eventually, after we have read DCAP:DEV_CAT */
		hMhl->hMscReq->eEvent |= BHDM_P_MHL_MSCREQ_EVENT_SINK_PLIM;
	}
}

/* Handling disconnect */
static void BHDM_P_Mhl_HandleDisconnect_isr
	( BHDM_P_Mhl_Handle hMhl )
{
	BREG_Handle hRegister;

	BDBG_ASSERT(hMhl);

	hRegister = hMhl->hRegister;

	BHDM_P_Mhl_Host_EnableMhlTx_isr(hMhl->hRegister, false);
	BHDM_P_Mhl_DdcReq_CancelAllCmds_isr(hMhl->hDdcReq);
	BHDM_P_Mhl_MscReq_CancelAllCmds_isr(hMhl->hMscReq);

	hMhl->hMscResp->eEvent = BHDM_P_MHL_MSCRESP_EVENT_NONE;
	hMhl->hDdcReq->eEvent = BHDM_P_MHL_DDCREQ_EVENT_NONE;
	hMhl->hMscReq->eEvent = BHDM_P_MHL_MSCREQ_EVENT_NONE;

#if 0
	hMhl->stCbusState.eLastHpdState = BHDM_P_Mhl_CbusHpd_eDown;
#endif
	BHDM_P_Mhl_DdcReq_ClrHpdState_isr(hMhl->hDdcReq); /* Clear all EDID stuff */

	/* Clear FCB Rev */
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, REV, 0);

	/* Reset state variables */
	hMhl->hMscReq->eDcapReadState = BHDM_P_Mhl_DcapReadState_eStart;
	hMhl->hMscReq->eDcapSentState = BHDM_P_Mhl_DcapSentState_eRdyNotSent;
	hMhl->hDdcReq->eEdidReadState = BHDM_P_Mhl_EdidReadState_eStart;
	hMhl->hMscReq->hReq->bAbortActive = false;
	hMhl->hMscResp->bIbAbortActive = false;
	hMhl->hMscResp->bObAbortActive = false;
	hMhl->hDdcReq->hReq->bAbortActive = false;
	hMhl->hMscReq->hReq->bActive = false;
	hMhl->hMscResp->bActive = false;
	hMhl->hDdcReq->hReq->bActive = false;
	hMhl->hMscReq->eUnhandledEvent = 0;
	hMhl->hMscResp->eUnhandledEvent = 0;
	hMhl->hDdcReq->eUnhandledEvent = 0;
	BHDM_P_Mhl_ResetHeartBeat_isr(hMhl);
	hMhl->ucHeartbeatDeadPkts = 0;
	hMhl->ulHeartbeatCount = 0;
	hMhl->bHeartbeatQueued = false;
	hMhl->ulTickCount = BHDM_P_MHL_CBUS_CMD_SEND_TICK_INTERVAL;
	hMhl->pstHbPkts = &s_astDefaultHbPkts[0];
	hMhl->ulNumHbPkts = s_ulDefaultHbPktCnt;
	hMhl->ulNumHbFailed = 0;
}

/* Handle transmission completion
   Arguments: eResult, state, last completed command in a batch

   For successful transmissions:
   1. READ_DEVCAP, we will get a callback only after the last command,
      so we mark the DCAP as valid
   2. EDID read, generate the next EDID block read

   For failed transmissions:
   1. Update the current MSC/DDC error code
   2. If it is a READ_DEVCAP transmission,
      remove remaining READ_DEVCAP commands and
      start reading from first DCAP register.
   3. If it is a DDC EDID read transaction,
      remove remaining DDC commands and
      requeue the current block read.

*/
static void BHDM_P_Mhl_HandleXmitRetryExceeded_isr
	( BHDM_P_Mhl_Handle hMhl )
{
	BREG_Handle hRegister;

	BDBG_ASSERT(hMhl);

	hRegister = hMhl->hRegister;

	hMhl->ucHeartbeatDeadPkts++;

	BDBG_ERR(("Dead packet #%d", hMhl->ucHeartbeatDeadPkts));

	if(hMhl->ucHeartbeatDeadPkts >= BHDM_P_MHL_CBUS_HEARTBEAT_DEAD_PKTS)
	{
		/* as disconnect is the highest priority event, overwrite all the wakeup discovery task events with disconnect and let the wakeup_discovery task deal with it */
		BDBG_MSG(("CBUS link dead!"));
		BHDM_P_Mhl_HandleDisconnect_isr(hMhl);
		BHDM_P_Mhl_Cbus_Disconnect_isr(hRegister);
	}
}

static void BHDM_P_Mhl_EnableMpmHostInterrupts_isr
	( const BHDM_P_Mhl_Handle hMhl )
{
	BERR_Code rc;
	uint8_t i;
	const BHDM_P_Mhl_InterruptCbTable *pMpmHostInterrupts;

	pMpmHostInterrupts = BHDM_P_Mhl_MpmHostInterrupts;

#if 0
	/* TODO: Check why this is not done by bcmdriver k */
	BREG_Write32(hMhl->hRegister, BCHP_MPM_HOST_L2_CPU_MASK_CLEAR, 0xffff);
#endif

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_MHL_MPM_HOST_INTR_ENUM(LAST); i++ )
	{
		/* now enable it; if specified for startup */
		if (!pMpmHostInterrupts[i].enable)
			continue;

		rc = BINT_EnableCallback_isr(hMhl->hMpmHostCallback[i]);
		rc = BERR_TRACE(rc);
	}
}

void BHDM_P_Mhl_EnableMpmHostInterrupts
	( const BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_EnableMpmHostInterrupts_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

static void BHDM_P_Mhl_EnableCbus0Interrupts_isr
	( const BHDM_P_Mhl_Handle hMhl )
{
	BERR_Code rc;
	uint8_t i;
	const BHDM_P_Mhl_InterruptCbTable *pCbus0Interrupts;

	pCbus0Interrupts = BHDM_P_Mhl_Cbus0Interrupts;

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_MHL_CBUS_INTR_0_ENUM(LAST); i++ )
	{
		/* now enable it; if specified for startup */
		if (!pCbus0Interrupts[i].enable)
			continue;

		rc = BINT_EnableCallback_isr(hMhl->hCbus0Callback[i]);
		rc = BERR_TRACE(rc);
	}
}

void BHDM_P_Mhl_EnableCbus0Interrupts
	( const BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_EnableCbus0Interrupts_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

static void BHDM_P_Mhl_EnableCbus1Interrupts_isr
	( const BHDM_P_Mhl_Handle hMhl )
{
	BERR_Code rc;
	uint8_t i;
	const BHDM_P_Mhl_InterruptCbTable *pCbus1Interrupts;

	pCbus1Interrupts = BHDM_P_Mhl_Cbus1Interrupts;

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_MHL_CBUS_INTR_1_ENUM(LAST); i++ )
	{
		/* now enable it; if specified for startup */
		if (!pCbus1Interrupts[i].enable)
			continue;

		rc = BINT_EnableCallback_isr(hMhl->hCbus1Callback[i]);
		rc = BERR_TRACE(rc);
	}
}

void BHDM_P_Mhl_EnableCbus1Interrupts
	( const BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_EnableCbus1Interrupts_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

static void BHDM_P_Mhl_DisableMpmHostInterrupts_isr
	( const BHDM_P_Mhl_Handle hMhl )
{
	BERR_Code rc;
	uint8_t i;
	const BHDM_P_Mhl_InterruptCbTable *pMpmHostInterrupts;

	pMpmHostInterrupts = BHDM_P_Mhl_MpmHostInterrupts;

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_MHL_MPM_HOST_INTR_ENUM(LAST); i++ )
	{
		/* now enable it; if specified for startup */
		if (!pMpmHostInterrupts[i].enable)
			continue;

		rc = BINT_DisableCallback_isr(hMhl->hMpmHostCallback[i]);
		rc = BERR_TRACE(rc);
	}
}

void BHDM_P_Mhl_DisableMpmHostInterrupts
	( const BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_DisableMpmHostInterrupts_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

static void BHDM_P_Mhl_DisableCbus0Interrupts_isr
	( const BHDM_P_Mhl_Handle hMhl )
{
	BERR_Code rc;
	uint8_t i;
	const BHDM_P_Mhl_InterruptCbTable *pCbus0Interrupts;

	pCbus0Interrupts = BHDM_P_Mhl_Cbus0Interrupts;

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_MHL_CBUS_INTR_0_ENUM(LAST); i++ )
	{
		/* now enable it; if specified for startup */
		if (!pCbus0Interrupts[i].enable)
			continue;

		rc = BINT_DisableCallback_isr(hMhl->hCbus0Callback[i]);
		rc = BERR_TRACE(rc);
	}
}

void BHDM_P_Mhl_DisableCbus0Interrupts
	( const BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_DisableCbus0Interrupts_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

static void BHDM_P_Mhl_DisableCbus1Interrupts_isr
	( const BHDM_P_Mhl_Handle hMhl )
{
	BERR_Code rc;
	uint8_t i;
	const BHDM_P_Mhl_InterruptCbTable *pCbus1Interrupts;

	pCbus1Interrupts = BHDM_P_Mhl_Cbus1Interrupts;

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_MHL_CBUS_INTR_1_ENUM(LAST); i++ )
	{
		/* now enable it; if specified for startup */
		if (!pCbus1Interrupts[i].enable)
			continue;

		rc = BINT_DisableCallback_isr(hMhl->hCbus1Callback[i]);
		rc = BERR_TRACE(rc);
	}
}

void BHDM_P_Mhl_DisableCbus1Interrupts
	( const BHDM_P_Mhl_Handle hMhl )
{
	BKNI_EnterCriticalSection();
	BHDM_P_Mhl_DisableCbus1Interrupts_isr(hMhl);
	BKNI_LeaveCriticalSection();
}

static void BHDM_P_Mhl_EnableMpmUart
	(BREG_Handle hRegister)
{
	uint32_t ulData;
	/*
		MPM UART takes control of AON_GPIO_12 (RX) and AON_GPIO_13 (TX)
		These setting are needed only if STRAP_MPM_POWERUP=0.
	*/
	ulData = BREG_Read32(hRegister, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
	ulData &= ~(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_12_MASK |
				BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_13_MASK);
	ulData |= (BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 7) |
				BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 7));
	BREG_Write32(hRegister, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, ulData);
}

/******************************************************************************
void BHDM_P_Mhl_HandleCbus0Interrupt_isr
Summary: Handle interrupts from the CBUS_INTR2_0
*******************************************************************************/
void BHDM_P_Mhl_HandleCbus0Interrupt_isr
	( void *pParam1,						/* [in] Device handle */
	  int   parm2 )							/* [in] MHL interrupt */
{
	BHDM_P_Mhl_Handle hMhl;
	BREG_Handle hRegister;

	hMhl = (BHDM_P_Mhl_Handle)pParam1;
	hRegister = hMhl->hRegister;

	hMhl->stCbusIntr.ulStatus0 = 0;

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		sprintf(acTemp, "%d", parm2);
		strcat(acStr, "Received CBUS_INTR2_0 interrupt ");
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#endif

	switch (parm2)
	{
	/* Disconnect has highest priority */
	case MAKE_MHL_CBUS_INTR_0_ENUM(DISCONNECT_RECEIVED):
		/* Disconnect occurs when
			1. After reconnecting MHL cable during hotplug.
			2. Sink sends a disconnect to src
		*/
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("CBUS disconnect received");
#else
		BDBG_MSG(("CBUS DISCONNECT received"));
#endif
		hMhl->bDisconnectReceived = true;

		BHDM_P_Mhl_GetChanStatus_isr(hMhl);

		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, DISCONNECT_RECEIVED, 1);

		BHDM_P_Mhl_HandleDisconnect_isr(hMhl);

		BKNI_SetEvent_isr(hMhl->BHDM_MHL_EventStandby) ;

		break ;

	case MAKE_MHL_CBUS_INTR_0_ENUM(DISCONNECT_DONE):
		/* Not handled by Host SW. */
		BDBG_MSG(("Received DISCONNECT_DONE interrupt but host SW does not handle this."));
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, DISCONNECT_DONE, 1);
		break ;


	case MAKE_MHL_CBUS_INTR_0_ENUM(DISCOVERY_SUCCEEDED):
		BDBG_MSG(("Received DISCOVERY_SUCCEEDED interrupt but host SW does not handle this."));
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, DISCOVERY_SUCCEEDED, 1);
		break ;

	case MAKE_MHL_CBUS_INTR_0_ENUM(IMP_CHANGED):
#if 1
		BDBG_MSG(("Received IMP_CHANGED interrupt but host SW does not handle this."));
#else
		{
			/* Verify if low impedance */

			uint32_t ulData;
			uint32_t ulStatus, ulDiff;

			ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_RCAL_CBUS_PHY_STATUS);
			ulStatus = BCHP_GET_FIELD_DATA(ulData, DVP_MT_AON_TOP_RCAL_CBUS_PHY_STATUS, CBUS_IMP_CHANGE);

			/* Wait at least 80ms for the Sink's discover resistance to settle down as required by
			   the MHL Spec: T_SRC_START_IMP_MEAS: */

			if (ulStatus != 0)
			{
				BTMR_ReadTimer_isr(hMhl->hMhlTimer, (unsigned *)&ulDiff);

				do
				{
					/* Wait for a minimum of 50 msecs before doing wakeup and discovery */
					BTMR_ReadTimer_isr(hMhl->hMhlTimer, (unsigned *)&ulStatus);
				} while ((ulStatus-ulDiff) <= 100*100); /* msecs */
			}
			else
			{
				BDBG_WRN(("IMP_CHANGED interrupt occurred but CBUS_PHY_STATUS shows otherwise."));
			}

			BHDM_P_Mhl_HandleWakeupAndDiscovery_isr(hMhl);
		}
#endif
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, IMP_CHANGED, 1);
		break;

	case MAKE_MHL_CBUS_INTR_0_ENUM(DISCOVERY_FAILED):
#if 1
		BDBG_MSG(("Received DISCOVERY_FAILED interrupt but host SW does not handle this."));
#else
		BDBG_MSG(("CBUS discovery failed."));
		BHDM_P_Mhl_HandleWakeupAndDiscovery_isr(hMhl);
#endif
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, DISCOVERY_FAILED, 1);
		break ;

	/* These are ignored */
	case MAKE_MHL_CBUS_INTR_0_ENUM(INITIATOR_PKT):
		BDBG_MSG(("Initiator packet interrupt is ignored."));
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, INITIATOR_PKT, 1);
		break;
	case MAKE_MHL_CBUS_INTR_0_ENUM(FOLLOWER_PKT):
		BDBG_MSG(("Follower packet interrupt is ignored."));
		hMhl->iHeartbeatTimer = BHDM_P_MHL_CBUS_HEARTBEAT_TICKS;
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, FOLLOWER_PKT, 1);
		break;
	case MAKE_MHL_CBUS_INTR_0_ENUM(FOLLOWER_DROP_PKT):
		BDBG_MSG(("Follower dropped packet is ignored."));
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, FOLLOWER_DROP_PKT, 1);
		break;
	case MAKE_MHL_CBUS_INTR_0_ENUM(SCHEDULER_DROP_PKT):
		BDBG_MSG(("Scheduler dropped packet is ignored."));
		hMhl->stCbusIntr.ulStatus0 = BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_STATUS, SCHEDULER_DROP_PKT, 1);
		break;
	}

	/* Clear interrupt */
	BHDM_P_Mhl_Cbus_ClearInt_isr(hRegister, &hMhl->stCbusIntr);

}

/******************************************************************************
void BHDM_P_Mhl_HandleCbusInterrupt_isr
Summary: Handle interrupts from the CBUS_INTR@_1.
*******************************************************************************/
void BHDM_P_Mhl_HandleCbus1Interrupt_isr
	( void *pParam1,						/* [in] Device handle */
	  int   parm2 )							/* [in] MHL interrupt */
{
	BHDM_P_Mhl_Handle hMhl;
	BREG_Handle hRegister;
	bool bMscReqIntErr = false, bMscRespIntErr = false, bDdcReqIntErr = false;

	hMhl = (BHDM_P_Mhl_Handle)pParam1;
	hRegister = hMhl->hRegister;

	hMhl->stCbusIntr.ulStatus1 = 0;

#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[40] = "";
		char acTemp[10] = "";
		sprintf(acTemp, "%d", parm2);
		strcat(acStr, "Received CBUS INTR2 1 interrupt ");
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#endif

	switch (parm2)
	{
	case MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_MSC_RECEIVED):
		BDBG_MSG(("Received MSC ABORT."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, ABORT_MSC_RECEIVED, 1);

		BHDM_P_Mhl_MscReq_HandleAbortMscReceived_isr(hMhl->hMscReq);
		break ;

	case MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_DDC_RECEIVED):
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Received DDC ABORT\n");
#else
		BDBG_MSG(("Received DDC ABORT"));
#endif
		BHDM_P_Mhl_DdcReq_HandleAbortDdcReceived_isr(hMhl->hDdcReq);
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, ABORT_DDC_RECEIVED, 1);

		/* Assert MSC REQ event to read DDC error code */
		hMhl->hMscReq->eEvent |= BHDM_P_MHL_MSCREQ_EVENT_READ_DDC_ERROR_CODE;
		break ;

	case MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_MSC_INBOUND_TIMEOUT_DONE):
		BDBG_MSG(("MSC IB ABORT timer expired"));

		BHDM_P_Mhl_GetChanStatus_isr(hMhl);
		BHDM_P_Mhl_MscResp_ClearAbort_isr(hMhl->hMscResp);

		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, ABORT_MSC_INBOUND_TIMEOUT_DONE, 1);

		hMhl->hMscReq->hReq->bAbortActive = false;
		hMhl->hMscResp->bIbAbortActive = false;
		break ;

	case MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_MSC_OUTBOUND_TIMEOUT_DONE):
		BDBG_MSG(("MSC OB ABORT timer expired"));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, ABORT_MSC_OUTBOUND_TIMEOUT_DONE, 1);
		BHDM_P_Mhl_HandleAbortMscObTimeoutDone_isr(hMhl);
		break ;

	case MAKE_MHL_CBUS_INTR_1_ENUM(ABORT_DDC_TIMEOUT_DONE):
		BDBG_MSG(("DDC ABORT timer expired"));
		BHDM_P_Mhl_DdcReq_HandleAbortDdcTimeoutDone_isr(hMhl->hDdcReq, &hMhl->stCbusState.eLastHpdState);
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, ABORT_DDC_TIMEOUT_DONE, 1);
		break ;

	/* MSC Requester interrupts */
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_DONE):
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("MSC Requestor command done.\n");
#else
		BDBG_MSG(("MSC Requestor command done."));
#endif
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_DONE, 1);
		BHDM_P_Mhl_MscReq_HandleDone_isr(hMhl->hMscReq, &hMhl->bSinkPlimValid,
									&hMhl->bHeartbeatQueued, &hMhl->ucHeartbeatDeadPkts);
		break;

	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_MISMATCH):
		BDBG_MSG(("MSC Requestor command mismatch."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_RX_MISMATCH, 1);
		bMscReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_CMD_ERROR):
		BDBG_MSG(("MSC Requestor received command error."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_RX_CMD_ERROR, 1);
		bMscReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_RX_TIMEOUT):
		BDBG_MSG(("MSC Requester received time out."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_RX_TIMEOUT, 1);
		bMscReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_ILLEGAL_SW_WR):
		BDBG_MSG(("MSC Requestor illegal SW write."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_ILLEGAL_SW_WR, 1);
		bMscReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_MAX_RETRIES_EXCEEDED):
		BDBG_MSG(("MSC Requestor exceeded maximum number of retries."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_MAX_RETRIES_EXCEEDED, 1);
		bMscReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_UNEXPECTED_INBOUND_PKT):
		BDBG_MSG(("MSC Requestor unexpected IB packet."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_UNEXPECTED_INBOUND_PKT, 1);
		bMscReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_BUS_STOLEN):
		BDBG_MSG(("MSC Requestor bus stolen."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_BUS_STOLEN, 1);
		bMscReqIntErr = true;
		break ;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_REQ_CANCELLED):
		BDBG_MSG(("MSC Requestor command cancelled."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_REQ_CANCELLED, 1);
		bMscReqIntErr = true;
		break ;

	/* MSC Responder interrupts */
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_IB_DONE):
		BDBG_MSG(("MSC Responder IB command done."));

		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_IB_DONE, 1);

		BHDM_P_Mhl_MscResp_HandleIbDone_isr(hMhl->hMscResp, &hMhl->stCbusState, &hMhl->hMscReq->eEvent,
											&hMhl->hDdcReq->eEvent);
		break;

	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_OB_DONE):
		BDBG_MSG(("MSC Responder OB command done."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_OB_DONE, 1);

		BHDM_P_Mhl_MscResp_HandleObDone_isr(hMhl->hMscResp);
		break;

	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_BAD_CMD):
		BDBG_MSG(("MSC Responder bad command."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_BAD_CMD, 1);
		bMscRespIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_UNEXPECTED_CMD):
		BDBG_MSG(("MSC Responder unexpected command."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_UNEXPECTED_CMD, 1);
		bMscRespIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_DATA_RECEIVED): /* ignored */
		BDBG_MSG(("MSC Responder received data."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_DATA_RECEIVED, 1);
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_DATA_OVERFLOW):
		BDBG_MSG(("MSC Responder data overflow."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_DATA_OVERFLOW, 1);
		bMscRespIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_RX_TIMEOUT):
		BDBG_MSG(("MSC Responder received time out."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_RX_TIMEOUT, 1);
		bMscRespIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_SW_TIMEOUT):
		BDBG_MSG(("MSC Responder SW timeout."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_SW_TIMEOUT, 1);
		bMscRespIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_ILLEGAL_SW_WR):
		BDBG_MSG(("MSC Responder illegal SW write."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_ILLEGAL_SW_WR, 1);
		bMscRespIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_MAX_RETRIES_EXCEEDED):
		BDBG_MSG(("MSC Responder exceeded maximum number of retries."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_MAX_RETRIES_EXCEEDED, 1);
		bMscRespIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(MSC_RESP_UNEXPECTED_INBOUND_PKT):
		BDBG_MSG(("MSC Responder unexpected IB packet."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, MSC_RESP_UNEXPECTED_INBOUND_PKT, 1);
		bMscRespIntErr = true;
		break;

	/* DDC error interrupts */
	case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_UNEXPECTED_INBOUND_PKT):
		BDBG_ERR(("DDC Requestor unexpected IB packet."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, DDC_REQ_UNEXPECTED_INBOUND_PKT, 1);
		bDdcReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_MAX_RETRIES_EXCEEDED):
		BDBG_ERR(("DDC Requestor exceeded maximum number of retries."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, DDC_REQ_MAX_RETRIES_EXCEEDED, 1);
		bDdcReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_ILLEGAL_SW_WR):
		BDBG_ERR(("DDC Requestor illegal SW write."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, DDC_REQ_ILLEGAL_SW_WR, 1);
		bDdcReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_TIMEOUT):
		BDBG_ERR(("DDC Requestor received time out."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, DDC_REQ_RX_TIMEOUT, 1);
		bDdcReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_CMD_ERROR):
		BDBG_ERR(("DDC Requestor received command error."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, DDC_REQ_RX_CMD_ERROR, 1);
		bDdcReqIntErr = true;
		break;
	case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_RX_MISMATCH):
		BDBG_ERR(("DDC Requestor command mismatch."));
		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, DDC_REQ_RX_MISMATCH, 1);
		bDdcReqIntErr = true;
		break;

	case MAKE_MHL_CBUS_INTR_1_ENUM(DDC_REQ_DONE):
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("DDC Requestor command done.\n");
#else
		BDBG_MSG(("DDC Requestor command done."));
#endif

		hMhl->stCbusIntr.ulStatus1 = BCHP_FIELD_DATA(CBUS_INTR2_1_CPU_STATUS, DDC_REQ_DONE, 1);
		BHDM_P_Mhl_DdcReq_HandleDone_isr(hMhl->hDdcReq);
		hMhl->ucHeartbeatDeadPkts = 0; /*  as we have completed some command means Sink is responding, so reset dead packet count*/
		break;
	}

	/* Clear interrupt */
	BHDM_P_Mhl_Cbus_ClearInt_isr(hRegister, &hMhl->stCbusIntr);

	if (bMscReqIntErr == true)
	{
		BERR_Code err;

		err = BHDM_P_Mhl_MscReq_HandleErrors_isr(hMhl->hMscReq, parm2, &hMhl->bStopPending,
												&hMhl->bHeartbeatQueued, &hMhl->stCbusState);
		if (err == BHDM_P_MHL_CBUS_XMIT_RETRY_EXCEEDED)
		{
			BHDM_P_Mhl_HandleXmitRetryExceeded_isr(hMhl);
		}
	}

	if (bMscRespIntErr == true)
	{
		BHDM_P_Mhl_MscResp_HandleErrors_isr(hMhl->hMscResp, parm2, &hMhl->hMscReq->eEvent);
	}

	if (bDdcReqIntErr == true)
	{
		BERR_Code err;

		err = BHDM_P_Mhl_DdcReq_HandleErrors_isr(hMhl->hDdcReq, parm2, &hMhl->bStopPending, &hMhl->hMscReq->eEvent);
		if (err == BHDM_P_MHL_CBUS_XMIT_RETRY_EXCEEDED)
		{
			BHDM_P_Mhl_HandleXmitRetryExceeded_isr(hMhl);
		}
	}

	/* Accumulate asserted events  */
	if (hMhl->hMscResp->eUnhandledEvent != 0)
	{
		hMhl->hMscResp->eEvent |= hMhl->hMscResp->eUnhandledEvent;
	}

	if (hMhl->hMscReq->eUnhandledEvent != 0)
	{
		hMhl->hMscReq->eEvent |= hMhl->hMscReq->eUnhandledEvent;
	}

	if (hMhl->hDdcReq->eUnhandledEvent != 0)
	{
		hMhl->hDdcReq->eEvent |= hMhl->hDdcReq->eUnhandledEvent;
	}


	/************** Handle Events **************/
	if (hMhl->hMscReq->eEvent != 0)
	{
		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_RECEIVED_DCAP_RDY)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_RECEIVED_DCAP_RDY;
			BHDM_P_Mhl_MscReq_HandleReceivedDcapRdy_isr(hMhl->hMscReq, &hMhl->bSinkPlimValid, &hMhl->stCbusState);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_RECEIVED_LINK_MODE)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_RECEIVED_LINK_MODE;
			BHDM_P_Mhl_HandleReceivedLinkMode_isr(hMhl);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_DCAP_CHG_INT)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_DCAP_CHG_INT;
			BHDM_P_Mhl_MscReq_HandleDcapChgInt_isr(hMhl->hMscReq, &hMhl->bSinkPlimValid,
					&hMhl->hMscResp->bObAbortActive, &hMhl->stCbusState);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_REQ_WRT_INT)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_REQ_WRT_INT;
			BHDM_P_Mhl_MscReq_HandleReqWrtInt_isr(hMhl->hMscReq);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_HEARTBEAT)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_HEARTBEAT;
			BHDM_P_Mhl_HandleHeartbeat_isr(hMhl);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_MSC_TX_ABORT)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_MSC_TX_ABORT;
			BHDM_P_Mhl_MscReq_HandleMscTxAbort_isr(hMhl->hMscReq);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_READ_DDC_ERROR_CODE)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_READ_DDC_ERROR_CODE;
			BHDM_P_Mhl_MscReq_HandleReadDdcErrorCode_isr(hMhl->hMscReq);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_SEND_RCPE_ERR_BUSY)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_SEND_RCPE_ERR_BUSY;
			BHDM_P_Mhl_MscReq_HandleSendRcpeErrBusy_isr(hMhl->hMscReq);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_NONE)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_NONE;
			BHDM_P_Mhl_MscReq_HandleSendRapkErrNone_isr(hMhl->hMscReq);
		}

		if (hMhl->hMscReq->eEvent & BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_BUSY)
		{
			hMhl->hMscReq->eEvent &= ~BHDM_P_MHL_MSCREQ_EVENT_SEND_RAPK_ERR_BUSY;
			BHDM_P_Mhl_MscReq_HandleSendRapkErrBusy_isr(hMhl->hMscReq);
		}
	}

	/* If nothing is going on in the MSC Requester, send the next command from MSC REQ Queue */
	BHDM_P_Mhl_Req_Active_isr(hMhl->hMscReq->hReq, &hMhl->hMscReq->hReq->bActive, &hMhl->hMscReq->hReq->bAbortActive);
	if(!hMhl->hMscReq->hReq->bAbortActive && !hMhl->hMscReq->hReq->bActive)
	{
		BDBG_MSG(("Send MSC Command"));
		BHDM_P_Mhl_Req_SendNextCmd_isr(hMhl->hMscReq->hReq,
										&hMhl->hMscReq->hReq->stLastCmd,
										&hMhl->hMscReq->hReq->bRetryLastCmd);
	}

	if (hMhl->hDdcReq->eEvent != 0)
	{
		if (hMhl->hDdcReq->eEvent & BHDM_P_MHL_DDCREQ_EVENT_SET_HPD)
		{
			hMhl->hDdcReq->eUnhandledEvent &= ~BHDM_P_MHL_DDCREQ_EVENT_SET_HPD;
			BHDM_P_Mhl_DdcReq_HandleSetHpd_isr(hMhl->hDdcReq,
				&hMhl->stCbusState.eLastHpdState, hMhl->bStopPending);
		}

		if (hMhl->hDdcReq->eEvent & BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT)
		{
			hMhl->hDdcReq->eUnhandledEvent &= ~BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT;
			BHDM_P_Mhl_DdcReq_HandleEdidChgInt_isr(hMhl->hDdcReq, &hMhl->hDdcReq->eUnhandledEvent);
		}

		if (hMhl->hDdcReq->eEvent & BHDM_P_MHL_DDCREQ_EVENT_CLR_HPD)
		{
			hMhl->hDdcReq->eEvent &= ~BHDM_P_MHL_DDCREQ_EVENT_CLR_HPD;
			BHDM_P_Mhl_DdcReq_HandleClrHpd_isr(hMhl->hDdcReq, &hMhl->stCbusState);

			if (hMhl->pfHotplugChangeCallback)
			{
				hMhl->pfHotplugChangeCallback(hMhl->pvHotplugChangeParm1,
											  hMhl->iHotplugChangeParm2,
											  (void *)&hMhl->stCbusState.eLastHpdState);
			}
		}

		if (hMhl->hDdcReq->eEvent & BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT)
		{
			hMhl->hDdcReq->eEvent &= ~BHDM_P_MHL_DDCREQ_EVENT_EDID_CHG_INT;
			BHDM_P_Mhl_DdcReq_HandleEdidChgInt_isr(hMhl->hDdcReq, &hMhl->hDdcReq->eUnhandledEvent);
		}

		if ((hMhl->hDdcReq->eEvent & BHDM_P_MHL_DDCREQ_EVENT_EDID_BLOCK_READ) &&
			!(hMhl->hDdcReq->eEvent & BHDM_P_MHL_DDCREQ_EVENT_CLR_HPD))
		{
			hMhl->hDdcReq->eEvent &= ~(BHDM_P_MHL_DDCREQ_EVENT_EDID_BLOCK_READ | BHDM_P_MHL_DDCREQ_EVENT_CLR_HPD);
			BHDM_P_Mhl_DdcReq_HandleEdidReadBlock_isr(hMhl->hDdcReq, &hMhl->hDdcReq->eUnhandledEvent);

			if (hMhl->hDdcReq->eEdidReadState == BHDM_P_Mhl_EdidReadState_eEnd && hMhl->pfHotplugChangeCallback)
			{
				hMhl->pfHotplugChangeCallback(hMhl->pvHotplugChangeParm1,
											  hMhl->iHotplugChangeParm2,
											  (void *)&hMhl->stCbusState.eLastHpdState);
			}
		}
	}

	if (hMhl->hDdcReq->eHdcpEvent != 0)
	{

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_VERSION)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_VERSION;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpVersionValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_RX_BCAPS)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_RX_BCAPS;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpRxBcapsValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_RX_STATUS)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_RX_STATUS;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpRxStatusValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_KSV)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_KSV;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpKsvValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_RX_KSV_LIST)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_RX_KSV_LIST;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpRepeaterKsvListValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_RX_PJ)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_RX_PJ;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpRxPjValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_RX_RI)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_RX_RI;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpRxRiValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_TX_AKSV)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_TX_AKSV;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpTxAksvValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_RX_BKSV)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_RX_BKSV;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpRxBksvValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_AN)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_AN;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpAnValueEvent);
		}

		if (hMhl->hDdcReq->eHdcpEvent & BHDM_P_MHL_HDCP_EVENT_AINFO)
		{
			hMhl->hDdcReq->eHdcpEvent &= ~BHDM_P_MHL_HDCP_EVENT_AINFO;
			BKNI_SetEvent(hMhl->hDdcReq->hHdcpAinfoByteValueEvent);
		}
	}

	/* If nothing is going on in the DDC Requester, send the next command from DDC REQ Queue */
	BHDM_P_Mhl_Req_Active_isr(hMhl->hDdcReq->hReq, &hMhl->hDdcReq->hReq->bActive, &hMhl->hDdcReq->hReq->bAbortActive);
	if(!hMhl->hDdcReq->hReq->bAbortActive && !hMhl->hDdcReq->hReq->bActive)
	{
#if BHDM_MHL_ENABLE_DEBUG
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Send DDC Command\n");
#else
		BDBG_MSG(("Send DDC Command"));
#endif

		BHDM_P_Mhl_Req_SendNextCmd_isr(hMhl->hDdcReq->hReq, &hMhl->hDdcReq->hReq->stLastCmd,
										&hMhl->hDdcReq->hReq->bRetryLastCmd);
	}

#if BHDM_P_MHL_CBUS_HEARTBEAT
		/* If there is nothing to send or the queue
		   is empty, decrement the heartbeat timer */
		if(hMhl->iHeartbeatTimer > 0)
		{
			hMhl->iHeartbeatTimer--;
		}

		if(hMhl->iHeartbeatTimer == 0)
		{
			/* This stops the heartbeart timer which will
			   get reset when the heartbeat command has been
			   sent successfully or we failed to queue the
			   command */
			hMhl->iHeartbeatTimer--;

			BDBG_MSG(("Heartbeat #0x%x  timer expired", hMhl->ulHeartbeatCount));

			/* If the current heartbeat has not
			   been sent, don't queue another one.
			   We next restart the heartbeat timer
			   if the current heartbeat has successfully
			   been sent */
			if(hMhl->bHeartbeatQueued)
			{
				BDBG_MSG(("Current heartbeat has not been sent yet!"));
			}
			else
			{
				hMhl->ulHeartbeatCount++;
				hMhl->ulEvents |= BHDM_P_Mhl_CbusEvent_eHeartbeat;
			}
		}
#endif

}

/******************************************************************************
void BHDM_P_Mhl_HandleMpmHostInterrupt_isr
Summary: Handle interrupts from the MPM host .
*******************************************************************************/
void BHDM_P_Mhl_HandleMpmHostInterrupt_isr
	( void *pParam1,						/* [in] Device handle */
	  int   parm2 )							/* [in] MHL interrupt */
{
	BHDM_P_Mhl_Handle hMhl;
	BHDM_Handle hHdmi;
	BREG_Handle hRegister;

	hMhl = (BHDM_P_Mhl_Handle)pParam1;
	hRegister = hMhl->hRegister;
	hHdmi = hMhl->hHdmi;

	hMhl->stMpmHostIntr.ulStatus = 0;

	switch (parm2)
	{

	case  MAKE_MHL_MPM_HOST_INTR_ENUM(S3_TO_S0):
		BDBG_MSG(("Received S3->S0 interrupt"));

		hMhl->stMpmHostIntr.ulStatus = BCHP_FIELD_DATA(MPM_HOST_L2_CPU_STATUS, S3_TO_S0, 1);
		BHDM_P_Mhl_Host_S3ToS0Handover_isr(hRegister);

		BHDM_P_Mhl_EnableCbus0Interrupts_isr(hMhl);
		BHDM_P_Mhl_EnableCbus1Interrupts_isr(hMhl);

		hMhl->hostState = BHDM_P_Mhl_HostCpuState_eActive;

		/* Now wait for HPD message from sink. Handled by MSC_RESP_IB_DONE interrupt. */

		break;
	case  MAKE_MHL_MPM_HOST_INTR_ENUM(S0_TO_S3):
		BDBG_MSG(("Received S0->S3 interrupt"));

		hMhl->stMpmHostIntr.ulStatus = BCHP_FIELD_DATA(MPM_HOST_L2_CPU_STATUS, S0_TO_S3, 1);

		BHDM_P_Mhl_DisableMpmHostInterrupts_isr(hMhl);
		BHDM_P_Mhl_DisableCbus0Interrupts_isr(hMhl);
		BHDM_P_Mhl_DisableCbus1Interrupts_isr(hMhl);

		if (hHdmi->bMhlMode)
		{
			/* Put the host into standby */
			BKNI_SetEvent_isr(hMhl->hEventStandby) ;
		}

		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(A2R_BAD_SIZE_INTR):
		BDBG_MSG(("Received A2R_BAD_SIZE interrupt"));
		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(A2R_TIMEOUT_INTR):
		BDBG_MSG(("Received A2R_TIMEOUT interrupt"));
		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(MSPI_INTR_0):
		BDBG_MSG(("Received MSPI_INTR_0 interrupt"));
		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(MSPI_INTR_1):
		BDBG_MSG(("Received MSPI_INTR_1 interrupt"));
		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE3):
		BDBG_MSG(("Received MPM_INTR_SPARE3 interrupt"));
		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE2):
		BDBG_MSG(("Received MPM_INTR_SPARE2 interrupt"));
		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE1):
		BDBG_MSG(("Received MPM_INTR_SPARE1 interrupt"));
		break;

	case MAKE_MHL_MPM_HOST_INTR_ENUM(MPM_INTR_SPARE0):
		BDBG_MSG(("Received MPM_INTR_SPARE0 interrupt"));
		break;

#if BHDM_P_MHL_ENABLE_MPM_UARTS
	case MAKE_MHL_MPM_HOST_INTR_ENUM(HOST_CLEARED_INTR):
		BDBG_MSG(("Received HOST_CLEARED interrupt"));
		break;
	case MAKE_MHL_MPM_HOST_INTR_ENUM(WDOG_INTR):
		BDBG_MSG(("Received WDOG interrupt"));
		break;
	case MAKE_MHL_MPM_HOST_INTR_ENUM(UART_INTR):
	case MAKE_MHL_MPM_HOST_INTR_ENUM(UART_ERR_INTR):
	case MAKE_MHL_MPM_HOST_INTR_ENUM(UART_RT_INTR):
	case MAKE_MHL_MPM_HOST_INTR_ENUM(UART_TX_INTR):
	case MAKE_MHL_MPM_HOST_INTR_ENUM(UART_RX_INTR):
	case MAKE_MHL_MPM_HOST_INTR_ENUM(UART_MS_INTR):
		BDBG_MSG(("Received MPM_UART %d interrupt", parm2));
		break;
#endif
	}

}
/******************************************************************************
BERR_Code BHDM_P_Mhl_Create
Summary: Create the MHL device
*******************************************************************************/
BERR_Code BHDM_P_Mhl_Create
	( BHDM_P_Mhl_Handle          *phMhl,
	  BCHP_Handle                 hChip,
	  BREG_Handle                 hRegister,
	  BINT_Handle                 hInterrupt,
	  BTMR_Handle                 hTimer,
	  const BHDM_P_Mhl_Settings  *pSettings )
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t i;
	BHDM_P_Mhl_Object *pMhl = NULL;
	const BHDM_P_Mhl_InterruptCbTable *pCbus0Interrupts;
	const BHDM_P_Mhl_InterruptCbTable *pCbus1Interrupts;
	const BHDM_P_Mhl_InterruptCbTable *pMpmHostInterrupts;

	BDBG_ENTER(BHDM_P_Mhl_Create);

	/* verify parameters */
	BDBG_ASSERT(hChip);
	BDBG_ASSERT(hRegister);
	BDBG_ASSERT(hInterrupt);
	BDBG_ASSERT(hTimer);
	BSTD_UNUSED(pSettings);

#if BHDM_MHL_ENABLE_DEBUG
	s_pfCbusLog = fopen("cbus_pkts.log", "w+");
	if (s_pfCbusLog == NULL)
	{
		BDBG_ERR(("Failed to open CBUS debug log file."));
		return BERR_OS_ERROR;
	}
#endif

	/* create the MHL Handle */
	pMhl = (BHDM_P_Mhl_Object *)BKNI_Malloc(sizeof(BHDM_P_Mhl_Object));
	if (pMhl == NULL)
	{
		BDBG_ERR(("Unable to allocate memory for HDMI Mhl object"));
		rc = BERR_OUT_OF_SYSTEM_MEMORY;
		goto done;
	}

	/* zero out all memory associated with the HDMI Device Handle before using */
	BKNI_Memset((void *)pMhl, 0, sizeof(BHDM_P_Mhl_Object));
	BDBG_OBJECT_SET(pMhl, HDMI_MHL);

	/* assign the handles passed in as parameters */
	pMhl->hChip      = hChip;
	pMhl->hRegister  = hRegister;
	pMhl->hInterrupt = hInterrupt;
	pMhl->hTmr       = hTimer;

	pCbus0Interrupts = BHDM_P_Mhl_Cbus0Interrupts;
	pCbus1Interrupts = BHDM_P_Mhl_Cbus1Interrupts;
	pMpmHostInterrupts = BHDM_P_Mhl_MpmHostInterrupts;

	BHDM_MHL_CHECK_RC(rc, BKNI_CreateEvent(&(pMhl->hEventStandby))) ;

	/* TODO: Power Management */

	/* Register CBUS interrupt callbacks. Defer enabling of CBUS interrupts during S3-to-S0. */
	for( i = 0; i < MAKE_MHL_CBUS_INTR_0_ENUM(LAST); i++ )
	{
		/*
		** DEBUG
		** Create ALL interrupt callbacks
		** enable debug callbacks as needed;
		*/

		BHDM_MHL_CHECK_RC( rc, BINT_CreateCallback(
			&(pMhl->hCbus0Callback[i]), pMhl->hInterrupt,
			pCbus0Interrupts[i].IntrId,
			BHDM_P_Mhl_HandleCbus0Interrupt_isr, (void *)pMhl, i ));

		/* clear interrupt callback */
		BHDM_MHL_CHECK_RC(rc, BINT_ClearCallback( pMhl->hCbus0Callback[i]));
	}

	for( i = 0; i < MAKE_MHL_CBUS_INTR_1_ENUM(LAST); i++ )
	{
		/*
		** DEBUG
		** Create ALL interrupt callbacks
		** enable debug callbacks as needed;
		*/

		BHDM_MHL_CHECK_RC( rc, BINT_CreateCallback(
			&(pMhl->hCbus1Callback[i]), pMhl->hInterrupt,
			pCbus1Interrupts[i].IntrId,
			BHDM_P_Mhl_HandleCbus1Interrupt_isr, (void *)pMhl, i ));

		/* clear interrupt callback */
		BHDM_MHL_CHECK_RC(rc, BINT_ClearCallback( pMhl->hCbus1Callback[i]));
	}

	/* Register and enable MPM HOST interrupt callbacks */
	for( i = 0; i < MAKE_MHL_MPM_HOST_INTR_ENUM(LAST); i++ )
	{
		/*
		** DEBUG
		** Create ALL interrupt callbacks
		** enable debug callbacks as needed;
		*/

		BHDM_MHL_CHECK_RC( rc, BINT_CreateCallback(
			&(pMhl->hMpmHostCallback[i]), pMhl->hInterrupt,
			pMpmHostInterrupts[i].IntrId,
			BHDM_P_Mhl_HandleMpmHostInterrupt_isr, (void *)pMhl, i ));

		/* clear interrupt callback */
		BHDM_MHL_CHECK_RC(rc, BINT_ClearCallback( pMhl->hMpmHostCallback[i]));
	}

	BHDM_P_Mhl_EnableMpmHostInterrupts(pMhl);

	/* keep created pointer */
	*phMhl = (BHDM_P_Mhl_Handle)pMhl;

	BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_MscReq_Create(&pMhl->hMscReq));
	BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_DdcReq_Create(&pMhl->hDdcReq));
	BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_MscResp_Create(&pMhl->hMscResp));


done:
	BDBG_LEAVE(BHDM_P_Mhl_Create);
	return rc;
}

/******************************************************************************
BERR_Code BHDM_P_Mhl_Close
Summary: Destroy the MHL device.
*******************************************************************************/
BERR_Code BHDM_P_Mhl_Destroy
	( BHDM_P_Mhl_Handle hMhl )
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t i;

	BDBG_ENTER(BHDM_P_Mhl_Destroy);
	BDBG_OBJECT_ASSERT(hMhl, HDMI_MHL );

	BTMR_StopTimer(hMhl->hMhlTimer);
	BTMR_DestroyTimer(hMhl->hMhlTimer);

	BHDM_P_Mhl_DisableCbus0Interrupts(hMhl);
	BHDM_P_Mhl_DisableCbus1Interrupts(hMhl);
	BHDM_P_Mhl_DisableMpmHostInterrupts(hMhl);

	BKNI_DestroyEvent(hMhl->hEventStandby);

	/* Destroy the MPM HOST Callbacks */
	for ( i = 0; i < MAKE_MHL_MPM_HOST_INTR_ENUM(LAST); i++ )
	{
		/* all interrupts are now created; destroy all on close */
		BHDM_MHL_CHECK_RC( rc, BINT_DestroyCallback( hMhl->hMpmHostCallback[i] ) );
	}

	/* Destroy the CBUS Callbacks */
	for ( i = 0; i < MAKE_MHL_CBUS_INTR_0_ENUM(LAST); i++ )
	{
		/* all interrupts are now created; destroy all on close */
		BHDM_MHL_CHECK_RC( rc, BINT_DestroyCallback( hMhl->hCbus0Callback[i] ) );
	}

	for ( i = 0; i < MAKE_MHL_CBUS_INTR_1_ENUM(LAST); i++ )
	{
		/* all interrupts are now created; destroy all on close */
		BHDM_MHL_CHECK_RC( rc, BINT_DestroyCallback( hMhl->hCbus1Callback[i] ) );
	}

	if (hMhl->hMscResp)
	{
		BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_MscResp_Destroy(hMhl->hMscResp));
	}

	if (hMhl->hMscReq)
	{
		BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_MscReq_Destroy(hMhl->hMscReq));
	}

	if (hMhl->hDdcReq)
	{
		BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_DdcReq_Destroy(hMhl->hDdcReq));
	}

	/* TODO: PM close */

#if BHDM_MHL_ENABLE_DEBUG
	fclose(s_pfCbusLog);
#endif

	/* free memory associated with the HDMI handle */
	BDBG_OBJECT_DESTROY(hMhl, HDMI_MHL );
	BKNI_Free( (void *) hMhl);
	hMhl = (BHDM_P_Mhl_Handle)NULL;

done:

	BDBG_LEAVE(BHDM_P_Mhl_Destroy);
	return rc;
}

BERR_Code BHDM_P_Mhl_Init
	( BHDM_P_Mhl_Handle hMhl,
	  BHDM_Handle       hHdmi )
{
	BERR_Code rc = BERR_SUCCESS;
	BREG_Handle hRegister;
	BTMR_Settings timerSettings  ;

	BDBG_ASSERT(hMhl);
	BDBG_ASSERT(hHdmi);

	hRegister = hMhl->hRegister;
	hMhl->hHdmi = hHdmi;


	BHDM_P_Mhl_EnableMpmUart(hRegister);

	/* create timer */
	/* create OTP Calculation Check expiration timer */
	BTMR_GetDefaultTimerSettings(&timerSettings);
	timerSettings.type =  BTMR_Type_eStopWatch;
	timerSettings.cb_isr = NULL;
	timerSettings.pParm1 = hMhl;
	timerSettings.parm2 = BHDM_P_TIMER_eCbusTimer;
	timerSettings.exclusive = false;
	rc = BTMR_CreateTimer(hMhl->hTmr, &hMhl->hMhlTimer, &timerSettings);

	if (rc != BERR_SUCCESS)
	{
		rc = BERR_TRACE(BERR_LEAKED_RESOURCE);
	}

	BTMR_StartTimer_isr(hMhl->hMhlTimer, 0);

	/* A non-zero value is taken as FCB has been initialised by the host */
	hMhl->ucHostFcbVersion = BHDM_P_MHL_HOST_GET_FIELD(hRegister, REV);

	hMhl->stCbusState.ucTxVendorId = BHDM_P_MHL_VENDOR_ID;

	/* Initial link mode = 24-bit per pixel, PATH_EN=0, MUTED=0 */
	hMhl->stCbusState.ucSrcLinkMode = (BHDM_P_Mhl_ClkMode_e24bit << BHDM_P_MHL_LINK_MODE_CLK_MODE_SHIFT) |
									  (BHDM_P_Mhl_PathEn_e0 << BHDM_P_MHL_LINK_MODE_PATH_EN_SHIFT) |
									  (BHDM_P_Mhl_MuteState_eUnmuted << BHDM_P_MHL_LINK_MODE_MUTED_SHIFT);

	/* Source DCAP to initialise the mailbox with */
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eDevStateAddr]       = 0; /* Undefined in the spec */
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eMhlVersionAddr]     = BHDM_P_MHL_CBUS_DCAP_MHL_VER;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eDevCatAddr]         = BHDM_P_MHL_CBUS_DCAP_DEV_CAT_TYPE |
																				    BHDM_P_MHL_CBUS_DCAP_DEV_CAT_POW |
																				    BHDM_P_MHL_CBUS_DCAP_DEV_CAT_PLIM;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eAdopterIdHiAddr]    = BHDM_P_MHL_BCM_ID >> 8;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eAdopterIdLoAddr]    = BHDM_P_MHL_BCM_ID & 0xFF;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eVidLinkModeAddr]    = BHDM_P_MHL_CBUS_DCAP_VID_LINK_MODE;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eAudLinkModeAddr]    = BHDM_P_MHL_CBUS_DCAP_AUD_LINK_MODE;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eVideoTypeAddr]      = BHDM_P_MHL_CBUS_DCAP_VIDEO_TYPE;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eLogDevMapAddr]      = BHDM_P_MHL_CBUS_DCAP_LOG_DEV_MAP;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eBandwidthAddr]      = BHDM_P_MHL_CBUS_DCAP_BANDWIDTH;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eFeatureFlagAddr]    = BHDM_P_MHL_CBUS_DCAP_FEATURE;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eDeviceIdHiAddr]     = BHDM_P_MHL_DEVICE_ID >> 8;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eDeviceIdLoAddr]     = BHDM_P_MHL_DEVICE_ID & 0xFF;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eScratchpadSizeAddr] = BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE;
	hMhl->stCbusState.aucCbusSrcDcap[BHDM_P_Mhl_DevCapOffset_eIntStatSizeAddr]    = BHDM_P_MHL_CBUS_DCAP_INT_STAT_SIZE;

	hMhl->bMhlLinkEstablished = false;

	hMhl->pfHotplugChangeCallback = NULL;
	hMhl->pvHotplugChangeParm1 = NULL;
	hMhl->iHotplugChangeParm2 = 0;

	/* MSC Requester Init */
	BHDM_P_Mhl_MscReq_Init(hMhl->hMscReq, hMhl->hRegister);

	/* DDC Requester Init */
	BHDM_P_Mhl_DdcReq_Init(hMhl->hDdcReq, hMhl->hRegister, (hHdmi->DeviceSettings.bResumeFromS3 || !hMhl->bChipIsInS3));

	/* MSC Responder Init */
	BHDM_P_Mhl_MscResp_Init(hMhl->hMscResp, hMhl->hMscReq, hMhl->hDdcReq, hMhl->hRegister);

#if BHDM_MHL_CTS /* CTS testing only */
	if(hMhl->ucHostFcbVersion == 0)
	{
		/* Initialise CBUS hardware only if it has not already been initialised
		   and we initialise the hardware first to give RCAL as much time
		   as possible to calibrate */
		BDBG_MSG(("Initialising CBUS HW"));
		BHDM_P_Mhl_Cbus_PhyInit(hRegister);
		BHDM_P_Mhl_Cbus_Init(hRegister);

		BHDM_P_Mhl_Host_MailboxInit(hRegister, BHDM_P_MHL_MAILBOX_REVISION,
									hMhl->stCbusState.aucCbusSrcDcap);
	}
	else
	{
		BHDM_P_Mhl_Host_MailboxInit(hRegister, BHDM_P_MHL_MAILBOX_REVISION,
									hMhl->stCbusState.aucCbusSrcDcap);

		hMhl->stCbusState.eLastHpdState = (BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, HPD)) ?
										 BHDM_P_Mhl_CbusHpd_eUp : BHDM_P_Mhl_CbusHpd_eDown;
		BHDM_P_Mhl_Cbus_GetDiscoveryStatus(hRegister, &hMhl->eDiscovery);

	}
#else

	hMhl->bVbusNotRequired = !hHdmi->bMhlMode;

	hMhl->stCbusState.eLastHpdState = (BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, HPD)) ?
									 BHDM_P_Mhl_CbusHpd_eUp : BHDM_P_Mhl_CbusHpd_eDown;

	if (hMhl->stCbusState.eLastHpdState == BHDM_P_Mhl_CbusHpd_eDown)
	{
		BDBG_WRN(("HPD is down."));
	}

	hMhl->ePathEnSentState = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SINK_PATH_EN);

	if (hMhl->ePathEnSentState != BHDM_P_Mhl_CbusPathEnSentState_ePending)
	{
		BDBG_WRN(("Sink has not PATH_EN = 1."));
	}

#endif

	/* Initialize HDMI EDID block */
	BKNI_Memcpy((void *)hMhl->hHdmi->AttachedEDID.Block, /* this has 128 entries */
		(void *)hMhl->hDdcReq->pucEdidBlock,             /* this has 512 entries */
		sizeof(uint8_t) * BHDM_EDID_BLOCKSIZE);          /* copy only 128 entries */


	BHDM_P_Mhl_ResetHeartBeat(hMhl);
	hMhl->ulTickCount = BHDM_P_MHL_CBUS_CMD_SEND_TICK_INTERVAL;
	hMhl->pstHbPkts = &s_astDefaultHbPkts[0];
	hMhl->ulNumHbPkts = s_ulDefaultHbPktCnt;

	return rc;
}

/* TODO: Handle this when going from S0 to S3 */
void BHDM_P_Mhl_HandleHandoverToMpm_isr
	( BHDM_P_Mhl_Handle hMhl )
{
	BREG_Handle hRegister;
	uint8_t ucSrcLinkMode;
	uint32_t ulData;

	BDBG_ASSERT(hMhl);

	hRegister = hMhl->hRegister;

	/* TODO: Send any pedning commands inthe Queues */

	/* Power down MHL TX PHY differential output EFORE sending PATH_EN=0.  */
	BHDM_P_Mhl_Host_EnableMhlTx_isr(hRegister, false);
	hMhl->ePathEnSentState = BHDM_P_Mhl_CbusPathEnSentState_eNotNeeded;

	/* Reply PATH_EN=0 to sink */
	BHDM_P_MHL_MAILBOX_CLR_FIELD_ISR(hRegister, SRC_PATH_EN_1);
	ucSrcLinkMode = BHDM_P_MHL_HOST_GET_FIELD_ISR(hRegister, SRC_LINK_MODE_2);
	BHDM_P_Mhl_MscReq_WriteStat_isr(hMhl->hMscReq, BHDM_P_Mhl_StatusAddr_eLinkModeAddr,
									ucSrcLinkMode, 1, true);
#if BHDM_MHL_CTS
	BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, MSC_REQ_PENDING, 1);
#else
	hMhl->hMscReq->hReq->bRequestPending = true;
#endif

	/* Update STAT registers in FCB */
	/* Update FCB Rev */
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, REV, 1);

	/* Update VBUS*/
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, VBUS_NREQ, hMhl->bVbusNotRequired);

	/* SINK_CONNECTED_RDY */
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, SINK_DCAP_RDY,
			       BHDM_P_MHL_MAILBOX_GET_FIELD_ISR(hRegister, SINK_CONNECTED_RDY));

	/* HPD */
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, HPD, hMhl->stCbusState.eLastHpdState);

	/* SRC_MSC_ERRORCODE */
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, SRC_MSC_ERRORCODE, hMhl->hMscReq->hReq->ucErrCode);

	/* SRC_DDC_ERRORCODE */
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, SRC_DDC_ERRORCODE, hMhl->hDdcReq->hReq->ucErrCode);

	/* SRC_LINK_MODE_2 */
	BHDM_P_MHL_HOST_SET_FIELD_ISR(hRegister, SRC_CLK_MODE_2, hMhl->eMhlClkMode);

	/* TODO: Load new FW and if so, set MPM_IMEM_VALID bit */
	ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_MISC_CTRL);
	ulData &= ~BCHP_MPM_CPU_CTRL_MISC_CTRL_SET_VALUE_OF_MPM_IMEM_VALID_MASK;
	ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_MISC_CTRL, SET_VALUE_OF_MPM_IMEM_VALID, 0);
	BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_MISC_CTRL, ulData);

	BHDM_P_Mhl_Host_S0ToS3Handover_isr(hRegister);

	hMhl->hostState = BHDM_P_Mhl_HostCpuState_eStopping;

}

static void BHDM_P_Mhl_SetMhlMode
	( BHDM_P_Mhl_Handle      hMhl,
	  BAVC_HDMI_BitsPerPixel eBpp )
{
	switch (eBpp)
	{
		case BAVC_HDMI_BitsPerPixel_e24bit:
			hMhl->eMhlCtrlMode = BHDM_P_Mhl_CtrlMode_eMhl24bpp;
			hMhl->eMhlClkMode = BHDM_P_Mhl_ClkMode_e24bit;
			break;
		case BAVC_HDMI_BitsPerPixel_ePacked:
			hMhl->eMhlCtrlMode = BHDM_P_Mhl_CtrlMode_eMhlPackedPixel;
			hMhl->eMhlClkMode = BHDM_P_Mhl_ClkMode_ePacked;
			break;
		default:
			hMhl->eMhlCtrlMode = BHDM_P_Mhl_CtrlMode_eHdmi;
			hMhl->eMhlClkMode = BHDM_P_Mhl_ClkMode_eHdmi;
			break;
	}
}

BERR_Code BHDM_P_Mhl_ConfigLink
	( BHDM_P_Mhl_Handle      hMhl,
	  BFMT_VideoFmt          eVideoFmt,
	  BAVC_HDMI_BitsPerPixel eBpp )
{
	BERR_Code rc = BHDM_P_MHL_CBUS_SUCCESS;
	uint32_t ulData;
	BREG_Handle hRegister;
	uint32_t ulHdmiOffset;

	BDBG_ASSERT(hMhl);

	BDBG_MSG(("Configuring MHL link for fmt %d, bpp %d", eVideoFmt, eBpp));

	/* Only 8bpp are supported */
	BDBG_ASSERT(eBpp == BAVC_HDMI_BitsPerPixel_e24bit);

	hRegister = hMhl->hRegister;
	ulHdmiOffset = hMhl->hHdmi->ulOffset;

	if (eVideoFmt == BFMT_VideoFmt_e1080p ||
		eVideoFmt == BFMT_VideoFmt_e1080p_50Hz)
	{
		uint8_t ucValue;

		/* Check if sink's DCAP supports PackedPixel */
		ucValue = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SINK_DCAP_VLINK_MODE);
		BDBG_ERR(("SINK_DCAP_VLINK_MODE [0x%x]", ucValue));

		if (ucValue & BHDM_P_MHL_VIDEO_LINK_MODE_PACKED_PIXEL)
		{
			BDBG_ERR(("Set to packed pixel mode [0x%x]", ucValue));
			/* override default BPP */
			eBpp = BAVC_HDMI_BitsPerPixel_ePacked;
		}
		else
		{
			BDBG_ERR(("1080p is not supported by the sink."));
			rc = BERR_INVALID_PARAMETER;
			return rc;
		}
	}

	BHDM_P_Mhl_SetMhlMode(hMhl, eBpp);
	BHDM_P_Mhl_SendClockMode(hMhl);

	/* 24-bit mode */
    if ( hMhl->eMhlCtrlMode == BHDM_P_Mhl_CtrlMode_eMhl24bpp )
    {
		BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CLK_WORD_SEL + ulHdmiOffset,
			BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CLK_WORD_SEL, CLK_WORD_SEL, 2));

		ulData = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CFG_0 + ulHdmiOffset);

		ulData &= ~(BCHP_HDMI_TX_PHY_TMDS_CFG_0_TMDS_CLK_WORD_2_MASK |
					BCHP_HDMI_TX_PHY_TMDS_CFG_0_TMDS_CLK_WORD_1_MASK |
					BCHP_HDMI_TX_PHY_TMDS_CFG_0_TMDS_CLK_WORD_0_MASK);
		ulData |= (BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_2, 0x7) |
				   BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_1, 0x3e0) |
				   BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_0, 0x3ff));

		BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CFG_0 + ulHdmiOffset, ulData);
    }
    else /* 16-bit packed pixel mode */
    {
		BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CLK_WORD_SEL + ulHdmiOffset,
			BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CLK_WORD_SEL, CLK_WORD_SEL, 3));

		BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CFG_1 + ulHdmiOffset,
			BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_1, TMDS_CLK_WORD_3, 0x3ff));

		ulData = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CFG_0 + ulHdmiOffset);
		ulData &= ~(BCHP_HDMI_TX_PHY_TMDS_CFG_0_TMDS_CLK_WORD_2_MASK |
					BCHP_HDMI_TX_PHY_TMDS_CFG_0_TMDS_CLK_WORD_1_MASK |
					BCHP_HDMI_TX_PHY_TMDS_CFG_0_TMDS_CLK_WORD_0_MASK);
		ulData |= (BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_2, 0x3ff) |
				   BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_1, 0x7) |
				   BCHP_FIELD_DATA(HDMI_TX_PHY_TMDS_CFG_0, TMDS_CLK_WORD_0, 0));
		BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_TMDS_CFG_0 + ulHdmiOffset, ulData);

		ulData = BREG_Read32(hRegister, BCHP_DVP_HT_VEC_INTERFACE_CFG);
		ulData &= ~BCHP_MASK(DVP_HT_VEC_INTERFACE_CFG, SEL_422);
		ulData |= BCHP_FIELD_DATA(DVP_HT_VEC_INTERFACE_CFG, SEL_422, 0x3);
		BREG_Write32(hRegister, BCHP_DVP_HT_VEC_INTERFACE_CFG, ulData);

	}

	/* Configure mhl_packer: */
	BREG_Write32(hRegister, BCHP_HDMI_MHL_CTL + ulHdmiOffset,
	BCHP_FIELD_DATA(HDMI_MHL_CTL, MODE, hMhl->eMhlCtrlMode));

	return rc;
}

void BHDM_P_Mhl_EnableMhlLink
	( BHDM_P_Mhl_Handle      hMhl,
	  BFMT_VideoFmt          eFormat )
{
	BREG_Handle hRegister;
	uint32_t ulData;
	uint32_t ulHdmiOffset;
	uint32_t ulTx2AmpCtrl;

	BDBG_ASSERT(hMhl);
	hRegister = hMhl->hRegister;
	ulHdmiOffset = hMhl->hHdmi->ulOffset;

	BDBG_MSG(("Enabling MHL link"));

	switch (eFormat)
	{
		case BFMT_VideoFmt_e480p:
		case BFMT_VideoFmt_e576p_50Hz:
			ulTx2AmpCtrl = 0xe;
			break;
		case BFMT_VideoFmt_e720p:
		case BFMT_VideoFmt_e1080p_24Hz:
		case BFMT_VideoFmt_e1080i:
		case BFMT_VideoFmt_e1080i_50Hz:
		case BFMT_VideoFmt_e1080p:
		case BFMT_VideoFmt_e1080p_50Hz:
			ulTx2AmpCtrl = 0xf;
			break;
		default:
			ulTx2AmpCtrl = 0xa;
			break;
	}

	/* Configure MHL TX high-speed outputs */
	/* Initialize HDMI/MHL Tx PHY */
	ulData = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_CHANNEL_SWAP + ulHdmiOffset);
	ulData &= ~(BCHP_HDMI_TX_PHY_CHANNEL_SWAP_TXCK_OUT_INV_MASK |
				BCHP_HDMI_TX_PHY_CHANNEL_SWAP_TX0_OUT_INV_MASK |
				BCHP_HDMI_TX_PHY_CHANNEL_SWAP_TX1_OUT_INV_MASK |
				BCHP_HDMI_TX_PHY_CHANNEL_SWAP_TX2_OUT_INV_MASK);
	ulData |= (BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TXCK_OUT_INV, 0) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX0_OUT_INV, 0) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX1_OUT_INV, 0) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_CHANNEL_SWAP, TX2_OUT_INV, 0));
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_CHANNEL_SWAP + ulHdmiOffset, ulData);

	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_MHL_CTL_0 + ulHdmiOffset,
			BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_0, IBIAS_INTR_EN, 0)); /*0 to use external 6.04K reference resistor */

	/* 480mV ( AMS i_hdmi_ctrl[208:205]=tx0_mhlamp_ctrl ) */
	/* The following typcally matches which HDMI_TX_PHY_MHL_CTL_1.TXn_EN lines are set
	   but it's ok to set all these 2 0xA */
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_MHL_CTL_0,
		(BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_0, TX0_AMP_CTRL, 0xA) |
		 BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_0, TX1_AMP_CTRL, 0xA) |
		 BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_0, TX2_AMP_CTRL, ulTx2AmpCtrl)));
	/*
	   MHL+/- uses the TMDS_D0 pin in the HDMI connector

	   Common mode clock amplitude control: (Summary from AMS)
		V_CM_SWING = ( TXi_AMP_CTRL[3] * 0.4mA +
					   TXi_AMP_CTRL[2] * 0.2mA +
					   TXi_AMP_CTRL[1] * 0.1mA +
					   TXi_AMP_CTRL[0] * 0.05mA ) * 16 * 60ohms
		HDMI_TX_PHY.MHL_CTL_0.TX0_AMP_CTRL = &hA '
	*/
	ulData = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_MHL_CTL_1 + ulHdmiOffset);
	ulData &= ~(BCHP_HDMI_TX_PHY_MHL_CTL_1_TXCK_EN_MASK |
				BCHP_HDMI_TX_PHY_MHL_CTL_1_TX0_EN_MASK |
				BCHP_HDMI_TX_PHY_MHL_CTL_1_TX1_EN_MASK |
				BCHP_HDMI_TX_PHY_MHL_CTL_1_TX2_EN_MASK);
	ulData |= (BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_1, TXCK_EN, 0) |
#if defined NEXUS_USE_7250_DGL && (BCHP_CHIP == 7250 && BCHP_VER >= BCHP_VER_B0) /* Male HDMI connector (all dongle boards) */
			   BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_1, TX0_EN, 0) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_1, TX1_EN, 0) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_1, TX2_EN, 1));
#else
			   BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_1, TX0_EN, 1) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_1, TX1_EN, 0) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_MHL_CTL_1, TX2_EN, 0));
#endif
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_MHL_CTL_1 + ulHdmiOffset, ulData);

	/*
		Grant MHL Tx output control to MPM FW
		HDMI_TX_PHY.POWERDOWN_CTL.TX_PWRDN_SEL = 1
		*_MHL_EN are needed in order for power_*_tx() subroutines to work,
		as required by the phy_i_*_pwrdn logic in falcon__7364.v`
	*/
	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulHdmiOffset,
		BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_PWRDN_SEL, 1));

	/* Enable MHL Tx */
	ulData = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulHdmiOffset);
	ulData &= ~(BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_0_PWRDN_MASK |
				BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_1_PWRDN_MASK |
				BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_2_PWRDN_MASK |
				BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_CK_PWRDN_MASK);
	ulData |= (BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN, 1) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN, 1) |
#if defined NEXUS_USE_7250_DGL && (BCHP_CHIP == 7250 && BCHP_VER >= BCHP_VER_B0) /* Male HDMI connector (all dongle boards) */
			   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN, 1) |
			   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, 0)); /* Only active TMDS pair in MHL mode */
#else
			   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN, 0) | /* Only active TMDS pair in MHL mode */
			   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, 1));
#endif

	BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulHdmiOffset, ulData);

	if (hMhl->bMhlLinkEstablished == false)
	{

		BDBG_MSG(("Sending TX_READY to MPM."));
		BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, HOST_TX_READY, 1);

		/* Set MPM_CPU_L2.TX_READY */
		ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_L2_SET0);
		ulData &= ~BCHP_MPM_CPU_L2_SET0_TX_READY_MASK;
		ulData |= BCHP_FIELD_DATA(MPM_CPU_L2_SET0, TX_READY, 1);
		BREG_Write32(hRegister, BCHP_MPM_CPU_L2_SET0, ulData);

		/* Wait for a few ms while MPM sends PATH_EN=1 from MPM to the sink
		   and then send/trigger STOP_REQUEST */
		BKNI_Delay(50000); /* 50 ms */

		BDBG_MSG(("Sending STOP_REQUEST to MPM."));
		BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, HOST_STOP_REQ, 1);

		/* Set MPM_CPU_L2.STOP_REQUEST */
		ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_L2_SET0);
		ulData &= ~BCHP_MPM_CPU_L2_SET0_STOP_REQUEST_MASK; /* SW_INTR.STOP_REQUEST bit 17 */
		ulData |= BCHP_FIELD_DATA(MPM_CPU_L2_SET0, STOP_REQUEST, 1);
		BREG_Write32(hRegister, BCHP_MPM_CPU_L2_SET0, ulData);

		hMhl->bMhlLinkEstablished = true;
	}
}

#if BHDM_FORCED_MHL_MODE
static void  BHDM_P_Mhl_ForcedMhlModeConfig
	( BREG_Handle           hRegister )
{
	uint32_t ulMpmWakeup = 0, ulData;
	uint8_t ucEdidState;

#if (BCHP_CHIP==7364)
	BHDM_P_Mhl_Cbus_Init(hRegister);
#endif

	/* Update FCB Rev */
	BHDM_P_MHL_HOST_SET_FIELD(hRegister, REV, 1);

	/* VBUS: hMhl->bVbusNotRequired */
	BHDM_P_MHL_HOST_SET_FIELD(hRegister, VBUS_NREQ, 1);

	BHDM_P_MHL_HOST_SET_FIELD(hRegister, SRC_CLK_MODE_2, 0x3);

	BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL,
		BCHP_FIELD_DATA(MPM_CPU_CTRL_RESET_CTRL, CPU_RESET_LEVEL, 1));

	BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL,
		BCHP_FIELD_DATA(MPM_CPU_CTRL_RESET_CTRL, CPU_RESET_LEVEL, 0));

	do
	{
		/* Wait for MPM FW to wake up chip */
		ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_PM_CTRL);
		ulMpmWakeup = BCHP_GET_FIELD_DATA(ulData, MPM_CPU_CTRL_PM_CTRL, MPM_WAKEUP_SYSTEM);
	} while (ulMpmWakeup == 0);

	/* Clear MPM_WAKEUP_SYSTEM and go to S0 */
	BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_PM_CTRL,
		BCHP_FIELD_DATA(MPM_CPU_CTRL_PM_CTRL, MPM_WAKEUP_SYSTEM, 0));

	/* Update FCB Rev */
	BHDM_P_MHL_HOST_SET_FIELD(hRegister, REV, 1);

	do
	{
		ucEdidState = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, EDID_VALID);

	} while (ucEdidState == 0);

}
#endif

BERR_Code BHDM_MHL_P_OpenMhl
	( BHDM_Handle hHdm )
{
	BERR_Code rc = BERR_SUCCESS;
	BREG_Handle hRegister = hHdm->hRegister;
	uint32_t ulData;

#if BHDM_FORCED_MHL_MODE
	hHdm->bMhlMode = true;
	BHDM_P_Mhl_ForcedMhlModeConfig(hRegister);
#else
	ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_STATUS);
	hHdm->bMhlMode = (BCHP_GET_FIELD_DATA(ulData, MPM_CPU_CTRL_STATUS, STRAP_MHL_POWERUP)) ? true : false;
#endif

	if (hHdm->bMhlMode)
	{
		uint8_t ucPathEn;
		bool bDisconnectDone = false;
		bool bChipIsInS3;

		ucPathEn = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SINK_PATH_EN);

		/* These handle subsequent invocations of the app while in S0 but only used when testing and or during development.
		   This is unused in practical applications. See BHDM_MHL_TEST_DEVEL_MODE.
		   Basically, it resets the MPM so that the proper transition of CBUS ownership is guaranteed. */
		bChipIsInS3 = (BCHP_GET_FIELD_DATA(ulData, MPM_CPU_CTRL_STATUS, CHIP_IS_IN_S3)) ? true : false;

		/* The following will only take p[alce when S3 was induced by the app and not the sink. This ensures that MPM would
		   go through the complete CBUS initialization and EDID read. */
		if ((hHdm->DeviceSettings.bResumeFromS3 && !ucPathEn)

#if BHDM_MHL_TEST_DEVEL_MODE
			|| !bChipIsInS3
#endif
			)
		{
			bool bMpmBootComplete = false;

			/* Clear FCB REV to trigger read of EDID */
			BHDM_P_MHL_HOST_SET_FIELD(hRegister, REV, 0);

			/* Hold reset MPM CPU */
			ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL);
			ulData &= ~BCHP_MPM_CPU_CTRL_RESET_CTRL_CPU_RESET_LEVEL_MASK;
			ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_RESET_CTRL, CPU_RESET_LEVEL, 1);
			BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL, ulData);

			/* Clear MPM boot complete flag */
			BREG_Write32(hRegister, BCHP_MT_CBUS_SPARE_REG_0, 0);

			/* Start CBUS disconnect sequence */
			ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_LINK_CTRL_0);
			ulData &= ~BCHP_MT_CBUS_LINK_CTRL_0_START_DISCONNECT_MASK;
			ulData |= BCHP_FIELD_DATA(MT_CBUS_LINK_CTRL_0, START_DISCONNECT, 1);
			BREG_Write32(hRegister, BCHP_MT_CBUS_LINK_CTRL_0, ulData);

			/* Poll DISCONNECT_DONE interrupt */
			do
			{
				ulData = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_STATUS);
				bDisconnectDone = (BCHP_GET_FIELD_DATA(ulData, CBUS_INTR2_0_CPU_STATUS, DISCONNECT_DONE)) ? true : false;
			} while (!bDisconnectDone);

			/* Release reset MPM CPU */
			ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL);
			ulData &= ~BCHP_MPM_CPU_CTRL_RESET_CTRL_CPU_RESET_LEVEL_MASK;
			ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_RESET_CTRL, CPU_RESET_LEVEL, 0);
			BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL, ulData);

			do
			{
				bMpmBootComplete = (BREG_Read32(hRegister, BCHP_MT_CBUS_SPARE_REG_0)) ? true : false;
			} while (!bMpmBootComplete);

			BDBG_MSG(("Completed reset of MPM."));
		}

		BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_Create(&hHdm->hMhl, hHdm->hChip, hRegister, hHdm->hInterrupt, hHdm->hTMR, &hHdm->stMhlSettings));
		hHdm->hMhl->bChipIsInS3 = bChipIsInS3;
		BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_Init(hHdm->hMhl, hHdm));
		BHDM_MHL_CHECK_RC(rc, BKNI_CreateEvent(&(hHdm->hMhl->BHDM_MHL_EventStandby)));
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_CloseMhl
	( BHDM_Handle hHdm )
{
	BERR_Code rc = BERR_SUCCESS;
	if (hHdm->bMhlMode)
	{
		uint32_t ulRetries = 0;

		BHDM_P_Mhl_HandleHandoverToMpm_isr(hHdm->hMhl);

		do
		{
			BDBG_MSG(("Waiting for MPM CPU to wake up"));
			if (BKNI_WaitForEvent(hHdm->hMhl->hEventStandby, 100) == BERR_TIMEOUT)
			{
				ulRetries++;
			}
			else
			{
				break;
			}
		} while (ulRetries < 3);

		if (ulRetries == 3)
		{
			BDBG_WRN(("MPM CPU hasn't fully booted. Will resume system standby regardless."));
		}

		BDBG_MSG(("Completed host-to-MPM CBUS handover."));

		BHDM_P_Mhl_DisableCbus0Interrupts(hHdm->hMhl);
		BHDM_P_Mhl_DisableCbus1Interrupts(hHdm->hMhl);
		BHDM_P_Mhl_DisableMpmHostInterrupts(hHdm->hMhl);

		BKNI_DestroyEvent((hHdm->hMhl->BHDM_MHL_EventStandby));

		BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_Destroy(hHdm->hMhl));
	}

done:
	return rc;
}

BERR_Code BHDM_MHL_P_ConfigMhlLink
	( BHDM_Handle hHdm )
{
	BERR_Code rc = BERR_SUCCESS;
	if (hHdm->bMhlMode)
	{
		/* TODO: Check Rx sense, HPD, and PATH_EN=1 befor enabling high speed link */

		/* Configure MHL. Send colorspace and color depth (only 8-bits is supported). */
		BHDM_MHL_CHECK_RC(rc, BHDM_P_Mhl_ConfigLink(hHdm->hMhl, hHdm->DeviceSettings.eInputVideoFmt,
													hHdm->DeviceSettings.stVideoSettings.eBitsPerPixel));
		/* Enable MHL Link. We can safely assume that at this point
		   the video path is completely configured. */
		BHDM_P_Mhl_EnableMhlLink(hHdm->hMhl, hHdm->DeviceSettings.eInputVideoFmt);
	}
done:
	return rc;
}

void BHDM_MHL_P_ClearHpdState_isr
	( BHDM_Handle hHdm )
{
	if (hHdm->bMhlMode)
	{
		BHDM_P_Mhl_DdcReq_ClrHpdState_isr(hHdm->hMhl->hDdcReq);
	}
}

BERR_Code BHDM_MHL_P_ConfigPreemphasis
	( const BHDM_Handle               hHdm,
	  const BHDM_Settings            *pstNewHdmiSettings,
	  BHDM_PreEmphasis_Configuration *pstNewPreEmphasisConfig )

{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_MSG(("Configuring pre-emphasis filters for MHL"));

	/* Additional setup of  HDMI/MHL Tx combo PHY */
	switch (pstNewHdmiSettings->eInputVideoFmt)
	{
	case BFMT_VideoFmt_e720p:
	case BFMT_VideoFmt_e1080p_24Hz:
	case BFMT_VideoFmt_e1080i:
	case BFMT_VideoFmt_e1080i_50Hz:
	case BFMT_VideoFmt_e1080p:
	case BFMT_VideoFmt_e1080p_50Hz:
		if (hHdm->bMhlMode)
		{
			pstNewPreEmphasisConfig->uiPreEmphasis_Ch0 = 0xa ;
			pstNewPreEmphasisConfig->uiPreEmphasis_Ch1 = 0xa ;
			pstNewPreEmphasisConfig->uiPreEmphasis_Ch2 = 0x2f ;
			pstNewPreEmphasisConfig->uiPreEmphasis_CK = 0xa ;

			pstNewPreEmphasisConfig->uiTermResSelData2 = 0x1 ;
		}
		break;

#if BHDM_HAS_HDMI_20_SUPPORT
	case BFMT_VideoFmt_e3840x2160p_50Hz :
	case BFMT_VideoFmt_e3840x2160p_60Hz :
		pstNewPreEmphasisConfig->uiPreEmphasis_Ch0 = 0x7d ;
		pstNewPreEmphasisConfig->uiPreEmphasis_Ch1 = 0x7d ;
		pstNewPreEmphasisConfig->uiPreEmphasis_Ch2 = 0x7d ;
		break;
#endif

	default:
		break;
	}


	return rc;
}

void BHDM_MHL_P_EnableTmdsData_isr
	( const BHDM_Handle               hHdm )
{
	uint32_t ulData, ulOffset = hHdm->ulOffset;
	BREG_Handle hRegister = hHdm->hRegister;

	if (hHdm->bMhlMode)
	{
		BDBG_MSG(("Enabling TMDS data for MHL"));

		/* Enable MHL Tx */
		ulData = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset);
		ulData &= ~(BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_0_PWRDN_MASK |
					BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_1_PWRDN_MASK |
					BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_2_PWRDN_MASK |
					BCHP_HDMI_TX_PHY_POWERDOWN_CTL_TX_CK_PWRDN_MASK);
		ulData |= (BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_CK_PWRDN, 0) |
				   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_1_PWRDN, 0) |
#if defined NEXUS_USE_7250_DGL && (BCHP_CHIP == 7250 && BCHP_VER >= BCHP_VER_B0) /* Male HDMI connector (all dongle boards) */
				   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN, 1) |
				   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, 0)); /* Only active TMDS pair in MHL mode */
#else
				   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_0_PWRDN, 0) | /* Only active TMDS pair in MHL mode */
				   BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_2_PWRDN, 1));
#endif

		BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL + ulOffset, ulData);
	}
}

void BHDM_MHL_P_GetSupportedVideoFormats
	( BHDM_Handle   hHDMI,
	  bool         *bMhlSupportedVideoFormats )
{
	if (hHDMI->bMhlMode)
	{
		uint32_t i;
		BHDM_P_Mhl_DeviceCapabilities stDevCap;
		BHDM_P_Mhl_GetDeviceCapabilities(hHDMI->hMhl, &stDevCap);

		for (i=0; i <BFMT_VideoFmt_eMaxCount; i++)
		{
			bMhlSupportedVideoFormats[i] = hHDMI->AttachedEDID.BcmSupportedVideoFormats[i];

			/* Overrides 1080p60 and 1080p50 if necessary. Will also override 4K formats until
			   MHL 3.0 is supported. */
			if (((i == BFMT_VideoFmt_e1080p ||
				  i == BFMT_VideoFmt_e1080p_50Hz ||
				  i == BFMT_VideoFmt_e1080p_60Hz_3DOU_AS ||
				  i == BFMT_VideoFmt_e1080p_60Hz_3DLR ||
				  i == BFMT_VideoFmt_eDVI_1920x1080p_60Hz_Red) && !stDevCap.bSupportsPackedPixel) ||
				  (i >= BFMT_VideoFmt_e3840x2160p_24Hz && i <= BFMT_VideoFmt_e4096x2160p_60Hz))
			{
				bMhlSupportedVideoFormats[i] = false;
			}
		}
	}
	else
	{
		BKNI_Memcpy(bMhlSupportedVideoFormats, &hHDMI->AttachedEDID.BcmSupportedVideoFormats,
					sizeof(hHDMI->AttachedEDID.BcmSupportedVideoFormats)) ;
	}
}

BERR_Code BHDM_MHL_P_ValidatePreferredFormat
	( BHDM_Handle            hHDMI,
	  BFMT_VideoFmt          eFmt )
{
	BERR_Code rc = BERR_SUCCESS;
	if (hHDMI->bMhlMode)
	{
		BHDM_P_Mhl_DeviceCapabilities stDevCap;
		BHDM_P_Mhl_GetDeviceCapabilities(hHDMI->hMhl, &stDevCap);

		BDBG_MSG(("Validating preferred format %d", eFmt));

		if (((eFmt == BFMT_VideoFmt_e1080p ||
			 eFmt == BFMT_VideoFmt_e1080p_50Hz ||
			 eFmt == BFMT_VideoFmt_e1080p_60Hz_3DOU_AS ||
			 eFmt == BFMT_VideoFmt_e1080p_60Hz_3DLR ||
			 eFmt == BFMT_VideoFmt_eDVI_1920x1080p_60Hz_Red) && !stDevCap.bSupportsPackedPixel)||
			(eFmt >= BFMT_VideoFmt_e3840x2160p_24Hz && eFmt <= BFMT_VideoFmt_e4096x2160p_60Hz))
		{
			rc = BHDM_UNSUPPORTED_VIDEO_FORMAT;
		}
	}
	return rc;
}
