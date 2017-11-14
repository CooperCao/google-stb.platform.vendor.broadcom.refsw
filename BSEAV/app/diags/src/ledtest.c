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
 *
 * Module Description:
 *
  *****************************************************************************/
#include <stdio.h>
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_led.h"
#include "bstd.h"
#include <string.h>
#include "prompt.h"

BDBG_MODULE(diags_led);

#if (BCHP_CHIP==7271) || (BCHP_CHIP==7268)
#define PINMUX_72XX 1
#endif

#if PINMUX_72XX
#include "bchp_aon_pin_ctrl.h"
#include "bchp_ldk.h"
#endif

static const char *g_message[] = {
    "this",
    "is",
    "a",
    "brcm",
    "chip",
    ""
};

void bcmLedTest(void)
{
    NEXUS_Error rc;
    NEXUS_LedHandle led;
    NEXUS_LedSettings settings;
    unsigned i;

#if PINMUX_72XX
    uint32_t aon_pin_mux_ctrl[5] = {0};
    uint32_t reg;
    NEXUS_Platform_ReadRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, &aon_pin_mux_ctrl[0]);
    NEXUS_Platform_ReadRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, &aon_pin_mux_ctrl[1]);
    NEXUS_Platform_ReadRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, &aon_pin_mux_ctrl[2]);
    NEXUS_Platform_ReadRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, &aon_pin_mux_ctrl[3]);
    NEXUS_Platform_ReadRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_4, &aon_pin_mux_ctrl[4]);

    for (i=0; i < 5; i++) {
        BDBG_MSG(("PIN_MUX_CTRL_%d: %08x", i, aon_pin_mux_ctrl[i]));
    }

    reg = aon_pin_mux_ctrl[0];
    reg &= ~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07)
        );

    reg |= (
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07, 1)
        );
    NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    reg = aon_pin_mux_ctrl[1];
    reg &= ~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15)
        );

    reg |= (
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15, 1)
        );
    NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

    reg = aon_pin_mux_ctrl[2];
    reg &= ~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17)
        );

    reg |= (
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 1)
        );
    NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);

    reg = aon_pin_mux_ctrl[3];
    reg &= ~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_18) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_19) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_20)
        );

    reg |= (
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_18, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_19, 1) |
            BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_20, 3)
        );
    NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);

    for (i=0; i < 5; i++) {
        NEXUS_Platform_ReadRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0 + 4*i, &reg);
        BDBG_MSG(("PIN_MUX_CTRL_%d: %08x", i, reg));
    }
#endif

    NEXUS_Led_GetDefaultSettings(&settings);
    settings.scrollingEnabled = true;
    led = NEXUS_Led_Open(0, &settings);

    while (1)
    {
        printf("\n\n");
        printf("=================\n");
        printf("  LED Test Menu  \n");
        printf("=================\n");
        printf("    0) Exit\n");
        printf("    1) Send text string\n");
        printf("    2) Set dots\n");
        printf("    3) Set digits\n");
        printf("    4) Clear digits\n");
        printf("    5) Set LEDs\n");
        printf("    6) Clear LEDs\n");

        switch(Prompt()) {
            case 0:
#if PINMUX_72XX
                NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_pin_mux_ctrl[0]);
                NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_pin_mux_ctrl[1]);
                NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_pin_mux_ctrl[2]);
                NEXUS_Platform_WriteRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_pin_mux_ctrl[3]);

                for (i=0; i < 5; i++) {
                    NEXUS_Platform_ReadRegister(BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0 + 4*i, &reg);
                    BDBG_MSG(("PIN_MUX_CTRL_%d: %08x", i, reg));
                }
#endif

                return;

            case 1:
                for (i=0;g_message[i][0];i++) {
                    rc = NEXUS_Led_WriteString(led, g_message[i], 0);
                    BDBG_ASSERT(!rc);
                    BKNI_Sleep(500);
                }
                break;

            case 2:
                for (i=0;i<20;i++) {
                    rc = NEXUS_Led_SetDot(led, i%4, (i/4)%2);
                    BDBG_ASSERT(!rc);
                    BKNI_Sleep(100);
                }
                break;

            case 3:
                NEXUS_Led_WriteSegments(led, 1, 0xff);
                NEXUS_Led_WriteSegments(led, 2, 0xff);
                NEXUS_Led_WriteSegments(led, 3, 0xff);
                NEXUS_Led_WriteSegments(led, 4, 0xff);
                break;

            case 4:
                NEXUS_Led_WriteSegments(led, 1, 0);
                NEXUS_Led_WriteSegments(led, 2, 0);
                NEXUS_Led_WriteSegments(led, 3, 0);
                NEXUS_Led_WriteSegments(led, 4, 0);
                break;

            case 5:
                NEXUS_Led_SetLedState(led, 0, true);
                NEXUS_Led_SetLedState(led, 1, true);
                NEXUS_Led_SetLedState(led, 2, true);
                NEXUS_Led_SetLedState(led, 3, true);
                break;

            case 6:
                NEXUS_Led_SetLedState(led, 0, false);
                NEXUS_Led_SetLedState(led, 1, false);
                NEXUS_Led_SetLedState(led, 2, false);
                NEXUS_Led_SetLedState(led, 3, false);
                break;

            default:
                break;
        }
    }
}
