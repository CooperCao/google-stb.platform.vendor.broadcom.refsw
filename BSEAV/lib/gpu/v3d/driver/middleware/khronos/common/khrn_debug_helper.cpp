/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#if HAS_DEBUG_HELPER

#ifdef inline
#undef inline
#endif

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <list>
#include <memory>
#include <atomic>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <ctime>

// don't pull in vcos.h into c++ file
typedef enum
{
   LOG_ERROR,
   LOG_WARN,
   LOG_INFO,
   LOG_TRACE
} log_level_t;

extern "C" void vcos_log_impl(log_level_t log_level, const char *fmt, ...);
#ifndef vcos_log
#define vcos_log(log_level, ...) vcos_log_impl(log_level, __VA_ARGS__)
#endif

#ifdef WIN32
static constexpr int TIMEOUT = 2000;      // in seconds
#else
static constexpr int TIMEOUT = 20;        // in seconds
#endif
static constexpr int MAX_RECORDING = 1000;   // number of lines to keep (currently ~20 lines per frame, so ~10frames)
static constexpr int MAX_STRLEN = 256;       // max line length including time

class dbgMessageHelper
{
public:
   dbgMessageHelper()
   {
      m_start_time = std::chrono::steady_clock::now();
   }

   ~dbgMessageHelper()
   {
      std::unique_lock<std::mutex> m(m_idle);

      if (m_init)
      {
         m_trigger = true;
         m_closedown = true;
         m.unlock();
         m_cv.notify_one();

         m_dbg_message_debug_thread.join();
      }
   }

   void add(const char *fmt, va_list args)
   {
      std::unique_lock<std::mutex> m(m_idle);

      if (!m_init)
      {
         // thread creation hangs on win32 if in constructor.  Defer to here
         std::thread t([this]() {
            std::unique_lock<std::mutex> m(m_idle);
            while (!m_closedown)
            {
               if (m_cv.wait_for(m, std::chrono::seconds(TIMEOUT), [this] { return m_trigger; }))
                  m_trigger = false;
               else
               {
                  if (m_list.size() > 0)
                  {
                     m_timedout = true; // print any API activity after timeout
                     vcos_log(LOG_ERROR, "no activity in %d seconds - %d entries", TIMEOUT, m_list.size());
                     int i = 0;
                     for (std::list<std::unique_ptr<std::string>>::reverse_iterator log = m_list.rbegin(); log != m_list.rend(); ++log, i++)
                        vcos_log(LOG_ERROR, "[%04d] %s", i, (*log)->c_str());
                  }
               }
            }
         });
         m_dbg_message_debug_thread = std::move(t);
         m_init = true;
      }

      char buffer[MAX_STRLEN];
      vsnprintf(buffer, MAX_STRLEN, fmt, args);

      auto now = std::chrono::steady_clock::now();
      auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time);
      auto n_ms = int_ms.count();
      constexpr int second = 1000;
      constexpr int minute = 60 * second;
      constexpr int hour = 60 * minute;

      int hours = static_cast<int>(n_ms / hour);
      n_ms -= (hours * hour);
      int minutes = static_cast<int>(n_ms / minute);
      n_ms -= (minutes * minute);
      int seconds = static_cast<int>(n_ms / second);
      n_ms -= (seconds * second);

      char buffer2[MAX_STRLEN];
      snprintf(buffer2, MAX_STRLEN, "[%04d:%02d:%02d:%03d] %s", hours, minutes, seconds, static_cast<int>(n_ms), buffer);

      if (m_timedout)
         vcos_log(LOG_ERROR, "TIMEOUT -> %s", buffer2);

      std::unique_ptr<std::string> log = std::unique_ptr<std::string>(new std::string(buffer2));
      m_list.push_front(std::move(log));

      if (m_list.size() > MAX_RECORDING)
         m_list.pop_back();

      m_trigger = true;
      m.unlock();
      m_cv.notify_one();
   }

private:
   std::thread             m_dbg_message_debug_thread;
   std::mutex              m_idle;
   std::condition_variable m_cv;
   std::list<std::unique_ptr<std::string>> m_list;
   std::chrono::steady_clock::time_point m_start_time;

   bool                    m_trigger = false;
   bool                    m_timedout = false;
   bool                    m_closedown = false;
   bool                    m_init = false;
};

static dbgMessageHelper s_dbghelper;

extern "C" void platform_dbg_message_add(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   s_dbghelper.add(fmt, args);
   va_end(args);
}
#endif // HAS_DEBUG_HELPER
