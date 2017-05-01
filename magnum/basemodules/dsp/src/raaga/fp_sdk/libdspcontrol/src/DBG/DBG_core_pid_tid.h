/****************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* Internal header file relating to DEBUG SERVER (Multi process) core component */
#ifndef _DBG_CORE_PID_TID_H_
#define _DBG_CORE_PID_TID_H_

#include "DBG_core_debug_server_common.h"

#define GET_NEXT_PID(PID_INFO)  PID_INFO = (PID_INFO)->p_pid_info_next
#define GET_NEXT_TID(TID_INFO)  TID_INFO = (TID_INFO)->p_thread_info_next
#define GET_NEXT_BP(BP_INFO)    BP_INFO = (BP_INFO)->p_bp_info_next
#define IS_PROCESS_FROZEN(PID_INFO) ((PID_INFO)->process_state == DBG_PROCESS_STATE_FROZEN)
#define IS_THREAD_FROZEN(THREAD_INFO) ((THREAD_INFO)->process_state == DBG_THREAD_STATE_FROZEN)
#define IS_PROCESS_QUEUE_EMPTY(PID_INFO_QUEUE) ((PID_INFO_QUEUE)->p_pid_info_head == NULL)
#define IS_THREAD_QUEUE_EMPTY(THREAD_INFO_QUEUE) ((THREAD_INFO_QUEUE)->p_thread_info_head == NULL)

#define PID_DSO_NOT_INITIALIZED (-1)
#define IS_PROCESS_DSO_INFO_INITIALIZED(PID_INFO)  (((PID_INFO)->process_dso_info.count) != PID_DSO_NOT_INITIALIZED)

typedef uint16_t dbg_pid_t ;
typedef uint16_t dbg_tid_t ;

typedef void (*dbg_thread_resume_action_t)(DBG_core_debug_server *p_dbg_server, dbg_pid_t pid, dbg_tid_t tid);

typedef enum
{
    DBG_PROCESS_STATE_CREATED,
    DBG_PROCESS_STATE_FROZEN,       /*A process frozen by the host*/
    DBG_PROCESS_STATE_FROZEN_BP,    /*A process frozen because of BP/WP/SS*/
    DBG_PROCESS_STATE_UNFROZEN,     /*A process can be unfrozen only by the host*/
    DBG_PROCESS_INVALID,            /*Equivalent to PROCESS_KILLED*/
    DBG_THREAD_STATE_CREATED,
    DBG_THREAD_STATE_FROZEN,        /*A thread frozen by the host*/
    DBG_THREAD_STATE_FROZEN_BP,     /*A thread frozen by BP/WP/SS*/
    DBG_THREAD_STATE_UNFROZEN,      /*A thread can be unfrozen only by the host*/
    DBG_THREAD_INVALID,         /*Equivalent to THREAD_KILLED*/
} dbg_process_state_t;

typedef struct dbg_core_thread_info_t
{
    dbg_tid_t                       tid;                /*Thread-id*/
    dbg_process_state_t             thread_state;       /*State of the thread*/
    dbg_thread_resume_action_t      resume_action;      /*Thread specific resume action
                                                          populated by the vCont handler*/
    struct dbg_core_thread_info_t   *p_thread_info_next; /*Pointer to the next thread*/
} dbg_core_thread_info_t;

typedef struct
{
    dbg_core_thread_info_t  *p_thread_info_head;
    dbg_core_thread_info_t  *p_thread_info_tail;
} dbg_core_thread_info_queue_t;

typedef struct
{
    char *dso_name;
    int   num_segments;
    uint32_t *segments;
} dso_info_t;

typedef struct
{
    int count;                          /*Number of dso associated with the pid*/
    dso_info_t *p_dso_info;              /*Pointer to the dso info structure*/
} dbg_process_dso_info_t;

#define DBG_BP_NUM_PENDING  0xFF

typedef struct dbg_core_bp_info_t
{
    dbg_pid_t   pid;
    dbg_tid_t   tid;
    uint8_t     u8_bp_wp_num;   /*Actual breakpoint/watchpoint number set by the target
                                  DBG_BP_NUM_PENDING - is a special number to indicate
                                  breakpoint is pending
                                */
    DBG_BPT_WPT_TYPE bpt_type;  /*Breakpoint/Watchpoint type*/
    uint32_t u32_addr;          /*Breakpoint/Watchpoint address*/
    uint32_t u32_wp_num_bytes;  /*Number of bytes to watch*/
    struct dbg_core_bp_info_t *p_bp_info_next;
}dbg_core_bp_info_t;

typedef struct
{
    dbg_core_bp_info_t *p_bp_info_head;
    dbg_core_bp_info_t *p_bp_info_tail;
} dbg_core_bp_info_queue_t;

typedef struct dbg_core_pid_info_t
{
    dbg_pid_t                   pid;                /*Process id*/
    uint8_t                     u8_core;            /*core in which the process currently exists*/
    uint8_t                     u8_process_attach;  /*Indicates if the process is attached to the debugger*/
    uint8_t                     u8_kill_pending;    /*Indicates if a process kill request is pending*/
    uint8_t                     u8_process_created; /*Indicates the process is created by the debugger*/
    dbg_process_state_t         process_state;      /*State of the process - we are on all-stop mode
                                                    The state of the process reflects the state of
                                                    all the  threads */
    dbg_process_dso_info_t      process_dso_info;    /*Information pertaining to dynamic shared objects
                                                      for the process */
    dbg_core_thread_info_queue_t thread_info_queue;  /*Queue of thread info*/

    /*Breakpoint / Watchpoint queues*/
    dbg_core_bp_info_queue_t         bp_info_queue;
    dbg_core_bp_info_t               *p_dbg_pending_bp;

    struct dbg_core_pid_info_t *p_pid_info_next;    /*Pointer to the next PID info structure*/
}dbg_core_pid_info_t;

typedef struct
{
    dbg_core_pid_info_t *p_pid_info_head;
    dbg_core_pid_info_t *p_pid_info_tail;
}dbg_core_pid_info_queue_t;

/*Allocate memory for a process info structure and populate known parameters*/
dbg_core_pid_info_t*
DBG_core_pid_info_alloc(dbg_pid_t pid, uint8_t u8_core,
                         uint16_t u16_num_threads,
                         dbg_tid_t *p_thread_list,
                         dbg_process_state_t process_state);

dbg_core_thread_info_t *
DBG_core_tid_info_alloc(dbg_tid_t tid,
                        dbg_process_state_t thread_state);

/*Deallocate a pid info structure
 * The function relies on the fact that the pid_info variable is removed
 * from the queue already*/
void
DBG_core_pid_info_dealloc(dbg_core_pid_info_t *p_pid_info);

void
DBG_core_tid_info_dealloc(dbg_core_thread_info_t *p_thread_info);

/*Enqueue a process info structure to the process info structure queue*/
void
DBG_core_enqueue_pid_info(dbg_core_pid_info_t *p_pid_info, dbg_core_pid_info_queue_t *p_pid_info_queue);

/*Enqueue a thread info structure to the thread info structure queue*/
void
DBG_core_enqueue_tid_info(dbg_core_thread_info_t *p_thread_info, dbg_core_thread_info_queue_t *p_thread_info_queue);

/*dequeue a process info structure from the tail of the process info structure queue*/
dbg_core_pid_info_t *
DBG_core_dequeue_pid_info(dbg_core_pid_info_queue_t *p_pid_info_queue);

/*dequeue a thread info structure from the tail of the thread info structure queue*/
dbg_core_thread_info_t *
DBG_core_dequeue_tid_info(dbg_core_thread_info_queue_t *p_thread_info_queue);

/*Update the thread state for a thread - if b_update_tid_list is true, update
 * the thread_info_queue if a thread_info corresponding a tid is not found
 * */
bool
DBG_core_thread_update_state(dbg_tid_t tid,
                             dbg_core_thread_info_queue_t *p_thread_info_queue,
                             dbg_process_state_t thread_state,
                             bool b_update_tid_list);

/*
 * Peek into the tid info queue and return the first entry - returns NULL if the queue is empty
 * Leaves the entry in the queue
 * */
dbg_core_thread_info_t *
DBG_core_peek_tid_info(dbg_core_thread_info_queue_t *p_thread_info_queue);

/*Find the process info structure for a given pid*/
dbg_core_thread_info_t *
DBG_core_find_thread_info(dbg_tid_t tid, dbg_core_thread_info_queue_t *p_thread_info_queue);

/*@brief - Remove a tid_info corresponding to a tid from the thread_info_queue*/
void
DBG_core_remove_tid_info(dbg_tid_t tid, dbg_core_thread_info_queue_t *p_thread_info_queue);

/*@brief - Set thread specific resume action*/
void
DBG_core_set_thread_resume_action(dbg_core_thread_info_t *p_thread_info,
                                  dbg_thread_resume_action_t p_resume_action);

/*
 * Peek into the pid info queue and return the first entry - returns NULL if the queue is empty
 * Leaves the entry in the queue
 * */
dbg_core_pid_info_t *
DBG_core_peek_pid_info(dbg_core_pid_info_queue_t *p_pid_info_queue);

/*Find the process info structure for a given pid*/
dbg_core_pid_info_t *
DBG_core_find_pid_info(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue);

/*Update state of the process*/
bool
DBG_core_process_update_state(dbg_pid_t pid,
                              dbg_core_pid_info_queue_t *p_pid_info_queue,
                              dbg_process_state_t process_state);

/*Get the state of a process*/
dbg_process_state_t
DBG_core_get_process_state(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue);

/*Update the thread list for a process*/
bool
DBG_core_process_update_thread_list(dbg_pid_t pid,
                                    dbg_core_pid_info_queue_t *p_pid_info_queue,
                                    dbg_tid_t *p_thread_list,
                                    uint16_t u16_num_threads);

/*@brief - Remove a pid_info corresponding to a pid from the pid_info_queue*/
void
DBG_core_remove_pid_info(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue);

/*@brief - Mark process as attached to the debugger*/
bool
DBG_core_attach_process(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue);

void
DBG_core_attach_process_1(dbg_core_pid_info_t *p_pid_info);

/*@brief - Mark process as detached to the debugger*/
bool
DBG_core_detach_process(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue);

void
DBG_core_detach_process_1(dbg_core_pid_info_t *p_pid_info);

/*@brief - Check if the process is attached*/
uint8_t
DBG_core_is_process_attached(dbg_core_pid_info_t *p_pid_info);

/*@brief - Mark the process as created by the debugger*/
void
DBG_core_process_created(dbg_core_pid_info_t *p_pid_info);

/*@brief - Check if the process is created by the debugger*/
uint8_t
DBG_core_is_process_created(dbg_core_pid_info_t *p_pid_info);

/*@brief - Mark as the process termination request is pending in the target*/
bool
DBG_core_process_kill_pending(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue);

void
DBG_core_process_kill_pending_1(dbg_core_pid_info_t *p_pid_info);

/*@brief - Set number of dynamic shared objects for the pid*/
bool
DBG_core_process_set_dso_count(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, int dso_count);

/*@brief - Get number of dsos for the pid*/
int
DBG_core_process_get_dso_count(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue);

/*@brief - Set a dso name for a shared object in the pid*/
bool
DBG_core_process_set_dso_soname(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, char *p_dso_name, int dso_index);

/*@brief - Get the name of a dso in the pid*/
char *
DBG_core_process_get_dso_soname(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue, int dso_index);

/*@brief - Set the segments for the dso in the pid*/
bool
DBG_core_process_set_dso_segments(dbg_pid_t pid,
                                  dbg_core_pid_info_queue_t *p_pid_info_queue,
                                  int dso_index,
                                  int num_segments,
                                  uint32_t *segments);


/*Get dso segments for the process-info and dso_index*/
uint32_t *
DBG_core_process_get_dso_segments(dbg_pid_t pid, dbg_core_pid_info_queue_t *p_pid_info_queue,
                                  int dso_index, int *num_segments);


/*Allocate memory for a breakpoint-info and populate information*/
dbg_core_bp_info_t *
DBG_core_bp_info_alloc(dbg_pid_t pid,
                       dbg_tid_t tid,
                       uint8_t  u8_bp_wp_num,
                       DBG_BPT_WPT_TYPE bpt_type,
                       uint32_t u32_addr,
                       uint32_t u32_wp_num_bytes);

/*Free breakpoint info memory previously allocated*/
void
DBG_core_bp_info_dealloc(dbg_core_bp_info_t *p_bp_info);

/*Enqueue a breakpoint_info to the queue*/
void
DBG_core_enqueue_bp_info(dbg_core_bp_info_t *p_bp_info, dbg_core_bp_info_queue_t *p_bp_info_queue);

void
DBG_core_set_bpt_type(DBG_BPT_WPT_TYPE bpt_type, dbg_core_bp_info_t *p_bp_info);

void
DBG_core_set_bp_num(uint8_t u8_bp_wp_num, dbg_core_bp_info_t *p_bp_info);

/*dequeue a breakpoint_info from the queue*/
dbg_core_bp_info_t *
DBG_core_dequeue_bp_info(dbg_core_bp_info_queue_t *p_bp_info_queue);

dbg_core_bp_info_t *
DBG_core_peek_bp_info(dbg_core_bp_info_queue_t *p_bp_info_queue);

/*Find a breakpoint_info for a given breakpoint/watchpoint number and thread-id*/
dbg_core_bp_info_t *
DBG_core_find_bp_info_from_bpnum(uint8_t u8_bp_wp_num,
                                 DBG_BPT_WPT_TYPE bpt_type,
                                 dbg_pid_t pid, dbg_tid_t tid,
                                 dbg_core_pid_info_queue_t *p_pid_info_queue);

dbg_core_bp_info_t *
DBG_core_find_bp_info_from_bpnum_1(uint8_t u8_bp_wp_num,
                                   DBG_BPT_WPT_TYPE bpt_type,
                                   dbg_tid_t tid,
                                   dbg_core_pid_info_t *p_pid_info);

/*Find a breakpoint_info for a given breakpoint/watchpoint address and thread-id*/
dbg_core_bp_info_t *
DBG_core_find_bp_info_from_addr(uint32_t u32_adddr,
                                DBG_BPT_WPT_TYPE bpt_type,
                                dbg_pid_t pid, dbg_tid_t tid,
                                dbg_core_pid_info_queue_t *p_pid_info_queue);

dbg_core_bp_info_t *
DBG_core_find_bp_info_from_addr_1(uint32_t u32_addr,
                                  DBG_BPT_WPT_TYPE bpt_type,
                                  dbg_tid_t tid,
                                  dbg_core_pid_info_t *p_pid_info);

/*Remove a breakpoint_info from the queue*/
void
DBG_core_remove_bp_info(uint8_t u8_bp_wp_num,
                        DBG_BPT_WPT_TYPE bpt_type,
                        dbg_core_bp_info_queue_t *p_bp_info_queue);

/*Get the breakpoint/watchpoint address for a given breakpoint/watchpoint number and thread-id*/
uint32_t
DBG_core_get_bp_addr(uint8_t u8_bp_wp_num, dbg_tid_t tid);

/*Get the number of bytes covered by the watchpoint wpnum for a thread tid*/
uint32_t
DBG_core_get_wp_num_bytes(uint8_t u8_bp_wp_num, dbg_pid_t pid, dbg_tid_t tid);


#endif
