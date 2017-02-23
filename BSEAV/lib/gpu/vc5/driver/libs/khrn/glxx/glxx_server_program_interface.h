
/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION

=============================================================================*/

#ifndef GLXX_SERVER_PROGRAM_INTERFACE_H
#define GLXX_SERVER_PROGRAM_INTERFACE_H

#include "glxx_server_state.h"

GLuint glxx_get_program_resource_index(GLXX_SERVER_STATE_T *state, GLuint p, GLenum programInterface, const GLchar *name);
GLint  glxx_get_program_resource_location(GLXX_SERVER_STATE_T *state, GLuint p, GLenum programInterface, const GLchar *name);
void   glxx_get_program_resource_name(GLXX_SERVER_STATE_T *state, GLuint p, GLenum programInterface, GLuint index,
                                      GLsizei bufSize, GLsizei *length, GLchar *name);
bool   glxx_get_program_resourceiv(GLXX_SERVER_STATE_T *state, GLuint p, GLenum programInterface, GLuint index, GLsizei propCount,
                                   const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);

#endif /* __GLXX_SERVER_PROGRAM_INTERFACE_H__ */
