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
*   API name: 3dSurround
*    Specific APIs related to Broadcom 3D Surround Processing
*
***************************************************************************/

#ifndef NEXUS_3D_SURROUND_H__
#define NEXUS_3D_SURROUND_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: 3dSurround

Header file: nexus_3d_surround.h

Module: Audio

Description: Broadcom 3D Surround audio processing stage

**************************************/

/**
Summary:
Handle for 3D Surround stage
**/
typedef struct NEXUS_3dSurround *NEXUS_3dSurroundHandle;

/***************************************************************************
Summary:
3dSurround Virtual Speaker Positions 
 
Description: 
Each speaker position value determines a horizontal (H) and vertical (V) 
speaker offset from the listener's perspective.  These are express in degrees. 
Speakers are described as Front, Surround, and Center. 
***************************************************************************/
typedef enum NEXUS_3dSurroundSpeakerPosition
{
    NEXUS_3dSurroundSpeakerPosition_eCenter,     /* Front H:30 V:15 Surround H:90 V:0 Center: H:0 V:15 */
    NEXUS_3dSurroundSpeakerPosition_eWide,       /* Front H:30 V:15 Surround H:105 V:30 Center: H:0 V:15 */ 
    NEXUS_3dSurroundSpeakerPosition_eExtraWide,  /* Front H:60 V:30 Surround H:120 V:45 Center: H:0 V:30 */ 
    NEXUS_3dSurroundSpeakerPosition_eMax
} NEXUS_3dSurroundSpeakerPosition;

/***************************************************************************
Summary:
3D Surround Settings
***************************************************************************/
typedef struct NEXUS_3dSurroundSettings
{
    bool enabled;                /* If true, processing is enabled.  Otherwise this stage is bypassed */

    bool softLimiterEnabled;     /* Turns the soft limiter function on or off. */

    NEXUS_3dSurroundSpeakerPosition speakerPosition; /* Virtual speaker position */
} NEXUS_3dSurroundSettings;

/***************************************************************************
Summary:
    Get default settings for a 3D Surround stage
***************************************************************************/
void NEXUS_3dSurround_GetDefaultSettings(
    NEXUS_3dSurroundSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a 3D Surround stage
***************************************************************************/
NEXUS_3dSurroundHandle NEXUS_3dSurround_Open( /* attr{destructor=NEXUS_3dSurround_Close}  */
    const NEXUS_3dSurroundSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close a 3D Surround stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_3dSurround_Close(
    NEXUS_3dSurroundHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a 3D Surround stage
***************************************************************************/
void NEXUS_3dSurround_GetSettings(
    NEXUS_3dSurroundHandle handle,
    NEXUS_3dSurroundSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a 3D Surround stage
***************************************************************************/
NEXUS_Error NEXUS_3dSurround_SetSettings(
    NEXUS_3dSurroundHandle handle,
    const NEXUS_3dSurroundSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for a 3D Surround stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_3dSurround_GetConnector(
    NEXUS_3dSurroundHandle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_3dSurround_AddInput(
    NEXUS_3dSurroundHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_3dSurround_RemoveInput(
    NEXUS_3dSurroundHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_3dSurround_RemoveAllInputs(
    NEXUS_3dSurroundHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_3D_SURROUND_H__ */
