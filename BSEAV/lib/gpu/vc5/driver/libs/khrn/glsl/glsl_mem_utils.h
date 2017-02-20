/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#define MEMORY_MAX_ALIGNMENT 8

#define aligned_sizeof(s) (aligned_size(sizeof(s)))
#define aligned_size(s)   (((s) + (MEMORY_MAX_ALIGNMENT-1)) & ~(MEMORY_MAX_ALIGNMENT-1))
