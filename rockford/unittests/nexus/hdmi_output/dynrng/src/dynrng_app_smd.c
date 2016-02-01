/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
******************************************************************************/

#include "nexus_platform.h"
#include "nexus_hdmi_output.h"
#include "bstd.h"
#include "bkni.h"
#include "dynrng_args.h"
#include "dynrng_app.h"
#include "dynrng_app_priv.h"
#include "dynrng_shell.h"
#include "dynrng_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int APP_ToggleSmd(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool stopToggle = false;
    SMD_SmdSource choices[SMD_SmdSource_eMax];
    SMD_SmdSource source;
    SMD_SmdSource oldSource;
    unsigned i = 0;
    unsigned count = 0;
    char buf[APP_MAX_INPUT_LEN];
    char * tok;

    oldSource = SMD_GetSmdSource(app->smd);
    fprintf(stdout, "SMD sources:");
    for (i = 0; i < SMD_SmdSource_eMax; i++)
    {
        choices[i] = SMD_SmdSource_eMax;
        fprintf(stdout, " %s", SMD_GetSmdSourceName(i));
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "Please enter a whitespace-separated list of sources from the list above, or press enter to toggle between zero and input: ");

    fgets(buf, APP_MAX_INPUT_LEN, stdin);

    count = 0;
    for (tok = strtok(buf, " \t\n"); tok; tok = strtok(NULL, " \t\n"))
    {
        source = SMD_ParseSmdSource(tok);
        if (source != SMD_SmdSource_eMax)
        {
            choices[count++] = source;
        }
    }

    if (!count)
    {
        choices[0] = SMD_SmdSource_eZero;
        choices[1] = SMD_SmdSource_eInput;
        count = 2;
    }

    fprintf(stdout, "Toggling among:");
    for (i = 0; i < count; i++)
    {
        fprintf(stdout, " %s", SMD_GetSmdSourceName(choices[i]));
    }
    fprintf(stdout, "\nType 'q<enter>' to exit\n");

    i = 0;
    while (!stopToggle)
    {
        source = choices[i];
        i = (i + 1) % count;

        fprintf(stdout, "\npress enter to set SMD source to %s\n", SMD_GetSmdSourceName(source));
        fgets(buf, APP_MAX_INPUT_LEN, stdin);
        if (buf[0] == 'q')
        {
            stopToggle = true;
            continue;
        }

        rc = APP_ApplySmdSource(app, source);
        if (rc) { rc = BERR_TRACE(rc); goto end; }

        /* wait for console to settle after application of HDMI changes */
        BKNI_Sleep(500);
    }

    fprintf(stdout, "\nReturning SMD source to %s\n", SMD_GetSmdSourceName(oldSource));
    rc = APP_ApplySmdSource(app, oldSource);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

int APP_SetSmdFilePath(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    char buf[APP_MAX_INPUT_LEN];

    if (!app->args.smdFilename)
    {
        char * p = NULL;
        fprintf(stdout, "Please enter the SMD filename: ");
        fgets(buf, APP_MAX_INPUT_LEN, stdin);
        p = strchr(buf, '\n');
        if (*p) { *p = 0; }
        app->args.smdFilename = UTILS_SetString(app->args.smdFilename, buf);
    }
    rc = SMD_SetFilePath(app->smd, app->args.smdFilename);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

int APP_ApplySmdSource(APP_AppHandle app, SMD_SmdSource source)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (source == SMD_SmdSource_eFile)
    {
        rc = APP_SetSmdFilePath(app);
        if (rc) { rc = BERR_TRACE(rc); goto end; }
    }

    rc = SMD_SetSmdSource(app->smd, source);
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SMD_GetMetadata(app->smd, &app->drmInfoFrame.metadata);
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = APP_ApplyDrmInfoFrame(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = APP_UpdateOsd(app);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

int APP_DoSmd(void * context, char * args)
{
    APP_AppHandle app = context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    SMD_SmdSource source = SMD_SmdSource_eMax;
    const char * sourceStr;
    const char * path = NULL;

    if (args)
    {
        if (!strcmp(args, "toggle"))
        {
            rc = APP_ToggleSmd(app);
            if (rc) { rc = BERR_TRACE(rc); goto end; }
        }
        else
        {
            sourceStr = strtok(args, " \t\n");
            source = SMD_ParseSmdSource(sourceStr);
            if (source != SMD_SmdSource_eMax)
            {
                path = strtok(NULL, " \t\n");
                if (path)
                {
                    app->args.smdFilename = UTILS_SetString(app->args.smdFilename, path);
                }
                APP_ApplySmdSource(app, source);
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
        SMD_Print(app->smd);
    }

end:
    return rc;
}

int APP_SetupSmdCommand(APP_AppHandle app)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(app->shell,
        "smd",
        "changes Static Metadata used at the STB HDMI output. With no args, reports current smd status to console.",
        "[bt2020|bt709|zero|input|file|toggle]",
        &APP_DoSmd,
        app);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandArg(command, "bt2020", false, "selects BT2020 color volume.  Content light is left unspecified.");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "bt709", false, "selects BT709 color volume.  Content light is left unspecified.");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "zero", false, "selects zero for all SMD fields -- this is the default, HDMI legal, and indicates that the info is unknown");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "file", false, "loads the SMD values from a file, specified either at startup by -m option or by query at the time of command issue (if -m was not given)");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "input", false, "uses the SMD values from the input, if present, otherwise zero / unspecified.");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandArg(command, "toggle", false, "enters an interactive mode allowing toggling of the transmitted SMD between zero and a set queried at the time of command issue");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}
