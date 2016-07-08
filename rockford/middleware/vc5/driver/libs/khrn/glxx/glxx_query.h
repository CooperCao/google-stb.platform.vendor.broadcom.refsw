/*=============================================================================
Broadcom Proprietary and Confidential. (c)20014 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
asynchronous query structure declaration + API.
=============================================================================*/
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
   enumify(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)
};

enum glxx_query_type
{
   GLXX_Q_OCCLUSION,
   GLXX_Q_TRANSF_FEEDBACK,
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


    unsigned required_updates; /* number of updates we require on this last
                                  "instance" of this query before we can use
                                  the result. */

   unsigned done_updates; /* number of updates made on the last "instance" of
                             this query */

   unsigned result;

   char *debug_label;     /* For KHR_debug */
} GLXX_QUERY_T;

extern bool glxx_queries_updates_lock_init(void);

/* returns false if query has a target and  the passed in target does not match
 * the query's target */
extern bool glxx_query_begin_new_instance(GLXX_QUERY_T *query,
      enum glxx_query_target target);

extern enum glxx_query_type glxx_query_target_to_type(enum glxx_query_target target);

extern GLXX_QUERY_T* glxx_query_create(unsigned name);
extern bool glxx_query_enable(GLXX_HW_RENDER_STATE_T *rs,
      GLXX_QUERY_T *query);
extern bool glxx_query_disable(GLXX_HW_RENDER_STATE_T *rs,
      enum glxx_query_type type);
unsigned glxx_query_get_result(GLXX_QUERY_T *query);

extern void glxx_queries_update(KHRN_QUERY_BLOCK_T *query_list,
      bool valid_results);

#endif
