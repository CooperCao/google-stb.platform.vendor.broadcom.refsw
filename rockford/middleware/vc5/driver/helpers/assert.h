/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef ASSERT_H
#define ASSERT_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "vcos_types.h" /* For VCOS_FUNCTION */

#ifdef __cplusplus
#define assertion_abort() ::abort()
#else
#define assertion_abort() abort()
#endif

/* This shouldn't be called directly. It is exposed for use by demand.h. */
#define assertion_failure(...)               \
   do                                        \
   {                                         \
      fprintf(stderr, "%s:%u: %s: ",         \
         __FILE__, __LINE__, VCOS_FUNCTION); \
      fprintf(stderr, __VA_ARGS__);          \
      fprintf(stderr, "\n");                 \
      assertion_abort();                     \
   } while (0)

/* assert_msg is supposed to be just like assert but it prints a custom message
 * when the assertion fails rather than just the assert condition */
#ifdef NDEBUG
#define assert_msg(COND, ...)
#else
#define assert_msg(COND, ...)             \
   do                                     \
   {                                      \
      if (!(COND))                        \
         assertion_failure(__VA_ARGS__);  \
   } while (0)
#endif

#ifdef NDEBUG
#define verif(COND)              \
   do                            \
   {                             \
      if (!(COND))               \
         builtin_unreachable();  \
   } while (0)
#else
#define verif(COND) assert(COND)
#endif

/** static_assert */

#if defined(static_assert) || (defined(__cplusplus) && \
   ((__cplusplus >= 201103l) || (defined(_MSC_VER) && (_MSC_VER >= 1600)) || \
   (defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))))))

/* Use already-defined static_assert or built-in static_assert */

#else

/* GCC >=4.6 (C only -- not C++!) */
#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))) && !defined(__cplusplus)
#  define static_assert _Static_assert
#endif

/* Clang */
#if !defined(static_assert) && defined(__has_extension)
#  if __has_extension(c_static_assert)
#     define static_assert _Static_assert
#  endif
#endif

#ifndef static_assert
#  define static_assert(COND, MESSAGE) extern int static_assert_[(COND) ? 1 : -1]
#endif

#endif

#define static_assrt(COND) static_assert(COND, #COND)

/** unreachable/not_impl */

/* GCC >=4.5 */
#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)))
#  define builtin_unreachable __builtin_unreachable
#endif

/* Clang */
#if !defined(builtin_unreachable) && defined(__has_builtin)
#  if __has_builtin(__builtin_unreachable)
#     define builtin_unreachable __builtin_unreachable
#  endif
#endif

/* MSVC */
#if !defined(builtin_unreachable) && defined(_MSC_VER)
#  define builtin_unreachable() __assume(0)
#endif

#ifndef builtin_unreachable
#  define builtin_unreachable() ((void)0)
#endif

#define unreachable() do { assert(0); builtin_unreachable(); } while (0)

#ifdef NDEBUG
#  define not_impl() do { assertion_failure("Not implemented!"); builtin_unreachable(); } while (0)
#else
#  define not_impl() unreachable()
#endif

#endif
