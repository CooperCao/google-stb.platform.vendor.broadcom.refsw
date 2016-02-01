/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
*   API name: Frontend 4501
*    APIs to open, close, and setup initial settings for a BCM4501
*    Dual-Channel Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_4501_H__
#define NEXUS_FRONTEND_4501_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"

/***************************************************************************
Summary:
    Settings for a BCM4501 Device
 ***************************************************************************/
typedef struct NEXUS_4501Settings
{
    /* The following parameters identify the device to be used. */
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use */
    uint16_t i2cAddr;               /* Device I2C Address */
    unsigned isrNumber;             /* L1 interrupt number (typically 0..63)*/

    /* The following parameters identify the channel of the device */
    unsigned channelNumber;         /* Which channel from the device will be opened */

    bool bitWideSync;
} NEXUS_4501Settings;

/***************************************************************************
Summary:
    Get the default settings for a BCM4501 tuner

Description:
See Also:
    NEXUS_Frontend_Open4501
 ***************************************************************************/
void NEXUS_Frontend_GetDefault4501Settings(
    NEXUS_4501Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM4501 device.

Description:
See Also:
    NEXUS_Frontend_Close4501
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open4501(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_4501Settings *pSettings
    );

/***************************************************************************
Summary:
    Results of BCM4501 device discovery
 ***************************************************************************/
typedef struct NEXUS_4501ProbeResults
{
    uint8_t revision;
} NEXUS_4501ProbeResults;

/***************************************************************************
Summary:
    Probe to see if a BCM4501 device exists with the specified settings

Description:
See Also:
    NEXUS_Frontend_Open4501
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe4501(
    const NEXUS_4501Settings *pSettings,
    NEXUS_4501ProbeResults *pResults        /* [out] */
    );

/***************************************************************************
Summary:
    Get an I2C Handle for the 4501's master I2C controller.

Description:
    The BCM4501 has an on-board I2C controller that can be used to control
    tuner/amplifier devices as opposed to using a host I2C Channel.
    Please be aware, this controller has limited functionality and only
    supports the ReadNoAddr and WriteNoAddr routines.

See Also:
    NEXUS_I2c_Open
    NEXUS_I2c_ReadNoAddr
    NEXUS_I2c_WriteNoAddr
 ***************************************************************************/
NEXUS_I2cHandle NEXUS_Frontend_Get4501MasterI2c(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Write4501HostRegister(
    NEXUS_FrontendHandle handle,
    uint8_t address,
    uint8_t data
    );

/***************************************************************************
Summary:
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Read4501HostRegister(
    NEXUS_FrontendHandle handle,
    uint8_t address,
    uint8_t *pData /* [out] */
    );

/***************************************************************************
Summary:
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Read4501Memory(
    NEXUS_FrontendHandle handle,
    uint16_t addr,       /* starting address */
    uint8_t *buffer,     /* attr{nelem=bufferSize} [out] buffer into which data is read */
    uint16_t bufferSize  /* size of buffer and number of bytes to read */
    );

/***************************************************************************
Summary:
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Write4501Memory(
    NEXUS_FrontendHandle handle,
    uint16_t addr,       /* starting address */
    const uint8_t *buffer, /* attr{nelem=bufferSize} buffer from which data is written */
    uint16_t bufferSize  /* size of buffer and number of bytes to write */
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_4501_H__ */

