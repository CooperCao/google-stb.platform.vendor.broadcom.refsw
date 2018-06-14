/******************************************************************************
 * Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#if NEXUS_HAS_ETBG
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_stc_channel.h"
#else
#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#endif
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_tsmf.h"
#include "nexus_etbg.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "bmmt.h"

BDBG_MODULE(tsmf_mmt_encrypted_live);

/**
  * tsmf_mmt_encrypted_live app decodes and renders AV from a
  * live extended TSMF mpeg2ts stream (3 byte header + 185 bytes
  * TLV data per 188 byte packet)
  *
  * Broadcom extended TSMF streamer is used to feed the stream
  * to BCM7278-MTSIF interface
  *
  * tears_of_steel_encrypted_mmt_tlv_4band_extTSMF_120Mbps.mtsif
  * sample stream is needed to test this sample app.
  *
  **/


#define MAX_PACKAGES 8
#define NUM_BANDS_IN_GROUP  4
#define RELATIVE_TS_NUM 1
#define MAX_KEYS  37
#define KEY_SIZE 16




static uint8_t even_key_table[MAX_KEYS][KEY_SIZE] =
{
    { 0xB0, 0xA1, 0x31, 0x08, 0xBF, 0xFF, 0xDA, 0x0F, 0x58, 0x3B, 0x9F, 0x52, 0xA2, 0x5D, 0xC6, 0x64 },
    { 0x93, 0x8E, 0xDD, 0xD4, 0x64, 0xC8, 0x40, 0x60, 0xD8, 0x8B, 0x9B, 0xCB, 0x09, 0xCA, 0x19, 0x8B },
    { 0x62, 0xE0, 0xF8, 0xC5, 0x31, 0x0A, 0x90, 0xC8, 0x22, 0xEF, 0xF6, 0x49, 0x20, 0xF9, 0x98, 0xF3 },
    { 0x2C, 0x73, 0x11, 0x64, 0x5F, 0x87, 0x60, 0xE5, 0x67, 0xC3, 0xA3, 0x66, 0x5C, 0xC0, 0x27, 0x91 },
    { 0xCE, 0x04, 0x5A, 0x96, 0x6E, 0x16, 0x89, 0x3D, 0x89, 0xC0, 0xB5, 0x79, 0x82, 0x19, 0x9D, 0x0A },
    { 0x77, 0x5F, 0x4F, 0xF0, 0x26, 0x12, 0xFD, 0xCF, 0x86, 0x7B, 0x49, 0x75, 0x23, 0xB0, 0xB9, 0x20 },
    { 0x20, 0xD8, 0x9B, 0x31, 0xC7, 0x72, 0x82, 0x4B, 0xFA, 0xDC, 0x56, 0xFD, 0x68, 0x96, 0xEB, 0x47 },
    { 0x51, 0x6F, 0x46, 0xD2, 0xF0, 0x62, 0xBF, 0x5F, 0x0E, 0x08, 0x0B, 0x7B, 0x22, 0xAB, 0x58, 0xE1 },
    { 0x0D, 0x35, 0xBC, 0x01, 0xF4, 0xEF, 0x31, 0x6C, 0xA9, 0x6F, 0x18, 0xDB, 0x52, 0x81, 0xE6, 0xD3 },
    { 0xE2, 0x1E, 0xB7, 0xD7, 0xB1, 0x3E, 0xAD, 0x58, 0xF0, 0xF3, 0x02, 0xFA, 0x67, 0x4A, 0x0D, 0x83 },
    { 0xD5, 0xF5, 0x4F, 0x3B, 0x58, 0xFE, 0xC1, 0xF7, 0x04, 0xFD, 0x2A, 0x21, 0x8F, 0xFE, 0x21, 0xEA },
    { 0xDF, 0x76, 0x32, 0xD1, 0x07, 0x3C, 0x3C, 0x35, 0xE7, 0xC0, 0x4A, 0x3C, 0xEB, 0xD9, 0x5B, 0x72 },
    { 0xB9, 0x60, 0xA2, 0x16, 0xB7, 0xC5, 0x6D, 0x4C, 0x22, 0xBA, 0x22, 0x7A, 0x8F, 0x12, 0xEE, 0x5F },
    { 0x8F, 0x43, 0x3E, 0x40, 0xCD, 0xBD, 0xA5, 0xFF, 0x44, 0x53, 0x5E, 0x2A, 0x93, 0x56, 0xFE, 0xC4 },
    { 0x99, 0x8E, 0x6E, 0x88, 0x63, 0x1B, 0x6A, 0x02, 0x13, 0x49, 0x1F, 0x1A, 0xD5, 0x6F, 0xDC, 0xAF },
    { 0xDB, 0x85, 0x2A, 0x2F, 0x25, 0xDB, 0x3C, 0x91, 0x16, 0xD5, 0x05, 0x9A, 0x31, 0x7C, 0xD7, 0xDF },
    { 0xE6, 0x90, 0xA3, 0xEF, 0x6F, 0x2B, 0x85, 0xC8, 0x94, 0x76, 0x38, 0xC6, 0x0D, 0x65, 0xAA, 0x58 },
    { 0xAF, 0x88, 0x29, 0x67, 0xC1, 0xA2, 0x98, 0x0A, 0xE0, 0x96, 0x2E, 0x78, 0xBC, 0x46, 0xD7, 0x2B },
    { 0xFE, 0x11, 0x4C, 0xAC, 0xD9, 0xC8, 0x16, 0xF2, 0x3C, 0xF7, 0x11, 0x63, 0x09, 0x34, 0xFB, 0x4E },
    { 0x50, 0x6E, 0x45, 0x53, 0x9B, 0x5F, 0x6D, 0x41, 0x8A, 0xD8, 0x1E, 0x37, 0x46, 0x18, 0x39, 0x52 },
    { 0x96, 0x43, 0x04, 0x66, 0x14, 0x53, 0xDB, 0x0D, 0xCF, 0x20, 0xA9, 0xCC, 0xCC, 0x48, 0x79, 0x77 },
    { 0xF1, 0x7E, 0xBA, 0x3E, 0x20, 0xD6, 0x83, 0x4B, 0x90, 0xCA, 0x98, 0xC1, 0x99, 0x9B, 0xD3, 0x6B },
    { 0xEB, 0x54, 0x5B, 0xA0, 0x09, 0x7A, 0xEC, 0x91, 0xD2, 0x79, 0x8F, 0x4B, 0x1B, 0x2B, 0x95, 0x67 },
    { 0x19, 0x20, 0xCD, 0xE6, 0x22, 0xBA, 0x38, 0x2C, 0x04, 0xCA, 0x5C, 0x08, 0x4F, 0x75, 0x90, 0xE4 },
    { 0x57, 0x49, 0xCA, 0xEA, 0xBD, 0x34, 0x07, 0x89, 0x68, 0xF8, 0xE0, 0xF4, 0x42, 0xEA, 0x84, 0x7A },
    { 0x97, 0x6F, 0x25, 0x7F, 0x26, 0xF4, 0x49, 0x31, 0xA6, 0xB2, 0x5E, 0x3A, 0x96, 0x18, 0xBC, 0x7E },
    { 0x88, 0x6A, 0x43, 0x80, 0x34, 0x09, 0x8C, 0xC3, 0xBE, 0x9D, 0x6A, 0x52, 0x10, 0x1E, 0x77, 0xF1 },
    { 0x87, 0x3E, 0xA1, 0x2B, 0xCC, 0x94, 0xFF, 0x65, 0x66, 0x8F, 0x34, 0xDA, 0x69, 0x68, 0x44, 0x3D },
    { 0x85, 0xC1, 0x7A, 0x50, 0x28, 0xC2, 0xDF, 0x62, 0xF7, 0x61, 0x10, 0x7E, 0x7A, 0x42, 0x5D, 0xE5 },
    { 0x1D, 0xE8, 0x1A, 0xC0, 0xCF, 0x9F, 0x69, 0x57, 0x35, 0xB5, 0xC4, 0x04, 0x9C, 0x1F, 0x02, 0x92 },
    { 0x1F, 0x4F, 0xB6, 0x86, 0xE1, 0x9F, 0x24, 0x59, 0x6D, 0x43, 0x24, 0xC0, 0x8F, 0xE1, 0xBC, 0x70 },
    { 0xEC, 0x0E, 0xC3, 0x58, 0x3F, 0xFA, 0xC2, 0x74, 0x70, 0x03, 0x68, 0x01, 0x46, 0x78, 0x7B, 0x35 },
    { 0x74, 0x5E, 0x53, 0xC1, 0x04, 0xDF, 0x5B, 0x74, 0x52, 0xAD, 0x10, 0x8A, 0x6C, 0xD6, 0xE5, 0x59 },
    { 0xE6, 0x86, 0xD7, 0xDA, 0xCD, 0x27, 0xDA, 0x71, 0x51, 0xF2, 0xCE, 0x55, 0x24, 0x32, 0x9C, 0xED },
    { 0x68, 0xE9, 0xBC, 0x6B, 0x07, 0x77, 0x16, 0x13, 0x13, 0x23, 0x9A, 0xE7, 0xA3, 0x62, 0xB7, 0x8D },
    { 0x85, 0x08, 0xA8, 0x25, 0xA1, 0xA6, 0xE6, 0xD8, 0x68, 0x85, 0x5C, 0xE5, 0xE6, 0xFD, 0x3D, 0xE9 },
    { 0x78, 0x40, 0x7B, 0xB9, 0x2E, 0xA6, 0x3A, 0xC1, 0xF6, 0x41, 0xF3, 0xAC, 0xDB, 0x65, 0x8B, 0xBA }

};

static uint8_t odd_key_table[MAX_KEYS][KEY_SIZE] =
{
    { 0xE8, 0xBF, 0x99, 0xA2, 0xE7, 0x2A, 0x89, 0x14, 0x2E, 0x07, 0x5A, 0xFB, 0x0E, 0xA8, 0x28, 0xE7 },
    { 0x72, 0x03, 0x7C, 0x98, 0x1B, 0xC3, 0x88, 0xE2, 0xC9, 0x38, 0x9F, 0x69, 0x95, 0x73, 0xC5, 0xC8 },
    { 0xEA, 0xD6, 0x59, 0x7F, 0xE1, 0x75, 0x45, 0xA2, 0xFC, 0xAB, 0x50, 0xA7, 0xE6, 0xC4, 0x51, 0x6A },
    { 0x9B, 0x68, 0x9E, 0x13, 0x64, 0xB4, 0xA3, 0xB4, 0xAD, 0x7A, 0x5A, 0x54, 0x1B, 0x5D, 0x17, 0xB3 },
    { 0x27, 0x42, 0x89, 0x29, 0x6C, 0x1A, 0x6E, 0x49, 0xE6, 0x9F, 0x5C, 0xA8, 0x57, 0x04, 0x24, 0x2F },
    { 0xA0, 0x31, 0x6B, 0x85, 0x81, 0x2E, 0xE0, 0xB3, 0x3E, 0x9D, 0x79, 0x2E, 0xB0, 0x7F, 0x15, 0x08 },
    { 0xC1, 0x70, 0x08, 0x33, 0xCB, 0x2F, 0x59, 0x04, 0x4F, 0xFE, 0xC5, 0x9E, 0xE3, 0x6B, 0xD3, 0x52 },
    { 0x95, 0xB8, 0x58, 0x79, 0xCF, 0xDA, 0x87, 0xB1, 0x73, 0x01, 0xE3, 0x9C, 0x02, 0x83, 0xCD, 0x06 },
    { 0x6A, 0x64, 0xA3, 0xB8, 0x19, 0xF5, 0x30, 0x78, 0x68, 0x71, 0x41, 0x71, 0x67, 0x63, 0x07, 0x02 },
    { 0x58, 0x73, 0xF3, 0xCF, 0xB7, 0x59, 0x11, 0x85, 0x31, 0x79, 0x5C, 0xDC, 0x20, 0x59, 0x99, 0x64 },
    { 0x24, 0x61, 0xE0, 0xDB, 0xB5, 0x09, 0x87, 0x70, 0xCE, 0x7D, 0x07, 0x06, 0xC4, 0x5F, 0x6B, 0xD6 },
    { 0x46, 0x2E, 0x9B, 0x6F, 0x3C, 0x54, 0x04, 0xEE, 0x96, 0x60, 0x3B, 0x1E, 0xA2, 0x52, 0xFE, 0x51 },
    { 0x26, 0xD0, 0x0C, 0x85, 0x65, 0x51, 0x6F, 0xE3, 0x26, 0x14, 0xF3, 0x2A, 0x0F, 0x91, 0x0A, 0x30 },
    { 0x04, 0x9C, 0xB0, 0x6C, 0x42, 0xE8, 0x60, 0xD7, 0x3F, 0xF6, 0x69, 0x0C, 0x80, 0xB7, 0x8C, 0x36 },
    { 0xD9, 0x04, 0x7B, 0x02, 0x15, 0x19, 0xF1, 0x2A, 0x82, 0x73, 0xAA, 0x4A, 0x3B, 0x29, 0x60, 0x87 },
    { 0x0E, 0x33, 0x77, 0x2D, 0xD2, 0xB1, 0x31, 0x2F, 0x5E, 0x2A, 0x9F, 0xFB, 0x1C, 0x91, 0x99, 0xDF },
    { 0x3B, 0x0A, 0x8A, 0x61, 0x83, 0x3B, 0x0C, 0xAA, 0xF7, 0x11, 0xC3, 0x94, 0xBC, 0xF4, 0xDA, 0xAA },
    { 0x33, 0x13, 0xE5, 0x3D, 0x7C, 0xED, 0x67, 0xC1, 0x58, 0xBC, 0xFC, 0xF2, 0xED, 0xCF, 0x3F, 0x8D },
    { 0x01, 0x9B, 0x1E, 0x81, 0xC3, 0x85, 0x66, 0x69, 0x25, 0xC9, 0xBF, 0xB5, 0x0A, 0x32, 0x1B, 0x0C },
    { 0x2C, 0xAA, 0x55, 0x37, 0x08, 0x99, 0x45, 0xB2, 0xEE, 0x60, 0xD3, 0x88, 0xBA, 0xF3, 0x3A, 0xD0 },
    { 0xF7, 0xB9, 0x04, 0xA2, 0x82, 0x2A, 0x9C, 0xED, 0x78, 0x25, 0x51, 0xE6, 0x0D, 0x3B, 0x87, 0x99 },
    { 0x6F, 0xAC, 0xCE, 0x2E, 0xF7, 0x97, 0xBC, 0xD1, 0x1C, 0x72, 0x5C, 0x45, 0xDE, 0x6C, 0x71, 0xD5 },
    { 0x07, 0x63, 0x4E, 0x81, 0xE0, 0x43, 0xA3, 0xF7, 0xDB, 0x7C, 0xE0, 0x13, 0x48, 0x2D, 0x1D, 0xA4 },
    { 0x1A, 0xB9, 0x69, 0xC1, 0x80, 0x72, 0x0B, 0x9E, 0x11, 0x9C, 0xC6, 0x07, 0x46, 0x7B, 0x5F, 0x89 },
    { 0x80, 0x1F, 0x8A, 0x8C, 0xF9, 0x2A, 0xBF, 0xE2, 0xBD, 0xE9, 0x7D, 0x1C, 0xE7, 0x34, 0x97, 0x7C },
    { 0x4C, 0xF9, 0x8C, 0x58, 0x9A, 0xB5, 0x6A, 0x9F, 0x7A, 0x59, 0xEE, 0x1C, 0xA2, 0x28, 0xE4, 0x5B },
    { 0x6A, 0xAC, 0xBE, 0x77, 0xA3, 0x95, 0x08, 0x5A, 0xA0, 0x98, 0x62, 0x5D, 0x82, 0xE4, 0x8F, 0x17 },
    { 0x7B, 0x9A, 0x67, 0x09, 0x3D, 0x26, 0xB4, 0xCA, 0x2F, 0xEF, 0x6D, 0x17, 0xE1, 0x93, 0x26, 0x6F },
    { 0xDD, 0x26, 0x0C, 0x86, 0x11, 0xD1, 0x76, 0x7A, 0x04, 0x89, 0x99, 0xCB, 0x32, 0x1D, 0x15, 0x91 },
    { 0xEC, 0x27, 0xD0, 0x50, 0x77, 0x05, 0x51, 0x3C, 0x5D, 0x36, 0xE6, 0xFE, 0xD1, 0x16, 0xA7, 0x87 },
    { 0x8D, 0x0C, 0xBC, 0xAA, 0xF4, 0x84, 0xB2, 0x27, 0x75, 0x50, 0xC9, 0xC6, 0xB4, 0xDB, 0x05, 0x4B },
    { 0x67, 0x08, 0xE3, 0x50, 0x6A, 0xAD, 0x2A, 0xD7, 0x49, 0x19, 0xE5, 0xF9, 0x70, 0x86, 0x25, 0x22 },
    { 0x9B, 0x34, 0x27, 0x05, 0x1F, 0x8C, 0xEB, 0x1C, 0x1C, 0x90, 0x47, 0xD8, 0x55, 0x06, 0x5B, 0x15 },
    { 0x2C, 0x12, 0xFC, 0xBE, 0x4A, 0xEA, 0x0D, 0x4B, 0x0F, 0x03, 0xB2, 0xF0, 0x01, 0xFA, 0x38, 0x07 },
    { 0x5B, 0x78, 0x6A, 0x82, 0xB7, 0xC6, 0x0B, 0x60, 0x97, 0x21, 0x7C, 0xE0, 0x32, 0xA2, 0x18, 0x90 },
    { 0x3C, 0xC3, 0xF9, 0x76, 0x49, 0xF2, 0x59, 0x9B, 0xEC, 0x41, 0x20, 0xD4, 0x65, 0x75, 0xAC, 0x83 },
    { 0xB5, 0x38, 0xD8, 0x24, 0xE0, 0x11, 0x12, 0x6F, 0x32, 0x12, 0x22, 0xA0, 0x16, 0x13, 0x3C, 0x77 },
};

typedef struct band_config
{
   bool isPrimary;
   unsigned bitRate;
   unsigned numSubFrameSlots;
}band_config;

typedef struct input_band
{
    NEXUS_TsmfHandle tsmf;
    NEXUS_FrontendHandle frontend;
    NEXUS_ParserBand parserBand;
    BKNI_EventHandle lockChanged;
}input_band;

static void print_usage(void)
{
    printf(
        "Usage: mmt_live \n"
        "  --help or -h for help\n"
        "  -tlv_pid # pid carrying the TLV data \n"
         );
}

static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    band_config input_config[NUM_BANDS_IN_GROUP] = {
       {true, 30, 4},
       {false, 30, 4},
       {false, 30, 4},
       {false, 30, 4}
    };
    input_band input[NUM_BANDS_IN_GROUP];
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_EtbgHandle hEtbg;
    NEXUS_Etbg_GroupSettings groupSettings;
    NEXUS_RemuxHandle hRmx;
    NEXUS_PidChannelSettings pidChannelOpenSettings;
    NEXUS_PidChannelHandle framePidChnl;
    NEXUS_TsmfSettings tsmfSettings;
    NEXUS_FrontendQamStatus qamStatus;
#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_SimpleStcChannelHandle simpleStcChannel;
    NEXUS_SimpleVideoDecoderHandle simpleVideoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings simpleVideoProgram;
    NEXUS_SimpleAudioDecoderHandle simpleAudioDecoder;
    NEXUS_SimpleAudioDecoderStartSettings simpleAudioProgram;
    NEXUS_SimpleAudioDecoderSettings simpleAudioDecoderSettings;
    NEXUS_MemoryAllocationSettings memSettings;
    NEXUS_ClientConfiguration clientConfig;
#else
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_AudioDecoderSettings audioDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
#endif
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_RemuxSettings rmxSettings;
    NEXUS_Etbg_OpenSettings etbgOpenSettings;
    NEXUS_Error rc;
    bmmt_t mmt = NULL;
    bmmt_open_settings open_settings;
    bmmt_stream_settings video_stream_settings;
    bmmt_stream_t video_stream;
    bmmt_stream_settings audio_stream_settings;
    bmmt_stream_t audio_stream;
    bmmt_msg_settings msg_settings;
    bmmt_msg_t amt_msg=NULL;
    bmmt_msg_t plt_msg=NULL;
    bmmt_msg_t mpt_msg=NULL;
    uint8_t mmt_si_buf[BMMT_MAX_MMT_SI_BUFFER_SIZE];
    uint8_t tlv_si_buf[BMMT_MAX_TLV_SI_BUFFER_SIZE];
    uint8_t msg_r = 0;
    bmmt_pl_table pl_table;
    bmmt_mp_table mp_table[MAX_PACKAGES];
    btlv_am_table am_table;
    btlv_ip_address ip_addr;
    NEXUS_FrontendAcquireSettings acquireSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    bool acquired = true;
    unsigned i = 0;
    int curarg = 1;
    NEXUS_RemuxParserBandwidth remuxParserBandwidth;

    bmmt_get_default_open_settings(&open_settings);
    open_settings.tlv_pid = 0x2d;
    /**
      *  read command line parameters
      **/
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp("-tlv_pid",argv[curarg]) && argc>curarg+1) {
             open_settings.tlv_pid = strtol(argv[++curarg],NULL,0);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
#else
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    for( i = 0; i < NEXUS_NUM_PARSER_BANDS; i++ )
    {
        platformSettings.transportModuleSettings.maxDataRate.parserBand[ i ] = 125000000;
    }
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
#endif
    NEXUS_Remux_GetDefaultSettings(&rmxSettings);
    hRmx = NEXUS_Remux_Open(NEXUS_ANY_ID, &rmxSettings);
    BDBG_ASSERT( hRmx );

    NEXUS_Remux_GetDefaultParserBandwidth(&remuxParserBandwidth);
    remuxParserBandwidth.maxDataRate = 125000000;
    remuxParserBandwidth.parserBand = NEXUS_ParserBand_e0;
    NEXUS_Remux_SetParserBandwidth(hRmx,&remuxParserBandwidth);

    rc = NEXUS_Remux_Start(hRmx);
    BDBG_ASSERT(!rc);

    NEXUS_Etbg_GetDefaultOpenSettings( &etbgOpenSettings);

    hEtbg = NEXUS_Etbg_Open( &etbgOpenSettings );
    BDBG_ASSERT( hEtbg );
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelOpenSettings);
    pidChannelOpenSettings.continuityCountEnabled = false;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    for (i=0;i<NUM_BANDS_IN_GROUP;i++)
    {
        NEXUS_FrontendQamSettings qamSettings;
        #ifdef NXCLIENT_SUPPORT
        NEXUS_FrontendAcquireSettings frontendAcquireSettings;
        NEXUS_Frontend_GetDefaultAcquireSettings(&frontendAcquireSettings);
        frontendAcquireSettings.capabilities.qam = true;
        input[i].frontend = NEXUS_Frontend_Acquire(&frontendAcquireSettings);
        BDBG_ASSERT((input[i].frontend));
        #else
        input[i].frontend = platformConfig.frontend[i];
        #endif
        BKNI_CreateEvent(&input[i].lockChanged);
        input[i].tsmf = NEXUS_Tsmf_Open( NEXUS_TSMF_INDEX(NEXUS_TsmfType_eBackend, i), NULL );
        BDBG_ASSERT(input[i].tsmf);
        NEXUS_Tsmf_GetSettings( input[i].tsmf, &tsmfSettings );
        tsmfSettings.sourceType = NEXUS_TsmfSourceType_eMtsifRx; /* Stream from MTSIF, parsed on the backend chip */
        tsmfSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector( input[i].frontend );
        tsmfSettings.enabled = true;
        tsmfSettings.fieldVerifyConfig.versionChangeMode = NEXUS_TsmfVersionChangeMode_eAllFrame;
        tsmfSettings.relativeTsNum = RELATIVE_TS_NUM;
        tsmfSettings.semiAutomaticMode = false;    /* Go full auto */
        rc = NEXUS_Tsmf_SetSettings( input[i].tsmf, &tsmfSettings );
        BDBG_ASSERT( !rc );

        input[i].parserBand = NEXUS_ParserBand_Open( NEXUS_ANY_ID );
        NEXUS_ParserBand_GetSettings( input[i].parserBand, &parserBandSettings );
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eTsmf;
        parserBandSettings.sourceTypeSettings.tsmf = input[i].tsmf;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        parserBandSettings.teiIgnoreEnabled = true;
        parserBandSettings.acceptAdapt00 = true;
        parserBandSettings.continuityCountEnabled = false;
        parserBandSettings.acceptNullPackets = false;
        rc = NEXUS_ParserBand_SetSettings( input[i].parserBand, &parserBandSettings );
        BDBG_ASSERT( !rc );
        rc = NEXUS_Etbg_AddParserBand(hEtbg, input[i].parserBand, input_config[i].bitRate, input_config[i].numSubFrameSlots);
        BDBG_ASSERT( !rc );
        if (input_config[i].isPrimary)
        {
            framePidChnl = NEXUS_PidChannel_Open( input[i].parserBand, 0x2F, &pidChannelOpenSettings );
            BDBG_ASSERT( framePidChnl );
            rc = NEXUS_Remux_AddPidChannel(hRmx, framePidChnl);
            BDBG_ASSERT( !rc );
            NEXUS_Etbg_GetGroupSettings(hEtbg, &groupSettings);
            groupSettings.primary = input[i].parserBand;
            NEXUS_Etbg_SetGroupSettings(hEtbg, &groupSettings);
        }
        BKNI_ResetEvent( input[i].lockChanged );
        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = 765 * 1000000;
        qamSettings.mode = NEXUS_FrontendQamMode_e256;
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.lockCallback.callback = lock_changed_callback;
        qamSettings.lockCallback.context = input[i].lockChanged;
        rc = NEXUS_Frontend_TuneQam( input[i].frontend, &qamSettings );
        BDBG_ASSERT( !rc );
        do
        {
            BKNI_WaitForEvent(input[i].lockChanged, BKNI_INFINITE);
            NEXUS_Frontend_GetQamStatus(input[i].frontend, &qamStatus);
            fprintf(stderr, "QAM Lock callback, frontend %p - lock status %d, %d\n", (void*)input[i].frontend,
            qamStatus.fecLock, qamStatus.receiverLock);
        }while ( !qamStatus.fecLock || !qamStatus.receiverLock);
    }
    rc = NEXUS_Etbg_Start(hEtbg);
    BDBG_ASSERT( !rc );
    NEXUS_Etbg_GetGroupSettings(hEtbg, &groupSettings);
    parserBand = groupSettings.primary;

#ifdef NXCLIENT_SUPPORT
simpleStcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.surfaceClient = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc AV decode resources")); return -1;}
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = 3840;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = 2160;
    connectSettings.simpleVideoDecoder[0].surfaceClientId =  allocResults.surfaceClient[0].id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxFormat = NEXUS_VideoFormat_e3840x2160p60hz;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[NEXUS_VideoCodec_eH265] = true;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.type = NxClient_VideoWindowType_eMain;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}
    simpleVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    NEXUS_SimpleVideoDecoder_GetSettings(simpleVideoDecoder,&videoDecoderSettings);
    videoDecoderSettings.maxWidth = 3840;
    videoDecoderSettings.maxHeight = 2160;
    videoDecoderSettings.discardThreshold = 10*45000;
    NEXUS_SimpleVideoDecoder_SetSettings(simpleVideoDecoder,&videoDecoderSettings);
    NEXUS_SimpleVideoDecoder_SetStcChannel(simpleVideoDecoder, simpleStcChannel);
    simpleAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    NEXUS_SimpleAudioDecoder_SetStcChannel(simpleAudioDecoder, simpleStcChannel);
    NEXUS_SimpleAudioDecoder_GetSettings(simpleAudioDecoder,&simpleAudioDecoderSettings);
    simpleAudioDecoderSettings.primary.discardThreshold = 10*1000;
    simpleAudioDecoderSettings.secondary.discardThreshold = 10*1000;
    NEXUS_SimpleAudioDecoder_SetSettings(simpleAudioDecoder,&simpleAudioDecoderSettings);
#else
    /**
      * Open display and add outputs
      **/
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    /**
      * Open nexus STC channel for AV sync
      **/
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /**
      *  Open nexus audio decoder and add outputs
      **/
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.fifoSize = 512*1024;
    audioDecoder = NEXUS_AudioDecoder_Open(0,&audioDecoderOpenSettings);
    if (platformConfig.outputs.audioDacs[0]) {
        if (NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0])) {
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        }
    }
    if (NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0])) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0])) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#endif

    /**
      *  Open nexus video decoder and nexus video window and connect
      *  them
      **/
    window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.maxWidth = 3840;
    videoDecoderSettings.maxHeight = 2160;
    videoDecoderSettings.discardThreshold = 10*45000;
    NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_AudioDecoder_GetSettings(audioDecoder,&audioDecoderSettings);
    audioDecoderSettings.discardThreshold = 10*1000;
    NEXUS_AudioDecoder_SetSettings(audioDecoder,&audioDecoderSettings);

    NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;
    rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
#endif
    /**
      *  mmt module instantiation
      **/
    open_settings.parserBand = parserBand;
    open_settings.input_format = ebmmt_input_format_mpeg2ts;
    mmt =  bmmt_open(&open_settings);
    BDBG_ASSERT(mmt);
    /**
      * open TLV AMT message context
      **/
    bmmt_msg_get_default_settings(&msg_settings);
    msg_settings.msg_type = ebmmt_msg_type_tlv;
    amt_msg = bmmt_msg_open(mmt,&msg_settings);
     /**
     * load static decryption tables
     **/
    bmmt_set_static_test_keys(mmt,odd_key_table,even_key_table,MAX_KEYS);
    /**
      * start the mmt module
      **/
    bmmt_start(mmt);
    /** extract service ids from input stream and their
      *   network addresses from TLV SI AMT
      **/
    while (msg_r < BMMT_MAX_MSG_BUFFERS )
    {
        uint8_t *buf = tlv_si_buf;
        size_t len;
        msg_read1:
        len = bmmt_msg_get_buffer(amt_msg,buf,BMMT_MAX_TLV_SI_BUFFER_SIZE);
        if (len)
        {
            if (bmmt_get_am_table(buf,len,&am_table))
                break;
            msg_r +=1;
        }
        else
        {
            BKNI_Sleep(50);
            goto msg_read1;
        }
    }
    /**
      *  close TLV msg context
      **/
    bmmt_msg_close(amt_msg);
    if (msg_r == BMMT_MAX_MSG_BUFFERS)
    {
        BDBG_ERR(("TLV SI AMT not found"));
        goto done_mmt;
    }
    /**
      *  set IP filtering for the TLV packets
      **/
    if (am_table.num_of_service_id)
    {
        if (am_table.services[0].is_ipv6)
        {
            ip_addr.type = btlv_ip_address_ipv6;
            BKNI_Memcpy(&ip_addr.address.ipv6.addr,&am_table.services[0].addr.ipv6.dst_addr,sizeof(ip_addr.address.ipv6.addr));
            ip_addr.address.ipv6.port = 0x0; /* ignore port since AMT doesn't provide port number */
        }
        else
        {
            ip_addr.type = btlv_ip_address_ipv4;
            BKNI_Memcpy(&ip_addr.address.ipv4.addr,&am_table.services[0].addr.ipv4.dst_addr,sizeof(ip_addr.address.ipv4.addr));
            ip_addr.address.ipv4.port = 0x0; /* ignore port since AMT doesn't provide port number */
        }
    }
    else
    {
        BDBG_WRN(("no services found in AMT"));
        goto done_mmt;
    }
    bmmt_set_ip_filter(mmt, &ip_addr);
    /**
      *  open PLT message context
      **/
    bmmt_msg_get_default_settings(&msg_settings);
    msg_settings.msg_type = ebmmt_msg_type_mmt;
    msg_settings.pid = 0x0;
    plt_msg = bmmt_msg_open(mmt,&msg_settings);
    /**
      *  extract PLT from PA message
      **/
    msg_r = 0;
    while (msg_r < BMMT_MAX_MSG_BUFFERS)
    {
        uint8_t *buf = mmt_si_buf;
        size_t len;
        msg_read2:
        len = bmmt_msg_get_buffer(plt_msg, buf,BMMT_MAX_MMT_SI_BUFFER_SIZE);
        if (len)
        {
            if (bmmt_get_pl_table(buf,len,&pl_table))
                break;
            msg_r +=1;
        }
        else
        {
            BKNI_Sleep(50);
            goto msg_read2;
        }
    }
    /**
      *  close plt message context
      **/
    bmmt_msg_close(plt_msg);
    if (msg_r == BMMT_MAX_MSG_BUFFERS)
    {
        BDBG_ERR(("MMT SI PLT not found"));
        goto done_mmt;
    }
    else
    {
        if (pl_table.num_of_packages)
        {
           /*for (i=0;i<pl_table.num_of_packages;i++) */
           i = 0;
           {
               bmmt_msg_get_default_settings(&msg_settings);
               msg_settings.msg_type = ebmmt_msg_type_mmt;
               switch (pl_table.packages[i].location_info.location_type)
               {
               case bmmt_general_location_type_id:
                   msg_settings.pid = pl_table.packages[i].location_info.data.packet_id;
                   break;
               case bmmt_general_location_type_ipv4:
                   msg_settings.pid = pl_table.packages[i].location_info.data.mmt_ipv4.packet_id;
                   break;
               case bmmt_general_location_type_ipv6:
                   msg_settings.pid = pl_table.packages[i].location_info.data.mmt_ipv6.packet_id;
                   break;
               default:
                   BDBG_WRN(("MPT packet ID not known"));
                   goto done_mmt;
                }
                /**
                  * open MPT message context
                  **/
                mpt_msg = bmmt_msg_open(mmt,&msg_settings);
                /**
                  * extract MPT from PA message
                  **/
                msg_r = 0;
                while (msg_r < BMMT_MAX_MSG_BUFFERS )
                {
                    uint8_t *buf = mmt_si_buf;
                    size_t len;
                    msg_read3:
                    len = bmmt_msg_get_buffer(mpt_msg, buf,BMMT_MAX_MMT_SI_BUFFER_SIZE);
                    if (len)
                    {
                         if (bmmt_get_mp_table(buf,len,&mp_table[i]))
                            break;
                         msg_r +=1;
                    }
                    else
                    {
                        BKNI_Sleep(50);
                        goto msg_read3;
                    }
                }
                if (msg_r == BMMT_MAX_MSG_BUFFERS)
                {
                    BDBG_ERR(("MMT SI PMT not found in MMT PID %u",msg_settings.pid));
                    goto done_mmt;
                }
                /*bmmt_msg_close(mpt_msg);*/
            }
        }
        else
        {
            BDBG_WRN(("no packages found in the PLT"));
            goto done_mmt;
        }
    }
    /**
      * find video asset index in the 1st MPT
      **/
    for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"hev1"));i++);
    /**
      * some sample streams have video string as hvc1
      **/
    if (i==mp_table[0].num_of_assets)
    {
        for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"hvc1"));i++);
    }
    /**
      * open video stream context
      **/
    if (i!=mp_table[0].num_of_assets)
    {
        /**
          *   open video stream context for the video packet ID in the
          *   1st asset of MPT
          **/
        bmmt_stream_get_default_settings(&video_stream_settings);
        switch (mp_table[0].assets[i].location_info[0].location_type)
        {
        case bmmt_general_location_type_id:
            video_stream_settings.pid = mp_table[0].assets[i].id[0] << 8 | mp_table[0].assets[i].id[1];
            /*video_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.packet_id;*/
            break;
        case bmmt_general_location_type_ipv4:
            video_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv4.packet_id;
            break;
        case bmmt_general_location_type_ipv6:
            video_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv6.packet_id;
            break;
        default:
            BDBG_WRN(("video stream location ID not supported"));
            goto done_mmt;
        }
        if (video_stream_settings.pid )
        {
            video_stream_settings.stream_type = bmmt_stream_type_h265;
            BDBG_WRN(("mp_table[0].assets[i].type %s",mp_table[0].assets[i].type));
            BDBG_WRN(("video_stream_settings.pid %04x",video_stream_settings.pid));
            video_stream = bmmt_stream_open(mmt,&video_stream_settings);
            BDBG_ASSERT(video_stream);
            videoPidChannel = bmmt_stream_get_pid_channel(video_stream);
            #ifdef NXCLIENT_SUPPORT
            NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&simpleVideoProgram);
            simpleVideoProgram.settings.codec = NEXUS_VideoCodec_eH265;
            simpleVideoProgram.settings.pidChannel = videoPidChannel;
            simpleVideoProgram.maxHeight = 2160;
            simpleVideoProgram.maxWidth = 3860;
            NEXUS_SimpleVideoDecoder_Start(simpleVideoDecoder,&simpleVideoProgram);
            #else
            NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
            videoProgram.codec = NEXUS_VideoCodec_eH265;
            videoProgram.pidChannel = videoPidChannel;
            videoProgram.stcChannel = stcChannel;
            NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
            #endif
        }
        else
        {
            BDBG_WRN(("video stream location ID not supported"));
        }
    }
    else
    {
        BDBG_WRN(("no video asset was found"));
    }
    /**
      * find audio asset index in the 1st MPT
      **/
    /*for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"mp4a"));i++) ;*/
    for (i=mp_table[0].num_of_assets;(i>0 && strcmp((char *)mp_table[0].assets[i].type,"mp4a"));i--) ;
    /**
      *   open audio stream context
      **/
    if(i!=mp_table[0].num_of_assets)
    {
        /**
          * open video stream context for the video packet ID in the 1st
          * asset of MPT
          **/
        bmmt_stream_get_default_settings(&audio_stream_settings);
        switch (mp_table[0].assets[i].location_info[0].location_type)
        {
        case bmmt_general_location_type_id:
            audio_stream_settings.pid = mp_table[0].assets[i].id[0] << 8 | mp_table[0].assets[i].id[1];
            /*audio_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.packet_id;*/
            break;
        case bmmt_general_location_type_ipv4:
            audio_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv4.packet_id;
            break;
        case bmmt_general_location_type_ipv6:
             audio_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv6.packet_id;
             break;
        default:
            BDBG_WRN(("audio stream location ID not supported"));
            goto done_mmt;
        }
        if (audio_stream_settings.pid )
        {
            audio_stream_settings.stream_type = bmmt_stream_type_aac;
            BDBG_WRN(("mp_table[0].assets[i].type %s",mp_table[0].assets[i].type));
            BDBG_WRN(("audio_stream_settings.pid %04x",audio_stream_settings.pid));
            audio_stream = bmmt_stream_open(mmt,&audio_stream_settings);
            BDBG_ASSERT(audio_stream);
            audioPidChannel = bmmt_stream_get_pid_channel(audio_stream);
            #ifdef NXCLIENT_SUPPORT
            NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&simpleAudioProgram);
            simpleAudioProgram.primary.codec = NEXUS_AudioCodec_eAacLoas;
            simpleAudioProgram.primary.pidChannel = audioPidChannel;
            NEXUS_SimpleAudioDecoder_Start(simpleAudioDecoder,&simpleAudioProgram);
            #else
            NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
            audioProgram.codec = NEXUS_AudioCodec_eAacLoas;
            audioProgram.pidChannel = audioPidChannel;
            audioProgram.stcChannel = stcChannel;
            NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
            #endif
        }
        else
        {
            BDBG_WRN(("audio stream location ID not supported"));
        }
    }
    else
    {
        BDBG_WRN(("no audio asset was found"));
    }
done_mmt:
    BDBG_WRN(("done with mmt"));


    BDBG_WRN(("Press any key to exit the app"));
    getchar();
    #ifdef NXCLIENT_SUPPORT
    NEXUS_SimpleAudioDecoder_Stop(simpleAudioDecoder);
    #else
    NEXUS_AudioDecoder_Stop(audioDecoder);
    #endif
    #ifdef NXCLIENT_SUPPORT
    NEXUS_SimpleVideoDecoder_Stop(simpleVideoDecoder);
    #else
    NEXUS_VideoDecoder_Stop(videoDecoder);
    #endif
    bmmt_stop(mmt);
    bmmt_close(mmt);
done:
    #ifdef NXCLIENT_SUPPORT
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    #else
    if(window)NEXUS_VideoWindow_RemoveAllInputs(window);
    if(window)NEXUS_VideoWindow_Close(window);
    if(display)NEXUS_Display_Close(display);
    if(videoDecoder)NEXUS_VideoDecoder_Close(videoDecoder);
    if(audioDecoder)NEXUS_AudioDecoder_Close(audioDecoder);
    #endif
    if (hRmx)
    {
       NEXUS_Remux_Stop(hRmx);
       NEXUS_Remux_RemoveAllPidChannels(hRmx);
       NEXUS_Remux_Close(hRmx);
    }
    if (hEtbg) {
       NEXUS_Etbg_Stop(hEtbg);
       NEXUS_Etbg_Close(hEtbg);
    }
    for (i=0;i<NUM_BANDS_IN_GROUP;i++)
    {
          BKNI_DestroyEvent(input[i].lockChanged);
          NEXUS_Tsmf_Close(input[i].tsmf);
    }
    #ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
    #else
    NEXUS_Platform_Uninit();
    #endif
    return 0;
}

#else  /* NEXUS_HAS_ETBG */
int main(void)
{
    printf("ERROR: This platform doesn't include tsmf.inc \n");
    return -1;
}
#endif
