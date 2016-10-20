/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 ***************************************************************************/

#include <stdio.h>
#include <string.h>

#include "bstd.h"
#include "bmma.h"
#include "bkni.h"
#include "breg_mem.h"

#include "splash_bmp.h"
#include "splash_file.h"
#include "splash_script_load.h"


BDBG_MODULE(splashrun);

int splash_script_run(
BREG_Handle hReg, BMMA_Heap_Handle *phMma, SplashData* pSplashData)
{
	SplashBufInfo  splashBuf;
	char  *bmpFileName = NULL;
	uint8_t  *bmpBuf = NULL;
	BMP_HEADER_INFO  myBmpInfo;
	SplashSurfaceInfo  *pSurInfo;
        BMMA_Block_Handle mmaBlock[SPLASH_MAX_SURFACE];
        void*             mmaSwAccess;
	int x,y;
	int ii;

	BDBG_MSG(("******** Splash BVN Script Loader !!!! ********"));
	BKNI_Memset((void *) &splashBuf, 0, sizeof(SplashBufInfo));

	splashBuf.hRulMma = *(phMma + pSplashData->iRulMemIdx);

	for (ii=0 ; ii < pSplashData->iNumSurface ; ++ii)
	{
		pSurInfo = pSplashData->pSurfInfo + ii;
		mmaBlock[ii] = BMMA_Alloc(
                    *(phMma+pSurInfo->ihMemIdx),
                    BSPLASH_SURFACE_BUF_SIZE(pSurInfo)*sizeof(uint32_t),
                    0, NULL);
                splashBuf.aulSurfaceBufOffset[ii] =
                    BMMA_LockOffset (mmaBlock[ii]);
                mmaSwAccess = BMMA_Lock (mmaBlock[ii]);
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
		splash_fillbuffer(mmaSwAccess, 0x00, 0x00, 0x00);

		if(bmpBuf)
		{
			BDBG_MSG(("*******************************"));
			BDBG_MSG(("splash.bmp: Width = %d Height = %d",
					  myBmpInfo.info.width, myBmpInfo.info.height));
			BDBG_MSG(("*******************************"));
			x = ((int)pSurInfo->ulWidth- (int)myBmpInfo.info.width)/2 ;
			y = ((int)pSurInfo->ulHeight- (int)myBmpInfo.info.height)/2 ;
			splash_render_bmp_into_surface(x, y, bmpBuf, mmaSwAccess);
			printf("** copy bmp into Surface %d done **\n", ii);
		}

		/* flush cached addr */
		BMMA_FlushCache(
                    mmaBlock[ii], mmaSwAccess,
                    BSPLASH_SURFACE_BUF_SIZE(pSurInfo));
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
		BMMA_Free(mmaBlock[ii]);
	}

	return 0;
}

/* end of file */
