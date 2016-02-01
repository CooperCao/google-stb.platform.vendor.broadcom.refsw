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
*   API name: AudioOutput
*    API to group multiple stereo outputs into a multichannel output
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AUDIO_OUTPUT_GROUP_H__
#define NEXUS_AUDIO_OUTPUT_GROUP_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: AudioOutputGroup

Header file: nexus_audio_output_group.h

Module: Audio

Description: Allows grouping of multiple stereo outputs for multichannel audio

**************************************/

/***************************************************************************
Summary:
Audio Output Group Handle
***************************************************************************/
typedef struct NEXUS_AudioOutputGroup *NEXUS_AudioOutputGroupHandle;

/***************************************************************************
Summary:
Audio Output Group Creation Settings 
 
Description: 
This structure defines an array of stereo outputs that are grouped together 
to receive a channel pair of data.  To setup for 5.1 you should fill 
the array elements for NEXUS_AudioChannelPair_eLeftRight, 
NEXUS_AudioChannelPair_eLeftRightSurround, and 
NEXUS_AudioChannelPair_eCenterLfe.  To setup for 7.1, you should fill 
those members as well as NEXUS_AudioChannelPair_eLeftRightRear. 
 
Outputs will receive data only for channels that are sent to the output.  So, 
if you setup an output for 5.1 but only feed stereo data, the outputs on 
the Surround and CenterLfe channel pairs will not receive any data. 
 
Outputs in the group should all run off the same timing source.  So, take care 
when connecting DACs with any other outputs as on most systems DACs will 
run on a different clock than other outputs. 
***************************************************************************/
typedef struct NEXUS_AudioOutputGroupCreateSettings
{    
    NEXUS_AudioOutputHandle outputs[NEXUS_AudioChannelPair_eMax]; /* Output per channel pair.  If set to NULL, the channel pair is not supported. */
} NEXUS_AudioOutputGroupCreateSettings;

/***************************************************************************
Summary:
Get Default Audio Output Group Creation Settings 
***************************************************************************/
void NEXUS_AudioOutputGroup_GetDefaultCreateSettings( 
    NEXUS_AudioOutputGroupCreateSettings *pSettings
    );

/***************************************************************************
Summary:
Create an audio output group
***************************************************************************/
NEXUS_AudioOutputGroupHandle NEXUS_AudioOutputGroup_Create( /* attr{destructor=NEXUS_AudioOutputGroup_Destroy}  */
    const NEXUS_AudioOutputGroupCreateSettings *pSettings
    );

/***************************************************************************
Summary:
Destroy an audio output group
***************************************************************************/
void NEXUS_AudioOutputGroup_Destroy(
    NEXUS_AudioOutputGroupHandle handle
    );

/***************************************************************************
Summary:
Get the audio connector for an audio output group
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioOutputGroup_GetConnector(
    NEXUS_AudioOutputGroupHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_OUTPUT_GROUP_H__ */

