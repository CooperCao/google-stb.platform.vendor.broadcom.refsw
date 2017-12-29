/******************************************************************************
 *  Copyright (C) 2016-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_gpio.h"
#include "bstd.h"
#include "bkni.h"

static const char *gpio_type_str[] = {
    "GPIO",
    "SGPIO",
    "unused",
    "AON_GPIO",
    "AON_SGPIO"
};

void gpio_interrupt(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_Error rc;
    NEXUS_GpioHandle pin;
    NEXUS_GpioSettings gpioSettings;
    BKNI_EventHandle event;
    NEXUS_PlatformSettings platformSettings;
    unsigned pinNumber = 0;
    NEXUS_GpioType gpioType = NEXUS_GpioType_eStandard;

    if (argc > 1) {
        pinNumber = strtoul(argv[1], NULL, 0);
    }

    if (argc > 2) {
        if      (strcmp(argv[2], "s" ) == 0) gpioType = NEXUS_GpioType_eSpecial;
        else if (strcmp(argv[2], "a" ) == 0) gpioType = NEXUS_GpioType_eAonStandard;
        else if (strcmp(argv[2], "as") == 0) gpioType = NEXUS_GpioType_eAonSpecial;
    }
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    BKNI_CreateEvent(&event);

    NEXUS_Gpio_GetDefaultSettings(gpioType, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eRisingEdge;
    gpioSettings.interrupt.callback = gpio_interrupt;
    gpioSettings.interrupt.context = event;
    pin = NEXUS_Gpio_Open(gpioType, pinNumber, &gpioSettings);

    printf("waiting for %s%d interrupt...\n", gpio_type_str[gpioType], pinNumber);
    while (1) {
        NEXUS_GpioStatus gpioStatus;

        rc = BKNI_WaitForEvent(event, 0xFFFFFFFF);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Gpio_GetStatus(pin, &gpioStatus);
        BDBG_ASSERT(!rc);

        printf("%s%d: value %d, interrupt status %d\n",
            gpio_type_str[gpioType],
            pinNumber,
            gpioStatus.value,
            gpioStatus.interruptPending);

        NEXUS_Gpio_ClearInterrupt(pin);
    }

    NEXUS_Platform_Uninit();
    return 0;
}
