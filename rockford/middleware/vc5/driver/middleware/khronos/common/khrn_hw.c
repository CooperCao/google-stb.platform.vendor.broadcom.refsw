/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Various cross-API hardware-specific functions
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_copy_buffer.h"
//#include "middleware/khronos/common/khrn_prod.h"
#include "middleware/khronos/common/khrn_interlock_priv.h"
#include "middleware/khronos/common/khrn_render_state.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_umem.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/glxx/glxx_inner.h" /* for glxx_hw_init, glxx_hw_term */

#ifndef WIN32
# include <sys/time.h>
# include <sys/times.h>
#endif

#ifndef KHRN_SINGLE_THREADED
static VCOS_EVENT_T *master_event;
#endif

void khrn_hw_common_flush(uint32_t older_than)
{
   khrn_render_state_flush_all_older(KHRN_RENDER_STATE_TYPE_NONE, older_than);
}

/*
   void khrn_hw_common_finish()

   Flush all queued stuff. Wait for all flushed stuff to finish.

   Preconditions:

   -

   Postcondtions:

   For all KHRN_IMAGE_T image:
      image.conceptual_readable_by_master is true
      image.conceptual_writeable_by_master is true
   conceptual_buffers_owned_by_master is true
   conceptual_programs_owned_by_master is true
*/

void khrn_hw_common_finish(uint32_t older_than)
{
   khrn_hw_common_flush(older_than);
   khrn_hw_common_wait();
}

#ifndef KHRN_SINGLE_THREADED
void khrn_specify_event(VCOS_EVENT_T *ev)
{
   master_event = ev;
}
#endif

int32_t khrn_do_suspend_resume(uint32_t up)
{
   UNUSED(up);

   /* This function probably isn't needed */
   return 0;
}
