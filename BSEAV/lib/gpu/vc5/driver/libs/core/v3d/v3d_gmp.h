/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "v3d_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum v3d_gmp_flags_t
{
   V3D_GMP_NO_ACCESS = 0,
   V3D_GMP_READ_ACCESS = 1 << 0,
   V3D_GMP_WRITE_ACCESS = 1 << 1,
} v3d_gmp_flags_t;

//! Add permissions bits to GMP table.
void v3d_gmp_add_permissions(void* table, v3d_addr_t start, v3d_addr_t end, v3d_gmp_flags_t flags);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

static inline v3d_gmp_flags_t operator|(v3d_gmp_flags_t a, v3d_gmp_flags_t b)
{
   return (v3d_gmp_flags_t)((int)a | (int)b);
}

static inline v3d_gmp_flags_t operator|=(v3d_gmp_flags_t& a, v3d_gmp_flags_t b)
{
   a = (v3d_gmp_flags_t)((int)a | (int)b);
   return a;
}

#endif
