/***************************************************************************
*  Copyright (C) 2004-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*   API name: Platform (private)
*    Common part of all kernel drivers
*
***************************************************************************/
#ifndef _NEXUS_SERVER_PROLOGUE_H_
#define _NEXUS_SERVER_PROLOGUE_H_

#include "nexus_types.h"
#include "nexus_memory.h"
#include "nexus_ipc_api.h"
#include "bkni.h"
#include "b_objdb.h"

struct nexus_driver_callback_map;

#define NEXUS_P_API_ID(module, api) NEXUS_P_API_##module##api##_id

/*
nexus_driver_client_state - one per client (including one for the server)
*/
BDBG_OBJECT_ID_DECLARE(nexus_driver_client_state);
struct nexus_driver_client_state {
    struct b_objdb_client client; /* address of this is the client.
                              also, client.trusted counts whether it is trusted. */
#if NEXUS_COMPAT_32ABI
    unsigned abi;
#endif
};

/*
nexus_driver_module_header - one per module
*/
struct nexus_driver_module_header {
    int16_t entry_count;
    int16_t open_count;
    struct nexus_driver_callback_map *callback_map;
    NEXUS_ModuleHandle module; /* sync for callbacks & objdb */
    const char *name;
    struct b_objdb_module objdb;
};

/*
nexus_driver_module_driver_state - one per client per module
*/
struct nexus_driver_module_driver_state {
    struct nexus_driver_client_state *client;
    struct nexus_driver_slave_scheduler *slave_scheduler;
};

typedef struct nexus_driver_callback_desc {
    NEXUS_CallbackDesc desc;
    void *interfaceHandle; /* interface handle that is used in NEXUS_Start/StopCallbacks */
} nexus_driver_callback_desc;

#if !defined(NEXUS_ABICOMPAT_MODE)
void nexus_driver_send_addr(void **paddr); /* inplace convert address from virtual to physical */
void nexus_driver_recv_addr_cached(void **paddr); /* inplace convert address from physical to virtual */
#endif

/* nexus_driver_callbacks */
void nexus_driver_callback_to_driver(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id, 
    const struct b_objdb_client *client, struct nexus_driver_slave_scheduler *context);
void nexus_driver_callback_to_user(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);
void nexus_driver_callback_update(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *old_handle, unsigned id, void *new_handle);
void nexus_driver_callback_to_driver_commit(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);
void nexus_driver_callback_to_driver_cancel(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);

struct b_objdb_client;
void nexus_driver_deactivate_callbacks(void *context, void *object,  const struct b_objdb_client *client, enum b_objdb_cancel_callbacks_action action);

struct nexus_p_server_process_output {
    void *data;
    unsigned size;
};

/** declare all module-specific functions. **/
#define NEXUS_PLATFORM_P_DRIVER_MODULE(X) \
    int nexus_server_##X##_open(struct nexus_driver_module_header **pHeader); \
    int nexus_server_##X##_close(void); \
    int nexus_server_##X##_process(void *driver_state, void *in_data, unsigned in_data_size, void *out_data, unsigned out_mem_size, struct nexus_p_server_process_output *out);
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE


#endif /* _NEXUS_SERVER_PROLOGUE_H_ */
