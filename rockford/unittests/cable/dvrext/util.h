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
#ifndef _UTIL_H
#define _UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "nexus_base_types.h"
#include "nexus_types.h"
#include "nexus_frontend.h"
#include "nexus_frontend_qam.h"
#include "nexus_parser_band.h"
#include "nexus_display.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_video_window.h"
#include "nexus_video_adj.h"
#include "nexus_video_input.h"
#include "tshdrbuilder.h"
#include "b_dvr_manager.h"
#include "b_dvr_mediastorage.h"
#include "b_dvr_datainjectionservice.h"
#include "b_dvr_recordservice.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_mediafile.h"

/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define TS_HEADER_BUF_LENGTH   189
#define INJECT_INTERVAL 50
#define PMT_PID 0x55 
#define PAT_PID 0x0
#define INJECT_INTERVAL 50
#define MAX_TSB_BUFFERS 8  /* number of media and nav file segments */
#define MAX_TSB_TIME 5*60*1000 /* 5 minutes in ms */
#define STORAGE_VOLUME_INDEX 0
#define TS_PACKET_SIZE 188  
#define NUM_WRITE_TS_PACKETS 8  
#define WRITE_BUFFER_SIZE (TS_PACKET_SIZE*NUM_WRITE_TS_PACKETS) 
#define NUM_WRITE_BUF_DESC 512
#define READ_BUFFER_SIZE WRITE_BUFFER_SIZE*NUM_WRITE_BUF_DESC
#define SOCKET_RECV_BUFFER_SIZE 6000000 

#define MEDIA_DEVICE_MOUNT_PATH "/data" /* this should be already available on the existing file system */
#define VSFS_DEVICE "/data/vsfs" /* This would be created by media storage */
#define VSFS_SIZE 50*1024 /* units = MB */
#define VOLUME_INDEX 0

#define STORAGE_DEVICE "/dev/sda"
/* command to zap the storage */
#define STORAGE_ZAP_COMMAND "sgdisk -Z /dev/sda"
#define STORAGE_PRINT_PARTITION_TABLE "sgdisk -p /dev/sda"

typedef struct channelInfo_t
{
    NEXUS_FrontendQamAnnex annex; 
    NEXUS_FrontendQamMode modulation;
    unsigned int  symbolRate;
    unsigned int  frequency;
    unsigned int vPid;
    unsigned int aPid;
    unsigned int pcrPid;
    NEXUS_VideoCodec vCodec;
    NEXUS_AudioCodec aCodec;
} channelInfo_t;

typedef struct psiInfo_t
{ 
    uint8_t pat[TS_HEADER_BUF_LENGTH];
    uint8_t pmt[TS_HEADER_BUF_LENGTH];
    unsigned injectCount;
    unsigned patCcValue;
    unsigned pmtCcValue;
    bool injectInProgress;
    B_SchedulerTimerId timer;
    NEXUS_PidChannelHandle injectChannel;
    B_DVR_DataInjectionServiceHandle injectService;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_RecordServiceHandle recordService;
    bool psiStop;
}psiInfo_t;

typedef struct strm_ctx_t
{
    B_DVR_MediaNodeSettings programSettings;
    B_DVR_MediaFileHandle mediaFile;
    unsigned char *buffer;
    unsigned char *alignedBuffer;
    struct sockaddr_in clientSocket;
    struct ifreq socketConfig;
    int  sockedFd;
    struct timeval start, stop;
    ssize_t bufSize;
    ssize_t readSize;
    ssize_t transmittedBytes;
    unsigned nextBuf;
    unsigned descCount;
    B_DVR_TSBServiceHandle tsbService;
    B_DVR_RecordServiceHandle recordService;
    B_SchedulerEventId producerId;
    B_SchedulerEventId consumerId;
    B_EventHandle producerEvent;
    B_EventHandle consumerEvent;
    bool streamingStop;
}strm_ctx_t;

typedef struct decode_ctx_t
{
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PidChannelHandle vPidChannel;
    NEXUS_PidChannelHandle aPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    bool playback;
    bool started;
}decode_ctx_t;

typedef enum play_operation
{   
    eQuit,
    ePause,
    ePlay,
    eRewind,
    eFastForward,
    eSlowMotionForward,
    eSlowMotionReverse,
    eFrameAdvance,
    eFrameReverse,
    eForwardJump,
    eReverseJump,
    eBadSignalHandling
}play_operation;

B_DVR_MediaStorageType get_storage_device_type(void);
unsigned app_timediff(
    struct timeval *start,
    struct timeval *stop); 

void app_create_psi(
    psiInfo_t *psiInfo,
    uint16_t pcrPid,
    uint16_t vidPid,
    uint16_t audPid,
    NEXUS_VideoCodec videoCodec,
    NEXUS_AudioCodec audioCodec);

void app_inject_psi(
    void *context);

void app_lock_changed_callback(
    void *context, 
    int param);

NEXUS_ParserBand app_tune_channel(
    NEXUS_FrontendHandle frontend,
    BKNI_EventHandle lockChangedEvent,
    channelInfo_t *channelInfo);

void app_udp_open(strm_ctx_t *ctx, 
                  unsigned client_port, 
                  char client_ip[10],
                  char server_eth_if[6]);
void app_udp_producer(void *producer);
void app_udp_consumer(void *consumer);
void app_udp_close(strm_ctx_t *ctx);

#if NEXUS_NUM_HDMI_OUTPUTS
void app_hotplug_callback(void *pParam, int iParam);
#endif
void app_decode_start(decode_ctx_t *ctx);
void app_decode_stop(decode_ctx_t *ctx);
void app_print_rec_metadata(B_DVR_MediaNode mediaNode);

#ifdef __cplusplus
}
#endif

#endif /*_UTIL_H */
