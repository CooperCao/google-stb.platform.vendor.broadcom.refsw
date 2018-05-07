/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "api_command_ids.h"
#include "packet.h"

#include <string>
#include <map>

class GLStdCommand
{
public:
   enum eMarkers
   {
      SKIPPED = 1
   };

public:
   GLStdCommand() : m_command(cmd_none) {}
   GLStdCommand(const Packet &packet, uint32_t context);

   std::string        AsString() const;
   std::string        AsHtmlString() const;

   static std::string MapEnum(uint32_t e, eGLCommand cmd);
   static std::string MapEGLEnum(uint32_t e, eGLCommand cmd);
   static const std::map<std::string, uint32_t> &EnumStringMapTable();

   eGLCommand    Command() const { return m_command; }
   uint32_t      Context() const { return m_context; }

   std::string   StringArg(uint32_t num) const;
   uint32_t      UIntArg(uint32_t num) const;
   int32_t       IntArg(uint32_t num) const;
   float         FloatArg(uint32_t num) const;
   std::string   StringBufferArg(uint32_t num) const;
   void*         VoidPtrArg(uint32_t num) const;
   uint32_t      NumArgs() const;
   void          Mark(uint32_t markerBits);
   uint32_t      Markers() const;

   const Packet &GetPacket() const { return m_packet; }
   std::string   MakeArgStr(uint8_t sig, const PacketItem &item) const;

private:
   std::string   MakeBitfieldStr(uint32_t val) const;

private:
   Packet                                 m_packet;
   uint32_t                               m_context;
   eGLCommand                             m_command;
   uint32_t                               m_markerBits;
   static std::map<std::string, uint32_t> m_allEnums;
};
