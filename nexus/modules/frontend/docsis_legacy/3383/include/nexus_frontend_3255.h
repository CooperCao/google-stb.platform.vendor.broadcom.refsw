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
*   API name: Frontend 3255
*    APIs to open, close, and setup initial settings for a BCM3255
*    Cable Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_3255_H__
#define NEXUS_FRONTEND_3255_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_gpio.h"

/***************************************************************************
Summary:
Docsis device Handle
***************************************************************************/
typedef struct NEXUS_3255Device *NEXUS_3255DeviceHandle;


/***************************************************************************
Summary:
Enumerations for docsis channel types
***************************************************************************/
typedef enum NEXUS_3255ChannelType
{
    NEXUS_3255ChannelType_eInBand,
    NEXUS_3255ChannelType_eOutOfBand,
    NEXUS_3255ChannelType_eDocsis,
    NEXUS_3255ChannelType_eMax
} NEXUS_3255ChannelType;

/***************************************************************************
Summary:
Capabilities for a docsis device.
***************************************************************************/
typedef struct NEXUS_3255DeviceCapabilities
{
    unsigned numOfDocsisChannels; /* docsis channels */
    unsigned numOfQamChannels;    /* QAM Video channels */
    bool isOobChannelSupported;   /* Is OOB channel available */ 
    unsigned totalChannels;       /* total number of Docsis + QAM + OOB channels */
    unsigned numOfTsmfParsers;    /* number of TSMF parsers on DOCSIS device */
}NEXUS_3255DeviceCapabilities;

/***************************************************************************
Summary:
Capabilities for a docsis channel
***************************************************************************/
typedef struct NEXUS_3255ChannelCapabilities
{
    NEXUS_3255ChannelType channelType;
}NEXUS_3255ChannelCapabilities;

/***************************************************************************
Summary:
Settings for a docsis device
***************************************************************************/
typedef struct NEXUS_3255DeviceSettings
{
    uint32_t chipID;  /* Docsis chip ID */
    uint32_t revID;   /* Docsis revision ID */ 
    bool enableFEC;   /* Docsis Channels will do FEC or not */
    bool fastAcquire; /* Docsis Channels'fast acquire enabled */
    bool autoAcquire; /* Docsis Channels auto acquire enabled */
    bool mtsif;       /* Are docsis channels connected to the host via MTSIF interface? */ 
    unsigned rpcTimeout;
} NEXUS_3255DeviceSettings;


/***************************************************************************
Summary:
Settings for a docsis channel
***************************************************************************/
typedef struct NEXUS_3255ChannelSettings
{
    unsigned channelNumber;               /* The channel index to open from this device */
    bool enableTsmfSupport;               /* enable tsmf parsing for a QAM video channel */ 
    NEXUS_CallbackDesc docsisStateChange; /* callback when docsis device status changes */
} NEXUS_3255ChannelSettings;


/***************************************************************************
Summary:
Types for a docsis device GPIO pin
***************************************************************************/
typedef enum NEXUS_3255DeviceGpioPin
{
    NEXUS_3255DeviceGpioPin_eOob,
    NEXUS_3255DeviceGpioPin_eMax
} NEXUS_3255DeviceGpioPin;

/***************************************************************************
Summary:
GPIO Settings for docsis device
***************************************************************************/
typedef struct NEXUS_3255DeviceGpioPinSettings
{
    NEXUS_GpioMode mode;
    NEXUS_GpioValue value;
} NEXUS_3255DeviceGpioPinSettings;

/***************************************************************************
Summary:
Docsis channel S-CDMA status
***************************************************************************/
typedef enum NEXUS_3255ChannelScdmaType
{
    NEXUS_3255ChannelScdmaType_eNo,
    NEXUS_3255ChannelScdmaType_eTcm,
    NEXUS_3255ChannelScdmaType_eTdma,
    NEXUS_3255ChannelScdmaType_eMax
} NEXUS_3255ChannelScdmaType;

/***************************************************************************
Summary:
Docsis Channel Upstream Modulation Type
***************************************************************************/
typedef enum NEXUS_3255ChannelUsModType
{
    NEXUS_3255ChannelUsModType_eQpsk,
    NEXUS_3255ChannelUsModType_eQam16,
    NEXUS_3255ChannelUsModType_eQam32,
    NEXUS_3255ChannelUsModType_eQam64,
    NEXUS_3255ChannelUsModType_eQam128,
    NEXUS_3255ChannelUsModType_eQam256,
    NEXUS_3255ChannelUsModType_eQam512,
    NEXUS_3255ChannelUsModType_eMax
} NEXUS_3255ChannelUsModType;

/***************************************************************************
Summary:
Docsis device Status
***************************************************************************/
typedef enum NEXUS_3255DeviceStatus
{
    NEXUS_3255DeviceStatus_eUninitialized,
    NEXUS_3255DeviceStatus_eOperational,
    NEXUS_3255DeviceStatus_eReset,
    NEXUS_3255DeviceStatus_eFailed,
    NEXUS_3255DeviceStatus_eMax
} NEXUS_3255DeviceStatus;

/***************************************************************************
Summary:
Docsis channel status
***************************************************************************/
typedef struct NEXUS_DocsisChannelStatus
{
    NEXUS_3255DeviceStatus deviceStatus; /* Docsis current status */
    int downstreamCenterFreq;            /* Docsis channel DS frequency in Hz */
    int downstreamPowerLevel;            /* Docsis channel DS power in tenths of dBmV */
    bool downstreamCarrierLock;          /* Docsis channel DS lock status */
	unsigned snrEstimate;       		 /* Docsis channel DS primary channel SNR estimate in 1/256 dB */
    NEXUS_3255ChannelScdmaType channelScdmaStatus; /* Docsis channel DS S-CDMA status */
    NEXUS_3255ChannelUsModType upstreamModType;    /* US modulation type */
    int upstreamXmtCenterFreq;           /* US tramsmition frequency in Hz */
    int upstreamPowerLevel;              /* US power in dBmV */
    int upStreamSymbolrate;              /* US Symbol rate */
    char macAddress[6];                  /* eCM Mac address */
    char ipv4Address[4];                 /* eCM IPv4 address */
    char ipv6Address[16];                /* eCM IPv6 address */
    unsigned lastKnownGoodFreq;          /* Last frequency that was locked for DOCSIS. units in Hz. 
                                            Stored in non-volatile memory of CM.
                                            For bonded channels, this is the center freq. */
} NEXUS_DocsisChannelStatus;

/***************************************************************************
Summary:
Get the 3255 Device Handle
***************************************************************************/
NEXUS_3255DeviceHandle NEXUS_Frontend_Get3255DeviceHandle(
    NEXUS_FrontendHandle handle
    );


/***************************************************************************
Summary:
Get the bit mask for the lock status of the 3128 channels 
***************************************************************************/
void NEXUS_Frontend_Get3255LockStatusForHostChannels(
    NEXUS_3255DeviceHandle handle,
    uint32_t *pLockStatus /* return what was set with NEXUS_Frontend_Set3255LockStatusForHostChannels */
    );

/***************************************************************************
Summary:
Send the bit mask of the lock status to the Docsis frontend over RPC.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set3255LockStatusForHostChannels(
    NEXUS_3255DeviceHandle handle,
    uint32_t lockStatus /* bit mask for the lock status of 3128 channels. this will tell 3255 the status of 3128 channels. */
    );

/***************************************************************************
Summary:
Get the default settings for a Docsis channel
***************************************************************************/
void NEXUS_Frontend_GetDefault3255ChannelSettings(
    NEXUS_3255ChannelSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Open a docsis channel and associate it with a frontend Handle.
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3255Channel( /* attr{destructor=NEXUS_Frontend_Close} */
    NEXUS_3255DeviceHandle deviceHandle,
    const NEXUS_3255ChannelSettings *pSettings
    );

/***************************************************************************
Summary:
Get the current settings of a docsis device's GPIO pins
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Get3255DeviceGpioPinSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255DeviceGpioPin pin,
    NEXUS_3255DeviceGpioPinSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the current settings of a docsis device's GPIO pins
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set3255DeviceGpioPinSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255DeviceGpioPin pin,
    const NEXUS_3255DeviceGpioPinSettings *pSettings
    );

/***************************************************************************
Summary:
Get the current status of a docsis channel for any docsisChannel index
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDocsisChannelStatus(
    NEXUS_FrontendHandle handle,
    unsigned docsisChannel,
    NEXUS_DocsisChannelStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Legacy mapping for backward compatibility
***************************************************************************/
typedef struct NEXUS_DocsisChannelStatus NEXUS_3255ChannelStatus;
#define NEXUS_Frontend_Get3255ChannelStatus(handle,status) NEXUS_Frontend_GetDocsisChannelStatus(handle,0,status)


/***************************************************************************
Summary:
Get the current settings for a docsis channel
***************************************************************************/
void NEXUS_Frontend_Get3255ChannelSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255ChannelSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the settings for a docsis channel.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set3255ChannelSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_3255ChannelSettings *pSettings
    );


/***************************************************************************
Summary:
AGC configuration per docsis channel
***************************************************************************/
typedef struct NEXUS_3255ChannelAgcConfig
{
    unsigned lnaChipId;
    unsigned agcValue;        /* AGC 5 bit value. */
    unsigned output1TiltGain; /* LNA Output1 Stage2 Tilt gain */
    unsigned output2TiltGain; /* LNA Output2 Stage2 Tilt gain */
    bool gainBoostEnabled;   /* True if Stage 1 gain boost is enabled */
    bool superBoostEnabled;   /* True if Stage1 super boost is enabled */
    bool tiltEnabled;        /* True if Stage 1 tilt is enabled  */
} NEXUS_3255ChannelAgcConfig;

/***************************************************************************
Summary:
Get agcGain configuration from the docsis controlled LNA
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Get3255ChannelAgcConfig(
    NEXUS_FrontendHandle handle,
    NEXUS_3255ChannelAgcConfig *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Get the default docsis device configurations.
***************************************************************************/
void NEXUS_Frontend_GetDefault3255DeviceSettings(
    NEXUS_3255DeviceSettings *pSettings);

/***************************************************************************
Summary:
Initialize a docsis device..
***************************************************************************/
NEXUS_3255DeviceHandle NEXUS_Frontend_Open3255Device(
    unsigned index,
    const NEXUS_3255DeviceSettings *pSettings
    );

/***************************************************************************
Summary:
De-initialize a docsis device
***************************************************************************/
void NEXUS_Frontend_Close3255Device(
    NEXUS_3255DeviceHandle handle
    );

/***************************************************************************
Summary:
Get the docsis device capabilities
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Get3255DeviceCapabilities(
    NEXUS_3255DeviceHandle handle,
    NEXUS_3255DeviceCapabilities *pCapabilities
    );

/***************************************************************************
Summary:
Get the docsis channel capabilities
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Get3255ChannelCapabilities(
    NEXUS_3255DeviceHandle handle,
    unsigned channel,
    NEXUS_3255ChannelCapabilities *pCapabilities
    );

/***************************************************************************
Summary:
Initiates a manual LNA reconfiguration which consists of a CPPM calculation.  
An LNA reconfiguration is automatically performed by the 3383 with every 20th tune 
attempt on the condition that the 20th tune attempt fails.  This API provides 
a mechanism to additionally perform this reconfiguration at any arbitrary point.  
However, the auto reconfiguration can be turned off by completely by calling 
NEXUS_Frontend_Set3255LockStatusForHostChannels() and passing in a
non-zero "lockStatus" value.  It can be turned on again by calling this same
function with a value of zero.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Config3255Lna(
    NEXUS_3255DeviceHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_3255_H__ */

