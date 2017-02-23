/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdbool.h>

void *CreateEvent(void);
bool DestroyEvent(void *p);
bool SetEvent(void *p);
bool WaitEvent(void *p);
bool ResetEvent(void *p);

#endif /* __EVENT_H__ */
