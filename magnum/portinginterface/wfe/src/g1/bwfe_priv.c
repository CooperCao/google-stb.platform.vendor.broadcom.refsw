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


/******************************************************************************
 BWFE_P_ReadModifyWriteRegister()
******************************************************************************/
void BWFE_P_ReadModifyWriteRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, reg, &val);
   val = (val & and_mask) | or_mask;
   BWFE_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_OrRegister()
******************************************************************************/
void BWFE_P_OrRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, reg, &val);
   val |= or_mask;
   BWFE_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_AndRegister()
******************************************************************************/
void BWFE_P_AndRegister(BWFE_ChannelHandle h, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   BWFE_P_ReadRegister(h, reg, &val);
   val &= and_mask;
   BWFE_P_WriteRegister(h, reg, val);
}


/******************************************************************************
 BWFE_P_ToggleBit()
******************************************************************************/
void BWFE_P_ToggleBit(BWFE_ChannelHandle h, uint32_t reg, uint32_t mask)
{
   uint32_t val;
   
   BWFE_P_ReadRegister(h, reg, &val);
      
   val |= mask;
   BWFE_P_WriteRegister(h, reg, val);
      
   val &= ~mask;
   BWFE_P_WriteRegister(h, reg, val);  
}


/******************************************************************************
 BWFE_P_ReadModifyWriteLaneReg()
******************************************************************************/
void BWFE_P_ReadModifyWriteLaneReg(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadLaneRegister(h, lane, reg, &val);
   val = (val & and_mask) | or_mask;
   BWFE_P_WriteLaneRegister(h, lane, reg, val);
}


/******************************************************************************
 BWFE_P_OrLaneReg()
******************************************************************************/
void BWFE_P_OrLaneReg(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   BWFE_P_ReadLaneRegister(h, lane, reg, &val);
   val |= or_mask;
   BWFE_P_WriteLaneRegister(h, lane, reg, val);
}


/******************************************************************************
 BWFE_P_AndLaneReg()
******************************************************************************/
void BWFE_P_AndLaneReg(BWFE_ChannelHandle h, uint8_t lane, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   BWFE_P_ReadLaneRegister(h, lane, reg, &val);
   val &= and_mask;
   BWFE_P_WriteLaneRegister(h, lane, reg, val);
}