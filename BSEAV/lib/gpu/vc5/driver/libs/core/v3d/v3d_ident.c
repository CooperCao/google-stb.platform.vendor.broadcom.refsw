/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_ident.h"
#include "v3d_limits.h"
#include "libs/util/gfx_options/gfx_options.h"
#include "libs/util/demand.h"
#include "libs/util/log/log.h"
#include "libs/util/snprintf.h"

LOG_DEFAULT_CAT("v3d_ident")

#if !V3D_VER_AT_LEAST(4,1,34,0)
int v3d_ver_from_ident(const V3D_IDENT_T *ident)
{
   return V3D_MAKE_VER(
      ident->tech_version,
      ident->revision,
      ident->sub_rev,
      0);
}
#endif

int v3d_ver_from_hub_ident(const V3D_HUB_IDENT_T *ident)
{
   uint32_t sub_rev = ident->sub_rev;
#if V3D_VER == V3D_MAKE_VER(3,2,1,0)
   if ((ident->tech_version == 3) && (ident->revision == 2) && (sub_rev == 0))
      /* 3.2.1 (special FPGA build) reports as 3.2.0 in hub; fix this up... */
      sub_rev = 1;
#endif
   return V3D_MAKE_VER(
      ident->tech_version,
      ident->revision,
      sub_rev,
      ident->compat_rev);
}

void v3d_check_ver(int v3d_ver)
{
   static unsigned no_ver_check = ~0u;
   if (no_ver_check == ~0u)
      no_ver_check = gfx_options_bool("V3D_NO_VER_CHECK", false);

   if (!no_ver_check)
   {
      uint32_t hw_tech_version = V3D_EXTRACT_TECH_VERSION(v3d_ver);
      uint32_t hw_revision = V3D_EXTRACT_REVISION(v3d_ver);
      uint32_t hw_sub_rev = V3D_EXTRACT_SUB_REV(v3d_ver);
      uint32_t hw_compat_rev = V3D_EXTRACT_COMPAT_REV(v3d_ver);

      bool compatible =
         (hw_tech_version == V3D_TECH_VERSION) &&
         (hw_revision == V3D_REVISION) &&
         (hw_sub_rev == V3D_SUB_REV) &&
         (hw_compat_rev >= V3D_COMPAT_REV);

      demand_msg(compatible,
         "V3D version of hardware (%u.%u.%u.%u) is not compatible with what software was compiled with (%u.%u.%u.%u)!",
         hw_tech_version, hw_revision, hw_sub_rev, hw_compat_rev,
         V3D_TECH_VERSION, V3D_REVISION, V3D_SUB_REV, V3D_COMPAT_REV);
   }
}

void v3d_check_ident(const V3D_IDENT_T *ident, uint32_t core)
{
   /* v3d_unpack_ident() does a bunch of checking itself, so there isn't much
    * checking left to do here... */

#if !V3D_VER_AT_LEAST(4,1,34,0)
   v3d_check_ver(v3d_ver_from_ident(ident));
#endif

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

   v3d_check_ver(v3d_ver_from_hub_ident(ident));

   assert(ident->num_cores <= V3D_MAX_CORES);
}

size_t v3d_sprint_device_name(char *buf, size_t buf_size, size_t offset, const V3D_IDENT_T *ident)
{
   // The format is "VideoCore XX HW (V3D-<H><S><T>)" where...
   // <H> is "5" for vc5 / "6" for vc6
   // <S> is the number of slices per core
   // <T> is (texture units per core - 1) * 5

   const char *vcx;
   switch (ident->tech_version)
   {
   case 3: vcx = "V";    break;    // VC5
   case 4: vcx = "VI";   break;    // VC6
   case 5: vcx = "VII";  break;
   case 6: vcx = "VIII"; break;
   case 7: vcx = "IX";   break;    // Future h/w dev tag
   default: unreachable();
   }

   return vcos_safe_sprintf(buf, buf_size, offset, "VideoCore %s HW (V3D-%u%u%u)",
            vcx, ident->tech_version + 2,
            ident->num_slices, (ident->num_tmus - 1) * 5);
}
