/*=============================================================================
 * Broadcom Proprietary and Confidential. (c)2015 Broadcom.
 * All rights reserved.
 * =============================================================================*/

#include "glxx_compute.h"
#include "gl_public_api.h"
#include "glxx_int_config.h"
#include "../common/khrn_render_state.h"
#include "glxx_server.h"
#include "glxx_server_pipeline.h"
#include "../gl20/gl20_program.h"
#include "libs/core/v3d/v3d_vpm_alloc.h"
#include "libs/util/profile/profile.h"

#if KHRN_GLES31_DRIVER

LOG_DEFAULT_CAT("glxx_compute")

typedef uint16_t tlb_uint_t;
#define TLB_UINT_BITS (sizeof(tlb_uint_t)*8u)
static_assrt(TLB_UINT_BITS == 8 || TLB_UINT_BITS == 16);

#define TLB_UINT_MAX ((1u << TLB_UINT_BITS) - 1u)

static_assrt(V3D_MAX_TLB_WIDTH_PX >= 64);
static_assrt(V3D_MAX_TLB_HEIGHT_PX >= 64);
#define TLB_WIDTH 64
#define TLB_HEIGHT (64 / (TLB_UINT_BITS/8))

static_assrt(sizeof(unsigned) >= 4);

#define COMPUTE_MAX_BLOCKS (KHRN_FMEM_MAX_BLOCKS/8)

typedef struct compute_dispatch_cmd
{
   uint32_t num_groups[3];
} compute_dispatch_cmd;

typedef struct compute_dispatch_volume
{
   unsigned origin[3];  // In work groups.
   unsigned size[3];    // In work groups.
} compute_dispatch_volume;

/*
 * Organisational structure of work-items:
 * item     : 1 work-item.
 * group    : work-group of Sx*Sy*Sz items.
 * row      : row of groups totalling 16-128 items, share single sync object (if required).
 * instance : column of rows with items occupying unique pixels in TLB.
 */

typedef struct compute_dispatch_config
{
   unsigned items_per_group;
   unsigned max_groups_per_row;
   unsigned groups_per_instance;
   unsigned whole_rows_per_instance;

   unsigned group_size[3];    // 3D num items in group.
   unsigned instance_size[3]; // 3D num groups in instance.
   unsigned shared_block_size;
} compute_dispatch_config;

static unsigned get_shared_buf_size(void)
{
   // Limit concurrent shared memory use to half the L2 cache size.
   unsigned const l2_size = 128 * 1024;
   return l2_size / 2;
}

static unsigned compute_num_instances(compute_dispatch_config const* cfg, compute_dispatch_volume const* vol)
{
   unsigned num_groups = vol->size[0] * vol->size[1] * vol->size[2];
   assert((num_groups % cfg->groups_per_instance) == 0);
   return num_groups / cfg->groups_per_instance;
}

static bool init_dispatch_config(
   compute_dispatch_config* cfg,
   v3d_threading_t threading,
   unsigned const wg_size[3],
   unsigned const shared_block_size,
   unsigned const num_groups[3])
{
   unsigned items_per_group = 1;
   for (unsigned i = 0; i != 3; ++i)
   {
      cfg->group_size[i] = wg_size[i];
      items_per_group *= wg_size[i];
   }
   cfg->items_per_group = items_per_group;
   cfg->shared_block_size = shared_block_size;

   // Compute maximum number of work-items that can run concurrently in one core.
   static_assrt(TLB_WIDTH * 2u >= GLXX_CONFIG_MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
   unsigned max_items_per_group = TLB_WIDTH * 2u;
   unsigned max_concurrent_groups = TLB_WIDTH * TLB_HEIGHT;
   if (shared_block_size > 0)
   {
      static_assrt(V3D_THREADING_T1 == 0 && V3D_THREADING_T2 == 1 && V3D_THREADING_T4 == 2);
      V3D_IDENT_T const* ident = v3d_scheduler_get_identity();
      max_items_per_group = gfx_umin(max_items_per_group, (ident->num_slices * ident->num_qpus_per_slice * V3D_VPAR) << threading);
      if (items_per_group > max_items_per_group)
         return false;

      // It is possible for the compiler to allocate more than the allowable memory due to padding.
      max_concurrent_groups = get_shared_buf_size() / shared_block_size;
      if (max_concurrent_groups == 0)
         return false;
   }

   // Compute maximum number of groups per row.
   // Limit to 1/4 of the shared memory size if possible.
   // Limit to powers of 2 to hopefully improve mapping of compute volumes into TLB.
   // timh-todo: set barrier quorum for partial rows to remove this limitation
   unsigned max_groups_per_row = gfx_umin(max_items_per_group / items_per_group, (max_concurrent_groups + 3u) / 4u);
   max_groups_per_row = gfx_next_power_of_2(max_groups_per_row + 1) / 2;

   unsigned max_groups_per_instance = gfx_umin(max_groups_per_row * (TLB_HEIGHT / 2), max_concurrent_groups);

   // Need to try and factor the 3D array of groups into a per-instance volume. Try and keep this simple for now.
   unsigned dims[3];
   unsigned num_dims = 0;
   for (unsigned i = 0; i != 3; ++i)
   {
      if (num_groups[i] > 1)
         dims[num_dims++] = i;
   }

   unsigned groups_per_instance = 1;
   unsigned instance_size[3] = { 1, 1, 1 };

   // Expand each dimension in turn until we can't.
   for (unsigned i = 0; num_dims != 0; )
   {
      // Wrap around.
      if (i == num_dims)
         i = 0;

      unsigned dim = dims[i];

      // Check we can grow this dimension within the dispatch volume.
      unsigned new_instance_size = gfx_umin(instance_size[dim] * 2, num_groups[dim]);
      if (new_instance_size > instance_size[dim])
      {
         // Check the global ID along this dimension doesn't overflow the TLB lookup table.
         if (TLB_UINT_MAX < 0xffff || new_instance_size * wg_size[dim] <= (TLB_UINT_MAX + 1))
         {
            // Check we can fit the additional work-groups into this instance.
            assert((groups_per_instance % instance_size[dim]) == 0);
            unsigned new_groups_per_instance = (groups_per_instance / instance_size[dim]) * new_instance_size;
            if (  new_groups_per_instance <= max_groups_per_instance
               && (new_groups_per_instance < max_groups_per_row || (new_groups_per_instance % max_groups_per_row) == 0)) // timh-todo: remove this limitation
            {
               // OK to expand this dimension.
               instance_size[dim] = new_instance_size;
               groups_per_instance = new_groups_per_instance;
               i += 1;
               continue;
            }
         }
      }

      // Remove this axis from candidates.
      for (unsigned j = i + 1; j != num_dims; ++j)
      {
         dims[j-1] = dims[j];
      }
      num_dims -= 1;
   }

   cfg->groups_per_instance = groups_per_instance;
   cfg->max_groups_per_row = gfx_umin(groups_per_instance, max_groups_per_row);
   cfg->whole_rows_per_instance = groups_per_instance / cfg->max_groups_per_row;
   assert((groups_per_instance % cfg->max_groups_per_row) == 0); // timh-todo: remove this limitation
   cfg->instance_size[0] = instance_size[0];
   cfg->instance_size[1] = instance_size[1];
   cfg->instance_size[2] = instance_size[2];
   return true;
}

static inline unsigned compose_index(unsigned const id[3], unsigned const size[3])
{
   assert(id[0] < size[0]);
   assert(id[1] < size[1]);
   assert(id[2] < size[2]);
   return (id[2]*size[1] + id[1])*size[0] + id[0];
}

static inline void decompose_index(unsigned id[3], unsigned index, unsigned const size[3])
{
   id[0] = index % size[0];
   id[1] = (index / size[0]) % size[1];
   id[2] = (index / size[0]) / size[1];
   assert(compose_index(id, size) == index);
}

static v3d_addr_t build_tlb_lookup(khrn_fmem* fmem, compute_dispatch_config const* cfg)
{
   tlb_uint_t* lookup = (tlb_uint_t*)khrn_fmem_data(fmem, TLB_WIDTH * TLB_HEIGHT * sizeof(tlb_uint_t) * 4, 256u);
   if (!lookup)
      return 0;

   GFX_BUFFER_DESC_PLANE_T plane = {
      .lfmt = TLB_UINT_BITS == 16 ? GFX_LFMT_R16G16B16A16_UINT_2D_UIF : GFX_LFMT_R8G8B8A8_UINT_2D_UIF,
      .offset = 0,
      .pitch = TLB_HEIGHT * sizeof(tlb_uint_t) * 4
   };
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, plane.lfmt);

   unsigned items_per_group = cfg->items_per_group;
   unsigned groups_per_instance = cfg->groups_per_instance;
   unsigned max_groups_per_row = cfg->max_groups_per_row;

   unsigned items_per_row = max_groups_per_row * items_per_group;
   unsigned lanes_per_row = gfx_uround_up_p2(items_per_row, V3D_VPAR);

   uint32_t row_index_shift = gfx_msb(items_per_group);
   row_index_shift += items_per_group > (1u << row_index_shift);

   unsigned num_whole_rows = cfg->whole_rows_per_instance;
   unsigned num_groups_partial_row = cfg->groups_per_instance - num_whole_rows*max_groups_per_row;
   for (unsigned row_index = 0; row_index <= num_whole_rows; ++row_index)
   {
      if (row_index == num_whole_rows && num_groups_partial_row == 0)
         break;

      for (unsigned item_index = 0; item_index != lanes_per_row; ++item_index)
      {
         uint32_t row_local_index = TLB_UINT_MAX;
         uint32_t global_id[3] = { 0, };

         // If lane is valid (not padding).
         unsigned group_index = row_index*max_groups_per_row + item_index/items_per_group;
         if (item_index < items_per_row && group_index < groups_per_instance)
         {
            uint32_t local_index = item_index % items_per_group;
            uint32_t row_index = item_index / items_per_group;

            row_local_index = (row_index << row_index_shift) | local_index;
            assert(row_local_index < TLB_UINT_MAX);

            uint32_t local_id[3];
            uint32_t group_id[3];
            decompose_index(local_id, local_index, cfg->group_size);
            decompose_index(group_id, group_index, cfg->instance_size);

            for (unsigned i = 0; i != 3; ++i)
            {
               global_id[i] = group_id[i]*cfg->group_size[i] + local_id[i];
               assert(global_id[i] <= TLB_UINT_MAX);
            }
         }

         unsigned x = item_index / 2;
         unsigned y = row_index * 2 + item_index % 2;
         unsigned offset = gfx_buffer_block_offset(&plane, &bd, x, y, 0, TLB_HEIGHT) / sizeof(tlb_uint_t);
         lookup[offset+0] = (tlb_uint_t)row_local_index;
         lookup[offset+1] = (tlb_uint_t)global_id[0];
         lookup[offset+2] = (tlb_uint_t)global_id[1];
         lookup[offset+3] = (tlb_uint_t)global_id[2];
      }
   }
   return khrn_fmem_hw_address(fmem, lookup);
}

static unsigned const NUM_PER_VERTEX_ATTRIBUTES = 2;

static bool build_per_vertex_data(uint8_t* packed_attrs, khrn_fmem* fmem, compute_dispatch_config const* cfg)
{
   unsigned num_whole_rows = cfg->whole_rows_per_instance;
   unsigned num_groups_partial_row = cfg->groups_per_instance - num_whole_rows * cfg->max_groups_per_row;

   unsigned max_items_per_row = cfg->max_groups_per_row * cfg->items_per_group;
   unsigned num_items_partial_row = num_groups_partial_row * cfg->items_per_group;

   // Need two lines if row has odd number of item.
   unsigned num_lines = (num_whole_rows << (max_items_per_row & 1))
                      + ((num_items_partial_row ? 1 : 0) << (num_items_partial_row & 1));

   unsigned sizeof_attr_data0 = sizeof(uint16_t) * 2 * 2 * num_lines;
   unsigned sizeof_attr_data1 = sizeof(float) * 2;

   uint8_t* const attr_data = (uint8_t*)khrn_fmem_data(fmem, sizeof_attr_data0 + sizeof_attr_data1, 4);
   if (!attr_data)
      return false;
   v3d_addr_t attr_addr = khrn_fmem_hw_address(fmem, attr_data);

   // Write per-vertex Xs,Ys data.
   {
      uint16_t* attr_data0 = (uint16_t*)attr_data;

      // Offset line along x by half a pixel (for diamond exit rasterisation).
      uint32_t const x0 = 128u + 0u;

      // timh-todo: use this line if using setmsf
      //uint32_t const x1 = 128u + gfx_uround_up(items_per_row, V3D_VPAR) * (256u / 2u);
      uint32_t x1 = 128u + (max_items_per_row / 2u) * 256u;
      bool is_odd = (max_items_per_row & 1) != 0;

      for (unsigned d = 0; d <= num_whole_rows; ++d)
      {
         if (d == num_whole_rows)
         {
            if (!num_items_partial_row)
               break;
            x1 = 128u + (num_items_partial_row / 2u) * 256u;
            is_odd = (num_items_partial_row & 1) != 0;
         }

         uint32_t const y = (d*2u + 1u) * 256u;
         attr_data0[0] = x0;
         attr_data0[1] = y;
         attr_data0[2] = x1;
         attr_data0[3] = y;
         attr_data0 += 4;

         // timh-todo: remove this if using setmsf
         // Odd fragment, made by rotating line and clipping the last pixel.
         if (is_odd)
         {
            attr_data0[0] = x1 + 128u;
            attr_data0[1] = y - 128u;
            attr_data0[2] = x1 + 128u;
            attr_data0[3] = y + 128u;
            attr_data0 += 4;
         }
      }
   }

   // Write constant Zs,1/Wc data
   {
      float* attr_data1 = (float*)(attr_data + sizeof_attr_data0);
      attr_data1[0] = 0.0f;
      attr_data1[1] = 1.0f;
   }

   // Pack per-vertex Xs,Ys attribute record.
   {
      V3D_SHADREC_GL_ATTR_T attr0 = { 0, };
      attr0.addr = attr_addr;
      attr0.size = 2;
      attr0.type = V3D_ATTR_TYPE_SHORT;
      attr0.read_as_int = true;
      attr0.vs_num_reads = 2;
      attr0.stride = sizeof(uint16_t) * 2;
      v3d_pack_shadrec_gl_attr((uint32_t*)packed_attrs, &attr0);
   }

   // Pack  constant Zs,1/Wc attribute record.
   {
      V3D_SHADREC_GL_ATTR_T attr1 = { 0, };
      attr1.addr = attr_addr + sizeof_attr_data0;
      attr1.size = 2;
      attr1.type = V3D_ATTR_TYPE_FLOAT;
      attr1.vs_num_reads = 2;
      v3d_pack_shadrec_gl_attr((uint32_t*)(packed_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE), &attr1);
   }

   return true;
}

static unsigned build_per_instance_data(
   uint8_t* packed_attrs,
   khrn_fmem* fmem,
   compute_dispatch_config const* cfg,
   compute_dispatch_volume const* vol,
   uint8_t const* vary_map,
   unsigned num_varys)
{
   // timh-todo: use 16-bit attributes if they fit.
   unsigned num_instances = compute_num_instances(cfg, vol);
   unsigned instances_size = sizeof(uint32_t) * (num_varys * num_instances + 1u/*overspill*/);
   assert(instances_size <= KHRN_FMEM_USABLE_BUFFER_SIZE);
   uint32_t* instances = (uint32_t*)khrn_fmem_begin_data(fmem, instances_size, V3D_ATTR_ALIGN);
   if (!instances)
      return 0;

   static_assrt(V3D_ATTR_ALIGN <= sizeof(uint32_t));
   V3D_SHADREC_GL_ATTR_T attr =
   {
      .addr = khrn_fmem_hw_address(fmem, instances),
      .size = 1u,
      .type = V3D_ATTR_TYPE_INT,
      .read_as_int = true,
      .vs_num_reads = 1u,
      .divisor = 1,
      .stride = sizeof(uint32_t) * num_varys
   };

   unsigned attr_index[3] = { num_varys, num_varys, num_varys };
   for (unsigned v = 0; v != num_varys; ++v)
   {
      v3d_pack_shadrec_gl_attr((uint32_t*)(packed_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE*v), &attr);
      attr.addr += sizeof(uint32_t);

      attr_index[vary_map[v]] = v;
   }

   unsigned const step_x = cfg->instance_size[0] * cfg->group_size[0];
   unsigned const step_y = cfg->instance_size[1] * cfg->group_size[1];
   unsigned const step_z = cfg->instance_size[2] * cfg->group_size[2];
   unsigned const begin_x = vol->origin[0] * cfg->group_size[0];
   unsigned const begin_y = vol->origin[1] * cfg->group_size[1];
   unsigned const begin_z = vol->origin[2] * cfg->group_size[2];
   unsigned const end_x = begin_x + vol->size[0] * cfg->group_size[0];
   unsigned const end_y = begin_y + vol->size[1] * cfg->group_size[1];
   unsigned const end_z = begin_z + vol->size[2] * cfg->group_size[2];

   // Fill out per instance elements (global ID offset in 3D).
   for (unsigned z = begin_z; z != end_z; z += step_z)
   {
      for (unsigned y = begin_y; y != end_y; y += step_y)
      {
         for (unsigned x = begin_x; x != end_x; x += step_x)
         {
            // Write all three attributes, the unwanted ones are written into the next instance
            // and will be ignored or overwritten. We could specialise this code or deinterleave
            // the streams for better performance.
            instances[attr_index[0]] = x;
            instances[attr_index[1]] = y;
            instances[attr_index[2]] = z;
            instances += num_varys;
         }
      }
   }

   khrn_fmem_end_data(fmem, instances);

   return num_instances;
}

static size_t const cl_begin_prims_size = V3D_CL_COMPRESSED_PRIM_LIST_CURRENT_IID_SIZE;
static inline void cl_begin_prims(uint8_t** prim_cl)
{
   v3d_cl_compressed_prim_list_current_iid(prim_cl);
}

static size_t const cl_end_prims_size = sizeof(uint8_t);
static inline void cl_end_prims(uint8_t** prim_cl)
{
   // Escape Code (1 byte)
   v3d_cl_add_8(prim_cl, 227u);   // 7:0	=227
}

static size_t const cl_instance_id_size = sizeof(uint8_t) + sizeof(uint32_t);
static inline void cl_instance_id(uint8_t** prim_cl, unsigned i)
{
   // Set Instance Id (5 bytes, 32-bit instance Id)
   v3d_cl_add_8(prim_cl, 35u);    // 7:0	=35
   v3d_cl_add_32(prim_cl, i);     // 39:8	Instance_Id
}

static size_t const cl_first_indices_size = sizeof(uint32_t);
static inline void cl_first_indices(uint8_t** prim_cl)
{
   // Coding 3 (4 bytes, 1 absolute 24-bit index, 1 relative index)
   uint32_t const coding =
         11u         // 3:0	=11 or 15 (Reversed Flag=bit[2])
      |  (1u << 4)   // 7:4	2's complement difference between new line index (1) and new line index (0)
      |  (0 << 8);   // 31:8	Absolute new line index (0)
   v3d_cl_add_32(prim_cl, coding);
}

static size_t const cl_next_indices_size = sizeof(uint16_t);
static inline void cl_next_indices(uint8_t** prim_cl)
{
   // Coding 1 (2 bytes, 2 relative indices)
   uint16_t const coding =
         7u          // 3:0	=7
      | (2u << 4)    // 7:4	2's complement difference between new line index (0) and prev line index (0)
      | (2u << 8)    // 11:8	2's complement difference between new line index (1) and prev line index (1)
      | (0u << 12);  // 12	Reversed Flag;
   v3d_cl_add_16(prim_cl, coding);
}

static bool write_compressed_instanced_prim_list(
   khrn_fmem* fmem,
   compute_dispatch_config const* cfg,
   unsigned num_instances,
   V3D_CL_GL_SHADER_T const* shader)
{
   unsigned num_whole_rows = cfg->whole_rows_per_instance;
   unsigned odd_whole_rows = (cfg->max_groups_per_row * cfg->items_per_group) & 1;

   unsigned num_partial_groups = cfg->groups_per_instance - num_whole_rows * cfg->max_groups_per_row;
   unsigned num_partial_rows = (unsigned)(num_partial_groups != 0);
   unsigned odd_partial_rows = (num_partial_groups * cfg->items_per_group) & 1;

   for (unsigned i = 0; i != num_instances; )
   {
      // This is messy with the odd row handling, over-estimate for now.
      size_t const prim_cl_size = 0
         + (V3D_CL_GL_SHADER_SIZE
         + cl_begin_prims_size
         + cl_instance_id_size
         + cl_end_prims_size
         + cl_first_indices_size
         + cl_next_indices_size * (1 + (odd_whole_rows|odd_partial_rows))) * (num_whole_rows + num_partial_rows)
         + V3D_CL_CLIP_SIZE * 2 * odd_partial_rows;

      // Try and fit some instanced prims into the end of this CLE buffer or a new one.
      unsigned buffer_remaining = khrn_fmem_cle_buffer_remaining(fmem);
      unsigned max_instances_this_iteration = buffer_remaining / prim_cl_size;
      if (!max_instances_this_iteration)
         max_instances_this_iteration = KHRN_FMEM_USABLE_BUFFER_SIZE / prim_cl_size;
      assert(max_instances_this_iteration > 0);

      unsigned num_instances_this_iteration = gfx_umin(max_instances_this_iteration, num_instances - i);
      uint8_t* prim_cl = khrn_fmem_begin_cle(fmem, prim_cl_size * num_instances_this_iteration);
      if (!prim_cl)
         return false;

      uint32_t clip_whole = (cfg->max_groups_per_row*cfg->items_per_group + 1u) / 2u;
      uint32_t clip_partial = (num_partial_groups*cfg->items_per_group + 1u) / 2u;

      for (unsigned end_instance = i + num_instances_this_iteration; i != end_instance; ++i)
      {
         assert(i < num_instances);
         unsigned is_odd = odd_whole_rows;
         for (unsigned d = 0; d <= num_whole_rows; ++d)
         {
            if (d == num_whole_rows)
            {
               if (!num_partial_rows)
                  break;
               is_odd = odd_partial_rows;

               if (odd_partial_rows)
                  v3d_cl_clip(&prim_cl, 0, 0, clip_partial, TLB_HEIGHT);
            }

            // timh-todo: using lots of shader records to control batching
            // refactor this if using setmsf.
            v3d_cl_nv_shader_indirect(&prim_cl, shader);

            cl_begin_prims(&prim_cl);
            cl_instance_id(&prim_cl, i);
            if (d == 0)
            {
               cl_first_indices(&prim_cl);
            }
            else
            {
               cl_next_indices(&prim_cl);
            }
            if (is_odd)
               cl_next_indices(&prim_cl); // timh-todo: remove if using setmsf
            cl_end_prims(&prim_cl);
         }

         if (odd_partial_rows)
            v3d_cl_clip(&prim_cl, 0, 0, clip_whole, TLB_HEIGHT);
      }

      khrn_fmem_end_cle(fmem, prim_cl);
   }

   return true;
}

static v3d_addr_t build_shader_uniforms(
   glxx_compute_render_state* rs,
   GLXX_SERVER_STATE_T const* state,
   GLXX_LINK_RESULT_DATA_T const* link_data,
   compute_dispatch_config const* cfg,
   unsigned const num_work_groups[3])
{
   glxx_hw_compute_uniforms cu =
   {
      .num_work_groups = num_work_groups,
      .shared_ptr = 0,
      .quorum = (cfg->max_groups_per_row * cfg->items_per_group + 15) / 16
   };

   // Allocate shared memory block on demand.
   unsigned const shared_block_size = cfg->shared_block_size;
   if (shared_block_size != 0)
   {
      glxx_compute_shared* cs = &state->shared->compute;
      if (!cs->shared_buf)
      {
         unsigned num_cores = v3d_scheduler_get_hub_identity()->num_cores;
         cs->shared_buf = gmem_alloc(get_shared_buf_size() * num_cores, 256u, GMEM_USAGE_V3D, "glxx_compute shared");
         if (!cs->shared_buf)
            return false;
      }

      cu.shared_ptr = khrn_fmem_lock_and_sync(&rs->fmem, cs->shared_buf, 0, GMEM_SYNC_TMU_DATA_READ | GMEM_SYNC_TMU_DATA_WRITE);
      if (!cu.shared_ptr)
         return false;
   }

   GL20_HW_INDEXED_UNIFORM_T iu = { .valid = false };
   return glxx_hw_install_uniforms(&rs->base, state, link_data->fs.uniform_map, &iu, &cu);
}

static unsigned build_shader_record(
   V3D_CL_GL_SHADER_T* shader,
   khrn_fmem* fmem,
   GLXX_LINK_RESULT_DATA_T const* link_data,
   v3d_addr_t unifs_addr,
   compute_dispatch_config const* cfg,
   compute_dispatch_volume const* vol)
{
   unsigned num_attrs = 0;
   unsigned per_vertex_base = num_attrs; num_attrs += NUM_PER_VERTEX_ATTRIBUTES;
   unsigned per_index_base = num_attrs; num_attrs += link_data->num_varys;

   unsigned shader_record_size = V3D_SHADREC_GL_MAIN_PACKED_SIZE + V3D_SHADREC_GL_ATTR_PACKED_SIZE * num_attrs;
   uint32_t* shader_record_packed = khrn_fmem_data(fmem, shader_record_size, V3D_SHADREC_ALIGN);
   if (!shader_record_packed)
      return 0;

   uint8_t* shader_record_attrs = (uint8_t*)shader_record_packed + V3D_SHADREC_GL_MAIN_PACKED_SIZE;
   if (!build_per_vertex_data(shader_record_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE*per_vertex_base, fmem, cfg))
      return 0;

   unsigned num_instances = build_per_instance_data(
      shader_record_attrs + V3D_SHADREC_GL_ATTR_PACKED_SIZE*per_index_base,
      fmem,
      cfg,
      vol,
      link_data->vary_map,
      link_data->num_varys);
   if (!num_instances)
      return 0;

   if (!khrn_fmem_record_res_interlock(fmem, link_data->fs.res_i, false, ACTION_RENDER))
      return 0;

   v3d_addr_t fs_addr = khrn_fmem_lock_and_sync(fmem, link_data->fs.res_i->handle, 0, GMEM_SYNC_QPU_INSTR_READ);
   if (!fs_addr)
      return 0;

   V3D_SHADREC_GL_MAIN_T shader_record = { 0, };
   shader_record.no_ez = true;
   shader_record.scb_wait_on_first_thrsw = (link_data->flags & GLXX_SHADER_FLAGS_TLB_WAIT_FIRST_THRSW) != 0;
   shader_record.num_varys = link_data->num_varys;
#if V3D_HAS_VARY_DISABLE
   shader_record.disable_implicit_varys = true;
#endif
#if !V3D_VER_AT_LEAST(3,3,0,0)
   shader_record.num_varys = gfx_umax(shader_record.num_varys, 1); // workaround GFXH-1276
#endif
#if V3D_HAS_TNG
   shader_record.vs_output_size = (V3D_OUT_SEG_ARGS_T){
      .sectors = v3d_vs_output_size(false, link_data->num_varys),
      .min_extra_req = 0,
      .pack = V3D_CL_VPM_PACK_X16 };
   shader_record.cs_output_size = (V3D_OUT_SEG_ARGS_T){
      .sectors = 1,
      .min_extra_req = 0,
      .pack = V3D_CL_VPM_PACK_X16 };
   shader_record.cs_input_size.min_req = 1;
   shader_record.vs_input_size.min_req = 1;
#else
   shader_record.vs_output_size = v3d_vs_output_size(false, link_data->num_varys);
#endif
   shader_record.fs.threading = link_data->fs.threading;
   shader_record.fs.propagate_nans = true;
   shader_record.fs.addr = fs_addr;
   shader_record.fs.unifs_addr = unifs_addr;
   v3d_pack_shadrec_gl_main(shader_record_packed, &shader_record);

   shader->addr = khrn_fmem_hw_address(fmem, shader_record_packed);
   shader->num_attr_arrays = num_attrs;

   return num_instances;
}

static glxx_compute_render_state* create_compute_render_state(GLXX_SERVER_STATE_T* server_state)
{
   glxx_compute_render_state* rs = khrn_render_state_get_glxx_compute(
      khrn_render_state_new(KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE)
      );
   rs->server_state = server_state;

   khrn_render_state_disallow_flush((KHRN_RENDER_STATE_T*)rs);

   if (!khrn_fmem_init(&rs->fmem, (KHRN_RENDER_STATE_T*)rs))
      goto error;

   // Initial state setup for all compute jobs.
   size_t const cl_size = 0
#if V3D_HAS_NEW_TLB_CFG
      + V3D_CL_TILE_RENDERING_MODE_CFG_SIZE*3
#else
      + V3D_CL_TILE_RENDERING_MODE_CFG_SIZE*4
#endif
      + V3D_CL_DISABLE_Z_ONLY_SIZE
      + V3D_CL_FLUSH_VCD_CACHE_SIZE
      + V3D_CL_VCM_CACHE_SIZE_SIZE
      + V3D_CL_ZERO_ALL_CENTROID_FLAGS_SIZE
      + V3D_CL_ZERO_ALL_FLATSHADE_FLAGS_SIZE
      + V3D_CL_SAMPLE_COVERAGE_SIZE
      + V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE
      + V3D_CL_LINE_WIDTH_SIZE
      + V3D_CL_VIEWPORT_OFFSET_SIZE
      + V3D_CL_CLIPZ_SIZE
      + V3D_CL_COLOR_WMASKS_SIZE
#if V3D_HAS_BASEINSTANCE
      + V3D_CL_SET_INSTANCE_ID_SIZE
#endif
      ;
   uint8_t* cl = khrn_fmem_begin_cle(&rs->fmem, cl_size);
   if (!cl)
      goto error;

   // Tile Rendering Mode (Common Configuration).
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T cfg;
      memset(&cfg, 0, sizeof(cfg));
      cfg.type = V3D_RCFG_TYPE_COMMON;
      cfg.u.common.num_rts = 1;
      cfg.u.common.frame_width = TLB_WIDTH;
      cfg.u.common.frame_height = TLB_HEIGHT;
      cfg.u.common.max_bpp = TLB_UINT_BITS == 16 ? V3D_RT_BPP_64 : V3D_RT_BPP_32;
      cfg.u.common.ez_disable = true;
#if V3D_HAS_NEW_TLB_CFG
      cfg.u.common.internal_depth_type = V3D_DEPTH_TYPE_24;
#else
      cfg.u.common.disable_rt_store_mask = 0xff;
#endif
      v3d_cl_tile_rendering_mode_cfg_indirect(&cl, &cfg);
   }

   // Rendering Configuration (Render Target Config).
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T cfg;
      memset(&cfg, 0, sizeof(cfg));
      cfg.type = V3D_RCFG_TYPE_COLOR;
      v3d_rt_bpp_t internal_bpp = TLB_UINT_BITS == 16 ? V3D_RT_BPP_64 : V3D_RT_BPP_32;
      v3d_rt_type_t internal_type = TLB_UINT_BITS == 16 ? V3D_RT_TYPE_16UI : V3D_RT_TYPE_8UI;
#if V3D_HAS_NEW_TLB_CFG
      for (unsigned i = 0; i != V3D_MAX_RENDER_TARGETS; ++i)
      {
         cfg.u.color.rts[0].internal_bpp = internal_bpp;
         cfg.u.color.rts[0].internal_type = internal_type;
      }
#else
      cfg.u.color.internal_bpp = internal_bpp;
      cfg.u.color.internal_type = internal_type;
      cfg.u.color.decimate_mode = V3D_DECIMATE_SAMPLE0;
      cfg.u.color.output_format = TLB_UINT_BITS == 16 ? V3D_PIXEL_FORMAT_RGBA16UI : V3D_PIXEL_FORMAT_RGBA8UI;
      cfg.u.color.dither_mode = V3D_DITHER_OFF;
      cfg.u.color.memory_format = V3D_MEMORY_FORMAT_UIF_NO_XOR;
#endif
      v3d_cl_tile_rendering_mode_cfg_indirect(&cl, &cfg);
   }

#if !V3D_HAS_NEW_TLB_CFG
   // Rendering Configuration (Z/Stencil Config).
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T cfg;
      memset(&cfg, 0, sizeof(cfg));
      cfg.type = V3D_RCFG_TYPE_Z_STENCIL;
      cfg.u.z_stencil.internal_type = V3D_DEPTH_TYPE_24;
      cfg.u.z_stencil.decimate_mode = V3D_DECIMATE_SAMPLE0;
      cfg.u.z_stencil.output_format = V3D_DEPTH_FORMAT_24_STENCIL8;
      cfg.u.z_stencil.memory_format = V3D_MEMORY_FORMAT_RASTER;
      cfg.u.z_stencil.uif_height_in_ub = 0;
      cfg.u.z_stencil.addr = 0;
      v3d_cl_tile_rendering_mode_cfg_indirect(&cl, &cfg);
   }
#endif

   // Rendering Configuration (Z & Stencil Clear Values). Must be last.
   v3d_cl_tile_rendering_mode_cfg_zs_clear_values(&cl, 0, 0.0f);

   v3d_cl_disable_z_only(&cl);
   v3d_cl_flush_vcd_cache(&cl);
   v3d_cl_vcm_cache_size(&cl, 1, 1);
   v3d_cl_zero_all_centroid_flags(&cl);
   v3d_cl_zero_all_flatshade_flags(&cl);
   v3d_cl_sample_coverage(&cl, 1.0f);
   v3d_cl_occlusion_query_counter_enable(&cl, 0);
   v3d_cl_line_width(&cl, 2.0f);
   v3d_cl_viewport_offset(&cl, 0, 0);
   v3d_cl_clipz(&cl, 0.0f, 1.0f);
   v3d_cl_color_wmasks(&cl, -1);
#if V3D_HAS_BASEINSTANCE
   v3d_cl_set_instance_id(&cl, 0);
#endif

   khrn_fmem_end_cle_exact(&rs->fmem, cl);

   khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);

   /* Record security status */
   rs->fmem.br_info.secure = egl_context_gl_secure(rs->server_state->context);

   return rs;

error:
   khrn_render_state_delete((KHRN_RENDER_STATE_T*)rs);
   return NULL;
}

static bool dispatch_compute_volume(
   glxx_compute_render_state* rs,
   GLXX_SERVER_STATE_T const* state,
   GLXX_LINK_RESULT_DATA_T const* link_data,
   compute_dispatch_config const* cfg,
   compute_dispatch_volume const* vol,
   unsigned const num_work_groups[3])
{
   bool ok = false;
   khrn_render_state_disallow_flush((KHRN_RENDER_STATE_T*)rs);

   // timh-todo: Cache recently used TLB lookup tables?
   v3d_addr_t tlb_lookup_addr = build_tlb_lookup(&rs->fmem, cfg);
   if (!tlb_lookup_addr)
      goto end;

   v3d_addr_t unifs_addr = build_shader_uniforms(rs, state, link_data, cfg, num_work_groups);
   if (!unifs_addr)
      goto end;

   V3D_CL_GL_SHADER_T shader;
   unsigned num_instances = build_shader_record(&shader, &rs->fmem, link_data, unifs_addr, cfg, vol);
   if (!num_instances)
      goto end;

   size_t const head_cl_size = 0
      + V3D_CL_CFG_BITS_SIZE
      + V3D_CL_CLIP_SIZE
#if V3D_HAS_NEW_TLB_CFG
      + V3D_CL_TILE_COORDS_SIZE
      + V3D_CL_LOAD_SIZE
      + V3D_CL_END_LOADS_SIZE
#else
      + V3D_CL_LOAD_GENERAL_SIZE
      + V3D_CL_TILE_COORDS_SIZE
#endif
      + V3D_CL_PRIM_LIST_FORMAT_SIZE;
   uint8_t* cl = khrn_fmem_begin_cle(&rs->fmem, head_cl_size);
   if (!cl)
      goto end;

   v3d_cl_clip(&cl, 0, 0, (cfg->max_groups_per_row*cfg->items_per_group + 1u)/2u, TLB_HEIGHT);

   V3D_CL_CFG_BITS_T cfg_bits = {
      .front_prims = true,
      .rast_oversample = V3D_MS_1X,           // These are all 0, but prefer enum name.
      .cov_update = V3D_COV_UPDATE_NONZERO,
      .depth_test = V3D_COMPARE_FUNC_ALWAYS
   };
   v3d_cl_cfg_bits_indirect(&cl, &cfg_bits);

#if V3D_HAS_NEW_TLB_CFG
   v3d_cl_tile_coords(&cl, 0, 0);
   V3D_CL_LOAD_T load_lookup = {
      .buffer = V3D_LDST_BUF_COLOR0,
      .memory_format = V3D_MEMORY_FORMAT_UIF_NO_XOR,
      .flipy = false,
      .decimate = V3D_DECIMATE_ALL_SAMPLES,
      .pixel_format = TLB_UINT_BITS == 16 ? V3D_PIXEL_FORMAT_RGBA16UI : V3D_PIXEL_FORMAT_RGBA8UI,
      .stride = TLB_HEIGHT / (TLB_UINT_BITS == 16 ? 4 : 8), /* Height in UIF-blocks */
      .height = 0, /* Not used when !flipy */
      .addr = tlb_lookup_addr};
   v3d_cl_load_indirect(&cl, &load_lookup);
   v3d_cl_end_loads(&cl);
#else
   V3D_CL_LOAD_GENERAL_T load_lookup = {
      .buffer = V3D_LDST_BUF_COLOR0,
      .raw_mode = true,
      .memory_format = V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR,
      .uif_height_in_ub = TLB_HEIGHT / (TLB_UINT_BITS == 16 ? 4 : 8),
      .addr = tlb_lookup_addr
   };
   v3d_cl_load_general_indirect(&cl, &load_lookup);
   v3d_cl_tile_coords(&cl, 0, 0);
#endif
   v3d_cl_prim_list_format(&cl, 2, false, false);

   khrn_fmem_end_cle_exact(&rs->fmem, cl);

   if (!write_compressed_instanced_prim_list(&rs->fmem, cfg, num_instances, &shader))
      goto end;

   size_t const tail_cl_size = 0
#if V3D_HAS_NEW_TLB_CFG
      + V3D_CL_STORE_SIZE
      + V3D_CL_END_TILE_SIZE
#else
      + V3D_CL_STORE_SUBSAMPLE_EX_SIZE
#endif
      + V3D_CL_NOP_SIZE;
   cl = khrn_fmem_begin_cle(&rs->fmem, tail_cl_size);

#if V3D_HAS_NEW_TLB_CFG
   V3D_CL_STORE_T dummy_store = {.buffer = V3D_LDST_BUF_NONE};
   v3d_cl_store_indirect(&cl, &dummy_store);
   v3d_cl_end_tile(&cl);
#else
   V3D_CL_STORE_SUBSAMPLE_EX_T dummy_store = {
      .disable_depth_clear = true,
      .disable_stencil_clear = true,
      .disable_color_clear = true,
      .stencil_store = false,
      .depth_store = false,
      .disable_rt_store_mask = 0xff
   };
   v3d_cl_store_subsample_ex_indirect(&cl, &dummy_store);
#endif

   // Track where we need to patch end render.
   static_assrt(V3D_CL_NOP_SIZE == V3D_CL_END_RENDER_SIZE);
   rs->end_render = cl;
   v3d_cl_nop(&cl);
   khrn_fmem_end_cle_exact(&rs->fmem, cl);

   ok = true;
end:
   khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);
   return ok;
}

static inline bool should_flush(glxx_compute_render_state* rs)
{
   return khrn_fmem_get_common_persist(&rs->fmem)->pool.n_buffers >= COMPUTE_MAX_BLOCKS;
}

static bool begin_dispatch(GLXX_LINK_RESULT_DATA_T** link_data, GLXX_SERVER_STATE_T* state)
{
   *link_data = NULL;

   // Flush render-state if using too many blocks.
   glxx_compute_render_state* rs = state->compute_render_state;
   if (rs != NULL && should_flush(rs))
   {
      glxx_compute_render_state_flush(rs);
      rs = NULL;
   }

   // Create new render state.
   if (rs == NULL)
   {
      rs = create_compute_render_state(state);
      if (!rs)
         return false;
      state->compute_render_state = rs;
   }

   khrn_render_state_disallow_flush((KHRN_RENDER_STATE_T*)rs);

   bool ok = false;
   if (  !khrn_fmem_record_fence_to_signal(&rs->fmem, state->fences.fence)
      || !khrn_fmem_record_fence_to_depend_on(&rs->fmem, state->fences.fence_to_depend_on) )
      goto end;

   state->shaderkey_common.backend = 0
#if !V3D_HAS_VARY_DISABLE
      | GLXX_PRIM_LINE
#endif
      | (GLXX_FB_I32 | (V3D_HAS_GFXH1212_FIX ? 0 : GLXX_FB_ALPHA_16_WORKAROUND)) << GLXX_FB_GADGET_S;

   if (!glxx_compute_image_like_uniforms(state, &rs->base))
      goto end;

   *link_data = glxx_get_shaders(state);
   if (!*link_data)
   {
      log_warn("[%s] shader compilation failed for %p", VCOS_FUNCTION, state);
      goto end;
   }

   ok = true;
end:
   khrn_render_state_allow_flush((KHRN_RENDER_STATE_T*)rs);
   return ok;
}

static bool dispatch_compute_recursive(
   GLXX_SERVER_STATE_T* state,
   GLXX_LINK_RESULT_DATA_T* link_data,
   compute_dispatch_volume const* vol,
   unsigned const num_work_groups[3],
   unsigned depth)
{
   if (!link_data && !begin_dispatch(&link_data, state))
      return false;

   // Compute config should remain valid even if we rebuild the link data
   compute_dispatch_config cfg = { 0, }; // Pointless initialisation to keep gcc happy.
   GLSL_PROGRAM_T* program = gl20_program_common_get(state)->linked_glsl_program;
   if (!init_dispatch_config(&cfg, link_data->fs.threading, program->wg_size, program->shared_block_size, vol->size))
      return false;

   // Compute number of groups we can do with this config.
   unsigned remainders[3];
   compute_dispatch_volume centre_vol;
   for (unsigned i = 0; i != 3; ++i)
   {
      remainders[i] = vol->size[i] % cfg.instance_size[i];
      centre_vol.origin[i] = vol->origin[i];
      centre_vol.size[i] = vol->size[i] - remainders[i];
   }

   /*
   VCOS_ALERT("Dispatch %u: (%u x %u x %u) groups of (%u x %u x %u) items as %u instances of (%u * %u = %u) items",
      depth,
      num_work_groups[0], num_work_groups[1], num_work_groups[2],
      cfg.group_size[0], cfg.group_size[1], cfg.group_size[2],
      centre_vol.size[0] * centre_vol.size[1] * centre_vol.size[2] / (cfg.instance_size[0] * cfg.instance_size[1] * cfg.instance_size[2]),
      cfg.groups_per_instance,
      cfg.items_per_group,
      cfg.groups_per_instance * cfg.items_per_group
      );//*/

   for (; ;)
   {
      // Limit number of instances so that dispatch_compute_volume can fit the whole volume.
      // Ideally dispatch_compute_volume would split itself up and share control list items
      // for TLB and shader data, but hopefully this limit isn't reached often (or at all) and
      // we can get away with recursion here. 1024 instances should fit comfortably in a few
      // fmem blocks.
      for (; ; )
      {
         unsigned num_instances = compute_num_instances(&cfg, &centre_vol);
         if (num_instances <= 1024)
            break;

         for (unsigned i = 3; i-- != 0; )
         {
            if (centre_vol.size[i] > cfg.instance_size[i])
            {
               unsigned half = (centre_vol.size[i] / cfg.instance_size[i] / 2u) * cfg.instance_size[i];
               remainders[i] += centre_vol.size[i] - half;
               centre_vol.size[i] = half;
               break;
            }
         }
      }

      if (dispatch_compute_volume(state->compute_render_state, state, link_data, &cfg, &centre_vol, num_work_groups))
         break;

      // If we ran out of memory then attempt a flush.
      if (!glxx_compute_render_state_flush(state->compute_render_state))
         return false;

      // Begin dispatch creates a new compute render state.
      if (!begin_dispatch(&link_data, state))
         return false;
   }

   // Handle edge cases, one dimension at a time.
   compute_dispatch_volume edge_vol = *vol;
   for (unsigned i = 0; i != 3; ++i)
   {
      if (remainders[i] == 0)
         continue;

      edge_vol.origin[i] += edge_vol.size[i] - remainders[i];
      edge_vol.size[i] = remainders[i];

      if (!dispatch_compute_recursive(state, link_data, &edge_vol, num_work_groups, depth + 1))
         return false;

      edge_vol.origin[i] = vol->origin[i];
      edge_vol.size[i] = vol->size[i] - remainders[i];
   }
   return true;
}

static void post_dispatch(glxx_compute_render_state* rs)
{
   if (!rs)
      return;

   if (khrn_options.no_compute_batching || should_flush(rs))
      glxx_compute_render_state_flush(rs);
}

static bool check_state(
   GLXX_SERVER_STATE_T* state,
   GLuint num_groups_x,
   GLuint num_groups_y,
   GLuint num_groups_z,
   bool is_indirect)
{
   static_assrt(GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT <= UINT16_MAX);
   if (  num_groups_x > GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT
      || num_groups_y > GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT
      || num_groups_z > GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT)
   {
      if (!is_indirect)
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return false;
   }

   if ((uint64_t)(num_groups_x * num_groups_y) * (uint64_t)num_groups_z > UINT32_MAX)
   {
      if (!is_indirect)
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      return false;
   }

   if (state->current_program != NULL) {
      if (!gl20_validate_program(state, &state->current_program->common)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   } else if (state->pipelines.bound != NULL) {
      if (!glxx_pipeline_validate(state->pipelines.bound)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
      if (!glxx_pipeline_create_compute_common(state->pipelines.bound)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   } else {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   GLSL_PROGRAM_T const* linked_program = gl20_program_common_get(state)->linked_glsl_program;
   if (!glsl_program_has_stage(linked_program, SHADER_COMPUTE))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   // Give up (without error) if nothing to do.
   if (!num_groups_x || !num_groups_y || !num_groups_z)
      return false;

   unsigned const* wg_size = linked_program->wg_size;
   if (wg_size[0] == 0 || wg_size[1] == 0 || wg_size[2] == 0)
      return false;

   return true;
}

GL_APICALL void GL_APIENTRY glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T* state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (!check_state(state, num_groups_x, num_groups_y, num_groups_z, false))
      goto end;

   compute_dispatch_volume const vol = {
      .origin = { 0, 0, 0 },
      .size = { num_groups_x, num_groups_y, num_groups_z }
   };

   if (!dispatch_compute_recursive(state, NULL, &vol, vol.size, 0))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   post_dispatch(state->compute_render_state);

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDispatchComputeIndirect(GLintptr indirect)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T* state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (indirect < 0 || (indirect & 3))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLXX_BUFFER_T* buffer = state->bound_buffer[GLXX_BUFTGT_DISPATCH_INDIRECT].obj;
   if (buffer == NULL || buffer->size < indirect+sizeof(compute_dispatch_cmd))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   compute_dispatch_cmd const* cmd = glxx_buffer_map_range(buffer, indirect, sizeof(compute_dispatch_cmd), GL_MAP_READ_BIT);
   if (!cmd)
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   compute_dispatch_volume const vol = {
      .origin = { 0, 0, 0 },
      .size = { cmd->num_groups[0], cmd->num_groups[1], cmd->num_groups[2] }
   };

   glxx_buffer_unmap_range(buffer, indirect, sizeof(compute_dispatch_cmd), GL_MAP_READ_BIT);

   if (!check_state(state, vol.size[0], vol.size[1], vol.size[2], true))
      goto end;

   if (!dispatch_compute_recursive(state, NULL, &vol, vol.size, 0))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   post_dispatch(state->compute_render_state);

end:
   glxx_unlock_server_state();
}

#endif

void glxx_compute_shared_init(glxx_compute_shared* cs)
{
   memset(cs, 0, sizeof(*cs));
}

void glxx_compute_shared_term(glxx_compute_shared* cs)
{
   gmem_free(cs->shared_buf);
}

bool glxx_compute_render_state_flush(glxx_compute_render_state* rs)
{
   khrn_render_state_begin_flush((KHRN_RENDER_STATE_T*)rs);

   khrn_fmem* fmem = &rs->fmem;
   khrn_fmem_end_clist(fmem);

   bool do_flush = rs->end_render != NULL;
   if (do_flush)
   {
      // Patch end render over trailing nop.
      uint8_t* cl_end = rs->end_render;
      v3d_cl_end_render(&cl_end);

      fmem->br_info.num_renders = 1;
      fmem->br_info.render_begins[0] = khrn_fmem_hw_address(fmem, fmem->cle_first);
      fmem->br_info.render_ends[0] = khrn_fmem_hw_address(fmem, cl_end);
      khrn_fmem_flush(fmem);
   }
   else
   {
      khrn_fmem_discard(fmem);
   }

   assert(rs == rs->server_state->compute_render_state);
   rs->server_state->compute_render_state = NULL;

   khrn_render_state_delete((KHRN_RENDER_STATE_T*)rs);
   return do_flush;
}
