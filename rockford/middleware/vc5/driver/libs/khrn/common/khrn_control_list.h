/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Common hardware/simpenrose interface

FILE DESCRIPTION
Drivers use this interface to create/edit control lists
=============================================================================*/

#ifndef KHRN_CONTROL_LIST_4_H
#define KHRN_CONTROL_LIST_4_H

#include "khrn_int_util.h"

/******************************************************************************
 Constants that come from the hw
 ******************************************************************************/

/* TODO: These are quite likely to have hardcoded values used elsewhere */
#define KHRN_HW_LOG2_TILE_WIDTH 6 /* non-ms */
#define KHRN_HW_LOG2_TILE_HEIGHT 6 /* non-ms */
#define KHRN_HW_TILE_WIDTH (1 << KHRN_HW_LOG2_TILE_WIDTH)
#define KHRN_HW_TILE_HEIGHT (1 << KHRN_HW_LOG2_TILE_HEIGHT)
#define KHRN_HW_VPM_BLOCKS_N (192 / 4)
#define KHRN_GAP_AT_END_OF_RENDER 5 /* This is the space for a branch at the end of the list */

/******************************************************************************
 Helper functions for constructing control lists
 ******************************************************************************/

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

static inline void put_word(void *addr, uint32_t n)
{
   uint8_t *p = (uint8_t *) addr;

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

static inline void add_pointer(uint8_t **p, v3d_addr_t ptr)
{
   add_word(p, ptr);
}

#define ADD_BYTE(p, n) add_byte(&(p), (n))
#define ADD_SHORT(p, n) add_short(&(p), (n))
#define ADD_WORD(p, n) add_word(&(p), (n))
#define ADD_FLOAT(p, n) add_float(&(p), (n))
#define ADD_POINTER(p, n) add_pointer(&(p), (n))

#endif
