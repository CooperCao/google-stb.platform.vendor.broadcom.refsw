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
#include "bstd.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g2.h"
#include "bast_g2_priv.h"
#include "bast_7340_priv.h"


BDBG_MODULE(bast_7340);


const uint32_t BAST_Sds_ChannelIntrID[BAST_G2_MAX_CHANNELS][BAST_Sds_MaxIntID] = {
   /* channel 0 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_0,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_0,
      BCHP_INT_ID_SDS_BTM_IS_0,
      BCHP_INT_ID_SDS_BRTM_IS_0,
      BCHP_INT_ID_SDS_GENTM_IS1_0,
      BCHP_INT_ID_SDS_GENTM_IS2_0,
      BCHP_INT_ID_SDS_XTLTM_IS_0,
      BCHP_INT_ID_SDS_if_agc_le_thresh_rise_0,
      BCHP_INT_ID_SDS_if_agc_le_thresh_fall_0,
      BCHP_INT_ID_SDS_rf_agc_le_thresh_rise_0,
      BCHP_INT_ID_SDS_rf_agc_le_thresh_fall_0,
      BCHP_INT_ID_SDS_rf_agc_gt_max_change_0,
      BCHP_INT_ID_SDS_sar_vol_gt_hi_thrsh_0,
      BCHP_INT_ID_SDS_sar_vol_lt_lo_thrsh_0,
      BCHP_INT_ID_DSDN_IS_0,
      BCHP_INT_ID_SDS_HP_IS_0,
      BCHP_INT_ID_MI2C_IS_0,
      BCHP_INT_ID_TURBO_LOCK_IS_0,
      BCHP_INT_ID_TURBO_NOT_LOCK_IS_0,
      BCHP_INT_ID_AFEC_LOCK_IS_0,
      BCHP_INT_ID_AFEC_NOT_LOCK_IS_0
   },
   /* channel 1 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_IS_1,
      BCHP_INT_ID_SDS_NOT_LOCK_IS_1,
      BCHP_INT_ID_SDS_BTM_IS_1,
      BCHP_INT_ID_SDS_BRTM_IS_1,
      BCHP_INT_ID_SDS_GENTM_IS1_1,
      BCHP_INT_ID_SDS_GENTM_IS2_1,
      BCHP_INT_ID_SDS_XTLTM_IS_1,
      BCHP_INT_ID_SDS_if_agc_le_thresh_rise_1,
      BCHP_INT_ID_SDS_if_agc_le_thresh_fall_1,
      BCHP_INT_ID_SDS_rf_agc_le_thresh_rise_1,
      BCHP_INT_ID_SDS_rf_agc_le_thresh_fall_1,
      BCHP_INT_ID_SDS_rf_agc_gt_max_change_1,
      0,
      0,
      BCHP_INT_ID_DSDN_IS_1,
      BCHP_INT_ID_SDS_HP_IS_1,
      BCHP_INT_ID_MI2C_IS_1,
      0,
      0,
      0,
      0
   }
};


/******************************************************************************
 BAST_g2_P_InitHandle()
******************************************************************************/
void BAST_g2_P_InitHandle(BAST_Handle h)
{
   BAST_g2_P_Handle *hDev = (BAST_g2_P_Handle *)(h->pImpl);

   hDev->xtalFreq = BAST_G2_XTAL_FREQ;
   hDev->ndiv = 55;
   hDev->m1div = 11;
   hDev->m2div = 7;
   hDev->m3div = 6;
}


/******************************************************************************
 BAST_g2_P_InitConfig()
******************************************************************************/
void BAST_g2_P_InitConfig(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (h->channel == 0)
   {
      hChn->bHasDiseqc = true;
      hChn->bExternalTuner = false;
      hChn->bHasDedicatedPll = true;
      hChn->bHasTunerDacPll = true;
      hChn->xportCtl = 0x1A18; /* parallel out, clkinv, clksup, tei */
      hChn->bHasAfec = true;
      hChn->bHasTfec = true;
   }
   else
   {
      hChn->bHasDiseqc = false;
      hChn->bExternalTuner = true;
      hChn->bHasTunerDacPll = false;
      hChn->bHasDedicatedPll = false;
      hChn->xportCtl = 0x1A18; /* parallel out, clkinv, clksup, tei */
      hChn->bHasAfec = false;
      hChn->bHasTfec = false;
   }
   hChn->tunerCtl = 0;
}


/******************************************************************************
 BAST_g2_P_ReadRegister_isrsafe()
******************************************************************************/
BERR_Code BAST_g2_P_ReadRegister_isrsafe(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to read */
   uint32_t           *val   /* [out] contains data that was read */
)
{
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t addr = reg;

   if ((addr >= 0x01100000) && (addr <= 0x011007F4) && (h->channel == 1))
   {
      /* do address translation for QPSK core only */
      addr &= 0x000FFFFF;
      addr |= 0x01300000;
   }
   *val = BREG_Read32(hDev->hRegister, addr);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_WriteRegister_isrsafe()
******************************************************************************/
BERR_Code BAST_g2_P_WriteRegister_isrsafe(
   BAST_ChannelHandle h,     /* [in] BAST channel handle */
   uint32_t           reg,   /* [in] address of register to write */
   uint32_t           *val   /* [in] contains data to be written */
)
{
   BAST_g2_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t addr = reg;

   if ((addr >= 0x01100000) && (addr <= 0x011007F4) && (h->channel == 1))
   {
      /* do address translation for QPSK core only */
      addr &= 0x000FFFFF;
      addr |= 0x01300000;
   }

   BREG_Write32(hDev->hRegister, addr, *val);
   return BERR_SUCCESS;
}
