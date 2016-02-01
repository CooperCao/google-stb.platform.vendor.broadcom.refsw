/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
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
#include "bsrf.h"
#include "bsrf_priv.h"
#include "bsrf_g1_priv.h"
#include "bsrf_lonestar_priv.h"


BDBG_MODULE(bsrf_lonestar_priv);


/******************************************************************************
 BSRF_P_GetRegisterAddress()
******************************************************************************/
void BSRF_P_GetRegisterAddress(BSRF_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;
   /* BDBG_MSG(("GetRegisterAddress(%d) %08X->%08X", h->channel, reg, *pAddr)); */
}


/******************************************************************************
 BSRF_P_ReadRegister()
******************************************************************************/
BERR_Code BSRF_P_ReadRegister(
   BSRF_ChannelHandle h,      /* [in] BSCS channel handle */
   uint32_t           reg,    /* [in] address of register to read */
   uint32_t           *val    /* [out] contains data that was read */
)
{
   BSRF_g1_P_Handle *hDev = (BSRF_g1_P_Handle *)h->pDevice->pImpl;
   uint32_t addr;

   BSRF_P_GetRegisterAddress(h, reg, &addr);
   *val = BREG_Read32(hDev->hRegister, addr);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_P_WriteRegister()
******************************************************************************/
BERR_Code BSRF_P_WriteRegister(
   BSRF_ChannelHandle h,      /* [in] BSCS channel handle */
   uint32_t           reg,    /* [in] address of register to write */
   uint32_t           val     /* [in] contains data to be written */
)
{
   BSRF_g1_P_Handle *hDev = (BSRF_g1_P_Handle *)h->pDevice->pImpl;
   uint32_t addr;

   BSRF_P_GetRegisterAddress(h, reg, &addr);
   BREG_Write32(hDev->hRegister, addr, val);

   return BERR_SUCCESS;
}
