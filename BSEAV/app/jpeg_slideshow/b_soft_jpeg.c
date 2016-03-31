/***************************************************************************
 *     Copyright (c) 2009, Broadcom Corporation
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
 *    Reference JPEG Decode into YUYV or RGB Color spaces with slideshow
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include "b_decode_jpeg.h"
#include "jpeglib.h"

#define USE_STDIO   /* Use STDIO or MMAP? */
#define AUTO_TIMEOUT 5 /* seconds */

#ifndef MAP_FAILED
#define MAP_FAILED      ((void *) -1)
#endif

#ifndef USE_STDIO
static void b_init_source(j_decompress_ptr cinfo)
{
    /* nothing */
}

static boolean b_fill_input_buffer(j_decompress_ptr cinfo)
{
    /* already filled */
    return true;
}

static void b_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    cinfo->src->bytes_in_buffer -= num_bytes;
    cinfo->src->next_input_byte += num_bytes;
}

static void b_term_source(j_decompress_ptr cinfo)
{
    /* nothing */
}

static void b_jpeg_mem_src(j_decompress_ptr cinfo, void *buffer, int length)
{
    if (cinfo->src == NULL)
    {   /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
        (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                    sizeof(struct jpeg_source_mgr));
    }

    cinfo->src->init_source = b_init_source;
    cinfo->src->fill_input_buffer = b_fill_input_buffer;
    cinfo->src->skip_input_data = b_skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default*/
    cinfo->src->term_source = b_term_source;
    cinfo->src->bytes_in_buffer = length;
    cinfo->src->next_input_byte = buffer;
}
#endif /* #ifndef USE_STDIO */

/* Currently supporting little-endian only */
typedef union
{
    struct
    {
        unsigned char b;
        unsigned char g;
        unsigned char r;
        unsigned char a;
    } argb;
    struct
    {
        unsigned char cr;
        unsigned char y1;
        unsigned char cb;
        unsigned char y0;
    } ycbcr;
} graphics_pixel;

typedef struct
{
    unsigned char grayscale;
} grayscale_pixel;

typedef struct
{
    unsigned char y;
    signed char cb;
    signed char cr;
} ycbcr_pixel;

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_pixel;

void DecompressGrayscale(j_decompress_ptr cinfo, graphics_pixel *pGraphicsMemory, int pitch, JSAMPARRAY pMemory)
{
    JDIMENSION lines_read, total_lines_read;
    int pixel_offset, rounded_width;
    unsigned char *pBaseGraphicsMemory = (unsigned char *)pGraphicsMemory;

    rounded_width = cinfo->output_width & ~0x1L;
    total_lines_read = 0;
    while ( total_lines_read < cinfo->output_height )
    {
        grayscale_pixel *pGrayscalePixels = (grayscale_pixel *)pMemory[0];

        lines_read = jpeg_read_scanlines(cinfo, pMemory, cinfo->rec_outbuf_height);
        total_lines_read += lines_read;

        while ( lines_read-- > 0 )
        {
            pGraphicsMemory = (graphics_pixel *)pBaseGraphicsMemory;
            /* Now we have cinfo.output_width * lines_read of data to copy to the bitmap */
            for ( pixel_offset=0; pixel_offset < rounded_width; pixel_offset+=2 )
            {
                /* Grayscale -> YCbCr -- Y = grayscale, CbCr = 0x80 (unity) */
                pGraphicsMemory->ycbcr.y0 = pGrayscalePixels[pixel_offset].grayscale;
                pGraphicsMemory->ycbcr.y1 = pGrayscalePixels[pixel_offset+1].grayscale;
                pGraphicsMemory->ycbcr.cr = pGraphicsMemory->ycbcr.cb = 0x80;
                pGraphicsMemory++;
            }
            /* Skip to next scanline */
            pGrayscalePixels += cinfo->output_width;
            pBaseGraphicsMemory += pitch;
        }
    }
}

void DecompressYCbCr(j_decompress_ptr cinfo, graphics_pixel *pGraphicsMemory, int pitch, JSAMPARRAY pMemory)
{
    JDIMENSION lines_read, total_lines_read;
    int pixel_offset, rounded_width;
    JDIMENSION output_width;
    JDIMENSION output_height;
    int rec_outbuf_height;
    ycbcr_pixel *pBase = (ycbcr_pixel *)pMemory[0];
    unsigned char *pBaseGraphicsMemory = (unsigned char *)pGraphicsMemory;

    output_height = cinfo->output_height;
    output_width = cinfo->output_width;
    rec_outbuf_height = cinfo->rec_outbuf_height;

    rounded_width = output_width & ~0x1L;
    total_lines_read = 0;
    while ( total_lines_read < output_height )
    {
        ycbcr_pixel *pYCbCrPixels = pBase;

        lines_read = jpeg_read_scanlines(cinfo, pMemory, rec_outbuf_height);
        total_lines_read += lines_read;

        while ( lines_read-- > 0 )
        {
            pGraphicsMemory = (graphics_pixel *)pBaseGraphicsMemory;
            /* Now we have cinfo.output_width * lines_read of data to copy to the bitmap */
            for ( pixel_offset=0; pixel_offset < rounded_width; pixel_offset+=2 )
            {
                int chroma;
                pGraphicsMemory->ycbcr.y0 = pYCbCrPixels[pixel_offset].y;
                pGraphicsMemory->ycbcr.y1 = pYCbCrPixels[pixel_offset+1].y;
                /* Could average for slightly better quality but most are 4:2:2 -> 4:4:4 anyway */
                chroma = pYCbCrPixels[pixel_offset].cr;
                pGraphicsMemory->ycbcr.cr = chroma;
                chroma = pYCbCrPixels[pixel_offset].cb;
                pGraphicsMemory->ycbcr.cb = chroma;
                pGraphicsMemory++;
            }
            /* Skip to next scanline */
            pYCbCrPixels += output_width;
            pBaseGraphicsMemory += pitch;
        }
    }
}

void DecompressRGB(j_decompress_ptr cinfo, graphics_pixel *pGraphicsMemory, int pitch, JSAMPARRAY pMemory)
{
    JDIMENSION lines_read, total_lines_read;
    JDIMENSION pixel_offset;
    unsigned char *pBaseGraphicsMemory = (unsigned char *)pGraphicsMemory;

    total_lines_read = 0;
    while ( total_lines_read < cinfo->output_height )
    {
        rgb_pixel *pRGBPixels = (rgb_pixel *)pMemory[0];

        lines_read = jpeg_read_scanlines(cinfo, pMemory, cinfo->rec_outbuf_height);
        total_lines_read += lines_read;

        while ( lines_read-- > 0 )
        {
            pGraphicsMemory = (graphics_pixel *)pBaseGraphicsMemory;
            /* Now we have cinfo.output_width * lines_read of data to copy to the bitmap */
            for ( pixel_offset=0; pixel_offset < cinfo->output_width; pixel_offset++ )
            {
                /* RGB -> ARGB */
                pGraphicsMemory->argb.a = 0xff;
                pGraphicsMemory->argb.r = pRGBPixels[pixel_offset].r;
                pGraphicsMemory->argb.g = pRGBPixels[pixel_offset].g;
                pGraphicsMemory->argb.b = pRGBPixels[pixel_offset].b;
                pGraphicsMemory++;
            }
            /* Skip to next scanline */
            pRGBPixels += cinfo->output_width;
            pBaseGraphicsMemory += pitch;
        }
    }
}

NEXUS_SurfaceHandle b_decompress_jpeg(const char *pszFilename, unsigned int maxWidth, unsigned int maxHeight)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    graphics_pixel *pGraphicsMemory;
    JSAMPROW pMemory;
    JSAMPROW pSampArray[8];
    int i;
    struct timeval timeStart, timeEnd;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SurfaceMemory surfaceMemory;
    NEXUS_SurfaceHandle returnSurface=NULL;
    FILE *pFile=NULL;
    int fd=-1;
    void *pBuffer=MAP_FAILED;
    struct stat st;
    unsigned int maxScale=8;

    /* Initialize surface settings */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);

    /* Initialize decoder */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    /* Open source file */
    #ifdef USE_STDIO
        /* STDIO -- Standard, but slower */
    pFile = fopen(pszFilename, "rb");
    if ( NULL == pFile )
    {
        fprintf(stderr, "Cannot open file '%s'\n", pszFilename);
        goto err;
    }
    jpeg_stdio_src(&cinfo, pFile);

    #else

        /* MMAP -- Linux specific, but faster */
    fd = open(pszFilename, O_RDONLY);
    if ( fd < 0 )
    {
        fprintf(stderr, "Cannot open file '%s'\n", pszFilename);
        goto err;
    }
    rc = fstat(fd, &st);
    if ( rc < 0 )
    {
        close(fd);
        return NULL;
    }
    pBuffer=mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if ( pMemory == MAP_FAILED )
    {
        goto err;
    }

    b_jpeg_mem_src(&cinfo, pBuffer, st.st_size);
    #endif

    jpeg_read_header(&cinfo, TRUE);

    /* Optimize for speed */
    cinfo.dct_method = JDCT_IFAST;
    cinfo.do_fancy_upsampling = FALSE;
    cinfo.do_block_smoothing = FALSE;

    /* Check for supported color spaces */
    switch ( cinfo.jpeg_color_space )
    {
    case JCS_YCbCr:
        cinfo.out_color_space = JCS_YCbCr;
        surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
        {
            char *pszFormat;
            if ( cinfo.comp_info[0].h_samp_factor > cinfo.comp_info[1].h_samp_factor )
            {
                if ( cinfo.comp_info[0].v_samp_factor > cinfo.comp_info[1].v_samp_factor )
                {
                    pszFormat = "4:2:0 [CANNOT DOWNSCALE]";
                    maxScale=1;
                }
                else
                    pszFormat = "4:2:2";
            }
            else
                pszFormat = "4:4:4";

            printf("%s: File color space is YCbCr (%s)\n", pszFilename, pszFormat);
        }
        break;
    case JCS_GRAYSCALE:
        cinfo.out_color_space = JCS_GRAYSCALE;
        surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
        printf("%s: File color space is Grayscale\n", pszFilename);
        break;
    case JCS_RGB:
        cinfo.out_color_space = JCS_RGB;
        surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        printf("%s: File color space is RGB\n", pszFilename);
        break;
    default:
        fprintf(stderr, "%s: Unsupported color space in file.  Currently supporting YCbCr, Grayscale, and RGB only.\n", pszFilename);
        goto err;
    }

    /* Determine output size */
    jpeg_calc_output_dimensions(&cinfo);
    printf("%s: Original image resolution is %dx%d\n", pszFilename, cinfo.image_width, cinfo.image_height);
    /* Downscale until less than output resolution. */
    while ( cinfo.scale_denom < maxScale &&
            (cinfo.output_width > maxWidth || cinfo.output_height > maxHeight) )
    {
        cinfo.scale_denom<<=1;
        jpeg_calc_output_dimensions(&cinfo);
    }
    printf("%s: Target image resolution is %dx%d (scaling=1/%d)\n", pszFilename, cinfo.output_width, cinfo.output_height, cinfo.scale_denom);

    /* Now, we have the official output size.  Create a surface to match */
    surfaceCreateSettings.height = cinfo.output_height;
    surfaceCreateSettings.width = (cinfo.output_width+1)&~1;

    /* Calculate graphics memory requirements */
    returnSurface = NEXUS_Surface_Create(&surfaceCreateSettings);
    if ( NULL == returnSurface )
    {
        fprintf(stderr, "%s: Unable to create graphics surface (%dx%d)\n", pszFilename, cinfo.output_width, cinfo.output_height);
        goto err;
    }
    else
    {
        (void)NEXUS_Surface_GetMemory(returnSurface, &surfaceMemory);
        pGraphicsMemory = (graphics_pixel *)surfaceMemory.buffer;
    }

    /* Fire up decoder & start timing */
    gettimeofday(&timeStart, NULL);
    jpeg_start_decompress(&cinfo);

    /* Intermediate buffer for decompression */
    pMemory = malloc(cinfo.output_width * cinfo.output_components * cinfo.rec_outbuf_height);
    if ( NULL == pMemory )
    {
        fprintf(stderr, "%s: Unable to allocate line buffer memory (rec_outbuf_height=%d)\n", pszFilename, cinfo.rec_outbuf_height);
        goto err;
    }

    /* Setup sample array */
    assert(cinfo.rec_outbuf_height <= 8);
    for ( i = 0; i < cinfo.rec_outbuf_height; i++ )
        pSampArray[i] = pMemory + (cinfo.output_width * i);

    switch ( cinfo.out_color_space )
    {
    case JCS_YCbCr:
        DecompressYCbCr(&cinfo, pGraphicsMemory, surfaceMemory.pitch, pSampArray);
        break;
    case JCS_GRAYSCALE:
        DecompressGrayscale(&cinfo, pGraphicsMemory, surfaceMemory.pitch, pSampArray);
        break;
    case JCS_RGB:
        DecompressRGB(&cinfo, pGraphicsMemory, surfaceMemory.pitch, pSampArray);
        break;
    default:
        fprintf(stderr, "%s: Unsupported color space in file.  Currently supporting YCbCr, Grayscale, and RGB only.\n", pszFilename);
        goto err;
    }

    jpeg_finish_decompress(&cinfo);

    gettimeofday(&timeEnd, NULL);

    timeEnd.tv_usec -= timeStart.tv_usec;
    if ( timeEnd.tv_usec < 0 )
    {
        timeEnd.tv_usec += 1000000;
        timeEnd.tv_sec--;
    }
    timeEnd.tv_sec -= timeStart.tv_sec;
    printf("%s: Elapsed time to decode was %u.%02u seconds\n", pszFilename, (unsigned)timeEnd.tv_sec, (unsigned)timeEnd.tv_usec/10000);

    jpeg_destroy_decompress(&cinfo);
    if ( pFile )
        fclose(pFile);
    if ( fd >= 0 )
        close(fd);
    if ( pBuffer != MAP_FAILED )
        munmap(pBuffer, st.st_size);

    return returnSurface;

err:

    jpeg_destroy_decompress(&cinfo);
    if ( pFile )
        fclose(pFile);
    if ( fd >= 0 )
        close(fd);
    if ( pBuffer != MAP_FAILED )
        munmap(pBuffer, st.st_size);
    if ( returnSurface )
        NEXUS_Surface_Destroy(returnSurface);

    return NULL;
}

