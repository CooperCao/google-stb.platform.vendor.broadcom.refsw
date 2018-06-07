/******************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* Module Description:
*
*****************************************************************************/
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"
#include "bsat_45308_priv.h"
#include "bchp_tm.h"
#include "bchp_clkgen.h"
#include "bchp_stb_chan_ctrl_0.h"
#include "bchp_stb_chan_ctrl_1.h"


#if (BCHP_CHIP != 45308)
#error "This file is for BCM45308 firmware only"
#endif

BDBG_MODULE(bsat_45308_priv);

#define BSAT_DEBUG_POWERDOWN(x) /*x*/
#if 1
#define PRINT BKNI_Printf
#else
#define PRINT printf
#endif


static const uint32_t BSAT_afec_clock_enable_status_reg[16] =
{
   BCHP_CLKGEN_SDS_AFEC_TOP_0_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_1_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_2_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_3_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_4_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_5_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_6_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_7_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_8_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_9_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_10_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_11_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_12_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_13_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_14_INST_CLOCK_ENABLE_STATUS,
   BCHP_CLKGEN_SDS_AFEC_TOP_15_INST_CLOCK_ENABLE_STATUS
};

static const uint32_t BSAT_afec_clock_enable_reg[16] =
{
   BCHP_CLKGEN_SDS_AFEC_TOP_0_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_1_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_2_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_3_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_4_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_5_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_6_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_7_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_8_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_9_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_10_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_11_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_12_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_13_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_14_INST_CLOCK_ENABLE,
   BCHP_CLKGEN_SDS_AFEC_TOP_15_INST_CLOCK_ENABLE
};


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
   uint32_t product_id, family_id, chip, n;
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pImpl);

   product_id = BREG_Read32(pDev->hRegister, BCHP_TM_PRODUCT_ID);
   family_id = BREG_Read32(pDev->hRegister, BCHP_TM_FAMILY_ID) >> 8;
   chip = (product_id >> 8) & 0xFFFFF;
   n = chip & 0xFF;
   if (family_id == 0x45302)
      *totalChannels = 2;
   else if (family_id == 0x45304)
      *totalChannels = 4;
   else if ((n == 0x16) || ((family_id & 0xFF) == 0x16))
      *totalChannels = 16;
   else if (n == 0)
      *totalChannels = 8;
   else
      *totalChannels = n;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ValidateAcqParams()
******************************************************************************/
BERR_Code BSAT_g1_P_ValidateAcqParams(BSAT_ChannelHandle h, BSAT_AcqSettings *pParams)
{
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   if ((pParams->mode == BSAT_Mode_eDvbs2_ACM) && (hDevImpl->sdsRevId < 0x74))
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
   else
      return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetRegisterAddress_isrsafe()
******************************************************************************/
void BSAT_g1_P_GetRegisterAddress_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;
   uint32_t i;

   if (h->channel > 0)
   {
      if (((reg >= BCHP_SDS_CG_0_RSTCTL) && (reg <= BCHP_SDS_GR_BRIDGE_0_SW_INIT_1)) ||
          ((reg >= BCHP_AFEC_0_RST) && (reg <= BCHP_AFEC_GLOBAL_0_SW_SPARE1)) ||
          ((reg >= BCHP_SDS_DSEC_0_DSRST) && (reg <= BCHP_SDS_DSEC_AP_0_SW_SPARE1)))
      {
         /* SDS, AFEC, or DSEC register access */
         *pAddr += ((uint32_t)(h->channel) * 0x100000);
      }
      else if (((reg >= BCHP_TFEC_0_0_TFECTL) && (reg <= BCHP_TFEC_0_0_LFSRSTATUS)) ||
               ((reg >= BCHP_TFEC_INTR2_0_0_CPU_STATUS) && (reg <= BCHP_TFEC_INTR2_0_0_PCI_MASK_CLEAR)))
      {
         /* TFEC or TFEC_INTR2 register access */
         if (h->channel & 1)
            *pAddr += 0x0080;
         *pAddr += ((uint32_t)(h->channel >> 1) * 0x100000);
      }
      else if (((reg >= BCHP_TFEC_MISC_0_POST_DIV_CTL) && (reg <= BCHP_TFEC_MISC_0_MISCCTL)) ||
               ((reg >= BCHP_TFEC_GR_BRIDGE_0_REVISION) && (reg <= BCHP_TFEC_GR_BRIDGE_0_SW_INIT_1)))
      {
         /* TFEC_MISC or TFEC_GR_BRIDGE register access */
         *pAddr += ((uint32_t)(h->channel >> 1) & 0x100000);
      }
      else if ((reg >= BCHP_STB_CHAN_CTRL_0_REVID) && (reg <= BCHP_STB_CHAN_CTRL_0_SW_SPARE1))
      {
         /* STB_CHAN_CTRL register access */
         if (h->channel >= 8)
            *pAddr += 0x100000;
      }
      else if ((reg >= BCHP_STB_CHAN_CH0_0_DEC_FCW) && (reg <= BCHP_STB_CHAN_CH0_0_SW_SPARE1))
      {
         /* STB_CHAN_CHx register access */
         if (h->channel >= 8)
         {
            *pAddr += 0x100000;
            i = h->channel - 8;
         }
         else
            i = h->channel;
         *pAddr += (i * 0x100);
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
   if (wait_time > 0)
      BKNI_Delay(wait_time);
}


/******************************************************************************
 BSAT_g1_P_InitHandleExt() - chip-specific initial settings in BSAT device handle
******************************************************************************/
BERR_Code BSAT_g1_P_InitHandleExt(BSAT_Handle h)
{
   BSAT_g1_P_Handle *hDev;
   uint32_t val;

   hDev = (BSAT_g1_P_Handle *)(h->pImpl);
   hDev->xtalFreq = BSAT_G1_XTAL_FREQ;

   val = BREG_Read32(hDev->hRegister, BCHP_TM_FAMILY_ID);
   if ((val >> 8) == 0x45316)
      hDev->chipFamily = BSAT_CHIP_FAMILY_GYPSUM_A0;
   else if ((val & 0xFF) == 0x20)
      hDev->chipFamily = BSAT_CHIP_FAMILY_C0;
   else
      hDev->chipFamily = BSAT_CHIP_FAMILY_B0;
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
 BSAT_g1_P_IsChannelizerOn()
******************************************************************************/
bool BSAT_g1_P_IsChannelizerOn(BSAT_ChannelHandle h)
{
   uint32_t val;

   if (h->channel < 8)
      val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_STB_CHAN_TOP_0_INST_ENABLE);
   else
      val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_STB_CHAN_TOP_1_INST_ENABLE);
   return (val == 3) ? true : false;
}

/******************************************************************************
 BSAT_g1_P_EnableTfecPll_isr() - enable/disable 486MHz tfec pll clock in clkgen
                                 pll3.ch3
******************************************************************************/
void BSAT_g1_P_EnableTfecPll_isr(BSAT_Handle h, bool bEnable)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pImpl);
   uint32_t val;

   val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_PLL3_SYS_PLL_CHANNEL_CTRL_CH_3);
   if (bEnable)
      val &= ~0x401;
   else
      val |= 0x401;

   BREG_Write32(pDev->hRegister, BCHP_CLKGEN_PLL3_SYS_PLL_CHANNEL_CTRL_CH_3, val);
   BSAT_DEBUG_POWERDOWN(PRINT("turn %s pll3.ch3\n", bEnable ? "on" : "off"));
}


/******************************************************************************
 BSAT_g1_P_EnableAfecPll_isr() - enable/disable 270MHz afec pll clock in clkgen
                                 pll4.ch4
******************************************************************************/
void BSAT_g1_P_EnableAfecPll_isr(BSAT_Handle h, bool bEnable)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pImpl);
   uint32_t val;

   val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4);
   if (bEnable)
      val &= ~0x401;
   else
      val |= 0x401;

   BREG_Write32(pDev->hRegister, BCHP_CLKGEN_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4, val);
   BSAT_DEBUG_POWERDOWN(PRINT("turn %s pll4.ch4\n", bEnable ? "on" : "off"));
}


/******************************************************************************
 BSAT_g1_P_IsTfecPllOn_isrsafe()
******************************************************************************/
bool BSAT_g1_P_IsTfecPllOn_isrsafe(BSAT_Handle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pImpl);
   uint32_t val;

   val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_PLL3_SYS_PLL_CHANNEL_CTRL_CH_3);
   if (val & 0x401)
      return false;
   else
      return true;
}


/******************************************************************************
 BSAT_g1_P_IsAfecPllOn_isrsafe()
******************************************************************************/
bool BSAT_g1_P_IsAfecPllOn_isrsafe(BSAT_Handle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pImpl);
   uint32_t val;

   val = BREG_Read32(pDev->hRegister, BCHP_CLKGEN_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4);
   if (val & 0x401)
      return false;
   else
      return true;
}


/******************************************************************************
 BSAT_g1_P_IsOkToDisableAfecPll_isrsafe()
******************************************************************************/
bool BSAT_g1_P_IsOkToDisableAfecPll_isrsafe(BSAT_Handle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pImpl);
   uint32_t reg, i, val;

   for (i = 0; i < 8; i++)
   {
      reg = BSAT_afec_clock_enable_status_reg[i];
      val = BREG_Read32(pDev->hRegister, reg);
      if (val & 1)
         return false;
   }
   return true;
}


/******************************************************************************
 BSAT_g1_P_IsOkToDisableTfecPll_isrsafe()
******************************************************************************/
bool BSAT_g1_P_IsOkToDisableTfecPll_isrsafe(BSAT_Handle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pImpl);
   uint32_t reg, i, val, mask;

   for (i = 0; i < 8; i++)
   {
      reg = BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE;
      reg += ((uint32_t)(i * 0x14));
      val = BREG_Read32(pDev->hRegister, reg);
      if (i < 5)
         mask = 0x88;
      else
         mask = 0x84;
      if (val & mask)
         return false;
   }
   return true;
}


/******************************************************************************
 BSAT_g1_P_TunerPowerUp() - power up the channelizer
******************************************************************************/
BERR_Code BSAT_g1_P_TunerPowerUp(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t mask, shift;

   BSAT_DEBUG_POWERDOWN(PRINT("Tuner%d power up\n", h->channel));

   if (BSAT_g1_P_IsChannelizerOn(h) == false)
   {
      /* turn on STB_CHAN_TOP RBUS clock */
      if (h->channel < 8)
      {
         BREG_Write32(pDev->hRegister, BCHP_CLKGEN_STB_CHAN_TOP_0_INST_ENABLE, 3);
         BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_0_XBAR_CTRL, ~0x03); /* toggle adc fifo */
         BSAT_g1_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_0_XBAR_CTRL, 0x03);   /* turn on adc fifo */
      }
      else
      {
         BREG_Write32(pDev->hRegister, BCHP_CLKGEN_STB_CHAN_TOP_1_INST_ENABLE, 3);
         BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_1_XBAR_CTRL, ~0x03); /* toggle adc fifo */
         BSAT_g1_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_1_XBAR_CTRL, 0x03);   /* turn on adc fifo */
      }
   }

   shift = (h->channel < 8) ? h->channel : (h->channel - 8);
   mask = (1 << shift);

   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, ~mask);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET, ~mask);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TunerPowerDown() - power down the channelizer
******************************************************************************/
BERR_Code BSAT_g1_P_TunerPowerDown(BSAT_ChannelHandle h)
{
   uint32_t mask, shift, val, totalChannels;

   BSAT_DEBUG_POWERDOWN(PRINT("Tuner%d power down\n", h->channel));

   shift = (h->channel < 8) ? h->channel : (h->channel - 8);
   mask = (1 << shift);

   BKNI_EnterCriticalSection();
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, mask);
   BKNI_LeaveCriticalSection();

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN);
   BSAT_g1_P_GetTotalChannels(h->pDevice, &totalChannels);
   if (h->channel < 8)
   {
      if (totalChannels <= 8)
         mask = (1 << totalChannels) - 1;
      else
         mask = 0xFF;
   }
   else
   {
      mask = (1 << (totalChannels - 8)) - 1;
   }

   BSAT_DEBUG_POWERDOWN(PRINT("BSAT_g1_P_TunerPowerDown(%d): totalChannels=%u, stb_chan_ctrl_pwrdn=0x%X, mask=0x%X\n", h->channel, totalChannels, val, mask));
   if ((val & mask) == mask)
   {
      /* stop STB_CHAN_TOP RBUS clock */
      if (h->channel < 8)
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_CLKGEN_STB_CHAN_TOP_0_INST_ENABLE, 0);
      else
         BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_CLKGEN_STB_CHAN_TOP_1_INST_ENABLE, 0);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_IsSdsOn()
******************************************************************************/
bool BSAT_g1_P_IsSdsOn(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t ctrl3, ctrl1, mask, reg, val;

   if (h->channel < 10)
   {
      if (h->channel & 1)
         mask = 0x53; /* check if rcvr1 is on */
      else
         mask = 0x27; /* check if rcvr0 is on */
   }
   else
   {
      if (h->channel & 1)
         mask = 0x59; /* check if rcvr1 is on */
      else
         mask = 0x33; /* check if rcvr0 is on */
   }

   reg = BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE;
   reg += ((uint32_t)(h->channel >> 1) * 0x14);
   val = BREG_Read32(pDev->hRegister, reg);
   if ((val & mask) != mask)
      return false;

   if (!BSAT_g1_P_IsChannelizerOn(h))
      return false;

   ctrl1 = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL1);
   if (ctrl1 & BCHP_TM_CLOCK_ENABLE_CTRL1_PM_DISABLE_ALL_CLOCKS_MASK)
      return false;

   /* check SDS PLL clock enable */
   ctrl3 = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   mask = 1 << h->channel;
   if ((ctrl3 & mask) == 0)
      return false;

   return true;
}


/******************************************************************************
 BSAT_g1_P_SdsPowerUp() - power up the sds core
******************************************************************************/
BERR_Code BSAT_g1_P_SdsPowerUp(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t reg, val, mask;

   BSAT_DEBUG_POWERDOWN(PRINT("SDS%d Power Up\n", h->channel));

   reg = BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE;
   reg += ((uint32_t)(h->channel >> 1) * 0x14);

   if (h->channel < 10)
   {
      if (h->channel & 1)
         mask = 0x53; /* rcvr1 */
      else
         mask = 0x27; /* rcvr0 */
   }
   else
   {
      if (h->channel & 1)
         mask = 0x59; /* rcvr1 */
      else
         mask = 0x33; /* rcvr0 */
   }

   BKNI_EnterCriticalSection();

   val = BREG_Read32(pDev->hRegister, reg);
   if ((val & mask) != mask)
   {
      val |= mask;
      BSAT_DEBUG_POWERDOWN(PRINT("SdsPowerUp(%d): clkgen_sds_top_2x_x_inst_clock_enable=0x%X\n", h->channel, val));
      BREG_Write32(pDev->hRegister, reg, val);
   }

   /* enable DUALSDS PLL clock */
   val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL1);
   if (val & 1)
   {
      val &= ~1;
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL1, val);
   }
   val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   val |= (1 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

   BKNI_LeaveCriticalSection();

   return BSAT_g1_P_IsSdsOn(h) ? BERR_SUCCESS : BSAT_ERR_POWERUP_FAILED;
}


/******************************************************************************
 BSAT_g1_P_SdsPowerDown() - power down the sds core
******************************************************************************/
BERR_Code BSAT_g1_P_SdsPowerDown(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val, reg, mask, mask2;

   BSAT_DEBUG_POWERDOWN(PRINT("SDS%d Power Down\n", h->channel));

   if (BSAT_g1_P_IsTfecOn_isrsafe(h))
   {
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x80); /* tfec_rst=1 */
      if (BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe(h) == false)
         BSAT_g1_P_TfecPowerDown_isrsafe(h);
   }
   BSAT_g1_P_AfecPowerDown_isrsafe(h);

   reg = BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE;
   reg += ((uint32_t)(h->channel >> 1) * 0x14);
   if (h->channel < 10)
   {
      mask2 = 0x74;
      if (h->channel & 1)
         mask = 0x50; /* rcvr1 */
      else
         mask = 0x24; /* rcvr0 */
   }
   else
   {
      mask2 = 0x6A;
      if (h->channel & 1)
         mask = 0x48; /* rcvr1 */
      else
         mask = 0x22; /* rcvr0 */
   }

   BKNI_EnterCriticalSection();

   /* read CLKGEN_SDS_TOP_2X_x_INST_CLOCK_ENABLE */
   val = BREG_Read32(pDev->hRegister, reg);
   val &= ~mask;
   if ((val & mask2) == 0)
      val = 0x03;  /* both channels are off, so turn off everything */
   BREG_Write32(pDev->hRegister, reg, val);
   BSAT_DEBUG_POWERDOWN(PRINT("SdsPowerDown(%d): clkgen_sds_top_2x_x_inst_clock_enable=0x%X\n", h->channel, val));

   /* disable DUALSDS PLL clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   val &= ~(1 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

   BKNI_LeaveCriticalSection();

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


/******************************************************************************
 BSAT_g1_P_IsAfecOn_isrsafe() - true if afec global is on
******************************************************************************/
bool BSAT_g1_P_IsAfecOn_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val;
   bool bIsOn = true;

   val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
   if ((val & (1 << h->channel)) == 0)
      bIsOn = false;
   else
   {
      /* check AFEC clock enable */
      val = BREG_Read32(pDev->hRegister, BSAT_afec_clock_enable_status_reg[h->channel]);
      if ((val & 0x03) != 0x03)
         bIsOn = false;
   }

   return bIsOn;
}


/******************************************************************************
 BSAT_g1_P_AfecPowerUp_isr() - power up the afec core
******************************************************************************/
BERR_Code BSAT_g1_P_AfecPowerUp_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val;

   BSAT_DEBUG_POWERDOWN(PRINT("AFEC%d Power Up\n", h->channel));
   if (BSAT_g1_P_IsAfecOn_isrsafe(h) == false)
   {
      if (BSAT_g1_P_IsAfecPllOn_isrsafe(h->pDevice) == false)
         BSAT_g1_P_EnableAfecPll_isr(h->pDevice, true);

      /* enable AFEC 54MHz/108MHz clocks */
      BREG_Write32(pDev->hRegister, BSAT_afec_clock_enable_reg[h->channel], 0x07);

      val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
      val |= (1 << h->channel);
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, val);

      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, ~BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK);
      return BSAT_g1_P_IsAfecOn_isrsafe(h) ? BERR_SUCCESS: BSAT_ERR_POWERDOWN_FAILED;
   }
   else
      return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecPowerDown_isrsafe() - power down the afec global core
******************************************************************************/
BERR_Code BSAT_g1_P_AfecPowerDown_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val;

   BSAT_DEBUG_POWERDOWN(PRINT("AFEC%d Power Down\n", h->channel));
   if (BSAT_g1_P_IsAfecOn_isrsafe(h))
   {
      BKNI_EnterCriticalSection();

      /* global power down */
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK);

      val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
      val &= ~(1 << h->channel);
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, val);

      /* disable AFEC clocks */
      BREG_Write32(pDev->hRegister, BSAT_afec_clock_enable_reg[h->channel], 0x4);

      if (BSAT_g1_P_IsOkToDisableAfecPll_isrsafe(h->pDevice))
         BSAT_g1_P_EnableAfecPll_isr(h->pDevice, false);

      BKNI_LeaveCriticalSection();

      return BSAT_g1_P_IsAfecOn_isrsafe(h) ? BSAT_ERR_POWERDOWN_FAILED : BERR_SUCCESS;
   }
   else
      return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetAfecClock_isrsafe() - AFEC clock should be 270 MHz
******************************************************************************/
BERR_Code BSAT_g1_P_GetAfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq)
{
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);
   uint32_t mdiv = (BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_CLKGEN_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4) & BCHP_CLKGEN_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4_MDIV_CH4_MASK) >> BCHP_CLKGEN_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4_MDIV_CH4_SHIFT;
   uint32_t div = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_CLKGEN_PLL4_SYS_PLL_DIV);
   uint32_t pdiv = (div & BCHP_CLKGEN_PLL4_SYS_PLL_DIV_PDIV_MASK) >> BCHP_CLKGEN_PLL4_SYS_PLL_DIV_PDIV_SHIFT;
   uint32_t ndiv = (div & BCHP_CLKGEN_PLL4_SYS_PLL_DIV_NDIV_INT_MASK) >> BCHP_CLKGEN_PLL4_SYS_PLL_DIV_NDIV_INT_SHIFT;

   *pFreq = (hDevImpl->xtalFreq / (pdiv * mdiv)) * ndiv;
   return BERR_SUCCESS;
}


#ifndef BSAT_EXCLUDE_TFEC
/******************************************************************************
 BSAT_g1_P_TfecPowerUp_isr() - power up the tfec core
******************************************************************************/
BERR_Code BSAT_g1_P_TfecPowerUp_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val, reg, mask;

   BSAT_DEBUG_POWERDOWN(PRINT("TFEC%d Power Up\n", h->channel));
   if (BSAT_g1_P_IsTfecOn_isrsafe(h) == false)
   {
      if (BSAT_g1_P_IsAfecPllOn_isrsafe(h->pDevice) == false)
         BSAT_g1_P_EnableAfecPll_isr(h->pDevice, true);

      if (BSAT_g1_P_IsTfecPllOn_isrsafe(h->pDevice) == false)
         BSAT_g1_P_EnableTfecPll_isr(h->pDevice, true);

      reg = BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE;
      reg += ((uint32_t)(h->channel >> 1) * 0x14);
      if (h->channel < 10)
         mask = 0x88;
      else
         mask = 0x84;
      val = BREG_Read32(pDev->hRegister, reg);
      if ((val & mask) != mask)
      {
         val |= mask;
         BREG_Write32(pDev->hRegister, reg, val);
      }

      val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
      val |= (1 << (16 + h->channel));
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

      return BSAT_g1_P_IsTfecOn_isrsafe(h) ? BERR_SUCCESS: BSAT_ERR_POWERDOWN_FAILED;
   }
   else
      return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TfecPowerDown_isrsafe() - power down the tfec core
******************************************************************************/
BERR_Code BSAT_g1_P_TfecPowerDown_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, reg, mask;

   BSAT_DEBUG_POWERDOWN(PRINT("TFEC%d Power Down\n", h->channel));

   if (BSAT_g1_P_IsTfecOn_isrsafe(h))
   {
      BINT_DisableCallback_isr(hChn->hTfecLockCb);
      BINT_DisableCallback_isr(hChn->hTfecNotLockCb);

      if (h->channel < 10)
         mask = 0x88;
      else
         mask = 0x84;

      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, 0x80); /* tfec_rst=1 */

      BKNI_EnterCriticalSection();

      val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
      val &= ~(1 << (h->channel + 16));
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

      reg = BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE;
      reg += ((uint32_t)(h->channel >> 1) * 0x14);
      val = BREG_Read32(pDev->hRegister, reg);
      val &= ~mask;
      BREG_Write32(pDev->hRegister, reg, val);

      if (BSAT_g1_P_IsOkToDisableTfecPll_isrsafe(h->pDevice))
         BSAT_g1_P_EnableTfecPll_isr(h->pDevice, false);

      if (BSAT_g1_P_IsOkToDisableAfecPll_isrsafe(h->pDevice))
         BSAT_g1_P_EnableAfecPll_isr(h->pDevice, false);

      BKNI_LeaveCriticalSection();
      return BSAT_g1_P_IsTfecOn_isrsafe(h) ? BSAT_ERR_POWERDOWN_FAILED : BERR_SUCCESS;
   }
   else
      return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_IsTfecOn_isrsafe()
******************************************************************************/
bool BSAT_g1_P_IsTfecOn_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t status, reg;
   uint32_t mask;
   bool bIsOn = false;

   mask = (BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE_SDS_TFEC_PLL_CLOCK_ENABLE_MASK | BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE_SDS0_TFEC_108_CLOCK_ENABLE_MASK);
   reg = BCHP_CLKGEN_SDS_TOP_2X_0_INST_CLOCK_ENABLE_STATUS;
   reg += ((uint32_t)(h->channel >> 1) * 0x14);
   status = BREG_Read32(pDev->hRegister, reg);
   if ((status & mask) == mask)
      bIsOn = true;

   BSAT_DEBUG_POWERDOWN(PRINT("TFEC%d is %s\n", h->channel, bIsOn ? "on" : "off"));
   return bIsOn;
}


/******************************************************************************
 BSAT_g1_P_GetTfecClock_isrsafe() - TFEC clock should be 475.2MHz (originally 486MHz)
******************************************************************************/
BERR_Code BSAT_g1_P_GetTfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq)
{
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);
   uint32_t mdiv = (BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_CLKGEN_PLL3_SYS_PLL_CHANNEL_CTRL_CH_3) & BCHP_CLKGEN_PLL3_SYS_PLL_CHANNEL_CTRL_CH_3_MDIV_CH3_MASK) >> BCHP_CLKGEN_PLL3_SYS_PLL_CHANNEL_CTRL_CH_3_MDIV_CH3_SHIFT;
   uint32_t div = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_CLKGEN_PLL3_SYS_PLL_DIV);
   uint32_t pdiv = (div & BCHP_CLKGEN_PLL3_SYS_PLL_DIV_PDIV_MASK) >> BCHP_CLKGEN_PLL3_SYS_PLL_DIV_PDIV_SHIFT;
   uint32_t ndiv = (div & BCHP_CLKGEN_PLL3_SYS_PLL_DIV_NDIV_INT_MASK) >> BCHP_CLKGEN_PLL3_SYS_PLL_DIV_NDIV_INT_SHIFT;
   *pFreq = (hDevImpl->xtalFreq / (pdiv * mdiv)) * ndiv;
   return BERR_SUCCESS;
}

#endif /* BSAT_EXCLUDE_TFEC */
