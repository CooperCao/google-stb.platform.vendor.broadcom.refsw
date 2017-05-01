/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
