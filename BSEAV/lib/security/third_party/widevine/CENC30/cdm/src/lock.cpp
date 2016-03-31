// Copyright 2015 Google Inc. All Rights Reserved.
//
// Lock class - provides a simple mutex implementation.

#include "lock.h"

#include <pthread.h>

namespace wvcdm {

class Lock::Impl {
 public:
  pthread_mutex_t mutex;
};

Lock::Lock() : impl_(new Lock::Impl()) {
  pthread_mutex_init(&impl_->mutex, NULL);
}

Lock::~Lock() {
  pthread_mutex_destroy(&impl_->mutex);
  delete impl_;
}

void Lock::Acquire() {
  pthread_mutex_lock(&impl_->mutex);
}

void Lock::Release() {
  pthread_mutex_unlock(&impl_->mutex);
}

}  // namespace wvcdm
