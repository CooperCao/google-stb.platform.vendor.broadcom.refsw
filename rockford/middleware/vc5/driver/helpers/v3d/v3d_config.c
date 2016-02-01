/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_config.h"
#include "helpers/v3d/v3d_limits.h"
#include "helpers/assert.h"
#include "helpers/assert.h"

static void check_v3d_tech_version(uint32_t hw_tech_version)
{
   assert_msg(hw_tech_version == V3D_TECH_VERSION,
      "V3D tech version of hardware (%u) does not match what software was compiled with (%u)!",
      hw_tech_version, V3D_TECH_VERSION);
}

static void check_v3d_revision(uint32_t hw_revision)
{
   assert_msg(hw_revision == V3D_REVISION,
      "V3D revision of hardware (%u) does not match what software was compiled with (%u)!",
      hw_revision, V3D_REVISION);
}

void v3d_check_ident(const V3D_IDENT_T *ident, uint32_t core)
{
   /* v3d_unpack_ident() does a bunch of checking itself, so there isn't much
    * checking left to do here... */

   check_v3d_tech_version(ident->v3d_tech_version);
   check_v3d_revision(ident->v3d_revision);

   assert(ident->core_index == core);

   assert(ident->num_qpus_per_slice <= V3D_MAX_QPUS_PER_SLICE);
   uint32_t num_qpus = ident->num_slices * ident->num_qpus_per_slice;
   assert(num_qpus >= V3D_MIN_QPUS_PER_CORE);
   assert(num_qpus <= V3D_MAX_QPUS_PER_CORE);
}

void v3d_check_hub_ident(const V3D_HUB_IDENT_T *ident)
{
   /* v3d_unpack_hub_ident() does a bunch of checking itself, so there isn't
    * much checking left to do here... */

   check_v3d_tech_version(ident->v3d_tech_version);
   check_v3d_revision(ident->v3d_revision);

   assert(ident->num_cores <= V3D_MAX_CORES);
}
