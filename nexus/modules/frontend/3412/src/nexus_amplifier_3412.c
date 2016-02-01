/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: Frontend 3412
*    APIs to open, close, and setup initial settings for a BCM3412 
*    Low-Noise Amplifier Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_i2c.h"
#include "blna_3412.h"
#include "priv/nexus_i2c_priv.h"

BDBG_MODULE(nexus_amplifier_3412);

/***************************************************************************
Summary:
	Get the default settings for a BCM3412 amplifier
See Also:
    NEXUS_Amplifier_Open3412
 ***************************************************************************/
void NEXUS_Amplifier_GetDefault3412Settings(
    NEXUS_3412Settings *pSettings   /* [out] */
    )
{
    BLNA_3412_Settings lnaSettings;

    BDBG_ASSERT(NULL != pSettings);

    BLNA_3412_GetDefaultSettings(&lnaSettings, NULL);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->i2cAddr = lnaSettings.i2cAddr;    
}

/***************************************************************************
Summary:
	Open a handle to a BCM 3412 device.
See Also:
    NEXUS_Amplifier_Close3412
 ***************************************************************************/
NEXUS_AmplifierHandle NEXUS_Amplifier_Open3412(
    const NEXUS_3412Settings *pSettings
    )
{
    BLNA_3412_Settings lnaSettings;
    BREG_I2C_Handle i2cHandle;
    NEXUS_Amplifier *pAmp;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);

    BLNA_3412_GetDefaultSettings(&lnaSettings, NULL);
    lnaSettings.i2cAddr = pSettings->i2cAddr;
    lnaSettings.skipInit = pSettings->skipInit;

    pAmp = BKNI_Malloc(sizeof(NEXUS_Amplifier));
    if ( NULL == pAmp )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Amplifier, pAmp);

    i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
    if ( NULL == i2cHandle )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        BKNI_Free(pAmp);
        return NULL;
    }

    errCode = BLNA_3412_Open(&pAmp->lnaHandle,
                             NULL,
                             i2cHandle,
                             &lnaSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        BKNI_Free(pAmp);
        return NULL;
    }

    if (lnaSettings.gainMode == BLNA_GainMode_eAGC)
    {
        pAmp->settings.gainMode = NEXUS_AmplifierGainMode_eAutomatic;
        pAmp->settings.gainSettings.automatic.outputLevel = lnaSettings.agcOutputLevel;
        pAmp->settings.gainSettings.automatic.deltaValue = lnaSettings.agcDeltaVal;
    }
    else
    {
        pAmp->settings.gainMode = NEXUS_AmplifierGainMode_eManual;
        pAmp->settings.gainSettings.manual.gain = lnaSettings.manualGainVal;
    }
    pAmp->settings.enabled = true;
    return pAmp;
}

