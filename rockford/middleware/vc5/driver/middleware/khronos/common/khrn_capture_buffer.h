/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
APIs for capturing a buffer to file
=============================================================================*/
#ifndef KHRN_CAPTURE_BUFFER
#define KHRN_CAPTURE_BUFFER

VCOS_EXTERN_C_BEGIN

/*
 Captures an image buffer to file.

 @param start_address   V3D address of start of buffer
 @param end_address     V3D address of end of buffer
 @param filename        name of file used to store buffer
 @param desc            attributes of image buffer
 */
extern void khrn_capture_image_buffer(v3d_addr_t begin_addr, v3d_addr_t end_addr,
   const char *filename, const GFX_BUFFER_DESC_T *desc);

/*
 Captures an unformatted buffer to file.

 @param start_address   V3D address of start of buffer
 @param end_address     V3D address of end of buffer
 @param filename        name of file used to store buffer
 */
extern void khrn_capture_unformatted_buffer(v3d_addr_t begin_addr, v3d_addr_t end_addr,
   const char *filename);

void *memcpy_from_v3d(void *dest, unsigned int v3d_addr_src, unsigned int length);

VCOS_EXTERN_C_END

#endif // KHRN_CAPTURE_BUFFER
