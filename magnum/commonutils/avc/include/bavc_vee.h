/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
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
