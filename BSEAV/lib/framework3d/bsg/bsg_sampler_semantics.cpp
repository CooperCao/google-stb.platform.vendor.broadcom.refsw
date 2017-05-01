/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_sampler_semantics.h"
#include "bsg_parse_utils.h"
#include "bsg_exception.h"

#include <string>
#include <sstream>

#include <stdint.h>

using namespace std;

namespace bsg
{

void GetToken(istringstream &iss, string &token, char delim)
{
   getline(iss, token, delim);

   token = ParseUtils::StripWhite(token);
}

GLenum StringToWrap(const string &s)
{
   if (s == "CLAMP")
      return GL_CLAMP_TO_EDGE;

   if (s == "REPEAT")
      return GL_REPEAT;

   if (s == "MIRROR")
      return GL_MIRRORED_REPEAT;

   return GL_FALSE;
}

GLenum StringToFilter(const string &s)
{
   if (s == "NEAREST")
      return GL_NEAREST;

   if (s == "LINEAR")
      return GL_LINEAR;

   if (s == "NEAREST_MIPMAP_NEAREST")
      return GL_NEAREST_MIPMAP_NEAREST;

   if (s == "NEAREST_MIPMAP_LINEAR")
      return GL_NEAREST_MIPMAP_LINEAR;

   if (s == "LINEAR_MIPMAP_NEAREST")
      return GL_LINEAR_MIPMAP_NEAREST;

   if (s == "LINEAR_MIPMAP_LINEAR")
      return GL_LINEAR_MIPMAP_LINEAR;

   return GL_FALSE;
}

static GLenum Target(SamplerSemantics::eType target)
{
   switch (target)
   {
   case SamplerSemantics::eSAMPLER_2D :
      return GL_TEXTURE_2D;

#ifdef BSG_USE_ES3
   case SamplerSemantics::eSAMPLER_3D :
      return GL_TEXTURE_3D;

   case SamplerSemantics::eSAMPLER_2D_ARRAY :
      return GL_TEXTURE_2D_ARRAY;
#endif

   case SamplerSemantics::eSAMPLER_CUBE :
      return GL_TEXTURE_CUBE_MAP;

   default: break;
   }

   return GL_FALSE;
}

void SamplerSemantics::Parse(const string &body, SamplerSemantics::eType target, const string &name)
{
   istringstream  iss(body);
   string         token;
   string         equal;
   string         semi;

   int32_t  unitValue         = -1;
   GLenum   targetValue       = Target(target);
   GLenum   wrapSValue        = GL_FALSE;
   GLenum   wrapTValue        = GL_FALSE;
   GLenum   minFilterValue    = GL_FALSE;
   GLenum   magFilterValue    = GL_FALSE;

   while (!iss.eof())
   {
      GetToken(iss, token, '=');

      if (token == "Unit")
      {
         string unit;
         GetToken(iss, unit, ';');

         istringstream ss(unit);
         ss >> unitValue;
      }
      else if (token == "Wrap")
      {
         string wrapS;
         string wrapT;

         GetToken(iss, wrapS, ',');
         wrapSValue = StringToWrap(wrapS);

         GetToken(iss, wrapT, ';');
         wrapTValue = StringToWrap(wrapT);
      }
      else if (token == "Filter")
      {
         string minFilter;
         string magFilter;

         GetToken(iss, minFilter, ',');
         minFilterValue = StringToFilter(minFilter);

         GetToken(iss, magFilter, ';');
         magFilterValue = StringToFilter(magFilter);
      }
   }

   if (unitValue < 0)
      BSG_THROW("Texture unit incorrect or missing");

   if (wrapSValue == GL_FALSE)
      BSG_THROW("Texture Wrap S incorrect or missing");

   if (wrapTValue == GL_FALSE)
      BSG_THROW("Texture Wrap T incorrect or missing");

   if (minFilterValue == GL_FALSE)
      BSG_THROW("Texture MinFilter incorrect or missing");

   if (magFilterValue == GL_FALSE)
      BSG_THROW("Texture MagFilter incorrect or missing");

   this->m_semantics[name] = GLSamplerState(unitValue, targetValue, wrapSValue, wrapTValue, minFilterValue, magFilterValue);
}

}
