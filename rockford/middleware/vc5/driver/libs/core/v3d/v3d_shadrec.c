/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "v3d_shadrec.h"
#include "v3d_align.h"

// Describe the size and alignment of the new memory blocks needed by
// v3d_create_nv_shader_record.
void v3d_get_nv_shader_record_alloc_sizes(V3D_NV_SHADER_RECORD_ALLOC_SIZES_T *sizes)
{
   unsigned num_attr = 2;       // Standard shaded vertex format has 2 attrs

   sizes->packed_shader_rec_size = V3D_SHADREC_GL_MAIN_PACKED_SIZE +
                                   num_attr * V3D_SHADREC_GL_ATTR_PACKED_SIZE;
   sizes->packed_shader_rec_align = V3D_SHADREC_ALIGN;

   sizes->defaults_size = 2 * 16;
   sizes->defaults_align = V3D_ATTR_DEFAULTS_ALIGN;
}
// Work around GFXH-1276. 0 varyings causes memory corruption.
#if !V3D_VER_AT_LEAST(3,3,0,0)
void v3d_workaround_gfxh_1276(V3D_SHADREC_GL_MAIN_T *record)
{
   /* This is more complicated for points, so for now skip the workaround */
   if (record->num_varys == 0 && !record->point_size_included)
      record->num_varys = 1;
}
#endif

// Packs a float attribute with given size and number of cs/vs reads into
// i'th attribute in shader record. Packed base points to first attribute.
static void make_simple_float_shader_attr(V3D_SHADREC_GL_ATTR_T *attr,
   int size, int cs_reads, int vs_reads, v3d_addr_t addr, int stride)
{
   attr->addr = addr;
   attr->size = size;
   attr->type = V3D_ATTR_TYPE_FLOAT;
   attr->signed_int = false;
   attr->normalised_int = false;
   attr->read_as_int = false;
   attr->cs_num_reads = cs_reads;
   attr->vs_num_reads = vs_reads;
   attr->divisor = 0;
   attr->stride = stride;
}

void v3d_create_nv_shader_record(
   uint32_t         *packed_shader_rec_ptr,
   v3d_addr_t        packed_shader_rec_addr,
   uint32_t         *defaults_ptr,
   v3d_addr_t        defaults_addr,
   v3d_addr_t        fshader_addr,
   v3d_addr_t        funif_addr,
   v3d_addr_t        vdata_addr,
   bool              does_z_writes,
   v3d_threading_t   threading)
{
   V3D_SHADREC_GL_MAIN_T shader_record;
   V3D_SHADREC_GL_ATTR_T attr[3];
   unsigned num_attr = 2;       // Standard shaded vertex format
   unsigned vdata_stride = 12;  // Xs, Ys, Zs (clip header and 1/Wc from defaults)

   // First attribute contains clip header (dummy values).
   // Last value is is 1/Wc for second attribute.
   for (int i = 0; i < 8; ++i)
      defaults_ptr[i] = gfx_float_to_bits((i == 7) ? 1.0f : 0.0f);

   shader_record.point_size_included = false;
   shader_record.clipping = false;
   shader_record.cs_vertex_id = false;       // N/A in NV shader record
   shader_record.cs_instance_id = false;     // N/A in NV shader record
   shader_record.vs_vertex_id = false;       // N/A in NV shader record
   shader_record.vs_instance_id = false;     // N/A in NV shader record
   shader_record.z_write = does_z_writes;
   shader_record.no_ez = false;              // TODO Set this if needed
   shader_record.cs_separate_blocks = false; // N/A in NV shader record
   shader_record.vs_separate_blocks = false; // N/A in NV shader record
   shader_record.scb_wait_on_first_thrsw = false;
   shader_record.disable_scb = false;
#if V3D_HAS_VARY_DISABLE
   shader_record.disable_implicit_varys = false;
#endif
#if V3D_HAS_TNG
   shader_record.cs_baseinstance = false;
   shader_record.vs_baseinstance = false;
#endif
#if V3D_HAS_INLINE_CLIP
   shader_record.prim_id_used = false;
   shader_record.prim_id_to_fs = false;
#endif
   // TODO centroid flag
#if V3D_HAS_SRS
   shader_record.sample_rate_shading = false;
#endif

   shader_record.num_varys = 0;

#if V3D_HAS_TNG
   // Coord and IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.cs_output_size = (V3D_OUT_SEG_ARGS_T) { .sectors = 1, .pack = V3D_CL_VPM_PACK_X16 };
   shader_record.cs_input_size = (V3D_IN_SEG_ARGS_T) { .sectors = 0, .min_req = 1 };
   // Vertex IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.vs_output_size = (V3D_OUT_SEG_ARGS_T) { .sectors = 1, .pack = V3D_CL_VPM_PACK_X16 };
   shader_record.vs_input_size = (V3D_IN_SEG_ARGS_T) { .sectors = 0, .min_req = 1 };
#else
   // Coord and IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.cs_output_size = 1;
   // Ignored in NV shader
   shader_record.cs_input_size = 0;
   // Vertex IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.vs_output_size = 1;
   // Ignored in NV shader
   shader_record.vs_input_size = 0;
#endif
   shader_record.defaults = defaults_addr;
   shader_record.fs.threading = threading;
   shader_record.vs.threading = V3D_THREADING_T1; // Ignored in NV shader
   shader_record.cs.threading = V3D_THREADING_T1; // Ignored in NV shader
   shader_record.fs.propagate_nans = false;
   shader_record.vs.propagate_nans = false; // Ignored in NV shader
   shader_record.cs.propagate_nans = false; // Ignored in NV shader
   shader_record.fs.addr = fshader_addr;
   shader_record.vs.addr = 0;
   shader_record.cs.addr = 0;
   shader_record.cs.unifs_addr = 0;
   shader_record.vs.unifs_addr = 0;
   shader_record.fs.unifs_addr = funif_addr;

   // Modify the shader record to avoid specifying 0 varyings
#if !V3D_VER_AT_LEAST(3,3,0,0)
   v3d_workaround_gfxh_1276(&shader_record);
#endif

   // Xc, Yc, Zc, Wc - clipping is disabled so we just use dummy default values
   make_simple_float_shader_attr(&attr[0], 4, 4, 0, shader_record.defaults, 0);

   // Xs, Ys, Zs from attribute data - 1/Wc from default values (1.0)
   make_simple_float_shader_attr(&attr[1], 3, 4, 4, vdata_addr, vdata_stride);

   // This allocation contains both main GL shader record,
   // followed by variable number of attribute records.
   char *attrs_packed = (char*)packed_shader_rec_ptr + V3D_SHADREC_GL_MAIN_PACKED_SIZE;
   for (unsigned i = 0; i < num_attr; ++i)
      v3d_pack_shadrec_gl_attr((uint32_t*)(attrs_packed + i * V3D_SHADREC_GL_ATTR_PACKED_SIZE), &attr[i]);

   v3d_pack_shadrec_gl_main(packed_shader_rec_ptr, &shader_record);
}
