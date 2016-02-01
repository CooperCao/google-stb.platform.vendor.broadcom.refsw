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
*   API name: 3dSurround
*    Specific APIs related to Broadcom 3D Surround Processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BAPE_3D_SURROUND_H__
#define BAPE_3D_SURROUND_H__

#include "bape.h"

/***************************************************************************
Summary: 
3dSurround Handle 
***************************************************************************/
typedef struct BAPE_3dSurround *BAPE_3dSurroundHandle;

/***************************************************************************
Summary:
3dSurround Virtual Speaker Positions 
 
Description: 
Each speaker position value determines a horizontal (H) and vertical (V) 
speaker offset from the listener's perspective.  These are express in degrees. 
Speakers are described as Front, Surround, and Center. 
***************************************************************************/
typedef enum BAPE_3dSurroundSpeakerPosition
{
    BAPE_3dSurroundSpeakerPosition_eCenter,     /* Front H:30 V:15 Surround H:90 V:0 Center: H:0 V:15 */
    BAPE_3dSurroundSpeakerPosition_eWide,       /* Front H:30 V:15 Surround H:105 V:30 Center: H:0 V:15 */ 
    BAPE_3dSurroundSpeakerPosition_eExtraWide,  /* Front H:60 V:30 Surround H:120 V:45 Center: H:0 V:30 */ 
    BAPE_3dSurroundSpeakerPosition_eMax
} BAPE_3dSurroundSpeakerPosition;

/***************************************************************************
Summary:
3dSurround Settings
***************************************************************************/
typedef struct BAPE_3dSurroundSettings
{
    bool enabled;               /* If true, processing is enabled.  Otherwise this stage is bypassed */
    bool softLimiterEnabled;    /* This feature has been deprecated.  Provided for backward compatibility only. */

    BAPE_3dSurroundSpeakerPosition speakerPosition; /* Virtual speaker position */
} BAPE_3dSurroundSettings;

/***************************************************************************
Summary:
    Get default settings for an 3dSurround stage
***************************************************************************/
void BAPE_3dSurround_GetDefaultSettings(
    BAPE_3dSurroundSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an 3dSurround stage
***************************************************************************/
BERR_Code BAPE_3dSurround_Create(
    BAPE_Handle deviceHandle,
    const BAPE_3dSurroundSettings *pSettings,
    BAPE_3dSurroundHandle *pHandle
    );

/***************************************************************************
Summary:
    Close an 3dSurround stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_3dSurround_Destroy(
    BAPE_3dSurroundHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an 3dSurround stage
***************************************************************************/
void BAPE_3dSurround_GetSettings(
    BAPE_3dSurroundHandle handle,
    BAPE_3dSurroundSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an 3dSurround stage
***************************************************************************/
BERR_Code BAPE_3dSurround_SetSettings(
    BAPE_3dSurroundHandle handle,
    const BAPE_3dSurroundSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an 3dSurround stage
***************************************************************************/
void BAPE_3dSurround_GetConnector(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector *pConnector
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_3dSurround_AddInput(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_3dSurround_RemoveInput(
    BAPE_3dSurroundHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_3dSurround_RemoveAllInputs(
    BAPE_3dSurroundHandle handle
    );

#endif /* #ifndef BAPE_3D_SURROUND_H__ */

