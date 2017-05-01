/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#define DEBUG_WARN 0
#define DEBUG_ERROR 1

extern void debug_log(int level, const char *f, ...);
