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
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_input_priv.h"
#include "bdbg.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>

BDBG_MODULE(platform);

static const char * eventNames[] =
{
    "unknown",
    "quit",
    "toggle osd",
    "toggle guide",
    "toggle output dynrng lock",
    "cycle colorimetry",
    "cycle output dynrng",
    "next thumbnail",
    "prev thumbnail",
    "cycle background",
    "next video setting",
    "prev video setting",
    "next graphics setting",
    "prev graphics setting",
    "next stream",
    "prev stream",
    "toggle pause",
    "toggle pig",
    "toggle details",
    "scenario 1",
    "scenario 2",
    "scenario 3",
    "scenario 4",
    "scenario 5",
    "scenario 6",
    "scenario 7",
    "scenario 8",
    "scenario 9",
    "scenario 0",
    "start shell",
    NULL
};

static const char * consoleHelp =
{
    "\nConsole Help:\n"
    "  w/s - next/prev video setting\n"
    "  a/d - next/prev graphics setting\n"
    "  z/c - next/prev stream\n"
    "  f/r - next/prev thumbnail\n"
    "  p   - toggle pause\n"
    "  ?   - toggle guide\n"
    "  e   - toggle osd\n"
    "  i   - toggle details\n"
    "  g   - toggle pig\n"
    "  x   - cycle output dynrng\n"
    "  o   - cycle colorimetry\n"
    "  .   - cycle background\n"
    "  0-9 - switch to scenario 0-9\n"
    "  q   - toggle output dynrng lock\n"
    "  `   - start shell\n"
};

#define MAX_LINE_LEN 256
bool platform_input_p_get_console_input(PlatformInputHandle input, PlatformInputEvent * pEvent, int * pParam)
{
    char line[MAX_LINE_LEN];

    BSTD_UNUSED(input);

    fprintf(stdout, "%s\n", consoleHelp);
    fprintf(stdout, "[dynrng]$ ");
    fflush(stdout);
    if (fgets(line, MAX_LINE_LEN, stdin))
    {
        switch (line[0])
        {
            case 'w':
                *pEvent = PlatformInputEvent_eNextVideoSetting;
                break;
            case 's':
                *pEvent = PlatformInputEvent_ePrevVideoSetting;
                break;
            case 'd':
                *pEvent = PlatformInputEvent_eNextGraphicsSetting;
                break;
            case 'a':
                *pEvent = PlatformInputEvent_ePrevGraphicsSetting;
                break;
            case 'z':
                *pEvent = PlatformInputEvent_eNextStream;
                break;
            case 'c':
                *pEvent = PlatformInputEvent_ePrevStream;
                break;
            case 'p':
                *pEvent = PlatformInputEvent_eTogglePause;
                break;
            case '?':
                *pEvent = PlatformInputEvent_eToggleGuide;
                break;
            case 'i':
                *pEvent = PlatformInputEvent_eToggleDetails;
                break;
            case 'e':
                *pEvent = PlatformInputEvent_eToggleOsd;
                break;
            case 'g':
                *pEvent = PlatformInputEvent_eTogglePig;
                break;
            case 'f':
                *pEvent = PlatformInputEvent_eNextThumbnail;
                break;
            case 'r':
                *pEvent = PlatformInputEvent_ePrevThumbnail;
                break;
            case 'x':
                *pEvent = PlatformInputEvent_eCycleOutputDynamicRange;
                break;
            case 'o':
                *pEvent = PlatformInputEvent_eCycleColorimetry;
                break;
            case '.':
                *pEvent = PlatformInputEvent_eCycleBackground;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                *pEvent = PlatformInputEvent_eScenario;
                *pParam = atoi(line);
                break;
            case 'q':
                *pEvent = PlatformInputEvent_eQuit;
                break;
            case 'l':
                *pEvent = PlatformInputEvent_eToggleOutputDynamicRangeLock;
                break;
            case '`':
                *pEvent = PlatformInputEvent_eStartCommandShell;
                break;
            default:
                *pEvent = PlatformInputEvent_eUnknown;
                break;
        }
    }
    return true;
}

PlatformInputHandle platform_input_open(PlatformHandle platform, PlatformInputMethod method)
{
    PlatformInputHandle input;

    input = BKNI_Malloc(sizeof(*input));
    BDBG_ASSERT(input);
    BKNI_Memset(input, 0, sizeof(*input));
    platform->input = input;
    input->platform = platform;
    input->method = method;
    input->nxInput = binput_open(NULL);
    BDBG_ASSERT(input->nxInput);
    return input;
}

void platform_input_close(PlatformInputHandle input)
{
    if (!input) return;
    input->platform->input = NULL;
    binput_close(input->nxInput);
    BKNI_Free(input);
}

PlatformInputEvent platform_input_p_event_from_key(b_remote_key key, int * pParam)
{
    PlatformInputEvent event;

    switch (key)
    {
        case b_remote_key_up:
            event = PlatformInputEvent_eNextVideoSetting;
            break;
        case b_remote_key_down:
            event = PlatformInputEvent_ePrevVideoSetting;
            break;
        case b_remote_key_right:
            event = PlatformInputEvent_eNextGraphicsSetting;
            break;
        case b_remote_key_left:
            event = PlatformInputEvent_ePrevGraphicsSetting;
            break;
        case b_remote_key_chan_up:
            event = PlatformInputEvent_eNextStream;
            break;
        case b_remote_key_chan_down:
            event = PlatformInputEvent_ePrevStream;
            break;
        case b_remote_key_pause:
        case b_remote_key_play:
            event = PlatformInputEvent_eTogglePause;
            break;
        case b_remote_key_select:
            event = PlatformInputEvent_eToggleGuide;
            break;
        case b_remote_key_info:
            event = PlatformInputEvent_eToggleDetails;
            break;
        case b_remote_key_guide:
            event = PlatformInputEvent_eTogglePig;
            break;
        case b_remote_key_menu:
            event = PlatformInputEvent_eToggleMosaicLayout;
            break;
        case b_remote_key_clear:
            event = PlatformInputEvent_eToggleOsd;
            break;
        case b_remote_key_fast_forward:
            event = PlatformInputEvent_eNextThumbnail;
            break;
        case b_remote_key_rewind:
            event = PlatformInputEvent_ePrevThumbnail;
            break;
        case b_remote_key_back:
            event = PlatformInputEvent_eCycleOutputDynamicRange;
            break;
        case b_remote_key_power:
            event = PlatformInputEvent_eToggleOutputDynamicRangeLock;
            break;
        case b_remote_key_stop:
            event = PlatformInputEvent_eCycleColorimetry;
            break;
        case b_remote_key_dot:
            event = PlatformInputEvent_eCycleBackground;
            break;
        case b_remote_key_one:
        case b_remote_key_two:
        case b_remote_key_three:
        case b_remote_key_four:
        case b_remote_key_five:
        case b_remote_key_six:
        case b_remote_key_seven:
        case b_remote_key_eight:
        case b_remote_key_nine:
        case b_remote_key_zero:
            event = PlatformInputEvent_eScenario;
            *pParam = (key - b_remote_key_one) + 1;
            break;
        default:
            event = PlatformInputEvent_eUnknown;
            break;
    }

    return event;
}

bool platform_input_p_get_remote_input(PlatformInputHandle input, PlatformInputEvent * pEvent, int * pParam)
{
    bool occurred = true;
    b_remote_key key;

    if (binput_read_no_repeat(input->nxInput, &key)) {
        binput_wait(input->nxInput, 1000);
        occurred = false;
    }
    else
    {
        *pEvent = platform_input_p_event_from_key(key, pParam);
    }

    return occurred;
}

bool platform_input_try(PlatformInputHandle input)
{
    bool occurred = true;
    PlatformInputEvent event;
    int param;

    BDBG_ASSERT(input);

    switch (input->method)
    {
        case PlatformInputMethod_eConsole:
            occurred = platform_input_p_get_console_input(input, &event, &param);
            if (!occurred) goto out;
            break;
        default:
        case PlatformInputMethod_eRemote:
            occurred = platform_input_p_get_remote_input(input, &event, &param);
            if (!occurred) goto out;
            break;
    }
    if (event == PlatformInputEvent_eUnknown) {
        occurred = false;
        goto out;
    }
    BDBG_MSG(("received '%s' event", eventNames[event]));
    if (input->eventHandlers[event].callback)
    {
        input->eventHandlers[event].callback(input->eventHandlers[event].context, param);
    }
    else
    {
        occurred = false;
    }

out:
    return occurred;
}

void platform_input_set_event_handler(PlatformInputHandle input, PlatformInputEvent event, PlatformCallback callback, void * callbackContext)
{
    BDBG_ASSERT(input);
    BDBG_ASSERT(event < PlatformInputEvent_eMax);
    input->eventHandlers[event].callback = callback;
    input->eventHandlers[event].context = callbackContext;
}
