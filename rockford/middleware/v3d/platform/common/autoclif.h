/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
Debug interface for internal use only
=============================================================================*/
#pragma once

#include <stdint.h>

extern void *MemDebugAutoclifAddrToPtr(void *context, uint32_t addr);
extern uint32_t MemDebugAutoclifPtrToAddr(void *context, void *p);
extern const char *MemDebugAutoclifGetClifFilename(void *context);
extern void MemDebugAutoclifReset(void *context);