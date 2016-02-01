/***************************************************************************
 * (c) 2002-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/

#include "widget_grid.h"
#include "string.h"
#include "bdbg.h"

BDBG_MODULE(widget_grid);

#define ABS(x)  (((x) < 0) ? -(x) : (x))

/* override default label draw routine to add meter option */
void grid_bwidget_draw(
        bwidget_t         widget,
        const bwin_rect * cliprect
        )
{
    bwin_t            win = bwidget_win(widget);
    bwin_settings     win_settings;
    int               right, bottom, middle, half;
    blabel_settings * label_settings;
    blabel_settings   labelSettings;
    CWidgetGrid *     pGrid;

    blabel_get(widget, &labelSettings);
    label_settings = &labelSettings;

    pGrid = (CWidgetGrid *)label_settings->widget.data;
    BDBG_ASSERT(NULL != pGrid);

    /* TODO: for efficieny, we may want bwidgets to have access to bwin private structures. */
    bwin_get(win, &win_settings);

    /* only draw if something's within the cliprect */
    if (cliprect && !BWIN_RECT_ISINTERSECTING(&win_settings.rect, cliprect))
    {
        return;
    }

    pGrid->drawBackground(widget, label_settings, win_settings.rect, cliprect);
    pGrid->drawBevel(widget, label_settings, win_settings.rect, cliprect);

    right  = win_settings.rect.x + win_settings.rect.width;
    middle = win_settings.rect.x + win_settings.rect.width/2;
    half   = win_settings.rect.height/2;
    bottom = win_settings.rect.y + win_settings.rect.height;

    /* X Axis - use average color between left/right bevels */
    bwin_draw_line(win, middle, win_settings.rect.y, middle, bottom,
            ((label_settings->bevel_color[1] + label_settings->bevel_color[3]) / 2) | COLOR_BLACK,
            cliprect);

    /* Y Axis - use average color between top/bottom bevels */
    bwin_draw_line(win, win_settings.rect.x, half, win_settings.rect.width, half,
            ((label_settings->bevel_color[0] + label_settings->bevel_color[2]) / 2) | COLOR_BLACK,
            cliprect);

    /* Draw X/Y Coordiantes */
    {
        int     w, h, i, max;
        int16_t gridx, gridy;

        max = pGrid->getNumPoints();

        w = win_settings.rect.width - label_settings->bevel;
        h = win_settings.rect.height - label_settings->bevel;

        for (i = 0; i < max; i++)
        {
            /* find the center */
            pGrid->getCoordinate(&gridx, &gridy, i);
            int x = middle;
            int y = half;

            /* convert width/height from 32768/-32767 to +/- width/2 and height/2 */
            x += gridx * w / 32768;
            y += gridy * h / 32768;
            BDBG_MSG(("%d,%d => %d,%d", gridx, gridy, x, y));

            /* prevent the app from crashing if our algorithm isn't quite right */
            if ((x >= (int)win_settings.rect.width) || (y >= (int)win_settings.rect.height) || (x < 0) || (y < 0))
            {
                BDBG_ERR(("Invalid constellation value: %d %d", x, y));
                continue;
            }

            /* Draw Constellation points */
            bwin_fill_ellipse(win, x, y, 1, 1, COLOR_BLACK, cliprect);
        }
    }

    pGrid->drawFocus(widget, label_settings, win_settings.rect, cliprect);
} /* grid_bwidget_draw */

CWidgetGrid::CWidgetGrid(
        const char *     strName,
        bwidget_engine_t engine,
        CWidgetBase *    pParentWidget,
        MRect            geometry,
        bwin_font_t      font,
        bwin_t           parentWin
        ) :
    CWidgetLabel(strName, engine, pParentWidget, geometry, font, parentWin),
    _showGrid(false),
    _imageGrid(NULL),
    _imageRenderMode(bwin_image_render_mode_tile),
    _color(0xFF80C42F),
    _level(0),
    _showRange(false),
    _rangeSize(1),
    _numPoints(0)
{
    blabel_settings labelSettings;

    /* use custom draw routine so we can draw the background Meter bar */
    blabel_get(getWidget(), &labelSettings);
    labelSettings.text_justify_horiz = bwidget_justify_horiz_center;
    labelSettings.widget.draw        = grid_bwidget_draw;
    blabel_set(getWidget(), &labelSettings);
}

CWidgetGrid::~CWidgetGrid()
{
    if (NULL != _imageGrid)
    {
        bwin_image_close(_imageGrid);
    }
}

void CWidgetGrid::setColor(uint32_t color)
{
    _color = color;
    bwin_repaint(bwidget_win(getWidget()), NULL);
}

eRet CWidgetGrid::loadMeterImage(
        const char *           filename,
        bwin_image_render_mode renderMode
        )
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(NULL != filename);

    if (NULL != _imageGrid)
    {
        bwin_image_close(_imageGrid);
    }

    _imageGrid = bwin_image_load(getWinEngine(), filename);
    CHECK_PTR_ERROR_GOTO("unable to load bwin image", _imageGrid, ret, eRet_ExternalError, error);

    _imageRenderMode = renderMode;

    bwin_repaint(bwidget_win(getWidget()), NULL);

error:
    return(ret);
} /* loadMeterImage */

void CWidgetGrid::showMeter(bool show)
{
    _showGrid = show;

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

/* sets the Meter value (range -32767-32767) */
void CWidgetGrid::setLevel(int16_t level)
{
    _level = level;

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

/* Add an X,Y pair */
void CWidgetGrid::addCoordinate(
        int16_t x,
        int16_t y
        )
{
    gridPoints * gridPts = new gridPoints;

    gridPts->x = x;
    gridPts->y = y;

    BDBG_MSG(("Add coordinates %d %d", x, y));
    _gridPoints.add(gridPts);

    bwin_repaint(bwidget_win(getWidget()), NULL);
}

/* Get Coordinates */
void CWidgetGrid::getCoordinate(
        int16_t * x,
        int16_t * y,
        int       i
        )
{
    MListItr<gridPoints> itr(&_gridPoints);
    gridPoints *         pGridPts = NULL;

    if ((x == NULL) || (y == NULL))
    {
        BDBG_ERR((" cannot pass in NULL pointers"));
        return;
    }

    if ((i > _gridPoints.total()) || (i < 0))
    {
        BDBG_ERR(("Cannot request index %d", i));
        return;
    }

    pGridPts = itr.at(i);
    if (pGridPts == NULL)
    {
        BDBG_WRN(("Cannot request Grid Points at index %d, Its NULL", i));
        return;
    }

    *x = pGridPts->x;
    *y = pGridPts->y;
} /* getCoordinate */

/* Number of Actual Points */
uint32_t CWidgetGrid::getNumPoints()
{
    return(_gridPoints.total());
}

/* Clear Grid */
void CWidgetGrid::clearGrid()
{
    _gridPoints.clear();
}