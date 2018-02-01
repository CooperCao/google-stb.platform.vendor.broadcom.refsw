/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
    "pause",
    "up",
    "down",
    "right",
    "left",
    "select",
    "quit",
    "channel up",
    "channel down",
    "number",
    "info",
    "menu",
    NULL
};

PlatformInputHandle platform_input_open(PlatformHandle platform)
{
    PlatformInputHandle input;

    input = BKNI_Malloc(sizeof(*input));
    BDBG_ASSERT(input);
    BKNI_Memset(input, 0, sizeof(*input));
    platform->input = input;
    input->platform = platform;
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
            event = PlatformInputEvent_eUp;
            break;
        case b_remote_key_down:
            event = PlatformInputEvent_eDown;
            break;
        case b_remote_key_right:
            event = PlatformInputEvent_eRight;
            break;
        case b_remote_key_left:
            event = PlatformInputEvent_eLeft;
            break;
        case b_remote_key_chan_up:
            event = PlatformInputEvent_eChannelUp;
            break;
        case b_remote_key_chan_down:
            event = PlatformInputEvent_eChannelDown;
            break;
        case b_remote_key_pause:
            event = PlatformInputEvent_ePause;
            break;
        case b_remote_key_select:
            event = PlatformInputEvent_eSelect;
            break;
        case b_remote_key_info:
            event = PlatformInputEvent_eInfo;
            break;
        case b_remote_key_menu:
            event = PlatformInputEvent_eMenu;
            break;
        case b_remote_key_clear:
            event = PlatformInputEvent_eExit;
            break;
#if 0
        case b_remote_key_power:
            event = PlatformInputEvent_ePower;
            break;
#endif
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
            event = PlatformInputEvent_eNumber;
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

    occurred = platform_input_p_get_remote_input(input, &event, &param);
    if (!occurred) goto out;

    if (event == PlatformInputEvent_eUnknown) {
        occurred = false;
        goto out;
    }
    BDBG_MSG(("received '%s' event", eventNames[event]));
    if (input->eventHandler.callback)
    {
        input->eventHandler.callback(input->eventHandler.context, event, param);
    }
    else
    {
        occurred = false;
    }

out:
    return occurred;
}

void platform_input_set_event_handler(PlatformInputHandle input, PlatformInputEventCallback callback, void * callbackContext)
{
    BDBG_ASSERT(input);
    input->eventHandler.callback = callback;
    input->eventHandler.context = callbackContext;
}
