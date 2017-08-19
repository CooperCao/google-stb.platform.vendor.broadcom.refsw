/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ****************************************************************************/

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSPLOG.h"

#include "fp_sdk_config.h"

#include "DBG_core_debug_server_mp.h"
#include "DBG_core_debug_server_dbp_mp.h"
#include "DBG_interfaces.h"

#if !(FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
#  error "This source is for the debug_server variant of the DBG core"
#endif

#if !FEATURE_IS(LIBC, HOSTED)
#  error "This compilation unit requires support for dynamic memory allocation"
#endif

static void
DBG_core_print_bp_info(dbg_core_bp_info_queue_t *p_bp_info_queue);

/*
 * @brief - Allocate memory for thread info structure and populate known parameters
 *
 * Parameters
 * tid                  - thread id
 * thread_state         - Initial thread state
 * u16_bpt_mask         - Breakpoint mask
 * u8_wp_mask           - Watchpoint mask
 * p_u32_bpt_address    - Buffer holding breakpoint addresses
 * p_u32_wp_address_top/p_u32_wp_address_bot - Buffer holding watchpoint addresses
 *
 * Returns
 * Pointer of type dbg_core_thread_info_t
 * NULL if allocation fails
 * */

dbg_core_thread_info_t *
DBG_core_tid_info_alloc(dbg_tid_t tid,
                        dbg_process_state_t thread_state)
{
    dbg_core_thread_info_t *p_tid_info = NULL;

    DSPLOG_DEBUG("DBG_core_tid_info_alloc(): Allocate dbg_core_tid_info_structure");

    if( (p_tid_info = (dbg_core_thread_info_t *)malloc(sizeof(dbg_core_thread_info_t))) == NULL)
    {
        DSPLOG_ERROR("Allocation of dbg_core_tid_info FAILED");
        return NULL;
    }

    /*Initialize thread info parameters*/
    p_tid_info->tid = tid;
    p_tid_info->thread_state = thread_state;
    p_tid_info->resume_action   = NULL;

    p_tid_info->p_thread_info_next = NULL;

    return p_tid_info;
}

void
DBG_core_tid_info_dealloc(dbg_core_thread_info_t *p_thread_info)
{
    DSPLOG_DEBUG("Dealloc thread info - tid 0x%x", p_thread_info->tid);
    free(p_thread_info);
}

void
DBG_core_set_thread_resume_action(dbg_core_thread_info_t *p_thread_info,
                                  dbg_thread_resume_action_t p_resume_action)
{
    p_thread_info->resume_action = p_resume_action;
}

/*Enqueue a thread info structure to the thread info structure queue*/
void
DBG_core_enqueue_tid_info(dbg_core_thread_info_t *p_thread_info, dbg_core_thread_info_queue_t *p_thread_info_queue)
{
    DSPLOG_DEBUG("Enqueue tid_info 0x%" PRIxPTR " in the queue 0x%" PRIxPTR,
                 (uintptr_t) p_thread_info, (uintptr_t) p_thread_info_queue);
    //If queue is empty
    if(p_thread_info_queue->p_thread_info_head == NULL)
    {
        p_thread_info->p_thread_info_next = NULL;
        p_thread_info_queue->p_thread_info_head = p_thread_info_queue->p_thread_info_tail = p_thread_info;
        return;
    }
    p_thread_info_queue->p_thread_info_tail->p_thread_info_next = p_thread_info;
    p_thread_info_queue->p_thread_info_tail = p_thread_info;
    p_thread_info_queue->p_thread_info_tail->p_thread_info_next = NULL;
}

/*dequeue a thread info structure from the tail of the thread info structure queue*/
dbg_core_thread_info_t *
DBG_core_dequeue_tid_info(dbg_core_thread_info_queue_t *p_thread_info_queue)
{
    DSPLOG_DEBUG("Dequeue pid_info from the queue 0x%" PRIxPTR, (uintptr_t) p_thread_info_queue);
    if(p_thread_info_queue->p_thread_info_head == NULL)
        return NULL;

    dbg_core_thread_info_t *p_thread_info;

    p_thread_info = p_thread_info_queue->p_thread_info_head;
    p_thread_info_queue->p_thread_info_head = p_thread_info_queue->p_thread_info_head->p_thread_info_next;
    return p_thread_info;
}

/*Update the thread state for a thread - if b_update_tid_list is true, update
 * the thread_info_queue if a thread_info corresponding a tid is not found
 * */
bool
DBG_core_thread_update_state(dbg_tid_t tid,
                             dbg_core_thread_info_queue_t *p_thread_info_queue,
                             dbg_process_state_t thread_state,
                             bool b_update_tid_list)
{
    dbg_core_thread_info_t *p_thread_info = DBG_core_find_thread_info(tid, p_thread_info_queue);

    if(p_thread_info == NULL)
    {
        if(b_update_tid_list)
        {
            p_thread_info = DBG_core_tid_info_alloc(tid, thread_state);

            if(p_thread_info == NULL)
            {
                DSPLOG_ERROR("dbp_thread_update_state(): Cannot allocate thread info for tid 0x%x", tid);
                return false;
            }
            DBG_core_enqueue_tid_info(p_thread_info, p_thread_info_queue);
        }
        else
        {
            DSPLOG_ERROR("dbp_thread_update_state(): Unable to locate thread info for tid 0x%x", tid);
            return false;
        }
        return true;
    }

    p_thread_info->thread_state      = thread_state;
    return true;
}

/*
 * Peek into the tid info queue and return the first entry - returns NULL if the queue is empty
 * Leaves the entry in the queue
 * */
dbg_core_thread_info_t *
DBG_core_peek_tid_info(dbg_core_thread_info_queue_t *p_thread_info_queue)
{
    return p_thread_info_queue->p_thread_info_head;
}

/*@brief - Remove a tid_info corresponding to a tid from the thread_info_queue*/
void
DBG_core_remove_tid_info(dbg_tid_t tid, dbg_core_thread_info_queue_t *p_thread_info_queue)
{
    DSPLOG_DEBUG("Remove thread_info corresponding to tid 0x%x", tid);

    dbg_core_thread_info_t *p_prev_thread_info, *p_thread_info, *p_thread_info_head;

    p_thread_info = p_thread_info_head =  DBG_core_peek_tid_info(p_thread_info_queue);

    while(p_thread_info != NULL)
    {
        if(p_thread_info->tid == tid)
        {
            if(p_thread_info == p_thread_info_head)
                p_thread_info_queue->p_thread_info_head = p_thread_info_queue->p_thread_info_head->p_thread_info_next;
            else
                p_prev_thread_info->p_thread_info_next = p_thread_info->p_thread_info_next;

            p_thread_info->p_thread_info_next = NULL;
            DBG_core_tid_info_dealloc(p_thread_info);
            break;
        }
        p_prev_thread_info = p_thread_info;
        GET_NEXT_TID(p_thread_info);
    }
}

/*Allocate memory for a process info structure and populate known parameters*/
dbg_core_pid_info_t *
DBG_core_pid_info_alloc(dbg_pid_t pid, uint8_t u8_core,
                         uint16_t u16_num_threads,
                         dbg_tid_t *p_thread_list,
                         dbg_process_state_t process_state)
{
    dbg_core_pid_info_t *p_pid_info = NULL;

    DSPLOG_DEBUG("DBG_core_pid_info_alloc(): Allocate a dbg_core_pid_info structure");

    if( (p_pid_info = (dbg_core_pid_info_t*) malloc(sizeof(dbg_core_pid_info_t))) == NULL )
    {
        DSPLOG_ERROR("Allocation of dbg_core_pid_info FAILED");
        return NULL;
    }

    /*Initialize other parameters*/
    p_pid_info->pid                 = pid;
    p_pid_info->u8_core             = u8_core;
    p_pid_info->process_state       = process_state;
    p_pid_info->u8_process_attach   = 0;
    p_pid_info->u8_kill_pending     = 0;
    p_pid_info->u8_process_created  = 0;
    p_pid_info->u32_freeze_count    = 0;
    p_pid_info->p_process_name      = NULL;
    p_pid_info->sba                 = 0;

    if(process_state == DBG_PROCESS_STATE_FROZEN || process_state == DBG_PROCESS_STATE_FROZEN_BP)
        p_pid_info->u32_freeze_count++;

    if(p_thread_list != NULL && u16_num_threads != 0)
    {
        uint16_t thread_count;
        /*Allocate and initialize thread-info for each known thread*/
        for(thread_count = 0; thread_count < u16_num_threads; thread_count++)
        {
            /*Allocate and intialize a thread info structure*/
            dbg_core_thread_info_t *p_thread_info = DBG_core_tid_info_alloc(p_thread_list[thread_count],
                                                                            DBG_THREAD_STATE_CREATED);

            if(p_thread_info == NULL)
            {
                DSPLOG_ERROR("Allocation of thread info for tid - 0x%x FAILED", p_thread_list[thread_count]);
                while( (p_thread_info = DBG_core_dequeue_tid_info(&p_pid_info->thread_info_queue)) != NULL)
                    DBG_core_tid_info_dealloc(p_thread_info);

                free(p_pid_info);
                return NULL;
            }

            /*enqueue the thread info in the queue*/
            DBG_core_enqueue_tid_info(p_thread_info, &p_pid_info->thread_info_queue);
        }
    }
    else
    {
        p_pid_info->thread_info_queue.p_thread_info_head = NULL;
        p_pid_info->thread_info_queue.p_thread_info_tail = NULL;
    }

    p_pid_info->process_dso_info.count = PID_DSO_NOT_INITIALIZED ;
    p_pid_info->process_dso_info.p_dso_info = NULL;

    p_pid_info->p_dbg_pending_bp = NULL;
    p_pid_info->bp_info_queue.p_bp_info_head = NULL;
    p_pid_info->bp_info_queue.p_bp_info_tail = NULL;

    p_pid_info->stop_reason_queue.p_stop_reason_head = NULL;
    p_pid_info->stop_reason_queue.p_stop_reason_tail = NULL;

    p_pid_info->p_pid_info_next = NULL;
    return p_pid_info;
}

/*Find the process info structure for a given pid*/
dbg_core_thread_info_t *
DBG_core_find_thread_info(dbg_tid_t tid, dbg_core_thread_info_queue_t *p_thread_info_queue)
{
    DSPLOG_DEBUG("Find threadd_info corresponding to tid 0x%x", tid);

    dbg_core_thread_info_t *p_thread_info = DBG_core_peek_tid_info(p_thread_info_queue);

    while(p_thread_info != NULL)
    {
        if(p_thread_info->tid == tid)
        {
            DSPLOG_DEBUG("thread_info for tid 0x%x found - 0x%" PRIxPTR,
                         tid, (uintptr_t) p_thread_info);
            return p_thread_info;
        }

        GET_NEXT_TID(p_thread_info);
    }

    DSPLOG_DEBUG("Cannot find thread_info for tid - 0x%x", tid);
    return NULL;
}

/*Deallocate a pid info structure
 * The function relies on the fact that the pid_info variable is removed
 * from the queue already*/
void
DBG_core_pid_info_dealloc(dbg_core_pid_info_t *p_pid_info)
{
    dbg_core_thread_info_t *p_thread_info = NULL;

    /*Deallocate thread info memory*/
    while( (p_thread_info = DBG_core_dequeue_tid_info(&p_pid_info->thread_info_queue)) != NULL)
        DBG_core_tid_info_dealloc(p_thread_info);

    if(IS_PROCESS_DSO_INFO_INITIALIZED(p_pid_info))
    {
        int dso_num;

        for(dso_num = 0; dso_num < p_pid_info->process_dso_info.count; dso_num++)
        {
            dso_info_t *p_dso_info = &p_pid_info->process_dso_info.p_dso_info[dso_num];
            free(p_dso_info->dso_name);
            free(p_dso_info->segments);
        }
    }

    if(p_pid_info->process_dso_info.p_dso_info)
        free(p_pid_info->process_dso_info.p_dso_info);

    if(p_pid_info->p_process_name)
        free(p_pid_info->p_process_name);

    free(p_pid_info);
}

/*Enqueue a process info structure to the process info structure queue*/
void
DBG_core_enqueue_pid_info(dbg_core_pid_info_t *p_pid_info, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Enqueue pid_info 0x%" PRIxPTR " in the queue 0x%" PRIxPTR,
                 (uintptr_t) p_pid_info, (uintptr_t) p_pid_info_queue);
    //If queue is empty
    if(p_pid_info_queue->p_pid_info_head == NULL)
    {
        p_pid_info->p_pid_info_next = NULL;
        p_pid_info_queue->p_pid_info_head = p_pid_info_queue->p_pid_info_tail = p_pid_info;
        return;
    }
    p_pid_info_queue->p_pid_info_tail->p_pid_info_next = p_pid_info;
    p_pid_info_queue->p_pid_info_tail = p_pid_info;
    p_pid_info_queue->p_pid_info_tail->p_pid_info_next = NULL;

}

/*dequeue a process info structure from the tail of the process info structure queue*/
dbg_core_pid_info_t *
DBG_core_dequeue_pid_info(dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Dequeue pid_info from the queue 0x%" PRIxPTR, (uintptr_t) p_pid_info_queue);
    if(p_pid_info_queue->p_pid_info_head == NULL)
        return NULL;

    dbg_core_pid_info_t *p_pid_info;

    p_pid_info = p_pid_info_queue->p_pid_info_head;
    p_pid_info_queue->p_pid_info_head = p_pid_info_queue->p_pid_info_head->p_pid_info_next;
    return p_pid_info;
}

/*
 * Peek into the pid info queue and return the first entry - returns NULL if the queue is empty
 * Leaves the entry in the queue
 * */
dbg_core_pid_info_t *
DBG_core_peek_pid_info(dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    return p_pid_info_queue->p_pid_info_head;
}

/*Find the process info structure for a given pid*/
dbg_core_pid_info_t *
DBG_core_find_pid_info(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Find pid_info corresponding to pid 0x%x", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_peek_pid_info(p_pid_info_queue);

    while(p_pid_info != NULL)
    {
        if(p_pid_info->pid == pid)
        {
            DSPLOG_DEBUG("pid_info for pid 0x%x found - 0x%" PRIxPTR, pid, (uintptr_t) p_pid_info);
            return p_pid_info;
        }

        GET_NEXT_PID(p_pid_info);
    }

    DSPLOG_DEBUG("Cannot find pid_info for pid - 0x%x", pid);
    return NULL;
}

/*@brief - Remove a pid_info corresponding to a pid from the pid_info_queue*/
void
DBG_core_remove_pid_info(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Remove pid_info corresponding to pid 0x%x", pid);

    dbg_core_pid_info_t *p_prev_pid_info, *p_pid_info, *p_pid_info_head;

    p_pid_info = p_pid_info_head =  DBG_core_peek_pid_info(p_pid_info_queue);

    while(p_pid_info != NULL)
    {
        if(p_pid_info->pid == pid)
        {
            if(p_pid_info == p_pid_info_head)
                p_pid_info_queue->p_pid_info_head = p_pid_info_queue->p_pid_info_head->p_pid_info_next;
            else
                p_prev_pid_info->p_pid_info_next = p_pid_info->p_pid_info_next;

            p_pid_info->p_pid_info_next = NULL;
            DBG_core_pid_info_dealloc(p_pid_info);
            break;
        }
        p_prev_pid_info = p_pid_info;
        GET_NEXT_PID(p_pid_info);
    }
}

bool
DBG_core_process_set_name(dbg_pid_t pid,
                          dbg_core_pid_info_queue_t *p_pid_info_queue,
                          char *p_process_name)
{
    DSPLOG_DEBUG("Update process name for the process 0x%x", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to the pid - 0x%x", pid);
        return false;
    }

    p_pid_info->p_process_name = (char *)malloc(strlen(p_process_name) + 1);

    if(p_pid_info->p_process_name == NULL)
    {
        DSPLOG_ERROR("Cannot allocate string for storing process name 0x%x", pid);
        return false;
    }

    strcpy(p_pid_info->p_process_name, p_process_name);
    return true;
}

char *
DBG_core_process_get_name(dbg_core_pid_info_t *p_pid_info)
{
    return p_pid_info->p_process_name;
}

bool
DBG_core_process_update_state(dbg_pid_t pid,
                              dbg_core_pid_info_queue_t *p_pid_info_queue,
                              dbg_process_state_t process_state)
{
    DSPLOG_DEBUG("Update process state for process 0x%x", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to the pid - 0x%x", pid);
        return false;
    }

    p_pid_info->process_state       = process_state;

    if(process_state == DBG_PROCESS_STATE_FROZEN || process_state == DBG_PROCESS_STATE_FROZEN_BP)
        p_pid_info->u32_freeze_count++;
    else if(process_state == DBG_PROCESS_STATE_UNFROZEN)
        p_pid_info->u32_freeze_count--;

    return true;
}

dbg_process_state_t
DBG_core_get_process_state(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue )
{
    DSPLOG_DEBUG("Get the process state for pid - 0x%x", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);
    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return DBG_PROCESS_INVALID;
    }
    return p_pid_info->process_state;
}

uint32_t
DBG_core_get_process_freeze_count(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Get the freeze count for pid - 0x%x", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);
    return DBG_core_get_process_freeze_count_1(p_pid_info);
}

uint32_t
DBG_core_get_process_freeze_count_1(dbg_core_pid_info_t *p_pid_info)
{
    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Invalid process info");
        return 0xFFFFFFFF;
    }

    return p_pid_info->u32_freeze_count;
}

bool
DBG_core_attach_process(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Attach process %d to the debugger", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return false;
    }

    DBG_core_attach_process_1(p_pid_info);
    return true;
}

void
DBG_core_attach_process_1(dbg_core_pid_info_t *p_pid_info)
{
    DSPLOG_DEBUG("Attach process %d to the debugger", p_pid_info->pid);

    p_pid_info->u8_process_attach = 1;
}

bool
DBG_core_detach_process(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Detach process %d to the debugger", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return false;
    }

    DBG_core_detach_process_1(p_pid_info);
    return true;
}

void
DBG_core_detach_process_1(dbg_core_pid_info_t *p_pid_info)
{
    DSPLOG_DEBUG("Detach process %d to the debugger", p_pid_info->pid);

    p_pid_info->u8_process_attach = 0;
}

uint8_t
DBG_core_is_process_attached(dbg_core_pid_info_t *p_pid_info)
{
    return p_pid_info->u8_process_attach;
}

void
DBG_core_process_created(dbg_core_pid_info_t *p_pid_info)
{
    p_pid_info->u8_process_created = 1;
}

uint8_t
DBG_core_is_process_created(dbg_core_pid_info_t *p_pid_info)
{
    return p_pid_info->u8_process_created;
}

bool
DBG_core_process_kill_pending(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_INFO("Mark process %d as terminate pending", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return false;
    }
    DBG_core_process_kill_pending_1(p_pid_info);
    return true;
}

void
DBG_core_process_kill_pending_1(dbg_core_pid_info_t *p_pid_info)
{
    p_pid_info->u8_kill_pending = 1;
}

bool
DBG_core_process_set_sba(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, uint32_t sba)
{
    DSPLOG_INFO("Set process %d SBA value 0x%x", pid, sba);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return false;
    }

    p_pid_info->sba = sba;
    return true;
}

uint32_t
DBG_core_process_get_sba(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_INFO("Get SBA value for process %d - returns 0 for invalid pid", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return 0;
    }

    return p_pid_info->sba;
}

bool
DBG_core_process_set_dso_count(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, int count)
{
    DSPLOG_INFO("Update dso count for process - 0x%x count %d", pid, count);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return false;
    }

    p_pid_info->process_dso_info.count = count;

    if(count > 0)
    {
        DSPLOG_INFO("set_dso_count(): Allocate dso_info for p%x count %d", pid, count);
        /*Free any previously allocated memory for storing dso information*/
        if(p_pid_info->process_dso_info.p_dso_info)
            free(p_pid_info->process_dso_info.p_dso_info);

        /*Allocate new memory for storing dso information*/
        p_pid_info->process_dso_info.p_dso_info =  (dso_info_t *)malloc(count * sizeof(dso_info_t));

        if(p_pid_info->process_dso_info.p_dso_info == NULL)
        {
            DSPLOG_ERROR("Cannot allocate dso_info for pid 0x%x", pid);
            return false;
        }

        dso_info_t *p_dso_info = p_pid_info->process_dso_info.p_dso_info;

        int i;
        for(i = 0; i < count; i++)
        {
            p_dso_info[i].dso_name = NULL;
            p_dso_info[i].num_segments = 0;
            p_dso_info[i].segments = NULL;
        }
    }

    return true;
}

int
DBG_core_process_get_dso_count(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Get DSO count for pid - 0x%x", pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return 0;
    }
    return p_pid_info->process_dso_info.count;
}

bool
DBG_core_process_set_dso_soname(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, char *p_dso_name, int dso_index)
{
    DSPLOG_INFO("Set dsoname[%d] %s for pid - 0x%x", dso_index, p_dso_name, pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return false;
    }

    if(dso_index >= p_pid_info->process_dso_info.count)
    {
        DSPLOG_ERROR("dso_index 0x%x >= expected count 0x%x", dso_index, p_pid_info->process_dso_info.count);
        return false;
    }

    dso_info_t *p_dso_info = &p_pid_info->process_dso_info.p_dso_info[dso_index];

    if(p_dso_info->dso_name != NULL)
        free(p_dso_info->dso_name);

    p_dso_info->dso_name = (char *)malloc(strlen(p_dso_name) + 1);

    if(p_dso_info->dso_name == NULL)
    {
        DSPLOG_ERROR("Cannot allocate string buffer to hold dsoname for pid 0x%x dso_index 0x%x", pid, dso_index);
        return false;
    }

    DSPLOG_INFO("set_dso_soname(): Copying %s to dso_info structure", p_dso_name);

    strcpy(p_dso_info->dso_name, p_dso_name);

    return true;
}

char *
DBG_core_process_get_dso_soname(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, int dso_index)
{
    DSPLOG_DEBUG("Set dsoname[%d] for pid - 0x%x", dso_index, pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return NULL;
    }

    if(dso_index >= p_pid_info->process_dso_info.count)
    {
        DSPLOG_ERROR("dso_index 0x%x >= expected count 0x%x", dso_index, p_pid_info->process_dso_info.count);
        return NULL;
    }

    dso_info_t *p_dso_info = &p_pid_info->process_dso_info.p_dso_info[dso_index];
    return p_dso_info->dso_name;
}

bool
DBG_core_process_set_dso_segments(dbg_pid_t pid,
                                  dbg_core_pid_info_queue_t *p_pid_info_queue,
                                  int dso_index,
                                  int num_segments,
                                  uint32_t *segments)
{
    DSPLOG_DEBUG("Set dsosegments[%d] for pid - 0x%x", dso_index, pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        return false;
    }

    if(dso_index >= p_pid_info->process_dso_info.count)
    {
        DSPLOG_ERROR("dso_index 0x%x >= expected count 0x%x", dso_index, p_pid_info->process_dso_info.count);
        return false;
    }

    dso_info_t *p_dso_info = &p_pid_info->process_dso_info.p_dso_info[dso_index];

    if(p_dso_info->segments != NULL)
        free(p_dso_info->segments);

    /* There are two values for each segment, the first is the base address and
     * the second is its size
     */
    p_dso_info->segments = (uint32_t *)malloc(num_segments * 2 * sizeof(uint32_t));
    if(p_dso_info->segments == NULL)
    {
        DSPLOG_ERROR("Cannot allocate segments buffer to hold dsosegments for pid 0x%x dso_index 0x%x", pid, dso_index);
        return false;
    }

    int i;
    for(i = 0; i < num_segments; i++)
    {
        DSPLOG_INFO("library %s segment[%d] - 0x%x", p_dso_info->dso_name, i, segments[2*i]);
        p_dso_info->segments[2*i]   = segments[2*i];   /* Copy base */
        p_dso_info->segments[2*i+1] = segments[2*i+1]; /* Copy size */
    }

    return true;
}

uint32_t *
DBG_core_process_get_dso_segments(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue,
                                  int dso_index, int *num_segments)
{
    DSPLOG_DEBUG("Get dsosegments[%d] for pid 0x%x", dso_index, pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid - 0x%x", pid);
        *num_segments = 0;
        return NULL;
    }

    if(dso_index >= p_pid_info->process_dso_info.count)
    {
        DSPLOG_ERROR("dso_index 0x%x >= expected count 0x%x", dso_index, p_pid_info->process_dso_info.count);
        *num_segments = 0;
        return NULL;
    }
    dso_info_t *p_dso_info = &p_pid_info->process_dso_info.p_dso_info[dso_index];
    *num_segments = p_dso_info->num_segments;
    return p_dso_info->segments;
}

bool
DBG_core_process_set_dso_sba(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue,
                             int dso_index, uint32_t sba)
{
    DSPLOG_DEBUG("Set SBA 0x%x for dso %d in pid 0x%x", sba, dso_index, pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid 0x%x", pid);
        return false;
    }

    if(dso_index >= p_pid_info->process_dso_info.count)
    {
        DSPLOG_ERROR("dso_index 0x%x >= expected count 0x%x", dso_index, p_pid_info->process_dso_info.count);
        return false;
    }

    p_pid_info->process_dso_info.p_dso_info[dso_index].sba = sba;
    return true;
}

uint32_t
DBG_core_process_get_dso_sba(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, int dso_index)
{
    DSPLOG_DEBUG("Get SBA for dso %d in pid 0x%x", dso_index, pid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to pid 0x%x", pid);
        return 0;
    }

    if(dso_index >= p_pid_info->process_dso_info.count)
    {
        DSPLOG_ERROR("dso_index 0x%x >= expected count 0x%x", dso_index, p_pid_info->process_dso_info.count);
        return 0;
    }

    return p_pid_info->process_dso_info.p_dso_info[dso_index].sba;
}

bool
DBG_core_process_update_thread_list(dbg_pid_t pid,
                                    dbg_core_pid_info_queue_t *p_pid_info_queue,
                                    dbg_tid_t *p_thread_list,
                                    uint16_t u16_num_threads)
{
    DSPLOG_INFO("Update thread list for pid - 0x%x num_threads - 0x%x", pid, u16_num_threads);

    dbg_core_thread_info_t *p_thread_info = NULL;
    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);
    int tid_num;

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("Cannot find process info corresponding to the pid - 0x%x", pid);
        return false;
    }
    else
    {
        /*For each thread in the input list that doesn't contain a thread_info -
         * create a thread_info and enqueue in the thread_info_queue*/
        for(tid_num = 0; tid_num < u16_num_threads; tid_num++)
        {
            dbg_core_thread_info_t *p_thread_info = DBG_core_find_thread_info(p_thread_list[tid_num], &p_pid_info->thread_info_queue);

            if(p_thread_info == NULL)
            {
                DSPLOG_INFO("process_update_thread_list(): Allocate tid_info for thread 0x%x", p_thread_list[tid_num]);
                p_thread_info = DBG_core_tid_info_alloc(p_thread_list[tid_num],
                        DBG_THREAD_STATE_CREATED);

                DBG_core_enqueue_tid_info(p_thread_info, &p_pid_info->thread_info_queue);
            }
        }

        /*Prune the pid_info queue by deleting all processes that doesn't exist in the p_dbp_response*/
        p_thread_info = DBG_core_peek_tid_info(&p_pid_info->thread_info_queue);
        while(p_thread_info != NULL)
        {
            for(tid_num = 0; tid_num < u16_num_threads;)
            {
                if(p_thread_info->tid == p_thread_list[tid_num])
                {
                    break;
                }
                else
                    tid_num++;
            }

            dbg_core_thread_info_t *p_prev_thread_info = p_thread_info;
            GET_NEXT_TID(p_thread_info);

            if(tid_num == u16_num_threads)
            {
                DSPLOG_INFO("process_update_thread_list(): Remove tid_info for thread 0x%x", p_prev_thread_info->tid);
                DBG_core_remove_tid_info(p_prev_thread_info->tid, &p_pid_info->thread_info_queue);
            }
        }

    }
    return true;
}

/*Allocate memory for a breakpoint-info and populate information*/
dbg_core_bp_info_t *
DBG_core_bp_info_alloc(dbg_pid_t pid,
                       dbg_tid_t tid,
                       uint8_t  u8_bp_wp_num,
                       DBG_BPT_WPT_TYPE bpt_type,
                       uint32_t u32_addr,
                       uint32_t u32_wp_num_bytes)
{
    DSPLOG_DEBUG("Allocate a breakpoint info");

    dbg_core_bp_info_t *p_bp_info = NULL;

    if( (p_bp_info = (dbg_core_bp_info_t *)malloc(sizeof(dbg_core_bp_info_t))) == NULL)
    {
        DSPLOG_ERROR("Breakpoint info allocation failed");
        return NULL;
    }

    p_bp_info->pid = pid;
    p_bp_info->tid = tid;
    p_bp_info->u8_bp_wp_num = u8_bp_wp_num;
    p_bp_info->bpt_type = bpt_type;
    p_bp_info->u32_addr = u32_addr;
    p_bp_info->u32_wp_num_bytes = u32_wp_num_bytes;

    return p_bp_info;
}

/*Free breakpoint info memory previously allocated*/
void
DBG_core_bp_info_dealloc(dbg_core_bp_info_t *p_bp_info)
{
    DSPLOG_DEBUG("Dealloc breakpoint info - bpnum 0x%x", p_bp_info->u8_bp_wp_num);
    free(p_bp_info);
}

/*Enqueue a breakpoint_info to the queue*/
void
DBG_core_enqueue_bp_info(dbg_core_bp_info_t *p_bp_info, dbg_core_bp_info_queue_t *p_bp_info_queue)
{
    DSPLOG_DEBUG("Enqueue bp_info 0x%" PRIxPTR " in the queue 0x%" PRIxPTR,
                 (uintptr_t) p_bp_info, (uintptr_t) p_bp_info_queue);
    //If queue is empty
    if(p_bp_info_queue->p_bp_info_head == NULL)
    {
        p_bp_info->p_bp_info_next = NULL;
        p_bp_info_queue->p_bp_info_head = p_bp_info_queue->p_bp_info_tail = p_bp_info;
        DBG_core_print_bp_info(p_bp_info_queue);
        return;
    }
    p_bp_info_queue->p_bp_info_tail->p_bp_info_next = p_bp_info;
    p_bp_info_queue->p_bp_info_tail = p_bp_info;
    p_bp_info_queue->p_bp_info_tail->p_bp_info_next = NULL;

    DBG_core_print_bp_info(p_bp_info_queue);
}

/*dequeue a breakpoint_info from the queue*/
dbg_core_bp_info_t *
DBG_core_dequeue_bp_info(dbg_core_bp_info_queue_t *p_bp_info_queue)
{
    DSPLOG_DEBUG("Dequeue bp_info from the queue 0x%" PRIxPTR, (uintptr_t) p_bp_info_queue);
    if(p_bp_info_queue->p_bp_info_head == NULL)
        return NULL;

    dbg_core_bp_info_t *p_bp_info;

    p_bp_info = p_bp_info_queue->p_bp_info_head;
    p_bp_info_queue->p_bp_info_head = p_bp_info_queue->p_bp_info_head->p_bp_info_next;

    DBG_core_print_bp_info(p_bp_info_queue);
    return p_bp_info;
}

dbg_core_bp_info_t *
DBG_core_peek_bp_info(dbg_core_bp_info_queue_t *p_bp_info_queue)
{
    return p_bp_info_queue->p_bp_info_head;
}

/*Find a breakpoint_info for a given breakpoint/watchpoint number and thread-id*/
dbg_core_bp_info_t *
DBG_core_find_bp_info_from_bpnum(uint8_t u8_bp_wp_num,
                                 DBG_BPT_WPT_TYPE bpt_type,
                                 dbg_pid_t pid, dbg_tid_t tid,
                                 dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Find bp_info corresponding to bp 0x%x for p%x.%x", u8_bp_wp_num, pid, tid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("find_bp_info(): Cannot find process p%x", pid);
        return NULL;
    }

    return DBG_core_find_bp_info_from_bpnum_1(u8_bp_wp_num, bpt_type, tid, p_pid_info);
}

dbg_core_bp_info_t *
DBG_core_find_bp_info_from_bpnum_1(uint8_t u8_bp_wp_num,
                                   DBG_BPT_WPT_TYPE bpt_type,
                                   dbg_tid_t tid,
                                   dbg_core_pid_info_t *p_pid_info)
{
    dbg_core_bp_info_queue_t *p_bp_info_queue = &p_pid_info->bp_info_queue;

    dbg_core_bp_info_t *p_bp_info = NULL;

    p_bp_info = DBG_core_peek_bp_info(p_bp_info_queue);

    while(p_bp_info != NULL)
    {
        if( (p_bp_info->tid == tid || p_bp_info->tid == 0xFFFF) &&
           p_bp_info->u8_bp_wp_num == u8_bp_wp_num)
        {
            switch(bpt_type)
            {
                case DBG_BREAKPOINT_TYPE_HW:
                case DBG_BREAKPOINT_TYPE_SW:
                    if(p_bp_info->bpt_type <= DBG_BREAKPOINT_TYPE_HW)
                    {
                        DSPLOG_DEBUG("bp_info for bp 0x%x in thread 0x%x found - 0x%" PRIxPTR,
                                     u8_bp_wp_num, tid, (uintptr_t) p_bp_info);
                        return p_bp_info;
                    }
                    break;

                case DBG_WATCHPOINT_TYPE_WRITE:
                case DBG_WATCHPOINT_TYPE_READ:
                case DBG_WATCHPOINT_TYPE_ACCESS:
                    if(p_bp_info->bpt_type <= DBG_WATCHPOINT_TYPE_ACCESS )
                    {
                        DSPLOG_DEBUG("bp_info for bp 0x%x in thread 0x%x found - 0x%" PRIxPTR,
                                     u8_bp_wp_num, tid, (uintptr_t) p_bp_info);
                        return p_bp_info;
                    }
                    break;
                default:
                    DSPLOG_ERROR("Unknown break-watch type");
                    return NULL;
            }
        }
        GET_NEXT_BP(p_bp_info);
    }

    DSPLOG_DEBUG("Cannot find bp_info for 0x%x in thread %x", u8_bp_wp_num, tid);
    return NULL;
}

dbg_core_bp_info_t *
DBG_core_find_bp_info_from_addr(uint32_t u32_addr,
                                DBG_BPT_WPT_TYPE bpt_type,
                                dbg_pid_t pid, dbg_tid_t tid,
                                dbg_core_pid_info_queue_t *p_pid_info_queue)
{
    DSPLOG_DEBUG("Find bp_info planted at addr 0x%x for thread %x", u32_addr, tid);

    dbg_core_pid_info_t *p_pid_info = DBG_core_find_pid_info(pid, p_pid_info_queue);

    if(p_pid_info == NULL)
    {
        DSPLOG_ERROR("find_bp_info(): Cannot find process p%x", pid);
        return NULL;
    }

    return DBG_core_find_bp_info_from_addr_1(u32_addr, bpt_type, tid, p_pid_info);
}

dbg_core_bp_info_t *
DBG_core_find_bp_info_from_addr_1(uint32_t u32_addr,
                                  DBG_BPT_WPT_TYPE bpt_type,
                                  dbg_tid_t tid,
                                  dbg_core_pid_info_t *p_pid_info)
{
    dbg_core_bp_info_queue_t *p_bp_info_queue = &p_pid_info->bp_info_queue;

    dbg_core_bp_info_t *p_bp_info = NULL;

    p_bp_info = DBG_core_peek_bp_info(p_bp_info_queue);

    if(p_bp_info == NULL)
    {
        DSPLOG_INFO("No breakpoint in the process p%x", p_pid_info->pid);
    }

    while(p_bp_info != NULL)
    {
        if(p_bp_info->tid == tid || p_bp_info->tid == 0xFFFF )
        {
            switch(bpt_type)
            {
                case DBG_BREAKPOINT_TYPE_HW:
                case DBG_BREAKPOINT_TYPE_SW:
                    if(p_bp_info->bpt_type <= DBG_BREAKPOINT_TYPE_HW &&
                       p_bp_info->u32_addr == u32_addr)
                    {
                        DSPLOG_DEBUG("bp_info for bp planted at 0x%x for thread %x found - 0x%" PRIxPTR,
                                     u32_addr, tid, (uintptr_t) p_bp_info);
                        return p_bp_info;
                    }
                    break;

                case DBG_WATCHPOINT_TYPE_WRITE:
                case DBG_WATCHPOINT_TYPE_READ:
                case DBG_WATCHPOINT_TYPE_ACCESS:
                    if(p_bp_info->bpt_type <= DBG_WATCHPOINT_TYPE_ACCESS &&
                       (u32_addr >= p_bp_info->u32_addr &&
                       (u32_addr <= p_bp_info->u32_addr + p_bp_info->u32_wp_num_bytes ))
                    )
                    {
                        DSPLOG_DEBUG("bp_info for bp planted at 0x%x for thread %x found - 0x%" PRIxPTR,
                                     u32_addr, tid, (uintptr_t) p_bp_info);
                        return p_bp_info;
                    }
                    break;

                default:
                    DSPLOG_ERROR("Unknown break-watch type");
                    return NULL;
            }

        }

        GET_NEXT_BP(p_bp_info);
    }

    DSPLOG_INFO("Cannot find bp at addr 0x%x in thread %x", u32_addr, tid);
    return NULL;

}

/*Remove a breakpoint_info from the queue*/
void
DBG_core_remove_bp_info(uint8_t u8_bp_wp_num, DBG_BPT_WPT_TYPE bpt_type, dbg_core_bp_info_queue_t *p_bp_info_queue)
{
    DSPLOG_DEBUG("Remove bp_info corresponding for bp/wp 0x%x", u8_bp_wp_num);

    dbg_core_bp_info_t *p_prev_bp_info, *p_bp_info, *p_bp_info_head;

    p_bp_info = p_bp_info_head =  DBG_core_peek_bp_info(p_bp_info_queue);
    p_prev_bp_info = NULL;

    while(p_bp_info != NULL)
    {
        if(p_bp_info->u8_bp_wp_num == u8_bp_wp_num &&
           p_bp_info->bpt_type == bpt_type)
        {
            DSPLOG_INFO("remove_bp_info(): removing bp 0x%x from p%x.%x",
                         u8_bp_wp_num, p_bp_info->pid, p_bp_info->tid );

            /* First entry in the list - Move the head of the queue to the
             * next entry*/
            if(p_bp_info == p_bp_info_head)
                p_bp_info_queue->p_bp_info_head = p_bp_info_queue->p_bp_info_head->p_bp_info_next;
            else
                p_prev_bp_info->p_bp_info_next = p_bp_info->p_bp_info_next;

            p_bp_info->p_bp_info_next = NULL;
            DBG_core_bp_info_dealloc(p_bp_info);
            break;
        }
        p_prev_bp_info = p_bp_info;
        GET_NEXT_BP(p_bp_info);
    }

    DBG_core_print_bp_info(p_bp_info_queue);
}

static void
DBG_core_print_bp_info(dbg_core_bp_info_queue_t *p_bp_info_queue)
{
    dbg_core_bp_info_t *p_bp_info = DBG_core_peek_bp_info(p_bp_info_queue);

    while(p_bp_info != NULL)
    {
        DSPLOG_INFO("breakpoint 0x%x @addr 0x%x available", p_bp_info->u8_bp_wp_num, p_bp_info->u32_addr);
        GET_NEXT_BP(p_bp_info);
    }
}

/*set breakpoint number*/
void
DBG_core_set_bp_num(uint8_t u8_bp_wp_num, dbg_core_bp_info_t *p_bp_info)
{
    if(p_bp_info != NULL)
        p_bp_info->u8_bp_wp_num = u8_bp_wp_num;
}

/*Get the breakpoint/watchpoint address for a given breakpoint/watchpoint number and thread-id*/
uint32_t
DBG_core_get_bp_addr(uint8_t u8_bp_wp_num, dbg_tid_t tid);

/*Get the number of bytes covered by the watchpoint wpnum for a thread tid*/
uint32_t
DBG_core_get_wp_num_bytes(uint8_t u8_bp_wp_num, dbg_pid_t pid, dbg_tid_t tid);

/*Allocate memory and populate stop_reason*/
dbg_core_stop_reason_t *
DBG_core_stop_reason_alloc(dbg_pid_t pid, dbg_tid_t tid,
                           dbg_stop_reason_t stop_reason,
                           DBG_BPT_WPT_TYPE bpt_type, uint32_t u32_addr)
{
    DSPLOG_DEBUG("Allocate stop_reason");

    dbg_core_stop_reason_t *p_core_stop_reason = NULL;

    if( (p_core_stop_reason = (dbg_core_stop_reason_t *)malloc(sizeof(dbg_core_stop_reason_t))) == NULL)
    {
        DSPLOG_ERROR("Unable to allocate memory for stop_reason for pid 0x%x", pid);
        return NULL;
    }

    p_core_stop_reason->pid = pid;
    p_core_stop_reason->tid = tid;
    p_core_stop_reason->stop_reason = stop_reason;
    p_core_stop_reason->bpt_type = bpt_type;
    p_core_stop_reason->u32_addr = u32_addr;

    return p_core_stop_reason;
}

/*Enqueue stop_reason in stop_reason_queue*/
void
DBG_core_enqueue_stop_reason(dbg_core_stop_reason_t *p_stop_reason, dbg_core_stop_reason_queue_t *p_stop_reason_queue)
{
    DSPLOG_DEBUG("Enqueue stop_reason 0x%" PRIxPTR " in the queue 0x%" PRIxPTR,
                 (uintptr_t) p_stop_reason, (uintptr_t) p_stop_reason_queue);
    //If queue is empty
    if(p_stop_reason_queue->p_stop_reason_head == NULL)
    {
        p_stop_reason->p_stop_reason_next = NULL;
        p_stop_reason_queue->p_stop_reason_head = p_stop_reason_queue->p_stop_reason_tail = p_stop_reason;
        return;
    }
    p_stop_reason_queue->p_stop_reason_tail->p_stop_reason_next = p_stop_reason;
    p_stop_reason_queue->p_stop_reason_tail = p_stop_reason;
    p_stop_reason_queue->p_stop_reason_tail->p_stop_reason_next = NULL;
}

/*dequeue a stop_reason from the tail of the stop_reason queue*/
dbg_core_stop_reason_t *
DBG_core_dequeue_stop_reason(dbg_core_stop_reason_queue_t *p_stop_reason_queue)
{
    DSPLOG_DEBUG("Dequeue stop_reason from the queue 0x%" PRIxPTR, (uintptr_t) p_stop_reason_queue);
    if(p_stop_reason_queue->p_stop_reason_head == NULL)
        return NULL;

    dbg_core_stop_reason_t *p_stop_reason;

    p_stop_reason = p_stop_reason_queue->p_stop_reason_head;
    p_stop_reason_queue->p_stop_reason_head = p_stop_reason_queue->p_stop_reason_head->p_stop_reason_next;
    return p_stop_reason;
}

dbg_core_stop_reason_t *
DBG_core_peek_stop_reason(dbg_core_stop_reason_queue_t *p_stop_reason_queue)
{
    return p_stop_reason_queue->p_stop_reason_head;
}

void
DBG_core_stop_reason_dealloc(dbg_core_stop_reason_t *p_stop_reason)
{
    DSPLOG_DEBUG("Dealloc stop_reason for pid 0x%x", p_stop_reason->pid);
    free(p_stop_reason);
}
