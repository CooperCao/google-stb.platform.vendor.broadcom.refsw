/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef KHRN_VCHIQ_H
#define KHRN_VCHIQ_H

#include "vchiq.h"

static inline int khrn_vchiq_open_service(
   VCHIQ_INSTANCE_T instance,
   VCHIQ_SERVICE_PARAMS_T *params,
   VCHIQ_CALLBACK_T callback,
   VCHIQ_SERVICE_HANDLE_T *pservice)
{
   params->callback = callback;
   return vchiq_open_service(instance, params, pservice);
}

#endif
