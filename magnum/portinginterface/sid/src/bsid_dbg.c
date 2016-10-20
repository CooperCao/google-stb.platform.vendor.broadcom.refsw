/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bdbg.h"

#include "bsid.h"
#include "bsid_dbg.h"
#include "bsid_priv.h"

#if defined(BSID_P_DEBUG_SAVE_BUFFER) || defined(BSID_P_DEBUG_TRACE_PLAYBACK) || defined(BSID_P_DEBUG_FW_DUMP_TO_FILE)
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

BDBG_MODULE(BSID_DEBUG);

/*******************************************************************************
  Static Funcs
********************************************************************************/
#ifdef BSID_P_DEBUG_SAVE_BUFFER
static void *fSaveImagesThread(void *pCtx)
{
    BSID_ChannelHandle hSidCh = (BSID_ChannelHandle)pCtx;

    while (true)
    {
        BKNI_EnterCriticalSection();
        BSID_P_UpdatePlaybackInfo_isr(hSidCh);
        BKNI_LeaveCriticalSection();
        BKNI_Sleep(30);
    }

    return NULL;
}

#ifdef BSID_P_DEBUG_SAVE_AS_YUV
static void SaveOutputImageAsYuyv(
    BSID_ChannelHandle hSidCh,
    void *output_buffer_address,
    uint32_t output_buffer_width,
    uint32_t output_buffer_pitch,
    uint32_t output_buffer_height,
    char *file_path)
{
    FILE *pFile;
    void *bufAddr = NULL;
    BSTD_UNUSED(hSidCh);

    BSTD_UNUSED(output_buffer_width);

    pFile = fopen(file_path, "wb");
    if (pFile == NULL)
    {
        BDBG_ERR(("SaveOutputImageAsYuyv: failed to open pOutBuf file"));
        return;
    }

    bufAddr = output_buffer_address;

    fwrite(bufAddr, sizeof(uint8_t), output_buffer_pitch * output_buffer_height, pFile);

    fclose(pFile);

    return;
}

#else /* save as PPM */

#define shift 10
/* BT.709 -> sRGB */
/* R = Y * 1.164 + Cb * 0.000 + Cr * 1.787 - 247 */
/* G = Y * 1.164 - Cb * 0.531 - Cr * 0.222 + 76 */
/* B = Y * 1.164 + Cb * 2.112 + Cr * 0.000 - 289 */
static const int64_t coeff[3][4] = {
    {(int64_t)(1.164*(1<<shift)), (int64_t)( 0.000*(1<<shift)), (int64_t)(  1.787*(1<<shift)), (int64_t)(-247.000*(1<<shift))},
    {(int64_t)(1.164*(1<<shift)), (int64_t)(-0.531*(1<<shift)), (int64_t)( -0.222*(1<<shift)), (int64_t)(  76.000*(1<<shift))},
    {(int64_t)(1.164*(1<<shift)), (int64_t)( 2.112*(1<<shift)), (int64_t)(  0.000*(1<<shift)), (int64_t)(-289.000*(1<<shift))}
};

static void SaveOutputImageAsPpm(
    BSID_ChannelHandle hSidCh,
    void *output_buffer_address,
    uint32_t output_buffer_width,
    uint32_t output_buffer_pitch,
    uint32_t output_buffer_height,
    char *file_path)
{
    FILE *pFile;
    void *bufAddr = NULL;
    uint32_t *bufAddr32 = NULL;
    uint32_t *pPixel;
    uint32_t Y08Cb8;
    uint32_t Y18Cr8;
    uint32_t Y, Cb, Cr;
    int64_t R, G, B;
    uint32_t i = 0;
    uint32_t n = 0;
    char header[30];
    uint8_t *pImage;
    uint8_t *pImageBase;
    BSTD_UNUSED(hSidCh);

    pFile = fopen(file_path, "wb");
    if (pFile == NULL)
    {
        BDBG_ERR(("SaveOutputImageAsPpm: failed to open pOutBuf file"));
        return;
    }

    bufAddr = output_buffer_address;

    pImageBase = pImage = (uint8_t *)BKNI_Malloc(output_buffer_width*output_buffer_height*3);
    if (pImageBase == NULL)
    {
        BDBG_WRN(("SaveOutputImageAsPpm: No memory to save Output Image"));
        fclose(pFile);
        return;
    }

    bufAddr32 = (uint32_t *)bufAddr;

    snprintf(header, 30, "P6\n%d %d\n255\n", output_buffer_width, output_buffer_height);

    fwrite(header, 1, strlen(header), pFile);

    i = 0;
    n = 0;
    do
    {
        /*BKNI_Printf("\ni=%d\n", i);*/
        pPixel = bufAddr32 + i;
        Y08Cb8 = ((*pPixel) & 0xFFFF);
        Y18Cr8 = ((*pPixel) & 0xFFFF0000)>>16;

        /*BKNI_Printf("Pixel 0x%x, YCb 0x%x YCr 0x%x\n", *pPixel, Y08Cb8, Y18Cr8);*/

        Y  = (Y08Cb8>>8)&0xFF;
        Cb = Y08Cb8 & 0xFF;
        Cr = Y18Cr8 & 0xFF;

        /* BT.601 -> sRGB */
        /* R = Y * 1.164 + Cb * 0.000 + Cr * 1.596 - 223 */
        /* G = Y * 1.164 - Cb * 0.391 - Cr * 0.813 + 135 */
        /* B = Y * 1.164 + Cb * 2.018 + Cr * 0.000 - 277 */
        R = ((Y*coeff[0][0]) + (Cb*coeff[0][1]) + (Cr*coeff[0][2]) + coeff[0][3])/(1<<shift);
        G = ((Y*coeff[1][0]) + (Cb*coeff[1][1]) + (Cr*coeff[1][2]) + coeff[1][3])/(1<<shift);
        B = ((Y*coeff[2][0]) + (Cb*coeff[2][1]) + (Cr*coeff[2][2]) + coeff[2][3])/(1<<shift);
        *pImage = (uint8_t)R; pImage++;
        *pImage = (uint8_t)G; pImage++;
        *pImage = (uint8_t)B; pImage++;

        Y  = (Y18Cr8>>8)&0xFF;
        Cb = Y08Cb8 & 0xFF;
        Cr = Y18Cr8 & 0xFF;

        /*BKNI_Printf("AY1CbCr 0x%x\n", AYCbCr);*/

        R = ((Y*coeff[0][0]) + (Cb*coeff[0][1]) + (Cr*coeff[0][2]) + coeff[0][3])/(1<<shift);
        G = ((Y*coeff[1][0]) + (Cb*coeff[1][1]) + (Cr*coeff[1][2]) + coeff[1][3])/(1<<shift);
        B = ((Y*coeff[2][0]) + (Cb*coeff[2][1]) + (Cr*coeff[2][2]) + coeff[2][3])/(1<<shift);
        *pImage = (uint8_t)R; pImage++;
        *pImage = (uint8_t)G; pImage++;
        *pImage = (uint8_t)B; pImage++;

        n+=6;

        i++;
    } while (i < ((output_buffer_pitch * output_buffer_height)>>2));

    fwrite(pImageBase, sizeof(uint8_t), n, pFile);

    fclose(pFile);

    BKNI_Free(pImageBase);

    BKNI_Printf("Image %s saved\n", file_path);

    return;
}
#endif /* BSID_P_DEBUG_SAVE_AS_YUV */

static void SaveCdb(
    BSID_ChannelHandle hSidCh,
    uint32_t input_buffer_offset[2],
    uint32_t input_buffer_size[2],
    uint32_t num_input_buffer,
    char *file_path)
{
    BERR_Code retCode = BERR_SUCCESS;
    FILE *pFile;
    void *bufAddr = NULL;
    unsigned int i = 0;

    /* save input */
    pFile = fopen(file_path, "wb");
    if (pFile == NULL)
    {
        BDBG_ERR(("SaveCdb: failed to open pOutBuf file"));
        return;
    }

    for (i = 0; i < num_input_buffer; i++)
    {
        retCode = BMEM_ConvertOffsetToAddress(hSidCh->hChMemHeap, input_buffer_offset[i], &bufAddr);
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BMEM_ConvertAddressToOffset returned with error 0x%x", retCode));
            return;
        }

        fwrite(bufAddr, sizeof(uint8_t), input_buffer_size[i], pFile);
    }

    fclose(pFile);

    return;
}
#endif /* BSID_P_DEBUG_SAVE_BUFFER */

#ifdef BSID_P_DEBUG_SAVE_BUFFER
/******************************************************************************
* Function name: BSID_P_Debug_CreateSaveImagesThread
*
* Comments:
*
******************************************************************************/
void BSID_P_Debug_CreateSaveImagesThread(BSID_ChannelHandle  hSidCh)
{
   pthread_create(&hSidCh->stDebugSaveData.SaveImagesThread, NULL, &fSaveImagesThread, (void *)hSidCh);
}

/******************************************************************************
* Function name: BSID_P_Debug_SaveImageData
*
* Comments:
*
******************************************************************************/
/* NOTE: at the point this is called, it is invoked from an API that is
   NOT actually being called from within an ISR context, so these do NOT
   need _isr suffix, in theory */
void BSID_P_Debug_SaveImageData(
    BSID_ChannelHandle  hSidCh,
    uint32_t image_index,
    void *output_buffer_address,
    uint32_t output_buffer_width,
    uint32_t output_buffer_pitch,
    uint32_t output_buffer_height,
    uint32_t input_buffer_offset[2],
    uint32_t input_buffer_size[2],
    uint32_t num_input_buffer)
{
    char path[] ="/tmp/";
    char file_path[256]="";
    char out_name[100]="out";
    char cdb_name[100]="cdb";

#ifdef BSID_P_DEBUG_SAVE_AS_YUV
    sprintf(out_name, "%s.%d.yuyv", out_name, image_index);
#else
    sprintf(out_name, "%s.%d.ppm", out_name, image_index);
#endif
    strcat(file_path, path);
    strcat(file_path, out_name);

    /* save output */
#ifdef BSID_P_DEBUG_SAVE_AS_YUV
    SaveOutputImageAsYuyv(
        hSidCh,
        output_buffer_address,
        output_buffer_width,
        output_buffer_pitch,
        output_buffer_height,
        file_path);
#else
    SaveOutputImageAsPpm(
        hSidCh,
        output_buffer_address,
        output_buffer_width,
        output_buffer_pitch,
        output_buffer_height,
        file_path);
#endif

    if (num_input_buffer)
    {
        strcpy(file_path, path);
        sprintf(cdb_name, "%s.%d.jpg", cdb_name, image_index);
        strcat(file_path, cdb_name);

        SaveCdb(
            hSidCh,
            input_buffer_offset,
            input_buffer_size,
            num_input_buffer,
            file_path);
    }

    return;
}
#endif /* BSID_P_DEBUG_SAVE_BUFFER */

/******************************************************************************
* Function name: BSID_P_Debug_Gettimeus
*
* Comments:
*
******************************************************************************/
#ifdef BSID_P_DEBUG_TRACE_PLAYBACK
uint32_t BSID_P_Debug_Gettimeus(void)
{
     struct timeval tv;

     gettimeofday (&tv, NULL);

     return (tv.tv_sec * 1000000 + tv.tv_usec);
}
#endif

#ifdef BDBG_DEBUG_BUILD
/* Perform Compile-time assertions for verifying firmware API
   Ensure definitions that are made in both bsid.h and in bsid_fw_api.h
   are consistent */
void BSID_P_Debug_CheckFirmwareAPI(void)
{
   BDBG_CASSERT(BSID_ARC_CODE_SIZE==BSID_FW_ARC_END_OF_CODE);
   /* verify constants defined in bsid.h */
   BDBG_CASSERT(BSID_MPEG2IFRAME_INTRA_QUANT_ENTRIES==SID_MPEG2IFRAME_INTRA_QUANT_ENTRIES);
   BDBG_CASSERT(BSID_CLUT_MAX_ENTRIES==SID_CLUT_MAX_ENTRIES);
   /* NOTE: These values must be a multiple of 4 bytes to ensure the ImageHeader is
   a multiple of 32-bit words */
   BDBG_CASSERT(BSID_MPEG2IFRAME_INTRA_QUANT_ENTRIES%4==0);
   BDBG_CASSERT(BSID_CLUT_MAX_ENTRIES%4==0);

   /* Verify Error codes in bsid_err.h
    (can't see a way to verify that codes weren't added on either side!) */
   BDBG_CASSERT(ERR_CODE_SUCCESS==BERR_SUCCESS);
#define CHECK_ERR_CODE(x)   BDBG_CASSERT(BERR_MAKE_CODE(BERR_SID_ID, ERR_CODE_##x) == BSID_ERR_##x)
   CHECK_ERR_CODE(GIF_BAD_FORMAT);
   CHECK_ERR_CODE(GIF_NO_COLOR_MAP);
   CHECK_ERR_CODE(GIF_NO_IMG_START);
   CHECK_ERR_CODE(GIF_BAD_ID);
   CHECK_ERR_CODE(GIF_ILLEGAL_SIZE);
   CHECK_ERR_CODE(GIF_BAD_EXTENSION);
   CHECK_ERR_CODE(GIF_EARLY_FILE_END);

   CHECK_ERR_CODE(PNG_BAD_HEADER);
   CHECK_ERR_CODE(PNG_BAD_COMP_METHOD);
   CHECK_ERR_CODE(PNG_BAD_HUFFMAN);
   CHECK_ERR_CODE(PNG_BAD_BLK_TYPE_0);
   CHECK_ERR_CODE(PNG_BAD_INTERLACE);
   CHECK_ERR_CODE(PNG_ILLEGAL_SIZE);
   CHECK_ERR_CODE(PNG_BAD_BPP);
   CHECK_ERR_CODE(PNG_BAD_CUST_HUFFMAN);
   CHECK_ERR_CODE(PNG_TOO_WIDE);
   CHECK_ERR_CODE(PNG_BAD_CRC);
   CHECK_ERR_CODE(PNG_BAD_TRANSPARENCY);
   CHECK_ERR_CODE(PNG_BAD_PALETTE);
   CHECK_ERR_CODE(PNG_BAD_GAMA_CHUNK_SIZE);

   CHECK_ERR_CODE(JPEG_BAD_HEADER);
   CHECK_ERR_CODE(JPEG_BAD_MARKER);
   CHECK_ERR_CODE(JPEG_NO_SOF_HUFF);
   CHECK_ERR_CODE(JPEG_UNKNOWN_FMT);
   CHECK_ERR_CODE(JPEG_BAD_NUM_COMPS);
   CHECK_ERR_CODE(JPEG_TOO_MANY_DEQUANT);
   CHECK_ERR_CODE(JPEG_UNKNOWN_HUFF_TC);
   CHECK_ERR_CODE(JPEG_UNKNOWN_HUFF_TH);
   CHECK_ERR_CODE(JPEG_BAD_HUFF_TABLE);
   CHECK_ERR_CODE(JPEG_BAD_QUANT);
   CHECK_ERR_CODE(JPEG_UNSUPP_TYPE);
   CHECK_ERR_CODE(JPEG_BAD_FRM_HEAD);
   CHECK_ERR_CODE(JPEG_BAD_RST_MARKER);
   CHECK_ERR_CODE(JPEG_BAD_SIZE);
   CHECK_ERR_CODE(JPEG_BAD_MARKER_SEGMENT);
   CHECK_ERR_CODE(JPEG_PROG_BAD_DC);
   CHECK_ERR_CODE(JPEG_PROG_BAD_AL_AH);
   CHECK_ERR_CODE(JPEG_PROG_BAD_HUFF_LOOKUP);
   CHECK_ERR_CODE(JPEG_B_STREAM_OVER_READ);

   CHECK_ERR_CODE(RLE_BAD_FILE);
   CHECK_ERR_CODE(RLE_NO_COMMAND_7);
   CHECK_ERR_CODE(RLE_UNKNOWN_COMMAND);
   CHECK_ERR_CODE(RLE_BAD_SUBFMT);
   CHECK_ERR_CODE(RLE_BAD_HEADER);
   CHECK_ERR_CODE(RLEB_BAD_PALETTE);
   CHECK_ERR_CODE(RLEB_BAD_SEG_TYPE);
   CHECK_ERR_CODE(RLE_DCSQT_TOO_BIG);
   CHECK_ERR_CODE(RLE_BAD_SIZE);
   CHECK_ERR_CODE(RLE_BAD_DCSQT);
   CHECK_ERR_CODE(RLE_ILLEGAL_SIZE);

   CHECK_ERR_CODE(MIFRM_BAD_HEADER);
   CHECK_ERR_CODE(MIFRM_BAD_PICT_HEAD);
   CHECK_ERR_CODE(MIFRM_BAD_SLICE);
   CHECK_ERR_CODE(MIFRM_BAD_MBLOCK_1);
   CHECK_ERR_CODE(MIFRM_BAD_MBLOCK_2);
   CHECK_ERR_CODE(MIFRM_BAD_MBLOCK_3);
   CHECK_ERR_CODE(MIFRM_BAD_MBLOCK_4);
   CHECK_ERR_CODE(MIFRM_PICT_TOO_BIG);
   CHECK_ERR_CODE(MIFRM_BAD_MBTYPE);
   CHECK_ERR_CODE(MIFRM_FE_ERROR_FOUND);
   CHECK_ERR_CODE(MIFRM_NO_COLOR_TAB);

   CHECK_ERR_CODE(BE_ERROR_FOUND);

   CHECK_ERR_CODE(SETUP_ERR);
   CHECK_ERR_CODE(UNSUPPORTED_FEATURE);
   CHECK_ERR_CODE(ILLEGAL_STRIDE);
   CHECK_ERR_CODE(NO_STATE_BUFFER);
   CHECK_ERR_CODE(BAD_PARAMS);
   CHECK_ERR_CODE(IMG_TOO_BIG);
   CHECK_ERR_CODE(INSUFFICIENT_INPUT_DATA);

   CHECK_ERR_CODE(BAD_PIX_CNT);

   CHECK_ERR_CODE(UNKNOWN_FMT);
   CHECK_ERR_CODE(CMD_TOO_BIG);
   CHECK_ERR_CODE(WATCHDOG);

#undef CHECK_ERR_CODE

   /**
    ** Important: The following checks of Image Header structure are necessary
       because the PI is copying from memory written to by the FW into memory
       used by the application based on two different interpretations of this
       structure (BSID_ImageHeader defined in bsid.h and SID_ImageHeader
       defined by the FW - equivalent of ST_BSID_ImageHeader in sidapi.h)
    ***
    **/

   /* verify consistency between enums defined in bsid.h for ImageHeader and those in bsid_fw_api.h
      Currently, the only one that is not "translated" by the PI is BSID_JpegSubType (which is
      equivalent to JpegImgType_t defined in jpeg.h) */
   BDBG_CASSERT((unsigned)BSID_JpegSubType_eGrayScale==(unsigned)JPEG_TYPE_GREY);
   BDBG_CASSERT((unsigned)BSID_JpegSubType_e444==(unsigned)JPEG_TYPE_444);
   BDBG_CASSERT((unsigned)BSID_JpegSubType_e422==(unsigned)JPEG_TYPE_422);
   BDBG_CASSERT((unsigned)BSID_JpegSubType_e420==(unsigned)JPEG_TYPE_420);
   BDBG_CASSERT((unsigned)BSID_JpegSubType_e422r==(unsigned)JPEG_TYPE_422r);
   BDBG_CASSERT((unsigned)BSID_JpegSubType_e411==(unsigned)JPEG_TYPE_411);

   /* verify sizes of image header struct from PI POV and FW POV are same */
   BDBG_CASSERT(sizeof(BSID_ImageHeader) == sizeof(SID_ImageHeader));

#define SIZEOF_MEMBER(t,x) sizeof(((t *)0)->x)
   /* verify the size of the members of ImageHeader are consistent between bsid.h and bsid_fw_api.h
      This is necessary since PI uses enums for some fields, yet FW uses uint for the same fields */
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,e_PixelFormat) == SIZEOF_MEMBER(SID_ImageHeader,ePixelFormat));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_GrayScale) == SIZEOF_MEMBER(SID_ImageHeader,ui32_GrayScale));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_BitPerPixel) == SIZEOF_MEMBER(SID_ImageHeader,ui32_BitPerPixel));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_ImgWidth) == SIZEOF_MEMBER(SID_ImageHeader,ui32_ImgWidth));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_ImgHeight) == SIZEOF_MEMBER(SID_ImageHeader,ui32_ImgHeight));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_SurPitch) == SIZEOF_MEMBER(SID_ImageHeader,ui32_SurPitch));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_SurWidth) == SIZEOF_MEMBER(SID_ImageHeader,ui32_SurWidth));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_SurHeight) == SIZEOF_MEMBER(SID_ImageHeader,ui32_SurHeight));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_MultiScan) == SIZEOF_MEMBER(SID_ImageHeader,ui32_MultiScan));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_NumClutEntries) == SIZEOF_MEMBER(SID_ImageHeader,ui32_NumClutEntries));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,aui32_Clut) == SIZEOF_MEMBER(SID_ImageHeader,ae_Clut));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,e_ClutPixelFormat) == SIZEOF_MEMBER(SID_ImageHeader,eClutPixelFormat));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,e_JpegSubtype) == SIZEOF_MEMBER(SID_ImageHeader,eJpegSubType));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_IsIntraQuantTablePresent) == SIZEOF_MEMBER(SID_ImageHeader,ui32_isIntraQuantTablePresent));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui8_IntraQuantTable) == SIZEOF_MEMBER(SID_ImageHeader,ui8_intra_quant_table));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_PicStruct) == SIZEOF_MEMBER(SID_ImageHeader,ui32_pic_struct));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_Gamma) == SIZEOF_MEMBER(SID_ImageHeader,ui32_gamma));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_HasTransparentData) == SIZEOF_MEMBER(SID_ImageHeader,ui32_HasTransparentData));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_TransparentColorIndex) == SIZEOF_MEMBER(SID_ImageHeader,ui32_TransparentColorIndex));
   BDBG_CASSERT(SIZEOF_MEMBER(BSID_ImageHeader,ui32_TransparentColorRGB) == SIZEOF_MEMBER(SID_ImageHeader,ui32_TransparentColorRGB));
#undef SIZEOF_MEMBER

   /* verify the offsets of the members of ImageHeader are consistent between bsid.h and bsid_fw_api.h */
   BDBG_CASSERT(offsetof(BSID_ImageHeader,e_PixelFormat) == offsetof(SID_ImageHeader,ePixelFormat));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_GrayScale) == offsetof(SID_ImageHeader,ui32_GrayScale));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_BitPerPixel) == offsetof(SID_ImageHeader,ui32_BitPerPixel));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_ImgWidth) == offsetof(SID_ImageHeader,ui32_ImgWidth));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_ImgHeight) == offsetof(SID_ImageHeader,ui32_ImgHeight));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_SurPitch) == offsetof(SID_ImageHeader,ui32_SurPitch));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_SurWidth) == offsetof(SID_ImageHeader,ui32_SurWidth));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_SurHeight) == offsetof(SID_ImageHeader,ui32_SurHeight));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_MultiScan) == offsetof(SID_ImageHeader,ui32_MultiScan));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_NumClutEntries) == offsetof(SID_ImageHeader,ui32_NumClutEntries));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,aui32_Clut) == offsetof(SID_ImageHeader,ae_Clut));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,e_ClutPixelFormat) == offsetof(SID_ImageHeader,eClutPixelFormat));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,e_JpegSubtype) == offsetof(SID_ImageHeader,eJpegSubType));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_IsIntraQuantTablePresent) == offsetof(SID_ImageHeader,ui32_isIntraQuantTablePresent));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui8_IntraQuantTable) == offsetof(SID_ImageHeader,ui8_intra_quant_table));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_PicStruct) == offsetof(SID_ImageHeader,ui32_pic_struct));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_Gamma) == offsetof(SID_ImageHeader,ui32_gamma));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_HasTransparentData) == offsetof(SID_ImageHeader,ui32_HasTransparentData));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_TransparentColorIndex) == offsetof(SID_ImageHeader,ui32_TransparentColorIndex));
   BDBG_CASSERT(offsetof(BSID_ImageHeader,ui32_TransparentColorRGB) == offsetof(SID_ImageHeader,ui32_TransparentColorRGB));

   /* verify that the size of the data queue entry is large enough to hold the SID Image Header */
   BDBG_CASSERT(sizeof(BSID_DataQueueEntry) >= sizeof(BSID_ImageHeader));
}

#endif /* if debug build */

/* end of file */
