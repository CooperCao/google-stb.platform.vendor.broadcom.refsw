/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "interface/vcos/vcos_types.h"
#include "vcos_platform.h"

/**
 * \file vcos_once.h
 *
 * Ensure something is called only once.
 *
 * Initialize once_control to VCOS_ONCE_INIT. The first
 * time this is called, the init_routine will be called. Thereafter
 * it won't.
 *
 * \sa pthread_once()
 *
 */

VCOS_STATUS_T vcos_once(VCOS_ONCE_T *once_control,
                        void (*init_routine)(void));

#ifdef __cplusplus
}
#endif
