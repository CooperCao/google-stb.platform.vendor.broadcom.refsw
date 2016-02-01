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

#include "bstd.h" 
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g2_priv.h"

BDBG_MODULE(bwfe_g2_priv_rffe);


/******************************************************************************
 BWFE_Rffe_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_g2_Rffe_P_PowerUp(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   BDBG_MSG(("RFFE%d pwrup", h->channel));
#ifdef BWFE_HAS_RFFE
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_RFFE_WRITER01_IN, 0x00000200);  /* bias control not shared */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_RFFE_SIGR01_IN, 0x00002000);     /* reset  RFPGA */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, 0x80000000); /* power up LNA/PGA */
#else
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_RFFE_CNTL0_IN, ~0x0000007E, 0x00000000);  /* set max P and N source currents */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_RFFE_CNTL0_IN, ~0x00000001);   /* power up adc buffer */
#endif
   return retCode;
}


/******************************************************************************
 BWFE_Rffe_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_g2_Rffe_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   BDBG_MSG(("RFFE%d pwrdown", h->channel));
#ifdef BWFE_HAS_RFFE
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_WBADC_SYS_CNTL_IN, ~0x80000000);  /* power down LNA/PGA */
#else
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_RFFE_CNTL0_IN, 0x00000001);  /* power down adc buffer */
#endif
   return retCode;
}


/******************************************************************************
 BWFE_g2_Rffe_P_EnableDpmMode()
******************************************************************************/
BERR_Code BWFE_g2_Rffe_P_EnableDpmMode(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

   if (h->bReference)
      return BERR_NOT_SUPPORTED;

#ifdef BWFE_HAS_RFFE
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_RFFE_WRITER01_IN, 0x00000020);  /* enable DPM mode */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, 0x00000030);      /* power up DPM pilot gen */
#else
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, ~0x00000001);       /* power up shaper_2_4 */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_DPM_PLL_CNTL0_IN, ~0x00000003);   /* power up DPM pilot gen */

   /* force pll reset */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_DPM_PLL_CNTL0_IN, 0x00000002);

   /* pilot gen reset, must hold */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, 0x00000008);
   BKNI_Delay(100);
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, ~0x00000008);
#endif
   return retCode;
}


/******************************************************************************
 BWFE_g2_Rffe_P_DisableDpmMode()
******************************************************************************/
BERR_Code BWFE_g2_Rffe_P_DisableDpmMode(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;

#ifdef BWFE_HAS_RFFE
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_RFFE_WRITER01_IN, ~0x00000020);   /* disable DPM mode */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, ~0xF0000030, 0x00000020); /* power down DPM pilot gen */   
#else
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_DPM_CNTL0_IN, 0x00000001);      /* power down shaper_2_4 */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_DPM_PLL_CNTL0_IN, 0x00000003);  /* power down DPM pilot gen and hold in reset */
#endif
   return retCode;
}
