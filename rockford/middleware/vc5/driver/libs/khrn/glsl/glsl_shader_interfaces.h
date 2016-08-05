/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_SHADER_INTERFACE_H
#define GLSL_SHADER_INTERFACE_H

#include "glsl_symbols.h"
#include "glsl_map.h"

typedef struct _ShaderInterfaces {
   SymbolList *uniforms;
   SymbolList *ins;
   SymbolList *outs;
   SymbolList *buffers;
   SymbolList *shared;
} ShaderInterfaces;

ShaderInterfaces *glsl_shader_interfaces_new(void);
void glsl_shader_interfaces_update(ShaderInterfaces *shader_interfaces, Symbol *symbol);

Dataflow **glsl_shader_interface_create_uniform_dataflow(const Symbol *s, int *ids);
Dataflow **glsl_shader_interface_create_buffer_dataflow(const Symbol *s, int *ids);

Map *glsl_shader_interfaces_id_map(const ShaderInterfaces *interfaces);
Map *glsl_shader_interfaces_create_dataflow(const ShaderInterfaces *interfaces,
                                            Map *symbol_ids);

#endif
