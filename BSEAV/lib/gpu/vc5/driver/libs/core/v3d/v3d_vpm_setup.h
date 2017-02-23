/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_VPM_SETUP_H
#define V3D_VPM_SETUP_H

typedef struct {
   bool horiz, laned;
   uint32_t size, y, x, p;
} V3D_VPM_ADDR_T;

static inline V3D_VPM_ADDR_T v3d_h32(uint32_t y)
{
   V3D_VPM_ADDR_T a = {true, false, 0, y, 0, 0};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_h16l(uint32_t y, uint32_t p)
{
   V3D_VPM_ADDR_T a = {true, true, 1, y, 0, p};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_h16p(uint32_t y, uint32_t p)
{
   V3D_VPM_ADDR_T a = {true, false, 1, y, 0, p};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_h8l(uint32_t y, uint32_t p)
{
   V3D_VPM_ADDR_T a = {true, true, 2, y, 0, p};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_h8p(uint32_t y, uint32_t p)
{
   V3D_VPM_ADDR_T a = {true, false, 2, y, 0, p};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_v32(uint32_t y, uint32_t x)
{
   V3D_VPM_ADDR_T a = {false, false, 0, y, x, 0};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_v16l(uint32_t y, uint32_t x, uint32_t p)
{
   V3D_VPM_ADDR_T a = {false, true, 1, y, x, p};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_v16p(uint32_t y, uint32_t x, uint32_t p)
{
   V3D_VPM_ADDR_T a = {false, false, 1, y, x, p};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_v8l(uint32_t y, uint32_t x, uint32_t p)
{
   V3D_VPM_ADDR_T a = {false, true, 2, y, x, p};
   return a;
}

static inline V3D_VPM_ADDR_T v3d_v8p(uint32_t y, uint32_t x, uint32_t p)
{
   V3D_VPM_ADDR_T a = {false, false, 2, y, x, p};
   return a;
}

static inline uint32_t v3d_vpm_setup_core(
   uint32_t n, int32_t stride, V3D_VPM_ADDR_T addr, bool v2)
{
   uint32_t hls;
   assert(addr.size <= 2);
   assert(addr.horiz ? (addr.x == 0) : !(addr.y & 0xf));
   hls = (gfx_bits(addr.horiz, 1) << 3) | (gfx_bits(addr.laned, 1) << 2) | (2 - addr.size);
   return v2 ?
      ((1 << 29) | (gfx_bitsw(n, 5) << 24) | (gfx_sbitsw(stride, 7) << 16) |
      (hls << 12) | ((gfx_bits(addr.y, 8) | gfx_bits(addr.x, 4)) << addr.size) | gfx_bits(addr.p, addr.size)) :
      ((gfx_bitsw(n, 4) << 20) | (gfx_bitsw(stride, 6) << 12) |
      (hls << 8) | ((gfx_bits(addr.y, 6) | gfx_bits(addr.x, 4)) << addr.size) | gfx_bits(addr.p, addr.size));
}

static inline uint32_t v3d_vpm_setup(
   uint32_t n, int32_t stride, V3D_VPM_ADDR_T addr)
{
   return v3d_vpm_setup_core(n, stride, addr, false);
}

static inline uint32_t v3d_vpm_setup_v2(
   uint32_t n, int32_t stride, V3D_VPM_ADDR_T addr)
{
   return v3d_vpm_setup_core(n, stride, addr, true);
}

#endif
