/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_QUERY_H
#define GLXX_QUERY_H

#include "gl_public_api.h"
#include "glxx_enum_types.h"
#include "../common/khrn_types.h"
#include "../common/khrn_fmem.h"
#include <stdbool.h>


enum glxx_query_target
{
   enumify(GL_NONE),  /* this enum is for queries that didn't have yet a
                         glBeginQuery(target) call */
   enumify(GL_ANY_SAMPLES_PASSED),
   enumify(GL_ANY_SAMPLES_PASSED_CONSERVATIVE),
   enumify(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN),
   enumify(GL_PRIMITIVES_GENERATED),
};

enum glxx_query_type
{
   GLXX_Q_OCCLUSION,
   GLXX_Q_PRIM_WRITTEN,
   GLXX_Q_PRIM_GEN,
   GLXX_Q_COUNT
};

typedef struct glxx_query
{
   uint32_t name;
   enum glxx_query_target target; /* if target = GL_NONE, the query was just
                                     generated and has never been used in a
                                     glBeginQuery */
   enum glxx_query_type type;
   uint64_t wait_point;    /* this is the point on the timeline that we need to
                              wait before the result for this query is
                              available; there is one timeline per query type
                              in the server state */

   uint64_t instance;     /* how many times this query was started (successful
                             glBeginQuery calls); we need to record this so we
                             can deal with the case when the same query gets
                             started many times (glBeginQuery/glEndQuery) and
                             only the last call needs to record the result in
                             the query */

   unsigned prim_drawn_by_us;  /*valid only for GLXX_Q_PRIM_GEN  queries */


    unsigned required_updates; /* number of updates we require on this last
                                  "instance" of this query before we can use
                                  the result. */

   unsigned done_updates; /* number of updates made on the last "instance" of
                             this query */

   unsigned result;

   char *debug_label;     /* For KHR_debug */
} GLXX_QUERY_T;

typedef struct
{
   struct glxx_query* query;
   uint64_t instance;
} glxx_instanced_query_t;

typedef struct glxx_query_block glxx_query_block;

extern bool glxx_queries_updates_lock_init(void);

/* returns false if query has a target and  the passed in target does not match
 * the query's target */
extern bool glxx_query_begin_new_instance(GLXX_QUERY_T *query,
      enum glxx_query_target target);

extern enum glxx_query_type glxx_query_target_to_type(enum glxx_query_target target);

extern GLXX_QUERY_T* glxx_query_create(unsigned name);

/* if query!- NULL, enable ooclusion queries on this rs, otherwise, disable
 * occlusion query */
extern bool glxx_occlusion_query_record(GLXX_HW_RENDER_STATE_T *rs, GLXX_QUERY_T *query);


extern unsigned glxx_query_get_result(GLXX_QUERY_T *query);

void glxx_occlusion_queries_update(glxx_query_block *query_list, bool valid_results);
void glxx_queries_release(glxx_query_block *query_list);

#if V3D_VER_AT_LEAST(4,1,34,0)
extern bool glxx_prim_counts_query_record(GLXX_HW_RENDER_STATE_T *rs,
      GLXX_QUERY_T *query_pg, GLXX_QUERY_T *query_pw);
extern void glxx_prim_drawn_by_us_record(GLXX_HW_RENDER_STATE_T *rs,
      unsigned no_prim);
extern bool glxx_write_prim_counts_feedback(GLXX_HW_RENDER_STATE_T *rs);

void glxx_prim_counts_queries_update(glxx_query_block *query_list, bool valid_results);

#endif

#endif
