/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "standby.h"

BDBG_MODULE(io);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;

void gpioCallback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    if(g_DeviceState.power_mode == ePowerModeS1 && g_StandbyNexusHandles.s1Event) {
        BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
    }
}

void gpio_open(void)
{
    NEXUS_GpioSettings gpioSettings;

    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eAonStandard, &gpioSettings);
    gpioSettings.mode = NEXUS_GpioMode_eInput;
    gpioSettings.interruptMode = NEXUS_GpioInterrupt_eRisingEdge;
    gpioSettings.interrupt.callback = gpioCallback;
    g_StandbyNexusHandles.gpioHandle = NEXUS_Gpio_Open(NEXUS_GpioType_eAonStandard, g_DeviceState.gpio_pin, &gpioSettings);
    if(!g_StandbyNexusHandles.gpioHandle) {
        printf("Could not open AON GPIO pin %d. Possibly incorrect pinmux\n", g_DeviceState.gpio_pin);
    }
}

void gpio_close(void)
{
    if(g_StandbyNexusHandles.gpioHandle)
        NEXUS_Gpio_Close(g_StandbyNexusHandles.gpioHandle);
    g_StandbyNexusHandles.gpioHandle = NULL;
}

void ir_open(void)
{
    NEXUS_IrInputSettings irSettings;
    NEXUS_IrInputDataFilter irPattern;

    if(!g_StandbyNexusHandles.input) {
        g_StandbyNexusHandles.input = binput_open(NULL);
        NEXUS_IrInput_GetDefaultDataFilter(&irPattern);
        irPattern.filterWord[0].patternWord = 0xf50aff00; /*power*/
        irPattern.filterWord[0].enabled = true;
        NEXUS_IrInput_SetDataFilter(binput_irhandle(g_StandbyNexusHandles.input), &irPattern);
    }
}

void ir_close(void)
{
    if (g_StandbyNexusHandles.input)
        binput_close(g_StandbyNexusHandles.input);
}

#if NEXUS_HAS_UHF_INPUT
void uhfCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;
    NEXUS_UhfInputHandle uhfHandle = *(NEXUS_UhfInputHandle *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        NEXUS_UhfInputEvent uhfEvent;
        rc = NEXUS_UhfInput_GetEvents(uhfHandle, &uhfEvent, 1, &numEvents, &overflow);
        if (numEvents && !uhfEvent.repeat) {
            printf("Received UHF event %08x\n", uhfEvent.code);
            if(g_StandbyNexusHandles.s1Event)
                BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
        }
    }
}
#endif

void uhf_open(void)
{
#if NEXUS_HAS_UHF_INPUT
    NEXUS_UhfInputSettings uhfSettings;

    if(!g_StandbyNexusHandles.uhfHandle) {
        NEXUS_UhfInput_GetDefaultSettings(&uhfSettings);
        uhfSettings.channel = NEXUS_UhfInputMode_eChannel1;
        uhfSettings.dataReady.callback = uhfCallback;
        uhfSettings.dataReady.context = &g_StandbyNexusHandles.uhfHandle;
        g_StandbyNexusHandles.uhfHandle = NEXUS_UhfInput_Open(0, &uhfSettings);
        BDBG_ASSERT(g_StandbyNexusHandles.uhfHandle);
    }
#endif
}

void uhf_close(void)
{
#if NEXUS_HAS_UHF_INPUT
    if(g_StandbyNexusHandles.uhfHandle)
        NEXUS_UhfInput_Close(g_StandbyNexusHandles.uhfHandle);
    g_StandbyNexusHandles.uhfHandle = NULL;
#endif
}

#if NEXUS_HAS_KEYPAD
void keypadCallback(void *pParam, int iParam)
{
    size_t numEvents = 1;
    NEXUS_Error rc = 0;
    bool overflow;
    NEXUS_KeypadHandle  keypadHandle = *(NEXUS_KeypadHandle *)pParam;
    BSTD_UNUSED(iParam);
    while (numEvents && !rc) {
        NEXUS_KeypadEvent keypadEvent;
        rc = NEXUS_Keypad_GetEvents(keypadHandle, &keypadEvent,1, &numEvents, &overflow);
        if (numEvents && !keypadEvent.repeat) {
            printf("Received Keypad event %08x\n", keypadEvent.code);
            if(g_StandbyNexusHandles.s1Event)
                BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
        }
    }
}

#endif
void keypad_open(void)
{
#if NEXUS_HAS_KEYPAD
    NEXUS_KeypadSettings keypadSettings;

    if(!g_StandbyNexusHandles.keypadHandle) {
        NEXUS_Keypad_GetDefaultSettings(&keypadSettings);
        keypadSettings.dataReady.callback = keypadCallback;
        keypadSettings.dataReady.context = &g_StandbyNexusHandles.keypadHandle;
        g_StandbyNexusHandles.keypadHandle = NEXUS_Keypad_Open(0, &keypadSettings);
    }
#endif
}

void keypad_close(void)
{
#if NEXUS_HAS_KEYPAD
    if(g_StandbyNexusHandles.keypadHandle)
        NEXUS_Keypad_Close(g_StandbyNexusHandles.keypadHandle);
    g_StandbyNexusHandles.keypadHandle = NULL;
#endif
}
