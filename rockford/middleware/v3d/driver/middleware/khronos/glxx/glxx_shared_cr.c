/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Common code shared between GL/VG
=============================================================================*/
/*
   MEM_HANDLE_T glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool create)

   Obtains a handle to a buffer object, given a shared state object, optionally
   creating a new one should it not already exist

   Returns either a valid handle, or MEM_INVALID_HANDLE.  If create is false, MEM_INVALID_HANDLE
   means the buffer did not exist.  If create is true, MEM_INVALID_HANDLE means insufficient memory

   Khronos documentation:

   -

   Implementation notes:

   -

   Preconditions:

   shared is a valid pointer to a shared state object
   If create == true then buffer != 0

   Postconditions:

   returns either a valid handle to a GLXX_BUFFER_T object or MEM_INVALID_HANDLE

   Invariants preserved:

   shared.buffers is a valid map and the elements are valid BUFFER_Ts

*/

MEM_HANDLE_T glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool create)
{
   MEM_HANDLE_T handle = khrn_map_lookup(&shared->buffers, buffer);

   if (create && handle == MEM_INVALID_HANDLE) {
      handle = MEM_ALLOC_STRUCT_EX(GLXX_BUFFER_T, MEM_COMPACT_DISCARD);                  // check, glxx_buffer_term

      vcos_assert(buffer);

      if (handle != MEM_INVALID_HANDLE) {
         mem_set_term(handle, glxx_buffer_term);

         glxx_buffer_init((GLXX_BUFFER_T *)mem_lock(handle, NULL), buffer);
         mem_unlock(handle);

         if (khrn_map_insert(&shared->buffers, buffer, handle))
            mem_release(handle);
         else {
            mem_release(handle);
            handle = MEM_INVALID_HANDLE;
         }
      }
   }

   return handle;
}

/*
   void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer)

   Removes the specified buffer from the shared object.

   Khronos documentation:

   -

   Implementation notes:

   -

   Preconditions:

   shared is a valid pointer to a shared state object
   buffer != 0
   shared->buffers contains buffer

   Postconditions:

   -

   Invariants preserved:

   shared.buffers is a valid map and the elements are valid BUFFER_Ts

*/



void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer)
{
   vcos_assert(buffer != 0);
   khrn_map_delete(&shared->buffers, buffer);
}

/*
   MEM_HANDLE_T glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture)

   Obtains a handle to a texture object, given a shared state object

   Returns either a valid handle, or MEM_INVALID_HANDLE.  MEM_INVALID_HANDLE
   means the texture did not exist.

   Khronos documentation:

   -

   Implementation notes:

   -

   Preconditions:

   shared is a valid pointer to a shared state object

   Postconditions:

   returns either a valid handle to a GLXX_TEXTURE_T object or MEM_INVALID_HANDLE

   Invariants preserved:

   -
*/

MEM_HANDLE_T glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture)
{
   return khrn_map_lookup(&shared->textures, texture);
}


/*
   void glxx_shared_delete_texture(GLXX_SHARED_T *shared, uint32_t texture)

   Removes the specified texture from the shared object.

   Khronos documentation:

   -

   Implementation notes:

   -

   Preconditions:

   shared is a valid pointer to a shared state object
   texture != 0
   shared->textures contains texture

   Postconditions:

   -

   Invariants preserved:

   shared.textures is a valid map and the elements are valid TEXTURE_Ts

*/

void glxx_shared_delete_texture(GLXX_SHARED_T *shared, uint32_t texture)
{
   vcos_assert(texture != 0);
   khrn_map_delete(&shared->textures, texture);
}
