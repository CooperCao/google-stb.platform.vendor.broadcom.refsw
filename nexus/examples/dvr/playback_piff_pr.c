/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
 *
 *****************************************************************************/
/*
 * playback_piff_pr.c:
 *
 * playback and decoding of PIFF files encrypted with the PlayReady DRM scheme.
 * This example utilizes the nexus_playback which allows the trick playback features
 * to be added.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file_pvr.h"
#include "nexus_file.h"
#endif

#include "bioatom.h"
#include "bfile_stdio.h"
#include "bfile_buffer.h"
#include "bpiff.h"
#include "bmp4_util.h"
#include "bmp4_player.h"

#include "nexus_wmdrmpd.h"
#include "nexus_wmdrmpd_types.h"
#include "nexus_security.h"

#include "prdy_http.h"
#include "blst_list.h"

BDBG_MODULE(playback_piff_pr);


/**************************************************************************
 * Defines
 **************************************************************************/

#define BUFFER_LENGTH       (16*4096)
#define NEXUS_TRANSPORT_TYPE    NEXUS_TransportType_eMp4Fragment


/*
 * Most of the typedefs and parse routines are borrowed from prdy_mp4.c
 */

typedef enum piff_err {
    piff_err_ok = 0,
    piff_err_fail,
    piff_err_buffer_size                   /* buffer too small */
} piff_err;

typedef struct piff_drm {
    bpiff_mp4_headers header;
} piff_drm;
typedef struct piff_drm *piff_drm_t;

typedef struct piff_file {
    NEXUS_FilePlayHandle fileplay;
    batom_factory_t factory;
    void *buffer_data;
    bfile_buffer_t file_buffer;
} piff_file;

typedef struct piff_context {
    piff_file file;
    piff_drm drm;
    NEXUS_WmDrmPdHandle wmDrmPd;
} piff_context;
typedef struct piff_context *piff_t;


/**************************************************************************
 * Globals
 **************************************************************************/

static piff_context piff;  /* top level context */


/**************************************************************************
 * Functions
 **************************************************************************/

int file_buffer_create(piff_file *file)
{
    bfile_buffer_cfg buffer_cfg;

    file->factory = batom_factory_create(bkni_alloc, 16);
    if (file->factory == NULL) {
        printf("### batom_factory_create failed\n");
        return -1;
    }

    file->buffer_data = malloc(BUFFER_LENGTH);   /* TODO: don't forget to free! */
    if (file->buffer_data == NULL) {
        printf("### Unable to malloc %d bytes\n", BUFFER_LENGTH);
        batom_factory_destroy(file->factory);
        file->factory = NULL;
        return -1;
    }
    bfile_buffer_default_cfg(&buffer_cfg);
    buffer_cfg.buf_len = BUFFER_LENGTH;
    buffer_cfg.buf = file->buffer_data;
    buffer_cfg.fd = file->fileplay->file.index;
    buffer_cfg.nsegs = BUFFER_LENGTH/4096;
    file->file_buffer = bfile_buffer_create(file->factory, &buffer_cfg);

    if (!file->file_buffer) {
        printf("bfile_buffer_create failed\n");
        batom_factory_destroy(file->factory);
        file->factory = NULL;
        free(file->buffer_data);
        file->buffer_data = NULL;
        return -1;
    }

    return 0;
}

int file_buffer_destroy(piff_file *file)
{
    if (file->file_buffer) { bfile_buffer_destroy(file->file_buffer); }
    if (file->buffer_data) { free(file->buffer_data); }
    if (file->factory) { batom_factory_destroy(file->factory); }

    return 0;
}

static void policy_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("policy_callback - event %p\n", context);
}

static int get_drm_info(piff_t piff_cxt)
{
    batom_t atom = NULL;
    bfile_buffer_result result;
    size_t size;
    size_t box_hdr_size;
    batom_cursor cursor;
    int rc = -1;
    bmp4_box box;
    bpiff_mp4_headers piff_header;
    off_t start, end;
    unsigned int file_size;

    printf("%s: start\n", __FUNCTION__);

    /*
     * 0) get file size and set initial read offset to zero
     * 1) Read BUFFER_LENGTH
     *    - error return if atom is zero
     * 2) Parse only the first box
     *    - error return if box size is zero
     *    - if moov is found, get out of the loop.
     * 3) release atom
     * 4) Skip the file offset by box size
     *    - error return if the file_size reached
     * 5) continue from 1)
     */

    if (bfile_buffer_get_bounds(piff_cxt->file.file_buffer, &start, &end)) {
        printf("%s: unable to get the bounds\n", __FUNCTION__);
    }
    else {
        printf("%s: start %#lx, end %#lx\n", __FUNCTION__, (unsigned long)start, (unsigned long)end);
        file_size = (unsigned int)end;
    }

    start = 0;
    while (file_size) {
        if (file_size > BUFFER_LENGTH) {
            size = BUFFER_LENGTH;
        }
        else {
            size = file_size;
        }
        printf("### start %#lx, size %#lx\n", (unsigned long)start, (unsigned long)size);
        atom = bfile_buffer_read(piff_cxt->file.file_buffer, start, size, &result);
        if (atom == NULL) {
            printf("%s: Unable to read file, result %d\n", __FUNCTION__, result);
            goto get_drm_info_exit;
        }
        batom_cursor_from_atom(&cursor, atom);
        /* find moov */
        box_hdr_size = bmp4_parse_box(&cursor, &box);
        printf("### hdr size %x\n", box_hdr_size);
        if (box_hdr_size == 0) {
            printf("### Moov not found\n");
            goto get_drm_info_exit;
        }
        if (box.type == BMP4_MOVIE) {
            printf("Found moov\n");
            break;
        }
        file_size -= size;
        start += box.size;
        batom_release(atom);
        atom = NULL;
    }

    /* Here, atom should be pointing to moov. Pass it to moov parser. */
    memset(&piff_header, 0, sizeof(piff_header));
    rc = bpiff_parse_moov(&piff_header, atom);
    if (rc) {
        printf("bpiff_parse_moov() failed (%d)\n", rc);
        goto get_drm_info_exit;
    }
    else {
        uint8_t *data = piff_header.psshSystemId.systemId.data;
        uint32_t i;
        printf("------------------------\n");
        printf(" P S S H\n");
        printf("------------------------\n");
        printf("* PSSH SystemID: ");
            for (i=0; i<16; i++) {
                printf("0x%x ", data[i]);
            }
            printf("\n");
        printf("* PSSH data length: %d\n", piff_header.psshDataLength);
        printf("* PSSH data (first 16 bytes): ");
            for (i=0; i<16; i++) {
                printf("0x%x ", piff_header.pPsshData[i]);
            }
            printf("\n");
        printf("* Num of protection schemes: %d\n", piff_header.nbOfSchemes);
            for (i=0; i<piff_header.nbOfSchemes; i++) {
                int j;
                printf("* Scheme %d\n", i);
                printf("  Box ver      0x%x\n", piff_header.scheme[i].schemeType.version);
                printf("  Box flags    0x%x\n", piff_header.scheme[i].schemeType.flags);
                printf("  Sch type     " B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(piff_header.scheme[i].schemeType.schemeType));
                printf("  Sch ver      0x%x\n", piff_header.scheme[i].schemeType.schemeVersion);
                printf("  Org fmt      " B_MP4_TYPE_FORMAT"\n", B_MP4_TYPE_ARG(piff_header.scheme[i].originalFormat.codingName));
                printf("  trackID      0x%x\n", piff_header.scheme[i].trackId);
                printf("  TE box ver   0x%x\n", piff_header.scheme[i].trackEncryption.version);
                printf("  TE box flags 0x%x\n", piff_header.scheme[i].trackEncryption.flags);
                printf("  TE algo      0x%x\n", piff_header.scheme[i].trackEncryption.info.algorithm);
                printf("  TE IV size   0x%x\n", piff_header.scheme[i].trackEncryption.info.ivSize);
                printf("  TE keyID     ");
                    for (j=0; j<16; j++) {
                        printf("0x%x ", piff_header.scheme[i].trackEncryption.info.keyId[j]);
                    }
                    printf("\n");
            }
        printf("------------------------\n");
    }
    memcpy(&piff_cxt->drm.header, &piff_header, sizeof(bpiff_mp4_headers));

    /* cleanup and exit */
    rc = 0;

get_drm_info_exit:
    if (atom) { batom_release(atom); }
    printf("%s: end\n", __FUNCTION__);

    return rc;
}

int main(int argc, char **argv)
{
#if NEXUS_HAS_PLAYBACK
    NEXUS_Error rc_nexus = NEXUS_SUCCESS;

    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_WmDrmPdHandle wmdrm;
    NEXUS_WmDrmPdSettings drmSettings;
    NEXUS_DmaHandle dma;
    NEXUS_VideoCodec vid_codec;
    uint16_t vid_pid_num;
    NEXUS_AudioCodec aud_codec;
    uint16_t aud_pid_num;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_SecurityKeySlotSettings genericSettings;
    NEXUS_KeySlotHandle genericKeyHandle;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif

    const char *fname = "./videos/SuperSpeedway_720_230.ismv";  /* default if not provided by user */
    int rc = 0;

    if (argc != 2) {
        printf("No filename provided. Will try %s by default.\n", fname);
    }
    else {
        fname = argv[1];
        printf("filename = %s\n", fname);
    }

    /*
     * NEXUS platform
     */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /*
     * Init the main context
     */
    memset(&piff, 0, sizeof(piff_context));

    /* Following parameters require prior knowledge from the stream */
    vid_pid_num = 0x2;
    vid_codec = NEXUS_VideoCodec_eH264;
    aud_pid_num = 0x1;
    aud_codec = NEXUS_AudioCodec_eAac;

    piff.file.fileplay = NEXUS_FilePlay_OpenPosix(fname, fname);
    if (piff.file.fileplay == NULL) {
        printf("Can't open fileplay for %s\n", fname);
        rc = -1;
        goto error_exit;
    }
    if (file_buffer_create(&piff.file) < 0) {
        printf("Unable to create file buffer\n");
        rc = -1;
        goto error_exit;
    }

    /*
     * STC
     */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /*
     * WMDRMPD
     */
    NEXUS_WmDrmPd_GetDefaultSettings(&drmSettings);

    dma = NEXUS_Dma_Open(0, NULL);
    assert(dma);
    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);

    drmSettings.dma = dma;
    drmSettings.transportType = NEXUS_TRANSPORT_TYPE;
    drmSettings.policyCallback.callback = policy_callback;
    drmSettings.policyCallback.context = NULL;
    drmSettings.policyCallback.param = 0;

    wmdrm = NEXUS_WmDrmPd_Create(&drmSettings);
    if (wmdrm == NULL) {
        printf("NEXUS_WmDrmPd_Create() failed\n");;
        rc = -1;
        goto error_exit;
    }
    piff.wmDrmPd = wmdrm;

    /*
     * Security Context
     */
    NEXUS_Security_GetDefaultKeySlotSettings(&genericSettings);
    genericSettings.keySlotEngine = NEXUS_SecurityEngine_eGeneric;
    genericKeyHandle = NEXUS_Security_AllocateKeySlot(&genericSettings);
    if (genericKeyHandle == NULL) {
        printf("NEXUS_Security_AllocateKeySlot failed\n");
        rc = -1;
        goto error_exit;
    }
    else {
        printf("Allocated key slot: %#lx\n", (unsigned long)genericKeyHandle);
    }

    NEXUS_KeySlot_SetTag(genericKeyHandle, (NEXUS_KeySlotTag)wmdrm);

    /*
     * Playback
     */
    playback = NEXUS_Playback_Create();
    assert(playback);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eMp4;  /* TODO: "eMp4Fragment" not supported by bmedia_player */
    playbackSettings.stcChannel = stcChannel;
    playbackSettings.playpumpSettings.securityContext = genericKeyHandle;

    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /*
     * Display
     */
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
    rc_nexus = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if (!rc && hdmiStatus.connected) {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if (!hdmiStatus.videoFormatSupported[displaySettings.format]) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    /*
     * Video decoder
     */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidSettings.allowTimestampReordering = false;
    playbackPidSettings.pidTypeSettings.video.codec = vid_codec; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, vid_pid_num, &playbackPidSettings);

    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = vid_codec;
    videoProgram.timestampMode = NEXUS_VideoDecoderTimestampMode_eDecode;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;


    /*
     * Audio decoder
     */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);

    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    /* valid for AAC? */
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* audio pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = pcmDecoder;
    playbackPidSettings.pidTypeSettings.audio.secondary = compressedDecoder;
    playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = aud_codec;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, aud_pid_num, &playbackPidSettings);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = aud_codec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /*
     * Read DRM info from the file
     */
    if (get_drm_info(&piff)) {
        printf("get_drm_info failed. Assuming a clean stream\n");
        goto start_playback;
    }

    if (piff.drm.header.pPsshData == NULL) {
        printf("Looks like a clean stream\n");
        goto start_playback;  /* assume a clean stream. start playback. */
    }

    /*
     * Load license
     */
    rc_nexus = NEXUS_WmDrmPd_SetPsshBox(wmdrm,
                                        (const NEXUS_WmDrmPdMp4PsshBoxInfo *)&piff.drm.header.psshSystemId.systemId,
                                        piff.drm.header.pPsshData,
                                        piff.drm.header.psshDataLength);
    if (rc_nexus != NEXUS_SUCCESS) {
        printf("NEXUS_WmDrmPd_SetPsshBox() failed\n");
        goto error_exit;
    }

    if (NEXUS_WmDrmPd_LoadLicense(wmdrm) != NEXUS_SUCCESS) {
        NEXUS_WmDrmPdLicenseChallenge challenge;
        int32_t ret;
        uint32_t start_offset, length;

        NEXUS_WmDrmPd_GetLicenseChallenge(wmdrm, &challenge);
        ret = bhttpclient_license_post_soap((char *)challenge.pUrl,
                                            (char *)challenge.pData,
                                            1,
                                            0,
                                            (unsigned char **)&(challenge.pResponseBuffer),
                                            &start_offset,
                                            &length);

        if (ret == 0) {
            challenge.pResponseBuffer[length] = '\0';
#if 1
            printf("Response: <<%s>>\n", challenge.pResponseBuffer);
#endif
            NEXUS_WmDrmPd_LicenseChallengeComplete(wmdrm, length, start_offset);
        }
        else {
            printf("Posting challenge failed\n");
            rc = -1;
            goto error_exit;
        }

        /* try again */
        NEXUS_WmDrmPd_LoadLicense(wmdrm);
    }

start_playback:
    /*
     * Start playback
     */
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_Playback_Start(playback, piff.file.fileplay, NULL);

#if 0
    for (i=0; i<20; i++) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
        fprintf(stderr, "numDecoded = %u, numDecodeErrors = %u, ptsErrorCount = %u\n", status.numDecoded, status.numDecodeErrors, status.ptsErrorCount);
        if (status.numDecodeErrors) {
            fprintf(stderr, "Failed with numDecodeErrors = %u\n", status.numDecodeErrors);
            rc = -1;
            break;
        }
        sleep(1);
    }
#else
    printf("Press ENTER to stop...\n");
    getchar();
#endif

error_exit:
    bpiff_FreeMp4Header(&piff.drm.header);
    file_buffer_destroy(&piff.file);

    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(pcmDecoder);
    NEXUS_AudioDecoder_Stop(compressedDecoder);

    NEXUS_Playback_Stop(playback);
    NEXUS_Playback_ClosePidChannel(playback, videoPidChannel);
    NEXUS_Playback_ClosePidChannel(playback, audioPidChannel);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_FilePlay_Close(piff.file.fileplay);

    NEXUS_WmDrmPd_Destroy(wmdrm);
    NEXUS_Security_FreeKeySlot(genericKeyHandle);
    NEXUS_Dma_Close(dma);

    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);

    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(compressedDecoder);

    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Platform_Uninit();

    return rc;
#else
    printf("### NEXUS_HAS_PLAYBACK == 0. This example requires a \"PLAYBACK\".\n");
    return -1;
#endif
}
