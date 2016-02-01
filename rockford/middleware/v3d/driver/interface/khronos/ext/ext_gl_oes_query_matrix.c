/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side implementation of the GL_OES_query_matrix extension.
http://www.khronos.org/registry/gles/extensions/OES/OES_query_matrix.txt
=============================================================================*/

#define GL_GLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/glxx/glxx_client.h"

#include "interface/khronos/glxx/glxx_int_impl.h"
#include "interface/khronos/glxx/gl11_int_impl.h"

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"

GL_API GLbitfield GL_APIENTRY glQueryMatrixxOES( GLfixed mantissa[16], GLint exponent[16] )
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   if (IS_OPENGLES_11(thread)) {
      int i;
      glQueryMatrixxOES_impl_11(mantissa);
      for(i=0;i<16;i++)
         exponent[i] = 0;

      return 0;
   }

   return 0xff; /* all components invalid */
}
