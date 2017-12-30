/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>

//gl 1.1 specific functions
extern void glAlphaFunc_impl_11(GLenum func, GLclampf ref);
extern void glAlphaFuncx_impl_11(GLenum func, GLclampx ref);
extern void glClearColorx_impl_11(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
extern void glClearDepthx_impl_11(GLclampx depth);
extern void glClipPlanef_impl_11(GLenum plane, const GLfloat *equation);
extern void glClipPlanex_impl_11(GLenum plane, const GLfixed *equation);
extern void glDepthRangex_impl_11(GLclampx zNear, GLclampx zFar);
extern void glFogf_impl_11(GLenum pname, GLfloat param);
extern void glFogfv_impl_11(GLenum pname, const GLfloat *params);
extern void glFrustumf_impl_11(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
extern void glFogx_impl_11(GLenum pname, GLfixed param);
extern void glFogxv_impl_11(GLenum pname, const GLfixed *params);
extern void glFrustumx_impl_11(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
extern void glGetClipPlanex_impl_11(GLenum pname, GLfixed eqn[4]);
 extern int glGetLightfv_impl_11(GLenum light, GLenum pname, GLfloat *params);
extern int glGetLightxv_impl_11(GLenum light, GLenum pname, GLfixed *params);
extern int glGetMaterialxv_impl_11(GLenum face, GLenum pname, GLfixed *params);
extern int glGetMaterialfv_impl_11(GLenum face, GLenum pname, GLfloat *params);
extern void glGetClipPlanef_impl_11(GLenum pname, GLfloat eqn[4]);
extern int glGetFixedv_impl_11(GLenum pname, GLfixed *params);
extern int glGetTexEnvfv_impl_11(GLenum env, GLenum pname, GLfloat *params);
extern int glGetTexEnviv_impl_11(GLenum env, GLenum pname, GLint *params);
extern int glGetTexEnvxv_impl_11(GLenum env, GLenum pname, GLfixed *params);
extern int glGetTexParameterxv_impl_11(GLenum target, GLenum pname, GLfixed *params);
extern void glLightModelf_impl_11(GLenum pname, GLfloat param);
extern void glLightModelfv_impl_11(GLenum pname, const GLfloat *params);
extern void glLightf_impl_11(GLenum light, GLenum pname, GLfloat param);
extern void glLightfv_impl_11(GLenum light, GLenum pname, const GLfloat *params);
extern void glLightModelx_impl_11(GLenum pname, GLfixed param);
extern void glLightModelxv_impl_11(GLenum pname, const GLfixed *params);
extern void glLightx_impl_11(GLenum light, GLenum pname, GLfixed param);
extern void glLightxv_impl_11(GLenum light, GLenum pname, const GLfixed *params);
extern void glLineWidthx_impl_11(GLfixed width);
extern void glLoadIdentity_impl_11(void);
extern void glLoadMatrixf_impl_11(const GLfloat *m);
extern void glLoadMatrixx_impl_11(const GLfixed *m);
extern void glLogicOp_impl_11(GLenum opcode);
extern void glMaterialf_impl_11(GLenum face, GLenum pname, GLfloat param);
extern void glMaterialfv_impl_11(GLenum face, GLenum pname, const GLfloat *params);
extern void glMultMatrixf_impl_11(const GLfloat *m);
extern void glOrthof_impl_11(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
extern void glPolygonOffsetx_impl_11(GLfixed factor, GLfixed units);
extern void glPointParameterf_impl_11(GLenum pname, GLfloat param);
extern void glPointParameterfv_impl_11(GLenum pname, const GLfloat *params);
extern void glRotatef_impl_11(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void glSampleCoveragex_impl_11(GLclampx value, GLboolean invert);
extern void glScalef_impl_11(GLfloat x, GLfloat y, GLfloat z);
extern void glShadeModel_impl_11(GLenum model);
extern void glTexEnvf_impl_11(GLenum target, GLenum pname, GLfloat param);
extern void glTexEnvfv_impl_11(GLenum target, GLenum pname, const GLfloat *params);
extern void glTexEnvi_impl_11(GLenum target, GLenum pname, GLint param);
extern void glTexEnviv_impl_11(GLenum target, GLenum pname, const GLint *params);
extern void glTexEnvx_impl_11(GLenum target, GLenum pname, GLfixed param);
extern void glTexEnvxv_impl_11(GLenum target, GLenum pname, const GLfixed *params);
extern void glTexParameterx_impl_11(GLenum target, GLenum pname, GLfixed param);
extern void glTexParameterxv_impl_11(GLenum target, GLenum pname, const GLfixed *params);
extern void glTranslatef_impl_11(GLfloat x, GLfloat y, GLfloat z);
extern void glMaterialx_impl_11(GLenum face, GLenum pname, GLfixed param);
extern void glMaterialxv_impl_11(GLenum face, GLenum pname, const GLfixed *params);
extern void glMatrixMode_impl_11(GLenum mode);
extern void glMultMatrixx_impl_11(const GLfixed *m);
extern void glOrthox_impl_11(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
extern void glPointParameterx_impl_11(GLenum pname, GLfixed param);
extern void glPointParameterxv_impl_11(GLenum pname, const GLfixed *params);
extern void glPointSizePointerOES_impl_11(void);
extern void glPopMatrix_impl_11(void);
extern void glPushMatrix_impl_11(void);
extern void glRotatex_impl_11(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
extern void glScalex_impl_11(GLfixed x, GLfixed y, GLfixed z);
extern void glTranslatex_impl_11(GLfixed x, GLfixed y, GLfixed z);

extern void glColorPointer_impl_11(void);
extern void glNormalPointer_impl_11(void);
extern void glTexCoordPointer_impl_11(GLenum unit);
extern void glVertexPointer_impl_11(void);

/*****************************************************************************************/
/*                                 OES extension functions                               */
/*****************************************************************************************/

//gl 1.1 specific
extern void glintColor_impl_11(float red, float green, float blue, float alpha);
extern void glQueryMatrixxOES_impl_11(GLfixed mantissa[16]);
extern void glDrawTexfOES_impl_11(GLfloat Xs, GLfloat Ys, GLfloat Zs, GLfloat Ws, GLfloat Hs, bool secure);