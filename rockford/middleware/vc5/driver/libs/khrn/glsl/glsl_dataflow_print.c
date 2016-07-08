/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_dataflow_visitor.h"
#include "glsl_dataflow_print.h"
#include "glsl_map.h"
#include "glsl_fastmem.h"

#include <stdlib.h>

static const char *df_type_string(DataflowType type) {
   switch(type) {
      case DF_BOOL:       return "bool";
      case DF_INT:        return "int";
      case DF_UINT:       return "uint";
      case DF_FLOAT:      return "float";
      case DF_FSAMPLER:   return "sampler";
      case DF_ISAMPLER:   return "isampler";
      case DF_USAMPLER:   return "usampler";
      case DF_ATOMIC:     return "atomic";
      case DF_FIMAGE:     return "image";
      case DF_IIMAGE:     return "iimage";
      case DF_UIMAGE:     return "uimage";
      case DF_VOID:       return "void";
      default: assert(0); return NULL;
   }
}

void glsl_print_dataflow(FILE *f, Dataflow *dataflow)
{
   const char *name = glsl_dataflow_info_get_name(dataflow->flavour);
   const char *type = df_type_string(dataflow->type);

   fprintf(f, "%%%d = %s %s", dataflow->id, name, type);

   for(int i = 0; i < dataflow->dependencies_count; i++) {
      if (dataflow->d.reloc_deps[i] == -1) {
         fprintf(f, " NULL");
      } else {
         fprintf(f, " %%%d", dataflow->d.reloc_deps[i]);
      }
   }

   switch (dataflow->flavour) {
      case DATAFLOW_CONST:
         switch(dataflow->type) {
            case DF_BOOL:         fprintf(f, " %s", dataflow->u.constant.value ? "true" : "false");     break;
            case DF_INT:          fprintf(f, " %i", (int)dataflow->u.constant.value);                   break;
            case DF_UINT:         fprintf(f, " %u", dataflow->u.constant.value);                        break;
            case DF_FLOAT:        fprintf(f, " %g", float_from_bits(dataflow->u.constant.value));       break;
            default:              break;
         }
         break;

      case DATAFLOW_CONST_SAMPLER:
         fprintf(f, " %d", dataflow->u.const_sampler.location);
         break;

      case DATAFLOW_IN:
      case DATAFLOW_UNIFORM:
      case DATAFLOW_STORAGE_BUFFER:
         fprintf(f, " %d", dataflow->u.linkable_value.row);
         break;

      case DATAFLOW_UNIFORM_BUFFER:
         fprintf(f, " %d", dataflow->u.linkable_value.row);
         break;

      case DATAFLOW_EXTERNAL:
         fprintf(f, " b: %d, o: %d", dataflow->u.external.block, dataflow->u.external.output);
         break;

      default:
         break;
   }
}
