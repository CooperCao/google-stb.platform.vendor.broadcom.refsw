/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
#ifndef NEXUS_PICTURE_DECODER_MODULE_H__
#define NEXUS_PICTURE_DECODER_MODULE_H__

#include "nexus_picture_decoder_thunks.h"
#include "nexus_base.h"
#include "nexus_picture_decoder.h"
#include "nexus_picture_decoder_init.h"
#if NEXUS_NUM_SID_VIDEO_DECODERS
#include "nexus_picture_decoder_sid_video_decoder_module.h"
#endif
#include "bsid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_PictureDecoder_P_HWState {
    BSID_Handle                sid;
    BSID_ChannelHandle         sidCh;
    BLST_S_HEAD(piclist, NEXUS_PictureDecoder) list;
    NEXUS_PictureDecoderHandle acquired;
    unsigned                   firmwareVersion;
    BKNI_EventHandle           watchdogEvent;
    NEXUS_EventCallbackHandle  watchdogEventHandler;
    /* image interface */
    void * img_context;
    BIMG_Interface img_interface;
} NEXUS_PictureDecoder_P_HWState;


typedef struct NEXUS_PictureDecoder_P_ModuleState {
    NEXUS_ModuleHandle module;
    NEXUS_PictureDecoderModuleSettings settings;
    NEXUS_PictureDecoder_P_HWState     hwState;
} NEXUS_PictureDecoder_P_ModuleState;

extern NEXUS_PictureDecoder_P_ModuleState g_NEXUS_PictureDecoder_P_ModuleState;

#define NEXUS_MODULE_SELF g_NEXUS_PictureDecoder_P_ModuleState.module

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_PictureDecoder);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PICTURE_DECODER_MODULE_H__ */

