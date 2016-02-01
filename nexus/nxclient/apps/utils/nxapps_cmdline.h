/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
#ifndef NXAPPS_CMDLINE_H__
#define NXAPPS_CMDLINE_H__

#include "nxclient.h"
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_simple_video_decoder.h"
#endif
#if NEXUS_HAS_SURFACE_COMPOSITOR
#include "nexus_surface_compositor_types.h"
#endif
#include "live_source.h"

#ifdef __cplusplus
extern "C" {
#endif

enum nxapps_cmdline_type
{
    nxapps_cmdline_type_SurfaceComposition,
    nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings,
    nxapps_cmdline_type_frontend,
    nxapps_cmdline_type_max
};

struct nxapps_cmdline_type_base
{
    bool allowed, set;
};

struct nxapps_cmdline
{
    struct {
        struct nxapps_cmdline_type_base base; /* internal base class */
        struct {
            bool set;
            int value;
        } zorder, alpha, visible;
        struct {
            bool set;
            NEXUS_Rect position;
            NEXUS_SurfaceRegion virtualDisplay;
        } rect;
    } comp; /* nxapps_cmdline_type_SurfaceComposition */
    struct {
        struct nxapps_cmdline_type_base base; /* internal base class */
        struct {
            bool set;
            int contrast, saturation, hue, brightness;
        } color;
        struct {
            bool set;
            int mnr, bnr, dcr;
        } dnr;
        struct {
            bool set;
            int value;
        } anr, sharp, dejagging, deringing;
        struct {
            bool set;
            bool enabled, mad22, mad32;
        } mad;
    } pq; /* nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings */
    struct {
        struct nxapps_cmdline_type_base base; /* internal base class */
        struct btune_settings tune;
        const char *freq_list;
    } frontend;
};

void nxapps_cmdline_init(struct nxapps_cmdline *cmdline);
void nxapps_cmdline_allow(struct nxapps_cmdline *cmdline, enum nxapps_cmdline_type st);
void nxapps_cmdline_print_usage(const struct nxapps_cmdline *cmdline);

int nxapps_cmdline_parse(int curarg, int argc, const char **argv, struct nxapps_cmdline *cmdline);
bool nxapps_cmdline_is_set(const struct nxapps_cmdline *cmdline, enum nxapps_cmdline_type st);

#if NEXUS_HAS_SIMPLE_DECODER
void nxapps_cmdline_apply_SimpleVideoDecodePictureQualitySettings(const struct nxapps_cmdline *cmdline, NEXUS_SimpleVideoDecoderPictureQualitySettings *pSettings);
#endif
#if NEXUS_HAS_SURFACE_COMPOSITOR
void nxapps_cmdline_apply_SurfaceComposition(const struct nxapps_cmdline *cmdline, NEXUS_SurfaceComposition *pComp);
#endif
bool parse_boolean(const char *s);

#ifdef __cplusplus
}
#endif

#endif
