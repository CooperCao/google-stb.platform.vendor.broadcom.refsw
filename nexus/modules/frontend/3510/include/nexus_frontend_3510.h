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
*   API name: Frontend 3510
*    APIs to open, close, and setup initial settings for a BCM3510
*    Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_3510_H__
#define NEXUS_FRONTEND_3510_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"

/***************************************************************************
Summary:
    Supporting devices for a BCM3510 Device
 ***************************************************************************/
typedef struct NEXUS_3510Devices
{
    NEXUS_AmplifierHandle amplifier;    /* Optional, pass NULL if no amplifier is used or if it is controlled by the demodulator itself */
    unsigned amplifierPortNumber;       /* If the amplifier is non-NULL, this indicates the port number to control on the amplifier */
    NEXUS_TunerHandle tuner;            /* Optional, pass NULL if the tuner controlled exernally */
    NEXUS_IfdHandle ifd;                /* Optional, pass NULL if no IFD is bound to this tuner (analog only) */
} NEXUS_3510Devices;

/***************************************************************************
Summary:
    Settings for a BCM3510 Device
 ***************************************************************************/
typedef struct NEXUS_3510Settings
{
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use */
    uint16_t i2cAddr;               /* Device I2C Address */
    unsigned isrNumber;             /* L1 interrupt number (typically 0..63)*/
    NEXUS_3510Devices devices;      /* Optional, set of attached devices and parameters */
} NEXUS_3510Settings;

/***************************************************************************
Summary:
	Get the default settings for a BCM3510 tuner
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3510Settings(
    NEXUS_3510Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
	Open a handle to a BCM3510 device.
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3510( /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_3510Settings *pSettings
    );

/***************************************************************************
Summary:
	Results of BCM3510 device discovery
 ***************************************************************************/
typedef struct NEXUS_3510ProbeResults
{
    uint8_t revision;
} NEXUS_3510ProbeResults;

/***************************************************************************
Summary:
	Probe to see if a BCM3510 device exists with the specified settings

Description:
	Probe to see if a BCM3510 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open3510
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3510(
    const NEXUS_3510Settings *pSettings,
    NEXUS_3510ProbeResults *pResults    /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_3510_H__ */


