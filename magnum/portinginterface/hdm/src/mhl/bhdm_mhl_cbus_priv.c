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

#include "bhdm_mhl_cbus_priv.h"
#include "bhdm_mhl_debug_priv.h"

BDBG_MODULE(BHDM_MHL_CBUS);
BDBG_OBJECT_ID(HDMI_MHL_CBUS);

void BHDM_P_Mhl_Cbus_Init
	( BREG_Handle       hRegister )
{
	uint32_t ulClkPeriod, ulCbusClkFreq = BHDM_P_MHL_CBUS_CLK_FREQ;
	uint32_t ulSyncMin, ulSyncMax;
	uint32_t ulData;

	ulCbusClkFreq /= 1000000; /* Now in MHz */
	ulClkPeriod = (1000 + (ulCbusClkFreq/2)) / ulCbusClkFreq;

	/* Tx Clock Generator */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_LINK_CFG_0);
	ulData &= ~(BCHP_MT_CBUS_LINK_CFG_0_TX_UI_PERIOD_MASK);
	ulData |= BCHP_FIELD_DATA(MT_CBUS_LINK_CFG_0, TX_UI_PERIOD, (ulCbusClkFreq/2));
	BREG_Write32(hRegister, BCHP_MT_CBUS_LINK_CFG_0, ulData);

	/* Rx CDR */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_RX_CDR_CFG_0);
	ulData &= ~(BCHP_MT_CBUS_RX_CDR_CFG_0_SAMPLING_OFFSET_MASK |
				BCHP_MT_CBUS_RX_CDR_CFG_0_FORCE_UI_PERIOD_MASK |
				BCHP_MT_CBUS_RX_CDR_CFG_0_FORCE_UI_PERIOD_ENABLE_MASK |
				BCHP_MT_CBUS_RX_CDR_CFG_0_EDGE_TOLERANCE_MASK);
	ulData |= (BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_0, SAMPLING_OFFSET, ulCbusClkFreq/4) |
				BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_0, FORCE_UI_PERIOD, ulCbusClkFreq/2) |
				BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_0, FORCE_UI_PERIOD_ENABLE, 0) |
				BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_0, EDGE_TOLERANCE, 3));
	BREG_Write32(hRegister, BCHP_MT_CBUS_RX_CDR_CFG_0, ulData);

	/* SYNC low minimum  = (3 min UI) - (duty cycle distortion) - (max jitter)
	   SYNC low maximum  = (3 max UI) + (duty cycle distortion) + (max jitter) */
	ulSyncMin = (3*400-  0-100) / ulClkPeriod;
	ulSyncMax = (3*600+  0+100) / ulClkPeriod;

	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_RX_CDR_CFG_1);
	ulData &= ~(BCHP_MT_CBUS_RX_CDR_CFG_1_SYNC_LOW_MIN_MASK |
				BCHP_MT_CBUS_RX_CDR_CFG_1_SYNC_LOW_MAX_MASK);
	ulData |= (BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_1, SYNC_LOW_MIN, ulSyncMin) |
				BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_1, SYNC_LOW_MAX, ulSyncMax));
	BREG_Write32(hRegister, BCHP_MT_CBUS_RX_CDR_CFG_1, ulData);

	/* SYNC high minimum = (1 min UI) - (duty cycle distortion) - (max jitter)
	   SYNC high maximum = (1 max UI) + (duty cycle distortion) + (max jitter) */
	ulSyncMin = (1*400- 0-100) / ulClkPeriod;
	ulSyncMax = (1*600+ 0+100) / ulClkPeriod;

	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_RX_CDR_CFG_2);
	ulData &= ~(BCHP_MT_CBUS_RX_CDR_CFG_2_SYNC_HIGH_MIN_MASK |
				BCHP_MT_CBUS_RX_CDR_CFG_2_SYNC_HIGH_MAX_MASK);
	ulData |= (BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_2, SYNC_HIGH_MIN, ulSyncMin) |
				BCHP_FIELD_DATA(MT_CBUS_RX_CDR_CFG_2, SYNC_HIGH_MAX, ulSyncMax));
	BREG_Write32(hRegister, BCHP_MT_CBUS_RX_CDR_CFG_2, ulData);

	/* Sink Arbitration high pulse minimum = (3 min bit time) - (duty cycle distortion) - (max jitter) */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_1);
	ulData &= ~BCHP_MT_CBUS_ARBITRATION_CFG_1_ARB_HIGH_MIN_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_ARBITRATION_CFG_1, ARB_HIGH_MIN, (3* 800-100-100) / ulClkPeriod);
	BREG_Write32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_1, ulData);

	/* Sink Arbitration low pulse minimum  = (1 min bit time) - (duty cycle distortion) - (max jitter) */
	/* Sink Arbitration low pulse maximum  = (1 max bit time) + (duty cycle distortion) + (max jitter) */

	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_2);
	ulData &= ~(BCHP_MT_CBUS_ARBITRATION_CFG_2_ARB_LOW_MIN_MASK |
				BCHP_MT_CBUS_ARBITRATION_CFG_2_ARB_LOW_MAX_MASK);
	ulData |= (BCHP_FIELD_DATA(MT_CBUS_ARBITRATION_CFG_2, ARB_LOW_MIN, (1* 800-100-100) / ulClkPeriod) |
				BCHP_FIELD_DATA(MT_CBUS_ARBITRATION_CFG_2, ARB_LOW_MAX, (1*1200+100+100) / ulClkPeriod));
	BREG_Write32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_2, ulData);

	/* Initiator */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_INITIATOR_CFG_0);
	ulData &= ~(BCHP_MT_CBUS_INITIATOR_CFG_0_MIN_ACK_DURATION_MASK |
				BCHP_MT_CBUS_INITIATOR_CFG_0_MAX_ACK_DURATION_MASK);
	ulData |= (BCHP_FIELD_DATA(MT_CBUS_INITIATOR_CFG_0, MIN_ACK_DURATION, 400 / ulClkPeriod) |
				BCHP_FIELD_DATA(MT_CBUS_INITIATOR_CFG_0, MAX_ACK_DURATION, 600 / ulClkPeriod));
	BREG_Write32(hRegister, BCHP_MT_CBUS_INITIATOR_CFG_0, ulData);

	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_INITIATOR_CFG_1);
	ulData &= ~BCHP_MT_CBUS_INITIATOR_CFG_1_MAX_ACK_WAIT_MASK;
	/* was 860, but some monitors reply ACK bit a bit late, e.g DELL */
	ulData |= BCHP_FIELD_DATA(MT_CBUS_INITIATOR_CFG_1, MAX_ACK_WAIT, 980 / ulClkPeriod);
	BREG_Write32(hRegister, BCHP_MT_CBUS_INITIATOR_CFG_1, ulData);

	/* Wake-up and Discovery */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_WAKE_CFG_0);
	ulData &= ~BCHP_MT_CBUS_WAKE_CFG_0_T_SRC_WAKE_START_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_WAKE_CFG_0, T_SRC_WAKE_START, 400);
	BREG_Write32(hRegister, BCHP_MT_CBUS_WAKE_CFG_0, ulData);

	/* T_SRC_WAKE_PULSE_WIDTH_1: min 18, typ 20, max 22ms */
	/* T_SRC_WAKE_PULSE_WIDTH_2: min 54, typ 60, max 66ms */
	/* T_SRC_WAKE_TO_DISCOVER:   min 100, max 1000ms
	   We set it a bit higher than 100 because hardware timer may not be accurate */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_WAKE_CFG_1);
	ulData &= ~(BCHP_MT_CBUS_WAKE_CFG_1_T_SRC_WAKE_PULSE_WIDTH_1_MASK |
				BCHP_MT_CBUS_WAKE_CFG_1_T_SRC_WAKE_PULSE_WIDTH_2_MASK |
				BCHP_MT_CBUS_WAKE_CFG_1_T_SRC_WAKE_TO_DISCOVER_MASK);
	ulData |= (BCHP_FIELD_DATA(MT_CBUS_WAKE_CFG_1, T_SRC_WAKE_PULSE_WIDTH_1, 20) |
				BCHP_FIELD_DATA(MT_CBUS_WAKE_CFG_1, T_SRC_WAKE_PULSE_WIDTH_2, 60) |
				BCHP_FIELD_DATA(MT_CBUS_WAKE_CFG_1, T_SRC_WAKE_TO_DISCOVER, 105));
	BREG_Write32(hRegister, BCHP_MT_CBUS_WAKE_CFG_1, ulData);

	/* T_SRC_CONN: max 240us */
	/* T_SRC_PULSE_WIDTH: min 80, typ 100, max 120us */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_DISCOVERY_CFG);
	ulData &= ~(BCHP_MT_CBUS_DISCOVERY_CFG_T_SRC_CONN_MASK |
				BCHP_MT_CBUS_DISCOVERY_CFG_T_SRC_PULSE_WIDTH_MASK);
	ulData |= (BCHP_FIELD_DATA(MT_CBUS_DISCOVERY_CFG, T_SRC_CONN, 100) |
				BCHP_FIELD_DATA(MT_CBUS_DISCOVERY_CFG, T_SRC_PULSE_WIDTH, 100));
	BREG_Write32(hRegister, BCHP_MT_CBUS_DISCOVERY_CFG, ulData);

	/* 3.3.12.1 fix - QD980 complained our T_REQ_HOLD after NACK is just shorter
	   than (7.99 instead of 8 bit period) as required by the spec. We set it to 9
	   bit period to make the test pass */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_0);
	ulData &= ~BCHP_MT_CBUS_ARBITRATION_CFG_0_T_REQ_HOLD_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_ARBITRATION_CFG_0, T_REQ_HOLD, 9);
	BREG_Write32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_0, ulData);

	/* CRDVP-747 workaround, forcing T_REQ_CONT to be zero for initator.
	H/W bug causes T_REQ_CONT to be more than maximum allowed (1 bit time) when two
	successive packets belong to different logical channels */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_0);
	ulData &= ~BCHP_MT_CBUS_ARBITRATION_CFG_0_T_REQ_CONT_INITIATOR_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_ARBITRATION_CFG_0, T_REQ_CONT_INITIATOR, 0);
	BREG_Write32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_0, ulData);

	/* TODO:  24 is FPGA outbound to inbound loopback latency */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_3);
	ulData &= ~BCHP_MT_CBUS_ARBITRATION_CFG_3_LOOPBACK_LATENCY_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_ARBITRATION_CFG_3, LOOPBACK_LATENCY, 42); /* Was 36 or 24*/
	BREG_Write32(hRegister, BCHP_MT_CBUS_ARBITRATION_CFG_3, ulData);

	/* T_DRV_HI_CBUS: min 100, max 400ns */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_LINK_CFG_0);
	ulData &= ~BCHP_MT_CBUS_LINK_CFG_0_T_DRV_HI_CBUS_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_LINK_CFG_0, T_DRV_HI_CBUS, 200/ulClkPeriod);
	BREG_Write32(hRegister, BCHP_MT_CBUS_LINK_CFG_0, ulData);

	/* T_SINK_CBUS_DISCONN: min 150us */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_LINK_CFG_1);
	ulData &= ~BCHP_MT_CBUS_LINK_CFG_1_T_DISCONNECT_IN_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_LINK_CFG_1, T_DISCONNECT_IN, 150);
	BREG_Write32(hRegister, BCHP_MT_CBUS_LINK_CFG_1, ulData);


	/* Set the inbound MSC ABORT timer to be slightly less than the spec
	   (2 seconds) so hardware will accept incoming MSC commands from
	   non-compliant sinks which send commands BEFORE the ABORT timer
	   expires. (The value set is Delta from 2 seconds, so 200 means
	   responder will accept incoming commands after 1800ms after ABORT.
	   Note that we will not send out any outbound MSC reqester packet
	   until the outbound MSC ABORT timer expires */
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_ABORT_CFG);
	ulData &= ~BCHP_MT_CBUS_ABORT_CFG_RESPONDER_DELAY_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_ABORT_CFG, RESPONDER_DELAY, 200);
	BREG_Write32(hRegister, BCHP_MT_CBUS_ABORT_CFG, ulData);


	/* CTS 6.3.10.6 failure workaround, increasing the MSC resonder timeout */
	ulData = BREG_Read32(hRegister, BCHP_MT_MSC_RESP_CFG_1);
	ulData &= ~BCHP_MT_MSC_RESP_CFG_1_REQUEST_TIMEOUT_MASK;
	ulData |= BCHP_FIELD_DATA(MT_MSC_RESP_CFG_1, REQUEST_TIMEOUT, 110);
	BREG_Write32(hRegister, BCHP_MT_MSC_RESP_CFG_1, ulData);

	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_SW_INIT);
	ulData &= ~BCHP_DVP_MT_AON_TOP_SW_INIT_CBUS_MASK;
	ulData |= BCHP_FIELD_DATA(DVP_MT_AON_TOP_SW_INIT, CBUS, 1);
	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_SW_INIT, ulData);

#if 1
		BKNI_Delay(675);
#else
		BKNI_Sleep(1); /* 675us in ARC600 */
#endif

	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_SW_INIT);
	ulData &= ~BCHP_DVP_MT_AON_TOP_SW_INIT_CBUS_MASK;
	ulData |= BCHP_FIELD_DATA(DVP_MT_AON_TOP_SW_INIT, CBUS, 0);
	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_SW_INIT, ulData);

	/* TODO: */
	BHDM_P_Mhl_Cbus_ClearAllInt_isr(hRegister);
}


/* Read the interrupt status registers */
bool BHDM_P_Mhl_Cbus_GetIntrStatus
	( BREG_Handle          hRegister,
	  BHDM_P_Mhl_CbusIntr *pIntrStat )
{
	pIntrStat->ulStatus0 = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_STATUS);
	pIntrStat->ulStatus1 = BREG_Read32(hRegister, BCHP_CBUS_INTR2_1_CPU_STATUS);

	/* This is just a short cut to see if there is any interrupts at all */
	return (pIntrStat->ulStatus0 || pIntrStat->ulStatus1) ? true : false;
}

/* Clear all interrupts */
void BHDM_P_Mhl_Cbus_ClearAllInt_isr
	( BREG_Handle          hRegister )
{
	uint32_t ulClearMask = 0xffffffff;
	uint32_t ulData;

	BREG_Write32(hRegister, BCHP_CBUS_INTR2_0_CPU_CLEAR, ulClearMask);
	BREG_Write32(hRegister, BCHP_CBUS_INTR2_1_CPU_CLEAR, ulClearMask);
#if BHDM_MHL_ENABLE_DEBUG
	{
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE("Clearing all interrupts.\n");
	}
#else
	BDBG_MSG(("Clearing all interrupts."));
#endif
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_INITIATOR_CFG_1);
	ulData &= ~(BCHP_MT_CBUS_INITIATOR_CFG_1_CLEAR_STATUS_MASK);
	ulData |= BCHP_FIELD_DATA(MT_CBUS_INITIATOR_CFG_1, CLEAR_STATUS, 1);
	BREG_Write32(hRegister, BCHP_MT_CBUS_INITIATOR_CFG_1, ulData);
	ulData |= BCHP_FIELD_DATA(MT_CBUS_INITIATOR_CFG_1, CLEAR_STATUS, 0);
	BREG_Write32(hRegister, BCHP_MT_CBUS_INITIATOR_CFG_1, ulData);

	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_FOLLOWER_CFG);
	ulData &= ~(BCHP_MT_CBUS_FOLLOWER_CFG_CLEAR_STATUS_MASK);
	ulData |= BCHP_FIELD_DATA(MT_CBUS_FOLLOWER_CFG, CLEAR_STATUS, 1);
	BREG_Write32(hRegister, BCHP_MT_CBUS_FOLLOWER_CFG, ulData);
	ulData |= BCHP_FIELD_DATA(MT_CBUS_FOLLOWER_CFG, CLEAR_STATUS, 0);
	BREG_Write32(hRegister, BCHP_MT_CBUS_FOLLOWER_CFG, ulData);

}

void BHDM_P_Mhl_Cbus_ClearInt_isr
	( BREG_Handle           hRegister,
	  BHDM_P_Mhl_CbusIntr  *pIntStat )
{
	uint32_t ulStatus0, ulStatus1;
	uint32_t ulCbusIntr0IgnoreMask, ulCbusIntr1IgnoreMask;

	/* Define these masks if we want to mask some intrs
	   in the log. The interrupts will always be cleared.
	   A mask of 0x0 means all intrs will be logged.
	   An example of intr0 mask might be:

	   CBUS_INTR2_0_CPU_STATUS_INITIATOR_PKT_SET |
	   CBUS_INTR2_0_CPU_STATUS_FOLLOWER_PKT_SET

	   which means initiator packet/follower packet interrupts
	   are not logged but will still be cleared.
	*/

#ifndef BHDM_P_MHL_CBUS_INTR0_LOG_IGNORE_MASK
#define BHDM_P_MHL_CBUS_INTR0_LOG_IGNORE_MASK 0x0
#endif
#ifndef BHDM_P_MHL_CBUS_INTR1_LOG_IGNORE_MASK
#define BHDM_P_MHL_CBUS_INTR1_LOG_IGNORE_MASK 0x0
#endif


	/* Note that we just print the current intr values out, not
	   updating the values stored in pIntStat */
	ulStatus0 = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_STATUS);
	ulStatus1 = BREG_Read32(hRegister, BCHP_CBUS_INTR2_1_CPU_STATUS);


	/* If only the following interrupts are fired,
	   we don't print the log out, set both of these
	   to zero if we want to log all interrupts fired */
	ulCbusIntr0IgnoreMask = BHDM_P_MHL_CBUS_INTR0_LOG_IGNORE_MASK;

	ulCbusIntr1IgnoreMask = BHDM_P_MHL_CBUS_INTR1_LOG_IGNORE_MASK;

	if((pIntStat->ulStatus0 & ~ulCbusIntr0IgnoreMask) || (ulStatus0 & ~ulCbusIntr0IgnoreMask))
	{
#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[80] = "";
		char acTemp[10] = "";
		strcat(acStr, "Current INTR_0:0x");
		sprintf(acTemp, "%X ", ulStatus0);
		strcat(acStr, acTemp);
		strcat(acStr, "  Clearing INTR_0:0x");
		sprintf(acTemp, "%X", pIntStat->ulStatus0);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
		BDBG_MSG(("Current INTR_0:0x%x Clearing INTR_0:0x%x", ulStatus0, pIntStat->ulStatus0));
#endif
	}

	if(pIntStat->ulStatus0)
	{
		BREG_Write32(hRegister, BCHP_CBUS_INTR2_0_CPU_CLEAR, pIntStat->ulStatus0);
	}

	if((pIntStat->ulStatus1 & ~ulCbusIntr1IgnoreMask) || (ulStatus1 & ~ulCbusIntr1IgnoreMask))
	{
#if BHDM_MHL_ENABLE_DEBUG
	{
		char acStr[80] = "";
		char acTemp[10] = "";
		strcat(acStr, "Current INTR_1:0x");
		sprintf(acTemp, "%X ", ulStatus1);
		strcat(acStr, acTemp);
		strcat(acStr, "  Clearing INTR_1:0x");
		sprintf(acTemp, "%X", pIntStat->ulStatus1);
		strcat(acStr, acTemp);
		strcat(acStr, "\n");
		BHDM_P_MHL_DEBUG_DUMP_TO_FILE(acStr);
	}
#else
		BDBG_MSG(("Current INTR_1:0x%x Clearing INTR_1:0x%x", ulStatus1, pIntStat->ulStatus1));
#endif
	}

	if(pIntStat->ulStatus1)
	{
		BREG_Write32(hRegister, BCHP_CBUS_INTR2_1_CPU_CLEAR, pIntStat->ulStatus1);
	}
}


/* packing/unpacking packet to and from a format in the packet register */
void BHDM_P_Mhl_Cbus_PktPack_isr
	( uint16_t                    *pusPktVal,
	  BHDM_P_Mhl_CbusPktType       ePktType,
	  BHDM_P_Mhl_CbusPktDirection  ePktDir,
	  uint8_t                      ucPktData )
{
	*pusPktVal = ((ePktType & BHDM_P_MHL_CBUS_PKT_TYPE_MASK) << BHDM_P_MHL_CBUS_PKT_TYPE_OFFSET) |
				  ((ePktDir & BHDM_P_MHL_CBUS_PKT_DIR_MASK) << BHDM_P_MHL_CBUS_PKT_DIR_OFFSET) |
				  ((ucPktData & BHDM_P_MHL_CBUS_PKT_DATA_MASK) << BHDM_P_MHL_CBUS_PKT_DATA_OFFSET);
}

void BHDM_P_Mhl_Cbus_PktUnpack_isr
	( uint16_t                     usPktVal,
	  BHDM_P_Mhl_CbusPktType      *pePktType,
	  BHDM_P_Mhl_CbusPktDirection *pePktDir,
	  uint8_t                     *pucPktData )
{
		*pePktType = (usPktVal >> BHDM_P_MHL_CBUS_PKT_TYPE_OFFSET) & BHDM_P_MHL_CBUS_PKT_TYPE_MASK;
		*pePktDir = (usPktVal >> BHDM_P_MHL_CBUS_PKT_DIR_OFFSET) & BHDM_P_MHL_CBUS_PKT_DIR_MASK;
		*pucPktData = (usPktVal >> BHDM_P_MHL_CBUS_PKT_DATA_OFFSET) & BHDM_P_MHL_CBUS_PKT_DATA_MASK;
}

/*
	Receive a command from a particular FIFO.
	Basically this is intended for user to either
	just pass in the command which is just programmed
	into the hardware to extract the response.

	The following members of the command must be filled in:
		- outgoing commands/data and incoming commands
		- ulNumPacketsCfg
		- bLastCmd - this only affects the result
		- pucReplyBuf - if a reply is expected, this gets the incoming data
		- ucReplyBufValidSize - only if a reply is expected

	Basically just copy the command configured in the hardware

	This function will overwrite the array of packets in cmd
	and update ulNumPacketsDone and pucReplyBuf if a reply is found

	Call the next function below for fetching incoming requests
*/
bool BHDM_P_Mhl_Cbus_PacketMatched_isr
	( BHDM_P_Mhl_CbusPkt          *pPacket,
	  BHDM_P_Mhl_CbusPktDirection  ePktDir,
	  BHDM_P_Mhl_CbusPktType       ePktType,
	  uint8_t                      ucPktData )
{
	/* We ignore the incoming data packet
	   returns true if what we read from FIFO is
	   want we expect */
	if (pPacket->ulDir != ePktDir || pPacket->ulType != ePktType ||
		(pPacket->ulDir == BHDM_P_Mhl_CbusPktDirection_eResp &&
		pPacket->ulType == BHDM_P_Mhl_CbusPktType_eCtrl &&
		pPacket->ucData != ucPktData))
	{
		return false;
	}

   return true;
}

void BHDM_P_Mhl_Cbus_SendPkts_isr
	( BREG_Handle                  hRegister,
	  uint32_t                     ulOutboundFifoReg,
	  BHDM_P_Mhl_CbusCmd          *pCmd,
	  BHDM_P_Mhl_CbusCmdQueueType  eQueueType )
{
	uint32_t i, ulBaseRegAddr = ulOutboundFifoReg;
	BHDM_P_Mhl_CbusPkt *pPkts = NULL;

	switch (eQueueType)
	{
		case BHDM_P_Mhl_CbusCmdQueueType_eShort:
			pPkts = &pCmd->cbusPackets.astShortCmd[0];
			break;
		case BHDM_P_Mhl_CbusCmdQueueType_eLong:
			pPkts = &pCmd->cbusPackets.astLongCmd[0];
			break;
		default:
			break;
	}

	BDBG_ASSERT(pPkts);

	for(i = 0; i < pCmd->ulNumPacketsCfg; i+=2)
	{
		/* packets are packed as 16-bits within 32-bit register in the block,
		  so we have to write packets as a pair */
		uint16_t pkt_val16 = 0;
		uint32_t pkt_val32 = 0;

		BHDM_P_Mhl_Cbus_PktPack_isr(&pkt_val16, pPkts[i].ulType,
		pPkts[i].ulDir, pPkts[i].ucData);
		pkt_val32 = pkt_val16;

		if(pCmd->ulNumPacketsCfg - i > 1)
		{
			/* We have at least 1 more packet, so write the upper packet,
			   otherwise leave that as zero */
			BHDM_P_Mhl_Cbus_PktPack_isr(&pkt_val16, pPkts[i+1].ulType,
										pPkts[i+1].ulDir, pPkts[i+1].ucData);
			pkt_val32 |= (pkt_val16 << 16);
		}

		/* Now write to the outbound FIFO register */
		BREG_Write32(hRegister, ulBaseRegAddr, pkt_val32);

		ulBaseRegAddr += 4; /* Next FIFO register */
	} /* End of writing to h/w packet FIFO */
}


/* Returns true if CBUS command belongs to MSC channel */
bool BHDM_P_Mhl_Cbus_CmdIsMsc_isr
	( BHDM_P_Mhl_CbusCmd      *pCmd )
{
	return (pCmd && (pCmd->eCmdType & BHDM_P_MHL_CBUS_CMD_CHAN_MASK) == BHDM_P_MHL_CBUS_CMD_CHAN_MSC) ?
			true : false;
}

/* Returns true if CBUS command belongs to DDC channel */
bool BHDM_P_Mhl_Cbus_CmdIsDdc_isr
	( BHDM_P_Mhl_CbusCmd      *pCmd )
{
	return (pCmd && (pCmd->eCmdType & BHDM_P_MHL_CBUS_CMD_CHAN_MASK) == BHDM_P_MHL_CBUS_CMD_CHAN_DDC) ?
			true : false;
}

/* Execute disconnect sequence */
void BHDM_P_Mhl_Cbus_Disconnect_isr
	( BREG_Handle          hRegister )
{
	uint32_t ulData;

	ulData = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_CLEAR);
	ulData &= ~BCHP_CBUS_INTR2_0_CPU_CLEAR_DISCONNECT_DONE_MASK;
	ulData |= BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_CLEAR, DISCONNECT_DONE, 1);
	BREG_Write32(hRegister, BCHP_CBUS_INTR2_0_CPU_CLEAR, ulData);

	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_LINK_CTRL_0);
	ulData &= ~BCHP_MT_CBUS_LINK_CTRL_0_START_DISCONNECT_MASK;
	ulData |= BCHP_FIELD_DATA(MT_CBUS_LINK_CTRL_0, START_DISCONNECT, 1);
	BREG_Write32(hRegister, BCHP_MT_CBUS_LINK_CTRL_0, ulData);

	/* TODO: May not need below */
#if 0
	do
	{
		ulData = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_CLEAR);
		tmp = BCHP_GET_FIELD_DATA(ulData, CBUS_INTR2_0_CPU_CLEAR, DISCONNECT_DONE);
#if 1
		BKNI_Delay(675);
#else
		BKNI_Sleep(1); /* 675us in ARC600 */
#endif

	} while(!tmp);

	ulData = BREG_Read32(hRegister, BCHP_CBUS_INTR2_0_CPU_CLEAR);
	ulData &= ~BCHP_CBUS_INTR2_0_CPU_CLEAR_DISCONNECT_DONE_MASK;
	ulData |= BCHP_FIELD_DATA(CBUS_INTR2_0_CPU_CLEAR, DISCONNECT_DONE, 1);
	BREG_Write32(hRegister, BCHP_CBUS_INTR2_0_CPU_CLEAR, ulData);
#endif
}

/* Impedance measurement calibration */
void BHDM_P_Mhl_Cbus_PhyWaitRcalDone
	( BREG_Handle hRegister )
{
	uint32_t ulData;
	do
	{
		ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_RCAL_CBUS_PHY_STATUS);
		BKNI_Sleep(10);
	} while (BCHP_GET_FIELD_DATA(ulData, DVP_MT_AON_TOP_RCAL_CBUS_PHY_STATUS, RCAL_DONE) == 0);
}

/* Configure PHY for impedance measurement */
void BHDM_P_Mhl_Cbus_ConfigPhyImpDetectMode
	( BREG_Handle                  hRegister )
{

	uint32_t ulData;

	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL);
	ulData &= ~(BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_MODE_MASK |
			   BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_OV_PROTECT_ENB_MASK |
			   BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_BYPASS_ENB_MASK |
			   BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_IMP_DET_RST_MASK |
			   BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_RZM_EN_MASK |
			   BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_RX_EN_MASK);

	ulData |= (BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, MODE, 0) |           /* default=1 for hotplug */
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, OV_PROTECT_ENB, 0) |  /* default */
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_BYPASS_ENB, 1) |
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_IMP_DET_RST, 0) |/* default */
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_RZM_EN, 0) |     /* default */
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_RX_EN, 0));      /* default=1 for hotplug */

	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL, ulData);
}

/* Measure impedance */
bool BHDM_P_Mhl_Cbus_PhyMeasureImp_isr
	( BREG_Handle                  hRegister )
{
	bool bCbusZOk;
	uint32_t ulData;

	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL);
	ulData &= ~(BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_OV_PROTECT_ENB_MASK |
			   BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_IMP_DET_RST_MASK);
	ulData |= (BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, OV_PROTECT_ENB, 1) |
			   BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_IMP_DET_RST, 1));
	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL, ulData);

	BDBG_MSG(("Measuring CBUS impedance"));

	/* This actually starts the measurement */
	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL);
	ulData &= ~BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_RZM_EN_MASK;
	ulData |= BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_RZM_EN, 1);
	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL, ulData);

	/* Must read the impedance measurement before disabling the pull up Z */
	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_RCAL_CBUS_PHY_STATUS);
	bCbusZOk = BCHP_GET_FIELD_DATA(ulData, DVP_MT_AON_TOP_RCAL_CBUS_PHY_STATUS, CBUS_ZOK) ? true : false;

	/* Finally, disable the pull up Z */
	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL);
	ulData &= ~BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_RZM_EN_MASK;
	ulData |= BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_RZM_EN, 0);
	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL, ulData);

	return bCbusZOk;
}

/* Call this after impedance measurement but
   BEFORE wakeup and discovery */
void BHDM_P_Mhl_Cbus_ConfigPhyNormalMode
	( BREG_Handle hRegister )
{
	uint32_t ulData;

	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL);
	ulData &= ~(BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_MODE_MASK |
				BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_OV_PROTECT_ENB_MASK |
				BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_BYPASS_ENB_MASK |
				BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_IMP_DET_RST_MASK |
				BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_RZM_EN_MASK |
				BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL_CBUS_RX_EN_MASK);

	ulData |= (BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, MODE, 0) |
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, OV_PROTECT_ENB, 1) |
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_BYPASS_ENB, 1) |
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_IMP_DET_RST, 1) |
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_RZM_EN, 0) |
			  BCHP_FIELD_DATA(DVP_MT_AON_TOP_CBUS_PHY_CTL, CBUS_RX_EN, 1));

	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_CBUS_PHY_CTL, ulData);
}

#if BHDM_MHL_CTS
/* Analog PHY initialisation */
void BHDM_P_Mhl_Cbus_PhyInit
	( BREG_Handle hRegister )
{
	uint32_t ulData;

	ulData = BREG_Read32(hRegister, BCHP_DVP_MT_AON_TOP_RCAL_PHY_CTL);
	ulData &= ~(BCHP_DVP_MT_AON_TOP_RCAL_PHY_CTL_POWER_DOWN_MASK |
				BCHP_DVP_MT_AON_TOP_RCAL_PHY_CTL_RESETB_MASK);

	ulData |= (BCHP_FIELD_DATA(DVP_MT_AON_TOP_RCAL_PHY_CTL, POWER_DOWN, 0) |
			   BCHP_FIELD_DATA(DVP_MT_AON_TOP_RCAL_PHY_CTL, RESETB, 0)) ;

	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_RCAL_PHY_CTL, ulData);

	ulData |= BCHP_FIELD_DATA(DVP_MT_AON_TOP_RCAL_PHY_CTL, RESETB, 1) ;

	BREG_Write32(hRegister, BCHP_DVP_MT_AON_TOP_RCAL_PHY_CTL, ulData);
}


/* Execute wakeup sequence, set timeout to zero if
   the user wants the function to return only
   if discovery is successful, otherwise the function
   returns after timeout ms has passed.

   Arguments: timeout in ms, discovery status

   timeout should be at least 2000ms)
*/
void BHDM_P_Mhl_Cbus_WakeupAndDiscovery_isr
	( BREG_Handle                  hRegister,
	  uint32_t                     ulDiscoveryTimeout,
	  BHDM_P_Mhl_CbusDiscovery    *peDiscovery )
{
	const uint32_t ulMinTimeout = 2000;
	uint32_t ulTimeRemained = ulDiscoveryTimeout;
	struct cbus_intr_t intr_stat;
	uint32_t ulData;

	if (ulTimeRemained < ulMinTimeout)
		ulTimeRemained = ulMinTimeout;

	*peDiscovery = BHDM_P_Mhl_CbusDiscovery_eDisconnected;

start:
	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_LINK_CTRL_0);
	ulData &= ~(BCHP_MT_CBUS_LINK_CTRL_0_START_WAKE_UP_MASK);
	ulData |= BCHP_FIELD_DATA(MT_CBUS_LINK_CTRL_0, START_WAKE_UP, 1);
	BREG_Write32(hRegister, BCHP_MT_CBUS_LINK_CTRL_0, ulData);

	do
	{
		mt_sleep(1*ARC600_TIMER_US_MS_FACTOR);
		if(time_remained)
			time_remained--;
		cbus_common_get_intr(reg_bank, &intr_stat);

		if(ulDiscoveryTimeout && ulTimeRemained == 0)
			break;
	} while(!CBUS_INTR_SET(0, intr_stat.status0, DISCOVERY_FAILED) &&
		!CBUS_INTR_SET(0, intr_stat.status0, DISCOVERY_SUCCEEDED));

	if (ulDiscoveryTimeout == 0 && CBUS_INTR_SET(0, intr_stat.status0, DISCOVERY_FAILED))
	{
		mt_sleep(100*ARC600_TIMER_US_MS_FACTOR);
		goto start; /* Try again */
	}

	if (CBUS_INTR_SET(0,intr_stat.status0, DISCOVERY_SUCCEEDED))
		*peDiscovery = BHDM_P_Mhl_CbusDiscovery_eSucceeded;
	else if(CBUS_INTR_SET(0, intr_stat.status0, DISCOVERY_FAILED))
		*peDiscovery = BHDM_P_Mhl_CbusDiscovery_eFailed;
	else
		*peDiscovery = BHDM_P_Mhl_CbusDiscovery_eTimeout;
}

/* Current discovery status */
void BHDM_P_Mhl_Cbus_GetDiscoveryStatus
	( BREG_Handle               hRegister,
	  BHDM_P_Mhl_CbusDiscovery *peDiscovery )
{
	uint32_t ulData;

	/* Unless link status indicates the link is connected,
	   we just treat the link as disconnected */

	ulData = BREG_Read32(hRegister, BCHP_MT_CBUS_LINK_STATUS);
	if (BCHP_GET_FIELD_DATA(ulData, MT_CBUS_LINK_STATUS, BUS_STATE) >= BHDM_P_Mhl_CbusLinkState_eDiscovery_eSucceeded)
	{
		*peDiscovery = BHDM_P_Mhl_CbusDiscovery_eSucceeded;
	}
	else
	{
		*peDiscovery = BHDM_P_Mhl_CbusDiscovery_eDisconnected;
	}
}
#endif
