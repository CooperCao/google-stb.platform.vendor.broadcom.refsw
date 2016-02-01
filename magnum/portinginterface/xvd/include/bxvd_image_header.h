/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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

#ifndef BXVD_IMAGE_HEADER_H__
#define BXVD_IMAGE_HEADER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * "Self-describing" firmware header 
 *
 * This structure contains the information necessary for the Aegis processor
 * to initiate the loading and execution of the AVD firmware. The members of
 * the structure correspond to the following map of the firmware after being
 * restructured and relocated by the SVP relocation utility:
 * 
 *    +------------+ -> uiRelocationBase and outerCode.uiAddr
 *    | Outer Loop |
 *    |    Code    |
 *    +------------+ -> outerCode.uiAddr + outerCode.uiSize (end of OUTER code)
 *   ~~~~~~~~~~~~~~~~
 *    +------------+ -> innerCode.uiAddr
 *    | Inner Loop |
 *    |    Code    |
 *    +------------+ -> innerCode.uiAddr + innerCode.uiSize (end of INNER code)
 *   ~~~~~~~~~~~~~~~~
 *    +------------+ -> signature.uiAddr
 *    | Signature  | 
 *    +------------+ -> signature.uiAddr + signature.uiSize
 *   ~~~~~~~~~~~~~~~~
 *    +------------+ -> outerData.uiAddr
 *    | Outer Loop |
 *    |    Data    |
 *    +------------+
 *    | Outer Loop |
 *    |     BSS    |
 *    +------------+ -> outerData.uiAddr + outerData.uiSize
 *   ~~~~~~~~~~~~~~~~
 *    +------------+ -> innerData.uiAddr
 *    | Inner Loop |
 *    |    Data    |
 *    +------------+
 *    | Inner Loop |
 *    |     BSS    |
 *    +------------+ -> innerData.uiAddr + innerData.uiSize
 *
 * ol_code_size, ol_data_size, il_code_size and il_data_size are values that
 * are known to the MetaWare linker but not included in the FW image. These
 * values could be provided by the firmware team or calculated by the SVP
 * relocation utility, but this would yield a value considerably smaller than
 * that given to the linker.
 * 
 * signature_size is an optional argument passed to the SVP relocation utility.
 * If the size is not specified to the utility, it defaults to 0 and the
 * signature_addr member is set to NULL. This means that the signature is
 * external to the firmware and that its location and size must be managed by
 * the application that created it. The signature_addr, if it is non-NULL
 * will always be equal to il_end_of_code.
 *
 * cmd_buffer_addr is the first 128 bytes of the outer loop data section. This
 * is equal to il_end_of_code or, if signature size is non-zero, il_end_of_code
 * + signature_size.
 *
 */

#if !BXVD_P_AVD_ARC600
typedef struct _BXVD_AvdImgSegment_
{
	uint32_t uiOffset;  /* Offset of the segment in the image (***
			     * TODO: absolute or relative to end of
			     * header?)*** */
	uint32_t uiAddress; /* Absolute physical address of the
			     * segment in memory */
	uint32_t uiSize;    /* Size of this segment */
} BXVD_AvdImgSegment;

typedef struct _BXVD_AvdImgHdr_
{
	uint32_t uiHeaderVersion;          /* header version */
	uint32_t uiHeaderSize;             /* size of entire header */
	uint32_t uiImageSize;              /* total size of image, not including header */
	uint32_t uiRelocationBase;         /* relocation base address */
	BXVD_AvdImgSegment outerCode;
	BXVD_AvdImgSegment innerCode;
	BXVD_AvdImgSegment commandBuffer;
	BXVD_AvdImgSegment outerData;
	BXVD_AvdImgSegment innerData;
	BXVD_AvdImgSegment signature;
} BXVD_AvdImgHdr;

#define BXVD_AVD_IMG_HDR_VERSION 1
#define BXVD_AVD_IMG_HDR_SIZE sizeof(BXVD_AvdImgHdr)
#endif

#ifdef __cplusplus
}
#endif

#endif /* BXVD_IMAGE_HEADER_H__ */
/* End of file. */
