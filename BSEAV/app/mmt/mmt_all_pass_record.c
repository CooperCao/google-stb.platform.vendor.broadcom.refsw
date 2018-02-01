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

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_core_utils.h"


#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

BDBG_MODULE(mmt_all_pass_record);

/**
 *  mmt_all_pass_record app records TLV-MMT stream in MPEG2
 *  style 188 bytes size packets.
 *
 *  DekTek DTU-215 streamer was used to stream a 38.8Mpbs
 *  MPEG2TS stream. In the streamXpress app for DTU-215
 *  modulator: 1. rmx should be turned off 2. Modulation
 *  parameters should be QAM-C and QAM-256
 *
 *  Output of this app can be tested using mmt_playback app.
 *
 *  brcm_mmt_1pgm_36mbps_hevc_aac.v2.ts is stream used for
 *  testing live. Customers can request this stream from
 *  Broadcom for testing
 **/

#define MAX_ACQUIRE_TIME 20000
typedef struct recordContext
{
    NEXUS_RecpumpHandle recpump;
    FILE *file;
    bool started;
    BKNI_EventHandle recEvent;
}recordContext;

static void * record_thread(void *context)
{
    const void *data_buffer;
    size_t data_buffer_size;
    int n;
    NEXUS_Error rc;
    recordContext *recCtx = (recordContext *) context;
    NEXUS_RecpumpStatus status;
    while (recCtx->started)
    {

        NEXUS_Recpump_GetStatus(recCtx->recpump,&status);
        if (status.started)
        {
            rc = NEXUS_Recpump_GetDataBuffer(recCtx->recpump, &data_buffer, &data_buffer_size);
            BDBG_ASSERT(!rc);
        }
        else
        {
            BKNI_Sleep(100);
            continue;
        }

        if (data_buffer_size== 0)
        {
            if (recCtx->started)
                BKNI_WaitForEvent(recCtx->recEvent, 10);
            fflush(recCtx->file);
            continue;
        }
        n = fwrite(data_buffer, 1, data_buffer_size, recCtx->file);
        BDBG_ASSERT(n== (int)data_buffer_size);
        rc = NEXUS_Recpump_DataReadComplete(recCtx->recpump, n);
        BDBG_ASSERT(!rc);
    }
    return NULL;
}
static void recpump_dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    return;
}

static void recpump_overflow_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BDBG_ERR(("Recpump overflow!"));
    return;
}


static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void cppm_complete_callback(void *context, int param)
{
    BSTD_UNUSED(param);

    BDBG_WRN(("cppm_complete_callback"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void cppm_callback(void *context, int param)
{
    NEXUS_Error rc;
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle) context;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceRecalibrateSettings calibrateSettings;
    BSTD_UNUSED(param);

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        BDBG_ERR(("Unable to retrieve frontend device handle.\n"));
    }

    NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(&calibrateSettings);
    calibrateSettings.cppm.enabled = true;
    calibrateSettings.cppm.threshold = 250;
    calibrateSettings.cppm.thresholdHysteresis = 50;
    calibrateSettings.cppm.powerLevelChange.callback = cppm_callback;
    calibrateSettings.cppm.powerLevelChange.context = (void *) frontend;
    rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
    if(rc) {
        BDBG_ERR(("NEXUS_FrontendDevice_Recalibrate exited with rc = %d", rc));
    }
}

static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static void print_usage(void)
{
    printf(
        "Usage: tune_ofdm\n"
        "  --help or -h for help\n"
        "  -freq   #   frequency in MHz\n"
        "  -mode   #   64 = QAM-64, 256 = QAM-256 (default is QAM-64)\n"
        "  -file   #   record file name \n"
        );
}



static NEXUS_Error get_status(NEXUS_FrontendHandle frontend, BKNI_EventHandle statusEvent)
{
    NEXUS_FrontendQamStatus qamStatus;
    int rc;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceStatus deviceStatus;

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        fprintf(stderr, "Unable to retrieve frontend device handle.\n");
    }

    rc = NEXUS_Frontend_RequestQamAsyncStatus(frontend);
    if(rc == NEXUS_SUCCESS){
        rc = BKNI_WaitForEvent(statusEvent, 1000);
        if (rc) {
            printf("Status not returned\n");
            return BERR_TRACE(rc);
        }
        NEXUS_Frontend_GetQamAsyncStatus(frontend , &qamStatus);

        if (deviceHandle) {
            rc = NEXUS_FrontendDevice_GetStatus(deviceHandle, &deviceStatus);
            if(rc) return BERR_TRACE(rc);
        }
    }
    else if(rc == NEXUS_NOT_SUPPORTED){
        rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        return BERR_TRACE(rc);
    }

    printf("\nDownstream lock = %d\n", qamStatus.fecLock);
    printf("Frequency = %d\n", qamStatus.settings.frequency);
    if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e64)
        printf("Mode = NEXUS_FrontendQamMode_e64\n");
    else if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e256)
        printf("Mode  = NEXUS_FrontendQamMode_e256\n");
    else
        printf("Mode = %d\n", qamStatus.settings.mode);
    if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eA)
        printf("Annex = NEXUS_FrontendQamAnnex_eA\n");
    else if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eB)
        printf("Annex  = NEXUS_FrontendQamAnnex_eB\n");
    else
        printf("Annex = %d\n", qamStatus.settings.annex);
    printf("Symbol rate = %d\n", qamStatus.symbolRate);
    printf("Snr estimate = %d\n", qamStatus.snrEstimate/100 );
    printf("FecCorrected = %d\n", qamStatus.fecCorrected);
    printf("FecUncorrected = %d\n", qamStatus.fecUncorrected);
    printf("DsChannelPower in dBmV = %d\n", qamStatus.dsChannelPower/10);
    printf("DsChannelPower in dBm = %d\n", qamStatus.dsChannelPower/10 - 48);
    if (deviceHandle) {
        printf("AVS enabled = %d\n", deviceStatus.avs.enabled);
        printf("AVS voltage = %d\n", deviceStatus.avs.voltage);
        printf("Device temperature = %d\n", deviceStatus.temperature);
    }
    return 0;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceRecalibrateSettings calibrateSettings;
    NEXUS_Error rc;
    unsigned freq = 576000000;
    BKNI_EventHandle statusEvent, lockChangedEvent, cppmEvent;
    bool done = false;
    bool cppm = false;
    bool waitForCppm = false;
    int curarg = 1;
    unsigned  mode = 256;
    unsigned maxAcquireTime = MAX_ACQUIRE_TIME;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_PidChannelSettings pidCfg;
    NEXUS_PidChannelHandle allPassPidChannel;
    pthread_t recordThread;
    recordContext recCtx;
    char fileName[512] = "live_mmt.ts";

    recCtx.file = NULL;
    recCtx.recpump = NULL;
    recCtx.started = false;

    /**
     *  read command line parameters
     **/
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-mode")) {
            mode = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-file")) {
            strcpy(fileName,argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            float f;
            if (sscanf(argv[++curarg], "%f", &f) != 1) f = 0;
            if (f < 1000000)
                freq = (unsigned)(f*1000) * 1000;
            else
                freq = (unsigned)f;
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    /**
     *  initialized nexus platform
     **/
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = 50 * 1000 * 1000;
    platformSettings.videoDecoderModuleSettings.deferInit = true;
#if NEXUS_HAS_VIDEO_ENCODER
    platformSettings.videoEncoderSettings.deferInit = true;
#endif
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    /**
     *  initialize the tuners and demods
    **/
    rc = NEXUS_Platform_InitFrontend();
    if(rc){rc = BERR_TRACE(rc); goto done;}
    {
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.qam = true;
        frontend = NEXUS_Frontend_Acquire(&settings);
        if (!frontend) {
            BDBG_ERR(("Unable to find QAM-capable frontend"));
            return -1;
        }
    }

    BKNI_CreateEvent(&statusEvent);
    BKNI_CreateEvent(&lockChangedEvent);
    BKNI_CreateEvent(&cppmEvent);
    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        fprintf(stderr, "Unable to retrieve frontend device handle.\n");
    }

    NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(&calibrateSettings);
    calibrateSettings.cppm.enabled = false;
    calibrateSettings.cppm.threshold = 260;
    calibrateSettings.cppm.thresholdHysteresis = 50;
    calibrateSettings.cppm.powerLevelChange.callback = cppm_callback;
    calibrateSettings.cppm.powerLevelChange.context = (void *) frontend;
    calibrateSettings.cppm.calibrationComplete.callback = cppm_complete_callback;
    calibrateSettings.cppm.calibrationComplete.context = cppmEvent;
    rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);

    while (!done) {
        bool acquired = false;
        bool firstTune = true;

        /**
         *  intialize the front end parameters
         **/
        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = freq;
        switch (mode) {
        default:
            BDBG_ERR(("Incorrect mode %d specified. Defaulting to 64(NEXUS_FrontendQamMode_e64)", mode));
        case 64:
            qamSettings.mode = NEXUS_FrontendQamMode_e64;
            qamSettings.symbolRate = 5056900;
            break;
        case 256:
            qamSettings.mode = NEXUS_FrontendQamMode_e256;
            qamSettings.symbolRate = 5360537;
            BDBG_WRN(("mode %d sym %d",qamSettings.mode,qamSettings.symbolRate));
            break;
        }
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.lockCallback.callback = lock_changed_callback;
        qamSettings.lockCallback.context = lockChangedEvent;
        qamSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
        qamSettings.asyncStatusReadyCallback.context = statusEvent;
        qamSettings.autoAcquire = true;
        NEXUS_Frontend_GetUserParameters(frontend, &userParams);
        /**
         *  map a parser band to the demod's input band.
         **/
        parserBand = NEXUS_ParserBand_e0;
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        /**
         * set the parser band parameters so that no packets are dropped
         * because of continuity count errors and missing adaptation
         * field.
         **/
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        parserBandSettings.allPass=true;
        parserBandSettings.teiIgnoreEnabled = true;
        parserBandSettings.acceptAdapt00 = true;
        parserBandSettings.continuityCountEnabled = false;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
        /**
         * open all pass pid channel
         **/
        NEXUS_PidChannel_GetDefaultSettings(&pidCfg);
        NEXUS_ParserBand_GetAllPassPidChannelIndex(parserBand, &pidCfg.pidChannelIndex);
        allPassPidChannel = NEXUS_PidChannel_Open(parserBand, 0, &pidCfg);
        BDBG_ASSERT(allPassPidChannel);
        /**
         * open a recpump
         **/
        BKNI_CreateEvent(&recCtx.recEvent);
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        recpumpOpenSettings.indexType = NEXUS_RecpumpIndexType_eNone;
        recpumpOpenSettings.data.bufferSize = recpumpOpenSettings.data.bufferSize*2;
        recCtx.recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BDBG_ASSERT(recCtx.recpump);
        /**
         * for opened recpump, disable flow control and set the output
         * to bulk type
         **/
        NEXUS_Recpump_GetSettings(recCtx.recpump, &recpumpSettings);
        recpumpSettings.data.dataReady.callback = recpump_dataready_callback;
        recpumpSettings.data.overflow.callback = recpump_overflow_callback;
        recpumpSettings.data.dataReady.context = recCtx.recEvent;
        recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eDisable;
        recpumpSettings.outputTransportType = NEXUS_TransportType_eBulk;
        NEXUS_Recpump_SetSettings(recCtx.recpump, &recpumpSettings);
        /**
         * add all pass pid channel to opened recpump
         **/
        NEXUS_Recpump_AddPidChannel(recCtx.recpump,allPassPidChannel, NULL);
        /**
         * Attempt to tune to the request QAM channel
         **/
        BKNI_ResetEvent(lockChangedEvent);
tune:
        BDBG_WRN(("tuning %d MHz... mode = %d", freq/1000000, qamSettings.mode));
        rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
        if(rc){rc = BERR_TRACE(rc); goto done;}

        {
            unsigned start_time = b_get_time();
            while (1) {
                unsigned current_time = b_get_time();
                NEXUS_FrontendFastStatus status;

                if (current_time - start_time > maxAcquireTime) {
                    BDBG_WRN(("application timeout. cannot acquire."));
                    break;
                }
                rc = BKNI_WaitForEvent(lockChangedEvent, maxAcquireTime - (current_time - start_time));
                if (rc == BERR_TIMEOUT) {
                    BDBG_WRN(("application timeout. cannot acquire."));
                    break;
                }
                BDBG_ASSERT(!rc);

                rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
                if (rc == NEXUS_SUCCESS) {
                    if (status.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
                        BDBG_WRN(("frontend locked"));
                        acquired = true;
                        break;
                    }
                    else if (status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked) {
                        BDBG_WRN(("frontend unlocked (acquireInProgress=%d)", status.acquireInProgress));
                         /* Wait for maxAcquireTime when unlocked*/
                    }
                    else if (status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal) {
                        if(!qamSettings.autoAcquire) {
                            BDBG_WRN(("No signal at the tuned frequency."));
                            break;
                        }
                        else
                            BDBG_WRN(("No signal at the tuned frequency. Waiting till the application timesout to allow auto acquire to try again."));
                    }
                    else {
                        BDBG_ERR(("unknown status: %d", status.lockStatus));
                    }
                }
                else if (rc == NEXUS_NOT_SUPPORTED) {
                    NEXUS_FrontendQamStatus qamStatus;
                    rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
                    if (rc) {rc = BERR_TRACE(rc); return false;}

                    BDBG_WRN(("frontend %s (fecLock=%d)", qamStatus.receiverLock?"locked":"unlocked", qamStatus.fecLock));
                    if (qamStatus.receiverLock) {
                        acquired = true;
                        break;
                    }
                    /* we can't conclude no lock until application timeout */
                }
                else {
                     BERR_TRACE(rc);
                }
            }
        }

        if (acquired) {
            /**
             *  open the output record file
             **/
            recCtx.file = fopen(fileName,"wb");
            BDBG_ASSERT(recCtx.file);
            /**
             *  initiate the thread to extract data from recpump buffers and
             *  write it to output record file
             **/
            recCtx.started = true;
            pthread_create(&recordThread, NULL, record_thread, &recCtx);
            /**
             * start the recpump
             **/
            NEXUS_Recpump_Start(recCtx.recpump);
        }
        else {
            if (firstTune) {
                /**
                 *  trigger CPPM due to lack of signal
                 **/
                firstTune = false;
                if (cppm) {
                    calibrateSettings.cppm.enabled = true;
                    rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
                    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);
                    if (cppm && waitForCppm) {
                        BDBG_WRN(("Waiting for CPPM to complete..."));
                        BKNI_WaitForEvent(cppmEvent, 5000);
                    }
                    goto tune;
                }
            }
            BDBG_WRN(("not starting record because frontend not acquired"));
        }

        {
            char buf[64];

again:
            BDBG_WRN(("Enter frequency to tune again (st for status, ENTER to repeat, 0 to exit)"));

            fgets(buf, 64, stdin);
            if (buf[0] != '\n') {
                if (strstr(buf, "st") == buf) {
                    get_status(frontend, statusEvent);
                    goto again;
                }
                else {
                    float f;
                    if (sscanf(buf, "%f", &f) != 1) f = 0;
                    if (f < 1000000)
                        freq = (unsigned)(f*1000) * 1000;
                    else
                        freq = (unsigned)f;
                    if (!freq){ done = true; }
                }
            }
        }
        /**
         * before exiting from the app, stop the thread and recpump
         **/
        recCtx.started = false;
        pthread_join(recordThread,NULL);
        NEXUS_Recpump_Stop(recCtx.recpump);
        if (recCtx.file) fclose(recCtx.file);
    }
    /**
      * clean up nexus resources before exiting
     **/
done:
    NEXUS_Recpump_RemoveAllPidChannels(recCtx.recpump);
    NEXUS_Recpump_Close(recCtx.recpump);
    NEXUS_PidChannel_Close(allPassPidChannel);
    BKNI_DestroyEvent(recCtx.recEvent);
    if(statusEvent)BKNI_DestroyEvent(statusEvent);
    if(lockChangedEvent)BKNI_DestroyEvent(lockChangedEvent);
    if(cppmEvent)BKNI_DestroyEvent(cppmEvent);
    NEXUS_Platform_Uninit();
    return 0;
}

#else  /* if NEXUS_HAS_FRONTEND */
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND*/
