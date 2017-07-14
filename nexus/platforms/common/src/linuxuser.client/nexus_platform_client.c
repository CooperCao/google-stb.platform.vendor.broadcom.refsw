/***************************************************************************
* Copyright (C) 2008-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/

#include "nexus_types.h"
#include "nexus_base.h"
#include "nexus_platform.h"
#include "nexus_platform_thunks.h"
#include "client/nexus_client_prologue.h"
#include "nexus_platform_client_impl.h"
#include "nexus_platform_local_priv.h"
#include "blst_slist.h"
#include <sys/types.h>
#include <sys/poll.h>
#include <errno.h>
#include <stdlib.h>

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

BDBG_MODULE(nexus_platform_client);

BDBG_OBJECT_ID(NEXUS_P_Client);
BDBG_OBJECT_ID(NEXUS_P_ClientModule);

BDBG_OBJECT_ID(NEXUS_P_ClientCallback);
struct NEXUS_P_ClientCallback
{
    BDBG_OBJECT(NEXUS_P_ClientCallback)
    int fd;
    NEXUS_ThreadHandle thread;
    NEXUS_P_ClientHandle client;
    BKNI_MutexHandle callbackLock; /* held while callback is running */
    unsigned lastStopCallbacksCount;
    struct {
        bool valid;
        NEXUS_Time time;
        struct nexus_callback_data callback_data;
    } current;
};

BDBG_OBJECT_ID_DECLARE(NEXUS_P_Client);
struct NEXUS_P_Client
{
    BDBG_OBJECT(NEXUS_P_Client)
    BKNI_MutexHandle lock;
    int fd; /* main socket for connection to server */
    struct NEXUS_P_ClientCallback callback[NEXUS_ModulePriority_eMax];
    NEXUS_TimerHandle callbackMonitorTimer;
};

BDBG_OBJECT_ID_DECLARE(NEXUS_P_ClientModule);
struct NEXUS_P_ClientModule
{
    BDBG_OBJECT(NEXUS_P_ClientModule)
    NEXUS_P_ClientHandle client;
    unsigned module_id;
    int fd; /* socket file descriptor */
    BKNI_MutexHandle lock; /* allows multithreaded calls on client */
    void *data;
    unsigned data_size; /* size of memory pointed to be data. not the amount of data within that memory. */
};

/* receive and fire callbacks from the server callbacks */
static void NEXUS_P_ClientCallbackThread(void *context)
{
    struct NEXUS_P_ClientCallback *callback = context;
    while (1) {
        struct nexus_callback_data data[NEXUS_CALLBACK_QUEUE_SIZE];
        int rc;
        unsigned i;
        uint8_t num;

        /* listen for callbacks */
        rc = b_nexus_read(callback->fd, data, sizeof(data));
        if (rc < 0 || rc%sizeof(data[0])!=0) {
            /* clean termination. exit the thread without comment. */
            if (callback->client->fd == -1) break;

            /* abnormal termination. exit the process. */
            BDBG_ERR(("Client terminating: %d %d. Check server console for error messages.", rc, errno));
            exit(1);
        }
        num = rc/sizeof(data[0]);
        if (!num) continue;

        rc = b_nexus_write(callback->fd, &num, sizeof(num));
        if (rc != sizeof(num)) {
            BERR_TRACE(rc);
            break;
        }

        /* TEST: add BKNI_Sleep(rand()%50) here to verify StartCallbacks waits until pipeline empty */
        for (i=0;i<num;i++) {
            if (!data[i].interface) {
                BDBG_MSG_TRACE(("received stop callbacks: %d", data[i].callback.param));
                callback->lastStopCallbacksCount = data[i].callback.param;
            }
            else if (data[i].callback.callback) {
                NEXUS_LockModule();
                NEXUS_Time_Get(&callback->current.time);
                callback->current.callback_data = data[i];
                callback->current.valid = true;
                NEXUS_UnlockModule();
                BKNI_AcquireMutex(callback->callbackLock);
                /* TEST: add BKNI_Sleep(rand()%50) here to test proper thread synchronization */
                BDBG_MSG_TRACE(("%p calling: %p(%p,%d)", callback, data[i].callback.callback, data[i].callback.context, data[i].callback.param));
                (data[i].callback.callback)(data[i].callback.context, data[i].callback.param);
                BKNI_ReleaseMutex(callback->callbackLock);
                NEXUS_LockModule();
                callback->current.valid = false;
                NEXUS_UnlockModule();
            }
        }
    }
    return;
}

static void NEXUS_P_ClientCallback_Monitor(unsigned priority, struct NEXUS_P_ClientCallback *callback, unsigned timeout)
{
    if(callback->current.valid) {
        NEXUS_Time now;
        unsigned duration;
        NEXUS_Time_Get(&now);
        duration = NEXUS_Time_Diff(&now, &callback->current.time);
        if (duration > timeout) {
            BDBG_WRN(("stuck callback %d msec, module priority %u callback:%p(%p,%u) interface:%p", duration, priority, (void *)(unsigned long)callback->current.callback_data.callback.callback, (void*)callback->current.callback_data.callback.context, callback->current.callback_data.callback.param, (void*)callback->current.callback_data.interface));
        }
    }
    return;
}


static void NEXUS_P_ClientCallbackMonitor(void *context)
{
    struct NEXUS_P_Client *client = context;
    unsigned timeout = 500;
    unsigned i;

    client->callbackMonitorTimer = NULL;

    for(i=0;i<NEXUS_ModulePriority_eMax;i++) {
        NEXUS_P_BaseCallback_Monitor(i, timeout);
        NEXUS_P_ClientCallback_Monitor(i, &client->callback[i], timeout);
    }
    client->callbackMonitorTimer = NEXUS_ScheduleTimerByPriority(Internal, timeout, NEXUS_P_ClientCallbackMonitor, client);
    return;
}


NEXUS_P_ClientHandle NEXUS_P_Client_Init(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    int rc;
    NEXUS_P_ClientHandle client;
    unsigned i;
    struct nexus_connect_info info;
    uint8_t done;

    NEXUS_LockModule();
    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(client, 0, sizeof(*client));
    BDBG_OBJECT_SET(client, NEXUS_P_Client);
    rc = BKNI_CreateMutex(&client->lock);
    if ( rc ) {
        BKNI_Free(client);
        rc = BERR_TRACE(rc);
        return NULL;
    }

    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct NEXUS_P_ClientCallback *callback = &client->callback[i];
        BDBG_OBJECT_SET(callback, NEXUS_P_ClientCallback);
        callback->fd = -1; /* set proper default in case init goes wrong */
        if(i==NEXUS_ModulePriority_eInternal) {
            continue;
        }
        rc = BKNI_CreateMutex(&callback->callbackLock);
        if ( rc ) {
            rc = BERR_TRACE(rc);
            goto error;
        }
    }

    client->fd = b_nexus_socket_connect(nexus_socket_type_main, 0);
    if (client->fd == -1) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }

    /* send authentication and see if we're accepted */
    info.abi = NEXUS_P_NATIVE_ABI;
    if (pSettings) {
        info.auth = *pSettings;
    } else {
        NEXUS_Platform_GetDefaultClientAuthenticationSettings(&info.auth);
    }

    /* this is some internal, manual IPC */
    rc = b_nexus_write(client->fd, &info, sizeof(info));
    if (rc != sizeof(info)) {
        BDBG_ERR(("unable to send authentication: %d. check server console for error messages.", rc));
        goto error;
    }
    rc = b_nexus_read(client->fd, &done, sizeof(done));
    if (rc != sizeof(done))  {
        BDBG_ERR(("unable to receive authentication response: %d. check server console for error messages.", rc));
        goto error;
    }

    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct NEXUS_P_ClientCallback *callback = &client->callback[i];
        if(i==NEXUS_ModulePriority_eInternal) {
            continue;
        }
        callback->client = client;

        callback->fd = b_nexus_socket_connect(nexus_socket_type_scheduler, i);
        if (callback->fd == -1) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        callback->current.valid = false;

        callback->thread = NEXUS_Thread_Create("nxcallback", NEXUS_P_ClientCallbackThread, callback, NULL);
        if (!callback->thread) {rc = BERR_TRACE(rc); goto error;}
    };
    client->callbackMonitorTimer = NEXUS_ScheduleTimerByPriority(Internal, 100, NEXUS_P_ClientCallbackMonitor, client);
    if(!client->callbackMonitorTimer) { rc=BERR_TRACE(NEXUS_OS_ERROR); goto error; }

    NEXUS_UnlockModule();
    return client;

error:
    NEXUS_UnlockModule();
    NEXUS_P_Client_Uninit(client);
    return NULL;
}

void NEXUS_P_Client_Disconnect(NEXUS_P_ClientHandle client)
{
    BDBG_OBJECT_ASSERT(client, NEXUS_P_Client);
    if (client->fd != -1) {
        int fd = client->fd;
        enum nexus_main_socket_message_type msg;
        
        /* when we close the main socket, the server will close the callback sockets immediately,
        so we have to set client->fd = -1 before the close so that the callback thread knows it's a clean shutdown. */
        client->fd = -1;
        
        msg = nexus_main_socket_message_type_disconnect;
        b_nexus_write(fd, &msg, sizeof(msg));
        
        b_nexus_socket_close(fd);
    }
}

void NEXUS_P_Client_Uninit(NEXUS_P_ClientHandle client)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(client, NEXUS_P_Client);
    NEXUS_LockModule();
    if(client->callbackMonitorTimer) {
        NEXUS_CancelTimer(client->callbackMonitorTimer);
        client->callbackMonitorTimer = NULL;
    }

    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct NEXUS_P_ClientCallback *callback = &client->callback[i];
        if(i==NEXUS_ModulePriority_eInternal) {
            continue;
        }
        if (callback->fd != -1) {
            b_nexus_socket_close(callback->fd);
        }
        if (callback->thread) {
            NEXUS_Thread_Destroy(callback->thread);
        }
        if ( callback->callbackLock)
            BKNI_DestroyMutex(callback->callbackLock);
        BDBG_OBJECT_UNSET(callback, NEXUS_P_ClientCallback);
    }
    if (client->lock) {
        BKNI_DestroyMutex(client->lock);
    }
    NEXUS_UnlockModule();
    BDBG_OBJECT_DESTROY(client, NEXUS_P_Client);
    BKNI_Free(client);
}

NEXUS_P_ClientModuleHandle NEXUS_P_Client_InitModule(NEXUS_P_ClientHandle client, unsigned module_id, unsigned data_size)
{
    int rc;
    NEXUS_P_ClientModuleHandle module;

    module = BKNI_Malloc(sizeof(*module));
    if (!module) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(module, 0, sizeof(*module));
    BDBG_OBJECT_SET(module, NEXUS_P_ClientModule);
    module->module_id = module_id;
    module->fd = -1;
    module->client = client;

    rc = BKNI_CreateMutex(&module->lock);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }

    module->fd = b_nexus_socket_connect(nexus_socket_type_module, module->module_id);
    if (module->fd == -1) {
        rc = NEXUS_UNKNOWN; /* no BERR_TRACE */
        goto error;
    }
    
    module->data_size = data_size;
    if (module->data_size) {
        module->data = BKNI_Malloc(module->data_size);
        if (!module->data) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
    }

    BDBG_MSG(("connect client module %d", module->module_id));
    return module;

error:
    NEXUS_P_Client_UninitModule(module);
    return NULL;
}

void NEXUS_P_Client_UninitModule(NEXUS_P_ClientModuleHandle module)
{
    BDBG_OBJECT_ASSERT(module, NEXUS_P_ClientModule);
    BDBG_MSG(("disconnect client module %d", module->module_id));
    if (module->fd != -1) {
        b_nexus_socket_close(module->fd);
    }
    if (module->data) {
        BKNI_Free(module->data);
    }
    if (module->lock) {
        BKNI_DestroyMutex(module->lock);
    }
    BDBG_OBJECT_DESTROY(module, NEXUS_P_ClientModule);
    BKNI_Free(module);
}

NEXUS_Error
NEXUS_P_Client_LockModule(NEXUS_P_ClientModuleHandle module, void **data, unsigned *size)
{
    NEXUS_Error rc;
    if (!module) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    BDBG_OBJECT_ASSERT(module, NEXUS_P_ClientModule);
    rc = BKNI_AcquireMutex(module->lock);
    if (rc) return BERR_TRACE(rc);
    *data = module->data;
    *size = module->data_size;
    return 0;
}

void
NEXUS_P_Client_UnlockModule(NEXUS_P_ClientModuleHandle module)
{
    BDBG_OBJECT_ASSERT(module, NEXUS_P_ClientModule);
    BKNI_ReleaseMutex(module->lock);
}

#define MAX_IPC_LEN (128*1024) /* Based on MMAP_THRESHOLD */

NEXUS_Error
NEXUS_P_Client_CallServer(NEXUS_P_ClientModuleHandle module, const void *in_params, unsigned in_param_size, void **out_params, unsigned out_param_mem, unsigned *p_out_param_size)
{
    int rc;
    size_t response_length;
    unsigned received;

    BDBG_OBJECT_ASSERT(module, NEXUS_P_ClientModule);

    /* BDBG_MSG_TRACE(("write %d", in_param_size)); */
    /* batom_range_dump(in_params, in_param_size, "SEND"); */
    rc = b_nexus_write(module->fd, in_params, in_param_size);
    if (rc != (int)in_param_size) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    rc = b_nexus_read(module->fd, *out_params, out_param_mem);
    if (rc <= 0) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if(rc<(int)sizeof(NEXUS_Ipc_Header)) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    /* BDBG_MSG_TRACE(("read %d", rc)); */
    /* batom_range_dump(*out_params,rc, "RECV"); */
    received = (unsigned)rc;
    response_length = ((NEXUS_Ipc_Header *)(*out_params))->packet_size;
     if ( response_length == 0 || response_length > MAX_IPC_LEN ) {
         return BERR_TRACE(NEXUS_NOT_SUPPORTED);
     }
    if(received<response_length) {
        void *new_out_param = *out_params;
        if( response_length > out_param_mem ) {
            new_out_param = BKNI_Malloc(response_length);
            if(new_out_param == NULL) {
                return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
            BKNI_Memcpy(new_out_param, *out_params, received);
        }
        while(received < response_length) {
            rc = b_nexus_read(module->fd, (uint8_t *)new_out_param + received, response_length - received);
            if (rc <= 0) {
                if(new_out_param != *out_params) {
                    BKNI_Free(new_out_param);
                }
                return BERR_TRACE(NEXUS_UNKNOWN);
            }
            received += rc;
        }
        *out_params = new_out_param;
    }
    *p_out_param_size = received;

    return 0;
}

void NEXUS_P_Client_StopCallbacks(NEXUS_P_ClientHandle client, void *interfaceHandle)
{
    unsigned i;
    unsigned stopCallbacksCount;
    int rc;
    enum nexus_main_socket_message_type msg;
    
    /* stop callbacks on server and learn stopCallbacksCount to wait for */
    BKNI_AcquireMutex(client->lock);
    msg = nexus_main_socket_message_type_stop_callbacks;
    rc = b_nexus_write(client->fd, &msg, sizeof(msg));
    if (rc != sizeof(msg)) {
        BERR_TRACE(NEXUS_UNKNOWN);
        BKNI_ReleaseMutex(client->lock);
        goto error;
    }
    rc = b_nexus_write(client->fd, &interfaceHandle, sizeof(interfaceHandle));
    if (rc != sizeof(interfaceHandle)) {
        BERR_TRACE(NEXUS_UNKNOWN);
        BKNI_ReleaseMutex(client->lock);
        goto error;
    }
    
    rc = b_nexus_read(client->fd, &stopCallbacksCount, sizeof(stopCallbacksCount));
    if (rc != sizeof(stopCallbacksCount)) {
        BERR_TRACE(NEXUS_UNKNOWN);
        BKNI_ReleaseMutex(client->lock);
        goto error;
    }
    BKNI_ReleaseMutex(client->lock);
    
    /* here all relevant callbacks are marked as 'stopped'. now, we must synchronize with every callback thread
    to ensure this function returns with no stopped callback in flight. */
    while (1) {
        bool synced = true;
        for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
            struct NEXUS_P_ClientCallback *callback = &client->callback[i];
            if(i==NEXUS_ModulePriority_eInternal) {
                continue;
            }
            BDBG_OBJECT_ASSERT(callback, NEXUS_P_ClientCallback);
            BKNI_AcquireMutex(callback->callbackLock);
            if (callback->lastStopCallbacksCount < stopCallbacksCount) {
                synced = false;
            }
            BKNI_ReleaseMutex(callback->callbackLock);
        }
        if (synced) break;
        BKNI_Sleep(1); /* avoid busy loop and unfair scheduling */
    }
    
    return;
    
error:
    BDBG_ERR(("unable to stop callbacks on %p", interfaceHandle));
    return;
}

void NEXUS_P_Client_StartCallbacks(NEXUS_P_ClientHandle client, void *interfaceHandle)
{
    int rc;
    enum nexus_main_socket_message_type msg;
    
    /* start callbacks on server */
    msg = nexus_main_socket_message_type_start_callbacks;
    BKNI_AcquireMutex(client->lock);
    rc = b_nexus_write(client->fd, &msg, sizeof(msg));
    if (rc != sizeof(msg)) {
        BERR_TRACE(NEXUS_UNKNOWN);
        BKNI_ReleaseMutex(client->lock);
        goto error;
    }
    rc = b_nexus_write(client->fd, &interfaceHandle, sizeof(interfaceHandle));
    if (rc != sizeof(interfaceHandle)) {
        BERR_TRACE(NEXUS_UNKNOWN);
        BKNI_ReleaseMutex(client->lock);
        goto error;
    }
    BKNI_ReleaseMutex(client->lock);
    
    return;
    
error:
    BDBG_ERR(("unable to start callbacks on %p", interfaceHandle));
    return;
}

NEXUS_Error NEXUS_P_ClientCall_Begin(NEXUS_P_ClientModuleHandle module, NEXUS_P_ClientCall_State *state, unsigned header)
{
    NEXUS_Error rc;
    state->module = module;
    state->varargs_offset = 0;
    state->header = header;
    rc = NEXUS_P_Client_LockModule(module, &state->data, &state->size);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    BDBG_ASSERT(state->size >= state->varargs_begin);

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_P_ClientCall_Call(NEXUS_P_ClientCall_State *state)
{
    NEXUS_Error rc;
    unsigned out_param_size;
    unsigned packet_size = state->varargs_begin + state->varargs_offset;
    void *out_param = state->data;

    ((NEXUS_Ipc_Header*)(state->data))->packet_size = packet_size;

    rc = NEXUS_P_Client_CallServer(state->module, state->data, packet_size, &out_param, state->size, &out_param_size);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    if(state->data != out_param) {
        if(state->data != state->module->data) {
            BKNI_Free(state->data);
        }
    }
    state->data = out_param;
    state->size = out_param_size;

    return NEXUS_SUCCESS;
}


static void *NEXUS_P_ClientCall_InVarArg_Alloc(NEXUS_P_ClientCall_State *state, unsigned vararg_size)
{
    void *data;
    unsigned aligned_vararg_size = B_IPC_DATA_ALIGN(vararg_size);
    unsigned required_size = state->varargs_offset + state->varargs_begin + aligned_vararg_size;
    if( required_size > state->size) {
        void *buf = BKNI_Malloc(required_size);
        if(buf==NULL) {
            (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memcpy(buf, state->data, state->size);
        state->size = required_size;
        if(state->data != state->module->data) {
            BKNI_Free(state->data);
        }
        state->data = buf;
    }
    data = (uint8_t *)state->data + state->varargs_begin + state->varargs_offset;
    state->varargs_offset += aligned_vararg_size;
    return data;
}


NEXUS_Error NEXUS_P_ClientCall_InVarArg(NEXUS_P_ClientCall_State *state, unsigned vararg_size, const void *src, int *field, bool *is_null)
{
    if(src) {
        void *data;
        void *old_data = state->data;
        *is_null = false; /* assign is_null earlies so don't worry about relocation */
        data = NEXUS_P_ClientCall_InVarArg_Alloc(state, vararg_size);
        if(data==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        /* field is the pointer into the state->data, so adjust it if data was relocated */
        /* freed pointer ('old_data') only used in the pointer math, it is not dereferenced */
        /* coverity[use_after_free: FALSE] */
        field = (void *)((uint8_t *)state->data + ((uint8_t *)field - (uint8_t *)old_data));
        *field = (uint8_t *)data - (uint8_t *)state->data - state->header;
        BKNI_Memcpy(data, src, vararg_size);
    } else {
        *field = -1;
        *is_null = true;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_P_ClientCall_InVarArg_AddrField(NEXUS_P_ClientCall_State *state, unsigned struct_size, unsigned field_offset, unsigned count, int varArg, int *varArgField)
{
    if(varArg!=-1) {
        void *old_data = state->data;
        NEXUS_Addr *dataAddr = NEXUS_P_ClientCall_InVarArg_Alloc(state, count*sizeof(NEXUS_Addr));
        const void *varArgData = (uint8_t *)state->data  + varArg + state->header;
        unsigned i;
        if(dataAddr==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        /* varArgField is the pointer into the state->data, so adjust it if data was relocated */
        /* freed pointer only used in the pointer math, it is not dereferenced */
        /* coverity[use_after_free: FALSE] */
        varArgField = (void *)((uint8_t *)state->data + ((uint8_t *)varArgField - (uint8_t *)old_data));
        *varArgField= (uint8_t *)dataAddr - (uint8_t *)state->data - state->header;
        for(i=0;i<count;i++) {
            void *ptr = *(void **)((uint8_t *)varArgData + i*struct_size + field_offset);
            dataAddr[i] = NEXUS_P_ClientCall_AddrToOffset(ptr);
            BDBG_MSG(("%p NEXUS_P_ClientCall_InVarArg_AddrField%d(%d) %p[%u]=" BDBG_UINT64_FMT "", (void *)state, varArg, *varArgField, ptr, i, BDBG_UINT64_ARG(dataAddr[i])));
        }
    } else {
        *varArgField= -1;
    }
    return NEXUS_SUCCESS;
}


static void *NEXUS_P_ClientCall_OutVarArg_Ptr(NEXUS_P_ClientCall_State *state, unsigned vararg_size, int vararg_offset)
{
    void *data;

    vararg_size = B_IPC_DATA_ALIGN(vararg_size);
    if(vararg_size + vararg_offset + state->header > state->size) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    data = (uint8_t *)state->data + vararg_offset + state->header;
    return data;
}

void NEXUS_P_ClientCall_OutVarArg(NEXUS_P_ClientCall_State *state, unsigned vararg_size, void *dest, int vararg_offset)
{
    if(vararg_offset >= 0) {
        void *src = NEXUS_P_ClientCall_OutVarArg_Ptr(state, vararg_size, vararg_offset);
        if(src && dest) {
            BKNI_Memcpy(dest, src, vararg_size);
        }
    }
    return;
}

void NEXUS_P_ClientCall_End(NEXUS_P_ClientCall_State *state)
{
    if(state->data != state->module->data) {
        BKNI_Free(state->data);
    }
    NEXUS_P_Client_UnlockModule(state->module);
    return;
}

void *NEXUS_P_ClientCall_OffsetToAddr(NEXUS_Addr addr)
{
    if(addr!=0) {
        return NEXUS_OffsetToCachedAddr(addr);
    }
    return NULL;
}

NEXUS_Addr NEXUS_P_ClientCall_AddrToOffset(const void *ptr)
{
    if(ptr!=NULL) {
        NEXUS_Addr addr = NEXUS_AddrToOffset(ptr);
        if(addr==0) {
            (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        return addr;
    }
    return 0;
}
