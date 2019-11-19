/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#ifndef __FENCE_INTERFACE_H__
#define __FENCE_INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>
#include "interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FENCE_WAIT_INFINITE 0xffffffff
#define INVALID_FENCE -1

typedef struct FenceInterface
{
   Interface base;

   void (*create)(void *context, int *fence); /* may be NULL */
   void (*destroy)(void *context, int fence); /* may be NULL if create is null*/
   bool (*keep)(void *context, int fence);    /* may be NULL if create is null*/
   bool (*wait)(void *context, int fence, uint32_t timeoutms); /* may be NULL if create is null*/
   void (*signal)(void *context, int fence);  /* may be NULL if create is null*/
} FenceInterface;

void FenceInterface_Create(const FenceInterface *fi, int *fence);

void FenceInterface_Destroy(const FenceInterface *fi,
      int *fence);

bool FenceInterface_Keep(const FenceInterface *fi, int fence);

bool FenceInterface_Wait(const FenceInterface *fi, int fence,
      uint32_t timeoutms);

void FenceInterface_Signal(const FenceInterface *fi, int fence);

void FenceInterface_WaitAndDestroy(const FenceInterface *fi,
      int *fence);

#ifdef __cplusplus
}
#endif

#endif /* __FENCE_INTERFACE_H__ */
