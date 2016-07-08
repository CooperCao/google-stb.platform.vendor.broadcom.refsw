/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  .  Except as set forth in an Authorized License, Broadcom grants
 *  no license , right to use, or waiver of any kind with respect to the
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
 *  LICENSORS BE LIABLE FOR  CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR  ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/
#include "nexus_video_decoder_module.h"
#include "nexus_still_decoder_impl.h"

BDBG_MODULE(nexus_video_decoder_zsp_avd);

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_OpenMosaic( unsigned parentIndex, unsigned index, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings )
{
    return NEXUS_VideoDecoder_P_OpenMosaic_Avd( parentIndex, index, pOpenSettings );
}

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_Open( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    if(index<NEXUS_NUM_VIDEO_DECODERS) {
        return NEXUS_VideoDecoder_P_Open_Avd(index, pOpenSettings);
    } else if (index<(NEXUS_NUM_VIDEO_DECODERS+NEXUS_NUM_ZSP_VIDEO_DECODERS)) {
        return NEXUS_VideoDecoder_P_Open_Zsp(index - NEXUS_NUM_VIDEO_DECODERS, pOpenSettings);
#if NEXUS_NUM_SOFT_VIDEO_DECODERS
    } else if (index<NEXUS_NUM_VIDEO_DECODERS+NEXUS_NUM_ZSP_VIDEO_DECODERS+NEXUS_NUM_SOFT_VIDEO_DECODERS) {
        return NEXUS_VideoDecoder_P_Open_Soft(index - (NEXUS_NUM_VIDEO_DECODERS+NEXUS_NUM_ZSP_VIDEO_DECODERS), pOpenSettings);
#endif
    } else {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
}

NEXUS_ModuleHandle NEXUS_VideoDecoderModule_Init(const NEXUS_VideoDecoderModuleInternalSettings *pSettings)
{
    NEXUS_Error rc;
    rc = NEXUS_VideoDecoderModule_P_Init_Zsp(pSettings);
    if(rc!=NEXUS_SUCCESS) {
        (void)BERR_TRACE(rc);
        return NULL;
    }
    return NEXUS_VideoDecoderModule_P_Init_Avd(pSettings);
}

void NEXUS_VideoDecoderModule_Uninit(void)
{
    NEXUS_VideoDecoderModule_P_Uninit_Avd();
    NEXUS_VideoDecoderModule_P_Uninit_Zsp();
}

