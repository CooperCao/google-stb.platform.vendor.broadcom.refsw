/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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

#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g2_priv.h"
#include "bwfe_4550_priv.h"

BDBG_MODULE(bwfe_4550_priv);


uint32_t BWFE_g2_ChannelIntrID[BWFE_NUM_CHANNELS][BWFE_g2_MaxIntID] = {
   /* wfe channel 0 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_0,
      BCHP_INT_ID_WFE_TIMER1_0,
      BCHP_INT_ID_WFE_CORRDONE_0,
      BCHP_INT_ID_WFE_CLPDTR_PO_0,
      BCHP_INT_ID_WFE_CLPDTR_PI_0
   },
   /* wfe channel 1 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_1,
      BCHP_INT_ID_WFE_TIMER1_1,
      BCHP_INT_ID_WFE_CORRDONE_1,
      BCHP_INT_ID_WFE_CLPDTR_PO_1,
      BCHP_INT_ID_WFE_CLPDTR_PI_1
   },
   /* wfe channel 2 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_2,
      BCHP_INT_ID_WFE_TIMER1_2,
      BCHP_INT_ID_WFE_CORRDONE_2,
      BCHP_INT_ID_WFE_CLPDTR_PO_2,
      BCHP_INT_ID_WFE_CLPDTR_PI_2
   },
   /* wfe channel 3 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_3,
      BCHP_INT_ID_WFE_TIMER1_3,
      BCHP_INT_ID_WFE_CORRDONE_3,
      BCHP_INT_ID_WFE_CLPDTR_PO_3,
      BCHP_INT_ID_WFE_CLPDTR_PI_3
   },
   /* wfe channel 4 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_4,
      BCHP_INT_ID_WFE_TIMER1_4,
      BCHP_INT_ID_WFE_CORRDONE_4,
      BCHP_INT_ID_WFE_CLPDTR_PO_4,
      BCHP_INT_ID_WFE_CLPDTR_PI_4
   },
   /* wfe channel 5 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_5,
      BCHP_INT_ID_WFE_TIMER1_5,
      BCHP_INT_ID_WFE_CORRDONE_5,
      BCHP_INT_ID_WFE_CLPDTR_PO_5,
      BCHP_INT_ID_WFE_CLPDTR_PI_5
   },
   /* wfe channel 6 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_6,
      BCHP_INT_ID_WFE_TIMER1_6,
      BCHP_INT_ID_WFE_CORRDONE_6,
      BCHP_INT_ID_WFE_CLPDTR_PO_6,
      BCHP_INT_ID_WFE_CLPDTR_PI_6
   },
   /* wfe channel 7 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_7,
      BCHP_INT_ID_WFE_TIMER1_7,
      BCHP_INT_ID_WFE_CORRDONE_7,
      BCHP_INT_ID_WFE_CLPDTR_PO_7,
      BCHP_INT_ID_WFE_CLPDTR_PI_7
   },
   /* wfe channel 8 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_8,
      BCHP_INT_ID_WFE_TIMER1_8,
      BCHP_INT_ID_WFE_CORRDONE_8,
      BCHP_INT_ID_WFE_CLPDTR_PO_8,
      BCHP_INT_ID_WFE_CLPDTR_PI_8
   },
   /* wfe channel 9 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_9,
      BCHP_INT_ID_WFE_TIMER1_9,
      BCHP_INT_ID_WFE_CORRDONE_9,
      BCHP_INT_ID_WFE_CLPDTR_PO_9,
      BCHP_INT_ID_WFE_CLPDTR_PI_9
   },
   /* wfe channel 10 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_10,
      BCHP_INT_ID_WFE_TIMER1_10,
      BCHP_INT_ID_WFE_CORRDONE_10,
      BCHP_INT_ID_WFE_CLPDTR_PO_10,
      BCHP_INT_ID_WFE_CLPDTR_PI_10
   },
   /* wfe channel 11 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_11,
      BCHP_INT_ID_WFE_TIMER1_11,
      BCHP_INT_ID_WFE_CORRDONE_11,
      BCHP_INT_ID_WFE_CLPDTR_PO_11,
      BCHP_INT_ID_WFE_CLPDTR_PI_11
   },
   /* wfe channel 12 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_12,
      BCHP_INT_ID_WFE_TIMER1_12,
      BCHP_INT_ID_WFE_CORRDONE_12,
      BCHP_INT_ID_WFE_CLPDTR_PO_12,
      BCHP_INT_ID_WFE_CLPDTR_PI_12
   },
   /* wfe channel 13 interrupts */
   {
      BCHP_INT_ID_WFE_TIMER0_13,
      BCHP_INT_ID_WFE_TIMER1_13,
      BCHP_INT_ID_WFE_CORRDONE_13,
      BCHP_INT_ID_WFE_CLPDTR_PO_13,
      BCHP_INT_ID_WFE_CLPDTR_PI_13
   }
};


uint32_t BWFE_g2_RefChannelIntrID[BWFE_NUM_REF_CHANNELS][BWFE_g2_MaxIntID] = {
   /* mdac cal channel 0 interrupts */
   {
      BCHP_INT_ID_MDAC_CAL_TIMER0_0,
      BCHP_INT_ID_MDAC_CAL_TIMER1_0,
      0,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PO_0,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PI_0
   },
   /* mdac cal channel 1 interrupts */
   {
      BCHP_INT_ID_MDAC_CAL_TIMER0_1,
      BCHP_INT_ID_MDAC_CAL_TIMER1_1,
      0,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PO_1,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PI_1
   },
   /* mdac cal channel 2 interrupts */
   {
      BCHP_INT_ID_MDAC_CAL_TIMER0_2,
      BCHP_INT_ID_MDAC_CAL_TIMER1_2,
      0,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PO_2,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PI_2
   },
   /* mdac cal channel 3 interrupts */
   {
      BCHP_INT_ID_MDAC_CAL_TIMER0_3,
      BCHP_INT_ID_MDAC_CAL_TIMER1_3,
      0,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PO_3,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PI_3
   }
};


/******************************************************************************
 BWFE_P_InitConfig()
******************************************************************************/
BERR_Code BWFE_P_InitConfig(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   /* TBD chip specific initializations */
   
   return retCode;
}


/******************************************************************************
 BWFE_P_MapToRefRegister()
******************************************************************************/
BERR_Code BWFE_P_MapToRefRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   switch (reg)
   {
      case BCHP_WFE_CORE_HDOFFCTL0:
         *pAddr = BCHP_MDAC_CAL_CORE_HDOFFCTL0;
         break;
      case BCHP_WFE_CORE_HDOFFCTL1:
         *pAddr = BCHP_MDAC_CAL_CORE_HDOFFCTL1;
         break;
      case BCHP_WFE_CORE_HDOFFSTS:
         *pAddr = BCHP_MDAC_CAL_CORE_HDOFFSTS;
         break;
      case BCHP_WFE_CORE_DGSCTL_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCTL_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSCTL_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCTL_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSCLP_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCLP_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSCLP_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCLP_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSHIST_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSHIST_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSHIST_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSHIST_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSCLPCNT_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCLPCNT_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSCLPCNT_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCLPCNT_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSACCUM_DMX0_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSACCUM_DMX0_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSACCUM_DMX0_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSACCUM_DMX0_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSACCUM_DMX1_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSACCUM_DMX1_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSACCUM_DMX1_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSACCUM_DMX1_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT011_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT011_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT010_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT010_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT001_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT001_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT000_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT000_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT111_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT111_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT110_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT110_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT101_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT101_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT100_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT100_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT011_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT011_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT010_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT010_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT001_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT001_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT000_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT000_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT111_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT111_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT110_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT110_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT101_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT101_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLUT100_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLUT100_PO_SLC;
         break;
      case BCHP_WFE_CORE_DGSLMS_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLMS_SLC;
         break;
      case BCHP_WFE_CORE_DGSLMSMU_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSLMSMU_SLC;
         break;
      case BCHP_WFE_CORE_DGSCOEFD_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCOEFD_SLC;
         break;
      case BCHP_WFE_CORE_DGSCOEFA_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCOEFA_SLC;
         break;
      case BCHP_WFE_CORE_DGSCOEFEN_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCOEFEN_SLC;
         break;
      /* TBD BCHP_MDAC_CAL_CORE_DOUTCTL? */
      case BCHP_WFE_CORE_TIMERCTL0:
         *pAddr = BCHP_MDAC_CAL_CORE_TIMERCTL0;
         break;
      case BCHP_WFE_CORE_TIMERCTL1:
         *pAddr = BCHP_MDAC_CAL_CORE_TIMERCTL1;
         break;
      case BCHP_WFE_CORE_DGSEPCTL_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSEPCTL_SLC;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_DMX0_PI_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX0_PI_SLC_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_DMX0_PO_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX0_PO_SLC_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_DMX1_PI_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX1_PI_SLC_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_DMX1_PO_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX1_PO_SLC_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX0_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX0_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX0_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX0_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX0_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX0_PO_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX0_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX0_PO_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX1_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX1_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX1_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX1_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX1_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX1_PO_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX1_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX1_PO_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_AGCCTL2:
         *pAddr = BCHP_MDAC_CAL_CORE_AGCCTL2;
         break;
      case BCHP_WFE_CORE_AGCCTL1:
         *pAddr = BCHP_MDAC_CAL_CORE_AGCCTL1;
         break;
      case BCHP_WFE_CORE_AGCTHRA1:
         *pAddr = BCHP_MDAC_CAL_CORE_AGCTHRA1;
         break;
      case BCHP_WFE_CORE_REG_AGC_LF_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_AGC_LF_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_AGC_LA_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_AGC_LA_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_AGC_CTRL_LF_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_AGC_CTRL_LF_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_AGC_CTRL_LA_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_AGC_CTRL_LA_INT_WDATA;
         break;
      case BCHP_WFE_CORE_AGCTHRA2:
         *pAddr = BCHP_MDAC_CAL_CORE_AGCTHRA2;
         break;
      case BCHP_WFE_ANA_RFFE_SIGR01_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_RFFE_SIGR01_IN;
         break;
      case BCHP_WFE_ANA_RFFE_WRITER01_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_RFFE_WRITER01_IN;
         break;
      case BCHP_WFE_ANA_ADC_CNTL0:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL0;
         break;
      case BCHP_WFE_ANA_ADC_CNTL1:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL1;
         break;
      case BCHP_WFE_ANA_ADC_CNTL2:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL2;
         break;
      case BCHP_WFE_ANA_ADC_CNTL3:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL3;
         break;
      case BCHP_WFE_ANA_DPM_CNTL0_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_DPM_CNTL0_IN;
         break;
      case BCHP_WFE_ANA_DPM_CNTL1_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_DPM_CNTL1_IN;
         break;
      case BCHP_WFE_ANA_WBADC_CAL_CNTL_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_WBADC_CAL_CNTL_IN;
         break;
      case BCHP_WFE_ANA_WBADC_SYS_CNTL_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_WBADC_SYS_CNTL_IN;
         break;
      case BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_WBADC_TIMING_CNTL_IN;
         break;
      case BCHP_WFE_ANA_WBADC_TOP_CNTL_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_WBADC_TOP_CNTL_IN;
         break;
      case BCHP_WFE_ANA_CLK_CTRL_IN:
         *pAddr = BCHP_MDAC_CAL_ANA_CLK_CTRL_IN;
         break;
      case BCHP_WFE_ANA_SW_SPARE0:
         *pAddr = BCHP_MDAC_CAL_ANA_SW_SPARE0;
         break;
      case BCHP_WFE_ANA_SW_SPARE1:
         *pAddr = BCHP_MDAC_CAL_ANA_SW_SPARE1;
         break;
      default:
         retCode = BERR_INVALID_PARAMETER;
   }
   
   return retCode;
}


/******************************************************************************
 BWFE_P_GetRegisterAddress()
******************************************************************************/
BERR_Code BWFE_P_GetRegisterAddress(BWFE_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   BERR_Code retCode = BERR_SUCCESS;
   *pAddr = reg;
   
   if (!h->bReference)
   {
      if ((reg >= 0xC2000) && (reg <= 0xC222C))
      {
         /* WFE register access */
         *pAddr &= 0x000C03FF;
         *pAddr |= (h->channel & 0x1) << 10;
         *pAddr |= (h->channel / 2 + 1) << 13;
      }
      else if ((reg >= 0xC2800) && (reg <= 0xC285C))
      {
         /* WBADC register access */
         switch (reg)
         {
            case BCHP_WFE_ANA_RFFE_SIGR01_IN:
            case BCHP_WFE_ANA_RFFE_WRITER01_IN:
            case BCHP_WFE_ANA_DPM_CNTL0_IN:
            case BCHP_WFE_ANA_DPM_CNTL1_IN:
            case BCHP_WFE_ANA_WBADC_CAL_CNTL_IN:
            case BCHP_WFE_ANA_WBADC_SYS_CNTL_IN:
            case BCHP_WFE_ANA_WBADC_TIMING_CNTL_IN:
            case BCHP_WFE_ANA_WBADC_TOP_CNTL_IN:
            case BCHP_WFE_ANA_CLK_CTRL_IN:
               *pAddr = reg + (h->channel & 0x1) * 0x4;
               break;
         }
         
         *pAddr &= 0x000C0FFF;
         *pAddr |= (h->channel / 2 + 1) << 13;
      }
   }
   else
   {
      /* reference adc register access */
      retCode = BWFE_P_MapToRefRegister(h, reg, pAddr);
      if (retCode)
      {
         BDBG_WRN(("BWFE_GetRegisterAddress: invalid reference register!"));
         return retCode;
      }
      
      reg = *pAddr;
      if ((reg >= 0xD0000) && (reg <= 0xD012C))
      {
         /* MDAC_CAL register access*/
         /* *pAddr &= 0x000F01FF; */
         *pAddr |= h->channel << 9;
      }
      else if ((reg >= 0xD0800) && (reg <= 0xD08A4))
      {
         /* MDAC_CAL WBADC register access */
         switch (reg)
         {
            case BCHP_MDAC_CAL_ANA_RFFE_SIGR01_IN:
            case BCHP_MDAC_CAL_ANA_RFFE_WRITER01_IN:
            case BCHP_MDAC_CAL_ANA_DPM_CNTL0_IN:
            case BCHP_MDAC_CAL_ANA_DPM_CNTL1_IN:
            case BCHP_MDAC_CAL_ANA_WBADC_CAL_CNTL_IN:
            case BCHP_MDAC_CAL_ANA_WBADC_SYS_CNTL_IN:
            case BCHP_MDAC_CAL_ANA_WBADC_TIMING_CNTL_IN:
            case BCHP_MDAC_CAL_ANA_WBADC_TOP_CNTL_IN:
            case BCHP_MDAC_CAL_ANA_CLK_CTRL_IN:
               *pAddr = reg + (h->channel * 0x4);
               break;
         }
      }
   }
   
   /* BDBG_MSG(("GetRegisterAddress(%d): %08X -> %08X", h->channel, reg, *pAddr)); */
   return retCode;
}


/******************************************************************************
 BWFE_P_ReadRegister()
******************************************************************************/
BERR_Code BWFE_P_ReadRegister(
   BWFE_ChannelHandle h,      /* [in] BWFE channel handle */
   uint32_t           reg,    /* [in] address of register to read */
   uint32_t           *val    /* [out] contains data that was read */
)
{
   uint32_t addr;
   
   BWFE_P_GetRegisterAddress(h, reg, &addr);
   *val = BREG_Read32(h->pDevice->hRegister, addr); 
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_WriteRegister()
******************************************************************************/
BERR_Code BWFE_P_WriteRegister(
   BWFE_ChannelHandle h,      /* [in] BWFE channel handle */
   uint32_t           reg,    /* [in] address of register to write */
   uint32_t           val     /* [in] contains data to be written */
)
{
   uint32_t addr;

   BWFE_P_GetRegisterAddress(h, reg, &addr);
   BREG_Write32(h->pDevice->hRegister, addr, val);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_GetSliceRegisterAddress()
******************************************************************************/
BERR_Code BWFE_P_GetSliceRegisterAddress(BWFE_ChannelHandle h, uint8_t slice, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;
   
   BSTD_UNUSED(h);
   BDBG_ASSERT(slice < BWFE_NUM_SLICES);
   BSTD_UNUSED(reg);
   BSTD_UNUSED(pAddr);
   
#if (BCHP_CHIP == 4538)
   /* slice mappings */
   switch (reg)
   {
      case BCHP_WFE_CORE_DGSCTL_PI_SLC:
      case BCHP_WFE_CORE_DGSCTL_PO_SLC:
      case BCHP_WFE_CORE_DGSCLP_PI_SLC:
      case BCHP_WFE_CORE_DGSCLP_PO_SLC:
      case BCHP_WFE_CORE_DGSHIST_PI_SLC:
      case BCHP_WFE_CORE_DGSHIST_PO_SLC:
      case BCHP_WFE_CORE_DGSCLPCNT_PI_SLC:
      case BCHP_WFE_CORE_DGSCLPCNT_PO_SLC:
      case BCHP_WFE_CORE_DGSACCUM_DMX0_PI_SLC:
      case BCHP_WFE_CORE_DGSACCUM_DMX0_PO_SLC:
      case BCHP_WFE_CORE_DGSACCUM_DMX1_PI_SLC:
      case BCHP_WFE_CORE_DGSACCUM_DMX1_PO_SLC:
      case BCHP_WFE_CORE_DGSLMS_SLC:
      case BCHP_WFE_CORE_DGSLMSMU_SLC:
      case BCHP_WFE_CORE_DGSCOEFD_SLC:
      case BCHP_WFE_CORE_DGSCOEFA_SLC:
      case BCHP_WFE_CORE_DGSCOEFEN_SLC:
      case BCHP_WFE_CORE_LICDCO_SLC:
         *pAddr = reg + slice * 0x4;
         break;
      
      case BCHP_WFE_CORE_DGSLUT011_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT010_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT001_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT000_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT111_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT110_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT101_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT100_PI_SLC:
      case BCHP_WFE_CORE_DGSLUT011_PO_SLC:
      case BCHP_WFE_CORE_DGSLUT010_PO_SLC:
      case BCHP_WFE_CORE_DGSLUT001_PO_SLC:
      case BCHP_WFE_CORE_DGSLUT000_PO_SLC:
      case BCHP_WFE_CORE_DGSLUT111_PO_SLC:
      case BCHP_WFE_CORE_DGSLUT110_PO_SLC:
      case BCHP_WFE_CORE_DGSLUT101_PO_SLC:
      case BCHP_WFE_CORE_DGSLUT100_PO_SLC:
         *pAddr = reg + slice * 0x40;
         break;
      
      case BCHP_WFE_CORE_CORRI0_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRI0_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRI0_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRI0_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX1_PO_SLC:
         *pAddr = reg + slice * 0x8;
         break;
      
      case BCHP_WFE_CORE_DGSEPCTL_SLC:
      case BCHP_WFE_CORE_REG_DGSEP_DMX0_PI_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX0_PO_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX1_PI_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX1_PO_SLC_INT_WDATA:	
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX0_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX0_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX0_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX0_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX1_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX1_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX1_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX1_PO_SLC_ERRP:
         *pAddr = reg + slice * 0x34;
         break;
      
      case BCHP_WFE_CORE_NRDCOCTL_PI_SLC:
      case BCHP_WFE_CORE_REG_NR_DCO_INT_WDATA_PI_SLC:
      case BCHP_WFE_CORE_NRDCOCTL_PO_SLC:
      case BCHP_WFE_CORE_REG_NR_DCO_INT_WDATA_PO_SLC:
      case BCHP_WFE_CORE_NRNOTCHCTL_PI_SLC:
      case BCHP_WFE_CORE_REG_NR_NOTCH_INT_WDATA_PI_SLC:
      case BCHP_WFE_CORE_NRNOTCHCTL_PO_SLC:
      case BCHP_WFE_CORE_REG_NR_NOTCH_INT_WDATA_PO_SLC:
         *pAddr = reg + slice * 0x10;
         break;
      
      case BCHP_WFE_CORE_NRAGCCTL_PI_SLC:
      case BCHP_WFE_CORE_REG_NR_AGC_LF_INT_WDATA_PI_SLC:
      case BCHP_WFE_CORE_REG_NR_AGC_LA_INT_WDATA_PI_SLC:
      case BCHP_WFE_CORE_NRAGCCTL_PO_SLC:
      case BCHP_WFE_CORE_REG_NR_AGC_LF_INT_WDATA_PO_SLC:
      case BCHP_WFE_CORE_REG_NR_AGC_LA_INT_WDATA_PO_SLC:
         *pAddr = reg + slice * 0x18;
         break;
      
      default:
         return BERR_INVALID_PARAMETER;
   }
#endif
   
   /* BDBG_MSG(("GetSliceRegisterAddress(%d) %08X -> %08X", slice, reg, *pAddr)); */
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_ReadSliceRegister()
******************************************************************************/
BERR_Code BWFE_P_ReadSliceRegister(
   BWFE_ChannelHandle h,     /* [in] BWFE channel handle */
   uint8_t            slice, /* [in] slice number */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [out] contains data that was read */
)
{
   if (slice >= BWFE_NUM_SLICES)
      return BERR_INVALID_PARAMETER;
   
#if (BCHP_CHIP == 4538)
   /* look up slice register */
   BWFE_CHK_RETCODE(BWFE_P_GetSliceRegisterAddress(h, slice, reg, &addr));
   reg = addr;
#endif
   
   /* map register to channel */
   return BWFE_P_ReadRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_WriteSliceRegister()
******************************************************************************/
BERR_Code BWFE_P_WriteSliceRegister(
   BWFE_ChannelHandle h,     /* [in] BWFE channel handle */
   uint8_t            slice, /* [in] slice number */
   uint32_t           reg,   /* [in] address of register to write */
   uint32_t           val    /* [in] contains data to be written */
)
{
   if (slice >= BWFE_NUM_SLICES)
      return BERR_INVALID_PARAMETER;
   
#if (BCHP_CHIP == 4538)
   /* look up slice register */
   BWFE_CHK_RETCODE(BWFE_P_GetSliceRegisterAddress(h, slice, reg, &addr));
   reg = addr;
#endif
   
   /* map register to channel */
   return BWFE_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_SetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_P_SetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t freqKhz)
{
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t ndiv_int, val;
   
   BSTD_UNUSED(hDev);
   
#if 1
   /* limit possible frequencies based on 54MHz */
   if (freqKhz % (BWFE_XTAL_FREQ_KHZ / 2) != 0)
      return BERR_INVALID_PARAMETER;
#else
   /* limit possible frequencies based on 54MHz */
   switch (freqKhz)
	{
      case 1728000:
      case 1755000:
      case 1782000:
      case 1809000:
      case 1836000:
      case 1863000:
      case 1890000:
      case 1917000:
      case 1944000:
      case 1971000:
      case 1998000:
      case 2025000:
         break;
      default:
         return BERR_INVALID_PARAMETER;
   }
#endif
   
   /* PLL initializiation sequence */
   /* 1) After power-up, make sure to reset the PLL by bit i_resetb and i_post_resetb. */
   /* 2) If post-divider channels are used, please use i_load_en_ch<2:0> to load the correct settings of the post-divider. */
   /* 3) Wait until the output bit o_lock becomes high  this indicates the PLL is locked. */
   
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_RST, 0x00000003);       /* pll global reset and post-div reset */
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_RST, 0x00000000);       /* release resets */
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_GAIN_CTRL, 0x0000033A); /* gain settings to reduce phase noise */
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_MISC_CTRL, 0x000A8C20); /* make sure clkout_sel is vco_clock/2 */
   
   /* calculate ndiv_int from freqKhz, ndiv_int = (Fs_adc * 4) / (54MHz * 2) */
   ndiv_int = (freqKhz << 1) / BWFE_XTAL_FREQ_KHZ;

   /* Fclk_rx_pll_vco = 54 MHz * (2 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20)) */
   /* Fs_adc = Fclk_rx_pll_vco/4 due to hardwired divs inside pll */
   /* digital rx_clk = Fclk_rx_pll_vco/(2 * RX_PLL_CLK_FS) = Fs_adc/4 */
   /* Example: Pdiv=1, Ndiv_int=66 -> Fclk_rx_pll_vco=7128MHz, Fs_adc=Fclk_rx_pll_vco/4=1782MHz, digital rx_clk = 445MHz  (RX_PLL_CLK_FS=8) */
   
   /* set pll dividers for rx clk */
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_PDIV, 0x00000001);            /* default prediv = 1 */
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_NDIV_INT, ndiv_int & 0xFF);   /* Ndiv_int=66 for 1782MHz */
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_NDIV_FRAC, 0x00000000);       /* Ndiv_frac=0 */
   BREG_Write32(hDev->hRegister, BCHP_TM_RX_PLL_CLK_FS, 0x00000308);          /* enable sample clk, post-div by 8 */
   
   /* wait for pll lock */
   BKNI_Sleep(5);
   
   /* check pll status */
   val = BREG_Read32(hDev->hRegister, BCHP_TM_RX_PLL_STATUS);
   if ((val & BCHP_TM_RX_PLL_STATUS_LOCK_MASK) == 0)
      BDBG_ERR(("rx pll NOT locked!"));
   
   /* enable wfe micros */
   val = BREG_Read32(hDev->hRegister, BCHP_TM_REG_CLK_EN);
   val |= 0x00000007;
   BREG_Write32(hDev->hRegister, BCHP_TM_REG_CLK_EN, val);
   
   return retCode;
}


/******************************************************************************
 BWFE_P_GetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_P_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz)
{
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t pdiv, ndiv_int, ndiv_frac;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   
   BSTD_UNUSED(hDev);
   
   pdiv = BREG_Read32(hDev->hRegister, BCHP_TM_RX_PLL_PDIV) & 0xF;
   ndiv_int = BREG_Read32(hDev->hRegister, BCHP_TM_RX_PLL_NDIV_INT) & 0xFF;
   ndiv_frac = BREG_Read32(hDev->hRegister, BCHP_TM_RX_PLL_NDIV_FRAC) & 0xFFFFF;
   
   /* Fclk_rx_pll_vco = 54 MHz * (2 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20)) */
   /* equivalent to (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) / (2^20 * Pdiv) */
   /* Fs_adc = Fclk_rx_pll_vco / 4 */
   
   BMTH_HILO_32TO64_Mul(BWFE_XTAL_FREQ_KHZ << 1, (ndiv_int << 20) + ndiv_frac, &P_hi, &P_lo);  /* (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 4 * pdiv << 20, &Q_hi, &Q_lo);  /* div by (2^20 * Pdiv) and additional factor of 4 for Fs_adc */
   
   /* BDBG_MSG(("Fs_adc=%d000", Q_lo)); */
   *freqKhz = Q_lo;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_SetDpmPilotFreq()
******************************************************************************/
BERR_Code BWFE_P_SetDpmPilotFreq(BWFE_Handle h, uint32_t freqKhz)
{
   BWFE_g2_P_Handle *hDev = h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, start_m, end_m, m, n, m_min, n_min, div, div_min, nlane, n_xtal, m_xtal, gcd;
   int32_t x, x_min; 

   gcd = BWFE_P_GCF(BWFE_DEF_FS_ADC_KHZ, BWFE_XTAL_FREQ_KHZ);
   n_xtal = BWFE_DEF_FS_ADC_KHZ / gcd;
   m_xtal = BWFE_XTAL_FREQ_KHZ / gcd;

   nlane = BWFE_NUM_LANES * BWFE_NUM_DMX_LANES;

   x_min = 0x7FFFFFFF;
   m_min = n_min = div_min = 0; /* needed to remove compiler warning */
   for (div = 2; div <= 3; div++)
   {
      start_m = ((div == 2) ? 75 : 112);
      end_m = ((div == 2) ? 148 : 222);

      for (m = start_m; m <= end_m; m++)
      {
         n = (((BWFE_XTAL_FREQ_KHZ * m * 2) / (div * freqKhz)) + 1) >> 1; /* round */
         if (n <= 256)
         {
            x = freqKhz - ((((BWFE_XTAL_FREQ_KHZ * m * 2) / (div * n)) + 1) >> 1);
            if (x < 0)
               x = -x;
            if (x < x_min)
            {
               x_min = x;
               m_min = m;
               n_min = n;
               div_min = div;
            }
         }
      }
   }

   m = m_min;
   n = n_min;
   div = div_min;

   hDev->dpmQddfsN = n * div * n_xtal;

   x = m * nlane * m_xtal;
   hDev->dpmQddfsM = x - ((((x * 2) / hDev->dpmQddfsN) + 1) >> 1) * hDev->dpmQddfsN;
   
   /* remove the common divider */
   gcd = BWFE_P_GCF(BWFE_ABS(hDev->dpmQddfsM), BWFE_ABS(hDev->dpmQddfsN));
   if (gcd > 1)
   {
      hDev->dpmQddfsM /= (int32_t)gcd;
      hDev->dpmQddfsN /= (int32_t)gcd;
   }
   BDBG_MSG(("BWFE_P_SetDpmPilotFreq(%d KHz): DPM_m=%d, DPM_n=%d, DPM_div=%d, dpmQddfsN=%d, dpmQddfsM=%d", freqKhz, m, n, div, hDev->dpmQddfsN, hDev->dpmQddfsM));
   
   /* power up dpm pll */
   val = BREG_Read32(h->hRegister, BCHP_TM_PWRDN);
   val &= ~BCHP_TM_PWRDN_DPM_PLL_MASK;
   BREG_Write32(h->hRegister, BCHP_TM_PWRDN, val);
   
   /* PLL initializiation sequence */
   /* 1) After power-up, make sure to reset the PLL by bit i_resetb and i_post_resetb. */
   /* 2) If post-divider channels are used, please use i_load_en_ch<2:0> to load the correct settings of the post-divider. */
   /* 3) Wait until the output bit o_lock becomes high  this indicates the PLL is locked. */
   
   BREG_Write32(hDev->hRegister, BCHP_TM_DPM_PLL_RST, 0x00000003);      /* pll global reset and post-div reset */
   BREG_Write32(hDev->hRegister, BCHP_TM_DPM_PLL_RST, 0x00000000);      /* release resets */ 
  
   /* note Fclk_dpm_pll_vco freq range: 2 - 4GHz */
   /* Fclk_dpm_pll_vco = 54 MHz * (1 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^24)) */
   /* Fc_dpm = Fclk_dpm_pll_vco / pilot_div */
   
   /* set pll dividers for dpm clk */
   BREG_Write32(hDev->hRegister, BCHP_TM_DPM_PLL_PDIV, div);                  /* default prediv = 1 */
   BREG_Write32(hDev->hRegister, BCHP_TM_DPM_PLL_NDIV_INT, m & 0x1FF);        /* set Ndiv_int to calculated m */
   BREG_Write32(hDev->hRegister, BCHP_TM_DPM_PLL_NDIV_FRAC, 0x00000000);      /* Ndiv_frac=0 */
   BREG_Write32(hDev->hRegister, BCHP_TM_DPM_PLL_CLK_PILOT, 0x00000300 | n);  /* enable pilot clk, set post-div to calculated n */

   /* wait for pll lock */
   BKNI_Sleep(5);
   
   /* check pll status */
   val = BREG_Read32(hDev->hRegister, BCHP_TM_DPM_PLL_STATUS);
   if ((val & BCHP_TM_DPM_PLL_STATUS_LOCK_MASK) == 0)
      BDBG_ERR(("dpm pll NOT locked!"));
   
   return retCode;
}


/******************************************************************************
 BWFE_P_GetDpmPilotFreq()
******************************************************************************/
BERR_Code BWFE_P_GetDpmPilotFreq(BWFE_Handle h, uint32_t *freqKhz)
{
   BWFE_g2_P_Handle *hDev = h->pImpl;
   uint32_t pdiv, pilot_div;
   uint32_t ndiv_int;
   uint32_t ndiv_frac;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   
   BSTD_UNUSED(hDev);
   
   pdiv = BREG_Read32(hDev->hRegister, BCHP_TM_DPM_PLL_PDIV) & 0xF;   
   ndiv_int = BREG_Read32(hDev->hRegister, BCHP_TM_RX_PLL_NDIV_INT) & 0x1FF;
   ndiv_frac = BREG_Read32(hDev->hRegister, BCHP_TM_RX_PLL_NDIV_FRAC) & 0xFFFFFF;
   pilot_div = BREG_Read32(hDev->hRegister, BCHP_TM_DPM_PLL_CLK_PILOT) & 0xFF;
   
   /* Fclk_dpm_pll_vco = 54 MHz * (1 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^24)) */
   /* equivalent to (54 MHz * 1) * (2^24 * Ndiv_int + Ndiv_frac) / (2^24 * Pdiv) */
   /* Fc_dpm = Fclk_dpm_pll_vco / pilot_div */
   
   BMTH_HILO_32TO64_Mul(BWFE_XTAL_FREQ_KHZ, (ndiv_int << 24) + ndiv_frac, &P_hi, &P_lo);  /* (54 MHz * 1) * (2^24 * Ndiv_int + Ndiv_frac) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, pdiv << 24, &Q_hi, &Q_lo);  /* div by (2^24 * Pdiv) and additional pilot_div for Fc_dpm */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, pilot_div, &Q_hi, &Q_lo); 
   /* BDBG_MSG(("Fc_dpm=%d000", Q_lo)); */
   *freqKhz = Q_lo;
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_InitAdc()
******************************************************************************/
BERR_Code BWFE_P_InitAdc(BWFE_ChannelHandle h)
{  
   uint32_t val;
   uint8_t mdac2_pwr, mdac1_pwr, sha_pwr, buf_pwr;
   
   /* increase cml clock buffer bias currents */
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML0_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML1_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML2_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML3_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML4_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML5_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML6_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML7_CTRL, 0x00002957);
   BREG_Write32(h->pDevice->hRegister, BCHP_TM_CKCML8_CTRL, 0x00002957);
   
   mdac2_pwr = 0x3;  /* default mdac 2 current */
   mdac1_pwr = 0x3;  /* default mdac 1 current */
   
   sha_pwr = 0x6;    /* I(uA) = (3 - x) * 40uA + 200uA */
   buf_pwr = 0x6;    /* I(uA) = (3 - x) * 40uA + 200uA */
   
   /* updated power settings from Jaffe */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, 0x5B65B556);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL2, 0x11450ADA);
   
   /* chip specific current settings */
   val = (mdac2_pwr << 21) | (mdac1_pwr << 18) | (sha_pwr << 15) | (buf_pwr << 12);
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, ~0x00FFF000, val & 0x00FFF000);
   
   /* power up adc */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_CLK_CTRL_IN, ~0x0000011F); /* reset all clocks */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x0007FFFF); /* default power up state */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00000400); /* release clkgen reset */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00000001); /* power up ADC */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00040000); /* release DGS reset */
   
   /* wait for ADC power up */
   BKNI_Sleep(1);

   /* must toggle clkgen and dgs reset again */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x00040400);
   BKNI_Delay(500);  /* wait 500us */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00040400); /* release clkgen and dgs reset */
   BKNI_Sleep(1);    /* wait 1ms after reset due to analog level shifters at cold */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_CLK_CTRL_IN, 0x0000011F); /* re-enable all clocks */

   /* enable ADC output, use negative edge */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_HDOFFCTL0, 0x0020000F);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_ShutdownAdc()
******************************************************************************/
BERR_Code BWFE_P_ShutdownAdc(BWFE_ChannelHandle h)
{
   /* gate off adc output */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_HDOFFCTL0, ~0x00200000);
   
   /* power down ADC and hold in reset */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x00000401);
   
   /* disable wbadc and mdac clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_CLK_CTRL_IN, ~0x00000019);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_CalcCorrPower()
******************************************************************************/
#ifdef BWFE_FLOAT_MODE
BERR_Code BWFE_P_CalcCorrPower(BWFE_ChannelHandle h, double *corrI, double *corrQ, uint32_t *corrPowerDb)
{
   double vec[2], phase, corrPower;
   double cos_k, sin_k, tmp_i, tmp_q;
   
#if 1
   /* theta = 2 * Fc / Fadc */
   phase = 2.0 * (double)(h->corrFreqHz / 1000) / (double)h->pDevice->adcSampleFreqKhz;
   BWFE_P_CordicRotationMode(vec, phase, true);
   cos_k = vec[0];
   sin_k = vec[1];
   
   /* phase0: corr[0] * exp(-1*j*2pi*fc/fs) ... negate sin term  */
   /* phase1: corr[1] * exp( 0*j*2pi*fc/fs) ... no rotation, so just init I & Q total */
   /* phase2: corr[2] * exp(+1*j*2pi*fc/fs)  */
   tmp_i = corrI[1] + (corrI[0] + corrI[2]) * cos_k  + (corrQ[0] - corrQ[2]) * sin_k;
   tmp_q = corrQ[1] + (corrQ[0] + corrQ[2]) * cos_k  + (corrI[2] - corrI[0]) * sin_k;
   
   /* phase3: corr[3] * exp(+2* j*2*pi*fc/fs) ... use double-angle formulas */
   sin_k = 2.0 * cos_k * sin_k;        /* sin(2x) = 2*cos(x)*sin(x) */
   cos_k = 2.0 * cos_k * cos_k - 1.0;  /* cos(2x) = 2*cos(x)^2 - 1  */
   tmp_i += (corrI[3] * cos_k - corrQ[3] * sin_k);
   tmp_q += (corrQ[3] * cos_k + corrI[3] * sin_k);
#else
   uint8_t k;
   
   tmp_i = 0;
   tmp_q = 0;
   
   for (k = 0; k < BWFE_LIC_L; k++)
   {
      /* theta = 2 * k * Fc / Fadc */
      phase = 2.0 * k * (double)(h->corrFreqHz / 1000) / (double)h->pDevice->adcSampleFreqKhz;
      if (phase >= 1.0) phase -= 2.0;   /* mod phase to within 2*pi */
      BWFE_P_CordicRotationMode(vec, phase, true);
      cos_k = vec[0];
      sin_k = vec[1];
      
      /* rotate by phase */
      tmp_i += corrI[k] * cos_k - corrQ[k] * sin_k;
      tmp_q += corrI[k] * sin_k + corrQ[k] * cos_k;
   }
#endif
   
   corrPower = (tmp_i * tmp_i + tmp_q * tmp_q); /* TBD check scaling: scale by correlation period? */
   //printf("corrPower=");
   //printDouble(corrPower, 5);
   //printf("\n");
   
   /* calculate power in dB */
   *corrPowerDb = BMTH_2560log10((uint32_t)corrPower);
   
   return BERR_SUCCESS;
}

#else

BERR_Code BWFE_P_CalcCorrPower(BWFE_ChannelHandle h, int32_t *corrI, int32_t *corrQ, uint32_t *corrPowerDb)
{
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   
   uint32_t corrPower;
   int32_t vec[2], phase;
   int32_t cos_k, sin_k, tmp_i, tmp_q;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   uint32_t R1_hi, R1_lo, R2_hi, R2_lo;
   
   /* theta = Fc / Fadc */
   BMTH_HILO_32TO64_Mul((hChn->corrFreqHz / 500), 2147483648UL, &P_hi, &P_lo);   /* Fc scaled up by 2^32 */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hDev->adcSampleFreqKhz, &Q_hi, &Q_lo);  /* div by Fadc */
   phase = Q_lo;
   BWFE_P_CordicRotationMode(vec, phase, true);
   cos_k = vec[0];
   sin_k = vec[1];
   
   /* phase0: corr[0] * exp(-1*j*2pi*fc/fs) ... negate sin term  */
   /* phase1: corr[1] * exp( 0*j*2pi*fc/fs) ... no rotation, so just init I & Q total */
   /* phase2: corr[2] * exp(+1*j*2pi*fc/fs)  */
   tmp_i = corrI[1];
   /* tmp_i += ((corrI[0] + corrI[2]) * cos_k + (corrQ[0] - corrQ[2]) * sin_k) >> BWFE_CORDIC_SHIFT */
   BWFE_P_SumThenMultiply(corrI[0], corrI[2], cos_k, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(corrQ[0], -corrQ[2], sin_k, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_i += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   tmp_q = corrQ[1];
   /* tmp_q += ((corrQ[0] + corrQ[2]) * cos_k + (corrI[2] - corrI[0]) * sin_k) >> BWFE_CORDIC_SHIFT */
   BWFE_P_SumThenMultiply(corrQ[0], corrQ[2], cos_k, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(corrI[2], -corrI[0], sin_k, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_q += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   /* phase3: corr[3] * exp(+2* j*2*pi*fc/fs) ... use double-angle formulas */
   BWFE_P_SumThenMultiply(0, cos_k, sin_k, &R1_hi, &R1_lo);    /* sin(2x) = 2*cos(x)*sin(x) */
   sin_k = (R1_hi << (32 - BWFE_CORDIC_SHIFT + 1)) | (R1_lo >> (BWFE_CORDIC_SHIFT - 1));  /* undo double cordic scale, multiply by 2 */
   BWFE_P_SumThenMultiply(0, cos_k, cos_k, &R2_hi, &R2_lo);    /* cos(2x) = 2*cos(x)^2 - 1  */
   cos_k = (R2_hi << (32 - BWFE_CORDIC_SHIFT + 1)) | (R2_lo >> (BWFE_CORDIC_SHIFT - 1));  /* undo double cordic scale, multiply by 2 */
   cos_k -= BWFE_CORDIC_SCALE;
   
   /* tmp_i += (corrI[3] * cos_k - corrQ[3] * sin_k); */
   BWFE_P_SumThenMultiply(0, corrI[3], cos_k, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(0, -corrQ[3], sin_k, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_i += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   /* tmp_q += (corrQ[3] * cos_k + corrI[3] * sin_k); */
   BWFE_P_SumThenMultiply(0, corrQ[3], cos_k, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(0, corrI[3], sin_k, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_q += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   //corrPower = (tmp_i * tmp_i + tmp_q * tmp_q); /* TBD check scaling: scale by correlation period? */
   BWFE_P_SumThenMultiply(0, tmp_i, tmp_i, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(0, tmp_q, tmp_q, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   corrPower = P_hi;
   
   /* BDBG_MSG(("tmp_i=%08X, tmp_q=%08X, corrPower=%08X", tmp_i, tmp_q, corrPower)); */
   *corrPowerDb = BMTH_2560log10(corrPower);
   
   return BERR_SUCCESS;
}

#endif
