/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef V3D_VER_H
#define V3D_VER_H

#ifndef V3D_TECH_VERSION
# error "V3D_TECH_VERSION is undefined"
#endif
#ifndef V3D_REVISION
# error "V3D_REVISION is undefined"
#endif

#define V3D_MAKE_VER(TECH_VERSION, REVISION) (((TECH_VERSION) << 8) | (REVISION))
#define V3D_EXTRACT_REVISION(VER) ((VER) & 0xff)
#define V3D_EXTRACT_TECH_VERSION(VER) ((VER) >> 8)

#define V3D_VER (V3D_MAKE_VER(V3D_TECH_VERSION, V3D_REVISION))
#define V3D_VER_AT_LEAST(TECH_VERSION, REVISION) (V3D_VER >= V3D_MAKE_VER(TECH_VERSION, REVISION))
#define V3D_VER_EQUAL(TECH_VERSION, REVISION) (V3D_VER == V3D_MAKE_VER(TECH_VERSION, REVISION))

/* TFU has YV12 support iff V3D_VER >= V3D_VER_TFU_YV12 */
#define V3D_VER_TFU_YV12 (V3D_MAKE_VER(3, 3))

#endif // V3D_VER_H

#include "v3d_ver_gen.h"
