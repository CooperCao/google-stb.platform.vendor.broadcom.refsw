/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Converts startcode index to bcmplayer index
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvlc.h"
#include "bcmindexer.h"
#include "bcmindexerpriv.h"
#include "mpeg2types.h"
#include "bcmplayer.h"
#include "bcmplayer_impl.h"
#include "bcmplayer_svc.h"

BDBG_MODULE(bcmplayer_svc);

#define BDBG_MSG_TRACE(x)  /* BDBG_MSG(x)  */
#define BDBG_MSG_READ(x)   /* BDBG_MSG(x) */
#define BNAV_SVC_TRACE(x)   /* BERR_TRACE(x)  */ x


int BNAV_Player_SVC_Reset(BNAV_Player_SVC_HandleImpl *player, BNAV_Player_Handle parent)
{
    unsigned i;
    BNAV_Player_State navState;

    BDBG_MSG_TRACE(("BNAV_Player_SVC_Reset: %#lx %#lx", (unsigned long)player, (unsigned long)parent));
    BKNI_Memset(player, 0, sizeof(*player));
    player->parent = parent;
    player->currentIndex = 0;
    player->after_seek = false;
    BDBG_CASSERT(BNAV_PLAYER_SVC_MAX_PIDS>=2);
    for(i=0;i<BNAV_PLAYER_SVC_MAX_PIDS;i++) {
        BNAV_Player_SVC_Pid *pid = player->pids+i;

        pid->pid = 0;
        pid->currentIndex = 0;
    }
    BNAV_Player_P_GetState(player->parent, &navState);
    player->pids[0].pid = navState.videoPid;
    player->pids[1].pid = navState.enhancementVideoPid;
    BDBG_MSG(("BNAV_Player_SVC_Reset: %#lx base:%#x ext:%#x", (unsigned long)player, player->pids[0].pid, player->pids[1].pid));
    return 0;
}

static int BNAV_Player_P_SVC_ReadEntry(BNAV_Player_SVC_HandleImpl *player, BNAV_SVC_Entry *entry, long index)
{
    BNAV_Entry *navEntry=NULL;
    int rc;

    if(index>=0) {
        navEntry = BNAV_Player_ReadEntry(player->parent, index);
    }
    if(navEntry == NULL) { 
        BDBG_MSG(("BNAV_Player_P_SVC_ReadEntry:%#lx: end of stream at %ld", (unsigned long)player, index));
        return -1;
    }

    rc = BNAV_Indexer_SVC_UnPackEntry(navEntry, entry);
    if(rc!=0) {return BNAV_SVC_TRACE(rc);}
    BDBG_MSG_READ(("ReadEntry:%#lx index:%ld pid:%#x type:%u  %u:%u", (unsigned long)player, index, entry->header.pid, entry->header.type, entry->common.offset, entry->common.payload_size));

    return 0;
}

static BNAV_Player_SVC_Pid *BNAV_Player_P_SVC_FindPid(BNAV_Player_SVC_HandleImpl *player, unsigned pid_no)
{
    unsigned i;
    for(i=0;i<BNAV_PLAYER_SVC_MAX_PIDS;i++) {
        BNAV_Player_SVC_Pid *pid = player->pids+i;

        if(pid->pid == pid_no) {
            return pid;
        } 
    }
    return NULL;
}

typedef struct BNAV_Player_SVC_PidState {
    long currentIndex;
    long nextIndex;
    bool forward;
} BNAV_Player_SVC_PidState;

static void BNAV_Player_P_SVC_PidState_Init(BNAV_Player_SVC_PidState *pidState, bool forward, long index)
{
    pidState->forward = forward;
    pidState->currentIndex = index;
    pidState->nextIndex = index;
    return;
}

static void BNAV_Player_P_SVC_PidState_Load(BNAV_Player_SVC_PidState *pidState, const BNAV_Player_SVC_Pid *pid, bool forward)
{
    pidState->forward = forward;
    pidState->currentIndex = pid->currentIndex;
    pidState->nextIndex = pid->currentIndex;
    return;
}

static void BNAV_Player_P_SVC_PidState_Save(const BNAV_Player_SVC_PidState *pidState, BNAV_Player_SVC_Pid *pid)
{
    pid->currentIndex = pidState->currentIndex;
    return;
}

static bool BNAV_Player_P_SVC_BaseOnly(const BNAV_Player_SVC_HandleImpl *player)
{
    return player->pids[1].pid == 0;
}

static int BNAV_Player_P_SVC_GetNextPidEntry(BNAV_Player_SVC_HandleImpl *player, BNAV_SVC_Entry *entry, const BNAV_Player_SVC_Pid *target_pid, BNAV_Player_SVC_PidState *pidState)
{
    long step = pidState->forward ? 1 : -1;
    long index = pidState->nextIndex;
    int rc = 0;

    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_GetNextPidEntry:%#lx %#x %ld %s", (unsigned long)player, target_pid->pid, pidState->nextIndex, pidState->forward?"forward":"backward"));
    for(;;) {
        BNAV_Player_SVC_Pid *pid;

        rc = BNAV_Player_P_SVC_ReadEntry(player, entry, index);
        if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}
        BDBG_MSG_TRACE(("BNAV_Player_P_SVC_GetNextPidEntry:%#lx %#x %ld:%#x:%u", (unsigned long)player, target_pid->pid, index, entry->header.pid, entry->header.type));
        pid = BNAV_Player_P_SVC_FindPid(player, entry->header.pid);
        /* if(pid==NULL) {return -BNAV_SVC_TRACE(BERR_NOT_SUPPORTED);} */
        if(pid==target_pid) {
            break; /* bingo */
        }
        index += step;
    }
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_GetNextPidEntry:%#lx %#x %ld->%ld %s", (unsigned long)player, target_pid->pid, pidState->nextIndex, index, pidState->forward?"forward":"backward"));
    pidState->nextIndex = index+step;
    pidState->currentIndex = index;
    return 0;
error:
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_GetNextPidEntry:%#lx %#x %ld->%ld %s ERROR", (unsigned long)player, target_pid->pid, pidState->nextIndex, index, pidState->forward?"forward":"backward"));
    pidState->nextIndex = index+step;
    return rc;
}

static int BNAV_Player_P_SVC_Find_IFrame(BNAV_Player_SVC_HandleImpl *player, BNAV_SVC_Entry *entry, const BNAV_Player_SVC_Pid *target_pid, BNAV_Player_SVC_PidState *pidState)
{
    int rc = 0;
    long startIndex = pidState->nextIndex;

    BSTD_UNUSED(startIndex);

    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_IFrame:%#lx %#x %ld %s", (unsigned long)player, target_pid->pid, pidState->nextIndex, pidState->forward?"forward":"backward"));
    for(;;) {
        rc = BNAV_Player_P_SVC_GetNextPidEntry(player, entry, target_pid, pidState);
        if(rc<0) { rc=BNAV_SVC_TRACE(rc);goto error;}
        if( (entry->header.type == BNAV_SVC_Entry_Type_eFrame || entry->header.type == BNAV_SVC_Entry_Type_eExtensionFrame)) {
            BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_IFrame:%#lx:%#x %ld pts:%u frame:%u", (unsigned long)player, target_pid->pid, pidState->currentIndex, entry->data.frame.pts, entry->data.frame.frame_type));
            if (entry->data.frame.frame_type == BNAV_SVC_Frame_Type_eI) {
                BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_IFrame:%#lx:%#x %ld found frame pts:%u frame:%u", (unsigned long)player, target_pid->pid, pidState->currentIndex, entry->data.frame.pts, entry->data.frame.frame_type));
                break; /* bingo */
            }
        }
    }
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_IFrame:%#lx %#x %ld->%ld %s", (unsigned long)player, target_pid->pid, startIndex, pidState->currentIndex, pidState->forward?"forward":"backward"));
    return 0;
error:
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_IFrame:%#lx %#x %ld->%ld %s ERROR", (unsigned long)player, target_pid->pid, startIndex, pidState->currentIndex, pidState->forward?"forward":"backward"));
    return rc;
}

static int BNAV_Player_SVC_P_TimeStampDiff(uint32_t ts1, uint32_t ts2)
{
    int32_t diff;
    diff = (int32_t)(ts1 - ts2);
    return diff;
}

static int BNAV_Player_P_SVC_Find_Timestamp(BNAV_Player_SVC_HandleImpl *player, BNAV_SVC_Entry *entry, const BNAV_Player_SVC_Pid *target_pid, BNAV_Player_SVC_PidState *pidState, uint32_t pts)
{
    int rc = 0;
    long startIndex = pidState->nextIndex;

    BSTD_UNUSED(startIndex);
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_Timestamp:%#lx %#x %ld %s", (unsigned long)player, target_pid->pid, pidState->nextIndex, pidState->forward?"forward":"backward"));
    for(;;) {
        rc = BNAV_Player_P_SVC_GetNextPidEntry(player, entry, target_pid, pidState);
        if(rc<0) { rc=BNAV_SVC_TRACE(rc);goto error;}
        if( (entry->header.type == BNAV_SVC_Entry_Type_eFrame || entry->header.type == BNAV_SVC_Entry_Type_eExtensionFrame)) {
            int diff = BNAV_Player_SVC_P_TimeStampDiff(entry->data.frame.pts, pts);
            BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_Timestamp:%#lx:%#x %ld timestamp:%u:%u(%d) frame:%u", (unsigned long)player, target_pid->pid, pidState->currentIndex, pts, entry->data.frame.pts, diff, entry->data.frame.frame_type));
            if(diff==0) {
                BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_Timestamp:%#lx:%#x %ld found timestamp:%u:%u(%d) frame:%u", (unsigned long)player, target_pid->pid, pidState->currentIndex, pts, entry->data.frame.pts, diff, entry->data.frame.frame_type));
                break; /* bingo */
            }
        }
    }
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_Timestamp:%#lx %#x %ld->%ld %s", (unsigned long)player, target_pid->pid, startIndex, pidState->currentIndex, pidState->forward?"forward":"backward"));
    return 0;
error:
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_Find_Timestamp:%#lx %#x %ld->%ld %s ERROR", (unsigned long)player, target_pid->pid, startIndex, pidState->currentIndex, pidState->forward?"forward":"backward"));
    return rc;
}


#define BIO_BLOCK_SIZE 4096

static int BNAV_Player_P_SVC_GetNextPlayEntry_Play(BNAV_Player_SVC_HandleImpl *player, const BNAV_Player_State *navState, BNAV_SVC_Entry *entry)
{
    long index = player->currentIndex;
    uint64_t nextOffset = player->currentOffset;
    int rc;
    uint64_t targetOffset = player->currentOffset + navState->normalPlayBufferSize;
    BNAV_Player_SVC_Pid *base_pid;

    base_pid = player->pids+0;

    targetOffset += (BIO_BLOCK_SIZE - 1);
    targetOffset -= targetOffset % BIO_BLOCK_SIZE; /* we need to stop alligned at IO block size */
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_GetNextPlayEntry_Play:%#lx offset %u -> %u", (unsigned long)player, (unsigned)player->currentOffset, (unsigned)targetOffset));

    for(;;) {
        BNAV_Player_SVC_Pid *pid;

        rc = BNAV_Player_P_SVC_ReadEntry(player, entry, index);
        if(rc!=0) {return BNAV_SVC_TRACE(rc);}

        pid = BNAV_Player_P_SVC_FindPid(player, entry->header.pid);
        /* if(pid==NULL) {return -BNAV_SVC_TRACE(BERR_NOT_SUPPORTED);} */

        if(pid==base_pid) {
            nextOffset = entry->common.offset + entry->common.payload_size;
            BDBG_MSG_TRACE(("play:%#lx %u pid:%#x type:%u %u:%u currentOffset:%u nextOffset:%u size:%u pts:%u", (unsigned long)player, index, pid->pid, entry->header.type, (unsigned)entry->common.offset, entry->common.payload_size, (unsigned)player->currentOffset, (unsigned)nextOffset, (unsigned)(nextOffset - player->currentOffset), (entry->header.type == BNAV_SVC_Entry_Type_eFrame || entry->header.type == BNAV_SVC_Entry_Type_eExtensionFrame) ? entry->data.frame.pts:0 ));

            if( entry->header.type == BNAV_SVC_Entry_Type_eFrame) {
                BNAV_Player_P_AddToPTSCache(player->parent, entry->data.frame.pts, index);
            }
            if(nextOffset >= targetOffset) {
                break;
            }
        }
        index++;
        if(pid!=NULL) {
            pid->currentIndex = index;
        }
    }
    player->currentIndex = index;
    BDBG_MSG(("play:%p data %lu:%u", (void *)player, (unsigned long)player->currentOffset, (unsigned)(targetOffset - player->currentOffset)));
    rc = BNAV_Player_AddSimpleData(player->parent, player->currentOffset, targetOffset - player->currentOffset);
    if(rc!=0) {return BNAV_SVC_TRACE(rc);}
    player->currentOffset = targetOffset;
    return 0;
}

static int BNAV_Player_P_SVC_PopulateFifo_Frame(BNAV_Player_SVC_HandleImpl *player, const BNAV_SVC_Entry *entry, const BNAV_Player_SVC_Pid *target_pid, long index)
{
    int rc=0;

    rc = BNAV_Player_AddData(player->parent, entry->common.offset, entry->common.payload_size, entry->data.frame.pts, 0 /* btp */, 0 /* display */, 0 /* skip */, target_pid->pid);
    if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}
    if(entry->header.type == BNAV_SVC_Entry_Type_eFrame) {
        BNAV_Player_P_AddToPTSCache(player->parent, entry->data.frame.pts, index);
        rc = BNAV_Player_AddStartCode(player->parent, 0x0A); /* Add end of sequence */
        if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}
    }

    BDBG_MSG(("BNAV_Player_P_SVC_PopulateFifo_Frame:%p %#x DONE", (void *)player, target_pid->pid));
    return 0;

error:
    BDBG_MSG(("BNAV_Player_P_SVC_PopulateFifo_Frame:%p %#x ERROR", (void *)player, target_pid->pid));
    return rc;
}

static int BNAV_Player_P_SVC_PopulateFifo_Trick(BNAV_Player_SVC_HandleImpl *player, BNAV_SVC_Entry *entry, const BNAV_Player_State *navState)
{
    BNAV_Player_SVC_Pid *base_pid;
    BNAV_Player_SVC_Pid *ext_pid;
    BNAV_Player_SVC_PidState base_state;
    BNAV_Player_SVC_PidState ext_state;
    long base_target;
    long ext_target;
    int rc;
    bool forward;
    bool baseOnly = BNAV_Player_P_SVC_BaseOnly(player);
    BNAV_SVC_Entry ext_entry;


    if(navState->playDir==eBpForward) {
        forward = true;
    } else if(navState->playDir==eBpReverse) {
        forward = false;
    } else {
        return -BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    base_pid = player->pids+0;
    ext_pid = player->pids+1;

    BNAV_Player_P_SVC_PidState_Load(&base_state, base_pid, forward );
    BNAV_Player_P_SVC_PidState_Load(&ext_state, ext_pid, forward );

    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_PopulateFifo_Trick:%#lx start index %ld,%ld %s", (unsigned long)player, base_state.nextIndex, ext_state.nextIndex, forward?"forward":"backward"));

    rc = BNAV_Player_P_SVC_Find_IFrame(player, entry, base_pid, &base_state);
    if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_PopulateFifo_Trick:%#lx found I frame at %ld:%u", (unsigned long)player, base_state.currentIndex, (unsigned)entry->data.frame.pts));

    if( !(entry->header.type == BNAV_SVC_Entry_Type_eFrame || entry->header.type == BNAV_SVC_Entry_Type_eExtensionFrame)) {
        rc = BNAV_SVC_TRACE(-1); goto error;
    }

    if(!baseOnly) {
        rc = BNAV_Player_P_SVC_Find_Timestamp(player, &ext_entry, ext_pid, &ext_state, entry->data.frame.pts);
        if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}
        BDBG_MSG_TRACE(("BNAV_Player_P_SVC_PopulateFifo_Trick:%#lx found timestamp at %ld", (unsigned long)player, ext_state.currentIndex));
    }

    base_target = base_state.currentIndex;
    ext_target = ext_state.currentIndex;
    BSTD_UNUSED(ext_target);
    BDBG_MSG(("BNAV_Player_P_SVC_PopulateFifo_Trick:%p base %u ext %u", (void *)player,  (unsigned)base_state.currentIndex, (unsigned)ext_state.currentIndex));

    player->currentIndex = base_target;
    rc = BNAV_Player_P_SVC_PopulateFifo_Frame(player, entry, base_pid, base_state.currentIndex);
    if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}

    if(!baseOnly) {
        rc = BNAV_Player_P_SVC_PopulateFifo_Frame(player, &ext_entry, ext_pid, ext_state.currentIndex);
        if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}
    }

    /* save new position for the next round */
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_PopulateFifo_Trick:%#lx next index %ld,%ld", (unsigned long)player, base_state.nextIndex, ext_state.nextIndex));
    base_state.currentIndex = base_state.nextIndex;
    ext_state.currentIndex = ext_state.nextIndex;
    BNAV_Player_P_SVC_PidState_Save(&base_state, base_pid);
    BNAV_Player_P_SVC_PidState_Save(&ext_state, ext_pid);
        
    return 0;

error:
    return rc;
}

static void BNAV_Player_P_SVC_AdjustSeek(BNAV_Player_SVC_HandleImpl *player, BNAV_SVC_Entry *entry, const BNAV_Player_State *navState)
{
    int rc;
    BNAV_Player_SVC_Pid *base_pid;
    BNAV_Player_SVC_Pid *ext_pid;
    BNAV_Player_SVC_PidState base_state;
    bool baseOnly = BNAV_Player_P_SVC_BaseOnly(player);

    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_AdjustSeek:%#lx", (unsigned long)player));

    /* after seeek, index points to the location of current I frames, however to play this frame it would be necessary to poll back to the previous I frame, and find matching PTS on the extension layer */
    base_pid = player->pids+0;
    ext_pid = player->pids+1;

    BNAV_Player_P_SVC_PidState_Load(&base_state, base_pid, false);

    rc = BNAV_Player_P_SVC_GetNextPidEntry(player, entry, base_pid, &base_state);
    if(rc==0) {
        /* 1. find previous I frame */
        rc = BNAV_Player_P_SVC_Find_IFrame(player, entry, base_pid, &base_state);
        BDBG_MSG_TRACE(("BNAV_Player_P_SVC_AdjustSeek:%#lx %sfound previous I frame at %ld:%u", (unsigned long)player, rc!=0?"not ":"", base_state.currentIndex, (unsigned)entry->data.frame.pts));
    }
    if(rc!=0) {
        BNAV_Player_P_SVC_PidState_Load(&base_state, base_pid, true);
        rc = BNAV_Player_P_SVC_Find_IFrame(player, entry, base_pid, &base_state);
        BDBG_MSG_TRACE(("BNAV_Player_P_SVC_AdjustSeek:%#lx %sfound next I frame at %ld:%u", (unsigned long)player, rc!=0?"not ":"",base_state.currentIndex, (unsigned)entry->data.frame.pts));
        if(rc!=0) { rc = BNAV_SVC_TRACE(-1); goto error; }
    }

    if( !(entry->header.type == BNAV_SVC_Entry_Type_eFrame || entry->header.type == BNAV_SVC_Entry_Type_eExtensionFrame)) {
        rc = BNAV_SVC_TRACE(-1); goto error;
    }
    player->currentIndex = base_state.currentIndex;
    if(navState->packetSize) {
        player->currentOffset = entry->common.offset - entry->common.offset%navState->packetSize; /* keep normal play pointer aligned with the packet boundary */
    }
    if(!baseOnly) {
        BNAV_Player_SVC_PidState ext_state;
        BNAV_Player_P_SVC_PidState_Load(&ext_state, ext_pid, false);
        rc = BNAV_Player_P_SVC_Find_Timestamp(player, entry, ext_pid, &ext_state, entry->data.frame.pts);
        if(rc!=0) {rc=BNAV_SVC_TRACE(rc);goto error;}
        BDBG_MSG_TRACE(("BNAV_Player_P_SVC_AdjustSeek:%#lx found previous timestamp at %ld", (unsigned long)player, ext_state.currentIndex));
        BNAV_Player_P_SVC_PidState_Save(&ext_state, ext_pid);
    }

    BNAV_Player_P_SVC_PidState_Save(&base_state, base_pid);


    BDBG_MSG(("BNAV_Player_P_SVC_AdjustSeek:%#lx done base %ld ext %ld", (unsigned long)player, base_pid->currentIndex, ext_pid->currentIndex));

    return;

error:
    player->currentIndex =  base_pid->currentIndex = ext_pid->currentIndex = BNAV_Player_P_GetFirstIndex(player->parent);
    BDBG_MSG(("BNAV_Player_P_SVC_AdjustSeek:%#lx back to first %ld", (unsigned long)player, base_pid->currentIndex));

    return;
}



int BNAV_Player_SVC_PopulateFifo(BNAV_Player_SVC_HandleImpl *player)
{
    int rc;
    BNAV_Player_State navState;
    BNAV_SVC_Entry svcEntry;

    BDBG_MSG_TRACE(("BNAV_Player_SVC_PopulateFifo: %#lx", (unsigned long)player));
    BNAV_Player_P_GetState(player->parent, &navState);
    if(player->after_seek) {
        player->after_seek = false;
        BNAV_Player_P_SVC_AdjustSeek(player, &svcEntry, &navState);
    }
    switch(navState.playMode) {
    case eBpPlayNormalByFrames:
    default:
        BDBG_ERR(("%#lx:Not supported playMode:%u", (unsigned long)player, navState.playMode));
        return -BERR_TRACE(BERR_NOT_SUPPORTED);
    case eBpPlayTimeSkip:
    case eBpPlayI:
        rc = BNAV_Player_P_SVC_PopulateFifo_Trick(player, &svcEntry, &navState);
        if(rc!=0) {return BNAV_SVC_TRACE(rc);}
        break;
    case eBpPlayNormal:
        rc = BNAV_Player_P_SVC_GetNextPlayEntry_Play(player, &navState, &svcEntry);
        if(rc!=0) {return BNAV_SVC_TRACE(rc);}
        break;
    }
    return  0;
}

void BNAV_Player_SVC_SetCurrentIndex(BNAV_Player_SVC_HandleImpl *player, long index)
{
    unsigned i;

    BDBG_MSG_TRACE(("BNAV_Player_SVC_SetCurrentIndex: %#lx %ld", (unsigned long)player, (unsigned long)index));
    player->currentIndex = index;
    player->after_seek = true;
    for(i=0;i<BNAV_PLAYER_SVC_MAX_PIDS;i++) {
        BNAV_Player_SVC_Pid *pid = player->pids+i;
        pid->currentIndex = index;
    }
    return;
}

int BNAV_Player_SVC_GetPositionInformation(BNAV_Player_SVC_HandleImpl *player, long index, BNAV_Player_Position *position)
{
    int rc;
    bool foundFrame;
    BNAV_SVC_Entry entry;
    BNAV_Player_State navState;
    BNAV_Player_SVC_PidState pidState;

    BDBG_MSG_TRACE(("BNAV_Player_SVC_GetPositionInformation: %#lx %#ld %#lx", (unsigned long)player, (long)index, (unsigned long)position));
    BDBG_ASSERT(position);
    BKNI_Memset(position, 0, sizeof(*position));
    BNAV_Player_P_GetState(player->parent, &navState);
    BNAV_Player_P_SVC_PidState_Init(&pidState, false, index);

    BSTD_UNUSED(foundFrame);
    for(foundFrame=false;;) {
        rc = BNAV_Player_P_SVC_ReadEntry(player, &entry, pidState.nextIndex);
        if(rc!=0) {
            if(!pidState.forward) {
                pidState.forward = true;
                pidState.nextIndex = pidState.currentIndex;
                continue;
            }
            return BERR_TRACE(rc);
        }
        pidState.currentIndex = pidState.nextIndex;
        pidState.nextIndex += pidState.forward ? 1 : -1;
        if(entry.header.pid != navState.videoPid) {
            continue;
        }
        if( (entry.header.type == BNAV_SVC_Entry_Type_eFrame || entry.header.type == BNAV_SVC_Entry_Type_eExtensionFrame)) {
            foundFrame = true;
            position->offsetLo = entry.common.offset;
            position->offsetHi = (entry.common.offset>>32);
            position->index = pidState.currentIndex;
            position->pts = entry.data.frame.pts;
            position->timestamp = entry.data.frame.timestamp;
            BDBG_MSG_TRACE(("BNAV_Player_SVC_GetPositionInformation:%#lx found frame %ld:%ld %u:%u:%u", (unsigned long)player, index, position->index, position->offsetLo, position->pts, position->timestamp));
            break;
        }
    }
    BDBG_MSG(("BNAV_Player_SVC_GetPositionInformation:%p found %ld:%ld %u:%u:%u", (void *)player, index, position->index, (unsigned)position->offsetLo, (unsigned)position->pts, (unsigned)position->timestamp));

    return 0;
}

#if !B_REFSW_MINIMAL
static int BNAV_Player_P_SVC_ReadOffset( void *player_, long index, uint64_t *offset)
{
    BNAV_Player_SVC_HandleImpl *player = player_;
    long startIndex = index;
    int rc=0;
    bool forward = false;

    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_ReadOffset: %#lx %ld", (unsigned long)player, index));
    for(;;) {
        BNAV_SVC_Entry entry;
        rc = BNAV_Player_P_SVC_ReadEntry(player, &entry, index);
        if(rc<0) { 
            rc=BNAV_SVC_TRACE(rc);
            if(forward) {
                goto error; 
            } else {
                index = startIndex;
                forward = true;
            }
        } else {
            *offset  = entry.common.offset;
            break;
        }
        /* SW7344-190, Coverity: 34347 */
        /* coverity[dead_error_condition] */
        /* coverity[dead_error_line] */
        index += forward?1:-1;
    }
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_ReadOffset: %#lx %ld->%ld:%u", (unsigned long)player, startIndex, index, (unsigned)*offset));
    return 0;

error:
    return rc;
}

long BNAV_Player_SVC_FindIndexFromOffset( BNAV_Player_SVC_HandleImpl *player, uint64_t offset) 
{
    long rc;

    BDBG_MSG_TRACE(("BNAV_Player_SVC_FindIndexFromOffset: %#lx %lu", (unsigned long)player, (unsigned long)offset));
    rc = BNAV_Player_P_FindIndexForOffset(player->parent, offset, player, BNAV_Player_P_SVC_ReadOffset);
    BDBG_MSG_TRACE(("BNAV_Player_SVC_FindIndexFromOffset: %#lx %lu -> %ld", (unsigned long)player, (unsigned long)offset, rc));
    return rc;
}
#endif

long BNAV_Player_SVC_FindIFrameFromIndex( BNAV_Player_SVC_HandleImpl *player, long index, eBpDirectionParam dir)
{
    BNAV_Player_SVC_PidState pidState;
    BNAV_Player_SVC_Pid *pid;
    BNAV_Player_State navState;
    BNAV_SVC_Entry entry;
    int rc;

    BDBG_MSG_TRACE(("BNAV_Player_SVC_FindIFrameFromIndex: %#lx %ld %s", (unsigned long)player, (long)index, dir==eBpForward?"forward":"reverse"));
    BNAV_Player_P_GetState(player->parent, &navState);
    pid = BNAV_Player_P_SVC_FindPid(player, navState.videoPid);
    if(pid==NULL) {return -BERR_TRACE(BERR_NOT_SUPPORTED);}
    BNAV_Player_P_SVC_PidState_Init(&pidState, dir==eBpForward, index);
    rc = BNAV_Player_P_SVC_Find_IFrame(player, &entry, pid, &pidState);
    if(rc!=0) {return BNAV_SVC_TRACE(rc);}

    BDBG_MSG_TRACE(("BNAV_Player_SVC_FindIFrameFromIndex: %#lx %ld->%ld %s", (unsigned long)player, (long)index, pidState.currentIndex, dir==eBpForward?"forward":"reverse"));
    return pidState.currentIndex;
}

static int BNAV_Player_P_SVC_ReadTimestamp(void *handle_, long index, unsigned long *timestamp)
{
    BNAV_Player_SVC_HandleImpl *player = handle_;
    long startIndex = index;
    int rc;

    BSTD_UNUSED(startIndex);
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_ReadTimestamp: %#lx %ld", (unsigned long)player, index));
    for(;;) {
        BNAV_SVC_Entry entry;

        rc = BNAV_Player_P_SVC_ReadEntry(player, &entry, index);
        if(rc<0) { rc=BNAV_SVC_TRACE(rc); goto error; }
        if( (entry.header.type == BNAV_SVC_Entry_Type_eFrame || entry.header.type == BNAV_SVC_Entry_Type_eExtensionFrame)) {
            *timestamp = entry.data.frame.timestamp;
            break;
        }
        index++;
    }
    BDBG_MSG_TRACE(("BNAV_Player_P_SVC_ReadTimestamp: %#lx %ld->%ld:%u", (unsigned long)player, startIndex, index, *timestamp));
    return 0;

error:
    return rc;
}

long BNAV_Player_SVC_FindIndexFromTimestamp( BNAV_Player_SVC_HandleImpl *player, unsigned long timestamp)
{
    BDBG_MSG_TRACE(("BNAV_Player_SVC_FindIndexFromTimestamp: %#lx %ld", (unsigned long)player, (unsigned long)timestamp));
    return BNAV_Player_P_SearchIndexByTimestamp(player->parent, timestamp, player, BNAV_Player_P_SVC_ReadTimestamp);
}

bool BNAV_Player_SVC_IndexIsBFrame(BNAV_Player_SVC_HandleImpl *player, long index)
{
    BDBG_MSG_TRACE(("BNAV_Player_SVC_IndexIsBFrame: %#lx %ld", (unsigned long)player, (long)index));
    BSTD_UNUSED(player);
    BSTD_UNUSED(index);
    return false;
}

