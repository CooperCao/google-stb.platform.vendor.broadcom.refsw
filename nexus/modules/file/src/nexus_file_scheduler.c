/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
#include "nexus_file_module.h"
#include "bfile_io.h"
#include "nexus_file_pvr.h"
#include "nexus_file_muxio.h"

BDBG_MODULE(nexus_file_scheduler);
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

struct NEXUS_File_P_Item {
    BLST_S_ENTRY(NEXUS_File_P_Item) list;
    enum {
      ioType_Read,
      ioType_Write,
      ioType_MuxRead,
      ioType_MuxWrite
    } ioType;
    union {
        struct {
            bfile_io_read_t fd;
        } read;
        struct {
            bfile_io_write_t fd;
        } write;
        struct {
            bfile_io_mux_t fd;
            off_t offset;
        } muxRead;
        struct {
            bfile_io_mux_t fd;
            off_t offset;
        } muxWrite;
    } io;
    void *buf; /* copy of buf */
    size_t length;
    void *cntx;
    NEXUS_File_Callback callback;
    NEXUS_ModuleHandle module;
    ssize_t result;
};

BLST_S_HEAD(NEXUS_File_P_Queue, NEXUS_File_P_Item);

struct NEXUS_File_P_IoWorker {
    NEXUS_ThreadHandle thread;
    unsigned no;
    struct NEXUS_File_P_Item *current;
};

struct NEXUS_File_P_Scheduler {
    NEXUS_FileModuleSettings settings;
    BKNI_EventHandle signal; /* signal to wakeup the thread */
    unsigned kill_count; /* number of task to be killed */
    unsigned workerThreads;
    struct NEXUS_File_P_Queue free, queued, completed;
    struct NEXUS_File_P_Item *items;
    NEXUS_TimerHandle timer;
    struct NEXUS_File_P_IoWorker *workers[NEXUS_FILE_MAX_IOWORKERS];/* worker threads to run */
#define MAX_ZOMBIES 10
    struct NEXUS_File_P_IoWorker *zombies[MAX_ZOMBIES];
};

static struct NEXUS_File_P_Scheduler g_NEXUS_File_P_Scheduler;

#define pScheduler (&g_NEXUS_File_P_Scheduler)

static void NEXUS_P_File_Scheduler_Thread(void *w);

#if 0
static void print_queues(void)
{
    struct NEXUS_File_P_Item *e;
    unsigned count;

    count = 0;
    for (e= BLST_S_FIRST(&pScheduler->free);e; e =BLST_S_NEXT(e, list)) {
        count++;
    }
    BKNI_Printf("free %d, ", count);

    count = 0;
    for (e= BLST_S_FIRST(&pScheduler->queued);e; e =BLST_S_NEXT(e, list)) {
        count++;
    }
    BKNI_Printf("queue %d, ", count);

    count = 0;
    for (e= BLST_S_FIRST(&pScheduler->completed);e; e =BLST_S_NEXT(e, list)) {
        count++;
    }
    BKNI_Printf("completed %d\n", count);
}
#endif

void
NEXUS_FilePlay_Cancel(NEXUS_FilePlayHandle file)
{
    unsigned i;
    for (i=0;i<pScheduler->workerThreads;i++) {
        struct NEXUS_File_P_IoWorker *worker = pScheduler->workers[i];
        if (worker->current && worker->current->ioType == ioType_Read && worker->current->io.read.fd == file->file.data) {
            char name[16];

            BDBG_WRN(("Cancelling File worker %p", (void *)pScheduler->workers[i]));
            /* this item's callback must be called with a failure code in order for other sw state to unwind. */

            /* the new worker gets the item, but the item has failed and should be added to the completed queue now. */
            pScheduler->workers[i] = BKNI_Malloc(sizeof(struct NEXUS_File_P_IoWorker));
            pScheduler->workers[i]->no = worker->no;
            pScheduler->workers[i]->current = NULL; /* don't give the item to the new worker. it will be serviced from NEXUS_P_File_TryCompleted. */
            worker->current->result = -1;
            BLST_S_INSERT_HEAD(&pScheduler->completed, worker->current, list);

            /* the cancelled worker no longer owns the item */
            worker->current = NULL;

            BKNI_Snprintf(name, sizeof(name), "nx_io_worker%u", i);
            pScheduler->workers[i]->thread = NEXUS_Thread_Create(name, NEXUS_P_File_Scheduler_Thread, pScheduler->workers[i], &pScheduler->settings.schedulerSettings[i]);
            if(!pScheduler->workers[i]->thread) {
                BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                /* no recovery */
            }
            /* only one can be active at a time */
            return;
        }
    }
    BDBG_WRN(("NEXUS_FilePlay_Cancel unable to find a pending NEXUS_FilePlayHandle %p. Are you sure I/O is hung on this file?", (void *)file));
}

static void NEXUS_File_P_CleanupZombies(void)
{
    int i;
    for (i=0;i<MAX_ZOMBIES;i++) {
        if (pScheduler->zombies[i]) {
            BDBG_WRN(("Cleaned up zombie %p [%d]", (void *)pScheduler->zombies[i], i));
            NEXUS_Thread_Destroy(pScheduler->zombies[i]->thread);
            BKNI_Free(pScheduler->zombies[i]);
            pScheduler->zombies[i] = NULL;
        }
    }
}

static void NEXUS_File_P_AddZombie(struct NEXUS_File_P_IoWorker *worker)
{
    int i;
    for (i=0;i<MAX_ZOMBIES;i++) {
        if (!pScheduler->zombies[i]) {
            pScheduler->zombies[i] = worker;
            return;
        }
    }
    BDBG_ERR(("Unable to keep zombie. NEXUS_ThreadHandle leak."));
}

void
NEXUS_File_AsyncRead(NEXUS_FileReadHandle fd, void *buf, size_t length, NEXUS_ModuleHandle module, NEXUS_File_Callback callback, void *cntx)
{
    struct NEXUS_File_P_Item *e;
    BDBG_ASSERT(fd);
    BDBG_ASSERT(buf);
    BDBG_ASSERT(module);
    BDBG_ASSERT(callback);

    NEXUS_File_P_CleanupZombies();

    e = BLST_S_FIRST(&pScheduler->free);
    if(e) {
        BLST_S_REMOVE_HEAD(&pScheduler->free, list);
        e->ioType = ioType_Read;
        e->io.read.fd = fd;
        e->buf = buf;
        e->length = length;
        e->cntx = cntx;
        e->module = module;
        e->callback = callback;
        e->result = -1;
        BLST_S_INSERT_HEAD(&pScheduler->queued, e, list);
        BDBG_MSG_TRACE(("rd: queue %#lx", (unsigned long)e));
        BKNI_SetEvent(pScheduler->signal);
        return;
    } else {
        BDBG_WRN(("NEXUS_File_AsyncRead: %#lx %#lx %u not enough descriptors", (unsigned long)cntx, (unsigned long)fd, (unsigned)length));
        callback(cntx, -1);
        return;
    }
}

void
NEXUS_File_AsyncWrite(NEXUS_FileWriteHandle fd, const void *buf, size_t length, NEXUS_ModuleHandle module, NEXUS_File_Callback callback, void *cntx)
{
    struct NEXUS_File_P_Item *e;
    BDBG_ASSERT(fd);
    BDBG_ASSERT(buf);
    BDBG_ASSERT(module);
    BDBG_ASSERT(callback);

    NEXUS_File_P_CleanupZombies();

    e = BLST_S_FIRST(&pScheduler->free);
    if(e) {
        BLST_S_REMOVE_HEAD(&pScheduler->free, list);
        e->ioType = ioType_Write;
        e->io.write.fd = fd;
        e->buf = (void *)buf;
        e->length = length;
        e->cntx = cntx;
        e->module = module;
        e->callback = callback;
        e->result = -1;
        BLST_S_INSERT_HEAD(&pScheduler->queued, e, list);
        BDBG_MSG_TRACE(("wr: queue %#lx", (unsigned long)e));
        BKNI_SetEvent(pScheduler->signal);
        return;
    } else {
        BDBG_WRN(("NEXUS_File_AsyncWrite: %#lx %#lx %u not enough descriptors", (unsigned long)cntx, (unsigned long)fd, (unsigned)length));
        callback(cntx, -1);
        return;
    }
    return;
}

/* calls all completed callbacks */
static void
NEXUS_P_File_TryCompleted(struct NEXUS_File_P_Scheduler *sched)
{
    if(BLST_S_FIRST(&pScheduler->completed)) {
        struct NEXUS_File_P_Item *e;
        struct NEXUS_File_P_Queue completed = sched->completed; /* save copy of completed queue */
        BLST_S_INIT(&sched->completed); /* clear completed queue */
        while( NULL!=(e=BLST_S_FIRST(&completed))) {
            struct NEXUS_File_P_Queue *queue;
            bool locked;

            BLST_S_REMOVE_HEAD(&completed, list);
            NEXUS_UnlockModule();
            locked = NEXUS_Module_TryLock(e->module);
            queue = &sched->completed;
            if(locked) {
                queue = &sched->free;
                /* call callback and add into the free list */
                e->callback(e->cntx, e->result);
                NEXUS_Module_Unlock(e->module);
            }
            NEXUS_LockModule();
            BLST_S_INSERT_HEAD(queue, e, list);
        }
    }
    return;
}

static void
NEXUS_P_File_CallCompleted(void *s)
{
    struct NEXUS_File_P_Scheduler *sched = s;
    BDBG_MSG_TRACE(("NEXUS_P_File_CallCompleted: %#lx", (unsigned long)s));
    sched->timer = NULL;
    NEXUS_P_File_TryCompleted(sched);
    if(BLST_S_FIRST(&pScheduler->completed)) {
        sched->timer = NEXUS_ScheduleTimer(10, NEXUS_P_File_CallCompleted, sched);
    }
    return;
}


static NEXUS_Error
NEXUS_P_File_FindAndDeque(struct NEXUS_File_P_Scheduler *sched, struct NEXUS_File_P_IoWorker *worker)
{
    int best_priority = 0;
    int priority = 0;
    struct NEXUS_File_P_Item *e= BLST_S_FIRST(&sched->queued);
    struct NEXUS_File_P_Item *active = e;
    struct NEXUS_File_P_Item *prev_active = NULL; /* keep prev around for easier delete */
    struct NEXUS_File_P_Item *prev = NULL;
    ssize_t size = 0;
    struct NEXUS_File_P_Queue *queue;
    bool locked;

    BSTD_UNUSED(worker);
    if(!e) {
        NEXUS_UnlockModule(); /* drop lock */
        BDBG_MSG(("%u: queue is empty", worker->no));
        /* coverity[check_return] */
        BKNI_WaitForEvent(sched->signal, 100); /* wait for 100 ms */
        NEXUS_LockModule(); /* restore lock */
        return 0;
    }
    BDBG_MSG_TRACE(("%u: ready %#lx", worker->no, (unsigned long)e));
    do {
        switch(e->ioType) {
        case ioType_Write:
            BDBG_ASSERT(e->io.write.fd);
            priority = e->io.write.fd->priority.get(e->io.write.fd->priority.cntx);
            BDBG_MSG_TRACE(("file[wr] %p priority %d", (void *)e->io.write.fd, priority));
            break;
        case ioType_Read:
            BDBG_ASSERT(e->io.read.fd);
            priority = e->io.read.fd->priority.get(e->io.read.fd->priority.cntx);
            BDBG_MSG_TRACE(("file[rd] %p priority %d", (void *)e->io.write.fd, priority));
            break;
        case ioType_MuxWrite:
            BDBG_ASSERT(e->io.read.fd);
            priority = e->io.muxWrite.fd->priority.get(e->io.muxWrite.fd->priority.cntx);
            BDBG_MSG_TRACE(("file[muxWr] %p priority %d", (void *)e->io.muxWrite.fd, priority));
            break;
        case ioType_MuxRead:
            BDBG_ASSERT(e->io.read.fd);
            priority = e->io.muxRead.fd->priority.get(e->io.muxRead.fd->priority.cntx);
            BDBG_MSG_TRACE(("file[muxRd] %p priority %d", (void *)e->io.muxRead.fd, priority));
            break;
        default:
            BDBG_ASSERT(0);
            break;
        }
        if(best_priority<=priority) { /* since we have LIFO, compensate for it, by giving more priority to last items */
            active = e;
            prev_active = prev;
            best_priority = priority;
        }
        prev = e;
    } while(NULL!=(e = BLST_S_NEXT(e, list)));
    BDBG_ASSERT(active);
    /* remove from the queued list */
    e=active;
    worker->current = e;
    BDBG_MSG_TRACE(("%u: dequeue %#lx", worker->no, (unsigned long)e));
    if(prev_active) {
        BLST_S_REMOVE_NEXT(&sched->queued, prev_active, list);
    } else {
        BLST_S_REMOVE_HEAD(&sched->queued, list);
    }
    NEXUS_UnlockModule(); /* drop lock */
    switch(e->ioType) {
    case ioType_Write:
        {
        size_t write_size = e->length;
        BDBG_ASSERT(e->io.write.fd);
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:wr]:+ %p %p %u %p %p", worker->no, (void *)e->io.write.fd, e->buf, (unsigned)e->length, (void *)(unsigned long)e->callback, e->cntx));
#if 0
        if(e->length/2>4096) {
            write_size = e->length/2 ;
            write_size -= write_size%4096;
        }
#endif
        size = e->io.write.fd->write(e->io.write.fd, e->buf, write_size);
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:wr]:- %p %p %u->%d %p %p", worker->no, (void *)e->io.write.fd, e->buf, (unsigned)e->length, (int)size, (void *)(unsigned long)e->callback, e->cntx));
        break;
        }
    case ioType_Read:
        BDBG_ASSERT(e->io.read.fd);
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:rd]:+ %p %p %u %p %p", worker->no, (void *)e->io.read.fd, e->buf, (unsigned)e->length, (void *)(unsigned long)e->callback, e->cntx));
        size = e->io.read.fd->read(e->io.read.fd, e->buf, e->length);

#if 0
/* PR 47854 - Test code to simulate an I/O error. Playback will cancel the thread and File should recover.
To give a realistic test, we must use random time. The max hang time must be sometimes greater than the
NEXUS_Playback_P_CheckWaitingIo timeout. */
{
    static int count = 0;
    static int next_hang = 100;
    if (++count % next_hang == 0) {
        unsigned hang_time = rand() % 8000;
        BDBG_WRN(("begin induced %d hang in worker %p", hang_time, worker));
        BKNI_Sleep(hang_time);
        BDBG_WRN(("end induced hang in worker %p", worker));
        next_hang = (rand() % 200) + 1;
    }
}
#endif
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:rd]:- %p %p %u->%d %p %p", worker->no, (void *)e->io.read.fd, e->buf, (unsigned)e->length, (int)size, (void *)(unsigned long)e->callback, e->cntx));
        break;
    case ioType_MuxWrite:
        BDBG_ASSERT(e->io.muxWrite.fd);
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:muxWr]:+ %p %p %lu:%u %p %p", worker->no, (void *)e->io.muxWrite.fd, e->buf, (unsigned long)e->io.muxWrite.offset, (unsigned)e->length, (void *)(unsigned long)e->callback, e->cntx));
        size = e->io.muxWrite.fd->write(e->io.muxWrite.fd, e->io.muxWrite.offset, (const NEXUS_FileMuxIoWriteAtom *)e->buf, e->length);
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:muxWr]:- %p %p %lu:%u->%d %p %p", worker->no, (void *)e->io.muxWrite.fd, e->buf, (unsigned long)e->io.muxWrite.offset, (unsigned)e->length, (int)size, (void *)(unsigned long)e->callback, e->cntx));
        break;
    case ioType_MuxRead:
        BDBG_ASSERT(e->io.muxRead.fd);
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:muxRd]:+ %p %p %lu:%u %p %p", worker->no, (void *)e->io.muxRead.fd, e->buf, (unsigned long)e->io.muxRead.offset, (unsigned)e->length, (void *)(unsigned long)e->callback, e->cntx));
        size = e->io.muxWrite.fd->read(e->io.muxRead.fd, e->io.muxRead.offset, (const NEXUS_FileMuxIoReadAtom *)e->buf, e->length);
        BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque[%u:muxRd]:- %p %p %lu:%u->%d %p %p", worker->no, (void *)e->io.muxRead.fd, e->buf, (unsigned long)e->io.muxRead.offset, (unsigned)e->length, (int)size, (void *)(unsigned long)e->callback, e->cntx));
        break;
    default:
        BDBG_ASSERT(0);
        break;
    }

    /* Do not dereference e until verifying worker->current. It may not be owned by this thread any more. */
    if (!worker->current) {
        NEXUS_LockModule();
        /* this worker was cancelled */
        BDBG_WRN(("Cancelled io completed. Worker %p is now a zombie.", (void *)worker));
        NEXUS_File_P_AddZombie(worker);
        return -1; /* this will cause the thread's loop to terminate */
    }

    e->result = size;
    locked = NEXUS_Module_TryLock(e->module);
    BDBG_MSG_TRACE(("NEXUS_P_File_FindAndDeque: %#lx:%#lx %slocked", (unsigned long)sched, (unsigned long)e, locked?"":"not"));

    queue = &sched->completed;
    if(locked) {
        worker->current = NULL;
        queue = &sched->free;
        /* call callback and add into the free list */
        e->callback(e->cntx, size);
        NEXUS_Module_Unlock(e->module);
    }
    NEXUS_LockModule();
    worker->current = NULL;
    BLST_S_INSERT_HEAD(queue, e, list);
    if(queue==&sched->completed && sched->timer==NULL) {
        sched->timer = NEXUS_ScheduleTimer(10, NEXUS_P_File_CallCompleted, sched);
    }

    return 0;
}

static void
NEXUS_P_File_Scheduler_Thread(void *w)
{
    struct NEXUS_File_P_IoWorker *worker=w;
    struct NEXUS_File_P_Scheduler *sched=pScheduler;

    BDBG_MSG(("NEXUS_P_File_Scheduler_Thread: %p started", (void *)worker));
    NEXUS_LockModule();
    for(;;) {
        if(sched->kill_count>0) {
            sched->kill_count--;
            break;
        }
        NEXUS_P_File_TryCompleted(sched);
        if (NEXUS_P_File_FindAndDeque(sched, worker)) {
            goto zombie;
        }
    }
    NEXUS_UnlockModule();
    BDBG_MSG(("NEXUS_P_File_Scheduler_Thread: %p stopped", (void *)worker));
    return;

zombie:
    NEXUS_UnlockModule();
    BDBG_MSG(("NEXUS_P_File_Scheduler_Thread zombie %p stopped", (void *)worker));
    return;
}

NEXUS_Error
NEXUS_File_P_Scheduler_Start(const NEXUS_FileModuleSettings *settings)
{
    unsigned i;
    BERR_Code rc;

    BDBG_ASSERT(settings);
    if(settings->workerThreads >= NEXUS_FILE_MAX_IOWORKERS || settings->workerThreads<1) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }

    pScheduler->workerThreads = settings->workerThreads;
    pScheduler->settings = *settings;
    pScheduler->kill_count = 0;
    pScheduler->timer = NULL;
    BLST_S_INIT(&pScheduler->free);
    BLST_S_INIT(&pScheduler->queued);
    BLST_S_INIT(&pScheduler->completed);
    BKNI_Memset(pScheduler->zombies, 0, sizeof(pScheduler->zombies));
    pScheduler->items = BKNI_Malloc(sizeof(*pScheduler->items)*settings->maxQueuedElements);
    if(!pScheduler->items) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}
    for(i=settings->maxQueuedElements;i>0;i--) { /* insert elements in the reverse order */
        BLST_S_INSERT_HEAD(&pScheduler->free, &pScheduler->items[i-1], list);
    }
    rc = BKNI_CreateEvent(&pScheduler->signal);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_event;}
    for(i=0;i<pScheduler->workerThreads;i++) {
        pScheduler->workers[i] = BKNI_Malloc(sizeof(struct NEXUS_File_P_IoWorker));
        pScheduler->workers[i]->no = i;
        pScheduler->workers[i]->thread = NULL;
        pScheduler->workers[i]->current = NULL;
    }
    for(i=0;i<pScheduler->workerThreads;i++) {
        char name[16];
        BKNI_Snprintf(name, sizeof(name), "nx_io_worker%u", i);
        pScheduler->workers[i]->thread = NEXUS_Thread_Create(name, NEXUS_P_File_Scheduler_Thread, pScheduler->workers[i], &settings->schedulerSettings[i]);
        if(!pScheduler->workers[i]->thread) {
            rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            /* there is no way to stop single thread, so do generic stop */
            NEXUS_File_P_Scheduler_Stop();
            goto error;
        }
    }
    return BERR_SUCCESS;

err_event:
    BKNI_Free(pScheduler->items);
err_alloc:
error:
    return rc;
}

void
NEXUS_File_P_Scheduler_Stop(void)
{
    unsigned i;

    pScheduler->kill_count = 0;
    /* we need to account for the case when not all threads were initialized */
    for(i=0;i<pScheduler->workerThreads;i++) {
        if(!pScheduler->workers[i]->thread) {
            break;
        }
        pScheduler->kill_count++;
    }
    for(;;) {
        unsigned count = pScheduler->kill_count;
        if(count==0) {
            break;
        }
        NEXUS_UnlockModule();
        for(i=0;i<count;i++) {
            BDBG_MSG(("NEXUS_File_P_Scheduler_Stop: waking thread %u out of %u", i, count));
            BKNI_SetEvent(pScheduler->signal);
            BKNI_Sleep(10); /* sleep 10 ms to let worker wake-up */
        }
        NEXUS_LockModule();
    }
    for(i=0;i<pScheduler->workerThreads;i++) {
        if(!pScheduler->workers[i]->thread) {
            break;
        }
        NEXUS_Thread_Destroy(pScheduler->workers[i]->thread);
        BKNI_Free(pScheduler->workers[i]);
    }
    NEXUS_File_P_CleanupZombies();
    if(pScheduler->timer) {
        NEXUS_CancelTimer(pScheduler->timer);
    }
    BKNI_DestroyEvent(pScheduler->signal);
    BKNI_Free(pScheduler->items);
    return;
}

void
NEXUS_File_AsyncMuxWrite(NEXUS_MuxFileIoHandle fd, off_t offset, const NEXUS_FileMuxIoWriteAtom *atoms, size_t atom_count, NEXUS_ModuleHandle module, NEXUS_File_Callback callback, void *cntx)
{
    struct NEXUS_File_P_Item *e;
    BDBG_ASSERT(fd);
    BDBG_ASSERT(atoms);
    BDBG_ASSERT(atom_count>0);
    BDBG_ASSERT(module);
    BDBG_ASSERT(callback);

    NEXUS_File_P_CleanupZombies();

    e = BLST_S_FIRST(&pScheduler->free);
    if(e) {
        BLST_S_REMOVE_HEAD(&pScheduler->free, list);
        e->ioType = ioType_MuxWrite;
        e->io.muxWrite.fd = fd;
        e->io.muxWrite.offset = offset;
        e->buf = (void *)atoms;
        e->length = atom_count;
        e->cntx = cntx;
        e->module = module;
        e->callback = callback;
        e->result = -1;
        BLST_S_INSERT_HEAD(&pScheduler->queued, e, list);
        BDBG_MSG_TRACE(("muxWr: queue %#lx", (unsigned long)e));
        BKNI_SetEvent(pScheduler->signal);
        return;
    } else {
        BDBG_WRN(("NEXUS_File_AsyncMuxWrite: %#lx %#lx %u not enough descriptors", (unsigned long)cntx, (unsigned long)fd, (unsigned)atom_count));
        callback(cntx, -1);
        return;
    }
    return;
}

void
NEXUS_File_AsyncMuxRead(NEXUS_MuxFileIoHandle fd, off_t offset, const NEXUS_FileMuxIoReadAtom *atoms, size_t atom_count, NEXUS_ModuleHandle module, NEXUS_File_Callback callback, void *cntx)
{
    struct NEXUS_File_P_Item *e;
    BDBG_ASSERT(fd);
    BDBG_ASSERT(atoms);
    BDBG_ASSERT(atom_count>0);
    BDBG_ASSERT(module);
    BDBG_ASSERT(callback);

    NEXUS_File_P_CleanupZombies();

    e = BLST_S_FIRST(&pScheduler->free);
    if(e) {
        BLST_S_REMOVE_HEAD(&pScheduler->free, list);
        e->ioType = ioType_MuxRead;
        e->io.muxRead.fd = fd;
        e->io.muxRead.offset = offset;
        e->buf = (void *)atoms;
        e->length = atom_count;
        e->cntx = cntx;
        e->module = module;
        e->callback = callback;
        e->result = -1;
        BLST_S_INSERT_HEAD(&pScheduler->queued, e, list);
        BDBG_MSG_TRACE(("muxRd: queue %#lx", (unsigned long)e));
        BKNI_SetEvent(pScheduler->signal);
        return;
    } else {
        BDBG_WRN(("NEXUS_File_AsyncMuxRead: %#lx %#lx %u not enough descriptors", (unsigned long)cntx, (unsigned long)fd, (unsigned)atom_count));
        callback(cntx, -1);
        return;
    }
    return;
}

