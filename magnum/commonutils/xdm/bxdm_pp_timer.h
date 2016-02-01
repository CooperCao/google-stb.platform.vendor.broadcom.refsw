/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
