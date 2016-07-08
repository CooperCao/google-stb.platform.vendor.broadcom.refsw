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

BDBG_MODULE(nexus_video_decoder_dsp_avd);

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_OpenMosaic( unsigned parentIndex, unsigned index, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings )
{
    return NEXUS_VideoDecoder_P_OpenMosaic_Avd( parentIndex, index, pOpenSettings );
}

static NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_P_OpenDispatch( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    unsigned bounds = NEXUS_NUM_VIDEO_DECODERS;
    if(index<bounds) {
        return NEXUS_VideoDecoder_P_Open_Avd(index, pOpenSettings);
    }
#if NEXUS_NUM_DSP_VIDEO_DECODERS
    bounds += NEXUS_NUM_DSP_VIDEO_DECODERS;
    if (index<bounds) {
        return NEXUS_VideoDecoder_P_Open_Dsp(index - (bounds-NEXUS_NUM_DSP_VIDEO_DECODERS), pOpenSettings);
    }
#endif

#if NEXUS_NUM_SID_VIDEO_DECODERS
    bounds += NEXUS_NUM_SID_VIDEO_DECODERS;
    if (index<bounds) {
        return NEXUS_VideoDecoder_P_Open_Sid(index - (bounds-NEXUS_NUM_SID_VIDEO_DECODERS), pOpenSettings);
    }
#endif

#if NEXUS_NUM_SOFT_VIDEO_DECODERS
    bounds += NEXUS_NUM_SOFT_VIDEO_DECODERS;
    if (index<bounds) {
        return NEXUS_VideoDecoder_P_Open_Soft(index - (bounds-NEXUS_NUM_SOFT_VIDEO_DECODERS), pOpenSettings);
    }
#endif
    (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
    return NULL;
}

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_Open( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    return NEXUS_VideoDecoder_P_OpenDispatch(index, pOpenSettings);
}

NEXUS_ModuleHandle NEXUS_VideoDecoderModule_Init( const NEXUS_VideoDecoderModuleInternalSettings *pModuleSettings, const NEXUS_VideoDecoderModuleSettings *pSettings)
{
    NEXUS_Error rc;
#if NEXUS_NUM_DSP_VIDEO_DECODERS
    rc = NEXUS_VideoDecoderModule_P_Init_Dsp(pModuleSettings);
    if(rc!=NEXUS_SUCCESS) {
        (void)BERR_TRACE(rc);
        return NULL;
    }
#endif

#if NEXUS_NUM_SID_VIDEO_DECODERS
    rc = NEXUS_VideoDecoderModule_P_Init_Sid(pModuleSettings);
    if(rc!=NEXUS_SUCCESS) {
        (void)BERR_TRACE(rc);
        return NULL;
    }
#endif

    return NEXUS_VideoDecoderModule_P_Init_Avd(pModuleSettings, pSettings);
}

void NEXUS_VideoDecoderModule_Uninit(void)
{
    NEXUS_VideoDecoderModule_P_Uninit_Avd();
#if NEXUS_NUM_DSP_VIDEO_DECODERS
    NEXUS_VideoDecoderModule_P_Uninit_Dsp();
#endif
#if NEXUS_NUM_SID_VIDEO_DECODERS
    NEXUS_VideoDecoderModule_P_Uninit_Sid();
#endif
    return;
}

