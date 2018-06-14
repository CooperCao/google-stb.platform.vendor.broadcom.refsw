/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "bstd.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "bsagelib_tools.h"
#include "bsagelib_crypto_types.h"

#include "sage_srai.h"

#include "utility_ids.h"
#include "utility_platform.h"
#include "securersa_tl.h"
#include "secure_rsa_test_vectors_host.h"

#include "nexus_platform.h"
#include "nexus_platform_init.h"
#include "nexus_security.h"
#include "nexus_security_client.h"
#include "nexus_dma.h"
#include "nexus_memory.h"

/* For playback test */
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif



BDBG_MODULE(sage_utility_securersa_tool);



static int SAGE_app_join_nexus(void);
static void SAGE_app_leave_nexus(void);
static void print_help(void);



#define MAX_ARG_STRING_LENGTH         (256)
#define SIZEOF_DRM_BINFILE_HEADER     (192)
#define SIZEOF_DYNAMIC_OFFSET_HEADER  (16)

#define RSA_BUFFER_LEN           RSA_KEY_LEN_3072
#define M2M_OUTPUT_BUFER_LEN                  512
#define IMPORT_EXPORT_BUFFER_LEN              256
#define HMAC_SHA1_EXPECTED_LEN                 20
#define HMAC_SHA256_EXPECTED_LEN               32

#define EXPECTED_KEYS_TEST1_RSA   0
#define EXPECTED_KEYS_TEST1_KEY3  0
#define EXPECTED_KEYS_TEST1_KPK   0
#define EXPECTED_KEYS_TEST2_RSA   6
#define EXPECTED_KEYS_TEST2_KEY3  0
#define EXPECTED_KEYS_TEST2_KPK   0
#define EXPECTED_KEYS_TEST3_RSA   8
#define EXPECTED_KEYS_TEST3_KEY3  8
#define EXPECTED_KEYS_TEST3_KPK   2
#define EXPECTED_KEYS_TEST4_RSA   6
#define EXPECTED_KEYS_TEST4_KEY3  4
#define EXPECTED_KEYS_TEST4_KPK   1
#define EXPECTED_KEYS_TEST5_RSA   0
#define EXPECTED_KEYS_TEST5_KEY3  0
#define EXPECTED_KEYS_TEST5_KPK   0

#if (NEXUS_SECURITY_API_VERSION == 1)
#define KEY_SLOT_NUMBER_CP   (KeyslotInfoCp.keySlotNumber)
#define KEY_SLOT_NUMBER_CA1  (KeyslotInfoCa1.keySlotNumber)
#define KEY_SLOT_NUMBER_CA2  (KeyslotInfoCa2.keySlotNumber)
#else
#define KEY_SLOT_NUMBER_CP   (KeyslotInfoCp.slotNumber)
#define KEY_SLOT_NUMBER_CA1  (KeyslotInfoCa1.slotNumber)
#define KEY_SLOT_NUMBER_CA2  (KeyslotInfoCa2.slotNumber)
#endif

#define FILE_NAME "spiderman_aes.ts"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eAc3
#define VIDEO_PID 0x11
#define AUDIO_PID 0x14

#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) { int x_offset;  \
    printf("[%s]", description_txt);                                       \
    for(x_offset = 0; x_offset < (int)(in_size); x_offset++)               \
    {                                                                      \
        if(x_offset%16 == 0) printf("\n");                                 \
                                                                           \
        printf("%02X ", in_ptr[x_offset]);                                 \
    }                                                                      \
    printf("\n");                                                          \
}



static char DrmBinfilePath[MAX_ARG_STRING_LENGTH];

static uint8_t RsaBuffer[RSA_BUFFER_LEN], TestData[RSA_BUFFER_LEN];
static uint32_t RsaBufferLen;

static uint8_t M2mOutput[M2M_OUTPUT_BUFER_LEN], M2mOutput2[M2M_OUTPUT_BUFER_LEN];

static uint8_t ImportExportBuffer[IMPORT_EXPORT_BUFFER_LEN];
static uint32_t ImportExportBufferLen;

static uint8_t HmacBuffer[HMAC_SHA256_EXPECTED_LEN];
static uint32_t HmacBufferLen;

static uint8_t DigestBuffer[DIGEST_LEN_SHA256];
static uint32_t DigestBufferLen;

static bool PlaybackDone = false;

static NEXUS_KeySlotHandle KeySlotHandleCp = NULL;
static NEXUS_KeySlotHandle KeySlotHandleCa1 = NULL, KeySlotHandleCa2 = NULL;
#if (NEXUS_SECURITY_API_VERSION == 1)
static NEXUS_SecurityKeySlotInfo KeyslotInfoCp, KeyslotInfoCa1, KeyslotInfoCa2;
#else
static NEXUS_KeySlotInformation KeyslotInfoCp, KeyslotInfoCa1, KeyslotInfoCa2;
#endif



static BERR_Code AllocateKeyslot(
    NEXUS_KeySlotHandle *keySlotHandle,
#if (NEXUS_SECURITY_API_VERSION == 1)
    NEXUS_SecurityKeySlotInfo *keyslotInfo,
#else
    NEXUS_KeySlotInformation *keyslotInfo,
#endif
    NEXUS_SecurityEngine engine
)
{
    BERR_Code rc = BERR_UNKNOWN;
#if (NEXUS_SECURITY_API_VERSION == 1)
    NEXUS_SecurityKeySlotSettings keySlotSettings;
#else
    NEXUS_KeySlotAllocateSettings keyslotSettings;
#endif


    if((keySlotHandle == NULL) || (keyslotInfo == NULL))
    {
        BDBG_ERR(("%s - NULL parameter", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

#if (NEXUS_SECURITY_API_VERSION == 1)
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = engine;
    keySlotSettings.client = NEXUS_SecurityClientType_eSage;

    *keySlotHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
#else
    NEXUS_KeySlot_GetDefaultAllocateSettings(&keyslotSettings);
    keyslotSettings.owner = NEXUS_SecurityCpuContext_eSage;
    keyslotSettings.useWithDma = true;

    if(engine == NEXUS_SecurityEngine_eCa)
    {
        keyslotSettings.slotType = NEXUS_KeySlotType_eIvPerSlot;
    }
    else
    {
        keyslotSettings.slotType = NEXUS_KeySlotType_eIvPerBlock;
    }

    *keySlotHandle = NEXUS_KeySlot_Allocate(&keyslotSettings);
#endif

    if(*keySlotHandle == NULL)
    {
        BDBG_ERR(("%s - Allocate keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

#if (NEXUS_SECURITY_API_VERSION == 1)
    NEXUS_Security_GetKeySlotInfo(*keySlotHandle, keyslotInfo);
#else
    NEXUS_KeySlot_GetInformation(*keySlotHandle, keyslotInfo);
#endif

    rc = BERR_SUCCESS;

ErrorExit:
    return rc;
}


static void FreeKeyslot(
    NEXUS_KeySlotHandle keySlotHandle
)
{
#if (NEXUS_SECURITY_API_VERSION == 1)
    NEXUS_Security_FreeKeySlot(keySlotHandle);
#else
    NEXUS_KeySlot_Free(keySlotHandle);
#endif
}


static void EndOfStream(
    void *context,
    int param
)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    PlaybackDone = true;
}

static int PlaybackCaKeys(void)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    const char *fname = FILE_NAME;


    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if(!file)
    {
        BDBG_ERR(("%s - Can't open file:%s", __FUNCTION__, fname));
        return 1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    PlaybackDone = false;

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    playbackSettings.endOfStreamCallback.callback = EndOfStream;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if (platformConfig.outputs.audioDacs[0] && NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]))
    {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Bring up video display and outputs */
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
    if(!rc && hdmiStatus.connected)
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format.
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if(!hdmiStatus.videoFormatSupported[displaySettings.format])
        {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Add video PID channel to keyslot */
    if(NEXUS_SUCCESS != NEXUS_KeySlot_AddPidChannel(KeySlotHandleCa1, videoProgram.pidChannel))
    {
        BDBG_ERR(("%s - Config PID channel failed", __FUNCTION__));
        return 1;
    }

    /* Add audio PID channel to keyslot */
    if(NEXUS_SUCCESS != NEXUS_KeySlot_AddPidChannel(KeySlotHandleCa2, audioProgram.pidChannel))
    {
        BDBG_ERR(("%s - Config PID channel failed", __FUNCTION__));
        return 1;
    }

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to quit");
    getchar();

    /*-----------------------------------------------------------------------------
     *              close crypto
     *-----------------------------------------------------------------------------*/

    NEXUS_KeySlot_RemovePidChannel(KeySlotHandleCa1, videoProgram.pidChannel);
    NEXUS_KeySlot_RemovePidChannel(KeySlotHandleCa2, audioProgram.pidChannel);


    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    if (platformConfig.outputs.audioDacs[0] && NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]))
    {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    }
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);

    return 0;
}


static void CompleteCallback(
    void *pParam,
    int iParam
)
{
    BSTD_UNUSED(iParam);
    BDBG_LOG(("%s - CompleteCallback:%#lx fired", __FUNCTION__,
              (unsigned long) pParam));
    BKNI_SetEvent(pParam);
}


static int RunM2mTest(
    uint8_t *input,
    uint32_t dataLen,
    uint8_t *output,
    uint8_t *expectedOutput
)
{
    int rc = 1;
    NEXUS_DmaHandle dma;
    NEXUS_DmaJobSettings jobSettings;
    NEXUS_DmaJobHandle dmaJob;
    NEXUS_DmaJobBlockSettings blockSettings[1];
    NEXUS_DmaJobStatus jobStatus;
    BKNI_EventHandle dmaEvent = NULL;
    uint8_t *dmaInput = NULL, *dmaOutput = NULL;
    uint32_t numBlockSettings = 0;


    if((input == NULL) || (output == NULL) || (dataLen == 0))
    {
        BDBG_ERR(("%s - Invalid Key3 route settings", __FUNCTION__));
        rc = 1;
        goto ErrorExit;
    }

    NEXUS_Memory_Allocate(dataLen, NULL, (void *) &dmaInput);
    NEXUS_Memory_Allocate(dataLen, NULL, (void *) &dmaOutput);
    if((dmaInput == NULL) || (dmaOutput == NULL))
    {
        BDBG_ERR(("%s - Couldn't allocate memory", __FUNCTION__));
        rc = 1;
        goto ErrorExit;
    }

    BKNI_Memcpy(dmaInput, input, dataLen);
    BKNI_Memset(dmaOutput, 0, dataLen);

    /* Open DMA handle */
    dma = NEXUS_Dma_Open(0, NULL);

    BKNI_CreateEvent(&dmaEvent);  /* create an event. */

    NEXUS_DmaJob_GetDefaultSettings(&jobSettings);
    NEXUS_DmaJob_GetDefaultBlockSettings(&blockSettings[0]);

    blockSettings[numBlockSettings].pSrcAddr                  = dmaInput;
    blockSettings[numBlockSettings].pDestAddr                 = dmaOutput;
    blockSettings[numBlockSettings].blockSize                 = dataLen;
    blockSettings[numBlockSettings].resetCrypto               = true;
    blockSettings[numBlockSettings].scatterGatherCryptoStart  = true;
    numBlockSettings++;

    jobSettings.numBlocks                   = numBlockSettings;
    jobSettings.keySlot                     = KeySlotHandleCp;
    jobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    jobSettings.completionCallback.callback = CompleteCallback;
    jobSettings.completionCallback.context  = dmaEvent;

    dmaJob = NEXUS_DmaJob_Create(dma, &jobSettings);
    if(dmaJob == NULL)
    {
        BDBG_ERR(("%s - Couldn't create dma job", __FUNCTION__));
        rc = 1;
        goto ErrorExit;
    }

    if(NEXUS_DmaJob_ProcessBlocks(dmaJob, blockSettings,
                                  numBlockSettings) == NEXUS_DMA_QUEUED)
    {
        BKNI_WaitForEvent(dmaEvent, BKNI_INFINITE);
        NEXUS_DmaJob_GetStatus(dmaJob, &jobStatus);
        BDBG_ASSERT(jobStatus.currentState == NEXUS_DmaJobState_eComplete);
    }

    NEXUS_DmaJob_Destroy(dmaJob);

    /* Make sure output matches expected output */
    BKNI_Memcpy(output, dmaOutput, dataLen);
    if(expectedOutput != NULL)
    {
        if(BKNI_Memcmp(output, expectedOutput, dataLen) != 0)
        {
            BDBG_ERR(("%s - Output mismatch", __FUNCTION__));
            DEBUG_PRINT_ARRAY("Output", dataLen, output);
            DEBUG_PRINT_ARRAY("Expected output", dataLen, expectedOutput);
            rc = 1;
            goto ErrorExit;
        }
    }

    BKNI_DestroyEvent(dmaEvent);

    NEXUS_Dma_Close(dma);

    rc = 0;

ErrorExit:
    if(dmaInput != NULL)
    {
        NEXUS_Memory_Free(dmaInput);
    }

    if(dmaOutput != NULL)
    {
        NEXUS_Memory_Free(dmaOutput);
    }

    return rc;
}


static BERR_Code ShowSecureRsaContext(
    SecureRsaTl_Handle hSecureRsaTl,
    uint32_t *rsaNumKeys,
    uint32_t *key3NumKeys,
    uint32_t *kpkNumKeys
)
{
    BERR_Code rc = BERR_UNKNOWN;
    SecureRsaTl_GetStatusSettings getStatusSettings;
    uint32_t *keyInfo = NULL;
    uint32_t keyInfoLen;
    uint32_t x;


    if((rsaNumKeys == NULL) || (key3NumKeys == NULL) || (kpkNumKeys == NULL))
    {
        BDBG_ERR(("%s - NULL parameter", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    SecureRsaTl_GetDefaultGetStatusSettings(&getStatusSettings);
    keyInfoLen = 0;
    getStatusSettings.rsaNumKeys = rsaNumKeys;
    getStatusSettings.key3NumKeys = key3NumKeys;
    getStatusSettings.kpkNumKeys = kpkNumKeys;
    getStatusSettings.keyInfo = keyInfo;
    getStatusSettings.keyInfoLen = &keyInfoLen;

    rc = SecureRsaTl_GetStatus(hSecureRsaTl, &getStatusSettings);
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Get Status Error", __FUNCTION__));
        goto ErrorExit;
    }

    BDBG_LOG(("%s - num keys %d %d %d, key info len %d", __FUNCTION__,
              *rsaNumKeys, *key3NumKeys, *kpkNumKeys, keyInfoLen));

    if(keyInfoLen != 0)
    {
        keyInfo = BKNI_Malloc(keyInfoLen);
        if(keyInfo == NULL)
        {
            BDBG_ERR(("%s - Couldn't allocate memory", __FUNCTION__));
            rc = BERR_UNKNOWN;
            goto ErrorExit;
        }

        getStatusSettings.keyInfo = keyInfo;

        SecureRsaTl_GetStatus(hSecureRsaTl, &getStatusSettings);

        BDBG_LOG(("%s - KeyInfo", __FUNCTION__));
        for(x=0; x<keyInfoLen/sizeof(uint32_t); x+=6)
        {
            BDBG_LOG(("%s -    0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x,",
                      __FUNCTION__, keyInfo[x], keyInfo[x + 1], keyInfo[x + 2],
                      keyInfo[x + 3], keyInfo[x + 4], keyInfo[x + 5]));
        }

        BKNI_Free(keyInfo);
    }

    rc = BERR_SUCCESS;

ErrorExit:
    return rc;
}


static BERR_Code TestSecureRsa(void)
{
    BERR_Code rc = BERR_SUCCESS;
    SecureRsaTl_Handle hSecureRsaTl = NULL;
    SecureRsaTl_LoadRsaPackageSettings loadRsaPackageSettings;
    SecureRsaTl_RsaSignVerifySettings rsaSignVerifySettings;
    SecureRsaTl_RsaHostUsageSettings rsaHostUsageSettings;
    SecureRsaTl_RsaDecryptAesSettings decryptAesSettings;
    SecureRsaTl_Key3ImportExportSettings key3ImportExportSettings;
    SecureRsaTl_Key3CalculateHmacSettings key3CalculateHmacSettings;
    SecureRsaTl_Key3AppendShaSettings key3AppendShaSettings;
    SecureRsaTl_Key3RouteSettings key3RouteSettings, key3RouteSettings2;
    SecureRsaTl_Key3UnrouteSettings key3UnrouteSettings, key3UnrouteSettings2;
    SecureRsaTl_Key3LoadClearIkrSettings key3LoadClearIkrSettings;
    SecureRsaTl_Key3IkrDecryptIkrSettings key3IkrDecryptIkrSettings;
    SecureRsaTl_RsaLoadPublicKeySettings rsaLoadPublicKeySettings;
    SecureRsaTl_KpkDecryptIkrSettings kpkDecryptIkrSettings;
    SecureRsaTl_KpkDecryptRsaSettings kpkDecryptRsaSettings;
    SecureRsaTl_RemoveKeySettings removeKeySettings;
    uint32_t rsaNumKeys, key3NumKeys, kpkNumKeys;


    BDBG_LOG(("%s - Secure RSA Tests", __FUNCTION__));

    BKNI_Memset(TestData, INPUT_DATA_VALUE, RSA_BUFFER_LEN);

    if((AllocateKeyslot(&KeySlotHandleCp, &KeyslotInfoCp,
                        NEXUS_SecurityEngine_eM2m) != BERR_SUCCESS) ||
       (AllocateKeyslot(&KeySlotHandleCa1, &KeyslotInfoCa1,
                        NEXUS_SecurityEngine_eCa) != BERR_SUCCESS) ||
       (AllocateKeyslot(&KeySlotHandleCa2, &KeyslotInfoCa2,
                        NEXUS_SecurityEngine_eCa) != BERR_SUCCESS))
    {
        BDBG_ERR(("%s - Key slot not allocated", __FUNCTION__));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }


    /* Initialize the Secure Rsa TL module */
    rc = SecureRsaTl_Init(&hSecureRsaTl);
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Secure RSA TL module couldn't init", __FUNCTION__));
        goto ErrorExit;
    }


    /* Check the number of keys */
    BDBG_LOG(("%s - Check the number of keys", __FUNCTION__));
    if((ShowSecureRsaContext(hSecureRsaTl, &rsaNumKeys, &key3NumKeys,
                             &kpkNumKeys) == BERR_SUCCESS) &&
       (rsaNumKeys == EXPECTED_KEYS_TEST1_RSA) &&
       (key3NumKeys == EXPECTED_KEYS_TEST1_KEY3) &&
       (kpkNumKeys == EXPECTED_KEYS_TEST1_KPK))
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Incorrect number of keys", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Load the RSA package */
    SecureRsaTl_GetDefaultLoadRsaPackageSettings(&loadRsaPackageSettings);
    strcpy(loadRsaPackageSettings.drm_binfile_path, DrmBinfilePath);

    rc = SecureRsaTl_LoadRsaPackage(hSecureRsaTl, &loadRsaPackageSettings);
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Couldn't load RSA package", __FUNCTION__));
        goto ErrorExit;
    }


    /* Check the number of keys */
    BDBG_LOG(("%s - Check the number of keys", __FUNCTION__));
    if((ShowSecureRsaContext(hSecureRsaTl, &rsaNumKeys, &key3NumKeys,
                             &kpkNumKeys) == BERR_SUCCESS) &&
       (rsaNumKeys == EXPECTED_KEYS_TEST2_RSA) &&
       (key3NumKeys == EXPECTED_KEYS_TEST2_KEY3) &&
       (kpkNumKeys == EXPECTED_KEYS_TEST2_KPK))
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Incorrect number of keys", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PKCS SHA-1 signing */
    BDBG_LOG(("%s - Test RSA PKCS SHA-1 signing", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaSignSettings(&rsaSignVerifySettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaSign(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, Sig_PkcsSha1, SIG_PKCS_SHA1_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Signing output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Signing error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PKCS SHA-1 verifying */
    BDBG_LOG(("%s - Test RSA PKCS SHA-1 verifying", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaVerifySettings(&rsaSignVerifySettings);
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;

    if(SecureRsaTl_RsaVerify(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Verify error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PKCS SHA-256 signing */
    BDBG_LOG(("%s - Test RSA PKCS SHA-256 signing", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaSignSettings(&rsaSignVerifySettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaSign(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, Sig_PkcsSha256, SIG_PKCS_SHA256_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Signing output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Signing error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PKCS SHA-256 verifying */
    BDBG_LOG(("%s - Test RSA PKCS SHA-256 verifying", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaVerifySettings(&rsaSignVerifySettings);
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;

    if(SecureRsaTl_RsaVerify(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Verify error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PSS with salt length 0 and SHA-1 signing */
    BDBG_LOG(("%s - Test RSA PSS with salt length 0 and SHA-1 signing", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaSignSettings(&rsaSignVerifySettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    rsaSignVerifySettings.sigPssSaltLen = 0;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaSign(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, Sig_PssSha1Salt0,
                       SIG_PSS_SHA1_SALT0_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Signing output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Signing error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PSS with salt length 0 and SHA-1 verifying */
    BDBG_LOG(("%s - Test RSA PSS with salt length 0 and SHA-1 verifying", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaVerifySettings(&rsaSignVerifySettings);
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    rsaSignVerifySettings.sigPssSaltLen = 0;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;

    if(SecureRsaTl_RsaVerify(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Verify error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PSS with salt length 16 and SHA-256 signing */
    BDBG_LOG(("%s - Test RSA PSS with salt length 16 and SHA-256 signing", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaSignSettings(&rsaSignVerifySettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    rsaSignVerifySettings.sigPssSaltLen = SALT_LEN_16;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaSign(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Signing error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PSS with salt length 16 and SHA-256 verifying */
    BDBG_LOG(("%s - Test RSA PSS with salt length 16 and SHA-256 verifying", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaVerifySettings(&rsaSignVerifySettings);
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    rsaSignVerifySettings.sigPssSaltLen = SALT_LEN_16;
    rsaSignVerifySettings.signature = RsaBuffer;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;

    if(SecureRsaTl_RsaVerify(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Verify error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA PSS with salt length 16 and SHA-256 verifying with a pre-generated signature */
    BDBG_LOG(("%s - Test RSA PSS with salt length 16 and SHA-256 verifying with a pre-generated signature", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaVerifySettings(&rsaSignVerifySettings);
    RsaBufferLen = SIG_PSS_SHA256_SALT16_LEN;
    rsaSignVerifySettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaSignVerifySettings.input = TestData;
    rsaSignVerifySettings.inputLen = RSA_KEY_LEN;
    rsaSignVerifySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    rsaSignVerifySettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    rsaSignVerifySettings.sigPssSaltLen = SALT_LEN_16;
    rsaSignVerifySettings.signature = Sig_PssSha256Salt16;
    rsaSignVerifySettings.signatureLen = &RsaBufferLen;

    if(SecureRsaTl_RsaVerify(hSecureRsaTl, &rsaSignVerifySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Verify error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA host usage with a private key */
    BDBG_LOG(("%s - Test RSA host usage with a private key", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaHostUsageSettings(&rsaHostUsageSettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaHostUsageSettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaHostUsageSettings.rsaKeyType = SecureRsaTl_RsaKeyType_ePrivate;
    rsaHostUsageSettings.input = TestData;
    rsaHostUsageSettings.inputLen = RSA_KEY_LEN;
    rsaHostUsageSettings.output = RsaBuffer;
    rsaHostUsageSettings.outputLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaHostUsage(hSecureRsaTl, &rsaHostUsageSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, HostUsage_PrivateA, RsaBufferLen) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Host usage output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Host usage error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA host usage with a private/public key */
    BDBG_LOG(("%s - Test RSA host usage with a private/public key", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaHostUsageSettings(&rsaHostUsageSettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaHostUsageSettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaHostUsageSettings.rsaKeyType = SecureRsaTl_RsaKeyType_ePublic;
    rsaHostUsageSettings.input = TestData;
    rsaHostUsageSettings.inputLen = RSA_KEY_LEN;
    rsaHostUsageSettings.output = RsaBuffer;
    rsaHostUsageSettings.outputLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaHostUsage(hSecureRsaTl, &rsaHostUsageSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, HostUsage_PublicA, RsaBufferLen) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Host usage output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Host usage error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA host usage with a public key */
    BDBG_LOG(("%s - Test RSA host usage with a public key", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaHostUsageSettings(&rsaHostUsageSettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaHostUsageSettings.rsaKeySlot = RSA_KEY_SLOT_VERIFY;
    rsaHostUsageSettings.rsaKeyType = SecureRsaTl_RsaKeyType_ePublic;
    rsaHostUsageSettings.input = TestData;
    rsaHostUsageSettings.inputLen = RSA_KEY_LEN;
    rsaHostUsageSettings.output = RsaBuffer;
    rsaHostUsageSettings.outputLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaHostUsage(hSecureRsaTl, &rsaHostUsageSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, HostUsage_PublicB, RsaBufferLen) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Host usage output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Host usage error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 IKR128 with PKCS encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 IKR128 with PKCS encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_IKR;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_IKR128_PKCS_LEN;
    decryptAesSettings.encKey = Enc_Ikr128_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_IKR_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Ikr;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_Ikr128_PkcsSha1;
    decryptAesSettings.signatureLen = SIG_IKR128_PKCS_SHA1_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_IKR_A;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 IKR256 with PKCS encryption, PKCS signature with SHA-256 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 IKR256 with PKCS encryption, PKCS signature with SHA-256 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_IKR;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_IKR256_PKCS_LEN;
    decryptAesSettings.encKey = Enc_Ikr256_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_IKR_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Ikr;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_Ikr256_PkcsSha256;
    decryptAesSettings.signatureLen = SIG_IKR256_PKCS_SHA256_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_256;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_IKR_B;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR128 export */
    BDBG_LOG(("%s - Test Key3 IKR128 export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    BKNI_Memset(ImportExportBuffer, 0, ImportExportBufferLen);

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR128 import */
    BDBG_LOG(("%s - Test Key3 IKR128 import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_C;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR256 export */
    BDBG_LOG(("%s - Test Key3 IKR256 export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    BKNI_Memset(ImportExportBuffer, 0, ImportExportBufferLen);

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR256 import */
    BDBG_LOG(("%s - Test Key3 IKR256 import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_D;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 128-bit Key3 HMAC */
    BDBG_LOG(("%s - Test 128-bit Key3 HMAC", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA1_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha1;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha1_Ikr128, HMAC_SHA1_IKR128_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA1_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_C;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha1;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha1_Ikr128, HMAC_SHA1_IKR128_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 256-bit Key3 HMAC */
    BDBG_LOG(("%s - Test 256-bit Key3 HMAC", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA256_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha256;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha256_Ikr256,
                       HMAC_SHA256_IKR256_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA256_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_D;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha256;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha256_Ikr256,
                       HMAC_SHA256_IKR256_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 128-bit Key3 appended SHA with input length 0 */
    BDBG_LOG(("%s - Test 128-bit Key3 appended SHA with input length 0", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3AppendShaSettings(&key3AppendShaSettings);
    DigestBufferLen = DIGEST_LEN_SHA1;
    key3AppendShaSettings.inputData = NULL;
    key3AppendShaSettings.inputDataLen = 0;
    key3AppendShaSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3AppendShaSettings.digestType = SecureRsaTl_DigestType_eSha1;
    key3AppendShaSettings.digest = DigestBuffer;
    key3AppendShaSettings.digestLen = &DigestBufferLen;
    BKNI_Memset(DigestBuffer, 0, DigestBufferLen);

    if(SecureRsaTl_Key3AppendSha(hSecureRsaTl, &key3AppendShaSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(DigestBuffer, KeyAppendSha1_Ikr128_A,
                       KEY_APPEND_SHA1_IKR128_A_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - SHA output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - SHA calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 128-bit Key3 appended SHA with input length 3 */
    BDBG_LOG(("%s - Test 128-bit Key3 appended SHA with input length 3", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3AppendShaSettings(&key3AppendShaSettings);
    DigestBufferLen = DIGEST_LEN_SHA1;
    key3AppendShaSettings.inputData = DataAbc;
    key3AppendShaSettings.inputDataLen = DATA_ABC_LEN;
    key3AppendShaSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3AppendShaSettings.digestType = SecureRsaTl_DigestType_eSha1;
    key3AppendShaSettings.digest = DigestBuffer;
    key3AppendShaSettings.digestLen = &DigestBufferLen;
    BKNI_Memset(DigestBuffer, 0, DigestBufferLen);

    if(SecureRsaTl_Key3AppendSha(hSecureRsaTl, &key3AppendShaSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(DigestBuffer, KeyAppendSha1_Ikr128_B,
                       KEY_APPEND_SHA1_IKR128_B_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - SHA output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - SHA calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 256-bit Key3 appended SHA with input length 0 */
    BDBG_LOG(("%s - Test 256-bit Key3 appended SHA with input length 0", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3AppendShaSettings(&key3AppendShaSettings);
    DigestBufferLen = DIGEST_LEN_SHA256;
    key3AppendShaSettings.inputData = NULL;
    key3AppendShaSettings.inputDataLen = 0;
    key3AppendShaSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    key3AppendShaSettings.digestType = SecureRsaTl_DigestType_eSha256;
    key3AppendShaSettings.digest = DigestBuffer;
    key3AppendShaSettings.digestLen = &DigestBufferLen;
    BKNI_Memset(DigestBuffer, 0, DigestBufferLen);

    if(SecureRsaTl_Key3AppendSha(hSecureRsaTl, &key3AppendShaSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(DigestBuffer, KeyAppendSha256_Ikr256_A,
                       KEY_APPEND_SHA256_IKR256_A_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - SHA output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - SHA calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 256-bit Key3 appended SHA with input length 3 */
    BDBG_LOG(("%s - Test 256-bit Key3 appended SHA with input length 3", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3AppendShaSettings(&key3AppendShaSettings);
    DigestBufferLen = DIGEST_LEN_SHA256;
    key3AppendShaSettings.inputData = DataAbc;
    key3AppendShaSettings.inputDataLen = DATA_ABC_LEN;
    key3AppendShaSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    key3AppendShaSettings.digestType = SecureRsaTl_DigestType_eSha256;
    key3AppendShaSettings.digest = DigestBuffer;
    key3AppendShaSettings.digestLen = &DigestBufferLen;
    BKNI_Memset(DigestBuffer, 0, DigestBufferLen);

    if(SecureRsaTl_Key3AppendSha(hSecureRsaTl, &key3AppendShaSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(DigestBuffer, KeyAppendSha256_Ikr256_B,
                       KEY_APPEND_SHA256_IKR256_B_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - SHA output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - SHA calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 IKR CA with OAEP encryption, PSS signature with salt length 0 and SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 IKR CA with OAEP encryption, PSS signature with salt length 0 and SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_IKR;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_eOaep;
    decryptAesSettings.encDigestType = SecureRsaTl_DigestType_eSha256;
    decryptAesSettings.encKeyLen = ENC_IKRCA_OAEP_SHA256_LEN;
    decryptAesSettings.encKey = Enc_IkrCa_OaepSha256;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_IKR_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Ikr;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    decryptAesSettings.sigPssSaltLen = 0;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_IkrCa_PssSha1Salt0;
    decryptAesSettings.signatureLen = SIG_IKRCA_PSS_SHA1_SALT0_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_IKR_A;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 IKR CP with OAEP encryption, PSS signature with salt length 16 and SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 IKR CP with OAEP encryption, PSS signature with salt length 16 and SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_IKR;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_eOaep;
    decryptAesSettings.encDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.encKeyLen = ENC_IKRCP_OAEP_SHA1_LEN;
    decryptAesSettings.encKey = Enc_IkrCp_OaepSha1;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_IKR_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Ikr;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    decryptAesSettings.sigPssSaltLen = SALT_LEN_16;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_IkrCp_PssSha256Salt16;
    decryptAesSettings.signatureLen = SIG_IKRCP_PSS_SHA256_SALT16_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_IKR_B;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CA export */
    BDBG_LOG(("%s - Test Key3 IKR CA export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    BKNI_Memset(ImportExportBuffer, 0, ImportExportBufferLen);

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CA import */
    BDBG_LOG(("%s - Test Key3 IKR CA import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_C;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CP export */
    BDBG_LOG(("%s - Test Key3 IKR CP export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    BKNI_Memset(ImportExportBuffer, 0, ImportExportBufferLen);

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CP import */
    BDBG_LOG(("%s - Test Key3 IKR CP import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_D;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CA routing and usage */
    BDBG_LOG(("%s - Test Key3 IKR CA routing and usage", __FUNCTION__));

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings);
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3RouteSettings.keySlotUsage = SecureRsaTl_Key3Usage_eCa;
    key3RouteSettings.keySlotNum = KEY_SLOT_NUMBER_CA1;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eDecrypt;
    key3RouteSettings.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3RouteSettings.keyType = BSAGElib_Crypto_KeyType_eOddAndEven;
    key3RouteSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
    key3RouteSettings.iv = Ca_AesCbc_Iv;
    key3RouteSettings.ivLen = CA_AESCBC_IV_LEN;
    key3RouteSettings.procIn = NULL;
    key3RouteSettings.procInLen = 0;

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings2);
    key3RouteSettings2.key3KeySlot = KEY3_KEY_SLOT_IKR_C;
    key3RouteSettings2.keySlotUsage = SecureRsaTl_Key3Usage_eCa;
    key3RouteSettings2.keySlotNum = KEY_SLOT_NUMBER_CA2;
    key3RouteSettings2.operation = BSAGElib_Crypto_Operation_eDecrypt;
    key3RouteSettings2.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings2.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3RouteSettings2.keyType = BSAGElib_Crypto_KeyType_eOddAndEven;
    key3RouteSettings2.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
    key3RouteSettings2.iv = Ca_AesCbc_Iv;
    key3RouteSettings2.ivLen = CA_AESCBC_IV_LEN;
    key3RouteSettings2.procIn = NULL;
    key3RouteSettings2.procInLen = 0;

    if((SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS) &&
       (SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings2) == BERR_SUCCESS))
    {
        /* Test playback */
        if(PlaybackCaKeys() == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Playback failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings2);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;
        key3UnrouteSettings2.key3KeySlot = key3RouteSettings2.key3KeySlot;

        if((SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS) ||
           (SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings2) != BERR_SUCCESS))
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CP routing and AES-CBC encryption */
    BDBG_LOG(("%s - Test Key3 IKR CP routing and AES-CBC encryption", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings);
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    key3RouteSettings.keySlotUsage = SecureRsaTl_Key3Usage_eCp;
    key3RouteSettings.keySlotNum = KEY_SLOT_NUMBER_CP;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eEncrypt;
    key3RouteSettings.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3RouteSettings.keyType = BSAGElib_Crypto_KeyType_eClear;
    key3RouteSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eClear;
    key3RouteSettings.iv = Cp_AesCbc_Iv;
    key3RouteSettings.ivLen = CP_AESCBC_IV_LEN;
    key3RouteSettings.procIn = NULL;
    key3RouteSettings.procInLen = 0;

    if(SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS)
    {
        /* Test M2M */
        if(RunM2mTest(Cp_AesCbc_Input, CP_AESCBC_INPUT_LEN,
                      M2mOutput, Cp_AesCbc_Output) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - M2M failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;

        if(SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CP routing and AES-CBC decryption */
    BDBG_LOG(("%s - Test Key3 IKR CP routing and AES-CBC decryption", __FUNCTION__));
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_D;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eDecrypt;

    if(SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS)
    {
        /* Test M2M */
        if(RunM2mTest(M2mOutput, CP_AESCBC_OUTPUT_LEN,
                      M2mOutput2, Cp_AesCbc_Input) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - M2M failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;

        if(SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 CA_128_4 with PKCS encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 CA_128_4 with PKCS encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_CA;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_CA_PKCS_LEN;
    decryptAesSettings.encKey = Enc_Ca_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_CA_128_4_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Ca_128_4;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_Ca_128_4_Pkcs;
    decryptAesSettings.signatureLen = SIG_CA_128_4_PKCS_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_CA_A;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CA_128_4 export */
    BDBG_LOG(("%s - Test Key3 CA_128_4 export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CA_A;

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CA_128_4 import */
    BDBG_LOG(("%s - Test Key3 CA_128_4 import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CA_B;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CA_128_4 routing and usage */
    BDBG_LOG(("%s - Test Key3 CA_128_4 routing and usage", __FUNCTION__));

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings);
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_CA_A;
    key3RouteSettings.keySlotNum = KEY_SLOT_NUMBER_CA1;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eDecrypt;
    key3RouteSettings.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3RouteSettings.keyType = BSAGElib_Crypto_KeyType_eOddAndEven;
    key3RouteSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
    key3RouteSettings.iv = Ca_AesCbc_Iv;
    key3RouteSettings.ivLen = CA_AESCBC_IV_LEN;
    key3RouteSettings.keyLadderAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.procIn = Ca_128_4_ProcIn;
    key3RouteSettings.procInLen = CA_128_4_PROCIN_LEN;

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings2);
    key3RouteSettings2.key3KeySlot = KEY3_KEY_SLOT_CA_B;
    key3RouteSettings2.keySlotNum = KEY_SLOT_NUMBER_CA2;
    key3RouteSettings2.operation = BSAGElib_Crypto_Operation_eDecrypt;
    key3RouteSettings2.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings2.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3RouteSettings2.keyType = BSAGElib_Crypto_KeyType_eOddAndEven;
    key3RouteSettings2.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
    key3RouteSettings2.iv = Ca_AesCbc_Iv;
    key3RouteSettings2.ivLen = CA_AESCBC_IV_LEN;
    key3RouteSettings2.keyLadderAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings2.procIn = Ca_128_4_ProcIn;
    key3RouteSettings2.procInLen = CA_128_4_PROCIN_LEN;

    if((SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS) &&
       (SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings2) == BERR_SUCCESS))
    {
        /* Test playback */
        if(PlaybackCaKeys() == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Playback failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings2);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;
        key3UnrouteSettings2.key3KeySlot = key3RouteSettings2.key3KeySlot;

        if((SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS) ||
           (SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings2) != BERR_SUCCESS))
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 CA_128_5 with PKCS encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 CA_128_5 with PKCS encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_CA;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_CA_PKCS_LEN;
    decryptAesSettings.encKey = Enc_Ca_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_CA_128_5_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Ca_128_5;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_Ca_128_5_Pkcs;
    decryptAesSettings.signatureLen = SIG_CA_128_5_PKCS_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_CA_A;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CA_128_5 export */
    BDBG_LOG(("%s - Test Key3 CA_128_5 export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CA_A;
    BKNI_Memset(ImportExportBuffer, 0, ImportExportBufferLen);

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CA_128_5 import */
    BDBG_LOG(("%s - Test Key3 CA_128_5 import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CA_B;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CA_128_5 key routing and usage */
    BDBG_LOG(("%s - Test Key3 CA_128_5 routing and usage", __FUNCTION__));

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings);
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_CA_A;
    key3RouteSettings.keySlotNum = KEY_SLOT_NUMBER_CA1;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eDecrypt;
    key3RouteSettings.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3RouteSettings.keyType = BSAGElib_Crypto_KeyType_eOddAndEven;
    key3RouteSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
    key3RouteSettings.iv = Ca_AesCbc_Iv;
    key3RouteSettings.ivLen = CA_AESCBC_IV_LEN;
    key3RouteSettings.keyLadderAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.procIn = Ca_128_5_ProcIn;
    key3RouteSettings.procInLen = CA_128_5_PROCIN_LEN;

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings2);
    key3RouteSettings2.key3KeySlot = KEY3_KEY_SLOT_CA_B;
    key3RouteSettings2.keySlotNum = KEY_SLOT_NUMBER_CA2;
    key3RouteSettings2.operation = BSAGElib_Crypto_Operation_eDecrypt;
    key3RouteSettings2.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings2.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3RouteSettings2.keyType = BSAGElib_Crypto_KeyType_eOddAndEven;
    key3RouteSettings2.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
    key3RouteSettings2.iv = Ca_AesCbc_Iv;
    key3RouteSettings2.ivLen = CA_AESCBC_IV_LEN;
    key3RouteSettings2.keyLadderAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings2.procIn = Ca_128_5_ProcIn;
    key3RouteSettings2.procInLen = CA_128_5_PROCIN_LEN;

    if((SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS) &&
       (SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings2) == BERR_SUCCESS))
    {
        /* Test playback */
        if(PlaybackCaKeys() == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Playback failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings2);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;
        key3UnrouteSettings2.key3KeySlot = key3RouteSettings2.key3KeySlot;

        if((SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS) ||
           (SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings2) != BERR_SUCCESS))
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 CP_128_4 with PKCS encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 CP_128_4 with PKCS encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_CP;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_CP_PKCS_LEN;
    decryptAesSettings.encKey = Enc_Cp_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_CP_128_4_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Cp_128_4;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_Cp_128_4_Pkcs;
    decryptAesSettings.signatureLen = SIG_CP_128_4_PKCS_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_CP_A;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CP_128_4 export */
    BDBG_LOG(("%s - Test Key3 CP_128_4 export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CP_A;
    BKNI_Memset(ImportExportBuffer, 0, ImportExportBufferLen);

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CP_128_4 import */
    BDBG_LOG(("%s - Test Key3 CP_128_4 import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CP_B;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CP_128_4 routing and usage */
    BDBG_LOG(("%s - Test Key3 CP_128_4 routing and usage", __FUNCTION__));

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings);
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_CP_A;
    key3RouteSettings.keySlotNum = KEY_SLOT_NUMBER_CP;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eEncrypt;
    key3RouteSettings.routeAlgorithm = BSAGElib_Crypto_Algorithm_e3DesAba;
    key3RouteSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eEcb;
    key3RouteSettings.keyType = BSAGElib_Crypto_KeyType_eClear;
    key3RouteSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eClear;
    key3RouteSettings.iv = NULL;
    key3RouteSettings.ivLen = 0;
    key3RouteSettings.keyLadderAlgorithm = BSAGElib_Crypto_Algorithm_e3DesAba;
    key3RouteSettings.procIn = Cp_128_4_ProcIn;
    key3RouteSettings.procInLen = CP_128_4_PROCIN_LEN;

    if(SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS)
    {
        /* Test M2M */
        if(RunM2mTest(Cp_3Des_Input, CP_3DES_INPUT_LEN,
                      M2mOutput, Cp_3Des_Output) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - M2M failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;

        if(SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt Key3 CP_128_5 with PKCS encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt Key3 CP_128_5 with PKCS encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_CP;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_CP_PKCS_LEN;
    decryptAesSettings.encKey = Enc_Cp_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_CP_128_5_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Cp_128_5;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_Cp_128_5_Pkcs;
    decryptAesSettings.signatureLen = SIG_CP_128_5_PKCS_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_CP_A;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CP_128_5 export */
    BDBG_LOG(("%s - Test Key3 CP_128_5 export", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ExportSettings(&key3ImportExportSettings);
    ImportExportBufferLen = IMPORT_EXPORT_BUFFER_LEN;
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CP_A;
    BKNI_Memset(ImportExportBuffer, 0, ImportExportBufferLen);

    if(SecureRsaTl_Key3Export(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 export failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CP_128_5 import */
    BDBG_LOG(("%s - Test Key3 CP_128_5 import", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3ImportSettings(&key3ImportExportSettings);
    key3ImportExportSettings.encKey3 = ImportExportBuffer;
    key3ImportExportSettings.encKey3Len = &ImportExportBufferLen;
    key3ImportExportSettings.key3KeySlot = KEY3_KEY_SLOT_CP_B;

    if(SecureRsaTl_Key3Import(hSecureRsaTl, &key3ImportExportSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 import failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 CP_128_5 routing and usage */
    BDBG_LOG(("%s - Test Key3 CP_128_5 routing and usage", __FUNCTION__));

    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings);
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_CP_B;
    key3RouteSettings.keySlotNum = KEY_SLOT_NUMBER_CP;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eEncrypt;
    key3RouteSettings.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eEcb;
    key3RouteSettings.keyType = BSAGElib_Crypto_KeyType_eClear;
    key3RouteSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eClear;
    key3RouteSettings.iv = NULL;
    key3RouteSettings.ivLen = 0;
    key3RouteSettings.keyLadderAlgorithm = BSAGElib_Crypto_Algorithm_e3DesAba;
    key3RouteSettings.procIn = Cp_128_5_ProcIn;
    key3RouteSettings.procInLen = CP_128_5_PROCIN_LEN;

    if(SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS)
    {
        /* Test M2M */
        if(RunM2mTest(Cp_AesEcb_Input, CP_AESECB_INPUT_LEN,
                      M2mOutput, Cp_AesEcb_Output) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - M2M failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;

        if(SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 load clear IKR CP */
    BDBG_LOG(("%s - Test Key3 load clear IKR CP", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3LoadClearIkrSettings(&key3LoadClearIkrSettings);
    key3LoadClearIkrSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3LoadClearIkrSettings.key = IkrCp_ClearKey;
    key3LoadClearIkrSettings.keyLen = IKRCP_CLEAR_KEY_LEN;

    if(SecureRsaTl_Key3LoadClearIkr(hSecureRsaTl, &key3LoadClearIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 load clear IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CP routing and AES-CTR encryption */
    BDBG_LOG(("%s - Test Key3 IKR CP routing and AES-CTR encryption", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3RouteSettings(&key3RouteSettings);
    key3RouteSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3RouteSettings.keySlotUsage = SecureRsaTl_Key3Usage_eCp;
    key3RouteSettings.keySlotNum = KEY_SLOT_NUMBER_CP;
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eEncrypt;
    key3RouteSettings.routeAlgorithm = BSAGElib_Crypto_Algorithm_eAes;
    key3RouteSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCounter;
    key3RouteSettings.keyType = BSAGElib_Crypto_KeyType_eClear;
    key3RouteSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eClear;
    key3RouteSettings.iv = Cp_AesCtr_Iv;
    key3RouteSettings.ivLen = CP_AESCTR_IV_LEN;
    key3RouteSettings.procIn = NULL;
    key3RouteSettings.procInLen = 0;

    if(SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS)
    {
        /* Test M2M */
        if(RunM2mTest(Cp_AesCtr_Input, CP_AESCTR_INPUT_LEN,
                      M2mOutput, Cp_AesCtr_Output) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - M2M failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;

        if(SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 IKR CP routing and AES-CTR decryption */
    BDBG_LOG(("%s - Test Key3 IKR CP routing and AES-CTR decryption", __FUNCTION__));
    key3RouteSettings.operation = BSAGElib_Crypto_Operation_eDecrypt;
    key3RouteSettings.iv = Cp_AesCtr_Iv;
    key3RouteSettings.ivLen = CP_AESCTR_IV_LEN;

    if(SecureRsaTl_Key3Route(hSecureRsaTl, &key3RouteSettings) == BERR_SUCCESS)
    {
        /* Test M2M */
        if(RunM2mTest(M2mOutput, CP_AESCTR_OUTPUT_LEN,
                      M2mOutput2, Cp_AesCtr_Input) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - M2M failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }

        /* Unroute the key */
        SecureRsaTl_GetDefaultKey3UnrouteSettings(&key3UnrouteSettings);
        key3UnrouteSettings.key3KeySlot = key3RouteSettings.key3KeySlot;

        if(SecureRsaTl_Key3Unroute(hSecureRsaTl, &key3UnrouteSettings) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s -    FAIL - Unroute keyslot failed", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Route keyslot failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test Key3 load clear IKR256 */
    BDBG_LOG(("%s - Test Key3 load clear IKR256", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3LoadClearIkrSettings(&key3LoadClearIkrSettings);
    key3LoadClearIkrSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3LoadClearIkrSettings.key = Ikr256_ClearKey;
    key3LoadClearIkrSettings.keyLen = IKR256_CLEAR_KEY_LEN;

    if(SecureRsaTl_Key3LoadClearIkr(hSecureRsaTl, &key3LoadClearIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key3 load clear IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 256-bit Key3 HMAC */
    BDBG_LOG(("%s - Test 256-bit Key3 HMAC", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA256_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha256;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha256_Ikr256,
                       HMAC_SHA256_IKR256_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt KPK with PKCS encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test RSA decrypt KPK with PKCS encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKpkSettings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_KPK;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_KPK_A_PKCS_LEN;
    decryptAesSettings.encKey = Enc_KpkA_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_KPK_A_LEN;
    decryptAesSettings.encKeySettings = KeySettings_KpkA;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_KpkA_PkcsSha1;
    decryptAesSettings.signatureLen = SIG_KPK_A_PKCS_SHA1_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KPK_KEY_SLOT_A;

    if(SecureRsaTl_RsaDecryptKpk(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt KPK failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test RSA decrypt KPK with OAEP encryption, unsigned */
    BDBG_LOG(("%s - Test RSA decrypt KPK with OAEP encryption, unsigned", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKpkSettings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_KPK;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_eOaep;
    decryptAesSettings.encDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.encKeyLen = ENC_KPK_B_OAEP_SHA1_LEN;
    decryptAesSettings.encKey = Enc_KpkB_OaepSha1;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_KPK_B_LEN;
    decryptAesSettings.encKeySettings = KeySettings_KpkB;
    decryptAesSettings.signature = NULL;
    decryptAesSettings.signatureLen = 0;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KPK_KEY_SLOT_B;

    if(SecureRsaTl_RsaDecryptKpk(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt KPK failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test KPK decrypt IKR with AES-ECB encryption */
    BDBG_LOG(("%s - Test KPK decrypt IKR with AES-ECB encryption", __FUNCTION__));
    SecureRsaTl_GetDefaultKpkDecryptIkrSettings(&kpkDecryptIkrSettings);
    kpkDecryptIkrSettings.ikrKeySlot = KEY3_KEY_SLOT_IKR_A;
    kpkDecryptIkrSettings.encKeySlot = KPK_KEY_SLOT_A;
    kpkDecryptIkrSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eEcb;
    kpkDecryptIkrSettings.encKey = Enc_KpkA_Ikr128;
    kpkDecryptIkrSettings.encKeyLen = ENC_KPK_A_IKR128_LEN;

    if(SecureRsaTl_KpkDecryptIkr(hSecureRsaTl, &kpkDecryptIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - KPK Decrypt IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 128-bit Key3 HMAC */
    BDBG_LOG(("%s - Test 128-bit Key3 HMAC", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA1_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha1;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha1_Ikr128, HMAC_SHA1_IKR128_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test KPK decrypt IKR with AES-CBC encryption */
    BDBG_LOG(("%s - Test KPK decrypt IKR with AES-CBC encryption", __FUNCTION__));
    SecureRsaTl_GetDefaultKpkDecryptIkrSettings(&kpkDecryptIkrSettings);
    kpkDecryptIkrSettings.ikrKeySlot = KEY3_KEY_SLOT_IKR_B;
    kpkDecryptIkrSettings.encKeySlot = KPK_KEY_SLOT_B;
    kpkDecryptIkrSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    kpkDecryptIkrSettings.encKey = Enc_KpkB_Ikr256;
    kpkDecryptIkrSettings.encKeyLen = ENC_KPK_B_IKR256_LEN;
    kpkDecryptIkrSettings.encIv = Iv_KpkB_Ikr256;
    kpkDecryptIkrSettings.encIvLen = IV_KPK_B_IKR256_LEN;

    if(SecureRsaTl_KpkDecryptIkr(hSecureRsaTl, &kpkDecryptIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - KPK Decrypt IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 256-bit Key3 HMAC */
    BDBG_LOG(("%s - Test 256-bit Key3 HMAC", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA256_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha256;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha256_Ikr256,
                       HMAC_SHA256_IKR256_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test IKR decrypt IKR with AES-ECB encryption */
    BDBG_LOG(("%s - Test IKR decrypt IKR with AES-ECB encryption", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3IkrDecryptIkrSettings(&key3IkrDecryptIkrSettings);
    key3IkrDecryptIkrSettings.ikrKeySlot = KEY3_KEY_SLOT_IKR_B;
    key3IkrDecryptIkrSettings.encKeySlot = KEY3_KEY_SLOT_IKR_A;
    key3IkrDecryptIkrSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eEcb;
    key3IkrDecryptIkrSettings.enableTransform = 0;
    key3IkrDecryptIkrSettings.encKey = Enc_Ikr128_Ikr128A;
    key3IkrDecryptIkrSettings.encKeyLen = ENC_IKR128_IKR128_A_LEN;
    key3IkrDecryptIkrSettings.encIv = NULL;
    key3IkrDecryptIkrSettings.encIvLen = 0;

    if(SecureRsaTl_Key3IkrDecryptIkr(hSecureRsaTl, &key3IkrDecryptIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - IKR Decrypt IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test IKR decrypt IKR with AES-CBC encryption */
    BDBG_LOG(("%s - Test IKR decrypt IKR with AES-CBC encryption", __FUNCTION__));
    key3IkrDecryptIkrSettings.ikrKeySlot = KEY3_KEY_SLOT_IKR_A;
    key3IkrDecryptIkrSettings.encKeySlot = KEY3_KEY_SLOT_IKR_B;
    key3IkrDecryptIkrSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3IkrDecryptIkrSettings.enableTransform = 0;
    key3IkrDecryptIkrSettings.encKey = Enc_Ikr128A_Ikr128B;
    key3IkrDecryptIkrSettings.encKeyLen = ENC_IKR128_A_IKR128_B_LEN;
    key3IkrDecryptIkrSettings.encIv = Iv_IkrIkr;
    key3IkrDecryptIkrSettings.encIvLen = IV_IKR_IKR_LEN;

    if(SecureRsaTl_Key3IkrDecryptIkr(hSecureRsaTl, &key3IkrDecryptIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - IKR Decrypt IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test IKR decrypt IKR with transform and AES-ECB encryption */
    BDBG_LOG(("%s - Test IKR decrypt IKR with transform and AES-ECB encryption", __FUNCTION__));
    key3IkrDecryptIkrSettings.ikrKeySlot = KEY3_KEY_SLOT_IKR_B;
    key3IkrDecryptIkrSettings.encKeySlot = KEY3_KEY_SLOT_IKR_A;
    key3IkrDecryptIkrSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eEcb;
    key3IkrDecryptIkrSettings.enableTransform = 1;
    key3IkrDecryptIkrSettings.encKey = Enc_Ikr128B_Ikr128C;
    key3IkrDecryptIkrSettings.encKeyLen = ENC_IKR128_B_IKR128_C_LEN;
    key3IkrDecryptIkrSettings.encIv = NULL;
    key3IkrDecryptIkrSettings.encIvLen = 0;

    if(SecureRsaTl_Key3IkrDecryptIkr(hSecureRsaTl, &key3IkrDecryptIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - IKR Decrypt IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test IKR decrypt IKR with transform and AES-CBC encryption */
    BDBG_LOG(("%s - Test IKR decrypt IKR with transform and AES-CBC encryption", __FUNCTION__));
    key3IkrDecryptIkrSettings.ikrKeySlot = KEY3_KEY_SLOT_IKR_A;
    key3IkrDecryptIkrSettings.encKeySlot = KEY3_KEY_SLOT_IKR_B;
    key3IkrDecryptIkrSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    key3IkrDecryptIkrSettings.enableTransform = 1;
    key3IkrDecryptIkrSettings.encKey = Enc_Ikr128C_Ikr256;
    key3IkrDecryptIkrSettings.encKeyLen = ENC_IKR128_C_IKR256_LEN;
    key3IkrDecryptIkrSettings.encIv = Iv_IkrIkr;
    key3IkrDecryptIkrSettings.encIvLen = IV_IKR_IKR_LEN;

    if(SecureRsaTl_Key3IkrDecryptIkr(hSecureRsaTl, &key3IkrDecryptIkrSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - IKR Decrypt IKR failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 256-bit Key3 HMAC */
    BDBG_LOG(("%s - Test 256-bit Key3 HMAC", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA256_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha256;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha256_Ikr256,
                       HMAC_SHA256_IKR256_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test load RSA 1024 public key, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test load RSA 1024 public key, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaLoadPublicKeySettings(&rsaLoadPublicKeySettings);
    rsaLoadPublicKeySettings.rsaKeySlot = RSA_KEY_SLOT_PUBLIC_A;
    rsaLoadPublicKeySettings.publicModulus = RsaPublic1024;
    rsaLoadPublicKeySettings.publicModulusLen = RSA_PUBLIC_1024_LEN;
    rsaLoadPublicKeySettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    rsaLoadPublicKeySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    rsaLoadPublicKeySettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    rsaLoadPublicKeySettings.signature = Sig_Public_Verify_1024_PkcsSha1;
    rsaLoadPublicKeySettings.signatureLen = SIG_PUBLIC_VERIFY_1024_PKCS_SHA1_LEN;

    if(SecureRsaTl_RsaLoadPublicKey(hSecureRsaTl, &rsaLoadPublicKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Load RSA public key failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test remove RSA verify key */
    BDBG_LOG(("%s - Test remove RSA verify key", __FUNCTION__));
    SecureRsaTl_GetDefaultRemoveKeySettings(&removeKeySettings);

    removeKeySettings.keyType = SecureRsaTl_KeyType_eRsa;
    removeKeySettings.keyslot.rsaKeySlot = RSA_KEY_SLOT_VERIFY;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test load RSA 2048 public key, PKCS signature with SHA-256 digest */
    BDBG_LOG(("%s - Test load RSA 2048 public key, PKCS signature with SHA-256 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaLoadPublicKeySettings(&rsaLoadPublicKeySettings);
    rsaLoadPublicKeySettings.rsaKeySlot = RSA_KEY_SLOT_VERIFY;
    rsaLoadPublicKeySettings.publicModulus = RsaPublic2048;
    rsaLoadPublicKeySettings.publicModulusLen = RSA_PUBLIC_2048_LEN;
    rsaLoadPublicKeySettings.sigKeySlot = RSA_KEY_SLOT_PUBLIC_A;
    rsaLoadPublicKeySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    rsaLoadPublicKeySettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    rsaLoadPublicKeySettings.signature = Sig_Public_1024_2048_PkcsSha256;
    rsaLoadPublicKeySettings.signatureLen = SIG_PUBLIC_1024_2048_PKCS_SHA1_LEN;

    if(SecureRsaTl_RsaLoadPublicKey(hSecureRsaTl, &rsaLoadPublicKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Load RSA public key failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test load RSA 3072 public key, PSS signature with salt length 0 and SHA-1 digest */
    BDBG_LOG(("%s - Test load RSA 3072 public key, PSS signature with salt length 0 and SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaLoadPublicKeySettings(&rsaLoadPublicKeySettings);
    rsaLoadPublicKeySettings.rsaKeySlot = RSA_KEY_SLOT_PUBLIC_B;
    rsaLoadPublicKeySettings.publicModulus = RsaPublic3072;
    rsaLoadPublicKeySettings.publicModulusLen = RSA_PUBLIC_3072_LEN;
    rsaLoadPublicKeySettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    rsaLoadPublicKeySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    rsaLoadPublicKeySettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    rsaLoadPublicKeySettings.sigPssSaltLen = 0;
    rsaLoadPublicKeySettings.signature = Sig_Public_2048_3072_PssSha1Salt0;
    rsaLoadPublicKeySettings.signatureLen = SIG_PUBLIC_2048_3072_PKCS_SHA1_LEN;

    if(SecureRsaTl_RsaLoadPublicKey(hSecureRsaTl, &rsaLoadPublicKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Load RSA public key failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test load RSA 1024 public key, PSS signature with salt length 16 and SHA-1 digest */
    BDBG_LOG(("%s - Test load RSA 1024 public key, PSS signature with salt length 16 and SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaLoadPublicKeySettings(&rsaLoadPublicKeySettings);
    rsaLoadPublicKeySettings.rsaKeySlot = RSA_KEY_SLOT_PUBLIC_A;
    rsaLoadPublicKeySettings.publicModulus = RsaPublic1024;
    rsaLoadPublicKeySettings.publicModulusLen = RSA_PUBLIC_1024_LEN;
    rsaLoadPublicKeySettings.sigKeySlot = RSA_KEY_SLOT_PUBLIC_B;
    rsaLoadPublicKeySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePss;
    rsaLoadPublicKeySettings.sigDigestType = SecureRsaTl_DigestType_eSha256;
    rsaLoadPublicKeySettings.sigPssSaltLen = SALT_LEN_16;
    rsaLoadPublicKeySettings.signature = Sig_Public_3072_1024_PssSha256Salt16;
    rsaLoadPublicKeySettings.signatureLen = SIG_PUBLIC_3072_1024_PSS_SHA256_SALT16_LEN;

    if(SecureRsaTl_RsaLoadPublicKey(hSecureRsaTl, &rsaLoadPublicKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Load RSA public key failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test load RSA verify key, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test load RSA verify key, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaLoadPublicKeySettings(&rsaLoadPublicKeySettings);
    rsaLoadPublicKeySettings.rsaKeySlot = RSA_KEY_SLOT_VERIFY;
    rsaLoadPublicKeySettings.publicModulus = RsaVerifyKey;
    rsaLoadPublicKeySettings.publicModulusLen = RSA_VERIFY_KEY_LEN;
    rsaLoadPublicKeySettings.sigKeySlot = RSA_KEY_SLOT_PUBLIC_A;
    rsaLoadPublicKeySettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    rsaLoadPublicKeySettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    rsaLoadPublicKeySettings.signature = Sig_Public_1024_Verify_PkcsSha1;
    rsaLoadPublicKeySettings.signatureLen = SIG_PUBLIC_1024_VERIFY_PKCS_SHA1_LEN;

    if(SecureRsaTl_RsaLoadPublicKey(hSecureRsaTl, &rsaLoadPublicKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Load RSA public key failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test KPK decrypt renewable RSA with AES-ECB encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test KPK decrypt renewable RSA with AES-ECB encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultKpkDecryptRsaSettings(&kpkDecryptRsaSettings);
    kpkDecryptRsaSettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    kpkDecryptRsaSettings.encKeySlot = KPK_KEY_SLOT_A;
    kpkDecryptRsaSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eEcb;
    kpkDecryptRsaSettings.encKeySettings = KeySettings_RenewableRsaA;
    kpkDecryptRsaSettings.encKeySettingsLen = KEY_SETTINGS_RENEWABLE_RSA_A_LEN;
    kpkDecryptRsaSettings.encKey = Enc_RenewableRsaA;
    kpkDecryptRsaSettings.encKeyLen = ENC_RENEWABLE_RSA_A_LEN;
    kpkDecryptRsaSettings.encIv = NULL;
    kpkDecryptRsaSettings.encIvLen = 0;
    kpkDecryptRsaSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    kpkDecryptRsaSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    kpkDecryptRsaSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    kpkDecryptRsaSettings.signature = Sig_RenewableRsaA_PkcsSha1;
    kpkDecryptRsaSettings.signatureLen = SIG_RENEWABLE_RSA_A_PKCS_SHA1_LEN;

    if(SecureRsaTl_KpkDecryptRsa(hSecureRsaTl, &kpkDecryptRsaSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - KPK Decrypt RSA failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test KPK decrypt renewable RSA with AES-CBC encryption, unsigned */
    BDBG_LOG(("%s - Test KPK decrypt renewable RSA with AES-CBC encryption, unsigned", __FUNCTION__));
    SecureRsaTl_GetDefaultKpkDecryptRsaSettings(&kpkDecryptRsaSettings);
    kpkDecryptRsaSettings.rsaKeySlot = RSA_KEY_SLOT_DECRYPT_IKR;
    kpkDecryptRsaSettings.encKeySlot = KPK_KEY_SLOT_B;
    kpkDecryptRsaSettings.variant = BSAGElib_Crypto_AlgorithmVariant_eCbc;
    kpkDecryptRsaSettings.encKeySettings = KeySettings_RenewableRsaB;
    kpkDecryptRsaSettings.encKeySettingsLen = KEY_SETTINGS_RENEWABLE_RSA_B_LEN;
    kpkDecryptRsaSettings.encKey = Enc_RenewableRsaB;
    kpkDecryptRsaSettings.encKeyLen = ENC_RENEWABLE_RSA_B_LEN;
    kpkDecryptRsaSettings.encIv = Iv_RenewableRsaB;
    kpkDecryptRsaSettings.encIvLen = IV_RENEWABLE_RSA_B_LEN;
    kpkDecryptRsaSettings.signature = NULL;
    kpkDecryptRsaSettings.signatureLen = 0;

    if(SecureRsaTl_KpkDecryptRsa(hSecureRsaTl, &kpkDecryptRsaSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - KPK Decrypt RSA failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test renewable RSA host usage with a private key */
    BDBG_LOG(("%s - Test renewable RSA host usage with a private key", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaHostUsageSettings(&rsaHostUsageSettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaHostUsageSettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaHostUsageSettings.rsaKeyType = SecureRsaTl_RsaKeyType_ePrivate;
    rsaHostUsageSettings.input = TestData;
    rsaHostUsageSettings.inputLen = RSA_KEY_LEN;
    rsaHostUsageSettings.output = RsaBuffer;
    rsaHostUsageSettings.outputLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaHostUsage(hSecureRsaTl, &rsaHostUsageSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, HostUsage_PrivateC, RsaBufferLen) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Host usage output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Host usage error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test renewable RSA host usage with a private/public Key */
    BDBG_LOG(("%s - Test renewable RSA host usage with a private/public Key", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaHostUsageSettings(&rsaHostUsageSettings);
    RsaBufferLen = RSA_KEY_LEN;
    rsaHostUsageSettings.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    rsaHostUsageSettings.rsaKeyType = SecureRsaTl_RsaKeyType_ePublic;
    rsaHostUsageSettings.input = TestData;
    rsaHostUsageSettings.inputLen = RSA_KEY_LEN;
    rsaHostUsageSettings.output = RsaBuffer;
    rsaHostUsageSettings.outputLen = &RsaBufferLen;
    BKNI_Memset(RsaBuffer, 0, RsaBufferLen);

    if(SecureRsaTl_RsaHostUsage(hSecureRsaTl, &rsaHostUsageSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(RsaBuffer, HostUsage_PublicC, RSA_KEY_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - Host usage output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Host usage error", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test renewable RSA decrypt Key3 IKR128 with PKCS encryption, PKCS signature with SHA-1 digest */
    BDBG_LOG(("%s - Test renewable RSA Decrypt Key3 IKR128 with PKCS encryption, PKCS signature with SHA-1 digest", __FUNCTION__));
    SecureRsaTl_GetDefaultRsaDecryptKey3Settings(&decryptAesSettings);
    decryptAesSettings.encKeySlot = RSA_KEY_SLOT_DECRYPT_IKR;
    decryptAesSettings.encPadType = SecureRsaTl_RsaEncryptionPadding_ePkcs;
    decryptAesSettings.encKeyLen = ENC_RENEWABLE_IKR128_PKCS_LEN;
    decryptAesSettings.encKey = Enc_Renewable_Ikr128_Pkcs;
    decryptAesSettings.encKeySettingsLen = KEY_SETTINGS_IKR_LEN;
    decryptAesSettings.encKeySettings = KeySettings_Ikr;
    decryptAesSettings.sigPadType = SecureRsaTl_RsaSignaturePadding_ePkcs;
    decryptAesSettings.sigDigestType = SecureRsaTl_DigestType_eSha1;
    decryptAesSettings.sigKeySlot = RSA_KEY_SLOT_VERIFY;
    decryptAesSettings.signature = Sig_Renewable_Ikr128_PkcsSha1;
    decryptAesSettings.signatureLen = SIG_RENEWABLE_IKR128_PKCS_SHA1_LEN;
    decryptAesSettings.aesKeyLen = KEY_LEN_128;
    decryptAesSettings.aesKeySlot = KEY3_KEY_SLOT_IKR_A;

    if(SecureRsaTl_RsaDecryptKey3(hSecureRsaTl, &decryptAesSettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - RSA Decrypt Key3 failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test 128-bit Key3 HMAC */
    BDBG_LOG(("%s - Test 128-bit Key3 HMAC", __FUNCTION__));
    SecureRsaTl_GetDefaultKey3CalculateHmacSettings(&key3CalculateHmacSettings);
    HmacBufferLen = HMAC_SHA1_EXPECTED_LEN;
    key3CalculateHmacSettings.inputData = DataAbc;
    key3CalculateHmacSettings.inputDataLen = DATA_ABC_LEN;
    key3CalculateHmacSettings.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    key3CalculateHmacSettings.hmacType = SecureRsaTl_HmacType_eSha1;
    key3CalculateHmacSettings.hmac = HmacBuffer;
    key3CalculateHmacSettings.hmacLen = &HmacBufferLen;
    BKNI_Memset(HmacBuffer, 0, HmacBufferLen);

    if(SecureRsaTl_Key3CalculateHmac(hSecureRsaTl, &key3CalculateHmacSettings) == BERR_SUCCESS)
    {
        if(BKNI_Memcmp(HmacBuffer, HmacSha1_Ikr128, HMAC_SHA1_IKR128_LEN) == 0)
        {
            BDBG_LOG(("%s -    Pass", __FUNCTION__));
        }
        else
        {
            BDBG_ERR(("%s -    FAIL - HMAC output mismatch", __FUNCTION__));
            rc = BERR_UNKNOWN;
        }
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - HMAC calculation failed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Check the number of keys */
    BDBG_LOG(("%s - Check the number of keys", __FUNCTION__));
    if((ShowSecureRsaContext(hSecureRsaTl, &rsaNumKeys, &key3NumKeys,
                             &kpkNumKeys) == BERR_SUCCESS) &&
       (rsaNumKeys == EXPECTED_KEYS_TEST3_RSA) &&
       (key3NumKeys == EXPECTED_KEYS_TEST3_KEY3) &&
       (kpkNumKeys == EXPECTED_KEYS_TEST3_KPK))
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Incorrect number of keys", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test remove keys */
    BDBG_LOG(("%s - Test remove keys", __FUNCTION__));
    SecureRsaTl_GetDefaultRemoveKeySettings(&removeKeySettings);

    removeKeySettings.keyType = SecureRsaTl_KeyType_eRsa;
    removeKeySettings.keyslot.rsaKeySlot = RSA_KEY_SLOT_SIGN_HOST_USAGE;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

    removeKeySettings.keyType = SecureRsaTl_KeyType_eRsa;
    removeKeySettings.keyslot.rsaKeySlot = RSA_KEY_SLOT_VERIFY;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

    removeKeySettings.keyType = SecureRsaTl_KeyType_eKey3;
    removeKeySettings.keyslot.key3KeySlot = KEY3_KEY_SLOT_IKR_A;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

    removeKeySettings.keyType = SecureRsaTl_KeyType_eKey3;
    removeKeySettings.keyslot.key3KeySlot = KEY3_KEY_SLOT_IKR_B;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

    removeKeySettings.keyType = SecureRsaTl_KeyType_eKey3;
    removeKeySettings.keyslot.key3KeySlot = KEY3_KEY_SLOT_CA_B;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

    removeKeySettings.keyType = SecureRsaTl_KeyType_eKey3;
    removeKeySettings.keyslot.key3KeySlot = KEY3_KEY_SLOT_CP_A;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

    removeKeySettings.keyType = SecureRsaTl_KeyType_eKpk;
    removeKeySettings.keyslot.kpkKeySlot = KPK_KEY_SLOT_A;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Key not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Check the number of keys */
    BDBG_LOG(("%s - Check the number of keys", __FUNCTION__));
    if((ShowSecureRsaContext(hSecureRsaTl, &rsaNumKeys, &key3NumKeys,
                             &kpkNumKeys) == BERR_SUCCESS) &&
       (rsaNumKeys == EXPECTED_KEYS_TEST4_RSA) &&
       (key3NumKeys == EXPECTED_KEYS_TEST4_KEY3) &&
       (kpkNumKeys == EXPECTED_KEYS_TEST4_KPK))
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Incorrect number of keys", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Test remove all keys */
    BDBG_LOG(("%s - Test remove all keys", __FUNCTION__));
    removeKeySettings.keyType = SecureRsaTl_KeyType_eAll;
    if(SecureRsaTl_RemoveKey(hSecureRsaTl, &removeKeySettings) == BERR_SUCCESS)
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Keys not removed", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }


    /* Check the number of keys */
    BDBG_LOG(("%s - Check the number of keys", __FUNCTION__));
    if((ShowSecureRsaContext(hSecureRsaTl, &rsaNumKeys, &key3NumKeys,
                             &kpkNumKeys) == BERR_SUCCESS) &&
       (rsaNumKeys == EXPECTED_KEYS_TEST5_RSA) &&
       (key3NumKeys == EXPECTED_KEYS_TEST5_KEY3) &&
       (kpkNumKeys == EXPECTED_KEYS_TEST5_KPK))
    {
        BDBG_LOG(("%s -    Pass", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s -    FAIL - Incorrect number of keys", __FUNCTION__));
        rc = BERR_UNKNOWN;
    }

ErrorExit:
    if(hSecureRsaTl != NULL)
    {
        SecureRsaTl_Uninit(hSecureRsaTl);
    }

    if(KeySlotHandleCp != NULL)
    {
        FreeKeyslot(KeySlotHandleCp);
    }

    if(KeySlotHandleCa1 != NULL)
    {
        FreeKeyslot(KeySlotHandleCa1);
    }

    if(KeySlotHandleCa2 != NULL)
    {
        FreeKeyslot(KeySlotHandleCa2);
    }

    return rc;
}


static int SAGE_app_join_nexus(void)
{
    int rc = 0;
    NEXUS_PlatformSettings platformSettings;

    BDBG_LOG(("\t*** Bringing up all Nexus modules for platform using default settings\n\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    if(NEXUS_Platform_Init(&platformSettings) == NEXUS_SUCCESS){
        BDBG_LOG(("\t*** Nexus has initialized successfully\n\n"));
        rc = 0;
    }
    else{
        BDBG_ERR(("\t### Failed to bring up Nexus\n\n"));
        rc = 1;
    }
    return rc;
}


static void SAGE_app_leave_nexus(void)
{
    NEXUS_Platform_Uninit();
}


/* main file
 * This platform can run in a standalone application or
 * be a part of a larger one.
 */
int main(int argc, char* argv[])
{
    BERR_Code rc = BERR_UNKNOWN;


    BDBG_LOG(("%s - \n\n\n\n\n", __FUNCTION__));
    BDBG_LOG(("%s - ***************************************************************", __FUNCTION__));
    BDBG_LOG(("%s - ***************************************************************", __FUNCTION__));
    BDBG_LOG(("%s - ***Broadcom Limited Secure RSA test utility (Copyright 2018)", __FUNCTION__));
    BDBG_LOG(("%s - ***************************************************************", __FUNCTION__));
    BDBG_LOG(("%s - ***************************************************************\n", __FUNCTION__));


    rc = SAGE_app_join_nexus();
    if(rc != 0){
        goto ErrorExit;
    }

#if defined(SECURE_RSA_TEST_1024) || defined(SECURE_RSA_TEST_2048) || defined(SECURE_RSA_TEST_3072)
    if(argc != 2)
    {
        BDBG_ERR(("%s - Invalid number of command line arguments (%u)\n",
                  __FUNCTION__, argc));
        print_help();
        rc = -1;
        goto ErrorExit;
    }

    if(strlen(argv[1]) >= MAX_ARG_STRING_LENGTH)
    {
        BDBG_ERR(("%s - String length of argument '%s' is too large (%u bytes)\n",
                  __FUNCTION__, argv[1], strlen(argv[1])));
        rc = -1;
        goto ErrorExit;
    }

    strcpy(DrmBinfilePath, argv[1]);
    rc = TestSecureRsa();
#else
#error Invalid test
#endif

ErrorExit:
    if(rc)
    {
        BDBG_ERR(("%s - Failure in SAGE Utility Secure RSA test", __FUNCTION__));
    }

    /* Leave Nexus: Finalize platform ... */
    BDBG_LOG(("%s - Closing Nexus driver...", __FUNCTION__));
    SAGE_app_leave_nexus();

    return rc;
}


static void print_help(void)
{
#if defined(SECURE_RSA_TEST_1024) || defined(SECURE_RSA_TEST_2048) || defined(SECURE_RSA_TEST_3072)
    BDBG_LOG(("%s - ./nexus brcm_utility_securersa_tool_%d <DRM bin file>\n",
              __FUNCTION__, 8*RSA_KEY_LEN));
#else
#error Invalid test
#endif
    return;
}
