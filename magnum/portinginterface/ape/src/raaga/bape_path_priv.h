/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * Module Description: APE Path routines
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 * $brcm_Log: $
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
void BAPE_PathNode_P_FindConsumersByType(
    BAPE_PathNode *pNode,
    BAPE_PathNodeType type,
    unsigned maxConsumers,
    unsigned *pNumFound,        /* [out] */
    BAPE_PathNode **pConsumers  /* [out] Must be an array of at least maxConsumers length */
    );

/***************************************************************************
Summary:
Search for consumers by a type and subtype
***************************************************************************/
void BAPE_PathNode_P_FindConsumersBySubtype(
    BAPE_PathNode *pNode,
    BAPE_PathNodeType type,
    unsigned subtype,
    unsigned maxConsumers,
    unsigned *pNumFound,        /* [out] */
    BAPE_PathNode **pConsumers  /* [out] Must be an array of at least maxConsumers length */
    );

/***************************************************************************
Summary:
Search for outputs on this path
***************************************************************************/
void BAPE_PathNode_P_GetConnectedOutputs(
    BAPE_PathNode *pNode,
    unsigned maxOutputs,
    unsigned *pNumFound,        /* [out] */
    BAPE_OutputPort *pOutputs   /* [out] Must be an array of at least maxOutputs length */
    );

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
void BAPE_Connector_P_GetFormat(
    BAPE_Connector connector,
    BAPE_FMT_Descriptor *pFormat    /* [out] */
    );

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
BAPE_PathConnection *BAPE_Connector_P_GetConnectionToSink(
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
