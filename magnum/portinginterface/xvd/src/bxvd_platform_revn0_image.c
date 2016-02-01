/***************************************************************************
 *     Copyright (c) 2009-2013, Broadcom Corporation
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
 ***************************************************************************/


#include "bstd.h"
#include "bxvd_image.h"
#include "bxvd_image_priv.h"

#if ((BCHP_CHIP == 7145) && (BCHP_VER == BCHP_VER_A0))
#include "bxvd_img_dec_elf_hercules_revn1.c"

#else
#include "bxvd_img_dec_elf_hercules.c"
#endif

#if BXVD_P_USE_BINARY_IMAGE
static const BXVD_IMAGE_ContextEntry outerELF =
{
  "bxvd_img_dec_elf_hercules_outer.bin"
};

static const BXVD_IMAGE_ContextEntry innerELF =
{
  "bxvd_img_dec_elf_hercules_inner.bin"
};

#else
static const BXVD_IMAGE_ContextEntry outerELF =
{
   &BXVD_IMG_ELF_DecOuter_size,
   BXVD_IMG_ELF_DecOuter,
   &BXVD_IMG_ELF_DecOuter_offset,
   BXVD_P_OL
};

static const BXVD_IMAGE_ContextEntry innerELF =
{
   &BXVD_IMG_ELF_DecInner_size,
   BXVD_IMG_ELF_DecInner,
   &BXVD_IMG_ELF_DecInner_offset,
   BXVD_P_IL
};
#endif

static const BXVD_IMAGE_ContextEntry* const BXVD_IMAGE_FirmwareImages[BXVD_IMAGE_RevK_FirmwareID_Max] =
{
	&outerELF, /* BXVD_IMG_RevK_FirmwareID_eOuterELF_AVD */
	&innerELF, /* BXVD_IMG_RevK_FirmwareID_eInnerELF_AVD */
};

const void* const BXVD_IMAGE_Context = (void*) BXVD_IMAGE_FirmwareImages;
