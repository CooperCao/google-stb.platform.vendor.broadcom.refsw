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
#include "bsg_common.h"
#include "bsg_time.h"

namespace bsg
{

int64_t Time::CalToMs(time_t cal)
{
   int64_t appSecs = cal - m_appCalEpoch;
   return appSecs * 1000;
}

time_t Time::MsToCal(int64_t ms)
{
   time_t calSecs = (ms / 1000) + m_appCalEpoch;
   return calSecs;
}

Time::Time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
   struct tm t = { 0 };

   t.tm_year = year - 1900;
   t.tm_mon = month - 1;
   t.tm_mday = day;
   t.tm_hour = hour;
   t.tm_min = minute;
   t.tm_sec = second;

   m_ms = CalToMs(mktime(&t));
}

void Time::FromCalendarTime(time_t calTime)
{
   m_ms = CalToMs(calTime);
}

std::string Time::CalendarTimeString(const std::string &format) const
{
   char        buffer[1024];
   time_t      time = MsToCal(m_ms);
   struct tm   *timeinfo;

   buffer[0] = '\0';
   timeinfo = localtime(&time);

   if (timeinfo)
      strftime(buffer, 1024, format.c_str(), timeinfo);

   return std::string(buffer);
}

uint16_t Time::CalendarYear() const
{
   time_t      time = MsToCal(m_ms);
   struct tm   *timeinfo;

   timeinfo = localtime(&time);

   if (timeinfo)
      return timeinfo->tm_year + 1900;

   return 0;
}

uint8_t Time::CalendarMonth() const
{
   time_t      time = MsToCal(m_ms);
   struct tm   *timeinfo;

   timeinfo = localtime(&time);

   if (timeinfo)
      return timeinfo->tm_mon + 1;

   return 0;
}

uint8_t Time::CalendarDay() const
{
   time_t      time = MsToCal(m_ms);
   struct tm   *timeinfo;

   timeinfo = localtime(&time);

   if (timeinfo)
      return timeinfo->tm_mday;

   return 0;
}

uint8_t Time::CalendarHour() const
{
   time_t      time = MsToCal(m_ms);
   struct tm   *timeinfo;

   timeinfo = localtime(&time);

   if (timeinfo)
      return timeinfo->tm_hour;

   return 99;
}

uint8_t Time::CalendarMinute() const
{
   time_t      time = MsToCal(m_ms);
   struct tm   *timeinfo;

   timeinfo = localtime(&time);

   if (timeinfo)
      return timeinfo->tm_min;

   return 99;
}

uint8_t Time::CalendarSecond() const
{
   time_t      time = MsToCal(m_ms);
   struct tm   *timeinfo;

   timeinfo = localtime(&time);

   if (timeinfo)
      return timeinfo->tm_sec;

   return 99;
}

}

