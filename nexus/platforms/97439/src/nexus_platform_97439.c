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
#include "bstd.h"
#include "nexus_platform_priv.h"
#include "nexus_platform_features.h"

BDBG_MODULE(nexus_platform_97439);

/*
 7439 BX support only.
*/
static void nexus_p_modifyDefaultMemoryConfigurationSettings( NEXUS_MemoryConfigurationSettings *pSettings )
{
#if NEXUS_HAS_VIDEO_DECODER
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = true;
    switch (g_pPreInitState->boxMode) {
    case 1:
    case 2:
        /* no mosaic support */
        pSettings->videoDecoder[0].mosaic.maxNumber = 0;
        break;
    default:
        break;
    }
#else
    BSTD_UNUSED(pSettings);
#endif
}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 162*1024*1024;
    pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 112*1024 *1024; /* CABACs(28)for 2 decoders + RAVE CDB(6+15) */
    pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 32*1024*1024;

    switch(boxMode)
    {
         default:
         /* 1 MEMC */
         case 1:
         case 12:
         case 13:
         case 17:
         case 19:
         case 20:
         case 22:
         case 23:
         case 26:
         case 28:
         case 29:
            pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = 124*1024*1024;
            pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
            break;
           /* All boxmodes below have 2 memc's*/
         case 2:
         case 4:
         case 5:
         case 6:
         case 7:
         case 9:
         case 10:
         case 14:
         case 16:
         case 18:
            /* GFD 0, 1,2,3 */
           pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = 0;
           /* Fall through */
         case 3: /* Cases that use default MEMC0 Graphics heap for GFD1 */
         case 21:
         case 24:
         case 25:
         case 27:
         case 30:
         case 31:
           pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].size = 256*1024*1024;
           pSettings->heap[NEXUS_MEMC1_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
           pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS;
           break;
    }

    pSettings->i2c[0].clock.type = NEXUS_GpioType_eAonSpecial;
    pSettings->i2c[0].clock.gpio = 0;
    pSettings->i2c[0].data.type = NEXUS_GpioType_eAonSpecial;
    pSettings->i2c[0].data.gpio = 1;
    pSettings->i2c[3].clock.type = NEXUS_GpioType_eSpecial;
    pSettings->i2c[3].clock.gpio = 0;
    pSettings->i2c[3].data.type = NEXUS_GpioType_eSpecial;
    pSettings->i2c[3].data.gpio = 1;

    return;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
   const char *board;
   /* TODO: some day this needs to become run-time vs. compile time. read product ID */
#if defined NEXUS_USE_7252S_VMS_SFF
    board = "7252S VMS SFF";
#elif defined NEXUS_USE_3390_VMS
    board = "93390 VMS";
#elif defined NEXUS_USE_7439_SFF
    board = "SFF board";
#elif defined NEXUS_USE_7439_SV_DR3
    board= "Use this only with 7251S chips on 7449SSV_DR3 Socket boards"
#elif (defined NEXUS_USE_7439_SV || defined  NEXUS_USE_7449_SV)
    board = "SV board";
#elif defined NEXUS_USE_7252S_SAT
    board = "SAT board";
#elif defined NEXUS_USE_7449S_CWM
    board = "SAT board";
#else
    board = "7439 Bx Based";
#endif
    BDBG_WRN(("*** Initializing %s Board ...***", board));
    return 0;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}
