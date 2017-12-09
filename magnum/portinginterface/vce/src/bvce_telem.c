/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
/* base modules */
#include "bstd.h"           /* standard types */
#include "bkni.h"           /* kernel interface */

#include "bvce_telem.h"

BDBG_MODULE(BVCE_TELEM);
BDBG_FILE_MODULE(BVCE_TELEM_STATE);

void
BVCE_Telem_GetDefaultCreateSettings(
         BVCE_Telem_Settings *pstTelemSettings
         )
{
   BDBG_ENTER( BVCE_Telem_GetDefaultCreateSettings );

   BDBG_ASSERT( pstTelemSettings );

   if ( NULL != pstTelemSettings )
   {
      BKNI_Memset( pstTelemSettings, 0, sizeof( BVCE_Telem_Settings ) );
   }

   BDBG_LEAVE( BVCE_Telem_GetDefaultCreateSettings );
}

typedef enum BVCE_Telem_P_ParserState
{
   BVCE_Telem_P_ParserState_eFileHeader,
   BVCE_Telem_P_ParserState_eLostSync,
   BVCE_Telem_P_ParserState_eFindSync,
   BVCE_Telem_P_ParserState_ePacketHeader,
   BVCE_Telem_P_ParserState_eProcessTimestamp,
   BVCE_Telem_P_ParserState_eComputePayloadSize,
   BVCE_Telem_P_ParserState_eProcessPayload,
   BVCE_Telem_P_ParserState_eDone
} BVCE_Telem_P_ParserState;

#if BDBG_DEBUG_BUILD
static const char* const BVCE_Telem_P_ParserStateLUT[BVCE_Telem_P_ParserState_eDone + 1] =
{
   "eFileHeader",
   "eLostSync",
   "eFindSync",
   "ePacketHeader",
   "eProcessTimestamp",
   "eComputePayloadSize",
   "eProcessPayload",
   "eDone"
};
#endif

static const char BVCE_TELEM_P_LostSyncMessage[] =
{
   '\n','\n','<','<','L','O','S','T',' ','S','Y','N','C','>','>','\n','\n'
};

typedef enum BVCE_Telem_P_FileHeaderState
{
   BVCE_Telem_P_FileHeaderState_eHeaderID,
   BVCE_Telem_P_FileHeaderState_eHeaderLength,
   BVCE_Telem_P_FileHeaderState_eTelemVersion,
   BVCE_Telem_P_FileHeaderState_eProductVersion,
   BVCE_Telem_P_FileHeaderState_eCPUSpeed,
   BVCE_Telem_P_FileHeaderState_eDone
} BVCE_Telem_P_FileHeaderState;

typedef enum BVCE_Telem_P_PacketHeaderState
{
   BVCE_Telem_P_PacketHeaderState_eStream,
   BVCE_Telem_P_PacketHeaderState_eGroup,
   BVCE_Telem_P_PacketHeaderState_eFlags,
   BVCE_Telem_P_PacketHeaderState_eSyncByte,
   BVCE_Telem_P_PacketHeaderState_eDone
} BVCE_Telem_P_PacketHeaderState;

typedef struct BVCE_Telem_P_Context
{
   BVCE_Telem_P_ParserState eParserState;
   unsigned uiBytesRead;
   unsigned uiBytesWritten;
   unsigned uiBytesInPayload;
   unsigned uiPaddingInPayload;

   struct
   {
      BVCE_Telem_P_FileHeaderState eState;
      unsigned uiBytesRead;
      uint32_t uiHeaderID;
      uint32_t uiHeaderLength;
      uint32_t uiTelemetryVersion;
      uint32_t uiProductVersion;
      uint32_t uiCPUSpeed;
   } stFileHeader;

   struct
   {
      BVCE_Telem_P_PacketHeaderState eState;

      uint8_t uiSyncByte;
      uint8_t uiFlags;
      uint8_t uiGroup;
      uint8_t uiStreamID;
   } stPacketHeader;
} BVCE_Telem_P_Context;

BERR_Code
BVCE_Telem_Create(
         BVCE_Telem_Handle *phVCETelem,
         const BVCE_Telem_Settings *pstTelemSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BVCE_Telem_Handle hVCETelem;

   BDBG_ENTER( BVCE_Telem_Create );

   BDBG_ASSERT( phVCETelem );
   BSTD_UNUSED( pstTelemSettings );

   if ( NULL == phVCETelem )
   {
      rc = BERR_TRACE( BERR_INVALID_PARAMETER );
   }
   else
   {
      *phVCETelem = NULL;

      hVCETelem = ( BVCE_Telem_Handle ) BKNI_Malloc( sizeof( BVCE_Telem_P_Context ) );

      if ( NULL == hVCETelem )
      {
         rc = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }
      else
      {
         BKNI_Memset( hVCETelem , 0, sizeof( BVCE_Telem_P_Context ) );

         *phVCETelem = hVCETelem;
      }
   }

   BDBG_LEAVE( BVCE_Telem_Create );

   return BERR_TRACE( rc );
}

BERR_Code
BVCE_Telem_Destroy(
         BVCE_Telem_Handle hVCETelem
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BVCE_Telem_Destroy );

   BDBG_ASSERT( hVCETelem );

   if ( NULL == hVCETelem )
   {
      rc = BERR_TRACE( BERR_INVALID_PARAMETER );
   }
   else
   {
      BKNI_Free( hVCETelem );
   }

   BDBG_LEAVE( BVCE_Telem_Destroy );

   return BERR_TRACE( rc );
}

#define SWAP32(x) ((x) = (((x) & 0xFF000000) >> 24 ) | (((x) & 0x00FF0000) >> 8 ) | (((x) & 0x0000FF00) << 8 ) | (((x) & 0x000000FF) << 24 ))
#define BVCE_TELEM_PACKET_HEADER_SYNC 0xBC
#define BVCE_TELEM_PACKET_HEADER_FLAGS_PAYLOAD_PRESENT 0x08
#define BVCE_TELEM_PACKET_HEADER_FLAGS_PAYLOAD_VARIABLE_LENGTH 0x40
#define BVCE_TELEM_PACKET_HEADER_STREAM_CONSOLE 0x02

BERR_Code
BVCE_Telem_Parse(
   BVCE_Telem_Handle hVCETelem,
   const uint8_t *pInputBuffer0, /* [in] */
   unsigned uiInputLength0, /* [in] */
   const uint8_t *pInputBuffer1, /* [in] */
   unsigned uiInputLength1, /* [in] */
   unsigned *puiInputLengthRead, /* out */
   char *szOutputBuffer,   /* [out] pointer to buffer where log is copied to */
   unsigned uiOutputLength,  /* [in] maximum number of bytes to copy to buffer */
   unsigned *puiOutputLengthWritten  /* [out] number of bytes copied from debug log */
   )
{
   BERR_Code rc = BERR_SUCCESS;
   unsigned uiTotalBytesToRead = uiInputLength0 + uiInputLength1;
   BDBG_ENTER( BVCE_Telem_Parse );

   BDBG_ASSERT( hVCETelem );
   BDBG_ASSERT( pInputBuffer0 );
   BDBG_ASSERT( puiInputLengthRead );
   BDBG_ASSERT( szOutputBuffer );
   BDBG_ASSERT( puiOutputLengthWritten );

   *puiInputLengthRead = 0;
   *puiOutputLengthWritten = 0;

   while ( ( *puiOutputLengthWritten < uiOutputLength )
           && ( *puiInputLengthRead < uiTotalBytesToRead )
           && ( BERR_SUCCESS == rc ) )
   {
      const uint8_t *puiByte;
      BVCE_Telem_P_ParserState eParserState = hVCETelem->eParserState;

      if ( *puiInputLengthRead < uiInputLength0 )
      {
         puiByte = pInputBuffer0 + *puiInputLengthRead;
      }
      else
      {
         puiByte = pInputBuffer1 + (*puiInputLengthRead - uiInputLength0);
      }

      switch ( hVCETelem->eParserState )
      {
         case BVCE_Telem_P_ParserState_eFileHeader:
            switch ( hVCETelem->stFileHeader.eState )
            {
               case BVCE_Telem_P_FileHeaderState_eHeaderID:
                  hVCETelem->stFileHeader.uiHeaderID <<= 8;
                  hVCETelem->stFileHeader.uiHeaderID |= *puiByte;
                  hVCETelem->uiBytesRead++;
                  (*puiInputLengthRead)++;

                  if ( 4 == hVCETelem->uiBytesRead )
                  {
                     SWAP32(hVCETelem->stFileHeader.uiHeaderID);
                     hVCETelem->uiBytesRead = 0;
                     hVCETelem->stFileHeader.eState++;
                  }
                  break;

               case BVCE_Telem_P_FileHeaderState_eHeaderLength:
                  hVCETelem->stFileHeader.uiHeaderLength <<= 8;
                  hVCETelem->stFileHeader.uiHeaderLength |= *puiByte;
                  hVCETelem->uiBytesRead++;
                  (*puiInputLengthRead)++;

                  if ( 4 == hVCETelem->uiBytesRead )
                  {
                     SWAP32(hVCETelem->stFileHeader.uiHeaderLength);
                     hVCETelem->uiBytesRead = 0;
                     hVCETelem->stFileHeader.eState++;
                  }
                  break;

               case BVCE_Telem_P_FileHeaderState_eTelemVersion:
                  hVCETelem->stFileHeader.uiTelemetryVersion <<= 8;
                  hVCETelem->stFileHeader.uiTelemetryVersion |= *puiByte;
                  hVCETelem->stFileHeader.uiBytesRead++;
                  hVCETelem->uiBytesRead++;
                  (*puiInputLengthRead)++;

                  if ( 4 == hVCETelem->uiBytesRead )
                  {
                     SWAP32(hVCETelem->stFileHeader.uiTelemetryVersion);
                     hVCETelem->uiBytesRead = 0;
                     hVCETelem->stFileHeader.eState++;
                  }
                  break;

               case BVCE_Telem_P_FileHeaderState_eProductVersion:
                  hVCETelem->stFileHeader.uiProductVersion <<= 8;
                  hVCETelem->stFileHeader.uiProductVersion |= *puiByte;
                  hVCETelem->stFileHeader.uiBytesRead++;
                  hVCETelem->uiBytesRead++;
                  (*puiInputLengthRead)++;

                  if ( 4 == hVCETelem->uiBytesRead )
                  {
                     SWAP32(hVCETelem->stFileHeader.uiProductVersion);
                     hVCETelem->uiBytesRead = 0;
                     hVCETelem->stFileHeader.eState++;
                  }
                  break;

               case BVCE_Telem_P_FileHeaderState_eCPUSpeed:
                  hVCETelem->stFileHeader.uiCPUSpeed <<= 8;
                  hVCETelem->stFileHeader.uiCPUSpeed |= *puiByte;
                  hVCETelem->stFileHeader.uiBytesRead++;
                  hVCETelem->uiBytesRead++;
                  (*puiInputLengthRead)++;

                  if ( 4 == hVCETelem->uiBytesRead )
                  {
                     SWAP32(hVCETelem->stFileHeader.uiCPUSpeed);
                     hVCETelem->uiBytesRead = 0;
                     hVCETelem->stFileHeader.eState++;
                  }
                  break;

               case BVCE_Telem_P_FileHeaderState_eDone:
                  if ( hVCETelem->stFileHeader.uiBytesRead < hVCETelem->stFileHeader.uiHeaderLength )
                  {
                     /* Skip Unknown Header Info */
                     hVCETelem->stFileHeader.uiBytesRead++;
                     (*puiInputLengthRead)++;
                  }
                  else
                  {
                     BDBG_MSG(("Header ID:%08x, Telemetry Version:%08x, Product Version:%08x, CPU Speed:%08x",
                        hVCETelem->stFileHeader.uiHeaderID,
                        hVCETelem->stFileHeader.uiTelemetryVersion,
                        hVCETelem->stFileHeader.uiProductVersion,
                        hVCETelem->stFileHeader.uiCPUSpeed
                        ));
                     hVCETelem->stFileHeader.uiHeaderLength = 0;
                     hVCETelem->uiBytesRead = 0;
                     hVCETelem->stFileHeader.eState = 0;
                     hVCETelem->eParserState = BVCE_Telem_P_ParserState_eFindSync;
                  }
                  break;
            }
            break;

         case BVCE_Telem_P_ParserState_eLostSync:
            if ( hVCETelem->uiBytesWritten < sizeof(BVCE_TELEM_P_LostSyncMessage)/sizeof(char) )
            {
               szOutputBuffer[*puiOutputLengthWritten] = BVCE_TELEM_P_LostSyncMessage[hVCETelem->uiBytesWritten];
               (*puiOutputLengthWritten)++;
               hVCETelem->uiBytesWritten++;
            }
            else
            {
               hVCETelem->uiBytesWritten = 0;
               hVCETelem->eParserState = BVCE_Telem_P_ParserState_eFindSync;
            }
            break;

         case BVCE_Telem_P_ParserState_eFindSync:
            if ( ( uiTotalBytesToRead - *puiInputLengthRead ) >= 4 )
            {
               if ( BVCE_TELEM_PACKET_HEADER_SYNC != puiByte[3] )
               {
                  (*puiInputLengthRead)++;
               }
               else
               {
                  hVCETelem->eParserState = BVCE_Telem_P_ParserState_ePacketHeader;
               }
            }
            else
            {
               BDBG_WRN(("Finding Sync..."));
               rc = BERR_UNKNOWN;
            }
            break;

         case BVCE_Telem_P_ParserState_ePacketHeader:
            switch ( hVCETelem->stPacketHeader.eState )
            {
               case BVCE_Telem_P_PacketHeaderState_eSyncByte:
                  hVCETelem->stPacketHeader.uiSyncByte = *puiByte;
                  if ( BVCE_TELEM_PACKET_HEADER_SYNC != hVCETelem->stPacketHeader.uiSyncByte )
                  {
                     BDBG_ERR(("Lost Sync!"));
                     hVCETelem->stPacketHeader.eState = 0;
                     hVCETelem->eParserState = BVCE_Telem_P_ParserState_eLostSync;
                  }
                  else
                  {
                     (*puiInputLengthRead)++;
                     hVCETelem->stPacketHeader.eState++;
                  }
                  break;

               case BVCE_Telem_P_PacketHeaderState_eFlags:
                  hVCETelem->stPacketHeader.uiFlags = *puiByte;
                  (*puiInputLengthRead)++;
                  hVCETelem->stPacketHeader.eState++;
                  break;

               case BVCE_Telem_P_PacketHeaderState_eGroup:
                  hVCETelem->stPacketHeader.uiGroup = *puiByte;
                  (*puiInputLengthRead)++;
                  hVCETelem->stPacketHeader.eState++;
                  break;

               case BVCE_Telem_P_PacketHeaderState_eStream:
                  hVCETelem->stPacketHeader.uiStreamID = *puiByte;
                  (*puiInputLengthRead)++;
                  hVCETelem->stPacketHeader.eState++;
                  break;

               case BVCE_Telem_P_PacketHeaderState_eDone:
                  hVCETelem->stPacketHeader.eState = 0;
                  hVCETelem->eParserState = BVCE_Telem_P_ParserState_eProcessTimestamp;
                  break;
            }
            break;

         case BVCE_Telem_P_ParserState_eProcessTimestamp:
            /* Skip Time Stamp */
            hVCETelem->uiBytesRead++;
            (*puiInputLengthRead)++;

            if ( 8 == hVCETelem->uiBytesRead )
            {
               hVCETelem->uiBytesRead = 0;
               hVCETelem->eParserState = BVCE_Telem_P_ParserState_eComputePayloadSize;
            }
           break;

         case BVCE_Telem_P_ParserState_eComputePayloadSize:
            if ( 0 != ( hVCETelem->stPacketHeader.uiFlags & BVCE_TELEM_PACKET_HEADER_FLAGS_PAYLOAD_PRESENT ) )
            {
               if ( 0 != ( hVCETelem->stPacketHeader.uiFlags & BVCE_TELEM_PACKET_HEADER_FLAGS_PAYLOAD_VARIABLE_LENGTH ) )
               {
                  /* Variable Length Payload */
                  hVCETelem->uiBytesInPayload <<= 8;
                  hVCETelem->uiBytesInPayload |= *puiByte;
                  hVCETelem->uiBytesRead++;
                  (*puiInputLengthRead)++;

                  if ( 4 == hVCETelem->uiBytesRead )
                  {
                     SWAP32(hVCETelem->uiBytesInPayload);
                     hVCETelem->uiPaddingInPayload = 4 - (hVCETelem->uiBytesInPayload % 4);
                     /* SW7425-4739: VCE FW v4.0.10.0+ has reduced the padding requirements for variable length telemetry payloads */
                     if ( 4 == hVCETelem->uiPaddingInPayload ) hVCETelem->uiPaddingInPayload = 0;
                     hVCETelem->uiBytesRead = 0;
                     hVCETelem->eParserState = BVCE_Telem_P_ParserState_eProcessPayload;
                  }
               }
               else
               {
                  /* Standard Payload */
                  hVCETelem->uiBytesInPayload = 4;
                  hVCETelem->uiPaddingInPayload = 0;
                  hVCETelem->eParserState = BVCE_Telem_P_ParserState_eProcessPayload;
               }
            }
            else
            {
               /* No Payload */
               hVCETelem->uiBytesInPayload = 0;
               hVCETelem->uiPaddingInPayload = 0;
               hVCETelem->eParserState = BVCE_Telem_P_ParserState_eDone;
            }
            break;

         case BVCE_Telem_P_ParserState_eProcessPayload:
            if ( hVCETelem->uiBytesRead < ( hVCETelem->uiBytesInPayload + hVCETelem->uiPaddingInPayload ) )
            {
               if ( hVCETelem->uiBytesRead < hVCETelem->uiBytesInPayload )
               {
                  if ( BVCE_TELEM_PACKET_HEADER_STREAM_CONSOLE == hVCETelem->stPacketHeader.uiStreamID )
                  {
                     szOutputBuffer[*puiOutputLengthWritten] = *puiByte;
                     (*puiOutputLengthWritten)++;
                  }
               }

               hVCETelem->uiBytesRead++;
               (*puiInputLengthRead)++;
            }
            else
            {
               hVCETelem->uiBytesRead = 0;
               hVCETelem->eParserState = BVCE_Telem_P_ParserState_eDone;
            }
            break;

         case BVCE_Telem_P_ParserState_eDone:
            hVCETelem->uiBytesRead = 0;
            hVCETelem->eParserState = BVCE_Telem_P_ParserState_ePacketHeader;
            break;
      }

      if ( hVCETelem->eParserState != eParserState )
      {
         BDBG_MODULE_MSG( BVCE_TELEM_STATE, ("eParserState (%s(%d) --> %s(%d))",
            BVCE_Telem_P_ParserStateLUT[eParserState],
            eParserState,
            BVCE_Telem_P_ParserStateLUT[hVCETelem->eParserState],
            hVCETelem->eParserState ));
      }
   }

   BDBG_LEAVE( BVCE_Telem_Parse );

   return BERR_TRACE( rc );
}
