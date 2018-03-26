/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "autoclif.h"
#include "memory_nexus_priv.h"

#define AUTOCLIF_DEBUG 0

#ifdef ANDROID
#define LOG_TAG "V3D"
#include <log/log.h>
#include <cutils/properties.h>
#include <libgen.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <search.h>

#define BLOCK_SIZE (1024 * 1024)

#ifdef ANDROID
static void autoclif_log(const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, fmt, args);
   va_end(args);
}
#else
static void autoclif_log(const char *fmt, ...)
{
   char buffer[256];
   va_list args;
   va_start(args, fmt);
   vsnprintf(buffer, sizeof(buffer), fmt, args);
   printf("%s\n", buffer);
}
#endif

static const BM2MC_PACKET_Blend copyColor = {BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
    BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};
static const BM2MC_PACKET_Blend copyAlpha = {BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, false,
    BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};

/* dst is a nexus blockhandle or virtual address, depending on whether in MMA mode or not.
   src is already a device offset provided by the clif parser */
static NEXUS_Error graphics_memcpy(void *context, void *dst, NEXUS_Addr src_addr, uint32_t size)
{
   NXPL_MemoryData *data = (NXPL_MemoryData*)context;
   const unsigned stride = 4096;
   void *pkt_buffer, *next;
   size_t pkt_size;
   NEXUS_Error rc;
   NEXUS_Addr dst_addr;

   if (size % stride)
      return BERR_TRACE(NEXUS_NOT_SUPPORTED);

   rc = NEXUS_Graphics2D_GetPacketBuffer(data->gfx, &pkt_buffer, &pkt_size, 1024);
   BDBG_ASSERT(!rc);

   /* unlock will be done later at free/cleanup */
   dst_addr = (NEXUS_Addr)data->memInterface->MemLockOffset(context, dst);

   next = pkt_buffer;
   {
      BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
      BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
      pPacket->plane.address = dst_addr;
      pPacket->plane.pitch = stride;
      pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
      pPacket->plane.width = stride / 4;
      pPacket->plane.height = size / stride;
      next = ++pPacket;
   }

   {
      BM2MC_PACKET_PacketBlend *pPacket = (BM2MC_PACKET_PacketBlend *)next;
      BM2MC_PACKET_INIT(pPacket, Blend, false);
      pPacket->color_blend = copyColor;
      pPacket->alpha_blend = copyAlpha;
      pPacket->color = 0;
      next = ++pPacket;
   }

   {
      BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
      BM2MC_PACKET_INIT(pPacket, SourceFeeder, false );
      pPacket->plane.address = src_addr;
      pPacket->plane.pitch = stride;
      pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
      pPacket->plane.width = stride / 4;
      pPacket->plane.height = size / stride;
      next = ++pPacket;
   }

   {
      BM2MC_PACKET_PacketScaleBlit *pPacket = next;
      BM2MC_PACKET_INIT(pPacket, ScaleBlit, true);
      pPacket->src_rect.x = 0;
      pPacket->src_rect.y = 0;
      pPacket->src_rect.width = stride / 4;
      pPacket->src_rect.height = size / stride;
      pPacket->out_rect.x = 0;
      pPacket->out_rect.y = 0;
      pPacket->out_rect.width = stride / 4;
      pPacket->out_rect.height = size/stride;
      next = ++pPacket;
   }

   rc = NEXUS_Graphics2D_PacketWriteComplete(data->gfx, (uint8_t*)next - (uint8_t*)pkt_buffer);
   if (rc != NEXUS_SUCCESS)
      return BERR_TRACE(rc);

   rc = NEXUS_Graphics2D_Checkpoint(data->gfx, NULL);
   if (rc == NEXUS_GRAPHICS2D_QUEUED)
      BKNI_WaitForEvent(data->checkpointEvent, BKNI_INFINITE);
   else if (rc != NEXUS_SUCCESS)
      return BERR_TRACE(rc);

   return NEXUS_SUCCESS;
}

static volatile void *copy_to_block(void *context, uint32_t src, uint32_t size, void **block)
{
   NXPL_MemoryData *data = (NXPL_MemoryData*)context;

   /* returns either a block handle or a virtual address depending on MMA mode */
   void *tmp = data->memInterface->Alloc(context, size, 4096, false);
   if (tmp == NULL)
   {
      printf("allocation failed : FATAL\n");
      exit(0);
   }

   /* invalidate cache prior to DMA */
   data->memInterface->FlushCache(context, tmp, size);

   graphics_memcpy(context, tmp, (NEXUS_Addr)src, size);

   /* invalidate again in case of speculative reads */
   data->memInterface->FlushCache(context, tmp, size);

   /* keep the block handle for the free/unmap in the MMA case */
   *block = tmp;

   return data->memInterface->MemLock(context, tmp);
}

static void free_block(void *context, void *block)
{
   NXPL_MemoryData *data = (NXPL_MemoryData*)context;

   data->memInterface->MemUnlock(context, block);

   data->memInterface->Free(context, block);
}

static int compare(const void *pa, const void *pb)
{
   if (*(uint32_t *)pa < *(uint32_t *)pb)
      return -1;
   if (*(uint32_t *)pa > *(uint32_t *)pb)
      return 1;
   return 0;
}

typedef struct
{
   uint32_t          key;        /* in our case an address masked to 1MB */
   volatile void     *address;   /* virtual address for the map */
   void              *block;     /* used to free up the MMA blocks */
   void              *context;   /* stash this, as its needed for free */
} map_entry_t;

void *MemDebugAutoclifAddrToPtr(void *context, uint32_t addr)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

   map_entry_t *p = calloc(1, sizeof(map_entry_t));
   if (p)
   {
      /* address is 1MB aligned in the map */
      p->key = addr & ~(BLOCK_SIZE - 1);
      p->context = context;

      /* is this already mapped? */
      void *h = tsearch((void *)p, &data->translationTable, compare);

      map_entry_t *hit = *(map_entry_t **)h;

      if (hit == NULL)
      {
         /* failure case, shouldn't ever do this */
         autoclif_log("failed to lookup/insert memory address\n");
         exit(0);
      }
      else if (hit != p)
      {
         /* match an existing entry, just go from that */
         free(p);

         void *ret = (void *)((uintptr_t)hit->address + (addr & (BLOCK_SIZE - 1)));

         if (AUTOCLIF_DEBUG)
         {
            autoclif_log("match existing phys(%p) key(%p) virt(%p) ret(%p)",
               (void *)(uintptr_t)addr,
               (void *)(uintptr_t)hit->key,
               hit->address,
               ret);
         }

         return ret;
      }
      else
      {
         /* no entry for this, so create an fd and insert */
         /* hit->address = ioremap(hit->key, BLOCK_SIZE, data->fd); */
         hit->address = copy_to_block(context, hit->key, BLOCK_SIZE, &hit->block);
         if (hit->address == NULL)
         {
            /* failure case, shouldn't ever do this */
            autoclif_log("failed to ioremap memory address");
            exit(0);
         }

         void *ret = (void *)((uintptr_t)hit->address + (addr & (BLOCK_SIZE - 1)));

         if (AUTOCLIF_DEBUG)
         {
            autoclif_log("match new phys(%p) key(%p) virt(%p) ret(%p)",
               (void *)(uintptr_t)addr,
               (void *)(uintptr_t)hit->key,
               hit->address,
               ret);
         }

         return ret;
      }
   }
   else
      return NULL;
}

uint32_t MemDebugAutoclifPtrToAddr(void *context, void *p)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(p);
   autoclif_log("Not implemented %s", __FUNCTION__);
   exit(0);
   return 0;
}

#ifdef ANDROID
extern const char *MemDebugAutoclifGetClifFilename(void *context)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;
   char buffer[256];
   static char filename[256];

   autoclif_log("ENTER MemDebugAutoclifGetClifFilename");

   snprintf(buffer, sizeof(buffer), "/proc/%d/cmdline", getpid());

   FILE *fp = fopen(buffer, "r");
   if (fp != NULL)
   {
      buffer[0] = '\0';
      fgets(buffer, sizeof(buffer), fp);
      fclose(fp);

      char *filename = basename(buffer);

      sprintf(filename, "/data/data/%s/rec_%u.clif", filename, data->filecnt++);

      autoclif_log("Capture to filename %s", filename);

      autoclif_log("EXIT MemDebugAutoclifGetClifFilename");

      return filename;
   }

   autoclif_log("EXIT MemDebugAutoclifGetClifFilename (path 2)");

   return NULL;
}
#else
extern const char *MemDebugAutoclifGetClifFilename(void *context)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;
   static char filename[256];
   sprintf(filename, "rec_%u.clif", data->filecnt++);
   return filename;
}
#endif

static void action(const void *nodep, const VISIT which, const int depth)
{
   BSTD_UNUSED(depth);
   map_entry_t *p;
   switch (which)
   {
      default:
      case endorder:
      case preorder:
         break;
      case leaf:
      case postorder:
         p = *(map_entry_t **)nodep;
         free_block(p->context, p->block);
         break;
   }
}

extern void MemDebugAutoclifReset(void *context)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;
   twalk(data->translationTable, action);
   tdestroy(data->translationTable, free);
   data->translationTable = NULL;
   return;
}
