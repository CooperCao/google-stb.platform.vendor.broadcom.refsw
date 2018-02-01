/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_reg_print.h"
#include "v3d_regs.h"
#include "v3d_gen.h"

v3d_reg_print_fn_t v3d_maybe_reg_print_fn(uint32_t addr)
{
   if (v3d_is_core_reg(addr))
      switch (v3d_reg_to_core0(addr))
      {
      case V3D_CTL_0_IDENT0:        return v3d_print_ident0;
      case V3D_CTL_0_IDENT1:        return v3d_print_ident1;
      case V3D_CTL_0_IDENT2:        return v3d_print_ident2;
      case V3D_CTL_0_IDENT3:        return v3d_print_ident3;
      case V3D_CTL_0_INT_STS:
#if V3D_VER_AT_LEAST(4,1,34,0)
      case V3D_CTL_0_INT_STS_MASKED:
#endif
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
#if V3D_VER_AT_LEAST(4,1,34,0)
      case V3D_QPS_0_SRQPC:         return v3d_print_srqpc;
#endif
      case V3D_QPS_0_SRQCS:         return v3d_print_srqcs;
      case V3D_GMP_0_CFG:           return v3d_print_gmp_cfg;
#if V3D_VER_AT_LEAST(3,3,0,0)
      case V3D_GMP_0_STATUS:        return v3d_print_gmp_status;
      case V3D_GMP_0_VIO_TYPE:      return v3d_print_gmp_vio_type;
#else
      case V3D_GMP_0_CS:            return v3d_print_gmp_cs;
      case V3D_GMP_0_RVT:
      case V3D_GMP_0_WVT:           return v3d_print_gmp_vio_type;
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
      case V3D_CSD_0_STATUS:        return v3d_print_csd_status;
      case V3D_CSD_0_QUEUED_CFG0:   return v3d_print_csd_cfg0;
      case V3D_CSD_0_QUEUED_CFG1:   return v3d_print_csd_cfg1;
      case V3D_CSD_0_QUEUED_CFG2:   return v3d_print_csd_cfg2;
      case V3D_CSD_0_QUEUED_CFG3:   return v3d_print_csd_cfg3;
      case V3D_CSD_0_QUEUED_CFG4:   return v3d_print_csd_cfg4;
      case V3D_CSD_0_QUEUED_CFG5:   return v3d_print_csd_cfg5;
      case V3D_CSD_0_QUEUED_CFG6:   return v3d_print_csd_cfg6;
      case V3D_CSD_0_CURRENT_CFG0:  return v3d_print_csd_cfg0;
      case V3D_CSD_0_CURRENT_CFG1:  return v3d_print_csd_cfg1;
      case V3D_CSD_0_CURRENT_CFG2:  return v3d_print_csd_cfg2;
      case V3D_CSD_0_CURRENT_CFG3:  return v3d_print_csd_cfg3;
      case V3D_CSD_0_CURRENT_CFG4:  return v3d_print_csd_cfg4;
      case V3D_CSD_0_CURRENT_CFG5:  return v3d_print_csd_cfg5;
      case V3D_CSD_0_CURRENT_CFG6:  return v3d_print_csd_cfg6;
      case V3D_CSD_0_CURRENT_ID0:   return v3d_print_csd_id0;
      case V3D_CSD_0_CURRENT_ID1:   return v3d_print_csd_id1;
#endif
      default:                      return NULL;
      }
   else
      switch (addr)
      {
#if V3D_VER_AT_LEAST(4,1,34,0)
      case V3D_HUB_CTL_INT_STS_MASKED:
#endif
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
#if V3D_VER_AT_LEAST(3,3,0,0)
      // TSY regs not accessible on 3.2; see GFXH-1683
      case V3D_TSY_CONFIG0:      return v3d_print_tsy_config0;
      case V3D_TSY_CONFIG1:      return v3d_print_tsy_config1;
      case V3D_TSY_CONTROL:      return v3d_print_tsy_control;
      case V3D_TSY_STATUS:       return v3d_print_tsy_status;
      case V3D_TSY_TSO_DATA:     return v3d_print_tsy_tso_data;
      case V3D_MMU_CTRL:         return v3d_print_mmu_ctrl;
#if !V3D_VER_AT_LEAST(4,1,34,0)
      case V3D_MMU_T_CTRL:       return v3d_print_mmu_ctrl;
#endif
#endif
      default:                   return NULL;
      }
}
