/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PG_LONGSCROLLING_H__
#define __PG_LONGSCROLLING_H__

#include "bsg_matrix.h"
#include "pg_controller.h"
#include "bsg_time.h"

#include "pg_mycallbackmodelviewmatrix.h"

namespace pg
{
   class App;
   class GUIDisplay;

   class LongScrolling
   {
   public:
      // Default Constructor
      LongScrolling();
      // Destructor
      ~LongScrolling(void);
      // Initialisation of the member variables
      // of the callback object and creation of the
      // scene graph for this object
      void Init(App *theApp,
               pg::Controller *m_controller,
               const bsg::Mat4 &projectionMatrix,
               float programWindowWidth,
               float programWindowHeight);
      // Start the rendering for the long scroll
      void Start(bsg::KeyEvent::eKeyCode keyScrollType);
      // function called at the end of the animation
      void Stop();
      // Returns true is the rendering for the long scroll
      // is started
      bool IsActive() const { return m_scrollActive; };
      // Call to render create textures and render the scene during the animation
      bsg::AnimBindingBase * RenderScene(bsg::SceneNodeHandle rootNode,
                                         GUIDisplay &guiDisplay,
                                         bsg::Time now,
                                         float activeRegionWidth,
                                         bsg::AnimationDoneNotifier *notifier);

      // Used by the Callback Scence Node
      void SetProgramsNodeWorlMat (const bsg::Mat4 &mat) { m_programsModelViewMat = mat;  };

      static void SetPeelOffAnimation(bool peeloff);
   private:
      // Constructor by copy not implemented
      LongScrolling(const LongScrolling &);
      // Assignment operator not implemented
      const LongScrolling& operator= (const LongScrolling &);

      enum eScrollStates {
         eSCROLL_NOT_ACTIVE,
         eSCROLL_RENDER_AND_COPY,
         eSCROLL_ANIMATING,
      };

      // Copy part of the frame buffer into the start or end texture
      void CopyToTexture(bool copyStartScreen);
      // Creation of the scene graph for this object
      void AddSceneNodes();


      // Pointer to the application
      App                           *m_pTheApp;
      // Pointer to the object controlling the Scene graph
      Controller                    *m_pController;
      // Scene Graph Node used to animate the scrolling
      bsg::SceneNodeHandle          m_scrollingAnimated;
      // Texture of the active region the animation start from
      bsg::TextureHandle            m_pgScrollingStartTex;
      // Texture of the active region the animation end to
      bsg::TextureHandle            m_pgScrollingEndTex;

      // Model View matrix of the node used to generate the texture
      bsg::Mat4                     m_programsModelViewMat;
      // Callback object to update the mode view matrix
      MyCallbackModelViewMatrix     *m_pCallBackModelViewMatrix;
      // Projection matrix created by the camera (only updated at initialisation)
      bsg::Mat4                     m_projectionMat;
      // Width of an active region
      float                         m_programWindowWidth;
      // Height of an active region
      float                         m_programWindowHeight;
      // Screen size
      uint32_t                      m_screen_width;
      uint32_t                      m_screen_height;
      // Flag indicating that the long scrolling start to be rendered
      bool                          m_scrollActive;
      // Flag indicating that the long scrolling is animated
      bool                          m_animating;
      // Flag to change the animation from scrolling start and end region
      // to the animation where only the start region is scrolled
      bool                          m_animPeelOffStart;
      // Key associated with the destination of the scroll
      bsg::KeyEvent::eKeyCode       m_keyScrollType;
   };

}

#endif // __PG_LONGSCROLLING_H__
