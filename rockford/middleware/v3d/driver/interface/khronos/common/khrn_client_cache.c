/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Cache

FILE DESCRIPTION
Implements the client end of the attribute cache.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/common/khrn_client_cache.h"
#include "interface/khronos/common/khrn_client_platform.h"

#include "interface/khronos/common/khrn_int_hash.h"
#include "interface/khronos/common/khrn_int_util.h"

#include "interface/khronos/glxx/glxx_int_impl.h"

/* for khrn_memcmp() */
#include "middleware/khronos/common/khrn_hw.h"

#include <assert.h>

static void link_insert(CACHE_LINK_T *link, CACHE_LINK_T *prev, CACHE_LINK_T *next)
{
   vcos_assert(prev->next == next);
   vcos_assert(next->prev == prev);

   link->prev = prev;
   link->next = next;
   prev->next = link;
   next->prev = link;
}

static void link_remove(CACHE_LINK_T *link)
{
   link->next->prev = link->prev;
   link->prev->next = link->next;
}

static void tree_init(uint8_t *tree, int depth)
{
   int i;

   tree[0] = depth + 1;

   for (i = 1; i < 1 << depth; i++)
      tree[i] = tree[i >> 1] - 1;
}

static int heap_avail(KHRN_CACHE_T *cache, int size)
{
   return cache->tree && cache->tree[1] >= size;
}

static int heap_alloc(KHRN_CACHE_T *cache, int size)
{
   int node, fixup;
   int i;

   assert(heap_avail(cache, size));

   /* Search down the tree for the lowest available block that is big enough
    * to accommodate the request.
    * Note that the sizes in each node will in general be underestimates (it is
    * the floor of log2 of the available memory below)
    */
   node = 1;
   for (i = 0; i < cache->client_depth - size; i++) {
      node <<= 1;
      /* If next node is big enough and this node is too small or this node is bigger than the next */
      if (cache->tree[node + 1] >= size && (cache->tree[node] < size || cache->tree[node] > cache->tree[node + 1]))
         node++;
   }

   /* Mark the node as empty */
   cache->tree[node] = 0;

   /* Traverse up the tree from the selected node.
    * If a node's neighbour value is less than its parent node, then recalculate the parent size
    * from it and its neighbour, then repeat for the parent.
    */
   for (fixup = node; cache->tree[fixup ^ 1] < cache->tree [fixup >> 1]; fixup >>= 1)
      cache->tree[fixup >> 1] = _max(cache->tree[fixup], cache->tree[fixup ^ 1]);

   return node * (1 << (size - 1)) - (1 << (cache->client_depth - 1));
}

static void heap_free(KHRN_CACHE_T *cache, int block)
{
   int node = block + (1 << (cache->client_depth - 1));
   int reset = 1;

   while (cache->tree[node] > 0) {
      node >>= 1;
      reset++;
   }

   cache->tree[node] = reset;

   while (cache->tree[node] == cache->tree[node ^ 1]) {
      node >>= 1;
      cache->tree[node] = cache->tree[node] + 1;
   }

   while (cache->tree[node] > cache->tree[node >> 1]) {
      cache->tree[node >> 1] = cache->tree[node];
      node >>= 1;
   }
}

static uint32_t hash(const void *data, int len, int sig)
{
   int hash;

//   if (len > 256)     // Essential for good performance on flying teapot, but kills Quake 3!!
//      len = 256;

   if (!((size_t)data & 3) && !(len & 3))
      hash = khrn_hashword((uint32_t *)data, len >> 2, 0);
   else
      hash = khrn_hashlittle(data, len, 0);

   return (hash & ~0xf) | sig;
}

static void clear_ptr_info_all(KHRN_CACHE_T *cache)
{
   uint32_t i;

   for (i = 0; i <= CACHE_SIG_INDEX; ++i)
   {
      CACHE_POINTER_INFO_T *ptrInfo = &cache->lastPtrEntry[i];

      ptrInfo->cacheEntry = NULL;
      ptrInfo->srcPtr     = NULL;
   }
}

int khrn_cache_init(KHRN_CACHE_T *cache)
{
   cache->tree = NULL;
   cache->data = NULL;

   cache->client_depth = 0;
   cache->server_depth = 0;

   cache->start.prev = NULL;
   cache->start.next = &cache->end;
   cache->end.prev = &cache->start;
   cache->end.next = NULL;

   clear_ptr_info_all(cache);

   return khrn_pointer_map_init(&cache->map, 64);
}


void khrn_cache_term(KHRN_CACHE_T *cache)
{
   khrn_platform_free(cache->tree);
   khrn_platform_free(cache->data);

   khrn_pointer_map_term(&cache->map);

   clear_ptr_info_all(cache);
}

static void send_create(CLIENT_THREAD_STATE_T *thread, int base, bool is_opengles_11)
{
   UNUSED(is_opengles_11);

   glintCacheCreate_impl(base);
}

static void send_delete(CLIENT_THREAD_STATE_T *thread, int base, bool is_opengles_11)
{
   UNUSED(is_opengles_11);

   glintCacheDelete_impl(base);
}

static int send_grow(CLIENT_THREAD_STATE_T *thread, bool is_opengles_11)
{
   UNUSED(is_opengles_11);

   return glintCacheGrow_impl();
}

static void send_data(CLIENT_THREAD_STATE_T *thread, int base, const void *data, int len, bool is_opengles_11)
{
   UNUSED(is_opengles_11);

   /* no need to chop the data into small chunks */
   if (len > 0)
      glintCacheData_impl(base, len, (char *)data);
}

static void clear_ptr_info_entry(KHRN_CACHE_T *cache, CACHE_ENTRY_T *entry)
{
   uint32_t i;

   /* Delete any remembered entries */
   for (i = 0; i <= CACHE_SIG_INDEX; ++i)
   {
      CACHE_POINTER_INFO_T *ptrInfo = &cache->lastPtrEntry[i];

      if (ptrInfo->cacheEntry == entry)
      {
         ptrInfo->cacheEntry = NULL;
         ptrInfo->srcPtr     = NULL;
      }
   }
}

static void discard(CLIENT_THREAD_STATE_T *thread, KHRN_CACHE_T *cache, CACHE_ENTRY_T *entry, bool is_opengles_11)
{
   heap_free(cache, (int)((uint8_t *)entry - cache->data) >> CACHE_LOG2_BLOCK_SIZE);

   khrn_pointer_map_delete(&cache->map, entry->key);

   link_remove(&entry->link);

   send_delete(thread, (int)((uint8_t *)entry - cache->data), is_opengles_11);

   clear_ptr_info_entry(cache, entry);
}

static void *relocate(void *data, void *user)
{
   return (uint8_t *)data - ((uint8_t **)user)[0] + ((uint8_t **)user)[1];
}

static void callback(KHRN_POINTER_MAP_T *map, uint32_t key, void *value, void *user)
{
   CACHE_ENTRY_T *entry = (CACHE_ENTRY_T *)value;

   entry->link.prev = (CACHE_LINK_T *)relocate(entry->link.prev, user);
   entry->link.next = (CACHE_LINK_T *)relocate(entry->link.next, user);

   // Coverity has rightly pointed out that the allocations done in the next codeline can fail
   // verify will only assert the code in debug mode. Use in release mode stays the same as before...
   verify(khrn_pointer_map_insert(map, key, relocate(value, user)));
}

static int grow(CLIENT_THREAD_STATE_T *thread, KHRN_CACHE_T *cache, bool is_opengles_11)
{
   /*
      try to grow the server cache
   */

   uint8_t *tree;
   uint8_t *data;
   int i;

   if (cache->server_depth == cache->client_depth) {
      if (cache->server_depth < CACHE_MAX_DEPTH && send_grow(thread, is_opengles_11))
         cache->server_depth++;
      else
         return 0;
   }

   tree = (uint8_t *)khrn_platform_malloc(1 << (cache->client_depth + 1), "KHRN_CACHE_T.tree");
   data = (uint8_t *)khrn_platform_malloc(1 << (cache->client_depth + CACHE_LOG2_BLOCK_SIZE), "KHRN_CACHE_T.data");

   if (!tree || !data) {
      khrn_platform_free(tree);
      khrn_platform_free(data);
      return 0;
   }

   /* Set up new tree structure
    *
    * The tree is a map of the available memory.  Each entry contains floor of log2 of
    * the available memory below it. So e.g. tree[1] represents the largest available block.
    * tree[2] and tree[3] are the two sub-blocks of tree[1], tree[2] is subdivided as
    * tree[4] and tree[5], tree[3] as tree[6] and tree[7] and so-on recursively.
    */
   tree_init(tree, cache->client_depth + 1);

   /* Patches in the changes from the old tree into the new tree (depth == 0 => no previous tree)
    *
    * Distributes the old tree into the top half of each of the size categories.
    */
   if (cache->client_depth != 0)
   {
      for (i = 1; i < 1 << cache->client_depth; i++)
         tree[i ^ 3 << _msb(i)] = cache->tree[i];

      /* Recalculate total size: if tree[2] == tree[3], then we have double
       * so add one to the log.
       */
      tree[1] = tree[3] + (tree[2] == tree[3]);
   }

   /*
      relocate pointermap and linked list pointers and last seen pointer info
   */
   {
      uint32_t i;
      uint8_t  *user[2];

      user[0] = cache->data;
      user[1] = data;

      khrn_pointer_map_iterate(&cache->map, callback, user);

      cache->start.next->prev = &cache->start;
      if (cache->start.next != &cache->end)
         cache->start.next = (CACHE_LINK_T *)relocate(cache->start.next, user);

      cache->end.prev->next = &cache->end;
      if (cache->end.prev != &cache->start)
         cache->end.prev = (CACHE_LINK_T *)relocate(cache->end.prev, user);

      for (i = 0; i <= CACHE_SIG_INDEX; ++i)
         cache->lastPtrEntry[i].cacheEntry = (CACHE_ENTRY_T *)relocate(cache->lastPtrEntry[i].cacheEntry, user);
   }

   /*
      set up new data block
   */

   if (cache->data)
      platform_memcpy(data, cache->data, 1 << (cache->client_depth + CACHE_LOG2_BLOCK_SIZE - 1));

   /*
      free old blocks, update structure
   */

   khrn_platform_free(cache->tree);
   khrn_platform_free(cache->data);

   cache->tree = tree;
   cache->data = data;

   cache->client_depth++;

   return 1;
}

int khrn_cache_lookup(CLIENT_THREAD_STATE_T *thread, KHRN_CACHE_T *cache, const void *data, int len, int sig, bool is_opengles_11)
{
   CACHE_ENTRY_T *entry = NULL;

   /* First check to see if this pointer is the same as the previously seen one for this sig (attribute type/index)
    * If there is a match and the submission is smaller than a previous one, then we can check that the data matches.
    * Finally if there is a data match, then we can reuse this entry.
    * This catches, in particular, a use-case where a single attribute array is used with a set of index arrays.
    */
   if (data == cache->lastPtrEntry[sig].srcPtr             &&
       len  <= cache->lastPtrEntry[sig].cacheEntry->len    &&
       khrn_memcmp(cache->lastPtrEntry[sig].cacheEntry->data, data, len) == 0)
   {
      entry = cache->lastPtrEntry[sig].cacheEntry;

      /* Refresh the entry so it is at the end of the deletion list */
      link_remove(&cache->lastPtrEntry[sig].cacheEntry->link);
      link_insert(&cache->lastPtrEntry[sig].cacheEntry->link, cache->end.prev, &cache->end);
   }
   else
   {
      int key = hash(data, len, sig);

      entry = (CACHE_ENTRY_T *)khrn_pointer_map_lookup(&cache->map, key);

      if (entry && entry->len >= len && !khrn_memcmp(entry->data, data, len)) {
         /* This is a live entry, so move link to end of discard queue
          */
         link_remove(&entry->link);
         link_insert(&entry->link, cache->end.prev, &cache->end);
      } else {
         int size = _max(_msb(len + sizeof(CACHE_ENTRY_T) - 1) + 2 - CACHE_LOG2_BLOCK_SIZE, 1);
         int block;

         CACHE_LINK_T *link;

         if (entry)
            discard(thread, cache, entry, is_opengles_11);

         /* Grow the cache until there is enough room, or we reach the maximum size */
         while (!heap_avail(cache, size) && grow(thread, cache, is_opengles_11));

         /* If there isn't enough room, discard entries until there is */
         for (link = cache->start.next; link != &cache->end && !heap_avail(cache, size); link = link->next)
            discard(thread, cache, (CACHE_ENTRY_T *)link, is_opengles_11);

         /* If there still isn't enough room, we are lost */
         if (!heap_avail(cache, size))
            return -1;

         block = heap_alloc(cache, size);

         entry = (CACHE_ENTRY_T *)(cache->data + (block << CACHE_LOG2_BLOCK_SIZE));
         entry->len = len;
         entry->key = key;
         platform_memcpy(entry->data, data, len);

         if (!khrn_pointer_map_insert(&cache->map, key, entry)) {
            heap_free(cache, block);
            return -1;
         }

         link_insert(&entry->link, cache->end.prev, &cache->end);

         send_create(thread, (int)((uint8_t *)entry - cache->data), is_opengles_11);
         send_data(thread, (int)(entry->data - cache->data), data, len, is_opengles_11);
      }
   }

   /* If this is a new pointer, then remember it and its corresponding cache entry */
   if (data != cache->lastPtrEntry[sig].srcPtr)
   {
      cache->lastPtrEntry[sig].srcPtr     = data;
      cache->lastPtrEntry[sig].cacheEntry = entry;
   }

   return (int)((uint8_t *)entry - cache->data);
}

int khrn_cache_get_entries(KHRN_CACHE_T *cache)
{
   return cache->map.entries;
}
