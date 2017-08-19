/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */

#include "framework.h"

#include "brdc.h"
#include "bvdc.h"
#include "framework_board.h"
#include "framework_board_bvdc.h"

#include "bhdm.h"

/* graphics */
#include "bavc.h"
#include "bvdc.h"
#include "bsur.h"
#include "bpxl.h"

BDBG_MODULE(SIMPLE_VDC);

/**************************************************************************/
#define __DISP_NAME(d)  #d
#define DISP_NAME(d)  __DISP_NAME(d)

#define DISP_DUAL    0
#define DISP_1       BVDC_DisplayId_eDisplay1
#define CMP_1        BVDC_CompositorId_eCompositor1

#define OUTPUT_HDM    1
#define HDMI_ID       0

#define BTST_P_GFX_WIDTH             (300)
#define BTST_P_GFX_HEIGHT            (200)

#define TestError(e, str)   {\
    err = e;\
    if (err != BERR_SUCCESS)\
    {\
        printf("%s: %d ERROR %x!! "str"\n", __FILE__,__LINE__, e );\
        goto Done;\
    }\
}

/**************************************************************************/
static void SetupGfxDisplay(
    BMMA_Heap_Handle        hMmaHeap,
    BVDC_Handle             hVdc,
    BAVC_SourceId           eSrcId,
    BVDC_Compositor_Handle  hCompositor,
    BFMT_VideoInfo       *  pVideoFmtInfo
)
{
    BVDC_Window_Handle hGfxWin;
    BVDC_Source_Handle hGfxSrc;
    BPXL_Plane stSurface;
    BAVC_Gfx_Picture stGfxPic;
    uint8_t *pImage;
    BERR_Code err;
    int ii, jj;

    TestError( BVDC_Source_Create(hVdc, &hGfxSrc, eSrcId, NULL), "BVDC_Source_Create");

    BPXL_Plane_Init(&stSurface, BTST_P_GFX_WIDTH, BTST_P_GFX_HEIGHT, BPXL_eA8_R8_G8_B8);
    TestError( BPXL_Plane_AllocateBuffers(&stSurface, hMmaHeap), "BPXL_Plane_AllocateBuffers: out of memory" );
    pImage = (uint8_t *)BMMA_Lock(stSurface.hPixels);
    /*BKNI_Memset((void*)pImage, 0x80, stSurface.ulBufSize);*/
    for(ii=0; ii<BTST_P_GFX_WIDTH; ii++)
        for(jj=0; jj<BTST_P_GFX_HEIGHT; jj++)
            *((uint32_t *)pImage + BTST_P_GFX_WIDTH * jj + ii) =
                (ii < BTST_P_GFX_WIDTH*1/8)? 0xFFFF0000 :
                (ii < BTST_P_GFX_WIDTH*2/8)? 0xFF00FF00 :
                (ii < BTST_P_GFX_WIDTH*3/8)? 0xFF0000FF :
                (ii < BTST_P_GFX_WIDTH*4/8)? 0xFF000000 :
                (ii < BTST_P_GFX_WIDTH*5/8)? 0xFFFFFFFF :
                (ii < BTST_P_GFX_WIDTH*6/8)? 0xFF00FFff :
                (ii < BTST_P_GFX_WIDTH*7/8)? 0xFFFF00FF : 0xFFFFFF00;

    BMMA_FlushCache(stSurface.hPixels, pImage, stSurface.ulBufSize);

    BKNI_Memset((void*)&stGfxPic, 0x0, sizeof(BAVC_Gfx_Picture));
    stGfxPic.pSurface = &stSurface;
    stGfxPic.eInOrientation = BFMT_Orientation_e2D;
    TestError( BVDC_Source_SetSurface (hGfxSrc, &stGfxPic), "BVDC_Source_SetSurface" );
    BMMA_Unlock(stSurface.hPixels, pImage);

    TestError( BVDC_Window_Create(hCompositor, &hGfxWin, BVDC_WindowId_eGfx0, hGfxSrc, NULL), "BVDC_Window_Create" );
    TestError( BVDC_Window_SetAlpha(hGfxWin, BVDC_ALPHA_MAX), "BVDC_Window_SetAlpha" );
    TestError( BVDC_Window_SetBlendFactor(hGfxWin, BVDC_BlendFactor_eSrcAlpha, BVDC_BlendFactor_eOneMinusSrcAlpha, BVDC_ALPHA_MAX), "BVDC_Window_SetBlendFactor" );
    TestError( BVDC_Window_SetZOrder(hGfxWin, 0), "BVDC_Window_SetZOrder" );
    TestError( BVDC_Window_SetScalerOutput(hGfxWin, 0, 0, BTST_P_GFX_WIDTH, BTST_P_GFX_HEIGHT), "BVDC_Window_SetScalerOutput" );
    TestError( BVDC_Window_SetDstRect(hGfxWin,
                   (pVideoFmtInfo->ulWidth - BTST_P_GFX_WIDTH)/2, (pVideoFmtInfo->ulHeight - BTST_P_GFX_HEIGHT)/2,
                   BTST_P_GFX_WIDTH, BTST_P_GFX_HEIGHT), "BVDC_Window_SetDstRect" );
    TestError( BVDC_ApplyChanges(hVdc), "BVDC_ApplyChanges" );

    TestError( BVDC_Window_SetVisibility(hGfxWin, true), "BVDC_Window_SetVisibility" );
    TestError( BVDC_ApplyChanges(hVdc), "BVDC_ApplyChanges" );

  Done:
    return;

}

/**************************************************************************/
static void TestInitVdcCfg(
    BVDC_Settings          *pVdcConfig
)
{
    pVdcConfig->eVideoFormat = BFMT_VideoFmt_e480p;

    BKNI_Memset((void*)&pVdcConfig->stHeapSettings, 0x0, sizeof(BVDC_Heap_Settings));
}

#if OUTPUT_HDM

/***************************************************************************
 * modified from vdc_test's BTST_P_OpenHdmi and BTST_P_ToggleHdmiOut_priv
 */
static void SetupHdmiOutput
    ( BVDC_Handle             hVdc,
      BVDC_Display_Handle     hDisplay0,
      BFramework_Info        *pFrmInfo)
{
    BHDM_Handle hHdm;
    BHDM_Settings  stHdmiSettings;
    BVDC_Display_HdmiSettings stVdcHdmiSettings;
    BERR_Code err;
    uint8_t ucRxAttached;

    BVDC_Display_GetHdmiSettings(hDisplay0, &stVdcHdmiSettings);
    stVdcHdmiSettings.ulPortId      = BVDC_Hdmi_0;
    stVdcHdmiSettings.eMatrixCoeffs = BAVC_MatrixCoefficients_eItu_R_BT_709;
    stVdcHdmiSettings.eColorComponent = BAVC_Colorspace_eYCbCr422;
    /*stVdcHdmiSettings.eColorComponent = BAVC_Colorspace_eRGB;*/
    TestError(BVDC_Display_SetHdmiSettings(hDisplay0, &stVdcHdmiSettings), "BVDC_Display_SetHdmiSettings");
    TestError(BVDC_ApplyChanges(hVdc), "BVDC_ApplyChanges");

    BHDM_GetDefaultSettings(&stHdmiSettings);
    stHdmiSettings.hTMR = pFrmInfo->hTmr;
    stHdmiSettings.eOutputPort = BHDM_OutputPort_eHDMI;
    stHdmiSettings.eOutputFormat = BHDM_OutputFormat_eHDMIMode;
    stHdmiSettings.eInputVideoFmt = DISP_FORMAT0; /* 480p */
    stHdmiSettings.eCoreId = HDMI_ID;
    stHdmiSettings.overrideDefaultColorimetry = true;
    stHdmiSettings.eColorimetry = stVdcHdmiSettings.eMatrixCoeffs;
    stHdmiSettings.stVideoSettings.eColorSpace = stVdcHdmiSettings.eColorComponent;
    TestError(BHDM_Open(
        &hHdm, pFrmInfo->hChp,
        pFrmInfo->hReg, pFrmInfo->hInt, pFrmInfo->hFrmWorkBoard->ahI2cRegHandles[BFramework_HDMI_OUTPUT_I2C_CHANNEL],
        &stHdmiSettings), "BHDM_Open");
    TestError(BHDM_EnableDisplay(hHdm, &stHdmiSettings), "BHDM_EnableDisplay");
    TestError(BHDM_SetHdmiDataTransferMode(hHdm, true), "BHDM_SetHdmiDataTransferMode");

    TestError(BHDM_RxDeviceAttached(hHdm, &ucRxAttached), "BHDM_RxDeviceAttached");
    printf("+++++++++++++++++++++++++++++++++++++\n");
    printf("+ (%d) HDMI RX %d device is attached! +\n", ucRxAttached, HDMI_ID);
    printf("+++++++++++++++++++++++++++++++++++++\n");

  Done:
    return;
}
#endif

/***************************************************************************
 * Main program (exposed API)
 *
 */
int app_main( int argc, char **argv )
{
    BSystem_Info sysInfo;
    BFramework_Info frmInfo;
    BVDC_Settings stVdcConfig;

    BRDC_Handle             hRdc;
    BVDC_Handle             hVdc;
    BVDC_Compositor_Handle  hCompositor0;
    BVDC_Compositor_Handle  hCompositor1;
    BVDC_Display_Handle     hDisplay0;
    BVDC_Display_Handle     hDisplay1;
    BVDC_CompositorId       eCmpId0;
    BVDC_DisplayId          eDisplayId0;
    BVDC_CompositorId       eCmpId1;
    BVDC_DisplayId          eDisplayId1;
    BFMT_VideoInfo          stVideoFmtInfo;
    BERR_Code               err;
    bool                    bDualDisplay;
    int                     iErr;
    BMMA_Heap_Handle        hGfxMem = NULL;

    eCmpId0 = BVDC_CompositorId_eCompositor0;
    eDisplayId0 = BVDC_DisplayId_eDisplay0;
    eCmpId1 = CMP_1;
    eDisplayId1 = DISP_1;
    err = BERR_SUCCESS;
    bDualDisplay = DISP_DUAL;

    /* System Init (interrupts/memory mapping) */
    iErr = BSystem_Init( argc, argv, &sysInfo );
    if ( iErr )
    {
        printf( "System init FAILED!\n" );
        return iErr;
    }

    /* Framework init (base modules) */
    iErr = BFramework_Init( &sysInfo, &frmInfo );
    if ( iErr )
    {
        printf( "Framework init FAILED!\n" );
        return iErr;
    }

#if 0
    /* turn on module msg */
    BDBG_SetModuleLevel("BVDC", BDBG_eMsg);
    BDBG_SetModuleLevel("BVDC_DISP", BDBG_eMsg);

    BDBG_SetModuleLevel("BVDC_CFC_1", BDBG_eMsg);
    BDBG_SetModuleLevel("BVDC_CFC_2", BDBG_eMsg);
    BDBG_SetModuleLevel("BVDC_CFC_3", BDBG_eMsg);
    BDBG_SetModuleLevel("BVDC_CFC_4", BDBG_eMsg);

    BDBG_SetModuleLevel("BHDM", BDBG_eMsg);
    BDBG_SetModuleLevel("BHDM_PACKET_AVI", BDBG_eMsg);
    BDBG_SetModuleLevel("BHDM_PACKET_DRM", BDBG_eMsg);
#endif

    /* open Register DMA */
#if (BCHP_CHIP==7445)
    TestError( BRDC_Open(&hRdc, frmInfo.hChp, frmInfo.hReg, frmInfo.hFrmWorkBoard->hMmaHeap2, NULL),
        "ERROR: BRDC_Open" );
#elif (BCHP_CHIP==7145)
    TestError( BRDC_Open(&hRdc, frmInfo.hChp, frmInfo.hReg, frmInfo.hFrmWorkBoard->hMmaHeap1, NULL),
        "ERROR: BRDC_Open" );
#else
    TestError( BRDC_Open(&hRdc, frmInfo.hChp, frmInfo.hReg, frmInfo.hMmaHeap, NULL),
        "ERROR: BRDC_Open" );
#endif

    /* open VDC */
    BVDC_GetDefaultSettings(frmInfo.hBox, &stVdcConfig);
    TestInitVdcCfg(&stVdcConfig);
    TestError( BVDC_Open(&hVdc,frmInfo.hChp, frmInfo.hReg, frmInfo.hMmaHeap,
        frmInfo.hInt, hRdc, frmInfo.hTmr, &stVdcConfig),
        "ERROR: BVDC_Open" );

    printf("Compositor %d -> Display %d (dspFmt %s)\n", eCmpId0, eDisplayId0, DISP_NAME(DISP_FORMAT0));

    /* create compositor handle */
    TestError( BVDC_Compositor_Create( hVdc, &hCompositor0, eCmpId0, NULL ),
        "ERROR: BVDC_Compositor_Create" );

    /* create display for our new compositor */
    TestError( BVDC_Display_Create( hCompositor0, &hDisplay0, eDisplayId0, NULL),
        "ERROR: BVDC_Display_Create" );

    /* Set display format */
    TestError( BVDC_Display_SetVideoFormat( hDisplay0, DISP_FORMAT0 ),
        "ERROR: BVDC_Display_SetVideoFormat" );

    /* Set the background color to red */
    TestError( BVDC_Compositor_SetBackgroundColor( hCompositor0,
        0x80, 0x80, 0x80 ),
        "ERROR: BVDC_Compositor_SetBackgroundColor" );

#if OUTPUT_HDM
    SetupHdmiOutput(hVdc, hDisplay0, &frmInfo);

#else /* #if OUTPUT_HDM */
    /* Set component DACs for all formats */
    TestError( BVDC_Display_SetDacConfiguration( hDisplay0,
        BFramework_Dac_Component_pr, BVDC_DacOutput_ePr),
        "ERROR: BVDC_Display_SetDacConfiguration" );
    TestError( BVDC_Display_SetDacConfiguration( hDisplay0,
        BFramework_Dac_Component_y, BVDC_DacOutput_eY),
        "ERROR: BVDC_Display_SetDacConfiguration" );
    TestError( BVDC_Display_SetDacConfiguration( hDisplay0,
        BFramework_Dac_Component_pb, BVDC_DacOutput_ePb),
        "ERROR: BVDC_Display_SetDacConfiguration" );

    /* set DAC configurations for specific display format */
    switch (DISP_FORMAT0)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_B:
        if (!bDualDisplay &&
            ((eDisplayId0 == BVDC_DisplayId_eDisplay1) ||
            ((eDisplayId0 == BVDC_DisplayId_eAuto) && (eCmpId0 == BVDC_CompositorId_eCompositor1))))
        {
            printf("Svideo/Composite\n");
            TestError( BVDC_Display_SetDacConfiguration( hDisplay0,
                BFramework_Dac_Composite_0, BVDC_DacOutput_eComposite),
                "ERROR: BVDC_Display_SetDacConfiguration" );
#if (BCHP_CHIP != 7550) && (BCHP_CHIP != 7422) && \
    (BCHP_CHIP != 7344) && (BCHP_CHIP != 7346)
            TestError( BVDC_Display_SetDacConfiguration( hDisplay0,
                BFramework_Dac_Svideo_Chroma, BVDC_DacOutput_eSVideo_Chroma),
                "ERROR: BVDC_Display_SetDacConfiguration" );
            TestError( BVDC_Display_SetDacConfiguration( hDisplay0,
                BFramework_Dac_Svideo_Luma, BVDC_DacOutput_eSVideo_Luma),
                "ERROR: BVDC_Display_SetDacConfiguration" );
#endif
        }
        break;
    default:
        break;
    }
#endif /* #if OUTPUT_HDM */

    /* add gfx display */
    TestError( BFMT_GetVideoFormatInfo( DISP_FORMAT0, &stVideoFmtInfo ),
               "ERROR: BFMT_GetVideoFormatInfo" );

#if (BCHP_CHIP==7445)
    hGfxMem = frmInfo.hFrmWorkBoard->hMmaHeap2;
#elif (BCHP_CHIP==7145)
    hGfxMem = frmInfo.hFrmWorkBoard->hMmaHeap1;
#else
    hGfxMem = frmInfo.hMmaHeap;
#endif

    SetupGfxDisplay(hGfxMem, hVdc, BAVC_SourceId_eGfx0, hCompositor0, &stVideoFmtInfo);

    if (bDualDisplay)
    {
        printf("Compositor %d -> Display %d (dspFmt %s)\n", eCmpId1, eDisplayId1, DISP_NAME(DISP_FORMAT1));

        /* create compositor handle */
        TestError( BVDC_Compositor_Create( hVdc, &hCompositor1, eCmpId1, NULL ),
            "ERROR: BVDC_Compositor_Create" );

        /* create display for our new compositor */
        TestError( BVDC_Display_Create( hCompositor1, &hDisplay1, eDisplayId1, NULL),
            "ERROR: BVDC_Display_Create" );

        /* Set display format */
        TestError( BVDC_Display_SetVideoFormat( hDisplay1, DISP_FORMAT1 ),
            "ERROR: BVDC_Display_SetVideoFormat" );

        /* Set the background color to green */
        TestError( BVDC_Compositor_SetBackgroundColor( hCompositor1,
            0x00, 0xFF, 0x0 ),
            "ERROR: BVDC_Compositor_SetBackgroundColor" );

        TestError( BVDC_Display_SetDacConfiguration( hDisplay1,
            BFramework_Dac_Composite_0, BVDC_DacOutput_eComposite),
            "ERROR: BVDC_Display_SetDacConfiguration" );
#if (BCHP_CHIP != 7550) && (BCHP_CHIP != 7422) && (BCHP_CHIP != 7425) && \
    (BCHP_CHIP != 7344) && (BCHP_CHIP != 7346)
        TestError( BVDC_Display_SetDacConfiguration( hDisplay1,
            BFramework_Dac_Svideo_Chroma, BVDC_DacOutput_eSVideo_Chroma),
            "ERROR: BVDC_Display_SetDacConfiguration" );
        TestError( BVDC_Display_SetDacConfiguration( hDisplay1,
            BFramework_Dac_Svideo_Luma, BVDC_DacOutput_eSVideo_Luma),
            "ERROR: BVDC_Display_SetDacConfiguration" );
#endif

        /* add gfx display */
        TestError( BFMT_GetVideoFormatInfo( DISP_FORMAT1, &stVideoFmtInfo ),
                   "ERROR: BFMT_GetVideoFormatInfo" );
#if (BCHP_CHIP==7445)
    hGfxMem = frmInfo.hFrmWorkBoard->hMmaHeap2;
#elif (BCHP_CHIP==7145)
    hGfxMem = frmInfo.hFrmWorkBoard->hMmaHeap1;
#else
    hGfxMem = frmInfo.hMmaHeap;
#endif
        SetupGfxDisplay(hGfxMem, hVdc, BAVC_SourceId_eGfx1, hCompositor1, &stVideoFmtInfo);
    }

    /* apply changes */
    TestError( BVDC_ApplyChanges(hVdc),
        "ERROR:BVDC_ApplyChanges" );

    /* Wait for user */
    printf( "Hit 'q' to exit\n" );
    while(true)
    {
        char c;
        c = getchar();
        if (c=='q')
            break;
    }

    return BERR_SUCCESS;


  Done:
    return err;
}

/* End of file */
