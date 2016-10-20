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

#ifndef DISPLAY_H__
#define DISPLAY_H__

#include "video_window.h"
#include "output.h"
#include "graphics.h"
#include "bwidgets.h"

#define UINT32_C(c)  c ## U
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

class CDisplayVbiData
{
public:
    CDisplayVbiData() :
        bTeletext(false),
        bClosedCaptions(true),
        bWss(false),
        bCgms(false),
        bVps(false),
        bGemstar(false),
        bAmol(false),
        bMacrovision(false),
        bDcs(false),
        amolType(NEXUS_AmolType_eMax),
        macrovisionType(NEXUS_DisplayMacrovisionType_eNone),
        dcsType(NEXUS_DisplayDcsType_eOff) {}
public:
    bool bTeletext;       /* Encode teletext data in the VEC */
    bool bClosedCaptions; /* Encode closed caption data in the VEC */
    bool bWss;            /* Encode WSS data in the VEC */
    bool bCgms;           /* Encode CGMS data in the VEC */
    bool bVps;            /* Encode VPS data in the VEC. Only supported for PAL and SECAM. */
    bool bGemstar;        /* Encode Gemstar data to the VEC. */
    bool bAmol;           /* Encode AMOL data to the VEC. */
    bool bMacrovision;    /* Enable macrovision in the VEC */
    bool bDcs;            /* Enable dcs in the VEC */

    NEXUS_AmolType               amolType;
    NEXUS_DisplayMacrovisionType macrovisionType;
    NEXUS_DisplayDcsType         dcsType;
};

class CDisplay : public CResource
{
public:
    CDisplay(
            const char *     name,
            const uint16_t   number,
            CConfiguration * pCfg
            );
    ~CDisplay(void);

    virtual eRet                         open(void);
    virtual void                         close(void);
    virtual eRet                         setContentMode(NEXUS_VideoWindowContentMode contentMode);
    virtual NEXUS_VideoWindowContentMode getContentMode(void);
    virtual eRet                         setColorSpace(NEXUS_ColorSpace colorSpace);
    virtual eRet                         setPreferredColorSpace(NEXUS_VideoFormat format);
    virtual eRet                         setMpaaDecimation(bool bMpaaDecimation);
    virtual eRet                         setDeinterlacer(bool bDeinterlacer);
    virtual eRet                         setBoxDetect(bool bBoxDetect);
    virtual NEXUS_DisplayAspectRatio     getAspectRatio(void);
    virtual eRet                         setAspectRatio(NEXUS_DisplayAspectRatio aspectRatio);
    NEXUS_VideoFormat                    validateFormat(NEXUS_VideoFormat format);
    virtual eRet                         setFormat(NEXUS_VideoFormat format, CGraphics * pGraphics = NULL, bool bNotify = true);
    virtual NEXUS_VideoFormat            getFormat(void);
    virtual eRet                         setUpdateMode(bool bAuto = true);
    virtual MRect                        getGraphicsGeometry(void);
    virtual eRet                         updateGraphicsGeometry(void);
    virtual bool                         isStandardDef(void);
    virtual eRet                         setVbiSettings(CDisplayVbiData * pSettings);
    virtual CDisplayVbiData              getVbiSettings(void);
    virtual eRet                         addOutput(COutput * pOutput);
    virtual eRet                         removeOutput(COutput * pOutput);
    virtual COutput *                    getOutput(eBoardResource outputType);
    virtual void                         dump();

    CVideoWindow *         checkoutVideoWindow(void);
    eRet                   checkinVideoWindow(CVideoWindow * pWindow = NULL);
    MList <CVideoWindow> * getVideoWindowList(void) { return(&_videoWindowList); }
    eRet                   updateVideoWindowGeometry(void);
    eRet                   enableOutputs(bool bEnable);
    MList <COutput> *      getOutputList(void) { return(&_outputList); }
    NEXUS_DisplayHandle    getDisplay(void)    { return(_display); }
    eRet                   setFramebuffer(NEXUS_SurfaceHandle surface);
    void                   setDefaultFormat(NEXUS_VideoFormat videoFormat) { _defaultFormat = videoFormat; }
    NEXUS_VideoFormat      getDefaultFormat(void)                          { return(_defaultFormat); }
    NEXUS_VideoFormat      getMaxFormat(void);
    MRect                  getMaxGraphicsGeometry(void)       { return(_rectMaxGraphicsGeometry); }
    void                   setMaxGraphicsGeometry(MRect rect) { _rectMaxGraphicsGeometry = rect; }
    uint16_t               getNumVideoWindows(void);
    bool                   hasGraphics(void);
    bool                   isSupported(void);
    bool                   isReservedForEncoder(void);
    void                   setAutoFormat(bool bAutoFormat) { _bAutoFormat = bAutoFormat; notifyObservers(eNotify_AutoVideoFormatChanged, &bAutoFormat); }
    bool                   isAutoFormat(void)              { return(_bAutoFormat); }
    void                   setModel(CModel * pModel)       { _pModel = pModel; }
    bool                   isMacrovisionCompatible(NEXUS_VideoFormat format = NEXUS_VideoFormat_eUnknown);
    void                   validateMacrovision(NEXUS_VideoFormat format);

protected:
    NEXUS_DisplayHandle      _display;
    MAutoList <CVideoWindow> _videoWindowList;
    MList <COutput>          _outputList;
    bool                     _bAutoFormat;
    NEXUS_VideoFormat        _defaultFormat;
    CModel *                 _pModel;
    MRect                    _rectMaxGraphicsGeometry;
    bool                     _bOutputsEnabled;
    CDisplayVbiData          _vbiSettings;
};

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H__ */