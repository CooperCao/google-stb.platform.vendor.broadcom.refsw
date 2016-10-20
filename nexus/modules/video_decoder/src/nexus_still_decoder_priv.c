/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_video_decoder_module.h"
#include "nexus_still_decoder_impl.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_still_decoder_priv);

void NEXUS_StillDecoder_P_CheckForEndCode( void *context )
{
    NEXUS_HwStillDecoderHandle stillDecoder = context;
    uint8_t endCode;
    bool found;
    BXVD_DecodeStillMode stillMode;
    BERR_Code rc=0;

    BDBG_OBJECT_ASSERT(stillDecoder, NEXUS_HwStillDecoder);
    BDBG_OBJECT_ASSERT(stillDecoder->current, NEXUS_StillDecoder);
    stillMode = stillDecoder->current->stillMode;
    endCode = stillDecoder->current->endCode;
    stillDecoder->timer = NULL;

    LOCK_TRANSPORT();
    if(endCode) {
        found = NEXUS_Rave_FindVideoStartCode_priv(stillDecoder->rave, endCode);
    } else {
        found = NEXUS_Rave_FindPts_priv(stillDecoder->rave, 0xFFFFFFFF);
    }
    UNLOCK_TRANSPORT();

    if (found) {
        NEXUS_RaveStatus raveStatus;
        NEXUS_Time curTime;

        NEXUS_Time_Get(&curTime);
        BDBG_MSG(("Found still in %ld msec", NEXUS_Time_Diff(&curTime, &stillDecoder->startTime)));

        stillDecoder->status.endCodeFound = true;
        NEXUS_TaskCallback_Fire(stillDecoder->endCodeFoundCallback);

        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(stillDecoder->rave, &raveStatus);
        UNLOCK_TRANSPORT();
        if (rc) {rc = BERR_TRACE(rc);return;}

        rc = BXVD_DecodeStillPicture(stillDecoder->xvdChannel, stillMode, &raveStatus.xptContextMap);
        if (rc) {rc = BERR_TRACE(rc);return;}
    }
    else {
        /* try again */
        stillDecoder->timer = NEXUS_ScheduleTimer(10, NEXUS_StillDecoder_P_CheckForEndCode, stillDecoder);
    }
}

void NEXUS_VideoDecoder_P_StillReady_isr(void *context, int param, void *data)
{
    NEXUS_HwStillDecoderHandle stillDecoder = (NEXUS_HwStillDecoderHandle)context;
    BXVD_StillPictureBuffers *buf = (BXVD_StillPictureBuffers *)data;
    NEXUS_Time curTime;

    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(stillDecoder, NEXUS_HwStillDecoder);

    /* store at isr time, create NEXUS_StripedSurfaceHandle at task time */
    stillDecoder->stripedSurface.recreate = true;
    stillDecoder->stripedSurface.buffers = *buf;

    NEXUS_Time_Get(&curTime);
    BDBG_MSG(("Decoded still in %ld msec", NEXUS_Time_Diff(&curTime, &stillDecoder->startTime)));
    stillDecoder->status.stillPictureReady = true;
    NEXUS_IsrCallback_Fire_isr(stillDecoder->stillPictureReadyCallback);
}
