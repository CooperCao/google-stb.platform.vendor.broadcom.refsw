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
#include "plm.h"
#include "plm_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void plm_p_clear(PlmHandle plm)
{
    assert(plm);
    pwl_clear_curve(plm->curve);
#ifdef DEBUG
    pwl_curve_print(plm->curve);
#endif
    pwl_set_curve_hw(plm->curve, plm->index, plm->rect);
}

void plm_p_apply(PlmHandle plm)
{
    assert(plm);
    if (!plm->currentSwitcher) { printf("%s: current plm switcher is not set\n", plm->name); return; }
    file_switcher_print(plm->currentSwitcher);
    pwl_load_curve(plm->curve, file_switcher_get_path(plm->currentSwitcher));
#ifdef DEBUG
    pwl_curve_print(plm->curve);
#endif
    pwl_set_curve_hw(plm->curve, plm->index, plm->rect);

    if (!plm->nl2l) return;
    if (plm->currentSwitcher==plm->hlg2hdrSwitcher)
    {
        nl2l_update(plm->nl2l, file_switcher_get_position(plm->currentSwitcher));
    }
}

void plm_p_clear_lra(PlmHandle plm)
{
    pwl_set_lra_hw(plm->curve, plm->index, plm->rect, 0);
}

void plm_p_apply_lra(PlmHandle plm)
{
    pwl_set_lra_hw(plm->curve, plm->index, plm->rect, plm_get(plm) > 0);
}

void plm_p_apply_current(PlmHandle plm)
{
    assert(plm);
    if (plm->currentSwitcher)
    {
        if (file_switcher_get_position(plm->currentSwitcher) != plm->plmSetting)
        {
            file_switcher_set_position(plm->currentSwitcher, plm->plmSetting);
        }
        /*plm_p_apply(plm);*/
        plm->apply(plm);
    }
    else
    {
        /*plm_p_clear(plm);*/
        plm->clear(plm);
    }
}

void plm_p_apply_initial(PlmHandle plm)
{
    assert(plm);
    if (plm->currentSwitcher)
    {
        file_switcher_first(plm->currentSwitcher);
        /*plm_p_apply(plm);*/
        plm->apply(plm);
    }
    else
    {
        /*plm_p_clear(plm);*/
        plm->clear(plm);
    }
}

void plm_p_compute_switcher(PlmHandle plm, PlatformDynamicRange input, PlatformDynamicRange output)
{
    assert(plm);
    plm->currentSwitcher = NULL;

    if (output == PlatformDynamicRange_eSdr)
    {
        switch (input)
        {
            case PlatformDynamicRange_eDolbyVision:
            case PlatformDynamicRange_eTechnicolorPrime:
            case PlatformDynamicRange_eHdr10:
                plm->currentSwitcher = plm->hdr2sdrSwitcher;
                break;
            default:
                break;
        }
    }
    else if (output == PlatformDynamicRange_eHdr10)
    {
        switch (input)
        {
            case PlatformDynamicRange_eDolbyVision:
            case PlatformDynamicRange_eTechnicolorPrime:
            case PlatformDynamicRange_eSdr:
                plm->currentSwitcher = plm->sdr2hdrSwitcher;
                break;
            case PlatformDynamicRange_eHlg:
                plm->currentSwitcher = plm->hlg2hdrSwitcher;
                break;
            default:
                break;
        }
    }
    else if (output == PlatformDynamicRange_eDolbyVision)
    {
        /* just needs to be non-NULL */
        if (input != PlatformDynamicRange_eDolbyVision)
        {
            plm->currentSwitcher = plm->hdr2sdrSwitcher;
        }
    }
    /* HLG needs no switcher */
}

PlmHandle plm_create(
    const char * name,
    const char * sdr2hdrRoot,
    const char * hdr2sdrRoot,
    const char * hlg2hdrRoot,
    PlatformHandle platform,
    unsigned index,
    unsigned rect,
    PwlPointMutator set_point,
    PwlPointAccessor get_point,
    PwlLraMutator set_lra,
    PwlLraAccessor get_lra)
{
    PlmHandle plm;
    static const char nl2lSubdir[] = "/nl2l";
    char * nl2lRoot = NULL;

    plm = malloc(sizeof(*plm));
    assert(plm);
    memset(plm, 0, sizeof(*plm));
    if (!name) name = UTIL_STR_NONE;
    plm->name = set_string(plm->name, name);
    plm->index = index;
    plm->rect = rect;
    plm->currentSwitcher = NULL;

    if (sdr2hdrRoot)
    {
        plm->sdr2hdrSwitcher = file_switcher_create(name, sdr2hdrRoot, NULL, true);
        assert(plm->sdr2hdrSwitcher);
    }
    if (hdr2sdrRoot)
    {
        plm->hdr2sdrSwitcher = file_switcher_create(name, hdr2sdrRoot, NULL, true);
        assert(plm->hdr2sdrSwitcher);
    }
    if (hlg2hdrRoot)
    {
        plm->hlg2hdrSwitcher = file_switcher_create(name, hlg2hdrRoot, NULL, true);
        assert(plm->hlg2hdrSwitcher);
        nl2lRoot = malloc(strlen(hlg2hdrRoot) + strlen(nl2lSubdir) + 1);
        assert(nl2lRoot);
        sprintf(nl2lRoot, "%s%s", hlg2hdrRoot, nl2lSubdir);
        plm->nl2l = nl2l_create(platform, "NL2L", nl2lRoot);
        if (nl2lRoot) free(nl2lRoot);
    }
    plm->curve = pwl_create_curve(name, PWL_POINTS, set_point, get_point, set_lra, get_lra);
    assert(plm->curve);

    /*plm-apply = plm_p_apply;*/
    /*pml->clear  = plm_p_clear;*/
    plm->apply = plm_p_apply_lra;
    plm->clear = plm_p_clear_lra;

    return plm;
}

void plm_destroy(PlmHandle plm)
{
    if (!plm) return;
    pwl_destroy_curve(plm->curve);
    file_switcher_destroy(plm->hdr2sdrSwitcher);
    file_switcher_destroy(plm->sdr2hdrSwitcher);
    file_switcher_destroy(plm->hlg2hdrSwitcher);
    if (plm->name) free(plm->name);
    free(plm);
}

void plm_update_dynamic_range(PlmHandle plm, PlatformDynamicRange input, PlatformDynamicRange output)
{
    assert(plm);
    plm_p_compute_switcher(plm, input, output);
}

void plm_reapply(PlmHandle plm)
{
    plm_p_apply_current(plm);
}

void plm_next(PlmHandle plm)
{
    assert(plm);
    if (!plm->currentSwitcher) { printf("%s is already in the correct luma space; no conversion performed\n", plm->name); return; }
    file_switcher_next(plm->currentSwitcher);
    plm->plmSetting = file_switcher_get_position(plm->currentSwitcher);
    /*plm_p_apply(plm);*/
    plm->apply(plm);
}

void plm_prev(PlmHandle plm)
{
    assert(plm);
    if (!plm->currentSwitcher) { printf("%s is already in the correct luma space; no conversion performed\n", plm->name); return; }
    file_switcher_prev(plm->currentSwitcher);
    plm->plmSetting = file_switcher_get_position(plm->currentSwitcher);
    /*plm_p_apply(plm);*/
    plm->apply(plm);
}

void plm_set(PlmHandle plm, int plmSetting)
{
    assert(plm);
    plm->plmSetting = plmSetting;
    if (plmSetting == -1)
    {
        printf("Clear switcher %s\n", plm->name);
        plm->currentSwitcher = NULL;
        return;
    }
    if (!plm->currentSwitcher) { printf("%s switcher not ready; storing setting (%d) for later application\n", plm->name, plmSetting); return; }
    if (file_switcher_get_position(plm->currentSwitcher) == plmSetting) return;
    file_switcher_set_position(plm->currentSwitcher, plmSetting);
    /*plm_p_apply(plm);*/
    plm->apply(plm);
}

FileSwitcherHandle plm_get_current_switcher(PlmHandle plm)
{
    return plm->currentSwitcher;
}

int plm_get(PlmHandle plm)
{
    return plm->currentSwitcher ? file_switcher_get_position(plm->currentSwitcher) : -1;
}
