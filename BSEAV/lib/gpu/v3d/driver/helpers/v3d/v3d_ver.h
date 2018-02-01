/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

/*
   1.0: 2707
   2.0: 2708 a0
   2.1: bcg a1, hera
   2.2: 2708 b0
   2.3: 2708 c0/1
   2.4: bcg b0, rhea/athena, samoa
   2.5: bcg b1
   2.6: capri
*/

/* if V3D_TECH_VERSION/V3D_REVISION aren't defined, try to figure them out from
 * other defines... */
#ifndef V3D_TECH_VERSION
   #define V3D_TECH_VERSION 2
#endif
#ifndef V3D_REVISION
   #ifdef __BCM2708A0__
      #ifdef HERA
         /* bcg a1, hera */
         #define V3D_REVISION 1
      #else
         /* 2708 a0 */
         #define V3D_REVISION 0
      #endif
   #elif defined(__BCM2708B0__)
      /* 2708 b0 */
      #define V3D_REVISION 2
   #elif defined(__BCM2708C0__) || defined(__BCM2708C1__)
      /* 2708 c0/1 */
      #define V3D_REVISION 3
   #else
      /* capri */
      #define V3D_REVISION 6
   #endif
#endif

#ifndef V3D_SUB_REV
#define V3D_SUB_REV 0
#endif

#ifndef V3D_HIDDEN_REV
#define V3D_HIDDEN_REV 0
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
