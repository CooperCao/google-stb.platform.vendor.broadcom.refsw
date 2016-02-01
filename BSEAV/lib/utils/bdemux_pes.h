/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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

#ifndef _BDEMUX_PES_H__
#define _BDEMUX_PES_H__

#include "bdemux_ts.h"

#define BDEMUX_PES_PTS (1<<8)
#define BDEMUX_PES_DATA_ALIGMENT (1<<9)
#define BDEMUX_PES_DISCONTINUITY  (1<<10)

#define BDEMUX_PES_PTS_UNITS	45000

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bdemux_pes {
	bdemux_ts ts;
	enum {bdemux_pes_state_sync,bdemux_pes_state_hdr, bdemux_pes_state_data} state;
	uint16_t pkt_offset; /* current offset in the PES packet */
	uint16_t pkt_data_offset; /* offset in the PES header where data starts */
	unsigned ts_flags; /* flags that are accumulated from the transport layer */
	uint16_t pkt_len; /* PES packet length */
	uint8_t id; /* stream ID */
	uint8_t hold_enable; /* consumer could set this bit to hold consumption of es data */
	uint16_t pes_packets;
	uint8_t pes_header[14];
	BDBG_OBJECT(bdemux_pes)
} bdemux_pes;

void bdemux_pes_init(bdemux_pes *pes, uint8_t id);
void bdemux_pes_data(bdemux_pes *pes, const uint8_t *data, size_t len, unsigned flags);
void bdemux_pes_header(bdemux_pes *pes, unsigned flags, uint32_t pts);

#ifdef __cplusplus
}
#endif

#endif /* _BDEMUX_PES_H__ */

