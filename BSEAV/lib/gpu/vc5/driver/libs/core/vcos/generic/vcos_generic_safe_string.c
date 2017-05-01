/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "libs/util/snprintf.h"

#if __VECTORC__
#undef _GNU_SOURCE
#endif

#if defined( __KERNEL__ )
#include <linux/string.h>
#include <linux/module.h>
#else
#include <string.h>
#endif

#include <stdarg.h>
#ifdef _GNU_SOURCE
#include <stdio.h> /* For asprintf/vasprintf */
#endif

#ifdef vcos_vasprintf
#undef vcos_vasprintf
#endif
int vcos_vasprintf(char **strp, const char *fmt, va_list ap)
{
#ifdef _GNU_SOURCE
   return vasprintf(strp, fmt, ap);
#else
   va_list ap2;
   char dummy[1];
   int out_length, out_length2;

#ifdef va_copy
   va_copy(ap2, ap);
#else
   /* The only portable way to do this is to use va_copy(). This apparently
    * works on quite a lot of platforms though (see
    * http://stackoverflow.com/questions/558223/va-copy-porting-to-visual-c)... */
   ap2 = ap;
#endif

   out_length = vsnprintf(dummy, sizeof(dummy), fmt, ap2);
   assert(out_length >= 0);

   va_end(ap2);

   *strp = malloc(out_length + 1);
   if (!*strp)
      return -1;

   out_length2 = vsnprintf(*strp, out_length + 1, fmt, ap);
   assert(out_length2 == out_length);

   return out_length2;
#endif
}

#ifdef vcos_asprintf
#undef vcos_asprintf
#endif
int vcos_asprintf(char **strp, const char *fmt, ...)
{
   va_list ap;
   int out_length;

   va_start(ap, fmt);
   out_length = vcos_vasprintf(strp, fmt, ap);
   va_end(ap);

   return out_length;
}

/** Like vsnprintf, except it places the output at the specified offset.
  * Output is truncated to fit in buflen bytes, and is guaranteed to be NUL-terminated.
  * Returns the string length before/without truncation.
  */
size_t vcos_safe_vsprintf(char *buf, size_t buflen, size_t offset, const char *fmt, va_list ap)
{
   size_t space = (offset < buflen) ? (buflen - offset) : 0;

   offset += vsnprintf(buf ? (buf + offset) : NULL, space, fmt, ap);

   return offset;
}

/** Like snprintf, except it places the output at the specified offset.
  * Output is truncated to fit in buflen bytes, and is guaranteed to be NUL-terminated.
  * Returns the string length before/without truncation.
  */
size_t vcos_safe_sprintf(char *buf, size_t buflen, size_t offset, const char *fmt, ...)
{
   size_t space = (offset < buflen) ? (buflen - offset) : 0;
   va_list ap;

   va_start(ap, fmt);

   offset += vsnprintf(buf ? (buf + offset) : NULL, space, fmt, ap);

   va_end(ap);

   return offset;
}

/** Copies string src to dst at the specified offset.
  * Output is truncated to fit in dstlen bytes, i.e. the string is at most
  * (buflen - 1) characters long. Unlike strncpy, exactly one NUL is written
  * to dst, which is always NUL-terminated.
  * Returns the string length before/without truncation.
  */
size_t vcos_safe_strcpy(char *dst, const char *src, size_t dstlen, size_t offset)
{
   if (offset < dstlen)
   {
      const char *p = src;
      char *endp = dst + dstlen -1;

      dst += offset;

      for (; *p!='\0' && dst != endp; dst++, p++)
         *dst = *p;
      *dst = '\0';
   }
   offset += strlen(src);

   return offset;
}
