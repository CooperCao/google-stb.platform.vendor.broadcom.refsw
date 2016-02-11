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
#include "dynrng_shell.h"
#include "dynrng_shell_priv.h"
#include "dynrng_utils.h"
#include <string.h>
#include <stdio.h>

int SHELL_DoHelp(void * context, char * args)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_ShellHandle shell = context;
    SHELL_CommandHandle cmd = NULL;
    SHELL_CommandAlias * pAlias = NULL;
    SHELL_CommandArg * pArg = NULL;
    const char * argStr = NULL;

    BSTD_UNUSED(args);

    if (!args)
    {
        for (cmd = BLST_Q_FIRST(&shell->commands); cmd; cmd = BLST_Q_NEXT(cmd, link))
        {
            pAlias = BLST_Q_FIRST(&cmd->aliases);
            fprintf(stdout, " %s", pAlias ? pAlias->spelling : "<unnamed>");
            if (cmd->syntax)
            {
                fprintf(stdout, " %s", cmd->syntax);
            }
            fprintf(stdout, "\n");
        }
    }
    else
    {
        for (argStr = strtok(args, " \t\n"); argStr; argStr = strtok(NULL, " \t\n"))
        {
            cmd = SHELL_FindCommandByName(shell, argStr);
            if (cmd)
            {
                pAlias = BLST_Q_FIRST(&cmd->aliases);
                fprintf(stdout, "%s: ", pAlias ? pAlias->spelling : "<unnamed>");
                if (cmd->syntax)
                {
                    fprintf(stdout, "%s %s", pAlias->spelling, cmd->syntax);
                }
                fprintf(stdout, "\n");
                if (cmd->help)
                {
                    fprintf(stdout, "    %s\n", cmd->help);
                }
                pAlias = BLST_Q_NEXT(pAlias, link);
                if (pAlias)
                {
                    fprintf(stdout, "  Aliases\n    ");
                    for (; pAlias; pAlias = BLST_Q_NEXT(pAlias, link))
                    {
                        fprintf(stdout, "%s ", pAlias->spelling);
                    }
                    fprintf(stdout, "\n");
                }
                if (!BLST_Q_EMPTY(&cmd->args))
                {
                    fprintf(stdout, "  Arguments:\n");
                    for (pArg = BLST_Q_FIRST(&cmd->args); pArg; pArg = BLST_Q_NEXT(pArg, link))
                    {
                        fprintf(stdout, "    %s    %s.", pArg->spelling, pArg->required ? "Required" : "Optional");
                        if (pArg->help)
                        {
                            fprintf(stdout, " %s", pArg->help);
                        }
                        fprintf(stdout, "\n");
                    }
                }
            }
            else
            {
                fprintf(stdout, "Command '%s' not found\n", args);
            }
        }
    }

    return rc;
}

int SHELL_DoExit(void * context, char * args)
{
    SHELL_ShellHandle shell = context;
    BSTD_UNUSED(args);
    shell->stop = true;
    return 0;
}

int SHELL_SetupDefaultCommands(SHELL_ShellHandle shell)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    command = SHELL_CreateCommand(shell, "exit", "exits the shell", NULL, &SHELL_DoExit, shell);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "x");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandAlias(command, "quit");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = SHELL_AddCommandAlias(command, "q");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    command = SHELL_CreateCommand(shell, "help", "prints a help message", NULL, &SHELL_DoHelp, shell);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = SHELL_AddCommandAlias(command, "h");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

SHELL_ShellHandle SHELL_Create(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_ShellHandle shell = NULL;

    shell = BKNI_Malloc(sizeof(struct SHELL_Shell));
    if (!shell) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(shell, 0, sizeof(struct SHELL_Shell));

    rc = SHELL_SetupDefaultCommands(shell);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    return shell;

error:
    if (shell)
    {
        SHELL_Destroy(shell);
    }
    return NULL;
}

void SHELL_DestroyCommandAlias(SHELL_CommandAlias * pAlias)
{
    if (pAlias)
    {
        if (pAlias->spelling)
        {
            BKNI_Free(pAlias->spelling);
        }
        BKNI_Free(pAlias);
    }
}

void SHELL_DestroyCommandArg(SHELL_CommandArg * pArg)
{
    if (pArg)
    {
        if (pArg->spelling)
        {
            BKNI_Free(pArg->spelling);
        }
        if (pArg->help)
        {
            BKNI_Free(pArg->help);
        }
        BKNI_Free(pArg);
    }
}

void SHELL_DestroyCommand(SHELL_CommandHandle command)
{
    SHELL_CommandArg * arg = NULL;
    SHELL_CommandAlias * alias = NULL;

    if (command)
    {
        for (alias = BLST_Q_FIRST(&command->aliases); alias; alias = BLST_Q_FIRST(&command->aliases))
        {
            BLST_Q_REMOVE_HEAD(&command->aliases, link);
            SHELL_DestroyCommandAlias(alias);
        }
        for (arg = BLST_Q_FIRST(&command->args); arg; arg = BLST_Q_FIRST(&command->args))
        {
            BLST_Q_REMOVE_HEAD(&command->args, link);
            SHELL_DestroyCommandArg(arg);
        }
        BKNI_Free(command);
    }
}

void SHELL_Destroy(SHELL_ShellHandle shell)
{
    SHELL_CommandHandle command = NULL;

    if (shell)
    {
        for (command = BLST_Q_FIRST(&shell->commands); command; command = BLST_Q_FIRST(&shell->commands))
        {
            BLST_Q_REMOVE_HEAD(&shell->commands, link);
            SHELL_DestroyCommand(command);
        }
        BKNI_Free(shell);
    }
}

SHELL_CommandHandle SHELL_CreateCommand(SHELL_ShellHandle shell, const char * name, const char * help, const char * argSyntax, SHELL_CommandHandler handler, void * context)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandHandle command = NULL;

    if (!shell || !name || !handler) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto error; }

    command = BKNI_Malloc(sizeof(struct SHELL_Command));
    if (!command) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(command, 0, sizeof(struct SHELL_Command));

    rc = SHELL_AddCommandAlias(command, name);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    command->help = UTILS_SetString(command->help, help);
    command->syntax = UTILS_SetString(command->syntax, argSyntax);
    command->handler = handler;
    command->context = context;

    BLST_Q_INSERT_TAIL(&shell->commands, command, link);

    return command;

error:
    if (command)
    {
        SHELL_DestroyCommand(command);
    }
    return NULL;
}

SHELL_CommandAlias * SHELL_CreateCommandAlias(const char * alias)
{
    SHELL_CommandAlias * pAlias = NULL;

    pAlias = BKNI_Malloc(sizeof(SHELL_CommandAlias));
    if (!pAlias) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(pAlias, 0, sizeof(SHELL_CommandAlias));

    pAlias->spelling = UTILS_SetString(pAlias->spelling, alias);
    if (!pAlias->spelling) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    return pAlias;

error:
    if (pAlias)
    {
        SHELL_DestroyCommandAlias(pAlias);
    }
    return NULL;
}

int SHELL_AddCommandAlias(SHELL_CommandHandle command, const char * alias)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandAlias * pAlias;

    if (!command) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    if (alias)
    {
        pAlias = SHELL_CreateCommandAlias(alias);
        if (!pAlias) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
        BLST_Q_INSERT_TAIL(&command->aliases, pAlias, link);
    }

end:
    return rc;
}

SHELL_CommandArg * SHELL_CreateCommandArg(const char * spelling, bool required, const char * help)
{
    SHELL_CommandArg * pArg = NULL;

    pArg = BKNI_Malloc(sizeof(SHELL_CommandArg));
    if (!pArg) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(pArg, 0, sizeof(SHELL_CommandArg));

    pArg->spelling = UTILS_SetString(pArg->spelling, spelling);
    if (!pArg->spelling) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    pArg->required = required;
    pArg->help = UTILS_SetString(pArg->help, help);

    return pArg;

error:
    if (pArg)
    {
        SHELL_DestroyCommandArg(pArg);
    }
    return NULL;
}

int SHELL_AddCommandArg(SHELL_CommandHandle command, const char * argSpelling, bool required, const char * argHelp)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    SHELL_CommandArg * pArg;

    if (!command) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    if (argSpelling)
    {
        pArg = SHELL_CreateCommandArg(argSpelling, required, argHelp);
        if (!pArg) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
        BLST_Q_INSERT_TAIL(&command->args, pArg, link);
    }

end:
    return rc;
}

SHELL_CommandHandle SHELL_FindCommandByName(SHELL_ShellHandle shell, const char * name)
{
    const SHELL_CommandAlias * pAlias = NULL;
    SHELL_CommandHandle command = NULL;
    SHELL_CommandHandle c = NULL;

    if (!shell || !name) { BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    for (c = BLST_Q_FIRST(&shell->commands); c; c = BLST_Q_NEXT(c, link))
    {
        for (pAlias = BLST_Q_FIRST(&c->aliases); pAlias; pAlias = BLST_Q_NEXT(pAlias, link))
        {
            if (!strcmp(name, pAlias->spelling))
            {
                command = c;
                break;
            }
        }
    }

end:
    return command;
}

bool SHELL_CommandRequiresArgs(SHELL_CommandHandle command)
{
    bool required = false;
    const SHELL_CommandArg * pArg = NULL;

    if (!command) { BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    for (pArg = BLST_Q_FIRST(&command->args); pArg; pArg = BLST_Q_NEXT(pArg, link))
    {
        if (pArg->required)
        {
            required = true;
            break;
        }
    }

end:
    return required;
}


int SHELL_Run(SHELL_ShellHandle shell)
{
    int rc = 0;
    char line[MAX_INPUT_LENGTH];
    char * name;
    char * args;
    char * comment;
    SHELL_CommandHandle cmd = NULL;

    /* wait for console to settle before presenting UI */
    BKNI_Sleep(1500);

    while (!shell->stop)
    {
        fprintf(stdout, "dynrngsh$ ");
        fgets(line, MAX_INPUT_LENGTH, stdin);
        comment = strchr(line, '#');
        if (comment) { *comment = 0; }
        name = line;
        args = strpbrk(line, " \t\n");
        if (args)
        {
            *args++ = 0;
            if (!*args) args = NULL;
        }

        name = UTILS_Trim(name);
        if (!name) continue;

        args = UTILS_Trim(args);

        cmd = SHELL_FindCommandByName(shell, name);
        if (cmd)
        {
            if (!SHELL_CommandRequiresArgs(cmd) || args)
            {
                if (cmd->handler)
                {
                    rc = cmd->handler(cmd->context, args);
                    if (rc) { fprintf(stderr, "Error performing command: '%s'\n", name); BERR_TRACE(rc); }
                }
            }
            else
            {
                fprintf(stderr, "Command '%s' requires arguments\n", name);
            }
        }
        else
        {
            fprintf(stderr, "Unrecognized command: '%s'\n", line);
        }
    }

    return 0;
}

void SHELL_Print(SHELL_ShellHandle shell)
{
    fprintf(stdout, "# SHELL\n");
    fprintf(stdout, "This function has not yet been implemented\n");
    BSTD_UNUSED(shell);
    /* TODO */
}
