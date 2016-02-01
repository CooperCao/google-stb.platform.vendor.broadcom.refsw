/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: Platform (private)
*    Common part of all kernel drivers
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _NEXUS_DRIVER_PROLOGUE_H_
#define _NEXUS_DRIVER_PROLOGUE_H_

#include "nexus_types.h"
#include "nexus_memory.h"
#include "nexus_base_ioctl.h"
#include "priv/nexus_core_driver.h"
#include "nexus_platform_client.h"
#include "bkni.h"
#include "b_objdb.h"

#if BDBG_DEBUG_BUILD
#define NEXUS_DRIVER_TRACE(x)   BERR_TRACE(x)
#else
#define NEXUS_DRIVER_TRACE(x)   (x)
#endif


struct nexus_driver_callback_map;

/*
nexus_driver_client_state - one per client (including one for the server)
*/
BDBG_OBJECT_ID_DECLARE(nexus_driver_client_state);
struct nexus_driver_client_state {
    BDBG_OBJECT(nexus_driver_client_state)
    enum {
        nexus_driver_clients_list,
        nexus_driver_allowed_clients_list,
        nexus_driver_cleanup_clients_list,
    } list;
    BLST_S_ENTRY(nexus_driver_client_state) link;
    NEXUS_Certificate certificate; /* set by the server, verified by join */
    unsigned pid; /* if zero, then not joined */
#define NEXUS_MAX_PROCESS_NAME 32
    char process_name[NEXUS_MAX_PROCESS_NAME];
    unsigned refcnt; /* number of nexus_driver_module_driver_state's opened */
    struct b_objdb_client client;
    bool dynamic; /* dynamically created by driver */
    bool joined; /* set true on join, false on uninit */
    unsigned numJoins;
};

/* struct nexus_driver_slave_scheduler; */

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

/* route all copy_to/from_user to local functions, this is needed in order to reduce size of generated drivers */
unsigned long copy_to_user_small(void * to, const void * from, unsigned long n);
unsigned long copy_from_user_small(void * to, const void * from, unsigned long n);

/*
nexus_driver_module_driver_state - one per client per module
*/
struct nexus_driver_module_driver_state {
    struct nexus_driver_client_state *client;
    struct nexus_driver_slave_scheduler *slave_scheduler;
    unsigned pid; /* store pid before client created */
    char process_name[NEXUS_MAX_PROCESS_NAME];
};

void nexus_driver_send_addr(void **paddr); /* inplace convert address from virtual to physical */
void nexus_driver_recv_addr_cached(void **paddr); /* inplace convert address from physical to virtual */
void nexus_driver_free(void *addr); /* special version of BKNI_Free which is safe to use for free NULL pointer */

/* nexus_driver_callbacks */
void nexus_driver_callback_to_driver(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id, 
    const struct b_objdb_client *client, struct nexus_driver_slave_scheduler *slave);
void nexus_driver_callback_to_user(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);
void nexus_driver_callback_update(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *old_handle, unsigned id, void *new_handle);
void nexus_driver_callback_to_driver_commit(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);
void nexus_driver_callback_to_driver_cancel(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);

/* nexus_generic_driver.c functions accessible from proxy */
int nexus_driver_module_init(unsigned index, struct nexus_driver_module_header *header, NEXUS_ModuleHandle module, const struct b_objdb_class *class_table, const char *name);
void nexus_driver_module_uninit(struct nexus_driver_module_header *header);
struct nexus_driver_client_state *nexus_driver_client_id(struct nexus_driver_module_driver_state *state);

#endif 
