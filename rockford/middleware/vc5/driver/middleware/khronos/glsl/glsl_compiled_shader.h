/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_COMPILED_SHADER_H_INCLUDED
#define GLSL_COMPILED_SHADER_H_INCLUDED

typedef struct {
   const Symbol *symbol;
   bool active;

   bool static_use;
   int *ids;
} InterfaceVar;

typedef struct {
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

   ShaderInterface uniform;
   ShaderInterface in;
   ShaderInterface out;
   ShaderInterface buffer;
};

InterfaceVar *interface_var_find(ShaderInterface *i, const char *name);
int interface_max_id(const ShaderInterface *i);

#endif
