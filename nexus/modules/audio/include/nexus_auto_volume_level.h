/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
* API Description:
*   API name: AutoVolumeLevel
*    Specific APIs related to automatic volume leveling
*
***************************************************************************/

#ifndef NEXUS_AUTO_VOLUME_LEVEL_H__
#define NEXUS_AUTO_VOLUME_LEVEL_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_processing_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AutoVolumeLevel

Header file: nexus_audio_volume_level.h

Module: Audio

Description:

**************************************/

/**
Summary:
Handle for auto volume leveling stage
**/
typedef struct NEXUS_AutoVolumeLevel *NEXUS_AutoVolumeLevelHandle;

/***************************************************************************
Summary:
Get default settings for an auto volume leveling stage
***************************************************************************/
void NEXUS_AutoVolumeLevel_GetDefaultSettings(
    NEXUS_AutoVolumeLevelSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
Open an auto volume leveling stage
***************************************************************************/
NEXUS_AutoVolumeLevelHandle NEXUS_AutoVolumeLevel_Open( /* attr{destructor=NEXUS_AutoVolumeLevel_Close}  */
    const NEXUS_AutoVolumeLevelSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an auto volume leveling stage

Description:
Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_AutoVolumeLevel_Close(
    NEXUS_AutoVolumeLevelHandle handle
    );

/***************************************************************************
Summary:
Get Settings for an auto volume leveling stage
***************************************************************************/
void NEXUS_AutoVolumeLevel_GetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AutoVolumeLevelSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
Set Settings for an auto volume leveling stage
***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_SetSettings(
    NEXUS_AutoVolumeLevelHandle handle,
    const NEXUS_AutoVolumeLevelSettings *pSettings
    );

/***************************************************************************
Summary:
Get the audio connector for an auto volume leveling stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_AutoVolumeLevel_GetConnector(
    NEXUS_AutoVolumeLevelHandle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_AddInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_RemoveInput(
    NEXUS_AutoVolumeLevelHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_AutoVolumeLevel_RemoveAllInputs(
    NEXUS_AutoVolumeLevelHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUTO_VOLUME_LEVEL_H__ */
