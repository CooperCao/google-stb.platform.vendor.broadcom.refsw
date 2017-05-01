/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "pg_info.h"

namespace pg
{

// Look for [tok, tok,...] patterns where tok can be
// AD, SL, S
void Description::ParseString(const std::string &desc)
{
   enum eState
   {
      eNORMAL,
      eINBRACKET,
      eFOUND_S,
      eFOUND_A
   };

   eState state = eNORMAL;

   for (uint32_t i = 0; i < desc.size(); ++i)
   {
      char  c = desc[i];

      switch (state)
      {
      case eNORMAL:
         if (c == '[')
            state = eINBRACKET;
         else
            m_result += c;
         break;

      case eINBRACKET:
         if (c == 'S')
            state = eFOUND_S;
         else if (c == 'A')
            state = eFOUND_A;
         break;

      case eFOUND_S:
         if (c == 'L')
            m_hasSigning = true;
         else
            m_hasSubtitles = true;

         state = eINBRACKET;
         break;

      case eFOUND_A:
         if (c == 'D')
         {
            m_hasAudioDescription = true;
         }
         state = eINBRACKET;
         break;
      }

      switch (state)
      {
      case eINBRACKET:
      case eFOUND_S:
      case eFOUND_A:
         if (c == ']')
            state = eNORMAL;
         break;

      case eNORMAL:
         break;
      }
   }
}

Description::Description() :
   m_hasAudioDescription(false),
   m_hasSubtitles(false),
   m_hasSigning(false)
{}

Description::Description(const std::string &desc) :
   m_hasAudioDescription(false),
   m_hasSubtitles(false),
   m_hasSigning(false)
{
   ParseString(desc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string ProgramInfo::StripTrailingDots(const std::string &str) const
{
   if (str == "")
      return "";

   std::string::const_iterator   last = str.end();

   --last;

   while (*last == '.')
   {
      if (last != str.begin())
         --last;
      else
         return "";
   }

   return std::string(str.begin(), last + 1);
}

uint32_t GridInfo::AddChannel(const ChannelInfo &channel)
{
   uint32_t chIndx = m_channels.size();
   m_channels.push_back(channel);
   return chIndx;
}

}
