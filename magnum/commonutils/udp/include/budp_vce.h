/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
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
