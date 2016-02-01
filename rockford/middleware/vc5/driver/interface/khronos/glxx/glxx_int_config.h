/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Declaration of implementation-dependent configuration properties.
=============================================================================*/

#ifndef GLXX_INT_CONFIG_H
#define GLXX_INT_CONFIG_H

#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_limits.h"

#define GL20_CONFIG_MAX_UNIFORM_VECTORS          300           /* Must be at least 246 to run Electopia */
#define GL20_CONFIG_MAX_VARYING_VECTORS           16

#define GL20_CONFIG_MAX_UNIFORM_SCALARS (GL20_CONFIG_MAX_UNIFORM_VECTORS*4)
#define GL20_CONFIG_MAX_VARYING_SCALARS (GL20_CONFIG_MAX_VARYING_VECTORS*4)

#define GL20_CONFIG_MAX_UNIFORM_LOCATIONS GL20_CONFIG_MAX_UNIFORM_SCALARS

#define GL20_CONFIG_MIN_ALIASED_POINT_SIZE              0.125f
#define GL20_CONFIG_MAX_ALIASED_POINT_SIZE            256.0f
#define GL20_CONFIG_MIN_ALIASED_LINE_WIDTH              0.125f
#define GL20_CONFIG_MAX_ALIASED_LINE_WIDTH             32.0f

#define GL20_CONFIG_MAX_VERTEX_TEXTURE_UNITS        16
#define GL20_CONFIG_MAX_FRAGMENT_TEXTURE_UNITS      16
#define GLXX_CONFIG_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 16
#define GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS      48
#define GLXX_CONFIG_MAX_TEXTURE_UNITS               48

#define GLXX_CONFIG_MAX_IMAGE_UNITS                4
#define GLXX_CONFIG_MAX_VERTEX_IMAGE_UNIFORMS      0
#define GLXX_CONFIG_MAX_FRAGMENT_IMAGE_UNIFORMS    4
#define GLXX_CONFIG_MAX_COMPUTE_IMAGE_UNIFORMS     4
#define GLXX_CONFIG_MAX_COMBINED_IMAGE_UNIFORMS    4

#define GLXX_CONFIG_MAX_COMPUTE_ATOMIC_COUNTERS         8
#define GLXX_CONFIG_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS  1
#define GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTERS          0
#define GLXX_CONFIG_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS   0
#define GLXX_CONFIG_MAX_FRAGMENT_ATOMIC_COUNTERS        8
#define GLXX_CONFIG_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 1
#define GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTERS        8
#define GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 1
#define GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS  4
#define GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_SIZE      32
#define GLXX_CONFIG_MAX_COMBINED_SHADER_OUTPUTS         (GLXX_CONFIG_MAX_IMAGE_UNITS + GLXX_MAX_RENDER_TARGETS)

#define GLXX_CONFIG_MAX_VERTEX_ATTRIBS            16
/* Bindings are flattened at draw time so there is no separate binding limit */
#define GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS    (GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
/* These are the minimums for GLES3.1, we have a full 32bits for the stride in
 * the command list shader record format so could go higher if required */
#define GLXX_CONFIG_MAX_VERTEX_ATTRIB_STRIDE      2048
/* There is no separate limitation on the relative offset of a vertex attribute
 * as each attribute has a separate base pointer (offset as appropriate) */
#define GLXX_CONFIG_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET (GLXX_CONFIG_MAX_VERTEX_ATTRIB_STRIDE - 1)

#define GLXX_CONFIG_MAX_SAMPLES                    4
#define GLXX_CONFIG_SUBPIXEL_BITS                  4
#define GLXX_CONFIG_MAX_SAMPLE_WORDS               1

#if KHRN_GLES31_DRIVER
#define GLXX_CONFIG_MAX_INTEGER_SAMPLES            4
#else
#define GLXX_CONFIG_MAX_INTEGER_SAMPLES            0
#endif
static_assrt(GLXX_CONFIG_MAX_INTEGER_SAMPLES <= GLXX_CONFIG_MAX_SAMPLES);

#define GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE        4096

static_assrt(GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE <= V3D_MAX_CLIP_WIDTH);
static_assrt(GLXX_CONFIG_MAX_FRAMEBUFFER_SIZE <= V3D_MAX_CLIP_HEIGHT);

// TODO These are currently from the spec. Figure out
//      our real limits
#define GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS  64
#define GLXX_CONFIG_MAX_TF_SEPARATE_ATTRIBS         4
#define GLXX_CONFIG_MAX_TF_SEPARATE_COMPONENTS      4

#define GLXX_MAX_RENDER_TARGETS  4
static_assrt(GLXX_MAX_RENDER_TARGETS <= V3D_MAX_RENDER_TARGETS);

#define GLXX_CONFIG_MAX_ELEMENT_INDEX              0x00ffffff  // We support 24-bit indices

// These are recommended values, not maximum values.
// Greater values will have less performance
#define GLXX_CONFIG_RECOMMENDED_ELEMENTS_INDICES   GLXX_CONFIG_MAX_ELEMENT_INDEX // All perform the same
#define GLXX_CONFIG_RECOMMENDED_ELEMENTS_VERTICES  0xffff      // shorts are faster than ints

// Shader supplies mipmap level directly, so there is no limit
#define GLXX_CONFIG_MAX_TEXTURE_LOD_BIAS           16.0f

#define GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS    36
#define GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE         16384

#define GLXX_CONFIG_MAX_VERTEX_SSBOS    0
#define GLXX_CONFIG_MAX_FRAGMENT_SSBOS  8
#define GLXX_CONFIG_MAX_COMPUTE_SSBOS   8
#define GLXX_CONFIG_MAX_COMBINED_SSBOS  8

#define GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS           8
#define GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCK_SIZE           (1<<27)
#define GLXX_CONFIG_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT      16

#define GLXX_CONFIG_MAX_VERTEX_UNIFORM_BLOCKS       12
#define GLXX_CONFIG_MAX_FRAGMENT_UNIFORM_BLOCKS     12
#define GLXX_CONFIG_MAX_COMPUTE_UNIFORM_BLOCKS      12
#define GLXX_CONFIG_MAX_COMBINED_UNIFORM_BLOCKS     24
#define GLXX_CONFIG_UNIFORM_BUFFER_OFFSET_ALIGNMENT 16

#define GLXX_CONFIG_MIN_TEXEL_OFFSET               -8
#define GLXX_CONFIG_MAX_TEXEL_OFFSET                7

#define GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT            65535
#define GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_X             128
#define GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Y             128
#define GLXX_CONFIG_MAX_COMPUTE_GROUP_SIZE_Z              64
#define GLXX_CONFIG_MAX_COMPUTE_WORK_GROUP_INVOCATIONS   128
#define GLXX_CONFIG_MAX_COMPUTE_SHARED_MEM_SIZE        16384

#define GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH      1024
#define GLXX_CONFIG_MAX_DEBUG_LOGGED_MESSAGES       16
#define GLXX_CONFIG_MAX_DEBUG_GROUP_STACK_DEPTH     64
#define GLXX_CONFIG_MAX_LABEL_LENGTH               256

/*
See Table 6.27. Implementation Dependent Values.
See tables 6.31, 6.32 and 6.33 for vertex, fragment and combined shader limits

Texture limits are given in glxx_texture_defines.h:

COMPRESSED_TEXTURE_FORMATS          -
NUM_COMPRESSED_TEXTURE_FORMATS     10

TODO: Not sure about these:
PROGRAM_BINARY_FORMATS              -
NUM_PROGRAM_BINARY_FORMATS          0
SHADER_BINARY_FORMATS               -
NUM_SHADER_BINARY_FORMATS           0
MAX_SERVER_WAIT_TIMEOUT             0

*/
#endif
