/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/vcos/vcos.h"

typedef int KHR_STATUS_T;
#define KHR_SUCCESS  VCOS_SUCCESS

/*
   thread-local storage
*/

typedef VCOS_TLS_KEY_T PLATFORM_TLS_T;

extern VCOS_STATUS_T platform_tls_create(PLATFORM_TLS_T *tls, void (*destructor)(void*));
extern void platform_tls_destroy(PLATFORM_TLS_T tls);
extern void platform_tls_set(PLATFORM_TLS_T tls, void *v);
extern void *platform_tls_get(PLATFORM_TLS_T tls);
extern void platform_tls_remove(PLATFORM_TLS_T tls);