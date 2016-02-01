/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
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
*   API name: Frontend 3520
*    APIs to open, close, and setup initial settings for a BCM3520
*    Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_3520_H__
#define NEXUS_FRONTEND_3520_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"

/***************************************************************************
Summary:
    Supporting devices for a BCM3520 Device
 ***************************************************************************/
typedef struct NEXUS_3520Devices
{
    NEXUS_AmplifierHandle amplifier;    /* Optional, pass NULL if no amplifier is used or if it is controlled by the demodulator itself */
    unsigned amplifierPortNumber;       /* If the amplifier is non-NULL, this indicates the port number to control on the amplifier */
    NEXUS_TunerHandle tuner;            /* Optional, pass NULL if the tuner controlled exernally */
    NEXUS_IfdHandle ifd;                /* Optional, pass NULL if no IFD is bound to this tuner (analog only) */
} NEXUS_3520Devices;

/***************************************************************************
Summary:
    Settings for a BCM3520 Device
 ***************************************************************************/
typedef struct NEXUS_3520Settings
{
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use */
    uint16_t i2cAddr;               /* Device I2C Address */
    unsigned isrNumber;             /* L1 interrupt number (typically 0..63)*/
    NEXUS_3520Devices devices;      /* Optional, set of attached devices and parameters */

    bool fastAcquisition;               /* Enable/Disable fast acquisition mode */

    bool bHead4;   /* parallel: 0=sync is 1 byte wide, 1=sync is 4 bytes wide; serial: see bSync1 */
    bool bSync1;   /* serial only: 1=sync is 1 bit wide; 0=sync is 32 bits wide if bHead4 is true else sync is 8 bits wide */
    bool bXBERT;   /* false=DS_PS_CLK runs continuously, true=DS_PS_CLK suppressed when DS_PS_SYNC active */
    bool bErrinv;  /* false=DS_PS_ERR is normal, true=DS_PS_ERR is inverted */
    bool bSyncinv; /* false=DS_PS_SYNC is normal, true=DS_PS_SYNC is inverted */
    bool bVldinv;  /* false=DS_PS_VALID is normal, true=DS_PS_VALID is inverted */
    bool bClksup;  /* false=DS_PS_CLK runs continuously, true=DS_PS_CLK suppressed when PS_VALID not active */
    bool bClkinv;  /* false=DS_PS_CLK is normal, true=DS_PS_CLK is inverted */
    bool bSerial;  /* false=serial data out, true=parallel data out */
} NEXUS_3520Settings;

/***************************************************************************
Summary:
	Get the default settings for a BCM3520 tuner
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3520Settings(
    NEXUS_3520Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
	Open a handle to a BCM3520 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3520( /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_3520Settings *pSettings
    );

/***************************************************************************
Summary:
	Results of BCM3520 device discovery
 ***************************************************************************/
typedef struct NEXUS_3520ProbeResults
{
    uint8_t revision;
} NEXUS_3520ProbeResults;

/***************************************************************************
Summary:
	Probe to see if a BCM3520 device exists with the specified settings

Description:
	Probe to see if a BCM3520 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3520(
    const NEXUS_3520Settings *pSettings,
    NEXUS_3520ProbeResults *pResults    /* [out] */
    );

/***************************************************************************
Summary:
	Get an I2C Handle for the 3520's master I2C controller.

Description:
    The BCM3520 has an on-board I2C controller that can be used to control
    tuner/amplifier devices as opposed to using a host I2C Channel.
    Please be aware, this controller has limited functionality and only
    supports the ReadNoAddr and WriteNoAddr routines.

See Also:
    NEXUS_I2c_Open
    NEXUS_I2c_ReadNoAddr
    NEXUS_I2c_WriteNoAddr
 ***************************************************************************/
NEXUS_I2cHandle NEXUS_Frontend_Get3520MasterI2c(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
	Get devices attached to 3520 device.  

Description:
    This allows the platform layer to override the initial device attachments
    specified at device open time.  This is required if any of the attached
    devices make use of the 3520 I2C Controller as opposed to the host-based
    controllers.

See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
void NEXUS_Frontend_Get3520Devices(
    NEXUS_FrontendHandle handle,
    NEXUS_3520Devices *pDevices /* [out] */
    );

/***************************************************************************
Summary:
	Set devices attached to 3520 device.  

Description:
    This allows the platform layer to override the initial device attachments
    specified at device open time.  This is required if any of the attached
    devices make use of the 3520 I2C Controller as opposed to the host-based
    controllers.

See Also:
    NEXUS_Frontend_Open3520
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set3520Devices(
    NEXUS_FrontendHandle handle,
    const NEXUS_3520Devices *pDevices
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_3520_H__ */


