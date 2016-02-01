/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_EXTENSIONS_H
#define GLSL_EXTENSIONS_H
#include "interface/khronos/common/khrn_int_common.h"

enum glsl_ext {
    /*
     * Extensions go in this part of the list and are terminated by
     * GLSL_EXT_COUNT
     */
    GLSL_EXT_IMAGE_EXTERNAL,
    GLSL_EXT_COUNT,

    GLSL_EXT_ALL,
    GLSL_EXT_NOT_SUPPORTED,
};

enum glsl_ext_status {
    GLSL_DISABLED,
    GLSL_ENABLED,
    GLSL_ENABLED_WARN,    /* enabled but issue warnings */
};

/*
 * Returns false if we don't support it anyway, incluing ALL (you never
 * support ALL-- it includes things in the future).
 *
 * warn means generate a warning whenever the extension is detectably used.
 */
extern bool glsl_ext_enable(enum glsl_ext extension, bool warn);

/*
 * Returns false if extension is specific (i.e. not just ALL) and we don't
 * support it anyway.
 */
extern bool glsl_ext_disable(enum glsl_ext extension);

extern enum glsl_ext_status glsl_ext_status(enum glsl_ext extension);

/* GLSL_EXT_NOT_SUPPORTED if we can't find it */
extern enum glsl_ext glsl_ext_lookup(const char *identifier);

/*
 * Returns code for shaders to "#include" when they see the #extension
 * directive, or NULL if there isn't any.
 */
extern const char *glsl_ext_contents(enum glsl_ext extension);

/* Return the *i'th identifier or NULL, incrementing *i */
extern const char *glsl_ext_get_identifier(int *i);

#endif  /* GLSL_EXTENSIONS_H */
