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
 ******************************************************************************/

#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g3_priv.h"
#include "bwfe_45216_priv.h"

BDBG_MODULE(bwfe_45216_priv);


uint32_t BWFE_g3_ChannelIntrID[BWFE_NUM_CHANNELS][BWFE_g3_MaxIntID] = {
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
   }
};


uint32_t BWFE_g3_RefChannelIntrID[BWFE_NUM_REF_CHANNELS][BWFE_g3_MaxIntID] = {
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
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   /* set default sample rate */
   hChn->adcSampleFreqKhz = h->pDevice->settings.xtalFreqKhz * 92;

   return retCode;
}


/******************************************************************************
 BWFE_P_MapToRefRegister()
******************************************************************************/
BERR_Code BWFE_P_MapToRefRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   BERR_Code retCode = BERR_SUCCESS;

   BSTD_UNUSED(h);

#if 0 /* TBD */
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
      case BCHP_WFE_CORE_DGSCTL2:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSCTL2;
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
      case BCHP_WFE_CORE_DGSACCUM_PI_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSACCUM_PI_SLC;
         break;
      case BCHP_WFE_CORE_DGSACCUM_PO_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSACCUM_PO_SLC;
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
      case BCHP_WFE_CORE_DGSBGLMS_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_DGSBGLMS_SLC;
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
      case BCHP_WFE_CORE_MDACSA_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_MDACSA_SLC;
         break;
      case BCHP_WFE_CORE_MDACSADU_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_MDACSADU_SLC;
         break;
      case BCHP_WFE_CORE_MDACSAOUT_SLC:
         *pAddr = BCHP_MDAC_CAL_CORE_MDACSAOUT_SLC;
         break;
      case BCHP_WFE_CORE_MDACSASTS:
         *pAddr = BCHP_MDAC_CAL_CORE_MDACSASTS;
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
      case BCHP_WFE_CORE_REG_DGSEP_DMX2_PI_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX2_PI_SLC_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_DMX2_PO_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX2_PO_SLC_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_DMX3_PI_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX3_PI_SLC_INT_WDATA;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_DMX3_PO_SLC_INT_WDATA:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_DMX3_PO_SLC_INT_WDATA;
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
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX2_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX2_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX2_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX2_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX2_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX2_PO_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX2_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX2_PO_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX3_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX3_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX3_PI_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX3_PI_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX3_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S1_DMX3_PO_SLC_ERRP;
         break;
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX3_PO_SLC_ERRP:
         *pAddr = BCHP_MDAC_CAL_CORE_REG_DGSEP_S2_DMX3_PO_SLC_ERRP;
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
      case BCHP_WFE_ANA_ADC_CNTL4:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL4;
         break;
      case BCHP_WFE_ANA_ADC_CNTL5:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL5;
         break;
      case BCHP_WFE_ANA_ADC_CNTL6:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL6;
         break;
      case BCHP_WFE_ANA_ADC_CNTL7:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL7;
         break;
      case BCHP_WFE_ANA_ADC_CNTL8:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL8;
         break;
      case BCHP_WFE_ANA_ADC_CNTL9:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL9;
         break;
      case BCHP_WFE_ANA_ADC_CNTL10:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL10;
         break;
      case BCHP_WFE_ANA_ADC_CNTL11:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL11;
         break;
      case BCHP_WFE_ANA_ADC_CNTL12:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL12;
         break;
      case BCHP_WFE_ANA_ADC_CNTL13:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL13;
         break;
      case BCHP_WFE_ANA_ADC_CNTL14:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL14;
         break;
      case BCHP_WFE_ANA_ADC_CNTL15:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL15;
         break;
      case BCHP_WFE_ANA_ADC_CNTL16:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL16;
         break;
      case BCHP_WFE_ANA_ADC_CNTL17:
         *pAddr = BCHP_MDAC_CAL_ANA_ADC_CNTL17;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL0:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL0;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL1:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL1;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL2:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL2;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL3:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL3;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL4:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL4;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL5:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL5;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL6:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL6;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL7:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL7;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL8:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL8;
         break;
      case BCHP_WFE_ANA_PILOT_CNTL9:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_CNTL9;
         break;
      case BCHP_WFE_ANA_PLL_CNTL0:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL0;
         break;
      case BCHP_WFE_ANA_PLL_CNTL1:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL1;
         break;
      case BCHP_WFE_ANA_PLL_CNTL2:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL2;
         break;
      case BCHP_WFE_ANA_PLL_CNTL3:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL3;
         break;
      case BCHP_WFE_ANA_PLL_CNTL4:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL4;
         break;
      case BCHP_WFE_ANA_PLL_CNTL5:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL5;
         break;
      case BCHP_WFE_ANA_PLL_CNTL6:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL6;
         break;
      case BCHP_WFE_ANA_PLL_CNTL7:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_CNTL7;
         break;
      case BCHP_WFE_ANA_SYS_CNTL:
         *pAddr = BCHP_MDAC_CAL_ANA_SYS_CNTL;
         break;
      case BCHP_WFE_ANA_PILOT_PLL_STAT:
         *pAddr = BCHP_MDAC_CAL_ANA_PILOT_PLL_STAT;
         break;
      case BCHP_WFE_ANA_PLL_STAT:
         *pAddr = BCHP_MDAC_CAL_ANA_PLL_STAT;
         break;
      case BCHP_WFE_ANA_CLK_CTRL:
         *pAddr = BCHP_MDAC_CAL_ANA_CLK_CTRL;
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
#endif

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
      if ((reg >= 0x7800000) && (reg <= 0x78010c0))
      {
         /* WFE and WBADC register access */
         *pAddr += h->channel * 0x100000;
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
   BWFE_g3_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t addr;

   BWFE_P_GetRegisterAddress(h, reg, &addr);
   *val = BREG_Read32(hDev->hRegister, addr);

   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_WriteRegister_isrsafe()
******************************************************************************/
BERR_Code BWFE_P_WriteRegister_isrsafe(
   BWFE_ChannelHandle h,      /* [in] BWFE channel handle */
   uint32_t           reg,    /* [in] address of register to write */
   uint32_t           val     /* [in] contains data to be written */
)
{
   BWFE_g3_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t addr;

   BWFE_P_GetRegisterAddress(h, reg, &addr);
   BREG_Write32(hDev->hRegister, addr, val);

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
      case BCHP_WFE_CORE_DGSACCUM_PI_SLC:
      case BCHP_WFE_CORE_DGSACCUM_PO_SLC:
      case BCHP_WFE_CORE_DGSCOEFD_SLC:
      case BCHP_WFE_CORE_DGSCOEFA_SLC:
         *pAddr = reg + slice * 0x8;
         break;

      case BCHP_WFE_CORE_DGSLMS_SLC:
      case BCHP_WFE_CORE_DGSBGLMS_SLC:
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
      case BCHP_WFE_CORE_CORRI0_DMX2_PI_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX2_PI_SLC:
      case BCHP_WFE_CORE_CORRI0_DMX2_PO_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX2_PO_SLC:
      case BCHP_WFE_CORE_CORRI0_DMX3_PI_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX3_PI_SLC:
      case BCHP_WFE_CORE_CORRI0_DMX3_PO_SLC:
      case BCHP_WFE_CORE_CORRI1_DMX3_PO_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX2_PI_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX2_PI_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX2_PO_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX2_PO_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX3_PI_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX3_PI_SLC:
      case BCHP_WFE_CORE_CORRQ0_DMX3_PO_SLC:
      case BCHP_WFE_CORE_CORRQ1_DMX3_PO_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX0_PI_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX0_PO_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX1_PI_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX1_PO_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX2_PI_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX2_PI_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX2_PO_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX2_PO_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX3_PI_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX3_PI_SLC:
      case BCHP_WFE_CORE_CORRP0_DMX3_PO_SLC:
      case BCHP_WFE_CORE_CORRP1_DMX3_PO_SLC:
         *pAddr = reg + slice * 0x8;
         break;

      case BCHP_WFE_CORE_DGSEPCTL_SLC:
      case BCHP_WFE_CORE_REG_DGSEP_DMX0_PI_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX0_PO_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX1_PI_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX1_PO_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX2_PI_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX2_PO_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX3_PI_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_DMX3_PO_SLC_INT_WDATA:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX0_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX0_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX0_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX0_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX1_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX1_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX1_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX1_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX2_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX2_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX2_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX2_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX3_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX3_PI_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S1_DMX3_PO_SLC_ERRP:
      case BCHP_WFE_CORE_REG_DGSEP_S2_DMX3_PO_SLC_ERRP:
         *pAddr = reg + slice * 0x64;
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
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_ChannelHandle hChn0 = (BWFE_ChannelHandle)(h->pDevice->pChannels[0]);
   uint32_t ndiv_int, val;
   uint8_t i;
   bool bPllLocked = false;

   /* limit possible frequencies based on 54MHz */
   if (freqKhz % (BWFE_XTAL_FREQ_KHZ / 2) != 0)
      return BERR_INVALID_PARAMETER;

   /* Fs_adc = F_ref / pdiv * ndiv_int *2 */
   /* calculate ndiv_int from freqKhz, ndiv_int = (Fs_adc) / (54MHz * 2) */
   ndiv_int = (freqKhz >> 1) / BWFE_XTAL_FREQ_KHZ;

   /* program ndiv for channel */
   BWFE_P_WriteRegister(hChn0, BCHP_WFE_ANA_PLL_CNTL1, ndiv_int & 0x3FF);

   /* disable pll isolation to clock chan and sds, and to read pll status */
   BWFE_P_AndRegister(hChn0, BCHP_WFE_ANA_CLK_CTRL, ~0x00000040);

   /* check ADC0 pll status */
   BWFE_P_ReadRegister(hChn0, BCHP_WFE_ANA_PLL_STAT, &val);
   if ((val & 0x00002000) == 0)
   {
      /* power up ADC0 pll */
      BWFE_P_OrRegister(hChn0, BCHP_WFE_ANA_SYS_CNTL, 0x00000001); /* power on ldo pll */
      BWFE_P_OrRegister(hChn0, BCHP_WFE_ANA_SYS_CNTL, 0x00000002); /* power on pll */

      BKNI_Delay(30);   /* wait 21us or more */

      BWFE_P_OrRegister(hChn0, BCHP_WFE_ANA_SYS_CNTL, 0x00010000); /* release pll reset */
      BWFE_P_OrRegister(hChn0, BCHP_WFE_ANA_SYS_CNTL, 0x00020000); /* release postdiv reset */

      for (i = 0; i < 20; i++)
      {
         /* wait for pll lock */
         BKNI_Delay(50);

         /* check pll status */
         BWFE_P_ReadRegister(hChn0, BCHP_WFE_ANA_PLL_STAT, &val);
         if (val & 0x00002000)
         {
            bPllLocked = true;
            break;
         }
      }

      if (!bPllLocked)
      {
         /* pll not locked */
         BDBG_WRN(("ADC0 pll NOT locked!"));
      }
   }

   return retCode;
}


/******************************************************************************
 BWFE_P_GetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_P_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_ChannelHandle hChn0 = (BWFE_ChannelHandle)(h->pDevice->pChannels[0]);
   uint32_t val, pdiv, ndiv_int;

   BWFE_P_ReadRegister(hChn0, BCHP_WFE_ANA_PLL_CNTL0, &val);
   pdiv = (val >> 18) & 0xF;

   BWFE_P_ReadRegister(hChn0, BCHP_WFE_ANA_PLL_CNTL1, &val);
   ndiv_int = val & 0x3FF;

   /* Fs_adc = F_ref / pdiv * ndiv_int *2 */
   *freqKhz = BWFE_XTAL_FREQ_KHZ * (ndiv_int << 1) / pdiv;
   return retCode;
}


/******************************************************************************
 BWFE_P_SetDpmPilotFreq()
******************************************************************************/
BERR_Code BWFE_P_SetDpmPilotFreq(BWFE_ChannelHandle h, uint32_t freqKhz)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(freqKhz);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_P_GetDpmPilotFreq()
******************************************************************************/
BERR_Code BWFE_P_GetDpmPilotFreq(BWFE_ChannelHandle h, uint32_t *freqKhz)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(freqKhz);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_P_EnableDpmPilot()
******************************************************************************/
BERR_Code BWFE_P_EnableDpmPilot(BWFE_ChannelHandle h)
{
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_P_DisableDpmPilot()
******************************************************************************/
BERR_Code BWFE_P_DisableDpmPilot(BWFE_ChannelHandle h)
{
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_P_InitAdc()
******************************************************************************/
BERR_Code BWFE_P_InitAdc(BWFE_ChannelHandle h)
{
   BWFE_ChannelHandle hAdjChn;
   BWFE_ChannelHandle hPllChn = (BWFE_ChannelHandle)(h->pDevice->pChannels[0]);
   bool bPllLocked = false;
   uint32_t val;
   uint8_t i;

   /* determine if adjacent adc is on since pll shared */
   if (h->channel & 1)
      hAdjChn = h->pDevice->pChannels[h->channel - 1];
   else
      hAdjChn = h->pDevice->pChannels[h->channel + 1];

#ifdef BWFE_HYBRID_ADC
   /* over-range threshold, same phase for output clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL0, ~0x00060001);

   /* hybrid settings */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, 0x5B452285);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL2, 0x555649A4);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL3, 0x10014000);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL4, 0x06E1B6E0);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL5, 0x22008008);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL6, 0x00000000);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL7, 0x00000000);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL8, 0x22008008);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL9, 0x00000000);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL10, 0x00000000);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL11, 0x49680000);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL12, 0x00002024);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL13, 0x0A952A54);
#else
   /* same phase for output clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_ADC_CNTL0, ~0x00000001);

   /* chip-specific adc settings */
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL1, 0x5B492385);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL2, 0x55764522);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL5, 0x22008008);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL11, 0x00000001);
   BWFE_P_WriteRegister(h, BCHP_WFE_ANA_ADC_CNTL12, 0x000C032B);
#endif

   BWFE_P_OrRegister(h, BCHP_WFE_ANA_CLK_CTRL, 0x00000031); /* enable wbadc and mdac clock */

   if ((hAdjChn->bEnabled) || (h->channel == 0))
      BWFE_P_AndRegister(h, BCHP_WFE_ANA_SYS_CNTL, ~0x03FCFFFC); /* do not touch pll if adj adc on, cannot power down adc0 pll */
   else
      BWFE_P_AndRegister(h, BCHP_WFE_ANA_SYS_CNTL, ~0x03FFFFFF); /* default power up state */

   BWFE_P_OrRegister(hPllChn, BCHP_WFE_ANA_SYS_CNTL, 0x00000001); /* power on ldo pll */
   BWFE_P_OrRegister(hPllChn, BCHP_WFE_ANA_SYS_CNTL, 0x00000002); /* power on pll */

   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x00000004); /* power on bandgap */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x00000008); /* power on adc clock */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x00000010); /* power up ADC */

   BKNI_Delay(30);   /* wait 21us or more */

   BWFE_P_OrRegister(hPllChn, BCHP_WFE_ANA_SYS_CNTL, 0x00010000); /* release pll reset */
   BWFE_P_OrRegister(hPllChn, BCHP_WFE_ANA_SYS_CNTL, 0x00020000); /* release postdiv reset */

   for (i = 0; i < 20; i++)
   {
      /* wait for pll lock */
      BKNI_Delay(50);

      /* check pll status */
      BWFE_P_ReadRegister(hPllChn, BCHP_WFE_ANA_PLL_STAT, &val);
      if (val & 0x00002000)
      {
         bPllLocked = true;
         break;
      }
   }

   if (!bPllLocked)
   {
      /* pll not locked */
      BDBG_WRN(("ADC%d pll NOT locked!", h->channel));
   }

   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x00040000); /* release ADC reset */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x00080000); /* release clkgen reset */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x00100000); /* release DGS reset */

   /* wait 1ms for ADC power up */
   BKNI_Sleep(1);

   /* enable ADC output, use positive edge */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_HDOFFCTL0, 0x00200000);

   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_ShutdownAdc()
******************************************************************************/
BERR_Code BWFE_P_ShutdownAdc(BWFE_ChannelHandle h)
{
   /* disable ADC output */
   if (BWFE_P_IsAdcOn(h))
      BWFE_P_AndRegister(h, BCHP_WFE_CORE_HDOFFCTL0, ~0x00200000);

   /* power down ADC and hold in reset */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_SYS_CNTL, ~0x00080010);

   /* disable wbadc, misc, corr, and mdac clock */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_CLK_CTRL, ~0x00000039);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_P_IsAdcOn()
******************************************************************************/
bool BWFE_P_IsAdcOn(BWFE_ChannelHandle h)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, BCHP_WFE_ANA_SYS_CNTL, &val);
   if (val & 0x10)
      h->bEnabled = true;
   else
      h->bEnabled = false;

   return h->bEnabled;
}


#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
/******************************************************************************
 BWFE_P_CalcCorrPower()
******************************************************************************/
BERR_Code BWFE_P_CalcCorrPower(BWFE_ChannelHandle h, int32_t *corrI, int32_t *corrQ, uint32_t *corrPowerDb)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;

   uint32_t corrPower;
   int32_t vec[2], phase, tmp_i, tmp_q;
   int32_t cos_1, sin_1, cos_k, sin_k, cos_k1, sin_k1, cos_k2, sin_k2;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   uint32_t R1_hi, R1_lo, R2_hi, R2_lo;
   uint8_t k;

   /* theta = Fc / Fadc */
   BMTH_HILO_32TO64_Mul((hChn->corrFreqHz / 500), 2147483648UL, &P_hi, &P_lo);   /* Fc scaled up by 2^32 */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->adcSampleFreqKhz, &Q_hi, &Q_lo);  /* div by Fadc */
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

   /*printf("Fc=%d / Fs=%d000\n", hChn->corrFreqHz, hChn->adcSampleFreqKhz);*/
   /*printf("cos_1=%08X | sin_1=%08X\n", cos_1, sin_1);*/

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

      /*printf("cos_%d=%08X | sin_%d=%08X\n", k, cos_k, k, sin_k);*/

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

   /*printf("cos_8=%08X | sin_8=%08X\n", cos_k, sin_k);*/

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

   /*printf("tmp_i=%08X | tmp_q=%08X -> P=%08X\n", tmp_i, tmp_q, P_hi);*/

   /* BDBG_MSG(("tmp_i=%08X, tmp_q=%08X, corrPower=%08X", tmp_i, tmp_q, corrPower)); */
   *corrPowerDb = BMTH_2560log10(corrPower);

   return BERR_SUCCESS;
}

#endif
