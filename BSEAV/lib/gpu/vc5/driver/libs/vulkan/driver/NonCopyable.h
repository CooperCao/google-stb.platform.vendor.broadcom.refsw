/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

namespace bvk
{

// Abstract base class that prevents object from being copied
class NonCopyable
{
public:
   NonCopyable() = default;
   NonCopyable(const NonCopyable &) = delete;
   NonCopyable &operator=(const NonCopyable &) = delete;
};

} // namespace bvk
