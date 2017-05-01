/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_pass.h"
#include "bsg_exception.h"

namespace bsg
{

void Pass::CacheSemantics()
{
   typedef std::map<std::string, EffectSemantics::eSemantic> Map;

   const Map &semantics = m_semantics.RequiredSemantics();

   for (Map::const_iterator i = semantics.begin(); i != semantics.end(); ++i)
   {
      EffectSemantics::eSemantic semantic = (*i).second;
      std::string                name     = (*i).first;

      if (semantic >= EffectSemantics::eVATTR_FIRST)
      {
         GLint index = m_program.GetAttribLocation(name);

         if (index < 0)
         {
            //BSG_THROW("Attribute " << name << " not found in vertex shader");
         }
         else
         {
            m_attributeSemantics.push_back(AttributeSemantic(semantic, index));
         }
      }
   }
}

}
