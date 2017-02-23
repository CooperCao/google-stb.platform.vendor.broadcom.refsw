/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
This is a default implementation of a Nexus platform layer used by V3D.
This illustrates one way in which the abstract memory interface might be
implemented. You can replace this with your own custom version if preferred.
=============================================================================*/

#ifndef PACKET_RGBA_H
#define PACKET_RGBA_H

#include "memory_nexus_priv.h"
#include <EGL/egl.h>

void memCopy2d_rgba(NXPL_MemoryData *data, BEGL_MemCopy2d *params);

#endif /* PACKET_RGBA_H */
