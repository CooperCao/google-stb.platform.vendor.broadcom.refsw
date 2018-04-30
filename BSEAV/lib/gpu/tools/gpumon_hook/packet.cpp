/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "packet.h"

#include "datasink.h"

#include <stdio.h>
#include <memory.h>
#include <assert.h>

static const char *PacketTypeToName(ePacketType t)
{
   switch (t)
   {
   case eUNKNOWN             : return "eUNKNOWN";
   case eAPI_FUNCTION        : return "eAPI_FUNCTION";
   case eRET_CODE            : return "eRET_CODE";
   case eGL_ERROR            : return "eGL_ERROR";
   case eRESPONSE            : return "eRESPONSE";
   case eBUFFER              : return "eBUFFER";
   case eUNIFORMS            : return "eUNIFORMS";
   case ePERF_DATA           : return "ePERF_DATA";
   case ePERF_DATA_NAMES     : return "ePERF_DATA_NAMES";
   case eINIT                : return "eINIT";
   case eSTATE               : return "eSTATE";
   case eBOTTLENECKSET       : return "eBOTTLENECKSET";
   case ePERFDATASET         : return "ePERFDATASET";
   case eMEMORY              : return "eMEMORY";
   case eSETVARSET           : return "eSETVARSET";
   case eSHADER_CHANGE_ACK   : return "eSHADER_CHANGE_ACK";
   case eEVENT_DATA          : return "eEVENT_DATA";
   case eBACKTRACE           : return "eBACKTRACE";
   case eREINIT              : return "eREINIT";
   case eTHREAD_CHANGE       : return "eTHREAD_CHANGE";
   default                   : return "ERROR";
   }
};

static const char *ItemTypeToName(eDataType t)
{
   switch(t)
   {
   case eFUNCTION : return "eFUNCTION";
   case eINT8 : return "eINT8";
   case eINT16 : return "eINT16";
   case eINT32 : return "eINT32";
   case eUINT8 : return "eUINT8";
   case eUINT16 : return "eUINT16";
   case eUINT32 : return "eUINT32";
   case eFLOAT : return "eFLOAT";
   case eVOID_PTR : return "eVOID_PTR";
   case eCHAR_PTR : return "eCHAR_PTR";
   case eBYTE_ARRAY : return "eBYTE_ARRAY";
   default: return "ERROR";
   }
}

/*static*/ size_t PacketItem::s_pointerSize = sizeof(void*);

/*static*/ bool PacketItem::SetPointerSize(size_t size)
{
   if (size <= sizeof(void*)) {
      s_pointerSize = size;
   }
   return s_pointerSize == size;
}

PacketItem::PacketItem(eDataType t, uintptr_t data, uint32_t numBytes) :
   m_type(t), m_numBytes(numBytes)
{
   switch (t) {
   case eFUNCTION:
   case eINT8:
   case eINT16:
   case eINT32:
   case eUINT8:
   case eUINT16:
   case eUINT32:
      m_data.i = (uint32_t)data;
      break;
   case eFLOAT: //data contains binary representation of float!
      m_data.i = (uint32_t)data;
      //m_data.f will now contain correct float value
      break;
   case eVOID_PTR:
   case eCHAR_PTR:
   case eBYTE_ARRAY:
      m_data.p = (void*)data;
      break;
   case eLAST_ITEM_TYPE:
   default:
      assert(false); //should not be here
      break;
   }
}

PacketItem::PacketItem(GLbyte b) :
   m_type(eINT8), m_data((uint32_t)b), m_numBytes(4)
{
}

PacketItem::PacketItem(GLubyte b) :
   m_type(eUINT8), m_data((uint32_t)b), m_numBytes(4)
{
}

PacketItem::PacketItem(GLshort s) :
   m_type(eINT16), m_data((uint32_t)s), m_numBytes(4)
{
}

PacketItem::PacketItem(GLushort u) :
   m_type(eUINT16), m_data((uint32_t)u), m_numBytes(4)
{
}

PacketItem::PacketItem(GLint i) :
   m_type(eINT32), m_data((uint32_t)i), m_numBytes(4)
{
}

PacketItem::PacketItem(GLuint u) :
   m_type(eUINT32), m_data((uint32_t)u), m_numBytes(4)
{
}

PacketItem::PacketItem(GLfloat f) :
   m_type(eFLOAT), m_data(f), m_numBytes(4)
{
}

PacketItem::PacketItem(void *p) :
   m_type(eVOID_PTR), m_data(p), m_numBytes(s_pointerSize)
{
}

PacketItem::PacketItem(const void *p) :
   m_type(eVOID_PTR), m_data((void *)p), m_numBytes(s_pointerSize)
{
}

PacketItem::PacketItem(void(*p)()) :
   m_type(eVOID_PTR), m_data((void *)p), m_numBytes(s_pointerSize)
{
}

PacketItem::PacketItem(GLDEBUGPROC p) :
   m_type(eVOID_PTR), m_data((void *)p), m_numBytes(s_pointerSize)
{
}

PacketItem::PacketItem(const char *c) :
   m_type(eCHAR_PTR), m_data((void *)c), m_numBytes(c != 0 ? strlen(c) + 1 : 0)
{
}

PacketItem::PacketItem(void *a, uint32_t numBytes) :
   m_type(eBYTE_ARRAY), m_data(a), m_numBytes(a != 0 ? numBytes : 0)
{
}

PacketItem::PacketItem(long int s) :
   m_type(eVOID_PTR), m_data(s), m_numBytes(s_pointerSize)
{
}

PacketItem::PacketItem(GLuint64 u) :
   m_type(eINT32), m_data((uint32_t)(u & 0xFFFFFFFF)), m_numBytes(4)
{
}

void PacketItem::DebugPrint() const
{
   printf(" Item type %s, numBytes = %d,", ItemTypeToName(m_type), m_numBytes);
   switch (m_type)
   {
   case eCHAR_PTR:
      printf(" data = %p:\"%s\"\n", m_data.p, (char*)m_data.p);
      break;
   case eVOID_PTR:
      printf(" data = %p\n", m_data.p);
      break;
   default:
      printf(" data = %08X\n", m_data.i);
      break;
   }
}

template< typename T >
struct ArrayDeleter
{
  void operator ()( T const * p)
  {
    delete[] p;
  }
};

std::shared_ptr<uint8_t> Packet::AddBuffer(size_t size)
{
   std::shared_ptr<uint8_t> buffer(new uint8_t[size], ArrayDeleter<uint8_t>());
   m_buffers.push_back(buffer);
   return buffer;
}

void Packet::DebugPrint() const
{
   printf("Packet type %s has %d items:\n", PacketTypeToName(m_type), (int)m_items.size());
   for (uint32_t i = 0; i < m_items.size(); i++)
      m_items[i].DebugPrint();
}

void PacketItem::Send(DataSink *sink)
{
   uint32_t t = m_type;
   sink->Write((uint8_t*)&t, sizeof(t));
   sink->Write((uint8_t*)&m_numBytes, sizeof(m_numBytes));
   if (m_type != eBYTE_ARRAY && m_type != eCHAR_PTR)
   {
      sink->Write(&m_data, m_numBytes);
   }
   else
      sink->Write(m_data.p, m_numBytes);
}

void Packet::Send(DataSink *sink)
{
   uint32_t t = m_type;
   sink->Write((uint8_t*)&t, sizeof(t));

   t = m_items.size();
   sink->Write((uint8_t*)&t, sizeof(t));

   std::vector<PacketItem>::iterator iter;
   for (iter = m_items.begin(); iter != m_items.end(); ++iter)
      (*iter).Send(sink);

   sink->Flush();
}
