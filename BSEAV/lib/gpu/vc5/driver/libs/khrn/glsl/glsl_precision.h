/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_compiler.h"
#include "glsl_errors.h"

#ifndef NELEMS
#define NELEMS(a)    (sizeof(a)/sizeof(a[0]))
#endif

/** Forward declaration of Precision Table type **/
typedef struct _PrecisionTable PrecisionTable;

/** Add a new table on top of the given table **/
extern PrecisionTable *glsl_prec_add_table(PrecisionTable *);

/** Remove the given table and return the underlaying table **/
extern PrecisionTable *glsl_prec_delete_table(PrecisionTable *);

/** Set table defaults based on shader flavour and language version **/
extern void glsl_prec_set_defaults(PrecisionTable *, ShaderFlavour);

/** Modify an entry in a precision table **/
extern void glsl_prec_modify_prec(PrecisionTable *, const SymbolType *, PrecisionQualifier);

/** Query a precision table for a given type **/
extern PrecisionQualifier glsl_prec_get_prec(const PrecisionTable *, const SymbolType *);
