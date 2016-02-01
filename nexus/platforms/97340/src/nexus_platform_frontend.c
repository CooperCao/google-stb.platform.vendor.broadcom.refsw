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
#if (BCHP_CHIP==7340)
#include "nexus_input_band.h"
#endif

BDBG_MODULE(nexus_platform_frontend);

/* Handle for the BCM3440/BCM3430 connected to the QPSK channel */
static NEXUS_TunerHandle g_qpskTuner;
static NEXUS_FrontendHandle g_frontends[NEXUS_MAX_FRONTENDS];


/* i2c channel assignments */
#define I2C_DEVICE_FPGA_CH 2
#if (BCHP_CHIP==7340)
#define I2C_DEVICE_VOLTAGE_REG_CH 1
#else /* 7342 */
#define I2C_DEVICE_VOLTAGE_REG_CH 3
#endif

#define ISL6423_CH0_I2C_ADDR 0x08
#define ISL6423_CH1_I2C_ADDR 0x09



NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_3440Settings qpskTunerSettings;
    #if BCHP_CHIP == 7340
    NEXUS_97340FrontendExtensionSettings extensionSettings;
    NEXUS_97340FrontendExtensionHandles extensionHandles;
    #else /* 7342*/
    unsigned boardRev=0;
    NEXUS_97342FrontendExtensionSettings extensionSettings;
    NEXUS_97342FrontendExtensionHandles extensionHandles;
    #endif
    NEXUS_73xxFrontendSettings settings;
    NEXUS_FrontendUserParameters userParams;
    unsigned i;
	uint8_t chipaddr, regaddr, data;

    NEXUS_Frontend_GetDefault73xxSettings(&settings);

    /* write 0x88 to Reg2 of FPGA to prevent interference with gpio's */
        chipaddr = 0x0E;
        regaddr = 0x02;
        if (!NEXUS_GetEnv("NEXUS_ASI")) {
            data = 0x80;
        }else{
            data= 0x00;
        }
        (void)NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_FPGA_CH], chipaddr, regaddr, &data, 1);


    /* Setup LNA configurations per-board. */

    #if BCHP_CHIP == 7340
    settings.lnaI2cChannelNumber = 0;
    /* The 97325 has the default input fron IN2 on the LNA and does not use the daisy */
    settings.lnaSettings.out1 = NEXUS_73xxLnaInput_eNone;
    settings.lnaSettings.out2 = NEXUS_73xxLnaInput_eNone;
    settings.lnaSettings.daisy = NEXUS_73xxLnaInput_eNone;
    #else /* 7342 */

    settings.lnaI2cChannelNumber = 1;
    /* TODO : The 97335 has defaults with IN1-> OUT1 and IN2 -> OUT2/DAISY */
    settings.lnaSettings.out1 = NEXUS_73xxLnaInput_eIn1Vga;
    settings.lnaSettings.out2 = NEXUS_73xxLnaInput_eIn2Vga;
    settings.lnaSettings.daisy = NEXUS_73xxLnaInput_eIn2Vga;
    #endif

    /* Open 73xx Demodulator Channels */
    for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
    {
        BDBG_MSG(("73xx Trying to open frontend channel %d", i));
        settings.channelNumber = i;

        #if BCHP_CHIP == 7340
        settings.lnaOutput = NEXUS_73xxLnaOutput_eNone;
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
#if (BCHP_CHIP==7342)
                case 0:  userParams.param1 = NEXUS_InputBand_e5; break;

                case 1: userParams.param1 = NEXUS_InputBand_e6; break;
                case 2: userParams.param1 = NEXUS_InputBand_e7; break;

                default: BDBG_ERR(("unsupported channel!"));
#else /* 97340 */
            case 0: userParams.param1 = NEXUS_InputBand_e2; break;
            case 1: userParams.param1 = NEXUS_InputBand_e3; break;
            default: BDBG_ERR(("unsupported channel!"));
#endif
                }
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(g_frontends[i], &userParams);
        }
        else
        {
            BDBG_ERR(("NEXUS_Frontend_Open7325 Failed!"));
        }
    }
#if (BCHP_CHIP==7340)
    {
        NEXUS_InputBandSettings inputBandSettings;

        /* set parallel on the two input bands */
        NEXUS_InputBand_GetSettings(2, &inputBandSettings);
        inputBandSettings.parallelInput = true;
        NEXUS_InputBand_SetSettings(2, &inputBandSettings);

        NEXUS_InputBand_GetSettings(3, &inputBandSettings);
        inputBandSettings.parallelInput = true;
        NEXUS_InputBand_SetSettings(3, &inputBandSettings);
    }
#endif

    /* Open QPSK tuner */
    NEXUS_Tuner_GetDefault3440Settings(&qpskTunerSettings);
    qpskTunerSettings.i2cDevice = NEXUS_Frontend_Get73xxMasterI2c(g_frontends[NEXUS_MAX_FRONTENDS-1]);
    g_qpskTuner = NEXUS_Tuner_Open3440(&qpskTunerSettings);
    if ( NULL == g_qpskTuner )
    {
        BDBG_ERR(("Unable to open QPSK tuner"));
    }

    /* TODO:  Configure ISL 6423 voltage regulator */

	data = 0x40;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH0_I2C_ADDR, 0x00, (const uint8_t *)&data, 1);
	data = 0x70;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH0_I2C_ADDR, 0x00,(const uint8_t *)&data, 1);
#if BCHP_CHIP==7342
	data = 0x40;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH1_I2C_ADDR, 0x00,(const uint8_t *) &data, 1);
	data = 0x70;
    NEXUS_I2c_Write(g_NEXUS_platformHandles.config.i2c[I2C_DEVICE_VOLTAGE_REG_CH], ISL6423_CH1_I2C_ADDR,0x00, (const uint8_t *) &data, 1);
#endif

    /* Install frontend extensions */
#if BCHP_CHIP == 7340
    NEXUS_Frontend_GetDefault97340ExtensionSettings(&extensionSettings);
    extensionSettings.primary = g_frontends[0];
#if (NEXUS_MAX_FRONTENDS > 1)
    extensionSettings.qpsk = g_frontends[1];
#endif
    extensionSettings.qpskTuner = g_qpskTuner;
    NEXUS_Frontend_Create97340Extension(&extensionSettings, &extensionHandles);
    g_NEXUS_platformHandles.config.frontend[0] = extensionHandles.primary;
#if (NEXUS_MAX_FRONTENDS > 1)
    g_NEXUS_platformHandles.config.frontend[1] = extensionHandles.qpsk;
#endif
#else
    NEXUS_Frontend_GetDefault97342ExtensionSettings(&extensionSettings);
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
    NEXUS_Frontend_Create97342Extension(&extensionSettings, &extensionHandles);
    g_NEXUS_platformHandles.config.frontend[0] = extensionHandles.primary;
#if (NEXUS_MAX_FRONTENDS > 1)
    g_NEXUS_platformHandles.config.frontend[1] = extensionHandles.secondary;
#endif
#if (NEXUS_MAX_FRONTENDS > 2)
    g_NEXUS_platformHandles.config.frontend[2] = extensionHandles.qpsk;
#endif
    #endif

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    int i, j;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        handleFound = false;
        if (g_frontends[i]) {
            tempHandle = NEXUS_Frontend_GetDevice(g_frontends[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                        handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }
        }
    }

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

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }
}

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    unsigned index_value = index;
    BDBG_ASSERT(pInputBand);

    if (index > index_value) {
        BDBG_ERR(("Only %ud streamer input available", index_value));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }


    BTRC_TRACE(ChnChange_TuneStreamer, START);

#if BCHP_CHIP==7340
    if (NEXUS_GetEnv("NEXUS_ASI") || index==1)
        /* ASI  */
        *pInputBand = NEXUS_InputBand_e1;
    else
        *pInputBand = NEXUS_InputBand_e0;

#else /* 7342 Board only */
    if (NEXUS_GetEnv("NEXUS_ASI") || index==1)
    {
            /* ASI  */
        BDBG_ERR(("ASI"));
        *pInputBand = NEXUS_InputBand_e3;
    }
    else
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
