/***************************************************************************
 *     Copyright (c) 2004-2010, Broadcom Corporation
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
 *   See Module Overview below.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
  ***************************************************************************/
/*=************************ Module Overview ********************************
<verbatim>

Overview:

This module contains the funtion prototypes for the XVD interrupt service
routines. These are now separate from both bxvd.c and bxvd_priv.c to facilitate
easier modification and maintainence.

</verbatim>
****************************************************************************/
#ifndef BXVD_INTR_H__
#define BXVD_INTR_H__

#include "bxvd.h"

/* This value clear the whole low order word except interrupts 1 thru 4 */
#define BXVD_INTR_INTGEN_CLEAR_VALUE 0x0000ffe1

#ifdef __cplusplus
extern "C" {
#endif

void BXVD_P_AVD_MBX_isr(void *pvXvd,
		    int iParam2);

void BXVD_P_AVD_PicDataRdy_isr(void *pvXvd,
			       int iParam2);

void BXVD_P_PictureDataReady_isr(BXVD_Handle hXvd,
				 BXVD_ChannelHandle hXvdCh,
				 BAVC_XVD_Picture *pPicItem);

void BXVD_P_PictureDataRdy_NoDecode_isr(BXVD_Handle hXvd,
					BXVD_DisplayInterrupt eDisplayInterrupt,
					BAVC_XVD_Picture *pPicItem);

void BXVD_P_AVD_StillPictureRdy_isr(void *pvXvd,
				    int iParam2);

void BXVD_P_WatchdogInterrupt_isr(void *pvXvd,
				  int param2);

void BXVD_P_VidInstrChkr_isr(void *pvXvd, 
                             int param2);

void BXVD_P_StereoSeqError_isr(void *pvXvd, 
                               int param2);

#ifdef __cplusplus
}
#endif

#endif /* BXVD_INTR_H__ */
