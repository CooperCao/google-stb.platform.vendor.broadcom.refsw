/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PLATFORM_COMMON__
#define __PLATFORM_COMMON__

#define UNUSED(X) (void)X

#ifndef NDEBUG
#define debug_printf printf
#else
#define debug_printf 1 ? 0 : printf
#endif

#endif /* __PLATFORM_COMMON__ */
