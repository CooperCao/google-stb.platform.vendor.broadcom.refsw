/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "splash_magnum.h"
#include "splash_bmp.h"

BDBG_MODULE(splash_bmp);

#define B_GET_U16(x) (x[1]<<8  | x[0]<<0)
#define B_GET_U32(x) (x[3]<<24 | x[2]<<16 | x[1]<<8 | x[0]<<0)

#define CLUT_SIZE_MAX       (1<<12) /* 4k should do (typically 256) */


uint8_t *s_bmpBuf;
BMP_HEADER_INFO s_bmpInfo;

uint8_t *s_surBuf;
BPXL_Format s_surPxlFmt;
uint32_t s_surBytesPerPixel;
uint32_t s_surPitch;
uint32_t s_surWidth;
uint32_t s_surHeight;

int splash_bmp_getinfo(uint8_t *imgAddress, BMP_HEADER_INFO *bmpinfo)
{
	uint8_t * ptr = imgAddress? imgAddress : s_bmpBuf ;

	if(ptr[0]!='B' || ptr[1]!='M')
	{
		BDBG_MSG(("Invalid file format byte0 = %c byte1 = %c", ptr[0], ptr[1]));
		return INVALID_FILE ;
	}

	bmpinfo->header.type      = B_GET_U16(ptr); ptr += 2;
	bmpinfo->header.size      = B_GET_U32(ptr); ptr += 4;
	bmpinfo->header.reserved1 = B_GET_U16(ptr); ptr += 2;
	bmpinfo->header.reserved2 = B_GET_U16(ptr); ptr += 2;
	bmpinfo->header.offset    = B_GET_U32(ptr); ptr += 4;

	bmpinfo->info.size        = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.width       = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.height      = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.planes      = B_GET_U16(ptr); ptr += 2;
	bmpinfo->info.bits        = B_GET_U16(ptr); ptr += 2;
	bmpinfo->info.compression = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.imagesize   = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.xresolution = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.yresolution = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.ncolours    = B_GET_U32(ptr); ptr += 4;
	bmpinfo->info.importantcolours = B_GET_U32(ptr); ptr += 4;

	return 0;
}

#define COMPOSE_ARGB8888(a,r,g,b) (\
	((unsigned long)(a) & 0xFF) << 24 | \
	((unsigned long)(r) & 0xFF) << 16 | \
	((unsigned long)(g) & 0xFF) << 8 | \
	((unsigned long)(b) & 0xFF) \
	)

#define COMPOSE_RGB565(r,g,b) (\
	((unsigned short)(r) & 0xF8) << 8 | \
	((unsigned short)(g) & 0xFC) << 3 | \
	((unsigned short)(b) & 0xF8) >> 3 \
	)

#define COMPOSE_AYCbCr8888(a,y,cb,cr) (\
	((unsigned long)(a) & 0xFF) << 24 | \
	((unsigned long)(y) & 0xFF) << 16 | \
	((unsigned long)(cb) & 0xFF) << 8 | \
	((unsigned long)(cr) & 0xFF) \
	)

static void bmp_into_surbuf(int x, int y, uint32_t* clut)
{
#ifndef B_4BPP_BMP_IMAGE
	uint8_t *dest_ptr, *src_ptr;
	uint8_t *srcrow_buffer ;
	uint32_t pitch ;
	int r, g, b ;
	uint32_t ulY, ulCr, ulCb, row, src_x, dest_x ;
	uint32_t ulWidth, ulHeight;
	uint32_t ulSrcOffsetX, ulSrcOffsetY, ulDstOffsetX, ulDstOffsetY;

	/* if bmp size is smaller than surface, render it into the middle of surface,
	 * otherwise, take the center part of the bmp, and render it to surface */
	if (s_bmpInfo.info.width <= s_surWidth)
	{
		ulWidth = s_bmpInfo.info.width;
		ulSrcOffsetX = 0;
		ulDstOffsetX = x;
	}
	else
	{
		ulWidth = s_surWidth;
		ulSrcOffsetX = -x;
		ulDstOffsetX = 0;
	}
	if (s_bmpInfo.info.height <= s_surHeight)
	{
		ulHeight = s_bmpInfo.info.height;
		ulSrcOffsetY = 0;
		ulDstOffsetY = y;
	}
	else
	{
		ulHeight = s_surHeight;
		ulSrcOffsetY = -y;
		ulDstOffsetY = 0;
	}

	pitch = s_bmpInfo.info.width * ((s_bmpInfo.info.ncolours) ? 1 : 3);

	if (pitch % 4)
		pitch += 4 - (pitch % 4);

    for (row=0;row<ulHeight;row++)
	{
		/* the bmp buffer starts with the bottom row. */
		srcrow_buffer = s_bmpBuf + s_bmpInfo.header.offset +
			(pitch * (s_bmpInfo.info.height - (row+ulSrcOffsetY) - 1));
		dest_ptr = s_surBuf + (s_surPitch * (row+ulDstOffsetY)) + x * s_surBytesPerPixel;
		for (src_x = 0; src_x < ulWidth; src_x++)
		{
			src_ptr = srcrow_buffer + ((src_x + ulSrcOffsetX)* (3-(s_bmpInfo.info.ncolours ? 2 : 0)));

			dest_x = ulDstOffsetX + src_x ;
            if (!s_bmpInfo.info.ncolours) {
                b = src_ptr[0] ; /* Blue is 0 in BMP files */
                g = src_ptr[1] ; /* Greeb is 1 in BMP files */
                r = src_ptr[2] ; /* Red is 2 in BMP files */
            } else {
                b = (((uint8_t*)(&clut[*src_ptr]))[0]); /* load Blue from CLUT  */
                g = (((uint8_t*)(&clut[*src_ptr]))[1]); /* load Green from CLUT */
                r = (((uint8_t*)(&clut[*src_ptr]))[2]); /* load Red from CLUT   */
			}
			if (s_surPxlFmt == BPXL_eR5_G6_B5)
			{
				*(uint16_t *)dest_ptr = COMPOSE_RGB565( r, g, b) ;
			}
			else if (s_surPxlFmt == BPXL_eA8_R8_G8_B8)
			{
				*(uint32_t *)dest_ptr = COMPOSE_ARGB8888( 0xFF, r, g, b) ;
			}
			else if (s_surPxlFmt == BPXL_eA8_Y8_Cb8_Cr8)
			{

				/* Convert RGB components to YCbCr */
				/* Y  = R *  0.257 + G *  0.504 + B *  0.098 + 16  */
				/* Cb = R * -0.148 + G * -0.291 + B *  0.439 + 128 */
				/* Cr = R *  0.439 + G * -0.368 + B * -0.071 + 128 */
				ulY  = r *  0x41CA + g *  0x8106 + b *  0x1916 + 0x100000;
				ulCb = r * -0x25E3 + g * -0x4A7F + b *  0x7062 + 0x800000;
				ulCr = r *  0x7062 + g * -0x5E35 + b * -0x122D + 0x800000;
				ulY  = ulY >> 16;
				ulCb = ulCb >> 16;
				ulCr = ulCr >> 16;
				*(uint32_t *)dest_ptr = COMPOSE_AYCbCr8888( 0xFF, ulY, ulCb, ulCr) ;
			}
			dest_ptr += s_surBytesPerPixel;
		}
	}

#else /* B_4BPP_BMP_IMAGE */
	int row, dest_x, src_x ;
	uint8_t *dest_ptr, *src_ptr;
	uint8_t *srcrow_buffer ;
	uint8_t *palette;
	uint32_t pitch ;
	int r, g, b ;
	int nn;
	uint32_t ulY, ulCr, ulCb;

	#if 0
	int t;
	for(t=0; t<16; t++)
	{
		BDBG_ERR(("offset %d ",sizeof (BMP_HEADER_INFO)));
		BDBG_ERR(("%d, b 0x%x, g 0x%x, r 0x%x, reserve 0x%x ",t,palette[t*4], palette[t*4+1], palette[t*4+2], palette[t*4+3]));
	}
	#endif

	pitch = s_bmpInfo.info.width / 2 ;
	if (pitch % 4)
		pitch += 4 - (pitch % 4);

	for (row=0;row<s_bmpInfo.info.height;row++)
	{
		/* the bmp buffer starts with the bottom row. */
		srcrow_buffer = s_bmpBuf + s_bmpInfo.header.offset + (pitch * (s_bmpInfo.info.height - row - 1));
		palette = s_bmpBuf + sizeof (BMP_HEADER_INFO) -2; /* TODO: Set proper palette address */
		BDBG_MSG(("scrow_buffer %p",srcrow_buffer));

		dest_ptr = s_surBuf + (s_surPitch * (row+y)) + x * s_surBytesPerPixel;
		for (src_x = 0; src_x < s_bmpInfo.info.width/2; src_x++)
		{
			src_ptr = srcrow_buffer + src_x;
			BDBG_MSG(("row %d src_x %d scr_ptr %p",row, src_x, src_ptr));
			dest_x = x + src_x ;

			for (nn = 0; nn < 2; nn++)
			{
				int shift = (1-nn)*4;
				int index = (src_ptr[0] >> shift) & 0xF;

				#if 0
				BDBG_ERR(("src_ptr[0] 0x%x, index %d,shift %d",src_ptr[0],index, shift));
				#endif

				b = palette[index*4];
				g = palette[index*4+1];
				r = palette[index*4+2];

				#if 0
				BDBG_ERR(("b %d, g %d,r %d",b,g,r));
				#endif
				if (s_surPxlFmt == BPXL_eR5_G6_B5)
				{
					*(uint16_t *)dest_ptr = COMPOSE_RGB565( r, g, b) ;
				}
				else if (s_surPxlFmt == BPXL_eA8_R8_G8_B8)
				{
					*(uint32_t *)dest_ptr = COMPOSE_ARGB8888( 0xFF, r, g, b) ;
				}
				else if (s_surPxlFmt == BPXL_eA8_Y8_Cb8_Cr8)
				{
					/* Convert RGB components to YCbCr */
					/* Y  = R *  0.257 + G *  0.504 + B *  0.098 + 16  */
					/* Cb = R * -0.148 + G * -0.291 + B *  0.439 + 128 */
					/* Cr = R *  0.439 + G * -0.368 + B * -0.071 + 128 */
					ulY  = r *  0x41CA + g *  0x8106 + b *  0x1916 + 0x100000;
					ulCb = r * -0x25E3 + g * -0x4A7F + b *  0x7062 + 0x800000;
					ulCr = r *  0x7062 + g * -0x5E35 + b * -0x122D + 0x800000;
					ulY  = ulY >> 16;
					ulCb = ulCb >> 16;
					ulCr = ulCr >> 16;
					*(uint32_t *)dest_ptr = COMPOSE_AYCbCr8888( 0xFF, ulY, ulCb, ulCr) ;
				}
				dest_ptr += s_surBytesPerPixel;
			}
		}
	}
#endif /* B_4BPP_BMP_IMAGE */
}

int splash_render_bmp_into_surface(
	int x,
	int y,
	uint8_t *bmpAddress,
	void* surfaceAddress)
{
    uint32_t index;
    uint32_t *colours;
    uint8_t *ptr;

#if CFG_SPLASH
    if(!bmpAddress || !surfaceAddress)
        return 1;
#else
    BDBG_ASSERT(bmpAddress);
    BDBG_ASSERT(surfaceAddress);
#endif
	if(bmpAddress) s_bmpBuf = bmpAddress ;

	if(splash_bmp_getinfo(bmpAddress, &s_bmpInfo) == INVALID_FILE)
		return INVALID_FILE ;

#if CFG_SPLASH
    colours = (uint32_t *)KMALLOC(sizeof(uint32_t) * CLUT_SIZE_MAX, 0);
#else
    colours = (uint32_t *)BKNI_Malloc(sizeof(uint32_t) * CLUT_SIZE_MAX);
#endif
    if (!colours)
    {
        BDBG_ERR(("Out of memory."));
        return 1;
    }

	BDBG_MSG(("image %x, size=%d, offset=%d",
		s_bmpInfo.header.type, s_bmpInfo.header.size, s_bmpInfo.header.offset));
	BDBG_MSG(("info %d, %dx%d compression %d",
		s_bmpInfo.info.size, s_bmpInfo.info.width, s_bmpInfo.info.height, s_bmpInfo.info.compression));

	if (s_bmpInfo.info.ncolours) {
        BDBG_MSG(("Palletised bitmap, ncolours: %d/%d", s_bmpInfo.info.ncolours, s_bmpInfo.info.importantcolours));
        ptr = bmpAddress + sizeof(BMP_HEADER_INFO) - 2;
        if (s_bmpInfo.info.ncolours > CLUT_SIZE_MAX) {
            BDBG_ERR(("not enough storage to hold CLUT: %d", s_bmpInfo.info.ncolours));
            return (1);
        }
        for (index = 0; index < s_bmpInfo.info.ncolours; index++, ptr+=4) {
            BKNI_Memcpy(&colours[index], ptr, sizeof(uint32_t));
            /*BDBG_MSG(("color[%d]=0x%08X", index, colours[index])); */ /* dump CLUT values */
        }
    }

	if (s_bmpInfo.info.compression) {
		BDBG_ERR(("compression not supported. (compression=%d)", s_bmpInfo.info.ncolours));
		return 1;
	}

	s_surBuf = surfaceAddress ;
	bmp_into_surbuf(x, y, colours) ;

#if CFG_SPLASH
        KFREE(colours);
#else
        BKNI_Free(colours);
#endif
	return 0 ;

}

int splash_set_surf_params(
	BPXL_Format pxlFmt,
	uint32_t splashPitch,
	uint32_t splashWidth,
	uint32_t splashHeight
	)
{
	s_surPitch = splashPitch;
	s_surWidth = splashWidth;
	s_surHeight = splashHeight;
	s_surPxlFmt = pxlFmt;
	switch(pxlFmt)
	{
		case BPXL_eA8_R8_G8_B8 :
	  	case BPXL_eA8_Y8_Cb8_Cr8:
			s_surBytesPerPixel = 4 ;
			break ;
		case BPXL_eR5_G6_B5 :
			s_surBytesPerPixel = 2;
			break ;
		default :
			/* printf("Error Unsupported pixel format %08x\n", pxl) ; */
			s_surBytesPerPixel = 10;
	} ;
	#if 0
	printf("splashPitch = %d, splashWidth = %d, splashHeight=%d Pixel Format = %08x \n"
		" bytesperpixel = %d\n",
		s_surPitch, s_surWidth, s_surHeight, pxl, s_surBytesPerPixel) ;
	#endif
	return 0;
}

int splash_fillbuffer( void* surfaceAddress, int r, int g, int b)
{
	uint32_t i, j;
	uint8_t *pCurr ;
	uint8_t *pLine = surfaceAddress ;
	/* fill surface */
	for(i=0; i<s_surHeight; i++)
	{
		pCurr = pLine;
		for(j=0; j<s_surWidth; j++)
		{
			if(s_surBytesPerPixel == 2)
				*(uint16_t *)pCurr = COMPOSE_RGB565( r, g, b) ;
			else if(s_surBytesPerPixel == 4)
				*(uint32_t *)pCurr = COMPOSE_ARGB8888( 0xFF, r, g, b) ;
			pCurr += s_surBytesPerPixel ;
		}
		pLine += s_surPitch ;
	}
	return 0;
}

