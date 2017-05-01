/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "packetreader.h"

#include "packet.h"
#include "datasource.h"

#include <assert.h>

#ifdef BIG_ENDIAN_CPU
#define TO_LE_W(w) \
{ \
   uint32_t tmp = *((uint32_t*)&w);\
   uint8_t *t = (uint8_t*)&tmp;\
   uint8_t *p = (uint8_t*)&w;\
   p[0] = t[3];\
   p[1] = t[2];\
   p[2] = t[1];\
   p[3] = t[0];\
}
#else
#define TO_LE_W(w)
#endif

int32_t PacketReader::Read32(DataSource &dataSource)
{
   uint8_t  buffer[4];

   int n = dataSource.Read(buffer, sizeof(buffer));
   if (n != sizeof(buffer))
      return -1;

   TO_LE_W(buffer);

   return To32(buffer);
}

bool PacketReader::Read(Packet *p, DataSource &dataSource)
{
   p->Reset();

   int32_t type = Read32(dataSource);
   if (type < 0 || type >= eLAST_PACKET_TYPE)
      return false;

   p->SetType((ePacketType)type);

   int32_t numItems = Read32(dataSource);
   if (numItems == -1)
      return false;

   for (int32_t i = 0; i < numItems; i++)
   {
      int32_t itemType = Read32(dataSource);
      if (itemType == -1)
         return false;

      int32_t numBytes = Read32(dataSource);
      if (numBytes == -1)
         return false;

      if (numBytes > 0)
      {
         if (itemType == eBYTE_ARRAY || itemType == eCHAR_PTR)
         {
            std::shared_ptr<uint8_t> buffer = p->AddBuffer(numBytes);
            int n = dataSource.Read(buffer.get(), numBytes);
            if (n == (int)numBytes)
            {
               p->AddItem(PacketItem((eDataType) itemType,
                     (uintptr_t) buffer.get(), numBytes));
            }
            else
               return false;
         }
         else if (itemType == eVOID_PTR)
         {
            // use the original pointer
            uintptr_t ptr = 0;
            assert((size_t)numBytes == PacketItem::GetPointerSize());
            assert((size_t)numBytes <= sizeof(ptr));
            int n = dataSource.Read(&ptr, numBytes);
            if (n == (int)numBytes)
               p->AddItem(PacketItem((eDataType)itemType, ptr, numBytes));
            else
               return false;
         }
         else
         {
            assert(numBytes <= 4);
            uint32_t data = Read32(dataSource);
            p->AddItem(PacketItem((eDataType)itemType, data, 4));
         }
      }
      else
      {
         p->AddItem(PacketItem((eDataType)itemType, 0, 0));
      }
   }

   return true;
}
