/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_render_options.h"
#include "bsg_application.h"

using namespace bsg;

void RenderOptions::SetEnableViewFrustumCull(bool b)
{
   m_enableViewFrustumCull = b;
}
