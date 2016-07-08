/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Support functions (mostly relating to shaders) for 2708 GLES2.0
implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_color.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_server.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"

#include "middleware/khronos/glxx/2708/glxx_tu_4.h"
#include "middleware/khronos/gl20/2708/gl20_support_4.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"
#include "middleware/khronos/glxx/glxx_hw.h"

void gl20_hw_iu_init(GL20_HW_INDEXED_UNIFORM_T *iu)
{
   iu->count = 0;
}

void gl20_hw_iu_close(GL20_HW_INDEXED_UNIFORM_T *iu)
{
   UNUSED(iu);
}
