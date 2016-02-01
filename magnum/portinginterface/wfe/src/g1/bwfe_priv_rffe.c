/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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

BDBG_MODULE(bwfe_priv_rffe);


/******************************************************************************
 BWFE_Rffe_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_Rffe_P_PowerUp(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_MSG(("RFFE%d pwrup", h->channel));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_RFFE_WRITER01_IN, 0x00000200);  /* bias control not shared */
   BWFE_P_ToggleBit(h, BCHP_WFE_ANA_U7_RFFE_SIGR01_IN, 0x00002000);     /* reset  RFPGA */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, 0x80000000); /* power up LNA/PGA */
   
   return retCode;
}


/******************************************************************************
 BWFE_Rffe_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_Rffe_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_MSG(("RFFE%d pwrdown", h->channel));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_U7_WBADC_SYS_CNTL_IN, ~0x80000000); /* power down LNA/PGA */
   
   return retCode;
}