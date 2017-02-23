/***************************************************************************
 *  Copyright (C) 2007-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_HDMI_DVO_H__
#define NEXUS_HDMI_DVO_H__
#include "nexus_video_types.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for the hdmi digital video output interface.
**/
typedef struct NEXUS_HdmiDvo *NEXUS_HdmiDvoHandle;

/**
Summary:
Settings for hdmi digital video output interface
**/
typedef struct NEXUS_HdmiDvoSettings
{
    unsigned outputMode; /*12 bit or  24 bit mode */
    NEXUS_ColorSpace colorSpace; 
    NEXUS_HdmiColorDepth colorDepth; 
} NEXUS_HdmiDvoSettings;


/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_HdmiDvo_GetDefaultSettings(
    NEXUS_HdmiDvoSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Open a new hdmi digital video output interface
**/
NEXUS_HdmiDvoHandle NEXUS_HdmiDvo_Open( /* attr{destructor=NEXUS_HdmiDvo_Close}  */
    unsigned index,
    const NEXUS_HdmiDvoSettings *pSettings    /* settings */
    );

/**
Summary:
Close the hdmi digital video output interface
**/
void NEXUS_HdmiDvo_Close(
    NEXUS_HdmiDvoHandle output
    );

/**
Summary:
Get current settings
**/
void NEXUS_HdmiDvo_GetSettings(
    NEXUS_HdmiDvoHandle output,
    NEXUS_HdmiDvoSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Apply new settings
**/
NEXUS_Error NEXUS_HdmiDvo_SetSettings(
    NEXUS_HdmiDvoHandle output,
    const NEXUS_HdmiDvoSettings *pSettings
    );

/**
Summary:
Returns the abstract NEXUS_VideoOutput connector for this Interface.
The NEXUS_VideoOutput connector is added to a Display in order to route that Displa's video to the output.

Description:
Used in NEXUS_Display_AddOutput
**/
NEXUS_VideoOutputHandle NEXUS_HdmiDvo_GetConnector(
    NEXUS_HdmiDvoHandle output
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_HDMI_DVO_H__ */
