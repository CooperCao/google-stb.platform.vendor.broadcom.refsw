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
*   API name: Audio Return Channel (ARC)
*    Specific APIs related to Audio Return Channel output.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_RETURN_CHANNEL_H__
#define NEXUS_AUDIO_RETURN_CHANNEL_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct NEXUS_AudioReturnChannel *NEXUS_AudioReturnChannelHandle;

/***************************************************************************
Summary:
Audio Return Channel Settings
***************************************************************************/
typedef struct NEXUS_AudioReturnChannelSettings
{
    bool muted;
} NEXUS_AudioReturnChannelSettings;


/***************************************************************************
Summary:
    Get default settings for an Audio Return Channel
See Also:

 ***************************************************************************/
void NEXUS_AudioReturnChannel_GetDefaultSettings(
    NEXUS_AudioReturnChannelSettings *pSettings   /* [out] default settings */
    );


/***************************************************************************
Summary:
    Open an Audio Return Channel
See Also:
    NEXUS_AudioReturnChannel_Close
 ***************************************************************************/
NEXUS_AudioReturnChannelHandle NEXUS_AudioReturnChannel_Open( /* attr{destructor=NEXUS_AudioReturnChannel_Close}  */
    unsigned index,
    const NEXUS_AudioReturnChannelSettings *pSettings     /* Pass NULL for default settings */
    );


/***************************************************************************
Summary:
    Close an Audio Return Channel
Description:
    Input to the Audio Return Channel must be removed prior to closing.
See Also:
    NEXUS_AudioReturnChannel_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
void NEXUS_AudioReturnChannel_Close(
    NEXUS_AudioReturnChannelHandle handle
    );


/***************************************************************************
Summary:
    Get Settings for an Audio Return Channel
See Also:
    NEXUS_AudioReturnChannel_SetSettings
 ***************************************************************************/
void NEXUS_AudioReturnChannel_GetSettings(
    NEXUS_AudioReturnChannelHandle handle,
    NEXUS_AudioReturnChannelSettings *pSettings    /* [out] Settings */
    );


/***************************************************************************
Summary:
    Set Settings for an Audio Return Channel
See Also:
    NEXUS_AudioReturnChannel_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_AudioReturnChannel_SetSettings(
    NEXUS_AudioReturnChannelHandle handle,
    const NEXUS_AudioReturnChannelSettings *pSettings    /* [in] Settings */
    );


/***************************************************************************
Summary:
    Get the audio connector for an Audio Return Channel
See Also:

 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioReturnChannel_GetConnector(
    NEXUS_AudioReturnChannelHandle handle
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_RETURN_CHANNEL_H__ */

