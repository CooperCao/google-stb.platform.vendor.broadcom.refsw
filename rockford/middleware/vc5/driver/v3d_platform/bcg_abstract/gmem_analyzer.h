/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :
Module   :

FILE DESCRIPTION
Optional analyzer stage for all gmem based allocs and frees.
Captures statistics and outputs on program termination.
=============================================================================*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GMEM_ANALYZER

extern void gmem_analyzer_init(void);
extern void gmem_analyzer_term(void);
extern void gmem_analyzer_alloc(gmem_alloc_item *item);
extern void gmem_analyzer_free(gmem_alloc_item *item);

#else

#define gmem_analyzer_init()
#define gmem_analyzer_term()
#define gmem_analyzer_alloc(i)
#define gmem_analyzer_free(i)

#endif // GMEM_ANALYZER

#ifdef __cplusplus
}
#endif
