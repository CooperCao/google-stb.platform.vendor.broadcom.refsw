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
#include "bsg_effect.h"
#include "bsg_application.h"
#include "bsg_pass.h"
#include "bsg_exception.h"
#include "bsg_parse_utils.h"
#include "bsg_sampler_semantics.h"

#include <string>
#include <iostream>
#include <istream>
#include <fstream>

using namespace std;


namespace bsg
{

void EffectOptions::ToLines(std::vector<std::string> &lines) const
{
   if (m_sortOrder != eAUTO)
   {
      stringstream ss;

      ss << "SortOrder = ";

      switch (m_sortOrder)
      {
      case eNO_SORT:
         ss << "NONE";
         break;

      case eFRONT_TO_BACK:
         ss << "FRONT_TO_BACK";
         break;

      case eBACK_TO_FRONT:
         ss << "BACK_TO_FRONT";
         break;

      case eAUTO:
         break;
      }

      lines.push_back(ss.str());
   }

   if (m_sortDepthOverride)
   {
      stringstream   ss;

      ss << "SortDepth = " << m_sortDepth;

      lines.push_back(ss.str());
   }
}

std::vector<std::string> Effect::m_includePaths;
std::vector<std::string> Effect::m_defaultDefines;

Effect::Effect() :
   m_loaded(false),
   m_lineNumber(1),
   m_state(ePASS_OR_OPTIONS),
   m_passNumber(-1)
{
   if (m_includePaths.size() == 0)
      m_includePaths.push_back(".");

   Reset();
}

Effect::~Effect()
{
   Reset();
}

void Effect::Reset()
{
   for (uint32_t p = 0; p < NumPasses(); p++)
      delete m_passes[p];

   m_passes.clear();

   m_loaded = false;
   m_lineNumber = 1;
   m_state = ePASS_OR_OPTIONS;
   m_passNumber = -1;
   m_options = EffectOptions();
}

void Effect::Load(std::istream &is, const std::vector<std::string> &defines)
{
   if (!is)
      BSG_THROW("Invalid stream");

   if (m_loaded)
      BSG_THROW("Effects can only be loaded once");

   Reset();

   m_defines = defines;

   m_passNumber = -1;
   m_state = ePASS_OR_OPTIONS;
   m_lineNumber = 1;

   while (!is.eof())
   {
      string line;

      getline(is, line);
      m_lineNumber++;

      line = ParseUtils::StripWhite(line);
      if (ParseUtils::StartsWith(line, "#include"))
         ProcessInclude(line);
      else
         ParseLine(line);
   }

   for (uint32_t p = 0; p < NumPasses(); p++)
      m_passes[p]->CacheSemantics();

   m_loaded = true;
}

void Effect::Load(const std::string &pathName, const std::vector<std::string> &defines)
{
   ifstream is;

   is.open(pathName.c_str());

   if (!is.is_open())
   {
      is.open(Application::Instance()->FindResource(pathName).c_str());

      if (!is.is_open())
         BSG_THROW("Can't find or can't open effect file " << pathName);
   }

   Load(is, defines);
}

void Effect::Read(const char *effect, const std::vector<std::string> &defines)
{
   istringstream is(effect, istringstream::in);
   Load(is, defines);
}

void Effect::ProcessInclude(const std::string &line)
{
   // Skip the #include
   std::string path       = ParseUtils::StripWhite(line.substr(8, string::npos));
   uint32_t    incLineNum = 0;

   std::string goodPath = Application::Instance()->FindResource(path);

   if (goodPath != "")
   {
      ifstream incStr(goodPath.c_str());

      if (!incStr)
         BSG_THROW("Couldn't find included file " << path);

      while (!incStr.eof())
      {
         string ln;
         getline(incStr, ln);
         incLineNum++;

         try
         {
            ln = ParseUtils::StripWhite(ln);
            if (ParseUtils::StartsWith(ln, "#include"))
               ProcessInclude(ln);
            else
               ParseLine(ln);
         }
         catch (Exception e)
         {
            BSG_THROW(e.Message() << " whilst processing '" << goodPath << "' at line " << incLineNum);
         }
      }
   }
   else
   {
      BSG_THROW("Cannot find include file '" << path << "' used at line " << m_lineNumber);
   }
}

Pass *Effect::GetPass(uint32_t p) const
{
   if (p < NumPasses())
      return m_passes[p];

   BSG_THROW("Invalid parameter");
   return NULL;
}

void Effect::ParseOptions(const std::string &str)
{
   int start = 0;
   int semi;

   do
   {
      semi = str.find_first_of(";", start);
      if (semi != (int)str.npos)
      {
         std::string line = str.substr(start, semi - start);
         
         int equal = line.find_first_of("=");
         string l = ParseUtils::StripWhite(line.substr(0, equal - 1));
         string r = ParseUtils::StripWhite(line.substr(equal + 1, line.npos));

         if (l == "SortOrder")
         {
            if (r == "FRONT_TO_BACK")
               m_options.SetSortOrder(EffectOptions::eFRONT_TO_BACK);
            else if (r == "BACK_TO_FRONT")
               m_options.SetSortOrder(EffectOptions::eBACK_TO_FRONT);
            else if (r == "NONE")
               m_options.SetSortOrder(EffectOptions::eNO_SORT);
            else if (r == "AUTO")
               m_options.SetSortOrder(EffectOptions::eAUTO);
            else
               BSG_THROW("Parse error on line " << m_lineNumber);
         }
         else if (l == "SortDepth")
         {
            if (r != "AUTO")
            {
               m_options.SetSortDepthOverride(true);
               stringstream   ss(r);
               float depth;
               ss >> depth;
               m_options.SetSortDepth(depth);
            }
         }
         else
            BSG_THROW("Parse error on line " << m_lineNumber);
      }

      start = semi + 1;
   }
   while (semi != (int)str.npos);
}

// Counts the number of excess open/close braces in a line
static void CountBraces(uint32_t &numOpen, uint32_t &numClose, const std::string &line)
{
   uint32_t slash = 0;     // Number of consecutive slashes.

   numOpen  = 0;
   numClose = 0;

   for (uint32_t i = 0; i < line.size() && slash != 2; ++i)
   {
      if (line[i] == '/')
      {
         slash++;
      }
      else
      {
         slash = 0;

         if (line[i] == '{')
            numOpen++;

         if (line[i] == '}')
            numClose++;
      }
   }

   if (numOpen > numClose)
   {
      numOpen = numOpen - numClose;
      numClose = 0;
   }
   else
   {
      numClose = numClose - numOpen;
      numOpen = 0;
   }
}

void Effect::ParseLine(const std::string &in)
{
   std::string line = ParseUtils::StripWhite(in);

   if (line.length() == 0) // Blank line
      return;

   if (line.length() >= 2 && line[0] == '/' && line[1] == '/')    // Comment
      return;

   istringstream iss(line, istringstream::in);
   string token;

   switch (m_state)
   {
   case ePASS_OR_OPTIONS :
      iss >> token;
      if (token == "PASS")
      {
         iss >> m_passNumber;

         if (iss.fail())
            BSG_THROW("Parse error on line " << m_lineNumber);

         if (m_passNumber + 1 > m_passes.size())
         {
            m_passes.resize(m_passNumber + 1);
            m_passes[m_passNumber] = new Pass;
         }
         m_state = ePASS_OPEN_BRACE;
      }
      else if (token == "OPTIONS")
      {
         if (iss.fail())
            BSG_THROW("Parse error on line " << m_lineNumber);

         m_state = eCAPTURE_OPTIONS;
         m_captureBlock = token;
         m_captureBraces = 0;
         m_captureText = "";
      }
      else
         BSG_THROW("Parse error on line " << m_lineNumber);
      break;
   case ePASS_OPEN_BRACE:
      iss >> token;
      if (token == "{")
         m_state = eIN_PASS;
      else
         BSG_THROW("Parse error on line " << m_lineNumber);
      break;
   case eIN_PASS:
      iss >> token;
      if (token == "}")
      {
         m_passes[m_passNumber]->Program().SetPrograms(m_vertShader, m_fragShader, m_defines);
         m_state = ePASS_OR_OPTIONS;
      }
      else if (token == "SEMANTICS" || token == "STATE" || token == "SAMPLER_2D" || token == "SAMPLER_CUBE" ||
#ifdef BSG_USE_ES3
               token == "SAMPLER_3D" ||
               token == "SAMPLER_2D_ARRAY" ||
#endif
               token == "VERTEX_SHADER" || token == "FRAGMENT_SHADER")
      {
         m_captureBlock = token;
         m_state = eCAPTURE_PASS;
         m_captureBraces = 0;
         m_captureText = "";
         if (token == "SAMPLER_2D" ||
#ifdef BSG_USE_ES3
             token == "SAMPLER_3D" ||
             token == "SAMPLER_2D_ARRAY" ||
#endif
             token == "SAMPLER_CUBE")
            iss >> m_samplerName;
      }
      else
         BSG_THROW("Parse error on line " << m_lineNumber);
      break;
   case eIN_OPTIONS:
      iss >> token;
      if (token == "}")
      {
         m_state = ePASS_OR_OPTIONS;
      }
      else if (token == "SortOrder")
      {
         m_captureBlock = token;
         m_state = eCAPTURE_OPTIONS;
         m_captureBraces = 0;
         m_captureText = "";
      }
      else
         BSG_THROW("Parse error on line " << m_lineNumber);
      break;
   case eCAPTURE_PASS:
      {
         uint32_t numOpen  = 0;
         uint32_t numClose = 0;
         bool     addLine  = true;

         CountBraces(numOpen, numClose, line);

         if (numOpen > 0)
         {
            addLine = m_captureBraces > 0;
            m_captureBraces += numOpen;
         }
         else if (numClose > 0)
         {
            m_captureBraces -= numClose;
            if (m_captureBraces == 0)  // End of block?
            {
               if (m_captureBlock == "SEMANTICS")
                  m_passes[m_passNumber]->Semantics().Parse(m_captureText);
               if (m_captureBlock == "STATE")
                  m_passes[m_passNumber]->State().Parse(m_captureText);
               else if (m_captureBlock == "VERTEX_SHADER")
                  m_vertShader = m_captureText;
               else if (m_captureBlock == "FRAGMENT_SHADER")
                  m_fragShader = m_captureText;
               else if (m_captureBlock == "SAMPLER_2D")
                  m_passes[m_passNumber]->Samplers().Parse(m_captureText, SamplerSemantics::eSAMPLER_2D, m_samplerName);
#ifdef BSG_USE_ES3
               else if (m_captureBlock == "SAMPLER_3D")
                  m_passes[m_passNumber]->Samplers().Parse(m_captureText, SamplerSemantics::eSAMPLER_3D, m_samplerName);
               else if (m_captureBlock == "SAMPLER_2D_ARRAY")
                  m_passes[m_passNumber]->Samplers().Parse(m_captureText, SamplerSemantics::eSAMPLER_2D_ARRAY, m_samplerName);
#endif
               else if (m_captureBlock == "SAMPLER_CUBE")
                  m_passes[m_passNumber]->Samplers().Parse(m_captureText, SamplerSemantics::eSAMPLER_CUBE, m_samplerName);
               m_state = eIN_PASS;
               addLine = false;
            }
         }

         if (addLine)
            m_captureText += line + "\n";

      }
      break;
   case eCAPTURE_OPTIONS:
      if (line[0] == '{')
      {
         if (m_captureBraces > 0)
            m_captureText += line + "\n";
         m_captureBraces++;
      }
      else if (line[0] == '}')
      {
         m_captureBraces--;
         if (m_captureBraces == 0)
         {
            if (m_captureBlock == "OPTIONS")
               ParseOptions(m_captureText);
            m_state = ePASS_OR_OPTIONS;
         }
         else
            m_captureText += line + "\n";
      }
      else
         m_captureText += line + "\n";
      break;
   }
}

}
