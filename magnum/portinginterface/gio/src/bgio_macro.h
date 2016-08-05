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
 *
 ***************************************************************************/

#ifndef BGIO_MACRO_H__
#define BGIO_MACRO_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 */

/* This macro take the check for a validity of a handle, and
 * cast to context pointer.
 */
#define BGIO_GENERIC_GET_CONTEXT(handle, context, structname) \
{ \
    if(!(handle) || \
       (((structname*)(handle))->ulBlackMagic != \
        (sizeof(structname) & 0xbac98800))) \
    { \
        BDBG_ERR(("NULL context handle")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (handle); \
    } \
}

/* This macro set the black magic for later handle validation
 */
#define BGIO_GENERIC_SET_BLACK_MAGIC(handle, structname) \
{ \
    ((structname*)(handle))->ulBlackMagic = sizeof(structname) & 0xbac98800; \
}

/* This macro shred the garbage for BKNI managed buffer
 */
#if BDBG_DEBUG_BUILD
#define BGIO_GENERIC_DESTROY_CONTEXT(struct_ptr, structname) \
{ \
    BKNI_Memset((void*)struct_ptr, 0xA3, sizeof(structname)); \
    BKNI_Free((void*)struct_ptr); \
    /*(void*)struct_ptr = NULL;*/ \
}
#else
#define BGIO_GENERIC_DESTROY_CONTEXT(struct_ptr, structname) \
{ \
    BKNI_Free((void*)struct_ptr); \
}
#endif

#define BGIO_RETURN_IF_ERR(result) \
    if ( BERR_SUCCESS != (result)) \
    {\
        return BERR_TRACE(result);  \
    }

#define BGIO_END_IF_ERR(success, result, label) \
    if ( false == (success) ) \
    {\
        eResult = BERR_TRACE(result);  \
        goto (label);  \
    }

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGIO_MACRO_H__ */

/* end of file */
