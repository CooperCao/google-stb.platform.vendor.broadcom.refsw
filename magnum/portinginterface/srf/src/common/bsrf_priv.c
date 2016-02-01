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
#include "bsrf.h"
#include "bsrf_priv.h"

BDBG_MODULE(bsrf_priv);


/******************************************************************************
 BSRF_P_ReadModifyWriteRegister()
******************************************************************************/
void BSRF_P_ReadModifyWriteRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   BSRF_P_ReadRegister(h, reg, &val);
   val = (val & and_mask) | or_mask;
   BSRF_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BSRF_P_OrRegister()
******************************************************************************/
void BSRF_P_OrRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   BSRF_P_ReadRegister(h, reg, &val);
   val |= or_mask;
   BSRF_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BSRF_P_AndRegister()
******************************************************************************/
void BSRF_P_AndRegister(BSRF_ChannelHandle h, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   BSRF_P_ReadRegister(h, reg, &val);
   val &= and_mask;
   BSRF_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BSRF_P_ToggleBit()
******************************************************************************/
void BSRF_P_ToggleBit(BSRF_ChannelHandle h, uint32_t reg, uint32_t mask)
{
   uint32_t val;

   BSRF_P_ReadRegister(h, reg, &val);

   val |= mask;
   BSRF_P_WriteRegister(h, reg, val);

   val &= ~mask;
   BSRF_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BSRF_P_GCF()
******************************************************************************/
uint32_t BSRF_P_GCF(uint32_t m, uint32_t n)
{
   uint32_t L1, L2, L3, L4;

   L1 = m;
   L2 = n;

   while (L2 > 0)
   {
      L3 = L1 / L2;
      L4 = L1 - (L2 * L3);
      L1 = L2;
      L2 = L4;
   }
   return L1;
}
