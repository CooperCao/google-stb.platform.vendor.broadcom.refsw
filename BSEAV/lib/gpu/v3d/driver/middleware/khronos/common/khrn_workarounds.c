/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/EGL/egl.h"
#include "vcfw/drivers/chip/abstract_v3d.h"

#include <stdio.h>

/* The workarounds table */
KHRN_WORKAROUNDS_T khrn_workarounds;

static void set_workarounds(uint32_t techRev, uint32_t revision, uint32_t tusPerSlice)
{
   /* All workarounds on by default */
   memset(&khrn_workarounds, (int)true, sizeof(khrn_workarounds));

   khrn_workarounds.TMUS_PER_SLICE    = tusPerSlice;
   khrn_workarounds.FB_BOTTOM_UP      = true;
   khrn_workarounds.FB_TOP_DOWN       = false;
   khrn_workarounds.HAS_32BIT_SUPPORT = false;

   if (techRev >= 3 && revision >= 1)
   {
      /* Features at B1 or above */
      khrn_workarounds.FB_BOTTOM_UP = false;
      khrn_workarounds.FB_TOP_DOWN  = true;
   }

   if (techRev >= 3 && revision >= 2)
   {
      /* Features at B2 or above */
      khrn_workarounds.HAS_32BIT_SUPPORT = true;

      /* Fixed at B2 or above */
      khrn_workarounds.HW2116 = false;    /* let ptb state counters wrap around safely */
      khrn_workarounds.HW2806 = false;    /* Flags get written back to wrong quad when z test result collides with new thread */
      khrn_workarounds.HW2885 = false;    /* lockup due to erroneous activity in varyings interpolation module when outputting to the coverage pipe */
      khrn_workarounds.HW2898 = false;    /* NPOT raster textures */
      khrn_workarounds.HW2903 = false;    /* Zero-size points break PTB */
      khrn_workarounds.HW2905 = false;    /* Multisample full depth load breaks early-z */
      khrn_workarounds.SBWAIT = false;    /* core has to issue a SBWAIT instruction to access the scoreboard */
      khrn_workarounds.GFXH724 = false;   /* Make top bits of INTCTL read masked interrupt status */
   }
}

void khrn_init_workarounds(void)
{
   BEGL_HWInfo info;
   BEGL_GetDriverInterfaces()->hwInterface->GetInfo(BEGL_GetDriverInterfaces()->hwInterface->context, &info);
   set_workarounds(info.techRev, info.revision, info.textureUnitsPerSlice);
}

KHAPI void khrn_fake_workarounds(uint32_t techRev, uint32_t rev, uint32_t tusPerSlice)
{
   UNUSED(techRev);
   UNUSED(rev);
   UNUSED(tusPerSlice);
#ifdef BCG_SIM_BUILD
   // Only allowed to fake up workarounds on the simulator
   set_workarounds(techRev, rev, tusPerSlice);
#endif
}
