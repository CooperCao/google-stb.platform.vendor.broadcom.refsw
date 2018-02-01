/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "texture_data.h"

namespace video_texturing
{

////////////////////////////////////////////////////////////////////////////////////////////////
// TextureData : Encapsulates the surface and texture data for EGLimage based texturing.
////////////////////////////////////////////////////////////////////////////////////////////////

// This thread waits for each fence (oldest first) and recycles buffers back to the decoder
// as soon as the fence fires.
static void BufferReleaseThread(TextureData *texture)
{
   while (!texture->ExitReleaseThread())
   {
      if (!texture->WaitForOldestBufferFenceAndRecycle())
         std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
}

void TextureData::Create(NXPL_PlatformHandle platform, NEXUS_Graphics2DHandle gfx2d,
                         BKNI_EventHandle m2mcDone, uint32_t numBuffers,
                         uint32_t mediaW, uint32_t mediaH, uint32_t texW, uint32_t texH,
                         uint32_t numMiplevels, BEGL_BufferFormat format,
                         bool aniso, bool secure)
{
   m_platform = platform;
   m_gfx2d    = gfx2d;
   m_m2mcDone = m2mcDone;
   m_format   = format;

   // Init the EGLimage extensions we need
   InitGLExtensions();

   BEGL_PixmapInfoEXT pixInfo{};
   NXPL_GetDefaultPixmapInfoEXT(&pixInfo);
   pixInfo.width  = texW;
   pixInfo.height = texH;
   pixInfo.secure = secure;
   pixInfo.format = format;
#if VC5
   pixInfo.miplevels = numMiplevels;
#endif

   // Make data for each buffer
   m_data.resize(numBuffers);

   // range based for not valid on GCC4.5
   for (std::vector<PerBufferData>::iterator it = m_data.begin(); it != m_data.end(); ++it)
   {
      glGenTextures(1, &it->textureID);

      if (!NXPL_CreateCompatiblePixmapEXT(platform, &it->eglPixmap, &it->nativePixmap, &pixInfo))
         throw "Failed during NXPL_CreateCompatiblePixmapEXT";

      // Fill the buffers so we don't draw garbage in the first frame or two
      NEXUS_Graphics2DFillSettings fillSettings;
      NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
      fillSettings.surface = it->nativePixmap;
      fillSettings.rect.width  = texW;
      fillSettings.rect.height = texH;
      fillSettings.color = 0;
      NEXUS_Graphics2D_Fill(m_gfx2d, &fillSettings);
      WaitForM2MCCompletion();

      EGLint attrList[] = { EGL_NONE };

      // Wrap the native pixmap (actually a NEXUS_Surface) as an EGLImage
      it->eglImage = m_eglCreateImageKHRFunc(eglGetCurrentDisplay(), EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,
                                             (EGLClientBuffer)it->eglPixmap, attrList);
      if (it->eglImage == EGL_NO_IMAGE_KHR)
         throw "Failed to create EGLImage";

      // Bind the EGL images as textures, and set filtering
      glBindTexture(GL_TEXTURE_2D, it->textureID);
      m_glEGLImageTargetTexture2DOESFunc(GL_TEXTURE_2D, it->eglImage);

      // Ensure we don't filter using values outside the texture
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      // Use bi-linear filtering
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_ES_VERSION_3_0 // At least ES3
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

      if (aniso && numMiplevels > 1)
      {
         GLfloat fLargest;
         glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
      }

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMiplevels - 1);
#else
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
   }

   m_bufferReleaseThread = std::thread(BufferReleaseThread, this);
}

void TextureData::Destroy()
{
   m_exitReleaseThread = true;
   m_bufferReleaseThread.join();

   // range based for not valid on GCC4.5
   for (std::vector<PerBufferData>::iterator it = m_data.begin(); it != m_data.end(); ++it)
   {
      it->fence.Destroy();
      glDeleteTextures(1, &it->textureID);
      m_eglDestroyImageKHRFunc(eglGetCurrentDisplay(), it->eglImage);
      NXPL_DestroyCompatiblePixmap(m_platform, it->eglPixmap);
   }
}

#ifdef VC5
// This should only be called on YUV surface types
static BEGL_Colorimetry NexusToBEGLColorPrimaries(NEXUS_MatrixCoefficients nmc)
{
   switch (nmc)
   {
   case NEXUS_MatrixCoefficients_eHdmi_RGB              : return BEGL_Colorimetry_RGB;
   case NEXUS_MatrixCoefficients_eItu_R_BT_709          : return BEGL_Colorimetry_BT_709;
   case NEXUS_MatrixCoefficients_eUnknown               : return BEGL_Colorimetry_Unknown;
   case NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB    : return BEGL_Colorimetry_Dvi_Full_Range_RGB;
   case NEXUS_MatrixCoefficients_eFCC                   : return BEGL_Colorimetry_FCC;
   case NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG     : return BEGL_Colorimetry_BT_470_2_BG;
   case NEXUS_MatrixCoefficients_eSmpte_170M            : return BEGL_Colorimetry_Smpte_170M;
   case NEXUS_MatrixCoefficients_eSmpte_240M            : return BEGL_Colorimetry_Smpte_240M;
   case NEXUS_MatrixCoefficients_eXvYCC_709             : return BEGL_Colorimetry_XvYCC_709;
   case NEXUS_MatrixCoefficients_eXvYCC_601             : return BEGL_Colorimetry_XvYCC_601;
   case NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL     : return BEGL_Colorimetry_BT_2020_NCL;
   case NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL      : return BEGL_Colorimetry_BT_2020_CL;
   case NEXUS_MatrixCoefficients_eHdmi_Full_Range_YCbCr : return BEGL_Colorimetry_Hdmi_Full_Range_YCbCr;
   default                                              : return BEGL_Colorimetry_XvYCC_601;
   }
}

void TextureData::DetermineColorimetry(NEXUS_MatrixCoefficients nmc)
{
   if (m_colorimetryValid)
      return;

   BEGL_Colorimetry colorimetry = NexusToBEGLColorPrimaries(nmc);
   m_colorimetryValid = true;

   printf("Surface reports ");
   switch (colorimetry)
   {
   case BEGL_Colorimetry_RGB                   : printf("RGB"); break;
   case BEGL_Colorimetry_BT_709                : printf("BT_709"); break;
   case BEGL_Colorimetry_Unknown               : printf("Unknown"); break;
   case BEGL_Colorimetry_Dvi_Full_Range_RGB    : printf("Dvi_Full_Range_RGB"); break;
   case BEGL_Colorimetry_FCC                   : printf("FCC"); break;
   case BEGL_Colorimetry_BT_470_2_BG           : printf("BT_470_2_BG"); break;
   case BEGL_Colorimetry_Smpte_170M            : printf("Smpte_170M"); break;
   case BEGL_Colorimetry_Smpte_240M            : printf("Smpte_240M"); break;
   case BEGL_Colorimetry_XvYCC_709             : printf("XvYCC_709"); break;
   case BEGL_Colorimetry_XvYCC_601             : printf("XvYCC_601"); break;
   case BEGL_Colorimetry_BT_2020_NCL           : printf("BT_2020_NCL"); break;
   case BEGL_Colorimetry_BT_2020_CL            : printf("BT_2020_CL"); break;
   case BEGL_Colorimetry_Hdmi_Full_Range_YCbCr : printf("Hdmi_Full_Range_YCbCr"); break;
   }
   printf(" colorimetry\n");
}
#endif

// Obtain ownership of a new video frame surface, if one was available.
// Ownership must be released via ReleaseVideoFrame before the same buffer
// can be acquired again.
bool TextureData::AcquireVideoFrame()
{
#if SINGLE_PROCESS
   // Do we have a free buffer to destripe into?
   uint32_t freeBuffer = ~0U;
   uint32_t start = m_curIndex == -1 ? 0 : (uint32_t)m_curIndex;
   for (uint32_t b = start; b < start + m_data.size(); b++)
   {
      uint32_t idx = b % m_data.size();
      if (!m_data[idx].acquired)
      {
         freeBuffer = idx;
         break;
      }
   }
   if (freeBuffer == ~0U)
      return false;  // No buffer available

   NEXUS_Error                   rc;
   NEXUS_VideoDecoderFrameStatus frameStatus;
   uint32_t                      num;

   // Is there a video frame available?
   rc = NEXUS_VideoDecoder_GetDecodedFrames(m_decoder, &frameStatus, 1, &num);
   if (num != 1)
      return false;

#ifdef VC5
   DetermineColorimetry(frameStatus.surfaceCreateSettings.matrixCoefficients);
#endif

   // Ensure we get full frames
   frameStatus.surfaceCreateSettings.bufferType = NEXUS_VideoBufferType_eFrame;

   NEXUS_StripedSurfaceHandle striped = NEXUS_StripedSurface_Create(&frameStatus.surfaceCreateSettings);
   assert(striped != NULL);

   m_curIndex = freeBuffer;

   assert(!m_data[m_curIndex].acquired);
   m_data[m_curIndex].acquired = true;

   m_buffersAcquired++;

   NEXUS_Graphics2DDestripeBlitSettings settings;
   NEXUS_Graphics2D_GetDefaultDestripeBlitSettings(&settings);
   settings.source.stripedSurface   = striped;
   settings.output.surface          = m_data[m_curIndex].nativePixmap;
   settings.horizontalFilter        = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
   settings.verticalFilter          = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;

   rc = NEXUS_Graphics2D_DestripeBlit(m_gfx2d, &settings);
   assert(!rc);

   // Wait for the conversion to complete
   WaitForM2MCCompletion(1000);

   NEXUS_StripedSurface_Destroy(striped);
   rc = NEXUS_VideoDecoder_ReturnDecodedFrames(m_decoder, NULL, 1);

   if (m_oldestBuffer == -1)
      m_oldestBuffer = m_curIndex;

   return true;
#else
   NEXUS_SurfaceHandle                    captureSurface = NULL;
   NEXUS_SimpleVideoDecoderCaptureStatus  captureStatus;
   uint32_t                               numReturned;

   NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(m_decoder, &captureSurface, &captureStatus,
                                                1, &numReturned);
   assert(numReturned == 0 || numReturned == 1);

   if (numReturned == 1)
   {
      // Find captureSurface in our surface list and mark it as acquired
      m_curIndex = FindSurfaceIndex(captureSurface);
      assert(!m_data[m_curIndex].acquired);
      m_data[m_curIndex].acquired = true;

      m_buffersAcquired++;

      if (m_oldestBuffer == -1)
         m_oldestBuffer = m_curIndex;

      return true;
   }
   else
   {
      return false;
   }
#endif
}

void TextureData::ReleaseVideoFrame(uint32_t indx)
{
   assert(m_data[indx].acquired);
   assert(m_data[indx].nativePixmap != NULL);

#if !SINGLE_PROCESS
   NEXUS_SurfaceHandle surf = m_data[indx].nativePixmap;
   NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(m_decoder, &surf, 1);
#endif

   m_data[indx].acquired = false;
   m_buffersAcquired--;

   m_oldestBuffer = (indx + 1) % NumBuffers();

   if ((int32_t)indx == m_curIndex) // All buffers recycled?
   {
      m_curIndex = -1;
      m_oldestBuffer = -1;
   }
}

void TextureData::InsertFence()
{
   if (m_curIndex >= 0)
      m_data[m_curIndex].fence.Create();
}

bool TextureData::BindTexture(GLenum target)
{
   if (m_curIndex >= 0)
      glBindTexture(target, m_data[m_curIndex].textureID);

   return m_curIndex >= 0;
}

bool TextureData::WaitForOldestBufferFenceAndRecycle()
{
   // Never release if we only own one buffer.
   // That may get re-used during the next render if a new video frame isn't ready.
   if (m_oldestBuffer != -1 && m_buffersAcquired > 1)
   {
      assert(m_data.at(m_oldestBuffer).acquired);

      m_data.at(m_oldestBuffer).fence.Wait();

      m_data[m_oldestBuffer].fence.Destroy();
      ReleaseVideoFrame(m_oldestBuffer);

      return true;
   }
   return false;
}

void TextureData::InitGLExtensions()
{
   // Map the function pointers for the GL and EGL extensions we will be using (if they exist)
   m_glEGLImageTargetTexture2DOESFunc = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress(
                                                                "glEGLImageTargetTexture2DOES");
   m_eglCreateImageKHRFunc  = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
   m_eglDestroyImageKHRFunc = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

   if (!m_glEGLImageTargetTexture2DOESFunc || !m_eglCreateImageKHRFunc || !m_eglDestroyImageKHRFunc)
      throw "EGLImage texturing is not supported. Cannot continue";
}

void TextureData::WaitForM2MCCompletion(uint32_t timeoutMs)
{
   NEXUS_Error rc;
   do
   {
      rc = NEXUS_Graphics2D_Checkpoint(m_gfx2d, NULL);
      if (rc == NEXUS_GRAPHICS2D_QUEUED)
         rc = BKNI_WaitForEvent(m_m2mcDone, timeoutMs);
   } while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);
}


uint32_t TextureData::FindSurfaceIndex(NEXUS_SurfaceHandle surf) const
{
   for (size_t i = 0; i < m_data.size(); i++)
      if (m_data[i].nativePixmap == surf)
         return i;
   assert(0);
   return 0;
}

} // namespace
