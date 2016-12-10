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
*   API name: CustomProcessing
*    Specific APIs related to custom audio post-processing
*
***************************************************************************/

#ifndef BAPE_CUSTOM_PROCESSING_H__
#define BAPE_CUSTOM_PROCESSING_H__

#include "bape.h"
#if !B_REFSW_MINIMAL
/***************************************************************************
Summary: 
CustomProcessing Handle 
***************************************************************************/
typedef struct BAPE_CustomProcessing *BAPE_CustomProcessingHandle;

/***************************************************************************
Summary:
CustomProcessing Settings
***************************************************************************/
typedef struct BAPE_CustomProcessingSettings
{
    BDSP_Algorithm algorithm;               /* Processing algorithm to be used.  Refer to BDSP for a list of algorithms. */
    size_t algorithmSettingsSize;           /* Size of the settings structure in bytes */
    BAPE_MultichannelFormat outputFormat;   /* Output PCM format of this processor */
} BAPE_CustomProcessingSettings;

/***************************************************************************
Summary:
    Get default settings for a CustomProcessing stage
***************************************************************************/
void BAPE_CustomProcessing_GetDefaultSettings(
    BAPE_CustomProcessingSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open a CustomProcessing stage
***************************************************************************/
BERR_Code BAPE_CustomProcessing_Create(
    BAPE_Handle deviceHandle,
    const BAPE_CustomProcessingSettings *pSettings,
    BAPE_CustomProcessingHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
    Close a CustomProcessing stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_CustomProcessing_Destroy(
    BAPE_CustomProcessingHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a CustomProcessing stage
 
Description: 
    This routine will retrieve the algorithm-independent settings.
 
See Also:
    BAPE_CustomProcessing_GetSettings
    BAPE_CustomProcessing_GetAlgorithmSettings
    BAPE_CustomProcessing_SetAlgorithmSettings
***************************************************************************/
void BAPE_CustomProcessing_GetSettings(
    BAPE_CustomProcessingHandle handle,
    BAPE_CustomProcessingSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for a CustomProcessing stage
 
Description: 
    This routine will update the algorithm-independent settings.
 
See Also:
    BAPE_CustomProcessing_GetSettings
    BAPE_CustomProcessing_GetAlgorithmSettings
    BAPE_CustomProcessing_SetAlgorithmSettings
***************************************************************************/
BERR_Code BAPE_CustomProcessing_SetSettings(
    BAPE_CustomProcessingHandle handle,
    const BAPE_CustomProcessingSettings *pSettings
    );

/***************************************************************************
Summary:
    Get Algorithm Settings for a CustomProcessing stage
 
Description: 
    This routine will update the algorithm-specific settings.
    The size of the passed structure must not exceed
    BAPE_CustomProcessingSettings.algorithmSettingsSize
 
See Also:
    BAPE_CustomProcessing_GetSettings
    BAPE_CustomProcessing_SetSettings
    BAPE_CustomProcessing_SetAlgorithmSettings
***************************************************************************/
void BAPE_CustomProcessing_GetAlgorithmSettings(
    BAPE_CustomProcessingHandle handle,
    void *pSettings,        /* [out] Should be defined as the correct data type for this custom algorithm */
    size_t settingsSize     /* Size of the settings structure in bytes */
    );

/***************************************************************************
Summary:
    Set Settings for a CustomProcessing stage
 
Description: 
    This routine will update the algorithm-specific settings.
    The size of the passed structure must not exceed
    BAPE_CustomProcessingSettings.algorithmSettingsSize
 
See Also:
    BAPE_CustomProcessing_GetSettings
    BAPE_CustomProcessing_SetSettings
    BAPE_CustomProcessing_GetAlgorithmSettings
***************************************************************************/
BERR_Code BAPE_CustomProcessing_SetAlgorithmSettings(
    BAPE_CustomProcessingHandle handle,
    const void *pSettings,  /* Should be defined as the correct data type for this custom algorithm */
    size_t settingsSize     /* Size of the settings structure in bytes */
    );

/***************************************************************************
Summary:
    Get the audio connector for a CustomProcessing stage
***************************************************************************/
void BAPE_CustomProcessing_GetConnector(
    BAPE_CustomProcessingHandle handle,
    BAPE_Connector *pConnector
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_CustomProcessing_AddInput(
    BAPE_CustomProcessingHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_CustomProcessing_RemoveInput(
    BAPE_CustomProcessingHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_CustomProcessing_RemoveAllInputs(
    BAPE_CustomProcessingHandle handle
    );

/***************************************************************************
Summary:
Get Status for a custom processing algorithm
***************************************************************************/
BERR_Code BAPE_CustomProcessing_GetAlgorithmStatus(
    BAPE_CustomProcessingHandle handle,
    void *pStatus,      /* [out] Should be defined as the correct data type for this custom algorithm */
    size_t statusSize   /* Size of the status structure in bytes */
    );

#endif
#endif /* #ifndef BAPE_CUSTOM_PROCESSING_H__ */
