/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include "Common.h"
#include "V3DPlatformBase.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "Fence.h"
#include "Semaphore.h"
#include "SwapchainKHR.h"
#include "SchedDependencies.h"
#include "Options.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/util/profile/profile.h"
#include "libs/platform/v3d_imgconv.h"

namespace bvk
{

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)

// Used to capture the hInstance of this DLL
static HINSTANCE s_instance{};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   if (fdwReason == DLL_PROCESS_ATTACH)
      s_instance = hinstDLL;
   return TRUE;
}

// MS-Windows event handling function:
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_CLOSE:
      PostQuitMessage(0);
      break;
   default:
      break;
   }

   return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

SurfaceWin32::SurfaceWin32(const VkAllocationCallbacks         *pAllocator,
                           const VkDisplaySurfaceCreateInfoKHR *pCreateInfo) :
   Allocating(pAllocator)
{
   directDisplay = true;

   const char *title = "Virtual Direct Display Mode - closing the window may cause a crash";

   // Make a virtual direct-display using a Win32 window
   WNDCLASSEX wndClass;

   wndClass.cbSize        = sizeof(WNDCLASSEX);
   wndClass.style         = CS_HREDRAW | CS_VREDRAW;
   wndClass.lpfnWndProc   = WndProc;
   wndClass.cbClsExtra    = 0;
   wndClass.cbWndExtra    = 0;
   wndClass.hInstance     = s_instance;
   wndClass.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
   wndClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
   wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
   wndClass.lpszMenuName  = nullptr;
   wndClass.lpszClassName = title;
   wndClass.hIconSm       = LoadIcon(nullptr, IDI_WINLOGO);

   if (!RegisterClassEx(&wndClass))
   {
      printf("Failed to register WndClass\n");
      throw(std::bad_alloc());
   }

   // Create window with the registered class:
   RECT wr = { 0, 0, static_cast<LONG>(pCreateInfo->imageExtent.width),
                     static_cast<LONG>(pCreateInfo->imageExtent.height) };
   AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

   hWnd = CreateWindowEx(0, title, title, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
                         200, 200,  wr.right - wr.left, wr.bottom - wr.top,
                         nullptr, nullptr, s_instance, nullptr);
   if (!hWnd)
   {
      printf("Failed to create window\n");
      throw(std::bad_alloc());
   }
}

#endif // VK_USE_PLATFORM_DISPLAY_KHR

V3DPlatformWin32::V3DPlatformWin32()
{
   CommonInitialize();
}

V3DPlatformWin32::~V3DPlatformWin32()
{
   delete[] m_rgba8888;
   CommonTerminate();
}

void V3DPlatformWin32::AttachSwapchain(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   SwapchainKHR                  *chain)
{
   PhysicalDevice *physDev = pDevice->GetPhysicalDevice();

   m_numImages = chain->Images().size();
   m_curImage  = 0;

   // Calculate the total size of all the images
   VkDeviceSize   totalSize = 0;
   for (uint32_t i = 0; i < chain->Images().size(); i++)
   {
      VkMemoryRequirements memReq;
      chain->Images()[i]->GetImageMemoryRequirements(pDevice, &memReq);

      memReq.size = gfx_u64round_up(memReq.size, memReq.alignment);
      totalSize += memReq.size;
   }

   // Find a suitable memory type index
   uint32_t memTypeIndex = physDev->FindSuitableMemoryTypeIndex(
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

   VkMemoryAllocateInfo memCI;
   memCI.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   memCI.pNext           = nullptr;
   memCI.allocationSize  = totalSize;
   memCI.memoryTypeIndex = memTypeIndex;

   // Make a single device memory block to include all images
   //   No need to try/catch, we have nothing to clean up at this point
   DeviceMemory *devMem = createObject<DeviceMemory, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
                                  pCallbacks, &g_defaultAllocCallbacks, pDevice, &memCI);

   VkDeviceSize offset = 0;
   uint32_t     width  = chain->CreateInfo().imageExtent.width;
   uint32_t     height = chain->CreateInfo().imageExtent.height;

   for (uint32_t i = 0; i < chain->Images().size(); i++)
   {
      VkMemoryRequirements memReq;
      chain->Images()[i]->GetImageMemoryRequirements(pDevice, &memReq);
      memReq.size = gfx_u64round_up(memReq.size, memReq.alignment);

      // Check all images are the same size
      assert(chain->Images()[i]->Extent().width  == width);
      assert(chain->Images()[i]->Extent().height == height);

      // Bind the memory to the image
      chain->Images()[i]->BindImageMemory(pDevice, devMem, offset);

      // Make windows BMI data for later use in StretchDIBits
      ImageData data;
      data.m_bmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFO) + sizeof(DWORD) * 3);
      data.m_bmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
      data.m_bmi->bmiHeader.biWidth         = width;
      // A negative height in the bmiHeader indicates a top-down bitmap (which matches what
      // our output frames are).
      data.m_bmi->bmiHeader.biHeight        = -(LONG)height;
      data.m_bmi->bmiHeader.biPlanes        = 1;
      data.m_bmi->bmiHeader.biBitCount      = 32;
      data.m_bmi->bmiHeader.biCompression   = BI_BITFIELDS;
      data.m_bmi->bmiHeader.biSizeImage     = static_cast<DWORD>(memReq.size);
      data.m_bmi->bmiHeader.biXPelsPerMeter = 2048;
      data.m_bmi->bmiHeader.biYPelsPerMeter = 2048;
      data.m_bmi->bmiHeader.biClrUsed       = 0;
      data.m_bmi->bmiHeader.biClrImportant  = 0;

      DWORD *dwMasks = (DWORD*)&data.m_bmi->bmiColors;
      dwMasks[0] = 0x000000FF; //red
      dwMasks[1] = 0x0000FF00; //green
      dwMasks[2] = 0x00FF0000; //blue

      // Set the initial state of the buffers?
      bool clearInitialBuffers = true;
      if (clearInitialBuffers)
      {
         void *memPtr = nullptr;
         devMem->MapMemory(pDevice, offset, memReq.size, 0, &memPtr);

         memset(memPtr, (i + 1) * (255 / m_numImages), static_cast<size_t>(memReq.size));

         VkMappedMemoryRange range{};
         range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
         range.size = memReq.size;
         devMem->FlushMappedRange(range);

         devMem->UnmapMemory(pDevice);
      }

      chain->PlatformImageData()[i] = data;

      offset += memReq.size;
   }

   // Record the device memory in the swapChain
   assert(chain->DevMemBacking().size() == 0);
   chain->DevMemBacking().push_back(devMem);
}

void V3DPlatformWin32::DetachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   for (uint32_t i = 0; i < chain->PlatformImageData().size(); i++)
   {
      free(chain->PlatformImageData()[i].m_bmi);
      chain->PlatformImageData()[i].m_bmi = nullptr;
   }
   chain->PlatformImageData().clear();

   for (uint32_t i = 0; i < chain->DevMemBacking().size(); i++)
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(chain->DevMemBacking()[i], pCallbacks);

   chain->DevMemBacking().clear();
}

void V3DPlatformWin32::ConvertTo8888(
   const Image &srcImage,
   void        *dstPtr,
   size_t       dstSize)
{
   GFX_BUFFER_DESC_T desc = {0};
   desc.width                 = srcImage.Extent().width;
   desc.height                = srcImage.Extent().height;
   desc.depth                 = 1;
   desc.num_planes            = 1;
   desc.planes[0].lfmt        = GFX_LFMT_R8G8B8X8_UNORM_2D_RSO;
   desc.planes[0].pitch       = desc.width * 4;

   v3d_imgconv_ptr_tgt dst;
   v3d_imgconv_init_ptr_tgt(&dst, dstPtr, &desc, 0, 0, 0, 0, 0);

   v3d_imgconv_gmem_tgt src;
   srcImage.InitGMemTarget(&src, SchedDependencies(), 0, 0);

   v3d_imgconv_convert_to_ptr(&dst, &src, desc.width, desc.height, 1,
                           1, /*secure_context=*/false);
}

void V3DPlatformWin32::ResizeDisplayBuffer(uint32_t w, uint32_t h)
{
   uint32_t size = w * h * 4;

   if (size != m_rgba8888Size || m_rgba8888 == nullptr)
   {
      if (m_rgba8888 != nullptr)
         delete[] m_rgba8888;

      m_rgba8888Size = size;
      m_rgba8888     = new uint8_t[size];
   }
}

void V3DPlatformWin32::DisplayFrame(
   Device            *device,
   SwapchainKHR      *chain,
   const Image       &image,
   const ImageData   &data)
{
   SurfaceWin32 *surf = FromSurfaceKHR<SurfaceWin32>(chain->CreateInfo().surface);

   MaybeDumpPresentedImage(image);

   // We convert the src image into RGBA8888 for display
   ResizeDisplayBuffer(image.Extent().width, image.Extent().height);
   ConvertTo8888(image, m_rgba8888, m_rgba8888Size);

   // The bmiHeader height may be negative (indicating a top-down image).
   int height = abs(data.m_bmi->bmiHeader.biHeight);

   // Copy the image into the window now
   HDC hdc = GetDC(surf->hWnd);
   StretchDIBits(hdc, 0, 0,
                      data.m_bmi->bmiHeader.biWidth, height,
                      0, 0, data.m_bmi->bmiHeader.biWidth, height,
                      m_rgba8888, data.m_bmi, DIB_RGB_COLORS, SRCCOPY);

   ReleaseDC(surf->hWnd, hdc);
}

VkResult V3DPlatformWin32::QueueFrame(
   Device                 *device,
   const VkPresentInfoKHR *presentInfo)
{
   // Wait for the waitSemaphores to fire
   for (uint32_t s = 0; s < presentInfo->waitSemaphoreCount; s++)
   {
      Semaphore *sem = fromHandle<Semaphore>(presentInfo->pWaitSemaphores[s]);
      sem->WaitNow();
   }

   for (uint32_t c = 0; c < presentInfo->swapchainCount; c++)
   {
      SwapchainKHR *chain = fromHandle<SwapchainKHR>(presentInfo->pSwapchains[c]);
      uint32_t      indx = presentInfo->pImageIndices[c];

      DisplayFrame(device, chain, *chain->Images()[indx], chain->PlatformImageData()[indx]);
   }

   return VK_SUCCESS;
}

VkResult V3DPlatformWin32::DequeueFrame(
   SwapchainKHR *chain,
   uint64_t      timeout,
   Semaphore    *semaphore,
   Fence        *fence,
   uint32_t     *pImageIndex)
{
   // The windows platform never has frames 'in-flight'
   // so we can signal semaphores and fences immediately
   *pImageIndex = m_curImage;
   m_curImage = (m_curImage + 1) % m_numImages;

   if (fence != nullptr)
      fence->Signal();

   if (semaphore != nullptr)
      semaphore->SignalNow();

   return VK_SUCCESS;
}

void V3DPlatformWin32::GetSurfaceExtent(
   VkExtent2D  *extent,
   VkSurfaceKHR surface) const
{
   RECT          r;
   SurfaceWin32 *surf = FromSurfaceKHR<SurfaceWin32>(surface);

   GetClientRect(surf->hWnd, &r);
   extent->width  = r.right - r.left;
   extent->height = r.bottom - r.top;
}

} // namespace bvk

#endif // VK_USE_PLATFORM_WIN32_KHR
