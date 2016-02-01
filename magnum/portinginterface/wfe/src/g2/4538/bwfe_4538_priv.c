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
#include "bwfe_4538_priv.h"

BDBG_MODULE(bwfe_4538_priv);


uint32_t BWFE_g2_ChannelIntrID[BWFE_NUM_CHANNELS][BWFE_g2_MaxIntID] = {
   {
      /* wfe channel 0 interrupts */
      BCHP_INT_ID_WFE_TIMER0_0,
      BCHP_INT_ID_WFE_TIMER1_0,
      BCHP_INT_ID_WFE_CORRDONE_0,
      BCHP_INT_ID_WFE_CLPDTR_PO_SLC0_0,
      BCHP_INT_ID_WFE_CLPDTR_PI_SLC0_0
   },
   {
      /* wfe channel 1 interrupts */
      BCHP_INT_ID_WFE_TIMER0_1,
      BCHP_INT_ID_WFE_TIMER1_1,
      BCHP_INT_ID_WFE_CORRDONE_1,
      BCHP_INT_ID_WFE_CLPDTR_PO_SLC0_1,
      BCHP_INT_ID_WFE_CLPDTR_PI_SLC0_1
   },
   {
      /* wfe channel 2 interrupts */
      BCHP_INT_ID_WFE_TIMER0_2,
      BCHP_INT_ID_WFE_TIMER1_2,
      BCHP_INT_ID_WFE_CORRDONE_2,
      BCHP_INT_ID_WFE_CLPDTR_PO_SLC0_2,
      BCHP_INT_ID_WFE_CLPDTR_PI_SLC0_2
   },
   {
      /* wfe channel 3 interrupts */
      BCHP_INT_ID_WFE_TIMER0_3,
      BCHP_INT_ID_WFE_TIMER1_3,
      BCHP_INT_ID_WFE_CORRDONE_3,
      BCHP_INT_ID_WFE_CLPDTR_PO_SLC0_3,
      BCHP_INT_ID_WFE_CLPDTR_PI_SLC0_3
   }
};


uint32_t BWFE_g2_RefChannelIntrID[BWFE_NUM_REF_CHANNELS][BWFE_g2_MaxIntID] = {
   {
      /* mdac cal channel 0 interrupts */
      BCHP_INT_ID_MDAC_CAL_TIMER0_0,
      BCHP_INT_ID_MDAC_CAL_TIMER1_0,
      0,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PO_0,
      BCHP_INT_ID_MDAC_CAL_CLPDTR_PI_0
   },
};


/******************************************************************************
 BWFE_P_InitConfig()
******************************************************************************/
BERR_Code BWFE_P_InitConfig(BWFE_ChannelHandle h)
{
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i;
   
   /* override default parameters */
   hChn->rfAgcThresh = 0xD800;
   
   /* center analog delay compensation initially */
   for (i = 0; i < BWFE_NUM_SLICES; i++)
      hChn->analogDelay[i] = 0x280;
   
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
      case BCHP_WFE_CORE_RSTCTL:
         *pAddr = BCHP_MDAC_CAL_CORE_RSTCTL;
         break;
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
      if ((reg >= 0xB0000) && (reg <= 0xB104C))
      {
         /* WFE and WBADC register access */
         *pAddr &= 0x000F1FFF;
         *pAddr |= (h->channel & 0x3) << 14;
      }
   }
   else
   {
      /* reference adc register access */
      retCode = BWFE_P_MapToRefRegister(h, reg, pAddr);
      if (retCode)
      {
         BDBG_WRN(("BWFE_GetRegisterAddress(%d): %08X invalid reference register!", h->channel, reg));
         return retCode;
      }
   }
   
   /*BDBG_MSG(("GetRegisterAddress(%d): %08X -> %08X", h->channel, reg, *pAddr)); */
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
      case BCHP_WFE_CORE_DGSCOEFD_SLC:
      case BCHP_WFE_CORE_DGSCOEFA_SLC:
         *pAddr = reg + slice * 0x8;
         break;
      
      case BCHP_WFE_CORE_DGSLMS_SLC:
      case BCHP_WFE_CORE_DGSLMSMU_SLC:
      case BCHP_WFE_CORE_DGSCOEFEN_SLC:
      case BCHP_WFE_CORE_MDACSA_SLC:
      case BCHP_WFE_CORE_MDACSADU_SLC:
      case BCHP_WFE_CORE_MDACSAOUT_SLC:
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
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr;
   
   if (slice >= BWFE_NUM_SLICES)
      return BERR_INVALID_PARAMETER;
   
   /* look up slice register */
   BWFE_CHK_RETCODE(BWFE_P_GetSliceRegisterAddress(h, slice, reg, &addr));
   reg = addr;
   
   /* map register to channel */
   BWFE_CHK_RETCODE(BWFE_P_ReadRegister(h, reg, val));
   
   done:
   return retCode;
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
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr;
   
   if (slice >= BWFE_NUM_SLICES)
      return BERR_INVALID_PARAMETER;
   
   /* look up slice register */
   BWFE_CHK_RETCODE(BWFE_P_GetSliceRegisterAddress(h, slice, reg, &addr));
   reg = addr;
   
   /* map register to channel */
   BWFE_CHK_RETCODE(BWFE_P_WriteRegister(h, reg, val));
   
   done:
   return retCode;
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
   
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_RST, 0x00000003);         /* pll global reset and post-div reset */
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_RST, 0x00000000);         /* release resets */
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_GAIN_CTRL, 0x00000318);   /* default gain settings */
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_MISC_CTRL, 0x80082000);   /* default misc settings */
#if (BCHP_VER>=BCHP_VER_C0)
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_LDO_CTRL, 0x00000015);    /* optimal ldo settings */
#else
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_LDO_CTRL, 0x00000205);    /* optimal ldo settings */
#endif
   
   /* calculate ndiv_int from freqKhz, ndiv_int = (Fs_adc) / (54MHz * 2) */
   ndiv_int = (freqKhz >> 1) / BWFE_XTAL_FREQ_KHZ;

   /* Fclk_rx_pll_vco = 54 MHz * (2 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20)) */
   /* Fs_adc = Fclk_rx_pll_vco/4 due to hardwired divs inside pll */
   /* digital rx_clk = Fclk_rx_pll_vco/(2 * RX_PLL_CLK_FS) = Fs_adc/4 */
   /* Example: Pdiv=1, Ndiv_int=66 -> Fclk_rx_pll_vco=7128MHz, Fs_adc=Fclk_rx_pll_vco/4=1782MHz, digital rx_clk = 445MHz  (RX_PLL_CLK_FS=8) */
   
   /* set pll dividers for rx clk */
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_PDIV, 0x00000001);            /* default prediv = 1 */
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_NDIV_INT, ndiv_int & 0xFF);   /* Ndiv_int=45 for 4860MHz */
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_NDIV_FRAC, 0x00000000);       /* Ndiv_frac=0 */
   BREG_Write32(hDev->hRegister, BCHP_TM_ANA_PLL_CLK_CH0, 0x00010308);         /* enable sample clk, post-div by 8 */

   /* wait for pll lock */
   BKNI_Sleep(5);
   
   /* check pll status */
   val = BREG_Read32(hDev->hRegister, BCHP_TM_ANA_PLL_STATUS);
   if ((val & BCHP_TM_ANA_PLL_STATUS_LOCK_MASK) == 0)
      BDBG_ERR(("rx pll NOT locked!"));
   
   /* enable aif, sds, fsk, chan */
   val = BREG_Read32(hDev->hRegister, BCHP_TM_REG_CLK_EN);
   val |= 0x0001FFF6;
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
   
   pdiv = BREG_Read32(hDev->hRegister, BCHP_TM_ANA_PLL_PDIV) & 0xF;
   ndiv_int = BREG_Read32(hDev->hRegister, BCHP_TM_ANA_PLL_NDIV_INT) & 0xFF;
   ndiv_frac = BREG_Read32(hDev->hRegister, BCHP_TM_ANA_PLL_NDIV_FRAC) & 0xFFFFF;
   
   /* Fclk_rx_pll_vco = 54 MHz * (2 / Pdiv) * (Ndiv_int + (Ndiv_frac / 2^20)) */
   /* equivalent to (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) / (2^20 * Pdiv) */
   /* Fs_adc = Fclk_rx_pll_vco / 4 */
   
   BMTH_HILO_32TO64_Mul(BWFE_XTAL_FREQ_KHZ << 1, (ndiv_int << 20) + ndiv_frac, &P_hi, &P_lo);  /* (54 MHz * 2) * (2^20 * Ndiv_int + Ndiv_frac) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, pdiv << 20, &Q_hi, &Q_lo);  /* div by (2^20 * Pdiv) */
   
   /* BDBG_MSG(("Fs_adc=%d000", Q_lo)); */
   *freqKhz = Q_lo;
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_SetDpmPilotFreq()
******************************************************************************/
BERR_Code BWFE_P_SetDpmPilotFreq(BWFE_ChannelHandle h, uint32_t freqKhz)
{
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t start_m, end_m, m, n, m_min, n_min, div, div_min;
   uint32_t n_xtal, m_xtal, gcd;
   int32_t x, x_min;
   uint8_t postdiv;
   
   x_min = 0x7FFFFFFF;
   m_min = n_min = div_min = 0;
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
   m = m_min & 0x1FF;
   n = n_min;
   div = div_min & 0xF;
   
   /* determine postdiv */
   switch (n)
   {
      case 1:
         postdiv = 6;
         break;
      case 2:
         postdiv = 4;
         break;
      case 4:
         postdiv = 0;
         break;
      default:
         postdiv = 6;
         break;
   }

   /* program dpm pll but do not enable yet */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, ~0x00000007, postdiv & 0x7);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_DPM_CNTL1_IN, 0x00000000);
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_DPM_PLL_CNTL0_IN, ~0x0001FFF0, 0x00200000 | (div << 13) | (m << 4));
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_DPM_PLL_CNTL1_IN, 0x80A36800);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_DPM_PLL_CNTL2_IN, 0x00000000);  /* frac div */

   /* calculate QDDFS ratio */
   gcd = BWFE_P_GCF(BWFE_DEF_FS_ADC_KHZ, BWFE_XTAL_FREQ_KHZ);
   n_xtal = BWFE_DEF_FS_ADC_KHZ / gcd;
   m_xtal = BWFE_XTAL_FREQ_KHZ / gcd;
   
   hDev->dpmQddfsN = n * div * n_xtal;
   hDev->dpmQddfsM = m * BWFE_LIC_L * m_xtal;
   
   /* remove common divider */
   gcd = BWFE_P_GCF(BWFE_ABS(hDev->dpmQddfsM), BWFE_ABS(hDev->dpmQddfsN));
   if (gcd > 1)
   {
      hDev->dpmQddfsM /= gcd;
      hDev->dpmQddfsN /= gcd;
   }
   hDev->dpmQddfsM = hDev->dpmQddfsM % hDev->dpmQddfsN;

   BDBG_MSG(("BWFE_P_SetDpmPilotFreq(%d KHz): DPM_m=%d, DPM_n=%d, DPM_div=%d, dpmQddfsN=%d, dpmQddfsM=%d", freqKhz, m, n, div, hDev->dpmQddfsN, hDev->dpmQddfsM));
   return retCode;
}


/******************************************************************************
 BWFE_P_GetDpmPilotFreq()
******************************************************************************/
BERR_Code BWFE_P_GetDpmPilotFreq(BWFE_ChannelHandle h, uint32_t *freqKhz)
{
   uint32_t val, postdiv, m, n, div;

   BWFE_P_ReadRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, &val);
   postdiv = val & 0xF;
   
   /* determine n */
   switch (postdiv)
   {
      case 6:
         n = 1;
         break;
      case 4:
         n = 2;
         break;
      case 0:
         n = 4;
         break;
      default:
         n = 1;
         break;
   }
   
   BWFE_P_ReadRegister(h, BCHP_WFE_ANA_DPM_PLL_CNTL0_IN, &val);
   m = (val >> 4) & 0x1FF;    /* retrieve feedback div */
   div = (val >> 13) & 0xF;   /* retrive pre-div */
   
   /* f_dpm = f_xtal/DIV * M/N */
   *freqKhz = BWFE_XTAL_FREQ_KHZ / div * m / n;
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_InitAdc()
******************************************************************************/
BERR_Code BWFE_P_InitAdc(BWFE_ChannelHandle h)
{
   uint32_t val;
   uint8_t mdac2_pwr, mdac1_pwr, sha_pwr, buf_pwr, baud;
   
   /* chip specific current settings */
#if (BCHP_VER>=BCHP_VER_C0)
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL0, 0xA0000402);   /* REFBUF_FS_Control=2, Full_Scale_Control=2 */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, 0x5365B154);   /* Iref_MDAC1_flash3=1, Iref_MDAC2_flash3=1 */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL2, 0x23860ADA);   /* MDAC1_tcntl=2, MDAC23_Tcntl=2 */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, 0x01048000);
#else
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, 0x5365B140);   /* vcom_buffer_pwr=0, flash6_lp_amp_pwr=2, iref_rffe=0 */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL2, 0x33870ADA);   /* optimal settings from analog team */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, 0x01048000);   /* decrease D2C power */
#endif
   
   mdac2_pwr = 0x2;  /* default mdac2 current */
   mdac1_pwr = 0x4;  /* default mdac1 current */

   sha_pwr = 0x5;    /* I(uA) = (3 - x) * 40uA + 200uA */
   buf_pwr = 0x6;    /* I(uA) = (3 - x) * 40uA + 200uA */
   
   val = (mdac2_pwr << 21) | (mdac1_pwr << 18) | (sha_pwr << 15) | (buf_pwr << 12);
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, ~0x00FFF000, val & 0x00FFF000);
   
#if (BCHP_VER==BCHP_VER_A1)
   if (h->channel == 2)
   {
      BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL0, 0x00000402);
      BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, 0x00000017);
      BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL2, 0xD541AACB);
      BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, 0x01040000);
   }
#endif
   
   /* power up adc */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_CLK_CTRL_IN, 0x00000031); /* enable wbadc and mdac clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x0007FFFF); /* default power up state */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00000400); /* release clkgen reset */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00000001); /* power up ADC */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00040000); /* release DGS reset */
   
   /* wait for ADC power up */
   BKNI_Sleep(1);
   
   /* must toggle clkgen reset */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x00000400); 
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x00000400);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_WBADC_TOP_CNTL_IN, 0x394003C0);
   
   /* enable ADC output, use positive edge */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_HDOFFCTL0, 0x00200000);
   
   /* disable dpm */
   BWFE_g2_Rffe_P_DisableDpmMode(h);
   
   /* set agc dec rate */
   if (!h->bReference)
   {
      BWFE_P_ReadRegister(h, BCHP_BAC_MSPI_CONTROL, &val);
      baud = (val >> 8) & 0xFF;
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_AGCDECRATE, 256 * (baud + 1)); /* 256 * mspi baud */
   }
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_ShutdownAdc()
******************************************************************************/
BERR_Code BWFE_P_ShutdownAdc(BWFE_ChannelHandle h)
{
   /* power down ADC and hold in reset */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x00000401);
   
   /* disable wbadc and mdac clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_CLK_CTRL_IN, ~0x00000031);
   
   return BERR_SUCCESS;
}


#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
/******************************************************************************
 BWFE_P_CalcCorrPower()
******************************************************************************/
BERR_Code BWFE_P_CalcCorrPower(BWFE_ChannelHandle h, int32_t *corrI, int32_t *corrQ, uint32_t *corrPowerDb)
{
   BWFE_g2_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g2_P_ChannelHandle *hChn = (BWFE_g2_P_ChannelHandle *)h->pImpl;
   
   uint32_t corrPower;
   int32_t vec[2], phase, tmp_i, tmp_q;
   int32_t cos_1, sin_1, cos_k, sin_k, cos_k1, sin_k1, cos_k2, sin_k2;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   uint32_t R1_hi, R1_lo, R2_hi, R2_lo;
   uint8_t k;
   
   /* theta = Fc / Fadc */
   BMTH_HILO_32TO64_Mul((hChn->corrFreqHz / 500), 2147483648UL, &P_hi, &P_lo);   /* Fc scaled up by 2^32 */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hDev->adcSampleFreqKhz, &Q_hi, &Q_lo);  /* div by Fadc */
   phase = Q_lo;
   BWFE_P_CordicRotationMode(vec, phase, true);
   cos_1 = vec[0];
   sin_1 = vec[1];
   
   /* phase0: corr[0] * exp(0*j*2pi*fc/fs) ... no rotation, just init I & Q total  */
   /* phase1: corr[1] * exp( 1*j*2pi*fc/fs), phase 15: corr[15] * exp(-1*j*2pi*fc/fs) ... negate sin term */
   /* phase2: corr[2] * exp(2*j*2pi*fc/fs), phase 14: corr[15] * exp(-2*j*2pi*fc/fs) ... negate sin term */
   /* phase3: corr[3] * exp(3*j*2pi*fc/fs), phase 13: corr[15] * exp(-3*j*2pi*fc/fs) ... negate sin term */
   /* phase4: corr[4] * exp(4*j*2pi*fc/fs), phase 12: corr[15] * exp(-4*j*2pi*fc/fs) ... negate sin term */
   /* phase5: corr[5] * exp(5*j*2pi*fc/fs), phase 11: corr[15] * exp(-5*j*2pi*fc/fs) ... negate sin term */
   /* phase6: corr[6] * exp(6*j*2pi*fc/fs), phase 10: corr[15] * exp(-6*j*2pi*fc/fs) ... negate sin term */
   /* phase7: corr[7] * exp(7*j*2pi*fc/fs), phase 9: corr[15] * exp(-7*j*2pi*fc/fs) ... negate sin term */
   /* phase8: corr[8] * exp(8*j*2pi*fc/fs) */
   
   /* k = 0, 1, -1 */
   tmp_i = corrI[7];
   /* tmp_i += ((corrI[6] + corrI[8]) * cos_k + (corrQ[6] - corrQ[8]) * sin_k) >> BWFE_CORDIC_SHIFT */
   BWFE_P_SumThenMultiply(corrI[6], corrI[8], cos_1, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(corrQ[6], -corrQ[8], sin_1, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_i += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   tmp_q = corrQ[7];
   /* tmp_q = ((corrQ[8] + corrQ[6]) * cos_k + (corrI[8] - corrI[6]) * sin_k) >> BWFE_CORDIC_SHIFT */
   BWFE_P_SumThenMultiply(corrQ[8], corrQ[6], cos_1, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(corrI[8], -corrI[6], sin_1, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_q += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   /* initial values for chebyshev */
   cos_k2 = 1 * BWFE_CORDIC_SCALE;  /* cos(k-2)x */
   sin_k2 = 0;                      /* sin(k-2)x */
   cos_k1 = cos_1;                  /* cos(k-1)x */
   sin_k1 = sin_1;                  /* sin(k-1)x */
   
   //printf("Fc=%d / Fs=%d000\n", h->corrFreqHz, h->pDevice->adcSampleFreqKhz);
   //printf("cos_1=%08X | sin_1=%08X\n", cos_1, sin_1);
   
   /* use chebyshev method to calculate k = 2, 3, 4, 5, 6, 7, -7, -6, -5, -4, -3, -2 */
   for (k = 2; k < BWFE_LIC_L / 2; k++)
   {
      /* cos(kx) = 2 * cos(x) * cos(k-1)x - cos(k-2)x */
      BWFE_P_SumThenMultiply(0, cos_1, cos_k1, &R1_hi, &R1_lo);
      cos_k = (R1_hi << (32 - BWFE_CORDIC_SHIFT + 1)) | (R1_lo >> (BWFE_CORDIC_SHIFT - 1));  /* undo double cordic scale, multiply by 2 */
      cos_k -= cos_k2;
      
      /* sin(kx) = 2 * cos(x) * sin(k-1)x - sin(k-2)x */
      BWFE_P_SumThenMultiply(0, cos_1, sin_k1, &R2_hi, &R2_lo);
      sin_k = (R2_hi << (32 - BWFE_CORDIC_SHIFT + 1)) | (R2_lo >> (BWFE_CORDIC_SHIFT - 1));  /* undo double cordic scale, multiply by 2 */
      sin_k -= sin_k2;
      
      //printf("cos_%d=%08X | sin_%d=%08X\n", k, cos_k, k, sin_k);
      
      /* rotate by phase */
      /* tmp_i += (corrI[k] + corrI[BWFE_LIC_L - k]) * cos_k + (corrQ[BWFE_LIC_L - k] - corrQ[k]) * sin_k */
      BWFE_P_SumThenMultiply(corrI[7-k], corrI[7+k], cos_k, &R1_hi, &R1_lo);
      BWFE_P_SumThenMultiply(corrQ[7-k], -corrQ[7+k], sin_k, &R2_hi, &R2_lo);
      BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
      tmp_i += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
      
      /* tmp_q += (corrQ[k] + corrQ[BWFE_LIC_L - k]) * cos_k + (corrI[k] - corrI[BWFE_LIC_L - k]) * sin_k */
      BWFE_P_SumThenMultiply(corrQ[7+k], corrQ[7-k], cos_k, &R1_hi, &R1_lo);
      BWFE_P_SumThenMultiply(corrI[7+k], -corrI[7-k], sin_k, &R2_hi, &R2_lo);
      BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
      tmp_q += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
      
      /* setup next iteration */
      cos_k2 = cos_k1;
      sin_k2 = sin_k1;
      cos_k1 = cos_k;
      sin_k1 = sin_k;
   }
   
   /* k = 8 */
   /* cos(8x) = 2 * cos(x) * cos(7x) - cos(6x) */
   BWFE_P_SumThenMultiply(0, cos_1, cos_k1, &R1_hi, &R1_lo);
   cos_k = (R1_hi << (32 - BWFE_CORDIC_SHIFT + 1)) | (R1_lo >> (BWFE_CORDIC_SHIFT - 1));  /* undo double cordic scale, multiply by 2 */
   cos_k -= cos_k2;
   
   /* sin(8x) = 2 * cos(x) * sin(7)x - sin(6)x */
   BWFE_P_SumThenMultiply(0, cos_1, sin_k1, &R2_hi, &R2_lo);
   sin_k = (R2_hi << (32 - BWFE_CORDIC_SHIFT + 1)) | (R2_lo >> (BWFE_CORDIC_SHIFT - 1));  /* undo double cordic scale, multiply by 2 */
   sin_k -= sin_k2;
   
   //printf("cos_8=%08X | sin_8=%08X\n", cos_k, sin_k);
   
   /* tmp_i += corrI[15] * cos_k - corrQ[15] * sin_k */
   BWFE_P_SumThenMultiply(0, corrI[15], cos_k, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(0, -corrQ[15], sin_k, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_i += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   /* tmp_q += corrI[8] * sin_k + corrQ[8] * cos_k */
   BWFE_P_SumThenMultiply(0, corrQ[15], cos_k, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(0, corrI[15], sin_k, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   tmp_q += (int32_t)((P_hi << (32 - BWFE_CORDIC_SHIFT)) | (P_lo >> BWFE_CORDIC_SHIFT));  /* undo cordic scale */
   
   /* corrPower = (tmp_i * tmp_i + tmp_q * tmp_q) */ /*TBD check scaling: scale by correlation period? */
   BWFE_P_SumThenMultiply(0, tmp_i, tmp_i, &R1_hi, &R1_lo);
   BWFE_P_SumThenMultiply(0, tmp_q, tmp_q, &R2_hi, &R2_lo);
   BMTH_HILO_64TO64_Add(R1_hi, R1_lo, R2_hi, R2_lo, &P_hi, &P_lo);
   corrPower = P_hi;
   
   //printf("tmp_i=%08X | tmp_q=%08X -> P=%08X\n", tmp_i, tmp_q, P_hi);
   
   /* BDBG_MSG(("tmp_i=%08X, tmp_q=%08X, corrPower=%08X", tmp_i, tmp_q, corrPower)); */
   *corrPowerDb = BMTH_2560log10(corrPower);
   
   return BERR_SUCCESS;
}

#endif
