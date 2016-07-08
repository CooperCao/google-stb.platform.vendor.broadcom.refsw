/*==============================================================================
 Broadcom Proprietary and Confidential. (c)2012 Broadcom.
 All rights reserved.
==============================================================================*/

#ifndef DGLENUM_H
#define DGLENUM_H

#include <stdint.h>

extern uint32_t khrn_glenum_from_str(const char *name);
extern uint32_t khrn_glenum_from_str_maybe(const char *name);
extern const char *khrn_glenum_to_str(uint32_t value);

#endif
