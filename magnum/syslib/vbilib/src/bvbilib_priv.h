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
#ifndef BVBILIB_PRIV_H__
#define BVBILIB_PRIV_H__

#include "bvbilib.h"
#include "blst_queue.h"
#include "bvbi_cap.h"            /* VBI hardware capabilities */

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * VBI internal enumerations and constants
 ***************************************************************************/


/***************************************************************************
 * VBI Internal data structures
 ***************************************************************************/

/* Linked list support: define decode_head, encode_head, field_head. */
typedef struct decode_head decode_head;
BLST_Q_HEAD(decode_head, BVBIlib_P_Decode_Handle);
typedef struct encode_head encode_head;
BLST_Q_HEAD(encode_head, BVBIlib_P_Encode_Handle);
typedef struct field_head field_head;
BLST_Q_HEAD(field_head, BVBIlib_P_FieldHanger);


typedef struct BVBIlib_P_FieldHanger
{
    /* The field handle itself */
    BVBI_Field_Handle hField;

    /* Linked list membership */
    BLST_Q_ENTRY(BVBIlib_P_FieldHanger) link;

} BVBIlib_P_FieldHanger;


BDBG_OBJECT_ID_DECLARE (BVBIlib);
typedef struct BVBIlib_P_Handle
{
    BDBG_OBJECT (BVBIlib)

    /* handed down from app. */
    BVBI_Handle hBvbi;

#if (BVBI_NUM_IN656 > 0)
    /* List of decode contexts */
    decode_head decode_contexts;
#endif

    /* List of encode contexts */
    encode_head encode_contexts;

} BVBIlib_P_Handle;


BDBG_OBJECT_ID_DECLARE (BVBIlib_List);
typedef struct BVBIlib_P_List_Handle
{
    BDBG_OBJECT (BVBIlib_List)

    /* Main VBI context, for creating field handles */
    BVBI_Handle hVbi;

    /* Number of BVBI_Field_Handle entries allocated at creation */
    int nAllocated;

    /* Number of BVBI_Field_Handle entries handed out.
      For debugging, mainly.  */
    int nInUse;

    /* Free list of field handles */
    field_head field_contexts;

    /* Field handle hangers with no associated field handles */
    field_head empty_hangers;

    /* Other settings */
    BVBIlib_List_Settings settings;

} BVBIlib_P_List_Handle;


BDBG_OBJECT_ID_DECLARE (BVBIlib_Dec);
typedef struct BVBIlib_P_Decode_Handle
{
    BDBG_OBJECT (BVBIlib_Dec)

    /* Back pointer to the BVBIlib context */
    BVBIlib_P_Handle *pVbilib;

    /* The subordinate BVBI_Decode handle */
    BVBI_Decode_Handle hVbiDec;

    /* Source of empty BVBI_Field_Handles to decode into */
    BVBIlib_List_Handle hVbill;

    /* Linked list membership */
    BLST_Q_ENTRY(BVBIlib_P_Decode_Handle) link;

} BVBIlib_P_Decode_Handle;


BDBG_OBJECT_ID_DECLARE (BVBIlib_Enc);
typedef struct BVBIlib_P_Encode_Handle
{
    BDBG_OBJECT (BVBIlib_Enc)

    /* Back pointer to the VBI context */
    BVBIlib_P_Handle *pVbilib;

    /* The subordinate BVBI_Encode handle */
    BVBI_Encode_Handle hVbiEnc;

    /* Maximum number of BVBI_Field_Handles to queue for encoding */
    int nMaxQueue;

    /* Input queue of field handles to encode */
    field_head encode_queue;

    /* Size of above queue */
    int encode_queue_length;

    /* Field handle hangers with no associated field handles */
    field_head empty_hangers;

    /* List of BVBI_Field_Handles to recycle to after use */
    BVBIlib_List_Handle hVbill;

    /* Linked list membership */
    BLST_Q_ENTRY(BVBIlib_P_Encode_Handle) link;

} BVBIlib_P_Encode_Handle;


/***************************************************************************
 * VBI private functions
 ***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* BVBILIB_PRIV_H__ */
