/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#ifndef NEXUS_HDMI_INPUT_EXT_H__
#define NEXUS_HDMI_INPUT_EXT_H__

#include "nexus_types.h"
#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_info.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
Summary:
Get a copy of the original Packet Bytes received by the HDMI receiver
**/
NEXUS_Error NEXUS_HdmiInput_GetRawPacketData(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputPacketType packetType,
    NEXUS_HdmiPacket *pPacket   /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetAviInfoFrameData(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiAviInfoFrame *pAviInfoFrame /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetAudioInfoFrameData(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiAudioInfoFrame *pAudioInfoFrame /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetSpdInfoFrameData(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiSpdInfoFrame *pSpdInfoFrame /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetAudioContentProtection(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiAudioContentProtection *pAcp /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetAudioClockRegeneration(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiAudioClockRegeneration *pAudioClockRegeneration /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetGeneralControlPacket(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiGeneralControlPacket *pGeneralControlPacket /* [out] */
    );


/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetVendorSpecificInfoFrameData(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiVendorSpecificInfoFrame *pVendorSpecificInfoFrame /* [out] */
    );


/**
Summary: 
Function to retrieve International Standard Recording Code and/or UPC/EAN Data 
describing the origin or owner details for each track of content on the medium
**/
NEXUS_Error NEXUS_HdmiInput_GetISRCData( 
    NEXUS_HdmiInputHandle hdmiInput, 
    NEXUS_HdmiISRC *pISRCData /* [out] */
    ) ;

/**
Summary:
Debug function to get data to form Eye Diagram

Description:
Use this function to create an Eye Diagram.
**/
NEXUS_Error NEXUS_HdmiInput_DebugGetEyeDiagramData(
    NEXUS_HdmiInputHandle hdmiInput,
    uint8_t *pAdcData /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif

