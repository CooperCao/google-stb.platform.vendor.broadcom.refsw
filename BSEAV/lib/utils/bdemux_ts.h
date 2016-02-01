/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * MPEG-2 Demux library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BDEMUX_TS_H__
#define _BDEMUX_TS_H__


#define BDEMUX_TS_ERROR (1<<0)
#define BDEMUX_TS_DISCONTINUITY (1<<1)
#define BDEMUX_TS_MARKED_DISCONTINUITY (1<<2)
#define BDEMUX_TS_PAYLOAD_UNIT_START (1<<3)

#define B_TS_SYNC	0x47
#define B_TS_PKT_LEN 188

#define B_TS_LOAD8(ptr, off)  ((((uint8_t *)(ptr))[(off)+0])) 
#define B_TS_LOAD16(ptr, off)  (((uint16_t)((uint8_t *)(ptr))[(off)+0]<<8) | ((uint16_t)((uint8_t *)(ptr))[(off)+1]))

#define B_TS_LOAD32(ptr, off)  (((uint32_t)((uint8_t *)(ptr))[(off)+0]<<24) | ((uint32_t)((uint8_t *)(ptr))[(off)+1]<<16) | ((uint32_t)((uint8_t *)(ptr))[(off)+2]<<8) | ((uint32_t)((uint8_t *)(ptr))[(off)+3]))

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bdemux_ts {
	uint16_t pid;
	uint8_t continuity_counter;
	uint8_t flags;
	BDBG_OBJECT(bdemux_ts)
} bdemux_ts;

typedef enum {bdemux_ts_action_hold, bdemux_ts_action_consume} bdemux_ts_action;

void bdemux_ts_init(bdemux_ts *ts, uint16_t pid);
int bdemux_ts_feed(bdemux_ts *ts, const uint8_t *data, size_t len);
bdemux_ts_action bdemux_ts_data(bdemux_ts *ts, unsigned flags, const uint8_t *data, size_t len);
#ifdef __cplusplus
}
#endif

#endif /* _BDEMUX_TS_H__ */

