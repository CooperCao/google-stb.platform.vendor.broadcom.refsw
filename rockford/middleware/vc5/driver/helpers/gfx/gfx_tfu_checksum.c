/*==============================================================================
 Copyright (c) 2013 Broadcom Europe Limited.
 All rights reserved.

 Module   : gfx_buffer

 FILE DESCRIPTION
 Calculates the TFU checksum for a buffer
==============================================================================*/

#include "gfx_tfu_checksum.h"

#define VCOS_LOG_CATEGORY (&gfx_tfu_checksum_log)

VCOS_LOG_CAT_T gfx_tfu_checksum_log =
   VCOS_LOG_INIT("gfx_tfu_checksum_log", VCOS_LOG_WARN);

void gfx_tfu_checksum_set_log_level(VCOS_LOG_LEVEL_T log_level, bool want_prefix)
{
   vcos_log_set_level(VCOS_LOG_CATEGORY, log_level);
   gfx_tfu_checksum_log.flags.want_prefix = want_prefix;
}

#define MSB64 ((uint64_t)(0x1) << 63)
#define POLY  ((uint64_t)0x04C11DB7 << 31 | MSB64)

void gfx_tfu_checksum_add_first_buffer(GFX_TFU_CHECKSUM_STATE_T *state, const uint32_t *buffer, size_t length)
{
   assert((length % 4) == 0); // Ensure whole number of words

   // First word inverted to bootstrap CRC to match TFU behaviour
   state->v = (uint64_t)(~buffer[0]) << 32;

   if (length > 4) // Continue checksum if there is data beyond the first word
   {
      gfx_tfu_checksum_add_buffer(state, &buffer[1], length - 4);
   }
}

// TFU checksum is CRC32 polynomial, 0X04C11DB7
void gfx_tfu_checksum_add_buffer(GFX_TFU_CHECKSUM_STATE_T *state, const uint32_t *buffer, size_t length)
{
   const size_t length_words = length / 4;
   size_t w = 0;

   assert((length % 4) == 0); // Ensure whole number of words

   for (w = 0; w < length_words; w++)
   {
      unsigned s = 0;
      state->v |= (uint64_t)buffer[w];

      for (s = 0; s < 32; s++)
      {
         if((state->v & MSB64) != 0)
         {
            state->v ^= POLY;
         }
         state->v = state->v << 1;
      }
   }
}

// TFU checksum is CRC32 polynomial, 0X04C11DB7
uint32_t gfx_tfu_checksum_finalise(const GFX_TFU_CHECKSUM_STATE_T *state)
{
   uint64_t v = state->v;
   uint32_t crc = 0;
   unsigned s = 0;

   // Add effect of 1 word of trailing zeros
   // Note: modify local copy of v
   // - effect of trailing zeros should not be persisted in the middle of a chained checksum
   v |= 0x0;

   for(s = 0; s < 32; s++)
   {
      if((v & MSB64) != 0)
      {
         v ^= POLY;
      }
      v = v << 1;
   }

   crc = v >> 32;
   return crc;
}

uint32_t gfx_tfu_checksum_check_buffer(const uint32_t *buffer, size_t length)
{
   GFX_TFU_CHECKSUM_STATE_T state;
   uint32_t crc = 0;
   gfx_tfu_checksum_add_first_buffer(&state, buffer, length);
   crc = gfx_tfu_checksum_finalise(&state);
   return crc;
}
