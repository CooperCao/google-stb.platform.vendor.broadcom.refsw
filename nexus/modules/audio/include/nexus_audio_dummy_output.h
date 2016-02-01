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
*   API name: AudioDummyOutput
*    Specific APIs related to audio dummy outputs.  Dummy outputs
*    are used to consume audio data without outputting to a physical
*    output such as I2S or DAC.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_DUMMY_OUTPUT_H__
#define NEXUS_AUDIO_DUMMY_OUTPUT_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioDummyOutput

Header file: nexus_audio_dummy_output.h

Module: Audio

Description: A hardware dummy sink output that 
             can consume audio data without
             rendering to a physical output
             such as DACs or I2S.

**************************************/

/**
Summary:
Handle for an audio dummy output
**/
typedef struct NEXUS_AudioDummyOutput *NEXUS_AudioDummyOutputHandle;

/***************************************************************************
Summary:
Audio Dummy Output Settings
***************************************************************************/
typedef struct NEXUS_AudioDummyOutputSettings
{
    NEXUS_AudioMultichannelFormat maxMultichannelFormat;    /* Maximum data width to be consumed.  This can only be set prior to open. */
} NEXUS_AudioDummyOutputSettings;

/***************************************************************************
Summary:
Get default settings for an Audio Dummy Output
***************************************************************************/
void NEXUS_AudioDummyOutput_GetDefaultSettings(
    NEXUS_AudioDummyOutputSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
Open an Audio Dummy Output
***************************************************************************/
NEXUS_AudioDummyOutputHandle NEXUS_AudioDummyOutput_Open( /* attr{destructor=NEXUS_AudioDummyOutput_Close}  */
    unsigned index,
    const NEXUS_AudioDummyOutputSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an Audio Dummy Output
    
Description:
Inputs must be removed prior to closing.
***************************************************************************/
void NEXUS_AudioDummyOutput_Close(
    NEXUS_AudioDummyOutputHandle handle
    );

/***************************************************************************
Summary:
Get Settings for an Audio Dummy Output
***************************************************************************/
void NEXUS_AudioDummyOutput_GetSettings(
    NEXUS_AudioDummyOutputHandle handle,
    NEXUS_AudioDummyOutputSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
Set Settings for an Audio Dummy Output
***************************************************************************/
NEXUS_Error NEXUS_AudioDummyOutput_SetSettings(
    NEXUS_AudioDummyOutputHandle handle,
    const NEXUS_AudioDummyOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Get the audio connector for an Audio Dummy Output
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioDummyOutput_GetConnector(
    NEXUS_AudioDummyOutputHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_DUMMY_OUTPUT_H__ */


