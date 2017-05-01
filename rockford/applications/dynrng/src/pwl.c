/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/
#include "nxclient.h"
#include "util_priv.h"
#include "pwl.h"
#include "pwl_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void pwl_destroy_curve(PwlCurveHandle curve)
{
    if (!curve) return;
    if (curve->points) free(curve->points);
    if (curve->name) free(curve->name);
    free(curve);
}

void pwl_clear_curve(PwlCurveHandle curve)
{
    PwlPoint * point;
    unsigned i;

    assert(curve);

    for (i = 0; i < curve->count; i++)
    {
        point = &curve->points[i];
        point->x = 1.0;
        point->y = 1.0;
        point->slope_man = 0.5;
        point->slope_exp = 1;
    }
    /* set up 1 unity gain region */
    curve->points[0].x = 0.0;
    curve->points[0].y = 0.0;
    curve->points[0].slope_man = 0.5;
    curve->points[0].slope_exp = 1;
}

PwlCurveHandle pwl_create_curve(const char * name, unsigned maxNumPoints, PwlPointMutator set_point, PwlPointAccessor get_point, PwlLraMutator set_lra, PwlLraAccessor get_lra)
{
    PwlCurveHandle curve;

    curve = malloc(sizeof(*curve));
    assert(curve);
    memset(curve, 0, sizeof(*curve));
    if (!name) name = UTIL_STR_NONE;
    curve->name = malloc(strlen(name) + 1);
    assert(curve->name);
    strcpy(curve->name, name);
    curve->points = malloc(maxNumPoints * sizeof(PwlPoint));
    assert(curve->points);
    memset(curve->points, 0, maxNumPoints * sizeof(PwlPoint));
    curve->count = maxNumPoints;
    curve->set_point = set_point;
    curve->get_point = get_point;
    pwl_clear_curve(curve);

    curve->set_lra = set_lra;
    curve->get_lra = get_lra;

    return curve;
}

void pwl_p_read_curve(PwlCurveHandle curve, const char * pwlFilename)
{
    FILE * pwlFile;
    static char line[LINE_LEN];
    char * p;
    PwlPoint * point = NULL;
    unsigned i;
    double x;
    double y;

    assert(curve);
    assert(pwlFilename);

    pwlFile = fopen(pwlFilename, "r");
    if (!pwlFile)
    {
        printf("Unable to open '%s'; using unit gain PWL\n", pwlFilename);
        pwl_clear_curve(curve);
        return;
    }

    i = 0;
    while (!feof(pwlFile))
    {
        memset(line, 0, LINE_LEN);
        if (!fgets(line, LINE_LEN, pwlFile)) break;

        /* get rid of newline */
        p = strchr(line, '\n');
        if (p) *p = 0;
        /* get rid of comments */
        p = strchr(line, '#');
        if (p) *p = 0;
        if (sscanf(line, "%lf,%lf", &x, &y) == 2)
        {
            if (i < curve->count)
            {
                point = &curve->points[i++];
                point->x = x < 0.0 ? 0.0 : x;
                point->y = y < 0.0 ? 0.0 : y;
            }
            else
            {
                printf("Only %u points supported. (%f,%f) ignored\n", curve->count, x, y);
            }
        }
    }
#ifdef DEBUG
    printf("%s: read file '%s'\n", curve->name, pwlFilename);
#endif
}

void pwl_p_compute_slope(double * slope_man, int * exp)
{
    assert(slope_man);
    assert(exp);

    *exp = 0;
    if (*slope_man == 0.0) return;
    if (*slope_man >= 1.0)
    {
        while (*slope_man >= 1.0)
        {
            *slope_man /= 2.0;
            ++*exp;
        }
    }
    else
    {
        while (*slope_man * 2.0 < 1.0)
        {
            *slope_man *= 2.0;
            --*exp;
        }
    }
}

void pwl_p_compute_curve_slopes(PwlCurveHandle curve)
{
    PwlPoint * p1;
    PwlPoint * p2;
    unsigned i;

    assert(curve);

    p1 = &curve->points[0];
    for (i = 1; i < curve->count; i++)
    {
        p2 = &curve->points[i];
        if (p2->x == 1.0 && p1->x == 1.0) break;
        if (p2->x - p1->x > 0.0)
        {
            p1->slope_man = (p2->y - p1->y) / (p2->x - p1->x);
            pwl_p_compute_slope(&p1->slope_man, &p1->slope_exp);
        }
        else if (p2->y - p1->y == 0.0)
        {
            p1->slope_man = 0.0;
            p1->slope_exp = 0;
        }
        else
        {
            printf("Divide by zero with point (%0.06f, %0.06f) and (%0.06f, %0.06f)\n", p1->x, p1->y, p2->x, p2->y);
        }
        p1 = p2;
    }
}

void pwl_print_curve(const PwlCurveHandle curve)
{
    const PwlPoint * p;
    unsigned i;

    assert(curve);

    printf("%s {\n", curve->name);
    for (i = 0; i < curve->count; i++)
    {
        p = &curve->points[i];
        printf("  %0.06f, %0.06f, %0.010f, %d\n", p->x, p->y, p->slope_man, p->slope_exp);
    }
    printf("}\n");
}

void pwl_load_curve(PwlCurveHandle curve, const char * pwlFilename)
{
    pwl_clear_curve(curve);
    pwl_p_read_curve(curve, pwlFilename);
    pwl_p_compute_curve_slopes(curve);
}

void pwl_get_curve_hw(PwlCurveHandle curve, unsigned inputIndex, unsigned rectIndex)
{
    PwlPoint * pPoint;
    unsigned i;

    assert(curve);
    if (!curve->get_point) { printf("%s: no point getter\n", curve->name); return; }

    for (i = 0; i < curve->count; i++)
    {
        pPoint = &curve->points[i];
        curve->get_point(inputIndex, rectIndex, i, &pPoint->slope_man, &pPoint->slope_exp, &pPoint->x, &pPoint->y);
    }
}

void pwl_set_curve_hw(PwlCurveHandle curve, unsigned inputIndex, unsigned rectIndex)
{
    PwlPoint * pPoint;
    unsigned i;

    assert(curve);
    if (!curve->set_point) { printf("%s: no point setter\n", curve->name); return; }

    for (i = 0; i < curve->count; i++)
    {
        pPoint = &curve->points[i];
        curve->set_point(inputIndex, rectIndex, i, pPoint->slope_man, pPoint->slope_exp, pPoint->x, pPoint->y);
    }
}

void pwl_get_lra_hw(PwlCurveHandle curve, unsigned inputIndex, unsigned rectIndex, bool *enabled)
{
    assert(curve);
    if (!curve->get_lra) { printf("%s: no lra getter\n", curve->name); return; }
    curve->get_lra(inputIndex, rectIndex, enabled);
}
void pwl_set_lra_hw(PwlCurveHandle curve, unsigned inputIndex, unsigned rectIndex, bool enabled)
{
    assert(curve);
    if (!curve->set_lra) { printf("%s: no lra setter\n", curve->name); return; }
    curve->set_lra(inputIndex, rectIndex, enabled);
}
