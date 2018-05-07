/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "perf_event.h"
#include "vcos_chrono.h"
#include <string.h>
#include <string>
#include <array>
#include <vector>
#include <mutex>
#include <atomic>

// A ring-buffer of a certain byte size.
// The caller is expected to check that new data will fit prior to pushing it.
template<size_t Size>
class RingBuffer
{
public:
   static_assert(Size % sizeof(uint32_t) == 0, "RingBuffer must be a multiple of uint32_t size");

   RingBuffer() : m_write((uintptr_t)m_buffer.data()), m_read((uintptr_t)m_buffer.data()) {}

   void Push(uint32_t data)
   {
      uintptr_t end = (uintptr_t)m_buffer.data() + Size;
      uintptr_t bytesToEnd = end - m_write;

      assert(BytesAvailable() > sizeof(uint32_t));

      if (bytesToEnd < sizeof(uint32_t))
      {
         assert(bytesToEnd == 0);
         m_write = (uintptr_t)m_buffer.data();
      }

      *((uint32_t*)m_write) = data;

      m_write     += sizeof(uint32_t);
      m_bytesUsed += sizeof(uint32_t);
   }

   void Push(uint64_t data)
   {
      Push((uint32_t)data);
      Push((uint32_t)(data >> 32));
   }

   uint32_t Pop(uint32_t bytes, void *buffer)
   {
      bytes = std::min((size_t)bytes, m_bytesUsed);

      uintptr_t end = (uintptr_t)m_buffer.data() + Size;

      uint32_t bytesToCopy = bytes;
      uint32_t bytesToEnd  = end - m_read;

      if (bytesToEnd > bytesToCopy)
      {
         // No wrapping
         memcpy(buffer, (void*)m_read, bytesToCopy);
      }
      else
      {
         // Buffer wraps
         memcpy(buffer, (void*)m_read, bytesToEnd);
         m_read = (uintptr_t)m_buffer.data();
         m_bytesUsed -= bytesToEnd;
         buffer = (void*)((uintptr_t)buffer + bytesToEnd);
         bytesToCopy -= bytesToEnd;

         memcpy(buffer, (void*)m_read, bytesToCopy);
      }

      m_read += bytesToCopy;
      m_bytesUsed -= bytesToCopy;

      return bytes;
   }

   uint32_t BytesReady() const { return m_bytesUsed; }

   uint32_t BytesAvailable() const { return Size - m_bytesUsed; }

private:
   std::array<uint8_t, Size>  m_buffer;
   uintptr_t                  m_write = 0;
   uintptr_t                  m_read = 0;
   size_t                     m_bytesUsed = 0;
};

class PerfFieldDesc
{
public:
   PerfFieldDesc(const std::string &name, bool isSigned, uint32_t numBytes) :
      m_name(name), m_signed(isSigned), m_numBytes(numBytes)
   {
      assert(numBytes == 4 || numBytes == 8);

      if (m_signed)
      {
         if (m_numBytes == 4)
            m_type = BCM_EVENT_INT32;
         else
            m_type = BCM_EVENT_INT64;
      }
      else
      {
         if (m_numBytes == 4)
            m_type = BCM_EVENT_UINT32;
         else
            m_type = BCM_EVENT_UINT64;
      }
   }

   std::string          m_name;
   bool                 m_signed;
   uint32_t             m_numBytes;
   bcm_sched_field_type m_type;
};

struct PerfEventDataHeader
{
   uint64_t timestamp;
   uint32_t track;
   uint32_t id;
   uint32_t event;
   uint32_t type;
};

class PerfEventDesc
{
public:
   PerfEventDesc() : m_size(sizeof(PerfEventDataHeader)) {}
   PerfEventDesc(const std::string &name) : m_name(name), m_size(sizeof(PerfEventDataHeader)) {}
   void AddField(const std::string &name, bool isSigned, uint32_t numBytes)
   {
      m_fields.emplace_back(name, isSigned, numBytes);
      m_size += numBytes;
   }

   uint32_t DataSize() const { return m_size; }

   std::string                m_name;
   std::vector<PerfFieldDesc> m_fields;
   uint32_t                   m_size;
};

class PerfData
{
public:
   PerfData() : m_active(false) {}

   void SetNexusCounts(uint32_t nexusTracks, uint32_t nexusEvents)
   {
      m_nexusTracks = nexusTracks;
      m_nexusEvents = nexusEvents;
   }
   void SetTimebaseOffset(int64_t offset) { m_timeBaseOffset = offset; }

   void AddTrackDesc(uint32_t idx, const std::string &name);
   void AddEventDesc(uint32_t idx, const PerfEventDesc &evDesc);

   const std::string   &TrackName(uint32_t trackIdx) const { return m_trackNames.at(trackIdx); }
   const PerfEventDesc &EventDesc(uint32_t eventIdx) const { return m_eventDescs.at(eventIdx); }

   uint32_t NumTracks() const { return m_trackNames.size(); }
   uint32_t NumEvents() const { return m_eventDescs.size(); }

   void AddData(uint32_t track, uint32_t event, uint32_t id, bcm_sched_event_type type,
                uint64_t timestamp, va_list args);

   uint32_t GetData(uint32_t bytes, void *buffer, bool *lostData);

   uint64_t TimeNow() const { return vcos_steady_clock_now_us() + m_timeBaseOffset; }

   bool IsActive()          { return m_active.load(); }
   void SetActive(bool val) { m_active.store(val);  }

private:
   std::atomic_bool            m_active;

   std::vector<std::string>    m_trackNames;
   std::vector<PerfEventDesc>  m_eventDescs;

   RingBuffer<2 * 1024 * 1024> m_dataBlock;
   bool                        m_lostData = false;

   std::mutex                  m_mutex;

   uint32_t                    m_nexusTracks = 0;
   uint32_t                    m_nexusEvents = 0;
   int64_t                     m_timeBaseOffset = 0;
};

void PerfData::AddTrackDesc(uint32_t idx, const std::string &name)
{
   if (m_trackNames.size() <= idx)
      m_trackNames.resize(idx + 1);
   m_trackNames.at(idx) = name;
}

void PerfData::AddEventDesc(uint32_t idx, const PerfEventDesc &evDesc)
{
   if (m_eventDescs.size() <= idx)
      m_eventDescs.resize(idx + 1);
   m_eventDescs.at(idx) = evDesc;
}

void PerfData::AddData(uint32_t track, uint32_t event, uint32_t id, bcm_sched_event_type type,
                       uint64_t timestamp, va_list args)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   assert(m_active);

   const PerfEventDesc &ed = EventDesc(event);

   // Will we fit? (event data without optional fields is 24 bytes)
   if (m_dataBlock.BytesAvailable() < ed.DataSize() + 24)
   {
      m_lostData = true;
      return;
   }

   m_dataBlock.Push(timestamp + m_timeBaseOffset);
   m_dataBlock.Push(track + m_nexusTracks);
   m_dataBlock.Push(id);
   m_dataBlock.Push(event + m_nexusEvents);
   m_dataBlock.Push((uint32_t)type);

   // Now deal with any extra fields
   for (uint32_t f = 0; f < ed.m_fields.size(); f++)
   {
      const PerfFieldDesc &fd = ed.m_fields.at(f);

      switch (fd.m_type)
      {
      case BCM_EVENT_INT32:  m_dataBlock.Push((uint32_t)va_arg(args, int32_t));  break;
      case BCM_EVENT_UINT32: m_dataBlock.Push((uint32_t)va_arg(args, uint32_t)); break;
      case BCM_EVENT_INT64:  m_dataBlock.Push((uint64_t)va_arg(args, int64_t));  break;
      case BCM_EVENT_UINT64: m_dataBlock.Push((uint64_t)va_arg(args, uint64_t)); break;
      }
   }
}

uint32_t PerfData::GetData(uint32_t bytes, void *buffer, bool *lostData)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   *lostData = m_lostData;

   if (bytes == 0 || buffer == NULL)
      return m_dataBlock.BytesReady();

   m_lostData = false;
   return m_dataBlock.Pop(bytes, buffer);
}

void PerfInitialize(EventContext *ctx)
{
   PerfData *pd = new PerfData;

   pd->AddTrackDesc(PERF_EVENT_TRACK_QUEUE, "Display queue");
   pd->AddTrackDesc(PERF_EVENT_TRACK_DISPLAY, "On display");

   PerfEventDesc queue("Queue");
   queue.AddField("Surface",  false, sizeof(uintptr_t));
   queue.AddField("Fence",    true, sizeof(int32_t));
   queue.AddField("Interval", true, sizeof(int32_t));
   pd->AddEventDesc(PERF_EVENT_QUEUE, queue);

   PerfEventDesc dequeue("Dequeue");
   dequeue.AddField("Surface", false, sizeof(uintptr_t));
   dequeue.AddField("Fence", true, sizeof(int32_t));
   pd->AddEventDesc(PERF_EVENT_DEQUEUE, dequeue);

   PerfEventDesc onDisplay("On Display");
   onDisplay.AddField("Surface", false, sizeof(uintptr_t));
   pd->AddEventDesc(PERF_EVENT_ON_DISPLAY, onDisplay);

   ctx->internals = (void*)pd;
}

void PerfTerminate(EventContext *ctx)
{
   delete (PerfData*)ctx->internals;
   ctx->internals = NULL;
}

// Event timeline functions. Used to hook extra display event tracks into the scheduler timeline.
void PerfAdjustEventCounts(EventContext *ctx, uint32_t *numTracks, uint32_t *numEvents)
{
   PerfData *pd = (PerfData*)ctx->internals;

   pd->SetNexusCounts(*numTracks, *numEvents);

   *numTracks += pd->NumTracks();
   *numEvents += pd->NumEvents();
}

BEGL_SchedStatus PerfGetEventTrackInfo(EventContext *ctx, uint32_t track,
                                       struct bcm_sched_event_track_desc *track_desc)
{
   PerfData *pd = (PerfData*)ctx->internals;

   track -= ctx->nexusTracks;

   if (track < pd->NumTracks())
   {
      strncpy(track_desc->name, pd->TrackName(track).c_str(), V3D_MAX_EVENT_STRING_LEN);
      return BEGL_SchedSuccess;
   }
   return BEGL_SchedFail;
}

BEGL_SchedStatus PerfGetEventInfo(EventContext *ctx, uint32_t event, struct bcm_sched_event_desc *event_desc)
{
   PerfData *pd = (PerfData*)ctx->internals;

   event -= ctx->nexusEvents;

   if (event < pd->NumEvents())
   {
      const PerfEventDesc &ed = pd->EventDesc(event);

      strncpy(event_desc->name, ed.m_name.c_str(), V3D_MAX_EVENT_STRING_LEN);
      event_desc->num_data_fields = ed.m_fields.size();
      return BEGL_SchedSuccess;
   }
   return BEGL_SchedFail;
}

BEGL_SchedStatus PerfGetEventDataFieldInfo(EventContext *ctx, uint32_t event, uint32_t field,
                                           struct bcm_sched_event_field_desc *field_desc)
{
   PerfData *pd = (PerfData*)ctx->internals;

   event -= ctx->nexusEvents;

   if (event < pd->NumEvents())
   {
      const PerfFieldDesc &fd = pd->EventDesc(event).m_fields.at(field);

      strncpy(field_desc->name, fd.m_name.c_str(), V3D_MAX_EVENT_STRING_LEN);
      field_desc->data_type = fd.m_type;
      return BEGL_SchedSuccess;
   }

   return BEGL_SchedSuccess;
}

BEGL_SchedStatus PerfSetEventCollection(EventContext *ctx, BEGL_SchedEventState state)
{
   PerfData *pd = (PerfData*)ctx->internals;

   if (state == BEGL_EventStart)
   {
      // Calculate difference in Nexus timestamp (in microseconds) to ours (also in microseconds)
      pd->SetTimebaseOffset(ctx->timeSync - vcos_steady_clock_now_us());
      pd->SetActive(true);
   }
   else if (state == BEGL_EventStop)
   {
      pd->SetActive(false);
   }

   return BEGL_SchedSuccess;
}

uint32_t PerfGetEventData(EventContext *ctx, uint32_t event_buffer_bytes, void *event_buffer,
                          uint32_t *lost_data, uint64_t *timestamp)
{
   PerfData *pd = (PerfData*)ctx->internals;

   *timestamp = pd->TimeNow();

   bool     lost = false;
   uint32_t bytes = pd->GetData(event_buffer_bytes, event_buffer, &lost);
   *lost_data = lost ? 1 : 0;

   return bytes;
}

// Note: variadic function for field values
void PerfAddEvent(EventContext *ctx, uint32_t track, uint32_t event, uint32_t id,
                  bcm_sched_event_type type, ...)
{
   PerfData *pd = (PerfData*)ctx->internals;

   if (pd->IsActive())
   {
      va_list args;
      va_start(args, type);

      pd->AddData(track, event, id, type, vcos_steady_clock_now_us(), args);

      va_end(args);
   }
}

// Note: variadic function for field values
void PerfAddEventWithTime(EventContext *ctx, uint32_t track, uint32_t event, uint32_t id,
                          bcm_sched_event_type type, uint64_t timestamp, ...)
{
   PerfData *pd = (PerfData*)ctx->internals;

   if (pd->IsActive())
   {
      va_list args;
      va_start(args, timestamp);

      pd->AddData(track, event, id, type, timestamp, args);

      va_end(args);
   }
}

uint64_t PerfGetTimeNow()
{
   return vcos_steady_clock_now_us();
}
