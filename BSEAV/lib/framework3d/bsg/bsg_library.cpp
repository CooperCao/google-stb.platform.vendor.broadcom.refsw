/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
