/******************************************************************************
* Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_types.h"
#if NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND

#include "priv/nexus_core.h"
#include "nexus_i2c.h"
#include "bfpga.h"
#include "bfpga_name.h"
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

BDBG_MODULE(nexus_platform_frontend);

/* i2c channel assignments */
#define I2C_DEVICE_VOLTAGE_REG_CH 3

#define ISL9492_CH0_I2C_ADDR 0x08
#define ISL9492_CH1_I2C_ADDR 0x09


#if NEXUS_PLATFORM_97346_H43 || NEXUS_PLATFORM_97346_EXT2

static NEXUS_GpioHandle gpioHandleInt = NULL;

#if NEXUS_PLATFORM_97346_EXT2
#define NEXUS_PLATFORM_GPIO_INTERRUPT 0
#define NEXUS_PLATFORM_GPIO_RESET 2
#define NEXUS_PLATFORM_I2C 3
#else /* H43 */
#define NEXUS_PLATFORM_GPIO_INTERRUPT 2
#define NEXUS_PLATFORM_GPIO_RESET 3
#define NEXUS_PLATFORM_I2C 2
#endif

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_FrontendDeviceHandle device;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    unsigned offset=0;
    NEXUS_FrontendDeviceOpenSettings deviceSettings;
    NEXUS_GpioSettings gpioSettings;
    unsigned i2c = NEXUS_PLATFORM_I2C;
    unsigned i2c_addr = 0x68;
    unsigned intGpio = NEXUS_PLATFORM_GPIO_INTERRUPT;
    NEXUS_GpioType intGpioType = NEXUS_GpioType_eStandard;

#if NEXUS_PLATFORM_97346_EXT2
    {
        NEXUS_7346FrontendSettings settings;
        NEXUS_Frontend_GetDefault7346Settings(&settings);

        /* Setup LNA configurations per-board. */
        settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn0;
        settings.lnaSettings.out1 = NEXUS_7346LnaInput_eIn1;
        settings.lnaSettings.daisy = NEXUS_7346LnaInput_eIn0;
        settings.isInternalLna = true;

        for ( i=0; i < NEXUS_7346_MAX_FRONTEND_CHANNELS; i++ )
        {
            BDBG_WRN(("734x Trying to open frontend channel %d", i));
            settings.channelNumber = i;

            pConfig->frontend[i] = NEXUS_Frontend_Open7346(&settings);
            if ( pConfig->frontend[i] )
            {
                NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
                switch (i)
                {
                case 0: userParams.param1 = NEXUS_InputBand_e8; break;
                case 1: userParams.param1 = NEXUS_InputBand_e9; break;
                default: BDBG_MSG(("unsupported channel!"));
                    }
                userParams.pParam2 = 0;
                NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
            }
            else
            {
                BDBG_MSG(("NEXUS_Frontend_Open7346 Failed!"));
            }
            BDBG_MSG(("pConfig->frontend[%d] = 0x%x", i, pConfig->frontend[i]));

        }
        offset = NEXUS_7346_MAX_FRONTEND_CHANNELS;
    }
#endif

    NEXUS_FrontendDevice_GetDefaultOpenSettings(&deviceSettings);

    deviceSettings.reset.enable = true;
    deviceSettings.reset.pin = NEXUS_PLATFORM_GPIO_RESET;
    deviceSettings.reset.type = NEXUS_GpioType_eStandard;
    deviceSettings.reset.value = NEXUS_GpioValue_eHigh;

    deviceSettings.i2cDevice = pConfig->i2c[i2c];
    deviceSettings.i2cAddress = i2c_addr;
#if 0
    deviceSettings.satellite.diseqc.i2cAddress= 0x0B;
    deviceSettings.satellite.diseqc.i2cDevice = pConfig->i2c[NEXUS_PLATFORM_SAT_I2C];
#endif

    {
        NEXUS_FrontendProbeResults probeResults;

        BSTD_UNUSED(userParams);

        NEXUS_FrontendDevice_Probe(&deviceSettings, &probeResults);
        if (probeResults.chip.familyId != 0) {
            BDBG_WRN(("Opening %x...",probeResults.chip.familyId));
            deviceSettings.satellite.enabled = true;

            BDBG_MSG(("Setting up interrupt on GPIO %d",intGpio));
            NEXUS_Gpio_GetDefaultSettings(intGpioType, &gpioSettings);
            gpioSettings.mode = NEXUS_GpioMode_eInput;
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
            gpioHandleInt = NEXUS_Gpio_Open(intGpioType, intGpio, &gpioSettings);
            BDBG_ASSERT(NULL != gpioHandleInt);

            deviceSettings.gpioInterrupt = gpioHandleInt;
            device = NEXUS_FrontendDevice_Open(0, &deviceSettings);

            if (device) {
                NEXUS_FrontendDeviceCapabilities capabilities;

                NEXUS_FrontendDevice_GetCapabilities(device, &capabilities);
                BDBG_MSG(("Opening %d %x frontends",capabilities.numTuners,probeResults.chip.familyId));
                for (i=0; i < capabilities.numTuners ; i++)
                {
                    NEXUS_FrontendChannelSettings channelSettings;
                    channelSettings.device = device;
                    channelSettings.channelNumber = i;
                    channelSettings.type = NEXUS_FrontendChannelType_eSatellite;
                    pConfig->frontend[i+offset] = NEXUS_Frontend_Open(&channelSettings);
                    if ( NULL == (pConfig->frontend[i+offset]) )
                    {
                        BDBG_ERR(("Unable to open %x demod %d (as frontend[%d])",probeResults.chip.familyId,i,i+offset));
                        continue;
                    }
                    BDBG_MSG(("%xfe: %d(%d):%p",probeResults.chip.familyId,i,i,(void *)pConfig->frontend[i+offset]));
                }

            } else {
                BDBG_ERR(("Unable to open detected %x frontend", probeResults.chip.familyId));
            }
        } else {
            BDBG_WRN(("No frontend found."));
        }
    }

    return NEXUS_SUCCESS;
}

#elif NEXUS_PLATFORM_97346_SV

#define NEXUS_PLATFORM_SAT_GPIO_INTERRUPT 62
#define NEXUS_PLATFORM_SAT_I2C 2
#define NEXUS_PLATFORM_SAT_FRONTEND_I2C_ADDRESS 0x69
#define NEXUS_PLATFORM_SAT_MTSIF_OFFSET 2

#define I2C_DEVICE_FPGA_CH 2
#define FPGA_CHIP_ADDR 0xE

static NEXUS_GpioHandle gpioHandleInt = NULL;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_7346FrontendSettings settings;
    NEXUS_FrontendUserParameters userParams;
    uint8_t  data_c, data_d, data_e, regaddr=0;
    unsigned i=0;
    uint8_t data;
    unsigned j;
    NEXUS_4506Settings settings4506;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_GpioSettings gpioSettings;

    NEXUS_Frontend_GetDefault7346Settings(&settings);

    /* Setup LNA configurations per-board. */
    settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.out1 = NEXUS_7346LnaInput_eIn1;
    settings.lnaSettings.daisy = NEXUS_7346LnaInput_eIn0;
    settings.isInternalLna = true;

 {
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
        reg &= ~(
                 BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062)
                 );
        reg |= (
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062, 0)
                );
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);
    }

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
            case 0: userParams.param1 = NEXUS_InputBand_e8; break;
            case 1: userParams.param1 = NEXUS_InputBand_e9; break;
            default: BDBG_MSG(("unsupported channel!"));
                }
            userParams.pParam2 = 0;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        }
        else
        {
            BDBG_MSG(("NEXUS_Frontend_Open7346 Failed!"));
        }
        BDBG_MSG(("pConfig->frontend[%d] = 0x%x", i, pConfig->frontend[i]));

    }

    BDBG_MSG(("Setting up interrupt on GPIO %d",NEXUS_PLATFORM_SAT_GPIO_INTERRUPT));
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eLow;
    gpioHandleInt = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, NEXUS_PLATFORM_SAT_GPIO_INTERRUPT, &gpioSettings);
    BDBG_ASSERT(NULL != gpioHandleInt);

   /* Open 4505 has only one Demodulator Channel */
    for ( j = 0; j < NEXUS_4506_MAX_FRONTEND_CHANNELS - 1 ; j++, i++ )
    {
        BDBG_WRN(("4506 Trying to open frontend channel %d", i));
        /* Open on-board 4506 */
        NEXUS_Frontend_GetDefault4506Settings(&settings4506);
        settings4506.i2cDevice = pConfig->i2c[NEXUS_PLATFORM_SAT_I2C];

        if (!settings4506.i2cDevice) {
            BDBG_MSG(("Unable to initialize I2C"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        settings4506.gpioInterrupt = gpioHandleInt;
        settings4506.i2cAddr = NEXUS_PLATFORM_SAT_FRONTEND_I2C_ADDRESS;
        /* As of now, only for 4506 daughter card, this need to be set true. Because the 4506 on the daughter card directly controls the external 3445 LNA. */
        settings4506.is3445ExternalLna = false;
        settings4506.channelNumber = j;
        pConfig->frontend[i] = NEXUS_Frontend_Open4506(&settings4506);
        if ( pConfig->frontend[i] )
        {
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
            switch (j)
            {
            case 0: userParams.param1 = NEXUS_InputBand_e5; break;

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

    /* Tone generated internally according to EXTMpin */
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    /* Dynamic current limit */
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    /* Internal linear regulator is turned on and boost circuit is on */
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00,(const uint8_t *) &data, 1);
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *) &data, 1);
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *) &data, 1);


    /* Configure FPGA on 7346(5) SV board */
    BDBG_MSG(("fpga i2c %d %p", I2C_DEVICE_FPGA_CH , g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH]));
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xc, &data_c, 1);
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xd, &data_d, 1);
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 0xe, &data_e, 1);

    BDBG_MSG(("FPGA version:  0x%02x", data_e));

    if ((data_c != 0x46) || (data_d != 0x73)) BDBG_MSG(("Reading of Transport FPGA yields CHIPID=0x%02x%02x", data_d, data_c));

    data = 0x80;
    (void)NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, 4, &data, 1);

    if (NEXUS_GetEnv("NEXUS_STREAMER")) {
        regaddr = 2;
        data = 0x00; /* Use streamer 0 for LVDS, pkt interface 0 */
    } else { /* ASI is default*/
        regaddr = 2;
        data = 0x10; /* Use streamer 1 for LVDS, pkt interface 2 */
    }
    (void)NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, regaddr, &data, 1);
    BKNI_Sleep(10);

    /* re-read data back and give a warning */
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], FPGA_CHIP_ADDR, regaddr, &data, 1);
    if (NEXUS_GetEnv("NEXUS_STREAMER")) {
       if (data != 0x00)
       {
           BDBG_ERR(("LVDS will not work data value is 0x%02x it should be 0x00", data));
       }
    } else { /* ASI is default*/
       if (data != 0x10)
       {
           BDBG_ERR(("ASI will not work data value is 0x%02x it should be 0x10", data));
       }
    }

    return BERR_SUCCESS;
}
#else
/* boards without external frontend SF,CWM,..  */
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_7346FrontendSettings settings;
    NEXUS_FrontendUserParameters userParams;
    unsigned i=0;
    uint8_t data;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;

    NEXUS_Frontend_GetDefault7346Settings(&settings);

    /* Setup LNA configurations per-board. */
    settings.lnaSettings.out0 = NEXUS_7346LnaInput_eIn0;
    settings.lnaSettings.out1 = NEXUS_7346LnaInput_eIn1;
    settings.lnaSettings.daisy = NEXUS_7346LnaInput_eIn0;
    settings.isInternalLna = true;

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
            case 0: userParams.param1 = NEXUS_InputBand_e8; break;
            case 1: userParams.param1 = NEXUS_InputBand_e9; break;
            default: BDBG_MSG(("unsupported channel!"));
                }
            userParams.pParam2 = 0;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
        }
        else
        {
            BDBG_MSG(("NEXUS_Frontend_Open7346 Failed!"));
        }
        BDBG_MSG(("pConfig->frontend[%d] = 0x%x", i, pConfig->frontend[i]));
    }

    /* Tone generated internally according to EXTMpin */
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    /* Dynamic current limit */
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    /* Internal linear regulator is turned on and boost circuit is on */
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    data = 0x20;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00,(const uint8_t *) &data, 1);
    data = 0x44;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *) &data, 1);
    data = 0x78;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL9492_CH1_I2C_ADDR, 0x00, (const uint8_t *) &data, 1);
    return NEXUS_SUCCESS;
}
#endif

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
#if NEXUS_PLATFORM_97346_SV || NEXUS_PLATFORM_97346_H43
    if (gpioHandleInt) {
       NEXUS_Gpio_Close(gpioHandleInt);
       gpioHandleInt = NULL;
    }
#endif
    return;
}

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{

    BDBG_ASSERT(pInputBand);
    BTRC_TRACE(ChnChange_TuneStreamer, START);

    BSTD_UNUSED(index);
    /* ASI is default unless you set runtime variable
       NEXUS_STREAMER*/
    *pInputBand = NEXUS_InputBand_e2;

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
/* NO I2C support so we cannot support the frontend. Still need stubs. */
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    return;
}
#endif /* NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND */
