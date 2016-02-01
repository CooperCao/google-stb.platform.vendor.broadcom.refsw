/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_CONFIG_H
#define V3D_CONFIG_H

#include "helpers/v3d/v3d_gen.h"

VCOS_EXTERN_C_BEGIN

extern void v3d_check_ident(const V3D_IDENT_T *ident, uint32_t core);
extern void v3d_check_hub_ident(const V3D_HUB_IDENT_T *ident);

VCOS_EXTERN_C_END

#endif
