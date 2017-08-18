/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef bxdm_pp_TIMER_H__
#define bxdm_pp_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Set to a non zero value to compile in the function timing code.
 * To reduce overhead, only enable this code when profiling the DM.
 */
#define BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING   0

/*
 * Set to a non zero value to compile in the callback timing code.
 */
#define BXDM_PPTMR_P_ENABLE_CALLBACK_TIMING   1


#define BXDM_PP_TIMER_P_MAX_STR_LEN 256
/* SW7445-572: with the change of the preceeding from 512 to 256, there are cases
 * when the debug string will be longer than the allocated storage.  To avoid
 * truncating the message, start printing once the size of the string
 * exceeds BXDM_PP_TIMER_P_PRINT_THRESHOLD */
#define BXDM_PP_TIMER_P_PRINT_THRESHOLD ( BXDM_PP_TIMER_P_MAX_STR_LEN - 64 )


typedef struct BXDM_PPTIMER_P_Sample
{
   uint32_t    uiNumSamples;
   uint32_t    uiStartTimeUsecs;
   uint32_t    uiEndTimeUsecs;
   uint32_t    uiElapseTimeUsecs;
   uint32_t    uiTotalTimeUsecs;

   uint32_t    uiAverageTimeUsecs;
   uint32_t    uiMinTimeUsecs;
   uint32_t    uiMaxTimeUsecs;

} BXDM_PPTIMER_P_Sample;

typedef enum BXDM_PPTIMER_P_Function
{
      BXDM_PPTIMER_P_Function_eMainIsr,
      BXDM_PPTIMER_P_Function_eMainInitPerVsync,
      BXDM_PPTIMER_P_Function_eDecoderDisplayInterruptEventIsr,
      BXDM_PPTIMER_P_Function_eMainSelectElement,
      BXDM_PPTIMER_P_Function_eSelPic1,
      BXDM_PPTIMER_P_Function_eSelPic2,
      BXDM_PPTIMER_P_Function_eSelPic3,
      BXDM_PPTIMER_P_Function_eValidatePictureHead,
      BXDM_PPTIMER_P_Function_eValidatePictureTail,
      BXDM_PPTIMER_P_Function_eEvaluateTsmState,
      BXDM_PPTIMER_P_Function_eMainCalculateVDC,
      BXDM_PPTIMER_P_Function_eMainUpdatePublic,
      BXDM_PPTIMER_P_Function_eMainCallbacks,
      BXDM_PPTIMER_P_Function_eMainPostIsr,
      BXDM_PPTIMER_P_Function_eDecoderGetPictureCountIsr,
      BXDM_PPTIMER_P_Function_eDecoderPeekAtPictureIsr,
      BXDM_PPTIMER_P_Function_eDecoderGetNextPictureIsr,
      BXDM_PPTIMER_P_Function_eDecoderReleasePictureIsr,
      BXDM_PPTIMER_P_Function_eDecoderGetPictureDropPendingCountIsr,
      BXDM_PPTIMER_P_Function_eDecoderRequestPictureDropIsr,

      BXDM_PPTIMER_P_Function_eMax
} BXDM_PPTIMER_P_Function;

typedef struct BXDM_PPTIMER_P_Data
{

   uint32_t    uiVsyncCnt;

#if BXDM_PPTMR_P_ENABLE_FUNCTION_TIMING
   /* XDM Function Times */
   BXDM_PPTIMER_P_Sample   astFunctions[BXDM_PPTIMER_P_Function_eMax];
#endif

   /* XDM Callback Times */
   BXDM_PPTIMER_P_Sample   astCallbacks[BXDM_PictureProvider_Callback_eMax];

}  BXDM_PPTIMER_P_Data;

/* SWSTB-6135 */
extern const uint32_t BXDM_P_VsyncsPerSecondLUT[];

#if BDBG_DEBUG_BUILD

void BXDM_PPTMR_P_SnapshotFunctionStartTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPTIMER_P_Function eFunctionIndex
   );

void BXDM_PPTMR_P_SnapshotFunctionEndTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PPTIMER_P_Function eFunctionIndex
   );

void BXDM_PPTMR_P_SnapshotCallbackStartTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_Callback eCallbackIndex
   );

void BXDM_PPTMR_P_SnapshotCallbackEndTime_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_Callback eCallbackIndex
   );

void BXDM_PPTMR_P_PrintResults_isr(
   BXDM_PictureProvider_Handle hXdmPP
   );

#else

/* non debug build */

#define BXDM_PPTMR_P_SnapshotFunctionStartTime_isr( hXdmPP, eFunctionIndex )
#define BXDM_PPTMR_P_SnapshotFunctionEndTime_isr( hXdmPP, eFunctionIndex )
#define BXDM_PPTMR_P_SnapshotCallbackStartTime_isr( hXdmPP, eCallbackIndex )
#define BXDM_PPTMR_P_SnapshotCallbackEndTime_isr( hXdmPP, eCallbackIndex )
#define BXDM_PPTMR_P_PrintResults_isr( hXdmPP )

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_TIMER_H__ */
