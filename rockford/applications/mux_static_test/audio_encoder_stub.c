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

#include <stdio.h>

/* base modules */
#include "bstd.h"           /* standard types */
#include "bkni.h"
#include "bavc.h"

#include "audio_encoder_stub.h"

BDBG_MODULE(AUDIO_ENCODER_STUB);

#define MAX_AUDIO_DESCRIPTORS 500
#define MAX_DESC_LINE_LENGTH   128   /* line length is actually fixed format at 108 bytes excluding termination */

typedef struct Audio_Encoded_Buffer
{
  size_t uiSize;
  char *szFilename;
} Audio_Encoded_Buffer;

/* NOTE: there are no ITB entries for this content ... the data is read directly from the CSV file as Audio Descriptors */
typedef struct Audio_Encoded_Content
{
      char *szFriendlyName;
      Audio_Encoded_Buffer stDesc;
      Audio_Encoded_Buffer stCDB;
      BAVC_AudioCompressionStd eProtocol;
} Audio_Encoded_Content;

static Audio_Encoded_Content astEncodedContent[] =
{
 {
  "testjun26_188 (AAC, 48kHz)",
  {
   0x00030000, /* not used */
   "content/testjun26_188/audio_00.csv",
  },
  {
   0x00300000,
   "content/testjun26_188/audio_00.cdb",
  },
  BAVC_AudioCompressionStd_eAac,
 }
};

typedef struct AudioEncoderStubContext
{
      AudioEncoderSettings stSettings;

      uint8_t *pCDBBuffer;
      uint32_t uiCDBBufferOffset;
      uint32_t uiCDBBufferSize;
      uint32_t uiCDBBufferWriteOffset;
      uint32_t uiCDBBufferReadOffset;

      /* note: descriptors are pre-loaded directly from CSV file */
      BAVC_AudioBufferDescriptor stDescriptors[MAX_AUDIO_DESCRIPTORS];
      /* movement of the write offset is paced based on ESCR - i.e. only descriptors relevant to
         the encode time are queued for delivery */
      uint32_t uiDescriptorWriteOffset;
      uint32_t uiDescriptorReadOffset;
      uint32_t uiDescriptorCount;

      uint32_t uiInitialESCR;
      uint32_t uiInitialESCRScaled;

      uint32_t uiCurrentESCR;

      FILE* hDesc;
      FILE* hCDB;

      BAVC_AudioCompressionStd eProtocol;
      bool bStop;
      bool bDone;
} AudioEncoderStubContext;


BERR_Code
app_GetAudioBufferDescriptors(
   AudioEncoderHandle hAudioEncoder,
   const BAVC_AudioBufferDescriptor *astDescriptors0[], /* Can be NULL if uiNumDescriptors0=0. */
   size_t *puiNumDescriptors0,
   const BAVC_AudioBufferDescriptor *astDescriptors1[], /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
   size_t *puiNumDescriptors1
   )
{
   BAVC_AudioBufferDescriptor *pCurrentDesc = &hAudioEncoder->stDescriptors[hAudioEncoder->uiDescriptorWriteOffset];

   /* move write offset up based on which ESCR is <= current ESCR */
   /* Pace the audio encoder output by only queuing audio frames that have been encoded since
    * last execution time */
   while (pCurrentDesc->stCommon.uiESCR <= hAudioEncoder->uiCurrentESCR)
   {
      if (hAudioEncoder->bDone)
         break;

      if (0 == hAudioEncoder->uiDescriptorCount)
         break;   /* no more descriptors available */

      BDBG_MSG(("Queuing descriptor %d: ESCR: %d (current = %d)", hAudioEncoder->uiDescriptorWriteOffset, pCurrentDesc->stCommon.uiESCR, hAudioEncoder->uiCurrentESCR));

      if (hAudioEncoder->bStop)
      {
         /* overwrite the current descriptor with a stop descriptor */
         BKNI_Memset(pCurrentDesc, 0, sizeof(BAVC_AudioBufferDescriptor));
         pCurrentDesc->stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS;
         /* stop processing more descriptors */
         hAudioEncoder->bDone = true;
      }
      hAudioEncoder->uiDescriptorWriteOffset++;
      if (hAudioEncoder->uiDescriptorWriteOffset == MAX_AUDIO_DESCRIPTORS)
         hAudioEncoder->uiDescriptorWriteOffset = 0;

      hAudioEncoder->uiDescriptorCount--;
      pCurrentDesc = &hAudioEncoder->stDescriptors[hAudioEncoder->uiDescriptorWriteOffset];
   } /* end while desc.escr <= current escr */

   /* NOTE: No CDB translation is required, since offsets in descriptors are relative to the start of the
   CDB buffer we load the data into */

   /* Assign array(s) and count(s) */
   if ( hAudioEncoder->uiDescriptorWriteOffset >= hAudioEncoder->uiDescriptorReadOffset )
   {
      *astDescriptors0 = &hAudioEncoder->stDescriptors[hAudioEncoder->uiDescriptorReadOffset];
      *puiNumDescriptors0 = hAudioEncoder->uiDescriptorWriteOffset - hAudioEncoder->uiDescriptorReadOffset;

      *astDescriptors1 = NULL;
      *puiNumDescriptors1 = 0;
   }
   else
   {
      *astDescriptors0 = &hAudioEncoder->stDescriptors[hAudioEncoder->uiDescriptorReadOffset];
      *puiNumDescriptors0 = MAX_AUDIO_DESCRIPTORS - hAudioEncoder->uiDescriptorReadOffset;

      *astDescriptors1 = &hAudioEncoder->stDescriptors[0];
      *puiNumDescriptors1 = hAudioEncoder->uiDescriptorWriteOffset;
   }

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code app_ConsumeAudioBufferDescriptors(AudioEncoderHandle hAudioEncoder, size_t uiNumDescriptors)
{
   while (uiNumDescriptors)
   {
      /* Move CDB Read Offset */
      hAudioEncoder->uiCDBBufferReadOffset += hAudioEncoder->stDescriptors[hAudioEncoder->uiDescriptorReadOffset].stCommon.uiLength;
      if (hAudioEncoder->uiCDBBufferReadOffset >= hAudioEncoder->uiCDBBufferSize)
      {
         hAudioEncoder->uiCDBBufferReadOffset = hAudioEncoder->uiCDBBufferReadOffset - hAudioEncoder->uiCDBBufferSize;
      }

      /* Move Descriptor Read Offset */
      hAudioEncoder->uiDescriptorReadOffset++;
      if (hAudioEncoder->uiDescriptorReadOffset == MAX_AUDIO_DESCRIPTORS)
         hAudioEncoder->uiDescriptorReadOffset = 0;

      uiNumDescriptors--;
   }

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code app_GetAudioBufferStatus(AudioEncoderHandle hAudioEncoder, BAVC_AudioBufferStatus *pBufferStatus)
{
   BKNI_Memset(pBufferStatus, 0, sizeof(BAVC_AudioBufferStatus));
   pBufferStatus->stCommon.pFrameBufferBaseAddress = hAudioEncoder->pCDBBuffer;
   return BERR_TRACE( BERR_SUCCESS );
}


/* read a descriptor from CSV file ...
   Format:
   channel,count,pacing(27Mhz),offset,length,flags,pts(90Khz),opts(45Khz),escr(27Mhz),tpb,shr
   (one descriptor per line)
   Returns false if no more descriptors ...
*/
bool ReadDescriptor(AudioEncoderHandle hAudioEncoder)
{
   char aLine[MAX_DESC_LINE_LENGTH];
   uint32_t uiDescriptorCount;
   uint32_t uiChannel;              /* value is not used */
   uint32_t uiPacing;               /* value is not used */
   uint32_t uiPTS;                  /* NOTE: This is a 32-bit value! PTS is actually a 64-bit value but sscanf does not support 64-bit reads */
   uint32_t uiOrigPTS;
   uint32_t uiESCR;
   BAVC_AudioBufferDescriptor *pDesc = &hAudioEncoder->stDescriptors[hAudioEncoder->uiDescriptorCount];
   BAVC_CompressedBufferDescriptor *pCommon = &pDesc->stCommon;

   BKNI_Memset(pDesc, 0, sizeof(BAVC_AudioBufferDescriptor));

   /* skip the first line of the file (the header) ... */
   if (0 == hAudioEncoder->uiDescriptorCount)
      if (NULL == fgets(aLine, MAX_DESC_LINE_LENGTH, hAudioEncoder->hDesc))
         return false;

   /* read a line from the CSV file */
   if (NULL == fgets(aLine, MAX_DESC_LINE_LENGTH, hAudioEncoder->hDesc))
      return false;

   /* process the string to extract the fields... */
   if (11 != sscanf(aLine, "%u,%u,%u,%u,%u,%x,%u,%u,%u,%hu,%hd", &uiChannel, &uiDescriptorCount, &uiPacing,
            &pCommon->uiOffset, &pCommon->uiLength, &pCommon->uiFlags, &uiPTS, &uiOrigPTS,
            &uiESCR, &pCommon->uiTicksPerBit, &pCommon->iSHR))
      return false;

   /* adjust ESCR, and PTS to start from zero ... */
   if (0 == hAudioEncoder->uiDescriptorCount)
   {
      hAudioEncoder->uiInitialESCR = uiESCR;
      hAudioEncoder->uiInitialESCRScaled = uiESCR / 300;
   }
   pCommon->uiPTS = uiPTS - hAudioEncoder->uiInitialESCRScaled;
   pCommon->uiESCR = uiESCR - hAudioEncoder->uiInitialESCR;

   if (uiDescriptorCount != hAudioEncoder->uiDescriptorCount)
      return false;
   return true;
}

BERR_Code app_OpenAudioEncoder(AudioEncoderHandle *phAudioEncoder, const AudioEncoderSettings* pstSettings)
{
   AudioEncoderStubContext *pAudioContext = NULL;
   uint32_t uiContentIndex = 0;

   pAudioContext = (AudioEncoderStubContext*) BKNI_Malloc(sizeof(AudioEncoderStubContext));

   BKNI_Memset(pAudioContext, 0, sizeof(AudioEncoderStubContext));

   pAudioContext->stSettings = *pstSettings;
   BDBG_MSG(("Using Content[%d] - %s",
               uiContentIndex,
               astEncodedContent[uiContentIndex].szFriendlyName
               ));

   pAudioContext->uiCDBBufferSize = astEncodedContent[uiContentIndex].stCDB.uiSize;
   pAudioContext->pCDBBuffer = (uint8_t *) BMEM_AllocAligned(pAudioContext->stSettings.hMem,
                     pAudioContext->uiCDBBufferSize, 8, 0);

   BMEM_ConvertAddressToOffset(pAudioContext->stSettings.hMem, (void*) pAudioContext->pCDBBuffer, &pAudioContext->uiCDBBufferOffset);

   BKNI_Memset(pAudioContext->pCDBBuffer, 0, pAudioContext->uiCDBBufferSize);

   /* Load CDB into memory */
   pAudioContext->hCDB = fopen(astEncodedContent[uiContentIndex].stCDB.szFilename, "rb");
   if ( NULL == pAudioContext->hCDB )
   {
      BDBG_ERR(("Error opening CDB file"));
      return BERR_TRACE(BERR_UNKNOWN);
   }

   pAudioContext->uiCDBBufferWriteOffset = fread(pAudioContext->pCDBBuffer, 1, pAudioContext->uiCDBBufferSize, pAudioContext->hCDB);

   pAudioContext->hDesc = fopen(astEncodedContent[uiContentIndex].stDesc.szFilename, "rb");
   if ( NULL == pAudioContext->hDesc )
   {
      BDBG_ERR(("Error opening Descriptors file"));
      return BERR_TRACE(BERR_UNKNOWN);
   }
   /* read descriptors from file into memory ... */
   while (ReadDescriptor(pAudioContext) && (pAudioContext->uiDescriptorCount < MAX_AUDIO_DESCRIPTORS))
      pAudioContext->uiDescriptorCount++;
   BDBG_MSG(("%d Audio Descriptors Read", pAudioContext->uiDescriptorCount));
#if 1
   {
      uint32_t i;
      for (i = 0; i < 5/*pAudioContext->uiDescriptorCount*/; i++)
      {
         BAVC_AudioBufferDescriptor *pDesc = &pAudioContext->stDescriptors[i];
         BDBG_MSG(("Descriptor [%d]: Offset: %d, Length: %d, Flags: %x, PTS: %lld, oPTS: %d, ESCR: %d, TPB: %d, iSHR: %d", i,
            pDesc->stCommon.uiOffset, pDesc->stCommon.uiLength, pDesc->stCommon.uiFlags, pDesc->stCommon.uiPTS,
            pDesc->stCommon.uiOriginalPTS, pDesc->stCommon.uiESCR, pDesc->stCommon.uiTicksPerBit, pDesc->stCommon.iSHR));
      }
   }
#endif

   pAudioContext->eProtocol = astEncodedContent[uiContentIndex].eProtocol;

   *phAudioEncoder = pAudioContext;
   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code app_CloseAudioEncoder(AudioEncoderHandle hAudioEncoder)
{
   fclose(hAudioEncoder->hCDB);
   fclose(hAudioEncoder->hDesc);

   BMEM_Free(hAudioEncoder->stSettings.hMem, hAudioEncoder->pCDBBuffer);

   BKNI_Free(hAudioEncoder);

   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code app_StopAudioEncoder(AudioEncoderHandle hAudioEncoder)
{
   hAudioEncoder->bStop = true;
   return BERR_TRACE(BERR_SUCCESS);
}

BERR_Code app_AudioEncoderIncrementTime(AudioEncoderHandle hAudioEncoder, unsigned uiMilliseconds)
{
   hAudioEncoder->uiCurrentESCR += 27000000 / 1000 * uiMilliseconds;

   return BERR_TRACE(BERR_SUCCESS);
}