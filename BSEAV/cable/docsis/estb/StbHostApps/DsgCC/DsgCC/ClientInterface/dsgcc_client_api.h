/*****************************************************************************

  Copyright (c) 2007  Broadcom Corporation
  All Rights Reserved
  No portions of this material may be reproduced in any form without the
  written permission of:
          Broadcom Corporation
          16215 Alton Parkway
          Irvine, California 92619
  All information contained in this document is Broadcom Corporation
  company private, proprietary, and trade secret.

******************************************************************************

  Filename:       dsgcc_client_api.h
  Author:         Pinar Taskiran
  Creation Date:  May 18, 2007

******************************************************************************
  Description:


******************************************************************************/

#ifndef DSGCC_CLIENT_API_H
#define DSGCC_CLIENT_API_H

#if defined __cplusplus
extern "C" {
#endif

/********************** Include Files ***************************************/
#ifdef DSG_RPC_INCLUDED
#include "dsgccClientRegistration_rpc.h"
#include "dsgccClientCallback_rpc.h"
#endif
/********************** Global Types ****************************************/

#define MAXCLIENTNAMESIZE 32
#define MAXNUMDSGCLIENTS 32

#ifndef DSG_RPC_INCLUDED
/****************************************************************************
** dsg_classifier
** This data structure is optionally used by clients to set DSG Classifier 
** information when requesting a DSG tunnel from DSG-CC.
** Within each classifier, several of the parameters are optional. If any 
** classifier parameter is to be used, the corresponding Boolean "xxxx_used" 
** flag should be set. For example, if the classifier includes an IP source 
** address, the address value is held in source_ip_adr and the flag 
** source_ip_used should be set to true. If the classifier does not include an 
** IP source address, source_ip_used should be set to false.
****************************************************************************/
typedef struct
{ 
	unsigned char source_ip_used;
	unsigned char source_ip_mask_used;
	unsigned char dest_ip_used;
	unsigned char port_used;
	unsigned int source_ip_adr;
	unsigned int source_ip_mask;
	unsigned int dest_ip_adr;
	unsigned short port_start;
	unsigned short port_end; 
} dsg_classifier;

/****************************************************************************
** struct dsgClientRegInfo
** This data structure is used by clients to provide unique identification 
** when communicating with DSG-CC.
****************************************************************************/
struct dsgClientRegInfo 
{
	unsigned int clientProgNum;				/* This field is reserved for the client to provide a callback method. */
											/* DSG-CC uses this method to notify the client of the changes in its tunnel status and the status of the DSG.*/
											
	unsigned int clientPort;				/* This is the local UDP port number to which the DSG-CC sends tunnel traffic for the client. */
											/* It is the responsibility of the clients to choose unique port numbers.*/
											
	unsigned int idType;					/* These 32 bits are laid out as follows: bit number (31=MSB 0=LSB) */
											
											/* 16 LSBs of idType indicate the client's ID type.                 */
											/* 0x0 - Reserved                                                   */
											/* 0x1 - Broadcast Type					(Subtype 50.4.1)			*/
											/* 0x2 - Well Known MAC Address Type	(Subtype 50.4.2)			*/
											/* 0x3 - CA Type						(Subtype 50.4.3)			*/
											/* 0x4 - Application Type				(Subtype 50.4.4)			*/
											/* 0x5 - ApplicationName Type										*/
											/* 0x6 - Vendor Type												*/
											
											/* 16 MSBs of idType indicate the packet format the client expects to receive its data in.	*/
											/* 0x0 - Ethernet packet (default)															*/
											/* 0x1 - IP/UDP packet payload																*/

	unsigned int clientId;					/* If Broadcast Type the Broadcast ID value is included. Broadcast IDs are assigned in [DSG].					*/
											/* If CA Type, the system_ID value is included.																	*/
											/* If Application ID, the application ID value is included. Application IDs are assigned by the MSO.			*/
											/* If Vendor Type, the vendor ID value is included.																*/
											/* For all other types, this field is set to '0'.																*/

	char clientName[MAXCLIENTNAMESIZE];		/* If Well Known MAC Address Type, the MAC address of the DSG Tunnel that the client is requesting is included.	*/
											/* If ApplicationName Type, this field includes a null-terminated text string.									*/
											/* For all other types, this field is set to '0'.																*/

	int handle;								/* This is an optional field to hold a unique client handle such as an RPC client handle. */
};

/****************************************************************************
** struct dsgccClientNotification
** This data structure is used by DSG-CC to to send notifications to clients. 
****************************************************************************/
struct dsgccClientNotification 
{
	int eventType;		// Type of the event
	int eventValue;		// Value to be passed
	int clientPort;		// Unique client identifier
};
#endif

/********************** Global Constants ************************************/

/* The bit mask constants for the keepalive status that is passed to external clients */
typedef enum DsgccClientStatus 
{
	DSGCC_CLIENT_REGISTERED		= 0x0001,
	DSG_IS_OPERATIONAL			= 0x0002,
	DSG_IS_IN_ADVANCED_MODE		= 0x0004,
	DSG_IS_IN_TWOWAY_MODE		= 0x0008,
	DSGCC_CLIENT_TUNNEL_OPEN	= 0x0010
} DsgccClientStatus;

typedef enum DsgccStatus 
{
	DSGCC_SUCCESS				= 0,
	DSGCC_ERROR					= 1,
	DSGCC_INVALID_PARAM			= 2,
	DSGCC_MAX_CLIENT_REACHED	= 3
} DsgccStatus;

typedef enum StatusCodes
{
	// Tunnel status
	DSGCC_TUNNEL_REQUEST_PENDING	= 1,	/* Tunnel request is being processed	*/
	DSGCC_TUNNEL_OPEN				= 2,	/* Tunnel is open						*/
	DSGCC_TUNNEL_NOT_AVAILABLE		= 3,	/* Tunnel cannot be opened				*/
	DSGCC_TUNNEL_PENDING			= 4,	/* eCM is scanning.						*/

	// DSG status
	DSGCC_TWOWAYMODE_CHANGE			= 5,
	DSGCC_ADVANCEMODE_CHANGE		= 6
} StatusCodes;

/********************** DSG-CC CLIENT API ****************************************************************************/
 
/*********************************************************************************************************************
 syntax			-	DsgccStatus dsgcc_RegisterClient(struct dsgClientRegInfo*  clientRegInfo); 

 description	-	This function tells the DSG-CC to begin forwarding DSG tunnel traffic to the client.

 parameter(s)	-	clientRegInfo   contains information for DSG Client Controller to find the correct 
									DSG tunnel for the client and establish a pipe to forward the tunnel 
									data to the client.

 value returned	-	DSGCC_SUCCESS,				Client is registered and tunnel is pending
					DSGCC_ERROR 				Unspecified errors
					DSGCC_INVALID_PARAM			Invalid client/tunnel parameters
					DSGCC_MAX_CLIENT_REACHED	Max number of clients reached. Tunnel request cannot be registered
*********************************************************************************************************************/
DsgccStatus dsgcc_RegisterClient( struct dsgClientRegInfo*  clientRegInfo ); 

/*********************************************************************************************************************
 syntax			-	DsgccStatus dsgcc_RegisterClientSetClassifier( struct dsgClientRegInfo*  clientRegInfo,
																	   dsg_classifier* dsgClassifier)
 

 description	-	This function tells the DSG-CC to begin forwarding DSG tunnel traffic that 
					matches classifier settings specified by the client

 parameter(s)	-	clientRegInfo   contains information for DSG Client Controller to find the correct 
									DSG tunnel for the client and establish a pipe to forward the tunnel 
									data to the client.
					dsg_classifier	contains specific classification rules to be applied to the tunnel 
									traffic the client requested

 value returned	-	DSGCC_SUCCESS,				Client is registered and tunnel is pending
					DSGCC_ERROR 				Unspecified errors
					DSGCC_INVALID_PARAM			Invalid client/tunnel parameters
					DSGCC_MAX_CLIENT_REACHED	Max number of clients reached. Tunnel request cannot be registered
*********************************************************************************************************************/
DsgccStatus dsgcc_RegisterClientSetClassifier( struct dsgClientRegInfo*  clientRegInfo, dsg_classifier* dsgClassifier);

/*********************************************************************************************************************
 syntax			-	DsgccStatus dsgcc_UnregisterClient(struct dsgClientRegInfo*  clientRegInfo) 

 description	-	This function notifies the DSG-CC that the client no longer wants to receive tunnel traffic.

 parameter(s)	-	clientRegInfo   contains information for DSG Client Controller to remove the client 
									from its list and stop forwarding tunnel data to it. 
										

 value returned	-	DSGCC_SUCCESS	means that the client was deregistered successfully and will no longer be sent tunnel traffic
					DSGCC_ERROR 	Unspecified errors
*********************************************************************************************************************/
DsgccStatus dsgcc_UnregisterClient( struct dsgClientRegInfo*  clientRegInfo ); 

/*********************************************************************************************************************
 syntax			-	DsgccStatus dsgcc_KeepAliveClient(struct dsgClientRegInfo*  clientRegInfo) 

 description	-	Each client may call this function to check its status and the status 
					of the DSG Client Controller, which returns a bit mask. 

 parameter(s)	-	clientRegInfo   contains information for DSG Client Controller to identify the 
									client and return information about client's registration and tunnel status.
										
 value returned	-	This is a 32 -bit value 

					16 LSBs of the return value is a bit-mask and shows current DSG and client status.

					DSGCC_CLIENT_REGISTERED		= 0x0001,
					DSG_IS_OPERATIONAL			= 0x0002,
					DSG_IS_IN_ADVANCED_MODE		= 0x0004,
					DSG_IS_IN_TWOWAY_MODE		= 0x0008,
					DSGCC_CLIENT_TUNNEL_OPEN	= 0x0010, 

					16 MSBs of the return value indicates the current status of the client's tunnel.

					DSGCC_TUNNEL_REQUEST_PENDING	= 1,	
					DSGCC_TUNNEL_OPEN				= 2,
					DSGCC_TUNNEL_NOT_AVAILABLE		= 3,
					DSGCC_TUNNEL_PENDING			= 4
*********************************************************************************************************************/				
unsigned int dsgcc_KeepAliveClient( struct dsgClientRegInfo*  clientRegInfo ); 

/*********************************************************************************************************************
 syntax			-	void dsgcc_ClientNotification(struct dsgccClientNotification *clientNotification) 

 description	-	DSG-CC calls this function to send tunnel/DSG status notifications to the client.
					DSG-CC sends tunnel status notifications only when the following changes in the tunnel status occur:
					Tunnel request is pending (initial state) --> tunnel is open 
					Tunnel request is pending (initial state) --> tunnel is not available
					tunnel is open --> tunnel is not available  (DCD may have changed and the tunnel is not available any more)
					tunnel is not available-->tunnel is open   (DCD may have changed and the tunnel is available now)

 parameter(s)	-	clientNotification   contains event type and value if applicable.
											 										
 value returned	-	n/a
**********************************************************************************************************************/			
void dsgcc_ClientNotification(struct dsgccClientNotification *clientNotification);

/*************************** Mapping of old API for compatibility*****************************************************/ 
#define dsgcc_RegisterClient(x) registerdsgclient(x) 
#define dsgcc_RegisterClientSetClassifier(x, y) registerdsgclientwithclassifier(x, y) 
#define dsgcc_UnregisterClient(x) unregisterdsgclient(x) 
#define dsgcc_KeepAliveClient(x) keepalivedsgclient(x) 

/*********************************************************************************************************************/
#if defined __cplusplus
}
#endif

#endif
