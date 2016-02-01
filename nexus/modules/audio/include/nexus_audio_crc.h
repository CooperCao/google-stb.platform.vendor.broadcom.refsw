/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: AudioCrc
*    CRC capture for Audio data
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef NEXUS_AUDIO_CRC_H__
#define NEXUS_AUDIO_CRC_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: Audio CRC

Header file: nexus_audio_crc.h

Module: Audio

Description: Broadcom Audio CRC capture interface

**************************************/

#define NEXUS_AudioCrcNumChannelPairs_Max       ((NEXUS_AudioChannel_eMax+1)/2)

/**
Summary:
Handle Audio CRC interface
**/
typedef struct NEXUS_AudioCrc* NEXUS_AudioCrcHandle;

/***************************************************************************
Summary:
Audio CRC Source Type 
 
Description: 
Specifies what stage to capture the CRC data
***************************************************************************/
typedef enum NEXUS_AudioCrcSourceType
{
    NEXUS_AudioCrcSourceType_ePlaybackBuffer, /* Capture data entering the FMM from a BAPE_MixerInput */
    NEXUS_AudioCrcSourceType_eOutputPort,     /* Capture CRC data for Audio entering a BAPE_OutputPort */
    NEXUS_AudioCrcSourceType_eMax
} NEXUS_AudioCrcSourceType;
 
/***************************************************************************
Summary:
Audio CRC Mode 
 
Description: 
Specifies the behavior of the CRC capture
***************************************************************************/
typedef enum NEXUS_AudioCrcMode
{
    NEXUS_AudioCrcMode_eFreeRun,
    NEXUS_AudioCrcMode_eSingle,
    NEXUS_AudioCrcMode_eMax
} NEXUS_AudioCrcMode;
 
/***************************************************************************
Summary:
CRC Open Settings
***************************************************************************/
typedef struct NEXUS_AudioCrcOpenSettings
{
    unsigned numChannelPairs;               /* Set to 1 for stereo or compressed capture, 3 for 5.1 or 4 for 7.1 */
    unsigned numEntries;                    /* Number of NEXUS_AudioCrcEntry to buffer */
    unsigned samplingPeriod;                /* Number of samples to accumulate per CRC */
    unsigned dataWidth;                     /* Supported values are 16, 20, or 24 bits per sample */
    NEXUS_AudioCrcMode captureMode;         /* Freerun or single capture. */
    uint16_t initialValue;                  /* Initial value.  0x0000 and 0xffff are currently supported.
                                               reset to this value during each start */
} NEXUS_AudioCrcOpenSettings;

/***************************************************************************
Summary:
CRC Input Settings
***************************************************************************/
typedef struct NEXUS_AudioCrcInputSettings
{
    NEXUS_AudioCrcSourceType sourceType;
    NEXUS_AudioOutputHandle output;         /* Must be set for all sourceTypes */
    NEXUS_AudioInput input;                 /* Only Set if sourceType == NEXUS_AudioCrcSourceType_ePlaybackBuffer */
} NEXUS_AudioCrcInputSettings;

/***************************************************************************
Summary:
    Get Default CRC Open Settings

Description:
***************************************************************************/
void NEXUS_AudioCrc_GetDefaultOpenSettings(
    NEXUS_AudioCrcOpenSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open Audio CRC 

Description:
    Open an Audio CRC.  Audio chain must be stopped to 
    perform this operation.
***************************************************************************/
NEXUS_AudioCrcHandle NEXUS_AudioCrc_Open(
    unsigned index,                                 /* index to CRC instance */
    const NEXUS_AudioCrcOpenSettings *pSettings     /* attr{null_allowed=y} Pass NULL for default settings */
    );

/***************************************************************************
Summary:
    Close Audio CRC 

Description:
    Close this CRC.  Audio chain must be stopped, and CRC input must be 
    removed to perform this operation.
***************************************************************************/
void NEXUS_AudioCrc_Close(
    NEXUS_AudioCrcHandle handle
    );

/***************************************************************************
Summary:
    Get Default CRC Input Settings

Description:
***************************************************************************/
void NEXUS_AudioCrc_GetDefaultInputSettings(
    NEXUS_AudioCrcInputSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Set the input for this CRC 

Description:
    Sets the input for this CRC.  Audio chain must be stopped to 
    perform this operation.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_SetInput(
    NEXUS_AudioCrcHandle handle,
    const NEXUS_AudioCrcInputSettings * pSettings
    );

/***************************************************************************
Summary:
    Clear the input from this CRC 

Description:
    Removes the input from this CRC.  Audio chain must be stopped to 
    perform this operation.
***************************************************************************/
void NEXUS_AudioCrc_ClearInput(
    NEXUS_AudioCrcHandle handle
    );

/***************************************************************************
Summary:
Audio CRC Entry
***************************************************************************/
typedef struct NEXUS_AudioCrcEntry
{
    uint16_t seqNumber;
    uint16_t value;
} NEXUS_AudioCrcEntry;

/***************************************************************************
Summary:
Audio CRC Data
***************************************************************************/
typedef struct NEXUS_AudioCrcData
{
    NEXUS_AudioCrcEntry crc0;
    NEXUS_AudioCrcEntry crc1;
    NEXUS_AudioCrcEntry crc2;
    NEXUS_AudioCrcEntry crc3;
} NEXUS_AudioCrcData;

/***************************************************************************
Summary:
Get Crc Data.

Description:
    Get CRC entries. numEntries will always be populated ( >= 0 ), 
    even if CRC input is not connected. An error will also be
    returned if there is no active CRC input.
***************************************************************************/
NEXUS_Error NEXUS_AudioCrc_GetCrcData(
    NEXUS_AudioCrcHandle handle,
    NEXUS_AudioCrcData *pData, /* [out] attr{nelem=numEntries;nelem_out=pNumReturned} pointer to CRC entries. */
    unsigned numEntries,
    unsigned *pNumReturned
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_CRC_H__ */


