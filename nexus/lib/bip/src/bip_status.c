/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/

#include <string.h>

#include "bip_priv.h"

BDBG_MODULE(bip_status);


/* Define the BIP_MAKE_STATUS_CODE macro so that it will populate the
 * BIP_StatusTextLookupTable with the descriptive text and the
 * statusCode.  */
#ifdef BIP_MAKE_STATUS_CODE
#undef BIP_MAKE_STATUS_CODE
#endif
#define BIP_MAKE_STATUS_CODE(name,class,id)  { NEXUS_MAKE_ERR_CODE(class,id), #name},

/* Allow for multiple inclusion of "bip_statuscodes.h". */
#ifdef BIP_STATUSCODES_H
#undef BIP_STATUSCODES_H
#endif

static struct BIP_StatusTextLookupTable
{
    BIP_StatusCode      statusCode;
    char *              statusName;
} BIP_StatusTextLookupTable [] =
{
    /* Start of the BIP_StatusTextLookupTable initialization. */

    /* When we include "bip_statuscodes.h", the BIP_MAKE_STATUS_CODE macro will
     * build a list of statuscodes and their associated text that looks
     * similar to this:
     *
     *      { NEXUS_MAKE_STATUS_CODE(0xB1F, 0x1), "BIP_ERR_NOT_INITIALIZED"     },
     *      { NEXUS_MAKE_STATUS_CODE(0xB1F, 0x2), "BIP_ERR_INVALID_PARAMETER"   },
     *      { NEXUS_MAKE_STATUS_CODE(0xB1F, 0x3), "BIP_ERR_OUT_OF_SYSTEM_MEMORY"},
     *
     * And thus populate the lookup table with the information that we need to
     * find the text for a statusCode.  */

#include "bip_statuscodes.h"
      /* End of the BIP_StatusTextLookupTable initialization. */
};

/**
 * Summary:
 * Convert a BIP_StatusCode to a text string.
 *
 * Description:
 * This function searches a table of BIP_StatusCodes for the specified
 * statusCode and returns a pointer to the associated text string.
 * The lookup table is built at compile time and will use statusCode
 * definitions from bip_statuscodes.h.
 *
 **/
const char *  BIP_StatusGetText( BIP_StatusCode statusCode)
{
    unsigned i;

    /* For any of the errno-encoded BIP status codes, just return the
     * text for BIP_ERR_OS_ERRNO. */
    if (BIP_StatusToErrno(statusCode) != 0) {
        statusCode = BIP_ERR_OS_ERRNO;
    }

    for (i=0 ; i<(sizeof BIP_StatusTextLookupTable/sizeof *BIP_StatusTextLookupTable) ; i++ ) {
        if (BIP_StatusTextLookupTable[i].statusCode == statusCode) {
            return BIP_StatusTextLookupTable[i].statusName;
        }
    }

    return NULL;
}


/**
 * Summary:
 * Convert an "errno" to a BIP_StatusCode.
 *
 * Description:
 * This function takes an "errno" integer and encodes it into a BIP_Status
 * value.  If the specified errno value is invalid (e.g., negative), then
 * BIP_ERR_OS_ERRNO_INVALID is returned.
 **/
BIP_Status  BIP_StatusFromErrno(int errnoValue)
{
    if (errnoValue <= 0) {return BIP_ERR_OS_ERRNO_INVALID;}
    if (errnoValue > (BIP_ERR_OS_ERRNO_MAX - BIP_ERR_OS_ERRNO_MIN + 1)) {return BIP_ERR_OS_ERRNO_INVALID;}

    return (BIP_ERR_OS_ERRNO_MIN + errnoValue - 1);
}


/**
 * Summary:
 * Extract the errno from a BIP_StatusCode.
 *
 * Description:
 * This function tries to extract an "errno" value from a BIP_StatusCode.
 * Only BIP_StatusCodes from BIP_ERR_OS_ERRNO_MIN to BIP_ERR_OS_ERRNO_MAX (inclusive) contain
 * an encoded "errno" value.  If the specified BIP_StatusCode does not contain an "errno"
 * value, this function will return zero (which is never a valid "errno").
 **/
int  BIP_StatusToErrno(BIP_Status bipStatus)
{
    if (bipStatus < BIP_ERR_OS_ERRNO_MIN) {return 0;}
    if (bipStatus > BIP_ERR_OS_ERRNO_MAX)  {return 0;}

    return (bipStatus - BIP_ERR_OS_ERRNO_MIN) + 1;
}


/**
 * Summary:
 * Print an error message for a BIP_Status
 *
 * Description:
 * This function prints an BDBG_ERR message (containing the source file and line
 * number) for a non-successful BIP_Status code.
 *  1. If the status code indicates BIP_SUCCESS, it returns successfully without printing.
 *  2. If the status code is not from BIP, then BIP_ERR_NOT_AVAILABLE is returned.
 *  3. If the BIP_Status contains an embedded "errno" value it will be extracted
 *     and printed descriptively.
 *  4. Otherwise, a human readable description of the BIP_Status is printed.
 **/
BIP_Status  BIP_Status_PrintError(const char *pFileName, int line, BIP_Status bipStatus)
{
    const char  *pBipStatusText = BIP_StatusGetText(bipStatus);
    int          errnoValue = BIP_StatusToErrno(bipStatus);
    char         emptyString[] = "";

    if (bipStatus == BIP_SUCCESS) {
        return BIP_SUCCESS;        /* If normal success, don't print an error message. */
    }

    if (pBipStatusText == NULL) {  /* Must not be a BIP_Status, maybe BERR_ or NEXUS_ status? */
        return BIP_ERR_NOT_AVAILABLE;
    }

    if (errnoValue != 0 ) {        /* If this is an errno-encoded BIP_Status... */
        char    errnoBuf[128];
        char   *pErrnoText = errnoBuf;

        #if ANDROID || ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE)
            /* Code for XSI version of strerror_r(). (Not tested.) */
            int     sysRc = strerror_r( errnoValue, errnoBuf, sizeof errnoBuf);
            if (sysRc != 0) { pErrnoText = emptyString;}
        #else
            /* Code for GNU version of strerror_r(). */
            pErrnoText = strerror_r( errnoValue, errnoBuf, sizeof errnoBuf);
            if (pErrnoText == NULL) {pErrnoText = emptyString;}
        #endif

        BDBG_ERR(("!!!Error: %s(0x%X) errno=%d(%s) at %s:%u", pBipStatusText, bipStatus, errnoValue, pErrnoText, pFileName, line));
    }
    else        /* Otherwise, just a normal discrete BIP_Status. */
    {
        BDBG_ERR(("!!!Error: %s(0x%X) at %s:%u", pBipStatusText, bipStatus, pFileName, line));
    }
    return BIP_SUCCESS;
}

/**
 * Summary:
 * Return the thread ID for the current process.
 *
 **/
int  BIP_Status_GetMyThreadId(void)
{
    int     pId;
    B_Error rc;

    rc = B_Thread_GetId( NULL, &pId );
    if (rc != B_ERROR_SUCCESS) {
        return (-1);
    }

    return pId;
}


/**
 * Summary:
 * Print an error message about an uninitialized Settings structure.
 *
 **/
void BIP_Settings_Print(const void *ptr, size_t size, const struct bip_settings_obj *obj, const char *id, const char *file, unsigned line) {

    BSTD_UNUSED(size);
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);

    if (ptr && obj->bip_settings_obj_id==id) {
        return;
    }

    BDBG_ERR(("Uninitialized %s structure! Did you forget to call GetDefaultSettings() or GetSettings()?", id));
    return;
}


/**
 * Summary:
 * Low-level interface to BDBG_ debug logging.
 *
 **/
#if USE_BIP_BDBG_PRINTMSG_FOR_LOGGING
#if BDBG_DEBUG_BUILD
void BIP_Status_BdbgPrintmsg(const char * func, int line, BDBG_pDebugModuleFile pModule, BDBG_Level lvl,  const char *fmt, ...)
{
    if (lvl >= (BDBG_Level)pModule->level) {
        char            printbuf[256];
        va_list         ap;             /* Argument pointer for vsnprintf().      */
        size_t          bytesUsed = 0;  /* How many bytes of printbuf are in use. */
        const char    * myThreadName = B_Thread_GetMyName();

        /* Build a message prefix that looks like:  "<Line number>: [<thread id>|<thread name>] <function name>()::" */
        if (myThreadName) {
            bytesUsed += snprintf(printbuf+bytesUsed, sizeof(printbuf)-bytesUsed, "%4d: [%12.12s] %s():: ", line, myThreadName, func);
        }
        else {
            bytesUsed += snprintf(printbuf+bytesUsed, sizeof(printbuf)-bytesUsed, "%4d: [%12.8x] %s():: ", line, BIP_Status_GetMyThreadId(), func);
        }

        va_start(ap, fmt);
        bytesUsed += vsnprintf(printbuf+bytesUsed, sizeof(printbuf)-bytesUsed, fmt, ap);
        va_end(ap);


        BDBG_P_PRINTMSG_PRIV((*pModule), (uint8_t)lvl, (printbuf));
    }
}
#endif  /* #if BDBG_DEBUG_BUILD */
#endif  /* #if USE_BIP_BDBG_PRINTMSG_FOR_LOGGING */
