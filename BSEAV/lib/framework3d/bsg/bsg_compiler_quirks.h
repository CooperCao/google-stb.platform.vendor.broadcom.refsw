/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_COMPILER_QUIRKS_H__
#define __BSG_COMPILER_QUIRKS_H__

#include "bsg_common.h"

// The BSG_NO_RETURN must be defined before we #include "bsg_exception.h"
#if (defined(__GNUC__) || defined(__clang__))
#define BSG_NO_RETURN  __attribute__((noreturn))
#else
#define BSG_NO_RETURN
#endif

//! When passing a temporary object by const reference, according to the strict letter of
//! C++ law, the object is copied.  However, the copy can be (and usually is) elided.
//! Even if elided, the compiler should respect the visibility of the copy constructor.
//! This behaviour is not useful and not what most modern compilers do.  In anticipation
//! of a revised standard, G++ from version 4.3 does not warn about the visibility of
//! a copy constructor in these cases.
//!
//! The macro is used to declare a private copy constructor and assignment operator.
//! However for G++ < 4.3 the copy constructor is made public to avoid
//! compile errors.  However, this does have the unfortunate effect of allowing copies on these
//! platforms, so the copy constructor has an asserts.in it to alert of problems.
//!
#if defined(__GNUC__)
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
#define BSG_NO_COPY(X) private: X(const X &); X &operator=(const X &);
#else
#include "bsg_exception.h"

#define BSG_NO_COPY(X) public: X(const X &) { BSG_THROW("Attempt to copy a 'no copy' object"); } private: X &operator=(const X &);
#endif
#else
#define BSG_NO_COPY(X) private: X(const X &); X &operator=(const X &);
#endif

#endif
