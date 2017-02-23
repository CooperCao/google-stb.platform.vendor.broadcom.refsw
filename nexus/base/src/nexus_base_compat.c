/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "nexus_base.h"
#include "nexus_base_priv.h"
#include "nexus_base_compat.h"
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_base_compat);

unsigned NEXUS_Compat_P_TruncateUnsigned(uint64_t src, const char *type)
{
    unsigned result = (unsigned) src;

    if(result != src) {
        BDBG_WRN(("Truncatinc %s from " BDBG_UINT64_FMT " to %#x", type, BDBG_UINT64_ARG(src), result));
    }
    return result;
}

NEXUS_Error NEXUS_Compat_From_NEXUS_CallbackDesc(const B_NEXUS_COMPAT_TYPE(NEXUS_CallbackDesc) *src, NEXUS_CallbackDesc *dst)
{
    dst->callback = (NEXUS_Callback)(unsigned long)src->callback;
    dst->context = (void *)(unsigned long)src->context;
    dst->param = src->param;
    dst->private_cookie = src->private_cookie;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Compat_To_NEXUS_CallbackDesc(const NEXUS_CallbackDesc *src, B_NEXUS_COMPAT_TYPE(NEXUS_CallbackDesc) *dst)
{
    dst->callback = (B_NEXUS_COMPAT_TYPE(ptr))(unsigned long)NEXUS_Compat_P_TruncateUnsigned((unsigned long)src->callback, "NEXUS_CallbackDesc.callback");
    dst->context = (B_NEXUS_COMPAT_TYPE(ptr))(unsigned long)NEXUS_Compat_P_TruncateUnsigned((unsigned long)src->context, "NEXUS_CallbackDesc.context");
    dst->param = src->param;
    dst->private_cookie = src->private_cookie;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Compat_From_unsigned_long(const B_NEXUS_COMPAT_TYPE(unsigned_long) *src, unsigned long *dst)
{
    *dst = *src;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Compat_To_unsigned_long(const unsigned long *src, B_NEXUS_COMPAT_TYPE(unsigned_long) *dst)
{
    *dst = NEXUS_Compat_P_TruncateUnsigned(*src, "unsigned long");
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Compat_From_size_t(const B_NEXUS_COMPAT_TYPE(size_t) *src, size_t *dst)
{
    *dst = *src;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Compat_To_size_t(const size_t *src, B_NEXUS_COMPAT_TYPE(size_t) *dst)
{
    *dst = NEXUS_Compat_P_TruncateUnsigned(*src, "size_t");
    return NEXUS_SUCCESS;
}

B_NEXUS_COMPAT_TYPE_HANDLE(handle) NEXUS_Compat_To_EnumHandle(const NEXUS_BaseObject *src)
{
    B_NEXUS_COMPAT_TYPE_HANDLE(handle) dst;
    if(src < (NEXUS_BaseObject *) NEXUS_BASEOBJECT_MIN_ID) {
        dst = (B_NEXUS_COMPAT_TYPE_HANDLE(handle))(unsigned long)src;
    } else if(  (unsigned long)src == (unsigned)-1 || (unsigned long)src == (unsigned long)-1) {
        dst = (B_NEXUS_COMPAT_TYPE_HANDLE(handle))(-1);
    } else {
        NEXUS_Error rc = NEXUS_BaseObject_GetId(src, &dst);
        if(rc!=NEXUS_SUCCESS) {
            BDBG_ERR(("NEXUS_Compat_To_EnumHandle:%p Invalid Object", (void *)src));
            (void)BERR_TRACE(rc);
        }
    }
    BDBG_MSG(("NEXUS_Compat_To_EnumHandle:%#lx -> %#lx", (unsigned long)src, (unsigned long)dst));
    return dst;
}

unsigned long NEXUS_Compat_From_EnumHandle(B_NEXUS_COMPAT_TYPE_HANDLE(handle) src)
{
    unsigned long dst;
    if(src < NEXUS_BASEOBJECT_MIN_ID) {
        dst = src;
    } else if( (int)(long)src == -1) {
        /* coverity[dead_error_line: FALSE] */
        dst = (unsigned long)-1;
    } else {
        NEXUS_BaseObject *object;
        NEXUS_Error rc = NEXUS_BaseObject_FromId(src, &object);
        if(rc!=NEXUS_SUCCESS) {
            BDBG_ERR(("NEXUS_Compat_From_EnumHandle:Can't map ID:%#x to object", (unsigned)src));
            (void)BERR_TRACE(rc);
        }
        dst = (unsigned long)object;
    }
    BDBG_MSG(("NEXUS_Compat_From_EnumHandle:%#lx -> %#lx", (unsigned long)src, (unsigned long)dst));
    return dst;
}

NEXUS_Error NEXUS_Compat_From_NEXUS_P_MemoryUserAddr(const B_NEXUS_COMPAT_TYPE(NEXUS_P_MemoryUserAddr) *src, NEXUS_P_MemoryUserAddr *dst)
{
    *dst = (NEXUS_P_MemoryUserAddr)(unsigned long)*src;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Compat_To_NEXUS_P_MemoryUserAddr(const NEXUS_P_MemoryUserAddr *src, B_NEXUS_COMPAT_TYPE(NEXUS_P_MemoryUserAddr) *dst)
{
    *dst = (B_NEXUS_COMPAT_TYPE(ptr))(unsigned long)NEXUS_Compat_P_TruncateUnsigned((unsigned long)*src, "NEXUS_P_MemoryUserAddr");
    return NEXUS_SUCCESS;
}

void NEXUS_P_CompatInVararg_Init(NEXUS_P_CompatInVararg *state, struct b_objdb_client *client, void *data, unsigned size)
{
    state->data = data;
    state->original_data = data;
    state->size = size;
    state->varargs_begin = 0;
    state->varargs_offset = 0;
    state->client = client;
    return;
}

void NEXUS_P_CompatInVararg_Shutdown(NEXUS_P_CompatInVararg *state)
{
    if(state->data != state->original_data) {
        BDBG_MSG(("%p: free %p:%u", (void *)state, state->data, state->size));
        BKNI_Free(state->data);
    }
    return;
}

static void *NEXUS_P_CompatInVararg_Allocate(NEXUS_P_CompatInVararg *state, unsigned vararg_size)
{
    unsigned aligned_vararg_size = B_IPC_DATA_ALIGN(vararg_size);
    unsigned required_size = state->varargs_offset + state->varargs_begin + aligned_vararg_size;
    void *data;
    BDBG_MSG(("%p: alloc %p:%u -> %u", (void *)state, state->data, state->size, required_size));
    if( required_size > state->size) {
        void *buf = nexus_client_driver_alloc(state->client, required_size);
        BDBG_MSG(("%p: realloc %p:%u -> %p:%u", (void *)state, state->data, state->size, buf, required_size));
        if(buf==NULL) {
            (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memcpy(buf, state->data, state->size);
        state->size = required_size;
        if(state->data != state->original_data) {
            BDBG_MSG(("%p: reaalloc free %p:%u", (void *)state, state->data, state->size));
            BKNI_Free(state->data);
        }
        state->data = buf;
    }
    data = (uint8_t *)state->data + state->varargs_begin + state->varargs_offset;
    state->varargs_offset += aligned_vararg_size;
    return data;
}

NEXUS_Error NEXUS_P_CompatInVararg_InVarArg(NEXUS_P_CompatInVararg *state, unsigned vararg_size, const void *src, int *field, bool *is_null)
{
    if(src) {
        void *data;
        void *old_data = state->data;
        *is_null = false; /* assign is_null earlies so don't worry about relocation */
        data = NEXUS_P_CompatInVararg_Allocate(state, vararg_size);
        if(data==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        /* field is the pointer into the state->data, so adjust it if data was relocated */
        /* freed pointer (old_data) only used in the pointer math, it is not dereferenced */
        /* coverity[use_after_free: FALSE] */
        field = (void *)((uint8_t *)state->data + (((uint8_t *)field - (uint8_t *)old_data)));
        *field = (uint8_t *)data - (uint8_t *)state->data;
        BKNI_Memcpy(data, src, vararg_size);
    } else {
        *is_null = true;
        *field = -1;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_P_CompatInVararg_Copy(NEXUS_P_CompatInVararg *state, unsigned compat_in_data_size, const void *compat_in_data, unsigned vararg_size, int compat_addr_offset, int *vararg_offset)
{
    *vararg_offset = -1;
    if(compat_addr_offset!=-1) {
        const void *compat_data;
        void *data;
        void *old_data = state->data;
        if(compat_addr_offset + vararg_size > compat_in_data_size) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
        compat_data = (void *)((uint8_t *)compat_in_data + compat_addr_offset);
        data = NEXUS_P_CompatInVararg_Allocate(state, vararg_size);
        if(data==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        /* field is the pointer into the state->data, so adjust it if data was relocated */
        /* freed pointer (old_data) only used in the pointer math, it is not dereferenced */
        /* coverity[use_after_free: FALSE] */
        vararg_offset = (void *)((uint8_t *)state->data + (((uint8_t *) vararg_offset - (uint8_t *)old_data)));
        *vararg_offset = (uint8_t *)data - (uint8_t *)state->data;
        BKNI_Memcpy(data, compat_data, vararg_size);
    }
    return NEXUS_SUCCESS;
}


NEXUS_BaseObjectId NEXUS_Compat_To_Handle(const void *object)
{
    NEXUS_BaseObjectId id = 0;
    if(object) {
        NEXUS_Error rc = NEXUS_BaseObject_GetId(object, &id);
        if(rc!=NEXUS_SUCCESS) {
            BDBG_ERR(("NEXUS_Compat_To_Handle:%p Invalid Object", (void *)object));
            (void)BERR_TRACE(rc);
        }
    }
    return id;
}


#if defined(NEXUS_Compat_From_Handle)
void *NEXUS_Compat_From_Handle_tagged(NEXUS_BaseObjectId id, const char *file, unsigned number)
#else
void *NEXUS_Compat_From_Handle(NEXUS_BaseObjectId id)
#endif
{
    NEXUS_BaseObject *object=NULL;
    if(id) {
        NEXUS_Error rc = NEXUS_BaseObject_FromId(id, &object);
        if(rc!=NEXUS_SUCCESS) {
#if defined(NEXUS_Compat_From_Handle)
            BDBG_ERR(("%s:%u", file, number));
#endif
            BDBG_ERR(("NEXUS_Compat_From_Handle: can't map ID:%#x to object", (unsigned)id));
            (void)BERR_TRACE(rc);
        }
    }
    return object;
}

B_NEXUS_COMPAT_TYPE(ptr) NEXUS_Compat_To_FakeHandle(const void *fakeHandle)
{
    if(fakeHandle!=NULL) {
        BDBG_ERR(("NEXUS_Compat_To_FakeHandle: not NULL(%p) used as a fake handle", fakeHandle));
    }
    return 0;
}

void *NEXUS_Compat_From_FakeHandle(B_NEXUS_COMPAT_TYPE(ptr) fakeHandle)
{
    if(fakeHandle!=0) {
        BDBG_ERR(("NEXUS_Compat_To_FakeHandle: not NULL(%#x) used as a fake handle", fakeHandle));
    }
    return NULL;
}
