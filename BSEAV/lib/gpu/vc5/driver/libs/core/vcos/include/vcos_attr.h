/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*=============================================================================
VideoCore OS Abstraction Layer - compiler-specific attributes
=============================================================================*/

#ifndef VCOS_ATTR_H
#define VCOS_ATTR_H

/**
 * Type attribute indicating the enum should be stored in as few bytes as
 * possible. MetaWare does this by default, so the attribute is useful when
 * structs need to be portable to GCC too.
 *
 * MSVC doesn't support VCOS_ENUM_PACKED, so code that needs to be portable
 * across all platforms but wants the type-safety and debug-info benefits
 * of enum types when possible, should do:
 *
 *    typedef enum VCOS_ENUM_PACKED { a = 0, b = 0xffff } EXAMPLE_T;
 *    struct foo {
 *       int bar;
 *       #if VCOS_HAS_ENUM_PACKED
 *       EXAMPLE_T baz;
 *       #else
 *       uint16_t baz;
 *       #endif
 *    };
 */
#if defined(__GNUC__)
# define VCOS_ENUM_PACKED  __attribute__ ((packed))
# define VCOS_HAS_ENUM_PACKED 1
#elif defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ENUM_PACKED /* packed enums are default on Metaware */
#ifndef VCOS_HAS_ENUM_PACKED /* To allow override when using hcl386 */
# define VCOS_HAS_ENUM_PACKED 1
#endif
#else
# define VCOS_ENUM_PACKED
# define VCOS_HAS_ENUM_PACKED 0
#endif

#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
#define VCOS_ATTR_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define VCOS_ATTR_NORETURN __declspec(noreturn)
#else
#define VCOS_ATTR_NORETURN
#endif

/** Variable attribute indicating the variable must be emitted even if it appears unused. */
#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ATTR_USED  __attribute__ ((used))
#else
# define VCOS_ATTR_USED
#endif

/** Variable attribute indicating the compiler should not warn if the variable is unused. */
#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ATTR_POSSIBLY_UNUSED  __attribute__ ((unused))
#else
# define VCOS_ATTR_POSSIBLY_UNUSED
#endif

/** Variable attribute requiring specific alignment.
 *
 * Use as:
 *   int VCOS_ATTR_ALIGNED(256) n;
 * or:
 *   VCOS_ATTR_ALIGNED(256) int n;
 * or if you don't want to support MSVC:
 *   int n VCOS_ATTR_ALIGNED(256);
 */
#if defined(__GNUC__) || defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_ATTR_ALIGNED(n)  __attribute__ ((aligned(n)))
#elif defined(_MSC_VER)
# define VCOS_ATTR_ALIGNED(n)  __declspec(align(n))
#else
/* Force a syntax error if this is used when the compiler doesn't support it,
 * instead of silently misaligning */
# define VCOS_ATTR_ALIGNED(n) VCOS_ATTR_ALIGNED_NOT_SUPPORTED_ON_THIS_COMPILER
#endif

/** Max alignment reported by vcos_alignof() for any type, ie the alignment
 * that malloc() guarantees */
#define VCOS_MAX_ALIGN 8 /* Just assume this for now */

/** Variable attribute requiring specific ELF section.
 *
 * Use as:
 *   int n VCOS_ATTR_SECTION(".foo") = 1;
 *
 * A pointer like &n will have type "VCOS_ATTR_SECTION_QUALIFIER int *".
 */
#if defined(__GNUC__)
# define VCOS_ATTR_SECTION(s)  __attribute__ ((section(s)))
# define VCOS_ATTR_SECTION_QUALIFIER
#elif defined(__HIGHC__) || defined(__VECTORC__)
/* hcvc requires 'far' else it'll put small objects in .sdata/.rsdata/.sbss */
# define VCOS_ATTR_SECTION(s)  __attribute__ ((far, section(s)))
# define VCOS_ATTR_SECTION_QUALIFIER _Far
#else
/* Force a syntax error if this is used when the compiler doesn't support it */
# define VCOS_ATTR_SECTION(s) VCOS_ATTR_SECTION_NOT_SUPPORTED_ON_THIS_COMPILER
# define VCOS_ATTR_SECTION_QUALIFIER
#endif

/** Define a function as a weak alias to another function.
 * @param ret_type     Function return type.
 * @param alias_name   Name of the alias.
 * @param param_list   Function parameter list, including the parentheses.
 * @param target_name  Target function (bare function name, not a string).
 */
#if defined(__GNUC__) || defined(__HIGHC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS(ret_type, alias_name, param_list, target_name) \
   __attribute__ ((weak, alias(#target_name))) ret_type alias_name param_list
#elif defined(__VECTORC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS(ret_type, alias_name, param_list, target_name) \
   pragma weak(alias_name, target_name)
#else
# define VCOS_WEAK_ALIAS(ret_type, alias, params, target)  static_assrt(0)
#endif

/** Define a function as a weak alias to another function, specified as a string.
 * @param ret_type     Function return type.
 * @param alias_name   Name of the alias.
 * @param param_list   Function parameter list, including the parentheses.
 * @param target_name  Target function name as a string.
 * @note Prefer the use of VCOS_WEAK_ALIAS - it is likely to be more portable.
 *       Only use VCOS_WEAK_ALIAS_STR if you need to do pre-processor mangling of the target
 *       symbol.
 */
#if defined(__GNUC__) || defined(__HIGHC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS_STR(ret_type, alias_name, param_list, target_name) \
   __attribute__ ((weak, alias(target_name))) ret_type alias_name param_list
#elif defined(__VECTORC__)
  /* N.B. gcc allows __attribute__ after parameter list, but hcvc seems to silently ignore it. */
# define VCOS_WEAK_ALIAS_STR(ret_type, alias_name, param_list, target_name) \
   pragma weak(alias_name, target_name)
#else
# define VCOS_WEAK_ALIAS_STR(ret_type, alias, params, target)  static_assrt(0)
#endif

/** Define a weak symbol
 *
 * e.g.
 *
 * VCOS_WEAK_SYM(int, foo);
 *
 */
#if defined(__GNUC__)
# define VCOS_WEAK_SYM(sym_type, sym_name) sym_type sym_name __attribute__((weak))
#elif defined(__HIGHC__) || defined(__VECTORC__)
# define VCOS_WEAK_SYM(sym_type, sym_name) \
   sym_type sym_name; \
   pragma weak(sym_name)
#else
# define VCOS_WEAK_SYM(sym_type, sym_name) static_assrt(0)
#endif

#endif /* VCOS_ATTR_H */
