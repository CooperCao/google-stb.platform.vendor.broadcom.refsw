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
#include "args.h"
#include "args_priv.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const Args defaultArgs =
{
    "dynrng",
    "../etc/dynrng/plm.conf",
    1
};

void args_p_print_usage(ArgsHandle args)
{
    assert(args);
    printf("Usage: %s OPTIONS\n", args->name);
    printf("-h  this help\n");
    printf("-c config-file-path  path to config file. Defaults to %s\n", defaultArgs.configPath);
    printf("-s startup-scenario-number  scenario to play on startup.  Defaults to %d\n", defaultArgs.scenario);
}

ArgsHandle args_create(int argc, char **argv)
{
    ArgsHandle args;
    int curarg = 1;

    args = malloc(sizeof(*args));
    assert(args);
    memset(args, 0, sizeof(*args));
    args->name = argv[0];
    args->scenario = defaultArgs.scenario;

    while (argc > curarg)
    {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help"))
        {
            args_p_print_usage(args);
            goto error;
        }
        else if (!strcmp(argv[curarg], "-c") && argc>curarg+1) {
            args->configPath = strdup(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-s") && argc>curarg+1) {
            args->scenario = strtoul(argv[++curarg], NULL, 0);
        }
        else
        {
            args_p_print_usage(args);
            goto error;
        }
        curarg++;
    }

    if (!args->configPath)
    {
        args->configPath = strdup(defaultArgs.configPath);
    }

    return args;

error:
    args_destroy(args);
    return NULL;
}

void args_destroy(ArgsHandle args)
{
    if (!args) return;
    if (args->configPath) free(args->configPath);
    free(args);
}
