/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_INTERN_H
#define GLSL_INTERN_H

extern void glsl_init_intern(int size);

extern const char *glsl_intern(const char *s, bool dup);

#endif
