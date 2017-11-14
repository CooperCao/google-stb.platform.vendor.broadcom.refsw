/***************************************************************************
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
 *
 * Module Description: APE Path routines
 *
 ***************************************************************************/

#include "bape_types.h"
#include "bape_priv.h"

#ifndef BAPE_PATH_PRIV_H_
#define BAPE_PATH_PRIV_H_

/***************************************************************************
Summary:
Build paths for all downstream connections from a node 
 
Description: 
FMM Resources will be acquired as needed.  DSP stages will be added to 
task create settings as needed.   
***************************************************************************/
BERR_Code BAPE_PathNode_P_AcquirePathResources(
    BAPE_PathNode *pNode
    );

/***************************************************************************
Summary:
Teardown paths for all downstream connections from a node as much as possible
 
Description: 
FMM Resources will be released.  DSP Mixers will also be destroyed.
***************************************************************************/
void BAPE_PathNode_P_ReleasePathResources(
    BAPE_PathNode *pNode
    );

/***************************************************************************
Summary:
Configure allocated resources. 
 
Description: 
FMM Resources will be configured from producer -> consumer.  DSP task is 
allocated prior to this call, and stage settings will be applied.  
***************************************************************************/
BERR_Code BAPE_PathNode_P_ConfigurePathResources(
    BAPE_PathNode *pNode
    );

/***************************************************************************
Summary:
Start paths for all downstream connections from a node 
 
Description: 
This will start the nodes in the path from consumer -> producer. 
***************************************************************************/
BERR_Code BAPE_PathNode_P_StartPaths(
    BAPE_PathNode *pNode
    );

/***************************************************************************
Summary:
Stop paths for downstream connections from this node
 
Description: 
This will stop the nodes in the path from producer -> consumer.   The decoder 
task handle is invalidated during this call. 
***************************************************************************/
void BAPE_PathNode_P_StopPaths(
    BAPE_PathNode *pNode
    );

/***************************************************************************
Summary:
Make a connection
***************************************************************************/
BERR_Code BAPE_PathNode_P_AddInput(
    BAPE_PathNode *pNode,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Destroy a connection
***************************************************************************/
BERR_Code BAPE_PathNode_P_RemoveInput(
    BAPE_PathNode *pNode,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs
***************************************************************************/
BERR_Code BAPE_PathNode_P_RemoveAllInputs(
    BAPE_PathNode *pNode
    );

/***************************************************************************
Summary:
Get current format capabilities
***************************************************************************/
void BAPE_PathNode_P_GetInputCapabilities(
    BAPE_PathNode *pNode, 
    BAPE_FMT_Capabilities *pCaps    /* [out] */
    );

/***************************************************************************
Summary:
Set current format capabilities
***************************************************************************/
BERR_Code BAPE_PathNode_P_SetInputCapabilities(
    BAPE_PathNode *pNode, 
    const BAPE_FMT_Capabilities *pCaps
    );

/***************************************************************************
Summary:
Get output status for this node
***************************************************************************/
void BAPE_PathNode_P_GetOutputStatus(
    BAPE_PathNode *pNode,
    BAPE_PathNodeOutputStatus *pStatus      /* [out] */
    );

/***************************************************************************
Summary:
Search for consumers by a type
***************************************************************************/
void BAPE_PathNode_P_FindConsumersByType_isrsafe(
    BAPE_PathNode *pNode,
    BAPE_PathNodeType type,
    unsigned maxConsumers,
    unsigned *pNumFound,        /* [out] */
    BAPE_PathNode **pConsumers  /* [out] Must be an array of at least maxConsumers length */
    );

/***************************************************************************
Summary:
Search for consumers by a type and subtype using PathNode
***************************************************************************/
void BAPE_PathNode_P_FindConsumersBySubtype_isrsafe(
    BAPE_PathNode *pNode,
    BAPE_PathNodeType type,
    unsigned subtype,
    unsigned maxConsumers,
    unsigned *pNumFound,        /* [out] */
    BAPE_PathNode **pConsumers  /* [out] Must be an array of at least maxConsumers length */
    );


/***************************************************************************
Summary:
Search for consumers by a type and subtype using PathConnector
***************************************************************************/
void BAPE_PathConnector_P_FindConsumersBySubtype_isrsafe(
    BAPE_PathConnector *pConnector,
    BAPE_PathNodeType type,
    unsigned subtype,
    unsigned maxConsumers,
    unsigned *pNumFound,        /* [out] */
    BAPE_PathNode **pConsumers   /* [out] Must be an array of at least maxConsumers length */
    );


/***************************************************************************
Summary:
Search for outputs on this path
***************************************************************************/
void BAPE_PathNode_P_GetConnectedOutputs_isrsafe(
    BAPE_PathNode *pNode,
    unsigned maxOutputs,
    unsigned *pNumFound,        /* [out] */
    BAPE_OutputPort *pOutputs   /* [out] Must be an array of at least maxOutputs length */
    );
#define BAPE_PathNode_P_GetConnectedOutputs BAPE_PathNode_P_GetConnectedOutputs_isrsafe
/***************************************************************************
Summary:
Determine if a node is a consumer from this node
***************************************************************************/
bool BAPE_PathNode_P_NodeIsConsumer(
    BAPE_PathNode *pSourceNode,
    BAPE_PathNode *pConsumerNode
    );

/***************************************************************************
Summary:
Determine if a node of specified type is between source and consumer
***************************************************************************/
bool BAPE_PathNode_P_HasNodeTypeBetween(
    BAPE_PathNodeType type,
    unsigned subtype,
    BAPE_PathNode * pSourceNode,
    BAPE_PathNode * pConsumerNode
    );

/***************************************************************************
Summary:
Determine if a Decoder has a DspMixer downstream without AIO in between
***************************************************************************/
BERR_Code BAPE_PathNode_P_GetDecodersDownstreamDspMixer(
    BAPE_PathNode * pSourceNode,
    BAPE_MixerHandle * fwMixer
    );

/***************************************************************************
Summary:
Remove all inputs
***************************************************************************/
BERR_Code BAPE_PathNode_P_RemoveAllInputs(
    BAPE_PathNode *pNode
    );

/***************************************************************************
Summary:
Set an connector's sample rate on the fly (used with decoders)
***************************************************************************/
void BAPE_Connector_P_SetSampleRate_isr(
    BAPE_Connector connector,
    unsigned sampleRate
    );

/***************************************************************************
Summary:
Retrieve a connector's format
***************************************************************************/
void BAPE_Connector_P_GetFormat_isrsafe(
    BAPE_Connector connector,
    BAPE_FMT_Descriptor *pFormat    /* [out] */
    );

#define BAPE_Connector_P_GetFormat BAPE_Connector_P_GetFormat_isrsafe

/***************************************************************************
Summary:
Modify a connector's format
***************************************************************************/
BERR_Code BAPE_Connector_P_SetFormat(
    BAPE_Connector connector,
    const BAPE_FMT_Descriptor *pNewFormat
    );

/***************************************************************************
Summary:
Get number of downstream connections for a connector
***************************************************************************/
unsigned BAPE_Connector_P_GetNumConnections(
    BAPE_Connector connector
    );

/***************************************************************************
Summary:
Get a connection between a connector and a destination node
***************************************************************************/
BAPE_PathConnection *BAPE_Connector_P_GetConnectionToSink_isrsafe(
    BAPE_Connector connector,
    BAPE_PathNode *pSink
    );

/***************************************************************************
Summary:
Set the connector mute state
***************************************************************************/
void BAPE_Connector_P_SetMute(
    BAPE_Connector connector,
    bool muted
    );

/***************************************************************************
Summary:
Remove all downstream connections from this connector
***************************************************************************/
void BAPE_Connector_P_RemoveAllConnections(
    BAPE_Connector connector
    );

/***************************************************************************
Summary:
Returns a null-terminated string representation of a BAPE_PathNodeType enum.
***************************************************************************/
const char *BAPE_PathNode_P_PathNodeTypeToText(
     BAPE_PathNodeType pathNodeType 
     );

/***************************************************************************
Summary:
Define a data type for the callback used by the following "enumerate" 
functions.  The callback arguments are: 
    pNode: pointer to the current node
    level: the depth of the current node
    index: the horizontal position of the current node (within the specified depth)
    
***************************************************************************/
typedef unsigned (*BAPE_PathNode_P_EnumerateCallback)(
    BAPE_PathNode *pNode, 
    int level, 
    int index
    );

/***************************************************************************
Summary:
Do a depth-first, pre-order traversal of the downstream PathNodes.  The 
specified callback will be called for each PathNode visited (except for the 
starting PathNode)
***************************************************************************/
BERR_Code BAPE_PathNode_P_EnumerateUpstreamPathNodes( 
    BAPE_PathNode  *pPathNode, 
    int level, 
    BAPE_PathNode_P_EnumerateCallback callback
    );


/***************************************************************************
Summary:
Do a depth-first, pre-order traversal of the upstream PathNodes.  The 
specified callback will be called for each PathNode visited (except for the 
starting PathNode)
***************************************************************************/
BERR_Code BAPE_PathNode_P_EnumerateDownstreamPathNodes( 
    BAPE_PathNode  *pPathNode, 
    int level, 
    BAPE_PathNode_P_EnumerateCallback callback
    );

/***************************************************************************
Summary: 
    Determine if a node is active (running or about to start) or not (idle). 
***************************************************************************/
bool BAPE_PathNode_P_IsActive(
    BAPE_PathNode *pPathNode
    );

/***************************************************************************
Summary:
    Find the connection between an input and a Path Node.
***************************************************************************/
BAPE_PathConnection *BAPE_PathNode_P_FindConnection(
    BAPE_PathNode *pNode,
    BAPE_Connector input
    );

/***************************************************************************
Summary: 
Check the tree for any non-connected (orphan) nodes. 
***************************************************************************/
void BAPE_PathNode_P_FindOrphans(BAPE_PathNode *pNode);

#endif
