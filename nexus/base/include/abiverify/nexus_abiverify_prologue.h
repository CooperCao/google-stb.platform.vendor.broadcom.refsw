/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
***************************************************************************/
#ifndef NEXUS_ABI_VERIFY_PROLOGUE_H__
#define NEXUS_ABI_VERIFY_PROLOGUE_H__

#include "nexus_base.h"

#define B_CONCAT_1(x,y) x ## y
#define B_CONCAT(x,y) B_CONCAT_1(x,y)

#include "nexus_base_compat.h"

#if NEXUS_P_ABI_VERIFY_MODE_VERIFY
#include <stdio.h>
#include <assert.h>


#define VERIFY_COMPAT_BEGIN(x)    void B_CONCAT(NEXUS_P_ABIVERIFY_MODULE, x) (void) {
#define VERIFY_COMPAT_END(x)      return;}
#define VERIFY_COMPAT_FIELD_RUNTIME(type, field, offset, size, sub_type) BDBG_CWARNING( (offset == offsetof(B_NEXUS_COMPAT_TYPE(type), field)) && (size == B_SIZEOF(B_NEXUS_COMPAT_TYPE(type),field)) );
#define VERIFY_COMPAT_FIELD_BUILDTIME(type, field, offset, size, sub_type) fprintf(stderr, "%s,%s,%u,%u,%u,%u,%s\n", #type, #field, offset, size, (unsigned)offsetof(B_NEXUS_COMPAT_TYPE(type), field), (unsigned)B_SIZEOF(B_NEXUS_COMPAT_TYPE(type), field), #sub_type); assert( (offset == offsetof(B_NEXUS_COMPAT_TYPE(type), field)) && (size == B_SIZEOF(B_NEXUS_COMPAT_TYPE(type),field)) );
#define VERIFY_COMPAT_FIELD(type, field, offset, size, sub_type) VERIFY_COMPAT_FIELD_RUNTIME(type, field, offset, size, sub_type); VERIFY_COMPAT_FIELD_BUILDTIME(type, field, offset, size, sub_type);
#define VERIFY_BEGIN(x)
#define VERIFY_END(x)
#define VERIFY_FIELD(type, field, sub_type)
#define VERIFY_STRUCT(type)
#define VERIFY_GENERIC(type, field, sub_type)
#define VERIFY_HANDLE(type, field, sub_type)
#define VERIFY_PLAIN_TYPE(type, field, sub_type)
#define VERIFY_FAKE_HANDLE(type, field, sub_type)
#define VERIFY_POINTER(type, field, sub_type)
#else
#undef B_NEXUS_COMPAT_TYPE_HANDLE
#define B_NEXUS_COMPAT_TYPE_HANDLE(handle) handle
#undef B_NEXUS_COMPAT_TYPE_FAKE_HANDLE
#define B_NEXUS_COMPAT_TYPE_FAKE_HANDLE(handle) handle
#undef B_NEXUS_COMPAT_TYPE_ENUM_HANDLE
#define B_NEXUS_COMPAT_TYPE_ENUM_HANDLE(handle) handle
#undef B_NEXUS_COMPAT_TYPE_MEMORY
#define B_NEXUS_COMPAT_TYPE_MEMORY(ptr) ptr
#undef B_NEXUS_COMPAT_TYPE_MISC
#define B_NEXUS_COMPAT_TYPE_MISC(type,alias) type
#undef B_NEXUS_COMPAT_TYPE_POINTER
#define B_NEXUS_COMPAT_TYPE_POINTER(type) type
#undef B_NEXUS_COMPAT_TYPE_NULL_POINTER
#define B_NEXUS_COMPAT_TYPE_NULL_POINTER(type) type
#if NEXUS_P_ABI_VERIFY_MODE_PRINT
#include <stdio.h>
#include <assert.h>
#define VERIFY_BEGIN(x)    void B_CONCAT(NEXUS_P_ABIVERIFY_MODULE, x) (void) {fprintf(stdout,"VERIFY_COMPAT_BEGIN(%s)\n", #x);
#define VERIFY_FIELD(type, field, sub_type) fprintf(stdout,"VERIFY_COMPAT_FIELD(%s,%s,%u,%u,%s)\n", #type, #field, (unsigned)offsetof(type, field), (unsigned)B_SIZEOF(type, field), #sub_type);
#define VERIFY_STRUCT(type)
#define VERIFY_PLAIN_TYPE(type, field, sub_type)
#define VERIFY_GENERIC(type, field, sub_type)
#define VERIFY_HANDLE(type, field, sub_type)
#define VERIFY_FAKE_HANDLE(type, field, sub_type)
#define VERIFY_POINTER(type, field, sub_type)
#define VERIFY_END(x)      fprintf(stdout,"VERIFY_COMPAT_END(%s)\n", #x);return;}
#else
#define VERIFY_END(x)      return;}
#if 1
#define VERIFY_BEGIN(x)    void B_CONCAT(NEXUS_P_ABIVERIFY_MODULE, x) (void) {
#define VERIFY_FIELD(type, field, sub_type) BDBG_CASSERT( (offsetof(type, field) == offsetof(B_NEXUS_COMPAT_TYPE(type), field)) && (B_SIZEOF(type,field) == B_SIZEOF(B_NEXUS_COMPAT_TYPE(type),field)) );
#define VERIFY_STRUCT(type) BDBG_CASSERT(sizeof(type) == sizeof(B_NEXUS_COMPAT_TYPE(type)));
#define VERIFY_GENERIC(type, field, sub_type) BDBG_CASSERT(sizeof(sub_type)==sizeof(int));
#define VERIFY_PLAIN(type, field, sub_type) BDBG_CASSERT(sizeof(sub_type)==sizeof(int));
#define VERIFY_HANDLE(type, field, sub_type) BDBG_CASSERT(sizeof(sub_type)==sizeof(void *));
#define VERIFY_FAKE_HANDLE(type, field, sub_type) BDBG_CASSERT(sizeof(sub_type)==sizeof(void *));
#define VERIFY_POINTER(type, field, sub_type) BDBG_CASSERT(sizeof(sub_type)==sizeof(void *));
#else
#include <stdio.h>
#include <assert.h>

#define VERIFY_BEGIN(x)    void B_CONCAT(NEXUS_P_ABIVERIFY_MODULE, x) (void) {fprintf(stdout,"%-40s,%8s,%8s,%8s,%8s,%20s\n","field", "offset", "size", "offset", "size", "type");
#define VERIFY_FIELD(type, field, sub_type) do { \
    unsigned offset = offsetof(type, field), size = B_SIZEOF(type, field),\
    offset_compat = offsetof(B_NEXUS_COMPAT_TYPE(type), field), size_compat=B_SIZEOF(B_NEXUS_COMPAT_TYPE(type), field); \
    fprintf(stdout,"%-40s,%8u,%8u,%8u,%8u,%20s\n", #type "." #field, offset, size, offset_compat, size_compat, #sub_type);\
    assert(offset==offset_compat); assert(size==size_compat);} while(0);
#define VERIFY_STRUCT(type) do { \
    unsigned size = sizeof(type), size_compat = sizeof(B_NEXUS_COMPAT_TYPE(type));\
    fprintf(stdout,"%-40s,,%8u,,%8u\n", #type, size, size_compat);\
    assert(size==size_compat);} while(0);
#define VERIFY_PLAIN(type, field, sub_type) do { fprintf(stdout, "%-40s,,%8u,,%8u,%20s\n", #type "." #field, (unsigned)sizeof(sub_type), (unsigned)sizeof(int), #sub_type); assert(sizeof(sub_type)==sizeof(int));} while(0);
#define VERIFY_HANDLE(type, field, sub_type) do { fprintf(stdout, "%-40s,,%8u,,%8u,%20s\n", #type "." #field, (unsigned)sizeof(sub_type), (unsigned)sizeof(void *), #sub_type); assert(sizeof(sub_type)==sizeof(void *));} while(0);
#define VERIFY_FAKE_HANDLE(type, field, sub_type) do { fprintf(stdout, "%-40s,,%8u,,%8u,%20s\n", #type "." #field, (unsigned)sizeof(sub_type), (unsigned)sizeof(void *), #sub_type); assert(sizeof(sub_type)==sizeof(void *));} while(0);
#define VERIFY_POINTER(type, field, sub_type) do { fprintf(stdout, "%-40s,,%8u,,%8u,%20s\n", #type "." #field, (unsigned)sizeof(sub_type), (unsigned)sizeof(void *), #sub_type); assert(sizeof(sub_type)==sizeof(void *));} while(0);
#endif /* #if 1 */
#endif
#endif

#endif /* NEXUS_ABI_VERIFY_PROLOGUE_H__ */
