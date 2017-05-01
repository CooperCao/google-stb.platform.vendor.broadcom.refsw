/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*=============================================================================
VideoCore OS Abstraction Layer - 'once'
=============================================================================*/

#ifndef VCOS_ONCE_H
#define VCOS_ONCE_H

#include "vcos_types.h"
#include "vcos_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

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
#endif
