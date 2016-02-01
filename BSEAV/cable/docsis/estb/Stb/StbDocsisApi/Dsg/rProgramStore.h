//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       .h
//  Author:         
//  Creation Date:  
//
//****************************************************************************
//  Description: 
//                  
//
//****************************************************************************

#ifndef R_PROGRAM_STORE_H
#define R_PROGRAM_STORE_H

//********************** Include Files ***************************************
#if defined __cplusplus
extern "C" {
#endif

//********************** Global Types ****************************************

#ifndef __cplusplus
#if !defined (TARGETOS_eCos)
typedef int bool;
#define true 1
#define false 0
#endif
#endif

// After each notification or transfer of image_chunk eCM waits on 
// this port to hear from eSTB.
#define ESTB_SWDLOAD_REPLY_PORT		0xAAA		
#define ESTB_PROGSTORE_WRITE_PORT	0x8888

#define HDR_SIZE					8
#define SEGMENT_SIZE                32768 

//==============
#define IMAGE_1				0x01
#define IMAGE_2				0x02
#define kMonolithic			0x10

#define DLOAD_SUCCESSFUL			0x10000000
#define DLOAD_FAILED				0x20000000
#define PROG_STORE_WRITE			0x30000000
#define PROG_STORE_COMPUTE_CRC		0x40000000
#define PROG_STORE_VALIDATE_IMGHDR	0x50000000
#define PROG_STORE_COMPUTE_CRC_AND_LOCKIN 0x60000000

// Maximum size (in bytes) of the image that can be stored to the
// device. Default set to 4M.
#define kMaxImageSize		(4 * 1024 * 1024)

//===============
#if defined __cplusplus
}
#endif

#endif
