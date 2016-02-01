/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
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
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "bboxreport_svgplot.h"
#include "nexus_platform_generic_features_priv.h"
#include "bmemperf_utils.h"
#include "namevalue.inc"

#define COLUMN_GAP              300
#define WINDOW_HGT              40
#define WINDOW_PAD              5
#define X_START                 0
#define BOX_WIDTH               130
#define HDMI_BOX_HEIGHT         120
#define COMPONENT_LINE_LENGTH   70
#define BOXMODE_TABLE_HTML_SIZE 4096
#define SVG_WIDTH               900

unsigned int textX = 300;                                  /* for debug text */
unsigned int textY = 430;                                  /* for debug text */
int          g_main_used            = 0;
int          g_pip_used             = 0;
int          g_num_encoders         = 0;
int          g_num_encoder_graphics = 0;
int          g_num_hdmi_inputs      = 0;
int          g_boxmode              = 0;
int          Xcoords    [10];
int          Ycoords    [10];
int          IdxCoords = 0;
int          RectCount = 101;

static void DrawSemiCircle(
    int X1,
    int Y1,
    int curveRight                                         /* 0 for right; 1 for left */
    )
{
    if (curveRight)
    {
        printf( "<path d=\"M%u,%u A5,5 0 0,1 %u,%u\" style=\"stroke:#0070C0; fill:#0070C0;\"/>\n", X1, Y1+20, X1, Y1 );
    }
    else
    {
        printf( "<path d=\"M%u,%u A5,5 0 0,1 %u,%u\" style=\"stroke:#0070C0; fill:#0070C0;\"/>\n", X1, Y1, X1, Y1+20 );
    }
}

static void DrawArrow(
    int         X1,
    int         Y1,
    int         X2,
    int         Y2,
    const char *marker
    )
{
    printf( "<line x1=%u y1=%u x2=%u y2=%u stroke=#0070C0 stroke-width=2 %s />\n", X1, Y1, X2, Y2, marker );
}

static void DrawDashedLine(
    int X1,
    int Y1,
    int X2,
    int Y2
    )
{
    printf( "<line stroke-width=\"4\" stroke-dasharray=\"5,5\" x1=%u y1=%u x2=%u y2=%u stroke=#0070C0 stroke-width=1 />\n", X1, Y1, X2, Y2 );
}

static void DrawText(
    int         X,
    int         Y,
    int         height,
    const char *description,
    const char *alignment
    )
{
    int lX = X + 22;
    int lY = Y + height / 2 + 5;

    printf( "<text x=\"%u\" y=\"%u\" fill=\"black\" text-anchor=\"%s\" >%s</text>\n", lX, lY, alignment, description );
}

static int DrawRect(
    int                                  X,
    int                                  Y,
    int                                  width,
    int                                  height,
    const char                          *rectTitle,
    const char                          *class,
    const Bboxreport_ElementDescriptive *multiline
    )
{
    int  offset            = 0;
    int  leftX             = X + width;
    int  bottomY           = Y + height;
    int  spacingX          = 20;
    int  spacingY          = 0;
    char mouseActions[256] = "";

    /* only activate mouseover/mouseout events for DECODE boxes */
    if (strstr( rectTitle, "DECODE" ))
    {
        unsigned int decoder = 0;
        sscanf( &rectTitle[7], "%u", &decoder );
        sprintf( mouseActions, "onmouseover=\"MyMouseOver(event, 'divbox%udecoder%u', 'rect%u' )\" onmouseout=\"MyMouseOut(event, 'divbox%udecoder%u', 'rect%u' )\" ",
            g_boxmode, decoder, RectCount, g_boxmode, decoder, RectCount );
    }
    else
    {
        mouseActions[0] = '\0';
    }
    printf( "<polygon points=\"%u,%u %u,%u %u,%u %u,%u \" class=%s id=rect%u onclick='MyClick(event)' %s />\n",
        X, Y, leftX, Y, leftX, bottomY, X, bottomY, class, RectCount, mouseActions );
    if (strlen( multiline->line[0] ) > 7)
    {
        spacingX = -20;                                    /* for 3840x2160p60, shift text to the left */
    }

    if (strstr( rectTitle, "DECODE" ) || strstr( rectTitle, "DISPLAY" ) || strstr( rectTitle, "ENCODE" ))
    {
        spacingY = -20;
    }
    else if (strstr( rectTitle, "GRAPHICS" ))
    {
        spacingY = -10;                                    /* graphics windows are short in height; shift text up a bit */
    }

#if 0
    printf( "<text x=%u y=%u >title:%s X:%03u; Y:%03u; wid:%04u; hgt:%u; center:%u</text>\n", textX, textY, rectTitle, X, Y, width, height, X + width/3 );
    textY += 20;
#endif
    DrawText( X + width/3, Y + spacingY, height, rectTitle, "middle" );
    /* output any/all lines of resolution/8bit/HEVC */
    offset = height;
    if (strlen( multiline->line[0] ))                      /* 3820x2160p60 */
    {
        DrawText( X + width/3, Y, offset, multiline->line[0], "middle" );
        offset += 40;
    }
    if (strlen( multiline->line[1] ))                      /* 10-bit */
    {
        STRING32 newline;
        memset( &newline, 0, sizeof( newline ));
        snprintf( newline, sizeof( newline ) - 1, "%s&nbsp;%s", multiline->line[1] /* 10-bit */, multiline->line[2] /* HEVC */ );
        DrawText( X + width/3, Y, offset, newline, "middle" );
        offset += 40;
    }

    RectCount++;
    return( bottomY );
}                                                          /* DrawRect */

static int DrawWindow(
    int         X,
    int         Y,
    int         width,
    int         height,
    const char *rectTitle,
    const char *description2,                              /* used for resolution */
    const char *csstag
    )
{
    int new_bottom = 0;
    Bboxreport_ElementDescriptive multiline;

    memset( &multiline, 0, sizeof( multiline ));

    new_bottom = DrawRect( X, Y, width, height, rectTitle, csstag, &multiline ); /*window*/
    DrawSemiCircle( X + width, Y + 10, 0 );                                      /* output (right side)*/
    if (strcmp( csstag, "windowbox" ) == 0)                                      /* skip the left side if window is graphics*/
    {
        DrawSemiCircle( X,         Y + 10, 1 );                                  /* input  (left side)*/
    }
    DrawSemiCircle( X + width + 70, Y + 10, 1 );                                 /* semicircle on left of display */
    if (description2)
    {
        DrawText( X + width/3, Y + 22, 14, description2, "middle" );
    }

    return( new_bottom );
}                                                          /* DrawWindow */

static unsigned int gfxIdx     = 0;
static unsigned int windowIdx  = 0;
static unsigned int displayIdx = 0;
static unsigned int encoderIdx = 0;

static int DrawWindowDisplay(
    int                 X,
    int                 Y,
    int                 width,
    int                 height,
    int                 decoder,
    const char         *tag,
    Bboxreport_SvgPlot *svgPlot
    )
{
    int   new_bottom         = 0;
    int   display_box_height = ( g_pip_used && decoder<=1 ) ? height * 3 + WINDOW_PAD : height * 2; /* if pip used, the display box needs to be taller to allow for extra window */
    bool  isMain             = false;
    bool  isPip              = false;
    bool  showSdDisplay      = false;
    char *description        = malloc( strlen( tag ) + 24 );
    char *description2       = NULL;                       /* used for graphics resolution */
    Bboxreport_ElementDescriptive multiline;

    memset( &multiline, 0, sizeof( multiline ));

    isMain        = decoder==0 && g_main_used;
    isPip         = decoder==1 && g_pip_used;
    showSdDisplay = (( displayIdx==1 ) && ( svgPlot->display[displayIdx].numVideoWindows>0 ));

#if 0
    printf( "<text x=%u y=%u >X:%03u; Y:%03u; wid:%04u; hgt:%u; dec:%u; pip:%u; isMain:%u; isPip:%u</text>\n",
        textX, textY, X, Y, width, height, decoder, g_pip_used, isMain, isPip );
    textY += 20;
#endif

    sprintf( description, "%s", "WINDOW" );
    new_bottom = DrawWindow( X + COLUMN_GAP, Y, width, height, description, description2, "windowbox" ); /* window */
    DrawArrow( X + COLUMN_GAP + width, Y + height / 2, X + 2*COLUMN_GAP - 100, Y + height / 2, "" );     /*connect window with display */
    DrawArrow( X + COLUMN_GAP,         Y + height / 2, X + COLUMN_GAP - 92,     Y + height / 2, "" );    /*connect window with dashed vertical line */
    windowIdx++;

    /* when pip used, we need to add another window box */
    if (svgPlot->display[decoder].numVideoWindows > 1)
    {
        int temp = 0;
        sprintf( description, "%s", "WINDOW" );
        temp = DrawWindow( X + COLUMN_GAP, new_bottom + WINDOW_PAD, width, height, description, description2, "windowbox" );                                                    /* window */
        DrawArrow( X + COLUMN_GAP + width, new_bottom + ( height + WINDOW_PAD ) / 2 + 3, X + COLUMN_GAP + COLUMN_GAP - 100, new_bottom + ( height + WINDOW_PAD ) / 2 + 3, "" ); /*connect window with display */
        DrawArrow( X + COLUMN_GAP,         new_bottom + height / 2 + 5, X + COLUMN_GAP - 92,     new_bottom + height / 2 + 5, "" );                                             /*connect window with dashed vertical line */
        windowIdx++;
        new_bottom = temp;
    }
#if 0
    printf( "<text x=%u y=%u >X %u; Y (%u); X2 (%u); Y2 (%u); showSdDisplay:%u; vdec[%u].gfxRes:(%s)</text>\n",
        textX, textY, X + COLUMN_GAP,         Y + height / 2, X + COLUMN_GAP - 90,     Y + height / 2,
        showSdDisplay, decoder, svgPlot->videoDecoder[decoder].str_gfx_resolution );
    textY += 20;
#endif /* if 0 */

    if (isMain || showSdDisplay || g_num_encoder_graphics)
    {
        sprintf( description, "%s %u", "GRAPHICS", gfxIdx );
        if (isMain)
        {
            description2 = svgPlot->videoDecoder[decoder].str_gfx_resolution;
        }
        else if (showSdDisplay)
        {
            description2 = svgPlot->videoDecoder[decoder].str_gfx_resolution;
        }
        else
        {
            description2 = svgPlot->encoder_gfx_resolution;
        }
        DrawWindow( X + COLUMN_GAP, new_bottom + WINDOW_PAD, width, height, description, description2, "graphicsbox" );                                                 /*graphics */
        DrawArrow( X + COLUMN_GAP + width, new_bottom + ( height + WINDOW_PAD ) / 2, X + COLUMN_GAP + COLUMN_GAP - 100, new_bottom + ( height + WINDOW_PAD ) / 2, "" ); /*connect window with display */
        gfxIdx++;
#if 0
        printf( "<text x=%u y=%u >hdmi %u; displayIdx (%u); main_used (%u); gfxIdx (%u)</text>\n", textX, textY, g_num_hdmi_inputs, displayIdx, g_main_used, gfxIdx );
        textY += 20;
#endif
    }

    sprintf( description, "%s %u", "DISPLAY", displayIdx );

    if (isMain || showSdDisplay)
    {
        strncpy( multiline.line[0], svgPlot->display[displayIdx].str_max_format, sizeof( multiline.line[0] ) - 1 );
        DrawRect( X + COLUMN_GAP + COLUMN_GAP - 100, Y, width, display_box_height, description, "displaybox", &multiline ); /*display */

        strncpy( multiline.line[0], svgPlot->display[displayIdx].str_max_format, sizeof( multiline.line[0] ) - 1 );
        DrawRect( X + COLUMN_GAP + COLUMN_GAP - 100, Y, width, display_box_height, description, "displaybox", &multiline ); /*display */
        if (isMain)
        {
            DrawArrow( X + 2* COLUMN_GAP + BOX_WIDTH - 100, Y + height / 2,      X + 2*COLUMN_GAP + COMPONENT_LINE_LENGTH, Y + height / 2, "marker-end=\"url(#Arrow)\"" ); /* HDMI out */
            DrawText( X + 2* COLUMN_GAP + BOX_WIDTH - 75, Y + WINDOW_PAD + 5, 20, "OUTPUTS", "start" );
            #if 0
            DrawArrow( X + 2* COLUMN_GAP + BOX_WIDTH - 100, Y + height + 20,     X + 2*COLUMN_GAP + COMPONENT_LINE_LENGTH, Y + height + 20, "marker-end=\"url(#Arrow)\""  ); /* COMPONENT out */
            DrawText( X + 2* COLUMN_GAP + BOX_WIDTH - 75, Y + WINDOW_PAD + 45, 20, "COMPONENT", "start" );
            #endif
        }
        else
        {
            DrawArrow( X + 2* COLUMN_GAP + BOX_WIDTH - 100, Y + WINDOW_PAD + 25, X + 2*COLUMN_GAP + COMPONENT_LINE_LENGTH, Y + WINDOW_PAD + 25, "marker-end=\"url(#Arrow)\""  ); /* CVBS out */
            DrawText( X + 2* COLUMN_GAP + BOX_WIDTH - 75, Y + WINDOW_PAD + 15, 20, "OUTPUTS", "start" );
        }
    }
    else                                                   /* must be encoder */
    {
        /* the encoders tell us which display index to use */
        int temp = svgPlot->videoEncoder[encoderIdx].displayIndex;
        if (temp < NEXUS_MAX_DISPLAYS)
        {
            strncpy( multiline.line[0], svgPlot->display[temp].str_max_format, sizeof( multiline.line[0] ) - 1 );
        }
#if 0
        printf( "<text x=%u y=%u >encode[%u] displayIndex:%u multiline:(%s) </text>\n", textX, textY, encoderIdx, temp, multiline.line[0] );
        textY += 20;
#endif
        DrawRect( X + COLUMN_GAP + COLUMN_GAP - 100, Y, width, display_box_height, description, "displaybox", &multiline ); /*display */

        strncpy( multiline.line[0], svgPlot->videoEncoder[encoderIdx].str_max_resolution, sizeof( multiline.line[0] ) - 1 );
        sprintf( description, "%s %u", "ENCODE", encoderIdx );
        DrawRect( X + 2*COLUMN_GAP + 100, Y, width, display_box_height, description, "encoderbox", &multiline ); /* encoder */
        DrawSemiCircle( X + 2*COLUMN_GAP + 100, Y + 30, 1 );                                                     /* semicircle on left  of encoder box */
        DrawSemiCircle( X + 2*COLUMN_GAP + 30,  Y + 30, 0 );                                                     /* semicircle on right of display box */
        DrawArrow( X + 2*COLUMN_GAP + 100, Y + 40, X + 2*COLUMN_GAP + 30,  Y + 40, "" );                         /*connect encoder window with display */
        encoderIdx++;
    }

    displayIdx++;

    free( description );

    return( display_box_height + 30 );
}                                                          /* DrawWindowDisplay */

static void DrawDecoder(
    int                                  X,
    int                                  Y,
    int                                  width,
    int                                  height,
    int                                  decoder,
    const char                          *tag,
    const Bboxreport_ElementDescriptive *multiline
    )
{
    char *description = malloc( strlen( tag ) + 24 );
    int   rect_center = Y + WINDOW_HGT - 10;

    sprintf( description, "%s %u", tag, decoder );
    if (strcmp( tag, "HDMIrx" ) == 0) {decoder--; }
    DrawRect( X, Y, width, height * 2, description, "decoderbox", multiline ); /*decoder */
    if (strcmp( tag, "HDMIrx" ) == 0) {decoder++; }
    DrawSemiCircle( X + width, rect_center, 0 );
    DrawArrow( X + width + 10 /*width of semicircle*/, rect_center+ 10, X + width + ( COLUMN_GAP/4 ), rect_center+ 10, "" ); /*connect decoder with vertical dashed line */

    free( description );
}                                                          /* DrawDecoder */

const char *get_maxformat_name(
    NEXUS_VideoFormat maxFormat
    )
{
    return( lookup_name( g_videoFormatStrs, maxFormat ));
}

const char *get_codec_name(
    int codec
    )
{
    return( lookup_name( g_videoCodecStrs, codec ));
}

int get_codec_value(
    int codec
    )
{
    return( lookup( g_videoCodecStrs, lookup_name( g_videoCodecStrs, codec )));
}

#define CODEC_STR_SIZE 1024

char *getVideoCodecsStr(
    unsigned int                       whichDecIdx,
    NEXUS_MemoryConfigurationSettings *memConfigSettings
    )
{
    unsigned int codec      = 0;
    unsigned int codecCount = 0;
    static char  codecStr[CODEC_STR_SIZE];

    #if 0
    char tempStr[32];
    #endif

    memset( codecStr, 0, sizeof( codecStr ));

    #if 0
    sprintf( tempStr, "dec[%u]: ", whichDecIdx );
    strncat( codecStr, tempStr, CODEC_STR_SIZE - strlen( codecStr ) -1 );
    #endif

    for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
    {
        if (whichDecIdx==99)
        {
            printf( "~DEBUG~%u:videoDecoder:%u; supported[%u:%s]:%u; \n~", __LINE__, whichDecIdx,
                codec, get_codec_name( codec ), memConfigSettings->videoDecoder[whichDecIdx].supportedCodecs[codec] );
        }
        /* codec is valid for the specified videoDecoder */
        if (memConfigSettings->videoDecoder[whichDecIdx].supportedCodecs[codec])
        {
            /* if something already added to string, add separator */
            if (codecCount)
            {
                strncat( codecStr, " ", CODEC_STR_SIZE - strlen( codecStr ) -1 );
            }
            strncat( codecStr, get_codec_name( codec ), CODEC_STR_SIZE - strlen( codecStr ) -1 );

            #if 0
            sprintf( tempStr, "[%u] ", get_codec_value( codec ));
            strncat( codecStr, tempStr, CODEC_STR_SIZE - strlen( codecStr ) -1 );
            #endif

            codecCount++;
        }
    }

    return( codecStr );
}                                                          /* getVideoCodecsStr */

/* rect-circle -p 1 -h 2 -e 4 (p=>pip, h->hdmi, e->encoders */
int Create_SvgPlot(
    Bboxreport_SvgPlot                *svgPlot,
    NEXUS_MemoryConfigurationSettings *memConfigSettings
    )
{
    unsigned int                  idx          = 0;
    unsigned int                  Ymax         = 50;       /* 1300 is about the most that can fit on the page */
    unsigned int                  Ytop         = 0;
    unsigned int                  num_decoders = svgPlot->num_decoders;
    unsigned int                  num_displays = 0;
    Bboxreport_ElementDescriptive multiline;

    if (memConfigSettings == NULL)
    {
        return( 0 );
    }
    g_main_used            = ( svgPlot->num_decoders - svgPlot->num_encoders )>=1;
    g_pip_used             = ( svgPlot->display[1].numVideoWindows > 1 );
#if NEXUS_HAS_HDMI_INPUT
    g_num_hdmi_inputs      = svgPlot->num_hdmi_inputs;
#endif
    g_num_encoder_graphics = svgPlot->num_encoders_graphics;
    g_boxmode              = svgPlot->boxmode;

    /* add a page-break after the first boxmode is output */
    /*if (svgPlot->boxmodeFirst == 0)*/
    {
        printf( "<p style=\"page-break-after:always;\"></p>\n" );
    }

    #if defined WATERMARK
    printf( "<img src=white_spacer.png height=0 id=svg%03uspacer >", svgPlot->boxmode );
    /*printf( "<div class=watermark id=watermark%u >Broadcom</div>\n", svgPlot->boxmode );*/
    #else
    printf( "<svg id=svg%03uspacer class=nodisplay height=\"100\" width=\"%u\" style=\"border:1px solid white;font-family:Arial;\" xmlns='http://www.w3.org/2000/svg' version='1.1' >see this</svg>\n",
        svgPlot->boxmode, SVG_WIDTH );
    #endif /* WATERMARK */
    printf( "<h2 class=margin20 >%s BoxMode %u %s</h2>\n", svgPlot->platform, svgPlot->boxmode, svgPlot->strDdrScb );
    printf( "<svg id=svg%03u height=\"800\" width=\"%u\" style=\"border:1px solid white;font-family:Arial;\" xmlns='http://www.w3.org/2000/svg' version='1.1' >\n",
        svgPlot->boxmode, SVG_WIDTH );
    /*printf( "<text x=\"0\" y=\"0\" fill=\"lightgray\" transform=\"translate(30) rotate(45 50 50)\" style=\"font-size:150;\" >Broadcom</text>\n" );*/
    printf( " <defs>\n" );
    printf( "    <marker id=\"Arrow\"  markerWidth=\"8\" markerHeight=\"8\" viewBox=\"-6 -6 12 12\"  refX=\"0\" refY=\"0\"  markerUnits=\"strokeWidth\"  orient=\"right\" >\n" );
    printf( "       <polygon points=\"-2,0 -5,5 5,0 -5,-5\" fill=\"#0070C0\" stroke=\"#0070C0\" stroke-width=\"1px\"/>\n" );
    printf( "    </marker>\n" );
    printf( " </defs>\n" );

#if 0
    printf( "<text x=%u y=%u >main %u; pip (%u); encoders (%u); gfx (%u); hdmi (%u); decoders (%u)</text>\n",
        textX, textY, svgPlot->main_used, svgPlot->pip_used, svgPlot->num_encoders,
        svgPlot->num_encoders_graphics, svgPlot->num_hdmi_inputs, num_decoders );
    textY += 20;
#endif /* if 0 */

    for (idx = 0; idx<num_decoders; idx++)
    {
        memset( &multiline, 0, sizeof( multiline ));

        strncpy( multiline.line[0], svgPlot->videoDecoder[idx].str_max_resolution, sizeof( multiline.line[0] ) - 1 );
        if (svgPlot->videoDecoder[idx].color_depth > 0)
        {
            snprintf( multiline.line[1], sizeof( multiline.line[1] ) - 1, "%u-bit", svgPlot->videoDecoder[idx].color_depth );
        }
        /*strncpy( multiline.line[2], svgPlot->videoDecoder[idx].str_compression, sizeof( multiline.line[2] ) - 1 );*/

#if 0
        printf( "<text x=%u y=%u >line1 (%s); line2 (%s); line3 (%s)</text>\n", textX, textY, multiline.line[0], multiline.line[1], multiline.line[2] );
        textY += 20;
#endif /* if 0 */

        DrawDecoder( X_START, Ytop, BOX_WIDTH, WINDOW_HGT, idx, "DECODE", &multiline );
        Ytop += HDMI_BOX_HEIGHT;
        if (Ytop > Ymax)
        {
            Ymax = Ytop;
        }
    }

#if NEXUS_HAS_HDMI_INPUT
    for (idx = 0; idx< (unsigned int) svgPlot->num_hdmi_inputs; idx++)
    {
        memset( &multiline, 0, sizeof( multiline ));

        DrawDecoder( X_START, Ytop, BOX_WIDTH, WINDOW_HGT, idx, "HDMIrx", &multiline );
        Ytop += HDMI_BOX_HEIGHT;
        if (Ytop > Ymax) {Ymax = Ytop; }
    }
#endif

    Ytop = 0;

    /* calculate the number of display windows to create */
    for (idx = 0; idx<NEXUS_MAX_DISPLAYS; idx++)
    {
        if (svgPlot->display[idx].numVideoWindows /* could be 0, 1, or 2 */)
        {
            num_displays++;
        }
    }

    for (idx = 0; idx<num_displays; idx++)
    {
        Ytop += DrawWindowDisplay( X_START, Ytop, BOX_WIDTH, WINDOW_HGT, idx, "DECODE", svgPlot );
        if (Ytop > Ymax)
        {Ymax = Ytop; }
    }

    /*printf("<text x=%u y=%u >DrawArrow(%u, %u, %u %u)</text>\n", textX, textY, BOX_WIDTH + 75, 5, BOX_WIDTH + 75, Ymax ); */
    DrawDashedLine( BOX_WIDTH + 77, 5, BOX_WIDTH + 77, Ymax );

    printf( "</svg><hr class=no-print width=\"%upx\" align=left style=\"height:1px;background-color:black;color:black;\" >", SVG_WIDTH );
    #if 0
    /* create popup elements that will be displayed when the user hovers the cursor over one of the DECODE boxes */
    for (idx = 0; idx<num_decoders; idx++)
    {
        char *codecStr = getVideoCodecsStr( idx, memConfigSettings );
        char *posSpace = NULL;
        printf( "<div id=divbox%udecoder%u class=\"padded\" style=\"display:none;\" ><table cellpaddign=10 ><tr><th>CODECS:</th></tr>", svgPlot->boxmode, idx );

        /* loop through seach individual codec and output each on a separate table row */
        while (*codecStr)
        {
            /* codecStr:mpeg2 mpeg1 h264 vp7 */
            posSpace = strstr( codecStr, " " );
            if (posSpace)
            {
                *posSpace = '\0';
            }
            printf( "<tr><td align=center>%s</td></tr>", codecStr );
            /* advance to the next codec name */
            codecStr += strlen( codecStr );
            if (posSpace)
            {
                codecStr++;                                /* if we found a space in the string, advance past the space/null-terminator */
            }
        }
        printf( "</table></div>" );
    }
    #endif /* if 0 */

    printf( "~SVGHEIGHT~svg%03u|%u", svgPlot->boxmode, Ymax );

    /* try to force the svg plot to the center of the page */
    if (Ymax < 1300)
    {
        int deltaHalf = ( 1300 - Ymax )/2;
        /* just dividing the delta by two resulted in plots that seemed to be too low on the page. If possible, raise the plot this many pixels on the page */
        if (deltaHalf > 50)
        {
            deltaHalf -= 50;
        }
        printf( "~SVGSPACER~svg%03uspacer|%u", svgPlot->boxmode, deltaHalf );
    }

    return( 0 );
}                                                          /* Create_SvgPlot */

int Add_BoxmodeTableHtml(
    Bboxreport_SvgPlot                *svgPlot,
    char                              *boxmodeTableHtml,
    NEXUS_MemoryConfigurationSettings *memConfigSettings
    )
{
    int  idx      = 0;
    int  codec    = 0;
    bool pip_used = false;
    char newline[256];
    int  num_hdmi_inputs = 0;

    if (memConfigSettings == NULL)
    {
        return( 0 );
    }

#if NEXUS_HAS_HDMI_INPUT
    num_hdmi_inputs = svgPlot->num_hdmi_inputs;
#endif
    snprintf( newline, sizeof( newline ) - 1, "<tr><td class=border align=center title=test >%d</td>", svgPlot->boxmode );
    PRINTF( "~DEBUG~new boxmode row 1 (%s)~", newline );
    strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );

    pip_used = ( svgPlot->display[1].numVideoWindows >= 2 );
    snprintf( newline, sizeof( newline ) - 1, "<td class=border align=center >%s</td>", ( pip_used ) ? "YES" : "" );
    strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );

    for (idx = 0; idx<NEXUS_MAX_VIDEO_DECODERS; idx++)
    {
        snprintf( newline, sizeof( newline ) - 1, "<td class=border align=center >" );
        strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );

        if (svgPlot->videoDecoder[idx].enabled)
        {
            int excludedCodecCount = 0;

            if (svgPlot->videoDecoder[idx].color_depth)
            {
                snprintf( newline, sizeof( newline ) - 1, "%s %u-bit", svgPlot->videoDecoder[idx].str_max_resolution,
                    svgPlot->videoDecoder[idx].color_depth );
                strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
            }

            #if 0
            if (strlen( svgPlot->videoDecoder[idx].str_gfx_resolution ))
            {
                snprintf( newline, sizeof( newline ) - 1, "<br>gfx:%s", svgPlot->videoDecoder[idx].str_gfx_resolution );
                strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
            }
            snprintf( newline, sizeof( newline ) - 1, "</td>" );
            strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );

            snprintf( newline, sizeof( newline ) - 1, "<br><span style=\"font-size:8pt;\">(%s)</span>", getVideoCodecsStr( idx, memConfigSettings ));
            strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
            #endif /* if 0 */

            for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
            {
                if (memConfigSettings->videoDecoder[idx].supportedCodecs[codec] != svgPlot->supportedCodecsSuperset[codec])
                {
                    /* if the is the first excluded codec, output the beginning of the span code */
                    if (excludedCodecCount == 0)
                    {
                        snprintf( newline, sizeof( newline ) - 1, "<br><span style=\"font-size:8pt;\">(excl: " );
                        strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
                    }
                    else                                   /* add a separator before appending the next codec name */
                    {
                        snprintf( newline, sizeof( newline ) - 1, ", " );
                        strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
                    }
                    snprintf( newline, sizeof( newline ) - 1, "%s", get_codec_name( codec ));
                    strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );

                    excludedCodecCount++;
                }
            }
            /* if any codec names were output above, close the span element */
            if (excludedCodecCount)
            {
                snprintf( newline, sizeof( newline ) - 1, ")</span>" );
                strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
            }
        }
    }

    snprintf( newline, sizeof( newline ) - 1, "<td class=border align=center >%u</td><td class=border align=center >", num_hdmi_inputs );
    strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
    if (svgPlot->num_encoders)
    {
        snprintf( newline, sizeof( newline ) - 1, "%u @ %s", svgPlot->num_encoders, svgPlot->videoEncoder[0].str_max_resolution );
        strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );

        if (strlen( svgPlot->encoder_gfx_resolution ))
        {
            snprintf( newline, sizeof( newline ) - 1, "<br>gfx:%s", svgPlot->encoder_gfx_resolution );
            strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );
        }
    }
    snprintf( newline, sizeof( newline ) - 1, "</td></tr>\n" );
    strncat( boxmodeTableHtml, newline, BOXMODE_TABLE_HTML_SIZE - strlen( boxmodeTableHtml ) - 1 );

    return( 0 );
}                                                          /* Add_BoxmodeTableHtml */

int printCodecs(
    NEXUS_MemoryConfigurationSettings *memConfigSettings
    )
{
    int idx;
    int codec;

    for (idx = 0; idx<NEXUS_MAX_VIDEO_DECODERS; idx++)
    {
        printf( "~DEBUG~%s: videoDecoder[%u].supported: ", __FUNCTION__, idx );
        for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
        {
            /* codec is valid for the specified videoDecoder */
            if (memConfigSettings->videoDecoder[idx].supportedCodecs[codec])
            {
                printf( "[%u:%s]; ", codec, get_codec_name( codec ));
            }
        }
        printf( "~" );
    }
    return( 0 );
} /* printCodecs */

#if STANDALONE
int main(
    int   argc,
    char *argv[]
    )
{
    unsigned int       idx;
    unsigned int       boxmode = 0;
    char              *pos     = NULL;
    Bboxreport_SvgPlot svgPlot;
    char              *lpszQueryString = getenv( "QUERY_STRING" );
    char               newline[2048];
    char               boxmodeTableHtml[BOXMODE_TABLE_HTML_SIZE];

    printf( "Content-type: text/html\n\n" );               // needed for cgi to work
    sprintf( newline, "~DEBUG~QUERY_STRING (%s)~", lpszQueryString );
    printf( "%s", newline );
    memset( &svgPlot, 0, sizeof( svgPlot ));
    memset( &boxmodeTableHtml, 0, sizeof( boxmodeTableHtml ));

    strcpy( svgPlot.platform, "97445" );

    if (strstr( lpszQueryString, "&init=" ))
    {
        sprintf( newline, "~DEBUG~detected init=~" );
        printf( "%s", newline );
        printf( "~boxmodes~1|3|6|7|8|12|14|1001~" );
    }
    else if (( pos = strstr( lpszQueryString, "&boxmode=" )))
    {
        sprintf( newline, "~DEBUG~detected boxmode=~" );
        printf( "%s", newline );

        if (argc > 1)
        {
            unsigned int value;
            for (idx = 1; idx < argc; idx++) {
                if (argv[idx][1] == 'm')
                {
                    sscanf( argv[idx + 1], "%u", &value );
                    svgPlot.main_used = value;
                }
                else if (argv[idx][1] == 'p')
                {
                    sscanf( argv[idx + 1], "%u", &value );
                    svgPlot.pip_used = value;
                }
                else if (argv[idx][1] == 'h')
                {
                    sscanf( argv[idx + 1], "%u", &value );
                    svgPlot.num_hdmi_inputs = NEXUS_NUM_HDMI_INPUTS = value;
                }
                else if (argv[idx][1] == 'e')
                {
                    sscanf( argv[idx + 1], "%u", &value );
                    svgPlot.num_encoders = NEXUS_NUM_ENCODERS = value;
                }
                else if (argv[idx][1] == 'g')
                {
                    sscanf( argv[idx + 1], "%u", &value );
                    svgPlot.num_encoders_graphics = NEXUS_NUM_ENCODERS_GRAPHICS = value;
                }
                idx++;
            }
        }

        pos += strlen( "&boxmode=" );
        sscanf( pos, "%u", &boxmode );
        sprintf( newline, "~DEBUG~sscanf boxmode=%u~", boxmode );
        printf( "%s", newline );
        fflush( stdout ); fflush( stderr );
        gfxIdx          = 0;
        windowIdx       = 0;
        displayIdx      = 0;
        encoderIdx      = 0;
        encoderGfxIdx   = 0;
        svgPlot.boxmode = boxmode;

        if (boxmode == 7)
        {
            svgPlot.num_encoders          = 6;
            svgPlot.num_encoders_graphics = 6;
        }
        else if (boxmode%2 == 0)
        {
            svgPlot.main_used             = g_main_used = 1;
            svgPlot.num_encoders          = 3;
            svgPlot.num_encoders_graphics = 3;
            svgPlot.num_hdmi_inputs       = 1;
        }
        else if (boxmode%2 == 1)
        {
            svgPlot.main_used             = g_main_used = 1;
            svgPlot.pip_used              = g_pip_used  = 1;
            svgPlot.num_encoders          = 3;
            svgPlot.num_encoders_graphics = 3;
        }

        Add_BoxmodeTableHtml( &svgPlot, boxmodeTableHtml );
        printf( "~PAGECONTENTS~" );
        Create_SvgPlot( &svgPlot );
        printf( "~" );
    }
    /* simulate the time it would normally take to complete NEXUS_Init() and NEXUS_Uninit() */
    usleep( 800000 );
    printf( "~BOXMODETABLE~%s~", boxmodeTableHtml );

    printf( "~ALLDONE~" );

    return( 0 );
}                                                          /* main */

#endif /* STANDALONE */
