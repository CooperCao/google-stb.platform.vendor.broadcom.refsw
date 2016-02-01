/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if !NEXUS_HAS_VIDEO_DECODER
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#else
#include "nexus_core_utils.h"
#include "nexus_surface.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(memconfig);

/**
This app queries the default heaps per platform, typically one heap per memory controller.
**/

static const char *get_heap_name(unsigned i, const NEXUS_PlatformSettings *pPlatformSettings)
{
    if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_MAIN) {
        return "main";
    }
    else if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) {
        return "picture buffers";
    }
    else if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_GRAPHICS) {
        return "graphics";
    }
    else if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS) {
        return "secondary graphics";
    }
    else if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_COMPRESSED_RESTRICTED_REGION) {
        return "video secure";
    }
    else if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_SAGE_RESTRICTED_REGION) {
        return "sage";
    }

    /* if no name, at least return the memoryType */
    switch (pPlatformSettings->heap[i].memoryType) {
    case NEXUS_MemoryType_eDriver: return "eDriver mapping";
    case NEXUS_MemoryType_eFull: return "eFull mapping";
    case NEXUS_MemoryType_eApplication: return "eApp mapping";
    case NEXUS_MemoryType_eDeviceOnly: return "eDevice mapping";
    default:
        {
        static char g_str[32];
        BKNI_Snprintf(g_str, 32, "0x%x mapping", pPlatformSettings->heap[i].memoryType);
        return g_str; /* NOTE: not reentrant */
        }
    }
}

void print_heaps(const NEXUS_PlatformSettings *pPlatformSettings)
{
    NEXUS_PlatformConfiguration platformConfig;
    unsigned i;
    NEXUS_Error rc;

    /* call NEXUS_Platform_GetConfiguration to get the heap handles */
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* example of getting status about each heap */
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus status;
        NEXUS_HeapHandle heap;

        heap = platformConfig.heap[i];
        if (!heap) continue;
        rc = NEXUS_Heap_GetStatus(heap, &status);
        BDBG_ASSERT(!rc);
        printf("heap[%d]: MEMC%d, offset 0x%08x, size %9d (%3d MB), %s\n",
            i, status.memcIndex, (unsigned)status.offset, status.size, status.size/1024/1024, get_heap_name(i, pPlatformSettings));
    }
}

static void mem_no_transcode(NEXUS_MemoryConfigurationSettings *pSettings)
{
#if NEXUS_HAS_VIDEO_ENCODER
    unsigned i;
    for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
        pSettings->videoEncoder[i].used = false;
    }
#else
    BSTD_UNUSED(pSettings);
#endif
}

static void mem_single_decode(NEXUS_MemoryConfigurationSettings *pSettings)
{
    unsigned i, j;
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
            pSettings->display[i].window[j].used = i<1;
        }
    }
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        pSettings->videoDecoder[i].used = i<1;
    }
}

static void mem_no_mvc_decode(NEXUS_MemoryConfigurationSettings *pSettings)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false;
    }
}

static void mem_no_hevc_4k_10bit(NEXUS_MemoryConfigurationSettings *pSettings)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        pSettings->videoDecoder[i].maxFormat = NEXUS_VideoFormat_e1080i;
        pSettings->videoDecoder[i].colorDepth = 8;
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH265] = false;
    }
}

static void mem_single_display(NEXUS_MemoryConfigurationSettings *pSettings)
{
    unsigned i, j;
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
            pSettings->display[i].window[j].used = i<1 && j<1;
        }
    }
}

static void mem_minimum(NEXUS_MemoryConfigurationSettings *pSettings)
{
    unsigned i, j;
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
            pSettings->display[i].window[j].used = i<1 && j<1;
            if (pSettings->display[i].window[j].used) {
                pSettings->display[i].maxFormat = NEXUS_VideoFormat_e1080i;
                pSettings->display[i].window[j].convertAnyFrameRate = false;
                pSettings->display[i].window[j].precisionLipSync = false;
                pSettings->display[i].window[j].capture = false;
                pSettings->display[i].window[j].deinterlacer = false;
            }
        }
    }
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        pSettings->videoDecoder[i].used = i<1;
        pSettings->videoDecoder[i].maxFormat = NEXUS_VideoFormat_e1080i;
        pSettings->videoDecoder[i].colorDepth = 8;
        memset(pSettings->videoDecoder[i].supportedCodecs, 0, sizeof(pSettings->videoDecoder[i].supportedCodecs));
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eMpeg2] = true;
        pSettings->videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH264] = true;
    }
#if NEXUS_HAS_VIDEO_ENCODER
    for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
        pSettings->videoEncoder[i].used = false;
    }
#endif
}

static void print_estimate(void)
{
    int rc;
    unsigned i;
    NEXUS_PlatformStatus status;
    rc = NEXUS_Platform_GetStatus(&status);
    BDBG_ASSERT(!rc);
    for (i=0;i<NEXUS_NUM_MEMC;i++) {
        printf(
        "Estimated MEMC%d:\n"
        "VideoDecoder general %d, secure %d\n"
        "VideoEncoder general %d, secure %d, firmware %d, index %d, data %d\n"
        "Display      general %d\n",
        i,
        status.estimatedMemory.memc[i].videoDecoder.general,
        status.estimatedMemory.memc[i].videoDecoder.secure,
        status.estimatedMemory.memc[i].videoEncoder.general,
        status.estimatedMemory.memc[i].videoEncoder.secure,
        status.estimatedMemory.memc[i].videoEncoder.firmware,
        status.estimatedMemory.memc[i].videoEncoder.index,
        status.estimatedMemory.memc[i].videoEncoder.data,
        status.estimatedMemory.memc[i].display.general);
    }
}

int main(int argc, char **argv)
{
    int curarg = 1;
    bool estimate = false;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-estimate")) {
            estimate = true;
        }
    }

    while (1) {
        NEXUS_PlatformSettings platformSettings;
        NEXUS_MemoryConfigurationSettings memConfigSettings;
        int rc;
        unsigned config;

        char buf[64];
        printf(
            "Select config:\n"
            "1) Highest: Everything enabled (default)\n"
            "2) No transcode\n"
            "3) Single decode (plus #2)\n"
            "4) No MVC (plus #3)\n"
            "5) No HEVC, 4K, 10 bit (plus #4)\n"
            "6) Single display (plus #5)\n"
            "7) Lowest: Single decode, single display, no MAD, no capture\n"
            "q) Quit\n"
            );
        fgets(buf, sizeof(buf), stdin);
        if (buf[0] == 'q') break;
        config = atoi(buf);
        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
        switch (config) {
        case 7:
            /* for minimum, set everything in one function */
            mem_minimum(&memConfigSettings);
            break;

        /* the following functions cascade */
        case 6:
            mem_single_display(&memConfigSettings);
            /* fall through */
        case 5:
            mem_no_hevc_4k_10bit(&memConfigSettings);
            /* fall through */
        case 4:
            mem_no_mvc_decode(&memConfigSettings);
            /* fall through */
        case 3:
            mem_single_decode(&memConfigSettings);
            /* fall through */
        case 2:
            mem_no_transcode(&memConfigSettings);
            break;

        case 1:
            /* no change. maximum capabilities. */
            break;

        default:
            printf("### Invalid selection\n");
            continue;
        }

        NEXUS_SetEnv("NEXUS_BASE_ONLY_INIT","y");
        rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
        BDBG_ASSERT(!rc);

        BKNI_Sleep(10); /* allow logger to print */
        print_heaps(&platformSettings);
        if (estimate) {
            print_estimate();
        }

        NEXUS_Platform_Uninit();
    }
    return 0;
}
#endif /* !NEXUS_HAS_VIDEO_DECODER  */
