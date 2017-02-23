/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef _LOAD_PKM_H__
#define _LOAD_PKM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   unsigned int   mWidth;
   unsigned int   mHeight;
   unsigned int   mSize;
   unsigned char  *mBytes;
   unsigned int   mFormat;    /* This is actually a GL enum, but we dont include any GL stuff in texture_io.
                                 Its supposed to work without having a GL implementation */
} PKMImage;

int LoadPKM(const char *fname, PKMImage *data);
void FreePKM(PKMImage *data);


typedef enum
{
   DF_565 = 0,
   DF_5551,
   DF_4444,
   DF_888,
   DF_8888,
   DF_ALPHA
} DEST_FORMAT;

typedef struct
{
   unsigned int   mWidth;
   unsigned int   mHeight;
   unsigned int   mStride;
   unsigned char  *mBytes;
   DEST_FORMAT    mFormat;
   void           *context;         /* an implementation dependant area to hang data off */
} PNGImage;

#define RGB565

int LoadPNG(const char *fname, int destFormat, PNGImage *data);
int SavePNG(const char *fname, PNGImage *data);
void FreePNG(PNGImage *data);



#ifdef __cplusplus
}

#endif

#endif /* _LOAD_PKM_H__ */
