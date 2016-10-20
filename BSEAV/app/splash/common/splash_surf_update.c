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

#include "bstd.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bpxl.h"

#include "splash_surf_update.h"
#if 0 
void FillRect(uint32_t color,
				uint32_t xStart, uint32_t yStart, uint32_t width, uint32_t height,
				void* splashAddress, uint32_t splashPitch, uint32_t splashWidth, uint32_t splashHeight)
{
	uint16_t*	pixel;
	uint32_t	x, y;
	
	/* ASSUMES 16BPP!! */
	
	/* Fix out of range problems */
	if (xStart > splashWidth)
		xStart = splashWidth;
	if (yStart > splashHeight)
		yStart = splashHeight;
	if (xStart + width > splashWidth)
		width = splashWidth - xStart;
	if (yStart + height > splashHeight)
		height = splashHeight - yStart;
	
	for (y = yStart; y < yStart + height; y++)
	{
		pixel = (uint16_t*)( ((uint8_t *)splashAddress) + y * splashPitch + xStart * sizeof(uint16_t));
		
		for (x = 0; x < width; x++)
		{
			*pixel++ = color;
		}
	}
	
}

void FillSurfaceMemory(uint32_t color, void* splashAddress, uint32_t splashPitch, uint32_t splashWidth, uint32_t splashHeight)
{
	FillRect(color, 0, 0, splashWidth, splashHeight, splashAddress, splashPitch, splashWidth, splashHeight);
}

void ShowColorsOnSurface(void* splashAddress, uint32_t splashPitch, uint32_t splashWidth, uint32_t splashHeight)
{
	int i;

	printf("In ShowColorsOnSurface splashAddress p\n" /* splashAddress*/);
	FillSurfaceMemory(0x0000, splashAddress, splashPitch, splashWidth, splashHeight); /* black */

	printf("Filled with Blue\n");

	FillRect(0xf800,   0, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
	FillRect(0x07e0, 100, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
	FillRect(0x001f, 200, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
	FillRect(0x0000, 300, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
	FillRect(0xffff, 400, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);

	printf("5 fill rects later\n");

	#if 0
	BKNI_Sleep(100000);
	#else
	
 	i = 100;
 	while (i--)
 	{
		BKNI_Sleep(2000);

		FillSurfaceMemory(0xf800, splashAddress, splashPitch, splashWidth, splashHeight); /* red */
	
		printf("red\n");

		BKNI_Sleep(1000);

		FillSurfaceMemory(0x07e0, splashAddress, splashPitch, splashWidth, splashHeight); /* green */

		printf("green\n");
		BKNI_Sleep(1000);

		FillSurfaceMemory(0x001f, splashAddress, splashPitch, splashWidth, splashHeight); /* blue */

		printf("blue\n");

		BKNI_Sleep(1000);

		printf("bars\n");
		
		FillRect(0xf800,   0, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
		FillRect(0x07e0, 100, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
		FillRect(0x001f, 200, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
		FillRect(0x0000, 300, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
		FillRect(0xffff, 400, 100, 100, 350, splashAddress, splashPitch, splashWidth, splashHeight);
 	}
 	#endif
} 	

#endif
