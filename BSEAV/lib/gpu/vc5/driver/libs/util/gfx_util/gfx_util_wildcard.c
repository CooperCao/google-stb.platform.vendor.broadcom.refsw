/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_util_wildcard.h"

#include <assert.h>
#include <string.h>

/* Handles ? but not * */
static bool char_matches(char p, char s)
{
   assert(p && (p != '*') && s);
   return (p == '?') || (p == s);
}

/* Handles ?s but not *s */
static bool str_matches(const char *p, const char *s, size_t len)
{
   for (; len; ++p, ++s, --len)
      if (!char_matches(*p, *s))
         return false;
   return true;
}

/* Returns a pointer to the first occurrence of p in s or NULL if p is not in
 * s. p may contain ?s but not *s. */
static const char *find_str(
   const char *p, size_t p_len,
   const char *s, size_t s_len)
{
   /* Noddy implementation... */
   assert(p_len > 0);
   for (size_t i = 0; (s_len - i) >= p_len; ++i)
   {
      if (str_matches(p, s + i, p_len))
         return s + i;
   }
   return NULL;
}

bool gfx_wildcard_pattern_matches(const char *p, const char *s)
{
   /* Handle everything up to the first star */
   while (*p && (*p != '*'))
   {
      if (!*s || !char_matches(*p, *s))
         return false;
      ++p;
      ++s;
   }

   /* Handle everything past the last star */
   const char *p_end = p + strlen(p);
   const char *s_end = s + strlen(s);
   while ((p != p_end) && (p_end[-1] != '*'))
   {
      if ((s == s_end) || !char_matches(p_end[-1], s_end[-1]))
         return false;
      --p_end;
      --s_end;
   }

   /* Empty pattern only matches empty string */
   if (p == p_end)
      return s == s_end;

   while ((p + 1) != p_end)
   {
      /* Pattern must now begin with a star and end with a star */
      assert((*p == '*') && (p_end[-1] == '*'));

      /* Do shortest match of initial star and everything up to the next star */
      ++p;
      const char *next_star = strchr(p, '*');
      assert(next_star && (next_star < p_end));
      if (next_star > p)
      {
         size_t len = next_star - p;
         s = find_str(p, len, s, s_end - s);
         if (!s)
            return false;
         p = next_star;
         s += len;
      }
   }

   /* Pattern should be just "*" at this point, which matches everything */
   assert((*p == '*') && ((p + 1) == p_end));
   return true;
}
