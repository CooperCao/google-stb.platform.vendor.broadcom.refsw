/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 2.0 2708 shaders
=============================================================================*/

#ifndef GL20_SHADER_4_H
#define GL20_SHADER_4_H

#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"



typedef struct
{
   GLXX_LINK_RESULT_KEY_T key;
   GLXX_LINK_RESULT_DATA_T data;
   bool used;
   bool failed;
} GL20_LINK_RESULT_CACHE_ENTRY_T;

#define GL20_LINK_RESULT_CACHE_SIZE 8
#define GL20_LINK_RESULT_NODE_COUNT 42

typedef struct
{
   MEM_HANDLE_T mh_blob;
   void *nodes[GL20_LINK_RESULT_NODE_COUNT];
   uint32_t vary_count;

   GL20_LINK_RESULT_CACHE_ENTRY_T cache[GL20_LINK_RESULT_CACHE_SIZE];
   uint32_t cache_used;
   uint32_t cache_next;

   uint32_t vattribs_live;    /* TODO duplicated information - also in GL20_PROGRAM_T */
   uint32_t cattribs_live;
} GL20_LINK_RESULT_T;

extern void gl20_link_result_term(void *v, uint32_t size);
extern bool gl20_link_result_get_shaders(
   GL20_LINK_RESULT_T *link_result,
   GLXX_HW_SHADER_RECORD_T *shader_out,
   MEM_HANDLE_T *cunifmap_out,
   MEM_HANDLE_T *vunifmap_out,
   MEM_HANDLE_T *funifmap_out,
   GLXX_SERVER_STATE_T *state,
   GLXX_ATTRIB_T *attrib,
   uint32_t *mergeable_attribs,
   uint32_t * cattribs_order_out,
   uint32_t * vattribs_order_out,
   bool     * wasInCache);

#endif
