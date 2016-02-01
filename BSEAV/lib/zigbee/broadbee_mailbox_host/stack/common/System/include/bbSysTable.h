/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysTable.h $
*
* DESCRIPTION:
*   Tables engine interface and implementation.
*
* $Revision: 2927 $
* $Date: 2014-07-15 16:04:50Z $
*
****************************************************************************************/
#ifndef _BB_SYS_TABLE_H
#define _BB_SYS_TABLE_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */
#include "bbSysTime.h"
#include "bbHalSystemTimer.h"

/************************* DEFINITIONS **************************************************/
// TODO: Move it to appropriate place. ///////////
#include "bbSysCrc.h"
INLINE uint16_t HAL_Hash(const uint32_t hashKey)
{
    // TODO: This is stub.
    return SYS_Crc16(0x2A, (uint8_t *)&hashKey, sizeof(hashKey));
}
//////////////////////////////////////////////////


#define SYS_TABLE_NO_BUSY_BIT_OFFSET 0U
#define SYS_TABLE_NO_HASH_DWORD 0U

#define SYS_TABLE_FOLD(tableDescr, iteratorName, hashKey) \
    void *iteratorName = SYS_TableGetFindPoint(tableDescr, hashKey); \
    for (uint8_t tries = SYS_TableRowNum(tableDescr); \
        0 < tries; \
        --tries, iteratorName = SYS_TableEntryGetNext(tableDescr, iteratorName))

/**************************************************************************//**
  \brief Constructs table descriptor

  \param[in] table - pointer to the table area.
  \param[in] rowNum - Number of rows in the table
  \param[in] entrySize - size of entry in bytes.
  \param[in] busyFlagOffset - offset of flagField in table entry structure.
  \param[in] busyFlagBitOffset - busy flag bit number.
 ******************************************************************************/
#define SYS_TABLE_INIT(                                                             \
        tableM, rowNumM, entrySizeM, busyFlagOffsetM, busyFlagBitOffsetM)           \
    (SYS_TableDescriptor_t)                                                         \
    {                                                                               \
        .table                = (void*)tableM,                                      \
        .rowNum               = rowNumM,                                            \
        .entrySize            = entrySizeM,                                         \
        .busyFlagOffset       = busyFlagOffsetM,                                    \
        .busyFlagBitOffset    = busyFlagBitOffsetM                                  \
    }

/**************************************************************************//**
  \brief Initializes the specified Age Data

  \param[in] lastTimeStamp - pointer to the lastTimeStamp variable
  \param[in] agingStep - aging step, ms
  \param[in] ttlInEntryOffset - offset of the TTL field
  \param[in] ttlBits - number of bits allocated for TTL in the byte starting with bit 0
 ******************************************************************************/
#define SYS_TABLE_AGEDATA_INIT(                                                     \
        lastTimeStampM, agingStepM, ttlInEntryOffsetM, ttlBitsM)                    \
    (SYS_TableAgingData_t)                                                          \
    {                                                                               \
        .lastTimeStamp = lastTimeStampM,                                            \
        .agingStep = agingStepM,                                                    \
        .ttlInEntryOffset = ttlInEntryOffsetM,                                      \
        .ttlMask = (1 << ttlBitsM) - 1                                              \
    }

/************************* TYPES *******************************************************/

/**//**
 * \brief Table descriptor type.
 */
typedef const struct _SysTableDescriptor_t
{
    uint8_t *table;
    uint8_t rowNum;
    uint8_t entrySize;
    uint8_t busyFlagOffset;
    uint8_t busyFlagBitOffset;
} SYS_TableDescriptor_t;

/**//**
 * \brief Aging Data of a table (additional descriptor)
 */
typedef const struct _SYS_TableAgingData_t
{
  SYS_Time_t    *lastTimeStamp;      /* System time, ms */
  uint16_t      agingStep;           /*  ms */
  uint8_t       ttlInEntryOffset;    /* offset of the byte where a TTL field resides */
  uint8_t       ttlMask;             /* TTL byte mask */
} SYS_TableAgingData_t;

/************************* PROTOTYPES **************************************************/

/**************************************************************************//**
  \brief Resets table

  \param[in] descr - table descriptor.
 ******************************************************************************/
INLINE void SYS_TableReset(SYS_TableDescriptor_t *const descr)
{
    memset(descr->table, 0U, descr->rowNum * descr->entrySize);
}

/**************************************************************************//**
  \brief Gets table size.
  \param[in] descr - table descriptor.
  \return table size.
 ******************************************************************************/
INLINE uint8_t SYS_TableRowNum(SYS_TableDescriptor_t *const descr)
{
    return descr->rowNum;
}

/**************************************************************************//**
  \brief Gets entry table size.
  \param[in] descr - table descriptor.
  \return entry table size.
 ******************************************************************************/
INLINE uint8_t SYS_TableGetEntrySize(SYS_TableDescriptor_t *const descr)
{
    return descr->entrySize;
}

/**************************************************************************//**
  \brief Gets start point of table area.
  \param[in] descr - table descriptor.
  \return first entry pointer.
 ******************************************************************************/
INLINE uint8_t *SYS_TableGetStartPoint(SYS_TableDescriptor_t *const descr)
{
    return descr->table;
}

/**************************************************************************//**
  \brief Gets end point of table area.
  \param[in] descr - table descriptor.
  \return last entry pointer.
 ******************************************************************************/
INLINE uint8_t *SYS_TableGetEndPoint(SYS_TableDescriptor_t *const descr)
{
    return descr->table + descr->rowNum * descr->entrySize;
}

/**************************************************************************//**
  \brief Gets starting point to find.
  \param[in] descr - table descriptor.
  \return entry pointer.
 ******************************************************************************/
INLINE uint8_t *SYS_TableGetFindPoint(SYS_TableDescriptor_t *const descr, const uint32_t hashKey)
{
    return SYS_TableGetStartPoint(descr) +
        (HAL_Hash(hashKey) % SYS_TableRowNum(descr)) * SYS_TableGetEntrySize(descr);
}

/**************************************************************************//**
  \brief Gets status of entry.
  \param[in] descr - table descriptor.
  \param[in] entry - entry to check
  \return true if entry is busy.
 ******************************************************************************/
INLINE bool SYS_TableEntryIsBusy(SYS_TableDescriptor_t *const descr, const void *const entry)
{
    const uint8_t *const bitField = (uint8_t *)entry + descr->busyFlagOffset;
    return BITMAP_ISSET(bitField, descr->busyFlagBitOffset);
}

/**************************************************************************//**
  \brief Gets the next entry of the specified table. It loops around table.

  \param[in] descr - table descriptor.
  \param[in] currentEntry - current entry (NULL - special case).

  \return   next entry pointer,
            if currentEntry = NULL, returns the first row
 ******************************************************************************/
INLINE void *SYS_TableEntryGetNext(SYS_TableDescriptor_t *const descr,
                                   const void *const currentEntry)
{
    uint8_t *nextOne;

    if (NULL != currentEntry)
    {
        nextOne = (uint8_t *)currentEntry + descr->entrySize;

        if (nextOne < SYS_TableGetEndPoint(descr))
            return (void*)nextOne;
    }

    return (void*) SYS_TableGetStartPoint(descr);
}

/**************************************************************************//**
  \brief Gets the next entry of the specified table. It does not loop.

  \param[in] descr - table descriptor.
  \param[in] currentEntry - current entry (NULL - special case).

  \return   next entry pointer,
            if currentEntry = NULL, returns the first row.
            Returns NULL, if the ens of table is reached.
 ******************************************************************************/
INLINE void *SYS_TableEntryGetNextDirect(SYS_TableDescriptor_t *const descr,
                                   const void *const currentEntry)
{
    uint8_t *nextOne, *res;

    if (NULL == currentEntry)
    {
        res = SYS_TableGetStartPoint(descr);
    }
    else
    {
        nextOne = (uint8_t *)currentEntry + descr->entrySize;

        if (nextOne < SYS_TableGetEndPoint(descr))
            res = nextOne;
        else
            res = NULL;
    }

    return (void*) res;
}

/**************************************************************************//**
  \brief Gets a next non-empty entry of the specified table. It does not loop.

  \param[in] descr - table descriptor.
  \param[in] currentEntry - current entry (NULL - special case).

  \return   next non-empty entry pointer,
            if currentEntry = NULL, returns the first non-empty row
            Returns NULL, if the ens of table is reached.
 ******************************************************************************/
INLINE void *SYS_TableEntryGetNextNonEmpty(SYS_TableDescriptor_t *const descr,
                                   const void *const currentEntry)
{
    void *res = (void*) currentEntry;

    do
    {
        res = SYS_TableEntryGetNextDirect(descr, res);
        if (SYS_TableEntryIsBusy(descr, res))
            break;
    } while (NULL != res);

    return res;
}

/**************************************************************************//**
  \brief Occupies the specified entry
  \param[in] descr - table descriptor.
  \param[in] entry - entry
 ******************************************************************************/
INLINE void SYS_TableEntryOccupy(SYS_TableDescriptor_t *const descr, void *const entry)
{
    uint8_t *const bitField = (uint8_t *)entry + descr->busyFlagOffset;
    BITMAP_SET(bitField, descr->busyFlagBitOffset);
}

/**************************************************************************//**
  \brief Sets up status of the entry as busy.
  \param[in] descr - table descriptor.
  \param[in] idx - index of the entry
 ******************************************************************************/
INLINE void SYS_TableEntryOccupyByIdx(SYS_TableDescriptor_t *const descr, uint8_t idx)
{
    uint8_t *const bitField = SYS_TableGetStartPoint(descr) +
        idx * SYS_TableGetEntrySize(descr) + descr->busyFlagOffset;
    BITMAP_SET(bitField, descr->busyFlagBitOffset);
}

/**************************************************************************//**
  \brief Frees the specified entry.
  \param[in] descr - table descriptor.
  \param[in] entry - entry to free
 ******************************************************************************/
INLINE void SYS_TableEntryFree(SYS_TableDescriptor_t *const descr, const void *const entry)
{
    uint8_t *const bitField = (uint8_t *)entry + descr->busyFlagOffset;
    BITMAP_CLR(bitField, descr->busyFlagBitOffset);
}

/**************************************************************************//**
  \brief Gets free entry.
  \param[in] descr - table descriptor.
  \return NULL if free entry not found and entry otherwise.
 ******************************************************************************/
INLINE void *SYS_TableEntryGetEmpty(SYS_TableDescriptor_t *const descr, const uint32_t hashKey)
{
    SYS_TABLE_FOLD(descr, entry, hashKey)
    {
        if (!SYS_TableEntryIsBusy(descr, entry))
            return entry;
    }
    return NULL;
}

/**************************************************************************//**
  \brief Swaps two entries.
  \param[in] descr - table descriptor.
  \param[in] firstEntry, secondEntry - entries to swap.
 ******************************************************************************/
INLINE void SYS_TableEntrySwap(SYS_TableDescriptor_t *const descr,
                               void *firstEntry, void *secondEntry)
{
    SWAP(firstEntry, secondEntry, SYS_TableGetEntrySize(descr));
}

/**************************************************************************//**
  \brief Returns index of the entry
  \param[in] descr - table descriptor.
  \param[in] entry - entry
 ******************************************************************************/
INLINE int SYS_TableEntryIndexGet(SYS_TableDescriptor_t *const descr,
        const void *const entry)
{
    return ((uint8_t*)entry - (uint8_t*)descr->table) / descr->entrySize;
}

/**************************************************************************//**
  \brief Returns pointer of the entry by the specified index
  \param[in] descr - table descriptor.
  \param[in] entryIdx - entry index
 ******************************************************************************/
INLINE void* SYS_TableEntryGetPointerByIndex(SYS_TableDescriptor_t *const descr, int entryIdx)
{
    return descr->table + entryIdx * descr->entrySize;
}


/**************************************************************************//**
  \brief Resets the specified Age Data

  \param[in] agingData - aging data of the table
 ******************************************************************************/
INLINE void SYS_TableAgeDataReset(SYS_TableAgingData_t *const agingData)
{
    *agingData->lastTimeStamp = 0;
}

/**************************************************************************//**
  \brief Returns TTL of the specified entry
  \param[in] agingData - aging data of the table
  \param[in] entry - entry of the table
 ******************************************************************************/
INLINE uint8_t SYS_TableAgeEntryTtlGet(SYS_TableAgingData_t *const agingData, void *const entry)
{
    return *((uint8_t*)entry + agingData->ttlInEntryOffset) & agingData->ttlMask;
}

/**************************************************************************//**
  \brief Sets up the TTL of the specified entry
  \param[in] agingData - aging data of the table
  \param[in] entry - entry of the table
  \param[in] value - new TTL value
 ******************************************************************************/
INLINE void SYS_TableAgeEntryTtlSet(SYS_TableAgingData_t *const agingData,
        void *const entry, uint8_t value)
{
    SYS_DbgAssert(!(value & ~agingData->ttlMask), SYS_TABLE_AGING_ILLEAGAL_VALUE);

    uint8_t *pTtl = (uint8_t*)entry + agingData->ttlInEntryOffset;

    *pTtl = (*pTtl & ~agingData->ttlMask) | value;
}

/**************************************************************************//**
  \brief Updates age of the specified entry
  \param[in] descr - table descriptor
  \param[in] agingData - aging data of the table
  \param[in] entry - entry of the table
 ******************************************************************************/
INLINE void SYS_TableAgeUpdateEntry(SYS_TableDescriptor_t *const descr,
        SYS_TableAgingData_t *const agingData, void *const entry)
{
    SYS_Time_t time, diff;
    uint8_t oldTtl, newTtl;

    time = HAL_GetSystemTime();
    diff = (uint8_t)MIN((time - *agingData->lastTimeStamp) / agingData->agingStep, UINT8_MAX);

    if (diff == 0)
        return;

    oldTtl = SYS_TableAgeEntryTtlGet(agingData, entry);
    newTtl = oldTtl - MIN(oldTtl, diff);

    if (newTtl)
        SYS_TableAgeEntryTtlSet(agingData, entry, newTtl);
    else
        SYS_TableEntryFree(descr, entry);
}

/**************************************************************************//**
  \brief Updates the TimeStamp of the Table.
  \param[in] agingData - aging data of the table
 ******************************************************************************/
INLINE void SYS_TableAgeUpdateTimeStamp(SYS_TableAgingData_t *const agingData)
{
    *agingData->lastTimeStamp = HAL_GetSystemTime();
}

#endif /* _BB_SYS_TABLE_H */