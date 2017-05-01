/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_TYPES_H
#define KHRN_TYPES_H
#include <stdint.h>

typedef struct khrn_resource khrn_resource;
typedef struct khrn_render_state khrn_render_state;
typedef struct glxx_hw_render_state GLXX_HW_RENDER_STATE_T;

/*
 * Set of render states. Since there are not very many of them, this can be
 * just be represented with bits with zero being the empty set.
 */
typedef uint16_t khrn_render_state_set_t;

/*
 * Set of all render states.
 */
static const khrn_render_state_set_t KHRN_RENDER_STATE_SET_ALL = ~0;

#endif /* KHRN_TYPES_H */
