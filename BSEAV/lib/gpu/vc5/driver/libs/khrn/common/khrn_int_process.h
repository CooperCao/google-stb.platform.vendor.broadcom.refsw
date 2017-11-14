/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_INT_PROCESS_H
#define KHRN_INT_PROCESS_H

#include "khrn_int_common.h"
#include "khrn_mem.h"

extern bool khrn_process_inited(void);
extern void *khrn_process_client_get(void);  /* CLIENT_PROCESS_STATE_T */
extern void *khrn_process_client_tls_check(void); /* CLIENT_THREAD_STATE_T */
extern void *khrn_process_client_tls_get(void);  /* CLIENT_THREAD_STATE_T */
extern void khrn_process_client_thread_detach(void);
extern void *khrn_process_egl_get(void);  /*EGL_SERVER_STATE_T*/
extern void *khrn_process_egl_thread(void);  /*EGL_SERVER_THREAD_T*/
extern void *khrn_process_glxx_lock(void);  /*GLXX_SERVER_STATE_T*/
extern void khrn_process_glxx_unlock(void);
extern uint64_t khrn_process_pid(void);
#endif
