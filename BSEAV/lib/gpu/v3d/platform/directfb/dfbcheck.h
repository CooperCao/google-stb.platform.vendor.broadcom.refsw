/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <directfb_version.h>
#include <directfb.h>

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
   { \
   int err = static_cast<int>(x);                              \
   if (err != DFB_OK) {                                        \
      fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );   \
      DirectFBErrorFatal( #x, static_cast<DFBResult>(err) );   \
   }                                                           \
   }
