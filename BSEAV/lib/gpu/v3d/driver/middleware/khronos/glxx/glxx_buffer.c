/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_util.h"

#include "middleware/khronos/glxx/glxx_buffer.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"

#include <string.h>

int glxx_buffer_get_size(GLXX_BUFFER_T *buffer)
{
   return (int)mem_get_size(buffer->pool[buffer->current_item].mh_storage);
}

static bool write_would_block(GLXX_BUFFER_INNER_T *item)
{
   /* Write will block if interlock would block, OR if the current interlock is still in flight in hardware */
   return khrn_interlock_write_would_block(&item->interlock) ||
          item->interlock.pos > khrn_get_last_done_seq();
}

static void glxx_buffer_inner_init(GLXX_BUFFER_INNER_T *item)
{
   assert(item->mh_storage == MEM_HANDLE_INVALID);
   MEM_ASSIGN(item->mh_storage, MEM_ZERO_SIZE_HANDLE);

   khrn_interlock_init(&item->interlock);
}

void glxx_buffer_init(GLXX_BUFFER_T *buffer, uint32_t name)
{
   uint32_t i;
   assert(buffer);

   buffer->name = name;

   buffer->usage = GL_STATIC_DRAW;

   for(i = 0; i< GLXX_BUFFER_POOL_SIZE; i++)
      glxx_buffer_inner_init(&buffer->pool[i]);

   buffer->current_item = 0;
}

static void glxx_buffer_inner_term(GLXX_BUFFER_INNER_T *item)
{
   MEM_ASSIGN(item->mh_storage, MEM_HANDLE_INVALID);

   khrn_interlock_term(&item->interlock);
}

void glxx_buffer_term(void *p)
{
   GLXX_BUFFER_T *buffer = p;
   uint32_t i;
   if(buffer->pool[buffer->current_item].mh_storage!=MEM_ZERO_SIZE_HANDLE)
      mem_unretain(buffer->pool[buffer->current_item].mh_storage);

   for(i = 0; i< GLXX_BUFFER_POOL_SIZE; i++)
      glxx_buffer_inner_term(&buffer->pool[i]);
}

static bool glxx_buffer_inner_data(GLXX_BUFFER_INNER_T *item, int32_t size, const void *data, bool is_new_item, bool transient)
{
   uint32_t current_size;
   assert(size >= 0);

   khrn_interlock_write_immediate(&item->interlock);

   current_size = mem_get_size(item->mh_storage);
   if (current_size != (uint32_t)size) {

      MEM_HANDLE_T handle;
      MEM_FLAG_T flags = MEM_FLAG_DIRECT | MEM_FLAG_DISCARDABLE;
      if(!is_new_item)
      {
         /* unretain existing, retain new */
         if(item->mh_storage!=MEM_ZERO_SIZE_HANDLE)
            mem_unretain(item->mh_storage);

         flags |= MEM_FLAG_RETAINED;
      }
      if (transient)
      {
         flags |= MEM_FLAG_TRANSIENT;
         flags &= ~MEM_FLAG_ZERO;
      }
      /* discardable so can be reclaimed if short of memory and no longer retained */
      if (khrn_workarounds.GFXH776)
      {
         /* indicies can be prefetched up to 256bits after the end.  This causes ARC violations */
         flags |= MEM_FLAG_256BIT_PAD;
      }
      handle = mem_alloc_ex((uint32_t)size, 4, flags,
         "GLXX_BUFFER_INNER_T.storage", MEM_COMPACT_DISCARD);

      if (handle == MEM_HANDLE_INVALID)
      {
         MEM_ASSIGN(item->mh_storage, MEM_ZERO_SIZE_HANDLE);
         return false;
      }

      MEM_ASSIGN(item->mh_storage, handle);
      mem_release(handle);
   }

   /*
      at this point buffer->mh_storage is guaranteed to have size size
   */

   if (data) {
      void *storage = mem_lock(item->mh_storage, NULL);
      khrn_memcpy(storage, data, size);
      khrn_hw_flush_dcache_range(storage, size);
      mem_unlock(item->mh_storage);
   }

   return true;
}

bool glxx_buffer_data(GLXX_BUFFER_T *buffer, int32_t size, const void *data, GLenum usage, bool transient)
{
   assert(size >= 0);

   if(write_would_block(&buffer->pool[buffer->current_item]))
   {
      //pick a non busy entry from the pool;
      uint32_t i;

      for(i = 0; i< GLXX_BUFFER_POOL_SIZE; i++)
      {
         if(i!=(uint32_t)buffer->current_item && !write_would_block(&buffer->pool[i]))
            break;
      }
      if(i<GLXX_BUFFER_POOL_SIZE)
      {  //found one
         //unretain current one, so when fixer also unretains, retain count will fall to 0
         mem_unretain(buffer->pool[buffer->current_item].mh_storage);
         //retain new one
         if(buffer->pool[i].mh_storage!=MEM_ZERO_SIZE_HANDLE)
            mem_retain(buffer->pool[i].mh_storage);
         buffer->current_item = i;

      } //else stick with the existing and wait
   }

   if(!glxx_buffer_inner_data(&buffer->pool[buffer->current_item], size, data, false, transient))
      return false;

   //successfuly allocated memory and copied data
   buffer->usage = usage;

   return true;
}

static void glxx_buffer_inner_subdata(GLXX_BUFFER_INNER_T *item, int32_t offset, int32_t size, const void *data)
{
   assert(offset >= 0 && size >= 0 && (uint32_t)offset + (uint32_t)size <= mem_get_size(item->mh_storage));
   assert(data);

   khrn_interlock_write_immediate(&item->interlock);

   if(size>0) {
      void *storage = (uint8_t *)mem_lock(item->mh_storage, NULL) + offset;
      khrn_memcpy(storage, data, size);
      khrn_hw_flush_dcache_range(storage, size);
      mem_unlock(item->mh_storage);
   }
}

void glxx_buffer_subdata(GLXX_BUFFER_T *buffer, int32_t offset, int32_t size, const void *data)
{
   assert(offset >= 0 && size >= 0);
   assert(data);

   //if write_would_block(buffer->pool[buffer->current_item]
   //we pick one of the other pool items, copy the entirety of the current item
   //into that item, and then the new subdata

   if(write_would_block(&buffer->pool[buffer->current_item]))
   {
      //pick a non busy entry from the pool;
      uint32_t i;

      for(i = 0; i< GLXX_BUFFER_POOL_SIZE; i++)
      {
         if(i!=(uint32_t)buffer->current_item && !write_would_block(&buffer->pool[i]))
               break;
      }
      if(i<GLXX_BUFFER_POOL_SIZE)
      {  //found one
         //copy existing data into this new item assuming alloc succeeds
         uint32_t existing_size = mem_get_size(buffer->pool[buffer->current_item].mh_storage);
         bool ok;
         void * existing_data = mem_lock(buffer->pool[buffer->current_item].mh_storage, NULL);
         ok = glxx_buffer_inner_data(&buffer->pool[i], existing_size, existing_data, true, false);
         mem_unlock(buffer->pool[buffer->current_item].mh_storage);
         if(ok)
         {
            //unretain current one, so when fixer also unretains, retain count will fall to 0
            mem_unretain(buffer->pool[buffer->current_item].mh_storage);
            //retain new one
            if(buffer->pool[i].mh_storage!=MEM_ZERO_SIZE_HANDLE)
               mem_retain(buffer->pool[i].mh_storage);
            buffer->current_item = i;
         }
      } //else stick with the existing and wait

   }

   glxx_buffer_inner_subdata(&buffer->pool[buffer->current_item],offset,size,data);
}

MEM_HANDLE_T glxx_buffer_get_storage_handle(GLXX_BUFFER_T *buffer)
{
   return buffer->pool[buffer->current_item].mh_storage;
}

uint32_t glxx_buffer_get_interlock_offset(GLXX_BUFFER_T *buffer)
{
   return (uint32_t)((char *)&buffer->pool[buffer->current_item].interlock - (char *)buffer);
}
