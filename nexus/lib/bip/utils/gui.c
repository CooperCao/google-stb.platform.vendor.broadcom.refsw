/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "gui.h"
#include <sys/time.h>

BDBG_MODULE(bip_play_gui);

/* Progress bar private data */
typedef struct BIP_Play_ProgressBarPrivateData
{
    /* Nexus surfaces for playback icons */
    NEXUS_SurfaceHandle icons[BIP_Play_ProgressBarIcon_eLast];
} BIP_Play_ProgressBarPrivateData;

/* Status bar private data */
typedef struct BIP_Play_StatusBarPrivateData
{
    /* Nexus surface for background */
    NEXUS_SurfaceHandle bg;
} BIP_Play_StatusBarPrivateData;

static BIP_Status BIP_Play_ProgressBarInit(
        BIP_Play_GuiHandle,
        BIP_Play_GuiElementGroup *
    );
static void BIP_Play_ProgressBarUninit(
        BIP_Play_GuiElementGroup *
    );
static void BIP_Play_ProgressBarUpdate(
    BIP_Play_GuiElementGroup *,
    BIP_Play_GuiHandle
);
static BIP_Play_ProgressBarPrivateData* BIP_Play_ProgressBar_InitPrivData(
        picdecoder_t
    );
static void BIP_Play_ProgressBar_UninitPrivData(
        BIP_Play_ProgressBarPrivateData* priv
    );
static BIP_Status BIP_Play_StatusBarInit(
        BIP_Play_GuiHandle,
        BIP_Play_GuiElementGroup *
    );
static void BIP_Play_StatusBarUninit(
        BIP_Play_GuiElementGroup *
    );
static void BIP_Play_StatusBarUpdate(
    BIP_Play_GuiElementGroup *,
    BIP_Play_GuiHandle
);
static BIP_Play_StatusBarPrivateData* BIP_Play_StatusBar_InitPrivData(
        picdecoder_t
    );
static void BIP_Play_StatusBar_UninitPrivData(
        BIP_Play_StatusBarPrivateData* priv
    );
static void BIP_Play_RenderGuiGroups(
        BIP_Play_GuiHandle hGui
    );
static void BIP_Play_CompositeGuiGroups(
        BIP_Play_GuiHandle hGui,
        NEXUS_SurfaceHandle surface
    );
static void BIP_Play_GuiStopThread(
        BIP_Play_GuiHandle hGui
    );
static void BIP_Play_ZeroFillSurface(
        BIP_Play_GuiHandle hGui,
        NEXUS_SurfaceHandle surface
    );

#define GUI_ELEMENT_GROUP_INITIALIZER(A, B, C)  \
    {0, {{{0}, 0, 0, {0}, {0}}}, NULL, 0, 0, {0}, {0}, A, B, C, NULL}
/* Globals */
/* Initialize the ElementGroup function pointers (Init/Update/Uninit) for all the element groups in the GUI */
BIP_Play_GuiElementGroup g_gfxElementGroups[] =
{
    /* Element group 1: Progress Bar */
    GUI_ELEMENT_GROUP_INITIALIZER(BIP_Play_ProgressBarInit, BIP_Play_ProgressBarUninit, BIP_Play_ProgressBarUpdate),
    /* Element group 2: Stream Status */
    GUI_ELEMENT_GROUP_INITIALIZER(BIP_Play_StatusBarInit, BIP_Play_StatusBarUninit, BIP_Play_StatusBarUpdate)
};

/* Icon files for progress bar */
const char* g_iconFiles[BIP_Play_ProgressBarIcon_eMax] = {
    "images/play.png",
    "images/pause.png",
    "images/fwd2x.png",
    "images/fwd4x.png",
    "images/fwd8x.png",
    "images/fwd16x.png",
    "images/rew2x.png",
    "images/rew4x.png",
    "images/rew8x.png",
    "images/rew16x.png",
};

BIP_Status BIP_Play_GuiInit(
        BIP_Play_GuiHandle *hGuiContext
    )
{
    unsigned i;
    BIP_Play_GuiContext *hGui;
    B_ThreadSettings threadSettings;
    struct bgui_settings guiSettings;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;

    *hGuiContext = NULL;

    /* Allocate gui context */
    hGui = B_Os_Calloc(1, sizeof(BIP_Play_GuiContext));
    BIP_CHECK_GOTO((hGui), ("BIP Play GUI Context allocation failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
    memset(hGui, 0, sizeof(BIP_Play_GuiContext));
    *hGuiContext = hGui;

    /* Initialize bgui */
    bgui_get_default_settings(&guiSettings);
    guiSettings.width = BIP_PLAY_FRAME_BUFFER_WIDTH;
    guiSettings.height = BIP_PLAY_FRAME_BUFFER_HEIGHT;
    hGui->gui = bgui_create(&guiSettings);
    BIP_CHECK_GOTO((hGui->gui), ("bgui_create() failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "Created bgui" BIP_MSG_PRE_ARG));

    /* Initialize graphics frame buffer */
    bgui_fill(hGui->gui, 0x0);
    bgui_submit(hGui->gui);

    /* Open font file */
    hGui->font = bfont_open(BIP_PLAY_GUI_FONT_FILE);
    BIP_CHECK_GOTO((hGui->font), ("bfont_open() failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "Initialized fonts" BIP_MSG_PRE_ARG));

    /* Set Gui context params */
    B_Time_Get(&hGui->streamInfoUpdateTime);
    hGui->streamInfo.vid = NEXUS_VideoCodec_eUnknown;
    hGui->streamInfo.aud = NEXUS_AudioCodec_eUnknown;
    hGui->alpha = 0xFF; /* Full Opaque */

    /* Open picture decoder - needed for getting bitmaps from .png (icone) files */
    hGui->picDec = picdecoder_open();
    if (hGui->picDec == NULL)
        BDBG_WRN((BIP_MSG_PRE_FMT "Could not open picture decoder! GUI may be missing icons" BIP_MSG_PRE_ARG));
    BDBG_MSG((BIP_MSG_PRE_FMT "Picture decoder opened" BIP_MSG_PRE_ARG));

    /* Set Gui group info */
    hGui->numGroups = sizeof(g_gfxElementGroups)/sizeof(BIP_Play_GuiElementGroup);
    hGui->gfxGroups = &g_gfxElementGroups[0];

    /* Initialize all ElementGroups */
    for (i = 0; i < hGui->numGroups; i++)
    {
        bipStatus = hGui->gfxGroups[i].init(hGui, &hGui->gfxGroups[i]);
        BIP_CHECK_GOTO((bipStatus == BIP_SUCCESS), ("Gfx Group[%d] Init failed", i), error, bipStatus, bipStatus);
    }

    /* Create GUI Thread */
    hGui->shutdownThread = false;
    B_Thread_GetDefaultSettings(&threadSettings);
    hGui->guiThread = B_Thread_Create("GUI Thread", BIP_Play_GuiThread, hGui, &threadSettings);
    BIP_CHECK_GOTO((hGui->guiThread), ("Gui thread creation failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);
    BDBG_MSG((BIP_MSG_PRE_FMT "GUI Thread created" BIP_MSG_PRE_ARG));

    /* By default disable graphics */
    hGui->visible = false;
    hGui->showStatus = false;
    bipStatus = BIP_SUCCESS;

error:
    return bipStatus;
}

void BIP_Play_GuiUninit(
        BIP_Play_GuiHandle hGui
    )
{
    unsigned i;

    if (hGui)
    {
        if (hGui->guiThread)
        {
            /* First stop the thread */
            BIP_Play_GuiStopThread(hGui);
            B_Thread_Destroy(hGui->guiThread);
        }

        /* Uninitalize ElementGroups */
        for (i = 0; i < hGui->numGroups; i++)
            hGui->gfxGroups[i].uninit(&hGui->gfxGroups[i]);

        /* Release picture decoder */
        if (hGui->picDec)
            picdecoder_close(hGui->picDec);

        /* Close fonts */
        if (hGui->font)
            bfont_close(hGui->font);

        /* Uninitialize bgui */
        if (hGui->gui)
            bgui_destroy(hGui->gui);

        /* Release context memory */
        B_Os_Free(hGui);
    }
}

void BIP_Play_GuiGetStreamInfo(
        BIP_Play_GuiHandle hGui,
        BIP_Play_GuiStreamInfo *pInfo
    )
{
    *pInfo = hGui->streamInfo;
}

void BIP_Play_GuiSetStreamInfo(
        BIP_Play_GuiHandle hGui,
        BIP_Play_GuiStreamInfo *pInfo
    )
{
    hGui->streamInfo = *pInfo;
    /* Update time when last set Stream info call was made
     * Needed to extrapolate playback time, until next setPosition */
    B_Time_Get(&hGui->streamInfoUpdateTime);
}

void BIP_Play_GuiSetRate(
        BIP_Play_GuiHandle hGui,
        float rate
    )
{
    if (hGui->streamInfo.rate != rate)
    {
        hGui->streamInfo.rate = rate;
        /* Reset updateTime and setPosition on a rate change */
        hGui->streamInfo.setPosition = hGui->streamInfo.currentPos;
        B_Time_Get(&hGui->streamInfoUpdateTime);
    }
}

void BIP_Play_GuiSetPosition(
        BIP_Play_GuiHandle hGui,
        unsigned position
    )
{
    if (hGui->streamInfo.setPosition != position)
    {
        /* Reset update time */
        hGui->streamInfo.setPosition = position;
        B_Time_Get(&hGui->streamInfoUpdateTime);
    }
}

void BIP_Play_GuiEnable(
        BIP_Play_GuiHandle hGui
    )
{
    hGui->visible = true;
}

void BIP_Play_GuiDisable(
        BIP_Play_GuiHandle hGui
    )
{
    hGui->visible = false;
}

void BIP_Play_GuiSetAlpha(
        BIP_Play_GuiHandle hGui,
        unsigned alpha
    )
{
    hGui->alpha = (alpha > 0xFF) ? 0xFF : alpha;
}

void BIP_Play_GuiSetAudioDecoderStatus(
        BIP_Play_GuiHandle hGui,
        NEXUS_AudioDecoderStatus *pStatus
    )
{
    hGui->audioStatus = *pStatus;
}

/* Update video decoder status */
void BIP_Play_GuiSetVideoDecoderStatus(
        BIP_Play_GuiHandle hGui,
        NEXUS_VideoDecoderStatus *pStatus
    )
{
    hGui->videoStatus = *pStatus;
}

/* Apply a global alpha to the color */
static uint32_t BIP_Play_UpdateAlpha(uint32_t color, uint32_t alpha)
{
    unsigned newAlpha;

    newAlpha = alpha;
    if (newAlpha & 0xFFFFFF00)
        newAlpha = 0xFF;

    newAlpha = newAlpha*(BIP_PLAY_GUI_GET_ALPHA(color)) >> 8;

    return BIP_PLAY_GUI_SET_ALPHA (color, newAlpha);
}

/* Function to make a surface transparent black */
static void BIP_Play_ZeroFillSurface(
        BIP_Play_GuiHandle hGui,
        NEXUS_SurfaceHandle surface
    )
{
    NEXUS_Error rc;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_SurfaceCreateSettings surfaceSettings;

    NEXUS_Surface_GetCreateSettings(surface, &surfaceSettings);
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect.x = 0;
    fillSettings.rect.y = 0;
    fillSettings.rect.width = surfaceSettings.width;
    fillSettings.rect.height = surfaceSettings.height;
    fillSettings.color = 0x0;
    rc = NEXUS_Graphics2D_Fill(bgui_blitter(hGui->gui), &fillSettings);
    if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL)
        bgui_checkpoint(hGui->gui);
}

/* Render the elements of an ElementGroup into the ElementGroups surface */
static void BIP_Play_RenderGuiGroups(
        BIP_Play_GuiHandle hGui
    )
{
    unsigned i, j;
    NEXUS_Error rc;
    BIP_Play_GuiElement *element;
    struct bfont_surface_desc desc;
    BIP_Play_GuiElementGroup *group;
    bfont_draw_text_settings textSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;

    /* For each Gui element Group */
    for (i = 0; i < hGui->numGroups; i++)
    {
        group = &hGui->gfxGroups[i];

        /* Clear the surface first */
        BIP_Play_ZeroFillSurface(hGui, group->surface);

        /* For each Gui element in the group */
        for (j = 0; j < group->numElems; j++)
        {
            element = &group->elements[j];
            switch (element->type)
            {
                case BIP_Play_GuiElementType_eFill:
                    if (NEXUS_RECT_AREA(element->position) != 0) /* finite output rectangle */
                    {
                        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
                        fillSettings.surface = group->surface;
                        fillSettings.rect = element->position;
                        fillSettings.color = BIP_Play_UpdateAlpha(element->color, hGui->alpha);
                        rc = NEXUS_Graphics2D_Fill(bgui_blitter(hGui->gui), &fillSettings);
                        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL)
                            bgui_checkpoint(hGui->gui);
                    }
                    break;
                case BIP_Play_GuiElementType_eImage:
                    if (element->image.surface
                        && (NEXUS_RECT_AREA(element->position) != 0) /* finite output rectangle */
                        && (NEXUS_RECT_AREA(element->image.position) != 0)) /* finite source rectangle */
                    {
                        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

                        blitSettings.output.surface = group->surface;
                        blitSettings.output.rect = element->position;
                        blitSettings.source.surface = element->image.surface;
                        blitSettings.source.rect = element->image.position;
                        blitSettings.colorOp = element->image.colorOp;
                        blitSettings.alphaOp = element->image.alphaOp;

                        rc = NEXUS_Graphics2D_Blit(bgui_blitter(hGui->gui), &blitSettings);
                        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL)
                            bgui_checkpoint(hGui->gui);
                    }
                    break;
                case BIP_Play_GuiElementType_eText:
                    if (strlen(element->text.data))
                    {
                        /* Make sure no blits are pending before rendering fonts using cpu */
                        bgui_checkpoint(hGui->gui);

                        bfont_get_default_draw_text_settings(&textSettings);
                        textSettings.valign = element->text.valign;
                        textSettings.halign = element->text.halign;
                        bfont_get_surface_desc(group->surface, &desc);
                        bfont_draw_text_ex(&desc, hGui->font, &element->position,
                                           element->text.data, -1,
                                           BIP_Play_UpdateAlpha(element->color, hGui->alpha),
                                           &textSettings);
                        /* Flush nexus surface after cpu access of the surface memory */
                        NEXUS_Surface_Flush(group->surface);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

/* Composite all the element groups into the final frame buffer */
static void BIP_Play_CompositeGuiGroups(
        BIP_Play_GuiHandle hGui,
        NEXUS_SurfaceHandle surface
    )
{
    unsigned i;
    NEXUS_Error rc;
    BIP_Play_GuiElementGroup *group;
    NEXUS_Graphics2DBlitSettings blitSettings;

    if (surface == NULL)
        return;

    /* Clear the surface first */
    BIP_Play_ZeroFillSurface(hGui, surface);

    /* For each Gui element Group */
    for (i = 0; i < hGui->numGroups; i++)
    {
        group = &hGui->gfxGroups[i];
        /* Blit only if src and dest rect areas are non-zero */
        if ((NEXUS_RECT_AREA(group->dstPosition) != 0)
            && (NEXUS_RECT_AREA(group->srcPosition) != 0))
        {
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

            blitSettings.output.surface = surface;
            blitSettings.output.rect = group->dstPosition;

            blitSettings.source.surface = group->surface;
            blitSettings.source.rect = group->srcPosition;

            blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

            rc = NEXUS_Graphics2D_Blit(bgui_blitter(hGui->gui), &blitSettings);
            if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL)
                bgui_checkpoint(hGui->gui);
        }
    }

    /* At this point we are ready to submit the frame buffer */
    bgui_checkpoint(hGui->gui);
}

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

/* GUI thread */
void BIP_Play_GuiThread(void *pParam)
{
    unsigned i;
    BIP_Play_GuiHandle hGui = (BIP_Play_GuiHandle)pParam;

/* #define DUMP_SCREENSHOTS */
#ifdef DUMP_SCREENSHOTS
    unsigned count = 0;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.width = BIP_PLAY_FRAME_BUFFER_WIDTH;
    createSettings.height = BIP_PLAY_FRAME_BUFFER_HEIGHT;
    surface = NEXUS_Surface_Create(&createSettings);
#endif

    /* Until asked to shutdown by the application */
    while (!(hGui->shutdownThread))
    {
        if (hGui->visible)
        {
            /* Call update for each Gui element group */
            for (i = 0; i < hGui->numGroups; i++)
            {
                hGui->gfxGroups[i].update(&hGui->gfxGroups[i], hGui);
            }

            /* Render each gui group */
            BIP_Play_RenderGuiGroups(hGui);

            /* Composite gui group into Frame Buffer */
            BIP_Play_CompositeGuiGroups(hGui, bgui_surface(hGui->gui));
        }
        else
            bgui_fill(hGui->gui, 0x0);

        /* Submit the surface */
        bgui_submit(hGui->gui);

#ifdef DUMP_SCREENSHOTS /* For debug*/
#if NXCLIENT_SUPPORT
        if ((++count % 2) == 1)
        {
            char filename[128];
            NEXUS_SurfaceMemory mem;

            snprintf(filename, 128, "output/screenshot-%d.bmp", (count-1)/2);
            NxClient_Screenshot(NULL, surface);
            NEXUS_Surface_GetMemory(surface, &mem);
            NEXUS_Surface_Flush(surface);
            picdecoder_write_bmp(filename, surface, 24);
        }
#endif /* NXCLIENT_SUPPORT */
#endif /* DUMP_SCREENSHOTS */

        B_Thread_Sleep(BIP_PLAY_GUI_REFRESH_PERIOD);
    }

#ifdef DUMP_SCREENSHOTS
    NEXUS_Surface_Destroy(surface);
#endif
    hGui->shutdownThread = false;
}

/* Stop the GUI thread */
static void BIP_Play_GuiStopThread(
        BIP_Play_GuiHandle hGui
    )
{
    hGui->shutdownThread = true;

    /* Block until thread exits */
    while ((volatile bool)(hGui->shutdownThread) == true)
        B_Thread_Sleep(10);
}

void BIP_Play_GuiShowStatus(
        BIP_Play_GuiHandle hGui
    )
{
    hGui->showStatus = true;
}

void BIP_Play_GuiHideStatus(
        BIP_Play_GuiHandle hGui
    )
{
    hGui->showStatus = false;
}

void BIP_Play_GuiSetPlayerStatus(
        BIP_Play_GuiHandle hGui,
        BIP_PlayerStatus *pStatus
    )
{
    hGui->streamInfo.playerStatus = *pStatus;
}

/************************************************/
/************ Progress Bar functions ************/
/************************************************/
/* Initialization function for the ProgressBar element group */
static BIP_Status BIP_Play_ProgressBarInit(
        BIP_Play_GuiHandle hGui,
        BIP_Play_GuiElementGroup *group
    )
{
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_SurfaceCreateSettings createSettings;

    /* Initialize Progress bar group */
    group->surfaceWidth = BIP_PLAY_PROGRESS_BAR_WIDTH;
    group->surfaceHeight = BIP_PLAY_PROGRESS_BAR_HEIGHT;

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = group->surfaceWidth;
    createSettings.height = group->surfaceHeight;

    /* Create surface for element group */
    group->surface = NEXUS_Surface_Create(&createSettings);
    BIP_CHECK_GOTO((group->surface), ("ProgressBar: Surface Creation failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    /* Setup what part of the element group needs to be seen on the frame buffer */
    group->srcPosition.x = 0;
    group->srcPosition.y = 0;
    group->srcPosition.width = BIP_PLAY_PROGRESS_BAR_WIDTH;
    group->srcPosition.height = BIP_PLAY_PROGRESS_BAR_HEIGHT;

    /* Setup the location of the element group in the frame buffer */
    group->dstPosition.x = BIP_PLAY_PROGRESS_BAR_POS_X;
    group->dstPosition.y = BIP_PLAY_PROGRESS_BAR_POS_Y;
    group->dstPosition.width = BIP_PLAY_PROGRESS_BAR_WIDTH;
    group->dstPosition.height = BIP_PLAY_PROGRESS_BAR_HEIGHT;

    /* Initialize private data - this structure has the pointers to icons used in progress bar */
    group->priv = (void *)BIP_Play_ProgressBar_InitPrivData(hGui->picDec);
    if (group->priv == NULL)
        BDBG_WRN((BIP_MSG_PRE_FMT "ProgressBar: Private data init failure, icons won't be seen!" BIP_MSG_PRE_ARG));

    group->numElems = BIP_Play_ProgressBarElement_eLast;

    /* Initialize each graphics element in the element group */
    /* Background */
    group->elements[BIP_Play_ProgressBarElement_eBG].type = BIP_Play_GuiElementType_eFill;
    group->elements[BIP_Play_ProgressBarElement_eBG].position = group->srcPosition;
    group->elements[BIP_Play_ProgressBarElement_eBG].color = BIP_PLAY_PROGRESS_BAR_BG_COLOR;

    /* Playback time */
    group->elements[BIP_Play_ProgressBarElement_eTime].type = BIP_Play_GuiElementType_eText;
    group->elements[BIP_Play_ProgressBarElement_eTime].position.x = BIP_PLAY_PROGRESS_BAR_TIME_POS_X;
    group->elements[BIP_Play_ProgressBarElement_eTime].position.y = BIP_PLAY_PROGRESS_BAR_TIME_POS_Y;
    group->elements[BIP_Play_ProgressBarElement_eTime].position.width = BIP_PLAY_PROGRESS_BAR_TIME_WIDTH;
    group->elements[BIP_Play_ProgressBarElement_eTime].position.height = BIP_PLAY_PROGRESS_BAR_TIME_HEIGHT;
    group->elements[BIP_Play_ProgressBarElement_eTime].text.valign = bfont_valign_center;
    group->elements[BIP_Play_ProgressBarElement_eTime].text.halign = bfont_halign_left;
    group->elements[BIP_Play_ProgressBarElement_eTime].color = BIP_PLAY_COLOR_WHITE;

    /* Stream URL */
    group->elements[BIP_Play_ProgressBarElement_eTitle].type = BIP_Play_GuiElementType_eText;
    group->elements[BIP_Play_ProgressBarElement_eTitle].position.x = BIP_PLAY_PROGRESS_BAR_TITLE_POS_X;
    group->elements[BIP_Play_ProgressBarElement_eTitle].position.y = BIP_PLAY_PROGRESS_BAR_TITLE_POS_Y;
    group->elements[BIP_Play_ProgressBarElement_eTitle].position.width = BIP_PLAY_PROGRESS_BAR_TITLE_WIDTH;
    group->elements[BIP_Play_ProgressBarElement_eTitle].position.height = BIP_PLAY_PROGRESS_BAR_TITLE_HEIGHT;
    group->elements[BIP_Play_ProgressBarElement_eTitle].text.valign = bfont_valign_center;
    /* TBD: halign = bfont_halign_right doesn't seem to work as expected */
    group->elements[BIP_Play_ProgressBarElement_eTitle].text.halign = bfont_halign_center;
    group->elements[BIP_Play_ProgressBarElement_eTitle].color = BIP_PLAY_COLOR_WHITE;

    /* Icon */
    group->elements[BIP_Play_ProgressBarElement_eIcon].type = BIP_Play_GuiElementType_eImage;
    group->elements[BIP_Play_ProgressBarElement_eIcon].position.x = BIP_PLAY_PROGRESS_BAR_ICON_POS_X;
    group->elements[BIP_Play_ProgressBarElement_eIcon].position.y = BIP_PLAY_PROGRESS_BAR_ICON_POS_Y;
    group->elements[BIP_Play_ProgressBarElement_eIcon].position.width = BIP_PLAY_PROGRESS_BAR_ICON_WIDTH;
    group->elements[BIP_Play_ProgressBarElement_eIcon].position.height = BIP_PLAY_PROGRESS_BAR_ICON_HEIGHT;
    group->elements[BIP_Play_ProgressBarElement_eIcon].image.colorOp = NEXUS_BlitColorOp_eCopySource;
    group->elements[BIP_Play_ProgressBarElement_eIcon].image.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    group->elements[BIP_Play_ProgressBarElement_eIcon].image.position.x = 0;
    group->elements[BIP_Play_ProgressBarElement_eIcon].image.position.y = 0;
    if (group->priv && ((BIP_Play_ProgressBarPrivateData*)(group->priv))->icons[BIP_Play_ProgressBarIcon_ePause])
    {
        group->elements[BIP_Play_ProgressBarElement_eIcon].image.surface =
            ((BIP_Play_ProgressBarPrivateData*)(group->priv))->icons[BIP_Play_ProgressBarIcon_ePause];
        NEXUS_Surface_GetCreateSettings(
            ((BIP_Play_ProgressBarPrivateData*)(group->priv))->icons[BIP_Play_ProgressBarIcon_ePause],
            &createSettings);
        group->elements[BIP_Play_ProgressBarElement_eIcon].image.position.width = createSettings.width;
        group->elements[BIP_Play_ProgressBarElement_eIcon].image.position.height = createSettings.height;
    }

    /* Cursor Background */
    group->elements[BIP_Play_ProgressBarElement_eCursorBG].type = BIP_Play_GuiElementType_eFill;
    group->elements[BIP_Play_ProgressBarElement_eCursorBG].position.x = BIP_PLAY_PROGRESS_BAR_CURSORBG_POS_X;
    group->elements[BIP_Play_ProgressBarElement_eCursorBG].position.y = BIP_PLAY_PROGRESS_BAR_CURSORBG_POS_Y;
    group->elements[BIP_Play_ProgressBarElement_eCursorBG].position.width = BIP_PLAY_PROGRESS_BAR_CURSORBG_WIDTH;
    group->elements[BIP_Play_ProgressBarElement_eCursorBG].position.height = BIP_PLAY_PROGRESS_BAR_CURSORBG_HEIGHT;
    group->elements[BIP_Play_ProgressBarElement_eCursorBG].color = BIP_PLAY_PROGRESS_BAR_CURSORBG_COLOR;

    /* Cursor Background */
    group->elements[BIP_Play_ProgressBarElement_eCursor].type = BIP_Play_GuiElementType_eFill;
    group->elements[BIP_Play_ProgressBarElement_eCursor].position.x = BIP_PLAY_PROGRESS_BAR_CURSOR_POS_X;
    group->elements[BIP_Play_ProgressBarElement_eCursor].position.y = BIP_PLAY_PROGRESS_BAR_CURSOR_POS_Y;
    group->elements[BIP_Play_ProgressBarElement_eCursor].position.width = 0;
    group->elements[BIP_Play_ProgressBarElement_eCursor].position.height = BIP_PLAY_PROGRESS_BAR_CURSORBG_HEIGHT;
    group->elements[BIP_Play_ProgressBarElement_eCursor].color = BIP_PLAY_PROGRESS_BAR_CURSOR_COLOR;

    /* Time Left */
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].type = BIP_Play_GuiElementType_eText;
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].position.x = BIP_PLAY_PROGRESS_BAR_TIME_LEFT_POS_X;
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].position.y = BIP_PLAY_PROGRESS_BAR_TIME_LEFT_POS_Y;
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].position.width = BIP_PLAY_PROGRESS_BAR_TIME_LEFT_WIDTH;
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].position.height = BIP_PLAY_PROGRESS_BAR_TIME_LEFT_HEIGHT;
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].text.valign = bfont_valign_center;
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].text.halign = bfont_halign_left;
    group->elements[BIP_Play_ProgressBarElement_eTimeLeft].color = BIP_PLAY_COLOR_WHITE;

    BDBG_MSG((BIP_MSG_PRE_FMT "Initialized ProgressBar Gui Element Group" BIP_MSG_PRE_ARG));
    bipStatus = BIP_SUCCESS;

error:
    return bipStatus;
}

/* Uninitialization function for the ProgressBar element group */
static void BIP_Play_ProgressBarUninit(
        BIP_Play_GuiElementGroup *group
    )
{
    if (group)
    {
        if (group->priv)
            BIP_Play_ProgressBar_UninitPrivData((BIP_Play_ProgressBarPrivateData*)group->priv);

        if (group->surface)
            NEXUS_Surface_Destroy(group->surface);
    }
}

/* Private data initialization - Open png files for icons and create nexus surfaces */
static BIP_Play_ProgressBarPrivateData* BIP_Play_ProgressBar_InitPrivData(
        picdecoder_t picDec
    )
{
    unsigned i;
    BIP_Play_ProgressBarPrivateData *priv;

    priv = B_Os_Calloc(1, sizeof(BIP_Play_ProgressBarPrivateData));

    if (priv)
    {
        memset (priv, 0, sizeof(BIP_Play_ProgressBarPrivateData));
        /* Open the icon png files and populate nexus surfaces */
        for (i = 0; i < BIP_Play_ProgressBarIcon_eLast; i++)
        {
            /* Create nexus surfaces and populate it with icon bitmaps */
            priv->icons[i] = picdecoder_decode(picDec, g_iconFiles[i]);
        }
    }

    return priv;
}

static void BIP_Play_ProgressBar_UninitPrivData(BIP_Play_ProgressBarPrivateData* priv)
{
    unsigned i;

    if (priv)
    {
        for (i = 0; i < BIP_Play_ProgressBarIcon_eLast; i++)
            if (priv->icons[i])
                NEXUS_Surface_Destroy(priv->icons[i]);

        B_Os_Free(priv);
    }
}

/* Update function for the ProgressBar element group */
static void BIP_Play_ProgressBarUpdate(
        BIP_Play_GuiElementGroup *group,
        BIP_Play_GuiHandle hGui
    )
{
    struct timeval now;
    unsigned timeElapsed, timeLeft;
    BIP_Play_ProgressBarPrivateData* priv;
    priv = (BIP_Play_ProgressBarPrivateData *)group->priv;

    /* Update Title */
    strncpy(group->elements[BIP_Play_ProgressBarElement_eTitle].text.data, hGui->streamInfo.url, MAX_TEXT_DATA_LEN);

    /* Get and update interpolated playback time and time left */
    B_Time_Get(&now);
    timeElapsed = B_Time_DiffMicroseconds(&now, &hGui->streamInfoUpdateTime)/1000;
    /* Compute current position */
    hGui->streamInfo.currentPos = hGui->streamInfo.setPosition + timeElapsed*hGui->streamInfo.rate;
    /* Bounds check for current position */
    if (hGui->streamInfo.currentPos > hGui->streamInfo.duration)
        hGui->streamInfo.currentPos = hGui->streamInfo.duration;
    /* Update gui data */
    snprintf(group->elements[BIP_Play_ProgressBarElement_eTime].text.data, MAX_TEXT_DATA_LEN,
             TIME_FORMAT_SPECIFIER_HH_MM_SS, TIME_FORMAT_VALUES_HH_MM_SS(hGui->streamInfo.currentPos));
    /* Compute time left and update gui data */
    timeLeft = hGui->streamInfo.duration - hGui->streamInfo.currentPos;
    snprintf(group->elements[BIP_Play_ProgressBarElement_eTimeLeft].text.data, MAX_TEXT_DATA_LEN,
             TIME_FORMAT_SPECIFIER_HH_MM_SS, TIME_FORMAT_VALUES_HH_MM_SS(timeLeft));

    /* Update cursor position - basically just width */
    group->elements[BIP_Play_ProgressBarElement_eCursor].position.width = ((int)hGui->streamInfo.duration > 0) ?
        (group->elements[BIP_Play_ProgressBarElement_eCursorBG].position.width*hGui->streamInfo.currentPos)/hGui->streamInfo.duration
        : 0;

    /* Update playback icon*/
    if (priv)
    {
        BIP_Play_GuiElement *iconElem = &group->elements[BIP_Play_ProgressBarElement_eIcon];
        switch ((int)hGui->streamInfo.rate)
        {
            case 0:   iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_ePlay];   break;
            case 1:   iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_ePause];  break;
            case 2:   iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eFwd2x];  break;
            case 4:   iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eFwd4x];  break;
            case 8:   iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eFwd8x];  break;
            case 16:  iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eFwd16x]; break;
            case -2:  iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eRew2x];  break;
            case -4:  iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eRew4x];  break;
            case -8:  iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eRew8x];  break;
            case -16: iconElem->image.surface = priv->icons[BIP_Play_ProgressBarIcon_eRew16x]; break;
            default: break;
        }
    }
}

/************************************************/
/************ Progress Bar functions ************/
/************************************************/
/* Initialization function for the StatusBar element group */
static BIP_Status BIP_Play_StatusBarInit(
        BIP_Play_GuiHandle hGui,
        BIP_Play_GuiElementGroup *group
    )
{
    unsigned j, fontHeight, first;
    BIP_Status bipStatus = BIP_ERR_INTERNAL;
    NEXUS_SurfaceCreateSettings createSettings;

    /* Get font height */
    bfont_get_height(hGui->font, &fontHeight);

    group->surfaceWidth = BIP_PLAY_STATUS_BAR_WIDTH;
    /* Height of the surface calculated based on the number of lines in status bar */
    group->surfaceHeight = 2*BIP_PLAY_STATUS_BAR_TOP_MARGIN +
                           (fontHeight*3/2)*(BIP_Play_StatusBarElement_eLast - 1);

    /* Initialize Status bar group */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = group->surfaceWidth;
    createSettings.height = group->surfaceHeight;

    /* Create surface for element group */
    group->surface = NEXUS_Surface_Create(&createSettings);
    BIP_CHECK_GOTO((group->surface), ("StatusBar: Surface Creation failed"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, bipStatus);

    /* Setup what part of the element group needs to be seen on the frame buffer
     * Initially it is nothing */
    group->srcPosition.x = 0;
    group->srcPosition.y = group->surfaceHeight;
    group->srcPosition.width = group->surfaceWidth;
    group->srcPosition.height = 0;

    /* Setup the location of the element group in the frame buffer */
    group->dstPosition.x = BIP_PLAY_STATUS_BAR_POS_X;
    group->dstPosition.y = BIP_PLAY_STATUS_BAR_POS_Y;
    group->dstPosition.width = group->srcPosition.width;
    group->dstPosition.height = group->srcPosition.height;

    /* Initialize private data - this structure has the pointers to icons used in status bar */
    group->priv = (void *)BIP_Play_StatusBar_InitPrivData(hGui->picDec);
    if (group->priv == NULL)
        BDBG_WRN((BIP_MSG_PRE_FMT "StatusBar: Private data init failure, background will be transparent!" BIP_MSG_PRE_ARG));

    group->numElems = BIP_Play_StatusBarElement_eLast;

    /* Initialize each graphics element in the element group */
    /* Background */
    group->elements[BIP_Play_StatusBarElement_eBG].type = BIP_Play_GuiElementType_eImage;
    group->elements[BIP_Play_StatusBarElement_eBG].position.x = 0;
    group->elements[BIP_Play_StatusBarElement_eBG].position.y = 0;
    group->elements[BIP_Play_StatusBarElement_eBG].position.width = group->surfaceWidth;
    group->elements[BIP_Play_StatusBarElement_eBG].position.height = group->surfaceHeight;
    group->elements[BIP_Play_StatusBarElement_eBG].image.colorOp = NEXUS_BlitColorOp_eCopySource;
    group->elements[BIP_Play_StatusBarElement_eBG].image.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    group->elements[BIP_Play_StatusBarElement_eBG].image.position.x = 0;
    group->elements[BIP_Play_StatusBarElement_eBG].image.position.y = 0;
    if (group->priv && ((BIP_Play_StatusBarPrivateData*)(group->priv))->bg)
    {
        group->elements[BIP_Play_StatusBarElement_eBG].image.surface = ((BIP_Play_StatusBarPrivateData*)(group->priv))->bg;
        NEXUS_Surface_GetCreateSettings(((BIP_Play_StatusBarPrivateData*)(group->priv))->bg, &createSettings);
        group->elements[BIP_Play_StatusBarElement_eBG].image.position.width = createSettings.width;
        group->elements[BIP_Play_StatusBarElement_eBG].image.position.height = createSettings.height;
    }

    /* All elements are of type text in status bar */
    first = BIP_Play_StatusBarElement_eVideoCodec; /* First text element */
    for (j = first; j < BIP_Play_StatusBarElement_eLast; j++)
    {
        group->elements[j].type = BIP_Play_GuiElementType_eText;
        group->elements[j].position.x = BIP_PLAY_STATUS_BAR_TEXT_POS_X;
        group->elements[j].position.y = BIP_PLAY_STATUS_BAR_TOP_MARGIN + (j-first)*fontHeight*3/2;
        group->elements[j].position.width = BIP_PLAY_STATUS_BAR_WIDTH;
        group->elements[j].position.height = fontHeight*3/2; /* Top and bottom margin = 25% of text height */
        group->elements[j].text.valign = bfont_valign_center;
        group->elements[j].text.halign = bfont_halign_left;
        group->elements[j].color = BIP_PLAY_COLOR_WHITE;
    }

    BDBG_MSG((BIP_MSG_PRE_FMT "Initialized StatusBar Gui Element Group" BIP_MSG_PRE_ARG));
    bipStatus = BIP_SUCCESS;

error:
    return bipStatus;
}

/* Uninitialization function for the StatusBar element group */
static void BIP_Play_StatusBarUninit(
        BIP_Play_GuiElementGroup *group
    )
{
    if (group)
    {
        if (group->priv)
            BIP_Play_StatusBar_UninitPrivData((BIP_Play_StatusBarPrivateData*)group->priv);

        if (group->surface)
            NEXUS_Surface_Destroy(group->surface);
    }
}

/* Private data initialization - Open png files for background and create nexus surface */
static BIP_Play_StatusBarPrivateData* BIP_Play_StatusBar_InitPrivData(
        picdecoder_t picDec
    )
{
    BIP_Play_StatusBarPrivateData *priv;

    priv = B_Os_Calloc(1, sizeof(BIP_Play_StatusBarPrivateData));

    if (priv)
    {
        memset (priv, 0, sizeof(BIP_Play_StatusBarPrivateData));
        /* Open the background png file and populate nexus surface */
        priv->bg = picdecoder_decode(picDec, BIP_PLAY_STATUS_BAR_BACKGROUND);
    }

    return priv;
}

static void BIP_Play_StatusBar_UninitPrivData(BIP_Play_StatusBarPrivateData* priv)
{
    if (priv)
    {
        if (priv->bg)
            NEXUS_Surface_Destroy(priv->bg);

        B_Os_Free(priv);
    }
}

/* Update function for the StatusBar element group */
static void BIP_Play_StatusBarUpdate(
        BIP_Play_GuiElementGroup *group,
        BIP_Play_GuiHandle hGui
    )
{
    int y;
    unsigned duration, yInc;
    BIP_Play_StatusBarPrivateData* priv;

    priv = (BIP_Play_StatusBarPrivateData *)group->priv;

    duration = hGui->streamInfo.playerStatus.lastPositionInMs;
    /* Update Status bar data */
    snprintf(group->elements[BIP_Play_StatusBarElement_eVideoCodec].text.data, MAX_TEXT_DATA_LEN,
             "Video Codec (res) : %s [%dx%d]", BIP_ToStr_NEXUS_VideoCodec(hGui->streamInfo.vid),
             hGui->streamInfo.vidWidth, hGui->streamInfo.vidHeight);
    snprintf(group->elements[BIP_Play_StatusBarElement_eAudioCodec].text.data, MAX_TEXT_DATA_LEN,
             "Audio Codec       : %s", BIP_ToStr_NEXUS_AudioCodec(hGui->streamInfo.vid));
    snprintf(group->elements[BIP_Play_StatusBarElement_eAvErrors].text.data, MAX_TEXT_DATA_LEN,
             "A/V Errors        : %d / %d", hGui->audioStatus.frameErrors, hGui->videoStatus.numDecodeErrors);
    snprintf(group->elements[BIP_Play_StatusBarElement_eBytesCons].text.data, MAX_TEXT_DATA_LEN,
             "Bytes consumed    : %lu", (long)hGui->streamInfo.playerStatus.stats.totalConsumed);
    snprintf(group->elements[BIP_Play_StatusBarElement_ePlayerState].text.data, MAX_TEXT_DATA_LEN,
             "Player State      : %s", BIP_ToStr_BIP_PlayerState(hGui->streamInfo.playerStatus.state));
    snprintf(group->elements[BIP_Play_StatusBarElement_eStreamDuration].text.data, MAX_TEXT_DATA_LEN,
             "Stream Duration   : " TIME_FORMAT_SPECIFIER_HH_MM_SS, TIME_FORMAT_VALUES_HH_MM_SS(duration));

    group->numElems = BIP_Play_StatusBarElement_eLast;

    /* Animate */
    y = group->srcPosition.y;
    /* one second animation time */
    yInc = group->surfaceHeight*BIP_PLAY_GUI_REFRESH_PERIOD/200;
    if (hGui->showStatus)
    {
        /* Drop down */
        if (y != 0)
            y -= yInc;
        if (y < 0)
            y = 0;
    }
    else
    {
        /* Wind up */
        if (y != (int)group->surfaceHeight)
            y += yInc;
        if (y > (int)group->surfaceHeight)
        {
            y = (int)group->surfaceHeight;
        }

        if (y == (int)group->surfaceHeight)
            group->numElems = 0;
    }
    /* set source y, height and dest height */
    group->srcPosition.y = y;
    group->srcPosition.height = group->surfaceHeight - y;
    group->dstPosition.height = group->surfaceHeight - y;
}
