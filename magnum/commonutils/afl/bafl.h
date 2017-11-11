/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* BAFL Broadcom ARC Firmware Loader.
 *
 * This module loads an optimized ELF image into memory for ARC processors.
 * An ELF image has been previousely been processed to remove unneeded
 * symbol tables abd other ELF tables that are not needed.
 *
 * The format of the resultant ELF files is as follows.
 *
 *------------------------------
 *       ELF Header            |
 *------------------------------
 *       Section Header        |
 *------------------------------
 *       Section Header        |
 *------------------------------
 *       Section Header        |
 *------------------------------
 *       Section Header        |
 *------------------------------
 *       Section Header        |
 *------------------------------
 *       Section Header        |
 *------------------------------
 *       Section Header        |
 *------------------------------
 *           .
 *           .
 *           .
 *------------------------------
 *       Section Header        |
 *------------------------------
 *       Section Data          |
 *------------------------------
 *       Section Data          |
 *------------------------------
 *       Section Data          |
 *------------------------------
 *       Section Data          |
 *------------------------------
 *       Section Data          |
 *------------------------------
 *       Section Data          |
 *------------------------------
 *       Section Data          |
 *------------------------------
 *           .
 *           .
 *           .
 *------------------------------
 *       Section Data          |
 *------------------------------
 *
 * The format of the ELF image must be in this format for the BAFL_Load routine
 * to read the image linearly using the BIMG interface. This will prevent
 * multiple copies from being perform which will result in a shorter load
 * time.
 */

#ifndef BAFL_H_
#define BAFL_H_

#include "bimg.h"

/*
 * BAFL_ImageHeader
 */
typedef struct BAFL_ImageHeader
{
   uint32_t uiHeaderVersion;          /* header version */
   uint32_t uiDevice;           /* Device being loaded */
   uint32_t uiHeaderSize;             /* size of entire header */
   uint32_t uiImageSize;              /* total size of image, not including header */
} BAFL_ImageHeader;

#define BAFL_IMAGE_HDR_VERSION 1
#define BAFL_IMAGE_HDR_SIZE  sizeof(BAFL_ImageHeader)

/*
 * BAFL_SectionInfo - indicates the virtual address and size of the ARC FW section
 */
typedef struct BAFL_SectionInfo
{
   void *pStartAddress; /* virtual address of start of section */
   BSTD_DeviceOffset offset; /* optional physical address for pStartAddress */
   uint32_t uiSize;     /* Size of section. "0" size indicates not present. */
} BAFL_SectionInfo;

/*
 * BAFL_BootInfo - provides the application's optional boot callback information
 * about the ARC whose FW has just been loaded.
 */
typedef struct BAFL_FirmwareLoadInfo
{
   BAFL_SectionInfo stCode;
   BAFL_SectionInfo stData;
} BAFL_FirmwareLoadInfo;

/***************************************************************************
 Summary:
  Enum used to specify ARC core boot mode.

 Description:
  This enum is used to determine if an ARC core boot is the result of a
  "normal" bootup sequence or the result of a watchdog restart.  The host
  may want to have a different boot/authentication sequence depending on
  the purpose of the reboot.  E.g. in watchdog mode, the security processor
  itself may not need re-configuration but the ARC itself may still need to
  be booted.
****************************************************************************/

typedef enum BAFL_BootMode
{
   BAFL_BootMode_eNormal = 0,   /* Normal boot */
   BAFL_BootMode_eWatchdog,     /* Watchdog reboot */
   BAFL_BootMode_eMaxModes
} BAFL_BootMode;

typedef struct BAFL_FirmwareInfo
{
   uint32_t uiArcInstance;  /* ARC identifier */
   BAFL_SectionInfo stCode; /* Code section info */
   struct BAFL_FirmwareInfo *pNext;
} BAFL_FirmwareInfo;

/*
 * BAFL_BootInfo - returned in the ARC Boot Callback.
 * Contains information pertaining to the boot type and code location for each ARC.
 */
typedef struct BAFL_BootInfo
{
   BAFL_BootMode eMode; /* ARC Boot mode */
   BAFL_FirmwareInfo *pstArc;
} BAFL_BootInfo;

BERR_Code
BAFL_Load(
         const uint32_t uiDevice,
         const BIMG_Interface *pImgInterface,
         void **pImageContext,
         uint32_t uiInstance,
         void *pStartAddress, /* Base address where to load */
         size_t uiMemSize,    /* Size of memory where FW is being loaded */
         bool bDataOnly,
         BAFL_FirmwareLoadInfo *pstFWLoadInfo
         );

#endif /* BAFL_H_ */
