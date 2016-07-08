/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Support functions (mostly relating to shaders) for 2708 GLES1.1
implementation.
=============================================================================*/
#ifndef GL11_SUPPORT_4_H
#define GL11_SUPPORT_4_H

#include "middleware/khronos/glxx/2708/glxx_inner_4.h"

extern bool gl11_hw_get_texunit_properties(GLXX_SERVER_STATE_T *state, GLXX_ATTRIB_T *texture_coord, uint32_t point_sprites);
extern void gl11_hw_setup_attribs_live(GLXX_SERVER_STATE_T *state, uint32_t *cattribs_live, uint32_t *vattribs_live);

#endif
