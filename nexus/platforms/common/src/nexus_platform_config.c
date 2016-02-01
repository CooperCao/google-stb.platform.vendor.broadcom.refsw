/***************************************************************************
*     (c)2004-2015 Broadcom Corporation
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
*   API name: Platform
*    Specific APIs to configure already initialized board.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "bchp_sun_top_ctrl.h"
#include "bchp_common.h"
#include "nexus_base.h"
#include "nexus_platform_priv.h"
#if NEXUS_HAS_I2C
#include "priv/nexus_i2c_priv.h"
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#endif
#if NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#include "nexus_svideo_output.h"
#include "nexus_video_output.h"
#include "priv/nexus_display_priv.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "priv/nexus_hdmi_output_priv.h"
#endif
#if NEXUS_HAS_CEC
#include "priv/nexus_cec_priv.h"
#endif
#if NEXUS_HAS_RFM
#include "nexus_rfm.h"
#include "priv/nexus_rfm_priv.h"
#endif
#if NEXUS_HAS_TRANSPORT
#include "nexus_pid_channel.h"
#include "nexus_input_band.h"
#endif
#ifdef NEXUS_FPGA_SUPPORT
#include "nexus_platform_fpga.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_output.h"
#include "priv/nexus_audio_output_priv.h"
#endif
#if NEXUS_HAS_GPIO
#include "nexus_gpio.h"
#endif

BDBG_MODULE(nexus_platform_config);
#ifdef NEXUS_FPGA_SUPPORT
static bool g_fpgaInit = false;
#endif
static void NEXUS_Platform_P_StartMonitor(void);
static void NEXUS_Platform_P_StopMonitor(void);

#if (((BCHP_CHIP == 7325) && (BCHP_VER <= BCHP_VER_A1))  \
 ||  ((BCHP_CHIP == 7325) && (BCHP_VER == BCHP_VER_B0))  \
 ||  ((BCHP_CHIP == 7335) && (BCHP_VER == BCHP_VER_A0))  \
 ||  ((BCHP_CHIP == 7335) && (BCHP_VER == BCHP_VER_B0))  \
 ||  ((BCHP_CHIP == 7400) && (BCHP_VER < BCHP_VER_E0))  \
 ||  ((BCHP_CHIP == 7405) && (BCHP_VER <= BCHP_VER_B0))  \
 ||  ((BCHP_CHIP == 7443) && (BCHP_VER == BCHP_VER_A0)) \
 ||  ((BCHP_CHIP == 7429) && (BCHP_VER == BCHP_VER_A0)))
#define NEXUS_HAS_HDMI_SOFTI2C_SUPPORT 1
#endif

#ifndef NEXUS_MAX_I2C_CHANNELS
#define NEXUS_MAX_I2C_CHANNELS 7
#endif

#ifndef NEXUS_HDMI_OUTPUT_I2C_CHANNEL
#if BCHP_CHIP == 7405
#define NEXUS_HDMI_OUTPUT_I2C_CHANNEL 1
#elif (BCHP_CHIP==7400) || (BCHP_CHIP==7420)  || (BCHP_CHIP==7335)  || (BCHP_CHIP==7336) || \
      (BCHP_CHIP==7125) || (BCHP_CHIP==7342)  || (BCHP_CHIP==7408)  || (BCHP_CHIP==7468) || \
      (BCHP_CHIP==7231) || (BCHP_CHIP==7346)  || (BCHP_CHIP==7358)  || (BCHP_CHIP==7552) || \
      (BCHP_CHIP==7344) || (BCHP_CHIP==7360)  || (BCHP_CHIP==7563)  || (BCHP_CHIP==7362) || \
      (BCHP_CHIP==7228) || (BCHP_CHIP==75635) || (BCHP_CHIP==73625) || (BCHP_CHIP==73465)
#define NEXUS_HDMI_OUTPUT_I2C_CHANNEL 0
#elif (BCHP_CHIP == 7325) ||  (BCHP_CHIP==7340)
#define NEXUS_HDMI_OUTPUT_I2C_CHANNEL 3
#elif (BCHP_CHIP==7422)  || (BCHP_CHIP==7425)  || (BCHP_CHIP==7435)  || (BCHP_CHIP==7584)  || \
      (BCHP_CHIP==7445)  || (BCHP_CHIP==7563)  || (BCHP_CHIP==7145)  || (BCHP_CHIP==7366)  || \
      (BCHP_CHIP==7364)  || (BCHP_CHIP==7250)  || (BCHP_CHIP==75635) || (BCHP_CHIP==7586)  || \
      (BCHP_CHIP==75845) || (BCHP_CHIP==75525) || (BCHP_CHIP==7268)  || (BCHP_CHIP==7271)  || \
      (BCHP_CHIP==74371) || (BCHP_CHIP==7439 && (BCHP_VER >= BCHP_VER_B0))
#define NEXUS_HDMI_OUTPUT_I2C_CHANNEL NEXUS_I2C_CHANNEL_HDMI_TX
#elif BCHP_CHIP==7439 && (BCHP_VER == BCHP_VER_A0)
#define NEXUS_HDMI_OUTPUT_I2C_CHANNEL NEXUS_I2C_CHANNEL_HDMI_TX
#define NEXUS_HDMI_OUTPUT_I2C_CHANNEL_1 NEXUS_I2C_CHANNEL_HDMI_TX_1
#elif (BCHP_CHIP==7429) || (BCHP_CHIP==74295)
#define NEXUS_HDMI_OUTPUT_I2C_CHANNEL 2
#else
#error UNSUPPORTED CHIP
#endif
#endif

#if BCHP_HDMI_TX_AUTO_I2C_REG_START
#define AUTO_I2C_ENABLED 1
#else
#define AUTO_I2C_ENABLED 0
#endif

/* TODO: remove BCHP_CHIP list when able */
#if ((BCHP_CHIP == 7563) || (BCHP_CHIP == 75635) || (BCHP_CHIP == 75525))
#define DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT 1
#endif

#if NEXUS_USE_7425_SV_BOARD || NEXUS_PLATFORM_7418SFF_H
#define NEXUS_HAS_EXTERNAL_RFM       (true)
#endif

#if NEXUS_HAS_EXTERNAL_RFM
extern NEXUS_Error NEXUS_Platform_P_InitExternalRfm(const NEXUS_PlatformConfiguration *pConfig);
#endif

static void NEXUS_Platform_P_StartMonitor(void);
static void NEXUS_Platform_P_StopMonitor(void);

#if NEXUS_HAS_I2C
void NEXUS_Platform_P_GetI2CGpioHandles(unsigned index, NEXUS_GpioHandle *gpioClk , NEXUS_GpioHandle *gpioData)
{
    NEXUS_GpioSettings gpioSettings;
    if(index == 0){
        if ((BCHP_CHIP==7429) || (BCHP_CHIP==7425) || (BCHP_CHIP==7439) || (BCHP_CHIP==7346)){
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonSpecial, &gpioSettings);
            *gpioClk = NEXUS_Gpio_Open(NEXUS_GpioType_eAonSpecial, 0, &gpioSettings);
            BDBG_ASSERT(NULL != gpioClk);
            *gpioData = NEXUS_Gpio_Open(NEXUS_GpioType_eAonSpecial, 1, &gpioSettings);
            BDBG_ASSERT(NULL != gpioData);
        }
        else if ((BCHP_CHIP==7584) || (BCHP_CHIP==7563) || (BCHP_CHIP==7400)){
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eSpecial, &gpioSettings);
            *gpioClk = NEXUS_Gpio_Open(NEXUS_GpioType_eSpecial, 0, &gpioSettings);
            BDBG_ASSERT(NULL != gpioClk);
            *gpioData = NEXUS_Gpio_Open(NEXUS_GpioType_eSpecial, 1, &gpioSettings);
            BDBG_ASSERT(NULL != gpioData);
        }
    }
    else if(index == 3){
        if(BCHP_CHIP==7439){
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eSpecial, &gpioSettings);
            *gpioClk = NEXUS_Gpio_Open(NEXUS_GpioType_eSpecial, 0, &gpioSettings);
            BDBG_ASSERT(NULL != gpioClk);
            *gpioData = NEXUS_Gpio_Open(NEXUS_GpioType_eSpecial, 1, &gpioSettings);
            BDBG_ASSERT(NULL != gpioData);
        }
    }
    else if(index == 2){
        if(BCHP_CHIP==7346){
            NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eSpecial, &gpioSettings);
            *gpioClk = NEXUS_Gpio_Open(NEXUS_GpioType_eSpecial, 4, &gpioSettings);
            BDBG_ASSERT(NULL != gpioClk);
            *gpioData = NEXUS_Gpio_Open(NEXUS_GpioType_eSpecial, 5, &gpioSettings);
            BDBG_ASSERT(NULL != gpioData);
        }
    }

    return;
}
#endif

NEXUS_Error NEXUS_Platform_P_Config(const NEXUS_PlatformSettings *pSettings)
{
    NEXUS_Error errCode=NEXUS_SUCCESS;
    unsigned i;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;

    /* remove unused warning */
    if (0) {goto error;}

#if NEXUS_HAS_I2C
    BDBG_CASSERT(NEXUS_MAX_I2C_CHANNELS <= NEXUS_MAX_CONFIG_HANDLES);
    /* Open I2C Channels */
    if (pSettings->openI2c)
    {
        NEXUS_I2cSettings i2cSettings;
        for ( i = 0; i < NEXUS_MAX_I2C_CHANNELS; i++ )
        {

#ifdef NEXUS_MOCA_I2C_CHANNEL
/* if NEXUS_MOCA_I2C_CHANNEL is defined in nexus_platform_features.h, nexus will avoid it */
            if (i == NEXUS_MOCA_I2C_CHANNEL) {
                continue;
            }
#endif

            NEXUS_I2c_GetDefaultSettings(&i2cSettings);

            i2cSettings.autoI2c.enabled = false;
#if NEXUS_USE_7584_WIFI
            if(i==1)continue;
#endif

            /* Because of 7344  A0 issues we need to disable the I2C channel being used for Moca as well as channel  3 */
#if (BCHP_CHIP==7344)
#if NEXUS_PLATFORM_7418SFF_H
            i2cSettings.softI2c = false;
            i2cSettings.interruptMode = false;
#endif
            if (i==3) {
                continue;
            }
#endif
#if (BCHP_CHIP==7364)
            if (i==2) {
                BDBG_MSG(("BSC-C shouldn't be used in 7364."));
                continue;
            }
#elif (BCHP_CHIP==7250)
            if (i==1) {
                BDBG_MSG(("BSC-B shouldn't be used in 7250."));
                continue;
            }
#endif

#if BCHP_CHIP==7439 && (BCHP_VER == BCHP_VER_A0)
            if (i==NEXUS_HDMI_OUTPUT_I2C_CHANNEL || i==NEXUS_HDMI_OUTPUT_I2C_CHANNEL_1)
#else
            if ( i == NEXUS_HDMI_OUTPUT_I2C_CHANNEL )
#endif
            {
                const char *hdmi_i2c_software_mode = NEXUS_GetEnv("hdmi_i2c_software_mode");
                #if NEXUS_HAS_HDMI_SOFTI2C_SUPPORT
                bool forceHardI2c = (hdmi_i2c_software_mode && (hdmi_i2c_software_mode[0] == 'n' ||
                                                                hdmi_i2c_software_mode[0] == 'N'));
                i2cSettings.softI2c = !forceHardI2c;
                #else
                bool forceSoftI2c = (hdmi_i2c_software_mode && (hdmi_i2c_software_mode[0] == 'y' ||
                                                                hdmi_i2c_software_mode[0] == 'Y'));
                i2cSettings.softI2c = forceSoftI2c;
                #endif
#if AUTO_I2C_ENABLED
                i2cSettings.autoI2c.enabled = true;
#endif
                /* HDMI i2c is always run at 100k */
                i2cSettings.clockRate = NEXUS_I2cClockRate_e100Khz;
            }
            else
            {
#if BCHP_CHIP == 7405 || BCHP_CHIP == 7125 || BCHP_CHIP == 7400 || BCHP_CHIP == 7335 || BCHP_CHIP == 7325 || \
    BCHP_CHIP == 7340 || BCHP_CHIP == 7342 || BCHP_CHIP == 7420 || BCHP_CHIP == 7468 || BCHP_CHIP == 7408
                /* config 65nm frontend i2c to be slow */
                i2cSettings.fourByteXferMode = false;
                i2cSettings.clockRate = NEXUS_I2cClockRate_e100Khz;
#else
                /* default all other frontend i2c to be fast */
                i2cSettings.fourByteXferMode = true;
                i2cSettings.clockRate = NEXUS_I2cClockRate_e400Khz;
#endif
#if NEXUS_PLATFORM_7241_T2SFF || NEXUS_PLATFORM_7241_DCSFBTSFF
                i2cSettings.interruptMode = false;
#endif
#if BCHP_CHIP == 7445
    #if NEXUS_USE_7445_SV
                if(i==3)i2cSettings.interruptMode = false;
    #else
                if(i==4)i2cSettings.interruptMode = false;
    #endif
#endif
            }

            /* To use a particular channel in soft I2c mode, set i2cSettings.softI2c = true and update NEXUS_Platform_P_GetI2CGpioHandles accordingly based on the gpio pins used.
                          Also, make sure that the pinmuxes for the clock and data gpio lines are set as gpio.
                     */
            if(i2cSettings.softI2c == true){
                NEXUS_Platform_P_GetI2CGpioHandles(i, &i2cSettings.clockGpio, &i2cSettings.dataGpio);
            }
            else{
                i2cSettings.softI2c = false;
                i2cSettings.clockGpio = NULL;
                i2cSettings.dataGpio = NULL;
            }

            BDBG_MSG(("i = %d, interruptMode = %d, autoI2c = %d, softI2c = %d, fourByteXferMode = %d, burstMode = %d",
                        i, i2cSettings.interruptMode, i2cSettings.autoI2c.enabled , i2cSettings.softI2c, i2cSettings.fourByteXferMode, i2cSettings.burstMode));
            pConfig->i2c[i] = NEXUS_I2c_Open(i, &i2cSettings);
            BDBG_MSG(("pConfig->i2c[%d] = %p", i, (void *)pConfig->i2c[i]));
            if(NULL == pConfig->i2c[i])
                continue;
        }
    }
#endif


#if NEXUS_FPGA_SUPPORT
    if ( pSettings->openFpga )
    {
        if ( !pSettings->openI2c )
        {
            BDBG_ERR(("Cannot open FPGA without opening I2C"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        BDBG_MSG(("FPGA"));
#if BCHP_CHIP == 7125
        errCode = NEXUS_Fpga_Init(pConfig->i2c[2]);
#elif BCHP_CHIP == 7468
        errCode = NEXUS_Fpga_Init(pConfig->i2c[1]);
#else
        errCode = NEXUS_Fpga_Init(pConfig->i2c[4]);
#endif
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto error;
        }
        g_fpgaInit = true;
    }
#endif

#if NEXUS_HAS_TRANSPORT
    {
    NEXUS_TransportCapabilities transportCapabilities;
    NEXUS_GetTransportCapabilities(&transportCapabilities);
    /* The 97400 has no valid signal on TS inputs */
    for ( i = 0; i<transportCapabilities.numInputBands; i++ )
    {
        NEXUS_InputBandStatus status;
        NEXUS_InputBandSettings inputBandSettings, orgSettings;

        /* check if input band supported */
        if (NEXUS_InputBand_GetStatus(i, &status)) continue;

        NEXUS_InputBand_GetSettings(i, &inputBandSettings);
        orgSettings = inputBandSettings;

#if (BCHP_CHIP == 7125)
#if (NEXUS_PLATFORM == 97125) || (NEXUS_PLATFORM == 97025)
        if (i == NEXUS_InputBand_e1)
        {   /* Correct polarity for streamer */
            inputBandSettings.clockActiveHigh= true;
        }
#endif
#endif
#if (NEXUS_PLATFORM == 97584) || (NEXUS_PLATFORM == 975845)
        if (i == NEXUS_InputBand_e9)
        {
            inputBandSettings.validEnabled = true;
            inputBandSettings.parallelInput = true;
        }
#endif
        if (BKNI_Memcmp(&inputBandSettings, &orgSettings, sizeof(inputBandSettings))) {
            /* only call setsettings if changed */
            NEXUS_InputBand_SetSettings(i, &inputBandSettings);
        }
    }
    }
#endif


#if NEXUS_HAS_DISPLAY
    if (pSettings->openOutputs)
    {
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        BDBG_CASSERT(NEXUS_NUM_COMPOSITE_OUTPUTS <= NEXUS_MAX_CONFIG_HANDLES);
        {
            NEXUS_CompositeOutputSettings compositeCfg;
            pConfig->outputs.composite[0] = NEXUS_CompositeOutput_Open(0, NULL);
            if(!pConfig->outputs.composite[0]) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_CompositeOutput, pConfig->outputs.composite[0], Create);

            NEXUS_CompositeOutput_GetSettings(pConfig->outputs.composite[0], &compositeCfg);
    #if BCHP_CHIP == 7400 || BCHP_CHIP == 7420
            compositeCfg.dac = NEXUS_VideoDac_e4;
    #elif BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7435 || \
          BCHP_CHIP == 7445 || BCHP_CHIP == 7145 || \
          (BCHP_CHIP == 7250 && defined(NEXUS_USE_7250_SV)) || \
          ((BCHP_CHIP == 7439) && (BCHP_VER >= BCHP_VER_B0))
            compositeCfg.dac = NEXUS_VideoDac_e3;
    #elif  BCHP_CHIP == 7468
    #if (NEXUS_PLATFORM == 97208)
            compositeCfg.dac = NEXUS_VideoDac_e3;
    #else
            compositeCfg.dac = NEXUS_VideoDac_e0;
    #endif
    #elif BCHP_CHIP == 7125 || BCHP_CHIP == 7408 || \
          BCHP_CHIP == 7346 || BCHP_CHIP == 7344 || BCHP_CHIP == 7231 || \
          BCHP_CHIP == 7358 || BCHP_CHIP == 7552 || BCHP_CHIP == 7360 || \
          BCHP_CHIP == 7584 || BCHP_CHIP == 7563 || BCHP_CHIP == 7362 || \
          BCHP_CHIP == 7228 || ((BCHP_CHIP == 7439) && (BCHP_VER == BCHP_VER_A0)) || \
          BCHP_CHIP == 7366 || \
          BCHP_CHIP == 7250 || BCHP_CHIP == 7364 || BCHP_CHIP == 74371 || BCHP_CHIP == 75635 || \
          BCHP_CHIP == 7586 || BCHP_CHIP == 73625 || BCHP_CHIP == 75845 || BCHP_CHIP == 73465 || \
          BCHP_CHIP == 75525
            compositeCfg.dac = NEXUS_VideoDac_e0;
    #elif BCHP_CHIP == 7429 || BCHP_CHIP==74295
            #if NEXUS_PLATFORM == 97241 || NEXUS_PLATFORM == 972415
            compositeCfg.dac = NEXUS_VideoDac_e0;
            #else
            compositeCfg.dac = NEXUS_VideoDac_e3;
            #endif
    #else
            compositeCfg.dac = NEXUS_VideoDac_e2;
    #endif

            errCode = NEXUS_CompositeOutput_SetSettings(pConfig->outputs.composite[0], &compositeCfg);
            if(errCode) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
    #if NEXUS_NUM_COMPOSITE_OUTPUTS >= 2
            pConfig->outputs.composite[1] = NEXUS_CompositeOutput_Open(1, NULL);
            if(!pConfig->outputs.composite[1]) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_CompositeOutput, pConfig->outputs.composite[1], Create);

            NEXUS_CompositeOutput_GetSettings(pConfig->outputs.composite[1], &compositeCfg);
            compositeCfg.dac = NEXUS_VideoDac_e3;
            errCode = NEXUS_CompositeOutput_SetSettings(pConfig->outputs.composite[1], &compositeCfg);
            if(errCode) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
    #endif
        }
#endif

#if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
        {
            pConfig->outputs.rfm[0] = NEXUS_Rfm_Open(0, NULL);
            if(!pConfig->outputs.rfm[0]) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_Rfm, pConfig->outputs.rfm[0], Create);
        }
#endif

#if NEXUS_NUM_COMPONENT_OUTPUTS
    BDBG_CASSERT(NEXUS_NUM_COMPONENT_OUTPUTS <= NEXUS_MAX_CONFIG_HANDLES);
        {
            NEXUS_ComponentOutputSettings componentCfg;
            pConfig->outputs.component[0] = NEXUS_ComponentOutput_Open(0, NULL);
            if(!pConfig->outputs.component[0]) { errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_ComponentOutput, pConfig->outputs.component[0], Create);

            NEXUS_ComponentOutput_GetSettings(pConfig->outputs.component[0], &componentCfg);
            componentCfg.type = NEXUS_ComponentOutputType_eYPrPb;
    #if BCHP_CHIP == 7400 || BCHP_CHIP == 7420 || BCHP_CHIP == 7422 || \
        BCHP_CHIP == 7425 || BCHP_CHIP == 7435 || BCHP_CHIP == 7445 || \
        BCHP_CHIP == 7145 || ((BCHP_CHIP == 7439) && (BCHP_VER >= BCHP_VER_B0))
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e0;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e1;
    #elif (BCHP_CHIP == 7468)  /* only on 97208. 97208 uses 7468 BCHP_CHIP*/
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e1;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e0;
    #elif BCHP_CHIP == 7125
        #if (NEXUS_PLATFORM == 97019)
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e5;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e3;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e4;
        #else
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e4;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e5;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e3;
        #endif
    #elif BCHP_CHIP==7346 || BCHP_CHIP==7231 || BCHP_CHIP == 7584 || BCHP_CHIP == 75845 || BCHP_CHIP == 73465
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e3;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e1;
    #elif  BCHP_CHIP==7344
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e1;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e3;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e2;
    #elif BCHP_CHIP == 7408
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e1;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e3;
    #elif (BCHP_CHIP == 7358) || (BCHP_CHIP == 7552) || (BCHP_CHIP == 7360) || \
          (BCHP_CHIP == 7362) || (BCHP_CHIP == 7228) || (BCHP_CHIP == 73625)
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e3;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e1;
    #elif  (BCHP_CHIP == 7429) || (BCHP_CHIP == 74295)
        #if (NEXUS_PLATFORM == 97241) || (NEXUS_PLATFORM == 972415)
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e3;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e1;
        #else
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e0;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e1;
        #endif
    #elif (BCHP_CHIP==7364) || (BCHP_CHIP==7366) || (BCHP_CHIP==7250)
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e0;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e2;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e1;
    #else
            componentCfg.dacs.YPrPb.dacY = NEXUS_VideoDac_e4;
            componentCfg.dacs.YPrPb.dacPr = NEXUS_VideoDac_e5;
            componentCfg.dacs.YPrPb.dacPb = NEXUS_VideoDac_e3;
    #endif
            errCode = NEXUS_ComponentOutput_SetSettings(pConfig->outputs.component[0], &componentCfg);
            if(errCode) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
        }
#endif

#if NEXUS_NUM_SVIDEO_OUTPUTS
        BDBG_CASSERT(NEXUS_NUM_SVIDEO_OUTPUTS <= NEXUS_MAX_CONFIG_HANDLES);
        {
            NEXUS_SvideoOutputSettings svideoCfg;
            pConfig->outputs.svideo[0] = NEXUS_SvideoOutput_Open(0, NULL);
            if(!pConfig->outputs.svideo[0]) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_SvideoOutput, pConfig->outputs.svideo[0], Create);

            NEXUS_SvideoOutput_GetSettings(pConfig->outputs.svideo[0], &svideoCfg);
    #if BCHP_CHIP == 7400
            svideoCfg.dacY = NEXUS_VideoDac_e6;
            svideoCfg.dacC = NEXUS_VideoDac_e5;
    #elif BCHP_CHIP == 7420
            svideoCfg.dacY = NEXUS_VideoDac_e6;
            svideoCfg.dacC = NEXUS_VideoDac_e5;
    #elif BCHP_CHIP == 7125
            svideoCfg.dacY = NEXUS_VideoDac_e1;
            svideoCfg.dacC = NEXUS_VideoDac_e2;
    #elif BCHP_CHIP == 7342 || BCHP_CHIP==7340
            svideoCfg.dacY = NEXUS_VideoDac_e0;
            svideoCfg.dacC = NEXUS_VideoDac_e1;
    #elif (BCHP_CHIP == 7468)  /*Svideo only on 7468 */
            svideoCfg.dacY = NEXUS_VideoDac_e1;
            svideoCfg.dacC = NEXUS_VideoDac_e2;
    #else
            svideoCfg.dacY = NEXUS_VideoDac_e0;
            svideoCfg.dacC = NEXUS_VideoDac_e1;
    #endif
            errCode = NEXUS_SvideoOutput_SetSettings(pConfig->outputs.svideo[0], &svideoCfg);
            if(errCode) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
        }
#endif

#if NEXUS_NUM_656_OUTPUTS
        BDBG_CASSERT(NEXUS_NUM_656_OUTPUTS <= NEXUS_MAX_CONFIG_HANDLES);
        for ( i = 0; i < NEXUS_NUM_656_OUTPUTS; i++ )
        {
            pConfig->outputs.ccir656[i] = NEXUS_Ccir656Output_Open(i, NULL);
            if(!pConfig->outputs.ccir656[i]) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_Ccir656Output, pConfig->outputs.ccir656[i], Create);
        }
#endif

#if NEXUS_NUM_AUDIO_DACS && NEXUS_HAS_AUDIO
        for ( i = 0; i < NEXUS_NUM_AUDIO_DACS; i++ )
        {
            pConfig->outputs.audioDacs[i] = NEXUS_AudioDac_Open(i, NULL);
            if ( NULL == pConfig->outputs.audioDacs[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_AudioDac, pConfig->outputs.audioDacs[i], Create);
        }
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS && NEXUS_HAS_AUDIO
        for ( i = 0; i < NEXUS_NUM_SPDIF_OUTPUTS; i++ )
        {
            pConfig->outputs.spdif[i] = NEXUS_SpdifOutput_Open(i, NULL);
            if ( NULL == pConfig->outputs.spdif[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_SpdifOutput, pConfig->outputs.spdif[i], Create);
        }
#endif

#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS && NEXUS_HAS_AUDIO
        for ( i = 0; i < NEXUS_NUM_AUDIO_DUMMY_OUTPUTS; i++ )
        {
            pConfig->outputs.audioDummy[i] = NEXUS_AudioDummyOutput_Open(i, NULL);
            if ( NULL == pConfig->outputs.audioDummy[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_AudioDummyOutput, pConfig->outputs.audioDummy[i], Create);
        }
#endif

#if NEXUS_NUM_I2S_OUTPUTS && NEXUS_HAS_AUDIO
        for ( i = 0; i < NEXUS_NUM_I2S_OUTPUTS; i++ )
        {
            pConfig->outputs.i2s[i] = NEXUS_I2sOutput_Open(i, NULL);
            if ( NULL == pConfig->outputs.i2s[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_I2sOutput, pConfig->outputs.i2s[i], Create);
        }
#if NEXUS_NUM_I2S_MULTI_OUTPUTS && NEXUS_HAS_AUDIO
        for ( i = 0; i < NEXUS_NUM_I2S_MULTI_OUTPUTS; i++ )
        {
            pConfig->outputs.i2sMulti[i] = NEXUS_I2sMultiOutput_Open(i, NULL);
            if ( NULL == pConfig->outputs.i2sMulti[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_I2sMultiOutput, pConfig->outputs.i2sMulti[i], Create);
        }
#endif
#endif

#if NEXUS_HAS_HDMI_OUTPUT
        if ( pSettings->openI2c )
        {

            for(i=0 ; i < NEXUS_NUM_HDMI_OUTPUTS ; i++)
            {

                NEXUS_HdmiOutputOpenSettings hdmiSettings ;
                NEXUS_HdmiOutput_GetDefaultOpenSettings(&hdmiSettings);

#if BCHP_CHIP==7439 && (BCHP_VER == BCHP_VER_A0)
              if (i==1) {
                 hdmiSettings.i2c = pConfig->i2c[NEXUS_HDMI_OUTPUT_I2C_CHANNEL_1];
              } else
#endif
              hdmiSettings.i2c = pConfig->i2c[NEXUS_HDMI_OUTPUT_I2C_CHANNEL];
              hdmiSettings.hotplugChangeThreshold = 50 ; /* # of HP Intrs in 1s that will disable HPD interrupts */
                                                                            /* threshold of 0 disables excessive hotplugCheck */

              /* SPD Infoframe settings */
              hdmiSettings.spd.deviceType = NEXUS_HdmiSpdSourceDeviceType_eDigitalStb ;

              BKNI_Snprintf((char *) &hdmiSettings.spd.vendorName, NEXUS_HDMI_SPD_VENDOR_NAME_MAX+1, "Broadcom") ;
              BKNI_Snprintf((char *) &hdmiSettings.spd.description, NEXUS_HDMI_SPD_DESCRIPTION_MAX+1, "STB Refsw Design") ;

              pConfig->outputs.hdmi[i] = NEXUS_HdmiOutput_Open(i, &hdmiSettings);
              if ( NULL == pConfig->outputs.hdmi[i] )
              {
                 errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                 goto error;
              }
              NEXUS_OBJECT_REGISTER(NEXUS_HdmiOutput, pConfig->outputs.hdmi[i], Create);
            }
#if DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT
            if (pConfig->outputs.hdmi[0]) {
                NEXUS_Module_Lock(g_NEXUS_platformHandles.display);
                NEXUS_DisplayModule_AddRequiredOutput_priv(NEXUS_HdmiOutput_GetVideoConnector(pConfig->outputs.hdmi[0]));
                NEXUS_Module_Unlock(g_NEXUS_platformHandles.display);
            }
#endif
        }
        else
        {
            BDBG_WRN(("Cannot open HDMI without opening I2C"));
        }
#endif
#if NEXUS_NUM_HDMI_DVO
        {
            NEXUS_HdmiDvoSettings hdmiDvoSettings;
            NEXUS_HdmiDvo_GetDefaultSettings(&hdmiDvoSettings);
            pConfig->outputs.hdmiDvo[0] = NEXUS_HdmiDvo_Open(0, &hdmiDvoSettings);
            if ( NULL == pConfig->outputs.hdmiDvo[0] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_HdmiDvo, pConfig->outputs.hdmiDvo[0], Create);
        }
#endif

#if NEXUS_HAS_CEC && NEXUS_NUM_CEC
        if (pSettings->openCec) {
            NEXUS_CecSettings cecSettings;
            NEXUS_Cec_GetDefaultSettings(&cecSettings);
            /* Open but disable CEC core by default. The app can enable CEC when needed */
            cecSettings.enabled = false;
#if (NEXUS_PLATFORM == 97429) || (NEXUS_PLATFORM == 97428) || (NEXUS_PLATFORM == 974295) || (NEXUS_PLATFORM == 974285)
            BDBG_MSG(("Default to NEXUS_CecController_eRx for 7429/7428 platform due to CEC wake up issue in S3 mode "));
            cecSettings.cecController = NEXUS_CecController_eRx;
#endif
            pConfig->outputs.cec[0] = NEXUS_Cec_Open(0, &cecSettings);
            if (NULL == pConfig->outputs.cec[0])
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_Cec, pConfig->outputs.cec[0], Create);
        }
#endif
        }
#endif /* NEXUS_HAS_DISPLAY */

#if NEXUS_HAS_EXTERNAL_RFM
    errCode = NEXUS_Platform_P_InitExternalRfm(pConfig);
    if(errCode != NEXUS_SUCCESS) { BERR_TRACE(errCode); goto error; }
#endif

    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        pConfig->heap[i] = g_pCoreHandles->heap[i].nexus;
    }
#if NEXUS_HAS_DISPLAY
    {
        NEXUS_DisplayCapabilities *cap;
        cap = BKNI_Malloc(sizeof(NEXUS_DisplayCapabilities));
        if (cap == NULL) { errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
        NEXUS_GetDisplayCapabilities(cap);
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            pConfig->supportedDisplay[i] = cap->display[i].numVideoWindows > 0;
        }
        pConfig->numWindowsPerDisplay = cap->display[0].numVideoWindows;
        BKNI_Free(cap);
    }
#endif

#if NEXUS_HAS_VIDEO_DECODER
    {
        NEXUS_VideoDecoderCapabilities *cap;
        cap = BKNI_Malloc(sizeof(NEXUS_VideoDecoderCapabilities));
        if (cap == NULL) { errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
        NEXUS_GetVideoDecoderCapabilities(cap);
        for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
            pConfig->supportedDecoder[i] = i < cap->numVideoDecoders;
        }
        BKNI_Free(cap);
    }
#endif

#if (NEXUS_USE_7250_CWF || NEXUS_USE_7250_SV) && NEXUS_HAS_AUDIO
    /* Enable external MCLK signal for audio DAC on 7250_CWF board */
    (void)NEXUS_AudioModule_EnableExternalMclk(1, NEXUS_AudioOutputPll_e0, NEXUS_ExternalMclkRate_e256Fs);
#elif (NEXUS_USE_7250_ACX16 || NEXUS_USE_7250_USFF || NEXUS_USE_7250_CD2) && NEXUS_HAS_AUDIO
    (void)NEXUS_AudioModule_EnableExternalMclk(0, NEXUS_AudioOutputPll_e0, NEXUS_ExternalMclkRate_e256Fs);
#endif

    NEXUS_Platform_P_StartMonitor();

    BDBG_MSG(("NEXUS_Platform_P_Config<< DONE"));
    return BERR_SUCCESS;

error:
    NEXUS_Platform_P_Shutdown();
    return errCode;
}

void NEXUS_Platform_P_Shutdown(void)
{
    unsigned i;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;

    NEXUS_Platform_P_StopMonitor();

#if NEXUS_MAX_I2C_CHANNELS
#ifdef NEXUS_FPGA_SUPPORT
    if (g_fpgaInit) {
        NEXUS_Fpga_Uninit();
        g_fpgaInit = false;
    }
#endif
#endif

#if NEXUS_HAS_DISPLAY
#if NEXUS_NUM_I2S_OUTPUTS && NEXUS_HAS_AUDIO
    for ( i = 0; i < NEXUS_NUM_I2S_OUTPUTS; i++ )
    {
        if ( NULL != pConfig->outputs.i2s[i] )
        {
            NEXUS_AudioOutput_Shutdown(NEXUS_I2sOutput_GetConnector(pConfig->outputs.i2s[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_I2sOutput, pConfig->outputs.i2s[i], Destroy);
            NEXUS_I2sOutput_Close(pConfig->outputs.i2s[i]);
            pConfig->outputs.i2s[i] = NULL;
        }
    }

#if NEXUS_NUM_I2S_MULTI_OUTPUTS && NEXUS_HAS_AUDIO
        for ( i = 0; i < NEXUS_NUM_I2S_MULTI_OUTPUTS; i++ )
        {
            if ( NULL != pConfig->outputs.i2sMulti[i] )
            {
                NEXUS_AudioOutput_Shutdown(NEXUS_I2sMultiOutput_GetConnector(pConfig->outputs.i2sMulti[i]));
                NEXUS_OBJECT_UNREGISTER(NEXUS_I2sMultiOutput, pConfig->outputs.i2sMulti[i], Destroy);
                NEXUS_I2sMultiOutput_Close(pConfig->outputs.i2sMulti[i]);
                pConfig->outputs.i2sMulti[i] = NULL;
            }
        }
#endif

#endif
#if NEXUS_NUM_SPDIF_OUTPUTS && NEXUS_HAS_AUDIO
    for ( i = 0; i < NEXUS_NUM_SPDIF_OUTPUTS; i++ ) {
        if (pConfig->outputs.spdif[i]) {
            NEXUS_AudioOutput_Shutdown(NEXUS_SpdifOutput_GetConnector(pConfig->outputs.spdif[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_SpdifOutput, pConfig->outputs.spdif[i], Destroy);
            NEXUS_SpdifOutput_Close(pConfig->outputs.spdif[i]);
            pConfig->outputs.spdif[i] = NULL;
        }
    }
#endif
#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS && NEXUS_HAS_AUDIO
    for ( i = 0; i < NEXUS_NUM_AUDIO_DUMMY_OUTPUTS; i++ ) {
        if (pConfig->outputs.audioDummy[i]) {
            NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(pConfig->outputs.audioDummy[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_AudioDummyOutput, pConfig->outputs.audioDummy[i], Destroy);
            NEXUS_AudioDummyOutput_Close(pConfig->outputs.audioDummy[i]);
            pConfig->outputs.audioDummy[i] = NULL;
        }
    }
#endif
#if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
    if (pConfig->outputs.rfm[0]) {
        #if NEXUS_HAS_AUDIO
        NEXUS_AudioOutput_Shutdown(NEXUS_Rfm_GetAudioConnector(pConfig->outputs.rfm[0]));
        #endif
        NEXUS_OBJECT_UNREGISTER(NEXUS_Rfm, pConfig->outputs.rfm[0], Destroy);
        NEXUS_Rfm_Close(pConfig->outputs.rfm[0]);
        pConfig->outputs.rfm[0] = NULL;
    }
#endif
#if NEXUS_NUM_AUDIO_DACS && NEXUS_HAS_AUDIO
    for ( i = 0; i < NEXUS_NUM_AUDIO_DACS; i++ ) {
        if (pConfig->outputs.audioDacs[i]) {
            NEXUS_AudioOutput_Shutdown(NEXUS_AudioDac_GetConnector(pConfig->outputs.audioDacs[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_AudioDac, pConfig->outputs.audioDacs[i], Destroy);
            NEXUS_AudioDac_Close(pConfig->outputs.audioDacs[i]);
            pConfig->outputs.audioDacs[i] = NULL;
        }
    }
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS
    for ( i = 0; i < NEXUS_NUM_SVIDEO_OUTPUTS; i++ ) {
        if (pConfig->outputs.svideo[i]) {
            NEXUS_VideoOutput_Shutdown(NEXUS_SvideoOutput_GetConnector(pConfig->outputs.svideo[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_SvideoOutput, pConfig->outputs.svideo[i], Destroy);
            NEXUS_SvideoOutput_Close(pConfig->outputs.svideo[i]);
        }
    }
#endif
#if NEXUS_NUM_656_OUTPUTS
    for ( i = 0; i < NEXUS_NUM_656_OUTPUTS; i++ ) {
        if (pConfig->outputs.ccir656[i]) {
            NEXUS_VideoOutput_Shutdown(NEXUS_Ccir656Output_GetConnector(pConfig->outputs.ccir656[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_Ccir656Output, pConfig->outputs.ccir656[i], Destroy);
            NEXUS_Ccir656Output_Close(pConfig->outputs.ccir656[i]);
            pConfig->outputs.ccir656[i] = NULL;
        }
    }
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    for ( i = 0; i < NEXUS_NUM_COMPONENT_OUTPUTS; i++ ) {
        if (pConfig->outputs.component[i]) {
            NEXUS_VideoOutput_Shutdown(NEXUS_ComponentOutput_GetConnector(pConfig->outputs.component[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_ComponentOutput, pConfig->outputs.component[i], Destroy);
            NEXUS_ComponentOutput_Close(pConfig->outputs.component[i]);
            pConfig->outputs.component[i] = NULL;
        }
    }
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    for ( i = 0; i < NEXUS_NUM_COMPOSITE_OUTPUTS; i++ ) {
        if (pConfig->outputs.composite[i]) {
            NEXUS_VideoOutput_Shutdown(NEXUS_CompositeOutput_GetConnector(pConfig->outputs.composite[i]));
            NEXUS_OBJECT_UNREGISTER(NEXUS_CompositeOutput, pConfig->outputs.composite[i], Destroy);
            NEXUS_CompositeOutput_Close(pConfig->outputs.composite[i]);
            pConfig->outputs.composite[i] = NULL;
        }
    }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#if DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT
    if (pConfig->outputs.hdmi[0]) {
        NEXUS_Module_Lock(g_NEXUS_platformHandles.display);
        NEXUS_DisplayModule_RemoveRequiredOutput_priv(NEXUS_HdmiOutput_GetVideoConnector(pConfig->outputs.hdmi[0]));
        NEXUS_Module_Unlock(g_NEXUS_platformHandles.display);
    }
#endif
    for ( i = 0; i < NEXUS_NUM_HDMI_OUTPUTS; i++ ) {
        if (pConfig->outputs.hdmi[i]) {
            #if NEXUS_HAS_VIDEO_DECODER
            NEXUS_VideoOutput_Shutdown(NEXUS_HdmiOutput_GetVideoConnector(pConfig->outputs.hdmi[i]));
            #endif
            #if NEXUS_HAS_AUDIO
            NEXUS_AudioOutput_Shutdown(NEXUS_HdmiOutput_GetAudioConnector(pConfig->outputs.hdmi[i]));
            #endif
            NEXUS_OBJECT_UNREGISTER(NEXUS_HdmiOutput, pConfig->outputs.hdmi[i], Destroy);
            NEXUS_HdmiOutput_Close(pConfig->outputs.hdmi[i]);
            pConfig->outputs.hdmi[i] = NULL;
        }
    }
#endif
#endif /* NEXUS_HAS_DISPLAY */

#if NEXUS_HAS_CEC && NEXUS_NUM_CEC
        if (pConfig->outputs.cec[0])
        {
            NEXUS_OBJECT_UNREGISTER(NEXUS_Cec, pConfig->outputs.cec[0], Destroy);
            NEXUS_Cec_Close(pConfig->outputs.cec[0]);
            pConfig->outputs.cec[0] = NULL;
        }
#endif

#if NEXUS_HAS_I2C
    for ( i = 0; i < NEXUS_MAX_I2C_CHANNELS; i++ ) {
        if (pConfig->i2c[i]) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_I2c, pConfig->i2c[i], Destroy);
            NEXUS_I2c_Close(pConfig->i2c[i]);
            BDBG_MSG(("pConfig->i2c[%d] = %p", i, (void *)pConfig->i2c[i]));
            pConfig->i2c[i] = NULL;
        }
    }
#endif
}

static NEXUS_TimerHandle g_platformTimer;

#define NEXUS_MODULE_NAME platform

#if NEXUS_HAS_VIDEO_DECODER
BDBG_FILE_MODULE(AVD);
#include "nexus_video_decoder_extra.h"
#endif

static void NEXUS_Platform_P_Timer(void *context)
{
#if NEXUS_HAS_VIDEO_DECODER
    const char *avd_monitor = NEXUS_GetEnv("avd_monitor");
#endif
    BSTD_UNUSED(context);

#if NEXUS_HAS_VIDEO_DECODER
    if (avd_monitor) {
        unsigned avdCore = NEXUS_atoi(avd_monitor);
        char buf[256];
        unsigned n;
        static bool init = false;

        if (!init) {
            if (!NEXUS_VideoDecoderModule_SetDebugLog(avdCore, true)) {
                init = true;
            }
        }
        if (init) {
            while (!NEXUS_VideoDecoderModule_ReadDebugLog(avdCore, buf, 255 /* one less */, &n) && n) {
                char *cur = buf;
                buf[n] = 0;

                /* print header for each line. makes for easier grep of logs. */
                while (*cur) {
                    char *next = cur;
                    bool more = false;
                    while (*next && (*next != '\n')) next++;
                    if (*next) {
                        *next = 0;
                        more = true;
                    }
                    BDBG_MODULE_LOG(AVD,("%d: %s", avdCore, cur));
                    if (more)
                        cur = ++next;
                    else
                        break;
                }
            }
        }
    }
#endif

    NEXUS_Platform_P_MonitorOS();

    g_platformTimer = NEXUS_ScheduleTimer(1000, NEXUS_Platform_P_Timer, NULL);
}

static void NEXUS_Platform_P_StartMonitor(void)
{
    g_platformTimer = NEXUS_ScheduleTimer(1000, NEXUS_Platform_P_Timer, NULL);
}

static void NEXUS_Platform_P_StopMonitor(void)
{
    if (g_platformTimer) {
        NEXUS_CancelTimer(g_platformTimer);
        g_platformTimer = NULL;
    }
}

#if BDBG_DEBUG_BUILD && !defined(NEXUS_CPU_ARM)
#include "bchp_sun_gisb_arb.h"
#include "bchp_sun_l2.h"
#define BCHP_INT_ID_GISB_TIMEOUT_INTR         BCHP_INT_ID_CREATE(BCHP_SUN_L2_CPU_STATUS, BCHP_SUN_L2_CPU_STATUS_GISB_TIMEOUT_INTR_SHIFT)

static BINT_CallbackHandle g_gisbInterrupt;

#if BCHP_CHIP == 7422 || BCHP_CHIP == 7425
static const char *g_pGisbMasters[] =
{
    "SSP",
    "CPU",
    "reserved",
    "pcie",
    "BSP",
    "RDC",
    "raaga",
    "reserved",
    "AVD1",
    "JTAG",
    "SVD0",
    "reserved",
    "VICE0",
    "reserved"
};
#elif BCHP_CHIP == 7435
static const char *g_pGisbMasters[] =
{
    "SSP",
    "CPU",
    "webCPU",
    "pcie",
    "BSP",
    "RDC",
    "raaga_0",
    "reserved",
    "AVD1",
    "JTAG",
    "SVD0",
    "reserved",
    "VICE0",
    "VICE_1",
    "raaga_1",
    "reserved"
};
#elif BCHP_CHIP == 7344 || BCHP_CHIP==7346 || BCHP_CHIP==7231 || BCHP_CHIP==7429 || BCHP_CHIP==74295 || BCHP_CHIP == 73465
static const char *g_pGisbMasters[] =
{
    "SSP",
    "CPU",
    "reserved",
    "BSP",
    "RDC",
    "raaga",
    "reserved",
    "JTAG",
    "SVD0",
    "reserved"
};
#elif BCHP_CHIP == 7358 || BCHP_CHIP == 7552 || BCHP_CHIP == 7360 || BCHP_CHIP == 7362 || \
      BCHP_CHIP == 7228 || BCHP_CHIP == 73625
static const char *g_pGisbMasters[] =
{
    "SSP",
    "CPU",
    "reserved",
    "reserved",
    "BSP",
    "RDC",
    "raaga",
    "AVD0",
    "reserved",
    "JTAG",
    "reserved"
};
#elif BCHP_CHIP == 7584 || BCHP_CHIP == 7563 || BCHP_CHIP == 75635 || BCHP_CHIP == 75845 || BCHP_CHIP == 75525
static const char *g_pGisbMasters[] =
{
    "SSP",
    "CPU",
    "reserved",
    "reserved",
    "BSP",
    "RDC",
    "raaga",
    "AVD0",
    "reserved",
    "JTAG",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "SCPU",
    "LEAP",
    "reserved"
};
#else
static const char *g_pGisbMasters[] =
{
    "SSB",
    "CPU",
    "PCI",
    "reserved",
    "BSP",
    "RDC",
    "RPTD",
    "AVD0",
    "reserved",
    "JTAG",
    "reserved"
};
#endif

static void NEXUS_Platform_P_GisbTimeout_isr(void *pParam, int iParam)
{
    uint32_t reg;
    BREG_Handle regHandle = pParam;
    reg = BREG_Read32(regHandle, BCHP_SUN_GISB_ARB_ERR_CAP_STATUS);
    if ( reg & BCHP_MASK(SUN_GISB_ARB_ERR_CAP_STATUS, valid) )
    {
        bool writing = (reg & BCHP_MASK(SUN_GISB_ARB_ERR_CAP_STATUS, write))?true:false;
        const char *pCore = "Unknown";
        unsigned i;
        reg = BREG_Read32(regHandle, BCHP_SUN_GISB_ARB_ERR_CAP_MASTER);
        for ( i = 0; i < sizeof(g_pGisbMasters)/sizeof(const char *); i++ )
        {
            if ( reg & 0x1 )
            {
                pCore = g_pGisbMasters[i];
                break;
            }
            reg >>= 1;
        }
        reg = BREG_Read32(regHandle, BCHP_SUN_GISB_ARB_ERR_CAP_ADDR);
        BSTD_UNUSED(writing);
        BDBG_ERR(("*****************************************************"));
        BDBG_ERR(("GISB Timeout %s addr 0x%08x by core %s", (writing)?"writing":"reading", reg, pCore));
        if (iParam) {
            BDBG_ERR(("This GISB timeout occurred before nexus started."));
        }
        BDBG_ERR(("*****************************************************"));
    }
    /* Clear error status */
    BREG_Write32(regHandle, BCHP_SUN_GISB_ARB_ERR_CAP_CLR, 0x1);

}

void NEXUS_Platform_P_ConfigureGisbTimeout(void)
{
    BERR_Code rc;

    /* This can produce a lot of console output if a block is failing. */
    /* Reduce GISB ARB timer to a small enough value for the CPU to trap errors.
    100 milliseconds is preferred by the HW architecture team. */
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_TIMER, 162000);    /* 1.5 milliseconds @ 108 MHz */
    /* INT is valid now.  Register for GISB interrupt to see if anyone is accessing invalid registers */
    rc = BINT_CreateCallback(&g_gisbInterrupt, g_pCoreHandles->bint, BCHP_INT_ID_GISB_TIMEOUT_INTR, NEXUS_Platform_P_GisbTimeout_isr, g_pCoreHandles->reg, 0);
    if (rc) {
        rc = BERR_TRACE(rc);
    }
    else {
        /* Poll for a pending bad access */
        BKNI_EnterCriticalSection();
        NEXUS_Platform_P_GisbTimeout_isr(g_pCoreHandles->reg, 1);
        BKNI_LeaveCriticalSection();
        /* Enable the interrupt */
        rc = BINT_EnableCallback(g_gisbInterrupt);
        if (rc) {
             rc = BERR_TRACE(rc);
        }
    }
}
#else
void NEXUS_Platform_P_ConfigureGisbTimeout(void)
{
}
#endif /* BDBG_DEBUG_BUILD && !defined(NEXUS_CPU_ARM) */
