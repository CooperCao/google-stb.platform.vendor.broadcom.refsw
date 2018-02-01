/******************************************************************************
 * Copyright (C) 2008-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
/* Example to tune a VSB channel using nexus */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_pid_channel.h"
#include "nexus_message.h"

#include "bstd.h"
#include "bkni.h"

#include <assert.h>
#include <stdlib.h>

#define LINE_BUF_SIZE 256
BKNI_EventHandle lockCallbackEvent;
uint32_t start_time, end_time, Total_time;
static    NEXUS_FrontendHandle frontend=NULL;

int lock_channel(void);

#include <sys/time.h>
unsigned long f_gettime(void)
{
#if !HAS_NPTL
    int rc;
    struct timeval now;
    rc = gettimeofday(&now, NULL);
    if (rc) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        return 0;
    }
    return now.tv_sec * 1000000 + now.tv_usec;
#else
    int rc;
    struct timespec now;
    /* It's ok to use clock_gettime even without NPTL. */
    rc = clock_gettime(CLOCK_MONOTONIC, &now);
    if (rc) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        return 0;
    }
    return now.tv_sec * 1000000 + now.tv_nsec / 1000;
#endif
}

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendOfdmStatus ofdmStatus;

    BSTD_UNUSED(param);
    end_time = (uint32_t) f_gettime();
    NEXUS_Frontend_GetOfdmStatus(frontend, &ofdmStatus);
    printf("OFDM Lock callback: receiverLock=%d, fecLock=%d  Nosignsl=%d\n", ofdmStatus.receiverLock, ofdmStatus.fecLock, ofdmStatus.noSignalDetected);
    printf("fecLock=%d \n",  ofdmStatus.fecLock);
    if (ofdmStatus.fecLock)
    {
        lock_channel();
        end_time = (uint32_t) f_gettime();
    }
    printf("Time spent is = %d ms \n",  ((end_time - start_time)/1000));
    Total_time += ((end_time - start_time)/1000);
    BKNI_SetEvent(lockCallbackEvent);
}


void message_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int lock_channel(void) {
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_MessageHandle msg;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelSettings pidChannelSettings;
    BKNI_EventHandle event;
    unsigned count = 100;
    void *buffer;
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_FrontendUserParameters userParams;


    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    /* Map a parser band to the demod's input band. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannel = NEXUS_PidChannel_Open(parserBand, 0x0, &pidChannelSettings);

    BKNI_CreateEvent(&event);

    /* use app-allocated message buffer as recommended in Nexus_Usage.pdf. */
    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.alignment = 1024;
#define BUFFER_SIZE 4096
    NEXUS_Memory_Allocate(BUFFER_SIZE, &allocSettings, &buffer);

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.dataReady.callback = message_callback;
    settings.dataReady.context = event;
    settings.maxContiguousMessageSize = 4096;
    settings.bufferSize = 0; /* don't have Message alloc the buffer. recommended for maximum flexibility. */
    msg = NEXUS_Message_Open(&settings);
    BDBG_ASSERT(msg);

    NEXUS_Message_GetDefaultStartSettings(msg, &startSettings);
    startSettings.pidChannel = pidChannel;
    startSettings.buffer = buffer; /* use the app-allocated buffer. */
    startSettings.bufferSize = BUFFER_SIZE;
    /* use the default filter for any data */

    if (NEXUS_Message_Start(msg, &startSettings)) goto error;

    /* Read the PAT a few times */
    while (count--) {
        const uint8_t *buffer;
        size_t size;
        int programNum, message_length;

        if (NEXUS_Message_GetBuffer(msg, (const void **)&buffer, &size)) goto error;
        if (!size) {
            BERR_Code rc = BKNI_WaitForEvent(event, 5 * 1000); /* wait 5 seconds */
            if (rc) {printf("test failed"); rc = -1; break;}
            continue;
        }

#define TS_READ_16( BUF ) ((uint16_t)((BUF)[0]<<8|(BUF)[1]))
#define TS_PSI_GET_SECTION_LENGTH( BUF )    (uint16_t)(TS_READ_16( &(BUF)[1] ) & 0x0FFF)

        /* We should always get whole PAT's because maxContiguousMessageSize is 4K */
        message_length = TS_PSI_GET_SECTION_LENGTH(buffer) + 3;
        BDBG_ASSERT(size >= (size_t)message_length);

        printf("Found PAT: id=%d size=%d\n", buffer[0], message_length);
        for (programNum=0;programNum<(TS_PSI_GET_SECTION_LENGTH(buffer)-7)/4;programNum++) {
            unsigned byteOffset = 8 + programNum*4;
            printf("  program %d: pid 0x%x\n",
                TS_READ_16( &buffer[byteOffset] ),
                (uint16_t)(TS_READ_16( &buffer[byteOffset+2] ) & 0x1FFF));
        }
        /* only complete one PAT */
        NEXUS_Message_ReadComplete(msg, message_length);
        count = 0;
    }

    NEXUS_Message_Stop(msg);
    NEXUS_Message_Close(msg);
    NEXUS_PidChannel_Close(pidChannel);
    BKNI_DestroyEvent(event);
    printf("finish lock channel");

    return 0;
error:
    return 1;
}



int main(int argc, char **argv)
{
    NEXUS_FrontendUserParameters userParams;

    NEXUS_FrontendOfdmSettings ofdmSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    unsigned i;
    unsigned freq = 473;
    FILE *pFile, *pFilew;
    char szLineBuf[LINE_BUF_SIZE];
    NEXUS_FrontendOfdmStatus ofdmStatus;

    pFile = fopen("channels.txt", "r");
    if ( NULL == pFile )
    {
        printf("channels.txt file not found.\n");
        return 0;
    }

    pFilew = fopen("channels_results.txt", "w");
    if ( NULL == pFilew )
    {
        printf("channels_results.txt file not found.\n");
        return 0;
    }

    if (argc > 1) {
        freq = atoi(argv[1]);
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    
    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.ofdm = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find OFDM-capable frontend\n");
        return -1;
    }

    BKNI_CreateEvent(&lockCallbackEvent);
    NEXUS_Frontend_GetDefaultOfdmSettings(&ofdmSettings);
    ofdmSettings.frequency = freq * 1000000;
    ofdmSettings.mode = NEXUS_FrontendOfdmMode_eIsdbt;
    ofdmSettings.bandwidth = NEXUS_FrontendOfdmBandwidth_e6Mhz;
    ofdmSettings.ifFrequency = 0;

    ofdmSettings.lockCallback.callback = lock_callback;
    ofdmSettings.lockCallback.context = frontend;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    ofdmSettings.mode = NEXUS_FrontendOfdmMode_eIsdbt;
    ofdmSettings.bandwidth =  NEXUS_FrontendOfdmBandwidth_e6Mhz;
    ofdmSettings.ifFrequency =  0;

    Total_time = 0;
    while ( fgets(szLineBuf, LINE_BUF_SIZE, pFile) )
    {
        i = atoi(szLineBuf);
        printf ("\nTune to %d Hz\n\n", i);
        ofdmSettings.frequency = i;
        /*fputs(szLineBuf, pFilew);*/

        ofdmSettings.acquisitionMode =  NEXUS_FrontendOfdmAcquisitionMode_eScan;
        printf("TuneOFDM Settings: Mode=%d, CCI=%d, BW= %d, priority=%d, Freq=%d, IFFreq=%d, manTPS=%d, manualMode=%d, manualTMCC=%d, Pullin=%d, \n AcqMode=%d", ofdmSettings.mode, ofdmSettings.cciMode, ofdmSettings.bandwidth, ofdmSettings.priority, ofdmSettings.frequency, ofdmSettings.ifFrequency, ofdmSettings.manualTpsSettings,ofdmSettings.manualModeSettings, ofdmSettings.manualTmccSettings,ofdmSettings.pullInRange, ofdmSettings.acquisitionMode);

        start_time = (uint32_t)f_gettime();
        NEXUS_Frontend_TuneOfdm(frontend, &ofdmSettings);
        BKNI_WaitForEvent(lockCallbackEvent, 10000);

        printf("Tuned %d Hz\n", ofdmSettings.frequency);
          
        NEXUS_Frontend_GetOfdmStatus(frontend, &ofdmStatus);
        if ( ofdmStatus.fecLock )
            {
                    fprintf(pFilew," %d \t Locked:\t %d \t Layer A(%d %d %d %d)", i, (end_time-start_time)/1000, ofdmStatus.modulationA, ofdmStatus.codeRateA, ofdmStatus.timeInterleavingA, ofdmStatus.numSegmentsA);
                    fprintf(pFilew," | Layer B(%d %d %d %d)", ofdmStatus.modulationB, ofdmStatus.codeRateB, ofdmStatus.timeInterleavingB, ofdmStatus.numSegmentsB);
                    fprintf(pFilew," | Layer C(%d %d %d %d) \r\n", ofdmStatus.modulationC, ofdmStatus.codeRateC, ofdmStatus.timeInterleavingC, ofdmStatus.numSegmentsC);
            }
        else if ( ofdmStatus.noSignalDetected )
            {
                    fprintf(pFilew," %d \t Nosignal:\t %d \t \r\n", i, (end_time-start_time)/1000);
            }
        else 
            {    
                    fprintf(pFilew," %d \t Unlocked:\t %d \t \r\n", i, (end_time-start_time)/1000);    
            }

    }

    printf("Total Scan time %d ms\n", Total_time );
    fclose (pFile);
    fclose (pFilew);
    printf("Press enter to tune again\n");
    BKNI_DestroyEvent(lockCallbackEvent);
    NEXUS_Frontend_Release(frontend);
    NEXUS_Platform_Uninit();

    return 0;
}
#else  /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */

