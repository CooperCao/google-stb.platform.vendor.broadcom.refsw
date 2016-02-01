/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
#ifndef NEXUS_CCIR656_INPUT_H__
#define NEXUS_CCIR656_INPUT_H__

#include "nexus_types.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
The supported methods for encapsulating VBI data into ITU-R 656 bitstreams
**/
typedef enum NEXUS_Ccir656VbiFormat
{
    NEXUS_Ccir656VbiFormat_eSaa7113 = 0,
    NEXUS_Ccir656VbiFormat_eModifiedSaa7113,
    NEXUS_Ccir656VbiFormat_eSaa7114,
    NEXUS_Ccir656VbiFormat_eSaa7114A,
    NEXUS_Ccir656VbiFormat_eSaa7114B,
    NEXUS_Ccir656VbiFormat_eSaa7114C,
    NEXUS_Ccir656VbiFormat_eSaa7115,
    NEXUS_Ccir656VbiFormat_eSmpte291,
    NEXUS_Ccir656VbiFormat_eMax
} NEXUS_Ccir656VbiFormat;


/**
Summary:
Settings for Ccir656Input input
**/
typedef struct NEXUS_Ccir656InputSettings {
    NEXUS_VideoFormat format; /* If autoFormatMode is false, format will be set as the video format. */
    struct {
        NEXUS_Ccir656VbiFormat format;
        unsigned bufferSize;    /* Size of 656 ancillary ring buffer in bytes. Default is zero.
                                   You will also need to set NEXUS_DisplayModuleSettings.vbi.ccir656InputBufferSize. */
        bool longHeader; /* If true, then incomping SMPTE 291M packets are expected to have the three
                            byte "component stream" header 00 FF FF.  Otherwise, packets are expected to have
                            the one byte "composite stream" header FF.  Default is false. */
        bool brokenDataCount; /* If true, then the incoming SMPTE 291M packets are expected to be in the
                                 historical, incorrect format produced by Broadcom ITU-R 656 encoders.  In particular,
                                 the packet DATA COUNT field counts bytes instead of DWORDs, and the accompanying
                                 parity bits are not present. Default is true. */
        NEXUS_CallbackDesc dataReady;
    } vbi;
} NEXUS_Ccir656InputSettings;

/**
Summary:
Handle for the Ccir656 input interface
**/
typedef struct NEXUS_Ccir656Input *NEXUS_Ccir656InputHandle;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_Ccir656Input_GetDefaultSettings(
    NEXUS_Ccir656InputSettings *pSettings /* [out] */
    );

/**
Summary:
Open a new Ccir656 Input.

**/
NEXUS_Ccir656InputHandle NEXUS_Ccir656Input_Open( /* attr{destructor=NEXUS_Ccir656Input_Close} */
    unsigned index,
    const NEXUS_Ccir656InputSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Close a Ccir656 Input.
**/
void NEXUS_Ccir656Input_Close(
    NEXUS_Ccir656InputHandle ccir656Input
    );

/**
Summary:
This function is used to get current configuration
**/
void NEXUS_Ccir656Input_GetSettings(
    NEXUS_Ccir656InputHandle ccir656Input,
    NEXUS_Ccir656InputSettings *pSettings /* [out] */
    );

/**
Summary:
This function is used to set new configuration
**/
NEXUS_Error NEXUS_Ccir656Input_SetSettings(
    NEXUS_Ccir656InputHandle ccir656Input,
    const NEXUS_Ccir656InputSettings *pSettings
    );

/**
Summary:
Returns the abstract NEXUS_VideoInput connector for the video input.
This connector is passed to an NEXUS_AnalogVideoDecoder to digitize the analog video.

Description:
Assign to NEXUS_AnalogVideoDecoderSettings.videoInput
**/
NEXUS_VideoInputHandle NEXUS_Ccir656Input_GetConnector(
    NEXUS_Ccir656InputHandle ccir656Input
    );

/**
Summary:
Header for 656 ancillary data packets in the ring buffer
**/
typedef struct NEXUS_Ccir656InputVbiHeader
{
    bool topField;
    uint8_t dataId;
    uint8_t secondId;
    unsigned payload; /* size of payload following this header in bytes */
} NEXUS_Ccir656InputVbiHeader;

/**
Summary:
Get pointer to data in the 656 ancillary data ring buffer

Description:
The application is responsible for parsing the header and the payload correctly.
Nexus will always atomically add a header plus all of its payload.
Application must handle wrap around. Wraparound could occur in the header or in the payload.
**/
NEXUS_Error NEXUS_Ccir656Input_GetBuffer(
    NEXUS_Ccir656InputHandle ccir656Input,
    void **pBuffer, /* [out] attr{memory=cached} pointer to data. size of data determined by size. */
    unsigned *pSize /* [out] */
    );

/**
Summary:
Update amount consumed from last NEXUS_Ccir656Input_GetBuffer call.
**/
NEXUS_Error NEXUS_Ccir656Input_ReadComplete(
    NEXUS_Ccir656InputHandle ccir656Input,
    unsigned sizeConsumed
    );

/**
Summary:
Flush 656 ancillary data ring buffer
**/
void NEXUS_Ccir656Input_Flush(
    NEXUS_Ccir656InputHandle ccir656Input
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_CCIR656_INPUT_H__ */

