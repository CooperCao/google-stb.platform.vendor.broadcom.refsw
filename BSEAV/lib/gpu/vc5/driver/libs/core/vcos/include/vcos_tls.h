/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef VCOS_TLS_H
#define VCOS_TLS_H

#include "vcos_types.h"
#include "vcos_platform.h"
#include "libs/util/demand.h"

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

   tls() { demand(vcos_tls_create(&m_key, NULL) == VCOS_SUCCESS); }
   ~tls() { vcos_tls_delete(m_key); }

   void set(void *value) { demand(vcos_tls_set(m_key, value) == VCOS_SUCCESS); }
   void *get() const { return vcos_tls_get(m_key); }
};

}

#endif

#endif
