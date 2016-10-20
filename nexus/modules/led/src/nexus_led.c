/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_led_module.h"
#include "bled.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_SPI
#include "nexus_spi.h"
#endif

BDBG_MODULE(nexus_led);

NEXUS_ModuleHandle g_NEXUS_ledModule;
struct {
    NEXUS_LedModuleSettings settings;
} g_NEXUS_led;

/****************************************
* Module functions
***************/
void NEXUS_LedModule_GetDefaultSettings(NEXUS_LedModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}
NEXUS_ModuleHandle NEXUS_LedModule_Init(const NEXUS_LedModuleSettings *pSettings)
{
    BDBG_ASSERT(!g_NEXUS_ledModule);
    g_NEXUS_ledModule = NEXUS_Module_Create("led", NULL);
    if (pSettings) {
        g_NEXUS_led.settings = *pSettings;
    }
    else {
        NEXUS_LedModule_GetDefaultSettings(&g_NEXUS_led.settings);
    }
    return g_NEXUS_ledModule;
}
void NEXUS_LedModule_Uninit()
{
    NEXUS_Module_Destroy(g_NEXUS_ledModule);
    g_NEXUS_ledModule = NULL;
}

/****************************************
* API functions
***************/

#define NEXUS_NUM_LED_DIGITS 4
#define NEXUS_ONE_DISPLAY_LENGTH NEXUS_NUM_LED_DIGITS

struct NEXUS_Led {
    NEXUS_OBJECT(NEXUS_Led);
    BLED_Handle led;
    NEXUS_LedSettings settings;
    uint8_t ledState;
    bool dots[NEXUS_NUM_LED_DIGITS];
    uint8_t chars[NEXUS_NUM_LED_DIGITS];
    NEXUS_LedString string;
    unsigned cur_position;
    unsigned total_length;
    NEXUS_TimerHandle scrollTimer;
#if NEXUS_HAS_SPI
    NEXUS_SpiHandle spi;
#endif
};

void NEXUS_Led_GetDefaultSettings(NEXUS_LedSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->brightness = 100;
}

NEXUS_LedHandle NEXUS_Led_Open(unsigned index, const NEXUS_LedSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_LedHandle led;
    BLED_Settings ledSettings;
    NEXUS_LedSettings defaultSettings;

    if (!pSettings) {
        NEXUS_Led_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* only one LED controller */
    if (index > 0) {
        BDBG_ERR(("invalid led[%d]", index));
        return NULL;
    }

    led = BKNI_Malloc(sizeof(*led));
    if (!led) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_Led, led);

#if NEXUS_HAS_SPI
    if (g_NEXUS_led.settings.spi.valid) {
        NEXUS_SpiSettings spiSettings;
        NEXUS_Spi_GetDefaultSettings(&spiSettings);
        spiSettings.clockActiveLow = true;
        led->spi = NEXUS_Spi_Open(g_NEXUS_led.settings.spi.index, &spiSettings);
        if (!led->spi) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        return led;
    }
#endif

    BLED_GetDefaultSettings(&ledSettings, g_pCoreHandles->chp);
    ledSettings.percentBrightness = pSettings->brightness;
    rc = BLED_Open(&led->led, g_pCoreHandles->chp, g_pCoreHandles->reg, &ledSettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    rc = NEXUS_Led_SetSettings(led, pSettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    return led;
error:
    NEXUS_Led_Close(led);
    return NULL;
}

static void NEXUS_Led_P_Finalizer(NEXUS_LedHandle led)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Led, led);

#if NEXUS_HAS_SPI
    if (led->spi) {
        NEXUS_Spi_Close(led->spi);
    }
#endif

    if (led->led) {
        BLED_Close(led->led);
    }

    if (led->scrollTimer) {
        NEXUS_CancelTimer(led->scrollTimer);
        led->scrollTimer = NULL;
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Led, led);
    BKNI_Free(led);
}


NEXUS_OBJECT_CLASS_MAKE(NEXUS_Led, NEXUS_Led_Close);

void NEXUS_Led_GetSettings(NEXUS_LedHandle led, NEXUS_LedSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(led, NEXUS_Led);
    *pSettings = led->settings;
}

static void NEXUS_Led_P_ScrollTimerCallback(void *pContext);

NEXUS_Error NEXUS_Led_SetSettings(NEXUS_LedHandle led, const NEXUS_LedSettings *pSettings)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(led, NEXUS_Led);

#if NEXUS_HAS_SPI
    if (!led->spi)
#endif
    {
        rc = BLED_AdjustBrightness(led->led, pSettings->brightness);
        if (rc) return BERR_TRACE(rc);
    }
    if (pSettings->scrollingEnabled) {
        if (!led->scrollTimer && led->total_length) {
            led->scrollTimer = NEXUS_ScheduleTimer(pSettings->scrollDelay,
                                                    NEXUS_Led_P_ScrollTimerCallback,
                                                    led);
            if (led->scrollTimer == NULL) {
                return BERR_TRACE(NEXUS_UNKNOWN);
            }
        }
    }
    else {
        if (led->scrollTimer){
            NEXUS_CancelTimer(led->scrollTimer);
            led->scrollTimer = NULL;
        }
    }

    led->settings = *pSettings;
    return 0;
}

static NEXUS_Error NEXUS_Led_P_WriteOneDisplayString(NEXUS_LedHandle led)
{
    unsigned i;

    for (i=0;i<NEXUS_NUM_LED_DIGITS;i++) {
        uint8_t code;
        BERR_Code rc;

        switch(led->chars[i]) {
        case 'A': case 'a': code = LED_A; break;
        case 'B': case 'b': code = LED_B; break;
        case 'C': case 'c': code = LED_C; break;
        case 'D': case 'd': code = LED_D; break;
        case 'E': case 'e': code = LED_E; break;
        case 'F': case 'f': code = LED_F; break;
        case 'G': case 'g': code = LED_G; break;
        case 'H': case 'h': code = LED_H; break;
        case 'I': case 'i': code = LED_I; break;
        case 'J': case 'j': code = LED_J; break;
        case 'K': case 'k': code = LED_K; break;
        case 'L': case 'l': code = LED_L; break;
        case 'M': case 'm': code = LED_M; break;
        case 'N': case 'n': code = LED_N; break;
        case 'O': case 'o': code = LED_O; break;
        case 'P': case 'p': code = LED_P; break;
        case 'Q': case 'q': code = LED_C; break;
        case 'R': case 'r': code = LED_R; break;
        case 'S': case 's': code = LED_S; break;
        case 'T': case 't': code = LED_T; break;
        case 'U': case 'u': code = LED_U; break;
        case 'V': case 'v': code = LED_V; break;
        case 'W': case 'w': code = LED_W; break;
        case 'X': case 'x': code = LED_X; break;
        case 'Y': case 'y': code = LED_Y; break;
        case 'Z': case 'z': code = LED_Z; break;
        case '-': code = LED_DASH; break;
        case '0': code = LED_NUM0; break;
        case '1': code = LED_NUM1; break;
        case '2': code = LED_NUM2; break;
        case '3': code = LED_NUM3; break;
        case '4': code = LED_NUM4; break;
        case '5': code = LED_NUM5; break;
        case '6': code = LED_NUM6; break;
        case '7': code = LED_NUM7; break;
        case '8': code = LED_NUM8; break;
        case '9': code = LED_NUM9; break;
        default: code = 0xFF;
        }

        if (led->dots[i]) {
            code &= ~0x80; /* 0 = on */
        }
        else {
            code |= 0x80; /* 1 = off */
        }

        BDBG_MSG(("write %d: %c%s, code=%x", i+1, led->chars[i], led->dots[i]?" (dot)":"", code));
        rc = BLED_Write(led->led, i+1, code);
        if (rc) { return BERR_TRACE(rc); }
    }
    return 0;
}

static void NEXUS_Led_P_ScrollTimerCallback(void *pContext)
{
    NEXUS_LedHandle led = pContext;
    unsigned copy_length;
    unsigned i, idx;

    BDBG_OBJECT_ASSERT(led, NEXUS_Led);
    led->scrollTimer = NULL; /* just in case we return without rescheduling */

    if (led->cur_position >= led->total_length) {
        led->cur_position = 0; /* wrap-around */
    }

    /* take care of end of string part */
    copy_length = NEXUS_NUM_LED_DIGITS;
    if ((led->total_length - led->cur_position) < NEXUS_ONE_DISPLAY_LENGTH){
        for(idx = (led->total_length - led->cur_position); idx < NEXUS_ONE_DISPLAY_LENGTH; idx++, copy_length--) {
           led->chars[idx] = 0x20; /* blank space */
        }
    }
    
    /* copy each character into the output buffer */
    for (i =0, idx=led->cur_position;i<copy_length;i++, idx++) {
        if (idx < led->total_length) {
            led->chars[i] = led->string.data[idx];
        }
        else {
            led->chars[i] = 0;
        }
    }
    BDBG_MSG(("'%c%c%c%c', total=%d, cur=%d", led->chars[0], led->chars[1], led->chars[2], led->chars[3], led->total_length, led->cur_position));
    led->cur_position++;
    NEXUS_Led_P_WriteOneDisplayString(led);
    
    BDBG_ASSERT(led->settings.scrollingEnabled);
    led->scrollTimer = NEXUS_ScheduleTimer(led->settings.scrollDelay,
                                            NEXUS_Led_P_ScrollTimerCallback,
                                            led);
}


NEXUS_Error NEXUS_Led_WriteString_driver(NEXUS_LedHandle led, const NEXUS_LedString *pString)
{
    unsigned i, idx;

    BDBG_OBJECT_ASSERT(led, NEXUS_Led);

    /* SPI LED does not support this function */
#if NEXUS_HAS_SPI
    if (led->spi) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#endif

    led->cur_position = 0;
    led->string = *pString;
    idx = 0;
    while (led->string.data[idx] != 0x00 && idx < sizeof(led->string.data)-1) {
        idx++;
    }
    led->total_length = idx;
    if (led->string.data[idx]) {
        BDBG_ASSERT(idx == sizeof(led->string.data)-1);
        led->string.data[idx] = 0x00;
        BDBG_WRN(("NEXUS_LedString.data was not null terminated"));
    }
    
    if (led->settings.scrollingEnabled) {
        /* retrigger timer immediately */
        if (!led->scrollTimer && led->total_length) {
            led->scrollTimer = NEXUS_ScheduleTimer(led->settings.scrollDelay,
                                                NEXUS_Led_P_ScrollTimerCallback,
                                                led);
            if(led->scrollTimer == NULL) {
                return BERR_TRACE(NEXUS_UNKNOWN);
            }
        }
    }
    else {
        BDBG_ASSERT(!led->scrollTimer);
        /* write the whole chars[] array */
        for (i=0;i<NEXUS_NUM_LED_DIGITS;i++) {
            if (i < led->total_length) {
                led->chars[i] = led->string.data[i];
            }
            else {
                led->chars[i] = 0;
            }
        }

        return NEXUS_Led_P_WriteOneDisplayString(led);
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Led_WriteSegments(NEXUS_LedHandle led, NEXUS_LedDigit digit, uint16_t value)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(led, NEXUS_Led);

    /* SPI LED does not support this function */
#if NEXUS_HAS_SPI
    if (led->spi) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#endif

    if (digit >= NEXUS_LedDigit_eMax) {
        return NEXUS_INVALID_PARAMETER;
    }

    rc = BLED_Write(led->led, digit, ~value);
    if (rc) { return BERR_TRACE(rc); }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Led_SetDot( NEXUS_LedHandle led, unsigned dotIndex, bool enabled )
{
    BDBG_OBJECT_ASSERT(led, NEXUS_Led);

    /* SPI LED does not support this function */
#if NEXUS_HAS_SPI
    if (led->spi) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#endif

    if (dotIndex >= NEXUS_NUM_LED_DIGITS) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    led->dots[dotIndex] = enabled;
    return NEXUS_Led_P_WriteOneDisplayString(led);
}

NEXUS_Error NEXUS_Led_SetLedState(NEXUS_LedHandle led, unsigned ledNumber, bool active)
{
    BDBG_OBJECT_ASSERT(led, NEXUS_Led);

#if NEXUS_HAS_SPI
    if (led->spi) {
        NEXUS_Error rc;
        uint8_t chipAddr;
        uint8_t data[2];

        if (active)
            led->ledState |= (1 << ledNumber);
        else
            led->ledState &= ~(1 << ledNumber);

        /* use chip address 0 */
        chipAddr = 0;
        data[0] = (chipAddr << 1) | 0x01;
        data[1] = led->ledState;

        rc = NEXUS_Spi_Write(led->spi, data, 2);
        if (rc) return BERR_TRACE(rc);

        return 0;
    }
#endif

    return BLED_SetDiscreteLED(led->led, active, ledNumber);
}

void NEXUS_Led_GetDefaultStartClockSettings(NEXUS_LedStartClockSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_Error NEXUS_Led_StartClock(NEXUS_LedHandle led, const NEXUS_LedStartClockSettings *pSettings)
{
#if HLCD_SUPPORT
    BERR_Code rc;
    bool hour_mode, display_mode;

    BDBG_OBJECT_ASSERT(led, NEXUS_Led);
    BDBG_ASSERT(pSettings);

#if NEXUS_HAS_SPI
    if (led->spi) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#endif
    
    switch(pSettings->displayMode) {
    case NEXUS_LedClockDisplayMode_e12Hour:
    hour_mode = 0; 
    display_mode = 1;
    break;
    case NEXUS_LedClockDisplayMode_e24Hour:
    default:
    hour_mode = 1; 
    display_mode = 0;
    break;
    }
    rc = BLED_StartHLCD(led->led, pSettings->hour, pSettings->min, pSettings->sec, hour_mode, display_mode);
    if (rc) { return BERR_TRACE(rc); }
    
    return 0;
#else
    BSTD_UNUSED(led);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

void NEXUS_Led_StopClock(NEXUS_LedHandle led)
{    
#if HLCD_SUPPORT
    BDBG_OBJECT_ASSERT(led, NEXUS_Led);

#if NEXUS_HAS_SPI
    if (led->spi) {
        return;
    }
#endif
    
    BLED_StopHLCD(led->led);    
#else
    BSTD_UNUSED(led);
#endif    
    return ;
}
