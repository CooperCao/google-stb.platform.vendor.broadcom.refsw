/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *  This is a magnum PI module used to control the hardware timers of a BCM7xxx part.
 *  This provides a means to create/destroy and start/stop timers when required.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bint.h"
#include "bchp_common.h"
#include "bchp_timer.h"
#include "bchp_int_id_timer.h"
#include "blst_list.h"  /* double linked list code */
#include "blst_squeue.h" /* single linked queue code */
#include "btmr.h"

/* Can't use std includes -- causes conflicts on some builds */
/*#include <stdio.h>*/
/*#include <string.h>*/

#include "bdbg.h"
BDBG_MODULE(btmr);

/* Note:
** This file contains two complete sets of timer control functions.
** That is, there is one set to control exclusive and/or shared timers (phys_) and another to control virtual timers (virt_).
** The published interface functions test which type of timer is being used and calls the appropriate set.
*/

/* Shared Timer Notes:
** The concept of a shared timer was added prior to adding virtual timers.  This allowed users to request a physical timer
** for reading only (delay operations) but share it with other requestors.  A physical timer is used for this purpose.
** Since the address to this timer is returned to each requestor (that wants to read the time without using this PI) 
** it is possible for any of the users to stop the timer -- this would be bad!  When virtual timers were introduced it was 
** decided to keep the shared timer as there is firmware in ancillary processors/dsps that can't use the PI but can read the 
** timer registers.  This lets them use a timer without having to use a unique physical timer.
*/

/* Virtual Timer Notes:
** This interface supports two types of virtual timers: stop-watch (or free-run) and count-down.
** One physical timer is used for the virtual count-down timer and the shared timer is used to support the virtual free-run timers.
** The physical timer for count-down timers is ONLY used to generate the interrupt.  All the time calculations are done
** using the shared free-run timer (for both the virtual count-down and virtual free-run timers).
** 
** When a count-down timer is pending, the physical timer counts the virtual count-down timer that has the smallest count-down period.
** A sorted list of the count-down timers is managed.  When a count-down timer is added to the list it is added in the list
** based on its timeout time before an item that has a larger timeout period.  The difference between that item and its
** preceeding entry defines how long the timeout needs to be for that timer.
** 
** The timer for free-run virtual timers is started at open time and the current time is used to provide the caller
** with a virtual time value (using the time when the user created the virtual timer).
*/

/* TODO:
** 1) Instead of using two timers to support virtual timers, try to use the free-run timer for the timeout by just letting it run 
** but set a new match value for the next timeout.  The pain would be in the inserting of a new virtual count-down timer that is
** less (smaller timeout) than the one that is currently running.  To calculate the time already run would be a bit more math.
** 2) Allow reading the registers for a virtual stop-watch timer (return the registers for the free-run timer used for supporting
** the virtual timers).  Then convert the create request for a shared timer to a virtual stop-watch and delete the concept of a
** shared timer from the documentation (depricated).
** 3) BTMR_ReadTimer should not return status as it can never fail -- return value instead.  This would require a change to the PI.
** 4) Change BTMR_ReadTimerMax to allow specifying a handle in case future timers blocks have different wrap points.
*/

/**** Compile-time options ****/

/*#define _FAIL_ON_CLOSE_*/ /* enable to cause close to fail on resource leaks (timers not returned). */
    /* The above is useful when users aren't looking at the error messages on close failures.
    ** Without the fail the user can over-look the issue and not do something about it.
    ** With this enable, BDBG_ASSERTS will occur when handles are not returned on close!
    */
/*#define _DESTROY_ON_CLOSE_*/ /* enable to cause close to try to clean up the resource leaks. */
    /* The above will try to destroy all the timers that were left pending.
    ** The main benefit is that we'll return all the memory we allocated (on user's behalf) and will destroy any pending handles.
    ** This means that if anyone tries to use any of the timer functions after the close that we'll assert on the handle.
    ** Note that this is wasted effort if you enable the _FAIL_ON_CLOSE_ option above.
    */

/*#ifdef _FAIL_ON_STANDBY_*/ /* enable to cause standby to fail if any timers are active on enting stand-by mode. */
    /* This should be a requirment in order to "find" the resources that are not freeing up their timers before we enter stand-by.
    ** The reason we need timers to be released is because we can't restore a timer.  That is, we can set a starting timer value.
    ** So any timer that was active before the standby will NOT be restored in the resume.
    */
/*#define _DESTROY_ON_STANDBY_*/ /* enable to cause standby to try to clean up the resource leaks. */
    /* The above will try to destroy all the timers that were left pending.
    ** The main benefit is that we'll return all the memory we allocated (on user's behalf) and will destroy any pending handles.
    ** This means that if anyone tries to use any of the timer functions after the resume that we'll assert on the handle.
    ** Note that this is wasted effort if you enable the _FAIL_ON_STANDBY_ option above.
    */

/* Define a minimum timeout period where its just not prudent to wait for the timer to timeout -- i.e. close enough!
** The danger here is that if they create a timer with less than the minimum hold time then we'll continue to
** process that timer in this loop.  So, if you want to use the MIN_HOLD you MUST ensure that no periodic timer gets
** created with a timeout period less than this timeout or we'll just hang!
** Note: this is different than the BTMR_MINIMUM_TIMEOUT value that decides a minimum value for periodic timers.
*/
#if 1
#define BTMR_MIN_HOLD 27 /* this is microsecond->timer conversion value */
#else
#define BTMR_MIN_HOLD (27*5)
#endif

/**** Misc definitions ****/

#define NumberOfTimers 4 /* this hardware has this many instances of this physical timer */
/* Note: This definition should be made into a platform definition should any platform implement a different number of sequential timers. */

#define TimerWrap     (1<<30)        /* timer wraps at this count (is 39.7 seconds) */
#define MaxTimerValue (TimerWrap-1)  /* the max timer count is one less than the wrap point */

#define TimerSelect(timer) (1<<(timer)) /* the timer is selected by the bit number */

#if (BCHP_CHIP == 11360)
/* Platform does not have a BCG-compatible timer
 * Ignore any reads/writes
*/
#undef BREG_Read32
#undef BREG_Write32
#define BREG_Read32
#define BREG_Write32
#endif

/* Forward reference */
static BERR_Code _BTMR_CreateTimer(BTMR_Handle device, BTMR_TimerHandle *phTimer, const BTMR_TimerSettings *pSettings, const char *file, int line);
static BERR_Code _BTMR_DestroyTimer(BTMR_TimerHandle timer, const char *file, int line);

/* There are different entry points for each of the timer handler functions based on whether the timer is a physical or virtual timer.
** Rather than make the decision each time the function is called I use a table to use the correct function.
** This table gets set at the timer creation.
*/
typedef struct {
    BERR_Code (*create)(BTMR_Handle device, BTMR_TimerHandle timer, const BTMR_TimerSettings *pSettings);
    BERR_Code (*destroy_isr)(BTMR_TimerHandle timer);
    BERR_Code (*start_isr)(BTMR_TimerHandle timer, unsigned startingValue);
    BERR_Code (*stop_isr)(BTMR_TimerHandle timer);
    unsigned (*read_isr)(BTMR_TimerHandle timer);
} ProcessingFunctions;

BDBG_OBJECT_ID(btmr_timer_t);

/* This is the data associated with an individual timer.
** An opaque pointer to one of these (handle) is returned on a successful create timer request.
*/
struct BTMR_P_TimerContext {
    BDBG_OBJECT(btmr_timer_t)    /* used to check if structure is valid */

    BTMR_Handle device;          /* the device handle for this timer (passed in on create) */
    BTMR_TimerSettings Settings; /* the settings passed in for this timer (passed in on create) */
    uint32_t initialValue;       /* how long should this timer run (passed in on start) */

    BLST_D_ENTRY(BTMR_P_TimerContext) created; /* for linking queue of created timers */
    const char *file; int line; /* this is location where timer was created (will help in tracking down missing destroys) */

    BTMR_TimerRegisters Registers; /* the offsets to the control and status registers */

    ProcessingFunctions *functions; /* the table of processing functions for this type of timer (virtual versus physical) */

    int timerNumber;    /* this indicates which timer was allocated (-1 for virtual timers) */
    uint32_t lastValue; /* this is the last value of the timer when it was stopped */
    bool cbEnabled;     /* indicates that I enabled the interrupt in the create */

    BLST_D_ENTRY(BTMR_P_TimerContext) link; /* for linking onto a virtual timeout queue */
    BLST_SQ_ENTRY(BTMR_P_TimerContext) qlink; /* for linking onto ToBeProcessed queue */

    uint32_t startingValue;  /* virtual timers need to record the value of the timer when it was started */
    uint32_t adjustedValue;  /* how long should this timer run when put on virtual timeout (difference from the timer before it) */

    bool running;     /* flag, so we know if the timer is already running -- shouldn't start a running or stop a stopped timer */
    bool processing;  /* this timer is being processed so don't try to stop or delete */

    volatile bool pleaseStop;  /* this periodic timer was stopped in the callback so don't restart it */
    volatile bool pleaseStart; /* this count-down timer was started in the callback so restart it */
    unsigned delayedValue;     /* the starting value when asked to (re)start from a call-back handler */
};

BDBG_OBJECT_ID(btmr_device_t);

/* This is the data associated with the timer module (group of timers).
** An opaque pointer to one of these (handle) is returned on a successful module open call.
*/
struct BTMR_P_Context {
    BDBG_OBJECT(btmr_device_t) /* used to check if structure is valid */
    BCHP_Handle hChip;         /* the chip handle (passed in on open) */
    BREG_Handle hRegister;     /* the register handle (passed in on open) */
    BINT_Handle hInterrupt;    /* the interrupt handle (passed in on open) */

    BTMR_DeviceSettings Settings; /* this is the list of available timers -- provided on request */
    uint32_t inUse;               /* bitmask which specifies which timers are already in use by this code */ 

    BTMR_TimerHandle Timers[NumberOfTimers];      /* list of allocated timers (so we can stop/destroy them on close) */
    BINT_CallbackHandle CbHandle[NumberOfTimers]; /* interrupt callbacks for each timer we control */

    BINT_CallbackHandle alt_CbHandle; /* interrupt callbacks for secondary block if interrupt is handled as Level 3 */
    unsigned int alt_CbEnabled; /* flag to indicate whether alternate callback is enabled (enable on 0->1 transition, disable on 1->0 transition) */

    BTMR_TimerHandle SharedTimer;    /* timer for shared timer use */
    BTMR_TimerHandle CountDownTimer; /* timer for managing virtual count-down timers */
    BTMR_TimerHandle FreeRunTimer;   /* timer for managing virtual free-run timers */

    BLST_D_HEAD(TOL, BTMR_P_TimerContext) TimeoutList; /* the list of vitrual count-down/periodic timers waiting to timeout */
    BLST_SQ_HEAD(TBPL, BTMR_P_TimerContext) ToBeProcessedList; /* the list of vitrual count-down/periodic timers waiting to be processed */

    BKNI_MutexHandle create_destroy_mutex; /* semaphore to protect against re-entrant code */

    unsigned int SharedCount;  /* count of users using this shared timer (destroy when count goes to 0) */
    unsigned int VirtualCount; /* count of outstanding virtual timers (for displaying error on close) */

    BLST_D_HEAD(VTCL, BTMR_P_TimerContext) VirtualCreateList; /* the list of vitrual timers created */

    bool in_standby_mode;     /* flag to indicate when we're in standby mode (don't let users play with timers in this state) */
    bool destroyed_utilities; /* flag to indicate that we destroyed the utility timers on standby and need to recreate on resume */
#if 0
    /* Save current settings on stand-by for restoring after resume */
    uint32_t control_registers[NumberOfTimers];
    uint32_t status_registers[NumberOfTimers];
    uint32_t is_reg;
    uint32_t ie_reg;
#endif
};

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* The timers run at 27Mhz (27,000,000 times a second) and a microsecond is 1,000,000th of a second, so ... */
#define ConversionFactor (27000000/1000000) /* <- yeah, I know, its 27 -- but the compiler will figure that out, right? */
#define TimerToMicroSeconds(timerValue)   ((unsigned)((unsigned)(timerValue) / ConversionFactor))
#define MicroSecondsToTimer(microSeconds) ((unsigned)((unsigned)(microSeconds) * ConversionFactor))

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* These are used to get the timer control/status register for specific timer set/number */
#define CONTROL_REG(device, timer) \
    (((device)->Settings.baseRegister + (BCHP_TIMER_TIMER0_CTRL-BCHP_TIMER_REG_START)) + ((int)(timer) * (BCHP_TIMER_TIMER1_CTRL-BCHP_TIMER_TIMER0_CTRL)))
#define STATUS_REG(device, timer)   \
    (((device)->Settings.baseRegister + (BCHP_TIMER_TIMER0_STAT-BCHP_TIMER_REG_START)) + ((int)(timer) * (BCHP_TIMER_TIMER1_STAT-BCHP_TIMER_TIMER0_STAT)))

/* These are used to get the interrupt status and enable/disable the interrupts when using a secondary timer block */
#define TIMER_IS_REG(device) \
    ((device)->Settings.baseRegister + (BCHP_TIMER_TIMER_IS-BCHP_TIMER_REG_START))
#define TIMER_IE_REG(device) \
    ((device)->Settings.baseRegister + (BCHP_TIMER_TIMER_IE0-BCHP_TIMER_REG_START))

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* This allocates one of the unused timers and returns its timer number (returns -1 if none available) */
/* We start from last to first in case someone uses a timer (assuming they picked 0) outside of this PI and didn't tell us! */
static int SelectAnAvailableTimer_isr(BTMR_Handle device)
{
    int timer, selected=-1;
    for (timer=NumberOfTimers-1; timer >= 0; timer--)
    {
        if (!(device->inUse & TimerSelect(timer))) 
        {
            device->inUse |= TimerSelect(timer); /* this timer is now in use (should anybody ask) */
            selected = timer; /* this is the timer I've selected just for you */
            break;
        }
    }
    return selected;
}

/* This frees a specific (selected) timer (makes it available again) */
static void ReleaseThisTimer_isr(BTMR_Handle device, int timerNumber)
{
    device->inUse &= ~TimerSelect(timerNumber); /* not in use anymore */
}

#if BDBG_DEBUG_BUILD
static char *_GetTimerType_isrsafe(bool exclusive, BTMR_Type type)
{
    if (type == BTMR_Type_eSharedFreeRun) return "Shared";
    if (exclusive)
    {
        switch (type) {
            case BTMR_Type_eCountDown:     return "Exclusive/CountDown";
            case BTMR_Type_ePeriodic:      return "Exclusive/Periodic";
            case BTMR_Type_eStopWatch:     return "Exclusive/StopWatch";
            default: return "Unknown";
        }
    }
    else
    {
        switch (type) {
            case BTMR_Type_eCountDown:     return "Virtual/CountDown";
            case BTMR_Type_ePeriodic:      return "Virtual/Periodic";
            case BTMR_Type_eStopWatch:     return "Virtual/StopWatch";
            default: return "Unknown";
        }
    }
}
static char *GetTimerType_isrsafe(const BTMR_TimerSettings *pSettings) { return _GetTimerType_isrsafe(pSettings->exclusive, pSettings->type); }
#endif /*BDBG_DEBUG_BUILD*/

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* This talks to the hardware to enable the specified timer */
static void EnableTimer_isr(BTMR_TimerHandle timer)
{
    uint32_t control_value, timeout;
    /*uint32_t was = BREG_Read32_isr(timer->device->hRegister, timer->Registers.control);*/

    /* Note: we always use Free-run mode!
    ** I haven't figured out the difference between a count-down and free-run timer.
    ** They seem to function exactly the same (count up to matching value, interrupt, and restart at 0).
    */
    
    if (timer->Settings.type == BTMR_Type_eStopWatch || timer->Settings.type == BTMR_Type_eSharedFreeRun) {
        /* Stop-watch timers count up until they wrap */
        /* Counts up from 0 to the maximum timer value that the timer can handle (then it resets to zero and continues to count up) */
        timeout = MaxTimerValue;
    } else {
        /* Count-down and Periodic timers count up to matching value */
        /* This is the same as if they STARTED at the initial value and counted down (since I can't get count-down mode to work) */
        timeout = timer->initialValue;
    }
    
    control_value = BCHP_MASK(TIMER_TIMER0_CTRL,ENA) | (BCHP_FIELD_DATA(TIMER_TIMER0_CTRL, TIMEOUT_VAL, timeout) & BCHP_MASK(TIMER_TIMER0_CTRL,TIMEOUT_VAL));

    /*BDBG_MSG(("EnableTimer_isr: Writing 0x%08x (was 0x%08x) to control register (offset=%08x)", control_value, was, timer->Registers.control));*/
    BREG_Write32_isr(timer->device->hRegister, timer->Registers.control, control_value);
    timer->running = true;
}

/* This talks to the hardware to disable the specified timer */
static void DisableTimer_isr(BTMR_TimerHandle timer)
{
    uint32_t control_value, was;

    control_value = was = BREG_Read32_isr(timer->device->hRegister, timer->Registers.control);
    control_value &= ~BCHP_MASK(TIMER_TIMER0_CTRL,ENA);

    /*BDBG_MSG(("DisableTimer_isr: Writing 0x%08x (was 0x%08x) to control register (offset=%08x)", control_value, was, timer->Registers.control));*/
    BREG_Write32_isr(timer->device->hRegister, timer->Registers.control, control_value);
    timer->running = false;
}    

/* This is special case version that disables the timer BEFORE we have a timer structure created for this timer (i.e. at init time) */
static void DisableTimerNumber(BTMR_Handle device, int timerNumber)
{
    uint32_t control_register = CONTROL_REG(device, timerNumber);
    uint32_t control_value, was;

    control_value = was = BREG_Read32_isr(device->hRegister, control_register);
    control_value &= ~BCHP_MASK(TIMER_TIMER0_CTRL,ENA);

    /*BDBG_MSG(("DisableTimerNumber: Writing 0x%08x (was 0x%08x) to control register (offset=%08x)", control_value, was, control_register));*/
    BREG_Write32(device->hRegister, control_register, control_value);
}        

/* This talks to hardware to get the timer value */
static uint32_t GetTimerValue_isr(BTMR_TimerHandle timer)
{
    uint32_t status_value = BREG_Read32_isr(timer->device->hRegister, timer->Registers.status);
    status_value &= MaxTimerValue; /* bits 30&31 are reserved but are sometimes set for some reason */
    return status_value;
}

/*
*******************************************************************************************************
*******************************************************************************************************
** These are the set of PHYSICAL (exclusive or shared) timer functions (and their utility functions).
*******************************************************************************************************
*******************************************************************************************************
*/

static BERR_Code JustStartTimer_isr(BTMR_TimerHandle timer, uint32_t initialValue)
{
    /* We don't allow starting a running timer (why not?) -- you must stop it first (???) */
    /* The problem here is that if they forgot to stop the timer and believe that this is actually
    ** starting the timer then they aren't going to get what they expected and won't know why.
    ** We have three choices here: 1) don't let them start a started timer, 2) stop the timer and restart
    ** with the new parameters, or 3) just say okay and keep going.
    */
    if (timer->running) {
#if 0
        BDBG_ERR(("BTMR_StartTimer: timer already running!"));
        return BERR_TRACE(BTMR_ERR_ALREADY_STARTED);
#else
        goto done; /* its already started -- success! (this is choice 3) */
#endif
    }

    /* Can't use an initial value of zero for count-down and periodic timers! */
    if ((timer->Settings.type == BTMR_Type_eCountDown || timer->Settings.type == BTMR_Type_ePeriodic) && !initialValue) {
        BDBG_ERR(("BTMR_StartTimer: can't use zero initial value with Count-down and Periodic timers!"));
        return BERR_TRACE(BTMR_ERR_NO_TIMEOUT_GIVEN);
    }

    /* We only have specific number of bits worth of timer value */
    if (initialValue >= MaxTimerValue) {
        BDBG_ERR(("BTMR_StartTimer: initial value too large!"));
        return BERR_TRACE(BTMR_ERR_TIMEOUT_TOO_LARGE);
    }

    timer->initialValue = initialValue;
    timer->lastValue = 0;
    
    EnableTimer_isr(timer);

  done:
    return BERR_SUCCESS;
}

static uint32_t JustReadTimer_isr(BTMR_TimerHandle timer)
{
    uint32_t runTime;
    
    /* Stopwatch (FreeRun) timers count up  -- just give them the value of the timer */
    runTime = GetTimerValue_isr(timer);

/*    printf("runtime = %d (0x%x) type=%s [initial = %d (0x%x)]\n",
           runTime, runTime, GetTimerType_isrsafe(&timer->Settings), timer->initialValue, timer->initialValue);
*/
    /* Count-down and Periodic timers count down (but don't let it go negative) */
    /* Note: we always use the hardware timer in a count UP mode (i.e. count up to matching value).
    ** If free-run then reading the timer IS the count.  In count-down we want the value of the timer
    ** (or what it WOULD be if it was really counting down).  Thus, we do some math and make sure its capped at zero.
    */
    if (runTime && (timer->Settings.type == BTMR_Type_eCountDown || timer->Settings.type == BTMR_Type_ePeriodic))
        runTime = (timer->initialValue > runTime) ? (timer->initialValue - runTime) : 0;

/*    printf("runtime = %d (0x%x) type=%s [initial = %d (0x%x)]\n",
           runTime, runTime, GetTimerType_isrsafe(&timer->Settings), timer->initialValue, timer->initialValue);
*/
    return runTime;
}

static BERR_Code JustStopTimer_isr(BTMR_TimerHandle timer)
{
    /* Should I let them stop a stopped timer??? */
    /* We have two choices here: 1) don't let them stop a stopped timer or 2) just say okay and keep going.  */
    if (!timer->running) {
#if 0
        BDBG_ERR(("BTMR_StopTimer: timer already stopped!"));
        return BERR_TRACE(BTMR_ERR_ALREADY_STOPPED);
#else
        goto done; /* its already stopped -- success! (this is choice 2) */
#endif
    }

    /* We're going to save off the last value of the timer when they stopped it.
    ** That way, if they read it AFTER it stopped we can tell them what it was.
    ** I'm pretty sure this is required as reading the status after stopping returns zero!
    */
    timer->lastValue = JustReadTimer_isr(timer);

    DisableTimer_isr(timer);

  done:
    return BERR_SUCCESS;
}

/*
** This is my ISR function for the physical Timers!
** Its registered at 'open' time, de-registered at 'close', enabled at 'create', and disabled at 'destroy'.
** This performs the callbacks to the owner of the count-down or periodic timer.
*/

static void Physical_Timer_Isr(BTMR_Handle device, int timerNumber)
{
    BTMR_TimerHandle timer;
    BTMR_TimerSettings *context;

    /* We're supposed to use asserts to catch logic errors (not run-time errors).
    ** So is getting an invalid handle or timer number a logic error??
    ** We should never be given a timerNumber for a timer that we don't control.
    */
#if 1
    BDBG_ASSERT(device);
    BDBG_OBJECT_ASSERT(device, btmr_device_t);
    BDBG_ASSERT((timerNumber >= 0) && (timerNumber < NumberOfTimers));
#else
    if (!device) {
        BDBG_ERR(("device parameter is null in ISR!"));
        return;
    }
#endif

	/* Always do this even with the assert above so we won't have a Coverity issue for bounds check of Timers array */
    if (timerNumber < 0 || timerNumber >= NumberOfTimers) {
        BDBG_ERR(("ISR received an invalid timer number (=%d)", timerNumber));
        return;
    }

    /*BDBG_WRN(("Physical interrupt (timer %d)", timerNumber));*/

#if 0
    /* We should never get an interrupt for a timer we don't control with this PI */
    if (device->Settings.timerMask & TimerSelect(timerNumber)) return;

    /* We should never get an interrupt for a timer we didn't open */
    if (!(device->inUse & TimerSelect(timerNumber))) return;
#endif

    /* Note: for the above cases, returning without handling the interrupt could mean we get called back forever, but not much else I can do... */

    /* Its been reported that we can be called (once??) at startup before anyone has created a timer.
    ** This is either an interrupt for a timer being used externally (and they neglected to indicate this in the open)
    ** or the timer was running and it generated an interrupt before the timer is created (why?).
    ** If we assert here they won't know why this it happening.  So don't use: BDBG_ASSERT(timer);
    ** But ignoring it is not great either -- in this case we're getting called for an interrupt we're not controlling
    ** and doing nothing with the interrupt.  I think this is the better choice (keeps the code from crashing).
    */
    timer = device->Timers[timerNumber];
    if (!timer) {
        BDBG_ERR(("timer is NULL in ISR!"));
        return;
    }

    /* Note: we cannot validate the timer with the following assert:
    **     BDBG_ASSERT(timer->running == true);
    ** This is because we allow for exclusive timers that are created through the PI but not started and stopped via the PI.  
    ** This means that if someone is using a timer outside of this PI and it causes an interrupt that I'll not know what to 
    ** do with the timer.  This is bad but there is no way to control this.
    */

    /* If the timer is being used externally, then I may not have an object created to handle this (unless we use this
    ** timer in the PI too).  This will catch anyone using this without telling me in the Open.
    */
    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);

    JustStopTimer_isr(timer);
    timer->lastValue = 0; /* count down timers always stop at zero */
    
    /* We flag this timer as processing in case they try to start/stop the timer in the call-back function */
    timer->processing = true;
    timer->running = false;

    /* If we have a callback function for this timer, then its time to call it! */
    /* Note: this is processing count-down timers so there had BETTER be a call-back function!!! */
    /* We ALWAYS call back with the timer stopped! */
    context = &timer->Settings;
    if (context->cb_isr)
        context->cb_isr(context->pParm1, context->parm2); 

    /* We're done processing this timer so its okay to stop or delete it now */
    timer->processing = false;
    
    /* Periodic timers continue to go off until stopped -- we stopped it earlier so start it up again! */
    /* They may have asked us to stop the timer in the callback.  We delayed this until done processing the call-back. */
    /* If they asked us to stop the timer then just don't start it back up. */
    if (context->type == BTMR_Type_ePeriodic && !timer->pleaseStop) {
        JustStartTimer_isr(timer, timer->initialValue);
        timer->running = true;
    }

    /* If they requested a count-down timer to be restarted in the call-back then do it now. */
    if (context->type == BTMR_Type_eCountDown && timer->pleaseStart) {
        BTMR_StartTimer_isr(timer, timer->delayedValue);
    }

    /* Make sure these are cleared before leaving (in case they tried to start a periodic or stop a countdown) */
    timer->pleaseStart = false;
    timer->pleaseStop  = false;
}

/*
** This PI supports two different timer blocks with two different interrupt methods.
** The primary (TMR) has a separate level two interrupt register for determining which of the four timers generated the interrupt.
** In the second method, this implementation is handled as a level three (the level one references a level two block).  All I am
** told in this method is one (or more) of the timers went off but not which one.  I have to figure that out myself by checking
** the level three timer interrupt status register and clearing the appropriate interrupt after processing.
*/

static void Physical_Timer_Selection_Isr(BTMR_Handle device)
{
    int timerNumber;
    uint32_t status, mask=0;

    /* This shouldn't be necessary because I didn't enable anything not being used, but ... */
    for (timerNumber=0; timerNumber < NumberOfTimers; timerNumber++) mask |= (1<<timerNumber);

    /* Figure out which one of the timers interrupted by reading the status register */
    status = BREG_Read32_isr(device->hRegister, TIMER_IS_REG(device)) & mask;
    if (!status) return;

    /* Clear the level three interrupts I'm about to process -- I'll work off the copy.
    ** By doing this before processing means that if any interrupt callback starts a timer that causes a NEW interrupt
    ** we won't kill that interrupt before leaving.  We'll get called right back, but we'll allow other interrupts
    ** to process before we do get called back.
    */
    BREG_Write32_isr(device->hRegister, TIMER_IS_REG(device), status);

    /* Process each of the timers sequentially */
    for (timerNumber=0; timerNumber < NumberOfTimers; timerNumber++)
    {
        if (status & (1<<timerNumber)) {
            /* Note: this print could print a LOT! */
            /*BDBG_MSG(("Processing interrupt for timer number (=%d)", timerNumber));*/
            Physical_Timer_Isr(device, timerNumber);
        }
    }

    /* Note: when I return the level two interrupt status will be cleared */
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

static BERR_Code phys_CreateTimer_isr(BTMR_Handle device, BTMR_TimerHandle timer, const BTMR_TimerSettings *pSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    int timerNumber;
    
    /* First, we have to select an available timer from the list of the available timers */
    BDBG_MSG(("BTMR_CreateTimer: looking for an available timer"));
    timerNumber = SelectAnAvailableTimer_isr(device);
    if (timerNumber < 0) {
        BDBG_ERR(("BTMR_CreateTimer: No timers available for create!"));
        retCode =  BERR_TRACE(BTMR_ERR_NO_TIMERS_AVAILABLE);
        goto done;
    }
    BDBG_MSG(("BTMR_CreateTimer: Using timer %d", timerNumber));
    
/* Enable callbacks only if the platform has a BCG-compatible timer */
#if (BCHP_CHIP != 11360)
    /* If they requested a callback (and are allowed to request a callback) then register for the interrupt! */
    if (pSettings->cb_isr)
    {
        if (device->alt_CbHandle)
        {
            uint32_t status;

            if (!device->alt_CbEnabled) 
            {
                /* This enables the level 2 interrupt for this timer */
                retCode = BINT_EnableCallback_isr(device->alt_CbHandle);
                if (retCode != BERR_SUCCESS) {
                    retCode = BERR_TRACE(retCode);
                    BDBG_ERR(("BTMR_CreateTimer: Enable Callback failed to register interrupt!"));
                    ReleaseThisTimer_isr(device, timerNumber);
                    goto done;
                }

                device->alt_CbEnabled++;
            }

            /* Enable the level 3 interrupt for this timer */
            /* I have to do this myself since this is a level 3 interrupt */
            status = BREG_Read32_isr(device->hRegister, TIMER_IE_REG(device));
            status |= 1<<timerNumber;
            BREG_Write32_isr(device->hRegister, TIMER_IE_REG(device), status);
        } 
        else 
        {
            retCode = BINT_EnableCallback_isr(device->CbHandle[timerNumber]);
            if (retCode != BERR_SUCCESS) {
                retCode = BERR_TRACE(retCode);
                BDBG_ERR(("BTMR_CreateTimer: Enable Callback failed to register interrupt (timer=%d)", timerNumber));
                ReleaseThisTimer_isr(device, timerNumber);
                goto done;
            }
        }

        timer->cbEnabled = true; /* remember we enabled this so we can disable in destroy */
        BDBG_MSG(("BTMR_CreateTimer: Enabled the callback for timer %d", timerNumber));
    }
#endif
    /* Physical timers have a timer value */
    timer->timerNumber = timerNumber;

    /* *** Special case: *** Ignore exclusive flag on shared timers!
    ** This lets anyone using the shared timer get its registers and stop it without notifying the others sharing the timer.
    ** This is bad, but necessary -- this lets us maximize the usage of the shared timer by hardware functions that need
    ** exclusive access but promise not to stop the timer.
    */
    if (pSettings->type == BTMR_Type_eSharedFreeRun)
        timer->Settings.exclusive = false;
    
    /* This is the offsets to the registers for this timer */
    /* When exclusive or shared access is requested we'll return these to the user (this is how they control this timer) */
    timer->Registers.control = CONTROL_REG(device, timerNumber);
    timer->Registers.status  = STATUS_REG(device, timerNumber);

    device->Timers[timerNumber] = timer; /* so we can destroy created timers on close */
    
    /* Just to be safe, make sure the timer isn't running until they specifically start it.
    ** Note that this will stop an external timer that they forgot to include in the open mask.
    ** Sorry, but there's nothing I can do about that ... you should have told me to exclude it.
    */
    DisableTimer_isr(timer);
    
    /* If they asked to create a Free Run timer then this is the first use of the shared timer */
    /* Start it ourselves -- they should NOT stop it (or restart it). */
    if (pSettings->type == BTMR_Type_eSharedFreeRun)
    {
        device->SharedTimer = timer;
        device->SharedCount = 1;
        JustStartTimer_isr(timer, 0);
    }

    BDBG_MSG(("BTMR_CreateTimer: Successfully created new physical timer!"));

  done:
    return (retCode);
}
static BERR_Code phys_CreateTimer(BTMR_Handle device, BTMR_TimerHandle timer, const BTMR_TimerSettings *pSettings)
{
    BERR_Code status;
    BKNI_EnterCriticalSection();
    status = phys_CreateTimer_isr(device, timer, pSettings);
    BKNI_LeaveCriticalSection();
    return status;
}

static BERR_Code phys_DestroyTimer_isr(BTMR_TimerHandle timer)
{
    BTMR_Handle device = timer->device;
    int timerNumber = timer->timerNumber;
    
    /* If I enabled the callback in the create then I need to disable it now */
    if (timer->cbEnabled) 
    {
        /* Don't leave the timer running -- we don't want any interrupts from this timer anymore */
        /* Hack: because I give out the registers for the shared timer and the fact that they can
        ** access those registers outside of this PI, I can't control how they use this timer.
        ** If they try to read the timer after I stop it they could hang.  There is no way for me
        ** to fix this or tell them this is happening.
        */
        if (timer->running) {
            BDBG_MSG(("BTMR_DestroyTimer: stopping timer!"));
            JustStopTimer_isr(timer);
        }

        if (device->alt_CbHandle)
        {
            uint32_t status;

            device->alt_CbEnabled--;

            /* Disable the level two interrupt if this is the last disable */
            if (!device->alt_CbEnabled)
                BINT_DisableCallback_isr(device->alt_CbHandle);

            /* Disable the level 3 interrupt for this timer */
            /* I have to do this myself since this is a level 3 interrupt */
            status = BREG_Read32_isr(device->hRegister, TIMER_IE_REG(device));
            status &= ~(1<<timerNumber);
            BREG_Write32_isr(device->hRegister, TIMER_IE_REG(device), status);
        }
        else
        {
            BINT_DisableCallback_isr(device->CbHandle[timerNumber]);
        }

        timer->cbEnabled = false;
        BDBG_MSG(("BTMR_DestroyTimer: Disabled the callback for timer %d", timerNumber));
    }

    /* On the last destroy we have no more shared timers -- will be re-created on next create */
    if (timer->Settings.type == BTMR_Type_eSharedFreeRun) {
        device->SharedTimer = NULL;
        device->SharedCount = 0;
        JustStopTimer_isr(timer);
    }

    device->Timers[timerNumber] = NULL; /* we're done with this timer -- its gone! */
    ReleaseThisTimer_isr(device, timerNumber);
    
    BDBG_MSG(("BTMR_DestroyTimer: Successfully destroyed physical timer!"));
    return BERR_SUCCESS;
}
#if 0
static BERR_Code phys_DestroyTimer(BTMR_TimerHandle timer)
{
    BERR_Code status;
    BKNI_EnterCriticalSection();
    status = phys_DestroyTimer_isr(timer);
    BKNI_LeaveCriticalSection();
    return status;
}
#endif

static BERR_Code phys_StartTimer_isr(BTMR_TimerHandle timer, unsigned startingValue)
{
    uint32_t initial;

#if 0 /* exclusive used to mean controlled externally -- now it means not virtual and don't share it */
    /* They are only supposed to call this function IF the timer is NOT exclusive */
    if (timer->Settings.exclusive) {
        BDBG_ERR(("BTMR_StartTimer: requested START operation on exclusive timer!"));
        return BERR_TRACE(BTMR_ERR_EXCLUSIVE_TIMER);
    }
#endif
    
    /* Starting a shared timer could mess up the others sharing this timer */
    if (timer->Settings.type == BTMR_Type_eSharedFreeRun) {
        BDBG_ERR(("BTMR_StartTimer: requested START operation on shared timer!"));
        return BERR_TRACE(BTMR_ERR_FREE_RUN_TIMER);
    }

    /* They talk microseconds, we talk timer ticks */
    initial = MicroSecondsToTimer(startingValue);

    return JustStartTimer_isr(timer, initial);
}

static BERR_Code phys_StopTimer_isr(BTMR_TimerHandle timer)
{
#if 0 /* exclusive used to mean controlled externally -- now it means not virtual and don't share it */
    /* They are only supposed to call this function IF the timer is NOT exclusive */
    if (timer->Settings.exclusive) {
        BDBG_ERR(("BTMR_StopTimer: requested STOP operation on exclusive timer!"));
        return BERR_TRACE(BTMR_ERR_EXCLUSIVE_TIMER);
    }
#endif
    
    /* Stopping a shared timer could mess up the others sharing this timer */
    if (timer->Settings.type == BTMR_Type_eSharedFreeRun) {
        BDBG_ERR(("BTMR_StopTimer: requested STOP operation on shared timer!"));
        return BERR_TRACE(BTMR_ERR_FREE_RUN_TIMER);
    }

    return JustStopTimer_isr(timer);
}

static unsigned phys_ReadTimer_isr(BTMR_TimerHandle timer)
{
    uint32_t runTime;

    /* Stopped timers return the value of the timer when it was stopped! */
    if (!timer->running) {
        runTime = timer->lastValue;
        BDBG_MSG(("BTMR_ReadTimer: (phys) read time from stopped timer (value=%x)", runTime));
    } else {
        runTime = JustReadTimer_isr(timer);
    }
 
    /* Stopwatch timers are relative to the initial value they supplied */
    if (timer->Settings.type == BTMR_Type_eStopWatch) {
        runTime += timer->initialValue;
        if (runTime >= TimerWrap) 
            runTime -= TimerWrap;
    }
    
    /* They talk microseconds, we talk timer ticks */
    return TimerToMicroSeconds(runTime);
}

/* These are the functions used by physical timers */
static const ProcessingFunctions physicalFunctions = { phys_CreateTimer, phys_DestroyTimer_isr, phys_StartTimer_isr, phys_StopTimer_isr, phys_ReadTimer_isr };

/*
***********************************************************************************
***********************************************************************************
** These are the set of VIRTUAL timer functions (and their utility functions).
***********************************************************************************
***********************************************************************************
*/

/* This uses the physical free-run timer to get the runtime of the virtual timer */
static uint32_t JustReadVirtualTimer_isr(BTMR_TimerHandle timer)
{
    BTMR_Handle device = timer->device;
    uint32_t currentTime, runTime, timeLeft;

    /* Start off with whatever the timer is currently at (currentTime) */
    /* This starts with whatever is in the actual timer (its counting up) */
    currentTime = GetTimerValue_isr(device->FreeRunTimer);

    /* Account for wrap by the physical timer */
    if (currentTime > timer->startingValue)
        runTime = currentTime - timer->startingValue;
    else
        runTime = (TimerWrap - timer->startingValue) + currentTime;

    if (timer->Settings.type == BTMR_Type_eStopWatch)
    {
        /* Stop Watch timers return the time expired (run-time). */
        /* Stop-watch timers can be supplied with a starting value -- account for that time! */
        runTime += timer->initialValue;
        if (runTime >= TimerWrap) 
            runTime -= TimerWrap;
/*printf("SW: current=%x, starting=%x, runTime=%x, initial=%x\n", currentTime, timer->startingValue, runTime, timer->initialValue);*/
        return runTime;
    }
    else
    {
        /* Count-down timers return the time left to run. */
        /* Count-down timers count down to zero -- make sure it stops at zero! */
        timeLeft = (timer->initialValue > runTime) ? (timer->initialValue - runTime) : 0;
/*printf("CD: current=%x, starting=%x, runTime=%x, initial=%x, timeleft=%x\n", currentTime, timer->startingValue, runTime, timer->initialValue, timeLeft);*/
        return timeLeft;
    }
}

/* A sorted list is kept for the virtual count-down timers.  The order determines who times out next! */
/* Each item on the list has an adjusted timeout value that is the time DIFFERENCE from the item before it. */

/* This puts items onto the sorted time-out list (on start timer request).
** We use a physical timer to timeout the smallest virtual timeout reqest.  If the new item's timeout 
** is smaller than the currently running timer (if any) then it will stop the physical timer,
** account for the time the timer already ran, and restart the timer with the new timeout.  The old
** (was running) timer's timeout value will be adjusted for time already processed (its already on the queue).
*/
static void PutTimerOnTimeoutQueue_isr(BTMR_Handle device, BTMR_TimerHandle timer)
{
    timer->adjustedValue = timer->initialValue;

    /* There are three cases to consider here:
    ** 1) Empty list -- no timer currently running.
    ** 2) New head item -- new timeout is less than currently running timer's timeout value.
    ** 3) Other -- new timeout is larger than currently running timer's timeout value.
    */
    
    if (BLST_D_EMPTY(&device->TimeoutList))
    {
        /* 1) If list is empty, then this is a new timeout value -- just start the timer */
        BLST_D_INSERT_HEAD(&device->TimeoutList, timer, link);
        JustStartTimer_isr(device->CountDownTimer, timer->adjustedValue);
    }
    else
    {
        /* If the list is NOT empty then we need to add this timer to the list in the appropriate location */
        uint32_t timeLeft;
        BTMR_TimerHandle walker = BLST_D_FIRST(&device->TimeoutList);
        
        timeLeft = JustReadTimer_isr(device->CountDownTimer);

        /* 2) Is the timeout of the new item is smaller than the remaining timeout of the currently running timer? */
        /* If so then we need to stop the timer, count off how must time has elapsed, and adjust the head item's timeout value. */
        if (timer->initialValue < timeLeft)
        {
            JustStopTimer_isr(device->CountDownTimer);

            BLST_D_INSERT_BEFORE(&device->TimeoutList, walker, timer, link);
            walker->adjustedValue = timeLeft - timer->initialValue;

            JustStartTimer_isr(device->CountDownTimer, timer->initialValue);
        }
        else
        {
            bool added = false;
            BTMR_TimerHandle last = walker;

            /* Account for the timeout of the currently running (or soon to be running) timer */
            timer->adjustedValue -= timeLeft;
            
            /* 3) We need to find the best place to put this new timeout. We adjust his timeout value as we skip over items. */
            /* Once found, we insert him and adjust the timeout of the guy that comes after him to account for the new timeout */
            for (walker = BLST_D_NEXT(walker, link); walker; last = walker, walker = BLST_D_NEXT(walker, link))
            {
                if (timer->adjustedValue < walker->adjustedValue)
                {
                    BLST_D_INSERT_BEFORE(&device->TimeoutList, walker, timer, link);
                    walker->adjustedValue -= timer->adjustedValue;
                    added = true;
                    break;
                }
                timer->adjustedValue -= walker->adjustedValue;
            }

            /* If the new item is larger timeout then all the others then it goes onto the end of the list! */
            if (!added) {
                BLST_D_INSERT_AFTER(&device->TimeoutList, last, timer, link);
            }
        }
    }
}

/* This takes an item off the sorted timeout list (on stop timer request).
** If the item was the head then it stops the timer and starts the next item on the queue.
** This assumes that the hardware timer is already running with either this timer or another timer.
*/
static void TakeTimerOffTimeoutQueue_isr(BTMR_Handle device, BTMR_TimerHandle timer)
{
    BTMR_TimerHandle head, next;
    
    /* There are two cases to consider here:
    ** 1) We're removing the head entry (and need to reset the timer).
    ** 2) Other -- we're removing a scheduled timeout that hasn't been scheduled yet.
    */

    head = BLST_D_FIRST(&device->TimeoutList);
    if (!head) {
        BDBG_ERR(("Told to delete timer from an empty timeout queue -- ignoring!"));
        /*BDBG_ASSERT(0);*/ /* I should really panic, right??? */
        return;
    }
    next = BLST_D_NEXT(timer, link);
    
    if (timer == head)
    {
        /* 1) If the item being deleted is the head item we have to account for the amount of time the timer has left to run... */
        uint32_t timeLeft = JustReadVirtualTimer_isr(timer);

        JustStopTimer_isr(device->CountDownTimer);

        BLST_D_REMOVE(&device->TimeoutList, timer, link);

        /* Adjust any next item AND start up a new timer (if there is one) */
        if (next) {
            next->adjustedValue += timeLeft;
            JustStartTimer_isr(device->CountDownTimer, next->adjustedValue);
        }
    }
    else
    {
        BLST_D_REMOVE(&device->TimeoutList, timer, link);

        /* 2) If the item being deleted is NOT the head item then we only need to adjust the next guy's timeout value */
        if (next)
            next->adjustedValue += timer->adjustedValue;
    }
}

/*
** This is my ISR function for the virtual count-down timers!
** This is registered as the callback for the physical timer used to process the virtual count-down timers.
** It is registered at at 'create' and disabled at 'destroy'.
** This performs the callbacks to the owner of the virtual timer.
** This is called back from the hardware ISR so this is run at ISR level.
*/

static void Virtual_Timer_Isr(BTMR_Handle device)
{
    BTMR_TimerHandle timer;
    BTMR_TimerSettings *context;

    /* Not getting a device means we have a logic error in our physical timer processing */
    BDBG_ASSERT(device);
    /* We should never be called here when there are no virtual timers running! */
    BDBG_ASSERT(!BLST_D_EMPTY(&device->TimeoutList));
    /* There should never be anything on the "to do" list */
    BDBG_ASSERT(BLST_SQ_EMPTY(&device->ToBeProcessedList));

    /* The timer should already be stopped, but ... */
    JustStopTimer_isr(device->CountDownTimer);

    /*BDBG_WRN(("Virtual timer interrupt"));*/

    /* This is called from the registered hardware ISR for this timer (for the CountDownTimer).
    ** It stopped the timer as part of the ISR processing.
    ** The timer only interrupts if it counted up to its adjusted value.
    */

    /* This interrupt occured because we have (at least) one count-down timer running and the next timer 
    ** to timeout (smallest timeout) just timed out.  We pull off ALL the timers that expired (could be 
    ** more than one) and queue them up for processing.  The benefit of delaying the processing is that
    ** a timer's callback function will not impact us from starting up the next virtual timer.
    */
    timer = BLST_D_FIRST(&device->TimeoutList);
    BDBG_ASSERT(timer);

    do {
        BLST_D_REMOVE_HEAD(&device->TimeoutList, link);

        BLST_SQ_INSERT_TAIL(&device->ToBeProcessedList, timer, qlink);

        /* We flag this timer as processing in case they try to start/stop the timer in the call-back function */
        timer->processing = true;
        timer->running = false;

        /* We don't need to physically stop the timer as it doesn't really exist */
        timer->lastValue = 0; /* count down timers always stop at zero */

        timer = BLST_D_FIRST(&device->TimeoutList);

    } while (timer && timer->adjustedValue <= BTMR_MIN_HOLD);

    /* If there is another timer waiting to start -- well, then start it! */
    if (timer) JustStartTimer_isr(device->CountDownTimer, timer->adjustedValue);

    /* We now have a list of timers that need to be processed (i.e. process the call-back).
    ** If its a periodic timer, put it back onto the timer list.
    ** We do this as a separate pass so that we can get the timer running again (with the next timer) as soon as possible.
    ** This way, the callback from a timer will not effect the time of the next timer.
    ** This will however affect the start time of a periodic timer -- we'll restart the timer after the call-back returns.
    */
    while ((timer = BLST_SQ_FIRST(&device->ToBeProcessedList)))
    {
        BLST_SQ_REMOVE_HEAD(&device->ToBeProcessedList, qlink);

        /* This timer expired!  Let the owner know it expired! */
        /* If we have a callback function for this timer, then its time to call it! */
        /* Note: this is processing count-down timers so there had BETTER be a call-back function!!! */
        context = &timer->Settings;
        if (context->cb_isr)
            context->cb_isr(context->pParm1, context->parm2); 

        /* We're done processing this timer so its okay to stop or delete it now */
        timer->processing = false;

        /* Periodic timers continue to go off until stopped -- start it up again! */
        /* They may have asked us to stop the timer in the callback.  We delayed this until done processing the call-back. */
        /* If they asked us to stop the timer then just don't start it back up. */
        if (context->type == BTMR_Type_ePeriodic && !timer->pleaseStop) {
            timer->startingValue = GetTimerValue_isr(device->FreeRunTimer);
            PutTimerOnTimeoutQueue_isr(device, timer);
            timer->running = true;
        }

        /* If they requested a count-down timer to be restarted in the call-back then do it now. */
        if (context->type == BTMR_Type_eCountDown && timer->pleaseStart) {
            BTMR_StartTimer_isr(timer, timer->delayedValue);
        }

        /* Make sure these are cleared before leaving (in case they tried to start a periodic or stop a countdown) */
        timer->pleaseStop  = false;
        timer->pleaseStart = false;
    }
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* Create the (two) timers we need to handle virtual timers */
/* This is used in the open call when requested to allocate these timers statically or in create call for dynamic when these are actually needed. */
static BERR_Code CreateUtilityTimers(BTMR_Handle device)
{
    /* If we've already created the timers needed for supporting virtual timers then nothing to do */
    if (device->CountDownTimer) return BERR_SUCCESS;

    BDBG_MSG(("Creating utility timers ..."));

    /* We need one physical timer in order to use virtual count-down timers.
    ** This timer will be controlled by our call back function (runs at ISR time).
    */
    {
        BTMR_TimerSettings settings = { BTMR_Type_eCountDown, (BTMR_CallbackFunc)Virtual_Timer_Isr, NULL, 0, true };
        settings.pParm1 = device; /* needs to be set at runtime */
        if (_BTMR_CreateTimer(device, &device->CountDownTimer, &settings, "CountDown", 0) != BERR_SUCCESS) {
            BDBG_ERR(("Failed to create physical timer needed for supporting virtual count-down timers!"));
            return BERR_TRACE(BTMR_ERR_NO_TIMERS_AVAILABLE);
        }
        BLST_D_INIT(&device->TimeoutList);
        BLST_SQ_INIT(&device->ToBeProcessedList);
        BDBG_MSG(("Created physical timer needed for supporting virtual CD timers (using timer %d)", device->CountDownTimer->timerNumber));
    }
    
    /* We'll use THE one shared timer in order to use virtual stop-watch (free-run) timers.
    ** This just runs the timer and uses the timer value to generate a current time value.
    ** Note: if a shared timer was allocated previously then this will use that shared timer.
    ** If no shared timer was previously created then this call will create THE shared timer.
    */
    {
        BTMR_TimerSettings settings = { BTMR_Type_eSharedFreeRun, NULL, NULL, 0, true };
        settings.pParm1 = device; /* needs to be set at runtime */
        if (_BTMR_CreateTimer(device, &device->FreeRunTimer, &settings, "FreeRun", 0) != BERR_SUCCESS) {
            _BTMR_DestroyTimer(device->CountDownTimer, "CountDown", 0);
            device->CountDownTimer = NULL;
            BDBG_ERR(("Failed to create shared timer needed for supporting virtual free-run timers!"));
            return BERR_TRACE(BTMR_ERR_NO_TIMERS_AVAILABLE);
        }
        BDBG_MSG(("Created (shared) timer needed for supporting virtual FR timers (using timer %d)", device->FreeRunTimer->timerNumber));
    }

    return BERR_SUCCESS;
}

/* Destroy the timers we created for handling the virtual timers */
static void DestroyUtilityTimers(BTMR_Handle device)
{
    /* If the timers were never created than nothing to destroy */
    if (!device->CountDownTimer) return;

    BDBG_MSG(("Destroying utility timers ..."));

    if (device->CountDownTimer) {
        _BTMR_DestroyTimer(device->CountDownTimer, "CountDown", 0);
        device->CountDownTimer = NULL;
    }
    if (device->FreeRunTimer) {
        _BTMR_DestroyTimer(device->FreeRunTimer, "FreeRun", 0);
        device->FreeRunTimer = NULL;
    }
    BDBG_MSG(("Destroyed timers needed for supporting virtual timers!"));
}

/* This is used by open to clean up on error and close to disable level two interrupts */
static void DestroyCallbacks(BTMR_Handle device)
{
    int timerNumber;

    /* The first thing we want to do is stop any running timers and make sure we won't be getting anymore interrupts */
    for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++)
    {
        /* Skip timers we don't control (excluded in the open) */
        if (device->Settings.timerMask & TimerSelect(timerNumber)) continue;

        /* If we created the callback in the open, make sure we don't have any pending interrupts */
        if (device->Settings.interruptNumber)
        {
            if (device->alt_CbHandle) {
                /*BINT_ClearCallback(device->alt_CbHandle);*/ /* not necessary -- destroy handles this */
                BINT_DisableCallback(device->alt_CbHandle);
                BINT_DestroyCallback(device->alt_CbHandle);
                device->alt_CbHandle = NULL;
                device->alt_CbEnabled = 0;
            }
        }
        else
        {
            if (device->CbHandle[timerNumber]) {
                /*BINT_ClearCallback(device->CbHandle[timerNumber]);*/ /* not necessary -- destroy handles this */
                BINT_DisableCallback(device->CbHandle[timerNumber]);
                BINT_DestroyCallback(device->CbHandle[timerNumber]);
                device->CbHandle[timerNumber] = NULL;
            }
        }

        /* If the callback is enabled for this timer then it isn't anymore */
        if (device->Timers[timerNumber])
            device->Timers[timerNumber]->cbEnabled = false;
    }
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

static BERR_Code virt_CreateTimer_isr(BTMR_Handle device, BTMR_TimerHandle timer, const BTMR_TimerSettings *pSettings)
{
    BERR_Code retCode = BERR_SUCCESS;

    BSTD_UNUSED(pSettings); /* the phys version needs the settings, the virt version does not */
 
    /* Virtual timers don't have an actual timer number -- only physical timers do */
    timer->timerNumber = -1;

    BDBG_MSG(("BTMR_CreateTimer: Successfully created new virtual timer!"));
    BLST_D_INSERT_HEAD(&device->VirtualCreateList, timer, created);
    device->VirtualCount++;

    return (retCode);
}
static BERR_Code virt_CreateTimer(BTMR_Handle device, BTMR_TimerHandle timer, const BTMR_TimerSettings *pSettings)
{
    BERR_Code status;

    /* We need some utility timers for handling virtual timers, so create them on first use */
    /* Note: This will succeed if they already exist (i.e. allocated statically in Open) */
    if (device->VirtualCount == 0) {
        if (CreateUtilityTimers(device) != BERR_SUCCESS) {
            return BERR_TRACE(BTMR_ERR_NO_TIMERS_AVAILABLE);
        }
    }

    BKNI_EnterCriticalSection();
    status = virt_CreateTimer_isr(device, timer, pSettings);
    BKNI_LeaveCriticalSection();
    return status;
}

static BERR_Code virt_DestroyTimer_isr(BTMR_TimerHandle timer)
{
    BTMR_Handle device = timer->device;

    /* It would be bad to free up a timer that is running -- it could be linked onto a list, etc.
    ** We have to make sure its stopped before we delete it!
    ** Don't use virt_StopTimer_isr as the processing tests gets done in the higher level functions.
    */
    if (timer->running) BTMR_StopTimer_isr(timer);
    
    BDBG_MSG(("BTMR_DestroyTimer: Successfully destroyed virtual timer!"));
    BLST_D_REMOVE(&device->VirtualCreateList, timer, created);
    device->VirtualCount--;

    return BERR_SUCCESS;
}
#if 0
static BERR_Code virt_DestroyTimer(BTMR_TimerHandle timer)
{
    BERR_Code status;
    BTMR_Handle device = timer->device;

#if 0
    /* We created some utility timers needed for processing virtual timers. 
    ** If this is the last virtual timer then we don't need these anymore. 
    */
    if (device->VirtualCount == 1)
        DestroyUtilityTimers(device);
#endif

    BKNI_EnterCriticalSection();
    status = virt_DestroyTimer_isr(timer);
    BKNI_LeaveCriticalSection();
    return status;
}
#endif

static BERR_Code virt_StartTimer_isr(BTMR_TimerHandle timer, unsigned startingValue)
{
    BTMR_Handle device = timer->device;

    /* Should I let them start a running timer??? */
    /* The problem here is that if they forgot to stop the timer and believe that this is actually
    ** starting the timer then they aren't going to get what they expected and won't know why.
    ** We have three choices here: 1) don't let them start a started timer, 2) stop the timer and restart
    ** with the new parameters, or 3) just say okay and keep going.
    */
    if (timer->running) 
    {
#if 0
        BDBG_ERR(("BTMR_StartTimer: timer already started!"));
        return BERR_TRACE(BTMR_ERR_ALREADY_STARTED);
#else
        goto done; /* its already started -- success! (this is choice 3) */
#endif
    }
    
    /* They talk microseconds, we talk timer ticks */
    timer->initialValue = MicroSecondsToTimer(startingValue);

    /* We'll use the free running timer to provide time values when user tries to read the current time */
    timer->startingValue = GetTimerValue_isr(device->FreeRunTimer);
    timer->lastValue = 0;
    
    /* If its a free run timer then its already running -- we use the physical free run timer for the time calculations.
    ** If its a count-down type timer then we need to use the physical count-down timer to do the timing.
    ** That timer is shared by other count-down type timers so we queue it up for processing.
    */
    if (timer->Settings.type == BTMR_Type_eCountDown || timer->Settings.type == BTMR_Type_ePeriodic)
        PutTimerOnTimeoutQueue_isr(device, timer);
 
    timer->running = true;

  done:
    return BERR_SUCCESS;
}

static BERR_Code virt_StopTimer_isr(BTMR_TimerHandle timer)
{
    BTMR_Handle device = timer->device;

    /* Should I let them stop a stopped timer??? */
    /* We have two choices here: 1) don't let them stop a stopped timer or 2) just say okay and keep going.  */
    if (!timer->running)
    {
#if 0
        BDBG_ERR(("BTMR_StopTimer: timer already stopped!"));
        return BERR_TRACE(BTMR_ERR_ALREADY_STOPPED);
#else
        goto done; /* its already stopped -- success! (this is choice 2) */
#endif
    }
 
    timer->lastValue = JustReadVirtualTimer_isr(timer);
    
    /* If its a free-run timer then we save a "last" (stopped) value above.
    ** If its a count-down type timer then its either running, queued to be run, or stopped.
    ** Regardless, we just dequeue the item so that it doesn't continue to run.
    */
    if (timer->Settings.type == BTMR_Type_eCountDown || timer->Settings.type == BTMR_Type_ePeriodic)
        TakeTimerOffTimeoutQueue_isr(device, timer);

    timer->running = false;

  done:
    return BERR_SUCCESS;
}

static unsigned virt_ReadTimer_isr(BTMR_TimerHandle timer)
{
    uint32_t runTime;

    /* Stopped timers return the value of the timer when it was stopped! */
    if (!timer->running) {
        runTime = timer->lastValue;
        BDBG_MSG(("BTMR_ReadTimer: (virt) read time from stopped timer (value=%x)", runTime));
    } else {
        runTime = JustReadVirtualTimer_isr(timer);
    }

    /* They talk microseconds, we talk timer ticks */
    return TimerToMicroSeconds(runTime);
}

/* These are the functions used by virtual timers */
static const ProcessingFunctions virtualFunctions = { virt_CreateTimer, virt_DestroyTimer_isr, virt_StartTimer_isr, virt_StopTimer_isr, virt_ReadTimer_isr };

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/* 
** These are the Stand-by/Resume processing functions for the TMR module.
** When we return from Standby Mode (deep sleep = S3) all of the timer registers have been reset (to defaults).
** If we had running timers then they are no longer running AND we can't resume them from the last known timer setting -- they always start from zero.
** Anyone that had an outstanding timer is going to get a different value when they read the timer after a resume.  We can't fix this!
** All we can do is find out if there are outstanding timers and say so.  We can destroy the timers (invalidating the handles) so that if they attempt
** to use them again we'll assert.  But we really just want to identify who had an outstanding timer and make sure that it gets destroyed before switch.
*/

/* In this round, do not do anything destructive -- just report so it can be fixed before next full release cycle */

/*#define destructive_standby*/ /* enable the define to perform destructive work when entering standby */
#define kill_utility_timers /* enable this to kill the timers I use for virtual timers (will be recreated on resume) */

/* Check to see if any timers are running and if so, ... */
static BERR_Code SetupForStandby(BTMR_Handle device)
{
    int timerNumber, timerCount;
    BTMR_TimerHandle timer;
    BERR_Code retCode = BERR_SUCCESS;

#if 0
    /* Don't leave any of the timers running.
    ** Note: if someone is still using this timer, even though it should be destroyed, this could cause a problem if he enters a wait loop.
    */
    BKNI_EnterCriticalSection();
    for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++)
    {
        /* Skip timers we don't control and stop the ones we do */
        if (device->Settings.timerMask & TimerSelect(timerNumber)) continue;
        if (device->Timers[timerNumber]) JustStopTimer_isr(device->Timers[timerNumber]);
    }
    BKNI_LeaveCriticalSection();
#endif

    /* See if we have any timers running waiting for timeouts */
    if (!BLST_D_EMPTY(&device->TimeoutList)) {
        timerCount = 0;
        BKNI_EnterCriticalSection();
        timer = BLST_D_FIRST(&device->TimeoutList);
        while (timer) {
            BTMR_TimerHandle next = BLST_D_NEXT(timer, link);
#ifdef destructive_standby
			TakeTimerOffTimeoutQueue_isr(device, timer);
#endif
            timer = next;
            timerCount++;
        }
        BDBG_ERR(("BTMR_Standby: called with %d running virtual timers!", timerCount));
        BKNI_LeaveCriticalSection();
    }

    /* Note: we shoule NEVER have to worry about anything on 'ToBeProcessedList' */

    /* If we have outstanding virtual timers then we allocated context for each one.
    ** The owner was supposed to destroy the timer before the stand-by was called, but ...
    ** We should probably make this into a fail at some point (but not yet)!
    */
    if (device->VirtualCount) 
    {
        BDBG_ERR(("BTMR_Standby: called with %d outstanding virtual timers!", device->VirtualCount));
        timer = BLST_D_FIRST(&device->VirtualCreateList);
        while (timer)
        {
            BTMR_TimerHandle next = BLST_D_NEXT(timer, created);
            BDBG_ERR(("BTMR_Standby: virtual timer created but not destroyed (created: %s,%d)", timer->file, timer->line));
#ifdef destructive_standby
            BLST_D_REMOVE(&device->VirtualCreateList, timer, created);
#ifdef _DESTROY_ON_STANDBY_
            BTMR_DestroyTimer(timer);
#else
            BDBG_OBJECT_DESTROY(timer, btmr_timer_t); /* if not destroying, make sure they can't use it after the resume */
#endif
#endif
            timer = next;
        }
#ifdef _FAIL_ON_STANDBY_
        BDBG_ASSERT(0);
#endif
    }

#ifdef kill_utility_timers
    /* We may have allocated a shared or virtual timer.  Free them up now. They will be re-created when needed. */
    if (device->CountDownTimer) {
        DestroyUtilityTimers(device);
        device->destroyed_utilities = true;
    }
#endif

    /* Unlike virtual timers (where we created context) all the shared timers use the same context so we can't keep track of who created them.
    ** So its not important that we clean these up other than to make sure the timer is stopped.
    */
    if (device->SharedCount) {
        BDBG_ERR(("BTMR_Standby: called with %d outstanding shared timers!", device->SharedCount));
        BKNI_EnterCriticalSection();
        JustStopTimer_isr(device->SharedTimer);
        BKNI_LeaveCriticalSection();
#ifdef destructive_standby
#ifdef _DESTROY_ON_STANDBY_
        device->SharedCount = 1; /* hack to make sure we really destroy the timer below (gets done on 1->0 transition) */
#endif
#endif
#ifdef _FAIL_ON_STANDBY_
        BDBG_ASSERT(0);
#endif
    }

    /* Now check on physical timers -- does anyone have one of these still pending? */
    timerCount = 0;
    for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++)
    {
        /* Skip timers we don't control (excluded in the open) */
        if (device->Settings.timerMask & TimerSelect(timerNumber)) continue;

        /* Destroy any existing timers (frees resources used in create) */
        /* Note: this doesn't prevent someone that didn't destroy their timer from attempting to continue to use it... */
        if ((timer = device->Timers[timerNumber]))
        {
            /* Special case the shared timer -- it won't correctly reflect who created but didn't destroy */
            if (timer->Settings.type == BTMR_Type_eSharedFreeRun) {
                BDBG_ERR(("BTMR_Standby: physical timer %d created but not destroyed (shared)", timerNumber));
            } else {
                BDBG_ERR(("BTMR_Standby: physical timer %d created but not destroyed (created: %s,%d)", timerNumber, timer->file, timer->line));
            }
#ifdef destructive_standby
#ifdef _DESTROY_ON_STANDBY_
            BTMR_DestroyTimer(timer);
#else
            BDBG_OBJECT_DESTROY(timer, btmr_timer_t); /* if not destroying, make sure they can't use it after the resume */
#endif
#endif
            timerCount++;
        }
    }
    if (timerCount) {
        BDBG_ERR(("BTMR_Standby: called with %d outstanding physical timers pending!", timerCount));
#ifdef _FAIL_ON_STANDBY_
        BDBG_ASSERT(0);
#endif
    }

#ifdef destructive_standby
    /* Final cleanup ... */
#ifndef _DESTROY_ON_STANDBY_
    BLST_D_INIT(&device->VirtualCreateList);
#endif
    device->VirtualCount = 0;
    device->SharedCount = 0;
#endif

    return retCode;
}

/* Do the necessary processing for a resume (from stand-by) operation */
static BERR_Code SetupForResume(BTMR_Handle device)
{
    BERR_Code retCode = BERR_SUCCESS;

    /* If there were pending shared timers (they all use same one) then it should be running */
    /* It was reset as part of the warmstart, so start it back up now */
    if (device->SharedCount) {
        BKNI_EnterCriticalSection();
        JustStartTimer_isr(device->SharedTimer, 0);
        BKNI_LeaveCriticalSection();
    }

#ifdef kill_utility_timers
#if 1
    /* If we destroyed the utility timers on the stand-by, we can re-create them now */
	/* Note: this isn't really necessary if all the other timers were destroyed before the stand-by as they will be created when needed. */
    if (device->destroyed_utilities && CreateUtilityTimers(device) != BERR_SUCCESS)
#else
    /* If using static utility timers, create the physical timers we need to handle virtual timers */
    /* Otherwise, we'll create the timers needed when someone requests a virtual timer */
    if (device->Settings.preallocUtilTimers && CreateUtilityTimers(device) != BERR_SUCCESS)
#endif
    {
        BDBG_ERR(("BTMR_Resume: failed to re-create utility timers needed to support virtual timers!"));
        retCode = BERR_TRACE(BTMR_ERR_NO_TIMERS_AVAILABLE);
    }
    device->destroyed_utilities = false;
#endif

#ifdef destructive_standby
    BLST_D_INIT(&device->VirtualCreateList);
#endif

    return retCode;
}

/* Make sure we don't have any timer interrupts pending */
static void DisableCallbacks(BTMR_Handle device)
{
    int timerNumber;

    for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++) 
    {
        /* Skip timers we don't control (excluded in the open) */
        if (device->Settings.timerMask & TimerSelect(timerNumber)) continue;

        /* If we created the callback in the open, make sure we don't have any pending interrupts */
        if (device->Settings.interruptNumber)
        {
            if (device->alt_CbHandle && device->alt_CbEnabled) {  
                BDBG_WRN(("Disable Alt CB"));
                BINT_DisableCallback(device->alt_CbHandle);
            }
        }
        else
        {
            if (device->CbHandle[timerNumber] && device->Timers[timerNumber] && device->Timers[timerNumber]->cbEnabled) {                
                BDBG_WRN(("Disable CB"));
                BINT_DisableCallback(device->CbHandle[timerNumber]);
            }
        }        
    }
}

/* If we had interrupts enabled before entering standby then restore on the resume */
static void EnableCallbacks(BTMR_Handle device)
{
    int timerNumber;

    for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++) 
    {
        /* Skip timers we don't control (excluded in the open) */
        if (device->Settings.timerMask & TimerSelect(timerNumber)) continue;

        /* If we created the callback in the open, make sure we don't have any pending interrupts */
        if (device->Settings.interruptNumber)
        {
            if (device->alt_CbHandle && device->alt_CbEnabled) {                
                BDBG_WRN(("Enable Alt CB"));
                BINT_EnableCallback(device->alt_CbHandle);
            }
        }
        else
        {
            if (device->CbHandle[timerNumber] && device->Timers[timerNumber] && device->Timers[timerNumber]->cbEnabled) {                
                BDBG_WRN(("Enable CB"));
                BINT_EnableCallback(device->CbHandle[timerNumber]);
            }
        }        
    }
}

/*\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\*/

/*
***********************************************************************************
***********************************************************************************
** These are the set of published timer functions (available for external use).
***********************************************************************************
***********************************************************************************
*/

BERR_Code BTMR_Open(BTMR_Handle *phDevice,
                    BCHP_Handle hChip, BREG_Handle hRegister, BINT_Handle hInterrupt,
                    const BTMR_DeviceSettings *pOpenSettings)
{
    BTMR_Handle device;
    BERR_Code retCode = BERR_SUCCESS;
    int timerNumber, timersInitialized = 0;

    /* Sanity check on the handles we've been given */
    BDBG_ENTER(BTMR_Open);
    BDBG_ASSERT(phDevice);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hInterrupt);
    /* Note: They don't have to give me the settings, I'll just use the default */
    
    /* Some say I shouldn't mess with the return variable unless I return success... */
    *phDevice = NULL; /* just to be sure caller doesn't use it */

    device = (BTMR_Handle) BKNI_Malloc(sizeof(*device));
    if (device == NULL)
    {
        BDBG_ERR(("BTMR_Open: Memory allocation failed!"));
        retCode =  BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }

    BKNI_Memset(device, 0, sizeof(*device));
    BDBG_OBJECT_SET(device, btmr_device_t);

    retCode = BKNI_CreateMutex(&device->create_destroy_mutex);
    if (retCode != BERR_SUCCESS) 
    {
        BDBG_ERR(("BTMR_Open: Mutext (create_destroy) allocation failed!"));
        retCode = BERR_TRACE(retCode);
        goto error2;
    }

    device->hChip      = hChip;
    device->hRegister  = hRegister;
    device->hInterrupt = hInterrupt;

    if (pOpenSettings)
        BKNI_Memcpy(&device->Settings, pOpenSettings, sizeof(BTMR_DeviceSettings));
    else
        BTMR_GetDefaultDeviceSettings(&device->Settings);

    /* Default this to the default timer set (in case they didn't use get defaults) */
    if (!device->Settings.baseRegister) {
        device->Settings.baseRegister = BCHP_TIMER_REG_START;
        device->Settings.interruptNumber = 0;
    }

    BDBG_MSG(("BTMR_Open: device=%p using MASK %#x (base=%p, pOpenSettings=%p)",
	      (void*)device, device->Settings.timerMask, device->Settings.baseRegister, pOpenSettings));
#if (BCHP_CHIP != 11360)
    BDBG_CASSERT(NumberOfTimers==4); /* note: you have to fix the table below when changing the number of timers */

#ifdef BCHP_PWR_RESOURCE_TMR
    retCode = BCHP_PWR_AcquireResource(device->hChip, BCHP_PWR_RESOURCE_TMR);
#endif

    /* If this timer block uses a level three interrupt then we have ONE callback for all the timers */
    /* Level two interrupts have their own individual interrupts */
    if (device->Settings.interruptNumber)
    {
        BDBG_MSG(("Interrupt number specified (using level 3)"));

        for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++) 
        {
            /* We only setup a callback for the timers we're supposed to control (i.e. ones NOT reserved) */
            if (device->Settings.timerMask & TimerSelect(timerNumber)) {
                BDBG_WRN(("Timer %u is reserved (skipping)", timerNumber));
                continue;
            }

            /* Make sure this timer is not running from a previous run (if they ^C out before) */
            DisableTimerNumber(device, timerNumber);

            timersInitialized++;  /* got at least one! */
        }

        if (timersInitialized) 
        {
            BERR_Code result;

            /* This is where we setup the interrupt callback for the timers. */
            /* Note: we have to fail if we couldn't enable the level two interrupt for all these timers */
            result = BINT_CreateCallback(&device->alt_CbHandle, hInterrupt, device->Settings.interruptNumber,
                                      (BINT_CallbackFunc)Physical_Timer_Selection_Isr, (void *)device, 0);
            if (result != BERR_SUCCESS) {
                BDBG_ERR(("BTMR_Open: Create Callback for interrupt failed!"));
                retCode = BERR_TRACE(result);
                goto error2;
            }

            BINT_ClearCallback(device->alt_CbHandle);
        }
    }
    else
    {
        BDBG_MSG(("Interrupt number not specified (using level 1)"));

        /* Create a callback for each of the timers we're controlling (but enable later) */
        for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++)
        {
            BERR_Code result;
            BINT_Id intId=0;

            /* We only setup a callback for the timers we're supposed to control (i.e. ones NOT reserved) */
            if (device->Settings.timerMask & TimerSelect(timerNumber)) {
                BDBG_WRN(("Timer %u is reserved (skipping)", timerNumber));
                continue;
            }
        
            /* Make sure this timer is not running from a previous run (if they ^C out before) */
            DisableTimerNumber(device, timerNumber);

            /*TODO: think of a way to set the intId value without having to resort to using the separate masks? */

            switch (timerNumber) {
                case 0: intId = BCHP_INT_ID_TMR0TO; break;
                case 1: intId = BCHP_INT_ID_TMR1TO; break;
                case 2: intId = BCHP_INT_ID_TMR2TO; break;
                case 3: intId = BCHP_INT_ID_TMR3TO; break;
            }

            /* This is where we setup the interrupt callback for the timers WE control. */
            /* Note: we don't have to fail just because we can't setup this call back -- just make it un-available! */
            result = BINT_CreateCallback(&device->CbHandle[timerNumber], hInterrupt, intId,
                                      (BINT_CallbackFunc)Physical_Timer_Isr, (void *)device, timerNumber);
            if (result != BERR_SUCCESS) {
                BDBG_ERR(("BTMR_Open: Create Callback for interrupt failed (timer=%d)", timerNumber));
                device->Settings.timerMask |= TimerSelect(timerNumber);
                device->CbHandle[timerNumber] = NULL;
                continue;
            }

            /* Make sure this interrupt is clear long before we enable it in the create funtion */
            /* Note: the comments for the create says is clears this, but an extra call shouldn't hurt */
            BINT_ClearCallback(device->CbHandle[timerNumber]);
            timersInitialized++;  /* got at least one! */
        }    
    }

    /* Its possible that all of the timers are being used outside of this driver or that we couldn't create the callback for
    ** any of the timers that we will control.  This means we can't create the timers we need to support virtual timers!
    ** This can either be considered an error or return success and then fail whenever someone wants to create a timer.
    ** I think this should be an error!
    */
    if (!timersInitialized) {
        BDBG_ERR(("BTMR_Open: failed to initialize any physical timers (all physical timers are being used externally)"));
        retCode = BERR_TRACE(BTMR_ERR_NO_TIMERS_AVAILABLE);
        goto error2;
    }
    
    /* Assume that any timer reserved is already "in use" */
    device->inUse = device->Settings.timerMask;

    /* If using static utility timers, create the physical timers we need to handle virtual timers */
    /* Otherwise, we'll create the timers needed when someone requests a virtual timer */
    if (device->Settings.preallocUtilTimers && CreateUtilityTimers(device) != BERR_SUCCESS) {
        BDBG_ERR(("BTMR_Open: failed to create utility timers needed to support virtual timers!"));
        retCode = BERR_TRACE(BTMR_ERR_NO_TIMERS_AVAILABLE);
        goto error1;
    }

#endif
    BLST_D_INIT(&device->VirtualCreateList);

    *phDevice = device; /* here's your handle for creating timers! */
    goto done;

  error1:
    DestroyCallbacks(device);

  error2:
    if (device->create_destroy_mutex)  BKNI_DestroyMutex(device->create_destroy_mutex);
    BDBG_OBJECT_DESTROY(device, btmr_device_t);
    BKNI_Free(device);

  done:
    BDBG_LEAVE(BTMR_Open);
    return (retCode);
}

BERR_Code BTMR_Close(BTMR_Handle device)
{
    int timerNumber, timersLeftOpen;
    BTMR_TimerHandle timer;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTMR_Close);
    BDBG_MSG(("BTMR_Close: device=%p", (void*)device));
    BDBG_OBJECT_ASSERT(device, btmr_device_t);

    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE);

    /* When our close is called, modules with outstanding timers should have already destroyed them.
    ** But that doesn't mean they did.  There are a couple of things we can do about this:
    ** 1) indicate that there were virtual timers left open and clean them up.
    ** 2) indicate there are outstanding shared timers (no clean-up necessary).
    ** 3) indicate that there are pending exclusive timers left open and clean them up.
    ** The clean-up is only so that we don't get yelled at for closing with allocated memory pending.
    ** We allocated it for someone else, but it'll still be our name on the error message and problem report.
    ** Use the compile flags to not do the cleanup (_DESTROY_ON_CLOSE_) or not be so nice about it (_FAIL_ON_CLOSE_).
    */

    /* Disable any interrupts and callbacks associated with the registered interrupt */
    DestroyCallbacks(device);

    /* From this point on we won't be getting any interrupts from any timers we control! */

    /* Don't leave any of the timers running (not that it'll hurt anything).
    ** Note: if someone is still using this timer, even though it should be closed, this could cause a problem if he enters a wait loop.
    */
    BKNI_EnterCriticalSection();
    for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++)
    {
        /* Skip timers we don't control and stop the ones we do */
        if (device->Settings.timerMask & TimerSelect(timerNumber)) continue;
        if (device->Timers[timerNumber]) JustStopTimer_isr(device->Timers[timerNumber]);
    }
    BKNI_LeaveCriticalSection();

    /* If we have outstanding virtual timers then we allocated context for each one.
    ** The owner was supposed to destroy the timer before the close was called, but ...
    ** We should probably make this into a fail at some point (but not yet)!
    */
    if (device->VirtualCount) 
    {
        BDBG_ERR(("BTMR_Close: called with %d outstanding virtual timers (resource leak)", device->VirtualCount));
        timer = BLST_D_FIRST(&device->VirtualCreateList);
        while (timer)
        {
            BTMR_TimerHandle next = BLST_D_NEXT(timer, created);
            BDBG_ERR(("BTMR_Close: virtual timer created but never destroyed (created: %s,%d)", timer->file, timer->line));
#ifdef _DESTROY_ON_CLOSE_
            BTMR_DestroyTimer(timer);
#endif
            timer = next;
        }
#ifdef _FAIL_ON_CLOSE_
        BDBG_ASSERT(0);
#endif
    }

    /* We may have statically allocated a shared or virtual timer as part of the open.  Free them up now. */
    DestroyUtilityTimers(device);

    /* This is just informative.
    ** Unlike virtual timers (where we created context) all the shared timers use the same context so we can't keep track of who created them.
    ** So its not important that we clean these up other than to make sure the timer is stopped.
    */
    if (device->SharedCount) {
        BDBG_ERR(("BTMR_Close: called with %d outstanding shared timers (resource leak)", device->SharedCount));
        device->SharedCount = 1; /* hack to make sure we really destroy the timer below (gets done on 1->0 transition) */
#ifdef _FAIL_ON_CLOSE_
        BDBG_ASSERT(0);
#endif
    }

    /* 
    ** Note: At this point, we've cleaned up after ourselves.  We've also done some clean up after others (virtual and shared timers).
    ** The only thing left to do is clean up any pending exclusive timers that did not get freed up before calling close.
    ** This is not something we're required to do, but this will prevent someone from thinking the TMR code exited with allocated memory.
    */

    timersLeftOpen = 0;
    for (timerNumber=0; timerNumber<NumberOfTimers; timerNumber++)
    {
        /* Skip timers we don't control (excluded in the open) */
        if (device->Settings.timerMask & TimerSelect(timerNumber)) continue;

        /* Destroy any existing timers (frees resources used in create) */
        /* Note: this doesn't prevent someone that didn't destroy their timer from attempting to continue to use it... */
        if ((timer = device->Timers[timerNumber]))
        {
            /* Special case the shared timer -- it won't correctly reflect who created but didn't destroy */
            if (timer->Settings.type == BTMR_Type_eSharedFreeRun) {
                BDBG_ERR(("BTMR_Close: physical timer %d created but never destroyed (shared)", timerNumber));
            } else {
                BDBG_ERR(("BTMR_Close: physical timer %d created but never destroyed (created: %s,%d)", timerNumber, timer->file, timer->line));
            }
#ifdef _DESTROY_ON_CLOSE_
            BTMR_DestroyTimer(timer);
#endif
            timersLeftOpen++;
        }
    }
    if (timersLeftOpen) {
        BDBG_ERR(("BTMR_Close: called with %d outstanding physical timers pending (resource leak)", timersLeftOpen));
#ifdef _FAIL_ON_CLOSE_
        BDBG_ASSERT(0);
#endif
    }
 
    BKNI_DestroyMutex(device->create_destroy_mutex);

#ifdef BCHP_PWR_RESOURCE_TMR
    retCode = BCHP_PWR_ReleaseResource(device->hChip, BCHP_PWR_RESOURCE_TMR);
#endif

    /* Mark it invalid in case he tries to give it to me again (close twice) */
    /* Note: this probably won't happen as the memory will be reassigned and stepped on, but ... */
    BDBG_OBJECT_DESTROY(device, btmr_device_t);
    BKNI_Free(device);

    BDBG_LEAVE(BTMR_Close);
    return retCode;
}

BERR_Code BTMR_Standby(BTMR_Handle device)
{
    BERR_Code retCode = BERR_SUCCESS; 
    BDBG_ENTER(BTMR_P_Standby);
    BDBG_OBJECT_ASSERT(device, btmr_device_t);

    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE); /* already in stand-by? */

    BDBG_WRN(("Entering TMR standby-mode"));
    device->in_standby_mode = true;     /* stop register accesses BEFORE we go to low power mode */

    DisableCallbacks(device);   /* disable interrupt callbacks */
    SetupForStandby(device);    /* do what's needed to setup for a stand-by operations */

#ifdef BCHP_PWR_RESOURCE_TMR
    retCode = BCHP_PWR_ReleaseResource(device->hChip, BCHP_PWR_RESOURCE_TMR);
#endif

    BDBG_WRN(("TMR standby complete!"));

    BDBG_LEAVE(BTMR_P_Standby);
    return retCode;
}

BERR_Code BTMR_Resume(BTMR_Handle device)
{
    BERR_Code retCode = BERR_SUCCESS; 
    BDBG_ENTER(BTMR_P_Resume);
    BDBG_OBJECT_ASSERT(device, btmr_device_t);

    if (!device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE); /* NOT in stand-by? */

#ifdef BCHP_PWR_RESOURCE_TMR
    retCode = BCHP_PWR_AcquireResource(device->hChip, BCHP_PWR_RESOURCE_TMR);
#endif

    BDBG_WRN(("Exiting TMR standby-mode"));

    EnableCallbacks(device); /* re-enable interrupt callbacks */
    SetupForResume(device);  /* do what's needed to recreate the environment as it was before the stand-by */

    device->in_standby_mode = false;  /* re-enable register accesses AFTER we return from low power mode */
    BDBG_WRN(("TMR resume complete!"));

    BDBG_LEAVE(BTMR_P_Resume);
    return retCode;
}

static const BTMR_DeviceSettings DefaultDeviceSettings = { 
    0,                    /* don't reserve any timers for private use */
    false,                /* dynamically allocate timer(s) needed for virtual timers */
    BCHP_TIMER_REG_START, /* default to primary TMR block */
    0                     /* use default interrupt numbers */
};

BERR_Code BTMR_GetDefaultDeviceSettings(BTMR_DeviceSettings *pSettings)
{
    BDBG_ASSERT(pSettings); /* don't write results to zero */

    BKNI_Memcpy(pSettings, &DefaultDeviceSettings, sizeof(BTMR_DeviceSettings));

#if 0 && defined(BCHP_WKTMR_REG_START)
    /* Don't use timer 3 on chips that are lacking WKTMR support, the kernel may be using it */
    /* This should be handled outside of this PI (in the open call) but might be easier to handle it here. */
    pSettings->timerMask |= 1<<3;    
    BDBG_ERR(("Setting mask to %p", pSettings->timerMask));
#endif

    return BERR_SUCCESS;
}

/* We need to recursively create timers when dynamically creating the utility timers needed for processing virtual timers.
** Because the CreateTimer function is re-entrant, this could allow us to try to crete the utility timers from two different tasks.
** A semaphore was added to mutually exclude the timer creates from occurring at the same time.  This function allows us to
** recursively enter the CreateTimer and by-pass the locking semaphore.
*/
static BERR_Code _BTMR_CreateTimer(BTMR_Handle device, BTMR_TimerHandle *phTimer, const BTMR_TimerSettings *pSettings, const char *file, int line)
{
    BERR_Code errCode = BERR_SUCCESS;
    BTMR_TimerHandle timer;

    /* If they want to use the Shared Free Run timer (and it already exists) -- give it to them! */
    if (pSettings->type == BTMR_Type_eSharedFreeRun && device->SharedTimer)
    {
        device->SharedCount++;          /* one more user sharing this timer! */
        *phTimer = device->SharedTimer; /* here's your handle for using this timer! */
        BDBG_MSG(("BTMR_CreateTimer: NOT creating THE timer for shared timer [already created, count=%d] (at: %s,%d)", device->SharedCount, file, line));
        return BERR_SUCCESS;
    }
    
    /* If not shared (or shared doesn't yet exist) then this is a new timer and we need a handle for further use */
    timer = (BTMR_TimerHandle) BKNI_Malloc(sizeof(*timer));
    if (timer == NULL)
    {
        BDBG_ERR(("BTMR_CreateTimer: Memory allocation failed!"));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(timer, 0, sizeof(*timer));
    BDBG_OBJECT_SET(timer, btmr_timer_t);

    /* 
    ** We'd like to enforce the use of virtual timers here by over-riding the exclusive setting to false.
    ** But that might break someone's code that really does want to manipulate the timer outside of this PI.
    ** That is something you can't do with Virtual Timers (could I fake this???).
    ** So don't do this!
    */

    /* Use the appropriate processing functions for this timer based on the requested type */
    if (pSettings->exclusive || pSettings->type == BTMR_Type_eSharedFreeRun)
        timer->functions = (ProcessingFunctions*)&physicalFunctions;
    else
        timer->functions = (ProcessingFunctions*)&virtualFunctions;

    timer->device = device;
    BKNI_Memcpy(&timer->Settings, pSettings, sizeof(BTMR_TimerSettings));

    timer->file = file; /* remember where this timer was created */
    timer->line = line; /* so we can tell the user on exit where an un-destroyed timer was created */

    BDBG_MSG(("BTMR_CreateTimer: creating %s(%d) timer (at: %s,%d)", GetTimerType_isrsafe(pSettings), pSettings->type, file, line));
    errCode = timer->functions->create(device, timer, pSettings);
    if (errCode) {
        BDBG_ERR(("BTMR_CreateTimer: failed to create %s(%d) timer (at: %s,%d)", GetTimerType_isrsafe(pSettings), pSettings->type, file, line));
        BKNI_Free(timer);
        return BERR_TRACE(errCode);
    }

    *phTimer = timer; /* here's your handle for using this timer! */

    return BERR_SUCCESS;
}

#if BDBG_DEBUG_BUILD
BERR_Code BTMR_CreateTimer_tagged(BTMR_Handle device, BTMR_TimerHandle *phTimer, const BTMR_TimerSettings *pSettings, const char *file, int line)
#else
BERR_Code BTMR_CreateTimer(BTMR_Handle device, BTMR_TimerHandle *phTimer, const BTMR_TimerSettings *pSettings)
#endif
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BTMR_CreateTimer);
    BDBG_OBJECT_ASSERT(device, btmr_device_t);
    BDBG_ASSERT(phTimer);   /* this is where I put his handle -- don't write to zero */
    BDBG_ASSERT(pSettings); /* this is the timer settings -- he MUST give me some settings */

    /* Some say I shouldn't mess with the return variable unless I return success... */
    *phTimer = NULL; /* just to be sure caller doesn't use it -- you don't have a timer till I give you one! */
    
    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE);

#if 1
    /*
    ** Now I COULD be nice and let them provide a callback when not expected (I just won't call it)
    ** or not provide a callback when I do expect it (nothing provided then nothing to call) but ...
    */
    if (pSettings->type == BTMR_Type_eStopWatch || pSettings->type == BTMR_Type_eSharedFreeRun)
    {
        /* StopWatch and FreeRun timers DON'T GET callbacks! Only used for sampling */
        if (pSettings->cb_isr) {
            BDBG_ERR(("BTMR_CreateTimer: StopWatch timer supplied with call back routine -- illegal!"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        /* Count-down and Periodic timers MUST supply a callback! */
        if (!pSettings->cb_isr) {
            BDBG_ERR(("BTMR_CreateTimer: Count-down or Periodic timer NOT supplied with call back routine -- illegal!"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
#endif
    
    /* 
    ** We have a re-entrancy issue with creating and destroying timers.
    ** Multiple processes can try to create/destroy a timer that can cause the Shared Timer and Virtual Timer counts to get stepped on.
    ** Because much of the work of creating a timer gets done at _isr level (in Critical Section) we have to block almost the entire
    ** create/destroy process.
    */

    BKNI_AcquireMutex(device->create_destroy_mutex);
#if BDBG_DEBUG_BUILD
    errCode = _BTMR_CreateTimer(device, phTimer, pSettings, file, line);
#else
    errCode = _BTMR_CreateTimer(device, phTimer, pSettings, NULL, 0);
#endif
    BKNI_ReleaseMutex(device->create_destroy_mutex);

    BDBG_MSG(("BTMR_CreateTimer: returning handle=0x%x", *(int*)phTimer));
    BDBG_LEAVE(BTMR_CreateTimer);
    return errCode;
}

/* We need to recursively destroy timers.
** This can happen when we're destroying the utility timers that are no longer needed after deleting the last virtual timer.
*/
static BERR_Code _BTMR_DestroyTimer(BTMR_TimerHandle timer, const char *file, int line)
{
    BERR_Code errCode = BERR_SUCCESS;
    BTMR_Handle device;

    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);

    device = timer->device;

    /* Shared Free Run timers only get destroyed on last destroy */
    if (timer->Settings.type == BTMR_Type_eSharedFreeRun && device->SharedCount > 1) 
    {
        BDBG_MSG(("BTMR_DestroyTimer: NOT deleting THE shared timer [still in use by others, count=%d] (at: %s,%d)", device->SharedCount, file, line));
        device->SharedCount--;
        return BERR_SUCCESS;
    }

    /* The shared timer reflects the first one to create one, not necessarily the one destroying it */
    BDBG_MSG(("BTMR_DestroyTimer: destroying %s timer %d (at: %s,%d)", GetTimerType_isrsafe(&timer->Settings), timer->timerNumber, file, line));
    if (timer->Settings.type != BTMR_Type_eSharedFreeRun)
    {
        BDBG_MSG(("BTMR_DestroyTimer: timer %d created: %s,%d", timer->timerNumber, timer->file, timer->line));
    }

    BKNI_EnterCriticalSection();
    errCode = timer->functions->destroy_isr(timer);
    BKNI_LeaveCriticalSection();

    /* Mark it as invalid in case he tries to give it to me again (destroy twice, or operation after destroy) */
    /* Note: this probably won't happen as the memory will be reassigned and stepped on, but ... */
    BDBG_OBJECT_DESTROY(timer, btmr_timer_t);
    BKNI_Free(timer);

    return errCode;
}

#if BDBG_DEBUG_BUILD
BERR_Code BTMR_DestroyTimer_tagged(BTMR_TimerHandle timer, const char *file, int line)
#else
BERR_Code BTMR_DestroyTimer(BTMR_TimerHandle timer)
#endif
{
    BERR_Code errCode = BERR_SUCCESS;
    BTMR_Handle device;

    BDBG_ENTER(BTMR_DestroyTimer);
    BDBG_MSG(("BTMR_DestroyTimer: handle=0x%x", timer));
    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);

    device = timer->device;
    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE);

    /* You are not allowed to delete a timer from within the count-down or periodic timer's callback function.
    ** This is just a bad thing to do.  Besides, the call-back is running from ISR and this isn't an _isr routine.
    */
    if (timer->processing) {
        BDBG_ERR(("BTMR_DestroyTimer: request to destroy timer from a timer callback operation!"));
        return BERR_TRACE(BTMR_ERR_DELETE_FROM_ISR);
    }

    BKNI_AcquireMutex(device->create_destroy_mutex);
#if BDBG_DEBUG_BUILD
    errCode = _BTMR_DestroyTimer(timer, file, line);
#else
    errCode = _BTMR_DestroyTimer(timer, NULL, 0);
#endif
    BKNI_ReleaseMutex(device->create_destroy_mutex);

    BDBG_LEAVE(BTMR_DestroyTimer);
    return errCode;
}

#if BDBG_DEBUG_BUILD
BERR_Code BTMR_GetTimerRegisters_tagged(BTMR_TimerHandle timer, BTMR_TimerRegisters *pRegisters, const char *file, int line)
#else
BERR_Code BTMR_GetTimerRegisters(BTMR_TimerHandle timer, BTMR_TimerRegisters *pRegisters)
#endif
{
    BTMR_Handle device;

    BDBG_ENTER(BTMR_GetTimerRegisters);
    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);
    BDBG_ASSERT(pRegisters); /* don't write results to zero */

    device = timer->device;
    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE);

    BDBG_MSG(("Getting the registers for timer number %d (%s) [%s:%d]", timer->timerNumber, GetTimerType_isrsafe(&timer->Settings), file, line));

    /* They are only supposed to call this function IF the timer was created as exclusive.
    ** Special case: we allow anyone using shared timer to also get its registers.
    ** We NEVER allow them to get the register values of a virtual timer (they don't exist).
    */
    if (!timer->Settings.exclusive && !(timer->Settings.type == BTMR_Type_eSharedFreeRun)) {
        BDBG_ERR(("BTMR_GetTimerRegisters: asked for registers to non-exclusive/non-shared timer!"));
        return BERR_TRACE(BTMR_ERR_EXCLUSIVE_OPERATION);
    }
    
    BKNI_Memcpy(pRegisters, &timer->Registers, sizeof(BTMR_TimerRegisters));

    BDBG_LEAVE(BTMR_GetTimerRegisters);
    return BERR_SUCCESS;
}

static const BTMR_TimerSettings DefaultTimerSettings =  { BTMR_Type_eStopWatch, NULL, NULL, 0, false };

BERR_Code BTMR_GetDefaultTimerSettings(BTMR_TimerSettings *pSettings)
{
    BDBG_ASSERT(pSettings); /* don't write results to zero */

    BKNI_Memcpy(pSettings, &DefaultTimerSettings, sizeof(BTMR_TimerSettings));
    return BERR_SUCCESS;
}

/* This gets the current settings for THIS timer as opposed to the above that gets the default for ANY timer */
BERR_Code BTMR_GetTimerSettings(BTMR_TimerHandle timer, BTMR_TimerSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);
    BDBG_ASSERT(pSettings); /* don't write results to zero */

    BKNI_Memcpy(pSettings, &timer->Settings, sizeof(BTMR_TimerSettings));
    return BERR_SUCCESS;
}

BERR_Code BTMR_StartTimer_isr(BTMR_TimerHandle timer, unsigned startingValue)
{
    BTMR_Handle device;

    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);

    device = timer->device;
    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE);

    /* If this was called from the count-down timer's callback function then we don't actually start it here.
    ** This could mess up the processing in the interrupt service routines.  So just mark it to be started.
    ** Note: we'll get called back here for virtual timers but with "processing" set to false this time.
    ** Also note that we can't provide proper status for this request because it doesn't happen until later.
    */
    if (timer->processing) {
        timer->delayedValue = startingValue;
        timer->pleaseStart = true;
        timer->pleaseStop = false; /* if they both stopped AND started in callback, honor the last one */
        return BERR_SUCCESS;
    }

    /* What should I do with a zero timer request?  Ignore request?  Error?  Make believe the timer expired (i.e. callback)? */
    if (!startingValue && (timer->Settings.type == BTMR_Type_eCountDown || timer->Settings.type == BTMR_Type_ePeriodic)) {
        BDBG_ERR(("BTMR_StartTimer: request to start count-down timer but given a zero count -- don't know what to do with that!!"));
        return BERR_TRACE(BTMR_ERR_NO_TIMEOUT_GIVEN);
    }
    
    /* A periodic timer with too small a timeout will cause a hang (timer continuously times out) */
    if (startingValue < BTMR_MINIMUM_TIMEOUT && timer->Settings.type == BTMR_Type_ePeriodic) {
        BDBG_ERR(("BTMR_StartTimer: request to start periodic timer with too small a timeout -- will cause a interrupt hang!!"));
        return BERR_TRACE(BTMR_ERR_TIMEOUT_TOO_SMALL);
    }

    /* We can't allow a periodic timer to start up with a timeout less than our minimum.
    ** This would allow us to continue to add the timer to the timeout list and then immediately time it out (and hang).
    ** Note: this is different than the MINIMUM_TIMEOUT value (and check) above.
    */
    if (startingValue < BTMR_MIN_HOLD && timer->Settings.type == BTMR_Type_ePeriodic)
        startingValue = BTMR_MIN_HOLD + 1;

    BDBG_MSG(("BTMR_StartTimer: starting %s timer %d (initial=%08x)", GetTimerType_isrsafe(&timer->Settings), timer->timerNumber, startingValue));
    return timer->functions->start_isr(timer, startingValue);
}
BERR_Code BTMR_StartTimer(BTMR_TimerHandle timer, unsigned startingValue)
{
    BERR_Code result;
    BDBG_ENTER(BTMR_StartTimer);
    BKNI_EnterCriticalSection();
    result = BTMR_StartTimer_isr(timer, startingValue);
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BTMR_StartTimer);
    return result;
}

BERR_Code BTMR_StopTimer_isr(BTMR_TimerHandle timer)
{
    BTMR_Handle device;

    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);

    device = timer->device;
    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE);

    /* If this was called from the count-down timer's callback function then we don't actually stop it here.
    ** This could mess up the processing in the interrupt service routines.  So just mark it to be stopped.
    ** Note that we can't provide proper status for this request because it doesn't happen until later.
    */
    if (timer->processing) {
        timer->pleaseStop = true;
        timer->pleaseStart = false; /* if they both stopped AND started in callback, honor the last one */
        return BERR_SUCCESS;
    }

    BDBG_MSG(("BTMR_StopTimer: stopping %s timer %d", GetTimerType_isrsafe(&timer->Settings), timer->timerNumber));
    return timer->functions->stop_isr(timer);
}
BERR_Code BTMR_StopTimer(BTMR_TimerHandle timer)
{
    BERR_Code result;
    BDBG_ENTER(BTMR_StopTimer);
    BKNI_EnterCriticalSection();
    result = BTMR_StopTimer_isr(timer);
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BTMR_StopTimer);
    return result;
}

BERR_Code BTMR_ReadTimer_isr(BTMR_TimerHandle timer, unsigned *pValue)
{
    BTMR_Handle device;

    BDBG_OBJECT_ASSERT(timer, btmr_timer_t);

    device = timer->device;
    if (device->in_standby_mode) return BERR_TRACE(BTMR_ERR_STANBY_ACTIVE);

    if (pValue) *pValue = timer->functions->read_isr(timer);
    return BERR_SUCCESS;
}
BERR_Code BTMR_ReadTimer(BTMR_TimerHandle timer, unsigned *pValue)
{
    BERR_Code result;
    BDBG_ENTER(BTMR_ReadTimer);
    BKNI_EnterCriticalSection();
    result = BTMR_ReadTimer_isr(timer, pValue);
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE(BTMR_ReadTimer);
    return result;
}

/* Note: we passed in a handle in case we ever have different wraps for different timer blocks */
unsigned BTMR_ReadTimerMax(void)
{
    return TimerToMicroSeconds(MaxTimerValue);
}

/* End of File */

