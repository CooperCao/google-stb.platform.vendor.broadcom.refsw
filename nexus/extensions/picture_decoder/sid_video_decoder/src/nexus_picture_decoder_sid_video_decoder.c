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

#include "nexus_picture_decoder_module.h"
#include "nexus_power_management.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_picture_decoder_sid_video_decoder);

BDBG_OBJECT_ID(NEXUS_SidVideoDecoder);

struct NEXUS_SidVideoDecoder {
    BDBG_OBJECT(NEXUS_SidVideoDecoder)
    bool opened;
    bool started;
    NEXUS_RaveStatus raveStatus;
    NEXUS_SidVideoDecoderStartSettings startSettings;
    BXDM_Decoder_Interface decoderInterface; /* keep a persisitent copy */
    BSID_ChannelHandle hSidCh;
};

static struct NEXUS_SidVideoDecoder g_decoders[NEXUS_NUM_SID_VIDEO_DECODERS];

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_PictureDecoder_P_ModuleState.settings.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_PictureDecoder_P_ModuleState.settings.transport)

void NEXUS_SidVideoDecoder_GetDefaultStartSettings_priv( NEXUS_SidVideoDecoderStartSettings *pSettings )
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

void NEXUS_SidVideoDecoder_GetDefaultOpenSettings_priv( NEXUS_SidVideoDecoderOpenSettings *pSettings )
{
    NEXUS_ASSERT_MODULE();
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->pictureHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    return;
}

NEXUS_SidVideoDecoderHandle NEXUS_SidVideoDecoder_Open_priv(unsigned index, const NEXUS_SidVideoDecoderOpenSettings *openSettings)
{
    NEXUS_SidVideoDecoderHandle  decoder;

    NEXUS_ASSERT_MODULE();
    BSTD_UNUSED(openSettings);
    if(index>=NEXUS_NUM_SID_VIDEO_DECODERS) { BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_args;}

    decoder = &g_decoders[index];
    if ( decoder->opened )
    {
        BDBG_ERR(("SID Video Decoder %u already opened", index));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_args;
    }

    BKNI_Memset(decoder, 0, sizeof(*decoder));
    BDBG_OBJECT_SET(decoder, NEXUS_SidVideoDecoder);
    decoder->opened = true;

    /* SID open channel */
    {
        BERR_Code retCode;
        BSID_Handle hSid = g_NEXUS_PictureDecoder_P_ModuleState.hwState.sid;
        BSID_OpenChannelSettings s_SidChOpenSettings;

        BSID_GetDefaultOpenChannelSettings(BSID_ChannelType_eMotion, &s_SidChOpenSettings);

        s_SidChOpenSettings.e_ChannelType = BSID_ChannelType_eMotion;
        s_SidChOpenSettings.u_ChannelSpecific.motion.hOutputBuffersMmaHeap = NEXUS_Heap_GetMmaHandle(openSettings->pictureHeap);
        s_SidChOpenSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber = 3;
        s_SidChOpenSettings.u_ChannelSpecific.motion.ui32_OutputMaxWidth = 1920;
        s_SidChOpenSettings.u_ChannelSpecific.motion.ui32_OutputMaxHeight = 1080;

        BDBG_MSG(("BSID_OpenChannel"));

        NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_ePictureDecoder, true);
        retCode = BSID_OpenChannel(hSid, &g_decoders[index].hSidCh, 0xAA, &s_SidChOpenSettings);
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BSID_OpenChannel failed err=0x%x", retCode));
            BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_args;
        }
    }

    return decoder;

err_args:
    return NULL;
}

void NEXUS_SidVideoDecoder_Close_priv(NEXUS_SidVideoDecoderHandle decoder)
{
    NEXUS_ASSERT_MODULE();

    BDBG_OBJECT_ASSERT(decoder, NEXUS_SidVideoDecoder);
    /*BDBG_OBJECT_DESTROY(decoder, NEXUS_SidVideoDecoder);*/

    /* sid close channel */
    {
        BSID_CloseChannel(decoder->hSidCh);
        NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_ePictureDecoder, false);
    }
    decoder->opened = false;

    return;
}

void NEXUS_SidVideoDecoder_GetDecoderInterface_priv(NEXUS_SidVideoDecoderHandle decoder, const BXDM_Decoder_Interface **decoderInterface, void **decoderContext)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_SidVideoDecoder);
    BSTD_UNUSED(decoderContext);

    /* SID get xdm interface */
    {
        BSID_GetXdmInterface(decoder->hSidCh, &decoder->decoderInterface, decoderContext);
    }

    *decoderInterface = &decoder->decoderInterface;
    return;
}

NEXUS_Error NEXUS_SidVideoDecoder_Start_priv(NEXUS_SidVideoDecoderHandle decoder, const NEXUS_SidVideoDecoderStartSettings *startSettings)
{
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_SidVideoDecoder);

    if ( decoder->started )
    {
        BDBG_ERR(("Already running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if(startSettings->raveContext) {
        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(startSettings->raveContext, &decoder->raveStatus);
        UNLOCK_TRANSPORT();
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto  err_rave_status;}
    }

    /* SID start decode */
    {
        BERR_Code retCode = BERR_SUCCESS;
        BSID_StartDecodeSettings gs_StartDecodeSettings;

        BSID_GetDefaultStartDecodeSettings(BSID_DecodeMode_eMotion, &gs_StartDecodeSettings);
        gs_StartDecodeSettings.uDecodeSettings.stMotion.ps_RaveContextMap = &decoder->raveStatus.xptContextMap;
        gs_StartDecodeSettings.uDecodeSettings.stMotion.hItbBlock = decoder->raveStatus.itbBlock;
        gs_StartDecodeSettings.uDecodeSettings.stMotion.hCdbBlock = decoder->raveStatus.cdbBlock;

        BDBG_MSG(("BSID_StartDecode"));
        retCode = BSID_StartDecode(decoder->hSidCh, &gs_StartDecodeSettings);
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BSID_StartDecode failed err=0x%x", retCode));
            rc=BERR_TRACE(NEXUS_UNKNOWN);
            goto  err_start_decode;
        }
    }

    decoder->startSettings = *startSettings;
    decoder->started = true;

    return NEXUS_SUCCESS;

err_start_decode:
err_rave_status:
    return BERR_TRACE(rc);
}

void NEXUS_SidVideoDecoder_Stop_priv(NEXUS_SidVideoDecoderHandle decoder)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_SidVideoDecoder);

    /* SID stop decode */
    {
        BSID_StopDecodeSettings settings;
        BDBG_MSG(("BSID_StopDecode"));
        BSID_GetDefaultStopSettings(&settings);
        BSID_StopDecode(decoder->hSidCh, &settings);
    }

    decoder->started = false;
    return;
}

NEXUS_Error NEXUS_SidVideoDecoder_Flush_priv(NEXUS_SidVideoDecoderHandle decoder)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(decoder, NEXUS_SidVideoDecoder);

    if ( !decoder->started )
    {
        BDBG_ERR(("Decoder is not running.  Cannot flush."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* SID disable for flush decode */
    {
        BERR_Code retCode = BERR_SUCCESS;

        BDBG_MSG(("BSID_DisableForFlush"));

        retCode = BSID_DisableForFlush(decoder->hSidCh);
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BSID_StopDecode failed err=0x%x", retCode));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
    }


    if(decoder->startSettings.raveContext) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Disable_priv(decoder->startSettings.raveContext);
        NEXUS_Rave_Flush_priv(decoder->startSettings.raveContext);
        UNLOCK_TRANSPORT();
    }

    /* SID flush channel */
    {
        BERR_Code retCode = BERR_SUCCESS;
        BSID_FlushSettings settings;

        BDBG_MSG(("BSID_FlushChannel"));
        BSID_GetDefaultFlushSettings(&settings);

        retCode = BSID_FlushChannel(decoder->hSidCh, &settings);
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BSID_FlushChannel failed err=0x%x", retCode));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
    }

    if(decoder->startSettings.raveContext) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Enable_priv(decoder->startSettings.raveContext);
        UNLOCK_TRANSPORT();
    }

    return NEXUS_SUCCESS;
}

void NEXUS_SidVideoDecoder_GetRaveSettings_priv(NEXUS_RaveOpenSettings *raveSettings)
{
    NEXUS_ASSERT_MODULE();

    /* SID Get default Itb/Cdb configuration */
    {
        BSID_GetRaveItbCdbConfigInfo(&raveSettings->config);
    }

    return;
}
