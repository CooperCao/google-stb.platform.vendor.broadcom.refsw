/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "compute.h"
#include "libs/core/v3d/v3d_limits.h"
#if !V3D_USE_CSD
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_gmp.h"
#include "libs/core/v3d/v3d_shadrec.h"
#include "libs/core/v3d/v3d_vpm_alloc.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "libs/core/gfx_buffer/gfx_buffer_slow_conv.h"
#include "libs/util/common.h"
#include "libs/util/log/log.h"
#include "libs/util/intrusive/intrusive_list.h"
#include "libs/util/intrusive/intrusive_slist.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/khrn/glsl/glsl_backend_cfg.h"
#include "libs/platform/gmem.h"
#include <new>
#include <mutex>
#endif

#if V3D_USE_CSD
static constexpr unsigned MAX_SGS = V3D_MAX_CSD_SGS;
#else
static constexpr unsigned MAX_SGS = V3D_MAX_TLB_HEIGHT_PX / 4u;
#endif

static uint32_t choose_max_wgs_per_sg(const compute_params* p)
{
   // If using barriers then restrict super-group size to allow ideally at least two in flight.
   uint32_t max_sg_size = V3D_USE_CSD ? (V3D_VPAR * p->wg_size) : (V3D_MAX_TLB_WIDTH_PX * 2u);
   if (p->has_barrier)
   {
      uint32_t num_qpu_thread_lanes = p->num_qpus * p->num_threads * V3D_VPAR;
      max_sg_size = gfx_umin(max_sg_size, num_qpu_thread_lanes / 2);
   }
   uint32_t max_wgs_per_sg = gfx_umax(max_sg_size / p->wg_size, 1u);

   // Don't grow super-groups beyond half available shared memory size.
   if (p->shared_block_size > 0)
   {
      assert(p->shared_mem_per_core >= p->shared_block_size);
      max_wgs_per_sg = gfx_umin(max_wgs_per_sg, gfx_umax(1u, p->shared_mem_per_core / (2 * p->shared_block_size)));
   }
   assert(max_wgs_per_sg > 0);
   return max_wgs_per_sg;
}

static uint32_t choose_wgs_per_sg(uint32_t max_wgs_per_sg, uint32_t wg_size)
{
   // Calculate super-group packing to maximise occupancy.
   float best_occupancy = 0.0f;
   uint32_t best_wgs_per_sg = 1;
   for (uint32_t wgs_per_sg = 1; wgs_per_sg <= max_wgs_per_sg; ++wgs_per_sg)
   {
      uint32_t items_per_sg = wgs_per_sg * wg_size;
      if (items_per_sg % V3D_VPAR == 0)
      {
         best_wgs_per_sg = wgs_per_sg;
         break;
      }

      float occupancy = (float)items_per_sg / (float)gfx_uround_up_p2(items_per_sg, V3D_VPAR);
      if (occupancy > best_occupancy)
      {
         best_occupancy = occupancy;
         best_wgs_per_sg = wgs_per_sg;
      }
   }
   return best_wgs_per_sg;
}

compute_sg_config compute_choose_sg_config(const compute_params* p)
{
   uint32_t max_wgs_per_sg = choose_max_wgs_per_sg(p);
   uint32_t wgs_per_sg = choose_wgs_per_sg(max_wgs_per_sg, p->wg_size);

   // Compute the maximum number of work-groups from HW constraints and shared-memory constraints.
   uint32_t max_wgs = MAX_SGS * wgs_per_sg;
   uint32_t max_wgs_in_mem = ~0u;
   if (p->shared_block_size > 0)
      max_wgs_in_mem = p->shared_mem_per_core / p->shared_block_size;
   max_wgs = gfx_umin(max_wgs, max_wgs_in_mem);

   // Try to increase the super-group size if we wouldn't saturate all the QPU threads due to the limit of 16 super-groups.
   if (!V3D_USE_CSD)
   {
      uint32_t max_qpu_threads = p->num_qpus * p->num_threads * V3D_VPAR;

      while (max_wgs * p->wg_size < max_qpu_threads
          && max_wgs < max_wgs_in_mem
          && wgs_per_sg < max_wgs_per_sg)
      {
         wgs_per_sg = gfx_umin(wgs_per_sg*2, max_wgs_per_sg);
         max_wgs = gfx_umin(MAX_SGS * wgs_per_sg, max_wgs_in_mem);
      }
   }

   compute_sg_config cfg{};
   cfg.wgs_per_sg = gfx_bits(wgs_per_sg, sizeof(cfg.wgs_per_sg)*8);
   cfg.max_wgs = gfx_bits(max_wgs, sizeof(cfg.max_wgs)*8);
   return cfg;
}

bool compute_allow_concurrent_jobs(const compute_params* p, uint32_t wgs_per_sg)
{
   // GFXH-1193: Shaders from other jobs can lock out the reserved QPU, so ensure
   // progress can be made without it. The QPS implements "fair" scheduling, and
   // since it doesn't schedule a whole compute super-group at once, it will
   // prioritise running other shaders, potentially locking out QPUs with a
   // partial super-group that are waiting for a barrier.

   // Fine if there are no barriers.
   if (!p->has_barrier)
      return true;

   // Use at most a quarter of the available QPU thread slots.
   uint32_t num_qpu_slots_per_sg = gfx_udiv_round_up_p2(p->wg_size * wgs_per_sg, V3D_VPAR);
   uint32_t num_qpu_slots = p->num_qpus * p->num_threads;
   assert(num_qpu_slots_per_sg <= num_qpu_slots);
   return (4 * num_qpu_slots_per_sg) <= num_qpu_slots;
}

#if !V3D_USE_CSD

LOG_DEFAULT_CAT("compute");

static constexpr unsigned NUM_BLOCKS_PER_CHUNK = 16;
static constexpr unsigned MEM_BLOCK_SIZE = 64*1024;
static constexpr unsigned MEM_CHUNK_SIZE = MEM_BLOCK_SIZE * NUM_BLOCKS_PER_CHUNK;
static_assrt((MEM_BLOCK_SIZE % V3D_MAX_ALIGN) == 0);

static constexpr unsigned MAX_USABLE_CL_BLOCK_SIZE = MEM_BLOCK_SIZE - V3D_CL_BRANCH_SIZE - V3D_MAX_CLE_READAHEAD;

static_assrt(V3D_MAX_TLB_WIDTH_PX >= 64);
static_assrt(V3D_MAX_TLB_HEIGHT_PX >= 64);
static constexpr unsigned TLB_WIDTH = 64;
static constexpr unsigned TLB_HEIGHT = 64 / sizeof(uint16_t);
static_assrt(MAX_SGS == TLB_HEIGHT/2);
static_assrt(sizeof(unsigned) >= 4);

#if V3D_VER_AT_LEAST(4,1,34,0)
static constexpr unsigned ROW_ROUND_UP = 2;
#else
static constexpr unsigned ROW_ROUND_UP = V3D_VPAR;
#endif

//=============================================================================
// Static compute runtime instance.
//=============================================================================

struct mem_block_tracker : intrusive_slist<mem_block_tracker>::hook
{
   struct mem_chunk* chunk;
};

struct mem_chunk : intrusive_list<mem_chunk>::hook
{
   gmem_handle_t handle = nullptr;
   mem_block_tracker blocks[NUM_BLOCKS_PER_CHUNK];
   intrusive_slist<mem_block_tracker> free_blocks{};
   unsigned num_used_blocks = 0;

   mem_chunk()
   {
      for (unsigned i = 0; i != NUM_BLOCKS_PER_CHUNK; ++i)
      {
         free_blocks.push_back(blocks[i]);
         blocks[i].chunk = this;
      }

      handle = gmem_alloc_and_map(MEM_CHUNK_SIZE, V3D_MAX_ALIGN, GMEM_USAGE_V3D_RW, "compute");
   }

   ~mem_chunk()
   {
      assert(num_used_blocks == 0);
      assert(free_blocks.size() == NUM_BLOCKS_PER_CHUNK);
      free_blocks.clear();
      gmem_free(handle);
   }
};

struct compute_runtime
{
   // Protected by the mutex.
   std::mutex mutex;
   unsigned refs;
   intrusive_list<mem_chunk> chunks;

   // We can't be sure that compute_term was called before unloading the driver,
   // so prevent static destruction asserting in this case.
#ifndef NDEBUG
   ~compute_runtime() { chunks.clear(); }
#endif

} runtime;

//=============================================================================
// Job memory allocator.
//=============================================================================

struct mem_block
{
   v3d_addr_t addr;
   uint8_t* ptr;
   uint8_t* cur;
   uint8_t* end;
};

struct compute_job_mem
{
   mem_block cl{};
   mem_block data{};

   intrusive_slist<mem_block_tracker> blocks;
};

static bool alloc_block(compute_job_mem& job, mem_block* block, bool for_cl)
{
   // Linear search to find chunk with free blocks, don't expect that many chunks.
   mem_chunk* chunk = nullptr;
   for (auto& c: runtime.chunks)
   {
      if (!c.free_blocks.empty())
      {
         chunk = &c;
         break;
      }
   }
   if (!chunk)
   {

      chunk = new (std::nothrow) mem_chunk();
      if (!chunk || !chunk->handle)
      {
         delete chunk;
         return false;
      }
      runtime.chunks.push_back(*chunk);
   }

   // Record use of this chunk/block in the job.
   mem_block_tracker& tracker = chunk->free_blocks.pop_front();
   job.blocks.push_back(tracker);
   chunk->num_used_blocks += 1;
   assert(chunk->num_used_blocks + chunk->free_blocks.size() == NUM_BLOCKS_PER_CHUNK);

   // Adjust end of block based on usage and if it is the last in the chunk.
   unsigned block_index = &tracker - chunk->blocks;
   unsigned block_size = MEM_BLOCK_SIZE;
   if (for_cl)
   {
      block_size -= V3D_CL_BRANCH_SIZE;
      if (block_index == NUM_BLOCKS_PER_CHUNK-1)
         block_size -= V3D_MAX_CLE_READAHEAD;
   }

   // Fill out mem_block struct
   unsigned block_offset = MEM_BLOCK_SIZE * block_index;
   block->addr = gmem_get_addr(chunk->handle) + block_offset;
   block->ptr = (uint8_t*)gmem_get_ptr(chunk->handle) + block_offset;
   block->end = block->ptr + block_size;
   block->cur = block->ptr;
   return true;
}

static void free_blocks(compute_job_mem& job)
{
   while (!job.blocks.empty())
   {
      // Return the block to it's owner.
      mem_block_tracker& block = job.blocks.pop_front();
      mem_chunk& chunk = *block.chunk;
      chunk.free_blocks.push_front(block);
      assert(chunk.num_used_blocks > 0);
      chunk.num_used_blocks -= 1;
      assert(chunk.num_used_blocks + chunk.free_blocks.size() == NUM_BLOCKS_PER_CHUNK);

      // Try to free any additional chunks past the first.
      if (&chunk != &runtime.chunks.front() && !chunk.num_used_blocks)
         runtime.chunks.erase_and_dispose(&chunk, [](mem_chunk* c) { delete c; } );
   }
}

static inline v3d_addr_t get_cl_start_addr(compute_job_mem& mem)
{
   if (!mem.cl.addr && !alloc_block(mem, &mem.cl, true))
      return 0;
   return mem.cl.addr + (mem.cl.cur - mem.cl.ptr);
}

static v3d_size_t remaining_cl_block_size(compute_job_mem& mem, v3d_size_t min_size)
{
   unsigned remaining = mem.cl.end - mem.cl.cur;
   return remaining >= min_size ? remaining : MAX_USABLE_CL_BLOCK_SIZE;
}

static uint8_t* begin_write_cl(compute_job_mem& mem, v3d_size_t max_size)
{
   if (max_size > v3d_size_t(mem.cl.end - mem.cl.cur))
   {
      uint8_t* old_cl = mem.cl.cur;
      if (!alloc_block(mem, &mem.cl, true))
         return nullptr;
      v3d_cl_branch(&old_cl, mem.cl.addr);
   }

   assert(mem.cl.cur + max_size <= mem.cl.end);
   return mem.cl.cur;
}

static void end_write_cl(compute_job_mem& mem, uint8_t* cur)
{
   assert(cur >= mem.cl.cur && cur <= mem.cl.end);
   mem.cl.cur = cur;
}

static void* data_alloc(compute_job_mem& mem, v3d_addr_t* addr, v3d_size_t size, v3d_size_t align)
{
   assert(align <= V3D_MAX_ALIGN);
   uint8_t* cur = (uint8_t*)gfx_align_up(mem.data.cur, align);
   if (size > v3d_size_t(mem.data.end - cur))
   {
      if (!alloc_block(mem, &mem.data, false))
         return nullptr;
      cur = mem.data.cur;
      assert(gfx_aligned(cur, align));
      assert(size <= v3d_size_t(mem.data.end - cur));
   }
   mem.data.cur = cur + size;
   *addr = mem.data.addr + (cur - mem.data.ptr);
   return cur;
}

//=============================================================================
// Instance config.
//=============================================================================

struct instance_config
{
   unsigned num_wgs;
   unsigned num_whole_rows;
   unsigned num_rows;
   uint16_t size[3]; // 3D num wgs in instance.
};

static instance_config get_instance_config(
   compute_program const& program,
   unsigned const extent_size[3])
{
   // Ensure program configuration meets compute runtime limitations.
   assert(program.items_per_wg * (uint32_t)program.wgs_per_sg <= TLB_WIDTH*2u);
   assert(program.max_wgs <= (uint32_t)program.wgs_per_sg * TLB_HEIGHT/2u);

   // Need to try and factor the 3D array of work-groups into 2D instances covering the TLB.
   unsigned dims[3];
   unsigned num_dims = 0;
   for (unsigned i = 0; i != 3; ++i)
   {
      if (extent_size[i] > 1)
         dims[num_dims++] = i;
   }

   unsigned max_wgs_per_instance = program.max_wgs;
   unsigned wgs_per_instance = 1;
   unsigned instance_size[3] = { 1, 1, 1 };

   // Expand each dimension in turn until we can't.
   for (unsigned i = 0; num_dims != 0; )
   {
      // Wrap around.
      if (i == num_dims)
         i = 0;

      unsigned dim = dims[i];

      // Check we can grow this dimension within the dispatch volume.
      unsigned new_instance_size = gfx_umin(instance_size[dim] * 2, extent_size[dim]);
      if (new_instance_size > instance_size[dim])
      {
         // Check we can fit the additional work-groups into this instance.
         assert((wgs_per_instance % instance_size[dim]) == 0);
         unsigned new_wgs_per_instance = (wgs_per_instance / instance_size[dim]) * new_instance_size;
         if (new_wgs_per_instance <= max_wgs_per_instance)
         {
            // OK to expand this dimension.
            instance_size[dim] = new_instance_size;
            wgs_per_instance = new_wgs_per_instance;
            i += 1;
            continue;
         }
      }

      // Remove this axis from candidates.
      for (unsigned j = i + 1; j != num_dims; ++j)
      {
         dims[j-1] = dims[j];
      }
      num_dims -= 1;
   }

   instance_config cfg{};
   assert(wgs_per_instance == instance_size[0] * instance_size[1] * instance_size[2]);
   cfg.size[0] = instance_size[0];
   cfg.size[1] = instance_size[1];
   cfg.size[2] = instance_size[2];
   cfg.num_wgs = wgs_per_instance;
   cfg.num_whole_rows = wgs_per_instance / program.wgs_per_sg;
   cfg.num_rows = cfg.num_whole_rows + (wgs_per_instance % program.wgs_per_sg ? 1 : 0);
   return cfg;
}

//=============================================================================
// Dispatch extents and splitting.
//=============================================================================

struct dispatch_extent
{
   unsigned origin[3];  // In work groups.
   unsigned size[3];    // In work groups.
};

struct split_extents
{
   dispatch_extent extent;
   dispatch_extent remainders[3];
};

static inline split_extents split_extent(
   instance_config const& cfg,
   dispatch_extent const& extent)
{
   split_extents split;

   // Compute number of wgs we can do with this config.
   unsigned remainders[3];
   for (unsigned i = 0; i != 3; ++i)
   {
      remainders[i] = extent.size[i] % cfg.size[i];
      split.extent.origin[i] = extent.origin[i];
      split.extent.size[i] = extent.size[i] - remainders[i];
   }

   // Handle edge cases, one dimension at a time.
   dispatch_extent edge = extent;
   for (unsigned i = 0; i != 3; ++i)
   {
      split.remainders[i] = edge;
      split.remainders[i].origin[i] += edge.size[i] - remainders[i];
      split.remainders[i].size[i] = remainders[i];

      edge.size[i] -= remainders[i];
   }

   return split;
}

//=============================================================================
// Control list and data building.
//=============================================================================

static inline unsigned compose_index(uint16_t const id[3], uint16_t const size[3])
{
   assert(id[0] < size[0]);
   assert(id[1] < size[1]);
   assert(id[2] < size[2]);
   return (id[2]*size[1] + id[1])*size[0] + id[0];
}

static inline void decompose_index(uint16_t id[3], unsigned index, uint16_t const size[3])
{
   id[0] = index % size[0];
   id[1] = (index / size[0]) % size[1];
   id[2] = (index / size[0]) / size[1];
   assert(compose_index(id, size) == index);
}

static v3d_addr_t build_tlb_lookup(
   compute_job_mem& mem,
   compute_program const& program,
   instance_config const& cfg)
{
   v3d_addr_t lookup_addr;
   uint16_t* lookup = (uint16_t*)data_alloc(mem, &lookup_addr, TLB_WIDTH * TLB_HEIGHT * sizeof(uint16_t) * 4, 256u);
   if (!lookup)
      return 0;

   GFX_BUFFER_DESC_PLANE_T plane{};
   plane.lfmt = GFX_LFMT_R16G16B16A16_UINT_2D_UIF;
   plane.offset = 0;
   plane.pitch = TLB_HEIGHT * sizeof(uint16_t) * 4;

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, plane.lfmt);

   unsigned items_per_row = program.wgs_per_sg * program.items_per_wg;
   unsigned lanes_per_row = gfx_uround_up_p2(items_per_row, ROW_ROUND_UP);

   uint32_t wg_in_mem_shift = gfx_log2(gfx_next_power_of_2(program.items_per_wg));

   for (unsigned row_index = 0; row_index != cfg.num_rows; ++row_index)
   {
      // Arrange so that the (last) partial row is at 0 for easy detection in the shader.
      unsigned y0 = (cfg.num_rows - row_index - 1) * 2;

      for (unsigned item_index = 0; item_index != lanes_per_row; ++item_index)
      {
         uint32_t gindex_lindex = UINT16_MAX;
         uint16_t wg_id[3] = { 0, };

         // If lane is valid (not padding).
         unsigned wg_index = row_index*program.wgs_per_sg + item_index/ program.items_per_wg;
         if (item_index < items_per_row && wg_index < cfg.num_wgs)
         {
            uint32_t local_index = item_index % program.items_per_wg;

            gindex_lindex = local_index;
            if (program.has_shared)
               gindex_lindex |= wg_index << wg_in_mem_shift;
            assert(gindex_lindex <= UINT16_MAX /2); // top bit means invalid

            decompose_index(wg_id, wg_index, cfg.size);
         }

         unsigned x = item_index / 2;
         unsigned y = y0 + item_index % 2;
         unsigned offset = gfx_buffer_block_offset(&plane, &bd, x, y, 0, TLB_HEIGHT) / sizeof(uint16_t);
         lookup[offset+0] = (uint16_t)gindex_lindex;
         lookup[offset+1] = (uint16_t)wg_id[0];
         lookup[offset+2] = (uint16_t)wg_id[1];
         lookup[offset+3] = (uint16_t)wg_id[2];
      }
   }
   return lookup_addr;
}

static unsigned const NUM_PER_VERTEX_ATTRIBUTES = 2;

static v3d_addr_t build_per_vertex_data(
   compute_job_mem& mem,
   compute_program const& program,
   instance_config const& cfg)
{
   unsigned wgs_per_partial_row = cfg.num_wgs - cfg.num_whole_rows*program.wgs_per_sg;
   unsigned items_per_partial_row = wgs_per_partial_row * program.items_per_wg;
   unsigned items_per_whole_row = program.wgs_per_sg * program.items_per_wg;

   unsigned sizeof_attr_data0 = sizeof(uint16_t) * 2 * 2 * cfg.num_rows;
   unsigned sizeof_attr_data1 = sizeof(uint32_t) * (2 + program.has_barrier);

   v3d_addr_t attr_addr;
   uint8_t* const attr_data = (uint8_t*)data_alloc(mem, &attr_addr, sizeof_attr_data0 + sizeof_attr_data1, 4);
   if (!attr_data)
      return 0;

   // Write per-vertex Xs,Ys data.
   {
      uint16_t* attr_data0 = (uint16_t*)attr_data;

      // Offset line along x by half a pixel (for diamond exit rasterisation).
      uint32_t x0 = 128u + 0u;
      uint32_t x1 = 128u + gfx_uround_up(items_per_whole_row, ROW_ROUND_UP) * (256u / 2u);

      for (unsigned d = 0; d != cfg.num_rows; ++d)
      {
         if (d == cfg.num_whole_rows)
            x1 = 128u + gfx_uround_up(items_per_partial_row, ROW_ROUND_UP) * (256u / 2u);

         // Arrange so that the (last) partial row is at 0 for easy detection in the shader.
         uint32_t const y = ((cfg.num_rows - d)*2u - 1u) * 256u;
         attr_data0[0] = x0;
         attr_data0[1] = y;
         attr_data0[2] = x1;
         attr_data0[3] = y;
         attr_data0 += 4;
      }
   }

   // Write constant Zs,1/Wc data, quorum.
   {
      uint32_t* attr_data1 = (uint32_t*)(attr_data + sizeof_attr_data0);
      attr_data1[0] = 0;
      attr_data1[1] = 0x3f800000;
      if (program.has_barrier)
      {
         unsigned quorum = gfx_udiv_round_up_p2(items_per_whole_row, V3D_VPAR);
         unsigned quorum0 = gfx_udiv_round_up_p2(items_per_partial_row, V3D_VPAR);
         if (!quorum0)
            quorum0 = quorum;

         attr_data1[2] = (quorum << 16) | quorum0;
      }
   }

   return attr_addr;
}

static inline unsigned get_num_instances(instance_config const& cfg, dispatch_extent const& extent)
{
   unsigned extent_num_wgs = extent.size[0] * extent.size[1] * extent.size[2];
   assert((extent_num_wgs % cfg.num_wgs) == 0);
   return extent_num_wgs / cfg.num_wgs;
}

static inline unsigned get_per_instance_data_size(unsigned num_instances, unsigned num_varys)
{
   return sizeof(uint16_t) * (num_varys * num_instances + 1u/*overspill*/);
}

static unsigned build_per_instance_data(
   v3d_addr_t* instances_addr,
   compute_job_mem& mem,
   compute_program const& program,
   instance_config const& cfg,
   dispatch_extent const& extent,
   uint8_t const* vary_map,
   unsigned num_varys)
{
   unsigned num_instances = get_num_instances(cfg, extent);
   unsigned instances_size = get_per_instance_data_size(num_instances, num_varys);

   uint16_t* instances = (uint16_t*)data_alloc(mem, instances_addr, instances_size, V3D_ATTR_ALIGN);
   if (!instances)
      return 0;
   uint16_t* instance_end = (uint16_t*)((char*)instances + instances_size);

   unsigned attr_index[3] = { num_varys, num_varys, num_varys };
   for (unsigned v = 0; v != num_varys; ++v)
   {
      attr_index[vary_map[v]] = v;
   }

   unsigned const step_x = cfg.size[0];
   unsigned const step_y = cfg.size[1];
   unsigned const step_z = cfg.size[2];
   unsigned const begin_x = extent.origin[0];
   unsigned const begin_y = extent.origin[1];
   unsigned const begin_z = extent.origin[2];
   unsigned const end_x = begin_x + extent.size[0];
   unsigned const end_y = begin_y + extent.size[1];
   unsigned const end_z = begin_z + extent.size[2];
   assert(end_x <= (1 << 16));
   assert(end_y <= (1 << 16));
   assert(end_z <= (1 << 16));

   // Fill out per instance elements (work-group ID offset in 3D).
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

   // check for overflow.
   assert((instances + 1) == instance_end);

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
   compute_job_mem& mem,
   instance_config const& cfg,
   unsigned num_instances)
{
   size_t const fixed_cl_size = cl_begin_prims_size + cl_end_prims_size;

   size_t const instance_cl_size = 0
      + cl_instance_id_size
      + cl_first_indices_size
      + cl_next_indices_size * (cfg.num_rows - 1);

   for (unsigned i = 0; i != num_instances; )
   {
      // Try and fit some instanced prims into the end of this CLE buffer or a new one.
      unsigned buffer_remaining = remaining_cl_block_size(mem, fixed_cl_size + instance_cl_size);
      unsigned max_instances_this_iteration = (buffer_remaining - fixed_cl_size) / instance_cl_size;
      assert(max_instances_this_iteration > 0);

      unsigned num_instances_this_iteration = gfx_umin(max_instances_this_iteration, num_instances - i);
      v3d_size_t prim_cl_size = fixed_cl_size + instance_cl_size * num_instances_this_iteration;
      uint8_t* prim_cl = begin_write_cl(mem, prim_cl_size);
      if (!prim_cl)
         return false;
      uint8_t* prim_cl_end = prim_cl + prim_cl_size;

      cl_begin_prims(&prim_cl);
      for (unsigned end_instance = i + num_instances_this_iteration; i != end_instance; ++i)
      {
         assert(i < num_instances);
         assert(cfg.num_rows != 0);

         cl_instance_id(&prim_cl, i);
         cl_first_indices(&prim_cl);
         for (unsigned d = 1; d != cfg.num_rows; ++d)
            cl_next_indices(&prim_cl);
      }
      cl_end_prims(&prim_cl);

      assert(prim_cl == prim_cl_end);
      end_write_cl(mem, prim_cl);
   }

   return true;
}

static void write_shader_record_attrs(
   uint32_t* packed_attrs,
   compute_program const& program,
   v3d_addr_t vertices_addr,
   unsigned num_vertices,
   v3d_addr_t instances_addr,
   unsigned num_instances)
{
   static_assrt((V3D_SHADREC_GL_ATTR_PACKED_SIZE % sizeof(uint32_t)) == 0);
   unsigned const attr_words = V3D_SHADREC_GL_ATTR_PACKED_SIZE / sizeof(uint32_t);

   // Pack per-vertex Xs,Ys attribute record.
   V3D_SHADREC_GL_ATTR_T attr0 = { 0, };
   attr0.addr = vertices_addr;
   attr0.size = 2;
   attr0.type = V3D_ATTR_TYPE_SHORT;
   attr0.read_as_int = true;
   attr0.vs_num_reads = 2;
   attr0.stride = sizeof(uint16_t) * 2;
#if V3D_VER_AT_LEAST(4,1,34,0)
   attr0.max_index = num_vertices - 1;
#endif
   v3d_pack_shadrec_gl_attr(packed_attrs + attr_words*0, &attr0);

   // Pack constant Zs,1/Wc attribute record.
   V3D_SHADREC_GL_ATTR_T attr1 = { 0, };
   attr1.addr = vertices_addr + attr0.stride*num_vertices;
   attr1.size = 2 + program.has_barrier;
   attr1.type = V3D_ATTR_TYPE_INT;
   attr1.read_as_int = true;
   attr1.vs_num_reads = attr1.size;
#if V3D_VER_AT_LEAST(4,1,34,0)
   attr1.max_index = 0;
#endif
   v3d_pack_shadrec_gl_attr(packed_attrs + attr_words*1, &attr1);

   // Pack per-instance attribute.
   if (!program.num_varys)
      return;
   V3D_SHADREC_GL_ATTR_T attr2 = { 0, };
   attr2.addr = instances_addr;
   attr2.size = program.num_varys;
   attr2.type = V3D_ATTR_TYPE_SHORT;
   attr2.read_as_int = true;
   attr2.vs_num_reads = program.num_varys;
   attr2.divisor = 1;
   attr2.stride = sizeof(uint16_t) * program.num_varys;
#if V3D_VER_AT_LEAST(4,1,34,0)
   attr2.max_index = num_instances - 1;
#endif
   v3d_pack_shadrec_gl_attr(packed_attrs + attr_words*2, &attr2);
}

static bool write_shader_record(
   compute_job_mem& mem,
   compute_program const& program,
   v3d_addr_t unifs_addr,
   v3d_addr_t vertices_addr,
   unsigned num_vertices,
   v3d_addr_t instances_addr,
   unsigned num_instances)
{
   unsigned num_attrs = 0;
   num_attrs += NUM_PER_VERTEX_ATTRIBUTES;
   num_attrs += (program.num_varys ? 1 : 0);

   v3d_addr_t shader_record_addr;
   unsigned shader_record_size = V3D_SHADREC_GL_MAIN_PACKED_SIZE + V3D_SHADREC_GL_ATTR_PACKED_SIZE * num_attrs;
   uint32_t* shader_record_packed = (uint32_t*)data_alloc(mem, &shader_record_addr, shader_record_size, V3D_SHADREC_ALIGN);
   if (!shader_record_packed)
      return false;

   V3D_SHADREC_GL_MAIN_T shader_record{};
   shader_record.no_ez = true;
   shader_record.scb_wait_on_first_thrsw = program.scb_wait_on_first_thrsw;
   shader_record.num_varys = program.num_varys + program.has_barrier;
#if V3D_VER_AT_LEAST(4,1,34,0)
   shader_record.no_prim_pack = true;
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
   shader_record.disable_implicit_varys = true;
   shader_record.vs_output_size.sectors = v3d_vs_output_size(false, shader_record.num_varys);
   shader_record.vs_output_size.min_extra_req = 0 ;
   shader_record.cs_output_size.sectors = 1;
   shader_record.cs_output_size.min_extra_req = 0;
   shader_record.cs_input_size.min_req = 1;
   shader_record.vs_input_size.min_req = 1;
#else
   shader_record.vs_output_size = v3d_vs_output_size(false, shader_record.num_varys);
# if !V3D_VER_AT_LEAST(3,3,0,0)
   shader_record.num_varys = gfx_umax(shader_record.num_varys, 1); // workaround GFXH-1276
# endif
#endif
   shader_record.fs.threading = program.threading;
#if V3D_VER_AT_LEAST(4,1,34,0)
   shader_record.fs.single_seg  = false;
#endif
   shader_record.fs.propagate_nans = true;
   shader_record.fs.addr = program.code_addr;
   shader_record.fs.unifs_addr = unifs_addr;
   v3d_pack_shadrec_gl_main(shader_record_packed, &shader_record);

   assert((V3D_SHADREC_GL_MAIN_PACKED_SIZE % sizeof(uint32_t)) == 0);
   uint32_t* shader_record_attrs = shader_record_packed + V3D_SHADREC_GL_MAIN_PACKED_SIZE / sizeof(uint32_t);
   write_shader_record_attrs(shader_record_attrs, program, vertices_addr, num_vertices, instances_addr, num_instances);

   uint8_t* cl = begin_write_cl(mem, V3D_CL_GL_SHADER_SIZE);
   if (!cl)
      return false;
   V3D_CL_GL_SHADER_T shader{};
   shader.addr = shader_record_addr;
   shader.num_attr_arrays = num_attrs;
   v3d_cl_nv_shader_indirect(&cl, &shader);
   end_write_cl(mem, cl);

   return true;
}

struct dispatch_state
{
   v3d_addr_t vertices_addr;
};

static inline bool begin_build_dispatch_extent_inner(
   compute_job_mem& mem,
   dispatch_state* state,
   compute_program const& program,
   instance_config const& cfg)
{
   // timh-todo: reuse per-vertex data.
   state->vertices_addr = build_per_vertex_data(mem, program, cfg);
   if (!state->vertices_addr)
      return false;

   // timh-todo: reuse TLB lookup tables.
   v3d_addr_t tlb_lookup_addr = build_tlb_lookup(mem, program, cfg);
   if (!tlb_lookup_addr)
      return false;

   size_t const cl_size = 0
#if V3D_VER_AT_LEAST(4,1,34,0)
      + V3D_CL_TILE_COORDS_SIZE
      + V3D_CL_LOAD_SIZE
      + V3D_CL_END_LOADS_SIZE
#else
      + V3D_CL_LOAD_GENERAL_SIZE
      + V3D_CL_TILE_COORDS_SIZE
#endif
      + V3D_CL_PRIM_LIST_FORMAT_SIZE
      ;

   uint8_t* cl = begin_write_cl(mem, cl_size);
   if (!cl)
      return false;
   uint8_t* cl_end = cl + cl_size;

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_tile_coords(&cl, 0, 0);
   V3D_CL_LOAD_T load_lookup{};
   load_lookup.buffer = V3D_LDST_BUF_COLOR0;
   load_lookup.memory_format = V3D_MEMORY_FORMAT_UIF_NO_XOR;
   load_lookup.flipy = false;
   load_lookup.decimate = V3D_DECIMATE_SAMPLE0;
   load_lookup.pixel_format = V3D_PIXEL_FORMAT_RGBA16UI;
   load_lookup.stride = TLB_HEIGHT / 4; /* Height in UIF-blocks */
   load_lookup.height = 0; /* Not used when !flipy */
   load_lookup.addr = tlb_lookup_addr;
   v3d_cl_load_indirect(&cl, &load_lookup);
   v3d_cl_end_loads(&cl);
#else
   V3D_CL_LOAD_GENERAL_T load_lookup{};
   load_lookup.buffer = V3D_LDST_BUF_COLOR0;
   load_lookup.raw_mode = false;
   load_lookup.memory_format = V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR;
   load_lookup.uif_height_in_ub = TLB_HEIGHT / 4;
   load_lookup.addr = tlb_lookup_addr;
   v3d_cl_load_general_indirect(&cl, &load_lookup);
   v3d_cl_tile_coords(&cl, 0, 0);
#endif
   v3d_cl_prim_list_format(&cl, 2, false, false);

   assert(cl == cl_end);
   end_write_cl(mem, cl);

   return true;
}

static bool build_dispatch_extent_inner(
   compute_job_mem& mem,
   compute_program const& program,
   v3d_addr_t unifs_addr,
   instance_config const& cfg,
   dispatch_extent const& extent,
   dispatch_state const& state)
{
   v3d_addr_t instances_addr;
   unsigned num_instances = build_per_instance_data(
      &instances_addr,
      mem,
      program,
      cfg,
      extent,
      program.vary_map,
      program.num_varys);
   if (!num_instances)
      return 0;

   if (!write_shader_record(mem, program, unifs_addr, state.vertices_addr, cfg.num_rows*2, instances_addr, num_instances))
      return false;

   if (!write_compressed_instanced_prim_list(mem, cfg, num_instances))
      return false;

   return true;
}

static bool end_build_dispatch_extent_inner(compute_job_mem& mem)
{
   size_t const cl_size = 0
#if V3D_VER_AT_LEAST(4,1,34,0)
      + V3D_CL_STORE_SIZE
      + V3D_CL_END_TILE_SIZE
#else
      + V3D_CL_STORE_SUBSAMPLE_EX_SIZE
#endif
      ;
   uint8_t* cl = begin_write_cl(mem, cl_size);
   if (!cl)
      return false;
   uint8_t* cl_end = cl + cl_size;

#if V3D_VER_AT_LEAST(4,1,34,0)
   V3D_CL_STORE_T dummy_store{};
   dummy_store.buffer = V3D_LDST_BUF_NONE;
   v3d_cl_store_indirect(&cl, &dummy_store);
   v3d_cl_end_tile(&cl);
#else
   V3D_CL_STORE_SUBSAMPLE_EX_T dummy_store{};
   dummy_store.disable_depth_clear = true;
   dummy_store.disable_stencil_clear = true;
   dummy_store.disable_color_clear = true;
   dummy_store.stencil_store = false;
   dummy_store.depth_store = false;
   dummy_store.disable_rt_store_mask = 0xff;
   v3d_cl_store_subsample_ex_indirect(&cl, &dummy_store);
#endif

   assert(cl == cl_end);
   end_write_cl(mem, cl);

   return true;
}


static bool build_dispatch_extent(
   compute_job_mem& mem,
   const compute_program& program,
   v3d_addr_t unifs_addr,
   const instance_config& cfg,
   const dispatch_extent& extent)
{
   dispatch_state state;
   if (!begin_build_dispatch_extent_inner(mem, &state, program, cfg))
      return false;

   // Process split.extent limiting number of instances so that they fit in the data blocks.
   unsigned per_instance_data_size = get_per_instance_data_size(get_num_instances(cfg, extent), program.num_varys);
   if (per_instance_data_size > MEM_BLOCK_SIZE)
   {
      unsigned step[3];
      {
         for (unsigned i = 0; i != 3; ++i)
         {
            assert(extent.size[i] % cfg.size[i] == 0);
            step[i] = extent.size[i] / cfg.size[i];
         }
         for (; ; )
         {
            unsigned num_instances = step[0] * step[1] * step[2];
            if (get_per_instance_data_size(num_instances, program.num_varys) <= MEM_BLOCK_SIZE)
               break;

            for (unsigned i = 3; i-- != 0; )
            {
               if (step[i] > 1)
               {
                  step[i] /= 2;
                  break;
               }
            }
         }
         for (unsigned i = 0; i != 3; ++i)
         {
            step[i] *= cfg.size[i];
         }
      }

      dispatch_extent extent_part;
      for (unsigned x = 0; x < extent.size[0]; x += step[0])
      {
         extent_part.origin[0] = extent.origin[0] + x;
         extent_part.size[0] = gfx_umin(extent.size[0] - x, step[0]);

         for (unsigned y = 0; y < extent.size[1]; y += step[1])
         {
            extent_part.origin[1] = extent.origin[1] + y;
            extent_part.size[1] = gfx_umin(extent.size[1] - y, step[1]);

            for (unsigned z = 0; z < extent.size[2]; z += step[2])
            {
               extent_part.origin[2] = extent.origin[2] + z;
               extent_part.size[2] = gfx_umin(extent.size[2] - z, step[2]);

               if (!build_dispatch_extent_inner(mem, program, unifs_addr, cfg, extent_part, state))
                  return false;
            }
         }
      }
   }
   else
   {
      if (!build_dispatch_extent_inner(mem, program, unifs_addr, cfg, extent, state))
         return false;
   }

   if (!end_build_dispatch_extent_inner(mem))
      return false;

   return true;
}


static bool build_dispatch_recursive(
   compute_job_mem& mem,
   const compute_program& program,
   v3d_addr_t unifs_addr,
   const dispatch_extent& extent,
   unsigned depth)
{
   instance_config cfg = get_instance_config(program, extent.size);
   split_extents split = split_extent(cfg, extent);

   log_trace("Dispatch %u: (%u x %u x %u) wgs of (%u x %u x %u) items as %u instances of (%u * %u = %u) items",
      depth,
      extent.size[0], extent.size[1], extent.size[2],
      program.wg_size[0], program.wg_size[1], program.wg_size[2],
      split.extent.size[0] * split.extent.size[1] * split.extent.size[2] / cfg.num_wgs,
      cfg.num_wgs,
      program.items_per_wg,
      cfg.num_wgs * program.items_per_wg
      );
   if (!build_dispatch_extent(mem, program, unifs_addr, cfg, split.extent))
      return false;

   // Handle remainders, one dimension at a time.
   for (unsigned i = 0; i != 3; ++i)
   {
      const dispatch_extent& rem = split.remainders[i];
      if (rem.size[0] && rem.size[1] && rem.size[2])
      {
         if (!build_dispatch_recursive(mem, program, unifs_addr, rem, depth + 1))
            return false;
      }
   }
   return true;
}

//=============================================================================
// API functions
//=============================================================================

void compute_init(void) noexcept
{
   std::lock_guard<std::mutex> lock(runtime.mutex);
   runtime.refs += 1;
}

void compute_term(void) noexcept
{
   std::lock_guard<std::mutex> lock(runtime.mutex);
   assert(runtime.refs != 0);

   if (!--runtime.refs)
      runtime.chunks.clear_and_dispose([](mem_chunk* c) { delete c; });
}

uint32_t compute_backend_flags(unsigned items_per_wg) noexcept
{
   uint32_t backend_flags = 0;

   // If our work-groups don't exactly map onto QPU batches, then the shader will
   // need to perform the disabled element test.
   if (items_per_wg % ROW_ROUND_UP)
      backend_flags |= GLSL_COMPUTE_PADDING;
   backend_flags |= GLSL_PRIM_LINE;
   backend_flags |= ((GLSL_FB_32 | GLSL_FB_INT | GLSL_FB_PRESENT) << GLSL_FB_GADGET_S);
# if !V3D_VER_AT_LEAST(4,1,34,0)
   backend_flags |= (GLSL_FB_ALPHA_16_WORKAROUND << GLSL_FB_GADGET_S);
#endif

   return backend_flags;
}

v3d_barrier_flags compute_mem_access_flags(void) noexcept
{
   return
      V3D_BARRIER_CLE_CL_READ
    | V3D_BARRIER_CLE_SHADREC_READ
    | V3D_BARRIER_TLB_IMAGE_READ
    | V3D_BARRIER_VCD_READ;
}

compute_job_mem* compute_job_mem_new() noexcept
{
   return new (std::nothrow) compute_job_mem();
}

void compute_job_mem_delete(compute_job_mem* job) noexcept
{
   if (!job)
      return;

   std::lock_guard<std::mutex> lock(runtime.mutex);
   assert(runtime.refs != 0);
   free_blocks(*job);
   delete job;
}

void compute_job_mem_flush(compute_job_mem* job) noexcept
{
   for (mem_block_tracker& block: job->blocks)
   {
      gmem_flush_mapped_range(block.chunk->handle, MEM_BLOCK_SIZE * (&block - block.chunk->blocks), MEM_BLOCK_SIZE);
   }
}

void compute_job_mem_enumerate_accesses(compute_job_mem const* job_mem, void (*callback)(void*, gmem_handle_t, v3d_barrier_flags), void* ctx)
{
   std::lock_guard<std::mutex> lock(runtime.mutex);

   // Enumerate accesses to chunks used by this job-mem.
   for (mem_chunk const& chunk: runtime.chunks)
   {
      for (mem_block_tracker const& block: job_mem->blocks)
      {
         if (block.chunk == &chunk)
         {
            v3d_barrier_flags rw_flags =
               V3D_BARRIER_CLE_CL_READ
             | V3D_BARRIER_CLE_SHADREC_READ
             | V3D_BARRIER_TLB_IMAGE_READ
             | V3D_BARRIER_VCD_READ;
            callback(ctx, chunk.handle, rw_flags);
            break;
         }
      }
   }
}

void compute_job_mem_patch_gmp_table(compute_job_mem const* job_mem, void* gmp_table)
{
   std::lock_guard<std::mutex> lock(runtime.mutex);

   for (mem_block_tracker const& block: job_mem->blocks)
   {
      unsigned block_index = &block - block.chunk->blocks;
      v3d_addr_t addr = gmem_get_addr(block.chunk->handle) + block_index*MEM_BLOCK_SIZE;
      v3d_gmp_add_permissions(gmp_table, addr, addr + MEM_BLOCK_SIZE, V3D_GMP_READ_ACCESS);
   }
}

bool compute_build_dispatch(
   compute_job_mem* mem,
   uint8_t* dispatch_in_primary_cl,
   const compute_program* program,
   v3d_addr_t unifs_addr,
   const uint32_t num_work_groups[3]) noexcept
{
   const dispatch_extent extent =
   {
      { 0, 0, 0 },
      { num_work_groups[0], num_work_groups[1], num_work_groups[2] },
   };

   std::lock_guard<std::mutex> lock(runtime.mutex);
   assert(runtime.refs != 0);

   uint8_t* cl;

   // Ensure CL block exists and get start address.
   v3d_addr_t start_addr = get_cl_start_addr(*mem);
   if (!start_addr)
      return false;

   // Generate the control list for this dispatch.
   if (!build_dispatch_recursive(*mem, *program, unifs_addr, extent, 0))
      return false;

   // Write return from sub-routine.
   cl = begin_write_cl(*mem, V3D_CL_RETURN_SIZE);
   if (!cl)
      return false;
   v3d_cl_return(&cl);
   end_write_cl(*mem, cl);

   // Patch primary control list with branch-sub to start address.
   v3d_cl_branch_sub(&dispatch_in_primary_cl, start_addr);

   return true;
}

void compute_clear_dispatch(uint8_t* dispatch_in_primary_cl) noexcept
{
   for (unsigned i = 0; i != V3D_CL_BRANCH_SIZE; ++i)
   {
      static_assrt(V3D_CL_NOP_SIZE == 1);
      v3d_cl_nop(&dispatch_in_primary_cl);
   }
}

bool compute_cl_begin(compute_cl_mem_if const* mem, void* ctx) noexcept
{
   // Initial state setup for all compute jobs.
   size_t const cl_size = 0
#if V3D_VER_AT_LEAST(4,1,34,0)
      + V3D_CL_TILE_RENDERING_MODE_CFG_SIZE*3
      + V3D_CL_SET_INSTANCE_ID_SIZE
#else
      + V3D_CL_TILE_RENDERING_MODE_CFG_SIZE*4
#endif
      + V3D_CL_DISABLE_Z_ONLY_SIZE
      + V3D_CL_CLEAR_VCD_CACHE_SIZE
      + V3D_CL_VCM_CACHE_SIZE_SIZE
      + V3D_CL_ZERO_ALL_CENTROID_FLAGS_SIZE
#if V3D_HAS_VARY_NOPERSP
      + V3D_CL_ZERO_ALL_NOPERSPECTIVE_FLAGS_SIZE
#endif
      + V3D_CL_ZERO_ALL_FLATSHADE_FLAGS_SIZE
      + V3D_CL_SAMPLE_STATE_SIZE
      + V3D_CL_OCCLUSION_QUERY_COUNTER_ENABLE_SIZE
      + V3D_CL_LINE_WIDTH_SIZE
      + V3D_CL_VIEWPORT_OFFSET_SIZE
      + V3D_CL_CLIPZ_SIZE
      + V3D_CL_CLIP_SIZE
      + V3D_CL_CFG_BITS_SIZE
      + V3D_CL_COLOR_WMASKS_SIZE
      ;
   uint8_t* cl = mem->write_cl(ctx, cl_size);
   if (!cl)
      return false;
   uint8_t* const cl_end = cl + cl_size;

   // Tile Rendering Mode (Common Configuration).
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T cfg{};
      cfg.type = V3D_RCFG_TYPE_COMMON;
      cfg.u.common.num_rts = 1;
      cfg.u.common.frame_width = TLB_WIDTH;
      cfg.u.common.frame_height = TLB_HEIGHT;
      cfg.u.common.max_bpp = V3D_RT_BPP_64;
      cfg.u.common.ez_disable = true;
#if V3D_VER_AT_LEAST(4,1,34,0)
      cfg.u.common.internal_depth_type = V3D_DEPTH_TYPE_24;
#else
      cfg.u.common.disable_rt_store_mask = 0xff;
#endif
      v3d_cl_tile_rendering_mode_cfg_indirect(&cl, &cfg);
   }

   // Rendering Configuration (Render Target Config).
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T cfg{};
      cfg.type = V3D_RCFG_TYPE_COLOR;
      v3d_rt_bpp_t internal_bpp = V3D_RT_BPP_64;
      v3d_rt_type_t internal_type = V3D_RT_TYPE_16UI;
#if V3D_VER_AT_LEAST(4,1,34,0)
      cfg.u.color.rt_formats[0].bpp = internal_bpp;
      cfg.u.color.rt_formats[0].type = internal_type;
#else
      cfg.u.color.internal_bpp = internal_bpp;
      cfg.u.color.internal_type = internal_type;
      cfg.u.color.decimate_mode = V3D_DECIMATE_SAMPLE0;
      cfg.u.color.output_format = V3D_PIXEL_FORMAT_RGBA16UI;
      cfg.u.color.dither_mode = V3D_DITHER_OFF;
      cfg.u.color.memory_format = V3D_MEMORY_FORMAT_UIF_NO_XOR;
#endif
      v3d_cl_tile_rendering_mode_cfg_indirect(&cl, &cfg);
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
   // Rendering Configuration (Z/Stencil Config).
   {
      V3D_CL_TILE_RENDERING_MODE_CFG_T cfg{};
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
   v3d_cl_clear_vcd_cache(&cl);
   v3d_cl_vcm_cache_size(&cl, 1, 1);
   v3d_cl_zero_all_centroid_flags(&cl);
#if V3D_HAS_VARY_NOPERSP
   v3d_cl_zero_all_noperspective(&cl);
#endif
   v3d_cl_zero_all_flatshade_flags(&cl);
#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_sample_state(&cl, 0xf, 1.0f);
#else
   v3d_cl_sample_state(&cl, 1.0f);
#endif
   v3d_cl_occlusion_query_counter_enable(&cl, 0);
   v3d_cl_line_width(&cl, 2.0f);
#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_viewport_offset(&cl, 0, 0, 0, 0);
#else
   v3d_cl_viewport_offset(&cl, 0, 0);
#endif
   v3d_cl_clipz(&cl, 0.0f, 1.0f);
   v3d_cl_clip(&cl, 0, 0, TLB_WIDTH, TLB_HEIGHT);
   V3D_CL_CFG_BITS_T cfg_bits{};
   cfg_bits.front_prims = true;
   cfg_bits.rast_oversample = V3D_MS_1X;           // These are all 0, but prefer enum name.
   cfg_bits.cov_update = V3D_COV_UPDATE_NONZERO;
   cfg_bits.depth_test = V3D_COMPARE_FUNC_ALWAYS;
   v3d_cl_cfg_bits_indirect(&cl, &cfg_bits);
   v3d_cl_color_wmasks(&cl, gfx_mask(V3D_MAX_RENDER_TARGETS * 4));
#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_set_instance_id(&cl, 0);
#endif
   assert(cl == cl_end);

   return true;
}

v3d_size_t compute_cl_dispatch_size(void) noexcept
{
   return V3D_CL_BRANCH_SIZE;
}

uint8_t* compute_cl_add_dispatch(compute_cl_mem_if const* mem, void* ctx) noexcept
{
   uint8_t* cl = mem->write_cl(ctx, V3D_CL_BRANCH_SIZE);
   if (cl)
      compute_clear_dispatch(cl);
   return cl;
}

void compute_cl_end(compute_cl_mem_if const* mem, void* ctx) noexcept
{
   uint8_t* cl = mem->write_cl_final(ctx, V3D_CL_END_RENDER_SIZE);
   v3d_cl_end_render(&cl);
}

#endif
