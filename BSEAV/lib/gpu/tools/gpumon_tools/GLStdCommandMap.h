/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "api_command_ids.h"
#include "packet.h"

#include <string>
#include <map>

class GLStdCommandMapper
{
public:
   static eGLCommand  FromString(const std::string &str);
   static std::string ToString(eGLCommand cmd);

   static std::string Signature(eGLCommand cmd);
   static std::string Signature(const std::string &str);

   static std::string ReturnSignature(eGLCommand cmd);
   static std::string ReturnSignature(const std::string &str);

   static int32_t NumCommands();

private:
   static void Init();

private:
   static bool                              m_inited;
   static std::map<std::string, eGLCommand> m_map;
};
