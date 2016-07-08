/*******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bchp.h"
const uint32_t BDSP_IMG_als_decode_array1[] = {
	0xa9fd0fff,
	0x07bffe47
};
const uint32_t BDSP_IMG_als_decode_header [2] = {sizeof(BDSP_IMG_als_decode_array1), 1};
const void * const BDSP_IMG_als_decode [2] = {BDSP_IMG_als_decode_header, BDSP_IMG_als_decode_array1};
const uint32_t BDSP_IMG_als_decode_inter_frame_array1[] = {
	0x00000000,
	0x0000007a,
	0x00000001,
	0x00000001,
	0x00000000,
	0x0000000b
};
const uint32_t BDSP_IMG_als_decode_inter_frame_header [2] = {sizeof(BDSP_IMG_als_decode_inter_frame_array1), 1};
const void * const BDSP_IMG_als_decode_inter_frame [2] = {BDSP_IMG_als_decode_inter_frame_header, BDSP_IMG_als_decode_inter_frame_array1};
const uint32_t BDSP_IMG_als_decode_tables_array1[] = {
	0xfff00020,
	0xfff00120,
	0xfff00320,
	0xfff00620,
	0xfff00a20,
	0xfff00f20,
	0xfff01520,
	0xfff01c20,
	0xfff02420,
	0xfff02d20,
	0xfff03720,
	0xfff04220,
	0xfff04e20,
	0xfff05b20,
	0xfff06920,
	0xfff07820,
	0xfff08820,
	0xfff09920,
	0xfff0ab20,
	0xfff0be20,
	0xfff0d220,
	0xfff0e720,
	0xfff0fd20,
	0xfff11420,
	0xfff12c20,
	0xfff14520,
	0xfff15f20,
	0xfff17a20,
	0xfff19620,
	0xfff1b320,
	0xfff1d120,
	0xfff1f020,
	0xfff21020,
	0xfff23120,
	0xfff25320,
	0xfff27620,
	0xfff29a20,
	0xfff2bf20,
	0xfff2e520,
	0xfff30c20,
	0xfff33420,
	0xfff35d20,
	0xfff38720,
	0xfff3b220,
	0xfff3de20,
	0xfff40b20,
	0xfff43920,
	0xfff46820,
	0xfff49820,
	0xfff4c920,
	0xfff4fb20,
	0xfff52e20,
	0xfff56220,
	0xfff59720,
	0xfff5cd20,
	0xfff60420,
	0xfff63c20,
	0xfff67520,
	0xfff6af20,
	0xfff6ea20,
	0xfff72620,
	0xfff76320,
	0xfff7a120,
	0xfff7e020,
	0xfff82020,
	0xfff86120,
	0xfff8a320,
	0xfff8e620,
	0xfff92a20,
	0xfff96f20,
	0xfff9b520,
	0xfff9fc20,
	0xfffa4420,
	0xfffa8d20,
	0xfffad720,
	0xfffb2220,
	0xfffb6e20,
	0xfffbbb20,
	0xfffc0920,
	0xfffc5820,
	0xfffca820,
	0xfffcf920,
	0xfffd4b20,
	0xfffd9e20,
	0xfffdf220,
	0xfffe4720,
	0xfffe9d20,
	0xfffef420,
	0xffff4c20,
	0xffffa520,
	0xffffff20,
	0x00005a20,
	0x0000b620,
	0x00011320,
	0x00017120,
	0x0001d020,
	0x00023020,
	0x00029120,
	0x0002f320,
	0x00035620,
	0x0003ba20,
	0x00041f20,
	0x00048520,
	0x0004ec20,
	0x00055420,
	0x0005bd20,
	0x00062720,
	0x00069220,
	0x0006fe20,
	0x00076b20,
	0x0007d920,
	0x00084820,
	0x0008b820,
	0x00092920,
	0x00099b20,
	0x000a0e20,
	0x000a8220,
	0x000af720,
	0x000b6d20,
	0x000be420,
	0x000c5c20,
	0x000cd520,
	0x000d4f20,
	0x000dca20,
	0x000e4620,
	0x000ec320,
	0x000f4120,
	0x000fc020,
	0xffffffcc,
	0xffffffe3,
	0xffffffe1,
	0x00000013,
	0xfffffff0,
	0x0000000c,
	0xfffffff9,
	0x00000009,
	0xfffffffb,
	0x00000006,
	0xfffffffc,
	0x00000003,
	0xfffffffd,
	0x00000003,
	0xfffffffe,
	0x00000003,
	0xffffffff,
	0x00000002,
	0xffffffff,
	0x00000002,
	0x00000004,
	0x00000005,
	0x00000004,
	0x00000004,
	0x00000004,
	0x00000003,
	0x00000003,
	0x00000003,
	0x00000003,
	0x00000003,
	0x00000003,
	0x00000003,
	0x00000002,
	0x00000002,
	0x00000002,
	0x00000002,
	0x00000002,
	0x00000002,
	0x00000002,
	0x00000002,
	0x00080000,
	0x00180010,
	0x00280020,
	0x00380030,
	0x00460040,
	0x0052004c,
	0x005c0058,
	0x00640060,
	0x00c000cc,
	0x00a600b3,
	0x008c0099,
	0x00730080,
	0x00590066,
	0x0040004c,
	0x00260033,
	0x000c0019,
	0xfff40000,
	0xffdaffe7,
	0xffc0ffcd,
	0xffa7ffb4,
	0xff8dff9a,
	0xff74ff80,
	0xff5aff67,
	0xff40ff4d
};
const uint32_t BDSP_IMG_als_decode_tables_header [2] = {sizeof(BDSP_IMG_als_decode_tables_array1), 1};
const void * const BDSP_IMG_als_decode_tables [2] = {BDSP_IMG_als_decode_tables_header, BDSP_IMG_als_decode_tables_array1};
/* End of File */
