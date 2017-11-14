/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "vulkan.h"

namespace bvk {

class ProcAddressFinder
{
public:
   ProcAddressFinder();

   PFN_vkVoidFunction GetProcAddress(const char *name) const;

private:
 #ifdef WIN32
   void *m_moduleHandle = nullptr;
 #endif
};

} // namespace bvk
