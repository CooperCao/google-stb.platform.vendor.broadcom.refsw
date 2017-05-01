/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdbool.h>

extern bool egl_process_init(void);
extern void egl_process_add_ref(void);
extern void egl_process_release(void);
