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
#include "shell.h"
#include "util_priv.h"
#include <string.h>
#include <stdio.h>
#include "shell_priv.h"

static int shell_p_do_help(void * context, char * args)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    ShellHandle shell = context;
    CommandHandle cmd = NULL;
    CommandAlias * pAlias = NULL;
    CommandArg * pArg = NULL;
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
            cmd = shell_p_find_command_by_name(shell, argStr);
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

void shell_help_command(ShellHandle shell, char * commandName)
{
    shell_p_do_help(shell, commandName);
}

static int shell_p_do_exit(void * context, char * args)
{
    ShellHandle shell = context;
    BSTD_UNUSED(args);
    shell->stop = true;
    return 0;
}

int shell_p_setup_default_commands(ShellHandle shell)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    CommandHandle command = NULL;

    command = shell_create_command(shell, "exit", "exits the shell", NULL, &shell_p_do_exit, shell);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = shell_add_command_alias(command, "x");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = shell_add_command_alias(command, "quit");
    if (rc) { rc = BERR_TRACE(rc); goto end; }
    rc = shell_add_command_alias(command, "q");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

    command = shell_create_command(shell, "help", "prints a help message", NULL, &shell_p_do_help, shell);
    if (!command) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    rc = shell_add_command_alias(command, "h");
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

ShellHandle shell_create(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    ShellHandle shell = NULL;

    shell = BKNI_Malloc(sizeof(struct Shell));
    if (!shell) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(shell, 0, sizeof(struct Shell));

    rc = shell_p_setup_default_commands(shell);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    return shell;

error:
    if (shell)
    {
        shell_destroy(shell);
    }
    return NULL;
}

void shell_p_destroy_command_alias(CommandAlias * pAlias)
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

void shell_p_destroy_command_arg(CommandArg * pArg)
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

void shell_p_destroy_command(CommandHandle command)
{
    CommandArg * arg = NULL;
    CommandAlias * alias = NULL;

    if (command)
    {
        for (alias = BLST_Q_FIRST(&command->aliases); alias; alias = BLST_Q_FIRST(&command->aliases))
        {
            BLST_Q_REMOVE_HEAD(&command->aliases, link);
            shell_p_destroy_command_alias(alias);
        }
        for (arg = BLST_Q_FIRST(&command->args); arg; arg = BLST_Q_FIRST(&command->args))
        {
            BLST_Q_REMOVE_HEAD(&command->args, link);
            shell_p_destroy_command_arg(arg);
        }
        BKNI_Free(command);
    }
}

void shell_destroy(ShellHandle shell)
{
    CommandHandle command = NULL;

    if (shell)
    {
        for (command = BLST_Q_FIRST(&shell->commands); command; command = BLST_Q_FIRST(&shell->commands))
        {
            BLST_Q_REMOVE_HEAD(&shell->commands, link);
            shell_p_destroy_command(command);
        }
        BKNI_Free(shell);
    }
}

CommandHandle shell_create_command(ShellHandle shell, const char * name, const char * help, const char * argSyntax, CommandHandler handler, void * context)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    CommandHandle command = NULL;

    if (!shell || !name || !handler) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto error; }

    command = BKNI_Malloc(sizeof(struct Command));
    if (!command) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(command, 0, sizeof(struct Command));

    rc = shell_add_command_alias(command, name);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    command->help = set_string(command->help, help);
    command->syntax = set_string(command->syntax, argSyntax);
    command->handler = handler;
    command->context = context;

    BLST_Q_INSERT_TAIL(&shell->commands, command, link);

    return command;

error:
    if (command)
    {
        shell_p_destroy_command(command);
    }
    return NULL;
}

CommandAlias * shell_create_command_alias(const char * alias)
{
    CommandAlias * pAlias = NULL;

    pAlias = BKNI_Malloc(sizeof(CommandAlias));
    if (!pAlias) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(pAlias, 0, sizeof(CommandAlias));

    pAlias->spelling = set_string(pAlias->spelling, alias);
    if (!pAlias->spelling) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

    return pAlias;

error:
    if (pAlias)
    {
        shell_p_destroy_command_alias(pAlias);
    }
    return NULL;
}

int shell_add_command_alias(CommandHandle command, const char * alias)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    CommandAlias * pAlias;

    if (!command) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    if (alias)
    {
        pAlias = shell_create_command_alias(alias);
        if (!pAlias) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
        BLST_Q_INSERT_TAIL(&command->aliases, pAlias, link);
    }

end:
    return rc;
}

CommandArg * shell_p_create_command_arg(const char * spelling, bool required, const char * help)
{
    CommandArg * pArg = NULL;

    pArg = BKNI_Malloc(sizeof(CommandArg));
    if (!pArg) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    BKNI_Memset(pArg, 0, sizeof(CommandArg));

    pArg->spelling = set_string(pArg->spelling, spelling);
    if (!pArg->spelling) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }
    pArg->required = required;
    pArg->help = set_string(pArg->help, help);

    return pArg;

error:
    if (pArg)
    {
        shell_p_destroy_command_arg(pArg);
    }
    return NULL;
}

int shell_add_command_arg(CommandHandle command, const char * argSpelling, bool required, const char * argHelp)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    CommandArg * pArg;

    if (!command) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto end; }

    if (argSpelling)
    {
        pArg = shell_p_create_command_arg(argSpelling, required, argHelp);
        if (!pArg) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
        BLST_Q_INSERT_TAIL(&command->args, pArg, link);
    }

end:
    return rc;
}

CommandHandle shell_p_find_command_by_name(ShellHandle shell, const char * name)
{
    const CommandAlias * pAlias = NULL;
    CommandHandle command = NULL;
    CommandHandle c = NULL;

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

bool shell_p_command_requires_args(CommandHandle command)
{
    bool required = false;
    const CommandArg * pArg = NULL;

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


void shell_run(ShellHandle shell)
{
    int rc = 0;
    char line[MAX_INPUT_LENGTH];
    char * name;
    char * args;
    char * comment;
    CommandHandle cmd = NULL;

    shell->stop = false;

    while (!shell->stop)
    {
        fprintf(stdout, "[dynrng]# ");
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

        name = trim(name);
        if (!name) continue;

        args = trim(args);

        cmd = shell_p_find_command_by_name(shell, name);
        if (cmd)
        {
            if (!shell_p_command_requires_args(cmd) || args)
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
}

void shell_print(ShellHandle shell)
{
    fprintf(stdout, "# SHELL\n");
    fprintf(stdout, "This function has not yet been implemented\n");
    BSTD_UNUSED(shell);
    /* TODO */
}
