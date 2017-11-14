/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
#pragma once

#include "formats_360.h"

namespace video_texturing
{

const char *VertexShaderStr(bool is360, Format360 fmt);
const char *FragmentShaderStr(bool is360, Format360 fmt);

} // namespace