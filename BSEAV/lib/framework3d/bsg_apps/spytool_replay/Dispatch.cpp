/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>

#include "Dispatch.h"

#define HOOK(f) \
   *(void **)&real_##f = (void*)eglGetProcAddress(#f);

Dispatch::Dispatch()
{
#include "apic.inc"
};

Dispatch::~Dispatch()
{
};
