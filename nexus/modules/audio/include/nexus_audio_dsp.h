/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: AudioDsp
*    Specific APIs related to audio DSP control and debug.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_DSP_H__
#define NEXUS_AUDIO_DSP_H__

/*=************************************
Interface: AudioDsp

Header file: nexus_audio_dsp.h

Module: Audio

Description: Audio DSP Control and debug

**************************************/

#include "nexus_types.h"

/***************************************************************************
Summary:
Types of audio DSP debug information
***************************************************************************/
typedef enum NEXUS_AudioDspDebugType
{
    NEXUS_AudioDspDebugType_eDramMessage,
    NEXUS_AudioDspDebugType_eUartMessage,
    NEXUS_AudioDspDebugType_eCoreDump,
    NEXUS_AudioDspDebugType_eMax
} NEXUS_AudioDspDebugType;

/***************************************************************************
Summary:
Audio DSP Debug Settings 
 
Description: 
Debug-related settings for the audio DSP. 
 
See Also: 
NEXUS_AudioModuleInitSettings
***************************************************************************/
typedef struct NEXUS_AudioDspDebugSettings
{
    struct
    {
        bool enabled;
        unsigned bufferSize;        /* If enabled = true, this specifies the debug buffer size in bytes.  
                                       Only required if you wish to override the default debug settings. */
    } typeSettings[NEXUS_AudioDspDebugType_eMax];
} NEXUS_AudioDspDebugSettings;

/***************************************************************************
Summary:
Get DSP Firmware Debug Data
***************************************************************************/
NEXUS_Error NEXUS_AudioDsp_GetDebugBuffer(
    unsigned dspIndex,      /* Ranges from 0 to NEXUS_NUM_AUDIO_DSP-1 */
    NEXUS_AudioDspDebugType debugType,
    const void **pBuffer,   /* [out] attr{memory=cached} Pointer to debug data */
    size_t *pBufferSize     /* [out] size of buffer in bytes */
    );

/***************************************************************************
Summary:
Consume DSP Firmware Debug Data.
***************************************************************************/
NEXUS_Error NEXUS_AudioDsp_DebugReadComplete(
    unsigned dspIndex,      /* Ranges from 0 to NEXUS_NUM_AUDIO_DSP-1 */
    NEXUS_AudioDspDebugType debugType,
    size_t bytesRead        /* Number of bytes consumed from the debug buffer */
    );

/***************************************************************************
Summary:
Categories of DSP Processing Algorithm Types.
***************************************************************************/
typedef enum NEXUS_AudioDspAlgorithmType
{
    NEXUS_AudioDspAlgorithmType_eAudioDecode,
    NEXUS_AudioDspAlgorithmType_eAudioPassthrough,
    NEXUS_AudioDspAlgorithmType_eAudioEncode,
    NEXUS_AudioDspAlgorithmType_eAudioMixer,
    NEXUS_AudioDspAlgorithmType_eAudioEchoCanceller,
    NEXUS_AudioDspAlgorithmType_eAudioProcessing,
    NEXUS_AudioDspAlgorithmType_eVideoDecode,
    NEXUS_AudioDspAlgorithmType_eVideoEncode,
    NEXUS_AudioDspAlgorithmType_eMax
} NEXUS_AudioDspAlgorithmType;

/***************************************************************************
Summary:
Audio DSP Algo settings

Description:
Application platform control to specify init time algorithm settings.

See Also:
NEXUS_AudioModuleInitSettings
***************************************************************************/
typedef struct NEXUS_AudioDspAlgorithmSettings
{
    struct
    {
        unsigned count;     /* number of DISTINCT algorithms supported */
    } typeSettings[NEXUS_AudioDspAlgorithmType_eMax];
} NEXUS_AudioDspAlgorithmSettings;

#endif /* #ifndef NEXUS_AUDIO_DSP_H__ */

