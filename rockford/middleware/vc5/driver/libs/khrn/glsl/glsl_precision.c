/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  glsl shader compiler
Module   :  Header file

FILE DESCRIPTION
Precision tracking and checking
=============================================================================*/

#include "glsl_common.h"
#include "glsl_globals.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_precision.h"

struct _PrecisionTable
{
   struct _PrecisionTable *parent;

   PrecisionQualifier prec[PRIMITIVE_TYPES_COUNT];
};

static void tbl_set_floats( PrecisionTable *tbl, PrecisionQualifier prec )
{
   tbl->prec[PRIM_FLOAT]  =
   tbl->prec[PRIM_VEC2]   =
   tbl->prec[PRIM_VEC3]   =
   tbl->prec[PRIM_VEC4]   =
   tbl->prec[PRIM_MAT2]   =
   tbl->prec[PRIM_MAT3]   =
   tbl->prec[PRIM_MAT4]   =
   tbl->prec[PRIM_MAT2X3] =
   tbl->prec[PRIM_MAT2X4] =
   tbl->prec[PRIM_MAT3X2] =
   tbl->prec[PRIM_MAT3X4] =
   tbl->prec[PRIM_MAT4X2] =
   tbl->prec[PRIM_MAT4X3] = prec;
}

static void tbl_set_ints( PrecisionTable *tbl, PrecisionQualifier prec )
{
   tbl->prec[PRIM_INT]   =
   tbl->prec[PRIM_IVEC2] =
   tbl->prec[PRIM_IVEC3] =
   tbl->prec[PRIM_IVEC4] =
   tbl->prec[PRIM_UINT]  =
   tbl->prec[PRIM_UVEC2] =
   tbl->prec[PRIM_UVEC3] =
   tbl->prec[PRIM_UVEC4] = prec;
}

/****************************************************************************
   Public Functions.  Defined in the corresponding header file.
 ****************************************************************************/

/****************************************************************************/
/**
   Create a new precision table.  If parent then copy precisions into this one.

   @param   parent     Pointer to parent table, if any.

   @return  Pointer to new table.
**/
PrecisionTable *glsl_prec_add_table( PrecisionTable *parent )
{
   PrecisionTable *p = malloc_fast( sizeof( *p ) );

   p->parent = parent;

   /* setup the table */
   if ( parent )
   {
      /* Copy parent precisions into this table */
      for (unsigned int i = 0; i < NELEMS( p->prec ); i++)
         p->prec[i] = parent->prec[i];
   }
   else
   {
      /* Set all precisions to unknown */
      for (unsigned int i = 0; i < NELEMS( p->prec ); i++)
         p->prec[i] = PREC_UNKNOWN;
   }

   return p;
}

/****************************************************************************/
/**
   Delete a precision table.

   @param    current     Pointer to current table.

   @return   Pointer to parent table.
**/
PrecisionTable *glsl_prec_delete_table( PrecisionTable *current )
{
   /* Because of compile error semantics we don't free. TODO: Use safemem */
   return current->parent;
}

/****************************************************************************/
/**
   Set default precisions in the given table, based on version and flavour.

   @param   tbl         Pointer to precision table
   @param   version     Shader version
   @param   flavour     Shader flavour (vertex, fragment, etc)
**/
void glsl_prec_set_defaults( PrecisionTable *tbl, int version, ShaderFlavour flavour )
{
   /* Set all precisions to unknown */
   for (unsigned int i = 0; i < NELEMS( tbl->prec ); i++ )
      tbl->prec[i] = PREC_UNKNOWN;

   if ( flavour == SHADER_FRAGMENT) {
      tbl_set_ints(   tbl, PREC_MEDIUMP );
   } else {
      tbl_set_floats( tbl, PREC_HIGHP );
      tbl_set_ints(   tbl, PREC_HIGHP );
   }

   tbl->prec[PRIM_SAMPLER2D]   = PREC_LOWP;
   tbl->prec[PRIM_SAMPLERCUBE] = PREC_LOWP;

   tbl->prec[PRIM_ATOMIC_UINT] = PREC_HIGHP;
}

/****************************************************************************/
/**
   Modify entry in given precision table.

   @param   tbl         Pointer to precision table
   @param   type        The type to modify
   @param   newprec     The new precision
**/
void glsl_prec_modify_prec( PrecisionTable *tbl, const SymbolType *type, PrecisionQualifier newprec )
{
   if ( type->flavour != SYMBOL_PRIMITIVE_TYPE )
      glsl_compile_error( ERROR_SEMANTIC, 28, g_LineNumber, NULL );

   /* Update all similar types (matrices, vectors) */
   if (type->u.primitive_type.index == PRIM_INT)
      tbl_set_ints( tbl, newprec );
   else if (type->u.primitive_type.index == PRIM_FLOAT)
      tbl_set_floats( tbl, newprec );
   else if (glsl_prim_is_prim_sampler_type(type) || glsl_prim_is_prim_image_type(type))
   {
      tbl->prec[type->u.primitive_type.index] = newprec;
   } else if (glsl_prim_is_prim_atomic_type(type)) {
      if (newprec != PREC_HIGHP)
         glsl_compile_error(ERROR_SEMANTIC, 28, g_LineNumber, "precision for atomic type must be highp");
   } else
      glsl_compile_error( ERROR_SEMANTIC, 28, g_LineNumber, "%s", type->name );
}

/****************************************************************************/
/**
   Query an entry in given precision table.

	For non-primitive types we just return NONE, to say that the given type
   has no precision.
	For primitive types simply return what is in the given precision table.

   @param   tbl         Pointer to precision table
   @param   type        The type to enquire about

   @retval  The precision of the queried type.
**/
PrecisionQualifier glsl_prec_get_prec( const PrecisionTable *tbl, const SymbolType *type )
{
   if ( type->flavour != SYMBOL_PRIMITIVE_TYPE )
      return PREC_NONE;

   return tbl->prec[type->u.primitive_type.index];
}
