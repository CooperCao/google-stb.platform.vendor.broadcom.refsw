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
 ***************************************************************************/

/* General includes */
#include "nexus_frontend_7364_priv.h"
/* End general includes */

/* Satellite includes */
#include "bsat.h"
#include "bdsq.h"
#include "bwfe.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_leap_host_l1.h"

#include "bsat_g1.h"
#include "bwfe_g3.h"
#include "bdsq_g1.h"
/* End satellite includes */

BDBG_MODULE(nexus_frontend_7364_satellite);

/* Generic function declarations */
static void NEXUS_Frontend_P_7364_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);

static NEXUS_Error NEXUS_Frontend_P_7364_Satellite_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
/* End of generic function declarations */

/* Satellite-specific functions */
static NEXUS_Error NEXUS_Frontend_P_7364_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
static void NEXUS_Frontend_P_7364_UntuneSatellite(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7364_ReapplyTransportSettings(void *handle);

static BDSQ_ChannelHandle NEXUS_Frontend_P_7364_GetDiseqcChannelHandle(void *handle, int index);
static NEXUS_Error NEXUS_Frontend_P_7364_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage);

static NEXUS_Error NEXUS_FrontendDevice_P_Get7364Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities);
static NEXUS_Error NEXUS_Frontend_P_Get7364RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Set7364RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings);

static void NEXUS_Frontend_P_7364_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings);

static NEXUS_Error NEXUS_Frontend_P_7364_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus);

static void NEXUS_Frontend_P_7364_ClearSpectrumCallbacks(void *handle);
/* End of satellite-specific function declarations */


/* Satellite frontend device implementation */

typedef struct NEXUS_FrontendDevice_P_7364_InitSettings {
    BSAT_Settings satSettings;
    BWFE_Settings wfeSettings;
    BDSQ_Settings dsqSettings;
} NEXUS_FrontendDevice_P_7364_InitSettings;

NEXUS_Error NEXUS_FrontendDevice_P_Init7364_Satellite(NEXUS_7364Device *pFrontendDevice)
{
    NEXUS_FrontendDevice_P_7364_InitSettings *pInitSettings;

    uint32_t numChannels;
    uint32_t numDsqChannels;

    unsigned i;
    BERR_Code errCode;

    BDBG_ASSERT(pFrontendDevice);

    pInitSettings = (NEXUS_FrontendDevice_P_7364_InitSettings *)BKNI_Malloc(sizeof(NEXUS_FrontendDevice_P_7364_InitSettings));
    if (!pInitSettings) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err;
    }
    BKNI_Memset(pInitSettings, 0, sizeof(NEXUS_FrontendDevice_P_7364_InitSettings));

    /* Initialize the acquisition processor */
    {
        uint32_t val;

        /* assert sw_init signals */
        val = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET);
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_stb_chan_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_wb_stat_top0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_mdac_cal_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds1_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds0_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_fsk_top_sw_init_MASK;
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET, val);

        /* release sw_init signals */
        val = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR);
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_stb_chan_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_wb_stat_top0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_mdac_cal_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds1_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds0_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_fsk_top_sw_init_MASK;
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR, val);

        /* enable the LEAP interrupt in HIF */
        val = BCHP_HIF_CPU_INTR1_INTR_W3_MASK_CLEAR_LEAP_CPU_INTR_MASK;
        BREG_Write32(g_pCoreHandles->reg, BCHP_HIF_CPU_INTR1_INTR_W3_MASK_CLEAR, val);

        /* enable the LEAP_HOST_L1 interrupts */
        val = BCHP_LEAP_HOST_L1_INTR_W0_STATUS_TFEC0_0_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_TFEC0_1_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_SDS0_0_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_SDS0_1_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_AIF_SAT_0_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_AFEC0_0_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_AFEC0_1_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_AFEC0_GLBL_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_DSEC0_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_FTM_INTR_MASK;
        val |= BCHP_LEAP_HOST_L1_INTR_W0_STATUS_MDAC_INTR_MASK;
        BREG_Write32(g_pCoreHandles->reg, BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR, val);
    }


    /* WFE Open */
    BWFE_g3_GetDefaultSettings(&pInitSettings->wfeSettings);
    errCode = BWFE_Open(&pFrontendDevice->satellite.satDevice->wfeHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &pInitSettings->wfeSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    BSAT_g1_GetDefaultSettings(&pInitSettings->satSettings);
    errCode = BSAT_Open(&pFrontendDevice->satellite.satDevice->satHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &pInitSettings->satSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    BDSQ_g1_GetDefaultSettings(&pInitSettings->dsqSettings);
    errCode = BDSQ_Open(&pFrontendDevice->satellite.satDevice->dsqHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &pInitSettings->dsqSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    /* Determine number of channels -- they will be opened later */
    BSAT_GetTotalChannels(pFrontendDevice->satellite.satDevice->satHandle, &numChannels);
    BDBG_MSG(("frontend has %d channels",numChannels));
    if (numChannels > NEXUS_SAT_MAX_CHANNELS)
    {
        BDBG_WRN(("This device supports more than the expected number of channels. Unexpected channels will not be initialized."));
    }
    pFrontendDevice->satellite.satDevice->numChannels = numChannels;

    /* Open all channels prior to InitAp */
    for ( i = 0; i < pFrontendDevice->satellite.satDevice->numChannels; i++ )
    {
        BSAT_ChannelSettings bsatChannelSettings;
        BSAT_GetChannelDefaultSettings(pFrontendDevice->satellite.satDevice->satHandle, i, &bsatChannelSettings);
        errCode = BSAT_OpenChannel(pFrontendDevice->satellite.satDevice->satHandle, &pFrontendDevice->satellite.satChannels[i], i, &bsatChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open sat channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    /* Determine number of inputs */
    errCode = BWFE_GetTotalChannels(pFrontendDevice->satellite.satDevice->wfeHandle, &pFrontendDevice->satellite.satDevice->wfeInfo);
    BDBG_ASSERT(!errCode);
    pFrontendDevice->satellite.satDevice->numWfe = pFrontendDevice->satellite.satDevice->wfeInfo.numChannels;

    /* Open WFE Channels */
    for (i=0; i < pFrontendDevice->satellite.satDevice->wfeInfo.numChannels; i++) {
        BWFE_ChannelSettings wfeChannelSettings;
        BWFE_GetChannelDefaultSettings(pFrontendDevice->satellite.satDevice->wfeHandle, i, &wfeChannelSettings);
        errCode = BWFE_OpenChannel(pFrontendDevice->satellite.satDevice->wfeHandle, &pFrontendDevice->satellite.satDevice->wfeChannels[i], i, &wfeChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open wfe channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
        errCode = BWFE_GetWfeReadyEventHandle(pFrontendDevice->satellite.satDevice->wfeChannels[i], &pFrontendDevice->satellite.satDevice->wfeReadyEvent[i]);
        if ( errCode ) {
            BDBG_ERR(("Unable to retrieve ready event for wfe channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    BWFE_Reset(pFrontendDevice->satellite.satDevice->wfeHandle);
    BSAT_Reset(pFrontendDevice->satellite.satDevice->satHandle);

    /* Open DSQ Channels */
    BDSQ_GetTotalChannels(pFrontendDevice->satellite.satDevice->dsqHandle, &numDsqChannels);
    pFrontendDevice->satellite.satDevice->numDsq = numDsqChannels;
    for (i=0; i < numDsqChannels; i++) {
        BDSQ_ChannelSettings dsqChannelSettings;
        BDSQ_GetChannelDefaultSettings(pFrontendDevice->satellite.satDevice->dsqHandle, i, &dsqChannelSettings);
        errCode = BDSQ_OpenChannel(pFrontendDevice->satellite.satDevice->dsqHandle, &pFrontendDevice->satellite.satDevice->dsqChannels[i], i, &dsqChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open dsq channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    BDSQ_Reset(pFrontendDevice->satellite.satDevice->dsqHandle);

    BKNI_Free(pInitSettings);

    return NEXUS_SUCCESS;

err:
    BDBG_MSG(("Error opening satellite frontend"));
    return NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NEXUS_FrontendDevice_Open7364_Satellite(NEXUS_7364Device *pFrontendDevice)
{
    NEXUS_FrontendSatDeviceSettings satDeviceSettings;

    NEXUS_Error rc;

    NEXUS_Frontend_P_Sat_GetDefaultDeviceSettings(&satDeviceSettings);

    pFrontendDevice->satellite.satDevice = NEXUS_Frontend_P_Sat_Create_Device(&satDeviceSettings);
    if (pFrontendDevice->satellite.satDevice == NULL) { return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); }

    rc = NEXUS_FrontendDevice_P_Init7364_Satellite(pFrontendDevice);

    pFrontendDevice->pGenericDeviceHandle->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Get7364Capabilities;

    return rc;
}
/* End satellite frontend device implementation */

/* Satellite implementation */
NEXUS_FrontendHandle NEXUS_Frontend_Open7364_Satellite( const NEXUS_FrontendChannelSettings *pSettings )
{
    NEXUS_7364Device *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_FrontendSatChannelSettings satChannelSettings;

    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->device);

    pFrontendDevice = pSettings->device;
    pDevice = (NEXUS_7364Device *)pFrontendDevice->pDevice;

    /* Return previously opened frontend handle. */
    if (pSettings->channelNumber >= pDevice->satellite.satDevice->numChannels) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (pDevice->satellite.handles[pSettings->channelNumber])
        return pDevice->satellite.handles[pSettings->channelNumber];

    /* Otherwise, open new frontend */
    BDBG_MSG(("Creating channel %u", pSettings->channelNumber));

    /* Open channel */
    NEXUS_Frontend_P_Sat_GetDefaultChannelSettings(&satChannelSettings);
    satChannelSettings.satDevice = pDevice->satellite.satDevice;
    satChannelSettings.satChannel = pDevice->satellite.satChannels[pSettings->channelNumber];
#define B_SAT_CHIP 7364
    satChannelSettings.satChip = B_SAT_CHIP;
    satChannelSettings.channelIndex = pSettings->channelNumber;
    satChannelSettings.pCloseParam = pDevice;
    satChannelSettings.pDevice = pDevice;
    satChannelSettings.closeFunction = NEXUS_Frontend_P_7364_CloseCallback;
    satChannelSettings.diseqcIndex = 0;
    satChannelSettings.capabilities.diseqc = true;
    if (pDevice->openSettings.satellite.diseqc.i2cDevice) {
        BREG_I2C_Handle i2cHandle = NULL;
        satChannelSettings.getDiseqcChannelHandle = NEXUS_Frontend_P_7364_GetDiseqcChannelHandle;
        satChannelSettings.getDiseqcChannelHandleParam = pDevice;
        satChannelSettings.setVoltage = NEXUS_Frontend_P_7364_SetVoltage;
        i2cHandle = NEXUS_I2c_GetRegHandle(pDevice->openSettings.satellite.diseqc.i2cDevice, NEXUS_MODULE_SELF);
        satChannelSettings.i2cRegHandle = i2cHandle; /* due to module locking, we need to save our register handle for Diseqc voltage control */
    }
    satChannelSettings.getDefaultDiseqcSettings = NEXUS_Frontend_P_7364_GetDefaultDiseqcSettings;

    satChannelSettings.wfeHandle = pDevice->satellite.satDevice->wfeHandle;

    satChannelSettings.deviceClearSpectrumCallbacks = NEXUS_Frontend_P_7364_ClearSpectrumCallbacks;

    frontend = NEXUS_Frontend_P_Sat_Create_Channel(&satChannelSettings);
    if (!frontend)
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_7364_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        goto err;
    }
    frontend->tuneSatellite = NEXUS_Frontend_P_7364_TuneSatellite;
    frontend->untune = NEXUS_Frontend_P_7364_UntuneSatellite;
    frontend->reapplyTransportSettings = NEXUS_Frontend_P_7364_ReapplyTransportSettings;
    frontend->pGenericDeviceHandle = pFrontendDevice;
    frontend->getSatelliteAgcStatus = NEXUS_Frontend_P_7364_GetSatelliteAgcStatus;
    frontend->getSatelliteRuntimeSettings = NEXUS_Frontend_P_Get7364RuntimeSettings;
    frontend->setSatelliteRuntimeSettings = NEXUS_Frontend_P_Set7364RuntimeSettings;

    frontend->standby = NEXUS_Frontend_P_7364_Satellite_Standby;

    pDevice->satellite.handles[pSettings->channelNumber] = frontend;

    /* preconfigure mtsif settings so platform doesn't need to */
    frontend->userParameters.isMtsif = true;
    frontend->mtsif.inputBand = pSettings->channelNumber + NEXUS_FRONTEND_7364_SATELLITE_INPUT_BAND;
                            /* IB for demod 0 is 8/18, demod 1 is 9/19. Offset channel by 18 to compensate. */

    return frontend;

err:
    return NULL;
}

static void NEXUS_Frontend_P_7364_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    NEXUS_7364Device *pDevice = pParam;

    BSTD_UNUSED(handle);
    BSTD_UNUSED(pDevice);
}

void NEXUS_FrontendDevice_P_Uninit7364_Satellite(NEXUS_7364Device *pFrontendDevice)
{
    unsigned i;

    /* Satellite teardown */
    if (pFrontendDevice->satellite.satDevice) {
        if (pFrontendDevice->satellite.satDevice->dsqHandle) {
            for (i=0 ; i < pFrontendDevice->satellite.satDevice->numDsq; i++) {
                if (pFrontendDevice->satellite.satDevice->dsqChannels[i]) {
                    BDSQ_CloseChannel(pFrontendDevice->satellite.satDevice->dsqChannels[i]);
                    pFrontendDevice->satellite.satDevice->dsqChannels[i] = NULL;
                }
            }
            BDSQ_Close(pFrontendDevice->satellite.satDevice->dsqHandle);
            pFrontendDevice->satellite.satDevice->dsqHandle = NULL;
        }
        if (pFrontendDevice->satellite.satDevice->wfeHandle) {
            for (i=0 ; i < pFrontendDevice->satellite.satDevice->numWfe; i++) {
                if (pFrontendDevice->satellite.satDevice->wfeChannels[i]) {
                    BWFE_CloseChannel(pFrontendDevice->satellite.satDevice->wfeChannels[i]);
                    pFrontendDevice->satellite.satDevice->wfeChannels[i] = NULL;
                }
            }
            BWFE_Close(pFrontendDevice->satellite.satDevice->wfeHandle);
            pFrontendDevice->satellite.satDevice->wfeHandle = NULL;
        }
        if (pFrontendDevice->satellite.satDevice->satHandle) {
            for (i=0 ; i < pFrontendDevice->satellite.satDevice->numChannels; i++) {
                if (pFrontendDevice->satellite.satChannels[i]) {
                    BSAT_CloseChannel(pFrontendDevice->satellite.satChannels[i]);
                    pFrontendDevice->satellite.satChannels[i] = NULL;
                }
            }
            BSAT_Close(pFrontendDevice->satellite.satDevice->satHandle);
            pFrontendDevice->satellite.satDevice->satHandle = NULL;
        }
    }
    /* End of satellite teardown */
}

void NEXUS_FrontendDevice_Close7364_Satellite(NEXUS_7364Device *pFrontendDevice)
{
    NEXUS_FrontendDevice_P_Uninit7364_Satellite(pFrontendDevice);
    if (pFrontendDevice->satellite.satDevice) {
        BKNI_Free(pFrontendDevice->satellite.satDevice);
        pFrontendDevice->satellite.satDevice = NULL;
    }
}

static NEXUS_Error NEXUS_Frontend_P_7364_Satellite_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7364Device *p7364Device;

    BDBG_OBJECT_ASSERT(pSatChannel, NEXUS_SatChannel);
    p7364Device = pSatChannel->settings.pDevice;

    BDBG_MSG(("NEXUS_Frontend_P_7364_Satellite_Standby: standby %p(%d) %s", handle, pSatChannel->channel, enabled ? "enabled" : "disabled"));

    /* update/restore handles */
    pSatChannel->satChannel = p7364Device->satellite.satChannels[pSatChannel->channel];

    if (pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Unregistering events on %p",(void *)pSatChannel));
        NEXUS_Frontend_P_Sat_UnregisterEvents(pSatChannel);
    } else if (pSettings->mode != NEXUS_StandbyMode_eDeepSleep && pSatChannel->frontendHandle->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Registering events on %p",(void *)pSatChannel));
        NEXUS_Frontend_P_Sat_RegisterEvents(pSatChannel);
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Get7364Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7364Device *p7364Device = (NEXUS_7364Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);

    BKNI_Memset(pCapabilities,0,sizeof(*pCapabilities));
    pCapabilities->numAdc = p7364Device->satellite.satDevice->numWfe;
    pCapabilities->numChannels = p7364Device->satellite.satDevice->numChannels;
    pCapabilities->externalBert = false;

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Get7364RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);

    pSettings->selectedAdc = pSatChannel->selectedAdc;
    pSatChannel->diseqcIndex = pSatChannel->selectedAdc;
    pSettings->externalBert.enabled = false;
    pSettings->externalBert.invertClock = false;

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Set7364RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7364Device *p7364Device;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    p7364Device = pSatChannel->settings.pDevice;

    BDBG_MSG(("adc: %d, mask: 0x%08x",pSettings->selectedAdc,p7364Device->satellite.satDevice->wfeInfo.availChannelsMask));
    /* Ensure the requested adc is within the value range, and advertised by the PI as being available */
    if (pSettings->selectedAdc > p7364Device->satellite.satDevice->numWfe || !((1<<pSettings->selectedAdc) & p7364Device->satellite.satDevice->wfeInfo.availChannelsMask) )
        return NEXUS_INVALID_PARAMETER;

    pSatChannel->selectedAdc = pSettings->selectedAdc;
    pSatChannel->diseqcIndex = pSettings->selectedAdc;
    pSatChannel->satDevice->wfeMap[pSatChannel->channel] = pSettings->selectedAdc;

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_7364_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7364Device *p7364Device = pSatChannel->settings.pDevice;
    BERR_Code rc = NEXUS_SUCCESS;
    BSAT_ChannelStatus satStatus;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NULL != p7364Device);

    if (pSatChannel->satChip != 7364) {
        return NEXUS_INVALID_PARAMETER;
    }

    errCode = BSAT_GetChannelStatus(pSatChannel->satChannel, &satStatus);
    if (errCode) {
        BDBG_MSG(("BSAT_GetChannelStatus returned %x",errCode));
        rc = errCode;
    } else {
        int i;
        uint8_t wfeBuf[4];

        BKNI_Memset(pStatus,0,sizeof(*pStatus));
        for (i=0; i<3; i++) {
            pStatus->agc[i].value = satStatus.agc.value[i];
            pStatus->agc[i].valid = (satStatus.agc.flags & 1<<i);
        }

        errCode = BWFE_ReadConfig(pSatChannel->satDevice->wfeChannels[pSatChannel->selectedAdc], BWFE_G3_CONFIG_RF_AGC_INT, wfeBuf, 4);
        if (errCode) {
            BDBG_MSG(("BSAT_GetChannelStatus returned %x",errCode));
            rc = errCode;
        } else {
            pStatus->agc[3].value = (wfeBuf[0] << 24) | (wfeBuf[1] << 16) | (wfeBuf[2] << 8) | wfeBuf[3];
            pStatus->agc[3].valid = true;
        }
    }

    return rc;
}

NEXUS_Error NEXUS_Frontend_P_Sat_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
void NEXUS_Frontend_P_Sat_Untune(void *handle);

static NEXUS_Error NEXUS_Frontend_P_7364_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_Error rc;

    rc = NEXUS_Frontend_P_SetMtsifConfig(pSatChannel->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }

    return NEXUS_Frontend_P_Sat_TuneSatellite(pSatChannel, pSettings);
}

static void NEXUS_Frontend_P_7364_UntuneSatellite(void *handle)
{
    NEXUS_SatChannel *pSatChannel = handle;

    NEXUS_Frontend_P_UnsetMtsifConfig(pSatChannel->frontendHandle);

    NEXUS_Frontend_P_Sat_Untune(pSatChannel);

    return;
}

NEXUS_Error NEXUS_Frontend_P_7364_ReapplyTransportSettings(void *handle)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_Error rc;

    rc = NEXUS_Frontend_P_SetMtsifConfig(pSatChannel->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }

    return NEXUS_SUCCESS;
}

static BDSQ_ChannelHandle NEXUS_Frontend_P_7364_GetDiseqcChannelHandle(void *handle, int index)
{
    NEXUS_SatChannel *pSatChannel = handle;
    NEXUS_7364Device *p7364Device;
    p7364Device = pSatChannel->settings.pDevice;
    return p7364Device->satellite.satDevice->dsqChannels[index];
}

static NEXUS_Error NEXUS_Frontend_P_7364_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_7364Device *p7364Device = pSatChannel->settings.pDevice;

    /* Just in case DSQ book-keeping requires it: */
    BDSQ_SetVoltage(p7364Device->satellite.satDevice->dsqChannels[pSatChannel->diseqcIndex], voltage == NEXUS_FrontendDiseqcVoltage_e18v);

    { /* Write voltage to A8299 */
        int channel = pSatChannel->diseqcIndex;
        uint8_t buf[2];
        uint8_t i2c_addr, shift, ctl;
        uint8_t A8299_control = p7364Device->satellite.A8299_control;

        i2c_addr = 0x8;

        if ((channel & 1) == 0)
            shift = 0;
        else
            shift = 4;

        buf[0] = 0;

        /* Clear A8299 i2c in case of fault */
        rc = BREG_I2C_WriteNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 1);
        BREG_I2C_ReadNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 1);

        ctl = (voltage == NEXUS_FrontendDiseqcVoltage_e18v) ? 0xC : 0x8;
        A8299_control &= ~((0x0F) << shift);
        A8299_control |= (ctl << shift);

        buf[0] = 0;
        buf[1] = A8299_control;

        p7364Device->satellite.A8299_control = A8299_control;

        BDBG_MSG(("A8299: channel=%d, i2c_addr=0x%X, ctl=0x%02X 0x%02X", channel, i2c_addr, buf[0], buf[1]));
        rc = BREG_I2C_WriteNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 2);
        if (rc) return BERR_TRACE(rc);

    }

    return rc;
}

static void NEXUS_Frontend_P_7364_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings)
{
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_7364Device *p7364Device = pSatChannel->settings.pDevice;
    BDSQ_GetChannelDefaultSettings(p7364Device->satellite.satDevice->dsqHandle, 0, settings);
}

static void NEXUS_Frontend_P_7364_ClearSpectrumCallbacks(void *handle)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7364Device *p7364Device;
    unsigned i;

    BDBG_ASSERT(pSatChannel);
    p7364Device = pSatChannel->settings.pDevice;

    for (i=0; i < p7364Device->satellite.satDevice->numChannels; i++) {
        NEXUS_SatChannel *pDevice = p7364Device->satellite.handles[i]->pDeviceHandle;
        if (pDevice) {
            if (pDevice->spectrumEventCallback) {
                NEXUS_UnregisterEvent(pDevice->spectrumEventCallback);
                pDevice->spectrumEventCallback = NULL;
            }
        }
    }

}
