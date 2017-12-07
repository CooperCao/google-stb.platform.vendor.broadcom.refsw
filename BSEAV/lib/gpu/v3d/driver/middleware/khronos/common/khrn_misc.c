/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/common/khrn_hw.h"

void khrn_misc_fifo_finish_impl(void)
{
   khrn_hw_common_finish();
}

void khrn_misc_rpc_flush_impl(void)
{
   /* states must be unlocked before eglMakeCurrent */
   egl_server_unlock();
}