/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <alloca.h>
#if 0
#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE(x)
#endif

#define B_IPC_FIELD(api, dir, field) __data.dir.field
#define B_IPC_CLIENT_PROLOGUE(module, MODULE, function, api) NEXUS_Error __rc=NEXUS_SUCCESS; MODULE##api##_ioctl_data __data;BSTD_UNUSED(__rc);NEXUS_PROXY_ENTER(function);if(nexus_proxy_module_state.fd < 0) { BERR_TRACE(NEXUS_OS_ERROR);goto done;}

#define B_IPC_CLIENT_MODULE_BEGIN(module, MODULE)

#define B_IPC_CLIENT_MODULE_END(module)
#define B_IPC_CLIENT_TRACE(x) BDBG_MSG_TRACE(x);
#define B_IPC_CLIENT_BEGIN(module, MODULE, result, api, function, args) result function args { result __result=(result)0;B_IPC_CLIENT_PROLOGUE(module,MODULE,function,api);
#define B_IPC_CLIENT_BEGIN_VOID(module, MODULE, api, function, args) void function args { B_IPC_CLIENT_PROLOGUE(module,MODULE,function,api);
#define B_IPC_CLIENT_BEGIN_DESTRUCTOR(module, MODULE, api, function, args, handle, has_callbacks) void function args { bool __has_callbacks = has_callbacks; if(__has_callbacks) {NEXUS_StopCallbacks((void *)handle);} { B_IPC_CLIENT_PROLOGUE(module,MODULE,function,api);
#define B_IPC_CLIENT_SET_RESULT(type, api) __result = B_IPC_FIELD(api, out, ret.__retval);
#define B_IPC_CLIENT_CHECK_RETURN_CODE(api) if(__result!=NEXUS_SUCCESS) {__rc=/*BERR_TRACE*/(__result);goto done;}
#define B_IPC_CLIENT_END_VOID(api)  done: NEXUS_PROXY_LEAVE(api);return;}
#define B_IPC_CLIENT_END_DESTRUCTOR(api, handle)  done: NEXUS_PROXY_LEAVE(api);} return;}
#define B_IPC_CLIENT_END_ACQUIRE_HANDLE(type, api) NEXUS_StartCallbacks((void*)__result); return __result; done: return (type)NULL;}
#define B_IPC_CLIENT_END_HANDLE(type, api) return __result; done: return (type)NULL;}
#define B_IPC_CLIENT_END(api)  return __result;done: return __rc;}
#define B_IPC_CLIENT_SEND(api, arg)  B_IPC_FIELD(api, in, args.arg) = arg;
#define B_IPC_CLIENT_SEND_IN_PTR(api, arg) B_IPC_FIELD(api, in, pointer.arg) = arg;
#define B_IPC_CLIENT_SEND_OUT_PTR(api, arg) B_IPC_FIELD(api, in, pointer.arg) = arg;
#define B_IPC_CLIENT_SEND_VARARG(api, arg, type, nelem) B_IPC_FIELD(api, in, pointer.arg) = arg;
#define B_IPC_CLIENT_SEND_VARARG_NELEM_CONVERT(api, arg, type, convert, nelem) B_IPC_CLIENT_SEND_VARARG(api, arg, type, nelem)
#define B_IPC_CLIENT_SEND_VARARG_ADDR(api, arg, type, field, field_addr, nelem) if(arg!=NULL) { B_IPC_FIELD(api, in, pointer.field_addr) = alloca(sizeof(NEXUS_Addr) * nelem); \
    if(B_IPC_FIELD(api, in, pointer.field_addr)==NULL) { __rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto done;} \
    __rc = NEXUS_P_ProxyCall_InVarArg_AddrField(B_IPC_FIELD(api, in, pointer.field_addr), arg, sizeof(*arg), (uint8_t *)&(arg->field) - (uint8_t *)arg, nelem); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);goto done;}\
    } else { B_IPC_FIELD(api, in, pointer.field_addr) = NULL;}
#define B_IPC_CLIENT_CALL(module, MODULE, api) if(ioctl( nexus_proxy_module_state.fd, IOCTL_##MODULE##api, &__data)!=0) {__rc=BERR_TRACE(NEXUS_OS_ERROR);goto done;}
#define B_IPC_CLIENT_RECV_OUT_PTR(api, arg)
#define B_IPC_CLIENT_RECV_ADDR(api, arg)  *arg = NEXUS_P_ProxyCall_OffsetToAddr(B_IPC_FIELD(api, out, memory.arg));
#define B_IPC_CLIENT_RECV_FIELD_ADDR(api, arg, field, memory_field)  arg->field = NEXUS_P_ProxyCall_OffsetToAddr(B_IPC_FIELD(api, out, memory.memory_field));
#define B_IPC_CLIENT_SEND_FIELD_ADDR(api, arg, field, memory_field)  B_IPC_FIELD(api, in, memory.memory_field) = (arg!=NULL) ?  NEXUS_P_ProxyCall_AddrToOffset(arg->field) : 0;
#define B_IPC_CLIENT_RECV_VARARG(api, arg, type, field)
#define B_IPC_CLIENT_SEND_ADDR(api, arg)  B_IPC_FIELD(api, in, memory.arg) = NEXUS_P_ProxyCall_AddrToOffset(arg);


/* END OF FILE */
