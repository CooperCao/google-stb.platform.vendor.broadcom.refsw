/***************************************************************************
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
 *
* Module Description: Framework initialization header file
*
***************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "framework.h"

#include "brdc.h"
#include "bvdc.h"
#include "framework_board.h"
#include "framework_board_bvdc.h"

#ifdef IKOS_EMULATION
#include "client.h"
#endif

#ifdef BVDC_MACRO_TEST
#include "macro_test.h"
#endif

BDBG_MODULE(SIMPLE_VDC);

/**************************************************************************/
#define DISP_FORMAT BFMT_VideoFmt_e1080i

/**************************************************************************/
#define TestError(e, str)   {\
    err = e;\
    if (err != BERR_SUCCESS)\
    {\
    BDBG_ERR(( str". %s: %d %x", __FILE__,__LINE__, e ));\
        goto Done;\
    }\
}

#ifdef IKOS_EMULATION

static void BTST_P_IkosInitCapture
    ( void )
{
    char achCapturePath[40];

    strcpy(achCapturePath, ".");

    if (IKOS_Client_InitCapture( 0, sizeof(achCapturePath), achCapturePath ) != 0)
        BDBG_ERR(("Failed to initialize IKOS capture"));

}

static void BTST_P_IkosStartCapture
    ( void )
{
    if (IKOS_Client_StartCapture() != 0)
        BDBG_ERR(("Failed to start IKOS capture"));
}

static void BTST_P_IkosStopCapture
    ( void )
{
    if (IKOS_Client_StopCapture() != 0)
        BDBG_ERR(("Failed to stop IKOS capture"));
}

#endif

/**************************************************************************/
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
    BFMT_VideoFmt           eDisplayFmt = BFMT_VideoFmt_eNTSC;
    BVDC_CompositorId       eCmpId0 = BVDC_CompositorId_eCompositor0;
    BVDC_DisplayId          eDisplayId0 = BVDC_DisplayId_eDisplay0;
    BVDC_CompositorId       eCmpId1 = BVDC_CompositorId_eCompositor0;
    BVDC_DisplayId          eDisplayId1 = BVDC_DisplayId_eDisplay0;
    BFMT_VideoInfo          stVideoFmtInfo;
    BERR_Code               err = BERR_SUCCESS;
    bool                    bDualDisplay=false;
    int                     i;

    /* System Init (interrupts/memory mapping) */
    int iErr = BSystem_Init( argc, argv, &sysInfo );
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

    if (argc > 1)
    {
        for(i=1; i < argc; i++)
        {
            if(!strcmp(argv[i], "NTSC"))
                eDisplayFmt = BFMT_VideoFmt_eNTSC;
            else if(!strcmp(argv[i], "480p"))
                eDisplayFmt = BFMT_VideoFmt_e480p;
            else if(!strcmp(argv[i], "576p"))
                eDisplayFmt = BFMT_VideoFmt_e576p_50Hz;
            else if(!strcmp(argv[i], "720p"))
                eDisplayFmt = BFMT_VideoFmt_e720p;
            else if(!strcmp(argv[i], "1080i"))
                eDisplayFmt = BFMT_VideoFmt_e1080i;
            else if(!strcmp(argv[i], "PAL"))
                eDisplayFmt = BFMT_VideoFmt_ePAL_B;
            else if (!strcmp(argv[i], "c0"))
                eCmpId0 = BVDC_CompositorId_eCompositor0;
            else if (!strcmp(argv[i], "c1"))
                eCmpId0 = BVDC_CompositorId_eCompositor1;
            else if (!strcmp(argv[i], "c2"))
                eCmpId0 = BVDC_CompositorId_eCompositor2;
            else if (!strcmp(argv[i], "d0"))
                eDisplayId0 = BVDC_DisplayId_eDisplay0;
            else if (!strcmp(argv[i], "d1"))
                eDisplayId0 = BVDC_DisplayId_eDisplay1;
            else if (!strcmp(argv[i], "d2"))
                eDisplayId0 = BVDC_DisplayId_eDisplay2;
            else if (!strcmp(argv[i], "auto"))
                eDisplayId0 = BVDC_DisplayId_eAuto;
            else if (!strcmp(argv[i], "dual"))
                bDualDisplay= true;
            else
            {
                fprintf (
                    stderr, "ERROR: unrecognized option \"%s\"\n", argv[i]);
#ifndef EMULATION
                exit (1);
#else
                printf("Unreconized option %s\n", argv[i]);
#endif
            }

        }
    }

#if 0
    BDBG_SetModuleLevel("BVDC", BDBG_eMsg);
    BDBG_SetModuleLevel("BVDC_DISP", BDBG_eMsg);
#endif

    /* get data about output format */
    TestError( BFMT_GetVideoFormatInfo( eDisplayFmt, &stVideoFmtInfo ),
        "ERROR: BFMT_GetVideoFormatInfo" );

    printf("Display format is %s\n", stVideoFmtInfo.pchFormatStr);
    printf("[Compositor %d -> Display %d]\n", eCmpId0, eDisplayId0);

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

    TestError( BVDC_Open(&hVdc,frmInfo.hChp, frmInfo.hReg, frmInfo.hMmaHeap,
        frmInfo.hInt, hRdc, frmInfo.hTmr, &stVdcConfig),
        "ERROR: BVDC_Open" );

    /* create compositor handle */
    TestError( BVDC_Compositor_Create( hVdc, &hCompositor0, eCmpId0, NULL ),
        "ERROR: BVDC_Compositor_Create" );

    /* create display for our new compositor */
    TestError( BVDC_Display_Create( hCompositor0, &hDisplay0, eDisplayId0, NULL),
        "ERROR: BVDC_Display_Create" );

    /* Set display format */
    TestError( BVDC_Display_SetVideoFormat( hDisplay0, eDisplayFmt ),
        "ERROR: BVDC_Display_SetVideoFormat" );

    /* Set the background color to red */
    TestError( BVDC_Compositor_SetBackgroundColor( hCompositor0,
        0xFF, 0x00, 0x00 ),
        "ERROR: BVDC_Compositor_SetBackgroundColor" );

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

#if (BCHP_CHIP != 3548) /** { **/
    /* set DAC configurations for specific display format */
    switch (eDisplayFmt)
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
    default: break;
    }
#endif /** } **/

    if (bDualDisplay)
    {
        printf("Dual Display\n");
        printf("[2nd Compositor %d -> Display %d]\n", eCmpId1, eDisplayId1);

        /* create compositor handle */
        TestError( BVDC_Compositor_Create( hVdc, &hCompositor1, eCmpId1, NULL ),
            "ERROR: BVDC_Compositor_Create" );

        /* create display for our new compositor */
        TestError( BVDC_Display_Create( hCompositor1, &hDisplay1, eDisplayId1, NULL),
            "ERROR: BVDC_Display_Create" );

        /* Set display format */
        TestError( BVDC_Display_SetVideoFormat( hDisplay1, BFMT_VideoFmt_eNTSC ),
            "ERROR: BVDC_Display_SetVideoFormat" );

        /* Set the background color to blue */
        TestError( BVDC_Compositor_SetBackgroundColor( hCompositor1,
            0x00, 0x00, 0xFF ),
            "ERROR: BVDC_Compositor_SetBackgroundColor" );

#if (BCHP_CHIP != 3548)
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
#endif
    }

    /* apply changes */
    TestError( BVDC_ApplyChanges(hVdc),
        "ERROR:BVDC_ApplyChanges" );

#ifdef IKOS_EMULATION
    /* start IKOS capture */
    BTST_P_IkosInitCapture();
    BTST_P_IkosStartCapture();

    printf("IKOS capture started \n");
#endif

#if 0
    while (1)
    {
        /* printf ("Black\n"); */
        TestError( BVDC_Compositor_SetBackgroundColor( hCompositor0,
            0x00, 0x00, 0x00 ),
            "ERROR: BVDC_Compositor_SetBackgroundColor" );
        TestError( BVDC_ApplyChanges(hVdc),
            "ERROR:BVDC_ApplyChanges" );
        BKNI_Sleep (1000);

        /* printf ("Red\n"); */
        TestError( BVDC_Compositor_SetBackgroundColor( hCompositor0,
            0xFF, 0x00, 0x00 ),
            "ERROR: BVDC_Compositor_SetBackgroundColor" );
        TestError( BVDC_ApplyChanges(hVdc),
            "ERROR:BVDC_ApplyChanges" );
        BKNI_Sleep (1000);

        /* printf ("Green\n"); */
        TestError( BVDC_Compositor_SetBackgroundColor( hCompositor0,
            0x00, 0xFF, 0x00 ),
            "ERROR: BVDC_Compositor_SetBackgroundColor" );
        TestError( BVDC_ApplyChanges(hVdc),
            "ERROR:BVDC_ApplyChanges" );
        BKNI_Sleep (1000);

        /* printf ("Blue\n"); */
        TestError( BVDC_Compositor_SetBackgroundColor( hCompositor0,
            0x00, 0x00, 0xFF ),
            "ERROR: BVDC_Compositor_SetBackgroundColor" );
        TestError( BVDC_ApplyChanges(hVdc),
            "ERROR:BVDC_ApplyChanges" );
        BKNI_Sleep (1000);
    }
#endif

    /* Wait for user */
    printf( "Hit key to exit\n" );
    getchar();

#ifdef IKOS_EMULATION
    BTST_P_IkosStopCapture();

    printf("IKOS capture stopped \n");
#endif

    /* Clean up! */
    TestError( BVDC_Display_Destroy( hDisplay0 ), "ERROR: BVDC_Display_Destroy" );
    TestError( BVDC_Compositor_Destroy( hCompositor0 ), "ERROR: BVDC_Compositor_Destroy" );
    TestError( BVDC_ApplyChanges( hVdc ), "ERROR: BVDC_ApplyChanges" );
    TestError( BVDC_Close( hVdc ), "ERROR: BVDC_Close" );
    TestError( BRDC_Close( hRdc ), "ERROR: BRDC_Close" );

Done:
    BFramework_Uninit(&frmInfo);
    BSystem_Uninit(&sysInfo);
    return err;
}

/* End of file */
