/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "v3d_regs.h"

bool v3d_is_bin_cle_reg(uint32_t addr)
{
   if (v3d_is_hub_reg(addr))
      return false;

   switch (v3d_reg_to_core0(addr))
   {
   /* BEGIN AUTO-GENERATED CODE (v3d_is_bin_cle_reg_cases) */
#if V3D_VER_EQUAL(3, 2)
   case V3D_CLE_0_CT0CS:
   case V3D_CLE_0_CT0EA:
   case V3D_CLE_0_CT0CA:
   case V3D_CLE_0_CT0RA0:
   case V3D_CLE_0_CT0LC:
   case V3D_CLE_0_CT0PC:
   case V3D_CLE_0_CT0SYNC:
   case V3D_CLE_0_CT0QBA:
   case V3D_CLE_0_CT0QEA:
   case V3D_CLE_0_CT0QMA:
   case V3D_CLE_0_CT0QMS:
   case V3D_CLE_0_CT0QSYNC:
   case V3D_CLE_0_CT0CAD:
#endif

#if V3D_VER_EQUAL(3, 3)
   case V3D_CLE_0_CT0CS:
   case V3D_CLE_0_CT0EA:
   case V3D_CLE_0_CT0CA:
   case V3D_CLE_0_CT0RA0:
   case V3D_CLE_0_CT0LC:
   case V3D_CLE_0_CT0PC:
   case V3D_CLE_0_CT0SYNC:
   case V3D_CLE_0_CT0QBA:
   case V3D_CLE_0_CT0QEA:
   case V3D_CLE_0_CT0QMA:
   case V3D_CLE_0_CT0QMS:
   case V3D_CLE_0_CT0QSYNC:
   case V3D_CLE_0_CT0CAD:
#endif

#if V3D_VER_EQUAL(3, 4)
   case V3D_CLE_0_CT0CS:
   case V3D_CLE_0_CT0EA:
   case V3D_CLE_0_CT0CA:
   case V3D_CLE_0_CT0RA0:
   case V3D_CLE_0_CT0LC:
   case V3D_CLE_0_CT0PC:
   case V3D_CLE_0_CT0SYNC:
   case V3D_CLE_0_CT0QBA:
   case V3D_CLE_0_CT0QEA:
   case V3D_CLE_0_CT0QMA:
   case V3D_CLE_0_CT0QMS:
   case V3D_CLE_0_CT0QSYNC:
   case V3D_CLE_0_CT0CAD:
#endif

#if V3D_VER_EQUAL(7, 0)
   case V3D_CLE_0_CT0CS:
   case V3D_CLE_0_CT0EA:
   case V3D_CLE_0_CT0CA:
   case V3D_CLE_0_CT0RA0:
   case V3D_CLE_0_CT0LC:
   case V3D_CLE_0_CT0PC:
   case V3D_CLE_0_CT0SYNC:
   case V3D_CLE_0_CT0QBA:
   case V3D_CLE_0_CT0QEA:
   case V3D_CLE_0_CT0QMA:
   case V3D_CLE_0_CT0QMS:
   case V3D_CLE_0_CT0QSYNC:
   case V3D_CLE_0_CT0CAD:
#endif
   /* END AUTO-GENERATED CODE (v3d_is_bin_cle_reg_cases) */
      return true;
   default:
      return false;
   }
}

bool v3d_is_render_cle_reg(uint32_t addr)
{
   if (v3d_is_hub_reg(addr))
      return false;

   switch (v3d_reg_to_core0(addr))
   {
   /* BEGIN AUTO-GENERATED CODE (v3d_is_render_cle_reg_cases) */
#if V3D_VER_EQUAL(3, 2)
   case V3D_CLE_0_CT1CS:
   case V3D_CLE_0_CT1EA:
   case V3D_CLE_0_CT1CA:
   case V3D_CLE_0_CT1RA0:
   case V3D_CLE_0_CT1LC:
   case V3D_CLE_0_CT1PC:
   case V3D_CLE_0_CT1CFG:
   case V3D_CLE_0_CT1TILECT:
   case V3D_CLE_0_CT1TSKIP:
   case V3D_CLE_0_CT1PTCT:
   case V3D_CLE_0_CT1SYNC:
   case V3D_CLE_0_CT1QBA:
   case V3D_CLE_0_CT1QEA:
   case V3D_CLE_0_CT1QCFG:
   case V3D_CLE_0_CT1QTSKIP:
   case V3D_CLE_0_CT1QBASE0:
   case V3D_CLE_0_CT1QBASE1:
   case V3D_CLE_0_CT1QBASE2:
   case V3D_CLE_0_CT1QBASE3:
   case V3D_CLE_0_CT1QBASE4:
   case V3D_CLE_0_CT1QBASE5:
   case V3D_CLE_0_CT1QBASE6:
   case V3D_CLE_0_CT1QBASE7:
   case V3D_CLE_0_CT1QSYNC:
   case V3D_CLE_0_CT1CAD:
#endif

#if V3D_VER_EQUAL(3, 3)
   case V3D_CLE_0_CT1CS:
   case V3D_CLE_0_CT1EA:
   case V3D_CLE_0_CT1CA:
   case V3D_CLE_0_CT1RA0:
   case V3D_CLE_0_CT1LC:
   case V3D_CLE_0_CT1PC:
   case V3D_CLE_0_CT1CFG:
   case V3D_CLE_0_CT1TILECT:
   case V3D_CLE_0_CT1TSKIP:
   case V3D_CLE_0_CT1PTCT:
   case V3D_CLE_0_CT1SYNC:
   case V3D_CLE_0_CT1QBA:
   case V3D_CLE_0_CT1QEA:
   case V3D_CLE_0_CT1QCFG:
   case V3D_CLE_0_CT1QTSKIP:
   case V3D_CLE_0_CT1QBASE0:
   case V3D_CLE_0_CT1QBASE1:
   case V3D_CLE_0_CT1QBASE2:
   case V3D_CLE_0_CT1QBASE3:
   case V3D_CLE_0_CT1QBASE4:
   case V3D_CLE_0_CT1QBASE5:
   case V3D_CLE_0_CT1QBASE6:
   case V3D_CLE_0_CT1QBASE7:
   case V3D_CLE_0_CT1QSYNC:
   case V3D_CLE_0_CT1CAD:
#endif

#if V3D_VER_EQUAL(3, 4)
   case V3D_CLE_0_CT1CS:
   case V3D_CLE_0_CT1EA:
   case V3D_CLE_0_CT1CA:
   case V3D_CLE_0_CT1RA0:
   case V3D_CLE_0_CT1LC:
   case V3D_CLE_0_CT1PC:
   case V3D_CLE_0_CT1CFG:
   case V3D_CLE_0_CT1TILECT:
   case V3D_CLE_0_CT1TSKIP:
   case V3D_CLE_0_CT1PTCT:
   case V3D_CLE_0_CT1SYNC:
   case V3D_CLE_0_CT1QBA:
   case V3D_CLE_0_CT1QEA:
   case V3D_CLE_0_CT1QCFG:
   case V3D_CLE_0_CT1QTSKIP:
   case V3D_CLE_0_CT1QBASE0:
   case V3D_CLE_0_CT1QBASE1:
   case V3D_CLE_0_CT1QBASE2:
   case V3D_CLE_0_CT1QBASE3:
   case V3D_CLE_0_CT1QBASE4:
   case V3D_CLE_0_CT1QBASE5:
   case V3D_CLE_0_CT1QBASE6:
   case V3D_CLE_0_CT1QBASE7:
   case V3D_CLE_0_CT1QSYNC:
   case V3D_CLE_0_CT1CAD:
#endif

#if V3D_VER_EQUAL(7, 0)
   case V3D_CLE_0_CT1CS:
   case V3D_CLE_0_CT1EA:
   case V3D_CLE_0_CT1CA:
   case V3D_CLE_0_CT1RA0:
   case V3D_CLE_0_CT1LC:
   case V3D_CLE_0_CT1PC:
   case V3D_CLE_0_CT1CFG:
   case V3D_CLE_0_CT1TILECT:
   case V3D_CLE_0_CT1TSKIP:
   case V3D_CLE_0_CT1PTCT:
   case V3D_CLE_0_CT1SYNC:
   case V3D_CLE_0_CT1QBA:
   case V3D_CLE_0_CT1QEA:
   case V3D_CLE_0_CT1QCFG:
   case V3D_CLE_0_CT1QTSKIP:
   case V3D_CLE_0_CT1QBASE0:
   case V3D_CLE_0_CT1QBASE1:
   case V3D_CLE_0_CT1QBASE2:
   case V3D_CLE_0_CT1QBASE3:
   case V3D_CLE_0_CT1QBASE4:
   case V3D_CLE_0_CT1QBASE5:
   case V3D_CLE_0_CT1QBASE6:
   case V3D_CLE_0_CT1QBASE7:
   case V3D_CLE_0_CT1QSYNC:
   case V3D_CLE_0_CT1CAD:
#endif
   /* END AUTO-GENERATED CODE (v3d_is_render_cle_reg_cases) */
      return true;
   default:
      return false;
   }
}
