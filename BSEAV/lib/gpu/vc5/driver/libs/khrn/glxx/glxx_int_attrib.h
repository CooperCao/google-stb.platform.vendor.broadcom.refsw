/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 and 2.0 client-side attribute information structure declarations.
=============================================================================*/

#ifndef GLXX_INT_ATTRIB_H
#define GLXX_INT_ATTRIB_H

/* GL 1.1 specific For indexing into arrays of handles/pointers */
#define GL11_IX_VERTEX 0
#define GL11_IX_COLOR 1
#define GL11_IX_NORMAL 2
#define GL11_IX_TEXTURE_COORD 3
#define GL11_IX_POINT_SIZE 7
#define GL11_IX_MATRIX_WEIGHT 8
#define GL11_IX_MATRIX_INDEX 9
#define GL11_IX_MAX_ATTRIBS 10

/* Special values passed to glintAttrib etc. instead of indices */
#define GL11_IX_CLIENT_ACTIVE_TEXTURE 0x80000000

#endif
