/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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

#if NEXUS_HAS_GRAPHICS2D
#include "bstd.h"
#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"

BDBG_MODULE(performance_test);

/**
Test runs in three modes: user mode single-process, user mode multi-process, kernel mode
Test results are posted on Broadcom-internal twiki http://twiki-01.broadcom.com/bin/view/Bseavsw/NexusMultiProcessPerformance.
**/

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}


#define TOTAL_LOOPS 5
#define TOTAL_TESTS_PER_LOOP 100000
unsigned g_timing_start;

#define START_TIMING() do { \
    g_timing_start = b_get_time(); \
    } while(0)
#define CHECK_TIMING() do { \
    if (i && i % TOTAL_TESTS_PER_LOOP == 0) { \
        unsigned end = b_get_time(); \
        BDBG_WRN(("%d in %d msec", TOTAL_TESTS_PER_LOOP, end - g_timing_start)); \
        g_timing_start = end; \
    } \
    } while(0)

int main(void)
{
    BKNI_EventHandle event;
    int rc;
    unsigned i;

#if NEXUS_MODE_client
    rc = NEXUS_Platform_Join();
    if (rc) return -1;
#else
    {
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    }
#endif
    BKNI_CreateEvent(&event);

    BDBG_WRN(("test 1: round trip time"));
    {
        NEXUS_SurfaceCreateSettings createSettings;

        START_TIMING();
        for (i=0;i<TOTAL_LOOPS*TOTAL_TESTS_PER_LOOP;i++) {
            NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
            CHECK_TIMING();
        }
    }

    BDBG_WRN(("test 1.5: graphics2d round trip time"));
    {
        NEXUS_Graphics2DOpenSettings openSettings;

        START_TIMING();
        for (i=0;i<TOTAL_LOOPS*TOTAL_TESTS_PER_LOOP;i++) {
            NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
            CHECK_TIMING();
        }
    }

    BDBG_WRN(("test 2: interrupt service + callback time"));
    {
        NEXUS_Graphics2DHandle gfx;
        NEXUS_Graphics2DSettings  gfxSettings;
        NEXUS_Graphics2DBlitSettings blitSettings;
        NEXUS_SurfaceHandle surface;
        NEXUS_SurfaceCreateSettings createSettings;

        /* do smallest possible blit */
        gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
        NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
        gfxSettings.checkpointCallback.callback = complete;
        gfxSettings.checkpointCallback.context = event;
        NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.width = 100;
        createSettings.height = 100;
        surface = NEXUS_Surface_Create(&createSettings);
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = surface;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = 1;
        blitSettings.source.rect.height = 1;
        blitSettings.output.surface = surface;
        blitSettings.output.rect = blitSettings.source.rect;

        START_TIMING();
        for (i=0;i<TOTAL_LOOPS*TOTAL_TESTS_PER_LOOP;i++) {
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);
            rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                rc = BKNI_WaitForEvent(event, 0xffffffff);
                BDBG_ASSERT(!rc);
            }
            CHECK_TIMING();
        }

        NEXUS_Graphics2D_Close(gfx);
        NEXUS_Surface_Destroy(surface);
    }

    BDBG_WRN(("done"));    
    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_GRAPHICS2D */
int main(void)
{
    printf("This application is not supported on this platform (needs graphics2d)!\n");
    return 0;
}
#endif
