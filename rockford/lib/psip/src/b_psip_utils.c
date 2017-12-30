/***************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* Description:
*   API name: PSIP
*    Utility routines for PSIP library.
*
***************************************************************************/
#include "b_os_lib.h"
#include "b_psip_lib.h"
#include "blst_list.h"
#include <string.h>

#include "psip_mss.h"

BDBG_MODULE(b_psip_lib);

#include "b_psip_defines.h"
#include "b_psip_states.h"
#include "b_psip_gettable.h"
#include "b_psip_job.h"
#include "b_psip_psip.h"

/***************************************************************************
Summary:
    Find the job associated with the given filterHandle.

Description:
    Search SI lib's list of pending API jobs.  Search for match to given
    filterHandle.  Only one API may be called at a time for a given 
    filterHandle so there should be one and only one match in the jobList.

Returns:
    None

See Also:
    None
****************************************************************************/
B_PSIP_Job * B_PSIP_FindJob(
    B_PSIP_Handle si,
    B_PSIP_FilterHandle filterHandle
    )
{
    B_PSIP_Job * pJob = NULL;
    for (pJob = BLST_S_FIRST(&si->jobList); 
         NULL != pJob; 
         pJob = BLST_S_NEXT(pJob, node))
    {
        if (pJob->apiSettings.filterHandle == filterHandle)
        {
            break;
        }
    }

    return pJob;
}

/***************************************************************************
Summary:
    Initializes given table collection filter

Description:
    Initializes given table collection filter

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetDefaultCollectionDataFilter(
    B_PSIP_Filter * filter
    )
{
    BDBG_ASSERT(NULL != filter);

    memset(filter->coef, 0x00, sizeof(filter->coef));
    memset(filter->mask, 0xFF, sizeof(filter->mask));
    memset(filter->excl, 0xFF, sizeof(filter->excl));
}

/***************************************************************************
Summary:
    Initializes data collection request data.

Description:
    Initializes data collection request data.  This data is used by the 
    calling code to perform the actual data collection and section filtering.

Returns:
    None

See Also:
    None
****************************************************************************/
void B_PSIP_GetDefaultCollectionData(
    B_PSIP_CollectionRequest * data
    )
{
    BDBG_ASSERT(NULL != data);

    memset(data, 0, sizeof(*data));
    B_PSIP_GetDefaultCollectionDataFilter(&data->filter);
}

/***************************************************************************
Summary:

Description:
    Verify current language code.

Returns:
    Returns true if the given language code matches one of the current language
    codes.

See Also:
    None
****************************************************************************/
static bool B_PSIP_IsCurrentLang(
    B_PSIP_Handle   si, 
    uint8_t       * codeLang)
{
    bool retVal = false;
    int i = 0;

    BDBG_ASSERT(NULL != si);
    BDBG_ASSERT(NULL != codeLang);

    for (i = 0; i < NUM_LANG_CODES; i++)
    {
        /*
        BDBG_MSG(("compare our language code:%c%c%c with codeLang: %c%c%c", 
                  si->settings.ISO_639_language_code[i][0],
                  si->settings.ISO_639_language_code[i][1],
                  si->settings.ISO_639_language_code[i][2],
                  codeLang[0], codeLang[1], codeLang[2]));
        */

        if ((si->settings.ISO_639_language_code[i][0] == codeLang[0]) &&
            (si->settings.ISO_639_language_code[i][1] == codeLang[1]) &&
            (si->settings.ISO_639_language_code[i][2] == codeLang[2]))
        {
            /* found matching language code */
            retVal = true;
        }
    }

    return retVal;
}

/***************************************************************************
Summary:
    Process the MSS and retrieve the string corresponding to currently 
    selected language.

Description:
    None

Input:
    si                      Handle to PSIP instance 
    MSS                     MSS string data (can be NULL)
    pBufferMSS              Buffer to hold retrieved string
    pLengthMSS              Size of pBufferMSS

Output:
    pBufferMSS              Buffer containing retrieved string
    pLengthMSS              Size of retrieved string

Returns:
    B_ERROR_SUCCESS         On valid string found
    B_ERROR_UNKNOWN         Otherwise

See Also:
    None
****************************************************************************/
B_Error B_PSIP_GetStringFromMSS(
    B_PSIP_Handle       si,
    PSIP_MSS_string     MSS,
    uint8_t           * pBufferMSS,
    uint16_t          * pLengthMSS
    )
{
    int          str_idx;
    int          lsize;
    char       * codeLang;
    uint8_t      numStrings = 0;

    BDBG_ASSERT(NULL != si);
    BDBG_ASSERT(NULL != pBufferMSS);
    BDBG_ASSERT(NULL != pLengthMSS);

    /*B_Mutex_Lock(si->mutex);*/


    if (MSS)
    {
		numStrings = PSIP_MSS_getNumStrings(MSS);
		memset(pBufferMSS, 0, *pLengthMSS);

        for (str_idx = 0; str_idx < numStrings; ++str_idx)
        {
            if (B_ERROR_SUCCESS != PSIP_MSS_getCode(MSS, str_idx, &codeLang))
            {
                BDBG_WRN(("PSIP_MSS_getCode failed"));
                memset(pBufferMSS, 0, *pLengthMSS);
                continue;
            }

            lsize = *pLengthMSS;
            if (!B_PSIP_IsCurrentLang(si, (uint8_t *)codeLang))
            {
                BDBG_WRN(("matching lang code not found (skip) str_idx:%d numStrings:%d", str_idx, numStrings));
                memset(pBufferMSS, 0, *pLengthMSS);
                continue;
            }

            /* found string with matching language code */
            if (B_ERROR_SUCCESS != PSIP_MSS_getString(MSS, str_idx, &lsize, (char *)pBufferMSS))
            {
                BDBG_ERR(("MSS invalid"));
                memset(pBufferMSS, 0, *pLengthMSS);
            }

            BDBG_MSG(("%s:%d idx = %d, %d/%d-%s",BSTD_FUNCTION, __LINE__, str_idx, lsize, *pLengthMSS, pBufferMSS));

            *pLengthMSS = lsize;
            /*B_Mutex_Unlock(si->mutex);*/
            return B_ERROR_SUCCESS;
        }
    }

    BDBG_WRN(("%s:%d no valid string found", BSTD_FUNCTION, __LINE__));
    *pLengthMSS = 0;
    return B_ERROR_UNKNOWN;
}
