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
#ifndef NEXUS_BASE_COMPAT_H
#define NEXUS_BASE_COMPAT_H

#define B_NEXUS_COMPAT_TYPE(type) NEXUS_Compat_##type
#define B_SIZEOF(type, field) sizeof(((type *)0)->field)

typedef uint32_t B_NEXUS_COMPAT_TYPE(ptr);
typedef uint32_t B_NEXUS_COMPAT_TYPE(size_t);
typedef uint32_t B_NEXUS_COMPAT_TYPE(unsigned_long);
typedef B_NEXUS_COMPAT_TYPE(ptr) B_NEXUS_COMPAT_TYPE(NEXUS_P_MemoryUserAddr);
typedef B_NEXUS_COMPAT_TYPE(ptr) B_NEXUS_COMPAT_TYPE(NEXUS_KeySlotTag);
typedef struct B_NEXUS_COMPAT_TYPE(NEXUS_CallbackDesc) {
    B_NEXUS_COMPAT_TYPE(ptr) callback;
    B_NEXUS_COMPAT_TYPE(ptr) context;
    int param;
    NEXUS_P_CALLBACK_COOKIE; /* private */
} B_NEXUS_COMPAT_TYPE(NEXUS_CallbackDesc);

typedef struct B_NEXUS_COMPAT_TYPE(NEXUS_Ipc_Header) {
   B_NEXUS_COMPAT_TYPE(size_t) packet_size;
   unsigned version;
   unsigned client_id;
   unsigned function_id;
   int result;
} B_NEXUS_COMPAT_TYPE(NEXUS_Ipc_Header);

#define B_NEXUS_COMPAT_TYPE_HANDLE(handle) NEXUS_BaseObjectId
#define B_NEXUS_COMPAT_TYPE_FAKE_HANDLE(handle) B_NEXUS_COMPAT_TYPE(ptr)
#define B_NEXUS_COMPAT_TYPE_ENUM_HANDLE(handle) NEXUS_BaseObjectId
#define B_NEXUS_COMPAT_TYPE_MEMORY(_ptr) B_NEXUS_COMPAT_TYPE(ptr)
#define B_NEXUS_COMPAT_TYPE_MISC(type,alias) B_NEXUS_COMPAT_TYPE(alias)
#define B_NEXUS_COMPAT_TYPE_POINTER(type) B_NEXUS_COMPAT_TYPE(ptr)
#define B_NEXUS_COMPAT_TYPE_NULL_POINTER(type) B_NEXUS_COMPAT_TYPE(ptr)

struct B_NEXUS_COMPAT_TYPE(nexus_callback_data)
{
    B_NEXUS_COMPAT_TYPE_HANDLE(handle) interface;
    B_NEXUS_COMPAT_TYPE(NEXUS_CallbackDesc) callback;
};


NEXUS_Error NEXUS_Compat_From_NEXUS_CallbackDesc(const B_NEXUS_COMPAT_TYPE(NEXUS_CallbackDesc) *src, NEXUS_CallbackDesc *dst);
NEXUS_Error NEXUS_Compat_To_NEXUS_CallbackDesc(const NEXUS_CallbackDesc *src, B_NEXUS_COMPAT_TYPE(NEXUS_CallbackDesc) *dst);
NEXUS_Error NEXUS_Compat_From_unsigned_long(const B_NEXUS_COMPAT_TYPE(unsigned_long) *src, unsigned long *dst);
NEXUS_Error NEXUS_Compat_To_unsigned_long(const unsigned long *src, B_NEXUS_COMPAT_TYPE(unsigned_long) *dst);
NEXUS_Error NEXUS_Compat_From_size_t(const B_NEXUS_COMPAT_TYPE(size_t) *src, size_t *dst);
NEXUS_Error NEXUS_Compat_To_size_t(const size_t *src, B_NEXUS_COMPAT_TYPE(size_t) *dst);
B_NEXUS_COMPAT_TYPE_HANDLE(handle) NEXUS_Compat_To_EnumHandle(const NEXUS_BaseObject *src);
unsigned long NEXUS_Compat_From_EnumHandle(B_NEXUS_COMPAT_TYPE_HANDLE(handle) src);
NEXUS_Error NEXUS_Compat_From_NEXUS_P_MemoryUserAddr(const B_NEXUS_COMPAT_TYPE(NEXUS_P_MemoryUserAddr) *src, NEXUS_P_MemoryUserAddr *dst);
NEXUS_Error NEXUS_Compat_To_NEXUS_P_MemoryUserAddr(const NEXUS_P_MemoryUserAddr *src, B_NEXUS_COMPAT_TYPE(NEXUS_P_MemoryUserAddr) *dst);
NEXUS_BaseObjectId NEXUS_Compat_To_Handle(const void *object);
void *NEXUS_Compat_From_Handle_tagged(NEXUS_BaseObjectId id, const char *file, unsigned number);
#define NEXUS_Compat_From_Handle(id) NEXUS_Compat_From_Handle_tagged(id, __FILE__, __LINE__)
B_NEXUS_COMPAT_TYPE(ptr) NEXUS_Compat_To_FakeHandle(const void *fakeHandle);
void *NEXUS_Compat_From_FakeHandle(B_NEXUS_COMPAT_TYPE(ptr) fakeHandle);


struct b_objdb_client;

typedef struct NEXUS_P_CompatInVararg {
    void *data;
    void *original_data;
    unsigned size;
    unsigned varargs_begin;
    unsigned varargs_offset;
    struct b_objdb_client *client;
} NEXUS_P_CompatInVararg;

void NEXUS_P_CompatInVararg_Init(NEXUS_P_CompatInVararg *state, struct b_objdb_client *client, void *data, unsigned size);
void NEXUS_P_CompatInVararg_Shutdown(NEXUS_P_CompatInVararg *state);
NEXUS_Error NEXUS_P_CompatInVararg_InVarArg(NEXUS_P_CompatInVararg *state, unsigned vararg_size, const void *src, int *field, bool *is_null);
NEXUS_Error NEXUS_P_CompatInVararg_Copy(NEXUS_P_CompatInVararg *state, unsigned compat_in_data_size, const void *compat_in_data, unsigned vararg_size, int compat_addr_offset, int *vararg_offset);

#endif /* NEXUS_BASE_COMPAT_H */
