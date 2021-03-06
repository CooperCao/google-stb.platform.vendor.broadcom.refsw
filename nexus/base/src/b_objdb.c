/***************************************************************************
 *     (c)2004-2014 Broadcom Corporation
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
#include "bstd.h"
#include "bkni.h"
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include "b_objdb.h"

/* b_objdb is a standalone piece of code. it is very important that there is no #include
into some part of a nexus driver. it should have no dependency other than BDBG and NEXUS_ModuleHandle. */

BDBG_MODULE(b_objdb);
BDBG_OBJECT_ID(b_objdb);
BDBG_OBJECT_ID(b_objdb_module);

#define BDBG_MSG_TRACE(X) 

static struct b_objdb_client *b_objdb_default_client = NULL;
/*
b_objdb is the object database. There is one instance of b_objdb per module.
There is one mutex (nexus module handle) per objdb.
*/
struct b_objdb {
    BDBG_OBJECT(b_objdb)
    BLST_S_HEAD(NEXUS_P_ObjDbModuleHead, b_objdb_module) module_list;
    struct NEXUS_P_BaseObjectTree object_tree;
};

static int NEXUS_P_BaseObject_Compare(const struct NEXUS_BaseObject* node, const struct NEXUS_BaseObject *key)
{
    if(key > node) {
        return 1;
    } else if(key==node) {
        return 0;
    } else {
        return -1;
    }
}

BLST_AA_TREE_GENERATE_FIND(NEXUS_P_BaseObjectTree , void *, NEXUS_BaseObject, node, NEXUS_P_BaseObject_Compare)
BLST_AA_TREE_GENERATE_INSERT(NEXUS_P_BaseObjectTree, void *, NEXUS_BaseObject, node, NEXUS_P_BaseObject_Compare)
BLST_AA_TREE_GENERATE_REMOVE(NEXUS_P_BaseObjectTree, NEXUS_BaseObject, node)
BLST_AA_TREE_GENERATE_FIRST(NEXUS_P_BaseObjectTree, NEXUS_BaseObject, node)
BLST_AA_TREE_GENERATE_NEXT(NEXUS_P_BaseObjectTree, NEXUS_BaseObject, node)

static struct b_objdb s_nexus_objdb;

static NEXUS_Error _b_objdb_init(struct b_objdb *db)
{
    BKNI_Memset(db, 0, sizeof(*db));

    BLST_AA_TREE_INIT(NEXUS_P_BaseObjectTree, &db->object_tree);
    BDBG_OBJECT_SET(db, b_objdb);
    return NEXUS_SUCCESS;
}

static void _b_objdb_uninit(struct b_objdb *db)
{
    BDBG_OBJECT_ASSERT(db, b_objdb);
    BDBG_OBJECT_DESTROY(db, b_objdb);
    return;
}

static NEXUS_Error _b_objdb_add_module(struct b_objdb *db, struct b_objdb_module *module,const struct b_objdb_class *class_list, NEXUS_ModuleHandle nexusModule)
{
    BDBG_ASSERT(class_list);
    BDBG_OBJECT_ASSERT(db, b_objdb);
    BDBG_OBJECT_SET(module, b_objdb_module);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    BLST_S_INSERT_HEAD(&db->module_list, module, link);
    module->class_list = class_list;
    module->db = db;
    nexusModule->objdb = module;
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    return NEXUS_SUCCESS;
}

static void _b_objdb_remove_module(struct b_objdb *db, struct b_objdb_module *module, NEXUS_ModuleHandle nexusModule)
{
    BDBG_OBJECT_ASSERT(db, b_objdb);
    BDBG_OBJECT_ASSERT(module, b_objdb_module);
    BDBG_ASSERT(module->db == db);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    module->db = NULL;
    nexusModule->objdb = NULL;
    BLST_S_REMOVE(&db->module_list, module, b_objdb_module, link);
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    BDBG_OBJECT_DESTROY(module, b_objdb_module);
    return;
}

NEXUS_Error b_objdb_init(void)
{
    return _b_objdb_init(&s_nexus_objdb);
}

void b_objdb_uninit(void)
{
    _b_objdb_uninit(&s_nexus_objdb);
    return;
}

NEXUS_Error b_objdb_add_module(struct b_objdb_module *module,const struct b_objdb_class *class_list, NEXUS_ModuleHandle nexusModule)
{
    return _b_objdb_add_module(&s_nexus_objdb, module, class_list, nexusModule);
}

void b_objdb_remove_module(struct b_objdb_module *module, NEXUS_ModuleHandle nexusModule)
{
    _b_objdb_remove_module(&s_nexus_objdb, module, nexusModule);
    return;
}

/* internal destroys may issue duplicate */
#define IS_INTERNAL_DESTROY_OP(operation) ( \
    (operation) == NEXUS_Object_P_RegisterUnregister_eUnregisterDestroy || \
    (operation) == NEXUS_Object_P_RegisterUnregister_eUnregisterClose || \
    (operation) == NEXUS_Object_P_RegisterUnregister_eSyncThunkDestroy)

static int b_objdb_insert(struct b_objdb_module *db, const NEXUS_BaseClassDescriptor *p_class, void *handle, NEXUS_Object_P_RegisterUnregister operation)
{
    NEXUS_BaseObject *base_object;
    bool acquiring = operation==NEXUS_Object_P_RegisterUnregister_eAutoAcquire || operation == NEXUS_Object_P_RegisterUnregister_eRegisterAcquire;
    const struct b_objdb_client *client = b_objdb_get_client();

    BDBG_OBJECT_ASSERT(db, b_objdb_module);
    base_object = (void *)((uint8_t *)handle + p_class->offset);
    BDBG_MSG(("%s %s:%p client=%p %u %p", acquiring?"acquire":"insert", p_class->type_name, (void *)handle, (void *)client, (unsigned)operation, (void *)base_object->state.objdb_class));
    BDBG_ASSERT(db->class_list);
    if (!client) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    BDBG_OBJECT_ASSERT(base_object, NEXUS_BaseObject);
    if(base_object->state.objdb_class==NULL) {
        if(operation!=NEXUS_Object_P_RegisterUnregister_eAutoAcquire) {
            const struct b_objdb_class *objdb_class;
            /* match objdb class information (release/close) with the NEXUS_BaseClassDescriptor */
            for(objdb_class = db->class_list;;objdb_class++) {
                if(objdb_class->base_class == p_class) {
                    BDBG_OBJECT_ASSERT(db->db, b_objdb);
                    base_object->state.objdb_class = objdb_class;
                    if(acquiring) {
                        base_object->state.client = NULL;
                        base_object->state.acquired_client = client;
#if 0
                        if(objdb_class->release==NULL) {
                            BDBG_WRN(("%p:use Acquire for class %s without release", handle, p_class->type_name));
                        }
#endif
                    } else {
                        base_object->state.client = client;
                        base_object->state.acquired_client = NULL;
                    }
                    base_object->state.order = db->order++; /* simplistic order. TODO: add a threshold to do compaction to avoid overflow */
                    base_object->state.insert_operation = operation;
                    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
                    BLST_AA_TREE_INSERT(NEXUS_P_BaseObjectTree, &db->db->object_tree, base_object, base_object);
#if BDBG_DEBUG_BUILD
                    {
                        NEXUS_BaseObject *object = BLST_AA_TREE_FIND(NEXUS_P_BaseObjectTree, &db->db->object_tree, base_object);
                        BDBG_ASSERT(base_object == object);
                    }
#endif
                    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
                    break;
                } else if(objdb_class->base_class==NULL) {
                    BDBG_ERR(("acquiring object of unknown class %s:%p", p_class->type_name, handle));
                    goto error;
                }
            }
        } else {
            BDBG_ERR(("acquiring object that wasn't inserted %s:%p", p_class->type_name, handle));
            goto error;
        }
    } else {
        BDBG_ASSERT(base_object->state.objdb_class);
        if(acquiring) {
            /* Do not warn on double acquire if client matches. */
            if (base_object->state.acquired_client && base_object->state.acquired_client != client) {
                BDBG_WRN(("object already acquired %s:%p by %p, not %p", p_class->type_name, handle, (void *)base_object->state.acquired_client, (void *)client));
                goto error;
            } else {
#if BDBG_DEBUG_BUILD
#if 0
                const struct b_objdb_class *objdb_class;
                /* match objdb class information (release/close) with the NEXUS_BaseClassDescriptor */
                for(objdb_class = db->class_list;;objdb_class++) {
                    if(objdb_class->base_class == p_class) {
                        if(objdb_class->release==NULL) {
                            BDBG_WRN(("%p:use Acquire for class %s without release", handle, p_class->type_name));
                        }
                        break;
                    }
                }
#endif
#endif
                base_object->state.acquired_client = client;
            }
        } else {
            /* Do not warn on double insert. We insert manually, in the proxy and in the syncthunk.
            Because syncthunk was re-added, we can gradually remove manual inserts. When all are
            removed, this warning can return. */
            goto error;
        }
    }
    return 0;
error:
    return -1;
}


static int b_objdb_remove(struct b_objdb_module *db, const NEXUS_BaseClassDescriptor *p_class, void *handle, NEXUS_Object_P_RegisterUnregister operation)
{
    NEXUS_BaseObject *base_object;
    const struct b_objdb_class *objdb_class;
    const struct b_objdb_client *client = b_objdb_get_client();
    bool releasing = operation==NEXUS_Object_P_RegisterUnregister_eAutoRelease || operation == NEXUS_Object_P_RegisterUnregister_eUnregisterRelease;

    BDBG_OBJECT_ASSERT(db, b_objdb_module);
    BDBG_MSG(("%s %s:%p client=%p %u", releasing?"release":"remove", p_class->type_name, (void *)handle, (void *)client, (unsigned)operation));

    /* client may be NULL or b_objdb_default_client in server context; this unifies the local logic. */
    if (client == b_objdb_default_client) client = NULL;

    base_object = (void *)((uint8_t *)handle + p_class->offset);
    BDBG_OBJECT_ASSERT(base_object, NEXUS_BaseObject);
    db->cancel_callbacks_locked(db->cancel_callbacks_context, handle, releasing?client:NULL);
    objdb_class = base_object->state.objdb_class;
    if(objdb_class) {
        if (releasing) {
#if 0
/* Logic for releasing from objdb is complex. Calls may come from client or server. We rely on public API to
prevent unauthorized releases, so this BDBG_ERR is only for internal debug. So compile it out by default. */
            if (base_object->state.acquired_client == client || !client) {
                base_object->state.acquired_client = NULL;
            } else {
                BDBG_ERR(("releasing object that is not acquired %s:%p", p_class->type_name, handle));
            }
#else
            base_object->state.acquired_client = NULL;
#endif
        } else {
#if 0
/* Logic for removing from objdb is complex. Calls may come from client or server, with object's client or acquired_client set or not set.
And because we remove unconditionally, these BDBG_WRN's are only for internal debug. So compile it out by default. */
            if (client==NULL || client->mode < NEXUS_ClientMode_eProtected || base_object->state.client == client || base_object->state.insert_operation == NEXUS_Object_P_RegisterUnregister_eSyncThunkCreate) {
                if (base_object->state.acquired_client && !IS_INTERNAL_DESTROY_OP(operation)) {
                    BDBG_WRN(("removing object which is still acquired %s:%p", p_class->type_name, handle));
                }
            } else {
                BDBG_WRN(("attempting to remove object that is not owned %s:%p", p_class->type_name, handle));
            }
#endif
            base_object->state.client = NULL;
            base_object->state.acquired_client = NULL;
        }
        if(base_object->state.client==NULL && base_object->state.acquired_client==NULL) {
            BDBG_OBJECT_ASSERT(db->db, b_objdb);
            BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
            BLST_AA_TREE_REMOVE(NEXUS_P_BaseObjectTree, &db->db->object_tree, base_object);
            BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
            base_object->state.objdb_class = NULL;
        }
    } else /* objdb_class==NULL */ {
        if(db->class_list) { /* if objdb was activated */
            if(!releasing) {
                /* if we manually REGISTER/UNREGISTER, then auto-remove will unregister and we'll 
                get a second unregister, but this should only happen for eUnregisterDestroy */
                if (!IS_INTERNAL_DESTROY_OP(operation)) {
                    BDBG_ERR(("removing unknown object %s:%p", p_class->type_name, handle));
                }
                goto error;
            }
        }
    }
    return 0;
error:
    return -1;
}

static int  b_objdb_verify_object_locked(const struct b_objdb *db, const NEXUS_BaseClassDescriptor *p_class, void *handle, const struct b_objdb_client *client)
{
    NEXUS_BaseObject *base_object;
    NEXUS_BaseObject *base_object_handle;

    BDBG_OBJECT_ASSERT(db, b_objdb);

    BDBG_MSG_TRACE(("verify: %s:%p client=%#x", p_class->type_name, handle, client));
    BDBG_ASSERT(client);

    base_object_handle = (void *)((uint8_t *)handle + p_class->offset); /* this object not verified and could be invalid */
    base_object = BLST_AA_TREE_FIND(NEXUS_P_BaseObjectTree, &db->object_tree, base_object_handle);
    if(base_object) {
        BDBG_ASSERT(base_object == base_object_handle); /* verify that we got the same thing */
        if(base_object->descriptor == p_class) {
            if (client) {
                if (client->mode < NEXUS_ClientMode_eProtected) {
                    /* don't need to check ownership. It exists, so it's good. */
                    goto done;
                }
                /* if you are the owner or have it acquired, you are allowed to use this handle */
                if (base_object->state.client == client || base_object->state.acquired_client == client || base_object->state.shared) {
                    goto done;
                }
            } else {
                /* no client check. The handle/type exists, so we pass. */
                goto done;
            }
        }
    }
    BDBG_WRN(("unknown object: %s:%p", p_class->type_name, handle));
    return -1;
done:
    BDBG_MSG_TRACE(("verify: %s:%p client=%#x success", p_class->type_name, handle, client));
    return 0;
}

NEXUS_Error b_objdb_verify_any_object(void *handle)
{
    NEXUS_Error rc;
    struct b_objdb *db = &s_nexus_objdb;
    NEXUS_BaseObject *base_object_handle = handle;
    const struct b_objdb_client *client = b_objdb_get_client();
    NEXUS_BaseObject *base_object;

    if (client==NULL || client->mode < NEXUS_ClientMode_eProtected) {
        return NEXUS_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(db, b_objdb);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    base_object = BLST_AA_TREE_FIND(NEXUS_P_BaseObjectTree, &db->object_tree, base_object_handle);
    if(base_object) {
        if (base_object->state.client == client || base_object->state.acquired_client == client || base_object->state.shared) {
            rc = NEXUS_SUCCESS; /* if you are the owner or have it acquired, you are allowed to use this handle */
        } else {
            rc = NEXUS_INVALID_PARAMETER;
        }
    } else {
        rc = NEXUS_INVALID_PARAMETER;
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    return rc;
}

int  b_objdb_verify_and_acquire(const NEXUS_BaseClassDescriptor *p_class, void *handle)
{
    int rc;
    const struct b_objdb_client *client = b_objdb_get_client();
    struct b_objdb *db = &s_nexus_objdb;

    if (client==NULL) {
        return 0;
    }

    BDBG_OBJECT_ASSERT(db, b_objdb);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    rc = b_objdb_verify_object_locked(db, p_class, handle, client);
    if(rc==0) {
        NEXUS_BaseObject_P_AcquireBaseObject(handle);
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    return rc;
}

static int b_objdb_module_get_newest_entry_locked(struct b_objdb_module *db_module, const struct b_objdb_client *client, NEXUS_BaseObject **object)
{
    struct b_objdb *db;
    NEXUS_BaseObject *newest=NULL;
    int rc = -1;
    NEXUS_BaseObject *base_object;

    BDBG_OBJECT_ASSERT(db_module, b_objdb_module);
    db = db_module->db;
    BDBG_OBJECT_ASSERT(db, b_objdb);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);

    for(base_object=BLST_AA_TREE_FIRST(NEXUS_P_BaseObjectTree, &db->object_tree);
        base_object;
        base_object=BLST_AA_TREE_NEXT(NEXUS_P_BaseObjectTree, &db->object_tree, base_object)) {
        const struct b_objdb_class *objdb_class;
        BDBG_OBJECT_ASSERT(base_object, NEXUS_BaseObject);
        BDBG_ASSERT(base_object->state.objdb_class);

        /* use match in objdb class to filter per-module objects */
        for(objdb_class = db_module->class_list;objdb_class->base_class!=NULL;objdb_class++) {
            if(base_object->state.objdb_class == objdb_class) {
                break;
            }
        }
        if(objdb_class->base_class == NULL) {
            continue; /* object from another module */
        }
        if (base_object->state.client != client && base_object->state.acquired_client != client) {
            continue;
        }
        if (newest==NULL || newest->state.order < base_object->state.order ) { /* find entry with maximum order */
            newest = base_object;
        }
    }
    if (newest) {
        *object = newest;
        rc = 0;
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);

    return rc;
}

static void b_objdb_module_uninit_entry_locked(struct b_objdb_module *db, NEXUS_BaseObject *base_object, const struct b_objdb_client *client)
{
    void *handle;
    const struct b_objdb_class *objdb_class;
    const NEXUS_BaseClassDescriptor *descriptor;

    BDBG_OBJECT_ASSERT(db, b_objdb_module);

    BDBG_OBJECT_ASSERT(base_object, NEXUS_BaseObject);
    descriptor = base_object->descriptor;
    objdb_class = base_object->state.objdb_class;
    handle = (void *)((uint8_t *)base_object - descriptor->offset); /* this object not verified and could be invalid */
    BDBG_ASSERT(objdb_class);
    if (base_object->state.acquired_client == client) {
        /* if it was acquired, it must have a release function registered */
        BDBG_MSG(("auto-release: [order %u] %s:%p client=%p", base_object->state.order, descriptor->type_name, (void *)handle, (void *)client));
        db->cancel_callbacks_locked(db->cancel_callbacks_context, handle, client);
        if(objdb_class->release) {
            objdb_class->release(handle);
        }
        base_object->state.acquired_client = NULL;
    }
    if (base_object->state.client == client) {
        BDBG_MSG(("auto-remove: [order %u] %s:%p client=%p", base_object->state.order, descriptor->type_name, (void *)handle, (void *)client));
        /* remove from database prior to calling destructor */
        BDBG_OBJECT_ASSERT(db->db, b_objdb);
        BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
        BLST_AA_TREE_REMOVE(NEXUS_P_BaseObjectTree, &db->db->object_tree, base_object);
        BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
        base_object->state.objdb_class = NULL;
        db->cancel_callbacks_locked(db->cancel_callbacks_context, handle, NULL);
        /* TODO: consider if only NEXUS_Object_P_RegisterUnregister_eAutoCreate is allowed to call destructor */
        if (base_object->state.insert_operation != NEXUS_Object_P_RegisterUnregister_eSyncThunkCreate) {
            objdb_class->destructor(handle);
        }
    }
    return;
}

void b_objdb_module_uninit_client_objects(struct b_objdb_module *db, struct b_objdb_client *client)
{
    struct b_objdb_client *temp_client = NULL;
    NEXUS_P_ThreadInfo *info = NEXUS_P_ThreadInfo_Get();

    /* we need to temporarily change the client to the one being cleaned up. */
    if (info) {
        temp_client = info->client;
        info->client = client;
    }
    NEXUS_Module_Lock(db->module);
    for(;;) {
        NEXUS_BaseObject *object=NULL;
        if (!b_objdb_module_get_newest_entry_locked(db, client, &object)) {
            b_objdb_module_uninit_entry_locked(db, object, client);
        }
        else {
            break;
        }
    }
    NEXUS_Module_Unlock(db->module);
    if (info) {
        info->client = temp_client;
    }
    return;
}

void b_objdb_module_uninit_client_callbacks(struct b_objdb_module *db, const struct b_objdb_client *client)
{
    BDBG_OBJECT_ASSERT(db, b_objdb_module);
    NEXUS_Module_Lock(db->module);
    (db->cancel_callbacks_locked)(db->cancel_callbacks_context, NULL, (void*)client);
    NEXUS_Module_Unlock(db->module);
    return;
}


void b_objdb_set_default_client(struct b_objdb_client *client)
{
    b_objdb_default_client = client;
    return;
}

void b_objdb_set_client(struct b_objdb_client *client)
{
    NEXUS_P_ThreadInfo *info = NEXUS_P_ThreadInfo_Get();
    if(info) {
        info->client = client;
    }
}

const struct b_objdb_client *b_objdb_get_client(void)
{
    NEXUS_P_ThreadInfo *info = NEXUS_P_ThreadInfo_Get();
    if(info && info->client) {
        return info->client;
    }
    return b_objdb_default_client;
}

NEXUS_Error NEXUS_BaseObject_P_RegisterUnregister(void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_Object_P_RegisterUnregister operation, NEXUS_ModuleHandle origin, NEXUS_ModuleHandle destination)
{
    NEXUS_Error rc=NEXUS_SUCCESS;

    if(origin==destination) {
        BDBG_ASSERT(NEXUS_Module_Assert(origin));
    } else {
        NEXUS_Module_Lock(destination);
    }
    if(destination->objdb) {
        switch(operation) {
            case NEXUS_Object_P_RegisterUnregister_eRegisterOpen:
            case NEXUS_Object_P_RegisterUnregister_eRegisterCreate:
            case NEXUS_Object_P_RegisterUnregister_eRegisterAcquire:
            case NEXUS_Object_P_RegisterUnregister_eAutoCreate:
            case NEXUS_Object_P_RegisterUnregister_eAutoAcquire:
            case NEXUS_Object_P_RegisterUnregister_eSyncThunkCreate:
                rc = b_objdb_insert(destination->objdb, descriptor, object, operation);
                break;
            case NEXUS_Object_P_RegisterUnregister_eUnregisterClose:
            case NEXUS_Object_P_RegisterUnregister_eUnregisterDestroy:
            case NEXUS_Object_P_RegisterUnregister_eUnregisterRelease:
            case NEXUS_Object_P_RegisterUnregister_eAutoDestroy:
            case NEXUS_Object_P_RegisterUnregister_eAutoRelease:
            case NEXUS_Object_P_RegisterUnregister_eSyncThunkDestroy:
                rc = b_objdb_remove(destination->objdb, descriptor, object, operation);
                break;
            default:
                BDBG_ASSERT(0);
                break;
        }
    }
    
    if(origin!=destination) {
        NEXUS_Module_Unlock(destination);
    }
    return rc;
}

NEXUS_BaseObject *b_objdb_find_object_and_acquire(const struct b_objdb_client *client, const char *type_name, void *object)
{
    struct b_objdb *db = &s_nexus_objdb;
    NEXUS_BaseObject *base_object;
    NEXUS_BaseObject *base_object_handle = object;
    NEXUS_BaseObject *result = NULL;

    BDBG_OBJECT_ASSERT(db, b_objdb);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    base_object = BLST_AA_TREE_FIND(NEXUS_P_BaseObjectTree, &db->object_tree, base_object_handle);
    if(base_object) {
        if(NEXUS_P_Base_StrCmp(base_object->descriptor->type_name, type_name)==0) {
            if(client->mode < NEXUS_ClientMode_eProtected || (base_object->state.client == client || base_object->state.acquired_client==client || base_object->state.shared)) {
                NEXUS_BaseObject_P_AcquireBaseObject(base_object);
                result = base_object;
            }
        }
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    return result;
}

NEXUS_Error b_objdb_set_object_shared(const struct b_objdb_client *client, void *handle, bool shared)
{
    struct b_objdb *db = &s_nexus_objdb;
    NEXUS_BaseObject *base_object;
    NEXUS_BaseObject *base_object_handle = handle;
    NEXUS_Error result = NEXUS_INVALID_PARAMETER;

    BDBG_OBJECT_ASSERT(db, b_objdb);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    base_object = BLST_AA_TREE_FIND(NEXUS_P_BaseObjectTree, &db->object_tree, base_object_handle);
    if(base_object) {
        if (base_object->state.client == client) {
            base_object->state.shared = shared;
            result = NEXUS_SUCCESS;
        }
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    return result;
}

b_objdb_get_object_list_result b_objdb_get_object_list(const struct b_objdb_client *calling_client, const struct b_objdb_client *owner,
    const char *type_name, b_objdb_object_instance *objects, size_t nobjects, size_t *validObjects)
{
    unsigned n;
    b_objdb_get_object_list_result rc;
    struct b_objdb *db = &s_nexus_objdb;
    NEXUS_BaseObject *object;

    BDBG_OBJECT_ASSERT(db, b_objdb);
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    for(rc=b_objdb_get_object_list_result_no_objects, n=0,object=BLST_AA_TREE_FIRST(NEXUS_P_BaseObjectTree, &db->object_tree);object;object=BLST_AA_TREE_NEXT(NEXUS_P_BaseObjectTree, &db->object_tree, object)) {
        if(NEXUS_P_Base_StrCmp(object->descriptor->type_name, type_name)!=0) {
            continue;
        }
        if (owner && object->state.client != owner && object->state.acquired_client != owner) {
            continue;
        }
        if (calling_client->mode < NEXUS_ClientMode_eProtected || object->state.client == calling_client || object->state.acquired_client == calling_client) {
            if(n<nobjects) {
                if (objects) {
                    objects[n].object = (uint8_t *)object - object->descriptor->offset;
                    objects[n].readOnly = false;
                }
                rc = b_objdb_get_object_list_result_done;
            }
            else if (!nobjects) {
                rc = b_objdb_get_object_list_result_done;
            }
            else {
                rc = b_objdb_get_object_list_result_overflow;
            }
            n++;
            *validObjects = n;
        }
    }
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    return rc;
}
