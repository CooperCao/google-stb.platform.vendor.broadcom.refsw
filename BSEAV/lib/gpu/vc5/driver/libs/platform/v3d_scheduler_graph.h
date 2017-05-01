/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/platform/bcm_sched_job.h"

void v3d_sched_graph_init(const char *filename);
void v3d_sched_graph_term();

void v3d_sched_graph_add_node(struct bcm_sched_job *job);

void v3d_sched_graph_add_bin_render_dep(uint64_t bin, uint64_t render);

void v3d_sched_graph_add_fence(
   int fence,
   struct bcm_sched_dependencies *completed_deps,
   struct bcm_sched_dependencies *finalised_deps);
