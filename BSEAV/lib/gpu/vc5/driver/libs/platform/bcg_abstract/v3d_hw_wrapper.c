/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "libs/sim/v3d_hw/v3d_hw.h"
#include "tools/v3d/simpenrose/top/simpenrose.h"

SPAPI void v3d_hw_simpenrose_init_shared(
   void           *heapPtr,      /* Cached CPU address of heap base     */
   size_t         heapSize,      /* Size of heap in bytes               */
   unsigned int   heapBasePhys,  /* Physical device offset of heap base */
   void           (*isr)(int)    /* ISR handler function pointer        */
   )
{
   struct simpenrose_init_args args;

   simpenrose_default_init_args(&args);
   args.base_address = heapBasePhys;

   v3d_hw_simpenrose_set_default_use_second_heap(false); /* Disable the second heap by default */
   v3d_hw_simpenrose_init(&args);
   v3d_hw_set_isr(isr);
   v3d_hw_set_heap(heapPtr, heapSize);
   v3d_hw_auto_tick_add_ref();
}

SPAPI uint32_t v3d_hw_read_reg_shared(int core, int64_t addr)
{
   return v3d_hw_read_reg(core, addr);
}

SPAPI void v3d_hw_write_reg_shared(int core, int64_t addr, uint32_t value)
{
   v3d_hw_write_reg(core, addr, value);
}
