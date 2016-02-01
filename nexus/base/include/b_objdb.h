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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ************************************************************/
#ifndef B_OBJDB_H__
#define B_OBJDB_H__

/**
object database

this api is internal to nexus.
used by nexus kernel-mode proxy and user-mode multiprocess thunks.
it uses static allocation and transparent structs, not dynamic allocation and opaque handles.
**/

struct b_objdb_client_default_heaps
{
    NEXUS_HeapHandle heap[NEXUS_DefaultHeapType_eMax];
};

/* the address of the b_objdb_client instance is the id for the client */
struct b_objdb_client
{
    NEXUS_ClientMode mode;
    struct {
        NEXUS_ClientResources allowed;
        NEXUS_ClientResources used;
    } resources;
    struct b_objdb_client_default_heaps default_heaps;
    NEXUS_ClientConfiguration config;
};

BDBG_OBJECT_ID_DECLARE(b_objdb);

struct b_objdb_class {
    const NEXUS_BaseClassDescriptor *base_class; /* used as a key to associate different information aboute the same object type (class) */
    void (*release)(void *); /* called when server/driver wants to release (dissasociate) object from a client/process */
    void (*destructor)(void *); /* called when server/driver wants to close object, it may not actually close object if there are further references to it */
};

struct b_objdb;

struct b_objdb_module {
    BDBG_OBJECT(b_objdb_module)
    BLST_S_ENTRY(b_objdb_module) link;
    NEXUS_ModuleHandle module;
    unsigned order; /* ensure that objects in a module are auto-removed in reverse order of how they were inserted */
    const struct b_objdb_class *class_list;
    const char *name; /* module name for debug */
    /* callback to cancel callbacks for a handle */
    void (*cancel_callbacks_locked)(void *context, void *handle, const struct b_objdb_client *client); /* called with module lock already acquired */
    void *cancel_callbacks_context;
    struct b_objdb *db; /* link back to database */
};



typedef void (*b_objdb_callback)(void *handle);


#define B_OBJDB_TABLE_BEGIN(NAME) static const struct b_objdb_class NAME[] = {
#define B_OBJDB_TABLE_END {NULL,NULL,NULL}};
#define B_OBJDB_TABLE_ENTRY(handletype,releasefunc,destructorfunc) {& handletype##_BaseObject_Descriptor, (void (*)(void *))releasefunc, (void (*)(void *))destructorfunc},

/* initialize global object data base */
NEXUS_Error b_objdb_init(void);
void b_objdb_uninit(void);

/* b_objdb_init will hook up the statically-declared class_table. the caller must still manually
set db->module, name, cancel_callbacks and cancel_callbacks_context */
NEXUS_Error b_objdb_add_module(struct b_objdb_module *module,const struct b_objdb_class *class_list, NEXUS_ModuleHandle nexusModule);
void b_objdb_remove_module(struct b_objdb_module *module, NEXUS_ModuleHandle nexusModule);

int  b_objdb_verify_and_acquire(const NEXUS_BaseClassDescriptor *p_class, void *handle);
NEXUS_Error b_objdb_verify_any_object(void *handle);
NEXUS_Error b_objdb_set_object_shared(const struct b_objdb_client *client, void *handle, bool shared);

/* uninit all objects for this client in this db */
void b_objdb_module_uninit_client_objects(struct b_objdb_module *db, struct b_objdb_client *client);

/* uninit all callbacks for this client */
void b_objdb_module_uninit_client_callbacks(struct b_objdb_module *db, const struct b_objdb_client *client);

/* this function sets default('server') client */
void b_objdb_set_default_client(struct b_objdb_client *client);

/* this function sets client _for_ a current process/thread */
void b_objdb_set_client(struct b_objdb_client *client);

/* this function return client _for_ a current process/thread or a default ('server') client if there is no active client set */
const struct b_objdb_client *b_objdb_get_client(void);

NEXUS_BaseObject *b_objdb_find_object_and_acquire(const struct b_objdb_client *client, const char *type_name, void *object);

typedef struct b_objdb_object_instance {
    bool readOnly; 
    void *object;
} b_objdb_object_instance;

typedef enum  b_objdb_get_object_list_result {
    b_objdb_get_object_list_result_done,
    b_objdb_get_object_list_result_overflow,
    b_objdb_get_object_list_result_no_objects
} b_objdb_get_object_list_result;

b_objdb_get_object_list_result b_objdb_get_object_list(const struct b_objdb_client *calling_client, const struct b_objdb_client *owner,
    const char *type_name, b_objdb_object_instance *objects, size_t nobjects, size_t *validObjects);

struct api_object_descriptor {
    const char *name;
    uint16_t offset;
    int16_t null_offset; /* -1 means that it's always not NULL */
    bool inparam;
    bool null_allowed;
    const NEXUS_BaseClassDescriptor *descriptor;
};

struct api_pointer_descriptor {
    const char *name;
    uint16_t offset;
    int16_t null_offset;
    bool inparam;
    bool null_allowed;
};

struct api_function_descriptor {
    const char *name;
#if !defined(NEXUS_MODE_driver)
    uint16_t in_buf_size;
    uint16_t out_buf_size;
#endif
    bool limited;
    uint8_t object_cnt;
    uint8_t pointer_cnt;
    const struct api_object_descriptor *objects;
    const struct api_pointer_descriptor *pointers;
};

NEXUS_Error nexus_p_api_call_verify(const struct b_objdb_client *client, NEXUS_ModuleHandle module,const struct api_function_descriptor *function, void *in_data
#if !defined(NEXUS_MODE_driver)
        , unsigned in_data_size, unsigned out_mem_size
#endif
        );
void nexus_p_api_call_completed(const struct b_objdb_client *client, NEXUS_ModuleHandle module, const struct api_function_descriptor *function, void *in_data
#if !defined(NEXUS_MODE_driver)
        , void *out_data
#endif
        );
void nexus_driver_shutdown_NEXUS_VideoInput_Shutdown(void *object);
void nexus_driver_shutdown_NEXUS_VideoOutput_Shutdown(void *object);
void nexus_driver_shutdown_NEXUS_AudioInput_Shutdown(void *object);
void nexus_driver_shutdown_NEXUS_AudioOutput_Shutdown(void *object);



#endif /* B_OBJDB_H__ */
