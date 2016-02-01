/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BDMA_SHARF_H__
#define BDMA_SHARF_H__

#include "bdma.h"          /* */

#if 0

#ifdef __cplusplus
extern "C" {
#endif


/**************************** Module Overview: *****************************
SHARF memory dma engine is also represented by regular memory dma sub-module
handle BDMA_Mem_Handle. The API functions and usage are exactly the same as
regular memory dma, except that

	o  Its BDMA_Mem_Handle sub-module handle should be created by
	   BDMA_Mem_Sharf_Create, rather than BDMA_Mem_Create; should be destoyed
	   by BDMA_Mem_Sharf_Destroy, rahtern than BDMA_Mem_Destroy.
	o  BDMA_Mem_Sharf_Tran_SetCrypto should be used to configure the sharf SCRAM
	   operations, and BDMA_Mem_SetCrypto should not be used for sharf dma.
	o  BDMA_Mem_Sharf_Tran_SetSgStartEnd should be used to set the start
	   point and/or end point of scatter-gather SCRAM.
	o  BDMA_Mem_Sharf_Tran_SetDmaBlockInfo(_isr) are used to replace the 
	   regular BDMA_Mem_Tran_SetDmaBlockInfo(_isr).

bdma_sharf.h could be considered as part of bdma.h for sharf particularly.
****************************************************************************/

/***************************************************************************
Summary:
	This enumeration represents the sharf dma engines.

Description:
	BDMA_Sharf is an enumeration which represents the sharf dma engines.

See Also:
	BDMA_Mem_Sharf_Create
***************************************************************************/
typedef enum BDMA_Sharf
{
	BDMA_Sharf_e0,  /* sharf memory dma engine 0 */
	BDMA_Sharf_e1,  /* sharf memory dma engine 1 */
	BDMA_Sharf_eInvalid
}
BDMA_Sharf;

/***************************************************************************
Summary:
	Creates a sub-module handle to sharf memory DMA engine.

Description:
	Once created, the handle to the unique memory DMA engine is required by
	sharf memory DMA sub-module functions in order to configure the DMA engine
	and perform DMA transfer. 

	This function also initializes the sub-module to default configure.

	Refer to BDMA_Mem_Handle and BDMA_Mem_Create2 description for detailed 
	usage info.

Input:
	hDma - Handle to the BDMA module.
	eEngine - The enum to identify the HW sharf memory dma engine.

Output:
	phMemDma - The created memory DMA sub-module handle. If failure 
	occurred phMemDma will holds NULL.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully created the handle.

See Also:
	BDMA_Mem_sharf_Destroy
**************************************************************************/
BERR_Code BDMA_Mem_Sharf_Create(
	BDMA_Handle              hDma,
	BDMA_Sharf               eEngine,
	BDMA_Mem_Settings *      pSettings,
	BDMA_Mem_Handle *        phMemDma );


/***************************************************************************
Summary:
	Destroys the sub-module handle of the memory DMA engine. 

Description:
	Once this function is called the sub-module handle can no longer be 
	used.

	Memory DMA engine sub-module handle should be destroyed before closing 
	the main module handle BDMA_Handle by BDMA_Close.

Input:
	hMemDma - The memory DMA engine sub-module handle to destroy.

Output:

Returns:
	BERR_INVALID_PARAMETER - hMemDma is not a valid memory DMA engine
	sub-module handle.
	BERR_SUCCESS - Successfully destroyed

See Also:
	BDMA_Mem_Sharf_Create
**************************************************************************/
BERR_Code BDMA_Mem_Sharf_Destroy(
	BDMA_Mem_Handle       hMemDma );

/***************************************************************************
Summary:
	This enumeration represents the possible SCRAM operations performed by
	sharf dma engines.

Description:
	BDMA_SharfMode is an enumeration which represents the possible SCRAM
	operations performed by sharf dma engines.

See Also:
	BDMA_Mem_Sharf_Tran_SetCrypto
***************************************************************************/
typedef enum BDMA_SharfMode
{
	BDMA_SharfMode_ePassThrough,
	BDMA_SharfMode_eSha1,
	BDMA_SharfMode_eAes128CbcDecrypt,
	BDMA_SharfMode_eAes128CbcEncrypt,
	BDMA_SharfMode_eCmac,
	BDMA_SharfMode_eAes128EcbDecrypt,
	BDMA_SharfMode_eAes128EcbEncrypt,
	BDMA_SharfMode_eInvalid
}
BDMA_SharfMode;


/***************************************************************************
Summary:
	Set sharf scram operation.

Description:
	During a DMA data transfer, sharf memory DMA engines could be configured 
	to perform different kind of SCRAM operations.

	In the case that eSharfMode is BDMA_SharfMode_ePassThrough, the data will
	pass through sharf without SCRAM operation.

	If eSharfMode has other valid value, the specified SCRAM is performed by
	sharf HW.

	Further, when scatter-gather mode is enabled by bSgEnable, the dma engine 
	will treat the data stream section of a list of dma blocks as a whole for 
	SCRAM purpose. The stream section starts from the beginning of the block 
	marked as scatter-gather start point, and ends at the end of the block 
	marked as scatter-gather end point. bSgEnable and eSharfMode can not 
	change between the scatter-gather start point and end point.

	When bUseBspKey is false, sharf uses the key wrapped in the head of the 
	scatter-gather data stream section, for the following data encrypt or
	decrypt. if bUseBspKey is true, sharf uses the key supplied by Aegis 
	directly for data encrypt and decrypt, regardless of the state of key-
	present set with BDMA_Mem_Sharf_Tran_SetSgStartEnd.

	The sub-module maintains a current state of settings by this API function.
	Each setting modifies the current state and is applied to the future 
	calls to BDMA_Mem_Tran_SetDmaBlockInfo(_isr), unless it is reset again. 

Input:
	hTran - The transfer handle to set crypto.
	BDMA_SharfMode - SCRAM operation mode.
	bSgEnable - Whether enable scatter-gather SCRAM operation.
	bUseBspKey - If it is true, sharf uses the key supplied by Aegis directly 
	(regardless of what BDMA_Mem_Sharf_Tran_SetSgStartEnd sets for key-present).
	bCmp8LstByts - If it is true, sharf will use only the 8 least significant 
	bytes for digest comparing.
Output:

Returns:
	BERR_SUCCESS - crypto setting is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Sharf_Tran_SetSgStartEnd, BDMA_Mem_Sharf_Tran_SetSgStartEnd_isr,
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr 
****************************************************************************/
BERR_Code BDMA_Mem_Sharf_Tran_SetCrypto(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_SharfMode           eSharfMode,
	bool                     bSgEnable,
	bool                     bUseBspKey,
	bool                     bCmp8LstByts);

/***************************************************************************
Summary:
	Set the start point and/or end point of scatter-gather.

Description:
	When SCRAM operation and scatter-gather mode are enabled, the sharf dma 
	engine will treat the data stream section of a list of dma blocks as a 
	whole for SCRAM purpose. The stream section starts from the beginning of 
	the block marked as scatter-gather start point, and ends at the end of 
	the block marked as scatter-gather end point.

	This API function is used to mark the block with ID ulBlockId as scatter-
	gather start and/or end point. It can also mark whether crypto key and/or
	digest/MAC reference value is prepended into the data transferred by dma.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	bStartSgScram - Whether this block is a scatter-gather start point.
	bEndSgScram - Whether this block is a scatter-gather end point.
	bKeyPresent - Whether crypto key is prepended into the data stream.
	bDigestPresent - Whether digest reference value is prepended into the data
	stream.
Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Sharf_Tran_SetCrypto, BDMA_Mem_Tran_SetDmaBlockInfo,
	BDMA_Mem_Sharf_Tran_SetSgStartEnd_isr
****************************************************************************/
BERR_Code BDMA_Mem_Sharf_Tran_SetSgStartEnd(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram,
	bool                     bKeyPresent,
	bool                     bDigestPresent );

/***************************************************************************
Summary:
	This enumeration represents the contexts sharf support.

Description:
	BDMA_Context is an enumeration which represents the contexts sharf support.

See Also:
	BDMA_Mem_Sharf_Tran_SetDmaBlockInfo, BDMA_Mem_Sharf_Tran_SetDmaBlockInfo_isr
***************************************************************************/
typedef enum BDMA_Context
{
	BDMA_Context_eSha0,       /* context 0 for sha op */
	BDMA_Context_eSha1,       /* context 1 for sha op */
	BDMA_Context_eSha2,       /* context 2 for sha op */
	BDMA_Context_eAesOrCmac,  /* context for AES or CMAC */
	BDMA_Context_eInvalid
}
BDMA_Context;

/***************************************************************************
Summary:
	Set the block info for one block of a DMA transfer.

Description:
	In order to perform a DMA transfer, user first creates a transfer handle,
	then uses it to set block info for each block, to start the DMA transfer, 
	and finally to get the transfer status. This API function is used to set 
	block info for one block. 

	ulBlockId is the index of the block, in the block list of the transfer
	represented by hTran. It starts from 0. eContext is the context number
	this block belongs to.

	The source and destination address are in memory bus address space. They
	are 28 bits unsigned integer number in 7038 chip. It is more likely that 
	they are converted from virtual memory mapped address using 
	BMEM_ConvertAddressToOffset and the virtual address is returned by
	BMEM_AllocAligned or BMEM_Alloc.

	Both addresses and block size are byte aligned. Max block size is
	0xfff,ffff (16 MByte). Source and destination region overlap with 
	(ulSrcBusAddr < ulDstBusAddr < ulSrcBusAddr+ulBlockSize) is not allowed.

	Encryption and decryption algorithms typically work on memory size of 
	multiple 8 bytes or 16 bytes, and certain residue mode clear termination 
	or OFB, might be used for the remaining bytes in the block.

	Please notice that when a non-word-aligned source address is used, the
	read endian determine which bytes to read in a 32 bits word.

	The current state of read endian, swap mode and encrypt / decrypt setting 
	is used during the transfer of this block.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	eContext - The context number this block belongs to.
	ulDstBusAddr - Destination address in memory bus address space.
	ulSrcBusAddr - Source address in memory bus address space.
	ulBlockSize - This block size in bytes.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BDMA_ERR_OVERLAP - Source and destination region overlap.
	BDMA_ERR_SIZE_OUT_RANGE - ulBlockSize is 0 or bigger than 16 MByte.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2
	BDMA_Mem_Tran_Start, BDMA_Mem_Tran_StartAndCallBack
	BDMA_Mem_SetByteSwapMode, BDMA_Mem_SetCrypt
	BDMA_Mem_Tran_SetDmaBlockInfo_isr
****************************************************************************/
BERR_Code BDMA_Mem_Sharf_Tran_SetDmaBlockInfo(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	BDMA_Context             eContext,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize );

/***************************************************************************
Summary:
	Set sharf scram operation.

Description:
	This is the "_isr" version of BDMA_Mem_Sharf_Tran_SetCrypto. It is used 
	in interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Sharf_Tran_SetCrypto.
	
Input:
	hTran - The transfer handle to set crypto.
	BDMA_SharfMode - SCRAM operation mode.
	bSgEnable - Whether enable scatter-gather SCRAM operation.
	bUseBspKey - If it is true, sharf uses the key supplied by Aegis directly 
	(regardless of what BDMA_Mem_Sharf_Tran_SetSgStartEnd sets for key-present).
	bCmp8LstByts - If it is true, sharf will use only the 8 least significant 
	bytes for digest comparing.
Output:

Returns:
	BERR_SUCCESS - crypto setting is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Sharf_Tran_SetCrypto,
	BDMA_Mem_Sharf_Tran_SetSgStartEnd, BDMA_Mem_Sharf_Tran_SetSgStartEnd_isr,
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr 
****************************************************************************/
BERR_Code BDMA_Mem_Sharf_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_SharfMode           eSharfMode,
	bool                     bSgEnable,
	bool                     bUseBspKey,
	bool                     bCmp8LstByts);

/***************************************************************************
Summary:
	Set the start point and/or end point of scatter-gather.

Description:
	This is the "_isr" version of BDMA_Mem_Sharf_Tran_SetSgStartEnd. It is 
	used in interrupt handler or critical session. For more description refer 
	to BDMA_Mem_Sharf_Tran_SetSgStartEnd.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	bStartSgScram - Whether this block is a scatter-gather start point.
	bEndSgScram - Whether this block is a scatter-gather end point.
	bKeyPresent - Whether crypto key is prepended into the data stream.
	bDigestPresent - Whether digest reference value is prepended into the data
	stream.
Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Sharf_Tran_SetCrypto, BDMA_Mem_Tran_SetDmaBlockInfo_isr,
	BDMA_Mem_Sharf_Tran_SetSgStartEnd
****************************************************************************/
BERR_Code BDMA_Mem_Sharf_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram,
	bool                     bKeyPresent,
	bool                     bDigestPresent );

/***************************************************************************
Summary:
	Set the block info for one block of a DMA transfer.

Description:
	This is the "_isr" version of BDMA_Mem_Sharf_Tran_SetDmaBlockInfo. It is 
	used in interrupt handler or critical session. For more description refer 
	to BDMA_Mem_Sharf_Tran_SetDmaBlockInfo.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	eContext - The context number this block belongs to.
	ulDstBusAddr - Destination address in memory bus address space.
	ulSrcBusAddr - Source address in memory bus address space.
	ulBlockSize - This block size in bytes.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BDMA_ERR_OVERLAP - Source and destination region overlap.
	BDMA_ERR_SIZE_OUT_RANGE - ulBlockSize is 0 or bigger than 16 MByte.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2
	BDMA_Mem_Tran_Start, BDMA_Mem_Tran_StartAndCallBack
	BDMA_Mem_SetByteSwapMode, BDMA_Mem_SetCrypt
	BDMA_Mem_Tran_SetDmaBlockInfo_isr
****************************************************************************/
BERR_Code BDMA_Mem_Sharf_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	BDMA_Context             eContext,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize );


#ifdef __cplusplus
}
#endif
#endif
#endif /* #ifndef BDMA_SHARF_H__ */

/* end of file */
