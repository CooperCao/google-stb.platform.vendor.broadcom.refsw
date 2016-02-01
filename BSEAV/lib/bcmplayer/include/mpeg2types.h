/***************************************************************************
 *     Copyright (c) 2002-2008, Broadcom Corporation
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
 * Module Description: Common mpeg2 types
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef MPEG2TYPES_H
#define MPEG2TYPES_H

#define TS_PID_BYTE			  1
#define TS_PID_MASK			  0x1FFF
#define TS_PAYLOAD_UNIT_START_BYTE	  1
#define TS_PAYLOAD_UNIT_START_MASK	  0x40
#define TS_ADAPT_BYTE			  3
#define TS_ADAPT_MASK			  0x30
#define TS_ADAPT_LENGTH_BYTE		  4
#define TS_RAI_BYTE                       5
#define TS_RAI_MASK                       0X40
#define TS_PES_LENGTH_BYTE		  4
#define TS_PES_PTS_DTS_FLAGS		  7
#define TS_PES_HEADER_LENGTH_BYTE	  8
#define TS_PES_PTS_32_30		  9
#define TS_PES_PTS_29_15		  10
#define TS_PES_PTS_14_0			  12

#define SC_ANY_NON_PTS	0xFF
#define SC_PTS			0xFE
#define SC_PES			0xE0
#define SC_SEQUENCE		0xB3
#define SC_PICTURE		0x00
#define SC_FIRST_SLICE	0x01
#define SC_EXTENSION	0xB5
#define SC_SEQ_END		0xB7
#define SC_GOP			0xB8

#define SC_PES_START	SC_PES
#define SC_PES_END		0xEF

#define PC_I_FRAME		0x01
#define PC_P_FRAME		0x02
#define PC_B_FRAME		0x03
#define PC_ANY_FRAME	0xFF

#define PICTURE_CODE_MASK	0x00003800
#define PICTURE_CODE_SHIFT	11

#define START_CODE_MASK		0xFF000000
#define START_CODE_SHIFT	24

#define INTRA_SLICE_FLAG_BIT	18
#define INTRA_SLICE_BIT			17

#define returnPictureCode( startCodeBytes ) \
	((unsigned char)(((startCodeBytes) & PICTURE_CODE_MASK) >> PICTURE_CODE_SHIFT))

#define returnStartCode( startCodeBytes ) \
	((unsigned char)(((startCodeBytes) & START_CODE_MASK)>>START_CODE_SHIFT))

#define returnIntraSliceBit( startCodeBytes ) \
	(((startCodeBytes) >> INTRA_SLICE_FLAG_BIT)&1? \
		((startCodeBytes) >> INTRA_SLICE_BIT)&1 : 0)

#endif /* MPEG2TYPES_H */
