/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
#pragma once

namespace video_texturing
{

// 360 video formats
enum Format360
{
   FORMAT_EQUIRECT     = 0, // equirectangular projection
   FORMAT_CUBE_32_0    = 1, // cube projection 3:2 rotation 0
   FORMAT_CUBE_32_90   = 2, // cube projection 3:2 rotation 90
   FORMAT_CUBE_32_270  = 3, // cube projection 3:2 rotation 270
   FORMAT_CUBE_32_P270 = 4, // cube projection 3:2 rotation p270
   FORMAT_CUBE_43_0    = 5, // cube projection 4:3 rotation 0
   FORMAT_FISHEYE      = 6, // fisheye projection
   FORMAT_ICOSAHEDRON  = 7, // icosahedron projection
   FORMAT_OCTAHEDRON   = 8, // octahedron projection
   FORMAT_EAP          = 9, // equal area projection
   FORMAT_TOTAL_NUM,        // number of supported formats
};

} // namespace