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
*   API name: SpdifOutput
*    Specific APIs related to SPDIF audio outputs.
*
***************************************************************************/

#ifndef NEXUS_SPDIF_OUTPUT_H__
#define NEXUS_SPDIF_OUTPUT_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: SpdifOutput

Header file: nexus_spdif_output.h

Module: Audio

Description: Route PCM or compressed audio to a SPDIF output

**************************************/

/**
Summary:
Handle for SPDIF output
**/
typedef struct NEXUS_SpdifOutput *NEXUS_SpdifOutputHandle;

/***************************************************************************
Summary:
SPDIF Output Settings
***************************************************************************/
typedef struct NEXUS_SpdifOutputSettings
{
    bool    limitTo16Bits;  /* If true, the output will be limited to 16 bits */

    bool    dither;         /* If true, a dither signal will be sent out when
                               there is no data on this output in PCM mode. */

    NEXUS_SpdifOutputBurstType burstType; /* Burst type insertion for compressed data */

    /* The information below can be changed while a decode is in progress */
    /* These values can be overridden with NEXUS_SpdifOutput_SetRawChannelStatus */
    NEXUS_AudioChannelStatusInfo channelStatusInfo;
} NEXUS_SpdifOutputSettings;

/***************************************************************************
Summary:
Get default settings for a SPDIF Output
***************************************************************************/
void NEXUS_SpdifOutput_GetDefaultSettings(
    NEXUS_SpdifOutputSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
Open a SPDIF Output
***************************************************************************/
NEXUS_SpdifOutputHandle NEXUS_SpdifOutput_Open( /* attr{destructor=NEXUS_SpdifOutput_Close}  */
    unsigned index,
    const NEXUS_SpdifOutputSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close a SPDIF Output

Description:
Input to the SPDIF output must be removed prior to closing.
***************************************************************************/
void NEXUS_SpdifOutput_Close(
    NEXUS_SpdifOutputHandle handle
    );

/***************************************************************************
Summary:
Get Settings for a SPDIF Output
***************************************************************************/
void NEXUS_SpdifOutput_GetSettings(
    NEXUS_SpdifOutputHandle handle,
    NEXUS_SpdifOutputSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
Set Settings for a SPDIF Output
***************************************************************************/
NEXUS_Error NEXUS_SpdifOutput_SetSettings(
    NEXUS_SpdifOutputHandle handle,
    const NEXUS_SpdifOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Get the audio connector for a SPDIF output
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_SpdifOutput_GetConnector(
    NEXUS_SpdifOutputHandle handle
    );

/***************************************************************************
Summary:
Program the transmitted channel status in raw format.

Description:
This API will set the SPDIF channel status bits using raw 64-bit values
per channel.  Not available on all platforms.  Once set, the values in
NEXUS_SpdifOutputSettings will be ignored.  To clear these settings, pass
NULL for pChannelStatus.
***************************************************************************/
NEXUS_Error NEXUS_SpdifOutput_SetRawChannelStatus(
    NEXUS_SpdifOutputHandle handle,
    const NEXUS_AudioRawChannelStatus *pChannelStatus /* attr{null_allowed=y} */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SPDIF_OUTPUT_H__ */

