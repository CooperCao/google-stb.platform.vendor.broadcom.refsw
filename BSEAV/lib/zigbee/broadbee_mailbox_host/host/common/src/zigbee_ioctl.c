/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* This is code which demonstrates the interface to the Zigbee kernel module
   driver. This code will need to be written by Benjamin's team. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "zigbee_ioctl.h"
#include "zigbee_driver.h"

/*******************************************************************************
   Download and start the Zigbee core.
*******************************************************************************/
int Zigbee_Ioctl_Start(int fd, unsigned char *pImage, unsigned int size_in_bytes)
{
    struct fw f;
    f.size_in_bytes = size_in_bytes;
    f.pImage = pImage;

    if (ioctl(fd, ZIGBEE_IOCTL_START, &f) == 0) {
        return ZIGBEE_START_OK;
    }
    return ZIGBEE_START_NOT_OK;
}

/*******************************************************************************
   Stop the Zigbee core.
*******************************************************************************/
int Zigbee_Ioctl_Stop(int fd)
{
    unsigned int data;

    if (ioctl(fd, ZIGBEE_IOCTL_STOP, &data) == 0) {
        return ZIGBEE_STOP_OK;
    }
    return ZIGBEE_STOP_NOT_OK;
}

/*******************************************************************************
   It is assumed that the data pointed to by pData has a 32 bit header at the
   top, which contains the size in bits 4:0, as specified in the BroadBee
   Application Software Reference Manual.

   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_WriteToMbox(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_WRITE_TO_MBOX, pData) == 0) {
        return ZIGBEE_WRITE_TO_MBOX_QUEUED;
    }
    return ZIGBEE_WRITE_TO_MBOX_ERROR;
}

/*******************************************************************************
   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_ReadFromMbox(int fd, char *pData)
{

    if (ioctl(fd, ZIGBEE_IOCTL_READ_FROM_MBOX, pData) == 0) {
        return ZIGBEE_READ_FROM_MBOX_OK;
    }
    return ZIGBEE_READ_FROM_MBOX_ERROR;
}

/*******************************************************************************
   This function may block.
*******************************************************************************/
int Zigbee_Ioctl_WaitForWDTInterrupt(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_WAIT_FOR_WDT_INTERRUPT, pData) == 0) {
        return ZIGBEE_WDT_OK;
    }
    return ZIGBEE_WDT_ERROR;
}

int Zigbee_Ioctl_GetRf4ceMacAddr(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_GET_RF4CE_MAC_ADDR, pData) == 0) {
        return ZIGBEE_WDT_OK;
    }
    return ZIGBEE_WDT_ERROR;
}

int Zigbee_Ioctl_GetZbproMacAddr(int fd, char *pData)
{
    if (ioctl(fd, ZIGBEE_IOCTL_GET_ZBPRO_MAC_ADDR, pData) == 0) {
        return ZIGBEE_WDT_OK;
    }
    return ZIGBEE_WDT_ERROR;
}

/* eof zigbee_ioctl.c */