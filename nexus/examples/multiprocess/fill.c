/******************************************************************************
 *  Copyright (C) 2008-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/* Nexus example app: multi-process fill.

A parent process calls NEXUS_Platform_Init and opens display, blitter and framebuffer.
Child processes call NEXUS_Platform_Join and opens blitter and off-screen surface.

hared memory is used for IPC to allow the children to blit into the framebuffer.
Each child uses two mutexes to synchronize with the parent. */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>

BDBG_MODULE(fill);

void parent_process(sem_t **mutex);
void child_process(unsigned idx, sem_t **mutex);

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}


/* global variable in shared memory */
uint32_t *g_offset;

#define NUM_CHILD_PROCESSES 6
#define CHILD_SURFACE_WIDTH  320
#define CHILD_SURFACE_HEIGHT 480

#define FRAMEBUFFER_WIDTH    720
#define FRAMEBUFFER_HEIGHT   480
#define PIXEL_FORMAT         NEXUS_PixelFormat_eA8_R8_G8_B8

int main(void)
{
    bool is_child = false;
    pid_t pID[NUM_CHILD_PROCESSES];
    unsigned *index;
    unsigned i;
    sem_t **mutex;

    BKNI_Init();

    /* mutex[n][0] blocks the n-th child. mutex[n][1] blocks the parent */
    mutex = mmap(NULL, sizeof(sem_t*)*NUM_CHILD_PROCESSES, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
    for (i=0; i<NUM_CHILD_PROCESSES; i++) {
        mutex[i] = mmap(NULL, sizeof(sem_t)*2, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
    }

    /* index is used to assign a unique index to each child */
    index = mmap(NULL, sizeof(unsigned), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
    *index = 0;

    /* g_offset is used to store the device offsets of the surface created by each child */
    g_offset = mmap(NULL, sizeof(uint32_t)*NUM_CHILD_PROCESSES, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, 0, 0);
    BKNI_Memset(g_offset, 0, sizeof(uint32_t)*NUM_CHILD_PROCESSES);

    for (i=0; i<NUM_CHILD_PROCESSES; i++) {
        if (sem_init(&mutex[i][0],1,0) < 0) {
            BDBG_ERR(("failed sem_init"));
            exit(1);
        }
        if (sem_init(&mutex[i][1],1,0) < 0) {
            BDBG_ERR(("failed sem_init"));
            exit(1);
        }
    }

    /* fork some child processes */
    for (i=0; i<NUM_CHILD_PROCESSES; i++) {
        pID[i] = fork();
        if (pID[i] == 0) { /* child */
            is_child = true;
            break;
        }
        else if (pID[i] < 0) {
            BDBG_ERR(("failed fork"));
            exit(1);
        }
    }

    if (!is_child) {
        parent_process(mutex);
    }
    else {
        child_process((*index)++, mutex);
    }

    return 0;
}

void parent_process(sem_t **mutex)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle framebuffer, offscreenSurface[NUM_CHILD_PROCESSES];
    NEXUS_Graphics2DHandle gfx;
    NEXUS_GraphicsSettings gfxSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Rect rect;
    NEXUS_MemoryStatus memStatus;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_Error rc;
    void *addr;
    unsigned i, j;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    BDBG_MSG(("parent_process: platform init"));
    NEXUS_Platform_GetConfiguration(&platformConfig);

    rc = NEXUS_Platform_StartServer(NULL);
    BDBG_ASSERT(!rc);

    /* Children can call NEXUS_Platform_Join now */
    for (i=0; i<NUM_CHILD_PROCESSES; i++) {
        sem_post(&mutex[i][0]);
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
		}
    }
#endif

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

    /* Create the framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = PIXEL_FORMAT;
    createSettings.width = FRAMEBUFFER_WIDTH; /* The width is fixed and will be upscaled */
#if NEXUS_HAS_GFD_VERTICAL_UPSCALE
    createSettings.height = FRAMEBUFFER_HEIGHT; /* The height is fixed and will be upscaled */
#else
    createSettings.height = 1080; /* The maximum height is 1080. For formats with height<1080, we clip. */
#endif
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    if (!createSettings.heap) createSettings.heap = platformConfig.heap[0];
    framebuffer = NEXUS_Surface_Create(&createSettings);

    /* Wait for all children to create their surfaces */
    for (i=0; i<NUM_CHILD_PROCESSES; i++) {
        sem_wait(&mutex[i][1]);
        BDBG_MSG(("parent_process: offset[%d] %x", i, g_offset[i]));
    }

    /* Create the offscreen surfaces that map to the child processes' surfaces */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = PIXEL_FORMAT;
    createSettings.width = CHILD_SURFACE_WIDTH;
    createSettings.height = CHILD_SURFACE_HEIGHT;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    if (!createSettings.heap) createSettings.heap = platformConfig.heap[0];
    rc = NEXUS_Heap_GetStatus(createSettings.heap, &memStatus);
    BDBG_ASSERT(!rc);

    for (i=0; i<NUM_CHILD_PROCESSES; i++) {
        addr = g_offset[i] - memStatus.offset + (uint8_t*)memStatus.addr;
        createSettings.pMemory = addr;
        offscreenSurface[i] = NEXUS_Surface_Create(&createSettings);
    }

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Display_GetGraphicsSettings(display, &gfxSettings);
    BDBG_ASSERT(gfxSettings.enabled==true);

    /* The position rect sets the destination and is always equal to the full display */
    gfxSettings.position.width = videoFormatInfo.width;
    gfxSettings.position.height = videoFormatInfo.height;

    /* The clipping is the portion of the framebuffer we will use. */
    gfxSettings.clip.width = FRAMEBUFFER_WIDTH; /* the difference between clip.width and position.width will trigger a GFD horizontal upscale. */
#if NEXUS_HAS_GFD_VERTICAL_UPSCALE
    gfxSettings.clip.height = FRAMEBUFFER_HEIGHT; /* the difference between clip.height and position.height will trigger a GFD vertical upscale. */
#else
    gfxSettings.clip.height = videoFormatInfo.height; /* there is no GFD vertical upscale, so this must equal position.height. */
#endif
    rc = NEXUS_Display_SetGraphicsSettings(display, &gfxSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
    BDBG_ASSERT(!rc);

    /* Tell children that parent is ready to blit */
    for (i=0; i<NUM_CHILD_PROCESSES; i++) {
        sem_post(&mutex[i][0]);
    }

    rect.x = 0;
    rect.y = 0;
    rect.width = FRAMEBUFFER_WIDTH / NUM_CHILD_PROCESSES;
    rect.height = videoFormatInfo.height;

    for (j=0;;j++) {
        for (i=0; i<NUM_CHILD_PROCESSES; i++) {
            sem_wait(&mutex[i][1]); /* wait for child to finish its fills */

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = offscreenSurface[i]; /* always blit from the whole offscreen surface */
            blitSettings.output.surface = framebuffer;
            rect.x = FRAMEBUFFER_WIDTH / NUM_CHILD_PROCESSES * i;
            blitSettings.output.rect = rect; /* blit into the clipped region of the framebuffer that will be displayed. */
            blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
            rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
            BDBG_ASSERT(!rc);

            BDBG_MSG(("parent_process: blit[%d]", i));
            sem_post(&mutex[i][0]);
        }

        if (j && j%50==0) {
            BDBG_ERR(("(%d * %d) blits completed by parent process", j, NUM_CHILD_PROCESSES));
        }
    }

}

void child_process(unsigned idx, sem_t **mutex)
{
    NEXUS_Error rc;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle offscreenSurface;
    NEXUS_SurfaceMemory mem;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_MemoryStatus memStatus;
    NEXUS_Graphics2DFillSettings fillSettings;
    BKNI_EventHandle event;
    unsigned i;

    BKNI_CreateEvent(&event);

    /* wait for parent to initialize before calling Join */
    sem_wait(&mutex[idx][0]);
    rc = NEXUS_Platform_Join();
    if (rc) {
        BDBG_ERR(("child_process[%d]: unable to join, err %d", idx, rc));
        exit(1);
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = event;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);


    /* Create offscreen surface, which each child process will fill */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = PIXEL_FORMAT;
    createSettings.width = CHILD_SURFACE_WIDTH;
    createSettings.height = CHILD_SURFACE_HEIGHT;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    if (!createSettings.heap) createSettings.heap = platformConfig.heap[0];
    offscreenSurface = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_GetMemory(offscreenSurface, &mem);
    rc = NEXUS_Heap_GetStatus(createSettings.heap, &memStatus);
    BDBG_ASSERT(!rc);

    g_offset[idx] = memStatus.offset + (uint8_t*)mem.buffer - (uint8_t*)memStatus.addr;
    BDBG_MSG(("child_process[%d]: pid %x, surface %p, offset %x", idx, getpid(), mem.buffer, g_offset[idx]));

    /* Tell parent that child has updated its offset */
    sem_post(&mutex[idx][1]);

    /* Wait for parent to get ready */
    sem_wait(&mutex[idx][0]);

    for (i=0;;i++) {
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = offscreenSurface;
        fillSettings.rect.x = rand() % (createSettings.width-20);
        fillSettings.rect.y = rand() % (createSettings.height-20);
        fillSettings.rect.width = 20;
        fillSettings.rect.height = 20;
        fillSettings.colorOp = NEXUS_FillOp_eCopy;
        fillSettings.alphaOp = NEXUS_FillOp_eCopy;
        fillSettings.color = 0xFF000000 | (rand()%0xFF<<((idx%3)*8));

        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        if (rc) {
            /* fill can fail because an internal queue is full. wait for all fills to complete, then continue. */
            rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED) {
                BKNI_WaitForEvent(event, 0xffffffff);
            }
        }

        if (i%500==0) {
            BDBG_MSG(("child_process[%d]: fill", idx));

            /* Tell the parent to blit and wait before proceeding with more fills */
            sem_post(&mutex[idx][1]);
            sem_wait(&mutex[idx][0]);
        }
    }
}
#else /* NEXUS_HAS_GRAPHICS2D */
int main(void)
{
    printf("This application is not supported on this platform (needs graphics2d)!\n");
    return 0;
}
#endif

