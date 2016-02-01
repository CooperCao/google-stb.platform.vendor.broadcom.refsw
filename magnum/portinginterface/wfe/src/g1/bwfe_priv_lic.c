/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
#include "bwfe.h"
#include "bwfe_priv.h"

BDBG_MODULE(bwfe_priv_lic);


/******************************************************************************
 BWFE_Lic_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_Lic_P_PowerUp(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   BDBG_MSG(("LIC%d pwrup", h->channel));
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_LICCTL1, 0x04000000);    /* reset LIC */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_LICCTL1, 0x08000000);   /* enable LIC output */
   
   for (i = 0; i < h->totalLanes / 2; i++)
   {
      /* set dco level for lane pairs */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICDCO0 + (4 * i), 0x04000400);
   }
   
   /* reset lic taps */
   BWFE_Lic_P_ResetTaps(h);
   
#ifdef BWFE_BYPASS_LIC
   /* bypass mode for bringup */
   BDBG_MSG(("bypass LIC%d", h->channel));
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_LICCTL1, 0x01000000);    /* bypass LIC */
#endif
   
   return retCode;
}


/******************************************************************************
 BWFE_Lic_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_Lic_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   BDBG_MSG(("LIC%d pwrdown", h->channel));
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x0800000F);   /* disable LIC output, deselect lanes */
   return retCode;
}


/******************************************************************************
 BWFE_Lic_P_RunLaneCorrection()
******************************************************************************/
BERR_Code BWFE_Lic_P_RunLaneCorrection(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;
   uint8_t lane, tap;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* set lic mu */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x000F0000, h->licMu << 16);
   
   /* turn on reference lane */
   if (h->licEnableRefLane)
      BWFE_P_PowerUpRefLane(h);  /* wake ADC2 for 4550, ADC4 for 4528 */
   
   /*calibrate lanes one at a time*/
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      /* set reference phase on ADC2 for 4550, ADC4 for 4528 */
      BWFE_P_SetRefPhase(h, lane);
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x0000FFFF, 1 << lane);   /* lane selection */
      
      /* update coefficients for all taps + dco tap */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCTL2, h->licUpdateMask);
      BKNI_Sleep(h->licRuntimeMs);   /* run LIC for specified runtime */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCTL2, 0);
      
      /* read all taps  + dco tap */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCOEFA, 0);   /* auto-incrementing address */
      for (tap = 0; tap < BWFE_NUM_LIC_TAPS; tap++)
      {
         BWFE_P_ReadRegister(h, BCHP_WFE_CORE_LICCOEFD, &val);
         h->licCoeff[lane][tap] = val;
      }
   }
   
   /* deselect lanes */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x0000FFFF);
   
   /* turn off reference lane */
   BWFE_P_PowerDownRefLane(h);   /* sleep ADC2 for 4550, ADC4 for 4528 */
   
   return retCode;
}


/******************************************************************************
 BWFE_Lic_P_RunLaneCorrectionAll()
******************************************************************************/
BERR_Code BWFE_Lic_P_RunLaneCorrectionAll(BWFE_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_ChannelHandle hChn;
   uint32_t val;
   uint8_t i, lane, tap;
   
   /* prepare all enabled channels for lane correction */
   for (i = 0; i < h->totalChannels; i++)
   {
      hChn = h->pChannels[i];
      if (hChn->bEnabled)
      {
         BWFE_P_ReadModifyWriteRegister(hChn, BCHP_WFE_CORE_LICCTL1, ~0x000F0000, hChn->licMu << 16); /* set lic mu */
         if (hChn->licEnableRefLane)
            BWFE_P_PowerUpRefLane(hChn);  /* turn on reference lane */
      }
   }
   
   /*calibrate lanes one at a time*/
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      for (i = 0; i < h->totalChannels; i++)
      {
         hChn = h->pChannels[i];
         if (hChn->bEnabled)
         {
            /* set reference phase */
            BWFE_P_SetRefPhase(hChn, lane);
            BWFE_P_ReadModifyWriteRegister(hChn, BCHP_WFE_CORE_LICCTL1, ~0x0000FFFF, 1 << lane);   /* lane selection */
            
            /* update coefficients for all taps + dco tap */
            BWFE_P_WriteRegister(hChn, BCHP_WFE_CORE_LICCTL2, hChn->licUpdateMask);
         }
      }
      
      BKNI_Sleep(h->pChannels[0]->licRuntimeMs);   /* run LIC for specified runtime */
      
      for (i = 0; i < h->totalChannels; i++)
      {
         hChn = h->pChannels[i];
         if (hChn->bEnabled)
         {
            BWFE_P_WriteRegister(hChn, BCHP_WFE_CORE_LICCTL2, 0);
            
            /* read all taps  + dco tap */
            BWFE_P_WriteRegister(hChn, BCHP_WFE_CORE_LICCOEFA, 0);   /* auto-incrementing address */
            for (tap = 0; tap < BWFE_NUM_LIC_TAPS; tap++)
            {
               BWFE_P_ReadRegister(hChn, BCHP_WFE_CORE_LICCOEFD, &val);
               hChn->licCoeff[lane][tap] = val;
            }
         }
      }
   }
   
   for (i = 0; i < h->totalChannels; i++)
   {
      hChn = h->pChannels[i];
      if (hChn->bEnabled)
      {
         BWFE_P_AndRegister(hChn, BCHP_WFE_CORE_LICCTL1, ~0x0000FFFF);  /* deselect lanes */
         BWFE_P_PowerDownRefLane(hChn);   /* turn off reference lane */
      }
   }
   
   return retCode;
}


/******************************************************************************
 BWFE_Lic_P_ResetTaps()
******************************************************************************/
BERR_Code BWFE_Lic_P_ResetTaps(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   /* reset LIC and read default taps */
   BWFE_P_ToggleBit(h, BCHP_WFE_CORE_LICCTL1, 0x04000000);
   retCode = BWFE_Lic_P_UpdateTaps(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_Lic_P_UpdateTaps()
******************************************************************************/
BERR_Code BWFE_Lic_P_UpdateTaps(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t lane, tap;
   uint32_t val;
   
   for (lane = 0; lane < BWFE_NUM_LANES; lane++)
   {
      /* lane selection */
      BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x0000FFFF, 1 << lane);
      
      /* read lic taps  + dco tap */
      BWFE_P_WriteRegister(h, BCHP_WFE_CORE_LICCOEFA, 0);   /* auto-incrementing address */
      for (tap = 0; tap < BWFE_NUM_LIC_TAPS; tap++)
      {
         BWFE_P_ReadRegister(h, BCHP_WFE_CORE_LICCOEFD, &val);
         h->licCoeff[lane][tap] = val;
      }
   }
   
   /* deselect lanes */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_LICCTL1, ~0x0000FFFF);
   
   return retCode;
}
