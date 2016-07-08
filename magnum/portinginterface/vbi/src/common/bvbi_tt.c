/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bvbi.h"                /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"           /* VBI internal data structures */

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
 *
 */
BERR_Code BVBI_Field_GetTTData_isr (
    BVBI_Field_Handle fieldHandle,
    int                    nLines,
    BVBI_TT_Line *        ttLines
)
{
    uint8_t *pData;
    uint32_t lineMask;
    int      iLine;
    int      iChar;
    BVBI_P_Field_Handle* pVbi_Fld;
    BVBI_P_TTData* pTTData;
    BERR_Code eErr = BERR_SUCCESS;

    BDBG_ENTER(BVBI_Field_GetTTData_isr);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    if(!ttLines)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Verify that data is present on this field handle */
    if (!(pVbi_Fld->ulWhichPresent & BVBI_P_SELECT_TT))
        return (BVBI_ERR_FIELD_NODATA);
    else if (pVbi_Fld->ulErrInfo & BVBI_LINE_ERROR_TELETEXT_NOENCODE)
        eErr = (BVBI_ERR_FIELD_BADDATA);

    /* Pull out the mask of valid lines */
    pTTData = BVBI_P_LCOP_GET_isr (pVbi_Fld, TTDataO);
    lineMask = pTTData->lineMask;
    lineMask <<= (pTTData->firstLine);

#ifdef BVBI_P_TTE_WA15
    /* Abort if user's array is too small */
    if (pTTData->firstLine >= nLines)
    {
        BDBG_ERR(("User array too small"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Loop over initial unused lines */
    for (iLine = 0 ; iLine < pTTData->firstLine ; ++iLine)
    {
        ttLines->ucFramingCode = BVBI_TT_INVALID_FRAMING_CODE;
        ++ttLines;
    }
#endif

    /* Loop over teletext data lines in the field handle */
    pData = pTTData->pucData + 4;
    for (iLine = 0 ; iLine < pTTData->ucLines ; ++iLine)
    {
        /* If data line is valid */
        if ((lineMask >> iLine) & 0x1)
        {
            /* Abort if user's array is too small */
#ifdef BVBI_P_TTE_WA15
            if (pTTData->firstLine + iLine >= nLines)
#else
            if (iLine >= nLines)
#endif
            {
                BDBG_ERR(("User array too small"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }

            ttLines->ucFramingCode = *pData++;
            for (iChar = 0 ; iChar < pTTData->ucLineSize - 1 ; ++iChar)
            {
                ttLines->aucData[iChar] = *pData++;
            }
            for ( ; iChar < BVBI_TT_MAX_LINESIZE ; ++iChar)
            {
                ttLines->aucData[iChar] = 0x0;
            }
        }
        else /* data line is invalid */
        {
#ifdef BVBI_P_TTE_WA15
            /* Info for user */
            if ( pTTData->firstLine + iLine < nLines)
            ttLines->ucFramingCode = BVBI_TT_INVALID_FRAMING_CODE;

            /* Skip over wasted space */
            pData += pTTData->ucLineSize;
#else
            ttLines->ucFramingCode = BVBI_TT_INVALID_FRAMING_CODE;
#endif
        }

        /* Next element of user's array */
        ++ttLines;
    }

    /* Indicate the rest of the lines in the user's data array is empty */
#ifdef BVBI_P_TTE_WA15
    for ( ; pTTData->firstLine + iLine < nLines ; ++iLine)
#else
    for ( ; iLine < nLines ; ++iLine)
#endif
    {
        ttLines->ucFramingCode = BVBI_TT_INVALID_FRAMING_CODE;
        ++ttLines;
    }

    BDBG_LEAVE(BVBI_Field_GetTTData_isr);
    return eErr;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_Field_SetTTData_isr (
    BVBI_Field_Handle fieldHandle,
    BFMT_VideoFmt    eVideoFormat,
    int                    nLines,
    BVBI_TT_Line *        ttLines
)
{
    uint32_t lineMask;
    uint8_t* pData;
    void*    cached_ptr;
    uint8_t  lineWidth;
    int      iLine;
    int      iChar;
    BVBI_P_Field_Handle* pVbi_Fld;
    BVBI_P_TTData* pTTData;
    uint16_t bytes_used;
    BMEM_Handle hMem;
    BERR_Code eErr;
#ifdef BVBI_P_TTXADR_WAROUND
    bool firstLine = true;
#endif

    BDBG_ENTER(BVBI_Field_SetTTData_isr);

    /* check parameters */
    pVbi_Fld = fieldHandle;
    BDBG_OBJECT_ASSERT (pVbi_Fld, BVBI_FIELD);
    if(!ttLines)
    {
        BDBG_ERR(("Invalid parameter"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hMem = pVbi_Fld->pVbi->hMem;

    switch (eVideoFormat)
    {
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
    case BFMT_VideoFmt_e720x482_NTSC:
    case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        lineWidth = 34;
        break;
    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
        lineWidth = 43;
        break;
    default:
        BDBG_LEAVE(BVBI_Field_SetTTData_isr);
        return BERR_TRACE (BERR_INVALID_PARAMETER);
        break;
    }

    /* Mark TT data in field handle "invalid" until it is complete */
    pVbi_Fld->ulWhichPresent &= ~BVBI_P_SELECT_TT;

    /* Initialize line mask to "empty" */
    lineMask = 0x0;

    /* Initialize pointer into field handle data */
    BVBI_P_LCOP_WRITE_isr (
        pVbi_Fld, TTDataO, &pVbi_Fld->pVbi->ttFreelist, clink);
    pTTData = BVBI_P_LCOP_GET_isr (pVbi_Fld, TTDataO);
    eErr = BERR_TRACE (
        BMEM_ConvertAddressToCached_isr (hMem, pTTData->pucData, &cached_ptr));
    if (eErr != BERR_SUCCESS)
    {
        BDBG_ERR(("Cache memory failure"));
        return eErr;
    }
    pData = (uint8_t*)cached_ptr;
    pData += 4;

    /* Check that the field handle is big enough to start.
       More checking will follow */
    bytes_used = 4;
    if (pTTData->ucDataSize < bytes_used)
    {
        BDBG_ERR(("Field handle cannot accomodate teletext data"));
        return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
    }

    /* Loop over lines of user's TT data */
    pTTData->firstLine = 0xFF;
    for (iLine = 0 ; iLine < nLines ; ++iLine)
    {
        /* If there is data on this line in user space */
        if (ttLines->ucFramingCode != BVBI_TT_INVALID_FRAMING_CODE)
        {
            /* Verify that there is enough room in field handle */
            bytes_used += lineWidth;
#ifdef BVBI_P_TTXADR_WAROUND
            if (firstLine)
            {
                ++bytes_used;
            }
#endif
            if (bytes_used > pTTData->ucDataSize)
            {
                BDBG_ERR(("Field handle cannot accomodate teletext data"));
                return BERR_TRACE(BVBI_ERR_FLDH_CONFLICT);
            }

            if (pTTData->firstLine == 0xFF)
                pTTData->firstLine = iLine;

            /* Copy one line of data */
            *pData++ = ttLines->ucFramingCode;
#ifdef BVBI_P_TTXADR_WAROUND
            if (firstLine)
            {
                /* Write framing code again! */
                *pData++ = ttLines->ucFramingCode;
                firstLine = false;
            }
#endif
            for (iChar = 0 ; iChar < lineWidth - 1 ; ++iChar)
            {
                *pData++ = ttLines->aucData[iChar];
            }

            /* Update line mask */
            lineMask |= (0x1 << iLine);

            /* Update line count in private data */
            pTTData->ucLines = (uint8_t)(iLine + 1);
        }
#ifdef BVBI_P_TTE_WA15
        else /* No data on this line in user space */
        {
            /* Create wasted space in the field handle! */
            if (pTTData->firstLine != 0xFF)
            {
                bytes_used += lineWidth;
                pData += lineWidth;
            }

        } /* If there is data on this line in user space */
#endif

        /* Go to next line of user's data */
        ++ttLines;
    }

    /* Write the line mask and line width to the start of the data */
    if (pTTData->firstLine == 0xFF)
#ifdef BVBI_P_TTE_WA15
        pTTData->firstLine = 0;
#else
        lineMask = 0x0;
#endif
    else
        lineMask = lineMask >> pTTData->firstLine;
    pTTData->lineMask = lineMask;
    pData = (uint8_t*)cached_ptr;
    *(uint32_t*)(pData) = 0xFFFFFFFF;
    BMEM_FlushCache_isr (hMem, cached_ptr, bytes_used);
    pTTData->ucLineSize = lineWidth;

    /* Indicate valid data is present */
    pVbi_Fld->ulWhichPresent |= BVBI_P_SELECT_TT;

    BDBG_LEAVE(BVBI_Field_SetTTData_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
* Implementation of supporting teletext functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_TT_Init( BVBI_P_Handle *pVbi )
{
#if (BVBI_NUM_TTE == 0) && (BVBI_NUM_TTE_656 == 0) /** { **/
    BSTD_UNUSED (pVbi);
#else /** } { **/
    uint8_t hwIndex;

    BDBG_ENTER(BVBI_P_TT_Init);

    /* Initialize TT encoders */
#if (BVBI_NUM_TTE > 0)
    for (hwIndex = 0 ; hwIndex < BVBI_NUM_TTE ; ++hwIndex)
        BVBI_P_TT_Enc_Init (pVbi->hReg, false, hwIndex);
#endif
#if (BVBI_NUM_TTE_656 > 0)
    for (hwIndex = 0 ; hwIndex < BVBI_NUM_TTE_656 ; ++hwIndex)
        BVBI_P_TT_Enc_Init (pVbi->hReg, true, hwIndex);
#endif

    BDBG_LEAVE(BVBI_P_TT_Init);
#endif /** } **/
    return BERR_SUCCESS;
}


uint32_t BVBI_P_TT_Size_Storage(uint32_t ulMaxLines, uint32_t ulMaxLineSize)
/*
    Sizes teletext data "slab."  Includes space for framing code.  Does not
    include space for alignment.
*/
{
    return 4  +  ulMaxLines * ulMaxLineSize;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_P_TTData_Alloc (
    BMEM_Handle hMem, uint8_t ucMaxLines, uint8_t ucLineSize,
    BVBI_P_TTData* pTTData)
{
    void* cached_ptr;
    BERR_Code eErr;

    /* Sanity check */
    if (!hMem || !pTTData)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Deallocate data if necessary */
    if (pTTData->pucData != 0)
    {
        BMEM_Free (hMem, pTTData->pucData);
        pTTData->ucDataSize =   0;
        pTTData->ucLines    =   0;
        pTTData->ucLineSize =   0;
        pTTData->pucData    = 0x0;
    }

    /* If user wants to hold any teletext data */
    if (ucMaxLines != 0)
    {
        uint32_t dataSize = BVBI_P_TT_Size_Storage (ucMaxLines, ucLineSize);
#ifdef BVBI_P_TTXADR_WAROUND
        ++dataSize;
#endif
        pTTData->pucData =
            (uint8_t*)(BMEM_AllocAligned (
                hMem,
                dataSize,
                8,
                0));
        if (!pTTData->pucData)
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        eErr = BERR_TRACE (
            BMEM_ConvertAddressToCached (hMem, pTTData->pucData, &cached_ptr));
        if (eErr != BERR_SUCCESS)
        {
            BDBG_ERR(("Cache memory failure"));
            BMEM_Free (hMem, pTTData->pucData);
            return eErr;
        }

        /* Zero out the line mask */
        *(uint32_t*)cached_ptr = 0x0;

        /* Complete the self-description of the teletext data */
        pTTData->ucDataSize =   dataSize;
        pTTData->ucLines    =          0;
        pTTData->ucLineSize =          0;

        /* Debug code */
        /*
        {
        uint8_t* start = cached_ptr;
        uint8_t* end = start + dataSize;
        while (start < end)
        {
            *(uint32_t*)start = 0xDEADBEEF;
            start += sizeof(uint32_t);
        }
        }
        */
    }

    return BERR_SUCCESS;
}

/***************************************************************************
* Static (private) functions
***************************************************************************/


/* End of file */
