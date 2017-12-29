/******************************************************************************
 * Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef B_IPC_NXCLIENT_P_COMPAT_H__
#define B_IPC_NXCLIENT_P_COMPAT_H__

#define B_NEXUS_COMPAT_TYPE(type) NxClient_P_Compat_##type
typedef uint32_t B_NEXUS_COMPAT_TYPE(ptr);
typedef uint32_t B_NEXUS_COMPAT_TYPE(size_t);
typedef uint32_t B_NEXUS_COMPAT_TYPE(unsigned_long);
#define B_NEXUS_COMPAT_TYPE_FAKE_HANDLE(handle) B_NEXUS_COMPAT_TYPE(ptr)
#define B_NEXUS_COMPAT_TYPE_MISC(type,alias) B_NEXUS_COMPAT_TYPE(alias)


#define NEXUS_P_COMPAT_IN_BEGIN(type) static NEXUS_Error NxClient_P_##type##_compat_in(const B_NEXUS_COMPAT_TYPE(type) *__src, type *__dst) { NEXUS_Error __rc=NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_END(type) return __rc;}

#define NEXUS_P_COMPAT_OUT_BEGIN(type) static NEXUS_Error NxClient_P_##type##_compat_out(const type *__src, B_NEXUS_COMPAT_TYPE(type) *__dst) { NEXUS_Error __rc=NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_OUT_END(type) return __rc;}


#define NEXUS_P_COMPAT_IN_PLAIN(rc, src, dst, type) dst=src; rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_GENERIC(rc, src, dst, type) BDBG_CASSERT(sizeof(src)==sizeof(unsigned)); dst = src; rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_STRUCT(rc, src, dst, type)  rc = NxClient_P_##type##_compat_in(&(src), &(dst));
#define NEXUS_P_COMPAT_IN_FAKE_HANDLE(rc, src, dst, type) dst = NxClient_P_Compat_From_Handle(src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_IN_SPECIAL(rc, src, dst, type) rc = NxClient_P_Compat_From_##type(&(src), &(dst));

#define NEXUS_P_COMPAT_OUT_PLAIN(rc, src, dst, type) NEXUS_P_COMPAT_IN_PLAIN(rc, src, dst, type)
#define NEXUS_P_COMPAT_OUT_GENERIC(rc, src, dst, type) NEXUS_P_COMPAT_IN_GENERIC(rc, src, dst, type)
#define NEXUS_P_COMPAT_OUT_STRUCT(rc, src, dst, type) rc = NxClient_P_##type##_compat_out(&(src), &(dst));
#define NEXUS_P_COMPAT_OUT_FAKE_HANDLE(rc, src, dst, type) dst = NxClient_P_Compat_To_Handle(src); rc = NEXUS_SUCCESS;
#define NEXUS_P_COMPAT_OUT_SPECIAL(rc, src, dst, type) rc = NxClient_P_Compat_To_##type(&(src), &(dst));

#define NEXUS_P_COMPAT_FIELD(DIR, rc, src, dst, type, field, KIND)  NEXUS_P_COMPAT_##DIR##_##KIND(rc, (src)->field, (dst)->field, type)
#define NEXUS_P_COMPAT_BEGIN_LOOP_1(level, size) {unsigned _n_##level##_0; for(_n_##level##_0=0;_n_##level##_0<size; _n_##level##_0++) {
#define NEXUS_P_COMPAT_END_LOOP_1(level, size) }}
#define NEXUS_P_COMPAT_FIELD_ARRAY_1(DIR, rc, src, dst, type, field, size, KIND) {unsigned _n_array;for(_n_array=0;_n_array<size;_n_array++) {NEXUS_P_COMPAT_FIELD(DIR, rc, src, dst, type, field[_n_array], KIND) }}
#define NEXUS_P_COMPAT_FIELD_ARRAY_2(DIR, rc, src, dst, type, field, size1, size2, KIND) {unsigned _n_array_1;for(_n_array_1=0;_n_array_1<size1;_n_array_1++) {unsigned _n_array_2;for(_n_array_2=0;_n_array_2<size2;_n_array_2++) {NEXUS_P_COMPAT_FIELD(DIR, rc, src, dst, type, field[_n_array_1][_n_array_2], KIND) }}}

#define NEXUS_P_COMPAT_COPY_UNION(field) BDBG_CASSERT(sizeof(__src->field)==sizeof(__dst->field)); BKNI_Memcpy(&__dst->field, &__src->field, sizeof(__src->field));

void *NxClient_P_Compat_From_Handle(B_NEXUS_COMPAT_TYPE(ptr) handle);
NEXUS_Error NxClient_P_Compat_From_size_t(const B_NEXUS_COMPAT_TYPE(size_t) *src, size_t *dst);
B_NEXUS_COMPAT_TYPE(ptr) NxClient_P_Compat_To_Handle(const void *handle);
NEXUS_Error NxClient_P_Compat_To_size_t(const size_t *src, B_NEXUS_COMPAT_TYPE(size_t) *dst);

#define B_NXCLIENT_P_COMPAT_BEGIN(class) static int b_ipc_##class##_process( unsigned abi, void **_b_ipc_id, unsigned entry, void *buf, size_t recv_size, bipc_send_data *send_data) {int rc=-1; if(abi==(sizeof(void*)*8)) {return b_ipc_##class##_process_native(_b_ipc_id, entry, buf, recv_size, send_data);} else if(abi==32) { B_NEXUS_COMPAT_TYPE(b_ipc_##class##_data) *__compat_data = buf; b_ipc_##class##_data __data; ;switch(entry) { default: rc=-1;break;
#define B_NXCLIENT_P_COMPAT_END(class) }} return rc; }
#define B_NXCLIENT_P_API_PROCESS_BEGIN(class,api) case b_ipc_##api:{
#define B_NXCLIENT_P_API_CONVERT_IN(class,api) if(NxClient_P_b_ipc_##api##_data_in_compat_in(&__compat_data->api.in, &__data.api.in)!=NEXUS_SUCCESS) {rc = -1;break;}
#define B_NXCLIENT_P_API_CALL(class,api) rc = b_ipc_##class##_process_native(_b_ipc_id, b_ipc_##api, &__data, sizeof(__data.api.in), send_data); if(rc!=0) {break;}
#define B_NXCLIENT_P_API_CONVERT_OUT(class,api) if(NxClient_P_b_ipc_##api##_data_out_compat_out(&__data.api.out, &__compat_data->api.out)!=NEXUS_SUCCESS) {rc = -1;break;} send_data->offset = offsetof(B_NEXUS_COMPAT_TYPE(b_ipc_##class##_data), api.out) - offsetof(B_NEXUS_COMPAT_TYPE(b_ipc_##class##_data), api); send_data->size = sizeof(__compat_data->api.out);
#define B_NXCLIENT_P_API_PROCESS_END(class,api) break;}

#endif /* B_IPC_NXCLIENT_P_COMPAT_H__ */
