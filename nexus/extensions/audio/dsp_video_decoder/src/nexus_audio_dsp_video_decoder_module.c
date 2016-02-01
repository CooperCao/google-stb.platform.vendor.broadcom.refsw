/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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

#include "nexus_audio_dsp_video_decoder_module.h"

BDBG_MODULE(nexus_audio_dsp_video_decoder);

BVDE_Handle g_NEXUS_vdeHandle;

NEXUS_Error NEXUS_AudioModule_P_InitDspVideoDecoder(void)
{
    BERR_Code errCode;
    BVDE_Settings vdeSettings;

    BVDE_GetDefaultSettings(&vdeSettings);
    vdeSettings.maxDspTasks = NEXUS_NUM_DSP_VIDEO_DECODERS;
    /* Stripe width not set, it should be retrieved from BCHP internally */
    errCode = BVDE_Open(&g_NEXUS_vdeHandle,
                        g_pCoreHandles->chp,
                        g_pCoreHandles->reg,
                        g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma,
                        g_pCoreHandles->bint,
                        g_pCoreHandles->tmr,
                        g_NEXUS_audioModuleData.dspHandle,
                        &vdeSettings
                        );

    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_vde;
    }

    errCode = NEXUS_DspVideoDecoder_P_InitWatchdog();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_watchdog;
    }

    return BERR_SUCCESS;

err_watchdog:
    BVDE_Close(g_NEXUS_vdeHandle);
    g_NEXUS_vdeHandle = NULL;
err_vde:
    return errCode;
}

void NEXUS_AudioModule_P_UninitDspVideoDecoder(void)
{
    if ( g_NEXUS_vdeHandle )
    {
        NEXUS_DspVideoDecoder_P_UninitWatchdog();
        BVDE_Close(g_NEXUS_vdeHandle);
        g_NEXUS_vdeHandle = NULL;
    }
}

