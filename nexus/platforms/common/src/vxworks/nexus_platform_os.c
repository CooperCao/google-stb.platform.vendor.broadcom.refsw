/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
*   API name: Platform vxworks
*    VxWorks OS routines
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "bkni.h"
#include "bchp_common.h"

BDBG_MODULE(nexus_platform_vxworks);

#define BDBG_MSG_IRQ(x)  /* BDBG_MSG(x) */

/* forward reference */
static int b_intmgr_vxInit(void);
static void b_intmgr_vxShutdown(void);

NEXUS_Error 
NEXUS_Platform_P_InitOS(void)
{
    BDBG_MSG(("Platform Initialization"));

    if (b_intmgr_vxInit() < 0)
        return NEXUS_OS_ERROR;
    else
        return NEXUS_SUCCESS;
}

NEXUS_Error 
NEXUS_Platform_P_UninitOS(void)
{
    b_intmgr_vxShutdown();
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Platform_P_InitOSMem(void)
{
	/* We don't need to do anything special here */
    return 0;
}

void NEXUS_Platform_P_UninitOSMem(void)
{
}

void *
NEXUS_Platform_P_MapMemory(unsigned long offset, unsigned long length, bool cached)
{
    BSTD_UNUSED(length);
    if (cached)
        return (void *)(offset | 0x80000000ul);
    else
        return (void *)(offset | 0xA0000000ul);
}

void 
NEXUS_Platform_P_UnmapMemory(void *pMem, unsigned long length)
{
    BSTD_UNUSED(pMem);
    BSTD_UNUSED(length);
    return;
}

#include <sysLib.h>

NEXUS_Error 
NEXUS_Platform_P_GetMainMemory(unsigned long defaultOsSize, unsigned long strapMemSize, unsigned long *pDriverBase, unsigned long *pDriverLength)
{
    unsigned long memStart = defaultOsSize;     

    BDBG_ASSERT(pDriverBase);
    BDBG_ASSERT(pDriverLength);

    /* Value returned from sysMemTop is physical memory address -- make it an offset */
    memStart = (unsigned long)((int)sysMemTop() - 0x80000000);

    /* I should always get a memory size, but ... use this as default */
    if (!strapMemSize) strapMemSize = 256*1024*1024;

    *pDriverBase = memStart;
    *pDriverLength = strapMemSize - memStart;
    BDBG_MSG(("top=%x, base=%x, length=%x (%d.)\n", sysMemTop(), *pDriverBase, *pDriverLength, *pDriverLength));

    return BERR_SUCCESS;
}


void 
NEXUS_Platform_P_ResetInterrupts(void)
{
    /* TBD -- what is this supposed to do??? */
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#undef CPUINT1_ConnectIsr /* is this needed anymore? */

/*#include "int1_api.h"*/
extern void CPUINT1_Disable(unsigned long);
extern void CPUINT1_Enable(unsigned long);
typedef void (*FN_L1_ISR) (void *, int);
extern int  CPUINT1_ConnectIsr(unsigned long, FN_L1_ISR, void*, int);

#include <taskLib.h>
#include <msgQLib.h>
#include <intLib.h>

#define MAX_INT_MSG        64   /* the maximum number of interrupt messages we'll let queue up */
#define INT_MSG_START_ID   1234 /* magic value to identify the start of a interrupt message */
#define INT_MSG_END_ID     6789 /* magic value to identify the end of an interrupt message */

#define MAX_INTERRUPTS     74 /* Roy, this varies from board to board -- get from external source */

typedef void *(*isr_t)();

/* We want to register a callback function with two callback parameters (a pointer and a int).
** The VxWorks isr callback function only includes the interrupt number and a single callback parameter (a pointer).
** So we stash away the second parameter (with the callback function) so we can retreive it later.
*/
static struct {
    isr_t pFunc;
    void *pFuncParam; /* record this too -- we could use it to validate what we get back from the OS isr function */
    int iFuncParam;
    bool enabled;
    int count;        /* for statistical purposes */
} isr_array[MAX_INTERRUPTS];

#if 1
#include <stdio.h>

void *the_pParam; int the_intId, the_iParam;
void last_interrupt(int id, void *pParam, int iParam)
{
    the_intId  = id, 
    the_pParam = pParam; 
    the_iParam = iParam;
}

void dump_ints()
{
    int i;
    for (i=0; i<MAX_INTERRUPTS; i++) {
        if (isr_array[i].pFunc) {
            printf("ISR %d: (%s) func=%p, pFuncParam=%p, iFuncParam=%d (count=%d)\n",
                   i,
                   isr_array[i].enabled?"enabled":"disabled",
                   isr_array[i].pFunc,
                   isr_array[i].pFuncParam,
                   isr_array[i].iFuncParam,
                   isr_array[i].count);
            isr_array[i].count = 0; /* reset interrupt counts -- who is still interrupting??? */
        }
    }
}
#endif

/* 
** We want to serialize the interrupts and handle them at task level.
** So, on an interrupt, we bundle the information about the interrupt into a message and send it to a high priority task for processing.
** The particular interrupt is disabled until the task processes the interrupt.
** This is the format of the message used to process the interrupt!
** Note: we don't trust the message system (??) so we put markers before and after the message data to ensure it was received uncorrupted!
*/
typedef struct {
    int intMsgStartId; /* marker before */
    int intId;
    isr_t pFunc;
    void *pFuncParam;
    int iFuncParam;
    int intMsgEndId;   /* marker after */
} InterruptMessage;

static struct {
    int taskId;
    MSG_Q_ID msgQId;
} isrTask;

static void ProcessOne(int id, void *pParam, int iParam, InterruptMessage *intMsg)
{
    int key;
    last_interrupt(id, pParam, iParam);

    BKNI_EnterCriticalSection();
    key = intLock();
    CPUINT1_Enable(intMsg->intId);
    (*intMsg->pFunc)(intMsg->pFuncParam,intMsg->iFuncParam);
    intUnlock(key);
    BKNI_LeaveCriticalSection();
}

static void 
vxInterruptTask(void)
{
    InterruptMessage intMsg;
    int rc;
    int errors = 0;

    for (;;) 
    {
        /* Wait for the new interrupt message to come in.
        ** Note: under normal operating conditions, we should be getting an interrupt all the time.
        ** Thus, if we include a timeout, we'll know when something is messed up.
        */
        rc = msgQReceive(isrTask.msgQId, (char*)&intMsg, sizeof(intMsg), 100 /*WAIT_FOREVER*/);

#define MAX_MESSAGES 5 /* only print this many failure messages until starts to work again */

        if (rc == ERROR || rc != sizeof(InterruptMessage)) {
            /* If the main task dies this task will print this repeatedly (when the timeout is set to 100 above) -- don't! */
            if (errors < MAX_MESSAGES) {
                BDBG_ERR(("msgQReceive failed rc=%d", rc));
            } else if (errors == MAX_MESSAGES) {
                BDBG_ERR(("msgQReceive failing (not printing anymore till receiving messages again)"));
            }
            errors++; /* note: this could wrap and print again, but ... so what */
            continue;
        }
        if (errors) {
            BDBG_ERR(("msgQReceive succeeding (received %d failures))", errors));
            errors = 0;
        }

        /* We don't trust the message system, so make sure this is a real message */
        if ((intMsg.intMsgStartId != INT_MSG_START_ID) || (intMsg.intMsgEndId != INT_MSG_END_ID)) {
            BDBG_ERR(("invalid interrupt id: %d %d", intMsg.intMsgStartId, intMsg.intMsgEndId));
            break;
        }

	/* We shouldn't have an interrupt that no one registered for, right? */
        if (isr_array[intMsg.intId].pFunc == NULL) {
            BDBG_ERR(("No ISR for interrupt id: %d", intMsg.intId));
            break;
        }
#if 0
	/* They could re-register a pending interrupt -- should we allow this?? */
        if (isr_array[intMsg.intId].pFunc == intMsg.pFunc) {
            BDBG_ERR(("The callback functions for interrupt %d don't match", intMsg.intId));
        }

	/* Note: its possible that they disabled the interrupt after it was received and processed as a message by the service routine.
	** But BEFORE the message was processed at task level (here).  We could check the enabled flag and toss this message...
	** We're relying on the fact that the interrupt is indeed disabled or we'll just hang forever processing an interrupt no one wants.
	*/
        if (isr_array[intMsg.intId].enabled != true) {
        	CPUINT1_Enable(intMsg.intId);
		continue;
	}
#endif

        isr_array[intMsg.intId].count++; /* keep statistics for dumping */

	/* Process one callback for this interrupt */
	ProcessOne(intMsg.intId, intMsg.pFuncParam, intMsg.iFuncParam, &intMsg);
    }
}

#include <logLib.h>

static void b_vxworks_Isr(void *intHandle, int intId)
{
    InterruptMessage intMsg; /* messages are copied so okay to build using stack */

    /* Disable interrupts at this level (and lower) before beginning interrupt processing */
    /* We'll re-enable it in the (task level) service routine. */
    CPUINT1_Disable(intId);

    intMsg.intMsgStartId = INT_MSG_START_ID;
    intMsg.intId         = intId;
    intMsg.pFunc         = isr_array[intId].pFunc;
    intMsg.pFuncParam    = intHandle; /* this is the pFuncParam passed in a connect time */
    intMsg.iFuncParam    = isr_array[intId].iFuncParam;
    intMsg.intMsgEndId   = INT_MSG_END_ID;

    if (msgQSend(isrTask.msgQId, (char*)&intMsg, sizeof(intMsg), NO_WAIT, MSG_PRI_NORMAL) != OK)
        logMsg("b_vxworks_Isr: msg send failed\n", 0,0,0,0,0,0);
}

static int b_intmgr_vxInit(void)
{	
    /* First, make sure no interrupts were left enabled.
    ** Note: you can't do this -- this prevents us from starting up (left here so you'll know not to do this).
    for (i=0; i<MAX_INTERRUPTS; i++) CPUINT1_Disable(i);
    */

    /* Create a message queue for (serialized) processing of interrupts.
    ** Interrupts are converted into a message and handled by a high priority processing task.
    */
    isrTask.msgQId = msgQCreate(MAX_INT_MSG, sizeof(InterruptMessage), MSG_Q_FIFO);
    if (isrTask.msgQId == NULL) {
        BDBG_ERR(("msgQCreate failed"));
        return -1;
    }

    memset(isr_array, 0, sizeof(isr_array));

    /* Create the task for interrupt handling.
    ** We chose priority of 10 to still leave a little headroom for other support tasks.
    */
    isrTask.taskId = taskSpawn((char *) "vxInterruptTask", 
                               10, /* task priority */
                               0 /* no floating point VX_FP_TASK */, 
                               4096, /* stack size */
                               (FUNCPTR) vxInterruptTask,
                               0,0,0,0,0,0,0,0,0,0);
    if (isrTask.taskId == ERROR) {
        BDBG_ERR(("b_intmgr_vxInit: failed, can't create task"));
        return -1;
    }

    return 0;
}

static void b_intmgr_vxShutdown(void)
{
    int i;

    /* First, stop any new interrupts from coming in. */
    /* Only disable the interrupts that were registered (disabling everything is bad). */
    for (i=0; i<MAX_INTERRUPTS; i++)
        if (isr_array[i].pFunc) 
            CPUINT1_Disable(i);

    BKNI_EnterCriticalSection();
    taskDelete(isrTask.taskId);
    BKNI_LeaveCriticalSection();
    msgQDelete(isrTask.msgQId);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

NEXUS_Error 
NEXUS_Platform_P_ConnectInterrupt(unsigned irqNum, NEXUS_Core_InterruptFunction pIsrFunc, void *pFuncParam, int iFuncParam)
{
    if (irqNum >= MAX_INTERRUPTS) {
        BDBG_ERR(("Interrupt number is greater then %d (irqNum=%0x=%d.)", MAX_INTERRUPTS, irqNum, irqNum));
        return NEXUS_OS_ERROR;
    }

    if (isr_array[irqNum].pFunc) {
        BDBG_ERR(("Interrupt number %d is already registered for a callback - can't register twice!)", irqNum));
        return NEXUS_OS_ERROR;
    }

    /* The OS connect routine only lets us register a single callback parameter (void*).
    ** We save away the other data for use when we receive the interrupt.
    */
    isr_array[irqNum].pFunc = (void*)pIsrFunc;
    isr_array[irqNum].pFuncParam = pFuncParam;
    isr_array[irqNum].iFuncParam = iFuncParam;
    isr_array[irqNum].enabled = false;
    isr_array[irqNum].count = 0;

    BDBG_MSG(("Connecting ISR %d (pFunc=%p, pFuncParam=%p, iFuncParam=%x(%d))", irqNum, pIsrFunc, pFuncParam, iFuncParam, iFuncParam));

    /* iFuncParam is saved above and irqNum is passed down */
    CPUINT1_ConnectIsr(irqNum, (FN_L1_ISR) b_vxworks_Isr, pFuncParam, irqNum);

    return NEXUS_SUCCESS;
}

void 
NEXUS_Platform_P_DisconnectInterrupt(unsigned irqNum)
{
    BDBG_MSG(("Disconnecting ISR %d", irqNum));
    if (isr_array[irqNum].pFunc) {
        /*CPUINT1_DisconnectIsr(irqNum);*/   /* Not sure this function exists... */
        isr_array[irqNum].pFunc = NULL;
    }
}

NEXUS_Error 
NEXUS_Platform_P_EnableInterrupt_isr(unsigned irqNum)
{
    CPUINT1_Enable(irqNum);
    isr_array[irqNum].enabled = true;
    return NEXUS_SUCCESS;
}

NEXUS_Error 
NEXUS_Platform_P_EnableInterrupt(unsigned irqNum)
{
    NEXUS_Error rc;
    BKNI_EnterCriticalSection();
    rc = NEXUS_Platform_P_EnableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();
    return rc;
}

void 
NEXUS_Platform_P_DisableInterrupt_isr(unsigned irqNum)
{
    isr_array[irqNum].enabled = false;
    CPUINT1_Disable(irqNum);
}

void 
NEXUS_Platform_P_DisableInterrupt(unsigned irqNum)
{
    BKNI_EnterCriticalSection();
    NEXUS_Platform_P_DisableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();
    return;
}

static bool g_NEXUS_magnum_init = false;

NEXUS_Error
NEXUS_Platform_P_Magnum_Init(void)
{
    BERR_Code rc;

    if (!g_NEXUS_magnum_init) {
        rc = BKNI_Init();
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }
        rc = BDBG_Init();
        if (rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); BKNI_Uninit(); return rc; }
        g_NEXUS_magnum_init = true;
    }
    return BERR_SUCCESS;
}

void 
NEXUS_Platform_P_Magnum_Uninit(void)
{
    if(g_NEXUS_magnum_init) {
        BDBG_Uninit();
        BKNI_Uninit();
        g_NEXUS_magnum_init = false;
    }
    return;
}

void 
NEXUS_Platform_P_InitDone(void)
{
    /* Currently, nothing to do for vxworks user */
    return;
}

