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
 * RTP parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BTIMESTAMP_H__
#define _BTIMESTAMP_H__

#ifdef __cplusplus
extern "C"
{
#endif

enum btimestamp_state {btimestamp_state_init, btimestamp_state_normal, btimestamp_state_wrap}; 
typedef struct btimestamp {
	uint32_t last_timestamp;
	unsigned wrap_count;
	enum btimestamp_state state;
} btimestamp;

/* this function initialized timestamp */
void btimestamp_init(btimestamp *timestamp);
/* this function adds new timestamp */
void btimestamp_add(btimestamp *timestamp, uint32_t ts);
/* this function returns current 64 bit value of timestamp */
uint64_t btimestamp_get(const btimestamp *timestamp);

typedef struct brtp_ntp {
	uint32_t ntp_msw;
	uint32_t ntp_lsw;
} brtp_ntp;

void brtp_ntp_init(brtp_ntp *ntp, uint32_t ntp_msw, uint32_t ntp_lsw);
/* this function returns a - b in mseconds */
int brtp_ntp_diff(const brtp_ntp *a, const brtp_ntp *b);

/* this function takes two timestamps and returs difference in half rate */
int brtp_delta_time(uint32_t a, uint32_t b);

/* this function takes two timestamps and returs difference in full rate */
int brtp_delta_time_full(uint32_t a, uint32_t b);
#define brtp_delta_time_full(a,b) ((int32_t)(a)-(int32_t)(b))



#ifdef __cplusplus
}
#endif

#endif /* _BTIMESTAMP_H__ */

