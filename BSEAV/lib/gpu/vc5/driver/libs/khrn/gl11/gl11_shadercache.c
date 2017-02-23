/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Handles vertex and fragment shader caches.
=============================================================================*/
#include "gl11_shader.h"
#include "gl11_shadercache.h"

#include "../glxx/glxx_hw.h"

#include "../common/khrn_int_hash.h"

static void cache_entry_clear(GL11_CACHE_ENTRY_T *e)
{
   e->used = false;
   /* TODO: Tidy up the blob as well */
   glsl_ir_program_free(e->blob);
   e->blob = NULL;
   glxx_binary_cache_invalidate(&e->backend_cache);
}

static uint32_t hashkey(const GL11_CACHE_KEY_T *key)
{
   return khrn_hashword((const uint32_t *)key, sizeof(GL11_CACHE_KEY_T)/4, 0xa28abf82);
}

static uint32_t bump(uint32_t hash)
{
   return hash * 0x23456789;
}

static bool find_cache_slot(GL11_CACHE_ENTRY_T *cache,
                            GL11_CACHE_KEY_T *key,
                            uint32_t *cache_loc)
{
   bool found;
   uint32_t hash, hash_word, original_hash;

   hash_word = hashkey(key);
   hash = hash_word % GL11_CACHE_SIZE;
   original_hash = hash;

   /* If something's in the cache then we've found it ... */
   found = cache[hash].used;

   if (cache[hash].used && memcmp(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T)))
   {
      /* ... unless it turns out to be a conflict */
      hash_word = bump(hash_word);
      hash = hash_word % GL11_CACHE_SIZE;
      found = cache[hash].used;
      if (cache[hash].used && memcmp(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T)))
      {
         hash_word = bump(hash_word);
         hash = hash_word % GL11_CACHE_SIZE;
         found = cache[hash].used;
         if (cache[hash].used && memcmp(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T)))
         {
            /* Three attempts at finding a cache have all yielded something which is being used by someone else. */
            hash = original_hash;
            assert(cache[hash].used);
            found = false;
         }
      }
   }

   *cache_loc = hash;
   return found;
}

/* Public functions: */

void gl11_hw_shader_cache_reset(GL11_CACHE_ENTRY_T *cache)
{
   int i;
   for (i = 0; i < GL11_CACHE_SIZE; i++) {
      if (cache[i].used) cache_entry_clear(&cache[i]);
   }
}

GL11_CACHE_ENTRY_T *gl11_get_backend_cache(GL11_CACHE_ENTRY_T *cache,
                                           GL11_CACHE_KEY_T *key)
{
   uint32_t hash;
   bool found = find_cache_slot(cache, key, &hash);

   if (!found) {
      /* If a conflicting entry is here, clear it */
      if (cache[hash].used) cache_entry_clear(&cache[hash]);

      cache[hash].used = true;
      memcpy(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T));

      cache[hash].blob = gl11_shader_get_dataflow(&cache[hash].key);
   }

   return &cache[hash];
}
