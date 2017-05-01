/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_mem.h"
#include "vcos.h"
#include "khrn_int_util.h"

khrn_mem_handle_t khrn_mem_handle_empty_string;

khrn_mem_handle_t khrn_mem_alloc_ex(size_t size, const char *desc, bool init, bool resizeable, bool discardable)
{
   /* Ignore resizeable and discardable */
   uint8_t *alloc = malloc(size + sizeof(khrn_mem_header));
   if (!alloc)
   {
      return KHRN_MEM_HANDLE_INVALID;
   }

   khrn_mem_header *header = (khrn_mem_header *)alloc;
   header->magic = KHRN_MEM_HEADER_MAGIC;
   header->ref_count = 1;
   header->term = NULL;
   header->size = size;

   void *result = (uint8_t *)alloc + sizeof(khrn_mem_header);
   if (init)
   {
      khrn_memset(result, 0, size);
   }

   assert(header == khrn_mem_get_header(result));

   return result;
}

void khrn_mem_release(khrn_mem_handle_t handle)
{
   if (handle == KHRN_MEM_HANDLE_INVALID)
      return;

   khrn_mem_header *header = khrn_mem_get_header(handle);
   int before_dec = vcos_atomic_dec(&header->ref_count);
   assert(before_dec > 0);

   if (before_dec == 1)
   {
      if (header->term)
         header->term(handle, header->size);

      free(header);
   }
}

bool khrn_mem_try_release(khrn_mem_handle_t handle)
{
   if (handle == KHRN_MEM_HANDLE_INVALID)
      return false;

   khrn_mem_header *header = khrn_mem_get_header(handle);
   assert(header->ref_count > 0);
   if (header->ref_count == 1)
      return false;

   header->ref_count--;
   return true;
}

bool khrn_mem_resize(khrn_mem_handle_t *handle_inout, size_t size)
{
   khrn_mem_handle_t handle = *handle_inout;
   assert(handle != KHRN_MEM_HANDLE_INVALID);

   khrn_mem_header *header = khrn_mem_get_header(handle);
   assert(header->ref_count == 1);
   size_t old_size = header->size;

   if (old_size == size)
      return true;

   /* TODO: realloc? */
   khrn_mem_handle_t new_handle = khrn_mem_alloc_ex(size, "realloc", false, true, false);
   if (new_handle == KHRN_MEM_HANDLE_INVALID)
      return false;

   void *src = khrn_mem_lock(handle);
   void *dst = khrn_mem_lock(new_handle);
   if (size > old_size)
   {
      khrn_memcpy(dst, src, old_size);
      khrn_memset((uint8_t *)dst + old_size, 0, size - old_size);
   }
   else
   {
      khrn_memcpy(dst, src, size);
   }
   khrn_mem_unlock(handle);
   khrn_mem_unlock(new_handle);
   khrn_mem_release(handle);
   *handle_inout = new_handle;
   return true;
}

bool khrn_mem_init(void)
{
   khrn_mem_handle_empty_string = khrn_mem_strdup("");
   if (khrn_mem_handle_empty_string == KHRN_MEM_HANDLE_INVALID)
      return false;

   return true;
}

void khrn_mem_term(void)
{
   khrn_mem_release(khrn_mem_handle_empty_string);
}

khrn_mem_handle_t khrn_mem_strdup(const char *str)
{
   khrn_mem_handle_t handle = khrn_mem_alloc_ex(strlen(str) + 1, "khrn_mem_strdup", false, false, false);
   if (handle == KHRN_MEM_HANDLE_INVALID)
      return KHRN_MEM_HANDLE_INVALID;

   vcos_safe_strcpy((char*)khrn_mem_lock(handle), str, strlen(str)+1, 0);
   khrn_mem_unlock(handle);
   return handle;
}