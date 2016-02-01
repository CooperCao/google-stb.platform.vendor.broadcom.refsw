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
*   API name: Frontend 3128
*    APIs to open, close, and setup initial settings for a BCM3128
*    Cable Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_3128_H__
#define NEXUS_FRONTEND_3128_H__

#include "nexus_frontend.h"
#if NEXUS_HAS_GPIO
#include "nexus_gpio.h"
#endif
#include "nexus_i2c.h"
#include "nexus_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NEXUS_3128_MAX_DOWNSTREAM_CHANNELS
#define NEXUS_3128_MAX_DOWNSTREAM_CHANNELS 8
#endif
#ifndef NEXUS_3128_MAX_OUTOFBAND_CHANNELS
#define NEXUS_3128_MAX_OUTOFBAND_CHANNELS 1
#endif

/***************************************************************************
Summary:
3128 Device Handle
***************************************************************************/
typedef struct NEXUS_3128Device *NEXUS_3128DeviceHandle;

/***************************************************************************
Summary:
Settings for a BCM3128 Device Channel
***************************************************************************/
typedef enum NEXUS_3128ChannelType
{
    NEXUS_3128ChannelType_eInBand,
    NEXUS_3128ChannelType_eOutOfBand,
    NEXUS_3128ChannelType_eUpstream,
    NEXUS_3128ChannelType_eMax
} NEXUS_3128ChannelType;

/***************************************************************************
Summary:
Settings for a BCM3128 Device Channel
***************************************************************************/
typedef enum NEXUS_3128OutOfBandInput
{
    NEXUS_3128OutOfBandInput_eNarrowBandAtoD, /* out of band uses its own dedicated analog to digital converter. */
    NEXUS_3128OutOfBandInput_eWideBandAtoD,   /* out of band uses the wideband analog to digital converter. */
    NEXUS_3128OutOfBandInput_eBandPassFilteredNarrowBandAtoD, /* out of band uses its own dedicated analog to digital converter.  The signal is also band pass filtered
                                                                 to filter out frequencies <70Mhz as normal out of band range is from 70-130 Mhz. This should help save some power.*/
    NEXUS_3128OutOfBandInput_eMax
} NEXUS_3128OutOfBandInput;

/***************************************************************************
Summary:
Settings for a BCM3128 Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice3128OpenSettings
{
    NEXUS_FrontendInterruptMode interruptMode; /* Either GPIO or an L1 can be used for notification via interrupts from the frontend to the host.
                                                                                       If not, polling can be used instead to check the status of frontend. */
    NEXUS_GpioHandle gpioInterrupt; /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;             /* L1 interrupt number. (typically 0..63). See gpioInterrupt for other interrupt option. */

    /* either I2C or SPI is used for the host to control the frontend. */
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use. spiDevice should be NULL to use I2C. */
    uint16_t i2cAddr;               /* master device I2C Address */
    uint8_t i2cSlaveAddr;           /* slave device I2C Address */

    NEXUS_SpiHandle spiDevice;      /* SPI device to use. i2cDevice should be NULL to use SPI. */
    uint16_t spiAddr;               /* master device SPI Address */

    bool configureWatchdog;         /* Depending on the platform decide whether to configure watchdog or not. */
    bool inBandOpenDrain;           /* If true, inband is open drain else it is push pull. */
    bool loadAP;                    /* Load Acquisition Processor microcode when starting. */
    bool isMtsif;                   /* Determines if the transport out if configured for legacy mode or MTSIF.
                                                                   True = MTSIF, False = Legacy mode. */
    struct {
        unsigned ifFrequency;       /* IF frequency used for 3128 OOB module. */
        bool openDrain;             /* If true, out of band is open drain else it is push pull. */
        NEXUS_3128OutOfBandInput   useWidebandAtoD; /* Specify which A to D converter is used and how. See the enum for more comments. */
        NEXUS_FrontendOutOfBandNyquistFilter nyquist; /* Nyquist filter used for out of band. */
    }outOfBand;

    struct {
        int bypassable;             /* in units of 1/100 db. */
        int total;                  /* includes the bypassable and all non-bypassable fixed gains before this device. in units of 1/100 db. */
    } externalFixedGain;

    struct {
        bool enableDaisyChain;      /* enable clock to the next frontend device. */
    } crystalSettings;

    struct {
        bool enabled;               /* If true, set the pinmux registers withs values as explained below. */
        unsigned data[4];           /* pin mux data to be applied, data[n]=register, data[n+1]=value. */
    } pinmux;
} NEXUS_FrontendDevice3128OpenSettings;

/***************************************************************************
Summary:
Settings for a BCM3128 Device
***************************************************************************/
typedef struct NEXUS_Frontend3128Settings
{
    NEXUS_FrontendDeviceHandle device;
    NEXUS_3128ChannelType type;     /* Channel type to open from this device */
    unsigned channelNumber;         /* Which channel to open from this device */

    /* the following have been deprecated. see NEXUS_FrontendDevice3128OpenSettings. */
    NEXUS_GpioHandle gpioInterrupt; /* deprecated */
    unsigned isrNumber;             /* deprecated */
    NEXUS_I2cHandle i2cDevice;      /* deprecated */
    uint16_t i2cAddr;               /* deprecated */
    uint8_t i2cSlaveAddr;           /* deprecated */
    NEXUS_SpiHandle spiDevice;      /* deprecated */
    uint16_t spiAddr;               /* deprecated */
    bool configureWatchdog;         /* deprecated */
    bool inBandOpenDrain;           /* deprecated */
    bool loadAP;                    /* deprecated */
    bool isMtsif;                   /* deprecated */
    unsigned ifFrequency;           /* deprecated */
    struct {
        bool openDrain;             /* deprecated */
        bool useWidebandAtoD;        /* deprecated */
        NEXUS_FrontendOutOfBandNyquistFilter nyquist; /* deprecated */
    }outOfBand;
} NEXUS_Frontend3128Settings;

/**
The following defines the packing of NEXUS_3128ConfigSettings.agcValue:

NEXUS_PACKED_AGC_VALUE(LNACHIPID, OUTPUT1_TILT_GAIN, OUTPUT2_TILT_GAIN, GAIN_BOOST_ENABLED, SUPER_BOOST_ENABLED, TILT_ENABLED, AGCVALUE) \
    (((LNACHIPID))<<16 | \
     ((OUTPUT1_TILT_GAIN)&0x3)<<14 | \
     ((OUTPUT2_TILT_GAIN)&0x3)<<12 | \
     ((GAIN_BOOST_ENABLED)?0x1:0x0)<<10 | \
     ((SUPER_BOOST_ENABLED)?0x1:0x0)<<9 | \
     ((TILT_ENABLED)?0x1:0x0)<<8 | \
     ((AGCVALUE)&0x1F) \
    )

Field names correspond to NEXUS_3255ChannelAgcConfig naming.
**/

/***************************************************************************
Summary:
Config settings for a BCM3128 Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice3128Settings
{
    uint32_t agcValue;                     /* Gain Value: packed value which can be built using the NEXUS_PACKED_AGC_VALUE() macro */
    NEXUS_CallbackDesc updateGainCallback; /* Callback will be called when the gain from the lna needs to be updated */
    bool enableRfLoopThrough;              /* True = Enables RF loop through */
    struct {
        NEXUS_FrontendOutOfBandOutputMode outputMode;
    } outOfBand;
} NEXUS_FrontendDevice3128Settings;

/***************************************************************************
Summary:
Get the default settings for a BCM3128 tuner

Description:

See Also:
NEXUS_Frontend_Open3128
***************************************************************************/
void NEXUS_Frontend_GetDefault3128Settings(
    NEXUS_Frontend3128Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Open a handle to a BCM3128 device.

Description:
Close with NEXUS_Frontend_Close
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3128( /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_Frontend3128Settings *pSettings
    );

/***************************************************************************
Summary:
Get the default config settings to a BCM3128 device.
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault3128Settings(
    NEXUS_FrontendDevice3128Settings *pConfigSettings   /* [out] */
    );

/***************************************************************************
Summary:
Get the config settings to a BCM3128 device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Get3128Settings(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_FrontendDevice3128Settings *pConfigSettings    /* [out]  */
    );

/***************************************************************************
Summary:
Set the config settings to a BCM3128 device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Set3128Settings(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_FrontendDevice3128Settings *pConfigSettings
    );

/***************************************************************************
Summary:
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault3128OpenSettings(
    NEXUS_FrontendDevice3128OpenSettings *pSettings
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open3128(
    unsigned index,
    const NEXUS_FrontendDevice3128OpenSettings *pSettings
    );

/* deprecated */
#define NEXUS_Frontend_GetDefault3128ConfigSettings NEXUS_FrontendDevice_GetDefault3128Settings
#define NEXUS_Frontend_3128_GetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Get3128Settings(NEXUS_Frontend_GetDevice(handle), pSettings)
#define NEXUS_Frontend_3128_SetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Set3128Settings(NEXUS_Frontend_GetDevice(handle), pSettings)
typedef NEXUS_Frontend3128Settings NEXUS_3128Settings;
typedef NEXUS_FrontendDevice3128Settings NEXUS_3128ConfigSettings;

/***************************************************************************
Summary:
    Results of BCM3128 device discovery
 ***************************************************************************/
typedef struct NEXUS_3128ProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_3128ProbeResults;

/***************************************************************************
Summary:
  Probe to see if a BCM3128 device exists with the specified settings

Description:
  Probe to see if a BCM3128 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open3128
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3128(
    const NEXUS_FrontendDevice3128OpenSettings *pSettings,
    NEXUS_3128ProbeResults *pResults    /* [out] */
    );

/***************************************************************************
Summary:
    Settings for a BCM3128 tuner
 ***************************************************************************/
typedef struct NEXUS_TunerOpen3128Settings
{
    NEXUS_FrontendDeviceHandle device;
} NEXUS_TunerOpen3128Settings;

/***************************************************************************
Summary:
    Get the default settings for a BCM3128 tuner
 ***************************************************************************/
void NEXUS_Tuner_GetDefaultOpen3128Settings(
    NEXUS_TunerOpen3128Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM3128 tuner.
 ***************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_Open3128(  /* attr{destructor=NEXUS_Tuner_Close} */
    unsigned index,
    const NEXUS_TunerOpen3128Settings *pSettings
    );

/**
proposed future API's for device-level status and channel-level runtime settings:

typedef struct NEXUS_Frontend3128Status
{
    unsigned tbd;
} NEXUS_Frontend3128Status;

NEXUS_Error NEXUS_FrontendDevice_Get3128Status(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_Frontend3128Status *pStatus
    );

typedef struct NEXUS_Frontend3128RuntimeSettings
{
    unsigned tbd;
} NEXUS_Frontend3128RuntimeSettings;

void NEXUS_Frontend_Get3128RuntimeSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_Frontend3128RuntimeSettings *pSettings
    );

NEXUS_Error NEXUS_Frontend_Set3128RuntimeSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_Frontend3128RuntimeSettings *pSettings
    );
**/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_3128_H__ */


