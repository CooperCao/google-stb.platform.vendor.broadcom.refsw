/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_MEM_UTILS_H_INCLUDED
#define GLSL_MEM_UTILS_H_INCLUDED

#define MEMORY_MAX_ALIGNMENT 8

#define aligned_sizeof(s) (aligned_size(sizeof(s)))
#define aligned_size(s)   (((s) + (MEMORY_MAX_ALIGNMENT-1)) & ~(MEMORY_MAX_ALIGNMENT-1))

#endif
