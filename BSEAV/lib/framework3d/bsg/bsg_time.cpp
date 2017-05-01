/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_common.h"
#include "bsg_time.h"

#include <ctime>
using namespace std::chrono;

namespace bsg
{

system_clock::time_point Time::m_appEpoch(system_clock::now());
high_resolution_clock::time_point HighResTime::m_appEpoch(high_resolution_clock::now());

Time Time::Now()
{
   auto elapsed = system_clock::now() - m_appEpoch;
   return Time(duration_cast<milliseconds>(elapsed).count());
}

int64_t Time::CalToMs(time_t cal)
{
   int64_t appSecs =
      cal - duration_cast<seconds>(m_appEpoch.time_since_epoch()).count();
   return appSecs * 1000;
}

time_t Time::MsToCal(int64_t ms)
{
   time_t calSecs = (ms / 1000) +
      duration_cast<seconds>(m_appEpoch.time_since_epoch()).count();
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

   m_ms = CalToMs(std::mktime(&t));
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

HighResTime HighResTime::Now()
{
   auto elapsed = high_resolution_clock::now() - m_appEpoch;
   return HighResTime(duration_cast<milliseconds>(elapsed).count());
}

}