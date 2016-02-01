/***************************************************************************
 *     Copyright (c) 2011, Broadcom Corporation
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
 * Module Description: 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef BCMPLAYER_AVC_H
#define BCMPLAYER_AVC_H

#include "bcmindexer_svc.h"

typedef struct BNAV_Player_SVC_Pid {
    unsigned short pid;
    long currentIndex;
} BNAV_Player_SVC_Pid ;

#define BNAV_PLAYER_SVC_MAX_PIDS    2

typedef struct BNAV_Player_SVC_HandleImpl {
    BNAV_Player_Handle parent;
    BNAV_Player_SVC_Pid pids[BNAV_PLAYER_SVC_MAX_PIDS];
    bool after_seek;
    long currentIndex;
    uint64_t currentOffset; /* should be a transport packet alligned */
} BNAV_Player_SVC_HandleImpl;


int BNAV_Player_SVC_Reset(BNAV_Player_SVC_HandleImpl *player, BNAV_Player_Handle parent);
int BNAV_Player_SVC_PopulateFifo(BNAV_Player_SVC_HandleImpl *player);
void BNAV_Player_SVC_SetCurrentIndex(BNAV_Player_SVC_HandleImpl *player, long index);
int BNAV_Player_SVC_GetPositionInformation(BNAV_Player_SVC_HandleImpl *player, long index, BNAV_Player_Position *position);
long BNAV_Player_SVC_FindIndexFromOffset( BNAV_Player_SVC_HandleImpl *player, uint64_t offset);
long BNAV_Player_SVC_FindIFrameFromIndex( BNAV_Player_SVC_HandleImpl *player, long index, eBpDirectionParam dir);
long BNAV_Player_SVC_FindIndexFromTimestamp( BNAV_Player_SVC_HandleImpl *player, unsigned long timestamp);
bool BNAV_Player_SVC_IndexIsBFrame(BNAV_Player_SVC_HandleImpl *player, long index);

#endif /* BCMPLAYER_AVC_H */

