
#define	IO_COMPLETE_REQUEST(irp, status, info) { \
	(irp)->IoStatus.Status = (status); \
	(irp)->IoStatus.Information = (info); \
	IoCompleteRequest(irp, IO_NO_INCREMENT); \
}


typedef struct _OPENRECORD {
	LIST_ENTRY			ListEntry;
	PFILE_OBJECT		FileObject;
	NDIS_HANDLE			BindingHandle;	// To the lower miniport
    NDIS_SPIN_LOCK      Lock;
	PIRP				PendingBind;
	PIRP				PendingUnbind;
	LIST_ENTRY			PendingRelays;
	LIST_ENTRY			PendingReads;
	LIST_ENTRY			PendingSends;
	LIST_ENTRY			PendingTransfers;
} OPENRECORD, *POPENRECORD;


typedef struct _IR_DEVICE_EXT {
	// Lock used to protect lists.
	//
	NDIS_SPIN_LOCK  ListLock;
	NDIS_SPIN_LOCK  TxListLock;

	NDIS_HANDLE			TxPacketPoolHandle;
    NDIS_HANDLE         TxBufferPoolHandle;
	NDIS_HANDLE			RxPacketPoolHandle;
    NDIS_HANDLE         RxBufferPoolHandle;

	// Linked list of free NDIS_PACKETs.
	//
    PNDIS_PACKET		pFreePacketHead;

	NDIS_HANDLE			ProtHandle;

    NDIS_EVENT          TxEvent;        // Event used to signal when a SendComplete occurs

	LIST_ENTRY			OpenRecords;

} IR_DEVICE_EXT, *PIR_DEVICE_EXT;



        //*******************************************************
        //*                                                     *
        //* Protocol reserved field definition.                 *
        //*                                                     *
        //*******************************************************
typedef struct _RSVD
{
    PNDIS_PACKET    pNext;
} RSVD, *PRSVD;


        //*******************************************************
        //*                                                     *
        //* GET macro for free packet list.                     *
        //*                                                     *
        //*******************************************************
__inline PNDIS_PACKET GetPacket( PIR_DEVICE_EXT pDevExt )
{
	PRSVD           pRsvd;
	PNDIS_PACKET    pTempPacket;

        //*******************************************************
        //*                                                     *
        //* Acqure spinlock and get a packet from the head.     *
        //*                                                     *
        //*******************************************************
    NdisAcquireSpinLock( &pDevExt->TxListLock );
    pTempPacket = pDevExt->pFreePacketHead;

    if ( pTempPacket != NULL ) {
		pRsvd = (PRSVD)pTempPacket->ProtocolReserved;
		pDevExt->pFreePacketHead = pRsvd->pNext;
		pRsvd->pNext = NULL;
	}

	NdisReleaseSpinLock( &pDevExt->TxListLock );

    return pTempPacket;
}

//*******************************************************
//*                                                     *
//* PUT macro for free packet list.                     *
//*                                                     *
//*******************************************************
__inline void PutPacket( PIR_DEVICE_EXT pDevExt, PNDIS_PACKET pPacket )
{
	PRSVD           pRsvd;
	PNDIS_PACKET    pTempPacket;

    NdisAcquireSpinLock( &pDevExt->TxListLock );

    pRsvd = (PRSVD)pPacket->ProtocolReserved;
    pRsvd->pNext = pDevExt->pFreePacketHead;
    pDevExt->pFreePacketHead = pPacket;
    NdisReleaseSpinLock( &pDevExt->TxListLock );
}





typedef ULONG DWORD;

#ifdef WIN95
#if !defined(WANTVXDWRAPS)
#define WANTVXDWRAPS
#include <basedef.h>
#include <vmm.h>
#include "vtd.h"
#include "vmmreg.h"
#include <vxdwraps.h>
#include <debug.h>
#define printf _Debug_Printf_Service
#define sprintf _Sprintf
#endif	/* WANTVXDWRAPS */
#else	/* WIN95 */
/*extern unsigned long _cdecl DbgPrint(char *fmt, ...);*/
#define	printf	DbgPrint
#endif	/* WIN95 */

#ifdef DBG
extern void ir_assert(char *exp, char *file, uint line);
#undef ASSERT
#define ASSERT(exp)     if (exp) ; else ir_assert(#exp, __FILE__, __LINE__)
#else
#undef ASSERT
#define ASSERT(exp) 
#endif
