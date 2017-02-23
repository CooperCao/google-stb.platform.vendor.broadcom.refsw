/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifdef DCC_SUPPORT

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "closed_caption.h"
#include "bwin.h"
#include "budp_dccparse.h"

BDBG_MODULE(closed_caption);

#define SCALE_720P_FONTS

static void loadFonts(
        BDCC_WINLIB_Handle WindowLib,
        int                index
        );

#ifdef FREETYPE_SUPPORT
uint32_t gScaleFactor[4] = { 100, 100, 100, 100 };
#elif defined (SCALE_480I_FONTS)
uint32_t gScaleFactor[4] = { 100, 100, 150, 225 };
#elif defined (SCALE_720P_FONTS)
uint32_t gScaleFactor[4] = { 66, 66, 100, 150 };
#else /* ifdef FREETYPE_SUPPORT */
uint32_t gScaleFactor[4] = { 100, 100, 100, 100 };
#endif /* ifdef FREETYPE_SUPPORT */

static digitalCC_setting dcc_setting;
NEXUS_SurfaceHandle      g_framebuffer1;

static void * mainLoop(void * param)
{
    digitalCC * dcc = (digitalCC *)param;
    uint32_t    numEntries, numValidEntries, i;
    /* coverity[stack_use_local_overflow] */
    NEXUS_ClosedCaptionData captionData[CC_DATA_BUF_SIZE];
    CCTest_Caption_Triplets cc708UserData[CC_DATA_BUF_SIZE];
    BERR_Code               err = BERR_SUCCESS;

    /*
    ** Now loop reading PictureUser Data queued by the VBI ISR and feeding it to the CC engine.
    */
    while (!dcc->exit)
    {
        /*
        ** Wait for new CC data or "x" milliseconds.
        ** The timeout allows B_Dcc_Process() t/o be called on a periodic
        ** basis, which in turn drives the time dependent actions (scrolling, flashing).
        */
        err = BKNI_WaitForEvent(dcc->ccDataEvent, 10);
        if ((BERR_SUCCESS != err) && (BERR_TIMEOUT != err))
        {
            BDBG_ERR(("BKNI_WaitForEvent error code %d", err));
        }

        buserdata_parse(dcc->userdata, dcc->videodecoder, captionData, CC_DATA_BUF_SIZE, &numEntries);

        BKNI_AcquireMutex(dcc->mutex);
        for (i = 0, numValidEntries = 0; i < numEntries; i++)
        {
            if ((dcc->setting.automode == true) && ((captionData[i].field & 0x2) == 2) &&
                (dcc->setting.ccMode != B_Dcc_Type_e708))
            {
                dcc->setting.ccMode = dcc_setting.ccMode = B_Dcc_Type_e708;
                BDBG_MSG((" AUTO CC mode %d CC service %d", dcc_setting.ccMode, dcc_setting.ccService));
                B_Dcc_Reset(dcc->ccEngine, NULL, dcc->setting.ccMode, dcc->setting.ccService);
            }
            cc708UserData[numValidEntries].ccType  = (uint8_t)(captionData[i].field);
            cc708UserData[numValidEntries].data[0] = captionData[i].data[0];
            cc708UserData[numValidEntries].data[1] = captionData[i].data[1];
            numValidEntries++;
        }

        /*
        ** Call  B_Dcc_Process() even if new data is not available.
        ** This drives the time depending events; scrolling, flashing.
        */

        if (B_Dcc_Process(dcc->ccEngine, (unsigned char *)cc708UserData, numValidEntries))
        {
            BDBG_ERR(("B_Dcc_Process returned with an Error\n"));
            goto error;
        }
        BKNI_ReleaseMutex(dcc->mutex);
    }

error:
    BKNI_ReleaseMutex(dcc->mutex);

    return(NULL);
} /* mainLoop */

CClosedCaption::CClosedCaption() :
    CMvcModel("closedCaption"),
    _pDisplay(NULL),
    _dcc(NULL),
    _pSurfaceClientDcc(NULL),
    _pBoardResources(NULL),
    _id(0)
{
}

void CClosedCaption::dcc_reset()
{
    BKNI_AcquireMutex(_dcc->mutex);
    BDBG_MSG((" DCC reset"));
    if (_dcc->setting.automode == true)
    {
        _dcc->setting.ccMode = dcc_setting.ccMode = B_Dcc_Type_e608;
    }
    B_Dcc_Reset(_dcc->ccEngine, NULL, _dcc->setting.ccMode, _dcc->setting.ccService);
    BKNI_ReleaseMutex(_dcc->mutex);
}

digitalCC_setting * CClosedCaption::dcc_getsetting(void)
{
    return(&dcc_setting);
}

static void dcc_userdata(
        void * context,
        int    param
        )
{
    digitalCC * dcc = (digitalCC *)context;

    BSTD_UNUSED(param);

    /* notify the 708 data servicing thread */
    BKNI_SetEvent(dcc->ccDataEvent);
}

static void dcc_config(
        digitalCC *              dcc,
        NEXUS_VideoFormat        format,
        NEXUS_DisplayAspectRatio aspectRatio
        )
{
    B_Dcc_Settings        ccEngineSettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    int                   index;

    NEXUS_VideoFormat_GetInfo(format, &videoFormatInfo);
    if (videoFormatInfo.height <= 576)
    {
        index = 0;
    }
    else
    if (videoFormatInfo.height <= 720)
    {
        index = 2;
    }
    else
    {
        index = 3;
    }
    index = 0;
#if !defined (SCALE_480I_FONTS) && !defined (SCALE_720P_FONTS)
    loadFonts(dcc->winlib, index);
#endif

    B_Dcc_GetDefaultSettings(&ccEngineSettings);
    ccEngineSettings.iSafeTitleX        = videoFormatInfo.width/ 10;  /* 20% of width / 2 according to CEB-10 */
    ccEngineSettings.iSafeTitleY        = videoFormatInfo.height/ 10; /* 20% of height / 2 according to CEB-10 */
    ccEngineSettings.ScaleFactor        = gScaleFactor[index];
    ccEngineSettings.Columns            = (aspectRatio == NEXUS_DisplayAspectRatio_e16x9) ? 42 : 32;  /* 32 for 4:3 42 for 16:9 */
    ccEngineSettings.uiTimeOutMilliSecs = 10000;
    B_Dcc_SetSettings(dcc->ccEngine, &ccEngineSettings);
} /* dcc_config */

int CClosedCaption::dcc_set(digitalCC_setting * setting)
{
    BKNI_AcquireMutex(_dcc->mutex);

    if (_dcc->setting.enabled != setting->enabled)
    {
        setting->enabled ? setting->dispOn = true : setting->dispOn = false;
        _dcc->setting.enabled              = setting->enabled;
        BDBG_MSG((" CC enabled = %d", setting->enabled));
    }

    /*
    ** A new caption mode or service has been specified.
    */
    if ((_dcc->setting.ccService != setting->ccService) || (_dcc->setting.ccMode != setting->ccMode))
    {
        _dcc->setting.ccService = setting->ccService;
        _dcc->setting.ccMode    = setting->ccMode;
        BDBG_MSG((" CC mode %d CC service %d", setting->ccMode, setting->ccService));
        B_Dcc_Reset(_dcc->ccEngine, NULL, _dcc->setting.ccMode, _dcc->setting.ccService);
    }

    /*
    ** Display has been switched on or off.
    */
    if (_dcc->setting.dispOn != setting->dispOn)
    {
        _dcc->setting.dispOn = setting->dispOn;

        if (!_dcc->setting.dispOn)
        {
            BDBG_MSG((" CC display is off"));
            B_Dcc_Reset(_dcc->ccEngine, NULL, _dcc->setting.ccMode, _dcc->setting.ccService);
            _dcc->winlibInterface.HideDisp(_dcc->winlib, true);
        }

        if (_dcc->setting.dispOn)
        {
            BDBG_MSG((" CC display is on"));
            _dcc->winlibInterface.HideDisp(_dcc->winlib, false);
        }
    }

    /*
    ** An override has been requested
    */
    if ((_dcc->setting.overrideMask != setting->overrideMask) || memcmp(&_dcc->setting.overrideValues, &setting->overrideValues, sizeof(B_Dcc_OverRides)))
    {
        BDBG_MSG((" CC overrideMask or orverrideValues changed"));
        _dcc->setting.overrideMask = setting->overrideMask;
        memcpy(&_dcc->setting.overrideValues, &setting->overrideValues, sizeof(B_Dcc_OverRides));
        B_Dcc_Override(_dcc->ccEngine, _dcc->setting.overrideMask, &_dcc->setting.overrideValues);
    }

    if ((_dcc->setting.format != setting->format) || (_dcc->setting.aspectRatio != setting->aspectRatio))
    {
        BDBG_MSG((" CC format or aspectRatio changed"));
        _dcc->setting.format      = setting->format;
        _dcc->setting.aspectRatio = setting->aspectRatio;
        dcc_config(_dcc, _dcc->setting.format, _dcc->setting.aspectRatio);
        B_Dcc_Reset(_dcc->ccEngine, NULL, _dcc->setting.ccMode, _dcc->setting.ccService);
    }
    if ((_dcc->setting.width != setting->width) || (_dcc->setting.height != setting->height))
    {
        BDBG_MSG((" CC width or height changed"));
        _dcc->setting.width  = setting->width;
        _dcc->setting.height = setting->height;
        _dcc->winlibInterface.SetFrameBufferSize(_dcc->winlib, setting->width, setting->height);
    }

    BKNI_ReleaseMutex(_dcc->mutex);

    return(0);
} /* dcc_set */

static NEXUS_SurfaceHandle client_flip(void * context)
{
    eRet             ret               = eRet_Ok;
    CSurfaceClient * pSurfaceClientDcc = (CSurfaceClient *)context;

    BDBG_ASSERT(NULL != g_framebuffer1);
    BDBG_ASSERT(NULL != pSurfaceClientDcc);

    ret = pSurfaceClientDcc->updateSurface();
    CHECK_ERROR_GOTO("unable to update DCC surface client", ret, error);

error:
    return(g_framebuffer1); /* same framebuffer. SurfaceCompositor will copy. */
}                           /* client_flip */

static void client_get_framebuffer_dimensions(
        void *              context,
        NEXUS_VideoFormat * format,
        unsigned *          width,
        unsigned *          height
        )
{
    BSTD_UNUSED(context);
    BDBG_ASSERT(NULL != g_framebuffer1);

    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_Surface_GetCreateSettings(g_framebuffer1, &surfaceCreateSettings);
    *width  = surfaceCreateSettings.width;
    *height = surfaceCreateSettings.height;
    *format = NEXUS_VideoFormat_eNtsc;
}

eRet CClosedCaption::dcc_init(
        CConfig * pConfig,
        CModel *  pModel
        )
{
    eRet                         ret          = eRet_Ok;
    NEXUS_Error                  nerror       = NEXUS_SUCCESS;
    CGraphics *                  pGraphics    = NULL;
    CSimpleVideoDecode *         pVideoDecode = NULL;
    digitalCC *                  dcc;
    NEXUS_VideoDecoderSettings   videoDecoderSettings;
    NEXUS_VideoFormatInfo        videoFormatInfo;
    NEXUS_SurfaceCreateSettings  surfaceSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    int index;

    BDBG_ASSERT(NULL != pConfig);
    BDBG_ASSERT(NULL != pModel);

    _pBoardResources = pConfig->getBoardResources();
    pGraphics        = pModel->getGraphics();
    pVideoDecode     = pModel->getSimpleVideoDecode();
    _pDisplay        = pGraphics->getDisplay();

    dcc = _dcc = (digitalCC *)BKNI_Malloc(sizeof(digitalCC));
    BKNI_Memset(dcc, 0, sizeof(digitalCC));
    dcc->videodecoder = pVideoDecode->getSimpleDecoder();

    /* create framebuffer surface for dcc with same properties as main atlas surface */
    {
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceSettings);
        surfaceSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        surfaceSettings.width       = pGraphics->getWidth();
        surfaceSettings.height      = pGraphics->getHeight();
        /* surfaceSettings.heap        = NEXUS_Platform_GetFramebufferHeap(0); */

        g_framebuffer1 = NEXUS_Surface_Create(&surfaceSettings);
        BDBG_MSG(("creating g_framebuffer1 w=%d h=%d", surfaceSettings.width, surfaceSettings.height));
        CHECK_PTR_ERROR_GOTO("surface create for ccClient failed.", g_framebuffer1, ret, eRet_ExternalError, error);

        /* fill ccSurface with same properties as main atlas surface */
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = g_framebuffer1;
        fillSettings.color   = 0x0;
        nerror               = NEXUS_Graphics2D_Fill(pGraphics->getBlitter(), &fillSettings);
        CHECK_NEXUS_ERROR_GOTO("graphics fill for g_framebuffer1 error", ret, nerror, error);
    }

    /* create surface client */
    {
        /* BLENDING_TYPE_SRC_OVER_NON_PREMULTIPLIED */
        NEXUS_BlendEquation alphaBlendEquation = {
            NEXUS_BlendFactor_eSourceAlpha,
            NEXUS_BlendFactor_eOne,
            false,
            NEXUS_BlendFactor_eDestinationAlpha,
            NEXUS_BlendFactor_eInverseSourceAlpha,
            false,
            NEXUS_BlendFactor_eZero
        };
        /* BLENDING_TYPE_SRC_OVER_NON_PREMULTIPLIED */
        NEXUS_BlendEquation colorBlendEquation = {
            NEXUS_BlendFactor_eSourceColor,
            NEXUS_BlendFactor_eSourceAlpha,
            false,
            NEXUS_BlendFactor_eDestinationColor,
            NEXUS_BlendFactor_eInverseSourceAlpha,
            false,
            NEXUS_BlendFactor_eZero
        };

        _pSurfaceClientDcc = (CSurfaceClient *)_pBoardResources->checkoutResource(_id, eBoardResource_surfaceClient);
        CHECK_PTR_ERROR_GOTO("unable to checkout digital closed captions surface client resource", _pSurfaceClientDcc, ret, eRet_NotAvailable, error);

        ret = _pSurfaceClientDcc->open(pGraphics->getSurfaceCompositor());
        CHECK_ERROR_GOTO("unable to open digital closed captions surface client", ret, error);

        ret = _pSurfaceClientDcc->setBlend(&alphaBlendEquation, &colorBlendEquation);
        CHECK_ERROR_GOTO("unable to set blend equations for DCC surface client", ret, error);
        ret = _pSurfaceClientDcc->setZOrder(0); /* lower close caption surface below atlas surface */
        CHECK_ERROR_GOTO("unable to set z-order for dcc surface client", ret, error);
        ret = _pSurfaceClientDcc->setSurface(g_framebuffer1);
        CHECK_ERROR_GOTO("unable to set surface with digital close captions surface client", ret, error);
    }

    BDCC_WINLIB_GetDefaultOpenSettings(&dcc->openSettings);
    dcc->openSettings.flip                       = client_flip;
    dcc->openSettings.context                    = _pSurfaceClientDcc;
    dcc->openSettings.get_framebuffer_dimensions = client_get_framebuffer_dimensions;
    dcc->openSettings.bwinEngine                 = pGraphics->getWinEngine();
    dcc->openSettings.framebufferWidth           = surfaceSettings.width;
    dcc->openSettings.framebufferHeight          = surfaceSettings.height;

    /*
    ** Initialize the Closed Caption window library.
    ** This library interfaces to bwin
    */
    if (BDCC_WINLIB_Open(&dcc->winlib, &dcc->openSettings))
    {
        BDBG_ERR(("Winlib Returned with an Error"));
        goto error;
    }

    /* get the winlib function table */
    BDCC_WINLIB_GetInterface(&dcc->winlibInterface);

    dcc->setting.format      = _pDisplay->getFormat();
    dcc->setting.aspectRatio = _pDisplay->getAspectRatio();

    NEXUS_VideoFormat_GetInfo(dcc->setting.format, &videoFormatInfo);
    if (videoFormatInfo.height <= 576)
    {
        index = 0;
    }
    else
    if (videoFormatInfo.height <= 720)
    {
        index = 2;
    }
    else
    {
        index = 3;
    }

    index = 0;
    loadFonts(dcc->winlib, index);
    dcc->winlibInterface.HideDisp(dcc->winlib, true);

    if (B_Dcc_Open(&dcc->ccEngine, dcc->winlib, &dcc->winlibInterface))
    {
        BDBG_ERR(("B_Dcc_Open  Returned with an Error\n"));
        goto error;
    }

    dcc_config(dcc, dcc->setting.format, dcc->setting.aspectRatio);

    dcc->setting.automode               = false;
    dcc->setting.ccMode                 = B_Dcc_Type_e608;
    dcc->setting.ccService              = 1;
    dcc->setting.dispOn                 = false;
    dcc->setting.enabled                = false;
    dcc->setting.overrideValues.PenSize = BDCC_PenSize_Standard;
    BKNI_Memcpy(&dcc_setting, &dcc->setting, sizeof(digitalCC_setting));

    if (B_Dcc_Init(dcc->ccEngine, dcc->setting.ccService, dcc->setting.ccMode))
    {
        BDBG_ERR(("B_Dcc_Init  Returned with an Error\n"));
        goto error;
    }

    BKNI_CreateEvent(&dcc->ccDataEvent);

    NEXUS_SimpleVideoDecoder_GetSettings(dcc->videodecoder, &videoDecoderSettings);
    videoDecoderSettings.userDataEnabled           = true;
    videoDecoderSettings.appUserDataReady.callback = dcc_userdata;
    videoDecoderSettings.appUserDataReady.context  = dcc;
    NEXUS_SimpleVideoDecoder_SetSettings(dcc->videodecoder, &videoDecoderSettings);

    dcc->userdata = buserdata_create(NULL);
    BKNI_CreateMutex(&dcc->mutex);
    pthread_create(&dcc->main_thread, NULL, (void *(*)(void *))(mainLoop), (void *)dcc);

    goto done;
error:
    FRE(dcc);
done:
    return(ret);
} /* dcc_init */

void CClosedCaption::dcc_uninit()
{
    eRet ret = eRet_Ok;

    if (_dcc == NULL) { return; }
    _dcc->exit = true;

    BKNI_SetEvent(_dcc->ccDataEvent);
    pthread_join(_dcc->main_thread, NULL);

    buserdata_destroy(_dcc->userdata);
    if (_dcc->ccEngine) { B_Dcc_Close(_dcc->ccEngine); }
    if (_dcc->winlib) { BDCC_WINLIB_UnloadFonts(_dcc->winlib); }
    if (_dcc->winlib) { BDCC_WINLIB_Close(_dcc->winlib); }

    BKNI_DestroyEvent(_dcc->ccDataEvent);
    BKNI_DestroyMutex(_dcc->mutex);
    BKNI_Free(_dcc);

    if (NULL != _pSurfaceClientDcc)
    {
        _pSurfaceClientDcc->close();
        ret = _pBoardResources->checkinResource(_pSurfaceClientDcc);
        CHECK_ERROR("unable to checkin digital closed captions surface client resource", ret);

        _pSurfaceClientDcc = NULL;
    }
    if (NULL != g_framebuffer1)
    {
        NEXUS_Surface_Destroy(g_framebuffer1);
        g_framebuffer1 = NULL;
    }
} /* dcc_uninit */

/*
** Illustrates how to load fonts and associate them with a specific pen "font style".
*/
static void loadFonts(
        BDCC_WINLIB_Handle WindowLib,
        int                index
        )
{
    int                    i, j, iErr;
    BDCC_FONT_DESCRIPTOR * pFont;
    BDCC_FONT_DESCRIPTOR   standardFonts[BDCC_FontStyle_Max_Value][BDCC_PenSize_Max_Size];
    BDCC_FONT_DESCRIPTOR   italicsFonts [BDCC_FontStyle_Max_Value][BDCC_PenSize_Max_Size];
    uint32_t               scaleFactor;

#ifdef FREETYPE_SUPPORT

    const char * pszFontFilenames[BDCC_FontStyle_Max_Value] =
    {
        "fonts/cc_fonts/cinecavB_type_03.ttf",   /* BDCC_FontStyle_Default    */
        "fonts/cc_fonts/cinecavB_type_03.ttf",   /* BDCC_FontStyle_MonoSerifs */
        "fonts/cc_fonts/cinecavB_serif_02.ttf",  /* BDCC_FontStyle_PropSerifs */
        "fonts/cc_fonts/cinecavB_mono_03.ttf",   /* BDCC_FontStyle_Mono       */
        "fonts/cc_fonts/cinecavB_sans_02.ttf",   /* BDCC_FontStyle_Prop       */
        "fonts/cc_fonts/cinecavB_casual_02.ttf", /* BDCC_FontStyle_Casual     */
        "fonts/cc_fonts/cinecavB_script_02.ttf", /* BDCC_FontStyle_Cursive    */
        "fonts/cc_fonts/cinecavB_sc_02.ttf"      /* BDCC_FontStyle_SmallCaps  */
    };

    int iFontSize[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /* 480i S M L      480p S M L      720p S M L     1080i S M L */

        { { 14, 18, 22 }, { 14, 18, 22 }, { 21, 28, 33 }, { 32, 41, 49 } }, /* BDCC_FontStyle_Default */
        { { 14, 18, 22 }, { 14, 18, 22 }, { 21, 28, 33 }, { 32, 41, 49 } }, /* BDCC_FontStyle_MonoSerifs */
        { { 12, 15, 19 }, { 12, 15, 19 }, { 18, 23, 29 }, { 28, 35, 44 } }, /* BDCC_FontStyle_PropSerifs */
        { { 14, 18, 22 }, { 14, 18, 22 }, { 21, 29, 34 }, { 34, 43, 52 } }, /* BDCC_FontStyle_Mono       */
        { { 12, 15, 19 }, { 12, 15, 19 }, { 18, 23, 29 }, { 28, 35, 44 } }, /* BDCC_FontStyle_Prop       */
        { { 13, 16, 20 }, { 13, 16, 20 }, { 19, 24, 30 }, { 30, 36, 45 } }, /* BDCC_FontStyle_Casual     */
        { { 12, 14, 18 }, { 12, 14, 18 }, { 16, 18, 25 }, { 25, 28, 38 } }, /* BDCC_FontStyle_Cursive    */
        { { 13, 15, 19 }, { 13, 15, 19 }, { 18, 23, 30 }, { 28, 35, 45 } }  /* BDCC_FontStyle_SmallCaps  */
    };

    /* we do not need to scale these fonts using the m2mc since the freetype engine can do it */
    scaleFactor = 100; /* scale factor in hundredths i.e. scale factor is 1.0 */

    BDBG_MSG(("USING TRUE-TYPE FONTS\n"));

#else /* ifdef FREETYPE_SUPPORT */

    /* The following is a trade-off for pre-rendered fonts.
    **
    ** We can use a base set of 14 pre-rendered fonts (7 normal and 7 italic)
    ** and generate the rest using the scalers but the scaler will reduce the font's quality
    ** or we can use a full set of 126 fonts to cover all of the font permutations with maximum quality
    */
#ifdef SCALE_480I_FONTS

    const char * pszStandardFontFilenames[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /*
         * non-italic
         * 480i S M L                                                                          480p S M L                                                           720p S M L                                   1080i S M L
         */

        { { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   } }, /* BDCC_FontStyle_Default */
        { { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   } }, /* BDCC_FontStyle_MonoSerifs */
        { { "fonts/cc_fonts/cinecavB12_serif.bwin_font",  "fonts/cc_fonts/cinecavB15_serif.bwin_font",  "fonts/cc_fonts/cinecavB19_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12_serif.bwin_font",  "fonts/cc_fonts/cinecavB15_serif.bwin_font",  "fonts/cc_fonts/cinecavB19_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12_serif.bwin_font",  "fonts/cc_fonts/cinecavB15_serif.bwin_font",  "fonts/cc_fonts/cinecavB19_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12_serif.bwin_font",  "fonts/cc_fonts/cinecavB15_serif.bwin_font",  "fonts/cc_fonts/cinecavB19_serif.bwin_font"  } }, /* BDCC_FontStyle_PropSerifs */
        { { "fonts/cc_fonts/cinecavB14_mono.bwin_font",   "fonts/cc_fonts/cinecavB18_mono.bwin_font",   "fonts/cc_fonts/cinecavB22_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_mono.bwin_font",   "fonts/cc_fonts/cinecavB18_mono.bwin_font",   "fonts/cc_fonts/cinecavB22_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_mono.bwin_font",   "fonts/cc_fonts/cinecavB18_mono.bwin_font",   "fonts/cc_fonts/cinecavB22_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_mono.bwin_font",   "fonts/cc_fonts/cinecavB18_mono.bwin_font",   "fonts/cc_fonts/cinecavB22_mono.bwin_font"   } }, /* BDCC_FontStyle_Mono  */
        { { "fonts/cc_fonts/cinecavB12_sans.bwin_font",   "fonts/cc_fonts/cinecavB15_sans.bwin_font",   "fonts/cc_fonts/cinecavB19_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12_sans.bwin_font",   "fonts/cc_fonts/cinecavB15_sans.bwin_font",   "fonts/cc_fonts/cinecavB19_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12_sans.bwin_font",   "fonts/cc_fonts/cinecavB15_sans.bwin_font",   "fonts/cc_fonts/cinecavB19_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12_sans.bwin_font",   "fonts/cc_fonts/cinecavB15_sans.bwin_font",   "fonts/cc_fonts/cinecavB19_sans.bwin_font"   } }, /* BDCC_FontStyle_Prop  */
        { { "fonts/cc_fonts/cinecavB13_casual.bwin_font", "fonts/cc_fonts/cinecavB16_casual.bwin_font", "fonts/cc_fonts/cinecavB20_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13_casual.bwin_font", "fonts/cc_fonts/cinecavB16_casual.bwin_font", "fonts/cc_fonts/cinecavB20_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13_casual.bwin_font", "fonts/cc_fonts/cinecavB16_casual.bwin_font", "fonts/cc_fonts/cinecavB20_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13_casual.bwin_font", "fonts/cc_fonts/cinecavB16_casual.bwin_font", "fonts/cc_fonts/cinecavB20_casual.bwin_font" } }, /* BDCC_FontStyle_Casual  */
        { { "fonts/cc_fonts/cinecavB12_script.bwin_font", "fonts/cc_fonts/cinecavB14_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12_script.bwin_font", "fonts/cc_fonts/cinecavB14_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12_script.bwin_font", "fonts/cc_fonts/cinecavB14_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12_script.bwin_font", "fonts/cc_fonts/cinecavB14_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font" } }, /* BDCC_FontStyle_Cursive */
        { { "fonts/cc_fonts/cinecavB13_sc.bwin_font",     "fonts/cc_fonts/cinecavB15_sc.bwin_font",     "fonts/cc_fonts/cinecavB19_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13_sc.bwin_font",     "fonts/cc_fonts/cinecavB15_sc.bwin_font",     "fonts/cc_fonts/cinecavB19_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13_sc.bwin_font",     "fonts/cc_fonts/cinecavB15_sc.bwin_font",     "fonts/cc_fonts/cinecavB19_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13_sc.bwin_font",     "fonts/cc_fonts/cinecavB15_sc.bwin_font",     "fonts/cc_fonts/cinecavB19_sc.bwin_font"     } }  /* BDCC_FontStyle_SmallCaps */
    };

    const char * pszItalicFontFilenames[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /* italic */

        { { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   } }, /* BDCC_FontStyle_Default */
        { { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   } }, /* BDCC_FontStyle_MonoSerifs */
        { { "fonts/cc_fonts/cinecavB12i_serif.bwin_font",  "fonts/cc_fonts/cinecavB15i_serif.bwin_font",  "fonts/cc_fonts/cinecavB19i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12i_serif.bwin_font",  "fonts/cc_fonts/cinecavB15i_serif.bwin_font",  "fonts/cc_fonts/cinecavB19i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12i_serif.bwin_font",  "fonts/cc_fonts/cinecavB15i_serif.bwin_font",  "fonts/cc_fonts/cinecavB19i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12i_serif.bwin_font",  "fonts/cc_fonts/cinecavB15i_serif.bwin_font",  "fonts/cc_fonts/cinecavB19i_serif.bwin_font"  } }, /* BDCC_FontStyle_PropSerifs */
        { { "fonts/cc_fonts/cinecavB14i_mono.bwin_font",   "fonts/cc_fonts/cinecavB18i_mono.bwin_font",   "fonts/cc_fonts/cinecavB22i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_mono.bwin_font",   "fonts/cc_fonts/cinecavB18i_mono.bwin_font",   "fonts/cc_fonts/cinecavB22i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_mono.bwin_font",   "fonts/cc_fonts/cinecavB18i_mono.bwin_font",   "fonts/cc_fonts/cinecavB22i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_mono.bwin_font",   "fonts/cc_fonts/cinecavB18i_mono.bwin_font",   "fonts/cc_fonts/cinecavB22i_mono.bwin_font"   } }, /* BDCC_FontStyle_Mono  */
        { { "fonts/cc_fonts/cinecavB12i_sans.bwin_font",   "fonts/cc_fonts/cinecavB15i_sans.bwin_font",   "fonts/cc_fonts/cinecavB19i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12i_sans.bwin_font",   "fonts/cc_fonts/cinecavB15i_sans.bwin_font",   "fonts/cc_fonts/cinecavB19i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12i_sans.bwin_font",   "fonts/cc_fonts/cinecavB15i_sans.bwin_font",   "fonts/cc_fonts/cinecavB19i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12i_sans.bwin_font",   "fonts/cc_fonts/cinecavB15i_sans.bwin_font",   "fonts/cc_fonts/cinecavB19i_sans.bwin_font"   } }, /* BDCC_FontStyle_Prop  */
        { { "fonts/cc_fonts/cinecavB13i_casual.bwin_font", "fonts/cc_fonts/cinecavB16i_casual.bwin_font", "fonts/cc_fonts/cinecavB20i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13i_casual.bwin_font", "fonts/cc_fonts/cinecavB16i_casual.bwin_font", "fonts/cc_fonts/cinecavB20i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13i_casual.bwin_font", "fonts/cc_fonts/cinecavB16i_casual.bwin_font", "fonts/cc_fonts/cinecavB20i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13i_casual.bwin_font", "fonts/cc_fonts/cinecavB16i_casual.bwin_font", "fonts/cc_fonts/cinecavB20i_casual.bwin_font" } }, /* BDCC_FontStyle_Casual  */
        { { "fonts/cc_fonts/cinecavB12i_script.bwin_font", "fonts/cc_fonts/cinecavB14i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12i_script.bwin_font", "fonts/cc_fonts/cinecavB14i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12i_script.bwin_font", "fonts/cc_fonts/cinecavB14i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12i_script.bwin_font", "fonts/cc_fonts/cinecavB14i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font" } }, /* BDCC_FontStyle_Cursive */
        { { "fonts/cc_fonts/cinecavB13i_sc.bwin_font",     "fonts/cc_fonts/cinecavB15i_sc.bwin_font",     "fonts/cc_fonts/cinecavB19i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13i_sc.bwin_font",     "fonts/cc_fonts/cinecavB15i_sc.bwin_font",     "fonts/cc_fonts/cinecavB19i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13i_sc.bwin_font",     "fonts/cc_fonts/cinecavB15i_sc.bwin_font",     "fonts/cc_fonts/cinecavB19i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13i_sc.bwin_font",     "fonts/cc_fonts/cinecavB15i_sc.bwin_font",     "fonts/cc_fonts/cinecavB19i_sc.bwin_font"     } }  /* BDCC_FontStyle_SmallCaps */
    };

    /* we need to scale these fonts using the m2mc since we have specified the 480i/p set for all of the display formats */

    scaleFactor = gScaleFactor[index]; /* scale factor in hundredths i.e. scale factor is 1.0 */

    BDBG_MSG(("SCALING 480i FONTS\n"));

#else /* ifdef SCALE_480I_FONTS */

#ifdef SCALE_720P_FONTS

    const char * pszStandardFontFilenames[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /*
         * non-italic
         * 480i S M L                                                                          480p S M L                                                           720p S M L                                   1080i S M L
         */

        { { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   } }, /* BDCC_FontStyle_Default */
        { { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   } }, /* BDCC_FontStyle_MonoSerifs */
        { { "fonts/cc_fonts/cinecavB18_serif.bwin_font",  "fonts/cc_fonts/cinecavB23_serif.bwin_font",  "fonts/cc_fonts/cinecavB29_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18_serif.bwin_font",  "fonts/cc_fonts/cinecavB23_serif.bwin_font",  "fonts/cc_fonts/cinecavB29_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18_serif.bwin_font",  "fonts/cc_fonts/cinecavB23_serif.bwin_font",  "fonts/cc_fonts/cinecavB29_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18_serif.bwin_font",  "fonts/cc_fonts/cinecavB23_serif.bwin_font",  "fonts/cc_fonts/cinecavB29_serif.bwin_font"  } }, /* BDCC_FontStyle_PropSerifs */
        { { "fonts/cc_fonts/cinecavB21_mono.bwin_font",   "fonts/cc_fonts/cinecavB29_mono.bwin_font",   "fonts/cc_fonts/cinecavB34_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_mono.bwin_font",   "fonts/cc_fonts/cinecavB29_mono.bwin_font",   "fonts/cc_fonts/cinecavB34_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_mono.bwin_font",   "fonts/cc_fonts/cinecavB29_mono.bwin_font",   "fonts/cc_fonts/cinecavB34_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_mono.bwin_font",   "fonts/cc_fonts/cinecavB29_mono.bwin_font",   "fonts/cc_fonts/cinecavB34_mono.bwin_font"   } }, /* BDCC_FontStyle_Mono  */
        { { "fonts/cc_fonts/cinecavB18_sans.bwin_font",   "fonts/cc_fonts/cinecavB23_sans.bwin_font",   "fonts/cc_fonts/cinecavB29_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18_sans.bwin_font",   "fonts/cc_fonts/cinecavB23_sans.bwin_font",   "fonts/cc_fonts/cinecavB29_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18_sans.bwin_font",   "fonts/cc_fonts/cinecavB23_sans.bwin_font",   "fonts/cc_fonts/cinecavB29_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18_sans.bwin_font",   "fonts/cc_fonts/cinecavB23_sans.bwin_font",   "fonts/cc_fonts/cinecavB29_sans.bwin_font"   } }, /* BDCC_FontStyle_Prop  */
        { { "fonts/cc_fonts/cinecavB19_casual.bwin_font", "fonts/cc_fonts/cinecavB24_casual.bwin_font", "fonts/cc_fonts/cinecavB30_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19_casual.bwin_font", "fonts/cc_fonts/cinecavB24_casual.bwin_font", "fonts/cc_fonts/cinecavB30_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19_casual.bwin_font", "fonts/cc_fonts/cinecavB24_casual.bwin_font", "fonts/cc_fonts/cinecavB30_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19_casual.bwin_font", "fonts/cc_fonts/cinecavB24_casual.bwin_font", "fonts/cc_fonts/cinecavB30_casual.bwin_font" } }, /* BDCC_FontStyle_Casual  */
        { { "fonts/cc_fonts/cinecavB16_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font", "fonts/cc_fonts/cinecavB25_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font", "fonts/cc_fonts/cinecavB25_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font", "fonts/cc_fonts/cinecavB25_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font", "fonts/cc_fonts/cinecavB25_script.bwin_font" } }, /* BDCC_FontStyle_Cursive */
        { { "fonts/cc_fonts/cinecavB18_sc.bwin_font",     "fonts/cc_fonts/cinecavB23_sc.bwin_font",     "fonts/cc_fonts/cinecavB30_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18_sc.bwin_font",     "fonts/cc_fonts/cinecavB23_sc.bwin_font",     "fonts/cc_fonts/cinecavB30_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18_sc.bwin_font",     "fonts/cc_fonts/cinecavB23_sc.bwin_font",     "fonts/cc_fonts/cinecavB30_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18_sc.bwin_font",     "fonts/cc_fonts/cinecavB23_sc.bwin_font",     "fonts/cc_fonts/cinecavB30_sc.bwin_font"     } }  /* BDCC_FontStyle_SmallCaps */
    };

    const char * pszItalicFontFilenames[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /* italic */

        { { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   } }, /* BDCC_FontStyle_Default */
        { { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   } }, /* BDCC_FontStyle_MonoSerifs */
        { { "fonts/cc_fonts/cinecavB18i_serif.bwin_font",  "fonts/cc_fonts/cinecavB23i_serif.bwin_font",  "fonts/cc_fonts/cinecavB29i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18i_serif.bwin_font",  "fonts/cc_fonts/cinecavB23i_serif.bwin_font",  "fonts/cc_fonts/cinecavB29i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18i_serif.bwin_font",  "fonts/cc_fonts/cinecavB23i_serif.bwin_font",  "fonts/cc_fonts/cinecavB29i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18i_serif.bwin_font",  "fonts/cc_fonts/cinecavB23i_serif.bwin_font",  "fonts/cc_fonts/cinecavB29i_serif.bwin_font"  } }, /* BDCC_FontStyle_PropSerifs */
        { { "fonts/cc_fonts/cinecavB21i_mono.bwin_font",   "fonts/cc_fonts/cinecavB29i_mono.bwin_font",   "fonts/cc_fonts/cinecavB34i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_mono.bwin_font",   "fonts/cc_fonts/cinecavB29i_mono.bwin_font",   "fonts/cc_fonts/cinecavB34i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_mono.bwin_font",   "fonts/cc_fonts/cinecavB29i_mono.bwin_font",   "fonts/cc_fonts/cinecavB34i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_mono.bwin_font",   "fonts/cc_fonts/cinecavB29i_mono.bwin_font",   "fonts/cc_fonts/cinecavB34i_mono.bwin_font"   } }, /* BDCC_FontStyle_Mono  */
        { { "fonts/cc_fonts/cinecavB18i_sans.bwin_font",   "fonts/cc_fonts/cinecavB23i_sans.bwin_font",   "fonts/cc_fonts/cinecavB29i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18i_sans.bwin_font",   "fonts/cc_fonts/cinecavB23i_sans.bwin_font",   "fonts/cc_fonts/cinecavB29i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18i_sans.bwin_font",   "fonts/cc_fonts/cinecavB23i_sans.bwin_font",   "fonts/cc_fonts/cinecavB29i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18i_sans.bwin_font",   "fonts/cc_fonts/cinecavB23i_sans.bwin_font",   "fonts/cc_fonts/cinecavB29i_sans.bwin_font"   } }, /* BDCC_FontStyle_Prop  */
        { { "fonts/cc_fonts/cinecavB19i_casual.bwin_font", "fonts/cc_fonts/cinecavB24i_casual.bwin_font", "fonts/cc_fonts/cinecavB30i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19i_casual.bwin_font", "fonts/cc_fonts/cinecavB24i_casual.bwin_font", "fonts/cc_fonts/cinecavB30i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19i_casual.bwin_font", "fonts/cc_fonts/cinecavB24i_casual.bwin_font", "fonts/cc_fonts/cinecavB30i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19i_casual.bwin_font", "fonts/cc_fonts/cinecavB24i_casual.bwin_font", "fonts/cc_fonts/cinecavB30i_casual.bwin_font" } }, /* BDCC_FontStyle_Casual  */
        { { "fonts/cc_fonts/cinecavB16i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font", "fonts/cc_fonts/cinecavB25i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font", "fonts/cc_fonts/cinecavB25i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font", "fonts/cc_fonts/cinecavB25i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font", "fonts/cc_fonts/cinecavB25i_script.bwin_font" } }, /* BDCC_FontStyle_Cursive */
        { { "fonts/cc_fonts/cinecavB18i_sc.bwin_font",     "fonts/cc_fonts/cinecavB23i_sc.bwin_font",     "fonts/cc_fonts/cinecavB30i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18i_sc.bwin_font",     "fonts/cc_fonts/cinecavB23i_sc.bwin_font",     "fonts/cc_fonts/cinecavB30i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18i_sc.bwin_font",     "fonts/cc_fonts/cinecavB23i_sc.bwin_font",     "fonts/cc_fonts/cinecavB30i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18i_sc.bwin_font",     "fonts/cc_fonts/cinecavB23i_sc.bwin_font",     "fonts/cc_fonts/cinecavB30i_sc.bwin_font"     } }  /* BDCC_FontStyle_SmallCaps */
    };

    /* we need to scale these fonts using the m2mc since we have specified the 720p set for all of the display formats */

    scaleFactor = gScaleFactor[index]; /* scale factor in hundredths i.e. scale factor is 1.0 */

    BDBG_MSG(("SCALING 720P FONTS\n"));

#else /*MAX_RENDERED_FONTS*/

    const char * pszStandardFontFilenames[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /*
         * non-italic
         * 480i S M L                                                                          480p S M L                                                           720p S M L                                   1080i S M L
         */

        { { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB32_type.bwin_font",   "fonts/cc_fonts/cinecavB41_type.bwin_font",   "fonts/cc_fonts/cinecavB49_type.bwin_font"   } }, /* BDCC_FontStyle_Default */
        { { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_type.bwin_font",   "fonts/cc_fonts/cinecavB18_type.bwin_font",   "fonts/cc_fonts/cinecavB22_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_type.bwin_font",   "fonts/cc_fonts/cinecavB28_type.bwin_font",   "fonts/cc_fonts/cinecavB33_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB32_type.bwin_font",   "fonts/cc_fonts/cinecavB41_type.bwin_font",   "fonts/cc_fonts/cinecavB49_type.bwin_font"   } }, /* BDCC_FontStyle_MonoSerifs */
        { { "fonts/cc_fonts/cinecavB12_serif.bwin_font",  "fonts/cc_fonts/cinecavB15_serif.bwin_font",  "fonts/cc_fonts/cinecavB19_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12_serif.bwin_font",  "fonts/cc_fonts/cinecavB15_serif.bwin_font",  "fonts/cc_fonts/cinecavB19_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18_serif.bwin_font",  "fonts/cc_fonts/cinecavB23_serif.bwin_font",  "fonts/cc_fonts/cinecavB29_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB28_serif.bwin_font",  "fonts/cc_fonts/cinecavB35_serif.bwin_font",  "fonts/cc_fonts/cinecavB44_serif.bwin_font"  } }, /* BDCC_FontStyle_PropSerifs */
        { { "fonts/cc_fonts/cinecavB14_mono.bwin_font",   "fonts/cc_fonts/cinecavB18_mono.bwin_font",   "fonts/cc_fonts/cinecavB22_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14_mono.bwin_font",   "fonts/cc_fonts/cinecavB18_mono.bwin_font",   "fonts/cc_fonts/cinecavB22_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21_mono.bwin_font",   "fonts/cc_fonts/cinecavB29_mono.bwin_font",   "fonts/cc_fonts/cinecavB34_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB34_mono.bwin_font",   "fonts/cc_fonts/cinecavB43_mono.bwin_font",   "fonts/cc_fonts/cinecavB52_mono.bwin_font"   } }, /* BDCC_FontStyle_Mono  */
        { { "fonts/cc_fonts/cinecavB12_sans.bwin_font",   "fonts/cc_fonts/cinecavB15_sans.bwin_font",   "fonts/cc_fonts/cinecavB19_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12_sans.bwin_font",   "fonts/cc_fonts/cinecavB15_sans.bwin_font",   "fonts/cc_fonts/cinecavB19_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18_sans.bwin_font",   "fonts/cc_fonts/cinecavB23_sans.bwin_font",   "fonts/cc_fonts/cinecavB29_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB28_sans.bwin_font",   "fonts/cc_fonts/cinecavB35_sans.bwin_font",   "fonts/cc_fonts/cinecavB44_sans.bwin_font"   } }, /* BDCC_FontStyle_Prop  */
        { { "fonts/cc_fonts/cinecavB13_casual.bwin_font", "fonts/cc_fonts/cinecavB16_casual.bwin_font", "fonts/cc_fonts/cinecavB20_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13_casual.bwin_font", "fonts/cc_fonts/cinecavB16_casual.bwin_font", "fonts/cc_fonts/cinecavB20_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19_casual.bwin_font", "fonts/cc_fonts/cinecavB24_casual.bwin_font", "fonts/cc_fonts/cinecavB30_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB30_casual.bwin_font", "fonts/cc_fonts/cinecavB36_casual.bwin_font", "fonts/cc_fonts/cinecavB45_casual.bwin_font" } }, /* BDCC_FontStyle_Casual  */
        { { "fonts/cc_fonts/cinecavB12_script.bwin_font", "fonts/cc_fonts/cinecavB14_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12_script.bwin_font", "fonts/cc_fonts/cinecavB14_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16_script.bwin_font", "fonts/cc_fonts/cinecavB18_script.bwin_font", "fonts/cc_fonts/cinecavB25_script.bwin_font" }, { "fonts/cc_fonts/cinecavB25_script.bwin_font", "fonts/cc_fonts/cinecavB28_script.bwin_font", "fonts/cc_fonts/cinecavB38_script.bwin_font" } }, /* BDCC_FontStyle_Cursive */
        { { "fonts/cc_fonts/cinecavB13_sc.bwin_font",     "fonts/cc_fonts/cinecavB15_sc.bwin_font",     "fonts/cc_fonts/cinecavB19_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13_sc.bwin_font",     "fonts/cc_fonts/cinecavB15_sc.bwin_font",     "fonts/cc_fonts/cinecavB19_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18_sc.bwin_font",     "fonts/cc_fonts/cinecavB23_sc.bwin_font",     "fonts/cc_fonts/cinecavB30_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB28_sc.bwin_font",     "fonts/cc_fonts/cinecavB35_sc.bwin_font",     "fonts/cc_fonts/cinecavB45_sc.bwin_font"     } }  /* BDCC_FontStyle_SmallCaps */
    };

    const char * pszItalicFontFilenames[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /* italic */

        { { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB32i_type.bwin_font",   "fonts/cc_fonts/cinecavB41i_type.bwin_font",   "fonts/cc_fonts/cinecavB49i_type.bwin_font"   } }, /* BDCC_FontStyle_Default */
        { { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_type.bwin_font",   "fonts/cc_fonts/cinecavB18i_type.bwin_font",   "fonts/cc_fonts/cinecavB22i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_type.bwin_font",   "fonts/cc_fonts/cinecavB28i_type.bwin_font",   "fonts/cc_fonts/cinecavB33i_type.bwin_font"   }, { "fonts/cc_fonts/cinecavB32i_type.bwin_font",   "fonts/cc_fonts/cinecavB41i_type.bwin_font",   "fonts/cc_fonts/cinecavB49i_type.bwin_font"   } }, /* BDCC_FontStyle_MonoSerifs */
        { { "fonts/cc_fonts/cinecavB12i_serif.bwin_font",  "fonts/cc_fonts/cinecavB15i_serif.bwin_font",  "fonts/cc_fonts/cinecavB19i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB12i_serif.bwin_font",  "fonts/cc_fonts/cinecavB15i_serif.bwin_font",  "fonts/cc_fonts/cinecavB19i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB18i_serif.bwin_font",  "fonts/cc_fonts/cinecavB23i_serif.bwin_font",  "fonts/cc_fonts/cinecavB29i_serif.bwin_font"  }, { "fonts/cc_fonts/cinecavB28i_serif.bwin_font",  "fonts/cc_fonts/cinecavB35i_serif.bwin_font",  "fonts/cc_fonts/cinecavB44i_serif.bwin_font"  } }, /* BDCC_FontStyle_PropSerifs */
        { { "fonts/cc_fonts/cinecavB14i_mono.bwin_font",   "fonts/cc_fonts/cinecavB18i_mono.bwin_font",   "fonts/cc_fonts/cinecavB22i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB14i_mono.bwin_font",   "fonts/cc_fonts/cinecavB18i_mono.bwin_font",   "fonts/cc_fonts/cinecavB22i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB21i_mono.bwin_font",   "fonts/cc_fonts/cinecavB29i_mono.bwin_font",   "fonts/cc_fonts/cinecavB34i_mono.bwin_font"   }, { "fonts/cc_fonts/cinecavB34i_mono.bwin_font",   "fonts/cc_fonts/cinecavB43i_mono.bwin_font",   "fonts/cc_fonts/cinecavB52i_mono.bwin_font"   } }, /* BDCC_FontStyle_Mono  */
        { { "fonts/cc_fonts/cinecavB12i_sans.bwin_font",   "fonts/cc_fonts/cinecavB15i_sans.bwin_font",   "fonts/cc_fonts/cinecavB19i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB12i_sans.bwin_font",   "fonts/cc_fonts/cinecavB15i_sans.bwin_font",   "fonts/cc_fonts/cinecavB19i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB18i_sans.bwin_font",   "fonts/cc_fonts/cinecavB23i_sans.bwin_font",   "fonts/cc_fonts/cinecavB29i_sans.bwin_font"   }, { "fonts/cc_fonts/cinecavB28i_sans.bwin_font",   "fonts/cc_fonts/cinecavB35i_sans.bwin_font",   "fonts/cc_fonts/cinecavB44i_sans.bwin_font"   } }, /* BDCC_FontStyle_Prop  */
        { { "fonts/cc_fonts/cinecavB13i_casual.bwin_font", "fonts/cc_fonts/cinecavB16i_casual.bwin_font", "fonts/cc_fonts/cinecavB20i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB13i_casual.bwin_font", "fonts/cc_fonts/cinecavB16i_casual.bwin_font", "fonts/cc_fonts/cinecavB20i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB19i_casual.bwin_font", "fonts/cc_fonts/cinecavB24i_casual.bwin_font", "fonts/cc_fonts/cinecavB30i_casual.bwin_font" }, { "fonts/cc_fonts/cinecavB30i_casual.bwin_font", "fonts/cc_fonts/cinecavB36i_casual.bwin_font", "fonts/cc_fonts/cinecavB45i_casual.bwin_font" } }, /* BDCC_FontStyle_Casual  */
        { { "fonts/cc_fonts/cinecavB12i_script.bwin_font", "fonts/cc_fonts/cinecavB14i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB12i_script.bwin_font", "fonts/cc_fonts/cinecavB14i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB16i_script.bwin_font", "fonts/cc_fonts/cinecavB18i_script.bwin_font", "fonts/cc_fonts/cinecavB25i_script.bwin_font" }, { "fonts/cc_fonts/cinecavB25i_script.bwin_font", "fonts/cc_fonts/cinecavB28i_script.bwin_font", "fonts/cc_fonts/cinecavB38i_script.bwin_font" } }, /* BDCC_FontStyle_Cursive */
        { { "fonts/cc_fonts/cinecavB13i_sc.bwin_font",     "fonts/cc_fonts/cinecavB15i_sc.bwin_font",     "fonts/cc_fonts/cinecavB19i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB13i_sc.bwin_font",     "fonts/cc_fonts/cinecavB15i_sc.bwin_font",     "fonts/cc_fonts/cinecavB19i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB18i_sc.bwin_font",     "fonts/cc_fonts/cinecavB23i_sc.bwin_font",     "fonts/cc_fonts/cinecavB30i_sc.bwin_font"     }, { "fonts/cc_fonts/cinecavB28i_sc.bwin_font",     "fonts/cc_fonts/cinecavB35i_sc.bwin_font",     "fonts/cc_fonts/cinecavB45i_sc.bwin_font"     } }  /* BDCC_FontStyle_SmallCaps */
    };

    /* we do not need to scale these fonts using the m2mc since we have pre-rendered a complete set of 126!! */
    scaleFactor = 100; /* scale factor in hundredths i.e. scale factor is 1.0 */

    BDBG_MSG(("USING MAX FONTS\n"));

#endif /* ifdef SCALE_720P_FONTS */

#endif /* ifdef SCALE_480I_FONTS */

#endif /* ifdef FREETYPE_SUPPORT */

    BKNI_Memset(standardFonts, 0, (sizeof(BDCC_FONT_DESCRIPTOR) * BDCC_FontStyle_Max_Value));
    BKNI_Memset(italicsFonts, 0, (sizeof(BDCC_FONT_DESCRIPTOR) * BDCC_FontStyle_Max_Value));

#ifdef FREETYPE_SUPPORT

    for (i = 0; i < BDCC_FontStyle_Max_Value; i++)
    {
        for (j = 0; j < BDCC_PenSize_Max_Size; j++)
        {
            standardFonts[i][j].pszFontFile = pszFontFilenames[i];
            standardFonts[i][j].iFontSize   = iFontSize[i][index][j];

            italicsFonts[i][j].pszFontFile = pszFontFilenames[i];
            italicsFonts[i][j].iFontSize   = iFontSize[i][index][j];
        }
    }

#else /* ifdef FREETYPE_SUPPORT */

    for (i = 0; i < BDCC_FontStyle_Max_Value; i++)
    {
        for (j = 0; j < BDCC_PenSize_Max_Size; j++)
        {
            standardFonts[i][j].pszFontFile = pszStandardFontFilenames[i][index][j];
            standardFonts[i][j].iFontSize   = -1;

            italicsFonts[i][j].pszFontFile = pszItalicFontFilenames[i][index][j];
            italicsFonts[i][j].iFontSize   = -1;
        }
    }

#endif /* ifdef FREETYPE_SUPPORT */

    BDBG_MSG(("\nCCTest_P_LoadFonts():\n"));

    /*
    ** First load the fonts.
    */
    for (i = 0; i < BDCC_FontStyle_Max_Value; i++)
    {
        for (j = 0; j < BDCC_PenSize_Max_Size; j++)
        {
            pFont = &standardFonts[i][j];

            if (pFont->pszFontFile) /* need not specify all fonts, minimum is the DEFAULT font */
            {
                iErr = BDCC_WINLIB_LoadFont(
                        WindowLib,
                        pFont->pszFontFile,
                        pFont->iFontSize,
                        (BDCC_FontStyle)i,
                        (BDCC_PenSize)j,
                        BDCC_PenStyle_Standard
                        );
                BDBG_MSG(("%s %s\n", iErr ? "\tFAILED to load font!!!:" : "\tloaded font:", pFont->pszFontFile));
            }

            pFont = &italicsFonts[i][j];

            if (pFont->pszFontFile) /* need not specify all fonts, minimum is the DEFAULT font */
            {
                iErr = BDCC_WINLIB_LoadFont(
                        WindowLib,
                        pFont->pszFontFile,
                        pFont->iFontSize,
                        (BDCC_FontStyle)i,
                        (BDCC_PenSize)j,
                        BDCC_PenStyle_Italics
                        );

                BDBG_MSG(("%s %s\n", iErr ? "\tFAILED to load font!!!:" : "\tloaded font:", pFont->pszFontFile));
            }
        }
    }

    BDBG_MSG(("\n"));
} /* end of CCTest_P_LoadFonts() */

#endif /* ifdef DCC_SUPPORT */