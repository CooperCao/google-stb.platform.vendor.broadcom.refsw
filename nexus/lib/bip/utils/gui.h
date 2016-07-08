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

#ifndef _BIP_PLAY_GUI_H_
#define _BIP_PLAY_GUI_H_

#include <stdint.h>
#include <stdbool.h>

#include "bip.h"
#include "bfont.h"
#include "bgui.h"
#include "picdecoder.h"

/* Terminology:
 * GuiElement      : The basic UI element (images, text, solid rectangles) that can be
 *                   rendered to a nexus surface using Graphics 2D Fill or a Blit call
 *                   or through cpu (font rendering)
 * GuiElementGroup : Collection of GuiElements rendered in a particular order
 *                   that make up a part if the GUI (ex: ProgressBar)
 * GUI             : Set of GuiElementGroups composited in a particular order
 *
 * Adding a new elememt group to the GUI:
 * The GUI is defined as a set of Element Groups with each ElementGroup being a collection of
 * basic UI elements (images, text, solid rectangles). For example the playback progress
 * bar (ProgressBar) is an element group and is made up of these elements Background, Cursor,
 * playback icon and the playback time elapsed. For each element group that needs to
 * be added to the GUI only the following functions need to be defined. All the rendering of
 * element groups and compositing the element groups to the frame buffer is taken care
 * of by the GUI thread.
 * Element group function handlers:
 * - Init:   This function will allocate the surface for the group and initializes all the elements
 *           in the element group. For instance the ProgressBar_Init would allocate the nexus
 *           surfaces needed for the playback icons and initialize them with the icon images.
 *           This function will also define location where the element group needs to be rendered
 *           in the final frame buffer. The element group's surface can be composited to the frame
 *           buffer as a whole or only in part (see srcPosition in BIP_Play_GuiElementGroup).
 * - Uninit: This function releases all the resources (memory, nexus surfaces, etc  ...) that
 *           was allocated in the Init function.
 * - Update: This function is called every 'BIP_PLAY_GUI_REFRESH_PERIOD' milli seconds and is expected
 *           to update the elements in the element group to reflect the current status of playback.
 *           For example, the playback time elapsed and the playback cursor position etc ...
 */

#define MAX_TEXT_DATA_LEN               128
#define MAX_GFX_ELEMENTS_PER_GROUP      10

/* Font file */
#define BIP_PLAY_GUI_FONT_FILE          "fonts/cinecavB19i_serif.bwin_font"

/* Frame buffer settings */
#define BIP_PLAY_FRAME_BUFFER_WIDTH     1280
#define BIP_PLAY_FRAME_BUFFER_HEIGHT    720

/* GUI refresh rate */
#define BIP_PLAY_GUI_REFRESH_PERIOD     30 /* In milli seconds */

/* Get/Set alpha macros */
#define BIP_PLAY_GUI_GET_ALPHA(color)        (((unsigned)color >> 24) & 0x000000FF)
#define BIP_PLAY_GUI_SET_ALPHA(color, alpha) (((unsigned)alpha << 24) | (color & 0x00FFFFFF))

#define NEXUS_RECT_AREA(A)  ((A).width*(A).height)

/* Format specifier for time from milli seconds */
#define TIME_FORMAT_SPECIFIER_HH_MM_SS      "%02u:%02u:%02u"
#define TIME_FORMAT_VALUES_HH_MM_SS(ms)     (ms/3600000), ((ms % 3600000)/60000), ((ms % 60000)/1000)

/* Graphics element type */
typedef enum BIP_Play_GuiElementType
{
    BIP_Play_GuiElementType_eFill,  /* Simple rectangle fill */
    BIP_Play_GuiElementType_eImage, /* Blit from a give nexus surface */
    BIP_Play_GuiElementType_eText,  /* Font rendering */
    BIP_Play_GuiElementType_eMax
} BIP_Play_GuiElementType;

/* Gui element : The basic element that can be rendered to a nexus surface
 * using Graphics 2D Fill or a Blit call or through cpu (font rendering)*/
typedef struct BIP_Play_GuiElement
{
    NEXUS_Rect position;    /* Position of the element */
    uint32_t   color;       /* alpha + color -> ARGB8888 format */
    BIP_Play_GuiElementType type;   /* Element type : image, text, fill ... */
    struct {
        bfont_valign valign;            /* Vertical Alignment */
        bfont_halign halign;            /* Horizontal Alignment */
        char data[MAX_TEXT_DATA_LEN];   /* Character sequence */
    } text;
    struct {
        NEXUS_SurfaceHandle surface; /* Source surface */
        NEXUS_Rect          position;/* Part of source surface that will be blitted */
        NEXUS_BlitColorOp   colorOp; /* Color Operation */
        NEXUS_BlitAlphaOp   alphaOp; /* Alpha operation */
    } image;
} BIP_Play_GuiElement;

struct BIP_Play_GuiContext;
struct BIP_Play_GuiElementGroup;
typedef struct BIP_Play_GuiContext* BIP_Play_GuiHandle;

/* Element group function handlers */
typedef BIP_Status (*BIP_Play_GuiGroupInit)(BIP_Play_GuiHandle, struct BIP_Play_GuiElementGroup*);
typedef void (*BIP_Play_GuiGroupUninit)(struct BIP_Play_GuiElementGroup*);
typedef void (*BIP_Play_GuiGroupUpdate)(struct BIP_Play_GuiElementGroup*, BIP_Play_GuiHandle);

/* Graphics element group - Ex: Progress bar */
typedef struct BIP_Play_GuiElementGroup
{
    unsigned int        numElems;   /* Number of elements in the group */
    BIP_Play_GuiElement elements[MAX_GFX_ELEMENTS_PER_GROUP];
    NEXUS_SurfaceHandle surface;    /* Surface for the element group */
    unsigned int     surfaceWidth;  /* Surface width */
    unsigned int     surfaceHeight; /* Surface height */
    NEXUS_Rect       srcPosition;   /* Part of this surface that will be composited to Frame Buffer */
    NEXUS_Rect       dstPosition;   /* Group's position in the final Frame Buffer */
    BIP_Play_GuiGroupInit   init;   /* Function handler to initialize element group */
    BIP_Play_GuiGroupUninit uninit; /* Function handler to uninitialize element group  */
    BIP_Play_GuiGroupUpdate update; /* Function handler to update element group */
    void* priv;  /* Private data */
} BIP_Play_GuiElementGroup;

/* All the information needed to update the element groups in the GUI */
typedef struct BIP_Play_GuiStreamInfo
{
    /* Steam playback info */
    unsigned currentPos; /* Current position in ms */
    unsigned duration;   /* Stream duration in ms */
    float    rate;       /* Stream playback rate */
    unsigned setPosition;          /* Stream position set using BIP_Play_GuiSetPosition() */
    NEXUS_VideoCodec vid;          /* Video codec */
    unsigned vidWidth;             /* Video Width */
    unsigned vidHeight;            /* Video Height */
    NEXUS_AudioCodec aud;          /* Audio codec */
    BIP_PlayerStatus playerStatus; /* Player status - has some redundant information
                                    * (including audio/video) codec info, but this data
                                    * is not available until player has been started */
    char url[MAX_TEXT_DATA_LEN]; /* URL string */
} BIP_Play_GuiStreamInfo;

typedef struct BIP_Play_GuiContext
{
    bool     visible;   /* Flag to disable/enable graphics */
    bool     showStatus;/* Flag to disable/enable player and/or server status */
    bgui_t   gui;       /* bgui handle */
    bfont_t  font;      /* Using a single font for all text for now */
    unsigned numGroups; /* Number of Gfx groups */
    BIP_Play_GuiElementGroup *gfxGroups; /* Points to the global g_gfxElementGroups */

    /* Stream info that goes on the UI */
    BIP_Play_GuiStreamInfo streamInfo;
    NEXUS_VideoDecoderStatus videoStatus;
    NEXUS_AudioDecoderStatus audioStatus;
    struct timeval streamInfoUpdateTime;   /* System time when the position was set -
                                  * Used to extrapolate current position */
    unsigned alpha;     /* Overall alpha as specified at command line */

    /* Graphics thread */
    B_ThreadHandle guiThread;    /* GUI Thread handle */
    bool shutdownThread;

    /* Picture decoder */
    picdecoder_t picDec;
} BIP_Play_GuiContext;

/* - Allocates and Initialize the Gui context
 * - Open bfont, bgui handles
 * - Allocates picture decoder
 * - Create and starts GUI thread
 * - Calls init function on all GuiElementGroups (g_gfxElementGroups) */
BIP_Status BIP_Play_GuiInit(
        BIP_Play_GuiHandle *hGuiContext
    );
/* - Uninitializes gui context
 * - Closes bfont and bgui
 * - Release picture decoder
 * - Stop GUI thread
 * - Calls uninit function on all GuiElementGroups */
void BIP_Play_GuiUninit(
        BIP_Play_GuiHandle hGui
    );
/* Gui Thread composites the frame buffer from the indivudual element
 * groups and updates the frame buffer */
void BIP_Play_GuiThread(
        void *pParam
    );
/* Get stream information as reflected by the GUI */
void BIP_Play_GuiGetStreamInfo(
        BIP_Play_GuiHandle hGui,
        BIP_Play_GuiStreamInfo *pInfo
    );
/* Update stream info in the GUI */
void BIP_Play_GuiSetStreamInfo(
        BIP_Play_GuiHandle hGui,
        BIP_Play_GuiStreamInfo *pInfo
    );

/* Apis to turn on/off graphics */
void BIP_Play_GuiEnable(
        BIP_Play_GuiHandle hGui
    );
void BIP_Play_GuiDisable(
        BIP_Play_GuiHandle hGui
    );

/* Set a global alpha for the entire GUI (Will be applied to all element groups) */
void BIP_Play_GuiSetAlpha(
        BIP_Play_GuiHandle hGui,
        unsigned alpha
    );
/* Update playback rate */
void BIP_Play_GuiSetRate(
        BIP_Play_GuiHandle hGui,
        float rate
    );
/* Update playback position */
void BIP_Play_GuiSetPosition(
        BIP_Play_GuiHandle hGui,
        unsigned position
    );

/* Enable player status on UI */
void BIP_Play_GuiShowStatus(
        BIP_Play_GuiHandle hGui
    );

/* Disable player status on UI */
void BIP_Play_GuiHideStatus(
        BIP_Play_GuiHandle hGui
    );

/* Update Player status */
void BIP_Play_GuiSetPlayerStatus(
        BIP_Play_GuiHandle hGui,
        BIP_PlayerStatus *pStatus
    );

/* Update audio decoder status */
void BIP_Play_GuiSetAudioDecoderStatus(
        BIP_Play_GuiHandle hGui,
        NEXUS_AudioDecoderStatus *pStatus
    );

/* Update video decoder status */
void BIP_Play_GuiSetVideoDecoderStatus(
        BIP_Play_GuiHandle hGui,
        NEXUS_VideoDecoderStatus *pStatus
    );

/*********************************/
/********* PROGRESS BAR **********/
/*********************************/
/* Progress bar size and location */
#define BIP_PLAY_PROGRESS_BAR_WIDTH         (unsigned)(0.9*BIP_PLAY_FRAME_BUFFER_WIDTH)  /* 90% */
#define BIP_PLAY_PROGRESS_BAR_HEIGHT        (unsigned)(0.15*BIP_PLAY_FRAME_BUFFER_HEIGHT) /* 15 %*/

/* Placed at the bottom */
#define BIP_PLAY_PROGRESS_BAR_POS_X         (unsigned)(0.05*BIP_PLAY_FRAME_BUFFER_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_POS_Y         (unsigned)(0.8*BIP_PLAY_FRAME_BUFFER_HEIGHT)

#define BIP_PLAY_PROGRESS_BAR_BG_COLOR      0xA0404040 /* Translucent dark gray */
#define BIP_PLAY_COLOR_WHITE                0xFFFFFFFF /* Opaque White */

/* Position and color for progress bar's gui elements */
/* Playback time elapsed */
#define BIP_PLAY_PROGRESS_BAR_TIME_POS_X    (unsigned)(0.09*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_TIME_POS_Y    (unsigned)(0.55*BIP_PLAY_PROGRESS_BAR_HEIGHT)
#define BIP_PLAY_PROGRESS_BAR_TIME_WIDTH    (unsigned)(0.20*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_TIME_HEIGHT   (unsigned)(0.36*BIP_PLAY_PROGRESS_BAR_HEIGHT)

/* Stream Title */
#define BIP_PLAY_PROGRESS_BAR_TITLE_POS_X   (unsigned)(0.08*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_TITLE_POS_Y   (unsigned)(0.10*BIP_PLAY_PROGRESS_BAR_HEIGHT)
#define BIP_PLAY_PROGRESS_BAR_TITLE_WIDTH   (unsigned)(0.92*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_TITLE_HEIGHT  (unsigned)(0.36*BIP_PLAY_PROGRESS_BAR_HEIGHT)

/* Playback icon (play, pause, ffwd, rewind) */
#define BIP_PLAY_PROGRESS_BAR_ICON_POS_X    (unsigned)(0.01*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_ICON_POS_Y    (unsigned)(0.20*BIP_PLAY_PROGRESS_BAR_HEIGHT)
#define BIP_PLAY_PROGRESS_BAR_ICON_WIDTH    (unsigned)(0.06*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_ICON_HEIGHT   (unsigned)(BIP_PLAY_PROGRESS_BAR_ICON_WIDTH)

/* Playback cursor background*/
#define BIP_PLAY_PROGRESS_BAR_CURSORBG_POS_X    (unsigned)(0.19*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_CURSORBG_POS_Y    (unsigned)(0.62*BIP_PLAY_PROGRESS_BAR_HEIGHT)
#define BIP_PLAY_PROGRESS_BAR_CURSORBG_WIDTH    (unsigned)(0.71*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_CURSORBG_HEIGHT   (unsigned)(0.18*BIP_PLAY_PROGRESS_BAR_HEIGHT)
#define BIP_PLAY_PROGRESS_BAR_CURSORBG_COLOR    0xFFC0C0C0 /* Off white */

/* Playback cursor */
#define BIP_PLAY_PROGRESS_BAR_CURSOR_POS_X      (unsigned)(BIP_PLAY_PROGRESS_BAR_CURSORBG_POS_X)
#define BIP_PLAY_PROGRESS_BAR_CURSOR_POS_Y      (unsigned)(BIP_PLAY_PROGRESS_BAR_CURSORBG_POS_Y)
#define BIP_PLAY_PROGRESS_BAR_CURSOR_COLOR      0xFF40C040 /* Dull Green */

/* Playback time left */
#define BIP_PLAY_PROGRESS_BAR_TIME_LEFT_POS_X   (unsigned)(0.91*BIP_PLAY_PROGRESS_BAR_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_TIME_LEFT_POS_Y   (unsigned)(0.55*BIP_PLAY_PROGRESS_BAR_HEIGHT)
#define BIP_PLAY_PROGRESS_BAR_TIME_LEFT_WIDTH   (unsigned)(BIP_PLAY_PROGRESS_BAR_TIME_WIDTH)
#define BIP_PLAY_PROGRESS_BAR_TIME_LEFT_HEIGHT  (unsigned)(BIP_PLAY_PROGRESS_BAR_TIME_HEIGHT)

/* Elements in progress bar */
typedef enum BIP_Play_ProgressBarElement
{
    BIP_Play_ProgressBarElement_eBG,        /* Background */
    BIP_Play_ProgressBarElement_eTime,      /* Playback poistion hh:mm:ss format */
    BIP_Play_ProgressBarElement_eTitle,     /* Stream URL */
    BIP_Play_ProgressBarElement_eIcon,      /* Play/Pause/Fwd/Rew */
    BIP_Play_ProgressBarElement_eCursorBG,  /* Cursor Background */
    BIP_Play_ProgressBarElement_eCursor,    /* Cursor */
    BIP_Play_ProgressBarElement_eTimeLeft,  /* Time left hh:mm:ss format */
    BIP_Play_ProgressBarElement_eLast,
    BIP_Play_ProgressBarElement_eMax = BIP_Play_ProgressBarElement_eLast
} BIP_Play_ProgressBarElement;

/* Icons used in progress bar */
typedef enum BIP_Play_ProgressBarIcon
{
    BIP_Play_ProgressBarIcon_ePlay,
    BIP_Play_ProgressBarIcon_ePause,
    BIP_Play_ProgressBarIcon_eFwd2x,
    BIP_Play_ProgressBarIcon_eFwd4x,
    BIP_Play_ProgressBarIcon_eFwd8x,
    BIP_Play_ProgressBarIcon_eFwd16x,
    BIP_Play_ProgressBarIcon_eRew2x,
    BIP_Play_ProgressBarIcon_eRew4x,
    BIP_Play_ProgressBarIcon_eRew8x,
    BIP_Play_ProgressBarIcon_eRew16x,
    BIP_Play_ProgressBarIcon_eLast,
    BIP_Play_ProgressBarIcon_eMax = BIP_Play_ProgressBarIcon_eLast
} BIP_Play_ProgressBarIcon;

/*********************************/
/********** STATUS BAR ***********/
/*********************************/
/* Status bar size and location */
#define BIP_PLAY_STATUS_BAR_WIDTH         (unsigned)(BIP_PLAY_FRAME_BUFFER_WIDTH)       /* 100% */

/* Top left */
#define BIP_PLAY_STATUS_BAR_POS_X         (unsigned)(0)
#define BIP_PLAY_STATUS_BAR_POS_Y         (unsigned)(0)

#define BIP_PLAY_STATUS_BAR_BACKGROUND    "images/status_bar_bg.png"
#define BIP_PLAY_COLOR_WHITE              0xFFFFFFFF /* Opaque White */

#define BIP_PLAY_STATUS_BAR_TEXT_POS_X    (unsigned)(0.05*BIP_PLAY_STATUS_BAR_WIDTH)
#define BIP_PLAY_STATUS_BAR_TOP_MARGIN    (unsigned)(0.02*BIP_PLAY_FRAME_BUFFER_HEIGHT)

/* Elements in status bar */
typedef enum BIP_Play_StatusBarElement
{
    BIP_Play_StatusBarElement_eBG,            /* Background */
    BIP_Play_StatusBarElement_eVideoCodec,    /* Video codec / resolution */
    BIP_Play_StatusBarElement_eAudioCodec,    /* Audio codec / bit rate? */
    BIP_Play_StatusBarElement_eAvErrors,      /* AV decoding errors */
    BIP_Play_StatusBarElement_eBytesCons,     /* Bytes consumed by media player */
    BIP_Play_StatusBarElement_ePlayerState,   /* Player state */
    BIP_Play_StatusBarElement_eStreamDuration,/* Stream Duration */
    BIP_Play_StatusBarElement_eLast,
    BIP_Play_StatusBarElement_eMax = BIP_Play_StatusBarElement_eLast
} BIP_Play_StatusBarElement;

#endif /* _BIP_PLAY_GUI_H_ */
