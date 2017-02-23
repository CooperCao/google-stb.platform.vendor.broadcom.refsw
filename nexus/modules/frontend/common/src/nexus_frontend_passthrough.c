/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#include "nexus_frontend_module.h"
#include "priv/nexus_frontend_mtsif_priv.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_frontend_passthrough);

#if NEXUS_FRONTEND_PASSTHROUGH

static void NEXUS_Frontend_P_Untune(void *handle)
{
    BSTD_UNUSED(handle);
    return;
}

static NEXUS_Error NEXUS_Frontend_P_TuneQam(void *handle, const NEXUS_FrontendQamSettings *pSettings)
{
    NEXUS_FrontendDeviceHandle pDevice = handle;
    NEXUS_TaskCallback_Set(pDevice->dummyLockCallback, &(pSettings->lockCallback));
    NEXUS_TaskCallback_Fire(pDevice->dummyLockCallback);
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_TuneVsb(void *handle, const NEXUS_FrontendVsbSettings *pSettings)
{
    NEXUS_FrontendDeviceHandle pDevice = handle;
    NEXUS_TaskCallback_Set(pDevice->dummyLockCallback, &(pSettings->lockCallback));
    NEXUS_TaskCallback_Fire(pDevice->dummyLockCallback);
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_FrontendDeviceHandle pDevice = handle;
    NEXUS_TaskCallback_Set(pDevice->dummyLockCallback, &(pSettings->lockCallback));
    NEXUS_TaskCallback_Fire(pDevice->dummyLockCallback);
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_TuneOfdm(void *handle, const NEXUS_FrontendOfdmSettings *pSettings)
{
    NEXUS_FrontendDeviceHandle pDevice = handle;
    NEXUS_TaskCallback_Set(pDevice->dummyLockCallback, &(pSettings->lockCallback));
    NEXUS_TaskCallback_Fire(pDevice->dummyLockCallback);
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_GetQamStatus(void *handle, NEXUS_FrontendQamStatus *pStatus)
{
    BSTD_UNUSED(handle);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->receiverLock = true;
    pStatus->fecLock = true;
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_GetVsbStatus(void *handle, NEXUS_FrontendVsbStatus *pStatus)
{
    BSTD_UNUSED(handle);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->receiverLock = true;
    pStatus->fecLock = true;
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_GetSatelliteStatus(void *handle, NEXUS_FrontendSatelliteStatus *pStatus)
{
    BSTD_UNUSED(handle);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->tunerLocked = true;
    pStatus->demodLocked = true;
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_GetOfdmStatus(void *handle, NEXUS_FrontendOfdmStatus *pStatus)
{
    BSTD_UNUSED(handle);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->receiverLock = true;
    pStatus->fecLock = true;
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_GetFastStatus(void *handle, NEXUS_FrontendFastStatus *pStatus)
{
    BSTD_UNUSED(handle);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->lockStatus = NEXUS_FrontendLockStatus_eLocked;
    return 0;
}

void NEXUS_Frontend_P_Passthrough_Close(NEXUS_FrontendHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    NEXUS_Frontend_P_Destroy(handle);
}

void NEXUS_Frontend_P_Passthrough_DeviceClose(void *device)
{
    NEXUS_FrontendDeviceHandle pDevice = device;
    NEXUS_TaskCallback_Destroy(pDevice->dummyLockCallback);
    BKNI_Free(pDevice);
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_OpenPassthrough(void)
{
    NEXUS_Error rc;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;

    pFrontendDevice = NEXUS_FrontendDevice_P_Create();
    if (NULL == pFrontendDevice) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error;}

    pFrontendDevice->dummyLockCallback = NEXUS_TaskCallback_Create(pFrontendDevice, NULL);
    if (NULL == pFrontendDevice->dummyLockCallback) { rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto error; }

    pFrontendDevice->close = NEXUS_Frontend_P_Passthrough_DeviceClose;
    pFrontendDevice->pDevice = pFrontendDevice;

    return pFrontendDevice;

error:
    if (pFrontendDevice) {
        BKNI_Free(pFrontendDevice);
    }
    return NULL;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_OpenPassthrough(void *pDevice)
{
    NEXUS_FrontendHandle frontendHandle = NULL;
    NEXUS_Error rc;
    unsigned i;

    frontendHandle = NEXUS_Frontend_P_Create(pDevice);
    if (NULL == frontendHandle) {rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto error;}

    frontendHandle->pGenericDeviceHandle = pDevice;

    /* "support" only qam, vsm, satellite for now */
    frontendHandle->capabilities.qam = true;
    frontendHandle->capabilities.vsb = true;
    frontendHandle->capabilities.satellite = true;
    frontendHandle->capabilities.ofdm = true;
    for (i=0; i<NEXUS_FrontendQamMode_eMax; i++) {
        frontendHandle->capabilities.qamModes[i] = true;
    }
    for (i=0; i<NEXUS_FrontendVsbMode_eMax; i++) {
        frontendHandle->capabilities.vsbModes[i] = true;
    }
    for (i=0; i<NEXUS_FrontendSatelliteMode_eMax; i++) {
        frontendHandle->capabilities.satelliteModes[i] = true;
    }
    for (i=0; i<NEXUS_FrontendOfdmMode_eMax; i++) {
        frontendHandle->capabilities.ofdmModes[i] = true;
    }

    frontendHandle->tuneQam = NEXUS_Frontend_P_TuneQam;
    frontendHandle->getQamStatus = NEXUS_Frontend_P_GetQamStatus;
    frontendHandle->tuneVsb = NEXUS_Frontend_P_TuneVsb;
    frontendHandle->getVsbStatus = NEXUS_Frontend_P_GetVsbStatus;
    frontendHandle->tuneSatellite = NEXUS_Frontend_P_TuneSatellite;
    frontendHandle->getSatelliteStatus = NEXUS_Frontend_P_GetSatelliteStatus;
    frontendHandle->tuneOfdm = NEXUS_Frontend_P_TuneOfdm;
    frontendHandle->getOfdmStatus = NEXUS_Frontend_P_GetOfdmStatus;
    frontendHandle->getFastStatus = NEXUS_Frontend_P_GetFastStatus;
    frontendHandle->untune = NEXUS_Frontend_P_Untune; /* must set this to avoid generic NEXUS_Frontend_Untune from handling it */
    frontendHandle->close = NEXUS_Frontend_P_Passthrough_Close;

    return frontendHandle;

error:
    if (frontendHandle) {
        BKNI_Free(frontendHandle);
    }

    return NULL;
}
#endif
