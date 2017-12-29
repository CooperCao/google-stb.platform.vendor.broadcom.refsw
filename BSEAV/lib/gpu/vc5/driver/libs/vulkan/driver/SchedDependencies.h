/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Common.h"
#include "libs/platform/v3d_scheduler.h"

namespace bvk {

// Helper class that wraps v3d_scheduler_deps and adds some useful
// methods.
class SchedDependencies : public v3d_scheduler_deps
{
public:
   SchedDependencies() { n = 0; }
   SchedDependencies(JobID id)
   {
      n = 0;
      if (id > 0)
         v3d_scheduler_deps_set(this, id);
   }

   SchedDependencies(const SchedDependencies &rhs)
   {
      v3d_scheduler_copy_deps(this, &rhs);
   }


   SchedDependencies &operator=(const SchedDependencies &rhs)
   {
      v3d_scheduler_copy_deps(this, &rhs);
      return *this;
   }

   SchedDependencies &operator+=(JobID id)
   {
      if (id > 0)
      {
         v3d_scheduler_add_dep(this, id);
         //printf("%p += %" PRIu64 " (n=%u)\n", this, id, n);
      }
      return *this;
   }

   SchedDependencies &operator+=(const SchedDependencies &deps)
   {
      v3d_scheduler_merge_deps(this, &deps);
      //printf("%p += %p (n=%u)\n", this, &deps, n);
      return *this;
   }

   JobID Amalgamate()
   {
      JobID id = 0;
      if (n > 1)
      {
        // printf("%p Amalgamate (n was %u)\n", this, n);

         id = v3d_scheduler_submit_null_job(this, nullptr, nullptr);
         *this = SchedDependencies(id);
      }
      else if (n == 1)
         id = dependency[0];

      return id;
   }
};

} // namespace bvk
