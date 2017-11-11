/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if HAS_DEBUG_HELPER
extern __attribute__((format(printf, 1, 2))) void platform_dbg_message_add(const char *fmt, ...);
#else
static inline __attribute__((format(printf, 1, 2))) void platform_dbg_message_add(const char *fmt __attribute__((unused)), ...) {};
#endif
