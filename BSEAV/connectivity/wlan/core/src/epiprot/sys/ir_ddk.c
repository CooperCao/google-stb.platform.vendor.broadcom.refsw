/*++
//*
//* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Epigram, Inc.;
//* the contents of this file may not be disclosed to third parties, copied or
//* duplicated in any form, in whole or in part, without the prior written
//* permission of Epigram, Inc.
//*
//* $Id$
//*

Copyright (c) 1992  Microsoft Corporation

Module Name:

    protocol.c

Abstract:

    Ndis Intermediate Miniport driver sample. This is a passthru driver.

Author:

    Jameel Hyder    jameelh@microsoft.com

Environment:


Revision History:


--*/

#include <ndis.h>
#include <ntddk.h>
#include <typedefs.h>
#include <stdio.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
/*#include <error.h>*/
#include "irelay.h"
#include "epivers.h"
#include "epiprot.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)   (sizeof(a)/sizeof(a[0]))
#endif

static NTSTATUS BuildVersionInfo(PUCHAR buf, IN OUT PULONG len);
static NTSTATUS BuildAdapterList(PUCHAR buf, PULONG inoutlen);
static NTSTATUS BuildAdapterXList(PUCHAR buf, IN OUT PULONG len);
static NTSTATUS RelayOidHandler(PIRP irp);
static NTSTATUS RelayQueryHandler(PIRP irp);

PDEVICE_OBJECT GlobalDeviceObject;

//
// Protocol proto-types
//
extern
VOID
PtOpenAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
);

extern
VOID
PtCloseAdapterComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  NDIS_STATUS             Status
);

extern
VOID
PtRequestComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  NDIS_STATUS             Status
);

extern
VOID
PtSendComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            pPacket,
    IN  NDIS_STATUS             Status
);

VOID
CancelReadIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP unusedIrp	
);

VOID
CancelSendIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP unusedIrp	
);

VOID
CancelRelayIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP unusedIrp	
);

VOID
CancelBindIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP unusedIrp	
);

VOID
CancelUnbindIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP CancelIrp 	/* don't use this sez OSR */
);

NTSTATUS
BindHandler(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP irp
);

NTSTATUS
UnbindHandler(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP irp
);

NTSTATUS DumpHandler(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP irp
);

NTSTATUS ReadHandler(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP irp
);

NTSTATUS SendHandler(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP irp
);

extern  NDIS_PHYSICAL_ADDRESS           HighestAcceptableMax;
extern  NDIS_HANDLE                     DriverHandle;
extern  NDIS_MEDIUM                     MediumArray[3];

        //*******************************************************
        //*                                                     *
        //* Number of transmit packets we will have.            *
        //*                                                     *
        //*******************************************************
#define NUM_TX_PACKETS  64
#define NUM_RX_PACKETS  8
         
int ListCount(PLIST_ENTRY pHead)
{
	PLIST_ENTRY     pTemp;
	int count;

	count = 0;
	for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
		count++;
	}

	return count;
}


POPENRECORD LookupOpenRecord(IN PFILE_OBJECT FileObject)
{
	PIR_DEVICE_EXT	pDevExt = GlobalDeviceObject->DeviceExtension;
	PLIST_ENTRY     pHead, pTemp;
	POPENRECORD		pRecord = NULL;

	pHead = &(pDevExt->OpenRecords);
	for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
		pRecord = CONTAINING_RECORD(pTemp, OPENRECORD, ListEntry);
		if (pRecord->FileObject == FileObject) {
			break;
		}
		pRecord = NULL;
	}

	return pRecord;
}



NTSTATUS
IoctlHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP irp
)
{
	PIR_DEVICE_EXT pDevExt = DeviceObject->DeviceExtension;
	PIO_STACK_LOCATION  iostack;
	void                *iob;
	int                 outlen;
	int                 code;
	NTSTATUS            Status;
	ULONG               inoutlen;
    int i;
	ULONG bytesprocessed;

    iostack = IoGetCurrentIrpStackLocation(irp);
    iob = irp->AssociatedIrp.SystemBuffer;
    outlen = iostack->Parameters.DeviceIoControl.OutputBufferLength;
    code = iostack->Parameters.DeviceIoControl.IoControlCode;

    Status = STATUS_SUCCESS;
    
    switch (code) 
    {
        case IOCTL_BIND:
			Status = BindHandler(DeviceObject, irp);
            break;

        case IOCTL_UNBIND:
			Status = UnbindHandler(DeviceObject, irp);
            break;

        case IOCTL_VERSION:
			inoutlen = outlen;
			Status = BuildVersionInfo((UCHAR *)iob, &inoutlen);
			IO_COMPLETE_REQUEST( irp, Status, inoutlen );
			break;

        case IOCTL_NDIS_QUERY_GLOBAL_STATS:
			Status	 = RelayQueryHandler(irp);
			break;

        case IOCTL_OID_RELAY:
			Status = RelayOidHandler(irp);
			break;

        case IOCTL_PKT_TX_RELAY:
			Status = SendHandler(DeviceObject, irp);
            break;
            
        case IOCTL_PKT_RX_RELAY:
			Status = ReadHandler(DeviceObject, irp);
            break;

	    case IOCTL_IR_DUMP:
			Status = DumpHandler(DeviceObject, irp);
            break;

        default:
//            ASSERT( 0 );
            Status = STATUS_NOT_SUPPORTED;
            IO_COMPLETE_REQUEST( irp, Status, 0 );
            break;
    }

    return (Status);
}


/*
  if there is no record of this open, fail?
  lookup based on original file handle
  if bound, fail.
  bind to lower level adapter.
*/
NTSTATUS
BindHandler(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP irp
)
{
	PIO_STACK_LOCATION  iostack;
	void                *iob;
	int                 inlen, outlen;
	POPENRECORD		pRecord;
	NTSTATUS		Status = STATUS_SUCCESS;
	NDIS_STATUS		NdisStatus, open_status;
	PIR_DEVICE_EXT  pDevExt = DeviceObject->DeviceExtension;
	PBindRequest    pBind;
	ANSI_STRING		aDevice;
	UNICODE_STRING	uDevice;
	UINT			MediumIndex;
	CHAR			DevBuffer[128];

	// Do some validation before proceeding to open the lower-level adapter.
	pRecord = LookupOpenRecord( irp->Tail.Overlay.OriginalFileObject );
	if (pRecord == NULL) {
		// no record of this file handle...probably fatal
		Status = RPC_NT_INVALID_BINDING;
	} else {
		NdisAcquireSpinLock( &pRecord->Lock );
		if (pRecord->BindingHandle != 0) {
			// Fail if we already have a completed binding for this handle.
			Status = RPC_NT_INVALID_BINDING;
		} else if (pRecord->PendingBind != NULL) {
			// Fail if there is a pending bind for this file handle.
			Status = RPC_NT_INVALID_BINDING;
		} else {
			// Mark this irp as a pending bind for this file handle.
			IoSetCancelRoutine(irp, CancelBindIrp);
			IoMarkIrpPending(irp);
			pRecord->PendingBind = irp;
		}
		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (Status != STATUS_SUCCESS) {
		IO_COMPLETE_REQUEST( irp, Status, 0 );
	} else {
		// go ahead and issue a NdisOpenAdapter to the lower driver.
		iostack = IoGetCurrentIrpStackLocation(irp);
		iob = irp->AssociatedIrp.SystemBuffer;
		inlen = iostack->Parameters.DeviceIoControl.InputBufferLength;
		pBind = (PBindRequest) iob;

		sprintf( DevBuffer, "\\Device\\%s", pBind->name );
		RtlInitAnsiString( &aDevice, DevBuffer );
		RtlAnsiStringToUnicodeString( &uDevice, &aDevice, TRUE );

		// REMIND - need the mediaum array here...
		NdisOpenAdapter(&NdisStatus,
						&open_status,
						&pRecord->BindingHandle,
						&MediumIndex,
						MediumArray,
						ARRAYSIZE(MediumArray),
						pDevExt->ProtHandle,
						irp->Tail.Overlay.OriginalFileObject,
						&uDevice,
						0,
						NULL);
		if ( NdisStatus != NDIS_STATUS_PENDING) {
			PtOpenAdapterComplete(pRecord->FileObject, NdisStatus, 
								  open_status);
		}
		Status = STATUS_PENDING;

		RtlFreeUnicodeString( &uDevice );

	}
	return Status;
}


/*
  if there is no record of this open, fail?
  lookup based on original file handle
  if bound, unbind.
*/
NTSTATUS
UnbindHandler(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP irp
)
{
	POPENRECORD		pRecord;
	NTSTATUS		Status = STATUS_SUCCESS;
	NDIS_STATUS		NdisStatus;
	UINT			MediumIndex;

	// Do some validation before proceeding to open the lower-level adapter.
	pRecord = LookupOpenRecord( irp->Tail.Overlay.OriginalFileObject );
	if (pRecord == NULL) {
		// no record of this file handle...probably fatal
		Status = RPC_NT_INVALID_BINDING;
	} else {
		NdisAcquireSpinLock( &pRecord->Lock );
		if (pRecord->BindingHandle == 0) {
			// It is not bound.
			Status = RPC_NT_INVALID_BINDING;
		} else if (pRecord->PendingUnbind != NULL) {
			// Fail if there is a pending unbind for this file handle.
			Status = RPC_NT_INVALID_BINDING;
		} else {
			// Mark this irp as a pending unbind for this file handle.
			IoSetCancelRoutine(irp, CancelUnbindIrp);
			IoMarkIrpPending(irp);
			pRecord->PendingUnbind = irp;
		}
		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (Status != STATUS_SUCCESS) {
		IO_COMPLETE_REQUEST( irp, Status, 0 );
	} else {
		NdisCloseAdapter( &NdisStatus, pRecord->BindingHandle );
		if ( NdisStatus != NDIS_STATUS_PENDING) {
			PtCloseAdapterComplete(pRecord->FileObject, NdisStatus);
		}
		Status = STATUS_PENDING;

	}
	return Status;


}

NTSTATUS
OpenHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP irp
)
{
	PIR_DEVICE_EXT	pDevExt = DeviceObject->DeviceExtension;
	POPENRECORD     pRecord = NULL;

	NdisAllocateMemory(&pRecord, sizeof(OPENRECORD), 
					   0, HighestAcceptableMax);
	if (pRecord == NULL) {
		return STATUS_NO_MEMORY;
	}
	NdisZeroMemory( pRecord, sizeof(OPENRECORD) );

	NdisAllocateSpinLock( &pRecord->Lock );
	InitializeListHead( &pRecord->PendingRelays );
	InitializeListHead( &pRecord->PendingReads );
	InitializeListHead( &pRecord->PendingSends );
	InitializeListHead( &pRecord->PendingTransfers );
	pRecord->FileObject = irp->Tail.Overlay.OriginalFileObject;
		
	// REMIND - protect this with a lock...
	InsertTailList(&pDevExt->OpenRecords, &pRecord->ListEntry);

	IO_COMPLETE_REQUEST( irp, STATUS_SUCCESS, 0 );

	return STATUS_SUCCESS;
}

NTSTATUS
CloseHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP irp
)
{
	POPENRECORD pRecord;
	NTSTATUS    Status = STATUS_SUCCESS;
	NDIS_STATUS	NdisStatus;
	PFILE_OBJECT FileObject;

	// REMIND - protect this with a lock...
	FileObject = irp->Tail.Overlay.OriginalFileObject;
	pRecord = LookupOpenRecord( FileObject );
	if (pRecord) {
		RemoveEntryList(&pRecord->ListEntry);

		ASSERT(LookupOpenRecord(FileObject) == NULL);
		ASSERT(pRecord->FileObject == FileObject);
		ASSERT(pRecord->PendingBind == NULL);
		ASSERT(pRecord->PendingUnbind == NULL);
		ASSERT(IsListEmpty(&pRecord->PendingReads));
		ASSERT(IsListEmpty(&pRecord->PendingSends));
		ASSERT(IsListEmpty(&pRecord->PendingRelays));
		ASSERT(IsListEmpty(&pRecord->PendingTransfers));

		if (pRecord->BindingHandle != 0) {
			NdisCloseAdapter( &NdisStatus, pRecord->BindingHandle );
			switch (NdisStatus) {
			case NDIS_STATUS_PENDING:
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
		}
		NdisFreeMemory( pRecord, sizeof(OPENRECORD), 0);
	} else {
		printf("CloseHandler (%s:%d) could not find the specified handle\n", __FILE__, __LINE__);
		Status = RPC_NT_INVALID_BINDING;
	}

	IO_COMPLETE_REQUEST( irp, Status, 0 );

    return Status;
}


static NTSTATUS BuildVersionInfo(PUCHAR buf, IN OUT PULONG len)
{
	PVersionResponse pvr;
	ULONG need;

	// Calculate the needed space...
	// If the buffer passed to us is smaller than this,    
	// return error.                                       
	need = sizeof(VersionResponse);
	if ( *len < need ) {
		*len = need;
		return STATUS_MORE_PROCESSING_REQUIRED;
	}
	pvr = (PVersionResponse) buf;
	pvr->VersionMS = (EPI_MAJOR_VERSION << 16) | EPI_MINOR_VERSION;
	pvr->VersionLS = (EPI_RC_NUMBER << 16) | EPI_INCREMENTAL_NUMBER;

	*len = need;
	return STATUS_SUCCESS;
}

VOID
CancelBindIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP CancelIrp 	/* don't use this sez OSR */
)
{
	PIRP			irpToCancel;
	PIR_DEVICE_EXT	pDevExt;
	PLIST_ENTRY     pHead, pTemp;
	POPENRECORD		pRecord;

	// 
	// Release the system-wide cancel spin lock as soon as we can
	//
	IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

	pDevExt = DeviceObject->DeviceExtension;
	
	// Iterate through all Open records, looking for cancellable 
	// IRPs along the way.
	// this should be protected by a spin lock...

	pHead = &(pDevExt->OpenRecords);
	pTemp = pHead->Flink;
	irpToCancel = NULL;
	while (!irpToCancel && pTemp != pTemp->Flink) {

		pRecord = CONTAINING_RECORD(pTemp, OPENRECORD, ListEntry);

		NdisAcquireSpinLock( &pRecord->Lock );

		// Iterate through allthe pending read irps associates with 
		// this adapter, looking for one to cancel.
		//
		irpToCancel = pRecord->PendingBind;
		if (irpToCancel && irpToCancel->Cancel) {
			pRecord->PendingBind = NULL;
			NdisReleaseSpinLock( &pRecord->Lock );
			break;
		} else {
			irpToCancel = NULL;
		}

		NdisReleaseSpinLock( &pRecord->Lock );
		pTemp = pTemp->Flink;
	}

	if (irpToCancel) {
		//
		// We found the request to cancel
		//
		IoSetCancelRoutine(irpToCancel, NULL);

		IO_COMPLETE_REQUEST( irpToCancel, STATUS_CANCELLED, 0 );
	}
}


VOID
CancelUnbindIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP CancelIrp 	/* don't use this sez OSR */
)
{
	PIRP			irpToCancel;
	PIR_DEVICE_EXT	pDevExt;
	PLIST_ENTRY     pHead, pTemp;
	POPENRECORD		pRecord;

	// 
	// Release the system-wide cancel spin lock as soon as we can
	//
	IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

	pDevExt = DeviceObject->DeviceExtension;
	
	// Iterate through all Open records, looking for cancellable 
	// IRPs along the way.
	// this should be protected by a spin lock...

	pHead = &(pDevExt->OpenRecords);
	pTemp = pHead->Flink;
	irpToCancel = NULL;
	while (!irpToCancel && pTemp != pTemp->Flink) {

		pRecord = CONTAINING_RECORD(pTemp, OPENRECORD, ListEntry);

		NdisAcquireSpinLock( &pRecord->Lock );

		// Iterate through allthe pending read irps associates with 
		// this adapter, looking for one to cancel.
		//
		irpToCancel = pRecord->PendingUnbind;
		if (irpToCancel && irpToCancel->Cancel) {
			pRecord->PendingUnbind = NULL;
			NdisReleaseSpinLock( &pRecord->Lock );
			break;
		} else {
			irpToCancel = NULL;
		}

		NdisReleaseSpinLock( &pRecord->Lock );
		pTemp = pTemp->Flink;
	}

	if (irpToCancel) {
		//
		// We found the request to cancel
		//
		IoSetCancelRoutine(irpToCancel, NULL);

		IO_COMPLETE_REQUEST( irpToCancel, STATUS_CANCELLED, 0 );
	}
}

VOID
CancelReadIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP CancelIrp 	/* don't use this sez OSR */
)
{
	PIRP pIrp, irpToCancel;
	PIR_DEVICE_EXT pDevExt;
	PLIST_ENTRY     pORHead, pORTemp;
	PLIST_ENTRY     pHead, pTemp;
	POPENRECORD pRecord;

	// 
	// Release the system-wide cancel spin lock as soon as we can
	//
	IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

	pDevExt = DeviceObject->DeviceExtension;
	
	// REMIND - we can probably use the original file handle of the cancel irp
	// to quickly find the open record we are interested in.

	// Iterate through all Open records, looking for cancellable 
	// IRPs along the way.
	// this should be protected by a spin lock...

	irpToCancel = NULL;
	pORHead = &(pDevExt->OpenRecords);
	for (pORTemp=pORHead->Flink; pORTemp != pORHead; pORTemp=pORTemp->Flink) {
		pRecord = CONTAINING_RECORD(pORTemp, OPENRECORD, ListEntry);
		
		NdisAcquireSpinLock( &pRecord->Lock );
		
		// Iterate through allthe pending read irps associates with 
		// this adapter, looking for one to cancel.
		//
		pHead = &(pRecord->PendingReads);
		for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
			pIrp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
			if (pIrp->Cancel) {
				RemoveEntryList(pTemp);
				irpToCancel = pIrp;
				break;
			}
		}

		NdisReleaseSpinLock( &pRecord->Lock );
	}

	
	if (irpToCancel) {
		//
		// We found the request to cancel
		//
		IoSetCancelRoutine(irpToCancel, NULL);
		
		IO_COMPLETE_REQUEST( irpToCancel, STATUS_CANCELLED, 0 );
	} else {
		printf("failed to find read IRP to cancel!\n");
	}
}

VOID
CancelSendIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP CancelIrp 	/* don't use this sez OSR */
)
{
	PIR_DEVICE_EXT	pDevExt;
	PIRP			pIrp, irpToCancel;
	PLIST_ENTRY     pORHead, pORTemp;  /* Open Record list pointers. */
	PLIST_ENTRY     pHead, pTemp;      /* IRP list pointers */
	POPENRECORD		pRecord;
	PNDIS_PACKET	NdisPacket;
	PNDIS_BUFFER    NdisBuffer;
	PVOID           pPktBuf;
	ULONG			PktBufLen; 

	// 
	// Release the system-wide cancel spin lock as soon as we can
	//
	IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

	pDevExt = DeviceObject->DeviceExtension;
	
	// REMIND - we can probably use the original file handle of the cancel irp
	// to quickly find the open record we are interested in.

	// Iterate through all Open records, looking for cancellable 
	// IRPs along the way.
	// this should be protected by a spin lock...

	irpToCancel = NULL;
	pORHead = &(pDevExt->OpenRecords);
	for (pORTemp=pORHead->Flink; pORTemp != pORHead; pORTemp=pORTemp->Flink) {
		pRecord = CONTAINING_RECORD(pORTemp, OPENRECORD, ListEntry);
		
		NdisAcquireSpinLock( &pRecord->Lock );
		
		// Iterate through allthe pending read irps associates with 
		// this adapter, looking for one to cancel.
		//
		pHead = &(pRecord->PendingSends);
		for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
			pIrp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
			if (pIrp->Cancel) {
				RemoveEntryList(pTemp);
				irpToCancel = pIrp;
				break;
			}
		}

		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (irpToCancel) {
		//
		// We found the request to cancel
		//
		IoSetCancelRoutine(irpToCancel, NULL);

		// get pointer to the ndis packet from the irp
		NdisPacket = *((PNDIS_PACKET*) &irpToCancel->Tail.Overlay.DriverContext[0]);

		NdisQueryPacket( NdisPacket, NULL, NULL, &NdisBuffer, NULL );
		NdisQueryBuffer( NdisBuffer, &pPktBuf, &PktBufLen );
		NdisFreeBuffer( NdisBuffer );
		NdisReinitializePacket( NdisPacket );
		
		pDevExt = GlobalDeviceObject->DeviceExtension;
		PutPacket( pDevExt, NdisPacket );

		IO_COMPLETE_REQUEST( irpToCancel, STATUS_CANCELLED, 0 );
	} else {
		printf("failed to find write IRP to cancel!\n");
	}
}

VOID
CancelRelayIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP CancelIrp 	/* don't use this sez OSR */
)
{
	PIRP			pIrp, irpToCancel;
	PIR_DEVICE_EXT	pDevExt;
	PLIST_ENTRY     pORHead, pORTemp;  /* Open Record list pointers. */
	PLIST_ENTRY     pHead, pTemp;      /* IRP list pointers */
	POPENRECORD		pRecord;

	// 
	// Release the system-wide cancel spin lock as soon as we can
	//
	IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

	pDevExt = DeviceObject->DeviceExtension;
	
	// REMIND - we can probably use the original file handle of the cancel irp
	// to quickly find the open record we are interested in.

	// Iterate through all Open records, looking for cancellable 
	// IRPs along the way.
	// this should be protected by a spin lock...

	irpToCancel = NULL;
	pORHead = &(pDevExt->OpenRecords);
	for (pORTemp=pORHead->Flink; pORTemp != pORHead; pORTemp=pORTemp->Flink) {
		pRecord = CONTAINING_RECORD(pORTemp, OPENRECORD, ListEntry);
		
		NdisAcquireSpinLock( &pRecord->Lock );
		
		// Iterate through allthe pending read irps associates with 
		// this adapter, looking for one to cancel.
		//
		pHead = &(pRecord->PendingRelays);
		for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
			pIrp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
			if (pIrp->Cancel) {
				RemoveEntryList(pTemp);
				irpToCancel = pIrp;
				break;
			}
		}

		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (irpToCancel) {
		//
		// We found the request to cancel
		//
		IoSetCancelRoutine(irpToCancel, NULL);

		IO_COMPLETE_REQUEST( irpToCancel, STATUS_CANCELLED, 0 );
	} else {
		printf("failed to find relay IRP to cancel!\n");
	}
}

VOID
CancelTransferIrp(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP CancelIrp 	/* don't use this sez OSR */
)
{
	PIRP pIrp, irpToCancel;
	PIR_DEVICE_EXT pDevExt;
	PLIST_ENTRY     pORHead, pORTemp;
	PLIST_ENTRY     pHead, pTemp;
	POPENRECORD pRecord;

	// 
	// Release the system-wide cancel spin lock as soon as we can
	//
	IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

	pDevExt = DeviceObject->DeviceExtension;
	
	// REMIND - we can probably use the original file handle of the cancel irp
	// to quickly find the open record we are interested in.

	// Iterate through all Open records, looking for cancellable 
	// IRPs along the way.
	// this should be protected by a spin lock...

	irpToCancel = NULL;
	pORHead = &(pDevExt->OpenRecords);
	for (pORTemp=pORHead->Flink; pORTemp != pORHead; pORTemp=pORTemp->Flink) {
		pRecord = CONTAINING_RECORD(pORTemp, OPENRECORD, ListEntry);
		
		NdisAcquireSpinLock( &pRecord->Lock );
		
		// Iterate through allthe pending transfer irps associates with 
		// this adapter, looking for one to cancel.
		//
		pHead = &(pRecord->PendingTransfers);
		for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
			pIrp = CONTAINING_RECORD(pTemp, IRP, Tail.Overlay.ListEntry);
			if (pIrp->Cancel) {
				RemoveEntryList(pTemp);
				irpToCancel = pIrp;
				break;
			}
		}

		NdisReleaseSpinLock( &pRecord->Lock );
	}

	
	if (irpToCancel) {
		//
		// We found the request to cancel
		//
		IoSetCancelRoutine(irpToCancel, NULL);
		
		IO_COMPLETE_REQUEST( irpToCancel, STATUS_CANCELLED, 0 );
	} else {
		printf("failed to find transfer IRP to cancel!\n");
	}
}


/* REMIND - need to return some success or failure from this routine */
VOID InitializeDeviceExtension(PVOID DeviceExtension, NDIS_HANDLE ProtHandle)
{
	PIR_DEVICE_EXT	pDevExt;
    NDIS_STATUS		Status;
	PNDIS_PACKET	pPacket;
	ULONG			Counter;

	pDevExt = DeviceExtension;
	NdisZeroMemory(pDevExt, sizeof(IR_DEVICE_EXT));

	pDevExt->ProtHandle = ProtHandle;

    NdisAllocateSpinLock( &pDevExt->ListLock );
    NdisAllocateSpinLock( &pDevExt->TxListLock );
	NdisInitializeEvent( &pDevExt->TxEvent );
	InitializeListHead( &pDevExt->OpenRecords );

	//* Allocate packet and buffer pools.                   *
    NdisAllocatePacketPool( &Status, &pDevExt->TxPacketPoolHandle,
        NUM_TX_PACKETS, 16 );
    NdisAllocateBufferPool( &Status, &pDevExt->TxBufferPoolHandle,
        NUM_TX_PACKETS );
    NdisAllocatePacketPool( &Status, &pDevExt->RxPacketPoolHandle,
        NUM_RX_PACKETS, 16 );
    NdisAllocateBufferPool( &Status, &pDevExt->RxBufferPoolHandle,
        NUM_RX_PACKETS );

	//* Populate the pFreePacketHead list                   *
    for ( Counter = 0; Counter < NUM_TX_PACKETS; Counter++ ) {
        NdisAllocatePacket( &Status, &pPacket, pDevExt->TxPacketPoolHandle );
        if ( pPacket != NULL ) {
            PutPacket( pDevExt, pPacket );
        } else {
            break;
        }
    }
}
	
NTSTATUS DumpHandler(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP irp
)
{
	PIO_STACK_LOCATION  iostack;
	PCHAR				buf, outbuf;
	int                 outlen;
	PIR_DEVICE_EXT		pDevExt = GlobalDeviceObject->DeviceExtension;
	PLIST_ENTRY			pHead, pTemp; 
	POPENRECORD			pRecord;
	NTSTATUS			Status;

	iostack = IoGetCurrentIrpStackLocation(irp);
	outbuf = (PCHAR) irp->AssociatedIrp.SystemBuffer;
	outlen = iostack->Parameters.DeviceIoControl.OutputBufferLength;
	/* big enough */
	if (outlen < 4000)
		return (4000);

	buf = outbuf;
	buf += sprintf(buf, "compiled %s %s version %s\n", 
				   __DATE__, __TIME__, EPI_VERSION_STR);

	pHead = &(pDevExt->OpenRecords);
	for (pTemp=pHead->Flink; pTemp != pHead; pTemp=pTemp->Flink) {
		pRecord = CONTAINING_RECORD(pTemp, OPENRECORD, ListEntry);
		if (pRecord->FileObject == irp->Tail.Overlay.OriginalFileObject ) {
			buf += sprintf(buf, "* ");
		} else {
			buf += sprintf(buf, "  ");
		}
		/*buf += sprintf(buf, " FO=0x%08x", (unsigned int)pRecord->FileObject);*/
		/*buf += sprintf(buf, " BH=0x%08x", (unsigned int)pRecord->BindingHandle);*/
		buf += sprintf(buf, "%s", (pRecord->PendingBind?" PB ":""));
		buf += sprintf(buf, "%s", (pRecord->PendingUnbind?" PU ":""));
		buf += sprintf(buf, "\n"); 
		buf += sprintf(buf, "\t%d pending relays\n", 
					   ListCount(&pRecord->PendingRelays));
		buf += sprintf(buf, "\t%d pending reads\n", 
					   ListCount(&pRecord->PendingReads));
		buf += sprintf(buf, "\t%d pending sends\n", 
					   ListCount(&pRecord->PendingSends));
		buf += sprintf(buf, "\t%d pending transfers\n", 
					   ListCount(&pRecord->PendingTransfers));
	}

	Status = STATUS_SUCCESS;
	IO_COMPLETE_REQUEST( irp, Status, strlen(outbuf)+1);
	
	return Status;
}

NTSTATUS ReadHandler(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP irp
)
{
	POPENRECORD pRecord;
	NTSTATUS Status;


	// Do some validation before proceeding to open the lower-level adapter.
	pRecord = LookupOpenRecord( irp->Tail.Overlay.OriginalFileObject );
	if (pRecord == NULL) {
		// no record of this file handle...probably fatal
		Status = RPC_NT_INVALID_BINDING;
	} else {
		NdisAcquireSpinLock( &pRecord->Lock );
		if (pRecord->BindingHandle == 0) {
			// It is not bound.
			Status = RPC_NT_INVALID_BINDING;
		} else {
			NdisZeroMemory( &irp->Tail.Overlay.ListEntry, 
							sizeof(LIST_ENTRY) );
			InsertTailList(&pRecord->PendingReads, 
						   &irp->Tail.Overlay.ListEntry);
			IoSetCancelRoutine(irp, CancelReadIrp);
			IoMarkIrpPending(irp);
			Status = STATUS_PENDING;
		}

		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (Status != STATUS_PENDING) {
		IO_COMPLETE_REQUEST( irp, Status, 0 );
	} 

	return Status;
}


static NTSTATUS RelayOidHandler(PIRP irp)
{
	PIO_STACK_LOCATION  iostack;
	void                *iob;
	int                 inlen, outlen;
	PIRELAY				pRelay;
	POPENRECORD         pRecord;
	PNDIS_REQUEST		pRequest;
	NTSTATUS			Status = STATUS_SUCCESS;
	NDIS_STATUS			NdisStatus;

	// Do some validation before proceeding to open the lower-level adapter.
	pRecord = LookupOpenRecord( irp->Tail.Overlay.OriginalFileObject );
	if (pRecord == NULL) {
		// no record of this file handle...probably fatal
		Status = RPC_NT_INVALID_BINDING;
	} else {
		NdisAcquireSpinLock( &pRecord->Lock );
		if (pRecord->BindingHandle == 0) {
			// Fail if we do not have a binding for this handle
			Status = RPC_NT_INVALID_BINDING;
		} 
		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (Status != STATUS_SUCCESS)
		goto error;

	// go ahead and issue a NdisOpenAdapter to the lower driver.
	NdisStatus = NdisAllocateMemory( &pRequest, sizeof( NDIS_REQUEST ), 
								  0, HighestAcceptableMax );
	if (NdisStatus == NDIS_STATUS_FAILURE ) {
		Status = STATUS_NO_MEMORY;
		goto error;
	}
	NdisZeroMemory( pRequest,  sizeof( NDIS_REQUEST ));

	iostack = IoGetCurrentIrpStackLocation(irp);
	iob = irp->AssociatedIrp.SystemBuffer;
	inlen = iostack->Parameters.DeviceIoControl.InputBufferLength;
	outlen = iostack->Parameters.DeviceIoControl.OutputBufferLength;
	pRelay = (PIRELAY)iob;

	if ( pRelay->rh.IsQuery ) {
		pRequest->RequestType = NdisRequestQueryInformation;
		pRequest->DATA.QUERY_INFORMATION.Oid = pRelay->rh.OID;
		pRequest->DATA.QUERY_INFORMATION.InformationBuffer = pRelay->Buffer;
		pRequest->DATA.QUERY_INFORMATION.InformationBufferLength = inlen - sizeof(pRelay->rh);
	} else {
		pRequest->RequestType = NdisRequestSetInformation;
		pRequest->DATA.SET_INFORMATION.Oid = pRelay->rh.OID;
		pRequest->DATA.SET_INFORMATION.InformationBuffer = pRelay->Buffer;
		pRequest->DATA.SET_INFORMATION.InformationBufferLength = inlen - sizeof(pRelay->rh);
	}

	// store a pointer to the ndis request in the irp
	// this is safe because this driver will hold the irp.
	*((PNDIS_REQUEST*) &irp->Tail.Overlay.DriverContext[0]) = pRequest;

	// insert in the pending request list before issuing the request
	// because the request might complete asynchronously before we
	// even notice that it is pending.

	NdisAcquireSpinLock( &pRecord->Lock );
	NdisZeroMemory( &irp->Tail.Overlay.ListEntry, sizeof(LIST_ENTRY) );
	InsertTailList(&pRecord->PendingRelays, &irp->Tail.Overlay.ListEntry);
	IoSetCancelRoutine(irp, CancelRelayIrp);
	IoMarkIrpPending(irp);
	NdisReleaseSpinLock( &pRecord->Lock );

	NdisRequest( &NdisStatus, pRecord->BindingHandle, pRequest );
	if ( NdisStatus != NDIS_STATUS_PENDING) {
		PtRequestComplete(pRecord->FileObject, pRequest, NdisStatus);
	}
	
	return STATUS_PENDING;

 error:
	IO_COMPLETE_REQUEST( irp, Status, 0 );
	return Status;

}


static NTSTATUS RelayQueryHandler(PIRP irp)
{
	PIO_STACK_LOCATION  iostack;
	void                *iob;
	int                 inlen, outlen;
	PIRELAY				pRelay;
	POPENRECORD         pRecord;
	PNDIS_REQUEST		pRequest;
	NTSTATUS			Status = STATUS_SUCCESS;
	NDIS_STATUS			NdisStatus;

	// Do some validation before proceeding to open the lower-level adapter.
	pRecord = LookupOpenRecord( irp->Tail.Overlay.OriginalFileObject );
	if (pRecord == NULL) {
		// no record of this file handle...probably fatal
		Status = RPC_NT_INVALID_BINDING;
	} else {
		NdisAcquireSpinLock( &pRecord->Lock );
		if (pRecord->BindingHandle == 0) {
			// Fail if we do not have a binding for this handle
			Status = RPC_NT_INVALID_BINDING;
		} 
		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (Status != STATUS_SUCCESS)
		goto error;

	// go ahead and issue a NdisOpenAdapter to the lower driver.
	NdisStatus = NdisAllocateMemory( &pRequest, sizeof( NDIS_REQUEST ), 
								  0, HighestAcceptableMax );
	if (NdisStatus == NDIS_STATUS_FAILURE ) {
		Status = STATUS_NO_MEMORY;
		goto error;
	}
	NdisZeroMemory( pRequest,  sizeof( NDIS_REQUEST ));

	iostack = IoGetCurrentIrpStackLocation(irp);
	iob = irp->AssociatedIrp.SystemBuffer;
	inlen = iostack->Parameters.DeviceIoControl.InputBufferLength;
	outlen = iostack->Parameters.DeviceIoControl.OutputBufferLength;

    pRequest->RequestType = NdisRequestQueryInformation;
    pRequest->DATA.QUERY_INFORMATION.Oid = *((int *) iob);
    pRequest->DATA.QUERY_INFORMATION.InformationBuffer = irp->UserBuffer;
    pRequest->DATA.QUERY_INFORMATION.InformationBufferLength = outlen;

	// store a pointer to the ndis request in the irp
	// this is safe because this driver will hold the irp.
	*((PNDIS_REQUEST*) &irp->Tail.Overlay.DriverContext[0]) = pRequest;

	// insert in the pending request list before issuing the request
	// because the request might complete asynchronously before we
	// even notice that it is pending.

	NdisAcquireSpinLock( &pRecord->Lock );
	NdisZeroMemory( &irp->Tail.Overlay.ListEntry, sizeof(LIST_ENTRY) );
	InsertTailList(&pRecord->PendingRelays, &irp->Tail.Overlay.ListEntry);
	IoSetCancelRoutine(irp, CancelRelayIrp);
	IoMarkIrpPending(irp);
	NdisReleaseSpinLock( &pRecord->Lock );

	NdisRequest( &NdisStatus, pRecord->BindingHandle, pRequest );
	if ( NdisStatus != NDIS_STATUS_PENDING) {
		PtRequestComplete(pRecord->FileObject, pRequest, NdisStatus);
	}
	
	return STATUS_PENDING;

 error:
	IO_COMPLETE_REQUEST( irp, Status, 0 );
	return Status;

}

NTSTATUS SendHandler(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP irp
)
{
	PIR_DEVICE_EXT	pDevExt = DeviceObject->DeviceExtension;
	PNDIS_PACKET	NdisPacket;
	PNDIS_BUFFER    NdisBuffer;
	NDIS_STATUS		NdisStatus;
	POPENRECORD     pRecord;
	NTSTATUS		Status;
	PIO_STACK_LOCATION  iostack;
	PVOID               iob;
	int                 inlen;

	// Do some validation before proceeding to open the lower-level adapter.
	pRecord = LookupOpenRecord( irp->Tail.Overlay.OriginalFileObject );
	if (pRecord == NULL) {
		// no record of this file handle...probably fatal
		Status = STATUS_INVALID_HANDLE;
	} else {
		NdisAcquireSpinLock( &pRecord->Lock );
		if (pRecord->BindingHandle == 0) {
			// It is not bound.
			Status = STATUS_INVALID_HANDLE;
		} else {
			Status = STATUS_SUCCESS;
		}
		NdisReleaseSpinLock( &pRecord->Lock );
	}

	if (Status == STATUS_SUCCESS) {
		NdisPacket = GetPacket( pDevExt );
		if ( NdisPacket == NULL ) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	if (Status != STATUS_SUCCESS) {
		IO_COMPLETE_REQUEST( irp, Status, 0 );
	} else {
		iostack = IoGetCurrentIrpStackLocation(irp);
		iob = irp->AssociatedIrp.SystemBuffer;
		inlen = iostack->Parameters.DeviceIoControl.InputBufferLength;

		NdisAllocateBuffer( &Status, &NdisBuffer, pDevExt->TxBufferPoolHandle,
							iob, inlen );

		NdisChainBufferAtFront( NdisPacket, NdisBuffer );

		// store a pointer to the ndis request in the irp
		// this is safe because this driver will hold the irp.
		*((PNDIS_PACKET*) &irp->Tail.Overlay.DriverContext[0]) = NdisPacket;

		// insert in the pending request list before issuing the request
		// because the request might complete asynchronously before we
		// even notice that it is pending.

		NdisAcquireSpinLock( &pRecord->Lock );
		NdisZeroMemory( &irp->Tail.Overlay.ListEntry, sizeof(LIST_ENTRY) );
		InsertTailList(&pRecord->PendingSends, &irp->Tail.Overlay.ListEntry);
		IoSetCancelRoutine(irp, CancelSendIrp);
		IoMarkIrpPending(irp);
		NdisReleaseSpinLock( &pRecord->Lock );

		NdisSend( &NdisStatus, pRecord->BindingHandle, NdisPacket );
		if ( NdisStatus != NDIS_STATUS_PENDING) {
			PtSendComplete( pRecord->FileObject, NdisPacket, NdisStatus );
		}
		Status = STATUS_PENDING;
	}

	return Status;
}

#ifdef DBG
void
ir_assert(char *exp, char *file, uint line)
{
	printf("assertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
	printf("\n");
	KeBugCheckEx(line, 0, 0, 0, 0);
}
#endif
