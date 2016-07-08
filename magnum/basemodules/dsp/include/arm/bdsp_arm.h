/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/

#ifndef BDSP_ARM_H_
#define BDSP_ARM_H_


/***************************************************************************
Summary:
Arm DSP Settings
***************************************************************************/
typedef struct BDSP_ArmSettings
{
    bool authenticationEnabled; /* If authentication is enabled, all the firmware execs needs to be downloaded at open time and
                                   the DSP must be booted via a separate call to BDSP_Arm_Boot() after HSM has been authenticated. */
    const BIMG_Interface *pImageInterface;      /* Interface to access firmware image. This interface must be
                                                   implemented and the function pointers must be stored here. */
    void **pImageContext;                       /* Context for the Image Interface. This is also provided by
                                                   the implementation of the Image interface */
    unsigned NumDevices;    /*Number of DSP supported in the System, currently used for estimation of Memory*/
    bool preloadImages;         /* If true, all firmware images will be loaded on startup.  Default=false. */
    unsigned maxAlgorithms[BDSP_AlgorithmType_eMax] ;
	BTEE_InstanceHandle hBteeInstance;

#ifdef SR_REMOVE  /*SR_TBD */
    BDSP_Arm_DebugTypeSettings debugSettings[BDSP_Arm_DebugType_eLast]; /* Debug information for the different types of debug logs */
    BMEM_Heap_Handle firmwareHeap;  /* Optional, this is the heap handle the firmware itself will be loaded into if
                                       specified otherwise the heap specified in the open call will be used */

#endif
} BDSP_ArmSettings;

/*********************************************************************
Summary:
    This structure contain elements that is returned by
        CalcThreshold_BlockTime_AudOffset API.

Description:

        ui32Threshold and ui32BlockTime goes to FW and
        ui32AudOffset goes to Application and TSM user cfg

See Also:
**********************************************************************/
typedef struct BDSP_ARM_CTB_Output
{
    uint32_t ui32Threshold;                                 /* Interms of samples */
    uint32_t ui32BlockTime;                                 /* Interms of Time (msec)  */
    uint32_t ui32AudOffset;                                 /* AudOffset in Time (msec) */

}BDSP_ARM_CTB_Output;


typedef struct BDSP_ARM_CTB_Input
{
    BDSP_AudioTaskDelayMode audioTaskDelayMode;
    BDSP_TaskRealtimeMode realtimeMode;
}BDSP_ARM_CTB_Input;

/***************************************************************************
Summary:
Firmware download status.
***************************************************************************/
typedef struct BDSP_Arm_DownloadStatus
{
    void    *pBaseAddress;      /* Pointer to base of downloaded firmware executables */
    dramaddr_t physicalAddress;   /* Physical memory offset of downloaded firmware executables */
    size_t   length;            /* Length of executables in bytes */
} BDSP_Arm_DownloadStatus;


/***************************************************************************
Summary:
Use case scenario provided by APE
***************************************************************************/
typedef struct BDSP_ArmUsageOptions
{
    bool           Codeclist[BDSP_Algorithm_eMax];  /* Total list containing the Codecs enabled or disabled */
    BDSP_AudioDolbyCodecVersion DolbyCodecVersion;
    BDSP_DataType IntertaskBufferDataType;
    unsigned NumAudioDecoders;
    unsigned NumAudioPostProcesses;
    unsigned NumAudioEncoders;
    unsigned NumAudioMixers;
    unsigned NumAudioPassthru;
    unsigned NumAudioEchocancellers;
    unsigned NumVideoDecoders;
    unsigned NumVideoEncoders;
} BDSP_ArmUsageOptions;

BERR_Code BDSP_Arm_Open(
    BDSP_Handle *pDsp,                      /* [out] */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMEM_Handle memHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    const BDSP_ArmSettings *pSettings
    );


void BDSP_Arm_GetDefaultSettings(
    BDSP_ArmSettings *pSettings     /* [out] */
    );

BERR_Code BDSP_Arm_GetDownloadStatus(
    BDSP_Handle handle,
    BDSP_Arm_DownloadStatus *pStatus /* [out] */
    );

BERR_Code BDSP_Arm_Initialize(BDSP_Handle handle);

#endif /* BDSP_ARM_H_ */
