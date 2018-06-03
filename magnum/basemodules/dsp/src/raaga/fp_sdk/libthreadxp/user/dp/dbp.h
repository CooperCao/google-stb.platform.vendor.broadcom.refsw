/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

/******************************************************************************
 *                Copyright (c) 2016 Broadcom Limited                         *
 *                                                                            *
 *      This material is the confidential trade secret and proprietary        *
 *      information of Broadcom Limited. It may not be reproduced, used       *
 *      sold or transferred to any third party without the prior written      *
 *            consent of Broadcom Limited. All rights reserved.               *
 *                                                                            *
 ******************************************************************************/

#pragma once
#include <libfp/c_utils.h>
#include <libfp/context_save.h>
#include <stdint.h>
#include <libthreadxp/pm_status.h>

/**
 * This is the first version of the debug protocol
 */
#define DBP_MAJOR 1
#define DBP_MINOR 0

/**
 * The structure of the debugger protocol in memory
 */
typedef enum
{
    DBP_CMD_INVALID,                  //!< DBP_CMD_INVALID
    DBP_CMD_GET_DBP_CONFIG,           //!< DBP_CMD_GET_DBP_CONFIG
    /*Commands to get OS information*/
    DBP_CMD_GET_PID_LIST,             //!< DBP_CMD_GET_PID_LIST
    DBP_CMD_PROCESS_GET_TID_LIST,     //!< DBP_CMD_PROCESS_GET_TID_LIST
    /*Commands to alter the state of the process*/
    DBP_CMD_PROCESS_CREATE,
    DBP_CMD_PROCESS_GET_STATE,        //!< DBP_CMD_PROCESS_GET_STATE
    DBP_CMD_PROCESS_FREEZE,           //!< DBP_CMD_PROCESS_FREEZE
    DBP_CMD_PROCESS_UNFREEZE,         //!< DBP_CMD_PROCESS_UNFREEZE
    DBP_CMD_PROCESS_TERMINATE,        //!< DBP_CMD_PROCESS_TERMINATE
    DBP_CMD_PROCESS_ATTACH,
    DBP_CMD_PROCESS_DETACH,
    /*Commands to alter the state of a thread in the process*/
    DBP_CMD_THREAD_GET_STATE,         //!< DBP_CMD_THREAD_GET_STATE
    DBP_CMD_THREAD_FREEZE,            //!< DBP_CMD_THREAD_FREEZE
    DBP_CMD_THREAD_UNFREEZE,          //!< DBP_CMD_THREAD_UNFREEZE
    /*Commands to get/set context information*/
    DBP_CMD_PROCESS_READ_MEMORY,      //!< DBP_CMD_PROCESS_READ_MEMORY
    DBP_CMD_PROCESS_WRITE_MEMORY,     //!< DBP_CMD_PROCESS_WRITE_MEMORY
    DBP_CMD_THREAD_READ_REGISTER,     //!< DBP_CMD_THREAD_READ_REGISTER
    DBP_CMD_THREAD_WRITE_REGISTER,    //!< DBP_CMD_THREAD_WRITE_REGISTER
    DBP_CMD_THREAD_READ_ALL_REGISTERS,//!< DBP_CMD_THREAD_READ_ALL_REGISTERS
    /*Commands to perform debug operations on the target*/
    DBP_CMD_SET_BREAKPOINT,           //!< DBP_CMD_SET_BREAKPOINT
    DBP_CMD_REMOVE_BREAKPOINT,        //!< DBP_CMD_REMOVE_BREAKPOINT
    DBP_CMD_SET_WATCHPOINT,           //!< DBP_CMD_SET_WATCHPOINT
    DBP_CMD_REMOVE_WATCHPOINT,        //!< DBP_CMD_REMOVE_WATCHPOINT
    DBP_CMD_THREAD_SET_SINGLE_STEP,   //!< DBP_CMD_THREAD_SET_SINGLE_STEP
    DBP_CMD_THREAD_CLEAR_SINGLE_STEP, //!< DBP_CMD_THREAD_CLEAR_SINGLE_STEP
    /*Commands relating to DSOs in a process*/
    DBP_CMD_PROCESS_GET_DSO_COUNT,    //!< DBP_CMD_PROCESS_GET_DSO_COUNT
    DBP_CMD_PROCESS_GET_DSO_SONAMES,  //!< DBP_CMD_PROCESS_GET_DSO_SONAME
    DBP_CMD_PROCESS_GET_DSO_SEGMENTS, //!< DBP_CMD_PROCESS_GET_DSO_SEGMENTS
    DBP_CMD_PROCESS_GET_EXE_SEGMENTS, //!< DBP_CMD_PROCESS_GET_EXE_SEGMENTS
    DBP_CMD_PROCESS_GET_DSO_INFO,

    /* Responses */
    DBP_RESP_BREAKPOINT_HIT,
    DBP_RESP_WATCHPOINT_HIT,
    DBP_RESP_SINGLE_STEP_COMPLETE,
    DBP_RESP_PROCESS_EXITED,          //Not currently implemented
    DBP_RESP_THREAD_CREATED,          //Not currently implemented
    DBP_RESP_THREAD_EXITED,           //Not currently implemented
    DBP_RESP_LIBRARY_LOADED,          //Not currently implemented
    DBP_RESP_LIBRARY_UNLOADED,        //Not currently implemented
    DBP_RESP_FATAL

} DBP_CMD_TYPE_ID;

typedef enum
{
    DWP_WPT_LOAD = 1,
    DBP_WPT_STORE= 2,
    DBP_WPT_ALL  = 1|2,
} DBP_WPT_TYPE_ID;

typedef enum
{
    // The initial error codes correspond to the process manager status values
    // although some of them may never be returned. This mapping must be
    // maintained because it allows the target to return status values from its
    // process manager back to the host.

    DBP_ERR_SUCCESS     = PM_SUCCESS,     // The call completed without error
    DBP_ERR_NO_ACCESS   = PM_NO_ACCESS,   // The presented capability does not allow use if this API
    DBP_ERR_NO_PROCESS  = PM_NO_PROCESS,  // There is no process on this system with the given id
    DBP_ERR_NO_THREAD   = PM_NO_THREAD,   // There is no thread in this process with the given id
    DBP_ERR_BUSY        = PM_BUSY,        // The resource specified is already in use
    DBP_ERR_NO_RESOURCE = PM_NO_RESOURCE, // There is no resource with the given value
    DBP_ERR_NUFFER      = PM_BUFFER,      // There isn't room in the message buffer for the result
    DBP_ERR_ERRNO       = PM_ERRNO,       // The call failed due to an error and errno is set
    DBP_ERR_TX_ERROR    = PM_TX_ERROR,    // The call failed due to an error and tx_status returned

    // Non-pm errors
    DBP_ERR_BAD_COMMAND,                  // The DBP message specified an unknown command id

} DBP_ERR_ID;

// The faults that are returned as the type field in a dbp_fault_rsp_t response.
typedef enum
{
    DBP_FAULT_CODE_UNMAPPED,
    DBP_FAULT_MISALIGNED_PC,
    DBP_FAULT_INVALID_INSN,
    DBP_FAULT_INVALID_SVC,
    DBP_FAULT_DATA_UNMAPPED,
    DBP_FAULT_BUS,
    DBP_FAULT_UNKNOWN,
} DBP_FAULT_ID;

typedef struct
{
    uint32_t base; // Base address of segment
    size_t   size; // Size of segment in bytes
} dbp_segment_info_t;

// A successful response
#ifdef __FIREPATH__
#define DBP_SUCCESS  0
#endif

// Indicate a positive (success)
#define DBP_PACK     0x80

// Indicate a failure
#define DBP_NACK     0x40


// -- FAILURE ------------------------------------------------------------------

// For a failure response, the size of the response indicates whether there is
// an associated pid and tid.

typedef struct __packed
{
    uint16_t pid;       // pid, if there is one
    uint16_t tid;       // tid, if there is one
} dbp_failure_rsp_t;

// -- DPB_CMD_GET_DBP_CONFIG ---------------------------------------------------

typedef struct __packed
{
} dbp_get_dbp_config_cmd_t;

typedef struct __packed
{
    uint8_t  major;             // The major version of the debug protocol
    uint8_t  minor;             // The minor version of the debug protocol
    uint16_t size;              // The size in bytes of the packet buffer
    uint32_t address;           // The physical address of the packet buffer
} dbp_get_dbp_config_rsp_t;

// -- DBP_CMD_GET_PID_LIST -----------------------------------------------------

typedef struct __packed
{
    uint8_t core;               // The single core for which the pids are needed
} dbp_get_pid_list_cmd_t;

typedef struct __packed
{
    uint16_t pid[1];            // First element in a list of pids for this core
} dbp_get_pid_list_rsp_t;


// -- DBP_CMD_PROCESS_GET_TID_LIST ---------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for which the list is required
} dbp_process_get_tid_list_cmd_t;

typedef struct __packed
{
    uint16_t pid;              // The pid from which the list was obtained
    uint16_t tid[];            // First element in a list of tids for this pid
} dbp_process_get_tid_list_rsp_t;

// -- DBP_CMD_PROCESS_CREATE ---------------------------------------------------
//
// This expects the host to have parsed the command line into elements so, for
// example, to spawn "helloworld.fpexe two 'a and b'" we expect to see this
// struct contain {.argc=3, .args='helloworld.fpexe\0two\0a and b\0'}
typedef struct __packed
{
    int  argc;      // Number of arguments (first is path to program)
    char args[];    // Sequence of <argc> 0-terminated argument strings
} dbp_process_create_cmd_t;


typedef struct __packed
{
    uint16_t pid;   // The process id of the newly created process
} dbp_process_create_rsp_t;

// -- DBP_CMD_PROCESS_GET_STATE ------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for which the state is required
} dbp_process_get_state_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid for which the state is provided
    uint8_t  is_suspended;      // true iff the process is suspended
    uint8_t  is_frozen;         // true iff the process is frozen
} dbp_process_get_state_rsp_t;

// -- DBP_CMD_PROCESS_FREEZE ---------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process to be frozen
} dbp_process_freeze_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // the pid of the process that has been frozen
    uint16_t tids[];            // List of tids for threads that are frozen
} dbp_process_freeze_rsp_t;

// -- DBP_CMD_PROCESS_UNFREEZE -------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process to be unfrozen
} dbp_process_unfreeze_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The process we unfroze
} dbp_process_unfreeze_rsp_t;

// -- DBP_CMD_PROCESS_WRITE_MEMORY ----------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process to which to write
    uint32_t address;           // Address (in process address space) to write
    uint16_t size;              // Size in bytes of block to be written
    uint8_t  data[];            // First byte of data to write
} dbp_process_write_memory_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The process we wrote to
    uint16_t written;           // The number of bytes we actually wrote
} dbp_process_write_memory_rsp_t;


// -- DBP_CMD_PROCESS_READ_MEMORY ----------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process from which to read
    uint32_t address;           // Address (in process address space) to read
    uint16_t size;              // Size in bytes of block to be read
} dbp_process_read_memory_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // the pid of the process from which we read
    uint32_t address;           // Address (in process address space) read
    uint16_t size;              // Size in bytes of block actually read
    uint8_t  data[];            // First byte of data read
} dbp_process_read_memory_rsp_t;

// -- DBP_CMD_PROCESS_TERMINATE ------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process to terminate
    uint8_t  exit_code;         // The exit code
} dbp_process_terminate_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // the pid of the process which we terminated
} dbp_process_terminate_rsp_t;

// -- DBP_CMD_PROCESS_ATTACH ---------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process to which to attach
} dbp_process_attach_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // the pid of the process to which we attached

} dbp_process_attach_rsp_t;

// -- DBP_CMD_PROCESS_DETACH ---------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process to detach
} dbp_process_detach_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // the pid of the process we detached

} dbp_process_detach_rsp_t;

// -- DBP_CMD_THREAD_GET_STATE ------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // the tid of the thread whose state we want
} dbp_thread_get_state_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // the tid of the thread whose state we want
    uint8_t  is_suspended;      // true iff the process is suspended
    uint8_t  is_frozen;         // true iff the process is frozen
} dbp_thread_get_state_rsp_t;

// -- DBP_CMD_THREAD_FREEZE ----------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // the tid of the thread to freeze
} dbp_thread_freeze_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // the tid of the thread to freeze
} dbp_thread_freeze_rsp_t;

// -- DBP_CMD_THREAD_UNFREEZE --------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // the tid of the thread to unfreeze
} dbp_thread_unfreeze_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // the tid of the thread to unfreeze
} dbp_thread_unfreeze_rsp_t;


// -- DBP_CMD_THREAD_READ_REGISTER ---------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // The tid of the thread whose register is read
    uint16_t regnum;            // Number of the register to read
} dbp_thread_read_register_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // the pid of the process  we read
    uint16_t tid;               // the tid of the thread we read
    uint64_t value;             // Value of the register (8, 32 or 64 bytes)
} dbp_thread_read_register_rsp_t;

// -- DBP_CMD_THREAD_WRITE_REGISTER --------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // The tid of the thread whose register is written
    uint16_t regnum;            // Number of the register to write
    uint64_t value;             // New value of the register (8, 32 or 64 bytes)
} dbp_thread_write_register_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // the pid of the process we wrote
    uint16_t tid;               // the tid of the thread we wrote
} dbp_thread_write_register_rsp_t;

// -- DBP_CMD_THREAD_READ_ALL_REGISTERS ----------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid of the process owning the thread
    uint16_t tid;               // The tid of the thread
} dbp_thread_read_all_registers_cmd_t;

typedef struct __packed
{
    uint16_t           pid;             // the pid of the process we wrote
    uint16_t           tid;             // the tid of the thread we wrote
    CS_registerContext context;         // the register context
    uint8_t            predicates[4];   // the predicate registers
} dbp_thread_read_all_registers_rsp_t;


// -- DBP_CMD_SET_BREAKPOINT ---------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
    uint32_t pc;                // Address for breakpoint
} dbp_set_breakpoint_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
    uint32_t pc;                // The address of the breakpoint set
    uint8_t  bp;                // The breakpoint number allocated
} dbp_set_breakpoint_rsp_t;

// -- DBP_CMD_REMOVE_BREAKPOINT ------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
    uint8_t  bp;                // Number of breakpoint
} dbp_remove_breakpoint_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
    uint8_t  bp;                // The breakpoint that was removed
} dbp_remove_breakpoint_rsp_t;

// -- DBP_CMD_SET_WATCHPOINT ---------------------------------------------------

typedef struct __packed
{
    uint16_t         pid;               // The pid for the process
    uint16_t         tid;               // The tid for the thread or 0xFFFF for all
    uint32_t         address;           // Address for breakpoint
    uint32_t         length;            // Coverage of watchpoint in bytes
    DBP_WPT_TYPE_ID  type : 8;          // Watchpoint type
} dbp_set_watchpoint_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
    uint32_t address;           // The address of the watchpoint set
    uint8_t  wp;                // The breakpoint number allocated
    DBP_WPT_TYPE_ID  type : 8;  // Watchpoint type
} dbp_set_watchpoint_rsp_t;

// -- DBP_CMD_REMOVE_WATCHPOINT ------------------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
    uint8_t  wp;                // Number of watchpoint
} dbp_remove_watchpoint_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
    uint8_t  wp;                // The watchpoint that was removed
} dbp_remove_watchpoint_rsp_t;

// -- DBP_CMD_THREAD_SET_SINGLE_STEP -------------------------------------------

typedef struct __packed
{
    uint16_t         pid;            // The pid for the process
    uint16_t         tid;            // The tid for the thread or 0xFFFF for all
} dbp_thread_set_single_step_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
} dbp_thread_set_single_step_rsp_t;

// -- DBP_CMD_THREAD_CLEAR_SINGLE_STEP -----------------------------------------

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
} dbp_thread_clear_single_step_cmd_t;

typedef struct __packed
{
    uint16_t pid;               // The pid for the process
    uint16_t tid;               // The tid for the thread or 0xFFFF for all
} dbp_thread_clear_single_step_rsp_t;


// -- DBP_CMD_PROCESS_GET_DSO_COUNT --------------------------------------------

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
} dbp_process_get_dso_count_cmd_t;

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
    uint16_t   count;       // The number of DSOs
} dbp_process_get_dso_count_rsp_t;

// -- DBP_CMD_PROCESS_GET_DSO_SONAMES ------------------------------------------

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
    uint32_t   start;       // Index of first SONAME to return
} dbp_process_get_dso_sonames_cmd_t;

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
    uint32_t   start;       // The first index requested
    uint32_t   count;       // The number of DSOs returned
    char       sonames[];   // Sequence of zero-terminated SONAME strings
} dbp_process_get_dso_sonames_rsp_t;

// -- DBP_CMD_PROCESS_GET_DSO_SEGMENTS -----------------------------------------

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
    uint32_t   index;       // Index of DSO to be queried
} dbp_process_get_dso_segments_cmd_t;

typedef struct __packed
{
    uint16_t   pid;                 // The pid for the process
    uint32_t   index;               // Index of the DSO queried
    uint32_t   count;               // The number of segments returned
    dbp_segment_info_t segments[];
} dbp_process_get_dso_segments_rsp_t;


// -- DBP_CMD_PROCESS_GET_EXE_SEGMENTS -----------------------------------------

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
} dbp_process_get_exe_segments_cmd_t;

typedef struct __packed
{
    uint16_t   pid;                 // The pid for the process
    uint32_t   count;               // The number of segments returned
    dbp_segment_info_t segments[];
} dbp_process_get_exe_segments_rsp_t;


// -- DBP_CMD_PROCESS_GET_DSO_INFO ---------------------------------------------
//
// The response contains an info field which is a sequence of dso descriptions.
// These descriptions have the following grammar:
//
// <info>      :== <dso_dx>*
// <dso_dx>    :== <soname> <#segments> <segments>
// <soname>    :== a 0-terminated ascii string
// <#segments> :== uint8_t
// <segments>  :== <segment>*
// <segment>   :== <base> <size>
// <base>      :== uint32_t
// <size>      :== uint32_t
//

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
} dbp_process_get_dso_info_cmd_t;

typedef struct __packed
{
    uint16_t   pid;         // The pid for the process
    uint8_t    count;       // The number of dso descriptions returned
    uint8_t    info[];      // Raw dso info - sequence of dso descriptions
} dbp_process_get_dso_info_rsp_t;


// The following events can be generated by the target without the need for a
// corresponding command. In all cases, the process or thread generating the
// event will be frozen to allow the debugger to intervene.

// -- DBP_RESP_FATAL -----------------------------------------------------------

typedef struct __packed
{
    uint16_t        pid;        // The pid of the faulting thread
    uint16_t        tid;        // The faulting thread
    uint32_t        pc;         // The pc at which the fault occurred
    uint32_t        address;    // The address (of relevent) causing the fault
    DBP_FAULT_ID    type : 32;  // The type of fatal event (a tx error type)

} dbp_fatal_rsp_t;

// -- DBP_RESP_BREAKPOINT_HIT --------------------------------------------------

typedef struct __packed
{
    uint16_t         pid;
    uint16_t         tid;
    uint32_t         address;
} dbp_breakpoint_hit_rsp_t;

// -- DBP_RESP_WATCHPOINT_HIT --------------------------------------------------

typedef struct __packed
{
    uint16_t         pid;
    uint16_t         tid;
    uint32_t         pc;
    uint32_t         address;
} dbp_watchpoint_hit_rsp_t;

// -- DBP_RESP_SINGLE_STEP_COMPLETE --------------------------------------------

typedef struct __packed
{
    uint16_t         pid;
    uint16_t         tid;
    uint32_t         pc;
} dbp_single_step_complete_rsp_t;


// -- DBP_RESP_PROCESS_EXITED --------------------------------------------------

typedef struct __packed
{
    uint16_t        pid;        // The pid of the faulting thread
} dbp_process_exited_rsp_t;

// -- DBP_RESP_THREAD_CREATED --------------------------------------------------

typedef struct __packed
{
    uint16_t        pid;        // The pid of the owning process
    uint16_t        tid;        // The tid of the newly created thread
} dbp_thread_created_rsp_t;

// -- DBP_RESP_THREAD_EXITED ---------------------------------------------------

typedef struct __packed
{
    uint16_t        pid;        // The pid of the owning process
    uint16_t        tid;        // The tid of the deleted thread
} dbp_thread_exited_rsp_t;


// -- DBP_RESP_LIBRARY_LOADED --------------------------------------------------

typedef struct __packed
{
    uint16_t        pid;        // The pid of the owning process
    uint16_t        tid;        // The thread that loaded the library
    uint32_t        address;    // Address of the mapped info for the library
} dbp_library_loaded_rsp_t;

// -- DBP_RESP_LIBRARY_UNLOADED ------------------------------------------------

typedef struct __packed
{
    uint16_t        pid;        // The pid of the owning process
    uint16_t        tid;        // The thread that unloaded the library
    uint32_t        address;    // Address of the mapped info for the library
} dbp_library_unloaded_rsp_t;



// -----------------------------------------------------------------------------

typedef struct __packed
{
    // Provide a tagged anonymous structure for the header so we can apply
    // sizeof without needing to sprinkle code with an extra level of field.
    struct dbp_header
    {
        uint16_t        msglen;       // Length of the entire payload in bytes
        DBP_CMD_TYPE_ID command : 8;  // The command provoking the response
        uint8_t         reserved;
    } dbp_header;

    // command-specific payloads
    union
    {
        dbp_get_dbp_config_cmd_t            get_dbp_config;
        dbp_get_pid_list_cmd_t              get_pid_list;

        dbp_process_create_cmd_t            process_create;
        dbp_process_get_state_cmd_t         process_get_state;
        dbp_process_freeze_cmd_t            process_freeze;
        dbp_process_unfreeze_cmd_t          process_unfreeze;
        dbp_process_attach_cmd_t            process_attach;
        dbp_process_detach_cmd_t            process_detach;
        dbp_process_read_memory_cmd_t       process_read_memory;
        dbp_process_write_memory_cmd_t      process_write_memory;
        dbp_process_terminate_cmd_t         process_terminate;
        dbp_process_get_tid_list_cmd_t      process_get_tid_list;
        dbp_process_get_dso_count_cmd_t     process_get_dso_count;
        dbp_process_get_dso_sonames_cmd_t   process_get_dso_sonames;
        dbp_process_get_dso_segments_cmd_t  process_get_dso_segments;
        dbp_process_get_exe_segments_cmd_t  process_get_exe_segments;
        dbp_process_get_dso_info_cmd_t      process_get_dso_info;

        dbp_set_breakpoint_cmd_t            set_breakpoint;
        dbp_remove_breakpoint_cmd_t         remove_breakpoint;
        dbp_set_watchpoint_cmd_t            set_watchpoint;
        dbp_remove_watchpoint_cmd_t         remove_watchpoint;

        dbp_thread_set_single_step_cmd_t    thread_set_single_step;
        dbp_thread_clear_single_step_cmd_t  thread_clear_single_step;
        dbp_thread_get_state_cmd_t          thread_get_state;
        dbp_thread_freeze_cmd_t             thread_freeze;
        dbp_thread_unfreeze_cmd_t           thread_unfreeze;
        dbp_thread_read_register_cmd_t      thread_read_register;
        dbp_thread_write_register_cmd_t     thread_write_register;
        dbp_thread_read_all_registers_cmd_t thread_read_all_registers;
    };
} dbp_command_t;


typedef struct
{
    uint16_t    msglen;
    uint8_t     response;
    DBP_ERR_ID  status : 8;
    union
    {
        dbp_failure_rsp_t                   failure;
        dbp_breakpoint_hit_rsp_t            breakpoint_hit;
        dbp_watchpoint_hit_rsp_t            watchpoint_hit;
        dbp_single_step_complete_rsp_t      single_step_complete;
        dbp_fatal_rsp_t                     fatal;
        dbp_process_exited_rsp_t            process_exited;
        dbp_get_dbp_config_rsp_t            get_dbp_response;
        dbp_get_pid_list_rsp_t              get_pid_list;
        dbp_thread_created_rsp_t            thread_created;
        dbp_thread_exited_rsp_t             thread_exited;
        dbp_library_loaded_rsp_t            library_loaded;
        dbp_library_unloaded_rsp_t          library_unloaded;

        dbp_process_create_rsp_t            process_create;
        dbp_process_get_state_rsp_t         process_get_state;
        dbp_process_freeze_rsp_t            process_freeze;
        dbp_process_unfreeze_rsp_t          process_unfreeze;
        dbp_process_attach_rsp_t            process_attach;
        dbp_process_detach_rsp_t            process_detach;
        dbp_process_read_memory_rsp_t       process_read_memory;
        dbp_process_write_memory_rsp_t      process_write_memory;
        dbp_process_terminate_rsp_t         process_terminate;
        dbp_process_get_tid_list_rsp_t      process_get_tid_list;
        dbp_process_get_dso_count_rsp_t     process_get_dso_count;
        dbp_process_get_dso_sonames_rsp_t   process_get_dso_sonames;
        dbp_process_get_dso_segments_rsp_t  process_get_dso_segments;
        dbp_process_get_exe_segments_rsp_t  process_get_exe_segments;
        dbp_process_get_dso_info_rsp_t      process_get_dso_info;

        dbp_set_breakpoint_rsp_t            set_breakpoint;
        dbp_remove_breakpoint_rsp_t         remove_breakpoint;
        dbp_set_watchpoint_rsp_t            set_watchpoint;
        dbp_remove_watchpoint_rsp_t         remove_watchpoint;

        dbp_thread_set_single_step_rsp_t    thread_set_single_step;
        dbp_thread_clear_single_step_rsp_t  thread_clear_single_step;
        dbp_thread_get_state_rsp_t          thread_get_state;
        dbp_thread_freeze_rsp_t             thread_freeze;
        dbp_thread_unfreeze_rsp_t           thread_unfreeze;
        dbp_thread_read_register_rsp_t      thread_read_register;
        dbp_thread_write_register_rsp_t     thread_write_register;
        dbp_thread_read_all_registers_rsp_t thread_read_all_registers;
    };
} dbp_response_t;
