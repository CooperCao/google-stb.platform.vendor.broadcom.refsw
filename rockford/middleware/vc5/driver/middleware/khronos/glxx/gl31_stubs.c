/*=============================================================================
 * Copyright (c) 2010 Broadcom Europe Limited.
 * All rights reserved.
 *
 * Project  :  khronos
 * Module   :  Header file
 *
 * FILE DESCRIPTION
 * Stub functions for ES3.1
 * =============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "helpers/v3d/v3d_ver.h"

GL_APICALL void GL_APIENTRY glUseProgramStages (GLuint pipeline, GLbitfield stages, GLuint program) {}
GL_APICALL void GL_APIENTRY glActiveShaderProgram (GLuint pipeline, GLuint program) {}
GL_APICALL GLuint GL_APIENTRY glCreateShaderProgramv (GLenum type, GLsizei count, const GLchar *const*strings) {return 0;}
GL_APICALL void GL_APIENTRY glBindProgramPipeline (GLuint pipeline) {}
GL_APICALL void GL_APIENTRY glDeleteProgramPipelines (GLsizei n, const GLuint *pipelines) {}
GL_APICALL void GL_APIENTRY glGenProgramPipelines (GLsizei n, GLuint *pipelines) {}
GL_APICALL GLboolean GL_APIENTRY glIsProgramPipeline (GLuint pipeline) {return GL_FALSE;}
GL_APICALL void GL_APIENTRY glGetProgramPipelineiv (GLuint pipeline, GLenum pname, GLint *params) {}

GL_APICALL void GL_APIENTRY glValidateProgramPipeline (GLuint pipeline) {}
GL_APICALL void GL_APIENTRY glGetProgramPipelineInfoLog (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {}
GL_APICALL void GL_APIENTRY glBindImageTexture (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) {}
