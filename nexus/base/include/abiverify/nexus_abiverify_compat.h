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
#include "nexus_base_compat.h"
#include "common/ipc/nexus_ipc_server_api.h"


#define NEXUS_P_COMPAT_UNION_BEGIN(module) static void nexus_p_##module##_verify_unions(void) {
#define NEXUS_P_COMPAT_UNION_STRUCT(type)
#define NEXUS_P_COMPAT_UNION_STRUCT_LOCAL(type)
#define NEXUS_P_COMPAT_UNION_END(module) return;}
#define NEXUS_P_COMPAT_UNION_VERIFY(struct, base, union, type) BDBG_CASSERT(B_SIZEOF(struct, base.union)==B_SIZEOF(B_NEXUS_COMPAT_TYPE(struct), base.union)); BDBG_CASSERT( (offsetof(struct, base.union) - offsetof(struct, base))==(offsetof(B_NEXUS_COMPAT_TYPE(struct), base.union) - offsetof(B_NEXUS_COMPAT_TYPE(struct), base)) );
#define NEXUS_P_COMPAT_UNION_VERIFY_PLAIN(struct, base, union, type) NEXUS_P_COMPAT_UNION_VERIFY(struct, base, union, type)
#define NEXUS_P_COMPAT_UNION_VERIFY_GENERIC(struct, base, union, type) BDBG_CASSERT(B_SIZEOF(struct,base.union)==sizeof(unsigned)); NEXUS_P_COMPAT_UNION_VERIFY(struct, base, union, type)
#define NEXUS_P_COMPAT_UNION_VERIFY_SPECIAL(struct, base, union, type) BDBG_ASSERT(0);  /* not supported */

#define NEXUS_P_COMPAT_IN_BEGIN(type) static NEXUS_Error type##_compat_in(const B_NEXUS_COMPAT_TYPE(type) *__src, type *__dst) { NEXUS_Error __rc=NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_END(type) return __rc;}
#define NEXUS_P_COMPAT_LOCAL_STRUCT(type) static void type##_compat_not_supported(void) { BDBG_CASSERT(0); /* structures with attribute local=true can't be referenced by non-local functions */;}

#define NEXUS_P_COMPAT_IN_PLAIN(rc, src, dst, type) dst=src; rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_GENERIC(rc, src, dst, type) BDBG_CASSERT(sizeof(src)==sizeof(unsigned)); dst = src; rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_MEMORY(rc, src, dst, type) dst = NULL; rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_HANDLE(rc, src, dst, type) dst = NEXUS_Compat_From_Handle(src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_FAKE_HANDLE(rc, src, dst, type) dst = NEXUS_Compat_From_FakeHandle(src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_ENUM_HANDLE(rc, src, dst, type) dst=NEXUS_Compat_From_EnumHandle((B_NEXUS_COMPAT_TYPE_HANDLE(handle))src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_SPECIAL(rc, src, dst, type) rc = NEXUS_Compat_From_##type(&(src), &(dst));
#define NEXUS_P_COMPAT_IN_STRUCT(rc, src, dst, type)  rc = type##_compat_in(&(src), &(dst));
#define NEXUS_P_COMPAT_IN_NULL_POINTER(rc, src, dst, type) dst = NULL; rc = NEXUS_SUCCESS;

#define NEXUS_P_COMPAT_FIELD(DIR, rc, src, dst, type, field, KIND)  NEXUS_P_COMPAT_##DIR##_##KIND(rc, (src)->field, (dst)->field, type)
#define NEXUS_P_COMPAT_FIELD_ARRAY_1(DIR, rc, src, dst, type, field, size, KIND) {unsigned _n_array;for(_n_array=0;_n_array<size;_n_array++) {NEXUS_P_COMPAT_FIELD(DIR, rc, src, dst, type, field[_n_array], KIND) }}
#define NEXUS_P_COMPAT_FIELD_ARRAY_2(DIR, rc, src, dst, type, field, size1, size2, KIND) {unsigned _n_array_1;for(_n_array_1=0;_n_array_1<size1;_n_array_1++) {unsigned _n_array_2;for(_n_array_2=0;_n_array_2<size2;_n_array_2++) {NEXUS_P_COMPAT_FIELD(DIR, rc, src, dst, type, field[_n_array_1][_n_array_2], KIND) }}}

#define NEXUS_P_COMPAT_OUT_BEGIN(type) static NEXUS_Error type##_compat_out(const type *__src, B_NEXUS_COMPAT_TYPE(type) *__dst) { NEXUS_Error __rc=NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_OUT_END(type) return __rc;}

#define NEXUS_P_COMPAT_OUT_PLAIN(rc, src, dst, type) NEXUS_P_COMPAT_IN_PLAIN(rc, src, dst, type)
#define NEXUS_P_COMPAT_OUT_GENERIC(rc, src, dst, type) NEXUS_P_COMPAT_IN_GENERIC(rc, src, dst, type)
#define NEXUS_P_COMPAT_OUT_MEMORY(rc, src, dst, type) dst = (B_NEXUS_COMPAT_TYPE(ptr))0; rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_OUT_FAKE_HANDLE(rc, src, dst, type) dst = NEXUS_Compat_To_FakeHandle(src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_OUT_HANDLE(rc, src, dst, type) dst = NEXUS_Compat_To_Handle((const NEXUS_BaseObject *)src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_OUT_ENUM_HANDLE(rc, src, dst, type) dst = NEXUS_Compat_To_EnumHandle((const NEXUS_BaseObject *)src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_OUT_SPECIAL(rc, src, dst, type) rc = NEXUS_Compat_To_##type(&(src), &(dst));
#define NEXUS_P_COMPAT_OUT_STRUCT(rc, src, dst, type) rc = type##_compat_out(&(src), &(dst));
#define NEXUS_P_COMPAT_OUT_NULL_POINTER(rc, src, dst, type ) dst = 0; rc = NEXUS_SUCCESS;

#define NEXUS_P_COMPAT_BEGIN_LOOP_1(level, size) {unsigned _n_##level##_0; for(_n_##level##_0=0;_n_##level##_0<size; _n_##level##_0++) {
#define NEXUS_P_COMPAT_END_LOOP_1(level, size) }}

#define NEXUS_P_COMPAT_COPY_UNION(field) BDBG_CASSERT(sizeof(__src->field)==sizeof(__dst->field));  BKNI_Memcpy(&__dst->field, &__src->field, sizeof(__src->field));

#define NEXUS_P_COMPAT_DECLARE(module) static struct { struct { b_##module##_module_ipc_in ipc;  b_##module##_vararg_data_in varargs;} in; struct { b_##module##_module_ipc_out  ipc;} out; b_##module##_vararg_data_out varargs;} B_##module##_compat_data;
#define NEXUS_P_COMPAT_PROLOGUE(module) NEXUS_Error nexus_p_driver_##module##_process_compat(unsigned __id, const B_NEXUS_COMPAT_TYPE(b_##module##_module_ipc_in) *__compat_in_data, unsigned compat_in_data_size, NEXUS_P_ServerOutVarArg_State *__compat_vout_data, struct nexus_driver_module_header *module_header, struct nexus_driver_slave_scheduler *__slave_scheduler) { NEXUS_Error __rc=NEXUS_SUCCESS;B_NEXUS_COMPAT_TYPE(b_##module##_module_ipc_out) *__compat_out_data=NULL; b_##module##_module_ipc_in *__in_data = &B_##module##_compat_data.in.ipc; b_##module##_module_ipc_out *__out_data ; NEXUS_P_CompatInVararg __vin_data; NEXUS_P_ServerOutVarArg_State __vout_data; NEXUS_P_ServerCall_OutVarArg_Init(&__vout_data, __compat_vout_data->client, &B_##module##_compat_data.out, sizeof(B_##module##_compat_data.out)); NEXUS_P_CompatInVararg_Init(&__vin_data, __compat_vout_data->client, __in_data,  sizeof(*__in_data));
#define NEXUS_P_COMPAT_EPILOGUE(module) default: __rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);break;}  NEXUS_P_CompatInVararg_Shutdown(&__vin_data); NEXUS_P_ServerCall_OutVarArg_Shutdown(&__vout_data); nexus_p_##module##_verify_unions();return __rc; }
#define NEXUS_P_COMPAT_DISPATCH(module) switch(__id) {
#define NEXUS_P_COMPAT_BEGIN(module,api) case NEXUS_P_API_##module##api##_id: __vin_data.varargs_begin =  B_IPC_DATA_ALIGN(((uint8_t *)&__in_data->api - (uint8_t *)__in_data) + sizeof(__in_data->api)); __compat_vout_data->varargs_begin = B_IPC_DATA_ALIGN(((uint8_t *)&__compat_out_data->api - (uint8_t *)__compat_out_data) + sizeof(__compat_out_data->api)); {
#define NEXUS_P_COMPAT_END(module,api) break;}
#define NEXUS_P_COMPAT_CONVERT_IN(module,api) if(compat_in_data_size<sizeof(__compat_in_data->api)) {__rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);break;} __rc = B##api##_ipc_in_compat_in(&__compat_in_data->api, &__in_data->api); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);break;}

#define NEXUS_P_COMPAT_CONVERT_OUT(module,api) __rc = B##api##_ipc_out_compat_out( &__out_data->api, &__compat_out_data->api);if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);break;} __compat_vout_data->varargs_begin = ((uint8_t *)&__compat_out_data->api - (uint8_t *)__compat_out_data) + sizeof(__compat_out_data->api);
#define NEXUS_P_COMPAT_PROCESS(module, api) __rc = nexus_p_driver_##module##_process(__id, __in_data, __vin_data.varargs_begin  + __vin_data.varargs_offset, &__vout_data, module_header, __slave_scheduler);if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);break;}  __out_data =  __vout_data.data; __compat_out_data = (void *)((uint8_t *)__compat_vout_data->data + __compat_vout_data->header);

#define NEXUS_P_COMPAT_VARARG_IN_DECLARE(api, arg, type,compat_type) const compat_type *__compat_##arg; type *__##arg;
#define NEXUS_P_COMPAT_VARARG_IN_PLACE(api, arg, type, compat_type, nelem) if(!__compat_in_data->api.pointer.is_null.arg) { if(compat_in_data_size < __compat_in_data->api.vararg.arg + __compat_in_data->api.args.nelem * sizeof(compat_type)) {__rc=BERR_TRACE(NEXUS_INVALID_PARAMETER); break;} __compat_##arg = (void *)((uint8_t *)__compat_in_data + __compat_in_data->api.vararg.arg);  } else {__compat_##arg = NULL;}
#define NEXUS_P_COMPAT_VARARG_IN_PLACE_WITH_CONVERT(api, arg, type, compat_type, convert, nelem) if(!__compat_in_data->api.pointer.is_null.arg) { if(compat_in_data_size < __compat_in_data->api.vararg.arg + convert(__compat_in_data->api.args.nelem)) {__rc=BERR_TRACE(NEXUS_INVALID_PARAMETER); break;} __compat_##arg = (void *)((uint8_t *)__compat_in_data + __compat_in_data->api.vararg.arg);  } else {__compat_##arg = NULL;}
#define NEXUS_P_COMPAT_VARARG_IN_CONVERT(api, arg, KIND, type, nelem) __rc = NEXUS_P_CompatInVararg_InVarArg(&__vin_data, sizeof(*__##arg) *  __compat_in_data->api.args.nelem,  __compat_##arg, &__in_data->api.vararg.arg, &__in_data->api.pointer.is_null.arg); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc); break;} ; __in_data = __vin_data.data;if(__compat_##arg != NULL) { unsigned __i; __##arg = (void *)( (uint8_t *)__in_data +__in_data->api.vararg.arg);for(__i=0;__i<__compat_in_data->api.args.nelem;__i++) { NEXUS_P_COMPAT_IN_##KIND(__rc,__compat_##arg[__i], __##arg[__i], type)}}
#define NEXUS_P_COMPAT_VARARG_IN_CONVERT_WITH_CONVERT(api, arg, type, convert, nelem) __rc = NEXUS_P_CompatInVararg_Copy(&__vin_data, compat_in_data_size, __compat_in_data, convert(__compat_in_data->api.args.nelem) , __compat_in_data->api.vararg.arg, &__in_data->api.vararg.arg); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);break;} BSTD_UNUSED(__##arg); __in_data=__vin_data.data;

#define NEXUS_P_COMPAT_VARARG_IN_ADDR_CONVERT(api, arg, type, addr_field, nelem) __rc = NEXUS_P_CompatInVararg_Copy(&__vin_data, compat_in_data_size, __compat_in_data, __compat_in_data->api.args.nelem * sizeof(NEXUS_Addr), __compat_in_data->api.vararg.addr_field, &__in_data->api.vararg.addr_field); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);break;} __in_data=__vin_data.data;

#define NEXUS_P_COMPAT_VARARG_OUT_DECLARE(api, arg, type,compat_type) compat_type *__compat_##arg; const type *__##arg;
#define NEXUS_P_COMPAT_VARARG_OUT_DECLARE_NELEM(api, arg, nelem) unsigned __n_##arg##_##nelem = __compat_in_data->api.args.nelem;
#define NEXUS_P_COMPAT_VARARG_OUT_UPDATE_NELEM(api, arg, nelem, nelem_out) __n_##arg##_##nelem = __out_data->api.pointer.nelem_out;
#define NEXUS_P_COMPAT_VARARG_OUT_BEGIN(api) { unsigned __compat_vararg_out_size = 0;
#define NEXUS_P_COMPAT_VARARG_OUT_SIZE(api, arg, type, compat_type, nelem) __compat_vararg_out_size += B_IPC_DATA_ALIGN(sizeof(compat_type) * __n_##arg##_##nelem);
#define NEXUS_P_COMPAT_VARARG_OUT_ALLOCATE(api) __rc = NEXUS_P_ServerCall_OutVarArg_Allocate(__compat_vout_data, __compat_vararg_out_size); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);break;} __compat_out_data=(void *)((uint8_t *)__compat_vout_data->data+__compat_vout_data->header);
#define NEXUS_P_COMPAT_VARARG_OUT_PLACE(api, arg, type, compat_type, nelem) if( __compat_in_data->api.pointer.out_is_null.arg) {__compat_##arg=NULL;} else { __compat_##arg=NEXUS_P_ServerCall_OutVarArg_Place(__compat_vout_data, sizeof(compat_type) * __n_##arg##_##nelem, &__compat_out_data->api.vararg.arg );if (__compat_##arg==NULL) { __rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);break;}}
#define NEXUS_P_COMPAT_VARARG_OUT_END(api) }
#define NEXUS_P_COMPAT_VARARG_OUT_CONVERT(api, arg, KIND, type, nelem)  if(__compat_##arg) { unsigned __i; __##arg = (void *)( (uint8_t *)__out_data +__out_data->api.vararg.arg);for(__i=0;__i<__n_##arg##_##nelem;__i++) { NEXUS_P_COMPAT_OUT_##KIND(__rc,__##arg[__i], __compat_##arg[__i], type)}}
