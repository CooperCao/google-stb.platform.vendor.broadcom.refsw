/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: DolbyDigitalReencode
*    Specific APIs related to Dolby Digital Reencoding used in Dolby MS11
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_DOLBY_DIGITAL_REENCODE_H__
#define NEXUS_DOLBY_DIGITAL_REENCODE_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_ac3_encode.h"
#include "nexus_audio_decoder_types.h"
#include "nexus_audio_processing_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: DolbyDigitalReencode

Header file: nexus_dolby_digital_reencode.h

Module: Audio

Description: Dolby Digital Reencode for MS11

**************************************/

/**
Summary:
Handle for Dolby Digital Reencoder stage
**/
typedef struct NEXUS_DolbyDigitalReencode *NEXUS_DolbyDigitalReencodeHandle;

/***************************************************************************
Summary:
    Get default settings for a Dolby Digital Reencoder
***************************************************************************/
void NEXUS_DolbyDigitalReencode_GetDefaultSettings(
    NEXUS_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a Dolby Digital Reencoder
***************************************************************************/
NEXUS_DolbyDigitalReencodeHandle NEXUS_DolbyDigitalReencode_Open(
    const NEXUS_DolbyDigitalReencodeSettings *pSettings /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close a Dolby Digital Reencoder
***************************************************************************/
void NEXUS_DolbyDigitalReencode_Close(
    NEXUS_DolbyDigitalReencodeHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for a Dolby Digital Reencoder
***************************************************************************/
void NEXUS_DolbyDigitalReencode_GetSettings(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Set Settings for a Dolby Digital Reencoder
***************************************************************************/
NEXUS_Error NEXUS_DolbyDigitalReencode_SetSettings(
    NEXUS_DolbyDigitalReencodeHandle handle,
    const NEXUS_DolbyDigitalReencodeSettings *pSettings
    );

/***************************************************************************
Summary:
    Connector Types for a Dolby Digital Reencoder
***************************************************************************/
#define NEXUS_DolbyDigitalReencodeConnectorType                 NEXUS_AudioConnectorType
#define NEXUS_DolbyDigitalReencodeConnectorType_eStereo         NEXUS_AudioConnectorType_eStereo
#define NEXUS_DolbyDigitalReencodeConnectorType_eMultichannel   NEXUS_AudioConnectorType_eMultichannel
#define NEXUS_DolbyDigitalReencodeConnectorType_eCompressed     NEXUS_AudioConnectorType_eCompressed
#define NEXUS_DolbyDigitalReencodeConnectorType_eMax            NEXUS_AudioConnectorType_eMax

/***************************************************************************
Summary:
    Get the audio connector for a Dolby Digital Reencoder
***************************************************************************/
NEXUS_AudioInput NEXUS_DolbyDigitalReencode_GetConnector(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioConnectorType connectorType
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyDigitalReencode_AddInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveAllInputs(
    NEXUS_DolbyDigitalReencodeHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DOLBY_DIGITAL_REENCODER_H__ */
