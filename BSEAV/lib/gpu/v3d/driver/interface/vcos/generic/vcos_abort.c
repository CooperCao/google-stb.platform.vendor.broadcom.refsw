/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/vcos/vcos.h"
#include <stdlib.h>

int vcos_verify_bkpts = 0;

int vcos_verify_bkpts_enabled(void)
{
   return vcos_verify_bkpts;
}

int vcos_verify_bkpts_enable(int enable)
{
   int old = vcos_verify_bkpts;
   vcos_verify_bkpts = enable;
   return old;
}

/**
  * Call the fatal error handler.
  */
void vcos_abort(void)
{
#if defined(VCOS_HAVE_BACKTRACE) && !defined(NDEBUG)
   vcos_backtrace_self();
#endif

   /* Insert chosen fatal error handler here */
   abort();
}
