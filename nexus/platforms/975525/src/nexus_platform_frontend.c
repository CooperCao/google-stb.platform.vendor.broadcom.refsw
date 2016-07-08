 /***************************************************************************
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
*
* Module Description:
*
* API Description:
*   API name: Platform linuxuser
*    linuxuser OS routines
*
*
***************************************************************************/
#include "nexus_types.h"
#include "nexus_platform_priv.h"
#include "nexus_platform.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#include "bchp_gio.h"

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND
#include "nexus_frontend_75525.h"

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendDeviceOpenSettings deviceOpenSettings;
    NEXUS_FrontendDeviceSettings deviceSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendChannelSettings channelSettings;
    NEXUS_PlatformStatus platformStatus;
    unsigned i=0;
    bool enableSatellite = false;

    rc = NEXUS_Platform_GetStatus(&platformStatus);
    if (rc) goto done;

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceOpenSettings);

    deviceOpenSettings.satellite.enabled = enableSatellite;
    deviceOpenSettings.terrestrial.enabled = true;
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.isrNumber = 95;

    device = NEXUS_FrontendDevice_Open(0, &deviceOpenSettings);

    if (device) {
        NEXUS_FrontendDevice_GetDefaultSettings(&deviceSettings);
        deviceSettings.rfDaisyChain = NEXUS_RfDaisyChain_eInternalLna;
        deviceSettings.rfInput = NEXUS_TunerRfInput_eInternalLna;
        deviceSettings.enableRfLoopThrough = false;
        NEXUS_FrontendDevice_SetSettings(device, &deviceSettings);

        channelSettings.device = device;
        channelSettings.channelNumber = i;
        channelSettings.type = NEXUS_FrontendChannelType_eTerrestrial;

        pConfig->frontend[i] = NEXUS_Frontend_Open(&channelSettings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_ERR(("Unable to open first 75525 dvbt tuner/demodulator channel."));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
        }

        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = false;
        userParams.param1 = NEXUS_InputBand_e0 + i; /* HARDCODING*/
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    } else {
        BDBG_ERR(("Unable to open 75525 device"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
done:
    return rc;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        handleFound = false;
        if (pConfig->frontend[i]) {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                        handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }
            BDBG_MSG(("Calling NEXUS_Frontend_Close[%d]",i));
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            BDBG_MSG(("Calling NEXUS_FrontendDevice_Close"));
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }

    return;
}
#else

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    return;
}

#endif

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
    /* 975525 Uses input band 0 for steramer */
    /* but the HW team recommands using input band 4 */
    *pInputBand = NEXUS_InputBand_e0;

    BTRC_TRACE(ChnChange_TuneStreamer, STOP);
    return NEXUS_SUCCESS;
}

NEXUS_FrontendHandle NEXUS_Platform_OpenFrontend(
    unsigned id /* platform assigned ID for this frontend. See NEXUS_FrontendUserParameters.id.
                   See nexus_platform_frontend.c for ID assignment and/or see
                   nexus_platform_features.h for possible platform-specific macros.
                */
    )
{
    NEXUS_Error errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    BSTD_UNUSED(errCode);
    BSTD_UNUSED(id);
    return NULL;
}
