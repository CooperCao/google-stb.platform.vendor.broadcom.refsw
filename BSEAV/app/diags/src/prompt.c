/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:  Generic prompt for Diagnostics S/W.
 *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#if __cplusplus
}
#endif

/***********************************************************************
 *
 *        Prompt: Display prompt, wait for user input entry, return an
 *        integer.
 *
 ***********************************************************************/
int Prompt(void)
{
    char buf[256];
    printf("\nChoice: ");
    fgets(buf, 256, stdin);
    return atoi(buf);
}

/***********************************************************************
 *
 *        NoPrompt: Wait for user input entry, return an integer.
 *
 ***********************************************************************/
int NoPrompt(void)
{
    char buf[256];
    fgets(buf, 256, stdin);
    return atoi(buf);
}

/***********************************************************************
 *
 *        NoPromptChar: Wait for user input entry in hex, return an
 *        integer.
 *
 ***********************************************************************/
int NoPromptHex(void)
{
    char buf[256];
    fgets(buf, 256, stdin);
    return (strtol(buf, NULL, 0));
}

/***********************************************************************
 *
 *        PromptChar: Display prompt, wait for user input entry,
 *        return a character.
 *
 ***********************************************************************/
char PromptChar( void )
{
    char buf[256];
    printf("\nChoice: ");
    fgets(buf, 256, stdin);
    return buf[0];
}

/***********************************************************************
 *
 *        NoPromptChar: Wait for user input entry, return a character.
 *
 ***********************************************************************/
char NoPromptChar( void )
{
    char buf[256];
    fgets(buf, 256, stdin);
    return buf[0];
}

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
int kbhit(void)
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); /* STDIN_FILENO is 0 */
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
