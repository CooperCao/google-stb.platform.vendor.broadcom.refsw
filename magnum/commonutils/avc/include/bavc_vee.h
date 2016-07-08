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
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BAVC_VEE_H__
#define BAVC_VEE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* This enum follows h264 spec of pic_struct syntax for picture cadence */
typedef enum BAVC_PicStruct
{
    BAVC_PicStruct_eFrame = 0,
    BAVC_PicStruct_eTopField,
    BAVC_PicStruct_eBotField,
    BAVC_PicStruct_eTopFirst,
    BAVC_PicStruct_eBotFirst,
    BAVC_PicStruct_eTopBotTopRepeat,
    BAVC_PicStruct_eBotTopBotRepeat,
    BAVC_PicStruct_eFrameDoubling,
    BAVC_PicStruct_eFrameTripling,
    BAVC_PicStruct_eReserved
} BAVC_PicStruct;

typedef struct BAVC_EncodePictureBuffer
{
    /* encode picture Y/C buffer */
    BMMA_Block_Handle    hLumaBlock;
    uint32_t             ulLumaOffset;
    BMMA_Block_Handle    hChromaBlock;
    uint32_t             ulChromaOffset;

    /* Y/C buffer format: 420; linear or striped */
    bool                 bStriped;

    /* if striped format, the stripe parameters */
    unsigned             ulStripeWidth;
    unsigned             ulLumaNMBY;
    unsigned             ulChromaNMBY;

    /* picture size */
    unsigned             ulWidth;
    unsigned             ulHeight;

    /* picture polarity: T/B/F */
    BAVC_Polarity        ePolarity;

    /* sample aspect ratio x/y */
    unsigned             ulAspectRatioX;
    unsigned             ulAspectRatioY;

    /* frame rate */
    BAVC_FrameRateCode   eFrameRate;

    /* Timestamp Parameters */
    uint32_t             ulOriginalPTS; /* 32-bit original PTS value (in 45 Khz or 27Mhz?) */
    uint32_t             ulSTCSnapshotLo; /* lower 32-bit STC snapshot when picture received at the displayable point (in 27Mhz) */
    uint32_t             ulSTCSnapshotHi; /* high 10-bit STC snapshot when picture received at the displayable point (in 27Mhz) */

    /* Displayable point Picture ID */
    uint32_t ulPictureId;

    /* Optional cadence info for PicAFF encode */
    bool                 bCadenceLocked;/* false if unused */
    BAVC_PicStruct       ePicStruct; /* h264 style pic_struct cadence info */

    /* Optional 2H1V/2H2V decimated luma buffers (share the same MMA block as Y/C buffer) */
    BMMA_Block_Handle    h2H1VLumaBlock;/* NULL if unused */
    uint32_t             ul2H1VLumaOffset;
    BMMA_Block_Handle    h2H2VLumaBlock;/* NULL if unused */
    uint32_t             ul2H2VLumaOffset;
} BAVC_EncodePictureBuffer;

#ifdef __cplusplus
}
#endif

#endif /* BAVC_VEE_H__ */
