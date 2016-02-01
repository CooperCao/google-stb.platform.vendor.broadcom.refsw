/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
 
#ifndef _BAST_G2_PRIV_FSK_H__
#define _BAST_G2_PRIV_FSK_H__

#define BAST_FSK_WRITE(r, v) \
   { BREG_Write32(((BAST_g2_P_Handle *)(h->pImpl))->hRegister, BCHP_##r, v); }

#define BAST_FSK_READ(r, v) \
   { *v = BREG_Read32(((BAST_g2_P_Handle *)(h->pImpl))->hRegister, BCHP_##r); }

#define BAST_INT_CB_DEF(intr, id, enable) \
	{intr, BAST_g2_Ftm_P_HandleInterrupt_isr, id, enable},


#define BCHP_INT_ID_FTM_UART            BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_UART_IM_SHIFT)
#define BCHP_INT_ID_FTM_UART_TX         BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_UART_TX_IM_SHIFT)
#define BCHP_INT_ID_FTM_UART_END        BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_UART_END_IM_SHIFT)
#define BCHP_INT_ID_FTM_UART_ERR        BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_UART_ERR_IM_SHIFT)
#define BCHP_INT_ID_FTM_RSSI_RISE       BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_RSSI_RISE_IM_SHIFT)
#define BCHP_INT_ID_FTM_RSSI_FALL       BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_RSSI_FALL_IM_SHIFT)
#define BCHP_INT_ID_FTM_CORR_STATE      BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_CORR_STATE_IM_SHIFT)
#define BCHP_INT_ID_FTM_CORR_BYTE       BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_CORR_BYTE_IM_SHIFT)
#define BCHP_INT_ID_FTM_CORR_TIMEOUT    BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_CORR_TIMEOUT_IM_SHIFT)
#define BCHP_INT_ID_FTM_CORR_AGC        BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_CORR_AGC_IM_SHIFT)
#define BCHP_INT_ID_FTM_CORR_PREAMBLE   BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_CORR_PREAMBLE_IM_SHIFT)
#define BCHP_INT_ID_FTM_TIMER1          BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_TIMER1_IM_SHIFT)
#define BCHP_INT_ID_FTM_TIMER2          BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_TIMER2_IM_SHIFT)
#define BCHP_INT_ID_FTM_TIMER3          BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_TIMER3_IM_SHIFT)
#define BCHP_INT_ID_FTM_ASSIST_IDLE     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_ASSIST_IDLE_IM_SHIFT)
#define BCHP_INT_ID_FTM_RX_PKT_RDY      BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_RX_PKT_RDY_IM_SHIFT)
#define BCHP_INT_ID_FTM_ASSIST_LVL1     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_ASSIST_LVL1_IM_SHIFT)
#define BCHP_INT_ID_FTM_ASSIST_LVL2     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_ASSIST_LVL2_IM_SHIFT)
#define BCHP_INT_ID_FTM_ASSIST_LVL3     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_ASSIST_LVL3_IM_SHIFT)
#define BCHP_INT_ID_FTM_ASSIST_LVL4     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_ASSIST_LVL4_IM_SHIFT)
#define BCHP_INT_ID_FTM_HB_PKT_SENT     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_HB_PKT_SENT_IM_SHIFT)
#define BCHP_INT_ID_FTM_SP_PKT_SENT     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_SP_PKT_SENT_IM_SHIFT)
#define BCHP_INT_ID_FTM_ASSIST_ERR      BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_ASSIST_ERR_IM_SHIFT)
#define BCHP_INT_ID_FTM_RX_PKT_RDY_CNT  BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_RX_PKT_RDY_CNT_IM_SHIFT)
#define BCHP_INT_ID_FTM_FPKT_CNT        BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_FPKT_CNT_IM_SHIFT)
#define BCHP_INT_ID_FTM_FBAD_CRC_CNT    BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_FBAD_CRC_CNT_IM_SHIFT)
#define BCHP_INT_ID_FTM_FRX_MSK_NM_CNT  BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_FRX_MSK_NM_CNT_IM_SHIFT)
#define BCHP_INT_ID_FTM_FXCLUDE_CNT     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_FXCLUDE_CNT_IM_SHIFT)
#define BCHP_INT_ID_FTM_RX_BIT_MASK_CHG BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_RX_BIT_MASK_CHG_IM_SHIFT)
#define BCHP_INT_ID_FTM_SER_NUM_CHG     BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_SER_NUM_CHG_IM_SHIFT)
#define BCHP_INT_ID_FTM_RX_TYPE1_CNT    BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_RX_TYPE1_CNT_IM_SHIFT)
#define BCHP_INT_ID_FTM_RX_TYPE2_CNT    BCHP_INT_ID_CREATE(BCHP_FTM_INTR2_CPU_STATUS, BCHP_FTM_PHY_IRQ_MSK_RX_TYPE2_CNT_IM_SHIFT)


/******************************************************************************
FSK framing IDs
******************************************************************************/
#define BAST_FSK_FRAME_FIRST_SEND_REPLY_REQUIRED   0xE2
#define BAST_FSK_FRAME_REPEAT_SEND_REPLY_REQUIRED  0xE3
#define BAST_FSK_FRAME_REPLY_OK                    0xE4
#define BAST_FSK_FRAME_ERR_INVALID_CMD             0xE5
#define BAST_FSK_FRAME_ERR_INVALID_LENGTH          0xE7
#define BAST_FSK_FRAME_ERR_INVALID_STB             0xED
#define BAST_FSK_FRAME_TIMING_LNB                  0xEC
#define BAST_FSK_FRAME_INVALID_CRC                 0xEE


/******************************************************************************
FSK command IDs
******************************************************************************/
#define BAST_FSK_CMD_TUNING       0x73
#define BAST_FSK_CMD_STATUS       0x74
#define BAST_FSK_CMD_CONFIG       0x78


/******************************************************************************
bit definitions for SFtmDevice.status
******************************************************************************/
#define BAST_FSK_STATUS_RESET              0x0000
#define BAST_FSK_STATUS_TX_IN_PROGRESS     0x0001
#define BAST_FSK_STATUS_RX_IN_PROGRESS     0x0002
//#define BAST_FTM_STATUS_START_TX           0x0010 /* indicate manual transmit pending */
//#define BAST_FTM_STATUS_TX_BUSY            0x0038


/******************************************************************************
bit definitions for SFtmDevice.buf_status
******************************************************************************/
#define BAST_FSK_BUF_ALL_EMPTY        0x00
#define BAST_FSK_BUF_TX_NOT_EMPTY     0x01
#define BAST_FSK_BUF_RX_NOT_EMPTY     0x02


/******************************************************************************
bit definitions for SFtmDevice.err_status
******************************************************************************/
#define BAST_FSK_ERR_NONE             0x00
//#define BAST_FSK_ERR_HOST_MSG_DROPPED 0x01
//#define BAST_FTM_ERR_RX_PKT_DROPPED   0x02
#define BAST_FSK_ERR_TX_PKT_DROPPED   0x04


/* timer enumeration */
typedef enum BAST_Fsk_TimerSelect{
   BAST_Fsk_TimerSelect_eRx = 0,
   BAST_Fsk_TimerSelect_eTx,
   BAST_Fsk_TimerSelect_eTimeCount
} BAST_Fsk_TimerSelect;


/* interrupt enumeration */
typedef enum BAST_Fsk_IntID{
   BAST_Fsk_IntID_eUart = 0,
   BAST_Fsk_IntID_eUartTx,
   BAST_Fsk_IntID_eUartEnd,
   BAST_Fsk_IntID_eUartErr,
   BAST_Fsk_IntID_eRssiRise,
   BAST_Fsk_IntID_eRssiFall,
   BAST_Fsk_IntID_eCorrState,
   BAST_Fsk_IntID_eCorrByte,
   BAST_Fsk_IntID_eCorrTimeout,
   BAST_Fsk_IntID_eCorrAgc,
   BAST_Fsk_IntID_eCorrPreamble,
   BAST_Fsk_IntID_eTimer1,
   BAST_Fsk_IntID_eTimer2,
   BAST_Fsk_IntID_eTimer3,
   BAST_Fsk_IntID_eAssistIdle,
   BAST_Fsk_IntID_eRxPktRdy,
   BAST_Fsk_IntID_eAssistLvl1,
   BAST_Fsk_IntID_eAssistLvl2,
   BAST_Fsk_IntID_eAssistLvl3,
   BAST_Fsk_IntID_eAssistLvl4,
   BAST_Fsk_IntID_eHbPktSent,
   BAST_Fsk_IntID_eSpPktSent,
   BAST_Fsk_IntID_eAssistErr,
   BAST_Fsk_IntID_eRxPktRdyCnt,
   BAST_Fsk_IntID_eFPktCnt,
   BAST_Fsk_IntID_eFBadCrcCnt,
   BAST_Fsk_IntID_eFRxMskNmCnt,
   BAST_Fsk_IntID_eFXcludeCnt,
   BAST_Fsk_IntID_eRxBitMaskChg,
   BAST_Fsk_IntID_eSerNumChg,
   BAST_Fsk_IntID_eRxType1Cnt,
   BAST_Fsk_IntID_eRxType2Cnt
} BAST_Fsk_IntID;


#endif   /* BAST_G2_PRIV_FSK_H__ */
