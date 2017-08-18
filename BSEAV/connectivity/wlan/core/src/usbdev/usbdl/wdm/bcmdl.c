/*
 * Broadcom Remote Download routines 
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include "bulkusb.h"
#include "bulkpnp.h"
#include "bulkpwr.h"
#include "bulkdev.h"
#include "bulkrwr.h"
#include "bulkwmi.h"
#include "bulkusr.h"

#include "usbrdl.h"
#include "usbrndis.h"


/* Issue a bulk OUT transfer */
NTSTATUS
BulkSend(PDEVICE_OBJECT DeviceObject, PURB urb, USBD_PIPE_HANDLE pipe, void *buffer, int buflen)
{
    NTSTATUS               ntStatus;
	/* Create the bulk urb */
	UsbBuildInterruptOrBulkTransferRequest( urb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
											pipe, buffer, NULL, buflen, 0, NULL);

	ntStatus = CallUSBD(DeviceObject, urb);

	if(!NT_SUCCESS(ntStatus))
		BulkUsb_DbgPrint(1, ("BCMDL: Bad bulksend\n"));

	return(ntStatus);
}

/* Issue a CTRL IN CMD/STATUS */
NTSTATUS
DlCmd(PDEVICE_OBJECT DeviceObject, PURB urb, UCHAR cmd, void *buffer, int buflen)
{
    NTSTATUS               ntStatus;
	
	urb->UrbControlVendorClassRequest.TransferBufferLength = buflen;
	urb->UrbControlVendorClassRequest.TransferBuffer =  buffer;
	urb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
	urb->UrbControlVendorClassRequest.Request = (UCHAR)cmd;

	ntStatus = CallUSBD(DeviceObject, urb);

	if(!NT_SUCCESS(ntStatus)) 
		BulkUsb_DbgPrint(1, ("BCMDL: Bad DLCMD\n"));

	return(ntStatus);
}

VOID
DownloadRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
{
    NTSTATUS               ntStatus;
    PIO_WORKITEM           workItem = (PIO_WORKITEM)Context;
	PDEVICE_EXTENSION 		deviceExtension;
    PUSBD_INTERFACE_INFORMATION interfaceInfo;
    USBD_PIPE_HANDLE pipeHandle=NULL;
	PURB urb=NULL, bulkurb=NULL;
	unsigned int i, send, sent, dllen;
	char *bulkchunk = NULL, *dlpos;
	rdl_state_t *state;
	bootrom_id_t *id;

    BulkUsb_DbgPrint(1, ("BCMDL: Download Routine - start\n"));

	/* init variables */
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    interfaceInfo = deviceExtension->UsbInterface;
    urb = ExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
	id = (bootrom_id_t*)ExAllocatePool(NonPagedPool, sizeof(bootrom_id_t));
	state = (rdl_state_t*)ExAllocatePool(NonPagedPool, sizeof(rdl_state_t));
    bulkurb = ExAllocatePool(NonPagedPool, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
	bulkchunk = ExAllocatePool(NonPagedPool, RDL_CHUNK);

	/* Get the pipe handle for the Bulk endpoint */
	for(i=0; i<interfaceInfo->NumberOfPipes; i++) {
		if((interfaceInfo->Pipes[i].PipeType == UsbdPipeTypeBulk) && 
			!(interfaceInfo->Pipes[i].EndpointAddress & 0x80)) {
			BulkUsb_DbgPrint(1, ("BCMDL: Found bulk pipe EP%d\n", i));
			/* set pipe handle */
			pipeHandle = interfaceInfo->Pipes[i].PipeHandle;
			break;
		}
	}

	/* check all the allocations */
	if (!urb || !id || !bulkchunk || !bulkurb || !state || !pipeHandle) {
        BulkUsb_DbgPrint(1, ("BCMDL:  Failed to allocate a resource\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		goto err;
	}
	
	/* Create the generic URB for getting status/issueing cmds */
	urb->UrbHeader.Function = URB_FUNCTION_VENDOR_INTERFACE;
	urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
	urb->UrbControlVendorClassRequest.Reserved = 0;
	urb->UrbControlVendorClassRequest.TransferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK ;
	urb->UrbControlVendorClassRequest.RequestTypeReservedBits = 0;
	urb->UrbControlVendorClassRequest.Value = (USHORT)1;
	urb->UrbControlVendorClassRequest.Index = (USHORT)0;
	urb->UrbControlVendorClassRequest.Reserved1 = 0;
	urb->UrbControlVendorClassRequest.UrbLink = NULL;

	/* Get the ID */
	DlCmd(DeviceObject, urb, (UCHAR)DL_GETVER, id, sizeof(bootrom_id_t));

	BulkUsb_DbgPrint(1, ("BCMDL: chip 0x%x rev 0x%x\n", id->chip, id->chiprev));

	/* Issue the start cmd */
	DlCmd(DeviceObject, urb, (UCHAR)DL_START, state, sizeof(rdl_state_t));

	/* Check we are in the Waiting state */
	if (state->state != DL_WAITING)
        BulkUsb_DbgPrint(1, ("BCMDL: Failed to DL_START\n"));

	/* init dl array state */
	sent=0;
	dlpos = &dlarray[0];
	dllen = sizeof(dlarray);

	/* Load the image */
	while((state->bytes != dllen)) { 
		/* Wait until the usb device reports it received all the bytes we sent */
		if ((state->bytes == sent) && (state->bytes != dllen)) {
 			if ((dllen-sent) < RDL_CHUNK) 
				send = dllen-sent;
			else
				send = RDL_CHUNK;

			/* simply avoid having to send a ZLP by 
			   ensuring we never have an even multiple
			   of 64 */ 
			if (!(send % 64))
				send -= 4;

			/* send data */
			memcpy(bulkchunk, dlpos, send);
		    ntStatus = BulkSend(DeviceObject, bulkurb, pipeHandle, bulkchunk, send); 
			if(!NT_SUCCESS(ntStatus)) goto err; 									  
			
			dlpos += send;
			sent += send;
		} 

	    ntStatus = DlCmd(DeviceObject, urb, (UCHAR)DL_GETSTATE, state, sizeof(rdl_state_t));
    	if(!NT_SUCCESS(ntStatus)) goto err;   

		/* restart if an error is reported */
		if ((state->state == DL_BAD_HDR) || (state->state == DL_BAD_CRC)) {
			BulkUsb_DbgPrint(1, ("BCMDL: Bad Hdr or Bad CRC\n"));
			goto err; 
		}
	} 
									 
	/* Check we are runnable */
	ntStatus = DlCmd(DeviceObject, urb, (UCHAR)DL_GETSTATE, state, sizeof(rdl_state_t));
	if(!NT_SUCCESS(ntStatus)) goto err; 

	/* Start the image */
	if (state->state == DL_RUNNABLE) {
		BulkUsb_DbgPrint(1, ("BCMDL: DL_GO\n"));
		ntStatus = DlCmd(DeviceObject, urb, (UCHAR)DL_GO, state, sizeof(rdl_state_t));
	}
	else
		BulkUsb_DbgPrint(1, ("BCMDL: Not runnable\n"));

err:
	/* Free the allocated memory */
    if (urb)
		ExFreePool(urb);
	if (id)
		ExFreePool(id);
	if (state)
		ExFreePool(state);
    if (bulkurb)
		ExFreePool(bulkurb);
	if (bulkchunk)
		ExFreePool(bulkchunk);

	IoFreeWorkItem(workItem);
	return;
}
