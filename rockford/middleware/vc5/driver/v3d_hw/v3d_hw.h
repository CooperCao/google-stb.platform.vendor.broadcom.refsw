/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_align.h"
#include "helpers/v3d/v3d_debug.h"
#include "helpers/v3d/v3d_gen.h"
#include "tools/v3d/clif_run/export_for_clif_cc.h"

#define V3D_HW_HUB_CORE V3D_MAX_CORES

/* v3d_hw::alloc_mem[_range] return type */
typedef enum
{
   /* Either a new heap was successfully allocated or there was already a heap
    * and it was sufficiently large */
   V3D_HW_ALLOC_SUCCESS,

   /* There was already a heap, but it was too small */
   V3D_HW_ALLOC_EXISTING_TOO_SMALL,

   /* There was no existing heap and the attempt to allocate one of the
    * requested size failed */
   V3D_HW_ALLOC_FAIL
} v3d_hw_alloc_res_t;

/* v3d_hw is really a class, but use struct so we can pass pointers around in
 * C. The only difference between struct and class in C++ is the default access
 * specifier. */
struct v3d_hw;

#ifdef __cplusplus

#include <functional>
#include <mutex>

struct EXPORT_FOR_CLIF_CC v3d_hw
{
private:

   /* get_[hub_]ident[_raw]() cache */
   std::mutex m_have_ident_mutex;
   bool m_have_hub_ident;
   uint32_t m_hub_ident_raw[4];
   V3D_HUB_IDENT_T m_hub_ident;
   bool m_have_ident[V3D_MAX_CORES];
   uint32_t m_ident_raw[V3D_MAX_CORES][4];
   V3D_IDENT_T m_ident[V3D_MAX_CORES];

   /* m_interrupt_mutex is held while we are handling interrupts and also
    * protects m_interrupts_enabled/m_handle_interrupts_on_reenable/m_isr */
   mutable vcos::fair_mutex m_interrupt_mutex;
   bool m_interrupts_enabled;
   bool m_handle_interrupts_on_reenable; /* Valid iff m_interrupts_enabled=false */
   std::function<void(uint32_t)> m_isr;
   vcos::tls m_in_isr; /* Set to non-NULL iff in ISR */

   /* Locked around get_mem_locked & alloc_mem_range_locked */
   mutable std::mutex m_mem_mutex;

   /* Non-copyable */
   v3d_hw(const v3d_hw &);
   v3d_hw &operator=(const v3d_hw &);

protected:
   v3d_hw(); /* Either instantiate a subclass directly or use v3d_hw_auto_make_unique() */
public:
   virtual ~v3d_hw() {}

   virtual void abort(); /* Cleanly shuts down v3d_hw if necessary then calls abort() */

   /** Misc ******************************************************************/

public:

   /* Return a short name for the platform, eg "pen" or "fpga" */
   virtual const char *name() const = 0;

   /* Return a string describing the platform */
   virtual const char *desc() const = 0;

   /* Has GCA (the BCG L3C, not determinable from ident) */
   virtual bool has_gca() const { return false; }

   /** V3D block functions ***************************************************/

public:

   /* All of the public "V3D block" functions are thread-safe. In particular:
    * - It is safe to call reset() in one thread while calling read_reg() in
    *   another. read_reg() will return either the pre-reset value or the
    *   post-reset value.
    * - It is safe to call set_isr() while interrupts are enabled -- the ISR
    *   will not be changed while it is executing! Once set_isr() returns it is
    *   guaranteed that the old ISR is not executing. */

   /* If bit i (< V3D_MAX_CORES) is set in core_mask, core i is reset.
    * If bit V3D_HW_HUB_CORE is set in core_mask, the hub is reset. */
   void reset(uint32_t core_mask=-1);

   /* Reset the entire underlying implementation.
    * e.g. on FPGA will also reset the whole wrapper. */
   void full_reset();

   /* Translate a virtual register address (eg V3D_HUB_CTL_IDENT0, see
    * helpers/v3d_regs.h) to a physical register address */
   virtual uint32_t physical_reg_addr(uint32_t virtual_addr) const { return virtual_addr; }

   /* Note that these take physical register addresses... */
   uint32_t read_reg_phys(uint32_t phys_addr) { return read_reg_common(phys_addr, /*is_phys=*/true); }
   void read_many_regs_phys(uint32_t start_phys_addr, uint32_t count, uint32_t *values)
   { read_many_regs_common(start_phys_addr, /*is_phys=*/true, count, values); }
   void write_reg_phys(uint32_t phys_addr, uint32_t value) { write_reg_common(phys_addr, /*is_phys=*/true, value); }

   /* ...while these take virtual register addresses */
   uint32_t read_reg(uint32_t virt_addr) { return read_reg_common(virt_addr, /*is_phys=*/false); }
   void read_many_regs(uint32_t start_virt_addr, uint32_t count, uint32_t *values)
   { read_many_regs_common(start_virt_addr, /*is_phys=*/false, count, values); }
   void write_reg(uint32_t virt_addr, uint32_t value) { write_reg_common(virt_addr, /*is_phys=*/false, value); }

   const V3D_HUB_IDENT_T *get_hub_ident();
   const V3D_IDENT_T *get_ident(uint32_t core);

   const uint32_t *get_hub_ident_raw();
   const uint32_t *get_ident_raw(uint32_t core);

   /* Sanity check the ident registers and print them to the v3d_hw log */
   void check_ident();

   /* If (and only if) this returns false, the hardware is ticked & interrupts
    * are processed asynchronously. Otherwise, tick() must called to guarantee
    * the hardware makes progress and interrupts are processed. */
   virtual bool requires_ticking() const = 0;

   /* Tick the hardware and process outstanding interrupts. This does not need
    * to be called or overridden if requires_ticking() returns false (though it
    * is harmless to call it in that case).
    *
    * Returns true if the hardware ticks asynchronously or progress was made by
    * this call. Not all synchronously-ticked platforms can detect if progress
    * was made, so this might always return true. */
   virtual bool tick();

   /* Return value has:
    * - bit i (< V3D_MAX_CORES) set if core i's interrupt line is high
    * - bit V3D_HW_HUB_CORE set if hub interrupt line is high */
   virtual uint32_t get_interrupt_status();

   /* uint32_t passed to ISR has same format as get_interrupt_status() return
    * value. Returns old ISR. */
   std::function<void(uint32_t)> set_isr(std::function<void(uint32_t)> isr);

   /* Is the current thread in the ISR? */
   bool in_interrupt() const { return !!m_in_isr.get(); }

   /* Disable/enable interrupts. Note that disabling interrupts just prevents
    * the ISR from being called -- the interrupt lines may still go high. */
   bool interrupts_enabled() const;
   bool disable_interrupts(); /* Returns true iff interrupts were enabled */
   bool enable_interrupts(); /* Returns true iff interrupts were disabled */

protected:

   virtual void reset_raw(uint32_t core_mask) = 0;
   virtual void full_reset_raw() { reset_raw(-1); }

   virtual void read_many_regs_phys_raw(uint32_t start_phys_addr, uint32_t count, uint32_t *values) = 0;
   virtual void write_reg_phys_raw(uint32_t phys_addr, uint32_t value) = 0;

   /* handle_interrupts() should be called when an interrupt line goes high
    * (though it is harmless to call it even if no interrupt lines are high).
    * On platforms where requires_ticking() returns true typically this will
    * just be called at the end of tick().
    *
    * After a handle_interrupts() call, interrupts_handled() will be called
    * once all outstanding interrupts have been handled. handle_interrupts()
    * needn't be called again before this.
    *
    * handle_interrupts() will return true iff it called the ISR. */
   bool handle_interrupts();
   virtual void interrupts_handled() {}

private:

   uint32_t read_reg_common(uint32_t addr, bool is_phys);
   void read_many_regs_common(uint32_t start_addr, bool is_phys, uint32_t count, uint32_t *values);
   void write_reg_common(uint32_t addr, bool is_phys, uint32_t value);

   /* Just like read_reg(), but don't print anything to the log. This is used
    * by the default get_interrupt_status() implementation to avoid spamming
    * the log. */
   uint32_t read_reg_no_log(uint32_t virt_addr);

   void ensure_have_hub_ident();
   void ensure_have_ident(uint32_t core);

   /** Memory functions ******************************************************/

public:

   /* If there is a heap, the V3D base address of the heap is returned and
    * *size is set to the size of the heap. If p is not NULL, *p is set to
    * point at the base of the heap in host memory. Note that host writes to
    * the heap may not be immediately visible to V3D and vice-versa:
    * sync_from/to_host() must be called to synchronise if
    * need_sync_from/to_host() return true. Additionally, after the first
    * successful call with p != NULL, the state of the heap as seen from the
    * host (through *p) may not match the state as seen from V3D.
    * sync_from/to_host() can be called to remedy this.
    *
    * The V3D base address of the heap is guaranteed to be non-0 and at least
    * V3D_MAX_ALIGN aligned. *p will also be at least V3D_MAX_ALIGN aligned.
    * The size of the heap is guaranteed to be a multiple of the sync block
    * size.
    *
    * If there is no heap, 0 is returned. alloc_mem[_range]() may be used to
    * allocate a heap.
    *
    * It is safe to call get_mem() at any point, even while another thread is
    * in alloc_mem[_range](). */
   v3d_addr_t get_mem(size_t *size, void **p) const;

   /* If a heap already exists, V3D_HW_ALLOC_SUCCESS is returned if it is at
    * least min_size bytes, otherwise V3D_HW_ALLOC_EXISTING_TOO_SMALL is
    * returned.
    *
    * If a heap does not exist, allocates the largest heap it can that is >=
    * min_size bytes and <= ideal_size bytes. V3D_HW_ALLOC_SUCCESS is returned
    * if this succeeds, V3D_HW_ALLOC_FAIL otherwise. The allocated heap need
    * not be explicitly freed -- it will be automatically freed when the
    * destructor is called. */
   v3d_hw_alloc_res_t alloc_mem_range(size_t min_size, size_t ideal_size);

   v3d_hw_alloc_res_t alloc_mem(size_t min_size)
   {
      return alloc_mem_range(min_size, min_size + get_sync_block_size() - 1);
   }

   /* Memory is conceptually divided into sync blocks: the byte at V3D address
    * a is in sync block (a / get_sync_block_size()).
    *
    * Concurrent access of a sync block from V3D and host will give undefined
    * results unless both accesses are reads.
    *
    * If need_sync_from/to_host() returns true, host/V3D writes are not
    * considered finished until the written data is synced to the other side
    * with sync_from/to_host.
    *
    * Note that sync_from/to_host() imply a host/V3D read of the memory region.
    * This means that eg directly writing to a region of memory on the host and
    * then calling sync_to_host() on that region will give undefined results.
    *
    * If discard=true, sync_from/to_host() need not make any of the CPU/V3D
    * writes since the last sync visible to the other side. The sync will still
    * count as a sync for the above rules however, and any subsequent writes
    * from either side are guaranteed not to be overridden by the "discarded"
    * writes. */
   virtual size_t get_sync_block_size() const { return 1; }
   void expand_to_sync_block_boundaries(v3d_addr_t *addr, size_t *size);
   virtual bool need_sync_from_host() const { return false; }
   virtual bool need_sync_to_host() const { return false; }
   void sync_from_host(v3d_addr_t addr, size_t size, bool discard=false);
   void sync_to_host(v3d_addr_t addr, size_t size, bool discard=false);

   /* write_mem() is equivalent to memcpy()ing via V3D and then calling
    * sync_to_host(). The results will be undefined if V3D accesses any of the
    * involved sync blocks during the write_mem() call, but accesses
    * immediately before/after are fine. write_mem() may be implemented as:
    * - sync_to_host(range expanded to sync block boundaries)
    * - memcpy() into host memory
    * - sync_from_host()
    *
    * read_mem() is equivalent to calling sync_to_host() and then memcpy()ing
    * from host memory. This means that eg directly writing to a region of
    * memory on the host and then calling read_mem() on that region will give
    * undefined results! You must call sync_from_host() between the write and
    * the call to read_mem() to get well-defined results. */
   void write_mem(v3d_addr_t addr, const void *p, size_t size);
   void read_mem(void *p, v3d_addr_t addr, size_t size);

   void write_mem_streaming(v3d_addr_t addr, size_t max_size,
      /* Fetch the next chunk_max_size bytes or fewer. Returns the actual
       * number of fetched bytes. These bytes will be written at chunk_addr.
       * The streaming write will terminate if fewer than chunk_max_size bytes
       * are fetched, or when max_size bytes have been written in total. */
      const std::function<size_t(void *chunk, size_t chunk_max_size, v3d_addr_t chunk_addr)> &fetch);
   void read_mem_streaming(v3d_addr_t addr, size_t size,
      const std::function<void(const void *chunk, size_t chunk_size, v3d_addr_t chunk_addr)> &handle);

   void set_mem(v3d_addr_t addr, uint8_t value, size_t size);

protected:

   /* Helper funcs for implementing alloc_mem_range_locked() */
   static size_t get_max_heap_size_from_addr_range(
      v3d_addr_t base_addr, v3d_addr_t max_addr);
   void *malloc_heap(size_t *size, size_t min_size, size_t ideal_size,
      v3d_addr_t base_addr, v3d_addr_t max_addr); /* Free with free_aligned! */

   /* get_mem/alloc_mem_range call these with m_mem_mutex locked */
   virtual v3d_addr_t get_mem_locked(size_t *size, void **p) const = 0;
   virtual v3d_hw_alloc_res_t alloc_mem_range_locked(size_t min_size, size_t ideal_size); /* Must be overridden if get_mem_locked() ever returns 0 */

   /* Raw sync/read/write functions. The normal functions do logging and range
    * checks and such and then call these.
    *
    * If you override sync_from/to_host_raw, you should also override
    * need_sync_from/to_host() to return true. */
   virtual void sync_from_host_raw(v3d_addr_t addr, size_t size, bool discard=false) {}
   virtual void sync_to_host_raw(v3d_addr_t addr, size_t size, bool discard=false) {}
   virtual void write_mem_raw(v3d_addr_t addr, const void *p, size_t size);
   virtual void read_mem_raw(void *p, v3d_addr_t addr, size_t size);
   virtual void write_mem_streaming_raw(v3d_addr_t addr, size_t max_size,
      const std::function<size_t(void *chunk, size_t chunk_max_size, v3d_addr_t chunk_addr)> &fetch);
   virtual void read_mem_streaming_raw(v3d_addr_t addr, size_t size,
      const std::function<void(const void *chunk, size_t chunk_size, v3d_addr_t chunk_addr)> &handle);

private:

#ifdef NDEBUG
   void check_range(v3d_addr_t addr, size_t size) {}
#else
   void check_range(v3d_addr_t addr, size_t size);
#endif

   /* Expands addr/size to sync block boundaries and then calls sync_to_host_raw */
   void sync_to_host_raw_expanded(v3d_addr_t addr, size_t size);

   /** Debug functions *******************************************************/

public:

   /* On most platforms these functions will not do anything... */

   virtual void set_debug_callback(v3d_debug_callback_t callback, void *p) {}

   /* vertex/frament_shader_debug: if true, attach qdebug when the QPS starts a
    * shader */
   virtual void set_fragment_shader_debug(bool enabled) {}
   virtual bool get_fragment_shader_debug() const { return false; }
   virtual void set_vertex_shader_debug(bool enabled) {}
   virtual bool get_vertex_shader_debug() const { return false; }

   virtual void memory_trace_file_open(const char *filename) {}
};

/** RAII helpers *************************************************************/

class v3d_hw_interrupt_disabler
{
   v3d_hw *m_hw;
   bool m_were_enabled;
   bool m_enable_on_destruct;

   /* Non-copyable */
   v3d_hw_interrupt_disabler(const v3d_hw_interrupt_disabler &);
   v3d_hw_interrupt_disabler &operator=(const v3d_hw_interrupt_disabler &);

public:

   v3d_hw_interrupt_disabler(v3d_hw *hw) :
      m_hw(hw),
      m_were_enabled(hw->disable_interrupts()),
      m_enable_on_destruct(m_were_enabled)
   {}

   ~v3d_hw_interrupt_disabler()
   {
      if (m_enable_on_destruct)
         m_hw->enable_interrupts();
   }

   bool were_enabled() const { return m_were_enabled; }
   void disable_on_destruct() { m_enable_on_destruct = false; }
   void enable_on_destruct() { m_enable_on_destruct = true; }
};

class v3d_hw_override_isr
{
   v3d_hw *m_hw;
   std::function<void(uint32_t)> m_old_isr;

   /* Non-copyable */
   v3d_hw_override_isr(const v3d_hw_override_isr &);
   v3d_hw_override_isr &operator=(const v3d_hw_override_isr &);

public:

   v3d_hw_override_isr(v3d_hw *hw, std::function<void(uint32_t)> isr) :
      m_hw(hw), m_old_isr(hw->set_isr(std::move(isr)))
   {}

   ~v3d_hw_override_isr()
   {
      m_hw->set_isr(std::move(m_old_isr));
   }
};

#endif

/** C interface **************************************************************/

/* These just directly call the functions in the class above */

VCOS_EXTERN_C_BEGIN

extern void v3d_hw_delete(struct v3d_hw *hw);

extern const char *v3d_hw_name(const struct v3d_hw *hw);
extern const char *v3d_hw_desc(const struct v3d_hw *hw);
extern bool v3d_hw_has_gca(const struct v3d_hw *hw);

extern void v3d_hw_reset(struct v3d_hw *hw);
extern uint32_t v3d_hw_physical_reg_addr(const struct v3d_hw *hw, uint32_t virtual_addr);
extern uint32_t v3d_hw_read_reg_phys(struct v3d_hw *hw, uint32_t phys_addr);
extern void v3d_hw_read_many_regs_phys(struct v3d_hw *hw,
   uint32_t start_phys_addr, uint32_t count, uint32_t *values);
extern void v3d_hw_write_reg_phys(struct v3d_hw *hw,
   uint32_t phys_addr, uint32_t value);
extern uint32_t v3d_hw_read_reg(struct v3d_hw *hw, uint32_t virt_addr);
extern void v3d_hw_read_many_regs(struct v3d_hw *hw,
   uint32_t start_virt_addr, uint32_t count, uint32_t *values);
extern void v3d_hw_write_reg(struct v3d_hw *hw, uint32_t virt_addr, uint32_t value);
extern const V3D_HUB_IDENT_T *v3d_hw_get_hub_ident(struct v3d_hw *hw);
extern const V3D_IDENT_T *v3d_hw_get_ident(struct v3d_hw *hw, uint32_t core);
extern const uint32_t *v3d_hw_get_hub_ident_raw(struct v3d_hw *hw);
extern const uint32_t *v3d_hw_get_ident_raw(struct v3d_hw *hw, uint32_t core);
extern void v3d_hw_check_ident(struct v3d_hw *hw);
extern bool v3d_hw_requires_ticking(const struct v3d_hw *hw);
extern bool v3d_hw_tick(struct v3d_hw *hw);
extern uint32_t v3d_hw_get_interrupt_status(struct v3d_hw *hw);
extern void v3d_hw_set_isr(struct v3d_hw *hw, void (*isr)(uint32_t, void *), void *p);
extern bool v3d_hw_in_interrupt(const struct v3d_hw *hw);
extern bool v3d_hw_interrupts_enabled(const struct v3d_hw *hw);
extern bool v3d_hw_disable_interrupts(struct v3d_hw *hw);
extern bool v3d_hw_enable_interrupts(struct v3d_hw *hw);

extern v3d_addr_t v3d_hw_get_mem(const struct v3d_hw *hw, size_t *size, void **p);
extern v3d_hw_alloc_res_t v3d_hw_alloc_mem_range(struct v3d_hw *hw, size_t min_size, size_t ideal_size);
extern v3d_hw_alloc_res_t v3d_hw_alloc_mem(struct v3d_hw *hw, size_t min_size);
extern size_t v3d_hw_get_sync_block_size(const struct v3d_hw *hw);
extern bool v3d_hw_need_sync_from_host(const struct v3d_hw *hw);
extern bool v3d_hw_need_sync_to_host(const struct v3d_hw *hw);
extern void v3d_hw_sync_from_host(struct v3d_hw *hw, v3d_addr_t addr, size_t size, bool discard);
extern void v3d_hw_sync_to_host(struct v3d_hw *hw, v3d_addr_t addr, size_t size, bool discard);
extern void v3d_hw_write_mem(struct v3d_hw *hw, v3d_addr_t addr, const void *p, size_t size);
extern void v3d_hw_read_mem(struct v3d_hw *hw, void *p, v3d_addr_t addr, size_t size);

extern void v3d_hw_set_debug_callback(struct v3d_hw *hw, v3d_debug_callback_t callback, void *p);
extern void v3d_hw_set_fragment_shader_debug(struct v3d_hw *hw, bool enabled);
extern bool v3d_hw_get_fragment_shader_debug(const struct v3d_hw *hw);
extern void v3d_hw_set_vertex_shader_debug(struct v3d_hw *hw, bool enabled);
extern bool v3d_hw_get_vertex_shader_debug(const struct v3d_hw *hw);
extern void v3d_hw_memory_trace_file_open(struct v3d_hw *hw, const char *filename);

VCOS_EXTERN_C_END
