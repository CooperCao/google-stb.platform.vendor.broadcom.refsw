/***************************************************************************
 *     Copyright (c) 2004, Broadcom Corporation
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

#ifndef BVSB_PRIV_H__
#define BVSB_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
Summary:
   This is the structure for the BVSB_Handle. 
******************************************************************************/
typedef struct BVSB_P_Handle
{
   BVSB_Settings    settings;          /* user settings */
   void             *pImpl;            /* pointer to chip-specific structure */
} BVSB_P_Handle;



#ifdef __cplusplus
}
#endif

#endif /* BVSB_PRIV_H__ */

