/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/
/**
This is a simple png reader with no bwin dependencies.
Used for api test.
**/
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <assert.h>

void priv_read(
    png_structp png_ptr,
    png_bytep data,
    png_size_t length)
{
    FILE* handle = (FILE*)png_get_io_ptr(png_ptr);
    if (fread(data, 1, length, handle) != length)
    {
        if (!ferror(handle))
            return;
    }
}

int main(int argc, char **argv)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_color_8p sig_bit;
    png_uint_32 width, height;
    int bit_depth, color_type;
    FILE *handle;

    handle = fopen(argv[1], "r");

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png_ptr);

    info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        return -1;
    }

#if 1
    png_set_read_fn(png_ptr, handle, priv_read);
#else
    png_init_io(png_ptr, handle);
#endif

#if 1
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr,
        &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
#if 0
    png_read_update_info(png_ptr, info_ptr);
    printf("rowbytes %d\n", png_get_rowbytes(png_ptr,info_ptr));
#endif

#if 0
    png_bytep row_pointers[height];
    int row;
    for (row = 0; row < height; row++)
    {
        row_pointers[row] = png_malloc(png_ptr,
            png_get_rowbytes(png_ptr,info_ptr));
    }
    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, info_ptr);
#endif
#else
    png_read_png(png_ptr, info_ptr, 0, NULL);
#endif

    printf("image %dx%d\n", width, height);
    png_destroy_read_struct(&png_ptr,&info_ptr,NULL);

    return 0;
}
