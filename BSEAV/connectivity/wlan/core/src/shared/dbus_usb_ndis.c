/*
 * Dongle BUS (DBUS) interface
 * USB NDIS Implementation
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


#include <typedefs.h>
#include <osl.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <usbrdl.h>
#include <ndis.h>
#include <usb.h>
#include <initguid.h>
#include <usbdlib.h>
#include <usbdrivr.h>
#if (0>= 0x0600)
#include <wdf.h>
#include <WdfMiniport.h>
#include <wdfusb.h>
#include <wdfdevice.h>
/* #include <wdfworkitem.h> #include <Wdfcore.h> */
#include <wdfrequest.h>
#endif

#include "dbus.h"

#if (0< 0x0600)
#define WDM_API
#else
/* #define WDM_API */
#define WDF_API
#endif

#if (defined(WDM_API) && defined(WDF_API))
#error	"Only one API interface can be defined: WDM_API or WDF_API"
#endif

#define NDIS_TAG_USBDRV		((ULONG)'MDWN')	/* USB driver tag name */

#define ETH_HEADER_SIZE		14
#define ETH_MAX_DATA_SIZE	1500
#define ETH_MAX_PACKET_SIZE	ETH_HEADER_SIZE + ETH_MAX_DATA_SIZE
#define ETH_MIN_PACKET_SIZE	60

/* START: Code Ported */
#define CTL_RETRY	3
#define MAX_CMD_RETRY	2
#define DL_RESETCFG     8       /* for single enumeration */
#define POSTBOOT_ID     0xA123  /* ID to detect if dongle has boot up */
#define REMOTE_WAKEUP_MASK 0x20
#define TXCMPLT_TIMEOUT	60	/* 30s tx_complete callback timeout */

#define USB_DEV_ISBAD(u)	(u->devid == 0xDEAD)

typedef struct usbos_info usbos_info_t;	/* forward declaration */

#define DBUS_USBOS_MAX_PIPES		5

#if (0>= 0x0600)
/* #define DBUS_USBOS_MAX_CONTREADS	usbinfo->pub->nrxq */
#define DBUS_USBOS_MAX_WRITE_REQ	100	/* RPC only uses DBUS_NTXQ=50 */
#define DBUS_USBOS_MAX_CONTROL_REQ	2	/* rpc only use 1 ctrl ep for callreturn */

typedef struct _dbus_usbos_wdf_res_tx {
	WDFREQUEST	write_req_array[DBUS_USBOS_MAX_WRITE_REQ];
	WDFSPINLOCK	write_req_array_spinlock;
	USHORT		write_req_max;
	USHORT		NextAvailReqIndex;
} dbus_usbos_wdf_res_tx;

typedef struct _dbus_usbos_TCB_wdf {
	LIST_ENTRY		List;		/* This must be the first entry */
	usbos_info_t		*usbinfo;
	void 			*txirb;		/* DBUS IO request block: dbus_irb_tx_t */
	ULONG			ulSize;		/* request len */
	CHAR			*pData;		/* request buffer */
	LONG			Ref;
	UCHAR			*txDataBuf;	/* data stream */

	WDFMEMORY		UrbMemory;
	PURB			Urb;
	PMDL			pMdl;
	USBD_PIPE_HANDLE	UsbdPipeHandle;
	WDFUSBPIPE		UsbPipe;
	UCHAR			QueueType;
	WDFIOTARGET		IoTarget;
} dbus_usbos_TCB_wdf, *pdbus_usbos_TCB_wdf;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(dbus_usbos_TCB_wdf, _get_ctx_TCB)

typedef struct _dbus_usbos_wdf_res_ctr {
	WDFREQUEST      control_req_array[DBUS_USBOS_MAX_CONTROL_REQ];
	WDFSPINLOCK     control_req_array_spinlock;
	USHORT          control_req_max;
	USHORT          NextAvailReqIndex;
} dbus_usbos_wdf_res_ctr;

typedef struct _dbus_usbos_wdf_ctl_info {
	usbos_info_t	*usbinfo;
	ULONG_PTR	va;		/* va for next segment of xfer */
	UCHAR		queue_type;
	UCHAR		Data[sizeof(ULONG)];
} dbus_usbos_wdf_ctl_info_t;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(dbus_usbos_wdf_ctl_info_t, _get_ctx_ctl_wdf)

typedef struct _DBUS_USBOS_PIPE_CONTEXT {
	size_t		MaxPacketLength;
	BYTE		MaxOutstandingWrites;
	WDFREQUEST*	pAvailableRequests;
	WDFSPINLOCK	RequestArrayLock;
	BYTE		NextAvailReqIndex;
	BOOLEAN		IsStarted;
	LONG		ReferenceCount;
} DBUS_USBOS_PIPE_CONTEXT;

typedef enum _WORKITEM_STATE {
	DBUS_WORKITEM_STATE_FREE = 0,
	DBUS_WORKITEM_STATE_BUSY = 1
} DBUS_WORKITEM_STATE;

typedef struct {
	usbos_info_t	*usbinfo;
	DBUS_WORKITEM_STATE	state;
} usbinfo_workitem_ctx_ndis6;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(usbinfo_workitem_ctx_ndis6, _get_context_workitem)

#define DBUS_USBOS_QUEUE_LOWPRIO	1
#define DBUS_USBOS_QUEUE_HIGHPRIO	2	/* TBD like WDF sample when needed */
#else
#define RX_RESET_THRESHOLD	5
#endif 

typedef struct dbus_ctl {
	void *buf;
	uint len;
} dbus_ctl_t;


struct usbos_info {
	dbus_pub_t *pub;

	void *cbarg;
	dbus_intf_callbacks_t *cbs;

	int devid;

	NDIS_HANDLE	AdapterHandle;
	PDEVICE_OBJECT	TopOfStackDO;	/* Device object we call when submitting Urbs */
	PDEVICE_OBJECT	pdo;
	PDEVICE_OBJECT	fdo;
	PDEVICE_OBJECT	nextDeviceObj;
	BOOLEAN		remote_wake_capable;
#if (0>= 0x0600)
	WDFDEVICE		WdfDevice;	/* WDF device object */
#endif

	/* WDM USB device object, keep out of WDF_API to avoid add ifndef WDF_API */
	PUSB_CONFIGURATION_DESCRIPTOR	UsbConfigurationDescriptor;
	PUSBD_INTERFACE_INFORMATION	UsbInterface;
	int				UsbInterfaceSize;
	USBD_PIPE_HANDLE	TxPipeHandleWdm;
	USBD_PIPE_HANDLE	RxPipeHandleWdm;

	/* WDF USB device object */
#ifdef WDF_API
	WDFUSBDEVICE		UsbDevice;	/* USB target device object */
	WDFIOTARGET		IoTarget;
	WDFUSBINTERFACE		WdfUsbInterface;	/* interface object */

	WDFUSBPIPE		RxPipeHandleWdf;
	WDFUSBPIPE		TxPipeHandleWdf;
	WDFUSBPIPE		TxPipeHandle2Wdf;
	WDFUSBPIPE		IntrPipeHandleWdf;
#endif	/* WDF_API */


#ifdef WDF_API
	WDFWAITLOCK		PipeState_WaitLock;	/* framework wait lock for PIPEs */

	dbus_usbos_wdf_res_ctr	ctl_resource_wdf;

	dbus_usbos_wdf_res_tx	tx_q_highprio;		/* high priority beacon, not used yet */
	dbus_usbos_wdf_res_tx	tx_q_lowprio;
	WDFTIMER		PendRequestTimer;
	WDF_REQUEST_SEND_OPTIONS  WriteRequestOptions;

	WDFSPINLOCK		WdfRecv_SpinLock;
	ULONG			ReaderMdlSize;
	LONG			ReaderCount;
	KEVENT			ReaderWaitEvent;

	BOOLEAN			device_goingdown;
	BOOLEAN			rx_flowctl_on;
	int			rx_flowctl_pending;
	WDFWORKITEM             rx_flowctl_WorkItem;	/* stop/resume rx pipe, PASSIVE LEVEL */

#endif	/* WDF_API */

	PUCHAR 			p_TCB_memory;
	PUCHAR 			p_txbuf_memory;
	PUCHAR			p_RCB_memory;
	PUCHAR			p_rxbuf_memory;

	/* state machine */
	ULONG			MP_Flags;
	bool			pnp_isdown;

	/* USB Interrupt Pipe */
	USBD_PIPE_HANDLE	IntrPipeHandleWdm;
	PURB			intUrb;
	ULONG			InterruptPayload[2];

	/* USB Control Pipe */
	PURB			ctlUrb;
	KMUTEX			ctl_mutex;
	dbus_ctl_t		txctl_info;
	dbus_ctl_t		rxctl_info;
	NDIS_EVENT		txctl_PendingEvent;
	NDIS_EVENT		rxctl_PendingEvent;
	BOOLEAN			txctl_pending;
	BOOLEAN			rxctl_pending;
	BOOLEAN			rxctl_deferrespok;	/* Get a response for setup from dongle */

	/* USB Bulk pipe */
	KSPIN_LOCK		IOCountSpinLock_Bulk;
	ULONG			IOoutstanding_Bulk;
	KEVENT			RemoveEvent;
	KEVENT			StopEvent;
	KEVENT			purge_event;
	KMUTEX			purge_mutex;
	int			bulkpipe_mps;

	/* USB Bulk tx/rx management */
	LIST_ENTRY		rx_busy_list;
	LIST_ENTRY		rx_free_list;
	LONG			rx_pkt_busyrecv;
	NDIS_SPIN_LOCK		rx_spinlock;
	LONG			rx_totbytesread;

	NDIS_SPIN_LOCK		tx_spinlock;		/* sync tx activity */
	LIST_ENTRY		tx_free_list;		/* preallocated TCB list */
	LIST_ENTRY		tx_busy_list;		/* TCB in use, pending completion */
	LONG			tx_pkt_busysend;
	ULONGLONG		tx_totbytessend;
	NDIS_MINIPORT_TIMER	tx_timer;

	ULONG			txpkt_pending_cnt;	/* to be used */
	int			tx_cmplt_timeout;	/* to be used */
	int			tx_low_watermark;	/* to be used */

#if (0>= 0x0600)
	NDIS_HANDLE		Rx_NetBufferListPool;
	NDIS_HANDLE		Rx_NetBufferPool;
	WDFWORKITEM		ctl_rx_WorkItem;
	WDFWORKITEM		ctl_tx_WorkItem;
#else
	NDIS_HANDLE		RecvPacketPoolHandle;
	NDIS_HANDLE		RecvBufferPoolHandle;
	NDIS_WORK_ITEM		ctl_rx_WorkItem;
	NDIS_WORK_ITEM		rx_reset_WorkItem;
	NDIS_WORK_ITEM		ctl_tx_WorkItem;

	NDIS_HANDLE		tx_bufferpool_ndis5;
	int			rx_reset_count;
	BOOLEAN			rx_reset_on;
#endif

	KEVENT			eve;
};

/*
 * The following NDIS status codes map directly to NT status codes.
 */
#define NT_STATUS_TO_NDIS_STATUS(_NtStatus, _pNdisStatus) \
	{ \
	if (((STATUS_SUCCESS == (_NtStatus)) || (STATUS_PENDING == (_NtStatus)) || \
		(STATUS_BUFFER_OVERFLOW == (_NtStatus)) || (STATUS_UNSUCCESSFUL == (_NtStatus)) || \
		(STATUS_INSUFFICIENT_RESOURCES == (_NtStatus)) || \
		(STATUS_NOT_SUPPORTED == (_NtStatus)))) \
		*(_pNdisStatus) = (NDIS_STATUS)(_NtStatus); \
	else if (STATUS_BUFFER_TOO_SMALL == (_NtStatus)) \
		*(_pNdisStatus) = NDIS_STATUS_BUFFER_TOO_SHORT; \
	else if (STATUS_INVALID_BUFFER_SIZE == (_NtStatus)) \
		*(_pNdisStatus) = NDIS_STATUS_INVALID_LENGTH; \
	else if (STATUS_INVALID_PARAMETER == (_NtStatus)) \
		*(_pNdisStatus) = NDIS_STATUS_INVALID_DATA; \
	else if (STATUS_NO_MORE_ENTRIES == (_NtStatus)) \
		*(_pNdisStatus) = NDIS_STATUS_ADAPTER_NOT_FOUND; \
	else if (STATUS_DEVICE_NOT_READY == (_NtStatus)) \
		*(_pNdisStatus) = NDIS_STATUS_ADAPTER_NOT_READY; \
	else \
		*(_pNdisStatus) = NDIS_STATUS_FAILURE; \
	}

typedef struct _ETH_HEADER
{
	UCHAR DstAddr[ETH_LENGTH_OF_ADDRESS];
	UCHAR SrcAddr[ETH_LENGTH_OF_ADDRESS];
	USHORT EthType;
} ETH_HEADER, *PETH_HEADER;


/*
 * An IRPLOCK allows for safe cancellation. The idea is to protect the IRP
 * while the canceller is calling IoCancelIrp. This is done by wrapping the
 * call in InterlockedExchange(s). The roles are as follows:
 *
 * Initiator/completion: Cancelable --> IoCallDriver() --> Completed
 * Canceller: CancelStarted --> IoCancelIrp() --> CancelCompleted
 * No cancellation:
 *   Cancelable-->Completed
 * Cancellation, IoCancelIrp returns before completion:
 *   Cancelable --> CancelStarted --> CancelCompleted --> Completed
 * Canceled after completion:
 *   Cancelable --> Completed -> CancelStarted
 * Cancellation, IRP completed during call to IoCancelIrp():
 *   Cancelable --> CancelStarted -> Completed --> CancelCompleted
 *
 *  The transition from CancelStarted to Completed tells the completer to block
 *  postprocessing (IRP ownership is transfered to the canceller). Similarly,
 *  the canceler learns it owns IRP postprocessing (free, completion, etc)
 *  during a Completed->CancelCompleted transition.
 */
typedef enum {
	IRPLOCK_CANCELABLE,
	IRPLOCK_CANCEL_STARTED,
	IRPLOCK_CANCEL_COMPLETE,
	IRPLOCK_COMPLETED
} IRPLOCK;

#if (0>= 0x0600)
typedef enum _DBUS_USB_MDL_TYPE
{
	INVALID_MDL = 0,
	DATA_MDL,
	PARTIAL_MDL
} DBUS_USB_MDL_TYPE;
#endif 

/* TCB (Transmit Control Block) */
typedef struct _TCB
{
	LIST_ENTRY		List;		/* This must be the first entry */
	void 			*txirb;		/* DBUS IO request block: dbus_irb_tx_t */
	LONG			Ref;
	PVOID			usbinfo;

	PIRP			Irp;
	IRPLOCK			IrpLock;
	PURB			Urb;
	PUCHAR			pData;
	ULONG			ulSize;

#if (0>= 0x0600)
	/* WDF_API_OLD, not used. Replaced by new WDF_API flow */
	WDFREQUEST		Request;
	PMDL			pMdl;
	DBUS_USB_MDL_TYPE	MdlType;	/* MdlType to use for transfer */
	PMDL			DataMdl;	/* NOT_YET, Mdl representing the Data buf below */
	PMDL			PartialMdl;	/* NOT_YET, Mdl used in the request */
	PNET_BUFFER		NetBuffer;
	PNET_BUFFER_LIST	NetBufferList;
#else
	PNDIS_BUFFER		Buffer_ndis5;	/* obsolete */
#endif

	UCHAR			*txDataBuf;	/* data stream */
} TCB, *PTCB;


/* RCB (Receive Control Block) */
typedef struct _RCB
{
	LIST_ENTRY		List;		/* This must be the first entry */
	void 			*rxirb;		/* DBUS IO request block: dbus_irb_rx_t */
	LONG			Ref;
	PVOID			usbinfo;

	PIRP			Irp;
	IRPLOCK			IrpLock;
	PURB			Urb;
	ULONG			ulSize;

#if (0< 0x0600)
	PUCHAR			pData_ndis5;
	PNDIS_PACKET		Packet_ndis5;	/* obsolete */
	PNDIS_BUFFER		Buffer_ndis5;	/* obsolete */
#else
	WDFREQUEST		Request;
	PMDL			pMdl;
	PNET_BUFFER		NetBuffer;
	PNET_BUFFER_LIST	NetBufferList;
#endif
	UCHAR			*rxDataBuf;
} RCB, *PRCB;

#if (0>= 0x0600)
#define MP_SET_RCB_IN_NBL(_NBL, _RCB)	(*((PRCB*)&NET_BUFFER_LIST_MINIPORT_RESERVED(_NBL)) = _RCB)
#define MP_GET_RCB_FROM_NBL(_NBL)	(*((PRCB*)&NET_BUFFER_LIST_MINIPORT_RESERVED(_NBL)))
#endif

#define fMP_RESET_IN_PROGRESS			0x00000001
#define fMP_DISCONNECTED			0x00000002
#define fMP_HALT_IN_PROGRESS			0x00000004
#define fMP_SURPRISE_REMOVED			0x00000008
#define fMP_INIT_IN_PROGRESS			0x00000020

#define fMP_TX_RESOURCE_ALLOCATED		0x00000040
#define fMP_RX_RESOURCE_ALLOCATED		0x00000080
#define fMP_POST_READS				0x00000100
#define fMP_POST_WRITES				0x00000200
#define fMP_INIT_DONE				0x00000400
#define fMP_INIT_PNP                            0x00000800

#define DEVICEREMOVED(status) ((status == STATUS_DEVICE_NOT_CONNECTED) ||\
	(status == STATUS_DEVICE_POWER_FAILURE) || (status == STATUS_DEVICE_DOES_NOT_EXIST) ||\
	(status == STATUS_DEVICE_REMOVED) || (status == STATUS_NO_SUCH_DEVICE))

#define MP_TEST_FLAG(_M, _F)	(((_M)->MP_Flags & (_F)) != 0)
#define MP_IS_READY(_M)		(((_M)->MP_Flags & \
	(fMP_RESET_IN_PROGRESS | fMP_HALT_IN_PROGRESS | fMP_INIT_IN_PROGRESS | \
	fMP_SURPRISE_REMOVED)) == 0)
#define MP_ISSET_FLAG(_M, _F)		((_M)->MP_Flags & (_F))
#define MP_SET_FLAG(_M, _F)		((_M)->MP_Flags |= (_F))
#define MP_CLEAR_FLAG(_M, _F)		((_M)->MP_Flags &= ~(_F))


/* Local function prototypes */

#if (0>= 0x0600)

#ifdef WDF_API

/* WDF init */
static NTSTATUS dbus_usbos_ctl_req_allocate_wdf(usbos_info_t* usbinfo, USHORT max);
static void dbus_usbos_ctl_req_deallocate_wdf(usbos_info_t* usbinfo);
static WDFREQUEST dbus_usbos_ctl_req_get_wdf(usbos_info_t* usbinfo);
static void dbus_usbos_ctl_req_return_wdf(usbos_info_t* usbinfo, WDFREQUEST Request);
static void dbus_usbos_ctl_req_completion_tx_wdf(WDFREQUEST  Request, WDFIOTARGET Target,
	PWDF_REQUEST_COMPLETION_PARAMS CompletionParams, WDFCONTEXT Context);
static void dbus_usbos_ctl_req_completion_rx_wdf(WDFREQUEST  Request, WDFIOTARGET Target,
	PWDF_REQUEST_COMPLETION_PARAMS CompletionParams, WDFCONTEXT Context);

static NTSTATUS dbus_usbos_ctl_req_submit_async_wdf(usbos_info_t* usbinfo, bool tx, BYTE Request,
	USHORT Value, ULONG Length, PVOID Buffer, USHORT Index);
static void dbus_usbos_stop_all_pipes_wdf(usbos_info_t* usbinfo);


/* WDF rx */
static bool dbus_usbos_rx_res_alloc_wdf(usbos_info_t *usbinfo);
static void dbus_usbos_rx_res_dealloc_wdf(usbos_info_t *usbinfo);
static bool dbus_usbos_rx_start_wdf(usbos_info_t *usbinfo);
static void dbus_usbos_rx_stop_wdf(usbos_info_t *usbinfo, bool cancel);
static bool dbus_usbos_rx_contread_setup_wdf(usbos_info_t *usbinfo, WDFUSBPIPE ReadPipe);
static BOOLEAN dbus_usbos_rx_contread_fail_cb_wdf(WDFUSBPIPE Pipe, NTSTATUS St, USBD_STATUS UsbdSt);
static VOID dbus_usbos_rx_contread_cb_wdf(WDFUSBPIPE, WDFMEMORY, size_t, WDFCONTEXT);

/* WDF tx */
static bool dbus_usbos_tx_send_irb_async_wdf(usbos_info_t *usbinfo, WDFREQUEST,
	dbus_usbos_TCB_wdf *);
static bool dbus_usbos_tx_send_buf_sync_wdf(usbos_info_t *usbinfo, ULONG, PVOID, dbus_irb_tx_t *);

static bool dbus_usbos_tx_req_alloc_wdf(usbos_info_t *usbinfo, uint NumTxd);
static void dbus_usbos_tx_req_dealloc_wdf(usbos_info_t *usbinfo);
static bool dbus_usbos_tx_req_worker_allocate_wdf(usbos_info_t *, dbus_usbos_wdf_res_tx *);
static void dbus_usbos_tx_req_worker_dealloc_wdf(usbos_info_t *, dbus_usbos_wdf_res_tx *);

static void dbus_usbos_tx_req_get_wdf(usbos_info_t *usbinfo, WDFREQUEST *wdfreq);
static void dbus_usbos_tx_req_return_wdf(usbos_info_t *usbinfo, WDFREQUEST);

/* WDF callback */
static NTSTATUS dbus_usbos_tx_complete_cb(WDFREQUEST, WDFIOTARGET, PWDF_REQUEST_COMPLETION_PARAMS,
	WDFCONTEXT);
static NTSTATUS dbus_usbos_rx_complete_cb(WDFREQUEST, WDFIOTARGET, PWDF_REQUEST_COMPLETION_PARAMS,
	WDFCONTEXT);

static NTSTATUS dbus_usbos_urb_complete_sync_cb_wdf(WDFREQUEST Request, WDFIOTARGET Target,
	PWDF_REQUEST_COMPLETION_PARAMS CompletionParams, WDFCONTEXT Context);

static VOID dbus_usbos_rx_flowctl(WDFWORKITEM  WorkItem);

#else	/* WDF_API */

static NTSTATUS dbus_usbos_urb_send_buf_async_wdm(usbos_info_t* usbinfo, PTCB pTCB);
static NTSTATUS dbus_usbos_urb_complete_sync_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Cxt);

static NTSTATUS dbus_usbos_tx_complete_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);
static NTSTATUS dbus_usbos_rx_complete_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);

static VOID dbus_usbos_txctl_workitem(WDFWORKITEM  WorkItem);

#endif	/* WDF_API */

#else	/* (NDISVER < 0x0600) */

static NTSTATUS dbus_usbos_urb_send_buf_async_wdm(usbos_info_t* usbinfo, PTCB pTCB);
static NTSTATUS dbus_usbos_urb_complete_sync_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Cxt);
static NTSTATUS dbus_usbos_urb_complete_sync_cb2(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Cxt);

static NTSTATUS dbus_usbos_tx_complete_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);
static NTSTATUS dbus_usbos_rx_complete_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);
static VOID dbus_usbos_txctl_workitem(PNDIS_WORK_ITEM WorkItem, PVOID Context);
static VOID dbus_usbos_rx_reset(PNDIS_WORK_ITEM WorkItem, PVOID Context);

#endif	

/* init/deinit */
static int  dbus_usbos_init_device(usbos_info_t* usbinfo);
static void dbus_usbos_disconnect(void);
static bool dbus_usbos_init_readselect_desc_wdm(usbos_info_t* usbinfo);
static bool dbus_usbos_init_select_interfaces_wdm(usbos_info_t*, PUSB_CONFIGURATION_DESCRIPTOR);
static VOID dbus_usbos_init_parse_pipes_wdm(usbos_info_t *usbinfo);
static bool dbus_usbos_init_config_device_wdm(usbos_info_t* usbinfo);
static int dbus_usbos_get_device_speed(usbos_info_t* usbinfo);

static bool dbus_usbos_txrx_resource_init(usbos_info_t *usbinfo);

static void dbus_usbos_deinit_abort_pipes(usbos_info_t *usbinfo);
static void dbus_usbos_deinit_abortreset_pipe(usbos_info_t *usbinfo, USBD_PIPE_HANDLE, bool);
static NTSTATUS dbus_usbos_deinit_reset_parentportbulk(PDEVICE_OBJECT DeviceObject);

/* utility */
static void dbusos_stop(usbos_info_t *usbos_info);	/* steal the high layer state */
static void dbus_usbos_urb_buildgeneric(PURB urb);
bool dbus_usbos_dl_cmd(usbos_info_t *usbinfo, UCHAR cmd,  void *buffer, int len);
int dbus_write_membytes(usbos_info_t *usbinfo, bool set, uint32 address, uint8 *data, uint size);

/* control pipe */
static NTSTATUS dbus_usbos_ctl_post_vendorreq(usbos_info_t *usbinfo, BOOLEAN, void *, int, bool);
static int dbus_usbos_ctl_tx_send_ioctl(usbos_info_t *usbinfo, CHAR *Buffer, ULONG Len, ULONG Out);
static int dbus_usbos_ctl_rx_async(usbos_info_t *usbinfo, uchar *msg, uint msglen);
static int dbus_usbos_acquire_mutex(PKMUTEX mutex);
static int dbus_usbos_release_mutex(PKMUTEX mutex);

/* tx/rx */
static bool dbus_usbos_tx_alloc(usbos_info_t* usbinfo, uint max_ntxq);
static void dbus_usbos_tx_free(usbos_info_t* usbinfo);
static void dbus_usbos_tx_freebusypkt(usbos_info_t* usbinfo, bool all);
static void dbus_usbos_tx_timeout_fn(PVOID sys_1, PVOID context, PVOID sys_2, PVOID sys_3);
static void dbus_usbos_tx_return_tcb(usbos_info_t *usbos_info, PTCB pTCB);
static int  dbus_usbos_tx_send_irb_async(usbos_info_t *, dbus_irb_tx_t*);

static bool dbus_usbos_rx_alloc(usbos_info_t* usbinfo, uint max_nrxq);
static void dbus_usbos_rx_free(usbos_info_t* usbinfo);
static void dbus_usbos_rx_freebusypkt(usbos_info_t* usbinfo, bool all);
static void dbus_usbos_rx_return_rcb(PRCB pRCB);
static int  dbus_usbos_rx_post_rcb(usbos_info_t *usbinfo, PRCB pRCB);
static int  dbus_usbos_rx_data_sendup(usbos_info_t *usbinfo, dbus_irb_rx_t *, uchar *p, uint len);
static void dbus_usbos_rx_data_sendup_null(usbos_info_t *usbinfo, dbus_irb_rx_t *rxirb,
	NTSTATUS ntStatus);

static void dbus_usbos_txrx_urb_waitcomplete(usbos_info_t* usbinfo);
static dbus_irb_t * dbus_usbos_txrx_getirb(usbos_info_t *usbinfo, bool send);

/* low level URB submit and completion callback */
static NTSTATUS dbus_usbos_urb_recv_buf_async(usbos_info_t *usbos_info, PRCB pRCB);

static NTSTATUS dbus_usbos_urb_submit_async(usbos_info_t *usbinfo, PURB Urb);
static NTSTATUS dbus_usbos_urb_complete_async_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Cxt);

static NTSTATUS dbus_usbos_urb_submit_sync(usbos_info_t *usbos_info, PURB Urb, bool blocking);
static NTSTATUS dbus_usbos_urb_submit_sync2(usbos_info_t *usbinfo, PURB Urb);
static NTSTATUS dbus_usbos_irp_complete_sync_cb
	(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);

static LONG     dbus_usbos_urb_ioinc_bulk(usbos_info_t* usbinfo);
static LONG     dbus_usbos_urb_iodec_bulk(usbos_info_t* usbinfo);

static bool  dbus_usbos_ctl_send_checkcrc(usbos_info_t* usbinfo);

/* high level interface APIs */
static void *dbus_usbos_intf_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs);
static void  dbus_usbos_intf_detach(dbus_pub_t *pub, void *info);
static int   dbus_usbos_intf_get_attrib(void *bus, dbus_attrib_t *attrib);
static int   dbus_usbos_intf_pnp(void *bus, int event);
static int   dbus_usbos_intf_up(void *bus);
static int   dbus_usbos_intf_down(void *bus);
static int   dbus_usbos_intf_stop(void *bus);
static int   dbus_usbos_intf_shutdown(void *bus);
static int   dbus_usbos_intf_set_config(void *bus, dbus_config_t *config);

static void *dbus_usbos_intf_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args);
static void *dbus_usbos_intf_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args);
static int   dbus_usbos_intf_tx_timer_init(void *bus);
static int   dbus_usbos_intf_tx_timer_start(void *bus, uint timeout);
static int   dbus_usbos_intf_tx_timer_stop(void *bus);
static int   dbus_usbos_intf_send_irb(void *bus, struct dbus_irb_tx *txirb);
static int   dbus_usbos_intf_recv_irb(void *bus, struct dbus_irb_rx *rxirb);
static int   dbus_usbos_intf_send_cancel_irb(void *bus, dbus_irb_tx_t *txirb);
static int   dbus_usbos_intf_recv_ctl(void *bus, uint8 *buf, int len);
static int   dbus_usbos_intf_send_ctl(void *bus, void *pkt, int len);	/* not used for RPC */
static bool  dbus_usbos_intf_recv_needed(void *bus);
static int   dbus_usbos_intf_recv_stop(void *bus);
static int   dbus_usbos_intf_recv_resume(void *bus);

static dbus_intf_t dbus_usbos_intf = {
	dbus_usbos_intf_attach,
	dbus_usbos_intf_detach,
	dbus_usbos_intf_up,
	dbus_usbos_intf_down,
	dbus_usbos_intf_send_irb,
#ifdef WDF_API
	NULL,	/* contread, no need of rxirb */
#else
	dbus_usbos_intf_recv_irb,
#endif
	dbus_usbos_intf_send_cancel_irb,
	dbus_usbos_intf_send_ctl,
	dbus_usbos_intf_recv_ctl,
	NULL, /* get_stats */

	dbus_usbos_intf_get_attrib,

	dbus_usbos_intf_pnp,
	NULL, /* remove */
	NULL, /* resume */
	NULL, /* suspend */
	dbus_usbos_intf_stop,
	NULL, /* reset */
	NULL, /* pktget */
	NULL, /* pktfree */
	NULL, /* iovar_op */
	NULL, /* dump */
	dbus_usbos_intf_set_config, /* set_config */
	NULL, /* get_config */
	NULL, /* device_exists */
	NULL, /* dl_isneeded */
	NULL, /* dl_start */
	NULL, /* dl_run */
	dbus_usbos_intf_recv_needed,
	dbus_usbos_intf_exec_rxlock,
	dbus_usbos_intf_exec_txlock,
	dbus_usbos_intf_tx_timer_init,
	dbus_usbos_intf_tx_timer_start,
	dbus_usbos_intf_tx_timer_stop,
	NULL, /* sched_dpc */
	NULL, /* lock */
	NULL, /* unlock */
	NULL, /* sched_probe_cb */
	dbus_usbos_intf_shutdown, /* shutdown */

	dbus_usbos_intf_recv_stop,
	dbus_usbos_intf_recv_resume
};

/* global variables */
static probe_cb_t	probe_cb = NULL;
static void		*probe_arg = NULL;
static disconnect_cb_t	disconnect_cb = NULL;
static void		*disc_arg = NULL;

/* Create the generic URB for getting status/issueing cmds */
static void
dbus_usbos_urb_buildgeneric(PURB urb)
{
	urb->UrbHeader.Function = URB_FUNCTION_VENDOR_INTERFACE;
	urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
	urb->UrbControlVendorClassRequest.Reserved = 0;
	urb->UrbControlVendorClassRequest.TransferFlags =
		USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
	urb->UrbControlVendorClassRequest.RequestTypeReservedBits = 0;
	urb->UrbControlVendorClassRequest.Value = (USHORT)1;
	urb->UrbControlVendorClassRequest.Index = (USHORT)0;
	urb->UrbControlVendorClassRequest.Reserved1 = 0;
	urb->UrbControlVendorClassRequest.UrbLink = NULL;
}

static void
dbus_usbos_deinit_abortreset_pipe(usbos_info_t *usbinfo, USBD_PIPE_HANDLE PipeHandle, bool reset)
{
	osl_t *osh = usbinfo->pub->osh;
	struct _URB_PIPE_REQUEST pipe_req;
	PURB urb;
	NTSTATUS ntStatus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	/* Device already gone, skip calling dbus_usbos_urb_submit_sync2
	 * which will anyway fail and can block indefinitely
	 */
	if (usbinfo->pnp_isdown)
		return;

	urb = (PURB)&pipe_req;
	urb->UrbHeader.Length = sizeof(struct _URB_PIPE_REQUEST);
	urb->UrbHeader.Function = (reset) ? URB_FUNCTION_RESET_PIPE : URB_FUNCTION_ABORT_PIPE;
	urb->UrbPipeRequest.PipeHandle = PipeHandle;

	ntStatus = dbus_usbos_urb_submit_sync2(usbinfo, urb);

	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: dbus_usbos_urb_submit_sync2 failed 0x%x\n", __FUNCTION__, ntStatus));
	}
}

static void
dbus_usbos_deinit_abort_pipes(usbos_info_t *usbinfo)
{
	ULONG i, numberConfiguredPipes;

	DBUSTRACE(("%s\n", __FUNCTION__));

#ifdef WDF_API
	dbus_usbos_stop_all_pipes_wdf(usbinfo);
	return;
#else
	if (usbinfo->UsbInterface) {
		numberConfiguredPipes = usbinfo->UsbInterface->NumberOfPipes;
		if (numberConfiguredPipes > DBUS_USBOS_MAX_PIPES) {
			numberConfiguredPipes = DBUS_USBOS_MAX_PIPES;
		}

		for (i = 0; i < numberConfiguredPipes; i++) {
			if (dbus_usbos_acquire_mutex(&usbinfo->purge_mutex) == DBUS_OK) {
				dbus_usbos_deinit_abortreset_pipe(usbinfo,
					usbinfo->UsbInterface->Pipes[i].PipeHandle, FALSE);
				dbus_usbos_release_mutex(&usbinfo->purge_mutex);
			}
		}
	}
#endif /* WDF_API */
}

static NTSTATUS
dbus_usbos_deinit_reset_parentportbulk(PDEVICE_OBJECT DeviceObject)
{
	KEVENT e;
	PIRP irp;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	IO_STATUS_BLOCK ioStatus;
	PIO_STACK_LOCATION nextStack;

	DBUSERR(("%s: begins\n", __FUNCTION__));

	KeInitializeEvent(&e, NotificationEvent, FALSE);


	irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_CYCLE_PORT, DeviceObject,
		NULL, 0, NULL, 0, TRUE, &e, &ioStatus);
	if (irp == NULL) {
		DBUSERR(("%s: memory alloc for irp failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}

	nextStack = IoGetNextIrpStackLocation(irp);
	ntStatus = IoCallDriver(DeviceObject, irp);
	if (ntStatus == STATUS_PENDING) {
		KeWaitForSingleObject(&e, Executive, KernelMode, FALSE, NULL);
	} else {
		ioStatus.Status = ntStatus;
	}

	ntStatus = ioStatus.Status;

	DBUSERR(("%s: ends\n", __FUNCTION__));
	return ntStatus;
}

bool
dbus_usbos_dl_send_bulk(usbos_info_t *usbinfo, void *buffer, int len)
{
	NTSTATUS  ntStatus;
	USBD_PIPE_HANDLE pipe;
	bool ret;
	PURB urb = NULL;
	struct _URB_BULK_OR_INTERRUPT_TRANSFER bulkurb_req;

	DBUSTRACE(("%s\n", __FUNCTION__));

	urb = (PURB)&bulkurb_req;

#ifdef WDM_API
	pipe = usbinfo->TxPipeHandleWdm;
	if (pipe == NULL) {
		return FALSE;
	}

	/* Create the bulk urb */
	UsbBuildInterruptOrBulkTransferRequest(urb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		pipe,
		buffer,
		NULL,
		len,
		0,
		NULL);
	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, TRUE);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: dbus_usbos_urb_submit_sync failed 0x%x\n", __FUNCTION__, ntStatus));
	}
	return (NT_SUCCESS(ntStatus));
#else
	dbus_usbos_urb_ioinc_bulk(usbinfo);
	ret = dbus_usbos_tx_send_buf_sync_wdf(usbinfo, len, buffer, NULL);
	dbus_usbos_urb_iodec_bulk(usbinfo);
	return ret;
#endif	/* WDM_API */
}

static int
dbus_usbos_intf_tx_timer_init(void *bus)
{
	usbos_info_t* usbos_info = (usbos_info_t*)bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;


	return DBUS_OK;
}

static int
dbus_usbos_intf_tx_timer_start(void *bus, uint timeout)
{
	usbos_info_t* usbos_info = (usbos_info_t*)bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (usbos_info->pub->busstate == DBUS_STATE_DOWN) {
		DBUSERR(("%s: Timer start called when DBUS down, ignoring\n", __FUNCTION__));
		return DBUS_ERR;
	}


	return DBUS_OK;
}

static int
dbus_usbos_intf_tx_timer_stop(void *bus)
{
	usbos_info_t* usbos_info = (usbos_info_t*)bus;
	bool cancelled = FALSE;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

	return DBUS_OK;

}

/* tx timeout, loop through each one in busy_list, invoke the callback */
static void
dbus_usbos_tx_timeout_fn(PVOID sys_1, PVOID context, PVOID sys_2, PVOID sys_3)
{
	PLIST_ENTRY pEntry = NULL;
#ifdef WDM_API
	PTCB pTCB = NULL;
#else
	WDFREQUEST writeRequest;
	dbus_usbos_TCB_wdf *writeContext;
#endif
	usbos_info_t *usbos_info = (usbos_info_t*)context;
	dbus_irb_tx_t *txirb = NULL;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return;

	if (usbos_info->pub->busstate == DBUS_STATE_DOWN) {
		DBUSTRACE(("%s: Timeout fn called when DBUS down, ignoring\n", __FUNCTION__));
		return;
	}

	return;
}

/* Wait for all URBs to complete before freeing memory */
static void
dbus_usbos_txrx_urb_waitcomplete(usbos_info_t* usbinfo)
{
	int timeoutCount = 6000; /* 6s */

	DBUSTRACE(("%s\n", __FUNCTION__));

	while (usbinfo->rx_pkt_busyrecv || usbinfo->tx_pkt_busysend) {
		if (--timeoutCount == 0) {
			DBUSERR(("%s: URB cleanup timeout!!!\n", __FUNCTION__));
			break;
		}

		NdisMSleep(1000); /* 1ms */
	}
	ASSERT(usbinfo->rx_pkt_busyrecv == 0);
	ASSERT(usbinfo->tx_pkt_busysend == 0);

	DBUSTRACE(("%s: tx_pkt_busysend = %d rx_pkt_busyrecv = %d\n",
		__FUNCTION__, usbinfo->tx_pkt_busysend, usbinfo->rx_pkt_busyrecv));
}

static dbus_irb_t *
dbus_usbos_txrx_getirb(usbos_info_t *usbinfo, bool send)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return NULL;

	if (usbinfo->cbs && usbinfo->cbs->getirb)
		return usbinfo->cbs->getirb(usbinfo->cbarg, send);

	return NULL;
}

/*
 * This function tries to cancel all the outstanding read IRP if it is not
 * already completed and frees the RCB block. This routine is called only by the Halt handler.
 */
static VOID
dbus_usbos_rx_freebusypkt(usbos_info_t* usbinfo, bool all)
{
	PLIST_ENTRY pEntry = NULL;
	PRCB pRCB = NULL;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (!MP_TEST_FLAG(usbinfo, fMP_RX_RESOURCE_ALLOCATED))
		return;

	while (!IsListEmpty(&usbinfo->rx_busy_list)) {
		pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(&usbinfo->rx_busy_list,
			&usbinfo->rx_spinlock);

		pRCB = CONTAINING_RECORD(pEntry, RCB, List);
		NdisInitializeListHead(&pRCB->List);

		if (InterlockedExchange((PVOID)&pRCB->IrpLock, IRPLOCK_CANCEL_STARTED)
			== IRPLOCK_CANCELABLE) {

			/*
			 * We got it to the IRP before it was completed. We can cancel
			 * the IRP without fear of losing it, as the completion routine
			 * will not let go of the IRP until we say so.
			 */

			DBUSTRACE(("%s: calling IoCancelIrp\n", __FUNCTION__));
			if (pRCB->Irp)
				IoCancelIrp(pRCB->Irp);

			/*
			 * Update IrpLock status so that the completion routine
			 * can continue processing the Irp. If completion
			 * routine already executed cleanup here!
			 */
			if (InterlockedExchange((PVOID)&pRCB->IrpLock, IRPLOCK_CANCEL_COMPLETE)
				== IRPLOCK_COMPLETED) {
				/* Need to do callback to return rxirb to dbus.c */
				dbus_usbos_rx_data_sendup_null(usbinfo, pRCB->rxirb,
					STATUS_CANCELLED);

				/* DONE, free the request */
				if (NdisInterlockedDecrement(&pRCB->Ref) == 0) {
					dbus_usbos_rx_return_rcb(pRCB);
				}
			}

			if (all == FALSE)
				goto exit;
		}
	}

exit:
	return;
}

/* This routine reinitializes the RCB block and puts it back into the rx_free_list for reuse. */
static void
dbus_usbos_rx_return_rcb(PRCB pRCB)
{
	usbos_info_t* usbinfo = pRCB->usbinfo;

	DBUSTRACE(("%s: pRCB = %p\n", __FUNCTION__, pRCB));

	ASSERT(pRCB->Irp);
	ASSERT(!pRCB->Ref);
	ASSERT(pRCB->usbinfo);

	IoReuseIrp(pRCB->Irp, STATUS_SUCCESS);

	/* Set the MDL field to NULL so don't double freeing the MDL in our call to IoFreeIrp. */
	pRCB->Irp->MdlAddress = NULL;
	pRCB->rxirb = NULL;

#if (0>= 0x0600)
	NdisAdjustMdlLength(pRCB->pMdl, DBUS_BUFFER_SIZE_RX);
#endif

	/* Insert the RCB back in the Recv free list */
	NdisAcquireSpinLock(&usbinfo->rx_spinlock);
	RemoveEntryList(&pRCB->List);
	NdisReleaseSpinLock(&usbinfo->rx_spinlock);

	NdisInterlockedDecrement(&usbinfo->rx_pkt_busyrecv);
	NdisInterlockedInsertTailList(&usbinfo->rx_free_list, &pRCB->List, &usbinfo->rx_spinlock);
	ASSERT(usbinfo->rx_pkt_busyrecv >= 0);
}

static int
dbus_usbos_rx_post_rcb(usbos_info_t *usbinfo, PRCB pRCB)
{
	int dbus_status;
	NTSTATUS ntStatus;

	ASSERT(usbinfo);

	DBUSTRACE(("%s\n", __FUNCTION__));

	/* Insert the RCB in the recv busy queue */
	NdisInterlockedIncrement(&usbinfo->rx_pkt_busyrecv);
	if (usbinfo->rx_pkt_busyrecv > usbinfo->pub->nrxq) {
		DBUSERR(("%s: Warn: rx_pkt_busyrecv(%d) > usbinfo->pub->nrxq(%d)\n",
			__FUNCTION__, usbinfo->rx_pkt_busyrecv, usbinfo->pub->nrxq));
	}

	NdisInterlockedInsertTailList(&usbinfo->rx_busy_list, &pRCB->List, &usbinfo->rx_spinlock);

	ntStatus = dbus_usbos_urb_recv_buf_async(usbinfo, pRCB);
	if (!NT_SUCCESS(ntStatus))
		dbus_status = (DEVICEREMOVED(ntStatus)) ? DBUS_ERR_NODEVICE : DBUS_ERR_RXFAIL;
	else
		dbus_status = DBUS_OK;

	if (dbus_status != DBUS_OK) {
		DBUSERR(("%s: dbus_usbos_urb_recv_buf_async failed, dbus status %d ntStatus 0x%x\n",
			__FUNCTION__, dbus_status, ntStatus));
		/* FIX: Re-insert back to queue and clear */
		/* dbus_usbos_rx_return_rcb(pRCB); */
	}

	return dbus_status;
}

/*
 *  This routine sends a read IRP to the target device to get
 *  the incoming network packet from the device.
 */
static NTSTATUS
dbus_usbos_urb_recv_buf_async(usbos_info_t* usbinfo, PRCB pRCB)
{
#ifdef WDM_API
	ULONG TransferFlags;
	NTSTATUS status;
	PIRP irp = pRCB->Irp;
	PIO_STACK_LOCATION  nextStack;
	PDEVICE_OBJECT TargetDO = usbinfo->nextDeviceObj;

	DBUSTRACE(("%s\n", __FUNCTION__));

	TransferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;

#if (0>= 0x0600)
	UsbBuildInterruptOrBulkTransferRequest(pRCB->Urb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		usbinfo->RxPipeHandleWdm,
		NULL,
		pRCB->pMdl,	/* Vista uses MDLs */
		DBUS_BUFFER_SIZE_RX,
		TransferFlags,
		NULL);
#else
	UsbBuildInterruptOrBulkTransferRequest(pRCB->Urb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		usbinfo->RxPipeHandleWdm,
		pRCB->pData_ndis5,
		NULL,
		DBUS_BUFFER_SIZE_RX,
		TransferFlags,
		NULL);

#endif 

	/*
	 * Obtain a pointer to the stack location of the first driver that will be
	 * invoked.  This is where the function codes and the parameters are set.
	 */
	nextStack = IoGetNextIrpStackLocation(irp);
	nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	nextStack->Parameters.Others.Argument1 = (PVOID) pRCB->Urb;
	nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

	pRCB->IrpLock = IRPLOCK_CANCELABLE;
	pRCB->Ref = 1;

	IoSetCompletionRoutine(irp, dbus_usbos_rx_complete_cb, pRCB, TRUE, TRUE, TRUE);
	status = IoCallDriver(TargetDO, irp);

	return status;
#else
	return -1;
#endif /* WDM_API */
}


/*
 * Completion routine for the read request. This routine
 * indicates the received packet from the WDM driver to
 * NDIS. This routine also handles the case where another
 * thread has canceled the read request.
 */
NTSTATUS
dbus_usbos_rx_complete_cb(
#ifdef WDM_API
	PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context
#else
	WDFREQUEST Request, WDFIOTARGET Target,
	PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
	WDFCONTEXT Context
#endif
)
{
	PRCB pRCB = (PRCB)Context;
	usbos_info_t* usbinfo = pRCB->usbinfo;
	dbus_irb_rx_t *rxirb = pRCB->rxirb;
	ULONG bytesRead;
	BOOLEAN bIndicateReceive = FALSE;
	PURB urb = pRCB->Urb;
	NTSTATUS status;
	int result = 0;

	DBUSTRACE(("%s\n", __FUNCTION__));

#ifdef WDM_API
	status = Irp->IoStatus.Status;
#else
	UNREFERENCED_PARAMETER(Target);
	UNREFERENCED_PARAMETER(Request);
	status = CompletionParams->IoStatus.Status;
#endif

#ifdef WDM_API
	/*
	 * If Cancel has started on this irp, let the
	 * cancel routine cleanup. If not, continue processing
	 */
	if (InterlockedExchange((PVOID)&pRCB->IrpLock, IRPLOCK_COMPLETED)
		== IRPLOCK_CANCEL_STARTED) {
		return STATUS_MORE_PROCESSING_REQUIRED;
	}
#endif

	if (!NT_SUCCESS(status)) {
		if (MP_ISSET_FLAG(usbinfo, fMP_POST_READS)) {
			DBUSERR(("%s, Read request failed 0x%x\n", __FUNCTION__, status));
		}

		if (!USBD_SUCCESS(pRCB->Urb->UrbHeader.Status)) {
			/* FIX: After unplug, this happens */
			if (MP_ISSET_FLAG(usbinfo, fMP_POST_READS))
				DBUSERR(("%s, urb failed with status = 0x%x\n", __FUNCTION__,
					pRCB->Urb->UrbHeader.Status));
		}

		/* Clear the flag to prevent any more reads from being posted to the target */
		MP_CLEAR_FLAG(usbinfo, fMP_POST_READS);

		bytesRead = 0;
	} else {
#ifdef WDM_API
		bytesRead = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
#else
		bytesRead = (ULONG)CompletionParams->IoStatus.Information;
#endif
		if (bytesRead) {
			usbinfo->rx_totbytesread += bytesRead;
			bIndicateReceive = TRUE;
		} else {
			DBUSERR(("%s: ********** packet is 0 byte ************\n", __FUNCTION__));
			bIndicateReceive = FALSE;
		}
	}

	if (bIndicateReceive) {
		DBUSTRACE(("%s, bytes read %d\n", __FUNCTION__, bytesRead));
#if (0< 0x0600)
		result = dbus_usbos_rx_data_sendup(usbinfo, pRCB->rxirb, (uchar*)pRCB->rxDataBuf,
			bytesRead);
			if (usbinfo->rx_reset_count) {
				NdisAcquireSpinLock(&usbinfo->rx_spinlock);
				usbinfo->rx_reset_count = 0;
				NdisReleaseSpinLock(&usbinfo->rx_spinlock);
			}
#else
		{
			PETH_HEADER     pEthHeader = NULL;
			PNET_BUFFER     NetBuffer;
			ULONG           BytesAvailable;

			NetBuffer = NET_BUFFER_LIST_FIRST_NB(pRCB->NetBufferList);

			/*
			 * During the call NdisAllocateNetBufferAndNetBufferList to
			 * allocate the NET_BUFFER_LIST, NDIS already
			 * initializes DataOffset, CurrentMdl and CurrentMdlOffset.
			 * We just need to update DataLength
			 * in the NET_BUFFER to reflect the received frame size.
			 */
			NET_BUFFER_DATA_LENGTH(NetBuffer) = bytesRead;
			NdisAdjustMdlLength(pRCB->pMdl, bytesRead);

			MP_SET_RCB_IN_NBL(pRCB->NetBufferList, pRCB);
			NdisQueryMdl(pRCB->pMdl, (PVOID)&pEthHeader, &BytesAvailable,
				NormalPagePriority);
			result = dbus_usbos_rx_data_sendup(usbinfo, pRCB->rxirb, (uchar*)pEthHeader,
				bytesRead);

		}
#endif 

		if (NdisInterlockedDecrement(&pRCB->Ref) == 0) {
			dbus_usbos_rx_return_rcb(pRCB);
		}
	}

	/* if dbus_usbos_rx_data_sendup is not called, still need to call recv_irb_complete() */
	if ((result != DBUS_OK) || (bIndicateReceive == FALSE)) {
		if (NdisInterlockedDecrement(&pRCB->Ref) == 0) {
			dbus_usbos_rx_return_rcb(pRCB);
		}

#if (0< 0x0600)
		if (pRCB->Urb->UrbHeader.Status == USBD_STATUS_XACT_ERROR) {
			NdisAcquireSpinLock(&usbinfo->rx_spinlock);
			if (usbinfo->rx_reset_count < RX_RESET_THRESHOLD) {
				usbinfo->rx_reset_count++;
				if (!usbinfo->rx_reset_on) {
					usbinfo->rx_reset_on = TRUE;
					if (usbinfo->cbarg && usbinfo->cbs &&
						usbinfo->cbs->rxerr_indicate)
						usbinfo->cbs->rxerr_indicate(usbinfo->cbarg, TRUE);

					DBUSERR(("%s: USBD_STATUS_XACT_ERROR, scheduling "
						"rx_reset_WorkItem, rx_reset_count = %d\n",
						__FUNCTION__, usbinfo->rx_reset_count));
					NdisScheduleWorkItem(&usbinfo->rx_reset_WorkItem);
				}
			}
			NdisReleaseSpinLock(&usbinfo->rx_spinlock);
		}
#endif 
		/* Need to do callback to return rxirb to dbus.c */
		dbus_usbos_rx_data_sendup_null(usbinfo, rxirb, status);
	}

	return STATUS_MORE_PROCESSING_REQUIRED;
}

static bool
dbus_usbos_rx_alloc(usbos_info_t* usbinfo, uint max_nrxq)
{
	osl_t *osh = usbinfo->pub->osh;
	PUCHAR pRCBMem = NULL, pBuf = NULL;
	uint i;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	NTSTATUS ntStatus;
	bool ret = FALSE;
#if (0< 0x0600)
	PNDIS_PACKET Packet = NULL;
#else
	WDF_OBJECT_ATTRIBUTES attributes;
	WDF_WORKITEM_CONFIG workitemConfig;
	NET_BUFFER_LIST_POOL_PARAMETERS ListPoolParameters;
	NET_BUFFER_POOL_PARAMETERS BufferPoolParameters;
#endif

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (MP_TEST_FLAG(usbinfo, fMP_RX_RESOURCE_ALLOCATED))
		return TRUE;

#ifdef WDF_API
	/* WDF_API use cont rx model */
	if (dbus_usbos_rx_res_alloc_wdf(usbinfo)) {
		if (dbus_usbos_rx_start_wdf(usbinfo))
			ret = TRUE;
	} else {
		DBUSERR(("%s: dbus_usbos_rx_res/start_wdf failed\n", __FUNCTION__));
	}
	/* done */
#else

	Status = NdisAllocateMemoryWithTag(&pBuf, DBUS_BUFFER_SIZE_RX * max_nrxq, NDIS_TAG_USBDRV);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: Failed to allocate memory for rx buffer\n", __FUNCTION__));
		goto error;
	}
	NdisZeroMemory(pBuf, DBUS_BUFFER_SIZE_RX * max_nrxq);
	usbinfo->p_rxbuf_memory = pBuf;

	Status = NdisAllocateMemoryWithTag(&pRCBMem, sizeof(RCB) * max_nrxq, NDIS_TAG_USBDRV);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: Failed to allocate memory for RCB's\n", __FUNCTION__));
		goto error;
	}
	NdisZeroMemory(pRCBMem, sizeof(RCB) * max_nrxq);
	usbinfo->p_RCB_memory = pRCBMem;

	/*
	 * Following are the lists to hold packets at different stages of processing.
	 * rx_free_list - Packets available for received operation
	 * rx_busy_list - Packets posted  to the lower WDM stack
	 * rx_spinlock is used to synchronize access to these lists.
	 */
	NdisInitializeListHead(&usbinfo->rx_free_list);
	NdisInitializeListHead(&usbinfo->rx_busy_list);
	NdisAllocateSpinLock(&usbinfo->rx_spinlock);

	/* Allocate a buffer pool for recv buffers */
#if (0< 0x0600)
	NdisAllocateBufferPool(&Status, &usbinfo->RecvBufferPoolHandle,	max_nrxq);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: NdisAllocateBufferPool for recv buffer failed\n", __FUNCTION__));
		goto error;
	}
	/* Allocate packet pool for receive indications */
	NdisAllocatePacketPool(&Status,	&usbinfo->RecvPacketPoolHandle,	max_nrxq,
		PROTOCOL_RESERVED_SIZE_IN_PACKET);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: NdisAllocatePacketPool failed\n", __FUNCTION__));
		goto error;
	}

	NdisInitializeWorkItem(&usbinfo->rx_reset_WorkItem, dbus_usbos_rx_reset, usbinfo);

#else /* (NDISVER >= 0x0600) */
	/*
	 * alloc the recv net buffer list pool with net buffer inside the list
	 *   do it before the RPC call so if this fail we don't bother with the RPC upcall at all
	 */
	NdisZeroMemory(&ListPoolParameters, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));
	ListPoolParameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	ListPoolParameters.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
	ListPoolParameters.Header.Size = sizeof(ListPoolParameters);
	ListPoolParameters.ProtocolId = 0;
	ListPoolParameters.ContextSize = 0;
	ListPoolParameters.fAllocateNetBuffer = TRUE;
	ListPoolParameters.PoolTag = NDIS_TAG_USBDRV;
	usbinfo->Rx_NetBufferListPool = NdisAllocateNetBufferListPool(usbinfo->AdapterHandle,
		&ListPoolParameters);
	if (usbinfo->Rx_NetBufferListPool == NULL) {
		DBUSERR(("%s: NdisAllocateNetBufferListPool for Recv failed\n", __FUNCTION__));
		Status = NDIS_STATUS_RESOURCES;
		goto error;
	}

	NdisZeroMemory(&BufferPoolParameters, sizeof(NET_BUFFER_POOL_PARAMETERS));
	BufferPoolParameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	BufferPoolParameters.Header.Revision = NET_BUFFER_POOL_PARAMETERS_REVISION_1;
	BufferPoolParameters.Header.Size = sizeof(BufferPoolParameters);
	BufferPoolParameters.PoolTag = NDIS_TAG_USBDRV;
	usbinfo->Rx_NetBufferPool = NdisAllocateNetBufferPool(usbinfo->AdapterHandle,
		&BufferPoolParameters);
	if (usbinfo->Rx_NetBufferPool == NULL) {
		DBUSERR(("%s: NdisAllocateNetBufferPool for Recv failed\n", __FUNCTION__));
		Status = NDIS_STATUS_RESOURCES;
		goto error;
	}
#endif 

	/* Divide p_RCB_memory block into RCBs, create a buf descriptor for the RCBs
	 *   continuous Data portion
	 */
	for (i = 0; i < max_nrxq; i++) {
		PRCB pRCB = (PRCB)pRCBMem;
		pRCB->rxDataBuf = pBuf;

		pRCB->usbinfo = usbinfo;

		/* Preallocate IRPs. Do not charge quota to the current process for this IRP. */
		pRCB->Irp = IoAllocateIrp(usbinfo->nextDeviceObj->StackSize, FALSE);
		if (pRCB->Irp == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			goto error;
		}

		pRCB->IrpLock = IRPLOCK_COMPLETED;
		pRCB->Urb = MALLOC(osh,	sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
		if (pRCB->Urb == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			goto error;
		}


#if (0>= 0x0600)
		/* Create a buffer descriptor(MDLs) for the Data portion of the RCBs. */
		pRCB->pMdl = NdisAllocateMdl(usbinfo->AdapterHandle, (PVOID)pRCB->rxDataBuf,
			DBUS_BUFFER_SIZE_RX);
		if (pRCB->pMdl == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			DBUSERR(("%s: NdisAllocateMdl for Recv failed\n", __FUNCTION__));
			goto error;
		}

		pRCB->NetBufferList = NdisAllocateNetBufferAndNetBufferList(
			usbinfo->Rx_NetBufferListPool,
			0,
			0,
			pRCB->pMdl,
			0,
			DBUS_BUFFER_SIZE_RX);
		if (pRCB->NetBufferList == NULL) {
			DBUSERR(("%s: NdisAllocateNetBufferList failed\n", __FUNCTION__));
			Status = NDIS_STATUS_RESOURCES;
			goto error;
		}
		pRCB->NetBuffer = NET_BUFFER_LIST_FIRST_NB(pRCB->NetBufferList);
		/* Store the RCB value in the miniport reserved field of NBL. */
		MP_SET_RCB_IN_NBL(pRCB->NetBufferList, pRCB);

#else /* (NDISVER < 0x0600) */

		/* Allocate a packet descriptor for receive packets from a preallocated pool */
		NdisAllocatePacket(&Status, &Packet, usbinfo->RecvPacketPoolHandle);
		if (Status != NDIS_STATUS_SUCCESS) {
			DBUSERR(("%s: NdisAllocatePacket failed\n", __FUNCTION__));
			goto error;
		}
		pRCB->Packet_ndis5 = Packet;
		NDIS_SET_PACKET_HEADER_SIZE(Packet, ETH_HEADER_SIZE);
		pRCB->Buffer_ndis5 = NULL;

		pRCB->pData_ndis5 = (PUCHAR)pRCB->rxDataBuf;
#endif 

		NdisInterlockedInsertTailList(&usbinfo->rx_free_list, &pRCB->List,
			&usbinfo->rx_spinlock);
		pRCBMem = pRCBMem + sizeof(RCB);
		pBuf = pBuf + DBUS_BUFFER_SIZE_RX;
	}
	ret = (Status == NDIS_STATUS_SUCCESS);

error:
#endif	/* WDF_API */

	if (ret)
		MP_SET_FLAG(usbinfo, fMP_RX_RESOURCE_ALLOCATED);
	else
		DBUSERR(("%s failed\n", __FUNCTION__));

	DBUSTRACE(("%s done\n", __FUNCTION__));
	return ret;
}

static VOID
dbus_usbos_rx_free(usbos_info_t* usbinfo)
{
	osl_t *osh = usbinfo->pub->osh;
	NDIS_STATUS Status;
	PRCB pRCB;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (!MP_TEST_FLAG(usbinfo, fMP_RX_RESOURCE_ALLOCATED))
		return;

	MP_CLEAR_FLAG(usbinfo, fMP_RX_RESOURCE_ALLOCATED);

#ifdef WDF_API
	dbus_usbos_rx_res_dealloc_wdf(usbinfo);
#else
	while (!IsListEmpty(&usbinfo->rx_free_list)) {
		pRCB = (PRCB)NdisInterlockedRemoveHeadList(&usbinfo->rx_free_list,
			&usbinfo->rx_spinlock);
		if (!pRCB)
			break;

#if (0< 0x0600)
		if (pRCB->Buffer_ndis5) {
			NdisFreeBuffer(pRCB->Buffer_ndis5);
			pRCB->Buffer_ndis5 = NULL;
		}

		if (pRCB->Packet_ndis5)
			NdisFreePacket(pRCB->Packet_ndis5);
#else
		if (pRCB->pMdl) {
			NdisFreeMdl(pRCB->pMdl);
			pRCB->pMdl = NULL;
		}

		if (pRCB->NetBufferList) {
			NdisFreeNetBufferList(pRCB->NetBufferList);
			pRCB->NetBufferList = NULL;
		}
		if (pRCB->Request) {
			WdfObjectDelete(pRCB->Request);
			pRCB->Request = NULL;
		}
#endif 

		if (pRCB->Irp)
			IoFreeIrp(pRCB->Irp);
		if (pRCB->Urb)
			MFREE(osh, pRCB->Urb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
	}

#if (0< 0x0600)
	if (usbinfo->RecvPacketPoolHandle) {
		NdisFreePacketPool(usbinfo->RecvPacketPoolHandle);
		usbinfo->RecvPacketPoolHandle = NULL;
	}

	if (usbinfo->RecvBufferPoolHandle) {
		NdisFreeBufferPool(usbinfo->RecvBufferPoolHandle);
		usbinfo->RecvBufferPoolHandle = NULL;
	}
#else /* (NDISVER >= 0x0600) */
	if (usbinfo->Rx_NetBufferListPool) {
		NdisFreeNetBufferListPool(usbinfo->Rx_NetBufferListPool);
		usbinfo->Rx_NetBufferListPool = NULL;
	}

	if (usbinfo->Rx_NetBufferPool) {
		NdisFreeNetBufferPool(usbinfo->Rx_NetBufferPool);
		usbinfo->Rx_NetBufferPool = NULL;
	}
#endif 

	ASSERT(IsListEmpty(&usbinfo->rx_free_list));
	ASSERT(IsListEmpty(&usbinfo->rx_busy_list));

	NdisFreeSpinLock(&usbinfo->rx_spinlock);

#endif	/* WDF_API */

	/* free the buffer memory at the last when all associated structuress are freed */
	if (usbinfo->p_rxbuf_memory) {
		NdisFreeMemory(usbinfo->p_rxbuf_memory,
			DBUS_BUFFER_SIZE_RX * usbinfo->pub->ntxq, 0);
		usbinfo->p_rxbuf_memory = NULL;
	}

	if (usbinfo->p_RCB_memory) {
		NdisFreeMemory(usbinfo->p_RCB_memory, sizeof(RCB) * usbinfo->pub->nrxq, 0);
		usbinfo->p_RCB_memory = NULL;
	}

	return;
}

/*
 *  This routine is called by the Halt handler to cancel any outstanding write requests.
 *
 *  Note: Since we are holding the original sendpacket while writing to the device,
 *    and since NDIS calls halt handler only all the outstanding sends are completed,
 *    this routine will never find any busy sendpackets in the queue.
 *  This routine will be useful if you want to cancel busy sends outside of halt handler
 *    for some reason.
 */
static VOID
dbus_usbos_tx_freebusypkt(usbos_info_t* usbinfo, bool all)
{
	PLIST_ENTRY pEntry = NULL;
	PTCB pTCB = NULL;

	if (!MP_TEST_FLAG(usbinfo, fMP_TX_RESOURCE_ALLOCATED))
		return;

	DBUSTRACE(("%s\n", __FUNCTION__));

	while (!IsListEmpty(&usbinfo->tx_busy_list)) {
		pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(&usbinfo->tx_busy_list,
			&usbinfo->tx_spinlock);

		pTCB = CONTAINING_RECORD(pEntry, TCB, List);
		NdisInitializeListHead(&pTCB->List);

		if (InterlockedExchange((PVOID)&pTCB->IrpLock, IRPLOCK_CANCEL_STARTED)
			== IRPLOCK_CANCELABLE) {

			/*
			 * We got it to the IRP before it was completed. We can cancel
			 * the IRP without fear of losing it, as the completion routine
			 * will not let go of the IRP until we say so.
			 */
			DBUSTRACE(("%s: calling IoCancelIrp\n", __FUNCTION__));
			if (pTCB->Irp)
				IoCancelIrp(pTCB->Irp);

			/*
			 * Update IrpLock status so that the completion routine
			 * can continue processing the Irp. If completion
			 * routine already executed cleanup here!
			 */
			if (InterlockedExchange((PVOID)&pTCB->IrpLock, IRPLOCK_CANCEL_COMPLETE)
				== IRPLOCK_COMPLETED) {
				/* Need to do callback to return txirb to dbus.c */
				if (usbinfo->cbarg && usbinfo->cbs) {
					if (usbinfo->cbs->send_irb_complete && pTCB->txirb)
						usbinfo->cbs->send_irb_complete(usbinfo->cbarg,
							pTCB->txirb, DBUS_STATUS_CANCELLED);
				}

				/* DONE, free the request */
				if (NdisInterlockedDecrement(&pTCB->Ref) == 0) {
					dbus_usbos_tx_return_tcb(usbinfo, pTCB);
				}
			}

			if (all == FALSE)
				goto exit;
		}
	}
exit:
	return;
}

/*
 *  This routine reinitializes the TCB block and puts it back
 *  into the tx_free_list for reuse.
 */
static void
dbus_usbos_tx_return_tcb(usbos_info_t* usbinfo, PTCB pTCB)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	ASSERT(pTCB->Irp);
	ASSERT(!pTCB->Ref);

#if (0>= 0x0600)
	pTCB->pMdl = NULL;
#else
	pTCB->Buffer_ndis5->Next = NULL;
#endif

	/* Reinitialize the IRP for reuse. */
	IoReuseIrp(pTCB->Irp, STATUS_SUCCESS);

	/* Set the MDL to NULL so that we don't end up free the MDL in our call to IoFreeIrp. */
	pTCB->Irp->MdlAddress = NULL;
	pTCB->txirb = NULL;
	/* reset ->pData back to real data buffer in case it was changed in send_irb non-copy */
	pTCB->pData = pTCB->txDataBuf;

	/* Insert the TCB back in the send free list */
	NdisAcquireSpinLock(&usbinfo->tx_spinlock);
	RemoveEntryList(&pTCB->List);
	NdisReleaseSpinLock(&usbinfo->tx_spinlock);

	NdisInterlockedInsertTailList(&usbinfo->tx_free_list, &pTCB->List, &usbinfo->tx_spinlock);
	NdisInterlockedDecrement(&usbinfo->tx_pkt_busysend);
	ASSERT(usbinfo->tx_pkt_busysend >= 0);
}

/*
 * Completion routine for the write request. This routine indicates to NDIS that send is complete
 *   1. invoke the dbus high layer callback
 *   2. recycle the send request resource if nobody has already done so
 * This routine also handles the case where another thread has canceled the write request.
 *
 * Returns STATUS_MORE_PROCESSING_REQUIRED - because this is an asynchronous IRP
 * and we want to reuse it.
 */
static NTSTATUS
dbus_usbos_tx_complete_cb
#ifdef WDM_API
	(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
#else
	(WDFREQUEST Request, WDFIOTARGET Target, PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
	WDFCONTEXT Context)
#endif
{
	NTSTATUS          status;
	int dbus_status;
	usbos_info_t *usbinfo;
#ifdef WDM_API
	PTCB pTCB;
#else
	dbus_usbos_TCB_wdf	*writeContext;
	PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;
#endif

	DBUSTRACE(("%s\n", __FUNCTION__));
	ASSERT(Context);

#ifdef WDM_API
	pTCB = (PTCB)Context;
	usbinfo = pTCB->usbinfo;
	status = Irp->IoStatus.Status;
#else
	UNREFERENCED_PARAMETER(Target);
	writeContext = (dbus_usbos_TCB_wdf *)Context;
	usbinfo = writeContext->usbinfo;
	status = CompletionParams->IoStatus.Status;
	/* For usb devices, we should look at the Usb.Completion param. */
	usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
#endif

	usbinfo->tx_cmplt_timeout = 0;

#ifdef WDM_API
	/*
	 * If Cancel has started on this irp, let the
	 * cancel routine cleanup. If not, continue processing
	 */
	if (InterlockedExchange((PVOID)&pTCB->IrpLock, IRPLOCK_COMPLETED)
		== IRPLOCK_CANCEL_STARTED) {
		return STATUS_MORE_PROCESSING_REQUIRED;
	}
#endif

	if (NT_SUCCESS(status)) {
#ifdef WDM_API
		if (Irp->IoStatus.Information > 0) {
			DBUSERR(("%s: Wrote %d bytes\n", __FUNCTION__, Irp->IoStatus.Information));
		} else  {
			/* always zero. why? */
			DBUSTRACE(("%s: ERROR, Failed to Write %d bytes\n",
				__FUNCTION__, Irp->IoStatus.Information));
		}
		usbinfo->tx_totbytessend += (ULONG)Irp->IoStatus.Information;
#else
		usbinfo->tx_totbytessend +=
			writeContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
		DBUSTRACE(("%s: Wrote %d bytes, Total bytes = %llu\n", __FUNCTION__,
			writeContext->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
			usbinfo->tx_totbytessend));
#endif

		dbus_status = DBUS_OK;
	} else if (status == STATUS_CANCELLED) {
		dbus_status = DBUS_STATUS_CANCELLED;
		DBUSTRACE(("%s: Write request CANCELLED 0x%x\n", __FUNCTION__, status));
	} else {
		/* Clear the flag to prevent any more writes from being posted to the target. */
		MP_CLEAR_FLAG(usbinfo, fMP_POST_WRITES);
		dbus_status = (DEVICEREMOVED(status)) ? DBUS_ERR_NODEVICE : DBUS_ERR_TXFAIL;
#ifdef WDF_API
		DBUSERR(("%s: Write failed: request 0x%x Status 0x%x UsbdStatus 0x%x \n",
			__FUNCTION__, Request, status, usbCompletionParams->UsbdStatus));
#else
		DBUSERR(("%s: Write request failed NTstatus 0x%x USBDstatus 0x%x\n",
			__FUNCTION__, status, pTCB->Urb->UrbHeader.Status));
#endif
	}

	if (usbinfo->cbarg && usbinfo->cbs) {
		dbus_irb_tx_t *txirb = NULL;

#ifdef WDM_API
		txirb = pTCB->txirb;
#else
		txirb = writeContext->txirb;
#endif

		if (usbinfo->cbs->send_irb_complete && (txirb != NULL))
			usbinfo->cbs->send_irb_complete(usbinfo->cbarg, txirb, dbus_status);
		else {
			ASSERT(usbinfo->cbs->send_irb_complete);
			/* ZLP packet was sent and handled for NDIS only;
			 * do not call send_irb_complete()
			 */
		}
	}

	/* DONE, free the request */
#ifdef WDF_API
	dbus_usbos_tx_req_return_wdf(usbinfo, Request);
#else
	if (NdisInterlockedDecrement(&pTCB->Ref) == 0) {
		dbus_usbos_tx_return_tcb(usbinfo, pTCB);
	}
#endif

	return STATUS_MORE_PROCESSING_REQUIRED;
}

#ifdef WDM_API
/* posts a write IRP to the target device to send the network packet out to the device */
static NTSTATUS
dbus_usbos_urb_send_buf_async_wdm(usbos_info_t* usbinfo, PTCB pTCB)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIRP irp = pTCB->Irp;
	PIO_STACK_LOCATION  nextStack;
	PDEVICE_OBJECT TargetDO = usbinfo->nextDeviceObj;

	/* NDIS60 should use Mdl, but no performance boost is observed,
	 * and it makes byte_copy skip trickier in dbus_usbos_tx_send_irb_async()
	 * so stick to NDI50 style pData
	UsbBuildInterruptOrBulkTransferRequest(pTCB->Urb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		usbinfo->TxPipeHandle,
		NULL,
		pTCB->DataMdl,
		pTCB->ulSize,
		0, NULL);
	*/

	UsbBuildInterruptOrBulkTransferRequest(pTCB->Urb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		usbinfo->TxPipeHandleWdm,
		pTCB->pData,
		NULL,
		pTCB->ulSize,
		0, NULL);

	/*
	 * Obtain a pointer to the stack location of the first driver that will be
	 * invoked.  This is where the function codes and the parameters are set.
	 */
	nextStack = IoGetNextIrpStackLocation(irp);
	nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	nextStack->Parameters.Others.Argument1 = (PVOID) pTCB->Urb;
	nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
	pTCB->IrpLock = IRPLOCK_CANCELABLE;

	IoSetCompletionRoutine(irp, dbus_usbos_tx_complete_cb, pTCB, TRUE, TRUE, TRUE);
	status = IoCallDriver(TargetDO, irp);

	DBUSTRACE(("%s: TransferBufferLength = %d\n", __FUNCTION__, pTCB->ulSize));
	if (!NT_SUCCESS(status)) {
		/*  Note: nothing needs to be done in caller for IoCallDriver failure at this stage
		 *  because dbus_usbos_tx_complete_cb will be invoked to clean thing up
		 */
		DBUSTRACE(("%s: IoCallDriver failed\n", __FUNCTION__));
	}
	return status;
}
#endif	/* WDM_API */

static bool
dbus_usbos_tx_alloc(usbos_info_t* usbinfo, uint max_ntxq)
{
	osl_t *osh = usbinfo->pub->osh;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	NTSTATUS ntStatus;
	uint i;
	PNDIS_BUFFER Buffer = NULL;
	PUCHAR pTCBMem = NULL, pBuf = NULL;
#if (0>= 0x0600) && !defined(WDF_API)
	ULONG partialMdlMemorySize = 0;
	PVOID partialMdlMemory = NULL;
#endif
	bool ret = FALSE;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (MP_TEST_FLAG(usbinfo, fMP_TX_RESOURCE_ALLOCATED))
		return TRUE;

	/* WDM and WDF: allocate tx data buffer to copy high level buf or chained pkt */
	Status = NdisAllocateMemoryWithTag(&pBuf, DBUS_BUFFER_SIZE_TX * max_ntxq, NDIS_TAG_USBDRV);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: Failed to allocate memory for tx buffer\n", __FUNCTION__));
		goto error;
	}
	NdisZeroMemory(pBuf, DBUS_BUFFER_SIZE_TX * max_ntxq);
	usbinfo->p_txbuf_memory = pBuf;

	/*
	 * Following are the lists to hold packets at different
	 * stages of processing.
	 * tx_free_list - Packets available for send operation
	 * tx_busy_list - Packets sent to the lower WDM stack
	 * tx_spinlock is used to synchronize access to these lists.
	 */
	NdisInitializeListHead(&usbinfo->tx_free_list);
	NdisInitializeListHead(&usbinfo->tx_busy_list);
	NdisAllocateSpinLock(&usbinfo->tx_spinlock);

#ifdef WDF_API
	/* WDF doesn't use TCB structure */
	ret = dbus_usbos_tx_req_alloc_wdf(usbinfo, max_ntxq);
	/* done */
#else
	/* Allocate a huge block of memory for all TCB's */
	Status = NdisAllocateMemoryWithTag(&pTCBMem, sizeof(TCB) * max_ntxq, NDIS_TAG_USBDRV);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: Failed to allocate memory for TCB's\n", __FUNCTION__));
		goto error;
	}
	NdisZeroMemory(pTCBMem, sizeof(TCB) * max_ntxq);
	usbinfo->p_TCB_memory = pTCBMem;

#if (0>= 0x0600)
	/*
	 * Find out the maximum number of pages an ethernet packet in a single MDL can span
	 * so that we can preallocate partial MDLs for each transfer.
	 * We will use this partial MDL to avoid buffer copy.
	 * Note: We add PAGE_SIZE below because we don't know the actual Virtual address.
	 */

	/* The size of the buffer could be 'n' pages but it can span 'n+1' pages because
	 * the Virtual Address can have an offset into the first Page.
	 */
	partialMdlMemorySize = ROUND_TO_PAGES(ETH_MAX_PACKET_SIZE) + PAGE_SIZE;

	/* Allocate a dummy memory to create partial MDLs */
	Status = NdisAllocateMemoryWithTag(&partialMdlMemory, partialMdlMemorySize,
		NDIS_TAG_USBDRV);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: Failed to allocate memory for partial Mdl\n", __FUNCTION__));
		goto error;
	}
#else /* (NDISVER < 0x0600) */

	/* Allocate a buffer pool for send buffers */
	NdisAllocateBufferPool(&Status,	&usbinfo->tx_bufferpool_ndis5, max_ntxq);
	if (Status != NDIS_STATUS_SUCCESS) {
		DBUSERR(("%s: NdisAllocateBufferPool for send buffer failed\n", __FUNCTION__));
		goto error;
	}
#endif 

	/*
	 * Divide the p_TCB_memory blob into TCBs and create a buffer descriptor for the
	 *   continuous Data portion of the TCBs. The reasons for doing this instead of
	 *   using the OriginalSend Packet buffers are when
	 * 1) high driver is not capable of handling chained buffers (MDLs).
	 * 2) high driver uses chained pkt
	 */
	for (i = 0; i < max_ntxq; i++) {
		PTCB pTCB = (PTCB)pTCBMem;
		pTCB->txDataBuf = pBuf;

		pTCB->usbinfo = usbinfo;

		/* Create a buffer descriptor(MDLs) for the Data portion of the TCBs */
#if (0>= 0x0600)
		/*
		 * If the target driver doesn't support chained MDLs then we will copy the data
		 * from the send NET_BUFFERS to TCB->Data buffer and allocate a single MDL to
		 * describe it. Note that NdisAllocateMdl along with allocating memory for MDL,
		 * it builds the MDL to describe the buffer.
		 */
		pTCB->DataMdl = NdisAllocateMdl(usbinfo->AdapterHandle,	(PVOID)pTCB->txDataBuf,
			DBUS_BUFFER_SIZE_TX);
		if (pTCB->DataMdl == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			DBUSERR(("%s: NdisAllocateMdl failed\n", __FUNCTION__));
			goto error;
		}
		pTCB->PartialMdl = NdisAllocateMdl(usbinfo->AdapterHandle, partialMdlMemory,
			partialMdlMemorySize);
		if (pTCB->PartialMdl == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			DBUSERR(("%s: NdisAllocateMdl failed for partial mdl\n", __FUNCTION__));
			goto error;
		}
		MmPrepareMdlForReuse(pTCB->PartialMdl);
#else /* (NDISVER < 0x0600) */

		NdisAllocateBuffer(&Status, &Buffer, usbinfo->tx_bufferpool_ndis5,
			(PVOID)pTCB->txDataBuf, DBUS_BUFFER_SIZE_TX);
		if (Status != NDIS_STATUS_SUCCESS) {
			DBUSERR(("%s: NdisAllocateBuffer failed\n", __FUNCTION__));
			goto error;
		}
		pTCB->Buffer_ndis5 = Buffer;
#endif 


		/* This data point is still used by both ndis5 and ndis6 for now
		 *   In urb_send_buf_async(), it gets BYTE copy from upper layer buffer
		 */
		pTCB->pData = pTCB->txDataBuf;

		/*
		 * Preallocate all the IRPs required.  Do not charge quota
		 * to the current process for this IRP. If the miniport is
		 * loaded as a function driver of the WDM stack, say in case
		 * of USB, the TargetDO will be the NextDeviceObject.
		 */
		pTCB->Irp = IoAllocateIrp(usbinfo->nextDeviceObj->StackSize, FALSE);
		if (pTCB->Irp == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			goto error;
		}
		pTCB->Urb = MALLOC(osh,	sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
		if (pTCB->Urb == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			goto error;
		}
		pTCB->IrpLock = IRPLOCK_COMPLETED;
		NdisInterlockedInsertTailList(&usbinfo->tx_free_list, &pTCB->List,
			&usbinfo->tx_spinlock);

		pTCBMem = pTCBMem + sizeof(TCB);
		pBuf = pBuf + DBUS_BUFFER_SIZE_TX;
	}
	ret = (Status == NDIS_STATUS_SUCCESS);

#endif	/* WDF_API */

error:

#if (0>= 0x0600) && !defined(WDF_API)
	/* We created an MDL of right size. We don't need the memory anymore. */
	if (partialMdlMemory) {
		NdisFreeMemory(partialMdlMemory, partialMdlMemorySize, 0);
	}
#endif
	if (ret)
		MP_SET_FLAG(usbinfo, fMP_TX_RESOURCE_ALLOCATED);
	else
		DBUSERR(("%s: failed\n", __FUNCTION__));

	DBUSTRACE(("%s: done, ret = %d\n", __FUNCTION__, ret));
	return ret;
}

static void
dbus_usbos_tx_free(usbos_info_t* usbinfo)
{
	osl_t *osh = usbinfo->pub->osh;
	NDIS_STATUS Status;
	PTCB pTCB;

	if (!MP_TEST_FLAG(usbinfo, fMP_TX_RESOURCE_ALLOCATED))
		return;

	DBUSTRACE(("%s\n", __FUNCTION__));

	MP_CLEAR_FLAG(usbinfo, fMP_TX_RESOURCE_ALLOCATED);

#ifdef WDF_API
	dbus_usbos_tx_req_dealloc_wdf(usbinfo);
#else
	while (!IsListEmpty(&usbinfo->tx_free_list)) {
		pTCB = (PTCB)NdisInterlockedRemoveHeadList(&usbinfo->tx_free_list,
			&usbinfo->tx_spinlock);
		if (!pTCB)
			break;

#if (0>= 0x0600)
		if (pTCB->DataMdl)	NdisFreeMdl(pTCB->DataMdl);
		if (pTCB->PartialMdl)	NdisFreeMdl(pTCB->PartialMdl);
		if (pTCB->Request)	WdfObjectDelete(pTCB->Request);
#else
		if (pTCB->Buffer_ndis5)	NdisFreeBuffer(pTCB->Buffer_ndis5);
#endif
		if (pTCB->Irp)		IoFreeIrp(pTCB->Irp);
		if (pTCB->Urb)
			MFREE(osh, pTCB->Urb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
	}

#if (0< 0x0600)
	if (usbinfo->tx_bufferpool_ndis5) {
		NdisFreeBufferPool(usbinfo->tx_bufferpool_ndis5);
		usbinfo->tx_bufferpool_ndis5 = NULL;
	}
#endif

#endif	/* WDF_API */

	ASSERT(IsListEmpty(&usbinfo->tx_free_list));

	NdisFreeSpinLock(&usbinfo->tx_spinlock);

	/* free the buffer memory at the last when all associated structuress are freed */
	if (usbinfo->p_txbuf_memory) {
		NdisFreeMemory(usbinfo->p_txbuf_memory,
			DBUS_BUFFER_SIZE_TX * usbinfo->pub->ntxq, 0);
		usbinfo->p_txbuf_memory = NULL;
	}

	if (usbinfo->p_TCB_memory) {
		NdisFreeMemory(usbinfo->p_TCB_memory, sizeof(TCB) * usbinfo->pub->ntxq, 0);
		usbinfo->p_TCB_memory = NULL;
	}
}

static int
dbus_usbos_state_change(void *bus, int state)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (usbos_info->cbarg && usbos_info->cbs) {
		if (usbos_info->cbs->state_change)
			usbos_info->cbs->state_change(usbos_info->cbarg, state);
	}

	return DBUS_OK;
}

#if NOT_USED_ANYMORE

static int dbus_usbos_state_change(void *bus, int state);
static int dbus_usbos_errhandler(void *bus, int err);

static int
dbus_usbos_errhandler(void *bus, int err)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (usbos_info->cbarg && usbos_info->cbs) {
		if (usbos_info->cbs->errhandler)
			usbos_info->cbs->errhandler(usbos_info->cbarg, err);
	}

	return DBUS_OK;
}

NTSTATUS
dbus_usbos_wait_interrupt(usbos_info_t* usbinfo)
{
	NTSTATUS ntStatus;

	usbinfo->InterruptPayload[0] = 0x5a5a5a5a;
	UsbBuildInterruptOrBulkTransferRequest(usbinfo->intUrb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		usbinfo->IntrPipeHandleWdm,
		usbinfo->InterruptPayload,
		NULL,
		8,
		0,
		NULL);
	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, usbinfo->intUrb, TRUE);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Bad Interrupt 0x%x\n", __FUNCTION__, ntStatus));
	}

	return (ntStatus);
}
#endif /* NOT_YET */

static NTSTATUS
dbus_usbos_ctl_post_vendorreq(usbos_info_t *usbinfo, BOOLEAN Input, void *buffer, int len,
	bool async)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	ULONG TransferFlags = 0;
	USHORT Function  = URB_FUNCTION_CLASS_INTERFACE;
	UCHAR Request = 0;
	PURB urb = usbinfo->ctlUrb;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (Input) {
		TransferFlags |= USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
		/* Send a special request */
		if (usbinfo->rxctl_deferrespok) {
			Function = URB_FUNCTION_VENDOR_INTERFACE;
			Request = DL_DEFER_RESP_OK;
		}
	}

#ifdef WDF_API
	if (async)
		ntStatus = dbus_usbos_ctl_req_submit_async_wdf(usbinfo, FALSE, (BYTE)Request,
			0, len, buffer, 0);
	else {
		DBUSERR(("%s: sync is not supported for WDF yet\n", __FUNCTION__));
		ASSERT(0);
	}
#else

	UsbBuildVendorRequest(urb, Function, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
		TransferFlags,
		0,
		Request,
		0,
		usbinfo->UsbInterface->InterfaceNumber,
		buffer,
		NULL,
		len,
		NULL);

	if (async)
		ntStatus = dbus_usbos_urb_submit_async(usbinfo, urb);
	else
		ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, TRUE);

#endif	/* WDF_API */
	if (!NT_SUCCESS(ntStatus)) {
		if (ntStatus == STATUS_DEVICE_NOT_CONNECTED)
			DBUSERR(("%s: Irp Error Device Not Connected\n", __FUNCTION__));
		else
			DBUSERR(("%s: Irp Error ntStatus 0x%x\n", __FUNCTION__, ntStatus));
		return ntStatus;
	}
	if (urb && !NT_SUCCESS(urb->UrbHeader.Status)) {
		DBUSERR(("%s: Bad urb->UrbHeader.Status 0x%x\n",
			__FUNCTION__, urb->UrbHeader.Status));
		return urb->UrbHeader.Status;
	}

	return (ntStatus);
}

/* not used for RPC */
static int
dbus_usbos_ctl_tx_send_ioctl(usbos_info_t *usbinfo, CHAR *Buffer, ULONG InLength, ULONG OutLength)
{
	int ntStatus = 0;
	LARGE_INTEGER	Interval;
	ULONG	i;

	DBUSTRACE(("%s\n", __FUNCTION__));

	for (i = 0; i < MAX_CMD_RETRY; i++) {
		ntStatus = dbus_usbos_ctl_post_vendorreq(usbinfo, FALSE, Buffer, InLength,
			FALSE);
		if (NT_SUCCESS(ntStatus)) {
			break;
		} else {
			if (ntStatus == STATUS_DEVICE_NOT_CONNECTED) {
				DBUSERR(("%s: Device Not Connected\n", __FUNCTION__));
				break;
			} else if (ntStatus == STATUS_NO_SUCH_DEVICE) {
				DBUSERR(("%s: No such device\n", __FUNCTION__));
				dbusos_stop(usbinfo);
				break;
			}
		}
	}

	return ntStatus;
}

/*
 *  This routine bumps up the I/O count.
 *  This routine is typically invoked when any of the
 *  dispatch routines handle new irps for the driver.
 */
static LONG
dbus_usbos_urb_ioinc_bulk(usbos_info_t* usbinfo)
{
	LONG  result = 0;
	KIRQL oldIrql;

	DBUSDBGLOCK(("%s\n", __FUNCTION__));

	KeAcquireSpinLock(&usbinfo->IOCountSpinLock_Bulk, &oldIrql);
	result = InterlockedIncrement(&usbinfo->IOoutstanding_Bulk);

	/* when IOoutstanding_Bulk bumps from 1 to 2, clear the StopEvent */
	if (result == 2)
		KeClearEvent(&usbinfo->StopEvent);
	KeReleaseSpinLock(&usbinfo->IOCountSpinLock_Bulk, oldIrql);
	return result;
}

/*
 *  This routine decrements the outstanding I/O count
 *  This is typically invoked after the dispatch routine
 *  has finished processing the irp.
 */
static LONG
dbus_usbos_urb_iodec_bulk(usbos_info_t* usbinfo)
{
	LONG  result = 0;
	KIRQL oldIrql;

	DBUSDBGLOCK(("%s\n", __FUNCTION__));

	KeAcquireSpinLock(&usbinfo->IOCountSpinLock_Bulk, &oldIrql);
	result = InterlockedDecrement(&usbinfo->IOoutstanding_Bulk);

	if (result == 1)
		KeSetEvent(&usbinfo->StopEvent, IO_NO_INCREMENT, FALSE);
	if (result == 0)
		KeSetEvent(&usbinfo->RemoveEvent, IO_NO_INCREMENT, FALSE);
	KeReleaseSpinLock(&usbinfo->IOCountSpinLock_Bulk, oldIrql);
	return result;
}

static VOID
dbus_usbos_init_parse_pipes_wdm(usbos_info_t *usbinfo)
{
	NTSTATUS ntStatus;
	PUSBD_INTERFACE_INFORMATION interfaceInfo;
	USBD_PIPE_HANDLE pipeHandle = NULL;
	ULONG i, numberConfiguredPipes;
	USBD_PIPE_TYPE PipeType;

	DBUSTRACE(("%s\n", __FUNCTION__));

	interfaceInfo = usbinfo->UsbInterface;
	numberConfiguredPipes = interfaceInfo->NumberOfPipes;
	if (numberConfiguredPipes > DBUS_USBOS_MAX_PIPES) {
		numberConfiguredPipes = DBUS_USBOS_MAX_PIPES;
	}

	for (i = 0; i < numberConfiguredPipes; i++) {

		PipeType = interfaceInfo->Pipes[i].PipeType;

		switch (PipeType) {
			case UsbdPipeTypeBulk:
				if (interfaceInfo->Pipes[i].EndpointAddress & 0x80) {
					if (usbinfo->RxPipeHandleWdm == NULL) {
						usbinfo->RxPipeHandleWdm =
							interfaceInfo->Pipes[i].PipeHandle;
					}
				} else {
					usbinfo->TxPipeHandleWdm =
						interfaceInfo->Pipes[i].PipeHandle;
					usbinfo->bulkpipe_mps =
						interfaceInfo->Pipes[i].MaximumPacketSize;
				}
			break;
			case UsbdPipeTypeInterrupt:
				usbinfo->IntrPipeHandleWdm =
					interfaceInfo->Pipes[i].PipeHandle;
			break;
			default:
			break;
		}
	}

	return;
}

/* This helper routine reads the configuration descriptor for the device in couple of steps. */
static bool
dbus_usbos_init_config_device_wdm(usbos_info_t* usbinfo)
{
	osl_t *osh = usbinfo->pub->osh;
	struct _URB_CONTROL_DESCRIPTOR_REQUEST ctl_desc_req;
	PURB urb;
	ULONG siz;
	NTSTATUS ntStatus;
	USB_CONFIGURATION_DESCRIPTOR conf_desc;
	PUSB_CONFIGURATION_DESCRIPTOR cfgDescriptor;
	bool ret = TRUE;
	urb = NULL;
	cfgDescriptor = NULL;

	DBUSTRACE(("%s\n", __FUNCTION__));

	ASSERT(usbinfo->UsbConfigurationDescriptor == NULL);

	/*
	 * Read the first configuration descriptor
	 * This requires two steps:
	 * 1. Read the fixed sized configuration desciptor (CD)
	 * 2. Read the CD with all embedded interface and endpoint descriptors
	 */

	urb = (PURB) &ctl_desc_req;

	UsbBuildGetDescriptorRequest(urb, (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
		USB_CONFIGURATION_DESCRIPTOR_TYPE,
		0,
		0,
		&conf_desc,
		NULL,
		sizeof(USB_CONFIGURATION_DESCRIPTOR),
		NULL);
	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, TRUE);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: dbus_usbos_urb_submit_sync failed 0x%x\n", __FUNCTION__, ntStatus));
		goto dbus_usbos_config_device_Exit;
	}
	siz = conf_desc.wTotalLength;

	/* FIX: Change to local, dynamic TotalLen???? */
	cfgDescriptor = MALLOC(osh, siz);
	if (!cfgDescriptor) {
		DBUSERR(("%s: Failed to alloc mem for config Descriptor\n", __FUNCTION__));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		goto dbus_usbos_config_device_Exit;
	}
	UsbBuildGetDescriptorRequest(urb, (USHORT)sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
		USB_CONFIGURATION_DESCRIPTOR_TYPE,
		0,
		0,
		cfgDescriptor,
		NULL,
		siz,
		NULL);
	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, TRUE);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Failed to read configuration descriptor 0x%x\n",
			__FUNCTION__, ntStatus));
		goto dbus_usbos_config_device_Exit;
	}
	if (cfgDescriptor) {
		/* save a copy of cfgDescriptor in usbinfo remember to free it later. */
		usbinfo->UsbConfigurationDescriptor = cfgDescriptor;
		if (cfgDescriptor->bmAttributes & REMOTE_WAKEUP_MASK) {
			/* this configuration supports remote wakeup */
			usbinfo->remote_wake_capable = TRUE;
		} else {
			usbinfo->remote_wake_capable = FALSE;
		}
		ret = dbus_usbos_init_select_interfaces_wdm(usbinfo, cfgDescriptor);
	}

dbus_usbos_config_device_Exit:
	if (cfgDescriptor) {
		MFREE(osh, cfgDescriptor, siz);
	}

	ret = NT_SUCCESS(ntStatus);

	return ret;
}

static bool
dbus_usbos_init_select_interfaces_wdm(usbos_info_t *usbinfo, PUSB_CONFIGURATION_DESCRIPTOR CD)
{
	osl_t *osh = usbinfo->pub->osh;
	LONG numberOfInterfaces, interfaceNumber, interfaceindex;
	ULONG i, intfSize, numberConfiguredPipes;
	PURB urb;
	PUCHAR pInf;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PUSB_INTERFACE_DESCRIPTOR   interfaceDescriptor;
	PUSBD_INTERFACE_LIST_ENTRY  interfaceList, tmp;
	PUSBD_INTERFACE_INFORMATION Interface;

	DBUSTRACE(("%s\n", __FUNCTION__));

	urb = NULL;
	Interface = NULL;
	interfaceDescriptor = NULL;
	numberOfInterfaces = CD->bNumInterfaces;
	interfaceindex = interfaceNumber = 0;

	/* Parse the configuration descriptor for the interface */
	intfSize = sizeof(USBD_INTERFACE_LIST_ENTRY) * (numberOfInterfaces + 1);
	tmp = interfaceList = MALLOC(osh, intfSize);
	if (!tmp) {
		DBUSERR(("%s: Failed to allocate mem for interfaceList\n", __FUNCTION__));
		return FALSE;
	}

	while (interfaceNumber < numberOfInterfaces) {

		interfaceDescriptor = USBD_ParseConfigurationDescriptorEx(CD, CD, interfaceNumber,
			0, -1, -1, -1);
		if (interfaceDescriptor) {
			interfaceList->InterfaceDescriptor = interfaceDescriptor;
			interfaceList->Interface = NULL;
			interfaceList++;
			interfaceNumber++;
		}
		interfaceindex++;
	}

	interfaceList->InterfaceDescriptor = NULL;
	interfaceList->Interface = NULL;
	urb = USBD_CreateConfigurationRequestEx(CD, tmp);
	if (!urb) {
		DBUSERR(("%s: USBD_CreateConfigurationRequestEx failed\n", __FUNCTION__));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}

	Interface = &urb->UrbSelectConfiguration.Interface;
	numberConfiguredPipes = Interface->NumberOfPipes;
	if (numberConfiguredPipes > DBUS_USBOS_MAX_PIPES) {
		numberConfiguredPipes = DBUS_USBOS_MAX_PIPES;
	}

	for (i = 0; i < numberConfiguredPipes; i++) {
		/*
		 * perform pipe initialization here
		 * set the transfer size and any pipe flags we use
		 * USBD sets the rest of the Interface struct members
		 */

		Interface->Pipes[i].MaximumTransferSize = 0xFFFFF;
	}
	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, TRUE);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Failed to select an interface 0x%x\n", __FUNCTION__, ntStatus));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}

	/* save a copy of interface information in the device extension */
	usbinfo->UsbInterfaceSize = Interface->Length;
	usbinfo->UsbInterface = MALLOC(osh, usbinfo->UsbInterfaceSize);
	if (usbinfo->UsbInterface) {
		RtlCopyMemory(usbinfo->UsbInterface, Interface, Interface->Length);
	} else {
		DBUSERR(("%s: memory alloc for UsbInterface failed\n", __FUNCTION__));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}

	Interface = &urb->UrbSelectConfiguration.Interface;

	DBUSERR(("%s: ---numberOfInterfaces %d-----\n", __FUNCTION__, numberOfInterfaces));
	DBUSERR(("%s: NumberOfPipes 0x%x Length 0x%x Alt Setting 0x%x Interface Number 0x%x\n"
		"Class/subclass/protocol 0x%x 0x%x 0x%x\n",
		__FUNCTION__, Interface->NumberOfPipes, Interface->Length,
		Interface->AlternateSetting, Interface->InterfaceNumber,
		Interface->Class, Interface->SubClass, Interface->Protocol));

	for (i = 0; i < numberConfiguredPipes; i++) {
		DBUSERR(("---------\n"));
		DBUSERR(("%s: PipeType 0x%x EndpointAddress 0x%x MaxPacketSize 0x%x Interval 0x%x\n"
			"Handle 0x%x MaximumTransferSize 0x%x\n",
			__FUNCTION__, Interface->Pipes[i].PipeType,
			Interface->Pipes[i].EndpointAddress,
			Interface->Pipes[i].MaximumPacketSize,
			Interface->Pipes[i].Interval,
			Interface->Pipes[i].PipeHandle,
			Interface->Pipes[i].MaximumTransferSize));
	}
	DBUSTRACE(("%s: ---------\n", __FUNCTION__));

exit:
	if (tmp) {
		MFREE(osh, tmp, intfSize);
	}

	if (urb) {	/* Need to use this call because of USBD_CreateConfigurationRequestEx() */
		ExFreePool(urb);
	}

	if (NT_SUCCESS(ntStatus))
		dbus_usbos_init_parse_pipes_wdm(usbinfo);

	return NT_SUCCESS(ntStatus);
}

/*
 * This routine configures the USB device. In this routines we get the device descriptor,
 * the configuration descriptor and select the configuration descriptor.
 */
static bool
dbus_usbos_init_readselect_desc_wdm(usbos_info_t* usbinfo)
{
	osl_t *osh = usbinfo->pub->osh;
	PURB urb;
	ULONG siz;
	NTSTATUS ntStatus;
	struct _URB_CONTROL_DESCRIPTOR_REQUEST ctl_desc_req;
	USB_DEVICE_DESCRIPTOR dev_desc;
	PUSB_DEVICE_DESCRIPTOR deviceDescriptor;
	USB_STRING_DESCRIPTOR str_desc;
	PUSB_STRING_DESCRIPTOR stringDescriptor;
	WCHAR buf[100];
	bool rt = TRUE;

	DBUSTRACE(("%s\n", __FUNCTION__));

	urb = NULL;
	deviceDescriptor = NULL;

	/* 1. Read the device descriptor */
	urb = (PURB) &ctl_desc_req;
	if (!urb) {
		DBUSERR(("%s: Failed to allocate memory for urb\n", __FUNCTION__));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		rt = FALSE;
		goto error;
	}

	siz = sizeof(USB_DEVICE_DESCRIPTOR);
	deviceDescriptor = &dev_desc;
	UsbBuildGetDescriptorRequest(urb, (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
		USB_DEVICE_DESCRIPTOR_TYPE,
		0,
		0,
		deviceDescriptor,
		NULL,
		siz,
		NULL);

	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, TRUE);
	if (NT_SUCCESS(ntStatus)) {
		ASSERT(deviceDescriptor->bNumConfigurations);
		DBUSERR(("%s: Descriptor Len = %d Vendor = 0x%x Product = 0x%x \n", __FUNCTION__,
			deviceDescriptor->bLength, deviceDescriptor->idVendor,
			deviceDescriptor->idProduct));
		usbinfo->pub->attrib.vid = deviceDescriptor->idVendor;
		usbinfo->pub->attrib.pid = deviceDescriptor->idProduct;
		rt = dbus_usbos_init_config_device_wdm(usbinfo);
		if (!rt)
			goto error;
	}

	/* for debugging only, get the length for the serieal no. (3) descriptor */
	siz = sizeof(USB_STRING_DESCRIPTOR);
	stringDescriptor = &str_desc;
	UsbBuildGetDescriptorRequest(urb, (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
		USB_STRING_DESCRIPTOR_TYPE,
		3,
		0,
		stringDescriptor,
		NULL,
		siz,
		NULL);
	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, TRUE);
	rt = NT_SUCCESS(ntStatus);

error:
	if (!rt)
		DBUSERR(("%s: *** Getting string descriptor FAILED *** \n", __FUNCTION__));

	return rt;
}

static NTSTATUS
dbus_usbos_irp_complete_sync_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
	usbos_info_t *usbos_info = (usbos_info_t *) Context;

	DBUSTRACE(("%s\n", __FUNCTION__));

	KeSetEvent(&usbos_info->eve, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}

static int
dbus_usbos_get_device_speed(usbos_info_t* usbinfo)
{
	PIRP irp;
	KEVENT *e = &usbinfo->eve;
	PIO_STACK_LOCATION nextStack;
	USB_BUS_INTERFACE_USBDI_V1 bus_intf1;
	NTSTATUS ntStatus;
	int status = DBUS_ERR;
	ULONG len = 0;

	DBUSTRACE(("%s\n", __FUNCTION__));

#if (0>= 0x0630) && defined WDF_API
	ntStatus = WdfUsbTargetDeviceQueryUsbCapability(usbinfo->UsbDevice,
		(GUID*)&GUID_USB_CAPABILITY_DEVICE_CONNECTION_SUPER_SPEED_COMPATIBLE,
		0, NULL, NULL);

	if (NT_SUCCESS(ntStatus))
		usbinfo->pub->device_speed = SUPER_SPEED;
	else {
		ntStatus = WdfUsbTargetDeviceQueryUsbCapability(usbinfo->UsbDevice,
		(GUID*)&GUID_USB_CAPABILITY_DEVICE_CONNECTION_HIGH_SPEED_COMPATIBLE,
		0, NULL, NULL);

		if (NT_SUCCESS(ntStatus))
			usbinfo->pub->device_speed = HIGH_SPEED;
		else
			usbinfo->pub->device_speed = FULL_SPEED;
	}
	return DBUS_OK;
#else
	usbinfo->pub->device_speed = INVALID_SPEED;
	memset(&bus_intf1, 0, sizeof(USB_BUS_INTERFACE_USBDI_V1));
	KeInitializeEvent(e, NotificationEvent, FALSE);

	irp = IoAllocateIrp(usbinfo->TopOfStackDO->StackSize, FALSE);
	if (irp == NULL) {
		DBUSERR(("%s: Failed to allocate irp!!\n", __FUNCTION__));
		return status;
	}

	/*
	 * QueryInterface for USB_BUS_INTERFACE_USBDI_V1
	 * Figure out if device is operating at high speed
	 */
	/* STATUS_NOT_SUPPORTED mandatory for pnp irps */
	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	nextStack = IoGetNextIrpStackLocation(irp);
	ASSERT(nextStack);
	nextStack->MajorFunction = IRP_MJ_PNP;
	nextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
	nextStack->Parameters.QueryInterface.Interface = (PINTERFACE) &bus_intf1;
	nextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;
	nextStack->Parameters.QueryInterface.InterfaceType = &USB_BUS_INTERFACE_USBDI_GUID;
	nextStack->Parameters.QueryInterface.Size = sizeof(USB_BUS_INTERFACE_USBDI_V1);
	nextStack->Parameters.QueryInterface.Version = USB_BUSIF_USBDI_VERSION_1;

	dbus_usbos_urb_ioinc_bulk(usbinfo);
	IoSetCompletionRoutine(irp, dbus_usbos_irp_complete_sync_cb, usbinfo, TRUE, TRUE, TRUE);
	ntStatus = IoCallDriver(usbinfo->TopOfStackDO, irp);
	if (ntStatus == STATUS_PENDING) {
		KeWaitForSingleObject(e, Executive, KernelMode, FALSE, NULL);
		ntStatus = irp->IoStatus.Status;
	}

	if (NT_SUCCESS(ntStatus)) {
		if (bus_intf1.IsDeviceHighSpeed != NULL) {
			if (bus_intf1.BusContext != NULL) {
				if (bus_intf1.IsDeviceHighSpeed(bus_intf1.BusContext))
					usbinfo->pub->device_speed = HIGH_SPEED;
				else
					usbinfo->pub->device_speed = FULL_SPEED;
				status = DBUS_OK;
			} else
				DBUSERR(("%s: bus_intf1.BusContext NULL!!\n", __FUNCTION__));
		} else
			DBUSERR(("%s: bus_intf1.IsDeviceHighSpeed NULL!!\n", __FUNCTION__));
	} else
		DBUSERR(("%s: IoCallDriver failed!!\n", __FUNCTION__));


	IoFreeIrp(irp);
	dbus_usbos_urb_iodec_bulk(usbinfo);
#endif 

	return status;
}
/*
 * Completion routine for the synchronous write request.
 * This routine indicates to NDIS that send operation is complete
 * and free the TCB if nobody has already done so.
 * This routine also handles the case where another thread has canceled the write request.
 *
 * Returns STATUS_MORE_PROCESSING_REQUIRED - this is an synchronous IRP and we want to reuse it.
 */
static NTSTATUS
dbus_usbos_urb_complete_sync_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
	usbos_info_t *usbos_info = (usbos_info_t *) Context;

	DBUSTRACE(("%s\n", __FUNCTION__));

	KeSetEvent(&usbos_info->eve, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}

static NTSTATUS
dbus_usbos_urb_complete_sync_cb2(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
	usbos_info_t *usbos_info = (usbos_info_t *) Context;

	DBUSTRACE(("%s\n", __FUNCTION__));

	KeSetEvent(&usbos_info->purge_event, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}

#ifdef WDF_API
static NTSTATUS
dbus_usbos_urb_complete_sync_cb_wdf(WDFREQUEST Request, WDFIOTARGET Target,
	PWDF_REQUEST_COMPLETION_PARAMS CompletionParams, WDFCONTEXT Context)
{
	usbos_info_t *usbos_info = (usbos_info_t *) Context;
	NTSTATUS ntStatus = WdfRequestGetStatus(Request);

	DBUSTRACE(("%s, status 0x%x\n", __FUNCTION__, ntStatus));

	/* If the sync call is cancelled due to timeout expiry it is completed
	 * with status STATUS_IO_TIMEOUT
	 */
	if (ntStatus == STATUS_IO_TIMEOUT)
		KeSetEvent(&usbos_info->eve, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}
#endif /* WDF_API */

static void
dbus_usbos_ctl_rxpending_cb(usbos_info_t *usbinfo, NTSTATUS ntstatus)
{
	if (usbinfo->pub->busstate == DBUS_STATE_DOWN) {
		DBUSTRACE(("%s: Error, bus is down!!!\n", __FUNCTION__));
		return;
	}

	if (ntstatus != STATUS_DEVICE_NOT_CONNECTED && usbinfo->cbarg &&
		usbinfo->cbs && usbinfo->cbs->ctl_complete) {
		usbinfo->rxctl_pending = FALSE;
		usbinfo->cbs->ctl_complete(usbinfo->cbarg, DBUS_CBCTL_READ, ntstatus);
	}
}

static NTSTATUS
dbus_usbos_urb_complete_async_cb(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
	usbos_info_t* usbinfo = (usbos_info_t*)Context;

	DBUSTRACE(("%s\n", __FUNCTION__));

	/* receive from control IN pipe. e.g. BMAC call_with_return */
	if (usbinfo->rxctl_pending) {
		dbus_usbos_ctl_rxpending_cb(usbinfo, Irp->IoStatus.Status);
	}

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

static NTSTATUS
dbus_usbos_urb_submit_async(usbos_info_t *usbinfo, PURB Urb)
{
	PIRP irp = NULL;
	NTSTATUS ntStatus;
	IO_STATUS_BLOCK ioStatus;
	PIO_STACK_LOCATION nextStack = NULL;

	DBUSTRACE(("%s\n", __FUNCTION__));

	irp = IoAllocateIrp(usbinfo->TopOfStackDO->StackSize, FALSE);
	if (irp == NULL) {
		ASSERT(0);
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	IoSetCancelRoutine(irp, NULL);
	nextStack = IoGetNextIrpStackLocation(irp);
	ASSERT(nextStack != NULL);
	nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
	nextStack->Parameters.Others.Argument1 = Urb;

	/* irp freed in dbus_usbos_urb_complete_async_cb */
	IoSetCompletionRoutine(irp, dbus_usbos_urb_complete_async_cb, (PVOID)usbinfo,
		TRUE, TRUE, TRUE);
	ntStatus = IoCallDriver(usbinfo->TopOfStackDO, irp);

	return ntStatus;
}

static NTSTATUS
dbus_usbos_urb_submit_sync2(usbos_info_t *usbinfo, PURB Urb)
{
	KEVENT *e = &usbinfo->purge_event;
	PIRP irp;
	PIO_STACK_LOCATION nextStack;
	NTSTATUS ntStatus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	KeInitializeEvent(e, SynchronizationEvent, FALSE);

	irp = IoAllocateIrp(usbinfo->TopOfStackDO->StackSize, FALSE);
	if (irp == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;
	nextStack = IoGetNextIrpStackLocation(irp);
	nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
	nextStack->Parameters.Others.Argument1 = Urb;

	IoSetCompletionRoutine(irp, dbus_usbos_urb_complete_sync_cb2, usbinfo, TRUE, TRUE, TRUE);
	ntStatus = IoCallDriver(usbinfo->TopOfStackDO, irp);

	if (ntStatus == STATUS_PENDING) {
		LARGE_INTEGER timeout;
		timeout.QuadPart = -10000 * 5000;
		ntStatus = KeWaitForSingleObject(e, Executive, KernelMode, FALSE, &timeout);
		if (ntStatus == STATUS_TIMEOUT) {
			ntStatus = STATUS_IO_TIMEOUT;
			IoCancelIrp(irp);
			KeWaitForSingleObject(e, Executive, KernelMode, FALSE, NULL);
		} else {
			ntStatus = irp->IoStatus.Status;
		}
	}
	IoFreeIrp(irp);
	return ntStatus;
}

/* This routine synchronously submits an urb down the stack. */
NTSTATUS
dbus_usbos_urb_submit_sync(usbos_info_t *usbinfo, PURB Urb, bool blocking)
{
	PIRP irp;
	KEVENT *e = &usbinfo->eve;
	NTSTATUS ntStatus;
	IO_STATUS_BLOCK ioStatus;
	PIO_STACK_LOCATION nextStack;
	LARGE_INTEGER timeout;

	DBUSTRACE(("%s block %d\n", __FUNCTION__, blocking));

	KeInitializeEvent(e, NotificationEvent, FALSE);

	irp = IoBuildDeviceIoControlRequest(IOCTL_INTERNAL_USB_SUBMIT_URB, usbinfo->TopOfStackDO,
		NULL, 0, NULL,	0, TRUE, e, &ioStatus);
	if (!irp) {
		DBUSERR(("%s: IoBuildDeviceIoControlRequest failed\n", __FUNCTION__));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	/* Invoke completion routine on error and cancel
	 * else we could have a case where a timeout occurs
	 * and we wait forever if the call is blocking
	 * e.g. surprise removal immediately after device arrival
	 */
	IoSetCompletionRoutine(irp, dbus_usbos_urb_complete_sync_cb, (PVOID)usbinfo,
		TRUE, TRUE, TRUE);
	nextStack = IoGetNextIrpStackLocation(irp);
	ASSERT(nextStack != NULL);
	nextStack->Parameters.Others.Argument1 = Urb;

	dbus_usbos_urb_ioinc_bulk(usbinfo);

	ioStatus.Status = STATUS_SUCCESS;

	ntStatus = IoCallDriver(usbinfo->TopOfStackDO, irp);

	if (ntStatus == STATUS_PENDING) {
		if (blocking) {
			ntStatus = KeWaitForSingleObject(e, Executive, KernelMode, FALSE, NULL);
		} else {
			timeout.QuadPart = -((LONGLONG) 2000 * 10 * 1000);
			ntStatus = KeWaitForSingleObject(e, Executive, KernelMode, FALSE,
				&timeout);
		}

		if (ntStatus == STATUS_TIMEOUT) {
			DBUSERR(("%s: wait timeout!!!\n", __FUNCTION__));
			IoCancelIrp(irp);
			if (blocking)
				KeWaitForSingleObject(e, Executive, KernelMode, FALSE, NULL);
			else
				KeWaitForSingleObject(e, Executive, KernelMode, FALSE, &timeout);

			ntStatus = STATUS_CANCELLED;
		} else {
			ntStatus = ioStatus.Status;
		}
	}

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	dbus_usbos_urb_iodec_bulk(usbinfo);

	DBUSDBGLOCK(("%s: done, ntStatus 0x%x\n", __FUNCTION__, ntStatus));

	return ntStatus;
}

/* copy and send one whole buffer, send ZLP is necessary
 * this function handles BOTH buf and pkt interface inside txirb structure
 * WARNING, the goto send_zlp could make the code path run twice
 */
static int
dbus_usbos_tx_send_irb_async(usbos_info_t *usbinfo, dbus_irb_tx_t* txirb)
{
	osl_t *osh = usbinfo->pub->osh;
	uchar *buf = txirb->buf;
	void *pkt = txirb->pkt;
	uint len = txirb->len;
	bool zlp = 0;
	UINT PacketCount;
	PTCB pTCB = NULL;
	UCHAR **ppData = NULL;
	int ret = DBUS_OK;
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef WDF_API
	WDFREQUEST      writeRequest;
	dbus_usbos_TCB_wdf	*writeContext;
#endif

	DBUSTRACE(("%s\n", __FUNCTION__));

	ASSERT(len <= DBUS_BUFFER_SIZE_TX);
	ASSERT(txirb);

send_zlp:
	/* (1) get request handler and its buffer address */
#ifdef WDF_API
	dbus_usbos_tx_req_get_wdf(usbinfo, &writeRequest);
	if (writeRequest == NULL) {
		DBUSERR(("%s: no write resource left!!\n", __FUNCTION__));
		ret = DBUS_ERR_TXDROP;
		goto err;
	}
	writeContext = _get_ctx_TCB(writeRequest);
	ppData = &(writeContext->pData);
	NdisAcquireSpinLock(&usbinfo->tx_spinlock);
	RemoveEntryList(&writeContext->List);
	NdisReleaseSpinLock(&usbinfo->tx_spinlock);

#else

	/* get TCB from free_list, move it to busy_list, then send */
	pTCB = (PTCB)NdisInterlockedRemoveHeadList(&usbinfo->tx_free_list, &usbinfo->tx_spinlock);
	if (pTCB == NULL) {
		DBUSTRACE(("%s: Error, No TCB; send_buf() failed\n", __FUNCTION__));
		ret = DBUS_ERR_TXDROP;
		goto err;
	}
	ppData = &(pTCB->pData);
#endif	/* WDF_API */

	/* (2) move buf or pkt(chained) input data to request handler */

	/* !!! BYTE_COPY, leave the full dongle case as it is */
	bcopy(pkt, *ppData, len);

	/* (3) send it */
#ifdef WDF_API
	writeContext->ulSize = len;
	writeContext->Ref = 1;
	writeContext->txirb = txirb;
	if (txirb)	/* save context for cancel from high layer */
		txirb->arg = writeContext;
	NdisInterlockedInsertTailList(&usbinfo->tx_busy_list, &writeContext->List,
		&usbinfo->tx_spinlock);

	DBUSTRACE(("%s: tx len = %d\n", __FUNCTION__, writeContext->ulSize));
	if (!dbus_usbos_tx_send_irb_async_wdf(usbinfo, writeRequest, writeContext)) {
		ret = DBUS_ERR_TXDROP;
		goto err;
	}

#else	/* WDM_API */

	pTCB->ulSize = len;
	pTCB->Ref = 1;
	pTCB->txirb = txirb;
	if (txirb)	/* save context for cancel from high layer */
		txirb->arg = pTCB;

	NdisInterlockedIncrement(&usbinfo->tx_pkt_busysend);
	ASSERT(usbinfo->tx_pkt_busysend <= usbinfo->pub->ntxq);

	NdisInitializeListHead(&pTCB->List);
	NdisInterlockedInsertTailList(&usbinfo->tx_busy_list, &pTCB->List, &usbinfo->tx_spinlock);

	/* Post an asynchronous write request */
	ntStatus = dbus_usbos_urb_send_buf_async_wdm(usbinfo, pTCB);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: dbus_usbos_urb_send_buf_async failed 0x%x\n",
			__FUNCTION__, ntStatus));
		ret = (DEVICEREMOVED(ntStatus)) ? DBUS_ERR_NODEVICE : DBUS_ERR_TXFAIL;
	} else {
		usbinfo->txpkt_pending_cnt++;
		ret = DBUS_OK;
	}
#endif	/* WDF_API */

	/* (4) ZLP, If packet is multiple of MPS, need to send zero-length packet (ZLP) */

	if (usbinfo->bulkpipe_mps == 0) {	/* low resource mode could skip this init */
		ASSERT(0);
		usbinfo->bulkpipe_mps = 512;
	}

	if ((len > 0) && ((len % usbinfo->bulkpipe_mps) == 0)) {
		txirb = NULL;
		len = 0;
		zlp = 1;
		goto send_zlp;
	}
err:
	return ret;
}

static bool
dbus_usbos_txrx_resource_init(usbos_info_t *usbinfo)
{
	osl_t *osh = usbinfo->pub->osh;
	NDIS_STATUS Status;
	ULONG unUsed;
	ULONG nameLength;
	int ret = 0;

	DBUSTRACE(("%s\n", __FUNCTION__));

	/* hibernate -> resume: resources already allocated */
	if (MP_ISSET_FLAG(usbinfo, fMP_INIT_PNP))
		return TRUE;

	if (MP_ISSET_FLAG(usbinfo, fMP_INIT_DONE))
		return FALSE;

	MP_SET_FLAG(usbinfo, fMP_POST_WRITES);
	MP_SET_FLAG(usbinfo, fMP_POST_READS);

	if (!dbus_usbos_tx_alloc(usbinfo, usbinfo->pub->ntxq)) {
		DBUSERR(("%s: Failed to allocate send side resources\n", __FUNCTION__));
		ret = -1;
		goto error;
	}

	if (!dbus_usbos_rx_alloc(usbinfo, usbinfo->pub->nrxq)) {
		/* if rx failed, free tx just allocated */
		dbus_usbos_tx_free(usbinfo);
		DBUSERR(("%s: Failed to send side resources\n", __FUNCTION__));
		ret = -1;
		goto error;
	}

	MP_SET_FLAG(usbinfo, fMP_INIT_DONE);

error:
	return (ret == 0);
}

static void
dbus_usbos_remove(osl_t *osh, void *instance)
{
	usbos_info_t *usbinfo = instance;
	BOOLEAN bDone = TRUE;
	LONG nHaltCount = 0;
	int err;

	DBUSTRACE(("%s: Enter \n", __FUNCTION__));

	if (usbinfo == NULL)
		return;

	ASSERT(osh);

	dbus_usbos_deinit_abort_pipes(usbinfo);

	NdisMSleep(1000); /* 1ms */
	usbinfo->pnp_isdown = TRUE;

#if (0>= 0x0600)
	if (usbinfo->ctl_tx_WorkItem)	WdfWorkItemFlush(usbinfo->ctl_tx_WorkItem);
	if (usbinfo->ctl_rx_WorkItem)	WdfWorkItemFlush(usbinfo->ctl_rx_WorkItem);
#endif 

	if (usbinfo->UsbInterface) {
		MFREE(osh, usbinfo->UsbInterface, usbinfo->UsbInterfaceSize);
		usbinfo->UsbInterface = NULL;
	}
	if (usbinfo->intUrb) {
		MFREE(osh, usbinfo->intUrb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
		usbinfo->intUrb = NULL;
	}
	if (usbinfo->ctlUrb) {
		MFREE(osh, usbinfo->ctlUrb, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
		usbinfo->ctlUrb = NULL;
	}

#ifdef WDF_API
	if (usbinfo->rx_flowctl_WorkItem)
		WdfWorkItemFlush(usbinfo->rx_flowctl_WorkItem);

	/*
	 * WdfIoTargetStop will cancel all the outstanding I/O and wait
	 * for them to complete before returning.
	 * !!! WdfIoTargetStop with the WdfUsbTargetPipeConfigContinuousReader can be called at
	 * IRQL PASSIVE_LEVEL only.
	 */
	WdfWaitLockAcquire(usbinfo->PipeState_WaitLock, NULL);

	if (usbinfo->IoTarget)
		WdfIoTargetStop(usbinfo->IoTarget, WdfIoTargetCancelSentIo);

	WdfWaitLockRelease(usbinfo->PipeState_WaitLock);

	dbus_usbos_ctl_req_deallocate_wdf(usbinfo);

#else	/* WDF_API */

	dbus_usbos_tx_freebusypkt(usbinfo, TRUE);

	if (dbus_usbos_acquire_mutex(&usbinfo->purge_mutex) == DBUS_OK) {
		dbus_usbos_rx_freebusypkt(usbinfo, TRUE);
		dbus_usbos_release_mutex(&usbinfo->purge_mutex);
	}

	dbus_usbos_txrx_urb_waitcomplete(usbinfo);
#endif	/* WDF_API */

	dbus_usbos_rx_free(usbinfo);
	dbus_usbos_tx_free(usbinfo);

#ifdef WDF_API
	if (usbinfo->UsbDevice) {
		WdfObjectDelete(usbinfo->UsbDevice);
		usbinfo->UsbDevice = NULL;
		/*
		 * WDFREQUEST objects will be deleted by kmdf framework
		 * since they're childs of UsbDevice object
		 */
	}
#endif	/* WDF_API */

	MP_CLEAR_FLAG(usbinfo, fMP_INIT_DONE);
	DBUSTRACE(("%s: Exit \n", __FUNCTION__));
}

static void
dbus_usbos_rx_data_sendup_null(usbos_info_t *usbinfo, dbus_irb_rx_t *rxirb, NTSTATUS ntStatus)
{
	int dbus_status = DBUS_OK;

	if (usbinfo->cbarg && usbinfo->cbs && rxirb) {
		ASSERT(usbinfo->cbs->recv_irb_complete);
		if (usbinfo->cbs->recv_irb_complete) {
			rxirb->buf = NULL;
			rxirb->actual_len = 0;
			dbus_status = (DEVICEREMOVED(ntStatus)) ?
				DBUS_ERR_NODEVICE : DBUS_ERR_RXFAIL;
			usbinfo->cbs->recv_irb_complete(usbinfo->cbarg,	rxirb, dbus_status);
		}
	}
}

static int
dbus_usbos_rx_data_sendup(usbos_info_t *usbinfo, dbus_irb_rx_t *rxirb, uchar *msg, uint len)
{
	osl_t *osh = usbinfo->pub->osh;
	void *pkt;
	int dbus_status = DBUS_OK;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo->pub->busstate == DBUS_STATE_DOWN) {
		DBUSTRACE(("%s: Got data but bus down!!!\n", __FUNCTION__));
		dbus_status = DBUS_ERR_RXFAIL;
		goto err;
	}

	ASSERT(len <= DBUS_BUFFER_SIZE_RX);
	if (usbinfo->cbarg && usbinfo->cbs) {
		if (usbinfo->cbs->recv_irb_complete && rxirb) {
			rxirb->buf = msg;
			rxirb->actual_len = len;

			usbinfo->cbs->recv_irb_complete(usbinfo->cbarg, rxirb, dbus_status);
		} else {
			DBUSERR(("%s: recv_irb_complete failed=0x%p\n", __FUNCTION__, rxirb));
			dbus_status = DBUS_ERR;
		}
	}

err:
	return dbus_status;
}

#if (0< 0x0600)
static VOID
dbus_usbos_rx_reset(PNDIS_WORK_ITEM WorkItem, PVOID Context)
{
	usbos_info_t *usbinfo = (usbos_info_t*)Context;

	if (usbinfo == NULL)
		return;

	if (usbinfo->pub->busstate == DBUS_STATE_DOWN) {
		DBUSERR(("%s: DBUS is down, ignoring rx_reset workitem\n", __FUNCTION__));
		return;
	}

	DBUSERR(("%s: rx_workiterm start\n", __FUNCTION__));

	/* usb analyzer trigger, (2) activate this to be trigger in this bad case */
	/* dbus_usbos_ctl_send_checkcrc(usbinfo); */

	if (dbus_usbos_acquire_mutex(&usbinfo->purge_mutex) == DBUS_OK) {
		dbus_usbos_deinit_abortreset_pipe(usbinfo, usbinfo->RxPipeHandleWdm, FALSE);
		dbus_usbos_rx_freebusypkt(usbinfo, TRUE);
		dbus_usbos_release_mutex(&usbinfo->purge_mutex);
	}

	MP_SET_FLAG(usbinfo, fMP_POST_READS);
	if (usbinfo->cbarg && usbinfo->cbs && usbinfo->cbs->rxerr_indicate)
		usbinfo->cbs->rxerr_indicate(usbinfo->cbarg, FALSE);

	NdisAcquireSpinLock(&usbinfo->rx_spinlock);
	usbinfo->rx_reset_on = FALSE;
	NdisReleaseSpinLock(&usbinfo->rx_spinlock);

	DBUSERR(("%s: rx_workiterm done\n", __FUNCTION__));
}
#endif	

static VOID
dbus_usbos_txctl_workitem
#if (0< 0x0600)
	(PNDIS_WORK_ITEM WorkItem, PVOID Context)
#else
	(WDFWORKITEM  WorkItem)
#endif
{
	NDIS_STATUS Status;
	int bytes, dbus_status = DBUS_OK;
	usbos_info_t *usbinfo;
#if (0< 0x0600)
	usbinfo = (usbos_info_t*)Context;
#else
	uint32 result;
	usbinfo_workitem_ctx_ndis6 *usbinfo_ctx;

	usbinfo_ctx = _get_context_workitem(WorkItem);
	usbinfo = usbinfo_ctx->usbinfo;
#endif
	DBUSTRACE(("%s\n", __FUNCTION__));

	ASSERT(usbinfo);
	if (usbinfo->pub->busstate == DBUS_STATE_DOWN) {
		DBUSERR(("%s: ERROR, txctl workitem state is down\n", __FUNCTION__));
		goto fail;
	}

	Status = dbus_usbos_ctl_tx_send_ioctl(usbinfo,
		(char*)usbinfo->txctl_info.buf, usbinfo->txctl_info.len, 0);

	if (usbinfo->cbarg && usbinfo->cbs) {
		if (usbinfo->cbs->ctl_complete)
			usbinfo->cbs->ctl_complete(usbinfo->cbarg, DBUS_CBCTL_WRITE, dbus_status);
	}

fail:
#if (0>= 0x0600)
	result = InterlockedExchange((PLONG)&usbinfo_ctx->state, DBUS_WORKITEM_STATE_FREE);
	ASSERT(result == DBUS_WORKITEM_STATE_BUSY);
#endif
	usbinfo->txctl_pending = FALSE;
	NdisSetEvent(&usbinfo->txctl_PendingEvent);
}

static int
dbus_usbos_txctl(usbos_info_t *usbinfo, uchar *msg, uint msglen)
{
	int timeout = 0;
	uint condition = 0;
	bool pending;
	int err = DBUS_OK;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo->pub->busstate == DBUS_STATE_DOWN) {
		DBUSERR(("%s: bus is down!\n", __FUNCTION__));
		return DBUS_ERR;
	}

	usbinfo->txctl_info.buf = msg;
	usbinfo->txctl_info.len = msglen;

	usbinfo->txctl_pending = TRUE;
#if (0< 0x0600)
	NdisInitializeWorkItem(&usbinfo->ctl_tx_WorkItem, dbus_usbos_txctl_workitem, usbinfo);
	NdisScheduleWorkItem(&usbinfo->ctl_tx_WorkItem);
#else
	{
		usbinfo_workitem_ctx_ndis6 *usbinfo_ctx;
		usbinfo_ctx = _get_context_workitem(usbinfo->ctl_tx_WorkItem);
		if (InterlockedCompareExchange((PLONG)&usbinfo_ctx->state,
			DBUS_WORKITEM_STATE_BUSY,
			DBUS_WORKITEM_STATE_FREE) == DBUS_WORKITEM_STATE_FREE) {
			WdfWorkItemEnqueue(usbinfo->ctl_tx_WorkItem);
		} else {
			/* Should not happen */
			DBUSERR(("%s: ERROR, tx ctl workitem already enqueued\n", __FUNCTION__));
			err = DBUS_ERR;
		}
	}
#endif 
	return err;
}

static int
dbus_usbos_ctl_rx_async(usbos_info_t *usbinfo, uchar *msg, uint msglen)
{
	int timeout = 0;
	uint condition = 0;
	bool pending;
	int cnt;
	int err = DBUS_OK;
	int ntStatus = 0;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo->pub->busstate == DBUS_STATE_DOWN) {
		return DBUS_ERR;
	}

#if (0< 0x0600)
	if (msglen > PAGE_SIZE)
		msglen = PAGE_SIZE;
#endif

	usbinfo->rxctl_info.buf = msg;
	usbinfo->rxctl_info.len = msglen;

	ASSERT(!usbinfo->rxctl_pending);
	usbinfo->rxctl_pending = TRUE;

	RtlZeroMemory(usbinfo->rxctl_info.buf, usbinfo->rxctl_info.len);
	ntStatus = dbus_usbos_ctl_post_vendorreq(usbinfo, TRUE,
		(char*)usbinfo->rxctl_info.buf, usbinfo->rxctl_info.len, TRUE);
	if (!NT_SUCCESS(ntStatus)) {
		err = DBUS_ERR;
	}
	return err;
}

#ifdef WDF_API
static bool
dbus_usbos_init_device_wdf(usbos_info_t* usbinfo)
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	WDF_OBJECT_ATTRIBUTES attributes;
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
	WDFUSBPIPE			pipe;
	WDF_USB_PIPE_INFORMATION	pipeInfo;
	WDF_USB_DEVICE_INFORMATION	deviceInfo;
	WDF_WORKITEM_CONFIG workitemConfig;
	usbinfo_workitem_ctx_ndis6 *usbinfo_ctx;

	UCHAR i, numberConfiguredPipes;
#if (0>= 0x0630)
	WDF_USB_DEVICE_CREATE_CONFIG Config;
#endif 

	/* create USB target device */
	/* For Windows8 use USB 3.0 driver stack by specifying USBD_CLIENT_CONTRACT_VERSION_602 */
#if (0>= 0x0630)
	WDF_USB_DEVICE_CREATE_CONFIG_INIT(&Config, USBD_CLIENT_CONTRACT_VERSION_602);
	ntStatus = WdfUsbTargetDeviceCreateWithParameters(usbinfo->WdfDevice,
		&Config, WDF_NO_OBJECT_ATTRIBUTES,
		&usbinfo->UsbDevice);
#else
	ntStatus = WdfUsbTargetDeviceCreate(usbinfo->WdfDevice,	WDF_NO_OBJECT_ATTRIBUTES,
		&usbinfo->UsbDevice);
#endif 

	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: WdfUsbTargetDeviceCreateWithParameters failed 0x%x !STATUS!\n",
			__FUNCTION__, ntStatus));
		goto error;
	}

	/* retrieve USB device config information. Available APIs are
	 *  WdfUsbTargetDeviceRetrieveConfigDescriptor
	 *  WdfUsbTargetDeviceGetDeviceDescriptor
	 *  WdfUsbTargetDeviceGetInterface
	 * At the end, driver must select a configuration with below API
	 */
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_SINGLE_INTERFACE(&configParams);
	ntStatus = WdfUsbTargetDeviceSelectConfig(usbinfo->UsbDevice,
		WDF_NO_OBJECT_ATTRIBUTES, &configParams);
	if (!NT_SUCCESS(ntStatus)) {
		goto error;
	}
	usbinfo->WdfUsbInterface = configParams.Types.SingleInterface.ConfiguredUsbInterface;
	numberConfiguredPipes = configParams.Types.SingleInterface.NumberConfiguredPipes;
	if (numberConfiguredPipes > DBUS_USBOS_MAX_PIPES) {
		numberConfiguredPipes = DBUS_USBOS_MAX_PIPES;
	}

	/* Get pipe handles */
	for (i = 0; i < numberConfiguredPipes; i++) {

		WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);
		pipe = WdfUsbInterfaceGetConfiguredPipe(usbinfo->WdfUsbInterface, i, &pipeInfo);

		/* By default, the framework reports an error if a driver uses a read buffer
		 * that is not an integral multiple of the pipe's MPS. Disable this feature
		 * by telling the framework it's okay to read less than MaximumPacketSize
		 */
		WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(pipe);

		/* BULK endpoint, one IN, one OUT */
		if (WdfUsbPipeTypeBulk == pipeInfo.PipeType) {
			DBUSERR(("%s: BulkInput Pipe is 0x%p direction : %s\n", __FUNCTION__, pipe,
				WdfUsbTargetPipeIsInEndpoint(pipe) ? "IN" : "OUT"));

			if (WdfUsbTargetPipeIsInEndpoint(pipe)) {
				 /* sencond BULK IN endpoint for rpc_call_with_return
				  * not used yet
				  */
				if (usbinfo->RxPipeHandleWdf == NULL) {
					usbinfo->RxPipeHandleWdf = pipe;
				}
			} else {
				ASSERT(WdfUsbTargetPipeIsOutEndpoint(pipe));

				if (usbinfo->TxPipeHandleWdf == NULL) {
					usbinfo->TxPipeHandleWdf = pipe;
				} else {
					/* we would like, but don't have second out Bulkpipe yet */
					ASSERT(0);
					usbinfo->TxPipeHandle2Wdf = pipe;
				}
				usbinfo->bulkpipe_mps = pipeInfo.MaximumPacketSize;
			}

		} else if (WdfUsbPipeTypeInterrupt == pipeInfo.PipeType) {
			/* Intr endpoint, one IN, not used */
			if (WdfUsbTargetPipeIsInEndpoint(pipe))
				usbinfo->IntrPipeHandleWdf = pipe;

			DBUSERR(("%s: InterruptPipe is 0x%p type 0x%x\n", __FUNCTION__, pipe,
				WdfUsbTargetPipeIsInEndpoint(pipe) ? "IN" : "OUT"));
		} else {
			DBUSERR(("%s: Pipe is 0x%p type 0x%x\n",
				__FUNCTION__, pipe, pipeInfo.PipeType));
		}
	}

	WDF_USB_DEVICE_INFORMATION_INIT(&deviceInfo);
	ntStatus = WdfUsbTargetDeviceRetrieveInformation(usbinfo->UsbDevice, &deviceInfo);
	usbinfo->remote_wake_capable =
		(deviceInfo.Traits & WDF_USB_DEVICE_TRAIT_REMOTE_WAKE_CAPABLE) != 0;

	/* To retrieve a handle to a device's local I/O target, use WdfDeviceGetIoTarget */
	/* usbinfo->IoTarget = WdfDeviceGetIoTarget(usbinfo->WdfDevice);
	if (usbinfo->IoTarget == NULL) {
		DBUSERR(("%s: WdfDeviceGetIoTarget failed \n", __FUNCTION__));
		goto error;
	}
	*/
	usbinfo->IoTarget = WdfUsbTargetDeviceGetIoTarget(usbinfo->UsbDevice);
	if (usbinfo->IoTarget == NULL) {
		DBUSERR(("%s: WdfUsbTargetDeviceGetIoTarget failed \n", __FUNCTION__));
		goto error;
	}

	/* allocate ctl pipe resource, tx/rx resource are allocated later */
	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	ntStatus = WdfWaitLockCreate(&attributes, &usbinfo->PipeState_WaitLock);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Couldn't create PipeState_WaitLock 0x%x !STATUS!\n",
			__FUNCTION__, ntStatus));
		goto error;
	}

	ntStatus = WdfSpinLockCreate(&attributes, &usbinfo->WdfRecv_SpinLock);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Couldn't create WdfRecv_SpinLock 0x%x !STATUS!\n",
			__FUNCTION__, ntStatus));
		goto error;
	}

	WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, usbinfo_workitem_ctx_ndis6);
	attributes.ParentObject = usbinfo->WdfDevice;
	WDF_WORKITEM_CONFIG_INIT(&workitemConfig, dbus_usbos_rx_flowctl);
	ntStatus = WdfWorkItemCreate(&workitemConfig, &attributes, &usbinfo->rx_flowctl_WorkItem);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: WdfWorkItemCreate for rx_flowctl failed 0x%x\n",
			__FUNCTION__, ntStatus));
		goto error;
	}
	usbinfo_ctx = _get_context_workitem(usbinfo->rx_flowctl_WorkItem);
	usbinfo_ctx->usbinfo = usbinfo;
	usbinfo->rx_flowctl_on = FALSE;
	usbinfo->rx_flowctl_pending = 0;

	ntStatus = dbus_usbos_ctl_req_allocate_wdf(usbinfo, DBUS_USBOS_MAX_CONTROL_REQ);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: control_requests_allocate failed 0x%x !STATUS!\n",
			__FUNCTION__, ntStatus));
		goto error;
	}

	/* preallocate tx for dl_start */
	if (!dbus_usbos_tx_alloc(usbinfo, usbinfo->pub->ntxq))
		goto error;

	return TRUE;
error:
	return FALSE;
}
#endif	/* WDF_API */

static bool
dbus_usbos_init_device_wdm(usbos_info_t* usbinfo)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	{
		/*
		 * WinXP only
		 * check the registry flag-whether the device should selectively suspend when idle
		 *
		 * Clear the DO_DEVICE_INITIALIZING flag.
		 * Note: Do not clear this flag until the driver has set the
		 * device power state and the power DO flags.
		 */
		POWER_STATE state;
		state.DeviceState = PowerDeviceD0;
	}

	usbinfo->intUrb = MALLOC(usbinfo->pub->osh, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
	if (!usbinfo->intUrb) {
		goto error;
	}

	usbinfo->ctlUrb = MALLOC(usbinfo->pub->osh,
		sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
	if (!usbinfo->ctlUrb) {
		goto error;
	}

	if (!dbus_usbos_init_readselect_desc_wdm(usbinfo)) {
		DBUSERR(("%s: dbus_usbos_readselect_desc not successful!!!\n", __FUNCTION__));
		goto error;
	}

	return TRUE;

error:
	return FALSE;
}

static int
dbus_usbos_init_device(usbos_info_t* usbinfo)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	DBUSTRACE(("%s\n", __FUNCTION__));

#if (0>= 0x0600)
	usbinfo->WdfDevice = usbinfo->pub->sh->wdfdevice;

#ifndef WDF_API
	{
	WDF_OBJECT_ATTRIBUTES attributes;
	WDF_WORKITEM_CONFIG workitemConfig;
	usbinfo_workitem_ctx_ndis6 *usbinfo_ctx;

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, usbinfo_workitem_ctx_ndis6);
	attributes.ParentObject = usbinfo->WdfDevice;
	WDF_WORKITEM_CONFIG_INIT(&workitemConfig, dbus_usbos_txctl_workitem);
	ntStatus = WdfWorkItemCreate(&workitemConfig, &attributes, &usbinfo->ctl_tx_WorkItem);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: WdfWorkItemCreate failed 0x%x\n", __FUNCTION__, ntStatus));
		NT_STATUS_TO_NDIS_STATUS(ntStatus, &Status);
		goto error;
	}
	usbinfo_ctx = _get_context_workitem(usbinfo->ctl_tx_WorkItem);
	usbinfo_ctx->usbinfo = usbinfo;
	}
#endif	/* WDM_API */
#endif	

	/* WDM/WDF urb_submit_tx sync object */

	/* Initialize the remove event to not-signaled.
	 * Initialize the stop event to signaled. This event is signaled when the
	 * OutstandingIO becomes 1
	 */
	KeInitializeEvent(&usbinfo->RemoveEvent, SynchronizationEvent, FALSE);
	KeInitializeEvent(&usbinfo->StopEvent, SynchronizationEvent, TRUE);

	/*
	 * OutstandingIo count biased to 1.
	 * Transition to 0 during remove device means IO is finished.
	 * Transition to 1 means the device can be stopped
	 */
	usbinfo->IOoutstanding_Bulk = 1;
	KeInitializeSpinLock(&usbinfo->IOCountSpinLock_Bulk);

	/* ctl pipe resource */
	KeInitializeMutex(&usbinfo->ctl_mutex, 0);

	KeInitializeMutex(&usbinfo->purge_mutex, 0);

	usbinfo->txctl_pending = FALSE;
	usbinfo->rxctl_pending = FALSE;
	usbinfo->txpkt_pending_cnt = 0;
	usbinfo->pnp_isdown = FALSE;
	NdisInitializeEvent(&usbinfo->txctl_PendingEvent);
	NdisInitializeEvent(&usbinfo->rxctl_PendingEvent);

#ifdef WDF_API
	if (!dbus_usbos_init_device_wdf(usbinfo))
		goto error;
#else
	if (!dbus_usbos_init_device_wdm(usbinfo))
		goto error;
#endif

	if (dbus_usbos_get_device_speed(usbinfo) != DBUS_OK) {
		DBUSERR(("%s: dbus_usbos_get_device_speed failed!!!\n", __FUNCTION__));
		goto error;
	}
	if (usbinfo->pub->device_speed == HIGH_SPEED)
		DBUSTRACE(("%s: HIGH SPEED DEVICE\n", __FUNCTION__));
	else if (usbinfo->pub->device_speed == FULL_SPEED)
		DBUSTRACE(("%s: FULL SPEED DEVICE\n", __FUNCTION__));
	else if (usbinfo->pub->device_speed == INVALID_SPEED) {
		DBUSTRACE(("%s: DEVICE SPEED INVALID\n", __FUNCTION__));
		goto error;
	}

	return 0;
error:
	DBUSERR(("%s failed\n", __FUNCTION__));
	return -1;
}


/* ============ Module Interface APIs ================ */

static void
dbusos_stop(usbos_info_t *usbos_info)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	usbos_info->pub->busstate = DBUS_STATE_DOWN;
}

static int
dbus_usbos_probe()
{
	int ep;
	int ret = DBUS_OK;
	ULONG nameLength;
	NTSTATUS ntStatus;
	void *usbos_info;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (probe_cb) {
		disc_arg = probe_cb(probe_arg, "", USB_BUS, 0);
		if (disc_arg == NULL) {
			DBUSERR(("%s: probe_cb return NULL\n", __FUNCTION__));
			return DBUS_ERR;
		}
	}

	return ret;
}

static void
dbus_usbos_disconnect()
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	if (disconnect_cb) {
		disconnect_cb(disc_arg);
	}
}

static int
dbus_usbos_intf_send_irb(void *bus, struct dbus_irb_tx *txirb)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int ret;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (!MP_ISSET_FLAG(usbos_info, fMP_POST_WRITES)) {
		DBUSERR(("%s: fMP_POST_WRITES cleared, not sending urb!!\n", __FUNCTION__));
		return DBUS_ERR_TXDROP;
	}

	if (usbos_info->pub->busstate == DBUS_STATE_DOWN) {
		DBUSERR(("%s: bus down!!!\n", __FUNCTION__));
		return DBUS_ERR_TXDROP;
	} else {
		return dbus_usbos_tx_send_irb_async(usbos_info, txirb);
	}
}

static int
dbus_usbos_intf_recv_irb(void *bus, struct dbus_irb_rx *rxirb)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	PRCB pRCB = NULL;
	int ret = DBUS_OK;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (usbos_info->pub->busstate == DBUS_STATE_DOWN)
		return DBUS_ERR_RXDROP;

	if (!MP_ISSET_FLAG(usbos_info, fMP_POST_READS)) {
		DBUSERR(("%s: fMP_POST_READS cleared, not posting rcbs!!\n", __FUNCTION__));
		return DBUS_ERR_RXDROP;
	}

#ifdef WDF_API
	/* WDF doesn't register this since it will get rxirb from dbus_usbos_txrx_getirb() */
#else
	pRCB = (PRCB)NdisInterlockedRemoveHeadList(&usbos_info->rx_free_list,
		&usbos_info->rx_spinlock);
	if (pRCB == NULL)
		return DBUS_ERR;

	rxirb->buf = pRCB->rxDataBuf;
	rxirb->buf_len = DBUS_BUFFER_SIZE_RX;
	rxirb->arg = pRCB;
	pRCB->rxirb = rxirb;

	ret = dbus_usbos_rx_post_rcb(usbos_info, pRCB);
#endif	/* WDF_API */
	return  ret;
}

static int
dbus_usbos_intf_send_cancel_irb(void *bus, dbus_irb_tx_t *txirb)
{
	PTCB pTCB = NULL;
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

#ifdef WDF_API
	{
		WDFREQUEST writeRequest;
		if (txirb)
			writeRequest = (WDFREQUEST)txirb->arg;

		DBUSERR(("%s not yet implemented!!\n", __FUNCTION__));
	}
#else
	if (txirb)
		pTCB = (PTCB) txirb->arg;
	if (pTCB) {
		NdisInitializeListHead(&pTCB->List);

		if (InterlockedExchange((PVOID)&pTCB->IrpLock, IRPLOCK_CANCEL_STARTED)
			== IRPLOCK_CANCELABLE) {
			DBUSERR(("%s: calling IoCancelIrp\n", __FUNCTION__));
			if (IoCancelIrp(pTCB->Irp))
				return DBUS_OK;
		}
	}
#endif	/* WDF_API */
	return DBUS_ERR;
}

static int
dbus_usbos_intf_send_ctl(void *bus, void *pkt, int len)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int ret = DBUS_OK;
	bool wait;
	int cnt = 0;
	int err;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (dbus_usbos_acquire_mutex(&usbos_info->ctl_mutex) != DBUS_OK) {
		DBUSERR(("%s: txctl, acquire mutex failed \n", __FUNCTION__));
		return DBUS_ERR;
	}

	/* FIX: Clean this up */
	if (usbos_info->txctl_pending == TRUE) {
		DBUSERR(("%s: txctl pending...\n", __FUNCTION__));
		if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
			DBUSERR(("%s: txctl pending so wait...\n", __FUNCTION__));
			/* wait for txctl request to complete */
			NdisResetEvent(&usbos_info->txctl_PendingEvent);
			wait = NdisWaitEvent(&usbos_info->txctl_PendingEvent, 3000); /* wait 3s */
			if (wait == FALSE) {
				DBUSERR(("%s: timed out!!\n", __FUNCTION__));
				err = DBUS_ERR;
				goto fail;
			}
		} else {
			DBUSERR(("%s: Cannot wait...at DISPATCH_LEVEL\n", __FUNCTION__));
			err = DBUS_ERR;
			goto fail;
		}
	}

	err = dbus_usbos_txctl(usbos_info, (uchar *)pkt, len);

fail:
	dbus_usbos_release_mutex(&usbos_info->ctl_mutex);
	return err;
}

/* used by RPC call with return
 *  1) dbus_usbos_intf_recv_ctl (post buffer to ctl EP)
 *  2) dbus_usbos_intf_send_irb (send rpccall through bulk EP)
 *  3) wait dbus_usbos_ctl_rxpending_cb to indicate rxctl finish
 *     in upper layer, the rpc_callreturn is already obtained in passed buffer in 1)
 */

static int
dbus_usbos_intf_recv_ctl(void *bus, uint8 *buf, int len)
{
	int ret = 0;
	bool wait = FALSE;
	int err;
	usbos_info_t *usbinfo = (usbos_info_t *) bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	/* usb analyzer trigger, (1) activate this to verify it's latched in this normal case */
	/* dbus_usbos_ctl_send_checkcrc(usbinfo); */

	if (dbus_usbos_acquire_mutex(&usbinfo->ctl_mutex) != DBUS_OK) {
		DBUSERR(("%s: rxctl, acquire mutex failed \n", __FUNCTION__));
		return DBUS_ERR;
	}

	if (usbinfo->rxctl_pending == TRUE) {
		DBUSERR(("%s: rxctl is pending!\n", __FUNCTION__));

		NdisResetEvent(&usbinfo->rxctl_PendingEvent);
		wait = NdisWaitEvent(&usbinfo->rxctl_PendingEvent, 3000); /* wait 3sec */
		if (wait == FALSE) {
			DBUSERR(("%s: rxctl wait timed out \n", __FUNCTION__));
			err = DBUS_ERR;
			goto fail;
		}
	}
	err = dbus_usbos_ctl_rx_async(usbinfo, (uchar *)buf, len);

fail:
	dbus_usbos_release_mutex(&usbinfo->ctl_mutex);
	return err;
}

static int
dbus_usbos_intf_get_attrib(void *bus, dbus_attrib_t *attrib)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if ((usbos_info == NULL) || (attrib == NULL))
		return DBUS_ERR;

	attrib->bustype = DBUS_USB;
	attrib->vid = usbos_info->pub->attrib.vid;
	attrib->pid = usbos_info->pub->attrib.pid;
	attrib->devid = usbos_info->pub->attrib.devid;
	attrib->mtu = usbos_info->bulkpipe_mps;

	return DBUS_OK;
}

static int
dbus_usbos_intf_pnp(void *bus, int event)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	DBUSERR(("PNP: %s event %d\n", __FUNCTION__, event));

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (event == DBUS_PNP_RESUME) {
		/* on resume, all RCBs need to be reposted since they were all cancelled */
		MP_SET_FLAG(usbos_info, fMP_POST_READS);
		MP_CLEAR_FLAG(usbos_info, fMP_INIT_PNP);
#ifdef WDF_API
		dbus_usbos_rx_start_wdf(usbos_info);
#endif
		dbus_usbos_state_change(usbos_info, DBUS_STATE_UP);
	} else if (event == DBUS_PNP_SLEEP) {
		MP_SET_FLAG(usbos_info, fMP_INIT_PNP);
#ifdef WDF_API
		dbus_usbos_rx_stop_wdf(usbos_info, TRUE);
#endif
		dbus_usbos_state_change(usbos_info, DBUS_STATE_SLEEP);
	} else if (event == DBUS_PNP_DISCONNECT) {
		dbusos_stop(usbos_info);
		usbos_info->pnp_isdown = TRUE;

		/* Cancel all pending URBs
		 * for WDF the URBs are cancelled by WdfIoTargetStop in the subsequent
		 * mhalt->dbus_usbos_remove call
		 */
#ifdef WDM_API
		dbus_usbos_tx_freebusypkt(usbos_info, TRUE);
		if (dbus_usbos_acquire_mutex(&usbos_info->purge_mutex) == DBUS_OK) {
			dbus_usbos_rx_freebusypkt(usbos_info, TRUE);
			dbus_usbos_release_mutex(&usbos_info->purge_mutex);
		}
#endif
#if (0< 0x0600)
		if (usbos_info->txctl_pending == TRUE) {
			DBUSERR(("%s: txctl is pending!\n", __FUNCTION__));

			NdisResetEvent(&usbos_info->txctl_PendingEvent);
			/* NdisWaitEvent(&usbos_info->txctl_PendingEvent, 3000); wait 3sec */
		}
#endif	
	}

	return DBUS_OK;
}

static int
dbus_usbos_intf_up(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int err;
	int ret = DBUS_OK;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

	if (!dbus_usbos_txrx_resource_init(usbos_info))
		return DBUS_ERR;

	/* Success, indicate usbos_info is fully up */
	usbos_info->pub->busstate = DBUS_STATE_UP;

	return DBUS_OK;
}

static int
dbus_usbos_intf_down(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	usbos_info->pub->busstate = DBUS_STATE_DOWN;
	return DBUS_OK;
}

static int
dbus_usbos_intf_stop(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	int err;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

	dbusos_stop(usbos_info);
	return DBUS_OK;
}

static int
dbus_usbos_intf_shutdown(void *bus)
{
	usbos_info_t* usbinfo = bus;

	if (usbinfo == NULL)
		return DBUS_ERR;

	if (KeGetCurrentIrql() > APC_LEVEL) {
		DBUSERR(("%s attempted at IRQL 0x%x\n", __FUNCTION__, KeGetCurrentIrql()));
		return DBUS_ERR;
	}

	dbus_usbos_dl_cmd(usbinfo, (UCHAR)DL_REBOOT, NULL, 0);

	dbus_usbos_intf_stop(usbinfo);

	return DBUS_OK;
}

/*
 * This function could be called at DISPATCH_LEVEL and the call chain leads to
 * WdfIoTargetStop, which has to be called at PASSIVE_LEVEL as we are using a
 * continuous reader for rx. Thus enqueue a workitem.
 *
 * The rx_flowctl_pending counter works in tandem with the recv_resume function
 */
static int
dbus_usbos_intf_recv_stop(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

#ifdef WDF_API
	WdfSpinLockAcquire(usbos_info->WdfRecv_SpinLock);
	usbos_info->rx_flowctl_pending++;
	WdfSpinLockRelease(usbos_info->WdfRecv_SpinLock);

	/* WDF allows multiple enqueues, but will execute workitem only once */
	WdfWorkItemEnqueue(usbos_info->rx_flowctl_WorkItem);
#endif

	/* WDM needs to do nothing since the recv_irb stop posting */

	return DBUS_OK;
}

/*
 * This function could be called at DISPATCH_LEVEL
 * Since it will invoke WdfIoTargetStart, a PASSIVE_LEVEL workitem is used to avoid the
 *  race that it will preempt WdfIoTargetStop, which was queued previously at PASSIVE_LEVEL
 */
static int
dbus_usbos_intf_recv_resume(void *bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL)
		return DBUS_ERR;

#ifdef WDF_API
	WdfSpinLockAcquire(usbos_info->WdfRecv_SpinLock);
	usbos_info->rx_flowctl_pending--;
	WdfSpinLockRelease(usbos_info->WdfRecv_SpinLock);

	/* WDF allows multiple enqueues, but will execute workitem only once */
	WdfWorkItemEnqueue(usbos_info->rx_flowctl_WorkItem);
#endif

	/* WDM needs to do nothing since the recv_irb posted more */

	return DBUS_OK;
}

static int
dbus_usbos_intf_set_config(void *bus, dbus_config_t *config)
{
	int err = DBUS_ERR;
	usbos_info_t* usbos_info = bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (config->config_id == DBUS_CONFIG_ID_RXCTL_DEFERRES) {
		usbos_info->rxctl_deferrespok = config->rxctl_deferrespok;
		err = DBUS_OK;
	}

	return err;
}

int
dbus_usbos_wait(usbos_info_t *usbinfo, uint16 ms)
{
	NdisStallExecution(ms * 1000);
	return DBUS_OK;
}

bool
dbus_usbos_dl_cmd(usbos_info_t *usbinfo, UCHAR cmd,  void *buffer, int buflen)
{
	NTSTATUS ntStatus;
	PURB urb = NULL;
	struct _URB_CONTROL_DESCRIPTOR_REQUEST ctl_desc_req;

	urb = (PURB)&ctl_desc_req;
	dbus_usbos_urb_buildgeneric(urb);

	if (cmd == DL_REBOOT) {
		/* FIX: Taken from shutdown()
		 * Need to update this function to support both directions.
		 */
		urb->UrbControlVendorClassRequest.TransferFlags = 0;
	}
	if (cmd == DL_GO) {
		/* for USB30 non-disconnect target on Windows */
		urb->UrbControlVendorClassRequest.Index = 1;
	}
	urb->UrbControlVendorClassRequest.TransferBufferLength = buflen;
	urb->UrbControlVendorClassRequest.TransferBuffer =  buffer;
	urb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
	urb->UrbControlVendorClassRequest.Request = (UCHAR)cmd;
	ntStatus = dbus_usbos_urb_submit_sync(usbinfo, urb, FALSE);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: BCMDL, Bad DLCMD ntStatus = 0x%x\n", __FUNCTION__, ntStatus));
		return FALSE;
	}

	return TRUE;
}

int
dbus_write_membytes(usbos_info_t* usbinfo, bool set, uint32 address, uint8 *data, uint size)
{

	DBUSTRACE(("Enter:%s\n", __FUNCTION__));
	return -1;
}

static bool
dbus_usbos_intf_recv_needed(void *bus)
{
	usbos_info_t* usbinfo = bus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return FALSE;

	return (usbinfo->rx_pkt_busyrecv == 0);
}

static void *
dbus_usbos_intf_exec_rxlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	usbos_info_t *usbinfo = bus;
	void *ret;
	if (usbinfo == NULL)
		return NULL;

#ifdef WDF_API
	WdfSpinLockAcquire(usbinfo->WdfRecv_SpinLock);
#else
	NdisAcquireSpinLock(&usbinfo->rx_spinlock);
#endif

	ret = cb(args);

#ifdef WDF_API
	WdfSpinLockRelease(usbinfo->WdfRecv_SpinLock);
#else
	NdisReleaseSpinLock(&usbinfo->rx_spinlock);
#endif

	return ret;
}
static void *
dbus_usbos_intf_exec_txlock(void *bus, exec_cb_t cb, struct exec_parms *args)
{
	usbos_info_t *usbinfo = bus;
	void *ret;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return NULL;

#ifdef WDF_API
	WdfSpinLockAcquire(usbinfo->tx_q_lowprio.write_req_array_spinlock);
#else
	NdisAcquireSpinLock(&usbinfo->tx_spinlock);
#endif

	ret = cb(args);

#ifdef WDF_API
	WdfSpinLockRelease(usbinfo->tx_q_lowprio.write_req_array_spinlock);
#else
	NdisReleaseSpinLock(&usbinfo->tx_spinlock);
#endif

	return ret;
}

/* the DL_CHECK_CRC is not supported, but the bRequeset is unique, just for debugging */
static bool
dbus_usbos_ctl_send_checkcrc(usbos_info_t* usbinfo)
{
	PURB urb = NULL;
	bootrom_id_t id;
	struct _URB_CONTROL_DESCRIPTOR_REQUEST ctl_desc_req;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return FALSE;

	urb = (PURB)&ctl_desc_req;
	id.chip = 0xDEAD;

	DBUSERR(("dbus_usbos_ctl_send_checkcrc, send\n"));
	dbus_usbos_urb_buildgeneric(urb);
	DBUSERR(("dbus_usbos_ctl_send_checkcrc, done\n"));

	/* ignore the result for now */
	return TRUE;
}

static int
dbus_usbos_acquire_mutex(PKMUTEX mutex)
{
	NTSTATUS ntStatus;
	LARGE_INTEGER timeout;

	if (KeGetCurrentIrql() == DISPATCH_LEVEL) {
		DBUSTRACE(("%s: at dispatch \n", __FUNCTION__));
		bzero(&timeout, sizeof(timeout));
	} else {
		timeout.QuadPart = -10000 * 7000;
	}

	ntStatus = KeWaitForMutexObject(mutex, Executive, KernelMode, FALSE, &timeout);
	if (ntStatus != STATUS_SUCCESS) {
		DBUSERR(("%s: ERROR, acquire mutex failed = 0x%x\n", __FUNCTION__, ntStatus));
		return DBUS_ERR;
	}

	return DBUS_OK;
}

static int
dbus_usbos_release_mutex(PKMUTEX mutex)
{
	KeReleaseMutex(mutex, FALSE);
	return DBUS_OK;
}

int
dbus_bus_osl_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, dbus_intf_t **intf, void *param1, void *param2)
{
	probe_cb = prcb;
	probe_arg = prarg;
	disconnect_cb = discb;
	*intf = &dbus_usbos_intf;

	return dbus_usbos_probe();
}

int
dbus_bus_osl_deregister(void)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	dbus_usbos_disconnect();
	return DBUS_OK;
}

void *
dbus_usbos_intf_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs)
{
	usbos_info_t *usbos_info;

	DBUSTRACE(("%s\n", __FUNCTION__));

	usbos_info = MALLOC(pub->osh, sizeof(usbos_info_t));
	if (usbos_info == NULL)
		return NULL;

	bzero(usbos_info, sizeof(usbos_info_t));
	usbos_info->pub = pub;
	usbos_info->cbarg = cbarg;
	usbos_info->cbs = cbs;
	usbos_info->tx_low_watermark = usbos_info->pub->ntxq / 4;
	usbos_info->AdapterHandle = pub->sh->adapterhandle;

	/* This is for both XP and Vista */
	NdisMGetDeviceProperty(usbos_info->AdapterHandle, &usbos_info->pdo, &usbos_info->fdo,
		&usbos_info->nextDeviceObj, NULL, NULL);

	usbos_info->TopOfStackDO = usbos_info->nextDeviceObj;

	/* bringup/config/activate the usb dongle device */
	if (dbus_usbos_init_device(usbos_info) < 0)
		goto error;

	return (void *)usbos_info;

error:
	if (usbos_info) {
		MFREE(pub->osh, usbos_info, sizeof(usbos_info_t));
	}
	return NULL;
}

void
dbus_usbos_intf_detach(dbus_pub_t *pub, void * bus)
{
	usbos_info_t *usbos_info = (usbos_info_t *) bus;
	osl_t *osh = pub->osh;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbos_info == NULL) {
		return;
	}

	dbusos_stop(usbos_info);
	dbus_usbos_remove(osh, usbos_info);
	MFREE(osh, usbos_info, sizeof(usbos_info_t));
}


#ifdef WDF_API

static NTSTATUS
dbus_usbos_ctl_req_allocate_wdf(usbos_info_t* usbinfo, USHORT max)
{
	WDF_OBJECT_ATTRIBUTES   requestAttributes, objectAttribs;
	UCHAR                   reqIdx;
	NTSTATUS                ntStatus = STATUS_SUCCESS;
	dbus_usbos_wdf_res_ctr*  cRes = &usbinfo->ctl_resource_wdf;

	DBUSTRACE(("%s\n", __FUNCTION__));

	WDF_OBJECT_ATTRIBUTES_INIT(&objectAttribs);
	objectAttribs.ParentObject = usbinfo->WdfDevice;
	ntStatus = WdfSpinLockCreate(&objectAttribs, &cRes->control_req_array_spinlock);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Could not create request lock: status(0x%08X)",
			__FUNCTION__, ntStatus));
		return ntStatus;
	}

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&requestAttributes, dbus_usbos_wdf_ctl_info_t);
	requestAttributes.ParentObject = usbinfo->UsbDevice;

	for (reqIdx = 0; reqIdx < max; reqIdx++) {
		WDFREQUEST* next;
		dbus_usbos_wdf_ctl_info_t* cri;
		WDF_USB_CONTROL_SETUP_PACKET  setupPacket;

		next = &cRes->control_req_array[reqIdx];

		ntStatus = WdfRequestCreate(&requestAttributes, usbinfo->IoTarget, next);
		if (!NT_SUCCESS(ntStatus)) {
			DBUSERR(("%s: Could not create request: status(0x%08X)",
				__FUNCTION__, ntStatus));
			cRes->control_req_max = reqIdx;
			cRes->NextAvailReqIndex = reqIdx;
			return ntStatus;
		}
		/* By doing this we allocate resources like the IRP upfront eventhough
		 * we may not have all the data yet.
		 */
		ntStatus = WdfUsbTargetDeviceFormatRequestForControlTransfer(usbinfo->UsbDevice,
			*next,
			&setupPacket,
			NULL,
			NULL);
		if (!NT_SUCCESS(ntStatus)) {
			DBUSERR(("%s: WdfUsbTargetDeviceFormatRequestForControlTransfer"
				" failed  0x%x \n", __FUNCTION__, ntStatus));
			return ntStatus;
		}

		/* set REQUEST_CONTEXT  parameters. */
		cri = _get_ctx_ctl_wdf(*next);
		cri->usbinfo = usbinfo;
	}

	cRes->control_req_max = max;
	cRes->NextAvailReqIndex = max;

	return ntStatus;
}

static void
dbus_usbos_ctl_req_deallocate_wdf(usbos_info_t* usbinfo)
{
	dbus_usbos_wdf_res_ctr*	cRes = &usbinfo->ctl_resource_wdf;
	USHORT reqIdx;

	DBUSTRACE(("%s\n", __FUNCTION__));

	/* Don't need a lock here because nobody can be making control request
	 * by the time this routine is called
	 */

	if (cRes->control_req_array_spinlock != NULL) {
		WdfObjectDelete(cRes->control_req_array_spinlock);
		cRes->control_req_array_spinlock = NULL;
	}
}

static WDFREQUEST
dbus_usbos_ctl_req_get_wdf(usbos_info_t* usbinfo)
{
	WDFREQUEST controlRequest = NULL;
	dbus_usbos_wdf_res_ctr *cRes = &usbinfo->ctl_resource_wdf;

	DBUSTRACE(("%s\n", __FUNCTION__));

	WdfSpinLockAcquire(cRes->control_req_array_spinlock);
	if (cRes->NextAvailReqIndex != 0) {
		/* Request is available */
		--(cRes->NextAvailReqIndex);
		controlRequest = cRes->control_req_array[cRes->NextAvailReqIndex];
		cRes->control_req_array[cRes->NextAvailReqIndex] = NULL;
	}

	WdfSpinLockRelease(cRes->control_req_array_spinlock);

	/* TODO : If we run out of control requests then allocate on the fly */
	return controlRequest;
}

static void
dbus_usbos_ctl_req_return_wdf(usbos_info_t* usbinfo, WDFREQUEST Request)
{
	dbus_usbos_wdf_res_ctr *cRes = &usbinfo->ctl_resource_wdf;
	WDF_REQUEST_REUSE_PARAMS params;

	DBUSTRACE(("%s\n", __FUNCTION__));

	WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
	WdfRequestReuse(Request, &params);

	WdfSpinLockAcquire(cRes->control_req_array_spinlock);

	if (cRes->NextAvailReqIndex >= cRes->control_req_max) {
		DBUSERR(("%s: Control Request list is already full!", __FUNCTION__));
	}
	cRes->control_req_array[cRes->NextAvailReqIndex++] = Request;

	WdfSpinLockRelease(cRes->control_req_array_spinlock);
}

static void
dbus_usbos_ctl_req_completion_rx_wdf(WDFREQUEST  Request, WDFIOTARGET Target,
	PWDF_REQUEST_COMPLETION_PARAMS CompletionParams, WDFCONTEXT Context)
{
	usbos_info_t* usbinfo;
	dbus_usbos_wdf_ctl_info_t* cri = (dbus_usbos_wdf_ctl_info_t*) Context;
	PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;
	NTSTATUS                    status;

	DBUSTRACE(("%s\n", __FUNCTION__));

	UNREFERENCED_PARAMETER(Target);
	usbinfo = cri->usbinfo;

	status = CompletionParams->IoStatus.Status;

	/* For usb devices, we should look at the Usb.Completion param. */
	usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: VendorRequest failed, request Status 0x%x UsbdStatus 0x%x\n",
			__FUNCTION__, status, usbCompletionParams->UsbdStatus));
		usbinfo->rxctl_pending = FALSE;
		/* FAILED */
	} else
		dbus_usbos_ctl_rxpending_cb(usbinfo, status);

	dbus_usbos_ctl_req_return_wdf(usbinfo, Request);
}

static void
dbus_usbos_ctl_req_completion_tx_wdf(WDFREQUEST  Request, WDFIOTARGET Target,
	PWDF_REQUEST_COMPLETION_PARAMS CompletionParams, WDFCONTEXT Context)
{
	usbos_info_t* usbinfo;
	dbus_usbos_wdf_ctl_info_t* cri;
	PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;
	NTSTATUS                    status;

	DBUSTRACE(("%s\n", __FUNCTION__));

	UNREFERENCED_PARAMETER(Target);

	status = CompletionParams->IoStatus.Status;

	/* For usb devices, we should look at the Usb.Completion param. */
	usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
	if (!NT_SUCCESS(status)) {
		DBUSERR(("%s: VendorRequest failed, request Status 0x%x UsbdStatus 0x%x\n",
		__FUNCTION__, status, usbCompletionParams->UsbdStatus));
	}
	cri = (dbus_usbos_wdf_ctl_info_t*)Context;
	usbinfo = cri->usbinfo;

	if (usbinfo->cbarg && usbinfo->cbs) {
		if (usbinfo->cbs->ctl_complete)
			usbinfo->cbs->ctl_complete(usbinfo->cbarg, DBUS_CBCTL_WRITE, status);
	}

	dbus_usbos_ctl_req_return_wdf(usbinfo, Request);
}

/*
Send/recv an asynchronous request on the pipe
Arguments:
Request - Request to be used to send the Vendor Control request.
Value - Value to be used in the Control Transfer Setup packet.
Length - Amount of data to be sent.
Buffer - Buffer which has the data to be sent.
Index - Index to be used in the Control Transfer Setup packet.
*/
static NTSTATUS
dbus_usbos_ctl_req_submit_async_wdf(usbos_info_t* usbinfo, bool tx, BYTE Request, USHORT Value,
	ULONG Length, PVOID Buffer, USHORT Index)
{
	WDFREQUEST	controlRequest;
	WDFIOTARGET	ioTarget;
	NTSTATUS	ntStatus;
	dbus_usbos_wdf_ctl_info_t* cri;
	WDF_USB_CONTROL_SETUP_PACKET	controlSetupPacket;
	WDFMEMORY	memoryObject;
	WDF_USB_BMREQUEST_DIRECTION dir;

	DBUSTRACE(("%s\n", __FUNCTION__));

	controlRequest = dbus_usbos_ctl_req_get_wdf(usbinfo);
	if (controlRequest == NULL) {
		/* No requests left */
		DBUSERR(("%s: Out of control request\n", __FUNCTION__));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	cri = _get_ctx_ctl_wdf(controlRequest);

	if (tx) {
		dir = BmRequestHostToDevice;
		/* !!! Copy the buffer which is allocated on the stack to the heap allocated from
		 * request context because we will be sending an async request
		 */
		NdisMoveMemory(cri->Data, (PUCHAR)Buffer, Length);
	} else {
		dir = BmRequestDeviceToHost;
	}

	WDF_USB_CONTROL_SETUP_PACKET_INIT_VENDOR(&controlSetupPacket,
		dir,
		BmRequestToInterface,	/* ?? use BmRequestToDevice */
		Request,
		Value,
		Index);
	/* Create a valid handle to memory object */
	if (tx) {
		ntStatus = WdfMemoryCreatePreallocated(WDF_NO_OBJECT_ATTRIBUTES, (PVOID)cri->Data,
			Length, &memoryObject);
	} else {
		/* submit original buffer directly to recv the data */
		ntStatus = WdfMemoryCreatePreallocated(WDF_NO_OBJECT_ATTRIBUTES, (PVOID)Buffer,
			Length, &memoryObject);
	}

	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: WdfMemoryCreatePreallocated: Failed - 0x%x \n",
			__FUNCTION__, ntStatus));
		return ntStatus;
	}
	ntStatus = WdfUsbTargetDeviceFormatRequestForControlTransfer(usbinfo->UsbDevice,
		controlRequest,
		&controlSetupPacket,
		memoryObject,
		NULL);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: WdfUsbTargetDeviceFormatRequestForControlTransfer: Failed - 0x%x \n",
			__FUNCTION__, ntStatus));
		return ntStatus;
	}
	if (tx) {
		WdfRequestSetCompletionRoutine(controlRequest,
			dbus_usbos_ctl_req_completion_tx_wdf, cri);
	} else {
		WdfRequestSetCompletionRoutine(controlRequest,
			dbus_usbos_ctl_req_completion_rx_wdf, cri);
	}

	/* !!! SEND IT !!! */
	if (WdfRequestSend(controlRequest, usbinfo->IoTarget, 0) == FALSE) {
		/* Failure - Return request */
		DBUSERR(("%s: WdfRequestSend: Failed - 0x%x \n", __FUNCTION__, ntStatus));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		goto error;
	}
	WdfObjectDelete(memoryObject);

	return ntStatus;

error:
	dbus_usbos_ctl_req_return_wdf(usbinfo, controlRequest);
	WdfObjectDelete(memoryObject);
	DBUSERR(("%s ERR!!\n", __FUNCTION__));

	return ntStatus;
}

static void
dbus_usbos_stop_all_pipes_wdf(usbos_info_t* usbinfo)
{
	DBUSERR(("%s\n", __FUNCTION__));

	/* stop the in endpoint first */
	dbus_usbos_rx_stop_wdf(usbinfo, TRUE);

	WdfWaitLockAcquire(usbinfo->PipeState_WaitLock, NULL);

	WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(usbinfo->TxPipeHandleWdf),
		WdfIoTargetCancelSentIo);

	WdfWaitLockRelease(usbinfo->PipeState_WaitLock);
}

/* ============ RX ================ */

static bool
dbus_usbos_rx_contread_setup_wdf(usbos_info_t *usbinfo, WDFUSBPIPE ReadPipe)
{
	NDIS_STATUS	ndisStatus;
	NTSTATUS	ntStatus;
	CHAR		dummyMdlBuffer[1];
	WDF_USB_CONTINUOUS_READER_CONFIG  config;

	DBUSTRACE(("%s\n", __FUNCTION__));

	/* Tell the framework that it's okay to read less than MaximumPacketSize */
	WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(ReadPipe);

	WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&config, dbus_usbos_rx_contread_cb_wdf, usbinfo,
		DBUS_BUFFER_SIZE_RX);

	/*
	 * Reader requests are not posted to the target automatically.
	 * Driver must explictly call WdfIoTargetStart to kick start the reader. (e.g. D0Entry)
	 *
	 * By defaut, framework queues two requests to the target endpoint
	 * Driver can configure up to 10 requests with CONFIG macro.
	 *
	 * I have tried from 4 to 10 readers and it hasn't impacted performance that much
	 * TODO : Try adjusting the number of readers to see the optimal value
	 */
	config.NumPendingReads = (UCHAR)usbinfo->pub->nrxq;	/* framework may limit it to 10 */

	config.EvtUsbTargetPipeReadersFailed = dbus_usbos_rx_contread_fail_cb_wdf;
	config.HeaderLength = MmSizeOfMdl(dummyMdlBuffer, DBUS_BUFFER_SIZE_RX);
	usbinfo->ReaderMdlSize = (ULONG)config.HeaderLength;

	/* WdfUsbTargetPipeConfigContinuousReader method configures the framework to continuously
	 *   read from a specified USB pipe
	 * After this, WdfIoTargetStart/WdfIoTargetStop must be called to start/stop the reader
	 */
	ntStatus = WdfUsbTargetPipeConfigContinuousReader(ReadPipe, &config);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: WdfUsbTargetPipeConfigContinuousReader failed with error 0x%x",
			__FUNCTION__, ntStatus));
	}

	NT_STATUS_TO_NDIS_STATUS(ntStatus, &ndisStatus);
	return (ndisStatus == NDIS_STATUS_SUCCESS);
}

/*
 * Callback function to stop/resume the continuous reader.
 * On a single CPU machine the rx_flowctl_pending should toggle between 0 and 1,
 * but on a multiple cpu/core machine the range could be higher.
 *
 * Spinlock required as the callback happens at PASSIVE_LEVEL and needs
 * to sync access with dbus_usbos_intf_recv_stop / dbus_usbos_intf_recv_resume
 * which can come in at DISPATCH_LEVEL
 */
static VOID
dbus_usbos_rx_flowctl(WDFWORKITEM  WorkItem)
{
	usbos_info_t *usbinfo;
	usbinfo_workitem_ctx_ndis6 *usbinfo_ctx;

	DBUSTRACE(("%s\n", __FUNCTION__));

	usbinfo_ctx = _get_context_workitem(WorkItem);
	usbinfo = usbinfo_ctx->usbinfo;

	if (usbinfo->pub->busstate == DBUS_STATE_DOWN) {
		DBUSERR(("%s: ERROR, bus is down\n", __FUNCTION__));
		return;
	}

	WdfSpinLockAcquire(usbinfo->WdfRecv_SpinLock);

	if (usbinfo->rx_flowctl_on && (usbinfo->rx_flowctl_pending <= 0)) {
		usbinfo->rx_flowctl_on = FALSE;
		WdfSpinLockRelease(usbinfo->WdfRecv_SpinLock);
		dbus_usbos_rx_start_wdf(usbinfo);
	} else if (!usbinfo->rx_flowctl_on && (usbinfo->rx_flowctl_pending > 0)) {
		usbinfo->rx_flowctl_on = TRUE;
		WdfSpinLockRelease(usbinfo->WdfRecv_SpinLock);
		dbus_usbos_rx_stop_wdf(usbinfo, FALSE);
	} else {
		WdfSpinLockRelease(usbinfo->WdfRecv_SpinLock);
	}
}

static bool
dbus_usbos_rx_start_wdf(usbos_info_t *usbinfo)
{
	NTSTATUS ntStatus;
	bool rt = TRUE;

	DBUSTRACE(("%s\n", __FUNCTION__));

	WdfWaitLockAcquire(usbinfo->PipeState_WaitLock, NULL);

	ntStatus = WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(usbinfo->RxPipeHandleWdf));

	if (!NT_SUCCESS(ntStatus)) {
		rt = FALSE;
		DBUSERR(("%s: Failed to start RxPipeHandleWdf\n", __FUNCTION__));
	}

	WdfWaitLockRelease(usbinfo->PipeState_WaitLock);

	return rt;
}

static void
dbus_usbos_rx_stop_wdf(usbos_info_t *usbinfo, bool cancel)
{
	DBUSTRACE(("%s\n", __FUNCTION__));

	/* For contread, WdfIoTargetStop should be called at PASSIVE_LEVEL */
	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	WdfWaitLockAcquire(usbinfo->PipeState_WaitLock, NULL);

	if (cancel)
		WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(usbinfo->RxPipeHandleWdf),
			WdfIoTargetCancelSentIo);
	else
		WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(usbinfo->RxPipeHandleWdf),
			WdfIoTargetWaitForSentIoToComplete);

	WdfWaitLockRelease(usbinfo->PipeState_WaitLock);
}

static bool
dbus_usbos_rx_res_alloc_wdf(usbos_info_t *usbinfo)
{
	NDIS_STATUS ndisStatus;

	DBUSTRACE(("%s\n", __FUNCTION__));

	usbinfo->device_goingdown = FALSE;

	usbinfo->ReaderCount = 1; /* bias the count to 1 */

	KeInitializeEvent(&usbinfo->ReaderWaitEvent, SynchronizationEvent, FALSE);

	/* this should be called once the USB device is created */
	return dbus_usbos_rx_contread_setup_wdf(usbinfo, usbinfo->RxPipeHandleWdf);
}

static void
dbus_usbos_rx_res_dealloc_wdf(usbos_info_t *usbinfo)
{
	LONG readerCount;

	DBUSTRACE(("%s\n", __FUNCTION__));

	/* stop the continous reader if it hasn't already */
	dbus_usbos_rx_stop_wdf(usbinfo, TRUE);

	usbinfo->device_goingdown = TRUE;
	/*
	 * We need to make sure all the Continous reader callbacks
	 * have been processed before we return to prevent leaking WDFMEMORY objects
	 * since we take a reference on these in the Continous reader callback.
	 * NOTE:  We initialized the ReaderCount to 1 so we can decrement the count here
	 */
	readerCount = InterlockedDecrement(&usbinfo->ReaderCount);

	if (readerCount != 0) {
		DBUSERR(("%s: Waiting for reader Cb's to complete\n", __FUNCTION__));
		KeWaitForSingleObject(&usbinfo->ReaderWaitEvent, Executive, KernelMode, TRUE, NULL);
	}
}

/* this is called at <= DISPATCH LEVEL */
static VOID
dbus_usbos_rx_contread_cb_wdf(WDFUSBPIPE Pipe, WDFMEMORY BufferHdl, size_t NumBytesRead,
	WDFCONTEXT Context)
{
	usbos_info_t * usbinfo = (usbos_info_t *) Context;
	PVOID buffer;
	PUCHAR  tempBuffer;
	LONG readerCount;
	dbus_irb_rx_t	*rxirb;

	DBUSTRACE(("%s\n", __FUNCTION__));

	UNREFERENCED_PARAMETER(Pipe);

	/* if (KeGetCurrentIrql() == DISPATCH_LEVEL) */

	InterlockedIncrement(&usbinfo->ReaderCount);

	if (usbinfo->device_goingdown) {
		InterlockedDecrement(&usbinfo->ReaderCount);
		return;
	}

	/* The buffer we get points to the header and not the actual data read
	 * The header is used to allocate an MDL which  maps the Virtual address
	 * of the buffer which has the data read from device
	 */
	tempBuffer = WdfMemoryGetBuffer(BufferHdl, NULL);
	buffer = tempBuffer + usbinfo->ReaderMdlSize;

	if (NumBytesRead == 0 || tempBuffer == NULL) {
		DBUSERR(("%s recvd 0 bytes or NULL buffer\n", __FUNCTION__));
		goto out;
	}

	DBUSTRACE(("%s: bytesread %d tempBuf 0x%x\n", __FUNCTION__, NumBytesRead, buffer));

	/* Prepare the MDL so the physical page mappings are done
		pkt->pMdl = (PMDL)tempBuffer;
		MmInitializeMdl(pkt->pMdl, buffer, NumBytesRead);
		MmBuildMdlForNonPagedPool(pkt->pMdl);
		pkt->BufferHdl = BufferHdl;
	*/

	/* ! send the pkt up */
	/* WdfSpinLockAcquire(usbinfo->WdfRecv_SpinLock); */

	WdfObjectReference(BufferHdl);	/* hold on the buffer for upper layer */

	rxirb = (dbus_irb_rx_t *)dbus_usbos_txrx_getirb(usbinfo, FALSE);
	if (rxirb != NULL) {
		if (dbus_usbos_rx_data_sendup(usbinfo, rxirb, (uchar*)buffer, NumBytesRead)
			!= DBUS_OK) {
			dbus_usbos_rx_data_sendup_null(usbinfo, rxirb, DBUS_ERR_RXFAIL);
		}
	} else {
		if (usbinfo->pub->busstate != DBUS_STATE_DOWN) {
			DBUSERR(("%s: no rxirb, fatal rx pkt tossed\n", __FUNCTION__));
			ASSERT(0);
		} else {
			DBUSERR(("%s: no rxirb after down, ignoring\n", __FUNCTION__));
		}
	}

	WdfObjectDereference(BufferHdl);

	/* WdfSpinLockRelease(usbinfo->WdfRecv_SpinLock); */

out:
	readerCount = InterlockedDecrement(&usbinfo->ReaderCount);

	if (readerCount == 0) {
		DBUSERR(("readerCount == 0, set ReaderWaitEvent\n"));
		KeSetEvent(&usbinfo->ReaderWaitEvent, IO_NO_INCREMENT, FALSE);
	}

	return;
}

/*
 *  a continuous reader has reported an error while processing a read request.
 * Return Value:
 *  If TRUE, causes the framework to reset the USB pipe and then restart the continuous reader.
 *  If FASLE, the framework does not reset the device or restart the continuous reader.
 *  If this event is not registered, framework default action is to reset the pipe and
 *   restart the reader.
 */
static BOOLEAN
dbus_usbos_rx_contread_fail_cb_wdf(WDFUSBPIPE Pipe, NTSTATUS Status, USBD_STATUS UsbdStatus)
{
	UNREFERENCED_PARAMETER(Status);

	DBUSERR(("%s:  NTSTATUS 0x%x, UsbdStatus 0x%x\n", __FUNCTION__, Status, UsbdStatus));

	if (UsbdStatus == USBD_STATUS_STALL_PID) {
		/* Send a clear stall URB. Pipereset will clear the stall. */
		WdfUsbTargetPipeResetSynchronously(Pipe, NULL, NULL);
	}

	return TRUE;
}


/* ============ TX ================ */


/*
 * Allocate         URB Memory
 * Allocate         WDFMEMORY objects
 */
static bool
dbus_usbos_tx_req_alloc_wdf(usbos_info_t *usbinfo, uint NumTxd)
{
	dbus_usbos_wdf_res_tx	*write_res;
	BOOLEAN			retValue;
	USHORT			max;

	DBUSTRACE(("%s\n", __FUNCTION__));

	max = (NumTxd > DBUS_USBOS_MAX_WRITE_REQ) ? DBUS_USBOS_MAX_WRITE_REQ : (BYTE)NumTxd;

	WDF_REQUEST_SEND_OPTIONS_INIT(&usbinfo->WriteRequestOptions, 0);
	/* set a timer for 2 seconds, used by sync requests */
	WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&usbinfo->WriteRequestOptions,
		WDF_REL_TIMEOUT_IN_SEC(2));

	write_res = &usbinfo->tx_q_lowprio;
	write_res->write_req_max = max;
	write_res->NextAvailReqIndex = max;
	retValue = dbus_usbos_tx_req_worker_allocate_wdf(usbinfo, write_res);

	return  retValue;
}

static void
dbus_usbos_tx_req_dealloc_wdf(usbos_info_t *usbinfo)
{
	dbus_usbos_wdf_res_tx    *write_res;

	DBUSTRACE(("%s\n", __FUNCTION__));

	write_res  = &usbinfo->tx_q_lowprio;
	dbus_usbos_tx_req_worker_dealloc_wdf(usbinfo, write_res);
}

static void
dbus_usbos_tx_req_worker_dealloc_wdf(usbos_info_t *usbinfo, dbus_usbos_wdf_res_tx *write_res)
{
	USHORT reqIdx;

	DBUSTRACE(("%s\n", __FUNCTION__));

	while (!IsListEmpty(&usbinfo->tx_free_list))
		NdisInterlockedRemoveHeadList(&usbinfo->tx_free_list, &usbinfo->tx_spinlock);

	if (write_res->write_req_array_spinlock != NULL) {
		WdfObjectDelete(write_res->write_req_array_spinlock);
		write_res->write_req_array_spinlock = NULL;
	}
}

static bool
dbus_usbos_tx_req_worker_allocate_wdf(usbos_info_t *usbinfo, dbus_usbos_wdf_res_tx *write_res)
{
	WDFIOTARGET             ioTarget;
	WDF_OBJECT_ATTRIBUTES   requestAttributes;
	UCHAR                   reqIdx;
	NTSTATUS                ntStatus;
	WDFMEMORY               urbMemory;
	WDF_OBJECT_ATTRIBUTES   objectAttribs;
	struct _URB_BULK_OR_INTERRUPT_TRANSFER *urbBuffer;
	CHAR                    dummyBuffer[1];
	WDFUSBPIPE		UsbPipe = usbinfo->TxPipeHandleWdf;
	PUCHAR			pBuf = NULL;

	DBUSTRACE(("%s\n", __FUNCTION__));

	ioTarget = WdfUsbTargetPipeGetIoTarget(UsbPipe);

	WDF_OBJECT_ATTRIBUTES_INIT(&objectAttribs);
	objectAttribs.ParentObject = usbinfo->WdfDevice;

	ntStatus = WdfSpinLockCreate(&objectAttribs, &write_res->write_req_array_spinlock);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Could not create request lock: status(0x%08X)",
			__FUNCTION__, ntStatus));
		return FALSE;
	}

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&requestAttributes, dbus_usbos_TCB_wdf);
	requestAttributes.ParentObject = usbinfo->UsbDevice;

	pBuf = usbinfo->p_txbuf_memory;

	for (reqIdx = 0; reqIdx < write_res->write_req_max; reqIdx++) {
		WDFREQUEST		*next;
		USBD_PIPE_HANDLE	usbdPipeHandle;
		dbus_usbos_TCB_wdf	*writeContext;

		next = &write_res->write_req_array[reqIdx];
		ntStatus = WdfRequestCreate(&requestAttributes, ioTarget, next);
		if (!NT_SUCCESS(ntStatus)) {
			DBUSERR(("%s: Could not create request: status(0x%08X)",
				__FUNCTION__, ntStatus));
			write_res->write_req_max = reqIdx;
			write_res->NextAvailReqIndex = reqIdx;
			return FALSE;
		}

		WDF_OBJECT_ATTRIBUTES_INIT(&objectAttribs);
		objectAttribs.ParentObject = *next;
#if (0>= 0x0630)
		ntStatus = WdfUsbTargetDeviceCreateUrb(usbinfo->UsbDevice,
			&objectAttribs, &urbMemory, &(PURB)urbBuffer);
#else
		ntStatus = WdfMemoryCreate(&objectAttribs, NonPagedPool, 0,
			sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER), &urbMemory, &urbBuffer);
#endif 
		if (!NT_SUCCESS(ntStatus)) {
			DBUSERR(("s: Could not create request lock: status(0x%08X)",
				__FUNCTION__, ntStatus));
			write_res->write_req_max = reqIdx;
			write_res->NextAvailReqIndex = reqIdx;
			return FALSE;
		}

		usbdPipeHandle = WdfUsbTargetPipeWdmGetPipeHandle(UsbPipe);
		UsbBuildInterruptOrBulkTransferRequest((PURB)urbBuffer,
			sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
			usbdPipeHandle,
			dummyBuffer,
			NULL,
			sizeof(dummyBuffer),
			USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK,
			NULL);
		/*
		 * !!! By calling WdfUsbTargetPipeFormatRequestForUrb the frameworks allocate a lot
		 *  of resources like the underlying IRP for the request and hence it is better to
		 *  do it at initilization to prevent an avoidable failure later
		 */
		ntStatus = WdfUsbTargetPipeFormatRequestForUrb(UsbPipe, *next, urbMemory, NULL);
		if (!NT_SUCCESS(ntStatus)) {
			DBUSERR(("%s: WdfUsbTargetPipeFormatRequestForUrb: Failed 0x%x \n",
				__FUNCTION__, ntStatus));
			return FALSE;
		}

		/* set REQUEST_CONTEXT  parameters */
		writeContext = _get_ctx_TCB(*next);
		writeContext->usbinfo = usbinfo;
		writeContext->UrbMemory = urbMemory;
		writeContext->Urb = (PURB)urbBuffer;
		writeContext->UsbdPipeHandle = usbdPipeHandle;
		writeContext->UsbPipe = UsbPipe;
		writeContext->IoTarget = ioTarget;

		writeContext->txDataBuf = pBuf;

		writeContext->pData = writeContext->txDataBuf;
		NdisInterlockedInsertTailList(&usbinfo->tx_free_list, &writeContext->List,
			&usbinfo->tx_spinlock);
		pBuf = pBuf + DBUS_BUFFER_SIZE_TX;
	}
	return TRUE;
}

static void
dbus_usbos_tx_req_get_wdf(usbos_info_t *usbinfo, WDFREQUEST *writeRequest)
{
	dbus_usbos_wdf_res_tx *write_res = &usbinfo->tx_q_lowprio;

	DBUSTRACE(("%s\n", __FUNCTION__));

	*writeRequest = NULL;

	WdfSpinLockAcquire(write_res->write_req_array_spinlock);

	if (write_res->NextAvailReqIndex != 0) {	/* Request is available */
		--(write_res->NextAvailReqIndex);
		*writeRequest =	write_res->write_req_array[write_res->NextAvailReqIndex];
		write_res->write_req_array[write_res->NextAvailReqIndex] = NULL;
	}

	WdfSpinLockRelease(write_res->write_req_array_spinlock);

	/* TODO : If we run out of Write requests then allocate on the fly */
}

static void
dbus_usbos_tx_req_return_wdf(usbos_info_t *usbinfo, WDFREQUEST Request)
{
	dbus_usbos_wdf_res_tx *write_res = &usbinfo->tx_q_lowprio;
	WDF_REQUEST_REUSE_PARAMS params;
	dbus_usbos_TCB_wdf	*writeContext;

	DBUSTRACE(("%s\n", __FUNCTION__));

	writeContext = _get_ctx_TCB(Request);

	NdisAcquireSpinLock(&usbinfo->tx_spinlock);
	RemoveEntryList(&writeContext->List);
	NdisReleaseSpinLock(&usbinfo->tx_spinlock);
	NdisInterlockedInsertTailList(&usbinfo->tx_free_list, &writeContext->List,
		&usbinfo->tx_spinlock);

	/* reset ->pData back to real data buffer in case it was changed in send_irb non-copy */
	writeContext->pData = writeContext->txDataBuf;

	WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
	WdfRequestReuse(Request, &params);

	WdfSpinLockAcquire(write_res->write_req_array_spinlock);

	if (write_res->NextAvailReqIndex >= write_res->write_req_max) {
		DBUSERR(("%s: Request list is already full!", __FUNCTION__));
		ASSERT(0);
	}

	write_res->write_req_array[write_res->NextAvailReqIndex++] = Request;

	WdfSpinLockRelease(write_res->write_req_array_spinlock);
}

static bool
dbus_usbos_tx_send_irb_async_wdf(usbos_info_t *usbinfo, WDFREQUEST writeRequest,
	dbus_usbos_TCB_wdf	*writeContext)
{
	NTSTATUS        ntStatus = NDIS_STATUS_SUCCESS;
	UCHAR QueueType = DBUS_USBOS_QUEUE_LOWPRIO;

	DBUSTRACE(("%s\n", __FUNCTION__));

	writeContext->QueueType = QueueType;
	writeContext->usbinfo = usbinfo;

	UsbBuildInterruptOrBulkTransferRequest(writeContext->Urb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		writeContext->UsbdPipeHandle,
		writeContext->pData,
		NULL,
		writeContext->ulSize,
		USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK,
		NULL);
	ntStatus = WdfUsbTargetPipeFormatRequestForUrb(writeContext->UsbPipe,
		writeRequest,
		writeContext->UrbMemory,
		NULL);

	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Failed to format request for urb\n", __FUNCTION__));
		goto error;
	}

	WdfRequestSetCompletionRoutine(writeRequest, dbus_usbos_tx_complete_cb, writeContext);

	if (!WdfRequestSend(writeRequest, writeContext->IoTarget, &usbinfo->WriteRequestOptions)) {
		DBUSERR(("%s: send failed\n", __FUNCTION__));
		ntStatus = WdfRequestGetStatus(writeRequest);
		goto error;
	}

	DBUSTRACE(("%s: done\n", __FUNCTION__));
	return TRUE;

error:
	ASSERT(!NT_SUCCESS(ntStatus));

	DBUSERR(("%s: call WdfRequestCompleteWithInformation\n"));
	WdfRequestCompleteWithInformation(writeRequest, ntStatus, 0);
	dbus_usbos_tx_req_return_wdf(usbinfo, writeRequest);
	return FALSE;
}

static bool
dbus_usbos_tx_send_buf_sync_wdf(usbos_info_t *usbinfo, ULONG Length, PVOID Buffer,
	dbus_irb_tx_t *txirb)
{
	WDFREQUEST      writeRequest;
	NTSTATUS        ntStatus = NDIS_STATUS_SUCCESS;
	dbus_usbos_TCB_wdf	*writeContext;
	UCHAR QueueType = DBUS_USBOS_QUEUE_LOWPRIO;
	KEVENT *e = &usbinfo->eve;
	WDF_REQUEST_SEND_OPTIONS option;
	LARGE_INTEGER timeout;

	DBUSTRACE(("%s\n", __FUNCTION__));

	dbus_usbos_tx_req_get_wdf(usbinfo, &writeRequest);
	if (writeRequest == NULL) {
		DBUSERR(("%s: no write resource left!!\n", __FUNCTION__));
		return FALSE;
	}

	writeContext = _get_ctx_TCB(writeRequest);
	writeContext->QueueType = QueueType;
	writeContext->usbinfo = usbinfo;
	writeContext->txirb = txirb;

	UsbBuildInterruptOrBulkTransferRequest(writeContext->Urb,
		sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
		writeContext->UsbdPipeHandle,
		Buffer,
		NULL,
		Length,
		USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK,
		NULL);
	ntStatus = WdfUsbTargetPipeFormatRequestForUrb(writeContext->UsbPipe,
		writeRequest,
		writeContext->UrbMemory,
		NULL);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Failed to format request for urb\n", __FUNCTION__));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		goto error;
	}

	WdfRequestSetCompletionRoutine(writeRequest, dbus_usbos_urb_complete_sync_cb_wdf, usbinfo);

	/* Allocate the request timer */
	ntStatus = WdfRequestAllocateTimer(writeRequest);
	if (!NT_SUCCESS(ntStatus)) {
		DBUSERR(("%s: Could not allocate timer for request, status(0x%08X)",
			__FUNCTION__, ntStatus));
		goto error;
	}

	WDF_REQUEST_SEND_OPTIONS_INIT(&option, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);

	KeInitializeEvent(e, NotificationEvent, FALSE);
	if (!WdfRequestSend(writeRequest, writeContext->IoTarget, &option)) {
		timeout.QuadPart = (LONGLONG)0;
		/* spin check only, no wait
		 * event signalled in dbus_usbos_urb_complete_sync_cb_wdf
		 */
		ntStatus = KeWaitForSingleObject(e, Executive, KernelMode, FALSE, &timeout);
		if (ntStatus == STATUS_SUCCESS) {
			ntStatus = STATUS_CANCELLED;
			DBUSERR(("%s: Timed out, auto-cancelled\n", __FUNCTION__));
		} else if (ntStatus == STATUS_TIMEOUT) {
			if (WdfRequestCancelSentRequest(writeRequest)) {
				ntStatus = STATUS_CANCELLED;
				DBUSERR(("%s: Timed out, driver cancelled", __FUNCTION__));
			} else {
				ntStatus = WdfRequestGetStatus(writeRequest);
				DBUSERR(("%s: Failed, status = 0x%x\n", __FUNCTION__, ntStatus));
			}
		}
	}

error:
	dbus_usbos_tx_req_return_wdf(usbinfo, writeRequest);
	DBUSTRACE(("%s: done\n", __FUNCTION__));

	return NT_SUCCESS(ntStatus);
}

#endif /* WDF_API */
