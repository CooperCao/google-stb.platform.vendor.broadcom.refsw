/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*   API name: 97340 Frontend Extension
*    Extension code to handle 97340 frontend board-specifics
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_extension);
BDBG_OBJECT_ID(NEXUS_97340QpskExtension);

typedef struct NEXUS_97340QpskExtension
{
    BDBG_OBJECT(NEXUS_97340QpskExtension)
    NEXUS_FrontendHandle frontend;
    NEXUS_TunerHandle tuner;
} NEXUS_97340QpskExtension;

static void NEXUS_Frontend_P_CloseSds97340(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_TuneSds97340(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);

static void NEXUS_Frontend_P_CloseQpsk97340(NEXUS_FrontendHandle handle);
static NEXUS_Error NEXUS_Frontend_P_TuneQpsk97340(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);

void NEXUS_Frontend_GetDefault97340ExtensionSettings(
    NEXUS_97340FrontendExtensionSettings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static NEXUS_FrontendDevice extensionDummyDevice;

NEXUS_Error NEXUS_Frontend_Create97340Extension(
    const NEXUS_97340FrontendExtensionSettings *pSettings,
    NEXUS_97340FrontendExtensionHandles *pHandles
    )
{
    NEXUS_FrontendExtensionSettings extensionSettings;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandles);

    BKNI_Memset(pHandles, 0, sizeof(*pHandles));

    BKNI_Memset(&extensionDummyDevice, 0, sizeof(extensionDummyDevice));

    if ( pSettings->primary )
    {
        NEXUS_Frontend_P_GetDefaultExtensionSettings(&extensionSettings);
        extensionSettings.tuneSatellite = NEXUS_Frontend_P_TuneSds97340;
        extensionSettings.parent = pSettings->primary;
        extensionSettings.close = NEXUS_Frontend_P_CloseSds97340;
        extensionSettings.pDeviceHandle = pSettings->primary;
        pHandles->primary = NEXUS_Frontend_P_CreateExtension(&extensionSettings);
        pHandles->primary->pGenericDeviceHandle = &extensionDummyDevice;
    }

    if ( pSettings->qpsk && pSettings->qpskTuner )
    {
        NEXUS_97340QpskExtension *pQpskExtension;

        pQpskExtension = BKNI_Malloc(sizeof(NEXUS_97340QpskExtension));
        if ( NULL == pQpskExtension )
        {
            if ( pHandles->primary )
            {
                NEXUS_Frontend_Close(pHandles->primary);
            }
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }

        BDBG_OBJECT_SET(pQpskExtension, NEXUS_97340QpskExtension);
        pQpskExtension->frontend = pSettings->qpsk;
        pQpskExtension->tuner = pSettings->qpskTuner;

        NEXUS_Frontend_P_GetDefaultExtensionSettings(&extensionSettings);
        extensionSettings.tuneSatellite = NEXUS_Frontend_P_TuneQpsk97340;
        extensionSettings.parent = pSettings->qpsk;
        extensionSettings.close = NEXUS_Frontend_P_CloseQpsk97340;
        extensionSettings.pDeviceHandle = pQpskExtension;
        pHandles->qpsk = NEXUS_Frontend_P_CreateExtension(&extensionSettings);
        pHandles->qpsk->pGenericDeviceHandle = &extensionDummyDevice;
        if ( NULL == pHandles->qpsk )
        {
            if ( pHandles->primary )
            {
                NEXUS_Frontend_Close(pHandles->primary);
            }
            BKNI_Free(pQpskExtension);
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }

    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_CloseSds97340(NEXUS_FrontendHandle handle)
{
    NEXUS_Frontend_P_DestroyExtension(handle);
}

static NEXUS_Error NEXUS_Frontend_P_TuneSds97340(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_FrontendHandle frontend = handle;
    NEXUS_73xxLnaSettings lnaSettings;

    BDBG_ASSERT(NULL != handle);

    /* NEXUS_Frontend_Get73xxLnaSettings(frontend, &lnaSettings); */


        /* configure lna for In2Out1 In2Out2 DaisyOff */
        lnaSettings.out1 = NEXUS_73xxLnaInput_eNone;
        lnaSettings.out2 = NEXUS_73xxLnaInput_eNone;
        lnaSettings.daisy = NEXUS_73xxLnaInput_eNone;

        /* NEXUS_Frontend_Set73xxLnaSettings(frontend, &lnaSettings); */

    /* Kick off the demodulator */
    return NEXUS_Frontend_TuneSatellite(frontend, pSettings);
}


static void NEXUS_Frontend_P_CloseQpsk97340(NEXUS_FrontendHandle handle)
{
    NEXUS_97340QpskExtension *pExtension;
    pExtension = handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(pExtension, NEXUS_97340QpskExtension);
    BDBG_OBJECT_DESTROY(pExtension, NEXUS_97340QpskExtension);
    BKNI_Free(pExtension);
    NEXUS_Frontend_P_DestroyExtension(handle);
}

static NEXUS_Error NEXUS_Frontend_P_TuneQpsk97340(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_FrontendSatelliteSettings newSettings;
    NEXUS_97340QpskExtension *pExtension;

    pExtension = handle;
    BDBG_OBJECT_ASSERT(pExtension, NEXUS_97340QpskExtension);

    if ( pSettings )
    {
        /* Tune the tuner */
        errCode = NEXUS_Tuner_SetFrequency(pExtension->tuner, NEXUS_TunerMode_eDigital, pSettings->frequency);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    
        /* Now set the filter */
        errCode = NEXUS_Tuner_Set3440SymbolRate(pExtension->tuner, pSettings->symbolRate, true  /* Eventually need a flag for the nyquist setting */);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    
        /* Retrieve the new IF offset */
        newSettings = *pSettings;
        NEXUS_Tuner_Get3440IfOffset(pExtension->tuner, &newSettings.ifOffset);
    }

    return NEXUS_Frontend_TuneSatellite(pExtension->frontend, &newSettings);
}

