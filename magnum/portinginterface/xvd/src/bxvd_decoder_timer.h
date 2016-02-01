/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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

#ifndef BXVD_DECODER_TIMER_H__
#define BXVD_DECODER_TIMER_H__

#include "bxvd.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Set to a non zero value to compile in the function timing code.
 * To reduce overhead, only enable this code when profiling the DM.
 */
#define BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING   0

/*
 * Set to a non zero value to compile in the callback timing code.
 */
#define BXVD_DECODER_TIMER_S_ENABLE_CALLBACK_TIMING   1


#define BXVD_DECODER_TIMER_P_MAX_STR_LEN 512

typedef struct BXVD_DecoderTimer_P_Sample
{
   uint32_t    uiNumSamples;
   uint32_t    uiStartTimeUsecs;
   uint32_t    uiEndTimeUsecs;
   uint32_t    uiElapseTimeUsecs;
   uint32_t    uiTotalTimeUsecs;

   uint32_t    uiAverageTimeUsecs;
   uint32_t    uiMinTimeUsecs;
   uint32_t    uiMaxTimeUsecs;

} BXVD_DecoderTimer_P_Sample;

/* To make the result easier to interpret, these should be in the order that calls are made.
 * Note: they can be any sample point in the code, not just function entry/exit points
 */
typedef enum BXVD_DecoderTimer_P_eFunc
{
   BXVD_DecoderTimer_P_eFuncDisplayIsr,
   BXVD_DecoderTimer_P_eFuncUQ_Vet,
   BXVD_DecoderTimer_P_eFuncUQ_DQT,
   BXVD_DecoderTimer_P_eFuncUQ_Val,
   BXVD_DecoderTimer_P_eFuncUQ_AddPic,

   BXVD_DecoderTimer_P_eFuncGetPicCount,
   BXVD_DecoderTimer_P_eFuncPeekAtPic,
   BXVD_DecoderTimer_P_eFuncGetNextPic,
   BXVD_DecoderTimer_P_eFuncReleasePic,

   BXVD_DecoderTimer_P_eFuncMax

} BXVD_DecoderTimer_P_eFunc;


typedef enum BXVD_DecoderTimer_P_eCallbacks
{
   BXVD_DecoderTimer_P_eCbUserData,
   BXVD_DecoderTimer_P_eCbPPBReceived,
   BXVD_DecoderTimer_P_eCbMax

} BXVD_DecoderTimer_P_eCallbacks;


typedef struct BXVD_DecoderTimer_P_Data
{

#if BXVD_DECODER_TIMER_S_ENABLE_FUNCTION_TIMING

   /* Decoder Function Times */
   BXVD_DecoderTimer_P_Sample   astFunctions[BXVD_DecoderTimer_P_eFuncMax];

#endif

   /* Decoder Callback Times */
   BXVD_DecoderTimer_P_Sample   astCallbacks[BXVD_DecoderTimer_P_eCbMax];

}  BXVD_DecoderTimer_P_Data;

#if BDBG_DEBUG_BUILD

void BXVD_DecoderTimer_P_SnapshotFunctionStartTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eFunc eFunctionIndex
   );
void BXVD_DecoderTimer_P_SnapshotFunctionEndTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eFunc eFunctionIndex
   );

void BXVD_DecoderTimer_P_SnapshotCallbackStartTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eCallbacks eCallbackIndex
   );

void BXVD_DecoderTimer_P_SnapshotCallbackEndTime_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_DecoderTimer_P_eCallbacks eCallbackIndex
   );

void BXVD_DecoderTimer_P_PrintResults_isr(
   BXVD_ChannelHandle hXvdCh
   );

#else

/* non debug build */

#define BXVD_DecoderTimer_P_SnapshotFunctionStartTime_isr( hXvdCh, eFunctionIndex )
#define BXVD_DecoderTimer_P_SnapshotCallbackStartTime_isr( hXvdCh, eCallbackIndex )
#define BXVD_DecoderTimer_P_SnapshotFunctionEndTime_isr( hXvdCh, eFunctionIndex )
#define BXVD_DecoderTimer_P_SnapshotCallbackEndTime_isr( hXvdCh, eCallbackIndex )
#define BXVD_DecoderTimer_P_PrintResults_isr( hXvdCh )

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXVD_DECODER_TIMER_H__ */
