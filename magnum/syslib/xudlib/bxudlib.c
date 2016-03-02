/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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

/**** Module Overview ****
 *
 * The XUD (Transcode User Data) API is a library used to rate convert user data from an input source 
 * so as to allow it to be inserted into the transcoded video.
 *
 */
#include "bstd.h"
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "bavc.h"
#include "budp.h"
#include "budp_dccparse.h"
#include "budp_dccparse_dss.h"
#include "budp_vce.h"
#include "bxudlib.h"

#if ( BXUDLIB_P_DUMP_INPUT_CC || BXUDLIB_P_DUMP_OUTPUT_CC || BXUDLIB_P_TEST_MODE )
#include <stdio.h>
#endif

#define BXUDLIB_P_DEBUG_CC_PACKET 0

BDBG_MODULE(BXUDlib);

#define B_MAX_VBI_CC_COUNT 32 /* required by UDPlib */
#define B_MAX_708_CC_COUNT 9 /* required by ViCE */
#define B_MAX_608_CC_COUNT 2 
#define BXUD_OUT_OF_SYNC_THRESHOLD 3

#define BXUD_DEFAULT_QUEUE_SIZE (B_MAX_VBI_CC_COUNT * BXUD_OUT_OF_SYNC_THRESHOLD)

typedef struct BXUD_EncoderPacketDescriptor
{
    BUDP_DCCparse_Format format;
    uint8_t ccCount;
    BUDP_DCCparse_ccdata ccData[B_MAX_VBI_CC_COUNT];
}BXUD_EncoderPacketDescriptor;

typedef enum BXUD_EncapsulationStd {
    BXUD_EncapsulationStd_Scte20,
    BXUD_EncapsulationStd_Scte21,
    BXUD_EncapsulationStd_A53,

    BXUD_EncapsulationStd_Max
}BXUD_EncapsulationStd;

typedef struct BXUD_CCData
{
   unsigned uiDecodePictureId;
   BUDP_DCCparse_ccdata ccData;
} BXUD_CCData;

typedef struct BXUD_StdInfo 
{
    bool synced;
    struct
    {
       unsigned uiWriteOffset;
       unsigned uiReadOffset;
       BXUD_CCData *astCCData;
    } type[2];
}BXUD_StdInfo;

typedef struct BXUDlib_P_Context
{
    BXUDlib_CreateSettings createSettings;
    BXUDlib_Settings settings;

    bool stgReceived;
    bool initialSync;
    bool bCurrentPolarityValid;
    BAVC_Polarity currentPolarity;
    uint32_t decodeId; 
    BUDP_DCCparse_Format currentFormat;

    BXUD_StdInfo stdInfo[BXUD_EncapsulationStd_Max];

    uint8_t auiFieldInfo[(sizeof(BUDP_Encoder_FieldInfo) + ((3 * sizeof(BUDP_Encoder_PacketDescriptor)) - sizeof(BUDP_Encoder_PacketDescriptor)) )];

#if BXUDLIB_P_DEBUG_CC_PACKET
    char outputStr[2][2049];
#endif
    unsigned repCnt;
    uint8_t numPacketDescriptors;
    BXUD_EncoderPacketDescriptor encoderPacketDescriptor[3];

#if BXUDLIB_P_DUMP_INPUT_CC
    FILE *hInputCCDump;
    bool bInputCCDumpHeaderPrinted;
#endif

#if BXUDLIB_P_TEST_MODE
    FILE *hInputCCDumpRAW;
    bool bInputCCDumpRAWHeaderPrinted;
#endif

#if BXUDLIB_P_DUMP_OUTPUT_CC
    FILE *hOutputCCDump;
#endif

    unsigned uiOutputQueueEntryCount;

    BVDC_Display_CallbackData stPreviousDisplayCallbackData;
    bool bPreviousDisplayCallbackDataValid;

    struct {
        BUDP_DCCparse_dss_cc_subtitle dssCcData[B_MAX_VBI_CC_COUNT];
        BUDP_DCCparse_dss_cc_subtitle dssSubData[B_MAX_VBI_CC_COUNT];
        BUDP_DCCparse_ccdata ccData[B_MAX_VBI_CC_COUNT];
    } udpData;
}BXUDlib_P_Context;

BERR_Code
BXUDlib_Create(BXUDlib_Handle *phXud, const BXUDlib_CreateSettings *pstXudCreateSettings)
{
    BXUDlib_P_Context *pContext = NULL;
    unsigned int i,j;

    pContext = (BXUDlib_P_Context *)BKNI_Malloc(sizeof(BXUDlib_P_Context));
    if( !pContext )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    
    BKNI_Memset(pContext, 0, sizeof(BXUDlib_P_Context)); 

    if( pstXudCreateSettings )
    {
        pContext->createSettings = *pstXudCreateSettings;
    }
    else
    {
        BXUDlib_GetDefaultCreateSettings(&pContext->createSettings);
    }

    /* default output format */
    pContext->stgReceived = false;
    pContext->initialSync = true;
    pContext->currentPolarity = BAVC_Polarity_eBotField; /* it will be toggled to top field in the STG callback */
    pContext->decodeId = 0;
    pContext->currentFormat = BUDP_DCCparse_Format_ATSC53;

    for( i = 0; i < BXUD_EncapsulationStd_Max; i++ )
    {
       BKNI_Memset( &pContext->stdInfo[i], 0, sizeof( pContext->stdInfo[i] ) );
       for ( j = 0; j < 2; j++ )
       {
          pContext->stdInfo[i].type[j].astCCData = (BXUD_CCData *)BKNI_Malloc(sizeof( BXUD_CCData ) * pContext->createSettings.queueSize );

          if ( !pContext->stdInfo[i].type[j].astCCData )
          {
             BXUDlib_Destroy(pContext);
             return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
          }

          BKNI_Memset( pContext->stdInfo[i].type[j].astCCData, 0, sizeof( BXUD_CCData ) * pContext->createSettings.queueSize );
       }
    }
    
#if BXUDLIB_P_DUMP_INPUT_CC
    pContext->hInputCCDump = (FILE*) fopen("BXUDLIB_USERDATA_INPUT.csv", "wb");
    if ( NULL == pContext->hInputCCDump )
    {
       BDBG_ERR(("Error opening input CC dump file"));
    }
#endif

#if BXUDLIB_P_TEST_MODE
    pContext->hInputCCDumpRAW = (FILE*) fopen("BXUDLIB_USERDATA_INPUT_RAW.csv", "wb");
    if ( NULL == pContext->hInputCCDumpRAW )
    {
       BDBG_ERR(("Error opening input CC RAW dump file"));
    }
#endif

#if BXUDLIB_P_DUMP_OUTPUT_CC
    pContext->hOutputCCDump = (FILE*) fopen("BXUDLIB_USERDATA_OUTPUT.csv", "wb");
    if ( NULL == pContext->hOutputCCDump )
    {
       BDBG_ERR(("Error opening output CC dump file"));
    }

    fprintf(pContext->hOutputCCDump, "stg_pic_id,stg_decode_pic_id,stg_polarity,queue_entry,descriptor_index,decode_pic_id,format,analog,polarity,valid,line_offset,type,data_0,data_1,notes,608_read,608_write,708_read,708_write\n");
#endif

    *phXud = (BXUDlib_Handle)pContext;
    return BERR_SUCCESS;
}


void
BXUDlib_Destroy(BXUDlib_Handle hXud)
{
    unsigned int i,j;
    BXUDlib_P_Context *pContext = (BXUDlib_P_Context *)hXud;

#if BXUDLIB_P_DUMP_OUTPUT_CC
    if ( NULL != pContext->hOutputCCDump )
    {
       fclose( pContext->hOutputCCDump );
       pContext->hOutputCCDump = NULL;
    }
#endif

#if BXUDLIB_P_TEST_MODE
    if ( NULL != pContext->hInputCCDumpRAW )
    {
       fclose( pContext->hInputCCDumpRAW );
       pContext->hInputCCDumpRAW = NULL;
    }
#endif

#if BXUDLIB_P_DUMP_INPUT_CC
    if ( NULL != pContext->hInputCCDump )
    {
       fclose( pContext->hInputCCDump );
       pContext->hInputCCDump = NULL;
    }
#endif

    for( i = 0; i < BXUD_EncapsulationStd_Max; i++ )
    {
       for ( j = 0; j < 2; j++)
       {
          if ( NULL != pContext->stdInfo[i].type[j].astCCData )
          {
             BKNI_Free( pContext->stdInfo[i].type[j].astCCData );
             pContext->stdInfo[i].type[j].astCCData = NULL;
          }
       }
    }

    BKNI_Free(pContext);
}

void
BXUDlib_GetDefaultCreateSettings(BXUDlib_CreateSettings *pstXudCreateSettings)
{
    BKNI_Memset(pstXudCreateSettings, 0, sizeof(BXUDlib_CreateSettings)); 

    /* default queue size */
    pstXudCreateSettings->queueSize = BXUD_DEFAULT_QUEUE_SIZE;
}


/* BXUDlib_SetSettings is called to change the config settings. When the outputPacketType 
   is changed, the library will flush its queue of cc packets and start afresh.
   The library will discard all incoming user data if the sink interface is not set.
   If the sink interface is set, the library will perform the appropriate rate 
   conversion and feed the user data to the sink interface at the STG rate

   BXUDlib_GetSettings can be called prior to SetSettings to get the default config settings.
*/
BERR_Code
BXUDlib_SetSettings(BXUDlib_Handle hXud, const BXUDlib_Settings *pstXudSettings)
{
    BXUDlib_P_Context *pContext = (BXUDlib_P_Context *)hXud;

    pContext->settings = *pstXudSettings;

    /* TODO: Have to flush the queue and reestablish sync */
    return BERR_SUCCESS;
}

void
BXUDlib_GetSettings(BXUDlib_Handle hXud, BXUDlib_Settings *pstXudSettings)
{
    BXUDlib_P_Context *pContext = (BXUDlib_P_Context *)hXud;

    *pstXudSettings = pContext->settings;
}

#if BXUDLIB_P_DEBUG_CC_PACKET
static void printCcPacket_isr(BXUDlib_P_Context *pContext, char in, uint32_t stgId, uint32_t decodeId, uint8_t ccCount, BUDP_DCCparse_ccdata *ccData, uint32_t valid)
{
    unsigned int i;
    unsigned int index = 0;

    if( in == 'I' )
    {
        BKNI_Snprintf(pContext->outputStr[index], 2048, "I:%d:%d:", decodeId, ccCount);
    }
    else if( in == 'O' )
    {
        BKNI_Snprintf(pContext->outputStr[index], 2048, "O:%d:%d:%d:", stgId, decodeId, ccCount);
    }
    else
    {
        BKNI_Snprintf(pContext->outputStr[index], 2048, "%c:%d:%d:%d:", in, stgId, decodeId, ccCount);
    }
    
    for( i = 0; i < ccCount; i++ )
    {

        if( valid & 0x01 )
        {
            index = 1 - index;
            BKNI_Snprintf(pContext->outputStr[index], 2048, "%s[%d:%d:%d:%d:%d:%d-%d:%d]",
                pContext->outputStr[1-index],
                ccData[i].bIsAnalog, ccData[i].polarity, 
                ccData[i].format, ccData[i].cc_valid,
                ccData[i].line_offset, ccData[i].seq.cc_type, 
                ccData[i].cc_data_1, ccData[i].cc_data_2);
        }
        
        valid >>= 1;
    }

    BDBG_MSG((pContext->outputStr[index]));
}

void
printUserData_isr(BXUDlib_P_Context *pContext, unsigned int decodeId, unsigned char *userData, unsigned int size )
{
    unsigned int i;
    unsigned int index = 0;

    BDBG_MSG(("-------------Decode id = %d-------------", decodeId));
    pContext->outputStr[index][0] = 0;
    for( i = 0; i < size; i++ )
    {
        index = 1 - index;
        BKNI_Snprintf(pContext->outputStr[index], 256, "%s%02X ", pContext->outputStr[1 - index],(unsigned int)userData[i]);

        if(!((i+1) % 16))
        {
           BDBG_MSG((pContext->outputStr[index]));
           pContext->outputStr[index][0] = 0;
        }
    }

    BDBG_MSG((pContext->outputStr[index]));
    BDBG_MSG(("------------------------------------------"));
}
#endif

static const uint8_t BXUDlib_P_SWAP8[256] =
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
    R6(0), R6(2), R6(1), R6(3)
};


/* BXUDlib_UserDataHandler_isr is called to provide XUD with user data packets (only closed 
   captioning is used for now). All packets are associated with a "decode" picture id. XUD copies and queues the data 
   provided in this callback for later processing.   
*/
BERR_Code
BXUDlib_UserDataHandler_isr(BXUDlib_Handle hXud, const BAVC_USERDATA_info  *pstUserData)
{
    BXUDlib_P_Context *pContext = (BXUDlib_P_Context *)hXud;
    uint32_t offset = 0;

    /*printUserData_isr(pContext, pstUserData->ulDecodePictureId, pstUserData->pUserDataBuffer, pstUserData->ui32UserDataBufSize); */
    while (offset < pstUserData->ui32UserDataBufSize) 
    {
        BERR_Code rc;
        size_t bytesParsed = 0;
        uint8_t ccCount = 0;
        BUDP_DCCparse_ccdata *ccData = hXud->udpData.ccData;

        /* Try MPEG2 DCC Parser First */
        rc = BUDP_DCCparse_isr(pstUserData, offset, &bytesParsed, &ccCount, ccData);
        /* Try AVC DCC Parser if the MPEG2 DCC Parser didn't find anything */
        if ( 0 == ccCount ) rc = BUDP_DCCparse_SEI_isr(pstUserData, offset, &bytesParsed, &ccCount, ccData);
#if B_REFSW_DSS_SUPPORT
        /* Try DSS DCC Parser last */
        if ( 0 == ccCount ) {
            uint8_t subCount = 0;
            BUDP_DCCparse_dss_cc_subtitle *dssCcData = hXud->udpData.dssCcData;
            BUDP_DCCparse_dss_cc_subtitle *dssSubData = hXud->udpData.dssSubData;

            rc = BUDP_DCCparse_DSS_isr(pstUserData, offset, &bytesParsed, &ccCount, dssCcData, &subCount, dssSubData);
            if (rc == BERR_SUCCESS)
            {
                unsigned i;
                BUDP_DCCparse_dss_cc_subtitle * pCcData = NULL;

#if 0
                if (ccCount)
#endif
                {
                    pCcData = dssCcData;
                }
#if 0
                else if (subCount)
                {
                    ccCount = subCount;
                    pCcData = dssSubData;
                }
#endif
                /* copy into supported format */
                for (i = 0; i < ccCount; i++)
                {
                    ccData[i].bIsAnalog = pCcData[i].bIsAnalog;
                    ccData[i].polarity = pCcData[i].polarity;
                    ccData[i].format = BUDP_DCCparse_Format_ATSC53;
                    ccData[i].cc_valid = 1;
                    ccData[i].cc_priority = pCcData[i].cc_priority;
                    /* copied from budp_dccparse.c where it says "Kludge" */
                    ccData[i].line_offset = 11;
                    /*
                     * when dss signals top field, same as NTSC field 1 data
                     * when dss signals bottom field, same as NTSC field 2 data (XDS)
                     */
                    if ((pCcData[i].polarity == BAVC_Polarity_eTopField) || (pCcData[i].polarity == BAVC_Polarity_eFrame))
                    {
                        ccData[i].seq.cc_type = 0;
                    }
                    else
                    {
                        ccData[i].seq.cc_type = 1;
                    }
                    ccData[i].cc_data_1 = pCcData[i].cc_data_1;
                    ccData[i].cc_data_2 = pCcData[i].cc_data_2;
                    ccData[i].active_format = 0;
                }
            }
        }
#endif /* DSS_SUPPORT */
        if (bytesParsed==0) 
        { 
            /* We aren't going anywhere...*/
            break;
        }
        offset += bytesParsed;
        /* We process bytesParsed even with error code. seems a bit dangerous. */
        if (rc == BERR_BUDP_PARSE_ERROR) 
        {
            break;
        }
        else if (rc != BERR_SUCCESS) 
        {
            continue;
        }
        else if ( 0 == ccCount )
        {
           continue;
        }

        /* UDPlib takes pointer w/o size, so this must be true.
           Otherwise we have overflow, from which there is no recovery. */
        BDBG_ASSERT(ccCount <= B_MAX_VBI_CC_COUNT);

#if BXUDLIB_P_TEST_MODE
        {
           unsigned uiIndex = 0;

           for ( uiIndex = 0; uiIndex < ccCount; uiIndex++ )
           {

              if ( NULL != pContext->hInputCCDumpRAW )
              {
                 if ( false == pContext->bInputCCDumpRAWHeaderPrinted )
                 {
                    fprintf(pContext->hInputCCDumpRAW, "Polarity,Format,Analog,CC Valid,CC Priority,CC Line Offset,CC Type,CC Data[0],CC Data[1]\n");
                    pContext->bInputCCDumpRAWHeaderPrinted = true;
                 }

                 fprintf(pContext->hInputCCDumpRAW, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                    ccData[uiIndex].polarity,
                    ccData[uiIndex].format,
                    ccData[uiIndex].bIsAnalog,
                    ccData[uiIndex].cc_valid,
                    ccData[uiIndex].cc_priority,
                    ccData[uiIndex].line_offset,
                    ccData[uiIndex].seq.cc_type,
                    ccData[uiIndex].cc_data_1,
                    ccData[uiIndex].cc_data_2
                    );
              }
           }
        }
#endif

#if BXUDLIB_P_DEBUG_CC_PACKET
        printCcPacket_isr(pContext, 'I', 0, emptyPacket[0]->decodePictureId, ccCount[0], emptyPacket[0]->ccData, emptyPacket[0]->valid);
#endif

        {
           unsigned uiIndex = 0;
           
           for ( uiIndex = 0; uiIndex < ccCount; uiIndex++ )
           {
              BXUD_StdInfo *pstdInfo = NULL;

              switch( ccData[uiIndex].format ) {

                 case BUDP_DCCparse_Format_DVS157: /* scte20 */
                    /* Start: Workaround for incorrect endianess of cc bytes in dvs157/scte20 data from BUDP_DCCParse_isr */
                    {
                       ccData[uiIndex].cc_data_1 = BXUDlib_P_SWAP8[ccData[uiIndex].cc_data_1];
                       ccData[uiIndex].cc_data_2 = BXUDlib_P_SWAP8[ccData[uiIndex].cc_data_2];
                    }
                    /* End: Workaround */
                    pstdInfo = &pContext->stdInfo[BXUD_EncapsulationStd_Scte20];
                    break;

                 case BUDP_DCCparse_Format_DVS053: /* scte21 */
                    pstdInfo = &pContext->stdInfo[BXUD_EncapsulationStd_Scte21];
                    break;

                 case BUDP_DCCparse_Format_SEI:
                    /*
                     * SEI format looks exactly like ATSC format from a spec
                     * POV.  We have also verified that SEI format and ATSC
                     * format will never occur in the same stream together.
                     * Therefore it is safe to treat SEI data as ATSC53 data.
                     */
                    ccData[uiIndex].format = BUDP_DCCparse_Format_ATSC53;
                    /* fall through */
                 case BUDP_DCCparse_Format_ATSC53:
                    pstdInfo = &pContext->stdInfo[BXUD_EncapsulationStd_A53];
                    break;

                 default:
                    /* ignore the packet */
                    BDBG_MSG(("Unsupported CC format %d, ignored", ccData[uiIndex].format));
                    break;
              }

              if ( NULL != pstdInfo )
              {
                 unsigned uiType = ccData[uiIndex].bIsAnalog ? 0 : 1;

                 unsigned uiTempWriteOffset = (pstdInfo->type[uiType].uiWriteOffset + 1) % pContext->createSettings.queueSize;
                 pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiWriteOffset].uiDecodePictureId = pstUserData->ulDecodePictureId;
                 pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiWriteOffset].ccData = ccData[uiIndex];
                 pstdInfo->type[uiType].uiWriteOffset = uiTempWriteOffset;

                 /* If the queue is full, we discard the oldest data in the queue */
                 if ( uiTempWriteOffset == pstdInfo->type[uiType].uiReadOffset )
                 {
                    BDBG_MSG(("User Data Queue[%d] Overflow. Dropping older data", ccData[uiIndex].format));
                    pstdInfo->type[uiType].uiReadOffset = (pstdInfo->type[uiType].uiReadOffset + 1) % pContext->createSettings.queueSize;
                 }

#if BXUDLIB_P_DUMP_INPUT_CC
                 if ( NULL != pContext->hInputCCDump )
                 {
                    if ( false == pContext->bInputCCDumpHeaderPrinted )
                    {
                       fprintf(pContext->hInputCCDump, "decode_pic_id,dropped,format,analog,polarity,valid,line_offset,type,data_0,data_1\n");
                       pContext->bInputCCDumpHeaderPrinted = true;
                    }

                    fprintf(pContext->hInputCCDump, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                       pstUserData->ulDecodePictureId,
                       ( uiTempWriteOffset == pstdInfo->type[uiType].uiReadOffset ),
                       ccData[uiIndex].format,
                       ccData[uiIndex].bIsAnalog,
                       ccData[uiIndex].polarity,
                       ccData[uiIndex].cc_valid,
                       ccData[uiIndex].line_offset,
                       ccData[uiIndex].seq.cc_type,
                       ccData[uiIndex].cc_data_1,
                       ccData[uiIndex].cc_data_2
                       );
                 }
#endif
              }
           }
        }
    }
    
    return BERR_SUCCESS;
}

static void
BXUDlib_P_OutputUserData_isr( BXUDlib_P_Context *pContext, uint32_t stgPictureId )
{
    void *pPrivateSinkContext;
    BXUDlib_UserDataSink_Add userDataAdd_isr; 

    BUDP_Encoder_FieldInfo *pstFieldInfo = (BUDP_Encoder_FieldInfo *) pContext->auiFieldInfo;
    BUDP_Encoder_PacketDescriptor *pstPacketDescriptor = pstFieldInfo->stPacketDescriptor;

    unsigned queuedCount;
    uint8_t i;

    userDataAdd_isr = pContext->settings.sinkInterface.userDataAdd_isr;
    pPrivateSinkContext = pContext->settings.sinkInterface.pPrivateSinkContext;

    if( userDataAdd_isr )
    {
        pstFieldInfo->uiStgPictureId = stgPictureId;
        pstFieldInfo->ePolarity = pContext->currentPolarity;
        pstFieldInfo->uiNumDescriptors = pContext->numPacketDescriptors;

        if( pContext->numPacketDescriptors == 0)
        {
            BDBG_MSG(("O:%d:%d:EMPTY", stgPictureId, pContext->decodeId));
        }

        for( i = 0; i < pContext->numPacketDescriptors; i++ )
        {
            pstPacketDescriptor[i].ePacketFormat = pContext->encoderPacketDescriptor[i].format;
            switch( pstPacketDescriptor[i].ePacketFormat ) {

	        case BUDP_DCCparse_Format_DVS157:
	            pstPacketDescriptor[i].data.stDvs157.stCC.uiNumLines = pContext->encoderPacketDescriptor[i].ccCount;
                pstPacketDescriptor[i].data.stDvs157.stCC.astLine = pContext->encoderPacketDescriptor[i].ccData;
	        break;

	        case BUDP_DCCparse_Format_ATSC53:
	            pstPacketDescriptor[i].data.stAtsc53.stCC.uiNumLines = pContext->encoderPacketDescriptor[i].ccCount;
                pstPacketDescriptor[i].data.stAtsc53.stCC.astLine = pContext->encoderPacketDescriptor[i].ccData;
	        break;
	        
	        case BUDP_DCCparse_Format_DVS053:
	            pstPacketDescriptor[i].data.stDvs053.stCC.uiNumLines = pContext->encoderPacketDescriptor[i].ccCount;
                pstPacketDescriptor[i].data.stDvs053.stCC.astLine = pContext->encoderPacketDescriptor[i].ccData;
	        break;

	        default:
	        break;
            }
        }
        
        userDataAdd_isr( pPrivateSinkContext, pstFieldInfo, 1, &queuedCount );

        if ( 1 != queuedCount )
        {
           BDBG_MSG(("Output QUEUE is full"));
#if BXUDLIB_P_DUMP_OUTPUT_CC
             if ( NULL != pContext->hOutputCCDump )
             {
                fprintf(pContext->hOutputCCDump, "%d,%d,%d,-,-,-,-,-,-,-,-,-,-,-,OUTPUT QUEUE FULL\n",
                   stgPictureId,
                   pContext->decodeId,
                   pContext->currentPolarity
                   );
             }
#endif
        }
        pContext->uiOutputQueueEntryCount += queuedCount;
    }
}

bool
BXUDlib_P_DataPresent_isr( BXUDlib_P_Context *pContext, BXUD_StdInfo *pstdInfo, uint32_t decodePictureId )
{
   unsigned int uiType;

   for ( uiType = 0; uiType < 2; uiType++ )
   {
      unsigned uiTempReadOffset = pstdInfo->type[uiType].uiReadOffset;

      while ( uiTempReadOffset != pstdInfo->type[uiType].uiWriteOffset )
      {
         if ( pstdInfo->type[uiType].astCCData[uiTempReadOffset].uiDecodePictureId == decodePictureId )
         {
            return true;
         }

         uiTempReadOffset++;
         uiTempReadOffset %= pContext->createSettings.queueSize;
      }
   }

   return false;
}

#define BXUD_ID_DELTA(a, b) ((a > b) ? (a - b) : (b - a))

bool
BXUDlib_P_DiscardOutOfSyncData_isr( BXUDlib_P_Context *pContext, BXUD_StdInfo *pstdInfo, uint32_t decodePictureId, uint8_t threshold  )
{
   unsigned int uiType;
   bool bResult = false;

   for ( uiType = 0; uiType < 2; uiType++ )
   {
      while ( pstdInfo->type[uiType].uiReadOffset != pstdInfo->type[uiType].uiWriteOffset )
      {
         unsigned uiDelta = BXUD_ID_DELTA(pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].uiDecodePictureId, decodePictureId);

         if( uiDelta <= threshold )
         {
            /* the current packet is within threshold */
            bResult = true;
            break;
         }
         if ( true == pstdInfo->synced )
         {
            BDBG_WRN(("Discarding out-of-sync CC Data (%d>%d)", uiDelta,threshold));
            /* SW7445-3487: If we are already synced and there's out-of-sync CC data.
             *              This indicates a discontinuity, so should re-sync
             *              with the smallest threshold */
            threshold = 1;
            pstdInfo->synced = false;
         }
#if BXUDLIB_P_DUMP_OUTPUT_CC
             if ( NULL != pContext->hOutputCCDump )
             {
                fprintf(pContext->hOutputCCDump, "-,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,DISCARD(%d)\n",
                   decodePictureId,
                   pContext->currentPolarity,
                   pContext->uiOutputQueueEntryCount,
                   pContext->numPacketDescriptors,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].uiDecodePictureId,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.format,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.bIsAnalog,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.cc_valid,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.line_offset,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.seq.cc_type,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.cc_data_1,
                   pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.cc_data_2,
                   threshold
                   );
             }
#endif

         pstdInfo->type[uiType].uiReadOffset++;
         pstdInfo->type[uiType].uiReadOffset %= pContext->createSettings.queueSize;
      }
   }

   return bResult;
}


BERR_Code
BXUDlib_P_Add_UserData_isr( BXUDlib_P_Context *pContext, BUDP_DCCparse_Format format, BXUD_StdInfo *pstdInfo, const BVDC_Display_CallbackData *pstDisplayCallbackData )
{
    BXUD_EncoderPacketDescriptor *pEncoderPacketDescriptor = &pContext->encoderPacketDescriptor[pContext->numPacketDescriptors];

    pEncoderPacketDescriptor->format = format;
    pEncoderPacketDescriptor->ccCount = 0;

    /* Only 608 data - no 708. The algorithm is simple...
       1. Discard any data that is too much out of sync.
       2. Push out all the data for the current field (all data must have same decode id)
    */
    {
       bool bResult = true;

       if ( false == pstdInfo->synced )
       {
          bResult = BXUDlib_P_DataPresent_isr(pContext, pstdInfo, pstDisplayCallbackData->ulDecodePictureId );

          if ( false == bResult )
          {
#if BXUDLIB_P_DUMP_OUTPUT_CC
             if ( NULL != pContext->hOutputCCDump )
             {
                fprintf(pContext->hOutputCCDump, "%d,%d,%d,%d,-,-,%d,-,-,-,-,-,-,-,SYNC,%d,%d,%d,%d\n",
                   pstDisplayCallbackData->ulStgPictureId,
                   pstDisplayCallbackData->ulDecodePictureId,
                   pContext->currentPolarity,
                   pContext->uiOutputQueueEntryCount,
                   format,
                   pstdInfo->type[0].uiReadOffset,
                   pstdInfo->type[0].uiWriteOffset,
                   pstdInfo->type[1].uiReadOffset,
                   pstdInfo->type[1].uiWriteOffset
                   );
             }
#endif
             return BERR_BXUD_NO_DATA;
          }
       }

       if ( true == bResult )
       {
          bResult = BXUDlib_P_DiscardOutOfSyncData_isr(pContext, pstdInfo, pstDisplayCallbackData->ulDecodePictureId, pstdInfo->synced ? BXUD_OUT_OF_SYNC_THRESHOLD : 1 );
       }

       if ( false == pstdInfo->synced )
       {
          pstdInfo->synced = bResult;
       }
    }


    /* if the current packet is does not match the currentPolarity, send an empty packet */
    {
       uint8_t output708Count = 0;
       unsigned int uiType;

       for ( uiType = 0; uiType < 2; uiType++ )
       {
          bool bDone = false;

          while ( ( false == bDone ) && ( pstdInfo->type[uiType].uiReadOffset != pstdInfo->type[uiType].uiWriteOffset ) )
          {

             if ( ( false == pContext->bCurrentPolarityValid )
                  && ( pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity != BAVC_Polarity_eFrame ) )
             {
                pContext->currentPolarity = pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity;
                pContext->bCurrentPolarityValid = true;
             }

             switch ( format )
             {
                case BUDP_DCCparse_Format_DVS157:
                case BUDP_DCCparse_Format_DVS053:
                  if( pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.cc_valid )
                  {
                     /* SW7425-5994: polarity should never equal eFrame for DVS157/DVS053. If it does, we need to
                      * recover gracefully, so we assume the polarity the same as the current polarity and then
                      * print a warning message
                      */
                      if ( pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity == BAVC_Polarity_eFrame )
                      {
                         BDBG_WRN(("Invalid source polarity seen for DVS157/DVS053 analog CC standards.  Overriding polarity: %d-->%d",
                            pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity,
                            pContext->currentPolarity));

                         pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity = pContext->currentPolarity;
                      }

                      if( pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity != pContext->currentPolarity )
                      {
                         bDone = true;
                      }
                  }
                  break;
                case BUDP_DCCparse_Format_ATSC53:
                  if( ( pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity != BAVC_Polarity_eFrame ) &&
                      ( pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity != pContext->currentPolarity )
                    )
                  {
                      bDone = true;
                  }

                  /* We only want a max of 9 708 packets per frame */
                  if ( pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity == BAVC_Polarity_eFrame )
                  {
                     if ( ( 0 == pEncoderPacketDescriptor->ccCount )
                          && ( pstdInfo->type[0].uiReadOffset != pstdInfo->type[0].uiWriteOffset )
                          && ( pstdInfo->type[1].astCCData[pstdInfo->type[1].uiReadOffset].uiDecodePictureId == pstdInfo->type[0].astCCData[pstdInfo->type[0].uiReadOffset].uiDecodePictureId )
                        )
                     {
                        /* Don't process 708 if there's corresponding 608 data for this picture id to be processed */
                        bDone = true;
                     }
                     else
                     {
                        if ( output708Count < B_MAX_708_CC_COUNT )
                        {
                           output708Count++;
                        }
                        else
                        {
                           bDone = true;
                        }
                     }
                  }
                  break;
                default:
                   bDone = true;
                   break;
             }

             if ( false == bDone )
             {
                pEncoderPacketDescriptor->ccData[pEncoderPacketDescriptor->ccCount] = pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData;
                pEncoderPacketDescriptor->ccCount++;

   #if BXUDLIB_P_DUMP_OUTPUT_CC
                if ( NULL != pContext->hOutputCCDump )
                {
                   fprintf(pContext->hOutputCCDump, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,DATA,%d,%d,%d,%d\n",
                      pstDisplayCallbackData->ulStgPictureId,
                      pstDisplayCallbackData->ulDecodePictureId,
                      pContext->currentPolarity,
                      pContext->uiOutputQueueEntryCount,
                      pContext->numPacketDescriptors,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].uiDecodePictureId,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.format,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.bIsAnalog,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.polarity,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.cc_valid,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.line_offset,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.seq.cc_type,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.cc_data_1,
                      pstdInfo->type[uiType].astCCData[pstdInfo->type[uiType].uiReadOffset].ccData.cc_data_2,
                      pstdInfo->type[0].uiReadOffset,
                      pstdInfo->type[0].uiWriteOffset,
                      pstdInfo->type[1].uiReadOffset,
                      pstdInfo->type[1].uiWriteOffset
                      );
                }
   #endif

                pstdInfo->type[uiType].uiReadOffset++;
                pstdInfo->type[uiType].uiReadOffset %= pContext->createSettings.queueSize;
             }
          }
       }
    }

    if( pEncoderPacketDescriptor->ccCount > 0 )
    {
#if BXUDLIB_P_DEBUG_CC_PACKET
        printCcPacket_isr(pContext, 'O', pstDisplayCallbackData->ulStgPictureId, pstDisplayCallbackData->ulDecodePictureId, ccCount, currentCcData, 0xffffffff );
        BDBG_MSG(("to collect %d out ccCount, ccCount=%d, valid = 0x%x", pEncoderPacketDescriptor->ccCount, currentPacket->ccCount,currentPacket->valid));
#endif
        pContext->numPacketDescriptors++;
        return BERR_SUCCESS;
    }
    else
    {
#if BXUDLIB_P_DUMP_OUTPUT_CC
       if ( NULL != pContext->hOutputCCDump )
       {
          fprintf(pContext->hOutputCCDump, "%d,%d,%d,%d,-,-,%d,-,-,-,-,-,-,-,NO DATA,%d,%d,%d,%d\n",
             pstDisplayCallbackData->ulStgPictureId,
             pstDisplayCallbackData->ulDecodePictureId,
             pContext->currentPolarity,
             pContext->uiOutputQueueEntryCount,
             format,
             pstdInfo->type[0].uiReadOffset,
             pstdInfo->type[0].uiWriteOffset,
             pstdInfo->type[1].uiReadOffset,
             pstdInfo->type[1].uiWriteOffset
             );
       }
#endif
        return BERR_BXUD_NO_DATA;
    }
}

/* BXUDlib_DisplayInterruptHandler_isr is the heartbeat of the XUD library where all the work is done.  
   XUD gets the user data from the source for uiDecodePictureId via the BXUDlib_UserDataCallback_isr.
   On this call, XUD will appropriately redistribute the user data packets to the STG rate (rate conversion).
   XUD will add the rate converted user data to the sink per uiStgPictureId.
*/
BERR_Code
BXUDlib_DisplayInterruptHandler_isr(BXUDlib_Handle hXud, const BVDC_Display_CallbackData *pstDisplayCallbackData1)
{
    BXUDlib_P_Context *pContext = (BXUDlib_P_Context *)hXud;
    BVDC_Display_CallbackData tempData = *pstDisplayCallbackData1;
    BVDC_Display_CallbackData *pstDisplayCallbackData = &tempData;
    unsigned i=0;
    
    BDBG_MSG(("(%d, %d):%d", pstDisplayCallbackData->ulStgPictureId, pstDisplayCallbackData->ulDecodePictureId, pstDisplayCallbackData->ePolarity));
    
    switch( pstDisplayCallbackData->sDisplayInfo.ulVertRefreshRate ) {
    
    case 5994:
    case 6000:/* 1:1 mapping */
        pContext->repCnt  = 1;
    break;

    case 2397:
    case 2400:/* 2:3 repeat cadence */
        pContext->repCnt  = (2==pContext->repCnt) ? 3 : 2;
    break;
	
	case 2997:
    case 3000:/* 2:2 repeat cadence */
        pContext->repCnt = 2;
    break;
	
    default:/* Only framerates of 24, 30 and 60 are supported */
        if(pContext->initialSync || pContext->repCnt) {
            BDBG_WRN(("Unsupported CC user data STG rate = %u", pstDisplayCallbackData->sDisplayInfo.ulVertRefreshRate));
        }
        pContext->repCnt = 0;/* don't fail; simply ignore it */
    break;

    }

    if ( true == pContext->bPreviousDisplayCallbackDataValid )
    {
       /* SW7425-5519: The STG incremented by more than one.  This means that we missed a few vsyncs (e.g. host was blocked),
        * so we need to re-sync the userdata against the decode ID.
        */
       if ( ( pContext->stPreviousDisplayCallbackData.ulStgPictureId + 1 ) != pstDisplayCallbackData->ulStgPictureId )
       {
          unsigned i;

          BDBG_WRN(("STG ID is out of sync (%u:%u).  Re-syncing CC userdata...",
              pContext->stPreviousDisplayCallbackData.ulStgPictureId,
              pstDisplayCallbackData->ulStgPictureId
              ));
          for ( i = 0; i < BXUD_EncapsulationStd_Max; i++ )
          {
             pContext->stdInfo[i].synced = false;
          }
       }
    }

    /* assume XUD input data queue is already converted to 60hz */
    for(i = 0; i < pContext->repCnt; i++) {
        pContext->decodeId = pstDisplayCallbackData->ulDecodePictureId;
        if( pstDisplayCallbackData->ePolarity == BAVC_Polarity_eFrame )
        {
            /* just toggle the field */
            if( pContext->currentPolarity == BAVC_Polarity_eTopField )
            {
                pContext->currentPolarity = BAVC_Polarity_eBotField;
            }
            else
            {
                pContext->currentPolarity = BAVC_Polarity_eTopField;
            }
        }
        else
        {
            pContext->currentPolarity = pstDisplayCallbackData->ePolarity;
            pContext->bCurrentPolarityValid = true;
        }

        pContext->numPacketDescriptors = 0;
        pContext->decodeId = pstDisplayCallbackData->ulDecodePictureId;
        BDBG_MSG(("Current field = %d", pContext->currentPolarity));
   
        BXUDlib_P_Add_UserData_isr(pContext, BUDP_DCCparse_Format_DVS157, &pContext->stdInfo[BXUD_EncapsulationStd_Scte20], pstDisplayCallbackData);
        BXUDlib_P_Add_UserData_isr(pContext, BUDP_DCCparse_Format_DVS053, &pContext->stdInfo[BXUD_EncapsulationStd_Scte21], pstDisplayCallbackData);
        BXUDlib_P_Add_UserData_isr(pContext, BUDP_DCCparse_Format_ATSC53, &pContext->stdInfo[BXUD_EncapsulationStd_A53], pstDisplayCallbackData);

        BXUDlib_P_OutputUserData_isr(pContext, pstDisplayCallbackData->ulStgPictureId);
    }
    pContext->initialSync = false;

    pContext->stPreviousDisplayCallbackData = *pstDisplayCallbackData;
    pContext->bPreviousDisplayCallbackDataValid = true;

    return BERR_SUCCESS;
}




