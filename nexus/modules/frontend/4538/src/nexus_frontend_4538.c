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
#include "nexus_frontend_ast.h"
#include "priv/nexus_transport_priv.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "priv/nexus_spi_priv.h"
#include "bhab.h"
#include "bhab_4538.h"
#include "bhab_4538_fw.h"
#include "bhab_satfe_img.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#include "bast.h"
#include "bast_4538.h"
#include "bwfe.h"
#include "bwfe_4538.h"
#if NEXUS_HAS_MXT
#include "bmxt.h"
#include "bchp_hif_cpu_intr1.h"
#endif

BDBG_MODULE(nexus_frontend_4538);

BDBG_OBJECT_ID(NEXUS_4538Device);

/* set to 1 to enable L1 interrupt messages */
#define NEXUS_FRONTEND_DEBUG_IRQ 0

#define NEXUS_4538_MAX_WFE_CHANNELS 4

/* set to 1 to disable asynchronous initialization */
#define DISABLE_4538_ASYNC_INIT 0

typedef struct NEXUS_4538DiseqcParam {
    int diseqcChannel;
    void *device;
} NEXUS_4538DiseqcParam;

typedef struct NEXUS_4538Device
{
    BDBG_OBJECT(NEXUS_4538Device)
    BLST_S_ENTRY(NEXUS_4538Device) node;
    NEXUS_FrontendDevice4538OpenSettings settings;
    BAST_Handle astHandle;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrCallback;
    BAST_ChannelHandle astChannels[NEXUS_4538_MAX_FRONTEND_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_4538_MAX_FRONTEND_CHANNELS];
    uint32_t numChannels;   /* prototype to match BAST_GetTotalChannels */
    uint8_t numAdc;         /* prototype to match BAST_GetAdcSelect */
    BHAB_Handle hab;
    BWFE_Handle wfeHandle;
    BWFE_ChannelInfo wfeInfo;
    BWFE_ChannelHandle wfeChannels[NEXUS_4538_MAX_WFE_CHANNELS];
    int wfeMap[NEXUS_4538_MAX_FRONTEND_CHANNELS];
    bool useChannels[NEXUS_4538_MAX_FRONTEND_CHANNELS];
    bool enabled[NEXUS_4538_MAX_WFE_CHANNELS];
    BKNI_EventHandle diseqcEvents[NEXUS_4538_MAX_WFE_CHANNELS];
    NEXUS_EventCallbackHandle diseqcCallbackEvents[NEXUS_4538_MAX_WFE_CHANNELS];
    NEXUS_TaskCallbackHandle diseqcAppCallbacks[NEXUS_4538_MAX_WFE_CHANNELS];
    NEXUS_4538DiseqcParam diseqcParams[NEXUS_4538_MAX_WFE_CHANNELS];
    BKNI_EventHandle calibrationEvents[NEXUS_4538_MAX_WFE_CHANNELS];
    NEXUS_EventCallbackHandle calibrationCallbackEvents[NEXUS_4538_MAX_WFE_CHANNELS];
    NEXUS_EventCallbackHandle ftmEventCallback;
    BKNI_EventHandle spectrumDataReadyEvent;
    NEXUS_EventCallbackHandle spectrumEventCallback;
    NEXUS_EventCallbackHandle spectrumDataReadyEventCallback;
    NEXUS_TaskCallbackHandle spectrumDataReadyAppCallback[NEXUS_4538_MAX_FRONTEND_CHANNELS];
    uint32_t *spectrumDataPointer;
    unsigned spectrumDataLength;

    NEXUS_GpioHandle gpioHandle;
    NEXUS_ThreadHandle deviceOpenThread;
    BREG_I2C_Handle i2cRegHandle;
    BREG_SPI_Handle spiRegHandle;
    bool settingsInitialized;
    BIMG_Interface imgInterface;
} NEXUS_4538Device;

#if NEXUS_FRONTEND_4538_FORCE_3448
static bool b_use3448 = true;
static bool b_3448_detect = false;
#elif NEXUS_FRONTEND_4538_FORCE_3447
static bool b_use3448 = false;
static bool b_3448_detect = false;
#else
static bool b_use3448 = false;
static bool b_3448_detect = true;
#endif

static BLST_S_HEAD(devList, NEXUS_4538Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);

static void NEXUS_Frontend_P_4538_IsrControl_isr(bool enable, void *pParam);
static void NEXUS_Frontend_P_4538_GpioIsrControl_isr(bool enable, void *pParam);
static void NEXUS_Frontend_P_4538_L1_isr(void *param1, int param2);
static void NEXUS_Frontend_P_4538_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_4538_DestroyDevice(void *handle);
static void NEXUS_Frontend_P_4538_IsrCallback(void *pParam);
static NEXUS_Error NEXUS_Frontend_P_4538_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
static void NEXUS_Frontend_P_4538_Untune(void *handle);
static NEXUS_Error NEXUS_Frontend_P_4538_ReapplyTransportSettings(void *handle);
static void NEXUS_Frontend_P_4588_FtmEventCallback(void *context);
static NEXUS_Error NEXUS_Frontend_P_4538_RequestSpectrumData(void *handle, const NEXUS_FrontendSpectrumSettings *pSettings);
static void NEXUS_Frontend_P_4538_SpectrumEventCallback(void *pParam);
static void NEXUS_Frontend_P_4538_SpectrumDataReadyCallback(void *pParam);
static NEXUS_Error NEXUS_FrontendDevice_P_4538_GetStatus(void * handle, NEXUS_FrontendDeviceStatus *pStatus);
static BAST_ChannelHandle NEXUS_Frontend_P_4538_GetDiseqcChannelHandle(void *handle, int index);
static NEXUS_EventCallbackHandle NEXUS_Frontend_P_4538_GetDiseqcEventHandle(void *handle, int index);
static NEXUS_TaskCallbackHandle NEXUS_Frontend_P_4538_GetDiseqcAppCallback(void *handle, int index);
static void NEXUS_Frontend_P_4538_SetDiseqcAppCallback(void *handle, int index, NEXUS_TaskCallbackHandle appCallback);
static void NEXUS_Frontend_P_4538_DiseqcEventHandler(void *pParam);
static void NEXUS_Frontend_P_4538_CalibrationEventHandler(void *pParam);
#if NEXUS_POWER_MANAGEMENT
static void NEXUS_Frontend_P_SetAdcPower(NEXUS_FrontendHandle handle);
#endif
static NEXUS_Error NEXUS_Frontend_P_4538_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus);
static void NEXUS_FrontendDevice_P_4538_GetCapabilities(void * handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);

static NEXUS_Error NEXUS_FrontendDevice_P_Get4538Capabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities);
static NEXUS_Error NEXUS_Frontend_P_Get4538RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_Set4538RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings);

static NEXUS_Error NEXUS_FrontendDevice_P_4538_Standby(void * handle, const NEXUS_StandbySettings *pSettings);
static NEXUS_Error NEXUS_Frontend_P_4538_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);

#ifndef NEXUS_FRONTEND_3447_POWERUP_VALUES
#define NEXUS_FRONTEND_3447_POWERUP_VALUES 0x32, 0x10
#endif

#if (NEXUS_FRONTEND_HAS_A8299_DISEQC)
#if (NEXUS_PLATFORM==97429) || (NEXUS_PLATFORM==974295)
/* 4538 SATIP board is 7429 + 4538, with A8299 to control diseqc.  This define covers that configuration. */
/* 7445 DBS is 7445 + 2x4538, with A8299 to control diseqc. */
#define A8299_DISEQC_PINMUX (BAST_4538_DSEC_PIN_MUX_GPIO18_GPIO19 << BAST_4538_DSEC_PIN_MUX_CHAN0_SHIFT) | \
                      (BAST_4538_DSEC_PIN_MUX_TXOUT0_TXEN0 << BAST_4538_DSEC_PIN_MUX_CHAN1_SHIFT) | \
                      (BAST_4538_DSEC_PIN_MUX_GPO6_GPO5 << BAST_4538_DSEC_PIN_MUX_CHAN2_SHIFT) | \
                      (BAST_4538_DSEC_PIN_MUX_GPO4_GPO3 << BAST_4538_DSEC_PIN_MUX_CHAN3_SHIFT) | \
                       BAST_4538_DSEC_PIN_MUX_TXOUT_GPIO18;
#elif (NEXUS_PLATFORM==97445)
/* 7445 DBS is 7445 + 2x4538, with A8299 to control diseqc. */
#define A8299_DISEQC_PINMUX (BAST_4538_DSEC_PIN_MUX_GPIO18_GPIO19 << BAST_4538_DSEC_PIN_MUX_CHAN1_SHIFT) | \
                      (BAST_4538_DSEC_PIN_MUX_TXOUT0_TXEN0 << BAST_4538_DSEC_PIN_MUX_CHAN0_SHIFT) | \
                      (BAST_4538_DSEC_PIN_MUX_GPO6_GPO5 << BAST_4538_DSEC_PIN_MUX_CHAN2_SHIFT) | \
                      (BAST_4538_DSEC_PIN_MUX_GPO4_GPO3 << BAST_4538_DSEC_PIN_MUX_CHAN3_SHIFT) | \
                       BAST_4538_DSEC_PIN_MUX_TXOUT_GPIO18;
#else
#error "A8299_DISEQC_PINMUX needs to be defined for this platform"
#endif

#if !defined(NEXUS_FRONTEND_A8299_0_I2C_ADDR) || !defined(NEXUS_FRONTEND_A8299_1_I2C_ADDR)
#error "A8299 I2C addresses need to be defined for this platform"
#endif
/***************************************************************************
Summary:
    A8299 register read
 ***************************************************************************/
static BERR_Code NEXUS_Frontend_P_4538_A8299_ReadRegister(const NEXUS_FrontendDevice4538OpenSettings *pSettings, uint8_t i2c_addr, uint8_t reg_addr, uint8_t *pVal)
{
    BERR_Code errCode;
    uint8_t val;
    BREG_I2C_Handle i2cHandle;

    if (pSettings->diseqc.i2cDevice)
        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->diseqc.i2cDevice, NULL);
    else
        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);
    /* the A8299 doesn't support i2c repeated start condition, so we have to do a write followed by a read */
    val = reg_addr;
    errCode = BREG_I2C_WriteNoAddr(i2cHandle, i2c_addr, &val, 1);
    if (errCode)
        goto done;
    errCode = BREG_I2C_ReadNoAddr(i2cHandle, i2c_addr, pVal, 1);

done:
    return errCode;
}
/***************************************************************************
Summary:
    A8299 voltage configuration
 ***************************************************************************/
BERR_Code NEXUS_Frontend_P_4538_A8299_SetVoltage(const NEXUS_FrontendDevice4538OpenSettings *pSettings, int channel, bool bVtop)
{
    uint8_t A8299_control[2] = {0x88, 0x88};
    uint8_t i2c_addr, shift, buf[2], chipIdx, ctl;
    BREG_I2C_Handle i2cHandle;

    BDBG_ASSERT(channel < 4);

    if (pSettings->diseqc.i2cDevice)
        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->diseqc.i2cDevice, NULL);
    else
        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);

    if (channel >= 2)
    {
        i2c_addr = NEXUS_FRONTEND_A8299_1_I2C_ADDR;
        chipIdx = 1;
    }
    else
    {
        i2c_addr = NEXUS_FRONTEND_A8299_0_I2C_ADDR;
        chipIdx = 0;
    }

    if ((channel & 1) == 0)
        shift = 4;
    else
        shift = 0;

    ctl = bVtop ? 0xC : 0x8;
    A8299_control[chipIdx] &= ~((0x0F) << shift);
    A8299_control[chipIdx] |= (ctl << shift);
    buf[0] = 0;
    buf[1] = A8299_control[chipIdx];

    BDBG_MSG(("A8299_%d: channel=%d, i2c_addr=0x%X, ctl=0x%X\n", chipIdx, channel, i2c_addr, buf[1]));
    return BREG_I2C_WriteNoAddr(i2cHandle, i2c_addr, buf, 2);
}
#endif

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 4538 device
 ***************************************************************************/
static void NEXUS_Frontend_P_4538_IsrControl_isr(bool enable, void *pParam)
{
    unsigned *isrNumber = (unsigned *)pParam;

    if ( enable )
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Enable 4538 Interrupt %u", *isrNumber));
#endif
        NEXUS_Core_EnableInterrupt_isr(*isrNumber);
    }
    else
    {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Disable 4538 Interrupt %u", *isrNumber));
#endif
        NEXUS_Core_DisableInterrupt_isr(*isrNumber);
    }
}

/***************************************************************************
Summary:
    Enable/Disable gpio interrupts for a 4538 device
 ***************************************************************************/
static void NEXUS_Frontend_P_4538_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle = (NEXUS_GpioHandle)pParam;

    if(enable){ 
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Enable 4538 Gpio Interrupt %p", (void*)gpioHandle));
#endif
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, true);
    }
    else {
#if NEXUS_FRONTEND_DEBUG_IRQ
        BDBG_MSG(("Disable 4538 Gpio Interrupt %p", (void*)gpioHandle));
#endif
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, false);
    }
}

static void NEXUS_Frontend_P_4538_IsrCallback(void *pParam)
{
    NEXUS_4538Device *pDevice = (NEXUS_4538Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("4538 ISR Callback"));
#endif
    BHAB_ProcessInterruptEvent(pDevice->hab);
}

static void NEXUS_Frontend_P_4538_L1_isr(void *param1, int param2)
{
    BHAB_Handle hab = (BHAB_Handle)param2;
    BSTD_UNUSED(param1);
#if NEXUS_FRONTEND_DEBUG_IRQ
    BDBG_MSG(("4538 L1 ISR"));
#endif

    BHAB_HandleInterrupt_isr(hab);
}

#if !NEXUS_USE_7445_DBS
static uint16_t NEXUS_Frontend_P_Is4538(const NEXUS_FrontendDevice4538OpenSettings *pSettings)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint16_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x",(void*)i2cHandle,pSettings->i2cAddr));
    buf[0]= 0x0;
    subAddr = 0x1;
    BREG_I2C_WriteNoAddr(i2cHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, pSettings->i2cAddr, buf, 1);
    chipId = buf[0];
    
    subAddr = 0x2;
    BREG_I2C_WriteNoAddr(i2cHandle, pSettings->i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, pSettings->i2cAddr, buf, 1);
    chipId = (chipId <<8) | buf[0];

    BDBG_MSG(("chip ID = 0x%04x", chipId));

    return chipId;
}
#endif

static volatile bool g_a8299_inited = false;

static NEXUS_Error NEXUS_FrontendDevice_P_Init4538_PreInitAP(NEXUS_4538Device *pDevice)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    NEXUS_FrontendDevice4538OpenSettings *pSettings = &pDevice->settings;
    BAST_Settings astSettings;
    BHAB_Settings habSettings;
    BREG_I2C_Handle i2cHandle = NULL;
    BREG_SPI_Handle spiHandle = NULL;
    void *habI2cSpiHandle = NULL;
    unsigned i;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PreInitAP(%p)(%02x)(%d)",(void*)pDevice,pSettings->i2cAddr,pSettings->reset.pin));

    if (pSettings->reset.enable) {
        NEXUS_GpioSettings gpioSettings;
        if (pDevice->gpioHandle) {
            NEXUS_Gpio_Close(pDevice->gpioHandle);
        }
        BDBG_MSG(("Setting GPIO %d high",pSettings->reset.pin));
        NEXUS_Gpio_GetDefaultSettings(pSettings->reset.type, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = pSettings->reset.value;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        pDevice->gpioHandle = NEXUS_Gpio_Open(pSettings->reset.type, pSettings->reset.pin, &gpioSettings);
        BDBG_ASSERT(NULL != pDevice->gpioHandle);

        BKNI_Sleep(100);
    }

    /* Setup in the thread, to avoid blocking on BAST_Open */
    if (pSettings->i2cDevice) {
        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
        BDBG_ASSERT(NULL != i2cHandle);
    } else if (pSettings->spiDevice) {
        spiHandle = NEXUS_Spi_GetRegHandle(pSettings->spiDevice);
        BDBG_ASSERT(NULL != spiHandle);
    } else {
        BERR_TRACE(NEXUS_NOT_SUPPORTED); goto init_error;
    }

    errCode = BHAB_4538_GetDefaultSettings(&habSettings);
    {
#if NEXUS_MODE_driver
        if (Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_FRONTEND_4538, &habSettings.pImgContext, &pDevice->imgInterface )== NEXUS_SUCCESS) {
            habSettings.pImgInterface = &pDevice->imgInterface;
        } else {
            habSettings.pImgContext = NULL;
        }
#else
        habSettings.pImgInterface = &BHAB_SATFE_IMG_Interface;
        habSettings.pImgContext = &BHAB_4538_IMG_Context;
#endif
    }

    if (i2cHandle) {
        habSettings.chipAddr = pSettings->i2cAddr;
        habSettings.slaveChipAddr = pSettings->i2cSlaveAddr;
        habI2cSpiHandle = (void *)i2cHandle;
    } else {
        habSettings.isSpi = true;
        habSettings.chipAddr = pSettings->spiAddr;
        habI2cSpiHandle = (void *)spiHandle;
    }
    if(pSettings->isrNumber) {
        habSettings.interruptEnableFunc = NEXUS_Frontend_P_4538_IsrControl_isr;
        habSettings.interruptEnableFuncParam = (void*)&pDevice->settings.isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        habSettings.interruptEnableFunc = NEXUS_Frontend_P_4538_GpioIsrControl_isr;
        habSettings.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
    }

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PreInitAP: open hab(%p)",(void*)habI2cSpiHandle));
    errCode = BHAB_Open( &pDevice->hab, habI2cSpiHandle, &habSettings);
    if ( errCode != BERR_SUCCESS ) { BDBG_ERR(("Frontend unable to initialize HAB")); goto init_error; }

    BAST_4538_GetDefaultSettings(&astSettings);
    if (i2cHandle) {
        astSettings.i2c.chipAddr = pSettings->i2cAddr;
    } else {
        astSettings.i2c.chipAddr = pSettings->spiAddr;
    }
    if(pSettings->isrNumber) {
        astSettings.i2c.interruptEnableFunc = NEXUS_Frontend_P_4538_IsrControl_isr;
        astSettings.i2c.interruptEnableFuncParam = (void*)&pDevice->settings.isrNumber;
    }
    else if(pSettings->gpioInterrupt){
        astSettings.i2c.interruptEnableFunc = NEXUS_Frontend_P_4538_GpioIsrControl_isr;
        astSettings.i2c.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
    }

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PreInitAP: open ast"));
    errCode = BAST_Open(&pDevice->astHandle, NULL /*chip*/, pDevice->hab, NULL /*BINT*/, &astSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto init_error;
    }

    /* Determine number of channels -- they will be opened later */
    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PreInitAP: get total channels"));
    BAST_GetTotalChannels(pDevice->astHandle, &pDevice->numChannels);
    if ( pDevice->numChannels > NEXUS_4538_MAX_FRONTEND_CHANNELS )
    {
        BDBG_WRN(("This 4538 device supports more than the expected number of channels. Unused channels will be powered down"));
    }

    /* Open all channels prior to InitAp */
    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PreInitAP: open channels"));
    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        BAST_ChannelSettings bastChannelSettings;
        BAST_GetChannelDefaultSettings(pDevice->astHandle, i, &bastChannelSettings);
        errCode = BAST_OpenChannel(pDevice->astHandle, &pDevice->astChannels[i], i, &bastChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto init_error;
        }
    }

    if(pSettings->isrNumber) {
        errCode = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                             NEXUS_Frontend_P_4538_L1_isr,
                                             (void *)pDevice,
                                             (int)pDevice->hab);
        if ( errCode != BERR_SUCCESS )
        {
            errCode = BERR_TRACE(errCode);
            goto init_error;
        }
    }
    else if(pSettings->gpioInterrupt){
        NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt,
                                             NEXUS_Frontend_P_4538_L1_isr,
                                             (void *)pDevice,
                                             (int)pDevice->hab);
    }

    return errCode;

init_error:
    BDBG_ERR(("4538(%p) initialization failed...",(void*)pDevice));
    BKNI_EnterCriticalSection();
    pDevice->pGenericDeviceHandle->openPending = false;
    pDevice->pGenericDeviceHandle->openFailed = true;
    BKNI_LeaveCriticalSection();
    return NEXUS_NOT_AVAILABLE;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init4538_InitAP(NEXUS_4538Device *pDevice)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    const uint8_t *fw = NULL;
    /* Initialize acquisition processor */
    BDBG_WRN(("Initializing 4538 Frontend core..."));
    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_InitAP: calling BAST_InitAp"));
#if !NEXUS_MODE_driver
    fw = bcm4538_ap_image;
#endif
    errCode = BAST_InitAp(pDevice->astHandle, fw);
    if ( errCode ) {
        BDBG_ERR(("4538 InitAP failed..."));
        errCode = BERR_TRACE(errCode);
    }
    BDBG_WRN(("Initializing 4538 core... Done"));

    return errCode;

}

static NEXUS_Error NEXUS_FrontendDevice_P_Init4538_PostInitAP(NEXUS_4538Device *pDevice)
{
    int i;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP(%p)",(void*)pDevice));
    {
        /* Cache adc capabilities */
        uint8_t dummy;
        BERR_Code e;
        e = BAST_GetAdcSelect(pDevice->astChannels[0], &dummy, &pDevice->numAdc);
        if (e) {
            errCode = BERR_TRACE(e);
            return errCode;
        }
    }
    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: %d adc",pDevice->numAdc));

    {
        BWFE_Settings wfeSettings;
        BWFE_4538_GetDefaultSettings(&wfeSettings);
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: open wfe"));
        errCode = BWFE_Open(&pDevice->wfeHandle, NULL /*chip*/, pDevice->hab, NULL /*BINT*/, &wfeSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            return errCode;
        }
    }
    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: %p wfe",(void*)pDevice->wfeHandle));

    /* Store WFE information */
    errCode = BWFE_GetTotalChannels(pDevice->wfeHandle, &pDevice->wfeInfo);
    BDBG_ASSERT(!errCode);
    pDevice->numAdc = pDevice->wfeInfo.numChannels;

    /* Open all WFE channels -- this provides the ability to power up/down ADCs and to read ADC-specific data */
    for ( i = 0; i < pDevice->numAdc; i++ )
    {
        BWFE_ChannelSettings wfeChannelSettings;
        BWFE_GetChannelDefaultSettings(pDevice->wfeHandle, i, &wfeChannelSettings);
        errCode = BWFE_OpenChannel(pDevice->wfeHandle, &pDevice->wfeChannels[i], i, &wfeChannelSettings);
        if ( errCode ) {
            BDBG_ERR(("Unable to open WFE channel %d", i));
            errCode = BERR_TRACE(errCode);
            return errCode;
        }
#if NEXUS_POWER_MANAGEMENT
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: disable wfe %d",i));
        BWFE_DisableInput(pDevice->wfeChannels[i]);
#else
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: enable wfe %d",i));
        BWFE_EnableInput(pDevice->wfeChannels[i]);
#endif
        pDevice->enabled[i] = false;
    }

    if (b_3448_detect) {
        uint32_t chipId = 0, chipRev = 0;
        BAST_4538_ReadBcm3447Register(pDevice->astHandle, 0, &chipId);
        BDBG_MSG(("3447[0]:%08x",chipId));
        BAST_4538_ReadBcm3447Register(pDevice->astHandle, 4, &chipRev);
        BDBG_MSG(("3447[4]:%08x",chipRev));
        if (chipId == 0x00003448 || (chipId == 0x00003447 && chipRev >= 0x00000100)) {
            /* assume 3448 */
            b_use3448 = true;
        }
    }

#if !NEXUS_POWER_MANAGEMENT
    /* Recover from 4538 B0 (and later) initializing in a powered down state.
       If we don't have power management, we need to turn everything back on. */
    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        BAST_PowerUp(pDevice->astChannels[i], BAST_CORE_ALL);
    }

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: writing %s initial configuration",b_use3448?"3448":"3447"));
    if (b_use3448) {
        /* Power up both 3448 channels to their default mapping */
        uint8_t hab_buffer[4] = {0x40, 0, NEXUS_FRONTEND_3447_POWERUP_VALUES};
        BAST_WriteConfig(pDevice->astChannels[0], BAST_4538_CONFIG_BCM3447_CTL, hab_buffer, 4);
    } else {
        /* Power up all 3447 channels to their default mapping */
        uint8_t hab_buffer[4] = {0, 0, NEXUS_FRONTEND_3447_POWERUP_VALUES};
        BAST_WriteConfig(pDevice->astChannels[0], BAST_4538_CONFIG_BCM3447_CTL, hab_buffer, 4);
    }
#endif

    if (!g_a8299_inited) {
#if NEXUS_FRONTEND_HAS_A8299_DISEQC
        /* NEXUS_Frontend_P_4538_A8299_ReadRegister */
        /* This block of code sets up the DISEQC pin mux.  It is specific to the 4538SATIP board. */
        {
            uint16_t disecqPinMux;
            uint8_t buf[2];
            disecqPinMux = A8299_DISEQC_PINMUX;
            buf[0] = (uint8_t)((disecqPinMux >> 8) & 0xFF);
            buf[1] = (uint8_t)(disecqPinMux & 0xFF);
            BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: initializing diseqc"));
            errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_4538_CONFIG_DSEC_PIN_MUX, buf, BAST_4538_CONFIG_LEN_DSEC_PIN_MUX);
            if (errCode)
               BDBG_ERR(("BAST_WriteConfig(BAST_4538_CONFIG_DSEC_PIN_MUX) error 0x%X\n", errCode));

            /* This initialization is taken from SATFE_94538_Command_diseqc_reset for 4538satip */
            /* Clear any fault condition */
            NEXUS_Frontend_P_4538_A8299_ReadRegister(&pDevice->settings, NEXUS_FRONTEND_A8299_0_I2C_ADDR, 0, &buf[0]);
            NEXUS_Frontend_P_4538_A8299_ReadRegister(&pDevice->settings, NEXUS_FRONTEND_A8299_0_I2C_ADDR, 1, &buf[0]);
            NEXUS_Frontend_P_4538_A8299_ReadRegister(&pDevice->settings, NEXUS_FRONTEND_A8299_1_I2C_ADDR, 0, &buf[0]);
            NEXUS_Frontend_P_4538_A8299_ReadRegister(&pDevice->settings, NEXUS_FRONTEND_A8299_1_I2C_ADDR, 1, &buf[0]);
            for (i = 0; i < 4; i++)
            {
                errCode = NEXUS_Frontend_P_4538_A8299_SetVoltage(&pDevice->settings, i, false);
                if (errCode)
                    BDBG_ERR(("NEXUS_Frontend_P_4538_A8299_SetVoltage(%d) error 0x%X\n", i, errCode));
            }

        }
#endif
        if (pDevice->settings.diseqc.setPinmux) {
            uint8_t buf[2];
            buf[0] = pDevice->settings.diseqc.pinmux[0];
            buf[1] = pDevice->settings.diseqc.pinmux[1];
            BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: writing provided diseqc pinmux (%02x,%02x)",pDevice->settings.diseqc.pinmux[0],pDevice->settings.diseqc.pinmux[1]));
            errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_4538_CONFIG_DSEC_PIN_MUX, buf, BAST_4538_CONFIG_LEN_DSEC_PIN_MUX);
        }
        g_a8299_inited = true;
    }

    /* Initialize each diseqc channel.  For 4538, there are 8 demods but 4 diseqc channels. */
    for (i = 0; i < 4; i++)
    {
        errCode = BAST_ResetDiseqc(pDevice->astChannels[i], 0);
        if (errCode)
            BDBG_ERR(("BAST_ResetDiseqc(%d) error 0x%X\n", i, errCode));
    }

#if NEXUS_HAS_MXT
    {
        /* open MXT */
        BERR_Code rc;
        BMXT_Settings mxtSettings;
        BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: configuring MXT"));
        BMXT_4538_GetDefaultSettings(&mxtSettings);
        mxtSettings.chipRev = BMXT_ChipRev_eC0;
        for (i=0; i<BMXT_NUM_MTSIF; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = true;
            NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
            mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        }
        mxtSettings.hHab = pDevice->hab;

        /* fix for daughtercard,
         * host needs to be opposite of 4538 daughtercard setting (both default to 1)
         * in order to decode */
        mxtSettings.MtsifTxCfg[0].TxClockPolarity = 0;

        rc = BMXT_Open(&pDevice->pGenericDeviceHandle->mtsifConfig.mxt, NULL, NULL, &mxtSettings);
        if (rc!=BERR_SUCCESS) errCode = rc;
        rc = NEXUS_Frontend_P_InitMtsifConfig(&pDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
        if (rc!=BERR_SUCCESS) errCode = rc;
    }
#endif

    if (pDevice->settings.mtsif[0].clockRate != 0) {
        uint32_t val;
        unsigned divider;
        BHAB_ReadRegister(pDevice->hab,0x90108,&val);
        switch (pDevice->settings.mtsif[0].clockRate) {
        case 81000000: /* 81 MHz. */
            divider = 26; break;
        case 85000000: /* 85 MHz. */
        case 86000000: /* 86 MHz. */
        case 86400000: /* 86.4 MHz. (Actual value) */
            divider = 25; break;
        case 90000000: /* 90 MHz. */
            divider = 24; break;
        case 94000000: /* 94 MHz. */
        case 93900000: /* 93.9 MHz. (Actual value) */
        case 95000000: /* 95.5 MHz. */
        case 95500000:
            divider = 23; break;
        case 98000000: /* 98.2 MHz. */
        case 98100000:
        case 98200000:
            divider = 22; break;
        case 102000000: /* 102 MHz. */
        case 102500000: /* 102.5 MHz. */
        case 102850000: /* 102.85 MHz. (Actual value) */
            /* The following all also map to 102 MHz, since it is the closest value. */
        case 101000000: /* 101.25 MHz. */
        case 101250000:
        case 101500000:
        case 104000000: /* 104.5 MHz. Also accept 104 Mhz. */
        case 104500000:
            divider = 21; break;
        case 108000000: /* 108 MHz. */
            divider = 20; break;
        case 135000000: /* 135 MHz. Too fast for 40nm backends. */
            divider = 16; break;
        default:
            BDBG_WRN(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: Unrecognized rate: %u, defaulting to 81MHz.",pDevice->settings.mtsif[0].clockRate));
            divider = 26;
            break;
        }
        val &= 0xFFFFFF00;
        val |= divider;
        BHAB_WriteRegister(pDevice->hab,0x90108,&val);
    }

    if (pDevice->settings.mtsif[0].driveStrength != 0) {
        BDBG_WRN(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: MTSIF drive strength cannot be changed on this frontend."));
    }

    pDevice->pGenericDeviceHandle->openPending = false;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538_PostInitAP: returning %d",errCode));
    return errCode;
}

NEXUS_Error NEXUS_FrontendDevice_P_Init4538Events(NEXUS_4538Device *pDevice)
{
    NEXUS_Error errCode;
    BKNI_EventHandle event;
    unsigned i;

    BDBG_MSG(("NEXUS_FrontendDevice_P_Init4538Events(%p)",(void*)pDevice));
    /* Open all WFE channels -- this provides the ability to power up/down ADCs and to read ADC-specific data */
    for ( i = 0; i < pDevice->numAdc; i++ )
    {
        errCode = BAST_GetDiseqcEventHandle(pDevice->astChannels[i], &pDevice->diseqcEvents[i]);
        if ( errCode )
        {
            BDBG_ERR(("Unable to retrieve diseqc event handle for channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err_init;
        }
        pDevice->diseqcParams[i].diseqcChannel = i;
        pDevice->diseqcParams[i].device = pDevice;
        pDevice->diseqcCallbackEvents[i] = NEXUS_RegisterEvent(pDevice->diseqcEvents[i], NEXUS_Frontend_P_4538_DiseqcEventHandler, &pDevice->diseqcParams[i]);
        if ( NULL == pDevice->diseqcCallbackEvents[i] )
        {
            goto err_init;
        }
        errCode = BWFE_GetWfeReadyEventHandle(pDevice->wfeChannels[i], &pDevice->calibrationEvents[i]);
        if ( errCode )
        {
            BDBG_ERR(("Unable to retrieve calibration event handle for channel %d", i));
            errCode = BERR_TRACE(errCode);
            goto err_init;
        }
        pDevice->calibrationCallbackEvents[i] = NEXUS_RegisterEvent(pDevice->calibrationEvents[i], NEXUS_Frontend_P_4538_CalibrationEventHandler, (void *)i);
        if ( NULL == pDevice->calibrationCallbackEvents[i] )
        {
            goto err_init;
        }
    }

    errCode = BWFE_GetSaDoneEventHandle(pDevice->wfeHandle, &event);
    if ( errCode ) {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }

    /* create event here, because of module lock requirements */
    errCode = BKNI_CreateEvent(&pDevice->spectrumDataReadyEvent);
    if ( errCode ) {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }
    pDevice->spectrumEventCallback = NEXUS_RegisterEvent(event, NEXUS_Frontend_P_4538_SpectrumEventCallback, pDevice);
    if ( !pDevice->spectrumEventCallback ) {
        errCode = BERR_TRACE(NEXUS_UNKNOWN);
        goto err_init;
    }
    pDevice->spectrumDataReadyEventCallback = NEXUS_RegisterEvent(pDevice->spectrumDataReadyEvent, NEXUS_Frontend_P_4538_SpectrumDataReadyCallback, pDevice);
    if ( !pDevice->spectrumEventCallback ) {
        errCode = BERR_TRACE(NEXUS_UNKNOWN);
        goto err_init;
    }

    /* Successfully opened the 4538.  Connect interrupt */
    BHAB_GetInterruptEventHandle(pDevice->hab, &pDevice->isrEvent);
    pDevice->isrCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_4538_IsrCallback, pDevice);
    if ( NULL == pDevice->isrCallback )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_init;
    }

    errCode = BAST_GetFtmEventHandle(pDevice->astHandle, &event);
    if ( errCode ) {
        errCode = BERR_TRACE(errCode);
        goto err_init;
    }

    pDevice->ftmEventCallback = NEXUS_RegisterEvent(event, NEXUS_Frontend_P_4588_FtmEventCallback, pDevice);
    if ( !pDevice->ftmEventCallback ) {
        errCode = BERR_TRACE(NEXUS_UNKNOWN);
        goto err_init;
    }

    return NEXUS_SUCCESS;

err_init:
    return BERR_TRACE(NEXUS_NOT_INITIALIZED);
}

NEXUS_Error NEXUS_Frontend_P_Init4538Handles(NEXUS_4538Device *pDevice)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    unsigned i;

    BDBG_MSG(("NEXUS_Frontend_P_Init4538Handles(%p:%d)",(void*)pDevice,pDevice->numChannels));
    for (i=0; i < pDevice->numChannels; i++) {
        BDBG_MSG(("NEXUS_Frontend_P_Init4538Handles(%p)",(void*)pDevice->handles[i]));
    }
    for (i=0; i < pDevice->numChannels; i++) {
        NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)pDevice->handles[i]->pDeviceHandle;
        NEXUS_FrontendAstSettings *pSettings = (NEXUS_FrontendAstSettings *)&pAstDevice->settings;
        /* These were NULL earlier, now we have values we can set. */
        pSettings->astHandle = pAstDevice->astHandle = pDevice->astHandle;
        pSettings->astChannel = pAstDevice->astChannel = pDevice->astChannels[i];
        BDBG_MSG(("NEXUS_Frontend_P_Ast_DelayedInitialization(%p)",(void*)pDevice->handles[i]));
        errCode = NEXUS_Frontend_P_Ast_DelayedInitialization(pDevice->handles[i]);
        if (errCode) {
            BERR_TRACE(errCode);
            break;
        }
    }

    BDBG_MSG(("NEXUS_Frontend_P_Init4538Handles: leaving..."));
    return errCode;
}

static NEXUS_Error NEXUS_Frontend_4538_P_DelayedInitialization(NEXUS_FrontendDeviceHandle handle)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_FrontendDeviceHandle deviceHandle = (NEXUS_FrontendDeviceHandle)handle;
    NEXUS_4538Device *pDevice = (NEXUS_4538Device *)deviceHandle->pDevice;

    BDBG_MSG(("NEXUS_Frontend_4538_P_DelayedInitialization(%p)",(void*)pDevice));
    /* Initialize remainder of 4538 device requirements (events and callbacks which require module lock) */
    errCode = NEXUS_FrontendDevice_P_Init4538Events(pDevice);
    if (errCode) {
        BDBG_ERR(("Unable to complete configuration of 4538"));
        BERR_TRACE((NEXUS_NOT_INITIALIZED));
    }

    /* Initialize AST-based frontend handles */
    if (!errCode) {
        errCode = NEXUS_Frontend_P_Init4538Handles(pDevice);
        if (errCode) {
            BDBG_ERR(("Unable to complete configuration of 4538 channel handles"));
            BERR_TRACE((NEXUS_NOT_INITIALIZED));
        }
    }

    BDBG_MSG(("NEXUS_Frontend_4538_P_DelayedInitialization: leaving..."));
    return errCode;
}

static void NEXUS_Frontend_4538_P_OpenDeviceThread(void *arg) {
    NEXUS_4538Device *pDevice= (NEXUS_4538Device *)arg;
    NEXUS_Error errCode;

    errCode = NEXUS_FrontendDevice_P_Init4538_InitAP(pDevice);
    if (errCode) goto init_error;
    errCode = NEXUS_FrontendDevice_P_Init4538_PostInitAP(pDevice);
    if (errCode) goto init_error;

    return;

init_error:
    BDBG_ERR(("4538(%p) initialization failed...",(void*)pDevice));
    BKNI_EnterCriticalSection();
    pDevice->pGenericDeviceHandle->openPending = false;
    pDevice->pGenericDeviceHandle->openFailed = true;
    BKNI_LeaveCriticalSection();
}

static NEXUS_Error NEXUS_FrontendDevice_P_Init4538(NEXUS_4538Device *pDevice)
{
    NEXUS_Error errCode;

    errCode = NEXUS_FrontendDevice_P_Init4538_PreInitAP(pDevice);
    if (!errCode)
        errCode = NEXUS_FrontendDevice_P_Init4538_InitAP(pDevice);
    if (!errCode)
        errCode = NEXUS_FrontendDevice_P_Init4538_PostInitAP(pDevice);

    if (!errCode)
        errCode = NEXUS_Frontend_4538_P_DelayedInitialization(pDevice->pGenericDeviceHandle);

    return errCode;
}



static void NEXUS_Frontend_P_Uninit4538(NEXUS_4538Device *pDevice)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);

    BDBG_MSG(("Closing 4538 device %p handles", (void*)pDevice));

    if (pDevice->deviceOpenThread)
        NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
        pDevice->pGenericDeviceHandle->mtsifConfig.mxt = NULL;
    }
#endif

    for ( i=0; i < pDevice->numChannels && NULL != pDevice->astChannels[i]; i++) {
        BAST_CloseChannel(pDevice->astChannels[i]);
        pDevice->astChannels[i] = NULL;
    }

    for ( i = 0; i < pDevice->numAdc; i++ )
    {
        if (pDevice->calibrationCallbackEvents[i]) {
            NEXUS_UnregisterEvent(pDevice->calibrationCallbackEvents[i]);
            pDevice->calibrationCallbackEvents[i] = NULL;
        }
    }
    if (pDevice->spectrumEventCallback) {
        NEXUS_UnregisterEvent(pDevice->spectrumEventCallback);
        pDevice->spectrumEventCallback = NULL;
    }
    if (pDevice->spectrumDataReadyEventCallback) {
        NEXUS_UnregisterEvent(pDevice->spectrumDataReadyEventCallback);
        pDevice->spectrumDataReadyEventCallback = NULL;
    }
    if (pDevice->spectrumDataReadyEvent) {
        BKNI_DestroyEvent(pDevice->spectrumDataReadyEvent);
        pDevice->spectrumDataReadyEvent = NULL;
    }
    for ( i = 0; i < pDevice->numChannels; i++) {
        if (pDevice->spectrumDataReadyAppCallback[i]) {
            NEXUS_TaskCallback_Destroy(pDevice->spectrumDataReadyAppCallback[i]);
            pDevice->spectrumDataReadyAppCallback[i] = NULL;
        }
    }

    for ( i=0; i < pDevice->wfeInfo.numChannels; i++ )
    {
        if (pDevice->wfeChannels[i]) {
            BDBG_MSG(("Closing WFE[%d]",i));
            BWFE_CloseChannel(pDevice->wfeChannels[i]);
            pDevice->wfeChannels[i] = NULL;
        }
    }

    if (pDevice->wfeHandle) {
        BWFE_Close(pDevice->wfeHandle);
        pDevice->wfeHandle = NULL;
    }

    if (pDevice->ftmEventCallback) {
        NEXUS_UnregisterEvent(pDevice->ftmEventCallback);
        pDevice->ftmEventCallback = NULL;
    }

    if (pDevice->astHandle) {
        BAST_Close(pDevice->astHandle);
        pDevice->astHandle = NULL;
    }

    if (pDevice->isrCallback) {
        BDBG_MSG(("Unregister isrCallback %p",(void*)pDevice->isrCallback));
        NEXUS_UnregisterEvent(pDevice->isrCallback);
        pDevice->isrCallback = NULL;
        BDBG_MSG(("...done"));
    }

    if (pDevice->settings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber);
    } else if (pDevice->settings.gpioInterrupt) {
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->settings.gpioInterrupt, NULL, NULL, 0);
    }

    if (pDevice->hab) {
        BHAB_Close(pDevice->hab);
        pDevice->hab = NULL;
    }

    if (pDevice->settings.reset.enable) {
        if (pDevice->gpioHandle) {
            NEXUS_Gpio_Close(pDevice->gpioHandle);
            pDevice->gpioHandle = NULL;
        }
    }
}

void NEXUS_FrontendDevice_GetDefault4538OpenSettings(
    NEXUS_FrontendDevice4538OpenSettings *pSettings
    )
{
    int i;
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    for (i=0; i < NEXUS_MAX_MTSIF; i++) {
        pSettings->mtsif[i].enabled = true;
    }
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open4538(
    unsigned index,
    const NEXUS_FrontendDevice4538OpenSettings *pSettings
    )
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_Error errCode;
    NEXUS_4538Device *pDevice=NULL;
    bool newDevice = false;

    BSTD_UNUSED(index);

    BDBG_ASSERT(NULL != pSettings);

    for ( pDevice = BLST_S_FIRST(&g_deviceList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);
        if ((pSettings->i2cAddr == pDevice->settings.i2cAddr) && (pSettings->i2cDevice == pDevice->settings.i2cDevice))
        {
            break;
        }
    }

    if ( NULL == pDevice )
    {
        BDBG_MSG(("Opening new 4538 device at 0x%02x",pSettings->i2cDevice ? pSettings->i2cAddr : pSettings->spiAddr));
        newDevice = true;
#if !NEXUS_USE_7445_DBS
        {
            uint16_t chipId=0;
            /* check is disabled for dual chip configurations due to asynchronous initialization */
            chipId = NEXUS_Frontend_P_Is4538(pSettings);
            if (chipId != 0x4538) {
                BDBG_WRN(("Warning: 4538 not found at i2c address 0x%02x",pSettings->i2cAddr));
            }
        }
#endif
        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); return NULL;}

        /* Memsetting the whole structure should cover initializing the child list. */
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));
        pDevice = BKNI_Malloc(sizeof(NEXUS_4538Device));
        if ( NULL == pDevice )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_4538Device);
        pDevice->settings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;
        BDBG_MSG(("NEXUS_FrontendDevice_Open4538(%p)(%02x)",(void*)pDevice,pSettings->i2cAddr));

        pDevice->numChannels = NEXUS_4538_MAX_FRONTEND_CHANNELS; /* Temporary value.  The real value is read in the initialization thread. */

        errCode = NEXUS_FrontendDevice_P_Init4538_PreInitAP(pDevice);
        if (errCode) goto err;

        pFrontendDevice->delayedInit = NEXUS_Frontend_4538_P_DelayedInitialization;
        pFrontendDevice->delayedInitializationRequired = true;

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

#if DISABLE_4538_ASYNC_INIT
        NEXUS_Frontend_4538_P_OpenDeviceThread(pDevice);
#else
        {
            NEXUS_ThreadSettings thread_settings;
            NEXUS_Thread_GetDefaultSettings(&thread_settings);
            thread_settings.priority = 0;
            pDevice->deviceOpenThread = NEXUS_Thread_Create("deviceOpenThread",
                                                            NEXUS_Frontend_4538_P_OpenDeviceThread,
                                                            (void*)pDevice,
                                                            &thread_settings);
        }
#endif

        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
    }
    else
    {
        BDBG_MSG(("Found 4538 device"));
        return pDevice->pGenericDeviceHandle;
        BDBG_MSG(("Found existing device"));
    }


    BDBG_MSG(("NEXUS_FrontendDevice_Open4538: configuring frontend device"));

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = 0x4538;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eSatellite;
    pFrontendDevice->getStatus = NEXUS_FrontendDevice_P_4538_GetStatus;
    pFrontendDevice->getCapabilities = NEXUS_FrontendDevice_P_4538_GetCapabilities;
    pFrontendDevice->close = NEXUS_Frontend_P_4538_DestroyDevice;
    pFrontendDevice->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Get4538Capabilities;

    pFrontendDevice->mode = NEXUS_StandbyMode_eOn;
    pFrontendDevice->standby = NEXUS_FrontendDevice_P_4538_Standby;

    pFrontendDevice->nonblocking.getCapabilities = true; /* does not require init complete to fetch number of demods */
    pFrontendDevice->nonblocking.getSatelliteCapabilities = true; /* does not require init complete to fetch number of demods */

    BDBG_MSG(("NEXUS_FrontendDevice_Open4538: returning %p",(void*)pFrontendDevice));
    return pFrontendDevice;

err:
    if (pDevice)
        NEXUS_Frontend_P_4538_DestroyDevice(pDevice);
    return NULL;
}

void NEXUS_Frontend_GetDefault4538Settings( NEXUS_4538Settings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_Open4538( const NEXUS_4538Settings *pSettings )
{
    NEXUS_Error errCode;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_4538Device *pDevice = NULL;
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    NEXUS_FrontendDevice4538OpenSettings openSettings;
    NEXUS_FrontendAstSettings astChannelSettings;
    BREG_I2C_Handle i2cHandle = NULL;
    if(pSettings->device == NULL) {
        BDBG_ASSERT(NULL != pSettings->i2cDevice);
        NEXUS_FrontendDevice_GetDefault4538OpenSettings(&openSettings);
        openSettings.gpioInterrupt = pSettings->gpioInterrupt;
        openSettings.i2cAddr = pSettings->i2cAddr;
        openSettings.i2cDevice  = pSettings->i2cDevice;
        openSettings.i2cSlaveAddr  = pSettings->i2cSlaveAddr;
        openSettings.isrNumber  = pSettings->isrNumber;
        pFrontendDevice = NEXUS_FrontendDevice_Open4538(0, &openSettings);
        pDevice = (NEXUS_4538Device *)pFrontendDevice->pDevice;
    }
    else {
        pFrontendDevice = pSettings->device;
        pDevice = (NEXUS_4538Device *)pSettings->device->pDevice;
    }
    /* Return already opened frontend handle. */
    if (pSettings->channelNumber >= pDevice->numChannels) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (pDevice->handles[pSettings->channelNumber])
        return pDevice->handles[pSettings->channelNumber];

    if (pDevice->settings.diseqc.i2cDevice) {
        i2cHandle = NEXUS_I2c_GetRegHandle(pDevice->settings.diseqc.i2cDevice, NEXUS_MODULE_SELF);
    } else {
        i2cHandle = NEXUS_I2c_GetRegHandle(pDevice->settings.i2cDevice, NEXUS_MODULE_SELF);
    }

    /* otherwise open new frontend */
    BDBG_MSG(("Creating channel %u", pSettings->channelNumber));

    /* Open channel */
    NEXUS_Frontend_P_Ast_GetDefaultSettings(&astChannelSettings);
    astChannelSettings.astHandle = pDevice->astHandle;
    astChannelSettings.astChannel = pDevice->astChannels[pSettings->channelNumber];
#define B_AST_CHIP 4538
    astChannelSettings.astChip = B_AST_CHIP;
    astChannelSettings.closeFunction = NEXUS_Frontend_P_4538_CloseCallback;
    astChannelSettings.pCloseParam = pDevice;
    astChannelSettings.pDevice = pDevice;
    astChannelSettings.channelIndex = pSettings->channelNumber;
    astChannelSettings.diseqcIndex = 0;
    astChannelSettings.i2cRegHandle = i2cHandle; /* due to module locking, we need to save our i2c register handle for Diseqc control */
    astChannelSettings.getDiseqcChannelHandle = NEXUS_Frontend_P_4538_GetDiseqcChannelHandle;
    astChannelSettings.getDiseqcChannelHandleParam = pDevice;
    astChannelSettings.getDiseqcEventHandle = NEXUS_Frontend_P_4538_GetDiseqcEventHandle;
    astChannelSettings.getDiseqcAppCallback = NEXUS_Frontend_P_4538_GetDiseqcAppCallback;
    astChannelSettings.setDiseqcAppCallback = NEXUS_Frontend_P_4538_SetDiseqcAppCallback;
    astChannelSettings.capabilities.diseqc = true;
    astChannelSettings.delayedInitialization = true;

    frontend = NEXUS_Frontend_P_Ast_Create(&astChannelSettings);
    if ( !frontend )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_4538_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        return NULL;
    }
    frontend->tuneSatellite = NEXUS_Frontend_P_4538_TuneSatellite;
    frontend->untune = NEXUS_Frontend_P_4538_Untune;
    frontend->reapplyTransportSettings = NEXUS_Frontend_P_4538_ReapplyTransportSettings;
    frontend->requestSpectrumData = NEXUS_Frontend_P_4538_RequestSpectrumData;
    frontend->getSatelliteAgcStatus = NEXUS_Frontend_P_4538_GetSatelliteAgcStatus;
    frontend->pGenericDeviceHandle = pFrontendDevice;
    frontend->getSatelliteRuntimeSettings = NEXUS_Frontend_P_Get4538RuntimeSettings;
    frontend->setSatelliteRuntimeSettings = NEXUS_Frontend_P_Set4538RuntimeSettings;

    frontend->standby = NEXUS_Frontend_P_4538_Standby;

    pDevice->handles[pSettings->channelNumber] = frontend;

    /* preconfigure mtsif settings so platform doesn't need to */
    frontend->userParameters.isMtsif = true;
    frontend->mtsif.inputBand = pSettings->channelNumber; /* IB to demod mapping is direct. */

    pDevice->spectrumDataReadyAppCallback[pSettings->channelNumber] = NEXUS_TaskCallback_Create(frontend, NULL);

    BDBG_MSG(("NEXUS_Frontend_Open4538: returning %p", (void*)frontend));

    return frontend;
}


NEXUS_Error NEXUS_Frontend_P_Ast_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
void NEXUS_Frontend_P_Ast_Untune(void *handle);

static NEXUS_Error NEXUS_Frontend_P_4538_TuneSatellite(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings)
{
    NEXUS_AstDevice *pAstDevice = handle;
    NEXUS_4538Device *p4538Device = pAstDevice->settings.pDevice;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(p4538Device, NEXUS_4538Device);
    
    rc = NEXUS_Frontend_P_SetMtsifConfig(pAstDevice->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }

    p4538Device->useChannels[pAstDevice->channel] = true;
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Frontend_P_SetAdcPower(pAstDevice->frontendHandle);
#endif

    return NEXUS_Frontend_P_Ast_TuneSatellite(pAstDevice, pSettings);
}

static void NEXUS_Frontend_P_4538_Untune(void *handle)
{
    NEXUS_AstDevice *pAstDevice = handle;
    NEXUS_4538Device *p4538Device = pAstDevice->settings.pCloseParam;

    BDBG_OBJECT_ASSERT(p4538Device, NEXUS_4538Device);

    NEXUS_Frontend_P_UnsetMtsifConfig(pAstDevice->frontendHandle);

    NEXUS_Frontend_P_Ast_Untune(pAstDevice);

    p4538Device->useChannels[pAstDevice->channel] = false;
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Frontend_P_SetAdcPower(pAstDevice->frontendHandle);
#endif

    return;
}

NEXUS_Error NEXUS_Frontend_P_4538_ReapplyTransportSettings(void *handle)
{
    NEXUS_AstDevice *pAstDevice = handle;
    NEXUS_Error rc;

    rc = NEXUS_Frontend_P_SetMtsifConfig(pAstDevice->frontendHandle);
    if (rc) { return BERR_TRACE(rc); }
    
    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_4588_FtmEventCallback(void *context)
{
    NEXUS_4538Device *pDevice = context;
    unsigned i;
    for (i=0;i<NEXUS_4538_MAX_FRONTEND_CHANNELS;i++) {
        if (pDevice->handles[i]) {
            NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(pDevice->handles[i]);
            if (astDevice) {
                NEXUS_TaskCallback_Fire(astDevice->ftmCallback);
            }
        }
    }
}

static void NEXUS_Frontend_P_4538_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_4538Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);

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

static void NEXUS_Frontend_P_4538_DestroyDevice(void *handle)
{
    NEXUS_4538Device *pDevice = (NEXUS_4538Device *)handle;
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);

#if NEXUS_HAS_MXT
    if (pDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(pDevice->pGenericDeviceHandle->mtsifConfig.mxt);
    }
#endif

    g_a8299_inited = false;

    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        if ( NULL != pDevice->handles[i] )
        {
            BDBG_ERR(("All channels must be closed before destroying device"));
            BDBG_ASSERT(NULL == pDevice->handles[i]);
        }
    }

    for ( i = 0; i < pDevice->numAdc; i++ )
    {
        if (pDevice->diseqcCallbackEvents[i]) {
            NEXUS_UnregisterEvent(pDevice->diseqcCallbackEvents[i]);
        }
    }
    for ( i = 0; i < pDevice->numChannels && NULL != pDevice->astChannels[i]; i++) {
        BAST_CloseChannel(pDevice->astChannels[i]);
    }
    for ( i = 0; i < pDevice->numAdc; i++ )
    {
        if (pDevice->calibrationCallbackEvents[i])
            NEXUS_UnregisterEvent(pDevice->calibrationCallbackEvents[i]);
        if (pDevice->wfeChannels[i])
            BWFE_CloseChannel(pDevice->wfeChannels[i]);
    }
    if (pDevice->spectrumEventCallback) {
        NEXUS_UnregisterEvent(pDevice->spectrumEventCallback);
    }
    if (pDevice->spectrumDataReadyEventCallback) {
        NEXUS_UnregisterEvent(pDevice->spectrumDataReadyEventCallback);
    }
    if (pDevice->spectrumDataReadyEvent) {
        BKNI_DestroyEvent(pDevice->spectrumDataReadyEvent);
        pDevice->spectrumDataReadyEvent = NULL;
    }
    for ( i = 0; i < pDevice->numChannels; i++) {
        if (pDevice->spectrumDataReadyAppCallback[i]) {
            NEXUS_TaskCallback_Destroy(pDevice->spectrumDataReadyAppCallback[i]);
        }
    }
    BWFE_Close(pDevice->wfeHandle);

    if (pDevice->ftmEventCallback) {
        NEXUS_UnregisterEvent(pDevice->ftmEventCallback);
    }
    BAST_Close(pDevice->astHandle);

    BDBG_MSG(("Destroying 4538 device %p", (void*)pDevice));

    if (pDevice->isrCallback) {
        NEXUS_UnregisterEvent(pDevice->isrCallback);
    }

    if (pDevice->settings.isrNumber) {
        NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber);
    } else if (pDevice->settings.gpioInterrupt) {
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->settings.gpioInterrupt, NULL, NULL, 0);
    }

    if (pDevice->hab) {
        BHAB_Close(pDevice->hab);
    }

    if (pDevice->deviceOpenThread)
        NEXUS_Thread_Destroy(pDevice->deviceOpenThread);
    pDevice->deviceOpenThread = NULL;

    if (pDevice->settings.reset.enable) {
        if (pDevice->gpioHandle) {
            NEXUS_Gpio_Close(pDevice->gpioHandle);
            pDevice->gpioHandle = NULL;
        }
    }

    BKNI_Free(pDevice->pGenericDeviceHandle);
    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_4538Device, node);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_4538Device);
    BKNI_Free(pDevice);
}

void NEXUS_FrontendDevice_P_4538_S3Standby(NEXUS_4538Device *pDevice)
{
    NEXUS_Frontend_P_Uninit4538(pDevice);
}

static NEXUS_Error NEXUS_FrontendDevice_P_4538_Standby(void *handle, const NEXUS_StandbySettings *pSettings)
{

    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_4538Device *pDevice;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_4538Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);

    BDBG_MSG(("NEXUS_FrontendDevice_P_4538_Standby: %p",(void*)handle));
    if ((pDevice->pGenericDeviceHandle->mode != NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode == NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_4538_Standby: Entering deep sleep..."));

        NEXUS_FrontendDevice_P_4538_S3Standby(pDevice);

        BKNI_EnterCriticalSection();
        pDevice->pGenericDeviceHandle->openPending = true;
        pDevice->pGenericDeviceHandle->openFailed = false;
        BKNI_LeaveCriticalSection();

    } else if ((pDevice->pGenericDeviceHandle->mode == NEXUS_StandbyMode_eDeepSleep) && (pSettings->mode != NEXUS_StandbyMode_eDeepSleep)) {

        BDBG_MSG(("NEXUS_FrontendDevice_P_4538_Standby: Waking up..."));
        BDBG_MSG(("NEXUS_FrontendDevice_P_4538_Standby: reinitializing..."));
        rc = NEXUS_FrontendDevice_P_Init4538(pDevice);
        if (rc) { rc = BERR_TRACE(rc); goto done;}
    }

done:
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_4538_RegisterEvents(NEXUS_AstDevice *pAstChannel)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    NEXUS_Frontend_P_Ast_RegisterEvents(pAstChannel);
    return errCode;
}

static NEXUS_Error NEXUS_Frontend_P_4538_UnregisterEvents(NEXUS_AstDevice *pAstChannel)
{
    NEXUS_4538Device *p4538Device = pAstChannel->settings.pDevice;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    int i;

    NEXUS_Frontend_P_Ast_UnregisterEvents(pAstChannel);
    for ( i = 0; i < p4538Device->numAdc; i++ )
    {
        if (p4538Device->diseqcCallbackEvents[i]) {
            NEXUS_UnregisterEvent(p4538Device->diseqcCallbackEvents[i]);
            p4538Device->diseqcCallbackEvents[i] = NULL;
        }
    }
    return errCode;
}

static NEXUS_Error NEXUS_Frontend_P_4538_Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_AstDevice *pAstChannel = (NEXUS_AstDevice *)handle;
    NEXUS_4538Device *p4538Device;

    BDBG_OBJECT_ASSERT(pAstChannel, NEXUS_AstDevice);
    p4538Device = pAstChannel->settings.pDevice;

    BDBG_MSG(("NEXUS_Frontend_P_4538_Standby: standby %p(%d) %s", (void*)handle, pAstChannel->channel, enabled ? "enabled" : "disabled"));

    BDBG_MSG(("Restoring handles on %p",(void*)pAstChannel));
    /* update/restore handles */
    pAstChannel->astChannel = p4538Device->astChannels[pAstChannel->channel];

    if (pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Unregistering events on %p",(void*)pAstChannel));
        NEXUS_Frontend_P_4538_UnregisterEvents(pAstChannel);
    } else if (pSettings->mode != NEXUS_StandbyMode_eDeepSleep && pAstChannel->frontendHandle->mode == NEXUS_StandbyMode_eDeepSleep) {
        BDBG_MSG(("Registering events on %p",(void*)pAstChannel));
        NEXUS_Frontend_P_4538_RegisterEvents(pAstChannel);
    }

    BDBG_MSG(("Done with standby configuration on %p",(void*)pAstChannel));
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_FrontendDevice_P_Get4538Capabilities(void *handle, NEXUS_Frontend4538Capabilities *pCapabilities)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_4538Device *p4538Device = (NEXUS_4538Device *)handle;

    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(p4538Device, NEXUS_4538Device);

    BKNI_Memset(pCapabilities,0,sizeof(*pCapabilities));
    pCapabilities->numAdc = p4538Device->numAdc;
    pCapabilities->numChannels = p4538Device->numChannels;
    pCapabilities->externalBert = false;

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Get4538RuntimeSettings(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    BERR_Code err;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)handle;
    NEXUS_4538Device *p4538Device;
    uint8_t adc, dummy;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    p4538Device = pAstDevice->settings.pDevice;
    BDBG_ASSERT(p4538Device);

    err = BAST_GetAdcSelect(pAstDevice->astChannel, &adc, &dummy);
    if (err) {
        rc = NEXUS_INVALID_PARAMETER;
    } else {
        pSettings->selectedAdc = adc;
        pAstDevice->diseqcIndex = adc;
        pSettings->externalBert.enabled = false;
        pSettings->externalBert.invertClock = false;
    }

    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_Set4538RuntimeSettings(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    BERR_Code err;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)handle;
    NEXUS_4538Device *p4538Device;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    p4538Device = pAstDevice->settings.pDevice;
    BDBG_ASSERT(p4538Device);

    BDBG_MSG(("adc: %d, mask: 0x%08x",pSettings->selectedAdc,p4538Device->wfeInfo.availChannelsMask));
    /* Ensure the requested adc is within the value range, and advertised by the PI as being available */
    if (pSettings->selectedAdc > p4538Device->numAdc || !((1<<pSettings->selectedAdc) & p4538Device->wfeInfo.availChannelsMask) )
        return NEXUS_INVALID_PARAMETER;

    err = BAST_SetAdcSelect(pAstDevice->astChannel, pSettings->selectedAdc);
    if (err) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    } else {
        pAstDevice->diseqcIndex = pSettings->selectedAdc;
        p4538Device->wfeMap[pAstDevice->channel] = pSettings->selectedAdc;
    }

    return rc;
}

#if NEXUS_POWER_MANAGEMENT
static void NEXUS_Frontend_P_SetAdcPower(NEXUS_FrontendHandle handle)
{
    NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)handle->pDeviceHandle;
    NEXUS_4538Device *p4538Device = pAstDevice->settings.pDevice;
    unsigned ix;
    bool inUse[NEXUS_4538_MAX_WFE_CHANNELS] = {false};
    bool waitForEvent[NEXUS_4538_MAX_WFE_CHANNELS] = {false};
    bool reprogram_lna = false;

    uint8_t powered_up[4] = {0x00, 0x00, NEXUS_FRONTEND_3447_POWERUP_VALUES};
    uint8_t current_lna[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t new_lna[4]     = {0x00, 0x00, 0x44, 0x44};

    BDBG_ASSERT(handle);

    /* Scan for in-use (tuned) channels */
    for (ix=0; ix < p4538Device->numChannels; ix++) {
        if (p4538Device->useChannels[ix]) {
            inUse[p4538Device->wfeMap[ix]] = true;
        }
    }

    BAST_ReadConfig(p4538Device->astChannels[0], BAST_4538_CONFIG_BCM3447_CTL, current_lna, 4);
    BDBG_MSG(("344%s:[old] %02x %02x %02x %02x", b_use3448 ? "8" : "7", current_lna[0], current_lna[1], current_lna[2], current_lna[3]));

    if (b_use3448) {
        powered_up[0] = 0x40;
        new_lna[0] = 0x40;
        new_lna[2] = 0x55;
    }

    if (inUse[0]) {
        new_lna[3] &= 0xF0; new_lna[3] |= powered_up[3] & 0x0F;
    }
    if (inUse[1]) {
        new_lna[3] &= 0x0F; new_lna[3] |= powered_up[3] & 0xF0;
    }
    if (inUse[2]) {
        new_lna[2] &= 0xF0; new_lna[2] |= powered_up[2] & 0x0F;
    }
    if (inUse[3]) {
        new_lna[2] &= 0x0F; new_lna[2] |= powered_up[2] & 0xF0;
    }

    /* Power up the in-use ADCs, power down the rest */
    for (ix=0; ix < p4538Device->numAdc; ix++) {
        if (inUse[ix]) {
            if (!p4538Device->enabled[ix]) {
                BWFE_EnableInput(p4538Device->wfeChannels[ix]);
                p4538Device->enabled[ix] = true;
                waitForEvent[ix] = true;
            }
        } else {
            if (p4538Device->enabled[ix]) {
                BWFE_DisableInput(p4538Device->wfeChannels[ix]);
                p4538Device->enabled[ix] = false;
            }
        }
    }
    for (ix=0; ix < p4538Device->numAdc; ix++) {
        if (waitForEvent[ix]) {
            BKNI_WaitForEvent(p4538Device->calibrationEvents[ix], 5000);
        }
    }
    /* don't rewrite LNA until calibration is complete */
    for (ix=1 ; ix < 4; ix++) {
        if (new_lna[ix] != current_lna[ix])
            reprogram_lna = true;
    }
    if (reprogram_lna) {
        BAST_WriteConfig(p4538Device->astChannels[0], BAST_4538_CONFIG_BCM3447_CTL, new_lna, 4);
        BDBG_MSG(("344%s:[new] %02x %02x %02x %02x", b_use3448 ? "8" : "7", new_lna[0], new_lna[1], new_lna[2], new_lna[3]));
    }


}
#endif

static void NEXUS_Frontend_P_4538_SpectrumDataReadyCallback(void *pParam)
{
    NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)pParam;
    NEXUS_4538Device *p4538Device = NULL;
    BDBG_ASSERT(NULL != pParam);
    p4538Device = pAstDevice->settings.pDevice;
    NEXUS_TaskCallback_Fire(p4538Device->spectrumDataReadyAppCallback[pAstDevice->channel]);
}

static void NEXUS_Frontend_P_4538_SpectrumEventCallback(void *pParam)
{
    NEXUS_4538Device *p4538Device = (NEXUS_4538Device *)pParam;
    BERR_Code rc;
    BDBG_ASSERT(pParam);
    rc = BWFE_GetSaSamples(p4538Device->wfeHandle, p4538Device->spectrumDataPointer);
    BKNI_SetEvent(p4538Device->spectrumDataReadyEvent);
}

static NEXUS_Error NEXUS_Frontend_P_4538_RequestSpectrumData(void *handle, const NEXUS_FrontendSpectrumSettings *pSettings)
{
    NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)handle;
    NEXUS_4538Device *p4538Device = pAstDevice->settings.pDevice;
    BERR_Code rc;

    BWFE_SpecAnalyzerParams saParams;

    saParams.freqStartHz = pSettings->startFrequency;
    saParams.freqStopHz = pSettings->stopFrequency;
    saParams.numSamples = pSettings->numSamples;
    saParams.numSweeps = 1;

    NEXUS_TaskCallback_Set(p4538Device->spectrumDataReadyAppCallback[pAstDevice->channel], &(pSettings->dataReadyCallback));
    if (p4538Device->spectrumDataReadyEventCallback) {
        NEXUS_UnregisterEvent(p4538Device->spectrumDataReadyEventCallback);
    }
    p4538Device->spectrumDataReadyEventCallback = NEXUS_RegisterEvent(p4538Device->spectrumDataReadyEvent, NEXUS_Frontend_P_4538_SpectrumDataReadyCallback, handle);
    p4538Device->spectrumDataPointer = pSettings->data;
    p4538Device->spectrumDataLength = pSettings->dataLength;

#if NEXUS_POWER_MANAGEMENT
    BWFE_EnableInput(p4538Device->wfeChannels[p4538Device->wfeMap[pAstDevice->channel]]);
#endif
    rc = BWFE_ScanSpectrum(p4538Device->wfeChannels[p4538Device->wfeMap[pAstDevice->channel]], &saParams);
    if (rc) { BERR_TRACE(rc); return NEXUS_INVALID_PARAMETER; }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_FrontendDevice_P_4538_GetStatus(void *handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    NEXUS_4538Device *pDevice = NULL;
    BHAB_AvsData avsData;
    BDBG_ASSERT(NULL != handle);
    pDevice = (NEXUS_4538Device *)handle;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    rc = BHAB_GetAvsData(pDevice->hab, &avsData);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    pStatus->avs.enabled = avsData.enabled;
    pStatus->temperature = avsData.temperature;
    if(avsData.enabled) {
        pStatus->avs.voltage = avsData.voltage;
    }
    pStatus->openPending = pDevice->pGenericDeviceHandle->openPending;
    pStatus->openFailed = pDevice->pGenericDeviceHandle->openFailed;
done:
    return rc;
}

static BAST_ChannelHandle NEXUS_Frontend_P_4538_GetDiseqcChannelHandle(void *handle, int index)
{
    NEXUS_4538Device *p4538Device = (NEXUS_4538Device *)handle;
    BDBG_ASSERT(handle);
    return p4538Device->astChannels[index];
}

static NEXUS_EventCallbackHandle NEXUS_Frontend_P_4538_GetDiseqcEventHandle(void *handle, int index)
{
    NEXUS_4538Device *p4538Device = (NEXUS_4538Device *)handle;
    BDBG_ASSERT(handle);
    return p4538Device->diseqcCallbackEvents[index];
}

static NEXUS_TaskCallbackHandle NEXUS_Frontend_P_4538_GetDiseqcAppCallback(void *handle, int index)
{
    NEXUS_4538Device *p4538Device = (NEXUS_4538Device *)handle;
    BDBG_ASSERT(handle);
    return p4538Device->diseqcAppCallbacks[index];
}

static void NEXUS_Frontend_P_4538_SetDiseqcAppCallback(void *handle, int index, NEXUS_TaskCallbackHandle appCallback)
{
    NEXUS_4538Device *p4538Device = (NEXUS_4538Device *)handle;
    BDBG_ASSERT(handle);
    p4538Device->diseqcAppCallbacks[index] = appCallback;
}

static void NEXUS_Frontend_P_4538_DiseqcEventHandler(void *pParam)
{
    NEXUS_4538DiseqcParam *pDiseqcParam = (NEXUS_4538DiseqcParam *)pParam;
    NEXUS_4538Device *p4538Device = NULL;
    BDBG_ASSERT(pParam);
    p4538Device = (NEXUS_4538Device *)pDiseqcParam->device;
    BDBG_ASSERT(p4538Device);

    BDBG_MSG(("4538 Diseqc Event for %d", pDiseqcParam->diseqcChannel));

    if (p4538Device->diseqcAppCallbacks[pDiseqcParam->diseqcChannel]) {
        NEXUS_TaskCallback_Fire(p4538Device->diseqcAppCallbacks[pDiseqcParam->diseqcChannel]);
    } else {
        BDBG_WRN(("Received 4538 diseqc event callback on %d without a registered app callback", pDiseqcParam->diseqcChannel));
    }
}

static void NEXUS_Frontend_P_4538_CalibrationEventHandler(void *pParam)
{
    int whichWfe = (int)pParam;
    BDBG_MSG(("4538 Calibration Event: %d",whichWfe));
}

static uint32_t NEXUS_Platform_P_I2c_Get4538ChipId(NEXUS_I2cHandle i2cDevice, uint16_t i2cAddr)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint16_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x",(void*)i2cHandle,i2cAddr));
    buf[0]= 0x0;
    subAddr = 0x1;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = buf[0];

    subAddr = 0x2;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = (chipId <<8) | buf[0];

    BDBG_MSG(("chip family ID = 0x%04x", chipId));

    return chipId;
}

#define DEBUG_SPI_READS 0
static uint32_t NEXUS_Platform_P_Spi_Get4538ChipId(NEXUS_SpiHandle spiDevice, uint16_t spiAddr)
{
    uint16_t chipId=0;
    uint8_t wData[2], rData[8];
    NEXUS_Error rc;

    BDBG_MSG(("Probing for 4538 at SPI 0x%02x",spiAddr));

    wData[0] = spiAddr << 1;
    wData[1] = 0x00;
#if DEBUG_SPI_READS
    {
        int i;
        for (i=0; i < 2; i++) {
            BDBG_MSG(("wData[%d]: 0x%02x",i,wData[i]));
        }
    }
#endif

    rc = NEXUS_Spi_Read(spiDevice, wData, rData, 8);
    if(rc){rc = BERR_TRACE(rc); goto done;}

#if DEBUG_SPI_READS
    {
        int i;
        for (i=0; i < 8; i++) {
            BDBG_MSG(("rData[%d]: 0x%02x",i,rData[i]));
        }
    }
#endif

    chipId = (rData[3] <<8) | rData[4];

    BDBG_MSG(("chip family ID = 0x%04x", chipId));

done:
    return chipId;
}

NEXUS_Error NEXUS_Frontend_P_Get4538ChipInfo(const NEXUS_FrontendDevice4538OpenSettings *pSettings, NEXUS_4538ProbeResults *pResults) {
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pSettings);
    pResults->chip.id = 0x4538;

    return rc;
}

NEXUS_Error NEXUS_Frontend_Probe4538(const NEXUS_FrontendDevice4538OpenSettings *pSettings, NEXUS_4538ProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_GpioHandle gpioHandle = NULL;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BSTD_UNUSED(gpioHandle);

    BKNI_Memset(pResults, 0, sizeof(*pResults));

    if (pSettings->reset.enable) {
        NEXUS_GpioSettings gpioSettings;
        BDBG_MSG(("Setting GPIO %d high",pSettings->reset.pin));
        NEXUS_Gpio_GetDefaultSettings(pSettings->reset.type, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = pSettings->reset.value;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        gpioHandle = NEXUS_Gpio_Open(pSettings->reset.type, pSettings->reset.pin, &gpioSettings);
        BDBG_ASSERT(NULL != gpioHandle);
        BKNI_Sleep(10);
    }

    if (pSettings->i2cDevice) {
        pResults->chip.familyId = (uint32_t)NEXUS_Platform_P_I2c_Get4538ChipId(pSettings->i2cDevice, pSettings->i2cAddr);
        if ( pResults->chip.familyId != 0x4538 )
        {
            BDBG_MSG(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
            rc = BERR_INVALID_PARAMETER; goto done;
        }
    } else if (pSettings->spiDevice) {
        pResults->chip.familyId = (uint32_t)NEXUS_Platform_P_Spi_Get4538ChipId(pSettings->spiDevice, pSettings->spiAddr);
        if ( pResults->chip.familyId != 0x4538 )
        {
            BDBG_MSG(("pResults->chip.familyId = 0x%x", pResults->chip.familyId));
            rc = BERR_INVALID_PARAMETER; goto done;
        }
    } else { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto done; }

    rc = NEXUS_Frontend_P_Get4538ChipInfo(pSettings, pResults);

done:
    if (gpioHandle) {
        NEXUS_Gpio_Close(gpioHandle);
    }
    return rc;
}

static NEXUS_Error NEXUS_Frontend_P_4538_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)handle;
    NEXUS_4538Device *p4538Device = pAstDevice->settings.pDevice;
    BERR_Code rc = NEXUS_SUCCESS;
    BAST_4538_AgcStatus status;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NULL != p4538Device);

    if (pAstDevice->astChip != B_AST_CHIP) {
        return NEXUS_INVALID_PARAMETER;
    }

    errCode = BAST_4538_GetAgcStatus(pAstDevice->astChannel, &status);
    if (!errCode) {
        pStatus->agc[0].valid = status.bLnaGainValid;
        pStatus->agc[0].value = status.lnaGain;
        pStatus->agc[1].valid = status.bChanAgcValid;
        pStatus->agc[1].value = status.chanAgc;
        pStatus->agc[2].valid = true;
        pStatus->agc[2].value = status.tunerFreq;
        pStatus->agc[3].valid = true;
        pStatus->agc[3].value = status.adcSelect;
    } else {
        rc = NEXUS_OS_ERROR;
    }

    return rc;
}

static void NEXUS_FrontendDevice_P_4538_GetCapabilities(void * handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    NEXUS_4538Device *pDevice = (NEXUS_4538Device *)handle;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4538Device);

    pCapabilities->numTuners = pDevice->numChannels;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Open4538(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    NEXUS_FrontendDevice4538OpenSettings settings;
    int i;

    BDBG_ASSERT(pSettings);
    if (pSettings->satellite.enabled) {
        NEXUS_FrontendDevice_GetDefault4538OpenSettings(&settings);
        settings.i2cDevice = pSettings->i2cDevice;
        settings.i2cAddr = pSettings->i2cAddress;
        settings.gpioInterrupt = pSettings->gpioInterrupt;
        settings.isrNumber = pSettings->isrNumber;
        settings.spiDevice = pSettings->spiDevice;
        settings.spiAddr = 0x20;
        for (i=0; i < NEXUS_MAX_MTSIF; i++) {
            settings.mtsif[i] = pSettings->mtsif[i];
        }
        return NEXUS_FrontendDevice_Open4538(index, &settings);
    }
    return NULL;
}

NEXUS_FrontendHandle NEXUS_Frontend_P_Open4538(const NEXUS_FrontendChannelSettings *pSettings)
{
    NEXUS_4538Settings settings;

    BDBG_ASSERT(pSettings);
    BDBG_MSG(("NEXUS_Frontend_P_Open4538: %d",pSettings->channelNumber));
    if (pSettings->type == NEXUS_FrontendChannelType_eSatellite) {
        NEXUS_Frontend_GetDefault4538Settings(&settings);
        settings.device = pSettings->device;
        settings.channelNumber = pSettings->channelNumber;
        return NEXUS_Frontend_Open4538(&settings);
    }
    return NULL;
}
