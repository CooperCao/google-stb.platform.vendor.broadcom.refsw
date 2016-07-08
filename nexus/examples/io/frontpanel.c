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
#include <stdio.h>
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_led.h"
#if NEXUS_HAS_IR_INPUT
#include "nexus_ir_input.h"
#endif
#if NEXUS_HAS_KEYPAD
#include "nexus_keypad.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include <string.h>

#if NEXUS_HAS_IR_INPUT || NEXUS_HAS_KEYPAD
static void dataready(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}
#endif

static const char *g_message[] = {
    "this",
    "is",
    "a",
    "brcm",
    "chip",
    ""
};

int main(int argc, char **argv)
{
    NEXUS_Error rc;
    NEXUS_LedHandle led;
    NEXUS_LedSettings ledSettings;
#if NEXUS_HAS_IR_INPUT
    NEXUS_IrInputHandle irInput;
    NEXUS_IrInputSettings irInputSettings;
#endif
#if NEXUS_HAS_KEYPAD
    NEXUS_KeypadHandle keypad;
    NEXUS_KeypadSettings keypadSettings;
#endif
    BKNI_EventHandle event;
    NEXUS_PlatformSettings platformSettings;
    unsigned i;
    int curarg = 1;
    bool silver = false;
    
    while (curarg < argc) {
        if (!strcmp("-silver", argv[curarg])) {
            silver = true;
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    BKNI_CreateEvent(&event);

    led = NEXUS_Led_Open(0, NULL);

    /* initial greeting */
    for (i=0;g_message[i][0];i++) {
        rc = NEXUS_Led_WriteString(led, g_message[i], 0);
        BDBG_ASSERT(!rc);
        BKNI_Sleep(500);
    }

    for (i=0;i<20;i++) {
        rc = NEXUS_Led_SetDot(led, i%4, (i/4)%2);
        BDBG_ASSERT(!rc);
        BKNI_Sleep(100);
    }

    /* test scrolling */
    NEXUS_Led_GetSettings(led, &ledSettings);
    ledSettings.scrollingEnabled = true;
    NEXUS_Led_SetSettings(led, &ledSettings);
    rc = NEXUS_Led_WriteString(led, "press front panel key", 0);
    BDBG_ASSERT(!rc);

    NEXUS_Led_SetLedState(led, 0, true);
    NEXUS_Led_SetLedState(led, 1, true);
    NEXUS_Led_SetLedState(led, 2, true);

#if NEXUS_HAS_IR_INPUT
    NEXUS_IrInput_GetDefaultSettings(&irInputSettings);
    irInputSettings.dataReady.callback = dataready;
    irInputSettings.dataReady.context = event;
    irInputSettings.mode = silver?NEXUS_IrInputMode_eCirNec:NEXUS_IrInputMode_eRemoteA;
    irInput = NEXUS_IrInput_Open(0, &irInputSettings);
#endif
#if NEXUS_HAS_KEYPAD
    NEXUS_Keypad_GetDefaultSettings(&keypadSettings);
    keypadSettings.dataReady.callback = dataready;
    keypadSettings.dataReady.context = event;
    keypad = NEXUS_Keypad_Open(0, &keypadSettings);
#endif

    printf("waiting for IR input or Keypad...\n");
    for (i=0;;i++) {
#if NEXUS_HAS_IR_INPUT || NEXUS_HAS_KEYPAD
        NEXUS_IrInputEvent irEvent;
        size_t num;
        bool overflow;
#endif

        /* check each input */
#if NEXUS_HAS_IR_INPUT
        rc = NEXUS_IrInput_GetEvents(irInput, &irEvent, 1, &num, &overflow);
        BDBG_ASSERT(!rc);
        if (num) {
            printf("ir event: %#x %s\n", irEvent.code, irEvent.repeat?"repeat":"");
            continue;
        }
#endif
#if NEXUS_HAS_KEYPAD
        {
        NEXUS_KeypadEvent keypadEvent;
        rc = NEXUS_Keypad_GetEvents(keypad, &keypadEvent, 1, &num, &overflow);
        BDBG_ASSERT(!rc);
        if (num) {
            printf("keypad event: %#x %s\n", keypadEvent.code, keypadEvent.repeat?"repeat":"");
            continue;
        }
        }
#endif

        BKNI_WaitForEvent(event, 500);

        NEXUS_Led_SetLedState(led, 0, i%4==0);
        NEXUS_Led_SetLedState(led, 1, i%4==1);
        NEXUS_Led_SetLedState(led, 2, i%4==2);
        NEXUS_Led_SetLedState(led, 3, i%4==3);
    }

    NEXUS_Platform_Uninit();
    return 0;
}
