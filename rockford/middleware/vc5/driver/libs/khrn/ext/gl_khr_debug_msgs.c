/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Module   :  Implementation of KHR_debug extension
=============================================================================*/

#include "gl_khr_debug_msgs.h"

#include <stdlib.h>  // For NULL

static const char * const debug_messages[] =
{
   // GLXX_BAD_MESSAGE_ID = 00000
   "Bad debug message id",
   // GLXX_SLOW_IMAGE_CONVERSION = 00001
   "Slow image conversion",
   // GLXX_SUPPRESSING_SLOW_CONV_MSGS = 00002
   "Suppressing further slow unaccelerated conversion messages"
};

const char *glxx_lookup_preset_debug_message(unsigned id)
{
   if (id < GLXX_NUM_PRESET_DEBUG_MESSAGES)
      return debug_messages[id];

   return NULL;
}
