/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BUDP_VCE_H_
#define BUDP_VCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bavc_types.h"
#include "budp_dccparse.h"

typedef struct BUDP_Encoder_Payload_CC
{
   unsigned uiNumLines;
   BUDP_DCCparse_ccdata *astLine;
} BUDP_Encoder_Payload_CC;

typedef struct BUDP_Encoder_PacketDescriptor_DVS157
{
   BUDP_Encoder_Payload_CC stCC;
} BUDP_Encoder_PacketDescriptor_DVS157;

typedef struct BUDP_Encoder_PacketDescriptor_DVS053
{
   BUDP_Encoder_Payload_CC stCC;
} BUDP_Encoder_PacketDescriptor_DVS053;

typedef struct BUDP_Encoder_PacketDescriptor_ATSC53
{
   BUDP_Encoder_Payload_CC stCC;
} BUDP_Encoder_PacketDescriptor_ATSC53;

/* BUDP_Encoder_PacketDescriptor contains the information necessary to create a single user data packet of ePacketType */
typedef struct BUDP_Encoder_PacketDescriptor
{
   BUDP_DCCparse_Format ePacketFormat; /* Indicates which user data packet format the encoder should
                                        * use when inserting this packet into the ES
                                        */

   union
   {
      BUDP_Encoder_PacketDescriptor_DVS157 stDvs157;
      BUDP_Encoder_PacketDescriptor_DVS053 stDvs053;
      BUDP_Encoder_PacketDescriptor_ATSC53 stAtsc53;
   } data;
} BUDP_Encoder_PacketDescriptor;

/* BUDP_Encoder_FieldInfo - contains ALL the user data packet(s) associated with a single field */
typedef struct BUDP_Encoder_FieldInfo
{
   unsigned uiStgPictureId; /* STG Picture ID associated with this user data. */
   BAVC_Polarity ePolarity; /* Polarity of this Encode. Only eTop or eBotField allowed */
   unsigned uiNumDescriptors; /* Can be larger than 1 for multiple Encode packets associated with this field */
   BUDP_Encoder_PacketDescriptor stPacketDescriptor[1]; /* Variable Length Array */
} BUDP_Encoder_FieldInfo;

#define BUDP_ENCODER_FIELDINFO_GET_NEXT(pFieldInfo)  \
   (BUDP_Encoder_FieldInfo *)((uint8_t *)(pFieldInfo) + sizeof(BUDP_Encoder_FieldInfo) + ((pFieldInfo)->uiNumDescriptors * sizeof(BUDP_Encoder_PacketDescriptor))- sizeof(BUDP_Encoder_PacketDescriptor))

#ifdef __cplusplus
}
#endif

#endif /* BUDP_VCE_H_ */
/* End of File */
