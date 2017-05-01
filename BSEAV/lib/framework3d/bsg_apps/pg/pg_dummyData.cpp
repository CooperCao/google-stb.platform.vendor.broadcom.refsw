/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "pg_dummyData.h"
#include "bsg_time.h"

#include <stdio.h>

#include <map>

namespace pg
{

   typedef struct
   {
      unsigned channelNumber;
      ChannelInfo::ServiceType type;
      const char *name;
      unsigned onid;
      unsigned tsid;
      unsigned sid;
      unsigned id;
      const char *providerName;
      bool isEncrypted;
      bool isHdtv;
      bool isVisible;
      bool isNumericSelectable;
   } DummyChannelData;

struct cridentry {
    const char *Data;
    unsigned Type;
};

typedef struct {
    unsigned ServiceId;
    int Content;
    unsigned CridCount;
    struct cridentry Crid[10];
    unsigned CridImi;
    bool DataFromCurrentTs;
    const char *Description;
    unsigned DvbOid;
    unsigned DvbSid;
    unsigned DvbTid;
    time_t End;
    const char *Guidance;
    bool IsHd;
    unsigned LinkedHdProgs;
    unsigned LinkedHdSimulcastProgs;
    bool Next;
    bool Now;
    unsigned PreferredServiceNameId;
    unsigned ProgId;
    unsigned ScheduleVersion;
    unsigned ServicePosition;
    time_t Start;
    const char *Title;
    bool UnsuitableBeforeWatershed;
} DummyProgData;

#include "pg_dummyData.data"

DummyData::DummyData()
{
}

void DummyData::InitGridInfo(GridInfo *info)
{
   std::map<uint32_t, uint32_t>  sidMap;

   bsg::Time minTime(2011, 10, 13, 06, 00, 00);

   // Quick hack to shift the database to now(ish) - round to nearest hour
   bsg::Time now = bsg::Time::Now();
   now = bsg::Time(now.CalendarYear(), now.CalendarMonth(), now.CalendarDay(), 6, 0, 0);
   bsg::Time offset = now - minTime;

   info->Clear();

   uint32_t numChannels = sizeof(serviceData) / sizeof(DummyChannelData);
   for (uint32_t c = 0; c < numChannels; c++)
   {
      ChannelInfo chan(serviceData[c].channelNumber, serviceData[c].name, serviceData[c].type);
      info->AddChannel(chan);
      sidMap.insert(std::pair<uint32_t, uint32_t>(serviceData[c].sid, c));
   }

   uint32_t numProgs = sizeof(progData) / sizeof(DummyProgData);
   for (uint32_t p = 0; p < numProgs; p++)
   {
      std::map<uint32_t, uint32_t>::iterator iter = sidMap.find(progData[p].DvbSid);
      if (iter != sidMap.end())
      {
         ChannelInfo &chInfo(info->GetChannel(sidMap[progData[p].DvbSid]));

         bsg::Time start, end;

         start.FromCalendarTime(progData[p].Start);
         end.FromCalendarTime(progData[p].End);

         // Hack the times
         start = start + offset;
         end = end + offset;

         if (progData[p].Title != NULL && progData[p].Description != NULL)
         {
            ProgramInfo prog(start, end, progData[p].Title, progData[p].Description, progData[p].IsHd);

            //printf("%s : %s -> %s : %s\n", chInfo.GetName().c_str(), start.CalendarTimeString().c_str(), end.CalendarTimeString().c_str(), progData[p].Title);

            chInfo.AddProgram(prog);
         }
      }
   }
}

}
