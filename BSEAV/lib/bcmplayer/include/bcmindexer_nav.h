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
 * Module Description: Private types and prototypes for bcmindexer & bcmplayer
 *
 ***************************************************************************/
#ifndef BCMINDEXER_NAV_H__
#define BCMINDEXER_NAV_H__

#ifdef __cplusplus
extern "C" {
#endif

/* enum for BNAV_get_frameType/BNAV_set_frameType */
typedef enum
{
    eSCTypeSeqHdr,          /* unused */
    eSCTypeIFrame,          /* I-frame */
    eSCTypePFrame,          /* P-frame */
    eSCTypeBFrame,          /* B-frame */
    eSCTypeGOPHdr,          /* unused */
    eSCTypeRPFrame,         /* Reference picture frame: P-frame with I-slice 0 */
    eSCTypeUnknown          /* Unknown or "don't care" frame type */
} eSCType;

/*******************************
*
* macros used to decode and encode BNAV_Entry
* for external use, use BNAV_Player_ReadIndex
*
******/

#define BNAV_set_frameType( pEntry, frameType ) \
        ((pEntry)->words[0] &= 0x00fffffful, \
        (pEntry)->words[0] |= ((unsigned long)(frameType) << 24))
#define BNAV_get_frameType( pEntry ) \
    ((eSCType)((pEntry)->words[0] >> 24))
#define BNAV_set_seqHdrStartOffset( pEntry, seqHdrStartOffset ) \
        ((pEntry)->words[0] &= 0xff000000ul, \
        (pEntry)->words[0] |= ((seqHdrStartOffset) & 0x00fffffful))
#define BNAV_get_seqHdrStartOffset( pEntry ) \
    ((unsigned long)((pEntry)->words[0] & 0xffffff))

#define BNAV_set_seqHdrSize( pEntry, seqHdrSize ) \
        ((pEntry)->words[1] &= 0x0000fffful, \
        (pEntry)->words[1] |= ((unsigned long)(seqHdrSize) << 16))
#define BNAV_get_seqHdrSize( pEntry ) \
    ((unsigned short)(((pEntry)->words[1] >> 16) & 0xffff))
#define BNAV_set_refFrameOffset( pEntry, refFrameOffset ) \
        ((pEntry)->words[1] &= 0xffff00fful, \
        (pEntry)->words[1] |= ((unsigned long)(refFrameOffset) << 8))
#define BNAV_get_refFrameOffset( pEntry ) \
        ((unsigned char)(((pEntry)->words[1] >> 8) & 0xff))

#define BNAV_set_version( pEntry, version ) \
        ((pEntry)->words[1] &= 0xffffff0ful, \
        (pEntry)->words[1] |= (unsigned long)(((version) & 0xf) << 4))
#define BNAV_get_version( pEntry ) \
        ((unsigned char)((pEntry)->words[1] & 0xf0)>>4)

/* NOTE: Lower 4 bits of [1] available */

/* 32 bit access to frameOffset lo/hi is deprecated. Use 64 bit access (below) to avoid programming errors. */
#define BNAV_set_frameOffsetHi( pEntry, frameOffsetHi ) \
        ((pEntry)->words[2] = (frameOffsetHi))
#define BNAV_get_frameOffsetHi( pEntry ) \
        ((pEntry)->words[2])
#define BNAV_set_frameOffsetLo( pEntry, frameOffsetLo ) \
        ((pEntry)->words[3] = (frameOffsetLo))
#define BNAV_get_frameOffsetLo( pEntry ) \
    ((pEntry)->words[3])

/* 64 bit access to frameOffset */
#define BNAV_get_frameOffset( pEntry ) \
        (((uint64_t)(pEntry)->words[2])<<32 | ((pEntry)->words[3]))
#define BNAV_set_frameOffset( pEntry, frameOffset ) \
        do {(pEntry)->words[2] = (frameOffset)>>32; (pEntry)->words[3] = (frameOffset)&0xFFFFFFFF;} while(0)

#define BNAV_set_framePts( pEntry, framePts ) \
    ((pEntry)->words[4] = (framePts))
#define BNAV_get_framePts( pEntry ) \
    ((pEntry)->words[4])
/**
* Maximum frameSize is 256 MB (28 bits). This should
* be large enough for the largest HD I-frame possible.
**/
#define BNAV_set_frameSize( pEntry, frameSize ) \
    ((pEntry)->words[5] &= 0xf0000000ul, \
    (pEntry)->words[5] |= ((unsigned long)(frameSize) & 0x0ffffffful))
#define BNAV_get_frameSize( pEntry ) \
    ((pEntry)->words[5] & 0x0ffffffful)

/* NOTE: Upper 4 bits of [5] available */

#define BNAV_set_timestamp( pEntry, timestamp) \
    ((pEntry)->words[6] = (timestamp))
#define BNAV_get_timestamp( pEntry ) \
    ((pEntry)->words[6])

/**
* 12 bits for packed vchip information. See BNAV_pack_vchip and
* BNAV_unpack_vchip.
**/
#define BNAV_set_packed_vchip( pEntry, vchipdata) \
    ((pEntry)->words[7] &= 0xfffff000ul, \
    (pEntry)->words[7] |= (unsigned long)((vchipdata) & 0xfff))
#define BNAV_get_packed_vchip( pEntry ) \
    ((unsigned short)((pEntry)->words[7] & 0xfff))

/* NOTE: Upper 20 bits of [7] available */


/*******************************
*
* macros used to decode and encode BNAV_AVC_Entry
*
******/

/* SPS is sequence parameter set. 24 bits allows 16 MB offset in stream. */
#define BNAV_set_SPS_Offset( pEntry, seqHdrStartOffset ) \
        ((pEntry)->words[8] &= 0xff000000ul, \
        (pEntry)->words[8] |= ((seqHdrStartOffset) & 0x00fffffful))
#define BNAV_get_SPS_Offset( pEntry ) \
    ((unsigned long)((pEntry)->words[8] & 0xffffff))

/* 16 bits allows 64 KB for SPS, which is far more than enough. */
#define BNAV_set_SPS_Size( pEntry, seqHdrStartOffset ) \
        ((pEntry)->words[9] &= 0xffff0000ul, \
        (pEntry)->words[9] |= ((seqHdrStartOffset) & 0x0000fffful))
#define BNAV_get_SPS_Size( pEntry ) \
    ((unsigned long)((pEntry)->words[9] & 0xffff))

#define BNAV_set_RandomAccessIndicator( pEntry, randomAccessIndicator) \
                (((BNAV_AVC_Entry*)pEntry)->words[10] &= 0xfffffffeul, \
                 ((BNAV_AVC_Entry*)pEntry)->words[10] |= ((randomAccessIndicator) & 0x00000001ul))
#define BNAV_get_RandomAccessIndicator( pEntry ) \
        ((unsigned long)(((BNAV_AVC_Entry*)pEntry)->words[10] & 0x1))

#ifdef __cplusplus
}
#endif

#endif /* BCMINDEXER_NAV_H__ */
