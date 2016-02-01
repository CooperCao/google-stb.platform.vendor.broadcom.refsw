/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "v3d_common.h"
#include "v3d_reg_print.h"
#include "v3d_regs.h"
#include "v3d_gen.h"

v3d_reg_print_fn_t v3d_maybe_reg_print_fn(uint32_t addr)
{
#if V3D_VER_EQUAL(3,3)
   if (v3d_is_mmu_reg(addr))
      switch(v3d_mmu_reg_to_core0(addr))
      {
      case V3D_MMU_0_CTRL: return v3d_print_mmu_ctrl;
      default: return NULL;
      }
   else
#endif
   if (v3d_is_hub_reg(addr))
      switch (addr)
      {
      case V3D_HUB_CTL_INT_STS:  return v3d_print_hub_intr;
      case V3D_HUB_CTL_IDENT0:   return v3d_print_hub_ident0;
      case V3D_HUB_CTL_IDENT1:   return v3d_print_hub_ident1;
      case V3D_HUB_CTL_IDENT2:   return v3d_print_hub_ident2;
      case V3D_HUB_CTL_IDENT3:   return v3d_print_hub_ident3;
      case V3D_TFU_SU:           return v3d_print_tfusu;
      case V3D_TFU_ICFG:         return v3d_print_tfuicfg;
      case V3D_TFU_IIS:          return v3d_print_tfuiis;
      case V3D_TFU_IOA:          return v3d_print_tfuioa;
      case V3D_TFU_IOS:          return v3d_print_tfuios;
      case V3D_TFU_COEF0:        return v3d_print_tfucoef0;
      case V3D_TFU_COEF1:        return v3d_print_tfucoef1;
      case V3D_TFU_COEF2:        return v3d_print_tfucoef2;
      case V3D_TFU_COEF3:        return v3d_print_tfucoef3;
#if V3D_VER_AT_LEAST(3,4)
      case V3D_MMU_CTRL:         return v3d_print_mmu_ctrl;
#endif
#if V3D_VER_EQUAL(3,3)
      case V3D_MMU_T_CTRL:       return v3d_print_mmu_ctrl;
#endif
      default:                   return NULL;
      }
   else
      switch (v3d_reg_to_core0(addr))
      {
      case V3D_CTL_0_IDENT0:        return v3d_print_ident0;
      case V3D_CTL_0_IDENT1:        return v3d_print_ident1;
      case V3D_CTL_0_IDENT2:        return v3d_print_ident2;
      case V3D_CTL_0_IDENT3:        return v3d_print_ident3;
      case V3D_CTL_0_INT_STS:
      case V3D_CTL_0_INT_SET:
      case V3D_CTL_0_INT_CLR:
      case V3D_CTL_0_INT_MSK_STS:
      case V3D_CTL_0_INT_MSK_SET:
      case V3D_CTL_0_INT_MSK_CLR:   return v3d_print_intr;
      case V3D_CTL_0_UIFCFG:        return v3d_print_uifcfg;
      case V3D_CLE_0_CT0CS:         return v3d_print_ct0cs;
      case V3D_CLE_0_CT1CS:         return v3d_print_ct1cs;
      case V3D_CLE_0_CT1CFG:        return v3d_print_ct1cfg;
      case V3D_CLE_0_PCS:           return v3d_print_pcs;
      default:                      return NULL;
      }
}
