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
 ******************************************************************************/
#include "nexus_base.h"
#include "nexus_platform_priv.h"
#include "b_objdb.h"
#include "nexus_power_management.h"

#if NEXUS_HAS_I2C
#include "nexus_i2c_init.h"
#include "priv/nexus_i2c_standby_priv.h"
#endif
#if NEXUS_HAS_TRANSPORT
#include "nexus_transport_init.h"
#include "nexus_input_band.h"
#include "priv/nexus_transport_standby_priv.h"
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder_init.h"
#include "priv/nexus_video_decoder_standby_priv.h"
#include "priv/nexus_video_decoder_priv.h"
#endif
#if NEXUS_HAS_DISPLAY
#include "nexus_display_init.h"
#include "nexus_video_input.h"
#include "nexus_video_output.h"
#include "priv/nexus_display_standby_priv.h"
#include "priv/nexus_display_priv.h"
#endif
#if NEXUS_HAS_SURFACE
#include "nexus_surface_init.h"
#include "priv/nexus_surface_module_local.h"
#endif
#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d_init.h"
#include "priv/nexus_graphics2d_standby_priv.h"
#endif
#ifdef NEXUS_HAS_GRAPHICSV3D
#include "nexus_graphicsv3d_init.h"
#include "priv/nexus_graphicsv3d_standby_priv.h"
#endif
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend_init.h"
#include "priv/nexus_frontend_standby_priv.h"
#endif
#if NEXUS_HAS_SECURITY
#include "nexus_security_init.h"
#include "priv/nexus_security_standby_priv.h"
#include "priv/nexus_security_priv.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_init.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "priv/nexus_audio_standby_priv.h"
#endif
#if NEXUS_HAS_FILE
#include "nexus_file_init.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback_init.h"
#endif
#if NEXUS_HAS_RECORD
#include "nexus_record_init.h"
#endif
#if NEXUS_HAS_DVB_CI
#include "nexus_dvb_ci_init.h"
#endif
#if NEXUS_HAS_IR_INPUT
#include "nexus_ir_input_init.h"
#include "priv/nexus_ir_input_standby_priv.h"
#endif
#if NEXUS_HAS_LED
#include "nexus_led_init.h"
#endif
#if NEXUS_HAS_KEYPAD
#include "nexus_keypad_init.h"
#endif
#if NEXUS_HAS_IR_BLASTER
#include "nexus_ir_blaster_init.h"
#endif
#if NEXUS_HAS_INPUT_CAPTURE
#include "nexus_input_capture_init.h"
#endif
#if NEXUS_HAS_UHF_INPUT
#include "nexus_uhf_input_init.h"
#include "priv/nexus_uhf_input_standby_priv.h"
#endif
#if NEXUS_HAS_GPIO
#include "nexus_gpio_init.h"
#include "priv/nexus_gpio_standby_priv.h"
#include "nexus_platform_shared_gpio.h"
#endif
#if NEXUS_HAS_SPI
#include "nexus_spi_init.h"
#endif
#if NEXUS_HAS_UART
#include "nexus_uart_init.h"
#include "priv/nexus_uart_standby_priv.h"
#endif
#if NEXUS_HAS_SMARTCARD
#include "nexus_smartcard_init.h"
#include "priv/nexus_smartcard_standby_priv.h"
#endif
#if NEXUS_HAS_DMA
#include "nexus_dma_init.h"
#include "priv/nexus_dma_standby_priv.h"
#endif
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output_init.h"
#include "priv/nexus_hdmi_output_standby_priv.h"
#endif
#if NEXUS_HAS_CEC
#include "nexus_cec_init.h"
#include "priv/nexus_cec_standby_priv.h"
#endif
#if NEXUS_HAS_HDMI_DVO
#include "nexus_hdmi_dvo_init.h"
#endif
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input_init.h"
#include "priv/nexus_hdmi_input_standby_priv.h"
#endif
#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder_init.h"
#include "priv/nexus_picture_decoder_standby_priv.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel_init.h"
#endif
#if NEXUS_HAS_ASTM
#include "nexus_astm_init.h"
#endif
#if NEXUS_HAS_RFM
#include "nexus_rfm_init.h"
#include "priv/nexus_rfm_standby_priv.h"
#endif
#include "nexus_core_utils.h"
#include "priv/nexus_core_module_local.h"
#if NEXUS_HAS_PWM
#include "nexus_pwm_init.h"
#include "nexus_pwm.h"
#include "priv/nexus_pwm_standby_priv.h"
#endif
#ifdef NEXUS_HAS_TOUCHPAD
#include "nexus_touchpad_init.h"
#endif
#if NEXUS_HAS_SIMPLE_AUDIO_PLAYBACK
#include "nexus_simple_audio_playback_init.h"
#endif
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_simple_decoder_init.h"
#endif
#if NEXUS_HAS_SURFACE_COMPOSITOR
#include "nexus_surface_compositor_init.h"
#include "priv/nexus_surface_compositor_standby_priv.h"
#endif
#if NEXUS_HAS_INPUT_ROUTER
#include "nexus_input_router_init.h"
#endif
#if NEXUS_HAS_TEMP_MONITOR
#include "nexus_temp_monitor_init.h"
#include "priv/nexus_temp_monitor_standby_priv.h"
#endif
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_init.h"
#include "priv/nexus_video_encoder_standby_priv.h"
#endif
#if NEXUS_HAS_STREAM_MUX
#include "nexus_stream_mux_init.h"
#endif
#if NEXUS_HAS_FILE_MUX
#include "nexus_file_mux_init.h"
#endif
#if NEXUS_HAS_NSK2HDI
#include "nexus_nsk2hdi_init.h"
#endif
#if NEXUS_BASE_OS_linuxkernel
#include "nexus_generic_driver_impl.h"
#endif
#if NEXUS_HAS_SAGE
#include "nexus_sage_init.h"
#include "nexus_sage_types.h"
#include "priv/nexus_sage_priv.h"
#endif
#if NEXUS_HAS_SCM
#include "nexus_scm_init.h"
#endif
#if NEXUS_HAS_ASP
#include "nexus_asp_init.h"
#endif
#include "nexus_scm_id_string.h"

#include "priv/nexus_core_standby_priv.h"

#include "bchp_pwr.h"
#include "bchp_common.h"
#ifdef BCHP_AVS_TMON_REG_START
#include "bchp_avs_tmon.h"
#endif
#ifdef BCHP_AON_CTRL_REG_START
#include "bchp_aon_ctrl.h"
#endif

#if NEXUS_COMPAT_32ABI
#include "nexus_base_compat.h"
#include "nexus_platform_api_compat.h"
#endif

BDBG_MODULE(nexus_platform);

NEXUS_PlatformHandles g_NEXUS_platformHandles;
NEXUS_PlatformSettings g_NEXUS_platformSettings;   /* saved platform settings */
NEXUS_P_PlatformInternalSettings g_NEXUS_platformInternalSettings; /* saved platform internal settings */
NEXUS_ModuleHandle g_NEXUS_platformModule = NULL;
static NEXUS_TimerHandle g_NEXUS_CallbackMonitorTimer = NULL;


static void NEXUS_P_Platform_CallbackMonitor(void *context)
{
    unsigned i;
    unsigned timeout = 100;

    BSTD_UNUSED(context);
    g_NEXUS_CallbackMonitorTimer = NULL;

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        NEXUS_P_BaseCallback_Monitor(i, timeout);
    }

    g_NEXUS_CallbackMonitorTimer = NEXUS_ScheduleTimerByPriority(Internal, timeout, NEXUS_P_Platform_CallbackMonitor, NULL);
}

#if defined(NEXUS_BASE_OS_linuxuser) && !B_HAS_BPROFILE
/* We can speed up init slightly by loading FW from multiple threads. */
#define MULTITHREADED_INIT 1
#endif

static NEXUS_Platform_P_ModuleInfo * NEXUS_Platform_P_AddModule(NEXUS_ModuleHandle module_handle,
                                NEXUS_ModuleStandbyLevel standby_level,
                                void (*uninit)(void),
                                NEXUS_Error (*standby)(bool, const NEXUS_StandbySettings *))
{
    NEXUS_Platform_P_ModuleInfo *module_info;

    if(!module_handle)
        return NULL;

    module_info = BKNI_Malloc(sizeof(*module_info));
    if(!module_info)
        return NULL;

    BKNI_Memset(module_info, 0, sizeof(*module_info));
    module_info->module = module_handle;
    module_info->standby_level = standby_level;
    module_info->locked = false;
    module_info->powerdown = false;
    module_info->uninit = uninit;
    module_info->standby = standby;
    BLST_Q_INSERT_HEAD(&g_NEXUS_platformHandles.handles, module_info, link);

    return module_info;
}

static void NEXUS_Platform_P_UninitModules(void)
{
    NEXUS_Platform_P_ModuleInfo *module_info;

    while(NULL != (module_info=BLST_Q_FIRST(&g_NEXUS_platformHandles.handles))) {
        BDBG_MSG(("uninit %s", NEXUS_Module_GetName(module_info->module)));
        if( module_info->uninit ) {
            module_info->uninit();
        }
        else if (module_info->module != g_NEXUS_platformHandles.core) {
            BDBG_WRN(("Uninit function not provided for module %s", NEXUS_Module_GetName(module_info->module)));
        }
        BLST_Q_REMOVE_HEAD(&g_NEXUS_platformHandles.handles, link);
        BKNI_Free(module_info);
    }
}

#if NEXUS_HAS_VIDEO_DECODER
static void NEXUS_Platform_P_InitVideoDecoder(void *context)
{
    NEXUS_VideoDecoderModuleInternalSettings *pVideoDecoderSettings = context;
    BDBG_MSG((">VIDEO_DECODER"));
    BDBG_ASSERT(g_NEXUS_platformHandles.transport);
    pVideoDecoderSettings->transport = g_NEXUS_platformHandles.transport;
    pVideoDecoderSettings->core = g_NEXUS_platformHandles.core;
    pVideoDecoderSettings->security = g_NEXUS_platformHandles.security;
    pVideoDecoderSettings->secureHeap = g_pCoreHandles->heap[NEXUS_VIDEO_SECURE_HEAP].nexus;
#if NEXUS_NUM_ZSP_VIDEO_DECODERS || NEXUS_NUM_DSP_VIDEO_DECODERS
    BDBG_ASSERT(g_NEXUS_platformHandles.audio);
    pVideoDecoderSettings->audio = g_NEXUS_platformHandles.audio;
#endif
#if NEXUS_NUM_SID_VIDEO_DECODERS && NEXUS_HAS_PICTURE_DECODER
    BDBG_ASSERT(g_NEXUS_platformHandles.pictureDecoder);
    pVideoDecoderSettings->pictureDecoder = g_NEXUS_platformHandles.pictureDecoder;
#endif
    g_NEXUS_platformHandles.videoDecoder = NEXUS_VideoDecoderModule_Init(pVideoDecoderSettings, &g_NEXUS_platformSettings.videoDecoderModuleSettings);
    if (!g_NEXUS_platformHandles.videoDecoder) {
        BDBG_ERR(("Unable to init VideoDecoder"));
    }
}
#endif

#if NEXUS_HAS_AUDIO
static void NEXUS_Platform_P_InitAudio(void *context)
{
    NEXUS_AudioModuleInternalSettings internalAudioSettings;
    NEXUS_AudioModuleSettings audioSettings;

    BSTD_UNUSED(context);
    BDBG_MSG((">AUDIO"));
    BDBG_ASSERT(g_NEXUS_platformHandles.transport);
    audioSettings = g_NEXUS_platformSettings.audioModuleSettings;
    NEXUS_AudioModule_GetDefaultInternalSettings(&internalAudioSettings);
    internalAudioSettings.modules.transport = g_NEXUS_platformHandles.transport;
    BDBG_ASSERT(g_NEXUS_platformHandles.surface);
    internalAudioSettings.modules.surface = g_NEXUS_platformHandles.surface;
    #if NEXUS_HAS_HDMI_OUTPUT
    BDBG_ASSERT(g_NEXUS_platformHandles.hdmiOutput);
    internalAudioSettings.modules.hdmiOutput = g_NEXUS_platformHandles.hdmiOutput;
    #endif
    #if NEXUS_HAS_HDMI_INPUT
    BDBG_ASSERT(g_NEXUS_platformHandles.hdmiInput);
    internalAudioSettings.modules.hdmiInput = g_NEXUS_platformHandles.hdmiInput;
    #endif
    #if NEXUS_HAS_RFM
    BDBG_ASSERT(g_NEXUS_platformHandles.rfm);
    internalAudioSettings.modules.rfm = g_NEXUS_platformHandles.rfm;
    #endif
    internalAudioSettings.modules.core = g_NEXUS_platformHandles.core;
    #if NEXUS_HAS_SECURITY
    BDBG_ASSERT(g_NEXUS_platformHandles.security);
    internalAudioSettings.modules.security = g_NEXUS_platformHandles.security;
    #endif
    #if NEXUS_HAS_SAGE
    BDBG_ASSERT(g_NEXUS_platformHandles.sage);
    internalAudioSettings.modules.sage = g_NEXUS_platformHandles.sage;
    #endif
    /* this is something used by the DTV... for now ignore the "DTV centric guarding schema" */
    #if NEXUS_HAS_FRONTEND
    internalAudioSettings.modules.frontend = g_NEXUS_platformHandles.frontend;
    #endif
    #if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    audioSettings.firmwareHeapIndex = g_pCoreHandles->defaultHeapIndex;
    #endif
    g_NEXUS_platformHandles.audio = NEXUS_AudioModule_Init(&audioSettings, &internalAudioSettings);
    if ( !g_NEXUS_platformHandles.audio )
    {
        BDBG_ERR(("Unable to init audio"));
    } else {
        NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.audio, NEXUS_ModuleStandbyLevel_eAll, NEXUS_AudioModule_Uninit, NEXUS_AudioModule_Standby_priv);
    }
}
#endif

#if NEXUS_HAS_DVB_CI
static void *g_pMemoryEbi = NULL;

static NEXUS_Error NEXUS_Platform_P_InitDvbci(void)
{

    g_pMemoryEbi = NEXUS_Platform_P_MapRegisterMemory(NEXUS_DVB_CI_MEMORY_BASE, NEXUS_DVB_CI_MEMORY_LENGTH);

    if (g_pMemoryEbi == NULL) {
        BDBG_ERR(("Unable to map DVB CI register space"));
         return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    NEXUS_P_AddMap(NEXUS_DVB_CI_MEMORY_BASE, NULL, NEXUS_AddrType_eUncached, g_pMemoryEbi, NEXUS_AddrType_eUncached, NEXUS_DVB_CI_MEMORY_LENGTH);

    return NEXUS_SUCCESS;
}

static void NEXUS_Platform_P_UninitDvbci(void)
{
    if (g_pMemoryEbi) {
        NEXUS_Platform_P_UnmapRegisterMemory(g_pMemoryEbi, NEXUS_DVB_CI_MEMORY_LENGTH);
        g_pMemoryEbi = NULL;
    }
}
#endif

#if BDBG_DEBUG_BUILD
static void NEXUS_Platform_P_Print(void)
{
#if BCHP_PWR_SUPPORT
    if (g_NEXUS_pCoreHandles) {
        BCHP_PWR_DebugPrint(g_NEXUS_pCoreHandles->chp);
    }
#endif
    BDBG_LOG(("Nexus Release %d.%d",NEXUS_P_GET_VERSION(NEXUS_PLATFORM) / NEXUS_PLATFORM_VERSION_UNITS,
        NEXUS_P_GET_VERSION(NEXUS_PLATFORM) % NEXUS_PLATFORM_VERSION_UNITS));
}
#endif

static NEXUS_Error nexus_platform_p_apply_memconfig(const NEXUS_Core_PreInitState *preInitState, const NEXUS_MemoryConfigurationSettings *pMemConfig)
{
    int rc;
    NEXUS_P_GetDefaultMemoryRtsSettings(preInitState, &g_NEXUS_platformHandles.rtsSettings);
    if (pMemConfig) {
        rc = NEXUS_P_ApplyMemoryConfiguration(preInitState, pMemConfig, &g_NEXUS_platformHandles.rtsSettings, &g_NEXUS_platformSettings, &g_NEXUS_platformInternalSettings);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

static void  NEXUS_Platform_P_StartServer(void)
{
    unsigned i;
    NEXUS_Platform_P_InitServer();
    /* add heap handles into the database */
    for(i=0;i<sizeof(g_pCoreHandles->heap)/sizeof(g_pCoreHandles->heap[0]);i++) {
        NEXUS_HeapHandle heap = g_pCoreHandles->heap[i].nexus;
        if(heap) {
            NEXUS_OBJECT_REGISTER(NEXUS_Heap, heap, Open);
        }
    }
    return;
}

static void  NEXUS_Platform_P_StopServer(void)
{
    unsigned i;

    /* remove heap handles from the database */
    for(i=0;i<sizeof(g_pCoreHandles->heap)/sizeof(g_pCoreHandles->heap[0]);i++) {
        NEXUS_HeapHandle heap = g_pCoreHandles->heap[i].nexus;
        if(heap) {
            NEXUS_OBJECT_UNREGISTER(NEXUS_Heap, heap, Close);
        }
    }
    NEXUS_Platform_P_UninitServer();
    return;
}

/***************************************************************************
Summary:
    Initialize Nexus
Description:
    This will initialize all board-specifics and then proceed to
    initialize the nexus modules above it.  This is the main entry point
    for all applications to start Nexus.
See Also:
    NEXUS_Platform_Uninit
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_Init_tagged( const NEXUS_PlatformSettings *pSettings,
    const NEXUS_MemoryConfigurationSettings *pMemConfig,
    unsigned platformCheck, unsigned versionCheck, unsigned structSizeCheck )
{
    NEXUS_Error errCode;
#if MULTITHREADED_INIT && NEXUS_HAS_VIDEO_DECODER
    NEXUS_ThreadHandle videoDecoderThread = NULL;
#endif
#if MULTITHREADED_INIT && NEXUS_HAS_AUDIO
    NEXUS_ThreadHandle audioThread = NULL;
#endif
    struct {
        NEXUS_Base_Settings baseSettings;
#if NEXUS_HAS_TRANSPORT
        NEXUS_TransportModuleInternalSettings transportSettings;
#endif
#if NEXUS_HAS_SECURITY
        NEXUS_SecurityModuleInternalSettings securitySettings;
#endif
#if NEXUS_HAS_FRONTEND
        NEXUS_FrontendModuleSettings frontendSettings;
#endif
#if NEXUS_HAS_PLAYBACK
        NEXUS_PlaybackModuleSettings playbackSettings;
#endif
#if NEXUS_HAS_RECORD
        NEXUS_RecordModuleSettings recordSettings;
#endif
#if NEXUS_HAS_GRAPHICS2D
        NEXUS_Graphics2DModuleInternalSettings graphics2DSettings;
#endif
#if NEXUS_HAS_DMA
        NEXUS_DmaModuleSettings dmaSettings;
#endif
#if NEXUS_HAS_HDMI_OUTPUT
        NEXUS_HdmiOutputModuleSettings hdmiSettings;
#endif
#if NEXUS_HAS_HDMI_INPUT
        NEXUS_HdmiInputModuleSettings hdmiInputSettings;
#endif
#if NEXUS_HAS_CEC
        NEXUS_CecModuleSettings cecSettings;
#endif
#if NEXUS_HAS_PICTURE_DECODER
        NEXUS_PictureDecoderModuleSettings pictureDecoderSettings;
#endif
#if NEXUS_HAS_SYNC_CHANNEL
        NEXUS_SyncChannelModuleSettings syncChannelSettings;
#endif
#if NEXUS_HAS_ASTM
        NEXUS_AstmModuleSettings astmSettings;
#endif
#if NEXUS_HAS_DVB_CI
        NEXUS_DvbCiModuleSettings dvbCiSettings;
#endif
#if NEXUS_HAS_UHF_INPUT
        NEXUS_UhfInputModuleSettings uhfSettings;
#endif
#if NEXUS_HAS_IR_INPUT
        NEXUS_IrInputModuleSettings irInputSettings;
#endif
#if NEXUS_HAS_INPUT_CAPTURE
        NEXUS_InputCaptureModuleSettings inputCaptureSettings;
#endif
#if NEXUS_HAS_LED
        NEXUS_LedModuleSettings ledSettings;
#endif
#if NEXUS_HAS_KEYPAD
        NEXUS_KeypadModuleSettings keypadSettings;
#endif
#if NEXUS_HAS_I2C
        NEXUS_I2cModuleSettings i2cSettings;
#endif
#if NEXUS_HAS_UART
        NEXUS_UartModuleSettings uartSettings;
#endif
#if NEXUS_HAS_TEMP_MONITOR
        NEXUS_TempMonitorModuleSettings tempMonitorSettings;
#endif
#if NEXUS_HAS_SPI
        NEXUS_SpiModuleSettings spiSettings;
#endif
#if NEXUS_HAS_GPIO
        NEXUS_GpioModuleSettings gpioSettings;
#endif
#if NEXUS_HAS_STREAM_MUX
        NEXUS_StreamMuxModuleSettings streamMuxModuleSettings;
#endif
#if NEXUS_HAS_NSK2HDI
        NEXUS_Nsk2hdiModuleSettings nsk2hdiSettings;
#endif
#if NEXUS_HAS_SURFACE
        NEXUS_SurfaceModuleSettings surfaceSettings;
#endif
#if NEXUS_HAS_SIMPLE_AUDIO_PLAYBACK
        NEXUS_SimpleAudioPlaybackModuleSettings simpleAudioPlaybackSettings;
#endif
#if NEXUS_HAS_SIMPLE_DECODER
        NEXUS_SimpleDecoderModuleSettings simpleDecoderSettings;
#endif
#if NEXUS_HAS_GRAPHICSV3D
        NEXUS_Graphicsv3dModuleSettings graphicsv3dSettings;
#endif
#if NEXUS_HAS_FILE
        NEXUS_FileModuleSettings fileModuleSettings;
#endif
#if NEXUS_HAS_DISPLAY
        NEXUS_DisplayModuleInternalSettings displaySettings;
#endif
#if NEXUS_HAS_SURFACE_COMPOSITOR
        NEXUS_SurfaceCompositorModuleSettings surfaceCompositorSettings;
#endif
#if NEXUS_HAS_INPUT_ROUTER
        NEXUS_InputRouterModuleSettings inputRouterSettings;
#endif
#if NEXUS_HAS_SMARTCARD
        NEXUS_SmartcardModuleInternalSettings smartcardSettings;
#endif
#if NEXUS_HAS_ASP
        NEXUS_AspModuleSettings aspSettings;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
        NEXUS_VideoEncoderModuleInternalSettings videoEncoderSettings;
#endif
#if NEXUS_HAS_VIDEO_DECODER
        NEXUS_VideoDecoderModuleInternalSettings videoDecoderSettings;
#endif
        NEXUS_ModuleSettings moduleSettings;
    } *state = NULL;
#if NEXUS_HAS_SIMPLE_AUDIO_PLAYBACK
    NEXUS_ModuleHandle simpleAudioPlayback;
#endif
    NEXUS_ModuleHandle handle;
    const NEXUS_Core_PreInitState *preInitState;

    BSTD_UNUSED(handle); /* in case it is unused */

#if !NEXUS_PLATFORM_NON_NUMERIC
    if ((!pSettings || pSettings->checkPlatformType) && platformCheck != NEXUS_PLATFORM) {
        /* This code also ensures that NEXUS_PLATFORM is numeric, not alpha-numeric */
        BDBG_ERR(("NEXUS_Platform_Init failed with platform mismatch (nexus=%d, caller=%d). The application must be recompiled for this platform.",
            NEXUS_PLATFORM, platformCheck));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
#else
    BSTD_UNUSED(platformCheck);
#endif

    if ((!pSettings || pSettings->checkPlatformType) && versionCheck != NEXUS_P_GET_VERSION(NEXUS_PLATFORM)) {
        BDBG_ERR(("NEXUS_Platform_Init failed with version mismatch (nexus=%d.%d, caller=%d.%d). Please recompile application and/or nexus.",
            NEXUS_P_GET_VERSION(NEXUS_PLATFORM) / NEXUS_PLATFORM_VERSION_UNITS,
            NEXUS_P_GET_VERSION(NEXUS_PLATFORM) % NEXUS_PLATFORM_VERSION_UNITS,
            versionCheck / NEXUS_PLATFORM_VERSION_UNITS,
            versionCheck % NEXUS_PLATFORM_VERSION_UNITS));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (structSizeCheck != NEXUS_P_GET_STRUCT_SIZES()
#if NEXUS_COMPAT_32ABI
        && (structSizeCheck != sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformSettings)) + sizeof(B_NEXUS_COMPAT_TYPE(NEXUS_PlatformConfiguration)))
#endif
        ) {
        BDBG_ERR(("%s failed with struct size mismatch (nexus=%u, caller=%u). Please recompile application and/or nexus.",
            "NEXUS_Platform", (unsigned)NEXUS_P_GET_STRUCT_SIZES(), structSizeCheck));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (g_NEXUS_platformModule != NULL) {
        BDBG_ERR(("Already initialized"));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    BDBG_MSG((">MAGNUM"));
    preInitState = NEXUS_Platform_P_PreInit();
    if (!preInitState) {errCode = -1; goto err_preinit;} /* no BERR_TRACE possible */

    state = BKNI_Malloc(sizeof(*state));
    if(state==NULL) { errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_state_alloc;}

    /* Create defaults if no settings were provided */
    if ( NULL == pSettings )
    {
        NEXUS_Platform_Priv_GetDefaultSettings(preInitState, &g_NEXUS_platformSettings);
    }
    else
    {
        g_NEXUS_platformSettings = *pSettings;
    }
    pSettings = &g_NEXUS_platformSettings;

    BKNI_Memset(&g_NEXUS_platformHandles, 0, sizeof(g_NEXUS_platformHandles));
    BLST_Q_INIT(&g_NEXUS_platformHandles.handles);

    errCode = nexus_platform_p_apply_memconfig(preInitState, pMemConfig);
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_memconfig;}

    BDBG_MSG((">OBJDB"));
    errCode = b_objdb_init();
    if(errCode!=NEXUS_SUCCESS) { errCode = BERR_TRACE(errCode);goto err_objdb_init; }

    BDBG_MSG((">BASE"));
    NEXUS_Base_GetDefaultSettings(&state->baseSettings);
    state->baseSettings.procInit = nexus_platform_p_add_proc;
    state->baseSettings.procUninit = nexus_platform_p_remove_proc;
    errCode = NEXUS_Base_Init(&state->baseSettings);
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_base; }

    NEXUS_Module_GetDefaultSettings(&state->moduleSettings);
#if BDBG_DEBUG_BUILD
    state->moduleSettings.dbgPrint = NEXUS_Platform_P_Print;
    state->moduleSettings.dbgModules = "nexus_platform;BCHP_PWR";
#endif
    g_NEXUS_platformModule = NEXUS_Module_Create("platform",  &state->moduleSettings);
    if ( !g_NEXUS_platformModule ) { errCode=BERR_TRACE(errCode); goto err_plaform_module; }
    NEXUS_LockModule();

    BDBG_MSG((">OS"));
    errCode = NEXUS_Platform_P_InitOS();
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_os; }

    /* Init the core modules */
    BDBG_MSG((">CORE_LOCAL"));
    errCode = NEXUS_CoreModule_LocalInit();
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_core_local; }
#if NEXUS_HAS_SURFACE
    BDBG_MSG((">SURFACE_LOCAL"));
    errCode = NEXUS_SurfaceModule_LocalInit();
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_surface_local; }
#endif
    BDBG_MSG((">CORE"));
    errCode = NEXUS_Platform_P_InitCore(preInitState, &g_NEXUS_platformSettings);
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_core; }

    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.core, NEXUS_ModuleStandbyLevel_eAlwaysOn, NULL, NEXUS_CoreModule_Standby_priv);

    {
        struct status {
            BCHP_Info info;
            char platformSwVersion[100];
        } *status;
        status = BKNI_Malloc(sizeof(*status));
        if (!status) { errCode=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_core; }
        NEXUS_Platform_GetReleaseVersion(status->platformSwVersion, sizeof(status->platformSwVersion));
        BCHP_GetInfo(g_pCoreHandles->chp, &status->info);
        BDBG_WRN(("%s, boxmode %d, pmap %d, product %x %c%u", status->platformSwVersion, preInitState->boxMode,
            preInitState->pMapId, status->info.productId, 'A' + (status->info.rev >> 4), (status->info.rev & 0xF)));
        BKNI_Free(status);
    }

    {
        static const char nexus_scm_id_string[]="NEXUS_SCM_ID_STRING:" NEXUS_SCM_ID_STRING;
        BDBG_LOG(("%s", nexus_scm_id_string));
    }
    NEXUS_P_PrintEnv(BDBG_STRING(""));

    if(NEXUS_GetEnv("profile_init")) {
        NEXUS_Profile_Start();
    }

    /* do board-specific configuration after Core is brought up */
    BDBG_MSG((">INIT_BOARD"));
    errCode = NEXUS_Platform_P_InitBoard();
    if (errCode) {errCode = BERR_TRACE(errCode); goto err_initboard;}

    BDBG_MSG((">PINMUX"));
    /* Init pinmuxes and vcxo control */
    errCode = NEXUS_Platform_P_InitPinmux();
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_postboard; }

    BDBG_MSG((">UUI"));
    /* Init Universal UART Interface */
    NEXUS_Platform_P_InitUUI();

    /* Start Interrupts */
    BDBG_MSG((">InitInterrupts"));
    errCode = NEXUS_Platform_P_InitInterrupts();
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err_postboard; }

    errCode = NEXUS_Platform_P_DropPrivilege(&g_NEXUS_platformSettings);
    BDBG_ASSERT(!errCode); /* failure will terminate */

    {
        const char *str = NEXUS_GetEnv("NEXUS_BASE_ONLY_INIT");
        if (str && !NEXUS_StrCmp(str, "y")) {
            unsigned i;
            /* NEXUS_Platform_P_Config will not be called, so populate heaps here */
            for(i=0;i<NEXUS_MAX_HEAPS;i++) {
                g_NEXUS_platformHandles.config.heap[i] = g_pCoreHandles->heap[i].nexus;
            }
            g_NEXUS_platformHandles.baseOnlyInit = true;
            goto base_only_init;
        }
    }

    /* First bring up the minimum modules necessary to launch the module-init threads. */

#if NEXUS_HAS_DMA
    BDBG_MSG((">DMA"));
    NEXUS_DmaModule_GetDefaultSettings(&state->dmaSettings);
    state->dmaSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
    g_NEXUS_platformHandles.dma = NEXUS_DmaModule_Init(&state->dmaSettings);
    if (!g_NEXUS_platformHandles.dma) {
        BDBG_ERR(("Unable to init Dma"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.dma, state->dmaSettings.common.standbyLevel, NEXUS_DmaModule_Uninit, NEXUS_DmaModule_Standby_priv);
#endif

#if NEXUS_HAS_TRANSPORT
    BDBG_MSG((">TRANSPORT"));
    NEXUS_TransportModule_GetDefaultInternalSettings(&state->transportSettings);
    state->transportSettings.dma = g_NEXUS_platformHandles.dma;
    state->transportSettings.core = g_NEXUS_platformHandles.core;
    state->transportSettings.secureHeap = g_pCoreHandles->heap[NEXUS_VIDEO_SECURE_HEAP].nexus;
    state->transportSettings.mainHeapIndex = g_pCoreHandles->defaultHeapIndex;
#if NEXUS_HAS_SECURITY
    state->transportSettings.postInitCalledBySecurity = true;
#endif
    g_NEXUS_platformHandles.transport = NEXUS_TransportModule_PreInit(&state->transportSettings, &g_NEXUS_platformSettings.transportModuleSettings);
    if (!g_NEXUS_platformHandles.transport) {
        BDBG_ERR(("Unable to init Transport"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.transport, g_NEXUS_platformSettings.transportModuleSettings.common.standbyLevel, NEXUS_TransportModule_Uninit, NEXUS_TransportModule_Standby_priv);
#endif

#if NEXUS_HAS_SECURITY
    BDBG_MSG((">SECURITY"));

    NEXUS_SecurityModule_GetDefaultInternalSettings(&state->securitySettings);
    state->securitySettings.transport = g_NEXUS_platformHandles.transport;
    state->securitySettings.callTransportPostInit = true;
    g_NEXUS_platformHandles.security = NEXUS_SecurityModule_Init(&state->securitySettings, &g_NEXUS_platformSettings.securitySettings);
    if (!g_NEXUS_platformHandles.security) {
        BDBG_ERR(("Unable to init security"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Module_Lock(g_NEXUS_platformHandles.security);
    NEXUS_Security_PrintArchViolation_priv(); /* print (and clear) any outstanding ARCH violations on boot */
    NEXUS_Module_Unlock(g_NEXUS_platformHandles.security);
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.security, g_NEXUS_platformSettings.securitySettings.common.standbyLevel, NEXUS_SecurityModule_Uninit, NEXUS_SecurityModule_Standby_priv);
#endif

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderModule_GetDefaultInternalSettings(&state->videoDecoderSettings, &g_NEXUS_platformSettings.videoDecoderModuleSettings);
#endif

#if NEXUS_HAS_SAGE
    BDBG_MSG((">SAGE"));
    {
        NEXUS_SageModuleInternalSettings sageInternalSettings;
        NEXUS_SageModule_GetDefaultInternalSettings(&sageInternalSettings);
        sageInternalSettings.security = g_NEXUS_platformHandles.security;
        sageInternalSettings.transport = g_NEXUS_platformHandles.transport;
#if NEXUS_HAS_VIDEO_DECODER
        sageInternalSettings.videoDecoderFirmware.block = state->videoDecoderSettings.firmware.block;
        sageInternalSettings.videoDecoderFirmware.size = state->videoDecoderSettings.firmware.size;
#endif
        sageInternalSettings.lazyUnmap = NEXUS_Platform_P_LazyUnmap();
        g_NEXUS_platformHandles.sage = NEXUS_SageModule_Init(&pSettings->sageModuleSettings, &sageInternalSettings);
        if (!g_NEXUS_platformHandles.sage) {
            BDBG_ERR(("Unable to init sage"));
            errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err;
        }
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.sage, pSettings->sageModuleSettings.common.standbyLevel, NEXUS_SageModule_Uninit,
#if NEXUS_POWER_MANAGEMENT
            NEXUS_SageModule_Standby_priv
#else
            NULL
#endif
    );
#endif

#if NEXUS_HAS_ASP
    BDBG_MSG((">ASP:"));
    NEXUS_AspModule_GetDefaultSettings(&state->aspSettings);
#if NEXUS_HAS_SECURITY
    state->aspSettings.modules.security = g_NEXUS_platformHandles.security;
#endif
#if NEXUS_HAS_SAGE
    state->aspSettings.modules.sage = g_NEXUS_platformHandles.sage;
#endif
    handle = NEXUS_AspModule_Init(&state->aspSettings);
    if ( !handle ) {
        BDBG_ERR(("Unable to init ASP"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_AspModule_Uninit, NULL);
#endif

    /* init power state, which blanks display. then bring up transport via security. */
    NEXUS_CoreModule_PostInit();
#if NEXUS_HAS_SECURITY
    NEXUS_Module_Lock( g_NEXUS_platformHandles.security );
    errCode = NEXUS_SecurityModule_InitTransport_priv();
    NEXUS_Module_Unlock( g_NEXUS_platformHandles.security );
    if (errCode) { errCode = BERR_TRACE(errCode); goto err; }
#endif

#if NEXUS_HAS_TRANSPORT
    BDBG_MSG((">VCXO"));
    errCode = NEXUS_Platform_P_InitVcxo();
    if ( errCode!=BERR_SUCCESS ) { errCode=BERR_TRACE(errCode); goto err; }
#endif

#if NEXUS_HAS_NSK2HDI
    BDBG_MSG((">NSK2HDI"));
    NEXUS_Nsk2hdiModule_GetDefaultSettings(&state->nsk2hdiSettings);
    state->nsk2hdiSettings.securityModule = g_NEXUS_platformHandles.security;
    state->nsk2hdiSettings.transportModule = g_NEXUS_platformHandles.transport;
    handle = NEXUS_Nsk2hdiModule_Init(&state->nsk2hdiSettings);
    if ( !handle )
    {
        BDBG_ERR(("Unable to init nsk2hdi"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_Nsk2hdiModule_Uninit, NULL);
#endif

#if NEXUS_HAS_SCM
    BDBG_MSG((">SCM"));
    g_NEXUS_platformHandles.scm = NEXUS_ScmModule_Init(NULL);
    if (!g_NEXUS_platformHandles.scm) {
        BDBG_ERR(("Unable to init scm"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    } else {
        NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.scm, NEXUS_ModuleStandbyLevel_eAll, NEXUS_ScmModule_Uninit, NULL);
    }
#endif

#if NEXUS_HAS_I2C
    /* Init I2C */
    BDBG_MSG((">I2C"));
    NEXUS_I2cModule_GetDefaultSettings(&state->i2cSettings);
    state->i2cSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
    g_NEXUS_platformHandles.i2c = NEXUS_I2cModule_Init(&state->i2cSettings);
    if ( !g_NEXUS_platformHandles.i2c ) {
        BDBG_ERR(("Unable to init I2C"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    else {
        NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.i2c, state->i2cSettings.common.standbyLevel, NEXUS_I2cModule_Uninit, NEXUS_I2cModule_Standby_priv);
    }
#endif


#if NEXUS_HAS_HDMI_INPUT
    BDBG_MSG((">HDMI_INPUT"));
    NEXUS_HdmiInputModule_GetDefaultSettings(&state->hdmiInputSettings);
#if NEXUS_HAS_SECURITY
    state->hdmiInputSettings.modules.security = g_NEXUS_platformHandles.security;
#endif
#if NEXUS_HAS_SAGE
    state->hdmiInputSettings.modules.sage = g_NEXUS_platformHandles.sage;
#endif
    g_NEXUS_platformHandles.hdmiInput = NEXUS_HdmiInputModule_Init(&state->hdmiInputSettings);
    if (!g_NEXUS_platformHandles.hdmiInput)
    {
        BDBG_ERR(("Unable to init hdmiInput"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.hdmiInput, NEXUS_ModuleStandbyLevel_eAll, NEXUS_HdmiInputModule_Uninit, NEXUS_HdmiInputModule_Standby_priv);
#endif


#if NEXUS_HAS_HDMI_OUTPUT
    BDBG_MSG((">HDMI_OUTPUT"));
    NEXUS_HdmiOutputModule_GetDefaultSettings(&state->hdmiSettings);
#if NEXUS_HAS_SECURITY
    state->hdmiSettings.modules.security = g_NEXUS_platformHandles.security;
#endif
#if NEXUS_HAS_SAGE
    state->hdmiSettings.modules.sage = g_NEXUS_platformHandles.sage;
#endif
#if NEXUS_HAS_HDMI_INPUT
    state->hdmiSettings.modules.hdmiInput = g_NEXUS_platformHandles.hdmiInput;
#endif

    g_NEXUS_platformHandles.hdmiOutput = NEXUS_HdmiOutputModule_Init(&state->hdmiSettings);
    if ( !g_NEXUS_platformHandles.hdmiOutput) {
        BDBG_ERR(("Unable to init HdmiOutput"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.hdmiOutput, NEXUS_ModuleStandbyLevel_eAll, NEXUS_HdmiOutputModule_Uninit, NEXUS_HdmiOutputModule_Standby_priv);
#endif

#if NEXUS_HAS_CEC
    BDBG_MSG((">CEC"));
    NEXUS_CecModule_GetDefaultSettings(&state->cecSettings);
    state->cecSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eAlwaysOn;
    state->cecSettings.hdmiOutput = g_NEXUS_platformHandles.hdmiOutput;
    g_NEXUS_platformHandles.cec = NEXUS_CecModule_Init(&state->cecSettings);
    if ( !g_NEXUS_platformHandles.cec) {
        BDBG_ERR(("Unable to init CEC"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.cec, state->cecSettings.common.standbyLevel, NEXUS_CecModule_Uninit, NEXUS_CecModule_Standby_priv);
#endif


#if NEXUS_HAS_HDMI_DVO
    BDBG_MSG((">HDMI_DVO"));
    g_NEXUS_platformHandles.hdmiDvo = NEXUS_HdmiDvoModule_Init(NULL);
    if (!g_NEXUS_platformHandles.hdmiDvo) {
        BDBG_ERR(("Unable to init HdmiDvo"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.hdmiDvo, NEXUS_ModuleStandbyLevel_eAll, NEXUS_HdmiDvoModule_Uninit, NULL);
#endif




#if NEXUS_HAS_RFM
    BDBG_MSG((">RFM"));
    g_NEXUS_platformHandles.rfm = NEXUS_RfmModule_Init(NULL);
    if (!g_NEXUS_platformHandles.rfm) {
        BDBG_ERR(("Unable to init Rfm"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.rfm, NEXUS_ModuleStandbyLevel_eAll, NEXUS_RfmModule_Uninit, NEXUS_RfmModule_Standby_priv);
#endif

#if NEXUS_HAS_SURFACE
    BDBG_MSG((">SURFACE"));
    NEXUS_SurfaceModule_GetDefaultSettings(&state->surfaceSettings);
    state->surfaceSettings.heapIndex = g_pCoreHandles->defaultHeapIndex;
    state->surfaceSettings.core = g_NEXUS_platformHandles.core;
    g_NEXUS_platformHandles.surface = NEXUS_SurfaceModule_Init(&state->surfaceSettings);
    if (!g_NEXUS_platformHandles.surface) {
        BDBG_ERR(("Unable to init Surface"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.surface, NEXUS_ModuleStandbyLevel_eAll, NEXUS_SurfaceModule_Uninit, NULL);
#endif


    /* Now that our init threads are launched, we can bring up modules in any order. */
#if NEXUS_HAS_SPI
    BDBG_MSG((">SPI"));
    NEXUS_SpiModule_GetDefaultSettings(&state->spiSettings);
    state->spiSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eAlwaysOn;
    handle = NEXUS_SpiModule_Init(&state->spiSettings);
    if ( !handle ) {
        BDBG_ERR(("Unable to init Spi"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->spiSettings.common.standbyLevel, NEXUS_SpiModule_Uninit, NULL);
#endif

    /* Other IO modules */
#if NEXUS_HAS_IR_INPUT
    BDBG_MSG((">IR_INPUT"));
    NEXUS_IrInputModule_GetDefaultSettings(&state->irInputSettings);
    state->irInputSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eAlwaysOn;
    handle = NEXUS_IrInputModule_Init(&state->irInputSettings);
    if ( !handle ) {
        BDBG_ERR(("Unable to init Ir Input"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->irInputSettings.common.standbyLevel, NEXUS_IrInputModule_Uninit, NEXUS_IrInputModule_Standby_priv);
#endif

#if NEXUS_HAS_INPUT_CAPTURE
    BDBG_MSG((">INPUT_CAPTURE"));
    NEXUS_InputCaptureModule_GetDefaultSettings(&state->inputCaptureSettings);
    state->inputCaptureSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eAlwaysOn;
    handle = NEXUS_InputCaptureModule_Init(&state->inputCaptureSettings);
    if ( !handle ) {
        BDBG_ERR(("Unable to init Input Capture"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->inputCaptureSettings.common.standbyLevel, NEXUS_InputCaptureModule_Uninit, NULL);
#endif

#if NEXUS_HAS_IR_BLASTER
    BDBG_MSG((">IR_BLASTER"));
    handle = NEXUS_IrBlasterModule_Init(NULL);
    if ( !handle ) {
        BDBG_ERR(("Unable to init Ir Blaster"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_IrBlasterModule_Uninit, NULL);
#endif

#if NEXUS_HAS_UHF_INPUT
    BDBG_MSG((">UHF_INPUT"));
    NEXUS_UhfInputModule_GetDefaultSettings(&state->uhfSettings);
    state->uhfSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
    handle = NEXUS_UhfInputModule_Init(&state->uhfSettings);
    if ( !handle ) {
        BDBG_ERR(("Unable to init Uhf Input"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->uhfSettings.common.standbyLevel, NEXUS_UhfInputModule_Uninit, NEXUS_UhfInputModule_Standby_priv);
#endif

#if NEXUS_HAS_GPIO
    BDBG_MSG((">GPIO"));
    NEXUS_GpioModule_GetDefaultSettings(&state->gpioSettings);
    state->gpioSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eAlwaysOn;
    NEXUS_Platform_P_GetGpioModuleOsSharedBankSettings(&state->gpioSettings.osSharedBankSettings);
    g_NEXUS_platformHandles.gpio = NEXUS_GpioModule_Init(&state->gpioSettings);
    if ( !g_NEXUS_platformHandles.gpio ) {
        BDBG_ERR(("Unable to init Gpio"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.gpio, state->gpioSettings.common.standbyLevel, NEXUS_GpioModule_Uninit, NEXUS_GpioModule_Standby_priv);
#endif

#if NEXUS_HAS_LED
    BDBG_MSG((">LED"));
    NEXUS_LedModule_GetDefaultSettings(&state->ledSettings);
    handle = NEXUS_LedModule_Init(&state->ledSettings);
    if ( !handle ) {
        BDBG_ERR(("Unable to init LED"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->ledSettings.common.standbyLevel, NEXUS_LedModule_Uninit, NULL);
#endif

#if NEXUS_HAS_KEYPAD
    NEXUS_KeypadModule_GetDefaultSettings(&state->keypadSettings);
    state->keypadSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eAlwaysOn;
    BDBG_MSG((">KEYPAD"));
    handle = NEXUS_KeypadModule_Init(&state->keypadSettings);
    if ( !handle ) {
        BDBG_ERR(("Unable to init Keypad"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->keypadSettings.common.standbyLevel, NEXUS_KeypadModule_Uninit, NULL);
#endif

#if NEXUS_HAS_UART
    BDBG_MSG((">UART"));
    NEXUS_UartModule_GetDefaultSettings(&state->uartSettings);
    handle = NEXUS_UartModule_Init(&state->uartSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init Uart"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->uartSettings.common.standbyLevel, NEXUS_UartModule_Uninit, NEXUS_UartModule_Standby_priv);
#endif

#if NEXUS_HAS_TOUCHPAD
    BDBG_ERR((">NEXUS_TOUCHPAD_SUPPORT"));
    handle = NEXUS_TouchpadModule_Init(NULL);
    if ( !handle ) {
        BDBG_ERR(("Unable to init Touchpad"));
        errCode=BERR_TRACE(errCode);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_TouchpadModule_Uninit, NULL);
#endif
#if NEXUS_HAS_FRONTEND
    BDBG_MSG((">FRONTEND"));
    NEXUS_FrontendModule_GetDefaultSettings(&state->frontendSettings);
    state->frontendSettings.i2cModule = g_NEXUS_platformHandles.i2c;
    state->frontendSettings.transport = g_NEXUS_platformHandles.transport;
    state->frontendSettings.common.standbyLevel = NEXUS_ModuleStandbyLevel_eActive;
    g_NEXUS_platformHandles.frontend = NEXUS_FrontendModule_Init(&state->frontendSettings);
    if ( !g_NEXUS_platformHandles.frontend )
    {
        BDBG_ERR(("Unable to init frontend"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.frontend, state->frontendSettings.common.standbyLevel, NEXUS_FrontendModule_Uninit, NEXUS_FrontendModule_Standby_priv);
#endif


#if NEXUS_HAS_AUDIO
    /* audio requires transport, hdmiOutput and rfm. so we can launch as early as here. */
#if MULTITHREADED_INIT && !NEXUS_NUM_ZSP_VIDEO_DECODERS && !NEXUS_NUM_DSP_VIDEO_DECODERS
    audioThread = NEXUS_Thread_Create("audio_init", NEXUS_Platform_P_InitAudio, NULL, NULL);
#else
    NEXUS_Platform_P_InitAudio(NULL);
    if (!g_NEXUS_platformHandles.audio) {
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
#endif
#endif

#if NEXUS_HAS_PICTURE_DECODER
    BDBG_MSG((">PICTURE_DECODER"));
    NEXUS_PictureDecoderModule_GetDefaultSettings(&state->pictureDecoderSettings);
    state->pictureDecoderSettings.surface = g_NEXUS_platformHandles.surface;
    state->pictureDecoderSettings.transport = g_NEXUS_platformHandles.transport;
    state->pictureDecoderSettings.security = g_NEXUS_platformHandles.security;
    g_NEXUS_platformHandles.pictureDecoder = NEXUS_PictureDecoderModule_Init(&state->pictureDecoderSettings);
    if (!g_NEXUS_platformHandles.pictureDecoder) {
        BDBG_ERR(("Unable to init PictureDecoder"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.pictureDecoder, NEXUS_ModuleStandbyLevel_eAll, NEXUS_PictureDecoderModule_Uninit, NEXUS_PictureDecoderModule_Standby_priv);
#endif

#if NEXUS_HAS_VIDEO_DECODER
    /* video decoder requires transport and secure heap. so we can launch as early as here. */
#if MULTITHREADED_INIT
    videoDecoderThread = NEXUS_Thread_Create("video_decoder_init", NEXUS_Platform_P_InitVideoDecoder, &state->videoDecoderSettings, NULL);
#else
    NEXUS_Platform_P_InitVideoDecoder(&state->videoDecoderSettings);
    if (!g_NEXUS_platformHandles.videoDecoder) {
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
#endif
#endif

#if NEXUS_HAS_DVB_CI
    BDBG_MSG((">DVB-CI"));

    errCode = NEXUS_Platform_P_InitDvbci();
    if (errCode != NEXUS_SUCCESS) {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }

    NEXUS_DvbCiModule_GetDefaultSettings(&state->dvbCiSettings);
    state->dvbCiSettings.modules.gpio = g_NEXUS_platformHandles.gpio;
    handle = NEXUS_DvbCiModule_Init(&state->dvbCiSettings);
    if ( !handle )
    {
        BDBG_ERR(("Unable to init DVB-CI module"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_DvbCiModule_Uninit, NULL);
#endif

#if NEXUS_HAS_GRAPHICS2D
    BDBG_MSG((">GRAPHICS2D"));
    NEXUS_Graphics2DModule_GetDefaultInternalSettings(&state->graphics2DSettings);
    state->graphics2DSettings.surface = g_NEXUS_platformHandles.surface;
    handle = NEXUS_Graphics2DModule_Init(&state->graphics2DSettings, &g_NEXUS_platformSettings.graphics2DModuleSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init Graphics2D"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_Graphics2DModule_Uninit, NEXUS_Graphics2DModule_Standby_priv);
#endif

#if NEXUS_HAS_SMARTCARD
    BDBG_MSG((">SMARTCARD"));
    NEXUS_SmartcardModule_GetDefaultInternalSettings(&state->smartcardSettings);
    handle = NEXUS_SmartcardModule_Init(&state->smartcardSettings, &g_NEXUS_platformSettings.smartCardSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init SmartCard"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, g_NEXUS_platformSettings.smartCardSettings.common.standbyLevel, NEXUS_SmartcardModule_Uninit, NEXUS_SmartcardModule_Standby_priv);
#endif

#if NEXUS_HAS_PWM
    BDBG_MSG((">PWM"));
    g_NEXUS_platformHandles.pwm = NEXUS_PwmModule_Init(&g_NEXUS_platformSettings.pwmSettings);
    if (!g_NEXUS_platformHandles.pwm) {
        BDBG_ERR(("NEXUS_PwmModule_Init failed"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.pwm, g_NEXUS_platformSettings.pwmSettings.common.standbyLevel, NEXUS_PwmModule_Uninit, NEXUS_PwmModule_Standby_priv);
#endif

#if NEXUS_HAS_TEMP_MONITOR
    BDBG_MSG((">TEMP_MONITOR"));
    NEXUS_TempMonitorModule_GetDefaultSettings(&state->tempMonitorSettings);
    handle = NEXUS_TempMonitorModule_Init(&state->tempMonitorSettings);
    if (!handle) {
        BDBG_ERR(("NEXUS_TempMonitorModule_Init failed"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->tempMonitorSettings.common.standbyLevel, NEXUS_TempMonitorModule_Uninit, NULL);
#endif

#if NEXUS_HAS_FILE
    BDBG_MSG((">FILE"));
    g_NEXUS_platformHandles.file = NEXUS_FileModule_Init(&g_NEXUS_platformSettings.fileModuleSettings);
    if (!g_NEXUS_platformHandles.file) {
        BDBG_ERR(("Unable to init File"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.file, g_NEXUS_platformSettings.fileModuleSettings.common.standbyLevel, NEXUS_FileModule_Uninit, NULL);
#endif

#if NEXUS_HAS_GRAPHICSV3D
    BDBG_MSG((">GRAPHICSV3D"));
    NEXUS_Graphicsv3dModule_GetDefaultSettings(&state->graphicsv3dSettings);
    state->graphicsv3dSettings.hHeap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
#if NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX
    state->graphicsv3dSettings.hHeapSecure = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE);
#endif
    handle = NEXUS_Graphicsv3dModule_Init(&state->graphicsv3dSettings);
    if (!handle) {
        BDBG_ERR(("Unable to initialize V3D graphics"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_Graphicsv3dModule_Uninit, NEXUS_Graphicsv3d_Standby_priv);
#endif

#if NEXUS_HAS_DISPLAY
    BDBG_MSG((">DISPLAY"));
    NEXUS_DisplayModule_GetDefaultInternalSettings(&state->displaySettings);
    state->displaySettings.modules.surface = g_NEXUS_platformHandles.surface;
    /* don't set pDisplaySettings->modules.videoDecoder here */
#if NEXUS_HAS_HDMI_OUTPUT
    state->displaySettings.modules.hdmiOutput = g_NEXUS_platformHandles.hdmiOutput;
#endif
#if NEXUS_HAS_HDMI_DVO
    state->displaySettings.modules.hdmiDvo = g_NEXUS_platformHandles.hdmiDvo;
#endif
#if NEXUS_HAS_HDMI_INPUT
    state->displaySettings.modules.hdmiInput = g_NEXUS_platformHandles.hdmiInput;
#endif
#if NEXUS_HAS_RFM
    state->displaySettings.modules.rfm = g_NEXUS_platformHandles.rfm;
#endif
#if NEXUS_HAS_PWM
    state->displaySettings.modules.pwm = g_NEXUS_platformHandles.pwm;
#endif
    state->displaySettings.modules.transport = g_NEXUS_platformHandles.transport;
    g_NEXUS_platformHandles.display = NEXUS_DisplayModule_Init(&state->displaySettings, &g_NEXUS_platformSettings.displayModuleSettings);
    if (!g_NEXUS_platformHandles.display) {
        BDBG_ERR(("Unable to init Display"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
#endif


#if MULTITHREADED_INIT && NEXUS_HAS_AUDIO
    /* audio is needed for sync, so join here */
    if (audioThread) {
        NEXUS_Thread_Destroy(audioThread);
        audioThread = NULL;
        if (!g_NEXUS_platformHandles.audio) {
            errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err;
        }
    }
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    BDBG_MSG((">SYNC_CHANNEL"));
    NEXUS_SyncChannelModule_GetDefaultSettings(&state->syncChannelSettings);
    state->syncChannelSettings.modules.display = g_NEXUS_platformHandles.display;
    state->syncChannelSettings.modules.audio = g_NEXUS_platformHandles.audio;
    handle = NEXUS_SyncChannelModule_Init(&state->syncChannelSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init SyncChannel"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    } else {
        NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_SyncChannelModule_Uninit, NULL);
    }
#endif

#if MULTITHREADED_INIT && NEXUS_HAS_VIDEO_DECODER
    /* videoDecoder is needed for astm, so join here */
    if (videoDecoderThread) {
        NEXUS_Thread_Destroy(videoDecoderThread);
        videoDecoderThread = NULL;
        if (!g_NEXUS_platformHandles.videoDecoder) {
            errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto err;
        }
    }
#endif

#if NEXUS_HAS_VIDEO_DECODER
#if NEXUS_HAS_DISPLAY
    NEXUS_DisplayModule_SetVideoDecoderModule(g_NEXUS_platformHandles.videoDecoder);
#endif
    /* Video Decoder and Display need to go in this order */
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.videoDecoder, NEXUS_ModuleStandbyLevel_eAll, NEXUS_VideoDecoderModule_Uninit, NEXUS_VideoDecoderModule_Standby_priv);
#endif
#if NEXUS_HAS_DISPLAY
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.display, NEXUS_ModuleStandbyLevel_eAll, NEXUS_DisplayModule_Uninit, NEXUS_DisplayModule_Standby_priv);
#endif

#if NEXUS_HAS_VIDEO_ENCODER
    BDBG_MSG((">VIDEO_ENCODER"));
    state->videoEncoderSettings = g_NEXUS_platformInternalSettings.videoEncoderSettings;

    state->videoEncoderSettings.display = g_NEXUS_platformHandles.display;
    state->videoEncoderSettings.transport = g_NEXUS_platformHandles.transport;
    state->videoEncoderSettings.audio = g_NEXUS_platformHandles.audio;
    state->videoEncoderSettings.core = g_NEXUS_platformHandles.core;
    state->videoEncoderSettings.security = g_NEXUS_platformHandles.security;
    g_NEXUS_platformHandles.videoEncoder = NEXUS_VideoEncoderModule_Init(&state->videoEncoderSettings, &g_NEXUS_platformSettings.videoEncoderSettings);
    if(!g_NEXUS_platformHandles.videoEncoder) {
        BDBG_ERR(("NEXUS_VideoEncoderModule_Init failed"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.videoEncoder, NEXUS_ModuleStandbyLevel_eAll, NEXUS_VideoEncoderModule_Uninit, NEXUS_VideoEncoderModule_Standby_priv);
#endif

#if NEXUS_HAS_STREAM_MUX
    BDBG_MSG((">STREAM_MUX"));
    NEXUS_StreamMuxModule_GetDefaultSettings(&state->streamMuxModuleSettings);
    state->streamMuxModuleSettings.transport = g_NEXUS_platformHandles.transport;
    state->streamMuxModuleSettings.audio = g_NEXUS_platformHandles.audio;
    state->streamMuxModuleSettings.videoEncoder = g_NEXUS_platformHandles.videoEncoder;
    state->streamMuxModuleSettings.core = g_NEXUS_platformHandles.core;
    handle = NEXUS_StreamMuxModule_Init(&state->streamMuxModuleSettings);
    if(!handle) {
        BDBG_ERR(("NEXUS_StreamMuxModule_Init: failed"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_StreamMuxModule_Uninit, NULL);
#endif

#if NEXUS_HAS_ASTM
    BDBG_MSG((">ASTM"));
    NEXUS_AstmModule_GetDefaultSettings(&state->astmSettings);
    state->astmSettings.modules.videoDecoder = g_NEXUS_platformHandles.videoDecoder;
    state->astmSettings.modules.audio = g_NEXUS_platformHandles.audio;
    state->astmSettings.modules.transport = g_NEXUS_platformHandles.transport;
    handle = NEXUS_AstmModule_Init(&state->astmSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init Astm"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_AstmModule_Uninit, NULL);
#endif

#if NEXUS_HAS_PLAYBACK
    BDBG_MSG((">PLAYBACK"));
    NEXUS_PlaybackModule_GetDefaultSettings(&state->playbackSettings);
    state->playbackSettings.modules.file = g_NEXUS_platformHandles.file;
    state->playbackSettings.modules.audioDecoder = g_NEXUS_platformHandles.audio;
    state->playbackSettings.modules.videoDecoder = g_NEXUS_platformHandles.videoDecoder;
    state->playbackSettings.modules.playpump = g_NEXUS_platformHandles.transport;
    g_NEXUS_platformHandles.playback = NEXUS_PlaybackModule_Init(&state->playbackSettings);
    if (!g_NEXUS_platformHandles.playback) {
        BDBG_ERR(("Unable to init playback"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(g_NEXUS_platformHandles.playback, NEXUS_ModuleStandbyLevel_eAll, NEXUS_PlaybackModule_Uninit, NULL);
#endif

#if NEXUS_HAS_RECORD
    BDBG_MSG((">RECORD"));
    NEXUS_RecordModule_GetDefaultSettings(&state->recordSettings);
    state->recordSettings.modules.file = g_NEXUS_platformHandles.file;
    state->recordSettings.modules.recpump = g_NEXUS_platformHandles.transport;
#if NEXUS_HAS_PLAYBACK
    state->recordSettings.modules.playback = g_NEXUS_platformHandles.playback;
#endif
    handle = NEXUS_RecordModule_Init(&state->recordSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init record"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->recordSettings.common.standbyLevel, NEXUS_RecordModule_Uninit, NULL);
#endif

#if NEXUS_HAS_FILE_MUX
    BDBG_MSG((">FILE_MUX"));
    handle = NEXUS_FileMuxModule_Init(NULL);
    if (!handle) {
        BDBG_ERR(("Unable to init File Mux"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_FileMuxModule_Uninit, NULL);
#endif

#if NEXUS_HAS_SIMPLE_AUDIO_PLAYBACK
    BDBG_MSG((">SIMPLE_AUDIO_PLAYBACK"));
    NEXUS_SimpleAudioPlaybackModule_GetDefaultSettings(&state->simpleAudioPlaybackSettings);
    state->simpleAudioPlaybackSettings.modules.audio = g_NEXUS_platformHandles.audio;
    simpleAudioPlayback = NEXUS_SimpleAudioPlaybackModule_Init(&state->simpleAudioPlaybackSettings);
    if (!simpleAudioPlayback) {
        BDBG_ERR(("Unable to init Simple Audio Playback"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(simpleAudioPlayback, NEXUS_ModuleStandbyLevel_eAll, NEXUS_SimpleAudioPlaybackModule_Uninit, NULL);
#endif

#if NEXUS_HAS_SIMPLE_DECODER
    BDBG_MSG((">SIMPLE_DECODER"));
    NEXUS_SimpleDecoderModule_GetDefaultSettings(&state->simpleDecoderSettings);
    state->simpleDecoderSettings.modules.videoDecoder = g_NEXUS_platformHandles.videoDecoder;
    state->simpleDecoderSettings.modules.display = g_NEXUS_platformHandles.display;
    state->simpleDecoderSettings.modules.audio = g_NEXUS_platformHandles.audio;
#if NEXUS_HAS_SIMPLE_AUDIO_PLAYBACK
    state->simpleDecoderSettings.modules.simpleAudioPlayback = simpleAudioPlayback;
#endif
    handle = NEXUS_SimpleDecoderModule_Init(&state->simpleDecoderSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init Simple Decoder"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_SimpleDecoderModule_Uninit, NULL);
#endif

#if NEXUS_HAS_SURFACE_COMPOSITOR
    BDBG_MSG((">SURFACE_COMPOSITOR"));
    NEXUS_SurfaceCompositorModule_GetDefaultSettings(&state->surfaceCompositorSettings);
    state->surfaceCompositorSettings.modules.display = g_NEXUS_platformHandles.display;
    handle = NEXUS_SurfaceCompositorModule_Init(&state->surfaceCompositorSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init SurfaceCompositor"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, NEXUS_ModuleStandbyLevel_eAll, NEXUS_SurfaceCompositorModule_Uninit, NEXUS_SurfaceCompositorModule_Standby_priv);
#endif

#if NEXUS_HAS_INPUT_ROUTER
    BDBG_MSG((">INPUT_ROUTER"));
    NEXUS_InputRouterModule_GetDefaultSettings(&state->inputRouterSettings);
    handle = NEXUS_InputRouterModule_Init(&state->inputRouterSettings);
    if (!handle) {
        BDBG_ERR(("Unable to init InputRouter"));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err;
    }
    NEXUS_Platform_P_AddModule(handle, state->inputRouterSettings.common.standbyLevel, NEXUS_InputRouterModule_Uninit, NULL);
#endif

    /* all modules are up, so we can bring up IPC server before opening interfaces. */
    NEXUS_Platform_P_StartServer();

    BDBG_MSG((">CONFIG"));
    errCode = NEXUS_Platform_P_Config(&g_NEXUS_platformSettings);
    if ( errCode!=BERR_SUCCESS ) { errCode = BERR_TRACE(errCode); goto err_config; }

#if NEXUS_HAS_FRONTEND
    if (g_NEXUS_platformSettings.openFrontend) {
        BDBG_MSG((">FRONTEND CONFIG"));
        errCode = NEXUS_Platform_InitFrontend();
        if ( errCode!=BERR_SUCCESS ) {
            BDBG_ERR(("Unable to init frontend. Allowing Nexus to still come up."));
        }
    }
#endif

    g_NEXUS_CallbackMonitorTimer = NEXUS_ScheduleTimerByPriority(Internal, 100, NEXUS_P_Platform_CallbackMonitor, NULL);
    BDBG_ASSERT(g_NEXUS_CallbackMonitorTimer);

    if (NEXUS_GetEnv("debug_mem")) {
        NEXUS_Memory_PrintStatus();/* print global memory status */
    }

    /* Success */
    if(NEXUS_GetEnv("profile_init")) {
        NEXUS_Profile_Stop("NEXUS_Platform_Init");
    }

base_only_init:
    BKNI_Free(state);
    BDBG_WRN(("initialized"));
    NEXUS_UnlockModule();

    BDBG_MSG((">THERMAL MONITOR"));
    errCode = NEXUS_Platform_P_InitializeThermalMonitor();
    if ( errCode!=BERR_SUCCESS ) {
        BDBG_ERR(("Unable to init thermal monitor. Allowing Nexus to still come up."));
    }

    return NEXUS_SUCCESS;

err_config:
    NEXUS_Platform_P_StopServer();
err:
    NEXUS_Platform_P_UninitModules();
    NEXUS_Platform_P_UninitInterrupts();
#if NEXUS_HAS_DVB_CI
    NEXUS_Platform_P_UninitDvbci();
#endif
err_postboard:
    NEXUS_Platform_P_UninitBoard();
err_initboard:
    NEXUS_Platform_P_UninitCore();
err_core:
#if NEXUS_HAS_SURFACE
    NEXUS_SurfaceModule_LocalUninit();
err_surface_local:
#endif
    NEXUS_CoreModule_LocalUninit();
err_core_local:
    NEXUS_Platform_P_UninitOS();
err_os:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_platformModule);
    g_NEXUS_platformModule = NULL;
err_plaform_module:
    NEXUS_Base_Uninit();
err_base:
    b_objdb_uninit();
err_objdb_init:
err_memconfig:
    BKNI_Free(state);
err_state_alloc:
    NEXUS_Platform_P_PreUninit();
err_preinit:
    BDBG_ASSERT(errCode); /* if we've taken this path, it's only because of failure */
    BDBG_ERR(("NEXUS_Platform_Init has failed. The system is not usable."));
    return errCode;
}

/***************************************************************************
Summary:
    Uninitialize Nexus
See Also:
    NEXUS_Platform_Init
 ***************************************************************************/
void NEXUS_Platform_Uninit(void)
{
    if (g_NEXUS_platformModule == NULL) {
        /* nexus is already uninitialized. nothing to do.
        do not use BERR or BDBG here. */
        return;
    }

    NEXUS_Platform_P_UninitializeThermalMonitor();

    NEXUS_LockModule();
    if (g_NEXUS_platformHandles.baseOnlyInit) {
        goto base_only_init;
    }
    if(g_NEXUS_CallbackMonitorTimer) {
        NEXUS_CancelTimer(g_NEXUS_CallbackMonitorTimer);
        g_NEXUS_CallbackMonitorTimer=NULL;
    }

#if NEXUS_POWER_MANAGEMENT
    g_NEXUS_platformSettings.standbySettings.mode = NEXUS_StandbyMode_eOn;
    NEXUS_Platform_SetStandbySettings_driver(&g_NEXUS_platformSettings.standbySettings);
#endif
#if NEXUS_HAS_FRONTEND
    /* shutdown whatever platform opened */
    NEXUS_Platform_UninitFrontend();
#endif

    /* close handles opened by NEXUS_Platform_P_Config (not in objdb) */
    NEXUS_Platform_P_Shutdown();

    /* close clients, close handles in objdb */
    NEXUS_Platform_P_StopServer();

    NEXUS_UnlockModule();
    NEXUS_Base_Stop();
    NEXUS_LockModule();

#ifdef BCHP_PWR_RESOURCE_BINT_OPEN
    BCHP_PWR_AcquireResource(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_BINT_OPEN);
#endif

base_only_init: /* base only still requires uninit of core */
    NEXUS_Platform_P_UninitModules();

#if NEXUS_HAS_DVB_CI
    NEXUS_Platform_P_UninitDvbci();
#endif

    NEXUS_Platform_P_UninitInterrupts();
#ifdef BCHP_PWR_RESOURCE_BINT_OPEN
    BCHP_PWR_ReleaseResource(g_NEXUS_pCoreHandles->chp, BCHP_PWR_RESOURCE_BINT_OPEN);
#endif

    BDBG_MSG(("<UNINIT_BOARD"));
    NEXUS_Platform_P_UninitBoard();

    if (NEXUS_GetEnv("debug_mem")) {
        NEXUS_Memory_PrintStatus();/* print global memory status */
    }

    BDBG_MSG(("<CORE"));
    NEXUS_Platform_P_UninitCore();
    BDBG_MSG(("<OS"));
    NEXUS_Platform_P_UninitOS();

    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_platformModule);
    g_NEXUS_platformModule = NULL;
    BKNI_Memset(&g_NEXUS_platformSettings, 0, sizeof(g_NEXUS_platformSettings));

#if NEXUS_HAS_SURFACE
    BDBG_MSG(("<SURFACE_LOCAL"));
    NEXUS_SurfaceModule_LocalUninit();
#endif

    BDBG_MSG(("<CORE_LOCAL"));
    NEXUS_CoreModule_LocalUninit();

    BDBG_MSG(("<BASE"));
    NEXUS_Base_Uninit();
    BDBG_MSG(("<OBJDB"));
    b_objdb_uninit();
    BDBG_MSG(("<MAGNUM"));
    NEXUS_Platform_P_PreUninit();

    return;
}

void NEXUS_Platform_GetConfiguration(NEXUS_PlatformConfiguration *pConfiguration)
{
    /* verify platform module lock is enabled */
    NEXUS_ASSERT_MODULE();
    *pConfiguration = g_NEXUS_platformHandles.config;
    return;
}

NEXUS_Error NEXUS_Platform_ReadRegister( uint32_t address, uint32_t *pValue )
{
    *pValue = BREG_Read32(g_pCoreHandles->reg, address);
    return 0;
}

void NEXUS_Platform_WriteRegister( uint32_t address, uint32_t value )
{
    BREG_Write32(g_pCoreHandles->reg, address, value);
}

NEXUS_Error NEXUS_Platform_GetStatus( NEXUS_PlatformStatus *pStatus )
{
    unsigned i, id;
    BCHP_Info info;

    BDBG_ASSERT(g_NEXUS_platformModule); /* make sure NEXUS_Platform_Init was called */

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    BCHP_GetInfo(g_pCoreHandles->chp, &info);
    pStatus->familyId = info.familyId;
    pStatus->chipRevision = info.rev + 0x10; /* convert from magnum's 00 = A0 to nexus's 10 = A0 */
    pStatus->chipId = info.productId;
    pStatus->boxMode = g_NEXUS_platformHandles.rtsSettings.boxMode;
    pStatus->estimatedMemory = g_NEXUS_platformHandles.estimatedMemory;

    /* modify struct with board specific status */
    id = NEXUS_Platform_P_ReadBoardId();
    pStatus->boardId.major = (id >> 4) & 0xF;
    pStatus->boardId.minor = id & 0xF;

#if NEXUS_HAS_DISPLAY
    if (g_NEXUS_platformHandles.display
#if NEXUS_POWER_MANAGEMENT
            && g_standbyState.settings.mode == NEXUS_StandbyMode_eOn
#endif
            ) {
        int rc;
        NEXUS_Module_Lock(g_NEXUS_platformHandles.display);
        rc = NEXUS_DisplayModule_GetStatus_priv(&pStatus->displayModuleStatus);
        NEXUS_Module_Unlock(g_NEXUS_platformHandles.display);
        if (rc) return BERR_TRACE(rc);
    }
#endif

    for (i=0;i<NEXUS_MAX_MEMC;i++) {
        pStatus->memc[i].size = g_platformMemory.memoryLayout.memc[i].size;
    }

    return 0;
}

void NEXUS_Platform_IsLicensedFeatureSupported(NEXUS_LicensedFeature feature, bool *pSupported)
{
    BERR_Code rc;
    BDBG_CASSERT(BCHP_LicensedFeature_eMax == (BCHP_LicensedFeature)NEXUS_LicensedFeature_eMax);

    if (feature >= NEXUS_LicensedFeature_eMax) {
        *pSupported = false;
        return;
    }
    rc = BCHP_HasLicensedFeature_isrsafe(g_pCoreHandles->chp, (BCHP_LicensedFeature)feature);
    *pSupported = (rc==BERR_SUCCESS);
    return;
}

#include "b_objdb.h"
/* implementation of attr{shutdown=NEXUS_VideoInput_Shutdown} */
void nexus_driver_shutdown_NEXUS_VideoInput_Shutdown(void *object)
{
#if NEXUS_HAS_DISPLAY
    if (object) NEXUS_VideoInput_Shutdown(object);
#else
    BSTD_UNUSED(object);
#endif
}

/* implementation of attr{shutdown=NEXUS_VideoOutput_Shutdown} */
void nexus_driver_shutdown_NEXUS_VideoOutput_Shutdown(void *object)
{
#if NEXUS_HAS_DISPLAY
    if (object) NEXUS_VideoOutput_Shutdown(object);
#else
    BSTD_UNUSED(object);
#endif
}

/* implementation of attr{shutdown=NEXUS_AudioInput_Shutdown} */
void nexus_driver_shutdown_NEXUS_AudioInput_Shutdown(void *object)
{
#if NEXUS_HAS_AUDIO
    if (object) NEXUS_AudioInput_Shutdown(object);
#else
    BSTD_UNUSED(object);
#endif
}

/* implementation of attr{shutdown=NEXUS_AudioOutput_Shutdown} */
void nexus_driver_shutdown_NEXUS_AudioOutput_Shutdown(void *object)
{
#if NEXUS_HAS_AUDIO
    if (object) NEXUS_AudioOutput_Shutdown(object);
#else
    BSTD_UNUSED(object);
#endif
}

void NEXUS_Platform_GetDefaultClientConfiguration(NEXUS_ClientConfiguration *pConfig)
{
    BKNI_Memset(pConfig, 0, sizeof(*pConfig));
    pConfig->mode = NEXUS_ClientMode_eProtected;
    pConfig->resources.dma.total = 4;
    pConfig->resources.graphics2d.total = 2;
    pConfig->resources.graphicsv3d.total = 2;
    pConfig->resources.pictureDecoder.total = 2;
    pConfig->resources.playpump.total = 2;
    pConfig->resources.recpump.total = 2;
    pConfig->resources.simpleAudioPlayback.total = 2;
    pConfig->resources.simpleStcChannel.total = 2;
    pConfig->resources.surface.total = 100;
    pConfig->resources.temporaryMemory.sizeLimit = 128*1024;
    return;
}

void NEXUS_Platform_GetDefaultStartServerSettings( NEXUS_PlatformStartServerSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->allowUnprotectedClientsToCrash = true;
    NEXUS_Platform_GetDefaultClientConfiguration(&pSettings->unauthenticatedConfiguration);
}

void NEXUS_Platform_GetDefaultClientSettings( NEXUS_ClientSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_Platform_GetDefaultClientConfiguration(&pSettings->configuration);
}

void NEXUS_Platform_GetDefaultInterfaceName(NEXUS_InterfaceName *type)
{
    BKNI_Memset(type, 0, sizeof(*type));
    return;
}

NEXUS_Error NEXUS_Platform_AcquireObject(NEXUS_ClientHandle clientHandle, const NEXUS_InterfaceName *type, void *object)
{
    const NEXUS_Platform_P_ModuleInfo *module_info;
    const struct b_objdb_client *client = nexus_p_platform_objdb_client(clientHandle);

    for (module_info = BLST_Q_FIRST(&g_NEXUS_platformHandles.handles); module_info; module_info = BLST_Q_NEXT(module_info, link)) {
        NEXUS_BaseObject  *base_object = b_objdb_find_object_and_acquire(client, type->name, object);
        if(base_object) {
            return NEXUS_SUCCESS;
        }
    }
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

void NEXUS_Platform_ReleaseObject(const NEXUS_InterfaceName *type, void *object)
{
    const NEXUS_Platform_P_ModuleInfo *module_info;
    for (module_info = BLST_Q_FIRST(&g_NEXUS_platformHandles.handles); module_info; module_info = BLST_Q_NEXT(module_info, link)) {
        NEXUS_BaseObject  *base_object = b_objdb_find_object_and_acquire(b_objdb_get_client(), type->name, object);
        if(base_object) {
            /* release it twice, first to compensate for acquire in b_objdb and then to release it */
#if BDBG_DEBUG_BUILD
            NEXUS_BaseObject_P_Release_Tagged(NULL, object, base_object->descriptor, NEXUS_MODULE_SELF, BSTD_FILE, BSTD_LINE);
            NEXUS_BaseObject_P_Release_Tagged(NULL, object, base_object->descriptor, NEXUS_MODULE_SELF, BSTD_FILE, BSTD_LINE); /* second required */
#else
            NEXUS_BaseObject_P_Release(NULL, object, base_object->descriptor, NEXUS_MODULE_SELF);
            NEXUS_BaseObject_P_Release(NULL, object, base_object->descriptor, NEXUS_MODULE_SELF); /* second required */
#endif
            return ;
        }
    }
    (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
    return ;
}

NEXUS_Error NEXUS_Platform_SetSharedHandle( void *object, bool shared )
{
    return b_objdb_set_object_shared(b_objdb_get_client(), object, shared);
}

NEXUS_Error NEXUS_Platform_GetClientObjects(NEXUS_ClientHandle client, const NEXUS_InterfaceName *type, NEXUS_PlatformObjectInstance *objects, size_t nobjects, size_t *pValidObjects)
{
    NEXUS_Error rc;

    NEXUS_ASSERT_STRUCTURE(NEXUS_PlatformObjectInstance, b_objdb_object_instance);
    NEXUS_ASSERT_FIELD(NEXUS_PlatformObjectInstance, object, b_objdb_object_instance, object);
    NEXUS_ASSERT_FIELD(NEXUS_PlatformObjectInstance, readOnly, b_objdb_object_instance, readOnly);
    BDBG_ASSERT(pValidObjects);
    if (b_objdb_get_client()->mode >= NEXUS_ClientMode_eProtected && !NEXUS_StrCmp(type->name, "NEXUS_Client")) {
        /* Any NEXUS_ClientHandle is owned by the server, even for the client's own handle (counter-intuitive).
        And any client accessing another client's handles must be NEXUS_ClientMode_eVerified, and this is usually the first query,
        so we provide feedback instead of silently returning nothing. */
        BDBG_ERR(("NEXUS_Platform_GetClientObjects will return no NEXUS_Client handles unless caller is NEXUS_ClientMode_eVerified"));
    }
    rc = b_objdb_get_object_list(b_objdb_get_client(), client?nexus_p_platform_objdb_client(client):NULL, type->name, (b_objdb_object_instance *)objects, nobjects, pValidObjects);
    switch(rc) {
    default:
    case b_objdb_get_object_list_result_no_objects:
        *pValidObjects = 0;
        return NEXUS_SUCCESS;
    case b_objdb_get_object_list_result_done:
        return NEXUS_SUCCESS;
    case b_objdb_get_object_list_result_overflow:
        return NEXUS_PLATFORM_ERR_OVERFLOW; /* no BERR_TRACE, could be normal */
    }
}

BDBG_FILE_MODULE(nexus_api);

#define BDBG_MSG_TRACE_API(x) /* BDBG_MODULE_MSG(nexus_api, x) */


#if NEXUS_HAS_TRANSPORT
extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR(NEXUS_Timebase);
extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR(NEXUS_ParserBand);
#endif
extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR(NEXUS_Heap);

#if defined(NEXUS_ABICOMPAT_MODE) || !defined(NEXUS_MODE_driver)
#define B_TEST_FOR_NULL(ptr,offset) ((offset>=0) && (*(bool *)((uint8_t *)(ptr)+ (offset))))
#else
#define B_TEST_FOR_NULL(ptr,offset) ((offset>=0) && (*(void **)((uint8_t *)(ptr)+ (offset))==NULL))
#endif


static NEXUS_Error nexus_p_api_verify_heap(const struct b_objdb_client *client, NEXUS_HeapHandle heap, NEXUS_HeapHandle *pHeap )
{
    unsigned i;

    /* allow NULL heap to pass through so driver can resolve proper default type */
    if (!heap) {
        *pHeap = NULL;
        return NEXUS_SUCCESS;
    }

    /* verify client's heap */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (heap == ((client->config.mode <= NEXUS_ClientMode_eVerified) ? g_pCoreHandles->heap[i].nexus : client->config.heap[i])) {
            *pHeap = heap;
            return NEXUS_SUCCESS;
        }
    }

    BDBG_ERR(("client %p tried to use invalid heap %p, but was rejected", (void *)client, (void *)heap));
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

static NEXUS_Error nexus_p_api_acquire_object(const struct b_objdb_client *client, const struct api_function_descriptor *function, const struct api_object_descriptor *object, void *in_data)
{
    void *handle;

    if(B_TEST_FOR_NULL(in_data, object->null_offset)) {
        handle = NULL;
    } else {
        handle = *(void **)((uint8_t *)in_data + object->offset);
    }
    if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_Heap)) {
        return nexus_p_api_verify_heap(client, handle, (NEXUS_HeapHandle *)((void **)((uint8_t *)in_data + object->offset)));
    }
#if NEXUS_HAS_TRANSPORT
    else if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_Timebase)) {
        if (client->mode < NEXUS_ClientMode_eProtected) {
            return NEXUS_SUCCESS;
        }
        if( handle == NULL) {
            *(NEXUS_Timebase *)((void **)((uint8_t *)in_data + object->offset)) = NEXUS_Timebase_eInvalid;
        }
    }
    else if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_ParserBand)) {
        if (client->mode < NEXUS_ClientMode_eProtected) {
            return NEXUS_SUCCESS;
        }
        if( handle == NULL) {
            *(NEXUS_ParserBand *)((void **)((uint8_t *)in_data + object->offset)) = NEXUS_ParserBand_eInvalid;
        }
    }
#endif
    if(object->null_allowed && handle==NULL) {
        return NEXUS_SUCCESS;
    }
    if(b_objdb_verify_and_acquire(object->descriptor, handle)!=0) {
        BDBG_ERR(("%s:%s %p not allowed", function->name, object->name, (void *)handle));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return NEXUS_SUCCESS;
}

static void nexus_p_api_release_object(const struct b_objdb_client *client, NEXUS_ModuleHandle module, const struct api_object_descriptor *object, void *in_data)
{
    void *handle;

    if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_Heap)) {
        return;
    }

    if(B_TEST_FOR_NULL(in_data, object->null_offset)) {
        handle = NULL;
    } else {
        handle = *(void **)((uint8_t *)in_data + object->offset);
    }
#if NEXUS_HAS_TRANSPORT
    if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_Timebase)) {
        if (client->mode < NEXUS_ClientMode_eProtected) {
            return;
        }
        if( (NEXUS_Timebase ) handle == NEXUS_Timebase_eInvalid) {
            handle = NULL;
        }
    } else if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_ParserBand)) {
        if (client->mode < NEXUS_ClientMode_eProtected) {
            return;
        }
        if( (NEXUS_ParserBand ) handle == NEXUS_ParserBand_eInvalid) {
            handle = NULL;
        }
    }
#else
    BSTD_UNUSED(client);
#endif
    if(handle) {
#if BDBG_DEBUG_BUILD
        NEXUS_BaseObject_P_Release_Tagged(NULL, handle, object->descriptor, module, "", 0);
#else
        NEXUS_BaseObject_P_Release(NULL, handle, object->descriptor, module);
#endif
    }
    return;
}


NEXUS_Error nexus_p_api_call_verify(const struct b_objdb_client *client, NEXUS_ModuleHandle module, const struct api_function_descriptor *function, void *in_data
#if defined(NEXUS_ABICOMPAT_MODE) || !defined(NEXUS_MODE_driver)
        , unsigned in_data_size, unsigned out_mem_size
#endif
        )
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    unsigned i;
    BSTD_UNUSED(in_data);
    BDBG_ASSERT(function);

    BDBG_MSG_TRACE_API((">verify:%p:%s", client, function->name));
    if(client && client->mode == NEXUS_ClientMode_eUntrusted && function->limited) {
        BDBG_ERR(("%s:not supported for client %p", function->name, (void *)client));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto done;
    }
#if defined(NEXUS_ABICOMPAT_MODE) || !defined(NEXUS_MODE_driver)
    if(in_data_size < function->in_buf_size) {
        BDBG_ERR(("%s:short in data %d < %d", function->name, in_data_size, function->in_buf_size));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done;
    }
    if(out_mem_size < function->out_buf_size) {
        BDBG_ERR(("%s:short out size %d < %d", function->name, out_mem_size, function->out_buf_size));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done;
    }
#endif
    if(client==NULL) { goto done; }

    for(i=0;i<function->pointer_cnt;i++) {
        const struct api_pointer_descriptor *pointer = &function->pointers[i];
        if(!pointer->null_allowed) {
            if( B_TEST_FOR_NULL(in_data,pointer->null_offset)) {
                BDBG_ERR(("%s:%s NULL not allowed", function->name, pointer->name));
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);goto done;
            }
        }
    }
    for(i=0;i<function->object_cnt;i++) {
        const struct api_object_descriptor *object= &function->objects[i];
        if(!object->inparam) {
            continue;
        }
        rc = nexus_p_api_acquire_object(client, function, object, in_data);
        if(rc!=NEXUS_SUCCESS) {
            /* unwind  */
            unsigned j;
            for(j=0;j<i;j++) {
                object= &function->objects[j];
                nexus_p_api_release_object(client, module, object, in_data);
            }
            goto done;
        }
    }
done:
    BDBG_MSG_TRACE_API(("<verify %p:%s -> %d", client, function->name, rc));
    return rc;
}

void nexus_p_api_call_completed(const struct b_objdb_client *client, NEXUS_ModuleHandle module, const struct api_function_descriptor *function, void *in_data
#if defined(NEXUS_ABICOMPAT_MODE) || !defined(NEXUS_MODE_driver)
        , void *out_data
#endif
        )
{
    unsigned i;

    if(client==NULL) { goto done; }

    for(i=0;i<function->object_cnt;i++) {
        const struct api_object_descriptor *object= &function->objects[i];
#if defined(NEXUS_MODE_driver) && !defined(NEXUS_ABICOMPAT_MODE)
        void *out_data = in_data;
#endif
        if(!object->inparam) {
#if NEXUS_HAS_TRANSPORT
            void **handle = (void **)((uint8_t *)out_data + object->offset);
            if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_Timebase)) {
                if(client->mode >= NEXUS_ClientMode_eProtected && *(NEXUS_Timebase*)handle == NEXUS_Timebase_eInvalid) {
                    *handle = NULL;
                }
            } else if(object->descriptor == &NEXUS_OBJECT_DESCRIPTOR(NEXUS_ParserBand)) {
                if(client->mode >= NEXUS_ClientMode_eProtected && *(NEXUS_ParserBand*)handle == NEXUS_ParserBand_eInvalid) {
                    *handle = NULL;
                }
            }
#else
    BSTD_UNUSED(out_data);
#endif
            continue;
        }
        nexus_p_api_release_object(client, module, object, in_data);
    }

done:
    return;
}


#if NEXUS_HAS_SECURITY
#include "priv/nexus_security_priv.h"
#endif
void NEXUS_Platform_P_SweepModules(void)
{
#if NEXUS_HAS_SECURITY
    NEXUS_Module_Lock(g_NEXUS_platformHandles.security);
    NEXUS_SecurityModule_Sweep_priv();
    NEXUS_Module_Unlock(g_NEXUS_platformHandles.security);
#endif
}

#if (NEXUS_POWER_MANAGEMENT && NEXUS_CPU_ARM) && (BCHP_PWR_RESOURCE_M2MC0 || BCHP_PWR_RESOURCE_M2MC1 || BCHP_PWR_RESOURCE_GRAPHICS3D)
static NEXUS_Error NEXUS_Platform_P_SetThermalScaling(BCHP_PWR_ResourceId resourceId, unsigned thermalPoint, unsigned maxThermalPoints)
{
#if ((BCHP_CHIP == 7260) && (BCHP_VER < BCHP_VER_B0)) /* Not available on 7260 A0 */
    BSTD_UNUSED(resourceId);
    BSTD_UNUSED(thermalPoint);
    BSTD_UNUSED(maxThermalPoints);
    BDBG_WRN(("Thermal Scaling is disabled on this platform!!"));
    return NEXUS_SUCCESS;
#else
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned clkRate, clkRateCur, clkRateMin, clkRateMax;

    if(!g_NEXUS_pCoreHandles->chp)
        return NEXUS_NOT_INITIALIZED;

    if (!BCHP_PWR_GetMinClockRate(g_NEXUS_pCoreHandles->chp, resourceId, &clkRateMin) &&
        !BCHP_PWR_GetMaxClockRate(g_NEXUS_pCoreHandles->chp, resourceId, &clkRateMax)) {
        BDBG_MSG(("Resource %u : Min freq %u, Max freq %u", resourceId, clkRateMin, clkRateMax));
        clkRate = clkRateMax - (thermalPoint * ((clkRateMax - clkRateMin)/maxThermalPoints));
        rc = BCHP_PWR_GetClockRate(g_NEXUS_pCoreHandles->chp, resourceId, &clkRateCur);
        if (!rc) {
            BDBG_MSG(("Resource %u : Current freq %u, Set freq %u", resourceId, clkRateCur, clkRate));
            if(clkRate != clkRateCur) {
                rc = BCHP_PWR_SetClockRate(g_NEXUS_pCoreHandles->chp, resourceId, clkRate);
                if(rc) { rc = BERR_TRACE(rc); }
            }
        }
    }

    return rc;
#endif
}
#endif

NEXUS_Error NEXUS_Platform_SetThermalScaling_driver(unsigned thermalPoint, unsigned maxThermalPoints)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
#if NEXUS_POWER_MANAGEMENT && NEXUS_CPU_ARM
    if(!g_NEXUS_pCoreHandles->chp)
        return NEXUS_NOT_INITIALIZED;

    if(maxThermalPoints == 0 || thermalPoint>maxThermalPoints)
        return NEXUS_INVALID_PARAMETER;

#ifdef BCHP_PWR_RESOURCE_M2MC0
    if (NEXUS_Platform_P_SetThermalScaling(BCHP_PWR_RESOURCE_M2MC0, thermalPoint, maxThermalPoints)) {
        BDBG_ERR(("Unable to set M2MC0 frequency"));
        rc = NEXUS_NOT_SUPPORTED;
    }
#endif
#ifdef BCHP_PWR_RESOURCE_M2MC1
    if (NEXUS_Platform_P_SetThermalScaling(BCHP_PWR_RESOURCE_M2MC1, thermalPoint, maxThermalPoints)) {
        BDBG_ERR(("Unable to set M2MC1 frequency"));
        rc = NEXUS_NOT_SUPPORTED;
    }
#endif
#ifdef BCHP_PWR_RESOURCE_GRAPHICS3D
    if (NEXUS_Platform_P_SetThermalScaling(BCHP_PWR_RESOURCE_GRAPHICS3D, thermalPoint, maxThermalPoints)) {
        BDBG_ERR(("Unable to set GRAPHICS3D frequency"));
        rc = NEXUS_NOT_SUPPORTED;
    }
#endif

#else
    BSTD_UNUSED(thermalPoint);
    BSTD_UNUSED(maxThermalPoints);
#endif
    return rc;
}

void NEXUS_Platform_GetHeapRuntimeSettings( NEXUS_HeapHandle heap, NEXUS_HeapRuntimeSettings *pSettings )
{
    NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
    NEXUS_Heap_GetRuntimeSettings_priv(heap, pSettings);
    NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
}

static NEXUS_Error NEXUS_Platform_P_SetPictureBufferSecure( bool secure )
{
    NEXUS_MemoryStatus status;
    NEXUS_HeapRuntimeSettings settings;
    int i;
    NEXUS_Error rc;

    for (i = 0;i < NEXUS_MAX_HEAPS;i++) {
        if (!g_pCoreHandles->heap[i].nexus) {
            continue;
        }

        rc = NEXUS_Heap_GetStatus_driver_priv(g_pCoreHandles->heap[i].nexus, &status);
        if (rc != NEXUS_SUCCESS) {
            return BERR_TRACE(rc);
        }

        if((status.heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS) || (status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT))
        {
            if(!(status.memoryType & NEXUS_MEMORY_TYPE_SECURE_OFF))
            {
                /* Cannot toggle picture buffer if either secure gfx or secure ext is enabled */
                /* Caller will "clean up" */
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }

        if(!(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) || !(status.memoryType & NEXUS_MEMORY_TYPE_SECURE)) {
            continue;
        }

        /* Toggle of URRTx not supported */
        if((i==NEXUS_MEMC0_URRT_HEAP)||(i==NEXUS_MEMC1_URRT_HEAP)||(i==NEXUS_MEMC2_URRT_HEAP)) {
            continue;
        }

        NEXUS_Heap_GetRuntimeSettings_priv(g_pCoreHandles->heap[i].nexus, &settings);
        settings.secure = secure;

        rc = NEXUS_Heap_SetRuntimeSettings_priv(g_pCoreHandles->heap[i].nexus, &settings);
        if (rc != NEXUS_SUCCESS) {
            return BERR_TRACE(rc);
        }
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Platform_P_ValidatePictureBufferSecure( void )
{
    NEXUS_MemoryStatus status;
    NEXUS_HeapRuntimeSettings settings;
    int i;
    NEXUS_Error rc;

    /* Only allow change from unsecure to secure if all secure picture buffers are secure */
    /* Only allow change from secure to unsecure if all secure picture buffers are secure */
    for (i = 0;i < NEXUS_MAX_HEAPS;i++) {
        if (!g_pCoreHandles->heap[i].nexus) {
            continue;
        }

        rc = NEXUS_Heap_GetStatus_driver_priv(g_pCoreHandles->heap[i].nexus, &status);
        if (rc != NEXUS_SUCCESS) {
            return BERR_TRACE(rc);
        }

        if(!(status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) || !(status.memoryType & NEXUS_MEMORY_TYPE_SECURE)) {
            continue;
        }

        NEXUS_Heap_GetRuntimeSettings_priv(g_pCoreHandles->heap[i].nexus, &settings);
        if(!settings.secure)
        {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_SetHeapRuntimeSettings( NEXUS_HeapHandle heap, const NEXUS_HeapRuntimeSettings *pSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HeapRuntimeSettings settings;
    bool callSage = false;
    bool pictureBuff=false;

    NEXUS_Module_Lock(g_NEXUS_platformHandles.core);

    NEXUS_Heap_GetRuntimeSettings_priv(heap, &settings);

    if(pSettings->secure!=settings.secure) {
        NEXUS_MemoryStatus status;

        switch (NEXUS_Heap_GetIndex_isrsafe(heap)) {
        case NEXUS_MEMC0_URRT_HEAP:
        case NEXUS_MEMC1_URRT_HEAP:
        case NEXUS_MEMC2_URRT_HEAP:
            /* don't allow toggle on some heaps */
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        default:
            break;
        }

#if NEXUS_HAS_VIDEO_DECODER
        {
        bool openDecoder;
        NEXUS_Module_Lock(g_NEXUS_platformHandles.videoDecoder);
        openDecoder = NEXUS_VideoDecoderModule_DecoderOpenInSecureHeaps_priv();
        NEXUS_Module_Unlock(g_NEXUS_platformHandles.videoDecoder);
        if (openDecoder) {
            BDBG_ERR(("cannot toggle URR security with open decoder"));
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
            goto EXIT;
        }
        }
#endif

        rc = NEXUS_Heap_GetStatus_driver_priv(heap, &status);
        if (rc != NEXUS_SUCCESS) {
            rc = BERR_TRACE(rc);
            goto EXIT; /* will not execute sage call, rc != NEXUS_SUCCESS */
        }

        /* Some notes....
        * 1. All secure picture buffers are "tied" together. I.e. toggle secure status of any single
        *   secure picture buffer will require all secure picture buffers to toggle, and a single
        *   message to sage to perform the same toggle action
        * 2. Secure GFX/EXT buffers must be marked unsecure BEFORE any picture buffer can be marked
        *   as unsecure.
        * 3. Secure picture buffer(s) must be marked secure BEFORE any secure GFX/EXT buffer
        *   can be marked secure
        */
        if((status.heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS) || (status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT))
        {
            callSage=true;
            rc = NEXUS_Platform_P_ValidatePictureBufferSecure();
            if (rc != NEXUS_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto EXIT; /* will not execute sage call, rc != NEXUS_SUCCESS */
            }
        }

        /* For any secure picture buffers, a "secure" toggle on/off must be passed up to SAGE */
        /* Note that any 1 picture heap secure status should reflect ALL secure picture heaps */
        if((status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) && (status.memoryType & NEXUS_MEMORY_TYPE_SECURE)) {
            callSage=true;
            pictureBuff=true;
            /* Make sure this transition is reflected in all other secure picture buffer heaps */
            rc = NEXUS_Platform_P_SetPictureBufferSecure(pSettings->secure);
            if (rc != NEXUS_SUCCESS) {
                rc = BERR_TRACE(rc);
                /* Attempt to restore any previous state information */
                NEXUS_Platform_P_SetPictureBufferSecure(settings.secure);
                goto EXIT; /* will not execute sage call, rc != NEXUS_SUCCESS */
            }
        }
    }

    rc = NEXUS_Heap_SetRuntimeSettings_priv(heap, pSettings);

EXIT:
    NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);

#if NEXUS_HAS_SAGE
    if((rc==NEXUS_SUCCESS) && (callSage)) {
        rc = NEXUS_Sage_UpdateHeaps();
        if(rc != NEXUS_SUCCESS) {
            rc = BERR_TRACE(rc);
            /* Attempt to keep nexus/sage in sync */
            NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
            if(pictureBuff)
            {
                NEXUS_Platform_P_SetPictureBufferSecure(settings.secure);
            }
            NEXUS_Heap_SetRuntimeSettings_priv(heap, &settings);
            NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
        }
    }
#else
    BSTD_UNUSED(callSage);
    BSTD_UNUSED(pictureBuff);
#endif

    return rc;
}

NEXUS_Error NEXUS_Platform_GetObjectFromId(NEXUS_BaseObjectId id, NEXUS_AnyObject *object)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_BaseObject *baseObject = NULL;

    if(id) {
        rc = NEXUS_BaseObject_FromId(id, &baseObject);
    }
    *object = baseObject;
    return rc;
}

NEXUS_Error NEXUS_Platform_GetIdFromObject( NEXUS_AnyObject object, NEXUS_BaseObjectId *id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    *id = 0;
    if(object) {
        rc = NEXUS_BaseObject_GetId(object, id);
    }
    return rc;
}

void NEXUS_Platform_GetFileModuleSettings_driver( NEXUS_FileModuleSettings *pFileModuleSettings )
{
    *pFileModuleSettings = g_NEXUS_platformSettings.fileModuleSettings;
}

void NEXUS_Platform_SetOverTempResetThreshold(unsigned overTemp, unsigned parkHigh, unsigned parkLow)
{
#if BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD && BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE
    unsigned threshold, value;


    /* Set over temp reset threshold */
    threshold = ((41004000 - overTemp*100)/48705)<<1;
    value = BREG_Read32(g_pCoreHandles->reg, BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD);
    value &= ~BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD_threshold_MASK;
    value |= threshold & BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD_threshold_MASK;
    BREG_Write32(g_pCoreHandles->reg, BCHP_AVS_TMON_TEMPERATURE_RESET_THRESHOLD, value);

    /* Store the value in AON */
#define AON_REG_OVERTEMP 11
    BKNI_EnterCriticalSection();
    value = BREG_Read32(g_pCoreHandles->reg, BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE + (AON_REG_OVERTEMP*4));
    value &= ~0xFFFFFF;
    value |= (parkHigh/1000)<<16 | (parkLow/1000)<<8 | (overTemp/1000);
    BREG_Write32(g_pCoreHandles->reg, BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE + (AON_REG_OVERTEMP*4), value);
    BKNI_LeaveCriticalSection();
#else
    BSTD_UNUSED(overTemp);
    BSTD_UNUSED(parkHigh);
    BSTD_UNUSED(parkLow);
#endif
}
