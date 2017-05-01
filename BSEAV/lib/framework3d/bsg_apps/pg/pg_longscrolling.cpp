/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "pg_longscrolling.h"
#include "pg_gui_display.h"
#include "pg.h"
#include "bsg_animator.h"
#include "bsg_shape.h"

using namespace pg;
using namespace bsg;

static bool s_peelOffLongScrollAnim = true;

LongScrolling::LongScrolling() :
   m_pTheApp(0),
   m_pController(0),
   m_scrollingAnimated(New),
   m_pgScrollingStartTex(New),
   m_pgScrollingEndTex(New),
   m_pCallBackModelViewMatrix(0),
   m_programWindowWidth(0),
   m_programWindowHeight(0),
   m_scrollActive(false),
   m_animating(false)
{

}

LongScrolling::~LongScrolling(void)
{

}

void LongScrolling::Init(App *theApp,
                           pg::Controller *controller,
                           const bsg::Mat4 &projectionMatrix,
                           float programWindowWidth,
                           float programWindowHeight)
{
   m_pTheApp = theApp;
   m_pController = controller;
   m_projectionMat = projectionMatrix;
   m_programWindowWidth = programWindowWidth;
   m_programWindowHeight = programWindowHeight;

   m_screen_height = m_pTheApp->GetOptions().GetHeight();
   m_screen_width = m_pTheApp->GetOptions().GetWidth();

   // The CallBack object is own by the Node so the pointer should be
   // deleted by the SceneNodeHandle
   m_pCallBackModelViewMatrix = new (MyCallbackModelViewMatrix);
   m_pCallBackModelViewMatrix->SetLongScrolling(this);
   m_pController->SetProgramsNodeCallBack(m_pCallBackModelViewMatrix);

   AddSceneNodes();
}

void LongScrolling::AddSceneNodes()
{
   // Scene Graph node containing all the nodes used for animation
   bsg::SceneNodeHandle & scrollingNode = m_pController->GetLongScrollingNode();

   // Objects for the start region
   EffectHandle   pgScrollingStartEffect(New);
   pgScrollingStartEffect->Load("pg_longscrolling_start.bfx");
   m_pgScrollingStartTex->SetAutoMipmap(false);
   MaterialHandle pgScrollingStartMaterial(New, "PgLongScrollingStartPanel");
   pgScrollingStartMaterial->SetEffect(pgScrollingStartEffect);
   pgScrollingStartMaterial->SetTexture("u_tex", m_pgScrollingStartTex);

   // Object for the end region
   EffectHandle   pgScrollingEndEffect(New);
   pgScrollingEndEffect->Load("pg_longscrolling_end.bfx");
   m_pgScrollingEndTex->SetAutoMipmap(false);
   MaterialHandle pgScrollingEndMaterial(New, "PgLongScrollingEndPanel");
   pgScrollingEndMaterial->SetEffect(pgScrollingEndEffect);
   pgScrollingEndMaterial->SetTexture("u_tex", m_pgScrollingEndTex);

   // Geometry used to display the long scrolling
   GeometryHandle scrollingStartRect = QuadFactory(Vec2(-m_programWindowWidth/2, -m_programWindowHeight/2),
                                            Vec2( m_programWindowWidth/2,  m_programWindowHeight/2 + ChannelStride() * 1.0f),
                                            0.0f, eZ_AXIS).MakeGeometry(pgScrollingStartMaterial);

   // This object is created according to the type of animation selected
   GeometryHandle scrollingEndRect;
   if (!s_peelOffLongScrollAnim)
   {
      scrollingEndRect = QuadFactory(Vec2(3 * -(m_programWindowWidth/2), -m_programWindowHeight/2),
                                               Vec2( -m_programWindowWidth/2,  m_programWindowHeight/2 + ChannelStride() * 1.0f),
                                               0.0f, eZ_AXIS).MakeGeometry(pgScrollingEndMaterial);
      scrollingEndRect->SetSortPriority(OVERLAY);
   }

   scrollingStartRect->SetSortPriority(OVERLAY);

   // Add object to the animated scene node
   scrollingNode->AppendChild(m_scrollingAnimated);
   m_scrollingAnimated->AppendGeometry(scrollingStartRect);
   if (!s_peelOffLongScrollAnim)
       m_scrollingAnimated->AppendGeometry(scrollingEndRect);

   // Make this scrolling geometry invisible
   scrollingNode->SetOpacity(0);
}

void LongScrolling::Start(bsg::KeyEvent::eKeyCode keyScrollType)
{
   if (!m_scrollActive)
   {
      m_scrollActive = true;
      m_animating = false;
      m_keyScrollType = keyScrollType;
   }
}

void LongScrolling::Stop()
{
   m_scrollActive = false;
   m_animating = false;
   m_pController->DisplayLongScrolling(false);

   // Reset the position of the object used for the animation
   m_scrollingAnimated->SetPosition(Vec3(0.0));

   m_keyScrollType = bsg::KeyEvent::KEY_NONE;
}

void LongScrolling::CopyToTexture(bool copyStartScreen)
{
   //TODO: These calculations should be only carried out once
   // at the begining of the animation (currently done twice)

   // Vectors in model space
   Vec4 tl_corner(-m_programWindowWidth/2, m_programWindowHeight/2 + ChannelStride() * 1.0f, 0.0, 1.0);
   Vec4 br_corner(m_programWindowWidth/2, -m_programWindowHeight/2, 0.0, 1.0);

   // Vectors in view space
   Vec4 tl_corner_view = m_projectionMat * m_programsModelViewMat * tl_corner;
   Vec4 br_corner_view = m_projectionMat * m_programsModelViewMat * br_corner;

   // Vector in homogenous view space
   bsg::Vec3 tl_corner_view_proj(tl_corner_view.Proj());
   bsg::Vec3 br_corner_view_proj(br_corner_view.Proj());

   // Vectors in screen space
   bsg::Vec2 tl_screen(m_screen_width  * (tl_corner_view_proj.X() + 1 ) / 2, m_screen_height  * (tl_corner_view_proj.Y() + 1 ) / 2);
   bsg::Vec2 br_screen(m_screen_width  * (br_corner_view_proj.X() + 1 ) / 2, m_screen_height  * (br_corner_view_proj.Y() + 1 ) / 2);

   // Size of the area of interest in the frame buffer
   float view_prog_width = br_screen.X() - tl_screen.X();
   float view_prog_height = tl_screen.Y() - br_screen.Y();

   // Copy an area of the frame buffer into one of the two textures
   if (copyStartScreen)
   {
      m_pgScrollingStartTex->CopyTexImage(uint32_t(tl_screen.X()),
                                             uint32_t(br_screen.Y()),
                                             uint32_t(view_prog_width)+1,
                                             uint32_t(view_prog_height)+1);
   }
   else
   {
      m_pgScrollingEndTex->CopyTexImage(uint32_t(tl_screen.X()),
                                             uint32_t(br_screen.Y()),
                                             uint32_t(view_prog_width+1),
                                             uint32_t(view_prog_height+1));
   }
}

bsg::AnimBindingBase * LongScrolling::RenderScene(bsg::SceneNodeHandle rootNode,
                                                      GUIDisplay &guiDisplay,
                                                      bsg::Time now,
                                                      float activeRegionWidth,
                                                      AnimationDoneNotifier *notifier)
{
   AnimBindingHermiteVec3 *animStart = NULL;

   if (m_animating)
   {
      // Normal rendering while animating
      m_pTheApp->RenderSceneGraph(rootNode);
   }
   else
   {
      // Preparing the animation

      // Check if the GUI is rotate as it has to be not rotated to grab the texture
      bool guiRotated = guiDisplay.IsRotated();
      if (guiRotated)
         guiDisplay.ResetRotation();

      // Remove the background to get the textures
      m_pTheApp->BackdropVisible(false);

      // Render the tree without any program selected
      m_pTheApp->RenderSceneGraph(rootNode);

      // Copy the part containing the programs into a tex
      bool startScreen = true;
      CopyToTexture(startScreen);

      // Change the active region to programs displayed after the scroll
      m_pTheApp->UpdateRegions(m_keyScrollType);

      // if it is a scroll animation create a texture for the end region
      if (!s_peelOffLongScrollAnim)
      {
         glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

         // Render the destination region
         m_pTheApp->RenderSceneGraph(rootNode);

         // Copy the part containing the programs into a tex
         startScreen = false;
         CopyToTexture(startScreen);

         // Display the object containing the textures to scroll
         bool displayLongScrolling = true;
         m_pController->DisplayLongScrolling(displayLongScrolling);
      }
      else
      {
         // Show overlaid start region which is going to be animated
         bsg::SceneNodeHandle & scrollingNode = m_pController->GetLongScrollingNode();
         scrollingNode->SetOpacity(1);
      }

      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Restore the position of the GUI
      if (guiRotated)
         guiDisplay.Rotate();

      // Set the backgroup visible
      m_pTheApp->BackdropVisible(true);

      // Render the first frame of the animation
      m_pTheApp->RenderSceneGraph(rootNode);

      // Create the animation
      animStart = new AnimBindingHermiteVec3(&m_scrollingAnimated->GetPosition());
      animStart->Interpolator()->Init(now, now + Metrics::GetPageScrollTime(), BaseInterpolator::eLIMIT, notifier);
      animStart->Evaluator()->Init(Vec3(0.0f, 0.0f, 0.0f), Vec3(activeRegionWidth, 0.0f, 0.0f));

      m_animating = true;
   }

   return animStart;
}

void LongScrolling::SetPeelOffAnimation(bool peeloff)
{
   s_peelOffLongScrollAnim = peeloff;
}
