/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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

#ifndef __BCM_HELP_MENU_H__
#define __BCM_HELP_MENU_H__

#include <string>
#include <vector>

#include "bsg_camera.h"
#include "bsg_scene_node.h"
#include "bsg_print.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"

//! Colour of the help button
enum eHELPBUTTON {
      eHELP_BUTTON_NONE,
      eHELP_BUTTON_RED,
      eHELP_BUTTON_GREEN,
      eHELP_BUTTON_YELLOW,
      eHELP_BUTTON_BLUE
  };

//! Flag to active animations
//! Multiple animations can be combined
enum eMenuAnimationType {
   eMENU_ANIM_FADE                  = 0x01,  //!< Default menu animation
   eMENU_ANIM_SCALE                 = 0x02,
   eMENU_ANIM_MOVE_FROM_HELP_BTN    = 0x04,  //!< Animate the menu moving from the position of the help button to the menu position
   eMENU_ANIM_MOVE_FROM_TOP_RIGHT   = 0x08,  //!< Animate the menu moving from the top right corner to the menu position
   eMENU_ANIM_MOVE_FROM_TOP_LEFT    = 0x10,  //!< Animate the menu moving from the top left corner to the menu position
   eMENU_ANIM_MOVE_FROM_BOT_RIGHT   = 0x20,  //!< Animate the menu moving from the bottom right corner to the menu position
   eMENU_ANIM_MOVE_FROM_BOT_LEFT    = 0x40,  //!< Animate the menu moving from the bottom left corner to the menu position
};

//! Default reference position to place the menu
enum eMenuPosition {
   eMENU_POSITION_TOP_RIGHT,
   eMENU_POSITION_TOP_LEFT,
   eMENU_POSITION_BOT_RIGHT,
   eMENU_POSITION_BOT_LEFT,
   eMENU_POSITION_CENTRE,
   eMENU_POSITION_NONE,                      //!< Used a percentage of the screen size to calculate the position of the menu
};

//! Menu that can be displayed
//! Several menus can be displayed at the same time
//! If several menus are displayed the order will from
//! eMENU_ID_0 at the top to eMENU_ID_7 at the bottom
enum eMenuID {
   eMENU_ID_0     = 0x01,
   eMENU_ID_1     = 0x02,
   eMENU_ID_2     = 0x04,
   eMENU_ID_3     = 0x08,
   eMENU_ID_4     = 0x10,
   eMENU_ID_5     = 0x20,
   eMENU_ID_6     = 0x40,
   eMENU_ID_7     = 0x80,
   eMENU_LAST     = eMENU_ID_7,
   eMENU_NOT_FINILISED = 0xFF,
};

//! Helper class used to store information about
//! a particular menu
class MenuPart
{
public:
   //! Constructor
   //!
   //! @param ID for this menu
   MenuPart(eMenuID menuID) :
      m_menuNode(bsg::New),
      m_numberItems(0),
      m_maxHeaderWidth(0),
      m_maxTextWidth(0)
   {
   }

   //! Returns the scene node for this menu
   //!
   //! @return the scene node for this menu
   const bsg::SceneNodeHandle GetMenuNode() const { return m_menuNode; }

   //! Accessor function
   //! @return the number of items within this menu
   unsigned int GetNumberOfItems()          const { return m_numberItems; }

   //! Accessor function
   //! @ return the width of the longer menu item header
   float GetMaxHeaderWidth()                const { return m_maxHeaderWidth; }

   //! Accessor function
   //! @ return the width of the longer menu item description text
   float GetMaxTextWidth()                  const { return m_maxTextWidth; }

   //! Add a menu item to this menu
   //!
   //! @param headerWidth width of the menu item header
   //! @param textWdith width of the menu item description text
   void AppendItem(bsg::SceneNodeHandle menuItem, float headerWidth, float textWidth)
   {
      m_menuNode->AppendChild(menuItem);
      m_numberItems++;
      if (m_maxHeaderWidth < headerWidth)
         m_maxHeaderWidth = headerWidth;
      if (m_maxTextWidth < textWidth)
         m_maxTextWidth = textWidth;
   }

   //! @return true if this menu is visible
   bool IsVisible()                       const { return m_menuNode->IsVisible(); }

   //! Set the visibility of this menu
   //!
   //! @param visible flag indicating if this menu should be visible
   void SetVisible(bool visible) { m_menuNode->SetVisible(visible); }

private:
   bsg::SceneNodeHandle m_menuNode;             //!< Scene node containing the menu items
   unsigned int         m_numberItems;          //!< Number of menu item in this menu
   float                m_maxHeaderWidth;       //!< Store the width of the larger header and used to tab the menu description
   float                m_maxTextWidth;         //!< Maximum size of the description texts used to create the menu background

};

class HelpMenu : public bsg::AnimationDoneNotifier
{
public:
   //! Constructor
   //!
   //! Creates one menu item describing the button to be pressed to display the full menu
   //!
   //! @param app[in] The Pointer to the BSG application
   //! @param button[in] The type of the coloured
   //! @param helpButtonText[in] The text to display next to the button image
   //! @param fontName[in] The font name
   //! @param colourText[in] The colour of the help button text
   //! @param fontSizePercent[in] The percentage of the screen height (range between 0.0 and 1.0)
   //! @param xPercentScreen[in] The position of the top corners as a percentage of the screen width (range between 0.0 and 1.0)
   //! @param yPercentScreen[in] The position of the left corners as a percentage of the screen height (range between 0.0 and 1.0)
   //! @param useOverscan if true only percentage of the screen will be used to display the menu
   //!
   //! The menu item characteristics are setup by default with the value passed in the constructor (e.g. colour of the menu item header)
   HelpMenu(bsg::Application *app,
            eHELPBUTTON button,
            const std::string &helpButtonText,
            const std::string &fontName,
            const bsg::Vec4 &colourText,
            float fontSizePercent,
            float xPercentScreen,
            float yPercentScreen,
            bool  useOverscan);

   //! Constructor
   //!
   //! Creates one menu item describing the button to be pressed to display the full menu
   //!
   //! @param app[in] The Pointer to the BSG application
   //! @param helpButtonHeaderText The text for the header: i.e. name of the button
   //! @param helpButtonText The text describing the function of the button
   //! @param fontName The font name
   //! @param colourHeader The colour for the header
   //! @param colourText The colour for the describing text
   //! @param fontSizePercentHeader The percentage of the screen height (range between 0.0 and 1.0)
   //! @param fontSizePercentText The percentage of the screen height (range between 0.0 and 1.0)
   //! @param xPercentScreen The position of the top corners as a percentage of the screen width (range between 0.0 and 1.0)
   //! @param yPercentScreen The position of the left corners as a percentage of the screen height (range between 0.0 and 1.0)
   //! @param useOverscan if true only percentage of the screen will be used to display the menu
   //!
   //! The menu item characteristics are setup by default with the value passed in the constructor (e.g. colour of the menu item header)
   HelpMenu(bsg::Application *app,
            const std::string &helpButtonHeaderText,
            const std::string &helpButtonText,
            const std::string &fontName,
            const bsg::Vec4 &colourHeader,
            const bsg::Vec4 &colourText,
            float fontSizePercentHeader,
            float fontSizePercentText,
            float xPercentScreen,
            float yPercentScreen,
            bool  useOverscan);

   //! Destructor
   ~HelpMenu(void);

   //! Return the scene node graph
   //! @return The scene node graph
   const bsg::SceneNodeHandle GetRootNode() const { return m_rootNode; };

   //! Adds an item into the menu
   //!
   //! @param[in] button The type of the button
   //! @param[in] menuItemText The text describing the function of the button
   void AddMenuItem(eHELPBUTTON button, const std::string &menuItemText, eMenuID menu = eMENU_ID_0);

   //! Adds an item into the menu
   //!
   //! @param[in] button The name of the button
   //! @param[in] menuItemText The text describing the function of the button
   void AddMenuItem(const std::string &menuItemHeader, const std::string &menuItemText, eMenuID menu = eMENU_ID_0);

   //! Set the position on the screen where the full menu should be displayed
   //!
   //! @param[in] xPercentScreen The x position in percentage of the screen
   //! @param[in] yPercentScreen The y position in percentage of the screen
   void SetMenuPosition(float xPercentScreen, float yPercentScreen);

   //! Set the position on the screen where the full menu should be displayed
   //! NOTE: This function should only be called after all the menu items
   //!       have been added
   //!
   //! @param[in] menuPosition One of the predefined positions
   //! @param[in] offset Offset from the menu position
   void SetMenuPosition(eMenuPosition menuPosition, const bsg::Vec2 &offset);

   //! Set the colour of the text describing the function of the buttons
   //! NOTE: This function should be called before adding any menu items
   //!
   //! @param[in] colour The colour of the text
   void SetMenuItemTextColour(const bsg::Vec4 &colour) {  m_menuItemTextColour =  colour; };

   //! Set the colour of the header i.e. button names
   //! NOTE: This function should be called before adding any menu items
   //!
   //! @param[in] colour The colour of the text
   void SetMenuItemHeaderColour(const bsg::Vec4 &colour) {  m_menuItemHeaderColour =  colour; };

   //! Set the size of the text use to describe the button description
   //! NOTE: This function should be called before adding any menu items
   //!
   //! @param[in] size The size of the text
   void SetMenuItemTextSize(float size);

   //! Set the size of the header i.e. the button name
   //! NOTE: This function should be called before adding any menu items
   //!
   //! @param[in] size The size of the text
   void SetMenuItemHeaderSize(float size);

   //! Toggle on and off the display of the full menu but only
   //! if the menu is not animated
   void ToggleMenu();

   //! Resize the camera ratio to match the change in screen size
   void Resize();

   //! Create a background for the menu (i.e. the list of menu item)
   //! and set the colour of this background
   //! NOTE: This function should only be called after all the menu items
   //!       have been added
   //! @param[in] colour Colour of the menu background
   //! @param[in] m_menuBackgroundRoundedCorners if true the background will be a rect with rounded corners
   void SetMenuBackgroundColour(const bsg::Vec4 &colour, bool roundedCorners);

   //! Create a background for the menu (i.e. the list of menu item)
   //! NOTE: This function should only be called after all the menu items
   //!       have been added
   void UseMenuBackground();

   //! Change the type of animation
   //!
   //! @param[in] type The type of animation
   void SetAnimationType(unsigned int type);

   //! Update the time for the menu animation
   void UpdateTime();

   //! Set which menu should be displayed and recalculte the background and position
   //!
   //! @param menuToDisplay combination of eMenuToDisplay bits indicating the menu to display
   void SetMenuToDisplay(unsigned short menuToDisplay);

   //! Layout the menu, position of the menu, background
   //! NOTE: This function should only be called after all the menu items
   //!       have been added
   void FinaliseMenuLayout();

private:

   //! Helper function to initialise the camera, the position of the menu, add node to the root node
   void InitMenuStructure(uint32_t screenWidth, uint32_t screenHeight, float xPercentScreen, float yPercentScreen);

   //! Set the parameter of the camera
   //!
   //! @param screenWidth The width of the window
   //! @param screenWidth The height of the window
   void SetCamera(uint32_t screenWidth, uint32_t screenHeight);

   //! Create a header (colours of the button) and a text to
   //!   describing the function of the button
   //!
   //! @param[in] buttonNode The node where the menu item should be created
   //! @param[in] button The colour of the button
   //! @param[in] text  The text describing the function of the button
   //! @param[in] font The font
   //! @param[in] fontSize The size of the font
   //! @param[in] colourText The colour of the text describing the button
   //! @param[out] imgSize Width of the menu item header
   //! @param[out] textWidth Width of the menu item description text
   void CreateColouredButtonNode(bsg::SceneNodeHandle &buttonNode,
                                 eHELPBUTTON button,
                                 const std::string &text,
                                 const bsg::PrintFontHandle &font,
                                 float fontSize,
                                 const bsg::Vec4 &colourText,
                                 float &imgSize,
                                 float &textWidth);

   //! Create a header (text of coloured button) and a text to
   //!   describing the function of the button
   //!
   //! @param[in] buttonNode The node where the menu item should be created
   //! @param[in] helpButtonHeaderText Text of the menu item header
   //! @param[in] helpButtonText Text of the menu item description
   //! @param[in] font The font
   //! @param[in] sizeHeader Text size of the menu item header
   //! @param[in] sizeText Text size of the menu item description
   //! @param[out] headerWidth Width of the menu item header
   //! @param[out] textWidth Width of the menu item description text
   void CreateHeaderAndTextNode(bsg::SceneNodeHandle &buttonNode,
                                const std::string &helpButtonHeaderText,
                                const std::string &helpButtonText,
                                const bsg::PrintFontHandle &font,
                                float sizeHeader,
                                float sizeText,
                                const bsg::Vec4 &colourHeader,
                                const bsg::Vec4 &colourText,
                                float &headerWidth,
                                float &textWidth);

   //! Create animations to hide the menu and add it to the animation list
   void CreateAnimToHideMenu();
   //! Create animations to display the menu and add it to the animation list
   void CreateAnimToDisplayMenu();

   //! Create the geometry for the background of the menu
   void CreateBackground();

   //! If the effect doesnt exist it is create
   //! This effect is used for the text in the menu items
   //!
   //! @return Returns an effect
   bsg::EffectHandle GetDefaultEffect() const;

   //! This function is called when an animation is finished
   //! This function is inherited from the class AnimationDoneNotifier
   //! @see AnimationDoneNotifier::Notify(const Time &time);
   virtual void Notify(const bsg::Time &time);

   //! Helper function to calculate the position of the Help button
   //!
   //! @return the position of the help button
   bsg::Vec2 GetHelpButtonPosition(uint32_t screenWidth, uint32_t screenHeight);

   //! Set the position of the menu using values from member variables
   void SetMenuPosition();

   //! Calculate the position of the menu using values from member variables
   //! Calls GetMenuPositionFromPercent() or GetMenuPositionFromRefPos()
   //!
   //! @return the position of the menu
   bsg::Vec2 GetMenuPosition();

   //! Calculate the position of the menu using percentages of the
   //! screen sizes
   //!
   //! @return the position of the menu
   bsg::Vec2 GetMenuPositionFromPercent();

   //! Calculate the position of the menu using a reference position
   //!
   //! @return the position of the menu
   bsg::Vec2 GetMenuPositionFromRefPos();

   //! Indent the menu description
   //! NOTE: This function should only be called after all the menu items
   //!       have been added
   void TabMenuItemDescriptions();

   //! Update the member variable m_animationMenuHidePosition
   void UpdateAnimationMenuHidePosition(uint32_t screenWidth, uint32_t screenHeight);

   //! Returns a pointer to a menu according to its ID
   //!
   //! @return a menu part (i.e. a menu with a particular ID)
   MenuPart *GetMenuPart(eMenuID menu);

   //! Returns the number of menu item cumulatively over all the visible menu and
   //! max characteristics over all visible menus
   //!
   //! @param[out] numberMenuItem Number of menu item over all the visible menus
   //! @param[out] maxHeaderWidth Max width of all the header of visible menus
   //! @param[out] maxTextWidth Max width of all the description text of visible menus
   void GetMenuPartsMaxSizes(unsigned int &numberMenuItem, float &maxHeaderWidth, float &maxTextWidth);

   //! Creates an animation to reposition the menu
   void AdjustMenuPositionAnimation();

   //! Map containing a menu ID and a pointer to MenuPart
   typedef std::map<eMenuID, MenuPart *> MenuMap;

   bsg::Application        *m_app;
   bsg::SceneNodeHandle    m_rootNode;                   //!< Root node of the whole help menu
   bsg::SceneNodeHandle    m_helpButtonNode;             //!< Node containing the button to press to get the help menu displayed
   bsg::SceneNodeHandle    m_helpMenuNode;               //!< Node containing the menu to be displayed
   bsg::SceneNodeHandle    m_menuBackgroundNode;         //!< Node containing the background of the menu
   bsg::CameraHandle       m_camera;                     //!< BSG camera used to display the menu
   MenuMap                 m_menuMap;                    //!< Map of menu IDs and menu part objects

   bsg::PrintFontHandle    m_font;                       //!< The text font used for menu texts

   bsg::EffectHandle       m_buttonEffect;               //!< Effect used for all the coloured button

   bsg::Vec4               m_menuItemHeaderColour;       //!< Colour for the header: i.e. the name of the button
   bsg::Vec4               m_menuItemTextColour;         //!< Colour of the text describing the function of the button
   float                   m_menuItemHeaderSize;         //!< Size of the text for the header: i.e. the name of the button
   float                   m_menuItemTextSize;           //!< Size of the text describing the function of the button
   bsg::Vec2               m_menuPercentPos;             //!< Position where the menu should be displayed in percent of the screen size
   eMenuPosition           m_menuReferencePos;           //!< Reference position used to place the menu
   bsg::Vec2               m_menuOffsetPos;              //!< Offset to place the menu with regard to the reference position
   bsg::Vec2               m_helpBtnPercentPos;          //!< Position where the help button should be displayed in percent of the screen size
   bsg::Vec4               m_menuBackgroundColour;       //!< Colour of the background menu
   bool                    m_useMenuBackground;          //!< Flag indicating if a background should be used when the menu is displayed
   bool                    m_menuBackgroundRoundedCorners; //!< Flag indicating if the menu background should have rounded corners
   bool                    m_useOverscan;                //!< Flag indicating if only the overscan area of the screen should be used

   bool                    m_menuDiplayed;               //!< Flag indicating if the menu is displayed
   unsigned short          m_menuToDisplay;              //!< Combination of menu IDs which should be displayed
   bool                    m_menuPosNeedAdjusted;        //!< Flag indicating that the menu needs to be move (e.g. due a change in its size)

   bool                    m_animationRunning;           //!< Flag indicating if an animation is running (Stops other menu animation starting)
   bsg::AnimationList      m_animList;                   //!< Animation list for the menu animations
   unsigned int            m_animationType;              //!< Indicate the type of animation to run when hiding/showing the menu
   bsg::Vec2               m_animationMenuHidePosition;  //!< Position when the menu is hidden
};

#endif  // endif __BCM_HELP_MENU_H__
