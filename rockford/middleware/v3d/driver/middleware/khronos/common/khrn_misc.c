/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  External interface
File     :  $RCSfile$
Revision :  $Revision$

FILE DESCRIPTION
External interface.
=============================================================================*/

#include "middleware/khronos/common/khrn_misc.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/gl20/gl20_server.h"
#ifndef NO_OPENVG
#include "middleware/khronos/vg/vg_server.h"
#endif /* NO_OPENVG */
#include "middleware/khronos/egl/egl_server.h"

void khrn_misc_fifo_finish_impl(void) /* todo: misleading name? */
{
   khrn_hw_common_finish();
}

void khrn_misc_rpc_flush_impl(void)
{
   /* states must be unlocked before eglMakeCurrent */
   egl_server_unlock();
}
