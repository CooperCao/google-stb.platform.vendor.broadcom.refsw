/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
=============================================================================*/

#ifndef PACKET_YV12_H
#define PACKET_YV12_H

#include "memory_nexus_priv.h"
#include <EGL/egl.h>

void memCopy2d_yv12(NXPL_MemoryData *data, BEGL_MemCopy2d *params);

#endif /* PACKET_YV12_H */
