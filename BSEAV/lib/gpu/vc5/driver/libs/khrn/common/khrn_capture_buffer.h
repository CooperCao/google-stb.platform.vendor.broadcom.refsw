/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

EXTERN_C_BEGIN

typedef struct khrn_memaccess khrn_memaccess;

/*
 Captures an image buffer to file.

 @param start_address   V3D address of start of buffer
 @param end_address     V3D address of end of buffer
 @param filename        name of file used to store buffer
 @param desc            attributes of image buffer
 */
void khrn_capture_image_buffer(khrn_memaccess* ma, v3d_addr_t begin_addr,
   v3d_addr_t end_addr, const char *filename, const GFX_BUFFER_DESC_T *desc);

/*
 Captures an unformatted buffer to file.

 @param start_address   V3D address of start of buffer
 @param end_address     V3D address of end of buffer
 @param filename        name of file used to store buffer
 */
void khrn_capture_unformatted_buffer(khrn_memaccess* ma, v3d_addr_t begin_addr,
   v3d_addr_t end_addr, const char *filename);

// Wait for outstanding captures to finish
void khrn_capture_wait(void);

EXTERN_C_END
