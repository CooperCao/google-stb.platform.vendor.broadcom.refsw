/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Module   :  Implementation of KHR_debug extension
=============================================================================*/

#ifndef __GLXX_DEBUG_MSGS_H__
#define __GLXX_DEBUG_MSGS_H__

#include "gl_public_api.h"

typedef enum
{
   GLXX_BAD_MESSAGE_ID = 0,
   GLXX_SLOW_IMAGE_CONVERSION,
   GLXX_SUPPRESSING_SLOW_CONV_MSGS,

   GLXX_NUM_PRESET_DEBUG_MESSAGES   // Must be last
} GLXX_KHR_DEBUG_MESSAGES;

extern const char *glxx_lookup_preset_debug_message(unsigned id);

#endif /* __GLXX_DEBUG_MSGS_H__ */
