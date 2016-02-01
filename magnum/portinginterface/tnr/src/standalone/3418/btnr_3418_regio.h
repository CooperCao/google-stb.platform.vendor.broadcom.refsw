/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BTNR_3418_REGIO_H__
#define BTNR_3418_REGIO_H__

#ifdef __cplusplus
extern "C" {
#endif

void BTNR_P_3418_WriteReg( BTNR_P_3418_Settings *pTnrImplData, unsigned bytOffset, uint8_t bytData );

void BTNR_P_3418_ReadReg( BTNR_P_3418_Settings *pTnrImplData, unsigned bytOffset, uint8_t *pbytData );

void BTNR_P_3418_WriteField8( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint8_t bytData );

void BTNR_P_3418_WriteField16( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint16_t sData );

void BTNR_P_3418_ReadField8( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint8_t *pbytData );

void BTNR_P_3418_ReadField16( BTNR_P_3418_Settings *pTnrImplData, char *pszField, uint16_t *psData );

int BTNR_P_3418_GetRegNumber();

#ifdef __cplusplus
}
#endif
 
#endif

