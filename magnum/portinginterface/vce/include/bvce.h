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

/**** Module Overview ****
 *
 * The VCE (Video Coding Engine) API is a library used to control the ViCE2 encoder on integrated chipsets.
 * The VCP PIs responsibilities include:
 *    - Loading and booting the ViCE2 FW
 *    - Managing memory allocation
 *    - Configuring channels
 *    - Providing error handling hooks
 *       - Watchdog and Error interrupt
 *       - Debug logging
 *       - Status Info
 *    - Queueing user data (CC)
 *
 * The VCE PI is not involved in any real-time aspect of the video encoding.  It's role is to setup
 * and configure the encoder, and then the encoder FW takes care of reading the source pictures.
 * After encoding starts, the VCE PI is involved in error handling (Watchdog and Error interrupts)
 * and reconfiguring the encoder based on the app's requirements.
 *
 * Each VICE2 hardware device is represented by a VCE Handle (BVCE_Handle).
 * Each VICE2 hardware device supports one or more encode channels represented by a VCE Channel (BVCE_Channel_Handle).
 *
 * -- FW Boot/Loading --
 * The VCE PI supports loading and booting the FW for all the ARCs in the VICE hardware.  The VCE PI also supports
 * loading of secure authenticated firmware from both an internal or external source.
 *
 * -- Memory Management --
 * To adhere to magnum PI coding conventions, all memory required for any of the encode channels is allocated up-front
 * in the BVCE_Open() call.  Subsequent BVCE_Channel_Open() calls sub-allocate memory as needed.
 *
 * There are 4 distinct memory regions that can be specified by the app:
 *    1) firmware memory - this is where the the ARC FW code/data is loaded
 *    2) system memory - this is where any data structures (e.g. user data) are allocated
 *    3) picture memory - this is where the FW picture buffers are allocated.  The separate memory is
 *       needed to support non-UMA systems where picture buffers could be in a separate physical memory.
 *    4) secure memory - this is where any data that needs to be secure (bin and Output buffers)
 *       would be allocated.
 *
 *  It is important to note that the heaps for the firmware, context, picture, and secure memory could all be the same (e.g. in
 *  a non-secure UMA system).
 *
 * -- Channel Configuration --
 * The VCE PI will enforce validity of settings such as combinations that are allowed (e.g. protocol + profile) and also what is
 * allowed to be changed dynamically (e.g. frame rate, bit rate) vs what is not allowed to be changed dynamicaly (e.g. protocol).
 * The VCE PI will remember the latest encoder configuration state to facilitate a clean restore during a watchdog reset and/or
 * context switch.
 *
 * -- User Data --
 * The VCE PI allows the app to queue user data packets to be inserted into the stream.
 *
 * -- Debug --
 * The VCE PI provides status and debug log information for the encoder.  Any of the ARCs' UART output can be
 * read by the host.  Also, debug commands can be sent to any of the ARCs.  The app can also query status and
 * error information from the encoders at anytime.
 *
 * -- Power Management --
 *  *** TODO ***
 */

#ifndef BVCE_H_
#define BVCE_H_

/* basemodules includes */
#include "bmma.h"
#include "bint.h"
#include "btmr.h"

/* commonutils includes */
#include "bavc.h"
#include "bafl.h" /* ARC firmware ELF Loader */
#include "budp_vce.h"
#include "bbox.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/******************************/
/* VCE Device Level Functions */
/******************************/

typedef struct BVCE_P_Context *BVCE_Handle;

/* BVCE_MemoryConfig - The memory configuration is determined by the application up-front based on the expected usage
 * modes and corresponding FW memory requirements.  The FW will provide a memory table.  This memory
 * is allocated during BVCE_Open() and is sub-allocated as needed in BVCE_Channel_Open().  The worst case memory requirements
 * needs to be determined here.
 *
 * The memory is allocated from the heaps as follows:
 *    - Picture Heap
 *       - Uncompressed picture buffers
 *    - Secure Heap
 *       - CABAC CMD/BIN buffer
 *
 * Note: VCE PI will coalesce memory sizes as needed depending on the actual heaps that are provided.  For maximum
 * application flexibility, it is recommended that the app always specify separate picture and secure heap
 * sizes and then specify the heaps that are required.  E.g. if only a system heap is specified (default), then the
 * total memory allocated from the system heap would be (system + picture + secure).
 */
typedef struct BVCE_MemoryConfig
{
      size_t uiPictureMemSize; /* The total amount of picture heap memory that should be allocated */
      size_t uiSecureMemSize;  /* The total amount of secure heap memory that should be allocated */
      size_t uiGeneralMemSize; /* The total amount of general heap memory that should be allocated */
      size_t uiFirmwareMemSize; /* The total amount of firmware heap memory that should be allocated */
      size_t uiIndexMemSize; /* The total amount of index (ITB) heap memory that should be allocated */
      size_t uiDataMemSize; /* The total amount of data (CDB) heap memory that should be allocated */
} BVCE_MemoryConfig;

/* BVCE_ArcInstance - enum allowing selection of a specific ARC in the ViCE encoder core
 * E.g. for secure boot, debug log processing, debug commands, etc. */
typedef enum BVCE_ArcInstance
{
      BVCE_ArcInstance_ePicArc, /* The Picture ARC (the master) */
      BVCE_ArcInstance_eMBArc,  /* The MacroBlock ARC  (the slave) */

      BVCE_ArcInstance_eMax
} BVCE_ArcInstance;

/* BVCE_Debug_BufferingMode - controls how the debug buffer is written by the FW */
typedef enum BVCE_Debug_BufferingMode
{
      BVCE_Debug_BufferingMode_eOverwriteOldData, /* FW continually writes to debug buffer
                                                   * and stops when debug buffer is being
                                                   * read.  This mode is useful for normal
                                                   * operation where the application does not
                                                   * need to periodically debug buffer but the
                                                   * latest debug buffer output is available to
                                                   * read to debug errors.  E.g. During a watchdog.
                                                   * the app can read the debug log to see if there
                                                   * is any information that would help diagnose the
                                                   * cause of the watchdog.
                                                   */
      BVCE_Debug_BufferingMode_eDiscardNewData, /* FW stops writing debug buffer when full.
                                                 * When full, the debug buffer output is dropped.
                                                 * This mode is useful for implementing a virtual
                                                 * UART when the physical UARTs are not present.
                                                 * The app needs to periodically read the debug log
                                                 * buffer to prevent it from getting full.
                                                 */

      BVCE_Debug_BufferingMode_eMax
} BVCE_Debug_BufferingMode;

typedef struct BVCE_PlatformSettings
{
   BBOX_Handle hBox;
   unsigned uiInstance;
} BVCE_PlatformSettings;

void BVCE_GetDefaultPlatformSettings(
   BVCE_PlatformSettings *pstPlatformSettings
   );

typedef struct BVCE_OpenSettings
{
      uint32_t uiSignature;

      unsigned uiInstance; /* VCE 0 or VCE 1 */

      /********************/
      /* Memory and Heaps */
      /********************/

      /* VCE Heap Usage
       *                                                              [Heap Usage Order]
       * [Heaps]                                    [CPU Accessible?] [Firmware] [Pictures] [CABAC] [Debug Log] [User Data] [CDB] [ITB] [Descriptors]
       * BVCE_Open(hMem) (aka hSystemMem)           Y                  2          2          3       1           1           3     3     2
       * BVCE_OpenSettings.hFirmwareMem[]           Y                  1          -          -       -           -           -     -     -
       * BVCE_OpenSettings.hPictureMem              N                  -          1          2       -           -           -     -     -
       * BVCE_OpenSettings.hSecureMem               N                  -          -          1       -           -           2     2     -
       * BVCE_Output_AllocBuffersSettings.hITBMem   Y/N (1)            -          -          -       -           -           -     1     -
       * BVCE_Output_AllocBuffersSettings.hCDBMem   Y/N (2)            -          -          -       -           -           1     -     -
       * BVCE_Output_OpenSettings.hDescriptorMem    Y                  -          -          -       -           -           -     -     1
       *
       * Notes:
       *    (1) ITB memory must be CPU accessible if BVCE_Output_GetBufferDescritors() will be called.
       *    (2) CDB memory must be CPU accessible if BVCE_Output_Settings.bEnableDataUnitDetection=true
       */

      BMMA_Heap_Handle hFirmwareMem[BVCE_ArcInstance_eMax]; /* If NULL, system memory handle (hMem) used.
                                                             * If non-NULL, heap should be created with "system bookkeeping"
                                                             * to ensure firmware is loaded at the expected location.
                                                             * One firmware memory heap exists for each ARC.  This is needed to
                                                             * support loading of ARC FW to specific memory regions (e.g. for
                                                             * authenticated firmware support).
                                                             */

      BMMA_Heap_Handle hPictureMem;  /* If NULL, system memory handle (hMem) used.
                                      * If non-NULL, all picture memory allocated from this heap
                                      */

      BMMA_Heap_Handle hSecureMem;   /* If NULL, system memory handle (hMem) used.
                                      * If non-NULL, all secure memory allocated from this heap
                                      */

      BVCE_MemoryConfig stMemoryConfig; /* The total amount of memory that needs to be allocated for *all* channels
                                         * E.g. Let's say the application intends to open 2 simultaneous encode
                                         * channels: 1 for SD MPEG2 and 1 for HD VC1.
                                         * The app would need to look up the memory requirements for each configuration
                                         * and combine them.
                                         *    - BVCE_MemoryConfig.uiPictureMemSize = PictureMemSize(SD MPEG2) + PictureMemSize(HD VC1)
                                         *    - BVCE_MemoryConfig.uiSecureMemSize = SecureMemSize(SD MPEG2) + SecureMemSize(HD VC1)
                                         */

      /*********************/
      /* Firmware and Boot */
      /*********************/

      /* Interface to access firmware image. This interface must be
       * implemented and the function pointers must be stored here.
       */
      const BIMG_Interface *pImgInterface;

      /* Context for the Image Interface. This is also provided by
       * the implementation of the Image interface
       */
      const void* const *pImgContext;

      /* Add Authenticated image support */

      /* Pointer to the ARC boot callback function. If non-NULL,
       * PI will call this function after the firmware has been
       * loaded into memory instead of booting the ARC. If this
       * function is NULL, then the PI will boot the ARC
       * normally. This function should return BERR_SUCCESS if
       * successful.
       */
      BERR_Code (*pARCBootCallback)(void* pPrivateData,
                                    const BAFL_BootInfo *pstBootInfo);

      /* Pointer to ARC boot callback private data that is passed
       * back into the callback.  Can be used to store any
       * information necessary for the application to boot the
       * core.
       */
      void *pARCBootCallbackData;

      /* If pARCBootCallback is non-NULL, then by default, the
       * VCE PI does NOT explicitly boot the core. However, if
       * bARCBootEnable is true, then the VCE PI will boot the
       * core AFTER the pARCBootCallback has been executed.
       * Note: If pARCBootCallback is NULL, VCE PI always boots
       * the core.
       */
      bool bARCBootEnable;

      /*********/
      /* Debug */
      /*********/
      BTMR_Handle hTimer; /* If non-NULL, may be used to gather performance data */

      /* Size of debug logging buffer */
      size_t uiDebugLogBufferSize[BVCE_ArcInstance_eMax];
      BVCE_Debug_BufferingMode eDebugLogBufferMode[BVCE_ArcInstance_eMax];

      bool bVerificationMode;
      bool bA2NPictureDrop;

      BBOX_Handle hBox;
} BVCE_OpenSettings;

void
BVCE_GetDefaultOpenSettings(
         const BVCE_PlatformSettings *pstPlatformSettings,
         BVCE_OpenSettings  *pstOpenSettings /* [out] Default VCE settings */
         );

BERR_Code
BVCE_Open(
         BVCE_Handle *phVce, /* [out] VCE Device handle returned */
         BCHP_Handle hChp,   /* [in] Chip handle */
         BREG_Handle hReg,   /* [in] Register handle */
         BMMA_Heap_Handle hMem,   /* [in] System Memory handle */
         BINT_Handle hInt,   /* [in] Interrupt handle */
         const BVCE_OpenSettings *pOpenSettings /* [in] VCE Device Open settings */
         );

BERR_Code
BVCE_Close(
         BVCE_Handle hVce
         );

BERR_Code
BVCE_GetTotalChannels(
         BVCE_Handle hVce,
         unsigned *puiTotalChannels
         );

typedef struct BVCE_VersionInfo
{
   unsigned uiFirmwareVersion;
   unsigned uiFirmwareApiVersion;
   unsigned uiBvn2ViceApiVersion;
} BVCE_VersionInfo;

BERR_Code
BVCE_GetVersionInfo(
   BVCE_Handle hVce,
   BVCE_VersionInfo *pstVersionInfo
   );

/* BVCE_Debug_ReadBuffer - reads the debug buffer log for the specified ARC into the
 * specified buffer.  If the debug buffer is configured for continuous mode, the
 * logging is temporarily disabled while the log is read out.
 */
BERR_Code
BVCE_Debug_ReadBuffer(
         BVCE_Handle hVce,
         BVCE_ArcInstance eARCInstance,
         char *szBuffer,   /* [in] pointer to buffer where log is copied to */
         size_t uiBufferSize,  /* [in] maximum number of bytes to copy to buffer */
         size_t *puiBytesRead  /* [out] number of bytes copied from debug log */
         );

/* BVCE_Debug_SendCommand - sends the debug command to the specified ARC */
BERR_Code
BVCE_Debug_SendCommand(
         BVCE_Handle hVce,
         BVCE_ArcInstance eARCInstance,
         char        *szCommand  /* [in] pointer to a double null terminated command string of debug uart commands */
         );

/* BVCE_Debug_DumpRegisters - dumps the relevant HW registers to the console to help debug VCE FW/HW issues */
void
BVCE_Debug_DumpRegisters(
        BVCE_Handle hVce
        );

typedef struct BVCE_Debug_FifoInfo
{
   size_t uiElementSize; /* Size of one element */
   BMMA_Block_Handle hBlock; /* Memory Block containing the VCE PI's internal BDBG_Fifo */
   unsigned uiOffset; /* Offset from start of memory block where BDBG_Fifo begins */
} BVCE_Debug_FifoInfo;

/* BVCE_Debug_GetLogMessageFifo - returns the internal BDBG_Fifo information.
 * The caller can then create one or more BDBG_Fifo readers via BDBG_Fifo_Create
 */
BERR_Code
BVCE_Debug_GetLogMessageFifo(
   BVCE_Handle hVce,
   BVCE_Debug_FifoInfo *pstDebugFifoInfo
   );

/* BVCE_ProcessWatchdog - reinitializes the encoder ARCs and restarts any channels
 * that were previously active.
 */
BERR_Code
BVCE_ProcessWatchdog(
         BVCE_Handle hVce
         );

/*********************************/
/* VCE Device Callback Functions */
/*********************************/

/* Device Callback - Watchdog
 *
 * Generated when the VICE2 watchdog timer has timed out.
 * The app can call BVCE_ProcessWatchdog to reinit and resume any previous encodes.
 * Alternatively, the app can call BCE_Close/BVCE_Open to do it's own error handling.
 */
typedef struct BVCE_WatchdogCallbackInfo
{
      unsigned uiUnused;
      /* TODO: Do we need any info here? */
} BVCE_WatchdogCallbackInfo;

typedef void (*BVCE_WatchdogCallbackHandler_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BVCE_WatchdogCallbackInfo *pInfo );

typedef struct BVCE_WatchdogCallbackSettings
{
      bool bEnable;

      BVCE_WatchdogCallbackHandler_isr fCallback;
      void *pPrivateContext;
      int32_t iPrivateParam;
} BVCE_WatchdogCallbackSettings;

typedef struct BVCE_CallbackSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BVCE_GetDefaultCallbackSettings() */

      BVCE_WatchdogCallbackSettings stWatchdog;
} BVCE_CallbackSettings;

void
BVCE_GetDefaultCallbackSettings(
         BVCE_CallbackSettings* pstCallbackSettings
         );

BERR_Code
BVCE_SetCallbackSettings(
         BVCE_Handle hVce,
         const BVCE_CallbackSettings* pstCallbackSettings
         );

BERR_Code
BVCE_GetCallbackSettings(
         BVCE_Handle hVce,
         BVCE_CallbackSettings* pstCallbackSettings
         );

/************************/
/* VCE Output Functions */
/************************/
/* BVCE_Output_XXX are DEPRECATED
 * Instead, set output parameters in BVCE_Channel_StartEncodeSettings.stOutput and
 * use BVCE_Channel_Output_XXX() function calls
*/
typedef struct BVCE_P_OutputBuffers *BVCE_OutputBuffers_Handle;

typedef struct BVCE_Output_AllocBuffersSettings
{
      uint32_t uiSignature;

      BAVC_CdbItbConfig stConfig;  /* [in] Size and alignment for ITB and CDB */
      BMMA_Heap_Handle hITBMem; /* [optional] If null, uses hSecureMem (if non-null) from VCE Handle. */
      BMMA_Heap_Handle hCDBMem; /* [optional] If null, uses hSecureMem (if non-null) from VCE Handle. */
} BVCE_Output_AllocBuffersSettings;

void
BVCE_Output_GetDefaultAllocBuffersSettings(
         const BBOX_Handle hBox,
         BVCE_Output_AllocBuffersSettings *pstOutputAllocBuffersSettings
   );

/* BVCE_Output_AllocBuffer - Allocates ITB/CDB data (in *addition* to any memory allocated in BVCE_Open) */
BERR_Code
BVCE_Output_AllocBuffers(
         BVCE_Handle hVce,
         BVCE_OutputBuffers_Handle *phVceOutputBuffers,
         const BVCE_Output_AllocBuffersSettings *pstOutputAllocBuffersSettings /* [in] VCE Output Alloc Buffer settings */
         );

/* Should only be called when the buffer is not attached to a Output context */
BERR_Code
BVCE_Output_FreeBuffers(
         BVCE_OutputBuffers_Handle hVceOutputBuffers
         );

typedef struct BVCE_Output_OpenSettings
{
      uint32_t uiSignature;

      unsigned uiInstance;

      bool bEnableDataUnitDetection; /* If TRUE: CDB will be parsed to detect data units and video buffer descriptors
                                      * returned by BVCE_Output_GetBufferDescriptors will indicate the location and type
                                      * of data unit.  E.g. For H.264, the start code for each NALU will generate a new
                                      * video buffer descriptor with the BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START
                                      * flag set.
                                      *
                                      * Note: If TRUE, it is required that VCE PI has full access to the CDB (e.g. it is not in
                                      * secure memory and not encrypted)
                                      *
                                      * Data Unit detection is needed for muxing certain protocols/container (e.g. H.264 in MP4)
                                      */
      uint32_t uiDescriptorQueueDepth; /* Default: 3192 Value of 0 reserved for later use */

      BMMA_Heap_Handle hDescriptorMem; /* [optional] Specifies heap used to allocate buffer and metadata descriptors.
                                        * If null, uses hMem from VCE Handle.
                                        */
} BVCE_Output_OpenSettings;

typedef struct BVCE_P_Output_Context *BVCE_Output_Handle;

void
BVCE_Output_GetDefaultOpenSettings(
         const BBOX_Handle hBox,
         BVCE_Output_OpenSettings *pstOutputOpenSettings
   );

BERR_Code
BVCE_Output_Open(
         BVCE_Handle hVce,
         BVCE_Output_Handle *phVceOutput,
         const BVCE_Output_OpenSettings *pstOutputOpenSettings /* [in] VCE Output settings */
         );

BERR_Code
BVCE_Output_Close(
         BVCE_Output_Handle hVceOutput
         );

/* BVCE_Output_SetBuffers -
 * Sets the output HW to use the specified output buffers.  The output HW pointers are initialized to point
 * to the beginning of the output buffers.
 * Cannot be called while actively used by an encode
 * After a stop/suspend, use caution when calling by ensuring the consumer of the output is finished
 */
BERR_Code
BVCE_Output_SetBuffers(
         BVCE_Output_Handle hVceOutput,
         const BVCE_OutputBuffers_Handle hVceOutputBuffers
         );

BERR_Code
BVCE_Output_Reset(
         BVCE_Output_Handle hVceOutput
         );

/* BVCE_Output_GetRegisters -
 * Returns the registers associated with the specified output hardware
 */
BERR_Code
BVCE_Output_GetRegisters(
         BVCE_Output_Handle hVceOutput,
         BAVC_VideoContextMap *pstVceOutputRegisters
         );

/* BVCE_Output_GetBufferDescriptors -
 * Returns video buffer descriptors for CDB content in the
 * BAVC_VideoBufferDescriptor array(s)
 */
BERR_Code
BVCE_Output_GetBufferDescriptors(
   BVCE_Output_Handle hVceOutput,
   const BAVC_VideoBufferDescriptor **astDescriptors0, /* Pointer to an array of descriptors. E.g. *astDescriptorsX[0] is the first descriptor. *astDescriptorsX may be set to NULL iff uiNumDescriptorsX=0. */
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor **astDescriptors1, /* Needed to handle FIFO wrap */
   size_t *puiNumDescriptors1
   );

/* BVCE_Output_ConsumeBufferDescriptors -
 * Reclaims the specified number of video buffer descriptors
 * The CDB read pointer is updated accordingly
 */
BERR_Code
BVCE_Output_ConsumeBufferDescriptors(
   BVCE_Output_Handle hVceOutput,
   size_t uiNumBufferDescriptors
   );

/* BVCE_Output_GetBufferStatus -
 * Returns the output buffer status (e.g. the base virtual address)
 */
BERR_Code
BVCE_Output_GetBufferStatus(
   BVCE_Output_Handle hVceOutput,
   BAVC_VideoBufferStatus *pBufferStatus
   );

/*******************************/
/* VCE Channel Level Functions */
/*******************************/

typedef struct BVCE_P_Channel_Context *BVCE_Channel_Handle;

typedef enum BVCE_MultiChannelMode
{
   BVCE_MultiChannelMode_eMulti, /* Multiple streams allowed at a time
                                  * Combined Resolution of active encodes CANNOT exceed Max Resolution
                                  * No Low Delay Support: BVCE_Channel_StartEncodeSettings.bPipelineLowDelayMode MUST BE false
                                  * Increased A2PDelay for ALL streams
                                  * DEPRECATED: Use eCustom and BVCE_Channel_OpenSettings.uiMaxNumChannels
                                  */

   BVCE_MultiChannelMode_eSingle, /* Only single stream allowed at a time
                                   * Low Delay Supported: BVCE_Channel_StartEncodeSettings.bPipelineLowDelayMode CAN be true
                                   * Max Resolution is supported
                                   * DEPRECATED: Use eCustom and BVCE_Channel_OpenSettings.uiMaxNumChannels
                                   */

   BVCE_MultiChannelMode_eMultiNRTOnly, /* Multiple NRT streams allowed at a time
                                         * NRT mode ONLY: BVCE_Channel_StartEncodeSettings.bNonRealTimeEncodeMode MUST BE true
                                         * Max Resolution is supported on EACH stream
                                         * DEPRECATED: Use eCustom and BVCE_Channel_OpenSettings.uiMaxNumChannels
                                         */

   BVCE_MultiChannelMode_eCustom, /* When set, the max number of simultaneous channels is specified in
                                   * BVCE_Channel_OpenSettings.uiMaxNumChannels */

   BVCE_MultiChannelMode_eMax
} BVCE_MultiChannelMode;

typedef struct BVCE_Channel_OpenSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BVCE_Channel_GetDefaultOpenSettings() */

      unsigned uiInstance; /* Channel instance */

      BVCE_MemoryConfig stMemoryConfig; /* The total amount of memory that needs to be sub-allocated for this channel */

      BVCE_MultiChannelMode eMultiChannelMode;  /* Specifies the expected usage for number and type of
                                                 * simultaneous channels on this device. */
      unsigned uiMaxNumChannels; /* If eMultiChannelMode = eCustom, specifies the maximum number of simultaneous channels
                                  * that will be operating simultaneously on this device.  A value of 0 indicates the max
                                  * channels supports by the device as determined by FW. */

      /* If hOutputHandle is NULL, then the app must specify the output settings in stOutput and
       * call BVCE_Channel_Output_XXX() functions instead of BVCE_Output_XXX().
       */
      struct
      {
         bool bAllocateOutput; /* Set to true if the output memory should be allocated in BVCE_Channel_Open
                                  and are using BVCE_Channel_Output_XXX() instead of using BVCE_Output_XXX() */

         BMMA_Heap_Handle hIndexMem; /* [optional] For ITB output. If null, uses hMem from VCE Handle. */
         BMMA_Heap_Handle hDataMem; /* [optional] For CDB output. If null, uses hSecureMem (if non-null) from VCE Handle. */

         bool bEnableDataUnitDetection; /* If TRUE: CDB will be parsed to detect data units and video buffer descriptors
                                         * returned by BVCE_Output_GetBufferDescriptors will indicate the location and type
                                         * of data unit.  E.g. For H.264, the start code for each NALU will generate a new
                                         * video buffer descriptor with the BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START
                                         * flag set.
                                         *
                                         * Note: If TRUE, it is required that VCE PI has full access to the CDB (e.g. it is not in
                                         * secure memory and not encrypted)
                                         *
                                         * Data Unit detection is needed for muxing certain protocols/container (e.g. H.264 in MP4)
                                         */
      } stOutput;
} BVCE_Channel_OpenSettings;

void
BVCE_Channel_GetDefaultOpenSettings(
         const BBOX_Handle hBox,
         BVCE_Channel_OpenSettings *pstChSettings
         );

BERR_Code
BVCE_Channel_Open(
         BVCE_Handle hVce,
         BVCE_Channel_Handle *phVceCh,
         const BVCE_Channel_OpenSettings *pstChOpenSettings /* [in] VCE Channel settings */
         );

BERR_Code
BVCE_Channel_Close(
         BVCE_Channel_Handle hVceCh
         );

/* Channel Callback - Error
 *
 * Generated if the channel channel has an error to report.
 */
typedef struct BVCE_Channel_EventCallbackInfo
{
      unsigned uiUnused;
      /* TODO: Do we need any info here? */
} BVCE_Channel_EventCallbackInfo;

typedef void (*BVCE_Channel_EventCallbackHandler_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         const BVCE_Channel_EventCallbackInfo *pInfo );

typedef struct BVCE_Channel_EventCallbackSettings
{
      bool bEnable;

      BVCE_Channel_EventCallbackHandler_isr fCallback;
      void *pPrivateContext;
      int32_t iPrivateParam;

      uint32_t uiErrorMask; /* 0 = all errors allowed.  See BVCE_CHANNEL_STATUS_FLAGS_ERROR_XXX */
      uint32_t uiEventMask; /* 0 = all events allowed (i.e. no events are masked).  See BVCE_CHANNEL_STATUS_FLAGS_EVENT_XXX  */
} BVCE_Channel_EventCallbackSettings;

typedef struct BVCE_Channel_DataReadyCallbackSettings
{
      bool bEnable;

      BVCE_Channel_EventCallbackHandler_isr fCallback;
      void *pPrivateContext;
      int32_t iPrivateParam;
} BVCE_Channel_DataReadyCallbackSettings;

typedef struct BVCE_Channel_CallbackSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BVCE_Channel_GetDefaultCallbackSettings() */

      BVCE_Channel_EventCallbackSettings stEvent;
      BVCE_Channel_DataReadyCallbackSettings stDataReady;
} BVCE_Channel_CallbackSettings;

void
BVCE_Channel_GetDefaultCallbackSettings(
         BVCE_Channel_CallbackSettings* pstCallbackSettings
         );

BERR_Code
BVCE_Channel_SetCallbackSettings(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_CallbackSettings* pstCallbackSettings
         );

BERR_Code
BVCE_Channel_GetCallbackSettings(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_CallbackSettings* pstCallbackSettings
         );

/* GOP Parameters */
typedef struct BVCE_GOPStructure
{
      bool bAllowNewGOPOnSceneChange; /* If true, the encoder will start a new GOP if it detects a scene change */

      unsigned uiDuration; /* If (uiDuration != 0):
                            *    If (bAllowOpenGOP == false): GOPLength = 1 + N*(1 + uiNumberOfBFrames)
                            *    If (bAllowOpenGOP == true) : GOPLength = (1 + N)*(1 + uiNumberOfBFrames)
                            *    The actual number of P frames (N) is determined by the FW based on the specified duration (in milliseconds).
                            *    The actual GOP duration will be accurate to at most within +/- (1 + uiNumberOfBFrames) of the specified duration.
                            *    Note: uiNumberOfPFrames is ignored if uiDuration is non-zero.
                            *
                            * If (uiDuration == 0):
                            *    If (bAllowOpenGOP == false): GOPLength = 1 + uiNumberOfPFrames*(1 + uiNumberOfBFrames)
                            *    If (bAllowOpenGOP == true) : GOPLength = (1 + uiNumberOfPFrames)*(1 + uiNumberOfBFrames)
                            */

      unsigned uiDurationRampUpFactor; /* If (uiDurationRampUpFactor != 0):
                                        *    The first uiDurationRampUpFactor GOPs will have a duration of uiDuration/uiDurationRampUpFactor each
                                        *    The next uiDurationRampUpFactor GOPs will have a duration of (2*uiDurationRampUpFactor-1) uiDurationRampUpFactor each
                                        *    After ramp-up, the uiTotalDuration = (2*uiDurationRampUpFactor)*uiDuration
                                        * Note: This field is ignored if uiDuration == 0
                                        */

      unsigned uiNumberOfPFrames; /* number of P frames between I frames. 0xFFFFFFFF indicated IP infinite mode
                                   * Note: This field is ignored if uiDuration != 0
                                   */

      /* The following are only relevant if uiNumberOfPFrames != 0 and uiNumberOfPFrames != 0xFFFFFFFF */
      unsigned uiNumberOfBFrames; /* number of B frames between I or P frames */

      /* The following are only relevant if uiNumberOfBFrames != 0 */
      bool bAllowOpenGOP;
} BVCE_GOPStructure;

typedef struct BVCE_BitRate
{
      /* Bit Rate (in bits/sec) where 1Mbps = 1000000 */
      unsigned uiMax;
      unsigned uiTarget; /* If "0", means CBR, and uiTarget=uiMax.  If non-zero, then VBR. */
} BVCE_BitRate;

typedef enum BVCE_EntropyCoding
{
   BVCE_EntropyCoding_eDefault,
   BVCE_EntropyCoding_eCAVLC,
   BVCE_EntropyCoding_eCABAC,

   /* Add new enums ABOVE this line */
   BVCE_EntropyCoding_eMax
} BVCE_EntropyCoding;

typedef struct BVCE_MemoryBandwidthSaving
{
   bool bSingleRefP; /* Force the encoder to use only one reference for P pictures. */
   bool bRequiredPatchesOnly; /* Force the encoder to use only the required patches (i.e. disable use of optional patches) */
} BVCE_MemoryBandwidthSaving;

/* Segment Mode Rate Control (RC) allows control over the total bits used (with some tolerance)
 * to encode a specified duration (segment).  It is useful for situations where the standard
 * HRD RC model doesn't apply (e.g. HLS streaming or other client "pull" scenarios).
 */
typedef struct BVCE_RateControl_Segment
{
   bool bEnable; /* If true, enables the Segment Mode Rate Control (RC) instead of the default HRD RC */
   unsigned uiDuration; /* duration of segment (in ms). The duration along with the frame rate
                         * is used to determine the maximum number of pictures within a segment.
                         */
   struct
   {
      struct
      {
         unsigned uiTolerance; /* percentage of tolerance allowed from the target bitrate */
         unsigned uiSlopeFactor; /* (internal dev use only) percentage used to adjust slope of tolerance */
      } stUpper, stLower; /* Specifies the upper and lower bounds for the bitrate tolerance relative to the target bitrate */
   } stTargetBitRatePercentage; /* Used to specify the bitrate tolerance in the segment */
} BVCE_RateControl_Segment;

typedef struct BVCE_RateControl_Hrd
{
   bool bDisableFrameDrop; /* This flag only applies to HRD mode rate control. When set, encoder will not drop pictures due to HRD model buffer underflow. */
} BVCE_RateControl_Hrd;

typedef struct BVCE_RateControl
{
   BVCE_RateControl_Hrd stHrdMode;
   BVCE_RateControl_Segment stSegmentMode;
} BVCE_RateControl;

typedef struct
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BVCE_Channel_GetDefaultStartEncodeSettings() */

      /* Encode Mode */
      bool bNonRealTimeEncodeMode;  /* If FALSE: Normal operation for encoding/transcoding a
                                     * video source in real time.
                                     *
                                     * If TRUE: aka As Fast As Possible (AFAP) Mode.
                                     * If VCE is not ready for a new picture
                                     * (e.g. encoder is busy, output CDB/ITB is full, etc),
                                     * it will put back pressure on BVN to prevent loss of
                                     * video frames */

      bool bPipelineLowDelayMode;   /* If FALSE: Output is written to CDB on frame boundaries.
                                      * The data for the first frame will not be written
                                     * to CDB until the entire frame has been encoded.
                                     *
                                     * NOTE: bNonRealTimeEncodeMode must be FALSE if this is TRUE
                                     *
                                     * If TRUE: Output is written to CDB on macroblock boundaries.
                                        * The data for the first frame will be written as soon
                                        * as the first macroblock has been encoded. When this
                                        * mode is enabled, the transmission of the encoded
                                        * frame can begin before the entire frame has finished
                                        * encoding.                                   *
                                        *
                                        * This mode is useful in cases where minimal latency is
                                        * needed (e.g. video conferencing, gaming, etc.).
                                        *
                                        * NOTE: I,P pictures only. B pictures are not allowed
                                        */

      bool bAdaptiveLowDelayMode;    /* If TRUE: Encoder will drop all incoming frames until the first
                                      * decoded frame from the new channel is seen.  The first frame will be
                                      * encoded with a low A2PDelay.  A2PDelay will automatically ramp to the
                                      * desired A2PDelay.
                                      */

      /* Protocol/Profile/Level */
      BAVC_VideoBufferInfo stProtocolInfo;

      BAVC_ScanType eInputType;

      BVCE_Output_Handle hOutputHandle; /* [DEPRECATED] Output Context Handle - HW + ITB/CDB to use for this encode. */

      unsigned uiStcIndex; /* Which STC broadcast the VCE encoder should use */

      /* Encoder Bounds - If the bounds are known and specified, encoder latency can be improved.
       * These bounds are for a single encode session.  E.g. if the output frame rate is known to to be
       * fixed at 30fps, then the output frame rate min/max can be set to 30.  */
      struct
      {
         /* Output Frame Rate -
          *    BVCE_Channel_EncodeSettings.stFrameRate.eFrameRate must be within eMin and eMax during encode */
         struct
         {
            BAVC_FrameRateCode eMin;
            BAVC_FrameRateCode eMax;
         } stFrameRate;

         /* Input Frame Rate -
          *    The input frame rate (from bvn) cannot go lower than eMin during the encode */
         struct
         {
            BAVC_FrameRateCode eMin;
         } stInputFrameRate;

         /* Picture Dimension -
          *    The input picture dimensions (from bvn) cannot go above stMax */
         struct
         {
            /* Size (in pixels) */
            struct
            {
               unsigned uiWidth;
               unsigned uiHeight;
            } stMax;
            struct
            {
               unsigned uiWidth;
               unsigned uiHeight;
            } stMaxInterlaced;
         } stDimensions;

         /* GOP Structure -
          *    The worst case GOP structure.  Delay increases with number of B frames */
         BVCE_GOPStructure stGOPStructure;

         struct
         {
            BVCE_BitRate stLargest; /* The largest bitrate intended to be used during the transcode. The default of 0 means the initial bitrate is the max */
         } stBitRate;
      } stBounds;

      unsigned uiRateBufferDelay; /* in ms.  Higher values indicate better quality but more latency.  0 indicates use encoder's defaults */

      unsigned uiNumParallelNRTEncodes;  /* uiNumParallelNRTEncodes indicates how many
                                            parallel encodes are expected.  0 indicates VCE
                                            defaults.  1 means normal NRT. 2 or
                                            more indicates FNRT. */

      BVCE_EntropyCoding eForceEntropyCoding; /* Force the type of entropy coding used.
                                               * Note: If the protocol/profile doesn't support the specified
                                               * entropy encoding, the default is used */

      BVCE_MemoryBandwidthSaving stMemoryBandwidthSaving; /* Allows fine tuning of memory bandwidth usage */

      BVCE_RateControl stRateControl;
} BVCE_Channel_StartEncodeSettings;


void
BVCE_Channel_GetDefaultStartEncodeSettings(
         const BBOX_Handle hBox,
         BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings
         );

/* BVCE_Channel_StartEncode - Configures the encoder and starts the encode process.
 *
 * Note: BVCE_Channel_SetEncodeSettings() should be called before this to set up the initial encode parameters
 */
BERR_Code
BVCE_Channel_StartEncode(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings
         );

/* BVCE_Channel_StopEncode - Stops the encode process.
 */
typedef enum BVCE_Channel_StopMode
{
   BVCE_Channel_StopMode_eNormal,    /* Default: Encoder will stop the encode cleanly.
                                      * Existing pictures will be finished.
                                      * An EOS will be appended to the ES stream.
                                      * The app MUST continue running the muxer thread
                                      * until the EOS before stopping the thread.
                                      */
   BVCE_Channel_StopMode_eImmediate, /* Encoder will stop as fast as possible.
                                      * Current pictures in flight may be dropped.
                                      * An EOS will be appended to the ES stream.
                                      *
                                      * The app MUST continue running the muxer thread
                                      * until the EOS before stopping the thread.
                                      */

   BVCE_Channel_StopMode_eAbort,     /* Encoder will stop as fast as possible.
                                      * Current pictures in flight may be dropped.
                                      * An EOS will NOT be appended to the ES stream.
                                      *
                                      * The app MUST END the the muxer thread
                                      * PRIOR to calling with the eAbort flag
                                      */

   BVCE_Channel_StopMode_eMax
} BVCE_Channel_StopMode;

typedef struct BVCE_Channel_StopEncodeSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BVCE_Channel_GetDefaultStopEncodeSettings() */

      BVCE_Channel_StopMode eStopMode;
} BVCE_Channel_StopEncodeSettings;

void
BVCE_Channel_GetDefaultStopEncodeSettings(
         BVCE_Channel_StopEncodeSettings *pstChStopEncodeSettings
         );

BERR_Code
BVCE_Channel_StopEncode(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_StopEncodeSettings *pstChStopEncodeSettings
         );

/* BVCE_Channel_FlushEncode - consumes all pending encoder buffer descriptors
 * If the channel has been stopped, then the flush will occur continuously
 * until the EOS is seen
 * */
BERR_Code
BVCE_Channel_FlushEncode(
         BVCE_Channel_Handle hVceCh
         );

/* Channel Settings */
typedef struct BVCE_FrameRate
{
      BAVC_FrameRateCode eFrameRate;

      bool bVariableFrameRateMode; /* See http://twiki-01.broadcom.com/bin/view/Arch/TranscodingBvnIntegration#User_Version */
} BVCE_FrameRate;

typedef struct BVCE_Channel_EncodeSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BVCE_Channel_GetDefaultEncodeSettings() */

      /* Frame Rate (in frames/second) */
      BVCE_FrameRate stFrameRate;

      /* Rate Control Parameters */
      BVCE_BitRate stBitRate;

      /* SW7425-5268: uiA2PDelay cannot be changed during an encode */
      unsigned uiA2PDelay; /* In 27Mhz clock ticks.  Desired "Arrival-to-Presentation" delay.  0 indicates use encoder's default. */

      /* GOP Parameters */
      BVCE_GOPStructure stGOPStructure;

      /* SW7425-5268: bITFPEnable cannot be changed during an encode */
      /* ITFP Mode */
      bool bITFPEnable;

      /* Number of Slices Per Picture */
      unsigned uiNumSlicesPerPic; /* 0 indicates use encoder's default. 16 is max. */
} BVCE_Channel_EncodeSettings;

void
BVCE_Channel_GetDefaultEncodeSettings(
         const BBOX_Handle hBox,
         BVCE_Channel_EncodeSettings *pstChEncodeSettings
         );

BERR_Code
BVCE_Channel_SetEncodeSettings(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_EncodeSettings *pstChEncodeSettings
         );

BERR_Code
BVCE_Channel_GetEncodeSettings(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_EncodeSettings *pstChEncodeSettings
         );

/* BVCE_Channel_BeginNewRAP() - Instructs VCE to create a Random Access Point (I/IDR frame) for the next picture received */
BERR_Code
BVCE_Channel_BeginNewRAP(
         BVCE_Channel_Handle hVceCh
         );

typedef struct BVCE_Channel_EncodeSettings_OnInputChange
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BVCE_Channel_GetDefaultEncodeSettings_OnInputChange() */

      BVCE_BitRate stBitRate;
} BVCE_Channel_EncodeSettings_OnInputChange;

/* BVCE_Channel_SetEncodeSettings_OnInputChange -
 * Sets the encode parameters to be used in sync with change to
 * the input to the encoder (e.g. resolution change)
 */
BERR_Code
BVCE_Channel_SetEncodeSettings_OnInputChange(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_EncodeSettings_OnInputChange *pstChEncodeSettings
         );

/* BVCE_Channel_GetEncodeSettings_OnInputChange -
 * Returns the pending settings (if any) otherwise the current settings
 */
BERR_Code
BVCE_Channel_GetEncodeSettings_OnInputChange(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_EncodeSettings_OnInputChange *pstChEncodeSettings
         );

#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_INVALID_INPUT_DIMENSION               0x00000001
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_LATE                        0x00000002
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_USER_DATA_DUPLICATE                   0x00000004
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_ADJUSTS_WRONG_FRAME_RATE              0x00000008
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_BVN_FRAME_RATE            0x00000010
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_RESOLUTION                0x00000020
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_MIN_FRAME_RATE           0x00000040
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MISMATCH_BVN_PIC_RESOLUTION           0x00000080
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_BITRATE_EXCEEDED                  0x00000100
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_BIN_BUFFER_FULL                       0x00000200
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_CDB_FULL                              0x00000400
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_PICARC_TO_CABAC_DINO_BUFFER_FULL      0x00000800
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_EBM_FULL                              0x00001000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_SLICES_EXCEEDED               0x00002000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_NUM_ENTRIES_INTRACODED_EXCEEDED   0x00004000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_IBBP_NOT_SUPPORTED_FOR_RESOLUTION     0x00008000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MBARC_BOOT_FAILURE                    0x00010000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MEASURED_ENCODER_DELAY_TOO_LONG       0x00020000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_CRITICAL                              0x00040000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_3_CH_MODE  0x00080000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_UNSUPPORTED_DISPLAY_FMT_IN_2_CH_MODE  0x00100000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_MAX_RESOLUTION_FOR_LEVEL_EXCEEDED     0x00200000
#define BVCE_CHANNEL_STATUS_FLAGS_ERROR_BITRATE_TOO_LOW                       0x00400000

#define BVCE_CHANNEL_STATUS_FLAGS_EVENT_INPUT_CHANGE                    0x00000001
#define BVCE_CHANNEL_STATUS_FLAGS_EVENT_EOS                             0x00000002

/* Channel Information */
typedef struct BVCE_Channel_Status
{
      uint32_t uiErrorFlags; /* See BVCE_CHANNEL_STATUS_FLAGS_ERROR_XXX */
      uint32_t uiEventFlags; /* See BVCE_CHANNEL_STATUS_FLAGS_EVENT_XXX */
      unsigned uiTotalErrors; /* Total number of errors that has occurred */
      unsigned uiTotalPicturesReceived; /* Number of pictures received at the input to the encoder */
      unsigned uiTotalPicturesDroppedDueToFrameRateConversion; /* Number of pictures that the encoder has configured to drop in order to follow the requested frame rate (Frame Rate Conversion) */
      unsigned uiTotalPicturesDroppedDueToErrors; /* Number of pictures that the encoder has configured to drop because ViCE2 did not finish the processing of the previous pictures on time and buffers are full. */
      unsigned uiTotalPicturesDroppedDueToHRDUnderflow; /* Number of pictures that the encoder has dropped because of a drop request from the Rate Control. The Rate Control may decide to drop picture in order to maintain the HRD buffer model. */
      unsigned uiTotalPicturesEncoded; /* Number of pictures output by the encoder */
      uint32_t uiLastPictureIdEncoded; /* Picture ID of the current picture being encoded. This is set as soon as the CME block decides to work on a picture. */
      unsigned uiEtsDtsOffset; /* The ETS to DTS offset for the encode session as determined by RC (in 27Mhz units)*/
      uint64_t uiSTCSnapshot; /* Snapshot of the STC at the time the GetStatus command is called. This is a 64-bit unsigned number. (in 27Mhz units) */
      unsigned uiAverageFramesPerSecond; /* The average encoder throughput in frames/sec */
} BVCE_Channel_Status;

/* BVCE_Channel_GetStatus - queries the FW for current status.  PI increments counts and ORs error flags bits as needed. */
BERR_Code
BVCE_Channel_GetStatus(
         BVCE_Channel_Handle hVceCh,
         BVCE_Channel_Status *pChannelStatus
         );

/* BVCE_Channel_ClearStatus - clears the specified fields of the status block
 * If pChannelStatus = NULL, then all status is cleared
 *
 * Set the bits in the flags (uiErrorFlags and uiEventFlags) for each bit to be cleared
 * Set the counters to be cleared to a non-zero value. All others should be zero.
 *
 * E.g. if you want to reset all error bits and uiTotalErrors, this is the sequence,
 *       BVCE_Channel_Status status;
 *       BKNI_Memset(&status, 0, sizeof(BVCE_Channel_Status));
 *       status.uiErrorFlags = 0xFFFFFFFF;
 *       status.uiTotalErrors = 1;
 *       BVCE_Channel_ClearStatus(hVceCh, &status);
 *
 * Note: Only state relative from a previous call to BVCE_Channel_GetStatus is cleared.
 *       E.g. 1) Call BVCE_Channel_GetStatus()
 *            2) Wait for 4 pictures to be transcoded
 *            3) Call BVCE_ChannelClearStatus(hVceCh, NULL) (to clear all state)
 *            4) Then immediately call BVCE_Channel_GetStatus(), uiTotalPicturesReceived will be >=4 (NOT 0).
 */
BERR_Code
BVCE_Channel_ClearStatus(
         BVCE_Channel_Handle hVceCh,
         const BVCE_Channel_Status *pChannelStatus
         );

typedef struct BVCE_Channel_Debug_DumpStateSettings
{
   uint32_t uiSignature;

   bool bDumpRegisters;
   bool bDumpCmdBuffer;
   bool bDumpBinBuffer;
   bool bDumpItbBuffer;
   bool bDumpCdbBuffer;
} BVCE_Channel_Debug_DumpStateSettings;

void
BVCE_Channel_Debug_GetDefaultDumpStateSettings(
   BVCE_Channel_Debug_DumpStateSettings *pstDumpStateSettings
   );

/* BVCE_Channel_Debug_DumpState - dumps the relevant state (Registers, CMD/BIN/ITB/CDB buffers) to disk to help debug VCE FW/HW issues
 *  Works only when all of the conditions are true:
 *     - in user mode.
 *     - secure memory is host accessible.
 *     - compiled with BVCE_TEST_MODE=y
 * */
void
BVCE_Channel_Debug_DumpState(
   BVCE_Channel_Handle hVceCh,
   const BVCE_Channel_Debug_DumpStateSettings *pstDumpStateSettings
   );

/************************/
/* VCE Output Functions */
/************************/

BERR_Code
BVCE_Channel_Output_Reset(
         BVCE_Channel_Handle hVceCh
         );

/* BVCE_Channel_Output_GetRegisters -
 * Returns the registers associated with the specified output hardware
 */
BERR_Code
BVCE_Channel_Output_GetRegisters(
         BVCE_Channel_Handle hVceCh,
         BAVC_VideoContextMap *pstVceChOutputRegisters
         );

/* BVCE_Channel_Output_GetBufferDescriptors -
 * Returns video buffer descriptors for CDB content in the
 * BAVC_VideoBufferDescriptor array(s)
 */
BERR_Code
BVCE_Channel_Output_GetBufferDescriptors(
   BVCE_Channel_Handle hVceCh,
   const BAVC_VideoBufferDescriptor **astDescriptors0, /* Pointer to an array of descriptors. E.g. *astDescriptorsX[0] is the first descriptor. *astDescriptorsX may be set to NULL iff uiNumDescriptorsX=0. */
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor **astDescriptors1, /* Needed to handle FIFO wrap */
   size_t *puiNumDescriptors1
   );

/* BVCE_Channel_Output_ConsumeBufferDescriptors -
 * Reclaims the specified number of video buffer descriptors
 * The CDB read pointer is updated accordingly
 */
BERR_Code
BVCE_Channel_Output_ConsumeBufferDescriptors(
   BVCE_Channel_Handle hVceCh,
   size_t uiNumBufferDescriptors
   );

/* BVCE_Output_GetBufferStatus -
 * Returns the output buffer status (e.g. the base virtual address)
 */
BERR_Code
BVCE_Channel_Output_GetBufferStatus(
   BVCE_Channel_Handle hVceCh,
   BAVC_VideoBufferStatus *pBufferStatus
   );

/*************/
/* User Data */
/*************/
/* BVCE_Channel_UserData_AddBuffers - adds user data field info structs(s) to user data queue for stream insertion */
BERR_Code
BVCE_Channel_UserData_AddBuffers_isr(
         BVCE_Channel_Handle hVceCh,
         const BUDP_Encoder_FieldInfo *pstUserDataFieldInfo, /* Pointer to first field info descriptor */
         size_t uiCount, /* Count of user data field buffer info structs */
         size_t *puiQueuedCount /* Count of user data field info structs queued by encoder (*puiQueuedCount <= uiCount) */
         );

/* BVCE_Channel_UserData_GetStatus_isr -
* Returns the user data status
*
*/
BERR_Code
BVCE_Channel_UserData_GetStatus_isr(
      BVCE_Channel_Handle hVceCh,
      BAVC_VideoUserDataStatus *pstUserDataStatus
);

/********************/
/* Helper Functions */
/********************/

/* A2PDelay */
typedef struct BVCE_A2PDelay
{
      unsigned uiMin; /* In 27Mhz clock ticks.  The minimum A2P delay given the specified settings */
      unsigned uiMax; /* In 27Mhz clock ticks.  The maximum supported A2P delay. */
} BVCE_A2PDelay;

BERR_Code
BVCE_GetA2PDelayInfo(
         const BVCE_Channel_EncodeSettings *pstChEncodeSettings, /* uiEndToEndDelay parameter is not used by this function */
         const BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings,
         BVCE_A2PDelay *pstA2PDelay
         );

/* Memory */
typedef struct BVCE_MemoryBoundsSettings
{
   unsigned uiUnused; /* For Future Expansion */
} BVCE_MemoryBoundsSettings;

/* BVCE_GetDefaultMemoryBoundsSettings - populates the memory bounds settings
 * with the worst case encoder parameters
 */
void
BVCE_GetDefaultMemoryBoundsSettings(
         const BVCE_PlatformSettings *pstPlatformSettings,
         BVCE_MemoryBoundsSettings *pstMemoryBoundsSettings
         );

/* BVCE_GetMemoryConfig - returns the memory requirements for single device */
void
BVCE_GetMemoryConfig(
         const BVCE_PlatformSettings *pstPlatformSettings,
         const BVCE_MemoryBoundsSettings *pstMemoryBoundsSettings,
         BVCE_MemoryConfig *pstMemoryConfig
         );

typedef struct BVCE_Channel_MemoryBoundsSettings
{
   BAVC_ScanType eInputType; /* Progressive or Interlaced */

   struct
   {
      /* Size (in pixels) */
      struct
      {
         unsigned uiWidth;
         unsigned uiHeight;
      } stMax;
   } stDimensions;

   struct
   {
      BVCE_BitRate stLargest; /* The largest bitrate intended to be used during the transcode. The default of 0 means the initial bitrate is the max */
   } stBitRate;
} BVCE_Channel_MemoryBoundsSettings;

/* BVCE_GetDefaultMemoryBoundsSettings - populates the memory bounds settings
 * with the worst case encoder parameters
 */
void
BVCE_Channel_GetDefaultMemoryBoundsSettings(
         const BBOX_Handle hBox,
         BVCE_Channel_MemoryBoundsSettings *pstChMemoryBoundsSettings
         );

typedef struct BVCE_Channel_MemorySettings
{
   unsigned uiInstance; /* Indicates which encoder instance is being used */

   struct
   {
      unsigned uiPicture;
      unsigned uiSecure;
   } memcIndex; /* Indicates which memory controllers are used for which memory types.  The indexes refer to BCHP_MemoryInfo.memc[] */

   BCHP_MemoryInfo *pstMemoryInfo;
} BVCE_Channel_MemorySettings;

/* BVCE_GetMemoryConfig - populates the memory config with
 * the memory required for the particular encode specified.
 * The memory config can be used directly in
 * BVCE_Channel_OpenSettings.stMemoryConfig and multiple
 * configs (1 for each channel) can be added together and
 * then used in BVCE_OpenSettings.stMemoryConfig
 */
void
BVCE_Channel_GetMemoryConfig(
         const BBOX_Handle hBox,
         const BVCE_Channel_MemorySettings *pstChMemorySettings,
         const BVCE_Channel_MemoryBoundsSettings *pstChMemoryBoundsSettings,
         BVCE_MemoryConfig *pstMemoryConfig
         );

/* Power Management */

/* BVCE_Power_Standby -
 * All channels must be explicitly stopped before VCE can be put into standby.
 * Channels DO NOT need to be closed.
 * All VCE device/channel handles remain valid after resume
 */
BERR_Code
BVCE_Power_Standby(
         BVCE_Handle hVce
         );

/* BVCE_Power_Resume -
 * Channels that were opened when standby was called are re-opened.
 * All VCE device/channel handles remain valid after resume
 */
BERR_Code
BVCE_Power_Resume(
         BVCE_Handle hVce
         );

#ifdef __cplusplus
}
#endif

#endif /* BVCE_H_ */
