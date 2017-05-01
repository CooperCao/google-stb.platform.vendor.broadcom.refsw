/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_hw_render_state.h"

bool glxx_cl_record_validate(GLXX_CL_RECORD_T *record)
{
   assert(record->in_begin == GLXX_CL_STATE_NUM);

   // check all records are present
   for (glxx_cl_state_t i = 0; i != GLXX_CL_STATE_NUM; ++i)
   {
      unsigned size = i < GLXX_CL_STATE_NUM_FIXED_SIZE ? GLXX_CL_STATE_SIZE[i] : record->sizes[i - GLXX_CL_STATE_NUM_FIXED_SIZE];
      if ((record->ptrs[i] == NULL || size == 0) && !((1 << i) & GLXX_CL_STATE_OPTIONAL))
      {
         return false;
      }
   }
   return true;
}

bool glxx_cl_record_apply(GLXX_CL_RECORD_T *record, khrn_fmem *fmem)
{
   assert(record->in_begin == GLXX_CL_STATE_NUM);

   // compute max size needed
   static unsigned max_size = 0;
   if (max_size == 0)
   {
      unsigned size = 0; // use local to cope with unlikely case of multiple threads in here.
      for (glxx_cl_state_t i = 0; i != GLXX_CL_STATE_NUM; ++i)
      {
         size += GLXX_CL_STATE_SIZE[i];
      }
      max_size = size;
   }

   uint8_t *instr = khrn_fmem_begin_cle(fmem, max_size);
   if (instr)
   {
      for (glxx_cl_state_t i = 0; i != GLXX_CL_STATE_NUM; ++i)
      {
         unsigned size = i < GLXX_CL_STATE_NUM_FIXED_SIZE ? GLXX_CL_STATE_SIZE[i] : record->sizes[i - GLXX_CL_STATE_NUM_FIXED_SIZE];
         assert((record->ptrs[i] != NULL && size != 0) || ((1 << i) & GLXX_CL_STATE_OPTIONAL));
         if (record->ptrs[i])
         {
            memcpy(instr, record->ptrs[i], size);
            instr += size;
         }
      }
      khrn_fmem_end_cle(fmem, instr);
      return true;
   }
   return false;
}