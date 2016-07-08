/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BACKEND_UNIFORMS_H_INCLUDED
#define GLSL_BACKEND_UNIFORMS_H_INCLUDED

/* Different types of uniform */
typedef enum {
   BACKEND_UNIFORM_UNASSIGNED = 0,
   BACKEND_UNIFORM_PLAIN,
   BACKEND_UNIFORM_LITERAL,
   BACKEND_UNIFORM_ADDRESS,
   BACKEND_UNIFORM_UBO_ADDRESS,
   BACKEND_UNIFORM_SSBO_ADDRESS,
   BACKEND_UNIFORM_ATOMIC_ADDRESS,
   BACKEND_UNIFORM_SSBO_SIZE,
   BACKEND_UNIFORM_TEX_PARAM0,
   BACKEND_UNIFORM_TEX_PARAM1,
   BACKEND_UNIFORM_TEX_SIZE_X,
   BACKEND_UNIFORM_TEX_SIZE_Y,
   BACKEND_UNIFORM_TEX_SIZE_Z,
#if !V3D_HAS_NEW_TMU_CFG
   BACKEND_UNIFORM_TEX_BASE_LEVEL,
   BACKEND_UNIFORM_TEX_BASE_LEVEL_FLOAT,
#endif
   BACKEND_UNIFORM_IMAGE_PARAM0,
   BACKEND_UNIFORM_IMAGE_PARAM1,
   BACKEND_UNIFORM_IMAGE_ARR_STRIDE,
   BACKEND_UNIFORM_IMAGE_SWIZZLING,
   BACKEND_UNIFORM_IMAGE_LX_ADDR,
   BACKEND_UNIFORM_IMAGE_LX_PITCH,
   BACKEND_UNIFORM_IMAGE_LX_SLICE_PITCH,
   BACKEND_UNIFORM_IMAGE_LX_WIDTH,
   BACKEND_UNIFORM_IMAGE_LX_HEIGHT,
   BACKEND_UNIFORM_IMAGE_LX_DEPTH,
   BACKEND_UNIFORM_SPECIAL,
   BACKEND_UNIFORM_LAST_ELEMENT = BACKEND_UNIFORM_SPECIAL
} BackendUniformFlavour;

/* Valid values for BACKEND_UNIFORM_SPECIALs. */
typedef enum {
   BACKEND_SPECIAL_UNIFORM_VP_SCALE_X,
   BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y,
   BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z,
   BACKEND_SPECIAL_UNIFORM_VP_SCALE_W,
   BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR,
   BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR,
   BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF,
   BACKEND_SPECIAL_UNIFORM_SAMPLE_MASK,
   BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X,
   BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Y,
   BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Z,
   BACKEND_SPECIAL_UNIFORM_SHARED_PTR,
   BACKEND_SPECIAL_UNIFORM_QUORUM,
   BACKEND_SPECIAL_UNIFORM_LAST_ELEMENT = BACKEND_SPECIAL_UNIFORM_QUORUM
} BackendSpecialUniformFlavour;

#endif
