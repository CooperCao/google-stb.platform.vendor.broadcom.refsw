/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
QPU disassembler.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void glsl_qdisasm_instruction(char *buffer, uint32_t buffer_length, uint32_t word0, uint32_t word1);
extern void glsl_formatplain_instruction(char *buffer, uint32_t buffer_length, uint32_t word0, uint32_t word1);
extern uint32_t *glsl_qdisasm_with_uniform(uint32_t *state, char *buffer, uint32_t buffer_length, uint32_t word0, uint32_t word1, uint32_t *uniform, bool is_uniform_map);
extern void glsl_qdisasm_dump(const void *shader, const void *uniforms, uint32_t count, bool is_uniform_map);
extern void glsl_qdisasm_py_dump(uint32_t count,uint32_t * words);


#ifdef __cplusplus
}
#endif
