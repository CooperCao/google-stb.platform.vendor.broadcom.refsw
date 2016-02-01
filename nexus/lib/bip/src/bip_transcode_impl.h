/******************************************************************************
 * (c) 2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#ifndef BIP_TRANSCODE_IMPL_H
#define BIP_TRANSCODE_IMPL_H

#include "bip.h"
#include "b_playback_ip_lib.h"
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

/*
 * States for Transcode Object.
 */
typedef enum BIP_TranscodeState
{
    BIP_TranscodeState_eUninitialized = 0,           /* Initial state: object is just created but not fully initialzed. */
    BIP_TranscodeState_eIdle,                        /* Idle state: transcode object is either just created or is just stopped. */
    BIP_TranscodeState_ePrepared,                    /* Prepared for Transcoding. */
    BIP_TranscodeState_eTranscoding,                 /* Transcoding is started. */
    BIP_TranscodeState_eMax
} BIP_TranscodeState;

typedef struct BIP_Transcode
{
    BDBG_OBJECT( BIP_Transcode )

    B_MutexHandle                                   hStateMutex;                          /* Mutex to synchronize a API invocation with callback invocation */
    BIP_Status                                      completionStatus;

    BIP_TranscodeCreateSettings                     createSettings;
    BIP_TranscodeSettings                           settings;
    BIP_TranscodeStartSettings                      startSettings;
    BIP_TranscodeProfile                            transcodeProfile;

    BIP_TranscodeState                              state;                                /* Main Transcode state. */
    bool                                            openedNexusHandles;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    BIP_TranscodeNexusHandles                       nexusHandles;
    NEXUS_SimpleStcChannelHandle                    stcChannel;
    NEXUS_SimpleVideoDecoderStartSettings           videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings           audioProgram;
    NEXUS_SimpleEncoderStartSettings                encoderStartSettings;
    bool                                            encoderStarted;
    bool                                            audioDecoderStarted;
    bool                                            videoDecoderStarted;
    bool                                            nxClientConnected;
    unsigned                                        connectId;
#endif/* NEXUS_HAS_VIDEO_ENCODER */

    /* One Arb per API */
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_TranscodeSettings      *pSettings;
    } getSettingsApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_TranscodeSettings      *pSettings;
    } setSettingsApi;
    struct
    {
        BIP_ArbHandle                               hArb;
        BIP_TranscodeStreamInfo                      transcodeStreamInfo;
        BIP_TranscodeProfile                        *pTranscodeProfile;
        NEXUS_RecpumpHandle                         hRecpump;
        bool                                        nonRealTime;
        BIP_TranscodePrepareSettings                settings;
    } prepareApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_TranscodeStartSettings *pSettings;
    } startApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } stopApi;
    struct
    {
        BIP_ArbHandle               hArb;
        BIP_TranscodeStatus          *pStatus;
    } getStatusApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } printStatusApi;
    struct
    {
        BIP_ArbHandle               hArb;
    } destroyApi;
#if 0
    BIP_TranscodeStats stats;
#endif
} BIP_Transcode;


#define BIP_TRANSCODE_PRINTF_FMT  \
    "[hTranscode=%p State=%s ]"

#define BIP_TRANSCODE_PRINTF_ARG(pObj)                                                                   \
    (pObj),                                                                                              \
    (pObj)->state==BIP_TranscodeState_eIdle                  ? "Idle"                                :   \
    (pObj)->state==BIP_TranscodeState_ePrepared              ? "Prepared"                            :   \
    (pObj)->state==BIP_TranscodeState_eTranscoding           ? "Transcoding"                         :   \
                                                               "StateNotMapped"

#endif /* BIP_TRANSCODE_IMPL_H */
