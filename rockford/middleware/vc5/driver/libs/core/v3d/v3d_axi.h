/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include "vcos_attr.h"

/* See http://confluence.broadcom.com/x/56yuCg */

#if V3D_VER_AT_LEAST(3,4,0,6) // actually CL 460454, but this is closest.

#if V3D_HAS_GMP_L2T_DEBUG

#define V3D_L2T_MASTER_ID_TMU        0
#define V3D_L2T_MASTER_ID_CLE        1
#define V3D_L2T_MASTER_ID_VCD        2
#define V3D_L2T_MASTER_ID_TMU_CONFIG 3
#define V3D_L2T_MASTER_ID_SL0_CACHE  4
#define V3D_L2T_MASTER_ID_SL1_CACHE  5
#define V3D_L2T_MASTER_ID_SL2_CACHE  6

static const char * const v3d_l2t_master_names[7] VCOS_ATTR_POSSIBLY_UNUSED = {
   "L2T(TMU)", "L2T(CLE)", "L2T(VCD)", "L2T(TMU_CONF)", "L2T(SL0)", "L2T(SL1)", "L2T(SL2)"
};

#endif

#define V3D_AXI_ID_L2T 0
#define V3D_AXI_ID_PTB 1
#define V3D_AXI_ID_PSE 2
#define V3D_AXI_ID_TLB 3
#define V3D_AXI_ID_CLE 4   // (Possibly  (see localparam CLE_THROUGH_L2T))
#define V3D_AXI_ID_TFU 5   // In hub.
#define V3D_AXI_ID_MMU 6   // In hub.
#define V3D_AXI_ID_GMP 7   // In hub.

static const char* const v3d_axi_id_names[8] VCOS_ATTR_POSSIBLY_UNUSED = {
   "L2T", "PTB", "PSE", "TLB", "CLE", "TFU", "MMU", "GMP"
   };

#elif V3D_VER_AT_LEAST(3,3,0,0)

#define V3D_AXI_ID_L2T 0
#define V3D_AXI_ID_CLE 1
#define V3D_AXI_ID_PTB 2
#define V3D_AXI_ID_PSE 3
#define V3D_AXI_ID_TLB 4
#define V3D_AXI_ID_GMP 7
#define V3D_AXI_ID_TFU 8

static const char* const v3d_axi_id_names[16] VCOS_ATTR_POSSIBLY_UNUSED = {
   "L2T", "CLE", "PTB", "PSE", "TLB", "-", "-", "GMP",
   "TFU", "-", "-", "-", "-", "-", "-", "-"};

#else

#define V3D_AXI_ID_L2C 0
#define V3D_AXI_ID_CLE 1
#define V3D_AXI_ID_PTB 2
#define V3D_AXI_ID_PSE 3
#define V3D_AXI_ID_VCD 4
#define V3D_AXI_ID_L2T 6
#define V3D_AXI_ID_TLB 7
#define V3D_AXI_ID_TFU 8

static const char* const v3d_axi_id_names[16] VCOS_ATTR_POSSIBLY_UNUSED = {
   "L2C", "CLE", "PTB", "PSE", "VCD", "VDW", "L2T", "TLB",
   "TFU", "-", "-", "-", "-", "-", "-", "-"};

#endif
