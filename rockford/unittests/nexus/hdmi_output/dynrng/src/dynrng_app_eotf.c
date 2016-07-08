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
******************************************************************************/

#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include "dynrng_app.h"
#include "dynrng_app_priv.h"
#include "dynrng_shell.h"
#include "dynrng_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const UTILS_StringIntMapEntry eotfNames[] =
{
    { "Input", APP_Eotf_eInput },
    { "SDR (GAMMA)", APP_Eotf_eSdr },
    { "HDR10", APP_Eotf_eHdr10 },
    { "HLG", APP_Eotf_eHlg },
    { NULL, APP_Eotf_eMax }
};

const char * APP_GetEotfName(APP_Eotf eotf)
{
    return UTILS_GetTableName(eotfNames, eotf);
}

static const UTILS_StringIntMapEntry eotfAliases[] =
{
    { "input", APP_Eotf_eInput },
    { "in", APP_Eotf_eInput },
    { "sdr", APP_Eotf_eSdr },
    { "smpte", APP_Eotf_eHdr10 },
    { "2084", APP_Eotf_eHdr10 },
    { "hdr10", APP_Eotf_eHdr10 },
    { "arib", APP_Eotf_eHlg },
    { "b67", APP_Eotf_eHlg },
    { "hlg", APP_Eotf_eHlg },
    { NULL, APP_Eotf_eMax }
};

const char * APP_GetEotfAlias(APP_Eotf eotf)
{
    return UTILS_GetTableName(eotfAliases, eotf);
}

APP_Eotf APP_ParseEotf(const char * eotfStr)
{
    return (APP_Eotf)UTILS_ParseTableAlias(eotfAliases, eotfStr);
}

int APP_ToggleEotf(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool stopToggle = false;
    APP_Eotf choices[APP_Eotf_eMax];
    APP_Eotf eotf;
    APP_Eotf oldEotf;
    unsigned i = 0;
    unsigned count = 0;
    char buf[APP_MAX_INPUT_LEN];
    char * tok;

    oldEotf = app->eotf;
    fprintf(stdout, "EOTF available:");
    for (i = 0; i < APP_Eotf_eMax; i++)
    {
        choices[i] = APP_Eotf_eMax;
        fprintf(stdout, " %s", APP_GetEotfAlias(i));
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "Please enter a whitespace-separated list of EOTFs from the list above, or press enter to toggle between SDR (GAMMA) and SMPTE ST2084: ");

    fgets(buf, APP_MAX_INPUT_LEN, stdin);

    count = 0;
    for (tok = strtok(buf, " \t\n"); tok; tok = strtok(NULL, " \t\n"))
    {
        eotf = APP_ParseEotf(tok);
        if (eotf != APP_Eotf_eMax)
        {
            choices[count++] = eotf;
        }
    }

    if (!count)
    {
        choices[0] = APP_Eotf_eSdr;
        choices[1] = APP_Eotf_eHdr10;
        count = 2;
    }

    fprintf(stdout, "Toggling among:");
    for (i = 0; i < count; i++)
    {
        fprintf(stdout, " %s", APP_GetEotfAlias(choices[i]));
    }
    fprintf(stdout, "\nType 'q<enter>' to exit\n");

    i = 0;
    while (!stopToggle)
    {
        eotf = choices[i];
        i = (i + 1) % count;

        fprintf(stdout, "\npress enter to set EOTF to %s\n", APP_GetEotfName(eotf));
        fgets(buf, APP_MAX_INPUT_LEN, stdin);
        if (buf[0] == 'q')
        {
            stopToggle = true;
            continue;
        }
        rc = APP_SetEotf(app, eotf);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
        /* wait for console to settle after application of HDMI changes */
        BKNI_Sleep(500);
    }

    fprintf(stdout, "\nReturning EOTF to %s\n", APP_GetEotfName(oldEotf));
    rc = APP_SetEotf(app, oldEotf);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

APP_Eotf APP_NexusEotfToApp(NEXUS_VideoEotf nxEotf)
{
    APP_Eotf eotf = APP_Eotf_eMax;

    switch (nxEotf)
    {
        case NEXUS_VideoEotf_eSdr:
            eotf = APP_Eotf_eSdr;
            break;
        case NEXUS_VideoEotf_eHlg:
            eotf = APP_Eotf_eHlg;
            break;
        case NEXUS_VideoEotf_eHdr10:
            eotf = APP_Eotf_eHdr10;
            break;
        default:
            break;
    }

    return eotf;
}

NEXUS_VideoEotf APP_AppEotfToNexus(APP_Eotf eotf)
{
    NEXUS_VideoEotf nxEotf = NEXUS_VideoEotf_eMax;

    switch (eotf)
    {
        case APP_Eotf_eSdr:
            nxEotf = NEXUS_VideoEotf_eSdr;
            break;
        case APP_Eotf_eHlg:
            nxEotf = NEXUS_VideoEotf_eHlg;
            break;
        case APP_Eotf_eHdr10:
            nxEotf = NEXUS_VideoEotf_eHdr10;
            break;
        default:
            break;
    }

    return nxEotf;
}

int APP_SetEotf(APP_AppHandle app, APP_Eotf eotf)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoEotf nxEotf = NEXUS_VideoEotf_eMax;

    app->eotf = eotf;
    switch (eotf)
    {
        case APP_Eotf_eInput:
            if (app->lastStreamInfoValid)
            {
                nxEotf = app->lastStreamInfo.eotf;
            }
            else
            {
                nxEotf = NEXUS_VideoEotf_eSdr;
            }
            break;
        default:
            nxEotf = APP_AppEotfToNexus(eotf);
            break;
    }

    if (nxEotf != NEXUS_VideoEotf_eMax)
    {
        app->drmInfoFrame.eotf = nxEotf;
        rc = APP_ApplyDrmInfoFrame(app);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
        rc = APP_UpdateOsd(app);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }

end:
    return rc;
}

void APP_PrintEotf(APP_AppHandle app)
{
    const char * outputEotfName = APP_GetEotfName(APP_NexusEotfToApp(app->drmInfoFrame.eotf));
    fprintf(stdout, "# EOTF\n");
    fprintf(stdout, "input = %s\n",
        app->lastStreamInfoValid ?
            APP_GetEotfName(APP_NexusEotfToApp(app->lastStreamInfo.eotf))
            : "unknown");
    fprintf(stdout, "output = %s\n", outputEotfName);
    if (app->sinkEdidValid)
    {
        fprintf(stdout, "TV %s %s\n",
            app->sinkEdid.hdrdb.eotfSupported[app->drmInfoFrame.eotf] ? "supports" : "does not support",
            outputEotfName);
    }
    else
    {
        fprintf(stdout, "TV support of %s unknown\n", outputEotfName);
    }
}

int APP_DoEotf(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    APP_Eotf eotf = APP_Eotf_eMax;
    const char * eotfStr;

    if (args)
    {
        eotfStr = strtok(args, " \t\n");
        if (!strcmp("toggle", eotfStr))
        {
            rc = APP_ToggleEotf(app);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
        }
        else
        {
            eotf = APP_ParseEotf(eotfStr);
            if (eotf != APP_Eotf_eMax)
            {
                rc = APP_SetEotf(app, eotf);
                if (rc) { rc = BERR_TRACE(rc); goto end; }
            }
            else
            {
                fprintf(stderr, "Unrecognized argument '%s' ignored\n", args);
                goto end;
            }
        }
    }
    else
    {
        APP_PrintEotf(app);
    }

end:
    return rc;
}

int APP_SetupEotfCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "eotf",
        "changes EOTF behavior on the STB HDMI output. With no args, reports current eotf status to console.",
        "[hdr10|hlg|sdr|input|toggle]",
        &APP_DoEotf,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "e");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "hdr10", false, "selects HDR10 EOTF");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "sdr", false, "selects SDR EOTF");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "hlg", false, "selects HLG EOTF");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "input", false, "selects input EOTF, if one is present, otherwise SDR GAMMA");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "toggle", false, "enters an interactive mode allowing toggling of the transmitted EOTF between SDR and HDR10");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}
