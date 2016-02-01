/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
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

#ifndef BWIN_RECT_H__
#define BWIN_RECT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
A rectangle.
Description:
If both width and height are 0, the rectangle is considered to be NULL.
If either width or height are 0, bwin_rect describes a line.
**/
typedef struct {
    int x; /* can be < 0 */
    int y; /* can be < 0 */
    unsigned width;
    unsigned height;
} bwin_rect;

/**
Summary:
Test if two rectangles are equal.
**/
#define BWIN_RECT_ISEQUAL(PRECT1, PRECT2)( \
    (PRECT1)->x == (PRECT2)->x && \
    (PRECT1)->y == (PRECT2)->y && \
    (PRECT1)->width == (PRECT2)->width && \
    (PRECT1)->height == (PRECT2)->height)

#define BWIN_RECT_ISINSIDE(PRECT_OUTSIDE, PRECT_INSIDE) (\
    (PRECT_OUTSIDE)->x <= (PRECT_INSIDE)->x && \
    (PRECT_OUTSIDE)->y <= (PRECT_INSIDE)->y && \
    (PRECT_OUTSIDE)->x+(int)(PRECT_OUTSIDE)->width >= (PRECT_INSIDE)->x+(int)(PRECT_INSIDE)->width && \
    (PRECT_OUTSIDE)->y+(int)(PRECT_OUTSIDE)->height >= (PRECT_INSIDE)->y+(int)(PRECT_INSIDE)->height)

#define BWIN_RECT_ISINTERSECTING(PRECT1, PRECT2) (\
    (PRECT1)->x < (PRECT2)->x+(int)(PRECT2)->width && \
    (PRECT1)->y < (PRECT2)->y+(int)(PRECT2)->height && \
    (PRECT1)->x+(int)(PRECT1)->width > (PRECT2)->x && \
    (PRECT1)->y+(int)(PRECT1)->height > (PRECT2)->y)

/**
Summary:
Assign the members of the rectangle.
**/
#define BWIN_SET_RECT(PRECT,X,Y,W,H) \
    do {(PRECT)->x = X; (PRECT)->y = Y; (PRECT)->width = W; (PRECT)->height = H;} while(0)

/**
Summary:
Test is a point is located inside a rectangle.
**/
#define BWIN_POINT_IN_RECT(X,Y,PRECT) (\
    (X)>=(PRECT)->x && \
    (Y)>=(PRECT)->y && \
    (X)<(PRECT)->x + (int)(PRECT)->width && \
    (Y)<=(PRECT)->y + (int)(PRECT)->height)

/**
Summary:
Find the intersection of two rectangles.
Description:
If the rectangles do not intersect, dest will be all 0's.

Beware when calculating the intersection of a rectangle and a line (a line has either
width or height of zero, but not both). This algorithm will not elimnate the rightmost
or bottommost line. See the implementatin of bwin_draw_line for an example of how
to deal with this.
**/
void
bwin_intersect_rect(bwin_rect *dest, const bwin_rect *s1, const bwin_rect *s2);

/**
Summary:
Find the union of two rectangles.
**/
void
bwin_union_rect(bwin_rect *dest, const bwin_rect *s1, const bwin_rect *s2);

#ifdef __cplusplus
}
#endif

#endif /* BWIN_RECT_H__ */
