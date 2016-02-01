/***************************************************************************
 *	   Copyright (c) 2007-2009, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *      OTF platform specific file
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#ifndef BOTF_PLATFORM_H_
#define BOTF_PLATFORM_H_

 /* Magnum */
#include "bstd.h"
#include "bkni.h"
#include "bmem.h"
#include "bxpt.h"
#include "bxpt_rave.h"

/* OTF */
#include "botf.h"
#include "botf_priv.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void BOTF_P_UpdateBufferPtrs(BOTF_Handle hOtf);
void BOTF_P_InitBufferPtrs(BOTF_Handle hOtf);
void BOTF_P_SetIPReadPtrs(BOTF_Handle hOtf, const void *CdbReadPtr, const void *ItbReadPtr);
void BOTF_P_SetOPITBValidPtr(BOTF_Handle hOtf);
void BOTF_P_SetOPITBWrapPtr(BOTF_Handle hOtf);
void BOTF_PlatformOpen(BOTF_Handle hOtf);
void BOTF_PlatformClose(BOTF_Handle hOtf);
void BOTF_PlatformFlushOpParser(BOTF_Handle hOtf);

#endif /* BOTF_PLATFORM_H_ */
