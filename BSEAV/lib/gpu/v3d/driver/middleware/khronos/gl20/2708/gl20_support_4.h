/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Support functions (mostly relating to shaders) for 2708 GLES2.0
implementation.
=============================================================================*/
#ifndef GL20_SUPPORT_4_H
#define GL20_SUPPORT_4_H

#include "middleware/khronos/glxx/2708/glxx_inner_4.h"

typedef struct
{
#define IU_MAX 3
   uint32_t index[IU_MAX];
   uint32_t size[IU_MAX];
   uint32_t *addr[IU_MAX];
   MEM_LOCK_T lbh[IU_MAX];
   uint32_t count;
} GL20_HW_INDEXED_UNIFORM_T;

//extern uint32_t gl20_install_tex_param(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program, uint32_t *data, GLXX_FIXABLE_BUF_T *buf, uint32_t u0, uint32_t u1);

extern void gl20_hw_iu_init(GL20_HW_INDEXED_UNIFORM_T *iu);
extern void gl20_hw_iu_close(GL20_HW_INDEXED_UNIFORM_T *iu);

#endif