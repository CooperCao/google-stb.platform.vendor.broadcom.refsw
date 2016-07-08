/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION

=============================================================================*/

#ifndef GLXX_SERVER_PIPELINE_H
#define GLXX_SERVER_PIPELINE_H

#include "glxx_server_state.h"
#include "glxx_server.h"

#include "../gl20/gl20_program.h"

typedef enum
{
   // Slots 0-4 are for graphics stages
   GRAPHICS_STAGE_VERTEX,
#if GLXX_HAS_TESSELLATION
   GRAPHICS_STAGE_TESS_CONTROL,
   GRAPHICS_STAGE_TESS_EVALUATION,
#endif
#if GLXX_HAS_TNG
   GRAPHICS_STAGE_GEOMETRY,
#endif
   GRAPHICS_STAGE_FRAGMENT,
   GRAPHICS_STAGE_COUNT,

   // Last slot is for the compute stage
   COMPUTE_STAGE_COMPUTE   = GRAPHICS_STAGE_COUNT,
   COMPUTE_STAGE_COUNT     = 1,

   STAGE_COUNT             = GRAPHICS_STAGE_COUNT + COMPUTE_STAGE_COUNT
} STAGE_T;

// In future we may need more state per stage, so keep this struct for now
typedef struct PIPELINE_STAGE_T_
{
   GL20_PROGRAM_T *program;
} PIPELINE_STAGE;

typedef struct GLXX_PIPELINE_T_
{
   int32_t                 name;
   GLboolean               initialised;
   unsigned                active_program;
   GLboolean               validation_status;

   PIPELINE_STAGE          stage[STAGE_COUNT];

   char                   *info_log;
   char                   *debug_label;

   // Holds the binary and other information for the pipeline "program"
   GL20_PROGRAM_COMMON_T   common;
   bool                    common_is_compute;
} GLXX_PIPELINE_T;

GLuint           glxx_pipeline_get_binding(const GLXX_SERVER_STATE_T *state);
GLXX_PIPELINE_T *glxx_pipeline_get(const GLXX_SERVER_STATE_T *state, GLuint pipeline);
GLuint           glxx_pipeline_get_program_name(const GLXX_SERVER_STATE_T *state, STAGE_T stage);
bool             glxx_pipeline_validate(const GLXX_PIPELINE_T *pipeline);
bool             glxx_pipeline_create_graphics_common(GLXX_PIPELINE_T *pipeline);
bool             glxx_pipeline_create_compute_common(GLXX_PIPELINE_T *pipeline);
GL20_PROGRAM_T  *glxx_pipeline_get_active_program(const GLXX_SERVER_STATE_T *state);

#endif /* __GLXX_SERVER_PIPELINE_H__ */
