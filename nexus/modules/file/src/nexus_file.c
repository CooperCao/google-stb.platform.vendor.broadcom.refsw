/***************************************************************************
 * Copyright (C) 2010-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 **************************************************************************/
#include "nexus_file_module.h"
#include "bcmplayer.h"
#include "bfile_util.h"
#include "nexus_file_muxio.h"
#include "nexus_file_posix.h"

BDBG_MODULE(nexus_file);

typedef struct b_file_bp_info {
    unsigned index_entrysize;
    NEXUS_FilePlayHandle play_file;
} b_file_bp_info;

static long
nexus_file_bp_read(void *buffer, long size, long count, void *fp )
{
    bfile_io_read_t f = ((b_file_bp_info *)fp)->play_file->file.index;

    return (long)f->read(f, buffer, (size_t)(count*size));
}

static long
nexus_file_bp_tell( void *fp )
{
    bfile_io_read_t f = ((b_file_bp_info *)fp)->play_file->file.index;
    return (long)f->seek(f, 0, SEEK_CUR);
}

static int
nexus_file_bp_seek( void *fp, long offset, int origin )
{
    bfile_io_read_t f = ((b_file_bp_info *)fp)->play_file->file.index;
    off_t rc;
    rc = f->seek(f, (off_t)offset, origin);
    if ( rc == (off_t)-1) {
        return -1;
    }
    return 0;
}

static int
nexus_file_bp_bounds(BNAV_Player_Handle handle, void *fp, long *firstIndex, long *lastIndex)
{
    bfile_io_read_t f = ((b_file_bp_info *)fp)->play_file->file.index;
    off_t first, last;
    unsigned index_entrysize =((b_file_bp_info *)fp)->index_entrysize;

    f->bounds(f, &first, &last);

    BSTD_UNUSED(handle);
    *firstIndex = first/index_entrysize;
    *lastIndex = (last-1)/index_entrysize;

    return 0;
}

static void
NEXUS_P_FilePlay_ConvertPosition(unsigned index_entrysize, const BNAV_Player_Position *nav_position, NEXUS_FilePosition *position)
{
   position->indexOffset = nav_position->index * index_entrysize;
   position->timestamp = nav_position->timestamp;
   position->mpegFileOffset = ((nav_position)->offsetLo|(((uint64_t)(nav_position)->offsetHi)<<32));
   return;
}

int
NEXUS_P_ReadFirstNavEntry(bfile_io_read_t f, BNAV_Entry *pNavEntry)
{
    off_t rc;

    rc = f->seek(f, 0, SEEK_SET);
    if ( rc == (off_t)-1) {
        return -1;
    }

    rc = f->read(f, pNavEntry, sizeof(*pNavEntry));
    if (rc != sizeof(*pNavEntry)) {
        return BERR_TRACE(-1);
    }
    
    return 0;
}

static NEXUS_Error
NEXUS_P_FilePlay_OpenBcmPlayer(BNAV_Player_Handle *bcm_player, b_file_bp_info *bp_info, NEXUS_FilePlayHandle file)
{
    NEXUS_Error rc;
    BNAV_Player_Settings cfg;
    BNAV_Entry navEntry;
    static const BNAV_DecoderFeatures features = {1,1,1,1};

    if(file->file.index==NULL) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_args; }

    BNAV_Player_GetDefaultSettings(&cfg);
    rc = NEXUS_P_ReadFirstNavEntry(file->file.index, &navEntry);
    if (!rc) {
        cfg.navVersion = BNAV_get_version(&navEntry);
    }
    bp_info->index_entrysize = BNAV_GetEntrySize(cfg.navVersion);
    bp_info->play_file = file;
    cfg.videoPid = 0x1FFF; /* since BNAV_Player doesn't like 0 */
    cfg.filePointer = bp_info;
    cfg.decoderFeatures = features;
    cfg.readCb = nexus_file_bp_read;
    cfg.tellCb = nexus_file_bp_tell;
    cfg.seekCb   = nexus_file_bp_seek;
    cfg.boundsCb = nexus_file_bp_bounds;
    cfg.transportTimestampEnabled = false;
    cfg.firstIndex = -1; /* fifo file may have firstIndex > 0 */
    if (BNAV_Player_Open(bcm_player, &cfg)!=0) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_player; }
    return NEXUS_SUCCESS;

err_player:
err_args:
    return rc;
}

NEXUS_Error NEXUS_FilePlay_Lock_priv( NEXUS_FilePlayHandle file )
{
    if (file->locked) return NEXUS_NOT_AVAILABLE;
    file->locked = true;
    return NEXUS_SUCCESS;
}

void NEXUS_FilePlay_Unlock_priv( NEXUS_FilePlayHandle file )
{
    if (file->locked) {
        file->locked = false;
    }
    else {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
}

NEXUS_Error
NEXUS_FilePlay_GetBounds(NEXUS_FilePlayHandle file, NEXUS_FilePosition *pFirst,  NEXUS_FilePosition *pLast)
{
    b_file_bp_info bp_info;
    int i_rc;
    NEXUS_Error rc;
    BNAV_Player_Handle bcm_player;
    long firstIndex, lastIndex;
    BNAV_Player_Position first, last;

    BDBG_ASSERT(file);
    BDBG_ASSERT(pFirst);
    BDBG_ASSERT(pLast);

    rc = NEXUS_FilePlay_Lock_priv(file);
    if (rc) {BERR_TRACE(rc); goto err_lock;}

    rc = NEXUS_P_FilePlay_OpenBcmPlayer(&bcm_player, &bp_info, file);
    if(rc!=NEXUS_SUCCESS) {goto err_player;}

    i_rc = nexus_file_bp_bounds(bcm_player, &bp_info, &firstIndex, &lastIndex);
    if(i_rc!=0) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_bounds;}
    i_rc = BNAV_Player_GetPositionInformation(bcm_player, firstIndex, &first);
    if(i_rc!=0) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_first;}
    i_rc = BNAV_Player_GetPositionInformation(bcm_player, lastIndex, &last);
    if(i_rc!=0) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_last;}
    NEXUS_P_FilePlay_ConvertPosition(bp_info.index_entrysize, &first, pFirst);
    NEXUS_P_FilePlay_ConvertPosition(bp_info.index_entrysize, &last, pLast);
    BNAV_Player_Close(bcm_player);
    NEXUS_FilePlay_Unlock_priv(file);
    return NEXUS_SUCCESS;

err_last:
err_first:
err_bounds:
    BNAV_Player_Close(bcm_player);
err_player:
    NEXUS_FilePlay_Unlock_priv(file);
err_lock:
    return rc;
}

NEXUS_Error
NEXUS_FilePlay_GetLocation( NEXUS_FilePlayHandle file, unsigned long timestamp, NEXUS_FilePosition *pPosition )
{
    b_file_bp_info bp_info;
    int i_rc;
    NEXUS_Error rc;
    BNAV_Player_Handle bcm_player;
    long i_frame_index;
    BNAV_Player_Position i_frame_position;
    long index;

    rc = NEXUS_FilePlay_Lock_priv(file);
    if (rc) {BERR_TRACE(rc); goto err_lock;}

    rc = NEXUS_P_FilePlay_OpenBcmPlayer(&bcm_player, &bp_info, file);
    if(rc!=NEXUS_SUCCESS) {goto err_player;}

    index = BNAV_Player_FindIndexFromTimestamp(bcm_player, timestamp);
    if(index==-1)  {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_find_index;}

    i_frame_index = BNAV_Player_FindIFrameFromIndex(bcm_player, index, eBpForward);
    if (i_frame_index == -1) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_find_iframe;}

    i_rc = BNAV_Player_GetPositionInformation(bcm_player, i_frame_index, &i_frame_position);
    if(i_rc!=0) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_position_iframe;}

    NEXUS_P_FilePlay_ConvertPosition(bp_info.index_entrysize, &i_frame_position, pPosition);
    BNAV_Player_Close(bcm_player);
    NEXUS_FilePlay_Unlock_priv(file);
    return NEXUS_SUCCESS;

err_position_iframe:
err_find_iframe:
err_find_index:
    BNAV_Player_Close(bcm_player);
err_player:
    NEXUS_FilePlay_Unlock_priv(file);
err_lock:
    return rc;
}

void
NEXUS_FilePlayOffset_GetDefaultSettings(NEXUS_FilePlayOffset_Settings *settings)
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->dataOffset = 0;
    settings->indexOffset = 0;
    return;
}

typedef struct NEXUS_FilePlayOffset {
    struct NEXUS_FilePlay  parent; /* must be first */
    NEXUS_FilePlayHandle  original;
} NEXUS_FilePlayOffset;

static void
NEXUS_P_FilePlayOffset_Close(struct NEXUS_FilePlay *file_)
{
    NEXUS_FilePlayOffset *file = (NEXUS_FilePlayOffset *)file_;
    bfile_read_offset_detach(file->parent.file.data);
    if(file->parent.file.index) {
        bfile_read_offset_detach(file->parent.file.index);
    }
    BKNI_Free(file);
    return;
}

NEXUS_FilePlayHandle
NEXUS_FilePlayOffset_Open( NEXUS_FilePlayHandle file, const NEXUS_FilePlayOffset_Settings *settings)
{
    NEXUS_FilePlayOffset *fileOffset;

    BDBG_ASSERT(file);
    BDBG_ASSERT(settings);
    BDBG_ASSERT(file->file.data);
    fileOffset = BKNI_Malloc(sizeof(*fileOffset));
    if(fileOffset==NULL) {(void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    fileOffset->parent.file.index = NULL;
    fileOffset->parent.file.data = bfile_read_offset_attach(file->file.data, -settings->dataOffset);
    if(fileOffset->parent.file.data==NULL) {(void)BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_data;}
    if(file->file.index) {
        fileOffset->parent.file.index = bfile_read_offset_attach(file->file.index, -settings->indexOffset);
        if(fileOffset->parent.file.index==NULL) {(void)BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_index;}
    }
    fileOffset->parent.file.close = NEXUS_P_FilePlayOffset_Close;
    fileOffset->original = file;
    return &fileOffset->parent;

err_index:
    bfile_read_offset_detach(file->file.data);
err_data:
    BKNI_Free(fileOffset);
err_alloc:
    return NULL;
}

void NEXUS_MuxFile_Close(NEXUS_MuxFileHandle file)
{
    file->close(&file->mux);
    return;
}

NEXUS_MuxFileHandle NEXUS_MuxFile_OpenPosix( const char *fileName)
{
    struct bfile_io_mux_posix *file;
    NEXUS_Error rc;

    file = BKNI_Malloc(sizeof(*file));
    if(!file) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}

    rc = b_file_io_mux_posix_open(file, fileName, 0, false); /* no DIRECT_IO */
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_open;}

    return &file->self;

err_open:
    BKNI_Free(file);
err_alloc:
    return NULL;
}

void NEXUS_FileRecord_GetDefaultOpenSettings(NEXUS_FileRecordOpenSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->data.directIo = true;
    return;
}

void NEXUS_FilePlay_GetDefaultOpenSettings(NEXUS_FilePlayOpenSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->data.directIo = true;
    return;
}


