/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.  
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#define BSG_NO_NAME_MANGLING

#include "spytool_replay.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_pkm.h"
#include "bsg_exception.h"

#include "GLES2/gl2.h"

#include <iostream>
#include <istream>
#include <fstream>

using namespace bsg;

extern SpyToolReplay *gReplay;

#define SetCurrentFrameFunc gReplay->SetCurrentFrameFunc

#define AddDisplayMapping gReplay->AddDisplayMapping
#define MapDisplay gReplay->MapDisplay

#define AddSurfaceMapping gReplay->AddSurfaceMapping
#define MapSurface gReplay->MapSurface

#define AddContextMapping gReplay->AddContextMapping
#define MapContext gReplay->MapContext

#define AddConfigMapping gReplay->AddConfigMapping
#define MapConfig gReplay->MapConfig

#define AddClientBufferMapping gReplay->AddClientBufferMapping
#define MapClientBuffer gReplay->MapClientBuffer

#define AddShaderMapping gReplay->AddShaderMapping
#define MapShader gReplay->MapShader

#define AddProgramMapping gReplay->AddProgramMapping
#define MapProgram gReplay->MapProgram

#define AddUniformMapping gReplay->AddUniformMapping
#define MapUniform gReplay->MapUniform

#define SetCaptureDataFile gReplay->SetCaptureDataFile
#define MapData(N) gReplay->MapDataImp(N, __FILE__, __LINE__)

#define GetNativeWindow gReplay->GetNativeWindow
#define GetNativePixmap gReplay->GetNativePixmap

#define eglWaitClient gReplay->eglWaitClient
#define eglMakeCurrent gReplay->eglMakeCurrent

#define scratchMemPtr gReplay->ScratchMem()

#ifdef NULL_DRAWS
#undef eglSwapBuffers
#undef glDrawElements
#define eglSwapBuffers(a, b)
#define glDrawElements(a, b, c, d)
#endif
