/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

/* to handle 10-bit video:
      [C0 C1 C2] --> 32-bit
 */
/* truncated 10-bit to 8-bit dump for MSB */
#define BTST_10BIT_GET_C0(dw) (((dw) & 0x3FF00000) >> 22)
#define BTST_10BIT_GET_C1(dw) (((dw) &    0xFFC00) >> 12)
#define BTST_10BIT_GET_C2(dw) (((dw) &      0x3FF) >> 2)
/* frame buffer pixels stored in byte stream format file, so no byte swap. */
/* Write the UV data (this also deinterleaves the UV  writing first U than V */
#define BTST_DEINTERLEAVE_UV(base, ptr, offset, val) {\
   if(ptr & 1) {\
       *((uint8_t*)(base)+ ptr/2 + offset) = val;\
   } else {\
       *((uint8_t*)(base)+ ptr/2) = val;\
   }\
   ptr++;\
}

/* SCB8.0/MAP8.0: When (ByteAddr[8] ^ ByteAddr[9]) == 1, ByteAddr[5] is inverted */
/* MAP 8.0/5.0 jword address shuffling */
#define BIT_EXTRACT(map8, VALUE, MSB, LSB)  ((map8)?(((VALUE>>9) & 1) ^ ((VALUE>>8) & 1))\
    :((VALUE>>LSB) & ((1<<(MSB-LSB+1))-1)))

void usage(char *argv)
{
    printf("This tool convert XVD luma/chroma buffer image dump files (default luma.img and chroma.img) into a combined raster order YUV 420 binary image file (default img.yuv) and YUV422 ascii file (default yuv422).\n");
    printf("Usage:\n");
    printf("\t%s [options]\n", argv);
    printf("\nOptions:\n");
    printf("-h                To print this usage.\n");
    printf("-dbg              To enable debug print of destriping info;\n");
    printf("-size W H         To specify picture size W and H;\n");
    printf("-stripeWidth N    To specify stripe width in N bytes;\n");
    printf("-yNMBY N          To specify luma buffer NMBY value;\n");
    printf("-cNMBY N          To specify chroma buffer NMBY value;\n");
    printf("-10bit            To specify the input y/c image files are 10-bit video;\n");
    printf("-shuffle          To specify the DDR address shuffling design for SCB 5.0(7445) or newer decoder;\n");
    printf("-map8             To specify the DDR address blind shuffling design for SCB 8.0(7271) or newer decoders;\n");
    printf("-y LUMA_FILE      To specify input luma buffer file;\n");
    printf("-c CHROMA_FILE    To specify input chroma buffer file;\n");
    printf("-o YUV_FILE       To specify output YUV binary file;\n");
    printf("-422 YUV422_FILE  To specify output YUV422 ASCii file;\n");
}

int main(int argc, char **argv) {
#define uint8_t  u_int8_t
#define uint32_t u_int32_t

    unsigned width  = 192;
    unsigned height = 128;
    unsigned uv_offset = (width/2) * (height/2);
    unsigned stripeWidth = 128;
    int      bDbg = 0;
    int      b10Bit = 0;
    int      bShuffle = 0; /* SCB 5.0 or newer decoder DDR MAP 5.0 or later has shuffling on JWords address */
    int      map8 = 0; /* SCB 8.0 decoder DDR MAP 8.0 has blind shuffling on DWord order within JWord */
    unsigned shuffleBitShift = (stripeWidth==256) ? 9 : 8;
    unsigned pixelsPerStripe = stripeWidth;
    unsigned pixelsPerGroup = b10Bit? 3:1;
    unsigned totalStripes = (width + pixelsPerStripe - 1) / pixelsPerStripe;
    unsigned lumaOffset = 0, chromaOffset = 0;
    unsigned line = 0;
    unsigned stripe = 0;
    unsigned pixel = 0;
    unsigned i = 0;
    unsigned yNMBY = 22, cNMBY=6;
    char yName[256] = "luma.img";/* XVD luma buffer binary dump */
    char cName[256] = "chroma.img";/* XVD chroma buffer binary dump */
    char yuvName[256] = "img.yuv";/*binary YUV file to be viewed by SMplayer.exe */
    char yuv422Name[256] = "yuv422";/* ascii yuv file to be converted to ppm file by yuv2ppm tool */
    FILE *fpLuma   = NULL;
    FILE *fpChroma = NULL;
    FILE *fpYUV    = NULL;
    FILE *yuvFile  = NULL;
    unsigned yptr=0, cptr=0;
    static unsigned char pFromY[(4096+5)/6*8*150*16], pFromC[(4096+5)/6*8*76*16];
    static unsigned char pToY[4096*2176],   pToC[4096*1200];

    for(i=1; i<argc; i++) {
        if(!strcmp("-h",argv[i])) {
            usage(argv[0]);
            return;
        }
        if(!strcmp("-size",argv[i])) {
            width  = atoi(argv[++i]);
            height = atoi(argv[++i]);
            uv_offset = (width/2) * (height/2);
        }
        if(!strcmp("-stripeWidth",argv[i])) {
            stripeWidth  = atoi(argv[++i]);
        }
        if(!strcmp("-yNMBY",argv[i])) {
            yNMBY  = atoi(argv[++i]);
        }
        if(!strcmp("-cNMBY",argv[i])) {
            cNMBY  = atoi(argv[++i]);
        }
        if(!strcmp("-dbg",argv[i])) {
            bDbg  = 1;
            fprintf(stderr, "Enabled debug messages.\n");
        }
        if(!strcmp("-10bit",argv[i])) {
            b10Bit  = 1;
        }
        if(!strcmp("-shuffle",argv[i])) {
            bShuffle  = 1;
        }
        if(!strcmp("-map8",argv[i])) {
            map8  = 1;
            bShuffle  = 1;
        }
        if(!strcmp("-y",argv[i])) {
            strcpy(yName, argv[++i]);
        }
        if(!strcmp("-c",argv[i])) {
            strcpy(cName, argv[++i]);
        }
        if(!strcmp("-o",argv[i])) {
            strcpy(yuvName, argv[++i]);
        }
        if(!strcmp("-422",argv[i])) {
            strcpy(yuv422Name, argv[++i]);
        }
    }
    fprintf(stderr, "Input:\n");
    fprintf(stderr, "\tluma file   : %s\n", yName);
    fprintf(stderr, "\tchroma file : %s\n", cName);
    fprintf(stderr, "\tsize        : %u x %u\n", width, height);
    fprintf(stderr, "\tstripe width: %u bytes\n", stripeWidth);
    fprintf(stderr, "\tyNMBY       : %u\n", yNMBY);
    fprintf(stderr, "\tcNMBY       : %u\n", cNMBY);
    fprintf(stderr, "\tDDR address %s shuffling\n", bShuffle?"with" : "without");
    fprintf(stderr, "\t%u-bit video\n", b10Bit? 10:8);
    fprintf(stderr, "Output:\n");
    fprintf(stderr, "\tYUV binary image file: %s\n", yuvName);
    fprintf(stderr, "\tYUV422 ASCii file    : %s.\n", yuv422Name);
    shuffleBitShift = (stripeWidth==256) ? 9 : 8;
    pixelsPerStripe = b10Bit ? stripeWidth / 8 * 6 : stripeWidth;
    pixelsPerGroup = b10Bit? 3 : 1;
    totalStripes = (width + pixelsPerStripe - 1) / pixelsPerStripe;

    if (yuvFile== NULL)
    {
      if ((yuvFile=fopen(yuv422Name, "w"))==NULL)
      {
         printf("Could not open %s\n", yuv422Name);
         return -1;
      }
    }
    if (fpYUV== NULL)
    {
      if ((fpYUV=fopen(yuvName, "wb"))==NULL)
      {
         printf("Could not open %s\n", yuvName);
         return -1;
      }
    }
    if (fpLuma== NULL)
    {
      if ((fpLuma=fopen(yName, "rb"))==NULL)
      {
         printf("Could not open %s\n", yName);
         return -1;
      }
    }
    if (fpChroma== NULL)
    {
      if ((fpChroma=fopen(cName, "rb"))==NULL)
      {
         printf("Could not open %s\n", cName);
         return -1;
      }
    }
    /* print header */
    fprintf(yuvFile, "%x\n",width);
    fprintf(yuvFile, "%x\n",height);

    /* process luma */
    fread(pFromY, 1, yNMBY*16*stripeWidth*totalStripes, fpLuma);
    fread(pFromC, 1, cNMBY*16*stripeWidth*totalStripes, fpChroma);

    if(bDbg) {
        fprintf(stderr, "===============================================================\n");
        fprintf(stderr, "Y@:C@(shuffle?) | line | pixel | stripe | [Y#:C#] | [Y:C]\n",line);
        fprintf(stderr, "---------------------------------------------------------------\n");
    }
    /* line by line dump */
    for (line=0; line < height; line++){
      if(bDbg) fprintf(stderr, "Line %d\n",line);
      fprintf(yuvFile, "// line %d\n",line);
      for (pixel=0; pixel < width; pixel+=pixelsPerGroup){
         /* determine which stripe contains this pixel */
         stripe = pixel / pixelsPerStripe;

         if(b10Bit) {
            uint32_t lumaDword, chromaDword;

            /* calculate the pixel group's luma byte addr */
            lumaOffset   = (pixel % pixelsPerStripe) / 3 * 4 + /* 6-pixel per 8-byte group */
               (line * stripeWidth) +
               (stripe * stripeWidth * yNMBY * 16);
            chromaOffset = (pixel % pixelsPerStripe) / 3 * 4 +
               (line/2) * stripeWidth + /* 420 */
               (stripe * stripeWidth * cNMBY * 16);
            if(bShuffle) {
                lumaOffset   = BIT_EXTRACT(map8, lumaOffset, shuffleBitShift, shuffleBitShift)? (lumaOffset ^ 0x20):lumaOffset;
                chromaOffset = BIT_EXTRACT(map8, chromaOffset, shuffleBitShift, shuffleBitShift)? (chromaOffset ^ 0x20):chromaOffset;
            }
            /* dword addressing; */
            lumaDword   = (*((uint32_t*)(pFromY + lumaOffset)));
            chromaDword = (*((uint32_t*)(pFromC + chromaOffset)));
            if(bDbg) fprintf(stderr, "%04x:%04x(%u:%u) %4u %4u %4u; [%6u:%6u]; [%08x:%08x]\n",lumaOffset, chromaOffset,
               BIT_EXTRACT(map8, lumaOffset, shuffleBitShift, shuffleBitShift), BIT_EXTRACT(map8, chromaOffset, shuffleBitShift, shuffleBitShift),
               line, pixel, stripe, yptr, cptr, lumaDword, chromaDword);
            if (pixel < width){
                pToY[yptr++] = BTST_10BIT_GET_C0(lumaDword);
                if((line & 1) == 0) /* write 420 UV */{
                    BTST_DEINTERLEAVE_UV(pToC, cptr, uv_offset, BTST_10BIT_GET_C0(chromaDword));
                }
                fprintf(yuvFile, "%02x%02x\n", BTST_10BIT_GET_C0(lumaDword),BTST_10BIT_GET_C0(chromaDword));/* Y0Cb0 */
            } else break;
            if (pixel+1 < width){
                pToY[yptr++] = BTST_10BIT_GET_C1(lumaDword);
                if((line & 1) == 0) /* write 420 UV */{
                    BTST_DEINTERLEAVE_UV(pToC, cptr, uv_offset, BTST_10BIT_GET_C1(chromaDword));
                }
                fprintf(yuvFile, "%02x%02x\n", BTST_10BIT_GET_C1(lumaDword),BTST_10BIT_GET_C1(chromaDword));/* Y1Cr0 */
            } else break;
            if (pixel+2 < width){
                pToY[yptr++] = BTST_10BIT_GET_C2(lumaDword);
                if((line & 1) == 0) /* write 420 UV */{
                    BTST_DEINTERLEAVE_UV(pToC, cptr, uv_offset, BTST_10BIT_GET_C2(chromaDword));
                }
                fprintf(yuvFile, "%02x%02x\n", BTST_10BIT_GET_C2(lumaDword),BTST_10BIT_GET_C2(chromaDword));/* Y2Cb2 */
            }
         } else {
            /* calculate the pixel's luma byte offset */
            lumaOffset = (pixel % pixelsPerStripe) +
                (line * stripeWidth) +
                (stripe * stripeWidth * yNMBY * 16);
            chromaOffset = (pixel % pixelsPerStripe) +
                (line/2) * stripeWidth + /* 420 */
                (stripe * stripeWidth * cNMBY * 16);
            lumaOffset   = lumaOffset ^ 0x03;/* byte swap */
            chromaOffset = chromaOffset ^ 0x03;/* byte swap */

            if(bShuffle) {
                lumaOffset   = BIT_EXTRACT(map8, lumaOffset, shuffleBitShift, shuffleBitShift)? (lumaOffset ^ 0x20):lumaOffset;
                chromaOffset = BIT_EXTRACT(map8, chromaOffset, shuffleBitShift, shuffleBitShift)? (chromaOffset ^ 0x20):chromaOffset;
            }
            if(bDbg) fprintf(stderr, "%06x:%06x(%u:%u) %4u %4u %4u; [%6u:%6u] [%02x:%02x]\n",lumaOffset, chromaOffset,
               BIT_EXTRACT(map8, lumaOffset, shuffleBitShift, shuffleBitShift), BIT_EXTRACT(map8, chromaOffset, shuffleBitShift, shuffleBitShift),
               line, pixel, stripe, yptr, cptr, pFromY[lumaOffset], pFromC[chromaOffset]);
            fprintf(yuvFile, "%02x%02x\n", pFromY[lumaOffset], pFromC[chromaOffset]);/* Y0Cb0 then Y1Cr0 required by yuv2ppm tool */
            /* byte addressing */
            pToY[yptr++] = pFromY[lumaOffset];
            if((line & 1) == 0) /* write 420 UV */{
                BTST_DEINTERLEAVE_UV(pToC, cptr, uv_offset, pFromC[chromaOffset]);
            }
         }
      }
    }

    fprintf(stderr, "\nY count = %u; C count = %u\n", yptr, cptr);
    fwrite(pToY, 1, yptr, fpYUV);
    fwrite(pToC, 1, cptr, fpYUV);

    fclose(fpChroma);
    fclose(fpLuma);
    fclose(fpYUV);
    fclose(yuvFile);
    return 0;
}
/* end of file */
