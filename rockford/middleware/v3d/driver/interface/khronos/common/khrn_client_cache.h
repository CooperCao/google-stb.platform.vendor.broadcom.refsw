/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
client side API
=============================================================================*/
#ifndef KHRN_CLIENT_CACHE_H
#define KHRN_CLIENT_CACHE_H

#include "interface/khronos/common/khrn_client_pointermap.h"
#include "interface/khronos/common/khrn_client.h"

typedef struct CACHE_LINK_S {
   struct CACHE_LINK_S *prev;
   struct CACHE_LINK_S *next;
} CACHE_LINK_T;

typedef struct {
   CACHE_LINK_T link;

   int len;
   int key;

   //on the server side
   //we store a KHRN_INTERLOCK_T in the
   //same space as this struct
   uint8_t pad_for_interlock[24];

   uint8_t data[1];
} CACHE_ENTRY_T;

typedef struct {
   const void     *srcPtr;
   CACHE_ENTRY_T  *cacheEntry;
} CACHE_POINTER_INFO_T;

#define CACHE_SIG_ATTRIB_0    0
#define CACHE_SIG_ATTRIB_1    1
#define CACHE_SIG_ATTRIB_2    2
#define CACHE_SIG_ATTRIB_3    3
#define CACHE_SIG_ATTRIB_4    4
#define CACHE_SIG_ATTRIB_5    5
#define CACHE_SIG_ATTRIB_6    6
#define CACHE_SIG_ATTRIB_7    7

#define CACHE_SIG_INDEX       8

typedef struct {
   uint8_t *tree;
   uint8_t *data;

   int client_depth;
   int server_depth;

   CACHE_LINK_T         start;
   CACHE_LINK_T         end;
   CACHE_POINTER_INFO_T lastPtrEntry[CACHE_SIG_INDEX + 1]; /* Maps the last pointer seen to the associated size and hash TODO: make this more generic */

   KHRN_POINTER_MAP_T map;
} KHRN_CACHE_T;

#define CACHE_LOG2_BLOCK_SIZE    6
#define CACHE_MAX_DEPTH          16    /* Max 4Mbytes == 64 * 2^16 */

extern int khrn_cache_init(KHRN_CACHE_T *cache);
extern void khrn_cache_term(KHRN_CACHE_T *cache);

extern int khrn_cache_lookup(CLIENT_THREAD_STATE_T *thread, KHRN_CACHE_T *cache, const void *data, int len, int sig, bool is_opengles_11);
extern int khrn_cache_get_entries(KHRN_CACHE_T *cache);

#endif
