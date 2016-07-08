/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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

 ******************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"

BDBG_MODULE(bsat_g1_priv_plc);

#define BSAT_DEBUG_PLC(x) /* x */


/******************************************************************************
 start of generated data
******************************************************************************/
/* generated 03-08-2016 10:46:51 */

#define BSAT_PLC_NUM_SYMRATES 7
#define BSAT_PLC_DAMP_SCALE 8

static const uint32_t BSAT_plc_symbol_rate[BSAT_PLC_NUM_SYMRATES] = {
	1000000,
	1875000,
	3750000,
	7500000,
	15000000,
	30000000,
	45000000
};

static const uint16_t BSAT_acq_legacy_scale[BSAT_PLC_NUM_SYMRATES] = {
	17,
	8,
	26,
	46,
	57,
	55,
	63
};

static const uint16_t BSAT_acq_afec_scale[BSAT_PLC_NUM_SYMRATES] = {
	136,
	137,
	146,
	221,
	516,
	950,
	1455
};

static const uint16_t BSAT_acq_tfec_scale[BSAT_PLC_NUM_SYMRATES] = {
	4,
	13,
	23,
	46,
	39,
	29,
	25
};

static const uint16_t BSAT_trk_legacy_scale[BSAT_PLC_NUM_SYMRATES] = {
	10,
	12,
	18,
	24,
	91,
	84,
	48
};

static const uint16_t BSAT_trk_afec_scale[BSAT_PLC_NUM_SYMRATES] = {
	143,
	143,
	150,
	225,
	315,
	638,
	929
};

static const uint16_t BSAT_trk_tfec_scale[BSAT_PLC_NUM_SYMRATES] = {
	4,
	5,
	15,
	24,
	24,
	28,
	24
};

static const uint32_t BSAT_acq_legacy_min_bw[BSAT_PLC_NUM_SYMRATES] = {
	3000,
	5500,
	5400,
	6500,
	7500,
	12000,
	14000
};

static const uint32_t BSAT_acq_afec_min_bw[BSAT_PLC_NUM_SYMRATES] = {
	3000,
	2800,
	5400,
	8900,
	13500,
	18000,
	19000
};

static const uint32_t BSAT_acq_tfec_min_bw[BSAT_PLC_NUM_SYMRATES] = {
	2600,
	4810,
	6240,
	8450,
	15080,
	21775,
	32662
};

static const uint32_t BSAT_trk_legacy_min_bw[BSAT_PLC_NUM_SYMRATES] = {
	3200,
	3000,
	3500,
	5000,
	6200,
	8100,
	9000
};

static const uint32_t BSAT_trk_afec_min_bw[BSAT_PLC_NUM_SYMRATES] = {
	1100,
	1100,
	1500,
	1800,
	2600,
	3100,
	6300
};

static const uint32_t BSAT_trk_tfec_min_bw[BSAT_PLC_NUM_SYMRATES] = {
	2600,
	4810,
	6240,
	7500,
	7500,
	9000,
	16000
};

static const uint8_t BSAT_plc_data[] = {
0x00,	0x26,	0x5C,	0x62,	0xA7,	0x7F,	0x9F,	0x10,	0x40,	0x48,	0x50,	0x58,	0x70,	0x80,	/* acq_default_dvb_low */
0x00,	0x00,	0x77,	0x78,	0xB8,	0xA4,	0xAF,	0x10,	0x40,	0x48,	0x50,	0x58,	0x70,	0x80,	/* acq_default_dvb_high */
0xF1,	0xE4,	0xEB,	0xAE,	0xED,	0xDA,	0xBE,	0x38,	0x40,	0x48,	0x50,	0x68,	0x70,	0x80,	/* acq_euro_dvb_low */
0xF7,	0xE4,	0xFE,	0xB9,	0xFE,	0xFF,	0xFE,	0x38,	0x40,	0x48,	0x50,	0x68,	0x70,	0x80,	/* acq_euro_dvb_high */
0x4C,	0x19,	0x00,	0x00,	0x00,	0x00,	0x00,	0x38,	0x38,	0x40,	0x48,	0x58,	0x78,	0x80,	/* acq_dtv_low */
0x5E,	0x32,	0x51,	0x2B,	0x3D,	0x12,	0x20,	0x38,	0x40,	0x48,	0x50,	0x64,	0x80,	0x88,	/* acq_dtv_high */
0x4C,	0xE4,	0xC4,	0xAE,	0xA7,	0x7F,	0xAF,	0x38,	0x40,	0x48,	0x50,	0x68,	0x70,	0x78,	/* acq_dcii_low */
0x5E,	0xE4,	0xC4,	0xFA,	0xCA,	0xA4,	0xCE,	0x38,	0x40,	0x48,	0x50,	0x68,	0x70,	0x78,	/* acq_dcii_high */
0x00,	0x16,	0x1C,	0x37,	0x1C,	0x04,	0x03,	0x10,	0x40,	0x48,	0x48,	0x48,	0x48,	0x60,	/* acq_default_pilot_afec_qpsk_low */
0x00,	0x19,	0x18,	0x29,	0x1C,	0x05,	0x05,	0x10,	0x40,	0x60,	0x70,	0x88,	0x98,	0xA8,	/* acq_default_pilot_afec_qpsk_high */
0x00,	0x16,	0x09,	0x17,	0x00,	0x00,	0x00,	0x10,	0x30,	0x40,	0x48,	0x48,	0x48,	0x50,	/* acq_default_pilot_afec_8psk_low */
0x00,	0x16,	0x09,	0x1C,	0x05,	0x05,	0x05,	0x10,	0x30,	0x40,	0x68,	0x68,	0x78,	0x80,	/* acq_default_pilot_afec_8psk_high */
0x00,	0xCE,	0xAF,	0x64,	0x22,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_pilot_afec_16apsk_low */
0x00,	0xFD,	0xDC,	0x81,	0x2F,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_pilot_afec_16apsk_high */
0x00,	0xCE,	0xAF,	0x64,	0x22,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_pilot_afec_32apsk_low */
0x00,	0xCE,	0xAF,	0x64,	0x22,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_pilot_afec_32apsk_high */
0x00,	0x52,	0x45,	0xA3,	0xFF,	0xFF,	0xFF,	0x10,	0x34,	0x3C,	0x34,	0x34,	0x30,	0x30,	/* acq_default_nopilot_afec_qpsk_low */
0x00,	0x52,	0x4F,	0xA3,	0xFF,	0xFF,	0xFF,	0x10,	0x34,	0x3C,	0x34,	0x34,	0x30,	0x30,	/* acq_default_nopilot_afec_qpsk_high */
0x00,	0x00,	0x00,	0x00,	0x09,	0x1A,	0x1E,	0x10,	0x88,	0x88,	0x88,	0x88,	0x88,	0x90,	/* acq_default_nopilot_afec_8psk_low */
0x00,	0x00,	0x00,	0x00,	0x09,	0x10,	0x1E,	0x10,	0x88,	0x88,	0x88,	0x88,	0x88,	0x90,	/* acq_default_nopilot_afec_8psk_high */
0x00,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_nopilot_afec_16apsk_low */
0x00,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_nopilot_afec_16apsk_high */
0x00,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_nopilot_afec_32apsk_low */
0x00,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_default_nopilot_afec_32apsk_high */
0x46,	0x76,	0xAF,	0x91,	0x2C,	0x15,	0x11,	0x38,	0x48,	0x50,	0x50,	0x50,	0x54,	0x70,	/* acq_euro_pilot_afec_qpsk_low */
0x3F,	0x68,	0x6B,	0x56,	0x31,	0x1A,	0x13,	0x40,	0x50,	0x60,	0x80,	0xA0,	0xB0,	0xC0,	/* acq_euro_pilot_afec_qpsk_high */
0x33,	0x52,	0x3B,	0x25,	0x14,	0x0E,	0x10,	0x40,	0x48,	0x50,	0x80,	0x80,	0x58,	0x80,	/* acq_euro_pilot_afec_8psk_low */
0x21,	0x47,	0x3B,	0x2E,	0x1C,	0x10,	0x13,	0x40,	0x48,	0x50,	0x80,	0x88,	0x88,	0x98,	/* acq_euro_pilot_afec_8psk_high */
0xCE,	0xCE,	0xAF,	0x64,	0x22,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_pilot_afec_16apsk_low */
0xFE,	0xFD,	0xDC,	0x81,	0x2F,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_pilot_afec_16apsk_high */
0xCE,	0xCE,	0xAF,	0x64,	0x22,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_pilot_afec_32apsk_low */
0xCE,	0xCE,	0xAF,	0x64,	0x22,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_pilot_afec_32apsk_high */
0x30,	0x6F,	0x5D,	0x69,	0x68,	0x3D,	0x45,	0x50,	0x50,	0x58,	0x60,	0x88,	0xA0,	0xA8,	/* acq_euro_nopilot_afec_qpsk_low */
0x37,	0x76,	0x7F,	0x7B,	0x6F,	0x2C,	0x4C,	0x48,	0x58,	0x58,	0x60,	0x70,	0xB0,	0xB8,	/* acq_euro_nopilot_afec_qpsk_high */
0x76,	0xD5,	0xFE,	0xFE,	0x75,	0x3C,	0x53,	0x60,	0x78,	0x80,	0x90,	0x88,	0xB8,	0xC8,	/* acq_euro_nopilot_afec_8psk_low */
0x6E,	0xF2,	0xDF,	0xDE,	0x7D,	0x42,	0x5A,	0x68,	0x58,	0x68,	0xA0,	0xB0,	0xC0,	0xF8,	/* acq_euro_nopilot_afec_8psk_high */
0xA2,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_nopilot_afec_16apsk_low */
0xA2,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_nopilot_afec_16apsk_high */
0xA2,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_nopilot_afec_32apsk_low */
0xA2,	0xA2,	0x86,	0x49,	0x16,	0x07,	0x04,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* acq_euro_nopilot_afec_32apsk_high */
0x32,	0x00,	0x00,	0x00,	0x2F,	0x00,	0x00,	0x38,	0x30,	0x30,	0x32,	0x46,	0x48,	0x50,	/* acq_tfec_qpsk_low */
0xFA,	0x16,	0x39,	0x0E,	0x00,	0x77,	0xCF,	0x40,	0x28,	0x2E,	0x30,	0x40,	0x3E,	0x46,	/* acq_tfec_qpsk_high */
0x00,	0x5C,	0xA3,	0xD0,	0xCB,	0xC7,	0xD6,	0x30,	0x28,	0x28,	0x30,	0x44,	0x48,	0x50,	/* acq_tfec_8psk_low */
0xE1,	0xF5,	0xFA,	0xFB,	0xFE,	0xF9,	0xFE,	0x30,	0x30,	0x30,	0x38,	0x40,	0x48,	0x50,	/* acq_tfec_8psk_high */
0x00,	0x00,	0x00,	0x2A,	0xFE,	0xFC,	0x15,	0x10,	0x28,	0x30,	0x48,	0x18,	0x18,	0x70,	/* trk_default_dvb_low */
0x00,	0x2A,	0x1C,	0x00,	0x00,	0x00,	0x00,	0x10,	0x30,	0x38,	0x40,	0x54,	0x70,	0x78,	/* trk_default_dvb_high */
0xC8,	0xFA,	0xC2,	0xA7,	0x56,	0x5E,	0xBC,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x70,	/* trk_euro_dvb_low */
0xE6,	0xFA,	0xFA,	0xFA,	0x61,	0x76,	0xFA,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x70,	/* trk_euro_dvb_high */
0x00,	0x00,	0x00,	0x2A,	0xFE,	0xFC,	0x15,	0x28,	0x28,	0x30,	0x48,	0x18,	0x18,	0x70,	/* trk_dtv_low */
0x00,	0x2A,	0x1C,	0x00,	0x00,	0x00,	0x00,	0x28,	0x30,	0x38,	0x40,	0x54,	0x70,	0x78,	/* trk_dtv_high */
0x1E,	0x7D,	0x96,	0x7D,	0x2A,	0x3A,	0x7D,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x68,	/* trk_dcii_low */
0x1E,	0x7D,	0x96,	0xD0,	0x40,	0x46,	0x92,	0x28,	0x30,	0x38,	0x40,	0x58,	0x60,	0x68,	/* trk_dcii_high */
0x0D,	0x16,	0x25,	0x3F,	0x2B,	0x16,	0x0C,	0x10,	0x30,	0x38,	0x38,	0x38,	0x38,	0x50,	/* trk_default_pilot_afec_qpsk_low */
0x0D,	0x17,	0x21,	0x29,	0x2B,	0x17,	0x0E,	0x10,	0x34,	0x50,	0x60,	0x78,	0x88,	0x98,	/* trk_default_pilot_afec_qpsk_high */
0x0D,	0x06,	0x07,	0x05,	0x03,	0x04,	0x05,	0x10,	0x28,	0x30,	0x3C,	0x3C,	0x3C,	0x3C,	/* trk_default_pilot_afec_8psk_low */
0x0D,	0x14,	0x17,	0x20,	0x17,	0x14,	0x0C,	0x10,	0x1C,	0x20,	0x20,	0x28,	0x30,	0x38,	/* trk_default_pilot_afec_8psk_3_4 */
0x0D,	0x0F,	0x0F,	0x0C,	0x09,	0x11,	0x0A,	0x10,	0x40,	0x48,	0x50,	0x5C,	0x60,	0x60,	/* trk_default_pilot_afec_8psk_5_6 */
0x0D,	0x1B,	0x1E,	0x20,	0x1B,	0x14,	0x0D,	0x10,	0x30,	0x40,	0x5C,	0x60,	0x68,	0x70,	/* trk_default_pilot_afec_8psk_high */
0x0D,	0xD1,	0xC5,	0x82,	0x5A,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_default_pilot_afec_16apsk_low */
0x0D,	0xFF,	0xF0,	0x9F,	0x6F,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_default_pilot_afec_16apsk_high */
0x0D,	0xD1,	0xC5,	0x82,	0x5A,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_default_pilot_afec_32apsk_low */
0x0D,	0xD1,	0xC5,	0x82,	0x5A,	0x22,	0x14,	0x10,	0x28,	0x28,	0x28,	0x10,	0x10,	0x10,	/* trk_default_pilot_afec_32apsk_high */
0x0D,	0x37,	0x6E,	0xA4,	0xFF,	0xFF,	0xFF,	0x10,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_default_nopilot_afec_qpsk_low */
0x0D,	0x37,	0x6E,	0xA4,	0xFF,	0xFF,	0xFF,	0x10,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_default_nopilot_afec_qpsk_high */
0x0D,	0x03,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x10,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x7A,	/* trk_default_nopilot_afec_8psk_low */
0x0D,	0x03,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x10,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_default_nopilot_afec_8psk_3_4 */
0x0D,	0x03,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x10,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_default_nopilot_afec_8psk_5_6 */
0x0D,	0x03,	0x0A,	0x10,	0x1D,	0x1F,	0x1F,	0x10,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_default_nopilot_afec_8psk_high */
0x0D,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_default_nopilot_afec_16apsk_low */
0x0D,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_default_nopilot_afec_16apsk_high */
0x0D,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_default_nopilot_afec_32apsk_low */
0x0D,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_default_nopilot_afec_32apsk_high */
0x34,	0x5A,	0x85,	0x63,	0x44,	0x23,	0x15,	0x28,	0x36,	0x3E,	0x43,	0x43,	0x43,	0x60,	/* trk_euro_pilot_afec_qpsk_low */
0x2D,	0x4E,	0x53,	0x41,	0x44,	0x25,	0x18,	0x30,	0x3C,	0x50,	0x73,	0x90,	0xA3,	0xB6,	/* trk_euro_pilot_afec_qpsk_high */
0x08,	0x0C,	0x0E,	0x0C,	0x08,	0x08,	0x0B,	0x30,	0x38,	0x40,	0x48,	0x48,	0x48,	0x48,	/* trk_euro_pilot_afec_8psk_low */
0x3D,	0x49,	0x49,	0x34,	0x27,	0x21,	0x15,	0x30,	0x38,	0x3D,	0x26,	0x30,	0x3A,	0x43,	/* trk_euro_pilot_afec_8psk_3_4 */
0x18,	0x1C,	0x1D,	0x16,	0x11,	0x1C,	0x12,	0x30,	0x38,	0x3F,	0x60,	0x6E,	0x73,	0x73,	/* trk_euro_pilot_afec_8psk_5_6 */
0x26,	0x34,	0x43,	0x34,	0x2C,	0x21,	0x16,	0x30,	0x36,	0x3C,	0x6E,	0x73,	0x7D,	0x86,	/* trk_euro_pilot_afec_8psk_high */
0xD1,	0xD1,	0xC5,	0x82,	0x5A,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_pilot_afec_16apsk_low */
0xFF,	0xFF,	0xF0,	0x9F,	0x6F,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_pilot_afec_16apsk_high */
0xD1,	0xD1,	0xC5,	0x82,	0x5A,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_pilot_afec_32apsk_low */
0xD1,	0xD1,	0xC5,	0x82,	0x5A,	0x32,	0x1F,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_pilot_afec_32apsk_high */
0x2C,	0x46,	0x65,	0x67,	0x96,	0x59,	0x5A,	0x40,	0x44,	0x48,	0x50,	0x78,	0x90,	0x98,	/* trk_euro_nopilot_afec_qpsk_low */
0x37,	0x57,	0x7D,	0x80,	0xB6,	0x50,	0x50,	0x38,	0x40,	0x44,	0x50,	0x60,	0xA0,	0xA8,	/* trk_euro_nopilot_afec_qpsk_high */
0x45,	0x6C,	0x9B,	0x9C,	0x57,	0x4D,	0x4D,	0x40,	0x48,	0x54,	0x74,	0x68,	0x80,	0x88,	/* trk_euro_nopilot_afec_8psk_low */
0x73,	0xB2,	0xFD,	0xFE,	0xB6,	0x67,	0x68,	0x50,	0x60,	0x74,	0x84,	0xB0,	0xD0,	0xF8,	/* trk_euro_nopilot_afec_8psk_3_4 */
0x5C,	0x8E,	0xCB,	0xCD,	0xD1,	0x72,	0x74,	0x58,	0x68,	0x84,	0x96,	0xC0,	0xE0,	0xE8,	/* trk_euro_nopilot_afec_8psk_5_6 */
0x65,	0x9B,	0xDD,	0xDF,	0xD6,	0x6F,	0x71,	0x40,	0x4C,	0x5C,	0x7C,	0xB8,	0xB0,	0xB8,	/* trk_euro_nopilot_afec_8psk_high */
0x26,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_nopilot_afec_16apsk_low */
0x26,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_nopilot_afec_16apsk_high */
0x26,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_nopilot_afec_32apsk_low */
0x26,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_euro_nopilot_afec_32apsk_high */
0x06,	0x0C,	0x0F,	0x19,	0x13,	0x0B,	0x06,	0x28,	0x30,	0x38,	0x38,	0x38,	0x48,	0x50,	/* trk_awgn_pilot_afec_qpsk_low */
0x07,	0x0E,	0x0D,	0x14,	0x13,	0x0B,	0x07,	0x30,	0x34,	0x50,	0x60,	0x78,	0x3E,	0x98,	/* trk_awgn_pilot_afec_qpsk_high */
0x00,	0x01,	0x01,	0x00,	0x00,	0x00,	0x00,	0x20,	0x28,	0x30,	0x3C,	0x3C,	0x3C,	0x3C,	/* trk_awgn_pilot_afec_8psk_low */
0x03,	0x05,	0x05,	0x06,	0x06,	0x08,	0x05,	0x18,	0x1C,	0x20,	0x20,	0x28,	0x30,	0x38,	/* trk_awgn_pilot_afec_8psk_3_4 */
0x03,	0x06,	0x04,	0x04,	0x03,	0x05,	0x07,	0x38,	0x40,	0x48,	0x50,	0x60,	0x60,	0x60,	/* trk_awgn_pilot_afec_8psk_5_6 */
0x06,	0x0A,	0x0F,	0x0D,	0x0A,	0x06,	0x06,	0x28,	0x30,	0x40,	0x5C,	0x60,	0x68,	0x70,	/* trk_awgn_pilot_afec_8psk_high */
0xD1,	0xD1,	0xC5,	0x82,	0x5A,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_pilot_afec_16apsk_low */
0xFF,	0xFF,	0xF0,	0x9F,	0x6F,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_pilot_afec_16apsk_high */
0xD1,	0xD1,	0xC5,	0x82,	0x5A,	0x22,	0x14,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_pilot_afec_32apsk_low */
0xD1,	0xD1,	0xC5,	0x82,	0x5A,	0x32,	0x1F,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_pilot_afec_32apsk_high */
0x07,	0x16,	0x32,	0x4C,	0x7A,	0x79,	0x7A,	0x20,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_awgn_nopilot_afec_qpsk_low */
0x08,	0x18,	0x32,	0x4C,	0x7A,	0x79,	0x7A,	0x20,	0x22,	0x22,	0x22,	0x22,	0x22,	0x22,	/* trk_awgn_nopilot_afec_qpsk_high */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x7A,	/* trk_awgn_nopilot_afec_8psk_low */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_awgn_nopilot_afec_8psk_3_4 */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_awgn_nopilot_afec_8psk_5_6 */
0x00,	0x00,	0x00,	0x04,	0x09,	0x0D,	0x0C,	0x78,	0x79,	0x79,	0x7A,	0x7A,	0x7A,	0x82,	/* trk_awgn_nopilot_afec_8psk_high */
0x26,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_nopilot_afec_16apsk_low */
0x26,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_nopilot_afec_16apsk_high */
0x26,	0x26,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_nopilot_afec_32apsk_low */
0x26,	0x00,	0x21,	0x15,	0x0C,	0x05,	0x00,	0x10,	0x79,	0x10,	0x10,	0x10,	0x10,	0x10,	/* trk_awgn_nopilot_afec_32apsk_high */
0x32,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x38,	0x30,	0x30,	0x28,	0x28,	0x30,	0x38,	/* trk_tfec_qpsk_low */
0xFA,	0x3A,	0x57,	0x2A,	0x2A,	0x24,	0x53,	0x40,	0x28,	0x2E,	0x28,	0x28,	0x30,	0x38,	/* trk_tfec_qpsk_high */
0x00,	0xEE,	0xFB,	0xE5,	0xE5,	0xD6,	0xA7,	0x28,	0x28,	0x28,	0x20,	0x20,	0x28,	0x38,	/* trk_tfec_8psk_low */
0x00,	0xEE,	0xFB,	0xFA,	0xFA,	0xFA,	0xFA,	0x28,	0x28,	0x28,	0x20,	0x20,	0x28,	0x38,	/* trk_tfec_8psk_high */
};

enum {
   BSAT_acq_default_dvb_low_plc,
   BSAT_acq_default_dvb_high_plc,
   BSAT_acq_euro_dvb_low_plc,
   BSAT_acq_euro_dvb_high_plc,
   BSAT_acq_dtv_low_plc,
   BSAT_acq_dtv_high_plc,
   BSAT_acq_dcii_low_plc,
   BSAT_acq_dcii_high_plc,
   BSAT_acq_default_pilot_afec_qpsk_low_plc,
   BSAT_acq_default_pilot_afec_qpsk_high_plc,
   BSAT_acq_default_pilot_afec_8psk_low_plc,
   BSAT_acq_default_pilot_afec_8psk_high_plc,
   BSAT_acq_default_pilot_afec_16apsk_low_plc,
   BSAT_acq_default_pilot_afec_16apsk_high_plc,
   BSAT_acq_default_pilot_afec_32apsk_low_plc,
   BSAT_acq_default_pilot_afec_32apsk_high_plc,
   BSAT_acq_default_nopilot_afec_qpsk_low_plc,
   BSAT_acq_default_nopilot_afec_qpsk_high_plc,
   BSAT_acq_default_nopilot_afec_8psk_low_plc,
   BSAT_acq_default_nopilot_afec_8psk_high_plc,
   BSAT_acq_default_nopilot_afec_16apsk_low_plc,
   BSAT_acq_default_nopilot_afec_16apsk_high_plc,
   BSAT_acq_default_nopilot_afec_32apsk_low_plc,
   BSAT_acq_default_nopilot_afec_32apsk_high_plc,
   BSAT_acq_euro_pilot_afec_qpsk_low_plc,
   BSAT_acq_euro_pilot_afec_qpsk_high_plc,
   BSAT_acq_euro_pilot_afec_8psk_low_plc,
   BSAT_acq_euro_pilot_afec_8psk_high_plc,
   BSAT_acq_euro_pilot_afec_16apsk_low_plc,
   BSAT_acq_euro_pilot_afec_16apsk_high_plc,
   BSAT_acq_euro_pilot_afec_32apsk_low_plc,
   BSAT_acq_euro_pilot_afec_32apsk_high_plc,
   BSAT_acq_euro_nopilot_afec_qpsk_low_plc,
   BSAT_acq_euro_nopilot_afec_qpsk_high_plc,
   BSAT_acq_euro_nopilot_afec_8psk_low_plc,
   BSAT_acq_euro_nopilot_afec_8psk_high_plc,
   BSAT_acq_euro_nopilot_afec_16apsk_low_plc,
   BSAT_acq_euro_nopilot_afec_16apsk_high_plc,
   BSAT_acq_euro_nopilot_afec_32apsk_low_plc,
   BSAT_acq_euro_nopilot_afec_32apsk_high_plc,
   BSAT_acq_tfec_qpsk_low_plc,
   BSAT_acq_tfec_qpsk_high_plc,
   BSAT_acq_tfec_8psk_low_plc,
   BSAT_acq_tfec_8psk_high_plc,
   BSAT_trk_default_dvb_low_plc,
   BSAT_trk_default_dvb_high_plc,
   BSAT_trk_euro_dvb_low_plc,
   BSAT_trk_euro_dvb_high_plc,
   BSAT_trk_dtv_low_plc,
   BSAT_trk_dtv_high_plc,
   BSAT_trk_dcii_low_plc,
   BSAT_trk_dcii_high_plc,
   BSAT_trk_default_pilot_afec_qpsk_low_plc,
   BSAT_trk_default_pilot_afec_qpsk_high_plc,
   BSAT_trk_default_pilot_afec_8psk_low_plc,
   BSAT_trk_default_pilot_afec_8psk_3_4_plc,
   BSAT_trk_default_pilot_afec_8psk_5_6_plc,
   BSAT_trk_default_pilot_afec_8psk_high_plc,
   BSAT_trk_default_pilot_afec_16apsk_low_plc,
   BSAT_trk_default_pilot_afec_16apsk_high_plc,
   BSAT_trk_default_pilot_afec_32apsk_low_plc,
   BSAT_trk_default_pilot_afec_32apsk_high_plc,
   BSAT_trk_default_nopilot_afec_qpsk_low_plc,
   BSAT_trk_default_nopilot_afec_qpsk_high_plc,
   BSAT_trk_default_nopilot_afec_8psk_low_plc,
   BSAT_trk_default_nopilot_afec_8psk_3_4_plc,
   BSAT_trk_default_nopilot_afec_8psk_5_6_plc,
   BSAT_trk_default_nopilot_afec_8psk_high_plc,
   BSAT_trk_default_nopilot_afec_16apsk_low_plc,
   BSAT_trk_default_nopilot_afec_16apsk_high_plc,
   BSAT_trk_default_nopilot_afec_32apsk_low_plc,
   BSAT_trk_default_nopilot_afec_32apsk_high_plc,
   BSAT_trk_euro_pilot_afec_qpsk_low_plc,
   BSAT_trk_euro_pilot_afec_qpsk_high_plc,
   BSAT_trk_euro_pilot_afec_8psk_low_plc,
   BSAT_trk_euro_pilot_afec_8psk_3_4_plc,
   BSAT_trk_euro_pilot_afec_8psk_5_6_plc,
   BSAT_trk_euro_pilot_afec_8psk_high_plc,
   BSAT_trk_euro_pilot_afec_16apsk_low_plc,
   BSAT_trk_euro_pilot_afec_16apsk_high_plc,
   BSAT_trk_euro_pilot_afec_32apsk_low_plc,
   BSAT_trk_euro_pilot_afec_32apsk_high_plc,
   BSAT_trk_euro_nopilot_afec_qpsk_low_plc,
   BSAT_trk_euro_nopilot_afec_qpsk_high_plc,
   BSAT_trk_euro_nopilot_afec_8psk_low_plc,
   BSAT_trk_euro_nopilot_afec_8psk_3_4_plc,
   BSAT_trk_euro_nopilot_afec_8psk_5_6_plc,
   BSAT_trk_euro_nopilot_afec_8psk_high_plc,
   BSAT_trk_euro_nopilot_afec_16apsk_low_plc,
   BSAT_trk_euro_nopilot_afec_16apsk_high_plc,
   BSAT_trk_euro_nopilot_afec_32apsk_low_plc,
   BSAT_trk_euro_nopilot_afec_32apsk_high_plc,
   BSAT_trk_awgn_pilot_afec_qpsk_low_plc,
   BSAT_trk_awgn_pilot_afec_qpsk_high_plc,
   BSAT_trk_awgn_pilot_afec_8psk_low_plc,
   BSAT_trk_awgn_pilot_afec_8psk_3_4_plc,
   BSAT_trk_awgn_pilot_afec_8psk_5_6_plc,
   BSAT_trk_awgn_pilot_afec_8psk_high_plc,
   BSAT_trk_awgn_pilot_afec_16apsk_low_plc,
   BSAT_trk_awgn_pilot_afec_16apsk_high_plc,
   BSAT_trk_awgn_pilot_afec_32apsk_low_plc,
   BSAT_trk_awgn_pilot_afec_32apsk_high_plc,
   BSAT_trk_awgn_nopilot_afec_qpsk_low_plc,
   BSAT_trk_awgn_nopilot_afec_qpsk_high_plc,
   BSAT_trk_awgn_nopilot_afec_8psk_low_plc,
   BSAT_trk_awgn_nopilot_afec_8psk_3_4_plc,
   BSAT_trk_awgn_nopilot_afec_8psk_5_6_plc,
   BSAT_trk_awgn_nopilot_afec_8psk_high_plc,
   BSAT_trk_awgn_nopilot_afec_16apsk_low_plc,
   BSAT_trk_awgn_nopilot_afec_16apsk_high_plc,
   BSAT_trk_awgn_nopilot_afec_32apsk_low_plc,
   BSAT_trk_awgn_nopilot_afec_32apsk_high_plc,
   BSAT_trk_tfec_qpsk_low_plc,
   BSAT_trk_tfec_qpsk_high_plc,
   BSAT_trk_tfec_8psk_low_plc,
   BSAT_trk_tfec_8psk_high_plc
};
/******************************************************************************
 end of generated data
******************************************************************************/


/******************************************************************************
 BSAT_g1_P_LookupPlcTable_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_LookupPlcTable_isr(const uint8_t *pBwTable, const uint16_t *pBwScaleTable, const uint32_t *pMinBwTable, uint32_t symRateIdx, uint32_t *pBw, uint32_t *pDamp8)
{
   BERR_Code retCode = BERR_SUCCESS;

   if ((pBwTable == NULL) || (pBwScaleTable == NULL) || (pMinBwTable == NULL))
   {
      BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
      goto done;
   }
   *pBw = ((uint32_t)pBwTable[symRateIdx] * (uint32_t)pBwScaleTable[symRateIdx]) + pMinBwTable[symRateIdx];
   *pDamp8 = pBwTable[BSAT_PLC_NUM_SYMRATES + symRateIdx];
   BSAT_DEBUG_PLC(BDBG_WRN(("BSAT_g1_P_LookupPlcTable_isr(): bw=%u, damp8=%d", *pBw, *pDamp8)));

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_SetPlc_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_SetPlc_isr(BSAT_ChannelHandle h, uint32_t bw, uint32_t damp)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i, j, b0, b1, b2, b3, val, P_hi, P_lo, Q_hi, Q_lo;
   bool ovr;

   BSAT_DEBUG_PLC(BDBG_WRN(("BSAT_g1_P_SetPlc_isr(): bw=%u, damp*8=%d", bw, damp)));

   /* compute 6-bit linear coefficient */
   for (i = 0; i < 7; i++)
   {
      b0 = (((1 << (i << 1)) * bw) >> 2) * damp;
      if (b0 > hChn->acqSettings.symbolRate)
         break;
   }
   i--;
   BMTH_HILO_32TO64_Mul((1 << (i << 1)) * damp, bw << 6, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);
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
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);

      if (ovr)
         Q_lo *= bw;
      Q_lo = (Q_lo + 1) >> 1; /* round */
      if (Q_lo > hChn->acqSettings.symbolRate)
         break;
   }
   j--;
   if (ovr)
      BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 5, bw << 7, &P_hi, &P_lo);
   else
      BMTH_HILO_32TO64_Mul((1 << (j << 1)) << 5, (bw * bw), &P_hi, &P_lo);

   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);
   if (ovr)
   {
      Q_lo = (Q_lo + 1) >> 1;
      BMTH_HILO_32TO64_Mul(Q_lo, bw << 2, &P_hi, &P_lo);
   }
   else
      BMTH_HILO_32TO64_Mul(Q_lo, 1 << 8, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqSettings.symbolRate, &Q_hi, &Q_lo);
   b2 = (Q_lo + 128) >> 8;
   if (b2 & 0xFFFFFFE0)
      b2 = 0x1F;
   b3 = 13 - j;

   val = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
   BSAT_DEBUG_PLC(BDBG_WRN(("PLC=0x%08X", val)));
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLC, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLC1, val);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FBLC, val);
   return BERR_SUCCESS;
}


#ifdef BSAT_HAS_DVBS2X
/******************************************************************************
 BSAT_g1_P_GetAcqPlcDvbs2x() - used for DVB-S2X and ACM modes
******************************************************************************/
BERR_Code BSAT_g1_P_GetAcqPlcDvbs2x(BSAT_ChannelHandle h, uint32_t *pBw, uint32_t *pDamp8)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_Mode mode = hChn->acqSettings.mode;
   uint32_t Fb = hChn->acqSettings.symbolRate;
   bool bPilot = BSAT_g1_P_AfecIsPilot_isr(h);

   *pDamp8 = 2<<3;
   if (Fb < 2000000)
      *pBw = 3000;
   else
   {
      *pBw = 25000;
      if (bPilot && (Fb <= 20000000))
         *pBw = 31000;
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetTrkPlcDvbs2x() - used for DVB-S2X and ACM modes
******************************************************************************/
BERR_Code BSAT_g1_P_GetTrkPlcDvbs2x(BSAT_ChannelHandle h, uint32_t *pBw, uint32_t *pDamp8)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_Mode mode = hChn->acqSettings.mode;
   uint32_t Fb = hChn->acqSettings.symbolRate;
   bool bPilot = BSAT_g1_P_AfecIsPilot_isr(h);

   *pDamp8 = 2<<3;

   if (Fb < 2000000)
      *pBw = 3000;
   else
   {
      *pBw = 25000;

      if (!bPilot)
      {
         if ((mode == BSAT_Mode_eDvbs2x_16apsk_26_45) || (mode == BSAT_Mode_eDvbs2x_16apsk_3_5))
            *pBw = 10000;
         else if ((mode == BSAT_Mode_eDvbs2x_16apsk_28_45) ||
                  (mode == BSAT_Mode_eDvbs2x_16apsk_23_36) ||
                  (mode == BSAT_Mode_eDvbs2x_16apsk_25_36))
            *pBw = 7500;
      }
      else
      {
         /* pilot */
         if (Fb <= 20000000)
            *pBw = 31000;
      }
   }
   return BERR_SUCCESS;
}
#endif


/******************************************************************************
 BSAT_g1_P_ConfigPlc_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_ConfigPlc_isr(BSAT_ChannelHandle h, bool bAcq)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, symbol_rate_hi, symbol_rate_lo, bw, bw_hi, bw_lo, damp8, damp8_hi, damp8_lo, baseTblIdx, symRateIdx, P_hi, P_lo, Q_hi, Q_lo;
   const uint32_t *pMinBwTable;
   const uint16_t *pScaleTable;
   const uint8_t *pPlcData;
   bool bDoInterp, bPilot = false;
   BERR_Code retCode;
   BSAT_Mode mode;
   BSAT_NetworkSpec networkSpec;

   BSAT_g1_P_GetNetworkSpec_isrsafe(h->pDevice, &networkSpec);

   baseTblIdx = 0;  /* baseTblIdx=index into plc_tables[] */
   pScaleTable = NULL;
   pMinBwTable = NULL;

   if (bAcq)
   {
      BSAT_DEBUG_PLC(BDBG_WRN(("setting acquisition PLC...")));

      mode = hChn->acqSettings.mode;
      if (hChn->configParam[BSAT_g1_CONFIG_PLC_CTL] & BSAT_g1_CONFIG_PLC_CTL_OVERRIDE_ACQ)
      {
         bw = hChn->configParam[BSAT_g1_CONFIG_PLC_ALT_ACQ_BW];
         damp8 = hChn->configParam[BSAT_g1_CONFIG_PLC_ALT_ACQ_DAMP];
         goto set_plc;
      }
      else
      {
#ifdef BSAT_HAS_DVBS2X
         if (BSAT_MODE_IS_DVBS2X(mode) || (mode == BSAT_Mode_eDvbs2_ACM))
         {
            if ((retCode = BSAT_g1_P_GetAcqPlcDvbs2x(h, &bw, &damp8)) != BERR_SUCCESS)
               return retCode;
            goto set_plc;
         }
#endif

         /* SAT PI determines acquisition PLC */
         if (BSAT_MODE_IS_LEGACY_QPSK(mode))
         {
            pScaleTable = BSAT_acq_legacy_scale;
            pMinBwTable = BSAT_acq_legacy_min_bw;
            if (BSAT_MODE_IS_DTV(mode))
            {
               /* DTV */
               if ((mode == BSAT_Mode_eDss_scan) || (mode == BSAT_Mode_eDss_6_7))
                  baseTblIdx = BSAT_acq_dtv_high_plc;
               else
                  baseTblIdx = BSAT_acq_dtv_low_plc;
            }
            else if (BSAT_MODE_IS_DCII(hChn->acqSettings.mode))
            {
               /* DCII */
               if ((mode == BSAT_Mode_eDcii_scan) || (mode >= BSAT_Mode_eDcii_3_4))
                  baseTblIdx = BSAT_acq_dcii_high_plc;
               else
                  baseTblIdx = BSAT_acq_dcii_low_plc;
            }
            else
            {
               /* DVB-S */
               if (networkSpec == BSAT_NetworkSpec_eEuro)
                  baseTblIdx = BSAT_acq_euro_dvb_low_plc;
               if ((mode == BSAT_Mode_eDvbs_scan) || (mode >= BSAT_Mode_eDvbs_3_4))
                  baseTblIdx++;
            }
         }
         else
         {
#ifndef BSAT_EXCLUDE_TFEC
            if (BSAT_MODE_IS_TURBO(mode))
            {
               baseTblIdx = BSAT_acq_tfec_qpsk_low_plc;
               pScaleTable = BSAT_acq_tfec_scale;
               pMinBwTable = BSAT_acq_tfec_min_bw;
            }
#endif /* BSAT_EXCLUDE_TFEC */
#ifndef BSAT_EXCLUDE_AFEC
            if (BSAT_MODE_IS_DVBS2(mode))
            {
               pScaleTable = BSAT_acq_afec_scale;
               pMinBwTable = BSAT_acq_afec_min_bw;
               baseTblIdx = BSAT_acq_default_pilot_afec_qpsk_low_plc;

               if (mode == BSAT_Mode_eDvbs2_scan)
               {
                  if ((hChn->actualMode != BSAT_Mode_eUnknown) && (hChn->actualMode != BSAT_Mode_eDvbs2_scan))
                     mode = hChn->actualMode;
               }

               bPilot = BSAT_g1_P_AfecIsPilot_isr(h);
               if (!bPilot)
                  baseTblIdx += 8;
               if (networkSpec == BSAT_NetworkSpec_eEuro)
                  baseTblIdx += 16;
               if (mode == BSAT_Mode_eDvbs2_scan)
                  baseTblIdx += 2; /* set to 8psk_low for dvbs2 scan */
               else if (mode >= BSAT_Mode_eDvbs2_32apsk_4_5)
                  baseTblIdx += 7;
               else if (mode >= BSAT_Mode_eDvbs2_32apsk_3_4)
                  baseTblIdx += 6;
               else if (mode >= BSAT_Mode_eDvbs2_16apsk_4_5)
                  baseTblIdx += 5;
               else if (mode >= BSAT_Mode_eDvbs2_16apsk_2_3)
                  baseTblIdx += 4;
               else if (mode >= BSAT_Mode_eDvbs2_8psk_8_9)
                  baseTblIdx += 3;
               else if (mode >= BSAT_Mode_eDvbs2_8psk_3_5)
                  baseTblIdx += 2;
               else if (mode >= BSAT_Mode_eDvbs2_Qpsk_3_4)
                  baseTblIdx++;
            }
#endif /* BSAT_EXCLUDE_AFEC */
            if ((pScaleTable == NULL) || (pMinBwTable == NULL))
            {
               /* should never get here */
               BDBG_ASSERT(0);
            }
         }
      }
   }
   else
   {
      mode = hChn->actualMode;

      BSAT_DEBUG_PLC(BDBG_WRN(("setting tracking PLC...")));
      /* tracking */
      if (hChn->configParam[BSAT_g1_CONFIG_PLC_CTL] & BSAT_g1_CONFIG_PLC_CTL_OVERRIDE_TRK)
      {
         bw = hChn->configParam[BSAT_g1_CONFIG_PLC_ALT_TRK_BW];
         damp8 = hChn->configParam[BSAT_g1_CONFIG_PLC_ALT_TRK_DAMP];
         goto set_plc;
      }
      else
      {
         /* SAT PI determines trk PLC */
#ifdef BSAT_HAS_DVBS2X
         if (BSAT_MODE_IS_DVBS2X(mode))
         {
            if ((retCode = BSAT_g1_P_GetTrkPlcDvbs2x(h, &bw, &damp8)) != BERR_SUCCESS)
               return retCode;
            goto set_plc;
         }
#endif

         if (BSAT_MODE_IS_LEGACY_QPSK(mode))
         {
            pScaleTable = BSAT_trk_legacy_scale;
            pMinBwTable = BSAT_trk_legacy_min_bw;
            if (mode >= BSAT_Mode_eDcii_3_4)
               baseTblIdx = BSAT_trk_dcii_high_plc;
            else if (mode >= BSAT_Mode_eDcii_1_2)
               baseTblIdx = BSAT_trk_dcii_low_plc;
            else if (mode >= BSAT_Mode_eDss_6_7)
               baseTblIdx = BSAT_trk_dtv_high_plc;
            else if (mode >= BSAT_Mode_eDss_1_2)
               baseTblIdx = BSAT_trk_dtv_low_plc;
            else
            {
               baseTblIdx = BSAT_trk_default_dvb_low_plc;
               if (networkSpec == BSAT_NetworkSpec_eEuro)
                  baseTblIdx += 2;
               if (mode >= BSAT_Mode_eDvbs_3_4)
                  baseTblIdx++;
            }
         }
         else
         {
#ifndef BSAT_EXCLUDE_TFEC
            if (BSAT_MODE_IS_TURBO(mode))
            {
               pScaleTable = BSAT_trk_tfec_scale;
               pMinBwTable = BSAT_trk_tfec_min_bw;
               baseTblIdx = BSAT_trk_tfec_qpsk_low_plc;
            }
#endif /* BSAT_EXCLUDE_TFEC */
#ifndef BSAT_EXCLUDE_AFEC
            if (BSAT_MODE_IS_DVBS2(mode))
            {
               pScaleTable = BSAT_trk_afec_scale;
               pMinBwTable = BSAT_trk_afec_min_bw;
               baseTblIdx = BSAT_trk_default_pilot_afec_qpsk_low_plc;
               bPilot = BSAT_g1_P_AfecIsPilot_isr(h);
               if (!bPilot)
                  baseTblIdx += 10;
               if (hChn->configParam[BSAT_g1_CONFIG_PLC_CTL] & BSAT_g1_CONFIG_PLC_CTL_AWGN)
                  baseTblIdx += 40;
               else if (networkSpec == BSAT_NetworkSpec_eEuro)
                  baseTblIdx += 20;
               if (mode >= BSAT_Mode_eDvbs2_32apsk_4_5)
                  baseTblIdx += 9;
               else if (mode >= BSAT_Mode_eDvbs2_32apsk_3_4)
                  baseTblIdx += 8;
               else if (mode >= BSAT_Mode_eDvbs2_16apsk_4_5)
                  baseTblIdx += 7;
               else if (mode >= BSAT_Mode_eDvbs2_16apsk_2_3)
                  baseTblIdx += 6;
               else if (mode >= BSAT_Mode_eDvbs2_8psk_8_9)
                  baseTblIdx += 5; /* 8psk_high */
               else if (mode == BSAT_Mode_eDvbs2_8psk_5_6)
                  baseTblIdx += 4; /* 8psk 5/6 */
               else if (mode == BSAT_Mode_eDvbs2_8psk_3_4)
                  baseTblIdx += 3; /* 8psk 3/4 */
               else if (mode >= BSAT_Mode_eDvbs2_8psk_3_5)
                  baseTblIdx += 2; /* 8psk_low */
               else if (mode >= BSAT_Mode_eDvbs2_Qpsk_3_4)
                  baseTblIdx++;
            }
#endif /* BSAT_EXCLUDE_AFEC */
            if ((pScaleTable == NULL) || (pMinBwTable == NULL))
            {
               /* should never get here */
               BDBG_ASSERT(0);
            }
         }
      }
   }

#ifndef BSAT_EXCLUDE_TFEC
   if (BSAT_MODE_IS_TURBO(hChn->actualMode))
   {
      if (hChn->actualMode >= BSAT_Mode_eTurbo_8psk_4_5)
         baseTblIdx += 3; /* turbo_8psk_high */
      else if (hChn->actualMode >= BSAT_Mode_eTurbo_8psk_2_3)
         baseTblIdx += 2; /* turbo_8psk_low */
      else if (hChn->actualMode >= BSAT_Mode_eTurbo_Qpsk_3_4)
         baseTblIdx++; /* turbo_qpsk_high */
   }
#endif /* BSAT_EXCLUDE_TFEC */

   BSAT_DEBUG_PLC(BDBG_WRN(("baseTblIdx = %d", baseTblIdx)));
   pPlcData = &BSAT_plc_data[baseTblIdx*14];

   bDoInterp = false;
   if (hChn->acqSettings.symbolRate <= BSAT_plc_symbol_rate[0])
      symRateIdx = 0;
   else if (hChn->acqSettings.symbolRate >= BSAT_plc_symbol_rate[BSAT_PLC_NUM_SYMRATES-1])
      symRateIdx = BSAT_PLC_NUM_SYMRATES - 1;
   else
   {
      for (symRateIdx = 1; symRateIdx < BSAT_PLC_NUM_SYMRATES-1; symRateIdx++)
      {
         if (hChn->acqSettings.symbolRate == BSAT_plc_symbol_rate[symRateIdx])
            break;

         if (hChn->acqSettings.symbolRate < BSAT_plc_symbol_rate[symRateIdx])
         {
            bDoInterp = true;
            break;
         }
      }
   }

   BSAT_DEBUG_PLC(BDBG_WRN(("symRateIdx = %d", symRateIdx)));

   BSAT_CHK_RETCODE(BSAT_g1_P_LookupPlcTable_isr(pPlcData, pScaleTable, pMinBwTable, symRateIdx, &bw, &damp8));
   if (bDoInterp)
   {
      /* do interpolation */
      bw_hi = bw;
      damp8_hi = damp8;
      symbol_rate_hi = BSAT_plc_symbol_rate[symRateIdx];
      symRateIdx--;
      symbol_rate_lo = BSAT_plc_symbol_rate[symRateIdx];

      BSAT_CHK_RETCODE(BSAT_g1_P_LookupPlcTable_isr(pPlcData, pScaleTable, pMinBwTable, symRateIdx, &bw_lo, &damp8_lo));

      if (bw_hi >= bw_lo)
         val = bw_hi - bw_lo;
      else
         val = bw_lo - bw_hi;
      BMTH_HILO_32TO64_Mul(val * 2, hChn->acqSettings.symbolRate - symbol_rate_lo, &P_hi, &P_lo);
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
      BMTH_HILO_32TO64_Mul(val * 2, hChn->acqSettings.symbolRate - symbol_rate_lo, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, symbol_rate_hi - symbol_rate_lo, &Q_hi, &Q_lo);
      Q_lo = (Q_lo + 1) >> 1;
      if (damp8_hi >= damp8_lo)
         damp8 = damp8_lo + Q_lo;
      else
         damp8 = damp8_lo - Q_lo;
   }

   set_plc:
   retCode = BSAT_g1_P_SetPlc_isr(h, bw, damp8);

   done:
   if (retCode == BERR_SUCCESS)
      hChn->bPlcTracking = (bAcq ? false : true);
   else
   {
      BDBG_ERR(("BSAT_g1_P_ConfigPlc_isr() error 0x%X", retCode));
   }
   return retCode;
}
