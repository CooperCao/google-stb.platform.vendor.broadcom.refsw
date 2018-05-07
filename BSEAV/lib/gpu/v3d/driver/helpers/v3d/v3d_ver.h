/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if EMBEDDED_SETTOP_BOX

/* This will define the start and end address defines for each register block */
#include "bchp_common.h"

/* Determine the core revision */
#include "bchp_v3d_ctl.h"

#if BCHP_V3D_CTL_IDENT0_IDENT0_DEFAULT==0x03443356
#define V3D_TECH_VERSION 2
#else
#error "VC4 not present!"
#endif

#if (BCHP_V3D_CTL_IDENT1_IDENT1_DEFAULT & 0xF)==1
#define V3D_REVISION 5
#elif (BCHP_V3D_CTL_IDENT1_IDENT1_DEFAULT & 0xF)==2
#define V3D_REVISION 6
#else
#error "UNKNOWN V3D_REVISION!"
#endif

/* Non STB builds require the version to be set on the build command */
#ifndef V3D_TECH_VERSION
#define V3D_TECH_VERSION 2
#endif

#ifndef V3D_REVISION
#error "V3D_REVISION must be defined!"
#endif

#endif

#ifndef V3D_SUB_REV
#define V3D_SUB_REV 0
#endif

#ifndef V3D_COMPAT_REV
#define V3D_COMPAT_REV 0
#endif

#define V3D_MAKE_VER(TECH_VERSION, REVISION, SUB_REV, COMPAT_REV) \
   (((TECH_VERSION) << 24) | ((REVISION) << 16) | ((SUB_REV) << 8) | (COMPAT_REV))
#define V3D_EXTRACT_TECH_VERSION(VER) ((VER) >> 24)
#define V3D_EXTRACT_REVISION(VER) (((VER) >> 16) & 0xff)
#define V3D_EXTRACT_SUB_REV(VER) (((VER) >> 8) & 0xff)
#define V3D_EXTRACT_COMPAT_REV(VER) ((VER) & 0xff)

#define V3D_VER (V3D_MAKE_VER(V3D_TECH_VERSION, V3D_REVISION, V3D_SUB_REV, V3D_COMPAT_REV))
#define V3D_VER_AT_LEAST(TECH_VERSION, REVISION, SUB_REV, COMPAT_REV) \
   (V3D_VER >= V3D_MAKE_VER(TECH_VERSION, REVISION, SUB_REV, COMPAT_REV))
