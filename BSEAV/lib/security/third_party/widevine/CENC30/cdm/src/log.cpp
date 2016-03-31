// Copyright 2015 Google Inc. All Rights Reserved.
//
// Log - implemented using stderr.

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace wvcdm {

LogPriority g_cutoff = LOG_WARN;

void InitLogging() {}

void Log(const char* file, int line, LogPriority level, const char* fmt, ...) {
  const char* severities[] = { "ERROR", "WARN", "INFO", "DEBUG", "VERBOSE" };
  if (level >= sizeof(severities) / sizeof(*severities)) {
    fprintf(stderr, "[FATAL:%s(%d)] Invalid log priority level: %d\n",
            file, line, level);
    return;
  }
  if (level > g_cutoff) return;

  fprintf(stderr, "[%s:%s(%d)] ", severities[level], file, line);

  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  putc('\n', stderr);
  fflush(stderr);
}

}  // namespace wvcdm
