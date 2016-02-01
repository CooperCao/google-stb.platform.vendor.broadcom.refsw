/***************************************************************************
 *     (c)2012-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef RINGBUF_CAPTURE_H__
#define RINGBUF_CAPTURE_H__

#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <mntent.h>
#include <dirent.h>

#include "bmem.h"
#include "bchp_aud_fmm_bf_ctrl.h"


typedef enum BDSP_P_RbufType
{
    BDSP_P_RbufType_eDecode,
    BDSP_P_RbufType_ePassthru,
    BDSP_P_RbufType_eMixer,
    BDSP_P_RbufType_eEncoder,
    BDSP_P_RbufType_eTranscode,
    BDSP_P_RbufType_eSM
}BDSP_P_RbufType;

typedef struct BDSP_P_RbufCapture
{
    unsigned char (*rbuf_init)(void *hRegister, void *hHeap);
    unsigned char (*rbuf_uninit)(void);
    unsigned char (*rbuf_capture_channel_start)(uint32_t ui32Channel, uint32_t ui32Path, uint32_t ui32RbufId, BDSP_P_RbufType eRbufType);
    unsigned char (*rbuf_capture_stop_channel)(uint32_t ui32Channel, uint32_t ui32Path, uint32_t ui32RbufId, BDSP_P_RbufType eRbufType);

}BDSP_P_RbufCapture;

BDSP_P_RbufCapture Rbuf_Setting;

void BDSP_P_RbufSetup(
	BDSP_P_RbufCapture sRbufCap
);

void BDSP_P_RbufSetup(
	BDSP_P_RbufCapture sRbufCap
);

#ifdef __cplusplus
extern "C"
{
#endif

/*#include "brap.h"
#include "brap_priv.h"*/

typedef struct Rbuf_Callback
{
	/* return is size written */
    int (*cap_callback_function)(void *cb_context, void *addr, int size);
}Rbuf_Callback;


typedef struct Rbuf_Callback_Info
{
	Rbuf_Callback	* cb;
	void			* cb_context;
}Rbuf_Callback_Info;


typedef struct Rbuf_Channel_Info
{
	BDSP_P_RbufType rb_type;
	uint32_t	path;
	uint32_t	channel;
	uint32_t	rbuf_id;
}Rbuf_Channel_Info;

int register_ringbuf_capture_callback(Rbuf_Channel_Info * ch_info, Rbuf_Callback_Info * cb_info);

void brbuf_init_ringbuf_capture(void);

void brbuf_uninit_ringbuf_capture(void);

#ifdef __cplusplus
}
#endif

#define RBUF_PRINT(format) \
              do {printf("\n--RBUF-- "); printf format;} while (0)\

#define RBUF_DEBUG(format) \
            if (rbuf_capture_debug_enabled) { \
              printf("\n--RBUF-- "); \
			  printf format;         \
            } else {}

#define RBUF_ALERT(x, ...) \
  if (!(x)) {   \
    rbuf_capture_alert_fail(#x, __FILE__, __LINE__, ##__VA_ARGS__);\
    goto alert; \
  } else {}

#define RBUF_ASSERT(x, ...) \
  if (!(x)) { \
    rbuf_capture_assert_fail(#x, __FILE__, __LINE__, ##__VA_ARGS__);\
  } else {}

/* global data */

BDSP_P_RbufCapture local_ringBufCB = { NULL, NULL, NULL, NULL };

/* local data */

static uint32_t  rbuf_capture_from_rbuf_to_cap_q_active=0;
static uint32_t  rbuf_capture_from_cap_q_to_disk_active=0;

static pthread_t rbuf_capture_from_rbuf_to_cap_q_thread;
static pthread_t rbuf_capture_from_cap_q_to_disk_thread;

#define RBUF_CAP_MAX_CHAN               (16)
static unsigned int cap_channel_count = 0;

/*   Q Properties:*/
/*   ==================================*/
/**/
/*   * : used*/
/*   - | unused*/
/**/
/*   EMPTY: WRITE == READ*/
/*   -----------------------*/
/*   ^         ^           ^*/
/*   |         |           |*/
/*   START     READ        END*/
/*             |           (ENTRIES-1)*/
/*             WRITE*/
/**/
/*   FULL: WRITE == WRAP(READ-1)*/
/*   *********X**************/
/*   ^        ^^           ^*/
/*   |        ||           |*/
/*   START    |READ        END*/
/*            WRITE        (ENTRIES-1)*/
/**/
/*   QUEUE ENTRIES:   32*/
/*   ENTRY:           **/
/*   AVAILABLE:       -*/
/*   X:               UNUSABLE*/
/*   AVAIL:           14*/
/*   ENTRIES:         (7+10)=17*/
/**/
/*   **********--------------X********/
/*   00000000001111111111222222222233*/
/*   01234567890123456789012345678901*/
/*   ^         ^              ^     ^*/
/*   |         |              |     |*/
/*   START     WRITE          READ  END*/
/*                                  (ENTRIES-1)*/
/**/
/*   QUEUE ENTRIES:   32*/
/*   ENTRY:           **/
/*   AVAILABLE:       -*/
/*   X:               UNUSABLE*/
/*   AVAIL:           (14+9)=23*/
/*   ENTRIES:         8*/
/**/
/*   ---------X********--------------*/
/*   00000000001111111111222222222233*/
/*   01234567890123456789012345678901*/
/*   ^         ^       ^            ^*/
/*   |         |       |            |*/
/*   START     READ    WRITE        END*/
/*                                  (ENTRIES-1)*/

/*   QUEUE ENTRIES:   32*/
/*   ENTRY:           **/
/*   AVAILABLE:       -*/
/*   X:               UNUSABLE*/
/*   AVAIL:           1+30=31*/
/*   ENTRIES:         0*/
/**/
/*   ------------------------------X-*/
/*   00000000001111111111222222222233*/
/*   01234567890123456789012345678901*/
/*   ^                              ^*/
/*   |                              |*/
/*   START                          END*/
/*                                  (ENTRIES-1)*/
/*                                  WRITE*/
/*                                  READ*/

/*   QUEUE ENTRIES:   32*/
/*   ENTRY:           **/
/*   AVAILABLE:       -*/
/*   X:               UNUSABLE*/
/*   AVAIL:           31+0=31*/
/*   ENTRIES:         0*/
/**/
/*   -------------------------------X*/
/*   00000000001111111111222222222233*/
/*   01234567890123456789012345678901*/
/*   ^                              ^*/
/*   |                              |*/
/*   START                          END*/
/*   WRITE                         (ENTRIES-1)*/
/*   READ*/

#define Q_NEXT(cur, start, entries)     ((entries-1) == cur) ? start : (cur+1)
#define Q_PREV(cur, start, entries)     (start == cur) ? (entries-1) : (cur-1)
#define Q_IS_FULL(r, w, entries)        (r == (Q_NEXT(w, 0, (entries-1))))
#define Q_IS_EMPTY(r, w)                (r == w)

#define Q_AVAIL(r, w, entries)          ((w >= r) ? (((entries-1+1) - w) + (r-1))     : ((r - w) - 1))
#define Q_AVAIL_END(r, w, entries)      ((w >= r) ?  ((entries-1+1) - w) - (r?0:1)    : 0)
#define Q_AVAIL_START(r, w, entries)    ((w >= r) ?                        (r?r-1:0)  : ((r - w) - 1))

#define Q_ENTRIES(r, w, entries)        ((w >= r) ? (w - r) : (((entries-1+1) - r) + w)
#define Q_ENTRIES_END(r, w, entries)    ((w >= r) ? 0       :  ((entries-1+1) - r))
#define Q_ENTRIES_START(r, w, entries)  ((w >= r) ? (w - r) :                        w)

#define Q_INC(cur, inc, start, entries) (((entries-1) < (cur+inc)) ? (start + (inc - ((entries-1+1)-cur))) : (cur+inc))

/* Per ring buffer capture buffer number of bytes*/
/* Assuming Worst Case 192Khz @ 32bit PCM Samples*/
/* (2*1024*1024) / (192000 * (32/8)) = 2.73 seconds of buffering per buffer*/
#define RBUF_PAGE_SIZE					4096
#define	RBUF_PAGE_MULTIPLE				512
#define RBUF_CAP_MAX_CHAN_ENTRIES       (RBUF_PAGE_MULTIPLE * RBUF_PAGE_SIZE) /* 512 * 4096 = 2MB */

typedef enum RBUF_CAPTURE_CHANNEL_STATE {
  RBUF_CAPTURE_CHANNEL_STATE_INACTIVE,
  RBUF_CAPTURE_CHANNEL_STATE_ACTIVE,
  RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP,
  RBUF_CAPTURE_CHANNEL_STATE_STOPPED,
  RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE,
  RBUF_CAPTURE_CHANNEL_STATE_NUM_STATES
} RBUF_CAPTURE_CHANNEL_STATE;


BDSP_P_RbufType	CP_TYPE;

#define RBUF_PATH_INVALID     (0xFFFFFFFF)
#define RBUF_CHANNEL_INVALID  (0xFFFFFFFF)
#define RBUF_ID_INVALID       (0xFFFFFFFF)
#define RBUF_INDEX_INVALID    (-1)

typedef struct rbuf_capture_channel_t
{
	uint32_t		active;

	BDSP_P_RbufType type;
	uint32_t		rbuf_id;
	uint32_t		channel;
	uint32_t		path;
	uint32_t		index;

	Rbuf_Callback	* cb;
	void			* cb_context;

	RBUF_CAPTURE_CHANNEL_STATE rbuf_to_cap_q_state;
	RBUF_CAPTURE_CHANNEL_STATE cap_q_to_disk_state;

	uint32_t		rbuf_count;
	uint8_t			*rbuf_base_virt_uncached;
	uint8_t			*rbuf_base_virt_cached;
	uint32_t		rbuf_base_phys;
	uint32_t		rbuf_num_entries;
	uint32_t		rbuf_prev_wraddr;
	uint32_t		rbuf_prev_rdaddr;
	uint32_t		rbuf_prev_endaddr;
	uint32_t		rbuf_initialized;
	uint32_t		rbuf_started;
	uint32_t		rbuf_reg_base;

	uint32_t		q_count;
	uint8_t			*q_ptr;
	uint32_t		q_r_idx;
	uint32_t		q_w_idx;
	uint32_t		q_num_entries;

	int8_t			file_name[512];
	int32_t			file;

	bool			rbuf_flush_onexit;
} rbuf_capture_channel_t;

static rbuf_capture_channel_t *rbuf_capture_channels;

static bool rbuf_capture_enabled;
static bool rbuf_capture_debug_enabled;
static bool rbuf_capture_multifile;
static bool rbuf_capture_app_player;

static uint32_t rbuf_capture_initialized;
static void *rbuf_reg_handle;
static void *rbuf_heap;

#define RBUF_CAPTURE_PATH_DEFAULT       "/mnt/hd"
static int8_t  rbuf_capture_path[512];
static int8_t  rbuf_copy_path[512];
#define RBUF_CAPTURE_FILENAME_DEFAULT   "rbuf_cap"
static int8_t  rbuf_capture_filename[512];

typedef enum RBUF_CAPTURE_COMMAND {
  RBUF_CAPTURE_COMMAND_INIT,
  RBUF_CAPTURE_COMMAND_CAPTURE_PATH,
  RBUF_CAPTURE_COMMAND_CAPTURE_FILENAME,
  RBUF_CAPTURE_COMMAND_COPY_PATH,
  RBUF_CAPTURE_COMMAND_ECHO,
  RBUF_CAPTURE_COMMAND_DECODER,
  RBUF_CAPTURE_COMMAND_PASSTHRU,
  RBUF_CAPTURE_COMMAND_MIXER,
  RBUF_CAPTURE_COMMAND_ENCODER,
  RBUF_CAPTURE_COMMAND_TRANSCODE,
  RBUF_CAPTURE_COMMAND_SPKR_MGMT,
  RBUF_CAPTURE_COMMAND_NUM_COMMANDS
} RBUF_CAPTURE_COMMAND;

static int8_t *RBUF_CAPTURE_COMMAND_strings[] =
    {
      (int8_t *)"#INIT",
      (int8_t *)"#CAPTURE_PATH",
      (int8_t *)"#CAPTURE_FILENAME",
      (int8_t *)"#COPY_PATH",
      (int8_t *)"#ECHO",
      (int8_t *)"#DECODER",
      (int8_t *)"#PASSTHRU",
      (int8_t *)"#MIXER",
      (int8_t *)"#ENCODER",
      (int8_t *)"#TRANSCODE",
      (int8_t *)"#SPKR_MGMT",
    };

typedef struct RBUF_CAPTURE_COMMAND_t {
	int8_t buffer[512];
	RBUF_CAPTURE_COMMAND command;
	uint32_t args[2];
} RBUF_CAPTURE_COMMAND_t;

static unsigned int loop_break = 0;
static unsigned int capture_count=0;
static bool cap_count_flag = false;

void start_rbuf_channels_from_env(void);

#endif
