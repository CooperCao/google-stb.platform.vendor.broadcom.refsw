/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdint.h>

#if WANT_PROFILING

#if !defined(__arm__)
#undef USE_ARM_CCNT
#define USE_ARM_CCNT 0
#endif

#define PROFILE_TIMER_ profile_timer_
#define PROFILE_SCOPE_ profile_scope_

typedef struct profile_timer
{
   volatile uint32_t ticks;
   volatile uint32_t calls;
} profile_timer;

#ifdef __cplusplus
extern "C" {
#endif

profile_timer* profile_timer_new(char const* group, char const* file, char const* function, char const* name);
void profile_timer_delete(profile_timer* timer);

static inline void profile_timer_add(profile_timer* timer, uint32_t ticks) { timer->ticks += ticks; timer->calls += 1; }
static inline void profile_timer_add_mt(profile_timer* timer, uint32_t ticks) { timer->ticks += ticks; timer->calls += 1; }

void profile_init(void);
void profile_shutdown(void);
void profile_on_finish(void);
void profile_on_swap(void);
void profile_poll(void);

#if USE_ARM_CCNT
static inline uint32_t profile_get_tick_count(void)
{
   // Read CCNT Register
   uint32_t value;
   asm ("mrc p15, 0, %0, c9, c13, 0": "=r"(value));
   return value;
}
#else
uint32_t profile_get_tick_count(void);
#endif

#ifdef __cplusplus
}
#endif

#else

static inline void profile_init(void) {}
static inline void profile_shutdown(void) {}
static inline void profile_on_finish(void) {}
static inline void profile_on_swap(void) {}
static inline void profile_poll(void) {}

#endif


#if defined(__cplusplus) && WANT_PROFILING

class profile_scope
{
public:
   profile_scope(profile_timer* timer) : m_timer(timer), m_start(profile_get_tick_count()) {}
   ~profile_scope() { profile_timer_add(m_timer, profile_get_tick_count() - m_start); }

   profile_scope(profile_scope const&) = delete;
   profile_scope& operator=(profile_scope const&) = delete;

private:
   profile_timer* m_timer;
   uint32_t m_start;
};

#define PROFILE_SCOPE(group, name)\
   static profile_timer* PROFILE_TIMER_ = profile_timer_new(group, __FILE__, __FUNCTION__, name);\
   profile_scope const PROFILE_SCOPE_(PROFILE_TIMER_);

#elif defined(__GNUC__) && WANT_PROFILING

typedef struct profile_scope
{
   profile_timer* timer;
   uint32_t start;
} profile_scope;

static inline void profile_scope_end(profile_scope const* scope)
{
   profile_timer_add(scope->timer, profile_get_tick_count() - scope->start);
}

#define PROFILE_SCOPE(group, name)\
   static profile_timer* PROFILE_TIMER_ = NULL;\
   if (!PROFILE_TIMER_) { PROFILE_TIMER_= profile_timer_new(group, __FILE__, __FUNCTION__, name); }\
   profile_scope const PROFILE_SCOPE_ __attribute__((__cleanup__(profile_scope_end))) = { PROFILE_TIMER_, profile_get_tick_count() };

#else

#define PROFILE_SCOPE(group, name)

#endif

#define PROFILE_FUNCTION(group) PROFILE_SCOPE(group, "")

// multi-threaded placeholders
#define PROFILE_FUNCTION_MT(group) PROFILE_FUNCTION(group)
#define PROFILE_SCOPE_MT(group, name) PROFILE_SCOPE(group, name)
