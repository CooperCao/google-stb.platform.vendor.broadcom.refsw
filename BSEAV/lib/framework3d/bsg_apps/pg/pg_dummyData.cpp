/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.  
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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

