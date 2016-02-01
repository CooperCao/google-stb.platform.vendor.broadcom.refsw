/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef _BAST_G2_PRIV_FTM_H__
#define _BAST_G2_PRIV_FTM_H__


#define BAST_FTM_MAJOR_VERSION 0x01
#define BAST_FTM_MINOR_VERSION 0x04
#define BAST_FTM_BUILD_NUM     0x000F


#define BAST_FTM_WRITE(r, v) \
   { BREG_Write32(((BAST_g2_P_Handle *)(h->pImpl))->hRegister, BCHP_##r, v); }

#define BAST_FTM_READ(r, v) \
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

#define BAST_FTM_MAX_NO_POLL_CNT 10

/* FTM timeout parameters */
#define BAST_FTM_TIMEOUT_RX  1500000 /* 1500 secs */
#define BAST_FTM_TIMEOUT_PKT  143000 /* 150 - 5 - (8bytes=2msecs) = 143 msecs */
#define BAST_FTM_TIMEOUT_REG 1500000 /* 1500 msecs */
#define BAST_FTM_TIMEOUT_HARD_RESET 1000000 /* 1000 msecs */


/******************************************************************************
FTM network packet IDs 
******************************************************************************/
#define BAST_FTM_PKT_RESP_FLAG         0x40

#define BAST_FTM_PKT_REG_REQUEST       0x01
#define BAST_FTM_PKT_XTUNE             0x05
#define BAST_FTM_PKT_HEARTBEAT         0x06
#define BAST_FTM_PKT_FTM_UP            0x08
#define BAST_FTM_PKT_LOCK              0x0E
#define BAST_FTM_PKT_UNLOCK            0x0F
#define BAST_FTM_PKT_REG_OFFER         0x14
#define BAST_FTM_PKT_ECHO              0x17
#define BAST_FTM_PKT_PING              0x19

#define BAST_FTM_PKT_REG_RESPONSE      (BAST_FTM_PKT_RESP_FLAG | BAST_FTM_PKT_REG_REQUEST)
#define BAST_FTM_PKT_XTUNE_RESP        (BAST_FTM_PKT_RESP_FLAG | BAST_FTM_PKT_XTUNE)
#define BAST_FTM_PKT_POLL              (BAST_FTM_PKT_RESP_FLAG | BAST_FTM_PKT_HEARTBEAT)
#define BAST_FTM_PKT_LOCK_RESP         (BAST_FTM_PKT_RESP_FLAG | BAST_FTM_PKT_LOCK)
#define BAST_FTM_PKT_UNLOCK_RESP       (BAST_FTM_PKT_RESP_FLAG | BAST_FTM_PKT_UNLOCK)
#define BAST_FTM_PKT_ECHO_RESP         (BAST_FTM_PKT_RESP_FLAG | BAST_FTM_PKT_ECHO)
#define BAST_FTM_PKT_PING_RESP         (BAST_FTM_PKT_RESP_FLAG | BAST_FTM_PKT_PING)

#define BAST_FTM_PKT_NACK_BAD_ADDRESS  0x32
#define BAST_FTM_PKT_NACK_BAD_CRC      0x33
#define BAST_FTM_PKT_NACK_BUSY         0x34
#define BAST_FTM_PKT_RE_REGISTER       0x35
#define BAST_FTM_PKT_NACK_HW_FAIL      0x36
#define BAST_FTM_PKT_NACK_UNSUPPORTED  0x37
#define BAST_FTM_PKT_NO_MORE_ADDRESSES 0x38


/******************************************************************************
FTM local message IDs 
******************************************************************************/
#define BAST_FTM_MSG_NOP               0x00
#define BAST_FTM_MSG_SET_RX_BIT_MASK   0x02
#define BAST_FTM_MSG_GET_RX_BIT_MASK   0x03
#define BAST_FTM_MSG_SET_OPTIONS       0x04
#define BAST_FTM_MSG_GET_OPTIONS       0x05
#define BAST_FTM_MSG_GET_VERSION       0x06
#define BAST_FTM_MSG_RESET_UC          0x07
#define BAST_FTM_MSG_RESET_FTM         0x08
#define BAST_FTM_MSG_GET_EXT_VERSION   0x0B
#define BAST_FTM_MSG_MONITOR_POLLS     0x0C

#define BAST_FTM_MSG_ERR_BAD_PKT_RCVD  0x81
#define BAST_FTM_MSG_ERR_BAD_PKT_SENT  0x82
#define BAST_FTM_MSG_ERR_INVALID_MSG   0x83
#define BAST_FTM_MSG_ERR_NOT_AVAILABLE 0x87
#define BAST_FTM_MSG_ERR_NOT_DETECTED  0x88
#define BAST_FTM_MSG_ERR_REREGISTER    0x89
#define BAST_FTM_MSG_ERR_BUSY          0x8A


/******************************************************************************
bit definitions for SFtmDevice.status
******************************************************************************/
#define BAST_FTM_STATUS_RESET              0x0000
#define BAST_FTM_STATUS_REG_REQ            0x0001  /* received reg request */ 
#define BAST_FTM_STATUS_REG_WAIT           0x0002  /* waiting after reg offer */
#define BAST_FTM_STATUS_REG_RESP           0x0004  /* waiting for reg response */
#define BAST_FTM_STATUS_REG_PENDING        (BAST_FTM_STATUS_REG_RESP | BAST_FTM_STATUS_REG_WAIT | BAST_FTM_STATUS_REG_REQ)
#define BAST_FTM_STATUS_HARD_RESET         0x0008  /* hard reset pending */
#define BAST_FTM_STATUS_START_TX           0x0010  /* indicate manual transmit pending */
#define BAST_FTM_STATUS_TX_IN_PROGRESS     0x0020  /* indicate manual transmit in progress */
#define BAST_FTM_STATUS_TX_BUSY            (BAST_FTM_STATUS_TX_IN_PROGRESS | BAST_FTM_STATUS_START_TX | BAST_FTM_STATUS_HARD_RESET)
#define BAST_FTM_STATUS_PING_RESP          0x0040
#define BAST_FTM_STATUS_POLL_PENDING       0x0080
#define BAST_FTM_STATUS_XTUNE_RESP         0x0100  /* waiting for xtune resp */
#define BAST_FTM_STATUS_NETWORK_XTUNE_RESP 0x0200  /* waiting for network xtune resp */
#define BAST_FTM_STATUS_REG_TIMEOUT        0x0400  /* registration timed out when tx in progress */
#define BAST_FTM_STATUS_POLL_MONITOR_MODE  0x0800  /* poll monitoring mode */


/******************************************************************************
bit definitions for SFtmDevice.buf_status
******************************************************************************/
#define BAST_FTM_BUF_ALL_EMPTY        0x00
#define BAST_FTM_BUF_TX_NOT_EMPTY     0x01
#define BAST_FTM_BUF_RX_NOT_EMPTY     0x02
#define BAST_FTM_BUF_HOST_NOT_EMPTY   0x04
#define BAST_FTM_BUF_ERR_NOT_EMPTY    0x08

/******************************************************************************
bit definitions for SFtmDevice.err_status
******************************************************************************/
#define BAST_FTM_ERR_NONE             0x00
#define BAST_FTM_ERR_HOST_MSG_DROPPED 0x01
#define BAST_FTM_ERR_RX_PKT_DROPPED   0x02
#define BAST_FTM_ERR_TX_PKT_DROPPED   0x04
#define BAST_FTM_ERR_ERR_MSG_DROPPED  0x08


/******************************************************************************
bit definitions for SFtmDevice.options
******************************************************************************/
#define BAST_FTM_OPTION_LOOPBACK           0x0001 /* not implemented */
#define BAST_FTM_OPTION_DISABLE_PKT_VERIFY 0x0002 /* not implemented */
#define BAST_FTM_OPTION_FORWARD_BAD_PKTS   0x0004
#define BAST_FTM_OPTION_VALIDATE_OUT_PKTS  0x0008 /* not implemented */
#define BAST_FTM_OPTION_INSERT_CRC         0x0010
#define BAST_FTM_OPTION_EDGE_INTR          0x0020 /* not implemented */
#define BAST_FTM_OPTION_HIGH_POLARITY_INTR 0x0040 /* not implemented */
#define BAST_FTM_OPTION_NETWORK_ACTIVITY   0x0080
#define BAST_FTM_OPTION_PROMISCUOUS        0x0100
#define BAST_FTM_OPTION_CARRIER_ON         0x0200
#define BAST_FTM_OPTION_PREAMBLE_DETECTED  0x0400
#define BAST_FTM_OPTION_DSWM13_DETECTED    0x0800
#define BAST_FTM_OPTION_SEND_RAW           0x4000 /* removed */
#define BAST_FTM_OPTION_NON_POLLED         0x8000
#define BAST_FTM_OPTIONS_IMPLEMENTED       0x8F94

/******************************************************************************
 FTM irq mask 
******************************************************************************/
#define BAST_FTM_IRQ_UART            0x00000001
#define BAST_FTM_IRQ_UART_TX         0x00000002
#define BAST_FTM_IRQ_UART_END        0x00000004
#define BAST_FTM_IRQ_UART_ERR        0x00000008
#define BAST_FTM_IRQ_RSSI_RISE       0x00000010
#define BAST_FTM_IRQ_RSSI_FALL       0x00000020
#define BAST_FTM_IRQ_CORR_STATE      0x00000040
#define BAST_FTM_IRQ_CORR_BYTE       0x00000080
#define BAST_FTM_IRQ_CORR_TIMEOUT    0x00000100
#define BAST_FTM_IRQ_CORR_AGC        0x00000200
#define BAST_FTM_IRQ_CORR_PREAMBLE   0x00000400
#define BAST_FTM_IRQ_TIMER1          0x00000800
#define BAST_FTM_IRQ_TIMER2          0x00001000
#define BAST_FTM_IRQ_TIMER3          0x00002000
#define BAST_FTM_IRQ_ASSIST_IDLE     0x00004000
#define BAST_FTM_IRQ_RX_PKT_RDY      0x00008000
#define BAST_FTM_IRQ_ASSIST_LVL1     0x00010000
#define BAST_FTM_IRQ_ASSIST_LVL2     0x00020000
#define BAST_FTM_IRQ_ASSIST_LVL3     0x00040000
#define BAST_FTM_IRQ_ASSIST_LVL4     0x00080000
#define BAST_FTM_IRQ_HB_PKT_SENT     0x00100000
#define BAST_FTM_IRQ_SP_PKT_SENT     0x00200000
#define BAST_FTM_IRQ_ASSIST_ERR      0x00400000
#define BAST_FTM_IRQ_RX_PKT_RDY_CNT  0x00800000
#define BAST_FTM_IRQ_FPKT_CNT        0x01000000
#define BAST_FTM_IRQ_FBAD_CRC_CNT    0x02000000
#define BAST_FTM_IRQ_FRX_MSK_NM_CNT  0x04000000
#define BAST_FTM_IRQ_FXCLUDE_CNT     0x08000000
#define BAST_FTM_IRQ_RX_BIT_MASK_CHG 0x10000000
#define BAST_FTM_IRQ_SER_NUM_CHG     0x20000000
#define BAST_FTM_IRQ_RX_TYPE1_CNT    0x40000000
#define BAST_FTM_IRQ_RX_TYPE2_CNT    0x80000000

#define BAST_FTM_IRQ_TX_DONE         0x00300006
#define BAST_FTM_IRQ_PKT_RCVD        0x00018000


/* timer enumeration */
typedef enum BAST_Ftm_TimerSelect{
   BAST_Ftm_TimerSelect_eRx = 0,
   BAST_Ftm_TimerSelect_eTx,
   BAST_Ftm_TimerSelect_eReg
} BAST_Ftm_TimerSelect;


/* exclude list enumeration */
typedef enum BAST_Ftm_Exclude{
   BAST_Ftm_Exclude_ePolls = 0,
   BAST_Ftm_Exclude_eOffers
} BAST_Ftm_Exclude;


/* interrupt enumeration */
typedef enum BAST_Ftm_IntID{
   BAST_Ftm_IntID_eUart = 0,
   BAST_Ftm_IntID_eUartTx,
   BAST_Ftm_IntID_eUartEnd,
   BAST_Ftm_IntID_eUartErr,
   BAST_Ftm_IntID_eRssiRise,
   BAST_Ftm_IntID_eRssiFall,
   BAST_Ftm_IntID_eCorrState,
   BAST_Ftm_IntID_eCorrByte,
   BAST_Ftm_IntID_eCorrTimeout,
   BAST_Ftm_IntID_eCorrAgc,
   BAST_Ftm_IntID_eCorrPreamble,
   BAST_Ftm_IntID_eTimer1,
   BAST_Ftm_IntID_eTimer2,
   BAST_Ftm_IntID_eTimer3,
   BAST_Ftm_IntID_eAssistIdle,
   BAST_Ftm_IntID_eRxPktRdy,
   BAST_Ftm_IntID_eAssistLvl1,
   BAST_Ftm_IntID_eAssistLvl2,
   BAST_Ftm_IntID_eAssistLvl3,
   BAST_Ftm_IntID_eAssistLvl4,
   BAST_Ftm_IntID_eHbPktSent,
   BAST_Ftm_IntID_eSpPktSent,
   BAST_Ftm_IntID_eAssistErr,
   BAST_Ftm_IntID_eRxPktRdyCnt,
   BAST_Ftm_IntID_eFPktCnt,
   BAST_Ftm_IntID_eFBadCrcCnt,
   BAST_Ftm_IntID_eFRxMskNmCnt,
   BAST_Ftm_IntID_eFXcludeCnt,
   BAST_Ftm_IntID_eRxBitMaskChg,
   BAST_Ftm_IntID_eSerNumChg,
   BAST_Ftm_IntID_eRxType1Cnt,
   BAST_Ftm_IntID_eRxType2Cnt
} BAST_Ftm_IntID;


#endif /* BAST_G2_PRIV_FTM_H__ */
