/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include <stdint.h>
#include <stdio.h>

#include "interface/khronos/common/khrn_api_command_ids.h"

#ifdef GL_SIDE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
typedef int8_t   GLbyte;
typedef uint8_t  GLubyte;
typedef int16_t  GLshort;
typedef uint16_t GLushort;
typedef int32_t  GLint;
typedef uint32_t GLuint;
typedef float    GLfloat;
#endif

#include <vector>
#include <memory>

enum eDataType
{
   eFUNCTION,
   eINT8,
   eINT16,
   eINT32,
   eUINT8,
   eUINT16,
   eUINT32,
   eFLOAT,
   eVOID_PTR,
   eCHAR_PTR,
   eBYTE_ARRAY
};

class Remote;
class Comms;

class PacketItem
{
public:
   static size_t GetPointerSize() { return 4; }

   PacketItem(eDataType t, uint32_t data);
   PacketItem(eDataType t, uint32_t data, uint32_t numBytes);
   PacketItem(GLbyte b);
   PacketItem(GLubyte b);
   PacketItem(GLshort s);
   PacketItem(GLushort u);
   PacketItem(GLint i);
   PacketItem(GLuint u);
   PacketItem(long int s);
   PacketItem(GLfloat f);
   PacketItem(void *p);
   PacketItem(const void *p);
   PacketItem(const char *c);
   PacketItem(void *a, uint32_t numBytes);

   void Send(Remote *rem);
   void Send(Comms *comms);

   eDataType Type() const { return m_type; }

   eGLCommand  GetFunc() const { return (eGLCommand)m_data; }
   bool        GetBoolean() const { return m_data != 0; }
   int8_t      GetInt8() const { return (int8_t)m_data; }
   int16_t     GetInt16() const { return (int16_t)m_data; }
   int32_t     GetInt32() const { return (int32_t)m_data; }
   uint8_t     GetUInt8() const { return (uint8_t)m_data; }
   uint16_t    GetUInt16() const { return (uint16_t)m_data; }
   uint32_t    GetUInt32() const { return (uint32_t)m_data; }
   float       GetFloat() const { return *((float*)(void*)&m_data); }
   void       *GetVoidPtr() const { return (void*)m_data; }
   char       *GetCharPtr() const { return (char*)m_data; }
   uint32_t    GetArray(uint8_t **ptr) const
   {
      if (m_numBytes == 0)
         *ptr = NULL;
      else
         *ptr = (uint8_t*)m_data;
      return m_numBytes;
   }
   void *GetArrayPtr(uint32_t *size = NULL) const
   {
      void *ptr = NULL;

      if (size != NULL)
         *size = m_numBytes;

      if (m_numBytes != 0)
         ptr = (void*)m_data;

      return ptr;
   }

private:
   eDataType   m_type;
   uint32_t    m_data;
   uint32_t    m_numBytes;
};

enum ePacketType
{
   eUNKNOWN,
   eAPI_FUNCTION,
   eRET_CODE,
   eGL_ERROR,
   eRESPONSE,
   eBUFFER,
   eUNIFORMS,
   ePERF_DATA,
   ePERF_DATA_NAMES,
   eINIT,
   eSTATE,
   eBOTTLENECKSET,
   ePERFDATASET,
   eMEMORY,
   eSETVARSET,
   eSHADER_CHANGE_ACK,
   eEVENT_DATA,
   eBACKTRACE,
   eREINIT,
   eTHREAD_CHANGE,
   eLAST_PACKET_TYPE /* Insert new ones before this */
};

class Packet
{
public:
   Packet(ePacketType type = eUNKNOWN) : m_type(type) {}
   ~Packet() {}

   void Reset() { m_type = eUNKNOWN; m_items.clear(); m_buffers.clear(); }
   bool IsValid() const { return m_type != eUNKNOWN; }

   std::shared_ptr<uint8_t> AddBuffer(size_t size);
   void AddItem(const PacketItem &item) { m_items.push_back(item); }
   void Send(Remote *rem);
   void Send(Comms *comms);

   void SetType(ePacketType t) { m_type = t; }
   ePacketType Type() const { return m_type; }
   uint32_t NumItems() const { return m_items.size(); }
   const PacketItem &Item(uint32_t i) const { return m_items[i]; }

private:
   ePacketType             m_type;
   std::vector<PacketItem> m_items;
   std::vector<std::shared_ptr<uint8_t>> m_buffers;
};
