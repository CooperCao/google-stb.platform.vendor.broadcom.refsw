/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/gl20/gl20_config.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include <GLES2/gl2ext.h>

extern bool gl20_server_state_init(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared, bool secure);