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
*
***************************************************************************/
#ifndef _NEXUS_IPC_SERVER_API_H_
#define _NEXUS_IPC_SERVER_API_H_

#include "nexus_types.h"
#include "nexus_memory.h"
#include "bkni.h"
#include "b_objdb.h"

#define NEXUS_P_API_ID(module, api) NEXUS_P_API_##module##api##_id

/* COMMON API which has to be supported by the Kernel and IPC 'drivers' */
struct nexus_driver_module_header;
struct nexus_driver_slave_scheduler;

/* nexus_driver_callbacks */
void nexus_driver_callback_to_driver(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id,
    const struct b_objdb_client *client, struct nexus_driver_slave_scheduler *slave);
void nexus_driver_callback_to_user(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);
void nexus_driver_callback_update(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *old_handle, unsigned id, void *new_handle);
void nexus_driver_callback_to_driver_commit(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);
void nexus_driver_callback_to_driver_cancel(struct nexus_driver_module_header *header, NEXUS_CallbackDesc *callback, void *handle, unsigned id);

typedef struct NEXUS_P_ServerOutVarArg_State {
    void *original_data;
    void *data;
    unsigned header;
    unsigned size;
    unsigned varargs_begin;
    unsigned varargs_offset;
    struct b_objdb_client *client;
} NEXUS_P_ServerOutVarArg_State;



NEXUS_Addr NEXUS_P_ServerCall_AddrToOffset(const void *ptr);
void *NEXUS_P_ServerCall_OffsetToAddr(NEXUS_Addr addr);
NEXUS_Error NEXUS_P_ServerCall_InVarArg_AddrField(unsigned in_data_size, void *in_data, unsigned struct_size, unsigned field_offset, unsigned count, int varArg, int varArgField);
void NEXUS_P_ServerCall_OutVarArg_Init(NEXUS_P_ServerOutVarArg_State *state, struct b_objdb_client *client, void *data, unsigned size);
NEXUS_Error NEXUS_P_ServerCall_OutVarArg_Allocate(NEXUS_P_ServerOutVarArg_State *state, unsigned total_data_size);
void *NEXUS_P_ServerCall_OutVarArg_Place(NEXUS_P_ServerOutVarArg_State *state, unsigned data_size, int *field_offset);
void NEXUS_P_ServerCall_OutVarArg_Shutdown(NEXUS_P_ServerOutVarArg_State *state);

#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) union b_##module##_module_ipc_in; NEXUS_Error nexus_p_driver_##module##_process(unsigned function, union b_##module##_module_ipc_in *__in_data, unsigned in_data_size, NEXUS_P_ServerOutVarArg_State *__vout_data,  struct nexus_driver_module_header *module_header, struct nexus_driver_slave_scheduler *__slave_scheduler);
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE

#if NEXUS_COMPAT_32ABI
#include "nexus_base_compat.h"
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) union B_NEXUS_COMPAT_TYPE(b_##module##_module_ipc_in); NEXUS_Error nexus_p_driver_##module##_process_compat(unsigned __id, const union B_NEXUS_COMPAT_TYPE(b_##module##_module_ipc_in) *__compat_in_data, unsigned compat_in_data_size, NEXUS_P_ServerOutVarArg_State *__compat_vout_data, struct nexus_driver_module_header *module_header, struct nexus_driver_slave_scheduler *__slave_scheduler);
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE
#endif

#endif /* _NEXUS_IPC_SERVER_API_H_ */
