/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "still_decode.h"
#include "convert.h"

#define CALLBACK_STILLDECODE_PICUREREADY  "CallbackStillDecodePictureReady"

BDBG_MODULE(atlas_still_decode);

/* bwin io callback that is triggered when it is safe to handle callbacks */
static void bwinPictureReadyCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CStillDecode * pStillDecode = (CStillDecode *)pObject;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pStillDecode);

    pStillDecode->stillDecodeCallback();
} /* bwinPictureReadyCallback */

static void NexusStillEventCallback(
        void * context,
        int    param
        )
{
    CStillDecode * pStillDecode = (CStillDecode *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pStillDecode);

    /* call callback from Nexus context!  callback code will have to sync with bwin */

    /* TODO: ??? changing this to call stillDecodePictureReadyCallback() may be necessary if
     * we are not waiting for the callback in CThumb::extract(). will also have to change the context
     * pointer in CStillDecode::start() when setting up the callback */
    pStillDecode->stillDecodeCallback();
} /* NexusStillEventCallback */

void CStillDecode::stillDecodePictureReadyCallback()
{
    CWidgetEngine * pWidgetEngine = getWidgetEngine();

    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(this, CALLBACK_STILLDECODE_PICUREREADY);
    }
} /* stillDecodePictureReadyCallback */

void CStillDecode::stillDecodeCallback()
{
    /* captured thumbnail is ready! */
    if (NULL != _callback)
    {
        _callback(_callbackParam);
    }
} /* stillDecodeCallback */

CStillDecode::CStillDecode(
        const char *     name,
        const uint16_t   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_decodeStill, pCfg),
    _stillDecode(NULL),
    _pWidgetEngine(NULL),
    _callback(NULL),
    _callbackParam(NULL)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NEXUS_NUM_STILL_DECODES > _number);

    BDBG_ASSERT(eRet_Ok == ret);
}

CStillDecode::~CStillDecode()
{
    close();
}

eRet CStillDecode::open()
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != _pWidgetEngine);

    if (true == isOpened())
    {
        CHECK_ERROR_GOTO("Still decoder is already opened.", ret, error);
    }

    _pWidgetEngine->addCallback(this, CALLBACK_STILLDECODE_PICUREREADY, bwinPictureReadyCallback);

    _stillDecode = NEXUS_StillDecoder_Open(NULL, 0, NULL);
    CHECK_PTR_ERROR_GOTO("Nexus still decoder open failed!", _stillDecode, ret, eRet_NotAvailable, error);

error:
    return(ret);
} /* open */

void CStillDecode::close()
{
    if (NULL != _stillDecode)
    {
        NEXUS_StillDecoder_Close(_stillDecode);
        _stillDecode = NULL;
    }

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_STILLDECODE_PICUREREADY);
    }
}

eRet CStillDecode::start(
        CPid *          pPid,
        B_EventCallback callback,
        void *          context
        )
{
    eRet        ret       = eRet_Ok;
    NEXUS_Error nerror    = NEXUS_SUCCESS;
    CPlatform * pPlatform = _pCfg->getPlatformConfig();
    NEXUS_StillDecoderStartSettings settings;

    /* sanity check - make sure currently pid codec is supported by still decoder */
    if (pPlatform->isMemSettingsValid())
    {
        NEXUS_MemoryConfigurationSettings * pMemSettings = pPlatform->getMemSettings();
        NEXUS_VideoCodec                    codec        = NEXUS_VideoCodec_eUnknown;

        BDBG_ASSERT(NULL != pPid);
        codec = pPid->getVideoCodec();

        if (NEXUS_VideoCodec_eH264_Mvc != codec)
        {
            /* compare video decoder and still decoder supported codec (we will ignore h264_mvc comparison) */
            if (pMemSettings->videoDecoder[0].supportedCodecs[codec] !=
                pMemSettings->stillDecoder[getNumber()].supportedCodecs[codec])
            {
                ret = eRet_InvalidState;
                BDBG_ERR(("Codec (%s) mismatch between video decoder and still decoder", videoCodecToString(codec).s()));
                CHECK_ERROR_GOTO("Update still decoder supported codecs in nexus_platform_xxxxx.c", ret, error);
            }
        }
    }

    _callback      = callback;
    _callbackParam = context;

    BDBG_MSG(("start still decode with pid:%p num:%d", (void *)pPid->getPidChannel(), pPid->getPid()));
    NEXUS_StillDecoder_GetDefaultStartSettings(&settings);
    settings.pidChannel                 = pPid->getPidChannel();
    settings.stillPictureReady.callback = NexusStillEventCallback;
    settings.stillPictureReady.context  = this;
    settings.codec                      = pPid->getVideoCodec();
    nerror                              = NEXUS_StillDecoder_Start(getStillVideoDecode(), &settings);
    CHECK_NEXUS_ERROR_GOTO("error starting still video decoder", ret, nerror, error);

error:
    return(ret);
} /* start */

eRet CStillDecode::stop()
{
    eRet ret = eRet_Ok;

    _callback = NULL;

    if (NULL != _stillDecode)
    {
        NEXUS_StillDecoder_Stop(_stillDecode);
    }

    return(ret);
} /* stop */