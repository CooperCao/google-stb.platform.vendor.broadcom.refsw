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

#if 0
#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE(x)
#endif

#if 0
#define NEXUS_CLIENT_ENTER(name) BDBG_LOG((">>%s", #name))
#define NEXUS_CLIENT_LEAVE(name) BDBG_LOG(("<<%s", #name))
#else
#define NEXUS_CLIENT_ENTER(f)
#define NEXUS_CLIENT_LEAVE(f)
#endif

#define B_IPC_DATA() ((b_ipc_this_module_data *)__state.data)
#define B_IPC_FIELD_DIR(api, dir) B_IPC_DATA()->data.dir.api
#define B_IPC_FIELD(api, dir, field) B_IPC_DATA()->data.dir.api.field
#define B_IPC_CLIENT_PROLOGUE(module, function, api) NEXUS_Error __rc; NEXUS_P_ClientCall_State __state;NEXUS_CLIENT_ENTER(function);\
    __state.varargs_begin = B_IPC_DATA_ALIGN( ((uint8_t *)&B_IPC_FIELD_DIR(api,in) -  (uint8_t *)__state.data) +  sizeof(B_IPC_FIELD_DIR(api,in)));\
    __rc = NEXUS_P_ClientCall_Begin(nexus_client_##module##_state, &__state, ((uint8_t *)&B_IPC_FIELD_DIR(api,in) -  (uint8_t *)__state.data));if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);goto err_begin;}

#define B_IPC_CLIENT_MODULE_BEGIN(module, MODULE) \
    BDBG_MODULE(nexus_##module##_client_ipc); \
    static NEXUS_P_ClientModuleHandle nexus_client_##module##_state;typedef B_IPC_##MODULE##_data b_ipc_this_module_data; \
    NEXUS_Error nexus_client_##module##_init(NEXUS_P_ClientModuleHandle module) { nexus_client_##module##_state = module; return NEXUS_SUCCESS;} \
    void nexus_client_##module##_uninit(void) { nexus_client_##module##_state = NULL; }


#define B_IPC_CLIENT_MODULE_END(module)
#define B_IPC_CLIENT_TRACE(x) BDBG_MSG_TRACE(x);
#define B_IPC_CLIENT_BEGIN(module, MODULE, result, api, function, args) result function args { result __result=(result)0;B_IPC_CLIENT_PROLOGUE(module,function,api);
#define B_IPC_CLIENT_BEGIN_VOID(module, MODULE, api, function, args) void function args { B_IPC_CLIENT_PROLOGUE(module,function,api);
#define B_IPC_CLIENT_BEGIN_DESTRUCTOR(module, MODUELE, api, function, args, handle, has_callbacks) void function args { bool __has_callbacks = has_callbacks; if(__has_callbacks) {NEXUS_StopCallbacks((void *)handle);} {B_IPC_CLIENT_PROLOGUE(module,function,api);
#define B_IPC_CLIENT_SET_RESULT(type, api) __result = B_IPC_FIELD(api, out, ret.__retval);
#define B_IPC_CLIENT_END_VOID(api)  err_call: NEXUS_P_ClientCall_End(&__state);err_begin: NEXUS_CLIENT_LEAVE(api);return;}
#define B_IPC_CLIENT_END_DESTRUCTOR(api, handle)  err_call: NEXUS_P_ClientCall_End(&__state);err_begin: if(__has_callbacks) {NEXUS_StartCallbacks((void *)handle);} NEXUS_CLIENT_LEAVE(api);} return;}
#define B_IPC_CLIENT_END_HANDLE(type, api) done_end: NEXUS_P_ClientCall_End(&__state); done: NEXUS_CLIENT_LEAVE(api);return __result; err_call: __result=0; goto done_end;;err_begin: __result=0;goto done;}
#define B_IPC_CLIENT_END(api)  done_end: NEXUS_P_ClientCall_End(&__state);done: return __result;err_call: __result=__rc; goto done_end;err_begin: __result=__rc;goto done;}
#define B_IPC_CLIENT_SEND(api, arg)  B_IPC_FIELD(api, in, args.arg) = arg;
#define B_IPC_CLIENT_SEND_IN_PTR(api, arg) BKNI_Memset(&B_IPC_FIELD(api, in, pointer.arg),0,sizeof(B_IPC_FIELD(api, in, pointer.arg)));B_IPC_FIELD(api, in, pointer.is_null.arg) = (arg == NULL);if(arg!=NULL) {B_IPC_FIELD(api, in, pointer.arg) = *arg;}
#define B_IPC_CLIENT_SEND_OUT_PTR(api, arg) B_IPC_FIELD(api, in, pointer.out_is_null.arg) = (arg == NULL);
#define B_IPC_CLIENT_SEND_VARARG(api, arg, type, nelem) __rc = NEXUS_P_ClientCall_InVarArg(&__state, sizeof(type)* nelem, arg, &(B_IPC_FIELD(api, in, vararg.arg)), &(B_IPC_FIELD(api, in, pointer.is_null.arg)) ); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);goto err_call;}
#define B_IPC_CLIENT_SEND_VARARG_NELEM_CONVERT(api, arg, type, convert, nelem) __rc = NEXUS_P_ClientCall_InVarArg(&__state, convert(nelem), arg, &(B_IPC_FIELD(api, in, vararg.arg)), &(B_IPC_FIELD(api, in, pointer.is_null.arg)) ); if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);goto err_call;}
#define B_IPC_CLIENT_SEND_VARARG_ADDR(api, arg, type, field, field_addr, nelem) __rc = NEXUS_P_ClientCall_InVarArg_AddrField(&__state, sizeof(*arg), (uint8_t *)&(arg->field) - (uint8_t *)arg, nelem, B_IPC_FIELD(api, in, vararg.arg), &(B_IPC_FIELD(api, in, vararg.field_addr)));if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);goto err_call;}
#define B_IPC_CLIENT_CALL(module, MODULE, api) B_IPC_DATA()->header.function_id = NEXUS_P_API_ID(module,api); B_IPC_DATA()->header.version = NEXUS_##module##_MODULE_VERSION; \
                                __rc = NEXUS_P_ClientCall_Call(&__state);if(__rc!=NEXUS_SUCCESS) {__rc=BERR_TRACE(__rc);goto err_call;}
#define B_IPC_CLIENT_RECV_OUT_PTR(api, arg) if(arg) { *arg = B_IPC_FIELD(api, out, pointer.arg); }
#define B_IPC_CLIENT_RECV_ADDR(api, arg)  *arg = NEXUS_P_ClientCall_OffsetToAddr(B_IPC_FIELD(api, out, memory.arg));
#define B_IPC_CLIENT_RECV_FIELD_ADDR(api, arg, field, memory_field)  arg->field = NEXUS_P_ClientCall_OffsetToAddr(B_IPC_FIELD(api, out, memory.memory_field));
#define B_IPC_CLIENT_SEND_FIELD_ADDR(api, arg, field, memory_field)  B_IPC_FIELD(api, in, memory.memory_field) = (arg!=NULL) ?  NEXUS_P_ClientCall_AddrToOffset(arg->field) : 0;
#define B_IPC_CLIENT_RECV_VARARG(api, arg, type, nelem) NEXUS_P_ClientCall_OutVarArg(&__state, sizeof(type) * nelem, arg, B_IPC_FIELD(api, out, vararg.arg));
#define B_IPC_CLIENT_SEND_ADDR(api, arg)  B_IPC_FIELD(api, in, memory.arg) = NEXUS_P_ClientCall_AddrToOffset(arg);


/* END OF FILE */
