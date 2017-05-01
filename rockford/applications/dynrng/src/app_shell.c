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
#include "platform.h"
#include "app.h"
#include "app_priv.h"
#include "app_shell_priv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#define strtok(X) strtok(X, " \n\t")

static const char * colorSpaceStrings[] =
{
    "auto",
    "rgb",
    "ycc",
    "ycc444",
    "ycc420",
    NULL
};

void app_p_print_set(AppHandle app, const AppSetPrinter * pPrinter)
{
    if (pPrinter->output.dynrng)
    {
        fprintf(stdout, "output dynamic range: '%s'\n", platform_get_dynamic_range_name(app->output.dynrng));
    }
    if (pPrinter->output.colorSpace)
    {
        fprintf(stdout, "output color space: %s\n", colorSpaceStrings[app->output.colorSpace]);
    }
}

static AppSetState app_p_handle_set_syntax_error(AppHandle app, const char * tok, const char * expected)
{
    fprintf(stdout, "Syntax error: expected [%s]: '%s'\n", expected, tok);
    shell_help_command(app->shell, "set");
    return AppSetState_eDone;
}

static AppSetState app_p_do_set_start(AppHandle app, AppSetState start, const char * tok, AppSetPrinter * pPrinter)
{
    AppSetState state = start;

    if (!strcmp(tok, "output") || tok[0] == 'o')
    {
        state = AppSetState_eOutput;
    }
    else
    {
        state = app_p_handle_set_syntax_error(app, tok, "block");
    }

    return state;
}

static AppSetState app_p_do_set_output(AppHandle app, AppSetState start, const char * tok, AppSetPrinter * pPrinter)
{
    AppSetState state = start;
    if (!strcmp(tok, "dynrng") || tok[0] == 'r')
    {
        pPrinter->output.dynrng = true;
        state = AppSetState_eOutputDynrng;
    }
    else if (!strcmp(tok, "clrspc") || tok[0] == 's')
    {
        pPrinter->output.colorSpace = true;
        state = AppSetState_eOutputColorSpace;
    }
    else
    {
        state = app_p_handle_set_syntax_error(app, tok, "name");
    }
    return state;
}

static AppSetState app_p_do_set_output_dynrng(AppHandle app, AppSetState start, const char * tok, AppSetPrinter * pPrinter)
{
    AppSetState state = start;
    if ((!strcmp(tok, "auto") || tok[0] == 'a') && app->output.dynrng != PlatformDynamicRange_eAuto)
    {
        app->output.dynrng = PlatformDynamicRange_eAuto;
        state = AppSetState_eUpdateModel;
    }
    else if ((!strcmp(tok, "dvs") || tok[0] == 'd') && app->output.dynrng != PlatformDynamicRange_eDolbyVision)
    {
        app->output.dynrng = PlatformDynamicRange_eDolbyVision;
        state = AppSetState_eUpdateModel;
    }
    else if ((!strcmp(tok, "hdr") || tok[0] == 'h') && app->output.dynrng != PlatformDynamicRange_eHdr10)
    {
        app->output.dynrng = PlatformDynamicRange_eHdr10;
        state = AppSetState_eUpdateModel;
    }
    else if ((!strcmp(tok, "sdr") || tok[0] == 's') && app->output.dynrng != PlatformDynamicRange_eSdr)
    {
        app->output.dynrng = PlatformDynamicRange_eSdr;
        state = AppSetState_eUpdateModel;
    }
    else
    {
        fprintf(stderr, "Unrecognized dynamic range specifier: '%s'\n", tok);
        state = AppSetState_eDone;
    }
    return state;
}

static AppSetState app_p_do_set_output_color_space(AppHandle app, AppSetState start, const char * tok, AppSetPrinter * pPrinter)
{
    AppSetState state = start;

    if ((!strcmp(tok, "auto") || tok[0] == 'a') && app->output.colorSpace != PlatformColorSpace_eAuto)
    {
        app->output.colorSpace = PlatformColorSpace_eAuto;
        state = AppSetState_eUpdateModel;
    }
    else if ((!strcmp(tok, "ycc") || tok[0] == 'y') && app->output.colorSpace != PlatformColorSpace_eYCbCr422)
    {
        app->output.colorSpace = PlatformColorSpace_eYCbCr422;
        state = AppSetState_eUpdateModel;
    }
    else if ((!strcmp(tok, "rgb") || tok[0] == 'r') && app->output.colorSpace != PlatformColorSpace_eRgb)
    {
        app->output.colorSpace = PlatformColorSpace_eRgb;
        state = AppSetState_eUpdateModel;
    }
    else
    {
        fprintf(stderr, "Unrecognized color space specifier: '%s'\n", tok);
        state = AppSetState_eDone;
    }

    return state;
}

int app_p_do_set(void * context, char * args)
{
    AppHandle app = context;
    AppSetState state = AppSetState_eStart;
    AppSetPrinter printer;
    char * tok = NULL;

    memset(&printer, 0, sizeof(printer));

    if (!args)
    {
        /* print out all variables */
        memset(&printer, 0xff, sizeof(printer));
        goto print;
    }

    tok = strtok(args);
    if (tok)
    {
        while (tok)
        {
            switch (state)
            {
                case AppSetState_eStart:
                    state = app_p_do_set_start(app, state, tok, &printer);
                    break;
                case AppSetState_eOutput:
                    state = app_p_do_set_output(app, state, tok, &printer);
                    break;
                case AppSetState_eOutputDynrng:
                    state = app_p_do_set_output_dynrng(app, state, tok, &printer);
                    break;
                case AppSetState_eOutputColorSpace:
                    state = app_p_do_set_output_color_space(app, state, tok, &printer);
                    break;
                default:
                    break;
            }
            tok = strtok(NULL);
        } /* no more tokens after this point */

        /* apply changes if needed */
        switch (state)
        {
            case AppSetState_eUpdateModel:
                app_p_update_sel_model(app);
                app_p_update_out_model(app);
                app_p_reapply_plm(app);
                break;
            default:
                break;
        }

        /* print out all variables if no other args given */
        switch (state)
        {
            case AppSetState_eOutput:
                memset(&printer.output, 0xff, sizeof(printer.output));
                break;
            default:
                break;
        }
        state = AppSetState_eDone;
    }
    else
    {
        /* print out all variables */
        memset(&printer, 0xff, sizeof(printer));
    }

print:
    app_p_print_set(app, &printer);
    return 0;
}

int app_p_setup_set_command(AppHandle app)
{
    int rc = 0;
    CommandHandle command = NULL;

    command = shell_create_command(app->shell,
        "set",
        "sets a single block variable",
        "[block] [module] [submodule] [name] [value]",
        &app_p_do_set,
        app);
    if (!command) { rc = -1; goto end; }
    rc = shell_add_command_alias(command, "s");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "output", false, "selects output block");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "o", false, "selects output block");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "dynrng", false, "auto | sdr | hdr - sets output dynrng");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "clrspc", false, "auto | ycc | rgb - sets output color space");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "auto", false, "sets output setting automatically from EDID and source info");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "ycc", false, "sets output color space to ycc");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "rgb", false, "sets output color space to rgb");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "sdr", false, "sets output dynrng to sdr");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "hdr", false, "sets output dynrng to hdr");
    if (rc) goto end;
    rc = shell_add_command_arg(command, "dvs", false, "sets output dynrng to dvs");
    if (rc) goto end;
end:
    return rc;
}

int app_p_do_pause(void * context, char * args)
{
    (void)args;
    app_p_toggle_pause(context, 0);
    return 0;
}

int app_p_setup_pause_command(AppHandle app)
{
    int rc = 0;
    CommandHandle command = NULL;

    command = shell_create_command(app->shell,
        "pause",
        "toggles pausing/playing the input stream",
        "",
        &app_p_do_pause,
        app);
    if (!command) { rc = -1; goto end; }
    rc = shell_add_command_alias(command, "play");
    if (rc) goto end;
    rc = shell_add_command_alias(command, "p");
    if (rc) goto end;

end:
    return rc;
}

int app_p_do_adv(void * context, char * args)
{
    AppHandle app = context;
    (void)args;
    stream_player_frame_advance(app->streamPlayer[0], false);
    return 0;
}

int app_p_setup_adv_command(AppHandle app)
{
    int rc = 0;
    CommandHandle command = NULL;

    command = shell_create_command(app->shell,
        "adv",
        "advances input stream one frame",
        "",
        &app_p_do_adv,
        app);
    if (!command) { rc = -1; goto end; }
    rc = shell_add_command_alias(command, "+");
    if (rc) goto end;

end:
    return rc;
}

void app_p_init_shell(AppHandle app)
{
    int rc = 0;

    rc = app_p_setup_set_command(app);
    if (rc) goto end;

    rc = app_p_setup_pause_command(app);
    if (rc) goto end;

    rc = app_p_setup_adv_command(app);
    if (rc) goto end;

end:
    return;
}
