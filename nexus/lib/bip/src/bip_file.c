/******************************************************************************
 * (c) 2015 Broadcom Corporation
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
 *
 *****************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>

#include "bip_priv.h"

BDBG_MODULE( bip_file );


/*****************************************************************************
 *  Return a pointer to the BIP_String's null-terminated char string.
 *****************************************************************************/
BIP_Status
BIP_File_MakeDirectoryPath(const char * pPathName )
{
    BIP_Status          rc = BIP_SUCCESS;
    BIP_StringHandle    hPathName = NULL;
    int                 osRc      = 0;  /* OS return code (from system calls). */
    int                 osErrno   = 0;  /* Copy of errno (from system calls).  */
    char               *pLastSlash = NULL;

    /**************************************************************************
     *  Start by just trying to create the specified directory.  But if that
     *  directory's parent doesn't exist, the mkdir() will fail and we'll
     *  get an ENOENT errno.
     **************************************************************************/
    BDBG_MSG(( BIP_MSG_PRE_FMT "Doing mkdir for %s" BIP_MSG_PRE_ARG, pPathName ));
    osRc = mkdir( pPathName, 0777); /* 0777 is octal protection mode to be adjusted my umask. */
    if (osRc < 0) {osErrno = errno;}   /* If error, save copy of errno. */
    BIP_CHECK_GOTO((   osRc == 0      ||
                    osErrno == ENOENT ||
                    osErrno == EEXIST  ), ( "System call: mkdir returned %d, errno=%d", osRc, osErrno ), error, BIP_StatusFromErrno(osErrno), rc );

    /**************************************************************************
     *  If the parent directory doesn't exist, then we'll try to create the
     *  parent by recursing into ourself.
     **************************************************************************/
    if (osErrno == ENOENT) {

        /**************************************************************************
         *  Build the parent directory's pathname by removing the last directory
         *  name from the path.
         **************************************************************************/
        pLastSlash = rindex(pPathName, '/');
        BIP_CHECK_GOTO((pLastSlash != NULL), ( "Can't find parent dir! Is CWD valid? dir name: \"%s\"", pPathName ), error, BIP_ERR_INVALID_PARAMETER, rc );

        hPathName = BIP_String_CreateFromCharN(pPathName, pLastSlash - pPathName);

        BDBG_MSG(( BIP_MSG_PRE_FMT "Dumping: " BIP_STRING_TO_PRINTF_FMT   BIP_MSG_PRE_ARG, BIP_STRING_TO_PRINTF_ARG(hPathName) ));

        /**************************************************************************
         *  Now call ourself to create the parent directory, and it's parent
         *  and it's parent's parent, and so on...
         **************************************************************************/
        BDBG_MSG(( BIP_MSG_PRE_FMT "Recursing into BIP_File_MakeDirectoryPath() for %s" BIP_MSG_PRE_ARG, BIP_String_GetString(hPathName) ));
        rc = BIP_File_MakeDirectoryPath(BIP_String_GetString(hPathName));
        BIP_CHECK_GOTO((rc == BIP_SUCCESS), ( "Recursive call failed to create: \"%s\"", BIP_String_GetString(hPathName)), error, rc, rc );

        BIP_String_Destroy(hPathName);  hPathName = NULL;

        /**************************************************************************
         *  Now the parent directory should exist, so we can try to create the
         *  specified directory again.
         **************************************************************************/
        BDBG_MSG(( BIP_MSG_PRE_FMT "Doing mkdir of %s" BIP_MSG_PRE_ARG, pPathName ));
        osRc = mkdir( pPathName, 0777); /* 0777 is octal protection mode to be adjusted my umask. */
        if (osRc < 0) {osErrno = errno;}   /* If error, save copy of errno. */
        BIP_CHECK_GOTO((osRc == 0 || osErrno == EEXIST), ( "System call: mkdir returned %d, errno=%d", osRc, osErrno ), error, BIP_StatusFromErrno(osErrno), rc );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "BIP_File_MakeDirectoryPath normal successful return, rc=0x%x" BIP_MSG_PRE_ARG, rc ));
    return (rc);

error:
    if (hPathName) {
        BIP_String_Destroy(hPathName);  hPathName = NULL;
    }
    BDBG_ERR(( BIP_MSG_PRE_FMT "BIP_File_MakeDirectoryPath failed! rc=0x%x" BIP_MSG_PRE_ARG, rc ));
    return (rc);
}


/*****************************************************************************
 *  Canonicalize a pathname contained in a BIP_String.
 *
 *  If an error occurs, the original contents of the BIP_String will
 *  remain intact.
 *****************************************************************************/
BIP_Status
BIP_File_Realpath(BIP_StringHandle hDest, const char *pPath )
{
    const char *pIn;
    const char *pSlash;
    size_t dirLen;
    int    level = 0;
    BIP_Status  rc = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "pPath=\"%s\""
               BIP_MSG_PRE_ARG, pPath));
    /* Clear out the result BIP_String. */
    rc = BIP_String_Clear(hDest);
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_Clear() Failed" ), error, rc, rc );
    pIn = pPath;

    /* Handle the leading "/" if there is one. */
    if (*pIn=='/') {
        pIn++;
        BIP_String_StrcatChar(hDest, "/");
    }

    /* Loop through each "/" in the pathname. */
    do {
        /* Find the end of the current entry, either "/" or end of string. */
        pSlash = index(pIn, '/');
        dirLen = pSlash ? (size_t)(pSlash-pIn) : strlen(pIn);

        BDBG_MSG(( BIP_MSG_PRE_FMT "pIn=%p pSlash=%p dirLen=%u pIn=%s"
                   BIP_MSG_PRE_ARG, pIn, pSlash, dirLen, pIn));

        /* For consective "/" characters, just ignore. */
        if (dirLen == 0) {
            if (!pSlash && level > 0) {  /* Add trailing "/" and end of path. */
                BIP_String_StrcatChar(hDest, "/");
            }
        }
        /* For a "./", just ignore that, also. */
        else if (dirLen == 1 && *pIn=='.') {
            ;
        }
        /* Now if we have a "..", then we need to remove a directory from our destination string. */
        else if (dirLen == 2 && *pIn=='.' && *(pIn+1)=='.') {
            const char *pDest = BIP_String_GetString(hDest);
            const char *pTrim;
                        /* Found "/../", remove one directory from hDest. */
            if (level > 0) {
                pTrim = rindex(pDest,'/');
                if (pTrim && level > 0) {
                    BIP_String_Trim(hDest, pTrim, 0);  /* Trim from pTrim to end of string. */
                    level--;
                }
            }
            else {
                    BIP_String_StrcatChar(hDest, "../");
            }
        }
        /* Anything else must be a directory name. Add it to the destination string. */
        else {
            if (level > 0) {
                BIP_String_StrcatChar(hDest, "/");
            }
            BIP_String_StrcatCharN(hDest, pIn, dirLen);
            level++;
        }

        /* If last directory was terminated by a "/", skip past it and try for the next. */
        if (pSlash) {
            pIn = pSlash + 1;
        }
    } while (pSlash != NULL);

    BDBG_MSG(( BIP_MSG_PRE_FMT  BIP_STRING_TO_PRINTF_FMT""
               BIP_MSG_PRE_ARG, BIP_STRING_TO_PRINTF_ARG(hDest)));

error:
    return(rc);
}
