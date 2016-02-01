/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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

#include "nexus_audio_dsp_video_encoder_module.h"
#include "bvee.h"

BDBG_MODULE(nexus_audio_dsp_video_encoder);

BVEE_Handle g_NEXUS_veeHandle;

NEXUS_Error NEXUS_AudioModule_P_InitDspVideoEncoder(void)
{
    BERR_Code errCode;
    BVEE_OpenSettings veeSettings;

    BVEE_GetDefaultOpenSettings(&veeSettings);
    veeSettings.maxdsptasks = NEXUS_NUM_DSP_VIDEO_ENCODERS;

    errCode = BVEE_Open(&g_NEXUS_veeHandle,
                        g_pCoreHandles->chp,
                        g_pCoreHandles->reg,
                        g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma,
                        g_pCoreHandles->bint,
                        g_pCoreHandles->tmr,
                        g_NEXUS_audioModuleData.dspHandle,
                        &veeSettings
                        );

    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_vee;
    }

    errCode = NEXUS_DspVideoEncoder_P_InitWatchdog();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_watchdog;
    }

    return BERR_SUCCESS;

err_watchdog:
    BVEE_Close(g_NEXUS_veeHandle);
    g_NEXUS_veeHandle = NULL;
err_vee:
    return errCode;
}

void NEXUS_AudioModule_P_UninitDspVideoEncoder(void)
{
    if ( g_NEXUS_veeHandle )
    {
        NEXUS_DspVideoEncoder_P_UninitWatchdog();
        BVEE_Close(g_NEXUS_veeHandle);
        g_NEXUS_veeHandle = NULL;
    }
}

