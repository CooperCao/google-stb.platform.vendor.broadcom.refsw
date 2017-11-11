/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "nexus_platform.h"
#include "nexus_display.h"
#include "default_nexus.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
   eInitFailed = 0,
   eInitSuccess
}
eInitResult;

extern eInitResult InitPlatformAndDefaultDisplay(NEXUS_DISPLAYHANDLE *handle, float *aspect, uint32_t w, uint32_t h, bool secure);
extern eInitResult InitPlatform(bool secure);
extern void TermPlatform(NEXUS_DISPLAYHANDLE handle);
extern NEXUS_DisplayHandle OpenDisplay(NEXUS_VideoFormat format, uint32_t w, uint32_t h, bool secure);
extern void InitPanelOutput(NEXUS_DisplayHandle display);
extern void InitComponentOutput(NEXUS_DisplayHandle display);
extern void InitCompositeOutput(NEXUS_DisplayHandle display, uint32_t w, uint32_t h);
extern void InitHDMIOutput(NEXUS_DisplayHandle display);

#ifdef __cplusplus
}
#endif
