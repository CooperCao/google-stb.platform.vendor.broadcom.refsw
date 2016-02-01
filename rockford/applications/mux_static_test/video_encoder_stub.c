/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include <stdio.h>          /* for printf */
#include <stdlib.h>
#include <string.h>

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */
#include "bmem.h"           /* memory interface */

#include "bavc.h"

#include "video_encoder_stub.h"

BDBG_MODULE(VIDEO_ENCODER_STUB);
BDBG_FILE_MODULE(VIDEO_ENCODER_STUB_NALU);

#if 0
#define VENC_PRINT_ITB 1
#endif

#define MAX_ITB_SIZE_PER_FRAME 64
#define MAX_CDB_SIZE_PER_FRAME 16*1024
#define MAX_VIDEODESCRIPTORS 32

typedef struct BVCE_Encoded_Buffer
{
  size_t uiSize;
  char *szFilename;
} BVCE_Encoded_Buffer;

typedef struct BVCE_Encoded_Content
{
      char *szFriendlyName;
      BVCE_Encoded_Buffer stITB;
      BVCE_Encoded_Buffer stCDB;
      BAVC_VideoCompressionStd eProtocol;
} BVCE_Encoded_Content;

#define VENC_DEFAULT_CONTENT_INDEX 1

BVCE_Encoded_Content astEncodedContent[] =
{
 {
  "CNN Ticker (H.264 Main, 720x480, I-Only)",
  {
   0x00030000,
   "content/cnnticker/I-Only/encoder_output.h264.itb",
  },
  {
   0x00300000,
   "content/cnnticker/I-Only/encoder_output.h264.cdb",
  },
  BAVC_VideoCompressionStd_eH264,
 },
 {
  "CNN Ticker (H.264 Main, 720x480, IP-Only, 16 pics/GOP)",
  {
   0x00030000,
   "content/cnnticker/IP-Only/encoder_output.h264.itb",
  },
  {
   0x00300000,
   "content/cnnticker/IP-Only/encoder_output.h264.cdb",
  },
  BAVC_VideoCompressionStd_eH264,
 },
};

typedef union ITBEntry
{
   uint32_t data[16];
   struct {
      struct
      {
         uint32_t reserved:23;
         uint32_t error:1;
         uint32_t entry_type:8;

         uint32_t cdb_address;
         uint32_t reserved0;
         uint32_t reserved1;
      } base_address;

      struct
      {
         uint32_t dts_32:1;
         uint32_t pts_32:1;
         uint32_t reserved1:13;
         uint32_t dts_valid:1;
         uint32_t reserved0:8;
         uint32_t entry_type:8;

         uint32_t pts;

         uint32_t dts;

         uint32_t byte_offset:8;
         uint32_t I_frame:1;
         uint32_t reserved4:23;
      } pts_dts;

      struct
      {
         uint32_t reserved0:24;
         uint32_t entry_type:8;

         uint32_t ticks_per_bit:16;
         int32_t shr:16;

         uint32_t reserved1;
         uint32_t reserved2;
      } bit_rate;

      struct
      {
         uint32_t reserved0:24;
         uint32_t entry_type:8;

         uint32_t escr;
         uint32_t original_pts;

         uint32_t metadata;
      } escr_metadata;

   } fields;
} ITBEntry;

typedef struct VideoEncoderStubContext
{
      VideoEncoderSettings stSettings;

      uint32_t *pITBBuffer;
      uint32_t uiITBBufferSize;
      uint32_t uiITBBufferWriteOffset;
      uint32_t uiITBBufferReadOffset;
      uint32_t uiITBBufferShadowReadOffset;


      uint8_t *pCDBBuffer;
      uint32_t uiCDBBufferOffset;
      uint32_t uiCDBBufferSize;
      uint32_t uiCDBBufferWriteOffset;
      uint32_t uiCDBBufferReadOffset;

      BAVC_VideoBufferDescriptor stDescriptors[MAX_VIDEODESCRIPTORS];
      uint32_t uiDescriptorWriteOffset;
      uint32_t uiDescriptorReadOffset;

      uint32_t uiCDBTranslation;
      bool bCDBTranslationInitialized;

      uint32_t uiESCRTranslation;
      bool bESCRTranslationInitialized;

      uint64_t uiPTSTranslation;
      bool bPTSTranslationInitialized;

      uint32_t uiESCR;

      FILE* hITB;
      FILE* hCDB;

      struct
      {
         uint32_t uiShiftRegister;
         uint32_t uiCurrentOffset;
      } stDataUnitSearch;

      BAVC_VideoCompressionStd eProtocol;

      bool bStop;
      bool bDone;
      bool bInputDone;
} VideoEncoderStubContext;

static void PopulateFrameDescriptorFromITB(VideoEncoderHandle hVideoEncoder, BAVC_VideoBufferDescriptor *pVideoDescriptor, ITBEntry *pITBEntry)
{
    if ( true == pITBEntry->fields.pts_dts.I_frame )
     {
        pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP;
     }

     pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID;
     pVideoDescriptor->stCommon.uiOriginalPTS = pITBEntry->fields.escr_metadata.original_pts;

     pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID;
     pVideoDescriptor->stCommon.uiPTS = pITBEntry->fields.pts_dts.pts_32;
     pVideoDescriptor->stCommon.uiPTS <<= 32;
     pVideoDescriptor->stCommon.uiPTS |= pITBEntry->fields.pts_dts.pts;

#if 0
     if ( true == pITBEntry->fields.pts_dts.dts_valid )
#endif
     {
        pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID;
        pVideoDescriptor->uiDTS = pITBEntry->fields.pts_dts.dts_32;
        pVideoDescriptor->uiDTS <<= 32;
        pVideoDescriptor->uiDTS |= pITBEntry->fields.pts_dts.dts;
     }

     pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID;
     pVideoDescriptor->stCommon.uiESCR = pITBEntry->fields.escr_metadata.escr;

     pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID;
     pVideoDescriptor->stCommon.uiTicksPerBit = pITBEntry->fields.bit_rate.ticks_per_bit;

     pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID;
     pVideoDescriptor->stCommon.iSHR = pITBEntry->fields.bit_rate.shr;

     /* Since we are using canned ITB/CDB, every ITB represents a single video frame and therefore the
        first descriptor is always is a frame start */
     pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START;

     /* DEBUG: Normalize PTS, DTS, and ESCR to start at 0 */
#if 1
     pVideoDescriptor->stCommon.uiPTS -= hVideoEncoder->uiESCRTranslation / 300;
     pVideoDescriptor->uiDTS -= hVideoEncoder->uiESCRTranslation / 300;
     pVideoDescriptor->stCommon.uiESCR -= hVideoEncoder->uiESCRTranslation;
#endif
}

/* Video Encoder Stub */
BERR_Code
app_GetVideoBufferDescriptors(
   VideoEncoderHandle hVideoEncoder,
   const BAVC_VideoBufferDescriptor *astDescriptors0[], /* Can be NULL if uiNumDescriptors0=0. */
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor *astDescriptors1[], /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
   size_t *puiNumDescriptors1
   )
{
   ITBEntry *pITBEntry;
   ITBEntry *pITBEntryNext;
   BAVC_VideoBufferDescriptor *pVideoDescriptor;

   BDBG_ENTER( app_GetVideoBufferDescriptors );

#if VENC_PRINT_ITB
   bool bPrintHeader = true;
#endif

   while ( 1 )
   {
      {
         /* Check for Available Descriptors - need at least one for the start of frame */
         uint32_t uiTempWriteOffset = hVideoEncoder->uiDescriptorWriteOffset + 1;
         if ( uiTempWriteOffset == MAX_VIDEODESCRIPTORS )
         {
            uiTempWriteOffset = 0;
         }
         if (uiTempWriteOffset == hVideoEncoder->uiDescriptorReadOffset )
         {
            break;
         }

         if (hVideoEncoder->bDone)
            break;

         /* check for stop, if so, insert EOS descriptor */
         if (hVideoEncoder->bStop)
         {
            /* Get Video Descriptor for this ITB entry */
            pVideoDescriptor = &hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorWriteOffset];
            hVideoEncoder->uiDescriptorWriteOffset++;
            if ( hVideoEncoder->uiDescriptorWriteOffset == MAX_VIDEODESCRIPTORS )
            {
               hVideoEncoder->uiDescriptorWriteOffset = 0;
            }

            BKNI_Memset(pVideoDescriptor, 0,  sizeof( BAVC_VideoBufferDescriptor ));
            pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;
            hVideoEncoder->bDone = true;
            break;
         }

         if (hVideoEncoder->bInputDone)
            break;

         /* Check for Available ITB Entries */
         if ( ( hVideoEncoder->uiITBBufferWriteOffset - hVideoEncoder->uiITBBufferShadowReadOffset ) < sizeof(ITBEntry) )
         {
            break;
         }

         pITBEntry = (ITBEntry *) ((uint8_t *)hVideoEncoder->pITBBuffer + hVideoEncoder->uiITBBufferShadowReadOffset );

         /* Check for EOS */
         if ( ( 0 == pITBEntry->fields.bit_rate.shr )
              && ( 0 == pITBEntry->fields.bit_rate.ticks_per_bit )
              && ( 0 == pITBEntry->fields.pts_dts.pts )
              && ( 0 == pITBEntry->fields.pts_dts.pts_32 )
              && ( 0 == pITBEntry->fields.pts_dts.dts )
              && ( 0 == pITBEntry->fields.pts_dts.dts_32 )
              && ( 0 == pITBEntry->fields.escr_metadata.original_pts ) )
         {
            /* Get Video Descriptor for this ITB entry */
            pVideoDescriptor = &hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorWriteOffset];
            hVideoEncoder->uiDescriptorWriteOffset++;
            if ( hVideoEncoder->uiDescriptorWriteOffset == MAX_VIDEODESCRIPTORS )
            {
               hVideoEncoder->uiDescriptorWriteOffset = 0;
            }

            BKNI_Memset(pVideoDescriptor, 0,  sizeof( BAVC_VideoBufferDescriptor ));
            pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;
            hVideoEncoder->bInputDone = true;
            break;
         }

         /* Check for Available ITB Entries */
         if ( ( hVideoEncoder->uiITBBufferWriteOffset - hVideoEncoder->uiITBBufferShadowReadOffset ) <= sizeof(ITBEntry) )
         {
            break;
         }
      }
#if VENC_PRINT_ITB
      if ( true == bPrintHeader )
      {
         BKNI_Printf("*** ITB Dump ***\n");
         BKNI_Printf("address,dts(90Khz),dts valid,pts(90Khz),opts(45Khz),escr(27Mhz),tpb,shr,error,I frame,metadata\n");
         bPrintHeader = false;
      }
#endif
      pITBEntryNext = (ITBEntry *) ((uint8_t *)hVideoEncoder->pITBBuffer + hVideoEncoder->uiITBBufferShadowReadOffset + sizeof(ITBEntry));
#if VENC_PRINT_ITB
      BKNI_Printf("%u,%llu,%d,%llu,%u,%u,%u,%d,%d,%d,%08x\n",
         pITBEntry->fields.base_address.cdb_address,
         (((uint64_t) pITBEntry->fields.pts_dts.dts_32 ) << 32) | pITBEntry->fields.pts_dts.dts,
         pITBEntry->fields.pts_dts.dts_valid,
         (((uint64_t) pITBEntry->fields.pts_dts.pts_32 ) << 32) | pITBEntry->fields.pts_dts.pts,
         pITBEntry->fields.escr_metadata.original_pts,
         pITBEntry->fields.escr_metadata.escr,
         pITBEntry->fields.bit_rate.ticks_per_bit,
         (int16_t) pITBEntry->fields.bit_rate.shr,
         pITBEntry->fields.base_address.error,
         pITBEntry->fields.pts_dts.I_frame,
         pITBEntry->fields.escr_metadata.metadata
         );
#endif

#if 0
      /* TODO: Split data buffer into exactly the number of bytes that would be encoded by uiESCR */
      if ( pITBEntry->fields.escr_metadata.escr > hVideoEncoder->uiESCR )
      {
         /* Pace the video encoder output by only queuing video frames that have been encoded since
          * last execution time */
         break;
      }
#endif

      if ( false == hVideoEncoder->bCDBTranslationInitialized )
      {
         hVideoEncoder->uiCDBTranslation = hVideoEncoder->uiCDBBufferOffset - pITBEntry->fields.base_address.cdb_address;
         hVideoEncoder->bCDBTranslationInitialized = true;
#if 0
         BKNI_Printf("Original Base = %08x, New Base = %08x, Translation = %08x\n",
                  pITBEntry->fields.base_address.cdb_address,
                  hVideoEncoder->uiCDBBufferOffset,
                  hVideoEncoder->uiCDBTranslation
                  );
#endif
      }

      if ( false == hVideoEncoder->bESCRTranslationInitialized )
      {
         hVideoEncoder->uiESCRTranslation = pITBEntry->fields.escr_metadata.escr;
         hVideoEncoder->bESCRTranslationInitialized = true;
#if 0
         BKNI_Printf("Original ESCR = %08x, New ESCR = %08x, Translation = %08x\n",
                  pITBEntry->fields.escr_metadata.escr,
                  0,
                  hVideoEncoder->uiESCRTranslation
                  );
#endif
      }

      if ( false == hVideoEncoder->bPTSTranslationInitialized )
      {
         hVideoEncoder->uiPTSTranslation = pITBEntry->fields.pts_dts.pts_32;
         hVideoEncoder->uiPTSTranslation <<= 32;
         hVideoEncoder->uiPTSTranslation |= pITBEntry->fields.pts_dts.pts;
         hVideoEncoder->bPTSTranslationInitialized = true;
#if 0
         BKNI_Printf("Original PTS = %0x, New PTS = %08x, Translation = %08x\n",
                  pITBEntry->fields.PTS_metadata.PTS,
                  0,
                  hVideoEncoder->uiPTSTranslation
                  );
#endif
      }

      /* check for CDB wrap (not supported) - if so, do nothing (i.e. stop producing output) */
      if (pITBEntryNext->fields.base_address.cdb_address < pITBEntry->fields.base_address.cdb_address)
      {
         /* Get Video Descriptor for this ITB entry */
         pVideoDescriptor = &hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorWriteOffset];
         hVideoEncoder->uiDescriptorWriteOffset++;
         if ( hVideoEncoder->uiDescriptorWriteOffset == MAX_VIDEODESCRIPTORS )
         {
            hVideoEncoder->uiDescriptorWriteOffset = 0;
         }

         BKNI_Memset(pVideoDescriptor, 0,  sizeof( BAVC_VideoBufferDescriptor ));
         pVideoDescriptor->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;

         hVideoEncoder->bInputDone = true;
         break;
      }

      if (BAVC_VideoCompressionStd_eH264 != hVideoEncoder->eProtocol)
      {
         /* Get Video Descriptor for this ITB entry */
         pVideoDescriptor = &hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorWriteOffset];
         hVideoEncoder->uiDescriptorWriteOffset++;
         if ( hVideoEncoder->uiDescriptorWriteOffset == MAX_VIDEODESCRIPTORS )
         {
            hVideoEncoder->uiDescriptorWriteOffset = 0;
         }

         BKNI_Memset(
                  pVideoDescriptor,
                  0,
                  sizeof( BAVC_VideoBufferDescriptor )
                  );

         /* populate the per-frame information ... */
         PopulateFrameDescriptorFromITB(hVideoEncoder, pVideoDescriptor, pITBEntry);

         pVideoDescriptor->stCommon.uiOffset = (pITBEntry->fields.base_address.cdb_address + hVideoEncoder->uiCDBTranslation) - hVideoEncoder->uiCDBBufferOffset;
         pVideoDescriptor->stCommon.uiLength = pITBEntryNext->fields.base_address.cdb_address - pITBEntry->fields.base_address.cdb_address;
      } /* end: if !H.264 */

      else /* BAVC_VideoCompressionStd_eH264 == hVideoEncoder->eProtocol */
      {
         uint32_t uiShiftRegister = hVideoEncoder->stDataUnitSearch.uiShiftRegister;
         /* this offset will either be zero (start of frame), or point to last byte of the
            latest start code that was found, but not processed */
         uint32_t uiOffset = hVideoEncoder->stDataUnitSearch.uiCurrentOffset;
         uint32_t uiPrevStartCodeOffset = 0;
         uint32_t i;
         bool bStartCode = false;
         uint32_t uiFrameOffset = (pITBEntry->fields.base_address.cdb_address + hVideoEncoder->uiCDBTranslation) - hVideoEncoder->uiCDBBufferOffset;
         uint32_t uiFrameLength = pITBEntryNext->fields.base_address.cdb_address - pITBEntry->fields.base_address.cdb_address;
         /* NOTE: need to adjust the starting byte by any offset left over from last time */
         uint8_t *pData = hVideoEncoder->pCDBBuffer + uiFrameOffset + uiOffset;

         /* create descriptors for each NALU and populate the per-NALU information: */
         /* in here, we know we have at least one available descriptor (checked above) */

         pVideoDescriptor = NULL;      /* to keep compiler happy (uninitialized warning) */
         /* scan the CDB data to find start codes ... */
         BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Current Offset = %d, Frame Offset = %d, Frame Length = %d, Shift Reg = %8.8x", uiOffset, uiFrameOffset, uiFrameLength, uiShiftRegister));
         /* if we ran out of descriptors last time, start processing again assuming the start
            code is already in the shift register (i.e. start from first byte of the NALU that
            was not processed previously) */
         for (i = uiOffset; i < uiFrameLength; i++)
         {
            /* NOTE: This code does not need to care if the NALU type is split from
               the start code.  Actual processing will need to take care of this possibility
               (since the start code may be in a different descriptor) */
            if (bStartCode)
            {
               BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Start code found, NALU type = %d", *pData));
               /* fill in the NALU type for this descriptor */
               pVideoDescriptor->uiDataUnitType = *pData;
            }
            uiShiftRegister |= *pData++;
            bStartCode = (uiShiftRegister == 0x00000001);
            uiShiftRegister <<= 8;
            if (bStartCode)
            {
               uint32_t uiTempWriteOffset = hVideoEncoder->uiDescriptorWriteOffset + 1;
               uint32_t uiStartCodeOffset = i-3;
               BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Start Code Found @ %d", i-3));
               /* NOTE: This code assumes that the very first bytes of a new ITB entry are the start code of the frame
                        If that's not the case, this will ignore any data prior to the first start code */

               /* populate the length information for the previous descriptor (not necessary to do
                  this again if we ran out of descriptors - would be done prior to exiting) */
               /* length of this descriptor = location of this start code - location of prev start code
                  (NOTE: length includes the start code) */
               if (0 != uiStartCodeOffset && pVideoDescriptor != NULL)
               {
                  BDBG_MSG(("NALU Detected @ offset: %d, Type = %d (byte = 0x%x), Length = %d", uiFrameOffset + uiPrevStartCodeOffset,
                           pVideoDescriptor->uiDataUnitType & 0x1f, pVideoDescriptor->uiDataUnitType, uiStartCodeOffset - uiPrevStartCodeOffset));

                  pVideoDescriptor->stCommon.uiLength = uiStartCodeOffset - uiPrevStartCodeOffset;
               }
               BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Looking for available descriptors"));
               /* check for available descriptors for this NALU */
               if ( uiTempWriteOffset == MAX_VIDEODESCRIPTORS )
               {
                  uiTempWriteOffset = 0;
               }
               if (uiTempWriteOffset == hVideoEncoder->uiDescriptorReadOffset )
               {
                  /* if not, quit - the code will come back here next time */
                  BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("No Descriptors"));
                  break;
               }
               BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Allocating descriptor"));
               /* allocate a descriptor for this NALU ... */
               pVideoDescriptor = &hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorWriteOffset];
               hVideoEncoder->uiDescriptorWriteOffset++;
               if ( hVideoEncoder->uiDescriptorWriteOffset == MAX_VIDEODESCRIPTORS )
               {
                  hVideoEncoder->uiDescriptorWriteOffset = 0;
               }
               BKNI_Memset(pVideoDescriptor, 0, sizeof( BAVC_VideoBufferDescriptor ));

               if (0 == uiStartCodeOffset)
               {
                  BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Populating descriptor with per-frame info"));
                  /* if first descriptor of the frame, populate the per-frame information */
                  PopulateFrameDescriptorFromITB(hVideoEncoder, pVideoDescriptor, pITBEntry);
               }
               BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Populating NALU info"));
               /* populate the NALU information for this descriptor */
               /* Note: length will be filled in when the Next Start Code detected */
               pVideoDescriptor->uiVideoFlags |= BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START;
               /* offset is the start code offset ... */
               pVideoDescriptor->stCommon.uiOffset = uiFrameOffset + uiStartCodeOffset;
               uiPrevStartCodeOffset = uiStartCodeOffset;
               BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Start Code Offset = %d",  uiStartCodeOffset));
            } /* end: if start code */
         } /* end for each byte */
         BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Done Frame"));

         hVideoEncoder->stDataUnitSearch.uiShiftRegister = uiShiftRegister;
         if (i != uiFrameLength)
         {
            /* frame was not completely processed (ran out of descriptors),
               so quit and come back later */
            BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Frame not complete (current index = %d), shift reg = %8.8x", i, uiShiftRegister));
            hVideoEncoder->stDataUnitSearch.uiCurrentOffset = i;
            /* roll back the current byte just shifted into the shift register, since it will be written in again next time round */
            hVideoEncoder->stDataUnitSearch.uiShiftRegister >>= 8;
            hVideoEncoder->stDataUnitSearch.uiShiftRegister &= ~0xFF;
            break;
         }
         else
         {
            /* Complete the last NALU descriptor in this frame ... */
            BDBG_MSG(("NALU Detected @ offset: %d, Type = %d (byte = 0x%x), Length = %d", uiFrameOffset + uiPrevStartCodeOffset,
                    pVideoDescriptor->uiDataUnitType & 0x1f, pVideoDescriptor->uiDataUnitType, uiFrameLength - uiPrevStartCodeOffset));

            pVideoDescriptor->stCommon.uiLength = uiFrameLength - uiPrevStartCodeOffset;
            hVideoEncoder->stDataUnitSearch.uiCurrentOffset = 0;
         }
      }  /* end: H.264 specific */

      /* Goto next frame's ITB Entry */
      /* TODO: Handle ITB wrap */
      BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Finding next ITB"));
      hVideoEncoder->uiITBBufferShadowReadOffset += sizeof(ITBEntry);
   } /* end: while 1 */

   BDBG_MODULE_MSG(VIDEO_ENCODER_STUB_NALU, ("Assigning descriptor array"));
   /* Assign array(s) and count(s) */
   if ( hVideoEncoder->uiDescriptorWriteOffset >= hVideoEncoder->uiDescriptorReadOffset )
   {
      *astDescriptors0 = &hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorReadOffset];
      *puiNumDescriptors0 = hVideoEncoder->uiDescriptorWriteOffset - hVideoEncoder->uiDescriptorReadOffset;

      *astDescriptors1 = NULL;
      *puiNumDescriptors1 = 0;
   }
   else
   {
      *astDescriptors0 = &hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorReadOffset];
      *puiNumDescriptors0 = MAX_VIDEODESCRIPTORS - hVideoEncoder->uiDescriptorReadOffset;

      *astDescriptors1 = &hVideoEncoder->stDescriptors[0];
      *puiNumDescriptors1 = hVideoEncoder->uiDescriptorWriteOffset;
   }

   BDBG_LEAVE( app_GetVideoBufferDescriptors );

   return BERR_TRACE( BERR_SUCCESS );
}


BERR_Code
app_ConsumeVideoBufferDescriptors(
   VideoEncoderHandle hVideoEncoder,
   size_t uiNumDescriptors
   )
{
   while ( uiNumDescriptors )
   {
      /* Move CDB Read Offset */
      hVideoEncoder->uiCDBBufferReadOffset += hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorReadOffset].stCommon.uiLength;
      if ( hVideoEncoder->uiCDBBufferReadOffset >= hVideoEncoder->uiCDBBufferSize )
      {
         hVideoEncoder->uiCDBBufferReadOffset = hVideoEncoder->uiCDBBufferReadOffset - hVideoEncoder->uiCDBBufferSize;
      }

      /* TODO: Load more CDB data from file */

      if ( true == ( hVideoEncoder->stDescriptors[hVideoEncoder->uiDescriptorReadOffset].stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )
      {
         /* Move ITB Read Offset */
         hVideoEncoder->uiITBBufferReadOffset += sizeof(ITBEntry);
         if ( hVideoEncoder->uiITBBufferReadOffset >= hVideoEncoder->uiITBBufferSize )
         {
            hVideoEncoder->uiITBBufferReadOffset = hVideoEncoder->uiITBBufferReadOffset - hVideoEncoder->uiITBBufferSize;
         }

         /* TODO: Load more ITB data from file */
      }

      /* Move Descriptor Read Offset */
      hVideoEncoder->uiDescriptorReadOffset++;
      hVideoEncoder->uiDescriptorReadOffset %= MAX_VIDEODESCRIPTORS;

      uiNumDescriptors--;
   }

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_GetVideoBufferStatus(
   VideoEncoderHandle hVideoEncoder,
   BAVC_VideoBufferStatus *pBufferStatus
)
{
   BKNI_Memset(pBufferStatus, 0, sizeof(BAVC_VideoBufferStatus));
#if 0
   pBufferStatus->stCommon.pFrameBufferBaseAddress = hVideoEncoder->pCDBBuffer;
#else
   BMEM_Heap_ConvertAddressToCached(hVideoEncoder->stSettings.hMem, hVideoEncoder->pCDBBuffer, (void **)&pBufferStatus->stCommon.pFrameBufferBaseAddress);
#endif

   return BERR_TRACE( BERR_SUCCESS );
}

#define SWAP32(val32) ((((uint32_t) (val32) & 0xff000000) >> 24) | \
                         (((uint32_t) (val32) & 0x00ff0000) >> 8) | \
                         (((uint32_t) (val32) & 0x0000ff00) << 8) | \
                         (((uint32_t) (val32) & 0x000000ff) << 24))

BERR_Code
app_OpenVideoEncoder(
         VideoEncoderHandle *phVideoEncoder,
         const VideoEncoderSettings* pstSettings
         )
{
   VideoEncoderStubContext *pVideoContext = NULL;
   uint32_t uiContentIndex = VENC_DEFAULT_CONTENT_INDEX;

   pVideoContext = ( VideoEncoderStubContext* ) BKNI_Malloc( sizeof ( VideoEncoderStubContext ) );

   BKNI_Memset(
            pVideoContext,
            0,
            sizeof ( VideoEncoderStubContext )
            );

   pVideoContext->stSettings = *pstSettings;

   BKNI_Printf("Using Content[%d] - %s\n",
               uiContentIndex,
               astEncodedContent[uiContentIndex].szFriendlyName
               );

   pVideoContext->uiITBBufferSize = astEncodedContent[uiContentIndex].stITB.uiSize;
   pVideoContext->pITBBuffer = (uint32_t *) BMEM_AllocAligned(
               pVideoContext->stSettings.hMem,
               pVideoContext->uiITBBufferSize,
               8,
               0);

   BKNI_Memset(
            pVideoContext->pITBBuffer,
            0,
            pVideoContext->uiITBBufferSize
            );

   /* Load ITB into memory */
   pVideoContext->hITB = fopen(astEncodedContent[uiContentIndex].stITB.szFilename, "rb");
   if ( NULL ==    pVideoContext->hITB )
   {
      printf("Error opening ITB file\n");
      return BERR_TRACE( BERR_UNKNOWN );
   }

   pVideoContext->uiITBBufferWriteOffset = fread( pVideoContext->pITBBuffer,
          1,
          pVideoContext->uiITBBufferSize,
          pVideoContext->hITB );


   pVideoContext->uiCDBBufferSize = astEncodedContent[uiContentIndex].stCDB.uiSize;
   pVideoContext->pCDBBuffer = (uint8_t *) BMEM_AllocAligned(
            pVideoContext->stSettings.hMem,
            pVideoContext->uiCDBBufferSize,
            8,
            0);

   BMEM_ConvertAddressToOffset(
            pVideoContext->stSettings.hMem,
            (void*) pVideoContext->pCDBBuffer,
            &pVideoContext->uiCDBBufferOffset
            );

   BKNI_Memset(
            pVideoContext->pCDBBuffer,
            0,
            pVideoContext->uiCDBBufferSize
            );

   /* Load CDB into memory */
   pVideoContext->hCDB = fopen(astEncodedContent[uiContentIndex].stCDB.szFilename, "rb");
   if ( NULL == pVideoContext->hCDB )
   {
      printf("Error opening CDB file\n");
      return BERR_TRACE( BERR_UNKNOWN );
   }

   pVideoContext->uiCDBBufferWriteOffset = fread( pVideoContext->pCDBBuffer,
          1,
          pVideoContext->uiCDBBufferSize,
          pVideoContext->hCDB );

   pVideoContext->stDataUnitSearch.uiShiftRegister =0;
   pVideoContext->stDataUnitSearch.uiCurrentOffset = 0;
   pVideoContext->eProtocol = astEncodedContent[uiContentIndex].eProtocol;

   *phVideoEncoder = pVideoContext;

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code
app_CloseVideoEncoder(
         VideoEncoderHandle hVideoEncoder
         )
{
   fclose( hVideoEncoder->hCDB );
   fclose( hVideoEncoder->hITB );

   BMEM_Free(
            hVideoEncoder->stSettings.hMem,
            hVideoEncoder->pCDBBuffer
            );

   BMEM_Free(
            hVideoEncoder->stSettings.hMem,
            hVideoEncoder->pITBBuffer
            );

   BKNI_Free(
            hVideoEncoder
            );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_StopVideoEncoder(
         VideoEncoderHandle hVideoEncoder
         )
{
   hVideoEncoder->bStop = true;
   return BERR_SUCCESS;
}

BERR_Code
app_VideoEncoderIncrementTime(
         VideoEncoderHandle hVideoEncoder,
         unsigned uiMilliseconds
         )
{
   hVideoEncoder->uiESCR += 27000000 / 1000 * uiMilliseconds;

   return BERR_TRACE( BERR_SUCCESS );
}

bool app_IsInputDone(
   VideoEncoderHandle hVideoEncoder
   )
{
   return hVideoEncoder->bInputDone;
}
