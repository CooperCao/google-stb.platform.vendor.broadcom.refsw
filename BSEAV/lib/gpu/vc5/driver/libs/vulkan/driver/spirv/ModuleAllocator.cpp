/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "ModuleAllocator.h"

#include "libs/util/log/log.h"

LOG_DEFAULT_CAT("spv::ModuleAllocator");

namespace spv {

void LogUsage(const char *name, const bvk::ArenaAllocator<bvk::SysMemCmdBlock, void*> &arena)
{
   size_t   sysMemBlockCount = 0;
   size_t   sysBytesAlloced = 0;
   size_t   sysBytesUsed = 0;
   size_t   sysDeletedBytesWasted = 0;

   arena.GetUsageData(&sysMemBlockCount, &sysBytesAlloced, &sysBytesUsed, &sysDeletedBytesWasted);

   log_trace("==============================================================");
   log_trace("Module Arena Allocator for %s", name);
   log_trace("System memory blocks used    : %zu", sysMemBlockCount);
   log_trace("System memory bytes alloced  : %zu", sysBytesAlloced);
   log_trace("System memory bytes used     : %zu", sysBytesUsed);
   log_trace("System wasted deleted bytes  : %zu", sysDeletedBytesWasted);
}

}