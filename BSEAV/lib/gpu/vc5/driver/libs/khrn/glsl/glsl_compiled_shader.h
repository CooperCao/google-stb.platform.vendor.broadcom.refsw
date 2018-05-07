/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_gen.h"

typedef enum InterfaceVarFlags {
   INTERFACE_VAR_NO_FLAGS                 = 0,
#if V3D_VER_AT_LEAST(4,1,34,0)
   INTERFACE_VAR_SAMPLER_DYNAMIC_INDEXING = 1 << 0,
#endif
} InterfaceVarFlags;

typedef struct {
   const Symbol *symbol;
   bool active;

   bool static_use;
   int *ids;
#if V3D_VER_AT_LEAST(4,1,34,0)
   InterfaceVarFlags *flags;   // parallel with ids.
#endif
} InterfaceVar;

typedef struct _ShaderInterface {
   int           n_vars;
   InterfaceVar *var;
} ShaderInterface;

typedef union iface_data {
   struct {
      unsigned vertices;
   } tcs;

   struct {
      v3d_cl_tess_type_t         mode;
      v3d_cl_tess_edge_spacing_t spacing;
      bool                       point_mode;
      bool                       cw;
   } tes;

   struct {
      enum gs_in_type         in;
      v3d_cl_geom_prim_type_t out;
      unsigned                n_invocations;
      unsigned                max_vertices;
   } gs;

   struct {
      bool                   early_tests;
      AdvancedBlendQualifier abq;
   } fs;

   struct {
      unsigned wg_size[3];
      unsigned shared_block_size;
   } cs;
} IFaceData;

struct CompiledShader_s {
   ShaderFlavour flavour;
   int           version;

   char            *str_block;
   SymbolType      *type_block;
   LayoutQualifier *lq_block;
   Symbol          *symbol_block;
   StructMember    *struct_member_block;

   IRShader  ir;
   IFaceData u;

   ShaderInterface uniform;
   ShaderInterface in;
   ShaderInterface out;
   ShaderInterface buffer;
};

InterfaceVar *interface_var_find(ShaderInterface *i, const char *name);
int interface_max_id(const ShaderInterface *i);
