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
#include "nl2l.h"
#include "nl2l_priv.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* HDR_CMP_0_V1_R00_TO_R15_NL_CONFIG.RECT0_SEL_NL2L */
typedef enum {
    selNl2l_e709 = 0,
    selNl2l_e1886,
    selNl2l_ePq,
    selNl2l_eBbc,
    selNl2l_eRam,
    selNl2l_eBypass,
    selNl2l_eMax
} selNl2l;

BDBG_MODULE(nl2l);

void nl2l_p_read_lut(Nl2lHandle nl2l, const char * lutFilename)
{
    FILE * lutFile;
    static char line[LINE_LEN];
    char * p;
    unsigned i, val, count = 0;
    bool foundLutSel = false;

    assert(lutFilename);
    nl2l->lutSel = selNl2l_ePq;

    lutFile = fopen(lutFilename, "r");
    if (!lutFile)
    {
        printf("Unable to open '%s'\n", lutFilename);
        return;
    }

    memset(nl2l->lutData, 0, sizeof(nl2l->lutData));

    i = 0;
    while (!feof(lutFile))
    {
        memset(line, 0, LINE_LEN);
        if (!fgets(line, LINE_LEN, lutFile)) break;

        /* get rid of newline */
        p = strchr(line, '\n');
        if (p) *p = 0;
        /* get rid of comments */
        p = strchr(line, '#');
        if (p) *p = 0;
        p = strchr(line, '/');
        if (p) *p = 0;

        if (!foundLutSel)
        {
            char * name;
            char * value;
            int v;

            p = strchr(line, '=');
            if (!p)
            {
                foundLutSel = true;
                nl2l->lutSel = selNl2l_eRam;
                goto lut;
            }

            name = line;
            value = p + 1;
            *p = 0;

            name = trim(name);
            value = trim(value);

            if (strlen(value) && strlen(name))
            {
                if (!strcmp(name, "sel"))
                {
                    if (sscanf(value, "%d", &v))
                    {
                        if (v <= selNl2l_eMax)
                        {
                            nl2l->lutSel = v;
                            foundLutSel = true;
                        }
                    }
                }
            }

            if (foundLutSel) continue;
            else
            {
                printf("Unable to process '%s'. Expected first line with \"sel=\" to select conversion mode\n", lutFilename);
                return;
            }
        }

lut:
        if (sscanf(line, "%x", &val) == 1)
        {
            if (i < LUT_SIZE)
            {
                nl2l->lutData[i++] = val;
            }
            else
            {
                printf("Only %u entries supported. (%xf) ignored\n", LUT_SIZE, val);
            }
        }
    }

    count = i;
    if ((nl2l->lutSel == selNl2l_eRam) && count < LUT_SIZE)
    {
        printf("Only %u entries found (expected %u)\n", count, LUT_SIZE);
    }

    nl2l->lutDataSize = count;
    fclose(lutFile);
}

static void time_diff(struct timeval *diff, const struct timeval *now, const struct timeval *then)
{
    diff->tv_sec = now->tv_sec - then->tv_sec;
    diff->tv_usec = now->tv_usec - then->tv_usec;
    if(diff->tv_usec < 0) {
        diff->tv_sec --;
        diff->tv_usec += 1000000;
    }
    return;
}

static void nl2l_p_update_write(Nl2lHandle nl2l)
{
    printf("Nl2l update (lutSel %u, size %u)\n", nl2l->lutSel, nl2l->lutDataSize);
    platform_display_set_nl2l_source(nl2l->lutSel);
    if (nl2l->lutSel == selNl2l_eRam)
    {
        platform_display_load_nl2l_lut(nl2l->lutDataSize, nl2l->lutData);
    }
}

void nl2l_p_scheduler_callback(void * pContext, int param)
{
    Nl2lHandle nl2l = pContext;
    #define MIN_WAIT 50

    if (nl2l->updater.scheduled)
    {
        struct timeval now, diff;
        unsigned diff_ms;
        gettimeofday(&now, NULL);
        time_diff(&diff, &now, &nl2l->updater.time);
        diff_ms = diff.tv_sec * 1000 + diff.tv_usec/1000;
        if (diff_ms > MIN_WAIT)
        {
            nl2l_p_update_write(nl2l);
            nl2l->updater.scheduled = false;
        }
    }
}

Nl2lHandle nl2l_create(PlatformHandle platform, const char * name, const char * hlg2hdrRoot)
{
    Nl2lHandle nl2l;

    assert(platform);

    nl2l = malloc(sizeof(*nl2l));
    assert(nl2l);
    memset(nl2l, 0, sizeof(*nl2l));

    nl2l->platform = platform;
    if (!name) name = UTIL_STR_NONE;
    nl2l->name = malloc(strlen(name) + 1);
    assert(nl2l->name);
    strcpy(nl2l->name, name);

    if (hlg2hdrRoot)
    {
        nl2l->switcher = file_switcher_create(name, hlg2hdrRoot, NULL, true);
        assert(nl2l->switcher);
    }

    nl2l->delayedUpdater = platform_scheduler_add_listener(platform_get_scheduler(platform), &nl2l_p_scheduler_callback, nl2l);
    if (!nl2l->delayedUpdater)
    {
        BDBG_WRN(("Unable to set NL2L values"));
    }

    return nl2l;
}

void nl2l_destroy(Nl2lHandle nl2l)
{
    if (!nl2l) return;
    if (nl2l->name) free(nl2l->name);
    if (nl2l->delayedUpdater)
    {
        platform_scheduler_remove_listener(platform_get_scheduler(nl2l->platform), nl2l->delayedUpdater);
    }
    free(nl2l);
}

void nl2l_update(Nl2lHandle nl2l, int nl2lSetting)
{
    assert(nl2l);

    if (nl2lSetting >= (int)file_switcher_get_count(nl2l->switcher))
    {
        printf("NL2L: file position out of bounds\n");
        return;
    }

    file_switcher_set_position(nl2l->switcher, nl2lSetting);
    nl2l_p_read_lut(nl2l, file_switcher_get_path(nl2l->switcher));

    if (nl2l->delayedUpdater)
    {
        nl2l->updater.scheduled = true;
        gettimeofday(&nl2l->updater.time, NULL);
    }
    else
    {
        nl2l_p_update_write(nl2l);
    }
}
