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

#ifndef SHELL_PRIV_H__
#define SHELL_PRIV_H__

#include "blst_queue.h"

#define MAX_INPUT_LENGTH 256

typedef struct CommandAlias
{
    BLST_Q_ENTRY(CommandAlias) link;
    char * spelling;
} CommandAlias;

typedef BLST_Q_HEAD(CommandAliasList, CommandAlias) CommandAliasList;

typedef struct CommandArg
{
    BLST_Q_ENTRY(CommandArg) link;
    char * spelling;
    bool required;
    char * help;
} CommandArg;

typedef BLST_Q_HEAD(CommandArgList, CommandArg) CommandArgList;

struct Command
{
    BLST_Q_ENTRY(Command) link;
    CommandAliasList aliases;
    CommandHandler handler;
    void * context;
    char * help;
    char * syntax;
    CommandArgList args;
};

typedef BLST_Q_HEAD(CommandList, Command) CommandList;

struct Shell
{
    CommandList commands;
    bool stop;
};

bool shell_p_command_requires_args(CommandHandle command);
CommandHandle shell_p_find_command_by_name(ShellHandle shell, const char * name);
void shell_p_destroy_command(CommandHandle command);
CommandAlias * shell_p_create_command_alias(const char * alias);
void shell_p_destroy_command_alias(CommandAlias * pAlias);
CommandArg * shell_p_create_command_arg(const char * spelling, bool required, const char * help);
void shell_p_destroy_command_arg(CommandArg * pArg);
void shell_p_print_command(CommandHandle command);
void shell_p_print_command_arg(CommandArg * pArg);
void shell_p_print_command_alias(CommandAlias * pAlias);

#endif /* SHELL_PRIV_H__ */
