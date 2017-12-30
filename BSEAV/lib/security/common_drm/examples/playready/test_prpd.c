/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <time.h>
#include <sys/time.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "nexus_random_number.h"
#include "drm_types.h"
#include "drm_common.h"

#define X509 void

#include "drm_common_swcrypto.h"
#include "drm_prdy.h"
#include "drm_metadata.h"
#include "drm_common.h"
#include "drm_prdy_http.h"

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

BDBG_MODULE(test_prpd);

#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[512]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 512) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf("%s",tmp); printf("\n");                    \
}

#define AES_EKL_SIZE          32
#define KEY_IV_BUFFER_SIZE    100

const char *g_sample[]   = {"This is the clear testing sample data1.",
                           "This is the clear testing sample data2.",
                           "This is the clear testing sample data3.",
                           "This is the clear testing sample data4."};
uint8_t *g_enc_sample[4] = {0,0,0,0};

uint8_t *g_aes_ctr_padding = NULL;

const char g_url[] = {"http://playready.directtaps.net/pr/svc/rightsmanager.asmx?"};

#if 1  /* OPL 100 */
/* Bear_Video_OPLs100.pyv header */
const uint8_t g_prdyHeaderObj[] = {
  0x84, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x03, 0x3C, 0x00, 0x57, 0x00, 0x52, 0x00, 0x4D, 0x00, 0x48, 0x00,
  0x45, 0x00, 0x41, 0x00, 0x44, 0x00, 0x45, 0x00, 0x52, 0x00, 0x20, 0x00, 0x78, 0x00, 0x6D, 0x00, 0x6C, 0x00, 0x6E, 0x00,
  0x73, 0x00, 0x3D, 0x00, 0x22, 0x00, 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00,
  0x73, 0x00, 0x63, 0x00, 0x68, 0x00, 0x65, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x73, 0x00, 0x2E, 0x00, 0x6D, 0x00, 0x69, 0x00,
  0x63, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x73, 0x00, 0x6F, 0x00, 0x66, 0x00, 0x74, 0x00, 0x2E, 0x00, 0x63, 0x00, 0x6F, 0x00,
  0x6D, 0x00, 0x2F, 0x00, 0x44, 0x00, 0x52, 0x00, 0x4D, 0x00, 0x2F, 0x00, 0x32, 0x00, 0x30, 0x00, 0x30, 0x00, 0x37, 0x00,
  0x2F, 0x00, 0x30, 0x00, 0x33, 0x00, 0x2F, 0x00, 0x50, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x79, 0x00, 0x52, 0x00, 0x65, 0x00,
  0x61, 0x00, 0x64, 0x00, 0x79, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x22, 0x00,
  0x20, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x3D, 0x00, 0x22, 0x00,
  0x34, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x22, 0x00, 0x3E, 0x00, 0x3C, 0x00,
  0x44, 0x00, 0x41, 0x00, 0x54, 0x00, 0x41, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x50, 0x00, 0x52, 0x00, 0x4F, 0x00, 0x54, 0x00,
  0x45, 0x00, 0x43, 0x00, 0x54, 0x00, 0x49, 0x00, 0x4E, 0x00, 0x46, 0x00, 0x4F, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4B, 0x00,
  0x45, 0x00, 0x59, 0x00, 0x4C, 0x00, 0x45, 0x00, 0x4E, 0x00, 0x3E, 0x00, 0x31, 0x00, 0x36, 0x00, 0x3C, 0x00, 0x2F, 0x00,
  0x4B, 0x00, 0x45, 0x00, 0x59, 0x00, 0x4C, 0x00, 0x45, 0x00, 0x4E, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x41, 0x00, 0x4C, 0x00,
  0x47, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x41, 0x00, 0x45, 0x00, 0x53, 0x00, 0x43, 0x00, 0x54, 0x00, 0x52, 0x00,
  0x3C, 0x00, 0x2F, 0x00, 0x41, 0x00, 0x4C, 0x00, 0x47, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x2F, 0x00,
  0x50, 0x00, 0x52, 0x00, 0x4F, 0x00, 0x54, 0x00, 0x45, 0x00, 0x43, 0x00, 0x54, 0x00, 0x49, 0x00, 0x4E, 0x00, 0x46, 0x00,
  0x4F, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4B, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x48, 0x00, 0x2B, 0x00, 0x6C, 0x00,
  0x31, 0x00, 0x57, 0x00, 0x34, 0x00, 0x58, 0x00, 0x4A, 0x00, 0x5A, 0x00, 0x6B, 0x00, 0x4B, 0x00, 0x30, 0x00, 0x72, 0x00,
  0x4B, 0x00, 0x56, 0x00, 0x35, 0x00, 0x33, 0x00, 0x62, 0x00, 0x4F, 0x00, 0x34, 0x00, 0x63, 0x00, 0x51, 0x00, 0x3D, 0x00,
  0x3D, 0x00, 0x3C, 0x00, 0x2F, 0x00, 0x4B, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4C, 0x00, 0x41, 0x00,
  0x5F, 0x00, 0x55, 0x00, 0x52, 0x00, 0x4C, 0x00, 0x3E, 0x00, 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00,
  0x2F, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x79, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00,
  0x79, 0x00, 0x2E, 0x00, 0x64, 0x00, 0x69, 0x00, 0x72, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x74, 0x00, 0x61, 0x00,
  0x70, 0x00, 0x73, 0x00, 0x2E, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x74, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x72, 0x00, 0x2F, 0x00,
  0x73, 0x00, 0x76, 0x00, 0x63, 0x00, 0x2F, 0x00, 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x73, 0x00,
  0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x72, 0x00, 0x2E, 0x00, 0x61, 0x00, 0x73, 0x00,
  0x6D, 0x00, 0x78, 0x00, 0x3F, 0x00, 0x3C, 0x00, 0x2F, 0x00, 0x4C, 0x00, 0x41, 0x00, 0x5F, 0x00, 0x55, 0x00, 0x52, 0x00,
  0x4C, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4C, 0x00, 0x55, 0x00, 0x49, 0x00, 0x5F, 0x00, 0x55, 0x00, 0x52, 0x00, 0x4C, 0x00,
  0x3E, 0x00, 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x6C, 0x00,
  0x61, 0x00, 0x79, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x79, 0x00, 0x2E, 0x00, 0x64, 0x00, 0x69, 0x00,
  0x72, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x74, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x2E, 0x00, 0x6E, 0x00,
  0x65, 0x00, 0x74, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x72, 0x00, 0x2F, 0x00, 0x73, 0x00, 0x76, 0x00, 0x63, 0x00, 0x2F, 0x00,
  0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x73, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x61, 0x00,
  0x67, 0x00, 0x65, 0x00, 0x72, 0x00, 0x2E, 0x00, 0x61, 0x00, 0x73, 0x00, 0x6D, 0x00, 0x78, 0x00, 0x3F, 0x00, 0x3C, 0x00,
  0x2F, 0x00, 0x4C, 0x00, 0x55, 0x00, 0x49, 0x00, 0x5F, 0x00, 0x55, 0x00, 0x52, 0x00, 0x4C, 0x00, 0x3E, 0x00, 0x3C, 0x00,
  0x44, 0x00, 0x53, 0x00, 0x5F, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x41, 0x00, 0x48, 0x00, 0x2B, 0x00, 0x30, 0x00,
  0x33, 0x00, 0x6A, 0x00, 0x75, 0x00, 0x4B, 0x00, 0x62, 0x00, 0x55, 0x00, 0x47, 0x00, 0x62, 0x00, 0x48, 0x00, 0x6C, 0x00,
  0x31, 0x00, 0x56, 0x00, 0x2F, 0x00, 0x51, 0x00, 0x49, 0x00, 0x77, 0x00, 0x52, 0x00, 0x41, 0x00, 0x3D, 0x00, 0x3D, 0x00,
  0x3C, 0x00, 0x2F, 0x00, 0x44, 0x00, 0x53, 0x00, 0x5F, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x43, 0x00,
  0x48, 0x00, 0x45, 0x00, 0x43, 0x00, 0x4B, 0x00, 0x53, 0x00, 0x55, 0x00, 0x4D, 0x00, 0x3E, 0x00, 0x65, 0x00, 0x34, 0x00,
  0x33, 0x00, 0x32, 0x00, 0x51, 0x00, 0x30, 0x00, 0x31, 0x00, 0x56, 0x00, 0x42, 0x00, 0x4E, 0x00, 0x41, 0x00, 0x3D, 0x00,
  0x3C, 0x00, 0x2F, 0x00, 0x43, 0x00, 0x48, 0x00, 0x45, 0x00, 0x43, 0x00, 0x4B, 0x00, 0x53, 0x00, 0x55, 0x00, 0x4D, 0x00,
  0x3E, 0x00, 0x3C, 0x00, 0x2F, 0x00, 0x44, 0x00, 0x41, 0x00, 0x54, 0x00, 0x41, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x2F, 0x00,
  0x57, 0x00, 0x52, 0x00, 0x4D, 0x00, 0x48, 0x00, 0x45, 0x00, 0x41, 0x00, 0x44, 0x00, 0x45, 0x00, 0x52, 0x00, 0x3E, 0x00
};
const uint8_t g_keyID[] = {0x1F, 0xE9, 0x75, 0x5B, 0x85, 0xC9, 0x66, 0x42, 0xB4, 0xAC, 0xA5, 0x79, 0xDD, 0xB3, 0xB8, 0x71};

#else
/* Bear_Video_OPLs0.pyv header */
const uint8_t g_prdyHeaderObj[] = {
 0x36, 0x03, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x2C, 0x03, 0x3C, 0x00, 0x57, 0x00, 0x52, 0x00, 0x4D, 0x00, 0x48, 0x00,
 0x45, 0x00, 0x41, 0x00, 0x44, 0x00, 0x45, 0x00, 0x52, 0x00, 0x20, 0x00, 0x78, 0x00, 0x6D, 0x00, 0x6C, 0x00, 0x6E, 0x00,
 0x73, 0x00, 0x3D, 0x00, 0x22, 0x00, 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00,
 0x73, 0x00, 0x63, 0x00, 0x68, 0x00, 0x65, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x73, 0x00, 0x2E, 0x00, 0x6D, 0x00, 0x69, 0x00,
 0x63, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x73, 0x00, 0x6F, 0x00, 0x66, 0x00, 0x74, 0x00, 0x2E, 0x00, 0x63, 0x00, 0x6F, 0x00,
 0x6D, 0x00, 0x2F, 0x00, 0x44, 0x00, 0x52, 0x00, 0x4D, 0x00, 0x2F, 0x00, 0x32, 0x00, 0x30, 0x00, 0x30, 0x00, 0x37, 0x00,
 0x2F, 0x00, 0x30, 0x00, 0x33, 0x00, 0x2F, 0x00, 0x50, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x79, 0x00, 0x52, 0x00, 0x65, 0x00,
 0x61, 0x00, 0x64, 0x00, 0x79, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x22, 0x00,
 0x20, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x3D, 0x00, 0x22, 0x00,
 0x34, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x22, 0x00, 0x3E, 0x00, 0x3C, 0x00,
 0x44, 0x00, 0x41, 0x00, 0x54, 0x00, 0x41, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x50, 0x00, 0x52, 0x00, 0x4F, 0x00, 0x54, 0x00,
 0x45, 0x00, 0x43, 0x00, 0x54, 0x00, 0x49, 0x00, 0x4E, 0x00, 0x46, 0x00, 0x4F, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4B, 0x00,
 0x45, 0x00, 0x59, 0x00, 0x4C, 0x00, 0x45, 0x00, 0x4E, 0x00, 0x3E, 0x00, 0x31, 0x00, 0x36, 0x00, 0x3C, 0x00, 0x2F, 0x00,
 0x4B, 0x00, 0x45, 0x00, 0x59, 0x00, 0x4C, 0x00, 0x45, 0x00, 0x4E, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x41, 0x00, 0x4C, 0x00,
 0x47, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x41, 0x00, 0x45, 0x00, 0x53, 0x00, 0x43, 0x00, 0x54, 0x00, 0x52, 0x00,
 0x3C, 0x00, 0x2F, 0x00, 0x41, 0x00, 0x4C, 0x00, 0x47, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x2F, 0x00,
 0x50, 0x00, 0x52, 0x00, 0x4F, 0x00, 0x54, 0x00, 0x45, 0x00, 0x43, 0x00, 0x54, 0x00, 0x49, 0x00, 0x4E, 0x00, 0x46, 0x00,
 0x4F, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4B, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x74 ,0x00 ,0x75 ,0x00 ,0x73 ,0x00,
 0x59 ,0x00 ,0x4E ,0x00 ,0x33 ,0x00 ,0x75 ,0x00 ,0x6F ,0x00 ,0x65 ,0x00 ,0x55 ,0x00 ,0x2B ,0x00 ,0x7A ,0x00 ,0x4C ,0x00,
 0x41 ,0x00 ,0x58 ,0x00 ,0x43 ,0x00 ,0x4A ,0x00 ,0x75 ,0x00 ,0x48 ,0x00 ,0x51 ,0x00 ,0x30 ,0x00 ,0x77 ,0x00 ,0x3D ,0x00,
 0x3D ,0x00, 0x3C, 0x00, 0x2F, 0x00, 0x4B, 0x00, 0x49, 0x00, 0x44, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4C, 0x00, 0x41, 0x00,
 0x5F, 0x00, 0x55, 0x00, 0x52, 0x00, 0x4C, 0x00, 0x3E, 0x00, 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00,
 0x2F, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x79, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00,
 0x79, 0x00, 0x2E, 0x00, 0x64, 0x00, 0x69, 0x00, 0x72, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x74, 0x00, 0x61, 0x00,
 0x70, 0x00, 0x73, 0x00, 0x2E, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x74, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x72, 0x00, 0x2F, 0x00,
 0x73, 0x00, 0x76, 0x00, 0x63, 0x00, 0x2F, 0x00, 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x73, 0x00,
 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x72, 0x00, 0x2E, 0x00, 0x61, 0x00, 0x73, 0x00,
 0x6D, 0x00, 0x78, 0x00, 0x3F, 0x00, 0x3C, 0x00, 0x2F, 0x00, 0x4C, 0x00, 0x41, 0x00, 0x5F, 0x00, 0x55, 0x00, 0x52, 0x00,
 0x4C, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x4C, 0x00, 0x55, 0x00, 0x49, 0x00, 0x5F, 0x00, 0x55, 0x00, 0x52, 0x00, 0x4C, 0x00,
 0x3E, 0x00, 0x68, 0x00, 0x74, 0x00, 0x74, 0x00, 0x70, 0x00, 0x3A, 0x00, 0x2F, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x6C, 0x00,
 0x61, 0x00, 0x79, 0x00, 0x72, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x79, 0x00, 0x2E, 0x00, 0x64, 0x00, 0x69, 0x00,
 0x72, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x74, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x2E, 0x00, 0x6E, 0x00,
 0x65, 0x00, 0x74, 0x00, 0x2F, 0x00, 0x70, 0x00, 0x72, 0x00, 0x2F, 0x00, 0x73, 0x00, 0x76, 0x00, 0x63, 0x00, 0x2F, 0x00,
 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x73, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x61, 0x00,
 0x67, 0x00, 0x65, 0x00, 0x72, 0x00, 0x2E, 0x00, 0x61, 0x00, 0x73, 0x00, 0x6D, 0x00, 0x78, 0x00, 0x3F, 0x00, 0x3C, 0x00,
 0x2F, 0x00, 0x4C, 0x00, 0x55, 0x00, 0x49, 0x00, 0x5F, 0x00, 0x55, 0x00, 0x52, 0x00, 0x4C, 0x00, 0x3E, 0x00, 0x3C, 0x00,
 0x43, 0x00, 0x48, 0x00, 0x45, 0x00, 0x43, 0x00, 0x4B, 0x00, 0x53, 0x00, 0x55, 0x00, 0x4D, 0x00, 0x3E, 0x00, 0x33 ,0x00,
 0x68, 0x00, 0x4E, 0x00, 0x79, 0x00, 0x46, 0x00, 0x39, 0x00, 0x38, 0x00, 0x51, 0x00, 0x51, 0x00, 0x6B, 0x00, 0x6F, 0x00,
 0x3D, 0x00, 0x3C, 0x00, 0x2F, 0x00, 0x43, 0x00, 0x48, 0x00, 0x45, 0x00, 0x43, 0x00, 0x4B, 0x00, 0x53, 0x00, 0x55, 0x00,
 0x4D, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x2F, 0x00, 0x44, 0x00, 0x41, 0x00, 0x54, 0x00, 0x41, 0x00, 0x3E, 0x00, 0x3C, 0x00,
 0x2F, 0x00, 0x57, 0x00, 0x52, 0x00, 0x4D, 0x00, 0x48, 0x00, 0x45, 0x00, 0x41, 0x00, 0x44, 0x00, 0x45, 0x00, 0x52, 0x00,
 0x3E, 0x00
};
const uint8_t g_keyID[] = {0xB6, 0xEB, 0x18, 0x37, 0x7B, 0xA8, 0x79, 0x4F, 0xB3, 0x2C, 0x05, 0xC2, 0x26, 0xE1, 0xD0, 0xD3};
#endif


#define MAX_LICENCE_RESPONSE_LENGTH (1024*64)
#define MAX_TIME_CHALLENGE_RESPONSE_LENGTH (1024*5)
#define MAX_URL_LENGTH (512)

/* **FixMe** put this here for now */
/*#define NETFLIX_EXT 1*/

#ifdef NETFLIX_EXT
/* **FixMe** test code until IsSecureStopAPI is implemented */
static bool		bIsSecureStopEnabled = false;
static uint8_t	sSessionIdBuf[DRM_PRDY_SESSION_ID_LEN];
#endif

static
int gen_random_num( uint32_t numberOfBytes, uint8_t *pRandomBytes)
{
    int rc = 0;
    NEXUS_RandomNumberGenerateSettings settings;
    NEXUS_RandomNumberOutput rngOutput;
    NEXUS_Error nxs_rc = NEXUS_SUCCESS;

    NEXUS_RandomNumber_GetDefaultGenerateSettings(&settings);
    settings.randomNumberSize = numberOfBytes;

    nxs_rc = NEXUS_RandomNumber_Generate(&settings, &rngOutput);
    if( (nxs_rc != NEXUS_SUCCESS) || (rngOutput.size != numberOfBytes) )
    {
        printf("%s - Error generating '%u' random bytes (only '%u' bytes returned) ", BSTD_FUNCTION, numberOfBytes, rngOutput.size);
        rc = -1;
        goto ErrorExit;
    }

    BKNI_Memcpy(pRandomBytes, rngOutput.buffer, numberOfBytes);

ErrorExit:
    return rc;
}

int gen_key_from_seed( const uint8_t * kid, uint8_t * key )
{
    DrmCommonInit_t commonDrmInit;
    uint8_t seed[30] = { 0x5D, 0x50, 0x68, 0xBE, 0xC9, 0xB3, 0x84, 0xFF, 0x60, 0x44,
                         0x86, 0x71, 0x59, 0xF1, 0x6D, 0x6B, 0x75, 0x55, 0x44, 0xFC,
                         0xD5, 0x11, 0x69, 0x89, 0xB1, 0xAC, 0xC4, 0x27, 0x8E, 0x88 };
    uint8_t seed_kid[30+16] = {0};
    uint8_t seed_kid_seed[30+16+30] = {0};
    uint8_t seed_kid_seed_kid[30+16+30+16] = {0};
    uint8_t sha_A[32] = {0};
    uint8_t sha_B[32] = {0};
    uint8_t sha_C[32] = {0};
    int i = 0;

    DRM_Common_BasicInitialize(&commonDrmInit);

    memcpy(seed_kid,seed,30);
    memcpy(&seed_kid[30],kid,16);

    memcpy(seed_kid_seed,seed,30);
    memcpy(&seed_kid_seed[30],kid,16);
    memcpy(&seed_kid_seed[30+16],seed,30);

    memcpy(seed_kid_seed_kid,seed,30);
    memcpy(&seed_kid_seed_kid[30],kid,16);
    memcpy(&seed_kid_seed_kid[30+16],seed,30);
    memcpy(&seed_kid_seed_kid[30+16+30],kid,16);

    DRM_Common_SwSha256(seed_kid, sha_A, 30+16);
    DRM_Common_SwSha256(seed_kid_seed, sha_B, 30+16+30);
    DRM_Common_SwSha256(seed_kid_seed_kid, sha_C, 30+16+30+16);

    DUMP_DATA_HEX("\tKID                    : ",kid,16);
    printf("\tGenerated Key from seed: ");
    for( i = 0; i < 16; i++)
    {
       key[i] = sha_A[i] ^ sha_A[i+16] ^ sha_B[i] ^ sha_B[i+16] ^ sha_C[i] ^ sha_C[i+16];
       printf("%02x ",key[i]);
    }
    printf("\n");
    DRM_Common_Finalize();
    return 0;
}

int encrypt_sample(uint8_t *key,
                   uint64_t *pIv,
                   uint64_t blockCounter,
                   size_t  byteOffset,
                   const NEXUS_DmaJobBlockSettings *pBlks,
                   uint32_t nDmaBlocks,
                   CommonCryptoHandle  cryptoHandle,
                   NEXUS_KeySlotHandle keySlot)
{
    NEXUS_DmaJobBlockSettings   blks[10];
    CommonCryptoJobSettings     jobSettings;
    CommonCryptoClearKeySettings keySettings;
    int                         rc = 0;
    uint32_t                    nb_Blks = 0;
    uint8_t                    *pBuf = NULL;
    unsigned                    ii=0;
    unsigned                    nbBlkInHeader = 0; /* header contains the descriptor pointing to the key and possibly an extra block
                                                      for padding when decryption doesn't start on a AES block boundary */
    /* Set Key and IV */
    rc = NEXUS_Memory_Allocate(16, NULL, (void *)&pBuf);
    if(rc != NEXUS_SUCCESS){
        printf("%s - NEXUS_Memory_Allocate failed, rc = %d\n", BSTD_FUNCTION, rc);
        goto ErrorExit;
    }


    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     !  NOTE: VERY IMPORTANT
     ! Before manually performing AES CTR encryption, we need to convert the IV
     ! from qword to network bytes because Playready Reader_Decrypt does the
     ! conversion before decrypting.
     !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    /*DUMP_DATA_HEX("\tIV bef: ", pBytes,8); */
    DRM_Prdy_qwordToNetworkbytes(&pBuf[8], 0, *pIv ); /* <== conversion */

    if(blockCounter != 0){
        DRM_Prdy_qwordToNetworkbytes(&pBuf[0], 0, blockCounter );
    }
    else {
        BKNI_Memset(&pBuf[0], 0, 8);
    }

    CommonCrypto_GetDefaultClearKeySettings(&keySettings);
    BKNI_Memcpy(keySettings.settings.key, key, 16);
    keySettings.settings.keySize = 16;
    keySettings.keySlot = keySlot;

    BKNI_Memcpy( keySettings.settings.iv, &pBuf[8], 8);
    BKNI_Memcpy(&keySettings.settings.iv[8], &pBuf[0], 8);
    keySettings.settings.ivSize = 16;

    CommonCrypto_LoadClearKeyIv(cryptoHandle, &keySettings);

    if(byteOffset == 0) {

        nbBlkInHeader = 0;
        for(; ii < nDmaBlocks; ii++){
            NEXUS_DmaJob_GetDefaultBlockSettings(&blks[ii]);
            blks[ii].pSrcAddr = pBlks[ii - nbBlkInHeader].pSrcAddr;
            blks[ii].pDestAddr = pBlks[ii - nbBlkInHeader].pDestAddr;
            blks[ii].blockSize = pBlks[ii - nbBlkInHeader].blockSize;

            blks[ii].resetCrypto = false;
            blks[ii].scatterGatherCryptoStart = false;
            blks[ii].scatterGatherCryptoEnd = false;
            blks[ii].cached = true;
        }
        nb_Blks = nDmaBlocks;
        blks[0].resetCrypto = true;
        blks[0].scatterGatherCryptoStart = true;
        blks[nb_Blks-1].scatterGatherCryptoEnd = true;

    }
    else {

        NEXUS_DmaJob_GetDefaultBlockSettings(&blks[0]);
        blks[0].pSrcAddr  = g_aes_ctr_padding;
        blks[0].pDestAddr = g_aes_ctr_padding;
        blks[0].blockSize = byteOffset;
        blks[0].resetCrypto = true;
        blks[0].scatterGatherCryptoStart = true;
        blks[0].scatterGatherCryptoEnd = false;
        blks[0].cached = true;
        ii++;
        nbBlkInHeader = ii;
        nb_Blks+= nbBlkInHeader;

        for(; ii <= nDmaBlocks; ii++){

            NEXUS_DmaJob_GetDefaultBlockSettings(&blks[ii]);
            blks[ii].pSrcAddr = pBlks[ii - nbBlkInHeader].pSrcAddr;
            blks[ii].pDestAddr = pBlks[ii - nbBlkInHeader].pDestAddr;
            blks[ii].blockSize = pBlks[ii - nbBlkInHeader].blockSize;

            blks[ii].resetCrypto = false;
            blks[ii].scatterGatherCryptoStart = false;
            blks[ii].scatterGatherCryptoEnd = false;
            blks[ii].cached = true;
        }
        nb_Blks+= nDmaBlocks;
        blks[nb_Blks-1].scatterGatherCryptoEnd = true;
    }

    CommonCrypto_GetDefaultJobSettings(&jobSettings);
    jobSettings.keySlot = keySlot;

    rc = CommonCrypto_DmaXfer(cryptoHandle, &jobSettings, blks, nb_Blks);
    if(rc != NEXUS_SUCCESS)
    {
        printf("%s - CommonCrypto_DmaXfer failed rc %x\n", BSTD_FUNCTION, rc);
    }

ErrorExit:
    if( pBuf) NEXUS_Memory_Free(pBuf);
    return rc;
}


/**********************************************************************************************
 * This example perfroms basic Playready 2.5 PD operations as following:
 *
 * Preparation:
 * 1.  Use the key ID, which is extracted from the Bear_Video_OPls0, to generate the encryption
 *     key based on the MS Playready key generation algorithm and the MS test key seed;
 * 2.  Settup keyslot and manually encrypt the static sample, which contains 4 subsamples, by
 *     using the generated key from the previous step in AES CTR mode;
 * 3.  Cleanup Playready license store for the Key ID;
 *
 * License acquisition:
 * 4.  Set the pre-defined Playready Header Object(extracted from Bear_Video_OPLs0) to the
 *     Playready context;
 * 5.  Perform license acquisition by generating a license challenge using the API;
 * 6.  Perfroms HTTP POST request to Microsoft test server for the license challenge.
 * 7.  Bind the license after a successful license response from step 6;
 *
 * Decryption:
 * 8.  Decrypt the encrypted sample by using the API;
 * 9.  Verify if the clear and decrytped matches;
 *
 * 10. Test completes
 **********************************************************************************************/
/*#define DRM_PRDY_ERRORCODE_EXTENSION*/

int main(int argc, char* argv[])
{
    int testResult = -1;

#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings nxAllocSettings;
    NxClient_AllocResults allocResults;
#else
    NEXUS_PlatformSettings platformSettings;
#endif

    /* DRM_Prdy specific */
    DRM_Prdy_Init_t                prdyParamSettings;
    DRM_Prdy_Handle_t              drm=NULL;

    CommonCryptoHandle             cryptoHandle=NULL;
    NEXUS_KeySlotHandle            keySlot=NULL;
    NEXUS_DmaJobBlockSettings      blk;
    CommonCryptoSettings           cryptoSettings;
    NEXUS_SecurityKeySlotSettings  keySlotSettings;
    CommonCryptoKeyConfigSettings  algSettings;
    int                            rc = 0;
    uint8_t                        key[16] = {0};
    uint8_t                        iv[16] = {0};

    uint64_t                       blockCounter;
    size_t                         byteOffset;
    uint32_t                       i = 0;

    uint16_t                       kidBase64W[30] = {0};
    uint32_t                       kidBase64Size=30;
    char                          *urlStr = NULL;
    char                          *licChallenge = NULL;
    uint32_t                       reqURLStrSize;
    uint32_t                       reqChStrSize;

    uint32_t                       post_ret;
    uint8_t                        non_quiet= 1; /*CH_DEFAULT_NON_QUIET;   */
    uint32_t                       app_security= 150; /*CH_DEFAULT_APP_SEC; */
    uint8_t                       *licResp = NULL;
    uint8_t                       *timeChResp = NULL;
    char                          *timeChURL = NULL;
    uint32_t                       startOffset, length;
    NEXUS_MemoryAllocationSettings allocSettings;
    DRM_Prdy_DecryptSettings_t     pDecryptSettings;
    DRM_Prdy_DecryptContext_t      decryptCtx;
    uint32_t                       numOfLicDeleted = 0;
    bool                           needToCleanDecryptCxt = false;
    DRM_Prdy_AES_CTR_Info_t        aesCtrInfo;
    char                          *clientInfoStr = NULL;
    uint64_t                      *ivUint64;
    DRM_Prdy_policy_t              licPolicy;

    wchar_t               *pTimeURL=NULL;
    uint8_t               *timeCh_data=NULL;
#ifdef NETFLIX_EXT
    char                  timeURL_cstr[256] = {0};
    uint32_t              timeURL_len=0;
    uint32_t              timeCh_len=0;
    uint32_t              secClkStatus;
    time_t                mod_systemTime;
#endif
    struct timeval        tv;
    time_t                systemTime;
    uint16_t              year =0;
    uint16_t              month =0;
    uint16_t              dayOfWeek =0;
    uint16_t              day =0;
    uint16_t              hour =0;
    uint16_t              minute =0;
    uint16_t              second =0;
    uint16_t              milliseconds =0;
    uint16_t              year1 =0;
    uint16_t              month1 =0;
    uint16_t              dayOfWeek1 =0;
    uint16_t              day1 =0;
    uint16_t              hour1 =0;
    uint16_t              minute1 =0;
    uint16_t              second1 =0;
    uint16_t              milliseconds1 =0;
#ifdef DRM_PRDY_ERRORCODE_EXTENSION
    DRM_Prdy_Error_e        drm_err = DRM_Prdy_fail;
#endif


    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    printf("\n\n");

    /* init Nexus */
#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "test_prpd");
    joinSettings.ignoreStandbyRequest = true;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
    NxClient_GetDefaultAllocSettings(&nxAllocSettings);
    rc = NxClient_Alloc(&nxAllocSettings, &allocResults);
    if (rc)
        return BERR_TRACE(rc);
#else
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
#endif

    DRM_Prdy_GetDefaultParamSettings(&prdyParamSettings);

#ifndef DRM_PRDY_ERRORCODE_EXTENSION
    drm =  DRM_Prdy_Initialize( &prdyParamSettings);
    if( drm == NULL)
    {
       printf("[FAILED] - %d Failed to create drm, quitting....\n",__LINE__);
       goto clean_exit;
    }
#else
    drm =  DRM_Prdy_Initialize_Ex( &prdyParamSettings, &drm_err);
    if( drm == NULL)
    {
       printf("[FAILED] - %d Failed to create drm (converted err %d), quitting....\n",__LINE__, drm_err);
       goto clean_exit;
    }
#endif
    printf("[PASSED] - %d Created a DRM_Prdy_context.\n",__LINE__);

#ifndef NETFLIX_EXT
    printf("\n\t - %d Testing Anti-rollback clock: getting the time from Playready: \n",__LINE__);
    DRM_Prdy_GetSystemTime(drm,&year,&month,&dayOfWeek,&day,&hour,&minute,&second,&milliseconds);
    printf("\t - %d Year:(%d) Month:(%d) DayOfWeek:(%d) Day:(%d) Hour:(%d) Minute:(%d) Second:(%d) Milliseconds:(%d) \n",__LINE__,
           year,month,dayOfWeek,day,hour,minute,second,milliseconds);

    /* NOW changing the system time */
    time( &systemTime);
    printf("\t - %d The System time in kernal now in second is: %d\n",__LINE__,(int)systemTime);

    printf("\t - %d sleep for 2 seconds..\n",__LINE__);
    sleep(2);

    systemTime -= 1800;
    tv.tv_sec = systemTime;
    tv.tv_usec = 0;
    settimeofday(&tv, 0);

    time( &systemTime);
    printf("\t - %d Rolled back the system time 1800 seconds and now is : %d\n",__LINE__,(int)systemTime);
    DRM_Prdy_GetSystemTime(drm,&year1,&month1,&dayOfWeek1,&day1,&hour1,&minute1,&second1,&milliseconds1);

    printf("\t - %d Rolled back the Playready time 1 hour : %d\n",__LINE__,hour-1);
    DRM_Prdy_SetSystemTime(drm,year,month,dayOfWeek,day,hour-1,minute,second,milliseconds);

    printf("\n\t - %d Getting the time from Playready again: \n",__LINE__);
    DRM_Prdy_GetSystemTime(drm,&year1,&month1,&dayOfWeek1,&day1,&hour1,&minute1,&second1,&milliseconds1);
    printf("\t - %d Year:(%d) Month:(%d) DayOfWeek:(%d) Day:(%d) Hour:(%d) Minute:(%d) Second:(%d) Milliseconds:(%d) \n",__LINE__,
           year1,month1,dayOfWeek1,day1,hour1,minute1,second1,milliseconds1);
    if( (hour1*minute1*second1) < (hour*minute*second))
    {
       printf("[FAILED] - %d Failed to prevent the Playready clock from being modified by setting the system time\n",__LINE__);
    }
    else
    {
       printf("[PASSED] - %d Successfully prevented the Playready clock from being modified by setting the system time\n",__LINE__);
    }

    printf("\t - %d Advancing the Playready time in 2 hour : %d\n",__LINE__,(uint32_t)hour1+2);
    DRM_Prdy_SetSystemTime(drm,year1,month1,dayOfWeek1,day1,hour1+2,minute1,second1,milliseconds1);

    printf("\n\t - %d Getting the time from Playready again: \n",__LINE__);
    DRM_Prdy_GetSystemTime(drm,&year1,&month1,&dayOfWeek1,&day1,&hour1,&minute1,&second1,&milliseconds1);
    printf("\t - %d Year:(%d) Month:(%d) DayOfWeek:(%d) Day:(%d) Hour:(%d) Minute:(%d) Second:(%d) Milliseconds:(%d) \n",__LINE__,
           (uint32_t)year1,(uint32_t)month1,(uint32_t)dayOfWeek1,(uint32_t)day1,(uint32_t)hour1,(uint32_t)minute1,(uint32_t)second1,(uint32_t)milliseconds1);
    if( (hour1*minute1*second1) < (hour*minute*second))
    {
       printf("[FAILED] - %d Failed to set the Playready clock\n",__LINE__);
    }
    else
    {
       printf("[PASSED] - %d Successfully set the Playready clock\n",__LINE__);
    }

#else

    /************************** INITIALIZE THE PRDY SECURE CLOCK with Microsof Time Server **************

       We need first to set the Playready secure clock using the time obtained from the Microsoft
       time server:
       1. Getting the current state of the secure clock
       2. if the clock is not set then:
          a. Generate a time challenge via DRM_Prdy_SecureClock_GenerateChallenge()
          b. Get the Secure Time Server petition URL from the ressult of #a or from the device certificate
          c. Submit a petition request to the petition server URL (obtained form the #b) via http GET request
          d. If the petition responds with redirection(301 or 302), extract the redirection URL from
             the response and perform step #c again with the redirection URL. Otherwise prcess next step.
          e. If the petition responds with "http 200", then extract the time challenge server URL from
             the http response body and proceed the next step #f. Othewise return error.
          f. send http POST for the time challenge generated in step #a to the time challenge URL
             obtained from step #e.
          g. If the http POST responds success, call DRM_Prdy_SecureClock_ProcessResponse() to process the
             http POST respose.
          h. if no error from step #g, the Playready secure clock is successfully set.

     ****************************************************************************************************/

    /* 1. Getting the current state of the secure clock*/
    if( DRM_Prdy_SecureClock_GetStatus( drm,&secClkStatus) !=  DRM_Prdy_ok)
    {
       printf("[FAILED] - %d DRM_Prdy_SecureClock_GetStatus failed, quitting....\n",__LINE__);
       goto clean_exit;
    }

    printf("[PASSED] - %d DRM_Prdy_SecureClock_GetStatus succsss, status %d\n",__LINE__,secClkStatus);
    if( secClkStatus == DRM_PRDY_CLK_SET)
    {
       printf("[WARNING] - %d For some reason the secure clock has been set, reset the clock again...\n",__LINE__);
    }
    else if( secClkStatus == DRM_PRDY_CLK_NOT_SET)
    {
       printf("\tThe secure clock hasn't been set, proceed with the time challenge...\n");
    }
    else
    {
       printf("[WARNING] - %d resetting the clock anyway...\n",__LINE__);
    }

    /* 2.a. Generate a time challenge  DRM_Prdy_SecureClock_GenerateChallenge() */
    if( DRM_Prdy_SecureClock_GenerateChallenge(drm,NULL,&timeURL_len,NULL,&timeCh_len) != DRM_Prdy_buffer_size)
    {
       printf("[FAILED] - %d DRM_Prdy_SecureClock_GenerateChallenge Failed, quitting....\n",__LINE__);
    }
    else
    {
       bool redirect = true;
       int32_t petRC=0;
       uint32_t petRespCode = 0;
       printf("[PASSED] - %d DRM_Prdy_SecureClock_GenerateChallenge succeeded.\n",__LINE__);
       printf("\tTime Server URL length %d, challenge len %d\n",timeURL_len,timeCh_len);

       rc = NEXUS_Memory_Allocate(timeURL_len*sizeof(wchar_t), NULL, (void **)(&pTimeURL));
       if(rc != NEXUS_SUCCESS)
       {
           printf("[FAILED] - %d NEXUS_Memory_Allocate failed for the Time server URL buffer, rc = %d\n",__LINE__, rc);
           goto clean_exit;
       }

       rc = NEXUS_Memory_Allocate(timeCh_len+1, NULL, (void **)(&timeCh_data));
       if(rc != NEXUS_SUCCESS)
       {
           printf("[FAILED] - %d NEXUS_Memory_Allocate failed for the time challenge buffer, rc = %d\n",__LINE__, rc);
           goto clean_exit;
       }

       BKNI_Memset(pTimeURL, 0, timeURL_len);
       BKNI_Memset(timeCh_data, 0, timeCh_len+1);

       if( DRM_Prdy_SecureClock_GenerateChallenge(drm,pTimeURL,&timeURL_len,timeCh_data,&timeCh_len) != DRM_Prdy_ok)
       {
           printf("[FAILED] - %d DRM_Prdy_SecureClock_GenerateChallenge failed.\n",__LINE__);
           goto clean_exit;
       }

       timeURL_cstr[timeURL_len] = 0; /* null terminator */
       if( !DRM_Prdy_convertWStringToCString( pTimeURL, timeURL_cstr, timeURL_len))
       {
           printf("[FAILED] - %d DRM_Prdy_convertWStringToCString failed to convert URL from wchar to char *, can't procceed time challenge.\n",__LINE__);
           goto clean_exit;
       }

       printf("\tTime challenge petition server URL: %s\n",timeURL_cstr);

       timeCh_data[timeCh_len] = 0;

       NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
       rc = NEXUS_Memory_Allocate(MAX_URL_LENGTH, &allocSettings, (void **)(&timeChURL ));
       if(rc != NEXUS_SUCCESS)
       {
           printf("[FAILED] - %d NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n",__LINE__, rc);
           goto TEST_GENEATE_SEED;
       }

       do
       {
           redirect = false;

           /* send the petition request to Microsoft with HTTP GET */
           petRC = DRM_Prdy_http_client_get_petition( timeURL_cstr,
                                                      &petRespCode,
                                                      (char**)&timeChURL);
           if( petRC != 0)
           {
               printf("[FAILED] - %d Secure Clock Petition request failed, rc = %d\n",__LINE__, petRC);
               goto TEST_GENEATE_SEED;
           }

           printf("[PASSED] - %d Time petition request sent successfully, rc = %d, with RespCode = %d\n",__LINE__,petRC,petRespCode);

           /* we need to check if the Pettion responded with redirection */

           if( petRespCode == 200)
           {
               redirect = false;
           }
           else if( petRespCode == 302 || petRespCode == 301)
           {
               char * isHttps = NULL;

               printf("\tPetition responded with redirection. The new redirect petition URL: \n\t%s\n",timeChURL);

               redirect = true;

               memset(timeURL_cstr,0,timeURL_len);

               /* check if the URL is "https" */
               isHttps = strstr(timeChURL,"https");
               if( isHttps )
               {
                   strcpy(timeURL_cstr,"http");
                   strcpy(timeURL_cstr+4,isHttps+5);
               }
               else
               {
                   strcpy(timeURL_cstr,timeChURL);
               }

               memset(timeChURL,0,MAX_URL_LENGTH);
               printf("\tRe-sending petition to URL: %s\n",timeURL_cstr);
           }
           else
           {
               printf("[FAILED] - %d Secure Clock Petition responded with unsupported result, rc = %d, can't get the tiem challenge URL\n",__LINE__, petRespCode);
               goto TEST_GENEATE_SEED;
           }

       } while (redirect);

       printf("[PASSED] - %d Petition requests succeeded. The responded time challage server URL:\n\t%s\n",__LINE__,timeChURL);

       NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
       rc = NEXUS_Memory_Allocate(MAX_TIME_CHALLENGE_RESPONSE_LENGTH, &allocSettings, (void **)(&timeChResp ));
       if(rc != NEXUS_SUCCESS)
       {
           printf("[FAILED] - %d NEXUS_Memory_Allocate failed for time challenge response buffer, rc = %d\n",__LINE__, rc);
           goto TEST_GENEATE_SEED;
       }

       BKNI_Memset(timeChResp, 0, MAX_TIME_CHALLENGE_RESPONSE_LENGTH);

       post_ret = DRM_Prdy_http_client_time_challenge_post(timeChURL,
                                                           (char *)timeCh_data,
                                                           non_quiet,
                                                           app_security,
                                                           (unsigned char**)&(timeChResp),
							   MAX_TIME_CHALLENGE_RESPONSE_LENGTH,
                                                           &startOffset,
                                                           &length);
       if( post_ret != 0)
       {
           printf("[FAILED] - %d Secure Clock Challenge request failed, rc = %d\n",__LINE__, post_ret);
           goto TEST_GENEATE_SEED;
       }

       printf("[PASSED] - %d DRM_Prdy_http_client_time_challenge_post succeeded for Secure Clock Challenge with response size = %d.\n",__LINE__,length);

       if( DRM_Prdy_SecureClock_ProcessResponse( drm, (uint8_t *) timeChResp, length) != DRM_Prdy_ok)
       {
           printf("[FAILED] - %d Secure Clock Process Challenge response failed\n",__LINE__);
           goto TEST_GENEATE_SEED;
       }

       printf("[PASSED] - %d Secure Clock Process Challenge response success. \n",__LINE__);

       /* NOW testing the system time */
       time( &systemTime);
       printf("\t - %d The System time in kernal now in second is: %d\n",__LINE__,(uint32_t)systemTime);

       DRM_Prdy_GetSystemTime(drm,&year,&month,&dayOfWeek,&day,&hour,&minute,&second,&milliseconds);
       printf("\t - %d Year: %d Month: %d DayOfWeek: %d Day: %d Hour: %d Minute: %d Second: %d Milliseconds: %d \n",__LINE__,
               (uint32_t)year,
               (uint32_t)month,
               (uint32_t)dayOfWeek,
               (uint32_t)day,
               (uint32_t)hour,
               (uint32_t)minute,
               (uint32_t)second,
               (uint32_t)milliseconds);


       systemTime -= 1800;
       tv.tv_sec = systemTime;
       tv.tv_usec = 0;
       settimeofday(&tv, 0);
       time( &mod_systemTime);

       printf("\t - %d Rollback System time for 30 minutes: %d\n",__LINE__,(uint32_t)mod_systemTime);

       printf("\t - %d Getting the time from Playready: \n",__LINE__);
       DRM_Prdy_GetSystemTime(drm,&year,&month,&dayOfWeek,&day,&hour,&minute,&second,&milliseconds);
       printf("\t - %d Year: %d Month: %d DayOfWeek: %d Day: %d Hour: %d Minute: %d Second: %d Milliseconds: %d \n",__LINE__,
               (uint32_t)year,
               (uint32_t)month,
               (uint32_t)dayOfWeek,
               (uint32_t)day,
               (uint32_t)hour,
               (uint32_t)minute,
               (uint32_t)second,
               (uint32_t)milliseconds);
       time( &systemTime);
       printf("\t - %d The System time in kernal now in second is: %d\n",__LINE__,(uint32_t)systemTime);

    }
TEST_GENEATE_SEED:
#endif


    /* generate a random IV */
    if( gen_random_num(16, iv) != 0) {
        printf("[FAILED] - %d Failed to generate IV.\n",__LINE__);
        goto clean_exit;
    }

#ifdef NETFLIX_EXT
	/* enable secure stop */
	if ( DRM_Prdy_TurnSecureStop(drm, 1) ) {
	    printf("[FAILED] - %d Failed to enable Secure Stop \n",__LINE__);
	}
	else{
		bIsSecureStopEnabled = true;
	}
#endif

    /* initialize the key slot and DMA blocks */
    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    cryptoHandle = CommonCrypto_Open(&cryptoSettings);
    if( cryptoHandle == NULL) {
        rc = -1;
        goto clean_exit;
    }

    /* Allocate key slot for AES Counter mode */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

    keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if(keySlot == NULL) {
        printf("[FAILED] - %d Failure to allocate key slot.\n", __LINE__);
        rc = -1;
        goto clean_exit;
    }

    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = keySlot;
    algSettings.settings.opType = NEXUS_SecurityOperation_eEncrypt;
    algSettings.settings.algType = NEXUS_SecurityAlgorithm_eAes128;
    algSettings.settings.algVariant = NEXUS_SecurityAlgorithmVariant_eCounter;
    algSettings.settings.termMode = NEXUS_SecurityTerminationMode_eClear;
    algSettings.settings.aesCounterMode = NEXUS_SecurityCounterMode_eGenericAllBlocks;
    algSettings.settings.enableExtKey = false;
    algSettings.settings.enableExtIv = false;
    /* always assume IV size 8 */
    algSettings.settings.aesCounterSize = NEXUS_SecurityAesCounterSize_e64Bits;

    /* Configure key slot for AES Counter mode */
    if(CommonCrypto_LoadKeyConfig( cryptoHandle, &algSettings) != NEXUS_SUCCESS) {
        printf("[FAILED] - %d CommonCrypto_ConfigAlg failed aes ctr\n", __LINE__);
        rc = -1;
    }

    /* generate key from MS seed */
    if( gen_key_from_seed(g_keyID,key) != 0) {
        printf("[FAILED] - %d Failed to generate a content key.", __LINE__);
        goto clean_exit;
    }

    /* Allocates 16 bytes of padding.*/
    rc = NEXUS_Memory_Allocate(16, NULL, (void **)(&g_aes_ctr_padding));
    if(rc != NEXUS_SUCCESS)
    {
        printf("[FAILED] - %d NEXUS_Memory_Allocate failed, rc = %d\n", __LINE__, rc);
        goto clean_exit;
    }
    BKNI_Memset(g_aes_ctr_padding, 0, 16);

    NEXUS_DmaJob_GetDefaultBlockSettings(&blk);

    /* Encrypt the sample in aes counter mode.
     * we are encrypting 4 subsamples using the
     * same IV to test the Reader_Decrypt API
     * later on. */
    blockCounter = 0;
    byteOffset   = 0;
    ivUint64 = (uint64_t *) &iv[8];
    for( i=0; i<4; ++i)
    {
        size_t s = strlen( g_sample[i]);
        rc = NEXUS_Memory_Allocate(s, NULL, (void *)&g_enc_sample[i]);
        if(rc != NEXUS_SUCCESS){
            printf("[FAILED] - %d NEXUS_Memory_Allocate failed, rc = %d\n",__LINE__, rc);
            goto clean_exit;
        }

        BKNI_Memset(g_enc_sample[i], 0, s);
        BKNI_Memcpy(g_enc_sample[i], g_sample[i], s);

        blk.pSrcAddr = g_enc_sample[i];
        blk.pDestAddr = g_enc_sample[i];
        blk.blockSize = s;
        if( encrypt_sample(key,ivUint64,blockCounter,byteOffset,&blk,1,cryptoHandle,keySlot) != 0)
        {
            printf("[FAILED] - %d Failed to encrypt sample.\n",__LINE__);
            goto clean_exit;
        }
        blockCounter = s / 16;
        byteOffset = s % 16;
        printf("\t - %d Subsample[%d] encrypted.\n",__LINE__,i);
    }

    /* We're now ready to test the PD operations */

    /* First cleanup any expired license */
    if( DRM_Prdy_Cleanup_LicenseStores( drm) != DRM_Prdy_ok)
    {
       printf("[WARNING] - %d Failed to cleanup the license store.\n",__LINE__);
    }
    printf("[PASSED] - %d cleanup the license store.\n",__LINE__);

    /* Set the Key ID to the context before sending lincese challenge */
    /* convert the Key ID to base64 wstring */
    if( DRM_Prdy_B64_EncodeW((uint8_t *)g_keyID,16,kidBase64W,&kidBase64Size) !=  DRM_Prdy_ok)
    {
        printf("[FAILED] - %d Failed to convert KID into base64W format \n",__LINE__);
        goto clean_exit;
    }
    /*DUMP_DATA_HEX("\tKEY Base64  ",kidBase64W,kidBase64Size); */

    /* delete also the associated license for the Key ID if existed */
    if( DRM_Prdy_StoreMgmt_DeleteLicenses(
                drm,
                kidBase64W,
                kidBase64Size,
                &numOfLicDeleted ) != DRM_Prdy_ok )
    {
       printf("[WARNING] - %d DRM_Prdy_StoreMgmt_DeleteLicenses failed.\n",__LINE__);
    }
    printf("[PASSED] - %d DeleteLicenses succeeded, # = %d.\n",__LINE__,numOfLicDeleted);

    /* Set the Playready Header Object for the content */
    if(  DRM_Prdy_Content_SetProperty( drm,
                                       DRM_Prdy_contentSetProperty_eAutoDetectHeader,
                                       (uint8_t *) g_prdyHeaderObj,
                                       sizeof( g_prdyHeaderObj )) != DRM_Prdy_ok )
    {
        printf("[FAILED] - %d Failed to SetProperty for the Playready Header Object.\n",__LINE__);
        goto clean_exit;
    }

    printf("[PASSED] - %d DRM_Prdy_Content_SetProperty for Playready Header Object with size = %d.\n",
            __LINE__,sizeof(g_prdyHeaderObj));

    BKNI_Memset(&decryptCtx, 0, sizeof(DRM_Prdy_DecryptContext_t));
    /* test to bind the license, it should fail since the license doesn't exist yet */
    if( DRM_Prdy_Reader_Bind( drm, &decryptCtx)!= DRM_Prdy_ok )
    {
        printf("[PASSED] - %d Reader_Bind failed...expecting, move on...\n",__LINE__);
    }
    else
    {
        printf("[WARNING] - %d found the license, move on...\n",__LINE__);
        if( DRM_Prdy_Reader_Close( &decryptCtx) != DRM_Prdy_ok )
        {
            printf("[FAILED] - %d DRM_Prdy_Reader_Close failed.\n",__LINE__);
            goto clean_exit;
        }
    }

    /* Prepare the license challenge */
    /* first, determine the size of the challenge and the URL string */
#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
        if( DRM_Prdy_Get_Buffer_Size( drm,
                                      DRM_Prdy_getBuffer_licenseAcq_challenge_Netflix,
                                      NULL,
                                      0,
                                     &reqURLStrSize,
                                     &reqChStrSize) != DRM_Prdy_ok )
        {
            printf("[FAILED] - %d Failed to determine the sizes of the URL string and the Challenge message.\n",__LINE__);
            goto clean_exit;
        }
    } else {
        if( DRM_Prdy_Get_Buffer_Size( drm,
                                      DRM_Prdy_getBuffer_licenseAcq_challenge,
                                      NULL,
                                      0,
                                     &reqURLStrSize,
                                     &reqChStrSize) != DRM_Prdy_ok )
        {
            printf("[FAILED] - %d Failed to determine the sizes of the URL string and the Challenge message.\n",__LINE__);
            goto clean_exit;
        }
    }
#else
    if( DRM_Prdy_Get_Buffer_Size( drm,
                                  DRM_Prdy_getBuffer_licenseAcq_challenge,
                                  NULL,
                                  0,
                                 &reqURLStrSize,
                                 &reqChStrSize) != DRM_Prdy_ok )
    {
        printf("[FAILED] - %d Failed to determine the sizes of the URL string and the Challenge message.\n",__LINE__);
        goto clean_exit;
    }
#endif

    printf("[PASSED] - %d DRM_Prdy_Get_Buffer_Size with required sizes of the URL = %d and Challenge = %d.\n",
            __LINE__,reqURLStrSize,reqChStrSize);

    rc = NEXUS_Memory_Allocate(reqURLStrSize, NULL, (void **)(&urlStr));
    if(rc != NEXUS_SUCCESS)
    {
        printf("[FAILED] - %d NEXUS_Memory_Allocate failed for the URL buffer, rc = %d\n",__LINE__, rc);
        goto clean_exit;
    }

    rc = NEXUS_Memory_Allocate(reqChStrSize+1, NULL, (void **)(&licChallenge));
    if(rc != NEXUS_SUCCESS)
    {
        printf("[FAILED] - %d NEXUS_Memory_Allocate failed for the license challenge buffer, rc = %d\n",__LINE__, rc);
        goto clean_exit;
    }
    BKNI_Memset(licChallenge, 0, reqChStrSize+1);

    /* call the API to generate the license challenge */
#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
        uint8_t    tNounce[DRM_PRDY_SESSION_ID_LEN];
        if( DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix( drm,
                                                   NULL,
                                                   0,
                                                   urlStr,
                                                   &reqURLStrSize,
                                                   licChallenge,
                                                   &reqChStrSize,
                                                   tNounce,
                                                   false ) != DRM_Prdy_ok )
        {
            printf("[FAILED] - %d Failed to generate license challenge .\n",__LINE__);
            goto clean_exit;
        }
	} else {
        if( DRM_Prdy_LicenseAcq_GenerateChallenge( drm,
                                                   NULL,
                                                   0,
                                                   urlStr,
                                                   &reqURLStrSize,
                                                   licChallenge,
                                                   &reqChStrSize) != DRM_Prdy_ok )
        {
            printf("[FAILED] - %d Failed to generate license challenge .\n",__LINE__);
            goto clean_exit;
        }
	}
#else
    if( DRM_Prdy_LicenseAcq_GenerateChallenge( drm,
                                               NULL,
                                               0,
                                               urlStr,
                                               &reqURLStrSize,
                                               licChallenge,
                                               &reqChStrSize) != DRM_Prdy_ok )
    {
        printf("[FAILED] - %d Failed to generate license challenge .\n",__LINE__);
        goto clean_exit;
    }
#endif
    printf("[PASSED] - %d DRM_Prdy_LicenseAcq_GenerateChallenge. \n",__LINE__);
    printf("\tURL: %s, the actual szie of the challenge: %d\n",urlStr,reqChStrSize);

    /*
     * IMPORTANT! The HTTP request will fail at the server side if the challenge string
     * doesn't end properly. Because Playready doesn't set the null terminator to the
     * end of the generated challenge string, that we need to set it manually. Also, the
     * actual size of the challenge is the one being generated which is returned from
     * the DRM_Prdy_LicenseAcq_GenerateChallenge(), not the one from the
     * DRM_Prdy_Get_Buffer_Size().
     */
    licChallenge[reqChStrSize] = 0;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    rc = NEXUS_Memory_Allocate(MAX_LICENCE_RESPONSE_LENGTH, &allocSettings, (void **)(&licResp ));
    if(rc != NEXUS_SUCCESS)
    {
        printf("[FAILED] - %d NEXUS_Memory_Allocate failed for license response buffer, rc = %d\n",__LINE__, rc);
        goto clean_exit;
    }

    /* send the challenge with HTTP POST */
    post_ret = DRM_Prdy_http_client_license_post_soap ((char *)urlStr,
                                                       (char *)licChallenge,
                                                       non_quiet,
                                                       app_security,
                                                       (unsigned char**)&(licResp),
						       MAX_LICENCE_RESPONSE_LENGTH,
                                                       &startOffset,
                                                       &length);
    if( post_ret != 0)
    {
        printf("[FAILED] - %d License request failed, rc = %d\n",__LINE__, post_ret);
        goto clean_exit;
    }
    printf("[PASSED] - %d DRM_Prdy_http_client_license_post_soap succeeded with response size = %d.\n",__LINE__,length);

#ifdef NETFLIX_EXT
	if (bIsSecureStopEnabled == true) {
		if( DRM_Prdy_LicenseAcq_ProcessResponse_SecStop( drm,
												 (char *)licResp,
												 length, sSessionIdBuf, NULL) != DRM_Prdy_ok )
		{
			printf("[FAILED] - %d Failed to process license response with SessionID buffer.\n",__LINE__);
			goto clean_exit;
		}
	}
	else {
		if( DRM_Prdy_LicenseAcq_ProcessResponse( drm,
												 (char *)licResp,
												 length, NULL) != DRM_Prdy_ok )
		{
			printf("[FAILED] - %d Failed to process license response.\n",__LINE__);
			goto clean_exit;
		}
	}
#else
    if( DRM_Prdy_LicenseAcq_ProcessResponse( drm,
                                             (char *)licResp,
                                             length, NULL) != DRM_Prdy_ok )
    {
        printf("[FAILED] - %d Failed to process license response.\n",__LINE__);
        goto clean_exit;
    }
#endif

    printf("[PASSED] - %d DRM_Prdy_LicenseAcq_ProcessResponse succeeded. \n",__LINE__);

    /* now we should be able to bind the license to the content */

    /* initialize the DecryptContext */
    DRM_Prdy_GetDefaultDecryptSettings( &pDecryptSettings);
    if( DRM_Prdy_SetDecryptContext ( &pDecryptSettings,
                                     &decryptCtx ) != DRM_Prdy_ok )
    {
        printf("[FAILED] - %d Set Decrypt Context, exiting...\n",__LINE__);
        goto clean_exit;
    }
    printf("[PASSED] - %d SetDecryptContext.\n",__LINE__);

#ifdef NETFLIX_EXT
    if( DRM_Prdy_Reader_Bind_Netflix(drm, sSessionIdBuf, &decryptCtx)!= DRM_Prdy_ok )
    {
        printf("[FAILED] %d - Reader_Bind_Netflix failed.\n",__LINE__);
        goto clean_exit;
    }

    printf("[PASSED] - %d DRM_Prdy_Reader_Bind_Netflix.\n",__LINE__);
#else
    if( DRM_Prdy_Reader_Bind(drm, &decryptCtx)!= DRM_Prdy_ok )
    {
        printf("[FAILED] %d - Reader_Bind failed.\n",__LINE__);
        goto clean_exit;
    }

    printf("[PASSED] - %d DRM_Prdy_Reader_Bind.\n",__LINE__);
#endif

    if( DRM_Prdy_Get_Protection_Policy( drm, &licPolicy) != DRM_Prdy_ok)
    {
        printf("[FAILED] - %d Get Protection Policy failed, exiting...\n",__LINE__);
    }
    else
    {
        printf("[PASSED] - %d DRM_Prdy_Get_Protection_Policy:\n",__LINE__);
        printf("\t\tCompressed DigitalVideo   : %d\n",licPolicy.t.play.minOPL.wCompressedDigitalVideo);
        printf("\t\tUnCompressed DigitalVideo : %d\n",licPolicy.t.play.minOPL.wUncompressedDigitalVideo);
        printf("\t\tAnalogVideo               : %d\n",licPolicy.t.play.minOPL.wAnalogVideo);
        printf("\t\tCompressedDigitalAudio    : %d\n",licPolicy.t.play.minOPL.wCompressedDigitalAudio);
        printf("\t\tUncompressedDigitalAudio  : %d\n",licPolicy.t.play.minOPL.wUncompressedDigitalAudio);
    }

    needToCleanDecryptCxt = true; /* set the flag to true for cleaning the context later */

    /* we can decrypt the sample now */
    BKNI_Memcpy( &aesCtrInfo.qwInitializationVector,&iv[8],8); /* IV is always having size of 8 */
    aesCtrInfo.qwBlockOffset = 0;
    aesCtrInfo.bByteOffset = 0;
    for( i=0; i<4; ++i)
    {
        size_t num_decrypt = strlen( g_sample[i]);
        if(DRM_Prdy_Reader_Decrypt(
                            &decryptCtx,
                            &aesCtrInfo,
                            (uint8_t *) g_enc_sample[i],
                            num_decrypt ) != DRM_Prdy_ok)
        {
            printf("[FAILED] - %d Reader_Decrypt failed on sample# %d.\n",__LINE__,i);
            /*DUMP_DATA_HEX("\tdecrypted sample: ",g_enc_sample[i],num_decrypt); */
            goto clean_exit;
        }

        aesCtrInfo.qwBlockOffset = num_decrypt / 16;
        aesCtrInfo.bByteOffset   = num_decrypt % 16;
#ifndef SAGE_ENABLE
        /* if SAGE is enable, we can't access the decryption buffer, and simply return */
        if(BKNI_Memcmp(g_enc_sample[i], g_sample[i], num_decrypt) != 0){
            printf("[FAILED] - %d BKNI_Memcmp failed on sample# %d.\n",__LINE__,i);
            goto clean_exit;
        }

        printf("[PASSED] - %d decrypted subsample[%d] matched.\n",__LINE__,i);
#endif
    }

    printf("[PASSED] - %d Reader_Decrypt succeeded.\n",__LINE__);

    printf("\n[PASSED]  - All tests completed.\n\n");

    testResult = 0;

clean_exit:

    if( g_aes_ctr_padding) NEXUS_Memory_Free(g_aes_ctr_padding);

    for( i=0; i<4; ++i) {
        if(g_enc_sample[i] != NULL)
            NEXUS_Memory_Free(g_enc_sample[i]);
    }

    if( keySlot)
        NEXUS_Security_FreeKeySlot(keySlot);

    if( cryptoHandle)
        CommonCrypto_Close(cryptoHandle);

    if( urlStr != NULL)
        NEXUS_Memory_Free(urlStr);

    if( clientInfoStr != NULL)
        NEXUS_Memory_Free(clientInfoStr);

    if( licChallenge != NULL)
        NEXUS_Memory_Free(licChallenge);

    if( licResp  != NULL)
        NEXUS_Memory_Free(licResp );

    if( timeChResp   != NULL)
        NEXUS_Memory_Free(timeChResp );

    if( timeChURL    != NULL)
        NEXUS_Memory_Free(timeChURL  );

    if( pTimeURL  != NULL)
        NEXUS_Memory_Free(pTimeURL );

    if( timeCh_data  != NULL)
        NEXUS_Memory_Free(timeCh_data );

    if(needToCleanDecryptCxt)
    {
        DRM_Prdy_Reader_Close( &decryptCtx);
    }

#ifdef NETFLIX_EXT
	{
		DRM_Prdy_Error_e	err;
	    char 				hash[256];
		uint32_t			i;
		printf("Calling DRM_Prdy_GetSecureStoreHash\n");
		err = DRM_Prdy_GetSecureStoreHash(hash);
		if (err == DRM_Prdy_ok)
		{
			for (i = 0; i < 256; i++)
			{
				printf("%02x ", hash[i]);
			}
		}
		else
		{
			printf("DRM_Prdy_GetSecureStoreHash returned error: %d\n", err);
		}
		printf("Calling DRM_Prdy_GetKeyStoreHash\n");
		err = DRM_Prdy_GetKeyStoreHash(drm,hash);
		if (err == DRM_Prdy_ok)
		{
			for (i = 0; i < 256; i++)
			{
				printf("%02x ", hash[i]);
			}
		}
		else
		{
			printf("DRM_Prdy_GetKeyStoreHash returned error: %d\n", err);
		}
		printf("Calling DRM_Prdy_DeleteSecureStore\n");
		err = DRM_Prdy_DeleteSecureStore();
		if (err != DRM_Prdy_ok)
		{
			printf("DRM_Prdy_DeleteSecureStore returned error: %d\n", err);
		}
		printf("Calling DRM_Prdy_DeleteKeyStore\n");
		err = DRM_Prdy_DeleteKeyStore(drm);
		if (err != DRM_Prdy_ok)
		{
			printf("DRM_Prdy_DeleteKeyStore returned error: %d\n", err);
		}
		printf("Calling DRM_Prdy_GetSecureStoreHash\n");
		err = DRM_Prdy_GetSecureStoreHash(hash);
		if (err == DRM_Prdy_ok)
		{
			for (i = 0; i < 256; i++)
			{
				printf("%02x ", hash[i]);
			}
		}
		else
		{
			printf("DRM_Prdy_GetSecureStoreHash returned error: %d\n", err);
		}
		printf("Calling DRM_Prdy_GetKeyStoreHash\n");
		err = DRM_Prdy_GetKeyStoreHash(drm,hash);
		if (err == DRM_Prdy_ok)
		{
			for (i = 0; i < 256; i++)
			{
				printf("%02x ", hash[i]);
			}
		}
		else
		{
			printf("DRM_Prdy_GetKeyStoreHash returned error: %d\n", err);
		}
	}
	if (bIsSecureStopEnabled == true)
	{
		uint8_t             *pSecureStop = NULL;
		uint16_t 			dataSize = 0;
		uint8_t 			dummy;
		uint8_t 			sessionIds[DRM_PRDY_MAX_NUM_SECURE_STOPS][DRM_PRDY_SESSION_ID_LEN];
		uint32_t 			count = 0;
		uint32_t			i;
		DRM_Prdy_Error_e	err;

		/* Get the list of Secure Stop Session IDs that are ready for release */
		err = DRM_Prdy_GetSecureStopIds(drm, sessionIds, &count);
		if (err == DRM_Prdy_ok)
		{
			int j;
			printf("DRM_Prdy_GetSecureStopIds retrieved the following %ld session ID(s): \n", count);
			for (i = 0; i < count; i++)
			{
				for (j = 0; j < DRM_PRDY_SESSION_ID_LEN; j++)
				{
						printf("%02x ", sessionIds[i][j]);
				}
				printf("\n");
			}
		}
		else
		{
			printf("DRM_Prdy_GetSecureStopIds failed with error: %d\n", err);
		}

		printf("Actual session ID is: \n");
		for (i = 0; i < DRM_PRDY_SESSION_ID_LEN; i++)
		{
			printf("%02x ", sSessionIdBuf[i]);
		}
		printf("\n");

		/* call once with zero size to determine actual size of secure stop */
		err = DRM_Prdy_GetSecureStop(drm, sSessionIdBuf, &dummy, &dataSize);
		if (err != DRM_Prdy_buffer_size)
		{
			printf("DRM_Prdy_GetSecureStop failed input data size = 0 with error: %d\n", err);
		}
		else
		{
			if ( NEXUS_SUCCESS == NEXUS_Memory_Allocate(dataSize, NULL, (void **)(&pSecureStop)) )
			{
				/* now get the secure stop */
				err = DRM_Prdy_GetSecureStop(drm, sSessionIdBuf, pSecureStop, &dataSize);
				if (err != DRM_Prdy_ok)
				{
					printf("DRM_Prdy_GetSecureStop failed with error %d\n", err);
				}
				NEXUS_Memory_Free(pSecureStop);
			}
		}
	}
#endif

    if( drm != NULL) DRM_Prdy_Uninitialize(drm);

#ifdef NXCLIENT_SUPPORT
    NxClient_Free(&allocResults);
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif

    return testResult;
}
