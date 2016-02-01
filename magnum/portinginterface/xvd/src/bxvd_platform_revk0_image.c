/***************************************************************************
 *     Copyright (c) 2009-2011, Broadcom Corporation
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


#if ((BCHP_CHIP == 7135) || \
     (BCHP_CHIP == 7231) || \
     (BCHP_CHIP == 7344) || \
     (BCHP_CHIP == 7346) || \
     (BCHP_CHIP == 7422) || \
     (BCHP_CHIP == 7425) || \
     (BCHP_CHIP == 7429) || \
     (BCHP_CHIP == 7435))
#define BXVD_P_SVD_PRESENT 1
#else
#define BXVD_P_SVD_PRESENT 0
#endif

#include "bstd.h"
#include "bxvd_image.h"
#include "bxvd_image_priv.h"

#if (BCHP_CHIP == 7640)
/* DVD platforms */
#include "bxvd_img_dec_elf_dione.c" 

#else
/* STB/DTV platforms */
#include "bxvd_img_dec_elf_aphrodite.c" 
#endif

static const BXVD_IMAGE_ContextEntry outerELF =
{
	&BXVD_IMG_ELF_DecOuter_size,
	BXVD_IMG_ELF_DecOuter,
	&BXVD_IMG_ELF_DecOuter_offset,
        BXVD_P_OL,
};

static const BXVD_IMAGE_ContextEntry innerELF =
{
	&BXVD_IMG_ELF_DecInner_size,
	BXVD_IMG_ELF_DecInner,
	&BXVD_IMG_ELF_DecInner_offset,
        BXVD_P_IL
};

#if BXVD_P_SVD_PRESENT 
static const BXVD_IMAGE_ContextEntry baseELF =
{
	&BXVD_IMG_ELF_DecBase_size,
	BXVD_IMG_ELF_DecBase,
	&BXVD_IMG_ELF_DecBase_offset,
        BXVD_P_BL
};
#endif

static const BXVD_IMAGE_ContextEntry* const BXVD_IMAGE_FirmwareImages[BXVD_IMAGE_RevK_FirmwareID_Max] = 
{	
	&outerELF, /* BXVD_IMG_RevK_FirmwareID_eOuterELF_AVD */
	&innerELF, /* BXVD_IMG_RevK_FirmwareID_eInnerELF_AVD */
#if BXVD_P_SVD_PRESENT 
	&baseELF,  /* BXVD_IMG_RevK_FirmwareID_eBaseELF_SVD */
#endif
};

const void* const BXVD_IMAGE_Context = (void*) BXVD_IMAGE_FirmwareImages;
