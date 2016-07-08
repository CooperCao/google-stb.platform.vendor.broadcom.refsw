/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "v3d_common.h"
#include "v3d_ident.h"
#include "v3d_limits.h"
#include "libs/util/demand.h"
#include "libs/util/log/log.h"

int v3d_ver_from_ident(const V3D_IDENT_T *ident)
{
   return V3D_MAKE_VER(
      ident->v3d_tech_version,
      ident->v3d_revision,
      ident->v3d_sub_rev,
      V3D_HIDDEN_REV); /* Note this is by definition not provided by hardware! */
}

int v3d_ver_from_hub_ident(const V3D_HUB_IDENT_T *ident)
{
   uint32_t v3d_sub_rev = ident->v3d_sub_rev;
   if ((ident->v3d_tech_version == 3) && (V3D_TECH_VERSION == 3) &&
      (ident->v3d_revision == 2) && (V3D_REVISION == 2) &&
      (v3d_sub_rev == 0) && (V3D_SUB_REV == 1))
      /* 3.2.1 (special FPGA build) reports as 3.2.0 in hub; fix this up... */
      v3d_sub_rev = 1;
   return V3D_MAKE_VER(
      ident->v3d_tech_version,
      ident->v3d_revision,
      v3d_sub_rev,
      V3D_HIDDEN_REV); /* Note this is by definition not provided by hardware! */
}

static void check_v3d_ver(int v3d_ver)
{
   demand_msg(v3d_ver == V3D_VER,
      "V3D version of hardware (%u.%u.%u) does not match what software was compiled with (%u.%u.%u)!",
      V3D_EXTRACT_TECH_VERSION(v3d_ver), V3D_EXTRACT_REVISION(v3d_ver),
      V3D_EXTRACT_SUB_REV(v3d_ver), V3D_TECH_VERSION, V3D_REVISION, V3D_SUB_REV);
}

void v3d_check_ident(const V3D_IDENT_T *ident, uint32_t core)
{
   /* v3d_unpack_ident() does a bunch of checking itself, so there isn't much
    * checking left to do here... */

   check_v3d_ver(v3d_ver_from_ident(ident));

   assert(ident->core_index == core);

   assert(ident->num_qpus_per_slice <= V3D_MAX_QPUS_PER_SLICE);
   uint32_t num_qpus = ident->num_slices * ident->num_qpus_per_slice;
   assert(num_qpus >= 1);
   assert(num_qpus <= V3D_MAX_QPUS_PER_CORE);
}

void v3d_check_hub_ident(const V3D_HUB_IDENT_T *ident)
{
   /* v3d_unpack_hub_ident() does a bunch of checking itself, so there isn't
    * much checking left to do here... */

   check_v3d_ver(v3d_ver_from_hub_ident(ident));

   assert(ident->num_cores <= V3D_MAX_CORES);
}
