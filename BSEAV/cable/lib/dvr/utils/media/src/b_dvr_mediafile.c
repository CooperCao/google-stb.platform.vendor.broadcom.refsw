/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
#include "nexus_memory.h"
#include "b_dvr_mediafile.h"
#include "b_dvr_playbackservice.h"
#include "b_dvr_manager.h"
#include "b_dvr_manager_priv.h"
#include "b_dvr_segmentedfile.h"
#include "b_dvr_tsbservice.h"
#include "b_dvr_file.h"
#include "b_dvr_drmservice.h"
#define  INSERT_SEQEND_HEADER 0
#define  PTS_PACING   0
#define  MAX_SKIP_RETRY_COUNT 10     // Retry count to attain desired PTS skip time

BDBG_MODULE(b_dvr_mediafile);
BDBG_OBJECT_ID(B_DVR_MediaFile);
typedef struct B_DVR_MediaFile
{
    BDBG_OBJECT(B_DVR_MediaFile)
    unsigned index;
    char mediaName[B_DVR_MAX_FILE_NAME_LENGTH]; 
    B_DVR_MediaNode mediaNode;
    B_DVR_MediaFileOpenMode openMode;
    B_DVR_MediaFileSettings settings;
    B_MutexHandle mediaFileMutex;
    B_DVR_MediaFilePlayOpenSettings openSettings;
    NEXUS_FilePlayHandle nexusFilePlay;
    NEXUS_FileRecordHandle nexusFileRecord;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPlaybackPidChannels;
    NEXUS_PidChannelHandle videoPlaybackPidChannels;
    NEXUS_PlaybackHandle playback;
    B_DVR_PlaybackServiceHandle playbackService;
    B_DVR_DRMServiceHandle drmService;
    off_t currentMediaOffset;
    unsigned long currentMediaTime;  
    unsigned long currentseqHdrOffset;
    off_t currentNavOffset; 
    unsigned long currentIndex;
    unsigned long previousIFramePTS;
	unsigned long currentIFramePTS;
    unsigned long FrameRateptsdelta;
    int ptsDelta;
    B_DVR_MediaFileStreamFrameResource frameResource;
    B_DVR_MediaFileStreamFrameRate StreamFrameRate;	
    B_DVR_MediaFileChunkInfo chunkInfo;
    B_DVR_DRMServiceStreamBufferInfo decBufferInfo;
    B_DVR_TSBServiceHandle tsbService;
    int epollFd;
    int inotifyFd;
    int inotifyWatchFd;
#if PTS_PACING
    unsigned speedNumerator;
    unsigned long frameCount;
    unsigned firstTimeFlagForIFrame;
    unsigned long ptsOfFirstIFrame;
    unsigned long startingPtsOfIFrame;
    unsigned long ptsGap;	 
    unsigned long ptsDlt;	 
    unsigned DTSEnabledFlag;
    unsigned long dtsOfFirstIFrame;
    unsigned long startingDtsOfIFrame;    
    off_t    pcrBaseOfFirstIFrame;
    off_t    pcrBaseOfSecondIFrame;
    unsigned long pcrExtOfFirstIFrame;
    unsigned long pcrExtOfSecondIFrame;
    off_t pcrGap;    
    off_t pcrDlt;  
    unsigned pcrGapCtlFlag;
    off_t    pcrTemp;
    unsigned pcrIntlCtlFlag;
#endif
    unsigned long moreData;   
}B_DVR_MediaFile;

int B_DVR_MediaFile_P_Poll(B_DVR_MediaFileHandle mediaFile, unsigned timeout)
{
    int eventCount;
    struct epoll_event epollEvent;
    struct inotify_event inotifyEvent;
    int rc=-1;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    eventCount = epoll_wait(mediaFile->epollFd,&epollEvent,1,timeout);
    if((eventCount >=1) && (epollEvent.events & EPOLLIN))
    {
        ssize_t numRead;
        BDBG_MSG(("eventCount %d event %d",eventCount,epollEvent.events));
        numRead = read(mediaFile->inotifyFd,&inotifyEvent,sizeof(inotifyEvent));
        if(numRead==-1)
        {
            BDBG_WRN(("unable to read inotify file data"));
            rc = -1;
        }
        else
        {
            if(inotifyEvent.mask & IN_MODIFY)
            {
                BDBG_MSG((" mediaInfo Fd %d has been updated",inotifyEvent.wd));
                rc=0;
            }
        }
    }
    else
    {
        rc=-1;
    }
    return rc;
}

void B_DVR_MediaFile_P_InProgressRecordingUpdate(B_DVR_MediaFileHandle mediaFile)
{
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    B_DVR_SegmentedFilePlaySettings segmentedFilePlaySettings;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_OBJECT_ASSERT(dvrManager,B_DVR_Manager);
    BDBG_MSG(("B_DVR_MediaFile_P_InProgressRecordingUpdate >>>"));
    if(!mediaFile->nexusFileRecord && 
       (mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS))
    { 
        B_DVR_SegmentedFilePlay_GetSettings(mediaFile->nexusFilePlay,&segmentedFilePlaySettings);
        if(mediaFile->mediaNode->mediaLinearEndOffset > segmentedFilePlaySettings.mediaLinearEndOffset)
        {
            BDBG_MSG(("update mediaLinearEndOffset %jd:%jd",mediaFile->mediaNode->mediaLinearEndOffset,
                      segmentedFilePlaySettings.mediaLinearEndOffset));
            B_DVR_SegmentedFilePlay_UpdateFileList(mediaFile->nexusFilePlay);
            segmentedFilePlaySettings.mediaLinearEndOffset = mediaFile->mediaNode->mediaLinearEndOffset;
            segmentedFilePlaySettings.navLinearEndOffset = mediaFile->mediaNode->navLinearEndOffset;
            B_DVR_SegmentedFilePlay_SetSettings(mediaFile->nexusFilePlay,&segmentedFilePlaySettings);
        } 
        else 
        {
            BDBG_MSG(("no update mediaLinearEndOffset %jd:%jd",mediaFile->mediaNode->mediaLinearEndOffset,
                      segmentedFilePlaySettings.mediaLinearEndOffset));
        }
    }
    BDBG_MSG(("B_DVR_MediaFile_P_InProgressRecordingUpdate <<<"));
    return;
}

#if PTS_PACING
void B_DVR_MediaFile_P_ResetVar(
    B_DVR_MediaFileHandle mediaFile)
{
    B_DVR_MediaFileSettings *pSettings = &mediaFile->settings;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_P_ResetVar >>>>>"));

    if ((pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIFrame) ||
        (pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIPFrame))
    {
        mediaFile->firstTimeFlagForIFrame = 0;
        mediaFile->ptsOfFirstIFrame = 0;
        mediaFile->startingPtsOfIFrame= 0;
        mediaFile->frameCount = 0;
        mediaFile->ptsDlt = 0;
        mediaFile->DTSEnabledFlag = 0;
        mediaFile->dtsOfFirstIFrame = 0;	
        mediaFile->startingDtsOfIFrame = 0;	
        mediaFile->pcrBaseOfFirstIFrame = 0;
        mediaFile->pcrBaseOfSecondIFrame = 0;
        mediaFile->pcrExtOfFirstIFrame = 0;
        mediaFile->pcrExtOfSecondIFrame = 0;
        mediaFile->pcrDlt = 0;
        mediaFile->pcrGap = 0;
        mediaFile->pcrTemp = 0;
        mediaFile->pcrGapCtlFlag = 0;
        mediaFile->pcrIntlCtlFlag = 0;
        BDBG_MSG(("B_DVR_MediaFile_P_ResetVar:: Original -- mediaFile->ptsGap =%ld speedNumerator= %ld",mediaFile->ptsGap,mediaFile->speedNumerator));
        if(mediaFile->speedNumerator > 1)
        {
           mediaFile->ptsGap = (mediaFile->ptsGap)/mediaFile->speedNumerator;
        }
    }
    BDBG_MSG(("B_DVR_MediaFile_P_ResetVar<<<<< mediaFile->ptsGap =%ld ",mediaFile->ptsGap));
}

void B_DVR_MediaFile_P_GetPTSDTS(
    unsigned char *buf,
    unsigned long *orgpts,
    unsigned long *orgdts)
{
    off_t pts;
    off_t dts;

    BDBG_MSG(("B_DVR_MediaFile_P_GetPTSDTS<<<<< "));
    
    if((buf[0]&0xF0) == 0x20)
    {
        /* read 33 bits of pts info */
        pts = (buf[0] & 0x0E) >> 1; /* 3 bits */
        pts <<= 8;
        pts |= buf[1]; /* 8 bits */
        pts <<= 7;
        pts |= (buf[2] >> 1); /* 7 bits */
        pts <<= 8;
        pts |= buf[3]; /* 8 bits */
        pts <<= 7;
        pts |= (buf[4] >> 1); /* 7 bits */
        /* downshift by one. we throw away the LSB. */
        BDBG_MSG(("B_DVR_MediaFile_P_GetPTSDTS>>>only-get-PTS::OrgPTS=%lld",pts));
        pts >>= 1;
        *orgpts = (unsigned long)pts;
        *orgdts = 0;
        BDBG_MSG(("B_DVR_MediaFile_P_GetPTSDTS>>>only-get-PTS::ShiftPTS=%lld returnpts=%ld returndts=%ld",pts,*orgpts,*orgdts));
    }
    else
    {
        if((buf[0]&0xF0) == 0x30)
        {
            /* read 33 bits of pts info */
            pts = (buf[0] & 0x0E) >> 1; /* 3 bits */
            pts <<= 8;
            pts |= buf[1]; /* 8 bits */
            pts <<= 7;
            pts |= (buf[2] >> 1); /* 7 bits */
            pts <<= 8;
            pts |= buf[3]; /* 8 bits */
            pts <<= 7;
            pts |= (buf[4] >> 1); /* 7 bits */

            /* read 33 bits of dts info */
            dts = (buf[5] & 0x0E) >> 1; /* 3 bits */
            dts <<= 8;
            dts |= buf[6]; /* 8 bits */
            dts <<= 7;
            dts |= (buf[7] >> 1); /* 7 bits */
            dts <<= 8;
            dts |= buf[8]; /* 8 bits */
            dts <<= 7;
            dts |= (buf[9] >> 1); /* 7 bits */

            /* downshift by one. we throw away the LSB. */
            BDBG_MSG(("B_DVR_MediaFile_P_GetPTSDTS>>>get-both PTS/DTS::OrgPTS=%lld OrgDTS=%lld",pts,dts));
            pts >>= 1;
            dts >>= 1;
            *orgpts = (unsigned long)pts;
            *orgdts = (unsigned long)dts;
            BDBG_MSG(("B_DVR_MediaFile_P_GetPTSDTS>>>get-both PTS/DTS::ShiftPTS=%lld returnpts=%ld returndts=%ld",pts,*orgpts,*orgdts));
        }
        else
        {
           *orgpts = 0;
           *orgdts = 0;
           BDBG_MSG(("B_DVR_MediaFile_P_GetPTSDTS>>>Nothing to do:: returnpts=%ld returndts=%ld",*orgpts,*orgdts));
        }
    }
    return;
}

void B_DVR_MediaFile_P_SetPTSDTS(
    unsigned char *buf, 
    unsigned char pts_dts_flags, 
    unsigned long pts,
    unsigned long dts)
{
    off_t full_pts = pts << 1; /* upshift by one so that bitshift math matches spec */
    off_t full_dts = dts << 1;
    BDBG_MSG(("B_DVR_MediaFile_SetPTSDTS<<<<< pts =%ld full_pts=%lld dts=%ld full_dts=%lld",pts,full_pts,dts,full_dts));

    if((pts_dts_flags == 0x02)&&
       ((buf[0]&0xF0) == 0x20))
    {
        BDBG_MSG(("B_DVR_MediaFile_SetPTSDTS:: Only set PTS value"));
        buf[0] = (((full_pts >> 30) & 0x7)  << 1) | 0x1 | (pts_dts_flags << 4);
        buf[1] = (((full_pts >> 22) & 0xff));
        buf[2] = (((full_pts >> 15) & 0x7f) << 1) | 0x1;
        buf[3] = (((full_pts >>  7) & 0xff));
        buf[4] = (((full_pts      ) & 0x7f) << 1) | 0x1;
    }
    else
    {
        if((pts_dts_flags == 0x03)&&
           ((buf[0]&0xF0) == 0x30))
        {
            BDBG_MSG(("B_DVR_MediaFile_SetPTSDTS:: set both PTS and DTS values"));
            buf[0] = (((full_pts >> 30) & 0x7)  << 1) | 0x1 | (pts_dts_flags << 4);
            buf[1] = (((full_pts >> 22) & 0xff));
            buf[2] = (((full_pts >> 15) & 0x7f) << 1) | 0x1;
            buf[3] = (((full_pts >>  7) & 0xff));
            buf[4] = (((full_pts      ) & 0x7f) << 1) | 0x1;
            buf[5] = (((full_dts >> 30) & 0x7)  << 1) | 0x1 | 0x10;
            buf[6] = (((full_dts >> 22) & 0xff));
            buf[7] = (((full_dts >> 15) & 0x7f) << 1) | 0x1;
            buf[8] = (((full_dts >>  7) & 0xff));
            buf[9] = (((full_dts      ) & 0x7f) << 1) | 0x1;
        }
    }

    BDBG_MSG(("B_DVR_MediaFile_SetPTSDTS >>>>>"));
}

bool B_DVR_MediaFile_P_IsPESStreamID(
    unsigned char stream_id)
{
    return
        ((stream_id & 0xFC) == 0xBC) || /* 1011 11xx */
        ((stream_id & 0xC0) == 0xC0) || /* 110x xxxx */
        ((stream_id & 0xF0) == 0xE0) || /* 1110 xxxx */
        ((stream_id & 0xF0) == 0xF0);   /* 1111 xxxx */
}

uint8_t B_DVR_MediaFile_P_CheckStartCode(
     unsigned char data, 
     uint8_t sccount)
{
    switch (data)
    {
      case 0:
        if (sccount >= 1)
            sccount = 2;
        else
            sccount = 1;
        break;
      case 1:
        if (sccount == 2)
        {
            /* we've got a start code! */
            sccount = 3;
        }
        else 
            sccount = 0;
        break;
      default:
        sccount = 0;
        break;
    }
    return sccount;
}

B_DVR_ERROR B_DVR_MediaFile_Lookup_PES(
    B_DVR_MediaFileHandle mediaFile,
    uint8_t *buffer,  
    uint8_t size)
{
    uint8_t i,pts_dts_flags;
    uint8_t sccount = 0;
    unsigned long pts = 0;
    unsigned long orgpts=0;
    unsigned long dts = 0;
    unsigned long orgdts=0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;   
    B_DVR_FileReadDirection direction;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_Lookup_PES >>>>>"));

    pts = mediaFile->ptsOfFirstIFrame + mediaFile->ptsDlt;
    BDBG_MSG(("B_DVR_MediaFile_Lookup_PES:: Direction is forward pts=%ld ptsOfFirstIFrame=%ld ptsDlt=%ld",pts,mediaFile->ptsOfFirstIFrame,mediaFile->ptsDlt));
    if(mediaFile->DTSEnabledFlag == 1)
    {
        dts = mediaFile->dtsOfFirstIFrame + mediaFile->ptsDlt;
        BDBG_MSG(("B_DVR_MediaFile_Lookup_PES:: Direction is forward dts=%ld dtsOfFirstIFrame=%ld ptsDlt=%ld",dts,mediaFile->dtsOfFirstIFrame,mediaFile->ptsDlt));
    }

    direction = (B_DVR_FileReadDirection)(mediaFile->settings.fileOperation.streamingOperation.direction);

    for(i=0; i<size; i++) 
    {
       if(sccount == 3) 
       {
          if(B_DVR_MediaFile_P_IsPESStreamID(buffer[i])) 
          {
              BDBG_MSG(("B_DVR_MediaFile_Lookup_PES:: Found out the PES StreamID %d",i));
              if(buffer[i+4]&0xc0)
              {
                 BDBG_MSG(("B_DVR_MediaFile_Lookup_PES:: Prepare to set PTS in function B_DVR_MediaFile_SetPTSDTS"));
                 pts_dts_flags = ((buffer[i+4]&0xc0)>>6)&0x03;
				 
                 if (mediaFile->speedNumerator > 1)
                 {
                     B_DVR_MediaFile_P_GetPTSDTS(&buffer[i+6],&orgpts,&orgdts);                
                     if (0 == mediaFile->startingPtsOfIFrame)
                     {
                        mediaFile->startingPtsOfIFrame = orgpts;
                     }
                     if (0 == mediaFile->startingDtsOfIFrame)
                     {
                        mediaFile->startingDtsOfIFrame = orgdts;
                     }
                     BDBG_WRN(("B_DVR_MediaFile_Lookup_PES:: Get Set-PTS/DTS in B_DVR_MediaFile_SetPTSDTS returnpts=%ld returndts=%ld",orgpts,orgdts));
                     if(direction == eB_DVR_FileReadDirectionForward)
                     {
                         pts = mediaFile->startingPtsOfIFrame + (orgpts - mediaFile->startingPtsOfIFrame) / mediaFile->speedNumerator;
                         dts = mediaFile->startingDtsOfIFrame + (orgdts - mediaFile->startingDtsOfIFrame) / mediaFile->speedNumerator;
                     }
                     else
                     {
                         pts = mediaFile->startingPtsOfIFrame + (mediaFile->startingPtsOfIFrame - orgpts) / mediaFile->speedNumerator;
                         dts = mediaFile->startingDtsOfIFrame + (mediaFile->startingDtsOfIFrame - orgdts) / mediaFile->speedNumerator;
                     }
                     
                     B_DVR_MediaFile_P_SetPTSDTS(&buffer[i+6],pts_dts_flags,pts,dts);
                     B_DVR_MediaFile_P_GetPTSDTS(&buffer[i+6],&orgpts,&orgdts);
                     BDBG_WRN(("B_DVR_MediaFile_Lookup_PES1:: Get Set-PTS/DTS in B_DVR_MediaFile_SetPTSDTS returnpts=%ld returndts=%ld",orgpts,orgdts));
                 }
                 break;
              }
          }
          sccount = 0;
       }
       sccount = B_DVR_MediaFile_P_CheckStartCode(buffer[i], sccount);
    }

    BDBG_MSG(("B_DVR_MediaFile_Lookup_PES <<<<"));
    return rc;
}

void B_DVR_MediaFile_Lookup_PCR(
    B_DVR_MediaFileHandle mediaFile,
    uint8_t *buffer)
{
    B_DVR_MediaFileSettings *pSettings = &mediaFile->settings;
    B_DVR_FileReadDirection direction;
    off_t pcr = 0;
    off_t pcrBase = 0;
    off_t pcrInDlt = 0;
    unsigned long pcrExt = 0;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR >>>>>"));

    direction = (B_DVR_FileReadDirection)(pSettings->fileOperation.streamingOperation.direction);
    pcrBase =(off_t)((buffer[0]<<25) | (buffer[1]<<17) | (buffer[2]<<9) | (buffer[3]<<1) | ((buffer[4] & 0x80)>>7));
    pcrExt = (unsigned long)(((buffer[4] & 0x01)<<8) | buffer[5]);

    if(mediaFile->pcrIntlCtlFlag == 0)
    {
       mediaFile->pcrTemp = (pcrBase<<9) + pcrExt;
       pcr = ((mediaFile->pcrBaseOfFirstIFrame<<9) + (off_t)(mediaFile->pcrExtOfFirstIFrame&0x1FF) + mediaFile->pcrDlt)&0x3FFFFFFFFFF;
       BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR:: Direction is forward,mediaFile->pcrIntlCtlFlag == 0, pcrBase=%lld,pcrExt=%ld ",pcrBase,pcrExt));
       BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR:: Direction is forward,mediaFile->pcrIntlCtlFlag == 0, pcr=%lld,mediaFile->pcrDlt=%lld ",pcr,mediaFile->pcrDlt));
       mediaFile->pcrIntlCtlFlag = 1;
    }
    else
    {
       pcr = (pcrBase<<9) + pcrExt;
       if(direction == eB_DVR_FileReadDirectionForward)
       {
          pcrInDlt = pcr - mediaFile->pcrTemp;
       }
       else
       {
          pcrInDlt = mediaFile->pcrTemp - pcr;
       }

	   if(mediaFile->speedNumerator > 1)
		 pcrInDlt = pcrInDlt/mediaFile->speedNumerator;
	   pcr = (mediaFile->pcrBaseOfFirstIFrame<<9) + mediaFile->pcrExtOfFirstIFrame + mediaFile->pcrDlt + pcrInDlt;
	   BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR:: Direction is forward,mediaFile->pcrIntlCtlFlag == 1, pcr=%lld,mediaFile->pcrDlt=%lld,pcrInDlt=%lld ",pcr,
				  mediaFile->pcrDlt,pcrInDlt));
    }

    pcrBase = (pcr>>9)&0x1FFFFFFFF;
    pcrExt  = (unsigned long)(pcr&0x1FF);
    BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR:: pcr=%lld ",pcr));
    BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR:: pcrBase=%lld pcrExt=%ld mediaFile->pcrDlt=%lld",pcrBase,pcrExt,mediaFile->pcrDlt));

    buffer[0] = (uint8_t)((pcrBase>>25) & 0xff);
    buffer[1] = (uint8_t)((pcrBase>>17) & 0xff);
    buffer[2] = (uint8_t)((pcrBase>>9)  & 0xff);
    buffer[3] = (uint8_t)((pcrBase>>1)  & 0xff);
    buffer[4] = (uint8_t)(((pcrBase<<7) & 0x80) | 0x7E |((pcrExt>>8)&0x01));
    buffer[5] = (uint8_t)(pcrExt & 0xff);
    BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR PCR[0]=0x%x PCR[1]=0x%x PCR[2]=0x%x <<<<",buffer[0],buffer[1],buffer[2]));
    BDBG_MSG(("B_DVR_MediaFile_Lookup_PCR PCR[3]=0x%x PCR[4]=0x%x PCR[5]=0x%x <<<<",buffer[3],buffer[4],buffer[5]));
}

B_DVR_ERROR B_DVR_MediaFile_P_Pts_Chg(
    B_DVR_MediaFileHandle mediaFile,
    uint8_t *buffer,
    ssize_t readSize)
{
    uint8_t *pBuf = buffer;
    ssize_t frameSize = readSize;
    uint8_t pktsize = 188;
    uint8_t startIndicator,adaptionCtl;
    unsigned long count,i;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_Pts_Chg >>>>> framesize=%d",frameSize));
    
    for(count=0,i=0;frameSize >0;count++)
    {        
        i = pktsize*count;
        if(pBuf[i]!=0x47)
        {
           BDBG_MSG(("B_DVR_MediaFile_Pts_Chg:: framesize=%d count=%d i=%d",frameSize,count,i));
           rc=B_DVR_UNKNOWN;
           break;
        }
        startIndicator = pBuf[i+1]&0x40;
        adaptionCtl = pBuf[i+3]&0x30;
        if((startIndicator== 0x40)||((adaptionCtl == 0x30)||(adaptionCtl == 0x20)))
        {
            BDBG_MSG(("B_DVR_MediaFile_Pts_Chg:: LoopCount=%d Adaption ControlField=%d startIndicator=%d",count,adaptionCtl,startIndicator));
            if((adaptionCtl == 0x30)||(adaptionCtl == 0x20))
            { 
               BDBG_MSG(("B_DVR_MediaFile_Pts_Chg:: Adaption ControlField for PCR and PES"));
               /*look up PCR in adaption code at first step*/
               if((pBuf[i+5]&0x10)&&(mediaFile->pcrGapCtlFlag == 2))
                  B_DVR_MediaFile_Lookup_PCR(mediaFile,&pBuf[i+6]);
               /*Continue to look up the PTS in PES packet in this frame at second step*/
               rc = B_DVR_MediaFile_Lookup_PES(mediaFile,&(pBuf[i+5+pBuf[i+4]]),pktsize-5-pBuf[i+4]);
            }
            else
            {
               BDBG_MSG(("B_DVR_MediaFile_Pts_Chg:: startIndicator for PES"));
               rc = B_DVR_MediaFile_Lookup_PES(mediaFile,&pBuf[i+4],pktsize-4);
            }
        }
        frameSize -= pktsize;
    }
    mediaFile->pcrIntlCtlFlag = 0;
    mediaFile->pcrTemp = 0;
    BDBG_MSG(("B_DVR_MediaFile_Pts_Chg <<<< count=%d",count));
    return rc;
}

void B_DVR_MediaFile_P_GetDTS(
    B_DVR_MediaFileHandle mediaFile,
    unsigned char *buf,
    unsigned char pts_dts_flags)
{
    off_t dts;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_P_GetDTS<<<<< "));

    if(pts_dts_flags != 0x03)
    {
       BDBG_ERR(("B_DVR_MediaFile_P_GetDTS::Something wrong, PTS/DTS flag setting is wrong!!! "));
       return;
    }  
    /* read 33 bits of dts info */
    dts = (buf[5] & 0x0E) >> 1; /* 3 bits */
    dts <<= 8;
    dts |= buf[6]; /* 8 bits */
    dts <<= 7;
    dts |= (buf[7] >> 1); /* 7 bits */
    dts <<= 8;
    dts |= buf[8]; /* 8 bits */
    dts <<= 7;
    dts |= (buf[9] >> 1); /* 7 bits */

    /* downshift by one. we throw away the LSB. */
    BDBG_MSG(("B_DVR_MediaFile_P_GetDTS>>>OrgPTS=%lld",dts));
    dts >>= 1;
    BDBG_MSG(("B_DVR_MediaFile_P_GetDTS>>>mediaFile->dtsOfFirstIFrame=%lld",dts));
    mediaFile->dtsOfFirstIFrame = (unsigned long)dts;
    return;
}


B_DVR_ERROR B_DVR_MediaFile_Get_FirstDTSValue(
    B_DVR_MediaFileHandle mediaFile,
    uint8_t *buffer,  
    uint8_t size)
{
    uint8_t i,pts_dts_flags;
    uint8_t sccount = 0;
    B_DVR_ERROR rc=B_DVR_SUCCESS;   

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_Get_FirstDTSValue >>>>>"));

    for(i=0; i<size; i++) 
    {
       if(sccount == 3) 
       {
          if(B_DVR_MediaFile_P_IsPESStreamID(buffer[i])) 
          {
              BDBG_MSG(("B_DVR_MediaFile_Get_FirstDTSValue:: Found out the PES StreamID %d",i));
              if(buffer[i+4]&0x40)
              {
                 BDBG_MSG(("B_DVR_MediaFile_Get_FirstDTSValue::Get first DTS value For I-Frame"));
                 mediaFile->DTSEnabledFlag = 1;
                 pts_dts_flags = ((buffer[i+4]&0xc0)>>6)&0x03;
                 B_DVR_MediaFile_P_GetDTS(mediaFile,&buffer[i+6],pts_dts_flags);
				 break;
              }
          }
          sccount = 0;
       }
       sccount = B_DVR_MediaFile_P_CheckStartCode(buffer[i], sccount);
    }

    BDBG_MSG(("B_DVR_MediaFile_Get_FirstDTSValue <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_MediaFile_P_LookUp_DTS(
    B_DVR_MediaFileHandle mediaFile,
    uint8_t *buffer,
    ssize_t readSize)
{
    uint8_t *pBuf = buffer;
    ssize_t frameSize = readSize;
    uint8_t pktsize = 188;
    uint8_t startIndicator,adaptionCtl;
    unsigned long count,i;
    B_DVR_ERROR rc=B_DVR_SUCCESS;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_LookUp_DTS >>>>> framesize=%d",frameSize));
    
    for(count=0,i=0;frameSize >0;count++)
    {        
        i = pktsize*count;
        if(pBuf[i]!=0x47)
        {
           BDBG_MSG(("B_DVR_MediaFile_LookUp_DTS:: framesize=%d count=%d i=%d",frameSize,count,i));
           rc=B_DVR_UNKNOWN;
           break;
        }
        startIndicator = pBuf[i+1]&0x40;
        adaptionCtl = pBuf[i+3]&0x30;
        if((startIndicator== 0x40)||((adaptionCtl == 0x30)||(adaptionCtl == 0x20)))
        {
            BDBG_MSG(("B_DVR_MediaFile_LookUp_DTS:: LoopCount=%d Adaption ControlField=%d startIndicator=%d",count,adaptionCtl,startIndicator));
            if((adaptionCtl == 0x30)||(adaptionCtl == 0x20))
            { 
               BDBG_MSG(("B_DVR_MediaFile_LookUp_DTS:: Adaption ControlField for PCR and PES"));
               rc = B_DVR_MediaFile_Get_FirstDTSValue(mediaFile,&(pBuf[i+5+pBuf[i+4]]),pktsize-5-pBuf[i+4]);
            }
            else
            {
               BDBG_MSG(("B_DVR_MediaFile_LookUp_DTS:: startIndicator for PES"));
               rc = B_DVR_MediaFile_Get_FirstDTSValue(mediaFile,&pBuf[i+4],pktsize-4);
            }
        }
        frameSize -= pktsize;
    }

    BDBG_MSG(("B_DVR_MediaFile_LookUp_DTS <<<< count=%d",count));
    return rc;
}

void B_DVR_MediaFile_Lookup_GetPCRBaseExt(
    B_DVR_MediaFileHandle mediaFile,
    uint8_t *buffer)
{
    B_DVR_MediaFileSettings *pSettings = &mediaFile->settings;
    B_DVR_FileReadDirection direction;
    off_t pcrBase=0;
    off_t temppcr=0;
    off_t pcr=0;
    unsigned long pcrExt=0;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_Lookup_GetPCRBaseExt >>>>>"));

    if(mediaFile->pcrGapCtlFlag == 0)
    {
        pcrBase =(off_t)((buffer[0]<<25) | (buffer[1]<<17) | (buffer[2]<<9) | (buffer[3]<<1) | ((buffer[4] & 0x80)>>7));
        pcrExt = (unsigned long)(((buffer[4] & 0x01)<<8) | buffer[5]);
        mediaFile->pcrBaseOfFirstIFrame = pcrBase;
        mediaFile->pcrExtOfFirstIFrame = pcrExt;
        BDBG_MSG(("B_DVR_MediaFile_Lookup_GetPCRBaseExt pcrBase[0]=0x%x pcrBase[1]=0x%x pcrBase[2]=0x%x pcrBase[3]=0x%x pcrBase[4]&0x80=0x%x<<<<",buffer[0],
                   buffer[1],buffer[2],buffer[3],buffer[4]&0x80));
        BDBG_MSG(("B_DVR_MediaFile_Lookup_GetPCRBaseExt pcrExt[0]=0x%x pcrExt[1]=0x%x <<<<",(buffer[4]&0x01)<<8,buffer[5]));
        BDBG_MSG(("B_DVR_MediaFile_Lookup_GetPCRBaseExt pcrBaseOfFirstIFrame=%lld pcrExtOfFirstIFrame=%ld <<<<",mediaFile->pcrBaseOfFirstIFrame,
                   mediaFile->pcrExtOfFirstIFrame));
    }
    else
    {
        if(mediaFile->pcrGapCtlFlag == 1)
        {
           pcrBase =(off_t)((buffer[0]<<25) | (buffer[1]<<17) | (buffer[2]<<9) | (buffer[3]<<1) | ((buffer[4] & 0x80)>>7));
           pcrExt = (unsigned long)(((buffer[4] & 0x01)<<8) | buffer[5]);
           mediaFile->pcrBaseOfSecondIFrame = pcrBase;
           mediaFile->pcrExtOfSecondIFrame = pcrExt;
           mediaFile->pcrGapCtlFlag = 2;
           pcr = (pcrBase<<9) + pcrExt;
           temppcr = (mediaFile->pcrBaseOfFirstIFrame << 9) + mediaFile->pcrExtOfFirstIFrame;
           direction = (B_DVR_FileReadDirection)(pSettings->fileOperation.streamingOperation.direction);
           if(direction == eB_DVR_FileReadDirectionForward)
           {
               mediaFile->pcrGap = pcr -temppcr;
           }
           else
           {
               mediaFile->pcrGap = temppcr - pcr;
           }
           BDBG_MSG(("B_DVR_MediaFile_Lookup_GetPCRBaseExt pcrBaseOfSecondIFrame=%lld pcrExtOfSecondIFrame=%ld <<<<",mediaFile->pcrBaseOfSecondIFrame,
                      mediaFile->pcrExtOfSecondIFrame));
           BDBG_MSG(("B_DVR_MediaFile_Lookup_GetPCRBaseExt  mediaFile->pcrGap=%lld<<<<",mediaFile->pcrGap));
        }
    }
}

B_DVR_ERROR B_DVR_MediaFile_P_LookUp_PCRBase(
    B_DVR_MediaFileHandle mediaFile,
    uint8_t *buffer,
    ssize_t readSize)
{
    uint8_t *pBuf = buffer;
    ssize_t frameSize = readSize;
    uint8_t pktsize = 188;
    uint8_t startIndicator,adaptionCtl;
    unsigned long count,i;
    B_DVR_ERROR rc=B_DVR_SUCCESS;
	
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_LookUp_PCRBase >>>>> framesize=%d",frameSize));
    
    for(count=0,i=0;frameSize >0;count++)
    {        
        i = pktsize*count;
        if(pBuf[i]!=0x47)
        {
           BDBG_MSG(("B_DVR_MediaFile_LookUp_PCRBase:: framesize=%d count=%d i=%d",frameSize,count,i));
           rc=B_DVR_UNKNOWN;
           break;
        }
        startIndicator = pBuf[i+1]&0x40;
        adaptionCtl = pBuf[i+3]&0x30;
        if((startIndicator== 0x40)||((adaptionCtl == 0x30)||(adaptionCtl == 0x20)))
        {
            BDBG_MSG(("B_DVR_MediaFile_LookUp_PCRBase:: LoopCount=%d Adaption ControlField=%d startIndicator=%d",count,adaptionCtl,startIndicator));
            if((adaptionCtl == 0x30)||(adaptionCtl == 0x20))
            { 
               BDBG_MSG(("B_DVR_MediaFile_LookUp_PCRBase:: Adaption ControlField for PCR and PES"));
               if(pBuf[i+5]&0x10)
                   B_DVR_MediaFile_Lookup_GetPCRBaseExt(mediaFile,&pBuf[i+6]);
            }
        }
        frameSize -= pktsize;
    }

    BDBG_MSG(("B_DVR_MediaFile_LookUp_PCRBase <<<< count=%d",count));
    return rc;
}
#endif

#if PTS_PACING
B_DVR_ERROR B_DVR_MediaFile_UserSettings(
    B_DVR_MediaFileHandle mediaFile,
    unsigned numerator)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_UserSet numerator=%d>>>>>",numerator));
    mediaFile->speedNumerator = numerator;
    mediaFile->previousIFramePTS = 0;
    BDBG_MSG(("B_DVR_MediaFile_UserSet <<<<"));
    return rc;
}
#else
B_DVR_ERROR B_DVR_MediaFile_UserSettings(
    B_DVR_MediaFileHandle mediaFile,
    unsigned numerator)
{
    B_DVR_ERROR rc=B_DVR_INVALID_PARAMETER;
    BSTD_UNUSED(mediaFile);
    BSTD_UNUSED(numerator);
    mediaFile->previousIFramePTS = 0;
	BDBG_MSG(("B_DVR_MediaFile_UserSet: PTS_PACING not enabled at compile time"));
    return rc;
}
#endif

int B_DVR_MediaFile_GetNumOfFrames(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileFrameType frameType,
    off_t startOffset,
    off_t endOffset)
{
    NEXUS_FilePlayHandle nexusFilePlay = mediaFile->nexusFilePlay;
    int numOfFrames = 0;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(nexusFilePlay);
    BDBG_MSG(("B_DVR_MediaFile_GetNumOfFrames >>>>>"));
    if(mediaFile->nexusFileRecord)
    {
        numOfFrames = B_DVR_SegmentedFileRecord_GetNumOfFrames(mediaFile->nexusFileRecord,frameType,startOffset,endOffset);
    }
    else
    {
        numOfFrames = B_DVR_SegmentedFilePlay_GetNumOfFrames(nexusFilePlay,frameType,startOffset,endOffset);
    }
    BDBG_MSG(("B_DVR_MediaFile_GetNumOfFrames <<<<"));
    return numOfFrames;
}

B_DVR_ERROR B_DVR_MediaFile_TrickModeIPFrameSkip(
    B_DVR_MediaFileHandle mediaFile,
    unsigned skipStep,
    unsigned dumpStep,   
    unsigned *expectedDeltaPts)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_MediaFileSettings *pSettings = &mediaFile->settings;
    B_DVR_FilePosition currentPosition;
	B_DVR_FilePosition iFramePosition;
    B_DVR_FileReadDirection direction;
    int skipPTSValue = 0;
    int accumulatePTS = 0;
    unsigned sizeOfFrame = 0;   
    unsigned count;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_TrickModeIPFrameSpeedRate Skipnum = %d >>>>",skipStep));	
    B_Mutex_Lock(mediaFile->mediaFileMutex);	
    BKNI_Memset((void *)&currentPosition,0,sizeof(B_DVR_FilePosition));
    if((pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIFrame) || 
       (pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIPFrame))
    {
			BDBG_MSG(("--- Currentindex=%d, mediaFile->FrameRateptsdelta = %d", mediaFile->currentIndex, mediaFile->FrameRateptsdelta));
			if (dumpStep == 0)
			{
			   skipPTSValue = *expectedDeltaPts - mediaFile->ptsDelta;
			   BDBG_MSG(("In-Skip-mode-- skipPTSValue:%d, Currenrindex=%d", skipPTSValue, mediaFile->currentIndex));
			   for(count = 0; count < (skipStep + MAX_SKIP_RETRY_COUNT); count++)
			   {
				   currentPosition.index = mediaFile->currentIndex;
				   direction = pSettings->fileOperation.streamingOperation.direction;
                   BDBG_MSG(("Mediafile segmentedMedia--currentofftset=%llx, testcurrentIndex:%lu",currentPosition.mpegFileOffset, currentPosition.index));
				   if(mediaFile->nexusFileRecord)
				   {
				       rc = B_DVR_SegmentedFileRecord_GetNextIFrame(mediaFile->nexusFileRecord,currentPosition,&iFramePosition,&sizeOfFrame,direction);
				   }
				   else
				   {
				       rc = B_DVR_SegmentedFilePlay_GetNextIFrame(mediaFile->nexusFilePlay,currentPosition,&iFramePosition,&sizeOfFrame,direction);
				   }

				   BDBG_MSG(("output testcurrentIndex:%lu, sizeOfFrame:%d", iFramePosition.index, sizeOfFrame));

                   if (rc != B_DVR_SUCCESS)
				   {
					  BDBG_WRN(("Error to GetNextIFrame! count:%d, skipStep:%d", count, skipStep));
					  break;
				   }
				   else
				   {
					  if (((eB_DVR_FileReadDirectionReverse == direction) && (iFramePosition.index > currentPosition.index))
					   || ((eB_DVR_FileReadDirectionForward == direction) && (iFramePosition.index < currentPosition.index)))
					  {
						 BDBG_WRN(("trick fail to GetNextIFrame(out of bound)! direction:%d, index:%d", direction, iFramePosition.index));
						 iFramePosition.index = currentPosition.index;
					  }
				   }
			   
				   if((iFramePosition.mpegFileOffset+sizeOfFrame) < mediaFile->mediaNode->mediaLinearEndOffset)
				   {
					   mediaFile->currentMediaOffset = iFramePosition.mpegFileOffset;
					   mediaFile->currentNavOffset = iFramePosition.navFileOffset;
					   mediaFile->currentMediaTime = iFramePosition.timestamp;
					   mediaFile->currentseqHdrOffset = iFramePosition.seqHdrOffset;
					   if(direction == eB_DVR_FileReadDirectionForward)
					   {
						  accumulatePTS = (iFramePosition.pts > mediaFile->currentIFramePTS)?(iFramePosition.pts - mediaFile->currentIFramePTS):0;
					   }
					   else
					   {
						  accumulatePTS = (mediaFile->currentIFramePTS > iFramePosition.pts)?(mediaFile->currentIFramePTS - iFramePosition.pts):0;
					   }
					   BDBG_MSG(("TrickModeIPFrameSkip:: currentIndex:%u, iFramePosition.pts=%u mediaFile->currentIFramePTS=%u, accumulatePTS:%d",
						  iFramePosition.index,iFramePosition.pts,mediaFile->currentIFramePTS, accumulatePTS));
				   }
				   else
				   {
					   BDBG_MSG(("Next I-Frame offset beyond end of file"));
					   break;
				   }  
			   
				   if(mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM)
				   {
					   if(accumulatePTS >= skipPTSValue)
					   {
						   if ((unsigned int)(accumulatePTS-skipPTSValue)>(*expectedDeltaPts))
						   {
							  if ((unsigned int)accumulatePTS < ((*expectedDeltaPts)*10))
							  {
								 /* in case accumulatePTS much larger than expected, should be an bad stream, ignore it.*/
								 /* only duplicate for normal stream fine tune. */
								 if(direction == eB_DVR_FileReadDirectionForward)
								 {
									if(mediaFile->currentIndex)
										mediaFile->currentIndex--;
								 }
								 else
								 {
									 mediaFile->currentIndex = mediaFile->currentIndex + 1;
								 }
								 mediaFile->ptsDelta -= *expectedDeltaPts;
								 BDBG_MSG(("dup when ptsDelta(%d) large... currentindex=%d",
									mediaFile->ptsDelta, mediaFile->currentIndex));
							  }
							  else
							  {
							     mediaFile->ptsDelta = 0;
								 BDBG_WRN(("stream error with huge PTS gap, ignore it..."));
							  }
						   }
						   else
						   {
							  mediaFile->currentIndex = iFramePosition.index;
							  mediaFile->ptsDelta += (accumulatePTS - *expectedDeltaPts);
						   }
						   BDBG_MSG(("TrickModeIPFrameSkipF:: accumulatePTS=%d skipPTSValue=%d ptsDelta:%d, currentindex=%d",
							  accumulatePTS, skipPTSValue, mediaFile->ptsDelta, mediaFile->currentIndex));
						   break;
					   }
					   else
					   {
					   
						   #if PTS_PACING
 		                       mediaFile->frameCount++;
						   #endif					 
						   
						   if(direction == eB_DVR_FileReadDirectionForward)
							  mediaFile->currentIndex = iFramePosition.index+1;
						   else
						   {
							  if(iFramePosition.index == 0)
								  mediaFile->currentIndex = iFramePosition.index;
							  else
								  mediaFile->currentIndex = iFramePosition.index -1;
						   }
						   if (accumulatePTS == 0){
							  BDBG_WRN(("in case PTS is not increasing, just sent out the junction frame currentIndex:%d", mediaFile->currentIndex));
							  break;
						   }
					   }
				   }
				   BDBG_MSG(("count:%d, skipStep:%d(+10), accumulatePTS=%d skipPTSValue=%d currentindex=%d", 
					  count, skipStep, accumulatePTS, skipPTSValue, mediaFile->currentIndex));
			   }
			}
			else
			{
			   mediaFile->ptsDelta -= *expectedDeltaPts;
			   direction = pSettings->fileOperation.streamingOperation.direction;
			   if(mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM)
			   {
				   if(direction == eB_DVR_FileReadDirectionForward)
				   {
					  if(mediaFile->currentIndex)
						  mediaFile->currentIndex--;
				   }
				   else
				   {
					   mediaFile->currentIndex = mediaFile->currentIndex + 1;
				   }
			   }
			}
	}
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    BDBG_MSG(("B_DVR_MediaFile_TrickModeIPFrameSpeedRate <<<<"));
    return rc;
}    

#define MAX_SAMPLE_LENGTH 20
#define MIN_SAMPLE_LENGTH 3
#define NUM_OF_RANKS 5
#define PTS_UNITS (1000*45)

B_DVR_ERROR B_DVR_MediaFile_StreamIPFrameRate(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileStreamFrameRate *frameRate)
{
	B_DVR_ERROR rc=B_DVR_SUCCESS;
	B_DVR_MediaFileSettings *pSettings = &mediaFile->settings;
	B_DVR_FilePosition currentPosition,iFramePosition;
	B_DVR_FileReadDirection direction;
	uint8_t count, rank, len = MAX_SAMPLE_LENGTH;
	uint8_t countterlen = NUM_OF_RANKS;
	unsigned long pts[len],ptsdelta=0;
	unsigned long ptsgap[len-1],maxptsgap=0;
	unsigned long ptsgapcount[NUM_OF_RANKS];  /*0~20,20~40,40~60,60~80,80~100*/
	unsigned long ptsgapresult[NUM_OF_RANKS];
	unsigned long tempptsgapresult[NUM_OF_RANKS];
        off_t         frameOffset[len];
	unsigned long frameOffsetgap[len-1];
        unsigned long frameOffsetresult[NUM_OF_RANKS];
	unsigned long tempvalue=0;
	unsigned long searchIndex=0;
	uint8_t  finalcount=0;
	unsigned sizeOfFrame = 0;	
	unsigned defaultframeRate =4;
	off_t linearEndOffset = 0;
	
	BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
	BDBG_MSG(("B_DVR_MediaFile_StreamIPFrameRate >>>>"));
   	BKNI_Memset((void *)&currentPosition,0,sizeof(B_DVR_FilePosition));

	/*in case of streamRate calculation fail, it might be better to do normal trick mode using default rate, instead of return error.*/
	frameRate->sfRateNumerator = defaultframeRate;
	frameRate->sfRateDenominator = 0;
	
	for(count = 0; count < (NUM_OF_RANKS); count++)
	{
		ptsgapcount[count] = 0;
		ptsgapresult[count] = 0;
                tempptsgapresult[count] = 0;
                frameOffsetresult[count] = 0;
	}

	direction = pSettings->fileOperation.streamingOperation.direction;
	if((pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIFrame) || 
	   (pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIPFrame))
	{
	   mediaFile->ptsDelta = 0;
	   searchIndex = mediaFile->currentIndex;
	   for(count = 0; count < len; count++)
	   {
		  pts[count] = 0;
		  currentPosition.index = searchIndex;
      	  BDBG_MSG(("Mediafile segmentedMedia--currentofftset=%llx, index:%lu",currentPosition.mpegFileOffset, currentPosition.index));
		  if(mediaFile->nexusFileRecord)
		  {
		      rc = B_DVR_SegmentedFileRecord_GetNextIFrame(mediaFile->nexusFileRecord,currentPosition,&iFramePosition,&sizeOfFrame,direction);
		  }
		  else
		  {
		      rc = B_DVR_SegmentedFilePlay_GetNextIFrame(mediaFile->nexusFilePlay,currentPosition,&iFramePosition,&sizeOfFrame,direction);
	      }

          if (rc != B_DVR_SUCCESS)
		  {
			 BDBG_WRN(("Error to GetNextIFrame!!"));
			 continue;
		  }
	
		  linearEndOffset = mediaFile->mediaNode->mediaLinearEndOffset;

		  if((iFramePosition.mpegFileOffset+sizeOfFrame) < linearEndOffset)
		  {
			 pts[count] = iFramePosition.pts;
                         frameOffset[count] = iFramePosition.mpegFileOffset;

			 if(direction == eB_DVR_FileReadDirectionForward)
				 searchIndex = iFramePosition.index +1;
			 else
			 {
				 if (iFramePosition.index){
					searchIndex = iFramePosition.index -1;
				 }
				 else{
					if(count < MIN_SAMPLE_LENGTH)
					{
					   mediaFile->StreamFrameRate.sfRateNumerator = defaultframeRate;
					   mediaFile->StreamFrameRate.sfRateDenominator = 0;
					   goto StreamIPFrameRate_err;
					}
					else
					{
					   len = count;
					   goto StreamIPFrameRate_Patrial;
					}
				 }
			 }
		  }
		  else
		  {
			 BDBG_WRN(("Next I-Frame offset beyond end of file"));
			 if(count < MIN_SAMPLE_LENGTH)
			 {
				mediaFile->StreamFrameRate.sfRateNumerator = defaultframeRate;
				mediaFile->StreamFrameRate.sfRateDenominator = 0;
				goto StreamIPFrameRate_err;
			}else{
				len = count;
				goto StreamIPFrameRate_Patrial;
			}
		  }  
	  }
	
StreamIPFrameRate_Patrial:
	  BDBG_MSG(("len= %d countterlen=%d",len,countterlen));
	  /* for exceptional case, it is reasonable treat as normal gap, e.g. 250ms,(45000/4) instead of 0.*/
	  if(direction == eB_DVR_FileReadDirectionForward)
	  {
		 for(count = 0; count < (len-1); count++)
		 {
			ptsgap[count] = (pts[count+1]>pts[count]) ? (pts[count+1]-pts[count]) : 11200;
         	        frameOffsetgap[count] = (unsigned long)(frameOffset[count+1]-frameOffset[count]);
		 }
	  }
	  else
	  {
		 for(count = 0; count < (len-1); count++)
		 {
			ptsgap[count] = (pts[count]>pts[count+1]) ? (pts[count]-pts[count+1]) : 11200;
       	                frameOffsetgap[count] = (unsigned long)(frameOffset[count]-frameOffset[count+1]);
		 }
	  }
	
	  /*1. Find out the max value from ptsgap.*/
	  for(count = 0; count < (len-1); count++)
	  {
		 if(maxptsgap < ptsgap[count])
			maxptsgap = ptsgap[count];
	  }
	  BDBG_MSG(("MaxPtsdelta= %d",maxptsgap));
	  /*2. comparible and find out max value of each rank and counting*/
	  for(count = 0; count < (len-1); count++)
	  {
		 for (rank = 0; rank < NUM_OF_RANKS; rank++)
		 {
			if(((maxptsgap-ptsgap[count]) < ((maxptsgap*(rank + 1))/NUM_OF_RANKS))
			   &&((maxptsgap-ptsgap[count]) >= ((maxptsgap*(rank))/NUM_OF_RANKS)))
			{
                           if(tempptsgapresult[rank] < ptsgap[count])
                           {
			       tempptsgapresult[rank] = ptsgap[count];
      			       frameOffsetresult[rank] = frameOffsetgap[count];
                           }
			   ptsgapresult[rank] += ptsgap[count];
			   ptsgapcount[rank]++;
			   break;
			}
		 }
	  }
	
	  /*3. By the account, to decide the final ptsdelta.*/
	  for(count = 0,finalcount = 0,tempvalue = 0; count < countterlen; count++)
	  {
		 BDBG_MSG(("ptsgapcounter= %ld, count-index=%d",ptsgapcount[count],count));
		 if(tempvalue < ptsgapcount[count])
		 {
			tempvalue = ptsgapcount[count];
			finalcount = count;
		 }
	  }
	  /*4. using average value instead of largest one of the bank.*/
	  if (ptsgapcount[finalcount]){
		 ptsdelta  = ptsgapresult[finalcount]/ptsgapcount[finalcount];
	  }
	  mediaFile->FrameRateptsdelta = ptsdelta;
	  BDBG_MSG(("ptsdelta= %d, finalcount-index=%d, counter:%d",ptsdelta,finalcount,ptsgapcount[finalcount]));
          mediaFile->frameResource.FrameRateptsdelta = ptsdelta;
	  mediaFile->frameResource.FramesGOPBuf = frameOffsetresult[finalcount];

	  if (ptsdelta < 33){
		 BDBG_WRN(("ptsdelta less than one frame, finalcount-index=%d\n", finalcount));
		 mediaFile->StreamFrameRate.sfRateNumerator = defaultframeRate;
		 mediaFile->StreamFrameRate.sfRateDenominator = 0;
		 goto StreamIPFrameRate_err;
	  }
	  else{
		if((PTS_UNITS/ptsdelta)!=0)
		{
			mediaFile->StreamFrameRate.sfRateNumerator =
			   (PTS_UNITS%ptsdelta)?
			   ((PTS_UNITS%ptsdelta>=(ptsdelta/2))?(PTS_UNITS/ptsdelta+1):(PTS_UNITS/ptsdelta)):
			   (PTS_UNITS/ptsdelta);
			mediaFile->StreamFrameRate.sfRateDenominator = 0;
		}
		else
		{
			mediaFile->StreamFrameRate.sfRateNumerator = 0;
			mediaFile->StreamFrameRate.sfRateDenominator = 
			   (ptsdelta%PTS_UNITS)?
			   ((ptsdelta%PTS_UNITS>=PTS_UNITS/2)?(ptsdelta/PTS_UNITS):(ptsdelta/PTS_UNITS-1)):
			   (ptsdelta/PTS_UNITS-1);
		}
	  }
	  frameRate->sfRateNumerator = mediaFile->StreamFrameRate.sfRateNumerator;
	  frameRate->sfRateDenominator = mediaFile->StreamFrameRate.sfRateDenominator;
#if PTS_PACING
	  mediaFile->ptsGap = ptsdelta;
	  BDBG_MSG(("B_DVR_MediaFile_StreamIPFrameRate Numerator=%d Denominator=%d ptsGap=%d",frameRate->sfRateNumerator,frameRate->sfRateDenominator,mediaFile->ptsGap));
#endif
	  BDBG_MSG(("B_DVR_MediaFile_StreamIPFrameRate Numerator=%d Denominator=%d ptsdelta=%d",
		 frameRate->sfRateNumerator,frameRate->sfRateDenominator,ptsdelta));
	}
	else
	{
		BDBG_WRN(("readMode setting is %d", pSettings->fileOperation.streamingOperation.readMode)); 	
	}
StreamIPFrameRate_err:
	BDBG_MSG(("B_DVR_MediaFile_StreamIPFrameRate <<<<"));
	return rc;

}

B_DVR_ERROR B_DVR_MediaFile_GetStreamInfo(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileStreamFrameResource *frameRes)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_GetStreamInfo >>>>"));
    frameRes->FramesGOPBuf = mediaFile->frameResource.FramesGOPBuf; 
    frameRes->FrameRateptsdelta = (mediaFile->frameResource.FrameRateptsdelta*1000)/(1000*45);
    BDBG_MSG(("B_DVR_MediaFile_GetStreamInfo <<<<"));
    return rc;
}

B_DVR_ERROR B_DVR_MediaFile_GetChunkInfo(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileChunkInfo *chunkInfo)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(chunkInfo);
    BDBG_MSG(("B_DVR_MediaFile_GetChunkInfo >>>>"));
    BKNI_Memcpy((void*)chunkInfo,(void *)&mediaFile->chunkInfo,sizeof(*chunkInfo));
    BDBG_MSG(("B_DVR_MediaFile_GetChunkInfo <<<<"));
    return rc;
}    

B_DVR_ERROR B_DVR_MediaFile_GetLocation(
    B_DVR_MediaFileHandle mediaFile,
    unsigned long timestamp,
    B_DVR_FilePosition *position)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(position);
    BDBG_MSG(("B_DVR_MediaFile_GetLocation timestamp = %lx>>>>>",timestamp));
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    if(mediaFile->nexusFileRecord)
    {
        rc = B_DVR_SegmentedFileRecord_GetLocation(mediaFile->nexusFileRecord,-1,timestamp,position);
    }
    else
    {
        rc = B_DVR_SegmentedFilePlay_GetLocation(mediaFile->nexusFilePlay,-1,timestamp,position);
    }
    
    if (rc == B_DVR_SUCCESS && position)
    {
        BDBG_MSG(("B_DVR_MediaFile_GetLocation timestamp =%lx offset=%llx<<<<",
                  position->timestamp,position->mpegFileOffset));
    } 
    else 
    {
        BDBG_ERR(("B_DVR_MediaFile_GetLocation Failed"));
    }
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    return rc;
}

B_DVR_ERROR B_DVR_MediaFile_GetNextFrame(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_FilePosition *framePosition)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(framePosition);
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    if(mediaFile->nexusFileRecord)
    {
        rc = B_DVR_SegmentedFileRecord_GetLocation(
               mediaFile->nexusFileRecord, 
               (mediaFile->currentIndex+2),
               -1,
               framePosition);
        BDBG_MSG(("%s:currentIndex:%lx, offset=%lld<<<<",__FUNCTION__,
           mediaFile->currentIndex, framePosition->mpegFileOffset));
    }
    else
    {
        rc = B_DVR_SegmentedFilePlay_GetLocation(
               mediaFile->nexusFilePlay, 
               (mediaFile->currentIndex+2),
               -1,
               framePosition);
        BDBG_MSG(("%s: currentIndex:%lx, offset=%lld, returnindex:%lx<<<<",__FUNCTION__,
           mediaFile->currentIndex, framePosition->mpegFileOffset, framePosition->index));
    }
    
    if (rc != B_DVR_SUCCESS)
    {
        BDBG_ERR(("B_DVR_MediaFile_GetNextFrame Failed"));
    }
    else
    {
       mediaFile->currentNavOffset = framePosition->navFileOffset;
       mediaFile->currentseqHdrOffset = framePosition->seqHdrOffset;
       mediaFile->currentMediaTime = framePosition->timestamp;
       mediaFile->currentIndex = (framePosition->index) ? (framePosition->index - 1) : 0;
     }
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    return rc;
}


B_DVR_ERROR B_DVR_MediaFile_GetTimeStamp(
    B_DVR_MediaFileHandle mediaFile,
    off_t offset,
    unsigned long *timestamp)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(timestamp);
    if(mediaFile->nexusFileRecord)
    {
        rc = B_DVR_SegmentedFileRecord_GetTimestamp(mediaFile->nexusFileRecord,offset,timestamp);
    }
    else if(mediaFile->nexusFilePlay)
    {
        rc = B_DVR_SegmentedFilePlay_GetTimestamp(mediaFile->nexusFilePlay,offset,timestamp);
    }
    else
    {
        rc = B_DVR_INVALID_PARAMETER;
        BDBG_ERR(("no corresponding handle..."));
    }
     
    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("failed on getting Timestamp"));
        rc=B_DVR_NOT_INITIALIZED;
    }
    return rc;
}

B_DVR_MediaFileHandle B_DVR_MediaFile_Open(
   const char *mediaName,
   B_DVR_MediaFileOpenMode openMode,
   B_DVR_MediaFilePlayOpenSettings *openSettings)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_ManagerHandle dvrManager;
    B_DVR_MediaNode mediaNode=NULL;
    B_DVR_SegmentedFilePlaySettings segmentedFilePlaySettings;
    B_DVR_MediaFileHandle mediaFile = NULL;
    unsigned i;	
    bool bIsMpeg2=true;
    BDBG_ASSERT(mediaName);
    BDBG_MSG(("B_DVR_MediaFile_Open %s >>>>>",mediaName));
    dvrManager = B_DVR_Manager_GetHandle(); 
    BDBG_ASSERT(dvrManager);

    if(dvrManager->mediaFileCount >= B_DVR_MAX_MEDIAFILE)
    {
        BDBG_WRN(("mediaFile max count reached %u",dvrManager->mediaFileCount));
        goto error_mediaFileCount;
    }
    mediaFile = BKNI_Malloc(sizeof(B_DVR_MediaFile));
    if(!mediaFile)
    {
        BDBG_ERR(("mediaFile allocate failed"));
        rc  = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error_allocMediaFile;  
    }
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(B_DVR_MediaFile),
                                            true,__FUNCTION__,__LINE__);
    BKNI_Memset(mediaFile,0,sizeof(B_DVR_MediaFile));	
    BDBG_OBJECT_SET(mediaFile,B_DVR_MediaFile);

    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    /* find a mediaFile slot in the dvrManager */
    for(i=0;i<B_DVR_MAX_MEDIAFILE;i++)
    {
        if(!dvrManager->mediaFile[i])
        {
            BDBG_MSG(("mediaFile Index %u",index));
            mediaFile->index = i;
            break;
        }
    }
    if(i>=B_DVR_MAX_MEDIAFILE)
    {
        B_Mutex_Unlock(dvrManager->dvrManagerMutex);
        BDBG_ERR(("MAX mediaFile instances used up. Free up a mediaFile instance"));
        goto error_maxMediaFileCount;
    }
    dvrManager->mediaFileCount++;
    dvrManager->mediaFile[mediaFile->index] = mediaFile;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    mediaFile->mediaFileMutex = B_Mutex_Create(NULL);
    if(!mediaFile->mediaFileMutex)
    {
        BDBG_ERR(("mediaFile mutex create failed"));
        rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
        goto error_mediaFileMutex;
    }
    mediaFile->openMode = openMode;
    BKNI_Memcpy(mediaFile->mediaName,mediaName,B_DVR_MAX_FILE_NAME_LENGTH);  
    BKNI_Memcpy(&(mediaFile->openSettings),openSettings,sizeof(B_DVR_MediaFilePlayOpenSettings));
    switch(openMode)
    {
    case eB_DVR_MediaFileOpenModeStreaming:
        {
            B_DVR_MediaNodeSettings mediaNodeSettings;
            B_DVR_SegmentedFileSettings segmentedFileSettings;
            BDBG_MSG(("media Streaming"));
            if(openSettings->subDir[0]=='\0')
            {
                mediaNodeSettings.subDir = NULL;
            }
            else
            {
                mediaNodeSettings.subDir = &openSettings->subDir[0];
            }
            mediaNodeSettings.programName = (char *)mediaName;
            mediaNodeSettings.volumeIndex = openSettings->volumeIndex;
            mediaFile->mediaNode = B_DVR_List_GetMediaNode(dvrManager->dvrList,&mediaNodeSettings,true);
            #ifdef MEDIANODE_ONDEMAND_CACHING
            if(!mediaFile->mediaNode)
            {
                mediaFile->mediaNode = BKNI_Malloc(sizeof(*mediaFile->mediaNode));
                if(!mediaFile->mediaNode) 
                {
                    BDBG_ERR(("error in allocating playbackService->mediaNode for %s",mediaName));
                    rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
                    break;
                }
                B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                        sizeof(*mediaFile->mediaNode),
                                                        true,__FUNCTION__,__LINE__);
                rc = B_DVR_List_GetMediaNodeFile(dvrManager->dvrList,&mediaNodeSettings,mediaFile->mediaNode);
                if(rc!=B_DVR_SUCCESS) 
                {
                    BDBG_ERR(("error in getting mediaNode for %s",mediaName));
                    BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                            sizeof(*mediaFile->mediaNode),
                                                            false,__FUNCTION__,__LINE__);
                    BKNI_Free(mediaFile->mediaNode);
                    mediaFile->mediaNode = NULL;
                    rc = B_DVR_INVALID_PARAMETER;
                    break;
                }
                rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,
                                             mediaNodeSettings.volumeIndex,
                                             mediaFile->mediaNode);
                if(rc!=B_DVR_SUCCESS) 
                {
                    BDBG_ERR(("error in adding mediaNode for %s to the dvr list",mediaName));
                    BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                            sizeof(*mediaFile->mediaNode),
                                                            false,__FUNCTION__,__LINE__);
                    BKNI_Free(mediaFile->mediaNode);
                    mediaFile->mediaNode = NULL;
                    rc = B_DVR_UNKNOWN;
                    break;
                }
            }
            #else
            if(!mediaFile->mediaNode)
            {  
                 BDBG_ERR(("mediaNode not found for %s",mediaName));
                 rc = B_DVR_INVALID_PARAMETER;
                 break;
            }
            #endif
            BDBG_MSG(("mediaNode return from List_GetMediaNode = %x",mediaFile->mediaNode));
                     
            if(mediaFile->mediaNode->recording == eB_DVR_RecordingTSB)
            {
                BDBG_MSG(("Live streaming through TSB"));
                BDBG_MSG(("nav %s media %s info %s subDir %s tsbcount = %d",
                          mediaFile->mediaNode->navFileName,
                          mediaFile->mediaNode->mediaFileName,
                          mediaFile->mediaNode->mediaNodeFileName,
                          mediaFile->mediaNode->mediaNodeSubDir,
                          dvrManager->tsbServiceCount));
                BDBG_MSG(("mediaNode Programname = %s subdir=%s",mediaFile->mediaNode->programName,mediaFile->mediaNode->mediaNodeSubDir));
                B_Mutex_Lock(dvrManager->dvrManagerMutex);
                for(i=0;i<B_DVR_MAX_TSB;i++)
                {
                    B_DVR_MediaNode localMediaNode = NULL;
                    if(dvrManager->tsbService[i] == NULL) continue;
                    localMediaNode= B_DVR_TSBService_GetMediaNode(dvrManager->tsbService[i],false);
                    if(localMediaNode == NULL) continue;
                    BDBG_MSG((" local-----nav %s media %s info %s subDir %s localMediaNode=%x",
                    localMediaNode->navFileName,
                    localMediaNode->mediaFileName,
                    localMediaNode->mediaNodeFileName,
                    localMediaNode->mediaNodeSubDir,
                    localMediaNode ));
                    BDBG_MSG(("Local mediaNode Programname = %s subdir=%s",mediaFile->mediaNode->programName,mediaFile->mediaNode->mediaNodeSubDir));
                    if(!strcmp((mediaFile->mediaNode->programName),(localMediaNode->programName))&&
                       !strcmp((mediaFile->mediaNode->mediaNodeSubDir),(localMediaNode->mediaNodeSubDir)))
                    {
                        BDBG_MSG((" local-----Found medianode in TSB loop"));
                        mediaFile->tsbService = dvrManager->tsbService[i];
                        mediaFile->nexusFileRecord = B_DVR_TSBService_GetFileRecordHandle(mediaFile->tsbService);
                        break;
                    }
                }
                B_Mutex_Unlock(dvrManager->dvrManagerMutex);
                if(i<B_DVR_MAX_TSB)
                { 
                    BDBG_MSG(("mediaNode for %s associated with an active TSB",mediaName));
                } 
                else
                {
                    BDBG_MSG(("mediaNode for %s not associated with an active TSB",mediaName));
                    rc = B_DVR_INVALID_PARAMETER;
                    break;
                }
            }
            else
            {
                BDBG_MSG(("Streaming of permanent recording"));
            }

            segmentedFileSettings.event = NULL;        /*unused in segmented playback */
            segmentedFileSettings.maxSegmentCount = 0; /*unused in segmented playback */
            segmentedFileSettings.mediaSegmentSize = 0; /*unused in segmented playback */
            segmentedFileSettings.mediaStorage = dvrManager->mediaStorage;
            segmentedFileSettings.registeredCallback = NULL; //B_DVR_PlaybackService_P_SegmentedFileEvent;
            segmentedFileSettings.service = eB_DVR_ServicePlayback;
            segmentedFileSettings.serviceIndex = dvrManager->playbackServiceCount;
            segmentedFileSettings.volumeIndex = openSettings->volumeIndex;
            if(openSettings->subDir[0]=='\0')
            {
                 segmentedFileSettings.metaDataSubDir = NULL;
            }
            else
            {
                segmentedFileSettings.metaDataSubDir = (char *)mediaFile->mediaNode->mediaNodeSubDir;
            }
            if(mediaFile->nexusFileRecord)
            {
                segmentedFileSettings.service = eB_DVR_ServiceTSB;
                mediaFile->nexusFilePlay = B_DVR_SegmentedFilePlay_Open(mediaFile->mediaNode->mediaFileName,
                                                                        mediaFile->mediaNode->navFileName,
                                                                        &segmentedFileSettings,
                                                                        mediaFile->nexusFileRecord); /*NULL);*/
            }
            else
            {
                if(mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED) 
                {
                    rc = B_DVR_SegmentedFilePlay_SanityCheck(mediaFile->mediaNode->mediaFileName,
                                                             mediaFile->mediaNode->navFileName,
                                                             &segmentedFileSettings);
                    if(rc!=B_DVR_SUCCESS) 
                    {
                        BDBG_ERR(("%s:Program:%s is corrupted and it's recommended to delete this program",
                                  __FUNCTION__,mediaFile->mediaNode->programName));
                        rc = B_DVR_INVALID_PARAMETER;
                        break;
                    }
                }

                mediaFile->nexusFilePlay = B_DVR_SegmentedFilePlay_Open(mediaFile->mediaNode->mediaFileName,
                                                                        mediaFile->mediaNode->navFileName,
                                                                        &segmentedFileSettings,
                                                                        NULL);
            }
             
            if(!mediaFile->nexusFilePlay)
            {
                BDBG_ERR(("error in opening segmentedFile media %s nav %s",
                          mediaFile->mediaNode->mediaFileName,mediaFile->mediaNode->navFileName));
                rc = B_DVR_INVALID_PARAMETER;
                break;
            }

            if(!(mediaFile->nexusFileRecord))
            {
                for(i=0;i<mediaFile->mediaNode->esStreamCount;i++)
                { 
                    if(mediaFile->mediaNode->esStreamInfo[i].pidType == eB_DVR_PidTypeVideo)
                    {
                        bIsMpeg2 = (mediaFile->mediaNode->esStreamInfo[i].codec.videoCodec== NEXUS_VideoCodec_eMpeg2) ? true: false;
                        BDBG_MSG(("bIsMpeg2:%d", bIsMpeg2));
                        break;
                    }
                }  
                B_DVR_SegmentedFilePlay_GetDefaultSettings(&segmentedFilePlaySettings);
                segmentedFilePlaySettings.mediaLinearStartOffset = mediaFile->mediaNode->mediaLinearStartOffset;
                segmentedFilePlaySettings.mediaLinearEndOffset = mediaFile->mediaNode->mediaLinearEndOffset;
                segmentedFilePlaySettings.navLinearEndOffset = mediaFile->mediaNode->navLinearEndOffset;
                segmentedFilePlaySettings.navLinearStartOffset = mediaFile->mediaNode->navLinearStartOffset;
                if (bIsMpeg2 == true)
                {
                    segmentedFilePlaySettings.navEntrySize = sizeof(BNAV_Entry);
                }  
                else 
                {
                    segmentedFilePlaySettings.navEntrySize = sizeof(BNAV_AVC_Entry);
                }

                B_DVR_SegmentedFilePlay_SetSettings(mediaFile->nexusFilePlay,&segmentedFilePlaySettings);
                if(mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED) 
                {
                    B_DVR_FilePosition first,last;
                    rc = B_DVR_SegmentedFilePlay_GetBounds(mediaFile->nexusFilePlay,&first,&last);
                    if(rc!=B_DVR_SUCCESS) 
                    {
                        B_DVR_SegmentedFilePlay_Close(mediaFile->nexusFilePlay);
                        BDBG_ERR(("B_DVR_SegmentedFilePlay_GetBounds failed for aborted recording %s",
                        mediaFile->mediaNode->programName));
                        rc=B_DVR_INVALID_PARAMETER;
                        break;
                    }
                    BDBG_WRN(("%s:updating the nav end off %lld with %lld",
                              __FUNCTION__,
                              mediaFile->mediaNode->navLinearEndOffset,
                              last.navFileOffset));
                    BDBG_WRN(("%s:updating the media end off %lld with %lld",
                              __FUNCTION__,
                              mediaFile->mediaNode->mediaLinearEndOffset,
                              last.mpegFileOffset));
                    BDBG_WRN(("%s:updating the media end time %u with %u",
                              __FUNCTION__,
                              mediaFile->mediaNode->mediaEndTime,
                              last.timestamp));
                    mediaFile->mediaNode->mediaLinearEndOffset = last.mpegFileOffset;
                    mediaFile->mediaNode->navLinearEndOffset = last.navFileOffset;
                    mediaFile->mediaNode->mediaEndTime = last.timestamp;
                    mediaFile->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED);
                    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,
                                               mediaFile->mediaNode,
                                               openSettings->volumeIndex,true);
                }
                segmentedFilePlaySettings.mediaLinearStartOffset = mediaFile->mediaNode->mediaLinearStartOffset;
                segmentedFilePlaySettings.mediaLinearEndOffset = mediaFile->mediaNode->mediaLinearEndOffset;
                segmentedFilePlaySettings.navLinearEndOffset = mediaFile->mediaNode->navLinearEndOffset;
                segmentedFilePlaySettings.navLinearStartOffset = mediaFile->mediaNode->navLinearStartOffset;
                if (bIsMpeg2 == true)
                {
                    segmentedFilePlaySettings.navEntrySize = sizeof(BNAV_Entry);
                }
                else 
                {
                    segmentedFilePlaySettings.navEntrySize = sizeof(BNAV_AVC_Entry);
                }				 
                B_DVR_SegmentedFilePlay_SetSettings(mediaFile->nexusFilePlay,&segmentedFilePlaySettings);
            }

            if(mediaFile->nexusFileRecord ||
               mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS)
            {
                char fullFileName[B_DVR_MAX_FILE_NAME_LENGTH];
                char metaDataPath[B_DVR_MAX_FILE_NAME_LENGTH];
                struct epoll_event epollEvent;
                B_DVR_MediaStorage_GetMetadataPath(dvrManager->mediaStorage,mediaNodeSettings.volumeIndex,metaDataPath);
                BDBG_MSG(("metaDataPath for vol %u is %s",mediaNodeSettings.volumeIndex,metaDataPath));
                if(mediaFile->mediaNode->mediaNodeSubDir[0]=='\0')
                {
                    BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s",metaDataPath,
                                  mediaFile->mediaNode->mediaNodeFileName);
                }
                else
                {
                    BKNI_Snprintf(fullFileName, sizeof(fullFileName),"%s/%s/%s", metaDataPath,
                                  mediaFile->mediaNode->mediaNodeSubDir,mediaFile->mediaNode->mediaNodeFileName);
                }
                BDBG_MSG(("media node file %s",fullFileName));
                mediaFile->inotifyFd = inotify_init();
                if(mediaFile->inotifyFd == -1)
                {
                    BDBG_ERR(("inotify_init failed"));
                    rc = B_DVR_OS_ERROR;
                    break;
                }
                mediaFile->inotifyWatchFd = inotify_add_watch(mediaFile->inotifyFd,fullFileName,IN_MODIFY);
                if(mediaFile->inotifyWatchFd == -1)
                {
                    BDBG_ERR(("inotify_add_watch"));
                    rc = B_DVR_OS_ERROR;
                    break;
                }
                BDBG_MSG(("mediaFile->inotifyWatchFd %d",mediaFile->inotifyWatchFd));
                mediaFile->epollFd = epoll_create(1);
                if(mediaFile->epollFd==-1)
                {
                    BDBG_ERR(("epoll_create failed"));
                    inotify_rm_watch(mediaFile->inotifyFd,mediaFile->inotifyWatchFd);
                    rc = B_DVR_OS_ERROR;
                    break;
                }
                epollEvent.data.fd = mediaFile->inotifyFd;
                epollEvent.events = EPOLLIN;
                if(epoll_ctl(mediaFile->epollFd,EPOLL_CTL_ADD,mediaFile->inotifyFd,&epollEvent)==-1)
                {
                    BDBG_ERR(("epoll_ctl failed"));
                    inotify_rm_watch(mediaFile->inotifyFd,mediaFile->inotifyWatchFd);
                    rc = B_DVR_OS_ERROR;
                    break;
                }
            }
            else
            {
                mediaFile->epollFd = -1;
                mediaFile->inotifyFd = -1;
            }
            mediaFile->currentMediaOffset = mediaFile->mediaNode->mediaLinearStartOffset;
            mediaFile->currentNavOffset = mediaFile->mediaNode->navLinearStartOffset;
        }
        break;
    case eB_DVR_MediaFileOpenModePlayback:
        {
            B_DVR_PlaybackServiceRequest playbackServiceRequest;
            B_DVR_MediaNodeSettings mediaNodeSettings;
            BKNI_Memcpy(playbackServiceRequest.programName,mediaName,sizeof(playbackServiceRequest.programName));
            BKNI_Memcpy(playbackServiceRequest.subDir,openSettings->subDir,sizeof(playbackServiceRequest.subDir));
            playbackServiceRequest.volumeIndex = openSettings->volumeIndex;
            playbackServiceRequest.playpumpIndex = openSettings->playpumpIndex;
            mediaNodeSettings.programName = (char *)mediaName;
            if(openSettings->subDir[0] == '\0')
            {
                mediaNodeSettings.subDir = NULL;
            }
            else
            {
                mediaNodeSettings.subDir = openSettings->subDir;
            }
            mediaNodeSettings.volumeIndex = openSettings->volumeIndex;
            mediaFile->mediaNode = B_DVR_List_GetMediaNode(dvrManager->dvrList,&mediaNodeSettings,true);
            #ifdef MEDIANODE_ONDEMAND_CACHING
             if(!mediaFile->mediaNode)
             {
                 mediaFile->mediaNode = BKNI_Malloc(sizeof(*mediaFile->mediaNode));
                 if(!mediaFile->mediaNode) 
                 {
                     BDBG_ERR(("error in allocating playbackService->mediaNode for %s",mediaName));
                     rc = B_DVR_OUT_OF_SYSTEM_MEMORY;
                     break;
                 }
                 B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                         sizeof(*mediaFile->mediaNode),
                                                         true,__FUNCTION__,__LINE__);
                 rc = B_DVR_List_GetMediaNodeFile(dvrManager->dvrList,&mediaNodeSettings,mediaFile->mediaNode);
                 if(rc!=B_DVR_SUCCESS) 
                 {
                     BDBG_ERR(("error in getting mediaNode for %s",mediaName));
                     BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                     B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                             sizeof(*mediaFile->mediaNode),
                                                             false,__FUNCTION__,__LINE__);
                     BKNI_Free(mediaFile->mediaNode);
                     mediaFile->mediaNode = NULL;
                     break;
                 }
                 rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,
                                              mediaNodeSettings.volumeIndex,
                                              mediaFile->mediaNode);
                 if(rc!=B_DVR_SUCCESS) 
                 {
                     BDBG_ERR(("error in adding mediaNode for %s to the dvr list",mediaName));
                     BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                     B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                             sizeof(*mediaFile->mediaNode),
                                                             false,__FUNCTION__,__LINE__);
                     BKNI_Free(mediaFile->mediaNode);
                     mediaFile->mediaNode = NULL;
                 }
             }
             #endif
            if(!mediaFile->mediaNode)
            {
                BDBG_ERR(("mediaNode not found %s",mediaName));
                rc = B_DVR_INVALID_PARAMETER;
                break;
            }
            mediaFile->playbackService= B_DVR_PlaybackService_Open(&playbackServiceRequest);
            if(!mediaFile->playbackService)
            {
                BDBG_ERR(("Unable to open playbackService"));
                #ifdef MEDIANODE_ONDEMAND_CACHING
                if(!B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                               mediaFile->openSettings.volumeIndex,
                                               mediaFile->mediaNode)) 
                {
                    BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                            sizeof(*mediaFile->mediaNode),
                                                            false,__FUNCTION__,__LINE__);
                    BKNI_Free(mediaFile->mediaNode);
                }
                #endif
                rc = B_DVR_UNKNOWN;
            }
            rc = B_DVR_PlaybackService_InstallCallback(mediaFile->playbackService,NULL,NULL);     
        }
        break;
    case eB_DVR_MediaFileOpenModeRecord:
        {
            char metaDataPath[B_DVR_MAX_FILE_NAME_LENGTH];
            B_DVR_SegmentedFileSettings segmentedFileSettings;
            BDBG_MSG(("mediaFileRecording"));
			BKNI_Memset((void *)&segmentedFileSettings,0,sizeof(B_DVR_SegmentedFileSettings));
            mediaFile->mediaNode = BKNI_Malloc(sizeof(*mediaNode));
            if(!mediaFile->mediaNode)
            {
                BDBG_ERR(("mediaNode allocate failed"));
                rc=B_DVR_UNKNOWN;
                break;
            }
            B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                    sizeof(*mediaNode), true,
                                                    __FUNCTION__,__LINE__);
            BKNI_Memset((void *)mediaFile->mediaNode,0,sizeof(*mediaFile->mediaNode));
            BDBG_OBJECT_SET(mediaNode,B_DVR_Media);
            mediaFile->mediaNode->mediaAttributes &= ~(B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM);
            mediaFile->mediaNode->recording = eB_DVR_RecordingPermanent;
            mediaFile->mediaNode->mediaAttributes |= B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM;
            mediaFile->mediaNode->transportType = NEXUS_TransportType_eTs;
            rc = B_DVR_MediaStorage_GetMetadataPath(dvrManager->mediaStorage,openSettings->volumeIndex,metaDataPath);
            if (rc!=B_DVR_SUCCESS) 
            {
                BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                        sizeof(*mediaFile->mediaNode),
                                                        false,__FUNCTION__,__LINE__);
                BKNI_Free(mediaFile->mediaNode);
                goto error_mediaFileMutex;
            }            
            mediaFile->mediaNode->navFileName[0] = '\0';
            if(openSettings->subDir[0]=='\0')
            {
                BKNI_Snprintf(mediaFile->mediaNode->mediaFileName,sizeof(mediaFile->mediaNode->mediaFileName),"%s/%s%s",
                              metaDataPath,mediaName,B_DVR_MEDIA_FILE_EXTENTION);
                BKNI_Snprintf(mediaFile->mediaNode->mediaNodeFileName,sizeof(mediaFile->mediaNode->mediaNodeFileName),"%s/%s%s",
                              metaDataPath,mediaName,B_DVR_MEDIA_NODE_FILE_EXTENTION);

            }
            else
            {
                BKNI_Snprintf(mediaFile->mediaNode->mediaFileName,sizeof(mediaFile->mediaNode->mediaFileName),"%s/%s/%s%s",
                              metaDataPath,openSettings->subDir,mediaName,B_DVR_MEDIA_FILE_EXTENTION);
                BKNI_Snprintf(mediaFile->mediaNode->mediaNodeFileName,sizeof(mediaFile->mediaNode->mediaNodeFileName),"%s/%s/%s%s",
                              metaDataPath,openSettings->subDir,mediaName,B_DVR_MEDIA_NODE_FILE_EXTENTION);
            }
            strncpy(mediaFile->mediaNode->programName,mediaName,B_DVR_MAX_FILE_NAME_LENGTH);
            mediaFile->mediaNode->programName[B_DVR_MAX_FILE_NAME_LENGTH-1]='\0';

            BDBG_MSG(("media %s, nav %s mediainfo %s",mediaFile->mediaNode->mediaFileName,
                      mediaFile->mediaNode->navFileName,mediaFile->mediaNode->mediaNodeFileName));

            if(openSettings->subDir[0]!='\0')
            {
                strncpy(mediaFile->mediaNode->mediaNodeSubDir,openSettings->subDir,B_DVR_MAX_FILE_NAME_LENGTH);
                mediaFile->mediaNode->mediaNodeSubDir[B_DVR_MAX_FILE_NAME_LENGTH-1] = '\0';
            }

            #ifdef MEDIANODE_ONDEMAND_CACHING
            rc = B_DVR_List_AddMediaNodeFile(dvrManager->dvrList,openSettings->volumeIndex,mediaFile->mediaNode);
            if(rc!=B_DVR_SUCCESS)
            {
                BDBG_MSG(("error in creating %s",mediaFile->mediaNode->mediaNodeFileName));
                BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                        sizeof(*mediaFile->mediaNode),
                                                        false,__FUNCTION__,__LINE__);
                BKNI_Free((void *)(mediaFile->mediaNode));
                break;
            }
            #endif

            rc = B_DVR_List_AddMediaNode(dvrManager->dvrList,openSettings->volumeIndex,mediaFile->mediaNode);
            if(rc!=B_DVR_SUCCESS)
            {
                BDBG_ERR(("error in adding mediaNode"));
                #ifdef MEDIANODE_ONDEMAND_CACHING
                B_DVR_List_RemoveMediaNodeFile(dvrManager->dvrList,openSettings->volumeIndex,mediaFile->mediaNode);
                #endif
                BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                        sizeof(*mediaFile->mediaNode),
                                                        false,__FUNCTION__,__LINE__);
                BKNI_Free((void *)(mediaFile->mediaNode));
                break;
            }

            segmentedFileSettings.mediaStorage = dvrManager->mediaStorage;
            segmentedFileSettings.registeredCallback = NULL;
            segmentedFileSettings.volumeIndex = openSettings->volumeIndex;
            segmentedFileSettings.mediaSegmentSize = dvrManager->mediaSegmentSize;
            segmentedFileSettings.serviceIndex = mediaFile->index;
            segmentedFileSettings.service = eB_DVR_ServiceMedia;
            segmentedFileSettings.maxSegmentCount = 0;
            segmentedFileSettings.event = NULL;
            mediaFile->nexusFileRecord = B_DVR_SegmentedFileRecord_Open(mediaFile->mediaNode->mediaFileName,
                                                                        NULL,
                                                                        &segmentedFileSettings);
            if(!mediaFile->nexusFileRecord)
            {
                BDBG_ERR(("failed to open segmentedFile : media %s, nav %s mediainfo %s",mediaFile->mediaNode->mediaFileName,
                      mediaFile->mediaNode->navFileName,mediaFile->mediaNode->mediaNodeFileName));
                rc = B_DVR_UNKNOWN;
                #ifdef MEDIANODE_ONDEMAND_CACHING
                B_DVR_List_RemoveMediaNodeFile(dvrManager->dvrList,openSettings->volumeIndex,mediaFile->mediaNode);
                #endif
                /* no need to check the error has the network record hasn't started*/
                B_DVR_List_RemoveMediaNode(dvrManager->dvrList,openSettings->volumeIndex,mediaFile->mediaNode);
                BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
                B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                        sizeof(*mediaFile->mediaNode),
                                                        false,__FUNCTION__,__LINE__);
                BKNI_Free(mediaFile->mediaNode);
            }

        }
        break;
    default:
        BDBG_ERR(("Invalid mediaFile operation"));

    }

    if(rc!=B_DVR_SUCCESS)
    {
        BDBG_ERR(("openMode operation failed"));
        goto error_mediaFile;
    }

    BDBG_MSG(("B_DVR_MediaFile_Open <<<<"));
    return mediaFile;
error_mediaFile:
    B_Mutex_Destroy(mediaFile->mediaFileMutex);
error_mediaFileMutex:
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->mediaFileCount--;
    dvrManager->mediaFile[mediaFile->index] = NULL;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);
error_maxMediaFileCount:
     BDBG_OBJECT_DESTROY(mediaFile,B_DVR_MediaFile);
     B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                             sizeof(*mediaFile),
                                             false,__FUNCTION__,__LINE__);
     BKNI_Free(mediaFile);
error_allocMediaFile:
error_mediaFileCount:
    BDBG_ERR(("B_DVR_MediaFile_Open rc %d<<<<",rc));
    return NULL;
    
}

B_DVR_ERROR B_DVR_MediaFile_Close(
    B_DVR_MediaFileHandle mediaFile)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_ASSERT(mediaFile);
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle(); 
    BDBG_ASSERT(dvrManager);
    BDBG_MSG(("B_DVR_MediaFile_Close >>>>"));
    B_Mutex_Lock(dvrManager->dvrManagerMutex);
    dvrManager->mediaFile[mediaFile->index] = NULL;
    dvrManager->mediaFileCount--;
    B_Mutex_Unlock(dvrManager->dvrManagerMutex);

    B_Mutex_Lock(mediaFile->mediaFileMutex);
    switch(mediaFile->openMode)
    {
    case eB_DVR_MediaFileOpenModeStreaming:
        {
            if(mediaFile->epollFd >=0)
            {
                struct epoll_event epollEvent;
                epollEvent.events = EPOLLIN;
                epollEvent.data.fd = mediaFile->inotifyFd;
                epoll_ctl(mediaFile->epollFd,EPOLL_CTL_DEL,mediaFile->inotifyFd,&epollEvent);
                close(mediaFile->epollFd);
            }
            if(mediaFile->inotifyFd >=0)
            {
                inotify_rm_watch(mediaFile->inotifyFd,mediaFile->inotifyWatchFd);
                close(mediaFile->inotifyFd);
            }
            if(mediaFile->nexusFilePlay) 
            {
                B_DVR_SegmentedFilePlay_Close(mediaFile->nexusFilePlay);
            }
        }
        break;
    case eB_DVR_MediaFileOpenModePlayback:
        {
            B_DVR_PlaybackService_RemoveCallback(mediaFile->playbackService);
            B_DVR_PlaybackService_Close(mediaFile->playbackService);
        }
        break;
    case eB_DVR_MediaFileOpenModeRecord:
        {
            unsigned esStreamIndex=0;  
            B_DVR_SegmentedFileRecord_Close(mediaFile->nexusFileRecord);
            for(esStreamIndex=0;esStreamIndex<mediaFile->mediaNode->esStreamCount;esStreamIndex++)
            { 
                BDBG_ERR(("pid %u",mediaFile->mediaNode->esStreamInfo[esStreamIndex].pid));
            
            }
        }
        break;
    default:
        BDBG_ERR(("invalid openMode"));
    }
    #ifdef MEDIANODE_ONDEMAND_CACHING
    if(!B_DVR_List_RemoveMediaNode(dvrManager->dvrList,
                                   mediaFile->openSettings.volumeIndex,
                                   mediaFile->mediaNode)) 
    {
        BDBG_OBJECT_DESTROY(mediaFile->mediaNode,B_DVR_Media);
        B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                                sizeof(*mediaFile->mediaNode),
                                                false,__FUNCTION__,__LINE__);
        BKNI_Free(mediaFile->mediaNode);
    }
    #endif
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    BDBG_OBJECT_DESTROY(mediaFile,B_DVR_MediaFile);
    B_DVR_Manager_P_UpdateSystemMemoryUsage(eB_DVR_MemoryType_System,
                                            sizeof(*mediaFile), false,
                                            __FUNCTION__,__LINE__);
    BKNI_Free(mediaFile);
    BDBG_MSG(("B_DVR_MediaFile_Close <<<<"));
    return rc; 
}


B_DVR_ERROR B_DVR_MediaFile_GetDefaultSettings(
    B_DVR_MediaFileSettings *pSettings)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_MediaFile_GetDefaultSettings >>>"));
	
    if (pSettings)
    {
        pSettings->seekSetting.seekmode = eB_DVR_MediaFileSeekModeByteOffsetBased;
        pSettings->seekSetting.mediaSeek.offset = 0;
        pSettings->fileOperation.streamingOperation.direction = eB_DVR_MediaFileDirectionForward;
        pSettings->fileOperation.streamingOperation.readMode = eB_DVR_MediaFileReadModeFull;
    }
    BDBG_MSG(("B_DVR_MediaFile_GetDefaultSettings <<<"));    
    return rc;
}

B_DVR_ERROR B_DVR_MediaFile_SetSettings(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileSettings *pSettings)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(pSettings);
    BDBG_MSG(("B_DVR_MediaFile_SetSettings >>>>"));

    B_Mutex_Lock(mediaFile->mediaFileMutex);
    if(mediaFile->openMode == eB_DVR_MediaFileOpenModeRecord)
    {
        BDBG_ERR(("Settings not required for media recording"));
        goto error;
    }
    BKNI_Memcpy((void *)&mediaFile->settings,(void *)pSettings,sizeof(*pSettings));
    #if PTS_PACING
    B_DVR_MediaFile_P_ResetVar(mediaFile);
    #endif
    if(mediaFile->openMode == eB_DVR_MediaFileOpenModePlayback)
    {
       if((pSettings->drmHandle == NULL)||((pSettings->fileOperation.playbackOperation.operation) < eB_DVR_OperationMax))
       {
         BDBG_MSG(("B_DVR_MediaFile_SetSettings: Playback mode set operation"));
         B_DVR_PlaybackService_SetOperation(mediaFile->playbackService,&mediaFile->settings.fileOperation.playbackOperation);
       }
       else
         BDBG_MSG(("B_DVR_MediaFile_SetSettings: Playback mode Non-set operation"));
    }

error:
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    return rc;
}

B_DVR_ERROR B_DVR_MediaFile_GetSettings(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileSettings *pSettings)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(pSettings);
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    BKNI_Memcpy((void *)pSettings,(void *)&mediaFile->settings,sizeof(*pSettings));
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    return rc;
}

ssize_t B_DVR_MediaFile_Read(
    B_DVR_MediaFileHandle mediaFile,
    unsigned *buffer, ssize_t size)
{
    B_DVR_FilePosition currentPosition;
    B_DVR_FilePosition iFramePosition;
    ssize_t returnSize=0;
    ssize_t readSize = 0;
    off_t returnSeekOffset=0;
    B_DVR_FileReadDirection direction;
    B_DVR_MediaFileSettings *pSettings = &mediaFile->settings;
    unsigned sizeOfFrame = 0;   
    off_t linearEndOffset,linearStartOffset;
    uint8_t *chunkbuffer;
    uint8_t  SequenceEndPkt[8];
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    int retryCount = 0;
    bool reReadOnlyOnce = false;


    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(buffer);
    BDBG_MSG(("B_DVR_MediaFile_Read mediaName %s desiredSize %x >>>",mediaFile->mediaName,(unsigned)(size)));
    BDBG_MSG(("buffer 0x%x size %x end %x",buffer,size, (uint8_t *)buffer + size));
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    BKNI_Memset(&currentPosition,0,sizeof(B_DVR_FilePosition));
    BKNI_Memset(&iFramePosition,0,sizeof(B_DVR_FilePosition));
    /*
     * Buffer size and address alignment check
     */
    if(((unsigned long)buffer%B_DVR_IO_BLOCK_SIZE)!=0)
    {
        BDBG_MSG(("buffer address isn't 4K aligned"));
    }

    if((size%B_DVR_IO_BLOCK_SIZE)!=0)
    {
        BDBG_MSG(("buffer size not 4K aligned"));
    }
    /*
     * By default try updating the bounds for 
     * in progress recording since the mediaNode on
     * recording is continuously updated based on
     * threshold interrupts. This isn't applicable
     * to TSB buffering because file bounds for 
     * read side are automatically updated.
     */
    if(!mediaFile->nexusFileRecord && 
       (mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS))
    {
        B_DVR_MediaFile_P_InProgressRecordingUpdate(mediaFile);
    }

    /*
     * Try file IO wait if the read is beyond the
     * record side bounds for either TSB buffering
     * or in-progress recording.
     */
    if(mediaFile->nexusFileRecord || 
       mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS)
    {
        if((mediaFile->currentMediaOffset < mediaFile->mediaNode->mediaLinearStartOffset ) ||
           (((mediaFile->currentMediaOffset)+size) > mediaFile->mediaNode->mediaLinearEndOffset))
        {
             int returnValue;
             BDBG_WRN(("CurrentMediaoffset beyonds tsb startoffset and endoffset"));
             BDBG_WRN(("CurrentMediaoffset=%llx",mediaFile->currentMediaOffset));
             BDBG_WRN(("startoffset=%llx endoffset=%llx",mediaFile->mediaNode->mediaLinearStartOffset,
                       mediaFile->mediaNode->mediaLinearEndOffset));
             do 
             {
                 returnValue = B_DVR_MediaFile_P_Poll(mediaFile, B_DVR_MEDIANODE_UPDATE_INTERVAL);               
                 retryCount++;
             }while(((mediaFile->currentMediaOffset+size) > mediaFile->mediaNode->mediaLinearEndOffset) && retryCount < B_DVR_SEGMENTED_RECORD_POLL_TIME);
             
             /*  
              * Since segmented bounds aren't updated 
              * automatically for in-progress recording
              * on the read side, update them here;
              */ 
             if (!mediaFile->nexusFileRecord && returnValue>=0) 
             {
                 B_DVR_MediaFile_P_InProgressRecordingUpdate(mediaFile);
             }
         }
    }
	
    BKNI_Memset((void *)&currentPosition,0,sizeof(B_DVR_FilePosition));
    SequenceEndPkt[0]=0; SequenceEndPkt[1]=0; SequenceEndPkt[2]=0x01;SequenceEndPkt[3]=0xb7;
    SequenceEndPkt[4]=0; SequenceEndPkt[5]=0; SequenceEndPkt[6]=0x01;SequenceEndPkt[7]=0xb7;
    if((pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIFrame) || 
       (pSettings->fileOperation.streamingOperation.readMode == eB_DVR_MediaFileReadModeIPFrame))
    {
     if (mediaFile->moreData == 0)
     {
        linearEndOffset = mediaFile->mediaNode->mediaLinearEndOffset;
        BDBG_MSG(("readMode: I/IP Frame linearendoffset = %llx",linearEndOffset));
        BKNI_Memset(&(mediaFile->chunkInfo),0,sizeof(mediaFile->chunkInfo));
        chunkbuffer = (uint8_t *)buffer;

        BDBG_MSG(("buffer %d returnSize %x",chunkbuffer,returnSize));
        currentPosition.mpegFileOffset = mediaFile->currentMediaOffset;
        currentPosition.navFileOffset = mediaFile->currentNavOffset;
        currentPosition.timestamp = mediaFile->currentMediaTime;
        currentPosition.index = mediaFile->currentIndex;
        direction = (B_DVR_FileReadDirection)(pSettings->fileOperation.streamingOperation.direction);

NextIFrameRead:
        BDBG_MSG(("\n Mediafile segmentedMedia--currentofftset=%llx currentindex =%lx \n",currentPosition.mpegFileOffset,currentPosition.index));
        if(mediaFile->nexusFileRecord)
        {
            B_DVR_SegmentedFileRecord_GetNextIFrame(mediaFile->nexusFileRecord,currentPosition,&iFramePosition,&sizeOfFrame,direction);
        }
        else
        {
            B_DVR_SegmentedFilePlay_GetNextIFrame(mediaFile->nexusFilePlay,currentPosition,&iFramePosition,&sizeOfFrame,direction);
        }
        if((iFramePosition.index == 0)&&(mediaFile->currentMediaOffset == iFramePosition.mpegFileOffset)&&
           (direction != eB_DVR_FileReadDirectionForward))
        {
            returnSize = 0;
            goto EndofMediaRead;
        }
        
        if(((iFramePosition.mpegFileOffset+sizeOfFrame) >= linearEndOffset) &&
           (direction == eB_DVR_FileReadDirectionForward))
        {
            BDBG_MSG(("End/Beginning of file :: returnSize=%x desiredReadSize = %x",returnSize,sizeOfFrame));
            goto EndofMediaRead;
        }
        if((iFramePosition.mpegFileOffset+sizeOfFrame) <= linearEndOffset)
        {
             mediaFile->currentMediaOffset = iFramePosition.mpegFileOffset;
             mediaFile->currentNavOffset = iFramePosition.navFileOffset;
             mediaFile->currentMediaTime = iFramePosition.timestamp;
             mediaFile->currentseqHdrOffset = iFramePosition.seqHdrOffset;
             if(mediaFile->currentIFramePTS != mediaFile->previousIFramePTS)
             {
                BDBG_MSG(("previousIFramePTS=%d firstcurrentIFramePTS=%d",mediaFile->previousIFramePTS,mediaFile->currentIFramePTS));
                mediaFile->previousIFramePTS = mediaFile->currentIFramePTS;
             }
             mediaFile->currentIFramePTS = iFramePosition.pts;
             BDBG_MSG(("RealcurrentIFramePTS=%d",mediaFile->previousIFramePTS,mediaFile->currentIFramePTS));
             linearStartOffset = mediaFile->mediaNode->mediaLinearStartOffset;

             if( mediaFile->currentMediaOffset != linearStartOffset)
             {
                unsigned long realseqHdrOffset = 0;
                /* for the case more I frame sharing one sequence header, e.g. several I frame in one GOP.*/
                /* if current I frame not directly following sequence header, only send I frame then...*/
                /*188 to be extended to the largest possible value of distance between sequence header and first pic header */
                realseqHdrOffset = (mediaFile->currentseqHdrOffset < 188) ? mediaFile->currentseqHdrOffset : 0;
                sizeOfFrame += realseqHdrOffset;
                mediaFile->currentMediaOffset -= realseqHdrOffset;
                sizeOfFrame += (mediaFile->currentMediaOffset%188);
                mediaFile->currentMediaOffset -= (mediaFile->currentMediaOffset%188);   
                BDBG_MSG(("currentseqHdrOffset=%x currentmediaoff=%llx",mediaFile->currentseqHdrOffset,mediaFile->currentMediaOffset));
                BDBG_MSG(("sizeOfFrameAdjuested=%x ",sizeOfFrame));
             }
             BDBG_MSG(("FrameType--- currentIFramePTS=%d",mediaFile->currentIFramePTS));
             BDBG_MSG(("sizeOfFrameAdjuested=%x offbyte=%llx ",sizeOfFrame,(mediaFile->currentMediaOffset%188)));
			 BDBG_MSG(("navOffset:%llx, currentindex:%lu, sizeOfFrameAdjuested=%x offbyte=%llx ", mediaFile->currentNavOffset, iFramePosition.index, sizeOfFrame,(mediaFile->currentMediaOffset%188)));
			 
			 if ((mediaFile->currentMediaOffset < linearStartOffset) && (reReadOnlyOnce == false))
			 {
				currentPosition.index = iFramePosition.index+1;
				reReadOnlyOnce = true;
				BDBG_WRN(("\n Out of boundary, ignore this I frame and try next one \n"));
				goto NextIFrameRead;
			 }		 
             returnSeekOffset = (mediaFile->nexusFilePlay->file.data)->seek(mediaFile->nexusFilePlay->file.data,mediaFile->currentMediaOffset,SEEK_SET);
             if(returnSeekOffset != mediaFile->currentMediaOffset)
             {
                BDBG_ERR(("\n Seek failed \n"));
                goto EndofMediaRead;
             }

             mediaFile->chunkInfo.elements++;
             mediaFile->chunkInfo.startTime = iFramePosition.timestamp;
             mediaFile->chunkInfo.startPos = returnSeekOffset/188;

             BDBG_MSG(("chunkbuffer = %d sizeOfFrameAdjuested=%x ",chunkbuffer,sizeOfFrame));   

			 if(((unsigned)returnSize+sizeOfFrame)> (unsigned)size)
			 {
				mediaFile->moreData = ((unsigned)returnSize+sizeOfFrame) - (unsigned)size;
				BDBG_MSG(("I frame can't fit buffer. sizeOfFrame:%u, size:%u, moreData:%u, %llx",
				  sizeOfFrame, size, mediaFile->moreData, mediaFile->currentNavOffset));
				sizeOfFrame = (unsigned)size;
			 }
			 else
			 {
				mediaFile->moreData = 0;
				sizeOfFrame = (sizeOfFrame)/188*188;
			 }

             readSize = (mediaFile->nexusFilePlay->file.data)->read(mediaFile->nexusFilePlay->file.data,chunkbuffer,sizeOfFrame);         
             BDBG_MSG(("read size = %x sizeOfFrameAdjuested=%x sybcbyte[0] =%x",readSize,sizeOfFrame,chunkbuffer[0]));
             if(readSize>0)
             {
                 if(mediaFile->settings.drmHandle)
                 {
                     BDBG_MSG(("In DRM Mode:chunkbuffer = %d Readsize=%d drmhandle=%lx",chunkbuffer,readSize,mediaFile->settings.drmHandle));   
                     mediaFile->decBufferInfo.streamBuf = (uint8_t *)chunkbuffer;
                     mediaFile->decBufferInfo.streamBufLen = readSize;
                     rc = B_DVR_DRMService_DecryptData(mediaFile->settings.drmHandle,&(mediaFile->decBufferInfo));
                     if(rc != B_DVR_SUCCESS)
                     {
                         returnSize = 0;
                         BDBG_ERR(("\n DRMService Decryption Job failed \n"));
                         goto EndofMediaRead;
                     }
                 }
             }
             else
             {
			     BDBG_ERR(("\n %s %d Read the bad size from segmented_posix_read %d \n",__FUNCTION__,__LINE__,returnSize));
				 goto EndofMediaRead;
             }
             
             returnSize += readSize;
             chunkbuffer += readSize;
             #if INSERT_SEQEND_HEADER
             if(readSize%188)
             {
                 returnSize += (188-readSize%188);
                 BKNI_Memset((void *)chunkbuffer,0,(188-readSize%188));
                 if((188-readSize%188)>=8)
                 {
                      BDBG_MSG(("leftsize= %d -->=8",(188-readSize%188)));
                      BKNI_Memcpy(chunkbuffer,SequenceEndPkt,8);
                      chunkbuffer += (188-readSize%188);
                  }else{
                      BDBG_MSG(("\n Need to build-up the end of packet\n"));
                  }
                  sizeOfFrame += (188-readSize%188);
                  readSize += (188-readSize%188);
              }
              #endif
              BDBG_MSG(("returnSize = %x sizeOfFrameAdjuested=%x readSize =%x chunkbuffer=%d",returnSize,sizeOfFrame,readSize,chunkbuffer));
              
              #if PTS_PACING		  
			  if(mediaFile->firstTimeFlagForIFrame == 0 && mediaFile->speedNumerator > 1)
              {
                  mediaFile->firstTimeFlagForIFrame = 1;
                  mediaFile->ptsOfFirstIFrame = iFramePosition.pts;
                  chunkbuffer = (uint8_t *)buffer;
                  rc = B_DVR_MediaFile_P_LookUp_DTS(mediaFile,chunkbuffer,readSize);
                  if(rc != B_DVR_SUCCESS)
                  {
                     BDBG_ERR(("Fisrt DTS lookup failed"));
                     goto EndofMediaRead;
                  }

                  chunkbuffer = (uint8_t *)buffer;
                  rc = B_DVR_MediaFile_P_LookUp_PCRBase(mediaFile,chunkbuffer,readSize);
                  if(rc != B_DVR_SUCCESS)
                  {
                     BDBG_ERR(("Fisrt DTS lookup failed"));
                     goto EndofMediaRead;
                  }
              }
              if(mediaFile->speedNumerator > 1)
              {
                  mediaFile->frameCount++;
                  mediaFile->ptsDlt = (unsigned long)(mediaFile->ptsGap * mediaFile->frameCount);
                  BDBG_MSG(("B_DVR_MediaFile_Read:: Before Pts-chg--mediaFile->ptsGap =%ld mediaFile->frameCount=%d mediaFile->ptsDlt=%ld",mediaFile->ptsGap,
                             mediaFile->frameCount,mediaFile->ptsDlt));
                  if(mediaFile->pcrGapCtlFlag == 1)
                  {
                      chunkbuffer = (uint8_t *)buffer;
                      rc = B_DVR_MediaFile_P_LookUp_PCRBase(mediaFile,chunkbuffer,readSize);
                      if(rc != B_DVR_SUCCESS)
                      {
                         BDBG_ERR(("Fisrt DTS lookup failed"));
                         goto EndofMediaRead;
                      }
                      mediaFile->pcrGap = (mediaFile->pcrGap)/mediaFile->speedNumerator;
                      mediaFile->pcrDlt = (mediaFile->pcrGap * mediaFile->frameCount);
                      BDBG_MSG(("B_DVR_MediaFile_Read:: Before Pts-chg--mediaFile->pcrGap =%lld mediaFile->frameCount=%d mediaFile->pcrDlt=%lld",mediaFile->pcrGap,
                                 mediaFile->frameCount,mediaFile->pcrDlt));
                  }
                  else
                  {
                      if(mediaFile->pcrGapCtlFlag == 2)
                         mediaFile->pcrDlt = (mediaFile->pcrGap * mediaFile->frameCount);
					  
					  BDBG_MSG(("B_DVR_MediaFile_Read:: Before Pts-chg--mediaFile->pcrGap =%lld mediaFile->frameCount=%d mediaFile->pcrDlt=%lld",mediaFile->pcrGap,
								 mediaFile->frameCount,mediaFile->pcrDlt));
                  }

                  chunkbuffer = (uint8_t *)buffer;
                  rc = B_DVR_MediaFile_P_Pts_Chg(mediaFile,chunkbuffer,readSize);
                  if(rc != B_DVR_SUCCESS)
                  {
                     BDBG_ERR(("PTS adjusted to be failure on PES or PCR"));
                     goto EndofMediaRead;
                  }			  
                  if( mediaFile->pcrGapCtlFlag == 0)
                     mediaFile->pcrGapCtlFlag = 1;
				  
              }
              else
                  BDBG_MSG(("Stream Speed rate Numerator is 1, and keep the original stream PTS in PES/PCR"));                   
              #endif
			  
              if(readSize!=(ssize_t)(sizeOfFrame) || !readSize)
              {
                  mediaFile->chunkInfo.remainingLength = (unsigned long)((ssize_t)(sizeOfFrame) - readSize);
                  BDBG_MSG(("End/Beginning of file readsize=%x returnSize=%x desiredReadSize = %x",readSize,returnSize,sizeOfFrame));
                  goto EndofMediaRead;
              }
              if(mediaFile->mediaNode->mediaAttributes & B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM)
              {
                  B_DVR_SegmentedFilePlayHandle segmentedFilePlay = (B_DVR_SegmentedFilePlayHandle)(mediaFile->nexusFilePlay);
                  B_DVR_SegmentedFileRecordHandle segmentedFileRecord = (B_DVR_SegmentedFileRecordHandle)(mediaFile->nexusFileRecord);
                  BNAV_Player_Position tempposition;
                  
                  if(direction == eB_DVR_FileReadDirectionForward)
                      mediaFile->currentIndex = iFramePosition.index +1;
                  else 
                  {   
                      if(iFramePosition.index == 0)
                           mediaFile->currentIndex = iFramePosition.index;
                      else
                           mediaFile->currentIndex = iFramePosition.index -1;
                  }
                  if(mediaFile->nexusFileRecord)
                  {
                      rc = BNAV_Player_GetPositionInformation(segmentedFileRecord->navPlayer,
                                                              mediaFile->currentIndex, &tempposition);
                  }
                  else
                  {
                      rc = BNAV_Player_GetPositionInformation(segmentedFilePlay->navPlayer,mediaFile->currentIndex, &tempposition);
                  }
                  if (rc != B_DVR_SUCCESS)
                  {
                      BDBG_MSG(("Fail on getting position information"));
                      goto EndofMediaRead;      
                  }
                  mediaFile->currentMediaOffset = (tempposition.offsetLo|(((off_t)tempposition.offsetHi)<<32));
                  if(mediaFile->nexusFileRecord)
                  {
                          mediaFile->currentNavOffset = tempposition.index*segmentedFileRecord->navEntrySize;
                  }
                  else
                  {
                      mediaFile->currentNavOffset = tempposition.index*segmentedFilePlay->navEntrySize;
                  }
                  
                  mediaFile->currentMediaTime = tempposition.timestamp;
                  if(direction == eB_DVR_FileReadDirectionForward)
                      mediaFile->chunkInfo.duration = (tempposition.timestamp - iFramePosition.timestamp);
                  else
                      mediaFile->chunkInfo.duration = (iFramePosition.timestamp - tempposition.timestamp);
              }
          }
          else
          {
              BDBG_MSG(("End/Beginning of file :: returnSize=%x desiredReadSize = %x",returnSize,sizeOfFrame));
              goto EndofMediaRead;
          }
	  }
	  else
	  {
		  if(mediaFile->moreData > (unsigned)size)
		  {
			 BDBG_MSG(("remaining I frame (%u) can't fit to buffer...", mediaFile->moreData));
			 mediaFile->moreData -= size;
			 if(mediaFile->moreData > (unsigned)size)
			 {
				mediaFile->moreData = 0;
				BDBG_WRN(("remaining I frame (%u) still can't fit to buffer, ignore it... ", mediaFile->moreData));
			 }
		  }
		  else
		  {
			 BDBG_MSG(("reading remaining I frame (%u) ", mediaFile->moreData));
			 size = (mediaFile->moreData+187)/188*188;
			 mediaFile->moreData = 0;
		  }
		  returnSize = (mediaFile->nexusFilePlay->file.data)->read(mediaFile->nexusFilePlay->file.data, buffer, size); 
		  if(returnSize>0)
		  {
			 if(mediaFile->settings.drmHandle) 
			 {	  
				 mediaFile->decBufferInfo.streamBuf =(uint8_t *)buffer;
				 mediaFile->decBufferInfo.streamBufLen = returnSize;
				 rc = B_DVR_DRMService_DecryptData(mediaFile->settings.drmHandle,&(mediaFile->decBufferInfo));
				 if(rc != B_DVR_SUCCESS)
				 {
					 returnSize = 0;
					 BDBG_ERR(("\n DRMService Decryption Job failed \n"));
				 }
			 }
		  }
		  else
		  {
			 BDBG_ERR(("\n %s %d Read the bad size from segmented_posix_read %d \n",__FUNCTION__,__LINE__,returnSize));
			 goto EndofMediaRead;
		  }
			  
		  if(returnSize!=size || !returnSize)
		  {
			  BDBG_WRN(("End/Beginning of file returnSize=%x size = %x",returnSize,size));
		  } 		
		  mediaFile->currentMediaOffset += returnSize;		  
	  }
    }
    else
    { 
        returnSize = (mediaFile->nexusFilePlay->file.data)->read(mediaFile->nexusFilePlay->file.data, buffer, size); 
        BDBG_MSG(("Done of read returnsize=%d syncByte=%x",returnSize,*((uint8_t *)buffer)));   
        if(returnSize>0)
        {
            if(mediaFile->settings.drmHandle) 
            {    
                mediaFile->decBufferInfo.streamBuf =(uint8_t *)buffer;
                mediaFile->decBufferInfo.streamBufLen = returnSize;
                rc = B_DVR_DRMService_DecryptData(mediaFile->settings.drmHandle,&(mediaFile->decBufferInfo));
                if(rc != B_DVR_SUCCESS)
                {
                    returnSize = 0;
                    BDBG_ERR(("\n DRMService Decryption Job failed \n"));
                }
            }
        }
        else
        {
			BDBG_ERR(("\n %s %d Read the bad size from segmented_posix_read %d \n",__FUNCTION__,__LINE__,returnSize));
			goto EndofMediaRead;
        }
        
        if(returnSize!=size || !returnSize)
        {
            BDBG_ERR(("End/Beginning of file returnSize=%x desiredReadSize = %x",returnSize,size));
        }			
        mediaFile->currentMediaOffset += returnSize;        
    }
EndofMediaRead:
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    BDBG_MSG(("B_DVR_MediaFile_Read mediaName %s returnSize %x >>>",mediaFile->mediaName,(unsigned)(returnSize)));
    return returnSize;
}

ssize_t B_DVR_MediaFile_Write(
    B_DVR_MediaFileHandle mediaFile,
    unsigned *buffer, ssize_t size)
{
    ssize_t writtenSize=0;
    B_DVR_ManagerHandle dvrManager = B_DVR_Manager_GetHandle();
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_Write >>>"));
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    writtenSize = mediaFile->nexusFileRecord->data->write(mediaFile->nexusFileRecord->data,buffer,size);
    if(writtenSize!=size)
    {
        BDBG_ERR(("Out of storage space"));
    }
    mediaFile->mediaNode->mediaLinearEndOffset+=writtenSize;
    B_DVR_List_UpdateMediaNode(dvrManager->dvrList,mediaFile->mediaNode,mediaFile->openSettings.volumeIndex,false);
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    BDBG_MSG(("B_DVR_MediaFile_Write >>>"));
    return writtenSize;
}

int B_DVR_MediaFile_Seek(
    B_DVR_MediaFileHandle mediaFile,
    B_DVR_MediaFileSeekSettings *seekSettings)
{

    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_FilePosition position;
    unsigned long seekTime=0;
    off_t returnSeekOffset=0;
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(seekSettings);
    BDBG_MSG(("B_DVR_MediaFile_Seek >>>>"));
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    BKNI_Memset(&position,0,sizeof(B_DVR_FilePosition));
    if(mediaFile->openMode!=eB_DVR_MediaFileOpenModeStreaming)
    {
        BDBG_ERR(("B_DVR_MediaFile_Seek not permitted for playback and record mode"));
        rc= B_DVR_INVALID_PARAMETER;
    }
    else
    {
        BKNI_Memcpy((void *)&mediaFile->settings.seekSetting,(void *)seekSettings, sizeof(*seekSettings));
        if(seekSettings->seekmode == eB_DVR_MediaFileSeekModeTimestampBased)
        { 
            BDBG_MSG(("seek time %u",seekSettings->mediaSeek.time));
            if((seekSettings->mediaSeek.time < mediaFile->mediaNode->mediaStartTime) ||
               (seekSettings->mediaSeek.time > mediaFile->mediaNode->mediaEndTime))
            {
                rc = B_DVR_INVALID_PARAMETER;
                goto error;
            }
            seekTime = seekSettings->mediaSeek.time;
        }
        else
        {
            BDBG_MSG(("seek offset %lld",seekSettings->mediaSeek.offset));
            if (seekSettings->mediaSeek.offset < mediaFile->mediaNode->mediaLinearStartOffset)
            {
               seekSettings->mediaSeek.offset = mediaFile->mediaNode->mediaLinearStartOffset;
               BDBG_MSG(("seek offset is less than LinerStartOffset, it's set LinerStartOffset"));
            }
            if(seekSettings->mediaSeek.offset > mediaFile->mediaNode->mediaLinearEndOffset)
            {
                seekSettings->mediaSeek.offset = mediaFile->mediaNode->mediaLinearEndOffset;
                BDBG_MSG(("seek offset is beyond LinerEndOffset, it's set LinerEndOffset"));
            }
            BDBG_MSG(("Before Get seektime from seekOffset off %lld",seekSettings->mediaSeek.offset));
            if(mediaFile->nexusFileRecord)
            {
                rc=B_DVR_SegmentedFileRecord_GetTimestamp(mediaFile->nexusFileRecord,
                                                          seekSettings->mediaSeek.offset,
                                                          &seekTime);
            }
            else
            {
                rc=B_DVR_SegmentedFilePlay_GetTimestamp(mediaFile->nexusFilePlay,
                                                        seekSettings->mediaSeek.offset,
                                                        &seekTime);
            }
            
            if(rc!=B_DVR_SUCCESS)
            {
               BDBG_ERR(("unable to get timeStamp for media offset %lld",seekSettings->mediaSeek.offset));
               rc = B_DVR_UNKNOWN;
               goto error;
            }
            BDBG_MSG(("After Get seektime from seekOffset time %u",seekTime));           
        }

        if(mediaFile->nexusFileRecord)
        {
            rc = B_DVR_SegmentedFileRecord_GetLocation(mediaFile->nexusFileRecord,-1,seekTime,&position);
        }
        else
        {
            rc = B_DVR_SegmentedFilePlay_GetLocation(mediaFile->nexusFilePlay,-1,seekTime,&position);
        }
        if(rc!=B_DVR_SUCCESS)
        {
            BDBG_ERR(("unable to get offset for time %u",seekTime));
            rc = B_DVR_INVALID_PARAMETER;
            goto error;
        }

        position.mpegFileOffset = B_DVR_IO_ALIGN_ROUND(position.mpegFileOffset);

        returnSeekOffset = mediaFile->nexusFilePlay->file.data->seek(mediaFile->nexusFilePlay->file.data,position.mpegFileOffset,SEEK_SET);
        if(returnSeekOffset!=position.mpegFileOffset)
        {
            BDBG_ERR(("Seek error"));
            rc = B_DVR_UNKNOWN;
            goto error;
        }
        else
        {
            uint8_t buf[188*2];
            unsigned syncIndex = 0;
            ssize_t  tempreadSize = 0;
            uint8_t *Drmbuf;
            if ( position.mpegFileOffset < (mediaFile->mediaNode->mediaLinearStartOffset +188))
            {
                position.mpegFileOffset = mediaFile->mediaNode->mediaLinearStartOffset;
            }
            else
            {
               position.mpegFileOffset -= 188;
            }
            returnSeekOffset = mediaFile->nexusFilePlay->file.data->seek(mediaFile->nexusFilePlay->file.data,position.mpegFileOffset,SEEK_SET);
            if(returnSeekOffset!=position.mpegFileOffset)
            {
                BDBG_ERR(("Seek error"));
                rc = B_DVR_UNKNOWN;
                goto error;
            }
            BDBG_MSG(("Medaifile current position = %lld",position.mpegFileOffset));
            if(mediaFile->settings.drmHandle)
            {
                NEXUS_Memory_Allocate(188*2+4, NULL, (void *)&Drmbuf);
                tempreadSize = mediaFile->nexusFilePlay->file.data->read(mediaFile->nexusFilePlay->file.data,Drmbuf,188*2);
            }else
                tempreadSize = mediaFile->nexusFilePlay->file.data->read(mediaFile->nexusFilePlay->file.data,buf,188*2);

            if(mediaFile->settings.drmHandle)
            {
               syncIndex = (position.mpegFileOffset%188)?(188-(position.mpegFileOffset%188)):0;
               NEXUS_Memory_Free(Drmbuf);
            }
            else
            {         
                BDBG_MSG(("In Non-DRM Mode in Seek Module:buffer = %llx ",buf));    
                for(syncIndex=0;syncIndex <188*2;)
                {
                    if(buf[syncIndex]==0x47)
                    {
                       BDBG_ERR(("sync byte %x",buf[syncIndex]));
                       break;
                    }
                    syncIndex++;
                }
            }
            BDBG_MSG(("Before seek position %lld syncindex= %ld",position.mpegFileOffset,syncIndex));
            position.mpegFileOffset += syncIndex;
            BDBG_MSG(("Real seek position %lld",position.mpegFileOffset ));

            if(mediaFile->nexusFileRecord)
            {
                rc=B_DVR_SegmentedFileRecord_GetTimestamp(mediaFile->nexusFileRecord,
                                                        position.mpegFileOffset,
                                                        &seekTime);
            }
            else
            {
                rc=B_DVR_SegmentedFilePlay_GetTimestamp(mediaFile->nexusFilePlay,
                                                        position.mpegFileOffset,
                                                        &seekTime);
            }

            if(rc!=B_DVR_SUCCESS)
            {
               BDBG_ERR(("unable to get timeStamp for media offset %lld",seekSettings->mediaSeek.offset));
               rc = B_DVR_UNKNOWN;
               goto error;
            }

            returnSeekOffset = mediaFile->nexusFilePlay->file.data->seek(mediaFile->nexusFilePlay->file.data,position.mpegFileOffset,SEEK_SET);
            if(returnSeekOffset!=position.mpegFileOffset)
            {
                BDBG_ERR(("Seek error"));
                rc = B_DVR_UNKNOWN;
                goto error;
            }

            if(mediaFile->nexusFileRecord)
            {
                rc = B_DVR_SegmentedFileRecord_GetLocation(mediaFile->nexusFileRecord,-1,seekTime,&position);
            }
            else
            {
                rc = B_DVR_SegmentedFilePlay_GetLocation(mediaFile->nexusFilePlay,-1,seekTime,&position);
            }
            if(rc!=B_DVR_SUCCESS)
            {
               BDBG_ERR(("unable to get offset for time %u",seekTime));
               rc = B_DVR_INVALID_PARAMETER;
               goto error;
            }

            mediaFile->currentMediaOffset = returnSeekOffset;
            mediaFile->currentMediaTime = position.timestamp;
            mediaFile->currentNavOffset = position.navFileOffset;
            mediaFile->currentIndex = position.index;	
            mediaFile->moreData = 0;
            #if PTS_PACING
            B_DVR_MediaFile_P_ResetVar(mediaFile);
            #endif
        }
    }
error:    
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    BDBG_MSG(("B_DVR_MediaFile_Seek seekOffset %lld currentindex = %lx<<<<",returnSeekOffset,mediaFile->currentIndex));
    return rc;
}

B_DVR_ERROR B_DVR_MediaFile_Stats(
        B_DVR_MediaFileHandle mediaFile,
        B_DVR_MediaFileStats *stats)

{
    B_DVR_ERROR rc =B_DVR_SUCCESS;
    B_DVR_PlaybackServiceStatus playbackServiceStatus;  
    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(stats);
    BDBG_MSG(("B_DVR_MediaFile_Stats >>>"));
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    switch(mediaFile->openMode)
    {
    case eB_DVR_MediaFileOpenModeStreaming:
        {
            BDBG_MSG(("eB_DVR_MediaFileOpenModeStreaming"));
            stats->startOffset = mediaFile->mediaNode->mediaLinearStartOffset;
            stats->endOffset = mediaFile->mediaNode->mediaLinearEndOffset;
            stats->currentOffset = mediaFile->currentMediaOffset;
            stats->startTime = mediaFile->mediaNode->mediaStartTime;
            stats->endTime = mediaFile->mediaNode->mediaEndTime;
            stats->currentTime = mediaFile->currentMediaTime;
            stats->moreData = mediaFile->moreData;
        }
        break;
    case eB_DVR_MediaFileOpenModePlayback:
        {
            BDBG_MSG(("eB_DVR_MediaFileOpenModePlayback"));
            rc = B_DVR_PlaybackService_GetStatus(mediaFile->playbackService,&playbackServiceStatus);
            if(rc!=B_DVR_SUCCESS)
            {
                BDBG_ERR(("error in getting playbackServiceStatus"));
                rc = B_DVR_UNKNOWN;
                goto error;
            }
            stats->startOffset = playbackServiceStatus.startTime;
            stats->endTime = playbackServiceStatus.endTime;
            stats->currentTime = playbackServiceStatus.currentTime;
            stats->startOffset = playbackServiceStatus.linearStartOffset;
            stats->endOffset = playbackServiceStatus.linearEndOffset;
            stats->currentOffset = playbackServiceStatus.linearCurrentOffset;
        }
        break;
    case eB_DVR_MediaFileOpenModeRecord:
        {
            BDBG_MSG(("eB_DVR_MediaFileOpenModeRecord"));
            stats->startOffset = mediaFile->mediaNode->mediaLinearStartOffset;
            stats->endOffset = mediaFile->mediaNode->mediaLinearEndOffset;
            stats->currentOffset = mediaFile->currentMediaOffset;
            stats->startTime = mediaFile->mediaNode->mediaStartTime;
            stats->endTime = mediaFile->mediaNode->mediaEndTime;
            stats->currentTime = mediaFile->currentMediaTime;
        }
        break;
    default:
        BDBG_ERR(("invalid openMode"));
        rc = B_DVR_INVALID_PARAMETER;
    }
    
error:     
    BDBG_MSG(("media time: start %u curr %u end %u",stats->startTime,stats->currentTime,stats->endTime));
    BDBG_MSG(("media offset: start %lld curr %lld end %lld",stats->startOffset,stats->currentOffset,stats->endOffset));
    BDBG_MSG(("B_DVR_MediaFile_Stats <<<"));
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    return rc;
}

B_DVR_ERROR B_DVR_MediaFile_Create(
    B_DVR_MediaFileMetaDataInfo *metaDataInfo)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;

    BSTD_UNUSED(metaDataInfo);
    return rc;

}

B_DVR_ERROR B_DVR_MediaFile_PlayStart(
    B_DVR_MediaFileHandle mediaFile, 
    B_DVR_MediaFilePlaySettings *playSettings)

{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    B_DVR_PlaybackServiceSettings playbackServiceSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_StcChannelSettings stcSettings;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_ASSERT(playSettings);
    BDBG_ASSERT(mediaFile->playbackService);
    BDBG_MSG(("B_DVR_MediaFile_PlayStart >>>>"));
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    if(mediaFile->openMode != eB_DVR_MediaFileOpenModePlayback)
    {
        BDBG_ERR(("B_DVR_MediaFile_PlayStart not supported for streaming and recording mode"));
        rc = B_DVR_INVALID_PARAMETER;
    }
    else
    {
        B_DVR_PlaybackService_GetSettings(mediaFile->playbackService,&playbackServiceSettings);
        mediaFile->playback = playbackServiceSettings.playback;
        playbackServiceSettings.videoDecoder[0]= mediaFile->openSettings.videoDecoder[0];
        playbackServiceSettings.videoDecoder[1]=NULL;
        playbackServiceSettings.audioDecoder[0]=mediaFile->openSettings.audioDecoder[0];
        playbackServiceSettings.audioDecoder[1]=NULL;
        playbackServiceSettings.stcChannel = mediaFile->openSettings.stcChannel;
        B_DVR_PlaybackService_SetSettings(mediaFile->playbackService,&playbackServiceSettings);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = mediaFile->mediaNode->esStreamInfo[0].codec.videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = mediaFile->openSettings.videoDecoder[0];
        mediaFile->videoPlaybackPidChannels =
             NEXUS_Playback_OpenPidChannel(mediaFile->playback,mediaFile->mediaNode->esStreamInfo[0].pid,&playbackPidSettings);

        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        if(mediaFile->openSettings.audioDecoder[0] != NULL)
        {
           playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
           playbackPidSettings.pidTypeSettings.audio.primary = mediaFile->openSettings.audioDecoder[0];
        }
        else
        {
           playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
           playbackPidSettings.pidTypeSettings.audio.primary = mediaFile->openSettings.audioDecoder[0];         
        }
        mediaFile->audioPlaybackPidChannels = 
            NEXUS_Playback_OpenPidChannel(mediaFile->playback,mediaFile->mediaNode->esStreamInfo[1].pid,&playbackPidSettings);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&(mediaFile->videoProgram));
        mediaFile->videoProgram.codec = mediaFile->mediaNode->esStreamInfo[0].codec.videoCodec;
        mediaFile->videoProgram.pidChannel = mediaFile->videoPlaybackPidChannels;
        mediaFile->videoProgram.stcChannel = mediaFile->openSettings.stcChannel;
        NEXUS_StcChannel_GetSettings(mediaFile->openSettings.stcChannel,&stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto; 
        NEXUS_StcChannel_SetSettings(mediaFile->openSettings.stcChannel,&stcSettings);

        if(mediaFile->settings.drmHandle)
        {
            B_DVR_PlaybackService_AddDrmSettings(mediaFile->playbackService,mediaFile->settings.drmHandle);
            BDBG_MSG(("B_DVR_MediaFile_PlayStart:: In DRM Enabled mode"));
        }
        NEXUS_VideoDecoder_Start(mediaFile->openSettings.videoDecoder[0], &(mediaFile->videoProgram));

        NEXUS_AudioDecoder_GetDefaultStartSettings(&(mediaFile->audioProgram));
        mediaFile->audioProgram.codec = mediaFile->mediaNode->esStreamInfo[1].codec.audioCodec;
        mediaFile->audioProgram.pidChannel = mediaFile->audioPlaybackPidChannels;
        mediaFile->audioProgram.stcChannel = mediaFile->openSettings.stcChannel;
        if(mediaFile->openSettings.audioDecoder[0] != NULL)
           NEXUS_AudioDecoder_Start(mediaFile->openSettings.audioDecoder[0],&(mediaFile->audioProgram));        
        B_DVR_PlaybackService_Start(mediaFile->playbackService);
        BDBG_MSG(("B_DVR_MediaFile_PlayStart---Afterservice_start"));
    }

    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    BDBG_MSG(("B_DVR_MediaFile_PlayStart <<<"));
    return rc;
}


B_DVR_ERROR B_DVR_MediaFile_PlayStop(
    B_DVR_MediaFileHandle mediaFile)
{
    B_DVR_ERROR rc=B_DVR_SUCCESS;
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
    BDBG_MSG(("B_DVR_MediaFile_PlayStop >>>>"));
    B_Mutex_Lock(mediaFile->mediaFileMutex);
    if(mediaFile->openMode != eB_DVR_MediaFileOpenModePlayback)
    {
        BDBG_ERR(("B_DVR_MediaFile_PlayStop not supported for streaming and recording"));
        rc = B_DVR_INVALID_PARAMETER;
    }
    else
    {
        B_DVR_PlaybackService_Stop(mediaFile->playbackService);
        if(mediaFile->settings.drmHandle)
        {
            BDBG_MSG(("B_DVR_MediaFile_PlayStop:: In DRM Enabled mode"));
            B_DVR_PlaybackService_RemoveDrmSettings(mediaFile->playbackService);
        }
        else
            BDBG_MSG(("B_DVR_MediaFile_PlayStop:: In Non-DRM mode"));

        NEXUS_VideoDecoder_GetSettings(mediaFile->openSettings.videoDecoder[0],&videoDecoderSettings);
        videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
        NEXUS_VideoDecoder_SetSettings(mediaFile->openSettings.videoDecoder[0],&videoDecoderSettings);
        if(mediaFile->openSettings.audioDecoder[0] != NULL)
           NEXUS_AudioDecoder_Stop(mediaFile->openSettings.audioDecoder[0]);
        NEXUS_VideoDecoder_Stop(mediaFile->openSettings.videoDecoder[0]);
        NEXUS_Playback_ClosePidChannel(mediaFile->playback,mediaFile->audioPlaybackPidChannels);
        NEXUS_Playback_ClosePidChannel(mediaFile->playback,mediaFile->videoPlaybackPidChannels);
    }
    B_Mutex_Unlock(mediaFile->mediaFileMutex);
    BDBG_MSG(("B_DVR_MediaFile_PlayStop <<<<"));
    return rc;
}


B_DVR_ERROR B_DVR_MediaFile_Delete(
    B_DVR_MediaNodeSettings *mediaNodeSettings)
{

    B_DVR_ERROR rc=B_DVR_SUCCESS;
    BSTD_UNUSED(mediaNodeSettings);
    return rc;

}

B_DVR_MediaNode B_DVR_MediaFile_GetMediaNode(
    B_DVR_MediaFileHandle mediaFile
    )
{
    B_DVR_MediaNode mediaNode=NULL; 
    if(mediaFile) 
    {
        BDBG_OBJECT_ASSERT(mediaFile,B_DVR_MediaFile);
        B_Mutex_Lock(mediaFile->mediaFileMutex);
        mediaNode = mediaFile->mediaNode;    
        B_Mutex_Unlock(mediaFile->mediaFileMutex);
    }
    return mediaNode;
}

