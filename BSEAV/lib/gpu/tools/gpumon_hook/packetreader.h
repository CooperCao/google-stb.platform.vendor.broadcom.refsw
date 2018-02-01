/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include <stdint.h>

class Packet;
class DataSource;

/*
 * Synchronously read complete packets from a DataSource
 */
class PacketReader
{
public:
   PacketReader() {}
   ~PacketReader() {}

   static int32_t Read32(DataSource &dataSource);
   static bool Read(Packet *p, DataSource &dataSource);

private:
   static uint32_t To32(uint8_t *ptr) { return *(uint32_t*)ptr; }
};
