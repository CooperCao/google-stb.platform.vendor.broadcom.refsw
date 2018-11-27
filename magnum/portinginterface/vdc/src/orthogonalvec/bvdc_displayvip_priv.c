/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
#include "bavc_vce.h"
/* ==============   MEMORY CALCULATIONS   ================== */

#if 0
#define BVDC_P_VIP_IBBP_NUMBER_OF_ORIGINAL_PICTURE_QUEUES                      (10)
#define BVDC_P_VIP_IBBP_NUMBER_OF_DECIMATED_PICTURE_QUEUES                     (12)

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
#endif

#define HOR_SIZE_IN_PELS_720P                                           1280
#define VER_SIZE_IN_PELS_720P                                           720

#define HOR_SIZE_IN_PELS_1080P                                           1920
#define VER_SIZE_IN_PELS_1080P                                           1088

#define HOR_SIZE_IN_PELS_PAL                                            720
#define VER_SIZE_IN_PELS_PAL                                            576


BDBG_MODULE(BVDC_VIP);
BDBG_FILE_MODULE(BVDC_VIP_MEM);
BDBG_FILE_MODULE(BVDC_DISP_VIP);
BDBG_FILE_MODULE(BVDC_VIP_Q);
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
* Function: BVDC_P_VIP_CalcDcxvNMBY_isr
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

static uint32_t BVDC_P_VIP_CalcDcxvNMBY_isr(uint32_t PictureHeightInMbs, uint32_t IsDcxvBuf , uint32_t IsInterlace , uint32_t IsChroma , uint32_t X, uint32_t Y)
{
    uint32_t Nmby;
    uint32_t DcxvPaddingHeight;
    uint32_t BuffereHeightInPels;
    uint32_t PictureHeightInPels;

    PictureHeightInPels = MULT16(PictureHeightInMbs);
    if (IsChroma == 1)
    PictureHeightInPels = DIV2_ROUNDUP(PictureHeightInPels);


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
      BuffereHeightInPels = PictureHeightInPels + DcxvPaddingHeight;
    }
    else
    {
      BuffereHeightInPels = PictureHeightInPels;
    }

    if (
        (IsInterlace == 1) &&
        (IsChroma == 1)
       )
    {
        BuffereHeightInPels = PictureHeightInPels;
    }

    Nmby = BVDC_P_VIP_CalcNMBY_isr(DIV16_ROUNDUP(BuffereHeightInPels) , X , Y);
    return(Nmby);
}


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

/* Workarounds specific to core 2_1_1_2 (7366) and 2_1_0_3 (7445) */
#if ( BCHP_CHIP == 7445 ) || ( BCHP_CHIP == 7366 )
#define WORKAROUND_HW_ISSUE_VICE2_196
#define WORKAROUND_HW_ISSUE_VICE2_210

#define WORKAROUND_HW_ISSUE_VICE2_210_LOWER_LIMIT 61
#define WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT 64

#define WORKAROUND_HW_ISSUE_VICE2_196_MAX_NON_DCXV_WIDTH 640
#define WORKAROUND_HW_ISSUE_VICE2_196_MAX_NON_DCXV_HEIGHT 480

#define WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_WIDTH 1024
#define WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_HEIGHT 576
#endif

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

    uint32_t Non_dcxv_BufferSize=0;

    uint32_t size_of_luma_buffer;
    uint32_t size_of_420_chroma_buffer;
    uint32_t size_of_shifted_422_chroma_buffer=0;
    uint32_t size_of_2h1v_luma_buffer;
    uint32_t size_of_2h2v_luma_buffer;
    BAVC_VCE_BufferConfig stVceBufferConfig;

    BAVC_VCE_GetDefaultBufferConfig_isrsafe(pstMemSettings->bInterlaced, &stVceBufferConfig );
    stVceBufferConfig.eScanType = pstMemSettings->bInterlaced ? BAVC_ScanType_eInterlaced : BAVC_ScanType_eProgressive;
    if ( false == pstMemSettings->bBframes ) stVceBufferConfig.uiNumberOfBFrames = 0;

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

#if defined(WORKAROUND_HW_ISSUE_VICE2_196) || defined(WORKAROUND_HW_ISSUE_VICE2_210)
    if (DcxvEnable)
    {
        uint32_t max_horizontal_size_in_mbs  = DIV16_ROUNDUP(MaxPictureWidthInPels);

        if (( (max_horizontal_size_in_mbs & 0x3) == 3 ) && ( BCHP_CHIP != 7366 ) )
        {
            max_horizontal_size_in_mbs = max_horizontal_size_in_mbs +1;
            MaxPictureWidthInPels = (max_horizontal_size_in_mbs<<4);
        }

        max_horizontal_size_in_mbs  = DIV16_ROUNDUP(MaxPictureWidthInPels);
        if ((max_horizontal_size_in_mbs >= WORKAROUND_HW_ISSUE_VICE2_210_LOWER_LIMIT) && (max_horizontal_size_in_mbs <= WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT) )
        {
            max_horizontal_size_in_mbs = WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT + 1;
            MaxPictureWidthInPels = (max_horizontal_size_in_mbs<<4);
        }

    }

    {
        uint32_t Non_dcxv_size_of_luma_buffer =
           BVDC_P_VIP_CalcStripeBufferSize( WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_WIDTH , WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_HEIGHT , 0 , bInterlaced, DramStripeWidth , X , Y, PageSize );
        uint32_t Non_dcxv_size_of_420_chroma_buffer
           = BVDC_P_VIP_CalcStripeBufferSize( WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_WIDTH , DIV2_ROUND(WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_HEIGHT) , 0 , bInterlaced, DramStripeWidth , X , Y, PageSize );
        Non_dcxv_BufferSize = Non_dcxv_size_of_luma_buffer + Non_dcxv_size_of_420_chroma_buffer;
    }
#endif

    /* Initialize BVDC_P_VIP stack of buffers */
    CurrAddress = 0;

    max_horizontal_size_in_pels = MaxPictureWidthInPels;
    max_vertical_size_in_pels   = MaxPictureHeightInPels;

    /*DCXV compression is used for luma and chroma in the progressive case in VIP */
    size_of_luma_buffer = BVDC_P_VIP_CalcStripeBufferSize( max_horizontal_size_in_pels , max_vertical_size_in_pels , DcxvEnable , bInterlaced , DramStripeWidth , X , Y, PageSize);
    size_of_420_chroma_buffer = BVDC_P_VIP_CalcStripeBufferSize( max_horizontal_size_in_pels , DIV2_ROUND(max_vertical_size_in_pels), false , bInterlaced , DramStripeWidth , X , Y, PageSize);

    /* TODO: may reduce if no B-picture required */
    pstMemConfig->ulNumOrigBuf           = BAVC_VCE_GetRequiredBufferCount_isrsafe( &stVceBufferConfig, BAVC_VCE_BufferType_eOriginal );
    pstMemConfig->ulLumaBufSize          = size_of_luma_buffer;
    pstMemConfig->ulChromaBufSize        = size_of_420_chroma_buffer;
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("# Original Buffers : %u", pstMemConfig->ulNumOrigBuf));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("Luma Buffer size   : %u", pstMemConfig->ulLumaBufSize));
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("Chroma Buffer size : %u", pstMemConfig->ulChromaBufSize));

    /* 1) Allocate Stack of buffers for the Original picture */
    BufferSize=size_of_luma_buffer+size_of_420_chroma_buffer;
    if (Non_dcxv_BufferSize > BufferSize) {
        BufferSize = Non_dcxv_BufferSize;
    }

    for (StackPointer=0; StackPointer<(pstMemConfig->ulNumOrigBuf << (bInterlaced)); StackPointer+=(1+bInterlaced))
    {
        CurrAddress=CurrAddress+BufferSize;
    }

    /*2) Allocate Stack of buffers for the Original Shifted 4:2:2 top field Chroma for interlaced */
    if(bInterlaced) {
        size_of_shifted_422_chroma_buffer = size_of_420_chroma_buffer;
        pstMemConfig->ulShiftedChromaBufSize = size_of_shifted_422_chroma_buffer;
        pstMemConfig->ulNumShiftedBuf        = BAVC_VCE_GetRequiredBufferCount_isrsafe( &stVceBufferConfig, BAVC_VCE_BufferType_eShiftedChroma);
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("# Shifted CrCb Bufs : %u", pstMemConfig->ulNumShiftedBuf));
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("Shifted CrCb BufSize: %u", pstMemConfig->ulShiftedChromaBufSize));
        BufferSize=size_of_shifted_422_chroma_buffer;
        for (StackPointer=0; StackPointer<(pstMemConfig->ulNumShiftedBuf); StackPointer+=1)
        {
            CurrAddress=CurrAddress+BufferSize;
        }
    }

    /*3) Allocate stack of buffers for the Decimated picture*/
    if(pstMemSettings->bDecimatedLuma) {
        uint32_t ulH1Vsize = 0, ulH2Vsize = 0;
        size_of_2h1v_luma_buffer = BVDC_P_VIP_CalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , max_vertical_size_in_pels , NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
        size_of_2h2v_luma_buffer = BVDC_P_VIP_CalcStripeBufferSize( DIV2_ROUND(max_horizontal_size_in_pels) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
        /* ViCE2v2 limit 960 for 2H vs 4H; ViCE3 limit 640 (CME) for 1H vs 2H */
        if(( BVDC_P_VIP_VER == 1 ) &&
           ( max_horizontal_size_in_pels > (HOR_SIZE_IN_PELS_1080P/2)))
        {
            ulH1Vsize = BVDC_P_VIP_CalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , max_vertical_size_in_pels , NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
            ulH2Vsize = BVDC_P_VIP_CalcStripeBufferSize( DIV4_ROUND(HOR_SIZE_IN_PELS_1080P) , DIV2_ROUND(max_vertical_size_in_pels) , NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
        } else if( BVDC_P_VIP_VER >= 2 ) /* ViCE3 CME throughput limit and 1H vs 2H */
        {
            ulH1Vsize = BVDC_P_VIP_CalcStripeBufferSize( BVDC_P_VIP_MAX_H1V_WIDTH, BVDC_P_VIP_MAX_H1V_HEIGHT, NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
            ulH2Vsize = BVDC_P_VIP_CalcStripeBufferSize( BVDC_P_VIP_MAX_H1V_WIDTH , DIV2_ROUND(BVDC_P_VIP_MAX_H1V_HEIGHT) , NonDCXVBuffer , bInterlaced , DramStripeWidth , X , Y, PageSize);
        }
        pstMemConfig->ul2H1VBufSize          = (size_of_2h1v_luma_buffer > ulH1Vsize)? size_of_2h1v_luma_buffer : ulH1Vsize;
        pstMemConfig->ul2H2VBufSize          = (size_of_2h2v_luma_buffer > ulH2Vsize)? size_of_2h2v_luma_buffer : ulH2Vsize;
        pstMemConfig->ulNumDecimBuf          = BAVC_VCE_GetRequiredBufferCount_isrsafe( &stVceBufferConfig, BAVC_VCE_BufferType_eDecimated );

        BDBG_MODULE_MSG(BVDC_VIP_MEM,("# Decimate Buffers : %u", pstMemConfig->ulNumDecimBuf));
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("ul2H1VBufSize      : %u", pstMemConfig->ul2H1VBufSize));
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("ul2H2VBufSize      : %u", pstMemConfig->ul2H2VBufSize));

        BufferSize=size_of_2h1v_luma_buffer+size_of_2h2v_luma_buffer;
        for (StackPointer=0; StackPointer<(pstMemConfig->ulNumDecimBuf << (bInterlaced)); StackPointer+=(1+bInterlaced))
        {
            CurrAddress=CurrAddress+BufferSize;
        }
    }

    pstMemConfig->ulTotalSize = CurrAddress;
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("Total size = %#x", pstMemConfig->ulTotalSize));
    return(CurrAddress);

}

/***************************************************************************
 * This is called when encoder starts or when display enables STG/VIP
 */
BERR_Code BVDC_P_Vip_AllocBuffer
    ( BVDC_P_Vip_Handle            hVip,
      BVDC_Display_Handle          hDisplay )
{
    BERR_Code rc = BERR_SUCCESS;
    BVDC_P_VipBufferNode *pBuffer;
    BVDC_P_VipAssocBufferNode *pBufferAssoc;
    uint32_t ulBlockOffset = 0;
    unsigned i;
    const BFMT_VideoInfo *pFmtInfo;
    BVDC_VipMemConfigSettings *pVipMemSettings = &hDisplay->stNewInfo.stVipMemSettings;
    BAVC_VCE_BufferConfig stVceBufferConfig;

    pFmtInfo = (hDisplay->pStgFmtInfo)?
        hDisplay->pStgFmtInfo : hDisplay->stNewInfo.pFmtInfo;

    BAVC_VCE_GetDefaultBufferConfig_isrsafe(pFmtInfo->bInterlaced, &stVceBufferConfig );
    stVceBufferConfig.eScanType = pFmtInfo->bInterlaced ? BAVC_ScanType_eInterlaced : BAVC_ScanType_eProgressive;
    if ( false == pVipMemSettings->bSupportBframes ) stVceBufferConfig.uiNumberOfBFrames = 0;

    /* compute the memory allocation; TODO: there might be runtime switch of DCXV for some workaround;
       debug capture probably requires DCXV off; */
    hVip->stMemSettings.DcxvEnable = BVDC_P_VIP_DCX_ON;

    hVip->stMemSettings.bInterlaced            = pFmtInfo->bInterlaced;
    hVip->stMemSettings.bDecimatedLuma         = pVipMemSettings->bSupportDecimatedLuma;
    hVip->stMemSettings.bBframes               = pVipMemSettings->bSupportBframes;
    hVip->stMemSettings.MaxPictureHeightInPels = pVipMemSettings->ulMaxHeight;
    hVip->stMemSettings.MaxPictureWidthInPels  = pVipMemSettings->ulMaxWidth;
    BDBG_ASSERT(pVipMemSettings->ulMemcId < 3);
    hVip->stMemSettings.PageSize        = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulPageSize;
    hVip->stMemSettings.DramStripeWidth = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulStripeWidth;
    hVip->stMemSettings.X = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulMbMultiplier;
    hVip->stMemSettings.Y = hDisplay->hVdc->stMemInfo.memc[pVipMemSettings->ulMemcId].ulMbRemainder;
    BVDC_P_MemConfig_GetVipBufSizes(&hVip->stMemSettings, &hVip->stMemConfig);

    BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] alloc %d bytes picture buffers(%u:%u:%u) on memc%u..",
        hVip->eId, hVip->stMemConfig.ulTotalSize, hVip->stMemConfig.ulNumOrigBuf,
        hVip->stMemConfig.ulNumShiftedBuf, hVip->stMemConfig.ulNumDecimBuf, pVipMemSettings->ulMemcId));

    /* Allocate picture buffers for VIP freeQ. TODO: add ITFP, B-picture and interlaced support. */
    hVip->hBlock = BMMA_Alloc(hDisplay->stNewInfo.hVipHeap, hVip->stMemConfig.ulTotalSize, hVip->stMemSettings.PageSize, NULL);
    if(hVip->hBlock==NULL) {
        BDBG_ERR(("Failed to allocate MMA block for display[%d]'s VIP[%d]", hDisplay->eId, hVip->eId));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    hVip->ullDeviceOffset = BMMA_LockOffset(hVip->hBlock);
    BDBG_MODULE_MSG(BVDC_VIP_MEM,("starting offset = "BDBG_UINT64_FMT, BDBG_UINT64_ARG(hVip->ullDeviceOffset)));
    for(i=0; i<hVip->stMemConfig.ulNumOrigBuf; i++){
        /* original luma queue */
        pBuffer = BKNI_Malloc(sizeof(BVDC_P_VipBufferNode));
        BDBG_ASSERT(pBuffer);
        pBuffer->ulBufferId = i;
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, pBuffer, link);

        BKNI_Memset(&pBuffer->stPicture, 0, sizeof(BAVC_EncodePictureBuffer));
        pBuffer->stPicture.hLumaBlock   = hVip->hBlock;
        pBuffer->stPicture.ulLumaOffset = ulBlockOffset;
        ulBlockOffset += hVip->stMemConfig.ulLumaBufSize;

        /* chroma queue */
        pBufferAssoc = BKNI_Malloc(sizeof(BVDC_P_VipAssocBufferNode));
        BDBG_ASSERT(pBufferAssoc);
        pBufferAssoc->ulBufferId = i;
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, pBufferAssoc, link);

        pBufferAssoc->hBlock   = hVip->hBlock;
        pBufferAssoc->ulOffset = ulBlockOffset;
        ulBlockOffset += hVip->stMemConfig.ulChromaBufSize;
    }

    if(pVipMemSettings->bSupportInterlaced) {/* interlaced shifted chroma buffer */
        for(i=0; i<hVip->stMemConfig.ulNumShiftedBuf; i++){
            /* shifted chroma queue */
            pBufferAssoc = BKNI_Malloc(sizeof(BVDC_P_VipAssocBufferNode));
            BDBG_ASSERT(pBufferAssoc);
            pBufferAssoc->ulBufferId = i;
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, pBufferAssoc, link);

            pBufferAssoc->hBlock = hVip->hBlock;
            pBufferAssoc->ulOffset = ulBlockOffset;
            ulBlockOffset += hVip->stMemConfig.ulShiftedChromaBufSize;
        }
    }

    if(pVipMemSettings->bSupportDecimatedLuma) {/* decimated luma buffers */
        for(i=0; i<hVip->stMemConfig.ulNumDecimBuf; i++){
            /* decimated 1v luma queue */
            pBufferAssoc = BKNI_Malloc(sizeof(BVDC_P_VipAssocBufferNode));
            BDBG_ASSERT(pBufferAssoc);
            pBufferAssoc->ulBufferId = i;
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, pBufferAssoc, link);

            pBufferAssoc->hBlock   = hVip->hBlock;
            pBufferAssoc->ulOffset = ulBlockOffset;
            ulBlockOffset += hVip->stMemConfig.ul2H1VBufSize;

            /* decimated 2v luma queue */
            pBufferAssoc = BKNI_Malloc(sizeof(BVDC_P_VipAssocBufferNode));
            BDBG_ASSERT(pBufferAssoc);
            pBufferAssoc->ulBufferId = i;
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, pBufferAssoc, link);

            pBufferAssoc->hBlock   = hVip->hBlock;
            pBufferAssoc->ulOffset = ulBlockOffset;
            ulBlockOffset += hVip->stMemConfig.ul2H2VBufSize;
        }
    }

    BVDC_P_ITFP_EPM_Preprocessor_init(hVip);
#if BVDC_P_DUMP_VIP_PICTURE
    hVip->pY = BMMA_Lock(hVip->hBlock);
    if(hVip->pY == NULL) {
        BDBG_ERR(("VIP dump failed to lock the memory block!"));
    }
    hVip->pfY = fopen("videos/vipY0.img", "wb");
    if(!hVip->pfY) {
        BDBG_ERR(("Failed to open videos/vipY.img to dump vip luma capture picture"));
    }
    hVip->pfC = fopen("videos/vipC0.img", "wb");
    if(!hVip->pfC) {
        BDBG_ERR(("Failed to open videos/vipC.img to dump vip chroma capture picture"));
    }
    if(hVip->stMemSettings.bDecimatedLuma) {
        hVip->pfY2H1V= fopen("videos/vip2H1V0.img", "wb");
        if(!hVip->pfY2H1V) {
            BDBG_ERR(("Failed to open videos/vip2H1V0.img to dump vip 2H1V decimated luma capture picture"));
        }
        hVip->pfY2H2V= fopen("videos/vip2H2V0.img", "wb");
        if(!hVip->pfY2H2V) {
            BDBG_ERR(("Failed to open videos/vip2H2V0.img to dump vip 2H2V decimated luma capture picture"));
        }
    }
    if(hVip->stMemSettings.bInterlaced) {
        hVip->pfCshifted = fopen("videos/vipCshift0.img", "wb");
        if(!hVip->pfCshifted) {
            BDBG_ERR(("Failed to open videos/vipCshift0.img to dump vip shifted chroma capture picture"));
        }
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
    BVDC_P_VipAssocBufferNode *pBufferAssoc;
    bool bItfp = hVip->hDisplay->stCurInfo.stVipMemSettings.bSupportItfp;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_MODULE_MSG(BVDC_DISP_VIP,("[%d] frees buffers", hVip->eId));

    /* unlink VIP with display first */
    BKNI_EnterCriticalSection();
    hVip->hDisplay->hVip = NULL;
    hVip->hDisplay       = NULL;
    BKNI_LeaveCriticalSection();

    /* 1) free original luma buffer queues */
    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stCaptureQ))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases captureQ buf[%d] %p, pNext=%p", hVip->eId, pBuffer->ulBufferId, (void *)pBuffer, (void *)BLST_SQ_NEXT(pBuffer, link)));
        BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQ, link);
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, pBuffer, link);
    }
    /* TODO: must make sure encoder releasing the delivered pictures before unlock/free the mma allocation */
    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stDeliverQ))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases deliveryQ buf[%d]", hVip->eId, pBuffer->ulBufferId));
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
    /* free itfp buffers */
    if(bItfp) {
        while(hVip->stEpmInfo.PreProcessorPictureQueue.Fullness--) {
            pBuffer = hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pPic;
            if(pBuffer) {
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, pBuffer, link);
                hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pPic = NULL;
                BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases ITFP[%u] buf[%d] %p, pNext=%p",
                    hVip->eId, hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr, pBuffer->ulBufferId,
                    (void *)pBuffer, (void *)BLST_SQ_NEXT(pBuffer, link)));
            }
            pBufferAssoc = hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pDecim1v;
            if(pBufferAssoc) {
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, pBufferAssoc, link);
                hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pDecim1v = NULL;
                BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases ITFP Decim1v buf[%d] %p, pNext=%p",
                    hVip->eId, pBufferAssoc->ulBufferId,
                    (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
            }
            pBufferAssoc = hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pDecim2v;
            if(pBufferAssoc) {
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, pBufferAssoc, link);
                hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pDecim2v = NULL;
                BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases ITFP Decim2v buf[%d] %p, pNext=%p",
                    hVip->eId, pBufferAssoc->ulBufferId,
                    (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
            }
            pBufferAssoc = hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pChroma;
            if(pBufferAssoc) {
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, pBufferAssoc, link);
                hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pChroma = NULL;
                BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases ITFP Chroma buf[%d] %p, pNext=%p",
                    hVip->eId, pBufferAssoc->ulBufferId,
                    (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
            }
            pBufferAssoc = hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pShifted;
            if(pBufferAssoc) {
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, pBufferAssoc, link);
                hVip->astItfpPicQ[hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr].pShifted = NULL;
                BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases ITFP Shifted buf[%d] %p, pNext=%p",
                    hVip->eId, pBufferAssoc->ulBufferId,
                    (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
            }
            hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr = (hVip->stEpmInfo.PreProcessorPictureQueue.RdPtr + 1) %
                hVip->stEpmInfo.PreProcessorPictureQueue.NumOfBuff;
        }
    }

    /* 2) free chroma buffer queues */
    while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stCaptureQchroma))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases captureQ chroma buf[%d] %p, pNext=%p", hVip->eId, pBufferAssoc->ulBufferId, (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
        BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQchroma, link);
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, pBufferAssoc, link);
    }
    /* TODO: must make sure encoder releasing the delivered pictures before unlock/free the mma allocation */
    while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stDeliverQchroma))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases deliveryQ chroma buf[%d]", hVip->eId, pBufferAssoc->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stDeliverQchroma, link);
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, pBufferAssoc, link);
    }
    /* free the intermediate pToCapture and pCapture buffers */
    if(hVip->pCaptureChroma) {
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, hVip->pCaptureChroma, link);
        hVip->pCaptureChroma = NULL;
    }
    if(hVip->pToCaptureChroma) {
        BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, hVip->pToCaptureChroma, link);
        hVip->pToCaptureChroma = NULL;
    }

    if(hVip->stMemSettings.bInterlaced) {/* interlaced shifted chroma buffer */
        /* 3) free shifted chroma luma buffer queues */
        while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stCaptureQshifted))){
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases captureQ shifted chroma buf[%d] %p, pNext=%p", hVip->eId, pBufferAssoc->ulBufferId, (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
            BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQshifted, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, pBufferAssoc, link);
        }
        /* TODO: must make sure encoder releasing the delivered pictures before unlock/free the mma allocation */
        while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stDeliverQshifted))){
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases deliveryQ shifted chroma buf[%d]", hVip->eId, pBufferAssoc->ulBufferId));
            BLST_SQ_REMOVE_HEAD(&hVip->stDeliverQshifted, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, pBufferAssoc, link);
        }
        /* free the intermediate pToCapture and pCapture buffers */
        if(hVip->pCaptureShifted) {
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, hVip->pCaptureShifted, link);
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases pCaptureShifted buf[%d]", hVip->eId, hVip->pCaptureShifted->ulBufferId));
            hVip->pCaptureShifted = NULL;
        }
        if(hVip->pToCaptureShifted) {
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, hVip->pToCaptureShifted, link);
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases pToCaptureShifted buf[%d]", hVip->eId, hVip->pToCaptureShifted->ulBufferId));
            hVip->pToCaptureShifted = NULL;
        }
    }

    if(hVip->stMemSettings.bDecimatedLuma) {
        /* 4) free decimated 1v luma buffer queues */
        while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stCaptureQdecim1v))){
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases captureQ decim 1v buf[%d] %p, pNext=%p", hVip->eId, pBufferAssoc->ulBufferId, (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
            BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQdecim1v, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, pBufferAssoc, link);
        }
        /* TODO: must make sure encoder releasing the delivered pictures before unlock/free the mma allocation */
        while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stDeliverQdecim1v))){
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases deliveryQ decim 1v buf[%d]", hVip->eId, pBufferAssoc->ulBufferId));
            BLST_SQ_REMOVE_HEAD(&hVip->stDeliverQdecim1v, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, pBufferAssoc, link);
        }
        /* free the intermediate pToCapture and pCapture buffers */
        if(hVip->pCaptureDecim1v) {
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, hVip->pCaptureDecim1v, link);
            hVip->pCaptureDecim1v = NULL;
        }
        if(hVip->pToCaptureDecim1v) {
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, hVip->pToCaptureDecim1v, link);
            hVip->pToCaptureDecim1v = NULL;
        }

        /* 5) free decimated 2v luma buffer queues */
        while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stCaptureQdecim2v))){
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases captureQ decim 2v buf[%d] %p, pNext=%p", hVip->eId, pBufferAssoc->ulBufferId, (void *)pBufferAssoc, (void *)BLST_SQ_NEXT(pBufferAssoc, link)));
            BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQdecim2v, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, pBufferAssoc, link);
        }
        /* TODO: must make sure encoder releasing the delivered pictures before unlock/free the mma allocation */
        while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stDeliverQdecim2v))){
            BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] releases deliveryQ decim 2v buf[%d]", hVip->eId, pBufferAssoc->ulBufferId));
            BLST_SQ_REMOVE_HEAD(&hVip->stDeliverQdecim2v, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, pBufferAssoc, link);
        }
        /* free the intermediate pToCapture and pCapture buffers */
        if(hVip->pCaptureDecim2v) {
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, hVip->pCaptureDecim2v, link);
            hVip->pCaptureDecim2v = NULL;
        }
        if(hVip->pToCaptureDecim2v) {
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, hVip->pToCaptureDecim2v, link);
            hVip->pToCaptureDecim2v = NULL;
        }
    }

    BVDC_P_ITFP_EPM_Preprocessor_flush(&hVip->stEpmInfo);
    /* free picture buffers for VIP queues */
    pBuffer = BLST_SQ_FIRST(&hVip->stFreeQ);
    BMMA_UnlockOffset(pBuffer->stPicture.hLumaBlock, hVip->ullDeviceOffset);
#if BVDC_P_DUMP_VIP_PICTURE
    BMMA_Unlock(hVip->hBlock, hVip->pY);
    if(hVip->pfY) {
        fclose(hVip->pfY);
    }
    if(hVip->pfC) {
        fclose(hVip->pfC);
    }
    if(hVip->pfY2H1V) {
        fclose(hVip->pfY2H1V);
    }
    if(hVip->pfY2H2V) {
        fclose(hVip->pfY2H2V);
    }
    if(hVip->pfCshifted) {
        fclose(hVip->pfCshifted);
    }
#endif
    BMMA_Free(hVip->hBlock);
    hVip->hBlock = NULL;
    /* release freeQ and deliverQ */
    while(NULL != (pBuffer=BLST_SQ_FIRST(&hVip->stFreeQ))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] freeQ orig buffer[%d] to remove", hVip->eId, pBuffer->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQ, link);
        BKNI_Free(pBuffer);
    }
    while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stFreeQchroma))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] freeQ chroma buffer[%d] to remove", hVip->eId, pBufferAssoc->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQchroma, link);
        BKNI_Free(pBufferAssoc);
    }
    while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stFreeQshifted))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] freeQ shifted chroma buffer[%d] to remove", hVip->eId, pBufferAssoc->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQshifted, link);
        BKNI_Free(pBufferAssoc);
    }
    while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stFreeQdecim1v))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] freeQ decim 1v buffer[%d] to remove", hVip->eId, pBufferAssoc->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQdecim1v, link);
        BKNI_Free(pBufferAssoc);
    }
    while(NULL != (pBufferAssoc=BLST_SQ_FIRST(&hVip->stFreeQdecim2v))){
        BDBG_MODULE_MSG(BVDC_VIP_MEM,("[%d] freeQ decim 2v buffer[%d] to remove", hVip->eId, pBufferAssoc->ulBufferId));
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQdecim2v, link);
        BKNI_Free(pBufferAssoc);
    }

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
    BVDC_P_VipAssocBufferNode *pBufferChroma;
    BVDC_P_VipAssocBufferNode *pBufferShifted;
    BVDC_P_VipAssocBufferNode *pBufferDecim1v;
    BVDC_P_VipAssocBufferNode *pBufferDecim2v;

    BDBG_ASSERT(pPicture);
    BKNI_Memset_isr((void*)pPicture, 0, sizeof(BAVC_EncodePictureBuffer));
    pBuffer = BLST_SQ_FIRST(&hVip->stCaptureQ);
    pBufferChroma = BLST_SQ_FIRST(&hVip->stCaptureQchroma);
    pBufferShifted = BLST_SQ_FIRST(&hVip->stCaptureQshifted);
    pBufferDecim1v = BLST_SQ_FIRST(&hVip->stCaptureQdecim1v);
    pBufferDecim2v = BLST_SQ_FIRST(&hVip->stCaptureQdecim2v);
    /* NOTE: delay one picture to avoid tearing */
    if(pBuffer && pBufferChroma &&
       ((pBufferDecim1v && pBufferDecim2v) || !hVip->stMemSettings.bDecimatedLuma) &&
       (pBufferShifted || (BAVC_Polarity_eTopField!=pBuffer->stPicture.ePolarity))) { /* move from captureQ to deliverQ */
        BKNI_Memcpy_isr((void*)pPicture, (void*)&pBuffer->stPicture, sizeof(BAVC_EncodePictureBuffer));
        /* enqueue chroma buf */
        pPicture->hChromaBlock   = pBufferChroma->hBlock;
        pPicture->ulChromaOffset = pBufferChroma->ulOffset;
        BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQchroma, link);
        BLST_SQ_INSERT_TAIL(&hVip->stDeliverQchroma, pBufferChroma, link);

        if(BAVC_Polarity_eTopField == pBuffer->stPicture.ePolarity) {
            /* enqueue shifted chroma buf */
            pPicture->hShiftedChromaBlock   = pBufferShifted->hBlock;
            pPicture->ulShiftedChromaOffset = pBufferShifted->ulOffset;
            BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQshifted, link);
            BLST_SQ_INSERT_TAIL(&hVip->stDeliverQshifted, pBufferShifted, link);
        }

        if(hVip->stMemSettings.bDecimatedLuma) {
            pPicture->h1VLumaBlock = pBufferDecim1v->hBlock;
            pPicture->ul1VLumaOffset = pBufferDecim1v->ulOffset;
            pPicture->h2VLumaBlock = pBufferDecim2v->hBlock;
            pPicture->ul2VLumaOffset = pBufferDecim2v->ulOffset;
            /* enqueue decim 1v buf */
            BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQdecim1v, link);
            BLST_SQ_INSERT_TAIL(&hVip->stDeliverQdecim1v, pBufferDecim1v, link);
            /* enqueue decim 2v buf */
            BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQdecim2v, link);
            BLST_SQ_INSERT_TAIL(&hVip->stDeliverQdecim2v, pBufferDecim2v, link);
        }
        BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] GET pic[%u][%#x] buf[%u:%u:%u:%u:%u]: %ux%u%c[stc=%#x, %#x][opts=%#x]", hVip->eId, pPicture->ulPictureId,
            pPicture->ulLumaOffset, pBuffer->ulBufferId, pBufferChroma->ulBufferId, pBufferShifted?pBufferShifted->ulBufferId:0,
            pBufferDecim1v?pBufferDecim1v->ulBufferId:0, pBufferDecim2v?pBufferDecim2v->ulBufferId:0,
            pPicture->ulWidth, pPicture->ulHeight, (BAVC_Polarity_eFrame==pPicture->ePolarity)?'p':
            ((BAVC_Polarity_eTopField==pPicture->ePolarity)? 'T':'B'),
            pPicture->ulSTCSnapshotHi, pPicture->ulSTCSnapshotLo, pPicture->ulOriginalPTS));
        BDBG_MODULE_MSG(BVDC_VIP_Q,("  cadence: type = %d, phase = %d", pPicture->stCadence.type, pPicture->stCadence.phase));
        BLST_SQ_REMOVE_HEAD(&hVip->stCaptureQ, link);
        BLST_SQ_INSERT_TAIL(&hVip->stDeliverQ, pBuffer, link);
#if BVDC_P_DUMP_VIP_PICTURE /* original pic buffer */
        if(!hVip->bDumped && pPicture->ulOriginalPTS) {/* dump the 1st unmute pictures*/
            void *pY, *pC, *pY1V, *pY2V, *pCshift;
            uint32_t totalStripes = (pPicture->ulWidth + pPicture->ulStripeWidth - 1) / pPicture->ulStripeWidth;
            uint32_t ulLumaBufSize = pPicture->ulLumaNMBY * totalStripes * pPicture->ulStripeWidth * 16;
            uint32_t ulChromaBufSize = pPicture->ulChromaNMBY * totalStripes * pPicture->ulStripeWidth * 16;
            char path[40];

            hVip->bDumped = (++hVip->dumpCnt) >= hVip->numPicsToCapture;/* only capture ten */
            pY = (void*)((uint8_t*)hVip->pY + pPicture->ulLumaOffset);
            pC = (void*)((uint8_t*)hVip->pY + pPicture->ulChromaOffset);
            BMMA_FlushCache_isr(pPicture->hLumaBlock, pY, ulLumaBufSize);
            BMMA_FlushCache_isr(pPicture->hChromaBlock, pC, ulChromaBufSize);

            BDBG_WRN(("=== opts = %#x", pPicture->ulOriginalPTS));
            BDBG_WRN(("stripeWidth = %u", pPicture->ulStripeWidth));
            BDBG_WRN(("yNMBY       = %u", pPicture->ulLumaNMBY));
            BDBG_WRN(("cNMBY       = %u", pPicture->ulChromaNMBY));
            BDBG_WRN(("stripe      = %u", pPicture->ulStripeWidth));
            BDBG_WRN(("format      = %ux%u%c", pPicture->ulWidth, pPicture->ulHeight, (BAVC_Polarity_eFrame==pPicture->ePolarity)?'p':'i'));
            BDBG_WRN(("luma pointer   = %p, offset = %#x, size = %u", pY, pPicture->ulLumaOffset, ulLumaBufSize));
            BDBG_WRN(("chroma pointer = %p, offset = %#x, size = %u", pC, pPicture->ulChromaOffset, ulChromaBufSize));
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
            if(hVip->stMemSettings.bDecimatedLuma) {
                uint32_t ulH1VBufSize = pPicture->ul1VLumaNMBY * totalStripes * pPicture->ulStripeWidth * 16;
                uint32_t ulH2VBufSize = pPicture->ul2VLumaNMBY * totalStripes * pPicture->ulStripeWidth * 16;
                pY1V = (void*)((uint8_t*)hVip->pY + pPicture->ul1VLumaOffset);
                pY2V = (void*)((uint8_t*)hVip->pY + pPicture->ul2VLumaOffset);
                BDBG_MSG(("2H1V pointer   = %p, offset = %#x, size = %u", pY1V, pPicture->ul1VLumaOffset, ulH1VBufSize));
                BDBG_MSG(("2H2V pointer   = %p, offset = %#x, size = %u", pY2V, pPicture->ul2VLumaOffset, ulH2VBufSize));
                BMMA_FlushCache_isr(pPicture->h1VLumaBlock, pY1V, ulH1VBufSize);
                BMMA_FlushCache_isr(pPicture->h2VLumaBlock, pY2V, ulH2VBufSize);
                if(hVip->pfY2H1V) {
                    fwrite(pY1V, 1, ulH1VBufSize, hVip->pfY2H1V);
                    fclose(hVip->pfY2H1V);
                    if(!hVip->bDumped) {
                        BKNI_Snprintf(path, 40, "videos/vipY2H1V%u.img", hVip->dumpCnt);
                        hVip->pfY2H1V = fopen(path, "wb");
                    } else {
                        hVip->pfY2H1V = NULL;
                    }
                }
                if(hVip->pfY2H2V) {
                    fwrite(pY2V, 1, ulH2VBufSize, hVip->pfY2H2V);
                    fclose(hVip->pfY2H2V);
                    if(!hVip->bDumped) {
                        BKNI_Snprintf(path, 40, "videos/vipY2H2V%u.img", hVip->dumpCnt);
                        hVip->pfY2H2V = fopen(path, "wb");
                    } else {
                        hVip->pfY2H2V = NULL;
                    }
                }
            }
            if(BAVC_Polarity_eTopField == pBuffer->stPicture.ePolarity) {
                pCshift = (void*)((uint8_t*)hVip->pY + pPicture->ulShiftedChromaOffset);
                BMMA_FlushCache_isr(pPicture->hShiftedChromaBlock, pCshift, ulChromaBufSize);
                BDBG_MSG(("shifted chroma pointer   = %p, offset = %#x, size = %u", pCshift, pPicture->ulShiftedChromaOffset, ulChromaBufSize));
                if(hVip->pfCshifted) {
                    fwrite(pCshift, 1, ulChromaBufSize, hVip->pfCshifted);
                    fclose(hVip->pfCshifted);
                    if(!hVip->bDumped) {
                        BKNI_Snprintf(path, 40, "videos/vipCshift%u.img", hVip->dumpCnt);
                        hVip->pfCshifted = fopen(path, "wb");
                    } else {
                        hVip->pfCshifted = NULL;
                    }
                }
            }
        }
#endif
    } else
    {
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("[%d] GET NULL[%p, %p, %p, %p, %p]", hVip->eId,
            (void*)pBuffer, (void*)pBufferChroma, (void*)pBufferShifted, (void*)pBufferDecim1v, (void*)pBufferDecim2v));
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
    BVDC_P_VipAssocBufferNode *pBufferAssoc;
    bool bFound = false;

    BDBG_ASSERT(pPicture);

    /* find the matching node */
    if(pPicture->hLumaBlock) {
        for(pBuffer = BLST_SQ_FIRST(&hVip->stDeliverQ); pBuffer; pBuffer= BLST_SQ_NEXT(pBuffer, link)) {
            BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] to RET pic[%u][%#x:%p] buf[%d]pic[%u][%#x:%p]..",
                hVip->eId, pPicture->ulPictureId, pPicture->ulLumaOffset, (void*)pPicture->hLumaBlock,
                pBuffer->ulBufferId, pBuffer->stPicture.ulPictureId, pBuffer->stPicture.ulLumaOffset, (void*)pBuffer->stPicture.hLumaBlock));
            if(pBuffer->stPicture.ulLumaOffset == pPicture->ulLumaOffset &&
               pBuffer->stPicture.hLumaBlock   == pPicture->hLumaBlock) {
                BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] RET pic[%u] buf[%d]..", hVip->eId, pPicture->ulPictureId, pBuffer->ulBufferId));
                /* move from deliverQ to freeQ */
                BLST_SQ_REMOVE(&hVip->stDeliverQ, pBuffer, BVDC_P_VipBufferNode, link);
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, pBuffer, link);
                bFound = true;
                break;
            }
        }
    }
    if(pPicture->hChromaBlock) {
        for(pBufferAssoc = BLST_SQ_FIRST(&hVip->stDeliverQchroma); pBufferAssoc; pBufferAssoc= BLST_SQ_NEXT(pBufferAssoc, link)) {
            BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] to RET chroma pic[%u][%#x:%p] buf[%d][%#x:%p]..",
                hVip->eId, pPicture->ulPictureId, pPicture->ulChromaOffset, (void*)pPicture->hChromaBlock,
                pBufferAssoc->ulBufferId, pBufferAssoc->ulOffset, (void*)pBufferAssoc->hBlock));
            if(pBufferAssoc->hBlock == pPicture->hChromaBlock &&
               pBufferAssoc->ulOffset == pPicture->ulChromaOffset) {
                BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] RET chroma pic[%u] buf[%d]..", hVip->eId, pPicture->ulPictureId, pBufferAssoc->ulBufferId));
                /* move from deliverQ to freeQ */
                BLST_SQ_REMOVE(&hVip->stDeliverQchroma, pBufferAssoc, BVDC_P_VipAssocBufferNode, link);
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, pBufferAssoc, link);
                bFound = true;
                break;
            }
        }
    }
    if(pPicture->hShiftedChromaBlock) {
        for(pBufferAssoc = BLST_SQ_FIRST(&hVip->stDeliverQshifted); pBufferAssoc; pBufferAssoc= BLST_SQ_NEXT(pBufferAssoc, link)) {
            BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] to RET shifted chroma pic[%u][%#x:%p] buf[%d][%#x:%p]..",
                hVip->eId, pPicture->ulPictureId, pPicture->ulShiftedChromaOffset, (void*)pPicture->hShiftedChromaBlock,
                pBufferAssoc->ulBufferId, pBufferAssoc->ulOffset, (void*)pBufferAssoc->hBlock));
            if(pBufferAssoc->hBlock == pPicture->hShiftedChromaBlock &&
               pBufferAssoc->ulOffset == pPicture->ulShiftedChromaOffset) {
                BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] RET shifted chroma pic[%u] buf[%d]..", hVip->eId, pPicture->ulPictureId, pBufferAssoc->ulBufferId));
                /* move from deliverQ to freeQ */
                BLST_SQ_REMOVE(&hVip->stDeliverQshifted, pBufferAssoc, BVDC_P_VipAssocBufferNode, link);
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, pBufferAssoc, link);
                bFound = true;
                break;
            }
        }
    }
    if(pPicture->h1VLumaBlock) {
        for(pBufferAssoc = BLST_SQ_FIRST(&hVip->stDeliverQdecim1v); pBufferAssoc; pBufferAssoc= BLST_SQ_NEXT(pBufferAssoc, link)) {
            BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] to RET decim 1v pic[%u][%#x:%p] buf[%d][%#x:%p]..",
                hVip->eId, pPicture->ulPictureId, pPicture->ul1VLumaOffset, (void*)pPicture->h1VLumaBlock,
                pBufferAssoc->ulBufferId, pBufferAssoc->ulOffset, (void*)pBufferAssoc->hBlock));
            if(pBufferAssoc->hBlock == pPicture->h1VLumaBlock &&
               pBufferAssoc->ulOffset == pPicture->ul1VLumaOffset) {
                BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] RET decim 1v pic[%u] buf[%d]..", hVip->eId, pPicture->ulPictureId, pBufferAssoc->ulBufferId));
                /* move from deliverQ to freeQ */
                BLST_SQ_REMOVE(&hVip->stDeliverQdecim1v, pBufferAssoc, BVDC_P_VipAssocBufferNode, link);
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, pBufferAssoc, link);
                bFound = true;
                break;
            }
        }
    }
    if(pPicture->h2VLumaBlock) {
        for(pBufferAssoc = BLST_SQ_FIRST(&hVip->stDeliverQdecim2v); pBufferAssoc; pBufferAssoc= BLST_SQ_NEXT(pBufferAssoc, link)) {
            BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] to RET decim 2v pic[%u][%#x:%p] buf[%d][%#x:%p]..",
                hVip->eId, pPicture->ulPictureId, pPicture->ul2VLumaOffset, (void*)pPicture->h2VLumaBlock,
                pBufferAssoc->ulBufferId, pBufferAssoc->ulOffset, (void*)pBufferAssoc->hBlock));
            if(pBufferAssoc->hBlock == pPicture->h2VLumaBlock &&
               pBufferAssoc->ulOffset == pPicture->ul2VLumaOffset) {
                BDBG_MODULE_MSG(BVDC_VIP_Q,("[%d] RET decim 2v pic[%u] buf[%d]..", hVip->eId, pPicture->ulPictureId, pBufferAssoc->ulBufferId));
                /* move from deliverQ to freeQ */
                BLST_SQ_REMOVE(&hVip->stDeliverQdecim2v, pBufferAssoc, BVDC_P_VipAssocBufferNode, link);
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, pBufferAssoc, link);
                bFound = true;
                break;
            }
        }
    }
    if(!bFound) {
        BDBG_MODULE_WRN(BVDC_VIP_Q,("[%d] RET a pic[%u] not found", hVip->eId, pPicture->ulPictureId));
    }
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

    BDBG_ENTER(BVDC_P_Vip_Create);

    BDBG_ASSERT(phVip);
    BDBG_ASSERT(hVdc);

    /* initialize bchp memory info */
    if(!hVdc->stMemInfo.memc[0].valid) {
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
#elif BCHP_VICE_VIP1_0_0_REG_START
    case 1:
        pVip->ulRegOffset = BCHP_VICE_VIP1_0_0_REG_START - BCHP_VICE_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP_0_1_REG_START
    case 2:
        pVip->ulRegOffset = BCHP_VICE2_VIP_0_1_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#elif BCHP_VICE_VIP_0_1_REG_START
    case 2:
        pVip->ulRegOffset = BCHP_VICE_VIP_0_1_REG_START - BCHP_VICE_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP1_0_1_REG_START
    case 3:
        pVip->ulRegOffset = BCHP_VICE2_VIP1_0_1_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#elif BCHP_VICE_VIP1_0_1_REG_START
    case 3:
        pVip->ulRegOffset = BCHP_VICE_VIP1_0_1_REG_START - BCHP_VICE_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP2_0_0_REG_START
    case 4:
        pVip->ulRegOffset = BCHP_VICE2_VIP2_0_0_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#elif BCHP_VICE_VIP2_0_0_REG_START
    case 4:
        pVip->ulRegOffset = BCHP_VICE_VIP2_0_0_REG_START - BCHP_VICE_VIP_0_0_REG_START;
        break;
#endif
#if BCHP_VICE2_VIP2_0_1_REG_START
    case 5:
        pVip->ulRegOffset = BCHP_VICE2_VIP2_0_1_REG_START - BCHP_VICE2_VIP_0_0_REG_START;
        break;
#elif BCHP_VICE_VIP2_0_1_REG_START
    case 5:
        pVip->ulRegOffset = BCHP_VICE_VIP2_0_1_REG_START - BCHP_VICE_VIP_0_0_REG_START;
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
    BLST_SQ_INIT(&pVip->stFreeQchroma);
    BLST_SQ_INIT(&pVip->stCaptureQchroma);
    BLST_SQ_INIT(&pVip->stDeliverQchroma);
    BLST_SQ_INIT(&pVip->stFreeQshifted);
    BLST_SQ_INIT(&pVip->stCaptureQshifted);
    BLST_SQ_INIT(&pVip->stDeliverQshifted);
    BLST_SQ_INIT(&pVip->stFreeQdecim1v);
    BLST_SQ_INIT(&pVip->stCaptureQdecim1v);
    BLST_SQ_INIT(&pVip->stDeliverQdecim1v);
    BLST_SQ_INIT(&pVip->stFreeQdecim2v);
    BLST_SQ_INIT(&pVip->stCaptureQdecim2v);
    BLST_SQ_INIT(&pVip->stDeliverQdecim2v);
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
    BDBG_ENTER(BVDC_P_Vip_Destroy);
    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);

    BDBG_MODULE_MSG(BVDC_DISP_VIP,("To destroy VIP%u", hVip->eId));

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
        BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_RD_HIST, DONE) |
        BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_RD_PCC,  DONE) |
        BVDC_P_VIP_RDB_ENUM(FW_CONTROL, PIC_START,    NULL) |
        BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_WR_REG,  NULL) );

    hVip->stRegs.ulConfig = (
        BVDC_P_VIP_RDB_ENUM(CONFIG, ENABLE_CTRL, ENABLE_BY_PICTURE) |
        BVDC_P_VIP_RDB_ENUM(CONFIG, AUTO_SHUTOFF,           ENABLE) |
        BVDC_P_VIP_RDB_ENUM(CONFIG, PADDING_MODE,           ENABLE) |
#if BVDC_P_VIP_RDB_MASK(CONFIG,   ADDRESSING_MODE)
        BVDC_P_VIP_RDB_ENUM(CONFIG, ADDRESSING_MODE,        STRIPE) |
#endif
#if BVDC_P_VIP_RDB_MASK(CONFIG,   DRAM_STRIPE_WIDTH)
        BVDC_P_VIP_RDB_DATA(CONFIG, DRAM_STRIPE_WIDTH,           1) |
#endif
        BVDC_P_VIP_RDB_ENUM(CONFIG, CHROMA_420,             ENABLE) |
        BVDC_P_VIP_RDB_ENUM(CONFIG, LUMA,                   ENABLE) );

    /* Initialize state. */
    hVip->bInitial = true;
    hVip->eVipDataMode = BVDC_P_VipDataMode_eStripe;

    if(hVip->stMemSettings.DcxvEnable)
    {
#if BVDC_P_VIP_RDB_MASK(DCXV_CFG, MODE)
        /* VICE2_VIP_0_0_DCXV_CFG */
        hVip->stRegs.ulDcxvCfg =  (
            BVDC_P_VIP_RDB_ENUM(DCXV_CFG, MODE,     COMPRESS) |
            BVDC_P_VIP_RDB_ENUM(DCXV_CFG, PREDICTION,    AVG) |
            BVDC_P_VIP_RDB_ENUM(DCXV_CFG, LINE_STORE, BITS_6));
#elif BVDC_P_VIP_RDB_MASK(DCXV_CFG, MODE_IME_PATH)
        /* VICE2_VIP_0_0_DCXV_CFG */
        hVip->stRegs.ulDcxvCfg =  (
            BVDC_P_VIP_RDB_ENUM(DCXV_CFG, MODE_IME_PATH,     COMPRESS) |
            BVDC_P_VIP_RDB_ENUM(DCXV_CFG, PREDICTION_IME_PATH,    AVG));
#endif
    }
    BDBG_LEAVE(BVDC_P_Vip_Init);
    return;
}

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
    BVDC_P_VipAssocBufferNode *pBufferChroma;
    BVDC_P_VipAssocBufferNode *pBufferShifted;
    BVDC_P_VipAssocBufferNode *pBufferDecim1v;
    BVDC_P_VipAssocBufferNode *pBufferDecim2v;
    const BFMT_VideoInfo *pFmtInfo;
    BVDC_Display_Handle hDisplay;

    BDBG_ENTER(BVDC_P_Vip_BuildRul_isr);
    BDBG_OBJECT_ASSERT(hVip, BVDC_VIP);
    hDisplay = hVip->hDisplay;

    pFmtInfo = (hDisplay->pStgFmtInfo)?
        hDisplay->pStgFmtInfo : hDisplay->stCurInfo.pFmtInfo;

    if (pFmtInfo->bInterlaced) {
       BDBG_ASSERT( hVip->stMemSettings.bInterlaced );
    }

    /* Two intermediate picture delays.
       Note RUL has a picture delay, so when isr building the next capture picture, the last capture just got triggered!
       and it takes one more picture time to complete the capture! We allow picture delivered only after its capture is done. */
    if(hVip->pCapture) {/* beingCaptured -> Capture Done */
        /* Note, ITFP queue filling may zero out pCapture and delay the delivery */
        if(hDisplay->stCurInfo.stVipMemSettings.bSupportItfp)
        {
            /* cadence detection given the stats available now with the just captured picture! */
            hVip->stEpmInfo.IsProgressive = !pFmtInfo->bInterlaced;
            BVDC_P_ITFP_EPM_Preprocessor_ReadVipStats_isr(hVip);
            BVDC_P_ITFP_EPM_Preprocessor_CadenceDetection_isr(hVip);
        }

        if(hVip->pCapture)
        {   /* possible ITFP queue delay */
            /* insert the captured picture to the CaptureQ available for GetBuffer API */
            BLST_SQ_INSERT_TAIL(&hVip->stCaptureQ, hVip->pCapture, link);
            BLST_SQ_INSERT_TAIL(&hVip->stCaptureQchroma, hVip->pCaptureChroma, link);
            /* only top field delivers shifted chroma buffer */
            if(hVip->pCaptureShifted && hVip->pCapture->stPicture.ePolarity == BAVC_Polarity_eTopField) {
                BLST_SQ_INSERT_TAIL(&hVip->stCaptureQshifted, hVip->pCaptureShifted, link);
                hVip->pCaptureShifted = NULL; /* clear the pointer */
            }
            if(hVip->stMemSettings.bDecimatedLuma) {
                BLST_SQ_INSERT_TAIL(&hVip->stCaptureQdecim1v, hVip->pCaptureDecim1v, link);
                hVip->pCaptureDecim1v = NULL; /* clear the pointer */
                BLST_SQ_INSERT_TAIL(&hVip->stCaptureQdecim2v, hVip->pCaptureDecim2v, link);
                hVip->pCaptureDecim2v = NULL; /* clear the pointer */
            }
        }
        hVip->pCapture = NULL; /* clear the pointer */
        hVip->pCaptureChroma = NULL; /* clear the pointer */
    }
    /* if last picture to be captured existed, now that last trigger fired this isr, move it to the pCapture */
    if(hVip->pToCapture) {
        /* drop the unexecuted buffer, including cmp slip2lock transition */
        if(hDisplay->stCurInfo.bStgNonRealTime ||
           (pList->bLastExecuted && /* if not executed, skip */
             /* only if the last two isrs both in slip2lock transition and current isr exits transition, skip it */
            ((0==hVip->ulPrevSlip2Lock) || (0==hVip->ulSlip2Lock) || (0!=hDisplay->hCompositor->ulSlip2Lock)))) {
            /* one picture delay: ToCapture -> beingCaptured */
            hVip->pCapture = hVip->pToCapture;
            hVip->pCaptureChroma = hVip->pToCaptureChroma;
            /* only top field delivers shifted chroma buffer */
            if(hVip->pCapture->stPicture.ePolarity == BAVC_Polarity_eTopField) {
                hVip->pCaptureShifted = hVip->pToCaptureShifted;
            } else {
                hVip->pCaptureShifted = NULL;
            }
            hVip->pToCaptureShifted = NULL; /* clear the pointer */
            if(hVip->stMemSettings.bDecimatedLuma) {
                hVip->pCaptureDecim1v = hVip->pToCaptureDecim1v;
                hVip->pCaptureDecim2v = hVip->pToCaptureDecim2v;
                hVip->pToCaptureDecim1v = NULL; /* clear the pointer */
                hVip->pToCaptureDecim2v = NULL; /* clear the pointer */
            }
            /* read stc snapshot by last trigger and saved to the buffer! */
            if(hDisplay->stCurInfo.ulStcSnapshotLoAddr && hDisplay->stCurInfo.ulStcSnapshotHiAddr) {
                /* 20150123 must read HI first to latch LO */
                hVip->pCapture->stPicture.ulSTCSnapshotHi = BREG_Read32_isr(hDisplay->hVdc->hRegister, hDisplay->stCurInfo.ulStcSnapshotHiAddr);
                hVip->pCapture->stPicture.ulSTCSnapshotLo = BREG_Read32_isr(hDisplay->hVdc->hRegister, hDisplay->stCurInfo.ulStcSnapshotLoAddr);
                BDBG_MODULE_MSG(BVDC_DISP_VIP, ("[%u] STC: %#x:%#x buf[%u]", hVip->eId, hVip->pCapture->stPicture.ulSTCSnapshotHi,
                    hVip->pCapture->stPicture.ulSTCSnapshotLo, hVip->pCapture->ulBufferId));
            }
        } else {/* if last RUL not executed, free the buffer right away */
            BDBG_MODULE_MSG(BVDC_DISP_VIP, ("[%u] exec = %d slip2lock = %u skip buf[%u]", hVip->eId,
                pList->bLastExecuted, hVip->ulSlip2Lock, hVip->pToCapture->ulBufferId));
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQ, hVip->pToCapture, link);
            BLST_SQ_INSERT_TAIL(&hVip->stFreeQchroma, hVip->pToCaptureChroma, link);
            if(hVip->pToCaptureShifted) {
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQshifted, hVip->pToCaptureShifted, link);
                hVip->pToCaptureShifted = NULL; /* clear the pointer */
            }
            if(hVip->pToCaptureDecim1v) {
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim1v, hVip->pToCaptureDecim1v, link);
                BLST_SQ_INSERT_TAIL(&hVip->stFreeQdecim2v, hVip->pToCaptureDecim2v, link);
                hVip->pToCaptureDecim1v = NULL; /* clear the pointer */
                hVip->pToCaptureDecim2v = NULL; /* clear the pointer */
            }
        }
        hVip->pToCapture = NULL; /* clear the pointer */
        hVip->pToCaptureChroma = NULL; /* clear the pointer */
        hVip->ulPrevSlip2Lock = hVip->ulSlip2Lock;
        hVip->ulSlip2Lock = hDisplay->hCompositor->ulSlip2Lock;
    }

    /* Get buffer from free Q (fifo) first */
    pBuffer = BLST_SQ_FIRST(&hVip->stFreeQ);
    pBufferChroma = BLST_SQ_FIRST(&hVip->stFreeQchroma);
    pBufferShifted = BLST_SQ_FIRST(&hVip->stFreeQshifted);
    pBufferDecim1v = BLST_SQ_FIRST(&hVip->stFreeQdecim1v);
    pBufferDecim2v = BLST_SQ_FIRST(&hVip->stFreeQdecim2v);

    /* if STG or VIP is disabled, put VIP back to auto drain mode */
    if(!hDisplay->stCurInfo.bEnableStg || !hDisplay->stCurInfo.hVipHeap) {
        hVip->stRegs.ulConfig  &= ~(
            BVDC_P_VIP_RDB_MASK(CONFIG, DROP_PROCESS_MODE) |
            BVDC_P_VIP_RDB_MASK(CONFIG,       ENABLE_CTRL));
    }
    /* In cases:
        1) not FULL and
        2.0) RT mode, or
        2.1) non-ignore, or
        2.2) previously non-ignore, but dropped due to FULL (this time may be marked as ignore repeat pic);
        capture it to VIP!
    */
    else if(pBuffer && pBufferChroma &&
            (pBufferShifted || !pFmtInfo->bInterlaced) &&
            ((pBufferDecim1v && pBufferDecim2v) || !hVip->stMemSettings.bDecimatedLuma) &&
            (!hDisplay->stCurInfo.bStgNonRealTime   ||
             !hDisplay->hCompositor->bIgnorePicture || hVip->bPrevNonIgnoreDropByFull)) {
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("[%d] has free buf[%d]..ign=%d, prevNIDBF=%d",
            hVip->eId, pBuffer->ulBufferId, hDisplay->hCompositor->bIgnorePicture, hVip->bPrevNonIgnoreDropByFull));

        /* if previously dropped due to buffer FULL, this time marked as ignore, clear the flag, capture the repeated pic */
        if(hVip->bPrevNonIgnoreDropByFull) {
            hVip->bPrevNonIgnoreDropByFull = false;
            hDisplay->hCompositor->bIgnorePicture = false;
        }

        /* populate the buffer info. */
        pBuffer->stPicture.bDcx          = hVip->stMemSettings.DcxvEnable;
        if (!pFmtInfo->bInterlaced)
        {
           pBuffer->stPicture.bChromaDcx = hVip->stMemSettings.DcxvEnable;
        }
        else
        {
           pBuffer->stPicture.bChromaDcx = false;
        }
#if ((BCHP_CHIP == 7278) && (BCHP_VER == BCHP_VER_A0))
        pBuffer->stPicture.b1VLumaDcx = false;
        pBuffer->stPicture.b2VLumaDcx = false;
#else
        pBuffer->stPicture.b1VLumaDcx = hVip->stMemSettings.DcxvEnable;
        pBuffer->stPicture.b2VLumaDcx = hVip->stMemSettings.DcxvEnable;
#endif

        pBuffer->stPicture.ulWidth       = pFmtInfo->ulDigitalWidth;
        pBuffer->stPicture.ulHeight      = pFmtInfo->ulDigitalHeight>>(pFmtInfo->bInterlaced);
#if BVDC_P_VIP_RDB_MASK(OUTPUT_PICTURE_SIZE, HSIZE_MB)
        pBuffer->stPicture.ulWidthInMbs  = DIV16_ROUNDUP(pFmtInfo->ulDigitalWidth);
        pBuffer->stPicture.ulHeightInMbs = DIV16_ROUNDUP(pFmtInfo->ulDigitalHeight>>(pFmtInfo->bInterlaced));
#ifdef WORKAROUND_HW_ISSUE_VICE2_196
        if (( pBuffer->stPicture.bDcx == true ) && (( pBuffer->stPicture.ulWidthInMbs & 0x3) == 3))
        {
            if (
                ( pBuffer->stPicture.ulWidthInMbs <=  (WORKAROUND_HW_ISSUE_VICE2_196_MAX_NON_DCXV_WIDTH/16) ) &&
                (!pFmtInfo->bInterlaced)  &&
                (pBuffer->stPicture.ulHeightInMbs <= (WORKAROUND_HW_ISSUE_VICE2_196_MAX_NON_DCXV_HEIGHT/16))
               )
            {
                pBuffer->stPicture.bDcx = false;
            }
            else
                pBuffer->stPicture.ulWidthInMbs++;
        }
#endif

#ifdef WORKAROUND_HW_ISSUE_VICE2_210
        if (( pBuffer->stPicture.bDcx ) && (pBuffer->stPicture.ulWidthInMbs >= WORKAROUND_HW_ISSUE_VICE2_210_LOWER_LIMIT) && (pBuffer->stPicture.ulWidthInMbs <= WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT) )
        {
            if (
                ( pBuffer->stPicture.ulWidthInMbs <=  (WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_WIDTH/16) ) &&
                (!pFmtInfo->bInterlaced)  &&
                (pBuffer->stPicture.ulHeightInMbs <= (WORKAROUND_HW_ISSUE_VICE2_196_210_MAX_NON_DCXV_HEIGHT/16)) &&
                (pFmtInfo->ulVertFreq <= BFMT_FREQ_FACTOR * 25)
                )
            {
                pBuffer->stPicture.bDcx = false;
            }
            else
                pBuffer->stPicture.ulWidthInMbs = WORKAROUND_HW_ISSUE_VICE2_210_UPPER_LIMIT + 1;
        }
#endif
#else
        pBuffer->stPicture.ulWidthInMbs  = DIV8_ROUNDUP(pFmtInfo->ulDigitalWidth);
        pBuffer->stPicture.ulHeightInMbs = DIV8_ROUNDUP(pFmtInfo->ulDigitalHeight>>(pFmtInfo->bInterlaced));
#endif
        pBuffer->stPicture.ulOriginalPTS = hDisplay->hCompositor->ulOrigPTS;
        pBuffer->stPicture.ePolarity  = eFieldPolarity;
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
        pBuffer->stPicture.bStriped      = (hVip->eVipDataMode == BVDC_P_VipDataMode_eStripe);
        pBuffer->stPicture.ulStripeWidth = hVip->stMemSettings.DramStripeWidth;
        pBuffer->stPicture.ulLumaNMBY    = BVDC_P_VIP_CalcDcxvNMBY_isr(DIV16_ROUNDUP(pFmtInfo->ulDigitalHeight),
            hVip->stMemSettings.DcxvEnable, pFmtInfo->bInterlaced, false, hVip->stMemSettings.X, hVip->stMemSettings.Y);
        pBuffer->stPicture.ulChromaNMBY  = BVDC_P_VIP_CalcDcxvNMBY_isr(DIV16_ROUNDUP(pFmtInfo->ulDigitalHeight),
            hVip->stMemSettings.DcxvEnable, pFmtInfo->bInterlaced, true, hVip->stMemSettings.X, hVip->stMemSettings.Y);

        /* move from freeQ to intermediate pToCapture pointer */
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQ, link);
        hVip->pToCapture = pBuffer;
        BLST_SQ_REMOVE_HEAD(&hVip->stFreeQchroma, link);
        hVip->pToCaptureChroma = pBufferChroma;
        /* only top field delivers shifted chroma */
        if(BAVC_Polarity_eTopField == eFieldPolarity) {
            BLST_SQ_REMOVE_HEAD(&hVip->stFreeQshifted, link);
            hVip->pToCaptureShifted = pBufferShifted;
        }
        if(hVip->stMemSettings.bDecimatedLuma) {
            BLST_SQ_REMOVE_HEAD(&hVip->stFreeQdecim1v, link);
            hVip->pToCaptureDecim1v = pBufferDecim1v;
            BLST_SQ_REMOVE_HEAD(&hVip->stFreeQdecim2v, link);
            hVip->pToCaptureDecim2v = pBufferDecim2v;
            /* update decimated luma NMBY */
            pBuffer->stPicture.ul1VLumaNMBY = BVDC_P_VIP_CalcDcxvNMBY_isr(DIV16_ROUNDUP(pFmtInfo->ulDigitalHeight),
            hVip->stMemSettings.DcxvEnable, pFmtInfo->bInterlaced, false, hVip->stMemSettings.X, hVip->stMemSettings.Y);
            pBuffer->stPicture.ul2VLumaNMBY = BVDC_P_VIP_CalcDcxvNMBY_isr(DIV16_ROUNDUP(pFmtInfo->ulDigitalHeight),
            hVip->stMemSettings.DcxvEnable, pFmtInfo->bInterlaced, false, hVip->stMemSettings.X, hVip->stMemSettings.Y);
        }

        BDBG_MODULE_MSG(BVDC_DISP_VIP,("[%u] display %d stg_id %d:", hVip->eId, hDisplay->eId, hDisplay->stStgChan.ulStg));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("%dx%d%c%d", pFmtInfo->ulDigitalWidth, pFmtInfo->ulDigitalHeight,
            (pBuffer->stPicture.ePolarity==BAVC_Polarity_eFrame)? 'p' : 'i', hDisplay->stCurInfo.ulVertFreq));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("stall STC? %d", hDisplay->hCompositor->bStallStc));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("repeat? %d", hDisplay->hCompositor->bPictureRepeatFlag));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("PicId %d DecodePicId %d type %d, pol %d, asp ratio 0x%x, origPTS 0x%x",
            pBuffer->stPicture.ulPictureId, hDisplay->hCompositor->ulDecodePictureId,
            hDisplay->hCompositor->ePictureType, eFieldPolarity,
            hDisplay->hCompositor->ulStgPxlAspRatio_x_y, pBuffer->stPicture.ulOriginalPTS));
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("BUILD buf %u:%u:%u:%u:%u", pBuffer->ulBufferId, pBufferChroma->ulBufferId,
            pBufferShifted?pBufferShifted->ulBufferId:0, pBufferDecim1v?pBufferDecim1v->ulBufferId:0, pBufferDecim2v?pBufferDecim2v->ulBufferId:0));

        /* set FW_CONTROL */
        hVip->stRegs.ulFwControl = (
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, PIC_START,   START) |
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_RD_HIST, DONE) |
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_RD_PCC,  DONE) |
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_WR_REG,  DONE) );


        /* set VIP config. TODO: add decimated luma, interlaced and itfp support */
        hVip->stRegs.ulConfig &= ~(
            BVDC_P_VIP_RDB_MASK(CONFIG,       ENABLE_CTRL) |
#if BVDC_P_VIP_RDB_MASK(CONFIG,   LUMA_DCMH2V)
            BVDC_P_VIP_RDB_MASK(CONFIG,       LUMA_DCMH2V) |
            BVDC_P_VIP_RDB_MASK(CONFIG,       LUMA_DCMH1V) |
#else
            BVDC_P_VIP_RDB_MASK(CONFIG,       LUMA_2H2V) |
            BVDC_P_VIP_RDB_MASK(CONFIG,       LUMA_2H1V) |
#endif
            BVDC_P_VIP_RDB_MASK(CONFIG,      SHIFT_CHROMA) |
#if BVDC_P_VIP_RDB_MASK(CONFIG,   LUMA_DCMH)
            BVDC_P_VIP_RDB_MASK(CONFIG,         LUMA_DCMH) |
#endif
            BVDC_P_VIP_RDB_MASK(CONFIG, DRAM_STRIPE_WIDTH) |
            BVDC_P_VIP_RDB_MASK(CONFIG, DROP_PROCESS_MODE) |
#if BVDC_P_VIP_RDB_MASK(CONFIG,   ADDRESSING_MODE)
            BVDC_P_VIP_RDB_MASK(CONFIG,   ADDRESSING_MODE) |
#endif
#if BVDC_P_VIP_RDB_MASK(CONFIG,   HISTOGRAM)
            BVDC_P_VIP_RDB_MASK(CONFIG,   HISTOGRAM) |
#endif
            BVDC_P_VIP_RDB_MASK(CONFIG,        INPUT_TYPE) );

        hVip->stRegs.ulConfig |= (
            BVDC_P_VIP_RDB_ENUM(CONFIG, ENABLE_CTRL,      ENABLE_BY_PICTURE) |
            BVDC_P_VIP_RDB_DATA(CONFIG, SHIFT_CHROMA, (BAVC_Polarity_eTopField==eFieldPolarity)?1:0) |
            BVDC_P_VIP_RDB_DATA(CONFIG, DRAM_STRIPE_WIDTH,  (pBuffer->stPicture.ulStripeWidth==128)) |
            BVDC_P_VIP_RDB_ENUM(CONFIG, DROP_PROCESS_MODE,          PROCESS) |
#if BVDC_P_VIP_RDB_MASK(CONFIG,   ADDRESSING_MODE)
            BVDC_P_VIP_RDB_DATA(CONFIG, ADDRESSING_MODE,  (uint32_t)hVip->eVipDataMode) |
#endif
            BVDC_P_VIP_RDB_DATA(CONFIG, INPUT_TYPE, eFieldPolarity) );

        if(hVip->stMemSettings.bDecimatedLuma) {
             hVip->stRegs.ulConfig |= (
#if BVDC_P_VIP_RDB_MASK(CONFIG,   HISTOGRAM)
                BVDC_P_VIP_RDB_DATA(CONFIG, HISTOGRAM, 0!=(pFmtInfo->bInterlaced? hVip->prevPrev1VLumaOffset : hVip->prev1VLumaOffset)) |
                BVDC_P_VIP_RDB_DATA(CONFIG, PCC,       0!=(pFmtInfo->bInterlaced? hVip->prevPrev1VLumaOffset : hVip->prev1VLumaOffset)) |
#endif
#if BVDC_P_VIP_RDB_MASK(CONFIG,   LUMA_DCMH2V)
                BVDC_P_VIP_RDB_ENUM(CONFIG, HISTOGRAM_SEL, LUMA_DCMH1V) |
                BVDC_P_VIP_RDB_ENUM(CONFIG, PCC_SEL,       LUMA_DCMH1V) |
                BVDC_P_VIP_RDB_ENUM(CONFIG, LUMA_DCMH2V,    ENABLE) |
                BVDC_P_VIP_RDB_ENUM(CONFIG, LUMA_DCMH1V,    ENABLE)
#else
                BVDC_P_VIP_RDB_ENUM(CONFIG, HISTOGRAM_SEL, LUMA_DCM2H1V) |
                BVDC_P_VIP_RDB_ENUM(CONFIG, PCC_SEL,       LUMA_DCM2H1V) |
                BVDC_P_VIP_RDB_ENUM(CONFIG, LUMA_2H2V,      ENABLE) |
                BVDC_P_VIP_RDB_ENUM(CONFIG, LUMA_2H1V,      ENABLE)
#endif
#if BVDC_P_VIP_RDB_MASK(CONFIG,   LUMA_DCMH)
                |
                BVDC_P_VIP_RDB_DATA(CONFIG, LUMA_DCMH,
                    (pBuffer->stPicture.ulWidth  > BVDC_P_VIP_MAX_H1V_WIDTH ||
                     pBuffer->stPicture.ulHeight > BVDC_P_VIP_MAX_H1V_HEIGHT)? 1 : 0)
#endif
            );

#ifdef BCHP_VICE_VIP_0_0_DCMH1V_BASE
             /* luma 2H1V address */
              hVip->stRegs.ull1VLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(DCMH1V_BASE, ADDR, hVip->ullDeviceOffset + pBufferDecim1v->ulOffset));

             /* luma 2H1V nmby */
              hVip->stRegs.ul1VLumaNMBY =  (
                 BVDC_P_VIP_RDB_DATA(DCMH1V_CFG, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
                 BVDC_P_VIP_RDB_DATA(DCMH1V_CFG, NMBY, pBuffer->stPicture.ul1VLumaNMBY));

             /* luma 2H2V address */
              hVip->stRegs.ull2VLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(DCMH2V_BASE, ADDR, hVip->ullDeviceOffset + pBufferDecim2v->ulOffset));

             /* luma 2H2V nmby */
              hVip->stRegs.ul2VLumaNMBY =  (
                 BVDC_P_VIP_RDB_DATA(DCMH2V_CFG, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
                 BVDC_P_VIP_RDB_DATA(DCMH2V_CFG, NMBY, pBuffer->stPicture.ul2VLumaNMBY));

             /* pcc/hist previous luma address */
              hVip->stRegs.ullPccLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(PCC_LUMA_BASE, ADDR, hVip->ullDeviceOffset + hVip->prev1VLumaOffset));
              hVip->stRegs.ullHistLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(HIST_LUMA_BASE, ADDR, hVip->ullDeviceOffset +
                    (pFmtInfo->bInterlaced? hVip->prevPrev1VLumaOffset : hVip->prev1VLumaOffset)));
#elif BVDC_P_VIP_RDB_MASK(CONFIG,   LUMA_DCMH2V)
            /* luma 2H1V address */
             hVip->stRegs.ull1VLumaAddr =  (
                BVDC_P_VIP_RDB_DATA(DCMH1V_ADDR, ADDR, hVip->ullDeviceOffset + pBufferDecim1v->ulOffset));

            /* luma 2H1V nmby */
             hVip->stRegs.ul1VLumaNMBY =  (
                BVDC_P_VIP_RDB_DATA(DCMH1V_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
                BVDC_P_VIP_RDB_DATA(DCMH1V_NMBY, NMBY, pBuffer->stPicture.ul1VLumaNMBY));

            /* luma 2H2V address */
             hVip->stRegs.ull2VLumaAddr =  (
                BVDC_P_VIP_RDB_DATA(DCMH2V_ADDR, ADDR, hVip->ullDeviceOffset + pBufferDecim2v->ulOffset));

            /* luma 2H2V nmby */
             hVip->stRegs.ul2VLumaNMBY =  (
                BVDC_P_VIP_RDB_DATA(DCMH2V_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
                BVDC_P_VIP_RDB_DATA(DCMH2V_NMBY, NMBY, pBuffer->stPicture.ul2VLumaNMBY));

             /* pcc/hist previous luma address */
              hVip->stRegs.ullPccLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(PCC_LUMA_ADDR, ADDR, hVip->ullDeviceOffset + hVip->prev1VLumaOffset));
              hVip->stRegs.ullHistLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(HIST_LUMA_ADDR, ADDR, hVip->ullDeviceOffset +
                    (pFmtInfo->bInterlaced? hVip->prevPrev1VLumaOffset : hVip->prev1VLumaOffset)));
#else
            /* luma 2H1V address */
             hVip->stRegs.ull1VLumaAddr =  (
                BVDC_P_VIP_RDB_DATA(2H1V_ADDR, ADDR, hVip->ullDeviceOffset + pBufferDecim1v->ulOffset));

            /* luma 2H1V nmby */
             hVip->stRegs.ul1VLumaNMBY =  (
                BVDC_P_VIP_RDB_DATA(2H1V_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
                BVDC_P_VIP_RDB_DATA(2H1V_NMBY, NMBY, pBuffer->stPicture.ul1VLumaNMBY));

            /* luma 2H2V address */
             hVip->stRegs.ull2VLumaAddr =  (
                BVDC_P_VIP_RDB_DATA(2H2V_ADDR, ADDR, hVip->ullDeviceOffset + pBufferDecim2v->ulOffset));

            /* luma 2H2V nmby */
             hVip->stRegs.ul2VLumaNMBY =  (
                BVDC_P_VIP_RDB_DATA(2H2V_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
                BVDC_P_VIP_RDB_DATA(2H2V_NMBY, NMBY, pBuffer->stPicture.ul2VLumaNMBY));

             /* pcc/hist luma address */
              hVip->stRegs.ullPccLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(PCC_LUMA_ADDR, ADDR, hVip->ullDeviceOffset + hVip->prev1VLumaOffset));
              hVip->stRegs.ullHistLumaAddr =  (
                 BVDC_P_VIP_RDB_DATA(HIST_LUMA_ADDR, ADDR, hVip->ullDeviceOffset +
                    (pFmtInfo->bInterlaced? hVip->prevPrev1VLumaOffset : hVip->prev1VLumaOffset)));
#endif

             /* pcc/hist line range */
              hVip->stRegs.ulStatsLineRange =
#if BVDC_P_VIP_RDB_MASK(PCC_LINE_RANGE, START)
                 BVDC_P_VIP_RDB_DATA(PCC_LINE_RANGE, START, (16>>pFmtInfo->bInterlaced)-1) |
                 BVDC_P_VIP_RDB_DATA(PCC_LINE_RANGE, END, 0xFFF);
#else
                 BVDC_P_VIP_RDB_DATA(PCC_LINE_RANGE, START_LINE, 0) |
                 BVDC_P_VIP_RDB_DATA(PCC_LINE_RANGE, END_LINE, (pFmtInfo->ulDigitalHeight - 16) >> pFmtInfo->bInterlaced);
#endif

            /* store the previous frame's decimated luma offset for ITFP */
            hVip->prevPrev1VLumaOffset = hVip->prev1VLumaOffset; /* interlaced previous frame */
            hVip->prev1VLumaOffset = pBufferDecim1v->ulOffset; /* progressive previous frame */
        }

        /* set VIP input pixel size */
        hVip->stRegs.ulInputPicSize =  (
            BVDC_P_VIP_RDB_DATA(INPUT_PICTURE_SIZE, HSIZE, pBuffer->stPicture.ulWidth) |
            BVDC_P_VIP_RDB_DATA(INPUT_PICTURE_SIZE, VSIZE, pBuffer->stPicture.ulHeight));

        /* set output MB size */
#if BVDC_P_VIP_RDB_MASK(OUTPUT_PICTURE_SIZE, HSIZE_MB)
        hVip->stRegs.ulOutputPicSize =  (
            BVDC_P_VIP_RDB_DATA(OUTPUT_PICTURE_SIZE, HSIZE_MB, pBuffer->stPicture.ulWidthInMbs) |
            BVDC_P_VIP_RDB_DATA(OUTPUT_PICTURE_SIZE, VSIZE_MB, pBuffer->stPicture.ulHeightInMbs));
#else
        hVip->stRegs.ulOutputPicSize =  (
            BVDC_P_VIP_RDB_DATA(OUTPUT_PICTURE_SIZE, HSIZE_8X8, pBuffer->stPicture.ulWidthInMbs) |
            BVDC_P_VIP_RDB_DATA(OUTPUT_PICTURE_SIZE, VSIZE_8X8, pBuffer->stPicture.ulHeightInMbs));
#endif


#if BCHP_VICE_VIP_0_0_DCMH1V_BASE
        /* luma address */
        hVip->stRegs.ullLumaAddr =  (
            BVDC_P_VIP_RDB_DATA(LUMA_BASE, ADDR, hVip->ullDeviceOffset + pBuffer->stPicture.ulLumaOffset));

        /* luma nmby */
        hVip->stRegs.ulLumaNMBY =  (
            BVDC_P_VIP_RDB_DATA(LUMA_CFG, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
            BVDC_P_VIP_RDB_DATA(LUMA_CFG, NMBY, pBuffer->stPicture.ulLumaNMBY));
#else
        /* luma address */
        hVip->stRegs.ullLumaAddr =  (
            BVDC_P_VIP_RDB_DATA(LUMA_ADDR, ADDR, hVip->ullDeviceOffset + pBuffer->stPicture.ulLumaOffset));

        /* luma nmby */
        hVip->stRegs.ulLumaNMBY =  (
            BVDC_P_VIP_RDB_DATA(LUMA_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
            BVDC_P_VIP_RDB_DATA(LUMA_NMBY, NMBY, pBuffer->stPicture.ulLumaNMBY));
#endif

        /* chroma address */
#if BCHP_VICE_VIP_0_0_DCMH1V_BASE
        hVip->stRegs.ullChromaAddr =  (
            BVDC_P_VIP_RDB_DATA(CHROMA_420_BASE, ADDR, hVip->ullDeviceOffset + pBufferChroma->ulOffset));

        /* chroma nmby */
        hVip->stRegs.ulChromaNMBY =  (
            BVDC_P_VIP_RDB_DATA(CHROMA_420_CFG, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
            BVDC_P_VIP_RDB_DATA(CHROMA_420_CFG, NMBY,    pBuffer->stPicture.ulChromaNMBY));
#elif BVDC_P_VIP_RDB_MASK(CHROMA_420_ADDR, ADDR)
        hVip->stRegs.ullChromaAddr =  (
            BVDC_P_VIP_RDB_DATA(CHROMA_420_ADDR, ADDR, hVip->ullDeviceOffset + pBufferChroma->ulOffset));

        /* chroma nmby */
        hVip->stRegs.ulChromaNMBY =  (
            BVDC_P_VIP_RDB_DATA(CHROMA_420_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
            BVDC_P_VIP_RDB_DATA(CHROMA_420_NMBY, NMBY,    pBuffer->stPicture.ulChromaNMBY));
#else
        hVip->stRegs.ullChromaAddr =  (
            BVDC_P_VIP_RDB_DATA(420_CHROMA_ADDR, ADDR, hVip->ullDeviceOffset + pBufferChroma->ulOffset));

        /* chroma nmby */
        hVip->stRegs.ulChromaNMBY =  (
            BVDC_P_VIP_RDB_DATA(420_CHROMA_NMBY, LINE_STRIDE_MODE, !pFmtInfo->bInterlaced) |
            BVDC_P_VIP_RDB_DATA(420_CHROMA_NMBY, NMBY,    pBuffer->stPicture.ulChromaNMBY));
#endif
        /* interlaced shifted chroma address */
        if(BAVC_Polarity_eTopField == eFieldPolarity) {
           /* The shifted chroma is stored only for top field and it's a frame chroma so it's stored as a frame */
#if BCHP_VICE_VIP_0_0_DCMH1V_BASE
           hVip->stRegs.ullShiftedChromaAddr =  (
                BVDC_P_VIP_RDB_DATA(SHIFT_CHROMA_BASE, ADDR, hVip->ullDeviceOffset + pBufferShifted->ulOffset));

            /* shifted chroma nmby */
           hVip->stRegs.ulShiftedChromaNMBY =  (
                BVDC_P_VIP_RDB_ENUM(SHIFT_CHROMA_CFG, LINE_STRIDE_MODE,               FRAME) |
                BVDC_P_VIP_RDB_DATA(SHIFT_CHROMA_CFG, NMBY, pBuffer->stPicture.ulChromaNMBY));
#else
           hVip->stRegs.ullShiftedChromaAddr =  (
                BVDC_P_VIP_RDB_DATA(SHIFT_CHROMA_ADDR, ADDR, hVip->ullDeviceOffset + pBufferShifted->ulOffset));

            /* shifted chroma nmby */
           hVip->stRegs.ulShiftedChromaNMBY =  (
                BVDC_P_VIP_RDB_ENUM(SHIFT_CHROMA_NMBY, LINE_STRIDE_MODE,               FRAME) |
                BVDC_P_VIP_RDB_DATA(SHIFT_CHROMA_NMBY, NMBY, pBuffer->stPicture.ulChromaNMBY));
#endif
        }

        /* DCXV config */
#if BVDC_P_VIP_RDB_MASK(DCXV_CFG, MODE)
        hVip->stRegs.ulDcxvCfg =  (
            BVDC_P_VIP_RDB_DATA(DCXV_CFG, MODE, hVip->stMemSettings.DcxvEnable));
#elif BVDC_P_VIP_RDB_MASK(DCXV_CFG, MODE_IMD_PATH)
        hVip->stRegs.ulDcxvCfg =  (
            BVDC_P_VIP_RDB_DATA(DCXV_CFG, MODE_CME_PATH, pBuffer->stPicture.b1VLumaDcx) |
            BVDC_P_VIP_RDB_DATA(DCXV_CFG, MODE_IMD_PATH, pBuffer->stPicture.bDcx)
            );
#endif

        /* build RUL */
        BDBG_CASSERT(2 == (((BVDC_P_VIP_RDB(OUTPUT_PICTURE_SIZE) - BVDC_P_VIP_RDB(INPUT_PICTURE_SIZE)) / sizeof(uint32_t)) + 1));
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BVDC_P_VIP_RDB(OUTPUT_PICTURE_SIZE) - BVDC_P_VIP_RDB(INPUT_PICTURE_SIZE)) / sizeof(uint32_t)) + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(BVDC_P_VIP_RDB(INPUT_PICTURE_SIZE) + hVip->ulRegOffset);
        *pList->pulCurrent++ = hVip->stRegs.ulInputPicSize;
        *pList->pulCurrent++ = hVip->stRegs.ulOutputPicSize;

        BVDC_P_VIP_WRITE_TO_RUL(PCC_LINE_RANGE, pList->pulCurrent, hVip->stRegs.ulStatsLineRange);
        BVDC_P_VIP_WRITE_TO_RUL(HIST_LINE_RANGE, pList->pulCurrent, hVip->stRegs.ulStatsLineRange);
#if BCHP_VICE_VIP_0_0_DCMH1V_BASE
        BVDC_P_VIP_WRITE_TO_RUL(LUMA_CFG, pList->pulCurrent,
            hVip->stRegs.ulLumaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(CHROMA_420_CFG, pList->pulCurrent,
            hVip->stRegs.ulChromaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(DCMH2V_CFG, pList->pulCurrent,
              hVip->stRegs.ul2VLumaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(DCMH1V_CFG, pList->pulCurrent,
              hVip->stRegs.ul1VLumaNMBY);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(DCMH1V_BASE) + hVip->ulRegOffset,
            hVip->stRegs.ull1VLumaAddr);
        if(BAVC_Polarity_eTopField == eFieldPolarity) {
            BVDC_P_VIP_WRITE_TO_RUL(SHIFT_CHROMA_CFG, pList->pulCurrent,
                  hVip->stRegs.ulShiftedChromaNMBY);
            BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
                  BVDC_P_VIP_RDB(SHIFT_CHROMA_BASE) + hVip->ulRegOffset,
                hVip->stRegs.ullShiftedChromaAddr);
        }
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(LUMA_BASE) + hVip->ulRegOffset,
            hVip->stRegs.ullLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(CHROMA_420_BASE) + hVip->ulRegOffset,
            hVip->stRegs.ullChromaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(DCMH2V_BASE) + hVip->ulRegOffset,
            hVip->stRegs.ull2VLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(PCC_LUMA_BASE) + hVip->ulRegOffset,
              hVip->stRegs.ullPccLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(HIST_LUMA_BASE) + hVip->ulRegOffset,
              hVip->stRegs.ullHistLumaAddr);
#else
        BVDC_P_VIP_WRITE_TO_RUL(LUMA_NMBY, pList->pulCurrent,
            hVip->stRegs.ulLumaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(CHROMA_420_NMBY, pList->pulCurrent,
            hVip->stRegs.ulChromaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(DCMH1V_NMBY, pList->pulCurrent,
              hVip->stRegs.ul1VLumaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(DCMH2V_NMBY, pList->pulCurrent,
              hVip->stRegs.ul2VLumaNMBY);
        BVDC_P_VIP_WRITE_TO_RUL(SHIFT_CHROMA_NMBY, pList->pulCurrent,
              hVip->stRegs.ulShiftedChromaNMBY);

        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(LUMA_ADDR) + hVip->ulRegOffset,
            hVip->stRegs.ullLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(CHROMA_420_ADDR) + hVip->ulRegOffset,
            hVip->stRegs.ullChromaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(DCMH1V_ADDR) + hVip->ulRegOffset,
            hVip->stRegs.ull1VLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(DCMH2V_ADDR) + hVip->ulRegOffset,
            hVip->stRegs.ull2VLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(SHIFT_CHROMA_ADDR) + hVip->ulRegOffset,
            hVip->stRegs.ullShiftedChromaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(PCC_LUMA_ADDR) + hVip->ulRegOffset,
              hVip->stRegs.ullPccLumaAddr);
        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent,
              BVDC_P_VIP_RDB(HIST_LUMA_ADDR) + hVip->ulRegOffset,
              hVip->stRegs.ullHistLumaAddr);
#endif

        if(hVip->bInitial) {
            BVDC_P_VIP_WRITE_TO_RUL(ERR_STATUS_ENABLE, pList->pulCurrent, 0x3FF);

#if BVDC_P_VIP_RDB_MASK(DCXV_CFG, MODE) /* could be dynamic for some workaround cases */
        BVDC_P_VIP_WRITE_TO_RUL(DCXV_CFG, pList->pulCurrent,
              hVip->stRegs.ulDcxvCfg);
#elif BVDC_P_VIP_RDB_MASK(DCXV_CFG, MODE_IMD_PATH) /* could be dynamic for some workaround cases */
        BVDC_P_VIP_WRITE_TO_RUL(DCXV_CFG, pList->pulCurrent,
              hVip->stRegs.ulDcxvCfg);
#endif

#if BVDC_P_VIP_RDB_MASK(BVB_PADDING_DATA, LUMA) /* could be dynamic for some workaround cases */
        BVDC_P_VIP_WRITE_TO_RUL(BVB_PADDING_DATA, pList->pulCurrent,
              0);
#endif

        hVip->bInitial = false;
        }
    } else /* freeQ empty: drop the picture (RT and NRT mode) */
    {
        BDBG_MODULE_MSG(BVDC_DISP_VIP,("[%d] drop pic%u[pBuf=%p:%p:%p:%p:%p][ign=%d]", hVip->eId, hDisplay->hCompositor->ulPicId, (void *)pBuffer,
            (void *)pBufferChroma, (void *)pBufferShifted, (void *)pBufferDecim1v, (void *)pBufferDecim2v, hDisplay->hCompositor->bIgnorePicture));
        /* set VIP config. drop it */
        hVip->stRegs.ulConfig &= ~(
            BVDC_P_VIP_RDB_MASK(CONFIG,       ENABLE_CTRL) |
            BVDC_P_VIP_RDB_MASK(CONFIG, DROP_PROCESS_MODE) |
            BVDC_P_VIP_RDB_MASK(CONFIG,        INPUT_TYPE) );

        hVip->stRegs.ulConfig |= (
            BVDC_P_VIP_RDB_ENUM(CONFIG, ENABLE_CTRL, ENABLE_BY_PICTURE) |
            BVDC_P_VIP_RDB_ENUM(CONFIG, DROP_PROCESS_MODE,        DROP) |
            BVDC_P_VIP_RDB_DATA(CONFIG, INPUT_TYPE,     eFieldPolarity) );

        /* set FW_CONTROL */
        hVip->stRegs.ulFwControl = (
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, PIC_START,   START) |
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_RD_HIST, DONE) |
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_RD_PCC,  DONE) |
            BVDC_P_VIP_RDB_ENUM(FW_CONTROL, DONE_WR_REG,  DONE) );

        /* set VIP input pixel size */
        hVip->stRegs.ulInputPicSize =  (
            BVDC_P_VIP_RDB_DATA(INPUT_PICTURE_SIZE, HSIZE, pFmtInfo->ulDigitalWidth) |
            BVDC_P_VIP_RDB_DATA(INPUT_PICTURE_SIZE, VSIZE, (pFmtInfo->ulDigitalHeight>>pFmtInfo->bInterlaced)));

        BVDC_P_VIP_WRITE_TO_RUL(INPUT_PICTURE_SIZE, pList->pulCurrent, hVip->stRegs.ulInputPicSize);

        if((pBuffer == NULL || pBufferChroma == NULL ||
            (pBufferShifted == NULL && pFmtInfo->bInterlaced) ||
            ((pBufferDecim1v == NULL || pBufferDecim2v == NULL) && hVip->stMemSettings.bDecimatedLuma)) &&
            hDisplay->stCurInfo.bStgNonRealTime) {/* if FULL, stall STC in NRT mode since it's dropped! */
            hDisplay->hCompositor->bStallStc = true;
            /* if dropping non-ignore pic due to buffer FULL, mark it to capture later when DM marks it as ignroe next time! */
            if(!hDisplay->hCompositor->bIgnorePicture) {
                hVip->bPrevNonIgnoreDropByFull = true;
            }
        }
        /* mark current picture as ignore since it's dropped! effect on NRT mode STG trigger mode and RepatPolarity! */
        hDisplay->hCompositor->bIgnorePicture = true;
    }

    BVDC_P_VIP_WRITE_TO_RUL(CONFIG, pList->pulCurrent, hVip->stRegs.ulConfig);
    BVDC_P_VIP_WRITE_TO_RUL(FW_CONTROL, pList->pulCurrent, hVip->stRegs.ulFwControl);
    BDBG_LEAVE(BVDC_P_Vip_BuildRul_isr);
    return;
}

#endif

/* End of file */
