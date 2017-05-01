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

#if BDBG_DEBUG_BUILD
#define NEXUS_P_SERVER_ERROR_TRACE(x)   BERR_TRACE(x)
#else
#define NEXUS_P_SERVER_ERROR_TRACE(x)   (x)
#endif

#define NEXUS_DRIVER_CALLBACK_TO_DRIVER(callback, handle, id)  nexus_driver_callback_to_driver(module_header, (callback), (void *)(handle), (id), __vout_data->client, __slave_scheduler)
#define NEXUS_DRIVER_CALLBACK_TO_CLIENT(callback, handle, id) nexus_driver_callback_to_user(module_header, (callback), (void*)(handle), (id))
#define NEXUS_DRIVER_CALLBACK_UPDATE(callback, old_handle, id, new_handle) nexus_driver_callback_update(module_header, (callback), (void*)(old_handle), (id), (void *)(new_handle))
#define NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(callback, handle, id)  nexus_driver_callback_to_driver_commit(module_header, (callback), (void *)(handle),(id))
#define NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(callback, handle, id)  nexus_driver_callback_to_driver_cancel(module_header, (callback), (void *)(handle),(id))

#define NEXUS_DRIVER_CREATE_OBJECT(module, type, object) NEXUS_P_##module##_Create_Object(object, &NEXUS_OBJECT_DESCRIPTOR(type));
#define NEXUS_DRIVER_DESTROY_OBJECT(module, type, object) NEXUS_P_##module##_Destroy_Object(object, &NEXUS_OBJECT_DESCRIPTOR(type));
#define NEXUS_DRIVER_ACQUIRE_OBJECT(module, type, object) NEXUS_P_##module##_Acquire_Object(object, &NEXUS_OBJECT_DESCRIPTOR(type));
#define NEXUS_DRIVER_RELEASE_OBJECT(module, type, object) NEXUS_P_##module##_Release_Object(object, &NEXUS_OBJECT_DESCRIPTOR(type));


#define B_IPC_FIELD(api, dir, field) __##dir##_data->api.field

#if __COVERITY__
#define B_IPC_SERVER_COVERITY_STACK() __coverity_stack_depth__(20000)
#else
#define B_IPC_SERVER_COVERITY_STACK()
#endif

#define B_IPC_SERVER_PROLOGUE(module)  static void nexus_p_##module##_api_call_completed(const NEXUS_P_ServerOutVarArg_State *__vout_data, const struct api_function_descriptor *function, void *in_data, void *out_data) { \
    nexus_p_api_call_completed(__vout_data->client, NEXUS_MODULE_SELF, function, in_data, out_data);return;} \
    static void NEXUS_P_##module##_Create_Object(void *object, const NEXUS_BaseClassDescriptor *descriptor) {NEXUS_BaseObject_P_RegisterUnregister(object, descriptor, NEXUS_Object_P_RegisterUnregister_eAutoCreate, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF);return;} \
    static void NEXUS_P_##module##_Destroy_Object(void *object, const NEXUS_BaseClassDescriptor *descriptor) {NEXUS_BaseObject_P_RegisterUnregister(object, descriptor, NEXUS_Object_P_RegisterUnregister_eAutoDestroy, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF);return;} \
    static void NEXUS_P_##module##_Acquire_Object(void *object, const NEXUS_BaseClassDescriptor *descriptor) {NEXUS_BaseObject_P_RegisterUnregister(object, descriptor, NEXUS_Object_P_RegisterUnregister_eAutoAcquire, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF);return;} \
    static void NEXUS_P_##module##_Release_Object(void *object, const NEXUS_BaseClassDescriptor *descriptor) {NEXUS_BaseObject_P_RegisterUnregister(object, descriptor, NEXUS_Object_P_RegisterUnregister_eAutoRelease, NEXUS_MODULE_SELF, NEXUS_MODULE_SELF);return;} \
    static NEXUS_Error nexus_p_##module##_api_call_verify(const NEXUS_P_ServerOutVarArg_State *__vout_data, const struct api_function_descriptor *function, void *__in_data, unsigned in_data_size) {\
    return nexus_p_api_call_verify(__vout_data->client, NEXUS_MODULE_SELF, function, __in_data, in_data_size, __vout_data->size);} \
    NEXUS_Error nexus_p_driver_##module##_process(unsigned function, b_##module##_module_ipc_in *__in_data, unsigned in_data_size, NEXUS_P_ServerOutVarArg_State *__vout_data, struct nexus_driver_module_header *module_header, struct nexus_driver_slave_scheduler *__slave_scheduler)  {\
    NEXUS_Error __rc = NEXUS_SUCCESS; b_##module##_module_ipc_out *__out_data = (void *)((uint8_t *)__vout_data->data + __vout_data->header);\
    BSTD_UNUSED(module_header); BSTD_UNUSED(__slave_scheduler);B_IPC_SERVER_COVERITY_STACK();
#define B_IPC_SERVER_DISPATCH(module) switch(function) {default:__rc=NEXUS_P_SERVER_ERROR_TRACE(NEXUS_NOT_SUPPORTED);break;goto err_alloc;goto err_fault;
#define B_IPC_SERVER_EPILOGUE(module) } done: return __rc; err_fault: return BERR_OS_ERROR; err_alloc: return NEXUS_OUT_OF_SYSTEM_MEMORY; goto done; }

#define B_IPC_SERVER_BEGIN(module, api) case NEXUS_P_API_ID(module,api): __vout_data->varargs_begin = ((uint8_t *)&__out_data->api - (uint8_t *)__out_data) + sizeof(__out_data->api); {
#define B_IPC_SERVER_VERIFY(module, api) __rc = nexus_p_##module##_api_call_verify(__vout_data, &_api##api, __in_data, in_data_size); if(__rc!=NEXUS_SUCCESS) {NEXUS_P_SERVER_ERROR_TRACE(NEXUS_INVALID_PARAMETER);goto err_fault;}
#define B_IPC_SERVER_COMPLETED(module, api) __rc = NEXUS_SUCCESS ; api##_done: nexus_p_##module##_api_call_completed(__vout_data, &_api##api, __in_data, __out_data);
#define B_IPC_SERVER_END(module, api) }  break; goto api##_done;
#define B_IPC_SERVER_PARAM(type, name)  type name;
#define B_IPC_SERVER_CALL(api, funcname, args) __result = B_IPC_FIELD(api, out, ret.__retval) = funcname args; if(B_IPC_FIELD(api, out, ret.__retval) !=NEXUS_SUCCESS) {__rc=NEXUS_UNKNOWN;};
#define B_IPC_SERVER_CONSTRUCTOR(module, api, _class, funcname) if(__rc==NEXUS_SUCCESS) {NEXUS_DRIVER_CREATE_OBJECT(module, _class, B_IPC_FIELD(api, out, ret.__retval));}
#define B_IPC_SERVER_CONSTRUCTOR_ENUM(module, api, _class, funcname) if(__rc==NEXUS_SUCCESS) {NEXUS_DRIVER_CREATE_OBJECT(module, _class, (void *)B_IPC_FIELD(api, out, ret.__retval));}
#define B_IPC_SERVER_ACQUIRE(module, api, _class, funcname) if(__rc==NEXUS_SUCCESS) {NEXUS_DRIVER_ACQUIRE_OBJECT(module, _class, B_IPC_FIELD(api, out, ret.__retval));}
#define B_IPC_SERVER_DESTROY(module, api, _class, first_arg) NEXUS_DRIVER_DESTROY_OBJECT(module,_class, first_arg);
#define B_IPC_SERVER_DESTROY_ENUM(module, api, _class, first_arg) NEXUS_DRIVER_DESTROY_OBJECT(module, _class, (void *)first_arg);
#define B_IPC_SERVER_RELEASE(module, api, _class, first_arg) NEXUS_DRIVER_RELEASE_OBJECT(module, _class, B_IPC_FIELD(api, in, args.first_arg));
#define B_IPC_SERVER_SHUTDOWN(api, _class, first_arg) nexus_driver_shutdown_close_##_class(B_IPC_FIELD(api, in, args.first_arg));
#define B_IPC_SERVER_CALL_HANDLE(api, funcname, args) __result = B_IPC_FIELD(api, out, ret.__retval) = funcname args; if(!__result) {__rc=NEXUS_UNKNOWN;}
#define B_IPC_SERVER_CALL_VOID(api, funcname, args) funcname args;
#define B_IPC_SERVER_SET_OUT_PTR(api, arg) arg = B_IPC_FIELD(api, in, pointer.out_is_null.arg ) ? NULL : &B_IPC_FIELD(api, out, pointer.arg);
#define B_IPC_SERVER_RECV(api, arg) arg = B_IPC_FIELD(api, in, args.arg);
#define B_IPC_SERVER_RECV_IN_PTR(api, arg) arg = B_IPC_FIELD(api, in, pointer.is_null.arg ) ? NULL : &B_IPC_FIELD(api, in, pointer.arg);
#define B_IPC_SERVER_SEND_FIELD_ADDR(api, arg, field, memory_field) B_IPC_FIELD(api, out, memory.memory_field) = NEXUS_P_ServerCall_AddrToOffset(B_IPC_FIELD(api, out, pointer.arg.field));
#define B_IPC_SERVER_DATA(type, arg) type __data_##arg;
#define B_IPC_SERVER_RESULT(type, arg) type  arg;
#define B_IPC_SERVER_SET_ADDR(arg)  arg = &__data_##arg;
#define B_IPC_SERVER_SEND_ADDR(api, arg) B_IPC_FIELD(api, out, memory.arg) = NEXUS_P_ServerCall_AddrToOffset(*arg);
#define B_IPC_SERVER_RECV_FIELD_ADDR(api, arg, field, memory_field) B_IPC_FIELD(api, in, pointer.arg.field) = NEXUS_P_ServerCall_OffsetToAddr(B_IPC_FIELD(api, in, memory.memory_field));
#define B_IPC_SERVER_RECV_ADDR(api, arg) arg = NEXUS_P_ServerCall_OffsetToAddr(B_IPC_FIELD(api, in, memory.arg));
#define B_IPC_SERVER_RECV_VARARG(api, arg, type, nelem) if(!B_IPC_FIELD(api, in, pointer.is_null.arg)) {  if(B_IPC_FIELD(api, in, vararg.arg) + sizeof(type)* B_IPC_FIELD(api, in, args.nelem) > in_data_size ) {(void)NEXUS_P_SERVER_ERROR_TRACE(NEXUS_INVALID_PARAMETER);goto api##_done;} arg = (void *)((uint8_t *)__in_data + B_IPC_FIELD(api, in, vararg.arg));} else { arg = NULL;}
#define B_IPC_SERVER_RECV_VARARG_NELEM_CONVERT(api, arg, type, convert, nelem) if(!B_IPC_FIELD(api, in, pointer.is_null.arg)) {  if(B_IPC_FIELD(api, in, vararg.arg) + convert(B_IPC_FIELD(api, in, args.nelem)) > in_data_size ) {(void)NEXUS_P_SERVER_ERROR_TRACE(NEXUS_INVALID_PARAMETER);goto api##_done;} arg = (void *)((uint8_t *)__in_data + B_IPC_FIELD(api, in, vararg.arg));} else { arg = NULL;}
#define B_IPC_SERVER_RECV_VARARG_ADDR(api, arg, type, field, field_addr, nelem) NEXUS_P_ServerCall_InVarArg_AddrField(in_data_size, __in_data, sizeof(*arg), (uint8_t *)&(arg->field) - (uint8_t *)arg, B_IPC_FIELD(api, in, args.nelem) , B_IPC_FIELD(api, in, vararg.arg), (B_IPC_FIELD(api, in, vararg.field_addr)));
#define B_IPC_SERVER_TRACE(x) NEXUS_P_TRACE_MSG(x);
#define B_IPC_SERVER_BEGIN_OUT_VARARG(api) {unsigned __varags_size = 0;
#define B_IPC_SERVER_SIZE_OUT_VARARG(api, arg,  type, length) __varags_size += B_IPC_DATA_ALIGN(sizeof(type) * length); B_IPC_FIELD(api, out, vararg.arg)=-1;
#define B_IPC_SERVER_ALLOCATE_OUT_VARARG(api)  __rc = NEXUS_P_ServerCall_OutVarArg_Allocate(__vout_data, __varags_size);if(__rc!=NEXUS_SUCCESS) { __rc = NEXUS_P_SERVER_ERROR_TRACE(__rc);goto api##_done; } __out_data=(void *)((uint8_t *)__vout_data->data+__vout_data->header);
#define B_IPC_SERVER_PLACE_OUT_VARARG(api, arg,  type, length) if (B_IPC_FIELD(api, in, pointer.out_is_null.arg)) { arg = NULL; } else { arg=NEXUS_P_ServerCall_OutVarArg_Place(__vout_data, sizeof(type) * length, &B_IPC_FIELD(api, out, vararg.arg));if(arg==NULL) { __rc = NEXUS_P_SERVER_ERROR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto api##_done;}}
#define B_IPC_SERVER_END_OUT_VARARG(api) }

#define B_IPC_CALLBACK_CONSTRUCTOR_FIELD_IN_PREPARE(api, arg, field, first_arg, id) if(arg) { NEXUS_DRIVER_CALLBACK_TO_DRIVER(&B_IPC_FIELD(api, in, pointer.arg.field), NULL, id);}
#define B_IPC_CALLBACK_CONSTRUCTOR_FIELD_IN_FINALIZE(api, arg, field, first_arg, id) if(arg) { if(__rc==NEXUS_SUCCESS) { NEXUS_DRIVER_CALLBACK_UPDATE(&B_IPC_FIELD(api, in, pointer.arg.field), NULL, id, B_IPC_FIELD(api, out, ret.__retval));} else {NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(&B_IPC_FIELD(api, in, pointer.arg.field), NULL, id);} }
#define B_IPC_CALLBACK_FIELD_OUT(api, arg, field, handle, id) if(__rc==NEXUS_SUCCESS && arg) { NEXUS_DRIVER_CALLBACK_TO_CLIENT(&(arg->field), handle, id);}
#define B_IPC_CALLBACK_FIELD_IN_PREPARE(api, arg, field, handle, id) if(arg) { NEXUS_DRIVER_CALLBACK_TO_DRIVER(&B_IPC_FIELD(api, in, pointer.arg.field), handle, id);}
#define B_IPC_CALLBACK_FIELD_IN_FINALIZE(api, arg, field, handle, id) if(arg) { if(__rc==NEXUS_SUCCESS) { NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(&B_IPC_FIELD(api, in, pointer.arg.field), handle, id);} else {NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(&B_IPC_FIELD(api, in, pointer.arg.field), handle, id);}}
#define B_IPC_CALLBACK_ENUM_FIELD_IN_PREPARE(api, arg, field, first_arg, id)  BDBG_CASSERT(0);  /* NOT SUPPORTED */
#define B_IPC_CALLBACK_ENUM_FIELD_IN_FINALIZE(api, arg, field, first_arg, id) BDBG_CASSERT(0);  /* NOT SUPPORTED */
#define B_IPC_CALLBACK_IN_PREPARE(api, arg, first_arg, id) if(arg) {NEXUS_DRIVER_CALLBACK_TO_DRIVER(&B_IPC_FIELD(api, in, pointer.arg), first_arg, id);}
#define B_IPC_CALLBACK_IN_FINALIZE(api, arg, first_arg, id) if(arg) { if(__rc==NEXUS_SUCCESS) { NEXUS_DRIVER_CALLBACK_TO_DRIVER_COMMIT(&B_IPC_FIELD(api, in, pointer.arg), first_arg, id);} else {NEXUS_DRIVER_CALLBACK_TO_DRIVER_CANCEL(&B_IPC_FIELD(api, in, pointer.arg), first_arg, id);}}
#define B_IPC_CALLBACK_INIT_FIELD_OUT(api, arg, field, first_arg, id) /* DO NOTHING, NEXUS_XXX_GetDefaultSettings */
#define B_IPC_CALLBACK_ENUM_FIELD_OUT(api, arg, field, first_arg, id) /* DO NOTHING, NEXUS_XXX_GetDefaultSettings */
#define B_IPC_CALLBACK_UNKNOWN_FIELD_IN_PREPARE(api, arg, field, first_arg, id)  BDBG_CASSERT(0);  /* NOT SUPPORTED */
#define B_IPC_CALLBACK_UNKNOWN_FIELD_IN_FINALIZE(api, arg, field, first_arg, id) BDBG_CASSERT(0);  /* NOT SUPPORTED */
#define B_IPC_CALLBACK_INIT_FIELD_IN_PREPARE(api, arg, field, first_arg, id) BDBG_CASSERT(0);  /* NOT SUPPORTED */
#define B_IPC_CALLBACK_INIT_FIELD_IN_FINALIZE(api, arg, field, first_arg, id) BDBG_CASSERT(0);  /* NOT SUPPORTED */
#define B_IPC_CALLBACK_UNKNOWN_FIELD_OUT(api, arg, field, first_arg, id)  /* DO NOTHING */
#define B_IPC_CALLBACK_CLEAR_FIELD_IN_PREPARE(api, arg, field, first_arg, id) if(arg) { /* clear callback */B_IPC_FIELD(api, in, pointer.arg.field.callback)=NULL;}
#define B_IPC_CALLBACK_CLEAR_FIELD_IN_FINALIZE(api, arg, field, first_arg, id) /* DO NOTHING */

#define B_IPC_SERVER_FAKE_HANDLE_NULL(api, arg, type) if(B_IPC_FIELD(api, in, args.arg) != NULL) {__rc = NEXUS_P_SERVER_ERROR_TRACE(NEXUS_INVALID_PARAMETER);goto api##_done;} /* FAKE handles if passed to function that allows NULL handles, must be NULL */
#define B_IPC_SERVER_FAKE_HANDLE(api, arg, type) BDBG_CASSERT(0); /* If functions can not access NULL as argument, it can't accept FAKE handles */

/* END OF FILE */
