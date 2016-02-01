/***************************************************************************
 *     Copyright (c) 2007-2008, Broadcom Corporation
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
 **************************************************************************/
#ifndef NEXUS_PLAYBACK_MODULE_H__
#define NEXUS_PLAYBACK_MODULE_H__

#include "nexus_base.h"
#include "nexus_playback_thunks.h"
#include "nexus_playback.h"
#if NEXUS_PLAYBACK_BLOCKAUTH
#include "nexus_playback_blockauth.h"
#endif
#include "priv/nexus_playback_notify.h"
#include "nexus_playback_init.h"

#ifdef __cplusplus
extern "C" {
#endif

extern NEXUS_ModuleHandle NEXUS_PlaybackModule;

#define NEXUS_MODULE_SELF NEXUS_PlaybackModule

void NEXUS_Playback_RecordProgress_priv(NEXUS_PlaybackHandle p);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PLAYBACK_MODULE_H__ */

