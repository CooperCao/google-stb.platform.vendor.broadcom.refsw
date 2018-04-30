/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/khrn_counters.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "interface/vcos/vcos.h"

struct KHRN_FMEM;
struct GLXX_HW_RENDER_STATE;
struct EGL_IMAGE;

typedef enum
{
   KHRN_BIN_RENDER_DONE,
   KHRN_TFCONVERT_DONE,
   KHRN_VG_DONE,
   KHRN_SWAPBUFFERS_DONE,
   KHRN_FENCE_WAIT_DONE
} KHRN_CALLBACK_REASONS_T;

bool khrn_hw_common_init(void);
void khrn_hw_common_term(void);

extern void lockCallback(void);
extern void unlockCallback(void);

/* flush all queued stuff */
extern void khrn_hw_common_flush(void);

/* finish all queued stuff */
extern void khrn_hw_common_finish(void);

/* wait for all flushed stuff to finish */
extern void khrn_hw_common_wait(void);

/* wait for a specific job to complete before progressing */
extern void khrn_wait_for_job_done(uint64_t jobSequenceNumber);

extern void khrn_issue_finish_job(void);
extern void khrn_issue_bin_render_job(struct GLXX_HW_RENDER_STATE *rs, bool secure);
extern void khrn_issue_tfconvert_job(struct KHRN_FMEM *fmem, bool secure);
extern void khrn_issue_swapbuffers_job(int fd, uint64_t, char type);
extern void khrn_issue_fence_wait_job(uint64_t);
extern void khrn_create_fence(int *fd, uint64_t *p, char type);
extern uint64_t khrn_fence_wait_async(int fd);

extern uint64_t khrn_get_last_issued_seq(void);
extern uint64_t khrn_get_last_done_seq(void);

/* handle CPU --> GPU copy */
extern void khrn_handlecpy(MEM_HANDLE_T hDst, size_t dstOffset, const void *src, size_t size);

static inline void khrn_memcpy(void *dest, const void *src, uint32_t size)       { khrn_par_memcpy(dest, src, size); }
static inline void khrn_memset(void *dest, uint32_t val, uint32_t size)          { khrn_par_memset(dest, val, size); }
static inline int  khrn_memcmp(const void *ptr1, const void *ptr2, size_t size)  { return khrn_par_memcmp(ptr1, ptr2, size); }
