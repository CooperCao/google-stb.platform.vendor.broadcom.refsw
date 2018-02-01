/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "libs/util/common.h"
#include "glsl_dataflow.h"

EXTERN_C_BEGIN

Dataflow *dflib_packHalf2x16(Dataflow *x, Dataflow *y);
Dataflow *dflib_packUnorm2x16(Dataflow *x, Dataflow *y);
Dataflow *dflib_packSnorm2x16(Dataflow *x, Dataflow *y);
Dataflow *dflib_packUnorm4x8(Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w);
Dataflow *dflib_packSnorm4x8(Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w);
Dataflow *dflib_pack_format(FormatQualifier f, Dataflow *v[4]); // returns vec4

void dflib_unpackHalf2x16(Dataflow *r[2], Dataflow *p);
void dflib_unpackUnorm2x16(Dataflow *r[2], Dataflow *p);
void dflib_unpackSnorm2x16(Dataflow *r[2], Dataflow *p);
void dflib_unpackUnorm4x8(Dataflow *r[4], Dataflow *p);
void dflib_unpackSnorm4x8(Dataflow *r[4], Dataflow *p);

EXTERN_C_END
