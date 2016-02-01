/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
*   API name: Effects
*    Specific APIs related to Karaoke Vocal Path Processing
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#ifndef BAPE_EFFECTS_H__
#define BAPE_EFFECTS_H__

#include "bape.h"

/***************************************************************************
Summary:
Effects Handle
***************************************************************************/
typedef struct BAPE_Effects *BAPE_EffectsHandle;

/***************************************************************************
Summary:
Effects Settings
***************************************************************************/
typedef struct BAPE_EffectsSettings
{
    /* Fade Effect */
    struct
    {
        unsigned level;             /* Percentage representing the volume level.
                                       0 is muted, 100 is full volume. Default is 100. */
        unsigned duration;          /* duration in milliseconds it will take to change
                                       to a new level. Valid values are 3 - 1000 */
        unsigned type;              /* specifies the type of fade -
                                       0- Linear, 1-Quad, 2-cubic, 3-Quart. Default is 2 (cubic)*/
    } fade;
} BAPE_EffectsSettings;

/***************************************************************************
Summary:
    Get default settings for a Effects stage
***************************************************************************/
void BAPE_Effects_GetDefaultSettings(
    BAPE_EffectsSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a Effects stage
***************************************************************************/
BERR_Code BAPE_Effects_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EffectsSettings *pSettings,
    BAPE_EffectsHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
    Close a Effects stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_Effects_Destroy(
    BAPE_EffectsHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a Effects stage
***************************************************************************/
void BAPE_Effects_GetSettings(
    BAPE_EffectsHandle handle,
    BAPE_EffectsSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a Effects stage
***************************************************************************/
BERR_Code BAPE_Effects_SetSettings(
    BAPE_EffectsHandle handle,
    const BAPE_EffectsSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for a Effects stage
***************************************************************************/
void BAPE_Effects_GetConnector(
    BAPE_EffectsHandle handle,
    BAPE_Connector *pConnector       /* [out] */
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_Effects_AddInput(
    BAPE_EffectsHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_Effects_RemoveInput(
    BAPE_EffectsHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_Effects_RemoveAllInputs(
    BAPE_EffectsHandle handle
    );

#endif /* #ifndef BAPE_EFFECTS_H__ */
