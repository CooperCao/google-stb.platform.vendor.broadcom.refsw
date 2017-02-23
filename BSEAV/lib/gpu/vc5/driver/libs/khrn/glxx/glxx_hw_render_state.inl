/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

static inline GLXX_CL_RECORD_T *glxx_hw_render_state_current_cl_record(GLXX_HW_RENDER_STATE_T *rs)
{
   return &rs->cl_records[rs->num_cl_records];
}

static inline void glxx_cl_record_begin(GLXX_CL_RECORD_T *record, glxx_cl_state_t state, uint8_t *instr)
{
   // end must be called before begin can be called again
   assert(record->in_begin == GLXX_CL_STATE_NUM);
   assert(instr != NULL);
#ifndef NDEBUG
   record->in_begin = instr ? state : GLXX_CL_STATE_NUM;
#endif

#ifdef NDEBUG
   if (GLXX_MULTICORE_BIN_ENABLED)
#endif
   {
      // store pointer to state
      record->ptrs[state] = instr;
   }
}

static inline void glxx_cl_record_end(GLXX_CL_RECORD_T *record, glxx_cl_state_t state, uint8_t *instr)
{
   // end must be preceeded by call to begin for same state
   assert(record->in_begin == state);
#ifndef NDEBUG
   record->in_begin = GLXX_CL_STATE_NUM;
#endif

   unsigned size = instr - record->ptrs[state];
   if (state >= GLXX_CL_STATE_NUM_FIXED_SIZE)
   {
      // assert on number of bytes were written for this state
      assert(size <= GLXX_CL_STATE_SIZE[state]);

      if (GLXX_MULTICORE_BIN_ENABLED)
      {
         // store variable size for this state
         record->sizes[state - GLXX_CL_STATE_NUM_FIXED_SIZE] = size;
      }
   }
   else
   {
      // assert correct number of bytes were written for this state
      assert(size == GLXX_CL_STATE_SIZE[state]);
   }
}

static inline uint8_t *glxx_hw_render_state_begin_cle(GLXX_HW_RENDER_STATE_T *rs, glxx_cl_state_t state)
{
   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, GLXX_CL_STATE_SIZE[state]);
   if (instr)
   {
      glxx_cl_record_begin(glxx_hw_render_state_current_cl_record(rs), state, instr);
   }
   return instr;
}

static inline void glxx_hw_render_state_end_cle(GLXX_HW_RENDER_STATE_T *rs, glxx_cl_state_t state, uint8_t *instr)
{
   glxx_cl_record_end(glxx_hw_render_state_current_cl_record(rs), state, instr);
   if (state >= GLXX_CL_STATE_NUM_FIXED_SIZE)
   {
      khrn_fmem_end_cle(&rs->fmem, instr);
   }
   else
   {
      khrn_fmem_end_cle_exact(&rs->fmem, instr);
   }
}
