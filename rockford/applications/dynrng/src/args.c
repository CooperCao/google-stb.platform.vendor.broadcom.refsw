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
#include "args.h"
#include "args_priv.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void args_p_print_usage(ArgsHandle args)
{
    unsigned i;
    assert(args);
    printf("Usage: %s OPTIONS\n", args->name);
    printf("-im method  'console'|'remote'. Defaults to 'remote'\n");
    printf("-is path    path to directory containing SDR images to display\n");
    printf("-bs path    path to directory containing SDR background full-screen images to display\n");
    printf("-gs path    path to directory containing graphics sdr2hdr luma settings\n");
    printf("-vh path    path to directory containing video sdr2hdr luma settings\n");
    printf("-vs path    path to directory containing video hdr2sdr luma settings\n");
    printf("-vhh path   path to directory containing video hlg2hdr luma settings\n");
    printf("-sc path    path to directory containing scenario files\n");
    printf("-ss path    path to directory containing SDR streams\n");
    printf("-sh path    path to directory containing HDR streams\n");
    printf("-sg path    path to directory containing HLG streams\n");
    printf("-sm path    path to directory containing mixed SDR/HDR streams\n");
    printf("-cfg path   path to config root directory (contains all cfg files in defined hierarchy)\n");
    printf("-rc path    path to shared resource root directory (contains streams and images in defined hierarchy)\n");
    printf("-ct color   hex constant specifying text fg color incl alpha, e.g. 0xff000000 is black 100% opaque\n");
    printf("-cm color   hex constant specifying main panel bg color incl alpha\n");
    printf("-cg color   hex constant specifying instruction panel bg color incl alpha\n");
    printf("-ci color   hex constant specifying info panel bg color incl alpha\n");
    printf("-cd color   hex constant specifying details panel bg color incl alpha\n");
    printf("--osd wxh   w is int constant specifying width of osd, h is height of osd\n");
    printf("--demo      run in demo mode, where all settings are reset on each change\n");
}

static const struct
{
    const char * const gfxSdr2HdrPath;
    const char * const vidSdr2HdrPath;
    const char * const vidHdr2SdrPath;
    const char * const vidHlg2HdrPath;
    const char * const scenarioPath;
    const char * const sdrStreamPath;
    const char * const hdrStreamPath;
    const char * const hlgStreamPath;
    const char * const mixStreamPath;
    const char * const sdrThumbnailPath;
    const char * const sdrBackgroundPath;
} defaultRelativePaths =
{
    "gfx/sdr2hdr",
    "vid/sdr2hdr",
    "vid/hdr2sdr",
    "vid/hlg2hdr",
    "scenarios",
    "streams/sdr",
    "streams/hdr",
    "streams/hlg",
    "streams/mix",
    "images/sdr",
    "backgrounds/sdr"
};

static const Args defaultArgs =
{
    PlatformInputMethod_eRemote,
    "nxdynrng",
    "../etc/dynrng/gfx/sdr2hdr",
    "../etc/dynrng/vid/sdr2hdr",
    "../etc/dynrng/vid/hdr2sdr",
    "../etc/dynrng/vid/hlg2hdr",
    "../etc/dynrng/scenarios",
    "../share/dynrng/streams/sdr",
    "../share/dynrng/streams/hdr",
    "../share/dynrng/streams/hlg",
    "../share/dynrng/streams/mix",
    "../share/dynrng/images/sdr",
    "../share/dynrng/backgrounds/sdr",
    "../etc/dynrng",
    "../share/dynrng",
    {
        { /* dims */
            1920,
            1080
        },
        { /* colors */
            0xffffffff, /* text, white 50% */
            0x00000000, /* mainPanelBg, black 0% */
            0x00000000, /* instructionPanelBg, black 0% */
            0xff000000, /* infoPanelBg, black 100% */
            0xff000000  /* detailsPanelBg, black 100% */
        }
    },
    false
};

void args_p_get_default(ArgsHandle args)
{
    assert(args);
    memset(args, 0, sizeof(*args));
    args->method = defaultArgs.method;
    memcpy(&args->osd, &defaultArgs.osd, sizeof(args->osd));
    args->demo = defaultArgs.demo;
}

static char * args_p_make_abs_path(const char * const root, const char * const rel)
{
    char * path;
    unsigned len;

    len = strlen(root) + 1 + strlen(rel) + 1;
    path = malloc(len);
    memset(path, 0, len);
    strcat(path, root);
    strcat(path, "/");
    strcat(path, rel);

    return path;
}

static char * args_p_get_path(char * path, const char * const root, const char * const rel, const char * const def)
{
    char * newPath = path;
    if (!newPath)
    {
        if (root)
        {
            newPath = args_p_make_abs_path(root, rel);
        }
        else
        {
            newPath = strdup(def);
        }
    }
    return newPath;
}

#define ARGS_P_GET_PATH(NAME, ROOT) args->NAME = args_p_get_path(args->NAME, args->ROOT, defaultRelativePaths.NAME, defaultArgs.NAME)

ArgsHandle args_create(int argc, char **argv)
{
    ArgsHandle args;
    int curarg = 1;

    args = malloc(sizeof(*args));
    assert(args);
    args_p_get_default(args);
    args->name = argv[0];

    while (argc > curarg)
    {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help"))
        {
            args_p_print_usage(args);
            goto error;
        }
        else if (!strcmp(argv[curarg], "-im") && argc>curarg+1) {
            switch (argv[++curarg][0])
            {
                case 'c':
                    args->method = PlatformInputMethod_eConsole;
                    break;
                default:
                case 'r':
                    args->method = PlatformInputMethod_eRemote;
                    break;
            }
        }
        else if (!strcmp(argv[curarg], "-is") && argc>curarg+1) {
            args->sdrThumbnailPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-bs") && argc>curarg+1) {
            args->sdrBackgroundPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-gs") && argc>curarg+1) {
            args->gfxSdr2HdrPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-vs") && argc>curarg+1) {
            args->vidSdr2HdrPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-vh") && argc>curarg+1) {
            args->vidHdr2SdrPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-vhh") && argc>curarg+1) {
            args->vidHlg2HdrPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sc") && argc>curarg+1) {
            args->scenarioPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ss") && argc>curarg+1) {
            args->sdrStreamPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sh") && argc>curarg+1) {
            args->hdrStreamPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sg") && argc>curarg+1) {
            args->hlgStreamPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sm") && argc>curarg+1) {
            args->mixStreamPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-cfg") && argc>curarg+1) {
            args->configRoot = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-rc") && argc>curarg+1) {
            args->sharedResourceRoot = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ct") && argc>curarg+1) {
            args->osd.colors.textFg = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-cm") && argc>curarg+1) {
            args->osd.colors.mainPanelBg = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-cg") && argc>curarg+1) {
            args->osd.colors.guidePanelBg = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-ci") && argc>curarg+1) {
            args->osd.colors.infoPanelBg = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-cd") && argc>curarg+1) {
            args->osd.colors.detailsPanelBg = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "--osd") && argc>curarg+1) {
            char * p = strchr(argv[++curarg], 'x');
            if (!p) { args_p_print_usage(args); goto error; }
            *p++ = 0;
            args->osd.dims.width = strtoul(argv[curarg], NULL, 0);
            args->osd.dims.height = strtoul(p, NULL, 0);
        }
        else if (!strcmp(argv[curarg], "--demo")) {
            args->demo = true;
        }
        else
        {
            args_p_print_usage(args);
            goto error;
        }
        curarg++;
    }

    /* fixup paths */
    ARGS_P_GET_PATH(sdrThumbnailPath, sharedResourceRoot);
    ARGS_P_GET_PATH(sdrBackgroundPath, sharedResourceRoot);
    ARGS_P_GET_PATH(sdrStreamPath, sharedResourceRoot);
    ARGS_P_GET_PATH(hdrStreamPath, sharedResourceRoot);
    ARGS_P_GET_PATH(hlgStreamPath, sharedResourceRoot);
    ARGS_P_GET_PATH(mixStreamPath, sharedResourceRoot);
    ARGS_P_GET_PATH(scenarioPath, configRoot);
    ARGS_P_GET_PATH(gfxSdr2HdrPath, configRoot);
    ARGS_P_GET_PATH(vidSdr2HdrPath, configRoot);
    ARGS_P_GET_PATH(vidHdr2SdrPath, configRoot);
    ARGS_P_GET_PATH(vidHlg2HdrPath, configRoot);

    return args;

error:
    args_destroy(args);
    return NULL;
}

void args_destroy(ArgsHandle args)
{
    if (!args) return;
    if (args->gfxSdr2HdrPath) free(args->gfxSdr2HdrPath);
    if (args->vidSdr2HdrPath) free(args->vidSdr2HdrPath);
    if (args->vidHdr2SdrPath) free(args->vidHdr2SdrPath);
    if (args->vidHlg2HdrPath) free(args->vidHlg2HdrPath);
    if (args->scenarioPath) free(args->scenarioPath);
    if (args->sdrStreamPath) free(args->sdrStreamPath);
    if (args->hdrStreamPath) free(args->hdrStreamPath);
    if (args->hlgStreamPath) free(args->hlgStreamPath);
    if (args->mixStreamPath) free(args->mixStreamPath);
    if (args->sdrThumbnailPath) free(args->sdrThumbnailPath);
    if (args->sdrBackgroundPath) free(args->sdrBackgroundPath);
    if (args->configRoot) free(args->configRoot);
    if (args->sharedResourceRoot) free(args->sharedResourceRoot);
    free(args);
}
