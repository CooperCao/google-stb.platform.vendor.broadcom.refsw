/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>

#include "api_command_ids.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES   1
#endif

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#include <GLES3/gl31.h>
// Need this until we are using the 3.2 headers
typedef void (GL_APIENTRY  *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
#include <GLES3/gl3ext.h>
#include <GLES3/gl3ext_brcm.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#include <vector>
#include <string>

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
   /**
    * @brief Set pointer size on target platform or in monitor dump file.
    *
    * The sizeof(void*) on the platform or in a monitor dump must not be
    * greater than the sizeof(void*) on the gpu monitor/replay system.
    *
    * @param size Size of void* pointer on target platform
    *             or in a monitor dump file in bytes
    * @return True on success, false if a requested size was greater than
    *         sizeof(void*).
    */
   static bool SetPointerSize(size_t size);

   PacketItem(eDataType t, uintptr_t data, uint32_t numBytes);
   PacketItem(GLbyte b);
   PacketItem(GLubyte b);
   PacketItem(GLshort s);
   PacketItem(GLushort u);
   PacketItem(GLint i);
   PacketItem(GLuint64 u);
   PacketItem(GLuint u);
   PacketItem(long int s);
   PacketItem(GLfloat f);
   PacketItem(void *p);
   PacketItem(void (*p)());
   PacketItem(GLDEBUGPROC p);
   PacketItem(const void *p);
   PacketItem(const char *c);
   PacketItem(void *a, uint32_t numBytes);

   void Send(Remote *rem);
   void Send(Comms *comms);

   eDataType Type() const { return m_type; }

   eGLCommand  GetFunc() const { return (eGLCommand)m_data.i; }
   bool        GetBoolean() const { return m_data.i != 0; }
   int8_t      GetInt8() const { return (int8_t)m_data.i; }
   int16_t     GetInt16() const { return (int16_t)m_data.i; }
   int32_t     GetInt32() const { return (int32_t)m_data.i; }
   uint8_t     GetUInt8() const { return (uint8_t)m_data.i; }
   uint16_t    GetUInt16() const { return (uint16_t)m_data.i; }
   uint32_t    GetUInt32() const { return (uint32_t)m_data.i; }
   float       GetFloat() const { return m_data.f; }
   void       *GetVoidPtr() const { return m_data.p; }
   char       *GetCharPtr() const { return (char*)m_data.p; }
   uint32_t    GetArray(uint8_t **ptr) const
   {
      if (m_numBytes == 0)
         *ptr = NULL;
      else
         *ptr = (uint8_t*)m_data.p;
      return m_numBytes;
   }
   void *GetArrayPtr(uint32_t *size = NULL) const
   {
      void *ptr = NULL;

      if (size != NULL)
         *size = m_numBytes;

      if (m_numBytes != 0)
         ptr = m_data.p;

      return ptr;
   }

   void DebugPrint() const;

private:
   /**
    * @brief Pointer size on target platform or in monitor dump file.
    */
   static size_t s_pointerSize;

   eDataType   m_type;
   union Data
   {
      uint32_t    i;
      float       f;
      void       *p;
      Data() {}
      Data(uint32_t x) : i(x) {}
      Data(float x) : f(x) {}
      Data(void *x) : p(x) {}
   };
   Data m_data;
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
   eBUFFER_OBJECT_DATA,
   ePIQ_DATA,
   eSYNC_OBJ_DATA,
   eQUERY_OBJ_DATA,
   eVERTEX_ARRAY_OBJ_DATA,
   ePROGRAM_PIPELINE_DATA,
   eINFO_DATA,
   eFRAMEBUFFER_INFO,
   eRENDERBUFFER_INFO,
   eLAST_PACKET_TYPE /* Insert new ones before this */
};

class Packet
{
public:
   Packet(ePacketType type = eUNKNOWN) : m_type(type) {}
   ~Packet() {}

   void Reset() { m_type = eUNKNOWN; m_items.clear(); }
   bool IsValid() const { return m_type != eUNKNOWN; }

   void AddItem(const PacketItem &item) { m_items.push_back(item); }
   void Send(Remote *rem);
   void Send(Comms *comms);

   void SetType(ePacketType t) { m_type = t; }
   ePacketType Type() const { return m_type; }
   uint32_t NumItems() const { return m_items.size(); }
   const PacketItem &Item(uint32_t i) const { return m_items[i]; }

   void DebugPrint() const;

private:
   ePacketType             m_type;
   std::vector<PacketItem> m_items;
};

#endif /* __PACKET_H__ */
