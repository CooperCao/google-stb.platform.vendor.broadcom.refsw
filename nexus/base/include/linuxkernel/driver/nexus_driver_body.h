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

#include "nexus_base_statistics.h"
#include "nexus_client_resources.h"

static struct nexus_driver_module_state {
    struct nexus_driver_module_header header;
    union nexus_driver_module_args args;
} nexus_driver_module_state;

#define NEXUS_IOCTL_FAULT(call, arg)
#define NEXUS_IOCTL_CLEAR(ptr)

#if 0
#define NEXUS_IOCTL_ENTER(name) BDBG_WRN((">>%s", #name))
#define NEXUS_IOCTL_LEAVE(name) BDBG_WRN(("<<%s", #name))
#else
#define NEXUS_IOCTL_ENTER(name)
#define NEXUS_IOCTL_LEAVE(name) NEXUS_P_API_STATS_STOP(#name, NEXUS_MODULE_SELF)
#endif


/* route all address conversions through dedicated functions */
#define NEXUS_DRIVER_SEND_ADDR(addr,type)  nexus_driver_send_addr((void *)(&addr))
#define NEXUS_DRIVER_RECV_ADDR(addr,type)  nexus_driver_recv_addr_##type((void *)(&addr))

#define CLIENT_ID (&client->client)

#define NEXUS_DRIVER_CALLBACK_TO_DRIVER(callback, handle, id)  nexus_driver_callback_to_driver(&module->header, (callback), (void *)(handle),(id), CLIENT_ID, (((struct nexus_driver_module_driver_state *)context)->slave_scheduler))
#define NEXUS_DRIVER_CALLBACK_TO_USER(callback, handle, id) nexus_driver_callback_to_user(&module->header, (callback), (void*)(handle), (id))
#define NEXUS_DRIVER_CALLBACK_UPDATE(callback, old_handle, id, new_handle) nexus_driver_callback_update(&module->header, (callback), (void*)(old_handle), (id), (void *)(new_handle))
#define NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(callback, handle, id)  nexus_driver_callback_to_driver_commit(&module->header, (callback), (void *)(handle),(id))
#define NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(callback, handle, id)  nexus_driver_callback_to_driver_cancel(&module->header, (callback), (void *)(handle),(id))

#define P_CLASS(module, class_no) (&module->header.objdb.class_table[class_no])

#define NEXUS_DRIVER_CREATE_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister((void *)object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoCreate, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)
#define NEXUS_DRIVER_DESTROY_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister((void *)object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoDestroy, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)
#define NEXUS_DRIVER_ACQUIRE_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister((void *)object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoAcquire, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)
#define NEXUS_DRIVER_RELEASE_OBJECT(type, object) NEXUS_BaseObject_P_RegisterUnregister((void *)object, &NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_Object_P_RegisterUnregister_eAutoRelease, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF)

#define NEXUS_DRIVER_ALLOC(size) nexus_client_driver_alloc(CLIENT_ID, size)
#define NEXUS_DRIVER_FREE(ptr) nexus_driver_free(ptr)

int
nexus_driver_module_open(unsigned index)
{
    return nexus_driver_module_init(index, &nexus_driver_module_state.header, NEXUS_MODULE_SELF, NEXUS_DRIVER_MODULE_CLASS_TABLE, NEXUS_DRIVER_MODULE_NAME);
}

void
nexus_driver_module_close(void)
{
    nexus_driver_module_uninit(&nexus_driver_module_state.header);
}

int
nexus_driver_module_ioctl(void *context, unsigned int cmd, unsigned long arg, unsigned long type, bool unlocked)
{
    int rc = 0;
    struct nexus_driver_module_state *module = &nexus_driver_module_state;
    struct nexus_driver_client_state *client = nexus_driver_client_id(context);
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



    BSTD_UNUSED(module);

    NEXUS_P_API_STATS_START(); /* this statistics count all API call overhead (including synchronization overhead) */

    if(cmd == NEXUS_IOCTL_MODULE_INIT) {
        PROXY_NEXUS_ModuleInit init;
        NEXUS_IOCTL_ENTER(VERSION);
        if (copy_from_user_small(&init, (void*)arg , sizeof(init) ) !=0) {return NEXUS_DRIVER_TRACE(-1);}
        if ( init.version != NEXUS_IOCTL_MODULE_VERSION ) {return NEXUS_DRIVER_TRACE(-1);}
        NEXUS_IOCTL_LEAVE(VERSION);
        return rc;
    }

    if (!unlocked) {
        NEXUS_LockModule();
        b_objdb_set_client(CLIENT_ID);/* CLIENT_ID */
        /* NEXUS_Module_SetCurrentClient(NEXUS_MODULE_SELF, client->defaultHeap, client->defaultFullyMappedHeap, client->config.mode); */
    }
    
    if(nexus_driver_module_state.header.entry_count != 0) {
        goto err_reentrant;
        /* coverity[unreachable] - added to prevent warning in nexus_driver_epilogue.h */
        goto err_alloc;
    }

