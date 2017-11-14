/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "APIVersion.h"
#include "vulkan.h"

namespace bvk {

// Current supported API version in this driver
static APIVersion s_curAPIVersion(1, 0, VK_HEADER_VERSION);

const APIVersion &bvk::APIVersion::CurAPIVersion()
{
   return s_curAPIVersion;
}

} // namespace bvk
