/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
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
*****************************************************************************/
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"
#include "bsat_7366_priv.h"
#include "bchp_clkgen.h"
#include "bchp_stb_chan_ctrl.h"
#include "bchp_avs_top_ctrl.h"


#if (BCHP_CHIP != 7366)
#error "This file is for BCM7366 firmware only"
#endif

BDBG_MODULE(bsat_7366_priv);

#define BSAT_DEBUG_POWERDOWN(x) /* x */


/* local functions */
uint32_t BSAT_g1_P_GetSdsClockEnableRegister(BSAT_ChannelHandle h);


const uint32_t BSAT_g1_ChannelIntrID[BSAT_G1_MAX_CHANNELS][BSAT_g1_MaxIntID] =
{
   /* channel 0 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_0,
      BCHP_INT_ID_SDS_NOT_LOCK_0,
      BCHP_INT_ID_SDS_BTM_0,
      BCHP_INT_ID_SDS_BRTM_0,
      BCHP_INT_ID_SDS_GENTM1_0,
      BCHP_INT_ID_SDS_GENTM2_0,
      BCHP_INT_ID_SDS_GENTM3_0,
      BCHP_INT_ID_MI2C_0,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_0,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_0,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_0,
      BCHP_INT_ID_SDS_DFT_DONE_0,
      BCHP_INT_ID_AFEC_LOCK_0,
      BCHP_INT_ID_AFEC_NOT_LOCK_0,
      BCHP_INT_ID_AFEC_MP_LOCK_0,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_0,
      BCHP_INT_ID_TFEC_LOCK_0,
      BCHP_INT_ID_TFEC_NOT_LOCK_0,
      BCHP_INT_ID_TFEC_SYNC_0
   },
   /* channel 1 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_1,
      BCHP_INT_ID_SDS_NOT_LOCK_1,
      BCHP_INT_ID_SDS_BTM_1,
      BCHP_INT_ID_SDS_BRTM_1,
      BCHP_INT_ID_SDS_GENTM1_1,
      BCHP_INT_ID_SDS_GENTM2_1,
      BCHP_INT_ID_SDS_GENTM3_1,
      BCHP_INT_ID_MI2C_1,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_1,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_1,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_1,
      BCHP_INT_ID_SDS_DFT_DONE_1,
      BCHP_INT_ID_AFEC_LOCK_1,
      BCHP_INT_ID_AFEC_NOT_LOCK_1,
      BCHP_INT_ID_AFEC_MP_LOCK_1,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_1,
      BCHP_INT_ID_TFEC_LOCK_1,
      BCHP_INT_ID_TFEC_NOT_LOCK_1,
      BCHP_INT_ID_TFEC_SYNC_1
   },
   /* channel 2 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_2,
      BCHP_INT_ID_SDS_NOT_LOCK_2,
      BCHP_INT_ID_SDS_BTM_2,
      BCHP_INT_ID_SDS_BRTM_2,
      BCHP_INT_ID_SDS_GENTM1_2,
      BCHP_INT_ID_SDS_GENTM2_2,
      BCHP_INT_ID_SDS_GENTM3_2,
      BCHP_INT_ID_MI2C_2,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_2,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_2,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_2,
      BCHP_INT_ID_SDS_DFT_DONE_2,
      BCHP_INT_ID_AFEC_LOCK_2,
      BCHP_INT_ID_AFEC_NOT_LOCK_2,
      BCHP_INT_ID_AFEC_MP_LOCK_2,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_2,
      BCHP_INT_ID_TFEC_LOCK_2,
      BCHP_INT_ID_TFEC_NOT_LOCK_2,
      BCHP_INT_ID_TFEC_SYNC_2
   },
   /* channel 3 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_3,
      BCHP_INT_ID_SDS_NOT_LOCK_3,
      BCHP_INT_ID_SDS_BTM_3,
      BCHP_INT_ID_SDS_BRTM_3,
      BCHP_INT_ID_SDS_GENTM1_3,
      BCHP_INT_ID_SDS_GENTM2_3,
      BCHP_INT_ID_SDS_GENTM3_3,
      BCHP_INT_ID_MI2C_3,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_3,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_3,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_3,
      BCHP_INT_ID_SDS_DFT_DONE_3,
      BCHP_INT_ID_AFEC_LOCK_3,
      BCHP_INT_ID_AFEC_NOT_LOCK_3,
      BCHP_INT_ID_AFEC_MP_LOCK_3,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_3,
      BCHP_INT_ID_TFEC_LOCK_3,
      BCHP_INT_ID_TFEC_NOT_LOCK_3,
      BCHP_INT_ID_TFEC_SYNC_3
   },
   /* channel 4 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_4,
      BCHP_INT_ID_SDS_NOT_LOCK_4,
      BCHP_INT_ID_SDS_BTM_4,
      BCHP_INT_ID_SDS_BRTM_4,
      BCHP_INT_ID_SDS_GENTM1_4,
      BCHP_INT_ID_SDS_GENTM2_4,
      BCHP_INT_ID_SDS_GENTM3_4,
      BCHP_INT_ID_MI2C_4,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_4,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_4,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_4,
      BCHP_INT_ID_SDS_DFT_DONE_4,
      BCHP_INT_ID_AFEC_LOCK_4,
      BCHP_INT_ID_AFEC_NOT_LOCK_4,
      BCHP_INT_ID_AFEC_MP_LOCK_4,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_4,
      BCHP_INT_ID_TFEC_LOCK_4,
      BCHP_INT_ID_TFEC_NOT_LOCK_4,
      BCHP_INT_ID_TFEC_SYNC_4
   },
   /* channel 5 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_5,
      BCHP_INT_ID_SDS_NOT_LOCK_5,
      BCHP_INT_ID_SDS_BTM_5,
      BCHP_INT_ID_SDS_BRTM_5,
      BCHP_INT_ID_SDS_GENTM1_5,
      BCHP_INT_ID_SDS_GENTM2_5,
      BCHP_INT_ID_SDS_GENTM3_5,
      BCHP_INT_ID_MI2C_5,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_5,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_5,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_5,
      BCHP_INT_ID_SDS_DFT_DONE_5,
      BCHP_INT_ID_AFEC_LOCK_5,
      BCHP_INT_ID_AFEC_NOT_LOCK_5,
      BCHP_INT_ID_AFEC_MP_LOCK_5,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_5,
      BCHP_INT_ID_TFEC_LOCK_5,
      BCHP_INT_ID_TFEC_NOT_LOCK_5,
      BCHP_INT_ID_TFEC_SYNC_5
   },
   /* channel 6 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_6,
      BCHP_INT_ID_SDS_NOT_LOCK_6,
      BCHP_INT_ID_SDS_BTM_6,
      BCHP_INT_ID_SDS_BRTM_6,
      BCHP_INT_ID_SDS_GENTM1_6,
      BCHP_INT_ID_SDS_GENTM2_6,
      BCHP_INT_ID_SDS_GENTM3_6,
      BCHP_INT_ID_MI2C_6,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_6,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_6,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_6,
      BCHP_INT_ID_SDS_DFT_DONE_6,
      BCHP_INT_ID_AFEC_LOCK_6,
      BCHP_INT_ID_AFEC_NOT_LOCK_6,
      BCHP_INT_ID_AFEC_MP_LOCK_6,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_6,
      BCHP_INT_ID_TFEC_LOCK_6,
      BCHP_INT_ID_TFEC_NOT_LOCK_6,
      BCHP_INT_ID_TFEC_SYNC_6
   },
   /* channel 7 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_7,
      BCHP_INT_ID_SDS_NOT_LOCK_7,
      BCHP_INT_ID_SDS_BTM_7,
      BCHP_INT_ID_SDS_BRTM_7,
      BCHP_INT_ID_SDS_GENTM1_7,
      BCHP_INT_ID_SDS_GENTM2_7,
      BCHP_INT_ID_SDS_GENTM3_7,
      BCHP_INT_ID_MI2C_7,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_7,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_7,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_7,
      BCHP_INT_ID_SDS_DFT_DONE_7,
      BCHP_INT_ID_AFEC_LOCK_7,
      BCHP_INT_ID_AFEC_NOT_LOCK_7,
      BCHP_INT_ID_AFEC_MP_LOCK_7,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_7,
      BCHP_INT_ID_TFEC_LOCK_7,
      BCHP_INT_ID_TFEC_NOT_LOCK_7,
      BCHP_INT_ID_TFEC_SYNC_7
   }
};


/******************************************************************************
 BSAT_g1_P_GetTotalChannels()
******************************************************************************/
BERR_Code BSAT_g1_P_GetTotalChannels(BSAT_Handle h, uint32_t *totalChannels)
{
   BSTD_UNUSED(h);

   /* just do this for now */
   *totalChannels = BSAT_G1_MAX_CHANNELS;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetRegisterAddress_isrsafe()
******************************************************************************/
void BSAT_g1_P_GetRegisterAddress_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;

   if (h->channel > 0)
   {
      if (((reg >= BCHP_SDS_CG_0_RSTCTL) && (reg <= BCHP_SDS_GR_BRIDGE_0_SW_INIT_1)) ||
          ((reg >= BCHP_TFEC_0_TFECTL) && (reg <= BCHP_TFEC_GR_BRIDGE_0_SW_INIT_1)))
      {
         /* SDS and TFEC register access */
         *pAddr += ((uint32_t)(h->channel) * 0x1000);
      }
      else if ((reg >= BCHP_STB_CHAN_CHx_DEC_FCW) && (reg <= BCHP_STB_CHAN_CHx_SW_SPARE1))
      {
         /* STB_CHAN_CHx register access */
         *pAddr += ((uint32_t)(h->channel) * 0x100);
      }
      else if ((reg >= BCHP_AFEC_RST) && (reg <= BCHP_AFEC_FAKEFRM_PARAM))
      {
         /* AFEC register access */
         if (h->channel & 1)
            *pAddr += 0x1000;
         *pAddr += ((uint32_t)(h->channel >> 1) * 0x8000);
      }
      else if ((reg >= BCHP_AFEC_INTR_CPU_STATUS) && (reg <= BCHP_AFEC_INTR_PCI_MASK_CLEAR))
      {
         /* AFEC_INTR register access */
         if (h->channel & 1)
            *pAddr += 0x400;
         *pAddr += ((uint32_t)(h->channel >> 1) * 0x8000);
      }
      else if (((reg >= BCHP_AFECNX_GLOBAL_CLK_CNTRL) && (reg <= BCHP_AFECNX_COMB_SMTH_FIFO_MAX)) ||
               ((reg >= BCHP_AFEC_GLOBAL_INTR_CPU_STATUS) && (reg <= BCHP_AFEC_GR_BRIDGE_SW_RESET_1)))
      {
         /* AFECNX_GLOBAL, AFECNX, AFEC_GLOBAL_INTR, AFEC_BR_BRIDGE register access */
         *pAddr += ((uint32_t)(h->channel >> 1) * 0x8000);
      }
   }
}


/******************************************************************************
 BSAT_g1_P_ReadRegister_isrsafe() - returns register value
******************************************************************************/
uint32_t BSAT_g1_P_ReadRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg)
{
   uint32_t addr;
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);

   BSAT_g1_P_GetRegisterAddress_isrsafe(h, reg, &addr);
/*BDBG_ERR(("read 0x%X", addr));*/
   return BREG_Read32(pDev->hRegister, addr);
}


/******************************************************************************
 BSAT_g1_P_WriteRegister_isrsafe()
******************************************************************************/
void BSAT_g1_P_WriteRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t val)
{
   uint32_t addr, wait_time;
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);

   BSAT_g1_P_GetRegisterWriteWaitTime_isrsafe(h, reg, &wait_time);
   BSAT_g1_P_GetRegisterAddress_isrsafe(h, reg, &addr);
   BREG_Write32(pDev->hRegister, addr, val);
/*BDBG_ERR(("write 0x%X=0x%X", addr, val));*/
   if (wait_time > 0)
      BKNI_Delay(wait_time);
}


/******************************************************************************
 BSAT_g1_P_InitHandleExt() - chip-specific initial settings in BSAT device handle
******************************************************************************/
BERR_Code BSAT_g1_P_InitHandleExt(BSAT_Handle h)
{
   BSAT_g1_P_Handle *hDev;

   hDev = (BSAT_g1_P_Handle *)(h->pImpl);
   hDev->xtalFreq = BSAT_G1_XTAL_FREQ;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_InitChannelHandleExt() - chip-specific initial settings in BSAT
                                    channel device handle
******************************************************************************/
BERR_Code BSAT_g1_P_InitChannelHandleExt(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->bHasAfec = true;
   hChn->bHasTfec = true;
   hChn->xportSettings.bOpllBypass = true;
   hChn->xportSettings.bSerial = false;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TunerPowerUp() - power up the channelizer
******************************************************************************/
BERR_Code BSAT_g1_P_TunerPowerUp(BSAT_ChannelHandle h)
{
   uint32_t mask = (1 << h->channel);

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, ~mask);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TunerPowerDown() - power down the channelizer
******************************************************************************/
BERR_Code BSAT_g1_P_TunerPowerDown(BSAT_ChannelHandle h)
{
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, (1 << h->channel) | 0x100); /* power down test channel also */
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetSdsClockEnableRegister()
******************************************************************************/
uint32_t BSAT_g1_P_GetSdsClockEnableRegister(BSAT_ChannelHandle h)
{
   uint32_t addr;

#if BCHP_VER>=BCHP_VER_B0
   switch (h->channel)
   {
      case 0:
         addr = BCHP_CLKGEN_DUALSDS_0_INST_EN_CH0;
         break;
      case 1:
         addr = BCHP_CLKGEN_DUALSDS_0_INST_EN_CH1;
         break;
      case 2:
         addr = BCHP_CLKGEN_DUALSDS_1_INST_EN_CH0;
         break;
      case 3:
         addr = BCHP_CLKGEN_DUALSDS_1_INST_EN_CH1;
         break;
      case 4:
         addr = BCHP_CLKGEN_DUALSDS_2_INST_EN_CH0;
         break;
      case 5:
         addr = BCHP_CLKGEN_DUALSDS_2_INST_EN_CH1;
         break;
      case 6:
         addr = BCHP_CLKGEN_DUALSDS_3_INST_EN_CH0;
         break;
      default: /* 7 */
         addr = BCHP_CLKGEN_DUALSDS_3_INST_EN_CH1;
         break;
   }
   /* TBD... */
#else
   if (h->channel < 2)
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_0_INST_PLL_CLOCK;
   else if (h->channel < 4)
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_1_INST_PLL_CLOCK;
   else if (h->channel < 6)
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_2_INST_PLL_CLOCK;
   else
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_3_INST_PLL_CLOCK;
#endif
   return addr;
}


/******************************************************************************
 BSAT_g1_P_GetTfecClockEnableRegister_isrsafe()
******************************************************************************/
uint32_t BSAT_g1_P_GetTfecClockEnableRegister_isrsafe(BSAT_ChannelHandle h)
{
   uint32_t addr;

#if BCHP_VER>=BCHP_VER_B0
   if (h->channel < 2)
      addr = BCHP_CLKGEN_DUALSDS_0_INST_PLL_CLOCK_EN;
   else if (h->channel < 4)
      addr = BCHP_CLKGEN_DUALSDS_1_INST_PLL_CLOCK_EN;
   else if (h->channel < 6)
      addr = BCHP_CLKGEN_DUALSDS_2_INST_PLL_CLOCK_EN;
   else
      addr = BCHP_CLKGEN_DUALSDS_3_INST_PLL_CLOCK_EN;
#else
   if (h->channel < 2)
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_0_INST_PLL_CLOCK;
   else if (h->channel < 4)
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_1_INST_PLL_CLOCK;
   else if (h->channel < 6)
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_2_INST_PLL_CLOCK;
   else
      addr = BCHP_CLKGEN_ONOFF_DUALSDS_WRAPPER_3_INST_PLL_CLOCK;
#endif
   return addr;
}


/******************************************************************************
 BSAT_g1_P_GetAfecClockEnableRegister_isrsafe()
******************************************************************************/
uint32_t BSAT_g1_P_GetAfecClockEnableRegister_isrsafe(BSAT_ChannelHandle h)
{
   uint32_t addr;

#if BCHP_VER>=BCHP_VER_B0
   if (h->channel < 2)
      addr = BCHP_CLKGEN_SDS_AFEC2X_TOP_0_INST_CLOCK_ENABLE;
   else if (h->channel < 4)
      addr = BCHP_CLKGEN_SDS_AFEC2X_TOP_1_INST_CLOCK_ENABLE;
   else if (h->channel < 6)
      addr = BCHP_CLKGEN_SDS_AFEC2X_TOP_2_INST_CLOCK_ENABLE;
   else
      addr = BCHP_CLKGEN_SDS_AFEC2X_TOP_3_INST_CLOCK_ENABLE;
#else
   if (h->channel < 2)
      addr = BCHP_CLKGEN_ONOFF_SDS_AFEC2X_TOP_WRAPPER_0_INST_CLOCK_ENABLE;
   else if (h->channel < 4)
      addr = BCHP_CLKGEN_ONOFF_SDS_AFEC2X_TOP_WRAPPER_1_INST_CLOCK_ENABLE;
   else if (h->channel < 6)
      addr = BCHP_CLKGEN_ONOFF_SDS_AFEC2X_TOP_WRAPPER_2_INST_CLOCK_ENABLE;
   else
      addr = BCHP_CLKGEN_ONOFF_SDS_AFEC2X_TOP_WRAPPER_3_INST_CLOCK_ENABLE;
#endif
   return addr;
}


/******************************************************************************
 BSAT_g1_P_GetAfecSramPowerRegister_isrsafe()
******************************************************************************/
uint32_t BSAT_g1_P_GetAfecSramPowerRegister_isrsafe(BSAT_ChannelHandle h)
{
   uint32_t addr = 0;

#if BCHP_VER>=BCHP_VER_B0
   if (h->channel < 2)
      addr = BCHP_AVS_TOP_CTRL_SRAM_POWER_GATE_IN_AFEC0;
   else if (h->channel < 4)
      addr = BCHP_AVS_TOP_CTRL_SRAM_POWER_GATE_IN_AFEC1;
   else if (h->channel < 6)
      addr = BCHP_AVS_TOP_CTRL_SRAM_POWER_GATE_IN_AFEC2;
   else
      addr = BCHP_AVS_TOP_CTRL_SRAM_POWER_GATE_IN_AFEC3;
#else
   addr = 0;
#endif
   return addr;
}


/******************************************************************************
 BSAT_g1_P_IsSdsOn()
******************************************************************************/
bool BSAT_g1_P_IsSdsOn(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val, addr;
#if BCHP_VER>=BCHP_VER_B0
   uint32_t addr_inst_clock_enable_ch_status, addr_inst_clock_disable_status;
#else
   uint32_t mask;
#endif

   addr = BSAT_g1_P_GetSdsClockEnableRegister(h);
#if BCHP_VER>=BCHP_VER_B0
   addr_inst_clock_enable_ch_status = addr - (h->channel & 1 ? 8 : 12);
   addr_inst_clock_disable_status = addr - (h->channel & 1 ? 24 : 20);
   val = BREG_Read32(pDev->hRegister, addr);
   if ((val & 1) == 0)
      return false;
   val = BREG_Read32(pDev->hRegister, addr_inst_clock_disable_status);
   if (val & ((h->channel & 1) ? 2 : 1))
      return false;
   val = BREG_Read32(pDev->hRegister, addr_inst_clock_enable_ch_status);
   if ((val & 0x3) != 0x3)
      return false;
   return true;
#else
   val =  BREG_Read32(pDev->hRegister, addr);
   mask = (h->channel & 0x1) ? 0x2 : 0x1;
   if ((h->channel == 2) || (h->channel == 3))
      mask = mask << 2;
   return (val & mask) ? true : false;
#endif
}


/******************************************************************************
 BSAT_g1_P_SdsPowerUp() - power up the sds core
******************************************************************************/
BERR_Code BSAT_g1_P_SdsPowerUp(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t addr;
#if BCHP_VER>=BCHP_VER_B0
   uint32_t inst_clock_disable, addr_inst_clock_enable_ch, addr_inst_clock_disable;
#else
   uint32_t val, mask;
#endif

   addr = BSAT_g1_P_GetSdsClockEnableRegister(h);
#if BCHP_VER>=BCHP_VER_B0
   addr_inst_clock_enable_ch = addr - (h->channel & 1 ? 12 : 16);
   addr_inst_clock_disable = addr - (h->channel & 1 ? 28 : 24);
   inst_clock_disable = BREG_Read32(pDev->hRegister, addr_inst_clock_disable);
   inst_clock_disable &= ~((h->channel & 1) ? 2 : 1);
   BREG_Write32(pDev->hRegister, addr_inst_clock_disable, inst_clock_disable);
   BREG_Write32(pDev->hRegister, addr_inst_clock_enable_ch, 0x3);
   BREG_Write32(pDev->hRegister, addr, 1);
#else
   val =  BREG_Read32(pDev->hRegister, addr);
   mask = (h->channel & 1) ? 0x2 : 0x1;
   if ((h->channel == 2) || (h->channel == 3))
      mask = mask << 2;
   val |= mask;
   BREG_Write32(pDev->hRegister, addr, val);
#endif
   return BSAT_g1_P_IsSdsOn(h) ? BERR_SUCCESS : BSAT_ERR_POWERUP_FAILED;
}


/******************************************************************************
 BSAT_g1_P_SdsPowerDown() - power down the sds core
******************************************************************************/
BERR_Code BSAT_g1_P_SdsPowerDown(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t addr;
#if BCHP_VER>=BCHP_VER_B0
   uint32_t inst_clock_disable, addr_inst_clock_enable_ch, addr_inst_clock_disable;
#else
   uint32_t val, mask;
#endif

   addr = BSAT_g1_P_GetSdsClockEnableRegister(h);

#if BCHP_VER>=BCHP_VER_B0
   addr_inst_clock_enable_ch = addr - (h->channel & 1 ? 12 : 16);
   addr_inst_clock_disable = addr - (h->channel & 1 ? 28 : 24);
   inst_clock_disable = BREG_Read32(pDev->hRegister, addr_inst_clock_disable);
   inst_clock_disable |= ((h->channel & 1) ? 2 : 1);
   BREG_Write32(pDev->hRegister, addr_inst_clock_disable, inst_clock_disable);
   BREG_Write32(pDev->hRegister, addr_inst_clock_enable_ch, 0);
   BREG_Write32(pDev->hRegister, addr, 0);
#else
   val = BREG_Read32(pDev->hRegister, addr);
   mask = (h->channel & 0x1) ? 0x2 : 0x1;
   if ((h->channel == 2) || (h->channel == 3))
      mask = mask << 2;
   BREG_Write32(pDev->hRegister, addr, val & ~mask);
#endif
   return BSAT_g1_P_IsSdsOn(h) ? BSAT_ERR_POWERUP_FAILED : BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetAgcStatus()
 agc[0] = {bit 0: lna_agc valid}, {bit 1: chan_agc valid}
 agc[1] = lna_agc
 agc[2] = chan_agc
 agc[3] = (not used)

******************************************************************************/
BERR_Code BSAT_g1_P_GetAgcStatus(BSAT_ChannelHandle h, BSAT_AgcStatus *pStatus)
{
   pStatus->flags = 0x02; /* indicate channelizer AGC value is valid */
   pStatus->value[1] = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_LF_INT);
   return BERR_SUCCESS;
}


#ifndef BSAT_EXCLUDE_AFEC
/******************************************************************************
 BSAT_g1_P_IsAfecOn_isrsafe() - true if afec global is on
******************************************************************************/
bool BSAT_g1_P_IsAfecOn_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val, addr;

   addr = BSAT_g1_P_GetAfecClockEnableRegister_isrsafe(h);
   val = BREG_Read32(pDev->hRegister, addr);
   if ((val & 0x7) != 0x7)
      return false;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CLK_CNTRL);
   if (val & BCHP_AFECNX_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK)
      return false;

   /* afec global is enabled */
   return true;
}


/******************************************************************************
 BSAT_g1_P_AfecPowerUp_isr() - power up the afec core
******************************************************************************/
BERR_Code BSAT_g1_P_AfecPowerUp_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t addr;

   if (BSAT_g1_P_IsAfecOn_isrsafe(h) == false)
   {
      BSAT_DEBUG_POWERDOWN(BDBG_MSG(("AFEC%d Power Up", h->channel)));

#if BCHP_VER>=BCHP_VER_B0
      /* turn on AFEC SRAM */
      addr = BSAT_g1_P_GetAfecSramPowerRegister_isrsafe(h);
      BREG_Write32(pDev->hRegister, addr, 0);
#endif

      /* turn on afec in clkgen */
      /* AFEC is currently powered down */
      /* turn on the 54/108/pll clocks in clkgen */
      addr = BSAT_g1_P_GetAfecClockEnableRegister_isrsafe(h);
      BREG_Write32(pDev->hRegister, addr, 0x7);

      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CLK_CNTRL, ~BCHP_AFECNX_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 1); /* full datapath reset */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 0);
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CONFIG, BCHP_AFECNX_0_GLOBAL_CONFIG_LDPC_ENA_MASK); /* enable LDPC decoder */
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_RESET, BCHP_AFECNX_0_GLOBAL_RESET_AFECNX_DP_RST_MASK);
#ifdef BCHP_AFECNX_CHANNEL_ENABLE
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_CHANNEL_ENABLE, 0);
#else
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_RST, 0);
#endif
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFEC_GLOBAL_INTR_CPU_CLEAR, 0xFFFFFFFF);
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_RESET, ~BCHP_AFECNX_0_GLOBAL_RESET_AFECNX_DP_RST_MASK);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecPowerDown_isrsafe() - power down the afec global core
******************************************************************************/
BERR_Code BSAT_g1_P_AfecPowerDown_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BSAT_ChannelHandle hOtherChan = BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(h);
   BSAT_g1_P_ChannelHandle *hOtherChanImpl;
   uint32_t addr;


   BSAT_DEBUG_POWERDOWN(BDBG_MSG(("AFEC%d Power Down", h->channel)));

   hChn->bAfecFlushFailed = false;

   if (hOtherChan)
   {
      hOtherChanImpl = (BSAT_g1_P_ChannelHandle *)(hOtherChan->pImpl);
      hOtherChanImpl->bAfecFlushFailed = false;
   }

   if (BSAT_g1_P_IsAfecOn_isrsafe(h) == false)
   {
      /* BDBG_MSG(("BSAT_g1_P_AfecPowerDown_isrsafe(%d): AFEC already powered down", h->channel)); */
      goto done;
   }

   /* first do a datapath reset */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 0);

   /* global power down */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CLK_CNTRL, BCHP_AFECNX_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK);

   /* turn off afec in clkgen */
   /* turn off the 54/108/pll clocks in clkgen */
   addr = BSAT_g1_P_GetAfecClockEnableRegister_isrsafe(h);
   BREG_Write32(pDev->hRegister, addr, 0x0);

#if BCHP_VER>=BCHP_VER_B0
   /* turn off AFEC SRAM */
   addr = BSAT_g1_P_GetAfecSramPowerRegister_isrsafe(h);
   BREG_Write32(pDev->hRegister, addr, 1);
#endif

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetAfecClock_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetAfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq)
{
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);
#if BCHP_VER>=BCHP_VER_B0
   uint32_t val = BREG_Read32(hDevImpl->hRegister, BCHP_CLKGEN_PLL_SDS_PLL_DIV);
   uint32_t pdiv = (val >> 10) & 0x0F;
   uint32_t vco = (hDevImpl->xtalFreq / pdiv) * (val & 0x3FF); /* default is 3.888GHz */
   uint32_t div = ((BREG_Read32(hDevImpl->hRegister, BCHP_CLKGEN_PLL_SDS_PLL_CHANNEL_CTRL_CH_0)) >> 1) & 0xFF;
   *pFreq = vco / div; /* should be 432MHz */
#else
   uint32_t val = BREG_Read32(hDevImpl->hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_DIV);
   uint32_t pdiv = (val >> 10) & 0x0F;
   uint32_t vco = (hDevImpl->xtalFreq / pdiv) * (val & 0x3FF); /* default is 3.888GHz */
   uint32_t div = ((BREG_Read32(hDevImpl->hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_5)) >> 1) & 0xFF;
   *pFreq = vco / div; /* should be 432MHz */
#endif
   return BERR_SUCCESS;
}
#endif /* BSAT_EXCLUDE_AFEC */


#ifndef BSAT_EXCLUDE_TFEC
/******************************************************************************
 BSAT_g1_P_TfecPowerUp_isr() - power up the tfec core
******************************************************************************/
BERR_Code BSAT_g1_P_TfecPowerUp_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val, addr, mask;

#if BCHP_VER>=BCHP_VER_B0
   val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3);
   if (val & BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3_CLOCK_DIS_CH3_MASK)
   {
      val &= ~BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3_CLOCK_DIS_CH3_MASK;
      val &= ~BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_HOLD_CH3_MASK;
      BREG_Write32(pDev->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3, val);
   }
#endif

   addr = BSAT_g1_P_GetTfecClockEnableRegister_isrsafe(h);
   val =  BREG_Read32(pDev->hRegister, addr);
#if BCHP_VER>=BCHP_VER_B0
   mask = (h->channel & 1) ? 0x2 : 0x1;
#else
   mask = (h->channel & 1) ? 0x8 : 0x4;
   if ((h->channel == 2) || (h->channel == 3))
      mask = mask >> 2;
#endif
   val |= mask;
   BREG_Write32(pDev->hRegister, addr, val);

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_TFEC_MISC_POST_DIV_CTL, ~(BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_en_MASK | BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_hold_MASK));
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_MISC_REGF_STBY, 0);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_TFEC_MISC_MISCCTL, BCHP_TFEC_MISC_0_MISCCTL_use_fifo_MASK);

   return BSAT_g1_P_IsTfecOn_isrsafe(h) ? BERR_SUCCESS: BSAT_ERR_POWERDOWN_FAILED;
}


/******************************************************************************
 BSAT_g1_P_TfecPowerDown_isrsafe() - power down the tfec core
******************************************************************************/
BERR_Code BSAT_g1_P_TfecPowerDown_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val, addr, mask;

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_TFEC_MISC_MISCCTL, ~BCHP_TFEC_MISC_0_MISCCTL_use_fifo_MASK);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_MISC_REGF_STBY, 1);
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_TFEC_MISC_POST_DIV_CTL, BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_en_MASK | BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_hold_MASK);

   addr = BSAT_g1_P_GetTfecClockEnableRegister_isrsafe(h);
   val =  BREG_Read32(pDev->hRegister, addr);
#if BCHP_VER>=BCHP_VER_B0
   mask = (h->channel & 1) ? 0x2 : 0x1;
#else
   mask = (h->channel & 1) ? 0x8 : 0x4;
   if ((h->channel == 2) || (h->channel == 3))
      mask = mask >> 2;
#endif
   BREG_Write32(pDev->hRegister, addr, val & ~mask);

#if BCHP_VER>=BCHP_VER_B0
   /* check if all TFECs are powered down */
   val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_DUALSDS_0_INST_PLL_CLOCK_EN);
   if (val == 0)
   {
      val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_DUALSDS_1_INST_PLL_CLOCK_EN);
      if (val == 0)
      {
         val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_DUALSDS_2_INST_PLL_CLOCK_EN);
         if (val == 0)
         {
            val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_DUALSDS_3_INST_PLL_CLOCK_EN);
            if (val == 0)
            {
               /* all TFEC blocks are powered down, so power down main pll */
               val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3);
               val |= BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3_CLOCK_DIS_CH3_MASK;
               val |= BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_HOLD_CH3_MASK;
               BREG_Write32(pDev->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3, val);
            }
         }
      }
   }
#endif

   return BSAT_g1_P_IsTfecOn_isrsafe(h) ? BSAT_ERR_POWERDOWN_FAILED : BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_IsTfecOn_isrsafe()
******************************************************************************/
bool BSAT_g1_P_IsTfecOn_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t addr, val, mask;

#if BCHP_VER>=BCHP_VER_B0
   val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3);
   if (val & BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3_CLOCK_DIS_CH3_MASK)
      return false;
#endif

   addr = BSAT_g1_P_GetTfecClockEnableRegister_isrsafe(h);
   val =  BREG_Read32(pDev->hRegister, addr);
#if BCHP_VER>=BCHP_VER_B0
   mask = (h->channel & 1) ? 0x2 : 0x1;
#else
   mask = (h->channel & 0x1) ? 0x8 : 0x4;
   if ((h->channel == 2) || (h->channel == 3))
      mask = mask >> 2;
#endif
   if ((val & mask) == 0)
      return false;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TFEC_MISC_MISCCTL);
   if ((val & BCHP_TFEC_MISC_0_MISCCTL_use_fifo_MASK) == 0)
      return false;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TFEC_MISC_REGF_STBY);
   if (val & 1)
      return false;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TFEC_MISC_POST_DIV_CTL);
   if (val & (BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_en_MASK | BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_hold_MASK))
      return false;

   return true;
}


/******************************************************************************
 BSAT_g1_P_GetTfecClock_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetTfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq)
{
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);
   uint32_t val = BREG_Read32(hDevImpl->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_DIV);
   uint32_t pdiv = (val >> 10) & 0x0F;
   uint32_t vco = (hDevImpl->xtalFreq / pdiv) * (val & 0x3FF); /* default is 3.6GHz */
#if BCHP_VER>=BCHP_VER_B0
   uint32_t div = ((BREG_Read32(hDevImpl->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_3)) >> 1) & 0xFF;
   *pFreq = vco / div; /* should be 240MHz */
#else
   uint32_t div = ((BREG_Read32(hDevImpl->hRegister, BCHP_CLKGEN_PLL_HVD_PLL_CHANNEL_CTRL_CH_5)) >> 1) & 0xFF;
   *pFreq = vco / div; /* should be 240MHz */
#endif
   return BERR_SUCCESS;
}
#endif /* BSAT_EXCLUDE_TFEC */
