/***************************************************************************
*     (c)2003-2009 Broadcom Corporation
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
* Description: RAD/EA module header file
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _B_PLAYBACK_IP_RAD_EA_H_
#define _B_PLAYBACK_IP_RAD_EA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEF_RAD_VERSION           3

#define RAD_BYTES_PER_BLOCK       (128)                                        // bytes in a rad block
#define EA_BYTES_PER_BLOCK        (1)                                          // bytes in an ea block
#define DECODE_BYTES_PER_BLOCK    (RAD_BYTES_PER_BLOCK + EA_BYTES_PER_BLOCK)   // bytes in a decoded block

#define BLOCKS_PER_FRAME_V3       (24)                                         // blocks in a frame
#define BLOCKS_PER_FRAME_V2       (8)                                          // blocks in a frame
#define BLOCKS_PER_FRAME          BLOCKS_PER_FRAME_V3                          // blocks in a frame

#define MIN_RAD_BYTES_FOR_DES     (8)                                          
#define RAD_BYTES_PER_FRAME       (RAD_BYTES_PER_BLOCK * BLOCKS_PER_FRAME)     // bytes in a rad frame
#define EA_BYTES_PER_FRAME        (EA_BYTES_PER_BLOCK * BLOCKS_PER_FRAME)      // bytes in an ea frame
#define MIN_EA_BYTES_FOR_DES      (EA_BYTES_PER_FRAME)

#define DECODE_BYTES_PER_FRAME    (DECODE_BYTES_PER_BLOCK * BLOCKS_PER_FRAME)  // bytes in a decoded frame

#define LISTEN_VERIFY_NUM_CHARS   (4)      // number of chars in header for LISTEN_VERIFY
#define LISTEN_VERIFY             "TULI"   // chars at beginning of header
#define RAD_VERIFY_NUM_CHARS      (4)      // number of chars in header for RAD_VERIFY
#define RAD_VERIFY                "RAD_"   // next chars in RAD header
#define EA_VERIFY_NUM_CHARS       (4)      // number of chars in header for EA_VERIFY
#define EA_VERIFY                 "EA__"   // next chars in EA header
#define RAD_MIN_BYTES_TO_FOLLOW   (24)     // minimum bytes in RAD header after the remaining header length parameter
#define EA_MIN_BYTES_TO_FOLLOW    (24)     // minimum bytes in EA header after the remaining header length parameter
#define RAD_MAJOR_VERSION             (3)      // major RAD version
#define RAD_MINOR_VERSION             (0)      // minor RAD version
//
// byte align for easy load.
#pragma pack(push, radea_headers, 1)
struct EA_HEADER {
  int   nSize;
  short sMajorVersion;
  union {
    struct {
      short      sMediaID;
      int        nToComplete;
      char       ID3Tag[128];
    } v1;
    struct {
      int        nMediaID;
      int        nMediaCodec;
      int        nMediaFormat;
      int        nRadToComplete;
      int        nValidationData;
    } v2;
    struct {
      short      sMinorVersion;
      int        nMediaID;
      int        nMediaCodec;
      int        nMediaFormat;
      int        nValidationData;
      int        nFileSize;
    } v3;
  } Header;
};

struct RAD_HEADER {
  int   nSize;
  short sMajorVersion;
  union {
    struct {
      short      sMediaID;
      int        nRadFileSize;
    } v1;
    struct {
      int        nMediaID;
      int        nMediaCodec;
      int        nMediaFormat;
      int        nRadFileSize;
    } v2;
    struct {
      short      sMinorVersion;
      int        nMediaID;
      int        nMediaCodec;
      int        nMediaFormat;
      int        nToComplete;
      int        nRadFileSize;
    } v3;
  } Header;
};

#pragma pack(pop, radea_headers)

#endif /* _B_PLAYBACK_IP_RAD_EA_H_ */
