/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG server

FILE DESCRIPTION
Top-level VG server-side functions.
=============================================================================*/

#ifndef VG_INT_IMPL_H
#define VG_INT_IMPL_H

extern void vgClearError_impl(void);
extern void vgSetError_impl(VGErrorCode error);
extern VGErrorCode vgGetError_impl(void);
extern void vgFlush_impl(void);
extern VGuint vgFinish_impl(void);
extern VGuint vgCreateStems_impl(VGuint count, VGHandle *vg_handles);
extern void vgDestroyStem_impl(VGHandle vg_handle);
extern void vgSetiv_impl(VGParamType param_type, VGuint count, const VGint *values);
extern void vgSetfv_impl(VGParamType param_type, VGuint count, const VGfloat *values);
extern void vgGetfv_impl(VGParamType param_type, VGint count, VGfloat *values);
extern void vgSetParameteriv_impl(VGHandle vg_handle, VG_CLIENT_OBJECT_TYPE_T client_object_type, VGint param_type, VGint count, const VGint *values);
extern void vgSetParameterfv_impl(VGHandle vg_handle, VG_CLIENT_OBJECT_TYPE_T client_object_type, VGint param_type, VGuint count, const VGfloat *values);
extern bool vgGetParameteriv_impl(VGHandle vg_handle, VG_CLIENT_OBJECT_TYPE_T client_object_type, VGint param_type, VGint count, VGint *values);
extern void vgLoadMatrix_impl(VGMatrixMode matrix_mode, const VG_MAT3X3_T *matrix);
extern void vgMask_impl(VGImage vg_handle, VGMaskOperation operation, VGint dst_x, VGint dst_y, VGint width, VGint height);
extern void vgRenderToMask_impl(VGPath vg_handle, VGbitfield paint_modes, VGMaskOperation operation);
extern void vgCreateMaskLayer_impl(VGHandle vg_handle, VGint width, VGint height);
extern void vgDestroyMaskLayer_impl(VGMaskLayer vg_handle);
extern void vgFillMaskLayer_impl(VGMaskLayer vg_handle, VGint x, VGint y, VGint width, VGint height, VGfloat value);
extern void vgCopyMask_impl(VGMaskLayer dst_vg_handle, VGint dst_x, VGint dst_y, VGint src_x, VGint src_y, VGint width, VGint height);
extern void vgClear_impl(VGint x, VGint y, VGint width, VGint height);
extern void vgCreatePath_impl(VGHandle vg_handle, VGint format, VGPathDatatype datatype, VGfloat scale, VGfloat bias, VGint segments_capacity, VGint coords_capacity, VGbitfield caps);
extern void vgClearPath_impl(VGPath vg_handle, VGbitfield caps);
extern void vgDestroyPath_impl(VGPath vg_handle);
extern void vgRemovePathCapabilities_impl(VGPath vg_handle, VGbitfield caps);
extern void vgAppendPath_impl(VGPath dst_vg_handle, VGPath src_vg_handle);
extern void vgAppendPathData_impl(VGPath vg_handle, VGPathDatatype datatype, VGint segments_count, const VGubyte *segments, VGuint coords_size, const void *coords);
extern void vgModifyPathCoords_impl(VGPath vg_handle, VGPathDatatype datatype, VGuint coords_offset, VGuint coords_size, const void *coords);
extern void vgTransformPath_impl(VGPath dst_vg_handle, VGPath src_vg_handle);
extern void vgInterpolatePath_impl(VGPath dst_vg_handle, VGPath begin_vg_handle, VGPath end_vg_handle, VGfloat t);
extern VGfloat vgPathLength_impl(VGPath vg_handle, VGint segments_i, VGint segments_count);
extern bool vgPointAlongPath_impl(VGPath vg_handle, VGint segments_i, VGint segments_count, VGfloat distance, VGbitfield mask, VGfloat *values);
extern bool vgPathBounds_impl(VGPath vg_handle, VGfloat *values);
extern bool vgPathTransformedBounds_impl(VGPath vg_handle, VGfloat *values);
extern void vgDrawPath_impl(VGPath vg_handle, VGbitfield paint_modes);
extern void vgCreatePaint_impl(VGHandle vg_handle);
extern void vgDestroyPaint_impl(VGPaint vg_handle);
extern void vgSetPaint_impl(VGPaint vg_handle, VGbitfield paint_modes);
extern void vgPaintPattern_impl(VGPaint vg_handle, VGImage pattern_vg_handle);
extern void vgCreateImage_impl(VGHandle vg_handle, VGImageFormat format, VGint width, VGint height, VGbitfield allowed_quality);
extern void vgDestroyImage_impl(VGImage vg_handle);
extern void vgClearImage_impl(VGImage vg_handle, VGint x, VGint y, VGint width, VGint height);
extern void vgImageSubData_impl(VGImage vg_handle, VGint dst_width, VGint dst_height, const void *data, VGint data_stride, VGImageFormat data_format, VGint src_x, VGint dst_x, VGint dst_y, VGuint width, VGuint height);
extern bool vgGetImageSubData_impl(VGImage vg_handle, VGint src_width, VGint src_height, void *data, VGint data_stride, VGImageFormat data_format, VGint dst_x, VGint src_x, VGint src_y, VGint width, VGint height);
extern void vgChildImage_impl(VGHandle vg_handle, VGImage parent_vg_handle, VGint parent_width, VGint parent_height, VGint x, VGint y, VGint width, VGint height);
extern VGImage vgGetParent_impl(VGImage vg_handle);
extern void vgCopyImage_impl(VGImage dst_vg_handle, VGint dst_x, VGint dst_y, VGImage src_vg_handle, VGint src_x, VGint src_y, VGint width, VGint height, bool dither);
extern void vgDrawImage_impl(VGImage vg_handle);
extern void vgSetPixels_impl(VGint dst_x, VGint dst_y, VGImage src_vg_handle, VGint src_x, VGint src_y, VGint width, VGint height);
extern void vgWritePixels_impl(const void *data, VGint data_stride, VGImageFormat data_format, VGint src_x, VGint dst_x, VGint dst_y, VGint width, VGint height);
extern void vgGetPixels_impl(VGImage dst_vg_handle, VGint dst_x, VGint dst_y, VGint src_x, VGint src_y, VGint width, VGint height);
extern void vgReadPixels_impl(void *data, VGint data_stride, VGImageFormat data_format, VGint dst_x, VGint src_x, VGint src_y, VGint width, VGint height);
extern void vgCopyPixels_impl(VGint dst_x, VGint dst_y, VGint src_x, VGint src_y, VGint width, VGint height);
extern void vgCreateFont_impl(VGHandle vg_handle, VGint glyphs_capacity);
extern void vgDestroyFont_impl(VGFont vg_handle);
extern void vgSetGlyphToPath_impl(VGFont vg_handle, VGuint glyph_id, VGPath path_vg_handle, bool is_hinted, VGfloat glyph_origin_x, VGfloat glyph_origin_y, VGfloat escapement_x, VGfloat escapement_y);
extern void vgSetGlyphToImage_impl(VGFont vg_handle, VGuint glyph_id, VGImage image_vg_handle, VGfloat glyph_origin_x, VGfloat glyph_origin_y, VGfloat escapement_x, VGfloat escapement_y);
extern void vgClearGlyph_impl(VGFont vg_handle, VGuint glyph_id);
extern void vgDrawGlyph_impl(VGFont vg_handle, VGuint glyph_id, VGbitfield paint_modes, bool allow_autohinting);
extern void vgDrawGlyphs_impl(VGFont vg_handle, VGuint glyphs_count, const VGuint *glyph_ids, const VGfloat *adjustments_x, const VGfloat *adjustments_y, VGbitfield paint_modes, bool allow_autohinting);
extern void vgColorMatrix_impl(VGImage dst_vg_handle, VGImage src_vg_handle, const VGfloat *matrix);
extern void vgConvolve_impl(VGImage dst_vg_handle, VGImage src_vg_handle, VGint kernel_width, VGint kernel_height, VGint shift_x, VGint shift_y, VGfloat scale, VGfloat bias, VGTilingMode tiling_mode, const VGshort *kernel);
extern void vgSeparableConvolve_impl(VGImage dst_vg_handle, VGImage src_vg_handle, VGint kernel_width, VGint kernel_height, VGint shift_x, VGint shift_y, const VGshort *kernel_x, const VGshort *kernel_y, VGfloat scale, VGfloat bias, VGTilingMode tiling_mode);
extern void vgGaussianBlur_impl(VGImage dst_vg_handle, VGImage src_vg_handle, VGfloat std_dev_x, VGfloat std_dev_y, VGTilingMode tiling_mode);
extern void vgLookup_impl(VGImage dst_vg_handle, VGImage src_vg_handle, const VGubyte *red_lut, const VGubyte *green_lut, const VGubyte *blue_lut, const VGubyte *alpha_lut, bool output_linear, bool output_pre);
extern void vgLookupSingle_impl(VGImage dst_vg_handle, VGImage src_vg_handle, VGImageChannel source_channel, bool output_linear, bool output_pre, const VGuint *lut);
extern void vguLine_impl(VGPath vg_handle, VGfloat p0_x, VGfloat p0_y, VGfloat p1_x, VGfloat p1_y);
extern void vguPolygon_impl(VGPath vg_handle, const VGfloat *ps, VGint ps_count, bool first, bool close);
extern void vguRect_impl(VGPath vg_handle, VGfloat x, VGfloat y, VGfloat width, VGfloat height);
extern void vguRoundRect_impl(VGPath vg_handle, VGfloat x, VGfloat y, VGfloat width, VGfloat height, VGfloat arc_width, VGfloat arc_height);
extern void vguEllipse_impl(VGPath vg_handle, VGfloat x, VGfloat y, VGfloat width, VGfloat height);
extern void vguArc_impl(VGPath vg_handle, VGfloat x, VGfloat y, VGfloat width, VGfloat height, VGfloat start_angle, VGfloat angle_extent, VGuint angle_o180, VGUArcType arc_type);
#if VG_KHR_EGL_image
extern VGImage vgCreateEGLImageTargetKHR_impl(VGeglImageKHR src_egl_handle, VGuint *format_width_height);
#endif

#endif /* VG_INT_IMPL_H */