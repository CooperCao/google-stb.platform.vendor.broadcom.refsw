/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Linux framebuffer platform

FILE DESCRIPTION
This is a linux framebuffer-specific implementation of a Nexus platform layer
used by V3D.
=============================================================================*/

#ifndef __DISPLAY_LINUXFB_H__
#define __DISPLAY_LINUXFB_H__

#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "default_linuxfb.h"

#include <EGL/egl.h>

#define DISPLAY_FLAG_PRINT_FILE_NAME       (1 << 0)
#define DISPLAY_FLAG_PRINT_BUF_ADDR        (1 << 1)
#define DISPLAY_FLAG_PRINT_FRAME_NO        (1 << 2)
#define DISPLAY_FLAG_SAVE_FRAME_PNG        (1 << 3)
#define DISPLAY_FLAG_SAVE_FRAME_PNG_FB     (1 << 4)
#define DISPLAY_FLAG_SAVE_FRAME_RAW        (1 << 5)
#define DISPLAY_FLAG_SAVE_CRC_PNG          (1 << 6)

#define FB_CONVERT_GL_FB (1 << 0)
#define FB_NO_VSYNC_WAIT (1 << 1)
#define FB_NO_DBL_BUF    (1 << 2)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   int                      fbfd;
   int                      cur_page;
   struct fb_var_screeninfo vinfo;
   struct fb_var_screeninfo initial_vinfo;
   struct fb_fix_screeninfo finfo;
   int                      fbmapsize;
   int                      page_size;
   void                     *fbp;
   unsigned int             flags;
} LFPL_FB_Info;

BEGL_MemoryInterface  *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface);
BEGL_HWInterface      *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks);
BEGL_DisplayInterface *LFPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   BEGL_HWInterface     *hwIface,
                                                   BEGL_DisplayCallbacks *displayCallbacks);

void NXPL_DestroyMemInterface(BEGL_MemoryInterface *iface);
void NXPL_DestroyHWInterface(BEGL_HWInterface *iface);
void LFPL_DestroyDisplayInterface(BEGL_DisplayInterface *iface);

extern NEXUS_HEAPHANDLE NXPL_MemHeap(BEGL_MemoryInterface *mem);

typedef struct
{
   BEGL_BufferSettings     settings;
   NEXUS_SURFACEHANDLE     surface;
} LFPL_BufferData;

#ifdef __cplusplus
}
#endif

#endif
