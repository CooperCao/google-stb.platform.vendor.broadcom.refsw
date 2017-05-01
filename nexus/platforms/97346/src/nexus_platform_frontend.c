/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*   API name: Platform Frontend
*    Platform Frontend Setup
*
***************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_types.h"
#if NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND

#include "priv/nexus_core.h"
#include "nexus_i2c.h"
#include "nexus_frontend.h"
#include "nexus_platform_features.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "nexus_base.h"
#include "bchp_sun_top_ctrl.h"
#include "nexus_frontend_7346.h"
#if NEXUS_FRONTEND_4506
#include "nexus_frontend_4506.h"
#endif
#if NEXUS_FRONTEND_4538
#include "nexus_frontend_4538.h"
#endif
#if NEXUS_PLATFORM_7418SFF_H
#include "nexus_frontend_3128.h"
#endif
#if NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44
#include "nexus_frontend_3461.h"
#endif
#if NEXUS_PLATFORM_97346_H43
#if NEXUS_FRONTEND_7366
#include "nexus_frontend_7366.h"
#endif
#if NEXUS_FRONTEND_45216
#include "nexus_frontend_45216.h"
#endif
#endif

BDBG_MODULE(nexus_platform_frontend);

/* i2c channel assignments */
#define I2C_DEVICE_FPGA_CH 2
#if (BCHP_CHIP==7344)
#define I2C_DEVICE_VOLTAGE_REG_CH 1
#else /* 7346 */
#define I2C_DEVICE_VOLTAGE_REG_CH 3
#endif

#define ISL9492_CH0_I2C_ADDR 0x08
#define ISL9492_CH1_I2C_ADDR 0x09
#define FPGA_CHIP_ADDR 0xE

#if NEXUS_PLATFORM_97346_HR44 || NEXUS_PLATFORM_97346_SHR44
    static NEXUS_GpioHandle gpioHandle = NULL;
#endif

#if defined(NEXUS_PLATFORM_7418SFF_H)
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i;
    NEXUS_3128Settings st3128Settings;
    NEXUS_FrontendUserParameters userParams;

    /* Open downstream tuners */
    for (i=0; i < NEXUS_3128_MAX_DOWNSTREAM_CHANNELS; i++)
    {
        BDBG_WRN(("Waiting for frontend 3123 channel %d to initialize", i));

        NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);
        st3128Settings.isrNumber = 18;
        st3128Settings.i2cDevice = pConfig->i2c[2];    /* Onboard tuner/demod use BSC_M2.*/
        st3128Settings.i2cAddr = 0x6f;
        st3128Settings.type = NEXUS_3128ChannelType_eInBand;
        st3128Settings.channelNumber = i;
        st3128Settings.ifFrequency = 0;
        st3128Settings.inBandOpenDrain=true;
        st3128Settings.loadAP = true;
        st3128Settings.configureWatchdog = false;
        st3128Settings.isMtsif = false;

        pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_MSG(("Unable to open onboard 3123 tuner/demodulator %d", i));
            continue;
        }
        BDBG_MSG(("pConfig->frontend[%d]",i, pConfig->frontend[i]));
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (pConfig->frontend[i])
             NEXUS_Frontend_Close(pConfig->frontend[i]);
    }

    return;
}

#elif NEXUS_PLATFORM_97346_H43

#include "priv/nexus_i2c_priv.h"

static NEXUS_GpioHandle gpioHandle = NULL;
static NEXUS_GpioHandle gpioHandleInt = NULL;

#define NEXUS_PLATFORM_H43_GPIO_INTERRUPT 2
#if NEXUS_FRONTEND_45216
#define NEXUS_PLATFORM_H43_FRONTEND_STRING "45216"
#define NEXUS_PLATFORM_H43_FRONTEND_I2C_ADDRESS 0x68
#define NEXUS_PLATFORM_H43_MTSIF_OFFSET 2

#define NEXUS_PLATFORM_H43_OPEN_SETTINGS NEXUS_FrontendDevice45216OpenSettings
#define NEXUS_PLATFORM_H43_GET_DEFAULT_OPEN_SETTINGS NEXUS_FrontendDevice_GetDefault45216OpenSettings
#define NEXUS_PLATFORM_H43_DEVICE_OPEN NEXUS_FrontendDevice_Open45216
#define NEXUS_PLATFORM_H43_FRONTEND_SETTINGS NEXUS_45216FrontendSettings
#define NEXUS_PLATFORM_H43_FRONTEND_OPEN NEXUS_Frontend_Open45216

/* If using 45208/45216 on a V00 version of the board (requiring FSK controlled by the 7346), comment this out. */
#define NEXUS_PLATFORM_H43_V10 1

#elif NEXUS_FRONTEND_7366
#define NEXUS_PLATFORM_H43_FRONTEND_STRING "4548"
#define NEXUS_PLATFORM_H43_FRONTEND_I2C_ADDRESS 0x8
#define NEXUS_PLATFORM_H43_MTSIF_OFFSET 8

#define NEXUS_PLATFORM_H43_OPEN_SETTINGS NEXUS_FrontendDevice7366OpenSettings
#define NEXUS_PLATFORM_H43_GET_DEFAULT_OPEN_SETTINGS NEXUS_FrontendDevice_GetDefault7366OpenSettings
#define NEXUS_PLATFORM_H43_DEVICE_OPEN NEXUS_FrontendDevice_Open7366
#define NEXUS_PLATFORM_H43_FRONTEND_SETTINGS NEXUS_7366FrontendSettings
#define NEXUS_PLATFORM_H43_FRONTEND_OPEN NEXUS_Frontend_Open7366

#elif NEXUS_FRONTEND_4538
#define NEXUS_PLATFORM_H43_FRONTEND_STRING "4538"
#define NEXUS_PLATFORM_H43_FRONTEND_I2C_ADDRESS 0x68
#define NEXUS_PLATFORM_H43_MTSIF_OFFSET 0

#define NEXUS_PLATFORM_H43_OPEN_SETTINGS NEXUS_FrontendDevice4538OpenSettings
#define NEXUS_PLATFORM_H43_GET_DEFAULT_OPEN_SETTINGS NEXUS_FrontendDevice_GetDefault4538OpenSettings
#define NEXUS_PLATFORM_H43_DEVICE_OPEN NEXUS_FrontendDevice_Open4538
#define NEXUS_PLATFORM_H43_FRONTEND_SETTINGS NEXUS_4538Settings
#define NEXUS_PLATFORM_H43_FRONTEND_OPEN NEXUS_Frontend_Open4538

#else
#error "Unsupported H43 frontend"
#endif

#if !NEXUS_PLATFORM_H43_V10
/* Note: This platform requires the 7346 FSK to be initialized.  This is done by opening a 7346 frontend,
 * and allowing power management to power everything else down.  We keep the handle so we can
 * uninitialize it during shutdown, but the resulting frontend handle is not exposed to the user
 * since it is not connected to any inputs.
 */
static NEXUS_FrontendHandle g_frontend7346[2] = {NULL,NULL};
#endif

#if NEXUS_PLATFORM_H43_V10
#include "bchp_sds_tuner_0.h"
#include "bchp_sds_tuner_1.h"
#include "bchp_sds_dsec_0.h"
#include "bchp_sds_dsec_1.h"
#include "bchp_sds_cg_0.h"
#include "bchp_sds_cg_1.h"
#include "bchp_tfec_misc_0.h"
#include "bchp_tfec_misc_1.h"
#include "bchp_afec_global_0.h"
#include "bchp_afec_global_1.h"
#include "bchp_clkgen.h"
#include "bchp_ftm_phy_ana.h"

/* This function powers up or down (based on enable) all of the 7346 internal
 * frontend components. Do NOT call if actually using the 7346 internal frontend
 * for ANYTHING, including tuning, diseqc, or fsk control.
 *
 * Note that if the internal frontend is already powered down and this is called
 * to power it down, there may be errors writing to some registers.
 *
 * Powering down the frontend and then exiting without powering up (e.g. by breaking
 * out of execution) may cause issues starting Nexus again without a reboot.
 *
 * Power up values are the reset values for the individual bits. Where possible,
 * each register is written only once. When bits are combined, the work with the
 * masks is shown in bits in a comment. */

/* Do not enable this if the application can exit without cleanup. This can cause issues
 * when writing to certain other registers. */
#define NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS 0
static void NEXUS_Platform_P_EnableInternal7346FrontendClocks(bool enableFrontend)
{
    uint32_t val;

    int enable  = enableFrontend ? 1 : 0; /* shortcut to bits which enable */
    int disable = enableFrontend ? 0 : 1; /* shortcut to bits which disable */

#if NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS
    NEXUS_Platform_ReadRegister(0x00420334,&val);
    val &= 0xFFFFFFFE;
    val &= 0x00000001;
    val |= enable;
    NEXUS_Platform_WriteRegister(0x00420334,val);
#else
    BSTD_UNUSED(enable);
#endif
    NEXUS_Platform_ReadRegister(0x00420210,&val);
    val &= 0xFFFFFFFE;
    val &= 0x00000001;
    val |= disable;
    NEXUS_Platform_WriteRegister(0x00420210,val);

#if NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS
    NEXUS_Platform_ReadRegister(0x00420380,&val);
    val &= 0xFFFFFFFE;
    val &= 0x00000001;
    val |= enable;
    NEXUS_Platform_WriteRegister(0x00420380,val);
#endif
    NEXUS_Platform_ReadRegister(0x00420284,&val);
    val &= 0xFFFFFFFE;
    val &= 0x00000001;
    val |= disable;
    NEXUS_Platform_WriteRegister(0x00420284,val);
}

static void NEXUS_Platform_P_EnableInternal7346Sds0(bool enableFrontend)
{
    uint32_t val;

    int disable = enableFrontend ? 0 : 1; /* shortcut to bits which disable */

    /* force enable */
    /* SPLL */
    NEXUS_Platform_ReadRegister(BCHP_SDS_CG_0_SPLL_PWRDN_RST,&val);
    val &= 0xFFFFFFF8; /* 1111 1111 1111 1111  1111 1111 1111 1000 */
    val &= 0x00000007; /* Mask off the reserved bits */
    val |= 0x00000000;
    NEXUS_Platform_WriteRegister(BCHP_SDS_CG_0_SPLL_PWRDN_RST,val);

    /* demod */
    NEXUS_Platform_ReadRegister(BCHP_SDS_CG_0_SPLL_MDIV_CTRL,&val);
    val &= ~BCHP_SDS_CG_0_SPLL_MDIV_CTRL_channel_en_MASK;
    val &= 0x000FFFFF; /* Mask off the reserved bits */
    val |= 0 << BCHP_SDS_CG_0_SPLL_MDIV_CTRL_channel_en_SHIFT;
    val = 0x00024006;
    NEXUS_Platform_WriteRegister(BCHP_SDS_CG_0_SPLL_MDIV_CTRL,val);

    /* enable SDS0/1 common, regardless of enable/disable state. Real state is written later. */
    NEXUS_Platform_ReadRegister(BCHP_SDS_TUNER_0_PWRUP_COMMON_R01,&val);
    val &= 0xFFFFFFE0; /* 1111 1111 1111 1111  1111 1111 1110 0000 */
    val &= 0x0000001F; /* Mask off the reserved bits, must be written as 0 */
    val |= 0x0000000F;
    NEXUS_Platform_WriteRegister(BCHP_SDS_TUNER_0_PWRUP_COMMON_R01,val);

    /* SDS0 */
    /* Tuner */
    NEXUS_Platform_ReadRegister(BCHP_SDS_TUNER_0_PWRUP_R01,&val);
    val &= 0xFF828020; /* 1111 1111 1000 0010  1000 0000 0010 0000 */
    val &= 0x007FFFFF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x007D7FDF : 0x00000000;
    NEXUS_Platform_WriteRegister(BCHP_SDS_TUNER_0_PWRUP_R01,val);

    /* Diseqc */
    NEXUS_Platform_ReadRegister(BCHP_SDS_DSEC_0_DS_COMMON_CONTROL,&val);
    val &= ~BCHP_SDS_DSEC_0_DS_COMMON_CONTROL_adc_pwrdn_MASK;
    val |= disable << BCHP_SDS_DSEC_0_DS_COMMON_CONTROL_adc_pwrdn_SHIFT;
    NEXUS_Platform_WriteRegister(BCHP_SDS_DSEC_0_DS_COMMON_CONTROL,val);
    NEXUS_Platform_ReadRegister(BCHP_SDS_DSEC_0_DSCTL02,&val);
    val &= ~BCHP_SDS_DSEC_0_DSCTL02_RXPWRDN_MASK;
    val |= disable << BCHP_SDS_DSEC_0_DSCTL02_RXPWRDN_SHIFT;
    NEXUS_Platform_WriteRegister(BCHP_SDS_DSEC_0_DSCTL02,val);

    /* AFEC */
    if (enableFrontend) {
        NEXUS_Platform_ReadRegister(0x00420234,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 0 : 1;
        NEXUS_Platform_WriteRegister(0x00420234,val);
        NEXUS_Platform_ReadRegister(0x004203c0,&val);
        val &= 0xFFFFFFFC;
        val &= 0x00000003;
        val |= enableFrontend ? 0 : 3;
        NEXUS_Platform_WriteRegister(0x004203c0,val);
#if NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS
        NEXUS_Platform_ReadRegister(0x004202fc,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 1 : 0;
        NEXUS_Platform_WriteRegister(0x004202fc,val);
#endif
    }
    NEXUS_Platform_ReadRegister(BCHP_AFEC_GLOBAL_0_CLK_CNTRL,&val);
    val &= 0xFF77FFFF; /* 1111 1111 0111 0111  1111 1111 1111 1111 */
    val &= 0x008888FF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00800000 : 0x00080000;
    NEXUS_Platform_WriteRegister(BCHP_AFEC_GLOBAL_0_CLK_CNTRL,val);
    NEXUS_Platform_ReadRegister(BCHP_CLKGEN_PLL_VCXO_PLL_CHANNEL_CTRL_CH_3,&val);
    val &= 0xFFFFFFEE; /* 1111 1111 1111 1111  1111 1111 1110 1110 */
    val &= 0x00003FFF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00000000 : 0x00000011;
    NEXUS_Platform_WriteRegister(BCHP_CLKGEN_PLL_VCXO_PLL_CHANNEL_CTRL_CH_3,val);

    if (!enableFrontend) {
#if NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS
        NEXUS_Platform_ReadRegister(0x004202fc,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 1 : 0;
        NEXUS_Platform_WriteRegister(0x004202fc,val);
#endif
        NEXUS_Platform_ReadRegister(0x004203c0,&val);
        val &= 0xFFFFFFFC;
        val &= 0x00000003;
        val |= enableFrontend ? 0 : 3;
        NEXUS_Platform_WriteRegister(0x004203c0,val);
        NEXUS_Platform_ReadRegister(0x00420234,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 0 : 1;
        NEXUS_Platform_WriteRegister(0x00420234,val);
    }

    /* TFEC */
    NEXUS_Platform_ReadRegister(BCHP_TFEC_MISC_0_POST_DIV_CTL,&val);
    val &= ~BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_en_MASK;
    val &= 0xFFFF0000; /* Mask off the reserved bits */
    val |= disable << BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_en_SHIFT;
    NEXUS_Platform_WriteRegister(BCHP_TFEC_MISC_0_POST_DIV_CTL,val);

    /* SDS0/1 common */
    NEXUS_Platform_ReadRegister(BCHP_SDS_TUNER_0_PWRUP_COMMON_R01,&val);
    val &= 0xFFFFFFE0; /* 1111 1111 1111 1111  1111 1111 1110 0000 */
    val &= 0x0000001F; /* Mask off the reserved bits, must be written as 0 */
    val |= enableFrontend ? 0x0000000F : 0x00000000;
    NEXUS_Platform_WriteRegister(BCHP_SDS_TUNER_0_PWRUP_COMMON_R01,val);

    if (!enableFrontend) {
        /* demod */
        NEXUS_Platform_ReadRegister(BCHP_SDS_CG_0_SPLL_MDIV_CTRL,&val);
        val &= ~BCHP_SDS_CG_0_SPLL_MDIV_CTRL_channel_en_MASK;
        val &= 0x000FFFFF; /* Mask off the reserved bits */
        val |= disable << BCHP_SDS_CG_0_SPLL_MDIV_CTRL_channel_en_SHIFT;
        NEXUS_Platform_WriteRegister(BCHP_SDS_CG_0_SPLL_MDIV_CTRL,val);

        /* SPLL */
        NEXUS_Platform_ReadRegister(BCHP_SDS_CG_0_SPLL_PWRDN_RST,&val);
        val &= 0xFFFFFFF8; /* 1111 1111 1111 1111  1111 1111 1111 1000 */
        val &= 0x00000007; /* Mask off the reserved bits */
        val |= enableFrontend ? 0x00000000 : 0x00000007;
        NEXUS_Platform_WriteRegister(BCHP_SDS_CG_0_SPLL_PWRDN_RST,val);
    }
}

static void NEXUS_Platform_P_EnableInternal7346Sds1(bool enableFrontend)
{
    uint32_t val;

    int disable = enableFrontend ? 0 : 1; /* shortcut to bits which disable */

    if (enableFrontend) {
        /* SPLL */
        NEXUS_Platform_ReadRegister(BCHP_SDS_CG_1_SPLL_PWRDN_RST,&val);
        val &= 0xFFFFFFF8; /* 1111 1111 1111 1111  1111 1111 1111 1000 */
        val &= 0x00000007; /* Mask off the reserved bits */
        val |= enableFrontend ? 0x00000000 : 0x00000007;
        NEXUS_Platform_WriteRegister(BCHP_SDS_CG_1_SPLL_PWRDN_RST,val);

        /* demod */
        NEXUS_Platform_ReadRegister(BCHP_SDS_CG_1_SPLL_MDIV_CTRL,&val);
        val &= 0xFFFFFBFF; /* 1111 1111 1111 1111  1111 1011 1111 1111 */
        val &= 0x000FFFFF; /* Mask off the reserved bits */
        val |= enableFrontend ? 0x00000000 : 0x00000400;
        NEXUS_Platform_WriteRegister(BCHP_SDS_CG_1_SPLL_MDIV_CTRL,val);
    }

    /* SDS1 */
    /* Tuner */
    NEXUS_Platform_ReadRegister(BCHP_SDS_TUNER_1_PWRUP_R01,&val);
    val &= 0xFF828020; /* 1111 1111 1000 0010  1000 0000 0010 0000 */
    val &= 0x007FFFFF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x007D7FDF : 0x00000000;
    NEXUS_Platform_WriteRegister(BCHP_SDS_TUNER_1_PWRUP_R01,val);

    /* Diseqc */ /* Note: DSEC_1 masks are shared with DSEC_0 */
    NEXUS_Platform_ReadRegister(BCHP_SDS_DSEC_1_DS_COMMON_CONTROL,&val);
    val &= ~BCHP_SDS_DSEC_0_DS_COMMON_CONTROL_adc_pwrdn_MASK;
    val |= disable << BCHP_SDS_DSEC_0_DS_COMMON_CONTROL_adc_pwrdn_SHIFT;
    NEXUS_Platform_WriteRegister(BCHP_SDS_DSEC_1_DS_COMMON_CONTROL,val);
    NEXUS_Platform_ReadRegister(BCHP_SDS_DSEC_1_DSCTL02,&val);
    val &= ~BCHP_SDS_DSEC_0_DSCTL02_RXPWRDN_MASK;
    val |= disable << BCHP_SDS_DSEC_0_DSCTL02_RXPWRDN_SHIFT;
    NEXUS_Platform_WriteRegister(BCHP_SDS_DSEC_1_DSCTL02,val);

    /* AFEC */
    if (enableFrontend) {
        NEXUS_Platform_ReadRegister(0x004202dc,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 0 : 1;
        NEXUS_Platform_WriteRegister(0x004202dc,val);
        NEXUS_Platform_ReadRegister(0x00420388,&val);
        val &= 0xFFFFFFFC;
        val &= 0x00000003;
        val |= enableFrontend ? 0 : 3;
        NEXUS_Platform_WriteRegister(0x00420388,val);
#if NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS
        NEXUS_Platform_ReadRegister(0x0042028c,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 1 : 0;
        NEXUS_Platform_WriteRegister(0x0042028c,val);
#endif
    }
    NEXUS_Platform_ReadRegister(BCHP_AFEC_GLOBAL_1_CLK_CNTRL,&val);
    val &= 0xFF77FFFF; /* 1111 1111 0111 0111  1111 1111 1111 1111 */
    val &= 0x008888FF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00800000 : 0x00080000;
    NEXUS_Platform_WriteRegister(BCHP_AFEC_GLOBAL_1_CLK_CNTRL,val);
    NEXUS_Platform_ReadRegister(BCHP_CLKGEN_PLL_AFEC_PLL_CHANNEL_CTRL_CH_0,&val);
    val &= 0xFFFFFFEE; /* 1111 1111 1111 1111  1111 1111 1110 1110 */
    val &= 0x00003FFF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00000000 : 0x00000011;
    NEXUS_Platform_WriteRegister(BCHP_CLKGEN_PLL_AFEC_PLL_CHANNEL_CTRL_CH_0,val);

    if (!enableFrontend) {
#if NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS
        NEXUS_Platform_ReadRegister(0x0042028c,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 1 : 0;
        NEXUS_Platform_WriteRegister(0x0042028c,val);
#endif
        NEXUS_Platform_ReadRegister(0x00420388,&val);
        val &= 0xFFFFFFFC;
        val &= 0x00000003;
        val |= enableFrontend ? 0 : 3;
        NEXUS_Platform_WriteRegister(0x00420388,val);
        NEXUS_Platform_ReadRegister(0x004202dc,&val);
        val &= 0xFFFFFFFE;
        val &= 0x00000001;
        val |= enableFrontend ? 0 : 1;
        NEXUS_Platform_WriteRegister(0x004202dc,val);
    }

    /* TFEC */
    NEXUS_Platform_ReadRegister(BCHP_TFEC_MISC_1_POST_DIV_CTL,&val);
    val &= ~BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_en_MASK;
    val &= 0xFFFF0000; /* Mask off the reserved bits */
    val |= disable << BCHP_TFEC_MISC_0_POST_DIV_CTL_tfec_clk_en_SHIFT;
    NEXUS_Platform_WriteRegister(BCHP_TFEC_MISC_1_POST_DIV_CTL,val);

    if (enableFrontend) {
        /* SPLL */
        NEXUS_Platform_ReadRegister(BCHP_SDS_CG_1_SPLL_PWRDN_RST,&val);
        val &= 0xFFFFFFF8; /* 1111 1111 1111 1111  1111 1111 1111 1000 */
        val &= 0x00000007; /* Mask off the reserved bits */
        val |= enableFrontend ? 0x00000000 : 0x00000007;
        NEXUS_Platform_WriteRegister(BCHP_SDS_CG_1_SPLL_PWRDN_RST,val);
    }
    /* demod */
    NEXUS_Platform_ReadRegister(BCHP_SDS_CG_1_SPLL_MDIV_CTRL,&val);
    val &= 0xFFFFFBFF; /* 1111 1111 1111 1111  1111 1011 1111 1111 */
    val &= 0x000FFFFF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00000000 : 0x00000400;
    NEXUS_Platform_WriteRegister(BCHP_SDS_CG_1_SPLL_MDIV_CTRL,val);

    if (!enableFrontend) {
        /* SPLL */
        NEXUS_Platform_ReadRegister(BCHP_SDS_CG_1_SPLL_PWRDN_RST,&val);
        val &= 0xFFFFFFF8; /* 1111 1111 1111 1111  1111 1111 1111 1000 */
        val &= 0x00000007; /* Mask off the reserved bits */
        val |= enableFrontend ? 0x00000000 : 0x00000007;
        NEXUS_Platform_WriteRegister(BCHP_SDS_CG_1_SPLL_PWRDN_RST,val);
    }
}

static void NEXUS_Platform_P_EnableInternal7346FtmCommon(bool enableFrontend)
{
    uint32_t val;

    uint32_t disable = enableFrontend ? 0 : 1; /* shortcut to bits which disable */

    if (enableFrontend) {
        NEXUS_Platform_ReadRegister(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1,&val);
        val &= ~BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK;
        val &= 0x0000003F; /* Mask off the reserved bits */
        val |= disable;
        NEXUS_Platform_WriteRegister(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1,val);
    }

    if (!enableFrontend) {
        NEXUS_Platform_ReadRegister(0x00706200,&val);
        val &= 0xFFFFFFFE; /* 1111 1111 1111 1111  1101 1111 1111 1111 */
        val &= 0xE00F2CBD; /* Mask off the reserved bits */
        val |= enableFrontend ? 0x00000000 : 0x00000001;
        NEXUS_Platform_WriteRegister(0x00706200,val);
    }

#if NEXUS_PLATFORM_7346_DISABLE_108_CLOCKS
    NEXUS_Platform_ReadRegister(0x00420218,&val);
    val = enableFrontend ? 0x00000007 : 0x00000000;
    NEXUS_Platform_WriteRegister(0x00420218,val);
#endif

    if (enableFrontend) {
        NEXUS_Platform_ReadRegister(0x00706200,&val);
        val &= 0xFFFFFFFE; /* 1111 1111 1111 1111  1101 1111 1111 1111 */
        val &= 0xE00F2CBD; /* Mask off the reserved bits */
        val |= enableFrontend ? 0x00000000 : 0x00000001;
        NEXUS_Platform_WriteRegister(0x00706200,val);
    }

    NEXUS_Platform_ReadRegister(0x004202f4,&val);
    val = enableFrontend ? 0x00000000 : 0x00000003;
    NEXUS_Platform_WriteRegister(0x004202f4,val);

    NEXUS_Platform_ReadRegister(0x004202d4,&val);
    val = disable;
    NEXUS_Platform_WriteRegister(0x004202d4,val);

    if (!enableFrontend) {
        NEXUS_Platform_ReadRegister(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1,&val);
        val &= ~BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1_CLOCK_DIS_CH1_MASK;
        val &= 0x0000003F; /* Mask off the reserved bits */
        val |= enableFrontend ? 0 : 1;
        NEXUS_Platform_WriteRegister(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1,val);
    }
}

static void NEXUS_Platform_P_EnableInternal7346Ftm0(bool enableFrontend)
{
    uint32_t val;

    if (enableFrontend) {
        NEXUS_Platform_P_EnableInternal7346FtmCommon(true);
    }

    NEXUS_Platform_ReadRegister(0x00706200,&val);
    val &= 0xFFFFFF7F; /* 1111 1111 1111 1111  1111 1111 0111 1111 */
    val &= 0xE00F2CBD; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00000000 : 0x00000080;
    NEXUS_Platform_WriteRegister(0x00706200,val);

    NEXUS_Platform_ReadRegister(BCHP_FTM_PHY_ANA_CTL0_1,&val);
    val &= 0xFFFFFEDF; /* 1111 1111 1111 1111  1111 1110 1101 1111 */
    val &= 0xFFFF07FF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00000000 : 0x00000120;
    NEXUS_Platform_WriteRegister(BCHP_FTM_PHY_ANA_CTL0_1,val);

    if (!enableFrontend) {
        NEXUS_Platform_P_EnableInternal7346FtmCommon(enableFrontend);
    }
}

static void NEXUS_Platform_P_EnableInternal7346Ftm1(bool enableFrontend)
{
    uint32_t val;

    if (!enableFrontend) {
        NEXUS_Platform_P_EnableInternal7346FtmCommon(true);
    }

    NEXUS_Platform_ReadRegister(0x00706200,&val);
    val &= 0xFFFFDFFF; /* 1111 1111 1111 1111  1101 1111 1111 1111 */
    val &= 0xE00F2CBD; /* Mask off the reserved bits */ /* 1110 0000 0000 1111  0010 1100 1011 1101 */
    val |= enableFrontend ? 0x00000000 : 0x00002000;
    NEXUS_Platform_WriteRegister(0x00706200,val);

    NEXUS_Platform_ReadRegister(BCHP_FTM_PHY_ANA_CTL1_1,&val);
    val &= 0xFFFFFEDF; /* 1111 1111 1111 1111  1111 1110 1101 1111 */
    val &= 0xFFFF07FF; /* Mask off the reserved bits */
    val |= enableFrontend ? 0x00000000 : 0x00000120;
    NEXUS_Platform_WriteRegister(BCHP_FTM_PHY_ANA_CTL1_1,val);
}

static void NEXUS_Platform_P_EnableInternal7346Frontend(bool enableFrontend)
{
    NEXUS_Platform_P_EnableInternal7346FrontendClocks(true); /* enable clocks for register writes */

    if (!enableFrontend) {
        NEXUS_Platform_P_EnableInternal7346Sds1(enableFrontend);
    }
    NEXUS_Platform_P_EnableInternal7346Sds0(enableFrontend);
    if (enableFrontend) {
        NEXUS_Platform_P_EnableInternal7346Sds1(enableFrontend);
    }

    if (!enableFrontend) {
        NEXUS_Platform_P_EnableInternal7346Ftm1(enableFrontend);
    }
    NEXUS_Platform_P_EnableInternal7346Ftm0(enableFrontend);
    if (enableFrontend) {
        NEXUS_Platform_P_EnableInternal7346Ftm1(enableFrontend);
    }

    NEXUS_Platform_P_EnableInternal7346FrontendClocks(enableFrontend);
}
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_PLATFORM_H43_OPEN_SETTINGS deviceSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_GpioSettings gpioSettings;
    unsigned i=0;

    NEXUS_PLATFORM_H43_GET_DEFAULT_OPEN_SETTINGS(&deviceSettings);
#if NEXUS_FRONTEND_7366
    deviceSettings.mtsif[0].clockRate = 108000000; /* Initialize at 108 MHz. */
#endif

    deviceSettings.reset.enable = true;
    deviceSettings.reset.pin = 3;
    deviceSettings.reset.type = NEXUS_GpioType_eStandard;
    deviceSettings.reset.value = NEXUS_GpioValue_eHigh;

    deviceSettings.i2cDevice = pConfig->i2c[2];
    deviceSettings.i2cAddr = NEXUS_PLATFORM_H43_FRONTEND_I2C_ADDRESS;

    BDBG_MSG(("Setting up interrupt on GPIO %d",NEXUS_PLATFORM_H43_GPIO_INTERRUPT));
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, NEXUS_PLATFORM_H43_GPIO_INTERRUPT, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandleInt);
    deviceSettings.gpioInterrupt = gpioHandleInt;

    BDBG_MSG(("Opening %s device",NEXUS_PLATFORM_H43_FRONTEND_STRING));
    device = NEXUS_PLATFORM_H43_DEVICE_OPEN(0, &deviceSettings);

    if (device) {
        NEXUS_FrontendDeviceCapabilities capabilities;
#if NEXUS_FRONTEND_45216 || NEXUS_FRONTEND_7366
        NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
#elif NEXUS_FRONTEND_4538
        capabilities.numTuners = 8;
#endif
        BDBG_MSG(("Opening %d %s frontends",capabilities.numTuners,NEXUS_PLATFORM_H43_FRONTEND_STRING));
        for (i=0; i < capabilities.numTuners; i++)
        {
            NEXUS_PLATFORM_H43_FRONTEND_SETTINGS channelSettings;

            channelSettings.device = device;
            channelSettings.channelNumber = i;
            pConfig->frontend[i] = NEXUS_PLATFORM_H43_FRONTEND_OPEN(&channelSettings);
            if ( NULL == (pConfig->frontend[i]) )
            {
                BDBG_ERR(("Unable to open %s demod %d",NEXUS_PLATFORM_H43_FRONTEND_STRING,i));
                continue;
            }
            BDBG_MSG(("%sfe: %d:%p",NEXUS_PLATFORM_H43_FRONTEND_STRING,i,(void *)pConfig->frontend[i]));
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.isMtsif = true;
            userParams.param1 = userParams.isMtsif ? channelSettings.channelNumber + NEXUS_PLATFORM_H43_MTSIF_OFFSET : NEXUS_InputBand_e0 + i;
            userParams.pParam2 = 0;
            BDBG_MSG(("%sfe: %d:%p: (%s,%i)",NEXUS_PLATFORM_H43_FRONTEND_STRING,i,(void *)pConfig->frontend[i],userParams.isMtsif ? "mtsif" : "not mtsif",userParams.param1));
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        }
#if NEXUS_FRONTEND_7366
    for (i=0; i < capabilities.numTuners; i++)
    {
        NEXUS_Error ec;
        NEXUS_Frontend7366RuntimeSettings settings;
        ec = NEXUS_Frontend_Get7366RuntimeSettings(pConfig->frontend[i], &settings);
        settings.selectedAdc = 1;
        if (!ec) {
            ec = NEXUS_Frontend_Set7366RuntimeSettings(pConfig->frontend[i], &settings);
            if (ec) BERR_TRACE(ec);
        } else {
            BERR_TRACE(ec);
        }
    }
#endif
    } else {
        BDBG_ERR(("Unable to open %s device",NEXUS_PLATFORM_H43_FRONTEND_STRING));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

#if NEXUS_PLATFORM_H43_V10
    NEXUS_Platform_P_EnableInternal7346Frontend(true); /* recover first in case of abnormal exit */
    NEXUS_Platform_P_EnableInternal7346Frontend(false);
#else
    {
        NEXUS_7346FrontendSettings open7346Settings;
        BDBG_MSG(("Opening 7346 placeholder..."));
        NEXUS_Frontend_GetDefault7346Settings(&open7346Settings);
        for (i=0; i < 2; i++) {
            open7346Settings.channelNumber = i; /* no need to open 1 */
            g_frontend7346[i] = NEXUS_Frontend_Open7346(&open7346Settings);
            NEXUS_Frontend_Untune(g_frontend7346[i]);
        }
    }
#endif

    return NEXUS_SUCCESS;
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
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }

    if(gpioHandleInt)
    {
        NEXUS_Gpio_Close(gpioHandleInt);
        gpioHandleInt = NULL;
    }

#if NEXUS_PLATFORM_H43_V10
    NEXUS_Platform_P_EnableInternal7346Frontend(true);
#else
    {
        tempHandle = NULL;
        for (i=0; i < 2; i++) {
            if (g_frontend7346[i]) {
                tempHandle = NEXUS_Frontend_GetDevice(g_frontend7346[i]);
                BDBG_MSG(("Closing 7346 placeholder[%d]",i));
                NEXUS_Frontend_Close(g_frontend7346[i]);
                g_frontend7346[i] = NULL;
            }
        }
        if (tempHandle) {
            NEXUS_FrontendDevice_Close(tempHandle);
        }
    }
#endif

    return;
}
#elif NEXUS_PLATFORM_97346_SHR44
#ifdef USE_SPI_FRONTEND
static NEXUS_SpiHandle g_3472spi[NEXUS_NUM_SPI_CHANNELS];
#endif

#include "priv/nexus_i2c_priv.h"

#define I2C_4538_INDEX_1 2
#define NUM_4538_CHANNELS_PER 8
#define I2C_4538_ADDRESS_1 0x68

#ifndef USE_SPI_FRONTEND
static bool NEXUS_Platform_P_Is4538(NEXUS_I2cHandle i2cDevice, uint16_t i2cAddr)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint16_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x",(void *)i2cHandle,i2cAddr));
    buf[0]= 0x0;
    subAddr = 0x1;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = buf[0];

    subAddr = 0x2;
    BREG_I2C_WriteNoAddr(i2cHandle, i2cAddr, (uint8_t *)&subAddr, 1);
    BREG_I2C_ReadNoAddr(i2cHandle, i2cAddr, buf, 1);
    chipId = (chipId <<8) | buf[0];

    BDBG_MSG(("chip ID = 0x%04x", chipId));

    return chipId == 0x4538;
}
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3461Settings st3472Settings;
    NEXUS_3461ProbeResults results;
    NEXUS_FrontendDevice3461OpenSettings deviceOpenSettings;
    NEXUS_FrontendDevice3461Settings deviceSettings;
    NEXUS_FrontendDeviceHandle parentDevice = NULL;
#ifdef USE_SPI_FRONTEND
    NEXUS_SpiSettings spiSettings;
#else
    unsigned j=0;
    NEXUS_FrontendDeviceLinkSettings linkSettings;
    uint16_t i2cAddr = I2C_4538_ADDRESS_1;
    NEXUS_4538Settings st4538Settings;
#endif

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eHigh;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioSettings.interrupt.callback = NULL;

    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 1, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
    NEXUS_Gpio_Close(gpioHandle);
    gpioHandle = NULL;

    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 4, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
    NEXUS_Gpio_Close(gpioHandle);
    gpioHandle = NULL;

    /* delay for reset extension */
    BKNI_Sleep(100);

    NEXUS_Frontend_GetDefault3461Settings(&st3472Settings);

    NEXUS_FrontendDevice_GetDefault3461OpenSettings(&deviceOpenSettings);
#ifdef USE_SPI_FRONTEND
    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = true;
    g_3472spi[0] = NEXUS_Spi_Open(0, &spiSettings);
    if (!g_3472spi[0]) {
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }
    deviceOpenSettings.i2cDevice = NULL;
    deviceOpenSettings.spiDevice = g_3472spi[0];
    deviceOpenSettings.spiAddr = 0x40;
#else
    if (!pConfig->i2c[3]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    deviceOpenSettings.i2cDevice = pConfig->i2c[3];/* BSC_M3 */
    deviceOpenSettings.i2cAddr = 0x6e;
#endif
    deviceOpenSettings.isrNumber = 21;
    deviceOpenSettings.inBandOpenDrain=true;
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.isMtsif = true;
    deviceOpenSettings.crystalSettings.enableDaisyChain = true;

    rc = NEXUS_Frontend_Probe3461(&deviceOpenSettings, &results);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    BDBG_MSG(("familyId = 0x%x", results.chip.familyId));
    BDBG_MSG(("chip.id = 0x%x", results.chip.id));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.major ));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.minor ));

    parentDevice = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if (NULL == parentDevice)
    {
        BDBG_ERR(("Unable to open 3472 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eOff;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = false;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(parentDevice, &deviceSettings);

    st3472Settings.type = NEXUS_3461ChannelType_eIsdbt;
    st3472Settings.device = parentDevice;
    for (i=0; i<(results.chip.id & 0xF); i++)
    {
        BDBG_MSG(("Waiting for onboard 3472 tuner/demodulator channel %d to initialize", i));
        st3472Settings.channelNumber = i;
        pConfig->frontend[i] = NEXUS_Frontend_Open3461(&st3472Settings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_MSG(("Unable to open 3472 tuner/demodulator channel %d", i));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = deviceOpenSettings.isMtsif ? st3472Settings.channelNumber : NEXUS_InputBand_e0+i;
        userParams.isMtsif = deviceOpenSettings.isMtsif;
        userParams.chipId = 0x3472;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        BDBG_MSG(("pConfig->frontend[%d] = 0x%x", i, pConfig->frontend[i]));
    }

#ifndef USE_SPI_FRONTEND
    deviceOpenSettings.i2cDevice = pConfig->i2c[3];/* BSC_M3 */
    deviceOpenSettings.i2cAddr = 0x6c;
    deviceOpenSettings.isrNumber = 22;

    rc = NEXUS_Frontend_Probe3461(&deviceOpenSettings, &results);
    rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    BDBG_MSG(("familyId = 0x%x", results.chip.familyId));
    BDBG_MSG(("chip.id = 0x%x", results.chip.id));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.major ));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.minor ));

    st3472Settings.device = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if (NULL == st3472Settings.device)
    {
        BDBG_ERR(("Unable to open 3472 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eOff;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = false;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(st3472Settings.device, &deviceSettings);

    st3472Settings.type = NEXUS_3461ChannelType_eIsdbt;
    for (j=0; j<(results.chip.id & 0xF); j++,i++)
    {
        BDBG_MSG(("Waiting for onboard 3472 tuner/demodulator channel %d to initialize", j));
        st3472Settings.channelNumber = j;
        pConfig->frontend[i] = NEXUS_Frontend_Open3461(&st3472Settings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_MSG(("Unable to open 3472 tuner/demodulator channel %d", j));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = deviceOpenSettings.isMtsif ? st3472Settings.channelNumber : NEXUS_InputBand_e0+i;
        userParams.isMtsif = deviceOpenSettings.isMtsif;
        userParams.chipId = 0x3472;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        BDBG_MSG(("pConfig->frontend[%d] = 0x%x", i, pConfig->frontend[i]));
    }


    NEXUS_FrontendDevice_GetDefaultLinkSettings(&linkSettings);
    linkSettings.rfInput = NEXUS_FrontendDeviceRfInput_eOff;
    linkSettings.mtsif = NEXUS_FrontendDeviceMtsifOutput_eDaisy;
    rc = NEXUS_FrontendDevice_Link(parentDevice, st3472Settings.device, &linkSettings);
    rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;

/* 4538 code. */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eHigh;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioSettings.interrupt.callback = NULL;
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, 0, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
    NEXUS_Gpio_Close(gpioHandle);

    /* delay for reset extension */
    BKNI_Sleep(200);

    BDBG_MSG(("Checking i2c: 0x%02x",I2C_4538_ADDRESS_1));
    if (!NEXUS_Platform_P_Is4538(pConfig->i2c[I2C_4538_INDEX_1],I2C_4538_ADDRESS_1)) {
#if NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH
        BDBG_ERR(("Unable to locate 4538 at 0x%02x",I2C_4538_ADDRESS_1));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
#else
        int ix;
        for (ix=0x68; ix<=0x6f; ix++) {
            BDBG_MSG(("Checking i2c: 0x%02x",ix));
            if (ix != I2C_4538_ADDRESS_1 && NEXUS_Platform_P_Is4538(pConfig->i2c[I2C_4538_INDEX_1],ix)) {
                i2cAddr = ix;
                BDBG_MSG(("Found 4538 at 0x%02x",ix));
                break;
            }
        }
        if (i2cAddr == I2C_4538_ADDRESS_1) {
            BDBG_ERR(("Unable to locate 4538"));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
        }
#endif
    }

    NEXUS_Frontend_GetDefault4538Settings(&st4538Settings);

    st4538Settings.i2cDevice = pConfig->i2c[I2C_4538_INDEX_1];
    st4538Settings.i2cAddr = i2cAddr;
    st4538Settings.isrNumber = 20;
    st4538Settings.gpioInterrupt = NULL;

    for (; i <  NUM_4538_CHANNELS_PER; i++)
    {
        st4538Settings.channelNumber = i;
        pConfig->frontend[i] = NEXUS_Frontend_Open4538(&st4538Settings);
        if ( NULL == (pConfig->frontend[i]) )
        {
            BDBG_ERR(("Unable to open onboard 4538 tuner/demodulator %d",i));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = true;
        userParams.param1 = userParams.isMtsif ? st4538Settings.channelNumber : NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }
#endif
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
        /* Close extensions first */
        if (pConfig->frontend[i])
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                    handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }
#ifdef USE_SPI_FRONTEND
    for (i=0; i<NEXUS_NUM_SPI_CHANNELS; i++)
    {
        if (g_3472spi[i])
        {
            NEXUS_Spi_Close(g_3472spi[i]);
            g_3472spi[i] = NULL;
        }
    }
#endif

#if NEXUS_PLATFORM_97346_HR44 || NEXUS_PLATFORM_97346_SHR44
    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
#endif

    return;
}
#else

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
#if (BCHP_CHIP == 7346) || (BCHP_CHIP == 73465)
    uint8_t  data_c, data_d, data_e, regaddr=0;
#endif
    NEXUS_7346FrontendSettings settings;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    uint8_t data;
    BCHP_Info info;
#if NEXUS_PLATFORM_97346_I2SFF
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned j;
    NEXUS_3461Settings st3472Settings;
    NEXUS_3461ProbeResults results;
    NEXUS_FrontendDevice3461OpenSettings deviceOpenSettings;
    NEXUS_FrontendDevice3461Settings deviceSettings;
#endif
#if NEXUS_PLATFORM_97346_SV || NEXUS_PLATFORM_97346_HR44 || NEXUS_PLATFORM_7344SV
/* 97346 SV has a shared i2c channel between MOCA and 4506.  This channel cannot be shared, so we cannot initialize the frontend. */
    /* disable NEXUS_MOCA_I2C_CHANNEL in nexus_platform_features.h for 7346 SV board to use external 4505*/
    unsigned j;
    NEXUS_4506Settings settings4506;
#if NEXUS_PLATFORM_97346_HR44
    NEXUS_GpioSettings gpioSettings;
#endif
#endif
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;

    BCHP_GetInfo(g_pCoreHandles->chp, &info);
    if (info.productId == 0x7418){
        return BERR_SUCCESS;
    }

    NEXUS_Frontend_GetDefault7346Settings(&settings);

    /* Setup LNA configurations per-board. */
#if (BCHP_CHIP == 7346) || (BCHP_CHIP == 73465)
#if NEXUS_PLATFORM_97346_SFF || NEXUS_PLATFORM_97346_I2SFF
#ifdef SFF_V10
    settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn1;
    settings.lnaSettings.out1 = NEXUS_7346LnaInput_eIn1;
    settings.lnaSettings.daisy = NEXUS_7346LnaInput_eNone;
#else
    settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.out1 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.daisy = NEXUS_7346LnaInput_eNone;
#endif
#elif NEXUS_PLATFORM_97346_HR44
    settings.isInternalLna = true;
    settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.out1 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.daisy = NEXUS_7346LnaInput_eIn0;
    settings.external3445Lna.enabled = true;
    settings.external3445Lna.i2cChannelNumber = 0; /* SDS0 */
    settings.external3445Lna.lnaOutput = NEXUS_3445LnaOutput_eOut1;
    settings.external3445Lna.settings.daisy = NEXUS_3445LnaInput_eIn2Vga;
    settings.external3445Lna.settings.out1  = NEXUS_3445LnaInput_eIn2Vga;
    settings.external3445Lna.settings.out2  = NEXUS_3445LnaInput_eIn2Vga;
#else
    settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.out1 = NEXUS_7346LnaInput_eIn1;
    settings.lnaSettings.daisy = NEXUS_7346LnaInput_eIn0;
#endif
    settings.isInternalLna = true;
#else /* 7344 SFF and SV board */
    settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.out1 = NEXUS_7346LnaInput_eNone;
    settings.lnaSettings.daisy = NEXUS_7346LnaInput_eIn0;
    settings.isInternalLna = true;
#endif

    /* Switch the frontends. Diseqc is on tuner 1 */
    /* Open 734x Demodulator Channels */
    for ( i = 0; i < NEXUS_7346_MAX_FRONTEND_CHANNELS; i++ )
    {
        BDBG_WRN(("734x Trying to open frontend channel %d", i));
        settings.channelNumber = i;

        pConfig->frontend[i] = NEXUS_Frontend_Open7346(&settings);
        if ( pConfig->frontend[i] )
        {
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            switch (i)
            {
#if (BCHP_CHIP==7346) || (BCHP_CHIP==73465)
            case 0: userParams.param1 = NEXUS_InputBand_e8; break;
            case 1: userParams.param1 = NEXUS_InputBand_e9; break;
            default: BDBG_MSG(("unsupported channel!"));
#else /* 97344 */
                case 0: userParams.param1 = NEXUS_InputBand_e3; break;

                default: BDBG_MSG(("unsupported channel!"));
#endif
                }
            userParams.pParam2 = 0;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        }
        else
        {
            BDBG_MSG(("NEXUS_Frontend_Open7346 Failed!"));
        }
        BDBG_MSG(("pConfig->frontend[%d] = %p", i, (void *)pConfig->frontend[i]));
    }

#if NEXUS_PLATFORM_97346_I2SFF
        if (!pConfig->i2c[2]) {
            BDBG_MSG(("Frontend daughter cannot be initialized without first initializing I2C BSC_M2."));
            return BERR_NOT_INITIALIZED;
        }

        NEXUS_Frontend_GetDefault3461Settings(&st3472Settings);

        NEXUS_FrontendDevice_GetDefault3461OpenSettings(&deviceOpenSettings);
        deviceOpenSettings.i2cDevice = pConfig->i2c[2];
        deviceOpenSettings.i2cAddr = 0x6c;
        deviceOpenSettings.isrNumber = 11;
        deviceOpenSettings.inBandOpenDrain=true;
        deviceOpenSettings.loadAP = true;
        deviceOpenSettings.isMtsif = true;
        deviceOpenSettings.crystalSettings.enableDaisyChain = true;

        rc = NEXUS_Frontend_Probe3461(&deviceOpenSettings, &results);
        if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);
        BDBG_MSG(("chip.familyId = 0x%x", results.chip.familyId));
        BDBG_MSG(("chip.id = 0x%x", results.chip.id));
        BDBG_MSG(("version.major = 0x%x", results.chip.version.major ));
        BDBG_MSG(("version.major = 0x%x", results.chip.version.minor ));

        st3472Settings.device = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
        if (NULL == st3472Settings.device)
        {
            BDBG_ERR(("Unable to open 3472 tuner/demodulator device"));
            rc = BERR_TRACE(BERR_NOT_INITIALIZED); return rc;
        }

        NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
        deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eOff;
        deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
        deviceSettings.enableRfLoopThrough = false;
        deviceSettings.terrestrial = true;
        NEXUS_FrontendDevice_Set3461Settings(st3472Settings.device, &deviceSettings);

        st3472Settings.type = NEXUS_3461ChannelType_eIsdbt;

        for (j=0; j<(results.chip.id & 0xF); j++, i++)
        {
            BDBG_MSG(("Waiting for onboard 3472 tuner/demodulator channel %d to initialize", j));
            st3472Settings.channelNumber = j;
            pConfig->frontend[i] = NEXUS_Frontend_Open3461(&st3472Settings);
            if (NULL == pConfig->frontend[i])
            {
                BDBG_MSG(("Unable to open 3472 tuner/demodulator channel %d", j));
                rc = BERR_TRACE(BERR_NOT_INITIALIZED); return rc;
            }
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.param1 = deviceOpenSettings.isMtsif ? st3472Settings.channelNumber : NEXUS_InputBand_e0+i;
            userParams.isMtsif = deviceOpenSettings.isMtsif;
            userParams.chipId = 0x3472;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
            BDBG_MSG(("pConfig->frontend[%d] = 0x%x", i, pConfig->frontend[i]));
        }
#endif

    /* Swap frontend handles now*/
#if NEXUS_PLATFORM_97346_SFF || NEXUS_PLATFORM_97346_I2SFF
{
    NEXUS_FrontendHandle tempfrontend1,tempfrontend2;
    tempfrontend1 =       pConfig->frontend[0];
    tempfrontend2 =       pConfig->frontend[1];
    pConfig->frontend[0]= tempfrontend2 ;
    pConfig->frontend[1]= tempfrontend1;

}
#endif


#if NEXUS_PLATFORM_97346_SV  || NEXUS_PLATFORM_97346_HR44 || NEXUS_PLATFORM_7344SV
#if NEXUS_PLATFORM_97346_HR44
       NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
       gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
       gpioSettings.value = NEXUS_GpioValue_eHigh;
       gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
       gpioSettings.interrupt.callback = NULL;
       gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 49, &gpioSettings);
       BDBG_ASSERT(NULL != gpioHandle);
       NEXUS_Gpio_Close(gpioHandle);
       gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 65, &gpioSettings);
       BDBG_ASSERT(NULL != gpioHandle);

       /* delay for reset extension */
       BKNI_Sleep(200);
#endif

    /* Open 4505 has only one Demodulator Channel */
#if NEXUS_PLATFORM_97346_HR44
    for ( j = 0; j < NEXUS_4506_MAX_FRONTEND_CHANNELS ; j++, i++ )
    {
        BDBG_WRN(("4506 Trying to open frontend channel %d", i));
        /* Open on-board 4506 */
        NEXUS_Frontend_GetDefault4506Settings(&settings4506);
        settings4506.i2cDevice = pConfig->i2c[2];
        if (!settings4506.i2cDevice) {
            BDBG_MSG(("Unable to initialize I2C"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        settings4506.isrNumber = 17;
        settings4506.i2cAddr = 0x69;
        settings4506.is3445ExternalLna = false;
        settings4506.channelNumber = j;
        pConfig->frontend[i] = NEXUS_Frontend_Open4506(&settings4506);
        if ( pConfig->frontend[i] )
        {
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            switch (j)
            {
                case 0: userParams.param1 = NEXUS_InputBand_e0; break;
                case 1: userParams.param1 = NEXUS_InputBand_e1; break;
                default: BDBG_MSG(("unsupported channel!"));
                }
            userParams.pParam2 = 0;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        }
        else
        {
            BDBG_MSG(("NEXUS_Frontend_Open4506 Failed!"));
        }
    }
#endif

#ifdef NEXUS_MOCA_I2C_CHANNEL
    BDBG_WRN(("Unable to initialize 4505 due to i2c channel shared with MOCA"));
    BDBG_WRN(("remove NEXUS_MOCA_I2C_CHANNEL define from your nexus_platform_features.h"));
    BSTD_UNUSED(j);
    BSTD_UNUSED(settings4506);
#else
    /* Open 4505 has only one Demodulator Channel */
#if NEXUS_PLATFORM_97346_HR44
    for ( j = 0; j < NEXUS_4506_MAX_FRONTEND_CHANNELS ; j++, i++ )
#else
    for ( j = 0; j < NEXUS_4506_MAX_FRONTEND_CHANNELS - 1 ; j++, i++ )
#endif
    {
        BDBG_WRN(("4506 Trying to open frontend channel %d", i));
        /* Open on-board 4506 */
        NEXUS_Frontend_GetDefault4506Settings(&settings4506);
#if NEXUS_PLATFORM_97346_HR44
        settings4506.i2cDevice = pConfig->i2c[3];
#elif (BCHP_CHIP==7346 && BCHP_VER==BCHP_VER_A0) || (NEXUS_PLATFORM_7344SV)
        settings4506.i2cDevice = pConfig->i2c[1];
#else
        settings4506.i2cDevice = pConfig->i2c[2];
#endif
        if (!settings4506.i2cDevice) {
            BDBG_MSG(("Unable to initialize I2C"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        settings4506.isrNumber = 11;
        settings4506.i2cAddr = 0x69;
        /* As of now, only for 4506 daughter card, this need to be set true. Because the 4506 on the daughter card directly controls the external 3445 LNA. */
        settings4506.is3445ExternalLna = false;
        settings4506.channelNumber = j;
        pConfig->frontend[i] = NEXUS_Frontend_Open4506(&settings4506);
        if ( pConfig->frontend[i] )
        {
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            switch (j)
            {
#if  NEXUS_PLATFORM_7344SV
            case 0: userParams.param1 = NEXUS_InputBand_e2; break;
#endif
#if NEXUS_PLATFORM_97346_SV || NEXUS_PLATFORM_97346_HR44
            case 0: userParams.param1 = NEXUS_InputBand_e5; break;
#endif

#if NEXUS_PLATFORM_97346_HR44
                case 1: userParams.param1 = NEXUS_InputBand_e3; break;
#endif
                default: BDBG_MSG(("unsupported channel!"));
                }
            userParams.pParam2 = 0;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        }
        else
        {
            BDBG_MSG(("NEXUS_Frontend_Open external 4506 Failed!"));
        }
    }
#endif
#endif
    /* Tone generated internally according to EXTMpin */
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    /* Dynamic current limit */
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    /* Internal linear regulator is turned on and boost circuit is on */
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
#if BCHP_CHIP==7346
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00,(const uint8_t *) &data, 1);
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *) &data, 1);
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *) &data, 1);
#endif

/* SET THE FPGA for STREAMER by DEFAULT */
    /* 7346 Board only */
#if ((BCHP_CHIP == 7346) || (BCHP_CHIP==73465)) && !defined(NEXUS_PLATFORM_97346_SFF)
    BDBG_MSG(("fpga i2c %d %p", I2C_DEVICE_FPGA_CH , (void *)g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH]));
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xc, &data_c, 1);
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xd, &data_d, 1);
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xe, &data_e, 1);

    BDBG_MSG(("FPGA version:  0x%02x", data_e));

    if ((data_c != 0x46) || (data_d != 0x73)) BDBG_MSG(("Reading of Transport FPGA yields CHIPID=0x%02x%02x", data_d, data_c));

    data = 0x80;
    (void)NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 4, &data, 1);

    if (NEXUS_GetEnv("NEXUS_ASI")) {
        regaddr = 2; /* value maybe 0?*/
        data = 0x10; /* Use streamer 1 for ASI, pkt interface 2 */
    } else {
        regaddr = 2;
        data = 0x00; /* Use streamer 0 for LVDS, pkt interface 0 */
    }
    (void)NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, regaddr, &data, 1);
#endif


    return BERR_SUCCESS;
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
        /* Close extensions first */
        if (pConfig->frontend[i])
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                    handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
           }
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
#if 0 /* Once SWM is in*/
        /* Close underlying demod */
        if ( pConfig->frontend[i] )
        {
            NEXUS_Frontend_Close(pConfig->frontend[i]);
        }
#endif
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }

#if NEXUS_PLATFORM_97346_HR44
    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
#endif
    return;
}
#endif
BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{

    BDBG_ASSERT(pInputBand);
    BTRC_TRACE(ChnChange_TuneStreamer, START);

#if BCHP_CHIP==7344

    switch (index)
    {
        case 0:
            *pInputBand = NEXUS_InputBand_e0;
            break;
        case 1:
            *pInputBand = NEXUS_InputBand_e1;
            break;
        default:
            BDBG_MSG(("index %d not supported", index));
            return NEXUS_NOT_SUPPORTED;
    }

#else
/* SET THE FPGA for STREAMER by DEFAULT */
    /* 7346 Board only */
#if ((BCHP_CHIP == 7346) || (BCHP_CHIP==73465)) && !defined(NEXUS_PLATFORM_97346_SFF)
    {
       uint8_t  data_c, data_d, data_e, regaddr=0;
       uint8_t data;

       BDBG_MSG(("fpga i2c %d %p", I2C_DEVICE_FPGA_CH , (void *)g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH]));
       (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xc, &data_c, 1);
       (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xd, &data_d, 1);
       (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xe, &data_e, 1);
       BDBG_MSG(("FPGA version:  0x%02x", data_e));

       if ((data_c != 0x46) || (data_d != 0x73)) BDBG_MSG(("Reading of Transport FPGA yields CHIPID=0x%02x%02x", data_d, data_c));

       data = 0x80;
       (void)NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 4, &data, 1);

       if (NEXUS_GetEnv("NEXUS_ASI")) {
           regaddr = 2; /* value maybe 0?*/
           data = 0x10; /* Use streamer 1 for ASI, pkt interface 2 */
       } else {
           regaddr = 2;
           data = 0x00; /* Use streamer 0 for LVDS, pkt interface 0 */
       }
       (void)NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, regaddr, &data, 1);
    }
#endif
    BSTD_UNUSED(index);
    *pInputBand = NEXUS_InputBand_e2;

#endif

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

#else
#endif /* NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND */



