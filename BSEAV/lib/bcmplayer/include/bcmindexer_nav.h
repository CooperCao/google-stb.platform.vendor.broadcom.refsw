/***************************************************************************
 *     Copyright (c) 2002-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Private types and prototypes for bcmindexer & bcmplayer
 *
 * Revision History:
 *
 * $brcm_Log: $
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

#define BNAV_set_frameOffsetHi( pEntry, frameOffsetHi ) \
        ((pEntry)->words[2] = (frameOffsetHi))
#define BNAV_get_frameOffsetHi( pEntry ) \
        ((pEntry)->words[2])
#define BNAV_set_frameOffsetLo( pEntry, frameOffsetLo ) \
        ((pEntry)->words[3] = (frameOffsetLo))
#define BNAV_get_frameOffsetLo( pEntry ) \
    ((pEntry)->words[3])
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
