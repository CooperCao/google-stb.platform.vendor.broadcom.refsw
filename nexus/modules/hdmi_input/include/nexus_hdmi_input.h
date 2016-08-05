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
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_HDMI_INPUT_H__
#define NEXUS_HDMI_INPUT_H__

/*=***********************************
HdmiInput is an HDMI receiver.
It routes audio and video data from an HDMI receiver port to the Display and Audio modules.
Each instance controls one HDMI receiver port.
*************************************/

#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_audio_types.h"
#include "nexus_spdif_types.h"
#include "nexus_hdmi_input_hdcp_types.h"
#include "nexus_timebase.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
Summary:
Handle for the HdmiInput receiver interface.

Description:
An HDMI frontend must be attached to an HDMI receiver in order to process audio and video.
The number of HDMI receivers is based on the number of simultaneous audio/video streams can be processed at a time (main and possibly PIP).
The number of HDMI frontends is based on the number of HDMI connections on the box.
**/
typedef struct NEXUS_HdmiInput *NEXUS_HdmiInputHandle;


/**
Summary:
Settings used to configure an HdmiInput receiver interface.
**/
typedef struct NEXUS_HdmiInputSettings
{
    NEXUS_Timebase timebase; /* The timebase which is locked to this HdmiInput.
                                When the video frame rate is discovered, it will be automatically configure the Timebase
                                if NEXUS_TimebaseSettings.sourceType == NEXUS_TimebaseSourceType_eHdDviIn for this timebase. */
    NEXUS_CallbackDesc avMuteChanged; /* called when NEXUS_HdmiInputStatus.avMute changes */
    NEXUS_CallbackDesc sourceChanged; /* called when the audio or video format changes. Call NEXUS_HdmiInput_GetStatus to get new format information. */
    bool forcePcFormat;  /* If true, a PC format will be used for the HDMI format if available.  Currently, this applies to 1080p inputs only.
                            This may be useful if capturing PC data over HDMI.  Default=false. */
    bool autoColorSpace; /* if autoColorSpace is true, then whatever color space reported by the HDMI receiver is used (default is true) */
    NEXUS_ColorSpace colorSpace; /* if autoColorSpace is false, then this value is used instead of the value reported by the HDMI receiver */
    bool useInternalEdid ; /* deprecated */
    bool powerDownUnusedPort ;  /* use if HDMI port will never be used in design */

    struct   {
        bool equalizationEnabled;
        bool externalSwitch;
        bool hpdDisconnected ;
        NEXUS_CallbackDesc cecCallback;
        NEXUS_CallbackDesc hotPlugCallback;
    } frontend;

    struct {
        bool parseAviInfoframe ;

        bool disableI2cSclPullUp ;
        bool disableI2cSdaPullUp ;

        bool enableHdmiHardwarePassthrough ;
    } hdr ;

    NEXUS_CallbackDesc aviInfoFrameChanged;
    NEXUS_CallbackDesc audioInfoFrameChanged;
    NEXUS_CallbackDesc spdInfoFrameChanged;
    NEXUS_CallbackDesc vendorSpecificInfoFrameChanged;
    NEXUS_CallbackDesc audioContentProtectionChanged;
    NEXUS_CallbackDesc gamutMetadataPacketChanged;
    NEXUS_HeapHandle heap;

    bool adcCalibration;
} NEXUS_HdmiInputSettings;


/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_HdmiInput_GetDefaultSettings(
    NEXUS_HdmiInputSettings *pSettings /* [out] */
    );

/**
Summary:
Open a new HdmiInput receiver interface
**/
NEXUS_HdmiInputHandle NEXUS_HdmiInput_Open( /* attr{destructor=NEXUS_HdmiInput_Close}  */
    unsigned index,
    const NEXUS_HdmiInputSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Open a new HdmiInput receiver interface with EDID RAM support
**/
NEXUS_HdmiInputHandle NEXUS_HdmiInput_OpenWithEdid( /* attr{destructor=NEXUS_HdmiInput_Close}  */
    unsigned index,
    const NEXUS_HdmiInputSettings *pSettings, /* attr{null_allowed=y} */
    const uint8_t *edidData, /* attr{nelem=edidDataSize} */
    uint16_t edidDataSize
    );

/**
Summary:
Close an HdmiInput receiver interface
**/
void NEXUS_HdmiInput_Close(
    NEXUS_HdmiInputHandle handle
    );

/**
Summary:
Get current settings
**/
void NEXUS_HdmiInput_GetSettings(
    NEXUS_HdmiInputHandle    handle,
    NEXUS_HdmiInputSettings *pSettings /* [out] */
    );

/**
Summary:
Set new settings
**/
NEXUS_Error NEXUS_HdmiInput_SetSettings(
    NEXUS_HdmiInputHandle handle,
    const NEXUS_HdmiInputSettings *pSettings
    );

/**
Summary:
Status returned by NEXUS_HdmiInput_GetStatus
**/
typedef struct NEXUS_HdmiInputStatus
{
    /* From HDMI RX */
    bool                validHdmiStatus ;
    uint8_t             hdmiMode; /* 1 - HDMI mode ; 0 - DVI mode */
    uint8_t             deviceAttached; /* 1 - HDMI cable connected ; 0 - HDMI cable disconnected */
    uint8_t             pllLocked; /* 1 - HDMI PLL locked ; 0 - HDMI PLL unlocked */
    bool                packetErrors; /* true - HDMI packet error occurs ; false - no any packet error occurs */
    bool                avMute; /* true - the current avmute status is mute ; false - the current avmute status is unmute */
    bool                hdcpRiUpdating; /* true - HDCP Ri key is updating ; false - HDCP Ri key is not updating */
    bool                symbolLoss; /* true - symbol loss is detected ; false - symbol loss is not detected  */
    uint32_t            lineClock; /* HDMI RX physical line clock */
    NEXUS_ColorSpace    colorSpace; /* The color space repored by the HDMI receiver. If NEXUS_HdmiInputSettings.autoColorSpace is true, this value is being used.
                                       If NEXUS_HdmiInputSettings.autoColorSpace is false, this value is being ignored. */
    unsigned colorDepth; /* 8bit color depth is standard. 10bit & 12bit deep color are supported with HDMI 1.3 platforms only */

    NEXUS_AspectRatio   aspectRatio; /* aspect ration from AVI info packet */
    uint32_t            width; /* the actual width in HDMI RX format detection */
    uint32_t            height; /* the actual height in HDMI RX format detection */

    /* From HDDVI */
    bool                interlaced; /* true - interlaced format ; false - progressive format. From the format data HDDVI locked to*/
    NEXUS_VideoFormat   originalFormat; /* which format HDDVI has been locked to */
    bool                noSignal; /* HDDVI detects no input signal */
    uint32_t            avWidth; /* the actual width in HDDVI format detection */
    uint32_t            avHeight; /* the actual height in HDDVI format detection */
    uint32_t            vertFreq; /* vertical frequency from the format data HDDVI locked to, in units of 1/100 Hz */
    NEXUS_VideoPolarity hPolarity; /* the actual horizontal polarity in HDDVI format detection */
    NEXUS_VideoPolarity vPolarity; /* the actual vertical polarity in HDDVI format detection */
    uint32_t            vBlank; /* the actual vertical blank in HDDVI format detection */
    uint32_t            hBlank; /* the actual horizontal blank in HDDVI format detection */

    uint8_t pixelRepeat; /* the actual pixel repeat status in HDDVI format detection */

    struct {
        uint16_t packets; /* The number of audio packet between two successive vsyncs */
        bool validSpdifInfo ; /* indicator that Stream Type, Word Length, and Sample Frequency are valid */
        NEXUS_SpdifStreamType streamType; /* audio format: PCM or not PCM */
        NEXUS_SpdifWordLength wordLength; /* audio word length, will be significant if it's PCM audio */
        NEXUS_SpdifSamplingFrequency sampleFreq; /* audio sample frequency, will be significant if it's PCM audio  */
    } audio; /* audio information */
} NEXUS_HdmiInputStatus;

/**
Summary:
Get status
**/
NEXUS_Error NEXUS_HdmiInput_GetStatus(
    NEXUS_HdmiInputHandle handle,
    NEXUS_HdmiInputStatus *pStatus /* [out] */
    );


/**
Summary:
Returns the abstract NEXUS_VideoInput connector for the HdmiInput.
This connector is passed to a VideoWindow to display the decoded video.

Description:
Returns:
NEXUS_VideoInput - abstract connector

See Also:
NEXUS_VideoWindow_AddInput
**/
NEXUS_VideoInputHandle NEXUS_HdmiInput_GetVideoConnector( /* attr{shutdown=NEXUS_VideoInput_Shutdown} */
    NEXUS_HdmiInputHandle handle
    );

/**
Summary:
Returns the abstract NEXUS_AudioInput connector for the HdmiInput.
This connector is passed to a Mixer or AudioOutput to route audio to an output.

Description:
Returns:
NEXUS_AudioInputHandle - abstract connector

See Also:
NEXUS_Mixer_AddInput
**/
NEXUS_AudioInputHandle NEXUS_HdmiInput_GetAudioConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_HdmiInputHandle handle
    );

/**
Summary:
Control the connection of HDMI by forcing the hotplug out pin to high or low.
**/
NEXUS_Error NEXUS_HdmiInput_SetHotPlug(
    NEXUS_HdmiInputHandle handle,
    bool status /* True - to disconnect, False - to connect */
    );

/**
Summary:
Manual hot plug configuration

Description:
Use this function if your board does not use our hotplug pin, but uses GPIO instead.
Invoke this function to make some necessary configuration changes when the connection status changes.
**/
NEXUS_Error NEXUS_HdmiInput_ConfigureAfterHotPlug(
    NEXUS_HdmiInputHandle handle,
    bool status /* True - connection up, False - connnection down */
    );

/**
Summary:
Use this function to load HDMI Rx EDID Data to on-chip EDID RAM
**/
NEXUS_Error NEXUS_HdmiInput_LoadEdidData(
    NEXUS_HdmiInputHandle handle,
    const uint8_t *dataBytes, /* attr{nelem=numBytes} */
    uint16_t numBytes
    );

/**
Summary:
Use this function to unload/free any resources used to load HDMI Rx EDID Data
**/
NEXUS_Error NEXUS_HdmiInput_UnloadEdidData(
    NEXUS_HdmiInputHandle handle
    );

/**
Summary:
Set the master HDMI frontend for 3DTV mode
This requires coordinated calls to both master and slave, and also required
NEXUS_VideoWindow_Add/RemoveInput calls to reinitialized the video pipeline.
See nexus/examples/3dtv/dtv for code.
**/
NEXUS_Error NEXUS_HdmiInput_SetMaster(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputHandle master
    );

/**
Summary:
Force a toggle of the Hot Plug on the hdmiInput
**/
void NEXUS_HdmiInput_ToggleHotPlug(
    NEXUS_HdmiInputHandle hdmiInput
    );

#ifdef __cplusplus
}
#endif

#endif
