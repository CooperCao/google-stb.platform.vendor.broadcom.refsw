/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "bdsp_raaga_priv_include.h"

BDBG_MODULE(bdsp_raaga_io_priv);

/***********************************************************************
Name        :   BDSP_Raaga_P_AddRaveInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which input must be fed from RAVE Buffer.
				pContextMap     -   Context map of the XPT.
				pInputIndex     -   Index of the input for this Stage returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the free input index for the stage which can be used for this RAVE buffer.
		2)  Treat the CDB and ITB buffers as two buffers for same input Index.
		3)  Populate the Source details for the Stage with appropriate values using the XPT descriptor, each for CDB and ITB buffer.
		4)  Populate the Address of the I/O buffers, each for CDB and ITB buffer.
		5)  Increment the number of inputs for this stage (both CDB and ITB are treated as 1 input) and
			also the number of RAVE buffers (as 1 for both CDB and ITB) in the eco-system.
		6)  Fill the Input index pointer for the PI for later use (only 1 as both CDB and ITB are treated as same input).
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddRaveInput(
	void *pStageHandle,
	const BAVC_XptContextMap *pContextMap,
	unsigned *pInputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned ipIndex=0;
	BDSP_P_ConnectionDetails *psStageInput;

	BDBG_ENTER(BDSP_Raaga_P_AddRaveInput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDSP_P_GetFreeInputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);
	psStageInput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageInput[ipIndex];

	psStageInput->eValid          = BDSP_AF_P_eValid;
	psStageInput->eConnectionType = BDSP_ConnectionType_eRaveBuffer;
	psStageInput->connectionHandle.rave.raveContextMap = *pContextMap;

	pRaagaStage->totalInputs+=1;
	pRaagaStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eRaveBuffer]+=1;
	*pInputIndex = ipIndex;

	BDBG_LEAVE(BDSP_Raaga_P_AddRaveInput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddFmmOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which output must be fed to FMM Buffer.
				dataType            -   Type of data that will fed into FMM buffer.
				pDescriptor     -   Descriptor for the FMM buffer.
				pOutputIndex        -   Index of the Ouput from this Stage returned back to PI.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free output index for the stage which can be used for this FMM buffer.
		2)  Depending on the datatype, figure out the number of channels and output type.
		3)  Populate the Dstination details for the Stage with appropriate values using the FMM descriptor provided by the PI.
		4)  Populate the Address of the I/O buffers and rate control data.
		5)  Increment the number of ouputs from this stage and also the number of FMM buffers in the eco-system.
		6)  Fill the Ouput index pointer for the PI for later use.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddFmmOutput(
	void *pStageHandle,
	BDSP_DataType dataType,
	const BDSP_FmmBufferDescriptor *pDescriptor,
	unsigned *pOutputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned opIndex=0, channels =0, index = 0;
	BDSP_P_ConnectionDetails *psStageOutput;

	BDBG_ENTER(BDSP_Raaga_P_AddFmmOutput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDSP_P_GetFreeOutputPortIndex(&(pRaagaStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaStage->sStageConnectionInfo.sStageOutput[opIndex];

	BDBG_MSG(("BDSP_Raaga_P_AddFmmOutput:Connecting FMM output to Stage (%d) with datatype = %d", pRaagaStage->eAlgorithm, dataType));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &pRaagaStage->sStageConnectionInfo.eStageOpDataType[opIndex]);
	if(channels!=pDescriptor->numBuffers)
	{
		errCode=BERR_TRACE(BERR_INVALID_PARAMETER);
		BDBG_ERR(("BDSP_Raaga_P_AddFmmOutput::FMM Output not added!"));
		goto end;
	}

	psStageOutput->eValid          = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType = BDSP_ConnectionType_eFmmBuffer;
	psStageOutput->dataType        = dataType;
	psStageOutput->connectionHandle.fmm.fmmDescriptor = *pDescriptor;

	for(index=0;index<BDSP_AF_P_MAX_CHANNEL_PAIR;index++)/*rate controller Initialize per pair of channels*/
	{
		psStageOutput->connectionHandle.fmm.rateController[index].wrcnt = BDSP_AF_P_WRCNT_INVALID;
	}

	for(index=0; index<((pDescriptor->numBuffers+1)>>1); index++)/*rate controller per pair of channels*/
	{
		psStageOutput->connectionHandle.fmm.rateController[index].wrcnt = pDescriptor->rateControllers[index].wrcnt;
	}

	pRaagaStage->totalOutputs+=1;
	pRaagaStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eFmmBuffer]+=1;
	*pOutputIndex = opIndex;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AddFmmOutput);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AddOutputStage

Type        :   PI Interface

Input       :   pSrcStageHandle -   Handle of the Source Stage for which output must be fed to another Stage.
				dataType            -   Type of Data which is output of the Stage.
				pDstStageHandle -   Handle of the Destination Stage which must be connected at Output.
				pSourceOutputIndex  -   Index of the Ouput for the Source Stage.
				pDestinationInputIndex  -   Index of the Input for the Destination Stage.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Retreive the the free output and index for the Source and Destination Stage respectively.
		2)  Interconnect the Stages but populating the Output and Input structures of Source Stage and Destination Stage using a InterStage Buffer.
		3)  Return the Source Output index and Destination Intput Index.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AddOutputStage(
	void *pSrcStageHandle,
	BDSP_DataType dataType,
	void *pDstStageHandle,
	unsigned *pSourceOutputIndex,
	unsigned *pDestinationInputIndex
)
{
	BERR_Code errCode=BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaSrcStage = (BDSP_RaagaStage *)pSrcStageHandle;
	BDSP_RaagaStage *pRaagaDstStage = (BDSP_RaagaStage *)pDstStageHandle;
	unsigned opIndex=0, ipIndex =0, channels =0, interStagePortIndex = 0;
	BDSP_P_ConnectionDetails *psStageOutput, *psStageInput;
	BDSP_P_InterStagePortInfo *psInterStagePortInfo;
	const BDSP_P_AlgorithmInfo *psAlgoInfo;

	BDBG_ENTER(BDSP_Raaga_P_AddOutputStage);
	BDBG_OBJECT_ASSERT(pRaagaSrcStage, BDSP_RaagaStage);
	BDBG_OBJECT_ASSERT(pRaagaDstStage, BDSP_RaagaStage);

	psAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaSrcStage->eAlgorithm);

	BDSP_P_GetFreeOutputPortIndex(&(pRaagaSrcStage->sStageConnectionInfo.sStageOutput[0]), &opIndex);
	BDBG_ASSERT(opIndex < BDSP_AF_P_MAX_OP_FORKS);
	BDSP_P_GetFreeOutputPortIndex(&(pRaagaDstStage->sStageConnectionInfo.sStageInput[0]), &ipIndex);
	BDBG_ASSERT(ipIndex < BDSP_AF_P_MAX_IP_FORKS);

	BDBG_MSG(("BDSP_Raaga_P_AddOutputStage: Connecting Stage(%d) to Stage(%d) with datatype =%d",
		pRaagaSrcStage->eAlgorithm, pRaagaDstStage->eAlgorithm, dataType));
	BDSP_P_GetDistinctOpTypeAndNumChans(dataType, &channels, &pRaagaSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex]);
	psStageOutput = (BDSP_P_ConnectionDetails *)&pRaagaSrcStage->sStageConnectionInfo.sStageOutput[opIndex];
	psStageOutput->eValid          = BDSP_AF_P_eValid;
	psStageOutput->eConnectionType = BDSP_ConnectionType_eStage;
	psStageOutput->dataType        = dataType;
	psStageOutput->connectionHandle.stage.hStage = &(pRaagaDstStage->stage);
	/*Data Access Population for Source Stage Output*/
	BDSP_Raaga_P_GetInterStagePortIndex(pRaagaSrcStage, pRaagaSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex],&interStagePortIndex);
	psInterStagePortInfo = &pRaagaSrcStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
	psInterStagePortInfo->ePortDataType = pRaagaSrcStage->sStageConnectionInfo.eStageOpDataType[opIndex];
	psInterStagePortInfo->branchFromPort++;
	psInterStagePortInfo->dataAccessAttributes.eDataAccessType    = BDSP_AF_P_Port_eChannelInterleavedPCM;
	psInterStagePortInfo->dataAccessAttributes.ui32bytesPerSample = 4;
	psInterStagePortInfo->dataAccessAttributes.ui32maxChannelSize = psAlgoInfo->samplesPerChannel*4;
	psInterStagePortInfo->dataAccessAttributes.ui32numChannels	  = psAlgoInfo->maxChannelsSupported;

	pRaagaSrcStage->totalOutputs+=1;
	pRaagaSrcStage->sStageConnectionInfo.numOutputs[BDSP_ConnectionType_eStage]+=1;
	*pSourceOutputIndex = opIndex;

	psStageInput = (BDSP_P_ConnectionDetails *)&pRaagaDstStage->sStageConnectionInfo.sStageInput[ipIndex];
	psStageInput->eValid		   = BDSP_AF_P_eValid;
	psStageInput->eConnectionType  = BDSP_ConnectionType_eStage;
	psStageInput->dataType         = dataType;
	psStageInput->connectionHandle.stage.hStage = &(pRaagaSrcStage->stage);

	pRaagaDstStage->totalInputs+=1;
	pRaagaDstStage->sStageConnectionInfo.numInputs[BDSP_ConnectionType_eStage]+=1;
	*pDestinationInputIndex = ipIndex;

	BDBG_LEAVE(BDSP_Raaga_P_AddOutputStage);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveInput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage from which the input must be removed.
				inputIndex      -   The index of the input for the Stage.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Retrieve the Connection type used at the input.
		2)  Make sure the Stage is not running if the input is FMM or Intertask buffer, when attempt is made to remove the input.
		3)  If the Stage is running or in Wtachdog recovery, then CIT Reconfigure needs to be performed.
		4)  If the connection type is Intertask task buffer, remove the destination handle of the intertask buffer.
		5)  If source handle of the intertask buffer is NULL, then inUse variable is set to false.
			Now the Intertask buffer can be destroyed by PI later.
		6)  Special handling is required if in watchdog recovery, Intertask buffer needs to be destroyed.
		7)  Disconnect the Input but resetting Stage Input structure.
***********************************************************************/
void BDSP_Raaga_P_RemoveInput(
	void *pStageHandle,
	unsigned inputIndex
)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_ConnectionType connectionType;
	BDSP_P_StageConnectionInfo *pStageConnectionInfo;
	BDSP_P_ConnectionDetails   *pStageInputDetails;

	BDBG_ENTER(BDSP_Raaga_P_RemoveInput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(inputIndex < BDSP_AF_P_MAX_IP_FORKS);
	if(pRaagaStage->totalInputs == 0)
	{
		BDBG_ERR(("BDSP_Raaga_P_RemoveInput: Invalid index(%d), Number of inputs to stage is already Zero", inputIndex));
		return;
	}
	pStageConnectionInfo = (BDSP_P_StageConnectionInfo *)&pRaagaStage->sStageConnectionInfo;
	pStageInputDetails = (BDSP_P_ConnectionDetails *)&pStageConnectionInfo->sStageInput[inputIndex];
	connectionType = pStageInputDetails->eConnectionType;
	if(pRaagaStage->running)
	{
		BDBG_ERR(("Cannot remove inputs when the stage is running"));
		return;
	}

	pStageConnectionInfo->numInputs[connectionType]--;

	pStageInputDetails->eValid = BDSP_AF_P_eInvalid;
	pStageInputDetails->eConnectionType = BDSP_ConnectionType_eMax;
	BKNI_Memset(&pStageInputDetails->connectionHandle, 0, sizeof(pStageInputDetails->connectionHandle));
	pRaagaStage->totalInputs--;

	BDBG_LEAVE(BDSP_Raaga_P_RemoveInput);
	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveAllInputs

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which all inputs must be removed.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Recursively remove the all the inputs connected to the stage.
***********************************************************************/
void BDSP_Raaga_P_RemoveAllInputs(
	void *pStageHandle
)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned i, numInputs;

	BDBG_ENTER(BDSP_Raaga_P_RemoveAllInputs);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	numInputs = pRaagaStage->totalInputs;

	for (i = 0; i < numInputs; i++)
	{
		BDSP_Raaga_P_RemoveInput(pStageHandle, i);
	}

	BDBG_ASSERT(0 == pRaagaStage->totalInputs);

	for (i = 0; i < BDSP_ConnectionType_eMax; i++)
	{
		BDBG_ASSERT(pRaagaStage->sStageConnectionInfo.numInputs[i] == 0);
	}

	BDBG_LEAVE(BDSP_Raaga_P_RemoveAllInputs);
	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveOutput

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which output must be removed.
				outputIndex     -   The index of the Output in the Stage.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Make sure the Stage is not running, when attempt is made to remove the output.
		2)  Retrieve the Connection type used at the output.
		3)  If the connection type is Intertask task buffer, remove the source handle of the intertask buffer.
		4)  If destination handle of the intertask buffer is NULL, then inUse variable is set to false.
			Now the Intertask buffer can be destroyed by PI later.
		5)  Special handling is required if in watchdog recovery, Intertask buffer needs to be destroyed.
		6)  Disconnect the Output but resetting Stage Output structure.
		7)  If the Output is RAVE buffer then disconnection needs to be done for both CDB and ITB buffer.
***********************************************************************/
void BDSP_Raaga_P_RemoveOutput(
	void *pStageHandle,
	unsigned outputIndex
)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	BDSP_ConnectionType connectionType;
	BDSP_P_StageConnectionInfo *pStageConnectionInfo;
	BDSP_P_ConnectionDetails   *pStageOutputDetails;

	BDBG_ENTER(BDSP_Raaga_P_RemoveOutput);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(outputIndex < BDSP_AF_P_MAX_OP_FORKS);

	if(pRaagaStage->totalOutputs == 0)
	{
		BDBG_ERR(("BDSP_Raaga_P_RemoveOutput: Invalid index(%d), Number of outputs to stage is already Zero", outputIndex));
		return;
	}
	pStageConnectionInfo = (BDSP_P_StageConnectionInfo *)&pRaagaStage->sStageConnectionInfo;
	pStageOutputDetails = (BDSP_P_ConnectionDetails *)&pStageConnectionInfo->sStageOutput[outputIndex];
	connectionType = pStageOutputDetails->eConnectionType;
	if(pRaagaStage->running)
	{
		BDBG_ERR(("Cannot remove output when the stage is running"));
		return;
	}

	if(BDSP_ConnectionType_eStage == connectionType)
	{
		BDSP_P_InterStagePortInfo *psInterStagePortInfo;
		unsigned interStagePortIndex = 0;
		BDSP_Raaga_P_GetInterStagePortIndex(pRaagaStage, pStageConnectionInfo->eStageOpDataType[outputIndex],&interStagePortIndex);
		psInterStagePortInfo = &pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[interStagePortIndex];
		psInterStagePortInfo->branchFromPort--;
		if(psInterStagePortInfo->branchFromPort == 0)
		{
			psInterStagePortInfo->ePortDataType = BDSP_AF_P_DistinctOpType_eMax;
			psInterStagePortInfo->tocIndex      = BDSP_AF_P_TOC_INVALID;
			BKNI_Memset(&psInterStagePortInfo->bufferDescriptorAddr[0], 0, (sizeof(dramaddr_t)*BDSP_AF_P_MAX_PORT_BUFFERS));
			BKNI_Memset(&psInterStagePortInfo->dataAccessAttributes, 0, sizeof(BDSP_AF_P_Port_sDataAccessAttributes));
		}
	}

	pStageConnectionInfo->numOutputs[connectionType]--;
	pStageConnectionInfo->eStageOpDataType[outputIndex] = BDSP_AF_P_DistinctOpType_eMax;

	pStageOutputDetails->eValid = BDSP_AF_P_eInvalid;
	pStageOutputDetails->eConnectionType = BDSP_ConnectionType_eMax;
	BKNI_Memset(&pStageOutputDetails->connectionHandle, 0, sizeof(pStageOutputDetails->connectionHandle));
	pRaagaStage->totalOutputs--;
	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_RemoveAllOutputs

Type        :   PI Interface

Input       :   pStageHandle        -   Handle of the Stage for which all outputs must be removed.

Return      :   None

Functionality   :   Following are the operations performed.
		1)  Recursively remove the all the outputs connected to the stage.
		2)  Special handling is done for RAVE buffer as both the outputs CDB and ITB buffers are removed in one instance.
***********************************************************************/
void BDSP_Raaga_P_RemoveAllOutputs(
	void *pStageHandle)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	unsigned i, numOutputs;

	BDBG_ENTER(BDSP_Raaga_P_RemoveAllOutputs);

	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	numOutputs = pRaagaStage->totalOutputs;

	for (i = 0; i < numOutputs; i++)
	{
		BDSP_Raaga_P_RemoveOutput(pStageHandle, i);
	}

	BDBG_ASSERT(0 == pRaagaStage->totalOutputs);

	for (i = 0; i < BDSP_ConnectionType_eMax; i++)
	{
		BDBG_ASSERT(pRaagaStage->sStageConnectionInfo.numOutputs[i] == 0);
	}

	BDBG_LEAVE(BDSP_Raaga_P_RemoveAllOutputs);
}
