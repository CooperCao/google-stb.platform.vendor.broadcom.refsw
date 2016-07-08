/******************************************************************************
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
******************************************************************************/

#include "dynrng_osd_priv.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define USE_RGB565 0

#define R5G3 (wr + 0)
#define G3B5 (wr + 1)
#define R8 (rd + 0)
#define G8 (rd + 1)
#define B8 (rd + 2)
int OSD_ConvertRgb888ToRgb565(OSD_ImageHandle image)
{
    int rc = 0;
    unsigned rd = 0;
    unsigned wr = 0;

    if (image->bytes_per_pixel != 3) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto end; }
    fprintf(stderr, "OSD PGM converting to 565\n");

    for (rd = 0, wr = 0; rd < image->bytes_per_pixel * image->width * image->height; wr += 2, rd += 4)
    {
        image->pixel_data[R5G3] = (image->pixel_data[R8] & 0xf8) << 0;
        image->pixel_data[R5G3] |= (image->pixel_data[G8] & 0xe0) >> 5;
        image->pixel_data[G3B5] = (image->pixel_data[G8] & 0x1c) << 3;
        image->pixel_data[G3B5] |= (image->pixel_data[B8] & 0xf8) >> 3;

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
        {
            /* swap bytes */
            unsigned char temp;
            temp = image->pixel_data[G3B5];
            image->pixel_data[G3B5] = image->pixel_data[R5G3];
            image->pixel_data[R5G3] = temp;
        }
#endif
    }

#if DEBUG
    {
        unsigned ch = 0;
        fprintf(stderr, "  \"");
        for (rd = 0; rd < 2 * image->width * image->height; rd++)
        {
            if (isprint(image->pixel_data[rd]))
            {
                ch += fprintf(stderr, "%c", image->pixel_data[rd]);
            }
            else
            {
                ch += fprintf(stderr, "\\%o", image->pixel_data[rd]);
            }

            if (ch > 71)
            {
                fprintf(stderr, "\"\n  \"");
                ch = 0;
            }
        }
        fprintf(stderr, "\"\n");
    }
#endif

    image->bytes_per_pixel = 2;

end:
    return rc;
}

int OSD_ParsePgmLine(char * line, OSD_ImageHandle image)
{
    int rc = 0;
    static int state = 0;
    static int maxValue = 1;
    char * t;
    static unsigned i = 0;

    if (!image->bytes_per_pixel)
    {
        state = 0;
        maxValue = 1;
        i = 0;
    }

    t = strtok(line, " \t\n");
    while (t)
    {
#if DEBUG
        fprintf(stderr, "pgmparser: t = '%s'\n", t);
        fprintf(stderr, "pgmparser: state = %d\n", state);
#endif
        switch (state)
        {
            case 0:
                if (strlen(t) >= 2)
                {
                    if (t[0] == 'P')
                    {
#if DEBUG
                        fprintf(stderr, "pgmparser: found 'P'\n");
#endif
                        switch (t[1])
                        {
                            case '2':
                                image->bytes_per_pixel = 1;
                                state = 1;
                                break;
                            case '3':
                                image->bytes_per_pixel = 4;
                                state = 1;
                                break;
                            default:
                                state = 0;
                                break;
                        }
#if DEBUG
                        fprintf(stderr, "pgmparser: bpp = %d\n", image->bytes_per_pixel);
#endif
                    }
                }
                else if (t[0] == '#')
                {
#if DEBUG
                    fprintf(stderr, "pgmparser: comment\n");
#endif
                    /* skip the rest of this line */
                    goto end;
                }
                else
                {
                    rc = 1;
                    goto end;
                }
                break;
            case 1:
                if (t[0] == '#')
                {
#if DEBUG
                    fprintf(stderr, "pgmparser: comment\n");
#endif
                    /* skip the rest of this line */
                    goto end;
                }
                else
                {
                    image->width = atoi(t);
                    if (image->width)
                    {
#if DEBUG
                        fprintf(stderr, "pgmparser: width = %d\n", image->width);
#endif
                        state = 2;
                    }
                    else
                    {
                        rc = 1;
                        goto end;
                    }
                }
                break;
            case 2:
                if (t[0] == '#')
                {
#if DEBUG
                    fprintf(stderr, "pgmparser: comment\n");
#endif
                    /* skip the rest of this line */
                    goto end;
                }
                else
                {
                    image->height = atoi(t);
                    if (image->height)
                    {
#if DEBUG
                        fprintf(stderr, "pgmparser: height = %d\n", image->height);
#endif
                        if (!image->pixel_data)
                        {
                            image->pixel_data = malloc(image->bytes_per_pixel * image->width * image->height);
                            if (!image->pixel_data) { rc = 1; goto end; }
#if DEBUG
                            fprintf(stderr, "pgmparser: allocated %u bytes @ %p for pixel data\n", image->bytes_per_pixel * image->width * image->height, (void *)image->pixel_data);
#endif
                        }
                        state = 3;
                    }
                    else
                    {
                        rc = 1;
                        goto end;
                    }
                }
                break;
            case 3:
                if (t[0] == '#')
                {
#if DEBUG
                    fprintf(stderr, "pgmparser: comment\n");
#endif
                    /* skip the rest of this line */
                    goto end;
                }
                else
                {
                    maxValue = atoi(t);
                    if (maxValue)
                    {
#if DEBUG
                        fprintf(stderr, "pgmparser: maxValue = %d\n", maxValue);
#endif
                        state = 4;
                    }
                    else
                    {
                        rc = 1;
                        goto end;
                    }
                }
                break;
            case 4:
                if (t[0] == '#')
                {
#if DEBUG
                    fprintf(stderr, "pgmparser: comment\n");
#endif
                    /* skip the rest of this line */
                    goto end;
                }
                else
                {
                    if (i < image->bytes_per_pixel * image->width * image->height)
                    {
                        image->pixel_data[i] = atoi(t);
                        switch (image->bytes_per_pixel)
                        {
                            case 4:
                                state = 5;
                                break;
                            case 1:
#if DEBUG
                                fprintf(stderr, "pgmparser: pixel[%d] = %d\n", i, image->pixel_data[i]);
#endif
                                break;
                            default:
                                break;
                        }
                        i++;
                    }
                    else
                    {
#if DEBUG
                        fprintf(stderr, "pgmparser: more data past size specified\n");
#endif
                        /* can't parse any more so end it */
                        goto end;
                    }
                }
                break;
            case 5:
                if (t[0] == '#')
                {
#if DEBUG
                    fprintf(stderr, "pgmparser: comment\n");
#endif
                    /* skip the rest of this line */
                    goto end;
                }
                else
                {
                    if (i < image->bytes_per_pixel * image->width * image->height)
                    {
                        image->pixel_data[i] = atoi(t);
                        i++;
                        state = 6;
                    }
                    else
                    {
#if DEBUG
                        fprintf(stderr, "pgmparser: more data past size specified\n");
#endif
                        /* can't parse any more so end it */
                        goto end;
                    }
                }
                break;
            case 6:
                if (t[0] == '#')
                {
#if DEBUG
                    fprintf(stderr, "pgmparser: comment\n");
#endif
                    /* skip the rest of this line */
                    goto end;
                }
                else
                {
                    if (i < image->bytes_per_pixel * image->width * image->height)
                    {
                        image->pixel_data[i++] = atoi(t);
                        image->pixel_data[i] = 0xFF; /* alpha */
#if DEBUG
                        fprintf(stderr, "pgmparser: pixel[%d] = (%d, %d, %d, %d)\n", i / 4,
                            image->pixel_data[i - 3],
                            image->pixel_data[i - 2],
                            image->pixel_data[i - 1],
                            image->pixel_data[i - 0]);
#endif
                        i++;
                        state = 4;
                    }
                    else
                    {
#if DEBUG
                        fprintf(stderr, "pgmparser: more data past size specified\n");
#endif
                        /* can't parse any more so end it */
                        goto end;
                    }
                }
                break;
            default:
#if DEBUG
                fprintf(stderr, "pgmparser: bad state: %d\n", state);
#endif
                break;
        }
        t = strtok(NULL, " \t\n");
    }

end:
    return rc;
}

OSD_ImageHandle OSD_LoadPgmImage(const char * pgmFilename)
{
    OSD_ImageHandle image = NULL;
    FILE * pgm = NULL;
    char buf[512];

    pgm = fopen(pgmFilename, "r");
    if (!pgm) goto error;

    image = malloc(sizeof(struct OSD_Image));
    if (!image) goto error;
    memset(image, 0, sizeof(struct OSD_Image));

    while (fgets(buf, 512, pgm))
    {
        if (OSD_ParsePgmLine(buf, image)) goto error;
    }

    fprintf(stdout, "Loaded '%s'\n", pgmFilename);

#if USE_RGB565
    if (OSD_ConvertRgb888ToRgb565(image)) goto error;
#endif

end:
    return image;

error:
    if (image)
    {
        if (image->pixel_data)
        {
            free(image->pixel_data);
        }
        free(image);
        image = NULL;
    }
    goto end;
}

OSD_ImageHandle OSD_GetImageById(OSD_ImageId id)
{
    OSD_ImageHandle image = NULL;

    switch (id)
    {
        case OSD_ImageId_eHlg:
            image = OSD_LoadPgmImage("dynrng-resources/images/arib.ppm");
            break;
        case OSD_ImageId_eHdr10:
            image = OSD_LoadPgmImage("dynrng-resources/images/smpte.ppm");
            break;
        case OSD_ImageId_eSdr:
            image = OSD_LoadPgmImage("dynrng-resources/images/sdr.ppm");
            break;
        case OSD_ImageId_eUnknown:
            image = OSD_LoadPgmImage("dynrng-resources/images/unknown.ppm");
            break;
        case OSD_ImageId_eYes:
            image = OSD_LoadPgmImage("dynrng-resources/images/yes.ppm");
            break;
        case OSD_ImageId_eNo:
            image = OSD_LoadPgmImage("dynrng-resources/images/no.ppm");
            break;
        case OSD_ImageId_eInput:
            image = OSD_LoadPgmImage("dynrng-resources/images/input.ppm");
            break;
        case OSD_ImageId_eOutput:
            image = OSD_LoadPgmImage("dynrng-resources/images/output.ppm");
            break;
        case OSD_ImageId_eTv:
            image = OSD_LoadPgmImage("dynrng-resources/images/tv.ppm");
            break;
        default:
            break;
    }

    return image;
}
