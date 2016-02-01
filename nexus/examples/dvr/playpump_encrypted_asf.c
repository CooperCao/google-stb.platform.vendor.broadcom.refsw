/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
/* Nexus example app: feeding an encrypted ASF file to playpump */

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playpump.h"
#include "nexus_security.h"
#include "drmcore.h"
#include "drmdecryptor.h"

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/MP10_Stiker_adv-drm.wmv"
#define TRANSPORT_TYPE NEXUS_TransportType_eAsf
#define VIDEO_CODEC NEXUS_VideoCodec_eVc1
#define VIDEO_PID 1

static bdrm_err example_license_callback(
    bdrm_acq_state* pstAcqState,
    bdrm_license_t license,
    const void * context
    );

/*
 * When building Nexus, you will need the following env variables:
 *
 * export MSDRM_PD_SUPPORT=y
 * export MEDIA_ASF_SUPPOR=y
 */

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

struct example_drm_state {
    basf_parser_t           asfParser;
    batom_factory_t         factory;
    batom_pipe_t            asfPipe;
    bool parsedHeader;
};

static void example_drm_feed_data(struct example_drm_state *state, const void *buffer, size_t buffer_size)
{
    batom_t atom;
    struct basf_parser_info info;

    if(!state->parsedHeader) {
        atom = batom_from_range(state->factory, buffer, buffer_size, NULL, NULL);
        batom_pipe_push(state->asfPipe, atom);
        basf_parser_feed(state->asfParser, state->asfPipe);
        basf_parser_get_info(state->asfParser, &info);
        if(info.state == basf_parser_state_data_object) {
            state->parsedHeader=true;
            batom_pipe_flush(state->asfPipe);
            basf_parser_reset(state->asfParser);
        }
    }
    return;
}

int main(void)
{
    FILE *file;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenPidChannelSettings playpumpPidChannelSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    BKNI_EventHandle event;
    const char *fname = FILE_NAME
    bdrm_cfg                stDrmConfig;
    bdrm_t                  stDrmContext;
    bdrm_decryptor_t        stDrmDcrypt;
    NEXUS_DmaHandle          hDma;
    basf_parser_cfg         asfParserCfg;
    struct example_drm_state example_drm_state;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Open up a DMA channel, it will be used by the security code. */
    hDma = NEXUS_Dma_Open(0, NULL);

    /* Save the DMA handle. */
    bdrm_set_dma_handle( hDma );

    BKNI_CreateEvent(&event);

    playpump = NEXUS_Playpump_Open(0, NULL);
    file = fopen(fname, "rb");
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto error;
    }

    example_drm_state.parsedHeader = false;
    example_drm_state.factory = batom_factory_create(bkni_alloc, 16);
    if (!example_drm_state.factory) { goto err_factory; }

    example_drm_state.asfPipe = batom_pipe_create(example_drm_state.factory);
    if (!example_drm_state.factory) { goto err_pipe; }

    basf_parser_default_cfg(&asfParserCfg);

    example_drm_state.asfParser= basf_parser_create(example_drm_state.factory, &asfParserCfg);
    if(!example_drm_state.asfParser) { goto err_asf_parser; }

    example_drm_state.asfPipe = batom_pipe_create(example_drm_state.factory);

    /* get license */
    stDrmConfig.lic_extern = NULL;
    stDrmConfig.lic_extern_ctx = NULL;
    stDrmConfig.ext_asf_scanning = true;

    if ((stDrmContext = bdrm_open( "", &stDrmConfig)) != NULL)
    {
        /* set up drm callbacks and contexts */
        stDrmConfig.lic_callback = example_license_callback;
        stDrmConfig.lic_callback_ctx = (void *)NULL;
        stDrmConfig.asf = example_drm_state.asfParser;

        bdrm_init_handlers( stDrmContext );
        bdrm_dcrypt_instance( stDrmContext, &stDrmDcrypt );
    }

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    playpumpSettings.transportType = TRANSPORT_TYPE;
    playpumpSettings.securityContext = (NEXUS_KeySlotHandle) (*(void **)&stDrmDcrypt );
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidChannelSettings);
    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, &playpumpPidChannelSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* Bring up video decoder and display. No audio to keep the example simple. */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
		}
    }
#endif

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    while (1) {
        void *buffer;
        size_t buffer_size;
        int n;
        NEXUS_Error rc;

        if (NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size))
            break;
        if (buffer_size == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        /* The first call to get_buffer will return the entire playback buffer.
        If we use it, we're going to have to wait until the descriptor is complete,
        and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;

        n = fread(buffer, 1, buffer_size, file);
        if (n < 0) goto error;
        if (n == 0) {
            BKNI_Sleep(1000); /* This is a simplistic way of making sure the MVD buffer is empty before flushing. */
            fseek(file, 0, SEEK_SET);
            NEXUS_Playpump_Flush(playpump);
        }
        else {
            example_drm_feed_data(&example_drm_state, buffer, n);
            rc = NEXUS_Playpump_WriteComplete(playpump, 0, n);
            BDBG_ASSERT(!rc);
        }
    }
    basf_parser_destroy(example_drm_state.asfParser);
    batom_pipe_destroy(example_drm_state.asfPipe);
    batom_factory_destroy(example_drm_state.factory);

    /* loops forever */

error:
err_asf_parser:
err_pipe:
err_factory:
    return 1;
}

/*
 * example callback function for the license state notification
 */
bdrm_err example_license_callback (
    bdrm_acq_state * pstAcqState,
    bdrm_license_t hLicense,
    const void * context
    )
{
    bdrm_license_info   stLicenseInfo;
    bdrm_license_state  eLicenseState;
    bdrm_http_state     eHttpState;
    uint8_t             count       = 0;
    uint8_t             index       = 0;
    bdrm_err            ret         = bdrm_err_ok;

    BSTD_UNUSED(context);

    if ( pstAcqState )
    {
        eLicenseState  = pstAcqState->license_state;
        eHttpState = pstAcqState->http_state;
        fprintf(stderr, "example_license_callback:: setting drm license state:%d http state: %d\n", eLicenseState, eHttpState);
    }
    else
    {
        return (ret) = bdrm_err_fail;
    }

    fprintf(stderr, "example_license_callback:: license state changed TO (%d)\n", eLicenseState);

    if ( bdrm_lic_acquired == eLicenseState
         || bdrm_lic_faillocal == eLicenseState
         || bdrm_lic_failtransfer == eLicenseState
        )
    {
        count = bdrm_license_get_count( hLicense );
        for (index = 0; index < count; index++)
        {
            BKNI_Memset( &stLicenseInfo, 0, sizeof(bdrm_license_info) );
            bdrm_license_get_item( hLicense, index, &stLicenseInfo);
            fprintf(stderr, "example_license_callback:: stream id=%d ",stLicenseInfo.stream_id );

            switch (stLicenseInfo.type)
            {
            case bdrm_lictype_from: fprintf(stderr,  "from" ); break;
            case bdrm_lictype_until: fprintf(stderr,  "until" ); break;
            case bdrm_lictype_fromuntil: fprintf(stderr,  "from-until" ); break;
            case bdrm_lictype_count: fprintf(stderr,  "count" ); break;
            case bdrm_lictype_unlimited: fprintf(stderr,  "unlimited" ); break;
            case bdrm_lictype_noright: fprintf(stderr,  "noright" ); break;
            case bdrm_lictype_unknown: fprintf(stderr,  "unknown" ); break;
            }

            fprintf(stderr, " type=(%d) play_count=(%d) plays=%d times_count=(%d)\n",
                      stLicenseInfo.type,
                      stLicenseInfo.plays_count,
                      stLicenseInfo.plays[0],
                      stLicenseInfo.times_count
                      /*asctime(&stLicenseInfo.times[0])*/
                      );
        }
    }
    else ret = bdrm_err_fail;

    return(ret);
}

