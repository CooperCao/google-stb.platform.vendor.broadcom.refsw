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
 *     LED output control
 *
 ***************************************************************************/

#ifndef NEXUS_LED_H__
#define NEXUS_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_Led *NEXUS_LedHandle;

/***************************************************************************
Summary:
    Settings for an LED controller
****************************************************************************/
typedef struct NEXUS_LedSettings
{
    unsigned brightness;            /* Percent of brightness (0..100) */
    bool scrollingEnabled;          /* Is scrolling enabled?  If false, data will be clipped to the size of the display */
    bool alwaysScroll;              /* Deprecated; use scrollingEnabled to control scrolling. */
    unsigned scrollDelay;           /* in milliseconds, delay between each character while scrolling */
} NEXUS_LedSettings;

/***************************************************************************
Summary:
    Digits on the LED display
****************************************************************************/
typedef enum NEXUS_LedDigit
{
    NEXUS_LedDigit_e1 = 1,
    NEXUS_LedDigit_e2,
    NEXUS_LedDigit_e3,
    NEXUS_LedDigit_e4,
    NEXUS_LedDigit_eMax
} NEXUS_LedDigit;

/***************************************************************************
Summary:
    Individual segments of a single LED digit
****************************************************************************/
typedef enum NEXUS_LedSegment
{
    NEXUS_LedSegment_eTop        = 0x01,
    NEXUS_LedSegment_eUpperRight = 0x02,
    NEXUS_LedSegment_eLowerRight = 0x04,
    NEXUS_LedSegment_eBottom     = 0x08,
    NEXUS_LedSegment_eLowerLeft  = 0x10,
    NEXUS_LedSegment_eUpperLeft  = 0x20,
    NEXUS_LedSegment_eMiddle     = 0x40,
    NEXUS_LedSegment_eDot        = 0x80,
    NEXUS_LedSegment_eMax
} NEXUS_LedSegment;

/***************************************************************************
Summary:
    This function returns the default settings for an LED controller
****************************************************************************/
void NEXUS_Led_GetDefaultSettings(
    NEXUS_LedSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    This function opens an LED controller
****************************************************************************/
NEXUS_LedHandle NEXUS_Led_Open( /* attr{destructor=NEXUS_Led_Close} */
    unsigned index,
    const NEXUS_LedSettings *pSettings /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    This function closes an LED controller
****************************************************************************/
void NEXUS_Led_Close(
    NEXUS_LedHandle handle
    );

/***************************************************************************
Summary:
    This function retrieves the current settings for an LED controller
****************************************************************************/
void NEXUS_Led_GetSettings(
    NEXUS_LedHandle handle,
    NEXUS_LedSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    This function sets the current settings for an LED controller
****************************************************************************/
NEXUS_Error NEXUS_Led_SetSettings(
    NEXUS_LedHandle handle,
    const NEXUS_LedSettings *pSettings
    );

/***************************************************************************
Summary:
Write a string to the seven-segment LED display.

Description:
If the length of pString is greater than the number of available LED characters,
the scrolling option specified in NEXUS_LedSettings will be used.

****************************************************************************/
NEXUS_Error NEXUS_Led_WriteString( /* attr{local=true} */
    NEXUS_LedHandle handle,
    const char *pString, /* ASCII string which is either NULL-terminated or terminated by the length parameter. */
    unsigned length /* This limits the number of characters used in pString. If 0 or greater than the length of pString, pString will be terminated by a NULL-terminator. */
    );

/***************************************************************************
Summary:
Structure based flavor of WriteString (which Nexus can safely proxy)

Example:
NEXUS_LedString str = {"Hello World"};
NEXUS_Led_WriteString_driver(led, &str);
****************************************************************************/
typedef struct NEXUS_LedString
{
    char data[128]; /* null terminated string */
} NEXUS_LedString;

NEXUS_Error NEXUS_Led_WriteString_driver(
    NEXUS_LedHandle handle,
    const NEXUS_LedString *pString
    );

/***************************************************************************
Summary:
Write a custom value to a single LED digit.

Description:
Use bitwise OR of NEXUS_LedSegment elements to construct the value to be written.

Nexus will do a bitwise negation, then write to LDK hardware.
For Nexus, 1 means the segment is lit; 0 means it is not.
For LDK hardware, 0 means the segment is lit; 1 means it is not.
****************************************************************************/
NEXUS_Error NEXUS_Led_WriteSegments(
    NEXUS_LedHandle handle,
    NEXUS_LedDigit digit,
    uint16_t value /* Value to be written to the digit. Writing 0 clears the digit */
    );

/***************************************************************************
Summary:
Set the dots on a seven-segment LED display.

Description:
The seven bars of the LED seven-segment display are controlled with NEXUS_Led_WriteString.
In addition, these digits also have dots, either in the lower right corner or upper left corner.
This API sets those bits.
By default they are off.
****************************************************************************/
NEXUS_Error NEXUS_Led_SetDot(
    NEXUS_LedHandle handle,
    unsigned dotIndex, /* corresponds to the index of characters in NEXUS_Led_WriteString's pString[] */
    bool enabled /* true = on, false = off */
    );

/***************************************************************************
Summary:
This function sets the state of a discrete LED on the LED controller.
****************************************************************************/
NEXUS_Error NEXUS_Led_SetLedState(
    NEXUS_LedHandle handle,
    unsigned ledNumber,
    bool active
    );


/***************************************************************************
Summary:
Determines the display format of time
****************************************************************************/
typedef enum NEXUS_LedClockDisplayMode
{
    NEXUS_LedClockDisplayMode_e12Hour,
    NEXUS_LedClockDisplayMode_e24Hour,
    NEXUS_LedClockDisplayMode_eMax
} NEXUS_LedClockDisplayMode;

/***************************************************************************
Summary:
Start Settings for AON Hardware Led Clock. Sets the hour, min, sec and display mode
****************************************************************************/
typedef struct NEXUS_LedStartClockSettings
{
    unsigned hour; /* 0..23, regardless of displayMode */
    unsigned min; /* 0..59 */
    unsigned sec; /* 0..59 */
    NEXUS_LedClockDisplayMode displayMode;
} NEXUS_LedStartClockSettings;

/***************************************************************************
Summary:
Get Default Start Settings for AON Hardware LED Clock
****************************************************************************/
void NEXUS_Led_GetDefaultStartClockSettings(
    NEXUS_LedStartClockSettings *pSettings /* [out] */
    );
   

/***************************************************************************
Summary:
This function starts the AON Hardware LED Clock
****************************************************************************/
NEXUS_Error NEXUS_Led_StartClock(
    NEXUS_LedHandle led,
    const NEXUS_LedStartClockSettings *pSettings
    ); 

/***************************************************************************
Summary:
This function stops the AON Hardware LED Clock
****************************************************************************/
void NEXUS_Led_StopClock(
    NEXUS_LedHandle led
    );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_LED_H__ */

