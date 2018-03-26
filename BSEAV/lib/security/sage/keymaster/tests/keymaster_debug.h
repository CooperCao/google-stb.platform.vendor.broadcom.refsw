/******************************************************************************
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

 ******************************************************************************/

#ifndef KEYMASTER_DEBUG_H__
#define KEYMASTER_DEBUG_H__

#ifdef ANDROID

#ifdef BDBG_H
#error magnum bdbg.h has already been included
#endif

#include <stdio.h>
#include <assert.h>

/* Under Android, redefine BDBG_LOG, etc, to use printf for the keymaster test */

#define KM_LOG_TO_STDOUT(...) fprintf(stdout, __VA_ARGS__)
#define KM_LOG_TO_STDERR(...) fprintf(stderr, __VA_ARGS__)

#define KM_LOG_NOP() (void)0

#if BDBG_NO_MSG
#define BDBG_MSG(x) KM_LOG_NOP()
#else
#define BDBG_MSG(x) KM_LOG_TO_STDOUT x, fprintf(stdout, "\n")
#endif

#define BDBG_LOG(x) KM_LOG_TO_STDOUT x, fprintf(stdout, "\n")
#define BDBG_WRN(x) KM_LOG_TO_STDERR x, fprintf(stderr, "\n")
#define BDBG_ERR(x) KM_LOG_TO_STDERR x, fprintf(stderr, "\n")

#define BDBG_ASSERT assert

/* Stubs for BDBG macros not appliable when compiling without the bdbg wrapper */
#define BDBG_MODULE(name)
#define BDBG_OBJECT(name)
#define BDBG_OBJECT_ASSERT(ptr,name)
#define BDBG_OBJECT_DESTROY(ptr,name)
#define BDBG_OBJECT_SET(ptr,name)
#define BDBG_OBJECT_UNSET(ptr,name)
#define BDBG_OBJECT_ID(name)
#define BDBG_OBJECT_ID_DECLARE(name)
#define BDBG_SetModuleLevel(module, level)

/* Define to ensure magnum bdbg.h is not included */
#define BDBG_H

#endif /* ANDROID */

#endif  /* KEYMASTER_DEBUG_H__ */
