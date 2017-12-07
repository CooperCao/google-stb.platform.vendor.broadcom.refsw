/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "display_helpers.h"

bool NexusToBeglFormat(BEGL_BufferFormat *result, NEXUS_PixelFormat format)
{
   bool  ok = true;

   switch (format)
   {
   case NEXUS_PixelFormat_eA8_B8_G8_R8:
      *result = BEGL_BufferFormat_eA8B8G8R8;
      break;
   case NEXUS_PixelFormat_eR8_G8_B8_A8:
      *result = BEGL_BufferFormat_eR8G8B8A8;
      break;
   case NEXUS_PixelFormat_eX8_B8_G8_R8:
      *result = BEGL_BufferFormat_eX8B8G8R8;
      break;
   case NEXUS_PixelFormat_eR8_G8_B8_X8:
      *result = BEGL_BufferFormat_eR8G8B8X8;
      break;
   case NEXUS_PixelFormat_eR5_G6_B5:
      *result = BEGL_BufferFormat_eR5G6B5;
      break;
      // Note : the 4444 formats are not endian swapped like the 8888 formats
      // which is why the RGBA is transposed for these
   case NEXUS_PixelFormat_eR4_G4_B4_A4:
      *result = BEGL_BufferFormat_eA4B4G4R4;
      break;
   case NEXUS_PixelFormat_eR4_G4_B4_X4:
      *result = BEGL_BufferFormat_eX4B4G4R4;
      break;
   case NEXUS_PixelFormat_eA4_B4_G4_R4:
      *result = BEGL_BufferFormat_eR4G4B4A4;
      break;
   case NEXUS_PixelFormat_eX4_B4_G4_R4:
      *result = BEGL_BufferFormat_eR4G4B4X4;
      break;
   case NEXUS_PixelFormat_eR5_G5_B5_A1:
      *result = BEGL_BufferFormat_eA1B5G5R5;
      break;
   case NEXUS_PixelFormat_eR5_G5_B5_X1:
      *result = BEGL_BufferFormat_eX1B5G5R5;
      break;
   case NEXUS_PixelFormat_eA1_B5_G5_R5:
      *result = BEGL_BufferFormat_eR5G5B5A1;
      break;
   case NEXUS_PixelFormat_eX1_B5_G5_R5:
      *result = BEGL_BufferFormat_eR5G5B5X1;
      break;
   case NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08:
      *result = BEGL_BufferFormat_eYUV422;
      break;
   case NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8:
      *result = BEGL_BufferFormat_eBSTC;
      break;
   case NEXUS_PixelFormat_eUIF_R8_G8_B8_A8:
      *result = BEGL_BufferFormat_eTILED;
      break;
#ifdef YV12_NEXUS_TESTING
   case NEXUS_PixelFormat_eA8_Y8:
      // Note: There is no Nexus equivalent for YV12. We will use a 16-bit format that we don't
      // use anywhere else. It's 16bpp not 12bpp, but there are no 12-bit Nexus formats.
      *result = BEGL_BufferFormat_eYV12;
      break;
#endif
   default:
      ok = false;
      break;
   }

   return ok;
}

bool BeglToNexusFormat(NEXUS_PixelFormat *result, BEGL_BufferFormat format)
{
   bool  ok = true;

   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:
      *result = NEXUS_PixelFormat_eA8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR8G8B8A8:
      *result = NEXUS_PixelFormat_eR8_G8_B8_A8;
      break;
   case BEGL_BufferFormat_eX8B8G8R8:
      *result = NEXUS_PixelFormat_eX8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR8G8B8X8:
      *result = NEXUS_PixelFormat_eR8_G8_B8_X8;
      break;
   case BEGL_BufferFormat_eR5G6B5:
      *result = NEXUS_PixelFormat_eR5_G6_B5;
      break;
      // Note : the 4444 formats are not endian swapped like the 8888 formats
      // which is why the RGBA is transposed for these
   case BEGL_BufferFormat_eR4G4B4A4:
      *result = NEXUS_PixelFormat_eA4_B4_G4_R4;
      break;
   case BEGL_BufferFormat_eR4G4B4X4:
      *result = NEXUS_PixelFormat_eX4_B4_G4_R4;
      break;
   case BEGL_BufferFormat_eA4B4G4R4:
      *result = NEXUS_PixelFormat_eR4_G4_B4_A4;
      break;
   case BEGL_BufferFormat_eX4B4G4R4:
      *result = NEXUS_PixelFormat_eR4_G4_B4_X4;
      break;
   case BEGL_BufferFormat_eA1B5G5R5:
      *result = NEXUS_PixelFormat_eR5_G5_B5_A1;
      break;
   case BEGL_BufferFormat_eX1B5G5R5:
      *result = NEXUS_PixelFormat_eR5_G5_B5_X1;
      break;
   case BEGL_BufferFormat_eR5G5B5A1:
      *result = NEXUS_PixelFormat_eA1_B5_G5_R5;
      break;
   case BEGL_BufferFormat_eR5G5B5X1:
      *result = NEXUS_PixelFormat_eX1_B5_G5_R5;
      break;
   case BEGL_BufferFormat_eYUV422:
      *result = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
      break;
   case BEGL_BufferFormat_eBSTC:
      *result = NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8;
      break;
   case BEGL_BufferFormat_eTILED:
      *result = NEXUS_PixelFormat_eUIF_R8_G8_B8_A8;
      break;
#ifdef YV12_NEXUS_TESTING
   case BEGL_BufferFormat_eYV12:
      // Note: There is no Nexus equivalent for YV12. We will use a 16-bit format that we don't
      // use anywhere else. It's 16bpp not 12bpp, but there are no 12-bit Nexus formats.
      *result = NEXUS_PixelFormat_eA8_Y8;
      break;
#endif
   default:
      ok = false;
      break;
   }

   return ok;
}

int BeglFormatNumBytes(BEGL_BufferFormat format)
{
   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:
   case BEGL_BufferFormat_eR8G8B8A8:
   case BEGL_BufferFormat_eX8B8G8R8:
   case BEGL_BufferFormat_eR8G8B8X8:
   case BEGL_BufferFormat_eTILED:
      return 4;
   case BEGL_BufferFormat_eR5G6B5:
   case BEGL_BufferFormat_eR4G4B4A4:
   case BEGL_BufferFormat_eR4G4B4X4:
   case BEGL_BufferFormat_eA4B4G4R4:
   case BEGL_BufferFormat_eX4B4G4R4:
   case BEGL_BufferFormat_eA1B5G5R5:
   case BEGL_BufferFormat_eX1B5G5R5:
   case BEGL_BufferFormat_eR5G5B5A1:
   case BEGL_BufferFormat_eR5G5B5X1:
   case BEGL_BufferFormat_eBSTC:
   case BEGL_BufferFormat_eYUV422:
   case BEGL_BufferFormat_eYV12:       // This is really 12bpp, but we use a 16bpp Nexus surface to store it in
      return 2;
   default:
      return 0;
   }

   return 0;
}
