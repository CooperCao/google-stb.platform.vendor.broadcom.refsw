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

#include "bhdm_mhl_debug_priv.h"

BDBG_MODULE(BHDM_MHL_DEBUG);
BDBG_OBJECT_ID(BHDM_MHL_DEBUG);

#if BHDM_MHL_ENABLE_DEBUG

#define BHDM_P_DEBUG_OUTPUT_TO_CONSOLE 0

/* Convert cbus command into a string */
typedef struct
{
	BHDM_P_Mhl_Command  eCmd;
	const char          acStr[15];
} BHDM_P_Mhl_CmdStrTab;

static const char *BHDM_P_Mhl_LookupCmdStr_isr
	( BHDM_P_Mhl_Command eCmd )
{
	static const BHDM_P_Mhl_CmdStrTab astCmdTab[] =
	{
		{BHDM_P_Mhl_Command_eEof,             "EOF"},
		{BHDM_P_Mhl_Command_eAck,             "ACK"},
		{BHDM_P_Mhl_Command_eNack,            "NACK"},
		{BHDM_P_Mhl_Command_eAbort,           "ABORT"},
		{BHDM_P_Mhl_Command_eWriteStat,       "WRITE_STAT"},
		{BHDM_P_Mhl_Command_eSetInt,          "SET_INT"},
		{BHDM_P_Mhl_Command_eReadDevCap,      "READ_DCAP"},
		{BHDM_P_Mhl_Command_eGetState,        "GET_STATE"},
		{BHDM_P_Mhl_Command_eGetVendorId,     "GET_VENDOR_ID"},
		{BHDM_P_Mhl_Command_eSetHpd,          "SET_HPD"},
		{BHDM_P_Mhl_Command_eClrHpd,          "CLR_HPD"},
		{BHDM_P_Mhl_Command_eMscMsg,          "MSC_MSG"},
		{BHDM_P_Mhl_Command_eGetSc1ErrorCode, "GET_SC1_ERR"},
		{BHDM_P_Mhl_Command_eGetDdcErrorCode, "GET_DDC_ERR"},
		{BHDM_P_Mhl_Command_eGetMscErrorCode, "GET_MSC_ERR"},
		{BHDM_P_Mhl_Command_eWriteBurst,      "WRITE_BURST"},
		{BHDM_P_Mhl_Command_eGetSc3ErrorCode, "GET_SC3_ERR"},
		{BHDM_P_Mhl_Command_eSof,             "SOF"},
		{BHDM_P_Mhl_Command_eCont,            "CONT"},
		{BHDM_P_Mhl_Command_eStop,            "STOP"}
	};

	uint32_t i;
	uint32_t ulSize = (sizeof(astCmdTab) / sizeof(astCmdTab[0]));

	for(i = 0; i < ulSize; i++)
	{
		if(astCmdTab[i].eCmd == eCmd)
		{
			return astCmdTab[i].acStr;
		}
	}
	return "???";
}

/********************************************************
	Output packets to console
*********************************************************/
void BHDM_P_Mhl_DumpCmdPkts_isr
	( BHDM_P_Mhl_CbusPkt          *pPackets,
	  uint32_t                     ulNumPackets,
	  BHDM_P_Mhl_CbusDest          eDestination )
{
	uint32_t i;
	char acTemp[10] = "";
	char acStr[200] = "";

	BDBG_ASSERT(pPackets);

	switch(eDestination)
	{
	case BHDM_P_Mhl_CbusDest_eMscReq:
		strcpy(acStr, "MSC REQ: ");
		break;
	case BHDM_P_Mhl_CbusDest_eMscResp:
		strcpy(acStr, "MSC RESP: ");
		break;
	case BHDM_P_Mhl_CbusDest_eDdcReq:
		strcpy(acStr, "DDC REQ: ");
		break;
	default:
		break;
	}

	if (pPackets[0].ulDir == (uint8_t)BHDM_P_Mhl_CbusPktDirection_eReq)
	{
		strcat(acStr, ">");
	}
	else
	{
		strcat(acStr, "<");
	}

	if(pPackets[0].ulType == (uint8_t)BHDM_P_Mhl_CbusPktType_eCtrl)
	{
		if (pPackets[0].ulDir == (uint8_t)BHDM_P_Mhl_CbusPktDirection_eReq)
		{
			strcat(acStr, ">");
		}
		else
		{
			strcat(acStr, "<");
		}

		strcat(acStr, BHDM_P_Mhl_LookupCmdStr_isr(pPackets[0].ucData));
		sprintf(acTemp, "%02X", pPackets[0].ucData);
		strcat(acStr, " (");
		strcat(acStr, acTemp);
		strcat(acStr, ") ");
	}
	else
	{
		sprintf(acTemp, "%02X", pPackets[0].ucData);
		strcat(acStr, acTemp);
		strcat(acStr, " ");
	}

	for(i = 1; i < ulNumPackets; i++)
	{
		if (pPackets[i].ulDir == (uint8_t)BHDM_P_Mhl_CbusPktDirection_eReq)
		{
			strcat(acStr, ">");
		}
		else
		{
			strcat(acStr, "<");
		}

		if(pPackets[i].ulType == (uint8_t)BHDM_P_Mhl_CbusPktType_eCtrl)
		{
			if (pPackets[i].ulDir == (uint8_t)BHDM_P_Mhl_CbusPktDirection_eReq)
			{
				strcat(acStr, ">");
			}
			else
			{
				strcat(acStr, "<");
			}
		}

		sprintf(acTemp, "%02X", pPackets[i].ucData);
		strcat(acStr, acTemp);
		strcat(acStr, " ");
	}

#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
	BDBG_MSG((acStr));
#else
	BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	BHDM_P_MHL_DEBUG_DUMP_TO_FILE("\n");
#endif

}

/********************************************************
	Output packets to console

	Delayed transmission.
	This is used for pacing the sink by artifically delaying
	the source's reply (ever so slightly).
	Delay is measured by clock ticks (e.g. 10ms)

	When a command is added, a delay can be optionally included.
	By default a command will have a zero delay, which means the
	command will go out as soon as possible, limited to the frequency
	of the ticks (100Hz for 10ms interval).

	At each clock tick, the delay of the first message in the queue
	is decremented if it is not already zero. When the delay reaches
	zero, the message is sent.

	Add a command to the queue, can fail if the queue is full
	Command is added based on the priority
	Higher priority commands always go first.
	Commands with the same priority are queued as FIFO.

	Arguments: array of packets, size of array
	          return buffer and size (can be null)
	      command type
	      priority
	      hardware destination
	      last command?

	Return cbus_success if successful

	Print the current state of the queue
*********************************************************/
void BHDM_P_Mhl_DumpCmdQueue_isr
	( BHDM_P_Mhl_Req_Handle    hReq )
{
	BHDM_P_Mhl_CmdQueue stLocalQ;
	BHDM_P_Mhl_CbusCmd stCmd;
	uint16_t pos = 0;
	char acStr[200] = "";
	char acTemp[10] = "";
	BHDM_P_Mhl_CbusDest eDest;
	BHDM_P_Mhl_CbusPkt *pPkts = NULL;

	BDBG_ASSERT(hReq);

	if (hReq->eType == BHDM_P_Mhl_ReqType_eDdc)
	{
#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
		BDBG_MSG(("Current DDC Q: "));
#else
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Current DDC Q:\n");
#endif
		eDest = BHDM_P_Mhl_CbusDest_eDdcReq;
		pPkts = stCmd.cbusPackets.astLongCmd;
	}
	else if (hReq->eType == BHDM_P_Mhl_ReqType_eMsc)
	{
#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
		BDBG_MSG(("Current MSC Q: "));
#else
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Current MSC Q:\n");
#endif
		eDest = BHDM_P_Mhl_CbusDest_eMscReq;
		pPkts = stCmd.cbusPackets.astShortCmd;
	}
	else
	{
		eDest = BHDM_P_Mhl_CbusDest_eUnknown;
#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
		BDBG_MSG(("Unknown REQ type. "));
#else
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Unknown REQ type.\n");
#endif
	}

	BDBG_ASSERT(eDest < BHDM_P_Mhl_CbusDest_eUnknown);
	BDBG_ASSERT(pPkts);

	/* save current command Q */
	BKNI_Memcpy(&stLocalQ, hReq->pstCmdQ, sizeof(BHDM_P_Mhl_CmdQueue));

	/* Get until FIFO is full */
	while (!BHDM_P_Mhl_Fifo_ReadData_isr(hReq->pstCmdQ, (uint8_t *)&stCmd, sizeof(stCmd)))
	{
		sprintf(acTemp, " %X", pos);
		strcpy(acStr, acTemp);
		strcat(acStr, ":");
		if(stCmd.eState == BHDM_P_Mhl_CbusCmdState_ePending)
		{
			strcat(acStr, "+");
		}
		else if(stCmd.eState == BHDM_P_Mhl_CbusCmdState_eSent)
		{
			strcat(acStr, "*");
		}
		else if(stCmd.eState == BHDM_P_Mhl_CbusCmdState_eCancelled)
		{
			strcat(acStr, "x");
		}
		else
		{
			strcat(acStr, "?");
			sprintf(acTemp, "%X", stCmd.eState);
			strcat(acStr, acTemp);
			strcat(acStr, "?");
		}

		strcat(acStr, "\n");
#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
		BDBG_MSG((acStr));
#else
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
#endif


		BHDM_P_Mhl_DumpCmdPkts_isr(pPkts, stCmd.ulNumPacketsCfg, eDest);

		pos++;

	}

	if (pos == 0) /* Q is empty, nothing was read */
	{
#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
		BDBG_MSG(("empty"));
#else
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("empty\n");
#endif
	}
	/* copy back the Command Q pointers to global structure */
	BKNI_Memcpy(hReq->pstCmdQ, &stLocalQ, sizeof(BHDM_P_Mhl_CmdQueue));

}

/* For debugging, we will sort out the constants later */
void BHDM_P_Mhl_FieldDecode_isr
	( uint8_t addr,
	  uint8_t data )
{
	uint32_t field0, field1, field2;
	char acStr[40];
	char acTemp[10];

	strcpy(acStr, " [");

	if(addr <= BHDM_P_Mhl_DevCapOffset_eIntStatSizeAddr)
		strcat(acStr, "DCAP-");
	else if(addr <= BHDM_P_Mhl_IntAddr_eDchangeIntAddr)
		strcat(acStr, "INT-");
	else if(addr <= BHDM_P_Mhl_StatusAddr_eLinkModeAddr)
		strcat(acStr, "STAT-");

	switch(addr)
	{
	case BHDM_P_Mhl_DevCapOffset_eDevStateAddr:
		strcat(acStr, "STATE:0x%X");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eMhlVersionAddr:
		field0 = BHDM_P_MHL_GET_FIELD(data, 0xF << 0, 0);
		field1 = BHDM_P_MHL_GET_FIELD(data, 0xF << 4, 4);
		strcat(acStr, "VER:");
		sprintf(acTemp, "%X.", field1);
		strcat(acStr, acTemp);
		sprintf(acTemp, "%X", field0);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eDevCatAddr:
	{
		field0 = BHDM_P_MHL_GET_FIELD(data, BHDM_P_MHL_DEV_CAT_MASK, BHDM_P_MHL_DEV_CAT_LSB);
		field1 = BHDM_P_MHL_GET_FIELD(data, BHDM_P_MHL_POW_OUT_MASK, BHDM_P_MHL_POW_OUT_LSB);
		field2 = BHDM_P_MHL_GET_FIELD(data, BHDM_P_MHL_POW_LIMIT_MASK, BHDM_P_MHL_POW_LIMIT_LSB);

		/* TODO: Dongle power limit is tricky:
		 *       If MHL 2.0 or 1.2 or earlier, dongle power limit is 100mA minimum.
		 *       If MHL 2.1, dongle power limit is (MHL 2.0 spec Table 7-28 note 7):
		 *         500mA min if PLIM[1:0] = 2'b00
		 *         900mA min if PLIM[1:0] = 2'b01
		 *        1500mA min if PLIM[1:0] = 2'b10
		 *         100mA min if PLIM[1:0] = 2'b11 */
		strcat(acStr, "CAT (PLIM.POUT.CAT):");
		sprintf(acTemp, "%X.", field2);
		strcat(acStr, acTemp);
		sprintf(acTemp, "%X.", field1);
		strcat(acStr, acTemp);
		sprintf(acTemp, "%X", field0);
		strcat(acStr, acTemp);
		break;
	}

	case BHDM_P_Mhl_DevCapOffset_eAdopterIdHiAddr:
		strcat(acStr, "ADT_ID_H:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eAdopterIdLoAddr:
		strcat(acStr, "ADT_ID_L:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eVidLinkModeAddr:
		strcat(acStr, "VLNK_MODE:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eAudLinkModeAddr:
		strcat(acStr, "ALNK_MODE:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eVideoTypeAddr:
		strcat(acStr, "VTYPE:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eLogDevMapAddr:
		strcat(acStr, "LOGDEV:");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eBandwidthAddr:
		strcat(acStr, "BW:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		strcat(acStr, "*5 = 0x");
		sprintf(acTemp, "%X", data*5);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eFeatureFlagAddr:
		strcat(acStr, "FEATURE:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eDeviceIdHiAddr:
		strcat(acStr, "DEV_ID_H:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eDeviceIdLoAddr:
		strcat(acStr, "DEV_ID_L:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eScratchpadSizeAddr:
		strcat(acStr, "SCR_SIZE:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eIntStatSizeAddr:
		strcat(acStr, "INT_SIZE:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_DevCapOffset_eDevCapAddrMax:
		strcat(acStr, "RSVR:");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_IntAddr_eRchangeIntAddr:
		strcat(acStr, "RCHG:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_IntAddr_eDchangeIntAddr:
		strcat(acStr, "DCHG:");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_StatusAddr_eConnectedRdyAddr:
		strcat(acStr, "CONN_RDY:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_StatusAddr_eLinkModeAddr:
		strcat(acStr, "LNK_MODE:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	default:
		strcat(acStr, "Invalid address");
		break;
	}

#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
	BDBG_MSG((acStr));
#else
	BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
#endif
}

void BHDM_P_Mhl_MscMsgDecode_isr
	( uint8_t action,
	  uint8_t data )
{
	char acStr[40];
	char acTemp[10];

	strcpy(acStr, " [");

	switch(action)
	{
	case BHDM_P_Mhl_MscMsgCommand_eRcp:
		strcat(acStr, "RCP key:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_MscMsgCommand_eRcpk:
		strcat(acStr, "RCPK key:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;

	case BHDM_P_Mhl_MscMsgCommand_eRcpe:
		strcat(acStr, "RCPE status:");
		switch(data)
		{
		case BHDM_P_Mhl_RcpError_eNone:
			strcat(acStr, "ok");
			break;
		case BHDM_P_Mhl_RcpError_eIneffective:
			strcat(acStr, "ineffective");
			break;
		case BHDM_P_Mhl_RcpError_eErrorBusy:
			strcat(acStr, "busy");
			break;
		default:
			strcat(acStr, "unknown code:0x");
			sprintf(acTemp, "%X", data);
			strcat(acStr, acTemp);
			break;
		}
		break;

	case BHDM_P_Mhl_MscMsgCommand_eRap:
		strcat(acStr, "RAP action:");
		switch(data)
		{
		case BHDM_P_Mhl_RapAction_ePoll:
			strcat(acStr, "poll");
			break;
		case BHDM_P_Mhl_RapAction_eContentOn:
			strcat(acStr, "content-on");
			break;
		case BHDM_P_Mhl_RapAction_eContentOff:
			strcat(acStr, "content-off");
			break;
		default:
			strcat(acStr, "unknown code:0x");
			sprintf(acTemp, "%X", data);
			strcat(acStr, acTemp);
			break;
		}
		break;

	case BHDM_P_Mhl_MscMsgCommand_eRapk:
		strcat(acStr, "RAPK status:");
		switch(data)
		{
		case BHDM_P_Mhl_RapError_eNone:
			strcat(acStr, "ok");
			break;
		case BHDM_P_Mhl_RapError_eUnrecognised:
			strcat(acStr, "unrecognised");
			break;
		case BHDM_P_Mhl_RapError_eUnsupported:
			strcat(acStr, "unsupported");
			break;
		case BHDM_P_Mhl_RapError_eBusy:
			strcat(acStr, "busy");
			break;
		default:
			strcat(acStr, "unknown code:0x");
			sprintf(acTemp, "%X", data);
			strcat(acStr, acTemp);
			break;
		}
		break;

	default:
		strcat(acStr, "unknown code:0x");
		sprintf(acTemp, "%X", data);
		strcat(acStr, acTemp);
		break;
	}

#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
	BDBG_MSG((acStr));
#else
	BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
#endif
}

/* Dump EDID */
void BHDM_P_Mhl_DumpEdid_isr
	( uint8_t    *pucEdidBlock,
	  uint32_t    ulEdidNumBlocks )
{
	uint32_t i,j,k;
	const uint32_t bytes_per_line = 16;
	uint8_t *ptr = pucEdidBlock;
	char acStr[40];
	char acTemp[10];

	BDBG_ASSERT(pucEdidBlock);

	BDBG_MSG(("*** EDID ***"));

	for(i = 0; i < ulEdidNumBlocks; i++)
	{
		strcat(acStr, "Block ");
		sprintf(acTemp, "%X", i);
		strcat(acStr, ": ");
		BDBG_MSG(("%s", acStr));

		for(j = 0;
		    j < BHDM_P_MHL_EDID_BLOCK_SIZE/bytes_per_line;
		    j++, ptr += bytes_per_line) {
			for(k = 0; k < bytes_per_line; k++)
			{
				BDBG_MSG(("%X", ptr[k]));
			}
		}
		BDBG_MSG((""));
	}
}

void BHDM_P_Mhl_DumpMailbox_isr
	( BREG_Handle hRegister )
{
	uint32_t i;

	for (i=0; i<BHDM_P_MHL_MAILBOX_SIZE; i++)
	{
		if (i%4) BDBG_MSG((""));
		BDBG_MSG(("2.2%X ", BHDM_P_Mhl_Mailbox_Read(hRegister, i)));
	}
}

void BHDM_P_Mhl_DumpRegisters_isr
	( BREG_Handle hRegister,
	  uint32_t    ulRegBaseAddr,
	  uint32_t    ulRegCount )
{
	uint32_t i, ulData;

#if !BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
	char acTemp[10];
	char acStr[200];
#endif

	for (i=0; i<ulRegCount; i++)
	{
		ulData = BREG_Read32(hRegister, ulRegBaseAddr + (i*4));
#if BHDM_P_DEBUG_OUTPUT_TO_CONSOLE
		BDBG_MSG(("[0x%x]: 0x%x\n", ulRegBaseAddr + (i*4), ulData));
#else
		{
			strcpy(acTemp,"");
			strcpy(acStr,"");

			strcat(acStr, "PKT REG 0x");
			sprintf(acTemp, "%X", ulRegBaseAddr + (i*4));
			strcat(acStr, acTemp);
			strcat(acStr, " = 0x");
			sprintf(acTemp, "%X", ulData);
			strcat(acStr, acTemp);
			strcat(acStr, "\n");
			BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
		}
#endif
	}
}

/* Print the Sink's DCAP */
void BHDM_P_Mhl_DumpSinkDcap_isr
	( BREG_Handle hRegister )
{
	BHDM_P_Mhl_DevCapOffset eDcapOffset;
	uint8_t astDcap[BHDM_P_MHL_CBUS_CAP_REG_SIZE];

	BDBG_MSG(("***SINK DCAP***"));

	BHDM_P_Mhl_Mailbox_GetSinkDcap(hRegister, astDcap);

	for(eDcapOffset = BHDM_P_Mhl_DevCapOffset_eDevStateAddr;
	    eDcapOffset < BHDM_P_Mhl_DevCapOffset_eDevCapAddrMax;
	    eDcapOffset++)
	{
		BHDM_P_Mhl_FieldDecode_isr(eDcapOffset, astDcap[eDcapOffset]);
	}
}

#else

void BHDM_P_Mhl_DumpCmdPkts_isr
	( BHDM_P_Mhl_CbusPkt          *pPackets,
	  uint32_t                     ulNumPackets,
	  BHDM_P_Mhl_CbusDest          eDestination )
{
	BSTD_UNUSED(pPackets);
	BSTD_UNUSED(ulNumPackets);
	BSTD_UNUSED(eDestination);
}

void BHDM_P_Mhl_DumpCmdQueue_isr
	( BHDM_P_Mhl_Req_Handle    hReq )
{
	BSTD_UNUSED(hReq);
}

void BHDM_P_Mhl_FieldDecode_isr
	( uint8_t    addr,
	  uint8_t    data )
{
	BSTD_UNUSED(addr);
	BSTD_UNUSED(data);
}

void BHDM_P_Mhl_MscMsgDecode_isr
	( uint8_t    action,
	  uint8_t    data )
{
	BSTD_UNUSED(action);
	BSTD_UNUSED(data);
}

void BHDM_P_Mhl_DumpEdid_isr
	( uint8_t   *pucEdidBlock,
	  uint32_t   ulEdidNumBlocks )
{
	BSTD_UNUSED(pucEdidBlock);
	BSTD_UNUSED(ulEdidNumBlocks);
}

void BHDM_P_Mhl_DumpMailbox_isr
	( BREG_Handle hRegister )
{
	BSTD_UNUSED(hRegister);
}

void BHDM_P_Mhl_DumpRegisters_isr
	( BREG_Handle hRegister,
	  uint32_t    ulRegBaseAddr,
	  uint32_t    ulRegCount )
{
	BSTD_UNUSED(hRegister);
	BSTD_UNUSED(ulRegBaseAddr);
	BSTD_UNUSED(ulRegCount);
}

void BHDM_P_Mhl_DumpSinkDcap_isr
	( BREG_Handle hRegister )
{
	BSTD_UNUSED(hRegister);

}

#endif
