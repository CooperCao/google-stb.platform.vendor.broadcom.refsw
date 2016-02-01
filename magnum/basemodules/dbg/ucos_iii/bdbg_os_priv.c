/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Revision: $
 * $brcm_Date: $
 * $brcm_Workfile: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/* uCOS include files */
#include <os.h>
#include <os_cfg_app.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg_os_priv.h"
#include <sys/time.h>

extern uint32_t total_ticks;

#define CHECK_CRITICAL() ( bos_in_interrupt() || (OSSchedLockNestingCtr > 0) || (OSIntNestingCtr > 0))
#define TICKS_TO_MS(ticks)	(ticks * 1000/OS_CFG_TICK_RATE_HZ)
#define MS_TO_TICKS(x)      ((x * OS_CFG_TICK_RATE_HZ)/ 1000)

static unsigned int initTicks;
static bool bdbg_init=false;
static OS_MUTEX g_mutex;

void
BDBG_P_InitializeTimeStamp(void)
{
    OS_ERR err;
    initTicks = OSTimeGet(&err);
    if (err != OS_ERR_NONE) {
        BDBG_P_PrintString("OSTimeGet did not return OS_ERR_NONE\n");
        BDBG_ASSERT(0);
    }
}

void
BDBG_P_GetTimeStamp(char *timeStamp, int size_t)
{
    unsigned int currentTicks;
    int hours, minutes, seconds;
    int milliseconds;
    int rc;
    OS_ERR err;

    currentTicks = OSTimeGet(&err);
    if (err != OS_ERR_NONE) {
        BDBG_P_PrintString("OSTimeGet did not return OS_ERR_NONE\n");
        BDBG_ASSERT(0);
    }

    milliseconds = (currentTicks - initTicks)*(1000/OS_CFG_TICK_RATE_HZ);

    /* Calculate the time   */
    seconds = milliseconds / 1000;
    milliseconds = milliseconds % 1000;
    minutes = seconds / 60;
    seconds = seconds % 60;
    hours = minutes / 60;
    minutes = minutes % 60;

    /* print the formatted time including the milliseconds  */
    rc = BKNI_Snprintf(timeStamp, size_t, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    return;
}

#ifdef MIPS_SDE
/***************************************************************************
Summary:
    Macro to write a cp0 register.

Description:
    asm macro to write a cp0 register given the register, select and value. (MIPS32)

See Also:
    bcm_read_cp0
***************************************************************************/
#define bcm_write_cp0(reg, sel, value)                  \
{       __asm__ __volatile__(".set\tpush\n\t"           \
            ".set\tmips32\n\t"                          \
            "mtc0\t%z0, " #reg ", " #sel "\n\t" \
            ".set\tpop\n\t"                         \
            : /* none */                                \
            : "r" ((unsigned int)value));               \
}

/***************************************************************************
Summary:
    Macro to read a cp0 register.

Description:
    asm macro to read a cp0 register given the register and select. (MIPS32)

See Also:
    bcm_read_cp0
***************************************************************************/
#define bcm_read_cp0(reg, sel)                          \
({ unsigned int bcm_read_cp0_res;                       \
        __asm__ __volatile__(   ".set\tpush\n\t"        \
            ".set\tmips32\n\t"                          \
            "mfc0\t%0, " #reg ", " #sel "\n\t"          \
            ".set\tpop\n\t"                         \
            : "=r" (bcm_read_cp0_res));                 \
    bcm_read_cp0_res;                                   \
})
#endif

/*****************************************************************************
 bos_in_interrupt()
*****************************************************************************/
bool bos_in_interrupt(void)
{
   uint32_t status;

#ifdef MIPS_SDE
   status = bcm_read_cp0($12, 0);
   return (status & 0x00000002) ? true : false;
#else
   status = OS_CPU_SR_Get();
   return ((status & 0x1f) == 0x12) ? true : false;
#endif
}

BERR_Code BDBG_P_OsInit(void)
{
    OS_ERR err;

    if (bdbg_init != false) {
        BDBG_ASSERT(0);
    }

    OSMutexCreate((OS_MUTEX *)&g_mutex,
                  (CPU_CHAR *)BDBG_P_OsInit,
                  (OS_ERR   *)&err);
    if (err != OS_ERR_NONE) {
        BDBG_P_PrintString("OSMutexCreate did not return OS_ERR_NONE\n");
        BDBG_ASSERT(0);
    }

    bdbg_init = true;

    return BERR_SUCCESS;
}

void BDBG_P_OsUninit(void)
{
    OS_ERR err;
    /* No way to release kernel object in uCOS */
    if (bdbg_init) {
        OSMutexDel((OS_MUTEX *)&g_mutex,
                   (OS_OPT    )OS_OPT_DEL_NO_PEND,
                   (OS_ERR   *)&err);
        if (err != OS_ERR_NONE) {
            BDBG_P_PrintString("OSMutexCreate did not return OS_ERR_NONE\n");
            BDBG_ASSERT(0);
        }
        bdbg_init = false;
    }
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Lock(void)
{
    /*unsigned char ucosError;*/
    OS_ERR err;

    if (bdbg_init == false) {
        /*ucosError = BERR_TRACE(BERR_NOT_INITIALIZED);*/
        return;
    }
    
    if (CHECK_CRITICAL()) {
        return;
    }
    
    /* Wait forever on this mutex */
    OSMutexPend((OS_MUTEX *)&g_mutex,
                (OS_TICK   )0,
                (OS_OPT    )OS_OPT_PEND_BLOCKING,
                (CPU_TS   *)NULL,
                (OS_ERR   *)&err);
    if (err != OS_ERR_NONE) {
        BDBG_P_PrintString("OSMutexPend did not return OS_ERR_NONE\n");
        BDBG_ASSERT(0);
    }
}

/* NOTE: this function is called from both magnum task and isr context */
void BDBG_P_Unlock(void)
{
    /*BERR_Code rc;*/
    OS_ERR err;

    if (bdbg_init==false) {
        /*rc = BERR_TRACE(BERR_NOT_INITIALIZED);*/
        return;
    }

    if (CHECK_CRITICAL()) {
        return;
    }

    OSMutexPost((OS_MUTEX *)&g_mutex,
                (OS_OPT    )OS_OPT_POST_NONE,
                (OS_ERR   *)&err);
    if (err != OS_ERR_NONE) {
        BDBG_P_PrintString("OSMutexPost did not return OS_ERR_NONE\n");
        BDBG_ASSERT(0);
    }
}

/* End of file */
