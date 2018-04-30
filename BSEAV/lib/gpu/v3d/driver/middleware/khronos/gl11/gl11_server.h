/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/glxx/gl11_int_config.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include <GLES/glext.h>

extern bool gl11_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared, bool secure);