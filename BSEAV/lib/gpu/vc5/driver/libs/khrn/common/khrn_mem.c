/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_mem.h"
#include "vcos.h"
#include "khrn_int_util.h"

const char *khrn_mem_empty_string;

void *khrn_mem_alloc(size_t size, const char *desc)
{
   khrn_mem_header *header = calloc(size + sizeof(khrn_mem_header), 1);
   if (!header)
      return NULL;

   header->magic = KHRN_MEM_HEADER_MAGIC;
   header->ref_count = 1;
   header->size = size;

   void *result = header + 1;
   assert(header == khrn_mem_get_header(result));
   return result;
}

void khrn_mem_release(const void *p)
{
   if (!p)
      return;

   khrn_mem_header *header = khrn_mem_get_header(p);
   int before_dec = vcos_atomic_dec(&header->ref_count);
   assert(before_dec > 0);

   if (before_dec == 1)
   {
      if (header->term)
         header->term((void *)p);

      free(header);
   }
}

bool khrn_mem_init(void)
{
   khrn_mem_empty_string = khrn_mem_strdup("");
   if (!khrn_mem_empty_string)
      return false;

   return true;
}

void khrn_mem_term(void)
{
   khrn_mem_release(khrn_mem_empty_string);
}

char *khrn_mem_strdup(const char *str)
{
   size_t size = strlen(str) + 1;
   char *dup = khrn_mem_alloc(size, "khrn_mem_strdup");
   if (!dup)
      return NULL;

   memcpy(dup, str, size);
   return dup;
}
