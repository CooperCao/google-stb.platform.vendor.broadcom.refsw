/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#include "v3d_barrier.h"

#include "v3d_scheduler.h"

#define V3D_BARRIER_CLE_READ (\
   V3D_BARRIER_CLE_CL_READ\
 | V3D_BARRIER_CLE_SHADREC_READ\
 | V3D_BARRIER_CLE_DRAWREC_READ\
 | V3D_BARRIER_CLE_PRIMIND_READ)

// Cache heirarchy differs with V3D version.
#define READS_VIA_L2C (\
   (!V3D_VER_AT_LEAST(3,3,0,0) ? V3D_BARRIER_QPU_INSTR_READ : 0)\
 | (!V3D_VER_AT_LEAST(3,3,0,0) ? V3D_BARRIER_QPU_UNIF_READ : 0)\
 | (!V3D_VER_AT_LEAST(3,3,0,0) ? V3D_BARRIER_TMU_CONFIG_READ : 0))

#define READS_VIA_L2T (\
   V3D_BARRIER_TMU_DATA_READ\
 | (V3D_VER_AT_LEAST(3,3,0,0) ? V3D_BARRIER_VCD_READ : 0)\
 | (V3D_VER_AT_LEAST(3,3,0,0) ? V3D_BARRIER_QPU_INSTR_READ : 0)\
 | (V3D_VER_AT_LEAST(3,3,0,0) ? V3D_BARRIER_QPU_UNIF_READ : 0)\
 | (V3D_VER_AT_LEAST(3,3,0,0) ? V3D_BARRIER_TMU_CONFIG_READ : 0)\
 | (V3D_VER_AT_LEAST(3,4,0,6) ? V3D_BARRIER_CLE_READ : 0))

v3d_cache_ops v3d_barrier_cache_cleans(
   v3d_barrier_flags src,
   v3d_barrier_flags dst)
{
   bool has_l3c = v3d_scheduler_get_hub_identity()->has_l3c;
   bool cross_core = v3d_scheduler_get_hub_identity()->num_cores > 1;
   v3d_barrier_flags consumers_outside_l1t = (V3D_BARRIER_MEMORY_RW | V3D_BARRIER_V3D_RW)
                                           & ~(cross_core ? 0 : V3D_BARRIER_TMU_DATA_READ | V3D_BARRIER_TMU_DATA_WRITE);
   v3d_barrier_flags consumers_outside_l2t = (V3D_BARRIER_MEMORY_RW | V3D_BARRIER_V3D_RW)
                                           & ~(cross_core ? 0 : READS_VIA_L2T | V3D_BARRIER_TMU_DATA_WRITE);
   v3d_barrier_flags consumers_outside_l3c = V3D_BARRIER_MEMORY_RW;
   v3d_barrier_flags writers_inside_l1t = V3D_BARRIER_TMU_DATA_WRITE;
   v3d_barrier_flags writers_inside_l2t = V3D_BARRIER_TMU_DATA_WRITE;
   v3d_barrier_flags writers_inside_l3c = V3D_BARRIER_V3D_WRITE;

   v3d_cache_ops clean = V3D_CACHE_OPS_NONE;

   if ((src & writers_inside_l1t) && (dst & consumers_outside_l1t))
   {
     #if V3D_VER_AT_LEAST(3,3,0,0)
      clean |= V3D_CACHE_CLEAN_L1TD;
     #endif
   }

   if ((src & writers_inside_l2t) && (dst & consumers_outside_l2t))
      clean |= V3D_CACHE_CLEAN_L2T;

   if (has_l3c && (src & writers_inside_l3c) && (dst & consumers_outside_l3c))
      clean |= V3D_CACHE_CLEAN_L3C;

   return clean;
}

v3d_cache_ops v3d_barrier_cache_flushes(
   v3d_barrier_flags src,
   v3d_barrier_flags dst)
{
   bool has_l3c = v3d_scheduler_get_hub_identity()->has_l3c;
   bool cross_core = v3d_scheduler_get_hub_identity()->num_cores > 1;

   // Assume byte-write masks are disabled and V3D caches (e.g. L2T/L3C)
   // are allowed to perform read/modify/write on a cache-line.
   bool assume_no_write_masks = true;

   v3d_barrier_flags writers_outside_l3c  = V3D_BARRIER_MEMORY_WRITE;
   v3d_barrier_flags writers_outside_l2c  = (V3D_BARRIER_MEMORY_WRITE | V3D_BARRIER_V3D_WRITE);
   v3d_barrier_flags writers_outside_l2t  = (V3D_BARRIER_MEMORY_WRITE | V3D_BARRIER_V3D_WRITE) & ~(cross_core ? 0 : V3D_BARRIER_TMU_DATA_WRITE);
   v3d_barrier_flags writers_outside_l1td = (V3D_BARRIER_MEMORY_WRITE | V3D_BARRIER_V3D_WRITE) & ~(cross_core ? 0 : V3D_BARRIER_TMU_DATA_WRITE);
   v3d_barrier_flags writers_outside_l1tc = (V3D_BARRIER_MEMORY_WRITE | V3D_BARRIER_V3D_WRITE);
   v3d_barrier_flags writers_outside_sic  = (V3D_BARRIER_MEMORY_WRITE | V3D_BARRIER_V3D_WRITE);
   v3d_barrier_flags writers_outside_suc  = (V3D_BARRIER_MEMORY_WRITE | V3D_BARRIER_V3D_WRITE);
   v3d_barrier_flags writers_outside_vcd  = (V3D_BARRIER_MEMORY_WRITE | V3D_BARRIER_V3D_WRITE);

   v3d_barrier_flags consumers_inside_l3c = V3D_BARRIER_V3D_READ | (assume_no_write_masks ? V3D_BARRIER_V3D_WRITE : 0);
   v3d_barrier_flags consumers_inside_l2c = READS_VIA_L2C;
   v3d_barrier_flags consumers_inside_l2t = READS_VIA_L2T | (assume_no_write_masks ? V3D_BARRIER_TMU_DATA_WRITE : 0);
   v3d_barrier_flags consumers_inside_l1td = V3D_BARRIER_TMU_DATA_READ;
   v3d_barrier_flags consumers_inside_l1tc = V3D_BARRIER_TMU_CONFIG_READ;
   v3d_barrier_flags consumers_inside_sic = V3D_BARRIER_QPU_INSTR_READ;
   v3d_barrier_flags consumers_inside_suc = V3D_BARRIER_QPU_UNIF_READ;
   v3d_barrier_flags consumers_inside_vcd = V3D_BARRIER_VCD_READ;

   v3d_cache_ops flush = V3D_CACHE_OPS_NONE;

   // No GCA (enabled and present) if there is an L3C or for multicore.
   if (has_l3c)
   {
      if ((src & writers_outside_l3c) && (dst & consumers_inside_l3c))
         flush |= V3D_CACHE_FLUSH_L3C;
   }
  #if !V3D_VER_AT_LEAST(4,0,2,0)
   else if (!cross_core)
   {
      // Assume that GCA uses (our) default config.
      v3d_barrier_flags writers_outside_gca =
            V3D_BARRIER_MEMORY_WRITE
          | V3D_BARRIER_TLB_IMAGE_WRITE
          | V3D_BARRIER_TLB_OQ_WRITE
          | V3D_BARRIER_TFU_WRITE;
      v3d_barrier_flags consumers_inside_gca =
            V3D_BARRIER_CLE_READ
      #if !V3D_VER_AT_LEAST(3,3,0,0)
         |  V3D_BARRIER_VCD_READ
      #endif
         |  V3D_BARRIER_PTB_PCF_READ;

      if ((src & writers_outside_gca) && (dst & consumers_inside_gca))
         flush |= V3D_CACHE_CLEAR_GCA;
   }
  #endif

   if ((src & writers_outside_l2c)  && (dst & consumers_inside_l2c))
   {
     #if !V3D_VER_AT_LEAST(3,3,0,0)
      flush |= V3D_CACHE_CLEAR_L2C;
     #endif
   }

   if ((src & writers_outside_l2t)  && (dst & consumers_inside_l2t))  flush |= V3D_CACHE_FLUSH_L2T;
   if ((src & writers_outside_l1td) && (dst & consumers_inside_l1td)) flush |= V3D_CACHE_CLEAR_L1TD;
   if ((src & writers_outside_l1tc) && (dst & consumers_inside_l1tc)) flush |= V3D_CACHE_CLEAR_L1TC;
   if ((src & writers_outside_sic)  && (dst & consumers_inside_sic))  flush |= V3D_CACHE_CLEAR_SIC;
   if ((src & writers_outside_suc)  && (dst & consumers_inside_suc))  flush |= V3D_CACHE_CLEAR_SUC;
   if ((src & writers_outside_vcd)  && (dst & consumers_inside_vcd))  flush |= V3D_CACHE_CLEAR_VCD;
   return flush;
}