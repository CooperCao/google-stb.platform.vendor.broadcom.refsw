/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "load_texture.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "berr.h"
#include "bkni.h"

#include <GLES2/gl2ext.h>

/*#define USE_NEXUS_PICTURE_DECODER*/

#ifdef USE_NEXUS_PICTURE_DECODER

#include "nexus_picture_decoder.h"

static NEXUS_PictureDecoderHandle   pictureDecoder = NULL;
static NEXUS_Graphics2DHandle       nexus_gfx = NULL;
static BKNI_EventHandle             blit_done_event = NULL;

static void blit_done(void *data, int unused)
{
   BSTD_UNUSED(unused);
   BKNI_SetEvent((BKNI_EventHandle)data);
}
#endif

void InitTextureLoader(void)
{
#ifdef USE_NEXUS_PICTURE_DECODER
   NEXUS_PictureDecoderOpenSettings    decoderSettings;
   NEXUS_Graphics2DOpenSettings        openSettings;
   NEXUS_Graphics2DSettings            gfxSettings;

   int pictureBufferSize;
   int multiScanBufferSize;
   void *pPictureBuffer;
   void *pMultiScanBuffer;

   if (pictureDecoder == NULL)
   {
      multiScanBufferSize = 1*1024*1024;
      pictureBufferSize = 1*1024*1024;
      pMultiScanBuffer = NULL;
      pPictureBuffer = NULL;

      NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
      decoderSettings.multiScanBufferSize = multiScanBufferSize;
      decoderSettings.bufferSize = pictureBufferSize;
      decoderSettings.multiScanBufferPtr = pMultiScanBuffer;
      decoderSettings.bufferPtr = pPictureBuffer;
      pictureDecoder = NEXUS_PictureDecoder_Open(0, &decoderSettings);
   }
   if (nexus_gfx == NULL)
   {
      NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
      openSettings.preAllocPacketMemory = true;
      openSettings.maxOperations = 10;
      nexus_gfx = NEXUS_Graphics2D_Open(0, &openSettings);

      /* Setup a blit finished callback */
      BKNI_CreateEvent(&blit_done_event);

      NEXUS_Graphics2D_GetSettings(nexus_gfx, &gfxSettings);
      gfxSettings.checkpointCallback.callback = blit_done;
      gfxSettings.checkpointCallback.context = blit_done_event;
      NEXUS_Graphics2D_SetSettings(nexus_gfx, &gfxSettings);
   }
#endif
}

void DestroyTextureLoader(void)
{
#ifdef USE_NEXUS_PICTURE_DECODER
   if (nexus_gfx != NULL)
   {
      NEXUS_Graphics2D_Close(nexus_gfx);
      nexus_gfx = NULL;
   }

   if (blit_done_event != NULL)
   {
      BKNI_DestroyEvent(blit_done_event);
      blit_done_event = NULL;
   }

   if (pictureDecoder != NULL)
   {
      NEXUS_PictureDecoder_Close(pictureDecoder);
      pictureDecoder = NULL;
   }
#endif
}

bool LoadTexture(const char *pictureFilename, GLint mipLevel)
{
#ifdef USE_NEXUS_PICTURE_DECODER
   NEXUS_SurfaceHandle           picture, rgba;
   NEXUS_PictureDecoderStatus    pictureStatus;
   NEXUS_PictureDecoderStartSettings   pictureSettings;
   NEXUS_SurfaceCreateSettings   createSettings;
   NEXUS_Graphics2DBlitSettings  blit_settings;
   NEXUS_SurfaceMemory           surfMem;
   NEXUS_Error                   err;

   FILE *fin;
   void *buffer;
   size_t size;
   int rc;
#endif

   if (strstr(pictureFilename, ".pkm"))
      return LoadCompressedTexture(pictureFilename, mipLevel);

#ifdef USE_NEXUS_PICTURE_DECODER
   if (pictureDecoder == NULL)
      return false;

   fin = fopen(pictureFilename, "rb");
   if (fin == NULL)
   {
      printf("Error opening texture file [%s]\n", pictureFilename);
      return false;
   }

   NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
   rc = fread(buffer, 1, size, fin); /* read file into the decoder's buffer */
   if (rc < 0)
   {
      printf("Error reading from texture file [%s]\n", pictureFilename);
      return false;
   }
   else if (rc == (int)size)
   {
      fseek(fin, 0, SEEK_END);
      if ((unsigned)ftell(fin) > size)
      {
         /* to decode larger file, requires to allocate larger buffer, see NEXUS_PictureDecoderOpenSettings bufferSize */
         printf("Picture file size %u is larger then buffer size %u, not supported\n", (unsigned)ftell(fin), (unsigned)size);
         return false;
      }
   }
   NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */

   NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
   if (strstr(pictureFilename, ".png"))
      pictureSettings.format = NEXUS_PictureFormat_ePng;
   else if (strstr(pictureFilename, ".jpg"))
      pictureSettings.format = NEXUS_PictureFormat_eJpeg;

   NEXUS_PictureDecoder_Start(pictureDecoder, &pictureSettings);

   do
   {
      NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
      if (pictureStatus.state == NEXUS_PictureDecoderState_eError)
      {
         printf("decoding failed\n");
         return false;
      }
      if (!pictureStatus.headerValid)
         usleep(10);
   }
   while (!pictureStatus.headerValid); /* wait for picture dimensions */

   /* create picture that could handle complete picture */
   NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
   createSettings.pixelFormat = pictureStatus.header.format;
   createSettings.width = pictureStatus.header.surface.width;
   createSettings.height = pictureStatus.header.surface.height;
   picture = NEXUS_Surface_Create(&createSettings);

   /* start decoding */
   NEXUS_PictureDecoder_DecodeSegment(pictureDecoder, picture, NULL);
   do
   {
      NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
      if (pictureStatus.state==NEXUS_PictureDecoderState_eError)
      {
         printf("decoding failed\n");
         return false;
      }
      if (pictureStatus.state != NEXUS_PictureDecoderState_eSegmentDone)
         usleep(10);
   }
   while (pictureStatus.state != NEXUS_PictureDecoderState_eSegmentDone); /* wait for picture to decode */

   NEXUS_PictureDecoder_Stop(pictureDecoder);

   /* This surface will be BGRA, we need RGBA, so blit to convert */
   NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
   createSettings.width = pictureStatus.header.surface.width;
   createSettings.height = pictureStatus.header.surface.height;
   createSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
   rgba = NEXUS_Surface_Create(&createSettings);

   NEXUS_Graphics2D_GetDefaultBlitSettings(&blit_settings);
   blit_settings.source.surface = picture;
   blit_settings.output.surface = rgba;
   NEXUS_Graphics2D_Blit(nexus_gfx, &blit_settings);

   err = NEXUS_Graphics2D_Checkpoint(nexus_gfx, NULL);
   if (err == NEXUS_GRAPHICS2D_QUEUED)
   {
      /* Wait for the blit to complete before continuing */
      BKNI_WaitForEvent(blit_done_event, 0xffffffff);
   }

   NEXUS_Surface_GetMemory(rgba, &surfMem);

   glTexImage2D(GL_TEXTURE_2D, mipLevel, GL_RGBA, createSettings.width, createSettings.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, surfMem.buffer);

   printf("Loaded %s\n", pictureFilename);

   /* Clean up */
   if (fin != NULL)
      fclose(fin);

   NEXUS_Surface_Destroy(picture);
   NEXUS_Surface_Destroy(rgba);

   return true;
#else
   return false;
#endif
}


void read_big_endian_2byte_word(unsigned short *blockadr, FILE *f)
{
   unsigned char bytes[2];
   unsigned short block;

   fread(&bytes[0], 1, 1, f);
   fread(&bytes[1], 1, 1, f);

   block = 0;
   block |= bytes[0];
   block = block << 8;
   block |= bytes[1];

   blockadr[0] = block;
}

#define ETC1_RGB_NO_MIPMAPS 0

bool LoadCompressedTexture(const char *pictureFilename, GLint mipLevel)
{
   /* Load Compressed file and use as texture */
   FILE *fp = fopen(pictureFilename, "rb");
   char magic[4];
   char version[2];
   unsigned short texture_type;
   unsigned short width;
   unsigned short height;
   unsigned short active_width;
   unsigned short active_height;
   void *texture;

   if (fp == NULL)
      return false;

   /* Read magic number */
   fread(&magic[0], sizeof(unsigned char), 1, fp);
   fread(&magic[1], sizeof(unsigned char), 1, fp);
   fread(&magic[2], sizeof(unsigned char), 1, fp);
   fread(&magic[3], sizeof(unsigned char), 1, fp);
   if (!(magic[0] == 'P' && magic[1] == 'K' && magic[2] == 'M' && magic[3] == ' '))
   {
      printf("\n\n The file %s is not a .pkm file.\n", pictureFilename);
      assert(0);
      return false;
   }

   /* Read version */
   fread(&version[0], sizeof(unsigned char), 1, fp);
   fread(&version[1], sizeof(unsigned char), 1, fp);
   if (!(version[0] == '1' && version[1] == '0'))
   {
      printf("\n\n The file %s is not of version 1.0 but of version %c.%c.\n", pictureFilename, version[0], version[1]);
      assert(0);
      return false;
   }

   /* Read texture type */
   read_big_endian_2byte_word(&texture_type, fp);
   if (!(texture_type == ETC1_RGB_NO_MIPMAPS))
   {
      printf("\n\n The file %s does not contain a texture of known format.\n", pictureFilename);
      printf("Known formats: ETC1_RGB_NO_MIPMAPS.\n");
      assert(0);
      return false;
   }

   read_big_endian_2byte_word(&width, fp);
   read_big_endian_2byte_word(&height, fp);

   read_big_endian_2byte_word(&active_width, fp);
   read_big_endian_2byte_word(&active_height, fp);

   /* allocate enough for 4bpp */
   texture = malloc((width * height * 4) / 8);
   fread(texture, sizeof(char), (width * height * 4) / 8, fp);

   fclose(fp);

   glCompressedTexImage2D(GL_TEXTURE_2D, mipLevel, GL_ETC1_RGB8_OES, active_width, active_height, 0,
                         (width * height * 4) / 8, texture);

   free(texture);

   return true;
}
