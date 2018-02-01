/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/*= Module Overview *********************************************************
<verbatim>

Overview
Timer module represents a generalize representatiion of a hardware timer.  The 
basic function of a timer is to count.  Some timers count up, while others count
down.  If the timer is given a specific time period, a callback routine (ISR level)
must be provided.  This callback will be used for notification that the timer expired.

This interface requires a set of timers to be opened.  Once opened, timers from
the set can be created, started & stopped, and then destroyed as needed.  When
the timers are no longer required they can be closed.  Unless an exclusive timer
is requested, all created timers are virtual.

Design
The design for BTMR PI API is broken into three parts.

o Part 1 (open/close):

    These APIs are used for opening and closing a general BTMR device.
    This defines a set of timers that can be created/destroyed dynamically.

o Part 2 (create/destroy):

    These APIs are used for dynamically allocating an available timer.
    Once created, a timer can be started/stopped/read as desired.
    When the timer is no longer needed it can be destroyed.

o Part 3 (start/stop/read):

    These APIs are used for starting and stopping a timer.  The timer can
    be read to get the current time value.

Usage
The usage of BTMR involves the following:

   * Opening a set of timers
   * Allocating a timer from the set (create)
   * Start timer
   * Stop/Read timer
   * Destroying the timer
   * Closing the set of timers

===================
Types of timers and timer types:
There are three types of timers (exclusive, shared, and virtual) and three timer types (stop-watch/free-run, count-down,
and periodic).  If a timer is needed, first choose the type of timer based on the description and usage notes below.
Then choose the timer type based on the timer requirements and restrictions (see "enum BTMR_Type" below).

Types of timers:
exclusive timer: Not virtual and not shared.  Can be manipulated with or without this API.
shared timer: Not virutal and shared by other shared timer requestors.  Cannot be started/stopped.
virtual timer: Cannot request its registers for external manipulation.

Timer types:
countdown: Counts down from initial value to zero.  Stops and calls back when timer reaches zero.
periodic: Counts down from initial value to zero.  Calls back when timer reaches zero and restarts on return from callback.
stopwatch/free-run: Counts up and wraps at maximum timer value.

===================
Exclusive timers:
Exclusive access can be requested when creating a timer.  This will prevent the timer from being virtualized
or shared with other requestors.  You can request the registers of an exclusive timer and control it directly
by manipulating the registers or by using the supplied API functions.  Care should be taken to NOT mix the
control (i.e. using this API and direct register control) as state information is saved when using the PI.
If a callback function is registered at the create, this PI will call the supplied callback function when
the countdown/periodic timer expires even if the timer is controlled externally.

Usage notes: There are a limited set of exclusive timers and the create will fail if no timer is available.
This type of timer should be used when the timer control is to be done using firmware by a sub-processor
or DSP that has access to the hardware register space and has need to start and stop the timer at different times.
The software would create the timer, request the location of the allocated timer's registers, and pass the location
to the underlying hardware/firmware.  If the hardware/firmware has need to read, but not stop/start the timer, then
a shared timer should be used instead.

===================
Shared timers:
A shared timer (BTMR_Type_eSharedFreeRun) is a stop watch timer that can only be read.  That is, it is a free running
timer that cannot be started or stopped (its always running, counting up).  A shared timer will create a (physical)
timer on the first request for a shared timer and then provide the same timer handle to subsequent shared timer create
requests.  This timer will be started on first create and stopped on the last destroy.  Because this timer is shared
with other requestors it cannot be started or stopped using this PI (only read).  Because a shared timer is a stop
watch timer it cannot be supplied with a callback function.

Usage notes: A shared timer, like a exculsive timer, should be used used when access to the timer's registers is needed
but when the external hardware/firmware does not need to start and stop the timer.

Warning: The exclusive flag is ignored when using shared timers.  This API lets you get the register locations of the
(one) shared timer.  Care should be taken to NOT WRITE those registers (i.e. start/stop the timer). 
Doing so would disrupt the usage of the shared timer and all its users.

===================
Virtual timers:
Unless an exclusive or shared timer is requested, all other timers are considered virtual.
That is, one physical timer is (allocated) used to create an unlimited set of virtual timers.  The only difference
in functionality is that a virtual timer cannot control the timer outside of this API (i.e. you CANNOT request the
pointer to it's registers) and additional overhead is needed to process the virtual timer.
Attempting to get the registers (BTMR_GetTimerRegisters) of a virtual timer will fail.

Usage notes: A virtual timer should be used whenever access to the timer's registers is not needed (i.e. controlled
outside of this PI).  This should be the default.

===================
Interrupt Requirements:
Callback functions are supplied on some timer types at creation time.  These callbacks are called in interrupt
context and MUST not call functions that aren't designated as ISR safe functions.  It is okay to restart a
countdown timer from the callback and to stop a periodic timer from the callback but the timer should NEVER be 
destroyed from the callback.

===================
Timer resolutions:
Timeouts and initial values are supplied in microsecond increments.  Reading the timer value using the PI returns a
value in microsecond increments.  Reading an exclusive or shared timer by using the timer registers will be in
timer tick values (based on 27Mhz clock).  A conversion function is supplied in this PI (BTMR_TimerToMicroSeconds).

A timeout or initial value must not exceed: 39,768,215 microseconds (39,768 milli-seconds or 39.7 seconds).
The function "BTMR_ReadTimerMax" can be used to get this value.

===================
Sample Code

volatile bool interrupt = false;
static void myIntHandler(context *myParam1, int myParam2) { interrupt = true; }

void main( void )
{
    BTMR_Handle      hTimerDevice;
    BTMR_DeviceSettings  deviceSettings;
    BREG_Handle      hReg;
    BCHP_Handle      hChip;
    BINT_Handle      hInt;
    BTMR_TimerHandle hTimer;
    
    // Do other initialization, i.e. for BREG, BCHP, BINT, etc.

    // Now, ready to setup the BTMR

    // Get defaults and make any changes required 
    BTMR_GetDefaultDeviceSettings(&deviceSettings);
    deviceSettings.timerMask = 0; // defines the timers to reserve for external use (none)

    BTMR_Open(&hTimerDevice, hChip, hReg, hInt, &deviceSettings);

    // Timers are now ready for use.

    // Create and start a timer.
    {
        BTMR_TimerSettings timerSettings = { BTMR_Type_eCountDown, (BTMR_CallbackFunc)myIntHandler, NULL, 0, false };
        BTMR_CreateTimer(hTimerDevice, &hTimer, &timerSettings);
        BTMR_StartTimer(hTimer, 1000); // interrupt in 1000 usec or 1 millisecond
    }

    while (!interrupt) ; // wait for timer to go off!

    BTMR_StopTimer(hTimer);
    BTMR_DestroyTimer(hTimer);

    BTMR_Close(hTimerDevice);
}
    
</verbatim>
***************************************************************************/

#ifndef BTMR_H__
#define BTMR_H__

#include "bchp.h"
#include "bint.h"
#include "berr_ids.h"

/***************************************************************************
Summary:
    Error Codes specific to BTMR.

Description:
    These define the set of error codes unique to the TMR functions.
    This list does not limit the returned error codes (also uses general errors defined elsewhere).

****************************************************************************/
#define BTMR_ERR_TODO                  BERR_MAKE_CODE(BERR_TMR_ID, 0)
#define BTMR_ERR_NO_TIMERS_AVAILABLE   BERR_MAKE_CODE(BERR_TMR_ID, 1) /* request for a timer when none available */
#define BTMR_ERR_EXCLUSIVE_OPERATION   BERR_MAKE_CODE(BERR_TMR_ID, 2) /* exclusive operation selected for non-exclusive device */
#define BTMR_ERR_EXCLUSIVE_TIMER       BERR_MAKE_CODE(BERR_TMR_ID, 3) /* non-exclusive operation selected for exclusive device */
#define BTMR_ERR_ALREADY_STARTED       BERR_MAKE_CODE(BERR_TMR_ID, 4) /* request to start a running timer */
#define BTMR_ERR_ALREADY_STOPPED       BERR_MAKE_CODE(BERR_TMR_ID, 5) /* request to stop a stopped timer (if enabled) */
#define BTMR_ERR_FREE_RUN_TIMER        BERR_MAKE_CODE(BERR_TMR_ID, 6) /* start/stop operation selected on free-run device */
#define BTMR_ERR_NO_TIMEOUT_GIVEN      BERR_MAKE_CODE(BERR_TMR_ID, 7) /* attempt to start count-down timer with no time value */
#define BTMR_ERR_TIMEOUT_TOO_SMALL     BERR_MAKE_CODE(BERR_TMR_ID, 8) /* attempt to create a periodic timer with too small timeout */
#define BTMR_ERR_TIMEOUT_TOO_LARGE     BERR_MAKE_CODE(BERR_TMR_ID, 9) /* attempt to create a count-down or periodic timer with too large a timeout */
#define BTMR_ERR_DELETE_FROM_ISR       BERR_MAKE_CODE(BERR_TMR_ID, 10)/* attempt to delete a count-down or periodic timer from callback routine */
#define BTMR_ERR_STANBY_ACTIVE         BERR_MAKE_CODE(BERR_TMR_ID, 11)/* attempt to perform a ttimer operation while in stand-by state */

/***************************************************************************
Summary:
    The handle for Timer device.

Description:
    An opaque handle for BTMR device.
    This is returned on a successful Open call and is passed into the
    Create and Close calls as input parameter.

See Also:
    BTMR_Open(), BTMR_Close(), BTMR_CreateTimer.

****************************************************************************/
typedef struct BTMR_P_Context *BTMR_Handle;


/***************************************************************************
Summary:
    The handle for Timer module.

Description:
    An opaque handle for BTMR timer (and allocated timer).
    This is returned on a successful Create call and is passed in to the
    Start, Stop, Read, and Destroy requests as input parameter.

See Also:
    BTMR_CreateTimer(), BTMR_DestroyTimer(), BTMR_StartTimer(), BTMR_StopTimer(),
    BTMR_ReadTimer(), BTMR_GetDefaultTimerSettings().

****************************************************************************/
typedef struct BTMR_P_TimerContext *BTMR_TimerHandle;


/***************************************************************************
Summary:
    The format for the callback.

Description:
    When a timer is registered with a callback, that callback will be called
    when the timer interrupts.  The callback will occur in interrupt context.
    This is passed in as past of the Timer Settings at creation time.
    Context that includes the timer handle should be passed (pParm1) to the
    callback if stopping the timer in the callback is desired.

See Also:
    BTMR_CreateTimer(), BTMR_StopTimer_isr().

****************************************************************************/
typedef void (*BTMR_CallbackFunc)(
    void *pParm1,                /* [in] First parameter used to define context */
    int parm2                    /* [in] Second parameter used to define context */
    ); 

/***************************************************************************
Summary:
    The mask that designates which timers are not to be used.

Description:
    This defines the timer set (set of timers) information.
    Legacy code exists that uses a particular timer outside of this interface.  
    The 'timerMask' mask defines which timers should not be included in the list of timers 
    controlled by this API.  
    Virtual timers require the use of two physical timers.  Setting preallocUtilTimers 
    to true forces these timers to be allocated statically at the Open call.  Setting it to
    false (default) will allocate these timers dynamically on first request for a virtual timer.
    The base register value defines the alternate set of timers to be used by the returned handle.  
    The interrupt number defines the level two interrupt that is registered for the alternate set
    of timers.  This flags the driver into handling the individual timers as level 3 interrupts.

Usage:
    When using an alternate timer block the following settings should be used to define the
    alternate timer block.  It is assumed that if the interrupt number parameter is defined that
    the specified interrupt will be used as a level two interrupt.
<verbatim>
    BTMR_GetDefaultDeviceSettings(&deviceSettings);
    deviceSettings.baseRegister = BCHP_DVP_HR_TMR_TIMER_IS;
    deviceSettings.interruptNumber = BCHP_INT_ID_TMR_IRQ_0;
    BTMR_Open(&hTimerDevice, hChip, hReg, hInt, &deviceSettings);
</verbatim>

See Also:
    BTMR_Open().

****************************************************************************/
typedef struct BTMR_DeviceSettings {
    unsigned timerMask;    /* Bitmask which specifies which timers are already in use by outside code. */ 
                           /* This allows OS or customer specific code to use the hardware timers outside of this module. 
                              To prevent the TMR module from using a timer set the corresponding bit to 1.
                              For example, to exclude timer number 3 set the timerMask value to 0x8. */
    bool preallocUtilTimers; /* Option to pre-allocate hardware timers required for use with virtual timers (default is false). */

    /* The following parameters are used to support an alternate TMR block. */
    uint32_t baseRegister;    /* This is the address of the base register for this timer block (default is BCHP_TIMER_REG_START). */
    unsigned interruptNumber; /* This is the level 2 interrupt number that should be used to register for a callback interrupt. */

} BTMR_DeviceSettings;

#define BTMR_DefaultSettings BTMR_DeviceSettings /* depricated name -- delete this later */
#define BTMR_GetDefaultSettings BTMR_GetDefaultDeviceSettings /* depricated name -- delete this later */

/***************************************************************************
Summary:
    Enumeration for timer mode.

Description:
    This enumeration defines the timer mode.

See Also:
    BTMR_CreateTimer().

****************************************************************************/
typedef enum BTMR_Type {
    BTMR_Type_eCountDown,     /* Timer starts at a specified value and counts down to zero.
                                 Must be restarted once triggered (if desired).
                                 Restrictions:
                                 The initial value when starting the timer must not be zero!
                                 This timer type requires a call-back function (called when timer expires).
                              */
    BTMR_Type_ePeriodic,      /* Like countdown but does not need to be restarted once triggered.
                                 Will continue to fire (callback) until stopped at interval defined by initial value.
                                 This timer type requires a call-back function (called each time timer expires).
                                 Restrictions:
                                 This timer must be supplied a timeout greater than one millisecond (a smaller
                                 values could hang the system).
                              */
    BTMR_Type_eStopWatch,     /* Free running stopwatch.
                                 Once started it continues to count up (and wraps) until stopped.
                                 The initial value defines the timers starting value.
                                 Restrictions:
                                 No callback function is allowed!
                              */
    BTMR_Type_eSharedFreeRun  /* This is special case of the free running stopwatch timer.
                                 Multiple creates of this timer type use a single shared timer.
                                 Restrictions:
                                 Once created it cannot be stopped or started (only read and destroyed).
                                 Destroying this timer does NOT affect the other shared requestors.
                                 No callback function is allowed!
                              */
} BTMR_Type;


/***************************************************************************
Summary:
    Structure for Timer Information.

Description:
    This defines the timer information.
    Callback function cannot be supplied when timer type is StopWatch (free-run)!
    The two parameters (pParm1 & pParm2) will be passed to the callback function when the timer triggers.

    In exclusive mode, the timer is not to be virtualized (each create uses hardware timer).

See Also:
    BTMR_CreateTimer(), BTMR_GetTimerSettings(), BTMR_GetTimerRegisters().

****************************************************************************/
typedef struct BTMR_TimerSettings {
    BTMR_Type type;             /* Timer type. */
    BTMR_CallbackFunc cb_isr;   /* Callback used for count-down type timers (can only call '_isr' functions from callback). */
    void *pParm1;               /* Context parameter returned when callback is executed. */
    int parm2;                  /* Context parameter returned when callback is executed. */
    bool exclusive;             /* Requestor wants exclusive access (not virtual and not shared). */
} BTMR_TimerSettings;

#define BTMR_Settings BTMR_TimerSettings /* depricated name -- delete this later */

/***************************************************************************
Summary:
    Structure for Timer Register Information.

Description:
    This defines the offsets returned on a register request when using exclusive mode.
    These can then be used to manually control the timer.

See Also:
    BTMR_GetTimerRegisters().

****************************************************************************/
typedef struct BTMR_TimerRegisters {
    uint32_t control; /* chip offset to register for timer control */
    uint32_t status;  /* chip offset to register for timer status */
} BTMR_TimerRegisters;


/***************************************************************************
Summary:
    Definition for minimum periodic timeout value.

Description:
    If a periodic timer is created with too small a timeout value it could easily cause a system hang!
    The timer would expire and before the callback function had finished the timer expires again (calling the callback ...)

See Also:
    BTMR_CreateTimer(), BTMR_ERR_TIMEOUT_TOO_SMALL

****************************************************************************/
#define BTMR_MINIMUM_TIMEOUT 100 /* (microseconds) minimum timeout value for periodic timer */

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This function opens and creates the base timer handle.

Description:
    This function is responsible for creating a set of timers for general purpose useage.
    This function will register for the timer interrupts.
    This is typically created at the system level and the resulting handle is passed to specific modules.

Returns:
    A valid handle that can be used to create/close on success.
    Status of the request.

See Also:
    BTMR_Close(), BTMR_CreateTimer().

****************************************************************************/
BERR_Code BTMR_Open( 
    BTMR_Handle *phDevice,                   /* [out] Return device handle on successful open */
    BCHP_Handle hChip,                       /* [in] Chip handle */
    BREG_Handle hRegister,                   /* [in] Register handle */
    BINT_Handle hInterrupt,                  /* [in] Interrupt handle */
    const BTMR_DeviceSettings *pOpenSettings /* [in] Settings */
    );

/***************************************************************************
Summary:
    This function closes a previously opened timer handle.

Description:
    This function is responsible for releasing a previously opened set of timers.
    This function will deregister for the interrupts for these timers.
    This is used th undo the actions of the open.
    Any valid timers (created but not yet destroyed) will be destroyed.

Returns:
    The status of the operation.

See Also:
    BTMR_Open().

****************************************************************************/
BERR_Code BTMR_Close(
    BTMR_Handle hDevice /* [in] Device handle from open */
    ); 

/***************************************************************************
Summary:
    This function returns the default device settings.

Description:
    This function is responsible for providing the default values for the device settings.

Returns:
    A copy of the default settings and the status of the request.

See Also:
    BTMR_Open().

****************************************************************************/
BERR_Code BTMR_GetDefaultDeviceSettings( 
    BTMR_DeviceSettings *pSettings /* [out] Default settings */
    );

/***************************************************************************
Summary:
    This function creates a timer instance.

Description:
    This function is responsible for creating a timer for general or exclusive
    use.  The timer is defined by the settings passed in.  Once created, the
    timer can be started, stopped, and read until no longer needed (destroyed).
    This function will call the supplied callback function when the timer expires.

Returns:
    A timer handle that can be used to control the allocated timer on successful creation.
    The status of request.

See Also:
    BTMR_Open(), BTMR_DestroyTimer().

****************************************************************************/
#if BDBG_DEBUG_BUILD
BERR_Code BTMR_CreateTimer_tagged( 
    BTMR_Handle hDevice,                  /* [in] Device handle from open */
    BTMR_TimerHandle *phTimer,            /* [out] Returns timer handle */
    const BTMR_TimerSettings *pSettings,  /* [in] Timer settings */
    const char *file, int line
    );
#define BTMR_CreateTimer(d,t,s) BTMR_CreateTimer_tagged(d,t,s,BSTD_FILE,BSTD_LINE)
#else
BERR_Code BTMR_CreateTimer(
    BTMR_Handle hDevice,                  /* [in] Device handle from open */
    BTMR_TimerHandle *phTimer,            /* [out] Returns timer handle */
    const BTMR_TimerSettings *pSettings   /* [in] Timer settings */
    );
#endif

/***************************************************************************
Summary:
    This function destroys a timer instance.

Description:
    This function is responsible for undoing the effect of creating a new timer.
    If the indicated timer is still running, it will be stopped before it is destroyed.

Returns:
    The status of request.

See Also:
    BTMR_CreateTimer().

****************************************************************************/
#if BDBG_DEBUG_BUILD
void BTMR_DestroyTimer_tagged(
    BTMR_TimerHandle hTimer, /* [in] Timer handle from create */
    const char *file, int line
    );
#define BTMR_DestroyTimer(d) BTMR_DestroyTimer_tagged(d,BSTD_FILE,BSTD_LINE)
#else
void BTMR_DestroyTimer(
    BTMR_TimerHandle hTimer  /* [in] Timer handle from create */
    );
#endif

/***************************************************************************
Summary:
    This function returns a set of defaults for the timer settings.

Description:
    This is used to initialize the settings parameter used to create a timer.
    After calling this, the settings should be modified to reflect the required
    functional aspects for the new timer.

Returns:
    A set of default settings for a timer.
    The status of request.

See Also:
    BTMR_CreateTimer().

****************************************************************************/
BERR_Code BTMR_GetDefaultTimerSettings(
    BTMR_TimerSettings *pSettings  /* [out] A set of defaults for the settings parameter to create */
    );

/***************************************************************************
Summary:
    Returns a structure that supplies the registers for this timer.

Description:
    When a timer is created in exclusive mode, it will not be controlled by timer start/stop/read functions.
    Instead, it will be controlled externally.  If a callback function is supplied, then, when an interrupt
    for that timer is generated, the call back function will be called.

Note:
    This function can also be used on a shared timer (BTMR_Type_eSharedFreeRun).  Because this timer is
    shared by other users, care should be taken to NOT start or stop this timer.  If you get the registers
    and stop the timer other user's code will break.  This was done because of the limited number of timers
    available.

Returns:
    A structure that contains the offsets for the registers for this timer.
    The status of request.

See Also:
    BTMR_CreateTimer(), BTMR_DestroyTimer().

****************************************************************************/
#if BDBG_DEBUG_BUILD
BERR_Code BTMR_GetTimerRegisters_tagged( 
    BTMR_TimerHandle hTimer,          /* [in] Timer handle from create */
    BTMR_TimerRegisters *pRegisters,  /* [out] Pointer to a set of registers to control this timer. */
    const char *file, int line        /* used to find who is using this feature */
    );
#define BTMR_GetTimerRegisters(t,r) BTMR_GetTimerRegisters_tagged(t,r,BSTD_FILE,BSTD_LINE)
#else
BERR_Code BTMR_GetTimerRegisters(
    BTMR_TimerHandle hTimer,          /* [in] Timer handle from create */
    BTMR_TimerRegisters *pRegisters   /* [out] Pointer to a set of registers to control this timer. */
    );
#endif

/***************************************************************************
Summary:
    This function returns the current timer settings.

Description:
    This function is responsible for providing a copy of the settings used to create the timer.

Returns:
    The current values being used by the timer.
    The status of request.

See Also:
    BTMR_CreateTimer().

****************************************************************************/
BERR_Code BTMR_GetTimerSettings( 
    BTMR_TimerHandle hTimer,      /* [in] Timer handle from create */
    BTMR_TimerSettings *pSettings /* [out] Current timer settings */
    );

/***************************************************************************
Summary:
    This function starts a timer with an initial value.

Description:
    This function is responsible for starting a previously created timer.
    This function cannot be called on timers that were created exclusive or shared.

    If the timer is a countdown or perodic timer, the counter will start at the initial value and count-down to zero.
    If the timer is a stopwatch or free run it will start at the initial value and count in the positive direction and wrap at the zero point.
    This function cannot be called on timers that were created shared.
    The initial value is in microseconds and cannot be zero for count-down timer types.

Returns:
    The status of request.

See Also:
    BTMR_CreateTimer(), BTMR_StopTimer(), BTMR_StartTimer_isr().

****************************************************************************/
BERR_Code BTMR_StartTimer( 
    BTMR_TimerHandle hTimer,   /* [in] Timer handle from create */
    unsigned initialValue      /* [in] Starting time value in microseconds */
    );

/***************************************************************************
Summary:
    This function starts a timer with an initial value from interrupt context.

Description:
    This is identical to the BTMR_StartTimer function but is called from interrupt context.
    This includes being called from timer callback functions (allowed).
    This function cannot be called on timers that were created shared.
    The same calling rules apply as BTMR_StartTimer.

Returns:
    The status of request.

See Also:
    BTMR_CreateTimer(), BTMR_StartTimer().

****************************************************************************/
BERR_Code BTMR_StartTimer_isr( 
    BTMR_TimerHandle hTimer,   /* [in] Timer handle from create */
    unsigned initialValue      /* [in] Starting time value in microseconds */
    );

/***************************************************************************
Summary:
    This function stops a timer.

Description:
    This function is responsible for stopping a previously started timer.
    This function cannot be called on timers that were created shared.

Returns:
    The status of request.

See Also:
    BTMR_CreateTimer(), BTMR_StartTimer(), BTMR_StopTimer_isr().

****************************************************************************/
void BTMR_StopTimer(
    BTMR_TimerHandle hTimer /* [in] Timer handle from create */
    );
  

/***************************************************************************
Summary:
    This function stops a timer from interrupt context.

Description:
    This is identical to the BTMR_StopTimer function but is called from interrupt context.
    This includes being called from timer callback functions (allowed).
    This function cannot be called on timers that were created shared.
    The same calling rules apply as BTMR_StopTimer.

Returns:
    The status of request.

See Also:
    BTMR_CreateTimer(), BTMR_StopTimer().

****************************************************************************/
void BTMR_StopTimer_isr(
    BTMR_TimerHandle hTimer /* [in] Timer handle from create */
    );
  

/***************************************************************************
Summary:
    This function reads the current timer value.

Description:
    This function is responsible for returning the current value of the timer.
    If reading a stopped timer then the last value of the timer before the
    stop will be returned.  An exception to that is a CountDown timer that
    has expired -- it will return 0.
    The timer must be started before valid values are returned.

Returns:
    The timer value.
    The status of request.

See Also:
    BTMR_CreateTimer().

****************************************************************************/
BERR_Code BTMR_ReadTimer( 
    BTMR_TimerHandle hTimer,  /* [in] Timer handle from create */
    unsigned *pValue          /* [out] Current timer value in microseconds */
    );

/***************************************************************************
Summary:
    This function reads the current timer value from interrupt context.

Description:
    This is identical to the BTMR_ReadTimer function but is called from interrupt context.
    This includes being called from timer callback functions (allowed).
    The same calling rules apply as BTMR_ReadTimer.

Returns:
    The timer value.
    The status of request.

See Also:
    BTMR_CreateTimer().

****************************************************************************/
BERR_Code BTMR_ReadTimer_isr( 
    BTMR_TimerHandle hTimer,  /* [in] Timer handle from create */
    unsigned *pValue          /* [out] Current timer value in microseconds */
    );

/***************************************************************************
Summary:
    This function returns the maximum timer value.

Description:
    This function is responsible for returning the maximum value that the
    timer will reach before wrapping back to zero.  The value returned is
    in microseconds.  This should be used to perform timer wrap calculations.

Returns:
    The maximum timer value.

****************************************************************************/
unsigned BTMR_ReadTimerMax_isrsafe(void);

#define BTMR_ReadTimerMax BTMR_ReadTimerMax_isrsafe /* depricated name -- delete this later */

/***************************************************************************
Summary:
    This function is used to save the state of the timers on entering S3 power mode.

Description:
    This function is responsible for saving the state of the timers when entering
    S3 power save mode.  The recipricle function BTMR_Resume should be used to restore
    from a enter.

Returns:
    The status of request.

See Also:
    BTMR_Open(), BTMR_Resume().

****************************************************************************/
BERR_Code BTMR_Standby(
    BTMR_Handle hDevice /* [in] Device handle from open */
    );

/***************************************************************************
Summary:
    This function is used to restore the state of the timers on exiting S3 power mode.

Description:
    This function is responsible for restoring the state of the timers when exiting
    S3 power save mode.  The recipricle function BTMR_Standby should be used to save
    before calling this.

Returns:
    The status of request.

See Also:
    BTMR_Open(), BTMR_Standby().

****************************************************************************/
BERR_Code BTMR_Resume(
    BTMR_Handle hDevice /* [in] Device handle from open */
    );

#ifdef __cplusplus
}
#endif
 
#endif /* BTMR_H__ */

