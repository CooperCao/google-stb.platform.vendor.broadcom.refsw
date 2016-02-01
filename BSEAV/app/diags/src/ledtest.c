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
#include <stdio.h>
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_led.h"
#include "bstd.h"
#include <string.h>
#include "prompt.h"

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
