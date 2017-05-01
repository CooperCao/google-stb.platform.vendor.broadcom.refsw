/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_printer.h"
#include <stdint.h>

VCOS_EXTERN_C_BEGIN

typedef void (*v3d_reg_print_fn_t)(uint32_t, struct v3d_printer *);

extern v3d_reg_print_fn_t v3d_maybe_reg_print_fn(uint32_t addr);

static inline void v3d_maybe_print_reg(uint32_t addr, uint32_t value, struct v3d_printer *printer)
{
   v3d_reg_print_fn_t fn = v3d_maybe_reg_print_fn(addr);
   if (fn)
      fn(value, printer);
}

VCOS_EXTERN_C_END
