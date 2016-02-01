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
*   API name: Frontend 45216
*    APIs to open, close, and setup initial settings for a BCM45216 Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_45216_H__
#define NEXUS_FRONTEND_45216_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"
#include "nexus_spi.h"

#ifndef NEXUS_45216_MAX_FRONTEND_CHANNELS
#define NEXUS_45216_MAX_FRONTEND_CHANNELS 16
#endif

/***************************************************************************
Summary:
45216 Device Handle
***************************************************************************/
typedef struct NEXUS_45216Device *NEXUS_45216DeviceHandle;

/***************************************************************************
Summary:
    Settings for a BCM45216 Device

Description:
    This structure provides the settings when opening a 45216 device. (A FrontendDeviceHandle
maps to the frontend chip, a FrontendHandle maps to a specific demod path.)
    In a typical external frontend configuration, the frontend uses an interrupt path to
alert the backend. This will usually be either an external interrupt (specified by
isrNumber), or as a GPIO Interrupt (specified via gpioInterrupt -- the handle is opened
by the caller, not by NEXUS_FrontendDevice_Open45216).
    An external frontend will require a communication path, requiring either i2cDevice and
i2cAddr to be specified, or spiDevice and spiAddr to be specified.  The I2C or SPI handle
needs to be opened by the caller.
    For the 45216, there are additional configuration options for the Diseqc configuration,
allowing an alternate i2cAddr and i2cDevice to be specified specifically for diseqc.  The
pinmux for diseqc configuration may also be specified by setting setPinmux to true and
providing the alternate pinmux values.
    On the 45216, mtsif[0] maps to MTSIF0 TX configuration, and mtsif[1] maps to MTSIF1 TX.
Both mtsif[0].clockRate and mtsif[1].clockRate can be set, and both mtsif[0].driveStrength and
mtsif[1].driveStrength can be set to 2, 4, 6, 8, 10, 12, 14, or 16.  If mtsif[x].enabled is set
to false (it will be set to true by default), the clock of that MTSIFx TX will be turned off,
and if both are disabled (by setting both mtsif[0].enabled and mtsif[1].enabled to false),
the master MTSIF TX clock will be turned off.

See Also:
    NEXUS_FrontendDevice_Open45216
 ***************************************************************************/
typedef struct NEXUS_FrontendDevice45216OpenSettings
{
    /* either GPIO or an L1 is used for notification from the frontend to the host. */
    NEXUS_FrontendDeviceReset reset;    /* Information required for controlling the GPIO reset for S3, if necessary. */
    NEXUS_GpioHandle gpioInterrupt;     /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;                 /* L1 interrupt number. (typically 0..63). See gpioInterrupt for other interrupt option. */

    /* Either SPI or I2C is used for the host to control the frontend chip. */
    NEXUS_I2cHandle i2cDevice;          /* I2C device to use. spiDevice should be NULL to use I2C. */
    uint16_t i2cAddr;                   /* master device I2C Address */
    uint8_t i2cSlaveAddr;               /* slave device I2C Address */

    NEXUS_SpiHandle spiDevice;          /* SPI device to use. i2cDevice should be NULL to use SPI. */
    uint16_t spiAddr;                   /* master device SPI Address */

    NEXUS_FrontendDeviceMtsifSettings mtsif[NEXUS_MAX_MTSIF]; /* Configure MTSIF rate and drive strength at open time. If values are 0, defaults are used. */

    struct {
        uint16_t i2cAddr;               /* I2C address to communicate with diseqc */
        NEXUS_I2cHandle i2cDevice;      /* I2C device to communicate with diseqc */
    } diseqc;
} NEXUS_FrontendDevice45216OpenSettings;

/***************************************************************************
Summary:
    Get default settings for a BCM45216 frontend device

Description:
See Also:
    NEXUS_FrontendDevice45216OpenSettings,
    NEXUS_FrontendDevice_Open45216
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault45216OpenSettings(
    NEXUS_FrontendDevice45216OpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Open a handle to a BCM45216 device

Description:
    This function opens a BCM45216 device and returns a handle.  The device maps to
the chip, and each chip only needs to be opened once.  It will then be used to open
the individual channels (demods) via NEXUS_Frontend_Open45216.
    index should be 0.

See Also:
    NEXUS_FrontendDevice45216OpenSettings
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open45216(
    unsigned index,
    const NEXUS_FrontendDevice45216OpenSettings *pSettings
    );

/***************************************************************************
Summary:
    Settings for a BCM45216 Channel
 ***************************************************************************/
typedef struct NEXUS_45216FrontendSettings
{
    NEXUS_FrontendDeviceHandle device;  /* Previously opened device to use */
    unsigned channelNumber;             /* Which channel to open from this device */
} NEXUS_45216FrontendSettings;

/***************************************************************************
Summary:
    Get the default settings for a BCM45216 frontend

Description:
See Also:
    NEXUS_Frontend_Open45216
 ***************************************************************************/
void NEXUS_Frontend_GetDefault45216Settings(
    NEXUS_45216FrontendSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM45216 device.


Description:
    This function opens a channel on a BCM45216 device and returns a frontend handle.
The channel maps to an individual demod of the device (which contains more than one
demod).  The total number of channels (demods) available can be retrieved with
NEXUS_FrontendDevice_GetSatelliteCapabilities.

See Also:
    NEXUS_Frontend_Close45216
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open45216(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_45216FrontendSettings *pSettings
    );

/***************************************************************************
Summary:
    Structure to hold the results of a 45216 probe call.
***************************************************************************/
typedef struct NEXUS_45216ProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_45216ProbeResults;


/***************************************************************************
Summary:
  Probe to see if a BCM45216 device exists with the specified settings

Description:
  Probe to see if a BCM45216 device exists with the specified settings

See Also:
    NEXUS_FrontendDevice_Open45216
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe45216(
    const NEXUS_FrontendDevice45216OpenSettings *pSettings,
    NEXUS_45216ProbeResults *pResults    /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_45216_H__ */
