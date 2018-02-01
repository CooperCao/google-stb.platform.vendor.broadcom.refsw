/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/common/khrn_hw.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "interface/khronos/common/khrn_options.h"

/******************************************************************************
constants that come from the hw
******************************************************************************/


#define KHRN_HW_SYSTEM_CACHE_LINE_SIZE 32
#define KHRN_HW_TLB_ALIGN 16 /* todo: is this right? */
#define KHRN_HW_LOG2_TILE_WIDTH 6 /* non-ms */
#define KHRN_HW_LOG2_TILE_HEIGHT 6 /* non-ms */
#define KHRN_HW_TILE_WIDTH (1 << KHRN_HW_LOG2_TILE_WIDTH)
#define KHRN_HW_TILE_HEIGHT (1 << KHRN_HW_LOG2_TILE_HEIGHT)
#define KHRN_HW_TILE_STATE_SIZE 48
#define KHRN_HW_BIN_MEM_ALIGN 256
#define KHRN_HW_BIN_MEM_GRANULARITY 4096
#define KHRN_HW_CL_BLOCK_SIZE_MIN 32
#define KHRN_HW_CL_BLOCK_SIZE_MAX 256
#define KHRN_HW_TEX_SIZE_MAX 2048 /* max width/height */
#define KHRN_HW_TEX_ALIGN 4096
#define KHRN_HW_VPM_BLOCKS_N (192 / 4)

/******************************************************************************
misc stuff
******************************************************************************/

static inline uint32_t khrn_hw_addr(const void *addr, MEM_LOCK_T *lbh)
{
   /* just for debug */
   if ((addr < lbh->p) || (addr >(void *)((uintptr_t)lbh->p + lbh->size)))
      printf("address out of range for block %p, lbh->p %p, lbh->size %d lbh->name %s\n", addr, lbh->p, lbh->size, lbh->desc);

   return lbh->offset + ((uintptr_t)addr - (uintptr_t)lbh->p);
}

static inline void khrn_hw_flush_dcache_range(void *p, uint32_t size) { mem_flush_cache_range(p, size); }

/******************************************************************************
stuff for writing control lists
******************************************************************************/

typedef uint64_t KHRN_SHADER_T;
static inline uint8_t get_byte(const uint8_t *p)
{
   #ifdef BIG_ENDIAN_CPU
      return ((uint8_t)*(uint8_t*)((uint32_t)&p[0]^0x3));
   #else
      return p[0];
   #endif
}

static inline uint16_t get_short(const uint8_t *p)
{
   #ifdef BIG_ENDIAN_CPU
      return ((uint16_t)*(uint8_t*)((uint32_t)&p[0]^0x3)) |
             ((uint16_t)*(uint8_t*)(((uint32_t)&p[1]^0x3)) << 8);
   #else
      return  (uint16_t)p[0] |
             ((uint16_t)p[1] << 8);
   #endif
}

static inline uint32_t get_word(const uint8_t *p)
{
   #ifdef BIG_ENDIAN_CPU
      return ((uint32_t)*(uint8_t*)((uint32_t)&p[0]^0x3)) |
             ((uint32_t)*(uint8_t*)(((uint32_t)&p[1]^0x3)) << 8) |
             ((uint32_t)*(uint8_t*)(((uint32_t)&p[2]^0x3)) << 16) |
             ((uint32_t)*(uint8_t*)(((uint32_t)&p[3]^0x3)) << 24);
   #else
      return  (uint32_t)p[0] |
             ((uint32_t)p[1] << 8) |
             ((uint32_t)p[2] << 16) |
             ((uint32_t)p[3] << 24);
   #endif
}

static inline void put_byte(uint8_t *p, uint8_t n)
{
   #ifdef BIG_ENDIAN_CPU
      *(uint8_t*)((uint32_t)&p[0]^0x3) = n;
   #else
      p[0] = n;
   #endif
}
static inline void put_short(uint8_t *p, uint16_t n)
{
   #ifdef BIG_ENDIAN_CPU
      *(uint8_t*)((uint32_t)&p[0]^0x3) = (uint8_t)n;
      *(uint8_t*)((uint32_t)&p[1]^0x3) = (uint8_t)(n >> 8);
   #else
      p[0] = (uint8_t)n;
      p[1] = (uint8_t)(n >> 8);
   #endif
}

static inline void put_word(uint8_t *p, uint32_t n)
{
   #ifdef BIG_ENDIAN_CPU
      *(uint8_t*)((uint32_t)&p[0]^0x3) = (uint8_t)n;
      *(uint8_t*)((uint32_t)&p[1]^0x3) = (uint8_t)(n >> 8);
      *(uint8_t*)((uint32_t)&p[2]^0x3) = (uint8_t)(n >> 16);
      *(uint8_t*)((uint32_t)&p[3]^0x3) = (uint8_t)(n >> 24);
   #else
      p[0] = (uint8_t)n;
      p[1] = (uint8_t)(n >> 8);
      p[2] = (uint8_t)(n >> 16);
      p[3] = (uint8_t)(n >> 24);
   #endif
}

static inline void put_float(uint8_t *p, float f)
{
   put_word(p, float_to_bits(f));
}

static inline void add_byte(uint8_t **p, uint8_t n)
{
   put_byte(*p, n);
   (*p) += 1;
}

static inline void add_short(uint8_t **p, uint16_t n)
{
   put_short(*p, n);
   (*p) += 2;
}

static inline void add_word(uint8_t **p, uint32_t n)
{
   put_word(*p, n);
   (*p) += 4;
}

static inline void add_float(uint8_t **p, float f)
{
   add_word(p, float_to_bits(f));
}

static inline void add_pointer(uint8_t **p, void *ptr, MEM_LOCK_T *lbh)
{
   add_word(p, khrn_hw_addr(ptr, lbh));   //PTR
}

#define ADD_BYTE(p, n) add_byte(&(p), (n))
#define ADD_SHORT(p, n) add_short(&(p), (n))
#define ADD_WORD(p, n) add_word(&(p), (n))
#define ADD_FLOAT(p, n) add_float(&(p), (n))
#define ADD_POINTER(p, n, b) add_pointer(&(p), (n), (b))

#define KHRN_HW_INSTR_HALT                   0
#define KHRN_HW_INSTR_NOP                    1
#define KHRN_HW_INSTR_MARKER                 2
#define KHRN_HW_INSTR_RESET_MARKER_COUNT     3
#define KHRN_HW_INSTR_FLUSH                  4
#define KHRN_HW_INSTR_FLUSH_ALL_STATE        5
#define KHRN_HW_INSTR_START_TILE_BINNING     6
#define KHRN_HW_INSTR_INCR_SEMAPHORE         7
#define KHRN_HW_INSTR_WAIT_SEMAPHORE         8
#define KHRN_HW_INSTR_BRANCH                16
#define KHRN_HW_INSTR_BRANCH_SUB            17
#define KHRN_HW_INSTR_RETURN                18
#define KHRN_HW_INSTR_REPEAT_START_MARKER   19
#define KHRN_HW_INSTR_REPEAT_FROM_START_MARKER 20
#define KHRN_HW_INSTR_STORE_SUBSAMPLE       24
#define KHRN_HW_INSTR_STORE_SUBSAMPLE_EOF   25
#define KHRN_HW_INSTR_STORE_FULL            26
#define KHRN_HW_INSTR_LOAD_FULL             27
#define KHRN_HW_INSTR_STORE_GENERAL         28
#define KHRN_HW_INSTR_LOAD_GENERAL          29
#define KHRN_HW_INSTR_GLDRAWELEMENTS        32
#define KHRN_HW_INSTR_GLDRAWARRAYS          33
#define KHRN_HW_INSTR_VG_COORD_LIST         41
#define KHRN_HW_INSTR_COMPRESSED_LIST       48
#define KHRN_HW_INSTR_CLIPPED_PRIM          49
#define KHRN_HW_INSTR_PRIMITIVE_LIST_FORMAT 56
#define KHRN_HW_INSTR_GL_SHADER             64
#define KHRN_HW_INSTR_NV_SHADER             65
#define KHRN_HW_INSTR_VG_SHADER             66
#define KHRN_HW_INSTR_INLINE_VG_SHADER      67
#define KHRN_HW_INSTR_STATE_CFG             96
#define KHRN_HW_INSTR_STATE_FLATSHADE       97
#define KHRN_HW_INSTR_STATE_POINT_SIZE      98
#define KHRN_HW_INSTR_STATE_LINE_WIDTH      99
#define KHRN_HW_INSTR_STATE_RHTX           100
#define KHRN_HW_INSTR_STATE_DEPTH_OFFSET   101
#define KHRN_HW_INSTR_STATE_CLIP           102
#define KHRN_HW_INSTR_STATE_VIEWPORT_OFFSET 103
#define KHRN_HW_INSTR_STATE_CLIPZ          104
#define KHRN_HW_INSTR_STATE_CLIPPER_XY     105
#define KHRN_HW_INSTR_STATE_CLIPPER_Z      106
#define KHRN_HW_INSTR_STATE_TILE_BINNING_MODE 112
#define KHRN_HW_INSTR_STATE_TILE_RENDERING_MODE 113
#define KHRN_HW_INSTR_STATE_CLEARCOL       114
#define KHRN_HW_INSTR_STATE_TILE_COORDS    115

/******************************************************************************
hw fifo
******************************************************************************/

extern bool khrn_hw_init(void);
extern void khrn_hw_term(void);

#define KHRN_HW_SPECIAL_0            ((MEM_HANDLE_T)0) /* used for extra thrsw removing hack. todo: no need for this hack on b0 */
#define KHRN_HW_SPECIAL_BIN_MEM      ((MEM_HANDLE_T)1)
#define KHRN_HW_SPECIAL_BIN_MEM_END  ((MEM_HANDLE_T)2)
#define KHRN_HW_SPECIAL_BIN_MEM_SIZE ((MEM_HANDLE_T)3)

extern void khrn_hw_wait(void);
