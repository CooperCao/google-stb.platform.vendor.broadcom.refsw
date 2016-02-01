/***************************************************************************
*     (c)2004-2009 Broadcom Corporation
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
*   API name: Frontend 3440
*    APIs to open, close, and setup initial settings for a BCM3440
*    Tuner Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_TUNER_3440_H__
#define NEXUS_TUNER_3440_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"

/***************************************************************************
Summary:
    Settings for a BCM3440 Device
 ***************************************************************************/
typedef struct NEXUS_3440Settings
{
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use */
    uint16_t i2cAddr;               /* Device I2C Address */
} NEXUS_3440Settings;

/***************************************************************************
Summary:
	Get the default settings for a BCM3440 tuner
 ***************************************************************************/
void NEXUS_Tuner_GetDefault3440Settings(
    NEXUS_3440Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
	Open a handle to a BCM3440 device.
 ***************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_Open3440(  /* attr{destructor=NEXUS_Tuner_Close} */
    const NEXUS_3440Settings *pSettings
    );

/***************************************************************************
Summary:
	Set the symbol rate for a 3440 tuner device

Description:
    Currently, it's expected that this function will be called every time 
    after the tuner frqeuency is changed.  The IF offset and internal filters
    will be setup only when this routine has been called.
 ***************************************************************************/
NEXUS_Error NEXUS_Tuner_Set3440SymbolRate(
    NEXUS_TunerHandle handle,
    uint32_t symbolRate,
    bool nyquist20          /* If true, the nyquist filter is 0.20.  If false, 0.35 is assumed. */
    );

/***************************************************************************
Summary:
	Get the IF Offset for a 3440 tuner device.  The frequency and symbol
    rate must be set prior to calling this routine. 
 ***************************************************************************/
void NEXUS_Tuner_Get3440IfOffset(
    NEXUS_TunerHandle handle,
    int32_t *pIfOffset
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_TUNER_3440_H__ */


