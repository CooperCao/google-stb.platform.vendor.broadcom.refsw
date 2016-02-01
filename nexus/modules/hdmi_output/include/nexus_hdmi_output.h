/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
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
 *                      HdmiOutput: Specific interfaces for an HDMI/DVI output.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_HDMI_OUTPUT_H__
#define NEXUS_HDMI_OUTPUT_H__

#include "nexus_i2c.h"
#include "nexus_video_types.h"
#include "nexus_hdmi_types.h"
#include "nexus_hdmi_output_control.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for the HDMI output interface.
**/
typedef struct NEXUS_HdmiOutput *NEXUS_HdmiOutputHandle;


/**
Summary:
Settings to open the HDMI output interface
**/
typedef struct NEXUS_HdmiOutputOpenSettings
{
    NEXUS_I2cHandle i2c;        /* TMDS I2C Bus Handle */

    NEXUS_HdmiSpdInfoFrame spd;  /* Source Product Description */

    bool bypassEdidChecking;    /* DEBUG/TEST only, bypasses EDID checking if true */
    bool i2cSoftwareMode;       /* If true, enable I2C Bit Bang Mode */

    unsigned maxEdidRetries;        /* Maximum number of retries to read EDID from the receiver */
    unsigned powerPollingInterval;  /* Polling interval for receiver power changes in ms */

    unsigned maxRxSenseRetries;  /* Maximum number of retries to check if attached Rx is powered */
    unsigned rxSenseInterval;    /* time between checks for Rx Sense in milliseconds */

    bool manualTmdsControl; /* Nexus will never automatically enable the TMDS signal. App must call NEXUS_HdmiOutput_SetTmdsSignal. */

    unsigned hotplugChangeThreshold; /* number of hotplug changes per second that is considered normal; above this is considered excessive.
                    threshold of 0 disables check. */
} NEXUS_HdmiOutputOpenSettings;


/*
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
*/
void NEXUS_HdmiOutput_GetDefaultOpenSettings(
    NEXUS_HdmiOutputOpenSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Open an HDMI output interface
**/
NEXUS_HdmiOutputHandle NEXUS_HdmiOutput_Open( /* attr{destructor=NEXUS_HdmiOutput_Close} */
    unsigned index,
    const NEXUS_HdmiOutputOpenSettings *pSettings    /* attr{null_allowed=y} settings */
    );

/**
Summary:
Close the HdmiOutput interface
**/
void NEXUS_HdmiOutput_Close(
    NEXUS_HdmiOutputHandle output
    );

/**
Summary:
Settings to configure the HDMI output interface
**/
typedef struct NEXUS_HdmiOutputSettings
{
    bool preemphasisEnabled;    /* If true, preemphasis will be enabled.  This can be used with long cables */
    unsigned preFormatChangeAvMuteDelay;   /* Delay (in msec) between sending AVMute packet and changing display format */
    unsigned postFormatChangeAvMuteDelay;  /* Delay (in msec) between changing display format and sending AVUnmute packet */
    NEXUS_VideoFormat outputFormat;        /* Optional. Defaults to NEXUS_VideoFormat_eUnknown, which means display format is used for the output format. Otherwise, this is the VEC format. For instance, VEC upscale will be performed. */

    NEXUS_CallbackDesc hotplugCallback;    /* Callback will be called when a hotplug event occurs */
    NEXUS_CallbackDesc cecCallback;        /* Callback will be called when a CEC message is sent or received. See NEXUS_HdmiOutputCecStatus. */
    NEXUS_CallbackDesc mhlStandbyCallback; /* This is meant for putting the system in S3 standby mode. This occurs when a MHL standby message is received.
                                              The app must put the system into standby ASAP when this callback is received. MHL CTS specifies that "ample time"
                                              is given for going into standby. Since "ample time" is amb'guous at best, there is no guarantee that the intended
                                              standby process will be completed. As such the user must take this into account when designing the system's
                                              standby process. If the standby process is completed, wake up from S3 is faster, ie., warm boot.
                                              If it's not completed, wake up will be similar to a cold boot. */

    bool hdmiRxScdcMonitoring ;  /* enable SCDC polling for HDMI 2.x devices */
    NEXUS_CallbackDesc hdmiRxStatusChanged;     /* Callback will be called when the attached HDMI Rx reports a change in status (HDMI 2.x only) */


    struct
    {
        bool           professionalMode;    /* [0:0] The professional mode flag.
                                                TRUE: Professional mode. Other user
                                                options will not be considered.
                                                FALSE: Consumer mode.*/
        bool           swCopyRight;         /* [2:2] Software CopyRight assert.
                                                TRUE: CopyRight is asserted
                                                FALSE: CopyRight is not asserted */
        uint16_t       categoryCode;        /* [8:15] Category Code */
        uint16_t       clockAccuracy;       /* [28:29] Clock Accuracy */
        bool           separateLRChanNum;   /* TRUE:  Left channel num = 0000
                                                      Right Channel Num = 0000
                                               FALSE: Left channel num = 1000
                                                      Right Channel Num = 0100 */
    } audioChannelStatusInfo;

    bool audioDitherEnabled;    /* If true, a dither signal will be sent out when
                                   there is no audio data on this output in PCM mode. */
    NEXUS_SpdifOutputBurstType  audioBurstType; /* burst type when the HDMI compressed audio
                                                   transmission is inactive (decoder underflow, mute, etc) */

    unsigned audioBurstPadding;      /* Choose to pad to the end of pause bursts with additional words.
                                   Valid values are either 0 or 2.  Default is 0 */

    bool mpaaDecimationEnabled; /* When MPAA decimation is enabled, a significant number of pixels in the output video
                                   signal are replaced with values interpolated from neighboring pixels.
                                   Therefore, video quality is lowered. */

    bool autoColorSpace; /* deprecated */
    NEXUS_ColorSpace colorSpace; /* if not NEXUS_ColorSpace_eAuto , then this value is used instead of the value reported by the HDMI receiver.
                                    if NEXUS_ColorSpace_eAuto or autoColorSpace, a colorspace is chosen based on receiver and transmitter preferences and capabilities. */
    unsigned colorDepth; /* default is 8bit standard color depth. 10bit & 12bit deep color are supported with HDMI 1.3 platform only.
                            if 0, a value is chosen based on receiver and transmitter preferences and capabilities.*/

    bool overrideColorRange; /* If true, value of colorRange, rather than the default value based on receiver preferences and capabilities, is used */
    NEXUS_ColorRange colorRange; /* ColorRange to use when overrideColorRange is set */

    bool overrideMatrixCoefficients; /* If true, use matrixCoefficients settings vs. default matrixCoefficents settings based on format */
    NEXUS_MatrixCoefficients matrixCoefficients; /* MatrixCoefficient to use when overrideMatrixCoefficients is set */

    /* Spread Spectrum configuration */
    struct
    {
        bool                     enable;  /* enable Spread Spectrum */
        uint32_t                 frequency;
        uint32_t                 delta;
    } spreadSpectrum;

    bool syncOnly;

    NEXUS_HdmiDynamicRangeMasteringInfoFrame dynamicRangeMasteringInfoFrame;  /* Dynamic Range And Mastering InfoFrame */

    unsigned crcQueueSize; /* if non-zero, CRC capture is enabled. use NEXUS_HdmiOutput_GetCrcData to retrieve data. */
} NEXUS_HdmiOutputSettings;


/**
Summary:
Re-Enable HDMI Tx Interrupt that was disabled due to excessive interrupts
**/
void NEXUS_HdmiOutput_ReenableHotplugInterrupt(
    NEXUS_HdmiOutputHandle output
    );

/**
Summary:
Get current settings
**/
void NEXUS_HdmiOutput_GetSettings(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Apply new settings
Note: If an illegal combination is set to the system, the setting is not applied and an error is returned.
As an example, if matrixCoefficients is overridden with NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL when
display video format is NTSC, it is an illegal combination.
**/
NEXUS_Error NEXUS_HdmiOutput_SetSettings(
    NEXUS_HdmiOutputHandle output,
    const NEXUS_HdmiOutputSettings *pSettings
    );


/**
Summary:
Macros for HDMI 3D Structures bit-fields, used in NEXUS_HdmiOutputStatus.hdmi3DFormatsSupported[]
**/
#define NEXUS_HdmiOutput_3DStructure_FramePacking               0x0001
#define NEXUS_HdmiOutput_3DStructure_FieldAlternative           0x0002
#define NEXUS_HdmiOutput_3DStructure_LineAlternative            0x0004
#define NEXUS_HdmiOutput_3DStructure_SideBySideFull             0x0008

#define NEXUS_HdmiOutput_3DStructure_LDepth                     0x0010
#define NEXUS_HdmiOutput_3DStructure_LDepthGraphics             0x0020
#define NEXUS_HdmiOutput_3DStructure_TopAndBottom               0x0040
#define NEXUS_HdmiOutput_3DStructure_Reserved07                 0x0080

#define NEXUS_HdmiOutput_3DStructure_SideBySideHalfHorizontal   0x0100
#define NEXUS_HdmiOutput_3DStructure_Reserved09                 0x0200
#define NEXUS_HdmiOutput_3DStructure_Reserved10                 0x0400
#define NEXUS_HdmiOutput_3DStructure_Reserved11                 0x0800

#define NEXUS_HdmiOutput_3DStructure_Reserved12                 0x1000
#define NEXUS_HdmiOutput_3DStructure_Reserved13                 0x2000
#define NEXUS_HdmiOutput_3DStructure_Reserved14                 0x4000
#define NEXUS_HdmiOutput_3DStructure_SideBySideHalfQuincunx     0x8000



/****
Summary:
Data structure containing audio and video latency information for auto lipsync feature
support by the HDMI receiver. These information are located in the receiver's
EDID PROM
******/
typedef struct NEXUS_HdmiOutputAutoLipsyncInfo
{
    uint8_t videoLatency;   /* video latency for progressive video formats (ms) */
    uint8_t audioLatency;   /* audio latency for progressive video formats (ms) */
    uint8_t interlacedVideoLatency;     /* video latency for interlaced video format (ms) */
    uint8_t interlacedAudioLatency;     /* audio lantecy for interfaced video format (ms) */
} NEXUS_HdmiOutputAutoLipsyncInfo;



/****
Summary:
Data structure containing Monitor Ranges data contained in a receiver's EDID
PROM
******/
typedef struct NEXUS_HdmiOutputMonitorRange
{
    uint16_t minVertical;     /* Minimum Vertical Rate (Hz) */
    uint16_t maxVertical;     /* Maximum Vertical Rate (Hz) */
    uint16_t minHorizontal;   /* Minimum Horizontal Rate (kHz) */
    uint16_t maxHorizontal;   /* Maximum Horizontal Rate (kHz) */
    uint16_t maxPixelClock;   /* Maximum Supported Pixel Clock Rate (MHz) */
    uint8_t secondaryTiming;  /* Secondary Timing Formula (if supported) */
    uint8_t secondaryTimingParameters[7];  /* Secondary Timing Formula Params */
} NEXUS_HdmiOutputMonitorRange;


/****
Summary:
Data structure containing Monitor Colorimetry Data Block  in a receiver's EDID ROM
******/

typedef  struct NEXUS_HdmiOutputMonitorColorimetry
{
    bool extendedColorimetrySupported[NEXUS_HdmiEdidColorimetryDbSupport_eMax];
    bool metadataProfileSupported[NEXUS_HdmiEdidColorimetryDbMetadataProfile_eMax];
} NEXUS_HdmiOutputMonitorColorimetry;

/****
Summary:
Tx Hardware Status
******/
typedef  struct NEXUS_HdmiOutputTxHardwareStatus
{
    bool clockPower;
    bool channelPower[3];
    bool hotplugInterruptEnabled;
    unsigned hotplugCounter;  /* total since device opened */
    unsigned rxSenseCounter;  /* total since device opened */
    bool scrambling ;  /* tx is sending a scrambled signal */
    unsigned unstableFormatDetectedCounter; /* total since last format change */
} NEXUS_HdmiOutputTxHardwareStatus;

/****
Summary:
Rx Hardware Status
******/
typedef  struct NEXUS_HdmiOutputRxHardwareStatus
{
    bool descrambling;  /* rx is descrambling the incoming signal */
} NEXUS_HdmiOutputRxHardwareStatus;



/**
Summary:
Status for the HDMI interface
**/
typedef struct NEXUS_HdmiOutputStatus
{
    /**********/
    /* HDMI Status variables */

    unsigned index;
    bool connected;    /* True if Rx device is connected; device may be ON or OFF */
                               /* if !connected the remaining values should be ignored */
                               /* HDMI Rx capability information (EDID) can be read with Rx power off */

    bool rxPowered;    /* True if Rx device is powered ON, false OFF */

    NEXUS_VideoFormat videoFormat;
    NEXUS_AspectRatio aspectRatio;
    NEXUS_ColorSpace colorSpace;

    NEXUS_AudioCodec audioFormat;
    unsigned audioSamplingRate; /* in units of Hz */
    unsigned audioSamplingSize;
    unsigned audioChannelCount;

    NEXUS_HdmiOutputTxHardwareStatus txHardwareStatus;
    NEXUS_HdmiOutputRxHardwareStatus rxHardwareStatus ;

    /**********/
    /* HDMI Rx information/capabilities (extracted from the Rx EDID) */

    bool hdmiDevice;    /* True if Rx supports HDMI, false if supports only DVI */
    char monitorName[14];              /* NULL-terminated string for the monitor name */
    NEXUS_HdmiOutputMonitorRange monitorRange;  /* supported min and max H/V freqency ranges */
    NEXUS_VideoFormat preferredVideoFormat;  /* monitor's preferred video format */
    bool videoFormatSupported[NEXUS_VideoFormat_eMax];
    uint16_t hdmi3DFormatsSupported[NEXUS_VideoFormat_eMax]; /* This is a bit-field value. Use NEXUS_HdmiOutput_3DStructure_xxx macros
                                                               to determine the supported 3D Structures for a particular video format. */

    /* Please see HDMI Specification Section 8.3.3 for Colorimetry Datablock details */
    NEXUS_HdmiOutputMonitorColorimetry monitorColorimetry;

    bool audioCodecSupported[NEXUS_AudioCodec_eMax];  /*  audio codecs the monitor supports */
    unsigned maxAudioPcmChannels;                     /* max number of PCM audio channels supported by the receiver -
                                                          2 for stereo, 6 or 8 typically for decoded multichannel data. */

    /* Please see HDMI Specification 1.3a under Section 8.6 for details of Physical Address.
        * Each node (A..D) represent an attached HDMI device on HDMI device tree. */
    uint8_t physicalAddressA;          /* Physical Address for HDMI node A */
    uint8_t physicalAddressB;          /* Physical Address for HDMI node B */
    uint8_t physicalAddressC;          /* Physical Address for HDMI node C */
    uint8_t physicalAddressD;          /* Physical Address for HDMI node D */

    NEXUS_HdmiOutputAutoLipsyncInfo autoLipsyncInfo;
} NEXUS_HdmiOutputStatus;

/**
Summary:
Get current status of the HDMI transmitter
**/
NEXUS_Error NEXUS_HdmiOutput_GetStatus(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputStatus *pStatus    /* [out] Status */
    );

/**
Summary:
Basic EDID data from the receiver device
**/
typedef struct NEXUS_HdmiOutputBasicEdidData
{
    uint8_t vendorID[2];  /* Vendor ID from Microsoft; compressed ASCII */
    uint8_t productID[2]; /* Product ID assigned by Vendor */
    uint8_t serialNum[4]; /* Serial # assigned by Vendor; may be undefined */
    uint8_t manufWeek;    /* Week of Manufture (1..53)          */
    uint16_t manufYear;   /* Year of Manufacture + 1990         */
    uint8_t edidVersion;  /* Version of Edid (version 1 or 2)   */
    uint8_t edidRevision; /* Revision of Edid 1.3 or 2.0        */
    uint8_t maxHorizSize; /* Max Horizontal Image size in cm.   */
    uint8_t maxVertSize;  /* Max Vertical Image size in cm.     */
    uint8_t extensions;   /* Number of 128 byte EDID extensions */

    uint8_t features;      /* Features are not supported; standby,suspend, etc */

    /*  Detailed Timings/Descriptors */
    char monitorName[14];              /* NULL-terminated string for the monitor name */
    NEXUS_HdmiOutputMonitorRange monitorRange;  /* supported min and max H/V freqency ranges */
    NEXUS_VideoFormat preferredVideoFormat;  /* monitor's preferred video format */
} NEXUS_HdmiOutputBasicEdidData;


/**
Summary:
Content of the HDMI Vendor Specific Data Block (VSDB)
located in the Version 3 Timing Extension of the HDMI Receiver's EDID.
See Section 8.3.2 in the HDMI 1.x Spec for details
**/
typedef struct NEXUS_HdmiOutputEdidRxHdmiVsdb
{
    bool valid;
    uint8_t version;

    /* Vendor Specific Data Block */
    uint8_t tagCode;      /* Vendor Specific Tag Code = 3 */
    uint8_t ieeeRegId[3]; /* Registration Identified 0x000C03 */

    /* Please see HDMI Specification 1.3a under Section 8.6 for details of Physical Address.
        * Each node (A..D) represent an attached HDMI device on HDMI device tree. */
    uint8_t physicalAddressA;          /* Physical Address for HDMI node A */
    uint8_t physicalAddressB;          /* Physical Address for HDMI node B */
    uint8_t physicalAddressC;          /* Physical Address for HDMI node C */
    uint8_t physicalAddressD;          /* Physical Address for HDMI node D */

    /* supported features */
    bool acpIsrcN;  /* sink supports ACP, ISRC1, or ISRC2 packets */
    bool deepColor30bit;
    bool deepColor36bit;
    bool deepColor48bit;
    bool deepColorY444;
    bool dviDual;

    uint16_t maxTmdsClockRate;

    /* supported features */
    bool latencyFieldsPresent;
    bool interlacedLatencyFieldsPresent;
    bool hdmiVideoPresent;
    bool contentTypeGraphicsText;
    bool contentTypePhoto;
    bool contentTypeCinema;
    bool contentTypeGame;

    uint8_t videoLatency;  /* Section 8.3.2 in the HDMI 1.x Spec for details */
    uint8_t audioLatency;
    uint8_t interlacedVideoLatency;
    uint8_t interlacedAudioLatency;

    bool hdmi3DPresent;
    uint8_t hdmi3DMultiPresent;

    uint8_t hdmiImageSize;
    uint8_t hdmiVICLen;
    uint8_t hdmi3DLen;

    /* Functions Supported */
    bool underscan;
    bool audio;
    bool yCbCr444;
    bool yCbCr422;

    /*
    HDMI VICs and 3D supported formats are contained in NEXUS_HdmiOutputStatus
        bool videoFormatSupported[NEXUS_VideoFormat_eMax];
        uint16_t hdmi3DFormatsSupported[NEXUS_VideoFormat_eMax];

    OR use NEXUS_HdmiOutput_GetEdidData or NEXUS_HdmiOutput_GetVideoFormatSupport
         for supported format information
    */

    uint8_t nativeFormatsInDescriptors;

} NEXUS_HdmiOutputEdidRxHdmiVsdb;

/**
Summary:
HDMI Forum Vendor Specific Data Block (HF VSDB) from HDMI 2.0
**/
typedef struct NEXUS_HdmiOutputEdidRxHdmiForumVsdb
{
    bool valid;

    /* Vendor Specific Data Block */
    uint8_t tagCode;      /* Vendor Specific Tag Code = 3 */
    uint8_t version;

    uint16_t maxTMDSCharacterRate;

    /* modes supported by attached HDMI Rx */
    bool _3dOsdDisparity;
    bool dualView;
    bool independentView;
    bool sub340McscScramble;  /* LTE_240Mcsc_scramble - Sink supports scrambling for pixel clocks <= 340MHz */
    bool readRequest;               /* rrCapable - Sink can initiate SCDC Read Request */
    bool scdc;               /* Status and Control Data Channel */
    bool deepColor420_30bit;  /* 4:2:0 10 bit color support */
    bool deepColor420_36bit;  /* 4:2:0 12 bit color support */
    bool deepColor420_48bit;  /* 4:2:0 16 bit color support */

} NEXUS_HdmiOutputEdidRxHdmiForumVsdb;



/**
Summary:
HDR (High Dynamic Range) Static Metatdata Data Block
**/
typedef struct NEXUS_HdmiOutputEdidRxHdrdb
{
    bool valid;  /* valid HDR DB found */

    bool eotfSupported[NEXUS_VideoEotf_eMax];

} NEXUS_HdmiOutputEdidRxHdrdb;

/**
Summary:
Parsed EDID Information
**/
typedef struct NEXUS_HdmiOutputEdidData
{
    bool valid;

    /**********************/
    /*    EDID BLOCK 0    */
    /**********************/

    /* all DVI/HDMI devices have EDID BLOCK 0 */
    /* All data from EDID Block 0 is contained in basicData */
    NEXUS_HdmiOutputBasicEdidData basicData;


    /**********************/
    /*    EDID BLOCK 1    */
    /**********************/

    /* VSDB (Vendor Specific data block)  - required for HDMI */
     NEXUS_HdmiOutputEdidRxHdmiVsdb hdmiVsdb;

    /* HF-VSDB (HDMI Forum Vendor Specific data block) - required for certain HDMI 2.0 features */
    NEXUS_HdmiOutputEdidRxHdmiForumVsdb hdmiForumVsdb;

    /* HDR (High Dynamic Range) Static Metadata data block - required for HDR support */
    NEXUS_HdmiOutputEdidRxHdrdb hdrdb;

    /* video formats supported from all EDID Blocks */
    bool videoFormatSupported[NEXUS_VideoFormat_eMax];
} NEXUS_HdmiOutputEdidData;


typedef struct NEXUS_HdmiOutputEdidVideoFormatSupport
{
    /*  YCbCr 4:4:4 and RGB 4:4:4 */
    bool yCbCr444rgb444; /* equivalent to NEXUS_HdmiOutputEdidData.videoFormatSupported */

    /* YCbCr 4:2:0 support */
    bool yCbCr420;

    /* YCbCr 4:2:0 support */
    bool yCbCr422;

    struct {
        bool framePacking;
        bool fieldAlternative;
        bool lineAlternative;
        bool sideBySideFull;

        bool lDepth;
        bool lDepthGraphics;
        bool topAndBottom;

        bool sideBySideHalfHorizontal;
        bool sideBySideHalfQuincunx;
    } _3d;

} NEXUS_HdmiOutputEdidVideoFormatSupport;

/**
Summary:
Get EDID information
**/
NEXUS_Error NEXUS_HdmiOutput_GetEdidData(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputEdidData *pEdidData    /* [out] */
    );


/**
Summary:
Get characteristics supported by video formats
**/
NEXUS_Error NEXUS_HdmiOutput_GetVideoFormatSupport(
    NEXUS_HdmiOutputHandle output,
    NEXUS_VideoFormat videoFormat,
    NEXUS_HdmiOutputEdidVideoFormatSupport *pVideoFormatSupport    /* [out] */
    );


/**
Summary:
Video information supported by the attached HDMI receiver
**/
typedef struct NEXUS_HdmiOutputSupportedVideoInfo
{
    uint8_t supportedVideoIDCode[64];
    uint8_t numSupportedVideoDescriptors;
} NEXUS_HdmiOutputSupportedVideoInfo;


/**
Summary:
Get basic EDID data from the receiver device
**/
NEXUS_Error NEXUS_HdmiOutput_GetBasicEdidData(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputBasicEdidData *pData    /* [out] Basic Data */
    );

/**
Summary:
Container for a block of EDID data from the receiver device
**/
typedef struct NEXUS_HdmiOutputEdidBlock
{
    uint8_t data[128];
} NEXUS_HdmiOutputEdidBlock;

/**
Summary:
Get a block of EDID data from the receiver device
**/
NEXUS_Error NEXUS_HdmiOutput_GetEdidBlock(
    NEXUS_HdmiOutputHandle output,
    unsigned blockNum,
    NEXUS_HdmiOutputEdidBlock *pBlock    /* [out] Block of raw EDID data */
    );

/**
Summary:
Returns the abstract NEXUS_VideoOutput connector for this Interface.
The NEXUS_VideoOutput connector is added to a Display in order to route that Display's video to the output.

Description:
Used in NEXUS_Display_AddOutput
**/
NEXUS_VideoOutputHandle NEXUS_HdmiOutput_GetVideoConnector( /* attr{shutdown=NEXUS_VideoOutput_Shutdown} */
    NEXUS_HdmiOutputHandle output
    );

/**
Summary:
Returns the abstract NEXUS_AudioOutputHandle connector for this Interface.
The NEXUS_AudioOutputHandle connector is added to an audio filter graph in order to route audio from that path to the output.

Description:
Used in NEXUS_AudioOutput_AddInput
**/
NEXUS_AudioOutputHandle NEXUS_HdmiOutput_GetAudioConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_HdmiOutputHandle output
    );


/**
Summary:
Send AVMute/AVUnMute to attached receivers to minimize video flashes, audio pops, etc.
during format changes, color space change, etc.

Description:
Use when changing video format
**/
NEXUS_Error NEXUS_HdmiOutput_SetAVMute(
    NEXUS_HdmiOutputHandle handle,
    bool mute
    );

typedef struct NEXUS_HdmiVendorSpecificInfoFrame NEXUS_HdmiOutputVendorSpecificInfoFrame;

/**
Summary:
Get current Vendor Specific Info Frame
**/
void NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputVendorSpecificInfoFrame *pVendorSpecificInfoFrame /* [out] */
    );

/**
Summary:
Set up the Vendor Specific Info Frame

Description:
Used to send Vendor Specific Information
**/
NEXUS_Error NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputVendorSpecificInfoFrame *pVendorSpecificInfoFrame
    );


/**
Summary:
Get current AVI Info Frame
**/
NEXUS_Error NEXUS_HdmiOutput_GetAviInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiAviInfoFrame *pAviInfoFrame
    );

/**
Summary:
Set up the Auxiliary Video Information(AVI) InfoFrame

Description:
Used to send AVI Information
**/
NEXUS_Error NEXUS_HdmiOutput_SetAviInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiAviInfoFrame *pAviInfoFrame
    );

/**
Summary:
Get current Audio Info Frame
**/
NEXUS_Error NEXUS_HdmiOutput_GetAudioInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiAudioInfoFrame *pAudioInfoFrame
    );

/**
Summary:
Set up the Audio InfoFrame

Description:
Used to send Audio Information
**/
NEXUS_Error NEXUS_HdmiOutput_SetAudioInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiAudioInfoFrame *pAudioInfoFrame
    );

/**
Summary:
Get current SPD (Source Product Description) Info Frame
**/
NEXUS_Error NEXUS_HdmiOutput_GetSpdInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiSpdInfoFrame *pSpdInfoFrame
    );

/**
Summary:
Set up the SPD (Source Product Description) InfoFrame

Description:
Used to send SPD (Source Product Description) Information
**/
NEXUS_Error NEXUS_HdmiOutput_SetSpdInfoFrame(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiSpdInfoFrame *pSpdInfoFrame
    );

typedef struct NEXUS_HdmiOutputTmdsSignalSettings
{
    bool clock ;
    bool data ;
} NEXUS_HdmiOutputTmdsSignalSettings ;

/**
Summary:
Enable/Disable the TMDS clock and/or data lines

Description:
This function  can independently turn the TMDS clock and data lines on and off.

If NEXUS_HdmiOutputOpenSettings.manualTmdsControl is false, this function will fail.
**/
NEXUS_Error NEXUS_HdmiOutput_SetTmdsSignal(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputTmdsSignalSettings *pSettings
    );

/**
Summary:
Get the current TMDS signal settings

Description:
This will return the last value passed into NEXUS_HdmiOutput_SetTmdsSignal
**/
void NEXUS_HdmiOutput_GetTmdsSignal(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputTmdsSignalSettings *pSettings /* [out] */
    );
/**
Summary:
Get the video information supported by the current attached HDMI receiver

Description:
This will return the CEA Video ID Codes supported by the attached HDMI TV/monitors
**/
NEXUS_Error NEXUS_HdmiOutput_GetSupportedVideoInfo(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputSupportedVideoInfo *pSupportedVideoInfo /* [out] */
    );


/**
Summary:
Get the current pre emphasis configuration
**/
NEXUS_Error NEXUS_HdmiOutput_GetPreEmphasisConfiguration(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputPreEmphasisConfiguration *pPreEmphasisConfiguration /* [out] */
    );


/**
Summary:
Set new pre emphasis configurations
**/
NEXUS_Error NEXUS_HdmiOutput_SetPreEmphasisConfiguration(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputPreEmphasisConfiguration *pPreEmphasisConfiguration
    );


/**
Summary:
HDMI Tx CRC data
**/
typedef struct NEXUS_HdmiOutputCrcData
{
    uint16_t crc;
} NEXUS_HdmiOutputCrcData;

/**
Summary:
Get HDMI Tx CRC data

Description:
You must set NEXUS_HdmiOutputSettings.crcQueueSize to a non-zero value (for example, 30) to capture data.
**/
NEXUS_Error NEXUS_HdmiOutput_GetCrcData(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputCrcData *pData, /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] array of crc data structures */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_HDMI_OUTPUT_H__ */

