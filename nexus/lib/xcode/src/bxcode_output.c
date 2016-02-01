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

BDBG_MODULE(bxcode_output);
BDBG_FILE_MODULE(bxcode_segment);

/* to derive AV sync coorelation error */
static void BXCode_P_avsync_correlation_error( BXCode_P_Context *pContext, BXCode_OutputStatus *pStatus)
{
    size_t size[2];
    const NEXUS_VideoEncoderDescriptor *vdesc[2];
    const NEXUS_AudioMuxOutputFrame *adesc[2];
    unsigned i, j, audioId;
    uint32_t v_opts, a_opts;
    uint64_t v_pts, a_pts, v_stc, a_stc;
    double v_opts2pts=0, a_opts2pts=0;
    bool validVframe = false, validAframe = false;

    NEXUS_VideoEncoder_GetBuffer(pContext->video[0].encoder, &vdesc[0], &size[0], &vdesc[1], &size[1]);
    for(j=0;j<2 && !validVframe;j++) {
        for(i=0;i<size[j];i++) {
            if(vdesc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID)
            {
                v_opts = vdesc[j][i].originalPts;
                v_pts  = vdesc[j][i].pts;
                v_stc  = vdesc[j][i].stcSnapshot;
                v_opts2pts = (double)v_pts / 90 - (double)v_opts / 45;
                BDBG_MSG(("V: opts=%u pts=%llu, stc=%llu, opts2pts=%f", v_opts, v_pts, v_stc, v_opts2pts));
                validVframe = true;
                break;
            }
        }
    }
    for(audioId=0; audioId<BXCODE_MAX_AUDIO_PIDS && pContext->startSettings.output.audio[audioId].pid; audioId++) {
        NEXUS_AudioMuxOutput_GetBuffer(pContext->audio[audioId].muxOutput, &adesc[0], &size[0], &adesc[1], &size[1]);
        for(j=0;j<2 && !validAframe;j++) {
            for(i=0;i<size[j];i++) {
                if(adesc[j][i].flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_PTS_VALID)
                {
                    a_opts = adesc[j][i].originalPts;
                    a_pts  = adesc[j][i].pts;
                    a_stc  = adesc[j][i].stcSnapshot;
                    a_opts2pts = (double)a_pts / 90 - (double)a_opts / 45;
                    BDBG_MSG(("A[%u]: opts=%u pts=%llu, stc=%llu, opts2pts=%f", audioId, a_opts, a_pts, a_stc, a_opts2pts));
                    validAframe = true;
                    break;
                }
            }
        }
        if(validVframe && validAframe) {
            pStatus->avSyncErr[audioId] = a_opts2pts - v_opts2pts;
            BDBG_MSG(("AV sync correlation error[%u] (positive means video leads): %.1f ms", audioId, pStatus->avSyncErr[audioId]));
            if(pStatus->avSyncErr[audioId] > 20 || pStatus->avSyncErr[audioId] < -20)
                BDBG_MSG(("AV sync error[%u] %.1f ms > 20ms!", audioId, pStatus->avSyncErr[audioId]));
        }
    }
}

/**
Summary:
**/
NEXUS_Error BXCode_GetOutputStatus(
    BXCode_Handle        handle,
    BXCode_OutputStatus *pStatus /* [out] */
    )
{
    NEXUS_Error rc;
    unsigned i;
    NEXUS_VideoEncoderStatus videoStatus;
    NEXUS_VideoEncoderClearStatus clearStatus;
    NEXUS_AudioMuxOutputStatus audioStatus;
    NEXUS_StreamMuxStatus tsMuxStatus;
    NEXUS_FileMuxStatus mp4MuxStatus;

    BDBG_OBJECT_ASSERT(handle, bxcode);
    BDBG_ASSERT(pStatus);

    if(!handle->started) {
        BDBG_WRN(("BXCode%u is not started yet!", handle->id));
        return NEXUS_NOT_AVAILABLE;
    }
    BKNI_Memset(pStatus, 0, sizeof(BXCode_OutputStatus));

    /* video encoder status */
    if(handle->startSettings.output.video.pid) {
        rc = NEXUS_VideoEncoder_GetStatus(handle->video[0].encoder, &videoStatus);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_VideoEncoder_GetStatus error!")); return BERR_TRACE(rc);}
        pStatus->video.version.firmware = videoStatus.version.firmware;
        pStatus->video.data.fifoSize = videoStatus.data.fifoSize;
        pStatus->video.data.fifoDepth = videoStatus.data.fifoDepth;
        pStatus->video.index.fifoSize = videoStatus.index.fifoSize;
        pStatus->video.index.fifoDepth = videoStatus.index.fifoDepth;
        pStatus->video.errorCount = videoStatus.errorCount;
        pStatus->video.errorFlags = videoStatus.errorFlags;
        pStatus->video.eventFlags = videoStatus.eventFlags;
        pStatus->video.pictureIdLastEncoded = videoStatus.pictureIdLastEncoded;
        pStatus->video.picturesEncoded   = videoStatus.picturesEncoded;
        pStatus->video.picturesPerSecond = videoStatus.picturesPerSecond;
        pStatus->video.picturesReceived  = videoStatus.picturesReceived;
        pStatus->video.picturesDroppedErrors = videoStatus.picturesDroppedErrors;
        pStatus->video.picturesDroppedFRC    = videoStatus.picturesDroppedFRC;
        pStatus->video.picturesDroppedHRD    = videoStatus.picturesDroppedHRD;
        pStatus->video.enabled = handle->settings.video.enabled;
        pStatus->video.pBufferBase = handle->video[0].pEncoderBufferBase;
        pStatus->video.pMetadataBufferBase = handle->video[0].pEncoderMetadataBufferBase;
        NEXUS_VideoEncoder_GetDefaultClearStatus(&clearStatus);
        NEXUS_VideoEncoder_ClearStatus(handle->video[0].encoder, &clearStatus);
    }

    /* audio encoder output status */
    for(i=0; i<BXCODE_MAX_AUDIO_PIDS && handle->startSettings.output.audio[i].pid; i++) {
        rc = NEXUS_AudioMuxOutput_GetStatus(handle->audio[i].muxOutput, &audioStatus);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_AudioMuxOutput_GetStatus error!")); return BERR_TRACE(rc);}
        pStatus->audio[i].numFrames = audioStatus.numFrames;
        pStatus->audio[i].numErrorFrames = audioStatus.numErrorFrames;
        pStatus->audio[i].data.fifoSize = audioStatus.data.fifoSize;
        pStatus->audio[i].data.fifoDepth = audioStatus.data.fifoDepth;
        pStatus->audio[i].enabled = handle->settings.audio[i].enabled;
        pStatus->numAudios++;
        pStatus->audio[i].pBufferBase = handle->audio[i].pEncoderBufferBase;
        pStatus->audio[i].pMetadataBufferBase = handle->audio[i].pEncoderMetadataBufferBase;
    }

    /* mux output status */
    switch(handle->startSettings.output.transport.type) {
    case BXCode_OutputType_eTs:
        rc = NEXUS_StreamMux_GetStatus(handle->streamMux, &tsMuxStatus);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_StreamMux_GetStatus error!")); return BERR_TRACE(rc);}
        pStatus->mux.duration = tsMuxStatus.duration;
        rc = NEXUS_Recpump_GetStatus(handle->recpump, &pStatus->mux.recpumpStatus);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_Recpump_GetStatus error!")); return BERR_TRACE(rc);}
        BXCode_P_avsync_correlation_error(handle, pStatus);
        break;
    case BXCode_OutputType_eMp4File:
        rc = NEXUS_FileMux_GetStatus(handle->fileMux, &mp4MuxStatus);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_FileMux_GetStatus error!")); return BERR_TRACE(rc);}
        pStatus->mux.duration = mp4MuxStatus.duration;
        BXCode_P_avsync_correlation_error(handle, pStatus);
        break;
    case BXCode_OutputType_eEs:
        BXCode_P_avsync_correlation_error(handle, pStatus);
        break;
    default:
        break;
    }
    return NEXUS_SUCCESS;
}

#define BXCODE_P_GOPS_IN_A_SEGMENT         1
#define BXCODE_P_TPIT_ENTRY_SIZE           24

static NEXUS_Error BXCode_P_WriteOutputDesc(
    BXCode_Handle      handle,
    unsigned           flag,
    const void        *ptr,
    size_t             size)
{
    if(BFIFO_WRITE_LEFT(&handle->outputDescFifo)==0) {
        BDBG_WRN(("Output descriptors FIFO is full!")); return NEXUS_NOT_AVAILABLE;
    } else {/* there is space to write */
        BFIFO_WRITE(&handle->outputDescFifo)->flags = flag;
        BFIFO_WRITE(&handle->outputDescFifo)->pData = ptr;
        BFIFO_WRITE(&handle->outputDescFifo)->size  = size;
        BFIFO_WRITE_COMMIT(&handle->outputDescFifo, 1);
        BDBG_MSG(("output desc with flag=%#x, addr=%p, size=%#x bytes .", flag, ptr, size));
        return NEXUS_SUCCESS;
    }
}

static NEXUS_Error BXCode_Output_P_GetSegmentDescriptors(
    BXCode_Handle                     handle,
    const BXCode_OutputTsDescriptor **pBuffer,  /* [out] pointer to BXCode_OutputTsDescriptor structs */
    size_t                           *pSize,    /* [out] number of BXCode_OutputTsDescriptor elements in pBuffer */
    const BXCode_OutputTsDescriptor **pBuffer2, /* [out] pointer to BXCode_OutputTsDescriptor structs */
    size_t                           *pSize2    /* [out] number of BXCode_OutputTsDescriptor elements in pBuffer2 */
    )
{
    NEXUS_Error rc;
    off_t bytesRecordedTillCurrentRai=0;
    const void *buffer, *indexBuf;
    void *dataWp, *indexWp;
    size_t datasize=0, indexsize=0;

    /* init with empty return */
    *pSize = *pSize2 = 0;
    *pBuffer = *pBuffer2 = NULL;

    /* output the first PAT/PMT descriptor */
    if(0==handle->ccValue) {
        rc = BXCode_P_WriteOutputDesc(handle, BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_START,
            (uint8_t*)handle->psiPkt + 2*BXCODE_P_TSPKT_LENGTH*(handle->ccValue%BXCODE_P_PSI_QUEUE_CNT), 2*BXCODE_P_TSPKT_LENGTH);
        if(NEXUS_SUCCESS==rc) {
            BDBG_MSG(("0) output PSI desc for initial segment %u.", handle->ccValue));
            BXCode_P_UpdateSystemData(handle);/* incrment cc value of psi */
        } else return BERR_TRACE(rc);
    }

    /* CDB data */
    rc = NEXUS_Recpump_GetDataBuffer(handle->recpump, &buffer, &datasize);
    if(rc) { BDBG_ERR(("NEXUS_Recpump_GetDataBuffer[%u] returns error!", handle->id)); return BERR_TRACE(rc); }

    /* ITB contains TPIT results */
    rc = NEXUS_Recpump_GetIndexBuffer(handle->recpump, &indexBuf, &indexsize);
    if(rc) { BDBG_ERR(("NEXUS_Recpump_GetIndexBuffer[%u] returns error!", handle->id)); return BERR_TRACE(rc); }

    /* consume as many index/data as possible for each Get */
    /* handle overlapped buffers between Gets before new ReadComplete:
           1) save index wp up to last Get(lastIndexWp);
           2) save data wp up to last Get (lastDataWp);
           3) now in Get, skip all indices already in output desc FIFO previously: |..R...W..W'..|; just get (W...W') indices, TODO: consider W' wraps.
           4) skip all data already in output desc FIFO: |..R...W..W'..|; just get (W...W') data, TODO: consider possible wrap of W'.
           where,
              R - buffer ptr returned by current Get;
              W - buffer end ptr in the last Get; could colocate with R if last ReadComplete consumed all;
              W' - buffer end ptr in the current Get;
     */

    do {
        if (indexsize) {/* assume one RAI index per segment */
            off_t highByte;

            /* skip old index parsed last time */
            BDBG_MSG(("indexsize = %u, lastIdxWp=%p, indexBuf=%p", indexsize, handle->lastIndexWp, indexBuf));
            indexWp = (uint8_t*)indexBuf + indexsize; /* index W' */
            if(handle->lastIndexWp && handle->lastIndexWp <= indexWp) {/* skip any indices already parsed in the past */
                indexBuf = handle->lastIndexWp;
                indexsize = (uint8_t*)indexWp - (uint8_t*)indexBuf;
            }
            handle->lastIndexWp = indexWp;/* lastIndexWp updated here */

            if(indexsize) {
                /* byte offset since the start of the record session */
                highByte = ((off_t)*((uint32_t*)indexBuf+2) >> 24);
                bytesRecordedTillCurrentRai = highByte << 32;
                bytesRecordedTillCurrentRai |= (off_t)*((uint32_t*)indexBuf+3);
                BDBG_MSG(("Seg[%u] byte offset: 0x%llx", handle->ccValue, bytesRecordedTillCurrentRai));
                BDBG_MSG(("fifo: R[%x], W[%x]", BFIFO_READ(&handle->outputDescFifo), BFIFO_WRITE(&handle->outputDescFifo)));
                highByte = (bytesRecordedTillCurrentRai)>>32;

                /* HLS NOTE: update output descriptors fifo writer */
                BDBG_MSG(("RAI tpit entry: tipt[0] 0x%x, tpit[2] 0x%x, tpit[3] 0x%x", *(uint32_t*)indexBuf, *((uint32_t*)indexBuf+2), *((uint32_t*)indexBuf+3)));
            }
        }

        /* record the segment TS stream; also insert PAT&PMT at start of each segments */
        if (datasize) {
            /* skip old data Get last time */
            BDBG_MSG(("datasize = %#x, lastDataWp=%p, dataBuf=%p", datasize, handle->lastDataWp, buffer));
            dataWp = (uint8_t*)buffer + datasize; /* data W' */
            if(handle->lastDataWp && handle->lastDataWp <= dataWp) {/* skip any indices already parsed in the past */
                buffer = handle->lastDataWp;
                datasize = (uint8_t*)dataWp - (uint8_t*)buffer;
            }
            if(0 == datasize) continue;

            /* if dataSize + totalRecordBytes < bytesRecordedTillCurrentRai or no new index, write dataSize of current segment;
                else, write up to bytesRecordedTillCurrentRai, then PAT/PMT for the next segment; */
            if((0==indexsize) || (datasize + handle->totalRecordBytes < bytesRecordedTillCurrentRai)) {
                rc = BXCode_P_WriteOutputDesc(handle, 0, buffer, datasize);
                if(NEXUS_SUCCESS == rc) {
                    BDBG_MSG(("1) output %#x/%#llx bytes data desc.", datasize, handle->totalRecordBytes));
                    handle->totalRecordBytes += datasize;
                    handle->lastDataWp = dataWp;/* lastDataWp updated here */
                    datasize = 0;
                } else return BERR_TRACE(rc);
            } else {/* one or more indices */
                if(bytesRecordedTillCurrentRai > handle->totalRecordBytes) {
                    /* record rest of the previous segment before the new RAI packet and increment buffer pointer */
                    rc = BXCode_P_WriteOutputDesc(handle, handle->firstRai? 0:BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_END, buffer,
                             bytesRecordedTillCurrentRai - handle->totalRecordBytes);
                    if(NEXUS_SUCCESS == rc) {
                        buffer = (uint8_t*)buffer + bytesRecordedTillCurrentRai - handle->totalRecordBytes;
                        handle->lastDataWp = (void*)buffer;/* lastDataWp updated here */
                        BDBG_MSG(("2) output %#llx bytes data (%#llx - %#llx) seg %s desc.", bytesRecordedTillCurrentRai - handle->totalRecordBytes,
                            bytesRecordedTillCurrentRai, handle->totalRecordBytes, handle->firstRai?"":"end"));
                        /* update the unread data size */
                        datasize -= bytesRecordedTillCurrentRai - handle->totalRecordBytes;
                        handle->totalRecordBytes += bytesRecordedTillCurrentRai - handle->totalRecordBytes;
                    } else return BERR_TRACE(rc);
                }

                if(handle->firstRai) {/* first pat/pmt already written previously for the 1st segment */
                    handle->firstRai = false;/* clear it */
                    handle->firstRaiSeen = true;/* to consume first RAI index later after seen */
                    BDBG_MSG(("first RAI seen!"));
                    /* if first time, single index for 1st RAI, send all data */
                    if(BXCODE_P_TPIT_ENTRY_SIZE==indexsize && datasize && !handle->lastDataWp) {
                        rc = BXCode_P_WriteOutputDesc(handle, 0, buffer,datasize);
                        if(NEXUS_SUCCESS == rc) {
                            buffer = (uint8_t*)buffer + datasize;
                            handle->lastDataWp = dataWp;/* lastDataWp updated here */
                            BDBG_MSG(("4) output %#x/%#llx bytes data desc.", datasize, handle->totalRecordBytes));
                            /* update the unread data size */
                            handle->totalRecordBytes += datasize;
                            datasize = 0;
                        } else return BERR_TRACE(rc);
                    }
                } else {
                    /* insert the PAT & PMT, then update continuity counter */
                    rc = BXCode_P_WriteOutputDesc(handle, BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_START,
                       (uint8_t*)handle->psiPkt + 2*BXCODE_P_TSPKT_LENGTH*(handle->ccValue%BXCODE_P_PSI_QUEUE_CNT), 2*BXCODE_P_TSPKT_LENGTH);
                    if(NEXUS_SUCCESS == rc) {
                        BDBG_MSG(("3) output PSI desc for segment %u.", handle->ccValue));
                        BXCode_P_UpdateSystemData(handle);/* update cc value of psi */
                    } else return BERR_TRACE(rc);
                }
                /* consumed one RAI index */
                indexsize -= BXCODE_P_TPIT_ENTRY_SIZE;
                indexBuf   = (uint8_t*)indexBuf + BXCODE_P_TPIT_ENTRY_SIZE;
            }
        }
    } while(datasize);
    *pBuffer = BFIFO_READ(&handle->outputDescFifo);
    *pSize = BFIFO_READ_PEEK(&handle->outputDescFifo);

    if(handle->outputDescFifo.bf_write < handle->outputDescFifo.bf_read) {/* wrap */
        *pBuffer2 = &handle->outputDescs[0];
        *pSize2   = handle->outputDescFifo.bf_write - handle->outputDescs;
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error BXCode_Output_P_ReadCompleteSegmentDescriptors(
    BXCode_Handle                   handle,
    size_t                          descriptorsCompleted
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    size_t bytesRead = 0, desc=0;
    /* defered first RAI consumption since the actual first RAI index may show up later than the first segment start header descriptor
       which is always at start of stream. */
    if(handle->firstRaiSeen) {
        handle->firstRaiSeen = false;
        BDBG_MSG(("Consumed the 1st index"));
        rc = NEXUS_Recpump_IndexReadComplete(handle->recpump, BXCODE_P_TPIT_ENTRY_SIZE);
        if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_Recpump_IndexReadComplete[%u] returns error!", handle->id));}
    }
    while(BFIFO_READ_LEFT(&handle->outputDescFifo) && desc++ < descriptorsCompleted) {
        bytesRead += (0 == (BFIFO_READ(&handle->outputDescFifo)->flags & BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_START))?
            BFIFO_READ(&handle->outputDescFifo)->size : 0;
        BDBG_MSG(("Done desc with flag=%#x, addr=%p, size=%#x bytes, recpump data bytes = %#x.", BFIFO_READ(&handle->outputDescFifo)->flags,
            BFIFO_READ(&handle->outputDescFifo)->pData, BFIFO_READ(&handle->outputDescFifo)->size, bytesRead));
        /* consumed one TPIT index entry per segment start after first RAI seen. The first RAI is consumed later. */
        if(BFIFO_READ(&handle->outputDescFifo)->flags & BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_START) {
            if(handle->segmentsStartConsumed++) {
                BDBG_MSG(("Consumed one index"));
                rc = NEXUS_Recpump_IndexReadComplete(handle->recpump, BXCODE_P_TPIT_ENTRY_SIZE);
                if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_Recpump_IndexReadComplete[%u] returns error!", handle->id));}
            }
        }
        /* consume completed output descriptor */
        BFIFO_READ_COMMIT(&handle->outputDescFifo, 1);
    }
    if(bytesRead) {
        rc = NEXUS_Recpump_DataReadComplete(handle->recpump, bytesRead);
        if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_Recpump_DataReadComplete[%u] returns error!", handle->id));}
    }
    return rc;
}

/**
Summary:
Get stream output data descriptors if outputting to memory. the descriptors include address pointers to the output data buffer.
The output buffer will be allocated internally from nexus user heap. The ts output buffer might not be contiguous to support
internally manual insertion of PAT/PMT prior to each video RAI packets for each start of HLS segments.
**/
NEXUS_Error BXCode_Output_GetDescriptors(
    BXCode_Handle                   handle,
    BXCode_OutputStream             stream,   /* [in] */
    const void                    **pBuffer,  /* [out] pointer to output data descriptors structs array */
    size_t                         *pSize,    /* [out] number of output data descriptors in pBuffer */
    const void                    **pBuffer2, /* [out] pointer to output data descriptors structs array */
    size_t                         *pSize2    /* [out] number of output data descriptors in pBuffer2 */
    )
{
    BERR_Code rc;
    const void *pData[2];
    size_t size[2];

    BDBG_OBJECT_ASSERT(handle, bxcode);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pBuffer2);
    if(!handle->started) {
        BDBG_WRN(("BXCode%u is not started yet!", handle->id));
        return NEXUS_NOT_AVAILABLE;
    }

    switch(stream.type) {
    case BXCode_OutputStreamType_eAes:
        if(handle->audio[stream.id].muxOutput && handle->startSettings.output.audio[stream.id].pid) {
            rc = NEXUS_AudioMuxOutput_GetBuffer(handle->audio[stream.id].muxOutput,
                (const NEXUS_AudioMuxOutputFrame**)pBuffer, pSize, (const NEXUS_AudioMuxOutputFrame**)pBuffer2, pSize2);
            if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_AudioMuxOutput_GetBuffer[%u] returns error code %d!", stream.id, rc)); return BERR_TRACE(rc); }
        } else return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    case BXCode_OutputStreamType_eVes:
        if(handle->video[stream.id].encoder && handle->startSettings.output.video.pid) {
            rc = NEXUS_VideoEncoder_GetBuffer(handle->video[stream.id].encoder,
                (const NEXUS_VideoEncoderDescriptor**)pBuffer, pSize, (const NEXUS_VideoEncoderDescriptor**)pBuffer2, pSize2);
            if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_VideoEncoder_GetBuffer[%u] returns error code %d!", stream.id, rc)); return BERR_TRACE(rc); }
        } else return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    case BXCode_OutputStreamType_eTs:
        if(handle->recpump && handle->startSettings.output.transport.type == BXCode_OutputType_eTs) {
            /* segmented TS output with maunal insertion of PSI that occupies separate descriptors */
            if(handle->startSettings.output.transport.segmented) {
                rc = BXCode_Output_P_GetSegmentDescriptors(handle, (const BXCode_OutputTsDescriptor**)pBuffer, pSize, (const BXCode_OutputTsDescriptor**)pBuffer2, pSize2);
                if(NEXUS_SUCCESS != rc) { BDBG_ERR(("BXCode_Output_P_GetDescriptors[%u] returns error!", handle->id)); return BERR_TRACE(rc); }
            } else { /* else one descriptor before wraparpund, the other after wraparound: simply forward recpump output */
                rc = NEXUS_Recpump_GetDataBufferWithWrap(handle->recpump, &pData[0], &size[0], &pData[1], &size[1]);
                if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_Recpump_GetDataBufferWithWrap[%u] returns error code %d!", handle->id, rc)); return BERR_TRACE(rc); }
                *pSize   = 0;
                *pSize2  = 0;
                *pBuffer = &handle->outputDescs[0];
                *pBuffer2= NULL;
                if(size[0]) {
                    handle->outputDescs[0].pData = pData[0];
                    handle->outputDescs[0].size  = size[0];
                    *pSize   = 1;
                    BDBG_MSG(("1) bxcode%u got %#x bytes @%#x", handle->id, size[0], pData[0]));
                }
                if(size[1]) {
                    handle->outputDescs[*pSize].pData = pData[1];
                    handle->outputDescs[*pSize].size  = size[1];
                    BDBG_MSG(("2) bxcode%u got %#x bytes @%#x", handle->id, size[1], pData[1]));
                    *pSize   += 1;
                }
            }
        } else return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    default:
        BDBG_ERR(("Invalid output stream type: %d", stream.type));
        return NEXUS_INVALID_PARAMETER;
    }
    return NEXUS_SUCCESS;
}

/**
Summary:
Return output stream descriptors
**/
NEXUS_Error BXCode_Output_ReadComplete(
    BXCode_Handle           handle,
    BXCode_OutputStream     stream,   /* [in] */
    unsigned                descriptorsCompleted /* must be <= *pSize + *pSize2 returned by last BXCode_Output_GetDescriptors call. */
    )
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(handle, bxcode);
    if(!handle->started) {
        BDBG_WRN(("BXCode%u is not started yet!", handle->id));
        return NEXUS_NOT_AVAILABLE;
    }

    switch(stream.type) {
    case BXCode_OutputStreamType_eAes:
        if(handle->audio[stream.id].muxOutput && handle->startSettings.output.audio[stream.id].pid) {
            rc = NEXUS_AudioMuxOutput_ReadComplete(handle->audio[stream.id].muxOutput, descriptorsCompleted);
            if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_AudioMuxOutput_ReadComplete[%u] returns error!", stream.id)); return BERR_TRACE(rc); }
        } else return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    case BXCode_OutputStreamType_eVes:
        if(handle->video[stream.id].encoder && handle->startSettings.output.video.pid) {
             rc = NEXUS_VideoEncoder_ReadComplete(handle->video[stream.id].encoder, descriptorsCompleted);
            if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_VideoEncoder_ReadComplete[%u] returns error!", stream.id)); return BERR_TRACE(rc); }
        } else return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    case BXCode_OutputStreamType_eTs:
        if(handle->recpump && handle->startSettings.output.transport.type == BXCode_OutputType_eTs) {
            /* segmented TS output with maunal insertion of PSI that occupies separate descriptors */
            if(handle->startSettings.output.transport.segmented) {
                rc = BXCode_Output_P_ReadCompleteSegmentDescriptors(handle, descriptorsCompleted);
                if(NEXUS_SUCCESS != rc) { BDBG_ERR(("BXCode_Output_P_GetDescriptors[%u] returns error!", handle->id)); return BERR_TRACE(rc); }
            } else { /* else one descriptor before wraparpund, the other after wraparound: simply forward recpump output */
                size_t bytesRead=0;
                switch(descriptorsCompleted) {
                case 1:
                    bytesRead = handle->outputDescs[0].size; break;
                case 2:
                    bytesRead = handle->outputDescs[0].size + handle->outputDescs[1].size; break;
                default:
                    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
                BDBG_MSG(("bxcode%u completed %#x bytes", handle->id, bytesRead));
                rc = NEXUS_Recpump_DataReadComplete(handle->recpump, bytesRead);
                if(NEXUS_SUCCESS != rc) { BDBG_ERR(("NEXUS_Recpump_DataReadComplete[%u] returns error!", handle->id)); return BERR_TRACE(rc); }
            }
        } else return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    default:
        BDBG_ERR(("Invalid output stream type: %d", stream.type));
        return NEXUS_INVALID_PARAMETER;
    }
    return NEXUS_SUCCESS;
}
#endif /* NEXUS_HAS_VIDEO_ENCODER */
