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

#ifndef BTHD_PRIV_H__
#define BTHD_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
Summary:
   This is the structure for the BTHD_Handle. 
******************************************************************************/
typedef struct BTHD_P_Handle
{
   BTHD_Settings    settings;          /* user settings */
   void             *pImpl;            /* pointer to chip-specific structure */
   uint32_t         magicId;
} BTHD_P_Handle;



#ifdef __cplusplus
}
#endif

#endif /* BTHD_PRIV_H__ */

