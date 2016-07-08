/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description: Bring up 93380sms platform
 *
 *****************************************************************************/
/* Basic bring-up of a streaming media server */

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#include "nexus_record.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(record_allpass_from_qam);

#define NO_INDEX_FILE 1

typedef struct
{
    NEXUS_FileRecordHandle file;
    NEXUS_RecordHandle record;
    NEXUS_RecpumpHandle recpump;
    NEXUS_PidChannelHandle pid;   
} B_REC_DESC;

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendQamStatus qamStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend %p\n", (void*)frontend);

    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
    fprintf(stderr, "QAM Lock callback, frontend %p - lock status %d, %d\n", (void*)frontend,
            qamStatus.fecLock, qamStatus.receiverLock);
}

static void fe_status(void *context)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendQamStatus qamStatus;

    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
    printf("FE %p:\n", (void *)frontend);
    printf("\t Receiver Lock: %s\n", qamStatus.receiverLock ? "yes" : "no");
    printf("\t FEC Lock: %s\n", qamStatus.fecLock ? "yes" : "no");
    printf("\t Ouput PLL Lock: %s\n", qamStatus.opllLock ? "yes" : "no");
    printf("\t Symbol Rate: %u\n", qamStatus.symbolRate);
    printf("\t Symbol Rate Error: %d\n", qamStatus.symbolRateError);
    printf("\t SNR: %u\n", qamStatus.snrEstimate);
    printf("\t Re-acquisition count: %u\n", qamStatus.reacquireCount);
    printf("\t Good RS count: %u\n", qamStatus.goodRsBlockCount);
    printf("\t ber raw count: %u\n", qamStatus.berRawCount);
}

int openRecording(char *filename, char *indexfilename, uint32_t freq, NEXUS_FrontendQamMode qamMode, uint32_t tunerNum, B_REC_DESC *pDesc)
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_FileRecordHandle file;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelSettings pidCfg0;
    NEXUS_PidChannelHandle pidCh0;
    NEXUS_RecordSettings recordCfg;
    NEXUS_Error rc;

printf("**********************************************\n");
printf("FILENAME: %s \n", filename);
printf("INDEX FILENAME: %s \n", indexfilename);
printf("FREQ: %d \n", freq);
printf("QAM MODE: %d \n", (int)qamMode);
printf("TUNER NUMBER: %u \n", tunerNum);
printf("**********************************************\n");

    NEXUS_Platform_GetConfiguration(&platformConfig);
    frontend = platformConfig.frontend[tunerNum];

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    
    parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    parserBandSettings.allPass=true;
    parserBandSettings.acceptNullPackets=false;

    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq;
    qamSettings.mode = qamMode;
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.lockCallback.callback = lock_callback;
    qamSettings.lockCallback.context = frontend;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    NEXUS_Frontend_TuneQam(frontend, &qamSettings);

    NEXUS_PidChannel_GetDefaultSettings(&pidCfg0) ;
    NEXUS_ParserBand_GetAllPassPidChannelIndex(parserBand, (unsigned *) &pidCfg0.pidChannelIndex);
    /* The pid number is redundant here. */
    pidCh0 = NEXUS_PidChannel_Open(parserBand, 0x0, &pidCfg0);

    recpump = NEXUS_Recpump_Open(tunerNum, NULL);
    record = NEXUS_Record_Create();
    NEXUS_Record_GetSettings(record, &recordCfg);
    recordCfg.recpump = recpump;
    NEXUS_Record_SetSettings(record, &recordCfg);

    file = NEXUS_FileRecord_OpenPosix(filename, indexfilename);
    if (!file) {
        BDBG_ERR(("unable to open %s, %s", filename, indexfilename));
        return BERR_TRACE(-1);
    }

#if NO_INDEX_FILE
    /* Do not generate an index */
    NEXUS_Record_AddPidChannel(record,pidCh0, NULL);
#else
    /* Generate index using a vid value */
    {
    NEXUS_RecordPidChannelSettings pidCfg;
    NEXUS_Record_GetDefaultPidChannelSettings(&pidCfg);
    pidCfg.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidCfg.recpumpSettings.pidTypeSettings.video.index = true;
    pidCfg.recpumpSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2; /* video codec is necessary to generate correct index */
    /* generate index using pid number instead of pid chanel number */
    pidCfg.recpumpSettings.pidTypeSettings.video.pid = 0x11;
    NEXUS_Record_AddPidChannel(record,pidCh0,&pidCfg);
    }
#endif

    NEXUS_Record_Start(record, file);
    pDesc->record = record;
    pDesc->file = file;
    pDesc->recpump = recpump;
    pDesc->pid=pidCh0;
    return 0;
}


void closeRecording(const B_REC_DESC *channel)
{
    NEXUS_Record_Stop(channel->record);
    NEXUS_Record_RemoveAllPidChannels(channel->record);
    NEXUS_PidChannel_Close(channel->pid);
    NEXUS_FileRecord_Close(channel->file);
    NEXUS_Record_Destroy(channel->record);
    NEXUS_Recpump_Close(channel->recpump);
}


#define BUFSIZE 50

unsigned getSelection(unsigned min, unsigned max)
{
    char inBuf[BUFSIZE];
    unsigned item;

    while(1)
    {
        fgets(inBuf, BUFSIZE, stdin);
        if(!sscanf(inBuf, "%u", &item) || (item > max) || (!(item >= min))) 
            printf("invalid value! Try Again >");
        else
            break;
    }

    printf("\n");
    return item;
}


int main(void)
{
    unsigned item, channel, freq, mod, i;
    bool exit = false;
    char recName[100], idxName[100];
    unsigned streamCnt[8], count;
    B_REC_DESC currentRecs[8];
    char *filenameStem = "videos/";
    unsigned stemSize;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    
    stemSize = strlen(filenameStem);

    memset(streamCnt, 0, sizeof(streamCnt)); /* current recording number on a record channel */
    memset(currentRecs, 0, sizeof(currentRecs)); /* record handles for each record channel */

    strcpy(recName, filenameStem);
    strcpy(idxName, filenameStem);
    strcpy(recName + stemSize, "T_0_STRM_0000.mpg");
    strcpy(idxName + stemSize, "T_0_STRM_0000.nav");

    rc = NEXUS_Platform_Init(NULL);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    
    while(!exit)
    {
        printf("\nMENU\n\n");
        printf("1/ Open recording on tuner\n");
        printf("2/ Close recording on tuner\n");
        printf("3/ Stats\n");
        printf("4/ Quit\n> ");
        item = getSelection(1, 4);

        switch(item){
            
            case 1: /* OPEN_RECORD */
            
                printf("Which recording channel do you wish to open [0-7]? ");
                channel = getSelection(0, 7);
                
                if(currentRecs[channel].record)
                {
                    printf("Must first close recording on channel %d\n", channel);
                    continue;
                }

                printf("Select frequency in Hz [54 - 1000000000] ");
                freq = getSelection(54, 1000000000);                

                printf("1/ QAM64\n");
                printf("2/ QAM256\n");
                printf("Select Modulation >");
                mod = getSelection(1, 2);

                count = ++(streamCnt[channel]);

                /* modify filename */
                recName[2 + stemSize] = idxName[2 + stemSize] = '0' + channel;
                recName[12 + stemSize] = idxName[12 + stemSize] = (count % 10) + '0';
                count /= 10;
                recName[11 + stemSize] = idxName[11 + stemSize] = (count % 10) + '0';
                count /= 10;                
                recName[10 + stemSize] = idxName[10 + stemSize] = (count % 10) + '0';
                count /= 10;
                recName[9 + stemSize] = idxName[9 + stemSize] = (count % 10) + '0';
                count /= 10;

                printf("Preparing to record file %s\n\n", recName);
               
                openRecording(recName, idxName, freq,
                    (mod == 1) ? NEXUS_FrontendQamMode_e64 : NEXUS_FrontendQamMode_e256, channel, &currentRecs[channel]);
                break;
                
            case 2: /* CLOSE RECORD */
            
                printf("which channel do you wish to close [0-7]? >");
                channel = getSelection(0, 7);
                if(currentRecs[channel].record)
                {
                    closeRecording(&(currentRecs[channel]));
                    currentRecs[channel].record = NULL;
                }
                else
                    printf("No recording to close on channel %d\n", channel);
                
                break;
            case 3:
                for(i=0; i < 8; i++)
                {
                    if(currentRecs[i].record)
                    {
                        char systemCommand[107];
                        sprintf(systemCommand, "du -h %s\n", recName);
                        printf("=========Tuner %d=========\n", i);
                        fe_status((void *)platformConfig.frontend[i]);
                        system(systemCommand);
                        printf("=======================\n");
                    }
                }
                break;
            case 4:
            default:
                for(i=0; i < 8; i++)
                {
                    if(currentRecs[i].record)
                    {
                        closeRecording(&(currentRecs[i]));
                        currentRecs[i].record = NULL;
                    }
        }
                exit = true;
                break;
    }
    }
    
    NEXUS_Platform_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
