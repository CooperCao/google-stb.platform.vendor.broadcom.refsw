/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "vcos_types.h"
#include <stdbool.h>
#include <stdio.h>

EXTERN_C_BEGIN

typedef enum
{
   GFX_TERM_COL_BLACK      = 0,
   GFX_TERM_COL_RED        = 1,
   GFX_TERM_COL_GREEN      = 2,
   GFX_TERM_COL_YELLOW     = 3,
   GFX_TERM_COL_BLUE       = 4,
   GFX_TERM_COL_MAGENTA    = 5,
   GFX_TERM_COL_CYAN       = 6,
   GFX_TERM_COL_WHITE      = 7,

   GFX_TERM_COL_BR_BLACK   = 8 + 0,
   GFX_TERM_COL_BR_RED     = 8 + 1,
   GFX_TERM_COL_BR_GREEN   = 8 + 2,
   GFX_TERM_COL_BR_YELLOW  = 8 + 3,
   GFX_TERM_COL_BR_BLUE    = 8 + 4,
   GFX_TERM_COL_BR_MAGENTA = 8 + 5,
   GFX_TERM_COL_BR_CYAN    = 8 + 6,
   GFX_TERM_COL_BR_WHITE   = 8 + 7,

   GFX_TERM_COL_INVALID
} gfx_term_col_t;

extern bool gfx_term_col_ok(FILE *f);
extern void gfx_term_col_set_fg(FILE *f, gfx_term_col_t color);
extern void gfx_term_col_set_bg(FILE *f, gfx_term_col_t color);
extern void gfx_term_col_reset(FILE *f);

extern size_t gfx_term_col_sprint_set_fg(char *buf, size_t buf_size, size_t offset,
   gfx_term_col_t color);
extern size_t gfx_term_col_sprint_set_bg(char *buf, size_t buf_size, size_t offset,
   gfx_term_col_t color);
extern size_t gfx_term_col_sprint_reset(char *buf, size_t buf_size, size_t offset);

EXTERN_C_END

#ifdef __cplusplus

#include <stdexcept>

class gfx_term_col_file
{
   FILE *m_f;
   bool m_col_ok;
   gfx_term_col_t m_current_col;

public:

   /* Non-copyable */
   gfx_term_col_file(const gfx_term_col_file &) = delete;
   gfx_term_col_file &operator=(const gfx_term_col_file &) = delete;

   gfx_term_col_file(FILE *f) :
      m_f(f),
      m_col_ok(gfx_term_col_ok(f)),
      m_current_col(GFX_TERM_COL_INVALID)
   {
   }

   ~gfx_term_col_file()
   {
      set_col(GFX_TERM_COL_INVALID);
   }

   void write(gfx_term_col_t col, const void *buf, size_t size)
   {
      if (size)
      {
         set_col(col);
         if (fwrite(buf, 1, size, m_f) != size)
            throw std::runtime_error("fwrite() failed!");
      }
   }

   void flush()
   {
      if (fflush(m_f) != 0)
         throw std::runtime_error("fflush() failed!");
   }

private:

   void set_col(gfx_term_col_t col)
   {
      if (m_current_col != col)
      {
         m_current_col = col;
         if (m_col_ok)
         {
            if (col == GFX_TERM_COL_INVALID)
               gfx_term_col_reset(m_f);
            else
               gfx_term_col_set_fg(m_f, col);
         }
      }
   }
};

#endif
