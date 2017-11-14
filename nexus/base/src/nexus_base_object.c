/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include "nexus_base_os_types.h"
#include "bkni_multi.h"

BDBG_MODULE(nexus_base_object);
#if 0
/* Replace "NEXUS_SomeObject" with object to trace */
#define NEXUS_P_TRACE_OBJECT(object) (NEXUS_P_Base_StrCmp((object)->descriptor->type_name,"NEXUS_SomeObject")==0)
#endif

#if defined(NEXUS_P_TRACE_OBJECT)
#define BDBG_MSG_OBJECT(o,x) do { if(NEXUS_P_TRACE_OBJECT(o)) {BDBG_LOG(x);} else { BDBG_MSG(x);}} while(0)
#else
#define BDBG_MSG_OBJECT(o,x) BDBG_MSG(x)
#endif

BDBG_OBJECT_ID(NEXUS_BaseObject);

void NEXUS_BaseObject_P_Init(NEXUS_BaseObject *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle object_module)
{
    BDBG_ASSERT(object);
    BDBG_ASSERT(descriptor);
    BDBG_OBJECT_INIT(object, NEXUS_BaseObject);
#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
    object->ref_cnt = 1;
    object->module = object_module;
#else
    BSTD_UNUSED(object_module);
#endif
    object->descriptor = descriptor;
    object->state.objdb_class = NULL;
    object->state.order = 0;
    object->state.client = NULL;
    object->state.acquired_client = NULL;
    object->state.shared = false;
    object->state.inCleanup = false;
    BDBG_MSG_OBJECT(object,("%p'%s': Init %p %p'%s'", (void *)descriptor, descriptor->type_name, (void *)(((uint8_t *)object)-descriptor->offset), (void *)object_module, object_module->pModuleName));
    return;
}

#if NEXUS_P_BASE_OBJECT_USE_REFCNT 
static int b_add_ref_cnf(NEXUS_BaseObject *object, int add)
{
    int result;
    result = object->ref_cnt + add;
    object->ref_cnt = result;
    return result;
}

/* when this function called, locks for owner object is already held */
#if BDBG_DEBUG_BUILD
void NEXUS_BaseObject_P_Acquire_Tagged(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module, const char *pFileName, unsigned lineNumber)
#else
void NEXUS_BaseObject_P_Acquire(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module)
#endif
{
    int result;
    NEXUS_BaseObject *base_object;

    BDBG_ASSERT(descriptor);
    BSTD_UNUSED(owner);
    BSTD_UNUSED(source_module);

    base_object = (void *)((uint8_t *)object + descriptor->offset);
    BDBG_OBJECT_ASSERT(base_object, NEXUS_BaseObject);

    BDBG_MSG_OBJECT(base_object,("%p'%s': Acquire %p %u %s:%u (%p'%s':%p'%s')", (void *)base_object->descriptor, base_object->descriptor->type_name, object, base_object->ref_cnt, pFileName, lineNumber, (void *)base_object->module, base_object->module->pModuleName, (void *)source_module, source_module->pModuleName ));
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    result = b_add_ref_cnf(base_object, 1);
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    BDBG_ASSERT(result>0);
    return;
}

/* this function called in conjunction with object verification, therefore the ref_cnt lock is already held */
void NEXUS_BaseObject_P_AcquireBaseObject(NEXUS_BaseObject *base_object)
{
    int result;

    BDBG_OBJECT_ASSERT(base_object, NEXUS_BaseObject);
    BDBG_MSG_OBJECT(base_object,("%p'%s': AcquireBaseObject %p %u ", (void *)base_object->descriptor, base_object->descriptor->type_name, (void *)base_object, base_object->ref_cnt));
    result = b_add_ref_cnf(base_object, 1);
    BDBG_ASSERT(result>0);
    return;
}

/* when this function called, locks for owner object is already held */
#if BDBG_DEBUG_BUILD
void NEXUS_BaseObject_P_Release_Tagged(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module, const char *pFileName, unsigned lineNumber)
#else
void NEXUS_BaseObject_P_Release(NEXUS_BaseObject *owner, void *object, const NEXUS_BaseClassDescriptor *descriptor, NEXUS_ModuleHandle source_module)
#endif
{
    int result;
    NEXUS_BaseObject *base_object;

    BDBG_ASSERT(descriptor);
    BSTD_UNUSED(owner);

    base_object = (void *)((uint8_t *)object + descriptor->offset);
    BDBG_OBJECT_ASSERT(base_object, NEXUS_BaseObject);

    BDBG_MSG_OBJECT(base_object,("%p'%s': Release %p %u %s:%u (%p'%s':%p'%s'", (void *)base_object->descriptor, base_object->descriptor->type_name, object, base_object->ref_cnt, pFileName, lineNumber, (void *)base_object->module, base_object->module->pModuleName, (void *)source_module, source_module->pModuleName ));
    BKNI_AcquireMutex(NEXUS_P_Base_State.baseObject.lock);
    result = b_add_ref_cnf(base_object, -1);
    BKNI_ReleaseMutex(NEXUS_P_Base_State.baseObject.lock);
    BDBG_ASSERT(result>=0);
    if(result<=0) {
        NEXUS_ModuleHandle lock_module = source_module == base_object->module ? NULL : base_object->module;
        void (*finalizer)(void *);
        if(lock_module) {
#if BDBG_DEBUG_BUILD
            NEXUS_Module_Lock_Tagged(lock_module, pFileName, lineNumber);
#else
            NEXUS_Module_Lock(lock_module);
#endif
        }
        BDBG_MSG_OBJECT(base_object,("%p: Finalize %p %u %s:%u", (void *)base_object->descriptor, object, base_object->ref_cnt, pFileName, lineNumber));
        BDBG_ASSERT(descriptor->finalizer);
        if(base_object->state.objdb_class) {
            BDBG_ERR(("about to destroy object %p:'%s' which still in the object database client:%p acquired_client:%p", (void *)base_object, base_object->descriptor->type_name, (void*)base_object->state.client, (void *)base_object->state.acquired_client));
            BDBG_ASSERT(base_object->state.objdb_class==NULL); /* by this time object should be already removed from the database, and if it's not removed, then object database would get corrupted */
        }
        finalizer = descriptor->finalizer;
        BDBG_OBJECT_DESTROY(base_object, NEXUS_BaseObject); 
        finalizer(object);
        if(lock_module) {
#if BDBG_DEBUG_BUILD
            NEXUS_Module_Unlock_Tagged(lock_module, pFileName, lineNumber);
#else
            NEXUS_Module_Unlock(lock_module);
#endif
        }
    }
    return;
}
#endif /* NEXUS_P_BASE_OBJECT_USE_REFCNT */


