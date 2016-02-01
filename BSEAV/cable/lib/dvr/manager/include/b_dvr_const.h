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
 * DVR extensions' macros for containing various hard coded values.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _B_DVR_CONST_H
#define _B_DVR_CONST_H

#define B_DVR_MAX_KEYS 16
#define B_DVR_MAX_PIDS 16
#define B_DVR_MAX_FILE_NAME_LENGTH        256
#define B_DVR_MEDIA_NODE_FILE_EXTENTION ".info"
#define B_DVR_MEDIA_NODE_FILE_SEARCH  "*.info"
#define B_DVR_MEDIA_FILE_EXTENTION ".ts"
#define B_DVR_NAVIGATION_FILE_EXTENTION ".nav"
#define MAX_PROGRAMS 16
#define B_DVR_SUCCESS              0
#define B_DVR_NOT_INITIALIZED      1
#define B_DVR_INVALID_PARAMETER    2
#define B_DVR_OUT_OF_SYSTEM_MEMORY 3
#define B_DVR_OUT_OF_DEVICE_MEMORY 4
#define B_DVR_TIMEOUT              5
#define B_DVR_OS_ERROR             6
#define B_DVR_LEAKED_RESOURCE      7
#define B_DVR_NOT_SUPPORTED        8
#define B_DVR_OUT_OF_RECORD_CHANNELS  9
#define B_DVR_OUT_OF_PLAYBACK_CHANNELS 10
#define B_DVR_UNKNOWN              13
#define B_DVR_FILE_NOT_FOUND   14
#define B_DVR_FILE_IO_ERROR        15
#define B_DVR_SYSTEM_ERR           20
#define B_DVR_MEDIASEG_FULL        21
#define B_DVR_NAVSEG_FULL          22
#define B_DVR_VOLUME_LIMITED       23
#define B_DVR_FAIL_TO_INVALIDATE_KEY    24
#define B_DVR_FAIL_TO_ADD_KEYSLOT   25
#define B_DVR_FAIL_TO_REMOVE_KEYSLOT    26
#define B_DVR_FAIL_TO_LOAD_CLEARKEY     27
#define B_DVR_FAIL_TO_GENERATE_SESSION_KEY  28
#define B_DVR_FAIL_TO_GENERATE_CONTROLWORD  29
#define B_DVR_FAIL_TO_LOAD_IV               30
#define B_DVR_DMAJOB_SUCCESS                    31
#define B_DVR_DMAJOB_FAIL                       32
#define B_DVR_DMAJOB_IN_QUEUE               33
#define B_DVR_INVALID_INDEX        34
#define B_DVR_NOT_REGISTERED       35
#define B_DVR_ALREADY_MOUNTED      36
#define B_DVR_NOT_FORMATTED        37
#define B_DVR_ALREADY_REGISTERED   38
#define B_DVR_REC_IN_USE       39


#define B_DVR_MAX_KEYSLOTS_PER_PID    2
#define B_DVR_MAX_KEYSLOTS_PER_SERVICE 4

#define B_DVR_MEDIASTORAGE_MAX_VOLUME   4
#define B_DVR_MAX_KEY_LENGTH            256      
#define B_DVR_DEFAULT_KEY_LENGTH        16      /*  key size 16 bytes (128 bits)  */

#define B_DVR_MAX_RECORDING 16
#define B_DVR_MAX_MEDIAFILE  16
#define B_DVR_MAX_PLAYBACK  16
#define B_DVR_MAX_TRANSCODE 2
#define B_DVR_MAX_TSB       16
#define B_DVR_MAX_DRM       44
#define B_DVR_MAX_DATAINJECTION       8
#define B_DVR_FILE_INVALID_POSITION ((unsigned long)-1)
#define B_DVR_INVALID_SEGMENT ((unsigned)(-1))
#define B_DVR_SEGMENTED_RECORD_POLL_TIME 30
#define B_DVR_MEDIA_SEGMENT_SIZE (188*4096*200)
/*Comments for calculation of worst case nav size for 150MB 
  need to be added */
#define B_DVR_NAV_SEGMENT_SIZE (((B_DVR_MEDIA_SEGMENT_SIZE/10000)*76)-((B_DVR_MEDIA_SEGMENT_SIZE/10000)*76)%64)
#define B_DVR_MEDIANODE_RESERVED_DATA_SIZE 8192
#define B_DVR_DATAINJECTION_DUMMY_PID 0x1ffe
#define IN_PROGRESS_RECORDING_UPDATE_TIMEOUT 30000 /* 30 sec -> units in ms*/
#define B_DVR_MEDIANODE_UPDATE_INTERVAL 1000 /* units in ms*/
#define B_DVR_RECORD_POLLING_INTERVAL 250 /* units in ms*/
#define B_DVR_MEDIA_PROBE_CHUNK_SIZE 4096*188
#define B_DVR_MEDIA_PROBE_TIMEOUT   30000 /* 30 sec timeout if none of ts packet comes */
#define B_DVR_LOW_BITRATE_STREAM    2*1024*1024  /* 2 Mpbs */
#define B_DVR_PRINT_REC_STATUS_INTERVAL 10*60*1000 /*10 mins*/
#define B_DVR_MEDIA_ATTRIBUTE_SEGMENTED_STREAM       (0x00000001)
#define B_DVR_MEDIA_ATTRIBUTE_ENCRYPTED_STREAM       (0x00000002)
#define B_DVR_MEDIA_ATTRIBUTE_HD_STREAM              (0x00000004)
#define B_DVR_MEDIA_ATTRIBUTE_AUDIO_ONLY_STREAM      (0x00000008)
#define B_DVR_MEDIA_ATTRIBUTE_HITS_STREAM            (0x00000010)
#define B_DVR_MEDIA_ATTRIBUTE_RECORDING_IN_PROGRESS  (0x00000020)
#define B_DVR_MEDIA_ATTRIBUTE_RECORDING_ABORTED      (0x00000400)
#define B_DVR_MEDIA_ATTRIBUTE_RECORDING_PROGRESSIVE  (0x00000800)
#define B_DVR_MEDIA_ATTRIBUTE_LOW_DELAY_STREAM       (0x00001000)
#define B_DVR_MEDIA_ATTRIBUTE_TRANSCODED_STREAM      (0x00002000)
#define B_DVR_MEDIA_ATTRIBUTE_REC_PAUSED             (0x00004000)

#define B_DVR_MEDIANODE_VERSION                      (10) //1.0

#endif /*_B_DVR_CONST_H */


