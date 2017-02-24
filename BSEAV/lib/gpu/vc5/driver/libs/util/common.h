/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

// EXTERN_C_*
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

// ATTRIBUTE_FORMAT
#if defined(__GNUC__) && (( __GNUC__ > 2 ) || (( __GNUC__ == 2 ) && ( __GNUC_MINOR__ >= 3 )))
#define ATTRIBUTE_FORMAT(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)  __attribute__ ((format (ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)))
#else
#define ATTRIBUTE_FORMAT(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)
#endif

// countof
#if defined(_MSC_VER)
    #include <stdlib.h>
    #define countof _countof
#elif defined(__cplusplus)
   #include <type_traits>
   template<typename Tin>
   constexpr std::size_t countof()
   {
      using T = typename std::remove_reference<Tin>::type;
      static_assert(std::is_array<T>::value, "countof() requires an array argument");
      static_assert(std::extent<T>::value > 0, "zero- or unknown-size array");
      return std::extent<T>::value;
   }
   #define countof(a) countof<decltype(a)>()
#else
   #define countof(x) (sizeof((x)) / sizeof((x)[0]))
#endif

// inline is a keyword in C99 and C++, although MSVC supports a
// good portion of C99, somehow the inline keyword missed the list
#if defined(_MSC_VER) && !defined(inline) && !defined(__cplusplus)
#define inline __inline
#endif

#ifdef NDEBUG
#define IS_DEBUG 0
#else
#define IS_DEBUG 1
#endif

#ifdef NDEBUG
#define debug_only(x) do { } while(0)
#else
#define debug_only(x) do { x; } while(0)
#endif

#ifdef _MSC_VER
#define PRIuSIZE "Iu"
#define PRIxSIZE "Ix"
#else
#define PRIuSIZE "zu"
#define PRIxSIZE "zx"
#endif