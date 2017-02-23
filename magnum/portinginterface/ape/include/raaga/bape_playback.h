/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Audio Playback Interface
 *
 ***************************************************************************/

#ifndef BAPE_PLAYBACK_H_
#define BAPE_PLAYBACK_H_

/***************************************************************************
Summary:
Playback Handle
***************************************************************************/
typedef struct BAPE_Playback *BAPE_PlaybackHandle;

/***************************************************************************
Summary:
Playback Open settings
***************************************************************************/
typedef struct BAPE_PlaybackOpenSettings
{
    BMMA_Heap_Handle heap;          /* Heap used for allocating buffers.  If not set, the heap
                                       provided in BAPE_Open will be used. */
    unsigned numBuffers;            /* Number of buffers to allocate.  For mono or interleaved stereo,
                                       one buffer is required.  For non-interleaved 7.1 data 8 buffers
                                       are required.  For interleaved 7.1 data, 4 buffers are required. */
    unsigned bufferSize;            /* Buffer size in bytes, must be a multiple of 256.  If more than one
                                       buffer is allocated, all buffers will be this size.  If this size
                                       is 0, buffers will be allocated from the internal buffer pool. */
    unsigned watermarkThreshold;    /* FIFO interrupt threshold in bytes.  
                                       When an amuont <= this threshold is available, 
                                       an interrupt will be raised. This value should
                                       be a multiple of 256. */    
} BAPE_PlaybackOpenSettings;

/***************************************************************************
Summary:
Get Default Playback Open Settings
***************************************************************************/
void BAPE_Playback_GetDefaultOpenSettings(
    BAPE_PlaybackOpenSettings *pSettings
    );

/***************************************************************************
Summary:
Open a playback channel
***************************************************************************/
BERR_Code BAPE_Playback_Open(
    BAPE_Handle hApe,
    unsigned index,
    const BAPE_PlaybackOpenSettings *pSettings,
    BAPE_PlaybackHandle *pHandle                    /* [out] */
    );

/***************************************************************************
Summary:
Close a playback channel
***************************************************************************/
void BAPE_Playback_Close(
    BAPE_PlaybackHandle hPlayback
    );

/***************************************************************************
Summary:
Audio playback settings
***************************************************************************/
typedef struct BAPE_PlaybackSettings
{
    BAPE_MultichannelFormat multichannelFormat;     /* Controls whether the playback channel outputs 2.0, 5.1, or 7.1 data.
                                                       This is not changeable on the fly. */
    bool compressedData;                            /* If true, compressed data will be fed in IEC61937 format.  
                                                       This is not changeable on the fly.  Compressed data should be
                                                       fed as 16-bit stereo interleaved data. */
    unsigned sampleRate;                            /* Sample Rate in Hz.  This is only used if BAPE_PlaybackStartSettings.sampleRate is 
                                                       set to 0 and allows changing of the sampleRate on the fly to permit pitch-shifting, 
                                                       or similar effects. */
} BAPE_PlaybackSettings;

/***************************************************************************
Summary:
Get current audio playback settings 
***************************************************************************/
void BAPE_Playback_GetSettings(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set current audio playback settings 
***************************************************************************/
BERR_Code BAPE_Playback_SetSettings(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackSettings *pSettings
    );

/***************************************************************************
Summary:
Playback Start Settings
***************************************************************************/
typedef struct BAPE_PlaybackStartSettings
{
    unsigned sampleRate;                    /* Sample Rate in Hz.  If set to 0, the value from BAPE_PlaybackSettings.sampleRate will be used instead. */
    unsigned bitsPerSample;                 /* Supports 8, 16, or 32 */
    bool isStereo;                          /* If true, data will be treated as stereo.  If false, data will be treated as mono. */
    bool isSigned;                          /* If true, data will be treated as signed.  If false, data will be treated as unsigned. */
    bool reverseEndian;                     /* If true, data will be endian-swapped in hardware.  Otherwise, data should match host endian mode. */
    bool loopEnabled;                       /* If true, data will loop continuously without requiring host intervention. */
    bool interleaved;                       /* If true, data for a channel pair is interleaved into a single buffer */
    unsigned startThreshold;                /* Size in bytes that must be committed to the hardware before data will flow into the mixer.  
                                               Must be a multiple of 256. */    
} BAPE_PlaybackStartSettings;

/***************************************************************************
Summary:
Get Default Playback Start Settings
***************************************************************************/
void BAPE_Playback_GetDefaultStartSettings(
    BAPE_PlaybackStartSettings *pSettings       /* [out] */
    );

/***************************************************************************
Summary:
Start Playback
***************************************************************************/
BERR_Code BAPE_Playback_Start(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop Playback
***************************************************************************/
void BAPE_Playback_Stop(
    BAPE_PlaybackHandle hPlayback
    );


/***************************************************************************
Summary:
Resume Playback
***************************************************************************/
BERR_Code BAPE_Playback_Resume(
    BAPE_PlaybackHandle hPlayback
    );

/***************************************************************************
Summary:
Suspend Playback
***************************************************************************/
void BAPE_Playback_Suspend(
    BAPE_PlaybackHandle hPlayback
    );

/***************************************************************************
Summary:
Flush Playback Buffer
***************************************************************************/
void BAPE_Playback_Flush(
    BAPE_PlaybackHandle hPlayback
    );

/***************************************************************************
Summary:
Get Playback Buffer 
 
Description: 
This routine will return the next contiguous buffer address and size.  If 
interrupts are enabled, the caller should call this routine and commit data 
until the size returned from this function is zero. 
***************************************************************************/
BERR_Code BAPE_Playback_GetBuffer(
    BAPE_PlaybackHandle hPlayback,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    );

/***************************************************************************
Summary:
Commit Data to the playback hardware
***************************************************************************/
BERR_Code BAPE_Playback_CommitData(
    BAPE_PlaybackHandle hPlayback,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    );

/***************************************************************************
Summary:
Playback Status
***************************************************************************/
typedef struct BAPE_PlaybackStatus
{
    unsigned queuedBytes;
    unsigned fifoSize;
} BAPE_PlaybackStatus;

/***************************************************************************
Summary:
Get Playback Status
***************************************************************************/
void BAPE_Playback_GetStatus(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Get Audio Source Connector for output data
***************************************************************************/
void BAPE_Playback_GetConnector(
    BAPE_PlaybackHandle hPlayback,
    BAPE_Connector *pConnector /* [out] */
    );

/***************************************************************************
Summary:
Playback Interrupt Handlers
***************************************************************************/
typedef struct BAPE_PlaybackInterruptHandlers
{
    struct
    {
        void (*pCallback_isr)(void *pParam1, int param2);
        void *pParam1;
        int param2;
    } watermark;
} BAPE_PlaybackInterruptHandlers;

/***************************************************************************
Summary:
Get Playback Interrupt Handlers
***************************************************************************/
void BAPE_Playback_GetInterruptHandlers(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackInterruptHandlers *pInterrupts     /* [out] */
    );

/***************************************************************************
Summary:
Set Playback Interrupt Handlers
***************************************************************************/
BERR_Code BAPE_Playback_SetInterruptHandlers(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackInterruptHandlers *pInterrupts
    );

#endif /* #ifndef BAPE_PLAYBACK_H_ */
