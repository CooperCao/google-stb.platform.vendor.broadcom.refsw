/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_SIMPLE_DECODER_MODULE_H__
#define NEXUS_SIMPLE_DECODER_MODULE_H__

#include "nexus_simple_decoder_thunks.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_simple_decoder_init.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_video_decoder_primer.h"
#include "nexus_simple_video_decoder_server.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_audio_playback.h"
#include "nexus_simple_audio_decoder_server.h"
#include "nexus_simple_decoder_types.h"
#include "nexus_simple_encoder.h"
#include "nexus_simple_encoder_server.h"

#ifdef __cplusplus
#error
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif
#define NEXUS_MODULE_NAME simple_decoder
#define NEXUS_MODULE_SELF g_NEXUS_simpleDecoderModule

extern NEXUS_ModuleHandle g_NEXUS_simpleDecoderModule;
extern NEXUS_SimpleDecoderModuleSettings g_NEXUS_simpleDecoderModuleSettings;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleVideoDecoder);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleVideoDecoderServer);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleAudioPlayback);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleAudioDecoder);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleAudioDecoderServer);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleStcChannel);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleEncoder);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SimpleEncoderServer);

#endif
