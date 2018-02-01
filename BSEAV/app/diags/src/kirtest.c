/******************************************************************************
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
 * Module Description:
 *
 *****************************************************************************/
/* Example to get IR remote input using nexus */

#include "nexus_platform.h"
#include "nexus_ir_input.h"
#include <assert.h>
#include <stdio.h>
#include "prompt.h"

#if (BCHP_CHIP==7278)
#include "bchp_sun_top_ctrl.h"
#endif

/*#define COMPATIBILITY_32_BIT_MODE*/

BDBG_MODULE(diags_kir);

static int get_number(void)
{
    char buf[256];
    fgets(buf, 256, stdin);
    return atoi(buf);
}

void irCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;
    NEXUS_IrInputHandle irHandle = *(NEXUS_IrInputHandle *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        NEXUS_IrInputEvent irEvent;
        rc = NEXUS_IrInput_GetEvents(irHandle,&irEvent,1,&numEvents,&overflow);
        if (numEvents)
            printf("irCallback: rc: %d, code: %08x, repeat: %s\n", rc, irEvent.code, irEvent.repeat ? "true" : "false");
    }
}

static NEXUS_IrInputMode bcmKirMode(void)
{
    NEXUS_IrInputMode mode;
    printf("Enter remote type:\n");
    printf("%2d Twirp\n", NEXUS_IrInputMode_eTwirpKbd);
    printf("%2d Sejin38Khz\n", NEXUS_IrInputMode_eSejin38KhzKbd);
    printf("%2d Sejin56Khz\n", NEXUS_IrInputMode_eSejin56KhzKbd);
    printf("%2d RemoteA\n", NEXUS_IrInputMode_eRemoteA);
    printf("%2d RemoteB\n", NEXUS_IrInputMode_eRemoteB);
    printf("%2d CirGI\n", NEXUS_IrInputMode_eCirGI);
    printf("%2d CirSaE2050\n", NEXUS_IrInputMode_eCirSaE2050);
    printf("%2d CirTwirp\n", NEXUS_IrInputMode_eCirTwirp);
    printf("%2d CirSony\n", NEXUS_IrInputMode_eCirSony);
    printf("%2d CirRecs80\n", NEXUS_IrInputMode_eCirRecs80);
    printf("%2d CirRc5\n", NEXUS_IrInputMode_eCirRc5);
    printf("%2d CirUei\n", NEXUS_IrInputMode_eCirUei);
    printf("%2d CirRfUei\n", NEXUS_IrInputMode_eCirRfUei);
    printf("%2d CirEchoStar\n", NEXUS_IrInputMode_eCirEchoStar);
    printf("%2d SonySejin\n", NEXUS_IrInputMode_eSonySejin);
    printf("%2d CirNec\n", NEXUS_IrInputMode_eCirNec);
    printf("%2d CirRC6\n", NEXUS_IrInputMode_eCirRC6);
    printf("%2d CirGISat\n", NEXUS_IrInputMode_eCirGISat);
    printf("%2d Custom\n", NEXUS_IrInputMode_eCustom);
    printf("%2d CirDirectvUhfr\n", NEXUS_IrInputMode_eCirDirectvUhfr);
    printf("%2d CirEchostarUhfr\n", NEXUS_IrInputMode_eCirEchostarUhfr);
    printf("%2d CirRcmmRcu\n", NEXUS_IrInputMode_eCirRcmmRcu);
    printf("%2d CirRste\n", NEXUS_IrInputMode_eCirRstep);
    printf("%2d CirXmp\n", NEXUS_IrInputMode_eCirXmp);
    printf("%2d CirXmp2Ack\n", NEXUS_IrInputMode_eCirXmp2Ack);
    printf("%2d CirRC6Mode0\n", NEXUS_IrInputMode_eCirRC6Mode0);
    printf("%2d CirRca\n", NEXUS_IrInputMode_eCirRca);
    printf("%2d CirToshibaTC9012\n", NEXUS_IrInputMode_eCirToshibaTC9012);
    printf("Choice:  ");
    mode = get_number();
    return mode;
}

static void bcmKirStartFiltering(NEXUS_IrInputHandle irHandle)
{
    NEXUS_IrInputDataFilter pattern;
    uint64_t data0, mask0;

#ifndef COMPATIBILITY_32_BIT_MODE
    uint64_t data1, mask1;
#endif

    printf("Enter (48 bit max) data pattern (in hex) for filter 0 (all one's if no filtering):  0x");
    scanf("%llx", &data0);

    printf("Enter (48 bit max) mask pattern (in hex) for filter 0  (all one's if no filtering):  0x");
    scanf("%llx", &mask0);

#ifndef COMPATIBILITY_32_BIT_MODE
    /* Ask for second filter info */
    printf("Enter (48 bit max) data pattern (in hex) for filter 1 (all one's if no filtering):  0x");
    scanf("%llx", &data1);

    printf("Enter (48 bit max) mask pattern (in hex) for filter 1 (all one's if no filtering):  0x");
    scanf("%llx", &mask1);
#endif

    NEXUS_IrInput_GetDefaultDataFilter(&pattern);

#ifdef COMPATIBILITY_32_BIT_MODE
    pattern.patternWord0 = data0 & 0xffffffff;
    pattern.patternWord1 = (data0 >> 32) & 0xffff;
    pattern.mask0 = mask0 & 0xffffffff;
    pattern.mask1 = (mask0 >> 32) & 0xffff;
    /* old way did not have a second filter */
#else
    pattern.filterWord[0].patternWord = data0;
    pattern.filterWord[0].mask = mask0;
    pattern.filterWord[0].enabled = true;
    pattern.filterWord[1].patternWord = data1;
    pattern.filterWord[1].mask = mask1;
    pattern.filterWord[1].enabled = true;
#endif

    while (kbhit()) {
        char c=getc(stdin);
        BSTD_UNUSED(c);
    }

    NEXUS_IrInput_EnableDataFilter(irHandle, &pattern);
}

static void bcmKirStopFiltering(NEXUS_IrInputHandle irHandle)
{
    NEXUS_IrInput_DisableDataFilter(irHandle);
}

void bcmKirTest (void)
{
    NEXUS_IrInputHandle irHandle = NULL;
    NEXUS_IrInputSettings irSettings;
    int enable;
    int channel=0;
    static int running=0;
    static int filtering=0;
    NEXUS_IrInputMode mode = NEXUS_IrInputMode_eCirEchoStar;

#if (BCHP_CHIP==7278)
    unsigned i;
    uint32_t reg;
    uint32_t pin_mux_ctrl[17];
    for (i=0; i < 17; i++) {
        NEXUS_Platform_ReadRegister(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0+i*4, &pin_mux_ctrl[i]);
        BDBG_MSG(("PIN_MUX_CTRL_%d: %08x", i, pin_mux_ctrl[i]));
    }

    reg = pin_mux_ctrl[10];
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_071)
        );

    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_071, 2)
        );
    NEXUS_Platform_WriteRegister(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);
#endif

    while (1)
    {
        printf("\n\n");
        printf("=========================\n");
        printf("  IR Receiver Test Menu  \n");
        printf("=========================\n");
        printf("    0) Exit\n");
        printf("    1) %s IR receiver\n", running==0 ? "Start" : "Stop");
        printf("    2) %s filtering\n", filtering==0 ? "Start" : "Stop");
        printf("    3) Change channel number (current channel=%d)\n", channel);
        printf("    4) Change remote type (current selection=%d)\n", mode);

        switch(Prompt()) {
            case 0:
                if (running==1) {
                    NEXUS_IrInput_Close(irHandle);
                    running=0;
                }
#if (BCHP_CHIP==7278)
                NEXUS_Platform_WriteRegister(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, pin_mux_ctrl[10]);
#endif
                return;

            case 1:
                if (running==0) {
                    NEXUS_IrInput_GetDefaultSettings(&irSettings);
                    irSettings.mode = mode;
                    irSettings.dataReady.callback = irCallback;
                    irSettings.dataReady.context = &irHandle;
                    irHandle = NEXUS_IrInput_Open(channel, &irSettings);
                    running=1;
                }
                else {
                    NEXUS_IrInput_Close(irHandle);
                    running=0;
                }
                break;

            case 2:
                if (!running) {
                    printf("Start IR receiver first.\n");
                    break;
                }
                if (filtering==0) {
                    bcmKirStartFiltering(irHandle);
                    filtering=1;
                }
                else {
                    bcmKirStopFiltering(irHandle);
                    filtering=0;
                }
                break;

            case 3:
                printf("Enter channel number:  ");
                scanf("%d", &channel);
                break;

            case 4:
                mode = bcmKirMode();
                if (running) {
                    irSettings.mode = mode;
                    NEXUS_IrInput_SetSettings(irHandle, &irSettings);
                }
                break;

            default:
                break;
        }
    }

    return;
}
