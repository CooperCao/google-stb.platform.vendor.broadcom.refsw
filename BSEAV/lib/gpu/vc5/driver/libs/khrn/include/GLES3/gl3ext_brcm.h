/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * FILE DESCRIPTION
 * Extensions which we invented. Included by gl_public_api.h (used in the driver).
 * If applications want to use these they must include this file explicitly.
 *
 * THIS INFORMATION SHOULD BE CONSISTENT WITH THE XML VERSION IN
 * <sw_root>/scripts/hook_codegen/brcmgl.xml
 *
 ******************************************************************************/

#ifndef __gl3ext_brcm_h_
#define __gl3ext_brcm_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GL_APIENTRYP
#define GL_APIENTRYP GL_APIENTRY*
#endif

/*  http://www.opengl.org/registry/api/enum.spec                              */
/*                                                                            */
/*  If an extension is experimental, allocate temporary enum values in the    */
/*  range 0x6000-0x8000 during development work.  When the vendor commits to  */
/*  releasing the extension, allocate permanent enum values (see link).       */

#ifndef GL_BRCM_image_formats
#define GL_BRCM_image_formats 1
#endif /* GL_BRCM_image_formats */

#ifndef GL_BRCM_no_perspective
#define GL_BRCM_no_perspective 1
#endif /* GL_BRCM_no_perspective */

#ifndef GL_BRCM_polygon_mode
#define GL_BRCM_polygon_mode 1
#define GL_POINT_BRCM                     0x1B00
#define GL_LINE_BRCM                      0x1B01
#define GL_FILL_BRCM                      0x1B02
typedef void (GL_APIENTRYP PFNGLPOLYGONMODEBRCMPROC) (GLenum mode);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glPolygonModeBRCM (GLenum mode);
#endif
#endif /* GL_BRCM_polygon_mode */

#ifndef GL_BRCM_provoking_vertex
#define GL_BRCM_provoking_vertex 1
#define GL_FIRST_VERTEX_CONVENTION_BRCM   0x8E4D
#define GL_LAST_VERTEX_CONVENTION_BRCM    0x8E4E
#define GL_PROVOKING_VERTEX_BRCM          0x8E4F
typedef void (GL_APIENTRYP PFNGLPROVOKINGVERTEXBRCMPROC) (GLenum mode);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glProvokingVertexBRCM (GLenum mode);
#endif
#endif /* GL_BRCM_provoking_vertex */

#ifndef GL_BRCM_sampler_fetch
#define GL_BRCM_sampler_fetch 1
#endif /* GL_BRCM_sampler_fetch */

#ifndef GL_BRCM_shader_framebuffer_fetch_depth_stencil
#define GL_BRCM_shader_framebuffer_fetch_depth_stencil 1
#endif /* GL_BRCM_shader_framebuffer_fetch_depth_stencil */

#ifndef GL_BRCM_texture_1D
#define GL_BRCM_texture_1D 1
#define GL_TEXTURE_1D_BRCM                0x0DE0
#define GL_TEXTURE_1D_ARRAY_BRCM          0x8C18
#define GL_SAMPLER_1D_BRCM                0x8B5D
#define GL_SAMPLER_1D_ARRAY_BRCM          0x8DC0
#define GL_INT_SAMPLER_1D_BRCM            0x8DC9
#define GL_INT_SAMPLER_1D_ARRAY_BRCM      0x8DCE
#define GL_UNSIGNED_INT_SAMPLER_1D_BRCM   0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_BRCM 0x8DD6
#define GL_TEXTURE_BINDING_1D_BRCM        0x8068
#define GL_TEXTURE_BINDING_1D_ARRAY_BRCM  0x8C1C
typedef void (GL_APIENTRYP PFNGLTEXIMAGE1DBRCMPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLTEXSUBIMAGE1DBRCMPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRYP PFNGLTEXSTORAGE1DBRCMPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
#ifdef GL_GLEXT_PROTOTYPES
GL_APICALL void GL_APIENTRY glTexImage1DBRCM (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glTexSubImage1DBRCM (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GL_APICALL void GL_APIENTRY glTexStorage1DBRCM (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
#endif
#endif /* GL_BRCM_texture_1D */

#ifndef GL_BRCM_texture_gather_lod
#define GL_BRCM_texture_gather_lod 1
#endif /* GL_BRCM_texture_gather_lod */

#ifndef GL_BRCM_texture_norm16
#define GL_BRCM_texture_norm16 1
#define GL_R16_BRCM                       0x822A
#define GL_RG16_BRCM                      0x822C
#define GL_RGBA16_BRCM                    0x805B
#define GL_R16_SNORM_BRCM                 0x8F98
#define GL_RG16_SNORM_BRCM                0x8F99
#define GL_RGBA16_SNORM_BRCM              0x8F9B
#endif /* GL_BRCM_texture_norm16 */

#ifndef GL_BRCM_texture_unnormalised_coords
#define GL_BRCM_texture_unnormalised_coords 1
#define GL_TEXTURE_UNNORMALISED_COORDS_BRCM 0x7900
#endif /* GL_BRCM_texture_unnormalised_coords */

#ifdef __cplusplus
}
#endif

#endif
