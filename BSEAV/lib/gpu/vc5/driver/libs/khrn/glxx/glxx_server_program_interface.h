/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_SERVER_PROGRAM_INTERFACE_H
#define GLXX_SERVER_PROGRAM_INTERFACE_H

#include "glxx_server_state.h"

unsigned glxx_get_program_resource_index   (GLXX_SERVER_STATE_T *state, unsigned p, GLenum interface, const char *name);
int      glxx_get_program_resource_location(GLXX_SERVER_STATE_T *state, unsigned p, GLenum interface, const char *name);
void     glxx_get_program_resource_name    (GLXX_SERVER_STATE_T *state, unsigned p, GLenum interface, unsigned index,
                                            GLsizei buf_size, GLsizei *length, char *name);
bool     glxx_get_program_resourceiv(GLXX_SERVER_STATE_T *state, unsigned p, GLenum interface, unsigned index, GLsizei propCount,
                                     const GLenum *props, GLsizei bufSize, GLsizei *length, int *params);

int      glxx_get_max_name_length(const GLSL_PROGRAM_T *p, GLenum interface);

#endif /* __GLXX_SERVER_PROGRAM_INTERFACE_H__ */
