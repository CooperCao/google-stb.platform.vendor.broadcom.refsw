/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2012-2016 Broadcom. All rights reserved.
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
#ifndef NEXUS_BASE_OBJECT_H
#define NEXUS_BASE_OBJECT_H

#include "blst_list.h"
#include "blst_aa_tree.h"

/* there is should be no reason to disable reference counting */
#define NEXUS_P_BASE_OBJECT_USE_REFCNT  1

BLST_AA_TREE_HEAD(NEXUS_P_BaseObjectTree, NEXUS_BaseObject);
BLST_AA_TREE_HEAD(NEXUS_P_BaseObjectIdTree, NEXUS_BaseObject);

typedef struct NEXUS_BaseClassDescriptor {
    unsigned offset;  /* used to convert from the NEXUS_BaseClass to the object  */
    const char *type_name; /* handle type name */
#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
    void (*finalizer)(void *); /* called when there is no further references to an object */
#endif
} NEXUS_BaseClassDescriptor;

/**
Summary:
Datatype embedded into the objects that support reference counting 

See Also:
NEXUS_OBJECT_INIT
**/

struct b_objdb_client;
struct b_objdb_class;

typedef enum NEXUS_Object_P_RegisterUnregister {
    NEXUS_Object_P_RegisterUnregister_eRegisterOpen, /* eRegister used by NEXUS_OBJECT_REGISTER */
    NEXUS_Object_P_RegisterUnregister_eRegisterCreate,
    NEXUS_Object_P_RegisterUnregister_eRegisterAcquire,
    NEXUS_Object_P_RegisterUnregister_eAutoCreate, /* eAuto used by proxy */
    NEXUS_Object_P_RegisterUnregister_eAutoAcquire,
    NEXUS_Object_P_RegisterUnregister_eUnregisterClose,
    NEXUS_Object_P_RegisterUnregister_eUnregisterDestroy,
    NEXUS_Object_P_RegisterUnregister_eUnregisterRelease,
    NEXUS_Object_P_RegisterUnregister_eAutoDestroy,
    NEXUS_Object_P_RegisterUnregister_eAutoRelease,
    NEXUS_Object_P_RegisterUnregister_eSyncThunkCreate, /* eSyncThunk used by nexus/build/tools/syncthunk */
    NEXUS_Object_P_RegisterUnregister_eSyncThunkDestroy,
    NEXUS_Object_P_RegisterUnregister_eMax
} NEXUS_Object_P_RegisterUnregister;

BDBG_OBJECT_ID_DECLARE(NEXUS_BaseObject);
typedef unsigned NEXUS_BaseObjectId;

typedef struct NEXUS_BaseObject {
    BDBG_OBJECT(NEXUS_BaseObject)
    BLST_AA_TREE_ENTRY(NEXUS_P_BaseObjectTree) node;
#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
    NEXUS_ModuleHandle module; /* module that have created this object, calls to finalizer must be done with acquired module lock */
    int ref_cnt;
#endif
    const NEXUS_BaseClassDescriptor *descriptor;
    NEXUS_BaseObjectId id; /* each object has an unique integer ID, this ID used as the object handle by 32-bit code, if 'driver' is 64-bit */
    BLST_AA_TREE_ENTRY(NEXUS_P_BaseObjectIdTree) nodeId;
    struct {
        const struct b_objdb_client *client; /* each object is owned by one client. destroy must match. verify must match client or acquired_client. */
        const struct b_objdb_client *acquired_client; /* each object can be acquired by one client. release must match. verify must match client or acquired_client. */
        bool shared; /* if true, bypass client ownership check */
        unsigned order; /* persistent order, used to destroy objects in exact opposite of create */
        const struct b_objdb_class *objdb_class;
        NEXUS_Object_P_RegisterUnregister insert_operation;
    } state;
} NEXUS_BaseObject;

void NEXUS_BaseObject_P_Init(NEXUS_BaseObject *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle object_module);
#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
void NEXUS_BaseObject_P_AcquireBaseObject(NEXUS_BaseObject *base_object);
#if BDBG_DEBUG_BUILD
void NEXUS_BaseObject_P_Acquire_Tagged(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module, const char *pFileName, unsigned lineNumber);
void NEXUS_BaseObject_P_Release_Tagged(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module, const char *pFileName, unsigned lineNumber);
#else
void NEXUS_BaseObject_P_Acquire(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module);
void NEXUS_BaseObject_P_Release(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module);
#endif
#else
#define NEXUS_BaseObject_P_AcquireBaseObject(base_object)
#define NEXUS_BaseObject_P_Acquire_Tagged(owner, object, descriptor, module, pFileName, lineNumber) (void)(module),(void)(object),(void)pFileName,(void)lineNumber
#define NEXUS_BaseObject_P_Release_Tagged(owner, object, descriptor, module, pFileName, lineNumber) (void)(module),(void)(object),(void)pFileName,(void)lineNumber
#endif

#define NEXUS_OBJECT(type) NEXUS_BaseObject _n_baseObject; BDBG_OBJECT(type) int _n_unused
#define NEXUS_OBJECT_ASSERT(type, handle) BDBG_OBJECT_ASSERT(handle, type)

#define NEXUS_OBJECT_DESCRIPTOR(type) type##_BaseObject_Descriptor

#define NEXUS_CASSERT_DECL(expr) extern const char nexus_p_cassert_decl[(expr)?1:-1]

/**
Summary:
Declares functions that are part of implementation for object reference counting.

Description:
NEXUS_OBJECT_CLASS_DECLARE should be placed into the header file inside the modules 'priv' directory.

See Also:
NEXUS_OBJECT_CLASS_MAKE
**/

#define NEXUS_OBJECT_CLASS_DECLARE(type) \
    BDBG_OBJECT_ID_DECLARE(type); \
    extern const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR(type); \
    NEXUS_Error type##_BaseObject_P_RegisterUnregister(type##Handle object, NEXUS_Object_P_RegisterUnregister operation, NEXUS_ModuleHandle origin)

#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
#define NEXUS_OBJECT_P_CLASS_MAKE_TAIL(type, destructor) \
    static void type##_P_Finalizer(type##Handle object); \
    static void type##_Obj_P_Finalizer(void *object) {NEXUS_ASSERT_MODULE(); type##_P_Finalizer(object); return;} \
    NEXUS_CASSERT_DECL(NEXUS_OFFSETOF(struct type, _n_baseObject)==0); \
    const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR(type) = {NEXUS_OFFSETOF(struct type, _n_baseObject), #type, type##_Obj_P_Finalizer}; \
    NEXUS_Error type##_BaseObject_P_RegisterUnregister(type##Handle object, NEXUS_Object_P_RegisterUnregister operation, NEXUS_ModuleHandle origin) {BDBG_OBJECT_ASSERT(object, type); return NEXUS_BaseObject_P_RegisterUnregister(object, & NEXUS_OBJECT_DESCRIPTOR(type), operation, origin, NEXUS_MODULE_SELF);} \
    BDBG_OBJECT_ID(type)
#else
#define NEXUS_OBJECT_P_CLASS_MAKE_TAIL(type, destructor) \
    static void type##_P_Finalizer(type##Handle object); \
    NEXUS_CASSERT_DECL(NEXUS_OFFSETOF(struct type, _n_baseObject)==0); \
    const NEXUS_BaseClassDescriptor NEXUS_OBJECT_DESCRIPTOR(type) = {NEXUS_OFFSETOF(struct type, _n_baseObject), #type}; \
    NEXUS_Error type##_BaseObject_P_RegisterUnregister(type##Handle object, NEXUS_Object_P_RegisterUnregister operation, NEXUS_ModuleHandle origin) {BDBG_OBJECT_ASSERT(object, type); return NEXUS_BaseObject_P_RegisterUnregister(object, & NEXUS_OBJECT_DESCRIPTOR(type), operation, origin, NEXUS_MODULE_SELF);} \
    BDBG_OBJECT_ID(type)
#endif

/**
Summary:
Defines functions and data that are part of implementation for object reference counting.

Description:
NEXUS_OBJECT_CLASS_MAKE should be placed into C file that is a part of modules implementation

See Also:
NEXUS_OBJECT_CLASS_DECLARE
**/

#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
#define NEXUS_OBJECT_CLASS_MAKE(type, destructor) \
    void destructor(type##Handle object) {NEXUS_OBJECT_RELEASE(object, type, object);} \
    NEXUS_OBJECT_P_CLASS_MAKE_TAIL(type, destructor)

#define NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(type, destructor) \
    void destructor(type##Handle object) {type##_P_Release(object);NEXUS_OBJECT_RELEASE(object, type, object);} \
    static void type##_P_Release(type##Handle object); \
    NEXUS_OBJECT_P_CLASS_MAKE_TAIL(type, destructor)
#else
#define NEXUS_OBJECT_CLASS_MAKE(type, destructor) \
    void destructor(type##Handle object) {type##_P_Finalizer(object); return;} \
    NEXUS_OBJECT_P_CLASS_MAKE_TAIL(type, destructor)
#define NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(type, destructor) \
    void destructor(type##Handle object) {type##_P_Release(object);type##_P_Finalizer(object); return;}\
    static void type##_P_Release(type##Handle object); \
    NEXUS_OBJECT_P_CLASS_MAKE_TAIL(type, destructor)
#endif

/**
Summary:
This macro should be used to initialize reference counted object

Description:
When intializes reference counter is set to 1, and used of NEXUS_OBJECT_RELEASE would cause call to the object finalizer (private destructor).
To properly support reference counting, public destructor/close API should be reduced just to call into the NEXUS_OBJECT_RELEASE, 
and proper destructor (private destructor) should be linked by means of NEXUS_OBJECT_CLASS_MAKE.

See Also:
NEXUS_OBJECT_ACQUIRE
NEXUS_OBJECT_RELEASE
**/
#define NEXUS_OBJECT_INIT(type, obj) do { NEXUS_ASSERT_MODULE(); BKNI_Memset((obj), 0, sizeof(*(obj)));BDBG_OBJECT_SET(obj,type);NEXUS_BaseObject_P_Init(&((obj)->_n_baseObject), & NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_MODULE_SELF);}while(0)

#define NEXUS_OBJECT_SET(type, obj) do { NEXUS_ASSERT_MODULE(); BDBG_OBJECT_SET(obj,type);NEXUS_BaseObject_P_Init(&((obj)->_n_baseObject), & NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_MODULE_SELF);}while(0)
#define NEXUS_OBJECT_UNSET(type, obj) BDBG_OBJECT_UNSET(obj,type)
#define NEXUS_OBJECT_DESTROY(type,obj) BDBG_OBJECT_DESTROY(obj,type) 
#define NEXUS_OBJECT_CLEAR(type,obj) do { BDBG_OBJECT_UNSET(obj,type); BKNI_Memset((obj), 0, sizeof(*(obj)));} while(0)


/**
Summary:
This macro is used to inform that object of said type is used and can't be destroyed

Description:
**/
#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
#if BDBG_DEBUG_BUILD
#define NEXUS_OBJECT_ACQUIRE(owner,type,object) NEXUS_BaseObject_P_Acquire_Tagged(&(owner)->_n_baseObject, (object), & NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_MODULE_SELF, BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_OBJECT_ACQUIRE(owner,type,object) NEXUS_BaseObject_P_Acquire(&(owner)->_n_baseObject, (object), & NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_MODULE_SELF)
#endif
#else
#define NEXUS_OBJECT_ACQUIRE(owner,type,object) (void)(object)
#endif

/**
Summary:
This macro is used to inform that object of said type is unused and can be safely destroyed

Description:
**/
#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
#if BDBG_DEBUG_BUILD
#define NEXUS_OBJECT_RELEASE(owner,type,object) NEXUS_BaseObject_P_Release_Tagged(&(owner)->_n_baseObject, (object), & NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_MODULE_SELF, BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_OBJECT_RELEASE(owner,type,object) NEXUS_BaseObject_P_Release(&(owner)->_n_baseObject, (object), & NEXUS_OBJECT_DESCRIPTOR(type), NEXUS_MODULE_SELF)
#endif
#else
#define NEXUS_OBJECT_RELEASE(owner,type,object) (void)(object)
#endif

NEXUS_Error NEXUS_BaseObject_P_RegisterUnregister(void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_Object_P_RegisterUnregister operation, NEXUS_ModuleHandle origin, NEXUS_ModuleHandle destination);
#define NEXUS_OBJECT_REGISTER(type, object, operation) type##_BaseObject_P_RegisterUnregister(object, NEXUS_Object_P_RegisterUnregister_eRegister##operation, NEXUS_MODULE_SELF)
#define NEXUS_OBJECT_UNREGISTER(type, object, operation) type##_BaseObject_P_RegisterUnregister(object, NEXUS_Object_P_RegisterUnregister_eUnregister##operation, NEXUS_MODULE_SELF)
NEXUS_BaseObject *NEXUS_BaseObject_FromId(NEXUS_BaseObjectId id);

#endif /* !defined NEXUS_BASE_OBJECT_H */

