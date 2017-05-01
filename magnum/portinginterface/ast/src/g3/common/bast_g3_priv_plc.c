/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/
/* generated 09-02-2010 09:58:32 */

#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"

BDBG_MODULE(bast_g3_priv_plc);

#define BAST_DEBUG_PLC(x) /* x */


#define BAST_g3_PLC_DAMP_SCALE 8
#define BAST_g3_PLC_NUM_SYMRATES 7

/******************************************************************************
 start of generated data
 1. copy all tables up to plc_data from plc_40nm.xls file
 2. rename "PLC_NUM_SYMRATES" to "BAST_g3_PLC_NUM_SYMRATES"
******************************************************************************/
/* generated 11-24-2013 17:38:42 */

static const uint32_t plc_symbol_rate[BAST_g3_PLC_NUM_SYMRATES] = {
	1000000,
	1875000,
	3750000,
	7500000,
	15000000,
	30000000,
	45000000
};

static const uint16_t acq_legacy_scale[BAST_g3_PLC_NUM_SYMRATES] = {
	12,
	8,
	21,
	46,
	46,
	36,
	51
};

static const uint16_t acq_ldpc_scale[BAST_g3_PLC_NUM_SYMRATES] = {
	66,
	131,
	146,
	221,
	516,
	950,
	1455
};

static const uint16_t acq_turbo_scale[BAST_g3_PLC_NUM_SYMRATES] = {
	6,
	12,
	20,
	20,
	16,
	29,
	25
};

static const uint16_t trk_legacy_scale[BAST_g3_PLC_NUM_SYMRATES] = {
	10,
	12,
	16,
	25,
	31,
	35,
	42
};

static const uint16_t trk_ldpc_scale[BAST_g3_PLC_NUM_SYMRATES] = {
	65,
	100,
	150,
	225,
	315,
	638,
	929
};

static const uint16_t trk_turbo_scale[BAST_g3_PLC_NUM_SYMRATES] = {
	10,
	8,
	12,
	8,
	14,
	16,
	24
};

static const uint32_t acq_legacy_min_bw[BAST_g3_PLC_NUM_SYMRATES] = {
	4300,
	5500,
	5400,
	6500,
	7500,
	12000,
	14000
};

static const uint32_t acq_ldpc_min_bw[BAST_g3_PLC_NUM_SYMRATES] = {
	2300,
	2800,
	5400,
	8900,
	13500,
	18000,
	19000
};

static const uint32_t acq_turbo_min_bw[BAST_g3_PLC_NUM_SYMRATES] = {
	2000,
	5000,
	7000,
	9000,
	12000,
	21775,
	32662
};

static const uint32_t trk_legacy_min_bw[BAST_g3_PLC_NUM_SYMRATES] = {
	3000,
	3000,
	4000,
	4800,
	7200,
	9100,
	10500
};

static const uint32_t trk_ldpc_min_bw[BAST_g3_PLC_NUM_SYMRATES] = {
	1100,
	1100,
	1500,
	1800,
	2600,
	3100,
	6300
};

static const uint32_t trk_turbo_min_bw[BAST_g3_PLC_NUM_SYMRATES] = {
	2000,
	3000,
	5000,
	7000,
	10000,
	12000,
	16000
};

static const uint8_t plc_data[] = {
0x5C,	0x26,	0x72,	0x62,	0xCF,	0xC2,	0xC4,	0x38,	0x40,	0x48,	0x50,	0x58,	0x70,	0x80,	/* acq_default_dvb_low */
0x7D,	0x00,	0x94,	0x78,	0xE4,	0xFA,	0xD8,	0x38,	0x40,	0x48,	0x50,	0x58,	0x70,	0x80,	/* acq_default_dvb_high */
0xE9,	0xE4,	0x72,	0x62,	0xCF,	0xC2,	0xC4,	0x38,	0x40,	0x48,	0x50,	0x58,	0x70,	0x80,	/* acq_euro_dvb_low */
0xF2,	0xE4,	0x94,	0x78,	0xE4,	0xFA,	0xD8,	0x38,	0x40,	0x48,	0x50,	0x58,	0x70,	0x80,	/* acq_euro_dvb_high */
0x00,	0x19,	0x00,	0x00,	0x00,	0x00,	0x00,	0x38,	0x38,	0x40,	0x48,	0x58,	0x78,	0x80,	/* acq_dtv_low */
0x19,	0x32,	0x64,	0x2B,	0x4C,	0x1C,	0x27,	0x38,	0x40,	0x48,	0x50,	0x64,	0x80,	0x88,	/* acq_dtv_high */
0x00,	0xE4,	0xF3,	0xAE,	0xCF,	0xC2,	0xD8,	0x38,	0x40,	0x48,	0x50,	0x68,	0x70,	0x78,	/* acq_dcii_low */
0x19,	0xE4,	0xF3,	0xFA,	0xFA,	0xFA,	0xFF,	0x38,	0x40,	0x48,	0x50,	0x68,	0x70,	0x78,	/* acq_dcii_high */
0x26,	0x17,	0x1C,	0x37,	0x1C,	0x04,	0x03,	0x38,	0x40,	0x48,	0x48,	0x48,	0x48,	0x60,	/* acq_default_pilot_ldpc_qpsk_low */
0x2A,	0x1A,	0x18,	0x29,	0x1C,	0x05,	0x05,	0x40,	0x40,	0x60,	0x70,	0x88,	0x98,	0xA8,	/* acq_default_pilot_ldpc_qpsk_high */
0x23,	0x17,	0x09,	0x17,	0x00,	0x00,	0x00,	0x30,	0x30,	0x40,	0x48,	0x48,	0x48,	0x50,	/* acq_default_pilot_ldpc_8psk_low */
0x23,	0x17,	0x09,	0x1C,	0x05,	0x05,	0x05,	0x30,	0x30,	0x40,	0x68,	0x68,	0x78,	0x80,	/* acq_default_pilot_ldpc_8psk_high */
0x41,	0x55,	0x45,	0xA3,	0xFF,	0xFF,	0xFF,	0x30,	0x34,	0x3C,	0x34,	0x34,	0x30,	0x30,	/* acq_default_nopilot_ldpc_qpsk_low */
0x41,	0x55,	0x4F,	0xA3,	0xFF,	0xFF,	0xFF,	0x30,	0x34,	0x3C,	0x34,	0x34,	0x30,	0x30,	/* acq_default_nopilot_ldpc_qpsk_high */
0x00,	0x00,	0x00,	0x00,	0x09,	0x1A,	0x1E,	0x88,	0x88,	0x88,	0x88,	0x88,	0x88,	0x90,	/* acq_default_nopilot_ldpc_8psk_low */
0x00,	0x00,	0x00,	0x00,	0x09,	0x10,	0x1E,	0x88,	0x88,	0x88,	0x88,	0x88,	0x88,	0x90,	/* acq_default_nopilot_ldpc_8psk_high */
0x9B,	0x7C,	0xAF,	0x91,	0x2C,	0x15,	0x11,	0x38,	0x48,	0x50,	0x50,	0x50,	0x54,	0x70,	/* acq_euro_pilot_ldpc_qpsk_low */
0x8B,	0x6C,	0x6B,	0x56,	0x31,	0x1A,	0x13,	0x40,	0x50,	0x60,	0x80,	0xA0,	0xB0,	0xC0,	/* acq_euro_pilot_ldpc_qpsk_high */
0x75,	0x55,	0x3B,	0x25,	0x14,	0x0E,	0x10,	0x40,	0x48,	0x50,	0x80,	0x80,	0x58,	0x80,	/* acq_euro_pilot_ldpc_8psk_low */
0x4F,	0x4A,	0x3B,	0x2E,	0x1C,	0x10,	0x13,	0x40,	0x48,	0x50,	0x80,	0x88,	0x88,	0x98,	/* acq_euro_pilot_ldpc_8psk_high */
0x6D,	0x74,	0x5D,	0x69,	0x68,	0x3D,	0x45,	0x50,	0x50,	0x58,	0x60,	0x88,	0xA0,	0xA8,	/* acq_euro_nopilot_ldpc_qpsk_low */
0x7C,	0x7C,	0x7F,	0x7B,	0x6F,	0x2C,	0x4C,	0x48,	0x58,	0x58,	0x60,	0x70,	0xB0,	0xB8,	/* acq_euro_nopilot_ldpc_qpsk_high */
0xFD,	0xDF,	0xFE,	0xFE,	0x75,	0x3C,	0x53,	0x60,	0x78,	0x80,	0x90,	0x88,	0xB8,	0xC8,	/* acq_euro_nopilot_ldpc_8psk_low */
0xEE,	0xFD,	0xDF,	0xDE,	0x7D,	0x42,	0x5A,	0x68,	0x58,	0x68,	0xA0,	0xB0,	0xC0,	0xF8,	/* acq_euro_nopilot_ldpc_8psk_high */
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x38,	0x30,	0x40,	0x48,	0x48,	0x50,	0x58,	/* acq_turbo_qpsk_low */
0xA7,	0x00,	0x1B,	0x05,	0xC1,	0x77,	0xCF,	0x40,	0x28,	0x40,	0x40,	0x40,	0x48,	0x50,	/* acq_turbo_qpsk_high */
0x64,	0xA7,	0x64,	0x96,	0x3F,	0xC7,	0xD6,	0x30,	0x28,	0x28,	0x30,	0x44,	0x48,	0x50,	/* acq_turbo_8psk_low */
0xFA,	0xFA,	0xFA,	0xFA,	0xFA,	0xF9,	0xFE,	0x30,	0x30,	0x30,	0x38,	0x40,	0x48,	0x50,	/* acq_turbo_8psk_high */
0x00,	0x32,	0x00,	0x00,	0x00,	0x00,	0x00,	0x28,	0x30,	0x38,	0x40,	0x48,	0x60,	0x70,	/* trk_default_dvb_low */
0x14,	0x43,	0x0D,	0x0C,	0x03,	0x03,	0x24,	0x28,	0x30,	0x38,	0x40,	0x48,	0x60,	0x70,	/* trk_default_dvb_high */
0xDC,	0xFA,	0xBC,	0xA8,	0xDB,	0xC5,	0xB3,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x70,	/* trk_euro_dvb_low */
0xFA,	0xFA,	0xFA,	0xF8,	0xFC,	0xFE,	0xFA,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x70,	/* trk_euro_dvb_high */
0x14,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x28,	0x28,	0x38,	0x40,	0x48,	0x60,	0x70,	/* trk_dtv_low */
0x14,	0x2A,	0x0D,	0x0C,	0x03,	0x03,	0x24,	0x28,	0x30,	0x38,	0x40,	0x48,	0x60,	0x70,	/* trk_dtv_high */
0x32,	0x7D,	0x8A,	0x80,	0x5A,	0x6F,	0x6B,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x68,	/* trk_dcii_low */
0x32,	0x7D,	0x8A,	0xD0,	0x9B,	0x8C,	0x83,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x68,	/* trk_dcii_high */
0x23,	0x1F,	0x25,	0x3F,	0x2B,	0x16,	0x0C,	0x28,	0x30,	0x38,	0x38,	0x38,	0x38,	0x50,	/* trk_default_pilot_ldpc_qpsk_low */
0x26,	0x21,	0x21,	0x29,	0x2B,	0x17,	0x0E,	0x30,	0x34,	0x50,	0x60,	0x78,	0x88,	0x98,	/* trk_default_pilot_ldpc_qpsk_high */
0x06,	0x09,	0x07,	0x05,	0x03,	0x04,	0x05,	0x20,	0x28,	0x30,	0x3C,	0x3C,	0x3C,	0x3C,	/* trk_default_pilot_ldpc_8psk_low */
0x1D,	0x1D,	0x17,	0x20,	0x17,	0x14,	0x0C,	0x18,	0x1C,	0x20,	0x20,	0x28,	0x30,	0x38,	/* trk_default_pilot_ldpc_8psk_3_4 */
0x1D,	0x16,	0x0F,	0x0C,	0x09,	0x11,	0x0A,	0x38,	0x40,	0x48,	0x50,	0x5C,	0x60,	0x60,	/* trk_default_pilot_ldpc_8psk_5_6 */
0x1D,	0x27,	0x1E,	0x20,	0x1B,	0x14,	0x0D,	0x28,	0x30,	0x40,	0x5C,	0x60,	0x68,	0x70,	/* trk_default_pilot_ldpc_8psk_high */
0x34,	0x4F,	0x6E,	0xA4,	0xFF,	0xFF,	0xFF,	0x20,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_default_nopilot_ldpc_qpsk_low */
0x34,	0x4F,	0x6E,	0xA4,	0xFF,	0xFF,	0xFF,	0x20,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_default_nopilot_ldpc_qpsk_high */
0x00,	0x04,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x7A,	/* trk_default_nopilot_ldpc_8psk_low */
0x00,	0x04,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_default_nopilot_ldpc_8psk_3_4 */
0x00,	0x04,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_default_nopilot_ldpc_8psk_5_6 */
0x00,	0x04,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_default_nopilot_ldpc_8psk_high */
0x72,	0x81,	0x85,	0x63,	0x44,	0x23,	0x15,	0x28,	0x36,	0x3E,	0x43,	0x43,	0x43,	0x60,	/* trk_euro_pilot_ldpc_qpsk_low */
0x64,	0x70,	0x53,	0x41,	0x44,	0x25,	0x18,	0x30,	0x3C,	0x50,	0x73,	0x90,	0xA3,	0xB6,	/* trk_euro_pilot_ldpc_qpsk_high */
0x11,	0x11,	0x0E,	0x0C,	0x08,	0x08,	0x0B,	0x30,	0x38,	0x40,	0x48,	0x48,	0x48,	0x48,	/* trk_euro_pilot_ldpc_8psk_low */
0x86,	0x69,	0x49,	0x34,	0x27,	0x21,	0x15,	0x30,	0x38,	0x3D,	0x26,	0x30,	0x3A,	0x43,	/* trk_euro_pilot_ldpc_8psk_3_4 */
0x36,	0x28,	0x1D,	0x16,	0x11,	0x1C,	0x12,	0x30,	0x38,	0x3F,	0x60,	0x6E,	0x73,	0x73,	/* trk_euro_pilot_ldpc_8psk_5_6 */
0x53,	0x4B,	0x43,	0x34,	0x2C,	0x21,	0x16,	0x30,	0x36,	0x3C,	0x6E,	0x73,	0x7D,	0x86,	/* trk_euro_pilot_ldpc_8psk_high */
0x61,	0x64,	0x65,	0x67,	0x96,	0x59,	0x5A,	0x40,	0x44,	0x48,	0x50,	0x78,	0x90,	0x98,	/* trk_euro_nopilot_ldpc_qpsk_low */
0x7A,	0x7C,	0x7D,	0x80,	0xB6,	0x50,	0x50,	0x38,	0x40,	0x44,	0x50,	0x60,	0xA0,	0xA8,	/* trk_euro_nopilot_ldpc_qpsk_high */
0x98,	0x9A,	0x9B,	0x9C,	0x57,	0x4D,	0x4D,	0x40,	0x48,	0x54,	0x74,	0x68,	0x80,	0x88,	/* trk_euro_nopilot_ldpc_8psk_low */
0xFE,	0xFE,	0xFD,	0xFE,	0xB6,	0x67,	0x68,	0x50,	0x60,	0x74,	0x84,	0xB0,	0xD0,	0xF8,	/* trk_euro_nopilot_ldpc_8psk_3_4 */
0xCB,	0xCB,	0xCB,	0xCD,	0xD1,	0x72,	0x74,	0x58,	0x68,	0x84,	0x96,	0xC0,	0xE0,	0xE8,	/* trk_euro_nopilot_ldpc_8psk_5_6 */
0xDE,	0xDD,	0xDD,	0xDF,	0xD6,	0x6F,	0x71,	0x40,	0x4C,	0x5C,	0x7C,	0xB8,	0xB0,	0xB8,	/* trk_euro_nopilot_ldpc_8psk_high */
0x0C,	0x11,	0x0F,	0x19,	0x13,	0x0B,	0x06,	0x28,	0x30,	0x38,	0x38,	0x38,	0x48,	0x50,	/* trk_awgn_pilot_ldpc_qpsk_low */
0x0F,	0x14,	0x0D,	0x14,	0x13,	0x0B,	0x07,	0x30,	0x34,	0x50,	0x60,	0x78,	0x3E,	0x98,	/* trk_awgn_pilot_ldpc_qpsk_high */
0x00,	0x02,	0x01,	0x00,	0x00,	0x00,	0x00,	0x20,	0x28,	0x30,	0x3C,	0x3C,	0x3C,	0x3C,	/* trk_awgn_pilot_ldpc_8psk_low */
0x08,	0x07,	0x05,	0x06,	0x06,	0x08,	0x05,	0x18,	0x1C,	0x20,	0x20,	0x28,	0x30,	0x38,	/* trk_awgn_pilot_ldpc_8psk_3_4 */
0x08,	0x08,	0x04,	0x04,	0x03,	0x05,	0x07,	0x38,	0x40,	0x48,	0x50,	0x60,	0x60,	0x60,	/* trk_awgn_pilot_ldpc_8psk_5_6 */
0x0E,	0x0F,	0x0F,	0x0D,	0x0A,	0x06,	0x06,	0x28,	0x30,	0x40,	0x5C,	0x60,	0x68,	0x70,	/* trk_awgn_pilot_ldpc_8psk_high */
0x0F,	0x1F,	0x32,	0x4C,	0x7A,	0x79,	0x7A,	0x20,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_awgn_nopilot_ldpc_qpsk_low */
0x12,	0x22,	0x32,	0x4C,	0x7A,	0x79,	0x7A,	0x20,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_awgn_nopilot_ldpc_qpsk_high */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x7A,	/* trk_awgn_nopilot_ldpc_8psk_low */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_awgn_nopilot_ldpc_8psk_3_4 */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_awgn_nopilot_ldpc_8psk_5_6 */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_awgn_nopilot_ldpc_8psk_high */
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x18,	0x28,	0x38,	0x38,	0x40,	0x48,	0x38,	/* trk_turbo_qpsk_low */
0x32,	0x8A,	0xD4,	0xFA,	0x47,	0x3F,	0x53,	0x18,	0x28,	0x30,	0x38,	0x40,	0x40,	0x38,	/* trk_turbo_qpsk_high */
0x32,	0x7D,	0x00,	0x00,	0x47,	0x3F,	0xA7,	0x28,	0x38,	0x40,	0x40,	0x40,	0x40,	0x38,	/* trk_turbo_8psk_low */
0xFA,	0xFA,	0xFA,	0xFA,	0xFA,	0xFA,	0xFA,	0x28,	0x30,	0x48,	0x48,	0x48,	0x48,	0x38,	/* trk_turbo_8psk_high */
};


/******************************************************************************
 end of generated data
******************************************************************************/

enum
{
   BAST_acq_default_dvb_low_bw, /* 0. acq_default_dvb_low_bw */
   BAST_acq_default_dvb_high_bw, /* 1. acq_default_dvb_high_bw */
   BAST_acq_euro_dvb_low_bw, /* 2. acq_euro_dvb_low_bw */
   BAST_acq_euro_dvb_high_bw, /* 3. acq_euro_dvb_high_bw */
   BAST_acq_dtv_low_bw, /* 4. acq_dtv_low_bw */
   BAST_acq_dtv_high_bw, /* 5. acq_dtv_high_bw */
   BAST_acq_dcii_low_bw, /* 6. acq_dcii_low_bw */
   BAST_acq_dcii_high_bw, /* 7. acq_dcii_high_bw */
   BAST_acq_default_pilot_ldpc_qpsk_low_bw, /* 8. acq_default_pilot_ldpc_qpsk_low_bw */
   BAST_acq_default_pilot_ldpc_qpsk_high_bw, /* 9. acq_default_pilot_ldpc_qpsk_high_bw */
   BAST_acq_default_pilot_ldpc_8psk_low_bw, /* 10. acq_default_pilot_ldpc_8psk_low_bw */
   BAST_acq_default_pilot_ldpc_8psk_high_bw, /* 11. acq_default_pilot_ldpc_8psk_high_bw */
   BAST_acq_default_nopilot_ldpc_qpsk_low_bw, /* 12. acq_default_nopilot_ldpc_qpsk_low_bw */
   BAST_acq_default_nopilot_ldpc_qpsk_high_bw, /* 13. acq_default_nopilot_ldpc_qpsk_high_bw */
   BAST_acq_default_nopilot_ldpc_8psk_low_bw, /* 14. acq_default_nopilot_ldpc_8psk_low_bw */
   BAST_acq_default_nopilot_ldpc_8psk_high_bw, /* 15. acq_default_nopilot_ldpc_8psk_high_bw */
   BAST_acq_euro_pilot_ldpc_qpsk_low_bw, /* 16. acq_euro_pilot_ldpc_qpsk_low_bw */
   BAST_acq_euro_pilot_ldpc_qpsk_high_bw, /* 17. acq_euro_pilot_ldpc_qpsk_high_bw */
   BAST_acq_euro_pilot_ldpc_8psk_low_bw, /* 18. acq_euro_pilot_ldpc_8psk_low_bw */
   BAST_acq_euro_pilot_ldpc_8psk_high_bw, /* 19. acq_euro_pilot_ldpc_8psk_high_bw */
   BAST_acq_euro_nopilot_ldpc_qpsk_low_bw, /* 20. acq_euro_nopilot_ldpc_qpsk_low_bw */
   BAST_acq_euro_nopilot_ldpc_qpsk_high_bw, /* 21. acq_euro_nopilot_ldpc_qpsk_high_bw */
   BAST_acq_euro_nopilot_ldpc_8psk_low_bw, /* 22. acq_euro_nopilot_ldpc_8psk_low_bw */
   BAST_acq_euro_nopilot_ldpc_8psk_high_bw, /* 23. acq_euro_nopilot_ldpc_8psk_high_bw */
   BAST_acq_turbo_qpsk_low_bw, /* 24. acq_turbo_qpsk_low_bw */
   BAST_acq_turbo_qpsk_high_bw, /* 25. acq_turbo_qpsk_high_bw */
   BAST_acq_turbo_8psk_low_bw, /* 26. acq_turbo_8psk_low_bw */
   BAST_acq_turbo_8psk_high_bw, /* 27. acq_turbo_8psk_high_bw */
   BAST_trk_default_dvb_low_bw, /* 28. trk_default_dvb_low_bw */
   BAST_trk_default_dvb_high_bw, /* 29. trk_default_dvb_high_bw */
   BAST_trk_euro_dvb_low_bw, /* 30. trk_euro_dvb_low_bw */
   BAST_trk_euro_dvb_high_bw, /* 31. trk_euro_dvb_high_bw */
   BAST_trk_dtv_low_bw, /* 32. trk_dtv_low_bw */
   BAST_trk_dtv_high_bw, /* 33. trk_dtv_high_bw */
   BAST_trk_dcii_low_bw, /* 34. trk_dcii_low_bw */
   BAST_trk_dcii_high_bw, /* 35. trk_dcii_high_bw */
   BAST_trk_default_pilot_ldpc_qpsk_low_bw, /* 36. trk_default_pilot_ldpc_qpsk_low_bw */
   BAST_trk_default_pilot_ldpc_qpsk_high_bw, /* 37. trk_default_pilot_ldpc_qpsk_high_bw */
   BAST_trk_default_pilot_ldpc_8psk_low_bw, /* 38. trk_default_pilot_ldpc_8psk_low_bw */
   BAST_trk_default_pilot_ldpc_8psk_3_4_bw, /* 39. trk_default_pilot_ldpc_8psk_3_4_bw */
   BAST_trk_default_pilot_ldpc_8psk_5_6_bw, /* 40. trk_default_pilot_ldpc_8psk_5_6_bw */
   BAST_trk_default_pilot_ldpc_8psk_high_bw, /* 41. trk_default_pilot_ldpc_8psk_high_bw */
   BAST_trk_default_nopilot_ldpc_qpsk_low_bw, /* 42. trk_default_nopilot_ldpc_qpsk_low_bw */
   BAST_trk_default_nopilot_ldpc_qpsk_high_bw, /* 43. trk_default_nopilot_ldpc_qpsk_high_bw */
   BAST_trk_default_nopilot_ldpc_8psk_low_bw, /* 44. trk_default_nopilot_ldpc_8psk_low_bw */
   BAST_trk_default_nopilot_ldpc_8psk_3_4_bw, /* 45. trk_default_nopilot_ldpc_8psk_3_4_bw */
   BAST_trk_default_nopilot_ldpc_8psk_5_6_bw, /* 46. trk_default_nopilot_ldpc_8psk_5_6_bw */
   BAST_trk_default_nopilot_ldpc_8psk_high_bw, /* 47. trk_default_nopilot_ldpc_8psk_high_bw */
   BAST_trk_euro_pilot_ldpc_qpsk_low_bw, /* 48. trk_euro_pilot_ldpc_qpsk_low_bw */
   BAST_trk_euro_pilot_ldpc_qpsk_high_bw, /* 49. trk_euro_pilot_ldpc_qpsk_high_bw */
   BAST_trk_euro_pilot_ldpc_8psk_low_bw, /* 50. trk_euro_pilot_ldpc_8psk_low_bw */
   BAST_trk_euro_pilot_ldpc_8psk_3_4_bw, /* 51. trk_euro_pilot_ldpc_8psk_3_4_bw */
   BAST_trk_euro_pilot_ldpc_8psk_5_6_bw, /* 52. trk_euro_pilot_ldpc_8psk_5_6_bw */
   BAST_trk_euro_pilot_ldpc_8psk_high_bw, /* 53. trk_euro_pilot_ldpc_8psk_high_bw */
   BAST_trk_euro_nopilot_ldpc_qpsk_low_bw, /* 54. trk_euro_nopilot_ldpc_qpsk_low_bw */
   BAST_trk_euro_nopilot_ldpc_qpsk_high_bw, /* 55. trk_euro_nopilot_ldpc_qpsk_high_bw */
   BAST_trk_euro_nopilot_ldpc_8psk_low_bw, /* 56. trk_euro_nopilot_ldpc_8psk_low_bw */
   BAST_trk_euro_nopilot_ldpc_8psk_3_4_bw, /* 57. trk_euro_nopilot_ldpc_8psk_3_4_bw */
   BAST_trk_euro_nopilot_ldpc_8psk_5_6_bw, /* 58. trk_euro_nopilot_ldpc_8psk_5_6_bw */
   BAST_trk_euro_nopilot_ldpc_8psk_high_bw, /* 59. trk_euro_nopilot_ldpc_8psk_high_bw */
   BAST_trk_awgn_pilot_ldpc_qpsk_low_bw, /* 60. trk_awgn_pilot_ldpc_qpsk_low_bw */
   BAST_trk_awgn_pilot_ldpc_qpsk_high_bw, /* 61. trk_awgn_pilot_ldpc_qpsk_high_bw */
   BAST_trk_awgn_pilot_ldpc_8psk_low_bw, /* 62. trk_awgn_pilot_ldpc_8psk_low_bw */
   BAST_trk_awgn_pilot_ldpc_8psk_3_4_bw, /* 63. trk_awgn_pilot_ldpc_8psk_3_4_bw */
   BAST_trk_awgn_pilot_ldpc_8psk_5_6_bw, /* 64. trk_awgn_pilot_ldpc_8psk_5_6_bw */
   BAST_trk_awgn_pilot_ldpc_8psk_high_bw, /* 65. trk_awgn_pilot_ldpc_8psk_high_bw */
   BAST_trk_awgn_nopilot_ldpc_qpsk_low_bw, /* 66. trk_awgn_nopilot_ldpc_qpsk_low_bw */
   BAST_trk_awgn_nopilot_ldpc_qpsk_high_bw, /* 67. trk_awgn_nopilot_ldpc_qpsk_high_bw */
   BAST_trk_awgn_nopilot_ldpc_8psk_low_bw, /* 68. trk_awgn_nopilot_ldpc_8psk_low_bw */
   BAST_trk_awgn_nopilot_ldpc_8psk_3_4_bw, /* 69. trk_awgn_nopilot_ldpc_8psk_3_4_bw */
   BAST_trk_awgn_nopilot_ldpc_8psk_5_6_bw, /* 70. trk_awgn_nopilot_ldpc_8psk_5_6_bw */
   BAST_trk_awgn_nopilot_ldpc_8psk_high_bw, /* 71. trk_awgn_nopilot_ldpc_8psk_high_bw */
   BAST_trk_turbo_qpsk_low_bw, /* 72. trk_turbo_qpsk_low_bw */
   BAST_trk_turbo_qpsk_high_bw, /* 73. trk_turbo_qpsk_high_bw */
   BAST_trk_turbo_8psk_low_bw, /* 74. trk_turbo_8psk_low_bw */
   BAST_trk_turbo_8psk_high_bw /* 75. trk_turbo_8psk_high_bw */
};


/******************************************************************************
 BAST_g3_P_LookupPlcTable_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_LookupPlcTable_isr(const uint8_t *pBwTable, const uint16_t *pBwScaleTable, const uint32_t *pMinBwTable, uint32_t symRateIdx, uint32_t *pBw, uint32_t *pDamp8)
{
   BERR_Code retCode = BERR_SUCCESS;

   if ((pBwTable == NULL) || (pBwScaleTable == NULL) || (pMinBwTable == NULL))
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      goto done;
   }
   *pBw = ((uint32_t)pBwTable[symRateIdx] * (uint32_t)pBwScaleTable[symRateIdx]) + pMinBwTable[symRateIdx];
   *pDamp8 = pBwTable[BAST_g3_PLC_NUM_SYMRATES + symRateIdx];
   BAST_DEBUG_PLC(BDBG_MSG(("BAST_g3_P_LookupPlcTable_isr(): bw=%u, damp8=%d", *pBw, *pDamp8)));

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_SetPlc_isr()
******************************************************************************/
BERR_Code BAST_g3_P_SetPlc_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t i, j, b0, b1, b2, b3, val, P_hi, P_lo, Q_hi, Q_lo;
   bool ovr;

   BAST_DEBUG_PLC(BDBG_MSG(("BAST_g3_P_SetPlc_isr(): bw=%u, damp*8=%d", bw, damp)));

   /* compute 6-bit linear coefficient */
   for (i = 0; i < 7; i++)
   {
      b0 = (((1 << (i << 1)) * bw) >> 2) * damp;
      if (b0 > hChn->acqParams.symbolRate)
         break;
   }
   i--;
   BMTH_HILO_32TO64_Mul((1 << (i << 1)) * damp, bw << 6, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   b0 = (Q_lo + 4) >> 3;
   if (b0 & 0xFFFFFFE0)
      b0 = 0x1F;
   b1 = 7 - i;

   /* compute 6-bit integrator coefficient */
   if (bw > 65535)
      ovr = true;
   else
      ovr = false;
   for (j = 0; j < 14; j++)
   {
      if (ovr)
         BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 1, bw, &P_hi, &P_lo);
      else
         BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 1, bw * bw, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);

      if (ovr)
         Q_lo *= bw;
      Q_lo = (Q_lo + 1) >> 1; /* round */
      if (Q_lo > hChn->acqParams.symbolRate)
         break;
   }
   j--;
   if (ovr)
      BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 5, bw << 7, &P_hi, &P_lo);
   else
      BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 5, (bw * bw), &P_hi, &P_lo);

   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   if (ovr)
   {
      Q_lo = (Q_lo + 1) >> 1;
      BMTH_HILO_32TO64_Mul(Q_lo, bw << 2, &P_hi, &P_lo);
   }
   else
      BMTH_HILO_32TO64_Mul(Q_lo, 1 << 8, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   b2 = (Q_lo + 128) >> 8;
   if (b2 & 0xFFFFFFE0)
      b2 = 0x1F;
   b3 = 13 - j;

   val = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
   BAST_DEBUG_PLC(BDBG_MSG(("PLC=0x%08X", val)));
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLC, &val);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLC1, &val);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FBLC, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_ConfigPlc_isr()
******************************************************************************/
BERR_Code BAST_g3_P_ConfigPlc_isr(BAST_ChannelHandle h, bool bAcq)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, symbol_rate_hi, symbol_rate_lo, bw, bw_hi, bw_lo, damp8, damp8_hi, damp8_lo, baseTblIdx, symRateIdx, P_hi, P_lo, Q_hi, Q_lo;
   const uint32_t *pMinBwTable;
   const uint16_t *pScaleTable;
   const uint8_t *pPlcData;
   bool bDoInterp;
   BERR_Code retCode;
   BAST_Mode mode;

#ifdef BAST_EXCLUDE_LDPC
   BSTD_UNUSED(acq_ldpc_scale);
   BSTD_UNUSED(trk_ldpc_scale);
   BSTD_UNUSED(acq_ldpc_min_bw);
   BSTD_UNUSED(trk_ldpc_min_bw);
#endif

   baseTblIdx = 0;  /* baseTblIdx=index into plc_tables[] */
   pScaleTable = NULL;
   pMinBwTable = NULL;

   if (bAcq)
   {
      mode = hChn->acqParams.mode;

      BAST_DEBUG_PLC(BDBG_MSG(("setting acquisition PLC...")));
      /* acquisition */
      if (hChn->plcCtl & BAST_G3_CONFIG_PLC_CTL_OVERRIDE_ACQ)
      {
         bw = hChn->altPlcAcqBw;
         damp8 = hChn->altPlcAcqDamp;
         goto set_plc;
      }
      else
      {
         /* AST PI determines acq PLC */
         if (BAST_MODE_IS_LEGACY_QPSK(mode))
         {
            pScaleTable = acq_legacy_scale;
            pMinBwTable = acq_legacy_min_bw;
            if (BAST_MODE_IS_DTV(mode))
            {
               if ((mode == BAST_Mode_eDss_scan) || (mode == BAST_Mode_eDss_6_7))
                  baseTblIdx = BAST_acq_dtv_high_bw;
               else
                  baseTblIdx = BAST_acq_dtv_low_bw;
            }
            else if (BAST_MODE_IS_DCII(hChn->acqParams.mode))
            {
               if ((mode == BAST_Mode_eDcii_scan) || (mode >= BAST_Mode_eDcii_3_4))
                  baseTblIdx = BAST_acq_dcii_high_bw;
               else
                  baseTblIdx = BAST_acq_dcii_low_bw;
            }
            else
            {
               if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
                  baseTblIdx = BAST_acq_euro_dvb_low_bw;
               if ((mode == BAST_Mode_eDvb_scan) || (mode >= BAST_Mode_eDvb_3_4))
                  baseTblIdx++;
            }
         }
         else
         {
            if (BAST_MODE_IS_TURBO(mode))
            {
               baseTblIdx = BAST_acq_turbo_qpsk_low_bw;
               pScaleTable = acq_turbo_scale;
               pMinBwTable = acq_turbo_min_bw;
            }
#ifndef BAST_EXCLUDE_LDPC
            else
            {
               pScaleTable = acq_ldpc_scale;
               pMinBwTable = acq_ldpc_min_bw;
               baseTblIdx = BAST_acq_default_pilot_ldpc_qpsk_low_bw;

               if (mode == BAST_Mode_eLdpc_scan)
               {
                  if ((hChn->actualMode != BAST_Mode_eUnknown) && (hChn->actualMode != BAST_Mode_eLdpc_scan))
                     mode = hChn->actualMode;
               }

               if (BAST_g3_P_IsLdpcPilotOn_isrsafe(h) == 0)
                  baseTblIdx += 4;
               if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
                  baseTblIdx += 8;
               if (mode == BAST_Mode_eLdpc_scan)
                  baseTblIdx += 2; /* set to 8psk_low for ldpc scan */
               else if (mode >= BAST_Mode_eLdpc_8psk_8_9)
                  baseTblIdx += 3;
               else if (mode >= BAST_Mode_eLdpc_8psk_3_5)
                  baseTblIdx += 2;
               else if (mode >= BAST_Mode_eLdpc_Qpsk_3_4)
                  baseTblIdx++;
            }
#else
            else
            {
               /* should never get here since there is no AFEC core */
               BERR_TRACE(retCode = BERR_NOT_SUPPORTED);
               goto done;
            }
#endif
         }
      }
   }
   else
   {
      mode = hChn->actualMode;

      BAST_DEBUG_PLC(BDBG_MSG(("setting tracking PLC...")));
      /* tracking */
      if (hChn->plcCtl & BAST_G3_CONFIG_PLC_CTL_OVERRIDE_TRK)
      {
         bw = hChn->altPlcTrkBw;
         damp8 = hChn->altPlcTrkDamp;
         goto set_plc;
      }
      else
      {
         /* AST PI determines trk PLC */
         if (BAST_MODE_IS_TURBO(mode))
         {
            pScaleTable = trk_turbo_scale;
            pMinBwTable = trk_turbo_min_bw;
            baseTblIdx = BAST_trk_turbo_qpsk_low_bw;
         }
         else if (BAST_MODE_IS_LEGACY_QPSK(mode))
         {
            pScaleTable = trk_legacy_scale;
            pMinBwTable = trk_legacy_min_bw;
            if (mode >= BAST_Mode_eDcii_3_4)
               baseTblIdx = BAST_trk_dcii_high_bw;
            else if (mode >= BAST_Mode_eDcii_1_2)
               baseTblIdx = BAST_trk_dcii_low_bw;
            else if (mode >= BAST_Mode_eDss_6_7)
               baseTblIdx = BAST_trk_dtv_high_bw;
            else if (mode >= BAST_Mode_eDss_1_2)
               baseTblIdx = BAST_trk_dtv_low_bw;
            else
            {
               baseTblIdx = BAST_trk_default_dvb_low_bw;
               if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
                  baseTblIdx += 2;
               if (mode >= BAST_Mode_eDvb_3_4)
                  baseTblIdx++;
            }
         }
#ifndef BAST_EXCLUDE_LDPC
         else
         {
            pScaleTable = trk_ldpc_scale;
            pMinBwTable = trk_ldpc_min_bw;
            baseTblIdx = 36;
            if (BAST_g3_P_IsLdpcPilotOn_isrsafe(h) == 0)
               baseTblIdx += 6;
            if (hChn->plcCtl & BAST_G3_CONFIG_PLC_CTL_AWGN)
               baseTblIdx += 24;
            else if (h->pDevice->settings.networkSpec == BAST_NetworkSpec_eEuro)
               baseTblIdx += 12;
            if (mode >= BAST_Mode_eLdpc_8psk_8_9)
               baseTblIdx += 5; /* 8psk_high */
            else if (mode == BAST_Mode_eLdpc_8psk_5_6)
               baseTblIdx += 4; /* 8psk 5/6 */
            else if (mode == BAST_Mode_eLdpc_8psk_3_4)
               baseTblIdx += 3; /* 8psk 3/4 */
            else if (mode >= BAST_Mode_eLdpc_8psk_3_5)
               baseTblIdx += 2; /* 8psk_low */
            else if (mode >= BAST_Mode_eLdpc_Qpsk_3_4)
               baseTblIdx++;
         }
#endif
      }
   }

   if (BAST_MODE_IS_TURBO(hChn->actualMode))
   {
      if (hChn->actualMode >= BAST_Mode_eTurbo_8psk_4_5)
         baseTblIdx += 3; /* turbo_8psk_high */
      else if (hChn->actualMode >= BAST_Mode_eTurbo_8psk_2_3)
         baseTblIdx += 2; /* turbo_8psk_low */
      else if (hChn->actualMode >= BAST_Mode_eTurbo_Qpsk_3_4)
         baseTblIdx++; /* turbo_qpsk_high */
   }

   BAST_DEBUG_PLC(BDBG_MSG(("baseTblIdx = %d", baseTblIdx)));
   pPlcData = &plc_data[baseTblIdx*14];

   bDoInterp = false;
   if (hChn->acqParams.symbolRate <= plc_symbol_rate[0])
      symRateIdx = 0;
   else if (hChn->acqParams.symbolRate >= plc_symbol_rate[BAST_g3_PLC_NUM_SYMRATES-1])
      symRateIdx = BAST_g3_PLC_NUM_SYMRATES - 1;
   else
   {
      for (symRateIdx = 1; symRateIdx < BAST_g3_PLC_NUM_SYMRATES-1; symRateIdx++)
      {
         if (hChn->acqParams.symbolRate == plc_symbol_rate[symRateIdx])
            break;

         if (hChn->acqParams.symbolRate < plc_symbol_rate[symRateIdx])
         {
            bDoInterp = true;
            break;
         }
      }
   }

   BAST_DEBUG_PLC(BDBG_MSG(("symRateIdx = %d", symRateIdx)));

   BAST_CHK_RETCODE(BAST_g3_P_LookupPlcTable_isr(pPlcData, pScaleTable, pMinBwTable, symRateIdx, &bw, &damp8));
   if (bDoInterp)
   {
      /* do interpolation */
      bw_hi = bw;
      damp8_hi = damp8;
      symbol_rate_hi = plc_symbol_rate[symRateIdx];
      symRateIdx--;
      symbol_rate_lo = plc_symbol_rate[symRateIdx];

      BAST_CHK_RETCODE(BAST_g3_P_LookupPlcTable_isr(pPlcData, pScaleTable, pMinBwTable, symRateIdx, &bw_lo, &damp8_lo));

      if (bw_hi >= bw_lo)
         val = bw_hi - bw_lo;
      else
         val = bw_lo - bw_hi;
      BMTH_HILO_32TO64_Mul(val * 2, hChn->acqParams.symbolRate - symbol_rate_lo, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, symbol_rate_hi - symbol_rate_lo, &Q_hi, &Q_lo);
      Q_lo = (Q_lo + 1) >> 1;
      if (bw_hi >= bw_lo)
         bw = bw_lo + Q_lo;
      else
         bw = bw_lo - Q_lo;

      if (damp8_hi >= damp8_lo)
         val = damp8_hi - damp8_lo;
      else
         val = damp8_lo - damp8_hi;
      BMTH_HILO_32TO64_Mul(val * 2, hChn->acqParams.symbolRate - symbol_rate_lo, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, symbol_rate_hi - symbol_rate_lo, &Q_hi, &Q_lo);
      Q_lo = (Q_lo + 1) >> 1;
      if (damp8_hi >= damp8_lo)
         damp8 = damp8_lo + Q_lo;
      else
         damp8 = damp8_lo - Q_lo;
   }

   set_plc:
   retCode = BAST_g3_P_SetPlc_isr(h, bw, damp8);

   done:
   if (retCode == BERR_SUCCESS)
      hChn->bPlcTracking = (bAcq ? false : true);
   else
   {
      BDBG_ERR(("BAST_g3_P_ConfigPlc_isr() error 0x%X", retCode));
   }
   return retCode;
}


