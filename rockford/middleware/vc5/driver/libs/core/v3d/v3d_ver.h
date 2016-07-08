/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include <assert.h>

/* STB builds may not have the version set on the build command (e.g. refsw builds).
 * They will pick up the core version from the RDB includes, as directed
 * by the BCHP setting from the plat scripts.
 *
 * If the versions are set on the build line, we will check that they match the RDB.
 */
#if EMBEDDED_SETTOP_BOX

/* This will define the start and end address defines for each register block */
#include "bchp_common.h"

/* Determine the core revision */
#ifdef BCHP_V3D_CTL_0_REG_START
#include "bchp_v3d_ctl_0.h"
#define RDB_TECH_VERSION   BCHP_V3D_CTL_0_IDENT0_TVER_DEFAULT
#define RDB_REVISION       BCHP_V3D_CTL_0_IDENT1_REV_DEFAULT
#define RDB_SUB_REV        BCHP_V3D_CTL_0_IDENT3_IPREV_DEFAULT
#define RDB_HIDDEN_REV     0
#else
#include "bchp_v3d_ctl.h"
#define RDB_TECH_VERSION   BCHP_V3D_CTL_IDENT0_TVER_DEFAULT
#define RDB_REVISION       BCHP_V3D_CTL_IDENT1_REV_DEFAULT
#define RDB_SUB_REV        BCHP_V3D_CTL_IDENT3_IPREV_DEFAULT
#define RDB_HIDDEN_REV     0
#endif

#ifdef V3D_TECH_VERSION
static_assert(V3D_TECH_VERSION == RDB_TECH_VERSION, "Tech version mismatch with RDB");
#else
#define V3D_TECH_VERSION RDB_TECH_VERSION
#endif

#ifdef V3D_REVISION
static_assert(V3D_REVISION == RDB_REVISION, "Revision mismatch with RDB");
#else
#define V3D_REVISION RDB_REVISION
#endif

#ifdef V3D_SUB_REV
static_assert(V3D_SUB_REV == RDB_SUB_REV, "Subrev mismatch with RDB");
#else
#define V3D_SUB_REV RDB_SUB_REV
#endif

#ifdef V3D_HIDDEN_REV
static_assert(V3D_HIDDEN_REV == RDB_HIDDEN_REV, "Hidden rev mismatch with RDB");
#else
#define V3D_HIDDEN_REV RDB_HIDDEN_REV
#endif

#else

/* Non STB builds require the version to be set on the build command */
#ifndef V3D_TECH_VERSION
# error "V3D_TECH_VERSION is undefined"
#endif
#ifndef V3D_REVISION
# error "V3D_REVISION is undefined"
#endif
#ifndef V3D_SUB_REV
# error "V3D_SUB_REV is undefined"
#endif
#ifndef V3D_HIDDEN_REV
# error "V3D_HIDDEN_REV is undefined"
#endif

#endif


#define V3D_MAKE_VER(TECH_VERSION, REVISION, SUB_REV, HIDDEN_REV) \
   (((TECH_VERSION) << 24) | ((REVISION) << 16) | ((SUB_REV) << 8) | (HIDDEN_REV))
#define V3D_EXTRACT_TECH_VERSION(VER) ((VER) >> 24)
#define V3D_EXTRACT_REVISION(VER) (((VER) >> 16) & 0xff)
#define V3D_EXTRACT_SUB_REV(VER) (((VER) >> 8) & 0xff)
#define V3D_EXTRACT_HIDDEN_REV(VER) ((VER) & 0xff)

#define V3D_VER (V3D_MAKE_VER(V3D_TECH_VERSION, V3D_REVISION, V3D_SUB_REV, V3D_HIDDEN_REV))
#define V3D_VER_AT_LEAST(TECH_VERSION, REVISION, SUB_REV, HIDDEN_REV) \
   (V3D_VER >= V3D_MAKE_VER(TECH_VERSION, REVISION, SUB_REV, HIDDEN_REV))

#include "v3d_ver_gen.h"
