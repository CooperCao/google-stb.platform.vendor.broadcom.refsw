/***************************************************************************
 *     (c)2010-2014 Broadcom Corporation
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
 **************************************************************************/
#include "bxcode.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "bxcode_priv.h"
#include "nexus_video_adj.h"
#include "nexus_core_utils.h"
#include "nexus_platform.h"
#include <fcntl.h>

BDBG_MODULE(bxcode_priv);
BDBG_FILE_MODULE(bxcode_psi);
BDBG_FILE_MODULE(chunk_state);
BDBG_FILE_MODULE(btp_fnrt);
BDBG_FILE_MODULE(video_playpump);
BDBG_FILE_MODULE(audio_playpump);

BXCode_P_State g_BXCode_P_State;
static NEXUS_PlatformConfiguration g_BXCode_platformConfig;

#define BXCODE_P_CHUNK_DURATION             10000 /* 10 seconds video chunk */
#define BXCODE_P_TSHEADER_LENGTH            189
#define BXCODE_P_PROGRAM_INFO_LENGTH_OFFSET (TS_PSI_LAST_SECTION_NUMBER_OFFSET+3)
#define BXCODE_P_PROGRAM_INFO_LENGTH(buf)   (TS_READ_16(&buf[BXCODE_P_PROGRAM_INFO_LENGTH_OFFSET])&0xFFF)
#define BXCODE_P_DESCRIPTOR_BASE(buf)       (&buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+5])
#define BXCODE_P_STREAM_BASE(buf)           (TS_PSI_LAST_SECTION_NUMBER_OFFSET + 5 + BXCODE_P_PROGRAM_INFO_LENGTH(buf))
#define BXCODE_P_TS_USER_DATA_ALL           (unsigned)(-1)
#define BXCODE_P_MAX_MESSAGE_FILTERS        20


typedef enum BXCode_P_BTPCommand
{
   BXCode_P_BTPCommand_eChunkId = 0x0D, /* reuse PICTURE_TAG */
   BXCode_P_BTPCommand_eLast = 0x82, /* LAST */
   BXCode_P_BTPCommand_eEOS = 0x0A /* protocol agnostic EOS or so-called INLINE_FLUSH or TPD */
} BXCode_P_BTPCommand;

static const struct {
    double frequency;
    NEXUS_VideoFrameRate nexusFramerate;
} BXCode_P_Verticalfrequency[NEXUS_VideoFrameRate_eMax] = {
    {0, NEXUS_VideoFrameRate_eUnknown},
    {10, NEXUS_VideoFrameRate_e10},
    {12.5, NEXUS_VideoFrameRate_e12_5},
    {15, NEXUS_VideoFrameRate_e15},
    {20, NEXUS_VideoFrameRate_e20},
    {23.976, NEXUS_VideoFrameRate_e23_976},
    {24, NEXUS_VideoFrameRate_e24},
    {25, NEXUS_VideoFrameRate_e25},
    {29.970, NEXUS_VideoFrameRate_e29_97},
    {30, NEXUS_VideoFrameRate_e30},
    {50, NEXUS_VideoFrameRate_e50},
    {59.940, NEXUS_VideoFrameRate_e59_94},
    {60, NEXUS_VideoFrameRate_e60},
    {14.985, NEXUS_VideoFrameRate_e14_985}};

/**
Private functions:
**/
static double BXCode_P_GetRefreshRateFromFrameRate(NEXUS_VideoFrameRate frameRate)
{
    unsigned i;
    for(i=0;i<sizeof(BXCode_P_Verticalfrequency)/sizeof(*BXCode_P_Verticalfrequency);i++) {
        if (frameRate == BXCode_P_Verticalfrequency[i].nexusFramerate) {
            return BXCode_P_Verticalfrequency[i].frequency;
        }
    }
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return 0; /* NEXUS_VideoFrameRate_eUnknown */
}

static void BXCode_P_VidSrcStreamChangedCallback(void *context, int param)
{
    NEXUS_VideoDecoderStreamInformation streamInfo;
    NEXUS_VideoDecoderHandle decoder = (NEXUS_VideoDecoderHandle)context;

    BSTD_UNUSED(param);
    NEXUS_VideoDecoder_GetStreamInformation(decoder, &streamInfo);
    BDBG_MSG(("Video Source Stream Change callback: %ux%u@%.3f%c",
        streamInfo.sourceHorizontalSize,
        streamInfo.sourceVerticalSize,
        BXCode_P_GetRefreshRateFromFrameRate(streamInfo.frameRate),
        streamInfo.streamProgressive? 'p' : 'i'));
}

static void BXCode_P_VidChunkDoneCallback(void *context, int param)
{
    BKNI_EventHandle event = (BKNI_EventHandle)context;

    BDBG_MODULE_MSG(chunk_state, ("Video[%d] chunk done callback...", param));
    BKNI_SetEvent(event);
}

static void BXCode_P_SeekNextChunk(
    BXCode_P_Context *pContext,
    BXCode_P_VideoChunk *nextChunk /* output */)
{
    BNAV_Player_Position *startPosition = &pContext->latestVideoChunk.startPosition;
    BNAV_Player_Position *endPosition   = &pContext->latestVideoChunk.endPosition;
    unsigned long dpts = 0;
    long index;

    BDBG_ASSERT(nextChunk);

    /* the latest chunk is global to be protected by mutex */
    B_Mutex_Lock(pContext->mutexChunk);
    index = BNAV_Player_FindIFrameFromIndex(pContext->bcmplayer, pContext->latestVideoChunk.startRapIndex, 1);
    BNAV_Player_GetPositionInformation(pContext->bcmplayer, index, startPosition);
    /* find the end of chunk index */
    do {
        /* kludge: in case this function returns index backwards by 1, search 2 ahead */
        index = BNAV_Player_FindIFrameFromIndex(pContext->bcmplayer, index+2, 1);
        if(index == -1) /* end of file */ {
            endPosition->offsetLo = (unsigned long) (-1);
            endPosition->offsetHi = (unsigned long) (-1);
            endPosition->index    = -1;
            break;
        }
        BNAV_Player_GetPositionInformation(pContext->bcmplayer, index, endPosition);
        /* discontinuity may result in a short chunk; but it should be ok */
        dpts = (endPosition->pts >= startPosition->pts)? (endPosition->pts - startPosition->pts)
                : ((unsigned long)(-1) - startPosition->pts + 1 + endPosition->pts);
        BDBG_MODULE_MSG(chunk_state, ("dpts[%lu], start[%ld]@pts=%08x, end[%ld] pts=%08x", dpts, startPosition->index, startPosition->pts, index, endPosition->pts));
    } while(dpts/45 < BXCODE_P_CHUNK_DURATION);

    /* in case stream file doesn't start with transport packet sync byte 0x47, e.g., bad file truncation,
        simple round down file offset with 188-byte might not start with transport packet header sync byte;
        subtract 188-byte to make sure the first I-frame's transport packet header is included.  */
    startPosition->offsetLo = (startPosition->offsetLo > 188)? (startPosition->offsetLo - 188) : 0;

    /* round down offset to 188-byte TP aligned */
    startPosition->offsetLo = startPosition->offsetLo / 188 * 188;
    if(endPosition->index != -1) {/* end of file no need to round down */
        endPosition->offsetLo   = endPosition->offsetLo / 188 * 188;
    }
    BDBG_MODULE_MSG(chunk_state, ("Chunk start at index %ld, end at index %ld with duration %u ms\n",
        pContext->latestVideoChunk.startRapIndex, index, dpts/45));
    pContext->latestVideoChunk.endRapIndex = index;

    pContext->latestVideoChunk.startOffset = pContext->latestVideoChunk.startPosition.offsetLo + ((uint64_t)pContext->latestVideoChunk.startPosition.offsetHi<<32);
    pContext->latestVideoChunk.endOffset   = pContext->latestVideoChunk.endPosition.offsetLo + ((uint64_t)pContext->latestVideoChunk.endPosition.offsetHi<<32);

    /* copy the chunk info */
    *nextChunk = pContext->latestVideoChunk;

    /* bump up chunk id to prepare the next chunk */
    pContext->latestVideoChunk.id++;
    pContext->latestVideoChunk.startRapIndex = pContext->latestVideoChunk.endRapIndex;
    B_Mutex_Unlock(pContext->mutexChunk);
}

static void BXCode_P_InsertBTP(
    BXCode_P_Context *pContext, void *buf, uint32_t chunkId, unsigned pid, BXCode_P_BTPCommand command)
{
    int i;
    static uint8_t btp[188] = {
            0x47, 0x1f, 0xff, 0x20,
            0xb7, 0x02, 0x2d, 0x00,
            'B',  'R',  'C',  'M',  /* signature */
            0x00, 0x00, 0x00, 0x0d, /* CHUNK_ID command (reuse PICTURE_TAG) */
            0x00, 0x00, 0x00, 0x00, /* dwod 1 */
            0x00, 0x00, 0x00, 0x00, /* dwod 2 */
            0x00, 0x00, 0x00, 0x00, /* dwod 3 */
            0x00, 0x00, 0x00, 0x00, /* dwod 4 */
            0x00, 0x00, 0x00, 0x00, /* dwod 5 */
            0x00, 0x00, 0x00, 0x00, /* dwod 6 */
            0x00, 0x00, 0x00, 0x00, /* dwod 7 */
            0x00, 0x00, 0x00, 0xbc, /* dwod 8 =188 to avoid RAVE dropping following packets */
            0x00, 0x00, 0x00, 0x00, /* dwod 9: chunkID in big endian */
            /* rest of BTP = 0x00 */
        };

    /* mutex to protect atomic static array from multi-thread */
    B_Mutex_Lock(pContext->mutexChunk);
    btp[1] = (pid >> 8) & 0x1f;
    btp[2] = pid & 0xff;  /* PID */

    btp[12 + 3] = command;

    switch(command)
    {
       case BXCode_P_BTPCommand_eChunkId:
       case BXCode_P_BTPCommand_eLast:
       case BXCode_P_BTPCommand_eEOS:
        /* big endian dword[9] for CHUNK_ID BTP command's chunkID payload */
        btp[12+36] = (unsigned char) ((chunkId & 0xff000000) >> 24);
        btp[12+36+1] = (unsigned char) ((chunkId & 0x00ff0000) >> 16);
        btp[12+36+2] = (unsigned char) ((chunkId & 0x0000ff00) >> 8);
        btp[12+36+3] = (unsigned char) (chunkId & 0x000000ff);
        break;
    }
    BKNI_Memcpy(buf,(void *)btp,188);
    BDBG_MODULE_MSG(btp_fnrt, ("BTP:"));
    for(i=0; i<52; i+=4) {
        BDBG_MODULE_MSG(btp_fnrt, ("%02x %02x %02x %02x", btp[i], btp[i+1], btp[i+2], btp[i+3]));
    }
    B_Mutex_Unlock(pContext->mutexChunk);
}

static void BXCode_P_TranscoderFinishCallback(void *context, int param)
{
    BXCode_P_Context *pContext = (BXCode_P_Context*)context;

    BSTD_UNUSED(param);
    pContext->toStop = true;
    BDBG_MSG(("BXCode%u finish callback invoked, now stop the mux.", pContext->id));
    BKNI_SetEvent(pContext->finishEvent);
}

static void BXCode_P_VideoImageInputBufferCallback( void *context, int param )
{
    NEXUS_CallbackDesc *desc = (NEXUS_CallbackDesc*)context;
    BSTD_UNUSED(param);
    if(desc->callback) {
        desc->callback(desc->context, desc->param);
    }
}

static void BXCode_P_PlaypumpCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void BXCode_P_RecpumpDataready_callback(void *context, int param)
{
    BXCode_P_Context *pContext = (BXCode_P_Context*)context;
    BSTD_UNUSED(param);
    if(pContext->startSettings.output.transport.dataCallback.callback) {
        pContext->startSettings.output.transport.dataCallback.callback(pContext->startSettings.output.transport.dataCallback.context,
            pContext->startSettings.output.transport.dataCallback.param);
    }
    BKNI_SetEvent(pContext->recpumpEvent);
}

static void BXCode_P_RecpumpOverflow_callback(void *context, int param)
{
    BDBG_WRN(("BXCode%d recpump overflew %s buffer!", param, (const char *)context));
}

static void BXCode_P_WindowMad(
    NEXUS_VideoWindowHandle  hWindow,
    bool bEnable,
    bool bLowDelay)
{
    NEXUS_VideoWindowMadSettings windowMadSettings;

    NEXUS_VideoWindow_GetMadSettings(hWindow, &windowMadSettings);
    windowMadSettings.deinterlace = bEnable;
    windowMadSettings.pqEnhancement = NEXUS_MadPqEnhancement_eAuto;
    windowMadSettings.enable32Pulldown = true;
    windowMadSettings.enable22Pulldown = true;
    windowMadSettings.gameMode = bLowDelay?NEXUS_VideoWindowGameMode_e4Fields_0Delay : NEXUS_VideoWindowGameMode_eOff;
    NEXUS_VideoWindow_SetMadSettings(hWindow, &windowMadSettings);
}

/* xcode window PQ setting */
static void BXCode_P_Window_dnr(NEXUS_VideoWindowHandle  hWindow)
{
    NEXUS_VideoWindowDnrSettings windowDnrSettings;

    NEXUS_VideoWindow_GetDnrSettings(hWindow, &windowDnrSettings);
    windowDnrSettings.mnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
    windowDnrSettings.bnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
    windowDnrSettings.dcr.mode = NEXUS_VideoWindowFilterMode_eEnable;
    windowDnrSettings.mnr.level = 0;
    windowDnrSettings.bnr.level = 0;
    windowDnrSettings.dcr.level = 0;
    windowDnrSettings.qp = 0;
    NEXUS_VideoWindow_SetDnrSettings(hWindow, &windowDnrSettings);
}

/*******************************
 * Add system data to stream_mux
 */
/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
static void BXCode_P_MessageOverflowCallback(void *context, int param)
{
    BXCode_P_Context  *pContext = (BXCode_P_Context  *)context;
    BDBG_WRN(("#### Context%d message PID[%d] buffer overflows! ###", pContext->id, param));
}
static void BXCode_P_MessagePsiCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}
static int BXCode_P_PMT_GetStreamByteOffset( const uint8_t *buf, unsigned bfrSize, int streamNum )
{
    int byteOffset;
    int i;

    /* After the last descriptor */
    byteOffset = BXCODE_P_STREAM_BASE(buf);

    for (i=0; i < streamNum; i++)
    {
        if (byteOffset >= (int)bfrSize || byteOffset >= TS_PSI_MAX_BYTE_OFFSET(buf))
            return -1;
        byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
    }

    return byteOffset;
}

int BXCode_P_AddPsiFilter(BXCode_P_Context *pContext, unsigned short pid, BKNI_EventHandle event)
{
    unsigned i;
    NEXUS_MessageSettings settings;
    NEXUS_MessageStartSettings startSettings;
    NEXUS_Error rc;
    BXCode_P_PsiMessage *msg = NULL;

    for (i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if (!pContext->psi_message[i].message) {
            msg = &pContext->psi_message[i];
            break;
        }
    }
    if (!msg) {
        return -1;
    }
    BDBG_MSG(("adding PSI filter[%u] for PID %u", i, pid));
    if(pContext->playback) {
        msg->pidChannel = NEXUS_Playback_OpenPidChannel(pContext->playback, pid, NULL);
    } else if(pContext->startSettings.input.type == BXCode_InputType_eStream) {
        msg->pidChannel = NEXUS_Playpump_OpenPidChannel(pContext->playpump, pid, NULL);
    } else {/* live */
        msg->pidChannel = NEXUS_PidChannel_Open(pContext->startSettings.input.parserBand, pid, NULL);
    }
    BDBG_ASSERT(msg->pidChannel);

    NEXUS_Message_GetDefaultSettings(&settings);
    settings.dataReady.callback = BXCode_P_MessagePsiCallback;
    settings.dataReady.context = event;
    msg->message = NEXUS_Message_Open(&settings);
    msg->pid = pid;
    msg->done = false;

    NEXUS_Message_GetDefaultStartSettings(msg->message, &startSettings);
    startSettings.pidChannel = msg->pidChannel;
    rc = NEXUS_Message_Start(msg->message, &startSettings);
    BDBG_ASSERT(!rc);

    return 0;
}

/* get TS user data PSI info from input PMT */
static void BXCode_P_GetUserDataPsiFromPmt(BXCode_P_Context *pContext)
{
    BKNI_EventHandle event;
    NEXUS_Error rc;
    unsigned i=0, count = 0, loop = 0, vPid = 0;
    unsigned *userDataPids;
    bool bFound = false;


    /* 1) parse input PAT and set up PMT filters if no specified PMT PID;
     * 2) parse each PMTs to find one with matching video PID;
     * 3) match selected user data stream PID;
     * 4) get user data descriptors;
     */

    if(pContext->playback) {
        /* assume source playback is not started yet, we need to get PSI about user data */
        NEXUS_Playback_Start(pContext->playback, pContext->file, NULL);
    } else if(pContext->startSettings.input.type == BXCode_InputType_eStream) {
        /* assume source playback is not started yet, we need to get PSI about user data */
        NEXUS_Playpump_Start(pContext->playpump);
    }
    vPid = pContext->startSettings.input.vPid;
    userDataPids = pContext->startSettings.input.userdataPid;

    /* to get input PMT, need to set up message filter */
    BKNI_CreateEvent(&event);

    BDBG_WRN(("starting PSI filter PID = %u", 0));

    /* if user specified PMT PID, msg[0] is for the PMT; else msg[0] is for PAT; */
    BXCode_P_AddPsiFilter(pContext, 0, event);

    /* Read the PAT/PMT */
    for (count=0;;count++) {
        const uint8_t *buffer;
        size_t size;
        unsigned programNum, message_length, streamNum;
        size_t num = NEXUS_MAX_MUX_PIDS;

        if (count == NEXUS_MAX_MUX_PIDS) {
            count = 0;
            if(++loop > NEXUS_MAX_MUX_PIDS) {BDBG_ERR(("failed to get input user data PMT!")); rc = -1; break;}
        }

        if (!pContext->psi_message[count].message || pContext->psi_message[count].done) {
            continue;
        }

        rc = NEXUS_Message_GetBuffer(pContext->psi_message[count].message, (const void **)&buffer, &size);
        BDBG_ASSERT(!rc);

        if (!size) {
            BERR_Code rc = BKNI_WaitForEvent(event, 5 * 1000); /* wait 5 seconds */
            if (rc == NEXUS_TIMEOUT) {
                BDBG_WRN(("no PSI messages"));
                rc = -1;
                break;
            }
            BDBG_ASSERT(!rc);
            continue;
        }

        /* We should always get whole PAT's because maxContiguousMessageSize is 4K */
        message_length = TS_PSI_GET_SECTION_LENGTH(buffer) + 3;
        BDBG_ASSERT(size >= (size_t)message_length);
        BDBG_MSG(("message[%u] size = %d, table ID = %u", count, size, buffer[0]));

        if (buffer[0] == 0) {
            /* 1) Program Association Table */
            BDBG_MSG(("PAT: size=%d", message_length));
            for (programNum=0;programNum<(unsigned)(TS_PSI_GET_SECTION_LENGTH(buffer)-7)/4;programNum++) {
                unsigned byteOffset = 8 + programNum*4;
                unsigned program = TS_READ_16( &buffer[byteOffset] );
                unsigned short pid = (uint16_t)(TS_READ_16( &buffer[byteOffset+2] ) & 0x1FFF);
                BDBG_MSG(("  program %d: PID %u", program, pid));
                /* add PMT filters for all programs */
                BXCode_P_AddPsiFilter(pContext, pid, event);
            }
        }
        else if (buffer[0] == 2) { /* PMT */
            TS_PMT_stream pmtStream;

            /* Program Table */
            BDBG_MSG(("Context%d found PMT PID[%u]:\nprogram number %d size=%d", pContext->id,
                pContext->psi_message[count].pid, TS_READ_16(&buffer[3]), message_length));
            /* need to validate the PMT section */
            if(!TS_PMT_validate(buffer, size)) {BDBG_ERR(("invalid PMT")); goto Done_getUserDataPsi;}

            streamNum = TS_PMT_getNumStreams(buffer, size);
            BDBG_MSG(("total streams: %d\n", streamNum));

            /* 2) search for all streams to match the video PID if no specified PMT PID */
            {
                for (i=0;i<streamNum;i++) {
                    TS_PMT_getStream(buffer, size, i, &pmtStream);
                    BDBG_MSG(("\tPID: %d, strem_type: %d", pmtStream.elementary_PID, pmtStream.stream_type));
                    if(pmtStream.elementary_PID == vPid) {
                        BDBG_MSG(("Found matching video PID %u!", vPid));
                        break;
                    }
                }
                if(i == streamNum)  goto Done_getUserDataPsi; /* not found so continue to next PMT */
            }
            bFound = true;/* found PMT */

            /* else, found the matching program */
            /* 3) search for all streams to extract user data PSI info */
            for (i=0;i<streamNum;i++) {
                unsigned streamOffset;

                TS_PMT_getStream(buffer, size, i, &pmtStream);

                /* 2) match user data PID */
                if(pContext->numUserDataPids == BXCODE_P_TS_USER_DATA_ALL) {/* get all */
                    /* all pass the VBI user data PES streams */
                    if(pmtStream.stream_type != TS_PSI_ST_13818_1_PrivatePES) continue;
                    if(num == NEXUS_MAX_MUX_PIDS) num = 0;/* start from 0 */
                    else {
                        ++num;
                        if(num >= NEXUS_MAX_MUX_PIDS) break;
                    }
                    /* STORE the found user data PIDs */
                    userDataPids[num] = pmtStream.elementary_PID;
                } else {/* get requested PIDs */
                    for(num=0; num < pContext->numUserDataPids; num++) {
                        /* 3) bingo! save the stream info and stream descriptor */
                        if(pmtStream.elementary_PID == (uint16_t)userDataPids[num]) {
                            break;
                        }
                    }
                    /* not found, check next stream */
                    if(num == pContext->numUserDataPids) continue;
                }

                /* 4) save user data PSI info */
                BDBG_MSG(("\tuser data PID: %d, strem_type: %d", pmtStream.elementary_PID, pmtStream.stream_type));
                /* save pmt stream info and remap PID */
                pContext->userDataStream[num] = pmtStream;
                if(pContext->startSettings.output.transport.userdataPid[num]) {/* remap PID */
                    pContext->userDataStream[num].elementary_PID = pContext->startSettings.output.transport.userdataPid[num];
                } else {/*pass-through PID */
                    pContext->startSettings.output.transport.userdataPid[num] = pContext->userDataStream[num].elementary_PID;
                }
                /* save stream descriptor size */
                streamOffset = BXCode_P_PMT_GetStreamByteOffset(buffer, size, i);
                pContext->userDataDescLen[num] = TS_READ_16(&buffer[streamOffset+3])&0x3FF;
                BDBG_MSG(("\tdescriptor length: %d", pContext->userDataDescLen[num]));

                /* sanity check descriptor size */
                if(pContext->userDataDescLen[num] > 188) {
                    BDBG_ERR(("User data descriptor length %d too long!"));
                    pContext->userDataStreamValid[num] = false;/* invalidate */
                    goto Done_getUserDataPsi;
                }
                BKNI_Memcpy(pContext->userDataDescriptors[num],&buffer[streamOffset+5],pContext->userDataDescLen[num]);
                /* mark it valid finally */
                pContext->userDataStreamValid[num] = true;
            }
            if(pContext->numUserDataPids == BXCODE_P_TS_USER_DATA_ALL) {
                pContext->numUserDataPids = num+1;/* found num of user data streams: STORE it */
                BDBG_MSG(("Context%d found %d user data PIDs to pass through.", pContext->id, pContext->numUserDataPids));
            }

        }
Done_getUserDataPsi:
        /* XPT HW is configured to pad all messages to 4 bytes. If we are calling NEXUS_Message_ReadComplete
        based on message length and not the size returned by NEXUS_Message_GetBuffer, then we must add that pad.
        If we are wrong, NEXUS_Message_ReadComplete will fail. */
        if (message_length % 4) {
            message_length += 4 - (message_length % 4);
        }
        /* only complete one PMT */
        rc = NEXUS_Message_ReadComplete(pContext->psi_message[count].message, message_length);
        BDBG_ASSERT(!rc);

        pContext->psi_message[count].done = true; /* don't parse this table any more */

        /* Only do once. TODO: may periodically watch for updated PMT info. */
        if(bFound) break;
    }

    for (i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if (pContext->psi_message[i].message) {
            NEXUS_Message_Close(pContext->psi_message[i].message);
            pContext->psi_message[i].message = NULL;
        }
        if (pContext->psi_message[i].pidChannel) {
            if(pContext->playback) {
                NEXUS_Playback_ClosePidChannel(pContext->playback, pContext->psi_message[i].pidChannel);
            } else if(pContext->startSettings.input.type == BXCode_InputType_eStream) {
                NEXUS_Playpump_ClosePidChannel(pContext->playpump, pContext->psi_message[i].pidChannel);
            } else {
                NEXUS_PidChannel_Close(pContext->psi_message[i].pidChannel);
            }
            pContext->psi_message[i].pidChannel = NULL;
        }
    }
    /* free the event resource */
    BKNI_DestroyEvent(event);

    if(pContext->startSettings.input.type == BXCode_InputType_eFile && pContext->openSettings.vpipes<=1) {
        NEXUS_Playback_Stop(pContext->playback);
    } else if(pContext->startSettings.input.type == BXCode_InputType_eStream) {
        NEXUS_Playpump_Stop(pContext->playpump);
    }
}

static void BXCode_P_BuildPsi(
    BXCode_P_Context *pContext,
    uint16_t  pcrPid,
    uint16_t  vidPid,
    uint16_t *audPid,
    uint8_t   vidStreamType,
    uint8_t  *audStreamType,
    unsigned  numAud
)
{
    uint8_t pat_pl_buf[BXCODE_P_TSHEADER_LENGTH], pmt_pl_buf[BXCODE_P_TSHEADER_LENGTH], *pmt;
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    unsigned streamNum;
    unsigned i;

    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;

    TS_PID_info pidInfo;
    TS_PID_state pidState;

    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, BXCODE_P_TSHEADER_LENGTH);

    TS_PAT_program_Init(&program, 1, pContext->startSettings.output.transport.pmtPid);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, BXCODE_P_TSHEADER_LENGTH);

    TS_PMT_setPcrPid(&pmtState, pcrPid);

    if(vidPid) {
        TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    for(i=0; i<numAud; i++) {
        TS_PMT_stream_Init(&pmt_stream, audStreamType[i], audPid[i]);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    /* add user data PID stream PSI info */
    pContext->numUserDataPids = pContext->startSettings.input.numTsUserDataPids;
    if(pContext->numUserDataPids && pContext->openSettings.vpipes<=1) {
        BXCode_P_GetUserDataPsiFromPmt(pContext);
        for(i=0; i<NEXUS_MAX_MUX_PIDS; i++) {
            if(pContext->userDataStreamValid[i]) {
                TS_PMT_addStream(&pmtState, &pContext->userDataStream[i], &streamNum);
                TS_PMT_setDescriptor(&pmtState,
                    pContext->userDataDescriptors[i],
                    pContext->userDataDescLen[i],
                    streamNum);
            }
        }
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);
    BDBG_MSG(("\nContext%d output PMT section:", pContext->id));
    for(i=0; i < (unsigned)pmtState.size; i+=8) {
        BDBG_MODULE_MSG(bxcode_psi, ("%02x %02x %02x %02x %02x %02x %02x %02x", pmtState.buf[i], pmtState.buf[i+1], pmtState.buf[i+2], pmtState.buf[i+3],
            pmtState.buf[i+4], pmtState.buf[i+5], pmtState.buf[i+6], pmtState.buf[i+7]));
    }

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pContext->psiPkt, BXCODE_P_TSHEADER_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pContext->psiPkt + buf_used, pat_pl_buf, pat_pl_size);
    BKNI_Memcpy(pat_pl_buf, (uint8_t*)pContext->psiPkt + 1, BXCODE_P_TSPKT_LENGTH);
    BKNI_Memcpy(pContext->psiPkt, pat_pl_buf, BXCODE_P_TSPKT_LENGTH); /* skip the 1st byte byproduct */

    TS_PID_info_Init(&pidInfo, pContext->startSettings.output.transport.pmtPid, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    pmt = (uint8_t*)pContext->psiPkt+BXCODE_P_TSPKT_LENGTH;
    TS_buildTSHeader(&pidInfo, &pidState, (void*)pmt, BXCODE_P_TSHEADER_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy(pmt + buf_used, pmt_pl_buf, pmt_pl_size);
    BKNI_Memcpy(pmt_pl_buf, pmt + 1, BXCODE_P_TSPKT_LENGTH);
    BKNI_Memcpy(pmt, pmt_pl_buf, BXCODE_P_TSPKT_LENGTH); /* skip the 1st byte byproduct */
    BDBG_MODULE_MSG(bxcode_psi, ("\nContext%d output PMT packet:", pContext->id));
    for(i=0; i < BXCODE_P_TSPKT_LENGTH; i+=8) {
        BDBG_MODULE_MSG(bxcode_psi, ("%02x %02x %02x %02x %02x %02x %02x %02x",
            *(pmt+i), *(pmt+i+1), *(pmt+i+2), *(pmt+i+3), *(pmt+i+4), *(pmt+i+5), *(pmt+i+6), *(pmt+i+7)));
    }

}

/* update CC: for BXCode internal manual PSI insertion (segmented TS output) */
void BXCode_P_UpdateSystemData(BXCode_P_Context *pContext)
{
    uint8_t ccByte, *pat, *pmt;

    pat = (uint8_t*)pContext->psiPkt + (pContext->ccValue % BXCODE_P_PSI_QUEUE_CNT)*BXCODE_P_TSPKT_LENGTH*2;
    pmt = (uint8_t*)pContext->psiPkt + (pContext->ccValue % BXCODE_P_PSI_QUEUE_CNT)*BXCODE_P_TSPKT_LENGTH*2 + BXCODE_P_TSPKT_LENGTH;

    ++pContext->ccValue;/* increment CC synchronously with PAT/PMT */
    ccByte = *(pat + 3); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

    /* need to increment CC value for PAT/PMT packets */
    ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
    *(pat + 3) = ccByte;
    /* need to flush the cache before set to TS mux hw */

    ccByte = *(pmt + 3);
    ccByte = (ccByte & 0xf0) | (pContext->ccValue & 0xf);
    *(pmt + 3) = ccByte;

    BDBG_MODULE_MSG(bxcode_psi, ("Updated PAT&PMT... ccPAT = %x ccPMT=%x", *(pat + 3) & 0xf, *(pmt + 3) & 0xf));
    return;
}

/* with BXCode internal periodic PSI insertion to stream mux (non-segmented TS output) */
static void BXCode_P_InsertSystemDataTimer(BXCode_P_Context *pContext)
{
    uint8_t *pat, *pmt;

    pat = (uint8_t*)pContext->psiPkt + (pContext->ccValue % BXCODE_P_PSI_QUEUE_CNT)*BXCODE_P_TSPKT_LENGTH*2;
    pmt = (uint8_t*)pContext->psiPkt + (pContext->ccValue % BXCODE_P_PSI_QUEUE_CNT)*BXCODE_P_TSPKT_LENGTH*2 + BXCODE_P_TSPKT_LENGTH;

    BXCode_P_UpdateSystemData(pContext);/* update cc byte */

    /* need to flush the cache before set to TS mux hw */
    NEXUS_Memory_FlushCache((void*)(pat + 3), 1);
    /* ping pong PAT pointer */
    pContext->psi[0].pData = (void*)(pat);

    NEXUS_Memory_FlushCache((void*)(pmt + 3), 1);
    /* ping pong PMT pointer */
    pContext->psi[1].pData = (void*)(pmt);

    NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[0]);
    NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[1]);
    BDBG_MODULE_MSG(bxcode_psi, ("Context%d inserted PAT&PMT... ccPAT = %x ccPMT=%x", pContext->id, *(pat + 3) & 0xf,
        *(pmt + 3) & 0xf));
    if(pContext->systemdataTimerIsStarted)
    {
        pContext->systemdataTimer = B_Scheduler_StartTimer(
            pContext->schedulerSystemdata,pContext->mutexSystemdata,
            pContext->startSettings.output.transport.intervalPsi,
            (B_TimerCallback)BXCode_P_InsertSystemDataTimer,
            pContext);
        if(pContext->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error %d", NEXUS_OUT_OF_SYSTEM_MEMORY));}
    }
    return;
}

static void BXCode_P_CreateSystemData( BXCode_P_Context  *pContext )
{
    uint8_t vidStreamType=0, audStreamType[BXCODE_MAX_AUDIO_PIDS];
    uint16_t audPid[BXCODE_MAX_AUDIO_PIDS];
    unsigned i, numAudPids=0;
    NEXUS_AudioCodec audCodec = NEXUS_AudioCodec_eUnknown;

    NEXUS_Memory_Allocate(BXCODE_P_TSHEADER_LENGTH*2*BXCODE_P_PSI_QUEUE_CNT, NULL, &pContext->psiPkt);

    /* decide the stream type to set in PMT */
    if(pContext->startSettings.output.video.pid) {
        switch(pContext->startSettings.output.video.encoder.codec)
        {
            case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
            case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
            case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
            case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
            default:
                BDBG_ERR(("Video encoder codec %d is not supported!\n", pContext->startSettings.output.video.encoder.codec));
                BDBG_ASSERT(0);
        }
    }
    for(i=0; i<BXCODE_MAX_AUDIO_PIDS && pContext->startSettings.output.audio[i].pid; i++)
    {
        if(!pContext->startSettings.output.audio[i].passthrough)
        {/* audio transcode */
            audCodec = pContext->startSettings.output.audio[i].codec;
        }
        else
        {/* audio passthrough */
            switch(pContext->startSettings.input.type) {
            case BXCode_InputType_eFile  :
            case BXCode_InputType_eStream:
            case BXCode_InputType_eLive  : audCodec = pContext->startSettings.input.aCodec[i]; break;
            default: BDBG_ERR(("BXCode%u input type %d no audio passthrough support.", pContext->id, pContext->startSettings.input.type)); break;
            }
        }
        audPid[numAudPids] = pContext->startSettings.output.audio[i].pid;
        switch(audCodec) {
            case NEXUS_AudioCodec_eMpeg:         audStreamType[numAudPids] = 0x4; break;
            case NEXUS_AudioCodec_eMp3:          audStreamType[numAudPids] = 0x4; break;
            case NEXUS_AudioCodec_eAac    :      audStreamType[numAudPids] = 0xf; break; /* ADTS */
            case NEXUS_AudioCodec_eAacPlus:      audStreamType[numAudPids] = 0x11; break;/* LOAS */
            /* MP2TS doesn't allow 14496-3 AAC+ADTS; here is placeholder to test AAC-HE before LOAS encode is supported; */
            case NEXUS_AudioCodec_eAacPlusAdts:  audStreamType[numAudPids] = 0x11; break;
            case NEXUS_AudioCodec_eAc3:          audStreamType[numAudPids] = 0x81; break;
            case NEXUS_AudioCodec_eLpcm1394:     audStreamType[numAudPids] = 0x83; break;
            default:
                BDBG_ERR(("Audio encoder codec %d is not supported!\n", audCodec));
        }
        /* count number of audio PIDs input output */
        numAudPids++;
    }
    BXCode_P_BuildPsi(pContext,
        pContext->startSettings.output.transport.pcrPid,
        pContext->startSettings.output.video.pid,
        audPid, vidStreamType, audStreamType, numAudPids);
    for(i=0; i<BXCODE_P_PSI_QUEUE_CNT; i++)
    {
        if(i > 0)
        {
            /* copy PAT */
            BKNI_Memcpy((uint8_t*)pContext->psiPkt + i*2*BXCODE_P_TSPKT_LENGTH, pContext->psiPkt, BXCODE_P_TSPKT_LENGTH);
            /* copy PMT */
            BKNI_Memcpy((uint8_t*)pContext->psiPkt + i*2*BXCODE_P_TSPKT_LENGTH + BXCODE_P_TSPKT_LENGTH,
                (uint8_t*)pContext->psiPkt + BXCODE_P_TSPKT_LENGTH, BXCODE_P_TSPKT_LENGTH);
        }
    }
    NEXUS_Memory_FlushCache(pContext->psiPkt, BXCODE_P_TSPKT_LENGTH*2*BXCODE_P_PSI_QUEUE_CNT);
    BKNI_Memset(pContext->psi, 0, sizeof(pContext->psi));
    pContext->psi[0].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pContext->psi[0].pData = pContext->psiPkt;
    pContext->psi[0].timestampDelta = 0;
    pContext->psi[1].size = 188;
    /* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
    pContext->psi[1].pData = (void*)((uint8_t*)pContext->psiPkt + BXCODE_P_TSPKT_LENGTH);/* discard byte 0 which is byproduct of BuldPsi */
    pContext->psi[1].timestampDelta = 0;
    if(!pContext->startSettings.output.transport.segmented) {/* non-segmented ts output use timer-based psi insertion */
        NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[0]);
        NEXUS_StreamMux_AddSystemDataBuffer(pContext->streamMux, &pContext->psi[1]);
        BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pContext->psiPkt + 3) & 0xf,
            *((uint8_t*)pContext->psiPkt + 3 + BXCODE_P_TSPKT_LENGTH) & 0xf));
    }
}

static void BXCode_P_StartSystemdata( BXCode_P_Context *pContext )
{
    if(!pContext->systemdataTimerIsStarted) {
        /* schedule a periodic timer to insert PAT/PMT */
        B_ThreadSettings settingsThread;
        pContext->mutexSystemdata = B_Mutex_Create(NULL);
        pContext->schedulerSystemdata = B_Scheduler_Create(NULL);
        /* create thread to run scheduler */
        B_Thread_GetDefaultSettings(&settingsThread);
        pContext->schedulerThread = B_Thread_Create("systemdata_Scheduler",
            (B_ThreadFunc)B_Scheduler_Run,
            pContext->schedulerSystemdata,
            &settingsThread);
        if (NULL == pContext->schedulerThread)
        {
            BDBG_ERR(("failed to create scheduler thread"));
        }
        pContext->systemdataTimer = B_Scheduler_StartTimer(
            pContext->schedulerSystemdata,pContext->mutexSystemdata,
            pContext->startSettings.output.transport.intervalPsi,
            (B_TimerCallback)BXCode_P_InsertSystemDataTimer,
            pContext);
        if(pContext->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error"));}
        pContext->systemdataTimerIsStarted = true;
    }
}

static void BXCode_P_StopSystemdata( BXCode_P_Context  *pContext )
{
    /* cancel system data timer */
    if(pContext->systemdataTimerIsStarted)
    {
        BDBG_MSG(("bxcode[%u] to stop TS mux system data", pContext->id));
        B_Scheduler_CancelTimer(pContext->schedulerSystemdata, pContext->systemdataTimer);
        B_Scheduler_Stop(pContext->schedulerSystemdata);
        B_Scheduler_Destroy(pContext->schedulerSystemdata);
        if (pContext->schedulerThread)
        {
            B_Thread_Destroy(pContext->schedulerThread);
            pContext->schedulerThread = NULL;
        }
        B_Mutex_Destroy(pContext->mutexSystemdata);
        pContext->systemdataTimer = NULL;
        pContext->systemdataTimerIsStarted = false;
    }
}

static void BXCode_P_DestroySystemData( BXCode_P_Context  *pContext )
{
    NEXUS_Memory_Free(pContext->psiPkt);
}

/* This thread records transcoded segmented TS stream into .ts file and the intended HLS segments info into txt index file */
static void BXCode_P_Recpump_thread(
    BXCode_P_Context  *pContext )
{
    unsigned segment=0;
    unsigned timeout=0;
    bool firstTime=true;
    off_t totalRecordBytes=0;
    off_t bytesRecordedTillCurrentRai=0;
    NEXUS_Error rc;
    double duration;

    /* record the segment #, segment byte offset in TS file, and segment duration in ms */
    fprintf(pContext->fpIndexRec, "segment,fileOffset,duration(sec)\n");
    write(pContext->fdTsRec, pContext->psiPkt, 188*2);
    BXCode_P_UpdateSystemData(pContext);

    /* get segment duration */
    if(pContext->settings.video.encoder.streamStructure.duration) {
        duration = (double)pContext->settings.video.encoder.streamStructure.duration/1000;
    } else {
        double framerate = BXCode_P_GetRefreshRateFromFrameRate(pContext->settings.video.encoder.frameRate);
        if(pContext->settings.video.encoder.streamStructure.openGop) {
            duration = (pContext->settings.video.encoder.streamStructure.framesP + 1)*
                (pContext->settings.video.encoder.streamStructure.framesB + 1)/framerate;
        } else {
            duration = (1 + pContext->settings.video.encoder.streamStructure.framesP *
                (pContext->settings.video.encoder.streamStructure.framesB + 1))/framerate;
        }
    }

    while((timeout < 3000) && (!pContext->toStop)) {
        const void *buffer, *indexBuf;
        size_t datasize=0, indexsize=0;

        /* record CDB data */
        rc = NEXUS_Recpump_GetDataBuffer(pContext->recpump, &buffer, &datasize);
        if(rc) break;

        /* ITB contains TPIT results */
        rc = NEXUS_Recpump_GetIndexBuffer(pContext->recpump, &indexBuf, &indexsize);
        if(rc) break;

        #define BRCM_TPIT_ENTRY_SIZE 24 /* HLS NOTE: Each TPIT entry is 6 dwords: 24 bytes */

        if (indexsize) {/* assume one RAI index per segment */
            off_t highByte;

            BDBG_MSG(("indexsize = %u", indexsize));
            /* byte offset since the start of the record session */
            highByte = ((off_t)*((uint32_t*)indexBuf+2) >> 24);
            bytesRecordedTillCurrentRai = highByte << 32;
            bytesRecordedTillCurrentRai |= (off_t)*((uint32_t*)indexBuf+3);
            BDBG_MSG(("Seg[%u] byte offset: %llu", segment, bytesRecordedTillCurrentRai));
            BDBG_MSG(("idxSize=%u", indexsize));
            /* log the segment info */
            /* increment segment counter: assume one RAI per segment. */
            fprintf(pContext->fpIndexRec, "%u,", segment++);
            highByte = (bytesRecordedTillCurrentRai+segment*188*2)>>32;

            fprintf(pContext->fpIndexRec, "0x%x%08x,%.1f\n", (uint32_t)highByte,
                (uint32_t)(bytesRecordedTillCurrentRai+segment*188*2),
                duration);

            /* HLS NOTE: update duration fifo reader: assume one RAI per segment */
#define BTST_TPIT_DEBUG
#ifdef BTST_TPIT_DEBUG
            BDBG_MSG(("RAI tpit entry: index bytesRead %d, tipt[0] 0x%x, tpit[2] 0x%x, tpit[3] 0x%x", *(uint32_t*)indexBuf, *((uint32_t*)indexBuf+2), *((uint32_t*)indexBuf+3)));
#endif
            indexBuf = (uint8_t *)indexBuf + BRCM_TPIT_ENTRY_SIZE;/* consume one index */
            rc = NEXUS_Recpump_IndexReadComplete(pContext->recpump, BRCM_TPIT_ENTRY_SIZE);
            BDBG_ASSERT(!rc);
        }

        /* record the segment TS stream; also insert PAT&PMT at start of each segments */
        if (datasize) {
            /* if dataSize + totalRecordBytes < bytesRecordedTillCurrentRai or no new index, write dataSize of current segment;
                else, write up to bytesRecordedTillCurrentRai, then PAT/PMT for the next segment; */
            if((0==indexsize) || (datasize + totalRecordBytes < bytesRecordedTillCurrentRai)) {
                write(pContext->fdTsRec, buffer, datasize);
                BDBG_MSG(("1) Wrote %u data.", datasize));
            } else {/* one or more indices */
                /* record rest of the previous segment before the new RAI packet and increment buffer pointer */
                write(pContext->fdTsRec, buffer, bytesRecordedTillCurrentRai - totalRecordBytes);
                buffer = (uint8_t*)buffer + bytesRecordedTillCurrentRai - totalRecordBytes;
                BDBG_MSG(("2) Wrote %llu data (%llu, %llu).", bytesRecordedTillCurrentRai - totalRecordBytes,
                    bytesRecordedTillCurrentRai, totalRecordBytes));

                if(firstTime) {/* first pat/pmt already written previously for the 1st segment */
                    firstTime = false;
                } else {
                    /* insert the PAT & PMT, then update continuity counter */
                    write(pContext->fdTsRec, pContext->psiPkt, 188*2);
                    BXCode_P_UpdateSystemData(pContext);/* update cc value of psi */
                    BDBG_MSG(("wrote pat and pmt for segment %u", segment-1));
                }
                /* update the data read complete size */
                datasize = bytesRecordedTillCurrentRai - totalRecordBytes;
            }
            rc = NEXUS_Recpump_DataReadComplete(pContext->recpump, datasize);
            totalRecordBytes += datasize;
            BDBG_ASSERT(!rc);
        }
#define BTST_CHUNK_TIMEOUT_INTERVAL   250 /* 250ms */
        if (!datasize && !indexsize) {
            if(BKNI_WaitForEvent(pContext->recpumpEvent, BTST_CHUNK_TIMEOUT_INTERVAL) == BERR_TIMEOUT) {
                timeout+=BTST_CHUNK_TIMEOUT_INTERVAL;
            }
            continue;
        } else {
            timeout = 0;
        }
    }

    BDBG_MSG(("recpump thread is stopped!"));
    /* file2file transcode may wait for EOF done callback */
    if(pContext->startSettings.input.type == BXCode_InputType_eFile &&
       pContext->startSettings.input.eofDone.callback) {
        pContext->startSettings.input.eofDone.callback(pContext->startSettings.input.eofDone.context,
            pContext->startSettings.input.eofDone.param);
    }
}

/* video chunk xcoder playpump feeder. This is for FNRT file2file or segmented TS output.
 */
static void BXCode_P_VideoPlaypump_thread(
    BXCode_P_Video  *pContext )
{
    typedef enum BTST_P_ChunkState {
        BTST_P_ChunkState_eGetNextChunk=0,
        BTST_P_ChunkState_eStart,
        BTST_P_ChunkState_eFeed,
        BTST_P_ChunkState_eToEnd,
        BTST_P_ChunkState_eWaitForDone,
        BTST_P_ChunkState_eEOF,
        BTST_P_ChunkState_eDone
    } BTST_P_ChunkState;
    BXCode_P_Context *bxcode = g_BXCode_P_State.handles[pContext->bxcodeId];

    /* while loop until EOF or quit */
    {
        BTST_P_ChunkState chunkState = BTST_P_ChunkState_eGetNextChunk;
        long startRapIndex;

        BDBG_MODULE_MSG(chunk_state, ("[%u] START --> eGetNextChunk", pContext->id));

        /* 2) while loop for chunk feed */
        while (chunkState != BTST_P_ChunkState_eDone) {
            void *buffer;
            size_t buffer_size;
            int n, playSize=0;
            NEXUS_Error rc;

            if (NEXUS_Playpump_GetBuffer(pContext->playpump, &buffer, &buffer_size)) {
               break;
            }
            if (buffer_size < ((chunkState==BTST_P_ChunkState_eEOF)? 3*188 : 188)) {
                BKNI_WaitForEvent(pContext->dataReadyEvent, 1000);
                continue;
            }

            /* The first call to get_buffer will return the entire playback buffer.
               If we use it, we're going to have to wait until the descriptor is complete,
               and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
            if (buffer_size > MAX_READ)
                buffer_size = MAX_READ;

            if(bxcode->toStop) { chunkState = BTST_P_ChunkState_eEOF; }
            switch(chunkState) {
               case BTST_P_ChunkState_eGetNextChunk:
                  /* if reached end of index file by another thread, stop this thread now */
                  B_Mutex_Lock(bxcode->mutexChunk);
                  startRapIndex = bxcode->latestVideoChunk.startRapIndex;
                  B_Mutex_Unlock(bxcode->mutexChunk);
                  if(startRapIndex == -1)
                  {
                     chunkState = BTST_P_ChunkState_eEOF;
                     BDBG_MODULE_MSG(chunk_state, ("[%u] eGetNextChunk --> eEOF", pContext->id));
                  }
                  else
                  {
                     /* 1) Seek the next chunk to start with; */
                     BXCode_P_SeekNextChunk(bxcode, &pContext->chunk);
                     pContext->fileOffset = pContext->chunk.startOffset;
                     BDBG_MODULE_MSG(chunk_state, ("Video[%d] to start chunk[%d] from %llu to %llu", pContext->id, pContext->chunk.id, pContext->chunk.startOffset, pContext->chunk.endOffset));
                     lseek(pContext->fd, pContext->chunk.startOffset, SEEK_SET);
                     chunkState = BTST_P_ChunkState_eStart;
                     BDBG_MODULE_MSG(chunk_state, ("[%u] eGetNextChunk --> eStart", pContext->id));

                     /* FNRT: flush playpump, rave and decoder to restart for next chunk */
                     if(bxcode->openSettings.vpipes > 1) {
                         NEXUS_VideoDecoder_Stop(pContext->decoder);/* this flush RAVE and AVD buffers */
                         NEXUS_VideoDecoder_Start(pContext->decoder, &pContext->vidProgram);
                     }
                  }
                  continue;
            /* insert start of chunk BTP */
            case BTST_P_ChunkState_eStart:
                BXCode_P_InsertBTP(bxcode, buffer, pContext->chunk.id, bxcode->startSettings.input.vPid, BXCode_P_BTPCommand_eChunkId);
                buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize = 188;
                chunkState = BTST_P_ChunkState_eFeed;
                BDBG_MODULE_MSG(chunk_state, ("[%u] eStart --> eFeed", pContext->id));
                break;
            /* feed until end of chunk */
            case BTST_P_ChunkState_eFeed:
                if ((buffer_size + pContext->fileOffset > pContext->chunk.endOffset) && (pContext->chunk.endOffset != -1)) {
                    buffer_size = pContext->chunk.endOffset - pContext->fileOffset;
                    chunkState = BTST_P_ChunkState_eToEnd;
                    BDBG_MODULE_MSG(chunk_state, ("[%u] eFeed --> eToEnd", pContext->id));
                }
                break;
            /* insert end of chunk BTP */
            case BTST_P_ChunkState_eToEnd:/* end with current chunkID+1 */
                BXCode_P_InsertBTP(bxcode, buffer, pContext->chunk.id + 1, bxcode->startSettings.input.vPid, BXCode_P_BTPCommand_eChunkId);
                buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize = 188;
                 /* FNRT: continue feed and wait for chunk done callback before restart of next chunk to allow chunk overlap among video threads */
                 if(bxcode->openSettings.vpipes > 1) {
                    chunkState = BTST_P_ChunkState_eWaitForDone;
                    BDBG_MODULE_MSG(chunk_state, ("[%u] eToEnd --> eWaitForDone", pContext->id));
                } else {
                    chunkState = BTST_P_ChunkState_eGetNextChunk;
                    BDBG_MODULE_MSG(chunk_state, ("[%u] eToEnd --> eGetNextChunk", pContext->id));
                }
                break;
            /* FNRT: continue to feed until ChunkDone callback if don't know exactly when to stop current chunk */
            case BTST_P_ChunkState_eWaitForDone:
                /* test chunkDone; if not, continue chunk feed, otherwise flush and start next chunk. */
                if(BKNI_WaitForEvent(pContext->chunkDoneEvent,1) == BERR_SUCCESS) {
                    BDBG_MSG(("Video xcoder[%d] chunk DONE!", pContext->id));
                    NEXUS_Playpump_Flush(pContext->playpump);
                    chunkState = BTST_P_ChunkState_eGetNextChunk;
                    BDBG_MODULE_MSG(chunk_state, ("[%u] eWaitForDone --> eGetNextChunk", pContext->id));
                    continue;
                }
                break;
            case BTST_P_ChunkState_eEOF:
               BXCode_P_InsertBTP(bxcode, buffer, 0, bxcode->startSettings.input.vPid, BXCode_P_BTPCommand_eEOS);
               buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize += 188;
               BXCode_P_InsertBTP(bxcode, buffer, 0, bxcode->startSettings.input.vPid, BXCode_P_BTPCommand_eLast);
               buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize += 188;
               BXCode_P_InsertBTP(bxcode, buffer, 0, bxcode->startSettings.input.vPid, BXCode_P_BTPCommand_eEOS);
               buffer = (uint8_t*)buffer + 188; buffer_size -= 188; playSize += 188;
               rc = NEXUS_Playpump_WriteComplete(pContext->playpump, 0, playSize);
               chunkState = BTST_P_ChunkState_eDone;
               BDBG_MODULE_MSG(chunk_state, ("[%u] eEOF --> eDone", pContext->id));
               continue;

            case BTST_P_ChunkState_eDone:
               continue;
            default:/* eDone */
                break;
            }

            n = read(pContext->fd, buffer, buffer_size);
            if (n < 0) BDBG_ASSERT(0);
            playSize += n;
            pContext->fileOffset += n;/* update file offset */
            if (n == 0 && buffer_size) {
               chunkState = BTST_P_ChunkState_eEOF;
               BDBG_MODULE_MSG(chunk_state, ("[%u] eEOF", pContext->id));
               BDBG_MSG(("Video[%u] to end playfile...", pContext->id));
            }
            rc = NEXUS_Playpump_WriteComplete(pContext->playpump, 0, playSize);
            BDBG_ASSERT(!rc);
            BDBG_MODULE_MSG(video_playpump, ("V[%d] played %d bytes\n", pContext->id, n));
        }
    }

    /* decrement count */
    B_Mutex_Lock(bxcode->mutexActiveXcoders);
    bxcode->activeXcoders--;
    BDBG_MSG(("Video playpump[%d] is stopped. Active video pumps count = %d", pContext->id, bxcode->activeXcoders));
    if(bxcode->startSettings.input.eofDone.callback && bxcode->activeXcoders==0) {
        bxcode->startSettings.input.eofDone.callback(bxcode->startSettings.input.eofDone.context,
            bxcode->startSettings.input.eofDone.param);
    }
    B_Mutex_Unlock(bxcode->mutexActiveXcoders);
}

/* audio xcoder playpump feeder for FNRT file2file transcode */
static void BXCode_P_AudioPlaypump_thread(
    BXCode_P_Audio  *pContext )
{
    bool endOfFile = false;
    BXCode_P_Context *bxcode = g_BXCode_P_State.handles[pContext->id];

    /* while loop until EOF or quit */
    while (!bxcode->toStop) {
        void *buffer;
        size_t buffer_size;
        int n;
        NEXUS_Error rc;

        if (NEXUS_Playpump_GetBuffer(pContext->playpump, &buffer, &buffer_size))
        break;
        if (buffer_size < 3*188) {
            BKNI_WaitForEvent(pContext->dataReadyEvent, 1000);
            continue;
        }

        /* insert LAST BTP */
        if(endOfFile) {
            BXCode_P_InsertBTP(bxcode, buffer, 0, bxcode->startSettings.input.aPid[0], BXCode_P_BTPCommand_eEOS);
            BXCode_P_InsertBTP(bxcode, (uint8_t*)buffer + 188, 0, bxcode->startSettings.input.aPid[0], BXCode_P_BTPCommand_eLast);
            BXCode_P_InsertBTP(bxcode, (uint8_t*)buffer + 2*188, 0, bxcode->startSettings.input.aPid[0], BXCode_P_BTPCommand_eEOS);
            rc = NEXUS_Playpump_WriteComplete(pContext->playpump, 0, 3*188);
            BDBG_ASSERT(!rc);
            break;
        }

        /* The first call to get_buffer will return the entire playback buffer.
           If we use it, we're going to have to wait until the descriptor is complete,
           and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;

        n = read(pContext->fd, buffer, buffer_size);
        if (n < 0) BDBG_ASSERT(0);
        if (n == 0 && buffer_size) {
            endOfFile = true;
            BDBG_MSG(("Audio[%u] to end playfile...", pContext->id));
        }
        rc = NEXUS_Playpump_WriteComplete(pContext->playpump, 0, n);
        BDBG_ASSERT(!rc);
        BDBG_MODULE_MSG(audio_playpump, ("A[%d] played %d bytes\n", pContext->id, n));
    }

    /* decrement count */
    B_Mutex_Lock(bxcode->mutexActiveXcoders);
    bxcode->activeXcoders--;
    BDBG_MSG(("Audio playpump[%d] is stopped. Active audio pump count = %d",
        pContext->id, bxcode->activeXcoders));

    /* fire EOF callback if all playpump threads finishes */
    if(bxcode->startSettings.input.eofDone.callback && bxcode->activeXcoders==0) {
        bxcode->startSettings.input.eofDone.callback(bxcode->startSettings.input.eofDone.context,
            bxcode->startSettings.input.eofDone.param);
    }
    B_Mutex_Unlock(bxcode->mutexActiveXcoders);
}

NEXUS_Error BXCode_P_SetVideoSettings(
    BXCode_P_Context      *bxcode,
    unsigned               i,
    const BXCode_Settings *pSettings
)
{
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_Error rc=NEXUS_SUCCESS;
    BXCode_P_Video *pContext = &bxcode->video[i];

#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
    customFormatSettings.width = pSettings->video.width;
    customFormatSettings.height = pSettings->video.height;
    customFormatSettings.interlaced = bxcode->startSettings.output.video.encoder.interlaced;
    if(pSettings->video.refreshRate) {
        customFormatSettings.refreshRate = pSettings->video.refreshRate;
    } else {
        switch(pSettings->video.encoder.frameRate) {
        case NEXUS_VideoFrameRate_e50:
        case NEXUS_VideoFrameRate_e25:
        case NEXUS_VideoFrameRate_e12_5:
            customFormatSettings.refreshRate = 50000;
            break;
        case NEXUS_VideoFrameRate_e29_97:
        case NEXUS_VideoFrameRate_e30:
        case NEXUS_VideoFrameRate_e59_94:
        case NEXUS_VideoFrameRate_e60:
        case NEXUS_VideoFrameRate_e24:
        case NEXUS_VideoFrameRate_e23_976:
        case NEXUS_VideoFrameRate_e15:
        case NEXUS_VideoFrameRate_e14_985:
        case NEXUS_VideoFrameRate_e20:
        case NEXUS_VideoFrameRate_e10:
        default:
            customFormatSettings.refreshRate = 60000;
            break;
        }
    }
    customFormatSettings.aspectRatio = pSettings->video.aspectRatio; /* don't care */
    customFormatSettings.sampleAspectRatio.x = pSettings->video.sampleAspectRatio.x;
    customFormatSettings.sampleAspectRatio.y = pSettings->video.sampleAspectRatio.y;
    customFormatSettings.dropFrameAllowed = true;
    rc = NEXUS_Display_SetCustomFormatSettings(pContext->display, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
    if(NEXUS_SUCCESS != rc) { BDBG_ERR(("Video[%u] failed to set format", pContext->id)); return BERR_TRACE(rc); }

    /* set 3D orientation type */
    NEXUS_Display_GetSettings(pContext->display, &displaySettings);
    displaySettings.display3DSettings.overrideOrientation = true;
    displaySettings.display3DSettings.orientation         = pSettings->video.orientation;
    rc = NEXUS_Display_SetSettings(pContext->display, &displaySettings);
    if(NEXUS_SUCCESS != rc) { BDBG_ERR(("Video[%u] failed to set orientation %d", pContext->id, pSettings->video.orientation)); return BERR_TRACE(rc); }

    /* update window settings: only handle contentMode for now; TODO: add other settings. */
    NEXUS_VideoWindow_GetSettings(pContext->window, &windowSettings);
    windowSettings.contentMode = pSettings->video.windowSettings.contentMode;
    NEXUS_VideoWindow_SetSettings(pContext->window, &windowSettings);
    bxcode->settings.video.windowSettings = windowSettings;/* save it */

    /* update gfx settings. */
    NEXUS_Display_SetGraphicsSettings(pContext->display, &pSettings->video.gfxSettings);
    if(pSettings->video.gfxSettings.enabled) {
        NEXUS_Display_SetGraphicsFramebuffer(pContext->display, pSettings->video.frameBuffer);
    }
#else /* DSP encoder */
    BSTD_UNUSED(customFormatSettings);
    NEXUS_Display_GetSettings(pContext->display, &displaySettings);
    switch(pSettings->video.encoder.frameRate) {
    case NEXUS_VideoFrameRate_e50:
    case NEXUS_VideoFrameRate_e25:
    case NEXUS_VideoFrameRate_e12_5:
        if(pSettings->video.height <= 576) {
            displaySettings.format = NEXUS_VideoFormat_ePal;
        } else {
            displaySettings.format = NEXUS_VideoFormat_e720p50hz;
        }
        break;
    case NEXUS_VideoFrameRate_e29_97:
    case NEXUS_VideoFrameRate_e30:
    case NEXUS_VideoFrameRate_e59_94:
    case NEXUS_VideoFrameRate_e60:
    case NEXUS_VideoFrameRate_e24:
    case NEXUS_VideoFrameRate_e23_976:
    case NEXUS_VideoFrameRate_e15:
    case NEXUS_VideoFrameRate_e14_985:
    case NEXUS_VideoFrameRate_e20:
    case NEXUS_VideoFrameRate_e10:
    default:
        if(pSettings->video.height <= 480) {
            displaySettings.format = NEXUS_VideoFormat_eNtsc;
        } else {
            displaySettings.format = NEXUS_VideoFormat_e720p;
        }
        break;
    }
    rc = NEXUS_Display_SetSettings(pContext->display, &displaySettings);
    if(NEXUS_SUCCESS != rc) { BDBG_ERR(("Video[%u] failed to set format", pContext->id)); return BERR_TRACE(rc); }

    bxcode->startSettings.output.video.encoder.interlaced = false; /* forced progressive soft encode for now due to DSP fw limitation; TODO: add interlaced. */
    /* NOTE: DSP encoder currently doesn't support dynamic resolution and encode size is set here; TODO: add dynamic resolution */
    bxcode->startSettings.output.video.encoder.bounds.inputDimension.max.width = pSettings->video.width;
    bxcode->startSettings.output.video.encoder.bounds.inputDimension.max.height = pSettings->video.height;

    /* set DSP encoder bvn capture buffer size; */
    NEXUS_VideoWindow_GetSettings(pContext->window, &windowSettings);
    windowSettings.position.width = pSettings->video.width;
    windowSettings.position.height = pSettings->video.height;

    /* cannot exceed maximum size */
    windowSettings.position.width = pSettings->video.width;
    windowSettings.position.height = pSettings->video.height;
    rc = NEXUS_VideoWindow_SetSettings(pContext->window, &windowSettings);
    if(NEXUS_SUCCESS != rc) { BDBG_ERR(("Video[%u] failed to set resolution", pContext->id)); return BERR_TRACE(rc); }
#endif
    return rc;
}

NEXUS_Error BXCode_P_SetAudioSettings(
    BXCode_P_Context      *bxcode,
    unsigned               i,
    const BXCode_Settings *pSettings
)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    NEXUS_AudioEncoderCodecSettings codecSettings;

    NEXUS_AudioEncoder_GetCodecSettings(bxcode->audio[i].encoder, bxcode->startSettings.output.audio[i].codec, &codecSettings);
    switch(bxcode->startSettings.output.audio[i].codec) {
    case NEXUS_AudioCodec_eAac:
        if(pSettings->audio[i].codec.codecSettings.aac.bitRate) codecSettings.codecSettings.aac.bitRate       = pSettings->audio[i].codec.codecSettings.aac.bitRate;
        if(pSettings->audio[i].codec.codecSettings.aac.sampleRate) codecSettings.codecSettings.aac.sampleRate = pSettings->audio[i].codec.codecSettings.aac.sampleRate;
        break;
    case NEXUS_AudioCodec_eAacPlus:
    case NEXUS_AudioCodec_eAacPlusAdts:
        if(pSettings->audio[i].codec.codecSettings.aacPlus.bitRate) codecSettings.codecSettings.aacPlus.bitRate       = pSettings->audio[i].codec.codecSettings.aacPlus.bitRate;
        if(pSettings->audio[i].codec.codecSettings.aacPlus.sampleRate) codecSettings.codecSettings.aacPlus.sampleRate = pSettings->audio[i].codec.codecSettings.aacPlus.sampleRate;
        break;
    case NEXUS_AudioCodec_eMp3:
        if(pSettings->audio[i].codec.codecSettings.mp3.bitRate) codecSettings.codecSettings.mp3.bitRate       = pSettings->audio[i].codec.codecSettings.mp3.bitRate;
        break;
    default:
        BDBG_ERR(("audio[%u] unsupported audio encoder codec %d!", i, bxcode->startSettings.output.audio[i].codec));
        break;
    }
    rc = NEXUS_AudioEncoder_SetCodecSettings(bxcode->audio[i].encoder, &codecSettings);
    if(NEXUS_SUCCESS != rc) { BDBG_ERR(("failed to set audio[%u] codec settings", i)); return BERR_TRACE(rc); }
    return rc;
}

static NEXUS_Error bxcode_p_open_video_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i)
{
    NEXUS_VideoEncoderCapabilities encoderCap;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindowScalerSettings scalerSettings;
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_DisplayCustomFormatSettings customFormatSettings;
#endif
    NEXUS_Error rc;
    BXCode_P_Video *pContext = &bxcode->video[i];

    /* bring up decoder and connect to local display */
    {
        NEXUS_VideoDecoderOpenSettings openSettings;
        NEXUS_VideoDecoderCapabilities cap;
        NEXUS_GetVideoDecoderCapabilities(&cap);
        NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
        openSettings.avc51Enabled = false;
        openSettings.svc3dSupported = false;
        openSettings.excessDirModeEnabled = false;
        openSettings.enhancementPidChannelSupported = false;

        /* video decoder assignment from top down */
        pContext->decoderId = (bxcode->openSettings.videoDecoderId == NEXUS_ANY_ID)?
            (cap.numVideoDecoders-1) : bxcode->openSettings.videoDecoderId;
        do {
           /* if decoder ID already in "opened list", then skip this decoder */
         if ((g_BXCode_P_State.openVideoDecoders & (1 << pContext->decoderId)) != 0) continue;
            pContext->decoder = NEXUS_VideoDecoder_Open(pContext->decoderId, &openSettings); /* take default capabilities */
        } while (!pContext->decoder && --pContext->decoderId != (unsigned int)-1);
        if(NULL==pContext->decoder) {
            BDBG_ERR(("BXCode%u video pipe%u failed to open video decoder %d!", bxcode->id, i, pContext->decoderId));
            return NEXUS_NOT_AVAILABLE;
        }
        BDBG_MSG(("BXCode%d opened video decoder %d.", bxcode->id, pContext->decoderId));
        g_BXCode_P_State.openVideoDecoders |= (1 << pContext->decoderId);
    }
    /****************************************
     * Bring up video display and encoder
     */

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
    * which might cause encoder display GISB error since encoder display would
    * trigger RDC to program mailbox registers in ViCE2;
    */
    /* TODO: increase encoder output CDB/ITB size to hold the chunk transcoder output while waiting for serial MUX */
    if(bxcode->openSettings.videoEncoder.data.fifoSize == 0) {
        NEXUS_VideoEncoder_GetDefaultOpenSettings(&bxcode->openSettings.videoEncoder);
    }
    pContext->id = (bxcode->openSettings.vpipes>1)? i : bxcode->id;
    pContext->encoder = NEXUS_VideoEncoder_Open(pContext->id, &bxcode->openSettings.videoEncoder);
    BDBG_ASSERT(pContext->encoder);
    BDBG_MSG(("BXCode%u video pipe%u opened video encoder %u.", bxcode->id, i, pContext->id));

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);

#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
#endif

    NEXUS_GetVideoEncoderCapabilities(&encoderCap);
    pContext->display = NEXUS_Display_Open(encoderCap.videoEncoder[pContext->id].displayIndex, &displaySettings);
    BDBG_ASSERT(pContext->display);
    BDBG_MSG(("BXCode%u video pipe%d opened encoder display%d.", bxcode->id, i, encoderCap.videoEncoder[pContext->id].displayIndex));
    pContext->window = NEXUS_VideoWindow_Open(pContext->display, 0);
    BDBG_ASSERT(pContext->window);

    NEXUS_VideoWindow_GetSettings(pContext->window, &windowSettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    windowSettings.position.width = 416;
    windowSettings.position.height = 224;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    windowSettings.visible = false;
#else
    /* disable forced capture */
    windowSettings.forceCapture = false;
#endif
    rc = NEXUS_VideoWindow_SetSettings(pContext->window, &windowSettings);
    BDBG_ASSERT(rc == NEXUS_SUCCESS);

    /* set transcoder window vnet mode bias to avoid black frame transition during dynamic resolution change for mfd source;
       NOTE: HDMI input might dynamically switch to 1080p60 and xcode BVN path doesn't have bandwidth to capture 1080p60! */
    {
        NEXUS_VideoWindow_GetScalerSettings(pContext->window, &scalerSettings);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
        scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
#endif
        scalerSettings.bandwidthEquationParams.delta = 1*1000*1000;
        NEXUS_VideoWindow_SetScalerSettings(pContext->window, &scalerSettings);
    }

    /* connect decoder to the encoder display; */
    NEXUS_VideoWindow_AddInput(pContext->window, NEXUS_VideoDecoder_GetConnector(pContext->decoder));

    /* VDC madr assignment is fixed */
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS /* disable MAD for dsp encoder for now */
    BXCode_P_WindowMad(pContext->window, false, false);
#else
    /* start small resol for rampup logic */
    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
    customFormatSettings.width = 640;
    customFormatSettings.height = 480;
    customFormatSettings.refreshRate = 59940;
    customFormatSettings.interlaced = false;
    customFormatSettings.aspectRatio = NEXUS_AspectRatio_eSquarePixel;
    customFormatSettings.dropFrameAllowed = true;
    rc = NEXUS_Display_SetCustomFormatSettings(pContext->display, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
    BDBG_ASSERT(rc == NEXUS_SUCCESS);
    BXCode_P_WindowMad(pContext->window, true, false);
#endif
    BXCode_P_Window_dnr(pContext->window);

    return NEXUS_SUCCESS;
}

static NEXUS_Error bxcode_p_open_audio_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i)
{
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_AudioMixerSettings audioMixerSettings;
    NEXUS_AudioMuxOutputCreateSettings audioMuxSettings;
    BXCode_P_Audio *pContext = &bxcode->audio[i];

    /* Open the audio decoder */
    /* Connect audio decoders to outputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.dspIndex    = bxcode->openSettings.audioDspId;
    pContext->decoder = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, &audioDecoderOpenSettings);
    if(!pContext->decoder) {BDBG_ERR(("BXCode%u failed to create audio decoder[%u]!", bxcode->id, i)); return NEXUS_NOT_AVAILABLE;}

    /* Open audio mixer.  The mixer can be left running at all times to provide continuous audio output despite input discontinuities.  */
    NEXUS_AudioMixer_GetDefaultSettings(&audioMixerSettings);
    audioMixerSettings.mixUsingDsp = true;
    audioMixerSettings.dspIndex    = bxcode->openSettings.audioDspId;
    pContext->mixer = NEXUS_AudioMixer_Open(&audioMixerSettings);
    if(!pContext->mixer) {BDBG_ERR(("BXCode%u] failed to create audio mixer[%u]!", bxcode->id, i)); return NEXUS_NOT_AVAILABLE;}

    /* Open audio mux output */
    /* increase audio mux output buffer size to match with parallel video encoders buffers at serial FNRT mux */
    NEXUS_AudioMuxOutput_GetDefaultCreateSettings(&audioMuxSettings);
    audioMuxSettings.data.fifoSize  *= bxcode->openSettings.vpipes;
    audioMuxSettings.index.fifoSize *= bxcode->openSettings.vpipes;
    pContext->muxOutput = NEXUS_AudioMuxOutput_Create(&audioMuxSettings);
    if(!pContext->muxOutput) {BDBG_ERR(("BXCode%u failed to create audio mux output[%u]!", bxcode->id, i)); return NEXUS_NOT_AVAILABLE;}
    BDBG_MSG(("BXCode%u audio[%u] mux output= %p", bxcode->id, i, pContext->muxOutput));
    return NEXUS_SUCCESS;
}

/* stop for non-FNRT usage */
void bxcode_p_pre_stop(BXCode_P_Context  *bxcode)
{
    BXCode_StartSettings *pSettings = &bxcode->startSettings;

    /* non-FNRT stops input here */
    if(bxcode->openSettings.vpipes > 1) return;
    if(!bxcode->started) return;

    /* disconnect sync channel only if both audio and video are enabled */
    if(pSettings->input.type != BXCode_InputType_eHdmi &&
       pSettings->output.audio[0].pid && pSettings->output.video.pid) {
        NEXUS_SyncChannelSettings syncChannelSettings;
        unsigned i;

        NEXUS_SyncChannel_GetSettings(bxcode->syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NULL;
        /* remove all audio PIDs to the same sync channel */
        for(i=0; i<BXCODE_MAX_AUDIO_PIDS && pSettings->output.audio[i].pid; i++) {
            syncChannelSettings.audioInput[i] = NULL;
        }
        NEXUS_SyncChannel_SetSettings(bxcode->syncChannel, &syncChannelSettings);
    }

    /************************************************
     * stop input playback
     */
    if(pSettings->input.type == BXCode_InputType_eFile) {
        NEXUS_Playback_Stop(bxcode->playback);
    } else if (pSettings->input.type == BXCode_InputType_eStream) {
        NEXUS_Playpump_Stop(bxcode->playpump);
    }
}

/* pre start for decoder av sync etc non-FNRT usage */
void bxcode_p_pre_start(BXCode_P_Context  *bxcode)
{
    unsigned i;
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_StcChannelPairSettings stcAudioVideoPair;
    BXCode_StartSettings *pSettings = &bxcode->startSettings;

    if(bxcode->openSettings.vpipes > 1) return;
    /* connect sync channel only if both audio and video are enabled */
    if(pSettings->input.type != BXCode_InputType_eHdmi &&
       pSettings->output.audio[0].pid && pSettings->output.video.pid) {
        NEXUS_SyncChannel_GetSettings(bxcode->syncChannel, &syncChannelSettings);
        syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(bxcode->video[0].decoder);
        /* NRT mode pairs AV stc channels: totally two STCs - one video, the other audio. */
        if(pSettings->nonRealTime) {
            NEXUS_StcChannel_GetDefaultPairSettings(&stcAudioVideoPair);
            stcAudioVideoPair.connected = true;
            stcAudioVideoPair.window = 300; /* 300ms AV window means when source discontinuity occurs, up to 300ms transition could occur with NRT transcoded stream */
            NEXUS_StcChannel_SetPairSettings(bxcode->video[0].stcChannel, bxcode->audio[0].stcChannel, &stcAudioVideoPair);
        }
        /* add all audio PIDs to the same sync channel assume they share the same timebase within the same multi-audio program stream! */
        for(i=0; i<BXCODE_MAX_AUDIO_PIDS && pSettings->output.audio[i].pid; i++) {
            if(pSettings->output.audio[i].passthrough) {
                syncChannelSettings.audioInput[i] = NEXUS_AudioDecoder_GetConnector(bxcode->audio[i].decoder, NEXUS_AudioDecoderConnectorType_eCompressed);
            } else if(pSettings->output.audio[i].pid) {
                syncChannelSettings.audioInput[i] = NEXUS_AudioDecoder_GetConnector(bxcode->audio[i].decoder, NEXUS_AudioDecoderConnectorType_eStereo);
            }
        }
        syncChannelSettings.enablePrecisionLipsync = false;/* to support 60->30 frc transcode */
        NEXUS_SyncChannel_SetSettings(bxcode->syncChannel, &syncChannelSettings);
    }
}

/* post start for encoder av sync etc non-FNRT usage */
void bxcode_p_post_start(BXCode_P_Context  *bxcode)
{
    unsigned i;
    unsigned encoderDelay = 0; /* in 27MHz clock  */
    BXCode_StartSettings *pSettings = &bxcode->startSettings;
    NEXUS_Error rc;

    if(bxcode->openSettings.vpipes > 1) return;

    /* av sync */
    /* find the max audio delay out of all audio PIDs */
    for(i=0; i<BXCODE_MAX_AUDIO_PIDS; i++) {
        if(encoderDelay < bxcode->audio[i].audioDelayStatus.endToEndDelay) encoderDelay = bxcode->audio[i].audioDelayStatus.endToEndDelay;
    }
    encoderDelay *= 27000; /* covnerted from ms to 27Mhz */

    /************************************************
     * Start encoder with av sync
     */
    /* get av sync encoder delay and start video encoder */
    if(pSettings->output.video.pid) {
        NEXUS_VideoEncoderStatus videoEncoderStatus;
        if(encoderDelay > bxcode->videoDelay.min)
        {
            if(encoderDelay > bxcode->videoDelay.max)
            {
                BDBG_ERR(("Audio encoderDelay is way too big! Use video encoderDelay max!"));
                encoderDelay = bxcode->videoDelay.max;
            }
            else
            {
                BDBG_WRN(("Use audio encoderDelay %u ms %u ticks@27Mhz!", encoderDelay/27000, encoderDelay));
            }
        }
        else
        {
            encoderDelay = bxcode->videoDelay.min;
            BDBG_WRN(("Use video encoderDelay %u ms %u ticks@27Mhz!", encoderDelay/27000, encoderDelay));
        }
        bxcode->settings.video.encoder.encoderDelay = encoderDelay;
        NEXUS_VideoEncoder_SetSettings(bxcode->video[0].encoder, &bxcode->settings.video.encoder);

        /* store the video encoder output buffer base address */
        NEXUS_VideoEncoder_GetStatus(bxcode->video[0].encoder, &videoEncoderStatus);
        if(videoEncoderStatus.bufferBlock) {
            NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &bxcode->video[0].pEncoderBufferBase);
        }
        BDBG_MSG(("video[%u] pEncoderBufferBase: %p", 0, bxcode->video[0].pEncoderBufferBase));
        if(videoEncoderStatus.metadataBufferBlock) {
            NEXUS_MemoryBlock_Lock(videoEncoderStatus.metadataBufferBlock, &bxcode->video[0].pEncoderMetadataBufferBase);
        }
        BDBG_MSG(("video[%u] pEncoderMetadataBufferBase: %p", 0, bxcode->video[0].pEncoderMetadataBufferBase));
    }

    /* start audio */
    for(i=0; i<BXCODE_MAX_AUDIO_PIDS; i++) {
        if(pSettings->output.audio[i].pid) {
            NEXUS_AudioMuxOutputStatus audioMuxStatus;
            bxcode->audio[i].audioMuxStartSettings.presentationDelay = encoderDelay/27000;
            rc = NEXUS_AudioMuxOutput_Start(bxcode->audio[i].muxOutput, &bxcode->audio[i].audioMuxStartSettings);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("audio mux out[%u] failed to start!", i)); return;}
            rc = NEXUS_AudioMixer_Start(bxcode->audio[i].mixer);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("audio mixer[%u] failed to start!", i)); return;}
            rc = NEXUS_AudioDecoder_Start(bxcode->audio[i].decoder, &bxcode->audio[i].audProgram);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("audio decoder[%u] failed to start!", i)); return;}

            /* store the audio mux output buffer base address */
            NEXUS_AudioMuxOutput_GetStatus(bxcode->audio[i].muxOutput, &audioMuxStatus);
            if(audioMuxStatus.bufferBlock) {
                NEXUS_MemoryBlock_Lock(audioMuxStatus.bufferBlock, &bxcode->audio[i].pEncoderBufferBase);
            }
            BDBG_MSG(("audio[%u] pEncoderBufferBase: %p", i, bxcode->audio[i].pEncoderBufferBase));
            if(audioMuxStatus.metadataBufferBlock) {
                NEXUS_MemoryBlock_Lock(audioMuxStatus.metadataBufferBlock, &bxcode->audio[i].pEncoderMetadataBufferBase);
            }
            BDBG_MSG(("audio[%u] pEncoderMetadataBufferBase: %p", i, bxcode->audio[i].pEncoderMetadataBufferBase));
        }
    }

    if(pSettings->input.type == BXCode_InputType_eFile) {
        rc = NEXUS_Playback_Start(bxcode->playback, bxcode->file, NULL);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("File playback failed to start!")); return;}
    } else if (pSettings->input.type == BXCode_InputType_eStream) {
        rc = NEXUS_Playpump_Start(bxcode->playpump);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("Stream playpump failed to start!")); return;}
    }

    if(pSettings->output.video.pid) {
        rc = NEXUS_VideoEncoder_Start(bxcode->video[0].encoder, &pSettings->output.video.encoder);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("video encoder failed to start!")); return;}
    }
}

void bxcode_init(BXCode_P_Context  *bxcode)
{
    unsigned i;
    bxcode->settings.video.enabled = true;/* enable by default */
    bxcode->settings.video.encoder.variableFrameRate = false;
    bxcode->settings.video.encoder.frameRate = NEXUS_VideoFrameRate_e29_97;
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    bxcode->settings.video.encoder.bitrateMax    = 1000000;
    bxcode->settings.video.width  = 640;
    bxcode->settings.video.height = 480;
#else
    bxcode->settings.video.encoder.bitrateMax    = 400000;
    bxcode->settings.video.width  = 416;
    bxcode->settings.video.height = 224;
#endif
    bxcode->settings.video.windowSettings.contentMode = NEXUS_VideoWindowContentMode_eFull;
    bxcode->settings.video.gfxSettings.visible           = true;
    bxcode->settings.video.gfxSettings.alpha             = 0xFF;/* default opaque gfx */
    bxcode->settings.video.gfxSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eSourceAlpha;
    bxcode->settings.video.gfxSettings.destBlendFactor   = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
    bxcode->settings.video.encoder.streamStructure.framesP = 29;
    bxcode->settings.video.encoder.streamStructure.framesB = 0;
    bxcode->settings.video.encoder.enableFieldPairing = true;
    NEXUS_VideoEncoder_GetDefaultStartSettings(&bxcode->startSettings.output.video.encoder);
    bxcode->startSettings.output.video.encoder.codec   = NEXUS_VideoCodec_eH264;
    bxcode->startSettings.output.video.encoder.profile = NEXUS_VideoCodecProfile_eMain;
    bxcode->startSettings.output.video.encoder.level   = NEXUS_VideoCodecLevel_e31;
    bxcode->startSettings.output.video.encoder.interlaced = false;
    for(i=0; i< BXCODE_MAX_AUDIO_PIDS; i++) {
        bxcode->startSettings.output.audio[i].codec   = NEXUS_AudioCodec_eAac;
        bxcode->settings.audio[i].enabled = true;
    }
}

NEXUS_Error bxcode_open_video_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i)
{
    NEXUS_Error rc;
    BXCode_P_Video *pContext = &bxcode->video[i];

    if(i >= NEXUS_NUM_VIDEO_ENCODERS) {
        BDBG_ERR(("Video transcoder %d doesn't exist!", i));
        return NEXUS_INVALID_PARAMETER;
    }
    if(pContext->encoder) {
        BDBG_WRN(("Video transcoder %d already opened!", i));
        return NEXUS_SUCCESS;
    }

    rc = bxcode_p_open_video_transcode(bxcode, i);
    if(rc != NEXUS_SUCCESS) {BDBG_WRN(("Video transcoder %d failed to open!", i)); return rc;}

    pContext->id = i;
    return NEXUS_SUCCESS;
}

void bxcode_close_video_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i)
{
    BXCode_P_Video *pContext = &bxcode->video[i];

    if(!pContext->encoder) {
        return;
    }

    /************************************
     * Bring down transcoder
     */

    /******************************************
     * nexus kernel mode requires explicit remove/shutdown video inputs before close windows/display
     */
    {
        NEXUS_VideoWindow_RemoveInput(pContext->window, NEXUS_VideoDecoder_GetConnector(pContext->decoder));
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pContext->decoder));
        NEXUS_VideoDecoder_Close(pContext->decoder);
        pContext->decoder = NULL;
        g_BXCode_P_State.openVideoDecoders &= ~(1 << pContext->decoderId);
    }
    NEXUS_VideoWindow_Close(pContext->window);
    NEXUS_Display_Close(pContext->display);
    pContext->display = NULL;

    NEXUS_VideoEncoder_Close(pContext->encoder);
    pContext->encoder = NULL;
}

/* stop without close */
void bxcode_stop_video_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i )
{
    BXCode_StartSettings *pSettings = &bxcode->startSettings;
    BXCode_P_Video *pContext = &bxcode->video[i];

    if(!pContext->started) {
        return;
    }

    /**************************************************
     * NOTE: stop sequence should be in front->back order
     */
    if(pSettings->input.type != BXCode_InputType_eHdmi && pSettings->input.type != BXCode_InputType_eImage) {
        NEXUS_VideoDecoder_Stop(pContext->decoder);
    }
    if(bxcode->openSettings.vpipes > 1) {/* FNRT */
        NEXUS_Playpump_Stop(pContext->playpump);
        NEXUS_Playpump_CloseAllPidChannels(pContext->playpump);
        NEXUS_Playpump_Close(pContext->playpump);
        B_Thread_Destroy(pContext->playpumpThread);
        BKNI_DestroyEvent(pContext->dataReadyEvent);
        BKNI_DestroyEvent(pContext->chunkDoneEvent);
        close(pContext->fd);
        pContext->fd = -1;
    }

    /* if file to file or es/mp4 output, stop encoder in normal mode to avoid unintended frame drop at end of file */
    if((pSettings->output.transport.type != BXCode_OutputType_eTs) ||
       (pSettings->input.type == BXCode_InputType_eFile &&
         pSettings->output.transport.type == BXCode_OutputType_eTs && pSettings->output.transport.file))
    {
        NEXUS_VideoEncoderStopSettings stopSettings;
        NEXUS_VideoEncoder_GetDefaultStopSettings(&stopSettings);
        /* only file-to-file transcode use normal stop mode to get whole eof; otherwise, stop AFAP. */
        stopSettings.mode = NEXUS_VideoEncoderStopMode_eNormal;
        NEXUS_VideoEncoder_Stop(pContext->encoder, &stopSettings);
        BDBG_MSG(("bxcode[%u] video encoder[%u] stopped in normal mode", bxcode->id, i));
    }

#if NEXUS_HAS_HDMI_INPUT /* resume input with video decoder */
    if(pSettings->input.type == BXCode_InputType_eHdmi) {
        NEXUS_VideoWindow_RemoveAllInputs(pContext->window);
        NEXUS_VideoWindow_AddInput(pContext->window, NEXUS_VideoDecoder_GetConnector(pContext->decoder));
    } else
#endif
    if(pSettings->input.type == BXCode_InputType_eImage) {
        NEXUS_VideoWindow_RemoveAllInputs(pContext->window);
        NEXUS_VideoInput_Shutdown(NEXUS_VideoImageInput_GetConnector(pContext->imageInput));
        NEXUS_VideoImageInput_Close(pContext->imageInput);
        NEXUS_VideoWindow_AddInput(pContext->window, NEXUS_VideoDecoder_GetConnector(pContext->decoder));
    }
    /* stopped */
    pContext->started = false;
    BDBG_MSG(("BXCode[%u] stopped video[%u], activeCnt=%d", bxcode->id, i, bxcode->activeXcoders));
}

/* start open */
void bxcode_start_video_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i )
{
    NEXUS_Error rc;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderSettings videoDecodeSettings;
    BXCode_P_Video *pContext = &bxcode->video[i];
    BXCode_StartSettings *pSettings = &bxcode->startSettings;
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    NEXUS_DisplayStgSettings stgSettings;
#endif

    if(pContext->started) {
        BDBG_WRN(("BXCode[%u] video[%u] was already started!", bxcode->id, i));
        return;
    }

    if(bxcode->openSettings.vpipes > 1) {/* FNRT settings */
        B_Mutex_Lock(bxcode->mutexActiveXcoders);
        bxcode->activeXcoders++;
        B_Mutex_Unlock(bxcode->mutexActiveXcoders);
    }
    BDBG_MSG(("BXCode[%u] starting video[%u], activeCnt=%d", bxcode->id, i, bxcode->activeXcoders));

    /* NRT mode each transcoder should only use 2 STCs on the same timebase:
           1) NRT video decode STC;
           2) NRT audio decode STC;
       NOTE: encoder/mux STC is only required in RT mode; NRT mode can reuse audio STC.
     */
    if(!pSettings->nonRealTime) {
        pContext->stcChannel = bxcode->stcChannelDecoder;
    }

    /****************************************
     * set up xcoder source: setup stc, pid channel, start decoder
     */
    if(pSettings->input.type == BXCode_InputType_eFile || pSettings->input.type == BXCode_InputType_eStream) {
        NEXUS_VideoDecoder_GetSettings(pContext->decoder, &videoDecodeSettings);
        videoDecodeSettings.streamChanged.callback = BXCode_P_VidSrcStreamChangedCallback;
        videoDecodeSettings.streamChanged.context  = pContext->decoder;
        videoDecodeSettings.streamChanged.param  = i;
        videoDecodeSettings.supportedCodecs[pSettings->input.vCodec] = true;
        videoDecodeSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture;
        /* TODO: set max WxH for decoder 0 to demo xcode from 4k source */

        if(bxcode->openSettings.vpipes > 1) {/* FNRT settings */
            NEXUS_PlaypumpSettings playpumpSettings;

            pContext->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
            if(NULL==pContext->playpump) {BDBG_ERR(("Video pipe[%u] failed to open video playpump!", i)); return;}
            BDBG_MSG(("BXCode[%u] video[%u] opened playpump.", bxcode->id, i));
            BKNI_CreateEvent(&pContext->chunkDoneEvent);
            videoDecodeSettings.fnrtSettings.chunkDone.param = i;
            videoDecodeSettings.fnrtSettings.chunkDone.context  = pContext->chunkDoneEvent;
            videoDecodeSettings.fnrtSettings.chunkDone.callback = BXCode_P_VidChunkDoneCallback;
            videoDecodeSettings.fnrtSettings.enable = true;
            videoDecodeSettings.fnrtSettings.preChargeCount = 10;/* TODO: make it configurable according to content */
            /* use stdio for file I/O to keep the example simple. */
            pContext->fd = open(pSettings->input.data, O_RDONLY | O_LARGEFILE, 0666);
            if (pContext->fd < 0) {
                BDBG_ERR(("can't open source file:%s\n", pSettings->input.data));
                BDBG_ASSERT(0);
            }
            rc = fcntl(pContext->fd, F_GETFL);
            if ((int)rc != -1) {
                rc = fcntl(pContext->fd, F_SETFL, rc | FD_CLOEXEC);
            }
            BKNI_CreateEvent(&pContext->dataReadyEvent);
            NEXUS_Playpump_GetSettings(pContext->playpump, &playpumpSettings);
            playpumpSettings.dataCallback.callback = BXCode_P_PlaypumpCallback;
            playpumpSettings.dataCallback.context = pContext->dataReadyEvent;
            NEXUS_Playpump_SetSettings(pContext->playpump, &playpumpSettings);
            /* this affects video encoder A2P delay */
            pSettings->output.video.encoder.numParallelEncodes = bxcode->openSettings.vpipes;

            /* Open the video pid channel */
            pContext->pidChannel = NEXUS_Playpump_OpenPidChannel(pContext->playpump, pSettings->input.vPid, NULL);
        } else {
            /* Open the video pid channel */
            if(pSettings->input.type == BXCode_InputType_eFile && pSettings->input.vPid) {
                NEXUS_PlaybackPidChannelSettings playbackPidSettings;
                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                playbackPidSettings.pidTypeSettings.video.codec = pSettings->input.vCodec; /* must be told codec for correct handling */
                playbackPidSettings.pidTypeSettings.video.index = pSettings->input.index;
                playbackPidSettings.pidTypeSettings.video.decoder = pContext->decoder;
                pContext->pidChannel = NEXUS_Playback_OpenPidChannel(bxcode->playback, pSettings->input.vPid, &playbackPidSettings);
            }
            if(pSettings->input.type == BXCode_InputType_eStream && pSettings->input.vPid) {
                pContext->pidChannel = NEXUS_Playpump_OpenPidChannel(bxcode->playpump, pSettings->input.vPid, NULL);
            }
            BDBG_MSG(("bxcode[%u] video[%u] opened PID channel %p for input %d.", bxcode->id, pContext->id, pContext->pidChannel, pSettings->input.type));
        }
        NEXUS_VideoDecoder_SetSettings(pContext->decoder, &videoDecodeSettings);
    } else if (pSettings->input.type == BXCode_InputType_eLive) {/* live input */
        /* set live input STC channel pcr mode */
        NEXUS_StcChannel_GetSettings(pContext->stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live pcr mode */

        if(pSettings->input.vPid) {/* open vpid channel */
            pContext->pidChannel = NEXUS_PidChannel_Open(pSettings->input.parserBand, pSettings->input.vPid, NULL);
            BDBG_MSG(("BXCode[%u] opened vPID %u for parser band %d.", bxcode->id, pSettings->input.vPid, pSettings->input.parserBand));
        } else
            goto BXCode_P_SetupVideoEncoder;

        if(pSettings->input.pcrPid != pSettings->input.vPid) {/* open pcr pid channel */
            bxcode->pcrPidChannel = NEXUS_PidChannel_Open(pSettings->input.parserBand, pSettings->input.pcrPid, NULL);
            BDBG_MSG(("BXCode[%u] opened PCR PID %u for parser band %d.", bxcode->id, pSettings->input.pcrPid, pSettings->input.parserBand));
            stcSettings.modeSettings.pcr.pidChannel = bxcode->pcrPidChannel;/* different PCR PID */
        } else {
            stcSettings.modeSettings.pcr.pidChannel = pContext->pidChannel; /* PCR happens to be on video pid */
        }
        NEXUS_StcChannel_SetSettings(pContext->stcChannel, &stcSettings);
        BDBG_MSG(("Video[%u] opened source vSTC [%p].", i, pContext->stcChannel));
    } else { /* hdmi/image input */
#if NEXUS_HAS_HDMI_INPUT
        if(pSettings->input.type == BXCode_InputType_eHdmi) {
            NEXUS_VideoInput videoInput = NEXUS_HdmiInput_GetVideoConnector(pSettings->input.hdmiInput);
            NEXUS_VideoWindow_RemoveAllInputs(pContext->window);
            NEXUS_VideoWindow_AddInput(pContext->window, videoInput);
        } else
#endif
        if(pSettings->input.type == BXCode_InputType_eImage) {
            NEXUS_PlatformSettings platform;
            NEXUS_VideoImageInputSettings imageInputSetting;
            /* disconnect mfd input first */
            NEXUS_VideoWindow_RemoveAllInputs(pContext->window);
            NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pContext->decoder));

            /* connect image input to window */
            NEXUS_Platform_GetSettings(&platform);
            BDBG_MSG(("bxcode[%u] to open image input[%u](%u)", bxcode->id, platform.videoDecoderModuleSettings.mfdMapping[pContext->decoderId], pContext->decoderId));
            pContext->imageInput = NEXUS_VideoImageInput_Open(platform.videoDecoderModuleSettings.mfdMapping[pContext->decoderId], NULL);
            if(!pContext->imageInput)
            {
                BDBG_ERR(("Can't create imageInput[%u]", platform.videoDecoderModuleSettings.mfdMapping[pContext->decoderId]));
                return;
            }
            NEXUS_VideoWindow_AddInput(pContext->window, NEXUS_VideoImageInput_GetConnector(pContext->imageInput));

            /* hook up image input callback for app to feed video surface input */
            NEXUS_VideoImageInput_GetSettings( pContext->imageInput, &imageInputSetting );
            imageInputSetting.imageCallback.callback = BXCode_P_VideoImageInputBufferCallback;
            imageInputSetting.imageCallback.context  = &pSettings->input.dataCallback;
            if (NEXUS_VideoImageInput_SetSettings(pContext->imageInput, &imageInputSetting ) != NEXUS_SUCCESS)
            {
                BDBG_ERR(("Failed to set video image input settings."));
                NEXUS_VideoImageInput_Close(pContext->imageInput);
                return;
            }
        }
        goto BXCode_P_SetupVideoEncoder;
    }

    /* Set up decoder Start structures now. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&pContext->vidProgram);
    pContext->vidProgram.codec = pSettings->input.vCodec;
    pContext->vidProgram.pidChannel = pContext->pidChannel;
    pContext->vidProgram.stcChannel = pContext->stcChannel;
    pContext->vidProgram.nonRealTime = pSettings->nonRealTime;
    BDBG_MSG(("bxcode[%u] video[%u] decoder %s mode", bxcode->id, i, pContext->vidProgram.nonRealTime?"NRT":"RT"));

BXCode_P_SetupVideoEncoder:
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    /* NRT setup - AFAP mode */
    NEXUS_Display_GetStgSettings(pContext->display, &stgSettings);
    stgSettings.enabled     = true;
    stgSettings.nonRealTime = pSettings->nonRealTime;
    NEXUS_Display_SetStgSettings(pContext->display, &stgSettings);
#endif
    rc = BXCode_P_SetVideoSettings(bxcode, i, &bxcode->settings);
    if(NEXUS_SUCCESS != rc) { BDBG_ERR(("BXCode[%u] failed to set video settings", bxcode->id)); }

    /****************************
     * start decoders
     */
    /* Start decoder */
    if(pSettings->input.type == BXCode_InputType_eFile || pSettings->input.type == BXCode_InputType_eStream || pSettings->input.type == BXCode_InputType_eLive) {
        NEXUS_VideoDecoder_Start(pContext->decoder, &pContext->vidProgram);
    }

    /****************************
     * setup video encoder
     */
    pSettings->output.video.encoder.input = pContext->display;
    pSettings->output.video.encoder.nonRealTime = pSettings->nonRealTime;

    /************************************
     * FNRT transcoder AV sync is handled by FNRT mux
     * so it could be started right away;
     */
    /* NOTE: video encoder delay is in 27MHz ticks */
    pSettings->output.video.encoder.stcChannel = bxcode->stcChannelEncoder;
    pSettings->output.video.encoder.numParallelEncodes = bxcode->openSettings.vpipes; /* This affects a2p delay for FNRT */
    NEXUS_VideoEncoder_GetDelayRange(pContext->encoder, &bxcode->settings.video.encoder, &pSettings->output.video.encoder, &bxcode->videoDelay);
    BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", bxcode->videoDelay.min/27000, bxcode->videoDelay.max/27000));
    if(bxcode->openSettings.vpipes > 1) {/* FNRT settings */
        bxcode->settings.video.encoder.encoderDelay = bxcode->videoDelay.min;/* fnrt encoder av sync handled by mux */
        if(bxcode->settings.video.encoder.bitrateTarget==0) {/* FNRT can force VBR for better quality */
            bxcode->settings.video.encoder.bitrateTarget = bxcode->settings.video.encoder.bitrateMax;
            bxcode->settings.video.encoder.bitrateMax *= 2;
        }
        NEXUS_VideoEncoder_SetSettings(pContext->encoder, &bxcode->settings.video.encoder);

        /************************************************
         * Start video encoder: encoder STC depends on NRT/RT mode */
        pSettings->output.video.encoder.stcChannel = pContext->stcChannel; /* debug stc snapshot */
        pSettings->output.video.encoder.encodeUserData = false; /* FNRT doesn't support CC user data for now; TODO */
        pSettings->output.video.encoder.adaptiveLowDelayMode = false; /* FNRT shouldn't need adaptive low delay */
        rc = NEXUS_VideoEncoder_Start(pContext->encoder, &pSettings->output.video.encoder);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("FNRT video encoder[%u] failed to start!", i)); return;}
        /* Start playpump thread */
        rc = NEXUS_Playpump_Start(pContext->playpump);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("FNRT playpump[%u] failed to start!", i)); return;}
        pContext->playpumpThread = B_Thread_Create("FNRT playpump thread", (B_ThreadFunc)BXCode_P_VideoPlaypump_thread, pContext, NULL);
    }
    /* started */
    pContext->started = true;
}

NEXUS_Error bxcode_open_audio_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i)
{
    BXCode_P_Audio *pContext = &bxcode->audio[i];
    int rc;

    if(pContext->muxOutput) {
        return NEXUS_SUCCESS;
    }

    /* each transcoder potentially should only use 2 STCs on the same timebase:
           1) NRT video decode STC; (RT/NRT)
           2) NRT audio decode STC; (NRT)
           3) encode/mux STC;  (RT)
       NOTE: to avoid the 3rd one since NRT mode doesn't really need it,
             encoder/mux STC is only required in RT mode; and in RT mode, a/v decoders share the same STC.
     */

    /****************************************
     * set up xcoder source
     */
    rc = bxcode_p_open_audio_transcode(bxcode, i);
    if(NEXUS_SUCCESS!=rc) {return BERR_TRACE(rc);}

    pContext->id = i;
    return NEXUS_SUCCESS;
}

void bxcode_close_audio_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i)
{
    BXCode_P_Audio *pContext = &bxcode->audio[i];

    if(!pContext->muxOutput) {
        return;
    }

    /******************************************
     * nexus kernel mode requires explicit remove/shutdown video inputs before close windows/display
     */
    NEXUS_AudioMixer_RemoveAllInputs(pContext->mixer);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioMixer_GetConnector(pContext->mixer));
    NEXUS_AudioMixer_Close(pContext->mixer);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(pContext->decoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(pContext->decoder);

    NEXUS_AudioOutput_Shutdown(NEXUS_AudioMuxOutput_GetConnector(pContext->muxOutput));
    NEXUS_AudioMuxOutput_Destroy(pContext->muxOutput);
    pContext->muxOutput = NULL;
}

/* acquire audio dummy output top down.
   TODO: share with external user.
 */
static unsigned getAudioDummyId(BXCode_P_Context  *bxcode)
{
    unsigned audioDummyId = NEXUS_NUM_AUDIO_DUMMY_OUTPUTS - 1;
    BSTD_UNUSED(bxcode);
    do {
        if(((1<<audioDummyId) & g_BXCode_P_State.usedAudioDummyOutputs) == 0) break;
    } while (audioDummyId--);
    if(audioDummyId == (unsigned)(-1)) {
        BDBG_ERR(("BXCode[%u] runs out of dummy output", bxcode->id));
    } else {
        g_BXCode_P_State.usedAudioDummyOutputs |= (1<<audioDummyId);
    }
    return audioDummyId;
}

static void putAudioDummyId(unsigned id)
{
    g_BXCode_P_State.usedAudioDummyOutputs &= ~(1<<id);
}

/* stop without close */
void bxcode_stop_audio_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i )
{
    BXCode_P_Audio *pContext = &bxcode->audio[i];
    BXCode_StartSettings *pSettings = &bxcode->startSettings;

    if(!pContext->started) {
        return;
    }

    /**************************************************
     * NOTE: stop sequence should be in front->back order
     */
    if(bxcode->openSettings.vpipes > 1) {/* FNRT */
        NEXUS_AudioDecoderStatus status;
        unsigned j;
        for(j=0; j<2;) {
            NEXUS_AudioDecoder_GetStatus(pContext->decoder, &status);
            BDBG_MSG(("audio[%u] has %u queued frames...", i, status.queuedFrames));
            if(status.queuedFrames < 3) {
                j++;
            }
            BKNI_Sleep(1000);
        }
    }
    NEXUS_AudioDecoder_Stop(pContext->decoder);
    NEXUS_AudioMixer_Stop(pContext->mixer);
    NEXUS_AudioMuxOutput_Stop(pContext->muxOutput);
    if(bxcode->openSettings.vpipes > 1) {/* FNRT */
        NEXUS_Playpump_Stop(pContext->playpump);
        NEXUS_Playpump_CloseAllPidChannels(pContext->playpump);
        NEXUS_Playpump_Close(pContext->playpump);
        B_Thread_Destroy(pContext->playpumpThread);
        BKNI_DestroyEvent(pContext->dataReadyEvent);
        close(pContext->fd);
        pContext->fd = -1;
    }

    if(!pSettings->output.audio[i].passthrough)
    {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pContext->muxOutput));
        NEXUS_AudioEncoder_RemoveAllInputs(pContext->encoder);
        NEXUS_AudioInput_Shutdown(NEXUS_AudioEncoder_GetConnector(pContext->encoder));
        NEXUS_AudioEncoder_Close(pContext->encoder);
    }
    else
    {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(pContext->muxOutput));
    }
    if(!pSettings->nonRealTime) {
        if((bxcode->numAudios == 1 || !pSettings->output.audio[i].passthrough)) {
            if(--bxcode->audioDummyRefCnt == 0) {/* reference counted */
                BDBG_MSG(("BXCode[%u] to remove dummy output for audio[%u]", bxcode->id, i));
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(g_BXCode_platformConfig.outputs.audioDummy[bxcode->audioDummyId]));
                NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(g_BXCode_platformConfig.outputs.audioDummy[bxcode->audioDummyId]));
                putAudioDummyId(bxcode->audioDummyId);/* release dummy output */
            }
        }
        if(bxcode->numAudios > 1 && bxcode->hwMixer && (i+1 == bxcode->numAudios)) {/* last audio PID releases its resource */
            BDBG_MSG(("BXCode[%u] to remove passthrough audio[%u] dspMixer->hwMixer", bxcode->id, i));
            NEXUS_AudioMixer_RemoveAllInputs(bxcode->hwMixer);
            BDBG_MSG(("BXCode[%u] to remove audio[%u] hwMixer->dummyOutput", bxcode->id, i));
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(g_BXCode_platformConfig.outputs.audioDummy[bxcode->audioDummyId2]));
            NEXUS_AudioOutput_Shutdown(NEXUS_AudioDummyOutput_GetConnector(g_BXCode_platformConfig.outputs.audioDummy[bxcode->audioDummyId2]));
            putAudioDummyId(bxcode->audioDummyId2);/* release dummy output */
            NEXUS_AudioMixer_Close(bxcode->hwMixer);
            bxcode->hwMixer = NULL;
        }
    }

    /* stopped */
    pContext->started = false;

    BDBG_MSG(("stopped audio[%d], activeCnt=%d", i, bxcode->activeXcoders));
}

/* start without open */
void bxcode_start_audio_transcode(
    BXCode_P_Context  *bxcode,
    unsigned           i )
{
    NEXUS_Error rc;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_AudioMixerSettings audioMixerSettings;
    NEXUS_AudioCodec audioCodec=NEXUS_AudioCodec_eAac;
    BXCode_P_Audio *pContext = &bxcode->audio[i];
    BXCode_StartSettings *pSettings = &bxcode->startSettings;

    if(pContext->started) {
        BDBG_WRN(("BXCode[%u] audio[%u] was already started!", bxcode->id, i));
        return;
    }

    if(bxcode->openSettings.vpipes > 1) {/* FNRT settings */
        B_Mutex_Lock(bxcode->mutexActiveXcoders);
        bxcode->activeXcoders++;
        B_Mutex_Unlock(bxcode->mutexActiveXcoders);
    }
    BDBG_MSG(("BXCode[%u], starting audio[%u], activeCnt=%d", bxcode->id, i, bxcode->activeXcoders));


    /* open decoder aSTC for NRT mode */
    if(!pSettings->nonRealTime) {
        pContext->stcChannel = bxcode->stcChannelDecoder;
    }

    /****************************************
     * set up xcoder source: setup stc, pid channel, start decoder
     */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&pContext->audProgram);

    if(pSettings->input.type == BXCode_InputType_eFile || pSettings->input.type == BXCode_InputType_eStream) {
        if(bxcode->openSettings.vpipes > 1) {/* FNRT */
            NEXUS_PlaypumpSettings playpumpSettings;

            pContext->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
            if(NULL==pContext->playpump) {
                BDBG_ERR(("BXCode[%u] audio[%u] failed to open playpump", bxcode->id, i)); return;
            }
            BDBG_MSG(("FNRT audio[%u] opened source playpump.", i));
            NEXUS_StcChannel_GetSettings(pContext->stcChannel, &stcSettings);
            stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eAudioMaster;
            NEXUS_StcChannel_SetSettings(pContext->stcChannel, &stcSettings);
            BDBG_MSG(("Audio[%u] masters source aSTC  [%p].", pContext->id, pContext->stcChannel));

            /* file I/O */
            pContext->fd = open(bxcode->startSettings.input.data, O_RDONLY | O_LARGEFILE, 0666);
            if (pContext->fd < 0) {
                BDBG_ERR(("can't open source file:%s", bxcode->startSettings.input.data));
                BDBG_ASSERT(0);
            }
            rc = fcntl(pContext->fd, F_GETFL);
            if ((int)rc != -1) {
                rc = fcntl(pContext->fd, F_SETFL, rc | FD_CLOEXEC);
            }
            BKNI_CreateEvent(&pContext->dataReadyEvent);
            NEXUS_Playpump_GetSettings(pContext->playpump, &playpumpSettings);
            playpumpSettings.dataCallback.callback = BXCode_P_PlaypumpCallback;
            playpumpSettings.dataCallback.context = pContext->dataReadyEvent;
            NEXUS_Playpump_SetSettings(pContext->playpump, &playpumpSettings);
            pContext->pidChannel = NEXUS_Playpump_OpenPidChannel(pContext->playpump, bxcode->startSettings.input.aPid[i], NULL);
        } else {
            /* Open the audio pid channel */
            if(pSettings->input.type == BXCode_InputType_eFile && pSettings->input.aPid[i]) {
                NEXUS_PlaybackPidChannelSettings playbackPidSettings;
                NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
                playbackPidSettings.pidTypeSettings.audio.primary = pContext->decoder;
                audioCodec = bxcode->startSettings.input.aCodec[i];
                pContext->pidChannel = NEXUS_Playback_OpenPidChannel(bxcode->playback, bxcode->startSettings.input.aPid[i], &playbackPidSettings);
            }
            if(pSettings->input.type == BXCode_InputType_eStream && pSettings->input.aPid[i]) {
                audioCodec = bxcode->startSettings.input.aCodec[i];
                pContext->pidChannel = NEXUS_Playpump_OpenPidChannel(bxcode->playpump, bxcode->startSettings.input.aPid[i], NULL);
            }
            BDBG_MSG(("BXCode[%u] audio[%u] opened PID channel %p for input %d.", bxcode->id, pContext->id, pContext->pidChannel, pSettings->input.type));
        }
    }
    else if (pSettings->input.type == BXCode_InputType_eLive) {/* live input */
        if(pSettings->input.aPid[i]) {/* open apid channel */
            audioCodec = bxcode->startSettings.input.aCodec[i];
            pContext->pidChannel = NEXUS_PidChannel_Open(pSettings->input.parserBand, pSettings->input.aPid[i], NULL);
            BDBG_MSG(("BXCode[%u] opened aPID[%u] %u from parser band %d.", bxcode->id, i, pSettings->input.aPid[i], pSettings->input.parserBand));
        } else
            goto BXCode_P_SetupAudioEncoder;
    } else if (pSettings->input.type == BXCode_InputType_eHdmi) {/* hdmi input */
#if NEXUS_HAS_HDMI_INPUT
        audioCodec = NEXUS_AudioCodec_eLpcm1394;/* TODO: support compressed hdmi audio transcode */
        pContext->audProgram.input = NEXUS_HdmiInput_GetAudioConnector(pSettings->input.hdmiInput);
        pContext->audProgram.latencyMode = NEXUS_AudioDecoderLatencyMode_eLow;/* dsp mixer after hdmi cannot use low delay mode */
#endif
    } else { /* TODO: image input */
        return;
    }

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    pContext->audProgram.codec = pSettings->input.aCodec[i];
    pContext->audProgram.pidChannel = pContext->pidChannel;
    pContext->audProgram.nonRealTime= bxcode->startSettings.nonRealTime;
    pContext->audProgram.stcChannel = pContext->stcChannel;

BXCode_P_SetupAudioEncoder:
    /* Connect decoder to mixer and set as master */
    NEXUS_AudioMixer_AddInput(pContext->mixer,
        NEXUS_AudioDecoder_GetConnector(pContext->decoder, pSettings->output.audio[i].passthrough
            ? NEXUS_AudioDecoderConnectorType_eCompressed : NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioMixer_GetSettings(pContext->mixer, &audioMixerSettings);
    audioMixerSettings.master = NEXUS_AudioDecoder_GetConnector(pContext->decoder, pSettings->output.audio[i].passthrough
            ? NEXUS_AudioDecoderConnectorType_eCompressed : NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_AudioMixer_SetSettings(pContext->mixer, &audioMixerSettings);

    if(!pSettings->output.audio[i].passthrough)
    {
        NEXUS_AudioEncoderSettings encoderSettings;

        /* Open audio encoder */
        NEXUS_AudioEncoder_GetDefaultSettings(&encoderSettings);
        audioCodec = encoderSettings.codec = bxcode->startSettings.output.audio[i].codec;
        pContext->encoder = NEXUS_AudioEncoder_Open(&encoderSettings);
        BDBG_ASSERT(pContext->encoder);
        rc = BXCode_P_SetAudioSettings(bxcode, i, &bxcode->settings);
        if(NEXUS_SUCCESS != rc) { BDBG_ERR(("BXCode[%u] failed to set audio codec settings", bxcode->id)); }

        /* Connect mixer to encoder */
        NEXUS_AudioEncoder_AddInput(pContext->encoder,
            NEXUS_AudioMixer_GetConnector(pContext->mixer));

        /* Connect mux to encoder */
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(pContext->muxOutput),
            NEXUS_AudioEncoder_GetConnector(pContext->encoder));
    }
    else
    {
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(pContext->muxOutput),
            NEXUS_AudioMixer_GetConnector(pContext->mixer));
    }

    if(!pSettings->nonRealTime) {
        NEXUS_Platform_GetConfiguration(&g_BXCode_platformConfig);

        /* Open the hw mixer -> dummy output for multiple audio PIDs in RT mode */
        if(bxcode->numAudios > 1 && !bxcode->hwMixer) {
            bxcode->hwMixer = NEXUS_AudioMixer_Open(NULL);
            if ( !bxcode->hwMixer )
            {
                BDBG_ERR(("BXCode[%u] unable to open HW mixer", bxcode->id));
                return;
            }
            bxcode->audioDummyId2 = getAudioDummyId(bxcode);/* acquire a dummy out */
            if(bxcode->audioDummyId2 == (unsigned)(-1)) {
                BDBG_ERR(("BXCode[%u] failed to acquire audio dummy output for RT mode audio[%u] hw mixer!", bxcode->id, i));
                return;
            }
            NEXUS_AudioOutput_AddInput(/* hwMixer -> dummy1 out */
                NEXUS_AudioDummyOutput_GetConnector(g_BXCode_platformConfig.outputs.audioDummy[bxcode->audioDummyId2]),
                NEXUS_AudioMixer_GetConnector(bxcode->hwMixer));
        }

        /* single audio PID or transcode audio PID: dspMixer -> dummy0 out */
        if(bxcode->numAudios == 1 || !pSettings->output.audio[i].passthrough) {
            bxcode->audioDummyId = getAudioDummyId(bxcode);
            if(bxcode->audioDummyId == (unsigned)(-1)) {
                BDBG_ERR(("BXCode[%u] failed to acquire audio dummy output for RT mode audio[%u]!", bxcode->id, i));
                return;
            }
            bxcode->audioDummyRefCnt++;/* up reference count */
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDummyOutput_GetConnector(g_BXCode_platformConfig.outputs.audioDummy[bxcode->audioDummyId]),
                NEXUS_AudioMixer_GetConnector(bxcode->audio[i].mixer));
        } else {/* dspMixer -> hwMixer(-> dummy1 out) */
            NEXUS_AudioMixer_AddInput(bxcode->hwMixer,
                NEXUS_AudioMixer_GetConnector(bxcode->audio[i].mixer));
        }
    }

    /************************************
     * transcoder AV sync is handled by FNRT mux
     * so audio can be started imemdiately; non-FNRT has to wait until post_start to set AV sync delay first!
     */
    NEXUS_AudioMuxOutput_GetDefaultStartSettings(&pContext->audioMuxStartSettings);
    /* audio NRT requires mux out to take NRT decode STC */
    pContext->audioMuxStartSettings.stcChannel        = bxcode->stcChannelEncoder;
    pContext->audioMuxStartSettings.nonRealTime       = bxcode->startSettings.nonRealTime;

    NEXUS_AudioMuxOutput_GetDelayStatus(pContext->muxOutput, audioCodec, &pContext->audioDelayStatus);
    BDBG_WRN(("\tAudio codec %d end-to-end delay = %u ms", audioCodec, pContext->audioDelayStatus.endToEndDelay));

    if(bxcode->openSettings.vpipes > 1) {/* FNRT start */
        pContext->audioMuxStartSettings.presentationDelay = pContext->audioDelayStatus.endToEndDelay/27000;/* in ms */

        /* audio start order is back-to-front */
        /* Start audio mux output */
        NEXUS_AudioMuxOutput_Start(pContext->muxOutput, &pContext->audioMuxStartSettings);
        /* Start audio mixer */
        NEXUS_AudioMixer_Start(pContext->mixer);
        NEXUS_AudioDecoder_Start(pContext->decoder, &pContext->audProgram);

        /* Start playpump thread */
        NEXUS_Playpump_Start(pContext->playpump);
        pContext->playpumpThread = B_Thread_Create("FNRT audio playpump thread", (B_ThreadFunc)BXCode_P_AudioPlaypump_thread, pContext, NULL);
    }

    /* started */
    pContext->started = true;
}

/*******************************
 * Set up stream_mux and record
 */
NEXUS_Error bxcode_open_mp4mux( BXCode_P_Context  *pContext )
{
    NEXUS_FileMuxCreateSettings muxCreateSettings;
    unsigned i;

    BKNI_CreateEvent(&pContext->finishEvent);
    NEXUS_FileMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = BXCode_P_TranscoderFinishCallback;
    muxCreateSettings.finished.context = pContext;
    pContext->fileMux = NEXUS_FileMux_Create(&muxCreateSettings);

    NEXUS_FileMux_GetDefaultStartSettings(&pContext->mp4MuxConfig, NEXUS_TransportType_eMp4);

    /* disable progressive download to speed up mp4 mux finish time */
    pContext->mp4MuxConfig.streamSettings.mp4.progressiveDownloadCompatible = pContext->startSettings.output.transport.progressiveDownload;
    pContext->mp4MuxConfig.video[0].track = 1;
    pContext->mp4MuxConfig.video[0].codec = pContext->startSettings.output.video.encoder.codec;
    pContext->mp4MuxConfig.video[0].encoder = pContext->video[0].encoder;

    for(i=0; i < BXCODE_MAX_AUDIO_PIDS && pContext->startSettings.output.audio[i].pid; i++) {
        pContext->mp4MuxConfig.audio[i].track = 2 + i;
        pContext->mp4MuxConfig.audio[i].codec = pContext->startSettings.output.audio[i].codec;
        pContext->mp4MuxConfig.audio[i].muxOutput = pContext->audio[i].muxOutput;
    }
    BKNI_Memcpy(pContext->mp4MuxConfig.tempDir, pContext->startSettings.output.transport.tmpDir, sizeof(pContext->mp4MuxConfig.tempDir));
    pContext->fileMuxOutput = NEXUS_MuxFile_OpenPosix(pContext->startSettings.output.transport.file);
    if (!pContext->fileMuxOutput) {
        BDBG_ERR(("can't open file:%s", pContext->startSettings.output.transport.file));
        return NEXUS_NOT_AVAILABLE;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error bxcode_open_tsmux( BXCode_P_Context  *pContext )
{
    NEXUS_StreamMuxCreateSettings muxCreateSettings;
    NEXUS_StreamMuxStartSettings muxConfig;
    NEXUS_PlaypumpOpenSettings playpumpConfig;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_MessageSettings messageSettings;
    NEXUS_MessageStartSettings messageStartSettings;
    unsigned i;

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
    playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
    playpumpConfig.numDescriptors = 64; /* set number of descriptors */
    playpumpConfig.streamMuxCompatible = true;

    if(pContext->startSettings.output.video.pid) {
        pContext->playpumpMuxVideo = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
        if(NULL==pContext->playpumpMuxVideo) {BDBG_ERR(("TS mux failed to open video playpump!")); return NEXUS_NOT_AVAILABLE;}
    }
    pContext->playpumpMuxSystem = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
    if(NULL==pContext->playpumpMuxSystem) {BDBG_ERR(("TS mux failed to open system playpump!")); return NEXUS_NOT_AVAILABLE;}

    BKNI_CreateEvent(&pContext->finishEvent);
    NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.finished.callback = BXCode_P_TranscoderFinishCallback;
    muxCreateSettings.finished.context = pContext;
    {/* reduce stream mux memory allocation if no TS user data passthru */
        NEXUS_StreamMuxConfiguration streamMuxConfig;
        NEXUS_StreamMux_GetDefaultConfiguration(&streamMuxConfig);
        if(0 == pContext->startSettings.input.numTsUserDataPids) {
            streamMuxConfig.userDataPids = 0;/* remove unnecessary memory allocation */
        } else if(pContext->startSettings.input.numTsUserDataPids == (unsigned)(-1)) {
            streamMuxConfig.userDataPids = NEXUS_MAX_MUX_PIDS;
        } else {
            streamMuxConfig.userDataPids = pContext->startSettings.input.numTsUserDataPids;
        }
        streamMuxConfig.audioPids = pContext->numAudios;
        streamMuxConfig.nonRealTime = pContext->startSettings.nonRealTime;
        streamMuxConfig.supportTts  = (pContext->startSettings.output.transport.timestampType != NEXUS_TransportTimestampType_eNone);
        NEXUS_StreamMux_GetMemoryConfiguration(&streamMuxConfig,&muxCreateSettings.memoryConfiguration);
    }
    pContext->streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
    if(NULL==pContext->streamMux) {BDBG_ERR(("TS mux failed to create!")); return NEXUS_NOT_AVAILABLE;}
    NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
    muxConfig.transportType = NEXUS_TransportType_eTs;
    muxConfig.nonRealTime = pContext->startSettings.nonRealTime;
    muxConfig.nonRealTimeRate = 8 * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */
    muxConfig.servicePeriod = 50; /* MSP */
    muxConfig.supportTts = (pContext->startSettings.output.transport.timestampType != NEXUS_TransportTimestampType_eNone);;

    muxConfig.video[0].pid = pContext->startSettings.output.video.pid;
    muxConfig.video[0].encoder = pContext->video[0].encoder;
    muxConfig.video[0].playpump = pContext->playpumpMuxVideo;

    for(i=0; i < BXCODE_MAX_AUDIO_PIDS && pContext->startSettings.output.audio[i].pid; i++) {
        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
        playpumpConfig.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
        playpumpConfig.numDescriptors = 64; /* set number of descriptors */
        playpumpConfig.streamMuxCompatible = true;
        pContext->playpumpMuxAudio[i] = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpConfig);
        if(NULL==pContext->playpumpMuxAudio[i]) {BDBG_ERR(("TS mux failed to open audio playpump%u!", i)); return NEXUS_NOT_AVAILABLE;}

        muxConfig.audio[i].pid = pContext->startSettings.output.audio[i].pid;
        muxConfig.audio[i].muxOutput = pContext->audio[i].muxOutput;
        muxConfig.audio[i].playpump = pContext->playpumpMuxAudio[i];
    }
    muxConfig.pcr.pid = pContext->startSettings.output.transport.pcrPid;
    muxConfig.pcr.playpump = pContext->playpumpMuxSystem;
    muxConfig.pcr.interval = 100;

    /******************************************
     * Set up xcoder record output
     */
    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    BDBG_MSG(("To open recpump with dataReadyThreshold = %d indexReadyThreshold=%d",
        recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.index.dataReadyThreshold));
    BDBG_MSG(("        recpump with data fifo size     = %d index fifo size    =%d",
        recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.index.bufferSize));
    pContext->recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    BDBG_ASSERT(pContext->recpump);

    if(pContext->startSettings.output.transport.segmented ||
      !pContext->startSettings.output.transport.file) {/* stream out or to segmented file */
        BKNI_CreateEvent(&pContext->recpumpEvent);
        NEXUS_Recpump_GetSettings(pContext->recpump, &recpumpSettings);
        /* NOTE: enable band hold to allow recpump to stall TS mux to avoid ave data corruption in case file i/o is slow
         * The encoders should make sure no output CDB/ITB buffer corruption (instead do frame drop) when output overflow! */
        recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
        recpumpSettings.data.dataReady.callback = BXCode_P_RecpumpDataready_callback;
        recpumpSettings.data.dataReady.context = pContext;
        recpumpSettings.index.dataReady.callback = BXCode_P_RecpumpDataready_callback; /* same callback */
        recpumpSettings.index.dataReady.context = pContext; /* same event */
        recpumpSettings.data.overflow.callback = BXCode_P_RecpumpOverflow_callback;
        recpumpSettings.data.overflow.context = "data";
        recpumpSettings.data.overflow.param = pContext->id;
        recpumpSettings.index.overflow.callback = BXCode_P_RecpumpOverflow_callback;
        recpumpSettings.index.overflow.context = "index";
        recpumpSettings.index.overflow.param = pContext->id;
        NEXUS_Recpump_SetSettings(pContext->recpump, &recpumpSettings);
    } else {
        pContext->record = NEXUS_Record_Create();
        if(NULL==pContext->record) {BDBG_ERR(("TS mux failed to open record!")); return NEXUS_NOT_AVAILABLE;}

        NEXUS_Record_GetSettings(pContext->record, &recordSettings);
        recordSettings.recpump = pContext->recpump;
        /* NOTE: enable band hold to allow recpump to stall TS mux to avoid ave data corruption in case file i/o is slow
         * The encoders should make sure no output CDB/ITB buffer corruption (instead do frame drop) when output overflow! */
        recordSettings.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
        recordSettings.pollingTimer = 50;
        recordSettings.writeAllTimeout = 100;
        NEXUS_Record_SetSettings(pContext->record, &recordSettings);
    }

    /*******************************
     * create system data PAT/PMT
     */
    BXCode_P_CreateSystemData(pContext);

    /*******************************
     *  TS user data pass through setup (only for non-FNRT)
     */
    if(pContext->numUserDataPids && pContext->openSettings.vpipes <= 1) {
        unsigned i;
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;
        NEXUS_PlaypumpOpenPidChannelSettings playpumpPidSettings;
        NEXUS_PidChannelSettings pidSettings;

        NEXUS_Message_GetDefaultSettings(&messageSettings);
        /* SCTE 270 spec max TS VBI user data bitrate=270Kbps, 256KB buffer can hold 7.5 seconds;
           worthy user data for video synchronization; TODO: may be reduced if unnecessary */
        messageSettings.bufferSize = 512*1024;
        messageSettings.maxContiguousMessageSize = 0; /* to support TS capture and in-place operation */
        messageSettings.overflow.callback = BXCode_P_MessageOverflowCallback; /* report overflow error */
        messageSettings.overflow.context  = pContext;

        /* open source user data PID channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther; /* capture the TS packets with the user data PES */
        playbackPidSettings.pidSettings.pidSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidSettings);
        playpumpPidSettings.pidSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;
        NEXUS_PidChannel_GetDefaultSettings(&pidSettings);
        pidSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE;

        for(i = 0; i < NEXUS_MAX_MUX_PIDS; i++) {
            if(pContext->userDataStreamValid[i]) {
                messageSettings.overflow.param = pContext->startSettings.input.userdataPid[i];
                BDBG_MSG(("BXCode%d opened message buffer for input user data PID %d", pContext->id,
                    pContext->startSettings.input.userdataPid[i]));
                muxConfig.userdata[i].message = NEXUS_Message_Open(&messageSettings);
                BDBG_ASSERT(muxConfig.userdata[i].message);
                pContext->userDataMessage[i] = muxConfig.userdata[i].message;
                /* remap PID only if different (NOTE: 7425 pid remapping may drop packets if i/o pids are the same) */
                if(pContext->startSettings.output.transport.userdataPid[i] &&
                    (pContext->startSettings.output.transport.userdataPid[i] != pContext->startSettings.input.userdataPid[i])) {
                    playbackPidSettings.pidSettings.pidSettings.remap.enabled = true;
                    playbackPidSettings.pidSettings.pidSettings.remap.pid     = pContext->startSettings.output.transport.userdataPid[i];
                    BDBG_MSG(("output user data PID %u", playbackPidSettings.pidSettings.pidSettings.remap.pid));
                }

                if(pContext->startSettings.input.type == BXCode_InputType_eFile && pContext->openSettings.vpipes<=1) {
                    pContext->pidChannelUserData[i] = NEXUS_Playback_OpenPidChannel(
                        pContext->playback, pContext->startSettings.input.userdataPid[i], &playbackPidSettings);
                } else if(pContext->startSettings.input.type == BXCode_InputType_eStream) {
                    pContext->pidChannelUserData[i] = NEXUS_Playpump_OpenPidChannel(
                        pContext->playpump, pContext->startSettings.input.userdataPid[i], &playpumpPidSettings);
                } else {/* live */
                    pContext->pidChannelUserData[i] = NEXUS_PidChannel_Open(
                        pContext->startSettings.input.parserBand, pContext->startSettings.input.userdataPid[i], &pidSettings);
                }

                /* must start message before stream mux starts */
                NEXUS_Message_GetDefaultStartSettings(muxConfig.userdata[i].message, &messageStartSettings);
                messageStartSettings.format = NEXUS_MessageFormat_eTs;
                messageStartSettings.pidChannel = pContext->pidChannelUserData[i];
                NEXUS_Message_Start(muxConfig.userdata[i].message, &messageStartSettings);

                /* open transcode mux output user data PidChannels out of system data channel */
                pContext->pidChannelMuxUserData[i] = NEXUS_Playpump_OpenPidChannel(pContext->playpumpMuxSystem,
                    pContext->startSettings.output.transport.userdataPid[i], NULL);
                BDBG_ASSERT(pContext->pidChannelMuxUserData[i]);
            }
        }
    }

    /* store the mux config */
    pContext->tsMuxConfig = muxConfig;

    /* open pcr PidChannels */
    pContext->pidChannelMuxPcr = NEXUS_Playpump_OpenPidChannel(pContext->playpumpMuxSystem, muxConfig.pcr.pid, NULL);
    if(NULL==pContext->pidChannelMuxPcr) {BDBG_ERR(("TS mux failed to open pcr pid channel!")); return NEXUS_NOT_AVAILABLE;}

    /* open pat/pmt pid channels if not segmented ts output (segmented stream use manual psi insertion based on RAI index) */
    if(!pContext->startSettings.output.transport.segmented && pContext->startSettings.output.transport.pmtPid) {
        pContext->pidChannelMuxPat = NEXUS_Playpump_OpenPidChannel(pContext->playpumpMuxSystem, 0, NULL);
        if(NULL==pContext->pidChannelMuxPat) {BDBG_ERR(("TS mux failed to open PAT pid channel!")); return NEXUS_NOT_AVAILABLE;}
        pContext->pidChannelMuxPmt = NEXUS_Playpump_OpenPidChannel(pContext->playpumpMuxSystem, pContext->startSettings.output.transport.pmtPid, NULL);
        if(NULL==pContext->pidChannelMuxPmt) {BDBG_ERR(("TS mux failed to open PAT pid channel!")); return NEXUS_NOT_AVAILABLE;}
    }

    /* set record index file name and open the record file handle */
    if((pContext->startSettings.output.video.encoder.codec!=NEXUS_VideoCodec_eMpeg2) &&
       (pContext->startSettings.output.video.encoder.codec!=NEXUS_VideoCodec_eH264))
    {
        BDBG_WRN(("no index record"));
    }

    return NEXUS_SUCCESS;
}

void bxcode_start_mux(
    BXCode_P_Context  *pContext )
{
    unsigned i;
    if(pContext->startSettings.output.transport.type == BXCode_OutputType_eTs) {/* TS mux */
        NEXUS_StreamMuxOutput muxOutput;

        /* non-segmented ts output use timer-based psi insertion */
        if(!pContext->startSettings.output.transport.segmented && pContext->startSettings.output.transport.pmtPid) {
            BXCode_P_StartSystemdata(pContext);
        }
        pContext->tsMuxConfig.stcChannel = pContext->stcChannelEncoder;
        NEXUS_StreamMux_Start(pContext->streamMux,&pContext->tsMuxConfig, &muxOutput);
        pContext->pidChannelMuxVideo = muxOutput.video[0];
        BDBG_MSG(("video pid channel %p", pContext->pidChannelMuxVideo));

        /* add multiplex data to the same record */
        if(!pContext->startSettings.output.transport.segmented &&
            pContext->startSettings.output.transport.file) {/* record to non-segmented file */
            NEXUS_RecordPidChannelSettings recordPidSettings;

            /* configure the video pid for indexing */
            if(pContext->startSettings.output.video.pid) {
                NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
                recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
                recordPidSettings.recpumpSettings.pidTypeSettings.video.index = /* RAVE only support mpeg2&avc indexing */
                        (pContext->startSettings.output.video.encoder.codec==NEXUS_VideoCodec_eMpeg2) ||
                        (pContext->startSettings.output.video.encoder.codec==NEXUS_VideoCodec_eH264);
                recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = pContext->startSettings.output.video.encoder.codec;
                NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelMuxVideo, &recordPidSettings);
                BDBG_MSG(("BXCode[%u] record added video pid channel %p", pContext->id, pContext->pidChannelMuxVideo));
            }

            for(i=0; i < BXCODE_MAX_AUDIO_PIDS && pContext->startSettings.output.audio[i].pid; i++) {
                pContext->pidChannelMuxAudio[i] = muxOutput.audio[i];
                NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelMuxAudio[i], NULL);
                BDBG_MSG(("BXCode[%u] record added audio pid channel[%u] %p", pContext->id, i, pContext->pidChannelMuxAudio[i]));
            }
            for(i=0; i < NEXUS_MAX_MUX_PIDS && pContext->userDataStreamValid[i]; i++) {
                NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelMuxUserData[i], NULL);
                BDBG_MSG(("BXCode[%u] record added user data pid channel[%u] %p", pContext->id, i, pContext->pidChannelMuxUserData[i]));
            }
            NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelMuxPcr, NULL);
            /* non-segmented ts output use timer-based psi insertion */
            if(pContext->startSettings.output.transport.pmtPid) {
                NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelMuxPat, NULL);
                NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelMuxPmt, NULL);
            }

            /* Start record of stream mux output */
            if(pContext->startSettings.input.loop) {
                NEXUS_FifoRecordSettings fifoRecordSettings;

                pContext->fifoRecord = NEXUS_FifoRecord_Create(pContext->startSettings.output.transport.file,
                    pContext->startSettings.output.transport.index);

                NEXUS_FifoRecord_GetSettings(pContext->fifoRecord, &fifoRecordSettings);
                fifoRecordSettings.interval = 60;/* 60 second fifo */
                BDBG_ASSERT(!NEXUS_FifoRecord_SetSettings(pContext->fifoRecord, &fifoRecordSettings));

                pContext->fileRecord = NEXUS_FifoRecord_GetFile(pContext->fifoRecord);
                BDBG_MSG(("Opened record fifo file."));
            }
            else {
                pContext->fileRecord = NEXUS_FileRecord_OpenPosix(pContext->startSettings.output.transport.file,
                    pContext->startSettings.output.transport.index);
            }
            NEXUS_Record_Start(pContext->record, pContext->fileRecord);
        } else {/* stream output or segmented output */
            NEXUS_RecpumpAddPidChannelSettings recpumpAddPidSettings;
            NEXUS_RecpumpTpitFilter filter;

            /* configure the video pid for indexing */
            if(pContext->startSettings.output.video.pid) {
                NEXUS_Recpump_GetDefaultAddPidChannelSettings(&recpumpAddPidSettings);
                recpumpAddPidSettings.pidType = NEXUS_PidType_eVideo;
                recpumpAddPidSettings.pidTypeSettings.video.index = false; /* TODO: add nav index */
                recpumpAddPidSettings.pidTypeSettings.video.codec = pContext->startSettings.output.video.encoder.codec;
                NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelMuxVideo, &recpumpAddPidSettings);
                BDBG_MSG(("BXCode[%u] recpump added video pid channel %p", pContext->id, pContext->pidChannelMuxVideo));
            }

            for(i=0; i < BXCODE_MAX_AUDIO_PIDS && pContext->startSettings.output.audio[i].pid; i++) {
                pContext->pidChannelMuxAudio[i] = muxOutput.audio[i];
                NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelMuxAudio[i], NULL);
                BDBG_MSG(("BXCode[%u] recpump added audio pid channel[%u] %p", pContext->id, i, pContext->pidChannelMuxAudio[i]));
            }
            for(i=0; i < NEXUS_MAX_MUX_PIDS && pContext->userDataStreamValid[i]; i++) {
                NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelMuxUserData[i], NULL);
                BDBG_MSG(("BXCode[%u] recpump added user data pid channel[%u] %p", pContext->id, i, pContext->pidChannelMuxUserData[i]));
            }
            NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelMuxPcr, NULL);
            /* non-segmented ts output use timer-based psi insertion */
            if(!pContext->startSettings.output.transport.segmented) {
                NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelMuxPat, NULL);
                NEXUS_Recpump_AddPidChannel(pContext->recpump, pContext->pidChannelMuxPmt, NULL);
            }

            /* Start record of stream mux output */
            /* HLS NOTE: set up TPIT filter to grep RAI to index HLS segments (one RAI per segment */
            if(pContext->startSettings.output.transport.segmented) {
                NEXUS_Recpump_GetDefaultTpitFilter(&filter);
                filter.config.mpeg.randomAccessIndicatorEnable = true;
                filter.config.mpeg.randomAccessIndicatorCompValue = true;
                NEXUS_Recpump_SetTpitFilter(pContext->recpump, pContext->pidChannelMuxVideo, &filter);

                /* HLS NOTE: since encoder GOP setting is infinite IPPP, the segment boundary I-frame is the only I of a segment;
                   or all the I-frames in TS output are at the starting of each segments! Here we reuse the nexus record module to
                   produce the TS record file while the index output file's I-frame index tells segment boundary. If the HLS transcode
                   output is directly used for HLS streaming, app may choose to use recpump and disables SCD index but enables
                   RAI TPIT filter to detect the TS output segment boundaries on the fly! See /nexus/examples/transport/tpit.c for
                   the setup of recpump TPIT filter. */
                if(pContext->startSettings.output.transport.file) { /* segmented ts file record */
                    int rc;
                    char indexFile[80];
                    pContext->fdTsRec = open(pContext->startSettings.output.transport.file, O_CREAT | O_TRUNC | O_RDWR | O_LARGEFILE, 0666);
                    if (pContext->fdTsRec<0) { BERR_TRACE(NEXUS_NOT_AVAILABLE); BDBG_ASSERT(0); }
                    rc = fcntl(pContext->fdTsRec, F_GETFL);
                    if ((int)rc != -1) {
                        rc = fcntl(pContext->fdTsRec, F_SETFL, rc | FD_CLOEXEC);
                    }
                    if(pContext->startSettings.output.transport.index) { /* segmented ts file record */
                        BKNI_Snprintf(indexFile, 80, "%s", pContext->startSettings.output.transport.index);
                    } else {
                        BKNI_Snprintf(indexFile, 80, "%s.hls", pContext->startSettings.output.transport.file);
                        BDBG_WRN(("Didn't specify output index file name. Will use %s as segments index file!", indexFile));
                    }
                    pContext->fpIndexRec = fopen(indexFile, "w");
                    if (!pContext->fpIndexRec) { BERR_TRACE(NEXUS_NOT_AVAILABLE); BDBG_ASSERT(0); }

                    pContext->recpumpThread = B_Thread_Create("Segmented TS recpump thread", (B_ThreadFunc)BXCode_P_Recpump_thread, pContext, NULL);
                }
            }
            NEXUS_Recpump_Start(pContext->recpump);

            /* if stream output, start output desc fifo for output buffer API and init stream output */
            if(!pContext->startSettings.output.transport.file) {
                BFIFO_INIT(&pContext->outputDescFifo, pContext->outputDescs, BXCODE_P_OUTPUT_DESC_FIFO_SIZE);
                pContext->firstRai = true;
                pContext->totalRecordBytes = 0;
                pContext->ccValue = 0;
            }
        }
    } else {/* MP4 mux */
        NEXUS_FileMux_Start(pContext->fileMux,&pContext->mp4MuxConfig, pContext->fileMuxOutput);
    }
}

void bxcode_stop_mux(
    BXCode_P_Context  *pContext )
{
    BXCode_StartSettings *pSettings = &pContext->startSettings;
    if(pSettings->output.transport.type == BXCode_OutputType_eTs) {
        unsigned i;
        /* non-segmented ts output use timer-based psi insertion */
        if(!pContext->startSettings.output.transport.segmented && pContext->startSettings.output.transport.pmtPid) {
            BXCode_P_StopSystemdata(pContext);
        }
        BDBG_MSG(("bxcode[%u] to finish TS mux", pContext->id));
        /* only wait for mux finish with file output; otherwise, stop ASAP! */
        if(pSettings->input.type == BXCode_InputType_eFile && pSettings->output.transport.file) {
            NEXUS_StreamMux_Finish(pContext->streamMux);
            if(BKNI_WaitForEvent(pContext->finishEvent, 5000)!=BERR_SUCCESS) {
                BDBG_WRN(("stream mux finish TIMEOUT"));
            }
        }
        if(!pContext->startSettings.output.transport.segmented &&
            pContext->startSettings.output.transport.file) {/* record to non-segmented file */
            BDBG_MSG(("bxcode[%u] to close output TS record file", pContext->id));
            NEXUS_Record_Stop(pContext->record);
            NEXUS_FileRecord_Close(pContext->fileRecord);
            NEXUS_Record_RemoveAllPidChannels(pContext->record);
            NEXUS_Record_Destroy(pContext->record);
        } else {/* TS stream or segmeneted file output */
            NEXUS_Recpump_Stop(pContext->recpump);
            NEXUS_Recpump_RemoveAllPidChannels(pContext->recpump);
            BKNI_DestroyEvent(pContext->recpumpEvent);
            if(pContext->startSettings.output.transport.segmented && pContext->fdTsRec>=0) {
                if(pContext->recpumpThread) {B_Thread_Destroy(pContext->recpumpThread); pContext->recpumpThread = NULL;}
                if(pContext->fpIndexRec) {fclose(pContext->fpIndexRec); pContext->fpIndexRec = NULL;}
                close(pContext->fdTsRec);
            }
        }

        /*****************************************
         * Note: remove all record PID channels before stream
         * mux stop since streammux would close the A/V PID channels
         */

        NEXUS_StreamMux_Stop(pContext->streamMux);
        NEXUS_StreamMux_Destroy(pContext->streamMux);
        /* stop encoder in abort mode to drain all encoder output */
        if(pSettings->output.video.pid && (pSettings->input.type != BXCode_InputType_eFile ||
            (pSettings->output.transport.type != BXCode_OutputType_eTs || !pSettings->output.transport.file)))
        {
            NEXUS_VideoEncoderStopSettings stopSettings;
            NEXUS_VideoEncoder_GetDefaultStopSettings(&stopSettings);
            /* only file-to-file transcode use normal stop mode to get whole eof; otherwise, stop AFAP. */
            stopSettings.mode = NEXUS_VideoEncoderStopMode_eAbort;
            NEXUS_VideoEncoder_Stop(pContext->video[0].encoder, &stopSettings);
            BDBG_MSG(("bxcode[%u] video encoder[0] stopped in abort mode", pContext->id));
        }

        if(pContext->startSettings.output.video.pid) {
            NEXUS_Playpump_Close(pContext->playpumpMuxVideo);
        }
        for(i=0; i < BXCODE_MAX_AUDIO_PIDS && pSettings->output.audio[i].pid; i++) {
            NEXUS_Playpump_Close(pContext->playpumpMuxAudio[i]);
        }

        /*******************************
         * create system data PAT/PMT
         */
        BXCode_P_DestroySystemData(pContext);

        /************************************
         * Bring down transcoder
         */
        for(i = 0; i < pContext->numUserDataPids && pContext->openSettings.vpipes<=1; i++) {
            if(pContext->userDataStreamValid[i]) {
                NEXUS_Message_Stop(pContext->userDataMessage[i]);
                NEXUS_Message_Close(pContext->userDataMessage[i]);
            }
        }
        /* close all system data PIDs */
        NEXUS_Playpump_CloseAllPidChannels(pContext->playpumpMuxSystem);
        NEXUS_Playpump_Close(pContext->playpumpMuxSystem);
    } else if(pSettings->output.transport.type == BXCode_OutputType_eMp4File) {
        NEXUS_FileMux_Finish(pContext->fileMux);
        while(BKNI_WaitForEvent(pContext->finishEvent, 5000)!=BERR_SUCCESS) {
            BDBG_WRN(("mp4 file mux finish TIMEOUT! Continue to try..."));
        }
        NEXUS_FileMux_Stop(pContext->fileMux);
        NEXUS_MuxFile_Close(pContext->fileMuxOutput);
        NEXUS_FileMux_Destroy(pContext->fileMux);
    }
    BKNI_DestroyEvent(pContext->finishEvent);
}
#endif /* NEXUS_HAS_VIDEO_ENCODER */
