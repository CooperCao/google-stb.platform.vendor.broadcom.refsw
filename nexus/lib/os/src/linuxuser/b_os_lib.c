/***************************************************************************
*     (c)2004-2015 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description:
*   API name: OS
*    Library routines for OS abstraction
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "b_os_lib.h"
#include "b_os_priv.h"
#include "bkni.h"
#include <pthread.h>

BDBG_MODULE(b_os_lib);

static pthread_mutex_t g_initMutex = PTHREAD_MUTEX_INITIALIZER;
static int g_refCount;

/***************************************************************************
Summary:
Initialize the OS Library

Description:
It is permissible to call this routine multiple times.  Internally, a
reference count will be maintained.
***************************************************************************/
B_Error B_Os_Init(void)
{
    B_Error errCode;
    int rc;

    rc = pthread_mutex_lock(&g_initMutex);
    if ( rc )
    {
        return B_ERROR_OS_ERROR;
    }

    /* Check for first open */
    if ( 0 == g_refCount++ )
    {
        errCode = BKNI_Init();
        if ( errCode ) {goto error_bkni;}

        errCode = B_Thread_P_Init();
        if ( errCode ) {goto error_thread;}

        errCode = B_Lock_P_Init();
        if ( errCode ) {goto error_lock;}
    }

    pthread_mutex_unlock(&g_initMutex);
    return B_ERROR_SUCCESS;

error_lock:
    B_Thread_P_Uninit();
error_thread:
    BKNI_Uninit();
error_bkni:
    g_refCount=0;
    pthread_mutex_unlock(&g_initMutex);
    return BERR_TRACE(errCode);

}

/***************************************************************************
Summary:
Un-Initialize the OS Library

Description:
This routine must be called once per call to B_Os_Init
***************************************************************************/
B_Error B_Os_Uninit(void)
{
    int rc;
    rc = pthread_mutex_lock(&g_initMutex);
    if ( rc )
    {
        return B_ERROR_OS_ERROR;
    }
    if (g_refCount == 0) {
        pthread_mutex_unlock(&g_initMutex);
        return BERR_TRACE(B_ERROR_NOT_INITIALIZED);
    }

    /* Check for last close */
    if ( 0 == --g_refCount )
    {
        B_Lock_P_Uninit();
        B_Thread_P_Uninit();
        BKNI_Uninit();
    }

    pthread_mutex_unlock(&g_initMutex);
    return B_ERROR_SUCCESS;
}
