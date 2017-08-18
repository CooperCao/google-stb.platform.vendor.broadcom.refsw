/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#if NEXUS_HAS_INPUT_ROUTER && NEXUS_HAS_TRANSPORT && NEXUS_HAS_VIDEO_DECODER
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "binput.h"
#include "picdecoder.h"
#include <sys/stat.h>
#include <dirent.h>
#include "blst_queue.h"
#include "thumbdecoder.h"
#include "bfont.h"
#include <pthread.h>

BDBG_MODULE(playviewer);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(void)
{
    printf(
    "'playviewer' is a thumbnail-based GUI for stream playback\n"
    "\n"
    "Options:\n"
    "  --help or -h for help\n"
    "  -view {full,text,thumb} default is full\n"
    "  -dir DIRECTORY          directory to scan. default is ./videos\n"
    "  -background IMAGE       default is nxclient/desktop_background.png\n"
    "  -r                      scan subdirectories recursively\n"
    );
}

struct file {
    BLST_Q_ENTRY(file) link;
    char streamname[256];
    char indexname[256];
    char label[64];
    NEXUS_SurfaceHandle thumbnail;
    NEXUS_Rect rect;
    struct {
        unsigned x, y;
    } coord;
};

struct page {
    BLST_Q_ENTRY(page) link;
    struct file *firstFile;
};

enum b_viewmode {
    b_viewmode_full,
    b_viewmode_text,
    b_viewmode_thumb,
    b_viewmode_max
} b_viewmode;

struct appcontext {
    bool done;
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle checkpointEvent, displayedEvent;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceHandle background;
    binput_t input;
    enum b_viewmode view;
    bool ui_update;

    BLST_Q_HEAD(filelist, file) files;
    struct file *currentFile;
    bfont_t font;
    unsigned font_height;
    pthread_t thread;
    bool hidden;

    BLST_Q_HEAD(pagelist, page) pages;
    struct page *currentPage;

    thumbdecoder_t thumbdec;
};

static void render_ui(struct appcontext *pContext);
static void launch_client(struct appcontext *pContext);

static int b_read_dir(const char *dirstr, int recursive, struct filelist *filelist);
static void b_free_dir(struct filelist *filelist);
static int b_start_thumbnails(struct appcontext *pContext);
static void b_stop_thumbnails(struct appcontext *pContext);

static void find_next(struct appcontext *pContext, int y_inc, int x_inc)
{
    struct file *file, *nextPF, *prevPF;
    struct page *p;
    BDBG_ASSERT(pContext->currentFile);

    /* Determine the last file of the previous page */
    prevPF = BLST_Q_PREV(pContext->currentPage->firstFile, link);

    /* Find the first file of the next page */
    p = BLST_Q_NEXT(pContext->currentPage, link);
    if (p) {
        nextPF = p->firstFile;
    }
    else {
        nextPF = NULL;
    }

    if (y_inc == -1) {
        /* up */
        file = BLST_Q_PREV(pContext->currentFile, link);
        if (!file) {
            file = pContext->currentFile;
        }
    }
    else if (y_inc == 1) {
        /* down */
        file = BLST_Q_NEXT(pContext->currentFile, link);
        if (!file) {
            file = pContext->currentFile;
        }
    }
    else if (x_inc) {
        for (file = pContext->currentPage->firstFile; file; file = BLST_Q_NEXT(file, link)) {
            if (file==nextPF) {
                /* Don't search past end of current page */
                file = NULL;
                break;
            }
            else if (file->coord.x == pContext->currentFile->coord.x + x_inc &&
                file->coord.y == pContext->currentFile->coord.y + y_inc) {
                break;    
            }
        }
        if (!file) {
            find_next(pContext, x_inc, 0);
            return;
        }
    }
    else {
        return;
    }
    BDBG_ASSERT(file);
    pContext->currentFile = file;

    /* See if we have moved to the first file of the next page, or the last
     * file of the previous page, and change pages accordingly.
     */ 
    p = NULL;
    if (file==prevPF) {
        p = BLST_Q_PREV(pContext->currentPage, link);
    }
    else if (file==nextPF) {
        p = BLST_Q_NEXT(pContext->currentPage, link);
    }
    if (p) {
        pContext->currentPage = p;
    }
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_ClientConfiguration clientConfig;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_SurfaceClientSettings client_settings;
    NEXUS_Error rc;
    int curarg = 1;
    int recursive = 0;
    struct page *p;
    struct appcontext appcontext, *pContext = &appcontext;
#if NEXUS_HAS_PICTURE_DECODER
    const char *background = "nxclient/desktop_background.png";
#endif
    const char *fontname = "nxclient/arial_18_aa.bwin_font";
    const char *dir = NULL;
    bool done = false;

    memset(pContext, 0, sizeof(*pContext));
    pContext->view = b_viewmode_full;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
#if NEXUS_HAS_PICTURE_DECODER
        else if (!strcmp(argv[curarg], "-background") && argc>curarg+1) {
            background = argv[++curarg];
        }
#endif
        else if (!strcmp(argv[curarg], "-dir") && argc>curarg+1) {
            dir = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-r")) {
            recursive = 1;
        }
        else if (!strcmp(argv[curarg], "-view") && argc>curarg+1) {
            ++curarg;
            if (!strcmp(argv[curarg],"text")) {
                pContext->view = b_viewmode_text;
            }
            else if (!strcmp(argv[curarg],"thumb")) {
                pContext->view = b_viewmode_thumb;
            }
            else {
                pContext->view = b_viewmode_full;
            }
        }
        else if (!dir) {
            dir = argv[curarg];
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }
    if (!dir) {
        dir = "videos";
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    rc = b_read_dir(dir, recursive, &pContext->files);
    if (rc) return -1;

    pContext->currentFile = BLST_Q_FIRST(&pContext->files);

    /* Allocate and assign first of our pages */
    p = BKNI_Malloc(sizeof(*p));
    memset(p, 0, sizeof(*p));
    p->firstFile = pContext->currentFile;
    BLST_Q_INSERT_TAIL(&pContext->pages, p, link);
    pContext->currentPage = p;

    NEXUS_Platform_GetClientConfiguration(&clientConfig);
    BKNI_CreateEvent(&pContext->checkpointEvent);
    BKNI_CreateEvent(&pContext->displayedEvent);

    pContext->font = bfont_open(fontname);
    if (!pContext->font) {
        BDBG_WRN(("unable to load font %s", fontname));
        pContext->view = b_viewmode_thumb;
    }
    else {
        bfont_get_height(pContext->font, &pContext->font_height);
    }
    pContext->input = binput_open(NULL);
    BDBG_ASSERT(pContext->input);
    
    if (pContext->view != b_viewmode_text) {
        b_start_thumbnails(pContext);
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    pContext->blit_client = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (!pContext->blit_client) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire failed"));
        return -1;
    }

    NEXUS_SurfaceClient_GetSettings(pContext->blit_client, &client_settings);
    client_settings.displayed.callback = complete;
    client_settings.displayed.context = pContext->displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(pContext->blit_client, &client_settings);
    BDBG_ASSERT(!rc);

    pContext->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(pContext->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = pContext->checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(pContext->gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 1280;
    createSettings.height = 720;
    createSettings.heap = clientConfig.heap[NXCLIENT_SECONDARY_GRAPHICS_HEAP]; /* if NULL, will use NXCLIENT_DEFAULT_HEAP */
    pContext->surface = NEXUS_Surface_Create(&createSettings);
    pContext->ui_update = true;

#if NEXUS_HAS_PICTURE_DECODER
    if (background) {
        picdecoder_t handle;
        handle = picdecoder_open();
        if (handle) {
            pContext->background = picdecoder_decode(handle, background);
            picdecoder_close(handle);
        }
    }
#endif

    while (!done) {
        b_remote_key key;
        
        /* if a render is still pending, it was a background request */
        if (pContext->ui_update) {
            render_ui(pContext);
        }
        
        if (binput_read_no_repeat(pContext->input, &key)) {
            binput_wait(pContext->input, 1000);
            continue;
        }
        switch (key) {
        case b_remote_key_left:
            find_next(pContext, 0, -1);
            render_ui(pContext);
            break;
        case b_remote_key_up:
            find_next(pContext, -1, 0);
            render_ui(pContext);
            break;
        case b_remote_key_right:
            find_next(pContext, 0, 1);
            render_ui(pContext);
            break;
        case b_remote_key_down:
            find_next(pContext, 1, 0);
            render_ui(pContext);
            break;
        case b_remote_key_select:
            launch_client(pContext);
            render_ui(pContext);
            break;
        case b_remote_key_stop:
        case b_remote_key_back:
        case b_remote_key_clear:
            done = true;
            break;
        default:
            break;
        }
    }

    b_stop_thumbnails(pContext);
    if (pContext->font) {
        bfont_close(pContext->font);
    }
    while ((p = BLST_Q_FIRST(&pContext->pages))) {
        BLST_Q_REMOVE(&pContext->pages, p, link);
        BKNI_Free(p);
    }
    b_free_dir(&pContext->files);
    NEXUS_SurfaceClient_Release(pContext->blit_client);
    NEXUS_Surface_Destroy(pContext->surface);
    if (pContext->background) {
        NEXUS_Surface_Destroy(pContext->background);
    }
    NEXUS_Graphics2D_Close(pContext->gfx);
    BKNI_DestroyEvent(pContext->displayedEvent);
    BKNI_DestroyEvent(pContext->checkpointEvent);
    binput_close(pContext->input);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}

static void render_ui(struct appcontext *pContext)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    int rc;
    struct file *file;
    NEXUS_Rect rect;
    unsigned top_y = 50, top_x = 50, x = 0, y = 0;
    struct bfont_surface_desc desc;
    
    pContext->ui_update = false;
    if (pContext->hidden) {
        return;
    }
    
    bfont_get_surface_desc(pContext->surface, &desc);
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);

    if (pContext->background) {
        blitSettings.source.surface = pContext->background;
        blitSettings.output.surface = pContext->surface;
        rc = NEXUS_Graphics2D_Blit(pContext->gfx, &blitSettings);
        BDBG_ASSERT(!rc);
    }
    else {
        fillSettings.surface = pContext->surface;
        fillSettings.color = 0;
        rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
        BDBG_ASSERT(!rc);
    }

#define THUMB_WIDTH 120
#define THUMB_HEIGHT 120
#define FULL_THUMB_WIDTH 60
#define FULL_THUMB_HEIGHT 60
#define SPACING 10
#define FOCUS_WIDTH 3

    rect.x = top_x;
    rect.y = top_y;
    
    switch (pContext->view) {
    case b_viewmode_text:
        rect.width = 240;
        rect.height = pContext->font_height;
        break;
    case b_viewmode_thumb:
        rect.width = THUMB_WIDTH;
        rect.height = THUMB_HEIGHT;
        break;
    default: /* full */
        rect.width = FULL_THUMB_WIDTH + SPACING + 150;
        rect.height = FULL_THUMB_HEIGHT;
        break;
    }
    
    /* blit thumbnails and focus outline */
    for (file = pContext->currentPage->firstFile; file; file = BLST_Q_NEXT(file, link)) {
        if (rect.x > 1280 - rect.width) {
            /* We've gone past the end of the current page,
             * check if the next page exists, if not add it.
             */
            struct page *p;
            p = BLST_Q_NEXT(pContext->currentPage, link);
            if (!p) {
                p = BKNI_Malloc(sizeof(*p));
                memset(p, 0, sizeof(*p));
                p->firstFile = file;
                BLST_Q_INSERT_TAIL(&pContext->pages, p, link);
            }
            break;
        }

        if (pContext->view != b_viewmode_text) {
            NEXUS_Rect thumbrect = rect;
            if (pContext->view == b_viewmode_full) {
                thumbrect.width = FULL_THUMB_WIDTH;
            }
            
            if (file->thumbnail) {
                blitSettings.source.surface = file->thumbnail;
                blitSettings.output.surface = pContext->surface;
                blitSettings.output.rect = thumbrect;
                rc = NEXUS_Graphics2D_Blit(pContext->gfx, &blitSettings);
                BDBG_ASSERT(!rc);
            }
            else {
                fillSettings.surface = pContext->surface;
                fillSettings.color = 0xFF00AA33;
                fillSettings.rect = thumbrect;
                rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
                BDBG_ASSERT(!rc);
            }
        }

        if (file == pContext->currentFile) {
            fillSettings.surface = pContext->surface;
            fillSettings.color = 0xFFFFFF00;
            fillSettings.rect.x = rect.x;
            fillSettings.rect.y = rect.y;
            fillSettings.rect.width = rect.width;
            fillSettings.rect.height = FOCUS_WIDTH;
            rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
            BDBG_ASSERT(!rc);
            fillSettings.rect.x = rect.x;
            fillSettings.rect.y = rect.y;
            fillSettings.rect.width = FOCUS_WIDTH;
            fillSettings.rect.height = rect.height;
            rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
            BDBG_ASSERT(!rc);
            fillSettings.rect.x = rect.x;
            fillSettings.rect.y = rect.y + rect.height - FOCUS_WIDTH;
            fillSettings.rect.width = rect.width;
            fillSettings.rect.height = FOCUS_WIDTH;
            rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
            BDBG_ASSERT(!rc);
            fillSettings.rect.x = rect.x + rect.width - FOCUS_WIDTH;
            fillSettings.rect.y = rect.y;
            fillSettings.rect.width = FOCUS_WIDTH;
            fillSettings.rect.height = rect.height;
            rc = NEXUS_Graphics2D_Fill(pContext->gfx, &fillSettings);
            BDBG_ASSERT(!rc);

            /* printf("Stream: %s\n", pContext->currentFile->streamname); */
        }
        
        file->coord.x = x;
        file->coord.y = y;
        file->rect = rect;

        rect.y += rect.height + SPACING;
        y++;
        if (rect.y > 720 - rect.height - SPACING) {
            rect.y = top_y;
            rect.x += rect.width + SPACING;
            y = 0;
            x++;
        }
    }
    for (; file; file = BLST_Q_NEXT(file, link)) {
        file->rect.width = file->rect.height = 0;
        file->coord.x = -1;
        file->coord.y = -1;
    }

    rc = NEXUS_Graphics2D_Checkpoint(pContext->gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(pContext->checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    
    /* draw text */
    if (pContext->view != b_viewmode_thumb) {
        for (file = pContext->currentPage->firstFile; file; file = BLST_Q_NEXT(file, link)) {
            if (file->rect.width) {
                NEXUS_Rect textrect = file->rect;
                if (pContext->view == b_viewmode_full) {
                    textrect.x += FULL_THUMB_WIDTH + SPACING;
                    textrect.width -= FULL_THUMB_WIDTH + SPACING;
                }
                bfont_draw_aligned_text(&desc, pContext->font, &textrect, file->label, -1, 0xFFCCCCCC, bfont_valign_center, bfont_halign_left);
            }
        }
    }    
    NEXUS_Surface_Flush(pContext->surface);

    rc = NEXUS_SurfaceClient_SetSurface(pContext->blit_client, pContext->surface);
    BDBG_ASSERT(!rc);
    rc = BKNI_WaitForEvent(pContext->displayedEvent, 5000);
    BDBG_ASSERT(!rc);
}

/**
server masks user input, hides itself, launched client, waits for it to exit, then unmasks
**/
static void launch_client(struct appcontext *pContext)
{
    unsigned pid;
    char cmdline[256];
    int rc;

    if (!pContext->currentFile) return;

    if (*pContext->currentFile->indexname) {
        snprintf(cmdline, 256, "play \"%s\" \"%s\"", pContext->currentFile->streamname, pContext->currentFile->indexname);
    }
    else {
        snprintf(cmdline, 256, "play \"%s\"", pContext->currentFile->streamname);
    }

    binput_set_mask(pContext->input, 0); /* nothing */
    pContext->hidden = true;
    NEXUS_SurfaceClient_Clear(pContext->blit_client);

    pid = fork();
    if (!pid) {
        rc = system(cmdline);
        if (rc == -1) {
            printf("unable to launch %s: %d\n", cmdline, errno);
        }
        exit(0);
    }

    waitpid(pid, 0, 0);
    
    pContext->hidden = false;
    binput_set_mask(pContext->input, 0xFFFFFFFF); /* everything */
}

static int b_read_dir(const char *dirstr, int recursive, struct filelist *filelist)
{
    DIR *dir;
    struct dirent *d;

    dir = opendir((char *)dirstr);
    if (!dir) {
        BDBG_WRN(("Directory '%s' not found", dirstr));
        return 1;
    }
    while ((d = readdir(dir))) {
        char *dot;
        struct file *f;
        if(strcmp(d->d_name, ".")==0 || strcmp(d->d_name, "..")==0) {
            continue; /* skip "." and ".." */
        }
        if (d->d_type == DT_DIR && recursive ) {
            char *name;
            size_t len = strlen(dirstr)+strlen(d->d_name)+2;
            name = BKNI_Malloc(len);
            snprintf(name, len, "%s/%s", dirstr, d->d_name);
            b_read_dir(name, recursive, filelist);
            BKNI_Free(name);
            continue;
        }
        else if (d->d_type != DT_REG && d->d_type != DT_LNK) continue;
        dot = strrchr(d->d_name, '.');

        if (dot) {
            if (!strcmp(dot, ".info") || !strcmp(dot, ".nfo")) {
                continue;
            }
            if (!strcmp(dot, ".nav")) {
                continue;
            }
        }

        f = BKNI_Malloc(sizeof(*f));
        memset(f, 0, sizeof(*f));
        snprintf(f->streamname, sizeof(f->streamname)-1, "%s/%s", dirstr, d->d_name);
        strncpy(f->label, d->d_name, sizeof(f->label)-1);
        /* printf("found %s\n", f->streamname); */
        BLST_Q_INSERT_TAIL(filelist, f, link);
    }

    /* second pass for .nav */
    rewinddir(dir);
    while ((d = readdir(dir))) {
        char *dot;
        if (d->d_type != DT_REG && d->d_type != DT_LNK) continue;
        if(strcmp(d->d_name, ".")==0 || strcmp(d->d_name, "..")==0) {
            continue; /* skip "." and ".." */
        }
        dot = strrchr(d->d_name, '.');
        if (dot && !strcmp(dot, ".nav")) {
            struct file *file;
            for (file = BLST_Q_FIRST(filelist); file; file = BLST_Q_NEXT(file, link)) {
                if (!strncmp(file->label, d->d_name, dot - d->d_name)) {
                    snprintf(file->indexname, sizeof(file->indexname)-1, "%s/%s", dirstr, d->d_name);
                }
            }
        }
    }

    closedir(dir);
    return 0;
}

static void b_free_dir(struct filelist *filelist)
{
    struct file *file;
    while ((file = BLST_Q_FIRST(filelist))) {
        BLST_Q_REMOVE(filelist, file, link);
        BKNI_Free(file);
    }
}

static void *thumbnail_thread(void *context)
{
    struct appcontext *pContext = context;
    struct file *file;
    NEXUS_SurfaceCreateSettings createSettings;
    thumbdecoder_t thumbdec;

    pContext->thumbdec = thumbdec = thumbdecoder_open();
    if (!thumbdec) {
        BERR_TRACE(-1);
        return NULL;
    }

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    /* store big enough for 3K to downscale once */
    createSettings.width = THUMB_WIDTH * 4;
    createSettings.height = THUMB_HEIGHT * 4;
    for (file = BLST_Q_FIRST(&pContext->files); file && !pContext->done; file = BLST_Q_NEXT(file, link)) {
        int rc;
        NEXUS_SurfaceHandle surface = NEXUS_Surface_Create(&createSettings);
        rc = thumbdecoder_open_file(thumbdec, file->streamname, *file->indexname?file->indexname:NULL);
        if (!rc) {
            rc = thumbdecoder_decode_still(thumbdec, 5000, surface);
            if (rc) {
                BDBG_WRN(("unable to decode thumbnail for '%s'", file->streamname));
            }
            thumbdecoder_close_file(thumbdec);
        }
        else {
            BDBG_WRN(("unable to get thumbnail for '%s'", file->streamname));
        }
        if (rc && surface) {
            NEXUS_Surface_Destroy(surface);
            surface = NULL;
        }
        
        if (surface) {
            file->thumbnail = surface;
            pContext->ui_update = true;
            binput_interrupt(pContext->input);
        }
    }

    pContext->thumbdec = NULL;
    thumbdecoder_close(thumbdec);

    return NULL;
}

static int b_start_thumbnails(struct appcontext *pContext)
{
    return pthread_create(&pContext->thread, NULL, thumbnail_thread, pContext);
}

static void b_stop_thumbnails(struct appcontext *pContext)
{
    if (pContext->thread) {
        pContext->done = true;
        if (pContext->thumbdec) thumbdecoder_decode_cancel(pContext->thumbdec);
        pthread_join(pContext->thread, NULL);
    }
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
