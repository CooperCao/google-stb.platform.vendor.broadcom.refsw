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
*   API name: DtsEncode
*    Specific APIs related to DTS Audio Encoding
*
***************************************************************************/

#ifndef NEXUS_DTS_ENCODE_H__
#define NEXUS_DTS_ENCODE_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: DtsEncode

Header file: nexus_dts_encode.h

Module: Audio

Description: DTS Audio Encoding

**************************************/

/**
Summary:
Handle for DTS Encode stage
**/
typedef struct NEXUS_DtsEncode *NEXUS_DtsEncodeHandle;

/***************************************************************************
Summary:
DTS Encode Settings
***************************************************************************/
typedef struct NEXUS_DtsEncodeSettings
{
    bool spdifHeaderEnabled;    /* If true, SPDIF header generation is enabled.  */
} NEXUS_DtsEncodeSettings;

/***************************************************************************
Summary:
    Get default settings for an DTS Encode stage
***************************************************************************/
void NEXUS_DtsEncode_GetDefaultSettings(
    NEXUS_DtsEncodeSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an DTS Encode stage
***************************************************************************/
NEXUS_DtsEncodeHandle NEXUS_DtsEncode_Open( /* attr{destructor=NEXUS_DtsEncode_Close}  */
    const NEXUS_DtsEncodeSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close an DTS Encode stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void NEXUS_DtsEncode_Close(
    NEXUS_DtsEncodeHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an DTS Encode stage
***************************************************************************/
void NEXUS_DtsEncode_GetSettings(
    NEXUS_DtsEncodeHandle handle,
    NEXUS_DtsEncodeSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an DTS Encode stage
***************************************************************************/
NEXUS_Error NEXUS_DtsEncode_SetSettings(
    NEXUS_DtsEncodeHandle handle,
    const NEXUS_DtsEncodeSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an DTS Encode stage
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_DtsEncode_GetConnector(
    NEXUS_DtsEncodeHandle handle
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DtsEncode_AddInput(
    NEXUS_DtsEncodeHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DtsEncode_RemoveInput(
    NEXUS_DtsEncodeHandle handle,
    NEXUS_AudioInputHandle input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_DtsEncode_RemoveAllInputs(
    NEXUS_DtsEncodeHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DTS_ENCODE_H__ */
