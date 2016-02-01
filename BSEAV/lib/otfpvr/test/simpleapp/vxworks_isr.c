/***************************************************************************
 *     (c)2012 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#include "blst_list.h"

/* #include <sysLib.h> */
#include <semLib.h>
#include <taskLib.h>
#include <msgQLib.h>
#include <intLib.h>
#include <logLib.h>
#include <timers.h>

#include "bstd_defs.h"
#include "berr.h"
#include "bdbg.h"

#if (BCHP_CHIP == 7401)
#include "bint_7401.h"
#endif
#if (BCHP_CHIP == 7400)
#include "bint_7400.h"
#endif
#include "int1_api.h"
#include "bint.h"
#include "bint_plat.h"
#include "bkni.h"

BDBG_MODULE(bsettop_vxworks);

#define MAX_INT_MSG			64
#define TASK_PRIORITY_HIGH	60
#define INT_MSG_START_ID	1234
#define INT_MSG_END_ID		6789

typedef struct 
{
	int intMsgStartId;
	void *intHandle;
	int	L1Shift;
    int intId;
	int	intMsgEndId;
} INT_MSG;

static struct {
	int      taskId;
	MSG_Q_ID msgQId;
} isrTask;

typedef void *(*isr_t)();

typedef struct
{
    isr_t pfunc;
    int L1Shift;
}isrInfo;

#define MAX_INTERRUPTS     64
isrInfo isr_array[MAX_INTERRUPTS];

static void 
vxInterruptTask(void)
{
	INT_MSG	intMsg;
	int	rc;
    int key = 0;

	for(;;) {

	 	/* Wait for the new ints message to come in */
	   	rc = msgQReceive(isrTask.msgQId, (char *)&intMsg.intMsgStartId, sizeof(INT_MSG), 100 /*WAIT_FOREVER*/);

		if (rc == ERROR || rc != sizeof(INT_MSG)) {
			/* BDBG_ERR(("msgQReceive failed rc=%d", rc)); */
			continue;
		}

        if ((intMsg.intMsgStartId != INT_MSG_START_ID) ||
   			(intMsg.intMsgEndId != INT_MSG_END_ID)) 
        {
			BDBG_ERR(("invalid id: %d %d", intMsg.intMsgStartId, intMsg.intMsgEndId));
			break;
    	}

        if (isr_array[intMsg.intId].pfunc == NULL) 
        {
            BDBG_ERR(("No ISR for id: %d", intMsg.intId));
			break;
        }

        BKNI_EnterCriticalSection();
        key = intLock();
        CPUINT1_Enable(intMsg.intId);
        (*isr_array[intMsg.intId].pfunc)(intMsg.intHandle, intMsg.L1Shift);
        intUnlock(key);
        BKNI_LeaveCriticalSection();
	}

	return;
}

void b_vxworks_Isr( BINT_Handle intHandle, int intId )
{
	INT_MSG intMsg;

    CPUINT1_Disable(intId);

	intMsg.intMsgStartId = INT_MSG_START_ID;
	intMsg.intHandle     = intHandle;
	intMsg.L1Shift       = isr_array[intId].L1Shift; /* or L1Shift */
    intMsg.intId         = intId; /* IntID */  
	intMsg.intMsgEndId   = INT_MSG_END_ID;

    if (msgQSend(isrTask.msgQId, (char *)&intMsg.intMsgStartId, sizeof(INT_MSG), NO_WAIT,
				MSG_PRI_NORMAL) != OK)
		logMsg("b_vxworks_Isr: msg send failed\n", 0,0,0,0,0,0);
}


int b_intmgr_vxInit( void )
{	
	//uint32_t				ebiPhysOffset7411;
	
	/* Get platform-specific EBI info for 7411. Don't really need the EBI info
     * for this purpose, but we do want the interrupt id.
     */
	//Get7411ConnectionInfo(&ebiPhysOffset7411, NULL, &irqInterruptId7411);

	isrTask.msgQId = msgQCreate(MAX_INT_MSG, sizeof(INT_MSG), MSG_Q_FIFO);

	if (isrTask.msgQId == NULL)
	{
		BDBG_ERR(("msgQCreate failed"));
		return -1;
	}
	
	/* Create the task for interrupt handling.
	TASK_PRIORITY_HIGH is 60, which is not high enough. We chose
	10 to still leave a little headroom for other tasks. */
	isrTask.taskId = taskSpawn((char *) "vxInterruptTask", 10,
			0 /* no floating point VX_FP_TASK */, 
			0x2000, (FUNCPTR) vxInterruptTask,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	if ( isrTask.taskId == ERROR )
	{
		BDBG_ERR(("%s: failed, can't create task", __FUNCTION__));
		return -1;
	}
	return 0;
}

void
b_intmgr_vxShutdown( void )
{
	BKNI_EnterCriticalSection();
	taskDelete( isrTask.taskId);
	BKNI_LeaveCriticalSection();
	msgQDelete( isrTask.msgQId);
}

int 
CPUINT1_ConnectIsr_vxWorks(unsigned long intId, void *pfunc, void *param1, int param2)
{
    if (intId >= MAX_INTERRUPTS) 
    {
        BDBG_ERR(("CPUINT1_ConnectIsr_vxWorks: Interrupt Number is greater then %d (intId=%0x, %d)", MAX_INTERRUPTS, intId, intId));
		return -1;
    }
    
    isr_array[intId].pfunc   = (void *)pfunc;
    isr_array[intId].L1Shift = param2;

    /* L1Shift is saved above and intId is passed down */
    CPUINT1_ConnectIsr(intId, (FN_L1_ISR) b_vxworks_Isr, param1, intId);

    return 0;
}

