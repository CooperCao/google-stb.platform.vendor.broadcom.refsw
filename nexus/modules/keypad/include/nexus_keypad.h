/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Keypad
*    Specific APIs related to Keypad Input Control.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef NEXUS_KEYPAD_H__
#define NEXUS_KEYPAD_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_Keypad *NEXUS_KeypadHandle;

/***************************************************************************
Summary:
Keypad Device Settings
***************************************************************************/
typedef struct NEXUS_KeypadSettings
{
    unsigned repeatFilterTime;      /* Key Repeat filter time (in milliseconds) -- events that occur in less than this amount of time will be flagged as repeats */
    unsigned eventQueueSize;        /* Max number of events that can be queued before an overflow */
    NEXUS_CallbackDesc dataReady;   /* Data ready callback */
    uint32_t prescale;              /* 27mhz clock prescale value */
    uint8_t debounce;               /* debounce count */
    uint16_t keyMask;               /* bitmask to disable key presses. if you set a bit to 1, the interrupt will be masked (i.e. disabled) and NEXUS_KeypadEvent.code 
                                       for that bit will never be set. not supported on all silicon. */
} NEXUS_KeypadSettings;

/***************************************************************************
Summary:
Get default settings for a keypad.
***************************************************************************/
void NEXUS_Keypad_GetDefaultSettings(
    NEXUS_KeypadSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Open a keypad
***************************************************************************/
NEXUS_KeypadHandle NEXUS_Keypad_Open( /* attr{destructor=NEXUS_Keypad_Close} */
    unsigned index,
    const NEXUS_KeypadSettings *pSettings  /* attr{null_allowed=y} May be passed as NULL for defaults */
    );

/***************************************************************************
Summary:
Close a keypad.
***************************************************************************/
void NEXUS_Keypad_Close(
    NEXUS_KeypadHandle handle
    );

/***************************************************************************
Summary:
Keypad Input Event

Description:
The Keypad interface does not have a key up event. It reports events when a key is pressed down.
A key up can be derived by the application by looking for any change in a code or
an application timeout after not receiving any more codes.
***************************************************************************/
typedef struct NEXUS_KeypadEvent
{
    bool repeat;        /* If true, this key is a repeated key. */
    uint16_t code;      /* Code from receiver. The value of the code is platform dependent.
                           Most platforms return a code which is a bitmask of all keys
                           that are currently being pressed down. */
} NEXUS_KeypadEvent;

/***************************************************************************
Summary:
Retrieve queued keypad events.

Description:
This function should be called in response to the dataReady callback.
The application should repeatedly call this function until numEventsRead returns 0, which means
the Nexus queue of events is empty.

See Also:
NEXUS_Keypad_FlushEvents
***************************************************************************/
NEXUS_Error NEXUS_Keypad_GetEvents(
    NEXUS_KeypadHandle handle,
    NEXUS_KeypadEvent *pEvents,     /* [out] attr{nelem=numEvents;nelem_out=pNumEventsRead;reserved=4} Pointer to an array of events */
    size_t numEvents,               /* Size of the event array */
    size_t *pNumEventsRead,         /* [out] Number of events actually read */
    bool *pOverflow                 /* [out] If true, an internal overflow has occurred. The application should increase the NEXUS_KeypadSettings.eventQueueSize
                                       or call this function more frequently. */
    );

/***************************************************************************
Summary:
Discard all queued events
***************************************************************************/
void NEXUS_Keypad_FlushEvents(
    NEXUS_KeypadHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_KEYPAD_H__ */

