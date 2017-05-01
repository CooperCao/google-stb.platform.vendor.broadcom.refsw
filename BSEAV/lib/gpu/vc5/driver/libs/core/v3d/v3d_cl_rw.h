/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_CL_RW_H
#define V3D_CL_RW_H

#include "libs/util/gfx_util/gfx_util.h"

static inline void v3d_cl_put_8(uint8_t *cl, uint32_t x)
{
   assert(!(x & ~0xff));
   *cl = (uint8_t)x;
}

static inline void v3d_cl_put_16(uint8_t *cl, uint32_t x)
{
   v3d_cl_put_8(cl, x & 0xff);
   v3d_cl_put_8(cl + 1, x >> 8);
}

static inline void v3d_cl_put_32(uint8_t *cl, uint32_t x)
{
   v3d_cl_put_16(cl, x & 0xffff);
   v3d_cl_put_16(cl + 2, x >> 16);
}

static inline void v3d_cl_put_addr(uint8_t *cl, v3d_addr_t addr)
{
   v3d_cl_put_32(cl, addr);
}

static inline void v3d_cl_put_float(uint8_t *cl, float f)
{
   v3d_cl_put_32(cl, gfx_float_to_bits(f));
}

static inline uint32_t v3d_cl_get_8(const uint8_t *cl)
{
   return *cl;
}

static inline uint32_t v3d_cl_get_16(const uint8_t *cl)
{
   return v3d_cl_get_8(cl) | (v3d_cl_get_8(cl + 1) << 8);
}

static inline uint32_t v3d_cl_get_32(const uint8_t *cl)
{
   return v3d_cl_get_16(cl) | (v3d_cl_get_16(cl + 2) << 16);
}

static inline v3d_addr_t v3d_cl_get_addr(const uint8_t *cl)
{
   return v3d_cl_get_32(cl);
}

static inline float v3d_cl_get_float(const uint8_t *cl)
{
   return gfx_float_from_bits(v3d_cl_get_32(cl));
}

static inline void v3d_cl_plus_32(uint8_t *cl, uint32_t x)
{
   v3d_cl_put_32(cl, v3d_cl_get_32(cl) + x);
}

static inline void v3d_cl_plus_addr(uint8_t *cl, v3d_addr_t addr)
{
   v3d_cl_plus_32(cl, addr);
}

static inline void v3d_cl_add_8(uint8_t **cl, uint32_t x)
{
   v3d_cl_put_8(*cl, x);
   *cl += 1;
}

static inline void v3d_cl_add_16(uint8_t **cl, uint32_t x)
{
   v3d_cl_put_16(*cl, x);
   *cl += 2;
}

static inline void v3d_cl_add_32(uint8_t **cl, uint32_t x)
{
   v3d_cl_put_32(*cl, x);
   *cl += 4;
}

static inline void v3d_cl_add_addr(uint8_t **cl, v3d_addr_t addr)
{
   v3d_cl_add_32(cl, addr);
}

static inline void v3d_cl_add_float(uint8_t **cl, float f)
{
   v3d_cl_add_32(cl, gfx_float_to_bits(f));
}

#endif
