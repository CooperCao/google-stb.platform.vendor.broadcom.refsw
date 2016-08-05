/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_LAYOUT_H
#define GLSL_LAYOUT_H

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
   LQ_RGBA32I,
   LQ_RGBA16I,
   LQ_RGBA8I,
   LQ_R32I,
   LQ_RGBA32UI,
   LQ_RGBA16UI,
   LQ_RGBA8UI,
   LQ_R32UI,
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

#endif // LAYOUT_H
