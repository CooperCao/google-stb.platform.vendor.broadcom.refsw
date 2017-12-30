/***************************************************************************
 *  Copyright (C) 2006-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ************************************************************/

#ifndef NEXUS_GENERIC_DRIVER_IMPL_H__
#define NEXUS_GENERIC_DRIVER_IMPL_H__

#include "priv/nexus_generic_driver.h"
#include "nexus_generic_driver_impl.h"
#include "nexus_platform_server.h"
#include "driver/nexus_driver_prologue.h"
#include "nexus_driver_ioctl.h"
#include "blst_slist.h"
#include "bdbg_log.h"
#include "b_memory_regions.h"

void nexus_driver_callback_uninit(struct nexus_driver_module_header *header);

#define NEXUS_DRIVER_MAX_LOG_READERS    4

struct nexus_driver_state {
    struct nexus_generic_driver_init_settings settings;
    int16_t open_count;
    bool active; /* platform_init has been called */
    BKNI_MutexHandle lock; /* low level lock to protect driver state, no nexus functions shall be called whith lock held */
    NEXUS_PlatformStartServerSettings serverSettings;

    struct nexus_driver_client_state *server;
    bool uninit_pending; /* shutdown needs to be performed in non-signal_pending context */
    bool cleanup_pending; /* cleanup_clients has at least one entry */
    bool uninit_in_progress; /* driver is shutting down. no new opens allowed. */
    struct {
        BDBG_Fifo_Handle logWriter;
        struct {
            BDBG_FifoReader_Handle fifo;
        }readers[NEXUS_DRIVER_MAX_LOG_READERS];
        unsigned char buffer[64*1024];
        void *dynamicBuffer;
    } debugLog;
    b_memory_region_set dynamic_memory;

    BLST_S_HEAD(nexus_driver_client_list, nexus_driver_client_state) clients; /* allowed, open */
    BLST_S_HEAD(nexus_driver_allowed_client_list, nexus_driver_client_state) allowed_clients; /* allowed, but not open */
    BLST_S_HEAD(nexus_driver_cleanup_client_list, nexus_driver_client_state) cleanup_clients; /* allowed, closed, need to be cleaned up */
};

extern struct nexus_driver_state nexus_driver_state;

BERR_Code nexus_driver_scheduler_init(void);
void nexus_driver_scheduler_uninit(void);
int nexus_driver_run_scheduler(NEXUS_ModulePriority priority, unsigned timeout, bool *p_has_callbacks, struct nexus_driver_slave_scheduler *slave);
unsigned nexus_driver_scheduler_dequeue(NEXUS_ModulePriority priority, nexus_driver_callback_desc *desc, unsigned nentries, struct nexus_driver_slave_scheduler *slave, bool compat);
void nexus_driver_deactivate_callbacks(void *context, void *object, const struct b_objdb_client *client);
void nexus_driver_scheduler_lock(void *client, NEXUS_ModulePriority priority, bool lock);
void nexus_driver_unlock_schedulers(void *client);

/* nexus_driver_objects */
struct nexus_driver_object_heap_entry *nexus_driver_module_object_get_newest(struct nexus_driver_module_header *header, unsigned class_no, void *client_id);
void nexus_driver_module_object_uninit(struct nexus_driver_module_header *header);
void nexus_driver_module_object_cleanup(struct nexus_driver_module_header *header);
void nexus_driver_module_object_uninit_one(struct nexus_driver_module_header *header, unsigned class_no, struct nexus_driver_object_heap_entry *object, void *client_id);

struct nexus_driver_slave_scheduler * nexus_driver_slave_scheduler_create(void);
void nexus_driver_slave_scheduler_release(struct nexus_driver_module_header *header, struct nexus_driver_slave_scheduler *slave);
void nexus_driver_slave_scheduler_destroy(struct nexus_driver_module_header *header, struct nexus_driver_slave_scheduler *slave);

/* nexus_generic_driver.c funcs called from nexus_platform_server. */
struct nexus_driver_client_state *nexus_driver_create_client(const NEXUS_Certificate *pCertificate, const NEXUS_ClientConfiguration *pConfig);
void nexus_driver_destroy_client(struct nexus_driver_client_state *client);
void nexus_driver_disable_clients(bool including_server);
void nexus_driver_get_client_configuration(const struct b_objdb_client *client_id, NEXUS_ClientConfiguration *pSettings);
void nexus_driver_server_close_module_headers(void);

int nexus_driver_server_postinit(void);
void nexus_driver_server_preuninit(void);

void NEXUS_P_Proxy_StopCallbacks( void *interfaceHandle );
void NEXUS_P_Proxy_StartCallbacks( void *interfaceHandle );

void NEXUS_Platform_P_TerminateProcess(unsigned pid);
NEXUS_Error nexus_p_set_client_mode(struct nexus_driver_client_state *client, NEXUS_ClientMode mode);

#endif /* NEXUS_GENERIC_DRIVER_IMPL_H__ */
