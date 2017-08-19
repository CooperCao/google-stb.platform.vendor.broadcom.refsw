/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_gen.h"

typedef struct {
   const Symbol *symbol;
   bool active;

   bool static_use;
   int *ids;
} InterfaceVar;

typedef struct _ShaderInterface {
   int           n_vars;
   InterfaceVar *var;
} ShaderInterface;

struct CompiledShader_s {
   ShaderFlavour flavour;
   int           version;

   char            *str_block;
   SymbolType      *type_block;
   LayoutQualifier *lq_block;
   Symbol          *symbol_block;
   StructMember    *struct_member_block;

   CFGBlock        *blocks;
   int              num_cfg_blocks;

   IROutput        *outputs;
   int              num_outputs;

   unsigned          tess_vertices;

   v3d_cl_tess_type_t         tess_mode;
   v3d_cl_tess_edge_spacing_t tess_spacing;
   bool                       tess_point_mode;
   bool                       tess_cw;

   enum gs_in_type         gs_in;
   v3d_cl_geom_prim_type_t gs_out;
   unsigned                gs_n_invocations;
   unsigned                gs_max_vertices;
   unsigned                gs_max_known_layers;

   bool                    early_fragment_tests;
   AdvancedBlendQualifier  abq;

   unsigned         cs_wg_size[3];
   unsigned         cs_shared_block_size;

   ShaderInterface uniform;
   ShaderInterface in;
   ShaderInterface out;
   ShaderInterface buffer;
};

InterfaceVar *interface_var_find(ShaderInterface *i, const char *name);
int interface_max_id(const ShaderInterface *i);
