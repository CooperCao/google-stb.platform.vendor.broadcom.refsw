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
*   API name: Platform Frontend
*    Platform Frontend Setup
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

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
#include "nexus_frontend_73xx.h"
#include "nexus_tuner_3440.h"
#include "nexus_frontend_extension.h"
#include "bchp_sun_top_ctrl.h"

BDBG_MODULE(nexus_platform_frontend);

/* Handle for the BCM3440/BCM3430 connected to the QPSK channel */
static NEXUS_TunerHandle g_qpskTuner;
static NEXUS_FrontendHandle g_frontends[NEXUS_MAX_FRONTENDS];


/* i2c channel assignments */
#define I2C_DEVICE_FPGA_CH 2
#if (BCHP_CHIP==7325)
#define I2C_DEVICE_VOLTAGE_REG_CH 1
#else
#define I2C_DEVICE_VOLTAGE_REG_CH 3
#endif

#define ISL6423_CH0_I2C_ADDR 0x08
#define ISL6423_CH1_I2C_ADDR 0x09

#if BCHP_CHIP == 7335 || BCHP_CHIP == 7336
/* For the 97335, we need to know the board revision */
static unsigned NEXUS_Platform_P_Get97335Revision(void)
{
    uint8_t data_c=0, data_d=0, data_f=0;
    unsigned boardRev;
    /* Determine board level */
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], 0xe, 0xc, &data_c, 1);
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], 0xe, 0xd, &data_d, 1);
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], 0xe, 0xf, &data_f, 1);
    boardRev = data_c & 0xf;
    BDBG_MSG(("FPGA:  0xc=0x%02x, 0xd=0x%02x, 0xf=0x%02x", data_c, data_d, data_f));
    BDBG_MSG(("BCM97335 P%d board detected", boardRev));
    return boardRev;
}

static unsigned NEXUS_Platform_P_Get97335FpgaRevision(void)
{
    uint8_t  data_f=0;
    unsigned fpgaRev;
    /* Determine fpga version */
    (void)NEXUS_I2c_Read(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], 0xe, 0xf, &data_f, 1);
    fpgaRev = (data_f >> 4) & 0x0F;
    BDBG_MSG(("FPGA version: 0xf=0x%02x", data_f));
    return fpgaRev;
}
#endif


NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_3440Settings qpskTunerSettings;
    #if BCHP_CHIP == 7325
    NEXUS_97325FrontendExtensionSettings extensionSettings;
    NEXUS_97325FrontendExtensionHandles extensionHandles;
    #else
    unsigned boardRev=0, fpgaRev=0;
    NEXUS_97335FrontendExtensionSettings extensionSettings;
    NEXUS_97335FrontendExtensionHandles extensionHandles;
    #endif
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    NEXUS_73xxFrontendSettings settings;
    NEXUS_FrontendUserParameters userParams;
    unsigned i;
	uint8_t data;

#if BCHP_CHIP==7335
    boardRev = NEXUS_Platform_P_Get97335Revision();
    fpgaRev = NEXUS_Platform_P_Get97335FpgaRevision();
#endif
    NEXUS_Frontend_GetDefault73xxSettings(&settings);

    /* Setup LNA configurations per-board. */

    #if BCHP_CHIP == 7325
    settings.lnaI2cChannelNumber = 0;
    /* The 97325 has the default input fron IN2 on the LNA and does not use the daisy */
    settings.lnaSettings.out1 = NEXUS_73xxLnaInput_eIn2Vga;
    settings.lnaSettings.out2 = NEXUS_73xxLnaInput_eIn2Vga;
    settings.lnaSettings.daisy = NEXUS_73xxLnaInput_eNone;
    #elif BCHP_CHIP == 7335 || BCHP_CHIP == 7336
    if (( boardRev > 1 ) || ( fpgaRev > 4 ))
    {
        settings.lnaI2cChannelNumber = 1;
    }
    else
    {
        settings.lnaI2cChannelNumber = 0;
    }
    /* The 97335 has defaults with IN1-> OUT1 and IN2 -> OUT2/DAISY */
    settings.lnaSettings.out1 = NEXUS_73xxLnaInput_eIn1Vga;
    settings.lnaSettings.out2 = NEXUS_73xxLnaInput_eIn2Vga;
    settings.lnaSettings.daisy = NEXUS_73xxLnaInput_eIn2Vga;
    #endif

    /* Open 73xx Demodulator Channels */
    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        BDBG_MSG(("73xx Trying to open frontend channel %d", i));
        settings.channelNumber = i;

        #if BCHP_CHIP == 7325
        settings.lnaOutput = NEXUS_73xxLnaOutput_eOut1 + i;
        #else
        switch ( i )
        {
        case 0:
            settings.lnaOutput = NEXUS_73xxLnaOutput_eOut1;
            break;
        case 1:
            settings.lnaOutput = NEXUS_73xxLnaOutput_eDaisy;
            break;
        case 2:
            settings.lnaOutput = NEXUS_73xxLnaOutput_eOut2;
            break;
        default:
            BDBG_ASSERT(i<=2);
            break;
        }
        #endif

        g_frontends[i] = NEXUS_Frontend_Open73xx(&settings);
        if ( g_frontends[i] )
        {
            NEXUS_Frontend_GetUserParameters(g_frontends[i], &userParams);
            switch (i)
            {
                case 0: userParams.param1 = NEXUS_InputBand_e2; break;
                case 1: userParams.param1 = NEXUS_InputBand_e3; break;
                #if (BCHP_CHIP==7335) || BCHP_CHIP == 7336
                case 2: userParams.param1 = NEXUS_InputBand_e5; break;
                #endif
                default: BDBG_ERR(("unsupported channel!"));
            }
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(g_frontends[i], &userParams);
        }
        else
        {
            BDBG_ERR(("NEXUS_Frontend_Open7325 Failed!"));
        }
    }

    /* Open QPSK tuner */
    NEXUS_Tuner_GetDefault3440Settings(&qpskTunerSettings);
    qpskTunerSettings.i2cDevice = NEXUS_Frontend_Get73xxMasterI2c(g_frontends[NEXUS_MAX_FRONTENDS-1]);
    g_qpskTuner = NEXUS_Tuner_Open3440(&qpskTunerSettings);
    if ( NULL == g_qpskTuner )
    {
        BDBG_ERR(("Unable to open QPSK tuner"));
    }

    /* Configure ISL 6423 voltage regulator */
    #if BCHP_CHIP == 7325
	data = 0x40;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH0_I2C_ADDR,0x00,(const uint8_t *)&data, 1);
	data = 0x70;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH0_I2C_ADDR, 0x00,(const uint8_t *) &data, 1);
    #else
    if ( boardRev >= 3 )
    {
        /* Configure CH0 normally to be controlled by 7335 */
        data= 0x40;
        NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    }
    else
    {
        /* Configure CH0 to be controlled via I2C */
        data=0x48;
        NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    }
    data=0x70;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH0_I2C_ADDR, 0x00, (const uint8_t *) &data, 1);
    data=0x40;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH1_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
    data=0x70;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH1_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
    #endif

    /* Setup pinmuxes on 97335 boards to use sgpio signals for the SDS I2C Channel 0 pins.  These control the input filter.  */
    #if BCHP_CHIP == 7335 || BCHP_CHIP == 7336
    if (( boardRev >= 3 ) || ( fpgaRev >= 5 ))
    {
        uint32_t reg;
        reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
        reg &=~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sds0_tnr_sda) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sds0_tnr_scl)
               );

        reg |=(
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sds0_tnr_sda, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sds0_tnr_scl, 1)
               );
        BREG_Write32(g_pCoreHandles->reg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,reg);
    }
    #endif

    /* Install frontend extensions */
    #if BCHP_CHIP == 7325
    NEXUS_Frontend_GetDefault97325ExtensionSettings(&extensionSettings);
    extensionSettings.primary = g_frontends[0];
#if (NEXUS_MAX_FRONTENDS > 1)
    extensionSettings.qpsk = g_frontends[1];
#endif
    extensionSettings.qpskTuner = g_qpskTuner;
    NEXUS_Frontend_Create97325Extension(&extensionSettings, &extensionHandles);
    g_NEXUS_platformHandles.config.frontend[0] = extensionHandles.primary;
#if (NEXUS_MAX_FRONTENDS > 1)
    g_NEXUS_platformHandles.config.frontend[1] = extensionHandles.qpsk;
#endif
    #else
    NEXUS_Frontend_GetDefault97335ExtensionSettings(&extensionSettings);
    extensionSettings.boardRev = boardRev;
    extensionSettings.primary = g_frontends[0];
#if (NEXUS_MAX_FRONTENDS > 1)
    extensionSettings.secondary = g_frontends[1];
#endif
#if (NEXUS_MAX_FRONTENDS > 2)
    extensionSettings.qpsk = g_frontends[2];
#endif
    extensionSettings.qpskTuner = g_qpskTuner;
    extensionSettings.isl6423I2cDevice = g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH];
    extensionSettings.fpgaI2cDevice = g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH];
    NEXUS_Frontend_Create97335Extension(&extensionSettings, &extensionHandles);
    g_NEXUS_platformHandles.config.frontend[0] = extensionHandles.primary;
#if (NEXUS_MAX_FRONTENDS > 1)
    g_NEXUS_platformHandles.config.frontend[1] = extensionHandles.secondary;
#endif
#if (NEXUS_MAX_FRONTENDS > 2)
    g_NEXUS_platformHandles.config.frontend[2] = extensionHandles.qpsk;
#endif
    #endif

    /* Set LNB Enabled by default */
    diseqcSettings.lnbEnabled = true;
   NEXUS_Frontend_SetDiseqcSettings(g_frontends[0], &diseqcSettings);
#if BCHP_CHIP ==7335
   NEXUS_Frontend_SetDiseqcSettings(g_frontends[1], &diseqcSettings);
#endif
    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    int i;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;

    for ( i = NEXUS_MAX_FRONTENDS-1; i >= 0; i-- )
    {
        /* Close extensions first */
        if (pConfig->frontend[i])
        {
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
        /* Close underlying demod */
        if ( g_frontends[i] )
        {
            NEXUS_Frontend_Close(g_frontends[i]);
            g_frontends[i] = NULL;
        }
    }

    if ( g_qpskTuner )
    {
        NEXUS_Tuner_Close(g_qpskTuner);
        g_qpskTuner = NULL;
    }
}

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{

    unsigned index_value = 0;

    BDBG_ASSERT(pInputBand);

    if (index > index_value) {
        BDBG_ERR(("Only %ud streamer input available", index_value));
        BDBG_ERR(("For ASI please export NEXUS_ASI=y on your Settop Box", index_value));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }


    BTRC_TRACE(ChnChange_TuneStreamer, START);
#if BCHP_CHIP == 7325
    *pInputBand = NEXUS_InputBand_e0;
#else

    BDBG_MSG(("We are using InputBand %u", index_value));

    /* ASI and LVDS get routed to this input */
    *pInputBand = NEXUS_InputBand_e1;

#endif

    BTRC_TRACE(ChnChange_TuneStreamer, STOP);
    return NEXUS_SUCCESS;
}

#else
#endif /* NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND */

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


