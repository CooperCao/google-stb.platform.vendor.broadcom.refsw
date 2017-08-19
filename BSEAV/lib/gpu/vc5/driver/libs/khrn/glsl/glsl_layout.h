/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>
#include "glsl_common.h"      /* LayoutQualifier definition */

typedef enum {
   LQ_FLAVOUR_PLAIN,
   LQ_FLAVOUR_INT,
   LQ_FLAVOUR_UINT
} LQFlavour;

typedef enum {
   LQ_RGBA32F,
   LQ_RGBA16F,
   LQ_R32F,
   LQ_RGBA8,
   LQ_RGBA8_SNORM,
   LQ_RG32F,
   LQ_RG16F,
   LQ_R11G11B10F,
   LQ_R16F,
   LQ_RGBA16,
   LQ_RGB10A2,
   LQ_RG16,
   LQ_RG8,
   LQ_R16,
   LQ_R8,
   LQ_RGBA16_SNORM,
   LQ_RG16_SNORM,
   LQ_RG8_SNORM,
   LQ_R16_SNORM,
   LQ_R8_SNORM,
   LQ_RGBA32I,
   LQ_RGBA16I,
   LQ_RGBA8I,
   LQ_R32I,
   LQ_RG32I,
   LQ_RG16I,
   LQ_RG8I,
   LQ_R16I,
   LQ_R8I,
   LQ_RGBA32UI,
   LQ_RGBA16UI,
   LQ_RGBA8UI,
   LQ_R32UI,
   LQ_RGB10A2UI,
   LQ_RG32UI,
   LQ_RG16UI,
   LQ_RG8UI,
   LQ_R16UI,
   LQ_R8UI,
   LQ_SHARED,
   LQ_PACKED,
   LQ_STD140,
   LQ_STD430,
   LQ_ROW_MAJOR,
   LQ_COLUMN_MAJOR,
   LQ_EARLY_FRAGMENT_TESTS,
   LQ_BINDING,
   LQ_OFFSET,
   LQ_LOCATION,
   LQ_SIZE_X,
   LQ_SIZE_Y,
   LQ_SIZE_Z,
   LQ_TRIANGLES,
   LQ_QUADS,
   LQ_ISOLINES,
   LQ_SPACING_EQUAL,
   LQ_SPACING_FRACTIONAL_EVEN,
   LQ_SPACING_FRACTIONAL_ODD,
   LQ_CW,
   LQ_CCW,
   LQ_POINT_MODE,
   LQ_VERTICES,
   LQ_POINTS,
   LQ_LINES,
   LQ_LINES_ADJACENCY,
   LQ_TRIANGLES_ADJACENCY,
   LQ_INVOCATIONS,
   LQ_LINE_STRIP,
   LQ_TRIANGLE_STRIP,
   LQ_MAX_VERTICES,
   LQ_BLEND_SUPPORT_MULTIPLY,
   LQ_BLEND_SUPPORT_SCREEN,
   LQ_BLEND_SUPPORT_OVERLAY,
   LQ_BLEND_SUPPORT_DARKEN,
   LQ_BLEND_SUPPORT_LIGHTEN,
   LQ_BLEND_SUPPORT_COLORDODGE,
   LQ_BLEND_SUPPORT_COLORBURN,
   LQ_BLEND_SUPPORT_HARDLIGHT,
   LQ_BLEND_SUPPORT_SOFTLIGHT,
   LQ_BLEND_SUPPORT_DIFFERENCE,
   LQ_BLEND_SUPPORT_EXCLUSION,
   LQ_BLEND_SUPPORT_HSL_HUE,
   LQ_BLEND_SUPPORT_HSL_SATURATION,
   LQ_BLEND_SUPPORT_HSL_COLOR,
   LQ_BLEND_SUPPORT_HSL_LUMINOSITY,
   LQ_BLEND_SUPPORT_ALL_EQUATIONS,
} LQ;

typedef struct {
   LQFlavour flavour;
   LQ id;
   uint32_t argument;
} LayoutID;

typedef struct layout_id_list {
   LayoutID *l;
   struct layout_id_list *next;
} LayoutIDList;

LayoutIDList *glsl_lq_id_list_new(LayoutID *id);
LayoutIDList *glsl_lq_id_list_append(LayoutIDList *l, LayoutID *id);

LayoutQualifier *glsl_layout_create(const LayoutIDList *l);

struct layout_data {
   const char *name;
   LQ lq;
};

const struct layout_data *glsl_layout_lookup(const char *name, unsigned int len);
AdvancedBlendQualifier    glsl_lq_to_abq(LQ lq);
