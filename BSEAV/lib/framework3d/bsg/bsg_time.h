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

#ifndef __BSG_TIME_H__
#define __BSG_TIME_H__

#include "bsg_common.h"

#include <stdint.h>
#include <time.h>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <stdio.h>

namespace bsg
{

const int64_t TIME_ARITH_FACTOR  = 0x1000000;
const float   TIME_ARITH_FLOAT   = (float)TIME_ARITH_FACTOR;
const double  TIME_ARITH_DOUBLE  = (double)TIME_ARITH_FACTOR;
const float   TIME_ARITH_RECIP   = (1.0f / TIME_ARITH_FLOAT);

//! The Time class encapsulates an absolute time value.
//! Facilities are provided for arithmetic on time values, and getting the current time.
class Time
{
public:
   enum eTimeUnit
   {
      eMILLISECONDS  = 1,
      eSECONDS       = 1000,
      eMINUTES       = 60 * 1000,
      eHOURS         = 60 * 60 * 1000,
      eDAYS          = 24 * 60 * 60 * 1000
   };

private:
   static int64_t ToMS(int32_t val,  eTimeUnit u)   { return (int64_t)val * (int64_t)u; }
   static int64_t ToMS(uint32_t val, eTimeUnit u)   { return (int64_t)val * (int64_t)u; }
   static int64_t ToMS(int64_t val,  eTimeUnit u)   { return (int64_t)val * (int64_t)u; }
   static int64_t ToMS(float val,    eTimeUnit u)   { return (int64_t)(val * (float)u); }
   static int64_t ToMS(double val,   eTimeUnit u)   { return (int64_t)(val * (double)u); }

public:
   Time() : m_ms(0) {}

   Time(const Time &rhs) : m_ms(rhs.m_ms) {}

   Time(int32_t val,    eTimeUnit u = eMILLISECONDS) : m_ms(ToMS(val, u)) {}
   Time(uint32_t val,   eTimeUnit u = eMILLISECONDS) : m_ms(ToMS(val, u)) {}
   Time(int64_t val,    eTimeUnit u = eMILLISECONDS) : m_ms(ToMS(val, u)) {}
   Time(float val,      eTimeUnit u = eSECONDS)      : m_ms(ToMS(val, u)) {}
   Time(double val,     eTimeUnit u = eSECONDS)      : m_ms(ToMS(val, u)) {}
   Time(uint32_t secs,  uint32_t  usecs)             : m_ms(ToMS(secs, eSECONDS) + ToMS(usecs / 1000, eMILLISECONDS)) {}

   Time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

   //! Set time from a calendar time
   void FromCalendarTime(time_t calTime);

   //! Return the time value as milliseconds (since the program was started for absolute time values)
   int64_t Milliseconds() const { return m_ms; }

   //! Return the time value as seconds (since the program was started for absolute time values)
   double Seconds() const { int64_t whole = m_ms / 1000; return (double)whole + (double)(m_ms - 1000 * whole) * 0.001f; }

   //! Return the time value as minutes (since the program was started for absolute time values)
   double Minutes() const { int64_t whole = m_ms / 60000; return (double)whole + (double)(m_ms - 60000 * whole) * 0.000016666666666f; }

   //! Return the time value as hours (since the program was started for absolute time values)
   double Hours() const { int64_t whole = m_ms / 3600000; return (double)whole + (double)(m_ms - 3600000 * whole) * 0.000000277777777f; }

   //! Return the time value as seconds (since the program was started for absolute time values)
   //! You should only use this for times you know will have smallish values, like deltas.
   float FloatSeconds() const
   {
      int64_t wholeSecs = m_ms / 1000;
      return (float)wholeSecs + (float)(m_ms - wholeSecs * 1000) * 0.001f;
   }

   //! Return the time value as minutes (since the program was started for absolute time values)
   //! You should only use this for times you know will have smallish values, like deltas.
   float FloatMinutes() const { int64_t whole = m_ms / 60000; return (float)whole + (float)(m_ms - 60000 * whole) * 0.000016666666666f; }

   //! Return the time value as hours (since the program was started for absolute time values)
   //! You should only use this for times you know will have smallish values, like deltas.
   float FloatHours() const { int64_t whole = m_ms / 3600000; return (float)whole + (float)(m_ms - 3600000 * whole) * 0.000000277777777f; }

   //! Return the time value as a time_t calendar time (only really relevant for absolute times, not relative ones)
   time_t  CalendarTime() const { return MsToCal(m_ms); }

   //! Return the calendar year
   uint16_t CalendarYear() const;

   //! Return the calendar month
   uint8_t CalendarMonth() const;

   //! Return the calendar day (of the month)
   uint8_t CalendarDay() const;

   //! Return the calendar hour
   uint8_t CalendarHour() const;

   //! Return the calendar minute
   uint8_t CalendarMinute() const;

   //! Return the calendar second
   uint8_t CalendarSecond() const;

   //! Return the calendar time as a formatted string. See help for strftime() for formatting options.
   std::string CalendarTimeString(const std::string &format = std::string("%d %B %Y - %X")) const;

   //! Multiply the time by a scalar
   Time operator*(float scale) const
   {
      return Time((int64_t)(m_ms * (double)scale));
   }

   //! Multiply the time by a scalar
   Time operator*(double scale) const
   {
      return Time((int64_t)(m_ms * scale));
   }

   //! Add two times
   Time operator+(const Time &rhs) const { return Time(m_ms + rhs.m_ms); }

   //! Subtract the rhs from this time
   Time operator-(const Time &rhs) const { return Time(m_ms - rhs.m_ms); }

   //! Compare two times for equality
   bool operator==(const Time &rhs) const { return m_ms == rhs.m_ms; }

   //! Compare two times for inequality
   bool operator!=(const Time &rhs) const { return m_ms != rhs.m_ms; }

   //! Is this < rhs
   bool operator<(const Time &rhs) const { return m_ms < rhs.m_ms; }

   //! Is this <= rhs
   bool operator<=(const Time &rhs) const { return m_ms <= rhs.m_ms; }

   //! Is this > rhs
   bool operator>(const Time &rhs) const { return m_ms > rhs.m_ms; }

   //! Is this >= rhs
   bool operator>=(const Time &rhs) const { return m_ms >= rhs.m_ms; }

   //! Return the current time
   static Time Now();

private:
   static int64_t CalToMs(time_t cal);
   static time_t  MsToCal(int64_t ms);

private:
   static  int64_t   m_appEpoch;
   static  time_t    m_appCalEpoch;

   int64_t           m_ms;
};

inline std::istream &operator>>(std::istream &is, Time &t)
{
   float secs;
   is >> secs;
   t = Time(secs);

   return is;
}

class HighResTime
{
public:
   HighResTime() : m_us(0) {};

   HighResTime(const HighResTime &rhs) : m_us(rhs.m_us) {}

   HighResTime(int64_t val) : m_us(val) {}

   //! Add two times
   HighResTime operator+(const HighResTime &rhs) const { return HighResTime(m_us + rhs.m_us); }

   //! Subtract the rhs from this time
   HighResTime operator-(const HighResTime &rhs) const { return HighResTime(m_us - rhs.m_us); }

   //! Compare two times for equality
   bool operator==(const HighResTime &rhs) const { return m_us == rhs.m_us; }

   //! Compare two times for inequality
   bool operator!=(const HighResTime &rhs) const { return m_us != rhs.m_us; }

   //! Is this < rhs
   bool operator<(const HighResTime &rhs) const { return m_us < rhs.m_us; }

   //! Is this <= rhs
   bool operator<=(const HighResTime &rhs) const { return m_us <= rhs.m_us; }

   //! Is this > rhs
   bool operator>(const HighResTime &rhs) const { return m_us > rhs.m_us; }

   //! Is this >= rhs
   bool operator>=(const HighResTime &rhs) const { return m_us >= rhs.m_us; }

   //! Return the current time
   static HighResTime Now();

   int64_t Microseconds() const { return m_us; };
private:
   static  int64_t   m_appEpoch;
   int64_t           m_us;
};

}

#endif /* __BSG_TIME_H__ */

