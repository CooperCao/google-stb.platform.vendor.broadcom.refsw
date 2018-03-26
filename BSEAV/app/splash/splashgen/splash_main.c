/******************************************************************************
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
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bstd.h"
#include "bkni.h"
#include "bint.h"
#include "bi2c.h"

#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#ifndef debug_only
#include "priv/nexus_core.h"
#endif
#include "nexus_types.h"
#include "bsplash_board.h"
#include "splash_vdc_rulgen.h"

BDBG_MODULE(splashgen);

ModeHandles g_stModeHandles;

static void print_usage(void)
{
    printf("\nUsage:\n");
    printf("\t -d <format> - Optional, for overiding HD display format.\n");
    printf("\t               Use the number in the parenthesis:\n");
    printf("\t               NTSC(0), PAL_G(1), 480p60(2), 576p50(3), 720p60(4)\n");
    printf("\t               720p50(5), 1080i60(6), 1080i50(7), 1080p60(8), 1080p50(9)\n\n");
    printf("\t               SD format will be NTSC  for 60Hz HD format\n");
    printf("\t                                 PAL_G for 50Hz HD format\n");
    printf("\n\t -f          - Scale splash bmp to fullscreen if hardware\n");
    printf("\t               feature available.\n");
    printf("\n\t -h          - prints this message\n\n");
}

int main(int argc, char *argv[])
{
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
    BREG_I2C_Handle i2cRegHandle = NULL;
    NEXUS_MemoryStatus stat;
    BMMA_Heap_Handle  hMma[SPLASH_NUM_MEM] = {NULL};
    uint32_t heap_size[SPLASH_NUM_MEM] = {0};
    uint32_t heap_idx[SPLASH_NUM_MEM] = {0xffffffff};
    uint32_t format;
    bool overrideFmt;
    bool bScaleToFullScreen = false;
    int ii, jj;

#ifdef SPLASH_SUPPORT_HDM
    BI2C_Handle i2cHandle;
    BI2C_ChannelSettings i2cChanSettings;
    BI2C_ChannelHandle i2cChanHandle;
#endif
    BBOX_Handle      hBox;
    int              rulMem;
    int              surf0Mem;
#if (SPLASH_NUM_SURFACE>1)
    int              surf1Mem;
#endif

    /* Process run-time command options, ok to have none */
    overrideFmt = false;
    for (ii = 1; ii < argc; ii++)
    {
            if (!strcmp(argv[ii], "-d"))
            {
                    format = strtol(argv[++ii], NULL, 0);
                    overrideFmt = true;
            }
            else if ((!strcmp(argv[ii], "-f")) || (!strcmp(argv[ii], "--fullscreen")))
            {
                    bScaleToFullScreen = true;
            }
            else
            {
                    print_usage();
                    return -1;
            }
    }

    BKNI_Memset((void*)&g_stModeHandles, 0x0, sizeof(ModeHandles));

    NEXUS_SetEnv("NEXUS_BASE_ONLY_INIT","y");
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openI2c = false;
    platformSettings.openFrontend = false;
    platformSettings.openFpga=false;
    platformSettings.openOutputs = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);

    for (ii=0; ii<NEXUS_MAX_HEAPS; ii++)
    {
        if (g_pCoreHandles->heap[ii].nexus == NULL)
            continue;

        rc = NEXUS_Heap_GetStatus(g_pCoreHandles->heap[ii].nexus, &stat);
        if (!rc)
        {
            bool bGotAll = true;

            BDBG_MSG(("mem %d: idx %d, type 0x%x", ii, stat.memcIndex, stat.memoryType));
            if (stat.memcIndex<SPLASH_NUM_MEM &&
                stat.memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED &&
                stat.size > heap_size[stat.memcIndex])
            {
                heap_idx[stat.memcIndex] = ii;
                heap_size[stat.memcIndex] = stat.size;
                hMma[stat.memcIndex] = NEXUS_Heap_GetMmaHandle(g_pCoreHandles->heap[ii].nexus);
            }

            for (jj=0; jj<SPLASH_NUM_MEM; jj++)
            {
                if (NULL == hMma[jj])
                {
                    bGotAll = false; break;
                }
            }
            if (bGotAll) break;
        }
    }

    /* box modes, if supported */
#if (BBOX_BOX_MODES_SUPPORTED)
    {
        BBOX_Settings stBoxSettings;
        NEXUS_PlatformStatus status;
        BBOX_Config  stBoxConfig;
        BBOX_Vdc_MemcIndexSettings  *pVdcMemConfig;

        NEXUS_Platform_GetStatus(&status);
        stBoxSettings.ulBoxId = status.boxMode;
        rc = BBOX_Open(&hBox, &stBoxSettings);
        if ( rc!=NEXUS_SUCCESS )
        {
            BDBG_ERR(("Failed to open BBOX"));
            goto error;
        }

        BBOX_GetConfig(hBox, &stBoxConfig);
        pVdcMemConfig = &stBoxConfig.stMemConfig.stVdcMemcIndex;
        rulMem = pVdcMemConfig->ulRdcMemcIndex;
        surf0Mem = pVdcMemConfig->astDisplay[0].aulGfdWinMemcIndex[0];
#if (SPLASH_NUM_SURFACE>1)
        surf1Mem = pVdcMemConfig->astDisplay[1].aulGfdWinMemcIndex[0];
#endif
    }
#else
    hBox = NULL;
    rulMem = SPLASH_RUL_MEM;
    surf0Mem = SPLASH_SURF0_MEM;
#if (SPLASH_NUM_SURFACE>1)
    surf1Mem = SPLASH_SURF1_MEM;
#endif
#endif

    /* Copy config from macro to internal ModeHandles struct
     * note: if we add surfaces or display more than 2, here is the
     * only place to port */
    /* PORT POINT: if we have more than 2 surfaces or displays */
    g_stModeHandles.hRulMem = hMma[rulMem];
    g_stModeHandles.iRulMemIdx = rulMem;
    g_stModeHandles.bScaleToFullScreen = bScaleToFullScreen;

    g_stModeHandles.surf[0].ePxlFmt = SPLASH_SURF0_PXL_FMT;
    g_stModeHandles.surf[0].hMma = hMma[surf0Mem];
    g_stModeHandles.surf[0].iMemIdx = surf0Mem;
    strcpy(&g_stModeHandles.surf[0].bmpFile[0], SPLASH_SURF0_BMP);

#if (SPLASH_NUM_SURFACE>1)
    if (surf1Mem != BBOX_MemcIndex_Invalid)
    {
            g_stModeHandles.surf[1].ePxlFmt = SPLASH_SURF1_PXL_FMT;
            g_stModeHandles.surf[1].hMma = hMma[surf1Mem];
            g_stModeHandles.surf[1].iMemIdx = surf1Mem;
            strcpy(&g_stModeHandles.surf[1].bmpFile[0], SPLASH_SURF1_BMP);
    }
#endif

    g_stModeHandles.disp[0].eDispFmt = SPLASH_DISP0_FMT;
    g_stModeHandles.disp[0].pSurf = &g_stModeHandles.surf[SPLASH_DISP0_SUR];
    g_stModeHandles.disp[0].iSurfIdx = SPLASH_DISP0_SUR;
    g_stModeHandles.disp[0].bGfdHasVertScale = false;
#ifdef BCHP_GFD_0_VERT_FIR_SRC_STEP
    g_stModeHandles.disp[0].bGfdHasVertScale = true;
#endif

#if (SPLASH_NUM_DISPLAY>1)
    g_stModeHandles.disp[1].eDispFmt = SPLASH_DISP1_FMT;
    g_stModeHandles.disp[1].pSurf = &g_stModeHandles.surf[SPLASH_DISP1_SUR];
    g_stModeHandles.disp[1].iSurfIdx = SPLASH_DISP1_SUR;
    g_stModeHandles.disp[1].bGfdHasVertScale = false;
#ifdef BCHP_GFD_1_VERT_FIR_SRC_STEP
    g_stModeHandles.disp[1].bGfdHasVertScale = true;
#endif
#endif

    /* override 1st and 2nd display formats by run-time command option, if there is */
    if (overrideFmt)
    {
        const BFMT_VideoInfo *pFmtInfo;

        switch (format)
        {
        case 0:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_eNTSC;
            break;
        case 1:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_ePAL_G;
            break;
        case 2:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e480p;
            break;
        case 3:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e576p_50Hz;
            break;
        case 4:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e720p;
            break;
        case 5:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e720p_50Hz;
            break;
        case 6:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e1080i;
            break;
        case 7:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e1080i_50Hz;
            break;
        case 8:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e1080p;
            break;
        case 9:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_e1080p_50Hz;
            break;
        default:
            g_stModeHandles.disp[0].eDispFmt = BFMT_VideoFmt_eNTSC;
            break;
        }

        /* if HD is 60hz then SD path will be NTSC
         * if HD is 50hz then SD path will be PAL_G */
        pFmtInfo = BFMT_GetVideoFormatInfoPtr(g_stModeHandles.disp[0].eDispFmt);
        g_stModeHandles.disp[1].eDispFmt = (
                        (pFmtInfo->ulVertFreq == (25 * BFMT_FREQ_FACTOR)) ||
                        (pFmtInfo->ulVertFreq == (50 * BFMT_FREQ_FACTOR)))
                ? BFMT_VideoFmt_ePAL_G : BFMT_VideoFmt_eNTSC;
    }

#ifdef SPLASH_SUPPORT_HDM
    rc = BI2C_Open( &i2cHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, NULL);
    if ( rc!=NEXUS_SUCCESS )
    {
        BDBG_ERR(("Failed to open i2c"));
        goto error;
    }
    BI2C_GetChannelDefaultSettings(i2cHandle, NEXUS_I2C_CHANNEL_HDMI_TX, &i2cChanSettings);

    i2cChanSettings.clkRate = BI2C_Clk_eClk100Khz;

    rc = BI2C_OpenChannel(i2cHandle, &i2cChanHandle, NEXUS_I2C_CHANNEL_HDMI_TX, &i2cChanSettings);
    if ( rc!=NEXUS_SUCCESS )
    {
        BDBG_ERR(("Failed to open i2c channel "));
        goto error;
    }

    rc = BI2C_CreateI2cRegHandle(i2cChanHandle, &i2cRegHandle);
    if ( rc!=NEXUS_SUCCESS )
    {
        BDBG_ERR(("Failed to open reg handle"));goto error;
    }
#endif

#ifdef debug_only
splash_generate_script(
              NULL,
              NULL,
              NULL,
              NULL,
              NULL,
              NULL);
#else
splash_generate_script(
              g_pCoreHandles->chp,
              g_pCoreHandles->reg,
              g_pCoreHandles->bint,
              i2cRegHandle,
              hBox);
#endif

error:
#ifndef BVDC_FOR_BOOTUPDATER
#if (BBOX_BOX_MODES_SUPPORTED)
    BBOX_Close(hBox);
#endif
#ifdef SPLASH_SUPPORT_HDM
    BI2C_CloseI2cRegHandle(i2cRegHandle) ;
    BI2C_CloseChannel(i2cChanHandle) ;
    BI2C_Close(i2cHandle) ;
#endif
    NEXUS_Platform_Uninit();
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
    return 0;
}

/* End of File */
