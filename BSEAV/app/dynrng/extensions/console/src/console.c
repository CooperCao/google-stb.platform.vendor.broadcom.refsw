/******************************************************************************
 * Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "platform.h"
#include "console_priv.h"
#include "util.h"
#include "error.h"
#include "string_list.h"
#include <linenoise.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

static const char * CONSOLE_HISTORY_FILENAME = "history.txt";
static const unsigned CONSOLE_HISTORY_MAX_LEN = 100;

static const char * gConsoleHelp =
{
    "\nConsole Help:\n"
    "  w/s - up/down\n"
    "  a/d - left/right\n"
    "  x   - select\n"
    "  p   - pause\n"
    "  0-9 - number\n"
    "  q   - quit\n"
};

static void * console_p_thread(void * ctx)
{
    ConsoleHandle console = ctx;
    console_p_run(console);
    return NULL;
}

static int console_p_tty_cbreak(int fd)
{
    struct termios ios;

    if (tcgetattr(fd, &ios) < 0)
    {
        return(-1);
    }

    ios.c_lflag    &= ~(ICANON | ECHO);
    ios.c_cc[VMIN]  = 1;
    ios.c_cc[VTIME] = 0;

    return(tcsetattr(fd, TCSAFLUSH, &ios));
} /* tty_cbreak */

static int console_p_tty_reset(int fd)
{
    struct termios ios;

    if (tcgetattr(fd, &ios) < 0)
    {
        return(-1);
    }

    ios.c_lflag    |= (ICANON | ECHO);
    ios.c_cc[VMIN]  = 1;
    ios.c_cc[VTIME] = 0;

    return(tcsetattr(fd, TCSANOW, &ios));
} /* tty_reset */

static int console_p_kbdhit(void)
{
    fd_set         rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    return(select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) && FD_ISSET(STDIN_FILENO, &rfds));
}

static StringListHandle g_console_p_lineCompletionList;

void console_p_tab_completion_handler(const char * buf, linenoiseCompletions * lc)
{
    if (!isalpha(buf[0]))
    {
        return;
    }

    if (g_console_p_lineCompletionList)
    {
        StringListCursorHandle slc = string_list_create_cursor(g_console_p_lineCompletionList);
        const char * str = NULL;
        size_t buflen = strlen(buf);
        for (str = string_list_cursor_first(slc); str; str = string_list_cursor_next(slc))
        {
            /* only add autocomplete choices that match current buffer */
            if (!strncmp(str, buf, buflen))
            {
                /* only add autocomplete choices that aren't exactly the same as given buffer */
                if (strlen(str) != buflen)
                {
                    linenoiseAddCompletion(lc, (char *)str);
                }
            }
        }
        string_list_cursor_destroy(slc);
    }
}

void console_p_run(ConsoleHandle console)
{
    PlatformInputEvent event;
    char * linebuf;
    char * line;
    int result = 0;
    static const char * prompt = "[dynrng]$ ";

    while (!console->done)
    {
        fprintf(stdout, "\r%s", prompt);
        fflush(stdout);
        usleep(200000); /* 200 ms */

        /* non-blocking check for key hit but do not read - linenoise will read the value after first
         * changing the terminal settings */
        if (console_p_kbdhit())
        {
            linebuf = linenoise(prompt);
            if (linebuf)
            {
                line = trim(linebuf);
                linenoiseHistoryAdd(line);
                event = PlatformInputEvent_eUnknown;

                if (line[0] == 0) continue;

                if (line[1] == 0)
                {
                    switch (line[0])
                    {
                        case 'w':
                            event = PlatformInputEvent_eUp;
                            break;
                        case 's':
                            event = PlatformInputEvent_eDown;
                            break;
                        case 'd':
                            event = PlatformInputEvent_eRight;
                            break;
                        case 'a':
                            event = PlatformInputEvent_eLeft;
                            break;
                        case 'p':
                            event = PlatformInputEvent_ePause;
                            break;
                        case 'x':
                            event = PlatformInputEvent_eSelect;
                            break;
                        case 'q':
                            event = PlatformInputEvent_ePower;
                            break;
                        case 'h':
                            fprintf(stdout, "%s\n", gConsoleHelp);
                            continue;
                            break;
                        default:
                            break;
                    }
                }
                if (event == PlatformInputEvent_eUnknown)
                {
                    if (console->createSettings.unrecognizedSyntax.callback)
                    {
                        result = console->createSettings.unrecognizedSyntax.callback(console->createSettings.unrecognizedSyntax.context, line);
                        if (result == -ERR_NOT_FOUND)
                        {
                            printf("unrecognized console syntax '%s'\n", line);
                        }
                    }
                }
                else
                {
                    if (console->createSettings.inputEvent.callback)
                    {
                        console->createSettings.inputEvent.callback(console->createSettings.inputEvent.context, event, 0);
                    }
                }
                free(linebuf);
            }
        }
    }
}

void console_get_default_create_settings(ConsoleCreateSettings * pSettings)
{
    if (pSettings)
    {
        memset(pSettings, 0, sizeof(*pSettings));
    }
}

ConsoleHandle console_create(const ConsoleCreateSettings * pSettings)
{
    ConsoleHandle console;

    console = malloc(sizeof(*console));
    if (!console) goto error;
    memset(console, 0, sizeof(*console));
    memcpy(&console->createSettings, pSettings, sizeof(*pSettings));

    console_p_tty_cbreak(STDIN_FILENO);

    if (pSettings->tabCompletionList)
    {
        g_console_p_lineCompletionList = pSettings->tabCompletionList;
    }

    linenoiseHistorySetMaxLen(CONSOLE_HISTORY_MAX_LEN);
    linenoiseHistoryLoad((char *)CONSOLE_HISTORY_FILENAME);
    linenoiseSetCompletionCallback(console_p_tab_completion_handler);
    /*linenoiseSetMultiLine(1);*/

    return console;

error:
    console_destroy(console);
    return NULL;
}

void console_destroy(ConsoleHandle console)
{
    if (!console) return;
    console_p_tty_reset(STDIN_FILENO);
    linenoiseSetCompletionCallback(NULL);
    g_console_p_lineCompletionList = NULL;
    linenoiseHistorySave((char *)CONSOLE_HISTORY_FILENAME);
    free(console);
}

int console_start(ConsoleHandle console)
{
    assert(console);
    return pthread_create(&console->thread, NULL, &console_p_thread, console);
}

void console_stop(ConsoleHandle console)
{
    assert(console);
    console->done = true;
    if (console->thread)
    {
        pthread_cancel(console->thread);
        pthread_join(console->thread, NULL);
    }
}
