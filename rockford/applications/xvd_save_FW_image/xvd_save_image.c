/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: xvd_save_image.c $
 * $brcm_Revision: Hydra_Software_Devel/2 $
 * $brcm_Date: 10/29/13 4:45p $
 *
 * Module Description:
 *
 * This program uses the magnum/commonutils/afl BAFL_Load routine to load
 * the bxvd_img_dec_elf_aphrodite.c into memory. The FW code images are then
 * saved to separate binary image files. The Outer loop ARC FW code region is
 * saved in Aphrodite_OL.bin, Inner loop Arc to Aphrodite_IL.bin and the Base
 * Layer ARC to Aphrodite_BK.bin.
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/xvd_save_FW_image/xvd_save_image.c $
 *
 * Hydra_Software_Devel/2   10/29/13 4:45p davidp
 * SW7425-5316: Change makefile to use new PI directory structure, also
 * add support for Hercules FW.
 *
 * Hydra_Software_Devel/1   2/11/11 3:38p davidp
 * SW7422-22: Initial checkin for xvd_save_image FW signing tool.
 *
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#include "berr.h"
#include "bimg.h"
#include "bafl.h"
#include "bxvd_image.h"
#include "bxvd_image_priv.h"

#define BXVD_P_SVD_PRESENT 1

#define MAX_PATHLEN     256

static uint8_t filename[MAX_PATHLEN];


static BERR_Code BXVD_IMAGE_Open(void *context,
                                void **image,
                                unsigned image_id);

static BERR_Code BXVD_IMAGE_Next(void *image,
                                unsigned chunk,
                                const void **data,
                                uint16_t length);

static void BXVD_IMAGE_Close(void *image);

const BIMG_Interface SVD_IMAGE_Interface =
{
    BXVD_IMAGE_Open,
    BXVD_IMAGE_Next,
    BXVD_IMAGE_Close
};

#if (XVD_CHIP == 7445) || (XVD_CHIP == 7145) || (XVD_CHIP == 7366) || (XVD_CHIP == 7439)
#define FW_IMAGE_PREFIX "Hercules"
#define FW_FILE_NAME "bxvd_img_dec_elf_hercules.c"

#else
#define FW_IMAGE_PREFIX "Aphrodite"
#define FW_FILE_NAME "bxvd_img_dec_elf_aphrodite.c"
#endif

#include FW_FILE_NAME


static const BXVD_IMAGE_ContextEntry outerELF =
{
    &BXVD_IMG_ELF_DecOuter_size,
    &BXVD_IMG_ELF_DecOuter[0],
        &BXVD_IMG_ELF_DecOuter_offset
};

static const BXVD_IMAGE_ContextEntry innerELF =
{
    &BXVD_IMG_ELF_DecInner_size,
    BXVD_IMG_ELF_DecInner,
        &BXVD_IMG_ELF_DecInner_offset
};

static const BXVD_IMAGE_ContextEntry baseELF =
{
    &BXVD_IMG_ELF_DecBase_size,
    BXVD_IMG_ELF_DecBase,
        &BXVD_IMG_ELF_DecBase_offset
};

static const BXVD_IMAGE_ContextEntry* const BXVD_IMAGE_FirmwareImages[BXVD_IMAGE_RevK_FirmwareID_Max] =
{
    &outerELF, /* BXVD_IMG_RevK_FirmwareID_eOuterELF_AVD */
    &innerELF, /* BXVD_IMG_RevK_FirmwareID_eInnerELF_AVD */
    &baseELF,  /* BXVD_IMG_RevK_FirmwareID_eBaseELF_SVD */
};

void* const BXVD_IMAGE_Context = (void*) BXVD_IMAGE_FirmwareImages;

#define FW_IMAGE_SIZE (1024*1024)

static uint8_t FWImage[FW_IMAGE_SIZE]; /* 1MB for FW image to be loaded into */

int32_t main( int32_t argc, char **argv )
{
   int32_t outfd;
   FILE *carfd = NULL;
   uint32_t relocAddr, ilAddr;
   uint32_t ileoc, oleoc;

   uint32_t *pFWImage;
   int i, FWCnt;

   BERR_Code rc;

   void **pImgContext; /* FW Image context */

   BAFL_FirmwareLoadInfo stBAFLoadInfo;
   BXVD_IMAGE_RevK_FirmwareID XVD_IMAGE_FWID;

   pImgContext = BXVD_IMAGE_Context;

   printf("\nXVD Save FW image utility, creating OL, IL and BL binary image files\n\n");

   printf("This utility must be rebuilt each time a new %s ", FW_FILE_NAME);
   printf("is being used by the xvd PI.\n");

   for (FWCnt=0; FWCnt<3; FWCnt++)
   {
      if (FWCnt == 0)
      {
         sprintf( &filename[0], "%s_OL.bin\0", FW_IMAGE_PREFIX);

         XVD_IMAGE_FWID = BXVD_IMAGE_RevK_FirmwareID_eOuterELF_AVD;

         printf("Loading then saving Outer loop code to %s\n", filename);
      }
      else if (FWCnt == 1)
      {
         sprintf( &filename[0], "%s_IL.bin\0", FW_IMAGE_PREFIX);

         XVD_IMAGE_FWID = BXVD_IMAGE_RevK_FirmwareID_eInnerELF_AVD;

         printf("Loading then saving Inner loop code to %s\n", filename);
      }
      else
      {
         sprintf( &filename[0], "%s_BL.bin\0", FW_IMAGE_PREFIX);

         XVD_IMAGE_FWID = BXVD_IMAGE_RevK_FirmwareID_eBaseELF_SVD;

         printf("Loading then saving Inner loop code to %s\n", filename);
      }

      /* Create the output image file with read and write permissions for owner
       * and read permission for group and other.
       */

#ifdef O_BINARY
      outfd = open(filename,
                   O_CREAT|O_WRONLY|O_TRUNC|O_BINARY,
                   S_IRUSR|S_IWUSR);
#else
      outfd = open(filename,
                   O_CREAT|O_WRONLY|O_TRUNC,
                   S_IRUSR|S_IWUSR);
#endif

      if (outfd < 0)
      {
         printf("Error: couldn't open output file %s\n", filename);
         return 0;
      }

      /* Zero memory, load FW, then save FW image */

      /* Zero FW Image */

      memset(&FWImage[0], '\0', FW_IMAGE_SIZE);

#if DEBUG_PRINTS
      printf("Loading FW to address: 0x%08x\n", &FWImage[0]);
#endif
      rc = BAFL_Load ( &SVD_IMAGE_Interface,
                       (void**)BXVD_IMAGE_Context,
                       XVD_IMAGE_FWID,
                       (void *) &FWImage[0],
                       FW_IMAGE_SIZE,
                       false,               /* bDataOnly */
                       &stBAFLoadInfo );

#if DEBUG_PRINTS
      {
         int ii;
         uint32_t * pTmpDest = (uint32_t *)&FWImage;

         for (ii=0; ii<50; ii++)
         {
            printf("%08x %08x %08x %08x %08x\n", pTmpDest, *pTmpDest, *(pTmpDest+1), *(pTmpDest+2), *(pTmpDest+3));
            pTmpDest+=4;
         }
      }
#endif


      printf("Code Size: %x\n\n", stBAFLoadInfo.stCode.uiSize);

      /* Write FW Image to output file */

      write(outfd, &FWImage[0], stBAFLoadInfo.stCode.uiSize);

      /* Close the image output file */
      close(outfd);
   }

   return 0;
}
static BERR_Code BXVD_IMAGE_Open(void *context,
                 void **image,
                 unsigned image_id)
{
    BXVD_IMAGE_ContextEntry *pContextEntry = NULL;
    BXVD_IMAGE_Container *pImageContainer = NULL;


    /* Validate the firmware ID range */
    if (image_id >= BXVD_IMAGE_FirmwareID_Max)
    {
           printf("Invalid image id %d\n", image_id);
           return 1;
    }

    /* Validate the image referenced by firmware ID exists in the context */
    pContextEntry = ((BXVD_IMAGE_ContextEntry**)context)[image_id];

    if (pContextEntry == NULL) {
        /* A NULL entry does not necessarily mean a hard error
         * since we could be trying to load authenticated
         * firmware.  So, we fail silently here and expect
         * this error to be reported by the caller depending
         * on the situation */
           return 1;
    }

    /* Allocate an image container struct */
    pImageContainer = (BXVD_IMAGE_Container*) malloc(sizeof(BXVD_IMAGE_Container));
    if (pImageContainer == NULL)
        {
           printf("Cannot allocate image container\n");
           return (1);
    }
    memset(pImageContainer, 0, sizeof(BXVD_IMAGE_Container));

        pImageContainer->imageHeader.uiImageSize = *(pContextEntry->puiImageSize);
        pImageContainer->imageHeader.uiRelocationBase = *(pContextEntry->puiImageOffset);
        pImageContainer->pImageData = pContextEntry->pImageData;

#if DEBUG_PRINTS
        printf("IContxt Iamge size: %0x\n", *(pContextEntry->puiImageSize));
        printf("IContxt Image Offset: %0x\n", *(pContextEntry->puiImageOffset));
        printf("IContxt Image Data Ptr: %0x\n", pContextEntry->pImageData);
#endif
    *image = pImageContainer;

    return (0);
}

static BERR_Code BXVD_IMAGE_Next(void *image,
                          unsigned chunk,
                          const void **data,
                          uint16_t length)
{
    BXVD_IMAGE_Container *pImageContainer = (BXVD_IMAGE_Container*) image;

        if (chunk == 0)
        {
#if DEBUG_PRINTS
           printf("Returning image chunk[%d]: %d bytes of image header\n", chunk, length);
#endif
           /* Validate length requested is same as our header size */
           if (length != sizeof(BXVD_AvdImgHdr))
           {
              printf("Incorrect image header length requested\n");
              return 1;
           }

           /* Return a pointer to the header */
           *data = &pImageContainer->imageHeader;
    }
        else
        {
#if DEBUG_PRINTS
           printf("Returning image chunk[%d]: %d bytes of image data\n", chunk, length);
#endif
           *data = (void *)((uint8_t *)(pImageContainer->pImageData) + (chunk-1)*length);
    }


    return 0;
}

static void BXVD_IMAGE_Close(void *image)
{
    /* Free the image container struct */
    if (image != NULL) {
        free(image);
    }
    return;
}
