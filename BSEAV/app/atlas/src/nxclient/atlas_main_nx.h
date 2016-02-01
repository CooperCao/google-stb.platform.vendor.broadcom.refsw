/***************************************************************************
 * (c) 2002-2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef ATLAS_MAIN_NX_H__
#define ATLAS_MAIN_NX_H__

/* atlas includes */
#include "atlas_main.h"
#include "nxclient.h"
#include "config_nx.h"
#include "model.h"

#ifdef __cplusplus
extern "C" {
#endif

class CAtlasNx : public CAtlas
{
public:
    CAtlasNx(
            uint16_t      number,
            eAtlasMode    mode,
            CChannelMgr * pChannelMgr,
#if DVR_LIB_SUPPORT
            CDvrMgr *     pDvrMgr,
#endif
            CLua *        pLua = NULL
            );

    virtual CControl *           controlCreate(void);
    virtual CDisplay *           displayCreate(void);
    virtual CGraphics *          graphicsCreate(void);
    virtual CVideoWindow *       videoWindowInitialize(CDisplay * pDisplay, CSimpleVideoDecode * pVideoDecode, eWindowType windowType);
    virtual void                 videoWindowUninitialize(CDisplay * pDisplay, CSimpleVideoDecode * pVideoDecode, CVideoWindow * pVideoWindow);
    virtual CStillDecode *       videoDecodeStillInitialize(void) { return(NULL); };
    virtual void                 videoDecodeStillUninitialize(void) { return; };
    virtual COutputHdmi *        outputHdmiInitialize(CDisplay * pDisplay);
    virtual COutputComponent *   outputComponentInitialize(CDisplay * pDisplay);
    virtual void                 outputComponentUninitialize(CDisplay * pDisplay, COutputComponent ** pOutputComponent);
    virtual COutputComposite *   outputCompositeInitialize(CDisplay * pDisplay);
    virtual void                 outputCompositeUninitialize(CDisplay * pDisplay, COutputComposite ** pOutputComposite);
    virtual COutputRFM *         outputRfmInitialize(CDisplay * pDisplay);
    virtual void                 outputRfmUninitialize(CDisplay * pDisplay, COutputRFM ** pOutputRfm);
    virtual COutputSpdif *       outputSpdifInitialize(void) { return(NULL); };
    virtual void                 outputSpdifUninitialize(COutputSpdif ** pOutputSpdif) { BSTD_UNUSED(pOutputSpdif); return; };
    virtual COutputAudioDac *    outputDacInitialize(void) { return(NULL); };
    virtual void                 outputDacUninitialize(COutputAudioDac ** pOutputDac) { BSTD_UNUSED(pOutputDac); return; };
#ifdef NETAPP_SUPPORT
    virtual CBluetooth *         bluetoothCreate(void);
    virtual CAudioCapture *      audioCaptureInitialize(CBluetooth * pBluetooth);
    virtual void                 audioCaptureUninitialize(void);
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_MAIN_NX_H__ */