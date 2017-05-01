/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_MAP_H
#define EGL_MAP_H
#include "vcos.h"
#include <stdbool.h>
#include "egl_types.h"

typedef struct
{
   const void     *handle;
   void           *value;
}
EGL_PAIR_T;

struct egl_map
{
   EGL_PAIR_T     *members;
   size_t         count;
   size_t         max;
};

extern void egl_map_init(EGL_MAP_T *map);

extern bool egl_map_insert(EGL_MAP_T *map, const void *handle, void *value);

/* Remove an entry if the handle is found in the map and return the value for
 * the handle. Returns NULL otherwise;
 */
extern void* egl_map_remove(EGL_MAP_T *map, const void *handle);

/* Returns NULL if handle isn't in the map */
extern void *egl_map_lookup(const EGL_MAP_T *map, const void *handle);

/*
 * Returns NULL if value isn't in the map, otherwise the first handle it finds
 * with that value (which in practice will be the only one)
 */
const void *egl_map_reverse_lookup(const EGL_MAP_T *map, void *value);

/* After being destroyed, map is still valid but empty */
extern void egl_map_destroy(EGL_MAP_T *map);

/* Move the contents of src into dest, leaving src empty */
extern void egl_map_move(EGL_MAP_T *dest, EGL_MAP_T *src);

/*
 * Iterates over all the items in the map.
 *
 * Set *i to 0 to start with and keep passing it back in. Returns NULL when
 * all elements of the map have been generated.
 */
extern EGL_PAIR_T *egl_map_items(EGL_MAP_T *map, size_t *i);

/* Remove and return an arbitary pair from the map, or NULL if it's empty */
extern EGL_PAIR_T *egl_map_pop(EGL_MAP_T *map);

#endif /* EGL_MAP_H */
