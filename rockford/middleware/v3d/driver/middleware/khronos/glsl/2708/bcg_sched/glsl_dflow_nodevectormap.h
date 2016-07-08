/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

Specialize generic map for nodevector pointers
=============================================================================*/

#define khrn_generic_map(X)      NodeVectorMap_##X
#define KHRN_GENERIC_MAP(X)      NodeVectorMap_##X
#define KHRN_GENERIC_MAP_KEY_T   uint32_t
#define KHRN_GENERIC_MAP_VALUE_T NodeVectorPtr

#define KHRN_GENERIC_MAP_VALUE_NONE    NULL
#define KHRN_GENERIC_MAP_VALUE_DELETED ((NodeVector *)(-1))

#define KHRN_GENERIC_MAP_ALLOC         NodeVectorMap_Alloc
#define KHRN_GENERIC_MAP_FREE          NodeVectorMap_Free

typedef NodeVector   *NodeVectorPtr;

extern void *NodeVectorMap_Alloc(uint32_t size, const char *ident);
extern void  NodeVectorMap_Free(void *);

#ifdef KHRN_MAP_C
   #include "interface/khronos/common/khrn_int_generic_map.c"
#else
   #include "interface/khronos/common/khrn_int_generic_map.h"
#endif

#undef KHRN_GENERIC_MAP_VALUE_T
#undef KHRN_GENERIC_MAP_KEY_T
#undef KHRN_GENERIC_MAP
#undef khrn_generic_map
#undef KHRN_GENERIC_MAP_VALUE_NONE
#undef KHRN_GENERIC_MAP_VALUE_DELETED
#undef KHRN_GENERIC_MAP_ALLOC
#undef KHRN_GENERIC_MAP_FREE
