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
* API Description:
*   API name: Platform ucos
*    uCos OS routines
*
***************************************************************************/
#include <stdio.h>

#include "bstd.h"
#include "bkni.h"
#include "blst_squeue.h"
#include "bchp_common.h"

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "nexus_core_utils.h"
#include "bkni.h"

#include "int1.h"
#include "ucos.h"       /* OSRunning */
#include "ucosdrv.h"    /* OSTimeTick() */

//#include "hditaskpriority.h"

BDBG_MODULE(nexus_platform_ucos);

/****************************************************************************
    Externs - should go away as we clean things up
****************************************************************************/
void * IRQInstall(int IRQNum, void (*ISRFun)(void));

/****************************************************************************
    Defines
****************************************************************************/
#if (BCHP_CHIP == 7401)
#define MAX_L1_INTERRUPTS   64
#elif (BCHP_CHIP == 7325)
#define MAX_L1_INTERRUPTS   70
#elif (BCHP_CHIP == 7358 || BCHP_CHIP == 7552)
#define MAX_L1_INTERRUPTS   96
#else
#error "Unsupported chip"
#endif

#define OS_TICK_MS          10
#define MAX_INT_MESSAGES    128

/****************************************************************************
    Typdefs and Structures
****************************************************************************/
typedef struct {
    NEXUS_Core_InterruptFunction pFunc;
    void *   pFuncParam;
    int      iFuncParam;
    bool     enabled;
    unsigned count;        /* for statistical purposes */
} NEXUS_Platform_Isr;

/* Create a singly-linked Q to manage L1 interrupts. Create a list to manage
   the pool of free interrupt objects, otherwise we would have to alloc/free
   at run-time.
*/
typedef struct NEXUS_Platform_IntObject {
    BLST_SQ_ENTRY(NEXUS_Platform_IntObject) next;
    unsigned irqNum;
} NEXUS_Platform_IntObject;

/* Our component context */
typedef struct {
    BKNI_EventHandle hIsrEvent;
    NEXUS_ThreadHandle hIsrThread;

    bool bPostOsInit;
    
    /* Create the head pointer and initialize it */
    BLST_SQ_HEAD(NEXUS_Platform_IntObjectList, NEXUS_Platform_IntObject) intList;
    BLST_SQ_HEAD(NEXUS_Platform_IntObjectPool, NEXUS_Platform_IntObject) intPool;

    NEXUS_Platform_IntObject intObjectArray[MAX_INT_MESSAGES];
    
    NEXUS_Platform_Isr isrArray[MAX_L1_INTERRUPTS];
    
} NEXUS_Platform_Context;


/****************************************************************************
    Static Variables
****************************************************************************/
static NEXUS_Platform_Context gContext;
static NEXUS_Platform_Context * gpContext = NULL;

/****************************************************************************
    Static Functions
****************************************************************************/
static void b_ucos_isr(void *intHandle, int intId);
static BERR_Code b_ucos_task_mode_init(void);
static void b_ucos_task_mode_isr(void * pParam);
static void b_ucos_timer_init(void);

/****************************************************************************
****************************************************************************/
NEXUS_Error 
NEXUS_Platform_P_InitOS(void)
{
    int i;
    BERR_Code rc;
    
    BDBG_MSG(("uCOS Platform Initialization"));

    if (gpContext != NULL) {
        return NEXUS_SUCCESS;
    }
    
    gpContext = &gContext;
    BKNI_Memset(gpContext, 0x00, sizeof(*gpContext));
    
    BLST_SQ_INIT(&gpContext->intList);
    BLST_SQ_INIT(&gpContext->intPool);

    rc = BKNI_CreateEvent(&gpContext->hIsrEvent);
    if (rc != BERR_SUCCESS) {
        /* No need to go any further. System's dead if we can't do this */
        rc = BERR_TRACE(rc);
        BKNI_Fail();
    }

    for (i = 0; i < MAX_INT_MESSAGES; i++) {
        BLST_SQ_INSERT_HEAD(&gpContext->intPool, &(gpContext->intObjectArray[i]), next);
    }
    
    b_ucos_task_mode_init();

    /* Install main dispatcher for CAUSE_IP3 */
    IRQInstall(2, INT1_Isr);

    return NEXUS_SUCCESS;
}

/****************************************************************************
****************************************************************************/
NEXUS_Error 
NEXUS_Platform_P_UninitOS(void)
{
    return NEXUS_SUCCESS;
}

/****************************************************************************
****************************************************************************/
void *
NEXUS_Platform_P_MapMemory(unsigned long offset, unsigned long length, bool cached)
{
    BSTD_UNUSED(length);

    BDBG_MSG(("MapMemory: o=%08x l=%d", offset, length));

    if (cached) {
        return (void *)(offset | 0x80000000ul);
    } else {
        return (void *)(offset | 0xA0000000ul);
    }
}

/****************************************************************************
****************************************************************************/
void 
NEXUS_Platform_P_UnmapMemory(void *pMem, unsigned long length)
{
    BSTD_UNUSED(pMem);
    BSTD_UNUSED(length);

    return;
}

/****************************************************************************
****************************************************************************/
NEXUS_Error 
NEXUS_Platform_P_GetMainMemory(unsigned long defaultOsSize, unsigned long strapMemSize, unsigned long *pDriverBase, unsigned long *pDriverLength)
{
    BSTD_UNUSED(defaultOsSize);
    BSTD_UNUSED(strapMemSize);

    BDBG_ASSERT(pDriverBase);
    BDBG_ASSERT(pDriverLength);

    BDBG_MSG(("NEXUS_Platform_P_GetMainMemory"));

    BDBG_ERR(("OsSize: %dMB StrapSize: %dMB", defaultOsSize >> 20, strapMemSize >> 20));

    /* Override the base/length. Assume 128MB system for now */
    if (strapMemSize > (128 * 1024 * 1024)) {
//        BDBG_ERR(("Reduce StrapSize to 128"));
//        strapMemSize = (128 * 1024 * 1024);
    }
    *pDriverBase = defaultOsSize;
    *pDriverLength = strapMemSize - *pDriverBase;

    BDBG_ERR(("Base: %08x Length: %d", *pDriverBase, *pDriverLength));


    return NEXUS_SUCCESS;
}

/****************************************************************************
****************************************************************************/
void 
NEXUS_Platform_P_ResetInterrupts(void)
{
    BDBG_MSG(("NEXUS_Platform_P_ResetInterrupts"));
    /* TBD -- what is this supposed to do??? */
}

/****************************************************************************
****************************************************************************/
NEXUS_Error 
NEXUS_Platform_P_ConnectInterrupt(unsigned irqNum, 
    NEXUS_Core_InterruptFunction pIsrFunc, void *pFuncParam, int iFuncParam)
{
    BERR_Code rc;
    
    /* We need to initialize some things after the rest of the system is up */
    if (gpContext->bPostOsInit == false) {
        b_ucos_timer_init();
        INT1_Init(g_pCoreHandles->reg);
        gpContext->bPostOsInit = true;
    }

    if (irqNum >= MAX_L1_INTERRUPTS) {
        BDBG_ERR(("Invalid int number %d", irqNum));
        return NEXUS_OS_ERROR;
    }

    if (gpContext->isrArray[irqNum].pFunc) {
        BDBG_ERR(("Interrupt %d already registered - can't register twice!", irqNum));
        return NEXUS_OS_ERROR;
    }

    gpContext->isrArray[irqNum].pFunc = pIsrFunc;
    gpContext->isrArray[irqNum].pFuncParam = pFuncParam;
    gpContext->isrArray[irqNum].iFuncParam = iFuncParam;
    gpContext->isrArray[irqNum].enabled = false;
    gpContext->isrArray[irqNum].count = 0;

    BDBG_MSG(("Connect IRQ %d (pFunc=%p, pFuncParam=%p, iFuncParam=0x%x(%d))", 
        irqNum, pIsrFunc, pFuncParam, iFuncParam, iFuncParam));

    /* iFuncParam is saved above and irqNum is passed down */
    rc = INT1_ConnectIsr(irqNum, b_ucos_isr, pFuncParam, irqNum);
    if (rc != BERR_SUCCESS) {rc = BERR_TRACE(rc); return NEXUS_OS_ERROR; }

    return NEXUS_SUCCESS;
}

/****************************************************************************
****************************************************************************/
void 
NEXUS_Platform_P_DisconnectInterrupt(unsigned irqNum)
{
    BDBG_MSG(("Disconnect ISR %d", irqNum));
    
    if (irqNum >= MAX_L1_INTERRUPTS) {
        BDBG_ERR(("Invalid int number %d", irqNum));
        return;
    }

    if (gpContext->isrArray[irqNum].pFunc) {
        INT1_RemoveIsr(irqNum);
        gpContext->isrArray[irqNum].enabled = false;
        gpContext->isrArray[irqNum].pFunc = NULL;
    }
}

/****************************************************************************
****************************************************************************/
NEXUS_Error 
NEXUS_Platform_P_EnableInterrupt_isr(unsigned irqNum)
{
    gpContext->isrArray[irqNum].enabled = true;
    INT1_EnableIsr(irqNum);

    return NEXUS_SUCCESS;
}

/****************************************************************************
****************************************************************************/
NEXUS_Error 
NEXUS_Platform_P_EnableInterrupt(unsigned irqNum)
{
    NEXUS_Error rc;
    
    BDBG_MSG(("EnableInt: %d", irqNum));
    BKNI_EnterCriticalSection();
    rc = NEXUS_Platform_P_EnableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();

    return rc;
}

/****************************************************************************
****************************************************************************/
void 
NEXUS_Platform_P_DisableInterrupt_isr(unsigned irqNum)
{
    gpContext->isrArray[irqNum].enabled = false;
    INT1_DisableIsr(irqNum);
}

/****************************************************************************
****************************************************************************/
void 
NEXUS_Platform_P_DisableInterrupt(unsigned irqNum)
{
    BDBG_MSG(("DisableInt: %d", irqNum));
    BKNI_EnterCriticalSection();
    NEXUS_Platform_P_DisableInterrupt_isr(irqNum);
    BKNI_LeaveCriticalSection();

    return;
}

/****************************************************************************
****************************************************************************/
static bool g_NEXUS_magnum_init = false;
NEXUS_Error
NEXUS_Platform_P_Magnum_Init(void)
{
#if 0
    BERR_Code rc;

    if (!g_NEXUS_magnum_init) {
        rc = BKNI_Init();
        if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
        rc = BDBG_Init();
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); BKNI_Uninit();return rc;}
        g_NEXUS_magnum_init = true;
    }
    return BERR_SUCCESS;
#else
BERR_Code rc;

if(!g_NEXUS_magnum_init) {
    rc = BKNI_Init();
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    rc = BDBG_Init();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); BKNI_Uninit();return rc;}
    rc = NEXUS_Base_Core_Init();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); BDBG_Uninit();BKNI_Uninit();return rc;}
   g_NEXUS_magnum_init = true;
}
return BERR_SUCCESS;

#endif    
}

/****************************************************************************
****************************************************************************/
void 
NEXUS_Platform_P_Magnum_Uninit(void)
{
    if (g_NEXUS_magnum_init) {
        BDBG_Uninit();
        BKNI_Uninit();
        g_NEXUS_magnum_init = false;
    }
}


/****************************************************************************
****************************************************************************/
static void 
b_ucos_task_mode_isr(void * pParam)
{
    NEXUS_Error err;
    NEXUS_Platform_Isr * pIsr;
    NEXUS_Platform_IntObject *pInt;

    while (1) {
        BKNI_WaitForEvent(gpContext->hIsrEvent, BKNI_INFINITE);

        /* Make sure we pull the interrupt off the head inside a critical 
           section. Don't want the L1 isr to change the head pointer while
           we're operating on it.
        */
        while (! BLST_SQ_EMPTY(&gpContext->intList)) {

            OSEnterCritical();
            pInt = BLST_SQ_FIRST(&gpContext->intList);
            BLST_SQ_REMOVE_HEAD(&gpContext->intList, next);
            OSExitCritical();

            if (pInt->irqNum >= MAX_L1_INTERRUPTS) {
                err = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto invalidIsr;
            }

            pIsr = &(gpContext->isrArray[pInt->irqNum]);
            
            if (pIsr->pFunc == NULL) {
                err = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto invalidIsr;
            }
            
            /* Call the ISR within a BKNI critical section. This allows
               interrupts to continue to flow 
            */
            BKNI_EnterCriticalSection();
            pIsr->pFunc(pIsr->pFuncParam, pIsr->iFuncParam);

            if (pIsr->enabled == true) {
                INT1_EnableIsr(pInt->irqNum);
            }
            BKNI_LeaveCriticalSection();

            /* Return this int object to the pool */
invalidIsr:            
            OSEnterCritical();
            BLST_SQ_INSERT_HEAD(&gpContext->intPool, pInt, next);
            OSExitCritical();
        }   /* interrupt list processing loop */
    }   /* Infinite task loop. Never exits */
}

/****************************************************************************
    Initialize the task mode driver model
****************************************************************************/
static BERR_Code
b_ucos_task_mode_init(void)
{
    NEXUS_ThreadSettings threadSettings;

    NEXUS_Thread_GetDefaultSettings(&threadSettings);
    threadSettings.priority = 8;
    gpContext->hIsrThread = NEXUS_Thread_Create("ISR Thread", b_ucos_task_mode_isr, NULL, &threadSettings);
    if (gpContext->hIsrThread == NULL) {
        BDBG_ERR(("Failed to create ISR thread"));
        return BERR_OS_ERROR;
    }
    
    return BERR_SUCCESS;
}

/****************************************************************************
****************************************************************************/
static void 
b_ucos_isr(void *intHandle, int intId)
{
    NEXUS_Platform_IntObject *pInt;

    if (BLST_SQ_EMPTY(&gpContext->intPool)) {
        /* This should be a fatal system error because we should never drop
           interrupts, but we may be able to continue to function, so just 
           return. 
        */
        BDBG_ERR(("INT Q Overflow!!"));
        return;
    }

    /* Disable the L1 interrupt at this level before beginning interrupt 
       processing. We'll re-enable it in the (task level) service routine. 
    */
    INT1_DisableIsr(intId);
    pInt = BLST_SQ_FIRST(&gpContext->intPool);
    BLST_SQ_REMOVE_HEAD(&gpContext->intPool, next);
    pInt->irqNum = intId;
    BLST_SQ_INSERT_TAIL(&gpContext->intList, pInt, next);
    BKNI_SetEvent_isr(gpContext->hIsrEvent);
}

/****************************************************************************
****************************************************************************/
static void 
b_ucos_timer_isr(void * pParm1, int iParm2)
{
    /* Announce OSTimeTick after OSRunning is set */
    extern BOOLEAN OSRunning;   

    if (OSRunning) {
        OSTimeTick();
    }
}

/****************************************************************************
    Using a chip timer until we can tap into the MIPS count/compare timer
****************************************************************************/
static void
b_ucos_timer_init(void)
{
    BERR_Code rc;
    BTMR_Handle hTimer;
    BTMR_Settings sTimerSettings;
    BTMR_TimerHandle hOsTimer;

    hTimer = g_pCoreHandles->tmr;
    
    if (hTimer == NULL) {
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        BKNI_Fail();
    }

    BTMR_GetDefaultTimerSettings(&sTimerSettings);
    
    sTimerSettings.type      = BTMR_Type_ePeriodic;
    sTimerSettings.exclusive = false;
    sTimerSettings.cb_isr    = b_ucos_timer_isr;
    sTimerSettings.pParm1    = NULL;
    sTimerSettings.parm2     = 0;

    rc = BTMR_CreateTimer(hTimer, &hOsTimer, &sTimerSettings);
    if (rc != BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        BKNI_Fail();
    }

    /* Initial value is in us. We're using a 10ms timer tick */
    rc = BTMR_StartTimer(hOsTimer, OS_TICK_MS * 1000);
    if (rc != BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        BKNI_Fail();
    }
}

NEXUS_Error NEXUS_Platform_P_GetHostMemory(NEXUS_PlatformMemory *pMemory)
{
    pMemory->osRegion[0].base = 0x04000000;
    pMemory->osRegion[0].length = pMemory->memc[0].length - 0x04000000;
    return BERR_SUCCESS;
}

void NEXUS_Platform_P_AtomicUpdateCallback_isr(void *callbackContext, uint32_t reg, uint32_t mask, uint32_t value)
{
    uint32_t temp;
    BSTD_UNUSED(callbackContext);
    temp = BREG_Read32(g_pCoreHandles->reg, reg);
    temp &= ~mask;
    temp |= value;
    BREG_Write32(g_pCoreHandles->reg, reg, temp);
}

NEXUS_Error NEXUS_Platform_P_InitOSMem(void)
{
    return 0;
}

void NEXUS_Platform_P_UninitOSMem(void)
{
    return;
}


