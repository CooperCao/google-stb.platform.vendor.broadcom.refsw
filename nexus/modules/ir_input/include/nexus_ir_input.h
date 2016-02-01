/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: IrInput
*    Specific APIs related to IR Input Control.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#ifndef NEXUS_IR_INPUT_H__
#define NEXUS_IR_INPUT_H__

#include "nexus_ir_input_custom.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=******************
Infrared Remote input

See nexus_ir_input_custom.h for custom settings available with NEXUS_IrInputMode_eCustom.
********************/

typedef struct NEXUS_IrInput *NEXUS_IrInputHandle;

/***************************************************************************
Summary:
IR Input Mode used in NEXUS_IrInputSettings

Description:
The "interrupt devices" for these modes are:
    1) TwirpKbd
    2) SejinKbd (includes eSejin38KhzKbd and eSejin56KhzKbd)
    3) RemoteA
    4) RemoteB
    5) All Cir devices
See NEXUS_IrInput_Open for how interrupt devices affect opening simultaneous IrInputs on the same channel.
***************************************************************************/
typedef enum NEXUS_IrInputMode
{
    NEXUS_IrInputMode_eTwirpKbd = 1,        /* TWIRP */
    NEXUS_IrInputMode_eSejin38KhzKbd,       /* Sejin IR keyboard (38.4KHz) */
    NEXUS_IrInputMode_eSejin56KhzKbd,       /* Sejin IR keyboard (56.0KHz) */
    NEXUS_IrInputMode_eRemoteA,             /* remote A */
    NEXUS_IrInputMode_eRemoteB,             /* remote B */
    NEXUS_IrInputMode_eCirGI,               /* Consumer GI */
    NEXUS_IrInputMode_eCirSaE2050,          /* Consumer SA E2050 */
    NEXUS_IrInputMode_eCirTwirp,            /* Consumer Twirp */
    NEXUS_IrInputMode_eCirSony,             /* Consumer Sony */
    NEXUS_IrInputMode_eCirRecs80,           /* Consumer Rec580 */
    NEXUS_IrInputMode_eCirRc5,              /* Consumer Rc5 */
    NEXUS_IrInputMode_eCirUei,              /* Consumer UEI */
    NEXUS_IrInputMode_eCirRfUei,            /* Consumer RF UEI */
    NEXUS_IrInputMode_eCirEchoStar,         /* Consumer EchoRemote */
    NEXUS_IrInputMode_eSonySejin,           /* Sony Sejin keyboard using UART D */
    NEXUS_IrInputMode_eCirNec,              /* Consumer NEC */
    NEXUS_IrInputMode_eCirRC6,              /* Consumer RC6 */
    NEXUS_IrInputMode_eCirGISat,            /* Consumer GI Satellite */
    NEXUS_IrInputMode_eCustom,              /* Customer specific type. See NEXUS_IrInput_SetCustomSettings. */
    NEXUS_IrInputMode_eCirDirectvUhfr,      /* DIRECTV uhfr (In IR mode) */
    NEXUS_IrInputMode_eCirEchostarUhfr,     /* Echostar uhfr (In IR mode) */
    NEXUS_IrInputMode_eCirRcmmRcu,          /* RCMM Remote Control Unit */
    NEXUS_IrInputMode_eCirRstep,            /* R-step Remote Control Unit */
    NEXUS_IrInputMode_eCirXmp,              /* XMP-2 */
    NEXUS_IrInputMode_eCirXmp2Ack,          /* XMP-2 Ack/Nak */
    NEXUS_IrInputMode_eCirRC6Mode0,         /* Consumer RC6 Mode 0 */
    NEXUS_IrInputMode_eCirRca,              /* Consumer RCA */
    NEXUS_IrInputMode_eCirToshibaTC9012,    /* Consumer Toshiba */
    NEXUS_IrInputMode_eCirXip,              /* Consumer Tech4home XIP protocol */
    NEXUS_IrInputMode_eMax
} NEXUS_IrInputMode;

/***************************************************************************
Summary:
IR Input Device Settings

Description:
See comment for customSettings regarding potential stack problems.
***************************************************************************/
typedef struct NEXUS_IrInputSettings
{
    unsigned repeatFilterTime;      /* Key repeat filter time (in milliseconds). For events received less than this amount apart, NEXUS_IrInputEvent.repeat = true.
                                       If 0, repeats will be filtered in hardware only (if available). */
    unsigned eventQueueSize;        /* Max number of events that can be queued before an overflow */
    NEXUS_CallbackDesc dataReady;   /* Called when a new NEXUS_IrInputEvent is available */
    NEXUS_IrInputMode mode;         /* IR Mode (default is eRemoteA) */
    unsigned channel_number;        /* IR Channel Number (default is 0). See NEXUS_IrInput_Open for information about using channel_number
                                       and multiple IR inputs. */
    bool useCustomSettings;         /* Set this to true to use customSettings for the above-specified 'mode'.
                                       See NEXUS_IrInput_GetCustomSettingsForMode */
    NEXUS_IrInputCustomSettings customSettings; /* NOTE: This is a large structure (currently 248 bytes). Please malloc
                                       NEXUS_IrInputSettings if this is a problem for your stack. */
    unsigned pulseWidthFilter; /* units in microseconds. if zero, filtering is disabled. if non-zero, any pulse less than this is rejected. */
} NEXUS_IrInputSettings;

/***************************************************************************
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
***************************************************************************/
void NEXUS_IrInput_GetDefaultSettings(
    NEXUS_IrInputSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Get default IR input settings for a specific IR input mode.

Description:
The returned settings can be user-modified and then passed into NEXUS_IrInput_Open
through NEXUS_IrInputSettings.customSettings. This will enable the IR receiver in
the mode specified by NEXUS_IrInputSettings.mode with the custom settings.
***************************************************************************/
NEXUS_Error NEXUS_IrInput_GetCustomSettingsForMode(
    NEXUS_IrInputMode mode,
    NEXUS_IrInputCustomSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open an IR receiver.

Description:
Each instance of IrInput must have a unique channel_number and mode, with the added restriction that
no two IrInputs on the same channel_number can share the same "interrupt device". See comments on
NEXUS_IrInputMode for the mapping from mode to "interrupt device".

Check with your hardware manual that multiple KIR channels are supported.
Some hardware platforms can have more than one channel if the
pin muxing is done correctly and there is a physical IR Receiver connected to the pins.
***************************************************************************/
NEXUS_IrInputHandle NEXUS_IrInput_Open( /* attr{destructor=NEXUS_IrInput_Close} */
    unsigned index, /* unused except for debug */
    const NEXUS_IrInputSettings *pSettings  /* attr{null_allowed=y} May be passed as NULL for defaults */
    );

/***************************************************************************
Summary:
Close an IR receiver.
 ***************************************************************************/
void NEXUS_IrInput_Close(
    NEXUS_IrInputHandle handle
    );

/***************************************************************************
Summary:
Get current settings
 ***************************************************************************/
void NEXUS_IrInput_GetSettings(
    NEXUS_IrInputHandle handle,
    NEXUS_IrInputSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set new settings
 ***************************************************************************/
NEXUS_Error NEXUS_IrInput_SetSettings(
    NEXUS_IrInputHandle handle,
    const NEXUS_IrInputSettings *pSettings
    );

/***************************************************************************
Summary:
Data returned from IR input

Description:
Returned by NEXUS_IrInput_GetEvents
***************************************************************************/

typedef struct NEXUS_IrInputEvent
{
    bool repeat;        /* true if this is a repeated key */
    uint32_t code;      /* code bits 0-31 from the receiver */
    uint32_t codeHigh;  /* code bits 32-47 from the receiver, if available. */
    unsigned interval;  /* Time interval between SW ISR for this event and the previous event, in units of milliseconds */
} NEXUS_IrInputEvent;

/***************************************************************************
Summary:
Retrieve queued IR events.

Description:
This function should be called in response to the dataReady callback.
It should be repeatedly called until numEventsRead returns 0.

See Also:
NEXUS_IrInput_FlushEvents
 ***************************************************************************/
NEXUS_Error NEXUS_IrInput_GetEvents(
    NEXUS_IrInputHandle handle,
    NEXUS_IrInputEvent *pEvents,    /* [out] attr{nelem=numEvents;nelem_out=pNumEventsRead;reserved=4} Pointer to an array of events */
    size_t numEvents,               /* Size of the event array */
    size_t *pNumEventsRead,         /* [out] Number of events actually read */
    bool *pOverflow                 /* [out] Has an overflow occurred? */
    );

/***************************************************************************
Summary:
Discard all queued IR events
***************************************************************************/
void NEXUS_IrInput_FlushEvents(
    NEXUS_IrInputHandle handle
    );

/***************************************************************************
Summary:
Data returned from NEXUS_IrInput_GetPreambleStatus
***************************************************************************/
typedef struct NEXUS_IrInputPreambleStatus
{
    bool preambleA;
    bool preambleB;
} NEXUS_IrInputPreambleStatus;

/***************************************************************************
Summary:
Check if preamble A or preamble B is detected
***************************************************************************/
NEXUS_Error NEXUS_IrInput_GetPreambleStatus(
    NEXUS_IrInputHandle handle,
    NEXUS_IrInputPreambleStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Data filter settings
***************************************************************************/
#define NEXUS_IR_INPUT_MAX_FILTER_WORDS 2
#define NEXUS_IR_INPUT_FILTER_DISABLED (uint64_t)~0

typedef struct NEXUS_IrInputDataFilter
{
    struct {
        bool enabled;           /* 0=filtering disabled for this word, 1=filtering enabled */
        uint64_t patternWord;   /* May be 32, 40, or 48 bits depending on the chip */
        uint64_t mask;          /* Applies to pattern */
                                /* If you want to not use the filtering, either clear enabled, */
                                /* or set patternWord and mask to all one's.  I.e., use the */
                                /* above NEXUS_IR_INPUT_FILTER_DISABLED define */
    } filterWord[NEXUS_IR_INPUT_MAX_FILTER_WORDS];

    /* deprecated. use filterWord[] instead. mutually exclusive with filterWord[0]. */
    uint32_t patternWord0;  /* LSB bits 0-31 of pattern word */
    uint32_t patternWord1;  /* MSB bits 32-47 of pattern word */
    uint32_t mask0;         /* Mask bits corresponding to patternWord0  */
    uint32_t mask1;         /* Mask bits corresponding to patternWord1  */
} NEXUS_IrInputDataFilter;

/***************************************************************************
Summary:
Get default NEXUS_IrInputDataFilter
***************************************************************************/
void NEXUS_IrInput_GetDefaultDataFilter(
    NEXUS_IrInputDataFilter *pPattern   /* [out] */
    );

/***************************************************************************
Summary:
Enables a data filter with the given data pattern
***************************************************************************/
NEXUS_Error NEXUS_IrInput_EnableDataFilter(
    NEXUS_IrInputHandle handle,
    const NEXUS_IrInputDataFilter *pPattern   /* data filter pattern to match */
    );

/***************************************************************************
Summary:
Disables a data filter.
***************************************************************************/
void NEXUS_IrInput_DisableDataFilter(
    NEXUS_IrInputHandle handle
    );

/***************************************************************************
Summary:
Sets a data filter with the given data pattern but does not apply it. Filter
will be applied when we go to standby. Use NEXUS_IrInput_EnableDataFilter if
filter needs to be applied immediately for use in Normal mode.
***************************************************************************/
NEXUS_Error NEXUS_IrInput_SetDataFilter(
    NEXUS_IrInputHandle handle,
    const NEXUS_IrInputDataFilter *pPattern  /* attr{null_allowed=y} data filter pattern to match. if NULL, clear previously set filter. */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_IR_INPUT_H__ */

