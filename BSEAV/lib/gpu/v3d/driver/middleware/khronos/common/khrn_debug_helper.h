/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if HAS_DEBUG_HELPER
extern void platform_dbg_message_add(const char *fmt, ...);
#else
static inline void platform_dbg_message_add(const char *fmt, ...) { UNUSED(fmt); };
#endif
