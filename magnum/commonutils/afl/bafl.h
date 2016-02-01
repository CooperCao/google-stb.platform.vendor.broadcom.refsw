/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
