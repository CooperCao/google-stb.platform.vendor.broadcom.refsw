/***************************************************************************
 *     Copyright (c) 2010-2012, Broadcom Corporation
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
 * Metadata processing definitions for File-based MP4 software mux
 *
 * [Revision History:]
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BMUXLIB_FILE_MP4_METADATA_H__
#define BMUXLIB_FILE_MP4_METADATA_H__

/* Includes */

#ifdef __cplusplus
extern "C" {
#endif

/****************************
*  D E F I N I T I O N S    *
****************************/


/****************************
*        T Y P E S          *
****************************/

/* types for each metadata entry written to temp storage
   (not necessarily per-sample information)
   NOTE: data is written into these structs in the format
   that would be written to the sample tables  ... */
typedef struct
{
   uint32_t uiCount;                                     /* count of number (consecutive) samples with this delta */
   uint32_t uiDelta;                                     /* DTS(n+1) - DTS(n) */
} BMUXlib_File_MP4_DTSDelta;

typedef struct
{
   uint32_t uiCount;                                     /* count of number (consecutive) samples with this offset */
   uint32_t uiOffset;                                    /* CTS(n) - DTS(n) */
} BMUXlib_File_MP4_CTSDTSOffset;

typedef uint32_t BMUXlib_File_MP4_SampleSize;            /* size of sample n */

typedef uint64_t BMUXlib_File_MP4_SampleOffset;          /* location of the sample within the mdat - later converted to file offsets */

typedef uint32_t BMUXlib_File_MP4_RAPSampleNum;          /* the sample number of the RAP */

typedef uint32_t BMUXlib_File_MP4_FourCC;                /* a four-character code (note: big-endian) */

typedef enum
{
   BMUXlib_File_MP4_P_MetadataType_eStts = 0,      /* data type: BMUXlib_File_MP4_DTSDelta */
   BMUXlib_File_MP4_P_MetadataType_eCtts,          /* data type: BMUXlib_File_MP4_CTSDTSOffset */
   BMUXlib_File_MP4_P_MetadataType_eStsz,          /* data type: BMUXlib_File_MP4_SampleSize */
   BMUXlib_File_MP4_P_MetadataType_eStco,          /* data type: BMUXlib_File_MP4_SampleOffset */
   BMUXlib_File_MP4_P_MetadataType_eStss,          /* data type: BMUXlib_File_MP4_RAPSampleNum */
   BMUXlib_File_MP4_P_MetadataType_eMax
} BMUXlib_File_MP4_P_MetadataType;

/* the current metadata for the sample being processed - this information is then converted to the metadata
   stored which is later written into the sample tables */
typedef struct
{
   uint32_t uiDTSDelta;                                  /* DTS(n+1) - DTS(n) (i.e. current sample duration) */
   uint32_t uiCTSDTSDiff;                                /* CTS(n) - DTS(n) */
   uint32_t uiSampleSize;                                /* size of sample n */
   uint64_t uiOffset;                                    /* location of the sample within the mdat - later converted to file offsets */
   bool bRandomAccess;                                   /* set if this sample is a RAP */
} BMUXlib_File_MP4_P_Metadata;

typedef struct
{
   void *pBuffer;
   uint32_t uiNumEntries;
   uint32_t uiEntrySize;                                 /* note that the size of the buffer depends on the entry type */
   uint32_t uiWriteIndex;                                /* current write index into the cache */
   uint32_t uiReadIndex;                                 /* current read location - either 0 or 1/2 buffer size */
   bool bFlushDone;                                      /* cache has been flushed to storage */
} BMUXlib_File_MP4_P_MetadataCache;

/****************************
*    P R O T O T Y P E S    *
****************************/

void BMUXlib_File_MP4_P_ProcessMetadata(BMUXlib_File_MP4_Handle hMP4Mux);
bool BMUXlib_File_MP4_P_RetrieveMetadataToCache(BMUXlib_File_MP4_Handle hMP4Mux, BMUXlib_File_MP4_P_MetadataType eMetadataType, bool *pbInCacheFlag);
void BMUXlib_File_MP4_P_StoreMetadataToCache(BMUXlib_File_MP4_Handle hMP4Mux, BMUXlib_File_MP4_P_Metadata *pMetadata);
void BMUXlib_File_MP4_P_FinalizeMetadata(BMUXlib_File_MP4_Handle hMP4Mux);
bool BMUXlib_File_MP4_P_FlushAllMetadata(BMUXlib_File_MP4_Handle hMP4Mux);
void BMUXlib_File_MP4_P_FreeMetadataCache(BMUXlib_File_MP4_P_MetadataCache *pMetadataCache);
void BMUXlib_File_MP4_P_AdjustOffsets(BMUXlib_File_MP4_Handle hMP4Mux);

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_FILE_MP4_METADATA_H__ */

/*****************************************************************************
* EOF
******************************************************************************/
