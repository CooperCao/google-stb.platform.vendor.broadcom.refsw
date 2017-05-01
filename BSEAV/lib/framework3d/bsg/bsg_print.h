/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#ifndef __BSG_PRINT_H__
#define __BSG_PRINT_H__

#include "bsg_common.h"
#include "bsg_gl_texture.h"
#include "bsg_geometry.h"
#include "bsg_vector.h"
#include "bsg_utf8.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

// Eliminate some windows namespace pollution
#undef max
#undef min

#include <limits>

namespace bsg
{

class PrintFont;
class Application;

//! The bsg::Print class takes PrintString arguments in its SetText() methods.  It is meant to be used as a temporary in
//! passing arguments of different types reducing the number of overloads of SetText.  A PrintString is simply a vector of 8-bit characters
//! (with some encoding) and can be constructed from either a C string, a std::string or a vector of characters.  Since conversions will be
//! performed automatically, you do not need to explicity construct a PrintString.  So, the following are all accepted:
//!
//! @code
//! const char             *myArray;
//! std::vector<uint8_t>   myVec;
//! myVec.push_back('a');
//! std::string            myString("Hello");
//!
//! printHandle->SetText(myArray,  10.0f);
//! printHandle->SetText(myString, 10.0f);
//! printHandle->SetText(myVec,    10.0f);
//! @endcode
//!
class PrintString : public std::vector<uint8_t>
{
public:
   PrintString()
   {}

   PrintString(const unsigned char *str) :
      std::vector<uint8_t>(strlen((const char *)str))
   {
      const unsigned char  *ptr = str;

      for (uint32_t i = 0; i < size(); ++i)
         operator[](i) = (uint8_t)*ptr++;
   }

   PrintString(const char *str) :
      std::vector<unsigned char>(strlen((const char *)str))
   {
      const char  *ptr = str;

      for (uint32_t i = 0; i < size(); ++i)
         operator[](i) = (uint8_t)*ptr++;
   }

   PrintString(const std::string &str) :
      std::vector<uint8_t>((uint8_t *)str.c_str(), (uint8_t *)str.c_str() + str.length())
   {}

   PrintString(const std::vector<uint8_t> &rhs) :
      std::vector<uint8_t>(rhs)
   {}
};

// @cond
struct PrintFontTraits
{
   typedef PrintFont       Base;
   typedef PrintFont       Derived;
   typedef PrintFontTraits BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<PrintFontTraits>    PrintFontHandle;
//! @}

class PrintCharInfo
{
public:
   PrintCharInfo() :
      m_advanceX(0.0f),
      m_size(0.0f),
      m_texture(0)
   {}

   PrintCharInfo(float advance, const Vec2 &middle, const Vec2 &u, const Vec2 &v, float size, uint32_t tex) :
      m_advanceX(advance),
      m_middle(middle),
      m_u(u),
      m_v(v),
      m_size(size),
      m_texture(tex)
   {}

   PrintCharInfo(const PrintCharInfo &rhs, float scale = 1.0f) :
      m_advanceX(scale * rhs.m_advanceX),
      m_middle(Vec2(scale) * rhs.m_middle),
      m_u(rhs.m_u),
      m_v(rhs.m_v),
      m_size(scale * rhs.m_size),
      m_texture(rhs.m_texture)
   {}

   float       GetAdvanceX() const { return m_advanceX;     }
   float       GetMiddleX()  const { return m_middle.X();   }
   float       GetMiddleY()  const { return m_middle.Y();   }
   const Vec2  &GetU()       const { return m_u;            }
   const Vec2  &GetV()       const { return m_v;            }
   float       GetSize()     const { return m_size;         }
   uint32_t    GetTexture()  const { return m_texture;      }

private:
   float    m_advanceX;
   Vec2     m_middle;
   Vec2     m_u;
   Vec2     m_v;
   float    m_size;
   uint32_t m_texture;
};

//! PrintFont is the class of fonts used by the Print class.
//! Use the Load method to populate the font accoring to a font name and point-size.
class PrintFont : public RefCount
{
   friend class Print;

public:
   PrintFont();
   ~PrintFont();

   //! Specify a range of character values that this font will be used for
   //! This can help to optimise the font, and reduce the amount of texture memory required.
   void UseCharRange(uint32_t from, uint32_t to)
   {
      m_range.push_back(std::pair<uint32_t, uint32_t>(from, to));
   }

   //! Load the font and generate its texture(s).  Only disable mipmap if you are sure this font will be used 2D
   //! and at the correct screen scale.
   void Load(const std::string &name, float pointSize);


   float          GetLineHeight() const   { return m_lineHeight;     }

   const PrintCharInfo &GetCharInfo(uint32_t ch) const
   {
      static PrintCharInfo   Zero;

      std::map<uint32_t, PrintCharInfo>::const_iterator  it = m_charInfo.find(ch);

      if (it == m_charInfo.end())
         return Zero;

      return (*it).second;
   }

   uint32_t GetNumTextures()                 const { return m_texture.size();                            }
   uint32_t GetTextureForChar(uint32_t ch)   const { return (*m_charInfo.find(ch)).second.GetTexture();  }

private:
   uint32_t       GetNumChars()            const    { return m_charInfo.size();   }
   TextureHandle  GetTexture(uint32_t t)   const    { return m_texture[t];        }

   Vec2  GetKerning(uint32_t prev, uint32_t next);

   bool  InRange(uint32_t ch)
   {
      if (m_range.size() == 0)
         return true;

      for (uint32_t i = 0; i < m_range.size(); ++i)
      {
         const std::pair<uint32_t, uint32_t> &range = m_range[i];

         if (ch >= range.first && ch < (range.first + range.second))
            return true;
      }

      return false;
   }

   bool IsValidGlyph(uint32_t ch)
   {
      return m_charInfo.find(ch) != m_charInfo.end();
   }

private:
   std::vector<TextureHandle> m_texture;
   void                       *m_face;       // FT_Font, but users of this class need not know.
   void                       *m_library;    // FT_Library, ditto

   // Font metrics
   float                                        m_lineHeight;
   std::map<uint32_t, PrintCharInfo>            m_charInfo;
   std::vector<std::pair<uint32_t, uint32_t> >  m_range;
};

class Print;

// @cond
struct PrintTraits
{
   typedef Geometry       Base;
   typedef Print          Derived;
   typedef GeometryTraits BaseTraits;
};
// @endcond

//! @addtogroup handles
//! @{
typedef Handle<PrintTraits>    PrintHandle;
//! @}

//! The PrintOptions class is used to configure a Print object.
//!
//! The position of the text can be anchored eTOP_LEFT, eTOP_RIGHT, eBOTTOM_LEFT, eBOTTOM_RIGHT, eCENTER,
//! eTOP_CENTER, eBOTTOM_CENTER, eCENTER_LEFT or eCENTER_RIGHT.
//!
//! Positioning is either line based, or used the bounding box of the text as controlled buy the "useBoundingBox" flag.
//! By default text is aligned according to its baseline.  This can look odd if the text is transformed in 3D, so in this case set
//! the use bounding-box flag.
class PrintOptions
{
   friend class Print;

public:
   enum eAnchor
   {
      eTOP_LEFT,
      eTOP_RIGHT,
      eBOTTOM_LEFT,
      eBOTTOM_RIGHT,
      eCENTER,
      eTOP_CENTER,
      eBOTTOM_CENTER,
      eCENTER_LEFT,
      eCENTER_RIGHT
   };

   PrintOptions() :
      m_useBoundingBox(false),
      m_anchor(eTOP_LEFT)
   {}

   PrintOptions(eAnchor anchor) :
      m_useBoundingBox(false),
      m_anchor(anchor)
   {}

   PrintOptions(eAnchor anchor, bool useBBox) :
      m_useBoundingBox(useBBox),
      m_anchor(anchor)
   {}

   PrintOptions &UseBoundingBox(bool use)       { m_useBoundingBox = use; return *this;   }
   bool GetUseBoundingBox() const               { return m_useBoundingBox;                }

   PrintOptions &SetAnchor(eAnchor anchor)      { m_anchor = anchor; return *this;        }
   eAnchor GetAnchor() const                    { return m_anchor;                        }

   Vec2 AnchorOffset(const Vec2 &mn, const Vec2 &mx) const;

private:
   bool           m_useBoundingBox;
   eAnchor        m_anchor;

   static const PrintOptions  m_default;
};

// @cond
class PrintState
{
public:
   PrintState(float height, const PrintFontHandle &font) :
      m_font(font),
      m_lineHeight(height),
      m_scale(height / font->GetLineHeight()),
      m_previousCh(0),
      m_currentCh(0)
   {}

   void SetPenX(float x)                           { m_penX = x; }
   void SetPenY(float y)                           { m_penY = y; }

   float GetPenX() const                           { return m_penX; }
   float GetPenY() const                           { return m_penY; }
   float GetLineHeight() const                     { return m_lineHeight; }

   const PrintFontHandle   &GetFont() const        { return m_font; }

   void SetPreviousChar(uint32_t ch)               { m_previousCh = ch; }
   void SetCurrentChar(uint32_t ch)                { m_currentCh  = ch; }

   uint32_t GetPreviousChar() const                { return m_previousCh; }
   uint32_t GetCurrentChar()  const                { return m_currentCh; }

   void SetQuad(const Vec4 &bl, const Vec4 &tr)    { m_bl = bl; m_tr = tr; }

   //! Return the bottom left vertex (x, y, u, v)
   const Vec4  &GetBL() const                      { return m_bl;       }

   //! Return the top right vertex (x, y, u, v)
   const Vec4  &GetTR() const                      { return m_tr;       }

   PrintCharInfo  GetCharInfo(uint32_t ch)
   {
      return PrintCharInfo(m_font->GetCharInfo(ch), m_scale);
   }

   PrintCharInfo  GetCharInfo()                    { return GetCharInfo(m_currentCh); }

private:
   PrintFontHandle   m_font;
   float             m_lineHeight;
   float             m_scale;
   float             m_penX;
   float             m_penY;
   Vec4              m_bl;
   Vec4              m_tr;
   uint32_t          m_previousCh;
   uint32_t          m_currentCh;
};
// @endcond

// @cond
class PrintCallback
{
public:
   virtual ~PrintCallback()  {}

   //! Called when rendering of a block of text starts.  May be called several times if the
   //! font uses multiple textures to hold the glyph data
   virtual void OnBegin(PrintState &state);

   //! Called when a newline character is encountered.  Method should handle moving to the
   //! next line.
   virtual void OnNewline(PrintState &state);

   //! Called for each character.  Method should handle moving to the next character position.
   virtual bool OnCharacter(PrintState &state);

   //! Called at the end of a piece of text.  As with OnBegin it might be called several times
   //! for the same block of text.
   virtual void OnEnd(PrintState &state);
};
// @endcond

//! Print objects can optionally have a formatter object.  Formatters are called prior to generating the geometry
//! for the text.  They have free reign to modify the string, and can e.g. format it to fit withing a region, or
//! change the text entirely.
class PrintFormatter
{
public:
   PrintFormatter() :
      m_scale(0.0f)
   {}

   virtual ~PrintFormatter()  {}

   //! Override this method to supply the format behavior
   virtual const std::vector<uint32_t> &Format(const std::vector<uint32_t> &str) = 0;

   void SetFont(float height, const PrintFontHandle &font)
   {
      m_font  = font;
      m_scale = height / font->GetLineHeight();
   }

   //! Get character metrics
   PrintCharInfo  GetCharInfo(uint32_t ch) const
   {
      return PrintCharInfo(m_font->GetCharInfo(ch), m_scale);
   }

   //! Special case -- get the advance distance for character
   float GetAdvanceX(uint32_t ch) const
   {
      return m_font->GetCharInfo(ch).GetAdvanceX() * m_scale;
   }

private:
   PrintFontHandle   m_font;
   float             m_scale;
};

//! A ragged right formatter.  Attempts to split at spaces, but will split words if necessary.
//! Leading spaces at the start of lines are suppressed, unless they immediately follow a newline
//! in which case they are retained (to facilitate a simple paragraph indent mechanism).
class PrintRaggedRight : public PrintFormatter
{
public:
   PrintRaggedRight(float limit) :
      m_limit(limit)
   {
   }

   virtual const std::vector<uint32_t> &Format(const std::vector<uint32_t> &str);

private:
   std::vector<uint32_t>   m_result;
   float                   m_limit;
};

//! @addtogroup scenegraph
//! @{

//! Print objects are a kind of Geometry and so can be placed into the scene-graph.
//! Print objects use fonts of type bsg::PrintFontHandle (not bsg::FontHandle which is for direct 2D text).
class Print : public Geometry
{
   friend class Handle<PrintTraits>;

public:
   virtual ~Print();

   //! Bind to the font with the specified options.  A default material is generated based on the options.
   void SetFont(const PrintFontHandle &font, const PrintOptions &options = PrintOptions::m_default);

   //! Bind to a font with the specified options and effect.  Materials are generated internally based on the effect.
   void SetFont(const PrintFontHandle &font, const EffectHandle &effect, const PrintOptions &options = PrintOptions::m_default);

   //! Sets the callback for the print object.  This is called during text rendering and provideds the
   //! application with control over how the text is layed out.  The default layout is left-to-right, tto
   //! to bottom and single spaced.
   void SetCallback(PrintCallback *newCallback);

   //! Sets the formatter callback -- used to format text prior to conversion to geometry e.g. to perform justification
   void SetFormatter(PrintFormatter *newFomatter);

   //! Gets the formatter (or NULL if none set)
   PrintFormatter *GetFormatter() { return m_formatter; }

   //! Specifies the text string and initial position.
   //! @{
   void SetText(const std::string &text, float height = 1.0f, const Vec2 &position = s_defaultPosition);
   void SetText(const PrintString &text, eStringEncoding enc, float height = 1.0f, const Vec2 &position = s_defaultPosition);
   //! @}

   //! Expose uniforms for this text
   template <class T>
   AnimTarget<T> &GetUniform(const std::string &name)
   {
      return m_material->GetUniform<T>(name);
   }

   //! Set a uniform
   template <class T>
   void SetUniform(const std::string &name, const T &val)
   {
      m_material->GetUniform<T>(name).Set(val);
   }

   //! Set a texture sampler
   void SetTexture(const std::string &name, const TextureHandle &texture)
   {
      m_material->SetTexture(name, texture);
   }

   void  GetBounds(Vec2 &mn, Vec2 &mx) const;
   float GetWidth()                    const;
   float GetHeight()                   const;

   static void DeleteDefaultEffect();

protected:
   Print();

private:
   void Init(const PrintFontHandle &font, const EffectHandle &effect, const PrintOptions &options, bool bespoke = false);
   EffectHandle GetDefaultEffect() const;
   void Reset(float height, const Vec2 &position);

private:
   static Vec2                   s_defaultPosition;

   PrintString                   m_text;
   eStringEncoding               m_encoding;

   PrintFontHandle               m_font;
   EffectHandle                  m_effect;
   MaterialHandle                m_material;
   std::vector<SurfaceHandle>    m_surfaces;

   Vec2                          m_min;
   Vec2                          m_max;
   bool                          m_bespoke;
   PrintOptions                  m_options;
   PrintCallback                 *m_callback;
   PrintFormatter                *m_formatter;
};

//! @}

}

#endif /* __BSG_PRINT_H__ */

#endif /* BSG_STAND_ALONE */
