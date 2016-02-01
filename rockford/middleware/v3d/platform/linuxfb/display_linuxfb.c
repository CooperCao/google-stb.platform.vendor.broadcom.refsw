/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Linux framebuffer platform for Nexus

FILE DESCRIPTION
DESC
=============================================================================*/

#include "display_linuxfb.h"

#include "nexus_platform.h"

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>
#include <semaphore.h>

#include "png.h"

#define MAX_SWAP_BUFFERS 2

#define DRIVER_NAME "LinuxFB"

/*
 * LinuxFB driver supports saving of a frame as PNG if
 *      1. PNG_OUT is defined
 *      2. save_frame_png is defined.
 * Define PNG_OUT in the LINUXFB_EXTRA_CFLAGS variable if it is needed
 */

/*
 * LinuxFB driver supports CRC calculation for every frame and comparing with a
 * reference. Two CRC calculation methods are supported:
 * crc32 - standard CRC32 calculation;
 * adler32 - fast CRC calculation (~2 times faster than crc32).
 * Define VERIFY_CRC with appropriate method (crc32 or adler32) in the
 * LINUXFB_EXTRA_CFLAGS variable outside of LinuxFB. LinuxFB's makefile will
 * pick it up and use.
 *
 * If VERIFY_CRC is defined, then
 *
 * export no_crc=y will disable CRC verification
 * export save_crc_table=y will force CRC reference table creation instead of
 * verification.
 *
 * Define VERIFY_CRC=calc_method (see above) in the
 * LINUXFB_EXTRA_CFLAGS variable if CRC calculation is needed.
 */

#include <errno.h>

#define UNUSED(X) (void)X

typedef void (*BufferGetRequirementsFunc)(BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);

typedef struct
{
   BufferGetRequirementsFunc  bufferGetRequirementsFunc;
   uint32_t                last_frame_no;
   bool                    stretch;
   BEGL_MemoryInterface    *memInterface;
   char                    *procname;
   LFPL_FB_Info            fb;
   uint32_t                flags;
   __pid_t                 pid;
#ifdef VERIFY_CRC
   FILE                       *crc_table_file;
   bool                       save_ref_crc_table;
#endif
} LFPL_DisplayData;

/* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
typedef struct
{
   sem_t                  lockSemaphore;
   NEXUS_SURFACEHANDLE    surface;
} LFPL_WindowState;

typedef struct
{
   uint32_t heapStartPhys;
   uint32_t heapSize;
   void     *heapStartCached;
} LFPL_HeapInfo;

/* These are needed to supplement the default conversion functions:
 * The framebuffer is not created in the Nexus arena, so passing the cachedAddr
 * to default conversion functions would return a bogus address and would
 * cause unpredictable memory access.
 */
typedef void *phys_to_cached(void *, uint32_t);
typedef uint32_t cached_to_phys(void *, void *);

static LFPL_HeapInfo fbHeapInfo;
static phys_to_cached *ptc_default;
static cached_to_phys *ctp_default;

/*****************************************************************************
 * Memory conversion functions
 *****************************************************************************/

static bool LFPL_IsValidFBCachedPtr(void *pCached)
{
   bool res = false;

   if ((pCached >= fbHeapInfo.heapStartCached) &&
      (pCached < (fbHeapInfo.heapStartCached + fbHeapInfo.heapSize)))
      res = true;

   return res;
}

static bool LFPL_IsValidFBPhysOffset(uint32_t offset)
{
   bool res = false;

   if ((offset >= fbHeapInfo.heapStartPhys) &&
      (offset < (fbHeapInfo.heapStartPhys + fbHeapInfo.heapSize)))
      res = true;

   return res;
}

/* These functions will supplement default memory conversion routines.
 * If the address requested for conversion belongs to the framebuffer range,
 * the proper address within the framebuffer will be returned.
 * Otherwise default ConvertPhysToCached/ConvertCachedToPhys function
 * will be used
 */

/* Return a cached memory pointer given a physical device memory offset. */
static void *LFPL_ConvertPhysicalToCached(void *context, uint32_t offset)
{
   if (LFPL_IsValidFBPhysOffset(offset))
      return (void*)((uintptr_t)fbHeapInfo.heapStartCached +
            (offset - fbHeapInfo.heapStartPhys));
   else
      /* Call default function for conversion */
      return ptc_default(context, offset);
}

/* Return a physical device memory offset given a cached pointer. */
static uint32_t LFPL_ConvertCachedToPhysical(void *context, void *pCached)
{
#ifndef NDEBUG
   if (pCached == 0)
      printf("LFPL : Trying to convert NULL pointer\n");
#endif
   if (LFPL_IsValidFBCachedPtr(pCached))
      return fbHeapInfo.heapStartPhys +
            ((uintptr_t)pCached - (uintptr_t)fbHeapInfo.heapStartCached);
   else
      /* Call default function for conversion */
      return ctp_default(context, pCached);
}

/*****************************************************************************
 * Frame buffer info and mmap
 *****************************************************************************/

static bool linuxfb_init(LFPL_FB_Info *fb)
{
   fb->fbfd = open("/dev/fb0", O_RDWR);
   if (fb->fbfd != -1)
   {
      fb->cur_page = 0;
      /* Get variable screen information*/
      int err = ioctl(fb->fbfd, FBIOGET_VSCREENINFO, &fb->vinfo);
      if (err == -1)
      {
         perror("Error reading variable information");
         goto close_fbfd;
      }
      memcpy(&fb->initial_vinfo, &fb->vinfo, sizeof(struct fb_var_screeninfo));
      /* Change virtual resolution info and save it*/
      fb->vinfo.yres_virtual = fb->vinfo.yres * 2;
      if (ioctl(fb->fbfd, FBIOPUT_VSCREENINFO, &fb->vinfo))
      {
         perror("Error setting variable information");
         goto close_fbfd;
      }
      /* Get fixed screen information*/
      err = ioctl(fb->fbfd, FBIOGET_FSCREENINFO, &fb->finfo);
      if (err == -1)
      {
         perror("Error reading fixed information");
         goto close_fbfd;
      }
      /* Figure out the size of the screen in bytes*/
      fb->fbmapsize = fb->finfo.smem_len;
      fb->page_size = fb->finfo.line_length * fb->vinfo.yres;
      /* Map the device to memory*/
      fb->fbp = mmap(0, fb->fbmapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fbfd, 0);
      if ((int)fb->fbp == -1)
      {
         perror("Error: failed to map framebuffer device to memory");
         goto close_fbfd;
      }
/*
 * If mapped framebuffer address info is needed,
 * add -DLINUXFB_DISP_FB_MAP_INFO in the LINUXFB_EXTRA_CFLAGS variable
 */
#ifdef LINUXFB_DISP_FB_MAP_INFO
      printf("The framebuffer device was mapped to memory successfully (0x%x -> 0x%x, %d bytes).\n",
             fb->finfo.smem_start, fb->fbp, fb->fbmapsize);
      printf("  %dx%d, %dbpp\n", fb->vinfo.xres, fb->vinfo.yres, fb->vinfo.bits_per_pixel);
#endif
      if (getenv("no_vsync_wait"))
         fb->flags |= FB_NO_VSYNC_WAIT;
      if (getenv("no_double_buf"))
         fb->flags |= FB_NO_DBL_BUF;
      if (getenv("convert_gl_to_fb"))
         fb->flags |= FB_CONVERT_GL_FB;
   }
   else
      fb->fbfd = -errno;
   return fb->fbfd >= 0;
close_fbfd:
   close(fb->fbfd);
   fb->fbfd = -1;
   return false;
}

/* Calculates offset to the start of the curent display page */
static unsigned int linuxfb_calc_offset(LFPL_FB_Info *fb)
{
   /* fb->cur_page has already been initialized to 0.
    * So change it only if no_double_buf is not set
    */
   if (!(fb->flags & FB_NO_DBL_BUF))
      fb->cur_page = (fb->cur_page + 1) % 2;
   return (fb->page_size * fb->cur_page);
}

/* Actions to be done after rendering. Invoked by DispBufferDisplay */
static void linuxfb_after_render(LFPL_FB_Info *fb)
{
   if (!(fb->flags & FB_NO_VSYNC_WAIT))
   {
      /* Wait for VSYNC */
      ioctl(fb->fbfd, FBIO_WAITFORVSYNC, 0);
   }
   if ((!(fb->flags & FB_NO_DBL_BUF))
       /* This is needed if we break an app when second screen was displayed,
        * and then set no_double_buf, and then started another V3D app.
        */
       || (fb->vinfo.yoffset > 0))
   {
      /* switch page smoothly */
      fb->vinfo.yoffset = fb->cur_page * fb->vinfo.yres;
      fb->vinfo.activate = FB_ACTIVATE_VBL;
      if (ioctl(fb->fbfd, FBIOPAN_DISPLAY, &fb->vinfo))
      {
         perror("Error panning display");
      }
   }
}

static int get_current_process_name(char **proc)
{
   FILE *fp = fopen("/proc/self/cmdline", "r");
   char *p = NULL;
   char *p1 = NULL;
   int len = 0;
   int result = 0;
   *proc = (char *)malloc(128);
   if (!fp)
      return -1;
   fgets(*proc, 127, fp);
   fclose(fp);
   p = strtok(*proc, "/");
   while (p != NULL)
   {
      p1 = p;
      len = strlen(p1);
      p = strtok(NULL, "/");
   }
   if (p1)
   {
      strncpy(*proc, p1, len);
      *(*proc + len) = '\0';
   }
   return len;
}

/*****************************************************************************
 * Different formats of saving of an image
 *****************************************************************************/

static void do_save_raw(char *prefix, LFPL_DisplayData *data, void *buf)
{
   /* create file */
   char fname[256];
   int fpd;
   sprintf(fname, "./raw_dumps/%s_%s[%d]_dump%06d.raw", prefix, data->procname, data->pid, data->last_frame_no);
   fpd = creat(fname, 0666);
   if (fpd >= 0)
   {
      if (data->flags & DISPLAY_FLAG_PRINT_FILE_NAME)
         printf("Dumping %s as %s, %dx%d, %d bytes\n", prefix, fname, data->fb.vinfo.xres, data->fb.vinfo.yres, data->fb.page_size);
      write(fpd, buf, data->fb.page_size);
      fsync(fpd);
      close(fpd);
   }
}

#ifdef PNG_OUT
static void do_save_png(char *prefix, void *buf, void *b_data, LFPL_DisplayData *data)
{
   FILE *fp;
   char fname[256];
   png_structp png_ptr;
   png_infop info_ptr = NULL;
   bool hasAlpha = true;
   png_bytepp row_pointers;
   int y, res = 0;
   uint32_t width, height, maxwidth, maxheight;
   char *buffer;
   BEGL_BufferDisplayState *state = (BEGL_BufferDisplayState *)buf;
   LFPL_BufferData *_buffer = state->buffer;

   width = _buffer->settings.width;
   height = _buffer->settings.height;
   maxwidth = _buffer->settings.pitchBytes;
   maxheight = _buffer->settings.height;
   buffer = _buffer->settings.cachedAddr;

   sprintf(fname, "./png_dumps/%s_%s[%d]_%06d.png", prefix, data->procname, data->pid, data->last_frame_no);
   if (data->flags & DISPLAY_FLAG_PRINT_FILE_NAME)
      printf("Outputting frame as %s, %dx%d maxwidth %d maxheight %d\n", fname, width, height, maxwidth, maxheight);
   mkdir("png_dumps", 0777);

   /* create file */
   fp = fopen(fname, "wb");
   if (!fp)
   {
      printf("ERROR (save_png): Cannot open file %s\n", fname);
      return;
   }

   /* initialize stuff */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      goto error0;

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
      goto error0;

   if (setjmp(png_jmpbuf(png_ptr)))
      goto error0;

   png_init_io(png_ptr, fp);

   /* write header */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error0;

   png_set_IHDR(png_ptr, info_ptr, width, height,
                8, hasAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   png_write_info(png_ptr, info_ptr);

   row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
   if (!row_pointers)
      goto error0;

   /* initialize the rowpointers for the image */
   if (!b_data)
   {
      row_pointers[maxheight - 1] = (png_bytep)buffer;
      for (y = maxheight - 1; y > 0; y--)
         row_pointers[y - 1] = (png_bytep)(row_pointers[y] + maxwidth);
   }
   else
   {
      row_pointers[0] = (png_bytep)buffer;
      for (y = 0; y < maxheight; y++)
         row_pointers[y + 1] = (png_bytep)(row_pointers[y] + maxwidth);
   }

   /* write bytes */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error1;

   png_write_image(png_ptr, row_pointers);

   /* end write */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error1;

   png_write_end(png_ptr, NULL);

   /* success */
   res = 1;

error1:
   /* cleanup heap allocation */
   free(row_pointers);

error0:
   png_destroy_write_struct(&png_ptr, &info_ptr);

   fclose(fp);
}

#endif

/* Show the current frame-rate */
static void DispShowFPS(LFPL_DisplayData *data)
{
   struct timeval cur_time;
   int            now_ms;
   static int     last_print_time  = 0;
   static int     last_print_frame = 0;

   gettimeofday(&cur_time, NULL);
   now_ms = cur_time.tv_usec / 1000;
   now_ms += cur_time.tv_sec * 1000;

   if (now_ms - last_print_time > 1000 || last_print_frame == 0)
   {
      if (now_ms - last_print_time != 0 && last_print_time != 0)
      {
         float fps = (float)(data->last_frame_no - last_print_frame) / ((float)(now_ms - last_print_time) / 1000.0f);
         fprintf(stdout,
               "\rFrame(s) count [%s pid %d]: %d (%.2f fps)                \r",
               data->procname, data->pid,
               data->last_frame_no, fps);
      }

      last_print_frame = data->last_frame_no;
      last_print_time  = now_ms;
   }
}

/* Auxiliary functions end */

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/
static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   LFPL_DisplayData *data = (LFPL_DisplayData *)context;
   LFPL_WindowState *windowState = (LFPL_WindowState *)state->windowState.platformState;
   LFPL_BufferData *buffer = (LFPL_BufferData *)state->buffer;

   data->last_frame_no++;
#ifdef VERIFY_CRC
   if (data->crc_table_file)
   {
      /* Because we rendered directly to the fb,
       * there is no separate GL buffer.
       * DO NOT forget to switch OFF the Linux cursor via
       * echo 0 > /sys/class/graphics/fbcon/cursor_blink before either saving
       * a CRC table or verifying against existing one!!!
       */
      uint32_t current_frame_crc = VERIFY_CRC(0, buffer->settings.cachedAddr, data->fb.page_size);
      if (!data->save_ref_crc_table)
      {
         /* Read CRC from the table file and verify our current frame against it */
         uint32_t read_crc;
         if (fread(&read_crc, sizeof(uint32_t), 1, data->crc_table_file) == 1)
         {
            if (read_crc != current_frame_crc)
            {
               printf("[%s pid %d] CRC mismatch, frame %d. Expected: 0x%x Got: 0x%x\n",
                     data->procname,
                     data->pid,
                     data->last_frame_no,
                     read_crc,
                     current_frame_crc);
#ifdef PNG_OUT
               do_save_png("gl-crc", state, NULL, data);
#endif
            }
         }
         /* End of CRC table, continue normal execution */
         else
         {
            fclose(data->crc_table_file);
            data->crc_table_file = NULL;
            printf("[%s pid %d] Reached the end of the CRC table file (last frame processed %d).\n"
                  "Resume normal execution (no CRC calculation anymore). "
                  "The framerate will increase\n", data->procname, data->pid, data->last_frame_no);
         }
      }
      else
      {
         /* File opened for writing. Write the reference CRC */
         fwrite(&current_frame_crc, sizeof(uint32_t), 1, data->crc_table_file);
         fflush(data->crc_table_file);
#ifdef PNG_OUT
         /* Save reference PNG to compare later */
         if (data->flags & DISPLAY_FLAG_SAVE_CRC_PNG)
            do_save_png("gl-ref", state, NULL, data);
#endif

      }
   } /* data->crc_table_file */
#endif
   if (data->flags & DISPLAY_FLAG_PRINT_FRAME_NO)
   {
      DispShowFPS(data);
      fflush(stdout);
   }
   if (data->flags & DISPLAY_FLAG_PRINT_BUF_ADDR)
      printf("DispBufferDisplay(Context is at 0x%x)\n", context);

#ifdef PNG_OUT
   if (data->flags & DISPLAY_FLAG_SAVE_FRAME_PNG)
      do_save_png("gl", state, NULL, data);
#endif

   if (data->fb.fbfd >= 0)
   {
      /* Rendering to the (invisible) buffer of the fb has been completed.
       * Now wait for vsync if required and pan the screen if needed
       */
      linuxfb_after_render(&data->fb);
      if (data->flags & DISPLAY_FLAG_SAVE_FRAME_RAW)
         /* Save raw FrameBuffer */
         do_save_raw("FBRaw", data, buffer->settings.cachedAddr);
#ifdef PNG_OUT
      if (data->flags & DISPLAY_FLAG_SAVE_FRAME_PNG_FB)
         do_save_png("fb", state, &data->fb, data);
#endif
      if (data->flags & DISPLAY_FLAG_PRINT_BUF_ADDR)
         printf("Frame buffer virtual = 0x%x physical = 0x%x; size: %dx%d\n",
               buffer->settings.cachedAddr, buffer->settings.physOffset,
                buffer->settings.width, buffer->settings.height);
   } /* fbfd >= 0 */
   sem_post(&windowState->lockSemaphore);
   return BEGL_Success;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   return BEGL_Success;
}

/* Request creation of an appropriate display buffer. Only the 3D driver knows the size and alignment constraints, so the
 * buffer create request must come from the driver. settings->totalByteSize is the size of the memory that the driver needs.
 * We could have just requested a block of memory using the memory interface, but by having the platform layer create a 'buffer'
 * it can actually create whatever type it desires directly, and then only have to deal with that type. For example, in a Nexus
 * platform layer, this function might be implemented to create a NEXUS_Surface (with the correct memory constraints of course).
 * When the buffer handle is passed out during BufferDisplay, the platform layer can simply use it as a NEXUS_Surface. It
 * doesn't have to wrap the memory each time, or perform any lookups. Since the buffer handle is opaque to the 3d driver, the
 * platform layer has complete freedom.
 */
static BEGL_BufferHandle DispBufferCreate(void *context, BEGL_BufferSettings *settings)
{
   LFPL_DisplayData              *data = (LFPL_DisplayData *)context;
   LFPL_BufferData               *buffer = NULL;
   NEXUS_SurfaceCreateSettings    surfSettings;
   NEXUS_MemoryAllocationSettings memSettings;
   NEXUS_MemoryStatus             memStatus;
   uint32_t                       bpp;

   buffer = (LFPL_BufferData *)malloc(sizeof(LFPL_BufferData));
   if (buffer != NULL)
   {
      memset(buffer, 0, sizeof(LFPL_BufferData));
      NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);
      switch (settings->format)
      {
         case BEGL_BufferFormat_eA8B8G8R8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
            break;
         case BEGL_BufferFormat_eR8G8B8A8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
            break;
         case BEGL_BufferFormat_eX8B8G8R8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eX8_B8_G8_R8;
            break;
         case BEGL_BufferFormat_eR8G8B8X8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_X8;
            break;
         case BEGL_BufferFormat_eR5G6B5:
         case BEGL_BufferFormat_eR5G6B5_Texture:
            bpp = 16;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR5_G6_B5;
            break;
         case BEGL_BufferFormat_eYUV422_Texture:
            bpp = 16;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
            break;
         case BEGL_BufferFormat_eVUY224_Texture:
            bpp = 16;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
            break;
         case BEGL_BufferFormat_eA8B8G8R8_Texture:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
            break;
         case BEGL_BufferFormat_eR8G8B8A8_Texture:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
            break;
         default:
            break;
      }

      surfSettings.width = settings->width;
      surfSettings.height = settings->height;
      surfSettings.alignment = settings->alignment;
      surfSettings.pitch = settings->pitchBytes;
      surfSettings.heap = NXPL_MemHeap(data->memInterface);
      NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
      memSettings.alignment = settings->alignment;
      memSettings.heap = surfSettings.heap;

      /* Surface must be created in our heap, and always with a cached address */
      NEXUS_Memory_Allocate(settings->totalByteSize, &memSettings, &surfSettings.pMemory);
      if (surfSettings.pMemory != NULL)
         buffer->surface = NEXUS_Surface_Create(&surfSettings);

      NEXUS_Heap_GetStatus(memSettings.heap, &memStatus);
            settings->physOffset = memStatus.offset + ((uintptr_t)surfSettings.pMemory - (uintptr_t)memStatus.addr);
            settings->cachedAddr = surfSettings.pMemory;

      buffer->settings = *settings;
   }

   return (BEGL_BufferHandle)buffer;
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   LFPL_DisplayData *data = (LFPL_DisplayData *)context;
   LFPL_BufferData *buffer = (LFPL_BufferData *)bufferState->buffer;

   if (buffer != NULL)
   {
      if (buffer->surface)
      {
         NEXUS_SurfaceCreateSettings createSettings;
         void *surfacePtr;

         NEXUS_Surface_GetCreateSettings(buffer->surface, &createSettings);
         NEXUS_Surface_Destroy(buffer->surface);
         surfacePtr = createSettings.pMemory;
         if (surfacePtr != NULL)
            data->memInterface->Free(data->memInterface->context, surfacePtr);
      }

      memset(buffer, 0, sizeof(LFPL_BufferData));
      free(buffer);
   }

   return BEGL_Success;
}

/* Get information about a created window buffer */
static BEGL_Error DispBufferGetCreateSettings(void *context, BEGL_BufferHandle bufHandle, BEGL_BufferSettings *settings)
{
   LFPL_DisplayData *data = (LFPL_DisplayData *)context;
   LFPL_BufferData *buffer = (LFPL_BufferData *)bufHandle;

   if (buffer != NULL)
   {
      *settings = buffer->settings;
      return BEGL_Success;
   }

   return BEGL_Fail;
}

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'.
 */
static BEGL_Error DispWindowGetInfo(void *context,
                                    BEGL_WindowHandle window,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   LFPL_DisplayData *data = (LFPL_DisplayData *)context;
   if (data != NULL)
   {
      if (flags & BEGL_WindowInfoWidth)
         info->width = data->fb.vinfo.xres;
      if (flags & BEGL_WindowInfoHeight)
         info->height = data->fb.vinfo.yres;

      if (flags & BEGL_WindowInfoSwapChainCount)
         info->swapchain_count = MAX_SWAP_BUFFERS;

      return BEGL_Success;
   }
   return BEGL_Fail;
}

static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   LFPL_DisplayData     *data = (LFPL_DisplayData *)context;
   LFPL_BufferData      *buffer = NULL;

   buffer = (LFPL_BufferData *)DispBufferCreate(context, settings);

   return (BEGL_BufferHandle)buffer;
}

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   LFPL_DisplayData *data = (LFPL_DisplayData *)context;

   if (data)
   {
      uint32_t offset;
      LFPL_WindowState *windowState = (LFPL_WindowState *)bufferAccess->windowState.platformState;
      LFPL_BufferData  *buffer = bufferAccess->buffer;
      sem_wait(&windowState->lockSemaphore);
      /* update BEGL_BufferSettings with new information
       * take double buffering into consideration
       * (switch page and get offset from the framebuffer start addr)
       */
      offset = linuxfb_calc_offset(&data->fb);
      buffer->settings.physOffset = (uint32_t)data->fb.finfo.smem_start + offset;
      buffer->settings.pitchBytes = data->fb.finfo.line_length;

      /* provide virtual address for the current framebuffer's page (swapchain) */
      buffer->settings.cachedAddr = data->fb.fbp + offset;

      buffer->settings.width = data->fb.vinfo.xres;
      buffer->settings.height = data->fb.vinfo.yres;

#ifdef LINUXFB_RGBA8888_ONLY
      buffer->settings.format = BEGL_BufferFormat_eA8B8G8R8;
#endif
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window)
{
   LFPL_WindowState *windowState = (LFPL_WindowState *)malloc(sizeof(LFPL_WindowState));
   LFPL_DisplayData *data = (LFPL_DisplayData *)context;
   if ((windowState != NULL) && (data))
   {
      memset(windowState, 0, sizeof(LFPL_WindowState));
      sem_init(&windowState->lockSemaphore, 0, MAX_SWAP_BUFFERS);
   }

   return (void *)windowState;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   LFPL_WindowState *windowState = (LFPL_WindowState *)swapChainCtx;

   if (windowState)
   {
      sem_destroy(&windowState->lockSemaphore);

      memset(windowState, 0, sizeof(LFPL_WindowState));
      free(windowState);
      return BEGL_Success;
   }

   return BEGL_Fail;
}


BEGL_DisplayInterface *LFPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   BEGL_HWInterface     *hwIface,
                                                   BEGL_DisplayCallbacks *displayCallbacks)
{
   LFPL_DisplayData *data;
   BEGL_DisplayInterface *disp = (BEGL_DisplayInterface *)malloc(sizeof(BEGL_DisplayInterface));

   /* Save pointers to original conversion functions.
    * They will be called if an address passed for conversion is not
    * in the framebuffer address range
    */
   ptc_default = memIface->ConvertPhysicalToCached;
   ctp_default = memIface->ConvertCachedToPhysical;

   if (disp != NULL)
   {
      data = (LFPL_DisplayData *)malloc(sizeof(LFPL_DisplayData));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (data != NULL)
      {
         NEXUS_MemoryStatus            memStatus;

         memset(data, 0, sizeof(LFPL_DisplayData));

         disp->context = (void *)data;
         disp->BufferDisplay = DispBufferDisplay;
         disp->WindowUndisplay = DispWindowUndisplay;
         disp->BufferCreate = DispBufferCreate;
         disp->BufferGet = DispBufferGet;
         disp->BufferDestroy = DispBufferDestroy;
         disp->BufferGetCreateSettings = DispBufferGetCreateSettings;
         disp->BufferAccess = DispBufferAccess;
         disp->WindowGetInfo = DispWindowGetInfo;
         disp->WindowPlatformStateCreate = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;

         data->memInterface = memIface;
         data->bufferGetRequirementsFunc = displayCallbacks->BufferGetRequirements;

         data->last_frame_no = 0;
         data->pid = getpid();

         /* Obtain flags from environment variables */
         data->flags = 0;
         if (getenv("print_output_filename"))
            data->flags |= DISPLAY_FLAG_PRINT_FILE_NAME;
         if (getenv("print_buf_addr"))
            data->flags |= DISPLAY_FLAG_PRINT_BUF_ADDR;
         if (getenv("print_frame_no"))
            data->flags |= DISPLAY_FLAG_PRINT_FRAME_NO;
         if (getenv("save_frame_png"))
            data->flags |= DISPLAY_FLAG_SAVE_FRAME_PNG;
         if (getenv("save_frame_png_fb"))
            data->flags |= DISPLAY_FLAG_SAVE_FRAME_PNG_FB;
         if (getenv("save_frame_raw"))
            data->flags |= DISPLAY_FLAG_SAVE_FRAME_RAW;
         if (getenv("save_crc_png"))
            data->flags |= DISPLAY_FLAG_SAVE_CRC_PNG;

         get_current_process_name(&data->procname);

#ifdef VERIFY_CRC
         if (!getenv("no_crc"))
         {
            char *fname = NULL;
            int len = strlen(data->procname);
            data->crc_table_file = NULL;
            if (len > 0)
            {
               fname = (char *)malloc(len + 5);
               snprintf(fname, len + 5, "%s.crc", data->procname);
               data->save_ref_crc_table = getenv("save_crc_table") ? true : false;
               if (data->save_ref_crc_table)
               {
                  data->crc_table_file = fopen(fname, "wb");
                  if (data->crc_table_file)
                     printf("----- Create reference CRC table %s for \"%s\" -----\n", fname, data->procname);
               }
               else
               {
                  data->crc_table_file = fopen(fname, "r");
                  if (data->crc_table_file)
                     printf("----- CRC Verification is ON for \"%s\" against %s -----\n", data->procname, fname);
                  else
                     printf("**** CRC verification is OFF for \"%s\" against %s: OS returned code %d opening the %s file\n", data->procname, fname, errno, fname);
               }
               free(fname);
            }
            else
            {
               printf("##### CRC Verification is OFF: cannot get process name #####\n");
            }
            if (data->crc_table_file)
            {
               printf("\nDOUBLE CHECK THAT LINUX CURSOR IS OFF (otherwise CRC data will be invalid)!\n");
               printf("Note: application will be running slowly\n\n");
            }

         }
         else
            printf("----- CRC verification is off due to no_crc variable is set -----\n");
#endif

#ifdef PNG_OUT
         printf("%s INFO: PNG output support enabled\n", DRIVER_NAME);
#endif

         memset(&data->fb, 0, sizeof(LFPL_FB_Info));
         if (!linuxfb_init(&data->fb))
         {
            printf("\n ### FATAL: Linux FB device is unavailable. ###\n");
            goto error1;
         }
         /* Fill in the FB heap info with
          * obtained Linux FB data
          */
         fbHeapInfo.heapSize = data->fb.fbmapsize;
         fbHeapInfo.heapStartCached = data->fb.fbp;
         fbHeapInfo.heapStartPhys = data->fb.finfo.smem_start;
         /* Patch conversion routines */
         memIface->ConvertCachedToPhysical = LFPL_ConvertCachedToPhysical;
         memIface->ConvertPhysicalToCached = LFPL_ConvertPhysicalToCached;
      }
      else
      {
         goto error0;
      }
   }
   return disp;

error1:
   free(data);

error0:
   free(disp);
   return NULL;
}

void LFPL_DestroyDisplayInterface(BEGL_DisplayInterface *mem)
{
   if (mem != NULL)
   {
      if (mem->context != NULL)
      {

         LFPL_DisplayData *data = (LFPL_DisplayData *)mem->context;
         if (data)
         {
            if (data->flags & DISPLAY_FLAG_PRINT_FRAME_NO)
               printf("Last frame displayed/rendered: %d                          \n", data->last_frame_no);

            if (data->fb.fbfd >= 0)
            {
               munmap(data->fb.fbp, data->fb.fbmapsize);
               if (!ioctl(data->fb.fbfd, FBIOPUT_VSCREENINFO, &data->fb.initial_vinfo))
                  printf("Restored initial screen parameters.\n");
               close(data->fb.fbfd);
            }
            if (data->procname)
            {
               free(data->procname);
               data->procname = NULL;
            }
#ifdef VERIFY_CRC
            if (data->crc_table_file)
               fclose(data->crc_table_file);
#endif
         }
         free(mem->context);
      }
      memset(mem, 0, sizeof(BEGL_DisplayInterface));
      free(mem);
   }
}

bool LFPL_CreateCompatiblePixmap(LFPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfo *info)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayCallbacks.PixmapCreateCompatiblePixmap != NULL)
   {
      BEGL_BufferHandle buffer = data->displayCallbacks.PixmapCreateCompatiblePixmap(info);
      if (buffer != NULL)
      {
         *pixmapHandle = (void*)buffer;
         *surface = ((LFPL_BufferData*)buffer)->surface;
         return true;
      }
   }

   return false;
}

void LFPL_DestroyCompatiblePixmap(LFPL_PlatformHandle handle, void *pixmapHandle)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL &&
       data->displayInterface != NULL &&
       data->displayInterface->BufferDestroy)
   {
      BEGL_BufferDisplayState bufferState;
      memset(&bufferState, 0, sizeof(BEGL_BufferDisplayState));
      bufferState.buffer = (BEGL_BufferHandle)pixmapHandle;

      data->displayInterface->BufferDestroy(data->displayInterface->context, &bufferState);
   }
}

void *LFPL_CreateNativeWindow(const LFPL_NativeWindowInfo *info)
{
   /* For linuxfb this function is a stub,
    * It should return a non-NULL pointer, which
    * is supposed to only be used as a parameter for this
    * module, and this module will simply ignore it.
    * This is done only for the sake of being compatible with
    * applications/khronos/v3d/nexus/ apps, so we don't
    * have to modify all of them to remove calls to NativeWindow
    * functions
    */
   return (void *)info;
}

void LFPL_UpdateNativeWindow(void *native, const LFPL_NativeWindowInfo *info)
{
   UNUSED(native);
   UNUSED(info);
}

void LFPL_DestroyNativeWindow(void *nativeWin)
{
   UNUSED(nativeWin);
   return;
}
