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

#ifndef DYNRNG_SHELL_PRIV_H__
#define DYNRNG_SHELL_PRIV_H__

#include "blst_queue.h"

#define MAX_INPUT_LENGTH 256

typedef struct SHELL_CommandAlias
{
    BLST_Q_ENTRY(SHELL_CommandAlias) link;
    char * spelling;
} SHELL_CommandAlias;

typedef BLST_Q_HEAD(SHELL_CommandAliasList, SHELL_CommandAlias) SHELL_CommandAliasList;

typedef struct SHELL_CommandArg
{
    BLST_Q_ENTRY(SHELL_CommandArg) link;
    char * spelling;
    bool required;
    char * help;
} SHELL_CommandArg;

typedef BLST_Q_HEAD(SHELL_CommandArgList, SHELL_CommandArg) SHELL_CommandArgList;

struct SHELL_Command
{
    BLST_Q_ENTRY(SHELL_Command) link;
    SHELL_CommandAliasList aliases;
    SHELL_CommandHandler handler;
    void * context;
    char * help;
    char * syntax;
    SHELL_CommandArgList args;
};

typedef BLST_Q_HEAD(SHELL_CommandList, SHELL_Command) SHELL_CommandList;

struct SHELL_Shell
{
    SHELL_CommandList commands;
    bool stop;
};

bool SHELL_CommandRequiresArgs(SHELL_CommandHandle command);
SHELL_CommandHandle SHELL_FindCommandByName(SHELL_ShellHandle shell, const char * name);
void SHELL_DestroyCommand(SHELL_CommandHandle command);
SHELL_CommandAlias * SHELL_CreateCommandAlias(const char * alias);
void SHELL_DestroyCommandAlias(SHELL_CommandAlias * pAlias);
SHELL_CommandArg * SHELL_CreateCommandArg(const char * spelling, bool required, const char * help);
void SHELL_DestroyCommandArg(SHELL_CommandArg * pArg);
void SHELL_PrintCommand(SHELL_CommandHandle command);
void SHELL_PrintCommandArg(SHELL_CommandArg * pArg);
void SHELL_PrintCommandAlias(SHELL_CommandAlias * pAlias);

#endif /* DYNRNG_SHELL_PRIV_H__ */
