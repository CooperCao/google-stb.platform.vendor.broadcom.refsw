/***************************************************************************
*  Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#define B_IPC_FIELD(api, dir, field) __##dir##_data->api.field
#define B_IOCTL_FIELD(api, dir, field) __ioctl->api.ioctl.dir.field

#define B_IPC_DRIVER_CALL_HELPER(mode, module) static NEXUS_Error nexus_p_driver_##module##_call##mode(unsigned function, const NEXUS_P_DriverInVararg *in, struct NEXUS_P_IpcProcessArgs *args) {BDBG_MSG(("%u data:%p(%p) varargs_begin:%u varargs_offset:%u", function, (void *)in->data, (void *)in->original_data, in->varargs_begin, in->varargs_offset)); return nexus_p_driver_##module##_process##mode(function, in->data, in->varargs_begin + in->varargs_offset, &args->vout_data, args->module_header, args->slave_scheduler);}

#define B_IPC_DRIVER_MODULE_BEGIN(_module, MODULE, mode) if ((NEXUS_IOCTL_NUM(cmd)/NEXUS_IOCTL_PER_MODULE) != NEXUS_IOCTL_##MODULE##_ID) { NEXUS_DRIVER_TRACE(-1); goto err_invalid_ioctl;} else {\
                                                  union nexus_driver_##_module##_args_##mode *__ioctl = &module->args.mode;\
                                                  b_##_module##_module_ipc_in_##mode  *__in_data = &nexus_driver_module_state.data.in.mode.ipc; b_##_module##_module_ipc_out_##mode  *__out_data = &nexus_driver_module_state.data.out.mode.ipc; \
                                                  switch(NEXUS_IOCTL_NUM(cmd)) { default: if(arg==0) { NEXUS_DRIVER_TRACE(-1);goto err_fault;} NEXUS_DRIVER_TRACE(-1);goto err_invalid_ioctl;

#define B_IPC_DRIVER_MODULE_END(module, MODULE) } }

#define B_IPC_DRIVER_BEGIN(_module,MODULE, api) case NEXUS_IOCTL_NUM(B_IPC_COMPAT_NAME(IOCTL_##MODULE##api)): NEXUS_P_TRACE_MSG((">%s" B_IPC_COMPAT_SELECT("", ":COMPAT"),  #api)); NEXUS_IOCTL_ENTER(api); __in_vararg.varargs_begin = B_IPC_DATA_ALIGN(((uint8_t *)&__in_data->api - (uint8_t *)__in_data) + sizeof(__in_data->api)); __ipc_args.vout_data.varargs_begin = B_IPC_DATA_ALIGN(((uint8_t *)&__out_data->api - (uint8_t *)__out_data) + sizeof(__out_data->api)); {
#define B_IPC_DRIVER_DATA(type, name) type name;
#define B_IPC_DRIVER_RECV_IOCTL(_module, api) if(copy_from_user_small(&__ioctl->api.ioctl.in, (void *)arg, sizeof(__ioctl->api.ioctl.in)) !=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}
#define B_IPC_DRIVER_SEND_IOCTL(_module, api) if(copy_to_user_small((void *)arg, &__ioctl->api.ioctl.out, sizeof(__ioctl->api.ioctl.out)) !=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}
#define B_IPC_DRIVER_RESULT(api) B_IOCTL_FIELD(api, out, ret.__retval) = B_IPC_FIELD(api, out, ret.__retval);
#define B_IPC_DRIVER_END(module, api)   NEXUS_P_TRACE_MSG(("<%s" B_IPC_COMPAT_SELECT("", ":COMPAT"), #api)); break;}

#define B_IPC_DRIVER_RECV_FIELD_ADDR(api, arg, field, field_addr) B_IPC_FIELD(api, in, memory.field_addr) = B_IOCTL_FIELD(api, in, memory.field_addr);
#define B_IPC_DRIVER_SEND_FIELD_ADDR(api, arg, field, field_addr) B_IOCTL_FIELD(api, out, memory.field_addr) = B_IPC_FIELD(api, out, memory.field_addr);
#define B_IPC_DRIVER_ASSIGN_PTR(api, arg) arg = B_IOCTL_FIELD(api, in, pointer.arg);
#define B_IPC_DRIVER_SET_OUT_PTR(api, arg) B_IPC_FIELD(api, in, pointer.out_is_null.arg) = !B_IOCTL_FIELD(api, in, pointer.arg);
#define B_IPC_DRIVER_RECV(api, arg) B_IPC_FIELD(api, in, args.arg) = B_IOCTL_FIELD(api, in, args.arg);
#define B_IPC_DRIVER_CALL(module, api) if( B_IPC_COMPAT_NAME(nexus_p_driver_##module##_call) (NEXUS_P_API_ID(module,api), &__in_vararg, &__ipc_args)!=NEXUS_SUCCESS) {NEXUS_DRIVER_TRACE(-1);goto err_fault;} __out_data = __ipc_args.vout_data.data;
#define B_IPC_DRIVER_COPY_OUT_PTR(api, arg) if(arg) {if(copy_to_user_small( B_IPC_COMPAT_POINTER(arg), &B_IPC_FIELD(api, out, pointer.arg), sizeof(B_IPC_FIELD(api, out, pointer.arg)))!=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}}
#define B_IPC_DRIVER_RECV_IN_PTR(api, arg) B_IPC_FIELD(api, in, pointer.is_null.arg) = !B_IOCTL_FIELD(api, in, pointer.arg); B_IPC_COMPAT_SELECT(, BKNI_Memset(&B_IPC_FIELD(api, in, pointer.arg), 0, sizeof(B_IPC_FIELD(api, in, pointer.arg))));if(B_IOCTL_FIELD(api, in, pointer.arg)) {if(copy_from_user_small(&B_IPC_FIELD(api, in, pointer.arg), B_IPC_COMPAT_POINTER(B_IOCTL_FIELD(api, in, pointer.arg)), sizeof(B_IPC_FIELD(api, in, pointer.arg)))!=0) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}}
#define B_IPC_DRIVER_SET_OUT_ADDR(api, arg) B_IPC_DRIVER_SET_OUT_PTR(api, arg)
#define B_IPC_DRIVER_COPY_ADDR(api, arg) B_IOCTL_FIELD(api, out, memory.arg) = B_IPC_FIELD(api, out, memory.arg);
#define B_IPC_DRIVER_RECV_ADDR(api, arg) B_IPC_FIELD(api, in, memory.arg) =  B_IOCTL_FIELD(api, in, memory.arg);
#define B_IPC_DRIVER_RECV_VARARG(api, arg, type, nelem) if(NEXUS_P_DriverInVararg_InVarArg(&__in_vararg, sizeof(type)* B_IOCTL_FIELD(api, in, args.nelem), B_IPC_COMPAT_POINTER(B_IOCTL_FIELD(api, in, pointer.arg)), &(B_IPC_FIELD(api, in, vararg.arg)), &(B_IPC_FIELD(api, in, pointer.is_null.arg)))!=NEXUS_SUCCESS) {NEXUS_DRIVER_TRACE(-1);goto err_alloc;}  __in_data = __in_vararg.data;
#define B_IPC_DRIVER_RECV_VARARG_NELEM_CONVERT(api, arg, type, convert, nelem) if(NEXUS_P_DriverInVararg_InVarArg(&__in_vararg, convert(B_IOCTL_FIELD(api, in, args.nelem)), B_IPC_COMPAT_POINTER(B_IOCTL_FIELD(api, in, pointer.arg)), &(B_IPC_FIELD(api, in, vararg.arg)), &(B_IPC_FIELD(api, in, pointer.is_null.arg)))!=NEXUS_SUCCESS) {NEXUS_DRIVER_TRACE(-1);goto err_alloc;}  __in_data = __in_vararg.data;
#define B_IPC_DRIVER_RECV_VARARG_ADDR(api, arg, type, field, field_addr, nelem) if(NEXUS_P_DriverInVararg_InVarArg_AddrField(&__in_vararg, B_IPC_COMPAT_POINTER(B_IOCTL_FIELD(api, in, pointer.field_addr)), B_IOCTL_FIELD(api, in, args.nelem), B_IPC_FIELD(api, in, vararg.arg), &(B_IPC_FIELD(api, in, vararg.field_addr)))!=NEXUS_SUCCESS) {NEXUS_DRIVER_TRACE(-1);goto err_alloc;}  __in_data = __in_vararg.data;
#define B_IPC_DRIVER_SEND_VARARG_NELEM_OUT(api, arg, type, nelem_out) if(NEXUS_P_Driver_OutVarArg(__out_data, sizeof(type) * B_IPC_FIELD(api, out, pointer.nelem_out), B_IPC_COMPAT_POINTER(arg), B_IPC_FIELD(api, out, vararg.arg))!=NEXUS_SUCCESS) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}
#define B_IPC_DRIVER_SEND_VARARG_NELEM(api, arg, type, nelem) if(NEXUS_P_Driver_OutVarArg(__out_data, sizeof(type) * B_IPC_FIELD(api, in, args.nelem), B_IPC_COMPAT_POINTER(arg), B_IPC_FIELD(api, out, vararg.arg))!=NEXUS_SUCCESS) {NEXUS_DRIVER_TRACE(-1);goto err_fault;}
/* END OF FILE */
