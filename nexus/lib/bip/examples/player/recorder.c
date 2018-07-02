/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include "nxclient.h"
#include "nexus_platform_client.h"

#include "bip.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "bmedia_probe.h"
#include "cmd_parsing.h"

BDBG_MODULE(recorder);

bool g_quit = false;
void sigHandler(int signalNum)
{
    g_quit = true;
    printf("Got Signal=%d, quitting (g_quit=%d) app...\n", signalNum, g_quit);
}

static FILE *fout = NULL;
static void recordData(void *context, int param)
{
    NEXUS_Error nrc;
    NEXUS_RecpumpHandle hRecpump = context;
    NEXUS_RecpumpStatus status;
    const void *pDataBuffer;
    size_t dataBufferLength;
    static uint64_t totalRead = 0;
    static unsigned count = 0;
    size_t written;

    if (param==2)
    {
        BDBG_MSG(("%s: param=%d!!", BSTD_FUNCTION, param));
        BDBG_MSG(("%s: Recpump Overflow!!", BSTD_FUNCTION));
    }

    while (true)
    {
        nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, &pDataBuffer, &dataBufferLength);
        BDBG_ASSERT(!nrc);

        nrc = NEXUS_Recpump_GetStatus(hRecpump, &status);
        BDBG_ASSERT(!nrc);
        BDBG_MSG(("depth=%u size=%u", status.data.fifoDepth, status.data.fifoSize));
        if (dataBufferLength == 0)
        {
            BDBG_MSG(("No data available to read, re-wait for the callback!"));
            return;
        }

        /* Save the data. */
        {
            if (fout)
            {
                written = fwrite(pDataBuffer, 1, dataBufferLength, fout);
                if (written != dataBufferLength)
                {
                    BDBG_WRN(("written=%u dataBufferLength=%u, ferror=%d, errno=%d", written, dataBufferLength, ferror(fout), errno));
                    exit(0);
                }
                fflush(fout);
            }

            nrc = NEXUS_Recpump_DataReadComplete(hRecpump, dataBufferLength);
            BDBG_ASSERT(!nrc);
            totalRead += dataBufferLength;
            if (count++%10 == 0) BDBG_WRN(("[%d]: dumped/wrote %u total=%llu bytes", getpid(), dataBufferLength, totalRead));
        }
    }
}

static void dataReadyCallback(void *context, int param)
{
    NEXUS_RecpumpHandle hRecpump = context;

    /* Doesn't do much. */
    BSTD_UNUSED(hRecpump);
    BDBG_MSG(("%s: param=%d!!", BSTD_FUNCTION, param));
}

static void playbackDoneCallbackFromBIP(
    void *context,
    int   param
    )
{
    AppCtx *pAppCtx = context;

    BSTD_UNUSED( param );
    BDBG_MSG(( BIP_MSG_PRE_FMT " Settings hEvent=%p)" BIP_MSG_PRE_ARG, pAppCtx->hCompletionEvent ));
    pAppCtx->playbackDone = true;
    B_Event_Set(pAppCtx->hCompletionEvent);
}

int main(int argc, char *argv[])
{
    AppCtx *pAppCtx;
    CmdOptions cmdOptions;
    BIP_Status bipStatus;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_PidChannelHandle          hAllPassPidChannel = NULL;
    BIP_PlayerHandle hPlayer = NULL;
    NEXUS_RecpumpHandle hRecpump = NULL;
#if 0
    NEXUS_RecordHandle hRecord = NULL;
    NEXUS_FileRecordHandle hFile;
    NEXUS_RecordSettings recordSettings;
#endif
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    BKNI_EventHandle hStateChangedEvent;

    /* Initialize BIP */
    {
        bipStatus = BIP_Init(NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Init Failed" ), errorBipInit, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        signal(SIGINT, sigHandler);
    }

    /* Allocate & initialize App Ctx */
    {
        pAppCtx = initAppCtx();
        BIP_CHECK_GOTO(( pAppCtx ), ( "initAppCtx Failed" ), errorBipInit, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        /* Parse commandline options */
        bipStatus = parseOptions( argc, argv, pAppCtx );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "parseOptions Failed" ), errorParseOptions, bipStatus, bipStatus );

        pAppCtx->hCompletionEvent = B_Event_Create(NULL);
        BIP_CHECK_GOTO(( pAppCtx->hCompletionEvent ), ( "B_Event_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
    }

    /* Initialize Nexus */
    {
        NxClient_JoinSettings joinSettings;

        NxClient_GetDefaultJoinSettings(&joinSettings);
        snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
        rc = NxClient_Join(&joinSettings);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Join Failed" ), error, BIP_ERR_INTERNAL, bipStatus );
    }

    /* Initialize DtcpIpClientFactory */
    {
        bipStatus = BIP_DtcpIpClientFactory_Init(NULL);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS || bipStatus == BIP_ERR_NOT_SUPPORTED), ( "BIP_DtcpIpClientFactory_Init Failed" ), errorBipInit, bipStatus, bipStatus );
    }

    if (BIP_String_GetLength(pAppCtx->hRecordFile))
    {
        fout=fopen(BIP_String_GetString(pAppCtx->hRecordFile), "w");
        BDBG_ASSERT(fout);
    }

    {
        NxClient_AllocSettings      allocSettings;
        NxClient_AllocResults       allocResults;
        NxClient_ConnectSettings    connectSettings;

        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.simpleVideoDecoder = 0;
        allocSettings.simpleAudioDecoder = 0;
        rc = NxClient_Alloc( &allocSettings, &allocResults );
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Alloc Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        NxClient_GetDefaultConnectSettings( &connectSettings );
        rc = NxClient_Connect(&connectSettings, &pAppCtx->connectId);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NxClient_Connect Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        /* We have successfully acquired all needed resources. */
    }

    /* Create BIP Player instance */
    {
        hPlayer = BIP_Player_Create( NULL );
        BIP_CHECK_GOTO(( hPlayer ), ( "BIP_Player_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        BKNI_CreateEvent(&hStateChangedEvent);
        BDBG_ASSERT(hStateChangedEvent);
    }

    /* Connect to Server & do HTTP Request/Response exchange. */
    {
        BIP_PlayerConnectSettings settings;
        const char *responseHeaders = NULL;

        BIP_Player_GetDefaultConnectSettings(&settings);
        settings.pUserAgent = "BIP Recorder Example";
        settings.deferServerConnection = true;
        bipStatus = BIP_Player_Connect( hPlayer, BIP_String_GetString(pAppCtx->hUrl), &settings );
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Connect Failed to Connect to URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
        BDBG_MSG(("http response headers >>> \n%s", responseHeaders));
    }

    /* Prepare the BIP Player using the default settings. */
    {
        BIP_PlayerPrepareSettings   prepareSettings;
        BIP_PlayerSettings          playerSettings;
        BIP_PlayerPrepareStatus     prepareStatus;

        BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
        BIP_Player_GetDefaultSettings(&playerSettings);
        playerSettings.playbackSettings.endOfStreamCallback.callback = playbackDoneCallbackFromBIP;
        playerSettings.playbackSettings.endOfStreamCallback.context = pAppCtx;
        playerSettings.playbackSettings.errorCallback.callback = playbackDoneCallbackFromBIP;
        playerSettings.playbackSettings.errorCallback.context = pAppCtx;
        playerSettings.playbackSettings.playpumpSettings.allPass = true;
        playerSettings.playbackSettings.playpumpSettings.acceptNullPackets = true;
        playerSettings.playbackSettings.playpumpSettings.timestamp.forceRestamping = pAppCtx->enableTransportTimestamps;
        if (pAppCtx->enableContinousPlay)
        {
            playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
        }
        else
        {
            playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        }
        playerSettings.playbackSettings.playpumpSettings.allPass = true;
        prepareSettings.enableHwOffload = pAppCtx->enableHwOffload;
        bipStatus = BIP_Player_Prepare(hPlayer, &prepareSettings, &playerSettings, NULL/*&probeSettings*/, NULL/*&streamInfo*/, &prepareStatus);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Prepare Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
    }

    /* Open All Pass PidChannel to allow receiving all of the PIDs in the stream. */
    {
        BIP_PlayerOpenPidChannelSettings  settings;

        BIP_Player_GetDefaultOpenPidChannelSettings(&settings);
        bipStatus = BIP_Player_OpenPidChannel(hPlayer, 0/* NA for allPass case*/, &settings, &hAllPassPidChannel);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_OpenPidChannel Failed to open allPass Pid channel" ), error, bipStatus, bipStatus );
        BDBG_MSG(("Allpass Pid channel is opened!"));
    }

    /* Setup the recording context. */
    {
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        recpumpOpenSettings.data.bufferSize = recpumpOpenSettings.data.bufferSize * 4;
        recpumpOpenSettings.data.dataReadyThreshold = 16384;
        hRecpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BDBG_ASSERT(hRecpump);

        NEXUS_Recpump_GetSettings(hRecpump, &recpumpSettings);
        recpumpSettings.data.dataReady.callback = dataReadyCallback;
        recpumpSettings.data.dataReady.context = hRecpump;
        recpumpSettings.data.dataReady.param = 0;
        recpumpSettings.data.overflow.callback = dataReadyCallback;
        recpumpSettings.data.overflow.context = hRecpump;
        recpumpSettings.data.dataReady.param = 1;
        recpumpSettings.bandHold = true;
        if (pAppCtx->enableTransportTimestamps)
        {
            recpumpSettings.timestampType = NEXUS_TransportTimestampType_e32_Binary;
        }
        rc = NEXUS_Recpump_SetSettings(hRecpump, &recpumpSettings);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);

        rc = NEXUS_Recpump_AddPidChannel(hRecpump, hAllPassPidChannel, NULL);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);

        rc = NEXUS_Recpump_Start(hRecpump);
        BDBG_ASSERT(rc == NEXUS_SUCCESS);
        BDBG_MSG(("Recpump is opened & started!"));
    }
#if 0
    {
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        /* set threshold to 80%. with band hold enabled, it's not actually a dataready threshold. it's
           a bandhold threshold. we are relying on the timer that's already in record. */
#if 0
        recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize * 8 / 10;
#endif
        hRecpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BIP_CHECK_GOTO(( hRecpump ), ( "NEXUS_Recpump_Open Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );
        hRecord = NEXUS_Record_Create();
        BIP_CHECK_GOTO(( hRecord ), ( "NEXUS_Record_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        NEXUS_Record_GetSettings(hRecord, &recordSettings);
        recordSettings.recpump = hRecpump;
        /* enable bandhold. required for record from playback. */
        recordSettings.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
        rc = NEXUS_Record_SetSettings(hRecord, &recordSettings);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_Record_SetSettings Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        hFile = NEXUS_FileRecord_OpenPosix( BIP_String_GetString(pAppCtx->hRecordFile), NULL );
        BIP_CHECK_GOTO(( hFile ), ( "NEXUS_FileRecord_OpenPosix Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus );

        rc = NEXUS_Record_AddPidChannel(hRecord, hPidChannel, NULL);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_Record_AddPidChannel Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );

        rc = NEXUS_Record_Start(hRecord, hFile);
        BIP_CHECK_GOTO(( rc == NEXUS_SUCCESS ), ( "NEXUS_Record_Start Failed" ), error, BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE, bipStatus );
    }
#endif

    /* NEXUS Setup is complete, now Prepare & start BIP_Player. */
    {
        BIP_PlayerStartSettings startSettings;

        BIP_Player_GetDefaultStartSettings(&startSettings);
        bipStatus = BIP_Player_Start(hPlayer, &startSettings);
        BIP_CHECK_GOTO(( bipStatus == BIP_SUCCESS ), ( "BIP_Player_Start Failed: URL=%s", BIP_String_GetString(pAppCtx->hUrl) ), error, bipStatus, bipStatus );
        BDBG_MSG(("BIP Player is started!"));
    }

    while (g_quit == false)
    {
        unsigned timeout = 100; /* msec */
        static unsigned count = 0;

        BDBG_MSG(("In recording loop!"));
        if (pAppCtx->printStatus && count++ % 10 == 0)
        {
            BIP_Player_PrintStatus(hPlayer);
        }
        recordData(hRecpump, 1);
        BKNI_WaitForEvent(hStateChangedEvent, timeout);
        if (pAppCtx->playbackDone)
        {
            BIP_Player_PrintStatus(hPlayer);
            NEXUS_Recpump_StopData(hRecpump);
            recordData(hRecpump, 1);
            break;
        }
    }

    bipStatus = BIP_SUCCESS;
    {
        NEXUS_RecpumpStatus status;
        char *pTmp;

        pTmp = getenv("testMode");
        if (pTmp)
        {
            NEXUS_Recpump_GetStatus(hRecpump, &status);
#if 0
            if (status.data.bytesRecorded != 209715128)
#endif
            if (status.data.bytesRecorded != 940000)
            {
                const void *pDataBuffer;
                size_t dataBufferLength;
                NEXUS_Error nrc;

                nrc = NEXUS_Recpump_GetDataBuffer(hRecpump, &pDataBuffer, &dataBufferLength);
                BDBG_ASSERT(!nrc);

                BDBG_WRN((">>>>>>>>>>>>>> Failed to receive all data bytes, got=%"PRIu64 " depth=%u dataBufferLength=%u",
                            status.data.bytesRecorded,
                            status.data.fifoDepth, dataBufferLength
                            ));
                pause();
            }
            else
            {
                BDBG_WRN((">>>>>>>>>>>>>> Received all data bytes=%"PRIu64 " depth=%u",
                            status.data.bytesRecorded,
                            status.data.fifoDepth
                            ));
            }
        }
    }
error:
    /* Stop the Player & Decoders. */
    {
        BIP_Player_Stop(hPlayer);
#if 0
        NEXUS_Record_Stop(hRecord);
        NEXUS_Record_RemoveAllPidChannels(hRecord);
#endif
        NEXUS_Recpump_Stop(hRecpump);
        NEXUS_Recpump_RemoveAllPidChannels(hRecpump);
    }

    /* Close the Resources. */
    {
        if (hStateChangedEvent) BKNI_DestroyEvent(hStateChangedEvent);
        if (hPlayer) BIP_Player_Disconnect(hPlayer);
#if 0
        NEXUS_FileRecord_Close(hFile);
        NEXUS_Record_Destroy(hRecord);
#endif
        NEXUS_Recpump_Close(hRecpump);
        if (pAppCtx->hCompletionEvent) B_Event_Destroy(pAppCtx->hCompletionEvent);
        if (hPlayer) BIP_Player_Destroy(hPlayer);
errorParseOptions:
        unInitAppCtx( pAppCtx );
        BIP_DtcpIpClientFactory_Uninit();
        NxClient_Uninit();
    }
errorBipInit:
    BIP_Uninit();
    return (0);
}
