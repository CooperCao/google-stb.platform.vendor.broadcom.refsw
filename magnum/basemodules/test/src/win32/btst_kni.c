/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "btst_kni.h"

#include <windows.h>

BDBG_MODULE(test_kni);

struct BTST_TaskObj
{
    HANDLE hTask;
    DWORD ThreadId;
    BTST_TaskFunction task_fn;
    void *taskData;
};

static struct {
        LARGE_INTEGER freq;
} state = {
    0
};


void 
BTST_GetTime(BTST_Time *pTime)
{
    LARGE_INTEGER cntr;

    BDBG_ASSERT(pTime);

    if ( !state.freq.QuadPart ) {
        if (!QueryPerformanceFrequency(&state.freq)) {
            BDBG_ERR(("Win32 Performance timer not avaliable"));
            pTime->sec = pTime->usec = 0;
            return;
        }
    }
    QueryPerformanceCounter(&cntr);

    pTime->sec = (uint32_t)(cntr.QuadPart/state.freq.QuadPart);
    pTime->usec = (uint32_t)((cntr.QuadPart*1000000)/state.freq.QuadPart)%1000000;
    return;
}


static DWORD WINAPI
Win32ThreadProc(LPVOID data)
{
    BTST_TaskHandle task = (BTST_TaskHandle)data;  

    task->task_fn(task, task->taskData);

    ExitThread(0);
    
	/* unreached */
	return 0;
}

BERR_Code 
BTST_CreateTask(BTST_TaskHandle *pTask,  BTST_TaskFunction task_fn, void *data)
{
    BTST_TaskHandle task;

    task = (BTST_TaskHandle) malloc(sizeof(*task));
    *pTask = task;
    if (!task) {
        return BERR_TRACE(BERR_OS_ERROR);
    }
    task->taskData = data;
    task->task_fn = task_fn;

    task->hTask = CreateThread(NULL, 0 /* default stack size */, Win32ThreadProc, task, 0 /* no flags */, &task->ThreadId);
    if (!task->hTask) {
        free(task);
        return BERR_TRACE(BERR_OS_ERROR);
    }

    return BERR_SUCCESS;
}


BERR_Code 
BTST_DestroyTask(BTST_TaskHandle task)
{
    DWORD rc;
    BERR_Code err;
    
    rc = WaitForSingleObject(task->hTask, INFINITE);
    if (rc==WAIT_OBJECT_0) {
        err = BERR_SUCCESS;
        if (!GetExitCodeThread(task->hTask, &rc)) {
            err = BERR_TRACE(BERR_OS_ERROR);
        }
    } else {
        err = BERR_TRACE(BERR_OS_ERROR);
    }
    CloseHandle(task->hTask);
    free(task);
    return err;
}
