/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bcm_help_menu.h"

#include "bsg_application.h"
#include "bsg_library.h"
#include "bsg_shape.h"

using namespace bsg;
using namespace std;

static const char *s_effectText =
   "OPTIONS                                                     \n"
   "{                                                           \n"
   "   SortOrder = BACK_TO_FRONT;                               \n"
   "}                                                           \n"
   "                                                            \n"
   "PASS 0                                                      \n"
   "{                                                           \n"
   "   SEMANTICS                                                \n"
   "   {                                                        \n"
   "      u_opacity   = SCALAR_OPACITY;                         \n"
   "      a_position    = VATTR_POSITION;                       \n"
   "      a_tc          = VATTR_TEXCOORD1;                      \n"
   "      u_textColor   = VECTOR4_USER;                         \n"
   "      u_mvp         = MATRIX4_MODEL_VIEW_PROJECTION;        \n"
   "   }                                                        \n"
   "                                                            \n"
   "   STATE                                                    \n"
   "   {                                                        \n"
   "      EnableBlend = true;                                   \n"
   "      BlendFunc = SRC_ALPHA, ONE_MINUS_SRC_ALPHA;           \n"
   "      EnableDepthTest = true;                               \n"
   "   }                                                        \n"
   "                                                            \n"
   "   SAMPLER_2D u_textSampler                                 \n"
   "   {                                                        \n"
   "      Unit = 0;                                             \n"
   "      Wrap = CLAMP, CLAMP;                                  \n"
   "      Filter = LINEAR_MIPMAP_LINEAR, LINEAR;                \n"
   "   }                                                        \n"
   "                                                            \n"
   "   VERTEX_SHADER                                            \n"
   "   {                                                        \n"
   "      uniform    float u_opacity;                           \n"
   "      attribute  vec4  a_position;                          \n"
   "      attribute  vec2  a_tc;                                \n"
   "      varying    vec2  v_tc;                                \n"
   "      uniform    mat4  u_mvp;                               \n"
   "      varying    float v_opacity;                           \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         v_opacity   = u_opacity;                           \n"
   "         gl_Position = u_mvp * a_position;                  \n"
   "         v_tc        = a_tc;                                \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "                                                            \n"
   "   FRAGMENT_SHADER                                          \n"
   "   {                                                        \n"
   "      precision mediump float;                              \n"
   "                                                            \n"
   "      uniform sampler2D   u_textSampler;                    \n"
   "      uniform vec4        u_textColor;                      \n"
   "      varying vec2        v_tc;                             \n"
   "      varying float       v_opacity;                        \n"
   "                                                            \n"
   "      void main()                                           \n"
   "      {                                                     \n"
   "         float alpha = texture2D(u_textSampler, v_tc).r;    \n"
   "         if (alpha < 0.01)                                  \n"
   "            discard;                                        \n"
   "         alpha *= v_opacity;                                \n"
   "         alpha *= u_textColor.a;                            \n"
   "         gl_FragColor = vec4(u_textColor.rgb, alpha);       \n"
   "      }                                                     \n"
   "   }                                                        \n"
   "}                                                           \n";

static EffectHandle s_textEffect;

const float DEFAULT_GAP_BETWEEN_HEADER_AND_DESCRIPTION = 10.0;    //!< Distance between the header and the description text
const float FULL_OPACITY = 1.0;                                   //!< Opacity value showing the help button or the help menu
const float NO_OPACITY = 0.0;                                     //!< Opacity value hidding the help button or the help menu
const Vec3 HIDE_SCALE(Vec3(0.0));                                 //!< Scale vector hidding the help button or the help menu
const Vec3 SHOW_SCALE(Vec3(1.0));                                 //!< Scale vector showing the help button or the help menu
const unsigned int BACKGROUND_SORT_PRIORITY = 0;                  //!< Drawing priority of the background (should less than for the menu)
const unsigned int MENU_SORT_PRIORITY = 100;                      //!< Drawing priority of the menu
const Vec4 DEFAULT_MENU_BACKGROUND_COLOUR(0.0, 0.0, 0.0, 1.0);    //!< Default colour for the background of the menu
const float SCREEN_HEIGHT_OVERSCAN = 0.9f;                        //!< Percentage of the screen height for the overscan
const float SCREEN_WIDTH_OVERSCAN = 0.9f;                         //!< Percentage of the screen width for the overscan

static uint32_t GetScreenWidth(const Application *app)
{
   if (app == 0)
      return 0;

   return app->GetWindowWidth();
}

static uint32_t GetScreenHeight(const Application *app)
{
   if (app == 0)
      return 0;

   return app->GetWindowHeight();
}

HelpMenu::HelpMenu(Application *app,
                   eHELPBUTTON button,
                   const string &helpButtonText,
                   const string &fontName,
                   const Vec4 &colourText,
                   float fontSizePercent,
                   float xPercentScreen,
                   float yPercentScreen,
                   bool  useOverscan) :
   m_app(app),
   m_rootNode(New),
   m_helpButtonNode(New),
   m_helpMenuNode(New),
   m_menuBackgroundNode(New),
   m_camera(New),
   m_font(New),
   m_buttonEffect(New),
   m_menuItemHeaderColour(colourText),
   m_menuItemTextColour(colourText),
   m_menuItemHeaderSize(0),
   m_menuItemTextSize(0),
   m_menuPercentPos(xPercentScreen, yPercentScreen),
   m_menuReferencePos(eMENU_POSITION_NONE),
   m_helpBtnPercentPos(xPercentScreen, yPercentScreen),
   m_useMenuBackground(false),
   m_menuBackgroundRoundedCorners(false),
   m_useOverscan(useOverscan),
   m_menuDiplayed(false),
   m_menuToDisplay(eMENU_NOT_FINILISED),
   m_menuPosNeedAdjusted(false),
   m_animationRunning(false),
   m_animationType(eMENU_ANIM_FADE)
{
   // Get the size of the window to adapt the menu size
   uint32_t screenHeight = GetScreenHeight(m_app);
   uint32_t screenWidth  = GetScreenWidth(m_app);

   InitMenuStructure(screenWidth, screenHeight, xPercentScreen, yPercentScreen);

   float fontSize = fontSizePercent * screenHeight;

   //Set the default text sizes for the menu item
   m_menuItemTextSize   = fontSize;
   m_menuItemHeaderSize = fontSize;

   // Create a font
   m_font->Load(fontName, fontSize);

   float imgWidth = 0;
   float textWidth = 0;
   CreateColouredButtonNode(m_helpButtonNode, button, helpButtonText,
                                 m_font, fontSize, colourText,
                                 imgWidth, textWidth);

}

HelpMenu::HelpMenu(Application *app,
                   const string &helpButtonHeaderText,
                   const string &helpButtonText,
                   const string &fontName,
                   const Vec4 &colourHeader,
                   const Vec4 &colourText,
                   float fontSizePercentHeader,
                   float fontSizePercentText,
                   float xPercentScreen,
                   float yPercentScreen,
                   bool useOverscan) :
   m_app(app),
   m_rootNode(New),
   m_helpButtonNode(New),
   m_helpMenuNode(New),
   m_menuBackgroundNode(New),
   m_camera(New),
   m_font(New),
   m_buttonEffect(New),
   m_menuItemHeaderColour(colourHeader),
   m_menuItemTextColour(colourText),
   m_menuItemHeaderSize(0),
   m_menuItemTextSize(0),
   m_menuPercentPos(xPercentScreen, yPercentScreen),
   m_menuReferencePos(eMENU_POSITION_NONE),
   m_helpBtnPercentPos(xPercentScreen, yPercentScreen),
   m_useMenuBackground(false),
   m_menuBackgroundRoundedCorners(false),
   m_useOverscan(useOverscan),
   m_menuDiplayed(false),
   m_menuToDisplay(eMENU_NOT_FINILISED),
   m_menuPosNeedAdjusted(false),
   m_animationRunning(false),
   m_animationType(eMENU_ANIM_FADE)
{
   // Get the size of the window to adapt the menu size
   uint32_t screenHeight = GetScreenHeight(m_app);
   uint32_t screenWidth  = GetScreenWidth(m_app);

   InitMenuStructure(screenWidth, screenHeight, xPercentScreen, yPercentScreen);

   m_menuItemTextSize   = fontSizePercentText   * screenHeight;
   m_menuItemHeaderSize = fontSizePercentHeader * screenHeight;

   // Create a font
   if (m_menuItemTextSize < m_menuItemHeaderSize)
      m_font->Load(fontName, m_menuItemHeaderSize);
   else
      m_font->Load(fontName, m_menuItemTextSize);

   float headerWidth = 0;
   float textWidth = 0;
   CreateHeaderAndTextNode(m_helpButtonNode, helpButtonHeaderText, helpButtonText, m_font,
                           m_menuItemHeaderSize, m_menuItemTextSize, colourHeader, colourText,
                           headerWidth, textWidth);
}


HelpMenu::~HelpMenu(void)
{

}

void HelpMenu::InitMenuStructure(uint32_t screenWidth, uint32_t screenHeight, float xPercentScreen, float yPercentScreen)
{
   SetCamera(screenWidth, screenHeight);

   // Compute the position of the help button
   Vec2 helpBtnPos = GetHelpButtonPosition(screenWidth, screenHeight);

   // Set the position of the button to display the menu
   m_helpButtonNode->SetPosition(Vec3(helpBtnPos.X(), helpBtnPos.Y(), 0.0));
   // Set the default position of the menu to the help button position
   m_helpMenuNode->SetPosition(Vec3(helpBtnPos.X(), helpBtnPos.Y(), 0.0));

   // Hide the menu
   m_helpMenuNode->SetOpacity(NO_OPACITY);
   // Show the help button
   m_helpButtonNode->SetOpacity(FULL_OPACITY);

   // Append the node used to display the background
   m_helpMenuNode->AppendChild(m_menuBackgroundNode);
   m_menuBackgroundNode->SetVisible(false);

   m_rootNode->AppendChild(m_helpButtonNode);
   m_rootNode->AppendChild(m_helpMenuNode);

   m_buttonEffect->Load("button.bfx");

   m_menuBackgroundColour = DEFAULT_MENU_BACKGROUND_COLOUR;
}

Vec2 HelpMenu::GetHelpButtonPosition(uint32_t screenWidth, uint32_t screenHeight)
{
   Vec2 helpBtnPos;

   // Calculate the position of the help button

   // If the overscan is used make sure that the help button is in
   // the overscan area
   float adjustedScreenWidth  = static_cast<float>(screenWidth);
   float adjustedScreenHeight = static_cast<float>(screenHeight);

   if (m_useOverscan)
   {
      adjustedScreenWidth  *= SCREEN_WIDTH_OVERSCAN;
      adjustedScreenHeight *= SCREEN_HEIGHT_OVERSCAN;
   }

   helpBtnPos.X() = (adjustedScreenWidth  * m_helpBtnPercentPos.X()) - (adjustedScreenWidth  / 2.0f);
   helpBtnPos.Y() = (adjustedScreenHeight * m_helpBtnPercentPos.Y()) - (adjustedScreenHeight / 2.0f);

   return helpBtnPos;
}

void HelpMenu::SetMenuItemTextSize(float size)
{
   m_menuItemTextSize = size * GetScreenHeight(m_app);
}

void HelpMenu::SetMenuItemHeaderSize(float size)
{
   m_menuItemHeaderSize = size * GetScreenHeight(m_app);
}

void HelpMenu::SetCamera(uint32_t screenWidth, uint32_t screenHeight)
{
   m_camera->SetType(Camera::eORTHOGRAPHIC);
   m_camera->SetClippingPlanes(0.0f, 20.0f);
   m_camera->SetYFov(static_cast<float>(screenHeight));
   m_camera->SetAspectRatio(screenWidth, screenHeight);

   // Add camera into its node and position it
   SceneNodeHandle cameraNode(New);
   cameraNode->SetCamera(m_camera);
   cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 0.0f, 1.0f),  // Where
                                    Vec3(0.0f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector
   cameraNode->SetCamera(m_camera);

   m_rootNode->AppendChild(cameraNode);
}

void HelpMenu::CreateHeaderAndTextNode(SceneNodeHandle &buttonNode,
                                       const string &helpButtonHeaderText,
                                       const string &helpButtonText,
                                       const PrintFontHandle &font,
                                       float sizeHeader,
                                       float sizeText,
                                       const Vec4 &colourHeader,
                                       const Vec4 &colourText,
                                       float &headerWidth,
                                       float &textWidth)
{
   float adjustTextHorizonPosition = 0.0;

   if (!helpButtonHeaderText.empty())
   {
      // Create geometries for the header
      PrintHandle headerGeom(New);
      headerGeom->SetFont(font, GetDefaultEffect(), PrintOptions(PrintOptions::eCENTER_LEFT, false));
      headerGeom->SetText(helpButtonHeaderText, sizeHeader);
      headerGeom->SetUniform("u_textColor", colourHeader);
      headerGeom->SetSortPriority(MENU_SORT_PRIORITY);

      // Store the max width to be able "tab" the description text
      headerWidth = headerGeom->GetWidth();

      adjustTextHorizonPosition = headerWidth + DEFAULT_GAP_BETWEEN_HEADER_AND_DESCRIPTION;

      SceneNodeHandle headerNode(New);
      headerNode->AppendGeometry(headerGeom);

      buttonNode->AppendChild(headerNode);
   }

   // Create geometries for the text
   PrintHandle textGeom(New);
   textGeom->SetFont(font, GetDefaultEffect(), PrintOptions(PrintOptions::eCENTER_LEFT, false));
   textGeom->SetText(helpButtonText, sizeText);
   textGeom->SetUniform("u_textColor", colourText);
   textGeom->SetSortPriority(MENU_SORT_PRIORITY);

   SceneNodeHandle textNode(New);
   textNode->AppendGeometry(textGeom);

   // Move the description text after the header
   textNode->SetPosition(Vec3(adjustTextHorizonPosition, 0.0f, 0.0f));

   // Calculate the max text width to create a background
   textWidth = textGeom->GetWidth();

   buttonNode->AppendChild(textNode);
}

void HelpMenu::CreateColouredButtonNode(SceneNodeHandle &buttonNode,
                                        eHELPBUTTON button,
                                        const string &text,
                                        const PrintFontHandle &font,
                                        float fontSize,
                                        const Vec4 &colourText,
                                        float &imgSize,
                                        float &textWidth)
{
   // Create description text geometries
   PrintHandle textGeom(New);
   textGeom->SetFont(font, GetDefaultEffect(), PrintOptions(PrintOptions::eCENTER_LEFT, false));
   textGeom->SetText(text, fontSize);
   textGeom->SetUniform("u_textColor", colourText);
   textGeom->SetSortPriority(MENU_SORT_PRIORITY);

   bsg::SceneNodeHandle buttonTextNode(New);
   buttonTextNode->AppendGeometry(textGeom);

   // Create the geometries for the textured button
   TextureHandle buttonTex(New);
   float adjustPosition = 0.0;

   // Common operations for all the coloured button
   switch (button)
   {
      // If the help button is one of the colored ones
      // Create the geometry and the material
      case eHELP_BUTTON_YELLOW:
      case eHELP_BUTTON_GREEN:
      case eHELP_BUTTON_BLUE:
      case eHELP_BUTTON_RED:
         {
            MaterialHandle buttonMaterial(New);
            buttonMaterial->SetEffect(m_buttonEffect);

            buttonTex->SetAutoMipmap(true);
            buttonMaterial->SetTexture("u_tex", buttonTex);

            imgSize = fontSize * 0.8f;
            GeometryHandle buttonQuad = QuadFactory (imgSize, imgSize, 0.0, eZ_AXIS).MakeGeometry(buttonMaterial);
            buttonQuad->SetSortPriority(MENU_SORT_PRIORITY);

            bsg::SceneNodeHandle buttonImgNode(New);
            buttonImgNode->AppendGeometry(buttonQuad);

            adjustPosition = imgSize * 1.5f;
            Vec2 mn, mx;
            textGeom->GetBounds(mn, mx);
            buttonImgNode->SetPosition(Vec3(imgSize/2.0f, mx.Y() / 2.0f, 0.0f));

            buttonNode->AppendChild(buttonImgNode);
         }
         break;
      default:
         break;
   }

   // Setting texture for the type of button
   switch (button)
   {
   case eHELP_BUTTON_RED:
         buttonTex->TexImage2D(Image("red_button", "png", Image::eRGBA8888));
      break;

   case eHELP_BUTTON_GREEN:
         buttonTex->TexImage2D(Image("green_button", "png", Image::eRGBA8888));
      break;

   case eHELP_BUTTON_YELLOW:
         buttonTex->TexImage2D(Image("yellow_button", "png", Image::eRGBA8888));
      break;

   case eHELP_BUTTON_BLUE:
         buttonTex->TexImage2D(Image("blue_button", "png", Image::eRGBA8888));
      break;

   default:
      break;
   }

   textWidth = textGeom->GetWidth();

   // Adding the description node after the button node
   // It is useful to know the order for the Tab/indent function
   buttonNode->AppendChild(buttonTextNode);

   // Adjust the position of the text
   buttonTextNode->SetPosition(Vec3(adjustPosition, 0.0, 0.0));

}

void HelpMenu::SetMenuPosition(float xPercentScreen, float yPercentScreen)
{
   // store the values used to calculate the menu position
   m_menuPercentPos.X() = xPercentScreen;
   m_menuPercentPos.Y() = yPercentScreen;
   // Set m_menuReferencePos to eMENU_POSITION_NONE which is used to decide
   // how to calculate the menu position (i.e. used a percentage of the screen size)
   m_menuReferencePos = eMENU_POSITION_NONE;

   SetMenuPosition();
}

void HelpMenu::SetMenuPosition(eMenuPosition menuPosition, const Vec2 &offset)
{
   // store the values used to calculate the menu position
   m_menuReferencePos = menuPosition;
   m_menuOffsetPos = offset;

   SetMenuPosition();
}

void HelpMenu::SetMenuPosition()
{
   Vec2 menuPos = GetMenuPosition();
   m_helpMenuNode->SetPosition(Vec3(menuPos.X(), menuPos.Y(), 0.0));
}

Vec2 HelpMenu::GetMenuPosition()
{
   Vec2 menuPos;

   if (eMENU_POSITION_NONE == m_menuReferencePos)
   {
      menuPos = GetMenuPositionFromPercent();
   }
   else
   {
      menuPos = GetMenuPositionFromRefPos();
   }

   return menuPos;
}

Vec2 HelpMenu::GetMenuPositionFromPercent()
{
   Vec2 menuPos;

   float screenHeight = 0.0f;
   float screenWidth  = 0.0f;

   if (m_useOverscan)
   {
      screenHeight = static_cast<float>(GetScreenHeight(m_app) * SCREEN_HEIGHT_OVERSCAN);
      screenWidth  = static_cast<float>(GetScreenWidth(m_app)  * SCREEN_WIDTH_OVERSCAN);
   }
   else
   {
      screenHeight = static_cast<float>(GetScreenHeight(m_app));
      screenWidth  = static_cast<float>(GetScreenWidth(m_app));
   }

   // Calculate the position of the help button
   menuPos.X() = (screenWidth  * m_menuPercentPos.X()) - (screenWidth  / 2.0f);
   menuPos.Y() = (screenHeight * m_menuPercentPos.Y()) - (screenHeight / 2.0f);

   return menuPos;
}

void HelpMenu::GetMenuPartsMaxSizes(unsigned int &numberMenuItem, float &maxHeaderWidth, float &maxTextWidth)
{
   // Go through all the menu which have been created
   for (MenuMap::iterator menuPartIt = m_menuMap.begin(); menuPartIt != m_menuMap.end(); menuPartIt++)
   {
      // Get the menu part
      MenuPart *menuPart = menuPartIt->second;

      // If the menu exist and is visible
      if ((menuPart) && (menuPart->IsVisible()))
      {
         numberMenuItem += menuPart->GetNumberOfItems();

         if (maxHeaderWidth < menuPart->GetMaxHeaderWidth())
            maxHeaderWidth = menuPart->GetMaxHeaderWidth();

         if (maxTextWidth < menuPart->GetMaxTextWidth())
            maxTextWidth = menuPart->GetMaxTextWidth();
      }
   }
}

Vec2 HelpMenu::GetMenuPositionFromRefPos()
{
   Vec2 menuPos;
   float largestHeight = 0;
   if (m_menuItemHeaderSize < m_menuItemTextSize)
      largestHeight = m_menuItemTextSize;
   else
      largestHeight = m_menuItemHeaderSize;

   // Go through the visible menus to find number of items and maximum sizes
   unsigned int numberMenuItem = 0;
   float        maxHeaderWidth = 0.0f;
   float        maxTextWidth   = 0.0f;

   GetMenuPartsMaxSizes(numberMenuItem, maxHeaderWidth, maxTextWidth);

   float menuHeight  = numberMenuItem * largestHeight;
   float paddingSize = largestHeight;
   float menuWidth   = maxHeaderWidth + DEFAULT_GAP_BETWEEN_HEADER_AND_DESCRIPTION + maxTextWidth;

   float screenHeight = 0.0;
   float screenWidth = 0.0;

   if (m_useOverscan)
   {
      screenHeight = static_cast<float>(GetScreenHeight(m_app) * SCREEN_HEIGHT_OVERSCAN);
      screenWidth  = static_cast<float>(GetScreenWidth(m_app)  * SCREEN_WIDTH_OVERSCAN);
   }
   else
   {
      screenHeight = static_cast<float>(GetScreenHeight(m_app));
      screenWidth  = static_cast<float>(GetScreenWidth(m_app));
   }

   switch (m_menuReferencePos)
   {
   case eMENU_POSITION_TOP_RIGHT:
      {
         menuPos.X() = (screenWidth - menuWidth - (paddingSize/2.0f)) - (screenWidth / 2.0f);
         menuPos.Y() = (screenHeight - (paddingSize))  - (screenHeight / 2.0f);
      }
      break;

   case eMENU_POSITION_TOP_LEFT:
      {
         menuPos.X() = (paddingSize/2.0f) - (screenWidth / 2.0f);
         menuPos.Y() = (screenHeight - (paddingSize))  - (screenHeight / 2.0f);
      }
      break;

   case eMENU_POSITION_BOT_RIGHT:
      {
         menuPos.X() = (screenWidth - menuWidth - (paddingSize/2.0f)) - (screenWidth / 2.0f);
         menuPos.Y() = (menuHeight + (paddingSize/2.0f))  - (screenHeight / 2.0f);
      }
      break;

   case eMENU_POSITION_BOT_LEFT:
      {
         menuPos.X() = (paddingSize/2.0f) - (screenWidth / 2.0f);
         menuPos.Y() = (menuHeight + (paddingSize/2.0f))  - (screenHeight / 2.0f);
      }
      break;

   case eMENU_POSITION_CENTRE:
      {
         menuPos.X() = - (menuWidth/2.0f) - (paddingSize/2.0f);
         menuPos.Y() = (menuHeight/2.0f) + paddingSize;
      }
      break;

   default:
      break;
   }

   menuPos.X() = menuPos.X() + m_menuOffsetPos.X();
   menuPos.Y() = menuPos.Y() + m_menuOffsetPos.Y();

   return menuPos;
}

MenuPart *HelpMenu::GetMenuPart(eMenuID menu)
{
   // Get the corresponding menu part
   MenuPart *menuPart = 0;
   // If this menu doesn't exist yet
   MenuMap::iterator menuIt(m_menuMap.find(menu));
   if (menuIt == m_menuMap.end())
   {
      // Create a new menu part and insert it into the map
      menuPart = new MenuPart(menu);
      menuPart = m_menuMap[menu] = menuPart;

      m_helpMenuNode->AppendChild(menuPart->GetMenuNode());
   }
   else
   {
      menuPart = menuIt->second;
   }

   return menuPart;
}

void HelpMenu::AddMenuItem(eHELPBUTTON button, const string &menuItemText, eMenuID menu)
{
   MenuPart *menuPart = GetMenuPart(menu);

   if (menuPart)
   {
      // Create the menu item

      SceneNodeHandle buttonNode(New);

      float headerWidth = 0;
      float textWidth = 0;
      CreateColouredButtonNode(buttonNode, button, menuItemText, m_font, m_menuItemTextSize, m_menuItemTextColour,
                              headerWidth, textWidth);

      // Place vertically the menu item according to the largest font size
      if (m_menuItemHeaderSize < m_menuItemTextSize)
         buttonNode->SetPosition(Vec3(0.0, -(menuPart->GetNumberOfItems() * m_menuItemTextSize), 0.0));
      else
         buttonNode->SetPosition(Vec3(0.0, -(menuPart->GetNumberOfItems() * m_menuItemHeaderSize), 0.0));

      // Add the menu item to the node of the menu part
      menuPart->AppendItem(buttonNode, headerWidth, textWidth);
   }
   else
   {
      //TODO: this should never happen
   }
}

void HelpMenu::AddMenuItem(const string &menuItemHeader, const string &menuItemText, eMenuID menu)
{
   MenuPart *menuPart = GetMenuPart(menu);

   if (menuPart)
   {
      // Create the menu item
      bsg::SceneNodeHandle buttonNode(New);

      float headerWidth = 0;
      float textWidth = 0;
      CreateHeaderAndTextNode(buttonNode, menuItemHeader, menuItemText, m_font,
                        m_menuItemHeaderSize, m_menuItemTextSize, m_menuItemHeaderColour, m_menuItemTextColour,
                        headerWidth, textWidth);

      if (m_menuItemHeaderSize < m_menuItemTextSize)
         buttonNode->SetPosition(Vec3(0.0, -(menuPart->GetNumberOfItems() * m_menuItemTextSize), 0.0));
      else
         buttonNode->SetPosition(Vec3(0.0, -(menuPart->GetNumberOfItems() * m_menuItemHeaderSize), 0.0));

      menuPart->AppendItem(buttonNode, headerWidth, textWidth);
   }
   else
   {
      //TODO: this should never happen
   }
}

void HelpMenu::Resize()
{
   uint32_t  screenWidth  = GetScreenWidth(m_app);
   uint32_t  screenHeight = GetScreenHeight(m_app);

   // Set the camera characteristics
   m_camera->SetYFov(static_cast<float>(screenHeight));
   m_camera->SetAspectRatio(screenWidth, screenHeight);

   // Set the position of the button to display the menu
   Vec2 helpBtnPos = GetHelpButtonPosition(screenWidth, screenHeight);
   m_helpButtonNode->SetPosition(Vec3(helpBtnPos.X(), helpBtnPos.Y(), 0.0));

   // Set position of the help menu
   SetMenuPosition();

   // update the parameters used to animate the menu
   UpdateAnimationMenuHidePosition(screenWidth, screenHeight);
}

void HelpMenu::SetMenuToDisplay(unsigned short menuToDisplay)
{
   // If we already display the same menus
   if (menuToDisplay != m_menuToDisplay)
   {
      m_menuToDisplay = menuToDisplay;

      // Go through all the menu make them visible/not visible according to menuToDisplay
      for (MenuMap::iterator menuPartIt = m_menuMap.begin(); menuPartIt != m_menuMap.end(); menuPartIt++)
      {
         // Get the menu part
            MenuPart *menuPart = menuPartIt->second;

            // If the menu exist
            if (menuPart)
            {
               if ((menuPartIt->first & m_menuToDisplay) == 0)
                  menuPart->SetVisible(false);
               else
                  menuPart->SetVisible(true);
            }
      }

      TabMenuItemDescriptions();

      // Move the menu is the size changed
      AdjustMenuPositionAnimation();
   }
}

void HelpMenu::AdjustMenuPositionAnimation()
{
   if (m_menuDiplayed)
   {
      if (!m_animationRunning)
      {
         m_menuPosNeedAdjusted = false;

         Vec3 startMenuPos = m_helpMenuNode->GetPosition();
         Vec2 menuPos(GetMenuPosition());

         if ((startMenuPos.X() != menuPos.X()) ||
            (startMenuPos.Y() != menuPos.Y()))
         {
            Time  now  = m_app->FrameTimestamp();

            // Animation of the help menu to the new position
            AnimBindingHermiteVec3 *showMenuAnim3 = new AnimBindingHermiteVec3(&m_helpMenuNode->GetPosition());
            showMenuAnim3->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);

            showMenuAnim3->Evaluator()->Init(startMenuPos, Vec3(menuPos.X(), menuPos.Y(), 0.0));

            m_animList.Append(showMenuAnim3);
         }
      }
      else
         m_menuPosNeedAdjusted = true;
   }
}

void HelpMenu::FinaliseMenuLayout()
{
   if (eMENU_NOT_FINILISED == m_menuToDisplay)
      SetMenuToDisplay(eMENU_ID_0);
}

void HelpMenu::TabMenuItemDescriptions()
{
   unsigned int numberMenuItem = 0;
   float maxHeaderWidth = 0.0f;
   float maxTextWidth = 0.0f;
   GetMenuPartsMaxSizes(numberMenuItem, maxHeaderWidth, maxTextWidth);

   float menuYOffset = 0.0;

   // Go through all the menu which have been created to move the help description text
   for (MenuMap::iterator menuPartIt = m_menuMap.begin(); menuPartIt != m_menuMap.end(); menuPartIt++)
   {
      // Get the menu part
      MenuPart *menuPart = menuPartIt->second;

      // If the menu exist and is visible
      if ((menuPart) && (menuPart->IsVisible()))
      {
         // Place the menu vertically
         Vec3 menuPos = menuPart->GetMenuNode()->GetPosition();
         menuPos.Y() = menuYOffset;
         menuPart->GetMenuNode()->SetPosition(menuPos);

         // Going through all the menu items
         unsigned int numberItems = menuPart->GetNumberOfItems();
         for (unsigned int index = 0; index < numberItems; ++index)
         {
            // Get a menu item
            SceneNodeHandle menuItem = menuPart->GetMenuNode()->GetChild(index);

            // If we have a header node and a text node
            uint32_t numParts = menuItem->NumChildren();
            if (numParts > 0)
            {
               // Get the description text which should be the last child
               SceneNodeHandle descriptionTextNode = menuItem->GetChild(numParts - 1);

               // Moving the position of the description text to the right
               // of the maximum header width to vertically align all of them
               Vec3 position = descriptionTextNode->GetPosition();
               position.X() = maxHeaderWidth + DEFAULT_GAP_BETWEEN_HEADER_AND_DESCRIPTION;
               descriptionTextNode->SetPosition(position);
            }
         }

         // Place vertically the menu item according to the largest font size
         if (m_menuItemHeaderSize < m_menuItemTextSize)
            menuYOffset += -(numberItems * m_menuItemTextSize);
         else
            menuYOffset += -(numberItems * m_menuItemHeaderSize);
      }
   }

   if (m_useMenuBackground)
      CreateBackground();
}

void HelpMenu::CreateBackground()
{
   m_menuBackgroundNode->SetVisible(true);
   m_menuBackgroundNode->ClearGeometry();

   EffectHandle  backgroundEffect(New);
   MaterialHandle backgroundMaterial(New);
   backgroundEffect->Load("help_menu_background.bfx");
   backgroundMaterial->SetEffect(backgroundEffect);
   backgroundMaterial->SetUniformValue("u_color", m_menuBackgroundColour);

   //TextureHandle backgroundTex(New);
   //backgroundTex->SetAutoMipmap(true);
   //backgroundTex->TexImage2D(Image("help_menu_background_glow", "png", Image::eRGBA8888));
   //backgroundMaterial->SetTexture("u_texture", backgroundTex);

   float largestHeight = 0;
   if (m_menuItemHeaderSize < m_menuItemTextSize)
      largestHeight = m_menuItemTextSize;
   else
      largestHeight = m_menuItemHeaderSize;

   // Get the max size of all visible menu
   unsigned int numberMenuItem = 0;
   float maxHeaderWidth = 0.0f;
   float maxTextWidth = 0.0f;
   GetMenuPartsMaxSizes(numberMenuItem, maxHeaderWidth, maxTextWidth);

   float menuHeight = numberMenuItem * largestHeight;

   float paddingSize = largestHeight;

   float backgroundWidth = maxHeaderWidth + DEFAULT_GAP_BETWEEN_HEADER_AND_DESCRIPTION + maxTextWidth;

   GeometryHandle backgroundQuad;

   if (m_menuBackgroundRoundedCorners)
      backgroundQuad = RoundedRectFactory (Vec3(0.0f), backgroundWidth + paddingSize, menuHeight + paddingSize, 10.0f, 50, eZ_AXIS).MakeGeometry(backgroundMaterial);
   else
      backgroundQuad = QuadFactory (backgroundWidth + paddingSize, menuHeight + paddingSize, 0.0, eZ_AXIS).MakeGeometry(backgroundMaterial);

   backgroundQuad->SetSortPriority(BACKGROUND_SORT_PRIORITY);

   m_menuBackgroundNode->AppendGeometry(backgroundQuad);

   m_menuBackgroundNode->SetPosition(Vec3((backgroundWidth)/2.0f, -((menuHeight)/2.0f) + largestHeight, 0.0f));

}

void HelpMenu::SetMenuBackgroundColour(const Vec4 &colour, bool roundedCorners)
{
   m_menuBackgroundColour = colour;
   m_menuBackgroundRoundedCorners = roundedCorners;

   UseMenuBackground();
}

void HelpMenu::UseMenuBackground()
{
   m_useMenuBackground = true;

   CreateBackground();
}

void HelpMenu::ToggleMenu()
{
   if (!m_animationRunning)
   {
      if (m_menuDiplayed)
      {
         CreateAnimToHideMenu();
      }
      else
      {
         CreateAnimToDisplayMenu();
      }

      m_menuDiplayed = !m_menuDiplayed;
   }
};

void HelpMenu::CreateAnimToHideMenu()
{
   Time  now  = m_app->FrameTimestamp();

   if (m_animationType & eMENU_ANIM_FADE)
   {
      // Animate the menu
      AnimBindingHermiteFloat *showMenuAnim = new AnimBindingHermiteFloat(&m_helpMenuNode->GetOpacity());
      showMenuAnim->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim->Evaluator()->Init(FULL_OPACITY, NO_OPACITY);

      m_animList.Append(showMenuAnim);

      // Animate the help button
      AnimBindingHermiteFloat *showMenuAnim2 = new AnimBindingHermiteFloat(&m_helpButtonNode->GetOpacity());
      showMenuAnim2->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim2->Evaluator()->Init(NO_OPACITY, FULL_OPACITY);

      m_animList.Append(showMenuAnim2);
   }

   if (m_animationType & eMENU_ANIM_SCALE)
   {
      // Animate the menu
      AnimBindingHermiteVec3 *showMenuAnim = new AnimBindingHermiteVec3(&m_helpMenuNode->GetScale());
      showMenuAnim->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim->Evaluator()->Init(SHOW_SCALE, HIDE_SCALE);

      m_animList.Append(showMenuAnim);

      // Animate the help button

      // Set the help button visible
      m_helpButtonNode->SetOpacity(FULL_OPACITY);

      AnimBindingHermiteVec3 *showMenuAnim2 = new AnimBindingHermiteVec3(&m_helpButtonNode->GetScale());
      showMenuAnim2->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim2->Evaluator()->Init(HIDE_SCALE, SHOW_SCALE);

      m_animList.Append(showMenuAnim2);
   }

   if ((m_animationType & eMENU_ANIM_MOVE_FROM_HELP_BTN) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_TOP_RIGHT) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_TOP_LEFT)  ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_BOT_RIGHT) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_BOT_LEFT))
   {
      // This animation should probably be combined with another

      //------ Animation of the help menu

      // Move the menu to its place
      AnimBindingHermiteVec3 *showMenuAnim3 = new AnimBindingHermiteVec3(&m_helpMenuNode->GetPosition());
      showMenuAnim3->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      Vec2 menuPos(GetMenuPosition());
      showMenuAnim3->Evaluator()->Init(Vec3(menuPos.X(), menuPos.Y(), 0.0), Vec3(m_animationMenuHidePosition.X(), m_animationMenuHidePosition.Y(), 0.0));

      m_animList.Append(showMenuAnim3);
   }
}

void HelpMenu::CreateAnimToDisplayMenu()
{
   Time  now  = m_app->FrameTimestamp();

   if (m_animationType & eMENU_ANIM_FADE)
   {
      // Animate the menu
      AnimBindingHermiteFloat *showMenuAnim = new AnimBindingHermiteFloat(&m_helpMenuNode->GetOpacity());
      showMenuAnim->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim->Evaluator()->Init(NO_OPACITY, FULL_OPACITY);

      m_animList.Append(showMenuAnim);

      // Animate the help button
      AnimBindingHermiteFloat *showMenuAnim2 = new AnimBindingHermiteFloat(&m_helpButtonNode->GetOpacity());
      showMenuAnim2->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim2->Evaluator()->Init(FULL_OPACITY, NO_OPACITY);

      m_animList.Append(showMenuAnim2);
   }

   if (m_animationType & eMENU_ANIM_SCALE)
   {
      // Animate the menu

      // Set the help menu visible
      m_helpMenuNode->SetOpacity(FULL_OPACITY);

      AnimBindingHermiteVec3 *showMenuAnim = new AnimBindingHermiteVec3(&m_helpMenuNode->GetScale());
      showMenuAnim->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim->Evaluator()->Init(HIDE_SCALE, SHOW_SCALE);

      m_animList.Append(showMenuAnim);

      // Animate the help button
      AnimBindingHermiteVec3 *showMenuAnim2 = new AnimBindingHermiteVec3(&m_helpButtonNode->GetScale());
      showMenuAnim2->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      showMenuAnim2->Evaluator()->Init(SHOW_SCALE, HIDE_SCALE);

      m_animList.Append(showMenuAnim2);
   }

   if ((m_animationType & eMENU_ANIM_MOVE_FROM_HELP_BTN) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_TOP_RIGHT) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_TOP_LEFT)  ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_BOT_RIGHT) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_BOT_LEFT))
   {
      // This animation should probably be combined with another

      // Animation of the help menu
      AnimBindingHermiteVec3 *showMenuAnim3 = new AnimBindingHermiteVec3(&m_helpMenuNode->GetPosition());
      showMenuAnim3->Interpolator()->Init(now, now + 1.0f * m_app->GetRateMultiplier(), BaseInterpolator::eLIMIT);
      Vec2 menuPos(GetMenuPosition());
      showMenuAnim3->Evaluator()->Init(Vec3(m_animationMenuHidePosition.X(), m_animationMenuHidePosition.Y(), 0.0), Vec3(menuPos.X(), menuPos.Y(), 0.0));

      m_animList.Append(showMenuAnim3);
   }
}

void HelpMenu::SetAnimationType(unsigned int type)
{
   m_animationType = type;

   uint32_t screenWidth  = GetScreenWidth(m_app);
   uint32_t screenHeight = GetScreenHeight(m_app);

   m_helpButtonNode->SetOpacity(FULL_OPACITY);
   m_helpMenuNode->SetOpacity(NO_OPACITY);

   if (m_animationType & eMENU_ANIM_FADE)
   {

   }

   if (m_animationType & eMENU_ANIM_SCALE)
   {
      m_helpButtonNode->SetScale(SHOW_SCALE);
      m_helpMenuNode->SetScale(HIDE_SCALE);
   }

   if ((m_animationType & eMENU_ANIM_MOVE_FROM_HELP_BTN) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_TOP_RIGHT) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_TOP_LEFT) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_BOT_RIGHT) ||
      (m_animationType & eMENU_ANIM_MOVE_FROM_BOT_LEFT))
   {
      // Update m_animationMenuHidePosition
      UpdateAnimationMenuHidePosition(screenWidth, screenHeight);

      // Set the position of the menu to be the same as the help button
      m_helpMenuNode->SetPosition(Vec3(m_animationMenuHidePosition.X(), m_animationMenuHidePosition.Y(), 0.0));
   }
}

void HelpMenu::UpdateAnimationMenuHidePosition(uint32_t screenWidth, uint32_t screenHeight)
{
   switch (m_animationType)
   {
      case eMENU_ANIM_MOVE_FROM_HELP_BTN:
         {
            m_animationMenuHidePosition = GetHelpButtonPosition(screenWidth, screenHeight);
         }
         break;

      case eMENU_ANIM_MOVE_FROM_TOP_RIGHT:
         {
            m_animationMenuHidePosition.X() = screenWidth -  (screenWidth  / 2.0f);
            m_animationMenuHidePosition.Y() = screenHeight - (screenHeight / 2.0f);
         }
         break;

      case eMENU_ANIM_MOVE_FROM_TOP_LEFT:
         {
            m_animationMenuHidePosition.X() = - (screenWidth / 2.0f);
            m_animationMenuHidePosition.Y() = screenHeight - (screenHeight / 2.0f);
         }
         break;

      case eMENU_ANIM_MOVE_FROM_BOT_RIGHT:
         {
            m_animationMenuHidePosition.X() = screenWidth - (screenWidth / 2.0f);
            m_animationMenuHidePosition.Y() = - (screenHeight / 2.0f);
         }
         break;

      case eMENU_ANIM_MOVE_FROM_BOT_LEFT:
         {
            m_animationMenuHidePosition.X() = - (screenWidth  / 2.0f);
            m_animationMenuHidePosition.Y() = - (screenHeight / 2.0f);
         }
         break;

      default:
         {
            m_animationMenuHidePosition = GetHelpButtonPosition(screenWidth, screenHeight);
         }
         break;
   }
}

void HelpMenu::UpdateTime()
{
   m_animationRunning = m_animList.UpdateTime(m_app->FrameTimestamp());

   if ((!m_animationRunning) && (m_menuPosNeedAdjusted))
      AdjustMenuPositionAnimation();
}

EffectHandle HelpMenu::GetDefaultEffect() const
{
   if (s_textEffect.IsNull())
   {
      s_textEffect = EffectHandle(New);
      s_textEffect->Read(s_effectText);
   }

   return s_textEffect;
}

void HelpMenu::Notify(const Time &time)
{
   AnimationDoneNotifier::Reset();

   // If there is not FADING animation
   if ((m_animationType & eMENU_ANIM_FADE) == 0)
   {
      if (m_menuDiplayed)
      {
         // Make the help button not visible
         m_helpButtonNode->SetOpacity(NO_OPACITY);
      }
      else
      {
         // Make the help menu not visible
         m_helpMenuNode->SetOpacity(NO_OPACITY);
      }
   }
}
