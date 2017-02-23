/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 texture unit structure declaration.
=============================================================================*/

#ifndef GL11_TEXUNIT_H
#define GL11_TEXUNIT_H

#include "interface/khronos/include/GLES/gl.h"

/*
   The state required for the current texture environment, for each texture unit,
   consists of

      X a six-valued integer indicating the texture function
      X an eight-valued integer indicating the RGB combiner function
      X a six-valued integer indicating the ALPHA combiner function
      X six four-valued integers indicating the combiner RGB and ALPHA source arguments
      X three four-valued integers indicating the combiner RGB operands
      X three two-valued integers indicating the combiner ALPHA operands,
      X four floating-point environment color values
      X two three-valued floating-point scale factors
*/

typedef struct {
   GLenum combine;
   GLenum source[3];
   GLenum operand[3];

   GLfloat scale;
} GL11_COMBINER_T;

typedef struct {
   bool active;
   bool complex;
   bool has_color;
   bool has_alpha;
} GL11_TEXUNIT_PROPS_T;

typedef struct {
   /*
    * GL_NONE for disabled, otherwise GL_TEXTURE_2D or GL_TEXTURE_EXTERNAL_OES
    */
   GLenum target_enabled;

   GLfloat color[4];

   GL11_COMBINER_T rgb;
   GL11_COMBINER_T alpha;

   /*
      Texture matrix stack for this unit

      Khronos name: TEXTURE_MATRIX
   */

   GL11_MATRIX_STACK_T stack;

   /*
      jeremyt 1/4/2010
      mh_bound_texture moved to state.bound_texture[i].twod
   */

   //cache of current matrix for install uniforms
   float current_matrix[16];
} GL11_TEXUNIT_T;

#endif
