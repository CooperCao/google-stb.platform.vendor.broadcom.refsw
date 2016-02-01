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

#include "bstd.h"
#include "bmem.h"
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
