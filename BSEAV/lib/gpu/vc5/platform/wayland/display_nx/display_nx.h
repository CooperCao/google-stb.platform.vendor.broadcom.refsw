/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *WLPL_CreateNexusDisplayInterface(
      struct BEGL_SchedInterface *schedIface);
void WLPL_DestroyNexusDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif
