/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#if EMBEDDED_SETTOP_BOX

#include "EGL/begl_memplatform.h"
#include "EGL/begl_schedplatform.h"

namespace bvk
{

class BVKPlatform
{
public:
   BVKPlatform() {}
   ~BVKPlatform();

   // It is inconvenient to auto register on construction due to
   // initialization ordering; this requires Nexus to be alive to
   // query the heaps when creating the memory interface.
   void Register();

private:
   BEGL_MemoryInterface *m_memoryInterface = nullptr;
   BEGL_SchedInterface  *m_schedInterface = nullptr;
};

}
#endif