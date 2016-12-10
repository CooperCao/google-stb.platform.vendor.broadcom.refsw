/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "atlas_main_nx.h"
#include "display_nx.h"
#include "control_nx.h"
#include "graphics_nx.h"
#include "output_nx.h"
#include "video_decode_nx.h"
#include "audio_decode_nx.h"
#include "bluetooth_nx.h"
#include "audio_capture_nx.h"

BDBG_MODULE(atlas_main);

CAtlasNx::CAtlasNx(
        uint16_t      number,
        eAtlasMode    mode,
        CChannelMgr * pChannelMgr,
        CLua *        pLua
        ) :
    CAtlas(number, mode, pChannelMgr, pLua)
{
}

CControl * CAtlasNx::controlCreate()
{
    return(new CControlNx("CAtlas::pControl (Nx)"));
}

CDisplay * CAtlasNx::displayCreate()
{
    eRet         ret      = eRet_Ok;
    CDisplayNx * pDisplay = NULL;

    pDisplay = (CDisplayNx *)_pBoardResources->checkoutResource(this, eBoardResource_display);
    CHECK_PTR_ERROR_GOTO("unable to checkout display", pDisplay, ret, eRet_NotAvailable, error);

    _model.addDisplay(pDisplay);

error:
    return(pDisplay);
}

CGraphics * CAtlasNx::graphicsCreate()
{
    eRet          ret       = eRet_Ok;
    CGraphicsNx * pGraphics = NULL;

    pGraphics = (CGraphicsNx *)_pBoardResources->checkoutResource(this, eBoardResource_graphics);
    CHECK_PTR_ERROR_GOTO("unable to checkout graphics", pGraphics, ret, eRet_NotAvailable, error);

    _model.addGraphics(pGraphics);

error:
    return(pGraphics);
}

eRet CAtlasNx::mosaicInitialize()
{
    eRet                 ret                = eRet_Ok;
    CStc *               pStcMosaic         = NULL;
    CSimpleVideoDecode * pVideoDecodeMosaic = NULL;

    ATLAS_MEMLEAK_TRACE("BEGIN");

    for (int winType = eWindowType_Mosaic1; winType < eWindowType_Max; winType++)
    {
        pStcMosaic = stcInitialize((eWindowType)winType);
        CHECK_PTR_ERROR_GOTO("unable to initialize mosaic simple stc", pStcMosaic, ret, eRet_NotAvailable, error);

        pVideoDecodeMosaic = videoDecodeInitialize(pStcMosaic, (eWindowType)winType);
        CHECK_PTR_WARN("unable to initialize mosaic video decode", pVideoDecodeMosaic, ret, eRet_NotAvailable);

        if (NULL == pVideoDecodeMosaic)
        {
            stcUninitialize(&pStcMosaic, (eWindowType)winType);
            if (winType > eWindowType_Mosaic1)
            {
                /* at least one mosiac initialized so return success */
                ret = eRet_Ok;
            }
            break;
        }
    }

    goto done;
error:
    mosaicUninitialize();
done:
    ATLAS_MEMLEAK_TRACE("END");
    return(ret);
} /* mosaicInitialize */

void CAtlasNx::mosaicUninitialize()
{
    CStc *               pStcMosaic         = NULL;
    CSimpleVideoDecode * pVideoDecodeMosaic = NULL;

    for (int winType = eWindowType_Mosaic1; winType < eWindowType_Max; winType++)
    {
        CSimpleVideoDecode * pVideoDecodeMosaic = _model.getSimpleVideoDecode((eWindowType)winType);
        CStc *               pStcMosaic         = _model.getStc((eWindowType)winType);

        videoDecodeUninitialize(&pVideoDecodeMosaic);
        stcUninitialize(&pStcMosaic, (eWindowType)winType);
    }
}

CSimpleVideoDecode * CAtlasNx::videoDecodeCreate(eWindowType windowType)
{
    eRet                   ret          = eRet_Ok;
    CSimpleVideoDecodeNx * pVideoDecode = NULL;

    if (eWindowType_Pip == windowType)
    {
        CConfigNx *             pConfig       = (CConfigNx *)_pConfig;
        NxClient_AllocResults * pAllocResults = pConfig->getAllocResultsPip();

        /* must use a particular video decoder for pip based on alloc results.
         * nxclient pip decoder alloc id must match checked out decoder resource number
         * to ensure we are checking out the correct pip decoder. */
        pVideoDecode =
            (CSimpleVideoDecodeNx *)_pBoardResources->checkoutResource(
                    this,
                    eBoardResource_simpleDecodeVideo,
                    ANY_INDEX,
                    pAllocResults->simpleVideoDecoder[0].id);
    }
    else
    {
        pVideoDecode = (CSimpleVideoDecodeNx *)_pBoardResources->checkoutResource(this, eBoardResource_simpleDecodeVideo);
    }
    CHECK_PTR_MSG_GOTO("unable to checkout simple video decoder", pVideoDecode, ret, eRet_NotAvailable, error);

    _model.addSimpleVideoDecode(pVideoDecode, windowType);
error:
    return(pVideoDecode);
} /* videoDecodeCreate */

CSimpleAudioDecode * CAtlasNx::audioDecodeCreate(eWindowType windowType)
{
    eRet                   ret          = eRet_Ok;
    CSimpleAudioDecodeNx * pAudioDecode = NULL;

    pAudioDecode = (CSimpleAudioDecodeNx *)_pBoardResources->checkoutResource(this, eBoardResource_simpleDecodeAudio);
    CHECK_PTR_ERROR_GOTO("unable to checkout simple audio decoder", pAudioDecode, ret, eRet_NotAvailable, error);

    /* add audio decode to model for main */
    _model.addSimpleAudioDecode(pAudioDecode, windowType);
error:
    return(pAudioDecode);
}

COutputHdmi * CAtlasNx::outputHdmiInitialize(CDisplay * pDisplay)
{
    eRet          ret         = eRet_Ok;
    COutputHdmi * pOutputHdmi = NULL;

    if (NULL == pDisplay)
    {
        return(pOutputHdmi);
    }

    /* add/setup HDMI output if in resource list */
    pOutputHdmi = outputHdmiCreate();
    CHECK_PTR_ERROR_GOTO("unable to checkout hdmi output", pOutputHdmi, ret, eRet_NotAvailable, error);

    /* connect output to display */
    ret = pDisplay->addOutput(pOutputHdmi);
    CHECK_ERROR_GOTO("Error adding HDMI output", ret, error);

    goto done;
error:
    outputHdmiUninitialize(pDisplay, &pOutputHdmi);
done:
    return(pOutputHdmi);
} /* outputHdmiInitialize */

#ifdef NETAPP_SUPPORT
CBluetooth * CAtlasNx::bluetoothCreate()
{
    eRet           ret        = eRet_Ok;
    CBluetoothNx * pBluetooth = NULL;

    pBluetooth = (CBluetoothNx *)_pBoardResources->checkoutResource(this, eBoardResource_bluetooth);
    CHECK_PTR_ERROR_GOTO("unable to checkout bluetooth Nx", pBluetooth, ret, eRet_OutOfMemory, error);

error:
    return(pBluetooth);
}

CAudioCapture * CAtlasNx::audioCaptureInitialize(CBluetooth * pBluetooth)
{
    eRet              ret           = eRet_Ok;
    CAudioCaptureNx * pAudioCapture = NULL;

    ATLAS_MEMLEAK_TRACE("BEGIN");

    if (NULL == pBluetooth)
    {
        return(pAudioCapture);
    }

    pAudioCapture = new CAudioCaptureNx("AudioCaptureNx");
    CHECK_PTR_ERROR_GOTO("unable to allocate Audio Capture Nx", pAudioCapture, ret, eRet_OutOfMemory, error);
    _model.setAudioCapture(pAudioCapture);

    /* Add bluetooth as an audio capture client */
    pAudioCapture->addClient(pBluetooth);

    ret = pAudioCapture->open(_pWidgetEngine);
    CHECK_ERROR_GOTO("unable to open Audio Capture Nx", ret, error);

    goto done;
error:
done:
    ATLAS_MEMLEAK_TRACE("END");
    return(pAudioCapture);
} /* audioCaptureInitialize */

void CAtlasNx::audioCaptureUninitialize()
{
    CAudioCaptureNx * pAudioCapture = (CAudioCaptureNx *)_model.getAudioCapture();

    if (NULL == pAudioCapture)
    {
        return;
    }

    pAudioCapture->close();
    pAudioCapture->clearClientList();
    _model.setAudioCapture(NULL);
    DEL(pAudioCapture);
}

#endif /* NETAPP_SUPPORT */

CVideoWindow * CAtlasNx::videoWindowInitialize(
        CDisplay *           pDisplay,
        CSimpleVideoDecode * pVideoDecode,
        eWindowType          windowType
        )
{
    BSTD_UNUSED(pDisplay);
    BSTD_UNUSED(pVideoDecode);
    BSTD_UNUSED(windowType);

    return(NULL);
}

void CAtlasNx::videoWindowUninitialize(
        CDisplay *           pDisplay,
        CSimpleVideoDecode * pVideoDecode,
        CVideoWindow *       pVideoWindow
        )
{
    BSTD_UNUSED(pDisplay);
    BSTD_UNUSED(pVideoDecode);
    BSTD_UNUSED(pVideoWindow);
}

COutputComponent * CAtlasNx::outputComponentInitialize(CDisplay * pDisplay)
{
    BSTD_UNUSED(pDisplay);
    return(NULL);
}

void CAtlasNx::outputComponentUninitialize(
        CDisplay *          pDisplay,
        COutputComponent ** pOutputComponent
        )
{
    BSTD_UNUSED(pDisplay);
    BSTD_UNUSED(pOutputComponent);
}

COutputComposite * CAtlasNx::outputCompositeInitialize(CDisplay * pDisplay)
{
    BSTD_UNUSED(pDisplay);
    return(NULL);
}

void CAtlasNx::outputCompositeUninitialize(
        CDisplay *          pDisplay,
        COutputComposite ** pOutputComposite
        )
{
    BSTD_UNUSED(pDisplay);
    BSTD_UNUSED(pOutputComposite);
}

COutputRFM * CAtlasNx::outputRfmInitialize(CDisplay * pDisplay)
{
    BSTD_UNUSED(pDisplay);
    return(NULL);
}

void CAtlasNx::outputRfmUninitialize(
        CDisplay *    pDisplay,
        COutputRFM ** pOutputRfm
        )
{
    BSTD_UNUSED(pDisplay);
    BSTD_UNUSED(pOutputRfm);
}

CSimpleAudioDecode * CAtlasNx::audioDecodeInitializePip(
        COutputHdmi *     pOutputHdmi,
        COutputSpdif *    pOutputSpdif,
        COutputAudioDac * pOutputAudioDac,
        COutputRFM *      pOutputRFM,
        CStc *            pStc,
        eWindowType       winType
        )
{
    return(CAtlas::audioDecodeInitialize(pOutputHdmi, pOutputSpdif, pOutputAudioDac, pOutputRFM, pStc, winType));
}