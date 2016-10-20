/******************************************************************************
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

 ******************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bfmt.h"
#include "bchp.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayvip_priv.h"

#if (BVDC_P_SUPPORT_VIP)

/* ==============   MEMORY CALCULATIONS   ================== */


#define BVDC_P_VIP_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES                      (9+1)
#define BVDC_P_VIP_IBBP_NUMBER_OF_DECIMATED_PICTURE_QUEUES                     (8+1)

#define BVDC_P_VIP_IP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES_PROGRESSIVE            (4)
#define BVDC_P_VIP_IP_NUMBER_OF_DECIMATED_PICTURE_QUEUES_PROGRESSIVE           (5)

#define BVDC_P_VIP_IP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES_INTERLACE              (4)
#define BVDC_P_VIP_IP_NUMBER_OF_DECIMATED_PICTURE_QUEUES_INTERLACE             (4)

#define BVDC_P_VIP_MAX_NUMBER_OF_ORIGINAL_PICTURE_QUEUES                       BVDC_P_VIP_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES
#define BVDC_P_VIP_MAX_NUMBER_OF_DECIMATED_PICTURE_QUEUES                      BVDC_P_VIP_IBBP_NUMBER_OF_DECIMATED_PICTURE_QUEUES

#define BVDC_P_VIP_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES                  BVDC_P_VIP_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES
#define BVDC_P_VIP_MAX_NUMBER_OF_IBBP_DECIMATED_PICTURE_QUEUES                 BVDC_P_VIP_IBBP_NUMBER_OF_DECIMATED_PICTURE_QUEUES

#define BVDC_P_VIP_MAX_NUMBER_OF_IP_ORIGINAL_PICTURE_QUEUES                    BVDC_P_VIP_IP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES_PROGRESSIVE
#define BVDC_P_VIP_MAX_NUMBER_OF_IP_DECIMATED_PICTURE_QUEUES                   BVDC_P_VIP_IP_NUMBER_OF_DECIMATED_PICTURE_QUEUES_PROGRESSIVE

#define BVDC_P_VIP_NUMBER_OF_ORIGINAL_PICTURE_DESCRIPTORS                      (BVDC_P_VIP_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES*2 + 1)

#define HOR_SIZE_IN_PELS_720P                                           1280
#define VER_SIZE_IN_PELS_720P                                           720

#define HOR_SIZE_IN_PELS_1080P                                           1920
#define VER_SIZE_IN_PELS_1080P                                           1088

#define HOR_SIZE_IN_PELS_PAL                                            720
#define VER_SIZE_IN_PELS_PAL                                            576


BDBG_MODULE(BVDC_VIP);
BDBG_FILE_MODULE(BVDC_VIP_MEM);
BDBG_FILE_MODULE(BVDC_DISP_VIP);
BDBG_OBJECT_ID(BVDC_VIP);

static uint8_t BVDC_P_VIP_CalcNMBY_isr
    ( uint32_t              PictureHeightInMbs,
      uint32_t              X,
      uint32_t              Y );
#define BVDC_P_VIP_CalcNMBY  BVDC_P_VIP_CalcNMBY_isr

/************************************************************************
* Function: BVDC_P_VIP_CalcWidthInStripes
*
* Actions: find number of stripes in a picture
*
* Params:
*        PictureWidthInPels: picture width in pels
*        DramStripeWidth:    Dram stripe width in bytes
*
* Returns:
*        Number of stripes in a picture width
*
************************************************************************/
static uint8_t BVDC_P_VIP_CalcWidthInStripes(uint32_t PictureWidthInPels,uint32_t DramStripeWidth)
{
    uint8_t WidthInStripes;

    if (DramStripeWidth==128)
    {
        WidthInStripes=(PictureWidthInPels>>7);
        if ((PictureWidthInPels-(WidthInStripes<<7))>0)
            WidthInStripes++;
    }
    else /* DramStripeWidth==256 */
    {
        WidthInStripes=(PictureWidthInPels>>8);
        if ((PictureWidthInPels-(WidthInStripes<<8))>0)
            WidthInStripes++;
    }
    BDBG_MODULE_MSG(BVDC_DISP_VIP, ("WidthInStripes = %u", WidthInStripes));
    return(WidthInStripes);

}


/************************************************************************
* Function: BVDC_P_VIP_AlignJword
*
* Actions: align a number to Jword(unit of 256bits).
*
* Params:
*        UnalignedNumber
*
* Returns:
*        Aligned number
*
************************************************************************/
static uint32_t BVDC_P_VIP_AlignJword(uint32_t UnalignedNumber)
{
    uint32_t AlignedNumber;
    AlignedNumber=(UnalignedNumber+31) & 0xffffffe0;
    return(AlignedNumber);
}



/************************************************************************
* Function: BVDC_P_VIP_CalcNMBY
*
* Actions: find the number of macroblocs in stripe height
*
* Params:
*        PictureHeightInMbs: hight of picture in macroblock units
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
static uint8_t BVDC_P_VIP_CalcNMBY_isr(uint32_t PictureHeightInMbs, uint32_t X, uint32_t Y)
{

    uint8_t nmby,n;

    for(n=0; n<18; ++n)  /* Supports frame sizes up to 1088, need to increase for bigger sizes */
    {/* pay attention is was only > due to a bug from the architecture cmodel */
        if((X*(uint32_t)n+Y) >= PictureHeightInMbs)
            break;
    }

    nmby = X*n+Y;
    return(nmby);


}


/************************************************************************
* Function: BVDC_P_VIP_CalcDcxvNMBY
*
* Actions: find the number of macroblocs in stripe height
*
* Params:
*        PictureHeightInMbs: hight of picture in Mbs
*        IsDcxvBuf: 1-DCXV buff 0-none DCXV buff
*        IsInterlace: 1-Interlace  0-not interlace
*        IsChroma: 1- chroma buff 0 -luma buff
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
#define DCXV_STRIPE_WIDTH 64
#define DCXV_PADDED_LINE 2
#define DCXV_COMP_RATIO 2

#define DCXVLumaBuffer            0
#define DCXVChromaBuffer          1

#define DCXVBuffer                1
#define NonDCXVBuffer             0
#define DIV2_ROUND(x)    ( ((x) + 1) >> 1 )
#define DIV2_ROUNDUP(x)  ( ((x) + 1) >> 1 )
#define DIV2_TRUNC(x)    ( (x) >> 1 )

#define DIV4_ROUND(x)    ( ((x) + 2) >> 2 )
#define DIV4_ROUNDUP(x)  ( ((x) + 3) >> 2 )
#define DIV4_TRUNC(x)    ( (x) >> 2 )

#define DIV8_ROUND(x)    ( ((x) + 4) >> 3 )
#define DIV8_ROUNDUP(x)  ( ((x) + 7) >> 3 )
#define DIV8_TRUNC(x)    ( (x) >> 3 )

#define DIV16_ROUND(x)   ( ((x) + 8) >> 4 )
#define DIV16_ROUNDUP(x) ( ((x) + 15) >> 4 )
#define DIV16_TRUNC(x)   ( (x) >> 4 )

#define DIV32_ROUNDUP(x) ( ((x) + 31) >> 5 )

#define MULT16(x)       ( (x) << 4 )
#define MULT8(x)        ( (x) << 3 )


/************************************************************************
* Function: EpmCalcStripeBufferSize
*
* Actions: find striped format buffer size
*
* Params:
*        PictureWidthInMb, PictureHeightInMbs: picture width and height in mbs
*        IsDcxvBuff
*
* Returns:
*        buffer size
*
************************************************************************/
static uint32_t BVDC_P_VIP_CalcStripeBufferSize(
    uint32_t PictureWidthInPels ,
    uint32_t PictureHeightInPels ,
    uint32_t IsDcxvBuf ,
    uint32_t IsInterlace ,
    uint32_t StripeWidth ,
    uint32_t X , uint32_t Y,
    uint32_t PageSize )
{

    uint32_t BufferWidthInPels , BuffereHeightInPels;
    uint32_t BuffSize;
    uint32_t DcxvPaddingWidth , DcxvPaddingHeight;
    uint32_t Nmby;

    DcxvPaddingWidth = (( PictureWidthInPels + DCXV_STRIPE_WIDTH - 1)/DCXV_STRIPE_WIDTH) * DCXV_STRIPE_WIDTH - PictureWidthInPels;

    if ( IsInterlace == 1 )
    {
            DcxvPaddingHeight = DCXV_PADDED_LINE *2;
    }
    else
    {
            DcxvPaddingHeight = DCXV_PADDED_LINE;
    }

    if ( IsDcxvBuf == 1 )
    {
      BufferWidthInPels = PictureWidthInPels + DcxvPaddingWidth;
      BufferWidthInPels = BufferWidthInPels / DCXV_COMP_RATIO;
      BuffereHeightInPels = PictureHeightInPels + DcxvPaddingHeight;
    }
    else
    {
      BufferWidthInPels = PictureWidthInPels;
      BuffereHeightInPels = PictureHeightInPels;
    }
    Nmby = BVDC_P_VIP_CalcNMBY(DIV16_ROUNDUP(BuffereHeightInPels) , X , Y);

    BuffSize = BVDC_P_VIP_AlignJword( BVDC_P_VIP_CalcWidthInStripes(BufferWidthInPels,StripeWidth) * StripeWidth * MULT16(Nmby) );
    BuffSize = ((BuffSize+PageSize-1) & (~(PageSize-1)));

    return(BuffSize);
}

/************************************************************************
* Function: BVDC_P_Vip_CalcMemAlloc
*
* Calculates the amount of device memory that is necessary for VIP capture
* to operate depending on various input parameters.
*
* Params:
*       InputType               - Input Type: ENCODER_INPUT_TYPE_INTERLACED or ENCODER_INPUT_TYPE_PROGRESSIVE
*       DramStripeWidth         - Stripe Width in bytes
*       MaxPictureWidthInPels   - Max picture H resolution allowed
*       MaxPictureHeightInPels  - Max picture V resolution allowed
*
* Returns:
*        Number of macroblocks per stripe height
*
************************************************************************/
uint32_t BVDC_P_MemConfig_GetVipBufSizes ( const BVDC_P_VipMemSettings *pstMemSettings, BVDC_P_VipMemConfig *pstMemConfig )
{

    uint32_t DramStripeWidth;
    uint32_t X, Y, PageSize;
    bool     bInterlaced;
    uint32_t DcxvEnable;
    uint32_t MaxPictureWidthInPels;
    uint32_t MaxPictureHeightInPels;
    uint8_t  StackPointer;
    uint32_t BufferSize;
    uint32_t CurrAddress;

    uint32_t max_horizontal_size_in_pels;
    uint32_t max_vertical_size_in_pels;

    uint32_t size_of_luma_buffer;
    uint32_t size_of_420_chroma_buffer;
#if BVDC_P_VIP_SUPPORT_INTERLACED
    uint32_t size_of_shifted_422_chroma_buffer=0;
#endif
#if BVDC_P_VIP_SUPPORT_DECIMBUFFER
    uint32_t size_of_2h1v_luma_buffer;
    uint32_t size_of_2h2v_luma_buffer;
#endif

    BKNI_Memset(pstMemConfig, 0, sizeof(BVDC_P_VipMemConfig));
    DramStripeWidth = pstMemSettings->DramStripeWidth;
    X = pstMemSettings->X;
    Y = pstMemSettings->Y;
    PageSize = pstMemSettings->PageSize;
    bInterlaced = pstMemSettings->bInterlaced;
    MaxPictureWidthInPels = pstMemSettings->MaxPictureWidthInPels;
    MaxPictureHeightInPels = pstMemSettings->MaxPictureHeightInPels;
    DcxvEnable = pstMemSettings->DcxvEnable;
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("DramStripeWidth: %u", DramStripeWidth));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("              X: %u", X));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("              Y: %u", Y));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("       PageSize: %u", PageSize));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("    bInterlaced: %u", bInterlaced));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("          Width: %u", MaxPictureWidthInPels));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("         Height: %u", MaxPictureHeightInPels));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("           DCXV: %u", DcxvEnable));

    /* Initialize BVDC_P_VIP stack of buffers */
    CurrAddress = 0;

    max_horizontal_size_in_pels = MaxPictureWidthInPels;
    max_vertical_size_in_pels   = MaxPictureHeightInPels;

    /*DCXV compression is used for luma and chroma in the progressive case in VIP */
    size_of_luma_buffer = BVDC_P_VIP_CalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , DcxvEnable , bInterlaced , DramStripeWidth , X , Y, PageSize);
    size_of_420_chroma_buffer = BVDC_P_VIP_CalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels), false , bInterlaced , DramStripeWidth , X , Y, PageSize);

    /* TODO: may reduce if no B-picture required */
    pstMemConfig->ulNumOrigBuf           = BVDC_P_VIP_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES;
    pstMemConfig->ulLumaBufSize          = size_of_luma_buffer;
    pstMemConfig->ulChromaBufSize        = size_of_420_chroma_buffer;
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("# Original Buffers : %u", pstMemConfig->ulNumOrigBuf));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("Luma Buffer size   : %u", pstMemConfig->ulLumaBufSize));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("Chroma Buffer size : %u", pstMemConfig->ulChromaBufSize));

    /* 1) Allocate Stack of buffers for the Original picture */
    BufferSize=size_of_luma_buffer+size_of_420_chroma_buffer;

    for (StackPointer=0; StackPointer<(BVDC_P_VIP_MAX_NUMBER_OF_IBBP_ORIGINAL_PICTURE_QUEUES << (1+bInterlaced)); StackPointer+=(1+bInterlaced))
    {
        CurrAddress=CurrAddress+BufferSize;
    }

    /*2) Allocate Stack of buffers for the Original Shifted 4:2:2 top field Chroma for interlaced */
#if BVDC_P_VIP_SUPPORT_INTERLACED
    size_of_shifted_422_chroma_buffer = bInterlaced? size_of_420_chroma_buffer : 0;
    pstMemConfig->ulShiftedChromaBufSize = size_of_shifted_422_chroma_buffer;
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("Shifted CrCb BufSize: %u", pstMemConfig->ulShiftedChromaBufSize));
    BufferSize=size_of_shifted_422_chroma_buffer;
    for (StackPointer=0; StackPointer<(BVDC_P_VIP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES); StackPointer+=1)
    {
        CurrAddress=CurrAddress+BufferSize;
    }
#endif

    /*3) Allocate stack of buffers for the Decimated picture*/
#if BVDC_P_VIP_SUPPORT_DECIMBUFFER
    size_of_2h1v_luma_buffer = BVDC_P_VIP_CalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , max_vertical_size_in_pels , NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
    size_of_2h2v_luma_buffer = BVDC_P_VIP_CalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
    pstMemConfig->ul2H1VBufSize          = size_of_2h1v_luma_buffer;
    pstMemConfig->ul2H2VBufSize          = size_of_2h2v_luma_buffer;
    pstMemConfig->ulNumDecimBuf          = BVDC_P_VIP_MAX_NUMBER_OF_IBBP_DECIMATED_PICTURE_QUEUES;

    BDBG_MODULE_MSG(BVDC_VIP_MEM,("# Decimate Buffers : %u", pstMemConfig->ulNumDecimBuf));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("ul2H1VBufSize      : %u", pstMemConfig->ul2H1VBufSize));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("ul2H2VBufSize      : %u", pstMemConfig->ul2H2VBufSize));

    BufferSize=size_of_2h1v_luma_buffer+size_of_2h2v_luma_buffer;
    for (StackPointer=0; StackPointer<(BVDC_P_VIP_MAX_NUMBER_OF_IBBP_DECIMATED_PICTURE_QUEUES << (1+bInterlaced)); StackPointer+=(1+bInterlaced))
    {
        CurrAddress=CurrAddress+BufferSize;
    }
#endif

    pstMemConfig->ulTotalSize = CurrAddress;
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("Total size = %#x", pstMemConfig->ulTotalSize));
    return(CurrAddress);

}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_Vip_AllocBuffer
    ( BVDC_P_Vip_Handle            hVip,
      BVDC_Display_Handle          hDisplay )
{
    BERR_Code rc = BERR_SUCCESS;
    BVDC_P_VipBufferNode *pBuffer;
    uint32_t ulBlockOffset = 0;
    BVDC_VipMemConfigSettings *pVipMemSettings = &hDisplay->stNewInfo.stVipMemSettings;

    /* acquire VIP power first */
#ifdef BCHP_PWR_RESOURCE_VIP
    BDBG_MSG(("Disp[%u]: Acquire BCHP_PWR_RESOURCE_VIP", hDisplay->eId));
    rc = BCHP_PWR_AcquireResource(hDisplay->hVdc->hChip, BCHP_PWR_RESOURCE_VIP);
    if(BERR_SUCCESS != rc) {
        return BERR_TRACE(rc);
    }
#ifdef BCHP_PWR_RESOURCE_VIP_SRAM
    rc = BCHP_PWR_AcquireResource(hDisplay->hVdc->hChip, BCHP_PWR_RESOURCE_VIP_SRAM);
    if(BERR_SUCCESS != rc) {
        return BERR_TRACE(rc);
    }
#endif
#endif

    /* compute the memory allocation */
    hVip->stMemSettings.DcxvEnable = false;
    hVip->stMemSettings.bInterlaced            = pVipMemSettings->bSupportInterlaced;
    hVip->stMemSettings.MaxPictureHeightInPels = pVipMemSettings->ulMaxHeight;
    hVip->stMemSettings.MaxPictureWidthInPels  = pVipMemSettings->ulMaxWidth;
    BDBG_ASSERT(pVipMemSettings->ulMemcId < 3);
    hVip->stMemSettings.PageSize        = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulPageSize;
    hVip->stMemSettings.DramStripeWidth = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulStripeWidth;
    hVip->stMemSettings.X = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulMbMultiplier;
    hVip->stMemSettings.Y = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulMbRemainder;
    BVDC_P_MemConfig_GetVipBufSizes(&hVip->stMemSettings, &hVip->stMemConfig);

    BDBG_MODULE_MSG(BVDC_VIP_MEM,("VIP[%d] alloc %d bytes picture buffers on memc%u..", hVip->eId, hVip->stMemConfig.ulTotalSize, pVipMemSettings->ulMemcId));

    /* Allocate picture buffers for VIP freeQ. TODO: add ITFP, B-picture and interlaced support. */
    hVip->hBlock = BMMA_Alloc(hDisplay->stNewInfo.hVipHeap, hVip->stMemConfig.ulTotalSize, hVip->stMemSettings.PageSize, NULL);
    if(hVip->hBlock==NULL) {
        BDBG_ERR(("Failed to allocate MMA block for display[%d]'s VIP[%d]", hDisplay->eId, hVip->eId));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    hVip->ullDeviceOffset = BMMA_LockOffset(hVip->hBlock);
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("starting offset = "BDBG_UINT64_FMT, BDBG_UINT64_ARG(hVip->ullDeviceOffset)));
    for(pBuffer = BLST_SQ_FIRST(&hVip->stFreeQ); pBuffer;  pBuffer= BLST_SQ_NEXT(pBuffer, link)) {
        BKNI_Memset(&pBuffer->stPicture, 0, sizeof(BAVC_EncodePictureBuffer));
        pBuffer->stPicture.hLumaBlock   = hVip->hBlock;
        pBuffer->stPicture.ulLumaOffset = ulBlockOffset;
        ulBlockOffset += hVip->stMemConfig.ulLumaBufSize;

        pBuffer->stPicture.hChromaBlock   = hVip->hBlock;
        pBuffer->stPicture.ulChromaOffset = ulBlockOffset;
        ulBlockOffset += hVip->stMemConfig.ulChromaBufSize;
#if BVDC_P_VIP_SUPPORT_DECIMBUFFER
        pBuffer->stPicture.h2H1VLumaBlock   = hVip->hBlock;
        pBuffer->stPicture.ul2H1VLumaOffset = ulBlockOffset;
        ulBlockOffset += hVip->stMemConfig.ul2H1VBufSize;

        pBuffer->stPicture.h2H2VLumaBlock   = hVip->hBlock;
        pBuffer->stPicture.ul2H2VLumaOffset = ulBlockOffset;
        ulBlockOffset += hVip->stMemConfig.ul2H2VBufSize;
#endif
    }

#if BVDC_P_DUMP_VIP_PICTURE
    hVip->pY = hVip->pC = BMMA_Lock(hVip->hBlock);
    hVip->pfY = fopen("videos/vipY0.img", "wb");
    if(!hVip->pfY) {
        BDBG_ERR(("Failed to open videos/vipY.img to dump vip luma capture picture"));
    }
    hVip->pfC = fopen("videos/vipC0.img", "wb");
    if(!hVip->pfC) {
        BDBG_ERR(("Failed to open videos/vipC.img to dump vip chroma capture picture"));
    }
    {
        const char *pValue = getenv("BVDC_VipCapNum");
        if(pValue) {
            hVip->numPicsToCapture = atoi(pValue);
            BDBG_WRN(("User requests to capture %u consecutive VIP pictures.", hVip->numPicsToCapture));
        } else {
            hVip->numPicsToCapture = 1;
        }
    }
#endif
    /* link VIP with display: make sure protected from interrupt */
    BKNI_EnterCriticalSection();
    hVip->hDisplay = hDisplay;
    hDisplay->hVip = hVip;
    BKNI_LeaveCriticalSection();
    return rc;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_Vip_FreeBuffer
    ( BVDC_P_Vip_Handle            hVip )
{
    BVDC_P_VipBufferNode *pBuffer;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_MODULE_MSG(BVDC_DISP_VIP,("VIP[%d] frees buffers", hVip->eId));
    /* release VIP power first */
#ifdef BCHP_PWR_RESOURCE_VIP
    BDBG_MSG(("Disp[%u]: releases BCHP_PWR_RESOURCE_VIP", hVip->eId));
    rc = BCHP_PWR_ReleaseResource(hVip->hDisplay->hVdc->hChip, BCHP_PWR_RESOURCE_VIP);
    if(BERR_SUCCESS != rc) {
        return BERR_TRACE(rc);
    }
#ifdef BCHP_PWR_RESOURCE_VIP_SRAM
    rc = BCHP_PWR_ReleaseResource(hVip->hDisplay->hVdc->hChip, BCHP_PWR_RESOURCE_VIP_SRAM);
    if(BERR_SUCCESS != rc) {
        return BERR_TRACE(rc);
    }
#endif
#endif

    /* unlink VIP with display first */
    BKNI_EnterCriticalSection();
    hVip->hDisplay->hVip = NULL;
    hVip->hDisplay       = NULL;
    BKNI_LeaveCriticalSection();

    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stCaptureQ))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("VIP[%d] releases captureQ buf[%d] %p, pNext=%p", hVip->eId, pBuffer->ulBufferId, (void *)pBuffer, (void *)BLST_SQ_NEXT(pBuffer, link)));
        BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQ, link);
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, pBuffer, link);
    }
    /* TODO: must make sure encoder releasing the delivered pictures before unlock/free the mma allocation */
    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stDeliverQ))){
        BDBG_ERR(("VIP[%d] releases deliveryQ buf[%d]", hVip->eId, pBuffer->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stDeliverQ, link);
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, pBuffer, link);
    }
    /* free the intermediate pToCapture and pCapture buffers */
    if(hVip->pCapture) {
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, hVip->pCapture, link);
        hVip->pCapture = NULL;
    }
    if(hVip->pToCapture) {
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, hVip->pToCapture, link);
        hVip->pToCapture = NULL;
    }
    /* free picture buffers for VIP queues */
    pBuffer = BLST_SQ_FIRST(&hVip->stFreeQ);
    BMMA_UnlockOffset(pBuffer->stPicture.hLumaBlock, hVip->ullDeviceOffset);
    BMMA_Free(hVip->hBlock);
    hVip->hBlock = NULL;
#if BVDC_P_DUMP_VIP_PICTURE
    BMMA_Unlock(hVip->hBlock, hVip->pY);
    if(hVip->pfY) {
        fclose(hVip->pfY);
    }
    if(hVip->pfC) {
        fclose(hVip->pfC);
    }
#endif

    return rc;
}

/***************************************************************************
 *
 */
void BVDC_P_Vip_GetBuffer_isr
    ( BVDC_P_Vip_Handle         hVip,
      BAVC_EncodePictureBuffer *pPicture )
{
    BVDC_P_VipBufferNode *pBuffer;

    BDBG_ASSERT(pPicture);
    BKNI_Memset_isr((void*)pPicture, 0, sizeof(BAVC_EncodePictureBuffer));
    pBuffer = BLST_SQ_FIRST(&hVip->stCaptureQ);
    /* NOTE: delay one picture to avoid tearing */
    if(pBuffer) { /* move from captureQ to deliverQ */
        BKNI_Memcpy_isr((void*)pPicture, (void*)&pBuffer->stPicture, sizeof(BAVC_EncodePictureBuffer));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("[%d] GET pic[%u] buf[%d]: %ux%u%c[stc=%#x, %#x]", hVip->eId, pPicture->ulPictureId, pBuffer->ulBufferId,
            pPicture->ulWidth, pPicture->ulHeight, (BAVC_Polarity_eFrame==pPicture->ePolarity)?'p':'i',
            pPicture->ulSTCSnapshotHi, pPicture->ulSTCSnapshotLo));
        BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQ, link);
        BLST_SQ_INSERT_TAIL(&hVip->stDeliverQ, pBuffer, link);
#if BVDC_P_DUMP_VIP_PICTURE
        if(!hVip->bDumped && pPicture->ulPictureId > 0) {/* skip initial 30 pictures in case mute */
            void *pY, *pC;
            uint32_t totalStripes = (pPicture->ulWidth + pPicture->ulStripeWidth - 1) / pPicture->ulStripeWidth;
            uint32_t ulLumaBufSize = pPicture->ulLumaNMBY * totalStripes * pPicture->ulStripeWidth * 16;
            uint32_t ulChromaBufSize = pPicture->ulChromaNMBY * totalStripes * pPicture->ulStripeWidth * 16;
            char path[40];

            hVip->bDumped = (++hVip->dumpCnt) >= hVip->numPicsToCapture;/* only capture ten */
            pY = (void*)((uint32_t)hVip->pY + pPicture->ulLumaOffset);
            pC = (void*)((uint32_t)hVip->pC + pPicture->ulChromaOffset);
            BMMA_FlushCache_isr(pPicture->hLumaBlock, pY, ulLumaBufSize);
            BMMA_FlushCache_isr(pPicture->hChromaBlock, pC, ulChromaBufSize);

            BDBG_MSG(("stripeWidth = %u", pPicture->ulStripeWidth));
            BDBG_MSG(("yNMBY       = %u", pPicture->ulLumaNMBY));
            BDBG_MSG(("cNMBY       = %u", pPicture->ulChromaNMBY));
            BDBG_MSG(("format      = %ux%u%c", pPicture->ulWidth, pPicture->ulHeight, (BAVC_Polarity_eFrame==pPicture->ePolarity)?'p':'i'));
            BDBG_MSG(("luma pointer   = %p, offset = %#x, size = %u", pY, pPicture->ulLumaOffset, ulLumaBufSize));
            BDBG_MSG(("chroma pointer = %p, offset = %#x, size = %u", pC, pPicture->ulChromaOffset, ulChromaBufSize));
            if(hVip->pfY) {
                fwrite(pY, 1, ulLumaBufSize, hVip->pfY);
                fclose(hVip->pfY);
                if(!hVip->bDumped) {
                    BKNI_Snprintf(path, 40, "videos/vipY%u.img", hVip->dumpCnt);
                    hVip->pfY = fopen(path, "wb");
                } else {
                    hVip->pfY = NULL;
                }
            }
            if(hVip->pfC) {
                fwrite(pC, 1, ulChromaBufSize, hVip->pfC);
                fclose(hVip->pfC);
                if(!hVip->bDumped) {
                    BKNI_Snprintf(path, 40, "videos/vipC%u.img", hVip->dumpCnt);
                    hVip->pfC = fopen(path, "wb");
                } else {
                    hVip->pfC = NULL;
                }
            }
        }
#endif
    } else
    {
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("VIP[%d] GET NULL", hVip->eId));
    }
}

/***************************************************************************
 *
 */
void BVDC_P_Vip_ReturnBuffer_isr
    ( BVDC_P_Vip_Handle               hVip,
      const BAVC_EncodePictureBuffer *pPicture )
{
    BVDC_P_VipBufferNode *pBuffer;

    BDBG_ASSERT(pPicture);

    /* find the matching node */
    for(pBuffer = BLST_SQ_FIRST(&hVip->stDeliverQ); pBuffer; pBuffer= BLST_SQ_NEXT(pBuffer, link)) {
        if(pBuffer->stPicture.hLumaBlock == pPicture->hLumaBlock &&
           pBuffer->stPicture.ulPictureId == pPicture->ulPictureId) {
            BDBG_MODULE_MSG(BVDC_DISP_VIP,("[%d] RET pic[%u] buf[%d]..", hVip->eId, pPicture->ulPictureId, pBuffer->ulBufferId));
            /* move from deliverQ to freeQ */
            BLST_SQ_REMOVE(&hVip->stDeliverQ, pBuffer, BVDC_P_VipBufferNode, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, pBuffer, link);
            return;
        }
    }
    BDBG_MODULE_WRN(BVDC_DISP_VIP,("[%d] RET a pic[%u] not found", hVip->eId, pPicture->ulPictureId));
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Vip_Create
    ( BVDC_P_Vip_Handle           *phVip,
      unsigned                     id,
      BVDC_Handle                  hVdc)
{
    unsigned i;
    BVDC_P_VipContext *pVip;
    BVDC_P_VipBufferNode *pBuffer;

    BDBG_ENTER(BVDC_P_Vip_Create);

    BDBG_ASSERT(phVip);
    BDBG_ASSERT(hVdc);

    /* initialize bchp memory info */
    if(hVdc->stMemInfo.memc[0].size == 0) {
        BCHP_GetMemoryInfo(hVdc->hChip, &hVdc->stMemInfo);
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("Get chip memory info:"));
        for(i=0; i<3; i++) {
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("memc[%u]:", i));
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("    StripeWidth = %u", hVdc->stMemInfo.memc[i].ulStripeWidth));
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("       PageSize = %u", hVdc->stMemInfo.memc[i].ulPageSize));
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("              X = %u", hVdc->stMemInfo.memc[i].ulMbMultiplier));
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("              Y = %u", hVdc->stMemInfo.memc[i].ulMbRemainder));
        }
    }

    /* The handle will be NULL if create fails. */
    *phVip = NULL;

    /* Alloc the context. */
    pVip = (BVDC_P_VipContext*)
        (BKNI_Malloc(sizeof(BVDC_P_VipContext)));
    if(!pVip)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pVip, 0x0, sizeof(BVDC_P_VipContext));
    BDBG_OBJECT_SET(pVip, BVDC_VIP);

    pVip->eId                  = id;

    switch(id)
    {
    case 0:
        pVip->ulRegOffset = 0;
        break;
#if BCHP_VICE2_VIP1_0_0_REG_START
    case 1:
        pVip->ulRegOffset = BCHP_VICE2_VIP1_0_0_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP_0_1_REG_START
    case 2:
        pVip->ulRegOffset = BCHP_VICE2_VIP_0_1_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP1_0_1_REG_START
    case 3:
        pVip->ulRegOffset = BCHP_VICE2_VIP1_0_1_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP2_0_0_REG_START
    case 4:
        pVip->ulRegOffset = BCHP_VICE2_VIP2_0_0_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP2_0_1_REG_START
    case 5:
        pVip->ulRegOffset = BCHP_VICE2_VIP2_0_1_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#endif

    default:
        BDBG_ASSERT(0);
    }
#if 0
    /* Vip reset address */
    pVip->ulResetRegAddr = BCHP_SUN_TOP_CTRL_SW_INIT_0;
    pVip->ulResetMask    = BCHP_SUN_TOP_CTRL_SW_INIT_0_VICE2_VIP_0_0_MASK << (pVip->eId);
#endif

    /* default VIP mem config */
    pVip->stMemSettings.bInterlaced = false;
    pVip->stMemSettings.MaxPictureHeightInPels = 480;
    pVip->stMemSettings.MaxPictureWidthInPels  = 720;

    /* initialize freeQ and deliverQ; all nodes in freeQ */
    BLST_SQ_INIT(&pVip->stFreeQ);
    BLST_SQ_INIT(&pVip->stCaptureQ);
    BLST_SQ_INIT(&pVip->stDeliverQ);
    for(i=0; i<BVDC_P_VIP_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES; i++){
        pBuffer = BKNI_Malloc(sizeof(BVDC_P_VipBufferNode));
        BDBG_ASSERT(pBuffer);
        pBuffer->ulBufferId = i;
        BLST_SQ_INSERT_TAIL(&pVip->stFreeQ, pBuffer, link);
    }
    BDBG_MODULE_MSG(BVDC_DISP_VIP,("Created VIP%u", pVip->eId));
    /* All done. now return the new fresh context to user. */
    *phVip = (BVDC_P_Vip_Handle)pVip;

    BDBG_LEAVE(BVDC_P_Vip_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vip_Destroy
    ( BVDC_P_Vip_Handle            hVip )
{
    BVDC_P_VipBufferNode *pBuffer;

    BDBG_ENTER(BVDC_P_Vip_Destroy);
    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);

    BDBG_MODULE_MSG(BVDC_DISP_VIP,("To destroy VIP%u", hVip->eId));
    /* release freeQ and deliverQ */
    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stFreeQ))){
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("VIP[%d] freeQ buffer[%d] to remove", hVip->eId, pBuffer->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQ, link);
        BKNI_Free(pBuffer);
    }
    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stCaptureQ))){
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("VIP[%d] captureQ buffer[%d] to remove", hVip->eId, pBuffer->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQ, link);
        BKNI_Free(pBuffer);
    }
    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stDeliverQ))){
        BDBG_ERR(("VIP[%d] delivery Q buffer[%d] not returned", hVip->eId, pBuffer->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stDeliverQ, link);
        BKNI_Free(pBuffer);
    }

    BDBG_OBJECT_DESTROY(hVip, BVDC_VIP);
    /* Release context in system memory */
    BKNI_Free((void*)hVip);

    BDBG_LEAVE(BVDC_P_Vip_Destroy);
    return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vip_Init
    ( BVDC_P_Vip_Handle            hVip )
{
    BDBG_ENTER(BVDC_P_Vip_Init);
    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);

    /* Clear out shadow registers. */
    BKNI_Memset((void*)&hVip->stRegs, 0x0, sizeof(BVDC_P_VipRegisterSetting));

    hVip->stRegs.ulFwControl = (
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, DONE_RD_HIST, DONE) |
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, DONE_RD_PCC,  DONE) |
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, PIC_START,    NULL) |
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, DONE_WR_REG,  NULL) );

    hVip->stRegs.ulConfig = (
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, ENABLE_CTRL, ENABLE_BY_PICTURE) |
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, AUTO_SHUTOFF,           ENABLE) |
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, PADDING_MODE,           ENABLE) |
#if BCHP_MASK(VICE2_VIP_0_0_CONFIG,   ADDRESSING_MODE)
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, ADDRESSING_MODE,        STRIPE) |
#endif
#if BCHP_MASK(VICE2_VIP_0_0_CONFIG,   DRAM_STRIPE_WIDTH)
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, DRAM_STRIPE_WIDTH,   BYTES_128) |
#endif
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, CHROMA_420,             ENABLE) |
        BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, LUMA,                   ENABLE) );

    /* Initialize state. */
    hVip->bInitial = true;
    hVip->eVipDataMode = BVDC_P_VipDataMode_eStripe;

#if BCHP_VICE2_VIP_0_0_DCXV_CFG
    if(hVip->stMemSettings.DcxvEnable)
    {
        /* VICE2_VIP_0_0_DCXV_CFG */
        hVip->stRegs.ulDcxvCfg =  (
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_DCXV_CFG, MODE,     COMPRESS) |
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_DCXV_CFG, PREDICTION,    AVG) |
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_DCXV_CFG, LINE_STORE, BITS_6));
    }
#endif
    BDBG_LEAVE(BVDC_P_Vip_Init);
    return;
}

#include "bchp_xpt_pcroffset.h"
/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Vip_BuildRul_isr
    ( const BVDC_P_Vip_Handle      hVip,
      BVDC_P_ListInfo             *pList,
      BAVC_Polarity                eFieldPolarity )
{
    BVDC_P_VipBufferNode *pBuffer;
    const BFMT_VideoInfo *pFmtInfo;
    BVDC_Display_Handle hDisplay = hVip->hDisplay;

    BDBG_ENTER(BVDC_P_Vip_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);

    pFmtInfo = (hDisplay->pStgFmtInfo)?
        hDisplay->pStgFmtInfo : hDisplay->stCurInfo.pFmtInfo;

    /* Two intermediate picture delays.
       Note RUL has a picture delay, so when isr building the next capture picture, the last capture just got triggered!
       and it takes one more picture time to complete the capture! We allow picture delivered only after its capture is done. */
    if(hVip->pCapture) {/* beingCaptured -> Capture Done */
        /* insert the captured picture to the CaptureQ available for GetBuffer API */
        BLST_SQ_INSERT_TAIL(&hVip->stCaptureQ, hVip->pCapture, link);
        hVip->pCapture = NULL; /* clear the pointer */
    }
    /* if last picture to be captured existed, now that last trigger fired this isr, move it to the pCapture */
    if(hVip->pToCapture) {
        /* one picture delay: ToCapture -> beingCaptured */
        hVip->pCapture = hVip->pToCapture;
        hVip->pToCapture = NULL; /* clear the pointer */
        /* read stc snapshot by last trigger and saved to the buffer! */
        if(hDisplay->stCurInfo.ulStcSnapshotLoAddr && hDisplay->stCurInfo.ulStcSnapshotHiAddr) {
            /* 20150123 must read HI first to latch LO */
            hVip->pCapture->stPicture.ulSTCSnapshotHi = BREG_Read32_isr(hDisplay->hVdc->hRegister, hDisplay->stCurInfo.ulStcSnapshotHiAddr);
            hVip->pCapture->stPicture.ulSTCSnapshotLo = BREG_Read32_isr(hDisplay->hVdc->hRegister, hDisplay->stCurInfo.ulStcSnapshotLoAddr);
        }
    }

    /* Get buffer from free Q (fifo) first */
    pBuffer = BLST_SQ_FIRST(&hVip->stFreeQ);

    /* if STG or VIP is disabled, put VIP back to auto drain mode */
    if(!hDisplay->stCurInfo.bEnableStg || !hDisplay->stCurInfo.hVipHeap) {
        hVip->stRegs.ulConfig  &= ~(
            BCHP_MASK(VICE2_VIP_0_0_CONFIG, DROP_PROCESS_MODE) |
            BCHP_MASK(VICE2_VIP_0_0_CONFIG,       ENABLE_CTRL));
    }
    /* In cases:
        1) not FULL and
        2.1) non-ignore, or
        2.2) previously non-ignore, but dropped due to FULL (this time may be marked as ignore repeat pic);
        capture it to VIP!
    */
    else if(pBuffer && (!hDisplay->hCompositor->bIgnorePicture || hVip->bPrevNonIgnoreDropByFull)) {
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("VIP[%d] has free buf[%d]..ign=%d, prevNIDBF=%d",
            hVip->eId, pBuffer->ulBufferId, hDisplay->hCompositor->bIgnorePicture, hVip->bPrevNonIgnoreDropByFull));

        /* if previously dropped due to buffer FULL, this time marked as ignore, clear the flag, capture the repeated pic */
        if(hVip->bPrevNonIgnoreDropByFull) {
            hVip->bPrevNonIgnoreDropByFull = false;
        }
        /* populate the buffer info. */
        pBuffer->stPicture.ulWidth = pFmtInfo->ulDigitalWidth;
        pBuffer->stPicture.ulHeight= pFmtInfo->ulDigitalHeight>>(pFmtInfo->bInterlaced);
        pBuffer->stPicture.ulOriginalPTS = hDisplay->hCompositor->ulOrigPTS;
        pBuffer->stPicture.ePolarity  = BAVC_Polarity_eFrame/* TODO: support interlaced! eFieldPolarity*/;
        switch(hDisplay->stCurInfo.ulVertFreq) {
        case 12000: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e120; break;
        case 11988: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e119_88; break;
        case 10000: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e100; break;
        case 6000: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e60; break;
        case 5994: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e59_94; break;
        case 5000: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e50; break;
        case 3000: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e30; break;
        case 2997: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e29_97; break;
        case 2500: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e25; break;
        case 2400: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e24; break;
        case 2398: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e23_976; break;
        case 2397: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e23_976; break;
        case 2000: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e20; break;
        case 1998: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e19_98; break;
        case 1500: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e15; break;
        case 1498: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e14_985; break;
        case 1250: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e12_5; break;
        case 1200: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e12; break;
        case 1199: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e11_988; break;
        case 1198: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e11_988; break;
        case 1000: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e10; break;
        case  999: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e9_99; break;
        case  750: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e7_5; break;
        case  749: pBuffer->stPicture.eFrameRate= BAVC_FrameRateCode_e7_493; break;
        default:   BDBG_ERR(("unsupported frame rate %d", hVip->hDisplay->stCurInfo.ulVertFreq));
        }

        pBuffer->stPicture.ulAspectRatioX = hDisplay->hCompositor->ulStgPxlAspRatio_x_y >> 16;
        pBuffer->stPicture.ulAspectRatioY = hDisplay->hCompositor->ulStgPxlAspRatio_x_y & 0xFFFF;
        pBuffer->stPicture.ulPictureId = hDisplay->hCompositor->ulPicId;

        /* TODO: handle linear; interlaced and DCXV buffer */
        pBuffer->stPicture.bStriped = (hVip->eVipDataMode == BVDC_P_VipDataMode_eStripe);
        pBuffer->stPicture.ulStripeWidth = hVip->stMemSettings.DramStripeWidth;
        pBuffer->stPicture.ulLumaNMBY = BVDC_P_VIP_CalcNMBY_isr(DIV16_ROUNDUP(pBuffer->stPicture.ulHeight), hVip->stMemSettings.X, hVip->stMemSettings.Y);
        pBuffer->stPicture.ulChromaNMBY = BVDC_P_VIP_CalcNMBY_isr(DIV32_ROUNDUP(pBuffer->stPicture.ulHeight), hVip->stMemSettings.X, hVip->stMemSettings.Y);

        /* move from freeQ to intermediate pToCapture pointer */
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQ, link);
        hVip->pToCapture = pBuffer;

        BDBG_MODULE_MSG(BVDC_DISP_VIP,("VIP display %d stg_id %d:", hDisplay->eId, hVip->eId));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("%dx%d%c%d", pBuffer->stPicture.ulWidth, pBuffer->stPicture.ulHeight,
            (pBuffer->stPicture.ePolarity==BAVC_Polarity_eFrame)? 'p' : 'i', hDisplay->stCurInfo.ulVertFreq));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("stall STC? %d", hDisplay->hCompositor->bStallStc));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("repeat? %d", hDisplay->hCompositor->bPictureRepeatFlag));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("PicId %d DecodePicId %d type %d, pol %d, asp ratio 0x%x, origPTS 0x%x",
            pBuffer->stPicture.ulPictureId, hDisplay->hCompositor->ulDecodePictureId,
            hDisplay->hCompositor->ePictureType, eFieldPolarity,
            hDisplay->hCompositor->ulStgPxlAspRatio_x_y, pBuffer->stPicture.ulOriginalPTS));

        /* set FW_CONTROL */
        hVip->stRegs.ulFwControl = (
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, PIC_START,  START) |
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, DONE_WR_REG, DONE) );

        /* set VIP config. TODO: add decimated luma, interlaced and itfp support */
        hVip->stRegs.ulConfig &= ~(
            BCHP_MASK(VICE2_VIP_0_0_CONFIG,       ENABLE_CTRL) |
#if BCHP_MASK(VICE2_VIP_0_0_CONFIG,   DRAM_STRIPE_WIDTH)
            BCHP_MASK(VICE2_VIP_0_0_CONFIG, DRAM_STRIPE_WIDTH) |
#endif
            BCHP_MASK(VICE2_VIP_0_0_CONFIG, DROP_PROCESS_MODE) |
#if BCHP_MASK(VICE2_VIP_0_0_CONFIG,   ADDRESSING_MODE)
            BCHP_MASK(VICE2_VIP_0_0_CONFIG,   ADDRESSING_MODE) |
#endif
            BCHP_MASK(VICE2_VIP_0_0_CONFIG,        INPUT_TYPE) );

        hVip->stRegs.ulConfig |= (
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, ENABLE_CTRL,      ENABLE_BY_PICTURE) |
#if BCHP_MASK(VICE2_VIP_0_0_CONFIG,   DRAM_STRIPE_WIDTH)
            BCHP_FIELD_DATA(VICE2_VIP_0_0_CONFIG, DRAM_STRIPE_WIDTH, (pBuffer->stPicture.ulStripeWidth==128)) |
#endif
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, DROP_PROCESS_MODE,          PROCESS) |
#if BCHP_MASK(VICE2_VIP_0_0_CONFIG,   ADDRESSING_MODE)
            BCHP_FIELD_DATA(VICE2_VIP_0_0_CONFIG, ADDRESSING_MODE,  (uint32_t)hVip->eVipDataMode) |
#endif
            BCHP_FIELD_DATA(VICE2_VIP_0_0_CONFIG, INPUT_TYPE,        eFieldPolarity) );

        /* set VIP input pixel size */
        hVip->stRegs.ulInputPicSize =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_INPUT_PICTURE_SIZE, HSIZE, pBuffer->stPicture.ulWidth) |
            BCHP_FIELD_DATA(VICE2_VIP_0_0_INPUT_PICTURE_SIZE, VSIZE, pBuffer->stPicture.ulHeight));

        /* set output MB size */
        hVip->stRegs.ulOutputPicSize =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_OUTPUT_PICTURE_SIZE, HSIZE_MB, pBuffer->stPicture.ulWidth/16) |
            BCHP_FIELD_DATA(VICE2_VIP_0_0_OUTPUT_PICTURE_SIZE, VSIZE_MB, pBuffer->stPicture.ulHeight/16));

        /* luma address */
        hVip->stRegs.ullLumaAddr =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_LUMA_ADDR, ADDR, hVip->ullDeviceOffset + pBuffer->stPicture.ulLumaOffset));

        /* luma nmby */
        hVip->stRegs.ulLumaNMBY =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_LUMA_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
            BCHP_FIELD_DATA(VICE2_VIP_0_0_LUMA_NMBY, NMBY, pBuffer->stPicture.ulLumaNMBY));

        /* chroma address */
        hVip->stRegs.ullChromaAddr =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_CHROMA_420_ADDR, ADDR, hVip->ullDeviceOffset + pBuffer->stPicture.ulChromaOffset));

        /* chroma nmby */
        hVip->stRegs.ulChromaNMBY =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_CHROMA_420_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
            BCHP_FIELD_DATA(VICE2_VIP_0_0_CHROMA_420_NMBY, NMBY, pBuffer->stPicture.ulChromaNMBY));

        /* TODO: add shifted chroma for interlaced; decimated luma for motion search */

        /* DCXV config */
#if BCHP_VICE2_VIP_0_0_DCXV_CFG
        hVip->stRegs.ulDcxvCfg =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_DCXV_CFG, MODE, hVip->stMemSettings.DcxvEnable));
#endif

        /* build RUL */
        BDBG_CASSERT(2 == (((BCHP_VICE2_VIP_0_0_OUTPUT_PICTURE_SIZE - BCHP_VICE2_VIP_0_0_INPUT_PICTURE_SIZE) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_VICE2_VIP_0_0_OUTPUT_PICTURE_SIZE - BCHP_VICE2_VIP_0_0_INPUT_PICTURE_SIZE) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VICE2_VIP_0_0_INPUT_PICTURE_SIZE + hVip->ulRegOffset);
        *pList->pulCurrent++ = hVip->stRegs.ulInputPicSize;
        *pList->pulCurrent++ = hVip->stRegs.ulOutputPicSize;

        BVDC_P_VIP_WRITE_TO_RUL(VICE2_VIP_0_0_LUMA_NMBY, pList->pulCurrent,
            hVip->stRegs.ulLumaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(VICE2_VIP_0_0_CHROMA_420_NMBY, pList->pulCurrent,
            hVip->stRegs.ulChromaNMBY);

        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_VICE2_VIP_0_0_LUMA_ADDR + hVip->ulRegOffset,
            hVip->stRegs.ullLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
            BCHP_VICE2_VIP_0_0_CHROMA_420_ADDR + hVip->ulRegOffset,
            hVip->stRegs.ullChromaAddr);

        if(hVip->bInitial) {
            BVDC_P_VIP_WRITE_TO_RUL(VICE2_VIP_0_0_ERR_STATUS_ENABLE, pList->pulCurrent, 0x3FF);

#if BCHP_VICE2_VIP_0_0_DCXV_CFG
            BVDC_P_VIP_WRITE_TO_RUL(VICE2_VIP_0_0_DCXV_CFG, pList->pulCurrent,
                hVip->stRegs.ulDcxvCfg);
#endif
            hVip->bInitial = false;
        }
    } else /* freeQ empty: drop the picture (RT and NRT mode) */
    {
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("VIP[%d] drop pic[pBuf=%p][ign=%d]", hVip->eId, (void *)pBuffer, hDisplay->hCompositor->bIgnorePicture));
        /* set VIP config. drop it */
        hVip->stRegs.ulConfig &= ~(
            BCHP_MASK(VICE2_VIP_0_0_CONFIG,       ENABLE_CTRL) |
            BCHP_MASK(VICE2_VIP_0_0_CONFIG, DROP_PROCESS_MODE) |
            BCHP_MASK(VICE2_VIP_0_0_CONFIG,        INPUT_TYPE) );

        hVip->stRegs.ulConfig |= (
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, ENABLE_CTRL, ENABLE_BY_PICTURE) |
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_CONFIG, DROP_PROCESS_MODE,        DROP) |
            BCHP_FIELD_DATA(VICE2_VIP_0_0_CONFIG, INPUT_TYPE,     eFieldPolarity) );

        /* set FW_CONTROL */
        hVip->stRegs.ulFwControl = (
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, PIC_START,  START) |
            BCHP_FIELD_ENUM(VICE2_VIP_0_0_FW_CONTROL, DONE_WR_REG, DONE) );

        /* set VIP input pixel size */
        hVip->stRegs.ulInputPicSize =  (
            BCHP_FIELD_DATA(VICE2_VIP_0_0_INPUT_PICTURE_SIZE, HSIZE, pFmtInfo->ulDigitalWidth) |
            BCHP_FIELD_DATA(VICE2_VIP_0_0_INPUT_PICTURE_SIZE, VSIZE, (pFmtInfo->ulDigitalHeight>>pFmtInfo->bInterlaced)));

        BVDC_P_VIP_WRITE_TO_RUL(VICE2_VIP_0_0_INPUT_PICTURE_SIZE, pList->pulCurrent,
            hVip->stRegs.ulInputPicSize);

        if(pBuffer == NULL && hDisplay->stCurInfo.bStgNonRealTime) {/* if FULL, stall STC in NRT mode since it's dropped! */
            hDisplay->hCompositor->bStallStc = true;
            /* if dropping non-ignore pic due to buffer FULL, mark it to capture later when DM marks it as ignroe next time! */
            if(!hDisplay->hCompositor->bIgnorePicture) {
                hVip->bPrevNonIgnoreDropByFull = true;
            }
        }
        /* mark current picture as ignore since it's dropped! effect on NRT mode STG trigger mode and RepatPolarity! */
        hDisplay->hCompositor->bIgnorePicture = true;
    }

    BVDC_P_VIP_WRITE_TO_RUL(VICE2_VIP_0_0_CONFIG, pList->pulCurrent, hVip->stRegs.ulConfig);
    BVDC_P_VIP_WRITE_TO_RUL(VICE2_VIP_0_0_FW_CONTROL, pList->pulCurrent, hVip->stRegs.ulFwControl);
    BDBG_LEAVE(BVDC_P_Vip_BuildRul_isr);
    return;
}

#endif

/* End of file */
