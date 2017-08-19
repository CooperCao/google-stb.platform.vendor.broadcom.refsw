/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_shadrec.h"
#include "v3d_align.h"

static const unsigned NUM_ATTR = 2;       // Standard shaded vertex format has 2 attrs

// Describe the size and alignment of the new memory blocks needed by
// v3d_create_nv_shader_record.
void v3d_get_nv_shader_record_alloc_sizes(V3D_NV_SHADER_RECORD_ALLOC_SIZES_T *sizes)
{
   sizes->packed_shader_rec_size = V3D_SHADREC_GL_MAIN_PACKED_SIZE +
                                   NUM_ATTR * V3D_SHADREC_GL_ATTR_PACKED_SIZE;
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
static void push_simple_float_shader_attr(uint32_t **attr_packed,
      unsigned size, unsigned cs_reads, unsigned vs_reads, v3d_addr_t addr, unsigned stride,
      uint32_t max_index)
{
   V3D_SHADREC_GL_ATTR_T attr = { 0, };
   attr.addr = addr;
   attr.size = size;
   attr.type = V3D_ATTR_TYPE_FLOAT;
   attr.cs_num_reads = cs_reads;
   attr.vs_num_reads = vs_reads;
   attr.stride = stride;
#if V3D_HAS_ATTR_MAX_INDEX
   attr.max_index = max_index;
#endif

   v3d_pack_shadrec_gl_attr(*attr_packed, &attr);

   static_assrt(V3D_SHADREC_GL_ATTR_PACKED_SIZE % sizeof(uint32_t) == 0);
   *attr_packed += V3D_SHADREC_GL_ATTR_PACKED_SIZE/sizeof(uint32_t);
}

void v3d_create_nv_shader_record(
   uint32_t         *packed_shader_rec_ptr,
   v3d_addr_t        packed_shader_rec_addr,
   uint32_t         *defaults_ptr,
   v3d_addr_t        defaults_addr,
   v3d_addr_t        fshader_addr,
   v3d_addr_t        funif_addr,
   v3d_addr_t        vdata_addr,
   uint32_t          vdata_max_index,
   bool              does_z_writes,
   v3d_threading_t   threading)
{
   V3D_SHADREC_GL_MAIN_T shader_record = { 0, };
   unsigned vdata_stride = 12;  // Xs, Ys, Zs (clip header and 1/Wc from defaults)

   // First attribute contains clip header (dummy values).
   // Last value is is 1/Wc for second attribute.
   for (int i = 0; i < 8; ++i)
      defaults_ptr[i] = gfx_float_to_bits((i == 7) ? 1.0f : 0.0f);

#if V3D_VER_AT_LEAST(4,0,2,0)
   // Coord and IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.cs_output_size = (V3D_OUT_SEG_ARGS_T) { .sectors = 1, };
   shader_record.cs_input_size = (V3D_IN_SEG_ARGS_T) { .sectors = 0, .min_req = 1 };
   // Vertex IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.vs_output_size = (V3D_OUT_SEG_ARGS_T) { .sectors = 1, };
   shader_record.vs_input_size = (V3D_IN_SEG_ARGS_T) { .sectors = 0, .min_req = 1 };
#else
   // Coord and IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.cs_output_size = 1;
   // Vertex IO VPM segment, measured in units of 8 words (we only need 3 words)
   shader_record.vs_output_size = 1;
#endif
   shader_record.defaults = defaults_addr;
   shader_record.fs.threading = threading;
   shader_record.fs.addr = fshader_addr;
   shader_record.fs.unifs_addr = funif_addr;

   // Modify the shader record to avoid specifying 0 varyings
#if !V3D_VER_AT_LEAST(3,3,0,0)
   v3d_workaround_gfxh_1276(&shader_record);
#endif

   // This allocation contains both main GL shader record,
   // followed by variable number of attribute records.
   v3d_pack_shadrec_gl_main(packed_shader_rec_ptr, &shader_record);

   static_assrt(V3D_SHADREC_GL_MAIN_PACKED_SIZE % sizeof(uint32_t) == 0);
   uint32_t *attrs = packed_shader_rec_ptr + V3D_SHADREC_GL_MAIN_PACKED_SIZE/sizeof(uint32_t);

   // Xc, Yc, Zc, Wc - clipping is disabled so we just use dummy default values
   push_simple_float_shader_attr(&attrs, 4, 4, 0, shader_record.defaults, 0, /*max_index*/ 0);

   // Xs, Ys, Zs from attribute data - 1/Wc from default values (1.0)
   push_simple_float_shader_attr(&attrs, 3, 4, 4, vdata_addr, vdata_stride, vdata_max_index);
}
