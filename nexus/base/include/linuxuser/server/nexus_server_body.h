/***************************************************************************
* Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Platform (private)
*    Common part of all kernel servers
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_base_statistics.h"
#include "nexus_client_resources.h"

#if 0
#define NEXUS_IPC_ENTER(name) BDBG_WRN((">>%s", #name))
#define NEXUS_IPC_LEAVE(name) BDBG_WRN(("<<%s", #name))
#else
#define NEXUS_IPC_ENTER(name)
#define NEXUS_IPC_LEAVE(name) NEXUS_P_API_STATS_STOP(#name, NEXUS_MODULE_SELF)
#endif

/* global storage for the module */
static struct nexus_driver_module_state {
    struct nexus_driver_module_header header;
} nexus_driver_module_state;


/* route all address conversions through dedicated functions */
#define NEXUS_DRIVER_SEND_ADDR(addr,type)  nexus_driver_send_addr((void *)(&addr))
#define NEXUS_DRIVER_RECV_ADDR(addr,type)  nexus_driver_recv_addr_##type((void *)(&addr))

#define CLIENT_ID (&client_module_state->client->client)

#define NEXUS_DRIVER_CALLBACK_TO_DRIVER(callback, handle, id)  nexus_driver_callback_to_driver(module_header, (callback), (void *)(handle), (id), CLIENT_ID, client_module_state->slave_scheduler)
#define NEXUS_DRIVER_CALLBACK_TO_CLIENT(callback, handle, id) nexus_driver_callback_to_user(module_header, (callback), (void*)(handle), (id))
#define NEXUS_DRIVER_CALLBACK_UPDATE(callback, old_handle, id, new_handle) nexus_driver_callback_update(module_header, (callback), (void*)(old_handle), (id), (void *)(new_handle))
#define NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(callback, handle, id)  nexus_driver_callback_to_driver_commit(module_header, (callback), (void *)(handle),(id))
#define NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(callback, handle, id)  nexus_driver_callback_to_driver_cancel(module_header, (callback), (void *)(handle),(id))

#define NEXUS_DRIVER_CREATE_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister(object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoCreate, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)
#define NEXUS_DRIVER_DESTROY_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister(object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoDestroy, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)
#define NEXUS_DRIVER_ACQUIRE_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister(object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoAcquire, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)
#define NEXUS_DRIVER_RELEASE_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister(object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoRelease, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)

#define NEXUS_P_SERVER_OUT_REALLOC(_length) { unsigned __length=_length;void *new_out = nexus_client_driver_alloc(CLIENT_ID, out_mem_size + __length);if(new_out==NULL) {(void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_fault;};BKNI_Memcpy(new_out, out_data, __variable_out_offset);if((void *)out_data != vout_data) {BKNI_Free(out_data);} out_data = new_out;out_mem_size = out_mem_size + __length; }while(0)

int
nexus_server_module_open(struct nexus_driver_module_header **pHeader)
{
    b_objdb_add_module(&nexus_driver_module_state.header.objdb, NEXUS_DRIVER_MODULE_CLASS_TABLE, NEXUS_MODULE_SELF);
    nexus_driver_module_state.header.objdb.name = NEXUS_SERVER_MODULE_NAME;
    nexus_driver_module_state.header.objdb.module = NEXUS_MODULE_SELF;
    nexus_driver_module_state.header.objdb.cancel_callbacks_locked = nexus_driver_deactivate_callbacks;
    nexus_driver_module_state.header.objdb.cancel_callbacks_context = &nexus_driver_module_state.header;

    nexus_driver_module_state.header.name = NEXUS_SERVER_MODULE_NAME;

    /* nexus_platform_server needs the thunk layer's header storage */
    *pHeader = &nexus_driver_module_state.header;
    nexus_driver_module_state.header.module = NEXUS_MODULE_SELF;
    if (nexus_driver_module_state.header.module) {
        return 0;
    }
    else {
        /* it is normal that a module is compiled in but not initialized. */
        return -1;
    }
}

int
nexus_server_module_close(void)
{
    b_objdb_remove_module(&nexus_driver_module_state.header.objdb, NEXUS_MODULE_SELF);
    BKNI_Memset(&nexus_driver_module_state.header, 0, sizeof(nexus_driver_module_state.header));
    return 0;
}

int
nexus_server_process(void *vclient, void *vin_data, unsigned in_data_size, void *vout_data, unsigned out_mem_size, struct nexus_p_server_process_output *out)
{
    int rc = 0;
    struct nexus_driver_module_driver_state *client_module_state = vclient;
    struct nexus_driver_module_header *module_header = &nexus_driver_module_state.header;
    nexus_server_args *in_data = vin_data;
    nexus_server_args *out_data = vout_data;
    unsigned function_id;
    unsigned __variable_out_offset = 0;
    struct nexus_driver_client_state *client = client_module_state->client;

    NEXUS_P_API_STATS_STATE();

    /*
     * The following is to address Coverity false positive. The Coverity stack-size checker adds 4 bytes for every function call made.
     * For very long function with lots of function calls, the 4 bytes addition will lead to stack-overflow Coverity false positive event.
     * The following set the stack size for the function to 20,000 bytes to workaround the false positive.
     * Vladimir verified the 4 byte addition is incorrect by looking at the assembly code generated by the C compiler.
     */
#if __COVERITY__
    __coverity_stack_depth__(20000);
#endif

    BSTD_UNUSED(__variable_out_offset); /* may be unused */
    NEXUS_P_API_STATS_START(); /* this statistics count all API call overhead (including synchronization overhead) */

    if (!in_data) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    function_id = in_data->header.function_id;

    if (in_data->header.version != NEXUS_IPC_HEADER_VERSION) {
        BDBG_ERR(("func %#x: wrong version: %#x != %#x", function_id, in_data->header.version, (unsigned)NEXUS_IPC_HEADER_VERSION));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    NEXUS_Module_Lock(module_header->module);
    b_objdb_set_client(&client->client); /* CLIENT_ID */

    switch(in_data->header.function_id) {
    default:
        goto err_invalid_ipc;
        /* coverity[unreachable] - added to prevent warning in nexus_driver_epilogue.h */
        goto err_alloc;
        /* coverity[unreachable] - added to prevent warning in nexus_driver_epilogue.h */
        goto err_fault;

