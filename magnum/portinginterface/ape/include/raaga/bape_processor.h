/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: Post Processor
*    Specific APIs related to Karaoke Vocal Path Processing
*
***************************************************************************/

#ifndef BAPE_PROCESSOR_H__
#define BAPE_PROCESSOR_H__

#include "bape.h"

/***************************************************************************
Summary:
Processor Handle
***************************************************************************/
typedef struct BAPE_Processor *BAPE_ProcessorHandle;

/***************************************************************************
Summary:
Fade Settings
***************************************************************************/
typedef struct BAPE_FadeSettings
{
    unsigned level;             /* Percentage representing the volume level.
                                   0 is muted, 100 is full volume. Default is 100. */
    unsigned duration;          /* duration in milliseconds it will take to change
                                   to a new level. Valid values are 3 - 1000 */
    unsigned type;              /* specifies the type of fade -
                                   0- Linear (Default), 1-Cubic-In, 2-Cubic-Out. */
} BAPE_FadeSettings;

/***************************************************************************
Summary:
KaraokeVocal Settings
***************************************************************************/
typedef struct BAPE_KaraokeVocalSettings
{
    /* Karaoke Vocal Echo Effect */
    struct
    {
        bool enabled;               /* Enable Echo effect. Default is false. */
        unsigned attenuation;       /* Percentage of attenuation of Echo Effect, specified in Q31 format.
                                       Valid Values 0-100. Default is 20. */
        unsigned delay;             /* Delay of Echo effect, specified in milliseconds.
                                       Valid values are 0-200. Default is 200 */
    } echo;
} BAPE_KaraokeVocalSettings;

/***************************************************************************
Summary:
Processor Settings
***************************************************************************/
typedef struct BAPE_ProcessorSettings
{
    union
    {
        BAPE_FadeSettings fade;
        BAPE_KaraokeVocalSettings karaokeVocal;
    } settings;
} BAPE_ProcessorSettings;

/***************************************************************************
Summary:
Fade Status
***************************************************************************/
typedef struct BAPE_FadeStatus
{
    /* Fade Effect Status */
    bool active;
    unsigned remaining;         /* MilliSeconds remaining in the current active fade */
    unsigned level;             /* Percentage representing the current volume level.
                                   0 is muted, 100 is full volume. Default is 100. */
} BAPE_FadeStatus;

/***************************************************************************
Summary:
Processor Status
***************************************************************************/
typedef struct BAPE_ProcessorStatus
{
    bool valid;
    union
    {
        BAPE_FadeStatus fade;
    } status;
} BAPE_ProcessorStatus;

/***************************************************************************
Summary:
Processor Settings
***************************************************************************/
typedef struct BAPE_ProcessorCreateSettings
{
    BAPE_PostProcessorType type;
} BAPE_ProcessorCreateSettings;

/***************************************************************************
Summary:
    Get default settings for a Processor stage
***************************************************************************/
void BAPE_Processor_GetDefaultCreateSettings(
    BAPE_ProcessorCreateSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a Processor stage
***************************************************************************/
BERR_Code BAPE_Processor_Create(
    BAPE_Handle deviceHandle,
    const BAPE_ProcessorCreateSettings *pSettings,
    BAPE_ProcessorHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
    Close a Processor stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_Processor_Destroy(
    BAPE_ProcessorHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a Processor stage
***************************************************************************/
void BAPE_Processor_GetSettings(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a Processor stage
***************************************************************************/
BERR_Code BAPE_Processor_SetSettings(
    BAPE_ProcessorHandle handle,
    const BAPE_ProcessorSettings *pSettings
    );

/***************************************************************************
Summary:
    Get Status for a Processor stage
***************************************************************************/
void BAPE_Processor_GetStatus(
    BAPE_ProcessorHandle handle,
    BAPE_ProcessorStatus *pStatus    /* [out] Status */
    );

/***************************************************************************
Summary:
    Get the audio connector for a Processor stage
***************************************************************************/
void BAPE_Processor_GetConnector(
    BAPE_ProcessorHandle handle,
    BAPE_Connector *pConnector       /* [out] */
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_Processor_AddInput(
    BAPE_ProcessorHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_Processor_RemoveInput(
    BAPE_ProcessorHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_Processor_RemoveAllInputs(
    BAPE_ProcessorHandle handle
    );

#endif /* #ifndef BAPE_PROCESSOR_H__ */
