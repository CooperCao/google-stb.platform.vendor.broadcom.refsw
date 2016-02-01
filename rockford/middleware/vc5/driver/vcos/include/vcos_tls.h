/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver

FILE DESCRIPTION
VideoCore OS Abstraction Layer - thread local storage
=============================================================================*/

#ifndef VCOS_TLS_H
#define VCOS_TLS_H

#include "vcos_types.h"
#include "vcos_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Create a new thread local storage data key visible to all threads in
  * the current process.
  *
  * @param key    The key to create
  */
static inline VCOS_STATUS_T vcos_tls_create(VCOS_TLS_KEY_T *key, void(*destructor)(void *));

/** Delete an existing TLS data key.
  */
static inline void vcos_tls_delete(VCOS_TLS_KEY_T tls);

/** Set the value seen by the current thread.
  *
  * @param key    The key to update
  * @param v      The value to set for the current thread.
  *
  * @return VCOS_SUCCESS, or VCOS_ENOMEM if memory for this slot
  * could not be allocated.
  *
  * If TLS is being emulated via VCOS then the memory required
  * can be preallocated at thread creation time
  */
static inline VCOS_STATUS_T vcos_tls_set(VCOS_TLS_KEY_T tls, void *v);

/** Get the value for the current thread.
  *
  * @param key    The key to update
  *
  * @return The current value for this thread.
  */
static inline void *vcos_tls_get(VCOS_TLS_KEY_T tls);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace vcos
{

class tls
{
   VCOS_TLS_KEY_T m_key;

   /* Non-copyable */
   tls(const tls &);
   tls &operator=(const tls &);

public:

   tls() { throw_if_error(vcos_tls_create(&m_key, NULL)); }
   ~tls() { vcos_tls_delete(m_key); }

   void set(void *value) { throw_if_error(vcos_tls_set(m_key, value)); }
   void *get() const { return vcos_tls_get(m_key); }
};

}

#endif

#endif
