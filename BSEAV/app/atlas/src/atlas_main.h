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

#ifndef ATLAS_MAIN_H__
#define ATLAS_MAIN_H__

#include "bstd.h"

/* atlas includes */
#include "convert.h"
#include "bwidgets.h"
#include "atlas.h"
#include "atlas_os.h"
#include "atlas_cfg.h"
#include "notification.h"
#include "platform.h"
#include "model.h"
#include "view.h"
#include "board.h"
#include "pidmgr.h"
#include "resource.h"
#include "channelmgr.h"
#if DVR_LIB_SUPPORT
#include "dvrmgr.h"
#include "tsb.h"
#endif
#include "control.h"
#include "atlas_lua.h"
#include "screen_main.h"
#include "still_decode.h"
#include "thumb.h"
#include "config.h"

#ifdef MPOD_SUPPORT
#include "cablecard.h"
#endif
#ifdef ESTB_CFG_SUPPORT
#include "b_estb_cfg_lib.h"
#endif
#ifdef  CDL_SUPPORT
#include "b_cdl_lib.h"
#include "ts_psi.h"
#endif
#ifdef DCC_SUPPORT
#include "closed_caption.h"
#endif
#ifdef SNMP_SUPPORT
#include "snmp.h"
#endif
#ifdef NETAPP_SUPPORT
#include "network.h"
#include "bluetooth.h"
#endif
#ifdef PLAYBACK_IP_SUPPORT
#include "streamer_udp.h"
#include "servermgr.h"
#include "discovery.h"
#include "playlist_db.h"
#endif
#include "audio_capture.h"

#include "nexus_video_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    eAtlasMode_SingleDisplay,
    eAtlasMode_HdSdSingleDisplay,
    eAtlasMode_DualDisplay,
    eAtlasMode_TripleDisplay
} eAtlasMode;

class CDisplay;
#ifdef MPOD_SUPPORT
class CCablecard;
#endif

class CAtlas
{
public:
    CAtlas(
            uint16_t      number,
            eAtlasMode    mode,
            CChannelMgr * pChannelMgr,
#if DVR_LIB_SUPPORT
            CDvrMgr *     pDvrMgr,
#endif
            CLua *        pLua = NULL
            );
    virtual ~CAtlas();

    virtual eRet                 initialize(CConfig * pConfig);
    virtual void                 uninitialize(void);
    virtual CControl *           controlCreate(void);
    virtual CDisplay *           displayCreate(void);
    virtual CGraphics *          graphicsCreate(void);
    virtual CSimpleVideoDecode * videoDecodeCreate(eWindowType windowType);
    virtual CSimpleAudioDecode * audioDecodeCreate(eWindowType windowType);
    virtual COutputHdmi *        outputHdmiCreate(void);
    virtual CVideoWindow *       videoWindowInitialize(CDisplay * pDisplay, CSimpleVideoDecode * pVideoDecode, eWindowType windowType);
    virtual void                 videoWindowUninitialize(CDisplay * pDisplay, CSimpleVideoDecode * pVideoDecode, CVideoWindow * pVideoWindow);
    virtual eRet                 mosaicInitialize(void);
    virtual void                 mosaicUninitialize(void);
    virtual CSimpleVideoDecode * videoDecodeInitialize(CStc * pStc, eWindowType windowType);
    virtual void                 videoDecodeUninitialize(CSimpleVideoDecode ** pVideoDecode);
    virtual CStillDecode *       videoDecodeStillInitialize(void);
    virtual void                 videoDecodeStillUninitialize(void);
    virtual COutputHdmi *        outputHdmiInitialize(CDisplay * pDisplay);
    virtual void                 outputHdmiDestroy(COutputHdmi ** pOutputHdmi);
    virtual void                 outputHdmiUninitialize(CDisplay * pDisplay, COutputHdmi ** pOutputHdmi);
    virtual COutputComponent *   outputComponentInitialize(CDisplay * pDisplay);
    virtual void                 outputComponentUninitialize(CDisplay * pDisplay, COutputComponent ** pOutputComponent);
    virtual COutputComposite *   outputCompositeInitialize(CDisplay * pDisplay);
    virtual void                 outputCompositeUninitialize(CDisplay * pDisplay, COutputComposite ** pOutputComposite);
    virtual COutputRFM *         outputRfmInitialize(CDisplay * pDisplay);
    virtual void                 outputRfmUninitialize(CDisplay * pDisplay, COutputRFM ** pOutputRfm);
    virtual COutputSpdif *       outputSpdifInitialize(void);
    virtual void                 outputSpdifUninitialize(COutputSpdif ** pOutputSpdif);
    virtual COutputAudioDac *    outputDacInitialize(void);
    virtual void                 outputDacUninitialize(COutputAudioDac ** pOutputDac);
    virtual CSimpleAudioDecode * audioDecodeInitializePip(COutputHdmi * pOutputHdmi, COutputSpdif * pOutputSpdif, COutputAudioDac * pOutputAudioDac, COutputRFM * pOutputRFM, CStc * pStc, eWindowType winType);
#ifdef NETAPP_SUPPORT
    virtual CBluetooth *    bluetoothCreate(void);
    virtual CAudioCapture * audioCaptureInitialize(CBluetooth * pBluetooth);
    virtual void            audioCaptureUninitialize(void);
#endif

    bwin_engine_t   getWinEngine(void);
    CWidgetEngine * getWidgetEngine(void) { return(_pWidgetEngine); }
    eRet            run(void);
    CControl *      getControl(void) { return(_pControl); }
    void            controlDestroy(CControl * pControl);
    void            displayDestroy(CDisplay * pDisplay);
    CDisplay *      displayInitialize(NEXUS_VideoFormat videoFormat, uint32_t framebufferWidth, uint32_t framebufferHeight);
    void            displayUninitialize(CDisplay ** pDisplay);
    void            graphicsDestroy(CGraphics * pGraphics);
    CGraphics *     graphicsInitialize(CDisplay * pDisplayHD, CDisplay * pDisplaySD);
    void            graphicsUninitialize(void);
    CIrRemote *     irRemoteCreate(void);
    void            irRemoteDestroy(CIrRemote * pIrRemote);
    CIrRemote *     irRemoteInitialize(void);
    void            irRemoteUninitialize(void);
#ifdef RF4CE_SUPPORT
    CRf4ceRemote * rf4ceRemoteCreate(void);
    void           rf4ceRemoteDestroy(CRf4ceRemote * pRf4ceRemote);
    CRf4ceRemote * rf4ceRemoteInitialize(void);
    void           rf4ceRemoteUninitialize(void);
#endif /* ifdef RF4CE_SUPPORT */
#ifdef NEXUS_HAS_UHF_INPUT
    CUhfRemote * uhfRemoteCreate(void);
    void         uhfRemoteDestroy(CUhfRemote * pUhfRemote);
    CUhfRemote * uhfRemoteInitialize(void);
    void         uhfRemoteUninitialize(void);
#endif /* ifdef NEXUS_HAS_UHF_INPUT */
#ifdef DCC_SUPPORT
    eRet digitalClosedCaptionsInitialize(CConfig * pConfig);
    void digitalClosedCaptionsUninitialize(void);
#endif
    CStc *               stcInitialize(eWindowType windowType);
    void                 stcUninitialize(CStc ** pStc, eWindowType windowType);
    void                 videoDecodeDestroy(eWindowType windowType);
    void                 audioDecodeDestroy(eWindowType windowType);
    CSimpleAudioDecode * audioDecodeInitialize(COutputHdmi * pOutputHdmi, COutputSpdif * pOutputSpdif, COutputAudioDac * pOutputAudioDac, COutputRFM * pOutputRFM, CStc * pStc, eWindowType winType);
    void                 audioDecodeUninitialize(CSimpleAudioDecode ** pAudioDecode, eWindowType winType);
    eRet                 guiInitialize(CConfig * pConfig, CGraphics * pGraphics);
    void                 guiUninitialize(void);
    void                 notificationsInitialize(void);
    void                 notificationsUninitialize(void);
    eRet                 snmpInitialize(void);
    void                 snmpUninitialize(void);
    eRet                 setPreferredVideoFormat(COutputHdmi * pOutputHdmi, COutputComponent * pOutputComponent, COutputComposite * pOutputComposite, COutputRFM * pOutputRFM);
#ifdef DVR_LIB_SUPPORT
    CTsb * dvrLibInitialize(void);
    void   dvrLibUninitialize(void);
#endif
#ifdef MPOD_SUPPORT
    CCablecard * cableCardInitialize(void);
    void         cableCardUninitialize(void);
#endif
#ifdef NETAPP_SUPPORT
    eRet               networkInitialize(void);
    void               networkUninitialize(void);
    void               bluetoothDestroy(CBluetooth * pBluetooth);
    CBluetoothRemote * bluetoothRemoteInitialize(void);
    void               bluetoothRemoteUninitialize(void);
    CBluetooth *       bluetoothInitialize(void);
    void               bluetoothUninitialize(void);
#endif /* ifdef NETAPP_SUPPORT */
#ifdef PLAYBACK_IP_SUPPORT
    eRet ipServerInitialize(void);
    void ipServerUninitialize(void);
    eRet ipServerStart(void);
    eRet ipServerStop(void);
#endif /* ifdef PLAYBACK_IP_SUPPORT */

#ifdef ESTB_CFG_SUPPORT
    void estbInitialize(void);
    void estbUninitialize(void);
#endif
    void mvcRelationshipsInitialize(CConfig * pConfig);
    void mvcRelationshipsUninitialize(void);

protected:
    CConfig *         _pConfig;
    CConfiguration *  _pCfg;
    CBoardResources * _pBoardResources;
    CWidgetEngine *   _pWidgetEngine;
    uint16_t          _number;
    eAtlasMode        _mode;
    CChannelMgr *     _pChannelMgr;
#if DVR_LIB_SUPPORT
    CDvrMgr * _pDvrMgr;
#endif
    CLua *        _pLua;
    CControl *    _pControl;
    CModel        _model;
    CScreen *     _pBackgroundScreen;
    CScreenMain * _pMainScreen;
};

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_MAIN_H__ */