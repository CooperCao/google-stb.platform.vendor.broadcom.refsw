/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bstd.h"
#include "bmem.h"
#include "bkni.h"
#include "breg_mem.h"

#include "splash_bmp.h"
#include "splash_file.h"
#include "splash_script_load.h"


BDBG_MODULE(splashrun);

int splash_script_run(
BREG_Handle hReg, BMEM_Handle *phMem, SplashData* pSplashData)
{
	SplashBufInfo  splashBuf;
	char  *bmpFileName = NULL;
	uint8_t  *bmpBuf = NULL;
	BMP_HEADER_INFO  myBmpInfo;
	SplashSurfaceInfo  *pSurInfo;
	void *apvBuf[SPLASH_MAX_SURFACE];
	void *pvBuf;
	int x,y;
	int ii;

	BDBG_MSG(("******** Splash BVN Script Loader !!!! ********"));
	BKNI_Memset((void *) &splashBuf, 0, sizeof(SplashBufInfo));

	splashBuf.hRulMem = *(phMem + pSplashData->iRulMemIdx);

	for (ii=0 ; ii < pSplashData->iNumSurface ; ++ii)
	{
		pSurInfo = pSplashData->pSurfInfo + ii;
		apvBuf[ii] = BMEM_Alloc(*(phMem+pSurInfo->ihMemIdx), BSPLASH_SURFACE_BUF_SIZE(pSurInfo));
		BMEM_Heap_ConvertAddressToCached(*(phMem+pSurInfo->ihMemIdx), apvBuf[ii], &pvBuf);
		BMEM_ConvertAddressToOffset(
			*(phMem+pSurInfo->ihMemIdx), apvBuf[ii], &splashBuf.aulSurfaceBufOffset[ii]);
		printf("** Allocating Surface %d Memory done **\n", ii);

		if ((bmpFileName == NULL) || strcmp(bmpFileName, &pSurInfo->bmpFile[0]))
		{
			if(bmpBuf)
				BKNI_Free(bmpBuf);

			bmpFileName = &pSurInfo->bmpFile[0];
			bmpBuf = splash_open_bmp(bmpFileName);
			if(bmpBuf)
				splash_bmp_getinfo(bmpBuf, &myBmpInfo);
			else
				BDBG_ERR(("Missing file %s. could use BSEAV/app/splash/splashgen/splash.bmp", bmpFileName));
		}

		splash_set_surf_params(pSurInfo->ePxlFmt, pSurInfo->ulPitch, pSurInfo->ulWidth, pSurInfo->ulHeight) ;

		/* splash_fillbuffer(splashBuf.apvSurfaceBufAddr[ii] , 0xF8, 0xE0, 0) ; */
		splash_fillbuffer(pvBuf, 0x00, 0x00, 0x00);

		if(bmpBuf)
		{
			BDBG_MSG(("*******************************"));
			BDBG_MSG(("splash.bmp: Width = %d Height = %d",
					  myBmpInfo.info.width, myBmpInfo.info.height));
			BDBG_MSG(("*******************************"));
			x = ((int)pSurInfo->ulWidth- (int)myBmpInfo.info.width)/2 ;
			y = ((int)pSurInfo->ulHeight- (int)myBmpInfo.info.height)/2 ;
			splash_render_bmp_into_surface(x, y, bmpBuf, pvBuf);
			printf("** copy bmp into Surface %d done **\n", ii);
		}

		/* flush cached addr */
		BMEM_Heap_FlushCache(*(phMem+pSurInfo->ihMemIdx), pvBuf, BSPLASH_SURFACE_BUF_SIZE(pSurInfo));
	}
	if(bmpBuf)
		BKNI_Free(bmpBuf);

	splash_bvn_init(hReg, &splashBuf, pSplashData);
	printf("** Splash display done **\n");

	printf("Press any key to continue ...");
	getchar();

	splash_bvn_uninit(hReg, &splashBuf);

	for (ii=0 ; ii < pSplashData->iNumSurface ; ++ii)
	{
		pSurInfo = pSplashData->pSurfInfo + ii;
		BMEM_Free(*(phMem + pSurInfo->ihMemIdx), apvBuf[ii]);
	}

	return 0;
}

/* end of file */
