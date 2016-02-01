/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef KHRN_FMEM_DEBUG_INFO_H
#define KHRN_FMEM_DEBUG_INFO_H
#ifdef KHRN_GEOMD

#include "helpers/v3d/v3d_addr.h"
#include <stdbool.h>

struct khrn_fmem;

extern void fmem_debug_info_insert(struct khrn_fmem *fmem, v3d_addr_t addr, unsigned draw_id,
                                   unsigned renderstate_id);
extern bool fmem_debug_info_query(v3d_addr_t addr, unsigned *rs_id, unsigned *draw_id);

/* Only for fmem internally */

struct fmem_debug_info
{
   v3d_addr_t  addr;
   unsigned    draw_id;
};

struct fmem_debug_info_vector
{
   struct fmem_debug_info  *v;
   int                     size;          /* How many items are used */
   int                     capacity;      /* What is the current maximum capacity */
   bool                    sorted;        /* Are the entries currently sorted */
   unsigned                renderstate_id;
};

extern bool fmem_debug_info_init_process(void);
extern void fmem_debug_info_term_process(void);
extern void fmem_debug_info_init(struct fmem_debug_info_vector *info);
extern void fmem_debug_info_deinit(struct fmem_debug_info_vector *info);
extern void fmem_debug_info_prepare_for_queries(struct fmem_debug_info_vector *info);

#endif
#endif
