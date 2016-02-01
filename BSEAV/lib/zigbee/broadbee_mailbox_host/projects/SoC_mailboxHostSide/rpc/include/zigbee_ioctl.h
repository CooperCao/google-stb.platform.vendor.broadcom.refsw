/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
/* This is code which demonstrates the interface to the Zigbee kernel module
   driver. This code will need to be written by Benjamin's team. */

#ifndef _ZIGBEE_IOCTL_H_
#define _ZIGBEE_IOCTL_H_

#define ZIGBEE_WRITE_TO_MBOX_QUEUED 0
#define ZIGBEE_WRITE_TO_MBOX_ERROR -1

#define ZIGBEE_READ_FROM_MBOX_NO_DATA 1
#define ZIGBEE_READ_FROM_MBOX_OK 0
#define ZIGBEE_READ_FROM_MBOX_ERROR -1

#define ZIGBEE_WDT_OK 0
#define ZIGBEE_WDT_ERROR -1

#define ZIGBEE_START_OK 0
#define ZIGBEE_START_NOT_OK -1

#define ZIGBEE_STOP_OK 0
#define ZIGBEE_STOP_NOT_OK -1

int Zigbee_Ioctl_Start(int fd, unsigned char *pImage, unsigned int size_in_bytes);

/*******************************************************************************
   Stop the Zigbee core.
*******************************************************************************/
int Zigbee_Ioctl_Stop(int fd);

/*******************************************************************************
   It is assumed that the data pointed to by pData has a 32 bit header at the
   top, which contains the size in bits 4:0, as specified in the BroadBee
   Application Software Reference Manual.

   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_WriteToMbox(int fd, char *pData);

/*******************************************************************************
   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_ReadFromMbox(int fd, char *pData);

/*******************************************************************************
   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_WaitForWDTInterrupt(int fd, char *pData);

int Zigbee_Ioctl_GetRf4ceMacAddr(int fd, char *pData);
int Zigbee_Ioctl_GetZbproMacAddr(int fd, char *pData);

#endif /* _ZIGBEE_IOCTL_H_ */