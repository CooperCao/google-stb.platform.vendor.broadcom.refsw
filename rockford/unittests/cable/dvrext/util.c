/***************************************************************************
 *     (c)2013-2015 Broadcom Corporation
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
 * Module Description:
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#include "util.h"
BDBG_MODULE(util);

B_DVR_MediaStorageType get_storage_device_type(void)
{
    B_DVR_MediaStorageType mediaStorageType;
    char *storageType=NULL;

    storageType = getenv("storage_type");
    if(!storageType) 
    {
        printf("\n export storage_type=loop or export storage_type=block");
        printf("\n no storage device type chosen");
        mediaStorageType = eB_DVR_MediaStorageTypeMax;
    }
    else
    {
        if(!strcmp(storageType,"block")) 
        {
            printf("\n DVR storage type is block device");
            printf("\n DVR is using SFS file system");
            mediaStorageType = eB_DVR_MediaStorageTypeBlockDevice;
            system("umount /dev/sda1");
            system("umount /dev/sda2");
            system("umount /dev/sda3"); 
        }
        else
        {
            if(!strcmp(storageType,"loop")) 
            {
                printf("\n DVR storage type is loop device");
                printf("\n DVR is using SFS file system");
                mediaStorageType = eB_DVR_MediaStorageTypeLoopDevice;
                system("umount /dev/loop0");
                system("umount /dev/loop1");
                system("umount /dev/loop2");
            }
            else
            {
                printf("\n export storage_type=loop or export storage_type=block");
                printf("\n invalid storage device type chosen");
                mediaStorageType = eB_DVR_MediaStorageTypeMax;
            }
        }
    }

    return mediaStorageType;
}

unsigned app_timediff(
    struct timeval *start,
    struct timeval *stop)
{
    unsigned timeDiff = 0;
    timeDiff = 1000*(stop->tv_sec - start->tv_sec) + ((stop->tv_usec - start->tv_usec)/1000);
    return timeDiff;
}

void app_create_psi(
    psiInfo_t *psiInfo,
    uint16_t pcrPid,
    uint16_t vidPid,
    uint16_t audPid,
    NEXUS_VideoCodec videoCodec,
    NEXUS_AudioCodec audioCodec)
{
    uint8_t pat_pl_buf[TS_HEADER_BUF_LENGTH], pmt_pl_buf[TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    unsigned streamNum;
    uint8_t  vidStreamType=0x2;
    uint8_t  audStreamType=0x4;
    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;
    TS_PID_info pidInfo;
    TS_PID_state pidState;

    /*printf("\n %s >>>",__FUNCTION__);*/
    switch(videoCodec)
    {
        case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
        case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
        case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
        case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
        default:
            BDBG_ERR(("Video encoder codec %d is not supported!\n",videoCodec));
    }
    switch(audioCodec)
    {
            case NEXUS_AudioCodec_eMpeg:         audStreamType = 0x4; break;
            case NEXUS_AudioCodec_eMp3:          audStreamType = 0x4; break;
            case NEXUS_AudioCodec_eAac    :      audStreamType = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlus:      audStreamType = 0x11; break;/* LOAS */
            /* MP2TS doesn't allow 14496-3 AAC+ADTS; here is placeholder to test AAC-HE before LOAS encode is supported; */
            case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType = 0x11; break;
            case NEXUS_AudioCodec_eAc3:          audStreamType = 0x81; break;
            default:
                BDBG_ERR(("Audio encoder codec %d is not supported!\n", audioCodec));
    }
    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&program, 1, PMT_PID);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, TS_HEADER_BUF_LENGTH);

    TS_PMT_setPcrPid(&pmtState, pcrPid);

    TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
    TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);

    if(audPid)
    {
        TS_PMT_stream_Init(&pmt_stream, audStreamType, audPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState,psiInfo->pat, TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    memcpy((uint8_t*)psiInfo->pat + buf_used, pat_pl_buf, pat_pl_size);
    /*printf("pmt_pl_size %u buf_used %u",pat_pl_size, buf_used);*/
    TS_PID_info_Init(&pidInfo, PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState,psiInfo->pmt, TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    memcpy((uint8_t*)psiInfo->pmt + buf_used, pmt_pl_buf, pmt_pl_size);
    return;
}

void app_inject_psi(
    void *context)
{
    psiInfo_t *psiInfo = (psiInfo_t *)(context);
    uint8_t ccByte=0;
    if(psiInfo->psiStop) 
    {
        return;
    }
    if(psiInfo->tsbService) 
    {
        B_DVR_TSBService_InjectDataStop(psiInfo->tsbService);
    }
    else
    {
        B_DVR_RecordService_InjectDataStop(psiInfo->recordService);
    }
    psiInfo->injectInProgress = false;
    psiInfo->injectCount++;
    if(psiInfo->injectCount%2 == 0)
    {
        ++psiInfo->patCcValue;
        ccByte = psiInfo->pat[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
        ccByte = (ccByte & 0xf0) | (psiInfo->patCcValue & 0xf);
        psiInfo->pat[4] = ccByte;
        if(psiInfo->tsbService) 
        {
            B_DVR_TSBService_InjectDataStart(psiInfo->tsbService,((void *)&psiInfo->pat[1]),188);
        }
        else
        {
            B_DVR_RecordService_InjectDataStart(psiInfo->recordService,((void *)&psiInfo->pat[1]),188);
        }
    }
    else
    {
        ++psiInfo->pmtCcValue;
        ccByte = psiInfo->pmt[4]; /* the 1st byte of pat/pmt arrays is for TSheader builder use */
        ccByte = (ccByte & 0xf0) | (psiInfo->pmtCcValue & 0xf);
        psiInfo->pmt[4] = ccByte;
        if(psiInfo->tsbService) 
        {
            B_DVR_TSBService_InjectDataStart(psiInfo->tsbService,((void *)&psiInfo->pmt[1]),188);
        }
        else
        {
            B_DVR_RecordService_InjectDataStart(psiInfo->recordService,((void *)&psiInfo->pmt[1]),188);
        }
    }
    psiInfo->injectInProgress = true;
    return;
}

void app_lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("\n lock change event received");
    BKNI_SetEvent((BKNI_EventHandle)context);
}

NEXUS_ParserBand app_tune_channel( NEXUS_FrontendHandle frontend,
                                   BKNI_EventHandle lockChangedEvent,
                                   channelInfo_t *channelInfo)
{
    NEXUS_Error rc;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendFastStatus fastStatus;
    NEXUS_ParserBand parserBand;
    NEXUS_Frontend_GetUserParameters(frontend,&userParams);
    parserBand = (NEXUS_ParserBand) userParams.param1;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BKNI_ResetEvent(lockChangedEvent);
    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency =  channelInfo->frequency;
    qamSettings.mode = channelInfo->modulation;
    qamSettings.annex = channelInfo->annex;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.symbolRate = channelInfo->symbolRate;
    qamSettings.lockCallback.callback = app_lock_changed_callback;
    qamSettings.lockCallback.context = lockChangedEvent;
    printf("\n tuning to freq %d qamMode %d \n",channelInfo->frequency,channelInfo->modulation);
    rc = NEXUS_Frontend_TuneQam(frontend,&qamSettings);
    if(rc)
    {
        printf("\n NEXUS_Frontend_TuneQam failed \n");
        parserBand = NEXUS_ParserBand_eInvalid;
    }
    BKNI_WaitForEvent(lockChangedEvent,2000);
    rc = NEXUS_Frontend_GetFastStatus(frontend, &fastStatus);
    if(rc)
    {   printf("\n NEXUS_Frontend_GetFastStatus failed \n");
        parserBand = NEXUS_ParserBand_eInvalid;
    }
    if(fastStatus.lockStatus != NEXUS_FrontendLockStatus_eLocked)
    {
        printf("\n QAM tuner failed to lock");
        parserBand = NEXUS_ParserBand_eInvalid;
    }
    return parserBand;
}

void app_udp_open(strm_ctx_t *ctx, 
                  unsigned client_port, 
                  char client_ip[10],
                  char server_eth_if[6])
{
    unsigned socketRcvBufSize = SOCKET_RECV_BUFFER_SIZE;
    int errCode=0;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    if(ctx->tsbService)
    {
        printf("\n tsb UDP streaming");
    }
    else
    {
        if(ctx->recordService)
        {
            printf("\n VOD UDP streaming");
        }
        else
        {
            printf("\n permanent recording streaming");
        }
    }

    ctx->sockedFd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(ctx->sockedFd > 0);

    ctx->clientSocket.sin_family = AF_INET;
    ctx->clientSocket.sin_port = htons(client_port);
    ctx->clientSocket.sin_addr.s_addr = inet_addr(client_ip);
    
    errCode = setsockopt(ctx->sockedFd, SOL_SOCKET, SO_SNDBUF,
                         (const char*)&socketRcvBufSize, 
                         sizeof(socketRcvBufSize)) ;
    assert(errCode >=0); 
    
    strcpy(ctx->socketConfig.ifr_name, server_eth_if);
    errCode = setsockopt(ctx->sockedFd,SOL_SOCKET, SO_BINDTODEVICE,
                         (void *)&ctx->socketConfig,sizeof(ctx->socketConfig));  
    assert(errCode >= 0);    

    if(!ctx->recordService) 
    {
        B_DVR_MediaFileSeekSettings mediaFileSeekSettings;
        B_DVR_MediaFileStats mediaFileStats;
        B_DVR_MediaFileOpenMode openMode;
        B_DVR_MediaFilePlayOpenSettings openSettings;
        B_DVR_MediaFileSettings fileSettings;
        off_t returnOffset=0;

        memset(&openMode,0,sizeof(openMode));
        openMode = eB_DVR_MediaFileOpenModeStreaming;
        memset(&openSettings,0,sizeof(openSettings));
        openSettings.volumeIndex = ctx->programSettings.volumeIndex;
        strcpy(openSettings.subDir,ctx->programSettings.subDir);
        ctx->mediaFile = B_DVR_MediaFile_Open(ctx->programSettings.programName,openMode,&openSettings);
        assert(ctx->mediaFile);

        rc = B_DVR_MediaFile_Stats(ctx->mediaFile,
                                   &mediaFileStats);
        assert(rc == B_DVR_SUCCESS);

        rc = B_DVR_MediaFile_GetSettings(ctx->mediaFile,
                                         &fileSettings);
        assert(rc == B_DVR_SUCCESS);
        fileSettings.fileOperation.streamingOperation.direction = eB_DVR_FileReadDirectionForward;
        fileSettings.fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeFull;
        rc = B_DVR_MediaFile_SetSettings(ctx->mediaFile,
                                         &fileSettings);
        assert(rc == B_DVR_SUCCESS);
        mediaFileSeekSettings.mediaSeek.offset = mediaFileStats.startOffset;
        mediaFileSeekSettings.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
        returnOffset = B_DVR_MediaFile_Seek(ctx->mediaFile,&mediaFileSeekSettings);
        assert(returnOffset>=0);
    }

    ctx->buffer = malloc(READ_BUFFER_SIZE + B_DVR_IO_BLOCK_SIZE);
    assert(ctx->buffer);
    ctx->alignedBuffer = (void*)B_DVR_IO_ALIGN_ROUND((unsigned long)ctx->buffer);
    assert(ctx->alignedBuffer);
    return;
}

void app_udp_producer(void *producer)
{
    strm_ctx_t *ctx =  (strm_ctx_t *)producer;
    ctx->nextBuf = 0;
read_again:
    if(ctx->streamingStop) 
    {
        return;
    }
    if(ctx->recordService) 
    {
        ctx->readSize = B_DVR_RecordService_Read(ctx->recordService,(unsigned *)ctx->alignedBuffer,READ_BUFFER_SIZE);
    }
    else
    {
        ctx->readSize = B_DVR_MediaFile_Read(ctx->mediaFile,(unsigned *)ctx->alignedBuffer,READ_BUFFER_SIZE);
    }
    assert(ctx->readSize >=0);
    if(!ctx->readSize)
    {   
         if(!(ctx->tsbService || ctx->recordService))
         { 
             printf("\n reached EOF");
             return;                                                            
         }
         else
         {
             goto read_again;
         }
    }
    B_Event_Set(ctx->consumerEvent);
    return;
}
void app_udp_consumer(void *consumer)
{
    strm_ctx_t *ctx =  (strm_ctx_t *)consumer;
    ssize_t transmitSize=0;
    unsigned desiredBitRate=0;
    unsigned char *tmpBuf=0;
    int errCode = 0;
    B_DVR_Media media;
    B_DVR_ManagerHandle dvrManager;
    B_DVR_ERROR rc = B_DVR_SUCCESS;
    unsigned timeDiff=0;
    unsigned transmitRate=0;
stream:
    if(ctx->streamingStop) 
    {
        return;
    }
    tmpBuf = ctx->alignedBuffer + (ctx->nextBuf * WRITE_BUFFER_SIZE);
    /* calculate send size */
    if((tmpBuf + WRITE_BUFFER_SIZE) <= (ctx->alignedBuffer + ctx->readSize))
    {
        transmitSize = WRITE_BUFFER_SIZE;
    }
    else
    {
        transmitSize = (ctx->alignedBuffer + ctx->readSize) - tmpBuf;
    }
    assert(transmitSize > 0);
    /* send the data */
    errCode = sendto(ctx->sockedFd,tmpBuf,transmitSize, 0, 
                     (struct sockaddr *)&ctx->clientSocket,
                     sizeof(struct sockaddr_in));
    assert(errCode >=0);
    ctx->nextBuf++;
    ctx->nextBuf %= NUM_WRITE_BUF_DESC;
    ctx->transmittedBytes += errCode;
    ctx->descCount++;
    dvrManager = B_DVR_Manager_GetHandle();
    rc = B_DVR_Manager_GetMediaNode(dvrManager,&ctx->programSettings,&media);
    assert(rc==0);
    gettimeofday(&ctx->stop,NULL);
    timeDiff = app_timediff(&ctx->start,&ctx->stop);
    transmitRate = (((unsigned)ctx->transmittedBytes)/timeDiff)*1000*8; /* bps */
    if(media.esStreamInfo[0].bitRate) 
    {
        desiredBitRate = media.esStreamInfo[0].bitRate;
    }
    else
    {
        desiredBitRate = media.maxStreamBitRate;
    }
    while(transmitRate > desiredBitRate)/*(media.maxStreamBitRate)) */
    {
        usleep(10000);
        gettimeofday(&ctx->stop,NULL);
        timeDiff = app_timediff(&ctx->start,&ctx->stop);
        transmitRate = (((unsigned)ctx->transmittedBytes)/timeDiff)*1000*8; /* bps */
    } 
    if((ctx->descCount % 10000) == 0) 
    {
        printf("\n descCount:%u bytes:%u transmitRate:%u maxRate:%u", 
               ctx->descCount, (unsigned)ctx->transmittedBytes,
               transmitRate, desiredBitRate);  
    }

    if((0 == ctx->nextBuf) || ((ctx->nextBuf*WRITE_BUFFER_SIZE) >= (unsigned)ctx->readSize))
    {
        B_Event_Set(ctx->producerEvent);
    }
    else
    {
        goto stream;
    }
    return;
}

void app_udp_close(strm_ctx_t *ctx)
{
    close(ctx->sockedFd);
    free(ctx->buffer);
    if(!ctx->recordService) 
    {
        B_DVR_MediaFile_Close(ctx->mediaFile);
    }
    return;
}
#if NEXUS_NUM_HDMI_OUTPUTS
void app_hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if (!status.videoFormatSupported[displaySettings.format])
        {
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
    return;
}
#endif
void app_decode_start(decode_ctx_t *ctx)
{
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_StcChannelSettings stcSettings;
    if(ctx->started) 
    {
        return;
    }
    if(ctx->playback) 
    {
        NEXUS_StcChannel_GetSettings(ctx->stcChannel,&stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
        NEXUS_StcChannel_SetSettings(ctx->stcChannel,&stcSettings);
    }
    else
    {
        NEXUS_StcChannel_GetSettings(ctx->stcChannel,&stcSettings);
        NEXUS_Timebase_GetSettings(stcSettings.timebase, &timebaseSettings);
        timebaseSettings.freeze = false;
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
        timebaseSettings.sourceSettings.pcr.pidChannel = ctx->pcrPidChannel;
        timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
        timebaseSettings.sourceSettings.pcr.maxPcrError = 0xFF;
        NEXUS_Timebase_SetSettings(stcSettings.timebase, &timebaseSettings);
        stcSettings.autoConfigTimebase = true;
        stcSettings.modeSettings.pcr.offsetThreshold = 8;
        stcSettings.modeSettings.pcr.maxPcrError = 255;
        stcSettings.modeSettings.pcr.disableTimestampCorrection = false;
        stcSettings.modeSettings.pcr.disableJitterAdjustment = false;
        stcSettings.modeSettings.pcr.pidChannel = ctx->pcrPidChannel;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
        NEXUS_StcChannel_SetSettings(ctx->stcChannel,&stcSettings);
    }
    NEXUS_VideoDecoder_Start(ctx->videoDecoder, &ctx->videoProgram);
    ctx->audioProgram.pidChannel = ctx->aPidChannel;
    NEXUS_AudioDecoder_Start(ctx->audioDecoder,&ctx->audioProgram);
    ctx->started = true;
    return;
}
void app_decode_stop(decode_ctx_t *ctx)
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    if(!ctx->started) 
    {
        return;
    }
    NEXUS_VideoDecoder_GetSettings(ctx->videoDecoder,&videoDecoderSettings);
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_VideoDecoder_SetSettings(ctx->videoDecoder,&videoDecoderSettings);
    NEXUS_AudioDecoder_Stop(ctx->audioDecoder);
    NEXUS_VideoDecoder_Stop(ctx->videoDecoder);
    ctx->started = false;
    return;
}

void app_print_rec_metadata(B_DVR_MediaNode mediaNode)
{
    unsigned i=0;
    bool first = true;
    B_DVR_MediaNodeSettings programInfo;
    printf("\n name      : %s",mediaNode->programName);
    printf("\n group     : %s",mediaNode->mediaNodeSubDir);
    printf("\n bitRate   : %u bps",mediaNode->maxStreamBitRate);
    printf("\n duration  : %u mins",(unsigned)(mediaNode->mediaEndTime - mediaNode->mediaStartTime)/(60*1000));
    printf("\n");
    for(i=0;i<mediaNode->esStreamCount;i++) 
    {
        if(mediaNode->esStreamInfo[i].pidType == eB_DVR_PidTypeVideo) 
        {
            if(first) 
            {
                printf("\n VIDEO ES");
                first = false;
            }
            printf("\n PID %u ",mediaNode->esStreamInfo[i].pid);
            printf("\n bitRate %u bps",mediaNode->esStreamInfo[i].bitRate);
            printf("\n picture dimension %ux%u",mediaNode->esStreamInfo[i].videoWidth,mediaNode->esStreamInfo[i].videoHeight);
            printf("\n frame rate %u fps",mediaNode->esStreamInfo[i].videoFrameRate);
            printf("\n nexus video codec %u",mediaNode->esStreamInfo[i].codec.videoCodec);
            printf("\n video level %u",mediaNode->esStreamInfo[i].level);
            printf("\n video profile %u",mediaNode->esStreamInfo[i].profile);

            printf("\n");
        }
    }
    first = true;
    for(i=0;i<mediaNode->esStreamCount;i++) 
    {
        if(mediaNode->esStreamInfo[i].pidType == eB_DVR_PidTypeAudio) 
        {
            if(first) 
            {
                printf("\n AUDIO ES");
                first = false;
            }
            printf("\n PID %u ",mediaNode->esStreamInfo[i].pid);
            printf("\n nexus audio codec %u ",mediaNode->esStreamInfo[i].codec.audioCodec);
            printf("\n SampleRate %u Hz",mediaNode->esStreamInfo[i].audioSampleRate);
            printf("\n SampleSize %u ",mediaNode->esStreamInfo[i].audioSampleSize);
            printf("\n ChannelCount %u",mediaNode->esStreamInfo[i].audioChannelCount);
            printf("\n");
        }
    }

    printf("\n META DATA FILES");
    printf("\n meta data for program: %s",mediaNode->mediaNodeFileName);
    printf("\n meta data for media files: %s",mediaNode->mediaFileName);
    printf("\n meta data for nav files: %s",mediaNode->navFileName);
    printf("\n");
    printf("\n MEDIA AND NAV FILE SEGMENTS INFO");
    programInfo.programName = mediaNode->programName;
    programInfo.subDir = mediaNode->mediaNodeSubDir;
    programInfo.volumeIndex = STORAGE_VOLUME_INDEX;
    B_DVR_Manager_PrintSegmentedFileRecord(B_DVR_Manager_GetHandle(),&programInfo);
    printf("\n\n");
    return;
}
