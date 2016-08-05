/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <stddef.h>

struct Interface
{
   void *context;
   void (*destroy)(void *context);
};

static inline void Interface_Destroy(struct Interface *interface)
{
   if (interface)
   {
      if (interface->destroy)
         interface->destroy(interface->context);
      interface->context = NULL;
   }
}

#endif /* __INTERFACE_H__ */
