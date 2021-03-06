/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Server-side RPC

FILE DESCRIPTION
Server-side RPC API.
=============================================================================*/

#ifndef KHRN_DISPATCH_RPC_H
#define KHRN_DISPATCH_RPC_H

#include "interface/khronos/common/khrn_int_util.h"

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/VG/openvg.h"

/******************************************************************************
workspace
******************************************************************************/

extern void *khdispatch_workspace;

extern void khdispatch_check_workspace(uint32_t size);

/******************************************************************************
core internal api
******************************************************************************/

/*
   control channel
*/

extern uint32_t *khdispatch_message;
extern uint32_t khdispatch_position;
extern uint32_t khdispatch_length;

static INLINE uint32_t khdispatch_pad_ctrl(uint32_t len) { return (len + 0x3) & ~0x3; }

static INLINE const void *khdispatch_read_ctrl(uint32_t len) /* advances khdispatch_pad_ctrl(len) bytes */
{
   const void *in = khdispatch_message + khdispatch_position;
   khdispatch_position += khdispatch_pad_ctrl(len) >> 2;
   vcos_assert(khdispatch_position <= khdispatch_length);
   return in;
}

extern void khdispatch_send_ctrl(const void *out, uint32_t len); /* khdispatch_pad_ctrl(len) bytes read/written */
extern void khdispatch_send_ctrl2(const void *out0, uint32_t len0, const void *out1, uint32_t len1); /* khdispatch_pad_ctrl(len0) + khdispatch_pad_ctrl(len1) bytes read/written */

/*
   bulk channel
*/

static INLINE uint32_t khdispatch_pad_bulk(uint32_t len) { return len; }

extern bool khdispatch_recv_bulk(void *in, uint32_t len); /* khdispatch_pad_bulk(len) bytes read/written */
extern void khdispatch_send_bulk(const void *out, uint32_t len); /* khdispatch_pad_bulk(len) bytes read/written */

/*
   async back-channel
*/

extern void khdispatch_send_async(uint32_t command, uint64_t pid, uint32_t sem);

/******************************************************************************
rpc call helpers
******************************************************************************/

/*
   in
*/

static INLINE uint32_t khdispatch_read_uint32(void)
{
   return *(const uint32_t *)khdispatch_read_ctrl(sizeof(uint32_t));
}

static INLINE float khdispatch_read_float(void)         { return float_from_bits(khdispatch_read_uint32()); }
static INLINE GLenum khdispatch_read_enum(void)         { return (GLenum)khdispatch_read_uint32(); }
static INLINE GLint khdispatch_read_int(void)           { return (GLint)khdispatch_read_uint32(); }
static INLINE GLintptr khdispatch_read_intptr(void)     { return (GLintptr)khdispatch_read_uint32(); }
static INLINE GLuint khdispatch_read_uint(void)         { return (GLuint)khdispatch_read_uint32(); }
static INLINE GLsizei khdispatch_read_sizei(void)       { return (GLsizei)khdispatch_read_uint32(); }
static INLINE GLsizeiptr khdispatch_read_sizeiptr(void) { return (GLsizeiptr)khdispatch_read_uint32(); }
static INLINE bool khdispatch_read_boolean(void)        { return !!khdispatch_read_uint32(); }
static INLINE GLbitfield khdispatch_read_bitfield(void) { return (GLbitfield)khdispatch_read_uint32(); }
static INLINE GLfixed khdispatch_read_fixed(void)       { return (GLfixed)khdispatch_read_uint32(); }
static INLINE VGHandle khdispatch_read_handle(void)     { return (VGHandle)khdispatch_read_uint32(); }
static INLINE void *khdispatch_read_eglhandle(void)     { return (void*)(uintptr_t)khdispatch_read_uint32(); }

extern bool khdispatch_read_in_bulk(void *in, uint32_t len, void **outdata); /* khdispatch_pad_bulk(len) bytes written */

/*
   out
*/

static INLINE void khdispatch_write_uint32(uint32_t u)
{
   khdispatch_send_ctrl(&u, sizeof(u));
}

static INLINE void khdispatch_write_float(float f)           { khdispatch_write_uint32(float_to_bits(f)); }
static INLINE void khdispatch_write_enum(GLenum e)           { khdispatch_write_uint32((uint32_t)e); }
static INLINE void khdispatch_write_int(GLint i)             { khdispatch_write_uint32((uint32_t)i); }
static INLINE void khdispatch_write_uint(GLuint u)           { khdispatch_write_uint32((uint32_t)u); }
static INLINE void khdispatch_write_boolean(bool b)          { khdispatch_write_uint32((uint32_t)b); }
static INLINE void khdispatch_write_bitfield(GLbitfield b)   { khdispatch_write_uint32((uint32_t)b); }
static INLINE void khdispatch_write_handle(VGHandle h)       { khdispatch_write_uint32((uint32_t)h); }
static INLINE void khdispatch_write_surfaceid(uint32_t id)   { khdispatch_write_uint32(id); }
static INLINE void khdispatch_write_glcontextid(uint32_t id) { khdispatch_write_uint32(id); }
static INLINE void khdispatch_write_vgcontextid(uint32_t id) { khdispatch_write_uint32(id); }
static INLINE void khdispatch_write_syncid(uint32_t id)      { khdispatch_write_uint32(id); }

extern void khdispatch_write_out_ctrl(const void *out, uint32_t len); /* khdispatch_pad_ctrl(len) bytes read */
extern void khdispatch_write_out_ctrl_res(const void *out, uint32_t len, uint32_t res); /* khdispatch_pad_ctrl(len) bytes read */
extern void khdispatch_write_out_bulk(const void *out, uint32_t len); /* khdispatch_pad_bulk(len) bytes read */
extern void khdispatch_write_out_bulk_res(const void *out, uint32_t len, uint32_t res); /* khdispatch_pad_bulk(len) bytes read */

#endif /* SERVER_RPC_H */
