/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "nxapps_cmdline.h"
#include "bstd.h"
#include "namevalue.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

BDBG_MODULE(nxapps_cmdline);

void nxapps_cmdline_init(struct nxapps_cmdline *cmdline)
{
    memset(cmdline, 0, sizeof(*cmdline));
#if NEXUS_HAS_FRONTEND
    get_default_tune_settings(&cmdline->frontend.tune);
#endif
}

static const struct nxapps_cmdline_type_base *get_base(const struct nxapps_cmdline *cmdline, enum nxapps_cmdline_type st)
{
    switch (st) {
    case nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings:
        return &cmdline->pq.base;
    case nxapps_cmdline_type_SurfaceComposition:
        return &cmdline->comp.base;
    case nxapps_cmdline_type_frontend:
        return &cmdline->frontend.base;
    default:
        return NULL;
    }
}

void nxapps_cmdline_allow(struct nxapps_cmdline *cmdline, enum nxapps_cmdline_type st)
{
    struct nxapps_cmdline_type_base *base = (struct nxapps_cmdline_type_base *)get_base(cmdline, st);
    if (base) base->allowed = true;
}

bool nxapps_cmdline_is_set(const struct nxapps_cmdline *cmdline, enum nxapps_cmdline_type st)
{
    const struct nxapps_cmdline_type_base *base = get_base(cmdline, st);
    return base?base->set:false;
}

bool parse_boolean(const char *s)
{
    if (*s) {
        if (*s == '1' || *s == 'y' || *s == 'Y' || *s == 'T' || *s == 't') return true;
        if (!strcasecmp(s, "on")) return true;
    }
    return false;
}

#if NEXUS_HAS_SIMPLE_DECODER
#define SETTOP_ANR_DNR_OFF_LEVEL -99
#define SETTOP_MAX_PQ_LEVELS 6
/* DNR levels are 0 = off and 1-5 */
static int32_t mnr_level[SETTOP_MAX_PQ_LEVELS] = {SETTOP_ANR_DNR_OFF_LEVEL,-75,-33,0,100,200};
static int32_t bnr_level[SETTOP_MAX_PQ_LEVELS] = {SETTOP_ANR_DNR_OFF_LEVEL,-75,-33,0,100,500};
static int32_t dcr_level[SETTOP_MAX_PQ_LEVELS] = {SETTOP_ANR_DNR_OFF_LEVEL,-90,-50,0,50,90};
/* ANR levels are 0 = off and 1-6 */
#define SETTOP_MAX_ANR_LEVELS 7
static int32_t anr_level[SETTOP_MAX_ANR_LEVELS] = {SETTOP_ANR_DNR_OFF_LEVEL,-1,0,50,100,150,200};
#endif

void nxapps_cmdline_print_usage(const struct nxapps_cmdline *cmdline)
{
    if (!cmdline) return;

    if (cmdline->comp.base.allowed) {
        printf(
        "  -zorder #\n"
        "  -rect x,y,w,h       position client in default 1920x1080 coordinates\n"
        "  -vrect w,h:x,y,w,h  position client in virtual coordinates\n"
        "  -alpha {0..0xFF}    graphics alpha\n"
        "  -visible on|off\n"
        );
    }
    if (cmdline->pq.base.allowed) {
#if NEXUS_HAS_SIMPLE_DECODER
        printf(
        "  -dnr MNR,BNR,DCR    video DNR. values range: 0 = off, 1..%u\n"
        "  -anr ANR            video ANR. value range: 0 = off, 1..%u\n"
        "  -sharp SHARPNESS    video sharpness. value range: 0 = off, 1..%u\n",
        SETTOP_MAX_PQ_LEVELS-1,SETTOP_MAX_ANR_LEVELS-1,SETTOP_MAX_PQ_LEVELS-1
        );
#endif
        printf(
        "  -mad on|off         video MAD (deinterlacer)\n"
        "  -mad22 on|off       video MAD 22-pulldown. defaults off. requires \"-mad on\" to be set first.\n"
        "  -mad32 on|off       video MAD 32-pulldown. defaults on. requires \"-mad on\" to be set first.\n"
        "  -dejagging on|off   video scaler dejagging\n"
        "  -deringing on|off   video scaler deringing\n"
        "  -color CONTRAST,SATURATION,HUE,BRIGHTNESS\n"
        "                       video color. values range between -32768 and 32767.\n"
        );
    }
    if (cmdline->frontend.base.allowed) {
        printf(
        "  -streamer #              if no -freq, default is streamer 0\n"
        "  -freq #[,#,#]            frequency list in MHz or Hz\n"
        );
#if NEXUS_HAS_FRONTEND
        printf(
        "  -qam [64|256]            If HW supports auto, # is not required\n"
        );
        print_list_option("ofdm",g_ofdmModeStrs);
        print_list_option("sat",g_satModeStrs);
#endif
        printf(
        "  -sym X                   Symbol rate in baud\n"
        );
    }
}

int nxapps_cmdline_parse(int curarg, int argc, const char **argv, struct nxapps_cmdline *cmdline)
{
    int org_curarg = curarg;

    /* SimpleVideoDecoderPictureQualitySettings */
    if (cmdline->pq.base.allowed) {
        bool hit = true;
        if (!strcmp(argv[curarg], "-color") && argc>curarg+1) {
            if (sscanf(argv[++curarg], "%d,%d,%d,%d", &cmdline->pq.color.contrast,&cmdline->pq.color.saturation,&cmdline->pq.color.hue,&cmdline->pq.color.brightness) == 4) {
                cmdline->pq.color.set = true;
            }
        }
        else if (!strcmp(argv[curarg], "-sharp") && argc>curarg+1) {
            if (sscanf(argv[++curarg], "%d", &cmdline->pq.sharp.value) == 1) {
                cmdline->pq.sharp.set = true;
            }
        }
        else if (!strcmp(argv[curarg], "-dnr") && argc>curarg+1) {
            if (sscanf(argv[++curarg], "%d,%d,%d", &cmdline->pq.dnr.mnr,&cmdline->pq.dnr.bnr,&cmdline->pq.dnr.dcr) == 3) {
                cmdline->pq.dnr.set = true;
            }
        }
        else if (!strcmp(argv[curarg], "-anr") && argc>curarg+1) {
            if (sscanf(argv[++curarg], "%d", &cmdline->pq.anr.value) == 1) {
                cmdline->pq.anr.set = true;
            }
        }
        else if (!strcmp(argv[curarg], "-mad") && argc>curarg+1) {
            cmdline->pq.mad.enabled = parse_boolean(argv[++curarg]);
            cmdline->pq.mad.set = true;
        }
        else if (!strcmp(argv[curarg], "-mad22") && argc>curarg+1) {
            if (!cmdline->pq.mad.set || !cmdline->pq.mad.enabled) {
                BDBG_ERR(("must set -mad on to set -mad22"));
                return -1;
            }
            cmdline->pq.mad.mad22 = parse_boolean(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-mad32") && argc>curarg+1) {
            if (!cmdline->pq.mad.set || !cmdline->pq.mad.enabled) {
                BDBG_ERR(("must set -mad on to set -mad32"));
                return -1;
            }
            cmdline->pq.mad.mad32 = parse_boolean(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-dejagging") && argc>curarg+1) {
            cmdline->pq.dejagging.value = parse_boolean(argv[++curarg]);
            cmdline->pq.dejagging.set = true;
        }
        else if (!strcmp(argv[curarg], "-deringing") && argc>curarg+1) {
            cmdline->pq.deringing.value = parse_boolean(argv[++curarg]);
            cmdline->pq.deringing.set = true;
        }
        else {
            hit = false;
        }
        if (hit) {
            cmdline->pq.base.set = true;
            goto done;
        }
    }

    if (cmdline->comp.base.allowed) {
        bool hit = true;
        if (!strcmp(argv[curarg], "-zorder") && argc>curarg+1) {
            cmdline->comp.zorder.value = atoi(argv[++curarg]);
            cmdline->comp.zorder.set = true;
        }
        else if (!strcmp(argv[curarg], "-alpha") && argc>curarg+1) {
            cmdline->comp.alpha.value = strtoul(argv[++curarg], NULL, 0) << 24;
            cmdline->comp.alpha.set = true;
        }
        else if (!strcmp(argv[curarg], "-rect") && argc>curarg+1) {
            unsigned x, y, width, height;
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &x,&y,&width,&height) == 4) {
                cmdline->comp.rect.position.x = x;
                cmdline->comp.rect.position.y = y;
                cmdline->comp.rect.position.width = width;
                cmdline->comp.rect.position.height = height;
                cmdline->comp.rect.virtualDisplay.width = 1920;
                cmdline->comp.rect.virtualDisplay.height = 1080;
                cmdline->comp.rect.set = true;
            }
        }
        else if (!strcmp(argv[curarg], "-vrect") && argc>curarg+1) {
            unsigned x, y, width, height, vw, vh;
            if (sscanf(argv[++curarg], "%u,%u:%u,%u,%u,%u", &vw,&vh,&x,&y,&width,&height) == 6) {
                cmdline->comp.rect.position.x = x;
                cmdline->comp.rect.position.y = y;
                cmdline->comp.rect.position.width = width;
                cmdline->comp.rect.position.height = height;
                cmdline->comp.rect.virtualDisplay.width = vw;
                cmdline->comp.rect.virtualDisplay.height = vh;
                cmdline->comp.rect.set = true;
            }
        }
        else if (!strcmp(argv[curarg], "-visible") && argc>curarg+1) {
            cmdline->comp.visible.value = parse_boolean(argv[++curarg]);
            cmdline->comp.visible.set = true;
        }
        else {
            hit = false;
        }
        if (hit) {
            cmdline->comp.base.set = true;
            goto done;
        }
    }

    if (cmdline->frontend.base.allowed) {
        bool hit = true;
        if (!strcmp(argv[curarg], "-streamer") && argc>curarg+1) {
            cmdline->frontend.tune.source = channel_source_streamer;
            cmdline->frontend.tune.freq = atoi(argv[++curarg]);
        }
#if NEXUS_HAS_FRONTEND
        else if (!strcmp(argv[curarg], "-qam")) {
            cmdline->frontend.tune.source = channel_source_qam;
            cmdline->frontend.tune.mode = 0;
            if (argc>curarg+1) {
                unsigned mode = atoi(argv[curarg+1]);
                if (mode == 64 || mode == 256) {
                    cmdline->frontend.tune.mode = mode;
                    curarg++;
                }
            }
        }
        else if (!strcmp(argv[curarg], "-ofdm")) {
            cmdline->frontend.tune.source = channel_source_ofdm;
            if (curarg+1<argc && argv[curarg+1][0]!='-') {
                ++curarg;
                if (argv[curarg][0] >= '0' && argv[curarg][0] <= '3') {
                    /* for compat with nexus/examples enum value syntax */
                    cmdline->frontend.tune.mode = atoi(argv[curarg]);
                }
                else {
                    cmdline->frontend.tune.mode = lookup(g_ofdmModeStrs, argv[curarg]);
                }
            }
            else {
                cmdline->frontend.tune.mode = NEXUS_FrontendOfdmMode_eDvbt2;
            }
        }
        else if (!strcmp(argv[curarg], "-sat")) {
            cmdline->frontend.tune.source = channel_source_sat;
            if (curarg+1<argc && argv[curarg+1][0]!='-') {
                ++curarg;
                if (argv[curarg][0] >= '0' && argv[curarg][0] <= '9' && argv[curarg][1] == 0) {
                    /* for compat with nexus/examples enum value syntax */
                    cmdline->frontend.tune.mode = atoi(argv[curarg]);
                }
                else {
                    cmdline->frontend.tune.mode = lookup(g_satModeStrs, argv[curarg]);
                }
            }
            else {
                cmdline->frontend.tune.mode = NEXUS_FrontendSatelliteMode_eDvb;
            }
        }
        else if (!strcmp(argv[curarg], "-sym") && curarg+1<argc) {
            cmdline->frontend.tune.symbolRate = atoi(argv[++curarg]);;
        }
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            cmdline->frontend.freq_list = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-adc") && curarg+1<argc) {
            cmdline->frontend.tune.adc = atoi(argv[++curarg]);;
        }
        else if (!strcmp(argv[curarg], "-voltage") && curarg+1<argc) {
            cmdline->frontend.tune.diseqcVoltage = lookup(g_diseqcVoltageStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-tone") && curarg+1<argc) {
            cmdline->frontend.tune.toneEnabled = lookup(g_diseqcToneEnabledStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ksyms") && curarg+1<argc) {
            cmdline->frontend.tune.symbolRate = strtoul(argv[++curarg], NULL, 0) * 1000;
        }
        else if (!strcmp(argv[curarg], "-networkspec") && curarg+1<argc) {
            cmdline->frontend.tune.satNetworkSpec = lookup(g_satNetworkSpecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-kufreq") && curarg+1<argc) {
            char buf[10];
            unsigned freq = strtoul(argv[++curarg], NULL, 0);
            if (freq < 11700) {
                /* low band */
                freq -= 9750;
                cmdline->frontend.tune.toneEnabled = 0;
            }
            else {
                /* high band */
                freq -= 10600;
                cmdline->frontend.tune.toneEnabled = 1;
            }
            snprintf(buf, sizeof(buf), "%d", freq);
            cmdline->frontend.freq_list = strdup(buf);
        }
#endif
        else {
            hit = false;
        }
        if (hit) {
            cmdline->frontend.base.set = true;
            goto done;
        }
    }
done:
    return curarg - org_curarg;
}

#if NEXUS_HAS_SIMPLE_DECODER
void nxapps_cmdline_apply_SimpleVideoDecodePictureQualitySettings(const struct nxapps_cmdline *cmdline, NEXUS_SimpleVideoDecoderPictureQualitySettings *pSettings)
{
    if (cmdline->pq.color.set) {
        pSettings->common.contrast = cmdline->pq.color.contrast;
        pSettings->common.saturation = cmdline->pq.color.saturation;
        pSettings->common.hue = cmdline->pq.color.hue;
        pSettings->common.brightness = cmdline->pq.color.brightness;
    }
    if (cmdline->pq.sharp.set) {
        if ((cmdline->pq.sharp.value > 0) && (cmdline->pq.sharp.value < SETTOP_MAX_PQ_LEVELS)) {
            pSettings->common.sharpnessEnable = true;
            pSettings->common.sharpness = -32768 + (cmdline->pq.sharp.value * (65536 / 5));
        } else {
            pSettings->common.sharpnessEnable = false;
            pSettings->common.sharpness = 0;
        }
    }
    if (cmdline->pq.dnr.set) {
        if ((cmdline->pq.dnr.mnr > 0) && (cmdline->pq.dnr.mnr < SETTOP_MAX_PQ_LEVELS)) {
            pSettings->dnr.mnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
            pSettings->dnr.mnr.level = mnr_level[cmdline->pq.dnr.mnr];
        } else {
            pSettings->dnr.mnr.mode = NEXUS_VideoWindowFilterMode_eDisable;
            pSettings->dnr.mnr.level = mnr_level[0];
        }

        if ((cmdline->pq.dnr.bnr > 0) && (cmdline->pq.dnr.bnr < SETTOP_MAX_PQ_LEVELS)) {
            pSettings->dnr.bnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
            pSettings->dnr.bnr.level = bnr_level[cmdline->pq.dnr.bnr];
        } else {
            pSettings->dnr.bnr.mode = NEXUS_VideoWindowFilterMode_eDisable;
            pSettings->dnr.bnr.level = bnr_level[0];
        }

        if ((cmdline->pq.dnr.dcr > 0) && (cmdline->pq.dnr.dcr < SETTOP_MAX_PQ_LEVELS)) {
            pSettings->dnr.dcr.mode = NEXUS_VideoWindowFilterMode_eEnable;
            pSettings->dnr.dcr.level = dcr_level[cmdline->pq.dnr.dcr];
        } else {
            pSettings->dnr.dcr.mode = NEXUS_VideoWindowFilterMode_eDisable;
            pSettings->dnr.dcr.level = dcr_level[0];
        }
    }
    if (cmdline->pq.anr.set) {
        if ((cmdline->pq.anr.value > 0) && (cmdline->pq.anr.value < SETTOP_MAX_ANR_LEVELS)) {
            pSettings->anr.anr.mode = NEXUS_VideoWindowFilterMode_eEnable;
            pSettings->anr.anr.level = anr_level[cmdline->pq.anr.value];
        } else {
            pSettings->anr.anr.mode = NEXUS_VideoWindowFilterMode_eDisable;
            pSettings->anr.anr.level = anr_level[0];
        }
    }
    if (cmdline->pq.mad.set) {
        pSettings->mad.deinterlace = cmdline->pq.mad.enabled;
        pSettings->mad.enable22Pulldown = cmdline->pq.mad.mad22;
        pSettings->mad.enable32Pulldown = cmdline->pq.mad.mad32;
    }
    if (cmdline->pq.dejagging.set) {
        pSettings->scaler.verticalDejagging = cmdline->pq.dejagging.value;
    }
    if (cmdline->pq.deringing.set) {
        pSettings->scaler.horizontalLumaDeringing =
        pSettings->scaler.verticalLumaDeringing =
        pSettings->scaler.horizontalChromaDeringing =
        pSettings->scaler.verticalChromaDeringing = cmdline->pq.deringing.value;
    }
}
#endif

#if NEXUS_HAS_SURFACE_COMPOSITOR
void nxapps_cmdline_apply_SurfaceComposition(const struct nxapps_cmdline *cmdline, NEXUS_SurfaceComposition *pComp)
{
    if (cmdline->comp.visible.set) {
        pComp->visible = cmdline->comp.visible.value;
    }
    if (cmdline->comp.zorder.set) {
        pComp->zorder = cmdline->comp.zorder.value;
    }
    if (cmdline->comp.rect.set) {
        pComp->position = cmdline->comp.rect.position;
        pComp->virtualDisplay = cmdline->comp.rect.virtualDisplay;
    }
    if (cmdline->comp.alpha.set) {
        /* NOTE: There are many ways alpha can be used. This uses the given alpha value for both the color and alpha planes.
        Your app may want to do something different. */
        static const NEXUS_BlendEquation g_colorBlend = {NEXUS_BlendFactor_eConstantAlpha, NEXUS_BlendFactor_eSourceColor, false,
            NEXUS_BlendFactor_eInverseConstantAlpha, NEXUS_BlendFactor_eDestinationColor, false, NEXUS_BlendFactor_eZero};
        static const NEXUS_BlendEquation g_alphaBlend = {NEXUS_BlendFactor_eConstantAlpha, NEXUS_BlendFactor_eSourceAlpha, false,
            NEXUS_BlendFactor_eInverseConstantAlpha, NEXUS_BlendFactor_eDestinationAlpha, false, NEXUS_BlendFactor_eZero};
        pComp->colorBlend = g_colorBlend;
        pComp->alphaBlend = g_alphaBlend;
        pComp->constantColor = cmdline->comp.alpha.value;
    }
}
#endif
