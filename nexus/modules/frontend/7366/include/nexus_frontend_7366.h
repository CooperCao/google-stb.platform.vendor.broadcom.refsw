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
*   API name: Frontend 7366
*    APIs to open, close, and setup initial settings for a BCM7366
*    Dual-Channel Integrated Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_7366_H__
#define NEXUS_FRONTEND_7366_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"
#include "nexus_spi.h"

#ifndef NEXUS_7366_MAX_FRONTEND_CHANNELS
#define NEXUS_7366_MAX_FRONTEND_CHANNELS 8
#endif

/***************************************************************************
Summary:
7366 Device Handle
***************************************************************************/
typedef struct NEXUS_7366Device *NEXUS_7366DeviceHandle;

/***************************************************************************
Summary:
    Settings for a BCM7366 Device

Description:
    This structure provides the settings when opening a 7366 device. (A FrontendDeviceHandle
maps to the frontend chip, a FrontendHandle maps to a specific demod path.)
    For the 7366, there are configuration options for communication with diseqc,
allowing an i2cAddr and i2cDevice to be specified.
    When used as an external frontend, the frontend uses an interrupt path to
alert the backend. This will usually be either an external interrupt (specified by
isrNumber), or as a GPIO Interrupt (specified via gpioInterrupt -- the handle is opened
by the caller, not by NEXUS_FrontendDevice_Open7366).
    An external frontend will require a communication path, requiring either i2cDevice and
i2cAddr to be specified, or spiDevice and spiAddr to be specified.  The I2C or SPI handle
needs to be opened by the caller.
    On the 7366, mtsif[x] should not be touched when used as an internal frontend.  When used
as an external frontend, mtsif[0].clockRate and mtsif[0].driveStrength should be used to control
MTSIF settings.  mtsif[0].enabled is ignored.

See Also:
    NEXUS_FrontendDevice_Open7366
 ***************************************************************************/
typedef struct NEXUS_FrontendDevice7366OpenSettings
{
/* General 7366 frontend settings */
    struct {
        uint16_t i2cAddr;               /* I2C address to communicate with diseqc */
        NEXUS_I2cHandle i2cDevice;      /* I2C device to communicate with diseqc */
    } diseqc;

/* When used as an external frontend, e.g. 4548, the following settings are required.
 * Do not set them unless the 7366 is being used as an external frontend with a different backend. */

    /* either GPIO or an L1 is used for notification from the frontend to the host. */
    NEXUS_GpioHandle gpioInterrupt;     /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;                 /* L1 interrupt number. (typically 0..63). See gpioInterrupt for other interrupt option. */

    /* Either SPI or I2C is used for the host to control the frontend chip. */
    NEXUS_I2cHandle i2cDevice;          /* I2C device to use. spiDevice should be NULL to use I2C. */
    uint16_t i2cAddr;                   /* master device I2C Address */
    uint8_t i2cSlaveAddr;               /* slave device I2C Address */

    NEXUS_SpiHandle spiDevice;          /* SPI device to use. i2cDevice should be NULL to use SPI. */
    uint16_t spiAddr;                   /* master device SPI Address */

    NEXUS_FrontendDeviceMtsifSettings mtsif[NEXUS_MAX_MTSIF]; /* Configure MTSIF rate and drive strength at open time. If values are 0, defaults are used. */
/* End of settings to use 7366 as external frontend. */

} NEXUS_FrontendDevice7366OpenSettings;

/***************************************************************************
Summary:
    Get default settings for a BCM7366 frontend device

Description:
See Also:
    NEXUS_FrontendDevice7366OpenSettings,
    NEXUS_FrontendDevice_Open7366
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault7366OpenSettings(
    NEXUS_FrontendDevice7366OpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Open a handle to a BCM7366 device

Description:
    This function opens a BCM7366 device and returns a handle.  The device maps to
the chip, and each chip only needs to be opened once.  It will then be used to open
the individual channels (demods) via NEXUS_Frontend_Open7366.
    index should be 0.

See Also:
    NEXUS_FrontendDevice7366OpenSettings
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open7366(
    unsigned index,
    const NEXUS_FrontendDevice7366OpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Settings for a BCM7366 Channel
 ***************************************************************************/
typedef struct NEXUS_7366FrontendSettings
{
    NEXUS_FrontendDeviceHandle device;  /* Previously opened device to use */
    unsigned channelNumber;             /* Which channel to open from this device */
} NEXUS_7366FrontendSettings;

/***************************************************************************
Summary:
    Get the default settings for a BCM7366 frontend

Description:
See Also:
    NEXUS_Frontend_Open7366
 ***************************************************************************/
void NEXUS_Frontend_GetDefault7366Settings(
    NEXUS_7366FrontendSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM7366 device.


Description:
    This function opens a channel on a BCM7366 device and returns a frontend handle.
The channel maps to an individual demod of the device (which contains more than one
demod).  The total number of channels (demods) available can be retrieved with
NEXUS_FrontendDevice_GetSatelliteCapabilities.

See Also:
    NEXUS_Frontend_Close7366
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7366(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_7366FrontendSettings *pSettings
    );

/* Backward compatibility macros */
typedef struct NEXUS_FrontendSatelliteCapabilities NEXUS_Frontend7366Capabilities;
#define NEXUS_FrontendDevice_Get7366Capabilities NEXUS_FrontendDevice_GetSatelliteCapabilities

typedef struct NEXUS_FrontendSatelliteRuntimeSettings NEXUS_Frontend7366RuntimeSettings;
#define NEXUS_Frontend_Get7366RuntimeSettings NEXUS_Frontend_GetSatelliteRuntimeSettings
#define NEXUS_Frontend_Set7366RuntimeSettings NEXUS_Frontend_SetSatelliteRuntimeSettings

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_7366_H__ */

