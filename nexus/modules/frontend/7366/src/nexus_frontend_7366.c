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
#include "nexus_frontend_module.h"
#include "nexus_frontend_sat.h"
#include "priv/nexus_transport_priv.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "priv/nexus_spi_priv.h"
#include "bsat.h"
#include "bdsq.h"
#include "bwfe.h"
#if NEXUS_HAS_FSK
#include "bfsk.h"
#endif
#if NEXUS_FRONTEND_BYPASS_LEAP
#include "bsat_g1.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_leap_host_l1.h"
#include "bwfe_g3.h"
#else
#if BCHP_CHIP==7366
#include "bchp_sun_top_ctrl.h"
#endif
#include "bsat_7366.h"
#include "bwfe_7366.h"
#include "bdsq_7366.h"
#if NEXUS_HAS_FSK
#include "bfsk_7366.h"
#endif
#endif
#include "bhab.h"
#include "bhab_7366.h"
#include "bhab_7366_fw.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#include "bhab_satfe_img.h"
#include "bdbg.h"

BDBG_MODULE(nexus_frontend_7366);

BDBG_OBJECT_ID(NEXUS_7366Device);

/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

#if (BCHP_CHIP==7366 || BCHP_CHIP==7399) && (BCHP_VER >= BCHP_VER_B0)
#define NEXUS_FRONTEND_7366_INTERRUPT 110
#else
#define NEXUS_FRONTEND_7366_INTERRUPT 104
#endif

#if (BCHP_CHIP==7366 || BCHP_CHIP==7399) && (BCHP_VER >= BCHP_VER_B0)
#define NEXUS_FRONTEND_7366_USE_DRAM 1
#endif

typedef struct NEXUS_7366Device
{
    BDBG_OBJECT(NEXUS_7366Device)
    NEXUS_FrontendDevice7366OpenSettings settings;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_SatDeviceHandle satDevice;
    uint32_t numChannels;   /* prototype to match BSAT_GetTotalChannels */
    uint8_t numAdc;         /* prototype to match BSAT_GetAdcSelect */
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrCallback;
    BSAT_ChannelHandle satChannels[NEXUS_7366_MAX_FRONTEND_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_7366_MAX_FRONTEND_CHANNELS];
    BWFE_ChannelInfo wfeInfo;
    uint32_t numDsqChannels;   /* prototype to match BDSQ_GetTotalChannels */
    int wfeMap[NEXUS_7366_MAX_FRONTEND_CHANNELS];
    uint8_t A8299_control;
    uint32_t cachedBertPinmux;
#if NEXUS_HAS_FSK
    uint32_t numFskChannels;
    BFSK_Handle fskHandle;
    BFSK_ChannelHandle fskChannels[NEXUS_7366_MAX_FRONTEND_CHANNELS];
    NEXUS_EventCallbackHandle ftmEventCallback[NEXUS_7366_MAX_FRONTEND_CHANNELS];
#endif
    BIMG_Interface imgInterface;
    void *leapBuffer;
} NEXUS_7366Device;

#if 0
static BLST_S_HEAD(devList, NEXUS_7366Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);
#endif

static void NEXUS_Frontend_P_7366_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_7366_DestroyDevice(void *handle);

static NEXUS_Error NEXUS_Frontend_P_7366_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
static void NEXUS_Frontend_P_7366_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_7366_ReapplyTransportSettings(void *handle);

static BDSQ_ChannelHandle NEXUS_Frontend_P_7366_GetDiseqcChannelHandle(void *handle, int index);
static NEXUS_Error NEXUS_Frontend_P_7366_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage);

static NEXUS_Error NEXUS_FrontendDevice_P_Get7366Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities);
static NEXUS_Error NEXUS_Frontend_P_Get7366RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Set7366RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings);

static void NEXUS_Frontend_P_7366_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings);
#if NEXUS_HAS_FSK
static BFSK_ChannelHandle NEXUS_Frontend_P_7366_GetFskChannelHandle(void *handle, int index);
static void NEXUS_Frontend_P_7366_FtmEventCallback(void *context);
#endif

#if 0
static void NEXUS_Frontend_P_7366_RestoreLnaCallback(NEXUS_FrontendHandle handle, void *pParam);
static BERR_Code NEXUS_Frontend_P_7366_I2cRead(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_7366_I2cWrite(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_7366_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_7366_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
#endif
static NEXUS_Error NEXUS_Frontend_P_7366_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus);

static void NEXUS_FrontendDevice_P_7366_GetCapabilities(void * handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);

static void NEXUS_Frontend_P_7366_ClearSpectrumCallbacks(void *handle);

static NEXUS_Error NEXUS_FrontendDevice_P_7366_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_7366_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

void NEXUS_FrontendDevice_GetDefault7366OpenSettings(NEXUS_FrontendDevice7366OpenSettings *pSettings)
{
    int i;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    for (i=0; i < NEXUS_MAX_MTSIF; i++) {
        pSettings->mtsif[i].enabled = true;
    }
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 7366 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7366_IsrControl_isr(bool enable, void *pParam)
{
    int isrNumber = (int)pParam;

    if ( enable )
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Enable 7366 Interrupt %u", isrNumber));
#endif
        NEXUS_Core_EnableInterrupt_isr(isrNumber);
    }
    else
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Disable 7366 Interrupt %u", isrNumber));
#endif
        NEXUS_Core_DisableInterrupt_isr(isrNumber);
    }
}

/***************************************************************************
Summary:
    Enable/Disable gpio interrupts for a 7366 device
 ***************************************************************************/
static void NEXUS_Frontend_P_7366_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle = (NEXUS_GpioHandle)pParam;

#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("%s 7366 Gpio Interrupt %p", enable ? "Enable" : "Disable", (void *)gpioHandle));
#endif
    NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, enable);
}

static void NEXUS_Frontend_P_7366_IsrCallback(void *pParam)
{
    NEXUS_7366Device *pDevice = (NEXUS_7366Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7366Device);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("7366 ISR Callback (hab: %p)",(void *)pDevice->satDevice->habHandle));
#endif
    BHAB_ProcessInterruptEvent(pDevice->satDevice->habHandle);
}

static void NEXUS_Frontend_P_7366_L1_isr(void *param1, int param2)
{
    NEXUS_7366Device *pDevice = (NEXUS_7366Device *)param1;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7366Device);
    BSTD_UNUSED(param2);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("7366 L1 ISR (hab: %p)",(void *)pDevice->satDevice->habHandle));
#endif

    BHAB_HandleInterrupt_isr(pDevice->satDevice->habHandle);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("Done: 7366 L1 ISR (hab: %p)",(void *)pDevice->satDevice->habHandle));
#endif
}

typedef struct NEXUS_FrontendDevice_P_7366_InitSettings {
    BHAB_Settings habSettings;
    BHAB_7366_Settings hab7366Settings;
    BSAT_Settings satSettings;
    BWFE_Settings wfeSettings;
    BDSQ_Settings dsqSettings;
#if NEXUS_HAS_FSK
    BFSK_Settings fskSettings;
#endif
} NEXUS_FrontendDevice_P_7366_InitSettings;

static NEXUS_Error NEXUS_FrontendDevice_P_Init7366(NEXUS_7366Device *pDevice)
{
    NEXUS_FrontendDevice_P_7366_InitSettings *pInitSettings;
    BHAB_Handle habHandle;
    BSAT_Handle satHandle;
    BWFE_Handle wfeHandle;
    BDSQ_Handle dsqHandle;
#if NEXUS_HAS_FSK
    BFSK_Handle fskHandle;
#endif
    BERR_Code errCode;
    unsigned i;
    void *regHandle;
    bool isExternal = false;
    NEXUS_FrontendDevice7366OpenSettings *pSettings = &pDevice->settings;
    const uint8_t *fw = NULL;

    pInitSettings = (NEXUS_FrontendDevice_P_7366_InitSettings *)BKNI_Malloc(sizeof(NEXUS_FrontendDevice_P_7366_InitSettings));
    if (!pInitSettings) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err;
    }
    BKNI_Memset(pInitSettings, 0, sizeof(NEXUS_FrontendDevice_P_7366_InitSettings));

    if (pSettings->spiDevice || pSettings->i2cDevice) {
        isExternal = true;
    }

#if BCHP_CHIP==7366
    /* Soft reset of the acquisition processor */
    {
        uint32_t val;

        /* assert sw_init signals */
        val = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET);
#if (BCHP_VER < BCHP_VER_B0)
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_bac_sw_init_MASK;
#endif
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_stb_chan_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_wb_stat_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_wb_stat_top0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_mdac_cal_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec0_sw_init_MASK;
#if (BCHP_VER >= BCHP_VER_B0)
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds0_top3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds0_top2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds0_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds0_top0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds1_top3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds1_top2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds1_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds1_top0_sw_init_MASK;
#else
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top0_sw_init_MASK;
#endif
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_fsk_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_leap_sw_init_MASK;
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET, val);

        /* release sw_init signals */
        val = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR);
#if (BCHP_VER < BCHP_VER_B0)
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_bac_sw_init_MASK;
#endif
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_stb_chan_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_wb_stat_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_wb_stat_top0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_mdac_cal_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec0_sw_init_MASK;
#if (BCHP_VER >= BCHP_VER_B0)
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds0_top3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds0_top2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds0_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds0_top0_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds1_top3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds1_top2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds1_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds1_top0_sw_init_MASK;
#else
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top3_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top2_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top1_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top0_sw_init_MASK;
#endif
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_fsk_top_sw_init_MASK;
        val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_leap_sw_init_MASK;
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR, val);
    }
#endif

    BHAB_7366_GetDefaultSettings(&pInitSettings->habSettings);
    {
#if NEXUS_MODE_driver
        if (Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_7366, &pInitSettings->habSettings.pImgContext, &pDevice->imgInterface ) == NEXUS_SUCCESS) {
            pInitSettings->habSettings.pImgInterface = &pDevice->imgInterface;
        } else {
            pInitSettings->habSettings.pImgContext = NULL;
        }
#else
        pInitSettings->habSettings.pImgInterface = &BHAB_SATFE_IMG_Interface;
        pInitSettings->habSettings.pImgContext = &BHAB_7366_IMG_Context;
#endif
    }

    regHandle = (void *)g_pCoreHandles->reg;
    if (isExternal) {
        if (pSettings->spiDevice) {
            BDBG_MSG(("Configuring for SPI"));
            pInitSettings->habSettings.chipAddr = pSettings->spiAddr;
            pInitSettings->habSettings.isSpi = true;
            regHandle = (void *)NEXUS_Spi_GetRegHandle(pSettings->spiDevice);
        } else if (pSettings->i2cDevice) {
            BDBG_MSG(("Configuring for I2C"));
            pInitSettings->habSettings.chipAddr = pSettings->i2cAddr;
            pInitSettings->habSettings.isSpi = false;
            regHandle = (void *)NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
        } else {
            regHandle = NULL;
        }
        pInitSettings->habSettings.isMtsif = true;
        if(pSettings->isrNumber) {
            BDBG_MSG(("Configuring for external interrupt"));
            pInitSettings->habSettings.interruptEnableFunc = NEXUS_Frontend_P_7366_IsrControl_isr;
            pInitSettings->habSettings.interruptEnableFuncParam = (void*)pSettings->isrNumber;
        }
        else if(pSettings->gpioInterrupt){
            BDBG_MSG(("Configuring for GPIO interrupt"));
            pInitSettings->habSettings.interruptEnableFunc = NEXUS_Frontend_P_7366_GpioIsrControl_isr;
            pInitSettings->habSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
        }
        BDBG_ASSERT(regHandle);
    }
    BDBG_MSG(("Calling BHAB_Open"));
    errCode = BHAB_Open(&habHandle, regHandle, &pInitSettings->habSettings);
    BDBG_MSG(("Calling BHAB_Open...Done: hab: %p",(void *)habHandle));
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    if (isExternal) {
        if(pSettings->isrNumber) {
            BDBG_MSG(("Connecting external interrupt"));
            errCode = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                                 NEXUS_Frontend_P_7366_L1_isr,
                                                 (void *)pDevice,
                                                 0);
            if ( errCode != BERR_SUCCESS )
            {
                errCode = BERR_TRACE(errCode);
                goto err;
            }
        }
        else if(pSettings->gpioInterrupt){
            BDBG_MSG(("Connecting GPIO interrupt"));
            NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt,
                                                 NEXUS_Frontend_P_7366_L1_isr,
                                                 (void *)pDevice,
                                                 0);
        }

    } else {
        BDBG_MSG(("Connecting internal interrupt"));
        errCode = NEXUS_Core_ConnectInterrupt(NEXUS_FRONTEND_7366_INTERRUPT,
                                             NEXUS_Frontend_P_7366_L1_isr,
                                             (void *)pDevice,
                                             0);
        if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }
        NEXUS_Core_EnableInterrupt(NEXUS_FRONTEND_7366_INTERRUPT);
    }

    errCode = BSAT_7366_GetDefaultSettings(&pInitSettings->satSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BSAT_Open(&satHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->satSettings); /* CHP and INT are unused by SAT */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BWFE_7366_GetDefaultSettings(&pInitSettings->wfeSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BWFE_Open(&wfeHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->wfeSettings); /* CHP and INT are unused by WFE */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BDSQ_7366_GetDefaultSettings(&pInitSettings->dsqSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BDSQ_Open(&dsqHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->dsqSettings); /* CHP and INT are unused by DSQ */
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

#if NEXUS_HAS_FSK
    errCode = BFSK_7366_GetDefaultSettings(&pInitSettings->fskSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }

    errCode = BFSK_Open(&fskHandle, g_pCoreHandles->chp, habHandle, NULL, &pInitSettings->fskSettings);
    if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }
#endif

    pDevice->satDevice->habHandle = habHandle;
    pDevice->satDevice->satHandle = satHandle;
    pDevice->satDevice->wfeHandle = wfeHandle;
    pDevice->satDevice->dsqHandle = dsqHandle;

    BDBG_MSG(("hab: %p, sat: %p, wfe: %p, dsq: %p",(void *)habHandle,(void *)satHandle,(void *)wfeHandle,(void *)dsqHandle));

#if NEXUS_FRONTEND_7366_USE_DRAM
    if (!isExternal)
    {
        NEXUS_MemoryAllocationSettings memSettings;

        BDBG_MSG(("Configuring 7366 with external memory"));
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
        memSettings.alignment = 0x100000;
        errCode = NEXUS_Memory_Allocate(0x100000,&memSettings,&pDevice->leapBuffer);
        if (errCode) {
            BDBG_ERR(("Unable to allocate memory for 7366 LEAP"));
            goto err;
        }
        pInitSettings->hab7366Settings.bUseInternalMemory = false;
        pInitSettings->hab7366Settings.pRam = pDevice->leapBuffer;
        pInitSettings->hab7366Settings.physAddr = NEXUS_AddrToOffset(pDevice->leapBuffer);
        pInitSettings->hab7366Settings.heap = NEXUS_Core_P_AddressToHeap(pDevice->leapBuffer, NULL);

        BDBG_MSG(("pRam: %p, physAddr: 0x%08x",pInitSettings->hab7366Settings.pRam,pInitSettings->hab7366Settings.physAddr));
        errCode = BHAB_7366_Configure(pDevice->satDevice->habHandle, &pInitSettings->hab7366Settings);
        if (errCode) {
            BERR_TRACE(errCode);
            goto err;
        }
    } else {
#endif
        pInitSettings->hab7366Settings.bUseInternalMemory = true;
        pInitSettings->hab7366Settings.physAddr = 0;
        pInitSettings->hab7366Settings.pRam = 0;
        errCode = BHAB_7366_Configure(pDevice->satDevice->habHandle, &pInitSettings->hab7366Settings);
        if (errCode) { BERR_TRACE(NEXUS_OS_ERROR); goto err; }
#if NEXUS_FRONTEND_7366_USE_DRAM
    }
#endif

#if NEXUS_HAS_FSK
    pDevice->fskHandle = fskHandle;
#endif

    /* Determine number of channels -- they will be opened later */
    BSAT_GetTotalChannels(pDevice->satDevice->satHandle, &pDevice->numChannels);
    BDBG_MSG(("frontend has %d channels",pDevice->numChannels));
    if ( pDevice->numChannels > NEXUS_7366_MAX_FRONTEND_CHANNELS )
    {
        BDBG_WRN(("This 7366 device supports more than the expected number of channels. Unexpected channels will not be initialized."));
    }
    pDevice->satDevice->numChannels = pDevice->numChannels;

    /* Open all channels prior to InitAp */
    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        BSAT_ChannelSettings bsatChannelSettings;
        BSAT_GetChannelDefaultSettings(pDevice->satDevice->satHandle, i, &bsatChannelSettings);
        errCode = BSAT_OpenChannel(pDevice->satDevice->satHandle, &pDevice->satChannels[i], i, &bsatChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    /* Determine number of inputs */
    errCode = BWFE_GetTotalChannels(pDevice->satDevice->wfeHandle, &pDevice->wfeInfo);
    BDBG_ASSERT(!errCode);
    pDevice->numAdc = pDevice->wfeInfo.numChannels;
    pDevice->satDevice->numWfe = pDevice->numAdc;

    BDBG_MSG(("pDevice->satDevice->wfeHandle: %p",(void *)pDevice->satDevice->wfeHandle));

    BDBG_WRN(("Initializing 7366 Frontend core..."));
    /* Initialize the acquisition processor */
#if !NEXUS_MODE_driver
    fw = bcm7366_ap_image;
#endif
    errCode = BHAB_InitAp(pDevice->satDevice->habHandle, fw);
    if ( errCode ) {
        BDBG_ERR(("Device initialization failed..."));

        errCode = BERR_TRACE(errCode);
        goto err;
    }
    BDBG_WRN(("Initializing 7366 core... Done"));

    if (isExternal) {
        /* Successfully opened the 7366.  Connect interrupt */
        BHAB_GetInterruptEventHandle(pDevice->satDevice->habHandle, &pDevice->isrEvent);
        pDevice->isrCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_7366_IsrCallback, pDevice);
        if ( NULL == pDevice->isrCallback )
        {
            errCode = BERR_TRACE(BERR_OS_ERROR);
            goto err;
        }
    }

    /* Open WFE Channels */
    for (i=0; i < pDevice->wfeInfo.numChannels; i++) {
        BWFE_ChannelSettings wfeChannelSettings;
        BWFE_GetChannelDefaultSettings(pDevice->satDevice->wfeHandle, i, &wfeChannelSettings);
        errCode = BWFE_OpenChannel(pDevice->satDevice->wfeHandle, &pDevice->satDevice->wfeChannels[i], i, &wfeChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open wfe channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
        errCode = BWFE_GetWfeReadyEventHandle(pDevice->satDevice->wfeChannels[i], &pDevice->satDevice->wfeReadyEvent[i]);
        if ( errCode ) {
            BDBG_ERR(("Unable to retrieve ready event for wfe channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    /* Open DSQ Channels */
    BDSQ_GetTotalChannels(pDevice->satDevice->dsqHandle, &pDevice->numDsqChannels);
    for (i=0; i < pDevice->numDsqChannels; i++) {
        BDSQ_ChannelSettings dsqChannelSettings;
        BDSQ_GetChannelDefaultSettings(pDevice->satDevice->dsqHandle, i, &dsqChannelSettings);
        errCode = BDSQ_OpenChannel(pDevice->satDevice->dsqHandle, &pDevice->satDevice->dsqChannels[i], i, &dsqChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open dsq channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    BDSQ_Reset(pDevice->satDevice->dsqHandle);

#ifdef NEXUS_HAS_FSK
    /* Open FSK Channels */
    BFSK_GetTotalChannels(pDevice->fskHandle, &pDevice->numFskChannels);

    for (i=0; i < pDevice->numFskChannels; i++) {
        BFSK_ChannelSettings fskChannelSettings;

        BFSK_GetChannelDefaultSettings(pDevice->fskHandle, i, &fskChannelSettings);
        errCode = BFSK_OpenChannel(pDevice->fskHandle, &pDevice->fskChannels[i], i, &fskChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open fsk channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err;
        }
    }

    {
        BKNI_EventHandle event;

        /* hook up the FTM data ready callback*/
        for(i=0; i<pDevice->numFskChannels; i++){
            errCode = BFSK_GetRxEventHandle(pDevice->fskChannels[i], &event);
            if (errCode) {
                errCode = BERR_TRACE(errCode);
                goto err;
            }

            pDevice->ftmEventCallback[i] = NEXUS_RegisterEvent(event, NEXUS_Frontend_P_7366_FtmEventCallback, pDevice);
            if (!pDevice->ftmEventCallback[i]) {
                errCode = BERR_TRACE(NEXUS_UNKNOWN);
                goto err;
            }
        }
    }
#endif

#if NEXUS_HAS_MXT
    {
        /* open MXT */
        BERR_Code rc;
        BMXT_Settings mxtSettings;
        BDBG_MSG(("NEXUS_FrontendDevice_Open7366: configuring MXT"));

        if (isExternal) {
            BMXT_4548_GetDefaultSettings(&mxtSettings);
            mxtSettings.hHab = pDevice->satDevice->habHandle;
        } else {
            BMXT_7366_GetDefaultSettings(&mxtSettings);
#if (BCHP_CHIP==7366 || BCHP_CHIP==7399) && (BCHP_VER >= BCHP_VER_B0)
            mxtSettings.chipRev = BMXT_ChipRev_eB0;
#endif
        }

        for (i=0; i<BMXT_NUM_MTSIF; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = true;
            NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
            mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        }

        mxtSettings.MtsifTxCfg[0].TxClockPolarity = 0;

        rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, g_pCoreHandles->chp, g_pCoreHandles->reg, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;
        rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
        if (rc!=BERR_SUCCESS) goto err;
    }
#endif
    if (isExternal) {
        /* If the 7366 is being used as a 4548, then the pinmuxing needs to be added for MTSIF on the 7366 */
        uint32_t val;
        uint32_t addr;
        BERR_Code e;

        addr = 0xf0404120; /* SUN_TOP_CTRL PIN_MUX_CTRL_08 */
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, addr, &val);
        val &= 0x0FF00000;
        val |= 0x01100000;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, addr, &val);

        addr = 0xf0404124; /* SUN_TOP_CTRL PIN_MUX_CTRL_09 */
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, addr, &val);
        val &= 0x0F00FFFF;
        val |= 0x10110000;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, addr, &val);

        addr = 0xf0404128; /* SUN_TOP_CTRL PIN_MUX_CTRL_10 */
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, addr, &val);
        val &= 0x00000000;
        val |= 0x11111111;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, addr, &val);

        addr = 0xf040412c; /* SUN_TOP_CTRL PIN_MUX_CTRL_11 */
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, addr, &val);
        val &= 0x000000F0;
        val |= 0x11111101;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, addr, &val);

        addr = 0xf0404130; /* SUN_TOP_CTRL PIN_MUX_CTRL_12 */
        e = BHAB_ReadRegister(pDevice->satDevice->habHandle, addr, &val);
        val &= 0x000000FF;
        val |= 0x00000011;
        BHAB_WriteRegister(pDevice->satDevice->habHandle, addr, &val);

        /* If the 7366 is being used as a 4548, then MTSIF controls may apply. */
        if (pSettings->mtsif[0].clockRate != 0 || pSettings->mtsif[1].clockRate != 0) {
            /* We catch MTSIF1 because 1 & 0 are internally swapped. If the platform code knows this, they may set [1] instead. */
            unsigned clockRate = pSettings->mtsif[1].clockRate ? pSettings->mtsif[1].clockRate : pSettings->mtsif[0].clockRate;
            unsigned divider;

            e = BHAB_ReadRegister(pDevice->satDevice->habHandle, 0x004e0344, &val);
            switch (clockRate) {
            case 81000000: /* 81 MHz. */
                divider = 40; break;
            case 95000000: /* 95.5 MHz. */
            case 95500000:
                divider = 34; break;
            case 98000000: /* 98.2 MHz. */
            case 98100000:
            case 98200000:
                divider = 33; break;
            case 101000000: /* 101.25 MHz. */
            case 101250000:
            case 101500000:
                divider = 32; break;
            case 104000000: /* 104.5 MHz. Also accept 104 Mhz. */
            case 104500000:
                divider = 31; break;
            case 108000000: /* 108 MHz. */
                divider = 30; break;
            case 135000000: /* 135 MHz. Too fast for 40nm backends, but the default for 7366/4548. */
                divider = 24; break;
            default:
                BDBG_WRN(("NEXUS_FrontendDevice_Open7366: Unrecognized rate: %u, defaulting to 108MHz.",clockRate));
                divider = 30;
                break;
            }
            val &= ~0x000001fe;
            val |= (divider << 1);
            BHAB_WriteRegister(pDevice->satDevice->habHandle, 0x004e0344, &val);
        }
        if (pSettings->mtsif[0].driveStrength != 0 || pSettings->mtsif[1].driveStrength != 0) {
            /* We catch MTSIF1 because 1 & 0 are internally swapped. If the platform code knows this, they may set [1] instead. */
            /* However, we control only the MTSIF1 pads, because they are the external MTSIF. */
            unsigned driveStrength = pSettings->mtsif[1].driveStrength ? pSettings->mtsif[1].driveStrength : pSettings->mtsif[0].driveStrength;
            unsigned str;

            e = BHAB_ReadRegister(pDevice->satDevice->habHandle, 0x004040A4, &val);

            switch (driveStrength) {
            case 2:
                str = 0; break;
            case 4:
                str = 1; break;
            case 6:
                str = 2; break;
            case 8: /* default/reset */
                str = 3; break;
            case 10:
                str = 4; break;
            case 12:
                str = 5; break;
            case 14:
                str = 6; break;
            case 16:
                str = 7; break;
            default:
                BDBG_WRN(("NEXUS_FrontendDevice_Open7366: Unrecognized drive strength: %u, defaulting to 8 mA.",driveStrength));
                str = 3; break;
            }
            val &= 0xFFFFFE3F;
            val |= str<<6;
            BHAB_WriteRegister(pDevice->satDevice->habHandle, 0x004040A4, &val);
        }
    }

    pDevice->pGenericDeviceHandle->openPending = false;

    BDBG_MSG(("Returning from NEXUS_FrontendDevice_P_Init7366"));
    BDBG_MSG(("pDevice->satDevice->wfeHandle: %p",(void *)pDevice->satDevice->wfeHandle));

    BKNI_Free(pInitSettings);
    return NEXUS_SUCCESS;
err:
    if (pInitSettings)
        BKNI_Free(pInitSettings);
    return NEXUS_UNKNOWN;
}


NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open7366(unsigned index, const NEXUS_FrontendDevice7366OpenSettings *pSettings)
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_7366Device *pDevice=NULL;

    BSTD_UNUSED(index);

    /* Check is maintained in case we need to introduce a list of devices later. */
    if (pDevice == NULL) {
        NEXUS_FrontendSatDeviceSettings satDeviceSettings;
        BERR_Code errCode;

        BDBG_MSG(("Opening new 7366 device"));

        pFrontendDevice = NEXUS_FrontendDevice_P_Create();
        if (NULL == pFrontendDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

        pDevice = BKNI_Malloc(sizeof(*pDevice));
        if (NULL == pDevice) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_7366Device);
        pDevice->settings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
        pDevice->A8299_control = 0x88; /* 13v, 13v default */

        NEXUS_Frontend_P_Sat_GetDefaultDeviceSettings(&satDeviceSettings);

        pDevice->satDevice = NEXUS_Frontend_P_Sat_Create_Device(&satDeviceSettings);
        if (pDevice->satDevice == NULL) { BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err; }

        errCode = NEXUS_FrontendDevice_P_Init7366(pDevice);
        if (errCode)
            goto err;
    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = 0x7366;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_7366_GetCapabilities;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eSatellite;
    pFrontendDevice->close = NEXUS_Frontend_P_7366_DestroyDevice;
    pFrontendDevice->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Get7366Capabilities;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_7366_Standby;

    return pFrontendDevice;

err:
    if (pDevice)
        NEXUS_Frontend_P_7366_DestroyDevice(pDevice);
    return NULL;
}

static void NEXUS_Frontend_P_Uninit7366(NEXUS_7366Device *pDevice)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7366Device);

    BDBG_MSG(("Closing 7366 device %p handles", (void *)pDevice));

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle) {
        if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
            BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
        }
    }
#endif

#if 0
    for ( i = 0; i < pDevice->numAdc; i++ )
    {
        if (pDevice->diseqcCallbackEvents[i]) {
            NEXUS_UnregisterEvent(pDevice->diseqcCallbackEvents[i]);
        }
    }
#endif

#if NEXUS_HAS_FSK
    if (pDevice->fskHandle) {
        for (i=0; i < pDevice->numFskChannels; i++)
        {
            if(pDevice->ftmEventCallback[i]){
                NEXUS_UnregisterEvent(pDevice->ftmEventCallback[i]);
            }
            if (pDevice->fskChannels[i]) {
                BFSK_CloseChannel(pDevice->fskChannels[i]);
            }
        }
        BFSK_Close(pDevice->fskHandle);
    }
#endif

    if (pDevice->satDevice) {
        if (pDevice->satDevice->dsqHandle) {
            for (i=0; i < pDevice->numDsqChannels; i++)
            {
                if (pDevice->satDevice->dsqChannels[i]) {
                    BDSQ_CloseChannel(pDevice->satDevice->dsqChannels[i]);
                    pDevice->satDevice->dsqChannels[i] = NULL;
                }
            }
            BDSQ_Close(pDevice->satDevice->dsqHandle);
            pDevice->satDevice->dsqHandle = NULL;
        }
    }

    for ( i=0; i < pDevice->numChannels && NULL != pDevice->satChannels[i]; i++) {
        BSAT_CloseChannel(pDevice->satChannels[i]);
        pDevice->satChannels[i] = NULL;
    }

    if (pDevice->satDevice) {
        for ( i=0; i < pDevice->wfeInfo.numChannels; i++ )
        {
            if (pDevice->satDevice->wfeChannels[i]) {
                BDBG_MSG(("Closing WFE[%d]",i));
                BWFE_CloseChannel(pDevice->satDevice->wfeChannels[i]);
                pDevice->satDevice->wfeChannels[i] = NULL;
            }
        }
        if (pDevice->satDevice->wfeHandle) {
            BWFE_Close(pDevice->satDevice->wfeHandle);
            pDevice->satDevice->wfeHandle = NULL;
        }

        if (pDevice->satDevice->satHandle) {
            BSAT_Close(pDevice->satDevice->satHandle);
            pDevice->satDevice->satHandle = NULL;
        }
    }

    if (pDevice->isrCallback) {
        NEXUS_UnregisterEvent(pDevice->isrCallback);
        pDevice->isrCallback = NULL;
    }

    if (pDevice->settings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber);
    } else if (pDevice->settings.gpioInterrupt) {
        NEXUS_GpioSettings gpioSettings;
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->settings.gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        NEXUS_Gpio_SetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
    } else {
        NEXUS_Core_DisableInterrupt(NEXUS_FRONTEND_7366_INTERRUPT);
        NEXUS_Core_DisconnectInterrupt(NEXUS_FRONTEND_7366_INTERRUPT);
    }

    if (pDevice->satDevice) {
        if (pDevice->satDevice->habHandle) {
            BHAB_Close(pDevice->satDevice->habHandle);
            pDevice->satDevice->habHandle = NULL;
        }
    }

    if (pDevice->leapBuffer) {
        NEXUS_Memory_Free(pDevice->leapBuffer);
        pDevice->leapBuffer = NULL;
    }
}

void NEXUS_Frontend_GetDefault7366Settings( NEXUS_7366FrontendSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_Open7366( const NEXUS_7366FrontendSettings *pSettings )
{
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_7366Device *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_FrontendSatChannelSettings satChannelSettings;
    BREG_I2C_Handle i2cHandle = NULL;

    if(pSettings->device == NULL) {
        NEXUS_FrontendDevice7366OpenSettings openSettings;
        NEXUS_FrontendDevice_GetDefault7366OpenSettings(&openSettings);
        pFrontendDevice = NEXUS_FrontendDevice_Open7366(0, &openSettings);
        pDevice = (NEXUS_7366Device *)pFrontendDevice->pDevice;
    }
    else {
        pFrontendDevice = pSettings->device;
        pDevice = (NEXUS_7366Device *)pSettings->device->pDevice;
    }

    /* Return previously opened frontend handle. */
    if (pSettings->channelNumber >= pDevice->numChannels) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (pDevice->handles[pSettings->channelNumber])
        return pDevice->handles[pSettings->channelNumber];

    /* Otherwise, open new frontend */
    BDBG_MSG(("Creating channel %u", pSettings->channelNumber));

    if (pDevice->settings.diseqc.i2cDevice) {
        i2cHandle = NEXUS_I2c_GetRegHandle(pDevice->settings.diseqc.i2cDevice, NEXUS_MODULE_SELF);
        BDBG_ASSERT(NULL != i2cHandle);
    }

    /* Open channel */
    NEXUS_Frontend_P_Sat_GetDefaultChannelSettings(&satChannelSettings);
    satChannelSettings.satDevice = pDevice->satDevice;
    satChannelSettings.satChannel = pDevice->satChannels[pSettings->channelNumber];
#define B_SAT_CHIP 7366
    satChannelSettings.satChip = B_SAT_CHIP;
    satChannelSettings.channelIndex = pSettings->channelNumber;
    satChannelSettings.pCloseParam = pDevice;
    satChannelSettings.pDevice = pDevice;
    satChannelSettings.closeFunction = NEXUS_Frontend_P_7366_CloseCallback;
    satChannelSettings.diseqcIndex = 0;
    if (pDevice->settings.diseqc.i2cDevice) {
        satChannelSettings.getDiseqcChannelHandle = NEXUS_Frontend_P_7366_GetDiseqcChannelHandle;
        satChannelSettings.getDiseqcChannelHandleParam = pDevice;
    }
    satChannelSettings.capabilities.diseqc = true;

    satChannelSettings.setVoltage = NEXUS_Frontend_P_7366_SetVoltage;
    satChannelSettings.i2cRegHandle = i2cHandle; /* due to module locking, we need to save our register handle for Diseqc voltage control */
#if 0
    satChannelSettings.getDiseqcEventHandle = NEXUS_Frontend_P_7366_GetDiseqcEventHandle;
    satChannelSettings.getDiseqcAppCallback = NEXUS_Frontend_P_7366_GetDiseqcAppCallback;
    satChannelSettings.setDiseqcAppCallback = NEXUS_Frontend_P_7366_SetDiseqcAppCallback;
#endif
    satChannelSettings.getDefaultDiseqcSettings = NEXUS_Frontend_P_7366_GetDefaultDiseqcSettings;
#if NEXUS_HAS_FSK
    satChannelSettings.getFskChannelHandle = NEXUS_Frontend_P_7366_GetFskChannelHandle;
#endif

    satChannelSettings.wfeHandle = pDevice->satDevice->wfeHandle;

    satChannelSettings.deviceClearSpectrumCallbacks = NEXUS_Frontend_P_7366_ClearSpectrumCallbacks;

    frontend = NEXUS_Frontend_P_Sat_Create_Channel(&satChannelSettings);
    if ( !frontend )
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_7366_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        goto err;
    }
    {
        BERR_Code errCode;
        NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)frontend->pDeviceHandle;
        BFEC_SystemVersionInfo info;
        /* populate firmware version */
        errCode = BHAB_GetVersionInfo(pDevice->satDevice->habHandle, &info);
        if (errCode) BERR_TRACE(errCode);
        else {
            pSatChannel->satDevice->type.firmwareVersion.buildId = info.firmware.buildId;
            pSatChannel->satDevice->type.firmwareVersion.buildType = info.firmware.buildType;
            pSatChannel->satDevice->type.firmwareVersion.major = info.firmware.majorVersion;
            pSatChannel->satDevice->type.firmwareVersion.minor = info.firmware.minorVersion;

        }
    }
    frontend->tuneSatellite = NEXUS_Frontend_P_7366_TuneSatellite;
    frontend->untune = NEXUS_Frontend_P_7366_Untune;
    frontend->reapplyTransportSettings = NEXUS_Frontend_P_7366_ReapplyTransportSettings;
    frontend->pGenericDeviceHandle = pFrontendDevice;
    frontend->getSatelliteAgcStatus = NEXUS_Frontend_P_7366_GetSatelliteAgcStatus;
    frontend->getSatelliteRuntimeSettings = NEXUS_Frontend_P_Get7366RuntimeSettings;
    frontend->setSatelliteRuntimeSettings = NEXUS_Frontend_P_Set7366RuntimeSettings;

    frontend->standby = NEXUS_Frontend_P_7366_Standby;

#if (BCHP_CHIP==7366 || BCHP_CHIP==7399) && (BCHP_VER >= BCHP_VER_B0)
#define NEXUS_7366_FRONTEND_MTSIF_OFFSET 18
#else
#define NEXUS_7366_FRONTEND_MTSIF_OFFSET 8
#endif
    /* preconfigure mtsif settings so platform doesn't need to */
    frontend->userParameters.isMtsif = true;
    frontend->mtsif.inputBand = pSettings->channelNumber + NEXUS_7366_FRONTEND_MTSIF_OFFSET;
                            /* IB for demod 0 is 8/18, demod 1 is 9/19, etc. Offset channel by 8/18 to compensate. */

    pDevice->handles[pSettings->channelNumber] = frontend;

    return frontend;

err:
    return NULL;
}

static void NEXUS_Frontend_P_7366_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_7366Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7366Device);

    /* Mark handle as destroyed */
    if ( handle ) {
        for ( i = 0; i < pDevice->numChannels; i++ ) {
            if ( handle == pDevice->handles[i] ) {
                pDevice->handles[i] = NULL;
                break;
            }
        }
        BDBG_ASSERT(i < pDevice->numChannels);
    }

}

static void NEXUS_Frontend_P_7366_DestroyDevice(void *handle)
{
    unsigned i;
    NEXUS_7366Device *pDevice = (NEXUS_7366Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7366Device);

    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        if ( NULL != pDevice->handles[i] )
        {
            BDBG_ERR(("All channels must be closed before destroying device"));
            BDBG_ASSERT(NULL == pDevice->handles[i]);
        }
    }

    NEXUS_Frontend_P_Uninit7366(pDevice);

    BDBG_MSG(("Destroying 7366 device %p", (void *)pDevice));

    if (pDevice->pGenericDeviceHandle) {
        BKNI_Free(pDevice->pGenericDeviceHandle);
        pDevice->pGenericDeviceHandle = NULL;
    }

    if (pDevice->satDevice) {
        BKNI_Free(pDevice->satDevice);
        pDevice->satDevice = NULL;
    }
#if 0
    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_7366Device, node);
#endif
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7366Device);
    BKNI_Free(pDevice);
}

void NEXUS_FrontendDevice_P_7366_S3Standby(NEXUS_7366Device *pDevice)
{
    NEXUS_Frontend_P_Uninit7366(pDevice);
}

static NEXUS_Error NEXUS_FrontendDevice_P_7366_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_7366Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_7366Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7366Device);

    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_7366_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_7366_S3Standby(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();
    } else if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_7366_Standby: Waking up..."));

        rc = NEXUS_FrontendDevice_P_Init7366(pDevice);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_7366_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7366Device *p7366Device;

    BDBG_OBJECT_ASSERT(pSatChannel, NEXUS_SatChannel);
    p7366Device = pSatChannel->settings.pDevice;

    BDBG_MSG(("NEXUS_Frontend_P_7366_Standby: standby %p(%d) %s", handle, pSatChannel->channel, enabled ? "enabled" : "disabled"));

    BDBG_MSG(("Restoring handles on %p",(void *)pSatChannel));
    /* update/restore handles */
    pSatChannel->satChannel = p7366Device->satChannels[pSatChannel->channel];

    if (pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Unregistering events on %p",(void *)pSatChannel));
        NEXUS_Frontend_P_Sat_UnregisterEvents(pSatChannel);
    } else if (pSettings->mode != NEXUS_StandbyMode_eDeepSleep && pSatChannel->frontendHandle->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Registering events on %p",(void *)pSatChannel));
        BDBG_MSG(("p7366Device->satDevice->wfeHandle: %p",(void *)p7366Device->satDevice->wfeHandle));
        NEXUS_Frontend_P_Sat_RegisterEvents(pSatChannel);
    }

    BDBG_MSG(("Done with standby configuration on %p",(void *)pSatChannel));
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Get7366Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_7366Device *p7366Device = (NEXUS_7366Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);

    BDBG_OBJECT_ASSERT(p7366Device, NEXUS_7366Device);

    BKNI_Memset(pCapabilities,0,sizeof(*pCapabilities));
    pCapabilities->numAdc = p7366Device->numAdc;
    pCapabilities->numChannels = p7366Device->numChannels;
    pCapabilities->externalBert = false;

    return rc;
}

#if (BCHP_CHIP==7366) && (BCHP_VER >= BCHP_VER_B0)
#define EXTERNAL_BERT_CONTROL 1
#define EXT_BERT_PINMUX_CTRL BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15
#define EXT_BERT_PINMUX_MASK (BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_094_MASK | BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_093_MASK)
#define EXT_BERT_PINMUX_SHIFT BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_093_SHIFT
#define EXT_BERT_PINMUX_VAL (BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_094_SDS_BERT_DATA << 4 | BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_093_SDS_BERT_CLK)
#define EXT_BERT_SELECT BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0
#define EXT_BERT_SELECT_MASK BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0_bert_pinmux_ctrl_MASK
#define EXT_BERT_SELECT_SHIFT BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0_bert_pinmux_ctrl_SHIFT
#endif
static NEXUS_Error NEXUS_Frontend_P_Get7366RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7366Device *p7366Device = pSatChannel->settings.pDevice;
    bool externalBertEnabled = false;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    BDBG_OBJECT_ASSERT(p7366Device, NEXUS_7366Device);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->selectedAdc = pSatChannel->selectedAdc;
    pSatChannel->diseqcIndex = pSatChannel->selectedAdc;
    pSettings->externalBert.invertClock = false;

#if EXTERNAL_BERT_CONTROL
    {
        uint32_t val;
        val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_PINMUX_CTRL);
        val &= EXT_BERT_PINMUX_MASK;
        val >>= EXT_BERT_PINMUX_SHIFT;
        if (val == EXT_BERT_PINMUX_VAL) {
            val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_SELECT);
            val &= EXT_BERT_SELECT_MASK;
            val >>= EXT_BERT_SELECT_SHIFT;
            if (val == pSatChannel->channel)
                externalBertEnabled = true;
        }
    }
#endif
    pSettings->externalBert.enabled = externalBertEnabled;

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Set7366RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7366Device *p7366Device;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    p7366Device = pSatChannel->settings.pDevice;
    BDBG_OBJECT_ASSERT(p7366Device, NEXUS_7366Device);

    BDBG_MSG(("adc: %d, mask: 0x%08x",pSettings->selectedAdc,p7366Device->wfeInfo.availChannelsMask));
    /* Ensure the requested adc is within the value range, and advertised by the PI as being available */
    if (pSettings->selectedAdc > p7366Device->numAdc || !((1<<pSettings->selectedAdc) & p7366Device->wfeInfo.availChannelsMask) )
        return NEXUS_INVALID_PARAMETER;

    pSatChannel->selectedAdc = pSettings->selectedAdc;
    pSatChannel->diseqcIndex = pSettings->selectedAdc;
    p7366Device->wfeMap[pSatChannel->channel] = pSettings->selectedAdc;
    pSatChannel->satDevice->wfeMap[pSatChannel->channel] = pSettings->selectedAdc;

#if EXTERNAL_BERT_CONTROL
    {
        uint32_t val;
        val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_PINMUX_CTRL);
        val &= EXT_BERT_PINMUX_MASK;
        val >>= EXT_BERT_PINMUX_SHIFT;
        if (val == EXT_BERT_PINMUX_VAL) { /* external bert currently enabled */
            val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_SELECT);
            val &= EXT_BERT_SELECT_MASK;
            val >>= EXT_BERT_SELECT_SHIFT;
            if (val == pSatChannel->channel) { /* external bert is on this channel */
                if (!pSettings->externalBert.enabled) { /* matching demod, disable. Do not disable if not the matching channel. */
                    val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_PINMUX_CTRL);
                    val &= ~(EXT_BERT_PINMUX_MASK);
                    val |= p7366Device->cachedBertPinmux << EXT_BERT_PINMUX_SHIFT;
                    BREG_Write32(g_pCoreHandles->reg, EXT_BERT_PINMUX_CTRL, val);
                }
            } else { /* not matching demod, switch external bert demod output */
                val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_SELECT);
                val &= ~(EXT_BERT_SELECT_MASK);
                val |= pSatChannel->channel << EXT_BERT_SELECT_SHIFT;
                BREG_Write32(g_pCoreHandles->reg, EXT_BERT_SELECT, val);
            }
        } else { /* external bert not currently enabled */
            if (pSettings->externalBert.enabled) { /* enable on this demod */
                val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_PINMUX_CTRL);
                p7366Device->cachedBertPinmux = (val & EXT_BERT_PINMUX_MASK) >> EXT_BERT_PINMUX_SHIFT; /* store current value */
                val &= ~(EXT_BERT_PINMUX_MASK);
                val |= EXT_BERT_PINMUX_VAL << EXT_BERT_PINMUX_SHIFT;
                BREG_Write32(g_pCoreHandles->reg, EXT_BERT_PINMUX_CTRL, val);

                val = BREG_Read32(g_pCoreHandles->reg, EXT_BERT_SELECT);
                val &= ~(EXT_BERT_SELECT_MASK);
                val |= pSatChannel->channel << EXT_BERT_SELECT_SHIFT;
                BREG_Write32(g_pCoreHandles->reg, EXT_BERT_SELECT, val);
            }
        }
    }
#endif

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_7366_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    NEXUS_SatChannel *pSatChannel = (NEXUS_SatChannel *)handle;
    NEXUS_7366Device *p7366Device = pSatChannel->settings.pDevice;
    BERR_Code rc = NEXUS_SUCCESS;
    BSAT_ChannelStatus satStatus;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NULL != p7366Device);

    if (pSatChannel->satChip != B_SAT_CHIP) {
        return NEXUS_INVALID_PARAMETER;
    }

    errCode = BSAT_GetChannelStatus(pSatChannel->satChannel, &satStatus);
    if ( errCode ) {
        BDBG_MSG(("BSAT_GetChannelStatus returned %x",errCode));
        rc = errCode;
    } else {
        int i;
        BKNI_Memset(pStatus,0,sizeof(*pStatus));
        for (i=0; i<3; i++) {
            pStatus->agc[i].value = satStatus.agc.value[i];
            pStatus->agc[i].valid = (satStatus.agc.flags & 1<<i);
        }
    }

    return rc;
}

NEXUS_Error NEXUS_Frontend_P_Sat_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
void NEXUS_Frontend_P_Sat_Untune(void *handle);

static NEXUS_Error NEXUS_Frontend_P_7366_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_SatChannel *pSatDevice = handle;
    NEXUS_Error rc;

    rc = NEXUS_Frontend_P_SetMtsifConfig(pSatDevice->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }

#if 0
    p7366Device->useChannels[pSatDevice->channel] = true;
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Frontend_P_SetAdcPower(pSatDevice->frontendHandle);
#endif
#endif

    return NEXUS_Frontend_P_Sat_TuneSatellite(pSatDevice, pSettings);
}

static void NEXUS_Frontend_P_7366_Untune(void *handle)
{
    NEXUS_SatChannel *pSatDevice = handle;

    NEXUS_Frontend_P_UnsetMtsifConfig(pSatDevice->frontendHandle);

    NEXUS_Frontend_P_Sat_Untune(pSatDevice);

#if 0
    p7366Device->useChannels[pSatDevice->channel] = false;
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Frontend_P_SetAdcPower(pSatDevice->frontendHandle);
#endif
#endif

    return;
}

NEXUS_Error NEXUS_Frontend_P_7366_ReapplyTransportSettings(void *handle)
{
    NEXUS_SatChannel *pSatDevice = handle;
    NEXUS_Error rc;

    rc = NEXUS_Frontend_P_SetMtsifConfig(pSatDevice->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }

    return NEXUS_SUCCESS;
}

static BDSQ_ChannelHandle NEXUS_Frontend_P_7366_GetDiseqcChannelHandle(void *handle, int index)
{
    NEXUS_SatChannel *pSatDevice = handle;
    NEXUS_7366Device *p7366Device;
    p7366Device = pSatDevice->settings.pDevice;
    return p7366Device->satDevice->dsqChannels[index];
}

static NEXUS_Error NEXUS_Frontend_P_7366_SetVoltage(void *pDevice, NEXUS_FrontendDiseqcVoltage voltage)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SatChannel *pSatChannel = pDevice;
    NEXUS_7366Device *p7366Device = pSatChannel->settings.pDevice;

    /* Just in case DSQ book-keeping requires it: */
    BDSQ_SetVoltage(p7366Device->satDevice->dsqChannels[pSatChannel->diseqcIndex], voltage == NEXUS_FrontendDiseqcVoltage_e18v);

    if (pSatChannel->settings.i2cRegHandle)
    { /* Write voltage to A8299 */
        int channel = pSatChannel->diseqcIndex;
        uint8_t buf[2];
        uint8_t i2c_addr, shift, ctl;
        uint8_t A8299_control = p7366Device->A8299_control;

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

        p7366Device->A8299_control = A8299_control;

        BDBG_MSG(("A8299: channel=%d, i2c_addr=0x%X, ctl=0x%02X 0x%02X", channel, i2c_addr, buf[0], buf[1]));
        rc = BREG_I2C_WriteNoAddr(pSatChannel->settings.i2cRegHandle, i2c_addr, buf, 2);
        if (rc) return BERR_TRACE(rc);

    }

    return rc;
}

static void NEXUS_Frontend_P_7366_GetDefaultDiseqcSettings(void *pDevice, BDSQ_ChannelSettings *settings)
{
    NEXUS_SatChannel *pSatDevice = pDevice;
    NEXUS_7366Device *p7366Device = pSatDevice->settings.pDevice;
    BDSQ_GetChannelDefaultSettings(p7366Device->satDevice->dsqHandle, 0, settings);
}

static void NEXUS_FrontendDevice_P_7366_GetCapabilities(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_7366Device *pDevice = (NEXUS_7366Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7366Device);

    pCapabilities->numTuners = pDevice->numChannels;
}

static void NEXUS_Frontend_P_7366_ClearSpectrumCallbacks(void *handle)
{
    NEXUS_SatChannel *pSatDevice = (NEXUS_SatChannel *)handle;
    NEXUS_7366Device *p7366Device;
    unsigned i;

    BDBG_ASSERT(pSatDevice);
    p7366Device = pSatDevice->settings.pDevice;
    BDBG_OBJECT_ASSERT(p7366Device, NEXUS_7366Device);

    for (i=0; i < p7366Device->numChannels; i++) {
        NEXUS_SatChannel *pDevice = p7366Device->handles[i]->pDeviceHandle;
        if (pDevice) {
            if (pDevice->spectrumEventCallback) {
                NEXUS_UnregisterEvent(pDevice->spectrumEventCallback);
                pDevice->spectrumEventCallback = NULL;
            }
        }
    }

}

#if NEXUS_HAS_FSK
static BFSK_ChannelHandle NEXUS_Frontend_P_7366_GetFskChannelHandle(void *pDevice, int index)
{
    NEXUS_SatChannel *pSatDevice = pDevice;
    NEXUS_7366Device *p7366Device = pSatDevice->settings.pDevice;

    if((unsigned)index < p7366Device->numFskChannels)
        return p7366Device->fskChannels[index];
    else
        return NULL;
}

static void NEXUS_Frontend_P_7366_FtmEventCallback(void *context)
{
    NEXUS_SatChannel *pSatDevice;
    NEXUS_7366Device *p7366Device = context;
    unsigned i;

    for (i=0;i<NEXUS_7366_MAX_FRONTEND_CHANNELS;i++) {
        if (p7366Device->handles[i]) {
            pSatDevice = (NEXUS_SatChannel *)p7366Device->handles[i]->pDeviceHandle;
            if(pSatDevice->ftmCallback)
                NEXUS_TaskCallback_Fire(pSatDevice->ftmCallback);
        }
    }
}

#endif

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open7366(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice7366OpenSettings settings;
    int i;

    if (pSettings->satellite.enabled) {
        NEXUS_FrontendDevice_GetDefault7366OpenSettings(&settings);
        settings.i2cDevice = pSettings->i2cDevice;
        settings.i2cAddr = pSettings->i2cAddress;
        settings.gpioInterrupt = pSettings->gpioInterrupt;
        settings.isrNumber = pSettings->isrNumber;
        settings.spiDevice = pSettings->spiDevice;
        settings.spiAddr = 0x20;
        for (i=0; i < NEXUS_MAX_MTSIF; i++) {
            settings.mtsif[i] = pSettings->mtsif[i];
        }
        return NEXUS_FrontendDevice_Open7366(index, &settings);
    }
    return NULL;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open7366(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_7366FrontendSettings settings;

    if (pSettings->type == NEXUS_FrontendChannelType_eSatellite) {
        NEXUS_Frontend_GetDefault7366Settings(&settings);
        settings.device = pSettings->device;
        settings.channelNumber = pSettings->channelNumber;
        return NEXUS_Frontend_Open7366(&settings);
    }
    return NULL;
}
