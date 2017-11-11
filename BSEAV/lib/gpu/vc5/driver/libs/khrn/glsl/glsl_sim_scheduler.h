/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "glsl_backflow.h"

typedef void (*glsl_sim_schedule_node)(void* context, Backflow* node);

typedef struct glsl_sim_schedule_params
{
   glsl_sim_schedule_node schedule_node;
   void* schedule_context;
   Backflow **queue;
   unsigned count;
   RegList* outputs;
   BackflowChain* iodeps;
   Backflow* branch_cond;
} glsl_sim_schedule_params;

void glsl_sim_schedule(glsl_sim_schedule_params* p);
