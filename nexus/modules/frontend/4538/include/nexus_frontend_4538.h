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
*   API name: Frontend 4538
*    APIs to open, close, and setup initial settings for a BCM4538
*    Dual-Channel Integrated Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_4538_H__
#define NEXUS_FRONTEND_4538_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"
#include "nexus_spi.h"

#define NEXUS_4538_MAX_FRONTEND_CHANNELS 8

/***************************************************************************
Summary:
4538 Device Handle
***************************************************************************/
typedef struct NEXUS_4538Device *NEXUS_4538DeviceHandle;

/***************************************************************************
Summary:
    Settings for a BCM4538 Device

Description:
    This structure provides the settings when opening a 4538 device. (A FrontendDeviceHandle
maps to the frontend chip, a FrontendHandle maps to a specific demod path.)
    In a typical external frontend configuration, the frontend uses an interrupt path to
alert the backend. This will usually be either an external interrupt (specified by
isrNumber), or as a GPIO Interrupt (specified via gpioInterrupt -- the handle is opened
by the caller, not by NEXUS_FrontendDevice_Open4538).
    An external frontend will require a communication path, requiring either i2cDevice and
i2cAddr to be specified, or spiDevice and spiAddr to be specified.  The I2C or SPI handle
needs to be opened by the caller.
    For the 4538, there are additional configuration options for the Diseqc configuration,
allowing an alternate i2cAddr and i2cDevice to be specified specifically for diseqc.  The
pinmux for diseqc configuration may also be specified by setting setPinmux to true and
providing the alternate pinmux values.
    On the 4538, mtsif[0] maps to MTSIF0 configuration.  mtsif[0].clockRate can be set,
but mtsif[0].driveStrength cannot.

See Also:
    NEXUS_FrontendDevice_Open4538
 ***************************************************************************/
typedef struct NEXUS_FrontendDevice4538OpenSettings
{
    /* either GPIO or an L1 is used for notification from the frontend to the host. */
    NEXUS_FrontendDeviceReset reset;    /* Information required for controlling the GPIO reset for S3, if necessary. */
    NEXUS_GpioHandle gpioInterrupt;     /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;                 /* L1 interrupt number. (typically 0..63). See gpioInterrupt for other interrupt option. */

    /* Either SPI or I2C is used for the host to control the frontend chip. */
    NEXUS_I2cHandle i2cDevice;          /* I2C device to use. spiDevice should be NULL to use I2C. diseqcI2cDevice should be set only if it differs from this. */
    uint16_t i2cAddr;                   /* master device I2C Address */
    uint8_t i2cSlaveAddr;               /* slave device I2C Address */

    NEXUS_SpiHandle spiDevice;          /* SPI device to use. i2cDevice should be NULL to use SPI. */
    uint16_t spiAddr;                   /* master device SPI Address */

    struct {
        uint16_t i2cAddr;               /* I2C address to communicate with diseqc */
        NEXUS_I2cHandle i2cDevice;      /* I2C device to communicate with diseqc */

        /* 4538-specific diseqc pinmux override. */
        bool setPinmux;                 /* If setPinmux is true, the values in diseqc.pinmux will be written, overriding the default diseqc pinmux */
        uint8_t pinmux[2];              /* If the default diseqc pinmux needs to be overridden for a specific configuration, the values for this may be taken from the BAST_4538_DSEC_PIN_MUX_* macros in bast_4538.h */
    } diseqc;

    NEXUS_FrontendDeviceMtsifSettings mtsif[NEXUS_MAX_MTSIF]; /* Configure MTSIF rate and drive strength at open time. If values are 0, defaults are used. */
} NEXUS_FrontendDevice4538OpenSettings;

/***************************************************************************
Summary:
    Settings for a BCM4538 Channel

Description:
See Also:
    NEXUS_FrontendDevice4538OpenSettings,
    NEXUS_FrontendDevice_Open4538,
    NEXUS_Frontend_Open4538
 ***************************************************************************/
typedef struct NEXUS_4538Settings
{
    NEXUS_FrontendDeviceHandle device;  /* Previously opened device to use */
    unsigned channelNumber;             /* Which channel to open from this device */

    /* Settings below this are deprecated and only used if device is NULL */
    NEXUS_GpioHandle gpioInterrupt;     /* Deprecated. */
    unsigned isrNumber;                 /* Deprecated. */
    NEXUS_I2cHandle i2cDevice;          /* Deprecated. */
    uint16_t i2cAddr;                   /* Deprecated. */
    uint8_t i2cSlaveAddr;               /* Deprecated. */
} NEXUS_4538Settings;

/***************************************************************************
Summary:
    Get default settings for a BCM4538 frontend device

Description:
See Also:
    NEXUS_FrontendDevice4538OpenSettings,
    NEXUS_FrontendDevice_Open4538
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault4538OpenSettings(
    NEXUS_FrontendDevice4538OpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Open a handle to a BCM4538 device

Description:
    This function opens a BCM4538 device and returns a handle.  The device maps to
the chip, and each chip only needs to be opened once.  It will then be used to open
the individual channels (demods) via NEXUS_Frontend_Open4538.
    index should be 0.

See Also:
    NEXUS_FrontendDevice4538OpenSettings
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open4538(
    unsigned index,
    const NEXUS_FrontendDevice4538OpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the default settings for a BCM4538 frontend channel

Description:
See Also:
    NEXUS_Frontend_Open4538
 ***************************************************************************/
void NEXUS_Frontend_GetDefault4538Settings(
    NEXUS_4538Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM4538 channel.

Description:
    This function opens a channel on a BCM4538 device and returns a frontend handle.
The channel maps to an individual demod of the device (which contains more than one
demod).  The total number of channels (demods) available can be retrieved with
NEXUS_FrontendDevice_GetSatelliteCapabilities.

See Also:
    NEXUS_Frontend_Close,
    NEXUS_FrontendDevice_GetSatelliteCapabilities
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open4538(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_4538Settings *pSettings
    );

/* Backward compatibility macros */
typedef struct NEXUS_FrontendSatelliteCapabilities NEXUS_Frontend4538Capabilities;
#define NEXUS_FrontendDevice_Get4538Capabilities NEXUS_FrontendDevice_GetSatelliteCapabilities

typedef struct NEXUS_FrontendSatelliteRuntimeSettings NEXUS_Frontend4538RuntimeSettings;
#define NEXUS_Frontend_Get4538RuntimeSettings NEXUS_Frontend_GetSatelliteRuntimeSettings
#define NEXUS_Frontend_Set4538RuntimeSettings NEXUS_Frontend_SetSatelliteRuntimeSettings

/***************************************************************************
Summary:
***************************************************************************/
typedef struct NEXUS_4538ProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_4538ProbeResults;


/***************************************************************************
Summary:
  Probe to see if a BCM4538 device exists with the specified settings

Description:
  Probe to see if a BCM4538 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open4538
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe4538(
    const NEXUS_FrontendDevice4538OpenSettings *pSettings,
    NEXUS_4538ProbeResults *pResults    /* [out] */
    );

/*
 * Power Estimation for 4538:
 *
 * The values returned in NEXUS_FrontendSatelliteAgcStatus are from
 * BAST_4538_AgcStatus:
 *
 * [0]: BAST_4538_AgcStatus.lnaGain
 * [1]: BAST_4538_AgcStatus.chanAgc
 * [2]: BAST_4538_AgcStatus.tunerFreq
 * [3]: BAST_4538_AgcStatus.adcSelect
 * [4]: Unused (valid=false always)
 */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_4538_H__ */

