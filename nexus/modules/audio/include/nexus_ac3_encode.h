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
*   API name: Ac3Encode
*    Specific APIs related to AC3 Audio Encoding
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef NEXUS_AC3_ENCODE_H__
#define NEXUS_AC3_ENCODE_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_processing_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: Ac3Encode

Header file: nexus_ac3_encode.h

Module: Audio

Description: AC3 Audio Encoding

**************************************/

/**
Summary:
Handle for AC3 Encode stage
**/
typedef struct NEXUS_Ac3Encode *NEXUS_Ac3EncodeHandle;

/***************************************************************************
Summary:
    Get default settings for an AC3 Encode stage
***************************************************************************/
void NEXUS_Ac3Encode_GetDefaultSettings(
    NEXUS_Ac3EncodeSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an AC3 Encode stage
***************************************************************************/
NEXUS_Ac3EncodeHandle NEXUS_Ac3Encode_Open( /* attr{destructor=NEXUS_Ac3Encode_Close}  */
    const NEXUS_Ac3EncodeSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close an AC3 Encode stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_Ac3Encode_Close(
    NEXUS_Ac3EncodeHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an AC3 Encode stage
***************************************************************************/
void NEXUS_Ac3Encode_GetSettings(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_Ac3EncodeSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an AC3 Encode stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_SetSettings(
    NEXUS_Ac3EncodeHandle handle,
    const NEXUS_Ac3EncodeSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an AC3 Encode stage
***************************************************************************/
NEXUS_AudioInput NEXUS_Ac3Encode_GetConnector( 
    NEXUS_Ac3EncodeHandle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_AddInput(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_RemoveInput(
    NEXUS_Ac3EncodeHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_Ac3Encode_RemoveAllInputs(
    NEXUS_Ac3EncodeHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AC3_ENCODE_H__ */

