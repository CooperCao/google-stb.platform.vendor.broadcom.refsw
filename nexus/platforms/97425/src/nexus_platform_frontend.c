/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_types.h"
#include "nexus_platform.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#include "bchp_gio.h"

#if NEXUS_HAS_FRONTEND
#if NEXUS_PLATFORM_4528_DBS
#include "nexus_spi.h"
#include "nexus_frontend_4528.h"
#endif
#if NEXUS_PLATFORM_4538_DBS
#include "nexus_spi.h"
#include "nexus_frontend_4538.h"
#endif
#if NEXUS_PLATFORM_4517_DBS
#include "nexus_frontend_4517.h"
#endif
#if NEXUS_PLATFORM_7425_DBS || NEXUS_PLATFORM_7422_DBS
#include "nexus_frontend_4506.h"
#endif
#if NEXUS_USE_FRONTEND_3461_DAUGHTER_CARD
#include "nexus_frontend_3461.h"
#endif
#endif
#include "bchp_sun_top_ctrl.h"

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_FRONTEND
#if NEXUS_PLATFORM_7422_CABLE || NEXUS_PLATFORM_7425_CABLE

#if defined(NEXUS_USE_7425_SV_BOARD) || defined(NEXUS_USE_7422_SV_BOARD)
    #include "nexus_frontend_31xx.h"
#else
    #if NEXUS_USE_7425_VMS_SFF || NEXUS_USE_7422_VMS_SFF
        #include "nexus_platform_cable_frontend.h"
        #if defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
            #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
                #include "nexus_docsis.h"
            #else
                #include "nexus_frontend_3255.h"
            #endif
            #include "nexus_frontend_3128.h"
        #elif defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT)
            #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
                #include "nexus_docsis.h"
            #else
                #include "nexus_frontend_3255.h"
            #endif
        #elif!(defined(NEXUS_USE_FRONTEND_3461_DAUGHTER_CARD))
            #include "nexus_frontend_3128.h"
        #endif
        #if !defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT) && defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
            #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
                #include "nexus_docsis.h"
            #else
                #include "nexus_frontend_3255.h"
            #endif
        #endif
    #else
        #include "nexus_frontend_31xx.h"
    #endif
#endif
#endif


#if NEXUS_PLATFORM_7422_CABLE || NEXUS_PLATFORM_7425_CABLE
    #if NEXUS_USE_7425_VMS_SFF || NEXUS_USE_7422_VMS_SFF
        static NEXUS_GpioHandle gpioHandle = NULL;
        static unsigned ltsidCount[NEXUS_MAX_FRONTENDS];
    #if defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT) || defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT) || defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
        #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
        static NEXUS_FrontendDeviceHandle hDocsisDevice = NULL;
        #else
        static NEXUS_3255DeviceHandle st3255DeviceHandle;
        #endif
    #endif
    #else
        #if NEXUS_NUM_FRONTEND_CARD_SLOTS
            static NEXUS_GpioHandle gpioHandle = NULL;
        #endif
        static NEXUS_GpioHandle tunerGpio[NEXUS_MAX_FRONTENDS];
    #endif
#endif
#endif

#if !defined(NEXUS_USE_7425_VMS_SFF) && !defined(NEXUS_USE_7422_VMS_SFF) && !defined(NEXUS_PLATFORM_4528_DBS) && !defined(NEXUS_PLATFORM_4538_DBS) && !defined(NEXUS_PLATFORM_4517_DBS)
NEXUS_Error bcm3405_init( void )
{
    NEXUS_Error rc;
    uint8_t data[]={0x3,0x0,    0x4,0x1a,   0x5,0x0,    0x6,0x4,
                    0x7,0x0,    0x8,0x13,   0x9,0x0,    0xa,0x40,
                    0xb,0x40,   0xc,0xd,    0xe,0xff,   0xf,0xff,
                    0x10,0xff,  0x11,0x77,  0x12,0x63,  0x13,0xa0,
                    0x14,0x9d,  0x15,0xaa,  0x16,0x45,  0x17,0x0,
                    0x18,0x88,  0x19,0xf0,  0x1a,0xf0,  0x1b,0xf0,
                    0x1c,0xf0,  0x1d,0xf0,  0x1e,0xf0,  0x1f,0xf0,
                    0x20,0xf0,  0x21,0xf0,  0x24,0xf0};
    uint8_t i;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;

    for(i=0; i < sizeof(data); i+=2)
    {
        rc = NEXUS_I2c_Write(pConfig->i2c[NEXUS_I2C_CHANNEL_LNA], 0x64, data[i], &data[i+1], 1);
    }
    return rc;
}
#endif

#if NEXUS_PLATFORM_7425_DBS || NEXUS_PLATFORM_7422_DBS
#include "nexus_frontend_4506.h"
#endif

#if NEXUS_HAS_FRONTEND && NEXUS_PLATFORM_4528_DBS
#include "bchp_hif_cpu_intr1.h"
#define ISL9492_CH0_I2C_ADDR 0x08
#define ISL9492_CH1_I2C_ADDR 0x09
static   NEXUS_GpioHandle gpioHandle = NULL;

#if (BCHP_CHIP==7425) && (BCHP_VER>=BCHP_VER_B0)
/* 7425 SATIPSW B1 values */
#define I2C_DEVICE_VOLTAGE_REG_CH 1
#define I2C_4528_INDEX_1 3
#define I2C_4528_INDEX_2 4
#else
/* 7425 SATIPSW A1 values */
#define I2C_DEVICE_VOLTAGE_REG_CH 4
#define I2C_4528_INDEX_1 1
#define I2C_4528_INDEX_2 2
#endif

#define NUM_4528_CHANNELS_PER 8
#define I2C_4528_ADDRESS_1 0x68
#define I2C_4528_ADDRESS_2 0x68
#define EXT_4528_IRQ_1 BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_00_CPU_INTR_SHIFT
#define EXT_4528_IRQ_2 BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_EXT_IRQ_01_CPU_INTR_SHIFT

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_GpioSettings gpioSettings;
    NEXUS_SpiSettings spiSettings;
    NEXUS_SpiHandle mixerPllSpi;
    NEXUS_Error rc;
    uint32_t data;
    NEXUS_4528Settings st4528Settings;
    NEXUS_FrontendUserParameters userParams;

    if ((!pConfig->i2c[I2C_4528_INDEX_1]) && (!pConfig->i2c[I2C_4528_INDEX_2]) ){
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
    gpioSettings.value = NEXUS_GpioValue_eHigh;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
    gpioSettings.interrupt.callback = NULL;
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 75, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);
    NEXUS_Gpio_Close(gpioHandle);
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 76, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandle);

    /* ADF4350 PLL frequency synthesizers use Spi Channel 0 */
    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = false;
    spiSettings.txLeadingCapFalling = false;
    mixerPllSpi = NEXUS_Spi_Open(0, &spiSettings);
    if (!mixerPllSpi)
    {
        BDBG_ERR(("Unable to open SPI For mixer PLL"));
    }
    /* register 5 */
    data = 0x05005800;
    rc = NEXUS_Spi_Write(mixerPllSpi, (uint8_t *)&data, sizeof(data));
    if (rc)
        BDBG_ERR(("NEXUS_Spi_Write reg 5 error"));

    /* register 4 */
    data = 0x6c619100;
    rc = NEXUS_Spi_Write(mixerPllSpi, (uint8_t *)&data, sizeof(data));
    if (rc)
        BDBG_ERR(("NEXUS_Spi_Write reg 4 error"));

    /* register 3 */
    data = 0xb3040000;
    rc = NEXUS_Spi_Write(mixerPllSpi, (uint8_t *)&data, sizeof(data));
    if (rc)
        BDBG_ERR(("NEXUS_Spi_Write reg 3 error"));

    /* register 2 */
    data = 0xc28e0100;
    rc = NEXUS_Spi_Write(mixerPllSpi, (uint8_t *)&data, sizeof(data));
    if (rc)
        BDBG_ERR(("NEXUS_Spi_Write reg 2 error"));

    /* register 1 */
    data = 0x39ad0008;
    rc = NEXUS_Spi_Write(mixerPllSpi, (uint8_t *)&data, sizeof(data));
    if (rc)
        BDBG_ERR(("NEXUS_Spi_Write reg 1 error"));

    /* register 0 */
    data = 0x2826dc02;
    rc = NEXUS_Spi_Write(mixerPllSpi, (uint8_t *)&data, sizeof(data));
    if (rc)
        BDBG_ERR(("NEXUS_Spi_Write reg 0 error"));

    NEXUS_Frontend_GetDefault4528Settings(&st4528Settings);

    st4528Settings.i2cDevice = pConfig->i2c[I2C_4528_INDEX_1];
    st4528Settings.i2cAddr = I2C_4528_ADDRESS_1;
    st4528Settings.isrNumber = EXT_4528_IRQ_1;

    for (i=0; i <  NUM_4528_CHANNELS_PER; i++)
    {
        st4528Settings.channelNumber = i;
        pConfig->frontend[i] = NEXUS_Frontend_Open4528(&st4528Settings);
        if ( NULL == (pConfig->frontend[i]) )
        {
            BDBG_ERR(("Unable to open onboard 4528 tuner/demodulator %d",i));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = true;
        userParams.param1 = userParams.isMtsif ? st4528Settings.channelNumber : NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

    st4528Settings.i2cDevice = pConfig->i2c[I2C_4528_INDEX_2];
    st4528Settings.i2cAddr = I2C_4528_ADDRESS_2;
    st4528Settings.isrNumber = EXT_4528_IRQ_2;

    for (j=0; j < NUM_4528_CHANNELS_PER; j++, i++)
    {
        st4528Settings.channelNumber = j;
        pConfig->frontend[i] = NEXUS_Frontend_Open4528(&st4528Settings);
        if ( NULL == (pConfig->frontend[i]) )
        {
            BDBG_ERR(("Unable to open onboard 4528 tuner/demodulator %d",j));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = true;
        userParams.param1 = userParams.isMtsif ? st4528Settings.channelNumber : NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

    /* Tone generated internally according to EXTMpin */
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);

    /* Dynamic current limit */
    data = 0x40;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    /* Internal linear regulator is turned on and boost circuit is on */
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);

    /* Tone generated internally according to EXTMpin */
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);

    /* Dynamic current limit */
    data = 0x40;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    /* Internal linear regulator is turned on and boost circuit is on */
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);

    return 0;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (pConfig->frontend[i]) {
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
    return;
}
#elif NEXUS_HAS_FRONTEND && NEXUS_PLATFORM_4538_DBS
#include "bchp_hif_cpu_intr1.h"
#define ISL9492_CH0_I2C_ADDR 0x08
#define ISL9492_CH1_I2C_ADDR 0x09
static   NEXUS_GpioHandle gpioHandle = NULL;
/* Uncomment the following define in order to disable the i2c address search */
/*#define NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH 1*/

#define I2C_DEVICE_VOLTAGE_REG_CH 1
#define I2C_4538_INDEX_1 4
#define I2C_4538_INDEX_2 4

#define NUM_4538_CHANNELS_PER 8
#define I2C_4538_ADDRESS_1 0x6a
#define EXT_4538_IRQ_1 16

#include "breg_i2c.h"
#include "priv/nexus_i2c_priv.h"

static bool NEXUS_Platform_P_Is4538(NEXUS_I2cHandle i2cDevice, uint16_t i2cAddr)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint16_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x",i2cHandle,i2cAddr));
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


NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;
    NEXUS_4538Settings st4538Settings;
    NEXUS_FrontendUserParameters userParams;
    uint16_t i2cAddr = I2C_4538_ADDRESS_1;

    if ((!pConfig->i2c[I2C_4538_INDEX_1]) && (!pConfig->i2c[I2C_4538_INDEX_2]) ){
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

    BDBG_MSG(("Checking i2c: 0x%02x",I2C_4538_ADDRESS_1));
    if (!NEXUS_Platform_P_Is4538(pConfig->i2c[I2C_4538_INDEX_1],I2C_4538_ADDRESS_1)) {
#if NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH
        BDBG_ERR(("Unable to locate 4538 at 0x%02x",I2C_4538_ADDRESS_1));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
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
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
#endif
    }

    NEXUS_Frontend_GetDefault4538Settings(&st4538Settings);

    st4538Settings.i2cDevice = pConfig->i2c[I2C_4538_INDEX_1];
    st4538Settings.i2cAddr = i2cAddr;
    st4538Settings.isrNumber = EXT_4538_IRQ_1;
    st4538Settings.gpioInterrupt = NULL;

    for (i=0; i <  NUM_4538_CHANNELS_PER; i++)
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

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (pConfig->frontend[i]) {
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
    return;
}

#elif NEXUS_HAS_FRONTEND && NEXUS_PLATFORM_4517_DBS
#include "bchp_hif_cpu_intr1.h"

static   NEXUS_GpioHandle gpioHandle = NULL;
/* Uncomment the following define in order to disable the i2c address search */
/*#define NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH 1*/

#define I2C_DEVICE_VOLTAGE_REG_CH 1
#define I2C_4517_INDEX_1 4
#define I2C_4517_INDEX_2 4

#define NUM_4517_CHANNELS_PER 3
#define I2C_4517_ADDRESS_1 0x6a
#define EXT_4517_IRQ_1 16

#include "breg_i2c.h"
#include "priv/nexus_i2c_priv.h"

static bool NEXUS_Platform_P_Is4517(NEXUS_I2cHandle i2cDevice, uint16_t i2cAddr)
{
    BREG_I2C_Handle i2cHandle;
    uint8_t buf[5];
    uint16_t chipId=0;
    uint8_t subAddr;

    i2cHandle = NEXUS_I2c_GetRegHandle(i2cDevice, NULL);
    BDBG_MSG(("i2c handle: %p, i2caddr: 0x%x",i2cHandle,i2cAddr));
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

    return chipId == 0x4517;
}


NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;
    NEXUS_4517Settings st4517Settings;
    NEXUS_FrontendUserParameters userParams;
    uint16_t i2cAddr = I2C_4517_ADDRESS_1;

    if ((!pConfig->i2c[I2C_4517_INDEX_1]) && (!pConfig->i2c[I2C_4517_INDEX_2]) ){
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

    BDBG_MSG(("Checking i2c: 0x%02x",I2C_4517_ADDRESS_1));
    if (!NEXUS_Platform_P_Is4517(pConfig->i2c[I2C_4517_INDEX_1],I2C_4517_ADDRESS_1)) {
#if NEXUS_PLATFORM_BYPASS_I2C_ADDRESS_SEARCH
        BDBG_ERR(("Unable to locate 4517 at 0x%02x",I2C_4517_ADDRESS_1));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
#else
        int ix;
        for (ix=0x68; ix<=0x6f; ix++) {
            BDBG_MSG(("Checking i2c: 0x%02x",ix));
            if (ix != I2C_4517_ADDRESS_1 && NEXUS_Platform_P_Is4517(pConfig->i2c[I2C_4517_INDEX_1],ix)) {
                i2cAddr = ix;
                BDBG_MSG(("Found 4517 at 0x%02x",ix));
                break;
            }
        }
        if (i2cAddr == I2C_4517_ADDRESS_1) {
            BDBG_ERR(("Unable to locate 4517"));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
#endif
    }

    NEXUS_Frontend_GetDefault4517Settings(&st4517Settings);

    st4517Settings.i2cDevice = pConfig->i2c[I2C_4517_INDEX_1];
    st4517Settings.i2cAddr = i2cAddr;
    st4517Settings.isrNumber = EXT_4517_IRQ_1;
    st4517Settings.gpioInterrupt = NULL;

    for (i=0; i <  NUM_4517_CHANNELS_PER; i++)
    {
        st4517Settings.channelNumber = i;
        pConfig->frontend[i] = NEXUS_Frontend_Open4517(&st4517Settings);
        if ( NULL == (pConfig->frontend[i]) )
        {
            BDBG_ERR(("Unable to open onboard 4517 tuner/demodulator %d",i));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = true;
        userParams.param1 = userParams.isMtsif ? st4517Settings.channelNumber : NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (pConfig->frontend[i]) {
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
    return;
}


#elif NEXUS_HAS_FRONTEND && (NEXUS_PLATFORM_7422_CABLE || NEXUS_PLATFORM_7425_CABLE)
#if NEXUS_NUM_FRONTEND_CARD_SLOTS
 static NEXUS_FrontendCardHandle g_frontendCards[NEXUS_NUM_FRONTEND_CARD_SLOTS];
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
#if !(defined(NEXUS_USE_FRONTEND_3461_DAUGHTER_CARD))
    unsigned i = 0;
#endif
    NEXUS_Error rc = NEXUS_SUCCESS;
#if NEXUS_NUM_FRONTEND_CARD_SLOTS
    unsigned idx = 0;
#endif

#if defined(NEXUS_USE_7425_SV_BOARD) || defined(NEXUS_USE_7422_SV_BOARD)
    NEXUS_Frontend31xxSettings st31xxSettings;
    NEXUS_FrontendDevice31xxOpenSettings st31xxDeviceOpenSettings;
    NEXUS_FrontendDevice31xxSettings deviceSettings;
    #if NEXUS_NUM_FRONTEND_CARD_SLOTS
        int card=0;
        unsigned numChannels;
        NEXUS_FrontendCardSettings cardSettings;
    #endif
#else
    #if NEXUS_USE_7425_VMS_SFF || NEXUS_USE_7422_VMS_SFF
        #if !(defined(NEXUS_USE_FRONTEND_3461_DAUGHTER_CARD))
            NEXUS_FrontendLTSIDParameters ltsidParams;
        #endif
        unsigned j=0;
        #if defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
            unsigned u3128ch;
            NEXUS_Frontend3128Settings st3128Settings;
            #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
                unsigned docsisChannel;
                NEXUS_DocsisOpenDeviceSettings docsisDeviceSettings;
                NEXUS_DocsisOpenChannelSettings docsisChannelSettings;
                NEXUS_DocsisDeviceCapabilities docsisDeviceCaps;
            #else
                unsigned u3255ch;
                NEXUS_3255DeviceSettings st3255DeviceSettings;
                NEXUS_3255ChannelSettings st3255ChannelSettings;
                NEXUS_3255DeviceCapabilities st3255DeviceCapabilities;
                NEXUS_3255ChannelCapabilities st3255ChannelCapabilities;
            #endif
            BREG_Handle hReg;
            NEXUS_3128ProbeResults results;
            NEXUS_FrontendDevice3128OpenSettings st3128DeviceOpenSettings;
        #elif defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT)
            #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
                unsigned docsisChannel;
                NEXUS_DocsisOpenDeviceSettings docsisDeviceSettings;
                NEXUS_DocsisOpenChannelSettings docsisChannelSettings;
                NEXUS_DocsisDeviceCapabilities docsisDeviceCaps;
            #else
                unsigned u3255ch;
                NEXUS_3255DeviceSettings st3255DeviceSettings;
                NEXUS_3255ChannelSettings st3255ChannelSettings;
                NEXUS_3255DeviceCapabilities st3255DeviceCapabilities;
                NEXUS_3255ChannelCapabilities st3255ChannelCapabilities;
            #endif
        #elif defined(NEXUS_USE_FRONTEND_3461_DAUGHTER_CARD)
            NEXUS_3461ProbeResults results;
            NEXUS_3461Settings st3461Settings;
            NEXUS_FrontendDeviceHandle parentDevice;
            NEXUS_FrontendDevice3461OpenSettings deviceOpenSettings;
            NEXUS_FrontendDevice3461Settings deviceSettings;
            NEXUS_FrontendType type;
        #else
            BREG_Handle hReg;
            unsigned u3128ch;
            NEXUS_3128ProbeResults results;
            NEXUS_Frontend3128Settings st3128Settings;
            NEXUS_FrontendDevice3128OpenSettings st3128DeviceOpenSettings;
            NEXUS_FrontendDevice3128Settings st3128DeviceSettings;
        #endif
        #if !(defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT) || defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)) && defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
            #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
                 unsigned docsisChannel;
                 NEXUS_DocsisOpenDeviceSettings docsisDeviceSettings;
                 NEXUS_DocsisOpenChannelSettings docsisChannelSettings;
                 NEXUS_DocsisDeviceCapabilities docsisDeviceCaps;
             #else
                 unsigned u3255ch;
                 NEXUS_3255DeviceSettings st3255DeviceSettings;
                 NEXUS_3255ChannelSettings st3255ChannelSettings;
                 NEXUS_3255DeviceCapabilities st3255DeviceCapabilities;
                 NEXUS_3255ChannelCapabilities st3255ChannelCapabilities;
             #endif
        #endif
    #else
        NEXUS_Frontend31xxSettings st31xxSettings;
        NEXUS_FrontendDevice31xxOpenSettings st31xxDeviceOpenSettings;
    #endif
#endif /* defined(NEXUS_USE_7425_SV_BOARD) || defined(NEXUS_USE_7422_SV_BOARD) */

    NEXUS_FrontendUserParameters userParams;
#if !defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT) || defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
    NEXUS_GpioSettings tunerGpioSettings;
#endif

#if !defined(NEXUS_USE_7425_VMS_SFF) && !defined(NEXUS_USE_7422_VMS_SFF)
    rc = bcm3405_init();
    if (rc != NEXUS_SUCCESS) {
        return BERR_SUCCESS;
    }
#endif
    if (!pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_0_1_2_3] && !pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_4_5]) {
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

#if defined(NEXUS_USE_7425_VMS_SFF) || defined(NEXUS_USE_7422_VMS_SFF)
    for (j=0; j<NEXUS_MAX_FRONTENDS; j++) {
        ltsidCount[j] = 0;
    }
#if defined NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT
    #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
    NEXUS_Docsis_GetDefaultOpenDeviceSettings(&docsisDeviceSettings);
    docsisDeviceSettings.rpcTimeOut = 50; /* units ms */
    hDocsisDevice = NEXUS_Docsis_OpenDevice(0,&docsisDeviceSettings);
    if(hDocsisDevice)
    {
        NEXUS_Docsis_GetDeviceCapabilities(hDocsisDevice,&docsisDeviceCaps);
        BDBG_MSG(("DOCSIS Capabilities : Total Channels %u QAM Channels %u Docsis Channels %u oob %s",
                  docsisDeviceCaps.totalChannels,docsisDeviceCaps.numQamChannels,
                  docsisDeviceCaps.numDataChannels,docsisDeviceCaps.numOutOfBandChannels?"true":"false" ));
        for (i=0, docsisChannel=0;
             (docsisChannel<(docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels)) && (i<NEXUS_MAX_FRONTENDS-1);
             docsisChannel++)
        {
            NEXUS_Docsis_GetDefaultOpenChannelSettings(&docsisChannelSettings);
            if(docsisChannel >=docsisDeviceCaps.numDataChannels)
            {
                docsisChannelSettings.autoAcquire = true;
                docsisChannelSettings.channelNum = docsisChannel;
                docsisChannelSettings.channelType = NEXUS_DocsisChannelType_eQam;
                docsisChannelSettings.fastAcquire = true;
                BDBG_MSG((" frontend index %u Docsis QAM channel %u", i, docsisChannel));
            }
            else
            {
                BDBG_MSG(("Docsis data channel %u",docsisChannel));
                continue;
            }
            pConfig->frontend[i] = NEXUS_Docsis_OpenChannel(hDocsisDevice,&docsisChannelSettings);
            if (!pConfig->frontend[i])
            {
                BDBG_ERR(("Unable to open docsis channel frontendIndex %u channel %u",i,docsisChannel));
                continue;
            }
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.param1 = docsisDeviceCaps.isMtsif ? docsisChannel  : NEXUS_InputBand_e0+docsisChannel;
            userParams.isMtsif = docsisDeviceCaps.isMtsif;
            userParams.chipId = 0x3384;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
            NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidParams.ltsidNum = userParams.param1;
            ltsidParams.chipId = 0x3384;
            ltsidParams.mtsifNum = 1;
            ltsidParams.mtsifEnabled = true;
            NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidCount[ltsidParams.ltsidNum] += 1;
            i++;
        }
    }
    else
    {
        BDBG_ERR(("NEXUS_Docsis_OpenDevice failed"));
    }
    #else
    NEXUS_Frontend_GetDefault3255DeviceSettings(&st3255DeviceSettings);
    st3255DeviceSettings.rpcTimeout = 50;
    st3255DeviceSettings.mtsif = true;
    st3255DeviceHandle = NEXUS_Frontend_Open3255Device(0,&st3255DeviceSettings);
    NEXUS_Frontend_Get3255DeviceCapabilities(st3255DeviceHandle,&st3255DeviceCapabilities);
    BDBG_MSG(("DOCSIS Capabilities : Total Channels %u QAM Channels %u Docsis Channels %u oob %s",
              st3255DeviceCapabilities.totalChannels,st3255DeviceCapabilities.numOfQamChannels,
              st3255DeviceCapabilities.numOfDocsisChannels,st3255DeviceCapabilities.isOobChannelSupported?"true":"false" ));
    /*
     * Open the DOCSIS Inband channels
     */
    for (i=0, u3255ch=0;
         (u3255ch<st3255DeviceCapabilities.totalChannels) && (i<NEXUS_MAX_FRONTENDS-1);
         u3255ch++)
    {
        NEXUS_Frontend_Get3255ChannelCapabilities(st3255DeviceHandle, u3255ch, &st3255ChannelCapabilities);
        if(st3255ChannelCapabilities.channelType == NEXUS_3255ChannelType_eInBand)
        {
            BDBG_MSG((" frontend index %u Docsis QAM channel %u", i, u3255ch));
            NEXUS_Frontend_GetDefault3255ChannelSettings(&st3255ChannelSettings);
            st3255ChannelSettings.channelNumber = u3255ch;
            pConfig->frontend[i] = NEXUS_Frontend_Open3255Channel(st3255DeviceHandle,&st3255ChannelSettings);
            if ( NULL == (pConfig->frontend[i]) )
            {
                BDBG_ERR(("Unable to open onboard 3255 tuner/demodulator %d", i));
                continue;
            }
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.param1 = st3255DeviceSettings.mtsif ? st3255ChannelSettings.channelNumber : NEXUS_InputBand_e0+u3255ch;
            userParams.isMtsif = st3255DeviceSettings.mtsif;
            userParams.chipId = 0x3255; /* 3255 API for BCM3383*/
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

            NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidParams.ltsidNum = userParams.param1;
            ltsidParams.chipId = 0x3255; /* 3255 API for BCM3383*/
            ltsidParams.mtsifNum = 1;
            ltsidParams.mtsifEnabled = true;
            NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidCount[ltsidParams.ltsidNum] += 1;

            i++;
        }
        else
        {
            BDBG_MSG(("Docsis Non-QAM channel %u", u3255ch));
        }
    }
    #endif
    /* GPIO 28 is used instead of EXT_IRQ. */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);

    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;

#if NEXUS_NUM_FRONTEND_CARD_SLOTS
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,82, &tunerGpioSettings);
#else
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,28, &tunerGpioSettings);
#endif
    if (NULL == gpioHandle)
    {
      BDBG_ERR(("Unable to open GPIO for tuner %d", i));
      return BERR_NOT_INITIALIZED;
    }

    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_DSTRM_TUNER];
    st3128DeviceOpenSettings.i2cAddr = 0x6c;
    st3128DeviceOpenSettings.isrNumber = 0;
    st3128DeviceOpenSettings.gpioInterrupt = gpioHandle;
    st3128DeviceOpenSettings.inBandOpenDrain=true;
    st3128DeviceOpenSettings.loadAP = true;
    st3128DeviceOpenSettings.isMtsif = true;

    hReg = g_pCoreHandles->reg;

    st3128DeviceOpenSettings.pinmux.data[0] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8;
    st3128DeviceOpenSettings.pinmux.data[1] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8) | 0x200000);

    st3128DeviceOpenSettings.pinmux.data[2] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12;
    st3128DeviceOpenSettings.pinmux.data[3] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12) | 0x2000000);

    rc = NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);

    st3128Settings.device = NEXUS_FrontendDevice_Open3128(0, &st3128DeviceOpenSettings);
    if(st3128Settings.device == NULL){
        BDBG_ERR(("Unable to open 3128 device."));
        rc = BERR_TRACE(rc); return BERR_NOT_INITIALIZED;
    }

    st3128Settings.loadAP = true;
    st3128Settings.type = NEXUS_3128ChannelType_eInBand;
    st3128Settings.isMtsif = true;

    /*
    * Open the BCM3128 InBand channels
    */
    for(u3128ch=0; ((i<NEXUS_MAX_FRONTENDS-1) && (u3128ch<(results.chip.id & 0xF))); u3128ch++)
    {
        BDBG_MSG((" frontend index %u BCM3128 QAM channel %u", i, u3128ch));
        st3128Settings.channelNumber = u3128ch;
        pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator channel %d", i));
            continue;
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0+u3128ch;
        userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
        userParams.chipId = 0x3128;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3128;
        ltsidParams.mtsifNum = 0;
        ltsidParams.mtsifEnabled = true;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;

        i++;
    }

#elif defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT)
    #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
    NEXUS_Docsis_GetDefaultOpenDeviceSettings(&docsisDeviceSettings);
    docsisDeviceSettings.rpcTimeOut = 50; /* units ms */
    hDocsisDevice = NEXUS_Docsis_OpenDevice(0,&docsisDeviceSettings);
    if(hDocsisDevice)
    {
        NEXUS_Docsis_GetDeviceCapabilities(hDocsisDevice,&docsisDeviceCaps);
        BDBG_MSG(("DOCSIS Capabilities : Total Channels %u QAM Channels %u Docsis Channels %u oob %s",
                  docsisDeviceCaps.totalChannels,docsisDeviceCaps.numQamChannels,
                  docsisDeviceCaps.numDataChannels,docsisDeviceCaps.numOutOfBandChannels?"true":"false" ));
        for (i=0, docsisChannel=0;
             (docsisChannel<(docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels)) && (i<NEXUS_MAX_FRONTENDS-1);
             docsisChannel++)
        {
            NEXUS_Docsis_GetDefaultOpenChannelSettings(&docsisChannelSettings);
            if(docsisChannel >= docsisDeviceCaps.numDataChannels)
            {
                docsisChannelSettings.autoAcquire = true;
                docsisChannelSettings.channelNum = docsisChannel;
                docsisChannelSettings.channelType = NEXUS_DocsisChannelType_eQam;
                docsisChannelSettings.fastAcquire = true;
                BDBG_MSG((" frontend index %u Docsis QAM channel %u", i, docsisChannel));
            }
            else
            {
                BDBG_MSG(("Docsis data channel %u",docsisChannel));
                continue;
            }
            pConfig->frontend[i] = NEXUS_Docsis_OpenChannel(hDocsisDevice,&docsisChannelSettings);
            if (!pConfig->frontend[i])
            {
                BDBG_ERR(("Unable to open docsis channel frontendIndex %u channel %u",i,docsisChannel));
                continue;
            }
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.param1 = docsisDeviceCaps.isMtsif ? docsisChannel : NEXUS_InputBand_e0+docsisChannel;
            userParams.isMtsif = docsisDeviceCaps.isMtsif;
            userParams.chipId = 0x3384;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
            NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidParams.ltsidNum = userParams.param1;
            ltsidParams.chipId = 0x3384;
            ltsidParams.mtsifNum = 1;
            ltsidParams.mtsifEnabled = true;
            NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidCount[ltsidParams.ltsidNum] += 1;
            i++;
        }
     }
     else
     {
         BDBG_ERR(("NEXUS_Docsis_OpenDevice failed"));
     }
    #else
    NEXUS_Frontend_GetDefault3255DeviceSettings(&st3255DeviceSettings);
    st3255DeviceSettings.rpcTimeout = 50;
    st3255DeviceSettings.mtsif = true;
    st3255DeviceHandle = NEXUS_Frontend_Open3255Device(0,&st3255DeviceSettings);
    NEXUS_Frontend_Get3255DeviceCapabilities(st3255DeviceHandle,&st3255DeviceCapabilities);
    BDBG_MSG(("DOCSIS Capabilities : Total Channels %u QAM Channels %u Docsis Channels %u oob %s",
              st3255DeviceCapabilities.totalChannels,st3255DeviceCapabilities.numOfQamChannels,
              st3255DeviceCapabilities.numOfDocsisChannels,st3255DeviceCapabilities.isOobChannelSupported?"true":"false" ));
    /*
     * Open the DOCSIS Inband channels
     */
    for (i=0, u3255ch=0;
         (u3255ch < st3255DeviceCapabilities.totalChannels) && (i < NEXUS_MAX_FRONTENDS-1);
          u3255ch++)
    {
        NEXUS_Frontend_Get3255ChannelCapabilities(st3255DeviceHandle, u3255ch, &st3255ChannelCapabilities);
        if (st3255ChannelCapabilities.channelType == NEXUS_3255ChannelType_eInBand)
        {
            BDBG_MSG((" frontend index %u Docsis QAM channel %u", i, u3255ch));
            NEXUS_Frontend_GetDefault3255ChannelSettings(&st3255ChannelSettings);
            st3255ChannelSettings.channelNumber = u3255ch;
            pConfig->frontend[i] = NEXUS_Frontend_Open3255Channel(st3255DeviceHandle, &st3255ChannelSettings);
            if ( NULL == (pConfig->frontend[i]) )
            {
                BDBG_ERR(("Unable to open onboard 3255 tuner/demodulator %d",i));
                continue;
            }
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            userParams.param1 = st3255DeviceSettings.mtsif ? st3255ChannelSettings.channelNumber : NEXUS_InputBand_e0+i;
            userParams.isMtsif = st3255DeviceSettings.mtsif;
            userParams.chipId = 0x3255; /* 3255 API for BCM3383*/
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

            NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidParams.ltsidNum = userParams.param1;
            ltsidParams.chipId = 0x3255; /* 3255 API for BCM3383*/
            ltsidParams.mtsifNum = 1;
            ltsidParams.mtsifEnabled = true;
            NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
            ltsidCount[ltsidParams.ltsidNum] += 1;

            i++;
        }
        else
        {
            BDBG_MSG(("Docsis Non-QAM channel %u", u3255ch));
        }
    }
    #endif
#elif defined(NEXUS_USE_FRONTEND_3461_DAUGHTER_CARD)
    BDBG_WRN(("Waiting for 3461 Downstream frontend(7231_EUSFF) to initialize"));
    /* GPIO 85 is used instead of GPIO_IRQ. */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);

    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    /* A hardware/board modification is required to connect the right gpio for the interrupt. */
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;

    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,85, &tunerGpioSettings);
    if (NULL == gpioHandle)
    {
      BDBG_ERR(("Unable to open GPIO for tuner."));
      return BERR_NOT_INITIALIZED;
    }

    if (!pConfig->i2c[3]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            return BERR_NOT_INITIALIZED;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[3];    /* Onboard tuner/demod use BSC_M3.*/
    deviceOpenSettings.i2cAddr = 0x6c;
    deviceOpenSettings.gpioInterrupt = gpioHandle;
    deviceOpenSettings.isrNumber= 0;
    deviceOpenSettings.loadAP = true;

    deviceOpenSettings.externalFixedGain.total = 8;       /* These are platform specific values given by the board designer. */
    deviceOpenSettings.externalFixedGain.bypassable = 14; /* These are platform specific values given by the board designer. */
    deviceOpenSettings.crystalSettings.enableDaisyChain = true;


    if(NEXUS_Frontend_Probe3461(&deviceOpenSettings, &results) != NEXUS_SUCCESS){
        BDBG_ERR(("3461 tuner not found"));
        rc = BERR_NOT_INITIALIZED; goto done;
    }
    BDBG_ERR(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_ERR(("chip.id = 0x%x", results.chip.id));
    BDBG_ERR(("version.major = 0x%x", results.chip.version.major ));
    BDBG_ERR(("version.minor = 0x%x", results.chip.version.minor ));


    parentDevice = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if (NULL == parentDevice)
    {
        BDBG_ERR(("Unable to open first 3461 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }
    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eInternalLna;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = false;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(parentDevice, &deviceSettings);

    NEXUS_Frontend_GetDefault3461Settings(&st3461Settings);
    st3461Settings.device = parentDevice;
    st3461Settings.type = NEXUS_3461ChannelType_eDvbt; /*REDUNDANT for now as there is only one instance of any demod running. */
    st3461Settings.channelNumber = 0;                    /*REDUNDANT for now. */

    pConfig->frontend[0] = NEXUS_Frontend_Open3461(&st3461Settings);
    if (NULL == pConfig->frontend[0])
    {
        BDBG_ERR(("Unable to open first 3461 dvbt2 tuner/demodulator channel."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetType(pConfig->frontend[0], &type);
    BDBG_ERR(("familyId = 0x%x", type.chip.familyId));
    BDBG_ERR(("chipId = 0x%x", type.chip.id));
    BDBG_ERR(("version.major = 0x%x", type.chip.version.major ));
    BDBG_ERR(("version.major = 0x%x", type.chip.version.minor ));
    BDBG_ERR(("version.buildType = 0x%x", type.chip.version.buildType ));
    BDBG_ERR(("version.buildId = 0x%x", type.chip.version.buildId ));
    BDBG_ERR(("bondoutOptions[0] = 0x%x", type.chip.bondoutOptions[0] ));
    BDBG_ERR(("bondoutOptions[1] = 0x%x", type.chip.bondoutOptions[1] ));

    BDBG_ERR(("firmwareVersion.major = 0x%x", type.firmwareVersion.major ));
    BDBG_ERR(("firmwareVersion.major = 0x%x", type.firmwareVersion.minor ));
    BDBG_ERR(("firmwareVersion.buildType = 0x%x", type.firmwareVersion.buildType ));
    BDBG_ERR(("firmwareVersion.buildId = 0x%x", type.firmwareVersion.buildId ));

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    /*userParams.isMtsif = false;*/
    userParams.param1 = NEXUS_InputBand_e0;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);
done:

#else
    /* GPIO 28 is used instead of EXT_IRQ. */
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);

    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;

#if NEXUS_NUM_FRONTEND_CARD_SLOTS
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,82, &tunerGpioSettings);
#else
    gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,28, &tunerGpioSettings);
#endif
    if (NULL == gpioHandle)
    {
      BDBG_ERR(("Unable to open GPIO for tuner %d", i));
      return BERR_NOT_INITIALIZED;
    }

    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_DSTRM_TUNER];
    st3128DeviceOpenSettings.i2cAddr = 0x6c;
    st3128DeviceOpenSettings.isrNumber = 0;
    st3128DeviceOpenSettings.gpioInterrupt = gpioHandle;
    st3128DeviceOpenSettings.inBandOpenDrain=true;
    st3128DeviceOpenSettings.loadAP = true;
    st3128DeviceOpenSettings.isMtsif = true;
    st3128DeviceOpenSettings.crystalSettings.enableDaisyChain = false;
    st3128DeviceOpenSettings.pinmux.enabled = true;

    hReg = g_pCoreHandles->reg;

    st3128DeviceOpenSettings.pinmux.data[0] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8;
    st3128DeviceOpenSettings.pinmux.data[1] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8) | 0x200000);

    st3128DeviceOpenSettings.pinmux.data[2] = BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12;
    st3128DeviceOpenSettings.pinmux.data[3] = (BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12) | 0x2000000);

    st3128DeviceOpenSettings.outOfBand.ifFrequency = 0;
    st3128Settings.outOfBand.openDrain = true;
    st3128DeviceSettings.outOfBand.outputMode = NEXUS_FrontendOutOfBandOutputMode_eFec;

    rc = NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);

    st3128Settings.device = NEXUS_FrontendDevice_Open3128(0, &st3128DeviceOpenSettings);
    if(st3128Settings.device == NULL){
        BDBG_ERR(("Unable to open 3128 device."));
        rc = BERR_TRACE(rc); return BERR_NOT_INITIALIZED;
    }

    st3128Settings.type = NEXUS_3128ChannelType_eInBand;
    st3128Settings.isMtsif = true;
    /* NEXUS_MAX_FRONTENDS=9; BCM3128 has 8 InBand Channels and 1 OOB channel
    * Open the BCM3128 InBand channels
    */
    for (i=0, u3128ch=0; u3128ch<(results.chip.id & 0xF); u3128ch++)
    {
        BDBG_WRN(("Waiting for onboard 3128 tuner/demodulator channel %d to initialize", u3128ch));
        st3128Settings.channelNumber = u3128ch;
        pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator channel %d", i));
            continue;
        }

        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0+i;
        userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
        userParams.chipId = 0x3128;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3128;
        ltsidParams.mtsifNum = 0;
        ltsidParams.mtsifEnabled = true;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;
        i++;
    }
#endif /* defined NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT */

#if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT

    #if !(defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT) || defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)) && defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
        #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
        NEXUS_Docsis_GetDefaultOpenDeviceSettings(&docsisDeviceSettings);
        docsisDeviceSettings.rpcTimeOut = 50; /* units ms */
        hDocsisDevice = NEXUS_Docsis_OpenDevice(0,&docsisDeviceSettings);
        if(hDocsisDevice)
        {
            NEXUS_Docsis_GetDeviceCapabilities(hDocsisDevice,&docsisDeviceCaps);
            BDBG_MSG(("DOCSIS Capabilities : Total Channels %u QAM Channels %u Docsis Channels %u oob %s",
                  docsisDeviceCaps.totalChannels,docsisDeviceCaps.numQamChannels,
                  docsisDeviceCaps.numDataChannels,docsisDeviceCaps.numOutOfBandChannels?"true":"false" ));
        }
        else
        {
            BDBG_ERR(("NEXUS_Docsis_OpenDevice failed"));
        }
        #else
        NEXUS_Frontend_GetDefault3255DeviceSettings(&st3255DeviceSettings);
        st3255DeviceSettings.rpcTimeout = 50;
        st3255DeviceHandle = NEXUS_Frontend_Open3255Device(0,&st3255DeviceSettings);
        NEXUS_Frontend_Get3255DeviceCapabilities(st3255DeviceHandle,&st3255DeviceCapabilities);
        #endif
    #endif
    /*
     * If OOB channel is present in the Docsis device, check for the channel number
     */
    #if defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
    if(hDocsisDevice && docsisDeviceCaps.numOutOfBandChannels)
    {
        NEXUS_Docsis_GetDefaultOpenChannelSettings(&docsisChannelSettings);
        docsisChannelSettings.channelType=NEXUS_DocsisChannelType_eOutOfBand;
        docsisChannelSettings.channelNum = docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels;
        pConfig->frontend[i] = NEXUS_Docsis_OpenChannel(hDocsisDevice,&docsisChannelSettings);
        if(!pConfig->frontend[i])
        {
            BDBG_ERR(("DOCSIS OOB channel open failed"));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = docsisDeviceCaps.isMtsif ? docsisChannelSettings.channelNum : NEXUS_InputBand_e0+i;
        userParams.isMtsif = docsisDeviceCaps.isMtsif;
        userParams.chipId = 0x3384;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3384;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;
        i++;
    }
    if(hDocsisDevice && docsisDeviceCaps.numUpStreamChannels)
    {
        NEXUS_Docsis_GetDefaultOpenChannelSettings(&docsisChannelSettings);
        docsisChannelSettings.channelType = NEXUS_DocsisChannelType_eUpstream;
        docsisChannelSettings.channelNum = docsisDeviceCaps.numDataChannels + docsisDeviceCaps.numQamChannels +
                                           docsisDeviceCaps.numOutOfBandChannels;
        pConfig->frontend[i] = NEXUS_Docsis_OpenChannel(hDocsisDevice,&docsisChannelSettings);
        if(!pConfig->frontend[i])
        {
            BDBG_ERR(("DOCSIS OOB channel open failed"));
        }
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = docsisDeviceCaps.isMtsif ? docsisChannelSettings.channelNum : NEXUS_InputBand_e0+i;
        userParams.isMtsif = docsisDeviceCaps.isMtsif;
        userParams.chipId = 0x3384;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3384;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;
    }
    #else
    u3255ch=0;
    if(st3255DeviceCapabilities.isOobChannelSupported)
    {
        for(u3255ch=0; u3255ch<st3255DeviceCapabilities.totalChannels; u3255ch++)
        {
            NEXUS_Frontend_Get3255ChannelCapabilities(st3255DeviceHandle, u3255ch, &st3255ChannelCapabilities);
            if(st3255ChannelCapabilities.channelType == NEXUS_3255ChannelType_eOutOfBand)
            {
                BDBG_MSG(("Found Docsis OOB channel index %u", u3255ch));
                break;
            }
        }
    }
    if (NEXUS_StrCmp(NEXUS_GetEnv("disable_oob_frontend"), "y") != 0)
    {
        BDBG_MSG(("Opening onboard 3255 OOB %u", i));
        NEXUS_Frontend_GetDefault3255ChannelSettings(&st3255ChannelSettings);
        st3255ChannelSettings.channelNumber = u3255ch;
        pConfig->frontend[i] = NEXUS_Frontend_Open3255Channel(st3255DeviceHandle, &st3255ChannelSettings);
        if ( NULL == (pConfig->frontend[i]) )
        {
            BDBG_ERR(("Unable to open 3255 tuner/demodulator OOB %d", i));
        }

        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = st3255DeviceSettings.mtsif ? st3255ChannelSettings.channelNumber : NEXUS_InputBand_e0+i;
        userParams.isMtsif = st3255DeviceSettings.mtsif;
        userParams.chipId = 0x3255;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

        NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidParams.ltsidNum = userParams.param1;
        ltsidParams.chipId = 0x3255;
        NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
        ltsidCount[ltsidParams.ltsidNum] += 1;
    }
    else
    {
        BDBG_MSG(("env - disable_oob_frontend set"));
    }
    #endif
#elif !(defined(NEXUS_USE_FRONTEND_3461_DAUGHTER_CARD))
    /*Open the BCM3128 OOB channel */
    st3128Settings.type = NEXUS_3128ChannelType_eOutOfBand;
    BDBG_WRN(("Waiting for onboard 3128 Oob channel %d to initialize", u3128ch));
    st3128Settings.channelNumber = u3128ch;
    pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
    if (NULL == pConfig->frontend[i])
    {
        BDBG_ERR(("Unable to open onboard 3128 Oob channel %d", i));
    }

    NEXUS_Frontend_GetDefault3128ConfigSettings(&st3128DeviceSettings);

#ifdef BCM3128_OOB_CABLECARD_SUPPORT
    st3128DeviceSettings.outOfBand.outputMode = NEXUS_FrontendOutOfBandOutputMode_eDifferentialDecoder;
#else
    st3128DeviceSettings.outOfBand.outputMode = NEXUS_FrontendOutOfBandOutputMode_eFec;
#endif

    rc = NEXUS_Frontend_3128_SetConfigSettings(pConfig->frontend[i], &st3128DeviceSettings);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
    userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0+i;
    userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);

    NEXUS_Frontend_GetLTSID(pConfig->frontend[i], &ltsidParams);
    ltsidParams.ltsidNum = userParams.param1;
    ltsidParams.chipId = 0x3128;
    NEXUS_Frontend_SetLTSID(pConfig->frontend[i], &ltsidParams);
    ltsidCount[ltsidParams.ltsidNum] += 1;
#endif /* NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT */

    /* TODO: this is completely bogus and relied on SetLTSID() overriding userParams, which it is not allowed to do.
       if you mess with userParams.param1, you will get no data routed */
    #if 0
    /* Check for multiple configurations on the same LTSID number */
    for (j=0; j<NEXUS_MAX_FRONTENDS; j++) {
        if (ltsidCount[j] > 1) {
            BDBG_ERR(("****************************************************"));
            BDBG_ERR(("ERROR: You have configured multiple frontends on LTSID number: %d", j));
            BDBG_ERR(("****************************************************"));
        }
    }
    #endif

#else /* defined(NEXUS_USE_7425_VMS_SFF) || defined(NEXUS_USE_7422_VMS_SFF) */

     /* Probe Daughercards First */
#if NEXUS_NUM_FRONTEND_CARD_SLOTS
     if (!pConfig->i2c[NEXUS_I2C_CHANNEL_DSTRM_TUNER]) {
         BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
         return BERR_NOT_INITIALIZED;
     }

     /* Find first empty frontend in config */
     for ( ; pConfig->frontend[idx] && idx < NEXUS_MAX_FRONTENDS; idx++ );
     if ( idx >= NEXUS_MAX_FRONTENDS )
     {
         return BERR_SUCCESS;
     }

     /* Probe first slot */
     NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);
     tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
     tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eFallingEdge;
     gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,82, &tunerGpioSettings);
     if (NULL == gpioHandle)
     {
         BDBG_ERR(("Unable to open GPIO for tuner %d", i));
     }

     rc = NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
     if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);


     NEXUS_FrontendCard_GetDefaultSettings(&cardSettings);
     cardSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_DSTRM_TUNER];
     cardSettings.isrNumber = 0;                             /* This slot  uses GPIO interrupt 50 */
     cardSettings.numChannels = (results.chip.id & 0xF); /* 3128 card has 8 downstream channels and 3124 has 4*/
     cardSettings.numOutOfBandChannels = 0;   /* for now till the oob support is provided in the PI*/
                                                             /* First slot has 1 out of band channel */
     cardSettings.gpioInterrupt = gpioHandle;                /* This slot uses GPIO for interrupt */

     g_frontendCards[card] = NEXUS_FrontendCard_Open(&cardSettings);
     if ( g_frontendCards[card] ) {
         BDBG_WRN(("Found tuner card in slot 0"));
         NEXUS_FrontendCard_GetNumChannels(g_frontendCards[card], &numChannels);
         for ( i=0; i < numChannels && idx < NEXUS_MAX_FRONTENDS; idx++, i++ )
         {
             pConfig->frontend[idx] = NEXUS_FrontendCard_GetChannel(g_frontendCards[card], i);
             NEXUS_Frontend_GetUserParameters(pConfig->frontend[idx], &userParams);
             userParams.param1 = NEXUS_InputBand_e0;
             userParams.pParam2 = 0;
             NEXUS_Frontend_SetUserParameters(pConfig->frontend[idx], &userParams);
         }
         card++;
     }
     else {
         BDBG_WRN(("Unable to find tuner card in slot 0"));
     }
#else /* NEXUS_NUM_FRONTEND_CARD_SLOTS */

    /* Old VMS and SV platforms have totally six tuners
    * Open the 31xx tuners' Inband channels */

    NEXUS_Frontend_GetDefault31xxSettings(&st31xxSettings);

    st31xxSettings.type = NEXUS_31xxChannelType_eInBand;
    st31xxSettings.channelNumber = 0;

    NEXUS_FrontendDevice_GetDefault31xxOpenSettings(&st31xxDeviceOpenSettings);

    st31xxDeviceOpenSettings.outOfBand.ifFrequency = 0;
    st31xxDeviceOpenSettings.inBandOpenDrain=true;
    st31xxDeviceOpenSettings.loadAP = true;
    st31xxDeviceOpenSettings.configureWatchdog = false;

    for (i=0; i<NEXUS_MAX_FRONTENDS-1-2; i++)
    {
        BDBG_WRN(("Waiting for onboard frontend %d to initialize", i));
        NEXUS_FrontendDevice_GetDefault31xxSettings(&deviceSettings);

        /* GPIO's are used instead of EXT_IRQ's
        * GPIO's have UPG_MAIN as their L1
        * GPIO's 19, 20, 26, 27, 62, and 63 are used
        * TODO:  Program GPIO settings and ensure Nexus frontend has
        *        support for GPIO interrupts
        */
        NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);
        tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
        tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eFallingEdge;

        switch(i) {
        case 0:
            tunerGpio[i] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,19, &tunerGpioSettings);
            st31xxDeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_0_1_2_3];
            st31xxDeviceOpenSettings.i2cAddr = 0x66;
            deviceSettings.enableDaisyChain = true;
            break;
        case 1:
            tunerGpio[i] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,20, &tunerGpioSettings);
            st31xxDeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_0_1_2_3];
            st31xxDeviceOpenSettings.i2cAddr = 0x67;
            deviceSettings.enableDaisyChain = true;
            break;
        case 2:
#if NEXUS_USE_7422_SV_BOARD || NEXUS_USE_7425_SV_BOARD
            tunerGpio[i] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,26, &tunerGpioSettings);
#else
            tunerGpio[i] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,24, &tunerGpioSettings);
#endif
            st31xxDeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_0_1_2_3];
            st31xxDeviceOpenSettings.i2cAddr = 0x68;
            deviceSettings.enableDaisyChain = true;
            break;
        case 3:
            tunerGpio[i] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,27, &tunerGpioSettings);
            st31xxDeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_0_1_2_3];
            st31xxDeviceOpenSettings.i2cAddr = 0x69;
            break;
        /* coverity[dead_error_condition] */
        case 4:
            tunerGpio[i] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,62, &tunerGpioSettings);
            st31xxDeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_4_5];
            st31xxDeviceOpenSettings.i2cAddr = 0x66;
            deviceSettings.enableDaisyChain = true;
            break;
        /* coverity[dead_error_condition] */
        case 5:
            tunerGpio[i] = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard,63, &tunerGpioSettings);
            st31xxDeviceOpenSettings.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_TUNERS_4_5];
            st31xxDeviceOpenSettings.i2cAddr = 0x67;
            break;
        }
        if (NULL == tunerGpio[i])
        {
            BDBG_ERR(("Unable to open GPIO for tuner %d", i));
            return BERR_TRACE(BERR_NOT_INITIALIZED);
        }

        st31xxDeviceOpenSettings.gpioInterrupt = tunerGpio[i];

        st31xxSettings.device = NEXUS_FrontendDevice_Open31xx(0, &st31xxDeviceOpenSettings);
        if (NULL == st31xxSettings.device)
        {
            BDBG_ERR(("Unable to open onboard 31xx tuner/demodulator device"));
            return BERR_TRACE(BERR_NOT_INITIALIZED);
        }

        NEXUS_FrontendDevice_Set31xxSettings(st31xxSettings.device, &deviceSettings);

        pConfig->frontend[i] = NEXUS_Frontend_Open31xx(&st31xxSettings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_ERR(("Unable to open onboard 3112 tuner/demodulator %d", i));
            continue;
        }

        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.param1 = NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }
#endif /* NEXUS_NUM_FRONTEND_CARD_SLOTS */
#endif /* defined(NEXUS_USE_7425_VMS_SFF) || defined(NEXUS_USE_7422_VMS_SFF)*/

    #if defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT) && defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
    /*
     * On platforms with DOCSIS and BCM3128 enabled, LNA device is shared between BCM3128 and DOCSIS,
     * but LNA device is controlled by DOCSIS. This linking would be used for extracting the
     * AGC val from DOCSIS device by the BCM3128 private APIs to program the AGC value into
     * BCM3128 device.
     */
    NEXUS_FrontendDevice_Link(hDocsisDevice,st3128Settings.device, NULL);
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
            BDBG_MSG(("NEXUS_Platform_UninitFrontend frontend %u",i));
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

#if defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT) || defined(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT) || defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    #if !defined(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT)
    if(st3255DeviceHandle)
    {
        NEXUS_Frontend_Close3255Device(st3255DeviceHandle);
    }
    #endif
#endif
#if defined NEXUS_USE_7425_VMS_SFF || defined NEXUS_USE_7422_VMS_SFF
    if(gpioHandle)
    {
        NEXUS_Gpio_Close(gpioHandle);
        gpioHandle = NULL;
    }
#else
    for (i=0; i<NEXUS_MAX_FRONTENDS-3; i++)
    {
        if(tunerGpio[i])
        {
            NEXUS_Gpio_Close(tunerGpio[i]);
            tunerGpio[i] = NULL;
        }
    }
#endif
    return;
}
#elif NEXUS_HAS_FRONTEND && (NEXUS_PLATFORM_7425_DBS || NEXUS_PLATFORM_7422_DBS)
    static NEXUS_FrontendHandle g_onboard4506[NEXUS_MAX_FRONTENDS] = {NULL};

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    NEXUS_4506Settings settings4506;
    NEXUS_FrontendHandle frontend = NULL;
    unsigned frontendNum = 0;
    NEXUS_GpioSettings tunerGpioSettings;
    NEXUS_GpioHandle tunerGpio;

    /* Open on-board 4506 */
    NEXUS_Frontend_GetDefault4506Settings(&settings4506);
    settings4506.i2cDevice = pConfig->i2c[NEXUS_I2C_CHANNEL_LNA];
    if (!settings4506.i2cDevice) {
        BDBG_ERR(("Unable to initialize I2C"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &tunerGpioSettings);
    tunerGpioSettings.mode = NEXUS_GpioMode_eInput;
    tunerGpioSettings.interruptMode = NEXUS_GpioInterrupt_eFallingEdge;
    tunerGpio = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 50, &tunerGpioSettings);
    if (NULL == tunerGpio)
    {
        BDBG_ERR(("Unable to open GPIO for tuner."));
    }

    /* Initialize the 4 onboard BCM4506 tuners on 97425DBS */
    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        /* GPIO's are used instead of EXT_IRQ's
        * GPIO's have UPG_MAIN as their L1
        * GPIO 50 is used
        * TODO:  Program GPIO settings and ensure Nexus frontend has
        *        support for GPIO interrupts */
        BDBG_WRN(("Waiting for frontend %d to initialize", i));
        settings4506.gpioInterrupt = tunerGpio;
        settings4506.isrNumber = 0;

        switch(i)
        {
        /* Tuner 0 */
        case 0:
        case 1:
            settings4506.i2cAddr = 0x69;
            break;

        /* Tuner 1 */
        case 2:
        case 3:
            settings4506.i2cAddr = 0x68;
            break;

        /* Tuner 2 */
        case 4:
        case 5:
            settings4506.i2cAddr = 0x67;
            break;

        /* Tuner 3 */
        case 6:
        case 7:
            settings4506.i2cAddr = 0x66;
            break;

        default:
            BDBG_ERR(("Invalid Frontend = %d", i));
            break;
        }

        /* Channel 0 of Tuner */
        if ( (i%2) == 0)
            settings4506.channelNumber = 0;
        /* Channel 1 of Tuner */
        else
            settings4506.channelNumber = 1;

        g_onboard4506[frontendNum] = frontend = NEXUS_Frontend_Open4506(&settings4506);

        if ( NULL == frontend )
        {
            BDBG_ERR(("Unable to init on-board 4506 "));
        }

        /* Set the appropriate input bands */
        NEXUS_Frontend_GetUserParameters(frontend, &userParams);
        userParams.pParam2 = 0;

        switch(i)
        {
        /* Tuner 0 */
        case 0:
            userParams.param1 = NEXUS_InputBand_e0;
            break;
        case 1:
            userParams.param1 = NEXUS_InputBand_e1;
            break;

        /* Tuner 1 */
        case 2:
            userParams.param1 = NEXUS_InputBand_e2;
            break;
        case 3:
            userParams.param1 = NEXUS_InputBand_e3;
#if NEXUS_PLATFORM_7425_DBS
            /* Reset ISL6423 wired to 4506B ch1 */
            {
                uint8_t i2cAddrISL = 0x09;
                uint8_t data = 0x40;
                NEXUS_I2c_WriteNoAddr(settings4506.i2cDevice, i2cAddrISL, &data, 1);
                data = 0x70;
                NEXUS_I2c_WriteNoAddr(settings4506.i2cDevice, i2cAddrISL, &data, 1);
            }
#endif
            break;

        /* Tuner 2 */
        case 4:
            userParams.param1 = NEXUS_InputBand_e4;
            break;
        case 5:
            userParams.param1 = NEXUS_InputBand_e5;
            break;

        /* Tuner 3 */
        case 6:
            userParams.param1 = NEXUS_InputBand_e6;
            break;
        case 7:
            userParams.param1 = NEXUS_InputBand_e7;
            break;

        default:
            BDBG_ERR(("Invalid Frontend = %d", i));
            break;
        }

        NEXUS_Frontend_SetUserParameters(frontend, &userParams);
        pConfig->frontend[frontendNum] = frontend;
        frontend = NULL;
        frontendNum++;
    }

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    unsigned i;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if(g_onboard4506[i]) {
            NEXUS_Frontend_Close(g_onboard4506[i]);
            g_onboard4506[i] = NULL;
        }
    }
}

#else

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return 0;
}

void NEXUS_Platform_UninitFrontend(void)
{
}
#endif

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
    *pInputBand = NEXUS_InputBand_e5;
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

