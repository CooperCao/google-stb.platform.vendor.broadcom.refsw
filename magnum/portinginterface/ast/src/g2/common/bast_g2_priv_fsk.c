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
 
#ifdef BAST_ENABLE_SKIT_FSK

#include "bast.h"
#include "bast_priv.h"
#include "bast_g2_priv.h"
#include "bast_g2_priv_fsk.h"

BDBG_MODULE(BAST_g2_priv_fsk);


/* private functions prototypes used by public ftm functions */
BERR_Code BAST_g2_Fsk_P_InitFsk(BAST_Handle);
BERR_Code BAST_g2_Fsk_P_InitDevice(BAST_Handle);
BERR_Code BAST_g2_Fsk_P_ConfigureFsk(BAST_Handle, bool, bool);
BERR_Code BAST_g2_Fsk_P_VerifyInterrupts(BAST_Handle);
BERR_Code BAST_g2_Fsk_P_EnableFskInterrupts(BAST_Handle, bool);
BERR_Code BAST_g2_Fsk_P_EnableReceiver(BAST_Handle, bool);
BERR_Code BAST_g2_Fsk_P_ScheduleTransmit(BAST_Handle, uint8_t);
BERR_Code BAST_g2_Fsk_P_TransmitPacket(BAST_Handle);
BERR_Code BAST_g2_Fsk_P_ComputeCrc16(BAST_Handle, uint8_t*, uint8_t, uint16_t*);


BAST_g2_Ftm_P_InterruptCbTable BAST_Ftm_Interrupts[] =
{
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_UART, BAST_Fsk_IntID_eUart, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_UART_TX, BAST_Fsk_IntID_eUartTx, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_UART_END, BAST_Fsk_IntID_eUartEnd, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_UART_ERR, BAST_Fsk_IntID_eUartErr, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_RSSI_RISE, BAST_Fsk_IntID_eRssiRise, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_RSSI_FALL, BAST_Fsk_IntID_eRssiFall, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_CORR_STATE, BAST_Fsk_IntID_eCorrState, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_CORR_BYTE, BAST_Fsk_IntID_eCorrByte, true)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_CORR_TIMEOUT, BAST_Fsk_IntID_eCorrTimeout, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_CORR_AGC, BAST_Fsk_IntID_eCorrAgc, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_CORR_PREAMBLE, BAST_Fsk_IntID_eCorrPreamble, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_TIMER1, BAST_Fsk_IntID_eTimer1, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_TIMER2, BAST_Fsk_IntID_eTimer2, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_TIMER3, BAST_Fsk_IntID_eTimer3, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_ASSIST_IDLE, BAST_Fsk_IntID_eAssistIdle, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_RX_PKT_RDY, BAST_Fsk_IntID_eRxPktRdy, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_ASSIST_LVL1, BAST_Fsk_IntID_eAssistLvl1, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_ASSIST_LVL2, BAST_Fsk_IntID_eAssistLvl2, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_ASSIST_LVL3, BAST_Fsk_IntID_eAssistLvl3, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_ASSIST_LVL4, BAST_Fsk_IntID_eAssistLvl4, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_HB_PKT_SENT, BAST_Fsk_IntID_eHbPktSent, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_SP_PKT_SENT, BAST_Fsk_IntID_eSpPktSent, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_ASSIST_ERR, BAST_Fsk_IntID_eAssistErr, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_RX_PKT_RDY_CNT, BAST_Fsk_IntID_eRxPktRdyCnt, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_FPKT_CNT, BAST_Fsk_IntID_eFPktCnt, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_FBAD_CRC_CNT, BAST_Fsk_IntID_eFBadCrcCnt, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_FRX_MSK_NM_CNT, BAST_Fsk_IntID_eFRxMskNmCnt, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_FXCLUDE_CNT, BAST_Fsk_IntID_eFXcludeCnt, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_RX_BIT_MASK_CHG, BAST_Fsk_IntID_eRxBitMaskChg, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_SER_NUM_CHG, BAST_Fsk_IntID_eSerNumChg, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_RX_TYPE1_CNT, BAST_Fsk_IntID_eRxType1Cnt, false)
   BAST_INT_CB_DEF(BCHP_INT_ID_FTM_RX_TYPE2_CNT, BAST_Fsk_IntID_eRxType2Cnt, false)
};


/******************************************************************************
 BAST_g2_P_ResetFtm() - Non-ISR context
******************************************************************************/ 
BERR_Code BAST_g2_P_ResetFtm(BAST_Handle h)
{
   BERR_Code retCode;
   
   /* turn off ftm interrupts */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableFskInterrupts(h, false));
   
   retCode = BAST_g2_Fsk_P_InitFsk(h);
   
   /* turn on ftm interrupts */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableFskInterrupts(h, true));
   
   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_g2_P_ReadFtm() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_ReadFtm(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [out] data read */
   uint8_t *n      /* [out] length of data read */
)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(h->pImpl);
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   *n = 0;
   
   if ((hFsk->buf_status & BAST_FSK_BUF_RX_NOT_EMPTY) == 0)
      return BERR_SUCCESS;
   
   /* disable interrupts */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableFskInterrupts(h, false));
   
   /* check rx buffer */
   if (hFsk->buf_status & BAST_FSK_BUF_RX_NOT_EMPTY)
   {
   #ifdef BAST_FSK_LOOPBACK_TEST
      *n = 11; /* fixed length downlink packet of 11 for SKIT */
   #else
      *n = 8; /* fixed length downlink packet of 8 for SKIT */
   #endif
      BKNI_Memcpy(pBuf, hFsk->rx_buffer, *n);
      hFsk->buf_status &= ~BAST_FSK_BUF_RX_NOT_EMPTY;
   }
   
   if (hFsk->buf_status & BAST_FSK_BUF_RX_NOT_EMPTY)
      BKNI_SetEvent(hDev->hFtmEvent);

   done:
   BAST_g2_Fsk_P_EnableFskInterrupts(h, true);
   return retCode;
}


/******************************************************************************
 BERR_Code BAST_g2_P_WriteFtm() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_WriteFtm(
   BAST_Handle h,  /* [in] BAST handle */
   uint8_t *pBuf,  /* [in] data to write */
   uint8_t n       /* [in] TDMA time slot */
)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t crc;
   
   /* check for invalid time slot */
   if ((n < 1) || (n > hFsk->n_channel + 1))
      return BERR_INVALID_PARAMETER;
   
   /* disable interrupts */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableFskInterrupts(h, false));
   
   /* check for busy conditions */
   if ((hFsk->status & BAST_FSK_STATUS_TX_IN_PROGRESS) || (hFsk->buf_status & BAST_FSK_BUF_TX_NOT_EMPTY))
   {
      BDBG_MSG(("TX busy: packet dropped"));
      hFsk->err_status |= BAST_FSK_ERR_TX_PKT_DROPPED;
      retCode = BAST_ERR_FTM_TX_BUSY;
      goto done;
   }
   
   /* protect tx buffer */
   hFsk->buf_status |= BAST_FSK_BUF_TX_NOT_EMPTY;
   BKNI_Memcpy(hFsk->tx_buffer, pBuf, 11);
   //printf("WriteFtm slot=%d %02X %02X %02X...\n", n, hFsk->tx_buffer[0], hFsk->tx_buffer[1], hFsk->tx_buffer[2]);
   
   /* auto-insert crc for now */
   BAST_g2_Fsk_P_ComputeCrc16(h, pBuf, 9, &crc);
   //printf("crc16 (%d) = %04X\n", 9, crc);
   hFsk->tx_buffer[9] = crc >> 8;
   hFsk->tx_buffer[10] = crc & 0xFF;
   retCode = BAST_g2_Fsk_P_ScheduleTransmit(h, n);
   
   done:
   BAST_g2_Fsk_P_EnableFskInterrupts(h, true);
   return retCode;
}


/******************************************************************************
 BAST_g2_P_PowerDownFtm() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_PowerDownFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
#ifndef BAST_EXCLUDE_POWERDOWN
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;
   
   /* disable receiver, reset state */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableReceiver(h, false));
   BAST_g2_Fsk_P_InitDevice(h);
   
   BAST_FSK_READ(FTM_PHY_ANA_MISC, &mb);
   mb |= 0x2081;  /* power down phy, stop 108MHz clock */
   BAST_FSK_WRITE(FTM_PHY_ANA_MISC, mb);
   
   done:
   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BAST_g2_P_PowerUpFtm() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_P_PowerUpFtm(
   BAST_Handle h  /* [in] BAST handle */
)
{
#ifndef BAST_EXCLUDE_POWERDOWN
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;
   
   BAST_FSK_READ(FTM_PHY_ANA_MISC, &mb);
   mb &= ~0x2080;  /* power up phy first */
   BAST_FSK_WRITE(FTM_PHY_ANA_MISC, mb);
   mb &= ~0x01;    /* start 108MHz clock */
   BAST_FSK_WRITE(FTM_PHY_ANA_MISC, mb);
   
   BAST_g2_Fsk_P_InitDevice(h);
   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BAST_g2_P_GetFtmEventHandle()
******************************************************************************/
BERR_Code BAST_g2_P_GetFtmEventHandle(BAST_Handle h, BKNI_EventHandle *hFtmEvent)
{
   BERR_Code retCode = BERR_SUCCESS;
  
   *hFtmEvent = ((BAST_g2_P_Handle *)(h->pImpl))->hFtmEvent;
   return retCode;
}


/******************************************************************************
 BAST_g2_Ftm_P_InitFsk()
******************************************************************************/ 
BERR_Code BAST_g2_Fsk_P_InitFsk(
   BAST_Handle h    /* [in] BAST channel handle */
)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;
   
   static uint32_t phy_coef[] =
   {
      0xfffe0011, 0x00200022, 0x0016fffe, 0xffe1ffcc, 0xffcaffe1, 0x000d003f, 0x0060005c, 0x002affd8, 0xff84ff56,
      0xff6fffd1, 0x006000e1, 0x011500D3, 0x0022ff3d, 0xfe85fe57, 0xfee4000d, 0x01680260, 0x02700165, 0xff87fd85,
      0xfc44fc7a, 0xfe580159, 0x045d0606, 0x054401dd, 0xfcb0f799, 0xf4f0f6bf, 0xfe010a1d, 0x18e92727, 0x316f352d
   };
   
   /* initialize ftm dev struct */
   BAST_g2_Fsk_P_InitDevice(h);

   BAST_FSK_WRITE(FTM_PHY_ADDR, 0x80000000);        /* write coef memory w/ auto increment */
   for(i = 0; i < 27; i++)                          /* write next 27 values to FTM_PHY_COEF */
      BAST_FSK_WRITE(FTM_PHY_COEF, phy_coef[i]);
   BAST_FSK_WRITE(FTM_PHY_ADDR, 0x80000000);
   
   /* setup RSSI */
   BAST_FSK_WRITE(FTM_PHY_RSSI_THRES, 0x000001E0);
   BAST_FSK_WRITE(FTM_PHY_RSSI_CTL, 0x10B00902);
   
   /* setup correlator */
   BAST_FSK_WRITE(FTM_PHY_CORR_TIMEOUT, 0x00010010);     /* return to idle on start bit timeout on 16 bits */
   BAST_FSK_WRITE(FTM_PHY_CORR_THRES, 0x00050726);       /* frame_thres=0x5, start_thres=0x726 */
   BAST_FSK_WRITE(FTM_PHY_CORR_PREAMBLE, 0x0055550D);    /* preamble */
   BAST_FSK_WRITE(FTM_PHY_CORR_PEAK_QUAL, 0x000B0510);   /* sample_length=11, clock_length=1296 (108*(sample_length+1)) */
   BAST_FSK_WRITE(FTM_PHY_CORR_CTL, 0x000B000D);         /* packet_length=11, frame_length=.21ms, dc offset on */
   BAST_FSK_WRITE(FTM_PHY_RST, 0x00000020);              /* reset correlator */
   
   BAST_FSK_WRITE(FTM_PHY_ASSIST_CTL, 0x00000001);       /* stop HW assist */
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, 0x010B0000);           /* set pre carrier to 290us / post carrier to 30us */
   BAST_FSK_WRITE(FTM_PHY_CTL, 0x00000401);  /* [0] enable FTM RX, [1]=1 rx inverted, [3:2]=0 board loopback, [10] auto_off_dis [15]=1 tx format offset binary, 0 2s complement */
   
   BAST_FSK_WRITE(FTM_PHY_FIRQ_STS, 0xFFFFFFFF);      /* clear all interrupts */
   BAST_FSK_WRITE(FTM_PHY_IRQ_MSK, 0x00000001);       /* enable UART RX */
   
   /* reset uart */
   BAST_FSK_WRITE(FTM_UART_FCR, 0xC1);
   BAST_FSK_WRITE(FTM_UART_MCR, 0x20);
   BAST_FSK_WRITE(FTM_UART_LCR, 0x80);
   BAST_FSK_WRITE(FTM_UART_DLL, 0xAD);
   BAST_FSK_WRITE(FTM_UART_LCR, 0x00);
   BAST_FSK_WRITE(FTM_UART_IER, 0x81);
   BAST_FSK_WRITE(FTM_UART_LCR, 0x03);
   
#if (BCHP_CHIP==7335)
   /* check ftm interrupts */
   retCode = BAST_g2_Fsk_P_VerifyInterrupts(h);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_MSG(("FSK interrupt out of sync!(1)"));
      BAST_g2_Fsk_P_ConfigureFsk(h, true, true);   /* swap clocks */
      
      /* check ftm interrupts again */
      retCode = BAST_g2_Fsk_P_VerifyInterrupts(h);
      if (retCode != BERR_SUCCESS)
      {
         BDBG_MSG(("FSK interrupt out of sync!(2)"));
         BAST_g2_Fsk_P_ConfigureFsk(h, true, false);   /* select default clock */
         
         /* check ftm interrupts one more time */
         retCode = BAST_g2_Fsk_P_VerifyInterrupts(h);
         if (retCode != BERR_SUCCESS)
         {
            BDBG_MSG(("FSK interrupt out of sync!(3)"));
            BAST_g2_Fsk_P_ConfigureFsk(h, false, false);   /* unswap clocks */
            
            retCode = BAST_g2_Fsk_P_VerifyInterrupts(h);
            if (retCode != BERR_SUCCESS)
            {
               hFsk->err_status |= 0x80;
               return BERR_UNKNOWN;
            }
         }
      }
   }
#endif
   
   /* setup FSK frequencies */
#ifdef BAST_FSK_LOOPBACK_TEST
   BAST_FSK_WRITE(FTM_PHY_RX_FCW, 0x00F097B5)      /* 6.5e6/108e6 * 2^24 -> 2'sc = 00f097b5 for 6.5M RX */
   BAST_FSK_WRITE(FTM_PHY_TX_FCW, 0x00F55555)      /* 4.5e6/108e6 * 2^24 -> 2'sc = 00f097b5 for 4.5M TX */
   BAST_FSK_WRITE(FTM_PHY_CORR_IRQ, 0x0000000F);   /* loopback packet length 11 for SKIT + 3 byte preamble */
#else
   BAST_FSK_WRITE(FTM_PHY_RX_FCW, 0x00F55555);     /* 4.5e6/108e6 * 2^24 -> 2'sc = 00f55555 for 4.5M RX */
   BAST_FSK_WRITE(FTM_PHY_TX_FCW, 0x00F097b5);     /* 6.5e6/108e6 * 2^24 -> 2'sc = 00f097b5 for 6.5M TX */
   BAST_FSK_WRITE(FTM_PHY_CORR_IRQ, 0x0000000B);   /* fixed length downlink packet of 8 for SKIT + 3 byte preamble */
#endif
   BAST_FSK_WRITE(FTM_PHY_TX_DEV, 0x000022F3);     /* TX_DEV = 57.6 KHz (57.6e3/108e6)*2^24 = 0x22F3 */
   
   /* turn on receiver */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableReceiver(h, true));
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_InitDevice()
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_InitDevice(BAST_Handle h)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(h->pImpl);
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb;
   
   /* init fsk settings */
   hFsk->n_channel = 0;
   hFsk->bLnbDetected = false;
   
   /* reset statuses */
   hFsk->status = BAST_FSK_STATUS_RESET;
   hFsk->buf_status = BAST_FSK_BUF_ALL_EMPTY;
   hFsk->err_status = BAST_FSK_ERR_NONE;
   
#if (BCHP_CHIP==7335)
   BAST_g2_Fsk_P_ConfigureFsk(h, false, true);   /* use diff osc for ftm clock */
#else
   BAST_g2_Fsk_P_ConfigureFsk(h, false, false);
#endif
   
   /* set up ftm tx level */
   if(hDev->ftmChannelSelect > BAST_FskChannelConfig_eCh0Tx_Ch1Rx)
   {
      BAST_FSK_READ(FTM_PHY_ANA1_0, &mb);
      mb &= ~0x3F;
      mb |= (hDev->ftmTxPower & 0x3F) | 0x40;  /* enable FSK voltage mode */
      BAST_FSK_WRITE(FTM_PHY_ANA1_0, mb);
   }
   else
   {
      BAST_FSK_READ(FTM_PHY_ANA0_0, &mb);
      mb &= ~0x3F;
      mb |= (hDev->ftmTxPower & 0x3F) | 0x40;  /* enable FSK voltage mode */
      BAST_FSK_WRITE(FTM_PHY_ANA0_0, mb);
   }
   
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_ConfigureFsk() - ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_ConfigureFsk(BAST_Handle h, bool bClkSwap, bool bAltClk)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(h->pImpl);
   uint32_t mb;
   
   /* set up fsk channels */
   if (!bClkSwap)
   {
      switch(hDev->ftmChannelSelect)
      {
         case BAST_FskChannelConfig_eCh0Tx_Ch0Rx:
            mb = 0x02060;  /* ch1_fskphy_pwrdn | ch0_fskphy_dac_en | ch0_tx_out_edge */
            break;
         case BAST_FskChannelConfig_eCh0Tx_Ch1Rx:
            mb = 0x80060;  /* ch1_ftm_rx_sel | ch0_fskphy_dac_en | ch0_tx_out_edge */
            break;
         case BAST_FskChannelConfig_eCh1Tx_Ch0Rx:
            mb = 0x11C00;  /* ch0_clk_swp | ch1_fskphy_dac_en | ch1_tx_out_edge | ch1_ftm_tx_sel */
            break;
         case BAST_FskChannelConfig_eCh1Tx_Ch1Rx:
            mb = 0x91C80;  /* ch1_ftm_rx_sel | ch0_clk_swp | ch1_fskphy_dac_en | ch1_tx_out_edge | ch1_ftm_tx_sel | ch0_fskphy_pwrdn */
            break;
         default:
            mb = 0x02060;  /* use ch0 tx ch0 rx as default */
      }
   }
   else
   {
      switch(hDev->ftmChannelSelect)
      {
         case BAST_FskChannelConfig_eCh0Tx_Ch0Rx:
            mb = 0x10060;  /* ch0_clk_swp | ch0_fskphy_dac_en | ch0_tx_out_edge */
            break;
         case BAST_FskChannelConfig_eCh0Tx_Ch1Rx:
            mb = 0x90060;  /* ch1_ftm_rx_sel | ch0_clk_swp | ch0_fskphy_dac_en | ch0_tx_out_edge */
            break;
         case BAST_FskChannelConfig_eCh1Tx_Ch0Rx:
            mb = 0x41C00;  /* ch1_clk_swp | ch1_fskphy_dac_en | ch1_tx_out_edge | ch1_ftm_tx_sel */
            break;
         case BAST_FskChannelConfig_eCh1Tx_Ch1Rx:
            mb = 0xC1C00;  /* ch1_ftm_rx_sel | ch1_clk_swp | ch1_fskphy_dac_en | ch1_tx_out_edge | ch1_ftm_tx_sel */
            BAST_FSK_WRITE(FTM_PHY_CTL, 0x00000003);
            break;
         default:
            mb = 0x10060;
      }
   }
   if (bAltClk)
      mb |= 0x4100;  /* use alternate clock source */
      
   BAST_FSK_WRITE(FTM_PHY_ANA_MISC, mb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_Ftm_P_VerifyInterrupts() - ISR context
******************************************************************************/ 
BERR_Code BAST_g2_Fsk_P_VerifyInterrupts(BAST_Handle h)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, mb, mb2, tx_ctl, phy_ctl, ana_misc;
   
   /* disable fskphy for loopback */
   BAST_FSK_READ(FTM_PHY_ANA_MISC, &ana_misc);
   mb = (ana_misc | 0x80010);    /* ch1_ftm_rx_sel | ch0_diseqc_sel */
   mb &= ~0x1440;                /* ch1_fskphy_dac_en=0 | ch1_ftm_tx_sel=0 | ch0_fsk_phy_dac_en=0 */
   BAST_FSK_WRITE(FTM_PHY_ANA_MISC, mb);
   
   /* set rx input to loopback and auto rx disable off */
   BAST_FSK_READ(FTM_PHY_CTL, &phy_ctl);
   mb = (phy_ctl | 0x40C);
   BAST_FSK_WRITE(FTM_PHY_CTL, mb);
   
   /* increase tx post-carrier to 16 bit time */
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, 0x0FC60000);
   
   /* enable correlator interrupt for state 1, byte 3 */
   BAST_FSK_WRITE(FTM_PHY_CORR_IRQ, 0x00000103);
   
   /* set trace countdown timers to 1 */
   BAST_FSK_WRITE(FTM_PHY_ASSIST_CNT1, 0x10000);   /* trace rx packet count */
   BAST_FSK_WRITE(FTM_PHY_ASSIST_CNT3, 0x0101);    /* trace offer count */
   BAST_FSK_READ(FTM_UART_ASSIST_CMDTRR, &mb);
   mb |= 0x1414;     /* track manual offer */
   BAST_FSK_WRITE(FTM_UART_ASSIST_CMDTRR, mb);
   
   /* clear all interrupts */
   BAST_FSK_WRITE(FTM_PHY_FIRQ_STS, 0xFFFFFFFF);
   for (i = 0; i < 32; i++)
   {
      retCode = BINT_ClearCallback_isr(hFsk->hCallback[i]);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }
   
   /* enable receiver and hw assist */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableReceiver(h, true));
   BAST_FSK_WRITE(FTM_PHY_ASSIST_CTL, 0x00000000);

   /* disable tx state machine */
   BAST_FSK_READ(FTM_PHY_TX_CTL, &tx_ctl);
   tx_ctl &= 0xFFFFFFFE;
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, tx_ctl);
   
   /* reset the tx fifo */
   mb = 0x05;
   BAST_FSK_WRITE(FTM_UART_FCR, mb);
   
   /* load uart with preamble and test packet */
   mb = 0x55;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x0D;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x14;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x00;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x0F;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x02;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x01;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x0F;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0xA7;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   
   /* start the transmit */
   tx_ctl &= 0xF0FF00FF;
   tx_ctl |= 0x0A000000;
   tx_ctl |= ((3 + 7) << 8);   /* set packet size for preamble + offer */
   tx_ctl |= 0x00000001;
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, tx_ctl);
   
   /* wait for uart end */
   while(1)
   {
      BAST_FSK_READ(FTM_PHY_IRQ_STS, &mb);
      if (mb & 0x4) break;
   }
   
   /* wait to disable hw assist */
   while(1)
   {
      BAST_FSK_WRITE(FTM_PHY_ASSIST_CTL, 0x00000001);
      BAST_FSK_READ(FTM_PHY_ASSIST_STS, &mb);
      if (mb & 0x1) break;
   }
   
   /* disable tx state machine */
   BAST_FSK_READ(FTM_PHY_TX_CTL, &mb);
   mb &= 0xFFFFFFFE;
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, mb);

   /* disable receiver */
   BAST_g2_Fsk_P_EnableReceiver(h, false);
   
   /* set rx input back and post-carrier back to default */
   BAST_FSK_WRITE(FTM_PHY_CTL, phy_ctl);
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, 0x010B0000);
   
   /* compare interrupt registers */
   BAST_FSK_READ(FTM_PHY_IRQ_STS, &mb);
   BAST_FSK_READ(FTM_INTR2_CPU_STATUS, &mb2);
   if (mb != mb2)
   {
      printf("***%08X!=%08X\n", mb, mb2);
      return BERR_UNKNOWN;
   }
   
   /* restore settings */
   BAST_FSK_WRITE(FTM_PHY_ANA_MISC, ana_misc);
   
   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_Fsk_P_EnableFskInterrupts() - Non-ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_EnableFskInterrupts(BAST_Handle h, bool bEnable)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i;
   
   if (bEnable)
   {
      for (i = 0; i < 32 ; i++)
      {
         /* clear callback */
         retCode = BINT_ClearCallback(hFsk->hCallback[i]);
         BDBG_ASSERT(retCode == BERR_SUCCESS);
            
         if (BAST_Ftm_Interrupts[i].enable)
         {
            /* enable the interrupt */
            retCode = BINT_EnableCallback(hFsk->hCallback[i]);
            BDBG_ASSERT(retCode == BERR_SUCCESS);
         }
         else
         {
            /* disable the interrupt */
            retCode = BINT_DisableCallback(hFsk->hCallback[i]);
            BDBG_ASSERT(retCode == BERR_SUCCESS);
         }
      }
   }
   else
   {
      for(i = 0; i < 32 ; i++)
      {
         /* clear and disable the interrupt */
         retCode = BINT_ClearCallback(hFsk->hCallback[i]);
         BDBG_ASSERT(retCode == BERR_SUCCESS);
         retCode = BINT_DisableCallback(hFsk->hCallback[i]);
         BDBG_ASSERT(retCode == BERR_SUCCESS);
      }
   }
   
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_EnableInterrupt_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_EnableInterrupt_isr(BAST_Handle h, BAST_Fsk_IntID intID, bool bEnable)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   
   /* clear callback */
   retCode = BINT_ClearCallback_isr(hFsk->hCallback[intID]);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   
   if (bEnable)
   {
      BAST_Ftm_Interrupts[intID].enable = true;
      retCode = BINT_EnableCallback_isr(hFsk->hCallback[intID]);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }
   else
   {
      BAST_Ftm_Interrupts[intID].enable = false;
      retCode = BINT_DisableCallback_isr(hFsk->hCallback[intID]);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }
   
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_EnableReceiver()
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_EnableReceiver(BAST_Handle h, bool bEnable)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb, old;
   
   BAST_FSK_READ(FTM_PHY_CORR_CTL, &old);
   mb = old;
   if (bEnable)
      mb |= 0x01;
   else
      mb &= ~0x01;
   
   /* check if value changed */
   if (old ^ mb)
      BAST_FSK_WRITE(FTM_PHY_CORR_CTL, mb);
   
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_ResetCorrelator()
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_ResetCorrelator(BAST_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb = 0;
   
   mb |= 0x0100; /* reset the UART */
   mb |= 0x20; /* put the correlator to idle state */
   BAST_FSK_WRITE(FTM_PHY_RST, mb);

   /* initialize the UART block */
   BAST_FSK_WRITE(FTM_UART_FCR, 0xC1);
   BAST_FSK_WRITE(FTM_UART_MCR, 0x20);
   BAST_FSK_WRITE(FTM_UART_LCR, 0x80);
   BAST_FSK_WRITE(FTM_UART_DLL, 0xAD);
   BAST_FSK_WRITE(FTM_UART_LCR, 0x00);
   BAST_FSK_WRITE(FTM_UART_IER, 0x81);
   BAST_FSK_WRITE(FTM_UART_LCR, 0x03);
   
   return retCode;
}

/******************************************************************************
 BAST_g2_Fsk_P_StopTimer() - ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_StopTimer(BAST_Handle h, BAST_Fsk_TimerSelect t)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb, mask;
   BAST_Fsk_IntID Timer_IntID;

   switch (t)
   {
      case BAST_Fsk_TimerSelect_eRx:
         mask = (1 << 20);
         Timer_IntID = BAST_Fsk_IntID_eTimer1;
         break;
      
      case BAST_Fsk_TimerSelect_eTx:
         mask = (1 << 19);
         Timer_IntID = BAST_Fsk_IntID_eTimer2;
         break;
      
      case BAST_Fsk_TimerSelect_eTimeCount:
         mask = (1 << 18);
         Timer_IntID = BAST_Fsk_IntID_eTimer3;
         break;
      
      default:
         BDBG_MSG(("BAST_g2_Fsk_P_StopTimer() - should not get here!\n"));
         return BERR_UNKNOWN;
   }

   /* stop timer control */
   BAST_FSK_READ(FTM_PHY_CTL, &mb);
   mb &= ~mask;
   BAST_FSK_WRITE(FTM_PHY_CTL, mb);
   
   /* clear and disable the interrupt */
   BAST_Ftm_Interrupts[Timer_IntID].enable = false;
   retCode = BINT_ClearCallback_isr(hFsk->hCallback[Timer_IntID]);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   BINT_DisableCallback_isr(hFsk->hCallback[Timer_IntID]);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_StartTimer() - ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_StartTimer(BAST_Handle h, BAST_Fsk_TimerSelect t, uint32_t count)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask, mb;
   BAST_Fsk_IntID Timer_IntID;

   /* stop timer */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_StopTimer(h, t));
      
   switch (t)
   {
      case BAST_Fsk_TimerSelect_eRx:
         mask = (1 << 20);
         Timer_IntID = BAST_Fsk_IntID_eTimer1;
         BAST_FSK_WRITE(FTM_PHY_TIMER1, count);  /* set requested time out */
         break;
      
      case BAST_Fsk_TimerSelect_eTx:
         mask = (1 << 19);
         Timer_IntID = BAST_Fsk_IntID_eTimer2;
         BAST_FSK_WRITE(FTM_PHY_TIMER2, count);  /* set requested time out */
         break;
      
      case BAST_Fsk_TimerSelect_eTimeCount:
         mask = (1 << 18);
         Timer_IntID = BAST_Fsk_IntID_eTimer3;
         BAST_FSK_WRITE(FTM_PHY_TIMER3, count);  /* set requested time out */
         break;
      
      default:
         BDBG_MSG(("BAST_g2_Fsk_P_StartTimer() - should not get here!\n"));
         return BERR_UNKNOWN;
   }
   
   /* clear and enable the selected timer interrupt */
   BAST_Ftm_Interrupts[Timer_IntID].enable = true;
   retCode = BINT_ClearCallback_isr(hFsk->hCallback[Timer_IntID]);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BINT_EnableCallback_isr(hFsk->hCallback[Timer_IntID]);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   
   /* enable selected timer */
   BAST_FSK_READ(FTM_PHY_CTL, &mb);
   mb |= mask;
   BAST_FSK_WRITE(FTM_PHY_CTL, mb);
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_ComputeCrc16()
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_ComputeCrc16(BAST_Handle h, uint8_t *pBuf, uint8_t len, uint16_t *crc16)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint16_t crc = 0xFFFF;  /* initialize accumulator */
   uint8_t i;
   
   /* CCITT algorithm */
   while (len--)
   {
      crc ^= (*pBuf++ & 0xFF) << 8;
      for (i = 0; i < 8; i++)
      {
         /* accumulate MSB first with polynomial X16+X12+X5+1 */
         if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
         else
            crc <<= 1;
      }
   }
   *crc16 = ~crc;    /* invert crc */
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_ScheduleTransmit() - ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_ScheduleTransmit(BAST_Handle h, uint8_t slot)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t local_timecount, tx_time, tx_delay;
   
   if (!hFsk->bLnbDetected)
      return BERR_NOT_INITIALIZED;
   
   BAST_FSK_READ(FTM_PHY_TIMER3, &local_timecount);
   //printf("TC_local=%d", local_timecount / 100);
   
   /* determine slot delay */
   tx_time = (slot - 1) * hFsk->slot_length / 100;    /* slot delay in usec */
   
   /* transmit in next tdma cycle */
   local_timecount = hFsk->timecount_max - local_timecount / 100 + 1;   /* invert local timecount */
   //printf(" -> %d, tx_slot=%d ", local_timecount, tx_time);

   tx_time += ((int)(local_timecount / hFsk->tdma_cycle) + 1) * hFsk->tdma_cycle;
   //printf("tx_time=%d\n", tx_time);
   //tx_delay += ((int)(local_timecount / hFsk->tdma_cycle) + 1) * hFsk->tdma_cycle * 100;
   tx_delay = tx_time - local_timecount;
   
   //printf("final_delay=%d\n", tx_delay);
   retCode = BAST_g2_Fsk_P_StartTimer(h, BAST_Fsk_TimerSelect_eTx, tx_delay * 100);
   
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_TransmitPacket() - ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_TransmitPacket(BAST_Handle h)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, mb, tx_ctl;
   
   //printf("Tx: %02X %02X %02X...\n", hFsk->tx_buffer[0], hFsk->tx_buffer[1], hFsk->tx_buffer[2]);
   
   if (hFsk->status & BAST_FSK_STATUS_RX_IN_PROGRESS)
      BDBG_MSG(("RX in progress when transmitting!\n"));
   
   /* disable the correlator while transmitting */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableReceiver(h, false));
   
   /* disable tx state machine */
   BAST_FSK_READ(FTM_PHY_TX_CTL, &tx_ctl);
   tx_ctl &= 0xFFFFFFFE;
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, tx_ctl);
   
   /* reset the tx fifo */
   mb = 0x05;
   BAST_FSK_WRITE(FTM_UART_FCR, mb);
   
   /* set ftm statuses */
   hFsk->status |= BAST_FSK_STATUS_TX_IN_PROGRESS;
   hFsk->buf_status &= ~BAST_FSK_BUF_TX_NOT_EMPTY;
   
   /* enable uart end interrupt */
   BAST_g2_Fsk_P_EnableInterrupt_isr(h, BAST_Fsk_IntID_eUartEnd, true);
   
   /* start the transmit */
   tx_ctl &= 0xF0FF00FF;
   tx_ctl |= 0x0A000000;         /* end delay length */
   tx_ctl |= ((11 + 3) << 8);    /* set packet size to include preamble */
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, tx_ctl);

   /* send out the preamble */
   mb = 0x55;
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   BAST_FSK_WRITE(FTM_UART_THR, mb);
   mb = 0x0D;
   BAST_FSK_WRITE(FTM_UART_THR, mb);  
   
   /* first byte is the length of the packet to be transmitted*/
   for (i = 0; i < 11; i++)
   {
      /* add the transmit data byte to the tx fifo */
      mb = hFsk->tx_buffer[i] & 0xFF;
      BAST_FSK_WRITE(FTM_UART_THR, mb); 
   }
   
   /* enable tx state machine */
   BAST_FSK_READ(FTM_PHY_TX_CTL, &tx_ctl);
   tx_ctl |= 0x01;
   BAST_FSK_WRITE(FTM_PHY_TX_CTL, tx_ctl);
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_TransmitDone()
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_TransmitDone(BAST_Handle h)
{
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   
   /* clear transmit in progress status */
   hFsk->status &= ~BAST_FSK_STATUS_TX_IN_PROGRESS;

   /* re-enable receiver after transmit done */
   BAST_CHK_RETCODE(BAST_g2_Fsk_P_EnableReceiver(h, true));
   
   done:
   return retCode;
}


/******************************************************************************
 BAST_g2_Fsk_P_ReceivePacket() - ISR context
******************************************************************************/
BERR_Code BAST_g2_Fsk_P_ReceivePacket(BAST_Handle h)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(h->pImpl);
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mb, packet_len;
   //uint32_t global_timecount;
   uint32_t local_timecount;
   uint8_t i, *pBuf;
   
   /* get the number of bytes in the rx fifo */
   BAST_FSK_READ(FTM_UART_RFL, &mb);
   packet_len = mb & 0xFF;
   
   //printf("ReceivePacket: %d\n", packet_len);

   if (packet_len && (hFsk->buf_status & BAST_FSK_BUF_RX_NOT_EMPTY))
   {
      //ftm_error_hook(H_MSG1_FTM_ERR_RX_FULL);
   }
   else
   {
      /* copy the data into rx_buffer */
      pBuf = hFsk->rx_buffer;
      for (i = 0; i < packet_len; i++)
      {
         BAST_FSK_READ(FTM_UART_RBR, &mb);
         *pBuf++ = mb & 0xFF;
      }
   
#if 0
      /* calculate crc16 of the packet */
      sval1 = 0xFFFF;
      XMEM_ADR_1 = RAM_TX_FTM >> 8;
      XMEM_ADR_0 = RAM_TX_FTM & 0xFF;
      for (data1 = ftm_tx_len; data1; data1--)
      {
         data2 = XMEM_DATA;
         compute_crc16(&sval1, data2);
      }
      sval1 = ~sval1; /* sval1 = CRC16 */
      
      BAST_g2_Fsk_P_ComputeCrc16();
#endif

      /* check for Timing_LNB message */
      if (hFsk->rx_buffer[0] == 0xEC)
      {
         if (!hFsk->bLnbDetected)
         {
            /* extract timing data */
            hFsk->n_channel = hFsk->rx_buffer[4] >> 3;      /* number of channels in LNB */
            hFsk->slot_length = 3000 + 500 * (hFsk->rx_buffer[4] & 0x07);  /* calculated in us */
            //hFsk->global_timecount = (hFsk->rx_buffer[2] << 8) | hFsk->rx_buffer[3];
            //printf("<Timing_LNB n_chan=%d TS=%d TC=%d>\n", hFsk->n_channel, hFsk->slot_length, hFsk->global_timecount);
            
            /* calculate TC max */
            hFsk->tdma_cycle = (hFsk->n_channel + 1) * (hFsk->slot_length / 100);
            hFsk->timecount_max = ((int)(65535/hFsk->tdma_cycle) * hFsk->tdma_cycle) - 1;
            //printf("fsk_cycle = %d * %d = %d\n", hFsk->n_channel + 1, hFsk->slot_length / 100, fsk_cycle);
            //printf("TCmax = floor(65535/%d) * (%d) - 1 = %d\n", fsk_cycle, fsk_cycle, hFsk->timecount_max);
            
            printf("[bLnbDetected: n_chan=%d TS=%d]\n", hFsk->n_channel, hFsk->slot_length);
            hFsk->bLnbDetected = true;
         }
         
         /* invert global timecount since local timer counts down */
         //global_timecount = (hFsk->rx_buffer[2] << 8) | hFsk->rx_buffer[3];
         local_timecount = hFsk->timecount_max + 1 - ((hFsk->rx_buffer[2] << 8) | hFsk->rx_buffer[3]);
         
         /* sync local timecount with global timecount */
         retCode = BAST_g2_Fsk_P_StartTimer(h, BAST_Fsk_TimerSelect_eTimeCount, local_timecount * 100 - 256);   /* account for one bit time delay */
         
         //printf("<TC_new=%d>\n", local_timecount);
      }
      
      hFsk->status &= ~BAST_FSK_STATUS_RX_IN_PROGRESS;
      hFsk->buf_status |= BAST_FSK_BUF_RX_NOT_EMPTY;
      BKNI_SetEvent(hDev->hFtmEvent);
   }
   
   return BAST_g2_Fsk_P_ResetCorrelator(h);
}


/******************************************************************************
 BAST_g2_Fsk_P_HandleInterrupt_isr() - ISR context
******************************************************************************/
void BAST_g2_Fsk_P_HandleInterrupt_isr(void *p, int param)
{
   BAST_Handle h = (BAST_Handle)p;
   BAST_g2_P_FtmDevice *hFsk = (BAST_g2_P_FtmDevice *) &(((BAST_g2_P_Handle *)(h->pImpl))->hFtmDev);
   BERR_Code retCode = BERR_SUCCESS;
   //uint32_t local_timecount;
   
   /* clear the interrupt */
   retCode = BINT_ClearCallback_isr(hFsk->hCallback[param]);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   
   switch (param)
	{
      case BAST_Fsk_IntID_eUartEnd:
         BDBG_MSG(("uart end!\n"));
         retCode = BAST_g2_Fsk_P_TransmitDone(h);
         break;
      
      case BAST_Fsk_IntID_eTimer1:
         BDBG_MSG(("timer1!\n"));
         retCode = BAST_g2_Fsk_P_ReceivePacket(h);
         break;
      
      case BAST_Fsk_IntID_eTimer2:
         BDBG_MSG(("timer2!\n"));
         BAST_CHK_RETCODE(BAST_g2_Fsk_P_StopTimer(h, BAST_Fsk_TimerSelect_eTx));
         retCode = BAST_g2_Fsk_P_TransmitPacket(h);
         break;
      
      case BAST_Fsk_IntID_eTimer3:
         BDBG_MSG(("timer3!\n"));
         break;
      
      case BAST_Fsk_IntID_eCorrByte:
         BDBG_MSG(("corr byte!\n"));
         hFsk->status |= BAST_FSK_STATUS_RX_IN_PROGRESS;
         //retCode = BAST_g2_Fsk_P_ReceivePacket(h);
         //BAST_FSK_READ(FTM_PHY_TIMER3, &local_timecount);
         //printf("TC_local=%d\n", local_timecount / 100);
         retCode = BAST_g2_Fsk_P_StartTimer(h, BAST_Fsk_TimerSelect_eRx, 256); /* wait one bit time for uart to finish */
         break;
         
      default:
         /* disable this unhandled interrupt */
         retCode = BINT_DisableCallback_isr(hFsk->hCallback[param]);
         BDBG_ASSERT(retCode == BERR_SUCCESS);
         BDBG_MSG(("Unhandled FTM Interrupt ID=0x%x !", param));
	}
   
   done:
   if (retCode != BERR_SUCCESS)
      BDBG_MSG(("Error in BAST_g2_Fsk_P_HandleInterrupt_isr(): %X\n", retCode));
}

#endif
