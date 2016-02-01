/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
 *****************************************************************************/

#include "nexus_platform.h"
#if NEXUS_HAS_INPUT_ROUTER
#include "nexus_input_client.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(input_client);

#if NEXUS_HAS_INPUT_ROUTER

static void callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void print_usage(void)
{
    printf(
    "usage: nexus input_client [--help|-h] [-device {ir|mouse|kbd|keypad}]\n"
    "options:\n"
    "  --help|-h    print this help screen\n"
    "  -device XX   specify which device you want receive. you can request multiple. by default, you get all.\n"
    );
}

int main(int argc, char **argv)
{
    NEXUS_InputClientHandle client;
    NEXUS_InputClientSettings settings;
    NEXUS_Error rc;
    unsigned client_id = 0;
    BKNI_EventHandle event;
    int curarg = 1;
    uint32_t filter = 0;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-device") && curarg+1 < argc) {
            curarg++;
            if (!strcmp(argv[curarg], "ir")) {
                filter |= 1<<NEXUS_InputRouterDevice_eIrInput;
            }
            else if (!strcmp(argv[curarg], "evdev")) {
                filter |= 1<<NEXUS_InputRouterDevice_eEvdev;
            }
            else if (!strcmp(argv[curarg], "keypad")) {
                filter |= 1<<NEXUS_InputRouterDevice_eKeypad;
            }
        }
        else {
            client_id = atoi(argv[1]);
        }
        curarg++;
    }

    rc = NEXUS_Platform_Join();
    if (rc) {
        return -1;
    }
    
    rc = BKNI_CreateEvent(&event);
    BDBG_ASSERT(!rc);

    client = NEXUS_InputClient_Acquire(client_id);
    BDBG_ASSERT(client);

    NEXUS_InputClient_GetSettings(client, &settings);
    if (filter) {
        settings.filterMask = filter; /* only what's requested */
    }
    else {
        settings.filterMask = 0xFFFFFFFF; /* everything */
    }
    settings.codeAvailable.callback = callback;
    settings.codeAvailable.context = event;
    rc = NEXUS_InputClient_SetSettings(client, &settings);
    BDBG_ASSERT(!rc);

    while (1) {
        NEXUS_InputRouterCode code;
        unsigned num;
        
        rc = NEXUS_InputClient_GetCodes(client, &code, 1, &num);
        BDBG_ASSERT(!rc);
        if (num == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }
        
        printf("code type %d: ", code.deviceType);
        switch (code.deviceType) {
        case NEXUS_InputRouterDevice_eIrInput:
            printf("ir %#x\n", code.data.irInput.code);
            break;
        case NEXUS_InputRouterDevice_eKeypad:
            printf("keypad %#x\n", code.data.keypad.code);
            break;
        case NEXUS_InputRouterDevice_eEvdev:
            printf("evdev type: %d, code: %d, value: %d\n", code.data.evdev.type, code.data.evdev.code, code.data.evdev.value);
            break;
        case NEXUS_InputRouterDevice_eUhfInput:
            /* TODO */
            break;
        case NEXUS_InputRouterDevice_eBluetoothRemote:
            /* TODO */
            break;
        default:
            printf("unknown deviceType %d\n", code.deviceType);
            break;
        }
    }

    NEXUS_InputClient_Release(client);
    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void)
{
    printf("ERROR: you must include input_router.inc in platform_modules.inc\n");
    return -1;
}
#endif
