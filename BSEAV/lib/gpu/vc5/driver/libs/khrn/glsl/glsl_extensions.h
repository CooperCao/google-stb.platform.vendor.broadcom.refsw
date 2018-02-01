/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>
#include <stdint.h>

enum glsl_ext {
    /*
     * Extensions go in this part of the list and are terminated by
     * GLSL_EXT_COUNT
     */
    GLSL_EXT_IMAGE_EXTERNAL,
    GLSL_EXT_BRCM_TEXTURE_1D,
    GLSL_EXT_SAMPLER_FETCH,
    GLSL_EXT_GATHER_LOD,
    GLSL_EXT_TEXTURE_LOD,
    GLSL_EXT_DERIVATIVES,
    GLSL_EXT_INTEGER_MIX,
    GLSL_EXT_TEXTURE_2DMSARRAY,
    GLSL_EXT_IMAGE_ATOMIC,
    GLSL_EXT_BOUNDING_BOX_OES,
    GLSL_EXT_BOUNDING_BOX_EXT,
    GLSL_EXT_BLEND_EQUATION_ADVANCED,

    GLSL_EXT_READ_DEPTH_STENCIL,

    GLSL_EXT_SAMPLE_VARIABLES,
    GLSL_EXT_MS_INTERPOLATION,

    GLSL_EXT_TEXTURE_BUFFER,
    GLSL_EXT_GPU_SHADER5,
    GLSL_EXT_CUBE_MAP_ARRAY,
    GLSL_EXT_IO_BLOCKS,
    GLSL_EXT_TESSELLATION,
    GLSL_EXT_TESSELLATION_POINT_SIZE,
    GLSL_EXT_GEOMETRY,
    GLSL_EXT_GEOMETRY_POINT_SIZE,

    GLSL_EXT_NOPERSPECTIVE,
    GLSL_EXT_IMAGE_FORMATS,

    GLSL_EXT_AEP,

    GLSL_EXT_COUNT,

    GLSL_EXT_ALL,
    GLSL_EXT_NOT_SUPPORTED,
};

#define GLSL_EXT_MAX_ID_COUNT 2u

enum glsl_ext_status {
    GLSL_DISABLED,
    GLSL_ENABLED,
    GLSL_ENABLED_WARN,    /* enabled but issue warnings */
};

/* Reset all extensions to disabled, ready to start compilation */
void glsl_ext_init(void);

/* warn means generate a warning whenever the extension is detectably used.  */
extern void glsl_ext_enable (enum glsl_ext extension, bool warn);
extern void glsl_ext_disable(enum glsl_ext extension);

extern enum glsl_ext_status glsl_ext_status(enum glsl_ext extension);

/* GLSL_EXT_NOT_SUPPORTED if we can't find it */
extern enum glsl_ext glsl_ext_lookup(const char *identifier);

/* Return the the id'th identifier of the ext'th extension. */
const char *glsl_ext_get_identifier(unsigned ext, unsigned id);

/* Return the stdlib mask needed for all enabled extensions */
extern uint64_t glsl_ext_get_symbol_mask(void);
