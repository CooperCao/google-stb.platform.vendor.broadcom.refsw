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
#include "bwfe_g3_priv.h"

BDBG_MODULE(bwfe_g3_priv_rffe);


/******************************************************************************
 BWFE_Rffe_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_g3_Rffe_P_PowerUp(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   BDBG_MSG(("RFFE%d pwrup", h->channel));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x80000000);    /* power up RFFE */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_SYS_CNTL, ~0x40000000);  /* reset RFFE agc */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x40000000);    /* release RFFE agc reset */
   
   return retCode;
}


/******************************************************************************
 BWFE_Rffe_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_g3_Rffe_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   BDBG_MSG(("RFFE%d pwrdown", h->channel));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_SYS_CNTL, ~0x80000000);  /* power down RFFE */
   
   return retCode;
}
