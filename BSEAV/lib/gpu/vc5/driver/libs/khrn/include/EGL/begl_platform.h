/*****************************************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************************************/

#pragma once

typedef enum
{
   BEGL_Success = 0,
   BEGL_Fail
} BEGL_Error;

typedef enum
{
   /* These formats are render target formats */
   BEGL_BufferFormat_eA8B8G8R8,
   BEGL_BufferFormat_eR8G8B8A8,
   BEGL_BufferFormat_eX8B8G8R8,
   BEGL_BufferFormat_eR8G8B8X8,
   BEGL_BufferFormat_eR5G6B5,

   BEGL_BufferFormat_eR4G4B4A4,
   BEGL_BufferFormat_eA4B4G4R4,
   BEGL_BufferFormat_eR4G4B4X4,
   BEGL_BufferFormat_eX4B4G4R4,

   BEGL_BufferFormat_eR5G5B5A1,
   BEGL_BufferFormat_eA1B5G5R5,
   BEGL_BufferFormat_eR5G5B5X1,
   BEGL_BufferFormat_eX1B5G5R5,

   /* non renderable input formats */
   BEGL_BufferFormat_eYV12,                  /* 3 planes layed out in Google format */
   BEGL_BufferFormat_eYUV422,                /* Single plane YUYV */
   BEGL_BufferFormat_eY8,                    /* 2 plane 8-bit striped video format - luma plane */
   BEGL_BufferFormat_eCr8Cb8,                /* 2 plane 8-bit striped video format - chroma plane */
   BEGL_BufferFormat_eCb8Cr8,                /* 2 plane 8-bit striped video format - chroma plane */
   BEGL_BufferFormat_eY10,                   /* 2 plane 10-bit striped video format - luma plane */
   BEGL_BufferFormat_eCr10Cb10,              /* 2 plane 10-bit striped video format - chroma plane */
   BEGL_BufferFormat_eCb10Cr10,              /* 2 plane 10-bit striped video format - chroma plane */

   /* renderable, but can only be used by the display and not re-read */
   BEGL_BufferFormat_eBSTC,

   /* A format which can be bound directly to the texture target without
    * requiring internal format conversions by the OpenGL driver.
    *
    * This is available only on Nexus based platform implementations and is
    * mapped to NEXUS_PixelFormat_eUIF_R8_G8_B8_A8, which has been coded in
    * Nexus and Magnum to produce memory layouts directly compatible
    * with V3D, the standard M2MC and mipmap M2MC hardware on supported SoCs.
    *
    * - Pixmaps of this type may be created with or without a mipchain.
    * - Only level 0 will be renderable by V3D
    * - Only pixmaps without a mipchain will be readable by the standard M2MC,
    *   i.e. via the Nexus 2D graphics blit APIs or Nexus surface compositor
    * - pixmaps with a mipchain can have the mipchain populated using the
    *   mipmap specific variant of the M2MC on SoCs where that is available
    * - pixmaps with a mipchain can not be read by any of the M2MCs
    * - Not all level 0 image sizes are supported to ensure compatibility
    *   between the two sides, this currently means the minimum width
    *   and height are both limited to 64 pixels.
    */
   BEGL_BufferFormat_eTILED,

   /* Can be used to return back an invalid format */
   BEGL_BufferFormat_INVALID
} BEGL_BufferFormat;

typedef enum
{
   BEGL_Colorimetry_RGB = 0,     // RGB must be zero
   BEGL_Colorimetry_BT_709,
   BEGL_Colorimetry_Unknown,
   BEGL_Colorimetry_Dvi_Full_Range_RGB,
   BEGL_Colorimetry_FCC,
   BEGL_Colorimetry_BT_470_2_BG,
   BEGL_Colorimetry_Smpte_170M,
   BEGL_Colorimetry_Smpte_240M,
   BEGL_Colorimetry_XvYCC_709,
   BEGL_Colorimetry_XvYCC_601,
   BEGL_Colorimetry_BT_2020_NCL,
   BEGL_Colorimetry_BT_2020_CL,
   BEGL_Colorimetry_Hdmi_Full_Range_YCbCr
} BEGL_Colorimetry;
