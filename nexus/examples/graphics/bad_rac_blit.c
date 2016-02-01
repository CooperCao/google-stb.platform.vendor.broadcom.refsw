/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_memory.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

BDBG_MODULE(blit);

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

void *buffer;
unsigned test_number = 0;
unsigned page_size = 8192;
unsigned surface_offset = 128;

void *cpu_read_func(void *context)
{
    BSTD_UNUSED(context);
    while (1) {
        volatile uint32_t pixel; /* volatile makes sure reads aren't optimized out */
        if (test_number != 1) {
            /* CPU read from a part of the buffer outside of the surface, but within the same page
            as the surface. this will fail. */
            pixel = ((uint32_t*)buffer)[(page_size + (rand() % surface_offset)) / sizeof(pixel)];
        }
        else {
            /* CPU read from a part of the buffer outside of the surface and outside the
            page of the surface. this will work. this is the default RAC page alignment
            in NEXUS_SurfaceCreateSettings.alignment. */
            pixel = ((uint32_t*)buffer)[(rand() % page_size) / sizeof(pixel)];
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle event;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Error rc;
    unsigned count = 0;
    NEXUS_MemoryAllocationSettings allocSettings;
    pthread_t thread;
    volatile uint32_t pixel; /* volatile makes sure reads aren't optimized out */

    /**
    This app demonstrates the recommendations in nexus_dma.h regarding RAC page alignment and the RAC.

    3 test scenarios:
    
    "nexus bad_rac_blit 0"
        result: this will fail - don't use RAC aligned surface and don't flush between HW write and CPU read
    "nexus bad_rac_blit 1"
        result: this will pass. uses RAC alignment surfaces.
    "nexus bad_rac_blit 2"
        result: this will pass. flush between HW write and CPU read.

    notes:
    some RAC's are 4K and others are 8K. 2nd param gives option.
    your surface_offset must be >= the L1 cache size, otherwise you are just testing L1 cache coherency. 3rd param gives option.
    **/
    if (argc > 1) {
        test_number = atoi(argv[1]); /* select 0, 1 or 2 */
    }
    if (argc > 2) {
        page_size = atoi(argv[2]); /* optional. you can try different page sizes. */
    }
    if (argc > 3) {
        surface_offset = atoi(argv[3]); /* optional. you can try different offsets. */
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    allocSettings.alignment = page_size;
    rc = NEXUS_Memory_Allocate(page_size * 2, &allocSettings, &buffer);
    BDBG_ASSERT(!rc);

    if (page_size*2 - surface_offset < 10*10*sizeof(pixel)) {
        printf("need to alloc enough memory for page + offset + 400 byte surface");
        return -1;
    }

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 10;
    createSettings.height = 10;
    /* put surface into the second page */
    createSettings.pMemory = (uint8_t*)buffer + page_size + surface_offset;
    createSettings.alignment = 0; /* don't have nexus provide any additional alignment */
    surface = NEXUS_Surface_Create(&createSettings);
    BDBG_WRN(("memory: buffer %p, surface addr %p, surface size %#x", buffer, createSettings.pMemory, createSettings.width*createSettings.height*sizeof(uint32_t)));

    BKNI_CreateEvent(&event);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    pthread_create(&thread, NULL, cpu_read_func, NULL);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    while (1) {
        if (test_number != 2) {
            /* it is a mistake to put the flush here */
            NEXUS_Surface_Flush(surface);
        }

        /* blit into surface. this has no reference to the CPU L1 cache or RAC. */
        fillSettings.surface = surface;
        fillSettings.color = rand();
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            BKNI_WaitForEvent(event, 0xffffffff);
        }

        if (test_number == 2) {
            /* if you flush between HW write and CPU read, it will work, regardless of alignment.
            this is the recommendation of nexus_dma.h. */
            NEXUS_Surface_Flush(surface);
        }

        /* CPU read from surface. this will read from the RAC. */
        pixel = ((uint32_t*)createSettings.pMemory)[0];
        if (pixel != fillSettings.color) {
            fprintf(stderr, "%d: failure %08x != %08x\n", count, pixel, fillSettings.color);
            return -1;
        }

        if (++count % 1000 == 0) BDBG_WRN(("%d tries", count));
    }

    return 0;
}
#else /* NEXUS_HAS_GRAPHICS2D */
int main(void)
{
    printf("ERROR: NEXUS_Graphics2D not supported\n");
    return -1;
}
#endif
