/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glxx/gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "../common/khrn_int_util.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_server_internal.h"

/*
   Khronos Documentation:
   Overview

    Many applications may need to query the contents and status of the
    current matrix at least for debugging purposes, especially as the
    implementations are allowed to implement matrix machinery either in
    any (possibly proprietary) floating point format, or in a fixed point
    format that has the range and accuracy of at least 16.16 (signed 16 bit
    integer part, unsigned 16 bit fractional part).

    This extension is intended to allow application to query the components
    of the matrix and also their status, regardless whether the internal
    representation is in fixed point or floating point.

New Procedures and Functions

    GLbitfield glQueryMatrixxOES( GLfixed mantissa[16],
                                  GLint   exponent[16] )

    mantissa[16] contains the contents of the current matrix in GLfixed
    format.  exponent[16] contains the unbiased exponents applied to the
    matrix components, so that the internal representation of component i
    is close to mantissa[i] * 2^exponent[i].  The function returns a status
    word which is zero if all the components are valid. If
    status & (1<<i) != 0, the component i is invalid (e.g., NaN, Inf).
    The implementations are not required to keep track of overflows.  In
    that case, the invalid bits are never set.

Implementation Notes:
   exponents are fixed at 16
   all components are valid
   we return the same values returned by glGetFixedv(pname, params);
   with pname set to the current matrix mode
*/

GL_APICALL GLbitfield GL_APIENTRY glQueryMatrixxOES ( GLfixed mantissa[16], GLint exponent[16] )
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   if (!state) return 0xff;

   GLenum pname = 0;
   switch(state->gl11.matrix_mode)
   {
      case GL_TEXTURE:
         pname = GL_TEXTURE_MATRIX;
         break;
      case GL_MODELVIEW:
         pname = GL_MODELVIEW_MATRIX;
         break;
      case GL_PROJECTION:
         pname = GL_PROJECTION_MATRIX;
         break;
      case GL_MATRIX_PALETTE_OES:
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);      /* Not defined what we should do, this seems sensible */
         return 0xff;
      default:
         unreachable();
   }

   verif(glxx_get_fixeds(state, pname, mantissa) == GL_NO_ERROR);
   for (unsigned i=0; i < 16; i++)
      exponent[i] = 0;

   glxx_unlock_server_state();
   return 0;
}
