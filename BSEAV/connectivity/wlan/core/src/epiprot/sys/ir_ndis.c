/*++
//*
//* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Epigram, Inc.;
//* the contents of this file may not be disclosed to third parties, copied or
//* duplicated in any form, in whole or in part, without the prior written
//* permission of Epigram, Inc.
//*
//* $Id$
//*
*/

#include <ndis.h>
#include <typedefs.h>
#include <ntddk.h>
#include <ntiologc.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include "irelay.h"
#include "epivers.h"
#include "vendor.h"
#include "epiprot.h"


NTSTATUS
IoctlHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP irp
);

NTSTATUS
OpenHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP irp
);

NTSTATUS
CloseHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP irp
);

extern VOID
CancelTransferIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP unusedIrp
);

NDIS_PHYSICAL_ADDRESS           HighestAcceptableMax = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);
NDIS_HANDLE                     DriverHandle = NULL;
NDIS_MEDIUM                     MediumArray[3] =
                                    {
                                        NdisMedium802_3,    // Ethernet
                                        NdisMedium802_5,    // Token-ring
                                        NdisMediumFddi      // Fddi
                                    };

//
// Protocol proto-types
//
VOID
PtOpenAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    );

VOID
PtCloseAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    );

extern
VOID
PtResetComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    );

VOID
PtRequestComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  NDIS_STATUS             Status
    );

extern
VOID
PtStatus(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    );

extern
VOID
PtStatusComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

VOID
PtSendComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            pPacket,
    IN  NDIS_STATUS             Status
    );

extern
VOID
PtTransferDataComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status,
    IN  UINT                    BytesTransferred
    );

extern
NDIS_STATUS
PtReceive(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookAheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    );

extern
VOID
PtReceiveComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    );

extern
INT
PtReceivePacket(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            pPacket
    );

extern
VOID
PtBindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    );

extern
VOID
PtUnbindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             UnbindContext
    );
    
VOID
PtUnload(
    VOID
    );

extern VOID InitializeDeviceExtension(
	PVOID DeviceExtension, 
	NDIS_HANDLE ProtHandle
);

extern 
POPENRECORD 
LookupOpenRecord(
	IN PFILE_OBJECT FileObject
);

extern PDEVICE_OBJECT GlobalDeviceObject;


#ifdef NTDDKSIM
/*
 * If we are compiled with the simulator, provide a unique name
 * for the DriverEntry routine.
 */
#define DRIVERENTRY ELAYERDriverEntry
#define DLLEXPORT __declspec(dllexport)
#else
#define DRIVERENTRY DriverEntry
NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath );
#pragma NDIS_INIT_FUNCTION(DriverEntry)
#define DLLEXPORT
#endif

DLLEXPORT
NTSTATUS
DRIVERENTRY(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath
    )
{
    NDIS_STATUS                     Status;
    NDIS_PROTOCOL_CHARACTERISTICS   PChars;
    PNDIS_CONFIGURATION_PARAMETER   Param;
    NDIS_STRING                     Name;
    PDEVICE_OBJECT DeviceObject = NULL;
    NDIS_HANDLE                     WrapperHandle;
    NDIS_STRING                     Symlink;
    NDIS_STRING                     DeviceString;
	NDIS_HANDLE                     ProtHandle = NULL;

    //
    // Now register the protocol.
    //
    NdisZeroMemory(&PChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    PChars.MajorNdisVersion = 4;
    PChars.MinorNdisVersion = 0;

    //
    // Make sure the protocol-name matches the service-name under which this protocol is installed.
    // This is needed to ensure that NDIS can correctly determine the binding and call us to bind
    // to miniports below.
    //
    NdisInitUnicodeString(&Name, RELAY_WIDE_PROT_NAME); // Protocol name
    PChars.Name = Name;
    PChars.OpenAdapterCompleteHandler = PtOpenAdapterComplete;
    PChars.CloseAdapterCompleteHandler = PtCloseAdapterComplete;
    PChars.SendCompleteHandler = PtSendComplete;
    PChars.TransferDataCompleteHandler = PtTransferDataComplete;
    
    PChars.ResetCompleteHandler = PtResetComplete;
    PChars.RequestCompleteHandler = PtRequestComplete;
    PChars.ReceiveHandler = PtReceive;
    PChars.ReceiveCompleteHandler = PtReceiveComplete;
    PChars.StatusHandler = PtStatus;
    PChars.StatusCompleteHandler = PtStatusComplete;
    PChars.BindAdapterHandler = PtBindAdapter;
    PChars.UnbindAdapterHandler = PtUnbindAdapter;
    PChars.UnloadHandler = PtUnload;

    //
    // See comment above.
    //
	//    PChars.ReceivePacketHandler = PtReceivePacket;

    NdisRegisterProtocol(&Status,
                         &ProtHandle,
                         &PChars,
                         sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

    ASSERT(Status == NDIS_STATUS_SUCCESS);
    
        //*******************************************************
        //*                                                     *
        //* RGB: Create a device.                               *
        //*                                                     *
        //*******************************************************    
    NdisInitializeString( &DeviceString, RELAY_DEVICE_NAME );
    Status = IoCreateDevice( DriverObject, sizeof(IR_DEVICE_EXT),
        &DeviceString, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject );

	GlobalDeviceObject = DeviceObject;

	// Create a symbolic link for this driver so that 
	// it is visible in user space.
	//
    NdisInitializeString( &Symlink, RELAY_DOS_DEVICE_NAME );
    Status = IoCreateSymbolicLink( &Symlink, &DeviceString );
    if ( Status != STATUS_SUCCESS ) {
        NdisFreeString( Symlink );
        NdisFreeString( DeviceString );
        NdisDeregisterProtocol(&Status, ProtHandle);
        return Status;
    }
    
	// Initialize the device extension. 
	//
	InitializeDeviceExtension(DeviceObject->DeviceExtension, ProtHandle);

    NdisFreeString( Symlink );
    NdisFreeString( DeviceString );
    
        //*******************************************************
        //*                                                     *
        //* RGB: Set the IOCTL handler.                         *
        //*                                                     *
        //*******************************************************    
    DriverObject->MajorFunction[IRP_MJ_CREATE] = OpenHandler;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = CloseHandler;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlHandler;


    return(Status);
}

/*++

Routine Description:

    Called by NDIS to bind to a miniport below.

Arguments:

    Status          - Return status of bind here.
    BindContext     - Can be passed to NdisCompleteBindAdapter if this call is pended.
    DeviceName      - Device name to bind to. This is passed to NdisOpenAdapter.
    SystemSpecific1 - Can be passed to NdisOpenProtocolConfiguration to read per-binding information
    SystemSpecific2 - Unused for NDIS 4.0.


Return Value:

    NDIS_STATUS_PENDING if this call is pended. In this case call NdisCompleteBindAdapter to complete.
    Anything else       Completes this call synchronously

--*/
VOID
PtBindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            DeviceName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    )
{

}

/*++

Routine Description:

    Completion routine for NdisOpenAdapter issued from within the PtBindAdapter. Simply
    unblock the caller.

Arguments:

    ProtocolBindingContext  Pointer to the adapter
    Status                  Status of the NdisOpenAdapter call
    OpenErrorStatus         Secondary status (ignored by us).

Return Value:

    None

--*/
VOID
PtOpenAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             NdisStatus,
    IN  NDIS_STATUS             OpenErrorStatus
    )
{
	PFILE_OBJECT	FileObject = (PFILE_OBJECT) ProtocolBindingContext;
	POPENRECORD		pRecord;
	PIRP			irp;
	NTSTATUS		Status;
	PNDIS_OPEN_BLOCK nob;

	// REMIND this should be protected by a spin lock.
	pRecord = LookupOpenRecord( FileObject );
	if ( pRecord == NULL ) {
		// did not find this as an open record, forget it.
		return;
	}

	nob = (PNDIS_OPEN_BLOCK) pRecord->BindingHandle;
	NdisAcquireSpinLock( &pRecord->Lock );
	irp = pRecord->PendingBind;
	pRecord->PendingBind = NULL;
	NdisReleaseSpinLock( &pRecord->Lock );

	if (!irp) {
		// Did not find irp in list of pending irps.
		// should really close the adapter here.
		return;
	} else if (irp->Cancel) {
		//
		// This IRP is cancelled.
		//

		// should really close the adapter here.

		//
		// Complete the request 
		IO_COMPLETE_REQUEST( irp, STATUS_CANCELLED, 0 );
		irp = NULL;
	} else {
		IoSetCancelRoutine(irp, NULL);

		switch (NdisStatus) {
		case NDIS_STATUS_SUCCESS:
			Status = STATUS_SUCCESS;
			break;
		default:
			// this is a deperate move in the face of an unknown NDIS error.
			// We really should return some innoccuous NT status error 
			// and somehow find a way to return the NDIS error value 
			// to the user for further decoding.
			Status = STATUS_UNEXPECTED_NETWORK_ERROR;
		} 
		
		IO_COMPLETE_REQUEST( irp, Status, 0 );
	}
}


VOID
PtUnbindAdapter(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             UnbindContext
    )
/*++

Routine Description:

    Called by NDIS when we are required to unbind to the adapter below.

Arguments:

    Status                  Placeholder for return status
    ProtocolBindingContext  Pointer to the adapter structure
    UnbindContext           Context for NdisUnbindComplete() if this pends

Return Value:

    Status for NdisIMDeinitializeDeviceContext

--*/
{

}

VOID
PtUnload(
    VOID
    )
{
    NDIS_STATUS Status;
    PIR_DEVICE_EXT pDevExt;

	pDevExt = GlobalDeviceObject->DeviceExtension;
    NdisDeregisterProtocol(&Status, pDevExt->ProtHandle);
    
    NdisFreeSpinLock( &pDevExt->ListLock );
    NdisFreeSpinLock( &pDevExt->TxListLock );

}

VOID
PtCloseAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             NdisStatus
)
{
	PFILE_OBJECT	FileObject = (PFILE_OBJECT) ProtocolBindingContext;
	POPENRECORD		pRecord;
	PIRP			irp;
	NTSTATUS		Status;

	// REMIND this should be protected by a spin lock.
	pRecord = LookupOpenRecord( FileObject );
	if ( pRecord == NULL ) {
		// did not find this as an open record, forget it.
		return;
	}

	ASSERT(pRecord->FileObject == FileObject);
	NdisAcquireSpinLock( &pRecord->Lock );
	irp = pRecord->PendingUnbind;
	pRecord->PendingUnbind = NULL;
	NdisReleaseSpinLock( &pRecord->Lock );

	pRecord->BindingHandle = 0;

	if (!irp) {
		// Did not find irp in list of pending irps.
		// should really close the adapter here.
		return;
	} else if (irp->Cancel) {
		//
		// This IRP is cancelled.
		//

		// should really close the adapter here.

		//
		// Complete the request 
		IO_COMPLETE_REQUEST( irp, STATUS_CANCELLED, 0 );
		irp = NULL;
	} else {
		IoSetCancelRoutine(irp, NULL);

		switch (NdisStatus) {
		case NDIS_STATUS_SUCCESS:
			Status = STATUS_SUCCESS;
			break;
		default:
			// this is a deperate move in the face of an unknown NDIS error.
			// We really should return some innoccuous NT status error 
			// and somehow find a way to return the NDIS error value 
			// to the user for further decoding.
			Status = STATUS_UNEXPECTED_NETWORK_ERROR;
		} 
		
		IO_COMPLETE_REQUEST( irp, Status, 0 );
	}
}


VOID
PtResetComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
    )
{

    //
    // We never issue a reset, so we should not be here.
    //
    ASSERT (0);
}

/*
  Completion handler for a perviously issued ndis request that was
  sent down to one of the adapters we are managing.  

  These requests may be sent in response to a relay OID from an
  application, or the irelay driver itself may issue ndis requests if
  it needs some information from a lower-level adapter, like mac
  address, etc.
*/
VOID
PtRequestComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  NDIS_STATUS             NdisStatus
    )
{
    PFILE_OBJECT    FileObject = (PFILE_OBJECT) ProtocolBindingContext;
    POPENRECORD     pRecord;
	PNDIS_REQUEST	pRequest;
	UINT			nbytes;
	PLIST_ENTRY     pHead, pTemp;
	PIRP			irp;
	NTSTATUS		Status;

	// REMIND this should be protected by a spin lock.
	pRecord = LookupOpenRecord( FileObject );
	if ( pRecord == NULL ) {
		// did not find this as an open record, forget it.
		return;
	}

	// remove it from the list of pending irps
	NdisAcquireSpinLock( &pRecord->Lock );
	pHead = &(pRecord->PendingRelays);
	irp = NULL;
	for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
		irp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
		pRequest = *((PNDIS_REQUEST*) &irp->Tail.Overlay.DriverContext[0]);
		if (pRequest == NdisRequest) {
			RemoveEntryList(pTemp);
			break;
		}
		irp = NULL;
	}
	NdisReleaseSpinLock( &pRecord->Lock );
	
	if (irp == NULL) {
		// Did not find irp in list of pending irps.
		NdisFreeMemory( NdisRequest, sizeof( NDIS_REQUEST ), 0 );
		return;
	}

	if (irp->Cancel) {
		//
		// This IRP is cancelled.
		//
		
		//
		// Complete the request 
		IO_COMPLETE_REQUEST( irp, STATUS_CANCELLED, 0 );
		irp = NULL;
	} else {
		IoSetCancelRoutine(irp, NULL);

		switch (NdisStatus) {
		case NDIS_STATUS_SUCCESS:
			if ( NdisRequest->RequestType == NdisRequestQueryInformation ) {
				nbytes = NdisRequest->DATA.QUERY_INFORMATION.BytesWritten;
			} else {
				 nbytes= NdisRequest->DATA.SET_INFORMATION.BytesRead;
			}
			nbytes += sizeof(RelayHeader);
			Status = STATUS_SUCCESS;
			break;
		case NDIS_STATUS_INVALID_LENGTH:
		case NDIS_STATUS_BUFFER_TOO_SHORT:
			Status = STATUS_MORE_PROCESSING_REQUIRED;
			if ( NdisRequest->RequestType == NdisRequestQueryInformation ) {
				nbytes = NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
			} else {
				nbytes = NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
			}
			nbytes += sizeof(RelayHeader);
			break;
        case NDIS_STATUS_INVALID_OID:
			nbytes = 0;
			Status = STATUS_INVALID_DEVICE_REQUEST;
			break;
        case NDIS_STATUS_INVALID_DATA:
			nbytes = 0;
			Status = STATUS_INVALID_PARAMETER;
			break;
		default:
			// this is a deperate move in the face of an unknown NDIS error.
			// We really should return some innoccuous NT status error 
			// and somehow find a way to return the NDIS error value 
			// to the user for further decoding.
			nbytes = 0;
			Status = STATUS_UNEXPECTED_NETWORK_ERROR;
		} 

		IO_COMPLETE_REQUEST( irp, Status, nbytes );
	}

	// remove from the list of stored IRPs 
	NdisFreeMemory( NdisRequest, sizeof( NDIS_REQUEST ), 0 );

	return;
}


VOID
PtStatus(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    )
/*++

Routine Description:

    Status handler for the lower-edge (protocol).

Arguments:

    ProtocolBindingContext  Pointer to the adapter structure
    GeneralStatus           Status code
    StatusBuffer            Status buffer
    StatusBufferSize        Size of the status buffer

Return Value:

    None

--*/
{

}


VOID
PtStatusComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{

}



//***********************************************************************
//*                                                                     *
//* VOID PtSendComplete(                                                *
//*     IN  NDIS_HANDLE     ProtocolBindingContext,                     *
//*     IN  PNDIS_PACKET    pPacket,                                    *
//*     IN  NDIS_STATUS     Status )                                    *
//*                                                                     *
//* Description:                                                        *
//* Called when a packet has been transmitted by the underlying         *
//* miniport driver.  We don't care about the status and will just      *
//* free all of the packet resources.                                   *
//*                                                                     *
//* Parameters:                                                         *
//* ProtocolBindingContext: the protocol instance on which pkt sent     *
//* pPacket:                pointer to the packet sent                  *
//* Status:                 status of the send                          *
//*                                                                     *
//* Returns:                                                            *
//* nothing                                                             *
//*                                                                     *
//***********************************************************************
VOID
PtSendComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            NdisPacket,
    IN  NDIS_STATUS             NdisStatus
    )
{
	PIR_DEVICE_EXT	pDevExt = GlobalDeviceObject->DeviceExtension;
    PFILE_OBJECT    FileObject = (PFILE_OBJECT) ProtocolBindingContext;
    POPENRECORD     pRecord;
	PLIST_ENTRY     pHead, pTemp;
	PIRP			irp;
	NTSTATUS		Status;
	PNDIS_PACKET    pPacket;
	PNDIS_BUFFER    NdisBuffer;
	PVOID           pPktBuf;
	ULONG			PktBufLen; 

	// REMIND this should be protected by a spin lock.
	pRecord = LookupOpenRecord( FileObject );
	if ( pRecord == NULL ) {
		// did not find this as an open record, forget it.
		return;
	}

	// remove it from the list of pending irps
	NdisAcquireSpinLock( &pRecord->Lock );
	pHead = &(pRecord->PendingSends);
	irp = NULL;
	for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
		irp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
		pPacket = *((PNDIS_PACKET*) &irp->Tail.Overlay.DriverContext[0]);
		if (pPacket == NdisPacket) {
			RemoveEntryList(pTemp);
			break;
		}
		irp = NULL;
	}
	NdisReleaseSpinLock( &pRecord->Lock );
	
	// check whether the irp was still on a list of pending irps.
	if (irp) {
		if (irp->Cancel) {
			//
			// This IRP is cancelled.
			//

			//
			// Complete the request 
			IO_COMPLETE_REQUEST( irp, STATUS_CANCELLED, 0 );
			irp = NULL;
		} else {
			IoSetCancelRoutine(irp, NULL);

			NdisQueryPacket( NdisPacket, NULL, NULL, &NdisBuffer, NULL );
			NdisQueryBuffer( NdisBuffer, &pPktBuf, &PktBufLen );
			NdisFreeBuffer( NdisBuffer );
			NdisReinitializePacket( NdisPacket );

			pDevExt = GlobalDeviceObject->DeviceExtension;
			PutPacket( pDevExt, NdisPacket );

			switch (NdisStatus) {
			case NDIS_STATUS_SUCCESS:
				Status = STATUS_SUCCESS;
				break;
			default:
				// this is a deperate move in the face of an unknown NDIS error.
				// We really should return some innoccuous NT status error 
				// and somehow find a way to return the NDIS error value 
				// to the user for further decoding.
				Status = STATUS_UNEXPECTED_NETWORK_ERROR;
			} 

			IO_COMPLETE_REQUEST( irp, Status, 0 );
		}
	}
}

VOID
PtTransferDataComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            NdisPacket,
    IN  NDIS_STATUS             NdisStatus,
    IN  UINT                    BytesTransferred
    )

{
	PIR_DEVICE_EXT	pDevExt = GlobalDeviceObject->DeviceExtension;
    PFILE_OBJECT    FileObject = (PFILE_OBJECT) ProtocolBindingContext;
    POPENRECORD     pRecord;
	PLIST_ENTRY     pHead, pTemp;
	PIRP			irp;
	NDIS_STATUS		Status;
	PNDIS_PACKET    pPacket;
	PNDIS_BUFFER    NdisBuffer;
	UINT 			HeaderBufferSize;

return;
	// REMIND this should be protected by a spin lock.
	pRecord = LookupOpenRecord( FileObject );
	if ( pRecord == NULL ) {
		// did not find this as an open record, forget it.
		return;
	}

	// remove it from the list of pending irps
	NdisAcquireSpinLock( &pRecord->Lock );
	pHead = &(pRecord->PendingTransfers);
	irp = NULL;
	for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
		irp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
		pPacket = *((PNDIS_PACKET*) &irp->Tail.Overlay.DriverContext[0]);
		if (pPacket == NdisPacket) {
			HeaderBufferSize = (UINT)(uintptr)irp->Tail.Overlay.DriverContext[1];
			RemoveEntryList(pTemp);
			break;
		}
		irp = NULL;
	}
	NdisReleaseSpinLock( &pRecord->Lock );
	
	// check whether the irp was still on a list of pending irps.
	if (irp) {
		if (irp->Cancel) {
			//
			// This IRP is cancelled.
			//

			//
			// Complete the request 
			IO_COMPLETE_REQUEST( irp, STATUS_CANCELLED, 0 );
			irp = NULL;
		} else {
			IoSetCancelRoutine(irp, NULL);

			NdisQueryPacket( NdisPacket, NULL, NULL, &NdisBuffer, NULL );
			NdisFreeBuffer( NdisBuffer );
			NdisFreePacket( NdisPacket );

			switch (NdisStatus) {
			case NDIS_STATUS_SUCCESS:
				Status = STATUS_SUCCESS;
				break;
			default:
				// this is a deperate move in the face of an unknown NDIS error.
				// We really should return some innoccuous NT status error 
				// and somehow find a way to return the NDIS error value 
				// to the user for further decoding.
				Status = STATUS_UNEXPECTED_NETWORK_ERROR;
			} 

			IO_COMPLETE_REQUEST( irp, Status, BytesTransferred + HeaderBufferSize);
		}
	}
}



NDIS_STATUS
PtReceive(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookAheadBuffer,
    IN  UINT                    LookAheadBufferSize,
    IN  UINT                    PacketSize
    )
{
	PFILE_OBJECT	FileObject = (PFILE_OBJECT) ProtocolBindingContext;
	PIO_STACK_LOCATION  iostack;
	PCHAR                iob;
	UINT                 inlen, outlen;
	POPENRECORD		pRecord;
	NDIS_STATUS		Status;
	PLIST_ENTRY		pe;
	USHORT          EtherType, LSType;
	PIRP            pIrp, tpIrp;
	PIR_DEVICE_EXT  pDevExt = GlobalDeviceObject->DeviceExtension;
	PNDIS_PACKET 	NdisPacket;
	PNDIS_BUFFER 	NdisBuffer;
	UINT 			BytesTransferred;
	PLIST_ENTRY		pHead, pTemp;

	// REMIND this should be protected by a spin lock.
	pRecord = LookupOpenRecord( FileObject );
	if ( pRecord == NULL ) {
		// did not find this as an open record, forget it.
		return NDIS_STATUS_NOT_ACCEPTED;
	}

	EtherType = ((PUCHAR)HeaderBuffer)[12];
	EtherType <<= 8;
	EtherType |= ((PUCHAR)HeaderBuffer)[13];

	LSType = ((PUCHAR)HeaderBuffer)[14];
	LSType <<= 8;
	LSType |= ((PUCHAR)HeaderBuffer)[15];

	// check for ILCP ethertype
	if ( (EtherType == (USHORT)ETHER_TYPE_BRCM) || (EtherType == (USHORT)ETHER_TYPE_802_1X) ) {

		pIrp = NULL;
		do {
			NdisAcquireSpinLock( &pRecord->Lock );
			pe = NULL;
			if (!IsListEmpty( &pRecord->PendingReads )) 
				pe = RemoveHeadList( &pRecord->PendingReads );
			NdisReleaseSpinLock( &pRecord->Lock );

			if ( pe ) {
				pIrp = CONTAINING_RECORD(pe, IRP, Tail.Overlay.ListEntry);

				if (pIrp->Cancel) {
					//
					// This IRP is cancelled.
					//

					//
					// Complete the request 
					IO_COMPLETE_REQUEST( pIrp, STATUS_CANCELLED, 0 );
					pIrp = NULL;
				} else {
					IoSetCancelRoutine(pIrp, NULL);
					iostack = IoGetCurrentIrpStackLocation(pIrp);
					iob = pIrp->AssociatedIrp.SystemBuffer;
					outlen = iostack->Parameters.DeviceIoControl.OutputBufferLength;
					if ( outlen < HeaderBufferSize) {
						HeaderBufferSize = outlen;
					}
					NdisMoveMemory( (PVOID)iob, HeaderBuffer, 
									HeaderBufferSize );
					iob += HeaderBufferSize;

					if (PacketSize <= LookAheadBufferSize) {
						if ( (outlen - HeaderBufferSize) <  LookAheadBufferSize) {
							LookAheadBufferSize = outlen - HeaderBufferSize;
						}

						NdisMoveMemory( (PVOID)iob, LookAheadBuffer, 
										LookAheadBufferSize );
						IO_COMPLETE_REQUEST( pIrp, STATUS_SUCCESS, 
										 HeaderBufferSize + LookAheadBufferSize );
					} else {
						/* LATER: if performance is critical, we should maintain our own packet 
						 * pool and use ReInitialize rather than alloc/free for every packet */
						NdisAllocatePacket(&Status, &NdisPacket, pDevExt->RxPacketPoolHandle);
						if (Status != NDIS_STATUS_SUCCESS) {
							IO_COMPLETE_REQUEST( pIrp, Status, 0); 
							break;
						}

						if ( (outlen - HeaderBufferSize) <  PacketSize) {
							PacketSize = outlen - HeaderBufferSize;
						}
						NdisAllocateBuffer(&Status, &NdisBuffer, 
							pDevExt->RxBufferPoolHandle, iob, 
							PacketSize );
						if (Status != NDIS_STATUS_SUCCESS) {
							NdisFreePacket( NdisPacket );
							IO_COMPLETE_REQUEST( pIrp, Status, 0); 
							break;
						}

						NdisChainBufferAtFront(NdisPacket, NdisBuffer);

						NdisAcquireSpinLock( &pRecord->Lock );
						NdisZeroMemory( &pIrp->Tail.Overlay.ListEntry, 
							sizeof(LIST_ENTRY) );
						InsertTailList(&pRecord->PendingTransfers, 
							&pIrp->Tail.Overlay.ListEntry);
						*((PNDIS_PACKET*) &pIrp->Tail.Overlay.DriverContext[0]) = NdisPacket;
						(UINT) (uintptr) pIrp->Tail.Overlay.DriverContext[1] = HeaderBufferSize;
						IoSetCancelRoutine(pIrp, CancelTransferIrp);
						NdisReleaseSpinLock( &pRecord->Lock );

						NdisTransferData( &Status, pRecord->BindingHandle,
							MacReceiveContext, 0, PacketSize,
							NdisPacket, &BytesTransferred);

						if (Status == NDIS_STATUS_SUCCESS )  {
							NdisFreeBuffer( NdisBuffer );
							NdisFreePacket( NdisPacket );

							NdisAcquireSpinLock( &pRecord->Lock );
							pHead = &(pRecord->PendingTransfers);
							for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
								tpIrp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
								if (tpIrp == pIrp) {
									RemoveEntryList(pTemp);
									break;
								}
							}
							IoSetCancelRoutine(pIrp, NULL);
							NdisReleaseSpinLock( &pRecord->Lock );

							IO_COMPLETE_REQUEST( pIrp, STATUS_SUCCESS, 
										 HeaderBufferSize + BytesTransferred );
						} else if (Status == NDIS_STATUS_PENDING) {
							/* Do nothing. Will complete in PtTransferDataComplete */
							DbgPrint( "Pending receive of %s bytes in NdisTransferData", PacketSize);
						} else {
							DbgPrint( "NdisTransferData failed with error %d", Status);
							IO_COMPLETE_REQUEST( pIrp, Status, 0); 
						}
					}
				}
			}
		} while (pe && !pIrp);
	}

    return NDIS_STATUS_NOT_ACCEPTED;
}


VOID
PtReceiveComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext
    )
/*++

Routine Description:

    Called by the adapter below us when it is done indicating a batch of received buffers.

Arguments:

    ProtocolBindingContext  Pointer to our adapter structure.

Return Value:

    None

--*/
{
}
