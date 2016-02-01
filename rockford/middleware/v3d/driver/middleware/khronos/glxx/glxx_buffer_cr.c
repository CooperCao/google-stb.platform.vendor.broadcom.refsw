/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Common code shared between GL/VG
=============================================================================*/
/*
   int glxx_buffer_get_size(GLXX_BUFFER_T *buffer)

   Return a buffer object's BUFFER_SIZE.

   Implementation notes:

   We rely on mem_get_size returning results consistent with mem_alloc. The buffer's
   size is not stored explicitly in the GLXX_BUFFER_T structure.

   Preconditions:

   buffer is a valid pointer

   Postconditions:

   result >= 0

   Invariants preserved:

   -
*/

int glxx_buffer_get_size(GLXX_BUFFER_T *buffer)
{
   return (int)mem_get_size(buffer->pool[buffer->current_item].mh_storage);
}
