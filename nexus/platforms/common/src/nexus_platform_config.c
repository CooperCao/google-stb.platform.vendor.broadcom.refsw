/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#if NEXUS_HAS_AUDIO
#include "nexus_audio_output.h"
#include "priv/nexus_audio_output_priv.h"
#endif
#if NEXUS_HAS_GPIO
#include "nexus_gpio.h"
#endif

BDBG_MODULE(nexus_platform_config);
static void NEXUS_Platform_P_StartMonitor(void);
static void NEXUS_Platform_P_StopMonitor(void);

#ifndef NEXUS_I2C_CHANNEL_HDMI_TX
/* define in nexus_platform_features.h */
#define NEXUS_I2C_CHANNEL_HDMI_TX 0
#endif

#if NEXUS_USE_7425_SV_BOARD || NEXUS_PLATFORM_7418SFF_H
#define NEXUS_HAS_EXTERNAL_RFM       (true)
#endif

#if NEXUS_HAS_EXTERNAL_RFM
extern NEXUS_Error NEXUS_Platform_P_InitExternalRfm(const NEXUS_PlatformConfiguration *pConfig);
#endif

static void NEXUS_Platform_P_StartMonitor(void);
static void NEXUS_Platform_P_StopMonitor(void);

#include "nexus_platform_config_custom.inc"

NEXUS_Error NEXUS_Platform_P_Config(const NEXUS_PlatformSettings *pSettings)
{
    NEXUS_Error errCode=NEXUS_SUCCESS;
    unsigned i;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    union {
        /* name of union loosly matches scope where variable accessed, this way we don't allocate excessively large memory block */
#if NEXUS_HAS_DISPLAY
        struct {
            NEXUS_DisplayCapabilities cap;
        } has_display;
#endif
#if NEXUS_HAS_VIDEO_DECODER
        struct {
            NEXUS_VideoDecoderCapabilities cap;
        } has_video_decoder;
#endif
#if NEXUS_HAS_AUDIO
        struct {
            NEXUS_AudioCapabilities audioCapabilities;
        } has_audio;
#endif
#if NEXUS_HAS_TRANSPORT
        struct {
            NEXUS_TransportCapabilities transportCapabilities;
            NEXUS_InputBandStatus status;
            NEXUS_InputBandSettings inputBandSettings;
            NEXUS_InputBandSettings orgSettings;
        } has_transport;
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
        struct {
            NEXUS_ComponentOutputSettings componentCfg;
        } num_component_outputs;
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        struct {
            NEXUS_CompositeOutputSettings compositeCfg;
        } num_composite_outputs;
#endif
#if NEXUS_HAS_I2C
        struct {
            NEXUS_I2cSettings i2cSettings;
#if NEXUS_HAS_GPIO
            NEXUS_GpioSettings gpioSettings;
#endif
        } has_i2c;
#endif
#if NEXUS_HAS_HDMI_OUTPUT
        struct {
            NEXUS_HdmiOutputOpenSettings openSettings;
        } has_hdmi_output;
#endif
    } *data;

    data = BKNI_Malloc(sizeof(*data));
    if(data==NULL) {
        goto err_malloc;
    }

#if NEXUS_HAS_I2C
    BDBG_CASSERT(NEXUS_MAX_I2C_CHANNELS <= NEXUS_MAX_CONFIG_HANDLES);
    /* Open I2C Channels */
    if (pSettings->openI2c)
    {
        NEXUS_I2cSettings *i2cSettings = &data->has_i2c.i2cSettings;
        for ( i = 0; i < NEXUS_MAX_I2C_CHANNELS; i++ )
        {
            if (pSettings->i2c[i].open)
            {
                *i2cSettings = pSettings->i2c[i].settings;

                if ( i == NEXUS_I2C_CHANNEL_HDMI_TX )
                {
                    const char *hdmi_i2c_software_mode = NEXUS_GetEnv("hdmi_i2c_software_mode");
                    #if NEXUS_HAS_HDMI_SOFTI2C_SUPPORT
                    bool forceHardI2c = (hdmi_i2c_software_mode && (hdmi_i2c_software_mode[0] == 'n' ||
                                                                    hdmi_i2c_software_mode[0] == 'N'));
                    i2cSettings->softI2c = !forceHardI2c;
                    #else
                    bool forceSoftI2c = (hdmi_i2c_software_mode && (hdmi_i2c_software_mode[0] == 'y' ||
                                                                    hdmi_i2c_software_mode[0] == 'Y'));
                    i2cSettings->softI2c = forceSoftI2c;
                    #endif
                    i2cSettings->clockRate = NEXUS_I2cClockRate_e100Khz;
                }

                if(i2cSettings->softI2c == true){
#if NEXUS_HAS_GPIO
                    NEXUS_GpioSettings *gpioSettings = &data->has_i2c.gpioSettings;

                    NEXUS_Gpio_GetDefaultSettings(pSettings->i2c[i].clock.type, gpioSettings);
                    i2cSettings->clockGpio = NEXUS_Gpio_Open(pSettings->i2c[i].clock.type, pSettings->i2c[i].clock.gpio, gpioSettings);
                    if(i2cSettings->clockGpio == NULL) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }

                    NEXUS_Gpio_GetDefaultSettings(pSettings->i2c[i].data.type, gpioSettings);
                    i2cSettings->dataGpio = NEXUS_Gpio_Open(pSettings->i2c[i].data.type, pSettings->i2c[i].data.gpio, gpioSettings);
                    if(i2cSettings->dataGpio == NULL) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
#endif
                }

                pConfig->i2c[i] = NEXUS_I2c_Open(i, i2cSettings);
            }
        }
    }
#endif

#if NEXUS_HAS_TRANSPORT
    {
    NEXUS_TransportCapabilities *transportCapabilities = &data->has_transport.transportCapabilities;
    NEXUS_GetTransportCapabilities(transportCapabilities);
    for ( i = 0; i<transportCapabilities->numInputBands; i++ )
    {
        NEXUS_InputBandStatus *status = &data->has_transport.status;
        NEXUS_InputBandSettings *inputBandSettings = &data->has_transport.inputBandSettings;
        NEXUS_InputBandSettings *orgSettings = &data->has_transport.orgSettings;

        /* check if input band supported */
        if (NEXUS_InputBand_GetStatus(i, status)) continue;

        NEXUS_InputBand_GetSettings(i, inputBandSettings);
        *orgSettings = *inputBandSettings;

#if (NEXUS_PLATFORM == 97584) || (NEXUS_PLATFORM == 975845)
        if (i == NEXUS_InputBand_e9)
        {
            inputBandSettings->validEnabled = true;
            inputBandSettings->parallelInput = true;
        }
#endif
        if (BKNI_Memcmp(inputBandSettings, orgSettings, sizeof(*inputBandSettings))) {
            /* only call setsettings if changed */
            NEXUS_InputBand_SetSettings(i, inputBandSettings);
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
            NEXUS_CompositeOutputSettings *compositeCfg = &data->num_composite_outputs.compositeCfg;
            pConfig->outputs.composite[0] = NEXUS_CompositeOutput_Open(0, NULL);
            if(!pConfig->outputs.composite[0]) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_CompositeOutput, pConfig->outputs.composite[0], Create);

            NEXUS_CompositeOutput_GetSettings(pConfig->outputs.composite[0], compositeCfg);
            compositeCfg->dac = g_NEXUS_compositeDacs[0];
            errCode = NEXUS_CompositeOutput_SetSettings(pConfig->outputs.composite[0], compositeCfg);
            if(errCode) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
        }
#endif

#if NEXUS_HAS_RFM && NEXUS_NUM_RFM_OUTPUTS
        {
            bool rfmCapable;
            if((BCHP_GetFeature(g_pCoreHandles->chp, BCHP_Feature_eRfmCapable, (void*) &rfmCapable) == NEXUS_SUCCESS) && rfmCapable) {
                pConfig->outputs.rfm[0] = NEXUS_Rfm_Open(0, NULL);
                if(!pConfig->outputs.rfm[0]) {errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
                NEXUS_OBJECT_REGISTER(NEXUS_Rfm, pConfig->outputs.rfm[0], Create);
            }
        }
#endif

#if NEXUS_NUM_COMPONENT_OUTPUTS
    BDBG_CASSERT(NEXUS_NUM_COMPONENT_OUTPUTS <= NEXUS_MAX_CONFIG_HANDLES);
        {
            NEXUS_ComponentOutputSettings *componentCfg = &data->num_component_outputs.componentCfg;
            pConfig->outputs.component[0] = NEXUS_ComponentOutput_Open(0, NULL);
            if(!pConfig->outputs.component[0]) { errCode = BERR_TRACE(BERR_NOT_SUPPORTED); goto error; }
            NEXUS_OBJECT_REGISTER(NEXUS_ComponentOutput, pConfig->outputs.component[0], Create);

            NEXUS_ComponentOutput_GetSettings(pConfig->outputs.component[0], componentCfg);
            componentCfg->type = NEXUS_ComponentOutputType_eYPrPb;
            componentCfg->dacs.YPrPb.dacY = g_NEXUS_componentDacs[0].dacY;
            componentCfg->dacs.YPrPb.dacPr = g_NEXUS_componentDacs[0].dacPr;
            componentCfg->dacs.YPrPb.dacPb = g_NEXUS_componentDacs[0].dacPb;
            errCode = NEXUS_ComponentOutput_SetSettings(pConfig->outputs.component[0], componentCfg);
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
            svideoCfg.dacY = NEXUS_VideoDac_e0;
            svideoCfg.dacC = NEXUS_VideoDac_e1;
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

#if NEXUS_HAS_AUDIO
    {
        NEXUS_AudioCapabilities *audioCapabilities = &data->has_audio.audioCapabilities;
        NEXUS_GetAudioCapabilities(audioCapabilities);

#if NEXUS_NUM_AUDIO_DACS
        BDBG_CASSERT(NEXUS_NUM_AUDIO_DACS <= sizeof(pConfig->outputs.audioDacs)/sizeof(pConfig->outputs.audioDacs[0]));
#endif
        for ( i = 0; i < audioCapabilities->numOutputs.dac; i++ )
        {
            pConfig->outputs.audioDacs[i] = NEXUS_AudioDac_Open(i, NULL);
            if ( NULL == pConfig->outputs.audioDacs[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_AudioDac, pConfig->outputs.audioDacs[i], Create);
        }

#if NEXUS_NUM_SPDIF_OUTPUTS
        BDBG_CASSERT(NEXUS_NUM_SPDIF_OUTPUTS <= sizeof(pConfig->outputs.spdif)/sizeof(pConfig->outputs.spdif[0]));
#endif
        for ( i = 0; i < audioCapabilities->numOutputs.spdif; i++ )
        {
            pConfig->outputs.spdif[i] = NEXUS_SpdifOutput_Open(i, NULL);
            if ( NULL == pConfig->outputs.spdif[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_SpdifOutput, pConfig->outputs.spdif[i], Create);
        }

#if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
        BDBG_CASSERT(NEXUS_NUM_AUDIO_DUMMY_OUTPUTS <= sizeof(pConfig->outputs.audioDummy)/sizeof(pConfig->outputs.audioDummy[0]));
#endif
        for ( i = 0; i < audioCapabilities->numOutputs.dummy; i++ )
        {
            pConfig->outputs.audioDummy[i] = NEXUS_AudioDummyOutput_Open(i, NULL);
            if ( NULL == pConfig->outputs.audioDummy[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_AudioDummyOutput, pConfig->outputs.audioDummy[i], Create);
        }

#if NEXUS_NUM_I2S_OUTPUTS
        BDBG_CASSERT(NEXUS_NUM_I2S_OUTPUTS<= sizeof(pConfig->outputs.i2s)/sizeof(pConfig->outputs.i2s[0]));
#endif
        for ( i = 0; i < audioCapabilities->numOutputs.i2s; i++ )
        {
            pConfig->outputs.i2s[i] = NEXUS_I2sOutput_Open(i, NULL);
            if ( NULL == pConfig->outputs.i2s[i] )
            {
                errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto error;
            }
            NEXUS_OBJECT_REGISTER(NEXUS_I2sOutput, pConfig->outputs.i2s[i], Create);
        }
    }
#endif /* NEXUS_HAS_AUDIO */

#if NEXUS_HAS_HDMI_OUTPUT
        if ( pSettings->openI2c )
        {
BDBG_CASSERT(NEXUS_NUM_HDMI_OUTPUTS <= NEXUS_MAX_HDMI_OUTPUTS);
            for(i=0 ; i < NEXUS_NUM_HDMI_OUTPUTS ; i++)
            {
                NEXUS_HdmiOutputOpenSettings *openSettings = &data->has_hdmi_output.openSettings;
                *openSettings = pSettings->hdmiOutputOpenSettings[i];
                openSettings->i2c = pConfig->i2c[NEXUS_I2C_CHANNEL_HDMI_TX];
                pConfig->outputs.hdmi[i] = NEXUS_HdmiOutput_Open(i, openSettings);
                if ( NULL == pConfig->outputs.hdmi[i] )
                {
                    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                    goto error;
                }
                NEXUS_OBJECT_REGISTER(NEXUS_HdmiOutput, pConfig->outputs.hdmi[i], Create);
            }
#if NEXUS_DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT
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
        {
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
        /* We open cec by default to initialize it and prevent unwanted wakeups
         * Close it if openCec == false */
        if (!pSettings->openCec) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_Cec, pConfig->outputs.cec[0], Destroy);
            NEXUS_Cec_Close(pConfig->outputs.cec[0]);
            pConfig->outputs.cec[0] = NULL;
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
        NEXUS_DisplayCapabilities *cap = &data->has_display.cap;
        if (cap == NULL) { errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
        NEXUS_GetDisplayCapabilities(cap);
        for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
            pConfig->supportedDisplay[i] = cap->display[i].numVideoWindows > 0;
        }
        pConfig->numWindowsPerDisplay = cap->display[0].numVideoWindows;
    }
#endif

#if NEXUS_HAS_VIDEO_DECODER
    {
        NEXUS_VideoDecoderCapabilities *cap = &data->has_video_decoder.cap;
        if (cap == NULL) { errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
        NEXUS_GetVideoDecoderCapabilities(cap);
        for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
            pConfig->supportedDecoder[i] = i < cap->numVideoDecoders;
        }
    }
#endif

#if (NEXUS_USE_7250_CWF || NEXUS_USE_7250_SV) && NEXUS_HAS_AUDIO
    /* Enable external MCLK signal for audio DAC on 7250_CWF board */
    (void)NEXUS_AudioModule_EnableExternalMclk(1, NEXUS_AudioOutputPll_e0, NEXUS_ExternalMclkRate_e256Fs);
#elif (NEXUS_USE_7250_ACX16 || NEXUS_USE_7250_USFF || NEXUS_USE_7250_CD2) && NEXUS_HAS_AUDIO
    (void)NEXUS_AudioModule_EnableExternalMclk(0, NEXUS_AudioOutputPll_e0, NEXUS_ExternalMclkRate_e256Fs);
#endif

    BKNI_Free(data);
    NEXUS_Platform_P_StartMonitor();

    BDBG_MSG(("NEXUS_Platform_P_Config<< DONE"));
    return BERR_SUCCESS;

error:
    BKNI_Free(data);
    NEXUS_Platform_P_Shutdown();
err_malloc:
    return errCode;
}

void NEXUS_Platform_P_Shutdown(void)
{
    unsigned i;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;

    NEXUS_Platform_P_StopMonitor();

#if NEXUS_HAS_DISPLAY
#if NEXUS_HAS_AUDIO
    {
        NEXUS_AudioCapabilities audioCapabilities;
        NEXUS_GetAudioCapabilities(&audioCapabilities);

        for ( i = 0; i < audioCapabilities.numOutputs.i2s; i++ )
        {
            if ( NULL != pConfig->outputs.i2s[i] )
            {
                NEXUS_AudioOutput_Shutdown(NEXUS_I2sOutput_GetConnector(pConfig->outputs.i2s[i]));
                NEXUS_OBJECT_UNREGISTER(NEXUS_I2sOutput, pConfig->outputs.i2s[i], Destroy);
                NEXUS_I2sOutput_Close(pConfig->outputs.i2s[i]);
                pConfig->outputs.i2s[i] = NULL;
            }
        }
        for ( i = 0; i < audioCapabilities.numOutputs.spdif; i++ ) {
            if (pConfig->outputs.spdif[i]) {
                NEXUS_AudioOutput_Shutdown(NEXUS_SpdifOutput_GetConnector(pConfig->outputs.spdif[i]));
                NEXUS_OBJECT_UNREGISTER(NEXUS_SpdifOutput, pConfig->outputs.spdif[i], Destroy);
                NEXUS_SpdifOutput_Close(pConfig->outputs.spdif[i]);
                pConfig->outputs.spdif[i] = NULL;
            }
        }

        for ( i = 0; i < audioCapabilities.numOutputs.dummy; i++ ) {
            if (pConfig->outputs.audioDummy[i]) {
                NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(pConfig->outputs.audioDummy[i]));
                NEXUS_OBJECT_UNREGISTER(NEXUS_AudioDummyOutput, pConfig->outputs.audioDummy[i], Destroy);
                NEXUS_AudioDummyOutput_Close(pConfig->outputs.audioDummy[i]);
                pConfig->outputs.audioDummy[i] = NULL;
            }
        }
    }
    #endif /* NEXUS_HAS_AUDIO */

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
    #if NEXUS_HAS_AUDIO
    {
        NEXUS_AudioCapabilities audioCapabilities;
        NEXUS_GetAudioCapabilities(&audioCapabilities);
        for ( i = 0; i < audioCapabilities.numOutputs.dac; i++ ) {
            if (pConfig->outputs.audioDacs[i]) {
                NEXUS_AudioOutput_Shutdown(NEXUS_AudioDac_GetConnector(pConfig->outputs.audioDacs[i]));
                NEXUS_OBJECT_UNREGISTER(NEXUS_AudioDac, pConfig->outputs.audioDacs[i], Destroy);
                NEXUS_AudioDac_Close(pConfig->outputs.audioDacs[i]);
                pConfig->outputs.audioDacs[i] = NULL;
            }
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
#if NEXUS_DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT
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

#if NEXUS_HAS_VIDEO_DECODER
static struct {
    bool init;
    unsigned hvd[NEXUS_MAX_XVD_DEVICES];
    bool started[NEXUS_MAX_XVD_DEVICES];
    unsigned num;
} avd_monitor;
#endif

static void NEXUS_Platform_P_Timer(void *context)
{
    BSTD_UNUSED(context);

#if NEXUS_HAS_VIDEO_DECODER
    if (!avd_monitor.init) {
        const char *env = NEXUS_GetEnv("avd_monitor");
        avd_monitor.init = true;
        avd_monitor.num = 0;
        if (env) {
            /* examples: avd_monitor=0 or =1 or =012. or "y" or "all" for all HVD's. */
            if (!NEXUS_P_Base_StrCmp(env, "y") || !NEXUS_P_Base_StrCmp(env, "all")) {
                unsigned i;
                for (i=0;i<NEXUS_MAX_XVD_DEVICES;i++) {
                    if (g_NEXUS_platformSettings.videoDecoderModuleSettings.avdEnabled[i]) {
                        avd_monitor.hvd[avd_monitor.num++] = i;
                    }
                }
            }
            else {
                const char *s;
                for (s=env;*s;s++) {
                    unsigned core = *s - '0';
                    if (core < NEXUS_MAX_XVD_DEVICES) {
                        avd_monitor.hvd[avd_monitor.num++] = core;
                    }
                }
            }
        }
    }

    if (avd_monitor.num) {
        unsigned i;
        for (i=0;i<avd_monitor.num;i++) {
            char buf[256];
            unsigned n;
            if (!avd_monitor.started[i]) {
                if (NEXUS_VideoDecoderModule_SetDebugLog(avd_monitor.hvd[i], true)) {
                    continue;
                }
                avd_monitor.started[i] = true;
            }
            while (!NEXUS_VideoDecoderModule_ReadDebugLog(avd_monitor.hvd[i], buf, 255 /* one less */, &n) && n) {
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
                    BDBG_MODULE_LOG(AVD,("%d: %s", avd_monitor.hvd[i], cur));
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
#if NEXUS_HAS_VIDEO_DECODER
    BKNI_Memset(&avd_monitor, 0, sizeof(avd_monitor));
#endif
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
    "SCPU"
};
#elif BCHP_CHIP == 7344 || BCHP_CHIP==7346 || BCHP_CHIP==7231 || BCHP_CHIP==7429 || BCHP_CHIP==74295 || BCHP_CHIP == 73465
static const char *g_pGisbMasters[] =
{
    "SSP",
    "CPU",
    "reserved",
    "reserved",
    "BSP",
    "RDC",
    "raaga",
    "reserved",
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
    /* We can't have a generic read of BCHP_SUN_GISB_ARB_ERR_CAP_MASTER. Must add chip to get feature. */
    "Unknown"
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

#if NEXUS_HAS_SAGE
bool NEXUS_Platform_P_EnableSageLog(void)
{
    const char *pinmux_env = NEXUS_GetEnv("sage_log");
    /* Only enable pin mux if this is set to 1 */
    return pinmux_env && (NEXUS_atoi(pinmux_env) == 1);
}
#endif

#if BDBG_DEBUG_BUILD && defined(BCHP_SUN_UUI_REG_START)
#include "bchp_sun_uui.h"

static int NEXUS_Platform_P_GetUUIUartID(const char *name) {
    unsigned i = 0;
    if (name) {
        if ((name[0]>='0')&&(name[0]<='9')) {
            i =  NEXUS_atoi(name);
        }
        else {
            for (i=0;(NEXUS_Platform_P_UartIds[i].name);i++) {
                /* homebrew strcmp requiring allowing \0 or , terminator for name */
                unsigned j;
                for (j=0;NEXUS_Platform_P_UartIds[i].name[j] == name[j] && name[j];j++) /* no-op */;
                if (!NEXUS_Platform_P_UartIds[i].name[j] && (!name[j] || name[j] == ',')) break;
            }
            i = (!NEXUS_Platform_P_UartIds[i].name) ? 0 : NEXUS_Platform_P_UartIds[i].id;
        }
    }
    return i;
}

static const char *NEXUS_Platform_P_GetUUIUartName(const unsigned id) {
    unsigned i = 0;
    for (i=0;(NEXUS_Platform_P_UartIds[i].name);i++) {
        if (NEXUS_Platform_P_UartIds[i].id == id) break;
    }
    if (!NEXUS_Platform_P_UartIds[i].name) {
        return "(invalid)";
    }
    else {
        return NEXUS_Platform_P_UartIds[i].name;
    }
}

static void NEXUS_Platform_P_PrintUartIDs(void) {
    int i;
    BDBG_WRN(("UUI UART IDs:"));
    for (i=0;(NEXUS_Platform_P_UartIds[i].name);i++) {
        BDBG_WRN(("%d: %s",NEXUS_Platform_P_UartIds[i].id,NEXUS_Platform_P_UartIds[i].name));
    }
}

void NEXUS_Platform_P_InitUUI(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;
    unsigned n,max,uui_uart_sel;
    bool uui_active = false;
    const char *env = NEXUS_GetEnv("uui_uarts");
#ifdef BCHP_SUN_UUI_UART_SEL6
    max = 6; /* may be too small, but won't be too large */
#else
    #error Please extend this code for new silicon
#endif
    if (env) {
        if (env[0]=='?') {
            NEXUS_Platform_P_PrintUartIDs();
        }
        else {
            const char *cur;
            cur = env;
            for (n=1;n<=max;n++) {
                uui_uart_sel = NEXUS_Platform_P_GetUUIUartID(cur);
                while (*cur && *cur !=',') {
                    cur++;
                }
                if (*cur && *cur ==',') {
                    cur++;
                }
                if (uui_uart_sel) {
                    if (!uui_active) {
                        BREG_Write32 (hReg, BCHP_SUN_UUI_COMMAND, BCHP_FIELD_DATA(SUN_UUI_COMMAND,start,1));
                        BREG_Write32 (hReg, BCHP_SUN_UUI_COMMAND, BCHP_FIELD_DATA(SUN_UUI_COMMAND,stop,0));
                        BKNI_Sleep(1);
                        reg = BREG_Read32(hReg, BCHP_SUN_UUI_STATUS);
                        if (reg & BCHP_MASK(SUN_UUI_STATUS,active) ) {
                            uui_active = true;
                        }
                        else {
                            BDBG_ERR(("Failed to activate UUI.  Check connection."));
                        }
                    }
                    BDBG_WRN(("Selecting UART %u: %s on UUI %d.",uui_uart_sel,NEXUS_Platform_P_GetUUIUartName(uui_uart_sel),n));
                    reg = BCHP_FIELD_DATA(SUN_UUI_UART_SEL1,enable,1);
                    reg |= BCHP_FIELD_DATA(SUN_UUI_UART_SEL1,select,uui_uart_sel);
                    BREG_Write32 (hReg, BCHP_SUN_UUI_UART_SEL1+((n-1)*(BCHP_SUN_UUI_UART_SEL2-BCHP_SUN_UUI_UART_SEL1)), reg);
                }
                else {
                    BDBG_MSG(("Disabling UART on UUI %d.",n));
                    reg = BCHP_FIELD_DATA(SUN_UUI_UART_SEL1,enable,0);
                    reg |= BCHP_FIELD_DATA(SUN_UUI_UART_SEL1,select,0);
                    BREG_Write32 (hReg, BCHP_SUN_UUI_UART_SEL1+((n-1)*(BCHP_SUN_UUI_UART_SEL2-BCHP_SUN_UUI_UART_SEL1)), reg);
                }
            }
        }
    }
    if (!uui_active) {
        BREG_Write32 (hReg, BCHP_SUN_UUI_COMMAND, BCHP_FIELD_DATA(SUN_UUI_COMMAND,stop,2));
    }
}
#else
void NEXUS_Platform_P_InitUUI(void)
{
}
#endif
