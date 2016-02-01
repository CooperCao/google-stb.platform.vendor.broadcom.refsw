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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bsynclib.h"
#include "bsynclib_channel_priv.h"

#ifndef BSYNCLIB_ALGO_H__
#define BSYNCLIB_ALGO_H__

/* needs to be >150 ms due to possible primer pts offset, but normally we expect in 150 ms range */
#define BSYNCLIB_ALGO_MAX_SUPPORTED_DELAY_MS 1000

void BSYNClib_Algo_AudioVideo_Sync(BSYNClib_Channel_Path * psAudio, BSYNClib_Channel_Path * psVideo, BSYNClib_Channel_Results * psResults);
void BSYNClib_Algo_AudioAudio_Sync(BSYNClib_Channel_Path * psPath);
void BSYNClib_Algo_VideoVideo_Sync(BSYNClib_Channel_Path * psPath);

void BSYNClib_Algo_AudioVideo_Allocator(BSYNClib_Channel_Path * psAudio, BSYNClib_Channel_Path * psVideo, BSYNClib_Channel_Results * psResults);
void BSYNClib_Algo_AudioAudio_Allocator(BSYNClib_Channel_Path * psPath);
void BSYNClib_Algo_VideoVideo_Allocator(BSYNClib_Channel_Path * psPath);

void BSYNClib_Algo_AudioVideo_MaxFinder(BSYNClib_Channel_Path * psAudio, BSYNClib_Channel_Path * psVideo, BSYNClib_Channel_Results * psResults);
void BSYNClib_Algo_AudioSource_MaxFinder(BSYNClib_Channel_Path * psPath);
void BSYNClib_Algo_AudioSink_MaxFinder(BSYNClib_Channel_Path * psPath);
void BSYNClib_Algo_VideoSource_MaxFinder(BSYNClib_Channel_Path * psPath);
void BSYNClib_Algo_VideoSink_MaxFinder(BSYNClib_Channel_Path * psPath);

BERR_Code BSYNClib_Algo_AudioSource_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2);
BERR_Code BSYNClib_Algo_AudioSink_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2);
BERR_Code BSYNClib_Algo_VideoSource_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2);
BERR_Code BSYNClib_Algo_VideoSink_Applicator(BSYNClib_Channel_Path * psPath, BSYNClib_Channel_SetDelay pfSetDelay, void * pvParm1, int iParm2);

void BSYNClib_Algo_RequantizeDelay(int iChannelIndex, unsigned int uiDelay, unsigned int uiQuantizationLevel, unsigned int * puiRequantizedDelay);
void BSYNClib_Algo_CalculateJitterToleranceImprovementFactor(BSYNClib_Channel_Path * psVideo, BSYNClib_VideoSource * psSource, unsigned int uiCurrentDelay, int * piJtiFactor, unsigned int * puiAdditionalDelay);

#endif /* BSYNCLIB_ALGO_H__ */

