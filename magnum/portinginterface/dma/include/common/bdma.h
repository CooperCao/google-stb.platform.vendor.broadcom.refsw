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
#ifndef BDMA_H__
#define BDMA_H__

#include "bchp.h"          /* Chip information */
#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bmem.h"          /* Chip memory access. */
#include "bint.h"          /* L2 Interrupt Manager */

#ifdef __cplusplus
extern "C" {
#endif


/**************************** Module Overview: *****************************
BDMA module provides porting interface API for higher level software to
control hardware DMA engines. The API exposes hardware functionalities and
hides the hardware and software details. It is targeted for Hydra core based
chip, and is also intended to fit future chips.

There might be multiple memory DMA engines and/or PCI DMA engines in a chip.
A memory DMA engine is used to move data from one location in DRAM to 
another location in DRAM. And a PCI DMA engine is used to move data between
DRAM and any PCI device connected to PCI bus.

Both memory DMA engine and PCI DMA engine are represented as sub-modules of
the BDMA software module.

In order to control a specific hardware DMA engine, higher level software
should first open a BDMA module handle by calling BDMA_Open, and use this
module handle to create a sub-module handle to the DMA engine by calling
either BDMA_Mem_Create2 or BDMA_Pci_Create2. The sub-module handle could 
then be used to setup the DMA engine and perform DMA data transfer using the
sub-module functions. All sub-module functions are prefixed with BDMA_Mem_
or BDMA_Pci_.

User could DMA transfer a list of contiguous blocks in one transfer action. 
In order to do this, user needs to create a transfer handle to represent the
transfer action, then to use it to set block info for each block, to start
the transfer, and finally to get the transfer status. A transfer handle is a
sub-module of a DMA engine.

Two API functions are provided to start a DMA transfer, one for "start" 
only (then the result should be checked actively by user), and another for 
"start and call back" (then the result will be informed by a callback when
the transfer is done).

The "started" transfer might be queued in BDMA first and then be issued to 
hardware in turn.

During a DMA data transfer, both memory and PCI DMA engines could be
programmed to perform translation between little endian and big endian with
either byte aligned swap or short aligned swap. Memory DMA engine could
also perform encryption or decryption. The crypto is set with
BDMA_Mem_Tran_SetCrypto. 

Both memory and PCI DMA engines maintain a current state of engine configure
of swap mode, or max burst size. Each DMA transfer uses the current
configure state.

The transfer handle could be re-used after a DMA transfer is started and
finished, for convenience and better performance. If there is no change to a 
block info, it does not need to be re-set too.

BDMA module is interrupt safe, in the sense that DMA transfer could be 
performed from both interrupt handler or a non-interrupt context such as
user mode process. An exception is that in interrupt handler the completion
of a DMA transfer can not be waited, and therefore call back has to been
used if the "done info" needs to be processed.

In order to support interrupt safety, two sets of transfer API functions are 
provided, one set for non-interrupt context such as user mode process, and 
another set used in interrupt handler or critical session. The second set of
API functions are named with an extra suffix "_isr". 

BDMA is also thread safe. BDMA transfer functions could also be called from
more than one threads.

However, user must ensure that interrupt handler (and its subroutines) and
code in critical session only calls BDMA API functions with suffix "_isr" 
and functions in non-interrupt context only calls BDMA API functions without
that suffix.

And also, if different engine configure are used in different thread or
interrupt handler, higher level software has to use extra mechanism (such
as mutex, but not critical session) to synchronize the API function call in 
order to ensure that the right configure is applied to each DMA transfer.

When a DMA engine sub-module handle or the DMA module handle is no longer
needed, it should be destroyed or closed.

A hardware DMA engine might also be shared by multiple users from both user
mode side and kernel side.

In this case, BDMA API functions should be called independently by each DMA
user to create BDMA_Handle, BDMA_Mem/Pci_Handle, and BDMA_Mem/Pci_Tran_Handle, 
to set block info, and to start a DMA transfer. The result could later be 
polled by the application, or be informed by call-back.

Each user (application) hence runs an independent instance of BDMA as if it 
owns the DMA hardware engine without knowing the sharing issue. Each BDMA 
instance in the system manages its own software context and DMA descriptor
buffer. Only when a transfer is started in hardware, the user provided call
back functions are used to lock the hardware from accessing by other BDMA
instance, and to release it.

Please refer to BDMA_Mem_Handle and BDMA_Pci_Handle and API functions for 
more usage information.

Notes:
------
Before using BDMA, higher level software must connect and enable level 1
interrupt. BDMA automatically creates call back with BINT for level 2
interrupt. Level 1 is left to higher level because it is platform dependent.
The L1 interrupt bit for both memory dma and PCI dma is bit HIF_CPU_INTR in 
register HIF_CPU_INTR1_INTR_W0_STATUS.

Design notes:

	o  The SDRAM address used in our API functions is in memory bus address,
	   rather than virtual address. Virtual address might look more convenient 
	   to user, but using bus address helps to ensure that the DMA transferred 
	   memory block physically exists and is contiguous, and it is very easy 
	   to get the bus address by BMEM_ConvertAddressToOffset if the memory 
	   block is allocated by BMEM_AllocAligned or BMEM_Alloc.
	o  Having user to pass in an array of block info might simplify the
	   process of transferring a list of block (i.e. transfer handle creating, 
	   block info setting, and finally starting), into a single function call. 
	   However, reading data from user provided buffer of indefinite length
	   often causes problem, because the buffer pointer and length can not be 
	   validated before crashing the sub-system. Our current design is more 
	   robust, and it saves the time of copying from user's buffer into 
	   internal DMA description.
	o  Transfer Id is also considered, but we decided to use transfer handle.
	   One difference to the user between transfer Id and handle is that,
	   transfer Id does not need to be explicitly destroyed, but transfer
	   handle does. Therefore transfer handle needs one more API function
	   call for each transfer. However, transfer Id can not be reused,
	   its records disappear automatically, and user can not control it.
	   Another question is that what is the right behavior if the "start"
	   returns an error, such as "invalid parameter" and "engine busy"?
	o  Another implementation option we considered is to have a "done" wait
	   loop in the BDMA module. We finally decided no to because user might 
	   needs to wait longer than what we could have coded and still needs to
	   implement a user's waiting loop.

****************************************************************************/


/***************************************************************************
Summary:
	The handle for the DMA module.

Description:
	This is an opaque handle that application created with BDMA_Open.
	BDMA_Handle represents the context of the BDMA module. It is needed
	before using any other API functions of DMA module. The higher level 
	software is responsive to ensure that there exists only one BDMA_Handle 
	per chip.

	When the DMA module handle is no longer needed, it should be closed
	by BDMA_Close.

See also:
	BDMA_Open, BDMA_Close
	BDMA_Mem_Handle, BDMA_Mem_Create2
	BDMA_Pci_Handle, BDMA_Pci_Create2
****************************************************************************/
typedef struct BDMA_P_Context *BDMA_Handle;

/***************************************************************************
Summary:
	This enumeration represents swap mode.

Description:
	BDMA_SwapMode is an enumeration which represents all swap modes supported
	during a DMA transfer. Swapping is mainly used to transfer little endian
	data to big endian, or the other way around.

See Also:
	BDMA_Mem_SetByteSwapMode, BDMA_Pci_SetSwapMode
***************************************************************************/
typedef enum BDMA_SwapMode
{
	BDMA_SwapMode_eNoSwap,     /* default, no swap */
	BDMA_SwapMode_e0123To2301, /* short aligned: swap shorts in uint32_t */
	BDMA_SwapMode_e0123To3210, /* byte aligned: swap bytes in uint32_t */
	BDMA_SwapMode_eInvalid
}
BDMA_SwapMode;

/***************************************************************************
Summary:
	This enumeration represents the endian format of the data.

Description:
	BDMA_Endian is an enumeration which represents the two possible endian
	formats, big-endian and little endian.

See Also:
	BDMA_Mem_SetByteSwapMode
***************************************************************************/
typedef enum BDMA_Endian
{
	BDMA_Endian_eLittle,     /* little endian format */
	BDMA_Endian_eBig,        /* big endian format */
	BDMA_Endian_eInvalid
}
BDMA_Endian;

/***************************************************************************
Summary:
	This enumeration represents the status of a transfer 

Description:
	BDMA_TranStatus is an enumeration which represents the status of a
	transfer.

	If an invalid key selection for encrypt / decrypt in a memory DMA transfer,
	BDMA_TranStatus_eFailed would be returned for the status query of the
	transfer.

See Also:
	BDMA_Mem_GetTranStatus, BDMA_Mem_GetTranStatus_isr
	BDMA_Pci_GetTranStatus, BDMA_Pci_GetTranStatus_isr
***************************************************************************/
typedef enum
{
	BDMA_TranStatus_eSucceeded,  /* Done in HW, successfully */ 
	BDMA_TranStatus_eFailed,     /* Done in HW, but failed */
	BDMA_TranStatus_eInProgress, /* Processed in HW right now */
	BDMA_TranStatus_eQueued,     /* Starting API called, not sent to HW yet. */
	BDMA_TranStatus_eCreated,    /* Tran created, Starting API not called yet */
	BDMA_TranStatus_eUnknown     /* Unknown, maybe a invalid Tran handle used */
}
BDMA_TranStatus;

/***************************************************************************
Summary:
	Function pointer type of user call back functions used to lock and 
	release a hardware DMA engine.

Description:
	The call back functions used in the case that multiple BDMA instances 
	exist in the system. They are coded by the user, and are passed to BDMA 
	when a dma engine sub-module handle is created.

	There are two call-back functions of this type, TryLock and ReleaseLock. 
	
	If multiple BDMA instances exist, before writing dma requests into dma
	(hardware) engine, each BDMA instance needs to check the hardware status 
	and to make sure that the hardware is not in use by another instance. 
	To ensure that the hardware status is not changed in the mean time by 
	another BDMA instance, TryLock is used to lock the hardware before this 
	BDMA instance reads the hardware registers. ReleaseLock is immediately 
	used after the dma requests are written to hardware, or after it is 
	decided that the hardware is still in use by another BDMA instance. If 
	the dma request can not be written into hardware this time, BDMA would 
	try again with result-polling from application, or with a dma-done _isr
	service.

	TryLock should return true if and only if it is successful. The return 
	value from ReleaseLock is not used.

	The lock time is short, and it does not depend on the DMA hardware 
	execution. However, a user mode application could still be preempted 
	after it gets the lock and before it releases it. In this case, a 
	kernel mode dma user could not get the lock and hence dma service until 
	the user mode application resumes execution and releases the lock. In
	the mean time, the hardware engine could indeed have been idle.

	TryLock could increase the user mode application to higher priority so
	that the application will unlikely be preempted. RleaseLock should then
	drop it to its original priority. How high priority the application 
	should be raised to by TryLock? It is up to the application to decide. 

	For a simple implementation, TryLock and ReleaseLock could use a 
	hardware semaphore (such as BCHP_SUN_TOP_CTRL_SEMAPHORE_15). A hardware 
	semaphore is a special register with some simple logic. The simple logic
	ensures that a non-zero value can be successfully written into it if and 
	only if it currently has value "0".

	In TryLock a unique non-zero user-id is written to the semaphore register,
	then the semaphore register value is read back. If the read back value 
	matches the written value (user-id), it means that is has got the 
	semaphore. RelaeseLock could simply write value "0" into the semaphore
	register to release it.

See Also:
	BDMA_Mem_Settings, BDMA_Pci_Settings, 
	BDMA_Mem_Create2, BDMA_Pci_Create2
****************************************************************************/
typedef bool (* BDMA_LockCallbackFunc)( void * pvParm1, int iParm2 );

/***************************************************************************
Summary:
	This function opens and initializes the DMA module BDMA.

Description:
	Before any thing could be done to a DMA engine in the chip, a DMA module
	handle, BDMA_Handle, must be opened. From this handle, the sub-module
	handle of memory DMA engine and/or PCI DMA engine could be created. 

	It is user's responsibility to ensure that there is only one BDMA module
	handle opened per chip.

	This BDMA module handle should eventually be closed by calling BDMA_Close,
	when it is no longer needed.

Input:
	hChip - The chip handle that application created earlier during chip
	initialization sequence. This handle is used for getting chip
	information, chip revision, and miscellaneous chip configurations.

	hRegister - The register handle that application created earlier during
	chip initialization sequence. This handle is used to access chip
	registers (DMA registers).

	hMemory - The local memory manager module handle. DMA descriptors
	are allocated in local memory with hMemory.

	hInterrupt - The level2 interrupt handle that application created
	earlier chip initialization sequence. This handle is use to install
	level 2 interrupt callback.

Output:
	phDma - The opened DMA module handle.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully opened BDMA.

See Also:
	BDMA_Close
	BDMA_Mem_Create2
	BDMA_Pci_Create2
**************************************************************************/
BERR_Code BDMA_Open(
	BDMA_Handle *            phDma,
	BCHP_Handle              hChip,
	BREG_Handle              hRegister,
	BMEM_Handle              hMemory,
	BINT_Handle              hInterrupt );

/***************************************************************************
Summary:
	Close the BDMA handle.

Description:
	Once this function is called the BDMA handle can no longer be used.

	All DMA sub-module handles should be destroyed before closing the BDMA
	module handle by BDMA_Close.

Input:
	hDma - The BDMA handle to be closed.

Output:

Returns:
	BERR_INVALID_PARAMETER - hDma is not a valid BDMA handle.
	BERR_SUBMODULE_EXIST - There is still DMA sub-module handle not destroyed.
	BERR_SUCCESS - Successfully destroyed.

See Also:
	BDMA_Open
	BDMA_Mem_Destroy
	BDMA_Pci_Destroy
**************************************************************************/
BERR_Code BDMA_Close(
	BDMA_Handle              hDma );


/***************************************************************************
 * Sub-Module: Memory DMA 
 **************************************************************************/

/***************************************************************************
Summary:
	Memory DMA engine sub-module handle, or Memory DMA handle for simplicity.

Description:
	This is an opaque handle that represents and controls one HW memory DMA 
	engine. It is needed when the memory DMA engine is configured or used.

	It is created with BDMA_Mem_Create2. 

	The typical process to configure and use a memory DMA engine is as
	follows:

	o Open the DMA module handle.
	o A sub-module handle to the memory DMA engine is created.
	o Configure memory DMA engine (sub-module), such as swapping mode, 
	  and crypto setting.
	o Call BDMA_Mem_Tran_Create2 to create a memory DMA transfer handle.
	o Set the block info (such as source address, destination address, and
	  block size) for each data block to be transferred by calling
	  BDMA_Mem_Tran_SetDmaBlockInfo.
	o Start the transfer by BDMA_Mem_Tran_StartDma, or 
	  BDMA_Mem_Tran_StartDmaAndCallBack.
	o Process the "DMA done" information. Depending on which starting API
	  function was called above, BDMA_Mem_Tran_StartDma or 
	  BDMA_Mem_Tran_StartDmaAndCallBack, the "DMA done" information could 
	  either be acquired by actively calling BDMA_Mem_Tran_GetStatus or
	  be informated by a call back from BDMA.

	There is one engine configure API function BDMA_Mem_SetByteSwapMode for 
	memory DMA sub-module, and one transfer configure API function
	BDMA_Mem_Tran_SetCrypto for memory DMA transfer sub-module. The sub-
	modules maintain a current state of configure. Each setting modifies the 
	current state and is applied to the future calls to 
	BDMA_Mem_Tran_SetDmaBlockInfo. This implies that in a transfer each block 
	could have different engine configure.

	We put current state of crypto setting into transfer, because it is 
	more likely to change from transfer to transfer, but swap mode is not.

	After the engine is configured, user could also perform a DMA transfer 
	in an interrupt handler or critical session, using the API function set 
	named with a "_isr" suffix.

	When a DMA engine sub-module handle is no longer needed, it should be
	destroyed with BDMA_Mem_Destroy.

	Please refer to the DMA module overview for more usage info.

See Also:
	BDMA, BDMA_Handle
	BDMA_Mem_Create2, BDMA_Mem_Destroy
	BDMA_Mem_SetByteSwapMode, BDMA_Mem_Tran_SetCrypto
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_StartDma, BDMA_Mem_Tran_StartDmaAndCallBack
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Reset, BDMA_Mem_Tran_Destroy
	BDMA_Mem_Tran_Create2_isr, BDMA_Mem_Tran_SetDmaBlockInfo_isr
	BDMA_Mem_Tran_StartDmaAndCallBack_isr
	BDMA_Mem_Tran_GetStatus_isr
	BDMA_Mem_Tran_Reset_isr, BDMA_Mem_Tran_Destroy_isr
***************************************************************************/
typedef struct BDMA_P_Mem_Context        *BDMA_Mem_Handle;

/***************************************************************************
Summary:
	This enumeration represents memory dma engines.

Description:
	BDMA_MemDmaEngine is an enumeration which represents all HW memory dma 
	engines there exist in the chip. It is used to indicate which HW engine
	to use when a memory dma handle is created by BDMA_Mem_Create2.

See Also:
	BDMA_Mem_Create2
***************************************************************************/
typedef enum BDMA_MemDmaEngine
{
	BDMA_MemDmaEngine_e0,     /* mem dma engine 0 */
	BDMA_MemDmaEngine_e1,     /* mem dma engine 1 */
	BDMA_MemDmaEngine_eUnknown
}
BDMA_MemDmaEngine;

/***************************************************************************
Summary:
	This enumeration represents crypto mode.

Description:
	When memory DMA engine is transfering a block of data, it could be 
	programmed to perform encryption and decryption. BDMA_CryptMode
	represents all crypto modes supported during a DMA transfer.

See Also:
	BDMA_Mem_Tran_SetCrypto
***************************************************************************/
typedef enum BDMA_CryptMode
{
	BDMA_CryptMode_eNoCrypt, /* default, crypto HW disabled */
	BDMA_CryptMode_eBlock,   /* always encrypt/decrypt 16 block */
	BDMA_CryptMode_eMpeg,    /* recognize mpeg transport packet */
	BDMA_CryptMode_eDirecTv, /* recognize DirecTv transport packet */
	BDMA_CryptMode_eInvalid
}
BDMA_CryptMode;

/***************************************************************************
Summary:
	This structure describes the settings for BDMA_Mem_Handle

Description:
	BDMA_Mem_Settings is a structure that is used to describe the public
	settings of a memory dma engine. The settings are in engine scope.

	ulTotalNumBlocks - The maximum number of blocks could be supported
	by this dma engine in total. Its default value is 256.

	ulMaxNumBlocksPerTran - The maxium number of blocks per transfer. Its
	default value is 128.

	bSupportCallBack - When multiple BDMA instance exist in the system, 
	only one instance can support call-back mode. bSupportCallBack indicates 
	whether this BDMA instance is the unique one that can support call-back 
	mode. Its default value is true.

	pTryLock - User provided call back function that is called to lock the 
	hardware memory dma engine from accessing by other BDMA instances. Its
	default value is NULL.

	pReleaseLock - User provided call back function that is called to release
	the lock so that the hardware memory dma engine could be accessed by
	other BDMA instances. Its default value is NULL.

	pvParm1 - Pointer to a user defined struct. It is passed to (*pTryLock)
	and (*pReleaseLock) as a parameter. Its default value is NULL.

	iParm2 - Second user parameter for the lock call-back functions. It is
	passed to (*pTryLock)and (*pReleaseLock) as a parameter. Its default value
	is 0.

	If there is only one BDMA instance in the system, bSupportCallBack,
	pTryLock, pReleaseLock, pvParm1, iParm2 should be left with default value.

See Also:
	BDMA_Mem_Create2, BDMA_Mem_GetDefaultSettings
***************************************************************************/
typedef struct BDMA_Mem_Settings
{
	uint32_t                           ulTotalNumBlocks;
	uint32_t                           ulMaxNumBlocksPerTran;

	/* settings for the case of multiple BDMA instances */
	BDMA_LockCallbackFunc              pTryLock_isr;
	BDMA_LockCallbackFunc              pReleaseLock_isr;
	void                             * pvParm1;
	int                                iParm2;
	bool                               bSupportCallBack;
}
BDMA_Mem_Settings;

/***************************************************************************
Summary:
	Creates a sub-module handle to memory DMA engine.

Description:
	This API function is used to create a BDMA_Mem_Handle to represent and
	control a specific HW memory dma engine. It is required by memory DMA <
	sub-module functions in order to configure the DMA engine and perform 
	DMA transfer. 

	If the sub-module handle has already been created for the same HW engine
	with the same BDMA, error is returned. This function also initializes 
	the sub-module to default configure.

	Before BDMA_Mem_Create2 is called, BDMA_Mem's inherent default setting
	structure should be queried by BDMA_Mem_GetDefaultSettings, modified and 
	then passed to BDMA_Mem_Create2. This avoid compatibilty problem if more 
	elements are added into BDMA_Mem_Settings in the future, and save 
	application tedious work of filling in the configuration structure.

	Refer to BDMA_Mem_Handle description for detailed usage info.

Input:
	hDma - Handle to the BDMA module.
	eEngine - The HW memory dma engine controlled by this BDMA_Mem_Handle
	pSettings - The basic settings. Defaut setting is used if pSettings is
	NULL.

Output:
	phMemDma - The created memory DMA sub-module handle. If failure 
	occurred phMemDma will holds NULL.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully created the handle.

See Also:
	BDMA_Mem_Destroy, BDMA_Mem_GetDefaultSettings
**************************************************************************/
BERR_Code BDMA_Mem_Create2(
	BDMA_Handle              hDma,
	BDMA_MemDmaEngine        eEngine,
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
	BDMA_Mem_Create2
**************************************************************************/
BERR_Code BDMA_Mem_Destroy(
	BDMA_Mem_Handle          hMemDma );

/***************************************************************************
Summary:
	This function gets BDMA_Mem's inherent default setting structure.

Description:
	BDMA_Mem's inherent default setting structure could be queried by this 
	API function prior to BDMA_Mem_Create2, modified and then passed to 
	BDMA_Mem_Create2. This avoid compatibilty problem if more elements are
	added into BDMA_Mem_Settings in the future, and save application tedious 
	work of filling in the configuration structure.

	It extremely inportant that pDefSettings has been assigned to a valid 
	struct buffer address by user before this function is called. 

Input:
	<None>

Output:
	pDefSettings - A copy of default settings structure.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully get VDC default settings.

See Also:
	BDMA_Mem_Create2.
**************************************************************************/
BERR_Code BDMA_Mem_GetDefaultSettings
	( BDMA_Mem_Settings               *pDefSettings );

/***************************************************************************
Summary:
	Set the data endian and swap mode.

Description:
	When memory DMA engine is transferring a block of data, it reads data
	from and writes data into memory according to data endian. And right 
	before the writing operation, data could be applied either byte aligned
	swap or short aligned swap inside a 32 bits word. This function sets 
	the data endian and the swap mode.
	
	The data endian affects which bytes to pick in a 32 bits word when a 
	non-word-aligned source address is used, and determines the order of 
	bytes in a word to send to the byte stream for further operation such 
	as encrypt, and the order of bytes in a word to store into memory.

	The sub-module maintains a current state of read endian and swap mode. 
	Each setting modifies the current state and is applied to the future 
	calls to BDMA_Mem_Tran_SetDmaBlockInfo(_isr), unless it is reset again. 
	This implies that in a transfer each block could have different read
	endian and swap mode.

Input:
	hMemDma - Sub-module handle to the memory DMA engine to modify.
	eDataEndian - Data endian (little or big endian) format in memory.
	eSwapMode - Swap mode enum

Output:

Returns:
	BERR_SUCCESS - Swap mode is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr 
****************************************************************************/
BERR_Code BDMA_Mem_SetByteSwapMode(
	BDMA_Mem_Handle          hMemDma,
	BDMA_Endian              eDataEndian,
	BDMA_SwapMode            eSwapMode );


/***************************************************************************
 * Sub-Module: Transfer of Memory DMA
 **************************************************************************/

/***************************************************************************
Summary:
	Memory DMA Transfer sub-module handle.

Description:
	A memory DMA transfer is a sub-module of the Memory DMA engine (again a
	sub-module of BDMA). BDMA_Mem_Tran_Handle is an opaque handle that holds 
	the context for a memory DMA transfer. It is created by 
	BDMA_Mem_Tran_Create2.

	To perform a transfer with the memory DMA engine (sub-module), a memory 
	DMA transfer handle must be created first, then the transfer handle could
	be used to set block info, to start, and to get the status.

	A transfer typically requests to dma a list of contiguous blocks. Each
	contiguous block is represented by a dma descriptor for hardware. The
	transfer is then described by a list of linked dma descriptors. When 
	"block info" is set for a block, it is set to the corresponding dma
	descriptor.

	A transfer handle could be reused after it is started and finished. It is 
	solely user's responsibility to ensure that each block has the right 
	addresses, swap mode and crypto setting.
	
	A transfer handle has to be explicitly destroyed when it is no longer 
	needed. Otherwise the max limit of the number of active transfer blocks
	would be easily reached, that means new transfer creation would fail.

	Please refer to BDMA_Mem_Handle description and the DMA module overview
	for more usage info.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_StartDma, BDMA_Mem_Tran_StartDmaAndCallBack
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Destroy, BDMA_Mem_Tran_Reset
	BDMA_Mem_Tran_Create2_isr, BDMA_Mem_Tran_SetDmaBlockInfo_isr
	BDMA_Mem_Tran_StartDmaAndCallBack_isr
	BDMA_Mem_Tran_GetStatus_isr
	BDMA_Mem_Tran_Destroy_isr, 	BDMA_Mem_Tran_Reset_isr
	BDMA_Mem_Handle, BDMA
***************************************************************************/
typedef struct BDMA_P_Mem_TranContext          *BDMA_Mem_Tran_Handle;
	
/***************************************************************************
Summary:
	Function pointer type of user DMA interrupt call back function.

Description:
	The call back function is coded by the user, is passed to BDMA when a 
	Tran is started with BDMA_Mem_Tran_StartDmaAndCallBack, and is called 
	by BDMA after the Tran is completed, in interrupt hanlder mode, to 
	process the "dma done info.

	It should take three parameters. One is the "user data buffer pointer", 
	passed to the transfer starting API function. It is for user to pass 
	user defined data structure to the call back function, in order to 
	process the "done info". The other two are the transfer handle and 
	eStatus, which are provided by the DMA module and are used to specify 
	which transfer just finished and the transfer status.

See Also:
	BDMA_Mem_Tran_StartDmaAndCallBack, BDMA_Mem_Tran_StartDmaAndCallBack_isr
****************************************************************************/
typedef void (* BDMA_Mem_CallbackFunc)( void *               pUserCallBackParam,\
										BDMA_Mem_Tran_Handle hTran, \
										BDMA_TranStatus      eStatus );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This API function is used to create the transfer handle.

	BDMA_Mem_Tran_Create2 takes an input parameter ulNumBlocks, to specify 
	the max number of contiguous data blocks this transfer is to transfer. 
	It should NOT be bigger than ulMaxNumBlocksPerTran in the 
	BDMA_Mem_Settings passed to BDMA_Mem_Create2.

	A transfer handle could be reused after it is started and finished, and a
	transfer handle has to be explicitly destroyed when it is no longer 
	needed.

	When a block info is set with BDMA_Mem_Tran_SetDmaBlockInfo, it is written
	into a descriptor in the memory, and the descriptor is later read by HW.
	Either cached address or non-cached address could be used by BDMA when the
	descriptor is written. A true value for bCachedDesc indicates cached 
	address should be used.

	When cached address is used for the descriptor writting, it will be 
	flushed into memory by BDMA_Mem_Tran_StartDmaAndCallBack or
	BDMA_Mem_Tran_StartDma.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hMemDma - Sub-module handle to the memory DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Max number of blocks in this transfer.
	bCachedDesc - Whether use cached address to modify dma descriptor.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_StartDma, BDMA_Mem_Tran_StartDmaAndCallBack
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Destroy, BDMA_Mem_Tran_Reset
	BDMA_Mem_Tran_Create2_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Create2(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Mem_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Set encryption / decryption.

Description:
	During a DMA data transfer, Memory DMA engines could be configured to
	perform encryption or decryption. This API function is used to set the 
	crypto.

	In the case that eCryptMode is BDMA_CryptMode_eNoCrypt, the crypto HW 
	module is disabled and the setting to ulKeySlot has no effect.

	If eCryptMode has other valid value, the crypto HW is enabled and 
	eCryptMode specifies how the crypto treats the transport packet 
	boundaries (refer to BDMA_CryptMode for detail). 

	Further, when scatter-gather SCRAM operation is enabled by bSgEnable, the 
	dma engine will treat the data stream section of a list of dma blocks as a 
	whole for encrypt or decrypt purpose. The stream section starts from the 
	beginning of the block marked as scatter-gather start point, and ends at 
	the end of the block marked as scatter-gather end point. bSgEnable can not 
	change between the scatter-gather start point and end point.

	The rest of the crypto configuration, such as what crypto operation 
	(either encryption or decryption) to perform, what crypto algorithm and 
	what key value to use during crypto operation, was programmed into a key 
	slot with the security module. ulKeySlot selects the key slot to use for
	the crypto. 

	There are 16 key slots in a 7401 chip for the moment. The value (between 
	0 and 15) of ulKeySlot passed to this function must be the same one used 
	in the security module when the key slot was programmed.

	The sub-module maintains a current state of crypt mode and key selection. 
	Each setting modifies the current state and is applied to the future 
	calls to BDMA_Mem_Tran_SetDmaBlockInfo(_isr), unless it is reset again. 
	This implies that in a transfer each block could use different crypto key.

	Application software is responsible to program the selected key slot with
	the security module. If the key slot is not configured well, the dma result 
	is undetermined.

Input:
	hTran - The transfer handle to set crypto.
	BDMA_CryptMode - Crypto mode. If it is BDMA_CryptMode_eNoCrypt, crypto is
	disabled. Otherwise it specifies how transport packet boundary is treated.
	ulKeySlot - Key slot (between 0 and 15) selected for the crypto.
	bSgEnable - Enable scatter-gather SCRAM operation.

Output:

Returns:
	BERR_SUCCESS - crypto setting is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetCrypto_isr,
	BDMA_Mem_Tran_SetSgStartEnd, BDMA_Mem_Tran_SetSgStartEnd_isr,
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr	
****************************************************************************/
BERR_Code BDMA_Mem_Tran_SetCrypto(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable);

/***************************************************************************
Summary:
	Set the block info for one block of a DMA transfer.

Description:
	Before a dma transfer can be started, block info has to be set for the
	blocks with block index from 0 to (ulNumActBlocks - 1) (ulNumActBlocks 
	is the the number of currently active blocks, passed as a parameter of
	BDMA_Mem_Tran_StartDma or BDMA_Mem_Tran_StartDmaAndCallBack). This API 
	function is used to set block info into HW dma descriptor for one block.

	ulBlockId is the index of the block, in the block list of the transfer
	represented by hTran. It starts from 0.

	The source and destination address are in memory bus address space. They
	are 28 bits unsigned integer number in 7401 chip. It is more likely that 
	they are converted from virtual memory mapped address using 
	BMEM_ConvertAddressToOffset and the virtual address is returned by
	BMEM_AllocAligned or BMEM_Alloc.

	Both addresses and block size are byte aligned. Max block size is
	0xfff,ffff (16 MByte). Source and destination region overlap with 
	(ulSrcBusAddr < ulDstBusAddr < ulSrcBusAddr+ulBlockSize) is not allowed.

	Encryption and decryption algorithms typically work on memory size of 
	multiple 8 bytes or 16 bytes, and certain residue mode, such as residue 
	clear termination or OFB, might be used for the remaining bytes in the 
	block.

	Please notice that when a non-word-aligned source address is used, the
	read endian determine which bytes to read in a 32 bits word.

	The current state of read endian, swap mode maintained in the dma engine,
	and the current state of crypto setting maintained in hTran, are used 
	during the transfer of this block.

	If the Tran is re-used, and there is no change to this block info (
	including the state of read endian ...), there is no need to set info
	for this block again. However, user should pay extra attention to correct 
	the previous setting to scatter-gather start/stop points for each active
	block, unless BDMA_Mem_Tran_Reset/_isr has been used to set them into 
	default values.	

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	ulDstBusAddr - Destination address in memory bus address space.
	ulSrcBusAddr - Source address in memory bus address space.
	ulBlockSize - This block size in bytes.
	bCryptInit - Whether initialize the encryption / decryption or continue
	the encryption / decryption from previous block.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BDMA_ERR_OVERLAP - Source and destination region overlap.
	BDMA_ERR_SIZE_OUT_RANGE - ulBlockSize is bigger than 16 MByte.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2
	BDMA_Mem_Tran_StartDma, BDMA_Mem_Tran_StartDmaAndCallBack
	BDMA_Mem_SetByteSwapMode, BDMA_Mem_Tran_SetCrypto
	BDMA_Mem_Tran_SetDmaBlockInfo_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_SetDmaBlockInfo(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit );

/***************************************************************************
Summary:
	Mark one block as the start point and/or end point of scatter-gather.

Description:
	When crypto and scatter-gather SCRAM operation are enabled, the dma engine 
	will treat the data stream section of a list of dma blocks as a whole for 
	encrypt or decrypt purpose. The stream section starts from the beginning 
	of the block marked as scatter-gather start point, and ends at the end of
	the block marked as scatter-gather end point.

	This API function is used to mark the block with ID of ulBlockId as scatter-
	gather start and/or end point.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	bStartSgScram - mark this block as scatter-gather start point.
	bEndSgScram - mark this block as scatter-gather end point.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetCrypto, BDMA_Mem_Tran_SetDmaBlockInfo,
	BDMA_Mem_Tran_SetSgStartEnd_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_SetSgStartEnd(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram );

/***************************************************************************
Summary:
	Start a DMA transfer.

Description:
	This API function is used to start a DMA transfer.
	
	BDMA_Mem_Tran_StartDma takes, as an input parameter, hTran to specify the 
	transfer to start.

	As starting a transfer with this API function, user needs to implement a 
	status checking and sleeping loop, to wait for the completion of the
	DMA transfer, and then to process the "done" info.

	Before a dma transfer can be started, block info has to be set with
	BDMA_Mem_Tran_SetDmaBlockInfo for all blocks with block index from 0 to 
	(ulNumActBlocks - 1). ulNumActBlocks should be bigger than 0, and less 
	than or equal to ulNumBlocks passed to BDMA_Mem_Tran_Create2.

	If this Tran is reused, user should pay extra attention to correct the 
	the previous setting to scatter-gather start/stop points for each active 
	block, unless BDMA_Mem_Tran_Reset/_isr has been used to set them into 
	default values.

	when the dma hardware engine is shared by multiple BDMA instances, the 
	transfer might be queued into software until the hardware is no longer in 
	use by another BDMA instance. User provided lock call back functions are 
	used to ensure atomic access to the hardware dma engine.

	If cached address is used for the descriptor writting, it will be 
	flushed into memory inside BDMA_Mem_Tran_StartDma.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BDMA_ERR_ENGINE_BUSY - The engine is busy.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_SetDmaBlockInfo, 
	BDMA_Mem_Tran_GetStatus, 
	BDMA_LockCallbackFunc, BDMA_Mem_Settings, 
	BDMA_Mem_Tran_StartDmaAndCallBack
****************************************************************************/
BERR_Code BDMA_Mem_Tran_StartDma(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks );

/***************************************************************************
Summary:
	Start a DMA transfer from a specific address.

Description:
	This API function is used to start a DMA transfer from some address.Refer 
	to BDMA_Mem_Tran_StartDma for more details.

Input:
	hTran - The transfer handle to start.
	ulFirstBlock - specific starting address
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BDMA_ERR_ENGINE_BUSY - The engine is busy.

****************************************************************************/

BERR_Code BDMA_Mem_Tran_StartDmaSubset(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks );
/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is used to start a DMA transfer, and to register a call 
	back function to be called when the transfer finishes. This API function
	can be used only if "true" is set to bSupportCallBack in the struct
	BDMA_Mem_Settings passed to BDMA_Mem_Create2, otherwise error is returned.

	It takes, as input parameters, hTran to specify the transfer to start,
	a call back function pointer, and a user data buffer pointer. Then the 
	call back function will be called by the "DMA done" interrupt handler
	of the module, when the transfer finishes.

	The call back function is ideally coded by the user. It should take three 
	parameters. One is the "user data buffer pointer", passed to this API
	function. It is for user to pass user defined data structure to the call 
	back function, in order to process the "done info". Another two are 
	transfer handle hTran and eStatus, which are provided by the DMA module 
	and are used to specify which transfer just finished and its status. 

	Before a dma transfer can be started, block info has to be set with
	BDMA_Mem_Tran_SetDmaBlockInfo for all blocks with block index from 0 to 
	(ulNumActBlocks - 1). ulNumActBlocks should be bigger than 0, and less 
	than or equal to ulNumBlocks passed to BDMA_Mem_Tran_Create2.

	If this Tran is reused, user should pay extra attention to correct the 
	the previous setting to scatter-gather start/stop points for each active 
	block, unless BDMA_Mem_Tran_Reset/_isr has been used to set them into 
	default values.

	when the dma hardware engine is shared by multiple BDMA instances, the 
	transfer might be queued into software until the hardware is no longer in 
	use by another BDMA instance. User provided lock call back functions are 
	used to ensure atomic access to the hardware dma engine.

	If cached address is used for the descriptor writting, it will be 
	flushed into memory inside BDMA_Mem_Tran_StartDmaAndCallBack.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_GetStatus
	BDMA_LockCallbackFunc, BDMA_Mem_Settings, 
	BDMA_Mem_Tran_StartDma
	BDMA_Mem_Tran_StartDmaAndCallBack_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_StartDmaAndCallBack(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Mem_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );
/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is used to start a DMA transfer

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.
	ulFirstBlock - specific starting address	
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_LockCallbackFunc, BDMA_Mem_Settings, 
	BDMA_Mem_Tran_StartDma
	BDMA_Mem_Tran_StartDmaAndCallBack_isr
	BDMA_Mem_Tran_StartDmaSubset
****************************************************************************/
BERR_Code BDMA_Mem_Tran_StartDmaSubsetAndCallBack(
	BDMA_Mem_Tran_Handle	 hTran,
	uint32_t				 ulFirstBlock,
	uint32_t				 ulNumActBlocks,
	BDMA_Mem_CallbackFunc	 pCallBackFunc_isr,
	void *					 pUserCallBackParam );

/***************************************************************************
Summary:
	Query the status of a transfer.

Description:
	In order to perform a DMA transfer, user first creates a transfer handle,
	then uses it to set each block info, to start the DMA transfer, and 
	finally to get the transfer status. This API function is used to get the
	status of a DMA transfer.

Input:
	hTran - The transfer handle to query status for.

Output:
	peTranStatus - Transfer status returned.

Returns:
	BERR_SUCCESS - Valid status is returned.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_StartDma, BDMA_Mem_Tran_StartDmaAndCallBack
	BDMA_Mem_Tran_GetStatus_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_GetStatus(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus );

/***************************************************************************
Summary:
	Destroy a DMA transfer handle.

Description:
	This API function is used to destroy a no-longer needed DMA transfer
	handle. It frees the resources occupied by this transfer handle. After
	a transfer handle is destroyed, it can no-longer be used to set block, 
	to start, or to get transfer status.
	
	BDMA_Mem_Tran_Destroy takes, as an input parameter, hTran to specify the 
	transfer handle to destroy.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to destroy.

Output:

Returns:
	BERR_SUCCESS - Transfer handle is destroyed.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_Reset, 
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Destroy_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Destroy(
	BDMA_Mem_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Reset a DMA transfer handle.

Description:
	This API function is used to reset DMA transfer handle to its initially
	created state. 

	For an initially created or reset transfer handle, the module ensures
	that block info is set for each transfered blocks, when it is started.
	Without resetting, user needs to pay extra attention, to ensure that
	all un-reset block info have correct addresses and configures.
	
	BDMA_Mem_Tran_Reset takes, as an input parameter, hTran to specify the 
	transfer handle to reset.

	After a transfer handle is reset, its status is reset to
	BDMA_TranStatus_eCreated.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to reset.

Output:

Returns:
	BERR_SUCCESS - The transfer handle is reset successfully.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_Destroy, 
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Reset_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Reset(
	BDMA_Mem_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_Create2. It is used in 
	interrupt handler or critical session. For more description refer to
	BDMA_Mem_Tran_Create2. 

Input:
	hMemDma - Sub-module handle to the memory DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Max number of blocks in this transfer.
	bCachedDesc - Whether use cached address to modify dma descriptor.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetDmaBlockInfo_isr
	BDMA_Mem_Tran_StartDmaAndCallBack_isr
	BDMA_Mem_Tran_GetStatus_isr
	BDMA_Mem_Tran_Destroy_isr, BDMA_Mem_Tran_Reset_isr
	BDMA_Mem_Tran_Create2
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Create2_isr(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Mem_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Set encryption / decryption.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_SetCrypto. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_SetCrypto. 

Input:
	hTran - The transfer handle to set crypto.
	BDMA_CryptMode - Crypto mode. If it is BDMA_CryptMode_eNoCrypt, crypto is
	disabled. Otherwise it specifies how transport packet boundary is treated.
	ulKeySlot - Key slot (between 0 and 15) selected for the crypto.
	bSgEnable - Enable scatter-gather SCRAM operation.

Output:

Returns:
	BERR_SUCCESS - crypto setting is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetCrypto,
	BDMA_Mem_Tran_SetSgStartEnd, BDMA_Mem_Tran_SetSgStartEnd_isr,
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr	
****************************************************************************/
BERR_Code BDMA_Mem_Tran_SetCrypto_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable);

/***************************************************************************
Summary:
	Set the block info for one block of a DMA transfer.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_SetDmaBlockInfo. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_SetDmaBlockInfo. 

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer.
	ulDstBusAddr - Destination address in memory bus address space.
	ulSrcBusAddr - Source address in memory bus address space.
	ulBlockSize - This block size in bytes.
	bCryptInit - Whether initialize the encryption / decryption or continue
	the encryption / decryption from previous block.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BDMA_ERR_OVERLAP - Source and destination region overlap.
	BDMA_ERR_SIZE_OUT_RANGE - ulBlockSize is bigger than 16 MByte.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2
	BDMA_Mem_Tran_StartDma, BDMA_Mem_Tran_StartDmaAndCallBack
	BDMA_Mem_SetByteSwapMode, BDMA_Mem_Tran_SetCrypto_isr
	BDMA_Mem_Tran_SetDmaBlockInfo_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_SetDmaBlockInfo_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit );

/***************************************************************************
Summary:
	Mark one block as the start point and/or end point of scatter-gather.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_SetSgStartEnd. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_SetSgStartEnd.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	bStartSgScram - mark this block as scatter-gather start point.
	bEndSgScram - mark this block as scatter-gather end point.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetCrypto_isr, BDMA_Mem_Tran_SetDmaBlockInfo_isr,
	BDMA_Mem_Tran_SetSgStartEnd
****************************************************************************/
BERR_Code BDMA_Mem_Tran_SetSgStartEnd_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	bool                     bStartSgScram,
	bool                     bEndSgScram );

/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_StartDmaAndCallBack. It is used 
	in interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_StartDmaAndCallBack. 

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2_isr, BDMA_Mem_Tran_SetDmaBlockInfo_isr
	BDMA_Mem_Tran_GetStatus_isr
	BDMA_Mem_Tran_StartDmaAndCallBack
****************************************************************************/
BERR_Code BDMA_Mem_Tran_StartDmaAndCallBack_isr(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,	
	BDMA_Mem_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
Summary:
	Query the status of a transfer.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_GetStatus. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_GetStatus.

Input:
	hTran - The transfer handle to query status for.

Output:
	peTranStatus - Transfer status returned.

Returns:
	BERR_SUCCESS - Valid status is returned.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_StartDma, BDMA_Mem_Tran_StartDmaAndCallBack
	BDMA_Mem_Tran_GetStatus_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_GetStatus_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus );

/***************************************************************************
Summary:
	Destroy a DMA transfer handle.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_Destroy. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_Destroy.

Input:
	hTran - The transfer handle to destroy.

Output:

Returns:
	BERR_SUCCESS - Transfer handle is destroyed.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_Reset, 
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Destroy_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Destroy_isr(
	BDMA_Mem_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Reset a DMA transfer handle.

Description:
	This is the "_isr" version of BDMA_Mem_Tran_Reset. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_Reset.

Input:
	hTran - The transfer handle to reset.

Output:

Returns:
	BERR_SUCCESS - The transfer handle is reset successfully.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Mem_Tran_Create2, BDMA_Mem_Tran_Destroy, 
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Reset_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Reset_isr(
	BDMA_Mem_Tran_Handle     hTran );


/***************************************************************************
 * Sub-Module: PCI DMA 
 **************************************************************************/

/***************************************************************************
Summary:
	PCI DMA engine sub-module handle, or PCI DMA handle for simplicity.

Description:
	This is an opaque handle that represents and controls one HW PCI DMA 
	engine. It is needed when the PCI DMA engine is configured or used.

	It is created with BDMA_Pci_Create2. 

	The typical process to configure and use a PCI DMA engine is as
	follows:

	o Open the DMA module handle.
	o A sub-module handle to the PCI DMA engine is created.
	o Configure PCI DMA engine (sub-module), such as swapping mode, max
	  burst size.
	o Call BDMA_Pci_Tran_Create2 to create a PCI DMA transfer handle.
	o Set the block info (such as memory bus address, PCI bus address, 
	  transfer direction, and block size) for each data block to be 
	  transferred by calling BDMA_Pci_Tran_SetBlockInfo.
	o Start the transfer by BDMA_Pci_Tran_StartDma, or 
	  BDMA_Pci_Tran_StartDmaAndCallBack.
	o Process the "DMA done" information. Depending on which starting API
	  function was called above, BDMA_Pci_Tran_StartDma or 
	  BDMA_Pci_Tran_StartDmaAndCallBack, the "DMA done" information could 
	  either be acquired by actively calling BDMA_Pci_Tran_GetStatus, or
	  informed by a call back from BDMA.

	There are two engine configure API functions for PCI DMA sub-module,
	BDMA_Pci_SetSwapMode and BDMA_Pci_SetMaxBurstSize. The sub-module 
	maintains a current state of configure. Each setting modifies current 
	state. Current swap mode is applied when BDMA_Pci_Tran_SetBlockInfo is 
	called in the future, unless it is reset again. This means that in a 
	transfer each block could have different swap mode. But Current max 
	burst size is applied to future calls of BDMA_Pci_Tran_StartDma and
	BDMA_Pci_Tran_StartDmaAndCallBack, that means the blocks of one transfer 
	have to use the same max burst size.

	After the engine is configured, user could also perform a DMA transfer 
	in an interrupt handler or critical session, using the API function set 
	named with an "_isr" suffix.

	When a DMA engine sub-module handle is no longer needed, it should be
	destroyed with BDMA_Pci_Destroy.

	Please refer to the DMA module overview for more usage info.

See Also:
	DMA, BDMA_Handle
	BDMA_Pci_Create2, BDMA_Pci_Destroy
	BDMA_Pci_SetSwapMode
	BDMA_Pci_Tran_Create2, BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_StartDma, BDMA_Pci_Tran_StartDmaAndCallBack
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_Tran_Reset, BDMA_Pci_Tran_Destroy
	BDMA_Pci_Tran_Create2_isr, BDMA_Pci_Tran_SetBlockInfo_isr
	BDMA_Pci_Tran_StartDmaAndCallBack_isr
	BDMA_Pci_Tran_GetStatus_isr
	BDMA_Pci_Tran_Reset_isr, BDMA_Pci_Tran_Destroy_isr
***************************************************************************/
typedef struct BDMA_P_Pci_Context        *BDMA_Pci_Handle;

/***************************************************************************
Summary:
	This enumeration represents pci dma engines.

Description:
	BDMA_PciDmaEngine is an enumeration which represents all HW pci dma 
	engines there exist in the chip. It is used to indicate which HW engine
	to use when a PCI dma handle is created by BDMA_Pci_Create2.

See Also:
	BDMA_Pci_Create2
***************************************************************************/
typedef enum BDMA_PciDmaEngine
{
	BDMA_PciDmaEngine_e0,     /* pci dma engine 0 */
	BDMA_PciDmaEngine_eUnknown
}
BDMA_PciDmaEngine;

/***************************************************************************
Summary:
	This enumeration represents DMA max burst size.

Description:
	BDMA_MaxBurstSize is an enumeration which represents supported DMA max 
	burst sizes during DMA transfer.

See Also:
	BDMA_Pci_SetMaxBurstSize	
***************************************************************************/
typedef enum BDMA_MaxBurstSize
{
	BDMA_MaxBurstSize_eBurst16,     /* max burst size 16 */
	BDMA_MaxBurstSize_eBurst1,      /* single transfer, no burst */
	BDMA_MaxBurstSize_eInvalid
}
BDMA_MaxBurstSize;

/***************************************************************************
Summary:
	This enumeration represents the direction of a PCI DMA transfer.

Description:
	BDMA_TranDir is an enumeration which represents the transfer direction
	of a block in a PCI DMA transfer.

See Also:
	BDMA_Pci_Tran_SetBlockInfo, BDMA_Pci_Tran_SetBlockInfo_isr
***************************************************************************/
typedef enum BDMA_TranDir
{
	BDMA_TranDir_eSdramToPciDevice,  /* from SDRAM to a PCI device */
	BDMA_TranDir_ePciDeviceToSdram,  /* from a PCI device to SDRAM */
	BDMA_TranDir_eInvalid
}
BDMA_TranDir;

/***************************************************************************
Summary:
	This structure describes the settings for BDMA_Pci_Handle

Description:
	BDMA_Pci_Settings is a structure that is used to describe the public
	settings of a pci dma engine.  The settings are in engine scope.

	ulTotalNumBlocks - The maximum number of blocks could be supported
	by this dma engine in total. Its default value is 256.

	ulMaxNumBlocksPerTran - The maxium number of blocks per transfer. Its
	default value is 128.

	bSupportCallBack - When multiple BDMA instance exist in the system, 
	only one instance can support call-back mode. bSupportCallBack indicates
	whether this BDMA instance is the unique one that can support call-back 
	mode. Its default value is true.

	pTryLock - User provided call back function that is called to lock the 
	hardware pci dma engine from accessing by other BDMA instances. Its
	default value is NULL.

	pReleaseLock - User provided call back function that is called to release
	the lock so that the hardware pci dma engine could be accessed by
	other BDMA instances. Its default value is NULL.

	pvParm1 - Pointer to a user defined struct. It is passed to (*pTryLock)
	and (*pReleaseLock) as a parameter. Its default value is NULL.

	iParm2 - Second user parameter for the lock call-back functions. It is
	passed to (*pTryLock)and (*pReleaseLock) as a parameter. Its default value
	is 0.

	If there is only one BDMA instance in the system, bSupportCallBack,
	pTryLock, pReleaseLock, pvParm1, iParm2 should be left with default value.

See Also:
	BDMA_Pci_Create2, BDMA_Pci_GetDefaultSettings
***************************************************************************/
typedef struct BDMA_Pci_Settings
{
	uint32_t                           ulTotalNumBlocks;
	uint32_t                           ulMaxNumBlocksPerTran;

	/* settings for the case of multiple PCI dma usage from multiple BDMA instances */
	BDMA_LockCallbackFunc              pTryLock;
	BDMA_LockCallbackFunc              pReleaseLock;
	void                             * pvParm1;
	int                                iParm2;
	bool                               bSupportCallBack;
}
BDMA_Pci_Settings;

/***************************************************************************
Summary:
	Creates a sub-module handle to PCI DMA engine.

Description:
	This API function is used to create a BDMA_Pci_Handle to represent and
	control a specific HW PCI dma engine. It is required by PCI DMA sub-
	module functions in order to configure the DMA engine and perform 
	DMA transfer. 

	If the sub-module handle has already been created for the same HW engine,
	error is returned. This function also initializes the sub-module to 
	default configure.

	Before BDMA_Pci_Create2 is called, BDMA_Pci's inherent default setting
	structure should be queried by BDMA_Pci_GetDefaultSettings, modified and 
	then passed to BDMA_Pci_Create2. This avoid compatibilty problem if more 
	elements are added into BDMA_Pci_Settings in the future, and save 
	application tedious work of filling in the configuration structure.

	Refer to BDMA_Pci_Handle description for detailed usage info.

Input:
	hDma - Handle to the BDMA module.
	eEngine - The HW pci dma engine controlled by this BDMA_Pci_Handle.
	pSettings - The basic settings. Default setting is used if pSettings is
	NULL.

Output:
	phPciDma - The created PCI DMA sub-module handle. If failure 
	occurred phPciDma will holds NULL.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully created the handle.

See Also:
	BDMA_Pci_Destroy
**************************************************************************/
BERR_Code BDMA_Pci_Create2(
	BDMA_Handle              hDma,
	BDMA_PciDmaEngine        eEngine,
	BDMA_Pci_Settings *      pSettings,
	BDMA_Pci_Handle *        phPciDma );

/***************************************************************************
Summary:
	Destroys the sub-module handle of the PCI DMA engine. 

Description:
	Once this function is called the sub-module handle can no longer be 
	used.

	PCI DMA engine sub-module handle should be destroyed before closing 
	the main module handle BDMA_Handle by BDMA_Close.

Input:
	hPciDma - The PCI DMA engine sub-module handle to destroy.

Output:

Returns:
	BERR_INVALID_PARAMETER - hPciDma is not a valid PCI DMA engine sub-
	module handle.
	BERR_SUCCESS - Successfully destroyed

See Also:
	BDMA_Pci_Create2
**************************************************************************/
BERR_Code BDMA_Pci_Destroy(
	BDMA_Pci_Handle          hPciDma );

/***************************************************************************
Summary:
	This function gets BDMA_Pci's inherent default setting structure.

Description:
	BDMA_Pci's inherent default setting structure could be queried by this 
	API function prior to BDMA_Pci_Create2, modified and then passed to 
	BDMA_Pci_Create2. This avoid compatibilty problem if more elements are
	added into BDMA_Pci_Settings in the future, and save application tedious 
	work of filling in the configuration structure.

	It extremely inportant that pDefSettings has been assigned to a valid 
	struct buffer address by user before this function is called. 

Input:
	<None>

Output:
	pDefSettings - A copy of default settings structure.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully get VDC default settings.

See Also:
	BDMA_Pci_Create2.
**************************************************************************/
BERR_Code BDMA_Pci_GetDefaultSettings
	( BDMA_Pci_Settings               *pDefSettings );

/***************************************************************************
Summary:
	Set the swap mode.

Description:
	During a DMA data transfer, PCI DMA engines could be programmed to 
	perform translation between little endian and big endian with either 
	byte aligned swap or short aligned swap. This function sets the swap
	mode.

	The sub-module maintains a current state of swap mode. Each setting 
	modifies the current state and is applied to the future calls to
	BDMA_Pci_Tran_SetBlockInfo(_isr), unless it is reset again. This 
	means that in a transfer each block could have different swap mode.

Input:
	hPciDma - Sub-module handle to the PCI DMA engine to modify.
	eSwapMode - Swap mode enum

Output:

Returns:
	BERR_SUCCESS - Swap mode is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BDMA_Pci_Tran_SetBlockInfo, BDMA_Pci_Tran_SetBlockInfo_isr	
****************************************************************************/
BERR_Code BDMA_Pci_SetSwapMode(
	BDMA_Pci_Handle          hPciDma,
	BDMA_SwapMode            eSwapMode );

/***************************************************************************
Summary:
	Set the max burst size during DMA transfer.

Description:
	During a DMA data transfer, PCI DMA engines could be programmed to 
	perform burst access on PCI bus. This function specifies the max burst
	size. PCI DMA engine might does burst access with burst size smaller
	than the max burst size specified here, depending on the memory bus and
	PCI bus traffic.

	The sub-module maintains a current state of max burst size. Each 
	setting modifies the current state and is applied to the future calls 
	of BDMA_Pci_Tran_StartDma/StartDmaAndCallBack(_isr), this means that the 
	blocks of one transfer have to use the same max	burst size.

Input:
	hPciDma - Sub-module handle to the PCI DMA engine to modify.
	eMaxBurstSize - Max burst size on PCI bus during DMA transfer.

Output:

Returns:
	BERR_SUCCESS - Max burst size is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BDMA_Pci_Tran_StartDma, BDMA_Pci_Tran_StartDmaAndCallBack
	BDMA_Pci_Tran_StartDmaAndCallBack_isr
****************************************************************************/
BERR_Code BDMA_Pci_SetMaxBurstSize(
	BDMA_Pci_Handle       hPciDma,
	BDMA_MaxBurstSize     eMaxBurstSize );


/***************************************************************************
 * Sub-Module: Transfer of PCI DMA
 **************************************************************************/

/***************************************************************************
Summary:
	PCI DMA Transfer sub-module handle.

Description:
	A PCI DMA transfer is a sub-module of the PCI DMA engine (a sub-module of 
	BDMA). BDMA_Pci_Tran_Handle is an opaque handle that holds the context 
	for a PCI DMA transfer. It is created with BDMA_Pci_Tran_Create2.

	To perform a transfer with the PCI DMA engine (sub-module), a PCI DMA 
	transfer handle must be created first, then the transfer handle could be 
	used to set block info, start, and get the status.

	A transfer typically requests to dma a list of contiguous blocks. Each
	contiguous block is represented by a dma descriptor for hardware. The
	transfer is then described by a list of linked dma descriptors. When 
	"block info" is set for a block, it is set to the corresponding dma
	descriptor.

	A transfer handle could be reused after it is started and finished. It is 
	solely user's responsibility to ensure that each block has the right 
	addresses and swap mode, max burst size.
	
	A transfer handle has to be explicitly destroyed when it is no longer 
	needed. Otherwise the max limit of the number of active transfer blocks
	would be easily reached, that means new transfer creation would fail.

	Please refer to BDMA_Pci_Handle description and the DMA module overview
	for more usage info.

See Also:
	BDMA_Pci_Tran_Create2, BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_StartDma, BDMA_Pci_Tran_StartDmaAndCallBack
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_Tran_Destroy, BDMA_Pci_Tran_Reset
	BDMA_Pci_Tran_Create2_isr, BDMA_Pci_Tran_SetBlockInfo_isr
	BDMA_Pci_Tran_StartDmaAndCallBack_isr
	BDMA_Pci_Tran_GetStatus_isr
	BDMA_Pci_Tran_Destroy_isr, 	BDMA_Pci_Tran_Reset_isr
	BDMA_Pci_Handle, BDMA
***************************************************************************/
typedef struct BDMA_P_Pci_TranContext          *BDMA_Pci_Tran_Handle;

/***************************************************************************
Summary:
	Function pointer type of user DMA interrupt call back function.

Description:
	The call back function is coded by the user, is passed to BDMA when a 
	Tran is started with BDMA_Pci_Tran_StartDmaAndCallBack, and is called 
	by BDMA after the Tran is completed, in interrupt hanlder mode, to 
	process the "dma done info.

	It should take three parameters. One is the "user data buffer pointer", 
	passed to the transfer starting API function. It is for user to pass 
	user defined data structure to the call back function, in order to 
	process the "done info". The other two are the transfer handle and 
	eStatus, which are provided by the DMA module and are used to specify 
	which transfer just finished and the transfer status.

See Also:
	BDMA_Pci_Tran_StartDmaAndCallBack, BDMA_Pci_Tran_StartDmaAndCallBack_isr
****************************************************************************/
typedef void (* BDMA_Pci_CallbackFunc)( void *               pUserCallBackParam,\
										BDMA_Pci_Tran_Handle hTran, \
										BDMA_TranStatus      eStatus );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This API function is used to create the transfer handle.

	BDMA_Pci_Tran_Create2 takes an input parameter ulNumBlocks, to
	specify the number of contiguous data blocks this transfer is to
	transfer. It should NOT be bigger than ulMaxNumBlocksPerTran in the 
	BDMA_Pci_Settings passed to BDMA_Pci_Create2.

	A transfer handle could be reused after it is started and finished, and
	a transfer handle has to be explicitly destroyed when it is no longer 
	needed.

	When a block info is set with BDMA_Pci_Tran_SetBlockInfo, it is written
	into a descriptor in the memory, and the descriptor is later read by HW.
	Either cached address or non-cached address could be used by BDMA when the
	descriptor is written. A true value for bCachedDesc indicates cached 
	address should be used.

	When cached address is used for the descriptor writting, it will be 
	flushed into memory by BDMA_Pci_Tran_StartDmaAndCallBack or
	BDMA_Pci_Tran_StartDma.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hPciDma - Sub-module handle to the PCI DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Max number of blocks in this transfer.
	bCachedDesc - Whether use cached address to modify dma descriptor.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_StartDma, BDMA_Pci_Tran_StartDmaAndCallBack
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_Tran_Destroy, BDMA_Pci_Tran_Reset
	BDMA_Pci_Tran_Create2_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Create2(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Pci_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Set the block info for one block of a DMA transfer.

Description:
	In order to perform a DMA transfer, user first creates a transfer handle,
	then uses it to set block info for each block, to start the DMA transfer, 
	and finally to get the transfer status. This API function is used to set 
	block info into HW dma descriptor for one block. 
	
	ulBlockId is the index of the block, in the block list of the transfer
	represented by hTran. It starts from 0.

	The PCI address is in the PCI bus address space. It is a 32 bits unsigned
	integer number. And the memory address is the memory bus address space.
	It is a 28 bits number.

	It is more likely that the memory address is converted from virtual 
	memory mapped address using BMEM_ConvertAddressToOffset and the virtual 
	address is returned by BMEM_AllocAligned or BMEM_Alloc.

	eTranDirection is to specify whether the transfer of this block is from 
	SDRAM to a PCI device or another way around.

	Both addresses and block size must be 32-bit aligned. Max block size is 
	0xfff,ffff (16 MByte), and minimum block size is 0x4.

	The current state of swap mode is used during the transfer of this block.

	If the Tran is re-used, and there is no change to this block info (
	including the state of read endian ...), there is no need to set info
	for this block again.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more info
	on the usage of the PCI sub-module API functions.

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer.
	ulMemBusAddr - Memory address in memory bus address space.
	ulPciBusAddr - PCI address in PCI bus address space.
	ulBlockSize - This block size in bytes.
	eTranDirection - Transfer direction of this block.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BDMA_ERR_OVERLAP - Source and destination region overlap.
	BDMA_ERR_NOT_WORD_ALIGNED - ulSrcBusAddr, ulDstBusAddr, or ulBlockSize
	is not 32-bit aligned.
	BDMA_ERR_SIZE_OUT_RANGE - ulBlockSize is bigger than 16 MByte.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create2
	BDMA_Pci_Tran_StartDma, BDMA_Pci_Tran_StartDmaAndCallBack
	BDMA_Pci_SetSwapMode
	BDMA_Pci_Tran_SetBlockInfo_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_SetBlockInfo(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulMemBusAddr,
	uint32_t                 ulPciBusAddr,
	uint32_t                 ulBlockSize,
	BDMA_TranDir             eTranDirection );

/***************************************************************************
Summary:
	Start a DMA transfer.

Description:
	This API function is used to start a DMA transfer.

	BDMA_Pci_Tran_StartDma takes, as an input parameter, hTran to specify the 
	transfer to start. 

	As starting a transfer with this API function, user needs to implement a 
	status checking and sleeping loop, to wait for the completion of the
	DMA transfer, and then to process the "done" info.

	The current state of max burst size is used during the transfer.

	Before a dma transfer can be started, block info has to be set with
	BDMA_Pci_Tran_SetBlockInfo for all blocks with block index from 0 to 
	(ulNumActBlocks - 1). ulNumActBlocks should be bigger than 0, and less 
	than or equal to ulNumBlocks passed to BDMA_Pci_Tran_Create2.

	If cached address is used for the descriptor writting, it will be 
	flushed into memory inside BDMA_Pci_Tran_StartDma.

	when the dma hardware engine is shared by multiple BDMA instances, the 
	transfer might be queued into software until the hardware is no longer in 
	use by another BDMA instance. User provided lock call back functions are 
	used to ensure atomic access to the hardware dma engine.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BDMA_ERR_ENGINE_BUSY - The engine is busy.

See Also:
	BDMA_Pci_Tran_Create2, BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_SetMaxBurstSize
	BDMA_Pci_Settings, BDMA_LockCallbackFunc,
	BDMA_Pci_Tran_StartDmaAndCallBack
****************************************************************************/
BERR_Code BDMA_Pci_Tran_StartDma(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks );

/***************************************************************************
Summary:
	Start a DMA transfer from a specific address.

Description:
	This API function is used to start a DMA transfer from some address.Refer 
	to BDMA_Mem_Tran_StartDma for more details.

Input:
	hTran - The transfer handle to start.
	ulFirstBlock - specific starting address
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BDMA_ERR_ENGINE_BUSY - The engine is busy.

****************************************************************************/
BERR_Code BDMA_Pci_Tran_StartDmaSubset(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulFirstBlock,
	uint32_t                 ulNumActBlocks );
/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is used to start a DMA transfer, and to register a call 
	back function to be called when the transfer finishes. This API function
	can be used only if "true" is set to bSupportCallBack in the struct
	BDMA_Pci_Settings passed to BDMA_Pci_Create2, otherwise error is returned.

	It takes, as input parameters, hTran to specify the transfer to start,
	a call back function pointer, and a user data buffer pointer. Then the 
	call back function will be called by the "DMA done" interrupt handler
	of the module, when the transfer finishes.

	The call back function is ideally coded by the user. It should take three 
	parameters. One is the "user data buffer pointer", passed to this API
	function. It is for user to pass user defined data structure to the call 
	back function, in order to process the "done info". Another two are 
	transfer handle hTran and eStatus, which are provided by the DMA module 
	and are used to specify which transfer just finished and its status. 

	The current state of max burst size is used during the transfer.

	Before a dma transfer can be started, block info has to be set with
	BDMA_Pci_Tran_SetBlockInfo for all blocks with block index from 0 to 
	(ulNumActBlocks - 1). ulNumActBlocks should be bigger than 0, and less 
	than or equal to ulNumBlocks passed to BDMA_Pci_Tran_Create2.

	If cached address is used for the descriptor writting, it will be 
	flushed into memory inside BDMA_Pci_Tran_StartDmaAndCallBack.

	when the dma hardware engine is shared by multiple BDMA instances, the 
	transfer might be queued into software until the hardware is no longer in 
	use by another BDMA instance. User provided lock call back functions are 
	used to ensure atomic access to the hardware dma engine.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create2, BDMA_Pci_Tran_SetBlockInfo,
	BDMA_Pci_Tran_GetStatus,
	BDMA_Pci_SetMaxBurstSize,
	BDMA_Pci_Settings, BDMA_LockCallbackFunc,
	BDMA_Pci_Tran_StartDma,
	BDMA_Pci_Tran_StartDmaAndCallBack_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_StartDmaAndCallBack(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,
	BDMA_Pci_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );
/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is used to start a DMA transfer

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.
	ulFirstBlock - specific starting address	
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_LockCallbackFunc, BDMA_Pci_Settings, 
	BDMA_Pci_Tran_StartDma
	BDMA_Pci_Tran_StartDmaAndCallBack_isr
	BDMA_Pci_Tran_StartDmaSubset
****************************************************************************/
BERR_Code BDMA_Pci_Tran_StartDmaSubsetAndCallBack(
	BDMA_Mem_Tran_Handle	 hTran,
	uint32_t				 ulFirstBlock,
	uint32_t				 ulNumActBlocks,
	BDMA_Mem_CallbackFunc	 pCallBackFunc_isr,
	void *					 pUserCallBackParam );

/***************************************************************************
Summary:
	Query the status of a transfer.

Description:
	In order to perform a DMA transfer, user first creates a transfer handle,
	then uses it to set each block info, to start the DMA transfer, and 
	finally to get the transfer status. This API function is used to get the
	status of a DMA transfer.

Input:
	hTran - The transfer handle to query status for.

Output:
	peTranStatus - Transfer status returned.

Returns:
	BERR_SUCCESS - Valid status is returned.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create2, BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_StartDma, BDMA_Pci_Tran_StartDmaAndCallBack
	BDMA_Pci_Tran_GetStatus_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_GetStatus(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus );

/***************************************************************************
Summary:
	Destroy a DMA transfer handle.

Description:
	This API function is used to destroy a no-longer needed DMA transfer
	handle. It frees the resources occupied by this transfer handle. After
	a transfer handle is destroyed, it can no-longer be used to set block, 
	to start, or to get transfer status.
	
	BDMA_Pci_Tran_Destroy takes, as an input parameter, hTran to specify the 
	transfer handle to destroy.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hTran - The transfer handle to destroy.

Output:

Returns:
	BERR_SUCCESS - Transfer handle is destroyed.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Pci_Tran_Create2, BDMA_Pci_Tran_Reset, 
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_Tran_Destroy_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Destroy(
	BDMA_Pci_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Reset a DMA transfer handle.

Description:
	This API function is used to reset DMA transfer handle to its initially
	created state.

	For an initially created or reset transfer handle, the module ensures
	that block info is set for each transfered block, when it is started.
	Without resetting, user needs to pay extra attention, to ensure that
	all un-reset block info have correct addresses and configures.
	
	BDMA_Pci_Tran_Reset takes, as an input parameter, hTran to specify the 
	transfer handle to reset.

	After a transfer handle is reset, its status is reset to
	BDMA_TranStatus_eCreated.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hTran - The transfer handle to reset.

Output:

Returns:
	BERR_SUCCESS - The transfer handle is reset successfully.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Pci_Tran_Create2, BDMA_Pci_Tran_Destroy, 
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_Tran_Reset_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Reset(
	BDMA_Pci_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This is the "_isr" version of BDMA_Pci_Tran_Create2. It is used in 
	interrupt handler or critical session. For more description refer to
	BDMA_Pci_Tran_Create2. 

Input:
	hPciDma - Sub-module handle to the PCI DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Max number of blocks in this transfer.
	bCachedDesc - Whether use cached address to modify dma descriptor.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_SetBlockInfo_isr
	BDMA_Pci_Tran_StartDmaAndCallBack_isr
	BDMA_Pci_Tran_GetStatus_isr
	BDMA_Pci_Tran_Destroy_isr, BDMA_Pci_Tran_Reset_isr
	BDMA_Pci_Tran_Create2
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Create2_isr(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	bool                     bCachedDesc,
	BDMA_Pci_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Set the block info for one block of a DMA transfer.

Description:
	This is the "_isr" version of BDMA_Pci_Tran_SetBlockInfo. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Pci_Tran_SetBlockInfo. 

Input:
	hTran - The transfer handle to set block info.
	ulBlockId - The block index in the block list of this transfer.
	ulMemBusAddr - Memory address in memory bus address space.
	ulPciBusAddr - PCI address in PCI bus address space.
	ulBlockSize - This block size in bytes.
	eTranDirection - Transfer direction of this block.

Output:

Returns:
	BERR_SUCCESS - The block info is set successfully.
	BDMA_ERR_OVERLAP - Source and destination region overlap.
	BDMA_ERR_NOT_WORD_ALIGNED - ulSrcBusAddr, ulDstBusAddr, or ulBlockSize
	is not 32-bit aligned.
	BDMA_ERR_SIZE_OUT_RANGE - ulBlockSize is bigger than 16 MByte.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create2_isr
	BDMA_Pci_Tran_StartDma_isr, BDMA_Pci_Tran_StartDmaAndCallBack_isr
	BDMA_Pci_SetSwapMode_isr
	BDMA_Pci_Tran_SetBlockInfo
****************************************************************************/
BERR_Code BDMA_Pci_Tran_SetBlockInfo_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulMemBusAddr,
	uint32_t                 ulPciBusAddr,
	uint32_t                 ulBlockSize,
	BDMA_TranDir             eTranDirection );

/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This is the "_isr" version of BDMA_Pci_Tran_StartDmaAndCallBack. It is used 
	in interrupt handler or critical session. For more description refer to 
	BDMA_Pci_Tran_StartDmaAndCallBack. 

Input:
	hTran - The transfer handle to start.
	ulNumActBlocks - Block with index from 0 to (ulNumActBlocks - 1) will be
	dma transfered with this start call.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create2_isr, BDMA_Pci_Tran_SetBlockInfo_isr,
	BDMA_Pci_Tran_GetStatus_isr,
	BDMA_Pci_Settings, BDMA_LockCallbackFunc,
	BDMA_Pci_Tran_StartDmaAndCallBack
****************************************************************************/
BERR_Code BDMA_Pci_Tran_StartDmaAndCallBack_isr(
	BDMA_Pci_Tran_Handle     hTran,
	uint32_t                 ulNumActBlocks,	
	BDMA_Pci_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
Summary:
	Query the status of a transfer.

Description:
	This is the "_isr" version of BDMA_Pci_Tran_GetStatus. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Pci_Tran_GetStatus.

Input:
	hTran - The transfer handle to query status for.

Output:
	peTranStatus - Transfer status returned.

Returns:
	BERR_SUCCESS - Valid status is returned.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create2_isr, BDMA_Pci_Tran_SetBlockInfo_isr
	BDMA_Pci_Tran_StartDmaAndCallBack_isr
	BDMA_Pci_Tran_GetStatus
****************************************************************************/
BERR_Code BDMA_Pci_Tran_GetStatus_isr(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_TranStatus *        peTranStatus );

/***************************************************************************
Summary:
	Destroy a DMA transfer handle.

Description:
	This is the "_isr" version of BDMA_Pci_Tran_Destroy. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Pci_Tran_Destroy.

Input:
	hTran - The transfer handle to destroy.

Output:

Returns:
	BERR_SUCCESS - Transfer handle is destroyed.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Pci_Tran_Create2_isr, BDMA_Pci_Tran_Reset_isr, 
	BDMA_Pci_Tran_GetStatus_isr
	BDMA_Pci_Tran_Destroy
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Destroy_isr(
	BDMA_Pci_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Reset a DMA transfer handle.

Description:
	This is the "_isr" version of BDMA_Pci_Tran_Reset. It is used in 
	interrupt handler or critical session. For more description refer to 
	BDMA_Pci_Tran_Reset.

Input:
	hTran - The transfer handle to reset.

Output:

Returns:
	BERR_SUCCESS - The transfer handle is reset successfully.
	BERR_INVALID_PARAMETER - The input hTran is invalid.

See Also:
	BDMA_Pci_Tran_Create2_isr, BDMA_Pci_Tran_Destroy_isr, 
	BDMA_Pci_Tran_GetStatus_isr
	BDMA_Pci_Tran_Reset
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Reset_isr(
	BDMA_Pci_Tran_Handle     hTran );


/***************************************************************************
 * Obsolete Memory Dma API: 
 **************************************************************************/

/***************************************************************************
Summary:
	Creates a sub-module handle to memory DMA engine.

Description:
	This API function is replaced by BDMA_Mem_Create2 and will be obsolete 
	in the future. New code should use the new one and stop using this API 
	function.

	Once created, the handle to the unique memory DMA engine is required 
	by memory DMA sub-module functions in order to configure the DMA engine
	and perform DMA transfer. 

	If the sub-module handle has already been created with the same hDma,
	error is returned. This function also initializes the sub-module to 
	default configure.

	Refer to BDMA_Mem_Handle description for detailed usage info.

Input:
	hDma - Handle to the BDMA module.

Output:
	phMemDma - The created memory DMA sub-module handle. If failure 
	occurred phMemDma will holds NULL.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully created the handle.

See Also:
	BDMA_Mem_Destroy
**************************************************************************/
BERR_Code BDMA_Mem_Create(
	BDMA_Handle              hDma,
	BDMA_Mem_Handle *        phMemDma );

/***************************************************************************
Summary:
	Set the swap mode.

Description:
	This API function is replaced by BDMA_Mem_SetByteSwapMode and will be 
	obsolete in the future. New code should use the new one and stop using 
	this API function.

	During a DMA data transfer, Memory DMA engines could be programmed to 
	perform translation between little endian and big endian with either 
	byte aligned swap or short aligned swap. This function sets the swap
	mode.

	The sub-module maintains a current state of swap mode. Each setting 
	modifies the current state and is applied to the future calls to
	BDMA_Mem_Tran_SetDmaBlockInfo(_isr), unless it is reset again. This 
	means that in a transfer each block could have different swap mode.

Input:
	hMemDma - Sub-module handle to the memory DMA engine to modify.
	eSwapMode - Swap mode enum

Output:

Returns:
	BERR_SUCCESS - Swap mode is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr	
****************************************************************************/
BERR_Code BDMA_Mem_SetSwapMode(
	BDMA_Mem_Handle          hMemDma,
	BDMA_SwapMode            eSwapMode );

/***************************************************************************
Summary:
	Set encryption / decryption.

Description:
	This API function is replaced by BDMA_Mem_Tran_SetCrypto and will be 
	obsolete in the future. New code should use the new one and stop using 
	this API function.

	During a DMA data transfer, Memory DMA engines could be configured to
	perform encryption or decryption. This API function is used to set the 
	crypto.

	In the case that eCryptMode is BDMA_CryptMode_eNoCrypt, the crypto HW 
	module is disabled and the setting to ulKeySlot has no effect.

	If eCryptMode has other valid value, the crypto HW is enabled and 
	eCryptMode specifies how the crypto treats the transport packet 
	boundaries (refer to BDMA_CryptMode for detail). 

	Further, when scatter-gather SCRAM operation is enabled by bSgEnable, the 
	dma engine will treat the data stream section of a list of dma blocks as a 
	whole for encrypt or decrypt purpose. The stream section starts from the 
	beginning of the block marked as scatter-gather start point, and ends at 
	the end of the block marked as scatter-gather end point. bSgEnable can not 
	change between the scatter-gather start point and end point.

	The rest of the crypto configuration, such as what crypto operation 
	(either encryption or decryption) to perform, what crypto algorithm and 
	what key value to use during crypto operation, was programmed into a key 
	slot with the security module. ulKeySlot selects the key slot to use for
	the crypto. 

	There are 16 key slots in a 7401 chip for the moment. The value (between 
	0 and 15) of ulKeySlot passed to this function must be the same one used 
	in the security module when the key slot was programmed.

	The sub-module maintains a current state of crypt mode and key selection. 
	Each setting modifies the current state and is applied to the future 
	calls to BDMA_Mem_Tran_SetDmaBlockInfo(_isr), unless it is reset again. 
	This implies that in a transfer each block could use different crypto key.

	Application software is responsible to program the selected key slot with
	the security module. If the key slot is not configured well, the dma result 
	is undetermined.

Input:
	hMemDma - Sub-module handle to the memory DMA engine to modify.
	BDMA_CryptMode - Crypto mode. If it is BDMA_CryptMode_eNoCrypt, crypto is
	disabled. Otherwise it specifies how transport packet boundary is treated.
	ulKeySlot - Key slot (between 0 and 15) selected for the crypto.
	bSgEnable - Enable scatter-gather SCRAM operation.

Output:

Returns:
	BERR_SUCCESS - crypto setting is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetSgStartEnd, BDMA_Mem_Tran_SetSgStartEnd_isr,
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr	
****************************************************************************/
BERR_Code BDMA_Mem_SetCrypto(
	BDMA_Mem_Handle          hMemDma,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot,
	bool                     bSgEnable);

/***************************************************************************
Summary:
	Set encryption / decryption.

Description:
	This API function is replaced by BDMA_Mem_Tran_SetCrypto and will be 
	obsolete in the future. New code should use the new one and stop using 
	this API function.

Input:
	hMemDma - Sub-module handle to the memory DMA engine to modify.
	BDMA_CryptMode - Crypto mode. If it is BDMA_CryptMode_eNoCrypt, crypto is
	disabled. Otherwise it specifies how transport packet boundary is treated.
	ulKeySlot - Key slot (between 0 and 15) selected for the crypto.

Output:

Returns:
	BERR_SUCCESS - crypto setting is set successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_SetDmaBlockInfo, BDMA_Mem_Tran_SetDmaBlockInfo_isr	
****************************************************************************/
BERR_Code BDMA_Mem_SetCrypt(
	BDMA_Mem_Handle          hMemDma,
	BDMA_CryptMode           eCryptMode,
	uint32_t                 ulKeySlot );

/***************************************************************************
Summary:
	Query the max number of blocks allowed in a transfer.

Description:
	This function is obsolete.

	User can transfer a list of contiguous data blocks in one transfer. This
	API function is used to query the max number of blocks allowed in one
	transfer.

Input:
	hMemDma - Sub-module handle to the memory DMA engine to perform DMA
	transfers.

Output:
	pulMaxNumBlocks - Max number of blocks allowed in one transfer by the
	memory DMA engine.

Returns:
	BERR_SUCCESS - Got the max number successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create, BDMA_Mem_Tran_Create_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_GetMaxNumBlocks(
	BDMA_Mem_Handle          hMemDma,
	uint32_t *               pulMaxNumBlocks );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This API function is replaced by BDMA_Mem_Tran_Create2 and will be 
	obsolete in the future. New code should use the new one and stop using 
	this API function.

	In order to perform DMA transfer with memory DMA engine, user needs to 
	first create a transfer handle, then to use it to set each block info, 
	to start the DMA transfer, and finally to get the transfer status. 
	This API function is used to create the transfer handle.

	BDMA_Mem_Tran_Create takes an input parameter ulNumBlocks, to
	specify the number of contiguous data blocks this transfer is to
	transfer. The max number of blocks allowed could be queried with 
	BDMA_Mem_Tran_GetMaxNumBlocks. It is likely close to 128.

	A transfer handle could be reused after it is started and finished, and a
	transfer handle has to be explicitly destroyed when it is no longer 
	needed.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hMemDma - Sub-module handle to the memory DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Number of blocks in this transfer.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_GetMaxNumBlocks
	BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_Start, BDMA_Mem_Tran_StartAndCallBack
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Destroy, BDMA_Mem_Tran_Reset
	BDMA_Mem_Tran_Create_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Create(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	BDMA_Mem_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Start a DMA transfer.

Description:
	This API function is replaced by BDMA_Mem_Tran_StartDma and will be 
	obsolete in the future. New code should use the new one and stop using 
	this API function.

	In order to perform a DMA transfer, user needs to first create a transfer 
	handle, then to use it to set blocks info, to start the DMA transfer, and
	finally to get the transfer status. This API function is used to start a
	DMA transfer.

	As starting a transfer with this API function, user needs to implement a 
	status checking and sleeping loop, to wait for the completion of the
	DMA transfer, and then to process the "done" info.
	
	BDMA_Mem_Tran_Start takes, as an input parameter, hTran to specify the 
	transfer to start. 

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BDMA_ERR_ENGINE_BUSY - The engine is busy.

See Also:
	BDMA_Mem_Tran_Create, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_StartAndCallBack
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Start(
	BDMA_Mem_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is replaced by BDMA_Mem_Tran_StartDmaAndCallBack and 
	will be obsolete in the future. New code should use the new one and stop 
	using this API function.

	In order to perform a DMA transfer, user first creates a transfer handle,
	then uses it to set blocks info, to start the DMA transfer, and finally 
	to get the transfer status. This API function is used to start a DMA 
	transfer, and to register a call back function to be called when the 
	transfer finishes.

	It takes, as input parameters, hTran to specify the transfer to start,
	a call back function pointer, and a user data buffer pointer. Then the 
	call back function will be called by the "DMA done" interrupt handler
	of the module, when the transfer finishes.

	The call back function is ideally coded by the user. It should take three 
	parameters. One is the "user data buffer pointer", passed to this API
	function. It is for user to pass user defined data structure to the call 
	back function, in order to process the "done info". Another two are 
	transfer handle hTran and eStatus, which are provided by the DMA module 
	and are used to specify which transfer just finished and its status. 
	Notice that it is possible to queue several transfers in memory DMA 
	engine.

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Start
	BDMA_Mem_Tran_StartAndCallBack_isr
****************************************************************************/
BERR_Code BDMA_Mem_Tran_StartAndCallBack(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_Mem_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This API function is replaced by BDMA_Mem_Tran_Create2_isr and will be 
	obsolete in the future. New code should use the new one and stop using 
	this API function.

	This is the "_isr" version of BDMA_Mem_Tran_Create. It is used in 
	interrupt handler or critical session. For more description refer to
	BDMA_Mem_Tran_Create. 

Input:
	hMemDma - Sub-module handle to the memory DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Number of blocks in this transfer.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_GetMaxNumBlocks
	BDMA_Mem_Tran_SetDmaBlockInfo_isr
	BDMA_Mem_Tran_StartAndCallBack_isr
	BDMA_Mem_Tran_GetStatus_isr
	BDMA_Mem_Tran_Destroy_isr, BDMA_Mem_Tran_Reset_isr
	BDMA_Mem_Tran_Create
****************************************************************************/
BERR_Code BDMA_Mem_Tran_Create_isr(
	BDMA_Mem_Handle          hMemDma,
	uint32_t                 ulNumBlocks,
	BDMA_Mem_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is replaced by BDMA_Mem_Tran_StartDmaAndCallBack_isr and 
	will be obsolete in the future. New code should use the new one and stop 
	using this API function.

	This is the "_isr" version of BDMA_Mem_Tran_StartAndCallBack. It is used 
	in interrupt handler or critical session. For more description refer to 
	BDMA_Mem_Tran_StartAndCallBack. 

Input:
	hTran - The transfer handle to start.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create_isr, BDMA_Mem_Tran_SetDmaBlockInfo_isr
	BDMA_Mem_Tran_GetStatus_isr
	BDMA_Mem_Tran_StartAndCallBack
****************************************************************************/
BERR_Code BDMA_Mem_Tran_StartAndCallBack_isr(
	BDMA_Mem_Tran_Handle     hTran,
	BDMA_Mem_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This function is obsolete.

	Calling this function is the same as calling BDMA_Mem_Tran_Reset(), 
	BDMA_Mem_Tran_SetDmaBlockInfo(), and BDMA_Mem_Tran_StartAndCallBack()
	in one call. The callback parameters can be set to null if there is 
	no callback function.

Input:
	hTran - The transfer handle to start.
	ulBlockId - The block index in the block list of this transfer, it starts
	from 0.
	ulDstBusAddr - Destination address in memory bus address space.
	ulSrcBusAddr - Source address in memory bus address space.
	ulBlockSize - This block size in bytes.
	bCryptInit - Whether initialize the encryption / decryption or continue
	the encryption / decryption from previous block.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Mem_Tran_Create, BDMA_Mem_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_Start
	BDMA_Mem_Tran_StartAndCallBack
****************************************************************************/
BERR_Code BDMA_Mem_Transfer(
	BDMA_Mem_Tran_Handle     hTran,
	uint32_t                 ulBlockId,
	uint32_t                 ulDstBusAddr,
	uint32_t                 ulSrcBusAddr,
	uint32_t                 ulBlockSize,
	bool                     bCryptInit,
	BDMA_Mem_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
 * Obsolete PCI Dma API: 
 **************************************************************************/

/***************************************************************************
Summary:
	Creates a sub-module handle to PCI DMA engine.

Description:
	This API function is replaced by BDMA_Pci_Create2 and will be obsolete
	in the future. New code should use the new one and stop using this API 
	function.

	Once created, the handle to the unique PCI DMA engine is required 
	by PCI DMA sub-module functions in order to configure the DMA engine
	and perform DMA transfer. 

	If the sub-module handle has already been created with the same hDma,
	error is returned. This function also initializes the sub-module to 
	default configure.

	Refer to BDMA_Pci_Handle description for detailed usage info.

Input:
	hDma - Handle to the BDMA module.

Output:
	phPciDma - The created PCI DMA sub-module handle. If failure 
	occurred phPciDma will holds NULL.

Returns:
	BERR_INVALID_PARAMETER - Invalid function parameters.
	BERR_SUCCESS - Successfully created the handle.

See Also:
	BDMA_Pci_Destroy
**************************************************************************/
BERR_Code BDMA_Pci_Create(
	BDMA_Handle              hDma,
	BDMA_Pci_Handle *        phPciDma );

/***************************************************************************
Summary:
	Query the max number of blocks allowed in a transfer.

Description:
	This API function is obsolete.

	User can transfer a list of contiguous data blocks in one transfer. This
	API function is used to query the max number of blocks allowed in one
	transfer.

Input:
	hPciDma - Sub-module handle to the PCI DMA engine to perform DMA
	transfers.

Output:
	pulMaxNumBlocks - Max number of blocks allowed in one transfer by the
	PCI DMA engine.

Returns:
	BERR_SUCCESS - Got the max number successfully.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create, BDMA_Pci_Tran_Create_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_GetMaxNumBlocks(
	BDMA_Pci_Handle          hPciDma,
	uint32_t *               pulMaxNumBlocks );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This API function is replaced by BDMA_Pci_Tran_Create2 and will be 
	obsolete in the future. New code should use the new one and stop 
	using this API function.

	In order to perform DMA transfer with PCI DMA engine, user needs to 
	first create a transfer handle, then to use it to set each block info, 
	to start the DMA transfer, and finally to get the transfer status. 
	This API function is used to create the transfer handle.

	BDMA_Pci_Tran_Create takes an input parameter ulNumBlocks, to
	specify the number of contiguous data blocks this transfer is to
	transfer. The max number of blocks allowed could be queried with 
	BDMA_Pci_Tran_GetMaxNumBlocks. It is likely close to 128.

	A transfer handle could be reused after it is started and finished, and
	a transfer handle has to be explicitly destroyed when it is no longer 
	needed.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hPciDma - Sub-module handle to the PCI DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Number of blocks in this transfer.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_GetMaxNumBlocks
	BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_Start, BDMA_Pci_Tran_StartAndCallBack
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_Tran_Destroy, BDMA_Pci_Tran_Reset
	BDMA_Pci_Tran_Create_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Create(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	BDMA_Pci_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Start a DMA transfer.

Description:
	This API function is replaced by BDMA_Pci_Tran_StartDma and will be 
	obsolete in the future. New code should use the new one and stop 
	using this API function.

	In order to perform a DMA transfer, user needs to first create a transfer 
	handle, then to use it to set each block info, to start the DMA transfer,
	and finally to get the transfer status. This API function is used to start 
	a DMA transfer.

	As starting a transfer with this API function, user needs to implement a 
	status checking and sleeping loop, to wait for the completion of the
	DMA transfer, and then to process the "done" info.

	BDMA_Pci_Tran_Start takes, as an input parameter, hTran to specify the 
	transfer to start. 

	The current state of max burst size is used during the transfer.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BDMA_ERR_ENGINE_BUSY - The engine is busy.

See Also:
	BDMA_Pci_Tran_Create, BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_SetMaxBurstSize
	BDMA_Pci_Tran_StartAndCallBack
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Start(
	BDMA_Pci_Tran_Handle     hTran );

/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is replaced by BDMA_Pci_Tran_StartDmaAndCallBack 
	and will be obsolete in the future. New code should use the new one
	and stop using this API function.

	In order to perform a DMA transfer, user first creates a transfer handle,
	then uses it to set block info for each block, to start the DMA transfer, 
	and finally to get the transfer status. This API function is used to start 
	a DMA transfer, and to register a call back function to be called when 
	the transfer finishes.

	It takes, as input parameters, hTran to specify the transfer to start,
	a call back function pointer, and a user data buffer pointer. Then the 
	call back function will be called by the "DMA done" interrupt handler
	of the module, when the transfer finishes.

	As asynchronous call back is used, it is possible to dynamically link 
	several tran requests in a DMA engine, some linked tran's done call back 
	might be lost. However, the last one is guaranteed, and it also indicates 
	that all trans before this one have also finished.

	The call back function is ideally coded by the user. It should take three 
	parameters. One is the "user data buffer pointer", passed to this API
	function. It is for user to pass user defined data structure to the call 
	back function, in order to process the "done info". Another two are 
	transfer handle hTran and eStatus, which are provided by the DMA module 
	and are used to specify which transfer just finished and its status. 
	Notice that as asynchronous call back is used, it is possible to queue 
	several transfers in PCI DMA engine.

	The current state of max burst size is used during the transfer.

	Please refer to the DMA module overview and BDMA_Pci_Handle for more 
	info on the usage of the PCI DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create, BDMA_Pci_Tran_SetBlockInfo
	BDMA_Pci_Tran_GetStatus
	BDMA_Pci_SetMaxBurstSize
	BDMA_Pci_Tran_Start
	BDMA_Pci_Tran_StartAndCallBack_isr
****************************************************************************/
BERR_Code BDMA_Pci_Tran_StartAndCallBack(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_Pci_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );

/***************************************************************************
Summary:
	Create a transfer handle.

Description:
	This API function is replaced by BDMA_Pci_Tran_Create2_isr and will 
	be obsolete in the future. New code should use the new one and stop 
	using this API function.

	This is the "_isr" version of BDMA_Pci_Tran_Create. It is used in 
	interrupt handler or critical session. For more description refer to
	BDMA_Pci_Tran_Create. 

Input:
	hPciDma - Sub-module handle to the PCI DMA engine to perform the DMA
	transfer.
	ulNumBlocks - Number of blocks in this transfer.

Output:
	phTran - The transfer handle created.

Returns:
	BERR_SUCCESS - The transfer handle is created successfully.
	BDMA_ERR_SIZE_OUT_RANGE - ulNumBlocks is 0 or bigger than the allowed.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_GetMaxNumBlocks
	BDMA_Pci_Tran_SetBlockInfo_isr
	BDMA_Pci_Tran_StartAndCallBack_isr
	BDMA_Pci_Tran_GetStatus_isr
	BDMA_Pci_Tran_Destroy_isr, BDMA_Pci_Tran_Reset_isr
	BDMA_Pci_Tran_Create
****************************************************************************/
BERR_Code BDMA_Pci_Tran_Create_isr(
	BDMA_Pci_Handle          hPciDma,
	uint32_t                 ulNumBlocks,
	BDMA_Pci_Tran_Handle *   phTran );

/***************************************************************************
Summary:
	Start a DMA transfer and register a call back function.

Description:
	This API function is replaced by BDMA_Pci_Tran_StartDmaAndCallBack_isr 
	and will be obsolete in the future. New code should use the new one 
	and stop using this API function.

	This is the "_isr" version of BDMA_Pci_Tran_StartAndCallBack. It is used 
	in interrupt handler or critical session. For more description refer to 
	BDMA_Pci_Tran_StartAndCallBack. 

Input:
	hTran - The transfer handle to start.
	pDoneCallBack_isr - Call back function used when the transfer finishes.
	pUserCallBackParam - User's data passed to the call back function.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.

See Also:
	BDMA_Pci_Tran_Create_isr, BDMA_Pci_Tran_SetBlockInfo_isr
	BDMA_Pci_Tran_GetStatus_isr
	BDMA_Pci_Tran_StartAndCallBack
****************************************************************************/
BERR_Code BDMA_Pci_Tran_StartAndCallBack_isr(
	BDMA_Pci_Tran_Handle     hTran,
	BDMA_Pci_CallbackFunc    pDoneCallBack_isr,
	void *                   pUserCallBackParam );

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

/***************************************************************************
Summary:
	Start a DMA Sharf transfer.

Description:
	In order to perform a DMA transfer, user needs to first create a transfer 
	handle, then to use it to set blocks info, to start the DMA transfer, and
	finally to get the transfer status. This API function is used to start a
	DMA transfer.

	As starting a transfer with this API function, user needs to implement a 
	status checking and sleeping loop, to wait for the completion of the
	DMA transfer, and then to process the "done" info.
	
	BDMA_Mem_Tran_Start takes, as an input parameter, hTran to specify the 
	transfer to start. 

	Please refer to the DMA module overview and BDMA_Mem_Handle for more 
	info on the usage of the memory DMA sub-module API functions.

Input:
	hTran - The transfer handle to start.

Output:

Returns:
	BERR_SUCCESS - Transfer starting request is accepted.
	BERR_INVALID_PARAMETER - One of the input parameters is invalid.
	BDMA_ERR_BLOCK_INFO_NOT_SET - A block info has not been set.
	BDMA_ERR_ENGINE_BUSY - The engine is busy.

See Also:
	BDMA_Mem_Tran_Create, BDMA_Mem_Sharf_Tran_SetDmaBlockInfo
	BDMA_Mem_Tran_GetStatus
	BDMA_Mem_Tran_StartAndCallBack
****************************************************************************/

BERR_Code BDMA_Mem_Sharf_Tran_Start(
	BDMA_Mem_Tran_Handle     hTran );
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BDMA_H__ */

/* end of file */
