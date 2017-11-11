/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 ******************************************************************************/

#ifndef NEXUS_SAGE_AUDIO_H__
#define NEXUS_SAGE_AUDIO_H__

#include "nexus_audio_types.h"
#include "nexus_rave.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _MAX_SARM_AUDIO_STREAMS     3
#define NEXUS_SAGE_AUDIO_RAVE_ALIGN 4096

typedef enum
{
    NEXUS_SageAudioState_eNone = 0, /* No Sync WORD Found */
    NEXUS_SageAudioState_eInit,     /* Sync WORD found with byte by byte search */
    NEXUS_SageAudioState_eSteady,   /* Sync WORD found with frame size */
    NEXUS_SageAudioState_eStop,     /* Stream is stopped */
    NEXUS_SageAudioState_eMax       /* Validity check */
} NEXUS_SageAudioState;

typedef struct NEXUS_SageAudioStatus
{
    /* SARM TA State for specified stream */
    NEXUS_SageAudioState state;
    /* Number of frames successfully copied to destination ITB/CDB */
    unsigned             numFrames;
    /* Total bytes discarded (copied as zeros) */
    unsigned             zeroBytes;
    /* Bytes sice last sync WORD was found */
    unsigned             lastSyncBytes;
    /* Sync State Transitions */
    unsigned             lostSyncCount; /* Number of time Sync frame was lost, after locking-in once */
    /* CDB tables current buffer depth */
    unsigned             inputContextDepth; /* RAVE writes/SAGE reads */
    unsigned             outputContextDepth; /* SAGE writes/Audio Decoder reads */
}NEXUS_SageAudioStatus;

typedef struct NEXUS_SageAudio_P_Context *NEXUS_SageAudioHandle;

/*
 * Open/Close routines
 */
/*************************
 * Summary:              *
 * Default Open Settings *
 *************************/
typedef struct NEXUS_SageAudioOpenSettings
{
    /* any create-time settings required from nexus audio -> nexus sage go here */
    unsigned reserved;
} NEXUS_SageAudioOpenSettings;

/***************************************************
 * Summary:                                        *
 * Get Default SAGE Audio Processing Open Settings *
 ***************************************************/
void NEXUS_SageAudio_GetDefaultOpenSettings_priv(
    NEXUS_SageAudioOpenSettings *pSettings /* out */
    );

/****************************************************
 * Summary:                                         *
 * Open a SAGE Audio Processor                      *
 * Separate Open for each audio stream is required. *
 ****************************************************/
NEXUS_SageAudioHandle NEXUS_SageAudio_Open_priv(
    const NEXUS_SageAudioOpenSettings *pSettings /* Pass NULL for default settings */
    );

/************************************************
 * Summary:                                     *
 * Close SAGE Audio Processing                  *
 * It will release all the associated memory    *
 * - Handle allocated during Open               *
 * - Shared status memory allocated during Open *
 * Make sure to NULL the handle after this call *
 ************************************************/
void NEXUS_SageAudio_Close_priv(
    NEXUS_SageAudioHandle hSageAudio
    );

/*
 * Start/Stop
 */
/**************************
 * Summary:               *
 * Default Start Settings *
 **************************/
typedef struct
{
    NEXUS_AudioCodec codec;     /* Type of audio codec */
    bool             routingOnly; /* Bypass Monitoring */
    NEXUS_RaveHandle inContext; /* RAVE writes/SAGE reads */
    NEXUS_RaveHandle outContext; /* SAGE writes/Audio Decoder reads */
} NEXUS_SageAudioStartSettings;

/*******************************************************
 * Summary:                                            *
 * Get Default SAGE Audio Processing Starting Settings *
 *******************************************************/
void NEXUS_SageAudio_GetDefaultStartSettings_priv(
    NEXUS_SageAudioStartSettings *pSettings /* out */
    );

/******************************************************
 * Summary:                                           *
 * Start SAGE Audio processing for given audio stream *
 ******************************************************/
NEXUS_Error NEXUS_SageAudio_Start_priv(
    NEXUS_SageAudioHandle               hSageAudio,
    const NEXUS_SageAudioStartSettings *pSettings /* NULL for default settings */
    );

/*****************************************************
 * Summary:                                          *
 * Stop SAGE Audio processing for given audio stream *
 *****************************************************/
NEXUS_Error NEXUS_SageAudio_Stop_priv(
    NEXUS_SageAudioHandle hSageAudio
    );

/*
 * Get Status
 */
/********************************************************************************
 * Summary:                                                                     *
 * Get SAGE Audio Processor status for a given Audio Stream                     *
 * - It also includes Total number of streams being handled (opened) by SARM TA *
 ********************************************************************************/
NEXUS_Error NEXUS_SageAudio_GetStatus_priv(
    NEXUS_SageAudioHandle hSageAudio,
    NEXUS_SageAudioStatus *SageAudioStatus /* out */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SAGE_AUDIO_H__ */
