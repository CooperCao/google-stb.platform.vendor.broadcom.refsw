/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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
 * Module Description: avs types hwich different form mpeg2types
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/ 

#ifndef AVSTYPES_H
#define AVSTYPES_H

#define SC_AVS_ANY_NON_PTS  0xFF
#define SC_AVS_PTS          0xFE
#define SC_AVS_PES          0xE1
#define SC_AVS_SEQUENCE     0xB0
#define SC_AVS_SEQ_END      0xB1
#define SC_AVS_USER_DATA    0xB2
#define SC_AVS_PICTURE_I    0xB3
#define SC_AVS_EXTENSION    0xB5
#define SC_AVS_PICTURE_PB   0xB6
#define SC_AVS_FIRST_SLICE  0x00


#define SC_AVS_PES_START  SC_AVS_PES
#define SC_AVS_PES_END      0xEF

#define PC_AVS_FORBIDDEN 0x00
#define PC_AVS_P_FRAME   0x01
#define PC_AVS_B_FRAME   0x02
#define PC_AVS_RESERVED  0x03

#define AVS_PICTURE_CODE_MASK   0x00c00000
#define AVS_PICTURE_CODE_SHIFT  22

#define returnAvsPictureCode( recordByteCountHi ) \
    ((unsigned char)(((recordByteCountHi) & AVS_PICTURE_CODE_MASK) >> AVS_PICTURE_CODE_SHIFT))


#endif /* AVSTYPES_H */
