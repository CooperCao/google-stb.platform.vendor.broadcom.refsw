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

#include "bsg_library.h"
#include "bsg_scene_node.h"
#include "bsg_geometry.h"
#include "bsg_camera.h"
#include "bsg_material.h"
#include "bsg_surface.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_fbo.h"
#include "bsg_font.h"
#include "bsg_print.h"
#include "bsg_constraint.h"

namespace bsg
{

// This is a syntactic token used to overload handle constructors
AllocNew New;

template<> Library<SceneNodeTraits>    *Library<SceneNodeTraits>   ::m_library = 0;
template<> Library<GeometryTraits>     *Library<GeometryTraits>    ::m_library = 0;
template<> Library<CameraTraits>       *Library<CameraTraits>      ::m_library = 0;
template<> Library<MaterialTraits>     *Library<MaterialTraits>    ::m_library = 0;
template<> Library<SurfaceTraits>      *Library<SurfaceTraits>     ::m_library = 0;
template<> Library<EffectTraits>       *Library<EffectTraits>      ::m_library = 0;
template<> Library<GLTextureTraits>    *Library<GLTextureTraits>   ::m_library = 0;
#ifndef BSG_STAND_ALONE
template<> Library<FontTraits>         *Library<FontTraits>        ::m_library = 0;
template<> Library<PrintFontTraits>    *Library<PrintFontTraits>   ::m_library = 0;
#endif
template<> Library<ConstraintTraits>   *Library<ConstraintTraits>  ::m_library = 0;
template<> Library<FramebufferTraits>  *Library<FramebufferTraits> ::m_library = 0;
template<> Library<RenderbufferTraits> *Library<RenderbufferTraits>::m_library = 0;

void Libraries::Create()
{
   Library<SceneNodeTraits>   ::CreateLibrary();
   Library<GeometryTraits>    ::CreateLibrary();
   Library<CameraTraits>      ::CreateLibrary();
   Library<MaterialTraits>    ::CreateLibrary();
   Library<SurfaceTraits>     ::CreateLibrary();
   Library<EffectTraits>      ::CreateLibrary();
   Library<GLTextureTraits>   ::CreateLibrary();
#ifndef BSG_STAND_ALONE
   Library<FontTraits>        ::CreateLibrary();
   Library<PrintFontTraits>   ::CreateLibrary();
#endif
   Library<ConstraintTraits>  ::CreateLibrary();
   Library<FramebufferTraits> ::CreateLibrary();
   Library<RenderbufferTraits>::CreateLibrary();
}

void Libraries::Destroy()
{
   Library<SceneNodeTraits>   ::DestroyLibrary();
   Library<GeometryTraits>    ::DestroyLibrary();
   Library<CameraTraits>      ::DestroyLibrary();
   Library<MaterialTraits>    ::DestroyLibrary();
   Library<SurfaceTraits>     ::DestroyLibrary();
   Library<EffectTraits>      ::DestroyLibrary();
   Library<GLTextureTraits>   ::DestroyLibrary();
#ifndef BSG_STAND_ALONE
   Library<FontTraits>        ::DestroyLibrary();
   Library<PrintFontTraits>   ::DestroyLibrary();
#endif
   Library<ConstraintTraits>  ::DestroyLibrary();
   Library<FramebufferTraits> ::DestroyLibrary();
   Library<RenderbufferTraits>::DestroyLibrary();
}

}
