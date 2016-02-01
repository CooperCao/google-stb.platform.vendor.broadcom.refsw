/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG

FILE DESCRIPTION
Extra VG client <-> server interface stuff.
=============================================================================*/

#ifndef VG_INT_H
#define VG_INT_H

/*
   object types as used on the client

   the server has a separate notion of object types, defined in
   middleware/khronos/vg/vg_server.c, which includes partially constructed
   objects ("blueprints") and a distinction between images and child images
*/

typedef enum {
   VG_CLIENT_OBJECT_TYPE_FONT       = 0,
   VG_CLIENT_OBJECT_TYPE_IMAGE      = 1,
   VG_CLIENT_OBJECT_TYPE_MASK_LAYER = 2,
   VG_CLIENT_OBJECT_TYPE_PAINT      = 3,
   VG_CLIENT_OBJECT_TYPE_PATH       = 4
} VG_CLIENT_OBJECT_TYPE_T;

#endif
