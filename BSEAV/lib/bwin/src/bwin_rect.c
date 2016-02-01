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

#include "bwin.h"
#include "bwin_rect.h"
#include "bwin_priv.h"

void printrect(const char *name, const bwin_rect *r)
{
    printf("%s (%d,%d,%d,%d)\n", name, r->x,r->y,r->width,r->height);
}

/* This function allows s1 or s2 to point to the same memory as dest. In place union. */
void
bwin_union_rect(bwin_rect *dest, const bwin_rect *s1, const bwin_rect *s2)
{
    if (s1->width == 0 || s1->height == 0) {
        *dest = *s2;
    }
    else if (s2->width == 0 || s2->height == 0) {
        *dest = *s1;
    }
    else {
        int r,b,x,y;
        x = min(s1->x,s2->x);
        y = min(s1->y,s2->y);
        r = max(s1->x + s1->width, s2->x + s2->width);
        b = max(s1->y + s1->height, s2->y + s2->height);
        dest->x = x;
        dest->y = y;
        dest->width = r - x;
        dest->height = b - y;
    }
}

/* This function allows s1 or s2 to point to the same memory as dest. In place intersection. */
void
bwin_intersect_rect(bwin_rect *dest, const bwin_rect *s1, const bwin_rect *s2)
{
    int r,b,x,y;
    x = max(s1->x, s2->x);
    y = max(s1->y, s2->y);
    r = min(s1->x + (int)s1->width, s2->x + (int)s2->width);
    b = min(s1->y + (int)s1->height, s2->y + (int)s2->height);
    if (r >= x && b >= y) {
        dest->x = x;
        dest->y = y;
        dest->width = r - x;
        dest->height = b - y;
    }
    else {
        dest->x = 0;
        dest->y = 0;
        dest->width = 0;
        dest->height = 0;
    }
#if 0
    printrect("inter", dest);
    printrect("     ", s1);
    printrect("     ", s2);
#endif
}
