/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#ifndef BDSP_AUDIO_TASK_H_
#define BDSP_AUDIO_TASK_H_

#include "bchp.h"
#include "bint.h"
#include "bmem.h"
#include "breg_mem.h"
#include "btmr.h"
#include "bimg.h"
#include "bdsp_task.h"
#include "bdsp_raaga_fw_settings.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_status.h"



/***************************************************************************
Summary:
    This structure returns information of bit rate change in stream

Notes:
    This structure will be passed on to application callback function
    on bit rate change interrupt
***************************************************************************/
typedef struct BDSP_AudioBitRateChangeInfo
{
    BDSP_Algorithm   eAlgorithm;   /* audio type */
    uint32_t ui32BitRate;    /* New Bit Rate value*/
                         /* If eType = DTS or DTS-HD and
                         ui32BitRateIndex = 29,30 or 31
                         ui32BitRate = 0 */
    uint32_t ui32BitRateIndex;    /* This has the Bit rate index
                                as given in the standard. This value
                                is zero for audio type AAC-HE*/
} BDSP_AudioBitRateChangeInfo;

/***************************************************************************
Summary:
Pause an audio DSP task
***************************************************************************/
BERR_Code BDSP_AudioTask_Pause(
    BDSP_TaskHandle task
    );

/***************************************************************************
Summary:
Resume an audio DSP task
***************************************************************************/
BERR_Code BDSP_AudioTask_Resume(
    BDSP_TaskHandle task
    );

/***************************************************************************
Summary:
Advance an audio DSP task
***************************************************************************/
BERR_Code BDSP_AudioTask_Advance(
    BDSP_TaskHandle task,
    unsigned ms
    );

/* PAUSE-UNPAUSE */

/***************************************************************************
Summary:
Get Default Task Freeze Settings
***************************************************************************/
void BDSP_AudioTask_GetDefaultFreezeSettings(
    BDSP_AudioTaskFreezeSettings *pSettings /*[out]*/
    );

/***************************************************************************
Summary:
Get Default Task UnFreeze Settings
***************************************************************************/
void BDSP_AudioTask_GetDefaultUnFreezeSettings(
    BDSP_AudioTaskUnFreezeSettings *pSettings /*[out]*/
    );

/***************************************************************************
Summary:
Freeze an audio DSP task
***************************************************************************/
BERR_Code BDSP_AudioTask_Freeze(
    BDSP_TaskHandle task,
    const BDSP_AudioTaskFreezeSettings *pFreezeSettings
    );

/***************************************************************************
Summary:
UnFreeze an audio DSP task
***************************************************************************/
BERR_Code BDSP_AudioTask_UnFreeze(
    BDSP_TaskHandle task,
    const BDSP_AudioTaskUnFreezeSettings *pUnFreezeSettings
    );

/* PAUSE-UNPAUSE */

/***************************************************************************
Summary:
Audio Interrupt Handlers for a task
***************************************************************************/
typedef struct BDSP_AudioInterruptHandlers
{
    /* Interrupt fires when first PTS is received */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } firstPts;
    /* Interrupt fires when TSM Fail (PTS Error) occurs */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } tsmFail;
    /* Interrupt fires when TSM transitions from fail -> pass in ASTM mode */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus);
        void *pParam1;
        int param2;
    } tsmPass;
    /* Interrupt fires when the decoder receives the first or any new sample rate in the stream */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2, unsigned streamSampleRate, unsigned baseSampleRate);
        void *pParam1;
        int param2;
    } sampleRateChange;
    /* Interrupt fires when the decoder achieves frame lock */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } lock;
    /* Interrupt fires when the decoder loses frame lock */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } unlock;
    /* Interrupt fires when the status buffer is valid */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } statusReady;
        /* Interrupt fires when the mode change happens */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2,const uint32_t ui32ModeValue);
        void *pParam1;
        int param2;
    } modeChange;

        /* Interrupt fires when the bitrate change happens */
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2,const BDSP_AudioBitRateChangeInfo *bitrateChangeInfo);
        void *pParam1;
        int param2;
    } bitrateChange;

    /* interrupt fires only when CDB underflows after the gate is open*/
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } CdbItbUnderflowAfterGateOpen;

    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } cdbItbOverflow;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } cdbItbUnderflow;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } startPtsReached;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } stopPtsReached;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } unlicensedAlgo;
   struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } rampEnable;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } encoderOutputOverflow;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } ancillaryData;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } dialnormChange;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } vencDataDiscarded;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } onDemandAudioFrameDelivered;
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } targetVolumeLevelReached;

} BDSP_AudioInterruptHandlers;

/***************************************************************************
Summary:
Get Current Interrupt Handlers for a task
***************************************************************************/
void BDSP_AudioTask_GetInterruptHandlers_isr(
    BDSP_TaskHandle task,
    BDSP_AudioInterruptHandlers *pHandlers   /* [out] */
    );

/***************************************************************************
Summary:
Set Current Interrupt Handlers for a task
***************************************************************************/
BERR_Code BDSP_AudioTask_SetInterruptHandlers_isr(
    BDSP_TaskHandle task,
    const BDSP_AudioInterruptHandlers *pHandlers
    );


/***************************************************************************
Summary:
Get default Datasync settings.
***************************************************************************/
BERR_Code BDSP_AudioTask_GetDefaultDatasyncSettings(
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
    );

/***************************************************************************
Summary:
Get default Tsm settings.
***************************************************************************/
BERR_Code BDSP_AudioTask_GetDefaultTsmSettings(
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
    );

/***************************************************************************
Summary:
Command to put an audio task in zero fill mode. Required for NRT xcode case, where incase
of audio gaps, audio stalls video too. This command avoids this deadlock.
***************************************************************************/
BERR_Code BDSP_AudioTask_AudioGapFill(
    BDSP_TaskHandle task
    );

#endif
