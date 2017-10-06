/*
 * Mac OS X specific portion of
 * Broadcom 802.11 Networking Device Driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Private1743:http://confluence.broadcom.com/display/WLAN/BuildIPValidation>>
 *
 * Copyright (c) 2002 Apple Computer, Inc.  All rights reserved.
 *
 * $Id$
 */
/* FILE-CSTYLED */

#include <libkern/libkern.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/proc.h>
#include <IOKit/IOTypes.h>
#include <IOKit/IOLib.h>
#include <IOKit/apple80211/apple80211_var.h>
#include <IOKit/apple80211/apple80211_ioctl.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/apple80211/IO80211Controller.h>
#include <IOKit/apple80211/IO80211Interface.h>
#include <IOKit/apple80211/IO80211WorkLoop.h>
#if defined(IO80211P2P)
#include <IOKit/apple80211/IO80211P2PInterface.h>
#include <IOKit/apple80211/apple80211_p2p.h>
#include <IOKit/apple80211/IO80211P2PUtils.h>
#include <IOKit/apple80211/IO80211VirtualInterface.h>
#endif 
#include <AssertMacros.h>
#include <IOKit/network/IOGatedOutputQueue.h>
#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/pci/IOPCIBridge.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#if VERSION_MAJOR > 9
#include <IOKit/apple80211/IO8Log.h>
#endif
#if VERSION_MAJOR >= 14
#include <IOKit/pwr_mgt/IOPMPrivate.h>
#endif
#if VERSION_MAJOR >= 15
#include <IOKit/apple80211/apple80211_return.h>
#endif

#include <typedefs.h>
#include <osl.h>
#include <bcmendian.h>

#include <AVSHeader.h>
#include <proto/ieee80211_radiotap.h>
#include <PPIHeader.h>
#ifdef MACOSX_DHD
#include <net/kpi_interface.h>
#endif /* MACOSX_DHD */
#ifndef kPCI2PCIBridgeControl
#define kPCI2PCIBridgeControl	0x3E
#endif


extern "C" {
	#include <wlioctl.h>
	#include <wlioctl_utils.h>
	#include <wlc_cfg.h>
	#include <bcmutils.h>
	#include <bcmdefs.h>
	#include <siutils.h>
	#include <sbconfig.h>
	#include <proto/802.11.h>
	#include <proto/802.1d.h>
	#include <bcmwpa.h>
	#include <epivers.h>
	#include <sbhndpio.h>
	#include <sbhnddma.h>
	#include <hnddma.h>
	#include <d11.h>
	#include <wlc_pub.h>
	#include <wlc_rate.h>
	#include <wlc_bsscfg.h>
	#include <wlc_scb.h>
	#include <wl_export.h>
	#include <wl_dbg.h>
	#include <bcmdevs.h>

	#include <pcie_core.h>
	#include <pcicfg.h>
#ifdef WLEXTLOG
	#include <wlc_extlog.h>
	#include <wlc_extlog_idstr.h>
#endif
	#include <wlc_assoc.h>
	#include <wlc_tpc.h>
#ifdef WLP2P
	#include <wlc_p2p.h>
#endif
#ifdef MACOSX_DHD
	#include <proto/eapol.h>
#else
	#include <wlc.h>
	#include <wlc_keymgmt.h>
	#include <wlc_channel.h>
	#include <wlc_pio.h>
	#include <wlc_scan_utils.h>
	#include <wlc_wowl.h>
	#include <wlc_stf.h>
	#include <wlc_ht.h>
#ifdef WL_LTR
	#include <wlc_ltr.h>
#endif /* WL_LTR */
	#include <wlc_11d.h>
#endif /* MACOSX_DHD */
	#include <bcmwifi_radiotap.h>
	#include <wlc_pcb.h>
#ifdef WLCSA
	#include <wlc_csa.h>
#endif
	#include <wlc_btcx.h>
	#include <wlc_rspec.h>
	#include <wlc_lq.h>
	#include <wlc_iocv.h>

#if defined(WL_EVENTQ) && defined(P2PO) && !defined(P2PO_DISABLED)
#include <wl_eventq.h>
#endif /* WL_EVENTQ && P2PO && P2PO_DISABLED */
#if (defined(P2PO) && !defined(P2PO_DISABLED)) || (defined(ANQPO) && \
	!defined(ANQPO_DISABLED))
#if (defined(P2PO) && !defined(P2PO_DISABLED))
#include <wl_gas.h>
#include <wl_p2po_disc.h>
#include <wl_p2po.h>
#endif
#if (defined(ANQPO) && !defined(ANQPO_DISABLED))
#include <wl_anqpo.h>
#endif
#endif	/* (P2PO && !P2PO_DISABLED) || (ANQPO && !ANQPO_DISABLED) */

#ifdef NEED_HARD_RESET
#include <wlc_hw.h>
#endif /* NEED_HARD_RESET */

#ifndef MACOSX_DHD
#include <wlc_dump.h>
#endif /* MACOSX_DHD */
}

#include <AvailabilityMacros.h>

#ifdef BONJOUR
#define BONJOUR_DEBUG
#undef BONJOUR_TRACE

#include <sys/mbuf.h>
#include <netinet/in.h>
#endif //BONJOUR

#include <proto/bcmipv6.h>

#if (VERSION_MAJOR > 11)
#include <sys/vnode.h>
#include <sys/file.h>
#endif
#ifndef MACOSX_DHD
#if VERSION_MAJOR > 11
	#define MACOS_AQM
#endif
#if VERSION_MAJOR > 9
	#define SUPPORT_FEATURE_WOW
	#define MAX_RSSI	100
#endif
#else
#if VERSION_MAJOR > 13
	#define MAX_RSSI	100
#endif /* VERSION_MAJOR > 13 */
#endif /* !MACOSX_DHD */

// Kernel debugging trace support
#if !defined(KERNEL_DEBUG_CONSTANT)
#define KERNEL_DEBUG_CONSTANT(x,a,b,c,d,e) do { } while(0)
#endif

#if !defined(KERNEL_DEBUG_CONSTANT1)
#define KERNEL_DEBUG_CONSTANT1(x,a,b,c,d,e) do { } while(0)
#endif


#ifdef WLTCPKEEPA
/* The size of the "wowl_pkt_info" iovar output data for tcpkeepalive: the output
 * data contains a wake_info_t header, a wake_pkt_t header and  ETHER_MAX_DATA.
 */
#define TCPKEEP_PKT_INFO_MAX_SZ (sizeof(wake_info_t) + sizeof(wake_pkt_t) + ETHER_MAX_DATA)

#define KeepaliveAddresstype(r) ((r)->u.ka4.addr_type)
#define KeepaliveSize(r) ((KeepaliveAddresstype(r) == Keepalive_addr_type_ipv4) ?  sizeof(struct keepalive_ipv4) : 0)
#endif /* WLTCPKEEPA */

#include "wl_macosx.h"

#define BCOPYADRS(from, to)		bcopy( (from), (to), ETHER_ADDR_LEN )
#define BCMPADRS(from, to)		bcmp( (from), (to), ETHER_ADDR_LEN )
/*
 * Takes a pointer, returns true if a 48-bit null address (all zeros)
 */
#ifndef ETHER_ISNULLADDR
#define ETHER_ISNULLADDR(ea) ((((uint8 *)(ea))[0] |		\
			    ((uint8 *)(ea))[1] |		\
			    ((uint8 *)(ea))[2] |		\
			    ((uint8 *)(ea))[3] |		\
			    ((uint8 *)(ea))[4] |		\
			    ((uint8 *)(ea))[5]) == 0)
#endif

#ifdef IO80211P2P
#ifndef APSTA
#error "APSTA needs to be defined"
#endif
#ifndef ETHER_SET_LOCALADDR
#define ETHER_SET_LOCALADDR(ea)	(((uint8 *)(ea))[0] = (((uint8 *)(ea))[0] | 2))
#endif
#ifndef ETHER_ISMULTI
#define ETHER_ISMULTI(ea) (((const uint8 *)(ea))[0] & 1)
#endif
#ifndef ETHER_IS_LOCALADDR
#define ETHER_IS_LOCALADDR(ea) 	(((uint8 *)(ea))[0] & 2)
#endif
#endif /* IO80211P2P */

/* Driver version */
#ifdef BCMDBG
	#define DBG_VER " - DEBUG"
#else
	#define DBG_VER
#endif
#ifdef FACTORY_BUILD
	#define FACTORY_VER " FACTORY"
#else
	#define FACTORY_VER
#endif
#ifdef WLTEST
	#define MFG_VER " MFG"
#else
	#define MFG_VER
#endif

#if defined(BCMBUILD)
// Leave blank for no Apple changes
// Use .1, .2, etc. for each Apple-specific change
#define APPLE_VERSION_STR	".3"

#define quote_string(x) #x
#define VersCat(maj, min, rc, inc) quote_string(maj) "." quote_string(min) "." quote_string(rc) "." quote_string(inc)
#define kVersionString	\
	"Broadcom BCM43xx 1.0 (" \
	VersCat(EPI_MAJOR_VERSION, EPI_MINOR_VERSION, EPI_RC_NUMBER, EPI_INCREMENTAL_NUMBER)\
	APPLE_VERSION_STR ")" DBG_VER FACTORY_VER MFG_VER
#else /* BCMBUILD */
#define quote_string(x) #x
#define VersCat(maj, min, rc, inc) quote_string(maj) "." quote_string(min) "." quote_string(rc) "." quote_string(inc)
#define kVersionString	\
	"Broadcom BCM43xx 1.0 (" \
	VersCat(EPI_MAJOR_VERSION, EPI_MINOR_VERSION, EPI_RC_NUMBER, EPI_INCREMENTAL_NUMBER)\
	STRINGIFY(kDriverAppleVersion) ")" DBG_VER FACTORY_VER MFG_VER
#endif /* BCMBUILD */

#ifdef BCMDBG
#define DPRINT(ARGS...)	printf( ARGS )
#else
#define DPRINT(ARGS...)
#endif

#define WL_SUPPORTED_IOVERSION 1U	// code supports version 1 of apple80211 interface
#define kIOFirmwareVersion	"IOFirmwareVersion"	// firmware version string property
#define kAirPortFeatures	"APFeatures"		// bit fields defining features
#define kAirPortChipRev		"APChipRev"		// chip version
#define APRoamTrigger		"APRoamTrigger"	// Lo-Power roam threshold rssi
#define kAirPortTAPD		"APTAPD"			// enabled (1) or disabled (0)

/*Apple specific chnage. Changing roam delta to 10 db from 20 db for smoother roaming*/
#define AGG_ROAM_DELTA 10

enum {
	kTxAntAuto = -1,		// -1 - auto track rx diversity
	kTxAntMain,			//  0 - Main only
	kTxAntAux,			//  1 - Aux only

	kRxAntMain = 0,			//  0 - Main only
	kRxAntAux,			//  1 - Aux only
	kRxAntAutoDiversity = 3
};

enum	// defines feature bits used by kAirPortFeatures property
{
	kAPFeatureWpaTKIP = 1,
	kAPFeatureWpaAES = 2
};

// sizes of various RSN keys/fields
#define kNonceSize		32
#define kPMKSize		32
#define kKeyIVSize		16
#define kMICSize		16
#define kMICKeySize		16
#define kEKKeySize		16		// size of EAPOL-KEY encryption key
#define kTkipPTKSize		64		// size in bytes of PTK for TKIP
#define kTkipGTKSize		32		// size in bytes of GTK for TKIP

#define kTxAntennaProperty	"antenna-tx"
#define kRxAntennaProperty	"antenna-rx-diversity"

#ifdef MACOS_AQM

#define AQM_TUNABLES_DICTIONARY_TUNABLES							"tunables"
#define AQM_TUNABLES_KEY_PULLMODE									"pullmode"
#define AQM_TUNABLES_KEY_TXRINGSIZE									"txringsize"
#define AQM_TUNABLES_KEY_TXSENDQSIZE								"txsendqsize"
#define AQM_TUNABLES_KEY_REAPCOUNT									"reapcount"
#define AQM_TUNABLES_KEY_REAPMIN									"reapmin"


#define AQM_TUNABLES_DEFAULT_TXRINNGSIZE							256
#define AQM_TUNABLES_MIN_TXRINGSIZE									16
#define AQM_TUNABLES_MAX_TXRINGSIZE									256

#define AQM_TUNABLES_DEFAULT_TXSENDQSIZE							1024
#define AQM_TUNABLES_MIN_TXSENDQSIZE								16
#define AQM_TUNABLES_MAX_TXSENDQSIZE								8192

#define AQM_TUNABLES_DEFAULT_REAPCOUNT								128
#define AQM_TUNABLES_MIN_REAPCOUNT									1
#define AQM_TUNABLES_MAX_REAPCOUNT									128

#define AQM_TUNABLES_DEFAULT_REAPMIN								32
#define AQM_TUNABLES_MIN_REAPMIN									1
#define AQM_TUNABLES_MAX_REAPMIN									32

// Tunables: highWaterMark, AC VO
#define AQM_TUNABLES_DEFAULT_HIGHWATERMARK_AC_VO					512
#define AQM_TUNABLES_MIN_HIGHWATERMARK_AC_VO						16
#define AQM_TUNABLES_MAX_HIGHWATERMARK_AC_VO						768

// Tunables: highWaterMark, AC VI
#define AQM_TUNABLES_DEFAULT_HIGHWATERMARK_AC_VI					512
#define AQM_TUNABLES_MIN_HIGHWATERMARK_AC_VI						16
#define AQM_TUNABLES_MAX_HIGHWATERMARK_AC_VI						768

// Tunables: highWaterMark, AC BE
#define AQM_TUNABLES_DEFAULT_HIGHWATERMARK_AC_BE					512
#define AQM_TUNABLES_MIN_HIGHWATERMARK_AC_BE						16
#define AQM_TUNABLES_MAX_HIGHWATERMARK_AC_BE						768

// Tunables: highWaterMark, AC BK
#define AQM_TUNABLES_DEFAULT_HIGHWATERMARK_AC_BK					512
#define AQM_TUNABLES_MIN_HIGHWATERMARK_AC_BK						16
#define AQM_TUNABLES_MAX_HIGHWATERMARK_AC_BK						768

// Helpers, bound max/min
#define BOUNDMAX( _a, _max )									( ((_a) > (_max)) ? (_max) : (_a) )
#define BOUNDMIN( _a, _min )									( ((_a) < (_min)) ? (_min) : (_a) )

#endif /* MACOS_AQM */

#ifdef NEED_HARD_RESET
/* Broadcom to Apple error code conversion table */
uint32 BcmAppleRetCodeMap[] = {
	kAppleBCMWLANCoreReturnUnknown,		/* WL_REINIT_RC_NONE		*/
	kAppleBCMWLANCoreReturnPSSync,		/* WL_REINIT_RC_PS_SYNC		*/
	kAppleBCMWLANCoreReturnPsmWD,		/* WL_REINIT_RC_PSM_WD		*/
	kAppleBCMWLANCoreReturnMacWake,		/* WL_REINIT_RC_MAC_WAKE	*/
	kAppleBCMWLANCoreReturnMacSuspend,	/* WL_REINIT_RC_MAC_SUSPEND	*/
	kAppleBCMWLANCoreReturnSpinWait,	/* WL_REINIT_RC_MAC_SPIN_WAIT	*/
	kAppleBCMWLANCoreReturnBusError,	/* WL_REINIT_RC_AXI_BUS_ERROR   */
	kAppleBCMWLANCoreReturnDeviceRemoved,	/* WL_REINIT_RC_DEVICE_REMOVED	*/
	kAppleBCMWLANCoreReturnPcieAER,		/* WL_REINIT_RC_PCIE_FATAL_ERROR*/
	kAppleBCMWLANCoreReturnFwTrap,		/* WL_REINIT_RC_OL_FW_TRAP	*/
	kAppleBCMWLANCoreReturnFifoErr,		/* WL_REINIT_RC_FIFO_ERR	*/
	kAppleBCMWLANCoreReturnTxStatus,	/* WL_REINIT_RC_INV_TX_STATUS	*/
	kAppleBCMWLANCoreReturnMqError,		/* WL_REINIT_RC_MQ_ERROR	*/
	kAppleBCMWLANCoreReturnThyTxErrThresh,	/* WL_REINIT_RC_PHYTXERR_THRESH	*/
	kAppleBCMWLANCoreReturnUserForced,	/* WL_REINIT_RC_USER_FORCED	*/
	kAppleBCMWLANCoreReturnFullReset,	/* WL_REINIT_RC_FULL_RESET	*/
	kAppleBCMWLANCoreReturnApBeacon,	/* WL_REINIT_RC_AP_BEACON	*/
	kAppleBCMWLANCoreReturnPMExcessWake,	/* WL_REINIT_RC_PM_EXCESSED	*/
	kAppleBCMWLANCoreReturnNoClock,		/* WL_REINIT_RC_NO_CLK		*/
	kAppleBCMWLANCoreReturnSwAssert,	/* WL_REINIT_RC_SW_ASSERT	*/
	kAppleBCMWLANCoreReturnScanTimeout, /* WL_REINIT_RC_SCAN_TIMEOUT */
	kAppleBCMWLANCoreReturnJoinTimeout,  /* WL_REINIT_RC_JOIN_TIMEOUT */
	};

static void _powerCycleOffOnThread( thread_call_param_t param0, thread_call_param_t param1 );

#endif /* NEED_HARD_RESET */

// station ie list structure
typedef struct sta_ie_list {
	SLIST_ENTRY(sta_ie_list) entries;
	struct ether_addr addr;
	bcm_tlv_t *tlv;
	UInt32 tlvlen;
} sta_ie_list_t;

// /Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/Kernel.framework/Versions/A/Headers/sys/kpi_mbuf.h

#define OVQ_ENQUEUE_MBUF(ovq, m) { \
        mbuf_setnextpkt((m), 0); \
        if ((ovq)->ovq_tail == 0) \
                (ovq)->ovq_head = (m); \
        else \
                mbuf_setnextpkt((mbuf_t)(ovq)->ovq_tail, (m)); \
        (ovq)->ovq_tail = (m); \
        (ovq)->ovq_len++; \
}

#define OVQ_PREPEND_MBUF(ovq, m) { \
        mbuf_setnextpkt((m), (ovq)->ovq_head); \
        if ((ovq)->ovq_tail == 0) \
                (ovq)->ovq_tail = (m); \
        (ovq)->ovq_head = (m); \
        (ovq)->ovq_len++; \
}
#define OVQ_DEQUEUE_MBUF(ovq, m) { \
        (m) = (ovq)->ovq_head; \
        if (m) { \
                if (((ovq)->ovq_head = mbuf_nextpkt((m))) == 0) \
                        (ovq)->ovq_tail = 0; \
                mbuf_setnextpkt((m), 0); \
                (ovq)->ovq_len--; \
        } \
}
static void IOPCILogDevice(IOPCIDevice * device, const char *log);

static void *g_brcm_controller = NULL;

#ifdef APPLE80211_POSTMESSAGE_TLV
static wl_bss_info_t *find_bssinfo_from_edata(uchar *edata, uint32 len);
#endif

static void wl_recover_nocard(struct wl_info *wl);

int NoCard = FALSE;
//===========================================================================
// IOPMPowerState definitions
//===========================================================================

// Implement 3 power states:
// 0 - Off
//
// 1 - Fast PowerSave (wl PM 2)
//	Use Idle Associated PM 2 power measurement
// 2 - On
//	Use Idle Associated PM 0 power measurement
//
enum {
	PS_INDEX_OFF = 0,
	PS_INDEX_DOZE = 1,
	PS_INDEX_ON = 2,
	PS_NUM_STATES = 3
};

// IOPMPowerState.capabilityFlags values for the 3 states
#define PS_CAP_OFF		0
#define PS_CAP_DOZE		kIOPMDoze
#define PS_CAP_ON		(IOPMDeviceUsable | IOPMContextRetained | IOPMConfigRetained | IOPMMaxPerformance)

// Mapping from Apple WMM AC to priority
static const int ac2prio[] = {
		PRIO_8021D_BE, // APPLE80211_WME_AC_BE	0
		PRIO_8021D_BK, // APPLE80211_WME_AC_BK	1
		PRIO_8021D_VI, // APPLE80211_WME_AC_VI	2
		PRIO_8021D_VO, // APPLE80211_WME_AC_VO	3
};

static const IOPMPowerState power_state_template[PS_NUM_STATES] = {
	// OFF
	{	kIOPMPowerStateVersion1,
		PS_CAP_OFF,		// capabilityFlags
		0,
		0,			// inputPowerRequirement
		0, 0, 0,
		0,			// timeToAttain
		0,
		5000,			// timeToLower -- 5 ms from wl up to down
		0, 0},
	// Doze
	{	kIOPMPowerStateVersion1,
		PS_CAP_DOZE,	// capabilityFlags
		kIOPMDoze,
		kIOPMDoze,		// inputPowerRequirement
		0, 0, 0,
		0,			// timeToAttain -- 20 ms from wl down to up
		0,
		5000,			// timeToLower
		0, 0},
	// ON
	{	kIOPMPowerStateVersion1,
		PS_CAP_ON,		// capabilityFlags
		0,
		kIOPMPowerOn,		// inputPowerRequirement
		0, 0, 0,
		0,			// timeToAttain
		0,
		0,			// timeToLower
		0, 0}
};

static const chip_power_levels_t chip_power_table[] = {
	{0x4321, 0, 140, 360, 1670},	// Any 4321 rev
	{0x4311, 0, 145, 320, 695},	// Any 4311 rev
	{0x0000, 0, 100, 300, 500}	// All others
};

#ifndef DLT_IEEE802_11_RADIO
#define	DLT_IEEE802_11_RADIO	127	/* 802.11 plus WLAN header */
#endif

struct wl_spatial_mode_data {
	int mode[5];		/* spatial mode for 2.4G 5G (low, mid, high, upper) */
};

#define ROAM_RSSITHRASHDIFF_MACOS	8	/* Roam RSSI thresh delta for MacOSX Setting */

//===========================================================================
// Brcm43xx_InputQueue class
//===========================================================================
class Brcm43xx_InputQueue : private IOPacketQueue
{
	OSDeclareDefaultStructors( Brcm43xx_InputQueue )

public:
	// Typedef Action method to be called on packets as they are dequeued
	typedef void	(*Action)(IONetworkController *target, mbuf_t m);

	// instance initialization
	bool		init(IONetworkController *inTarget, Action inAction);

	// factory creation and init function
	static Brcm43xx_InputQueue *
			withTargetAndAction(IONetworkController *inTarget, Action inAction);

	// release/free method
	virtual void	release() const;

	// enable the queue, allowing packets to be queued and processing
	// the packets with the Action method
	void		enable();

	// disable the queue, flushing all held packets and preventing
	// additional packets from being enqueued
	UInt32		disable();

	// dequeue and free all packets on the queue without calling
	// the Action method
	virtual UInt32	flush();

	// enqueue an mbuf or mbuf chain, up to the queue capacity
	virtual UInt32	enqueueWithDrop(mbuf_t m);

	// set or clear the hold state
	void		serviceHold(bool hold);

	// process all packets in the queue, dequeing and calling
	// the Action method
	void		service();

protected:
	void		free();
	void		scheduleService();
	static void	serviceInterrupt(OSObject * owner,
	                                 IOInterruptEventSource * src,
	                                 int count);

	Action	action;		// action to be performed on each mbuf
	UInt32	capacity;	// capacity of queue when enabled
	bool	hold;		// prevent/allow service scheduling
	IONetworkController *	target;
	IOInterruptEventSource * interruptSrc;
};

#define RETRY_SHORT_AC_VO	15  /* VO class short retry limit */
#define OV_NUM_PRIOS 	4  			//Num queues supported by MacOS

#ifdef IO80211P2P
#if !defined(WLP2P) && !defined(WLTEST)
#error "WLP2P needs to be defined for IO80211P2P"
#endif

//===========================================================================
// AirPort_Brcm43xxP2PInterface class
//===========================================================================
class AirPort_Brcm43xxP2PInterface;

struct virt_intf_entry {
	SLIST_ENTRY(virt_intf_entry) entries;
	AirPort_Brcm43xxP2PInterface *obj;
};

// Store wl_if information in the class inherited from super class to make it
// easier to look up.
class AirPort_Brcm43xxP2PInterface : public IO80211P2PInterface
{
	OSDeclareDefaultStructors(AirPort_Brcm43xxP2PInterface)

public:

	virtual bool init( IO80211Controller * controller,
					   struct ether_addr * addr,
					   UInt32 role,
					   const char * prefix );

	struct wlc_if *wlcif;		/* wlc interface handle */
	uint	unit;
	char	ifname[IFNAMSIZ];
	bool listen_complete;

	intf_info_blk_t intf_info; // Information about this interface

	txflc_t intf_txflowcontrol;		// contains all flow control flags and queues

	// Linked List
	struct virt_intf_entry list_elt;

#if VERSION_MAJOR > 10
	bool opp_ps_enable;
	UInt32 ct_window;
#endif // VERSION_MAJOR > 10
};
#endif /* IO80211P2P */

//===========================================================================
// AirPort_Brcm43xxInterface class
//===========================================================================

class AirPort_Brcm43xxInterface : public IO80211Interface
{
	OSDeclareDefaultStructors(AirPort_Brcm43xxInterface)

public:
	virtual bool	init(IONetworkController *controller );
#if VERSION_MAJOR > 9
	virtual bool	setLinkState( IO80211LinkState linkState , UInt32 reason = 1);
#else
	virtual bool	setLinkState( IO80211LinkState linkState);
#endif
	// Apple change: <rdar://problem/6394678>
	virtual SInt32	performCommand( IONetworkController	* controller,
	                                unsigned long cmd,
	                                void * arg0,
	                                void * arg1 );
	intf_info_blk_t intf_info;

	bool	suppressLink;
	// Apple change: <rdar://problem/6394678>
	bool	SIOCSIFFLAGS_InProgress;

#ifdef MACOS_AQM
	int getTxPacketsPending( int ac, int *available = NULL );
#endif /* MACOS_AQM */
};

class AirPort_Brcm43xx;
//===========================================================================
// AirPort_Brcm43xxTimer class
//===========================================================================
class AirPort_Brcm43xxTimer : public OSObject
{
	OSDeclareDefaultStructors( AirPort_Brcm43xxTimer )

protected:
	void (*fn)(void *);
	void *fn_arg;
	uint ms;
	bool set;
	bool periodic;

public:
	// instance initialization
	bool init();
	static AirPort_Brcm43xxTimer *initWithParams(AirPort_Brcm43xx *wl, void (*fn) (void*), void *fn_arg,
	                                             const char *name);
	void initTimer(AirPort_Brcm43xx *wl, void (*fn) (void*), void *fn_arg,
	               const char *name);
	void addTimer(uint ms, bool periodic);
	bool delTimer();
	void timerCallback();
	void restartTimer();
	void freeTimerName();

	IOTimerEventSource *iotimer;
	AirPort_Brcm43xxTimer *next;
	AirPort_Brcm43xx *wl;

#if defined(BCMDBG) || defined(WL_TIMERSTATS)
	char *name;
#endif
#ifdef WL_TIMERSTATS
	uint64 scheduled_count;
	uint64 canceled_count;
	uint64 fired_count;
#endif /* WL_TIMERSTATS */
};

//===========================================================================
// AirPort_Brcm43xx class
//===========================================================================

#ifdef BONJOUR
enum {
    kDNSServiceType_A         = 1,      /* Host address. */
    kDNSServiceType_CNAME     = 5,      /* Canonical name. */
    kDNSServiceType_AAAA      = 28,     /* IPv6 Address. */
    kDNSServiceType_NULL      = 10,     /* Null resource record (used for keepalive) */
    kDNSServiceType_PTR       = 12,     /* Domain name pointer. */
    kDNSServiceType_TXT       = 16,     /* One or more text strings (NOT "zero or more..."). */
    kDNSServiceType_SRV       = 33      /* Server Selection. */
};
#endif //BONJOUR

#define kBoardId						"board-id"

/* WL_LOCK() will use inGate() to verify if the caller already has the lock.
 * It would panic() if caller does not have the lock.
 * Additional debug code is protected in WL_LOCK_CHECK, USE_SPIN_LOCK and LOCK_DEBUG
 * which can enabled for debug purpose only
 */
int AirPort_Brcm43xx::WL_LOCK(void)
{
	int ret = BCME_OK;

#ifdef WL_LOCK_CHECK

#ifdef USE_SPIN_LOCK
	lck_spin_lock(wl_spin_lock);
#endif /* USE_SPIN_LOCK */

	ASSERT(wl_lock_count >= 0);

	if ((wl_lock_count) && (wl_lock_thread_id != (void*)IOThreadSelf())) {

		WL_ERROR(("LOCK_FAILED: PrevLockOwner:%p, CurrLockOwner:%p, wl_lock_count:%d\n",
			(void *)OSL_OBFUSCATE_BUF(wl_lock_thread_id),
			(void*)OSL_OBFUSCATE_BUF(IOThreadSelf()),
			wl_lock_count));

#if defined(LOCK_DEBUG)
		wl_print_backtrace("LOCK FAILED", NULL, 0);
		wl_print_backtrace("PREV_LOCK_BY",
			(void*)&prev_lock_backtrace[0], prev_lock_backtrace_depth);
#endif /* LOCK_DEBUG */

		ret = BCME_ERROR;
	} else {
		wl_lock_thread_id = IOThreadSelf();
		wl_lock_count++;
	}

#if defined(LOCK_DEBUG)
	prev_lock_backtrace_depth = OSBacktrace(&prev_lock_backtrace[0], MAX_BACKTRACE_DEPTH);
#endif /* LOCK_DEBUG */

#ifdef USE_SPIN_LOCK
	lck_spin_unlock(wl_spin_lock);
#endif /* USE_SPIN_LOCK */

#endif /* WL_LOCK_CHECK */

#if VERSION_MAJOR >= 15
	if(!getWorkLoop()->inGate()) {
		osl_panic("!inGate()");
	} else
#endif /* VERSION_MAJOR >= 15 */

	if (ret) {
		ASSERT(!"WL_LOCK");
	}

	return ret;
}

/* WL_UNLOCK() will use inGate() to verify if the caller still/already has the lock.
 * It would panic() if caller does not have the lock.
 * Additional debug code is protected in WL_LOCK_CHECK, USE_SPIN_LOCK and LOCK_DEBUG
 * which can enabled for debug purpose only
 */
int AirPort_Brcm43xx::WL_UNLOCK(void)
{
	int ret = BCME_OK;

#ifdef WL_LOCK_CHECK

#ifdef USE_SPIN_LOCK
	lck_spin_lock(wl_spin_lock);
#endif /* USE_SPIN_LOCK */

	if (wl_lock_count && (wl_lock_thread_id == IOThreadSelf())) {
		wl_lock_count--;

		if (!wl_lock_count) {
			wl_lock_thread_id = NULL;
		}
	}  else {
		WL_ERROR(("UNLOCK_FAILED: PrevLockOwner:%p, CurrUnlockBy:%p, LockCount:%d\n",
			wl_lock_thread_id, IOThreadSelf(), wl_lock_count));

#if defined(LOCK_DEBUG)
		wl_print_backtrace("UNLOCK FAILED", NULL, 0);
		wl_print_backtrace("UNLOCK FAILED - PREV LOCK AT",
			(void*)&prev_lock_backtrace[0], prev_lock_backtrace_depth);
		wl_print_backtrace("UNLOCK FAILED - PREV UNLOCK AT",
			(void*)&prev_unlock_backtrace, prev_unlock_backtrace_depth);
#endif /* LOCK_DEBUG */
		ret = BCME_ERROR;
	}

	ASSERT(wl_lock_count >= 0);

#if defined(LOCK_DEBUG)
	prev_unlock_backtrace_depth = OSBacktrace(&prev_unlock_backtrace[0], MAX_BACKTRACE_DEPTH);
#endif /* LOCK_DEBUG */

#ifdef USE_SPIN_LOCK
	lck_spin_unlock(wl_spin_lock);
#endif /* USE_SPIN_LOCK */

#endif /* WL_LOCK_CHECK */


#if VERSION_MAJOR >= 15
	if(!getWorkLoop()->inGate()) {
		osl_panic("!inGate()");
	} else
#endif /* VERSION_MAJOR >= 15 */

	if (ret) {
		ASSERT(!"WL_UNLOCK");
	}

	return ret;
}

#ifdef RDR5858823
#define APPLE80211_NO_NETWORKS 0x10000
#endif

// local prototypes

extern "C" {
static void wl_timer_callback(OSObject *owner, IOTimerEventSource *sender);
}

extern uint wl_msg_level;
extern uint wl_msg_level2;

// Check for IOCTL structure version values
SInt32
version_check(u_int32_t version, const char* ioname)
{
	SInt32 err = BCME_OK;

#ifndef BCMDBG
	BCM_REFERENCE(ioname);
#endif

	if (version == 0) {
		WL_ERROR(("WARNING: ioctl \"%s\" version 0 does not match supported version %u\n",
			ioname, WL_SUPPORTED_IOVERSION));
	} else if (version > WL_SUPPORTED_IOVERSION) {
		WL_ERROR(("ioctl \"%s\" version %u greater than supported version %u\n",
			ioname, version, WL_SUPPORTED_IOVERSION));
		err = EINVAL;
	}

	return err;
}

// Convert apple80211 rate in Mbps to 500Kbps 802.11 rate
static uint8
apple2mac_rate(uint32 apple_rate)
{
	// convert Mbps to 500 Kbps units
	uint8 mac_rate = (uint8)(apple_rate * 2);

	// 5.5 MBps is trucated to 5 MBps, so fixup
	if (mac_rate == 10)
		mac_rate = WLC_RATE_5M5;

	return mac_rate;
}

// Apple change: Some platforms use FAST powersave at all times
// We could also use this for future platforms if we
// decide on an IORegistry property to request powersave at all times
static bool alwaysWantPS()
{
	bool	result = true;
	char	model[32] = {0};


#ifdef FACTORY_BUILD
	return false;
#endif

	// Turn this function around and list the model names
	// that do NOT want PM 2 at all times
	// rdar://6904388 has a list of currently (as of SL GM) shipping model names
	// The assumption here is that going forward, all systems will want PM 2 always

	if( PEGetModelName( model, sizeof( model ) ) )
	{
		if( ( strncmp( model, "iMac",			 4 ) == 0 ) ||	// any iMac
		    ( strncmp( model, "MacPro",			 6 ) == 0 ) ||	// any Mac Pro tower
		    ( strncmp( model, "MacBook3",		 8 ) == 0 ) ||	// K36
		    ( strncmp( model, "MacBook4",		 8 ) == 0 ) ||	// K36A
		    ( strncmp( model, "MacBookPro4,",	11 ) == 0 )		// M87, M88
		   )
			result = false;
	}
	return result;
}

#pragma mark -
#pragma mark AirPort_Brcm43xxTimer Implementation

//=============================================================================
// AirPort_Brcm43xxTimer
//=============================================================================
#define super OSObject
OSDefineMetaClassAndStructors( AirPort_Brcm43xxTimer, OSObject )

bool
AirPort_Brcm43xxTimer::init()
{
	return super::init();
}

AirPort_Brcm43xxTimer*
AirPort_Brcm43xxTimer::initWithParams(AirPort_Brcm43xx *wl, void (*fn) (void*), void *fn_arg,
                                      const char *name)
{
	AirPort_Brcm43xxTimer *t = new AirPort_Brcm43xxTimer;
	if (!t || !t->init())
		return NULL;

	t->initTimer(wl, fn, fn_arg, name);
	return t;
}

void
AirPort_Brcm43xxTimer::initTimer(AirPort_Brcm43xx *wl, void (*fn) (void*), void *fn_arg,
                                 const char *name)
{
	this->wl = wl;
	this->fn = fn;
	this->fn_arg = fn_arg;

#if defined(BCMDBG) || defined(WL_TIMERSTATS)
	if ((this->name = (char *)MALLOC(wl->osh, strlen(name) + 1)))
		strncpy(this->name, name, strlen(name) + 1);
#else
	BCM_REFERENCE(name);
#endif
#ifdef WL_TIMERSTATS
	this->scheduled_count = 0;
	this->canceled_count = 0;
	this->fired_count = 0;
#endif /* WL_TIMERSTATS */
}

void
AirPort_Brcm43xxTimer::addTimer(uint dur, bool per)
{
	 ASSERT(!set);
#ifdef BCMDBG
	if (set)
		WL_ERROR(("timer %s assert!\n", this->name));
#endif
	ms = dur;
	periodic = per;
	set = TRUE;
	iotimer->setTimeoutMS(ms);
	WLINC_TIMERCNT(scheduled_count);
}

bool
AirPort_Brcm43xxTimer::delTimer()
{
	if (set) {
		set = FALSE;
		iotimer->cancelTimeout();
		WLINC_TIMERCNT(canceled_count);
	}
	return TRUE;
}

void
AirPort_Brcm43xxTimer::timerCallback()
{
	if (!set)
		return;
	set = FALSE;
	WLINC_TIMERCNT(fired_count);
	if (periodic) {
		set = TRUE;
		iotimer->setTimeoutMS(ms);
		WLINC_TIMERCNT(scheduled_count);
	}

	fn(fn_arg);
}

void
AirPort_Brcm43xxTimer::restartTimer()
{
	delTimer();
	if ((ms == 0) && !periodic) {
		ms += 10;
	}
	addTimer(ms, periodic);
}

void
AirPort_Brcm43xxTimer::freeTimerName()
{
#if defined(BCMDBG) || defined(WL_TIMERSTATS)
	if (name) {
		MFREE(wl->osh, name, strlen(name) + 1);
		name = NULL;
	}
#endif
}

#pragma mark -
#pragma mark AirPort_Brcm43xxInterface Implementation

//=============================================================================
// AirPort_Brcm43xxInterface
//=============================================================================
#undef  super
#define super IO80211Interface
OSDefineMetaClassAndStructors( AirPort_Brcm43xxInterface, IO80211Interface )

bool
AirPort_Brcm43xxInterface::init(IONetworkController *controller)
{
	suppressLink = false;

	return super::init(controller);
}

#if VERSION_MAJOR > 9
bool
AirPort_Brcm43xxInterface::setLinkState( IO80211LinkState linkState , UInt32 reason )
#else
bool
AirPort_Brcm43xxInterface::setLinkState( IO80211LinkState linkState )
#endif
{
	bool ret = FALSE;

	WL_PORT(("wl: setLinkState: %s\n", lookup_name(IO80211LinkState_names, linkState)));

	if (!suppressLink) {
#if VERSION_MAJOR > 9
		ret = super::setLinkState(linkState, reason);
#else
		ret = super::setLinkState(linkState);
#endif
	}
	else
		ret = true;

#ifdef MACOS_AQM
	AirPort_Brcm43xx *controller = OSDynamicCast( AirPort_Brcm43xx, getController() );
	if(	controller
		&& OSDynamicCast( IO80211Interface, this )
		&& OSDynamicCast( IO80211Interface, this )->aqmSupported() )
	{
		if( linkState == kIO80211NetworkLinkDown )
		{
			WL_IOCTL(("wl%d: AQM: ::setlinkState DOWN: stopOutputThread & flushOutputQueue\n",
							controller->unit));

			stopOutputThread();
			flushOutputQueue();
		}
		else
		if( linkState == kIO80211NetworkLinkUp )
		{
			WL_IOCTL(("wl%d: AQM: ::setlinkState   UP: startOutputThread\n",
							controller->unit));

			startOutputThread();
		}
	}
#endif /* MACOS_AQM */
	return ret;
}
// Apple change: <rdar://problem/6394678>
SInt32
AirPort_Brcm43xxInterface::performCommand( IONetworkController	* controller,
                                           unsigned long cmd,
                                           void * arg0,
                                           void * arg1 )
{
	if( cmd == SIOCSIFFLAGS )
		SIOCSIFFLAGS_InProgress = true;

	SInt32 ret = super::performCommand( controller, cmd, arg0, arg1 );

	if( cmd == SIOCSIFFLAGS )
		SIOCSIFFLAGS_InProgress = false;

	return ret;
}


#ifdef MACOS_AQM

#define TXPACKETSPENDING_DEFAULT_PARANOID_MAXVALUE			1024

int
AirPort_Brcm43xxInterface::getTxPacketsPending( int ac, int *available )
{
	int pending = 0, navail = -1;
	AirPort_Brcm43xx *controller = (AirPort_Brcm43xx *)getController();
	struct wlc_info	 *pWLC = controller->pWLC;
	int max = 0;

	pending = wlc_get_prec_txpending( pWLC, ac2prio[ac] );
	max     = wlc_get_pmax_tid( pWLC, ac2prio[ac] );

	if( max <= 0 ) {
		// If max <= 0, max then really needs to be adjusted then.
		// In this situation by adjusting max and reflecting an inaccurate maximum count,
		//   what will probably happen is that under high stress load the driver will drop packets
		//   since (max - pending) will not accurately reflect the proper back pressure to the
		//   network stack thus dropping packets, ideally the outputStart/processServiceClassQueue
		//   should call an API to check if the AC is flow controlled.

		max = TXPACKETSPENDING_DEFAULT_PARANOID_MAXVALUE;
	}

	navail  = max - pending;
	navail  = (navail > 0) ? navail : 0;

	//navail  = -1;  // -1: Unknown available     0: None available,   > 0: N packets available

	if( available ) *available = navail;

	return pending;
}
#endif /* MACOS_AQM */


#ifdef IO80211P2P
#pragma mark -
#pragma mark AirPort_Brcm43xxP2PInterface Implementation

//=============================================================================
// AirPort_Brcm43xxP2PInterface
//=============================================================================
#undef  super
#define super IO80211P2PInterface
OSDefineMetaClassAndStructors( AirPort_Brcm43xxP2PInterface, IO80211P2PInterface );

bool
AirPort_Brcm43xxP2PInterface::init( IO80211Controller * controller,
                                    struct ether_addr * addr,
                                    UInt32 role,
                                    const char * prefix )
{
	bzero(ifname, sizeof(ifname));
#if VERSION_MAJOR > 10
	opp_ps_enable = FALSE;
	ct_window = 0;
#endif
	list_elt.obj = this;

	return super::init( controller, addr, role, prefix );
}
#endif /* IO80211P2P */


#pragma mark -
#pragma mark Brcm43xx_InputQueue Implementation

//=============================================================================
// Brcm43xx_InputQueue
//=============================================================================
#undef  super
#define super IOPacketQueue
OSDefineMetaClassAndStructors( Brcm43xx_InputQueue, IOPacketQueue )

// -----------------------------------------------------------------------------
// Make our private superclass release() method public
//
void
Brcm43xx_InputQueue::release() const
{
	super::release();
}

// -----------------------------------------------------------------------------
// Method: init
//
bool
Brcm43xx_InputQueue::init( IONetworkController *inTarget,
                           Brcm43xx_InputQueue::Action inAction )
{
	// initialize the IOPacketQueue with capacity = 0 to match our disabled state
	if (super::initWithCapacity(0) == false)
		return false;

	target = inTarget;
	action = inAction;
	hold = FALSE;
	capacity = 1000;

	// Allocate and attach an IOInterruptEventSource object to the target's workloop.
	interruptSrc = IOInterruptEventSource::interruptEventSource(this, serviceInterrupt);
	if (!interruptSrc ||
	    (target->getWorkLoop()->addEventSource(interruptSrc) != kIOReturnSuccess))
		return false;

	return true;
}

// -----------------------------------------------------------------------------
// Factory method that will construct and initialize
// a Brcm43xx_InputQueue object.
//
/* static */ Brcm43xx_InputQueue *
Brcm43xx_InputQueue::withTargetAndAction(IONetworkController *inTarget,
                                         Brcm43xx_InputQueue::Action inAction)
{
	Brcm43xx_InputQueue * queue = new Brcm43xx_InputQueue;

	if (queue && !queue->init(inTarget, inAction)) {
		queue->release();
		queue = 0;
	}
	return queue;
}

// -----------------------------------------------------------------------------
// Frees the Brcm43xx_InputQueue instance.
//
void
Brcm43xx_InputQueue::free()
{
	if (interruptSrc) {
		IOWorkLoop * wl = interruptSrc->getWorkLoop();
		if (wl)
			wl->removeEventSource(interruptSrc);
		interruptSrc->release();
	}
	interruptSrc = 0;

	super::free();
}

// -----------------------------------------------------------------------------
// Enable the input queue, allowing packets to be enqueued and calling
// the Action function
// This method needs to be called while protected in the work loop
//
void
Brcm43xx_InputQueue::enable()
{
	setCapacity(capacity);
}

// -----------------------------------------------------------------------------
// Free all packets currently in the queue and accept no more by
// setting the capacity to zero.
// Returns the number of mbufs freed.
// This method needs to be called while protected in the work loop
//
UInt32
Brcm43xx_InputQueue::disable()
{
	setCapacity(0);
	return flush();
}

// -----------------------------------------------------------------------------
// Remove all packets from the queue and free them.
// Returns the number of mbufs freed.
// This method needs to be called while protected in the work loop
//
UInt32
Brcm43xx_InputQueue::flush()
{
	return super::flush();
}

// -----------------------------------------------------------------------------
// Add an mbuf to the queue and signal deferred processing
// This method needs to be called while protected in the work loop
//
UInt32
Brcm43xx_InputQueue::enqueueWithDrop(mbuf_t m)
{
	UInt32 dropCount = 0;

	dropCount = super::enqueueWithDrop(m);

	if (!hold && dropCount == 0)
		scheduleService();

	return dropCount;
}

// -----------------------------------------------------------------------------
// Put service() scheduling on or off "hold". When packets are enqueued,
// do not schedule a service call if hold is set to true.
//
void
Brcm43xx_InputQueue::serviceHold(bool inHold)
{
	hold = inHold;
}

// -----------------------------------------------------------------------------
// Process all packets on the inputQueue, calling the action function on each
// This method needs to be called while protected in the work loop
//
void
Brcm43xx_InputQueue::service()
{
	mbuf_t m = NULL;

	while ((m = dequeue()) != NULL)
		(*action)(target, m);
}

// -----------------------------------------------------------------------------
// IOInterruptEventSource Action method called on the target's work loop
// This method in turn calls the Brcm43xx_InputQueue::Action method provided
// by the target at initialization.
//
/* static */ void
Brcm43xx_InputQueue::serviceInterrupt(OSObject * owner,
                                      IOInterruptEventSource * src,
                                      int count)
{
	BCM_REFERENCE(src);
	BCM_REFERENCE(count);

	((Brcm43xx_InputQueue *)owner)->service();
}

// -----------------------------------------------------------------------------
// Schedule the serviceAction to be called on the target's work loop,
// which will run the service() method.
//
void
Brcm43xx_InputQueue::scheduleService()
{
        interruptSrc->interruptOccurred(0, 0, 0);
}


#pragma mark -
#pragma mark AirPort_Brcm43xx Implementation

//===========================================================================
// AirPort_Brcm43xx
//===========================================================================
#undef  super
#define super IO80211Controller
#ifndef MACOSX_DHD
OSDefineMetaClassAndStructors( AirPort_Brcm43xx, IO80211Controller )
#else
OSDefineMetaClassAndAbstractStructors( AirPort_Brcm43xx, IO80211Controller )
#endif /* !MACOSX_DHD */

// -----------------------------------------------------------------------------
// Method: init
//
// Purpose:
//   Perform minimal initialization of the driver instance and prepare it
//   to handle a probe() call. Remember that the initialization order is
//   init(), probe(), and then start().

bool
AirPort_Brcm43xx::init( OSDictionary * properties )
{
	if (super::init( properties ) == false)
		return false;

#if defined(USE_KLOG) && defined(BCMDBG)
	// configure KLOG to send output to KLOG kext only
	KernelDebugSetOutputType( kKernelDebugOutputKextLoggerType );
#endif

	bpfRegistered = false;
	avsEnabled = false;
	ieee80211Enabled = false;
	bsdEnabled = false;
	tx_chain_offset = NULL;

	_pciDeviceId = 0;
	_pciVendorId = 0;

#ifdef BCMDBG
	wl_msg_level |=	(WL_ERROR_VAL)	// errors
	//	|	0x0002	// function trace
	//	|	0x0004	// one-line frame tx/rx summary
	//	|	0x0008	// complete frame tx/rx in hex
	//	|	0x0010	// informational
	//	|	0x0020	// TMP messages
	//	|	0x0040	// OID indications
	//	|	0x0080	// rate change
	//	|	0x0100	// association
	//	|	0x0200	// ethernet header tx/rx in hex
	//	|	0x0400	// AP power save messages
	//	|	0x0800	// TX power control messages
	//	|	0x1000	// port
	//	|	0x4000	// wsec stuff
	;
	wl_msg_level2 |= 0;
#else /* !BCMDBG */
	// For non-Debug drivers, be silent until msglevel is enabled
	wl_msg_level  = 0;
	wl_msg_level2 = 0;
#endif

#if defined(IO80211P2P) && defined(FAST_ACTFRM_TX)
	actFrmPending = false;
#endif

#ifdef MACOS_AQM
	// Change this for default scheduling:
	//		-1:																	AQM disabled
	//		IONetworkInterface::kOutputPacketSchedulingModelDriverManaged:		TCQ (traffic class queueing)
	_outputPacketSchedulingModel = IONetworkInterface::kOutputPacketSchedulingModelDriverManaged;
#endif /* MACOS_AQM */
#ifdef AAPLPWROFF
	_lastUserRequestedPowerState = PS_INDEX_ON;
#endif

	g_brcm_controller = (void*)this;

	return true;
}

// Add "board-id" for 0x4331 device systems that are to allowed to load the driver
const char
*_dev0x4331ValidLoadDriverBoardIds[] =
{
	"Mac-00BE6ED71E35EB86",                     // D7       :       - iMac
	"Mac-7DF2A3B5E5D671ED",                     // D7i      :       - iMac
	"Mac-FC02E91DDD3FA6A4",                     // D8       :       - iMac

	"Mac-ACE8A17C0DE83137",                     // J50      :       - Mac Mini
	"Mac-031AEE4D24BFF0B1",                     // J50i     :       - Mac Mini
	"Mac-F65AE981FFA204ED",                     // J50s     :       - Mac Mini
	"Mac-C6EFA63962FC6EA0",                     // J50g     :       - Mac Mini

	"Mac-AFD8A9D944EA4843",                     // D1       :       - 13" MacBook Pro Retina
	"Mac-C3EC7CD22292981F",                     // D2       :       - 15" MacBook Pro Retina

	"Mac-6F01561E16C75D06",                     // J30      :       - 13" MacBook Pro
	"Mac-4B7AC7E43945597E",                     // J31      :       - 15" MacBook Pro

	NULL,                                       // sentinel
};

// Add "board-id" for 0x4353 device systems that are to allowed to load the driver
const char
*_dev0x4353ValidLoadDriverBoardIds[] =
{
	"Mac-66F35F19FE2A0D05",                     // J11      :  2012 - 11" MacBook Air, 43224
	"Mac-2E6FAB96566FE58C",                     // J13      :  2012 - 13" MacBook Air, 43224

	NULL,                                       // sentinel
};

bool
AirPort_Brcm43xx::canLoad( IOService *provider )
{
	IOPCIDevice *nub = OSDynamicCast(IOPCIDevice, provider);
	UInt16 deviceID = 0;

	if( !nub )
	{
		// Can load driver
		return true;
	}

	deviceID = nub->configRead16(kIOPCIConfigDeviceID);

	if ( deviceID == BCM4331_D11N_ID )// Check valid BCM4331 dualband board-id list
	{
		for( int i=0 ; _dev0x4331ValidLoadDriverBoardIds[i] != NULL; ++i )
		{
			// Check "board-id"
			if( (checkBoardId( _dev0x4331ValidLoadDriverBoardIds[i] ) == true) )
			{
				return true;
			}
		}

		// Not in valid list
		return false;
	}
	else if ( deviceID == BCM43224_D11N_ID )// Check valid BCM43224 dualband board-id list
	{
		for( int i=0 ; _dev0x4353ValidLoadDriverBoardIds[i] != NULL; ++i )
		{
			// Check "board-id"
			if( (checkBoardId( _dev0x4353ValidLoadDriverBoardIds[i] ) == true) )
			{
				return true;
			}
		}

		// Not in valid list
		return false;
	}

	// Can load driver
	return true;
}

// Apple change: <rdar://problem/7357113>
// This method is called with the "score" initialized
// to the value defined as IOProbeScore in the Info.plist
// So we can't use kIODefaultProbeScore as the base,
// since the Info.plist will have a much higher number
IOService*
AirPort_Brcm43xx::probe(IOService *provider, SInt32 *score)
{
	WL_PORT(("wl: probe "));
	if( super::probe( provider, score ) )
	{
		WL_PORT(("%d\n", (int)*score));
	}

#if defined(ENABLE_LOADDRIVER_CHECKBOARDID)
	if( (canLoad( provider ) == false) )
	{
		return( NULL );
	}
#endif /* ENABLE_LOADDRIVER_CHECKBOARDID */

	IOPCIDevice *nub = OSDynamicCast(IOPCIDevice, provider);
	if (nub != NULL)
	{
		_pciDeviceId = nub->configRead16(kIOPCIConfigDeviceID);
		_pciVendorId = nub->configRead16(kIOPCIConfigVendorID);
	}

	return(this);
}

#ifdef CCA_STATS
static const struct {
	int16 htol;
	int16 ltoh;
} chq_metric_pairs[] = {
	/* CCA[0] */
	{23, 27}, {48, 52}, {68, 72}, {78, 82}, {88, 92}, {93, 97}, {0, 0},
	/* NF[1] */
	{-95, -90}, {-85, -75}, {-70, -65}, {0, 0},
	/* NF_LTE[2] */
	{0, 0}, /* NF_LTE is not needed */
};

bool
AirPort_Brcm43xx::setupCCAChannelQualityEvent(bool start)
{
	int err = BCME_OK;

	wl_chan_qual_event_t chq = {};
	uint id = 0, pair = 0, level = 0;

	memset(&chq, 0, sizeof(wl_chan_qual_event_t));

	/* For start, set chq_event parameters per chq_metric_pairs.
	   For stop, reset parameters, note rate_limit_msec=0 would stop the cca timer.
	*/
	if(start) {
		chq.rate_limit_msec = 5000;
		for (id = 0, pair = 0; id < WL_CHAN_QUAL_TOTAL; id++) {
			chq.metric[id].id = id;
			for (level = 0; level < MAX_CHAN_QUAL_LEVELS; level ++) {
				int16 htol = 0, ltoh = 0;
				htol = chq_metric_pairs[pair].htol;
				ltoh = chq_metric_pairs[pair].ltoh;
				pair ++;
				if (pair > sizeof(chq_metric_pairs)/sizeof(chq_metric_pairs[0]))
					break;

				/* double zeros terminate one metric */
				if ((htol == 0) && (ltoh == 0))
					break;

				/* make sure that ltoh >= htol */
				if (ltoh < htol) {
					WL_ERROR(("wl%d: %s wrong chq_event parameters.id=%d,level=%d\n",
						unit, __FUNCTION__, id, level));
					return false;
				}

				chq.metric[id].htol[level] = htol;
				chq.metric[id].ltoh[level] = ltoh;

				/* all metric threshold levels must be in increasing order */
				if (level > 0) {
					if ((chq.metric[id].htol[level] <= chq.metric[id].htol[level - 1]) ||
						(chq.metric[id].ltoh[level] <= chq.metric[id].ltoh[level - 1])) {
						WL_ERROR(("wl%d: %s wrong chq_event parameters.id=%d,level=%d\n",
							unit, __FUNCTION__, id, level));
						return false;
					}
				}
			}
			chq.metric[id].num_levels = level;
		}
		chq.num_metrics = id;
	}
	err = wlIovarOp("chq_event", NULL, 0, &chq, sizeof(chq), IOV_SET, NULL);
	if (err) {
		WL_IOCTL(("wl%d: chq_event: error \"%s\" (%d)\n", unit, bcmerrorstr(err), err));
		return false;
	}

	return true;
}
#endif /* CCA_STATS */

// -----------------------------------------------------------------------------
// Method: start
//
// Purpose:
//   Hardware was detected and initialized, start the driver. The
//   controller can be initialized, but it is not enabled until we
//   receive the enable() call.
bool
AirPort_Brcm43xx::start( IOService *provider )
{
	intf_info_blk_t *blk = NULL;
	int32_t value = -1;
	int32_t rifs_advert = 0;
	int err = BCME_OK;

	WL_IOCTL(("wl: start\n"));
	kprintf("wl: start\n");

	// Start our superclass first.
	if (!super::start( provider )) {
		WL_PRINT(("AirPort_Brcm43XX::start: super::start fails\n"));
		return false;
	}

	// Early log parsing
	_logging = 0x0;
	if( PE_parse_boot_argn( "bcom.logging", &value, sizeof(value) ) )
	{
		_logging = value;
	}

	if( !getCommandGate() )
	{
		WL_PRINT(("AirPort_Brcm43XX::start: No command gate, failing\n"));
		return false;
	}
	pciNub = OSDynamicCast(IOPCIDevice, provider);
	if (pciNub == NULL)
	{
		WL_PRINT(("AirPort_Brcm43XX::start: OSDynamicCast(IOPCIDevice, provider) failed!\n"));
		return false;
	}

	if( PE_parse_boot_argn( "bcom.panic.invalid-mmio-read", &value, sizeof(value) ) )
	{
		// Boot arg to control forced panic on backplane timeout
		WL_PRINT(("io80211.panic_mimo_invalid = %d\n", value ));

		_panicOnBPT = (value ? TRUE : FALSE);
	} else {
		_panicOnBPT = FALSE;
	}

	if( PE_parse_boot_argn( "bcom.bpt.check", &value, sizeof(value) ) )
	{
		// Boot arg to control forced panic on backplane timeout
		WL_PRINT(("bcom.bpt.check = %d\n", value ));

		_checkAllWrappers = (value ? TRUE : FALSE);
	} else {
		_checkAllWrappers = FALSE;
	}

	//By default, the value is 1; 0 means bypass the power on/off recovery.
	if( PE_parse_boot_argn( "bcom.fatal-error-recovery", &_fatalErrRecovery, sizeof(_fatalErrRecovery)))
	{
		// Boot arg to bypass the fata-error-recovery
		WL_PRINT(("bcom.fatal-error-recovery = %d\n", _fatalErrRecovery ));
	}
	else
	{
		_fatalErrRecovery = 1;
	}

	osh = osl_attach(pciNub);
	if (!osh) {
		WL_PRINT(("osl attach failed \n"));
		return false;
	}

#ifdef PCIE_AER_EVT
	IOPCIEventSource::Action action = OSMemberFunctionCast(IOPCIEventSource::Action, this, &AirPort_Brcm43xx::receivePCIEvent);
	pcieEventSource = pciNub->createEventSource(this, action, 0);

	if (pcieEventSource == NULL) {
		WL_ERROR(("Create PCIEventSource failed\n"));
	} else {
		uint32 deviceControl = 0;

		getWorkLoop()->addEventSource(pcieEventSource);
		pcieEventSource->enable();

		if (pciNub!= NULL)
		{
			deviceControl = pciNub->configRead32(PCIE_CFG_DEVICE_CONTROL);
			deviceControl |= PCIE_DC_AER_CORR_EN |
					PCIE_DC_AER_NON_FATAL_EN |
					PCIE_DC_AER_FATAL_EN |
					PCIE_DC_AER_UNSUP_EN;
			pciNub->configWrite32(PCIE_CFG_DEVICE_CONTROL, deviceControl);
		}

		WL_PORT(("PCIE Event call back registered\n"));
	}
#endif
#ifdef AAPLPWROFF
	_powerChangeNotifier = registerPrioritySleepWakeInterest( powerChangeCallback, this, 0 );
	if( !_powerChangeNotifier )
		goto fail;

	//Need to call hasPCIPowerManagement, not for system capability, but to set the rigth state in PCI family to enable Power Mgmt
	if ( pciNub->hasPCIPowerManagement ( kPCIPMCPMESupportFromD3Cold ) ) {
		kprintf("Enabling PCI power management\n");
		pciNub->enablePCIPowerManagement ( kPCIPMCSPowerStateD3 );
	}

#if VERSION_MAJOR > 10
	_enabledDeviceRecovery = true;

	// Check platform expert for "io80211.pcirecovery"
	if (PE_parse_boot_argn("io80211.pcirecovery", &value, sizeof(value)))
	{
		// Specified boot-arg "io80211.pcirecovery"
		printf("Over-riding PCI device recovery, found \"io80211.pcirecovery\" arg, value[%d]\n", value );
		_enabledDeviceRecovery = value;
	}


	// pass NULL or currentHandler, currentRef to get current handler installed
	// pass NULL or handler, ref to set handler for device
	// messages: kIOMessageDeviceWillPowerOff, kIOMessageDeviceHasPoweredOff,
	//           kIOMessageDeviceWillPowerOn, kIOMessageDeviceHasPoweredOn
	// state: D3

	pciNub->setConfigHandler(&configHandler, (void *) this, NULL, NULL );
#endif /* VERSION_MAJOR > 10 */
#endif /* AAPLPWROFF */

#if !defined(BCMDBG) && defined(BCMDBG_ASSERT)
	g_assert_type = TRUE;
#else
	{
		OSBoolean *msgProp = OSDynamicCast( OSBoolean, getProperty("wl_assert_bypass") );
		if (msgProp) {
			kprintf("asrt_byp: 0x%x\n", msgProp->getValue());
			g_assert_type = msgProp->getValue();
		}
	}
#endif

#if defined(BCMDBG) && defined(BCMDBG_ASSERT)
	// Allow over-ride assert_type
	if( PE_parse_boot_argn( "io80211.assert_type", &value, sizeof(value) ) )
	{
		// Specified boot-arg "io80211.assert_type"
		printf("Over-riding assert_type, found \"io80211.assert_type\" arg, value[%d]\n", value );

		g_assert_type = value ? TRUE : FALSE;
	}
#endif /* defined(BCMDBG) && defined(BCMDBG_ASSERT) */

	iobuf_small = (uint8*)MALLOC(osh, WLC_IOCTL_SMLEN);
	if (!iobuf_small) {
		WL_PRINT(("allocate iobuf_small failed \n"));
		goto fail;
	}

	if (!wlcStart()) {
		WL_PRINT(("wlcStart failed \n"));
		goto fail;
	}

#if (defined(WL_EVENTQ) && defined(P2PO) && !defined(P2PO_DISABLED))
	/* allocate wl_eventq info struct */
	if ((wlevtq = wl_eventq_attach(pWLC)) == NULL) {
		WL_PRINT(("wl%d: wl_eventq_attach failed\n", pub->unit));
		goto fail;
	}
#endif /* WL_EVENTQ && P2PO && ANQPO */

#if (defined(P2PO) && !defined(P2PO_DISABLED)) || (defined(ANQPO) && \
	!defined(ANQPO_DISABLED))
	/* allocate gas info struct */
	if ((gas = wl_gas_attach(pWLC, wlevtq)) == NULL) {
		WL_PRINT(("wl%d: wl_gas_attach failed\n", pub->unit));
		goto fail;
	}
#endif /* P2PO || ANQPO */

#if defined(P2PO) && !defined(P2PO_DISABLED)
	/* allocate the disc info struct */
	if ((disc = wl_disc_attach(pWLC)) == NULL) {
		WL_PRINT(("wl%d: wl_disc_attach failed\n", pub->unit));
		goto fail;
	}

	/* allocate the p2po info struct */
	if ((p2po = wl_p2po_attach(pWLC, wlevtq, gas, disc)) == NULL) {
		WL_PRINT(("wl%d: wl_p2po_attach failed\n", pub->unit));
		goto fail;
	}
#endif /* defined(P2PO) && !defined(P2PO_DISABLED) */

#if defined(ANQPO) && !defined(ANQPO_DISABLED)
	/* allocate the anqpo info struct */
	if ((anqpo = wl_anqpo_attach(pWLC, gas)) == NULL) {
		WL_PRINT(("wl%d: wl_anqpo_attach failed\n", pub->unit));
		goto fail;
	}
#endif /* defined(ANQPO) && !defined(ANPO_DISABLED) */

	// Build an mbuf pool in address range < 1GB if needed
	if (osl_alloc_private_mbufs(osh)) {
		WL_PRINT(("wl%d: osl_alloc_private_mbufs failed\n", pub->unit));
		goto fail;
	}

	// Allocate memory for tx_chain_offset;
	tx_chain_offset = (int32_t*)MALLOC(osh, (sizeof(int32_t) * WLC_CHAN_NUM_TXCHAIN));
	if (!tx_chain_offset) {
		WL_PRINT(("wl%d: tx_chain_offset failed\n", pub->unit));
		goto fail;
	}

	bzero(tx_chain_offset, (sizeof(int32_t) * WLC_CHAN_NUM_TXCHAIN));
#ifndef MACOSX_DHD
	if (!createInterruptEvent()) {
		WL_PRINT(("wl%d: createInterruptEvent() failed\n", pub->unit));
		goto fail;
	}
#endif /* !MACOSX_DHD */

	inputQueue = Brcm43xx_InputQueue::withTargetAndAction((IONetworkController*)this,
	                                                      inputAction);
	if (!inputQueue) {
		WL_PRINT(("wl%d: inputQueue::withTargetAndAction failed\n", pub->unit));
		goto fail;
	}

	// publish the medium dictionary to announce our media capabilities
	(void) PubWirelessMedium();

	// IONetworkController will allocate & register the interface instance for us
	if (!attachInterface( (IONetworkInterface **)&myNetworkIF, true )) {
		WL_PRINT(("AirPort_Brcm43XX::start: attachInterface failed\n"));
		goto fail;
	}

	bzero(ifname, sizeof(ifname));

	blk = &myNetworkIF->intf_info;

	blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
	blk->deauth_reason = 0;
	blk->link_deauth_reason = 0;
	blk->cached_assocstatus = APPLE80211_STATUS_UNAVAILABLE;
	blk->authtype_lower = APPLE80211_AUTHTYPE_OPEN;
	blk->authtype_upper = APPLE80211_AUTHTYPE_NONE;

	bzero(&blk->deauth_ea, ETHER_ADDR_LEN);
	bzero(&configured_bssid, ETHER_ADDR_LEN);

#ifdef IO80211P2P
#ifdef P2P_IE_OVRD
	{
	wlc_ssid_t ssid = {};
	ssid.SSID_len = snprintf((char *)ssid.SSID, sizeof(ssid.SSID), "Air-");
	(void) wlIovarOp("p2p_ssid", NULL, 0, (void *)&ssid, sizeof(ssid), IOV_SET, NULL);
	}
#endif /* P2P_IE_OVRD */
	// Virtual Interface Linked list init
	SLIST_INIT(&virt_intf_list);
#endif /* IO80211P2P */

	// initialize scan state
	scan_done = 0;
	scan_event_external = 0;
	SLIST_INIT(&scan_result_list);
	current_scan_result = NULL;

	// initialize assoc sta ie list
	SLIST_INIT(&assocStaIEList);

	// initialize mscan maclist and state
	bzero(mscan_bssid_list, (ETHER_ADDR_LEN * APPLE80211_MAX_SCAN_BSSID_CNT));
	num_mscan_bssid = 0;
	inMultiScan = FALSE;

	registerService();		// let potential clients know we exist

	/* print hello string */
	if (revinfo.deviceid > 0x9999)
		kprintf("wl%d: Broadcom BCM%d 802.11 Wireless Controller\n" EPI_VERSION_STR"\n",
			unit, revinfo.deviceid);
	else
		kprintf("wl%d: Broadcom BCM%04x 802.11 Wireless Controller\n" EPI_VERSION_STR"\n",
			unit, revinfo.deviceid);
#ifdef BCMDBG
#define quote_sym_val(x) quote_string(x)
	kprintf(" (Compiled in " quote_sym_val( SRCROOT ) " at " __TIME__ " on " __DATE__ ")\n");
#endif /* BCMDBG */

#ifdef AAPLPWROFF
	enableTruePowerOffIfSupported();
#endif

	// Driver has loaded; the module has the proper configspace contents.
	// save PCIe configuration.
	/* cached PCIe configspace after wlcStart successful */
	if (BUSTYPE(revinfo.bus) == PCI_BUS) {
#ifdef BCMDBG
		IOPCILogDevice( pciNub, "BRCM: PCIe config-space save" );
#endif /* BCMDBG */
#ifndef MACOSX_DHD
		si_pcie_configspace_cache(pub->sih);
#endif /* !MACOSX_DHD */
#ifdef AAPLPWROFF
#if VERSION_MAJOR > 10
		saveDeviceState( pciNub );
#endif /* VERSION_MAJOR > 10 */
#endif /* AAPLPWROFF */

		uint16 vendorId = pciNub->configRead16(kIOPCIConfigVendorID);
		uint32 bar0 = pciNub->configRead32(kIOPCIConfigBaseAddress0);

		printf("wl%d: Broadcom BCM%04x, vendorID[0x%04x] BAR0[0x%08x]\n" EPI_VERSION_STR"\n",
			unit, revinfo.deviceid, vendorId, bar0);
	}

#if defined(WLBTCPROF) && defined(APPLE80211_IOC_BTCOEX_PROFILES)
	memset(&btc_in_profile[0], 0, sizeof(struct apple80211_btc_profiles_data));
	memset(&btc_in_config, 0, sizeof(struct apple80211_btc_config_data));
#endif /* WLBTCPROF && APPLE80211_IOC_BTCOEX_PROFILES */

#if defined(ENABLE_CORECAPTURE)
	if( initializeCoreCapture() != true )
	{
		WL_ERROR(("wl%d: Failed to initialize corecapture\n", unit));

		// Fall-through, if corecapture fails to initialize, still load the driver
	}
#endif /* ENABLE_CORECAPTUE */

#ifdef NEED_HARD_RESET
	_pwrCycleOffOnThreadCall = thread_call_allocate(_powerCycleOffOnThread, this);
#endif /* NEED_HARD_RESET */

#ifdef CCA_STATS
	if (setupCCAChannelQualityEvent(true) != true) {
		WL_ERROR(("wl%d: Failed to initialize chq_event\n", unit));

		// Fall-through, if chq_event fails to initialize, still load the driver
	}
#endif /* CCA_STATS */

	err = wlIovarOp("rifs_advert", NULL, 0, (void *)&rifs_advert,
			sizeof(rifs_advert), IOV_SET, NULL);
	if (err) {
		WL_ERROR(("wl%s: Failed to set rifs_advert to 0\n", __FUNCTION__));
		goto fail;
	}
	return true;

fail:
	WL_PRINT(("AirPort_Brcm43XX::start: failed\n"));
	stop_local();

	return false;
}


#ifdef MACOS_VIRTIF
#ifndef ETHER_SET_LOCALADDR
#define ETHER_SET_LOCALADDR(ea)	(((uint8 *)(ea))[0] = (((uint8 *)(ea))[0] | 2))
#endif

extern "C" {

/////////////////////////////////////////////////////////////////
extern IOEthernetInterface *
brcmVirtEther_create_interface(IOEthernetController *hdl, IOEthernetAddress *addrs, IOOutputAction _outAction,
                               IONetworkController::Action _ioctlAction, uint unit);

extern bool
brcmVirtEther_destroy_interface(IOEthernetInterface *intf);

/////////////////////////////////////////////////////////////////

}

wl_if_t *
AirPort_Brcm43xx::createVirtIf(struct wlc_if *wlcif, uint unit)
{
	IOEthernetAddress virtaddr = NULL;

	if (apif.virtIf != NULL || !WLC_P2P_CAP(pWLC))
		return NULL;

	bzero(&apif, sizeof(wl_if_t));
	apif.wl = (struct wl_info*)this;
	apif.wlcif = wlcif;
	apif.unit = unit;

	bzero(&virtaddr, sizeof(IOEthernetAddress));

	IOLog("%s\n", __FUNCTION__);
	// NOW the source of the mac address is 'wl p2p_ifadd <mac>'
	if (getHardwareAddress(&virtaddr) == kIOReturnSuccess) {
		ETHER_SET_LOCALADDR(&virtaddr);

		wlc_iovar_op(pWLC, "cur_etheraddr", NULL, 0, &virtaddr, ETHER_ADDR_LEN, IOV_SET, wlcif);

		if ((mysecondIF = brcmVirtEther_create_interface((IOEthernetController *)this,
		                                                 &virtaddr,
		                                                 (IOOutputAction)&AirPort_Brcm43xx::outputPacketVirtIf,
		                                                 (IONetworkController::Action)&AirPort_Brcm43xx::handleVirtIOC,
		                                                 unit)) == NULL)
			goto err;
		apif.virtIf = mysecondIF;
	}
	/* Populate all the interfaces including this new one with the multicast addresses */
	populateMulticastMode( );
	populateMulticastList( );

	return &apif;
err:
	bzero(&apif, sizeof(wl_if_t));
	return NULL;
}

void
AirPort_Brcm43xx::destroyVirtIf(wl_if_t *wlif)
{
	if (wlif != &apif)
		return;
	if (apif.virtIf) {
		ASSERT(apif.virtIf == mysecondIF);
		if (brcmVirtEther_destroy_interface(mysecondIF))
			mysecondIF = NULL;
	}

	bzero(&apif, sizeof(wl_if_t));
}
#endif /* MACOS_VIRTIF */

#ifdef IO80211P2P

IO80211VirtualInterface *
AirPort_Brcm43xx::createVirtualInterface( struct ether_addr * addr, UInt32 role )
{
	IO80211VirtualInterface * vif = NULL;
	intf_info_blk_t *blk = NULL;

	switch( role )
	{
		case APPLE80211_VIRT_IF_ROLE_P2P_DEVICE:
		case APPLE80211_VIRT_IF_ROLE_P2P_CLIENT:
		case APPLE80211_VIRT_IF_ROLE_P2P_GO:
			vif = (IO80211VirtualInterface *)OSTypeAlloc( AirPort_Brcm43xxP2PInterface );
                        if(vif==NULL)
                                break;
			if(!vif->init( (IO80211Controller *)this, addr, role, "p2p" ) )
			{
				vif->release();
				vif = NULL;
				break;
			}
			blk = &((AirPort_Brcm43xxP2PInterface*)vif)->intf_info;

			blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
			blk->deauth_reason = 0;
			blk->link_deauth_reason = 0;
			blk->cached_assocstatus = APPLE80211_STATUS_UNAVAILABLE;

			bzero(&blk->deauth_ea, ETHER_ADDR_LEN);
			blk->authtype_lower = APPLE80211_AUTHTYPE_OPEN;
			blk->authtype_upper = APPLE80211_AUTHTYPE_NONE;

			break;
		default:
			vif = super::createVirtualInterface( addr, role );
	}

	return vif;
}

wl_if_t *
AirPort_Brcm43xx::createVirtIf(struct wlc_if *wlcif, uint unit)
{
	int bcmerr = BCME_OK;
	IOEthernetAddress virtaddr;
	AirPort_Brcm43xxP2PInterface *newInterfacePtr = NULL;

	if (!WLC_P2P_CAP(pWLC))
		return NULL;
	bzero(&virtaddr, sizeof(IOEthernetAddress));

	/* validate _attachingIfRole before proceeding */
	validateIfRole(wlcif);

	bcmerr = wlc_iovar_op(pWLC, "cur_etheraddr", NULL, 0, &virtaddr, ETHER_ADDR_LEN, IOV_GET, wlcif);
	if (bcmerr) {
		WL_ERROR(("unit %d failed, cur_etheraddr returned error = %d\n", unit, osl_error(bcmerr)));
		return NULL;
	}

	bool result = attachVirtualInterface( (IO80211VirtualInterface **)&newInterfacePtr,
	                                      (struct ether_addr *)&virtaddr,
	                                      _attachingIfRole );
	if( !result ) {
		WL_ERROR(("unit %d failed, result = %d\n", unit, result));
		return NULL;
	}

	newInterfacePtr->wlcif = wlcif;
	newInterfacePtr->unit = unit;

	snprintf(newInterfacePtr->ifname, IFNAMSIZ, "%s", newInterfacePtr->getBSDName());

	WL_P2P(("Created virtif %p %s\n", newInterfacePtr, newInterfacePtr->ifname));
	// Add it to the list
	SLIST_INSERT_HEAD(&virt_intf_list, &newInterfacePtr->list_elt, entries);

	populateMulticastMode( );
	populateMulticastList( );

	return (wl_if_t *)newInterfacePtr;
}

void
AirPort_Brcm43xx::destroyVirtIf(AirPort_Brcm43xxP2PInterface *removeIf)
{
	if (!removeIf)
		return;

	// De-link it from the linklist
	SLIST_REMOVE(&virt_intf_list, &removeIf->list_elt, virt_intf_entry, entries);

	// <rdar://problem/8501031> Virtual interfaces must be detached asynchronously
	// to avoid deadlocks
	detachVirtualInterface( removeIf , false );
	removeIf->release();
	removeIf = NULL;
}

/* Currently, when issuing wl p2p_ifadd, _attachingIfRole
 * will most likely not be setup yet. Thus, we will try to
 * detect non-valid _attachingIfRole values and make it valid
 * using information retrieved from bsscfg structure
 */
void AirPort_Brcm43xx::validateIfRole(struct wlc_if *wlcif)
{
	wlc_bsscfg_t *bsscfg = NULL;

	switch(_attachingIfRole)
	{
		case APPLE80211_VIRT_IF_ROLE_P2P_DEVICE:
		case APPLE80211_VIRT_IF_ROLE_P2P_CLIENT:
		case APPLE80211_VIRT_IF_ROLE_P2P_GO:
			return;
		default:
			/* we have non-valid _attachineIfRole, make it valid */
			bsscfg = wl_find_bsscfg(wlcif);
			ASSERT(bsscfg != NULL);
			if (bsscfg->flags & WLC_BSSCFG_P2P_DISC) {
				_attachingIfRole = APPLE80211_VIRT_IF_ROLE_P2P_DEVICE;
			} else if (BSSCFG_STA(bsscfg)) {
				_attachingIfRole = APPLE80211_VIRT_IF_ROLE_P2P_CLIENT;
			} else {
				_attachingIfRole = APPLE80211_VIRT_IF_ROLE_P2P_GO;
			}
	}
}
#endif /* IO80211P2P */

IOReturn
AirPort_Brcm43xx::terminate_gated(OSObject *owner, void *arg0, void *arg1, void *arg2, void *arg3)
{

    BCM_REFERENCE(arg0);
    BCM_REFERENCE(arg1);
    BCM_REFERENCE(arg2);
    BCM_REFERENCE(arg3);

#ifdef AAPLPWROFF
    AirPort_Brcm43xx *controller = (AirPort_Brcm43xx *)owner;
    if (controller == NULL)
    {
        return kIOReturnSuccess;
    }

    if (controller->platformSupportsTruePowerOff()) {
        controller->setACPIProperty("PDEN", false);
    }
#endif

    return kIOReturnSuccess;
}

bool
AirPort_Brcm43xx::terminate(IOOptionBits options)
{
    getCommandGate()->runAction(AirPort_Brcm43xx::terminate_gated, NULL, NULL, NULL, NULL);
    return super::terminate(options);
}

// -----------------------------------------------------------------------------
//// Method: stop_local
//
// Purpose:
//   Release all our local sub-class resources
void
AirPort_Brcm43xx::stop_local()
{
#ifdef CCA_STATS
	/* stop channel quality event reporting */
	setupCCAChannelQualityEvent(false);
#endif /* CCA_STATS */
	scanFreeResults();

	if (myNetworkIF)				// detach from the network
	{
		detachInterface( (IONetworkInterface*)myNetworkIF, TRUE );
		myNetworkIF->release();
	}
	myNetworkIF = NULL;


#ifdef MACOS_VIRTIF
	destroyVirtIf(&apif);
#endif /* MACOS_VIRTIF */

	if (interruptSrc) {
		interruptSrc->disable();
		getWorkLoop()->removeEventSource( interruptSrc );
		interruptSrc->release();
	}
	interruptSrc = NULL;

	wlcStop();

#ifdef IO80211P2P
	// After wlc has been detached, all the bsscfgs should have been freed
	ASSERT(SLIST_EMPTY(&virt_intf_list));
#endif /* IO80211P2P */

	if (inputQueue) {
		inputQueue->disable();
		inputQueue->release();
	}
	inputQueue = NULL;

	if (iobuf_small)
		MFREE(osh, iobuf_small, WLC_IOCTL_SMLEN);
	iobuf_small = NULL;

	if (tx_chain_offset) {
		MFREE(osh, tx_chain_offset, (sizeof(int32_t) * WLC_CHAN_NUM_TXCHAIN));
		tx_chain_offset = NULL;
	}

	/* All memory should be freed before osl_detach()
	 * There is a BCMDBG_MEM check in osl_detach() */
	if (osh)
		osl_detach(osh);
	osh = NULL;

#ifdef AAPLPWROFF
	if( _pwrOffThreadCall )
		thread_call_free( _pwrOffThreadCall );

	if( _powerChangeNotifier )
	{
		_powerChangeNotifier->remove();
		_powerChangeNotifier = NULL;
	}

#if VERSION_MAJOR > 10
	// pass NULL or currentHandler, currentRef to get current handler installed
	// pass NULL or handler, ref to set handler for device
	// messages: kIOMessageDeviceWillPowerOff, kIOMessageDeviceHasPoweredOff,
	//           kIOMessageDeviceWillPowerOn, kIOMessageDeviceHasPoweredOn
	ASSERT(pciNub != NULL);
	pciNub->setConfigHandler(NULL, NULL, NULL, NULL);
#endif /* VERSION_MAJOR > 10 */
#endif /* AAPLPWROFF */
}

// -----------------------------------------------------------------------------
//// Method: stop
//
// Purpose:
//   Release all driver resources.
void
AirPort_Brcm43xx::stop( IOService * provider )
{
	WL_IOCTL(("wl: stop\n"));

	if (osh) {
		stop_local();
	}

	super::stop( provider );
}

// -----------------------------------------------------------------------------
bool
AirPort_Brcm43xx::checkCardPresent( bool do_print, uint32 *status )
{
	if (status) *status = 0;

	if (timeNow >= 3)
		return TRUE;

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		WL_PORT(("%s: timeNow %d\n", __FUNCTION__, timeNow));
	}
#endif

	if ( pciNub != NULL ) {
		uint16 vendorID = pciNub->configRead16( kIOPCIConfigVendorID );
		uint16 deviceID = pciNub->configRead16( kIOPCIConfigDeviceID );

		if (vendorID != VENDOR_BROADCOM) {
			if (do_print)
				WL_ERROR(("Vendor ID 0x%x is not valid\n", vendorID));
			if (status) *status = CARD_NOT_POWERED;
			return FALSE;
		}

		if (deviceID == 0x0 || deviceID == 0xFFFF) {
			if (do_print)
				WL_ERROR(("Device ID 0x%x is not valid\n", deviceID));
			if (status) *status = CARD_NOT_POWERED;
			return FALSE;
		}

		uint32	bar0 = pciNub->configRead32( kIOPCIConfigBaseAddress0 );

		if (bar0 == 0xffffffff) {
#ifdef AAPLPWROFF
			if (do_print)
				WL_ERROR(( "WLAN adapter not yet powered or its not in the"
				      " system (stateNumber = %d)!\n", (int)_powerState ));
#endif /* AAPLPWROFF */
			if (status) *status = NOCARD_PRESENT;

#if defined(MAC_OS_X_VERSION_10_9)
			UInt32 kvCount = 1;
			const char *keys[] = {"com.apple.message.failure"};
			const char *values[] = {"NoCardPresent"};
			IO8LogMT("wifi.driver.broadcom.card_not_found", keys, values, kvCount);
#else
				IO8LogMT((APPLE80211_MT_FLAG_DOMAIN | APPLE80211_MT_FLAG_SIG ),
					 (char *)"wifi.cardNotFound",
					 NULL,
					 (char *)"WLAN adapter not yet powered or its not in the system",
					 NULL,
					 NULL,
					 0,
					 0,
					 0,
					 0,
					 NULL,
					 NULL,
					 NULL);
#endif

			return FALSE;
		}
		if (bar0 == 0x4) {
#ifdef AAPLPWROFF
			if (do_print)
				WL_ERROR(( "WLAN adapter PCIE config space was not restored (stateNumber = %d)\n",
				      (int)_powerState ));
#endif /* AAPLPWROFF */
			if (status) *status = BAR0_INVALID;

#if defined(MAC_OS_X_VERSION_10_9)
			UInt32 kvCount = 1;
			const char *keys[] = {"com.apple.message.failure"};
			const char *values[] = {"PCIEConfigNotRestored"};

			IO8LogMT("wifi.driver.broadcom.card_not_found", keys, values, kvCount);
#else
				IO8LogMT((APPLE80211_MT_FLAG_DOMAIN | APPLE80211_MT_FLAG_SIG ),
					 (char *)"wifi.cardNotFound",
					 NULL,
					 (char *)"WLAN adapter PCIE config space was not restored",
					 NULL,
					 NULL,
					 0,
					 0,
					 0,
					 0,
					 NULL,
					 NULL,
					 NULL);
#endif

			return FALSE;
		}
		return TRUE;
	} else {

#if defined(MAC_OS_X_VERSION_10_9)
		UInt32 kvCount = 1;
		const char *keys[] = {"com.apple.message.failure"};
		const char *values[] = {"PCINubNotInitialized"};

		IO8LogMT("wifi.driver.broadcom.card_not_found", keys, values, kvCount);
#else
			IO8LogMT((APPLE80211_MT_FLAG_DOMAIN | APPLE80211_MT_FLAG_SIG ),
				 (char *)"wifi.cardNotFound",
				 NULL,
				 (char *)"PCI nub not initialized",
				 NULL,
				 NULL,
				 0,
				 0,
				 0,
				 0,
				 NULL,
				 NULL,
				 NULL);
#endif

		return FALSE;
	}
}
#ifndef MACOSX_DHD
void
AirPort_Brcm43xx::deviceWowlClr()
{
	if (wowl_cap) {
		bool wowl_active = WOWL_ACTIVE(pub);

		// Apple change: Always call wlc_wowl_clear to make sure the wake
		// indication bit gets cleared even if WOW was not enabled
		wlc_wowl_clear(((AirPort_Brcm43xx *)this)->pWLC->wowl);
		wlc_wowl_wake_reason_process(((AirPort_Brcm43xx *)this)->pWLC->wowl);

		/* Clear PME in the pci config. This needs to be done after
		 * code above has cached the status
		 */
		if (wowl_active) {
			if (BUSTYPE(pub->sih->bustype) == PCI_BUS)
				si_pci_pmeclr(pub->sih);
		}

	}
}
#endif /* !MACOSX_DHD */
IOReturn
AirPort_Brcm43xx::enable( IONetworkInterface * aNetif )
{
	AirPort_Brcm43xxInterface *brcm_interface = NULL;
	IOReturn err = BCME_OK;
	uint32 status;

	KERNEL_DEBUG_CONSTANT(WiFi_init_enable | DBG_FUNC_NONE, 0, 0, 0, 0, 0);

#if defined(IO80211P2P)
	if( OSDynamicCast( IO80211P2PInterface, aNetif ) )
		return super::enable( aNetif );
#endif

	WL_IOCTL(("wl%d: enable\n", unit));

#ifndef MACOSX_DHD
	// Apple change: This call is redundant with the call in AirPort_Brcm43xx::syncPowerState.
	// Only make it if WoWL is still somehow active at this point.
	if(WOWL_ACTIVE(pub))
		deviceWowlClr();

	if (ASSOC_RECREATE_ENAB(pub) && pub->associated) {
		brcm_interface = OSDynamicCast( AirPort_Brcm43xxInterface, aNetif );
		if (brcm_interface)
			brcm_interface->suppressLink = true;
	}
#endif /* !MACOSX_DHD */
	err = super::enable( aNetif );

	if (brcm_interface)
		brcm_interface->suppressLink = false;

#if VERSION_MAJOR > 12
	// Initialize the linkState if the linkstate == kIO80211NetworkLinkUndefined,
 	//   support for "Always On Always Connected"
 	// Without this fix the ifconfig "status" has no property and some upper layer applications fail without
 	//   the ifconfig "status" property being set.
	// <rdar://problem/13013042> BRCM: Implement setLinkState in controller ::enable for AoAc support
 	if( aNetif && (IFMTHD( aNetif, linkState ) == kIO80211NetworkLinkUndefined) )
 	{
#ifdef APPLE80211_IOC_LINK_CHANGED_EVENT_DATA
		intf_info_blk_t *blk = IFINFOBLKP(aNetif);

		if (blk) {
			blk->isLinkDown = TRUE;
			blk->link_changed_reason.link_down_reason.reason = 0;
		}
#endif
		setLinkState( kIO80211NetworkLinkDown, DOT11_RC_DISASSOC_LEAVING );
 	}
#endif /* VERSION_MAJOR > 12 */

	inputQueue->enable();

#ifdef MACOS_AQM
	// If AQM supported, then start output thread
	if( brcm_interface && brcm_interface->aqmSupported() ) /* AQM enabled */
	{
		WL_IOCTL(("wl%d: ::enable AQM: startOutputThread\n", unit));

		aNetif->startOutputThread();
	}
#endif /* MACOS_AQM */

	if( err == kIOReturnSuccess && !bpfRegistered )
	{
		IO80211Interface * interface = OSDynamicCast( IO80211Interface, aNetif );

		if( interface )
		{
			err = interface->bpfAttach( DLT_IEEE802_11_RADIO_AVS, AVS_HEADER_LEN );

			if( err == 0 )
				bpfRegistered = true;

			err = interface->bpfAttach( DLT_IEEE802_11, MAC_80211_HEADER_LEN );

			if( err == 0 && !bpfRegistered )
				bpfRegistered = true;

			err = interface->bpfAttach( DLT_IEEE802_11_RADIO,
				MAC_80211_HEADER_LEN + sizeof(struct wl_radiotap_hdr));

			if( err == 0 && !bpfRegistered )
				bpfRegistered = true;

			err = interface->bpfAttach( DLT_PPI, MAC_80211_HEADER_LEN + PPI_HEADER_LEN );

			if( err == 0 )
				bpfRegistered = true;
		}
	}

	if (PHYTYPE_11N_CAP(phytype) && !enable11n) {
		struct apple80211_phymode_data pd = { APPLE80211_VERSION, 0, 0 };

		if( getPHY_MODE( (IO80211Interface *)aNetif, &pd ) != 0 )
			pd.phy_mode = ( APPLE80211_MODE_11A | APPLE80211_MODE_11B | APPLE80211_MODE_11G );
		else
			pd.phy_mode &= ~APPLE80211_MODE_11N;

		setPHY_MODE( (IO80211Interface *)aNetif, &pd );
	}

	// Force bringup of driver to apply all PCI setup initialization.
	// Airport Off is taken care of by SW radio disable
	if (checkCardPresent( TRUE, &status )) {
#ifndef MACOSX_DHD
		if (_up) {
			wl_up( (struct wl_info *)this );
		}
#else
		wl_up( (struct wl_info *)this );
#endif /* !MACOSX_DHD */

		//Override ASPM depending on the model
#ifdef AAPLPWROFF
		checkOverrideASPM();
#endif /* AAPLPWROFF */
		NoCard = FALSE;
	} else {
		kprintf("enable:Setting NO card to true !\n");
#ifndef MACOSX_DHD
		WL_HEALTH_LOG(((AirPort_Brcm43xx *)this)->pWLC, status);
		wl_recover_nocard( (struct wl_info *)this );
#endif /* !MACOSX_DHD */
		NoCard= TRUE;
	}

	// Now that the controller is enabled, wake the power thread.
#ifdef AAPLPWROFF
	if( _up && _powerSleep ) {
		ASSERT(!_down);
		getCommandGate()->commandWakeup( this, true );
	}
#endif /* AAPLPWROFF */

	return err;
}


#ifndef MACOSX_DHD
__BEGIN_DECLS

void
wl_devicerecovery(struct wl_info *wl)
{
	errno_t err = 0;
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;
	struct wlc_info *wlc = wlobj ? wlobj->pWLC : NULL;
	IOPCIDevice *device = wlobj ? wlobj->getPCIDevice() : NULL;

	WL_ERROR(("%s: wl[%p] wlobj[%p] wlc[%p] device[%p]\n", __FUNCTION__,
		OSL_OBFUSCATE_BUF(wl), OSL_OBFUSCATE_BUF(wlobj),
		OSL_OBFUSCATE_BUF(wlc), OSL_OBFUSCATE_BUF(device)));

	require_action(wlc,    exit, err = -1);
	require_action(wlobj,  exit, err = -1);
	require_action(device, exit, err = -1);

	wlobj->doDeviceRecovery(device, NULL);

exit:
	return;
}
__END_DECLS


static void wl_nocard_timer(void *arg);

static void
wl_recover_nocard(struct wl_info *wl)
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;

	if (!wlobj) {
		IOLog("wl_recover_nocard: NULL wl\n");
		return;
	}

	if (wlobj->nocard_timer) {
		IOLog("wl_recover_nocard: Timer already started\n");
		return;
	}

	if (!(wlobj->nocard_timer = wl_init_timer(wl, wl_nocard_timer, (void *)wl, "nocard"))) {
		IOLog("wl_init_timer for nocard failed\n");
		return;
	}

	/* start a one shot timerto fire after 1 sec*/
	wl_add_timer(wl, wlobj->nocard_timer, 1000, FALSE);
}

static void
wl_nocard_timer(void *arg)
{
	struct wl_info *wl = (struct wl_info *)arg;
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;
	AirPort_Brcm43xx *drv = (AirPort_Brcm43xx*)arg;
	if (wlobj->nocard_timer) {
		wl_free_timer(wl, wlobj->nocard_timer);
		wlobj->nocard_timer = NULL;
	}

	drv->wl_nocard_war();
}

void
AirPort_Brcm43xx::wl_nocard_war(void)
{

	kprintf("In wl_nocard WAR function\n");


	//Do power cycle
	if(NoCard)
	{
		struct apple80211_power_data pd = { APPLE80211_VERSION, 0, {0} };

		pd.num_radios = ( phytype == PHY_TYPE_N ) ? 2 : 1;

		// First power off...then power on
		pd.power_state[0] = APPLE80211_POWER_OFF;
		if( pd.num_radios == 2 )
			pd.power_state[1] = APPLE80211_POWER_OFF;

		WL_PRINT(("%s: No card detected!  Trying to recover, setPOWER(OFF)\n", __FUNCTION__));

		SInt32 err = setPOWER( (IO80211Interface *)myNetworkIF, &pd );

		kprintf( "AirPort_Brcm43xx::powerOffThread: setPOWER(OFF) returned %d.\n", (int)err );

		OSL_DELAY(110000);

		pd.power_state[0] = APPLE80211_POWER_ON;
		if( pd.num_radios == 2 )
			pd.power_state[1] = APPLE80211_POWER_ON;

		WL_PRINT(("%s: No card detected!  Trying to recover, setPOWER(ON)\n", __FUNCTION__));

		// Apple Change: As a side effect of the fix for <rdar://problem/6394678>, this request
		//				 was not being honored. Adding the _powerOffThreadRequest flag fixes that.
		_powerOffThreadRequest = true;
		err = setPOWER( (IO80211Interface *)myNetworkIF, &pd );
		_powerOffThreadRequest = false;
		kprintf( "AirPort_Brcm43xx::powerOffThread: setPOWER(ON) returned %d.\n", (int)err );

		NoCard = FALSE;

		return;
	}


}
#endif /* !MACOSX_DHD */

#ifdef IO80211P2P
SInt32
AirPort_Brcm43xx::enableVirtualInterface( IO80211VirtualInterface * vif )
{
	SInt32 ret = super::enableVirtualInterface( vif );

	if( ret == 0 )
	{
		wlc_bsscfg_enable(pWLC, wl_find_bsscfg(((AirPort_Brcm43xxP2PInterface *)vif)->wlcif));
		vif->startOutputQueues();
	}

	return ret;
}

SInt32
AirPort_Brcm43xx::disableVirtualInterface( IO80211VirtualInterface * vif )
{
	SInt32 ret = super::disableVirtualInterface( vif );

	if( ret == 0 )
	{
		vif->stopOutputQueues();
		wlc_bsscfg_disable(pWLC, wl_find_bsscfg(((AirPort_Brcm43xxP2PInterface *)vif)->wlcif));
	}

	return ret;
}
#endif /* IO80211P2P */

IOReturn
AirPort_Brcm43xx::disable( IONetworkInterface * aNetif )
{
	IOReturn ret = 0;

	WL_IOCTL(("wl%d: disable\n", unit));

	KERNEL_DEBUG_CONSTANT(WiFi_init_disable | DBG_FUNC_NONE, 0, 0, 0, 0, 0);

#ifdef IO80211P2P
	if( OSDynamicCast( IO80211P2PInterface, aNetif ) )
		return super::disable( aNetif );
#endif

	(void)inputQueue->disable();

#ifdef MACOS_AQM
	// If AQM supported, then stop output thread
	if( ((IO80211Interface *)aNetif)->aqmSupported() ) /* AQM enabled */
	{
		WL_IOCTL(("wl%d: AQM: ::disable stopOutputThread & flushOutputQueue\n", unit));

		aNetif->stopOutputThread(); // stop packets going from the Q to the driver
		aNetif->flushOutputQueue(); // flush packets in the Q
	}
#endif /* MACOS_AQM */

	// Apple change: When the IFF_UP flag is cleared on an interface,
	// like when doing an 'ifconfig enX down', disable may be called
	// on two different threads simultaneously. One disable call is
	// made in response to the IFF_UP flag being cleared (this blocks
	// in commandSleep() waiting for the device to power down). A second
	// disable call will come in in response to the driver lowering its
	// power state. The second disable call can return immediately.
	if( !((IO80211Interface *)aNetif)->enabledBySystem() )
		return kIOReturnSuccess;
#ifdef MACOSX_DHD
	ret = super::disable( aNetif );
#else
	if (BUSTYPE(revinfo.bus) == PCI_BUS)
		si_pci_sleep(pub->sih);
	ret = super::disable( aNetif );
	if (pub && _down)
		pub->hw_up = FALSE;
#endif /* MACOSX_DHD */

	timeNow = 0;
	// Now that the controller is disabled, wake the power thread
#ifdef AAPLPWROFF
	if( _down && _powerSleep ) {
		ASSERT(!_up);
		getCommandGate()->commandWakeup( this, true );
	}
#endif /* AAPLPWROFF */

	return ret;
}

// -----------------------------------------------------------------------------
IOReturn
AirPort_Brcm43xx::registerWithPolicyMaker( IOService * policyMaker )
{
	const chip_power_levels_t *pwr = NULL;
	IOPMPowerState power_states[PS_NUM_STATES] = {};
	wlc_rev_info_t	revinfo = {};
	int num = 0;
	int rev = 0;
	int err = BCME_OK;

	memset(&revinfo, 0, sizeof(revinfo));

	err = wlIoctl(WLC_GET_REVINFO, &revinfo, sizeof(revinfo), FALSE, NULL);
	if (!err) {
		num = revinfo.chipnum;
		rev = revinfo.chiprev;
	}

	ASSERT(sizeof(power_state_template) == sizeof(power_states));

	// copy the template
	bcopy(power_state_template, power_states, sizeof(power_states));

	// update the power state array with the particular hardware power consumption numbers
	pwr = chip_power_lookup(num, rev);

	power_states[PS_INDEX_OFF].staticPower = pwr->down;
	power_states[PS_INDEX_DOZE].staticPower = pwr->down;
	power_states[PS_INDEX_ON].staticPower = pwr->idle_pm0;

	// register ourselves with the policy-maker
	IOReturn res = policyMaker->registerPowerDriver(this, power_states, PS_NUM_STATES);
	if (res)
		WL_ERROR(("wl%d: registerPowerDriver err = %d\n", unit, res));

	return res;
}

IOReturn
AirPort_Brcm43xx::powerStateWillChangeTo(
	IOPMPowerFlags newPowerStateFlags, unsigned long newPowerState, IOService* theDriver)
{
	KERNEL_DEBUG_CONSTANT(WiFi_power_powerStateWillChangeTo | DBG_FUNC_NONE, newPowerState, 0, 0, 0, 0);

	BCM_REFERENCE(newPowerStateFlags);
	BCM_REFERENCE(theDriver);

	_powerStateWillChangeToTimeStamp = OSL_SYSUPTIME_US64();

	kprintf( "AirPort_Brcm43xx::powerStateWillChangeTo: %d, timestamp[0x%016llx]\n", (int)newPowerState,
		 _powerStateWillChangeToTimeStamp );

	WL_INFORM(("wl%d: %s: PowerFlags 0x%04x new state %d theDriver %p\n",
	           unit, __FUNCTION__,
	           (uint)newPowerStateFlags, (uint)newPowerState,
		   OSL_OBFUSCATE_BUF(theDriver)));

	/* Outside of workloop, so do atomic increment */
	if( newPowerState == PS_INDEX_DOZE)
		OSIncrementAtomic( &entering_sleep );

#ifdef AAPLPWROFF
	// powerStateWillChangeTo is called without the workloop lock held, increment
	// _powerOffInProgress safely.
	if( newPowerState == PS_INDEX_DOZE || newPowerState == PS_INDEX_OFF )
		OSIncrementAtomic( &_powerOffInProgress );

	kprintf( "AirPort_Brcm43xx::powerStateWillChangeTo: %d, ACK'ing\n", (int)newPowerState );
	WL_PCIE(("AirPort_Brcm43xx::powerStateWillChangeTo: %d, ACK'ing\n", (int)newPowerState));

	return kIOPMAckImplied;
#else
	return 0;
#endif
}

static void
IOPCILogDevice(IOPCIDevice * device, const char *log)
{
	char     pathBuf[PCI_CONFIG_SPACE_SIZE];
	int      len;
	uint32_t  offset, data = 0;

	if (WL_PCIE_ON() == 0)
		return;
	len = sizeof(pathBuf);
	if (device->getPath( pathBuf, &len, gIOServicePlane))
        WL_PRINT(("%s : ints(%d) %s\n", log, ml_get_interrupts_enabled(), pathBuf));
	WL_PRINT(("         0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F"));
	for (offset = 0; offset < PCI_CONFIG_SPACE_SIZE; offset++) {
		if( 0 == (offset & 3))
			data = device->configRead32(offset);
		if( 0 == (offset & 15))
            WL_PRINT(("\n    %03X:", offset));
        WL_PRINT((" %02x", data & 0xff));
		data >>= 8;
	}
	WL_PRINT(("\n"));
}

#ifdef AAPLPWROFF
IOReturn
AirPort_Brcm43xx::powerStateDidChangeTo (
                    IOPMPowerFlags  capabilities,
                    unsigned long   stateNumber,
                    IOService*      whatDevice)
{
	KERNEL_DEBUG_CONSTANT(WiFi_power_powerStateDidChangeTo | DBG_FUNC_NONE, stateNumber, 0, 0, 0, 0);

	BCM_REFERENCE(capabilities);
	BCM_REFERENCE(whatDevice);

	_powerStateDidChangeToTimeStamp = OSL_SYSUPTIME_US64();

	kprintf( "AirPort_Brcm43xx::powerStateDidChangeTo: %d, timestamp[0x%016llx]\n", (int)stateNumber,
		 _powerStateDidChangeToTimeStamp );
	WL_PCIE(("AirPort_Brcm43xx::powerStateDidChangeTo: %d, timestamp[0x%016llx] calendartime[0x%016llx]\n", (int)stateNumber,
		 _powerStateDidChangeToTimeStamp, OSL_CALENDARTIME_US64()));

	// powerStateDidChangeTo is called without the workloop lock held, decrement
	// _powerOffInProgress safely.
	if( stateNumber == PS_INDEX_DOZE || stateNumber == PS_INDEX_OFF )
		OSDecrementAtomic( &_powerOffInProgress );

	// Apple change: run sync power state when power state did change to catch
	//				 the case where the system wakes, powers on the device,
	//				 and the driver can check if the PCI device caused the wake event
	if( stateNumber == PS_INDEX_ON )
	{
		int state = SYNC_STATE_DID_CHANGE;
		getCommandGate()->runAction( AirPort_Brcm43xx::syncPowerStateGated, &stateNumber, &state, NULL, NULL );
	}

	kprintf( "AirPort_Brcm43xx::powerStateDidChangeTo: %d, ACK'ing\n", (int)stateNumber );
	WL_PCIE(("AirPort_Brcm43xx::powerStateDidChangeTo: %d, ACK'ing\n", (int)stateNumber));

	return kIOPMAckImplied;
}

static void _powerOffThread( thread_call_param_t param0, thread_call_param_t param1 )
{
	AirPort_Brcm43xx * controller = (AirPort_Brcm43xx *)param0;

	BCM_REFERENCE(param1);

	OSIncrementAtomic( &controller->_powerOffThreadRunning );
	controller->getCommandGate()->runAction( AirPort_Brcm43xx::powerOffThreadGated );
	OSDecrementAtomic( &controller->_powerOffThreadRunning );
}

// static
IOReturn
AirPort_Brcm43xx::powerOffThreadGated( OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3 )
{
	BCM_REFERENCE(arg0);
	BCM_REFERENCE(arg1);
	BCM_REFERENCE(arg2);
	BCM_REFERENCE(arg3);

	((AirPort_Brcm43xx *)owner)->WL_LOCK();
	((AirPort_Brcm43xx *)owner)->powerOffThread();
	((AirPort_Brcm43xx *)owner)->WL_UNLOCK();

	return kIOReturnSuccess;
}

void
AirPort_Brcm43xx::powerOffThread()
{
	struct apple80211_power_data pd = { APPLE80211_VERSION, 0, {0} };

	pd.num_radios = num_radios;

	// First power on...then power off
	for (uint i = 0; i < num_radios; i++)
		pd.power_state[i] = APPLE80211_POWER_ON;

	kprintf( "AirPort_Brcm43xx::powerOffThread: calling setPOWER(ON)\n" );

	SInt32 err = setPOWER( myNetworkIF, &pd );

	kprintf( "AirPort_Brcm43xx::powerOffThread: setPOWER(ON) returned %d.\n", (int)err );

	for (uint i = 0; i < num_radios; i++)
		pd.power_state[i] = APPLE80211_POWER_OFF;

	// Apple Change: As a side effect of the fix for <rdar://problem/6394678>, this request
	//				 was not being honored. Adding the _powerOffThreadRequest flag fixes that.
	_powerOffThreadRequest = true;

	kprintf( "AirPort_Brcm43xx::powerOffThread: calling setPOWER(OFF)\n" );

	err = setPOWER( myNetworkIF, &pd );
	_powerOffThreadRequest = false;
	kprintf( "AirPort_Brcm43xx::powerOffThread: setPOWER(OFF) returned %d.\n", (int)err );
}

#if (VERSION_MAJOR >= 14) && defined(ENABLE_PMROOT_claimSystemWakeEvent)
#define WAKE_REASON_PREFIX		"WiFi."
#define WAKE_REASON_PREFIX_LENGTH	5

static void wakeLog( const char * reason1, const char * reason2, OSData * wakeReasonBuffer )
{
	IOLog( "ARPT: Wake Reason: %s\n", reason1 );
	IO8Log( "ARPT: Wake Reason: %s\n", reason1 );

	if( wakeReasonBuffer->getLength() > WAKE_REASON_PREFIX_LENGTH )
		wakeReasonBuffer->appendByte( '+', 1 );

	wakeReasonBuffer->appendBytes( reason2, strlen( reason2 ) );
}
#endif /* VERSION_MAJOR >= 14 && ENABLE_PMROOT_claimSystemWakeEvent */

void
AirPort_Brcm43xx::checkWakeIndication()
{
	wl_wowl_wakeind_t wake = {};
	int err = BCME_OK;

	err = wlIovarOp( "wowl_wakeind",
	                 NULL,
	                 0,
	                 &wake,
	                 sizeof( wake ),
	                 IOV_GET,
	                 NULL );

	if( err )
		return;
#ifdef WLTCPKEEPA
	if ((wake.ucode_wakeind & WL_WOWL_TCPKEEP_DATA) != 0) {
		uchar *dump_buf;
		wake_info_t *wake_pkt_info;
		wake_pkt_t *wake_pkt;
		void *sdu, *frame;
		wlc_pkttag_t *pkttag;

		dump_buf = (uint8*)MALLOC(osh, TCPKEEP_PKT_INFO_MAX_SZ);
		if (!dump_buf)
			return;

		memset(dump_buf, 0, TCPKEEP_PKT_INFO_MAX_SZ);

		if ((err = wlIovarOp("wowl_pkt_info", NULL, 0, dump_buf,
		    TCPKEEP_PKT_INFO_MAX_SZ, IOV_GET, NULL)) < 0) {
                        MFREE(osh, dump_buf, TCPKEEP_PKT_INFO_MAX_SZ);
			return;
		}

		wake_pkt_info = (wake_info_t *) dump_buf;
		wake_pkt = (wake_pkt_t *) &dump_buf[wake_pkt_info->wake_info_len];

		sdu = PKTGET(osh, wake_pkt->wake_pkt_len, FALSE);
		if (sdu == NULL) {
			MFREE(osh, dump_buf, TCPKEEP_PKT_INFO_MAX_SZ);
			return;
		}
		frame = PKTDATA(osh, sdu);
		bcopy(&wake_pkt->packet, frame,  wake_pkt->wake_pkt_len);

		MFREE(osh, dump_buf, TCPKEEP_PKT_INFO_MAX_SZ);

		pkttag = WLPKTTAG(sdu);
		if (pkttag == NULL) {
			PKTFREE(osh, sdu, FALSE);
			return;
		}
		wl_sendup((struct wl_info*)this, NULL, sdu, 1);
	}
#endif /* WLTCPKEEPA */

#ifdef SUPPORT_FEATURE_WOW
	if( wake.pci_wakeind )
		setProperty( APPLE80211_REGKEY_CAUSED_WAKE, true );
#endif

	if (wake.ucode_wakeind != 0) {

#if (VERSION_MAJOR >= 14) && defined(ENABLE_PMROOT_claimSystemWakeEvent)

	#define WAKE_LOG( reasonStr1, reasonStr2 ) wakeLog( reasonStr1, reasonStr2, wakeReasonBuffer );

		OSData * wakeReasonBuffer = OSData::withCapacity( 0 );
		if( !wakeReasonBuffer )
			return;
		wakeReasonBuffer->appendBytes( WAKE_REASON_PREFIX, WAKE_REASON_PREFIX_LENGTH );

#else /* VERSION_MAJOR < 14 || ENABLED_PMROOT_claimSystemWakeEvent */

	#define WAKE_LOG( reasonStr1, reasonStr2 ) { IOLog( "ARPT: Wake Reason: " reasonStr1 "\n" ); IO8Log( "ARPT: Wake Reason: " reasonStr1 "\n" ); }

#endif /* VERSION_MAJOR >= 14 && ENABLED_PMROOT_claimSystemWakeEvent */

		if ((wake.ucode_wakeind & WL_WOWL_MAGIC) == WL_WOWL_MAGIC)
			WAKE_LOG("MAGIC packet received", "MagicPacket")
		if ((wake.ucode_wakeind & WL_WOWL_NET) == WL_WOWL_NET)
			WAKE_LOG("Packet received with Netpattern", "NetpatternPacket")
		if ((wake.ucode_wakeind & WL_WOWL_DIS) == WL_WOWL_DIS)
			WAKE_LOG("Disassociation/Deauth received", "Disassoc/Deauth")
		if ((wake.ucode_wakeind & WL_WOWL_RETR) == WL_WOWL_RETR)
			WAKE_LOG("Retrograde TSF detected", "RetrogradeTSF")
		if ((wake.ucode_wakeind & WL_WOWL_BCN) == WL_WOWL_BCN)
			WAKE_LOG("Beacons Lost", "BeaconLost")
		if ((wake.ucode_wakeind & WL_WOWL_TST) == WL_WOWL_TST)
			WAKE_LOG("Test Mode", "TestMode")
		if ((wake.ucode_wakeind & WL_WOWL_M1) == WL_WOWL_M1)
			WAKE_LOG("PTK Refresh received", "PTKRefresh")
		if ((wake.ucode_wakeind & WL_WOWL_EAPID) == WL_WOWL_EAPID)
			WAKE_LOG("EAP-Identity request received", "EAPIdentityRequest")
		if ((wake.ucode_wakeind & WL_WOWL_GTK_FAILURE) == WL_WOWL_GTK_FAILURE)
			WAKE_LOG("Wake on GTK failure", "GTKFailure")
		if ((wake.ucode_wakeind & WL_WOWL_EXTMAGPAT) == WL_WOWL_EXTMAGPAT)
			WAKE_LOG("Extended Magic Packet received", "ExtendedMagicPacket")
		if ((wake.ucode_wakeind & WL_WOWL_KEYROT) == WL_WOWL_KEYROT)
			WAKE_LOG("Key Rotation Packet received", "KeyRotationPacket")
		if ((wake.ucode_wakeind & (WL_WOWL_NET | WL_WOWL_MAGIC | WL_WOWL_EXTMAGPAT))) {
			if ((wake.ucode_wakeind & WL_WOWL_BCAST) == WL_WOWL_BCAST) {
				WAKE_LOG("Broadcast/Mcast frame received", "Bcast/McastFrame")
			} else {
				WAKE_LOG("Unicast frame received", "UnicastFrame")
			}
		}
#undef WAKE_LOG

#if (VERSION_MAJOR >= 14) && defined(ENABLE_PMROOT_claimSystemWakeEvent)
	// Make sure at least the null terminator got appended
	if( wakeReasonBuffer->appendByte( '\0', 1 ) )
		getPMRootDomain()->claimSystemWakeEvent(this, kIOPMWakeEventSource, (char*)wakeReasonBuffer->getBytesNoCopy() );

	wakeReasonBuffer->release();
#endif /* VERSION_MAJOR >= 14 && ENABLE_PMROOT_claimSystemWakeEvent */
	}
}

void
AirPort_Brcm43xx::checkInterfacePowerState()
{
	IO80211Interface * interface = (IO80211Interface *)myNetworkIF;

	if( ( interface->getFlags() & IFF_UP ) && interface->poweredOnByUser() )
		return;

	printf( "AirPort_Brcm43xx::checkInterfacePowerState: Check _pwrOffThreadCall!\n" );

	if( !_pwrOffThreadCall )
	{
		// No need to check for failure..thread_call_allocate will panic if it fails
		_pwrOffThreadCall = thread_call_allocate( _powerOffThread, this );
	}

	if( !_powerOffThreadRunning )
	{
		kprintf( "AirPort_Brcm43xx::checkInterfacePowerState: Entering _pwrOffThreadCall!\n" );
		thread_call_enter( _pwrOffThreadCall );
	}
}

#if VERSION_MAJOR > 10
void
AirPort_Brcm43xx::SecondaryBusReset(IOPCIDevice * device)
{
	uint16 bridgeControl = 0;
	IOPCIBridge *parent = NULL;

	if (device == NULL) {
		kprintf("%s: device == NULL\n", __FUNCTION__);
		return;
	}

	if (BUSTYPE(revinfo.bus) != PCI_BUS) {
		return;
	}
	if( !(parent = (IOPCIBridge *)(device->getParentEntry(gIOServicePlane))) ) {
		return;
	}

	WL_PCIE(("AirPort_Brcm43xx::SecondaryBusReset: Issuing PCIe secondary bus reset, device[%p] parent[%p]\n",
		OSL_OBFUSCATE_BUF(device),OSL_OBFUSCATE_BUF(parent)));

	//Issue secondary bus reset, Bit 6 is secondary bus reset
	bridgeControl = OSL_PCI_READ_CONFIG_PARENT(osh, parent, kPCI2PCIBridgeControl, 2);
	bridgeControl |= (1<<6);
	OSL_PCI_WRITE_CONFIG_PARENT(osh, parent, kPCI2PCIBridgeControl, 2, bridgeControl);
	OSL_DELAY(10000);	//reset for 10ms */

	// De-assert secondary bus reset
	bridgeControl &= ~(1<<6);
	OSL_PCI_WRITE_CONFIG_PARENT(osh, parent, kPCI2PCIBridgeControl, 2, bridgeControl);
	OSL_DELAY(10000);	//delay for 10ms */
	WLCNTINCR(pub->_cnt->pciereset);
}

IOReturn AirPort_Brcm43xx::restoreDeviceState( IOPCIDevice *device )
{
	int i = 0;

	WL_PCIE(("%s: device[%p]\n", __FUNCTION__, OSL_OBFUSCATE_BUF(device)));
	for (i = 0; i < PCI_CONFIG_SPACE_SIZE; i += 4) {
		device->configWrite32( i, _pciSavedConfig[i>>2] );
	}

	WLCNTINCR(pub->_cnt->cfgrestore);
	return kIOReturnSuccess;
}

IOReturn AirPort_Brcm43xx::saveDeviceState( IOPCIDevice *device, int override )
{
	int i = 0;
	uint32 data = 0;

	WL_PCIE(("%s: device[%p], override[%d]\n", __FUNCTION__, OSL_OBFUSCATE_BUF(device), override));
	for( i = 0; i < PCI_CONFIG_SPACE_SIZE; i += 4 ) {
		_pciSavedConfig[i>>2] = device->configRead32( i );
	}

	WL_PCIE(("%s: dump of cached config-space\n", __FUNCTION__));
	for (i = 0; i < PCI_CONFIG_SPACE_SIZE; i++) {
		if( 0 == (i & 3))
			data = device->configRead32(i);
		if( 0 == (i & 15))
			WL_PCIE(("\n    %02X:", i));
		WL_PCIE((" %02x", data & 0xff));
		data >>= 8;
	}
	WL_PCIE(("\n"));

    return kIOReturnSuccess;
}

#define MAX_PCI_LINKLIST	10
IOReturn AirPort_Brcm43xx::configValidChk( IOPCIDevice *device )
{
	uint8 id = 0, nextptr = 0, cap = 0;
	uint8 *ptr = (uint8 *)&_pciSavedConfig[PCI_CFG_CAPPTR>>2];
	uint loops = 0;

	WL_PCIE(("%s: device[%p] nextptr 0x%x\n", __FUNCTION__, OSL_OBFUSCATE_BUF(device), *ptr));
	/* point to first cap ID in link list */
	ptr = (uint8 *)&_pciSavedConfig[*ptr>>2];
	for (loops = 0; loops < MAX_PCI_LINKLIST; loops++) {
		id = *(ptr);
		nextptr = *(ptr+1);
		cap = *(ptr+2);
		switch (id) {
			case PCI_CAP_POWERMGMTCAP_ID:
				WL_PCIE(("%s: pwrcap: 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
					id, nextptr, cap));
				break;
			case PCI_CAP_MSICAP_ID:
				WL_PCIE(("%s: msicap: 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
					id, nextptr, cap));
				/* check to makesure MSI is enabled */
				if ((cap & MSI_ENABLE) == 0) {
					WL_PCIE(("%s: set MSI_ENABLE\n", __FUNCTION__));
					*(ptr+2) |= MSI_ENABLE;
				}
				break;
			case PCI_CAP_PCIECAP_ID:
				WL_PCIE(("%s: pcicap: 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
					id, nextptr, cap));
				break;
			default:
				WL_PCIE(("%s: unknown: 0x%02x 0x%02x 0x%02x\n", __FUNCTION__,
					id, nextptr, cap));
				break;
		}
		/* found end of cap link list */
		if (nextptr == 0)
			break;
		ptr = (uint8 *)&_pciSavedConfig[nextptr>>2];
	}
	if (loops == MAX_PCI_LINKLIST) {
		WL_PCIE(("%s: Possible more capability or corrupted config-space\n", __FUNCTION__));
	}
	return kIOReturnSuccess;
}


#define PCI_DEVICEDELAY_USEC                    200000   /* useconds */

#define CHECKDEVICEMASK_VENDORID                (1<<0)
#define CHECKDEVICEMASK_BAR0                    (1<<4)

#define PCI_ISBARVALID(_bar)                  ((((_bar) != 0x4) && ((_bar) != 0xffffffff)) ? 1 : 0)

bool
AirPort_Brcm43xx::checkMaskIsDeviceValid(IOPCIDevice *device, IOPCIBridge *parent, uint32 checkmask)
{
	uint32 validmask = 0;
	uint32 valid = 0;
	uint16 vendorId = 0;
	uint32 bar0 = 0;
	BCM_REFERENCE(device);
	BCM_REFERENCE(parent);

	if ((checkmask & CHECKDEVICEMASK_VENDORID)) /* check vendor ID */
	{
		valid = ((vendorId = device->configRead16(kIOPCIConfigVendorID)) == VENDOR_BROADCOM) ? 1 : 0;
		validmask  = valid ? (validmask | (CHECKDEVICEMASK_VENDORID)) : validmask;

		if (!valid)
		{
			 WL_PCIE(("BRCM: IsDeviceValid: Check failed, "
				"vendorID!   VendorId[%04x] != %04x\n",
				vendorId, VENDOR_BROADCOM));
		}
	}

	if ((checkmask & CHECKDEVICEMASK_BAR0)) /* check BAR0 */
	{
		bar0       = device->configRead32(kIOPCIConfigBaseAddress0);
		valid      = PCI_ISBARVALID(bar0) ? 1 : 0;
		validmask  = valid ? (validmask | (CHECKDEVICEMASK_BAR0)) : validmask;

		if (!valid)
		{
			WL_PCIE(("BRCM: IsDeviceVaild: Check failed, BAR0!   BAR0[0x%08x]\n", bar0));
		}
	}

	/* Must exactly match checkmask */
	valid = ((validmask & checkmask) == checkmask) ? 1 : 0;

	return(valid);
}

IOReturn
AirPort_Brcm43xx::doDeviceRecovery(IOPCIDevice *device, IOPCIBridge *parent)
{
	WL_PCIE(("%s: Issuing PCIe secondary bus reset\n", "AirPort_Brcm43xx::doDeviceRecovery"));

	BCM_REFERENCE(parent);

	this->SecondaryBusReset(device);

	WL_PCIE(("%s: Delaying %4d msec after PCIe secondary bus reset before probing the chip\n",
		"AirPort_Brcm43xx::doDeviceRecovery", (PCI_DEVICEDELAY_USEC/1000)));
	OSL_DELAY(PCI_DEVICEDELAY_USEC);

	// Restore PCI config, this ensures first access after reset are reads
	IOPCILogDevice(device, "BRCM: Device recovery, PCIe config space, before restore:");
	this->restoreDeviceState(device);
	IOPCILogDevice(device, "BRCM: Device recovery, PCIe config space,  after restore:");

	OSL_DELAY(20000);

	return kIOReturnSuccess;
}

IOReturn
AirPort_Brcm43xx::tryDeviceRecovery(IOPCIDevice *device, IOPCIBridge *parent,
	int checkrecovery, int retries, uint32 checkmask)
{
	int i = 0;

	if ((_enabledDeviceRecovery == false) && checkrecovery)
	{
		return kIOReturnError;
	}

	while ((i++ < retries))
	{
		if (checkMaskIsDeviceValid(device, parent, checkmask))
		{
			return kIOReturnSuccess;
		}

		WL_PCIE(("%s: Failed PCIe device check, attempt[%3d]!  "
			"Trying device recovery\n",
                        "AirPort_Brcm43xx::tryDeviceRecovery", i));
		doDeviceRecovery(device, parent);
	}

	return kIOReturnError;
}


IOReturn
AirPort_Brcm43xx::configHandler( void * ref, IOMessage message, IOPCIDevice * device, uint32_t state )
{
	IOPCIBridge *parent;
	AirPort_Brcm43xx *wlObj = ((AirPort_Brcm43xx *)ref);
	uint16 vendorId;
	uint32 bar0;
	IOByteCount capa = 0;
	int flags = 0;

	if (wlObj == NULL || device == NULL)
		return kIOReturnError;

	WL_PCIE(("%s: PCIe config handler: [%s], message[0x%x] state[%x] timestamp[0x%016llx] calendartime[0x%016llx]\n",
		"AirPort_Brcm43xx::configHandler", device->getName(),
			  (int) message, state, (wlObj ? wlObj->_configHandlerTimeStamp : 0), OSL_CALENDARTIME_US64() ));
	wlObj->_configHandlerTimeStamp = OSL_SYSUPTIME_US64();

	parent = (IOPCIBridge *)(device->getParentEntry(gIOServicePlane));

	if(message == kIOMessageDeviceWillPowerOn) {
		//Add 100 msec delay for X9 based chipsets where restore may happen too early due to EFI issue

		// Check the device ID, if device id == (0x4331 || 0x4353), then delay
		// Please refer to the canLoad method, for device ID's
		if((wlObj->_pciDeviceId == BCM4331_D11N_ID)
		    || (wlObj->_pciDeviceId == BCM43224_D11N_ID)
		   )
		{
			WL_PCIE(("Delaying %4d msec before probing the chip\n", (PCI_DEVICEDELAY_USEC/1000)));
			OSL_DELAY(PCI_DEVICEDELAY_USEC);
		}

		vendorId = device->configRead16(kIOPCIConfigVendorID);

		WL_PCIE(("kIOMessageDeviceWillPowerOn: vendorid[0x%04x]\n", vendorId));

		if(vendorId != VENDOR_BROADCOM) {
			// Display to system.log
			printf(  "VendorID check failed! vendorId[%04x] != %04x, issuing PCIe"
				" secondary bus reset\n\n",
				vendorId, VENDOR_BROADCOM );

			// Display when PCIE logging enabled
			WL_PCIE(( "VendorID check failed! vendorId[%04x] != %04x, issuing PCIe"
				" secondary bus reset\n\n",
				vendorId, VENDOR_BROADCOM ));

			//Add delay for X9 based chipsets where restore may happen too early due to EFI issue
			//Shouldn't hurt on other platforms

			wlObj->tryDeviceRecovery(device, parent, true, 8,
						(CHECKDEVICEMASK_VENDORID));

			    vendorId = device->configRead16(kIOPCIConfigVendorID);
			if (vendorId == VENDOR_BROADCOM) {
				goto exit;
			}

			// Failed vendor ID check
			WL_PCIE(("Failed vendorId[%04x] != %04x\n", vendorId, VENDOR_BROADCOM));
			WL_HEALTH_LOG(wlObj->pWLC, VENDORID_INVALID);
		}
	} else if(message == kIOMessageDeviceHasPoweredOn) {
		//<rdar://problem/9206182> NOCARD: Can't turn Wi-Fi back on
		bar0 = device->configRead32(kIOPCIConfigBaseAddress0);

		WL_PCIE(("kIOMessageDeviceHasPoweredOn: bar0[0x%08x]\n", bar0));

		if ((device->getInterruptType(1, &flags) == kIOReturnSuccess) &&
		    (flags & kIOInterruptTypePCIMessaged) &&
		    (device->extendedFindPCICapability(kIOPCIMSICapability, &capa)) &&
		    (capa != 0))
		{
			// Valid MSI
			WL_PCIE(("MSI capability enabled device, offset[0x%02llx]\n", capa));
		}

		uint16 linkStatus = device->configRead16(0xe2);
		uint32 unCorrErrors = device->extendedConfigRead32(0x104);
		uint32 corrErrors = device->extendedConfigRead32(0x110);

			//Print Link status and error registers
		WL_PCIE(("LinkStatus[0x%04x], Uncorrectible Errors[0x%08x], Correctible Errors[0x%08x]\n",
			 linkStatus, unCorrErrors, corrErrors));

		if ((bar0 == 0xffffffff) || (bar0 == 0x04)) {
			// Display to system.log
			printf( "Invalid bar0[%08x], issuing PCIe secondary "
				 "bus reset, attempting device recovery\n",
				bar0);

			// Display when PCIE logging enabled
			WL_PCIE(("Invalid bar0[%08x], issuing PCIe secondary "
				 "bus reset, attempting device recovery\n",
				 bar0));

			wlObj->tryDeviceRecovery(device, parent, true, 8,
				 (CHECKDEVICEMASK_VENDORID|CHECKDEVICEMASK_BAR0));

		    //Read bar0 after configuration
		    bar0 = device->configRead32(kIOPCIConfigBaseAddress0);
			WL_PCIE(("bar0[0x%08x] after attempting device recovery\n", bar0));

			if ((bar0 == 0xffffffff) || (bar0 == 0x04))
			    WL_HEALTH_LOG(wlObj->pWLC, BAR0_INVALID);

		    //Clear No card status after this restore attempt
		    NoCard = FALSE;
		}
		/* Restore the PCI_INT_MASK if it is not same as one before power off */
		uint32 temp_pci_int_mask = OSL_PCI_READ_CONFIG(wlObj->osh, PCI_INT_MASK, sizeof(uint32));
		if (wlObj->pci_int_mask && (temp_pci_int_mask != wlObj->pci_int_mask)) {
			WL_PCIE(("Restore PCI_INT_MASK: Curr:%x, New:%x\n", temp_pci_int_mask, wlObj->pci_int_mask));
			OSL_PCI_WRITE_CONFIG(wlObj->osh, PCI_INT_MASK, sizeof(uint32), wlObj->pci_int_mask);
		}
	}
	else if(message == kIOMessageDeviceWillPowerOff) {
		WL_PCIE(("kIOMessageDeviceWillPowerOff\n"));
		/* OS does not save/restore non standard PCI config space registers.
		 * So, Store the PCI_INT_MASK register before power off */
		wlObj->pci_int_mask = OSL_PCI_READ_CONFIG(wlObj->osh, PCI_INT_MASK, sizeof(uint32));
	}
	else if(message == kIOMessageDeviceHasPoweredOff) {
		WL_PCIE(("kIOMessageDeviceHasPoweredOff\n"));
	}

exit:
    return kIOReturnSuccess;
}
#endif /* VERSION_MAJOR > 10 */

IOReturn
AirPort_Brcm43xx::setPowerState( unsigned long powerStateOrdinal, IOService * whatDevice )
{
	KERNEL_DEBUG_CONSTANT(WiFi_power_powerStateDidChangeTo | DBG_FUNC_NONE, powerStateOrdinal, 0, 0, 0, 0);

	BCM_REFERENCE(whatDevice);

	_setPowerStateTimeStamp = OSL_SYSUPTIME_US64();

	kprintf( "AirPort_Brcm43xx:setPowerState: Called powerStateOrdinal = %d, timestamp[0x%016llx]\n", (int)powerStateOrdinal,
		 _setPowerStateTimeStamp );

	WL_PCIE(("AirPort_Brcm43xx:setPowerState: Called powerStateOrdinal = %d, timestamp[0x%016llx] calendartime[0x%016llx]\n", (int)powerStateOrdinal,
		 _setPowerStateTimeStamp, OSL_CALENDARTIME_US64()));

	// Apple change: State selector added to syncPowerState so it can be called from multiple
	// power transition callbacks
	int state = SYNC_STATE_SET_POWER;
	getCommandGate()->runAction( AirPort_Brcm43xx::syncPowerStateGated, &powerStateOrdinal, &state, NULL, NULL );

	kprintf("AirPort_Brcm43xx::setPowerState: powerStateOrdinal = %d, ACK'ing.\n", (int)powerStateOrdinal );
	WL_PCIE(("AirPort_Brcm43xx::setPowerState: powerStateOrdinal = %d, ACK'ing.\n", (int)powerStateOrdinal));

	return kIOPMAckImplied;
}

// Enable default wake bits
void
enableDefaultWowlBits(struct wlc_info *pWLC)
{
	BCM_REFERENCE(pWLC);
	return;
}

// This function should be called in syncPowerState as the system is going to sleep.
// The algorithm is defined in <rdar://problem/14887884>. WoW can only be enabled when
// the system is on AC power. Scan offloads will be disabled when TCP keep-alive is
// not allowed by the power management policy.
bool
AirPort_Brcm43xx::leaveModulePoweredForOffloads()
{
	bool leavePowered = false;
	IO80211Interface * primaryInterface = getNetworkInterface();
#ifdef APPLE80211_IOC_OFFLOAD_SCAN
	struct apple80211_offload_scan_data osd = {};

	memset(&osd, 0, sizeof(osd));
	osd.version = APPLE80211_VERSION;

	SInt32 err = getOFFLOAD_SCAN( primaryInterface, &osd );
	require( err == 0, exit );

	leavePowered = ( osd.ssid_count || primaryInterface->wowEnabled() );

exit:

#else
	leavePowered = primaryInterface->wowEnabled();
#endif /* APPLE80211_IOC_OFFLOAD_SCAN */

	RELEASE_PRINT(("wl%d: %s: Wi-Fi will %s.\n",
			pub->unit, __FUNCTION__, leavePowered ? "stay on" : "turn off"));

	return leavePowered;
}

IOReturn
AirPort_Brcm43xx::syncPowerState( unsigned long powerStateOrdinal, int callback )
{
	kprintf("AirPort_Brcm43xx::syncPowerState: powerStateOrdinal = %d, callback[%x]"
		"systemWoke[%x] _powerSleep[%x] _powerState[%lx]\n",
		(int)powerStateOrdinal, callback, systemWoke, _powerSleep, _powerState );
	WL_PCIE(("AirPort_Brcm43xx::syncPowerState: powerStateOrdinal = %d, callback[%x]"
		"systemWoke[%x] _powerSleep[%x] _powerState[%lx]\n",
		(int)powerStateOrdinal, callback, systemWoke, _powerSleep, _powerState));

	// Apple change: State selector added to syncPowerState so it can be called from multiple
	//				 power transition callbacks
	if( callback == SYNC_STATE_DID_CHANGE )
	{
		// First time the card powers up after wake, check for the wake indication property.
		// IO80211Family will clear the wake indication property before the system sleeps the
		// next time.
		if( systemWoke && powerStateOrdinal == PS_INDEX_ON )
		{
			if( wowl_cap )
				checkWakeIndication();

			systemWoke = false;
		}
	}
	else
	{
		if( _powerSleep )
		{
			// If power is transitioning from on to off and the controller is not enabled, then there will be
			// no disable() callback to wake the power ioctl thread. Likewise, if power is transitioning from
			// off to on and the controller is enabled, there will be no enable callback. In either case the
			// thread requesting the power change must be woken here.

			IO80211Interface * interface = getNetworkInterface();
			bool enabled = interface->enabledBySystem();
			UInt16 interfaceFlags = interface->getFlags();

			if( ( powerStateOrdinal == PS_INDEX_ON && ( enabled || ( interfaceFlags & IFF_UP ) == 0 ) ) ||
			    ( powerStateOrdinal != PS_INDEX_ON && !enabled ) )
				{
					getCommandGate()->commandWakeup( this, true );
				}
		}

		if( _powerState != powerStateOrdinal )	{
			// This setPowerState callback was made in response to a system power event. Synchronize
			// the driver's state here.

			if( powerStateOrdinal == PS_INDEX_ON )	{
				uint32 status;
				pWLC->down_override = 0;
				pub->hw_off = false;

				/* Entry point from resume */
				// Only make it if WoWL is still somehow active at this point. There can be multiple
				//syncpowerstatecalls for the same transition that can wipe out wake reason
				if(WOWL_ACTIVE(pub))
				deviceWowlClr();

				if( checkCardPresent( TRUE, &status ) ) {
					wl_up( (struct wl_info *)this );
#ifdef WL_ASSOC_RECREATE
					if (!ASSOC_RECREATE_ENAB(pWLC->pub)) {
						/* WAR for legacy modules unable to create new association after
						 * ucode WOWL sleep due to AP putting replies in Power Save
						 * queue. Moved bsscfg disable here from WOWL clear path so
						 * disassoc in bsscfg_disable can go out and clear AP state.
						 * At this point primary bsscfg is disabled except the WAR case.
						 */
						if (pWLC->cfg && BSSCFG_STA(pWLC->cfg) && pWLC->cfg->enable)
							wlc_bsscfg_disable(pWLC, pWLC->cfg);
					}
#endif /* WL_ASSOC_RECREATE */

					NoCard = FALSE;
				} else {
					kprintf("syncPowerState:Setting NO card to true !\n");
					WL_HEALTH_LOG(pWLC, status);
					NoCard= TRUE;
				}
			} else	{
				int wowl_test_mode = 0;
				int wowl_user = 0;
				bool wowl_enab = FALSE;
				bool will_power_off = FALSE;

#if VERSION_MAJOR >= 15
				int sleep_type = getIOPMSystemSleepType(NULL, 0);

				if ((sleep_type == kIOPMSleepTypePowerOff) ||
					(sleep_type == kIOPMSleepTypeHibernate)) {
					WL_PCIE(("sleep_type:%d Skip enabling offloads\n", sleep_type));
					will_power_off = TRUE;
				}
#endif /* VERSION_MAJOR >= 15 */

#ifdef  WLTCPKEEPA
				wl_update_tcpkeep_seq(pWLC);
#endif
				enableDefaultWowlBits(pWLC);

				(void) wlIovarGetInt( "wowl_test", &wowl_test_mode );
				(void) wlIovarGetInt( "wowl", &wowl_user );

				/* in order to support scan offload in non-associated state
				 * driver required to enter into wowl mode
				 */
				if (wowl_test_mode || wowl_user) {
					if(!pub->hw_off) {
						wowl_enab = TRUE;
					}
				}
				pWLC->down_override = 1;

				/* If in Test mode, just enable WOWL regardless of other conditions */
				if (wowl_enab && (will_power_off == FALSE) && wlc_wowl_enable(pWLC->wowl)) {
					/* Enable PME in the pci config */
					if (BUSTYPE(revinfo.bus) == PCI_BUS)
						si_pci_pmeen(pub->sih);
#ifdef AAPLPWROFF
					platformWoWEnable(true);
#endif
				} else {
					if (!pub->hw_off)
						si_pcie_prep_D3(pub->sih, _systemSleeping);
					wl_down((struct wl_info *)this);
				}
				// Always send to system.log
				RELEASE_PRINT(( "AirPort_Brcm43xx::syncPowerState: WWEN[%s]\n", _wwenState ? "enabled" : "disabled" ));

				// Return to default client STA mode
				setAP(FALSE);
				setIBSS(FALSE);
				pub->hw_up = FALSE;
				pub->hw_off = true;
			}
		}

		_powerState = powerStateOrdinal;
	}

	kprintf( "AirPort_Brcm43xx::syncPowerState: returned: powerStateOrdinal = %d, callback[%x] systemWoke[%x] _powerSleep[%x] _powerState[%lx]\n",
		(int)powerStateOrdinal, callback, systemWoke, _powerSleep, _powerState );

	return 0;
}

// static
IOReturn
AirPort_Brcm43xx::syncPowerStateGated( OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3 )
{
	BCM_REFERENCE(arg2);
	BCM_REFERENCE(arg3);

	((AirPort_Brcm43xx *)owner)->WL_LOCK();

	// Apple change: State selector added to syncPowerState so it can be called from multiple
	// power transition callbacks
	((AirPort_Brcm43xx *)owner)->syncPowerState( *(unsigned long *)arg0, *(int *)arg1 );

	((AirPort_Brcm43xx *)owner)->WL_UNLOCK();

	return kIOReturnSuccess;
}

void
AirPort_Brcm43xx::powerChange( UInt32 messageType, void *messageArgument )
{
	// When this gets called the PCI device will be powered on
	// Config space is accessible, but config space is not fully restored

#if VERSION_MAJOR > 10
	IOPMSystemCapabilityChangeParameters *params = (IOPMSystemCapabilityChangeParameters *) messageArgument;

	kprintf( "AirPort_Brcm43xx::powerChange: messageType[0x%08x], params(changeFlags[0x%08x] fromCapabilities[0x%08x] toCapabilities[0x%08x])\n",
                (uint32)messageType,
                (params ? params->changeFlags : 0),
                (params ? params->fromCapabilities : 0),
                (params ? params->toCapabilities : 0)
            );

	//Check if this is a Dark wake or Maintenance wake

	// *** APPLE CHANGE ***
	// <rdar://problem/14347692> POWER: J44 will occasionally not achieve Deep Idle sleep after hibernation.
	//changed kIOPMSystemCapabilityWillChange to kIOPMSystemCapabilityDidChange
	if ((messageType == kIOMessageSystemCapabilityChange) && params &&
	    (params->changeFlags & kIOPMSystemCapabilityDidChange) &&
	    (!(params->fromCapabilities & kIOPMSystemCapabilityCPU)) &&
	    (params->toCapabilities & kIOPMSystemCapabilityCPU))
	{
		// if power is off.
		printf( "AirPort_Brcm43xx::powerChange: System Wake - Full Wake/ Dark Wake / Maintenance wake\n" );
		kprintf( "AirPort_Brcm43xx::powerChange: System Wake - Full Wake/ Dark Wake / Maintenance wake\n" ); // print to firewire/mojo

		// See <rdar://problem/5774509> for a description of what has
		// to happen when waking from sleep with AirPort off...
		checkInterfacePowerState();
		_systemSleeping = false;
		// Apple change: track system wake
		systemWoke = true;

		// Make platform WoW disable callback since the system just woke
		// <rdar://problem/8301379>: Don't make this conditional on WOWL_ACTIVE(pub).
		if( wowl_cap )
			platformWoWEnable( false );
	}

	else if ((messageType == kIOMessageSystemCapabilityChange) && params &&
			 (params->changeFlags & kIOPMSystemCapabilityWillChange) &&
			 (params->fromCapabilities & kIOPMSystemCapabilityCPU) &&
			 (!(params->toCapabilities & kIOPMSystemCapabilityCPU)))
	{
		printf( "AirPort_Brcm43xx::powerChange: System Sleep \n" );
		kprintf( "AirPort_Brcm43xx::powerChange: System Sleep \n" );  // print to firewire/mojo

		_systemSleeping = true;

#ifdef BONJOUR
		if (fOffloadLen == 0) {
			RELEASE_PRINT(("wl%d: %s: *** BONJOUR/MDNS OFFLOADS ARE NOT RUNNING.\n",
				unit, __FUNCTION__));
		}
#endif

		// Make sure wowl wake indication is reset before going to sleep even
		// if power is off.
		if( _powerState == PS_INDEX_DOZE && wowl_cap ) {
			wlc_wowl_clear(((AirPort_Brcm43xx *)this)->pWLC->wowl);
			wlc_wowl_wake_reason_process(((AirPort_Brcm43xx *)this)->pWLC->wowl);
		}
	}

	kprintf( "AirPort_Brcm43xx::powerChange: returning: _powerState[%ld] _systemSleeping[%d] systemWoke[%d]\n",
		_powerState, _systemSleeping, systemWoke );
#endif
}

// static
IOReturn
AirPort_Brcm43xx::powerChangeCallbackGated( OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3 )
{
	BCM_REFERENCE(arg2);
	BCM_REFERENCE(arg3);

	((AirPort_Brcm43xx *)owner)->powerChange( *(UInt32 *)arg0, arg1 );
	return kIOReturnSuccess;
}

// static
IOReturn
AirPort_Brcm43xx::powerChangeCallback( void * target, void * refCon, UInt32 messageType,
	IOService * service, void * messageArgument, vm_size_t argSize )
{
	BCM_REFERENCE(refCon);
	BCM_REFERENCE(service);
	BCM_REFERENCE(argSize);

	kprintf( "AirPort_Brcm43xx::powerChangeCallback: messageType[0x%08x]\n", messageType ); // print to firewire/mojo

	switch( messageType )
	{
		case kIOMessageSystemWillSleep:
		case kIOMessageSystemWillPowerOn:
#if VERSION_MAJOR > 10
		case kIOMessageSystemCapabilityChange:
#endif
			break;

		default:
			return kIOReturnSuccess;
	}

	return ((AirPort_Brcm43xx *)target)->getCommandGate()->runAction( AirPort_Brcm43xx::powerChangeCallbackGated,
		&messageType, messageArgument );
}

// Add "board-id" for disabled TAPD systems
const char *_tapdDisabledBoardIds[] =
{

	NULL,                                       // sentinel
};

bool AirPort_Brcm43xx::platformSupportsTruePowerOff()
{
	char	model[32] = {};
	static	int	supportsTAPD = -1;	// -1 is uninitialized, otherwise 0 or 1

	if( supportsTAPD >= 0 )
		return (bool)supportsTAPD;

	if (((CHIPID(revinfo.chipnum) == BCM4360_CHIP_ID) &&
	    (CHIPREV(revinfo.chiprev) <= 2)) || (CHIPID(revinfo.chipnum) == BCM4335_CHIP_ID)) {
		super::setProperty( kAirPortTAPD, (unsigned long long)0, 32 );	// TAPD is disabled
		supportsTAPD = 0;
		return false;
	}

	// These platforms don't support at all, regardless of the override
	if( PEGetModelName( model, sizeof( model ) ) ) {
		if (( strncmp( model, "MacBookPro6,2", 32 ) == 0 ) ||	// K18
		    ( strncmp( model, "MacBookPro6,1", 32 ) == 0 )) {	// K17
			super::setProperty( kAirPortTAPD, (unsigned long long)0, 32 );	// TAPD is disabled
			supportsTAPD = 0;
			return false;
		}
	}

	OSBoolean * truePowerOffOverride = OSDynamicCast( OSBoolean, getProperty( "TruePowerOff" ) );
	if( truePowerOffOverride && truePowerOffOverride->isFalse() ) {
		super::setProperty( kAirPortTAPD, (unsigned long long)0, 32 );	// TAPD is disabled
		supportsTAPD = 0;
		return false;
	}

	// Check systems for disabled TAPD
	for( int i=0 ; _tapdDisabledBoardIds[i] != NULL; ++i )
	{
		// Check "board-id"
		if( (checkBoardId( _tapdDisabledBoardIds[i] ) == true) )
		{
			super::setProperty( kAirPortTAPD, (unsigned long long)0, 32 );	// TAPD is disabled
			supportsTAPD = 0;
			return false;
		}
	}

	// If system not found, then default to TAPD enabled
	super::setProperty( kAirPortTAPD, (unsigned long long)1, 32 );	// TAPD is enabled
	supportsTAPD = 1;
	return true;
}

#define ASPM_L0s	0x1
#define ASPM_L1s	0x2
#define ASPM_OFF	0
#define ASPM_ON		1
#define PCI_ROOT_LINK_CTRL	0x90
#define PCI_DEVICE_LINK_CTRL	0xe0

void AirPort_Brcm43xx::checkOverrideASPM()
{
	char model[32] = {0};

	if( PEGetModelName( model, sizeof( model ) ) ) {
		// set L0s off and L1s on for both sides
		if (( strncmp( model, "MacBookAir4,1", 32 ) == 0 ) ||
		    ( strncmp( model, "MacBookAir4,2", 32 ) == 0 )) {
			//Set ASPM bits on root port
			overrideASPMRoot(ASPM_OFF, ASPM_ON);
			//Set ASPM bits on device side
			overrideASPMDevice(ASPM_OFF, ASPM_ON);
		}
	}
	return;
}

void AirPort_Brcm43xx::overrideASPMRoot(bool L0s, bool L1s)
{
	uint16 linkControl = 0;

	//Set ASPM bits on root port
	IOPCIBridge *parent= (IOPCIBridge *)(pciNub->getParentEntry(gIOServicePlane));
	linkControl = OSL_PCI_READ_CONFIG_PARENT(osh, parent, PCI_ROOT_LINK_CTRL, 2);

	//Modify L0s bit value to adjust settings
	if (L0s)
		linkControl |= ASPM_L0s;
	else
		linkControl &= ~ASPM_L0s;

	//Modify L1s bit value to adjust settings
	if (L1s)
		linkControl |= ASPM_L1s;
	else
		linkControl &= ~ASPM_L1s;

	OSL_PCI_WRITE_CONFIG_PARENT(osh, parent, PCI_ROOT_LINK_CTRL, 2, linkControl);

	return;
}

void AirPort_Brcm43xx::overrideASPMDevice(bool L0s, bool L1s)
{
	uint16 linkControl = 0;

	//Set ASPM bits on device side
	linkControl = pciNub->configRead16(PCI_DEVICE_LINK_CTRL);

	//Modify L0s bit value to adjust settings
	if (L0s)
		linkControl |= ASPM_L0s;
	else
		linkControl &= ~ASPM_L0s;

	//Modify L1s bit value to adjust settings
	if (L1s)
		linkControl |= ASPM_L1s;
	else
		linkControl &= ~ASPM_L1s;

	pciNub->configWrite16(PCI_DEVICE_LINK_CTRL, linkControl);

	return;
}

void
AirPort_Brcm43xx::setACPIProperty( const char * propName, bool enable )
{
	OSObject * acpiObj = pciNub->getProperty( "acpi-device" );

	if( !acpiObj )
		return;

	UInt32 val = enable ? 1 : 0;
	OSObject * argNumber = OSNumber::withNumber( val, 32 );

	if( !argNumber )
		return;

	// Need to use metaCast for PowerPC, see rdar://6247437
	IOACPIPlatformDevice * acpiDevice = (IOACPIPlatformDevice *)acpiObj->metaCast( "IOACPIPlatformDevice" );

	if( acpiDevice )
		acpiDevice->evaluateObject( propName, NULL, &argNumber, 1 );

	// Ignore errors, since this request might be made on a system that does not support the property

	argNumber->release();
}

// Apple change: Support for WoW on systems with WoW hardware, but no EFI support
bool
AirPort_Brcm43xx::acpiPropertySupported( const char * propName )
{
	OSObject * acpiObj = pciNub->getProperty( "acpi-device" );

	if( !acpiObj )
		return false;

	IOACPIPlatformDevice * acpiDevice = (IOACPIPlatformDevice *)acpiObj->metaCast( "IOACPIPlatformDevice" );

	if( !acpiDevice )
		return false;

	return ( acpiDevice->validateObject( propName ) == kIOReturnSuccess );
}

IOReturn
AirPort_Brcm43xx::callPlatformFunction( const char * funcName, bool val )
{
	IOService * plugin = NULL, * resources = NULL;
	mach_timespec_t waitTimeout = { 5, 0 };
	const OSSymbol * nameSymbol = OSSymbol::withCString( funcName );
	IOReturn ioret = kIOReturnSuccess;

	if( !nameSymbol ) {
		return kIOReturnError;
	}

	resources = waitForService( resourceMatching( "IOPlatformPlugin" ), &waitTimeout );

	if( resources && ( plugin = OSDynamicCast( IOService, resources->getProperty( "IOPlatformPlugin" ) ) ) )
	{
		ioret = plugin->callPlatformFunction( nameSymbol, false, (void *)val, NULL, NULL, NULL );

		// Ignore error...this could be called on systems that don't support this platform function
	}
	else
	{
		IOLog( "AirPort_Brcm43xx::callPlatformFunction: Failed to find platform"
		       " plugin (plugin = %p, resources = %p)\n", plugin, resources );
	}

	nameSymbol->release();

	return ioret;
}

void
AirPort_Brcm43xx::enableTruePowerOffIfSupported()
{
	if( platformSupportsTruePowerOff() )
		setACPIProperty( "PDEN", true );
}

void
AirPort_Brcm43xx::platformWoWEnable( bool enable )
{
	IOReturn ioret;

	if( platformSupportsTruePowerOff() )
	{
		// Apple change: Support for WoW on systems with WoW hardware, but no EFI support
		if( acpiPropertySupported( "WWEN" ) )
			setACPIProperty( "WWEN", enable );
		else
		{
			ioret = callPlatformFunction( "WWEN", enable );
			WL_PCIE( ( "AirPort_Brcm43xx::platformWoWEnable: WWEN[%s] ioret[0x%08x] via callPlatformFunction()\n",
				    enable ? "enable" : "disable",
				    ioret ) );

		}

		_wwenState = enable;

		// Always send to system.log
		RELEASE_PRINT(( "AirPort_Brcm43xx::platformWoWEnable: WWEN[%s]\n", enable ? "enable" : "disable" ));
	}
}

#endif /* AAPLPWROFF */

// -----------------------------------------------------------------------------
// Lookup table for the power levels per chip and chip rev
// Table is searched for a chip match and chip rev match
// Search stops with an exact match.
// If the chipnum is match, but a chiprev is not found, the last chipmum
// match entry is used.
// If a chip match is not found, the last entry in the table is used.
/* static */ const chip_power_levels_t*
AirPort_Brcm43xx::chip_power_lookup(uint chipnum, uint chiprev)
{
	int i = 0;
	int len = (int)(sizeof(chip_power_table)/sizeof(chip_power_table[0]));
	const chip_power_levels_t *pwr = NULL;
	const chip_power_levels_t *chip_match = NULL;

	for (i = 0; i < len; i++) {
		pwr = &chip_power_table[i];
		if (pwr->chipnum == chipnum &&
		    pwr->chiprev == chiprev)
			return pwr;

		if (pwr->chipnum == chipnum)
			chip_match = pwr;
	}

	// No exact match, return the last chip match,
	// or the last table entry
	if (chip_match)
		return chip_match;
	else
		return pwr;
}

// -----------------------------------------------------------------------------
// Allocate and return a new AirPort_Brcm43xxInterface instance.
IONetworkInterface *
AirPort_Brcm43xx::createInterface()
{
	AirPort_Brcm43xxInterface *netif = new AirPort_Brcm43xxInterface;

	if (netif && (netif->init(this) == false)) {
		netif->release();
		netif = 0;
	}
	return netif;
}


// -----------------------------------------------------------------------------
bool
AirPort_Brcm43xx::configureInterface( IONetworkInterface *netif )
{
	if ( super::configureInterface( netif ) == false )
        return false;

#if defined(IO80211P2P)
	if( !OSDynamicCast( IO80211P2PInterface, netif ) )
#endif
	{
		// get a pointer to the generic and Ethernet statistics structures.
		IONetworkParameter * param = netif->getParameter( kIONetworkStatsKey );
		if (!param || !(netStats = (IONetworkStats *) param->getBuffer()))
			return false;

		param = netif->getParameter( kIOEthernetStatsKey );
		if (!param || !(etherStats = (IOEthernetStats *) param->getBuffer()))
			return false;
	}

#ifdef MACOS_AQM

#define ENABLE_DRIVER_OUTPUT_PULLMODEL			1

	// Tunable/configurables from driver plist
	OSDictionary *tunables = OSDynamicCast( OSDictionary, getProperty( AQM_TUNABLES_DICTIONARY_TUNABLES ) );
	int32_t value = -1;

	// Default tunables
	_aqmTunables.pullmode = (getOutputPacketSchedulingModel() >= 0) ? (getOutputPacketSchedulingModel() >= 0) : -1;
	_aqmTunables.txringsize = AQM_TUNABLES_DEFAULT_TXRINNGSIZE;
	_aqmTunables.txsendqsize = AQM_TUNABLES_DEFAULT_TXSENDQSIZE;
	_aqmTunables.reapcount = AQM_TUNABLES_DEFAULT_REAPCOUNT;
	_aqmTunables.reapmin = AQM_TUNABLES_DEFAULT_REAPMIN;

	// Get tunables

	if( tunables )
	{
		// Have tunables dictionary, parse tunables

		OSNumber *pullmode = OSDynamicCast( OSNumber, tunables->getObject( AQM_TUNABLES_KEY_PULLMODE ) );
		if( pullmode )
		{
			_aqmTunables.pullmode = (int)(pullmode->unsigned32BitValue());

			// Assume parsed value invalid, disable AQM
			setOutputPacketSchedulingModel( -1 );

			// Check valid AQM scheduling models/modes, if valid: enable AQM w/valid mode
			if( (_aqmTunables.pullmode == 0) || (_aqmTunables.pullmode == 1) )
			{
				setOutputPacketSchedulingModel( _aqmTunables.pullmode );
			}
		}

		OSNumber *txringsize = OSDynamicCast( OSNumber, tunables->getObject( AQM_TUNABLES_KEY_TXRINGSIZE ) );
		if( txringsize )
		{
			_aqmTunables.txringsize = txringsize->unsigned32BitValue();
			_aqmTunables.txringsize = BOUNDMAX( _aqmTunables.txringsize, AQM_TUNABLES_MAX_TXRINGSIZE );	// bound max
			_aqmTunables.txringsize = BOUNDMIN( _aqmTunables.txringsize, AQM_TUNABLES_MIN_TXRINGSIZE );	// bound min
		}

		OSNumber *txsendqsize = OSDynamicCast( OSNumber, tunables->getObject( AQM_TUNABLES_KEY_TXSENDQSIZE ) );
		if( txsendqsize )
		{
			_aqmTunables.txsendqsize = txsendqsize->unsigned32BitValue();
			_aqmTunables.txsendqsize = BOUNDMAX( _aqmTunables.txsendqsize, AQM_TUNABLES_MAX_TXSENDQSIZE );	// bound max
			_aqmTunables.txsendqsize = BOUNDMIN( _aqmTunables.txsendqsize, AQM_TUNABLES_MIN_TXSENDQSIZE );	// bound min
		}

		OSNumber *reapcount = OSDynamicCast( OSNumber, tunables->getObject( AQM_TUNABLES_KEY_REAPCOUNT ) );
		if( reapcount )
		{
			_aqmTunables.reapcount = reapcount->unsigned32BitValue();
			_aqmTunables.reapcount = BOUNDMAX( _aqmTunables.reapcount, AQM_TUNABLES_MAX_REAPCOUNT );	// bound max
			_aqmTunables.reapcount = BOUNDMIN( _aqmTunables.reapcount, AQM_TUNABLES_MIN_REAPCOUNT );	// bound min
		}

		OSNumber *reapmin = OSDynamicCast( OSNumber, tunables->getObject( AQM_TUNABLES_KEY_REAPMIN ) );
		if( reapmin )
		{
			_aqmTunables.reapmin = reapmin->unsigned32BitValue();
			_aqmTunables.reapmin = BOUNDMAX( _aqmTunables.reapmin, AQM_TUNABLES_MAX_REAPMIN );		// bound max
			_aqmTunables.reapmin = BOUNDMIN( _aqmTunables.reapmin, AQM_TUNABLES_MIN_REAPMIN );		// bound min
		}
	}


	printf( "BRCM tunables:\n" );
	printf( "  pullmode[%d] txringsize[%5d] txsendqsize[%d] reapmin[%5d] reapcount[%5d]\n",
				_aqmTunables.pullmode, _aqmTunables.txringsize, _aqmTunables.txsendqsize,
				_aqmTunables.reapmin, _aqmTunables.reapcount );


#if defined(ENABLE_DRIVER_OUTPUT_PULLMODEL) && ENABLE_DRIVER_OUTPUT_PULLMODEL
	int32_t workloopmode = kIONetworkWorkLoopSynchronous;

	// Check platform expert for "io80211.pullmode"
	if( PE_parse_boot_argn( "io80211.pullmode", &value, sizeof(value) ) )
	{
		// Specified boot-arg "io80211.pullmode"
		printf("Over-riding Tx AQM pullmode, found \"io80211.pullmode\" arg, value[%d]\n", value );

		// Assume parsed value invalid, disable AQM
		setOutputPacketSchedulingModel( -1 );

		// Check valid AQM scheduling models/modes, if valid: enable AQM w/valid mode
		if( value == IONetworkInterface::kOutputPacketSchedulingModelDriverManaged )
		{
			setOutputPacketSchedulingModel( value );
		}
	}

	// Check platform expert for "io80211.txsendqsize"
	if( PE_parse_boot_argn( "io80211.txsendqsize", &value, sizeof(value) ) )
	{
		// Specified boot-arg "io80211.txsendqsize"
		_aqmTunables.txsendqsize = value;
		_aqmTunables.txsendqsize = BOUNDMAX( _aqmTunables.txsendqsize, AQM_TUNABLES_MAX_TXSENDQSIZE );		// bound max
		_aqmTunables.txsendqsize = BOUNDMIN( _aqmTunables.txsendqsize, AQM_TUNABLES_MIN_TXSENDQSIZE );		// bound min
		printf("Over-riding Tx AQM txsendqsize, found \"io80211.txsendqsize\" arg, value[%d], bounded[%d]\n",
					value, _aqmTunables.txsendqsize );
	}


	if(	OSDynamicCast( IO80211Interface, netif )
		&& (getOutputPacketSchedulingModel() >= 0) /* schedule configured - AQM enabled */
		&& netif->configureOutputPullModel( _aqmTunables.txringsize, (IOOptionBits)workloopmode, _aqmTunables.txsendqsize,
						    getOutputPacketSchedulingModel() ) != kIOReturnSuccess )
	{
		return( false );
	}

	if( pWLC && (getOutputPacketSchedulingModel() >= 0) ) /* schedule configured - AQM enabled */
	{
		pWLC->oscallback_dotxstatus = oscallback_dotxstatus;
	}

#endif /* ENABLEDRIVER_OUTPUTPULLMODEL */

#endif /* MACOS_AQM */

	return true;
}

#ifdef MACOSX_DHD
void
AirPort_Brcm43xx::getOsIfaceName(char *ifname)
{
	if (!ifname)
		return;

	if(myNetworkIF) {
		snprintf(ifname, IFNAMSIZ, "%s%d",
				myNetworkIF->getNamePrefix(),
				myNetworkIF->getUnitNumber()
				);
	}
}

OSObject *
AirPort_Brcm43xx::getPrimaryIface(void)
{
	return (OSObject *)myNetworkIF;
}
#endif /* MACOSX_DHD */

#ifndef MACOSX_DHD
#ifdef WLCSO
// -----------------------------------------------------------------------------
IOReturn
AirPort_Brcm43xx::getChecksumSupport( UInt32 * checksumMask,
                                      UInt32   checksumFamily,
                                      bool     isOutput )
{
	if (checksumFamily == kChecksumFamilyTCPIP) {
		// same mask for both input and output, isOutput true/false
		*checksumMask = kChecksumIP | kChecksumTCP | kChecksumUDP;
	} else {
		*checksumMask = 0;
	}

	IOLog("%s: checksumMask 0x%x\n", __FUNCTION__, (uint)(*checksumMask));

	return kIOReturnSuccess;
}
#endif // WLCSO
#endif /* !MACOSX_DHD */

const char *
AirPort_Brcm43xx::getIFName(void)
{
	return getIFName(myNetworkIF, ifname);
}

const char *
AirPort_Brcm43xx::getIFName(OSObject *netIf, char *name)
{
	if (name[0] != '\0')
		return name;

	if(!netIf)
		return "??";

#ifdef IO80211P2P
	const char * bsdName = IFMTHD( netIf, getBSDName );

	return bsdName[0] ? bsdName : "??";
#else
	IO80211Interface *intf = (IO80211Interface *)netIf;
	if (!intf) {
		return "??";
	} else if (intf->isRegistered()) {
		snprintf(name, IFNAMSIZ, "%s%d",
			intf->getNamePrefix(), intf->getUnitNumber());
		return name;
	} else {
		return intf->getNamePrefix();
	}
#endif /* IO80211P2P */
}

// -----------------------------------------------------------------------------
// publish the medium dictionary to announce our media capabilities
// JS: is it correct to publish the other speeds (1, 2, 5.5, etc.) as additional media?
bool
AirPort_Brcm43xx::PubWirelessMedium()
{
	OSDictionary *mediumDic = OSDictionary::withCapacity(1); //only 1 medium for now: 11Mbps
	if (!mediumDic) {
		DPRINT("AirPort_Brcm43xx::PubWirelessMedium: can not create a medium dictionary\n");
		return false;
	}

	IONetworkMedium	*medium = IONetworkMedium::medium((IOMediumType) kIOMediumIEEE80211Auto,
							  (UInt64)LINK_SPEED_80211B );
	if (medium)
	{
		mediumDic->setObject( medium->getKey(), medium );
		setCurrentMedium( medium );
		medium->release();

		bool res = publishMediumDictionary( mediumDic );
		if (!res)
			DPRINT("AirPort_Brcm43xx::PubWirelessMedium: publishMediumDictionary failed\n");
	}

	mediumDic->release();

	return true;
}

// -----------------------------------------------------------------------------
//   Returns a string with the name of the vendor.
const OSString*
AirPort_Brcm43xx::newVendorString() const
{
	if (revinfo.boardvendor == VENDOR_APPLE)
		return OSString::withCString("Apple");
	else
		return OSString::withCString("Unknown");
}

// -----------------------------------------------------------------------------
//   Returns a string with the model of the adapter.
const OSString*
AirPort_Brcm43xx::newModelString() const
{
	int band_support = WLC_BAND_ALL; // assume dual band if bandSupprort call fails

        (void)bandSupport(&band_support);

    if (PHYTYPE_IS(phytype, PHY_TYPE_AC))
        return OSString::withCString("Wireless Network Adapter (802.11 a/b/g/n/ac)");
    else if (PHYTYPE_11N_CAP(phytype) && enable11n)
		return OSString::withCString("Wireless Network Adapter (802.11 a/b/g/n)");
	else if (band_support == WLC_BAND_ALL)
		return OSString::withCString("Wireless Network Adapter (802.11 a/b/g)");
	else
		return OSString::withCString("Wireless Network Adapter (802.11 b/g)");
}

int
AirPort_Brcm43xx::setAutoCountry(bool val)
{
	if (isCntryDefaultSupported(revinfo.boardid) && val) {
		/* disable autocountry on all new platform 2016+ */
		val = FALSE;
	}
	return wlIovarSetInt("autocountry", val);
}

// Add "board-id" for lowering phywreg_limit
struct {
	const char *boardId;
	int limit;
} _phyWregLimitOverrides[] =
{
	{ "Mac-06F11F11946D27C5", 8},        // J145G    :  2015
	{ NULL, 0}                           // sentinel
};

// -----------------------------------------------------------------------------
// start up the wlc side
bool
AirPort_Brcm43xx::wlcStart()
{
	char country[WLC_CNTRY_BUF_SZ];
	uint attach_err, status = 0;
	int err = BCME_OK;
	char model[32] = {0};
	void *btparam = NULL; // bus device handle
	uint bustype = PCI_BUS;
	void *regs = NULL;
	uint16 i = 0;
	uint32 subvendorID = NULL;
	int _ioctl = 0;
	host_cc = FALSE;

	btparam = NULL;
	bustype = PCI_BUS;

#ifdef WL_LOCK_CHECK
        wl_lock_count = 0;
        wl_lock_thread_id = NULL;

#ifdef USE_SPIN_LOCK
	wl_spin_lock_grp_attr = lck_grp_attr_alloc_init();
	lck_grp_attr_setstat(wl_spin_lock_grp_attr);

	wl_spin_lock_grp = lck_grp_alloc_init("locK", wl_spin_lock_grp_attr);

	wl_spin_lock_attr = lck_attr_alloc_init();
	wl_spin_lock = lck_spin_alloc_init(wl_spin_lock_grp, wl_spin_lock_attr);
#endif /* USE_SPIN_LOCK */
#endif /* WL_LOCK_CHECK */

	if (!checkCardPresent(TRUE, &status)) {
		for (i=0; i < 50; i++) {
			IOSleep(20);
			if (checkCardPresent(TRUE, &status)) {
				break;
			}
		}
		if (status == NOCARD_PRESENT || status == BAR0_INVALID || status == CARD_NOT_POWERED) {
			return false;
		}
	}

	// Initialize PCI configuration
	pciNub->configWrite32( kIOPCIConfigCommand,
		kIOPCICommandMemorySpace | kIOPCICommandBusMaster | kIOPCICommandMemWrInvalidate );

	// Map PCI register space
	ioMapBAR0 = pciNub->mapDeviceMemoryWithRegister( kIOPCIConfigBaseAddress0 );
	if ( ioMapBAR0 == NULL )
	{
		kprintf("No Map PCI register space\n");
		return false;
	}

	regs = (void *)ioMapBAR0->getVirtualAddress();
	if (!regs) {
		kprintf("mapping of pci register space failed\n");
		return false;
	}
	uint16 vendorID =	pciNub->configRead16( kIOPCIConfigVendorID );
	uint16 deviceID =	pciNub->configRead16( kIOPCIConfigDeviceID );
#ifdef BCMDBG
	uint16 ssVendorID = pciNub->configRead16( kIOPCIConfigSubSystemVendorID );
	kprintf("wlcStart: vendorID/0x%4X deviceID/0x%4X ssVendorID/0x%4X\n",
		vendorID, deviceID, ssVendorID);
#endif /* BCMDBG */
#ifndef MACOSX_DHD
#if defined(BCM_BACKPLANE_TIMEOUT)
        osl_set_wl(osh, (void*)this, &wl_fatal_error, _checkAllWrappers);
#endif

	pWLC = (wlc_info_t *)wlc_attach( (void *)this,	// callbacks return our driver object
					vendorID,		// vendorid
					deviceID,		// deviceid
					0,			// unit #(if more than one instance of driver)
					FALSE,			// piomode, false -> DMA
					osh,			// for callbacks to OSL
					regs,			// ptr to Host Interface Registers
					bustype,		// bus type
					btparam,		// bus device handle
					NULL,			// dict handle
					&attach_err);
	if (pWLC == NULL)
		return false;
	pub = pWLC->pub;
	// WAR for Radar 11281336: Fix up SubSystemID property
	kprintf("Setting property subsystem-id to:%x\n", pub->sih->boardtype);
	pciNub->setProperty("subsystem-id",(uint8 *)&(pub->sih->boardtype), 4);
#else
	sih = si_attach(deviceID, osh, regs, PCI_BUS, this, NULL, NULL);
	if (!sih) {
		kprintf("sih attach failed \n");
		return false;
	}
	// WAR for Radar 11281336: Fix up SubSystemID property
	kprintf("Setting property subsystem-id to:%x\n", sih->boardtype);
	pciNub->setProperty("subsystem-id",(uint8 *)&(sih->boardtype), 4);
#endif /* !MACOSX_DHD */
	subvendorID = VENDOR_APPLE;
	kprintf("Setting property subsystem-vendor-id to:%x\n", subvendorID);
	pciNub->setProperty("subsystem-vendor-id", (uint8 *)&(subvendorID), 4);

	// cache chip revision information
	if ((err = wlIoctl(_ioctl = WLC_GET_REVINFO, &revinfo, sizeof(revinfo), FALSE, NULL)))
		goto ioctl_fail;

	(void) wlIoctlGetInt(WLC_GET_PHYTYPE, &phytype);

	num_radios = 1;
#ifdef WL11N
	int chain;
	err = wlIovarGetInt("hw_txchain", &chain);
	if (!err) {
		num_radios = bcm_bitcount((uint8*)&chain, sizeof(chain));
	} else
		kprintf("wl%d: error %d from iovar \"hw_txchain\"\n",
			unit, err);
#endif /* WL11N */

#if VERSION_MAJOR <= 8
	if (super::doFeatureCheck() != 0) {
		kprintf("AirPort_Brcm43xx::start: doFeatureCheck failed!\n");
		return false;
	}
#else
	// Override to allow 11n without 11n enabler
	enableFeature(kIO80211Feature80211n, NULL);
#endif /* VERSION_MAJOR */

#ifndef MACOSX_DHD
	/* Model specific settings */
	if ( PEGetModelName(model, sizeof(model)) ) {
		/* Disable PCIE override WAR for some models */
		if ( ( strcmp(model, "MacBook3,1") == 0 ) )
			wlc_pcie_war_ovr_update(pWLC, PCIE_ASPM_ENAB);
		/* Modify PHY write reglimit for some models */
		if ((strncmp(model, "MacBookAir5", 11) == 0))
			(void) wlIovarSetInt("phywreg_limit", 8);
	}

	// J145G specific setting
	for (i = 0; _phyWregLimitOverrides[i].boardId != NULL; i++) {
		if (checkBoardId(_phyWregLimitOverrides[i].boardId) == true) {
			(void) wlIovarSetInt("phywreg_limit",
				_phyWregLimitOverrides[i].limit);
		}
	}
#endif /* !MACOSX_DHD */

	err = wlIoctl(WLC_GET_COUNTRY, country, WLC_CNTRY_BUF_SZ, FALSE, NULL);
	if (err)
		bzero(country, WLC_CNTRY_BUF_SZ);

#ifndef MACOSX_DHD
	// Change a SPROM rev 1 card claiming to be Jordan to France.
	// Older Apple cards manufacured for France were given the Jordan locale value to
	// disable channels 1-9, that are not available in all areas of France. Since then
	// it was decided that the entire 1-13 range was fine.
	if (pub->sromrev == 1 && !bcmp(country, "JO", 2)) {
		bzero(country, WLC_CNTRY_BUF_SZ);
		strncpy(country, "FR", WLC_CNTRY_BUF_SZ);
		(void) wlIoctl(WLC_SET_COUNTRY, country, WLC_CNTRY_BUF_SZ,
				TRUE, NULL);

	}
#endif /* !MACOSX_DHD */
	// Enable Auto Country Discovery for devices set to aggregate country SKUs
	if (revinfo.boardvendor == VENDOR_APPLE &&
	    (!bcmp(country, "XA", 2) ||  // ETSI Worldwide m35a
	     !bcmp(country, "XB", 2) ||  // Americas/Taiwan SKU1 m35a
	     !bcmp(country, "X0", 2) ||  // FCC Wordwide SKU1
	     !bcmp(country, "X1", 2) ||  // Worldwide RoW 1 SKU4
	     !bcmp(country, "X2", 2) ||  // Worldwide RoW 2 SKU5
	     !bcmp(country, "X3", 2))) { // ETSI Worldwide SKU3
		(void) wlIovarOp("autocountry_default", NULL, 0,
		             country, strlen(country), TRUE, NULL);

#ifdef FACTORY_BUILD
		(void) setAutoCountry(FALSE);
#else
		(void) setAutoCountry(TRUE);
#endif
	}

#ifdef CNTRY_DEFAULT
	if (isCntryDefaultSupported(revinfo.boardid)) {
		if (isCntryRestricted(country)) {
			setRestrictedCntryCode(country, TRUE);
		} else {
			setRestrictedCntryCode(country, FALSE);
		}
	}
#endif /* CNTRY_DEFAULT */

	// Force Interference Mitigation for all chips after 4306b0
	// APPLE CHANGE: Disable interference mitigation on PowerPC
#ifdef __ppc__
	forceInterferenceMitigation = FALSE;
	(void) wlIoctlSetInt( WLC_SET_INTERFERENCE_MODE, INTERFERE_NONE );

	coalesce_packet = TRUE;
#else
#ifndef MACOSX_DHD
	forceInterferenceMitigation = TRUE;
#endif /* !MACOSX_DHD */
	coalesce_packet = FALSE;

	// D11 rev40+ chips have large agg sizes and need to coalesce to conserve DMA ring entries.
	// Also AMSDU needs tailroom to work
	if (D11REV_GE(revinfo.corerev, 40)) {
		coalesce_packet = TRUE;
	}
#endif
	SelectAntennas();					// set tx/rx antennas based on model

	if (revinfo.chipnum == BCM4331_CHIP_ID && revinfo.boardid == BCM94331X19) {
		if (!err) {
			if ((revinfo.boardid == BCM94331X19) &&
			    ((revinfo.boardrev == 0x1201) || (revinfo.boardrev == 0x1202))) {
				int aspm = 0;
				(void) wlIovarGetInt("aspm", &aspm);
				aspm |= 0x1;
				(void) wlIovarSetInt("aspm", aspm);
			}
		}
	}

	(void) wlIoctlSetInt(WLC_SET_FAKEFRAG, 1);
	(void) wlIoctlSetInt(WLC_SET_GMODE, GMODE_AUTO);

	/* Force Roam delta to 10 */
	{
		UInt32 roamDelta = AGG_ROAM_DELTA;
		int roam_delta_set[2] = {0};
		int beacon_rssi = 8;

		roam_delta_set[0] = (int)roamDelta;
		roam_delta_set[1] = WLC_BAND_ALL;

		(void) wlIoctl(WLC_SET_ROAM_DELTA, roam_delta_set, sizeof(roam_delta_set), true, NULL);
		/* add 0x200 which is bit 9 flag to beacon_rssi */
		beacon_rssi |= 0x0200;
		/* set the rssi_win and snr_win to accept only beacon */
		(void) wlIovarSetInt("rssi_win", beacon_rssi);
		(void) wlIovarSetInt("snr_win", beacon_rssi);
	}

	if( alwaysWantPS() )
		(void) wlIoctlSetInt(WLC_SET_PM, PM_FAST);

	/* Turn off vlan mode */
	(void) wlIovarSetInt("vlan_mode", 0);

#ifdef IO80211P2P
	(void) wlIovarSetInt("apsta", 1);
#endif /* IO80211P2P */

	/* Turn off hardware radio monitor */
#ifdef WL_BSSLISTSORT
	(void) wlIovarSetInt("wlfeatureflag", WL_SWFL_NOHWRADIO | WL_SWFL_WLBSSSORT);
#else
	(void) wlIovarSetInt("wlfeatureflag", WL_SWFL_NOHWRADIO);
#endif
	// set default WLAN desense level
	if ((revinfo.chipnum == BCM4360_CHIP_ID) ||
	    (revinfo.chipnum == BCM4350_CHIP_ID) ||
	    (revinfo.chipnum == BCM43602_CHIP_ID)) {
		(void) wlIovarSetInt("btc_rxgain_level", 3);
	} else {
		(void) wlIovarSetInt("btc_rxgain_level", 1);
	}

	// set default 11n Bandwidth to 20MHz only in 2.4GHz, 20/40 in 5GHz
	(void) wlIovarSetInt("mimo_bw_cap", WLC_N_BW_20IN2G_40IN5G);
#ifndef MACOSX_DHD
	if (!WLCISACPHY(pWLC->band))
#else
	if (!PHYTYPE_IS(phytype, PHY_TYPE_AC))
#endif /* !MACOSX_DHD */
	{
		(void) wlIovarSetInt("ampdu_rx_factor", AMPDU_RX_FACTOR_64K);
	}

	// turn on the 40 MHz intolerant bit, applicable only on 2GHz
	(void) wlIovarSetInt("intol40", ON);

	(void) wlIovarSetInt("frameburst_override", 1);

	(void) wlIovarSetInt("brcm_ie", 0);

	//No preference to 5Ghz network if ap_compare APi is used
#ifndef WL_BSSLISTSORT
	// Join preference to join 5Ghz network if better by at least 10 db
	{
		uint8 buf[JOIN_PREF_IOV_LEN] = {0};
		uint8 *ptr = buf;
		bzero(buf, sizeof(buf));

		PREP_JOIN_PREF_RSSI(ptr);
		ptr += (2 + WLC_JOIN_PREF_LEN_FIXED);
		PREP_JOIN_PREF_RSSI_DELTA(ptr, JOIN_RSSI_DELTA, JOIN_RSSI_BAND);
		(void) wlIovarOp("join_pref", NULL, 0,
		             buf, JOIN_PREF_IOV_LEN, TRUE, NULL);
	}
#endif

#ifdef WLSCANCACHE
	// Use the scan cache in association attempts
	(void) wlIovarSetInt("assoc_cache_assist", 1);
#endif

#ifndef MACOSX_DHD
	wowl_cap = wlc_wowl_cap(pWLC);

	// Apple change: make this query once
	wowl_cap_platform = wowCapablePlatform();
#endif /* MACOSX_DHD */

	sgi_cap = 0;
	(void) wlIovarGetInt("sgi_rx", (int*)&sgi_cap);

	if ((revinfo.chipnum == BCM4331_CHIP_ID) ||
	    (revinfo.chipnum == BCM4360_CHIP_ID) ||
	    (revinfo.chipnum == BCM4350_CHIP_ID) ||
	    (revinfo.chipnum == BCM43602_CHIP_ID)) {
		(void) wlIovarSetInt("stbc_tx", -1);
		(void) wlIovarSetInt("stbc_rx", 1);
	}
#ifndef MACOSX_DHD
#ifdef WL_LTR
	if (CHIPID(revinfo.chipnum) == BCM4350_CHIP_ID) {
		(void) wlIovarSetInt("ltr_active_lat", LTR_LATENCY_140US);
	} else if (CHIPID(revinfo.chipnum) == BCM43602_CHIP_ID) {
		(void) wlIovarSetInt("ltr_active_lat", LTR_LATENCY_140US);
	}
#endif
#endif /* !MACOSX_DHD */
#if (VERSION_MAJOR > 11)
	/* Only enable rcvlazy ints for 4360B0 and later */
	if (D11REV_GE(revinfo.corerev, 42)) {
		if (!D11REV_IS(revinfo.corerev, 48))
			(void) wlIovarSetInt("rcvlazy", 0x20000064);
	}
#endif
	(void) wlIovarSetInt("roam_rssi_cancel_hysteresis", ROAM_RSSITHRASHDIFF_MACOS);

	(void) wlIovarSetInt("mimo_preamble", WLC_N_PREAMBLE_MIXEDMODE);

#ifdef WL_ASSOC_RECREATE
	if (D11REV_GE(revinfo.corerev, 42))
		(void) wlIovarSetInt("assoc_recreate", ON);
#endif

	/* override the return to pm idle period to 110ms (default 200ms) */
	if (revinfo.boardid == BCM94350X14P2) {
		(void) wlIovarSetInt("pm2_sleep_ret", 110);
	}

	/* cached default nmode setting */
	(void) wlIovarGetInt("nmode", &cached_nmode_state);
	(void) wlIovarGetInt("vhtmode", &cached_vhtmode_state);

	if ((revinfo.chipnum == BCM4331_CHIP_ID) ||
	    (revinfo.chipnum == BCM4360_CHIP_ID) ||
	    (revinfo.chipnum == BCM4350_CHIP_ID) ||
	    (revinfo.chipnum == BCM43602_CHIP_ID)) {
		struct wl_spatial_mode_data spatial_mode = {};
		int err = BCME_OK;

		/* Set default condition for Spatial Policy
		 * Per customer requirement, disable CDD for 5G low channels
		 */
		spatial_mode.mode[SPATIAL_MODE_2G_IDX] = AUTO; /* 2.4G channels */
		/* 5G_Lower channels - 36,40,44,48 (UNII -1) */
		spatial_mode.mode[SPATIAL_MODE_5G_LOW_IDX] = OFF;
		/* 5G_Middle channels - 52,56,60,64(UNII-2 lower band) */
		spatial_mode.mode[SPATIAL_MODE_5G_MID_IDX] = AUTO;
		/* 5G_H channels - 100 through 140((UNII-2 upper band) */
		spatial_mode.mode[SPATIAL_MODE_5G_HIGH_IDX] = AUTO;
		/* 5G_Upper channels - 149, 153,157,161(UNII-3 band) */
		spatial_mode.mode[SPATIAL_MODE_5G_UPPER_IDX] = AUTO;

		err = wlIovarOp("spatial_policy", NULL, 0,
			&spatial_mode, sizeof(spatial_mode), IOV_SET, NULL);
		if (err) {
			WL_IOCTL(("wl%d: setCDD_MODE: error \"%s\" (%d) from iovar \"spatial_mode\"\n",
				  unit, bcmerrorstr(err), err));
		}
	}
#ifndef MACOSX_DHD
	PRIOFC_ENAB(pub) = TRUE;	// Priority based flowcontrol, should be iovar
#endif /* !MACOSX_DHD */
	// set some static properties
	super::setProperty( kIOFirmwareVersion, getVersionString() );
	super::setProperty( kAirPortFeatures, kAPFeatureWpaTKIP, 32 );	// we support WPA-TKIP!
	super::setProperty( kAirPortChipRev, revinfo.chiprev, 32 );		// 4306 version

	// change default lo-power roam threshold
	// on Intel only
#ifdef __i386
	{
		OSNumber *numbProp = OSDynamicCast( OSNumber, getProperty(APRoamTrigger) );
		if (numbProp) {
			UInt32	roamTrig = numbProp->unsigned32BitValue();
			int roam_set[2] = {0};

			roam_set[0] = (int)roamTrig;
			roam_set[1] = WLC_BAND_ALL;

			(void) wlIoctl(WLC_SET_ROAM_TRIGGER, roam_set,
					sizeof(roam_set), true, NULL);
		}
	}
#endif

#ifdef BCMDBG
	{
		OSNumber *msgProp = OSDynamicCast( OSNumber, getProperty("wl_msg_level") );
		if (msgProp) {
			IOLog("wl_msg_level: 0x%x\n", msgProp->unsigned32BitValue());
			wl_msg_level = msgProp->unsigned32BitValue();
		}

		msgProp = OSDynamicCast( OSNumber, getProperty("wl_msg_level2") );
		if (msgProp) {
			IOLog("wl_msg_level2: 0x%x\n", msgProp->unsigned32BitValue());
			wl_msg_level2 = msgProp->unsigned32BitValue();
		}
	}
#endif

	{
		// Allow over-ride wl_msg_level
		int32_t value = 0;
		if( PE_parse_boot_argn( "wl_msg_level", &value, sizeof(value) ) )
		{
			printf("Over-riding wl_msg_level, found \"wl_msg_level\" arg, value[0x%08x]\n", value );
			wl_msg_level = value;
		}

		// Allow over-ride wl_msg_level2
		if( PE_parse_boot_argn( "wl_msg_level2", &value, sizeof(value) ) )
		{
			printf("Over-riding wl_msg_level2, found \"wl_msg_level2\" arg, value[0x%08x]\n", value );
			wl_msg_level2 = value;
		}

		if( PE_parse_boot_argn( "io80211.linkdebug", &value, sizeof(value) ) )
		{
			printf("Over-riding io80211.linkdebug, found \"io80211.linkdebug\" arg, value[0x%08x]\n", value );
			_linkdebug = value;
		}
	}

#if VERSION_MAJOR > 9
	link_qual_config.version = APPLE80211_VERSION;
	link_qual_config.rssi_divisor = APPLE80211_LINK_QUAL_DEFAULT_DIVISOR;
	link_qual_config.tx_rate_divisor = APPLE80211_LINK_QUAL_DEFAULT_DIVISOR;
	link_qual_config.min_interval = APPLE80211_LINK_QUAL_DEFAULT_MIN_INTERVAL;
#if VERSION_MAJOR > 13
	link_qual_config.hysteresis = APPLE80211_LINK_QUAL_DEFAULT_HYSTERESIS;
	rssi_hysteresis = MAX_RSSI * link_qual_config.hysteresis / link_qual_config.rssi_divisor / 100;
#endif
#ifndef MACOSX_DHD
	if (wlc_module_register(pub, NULL, "macosx", this, NULL, wl_macos_watchdog, NULL, NULL))
	{
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          unit, __FUNCTION__));
		return false;
	}
#endif /* !MACOSX_DHD */
#endif

#ifdef WLEXTLOG
	{
		wlc_extlog_cfg_t w_cfg = {};

		bzero(&w_cfg, sizeof(wlc_extlog_cfg_t));
		w_cfg.module = 0;
		w_cfg.level = WL_LOG_LEVEL_ERR;
		w_cfg.flag = LOG_FLAG_EVENT;
		err = wlIovarOp("extlog_cfg", NULL, 0, &w_cfg, sizeof(wlc_extlog_cfg_t), IOV_SET, NULL);
		if (err) {
			kprintf("wl%d: extlog cfg failed: error \"%s\" (%d)\n",
			          unit, bcmerrorstr(err), err);
		}
	}
#endif

#if defined(FACTORY_MERGE)
	disableFactory11d = FALSE;          // 11d for factory testing
	disableFactoryPowersave = FALSE;    // Power save mode for factory testing
	disableFactoryRoam = FALSE;         // Roam for factory testing
#endif

	return true;

ioctl_fail:
	WL_ERROR(("%s: ioctl %d failed with error %d\n", __FUNCTION__, _ioctl, err));
	return false;
}

// -----------------------------------------------------------------------------
// shut down the wlc
void
AirPort_Brcm43xx::wlcStop()
{
#if defined(WL_LOCK_CHECK) && defined(USE_SPIN_LOCK)
	if (wl_spin_lock) {
		lck_spin_destroy(wl_spin_lock, wl_spin_lock_grp);
		lck_spin_free(wl_spin_lock, wl_spin_lock_grp);
	}
#endif /* WL_LOCK_CHECK && USE_SPIN_LOCK */

#ifndef MACOSX_DHD
	if (pWLC) {
		wl_down((struct wl_info *)this);
#if VERSION_MAJOR > 9
		wlc_module_unregister(pub, "macosx", this);
#endif
#if (defined(P2PO) && !defined(P2PO_DISABLED)) || (defined(ANQPO) && \
	!defined(ANQPO_DISABLED))
#if defined(P2PO) && !defined(P2PO_DISABLED)
		if (p2po)
			wl_p2po_detach(p2po);
		if (disc)
			wl_disc_detach(disc);
#endif /* defined(P2PO) && !defined(P2PO_DISABLED) */
#if defined(ANQPO) && !defined(ANQPO_DISABLED)
		if (anqpo)
			wl_anqpo_detach(anqpo);
#endif /* defined(ANQPO) && !defined(ANQPO_DISABLED) */
		if (gas)
			wl_gas_detach(gas);
#endif	/* (P2PO && !P2PO_DISABLED) || (ANQPO && !ANQPO_DISABLED) */
#if defined(WL_EVENTQ) && defined(P2PO) && !defined(P2PO_DISABLED)
		if (wlevtq) {
			wl_eventq_detach(wlevtq);
			wlevtq = NULL;
		}
#endif /* WL_EVENTQ && P2PO && P2PO_DISABLED */
		wlc_detach( pWLC );
	}

#ifdef ENABLE_CORECAPTURE
		if (_FWData) {
			_FWData->release();
			_FWData = NULL;
		}

		if (_UCMData) {
			_UCMData->release();
			_UCMData = NULL;
		}
#endif

	pWLC = NULL;
#else
	wl_down((struct wl_info *)this);
#endif /* !MACOSX_DHD */
	if (ioMapBAR0)
		ioMapBAR0->release();
	ioMapBAR0 = NULL;

	if (ioMapBAR1)
		ioMapBAR1->release();
	ioMapBAR1 = NULL;
}

// -----------------------------------------------------------------------------
// set tx/rx antennas based on registry properties
void
AirPort_Brcm43xx::SelectAntennas()
{
	int antSetting = kTxAntMain;
	int err = BCME_OK;

	// select TX antenna setting

	OSData *antType = OSDynamicCast( OSData, pciNub->getProperty(kTxAntennaProperty) );
	if (antType)
	{
		if (antType->isEqualTo( "Aux", 3 ))		antSetting = kTxAntAux;
		else if (antType->isEqualTo( "Auto", 4 ))	antSetting = kTxAntAuto;
		else if (antType->isEqualTo( "Main", 4 ))	antSetting = kTxAntMain;
		else err = TRUE;

		if (!err)
			err = wlIoctlSetInt(WLC_SET_TXANT, antSetting );
	}

	// select RX antenna setting
	err = FALSE;
	antType = OSDynamicCast( OSData, pciNub->getProperty(kRxAntennaProperty) );
	if (antType)
	{
		if (antType->isEqualTo( "Aux", 3 ))		antSetting = kRxAntAux;
		else if (antType->isEqualTo( "Main", 4 ))	antSetting = kRxAntMain;
		else if (antType->isEqualTo( "Auto", 4 ))	antSetting = kRxAntAutoDiversity;
		else err = TRUE;

		if (!err)
			err = wlIoctlSetInt(WLC_SET_ANTDIV, antSetting);
	}
}
// -----------------------------------------------------------------------------
// Create and register an interrupt event source. The provider will
// take care of the low-level interrupt registration stuff.
bool
AirPort_Brcm43xx::createInterruptEvent()
{
	ASSERT( pciNub );

	int intIndex = 0;
	int flags = 0;
	IOReturn res = pciNub->getInterruptType( 1, &flags );

	if (res == 0 && (flags & kIOInterruptTypePCIMessaged))
		intIndex = 1;

	interruptSrc = IOInterruptEventSource::interruptEventSource(
							this,
							interruptEntryPoint,
							pciNub,
							intIndex );

	if (!interruptSrc || (getWorkLoop()->addEventSource( interruptSrc ) != kIOReturnSuccess))
	{
		DPRINT("AirPort_Brcm43xx::start: can not register interrupt\n");
		return false;
	}
	interruptSrc->enable();			// do this now, in case interrupt line is shared

	return true;
}

// -----------------------------------------------------------------------------
// static method device interrupt entry point

/* static */ void
AirPort_Brcm43xx::interruptEntryPoint( OSObject *driver, IOInterruptEventSource *src, int count )
{
#ifndef MACOSX_DHD
	bool wantdpc = FALSE;
	AirPort_Brcm43xx *wlobj = OSDynamicCast(AirPort_Brcm43xx, driver);
	struct wlc_info * wlc = NULL;

	BCM_REFERENCE(src);
	BCM_REFERENCE(count);

	ASSERT(wlobj);
	if (!wlobj) {
		return;
	}
	wlobj->WL_LOCK();

	wlc = wlobj->pWLC;

	(void) wlc_isr( wlc, &wantdpc );	// Primary int handler

	if (wantdpc) {

		wlc_dpc_info_t dpci = {0};

		wlobj->inputQueue->serviceHold(TRUE);

		wlc_dpc( wlc, FALSE, &dpci );		// Secondary int handler

		// wlc_dpc could brning down the driver
		if (wlobj->pub->up)
			wlc_intrson( wlc );
	}

	wlobj->WL_UNLOCK();

	if (wantdpc) {
		wlobj->inputQueue->service();
		wlobj->inputQueue->serviceHold(FALSE);
	}
#endif /* !MACOSX_DHD */
}

// -----------------------------------------------------------------------------
// static method for Brcm43xx_InputQueue::Action
//
/* static */ void
AirPort_Brcm43xx::inputAction(IONetworkController *target, mbuf_t m)
{
	// Call the IO80211Controller::inputPacket()
	((AirPort_Brcm43xx*)target)->inputPacket(m);
	((AirPort_Brcm43xx*)target)->netStats->inputPackets++;
}
#ifndef MACOSX_DHD
int
AirPort_Brcm43xx::wlIovarOp(const char *name, void *params, size_t p_len, void *buf, size_t len, bool set,
                                OSObject *iface)
{
        int err = BCME_OK;
        struct wlc_if *wlcif = wlLookupWLCIf(iface);

        err = wlc_iovar_op(pWLC, name, params, p_len, buf, len, set, wlcif);
        if (err)
                WL_ERROR(("%s: wlc_iovar_op called with cmd \"%s\" returned error %d\n",
                                __FUNCTION__, name, err));

        return err;
}

int
AirPort_Brcm43xx::wlIovarGetInt(const char *name, int *arg) const
{
        int err = BCME_OK;
        err = wlc_iovar_op(pWLC, name, NULL, 0, arg, sizeof(int), IOV_GET, NULL);
        if (err)
                WL_ERROR(("%s: wlc_iovar_op called with cmd \"%s\" returned error %d\n",
                                __FUNCTION__, name, err));

        return err;
}

int
AirPort_Brcm43xx::wlIoctl(uint cmd, void *arg, size_t len, bool set, OSObject *iface)
{
        int err = BCME_OK;
        struct wlc_if *wlcif = wlLookupWLCIf(iface);

	BCM_REFERENCE(set);

        err = wlc_ioctl(pWLC, cmd, arg, len, wlcif);
        if (err)
                WL_ERROR(("%s: wlc_ioctl called with cmd %d returned error %d\n",
                                __FUNCTION__, cmd, err));

        return err;
}

int
AirPort_Brcm43xx::wlIoctlGet(uint cmd, void *arg, size_t len, OSObject *iface) const
{
        int err = BCME_OK;
        struct wlc_if *wlcif = wlLookupWLCIf(iface);

        err = wlc_ioctl(pWLC, cmd, arg, len, wlcif);
        if (err)
                WL_ERROR(("%s: wlc_ioctl called with cmd %d returned error %d\n",
                                __FUNCTION__, cmd, err));

        return err;
}
#endif /* !MACOSX_DHD */
int
AirPort_Brcm43xx::wlIoctlGetInt(uint cmd, int *arg) const
{
	return wlIoctlGet(cmd, (void*)arg, sizeof(int), NULL);
}

int
AirPort_Brcm43xx::wlIoctlSetInt(uint cmd,  int arg)
{
	return wlIoctl(cmd, (void*)&arg, sizeof(arg), true, NULL);
}

int
AirPort_Brcm43xx::wlIovarSetInt(const char *name, int arg)
{
	return wlIovarOp(name, NULL, 0, (void *)&arg, sizeof(arg), IOV_SET, NULL);
}

SInt32
AirPort_Brcm43xx::wlIocCommon(struct apple80211req *req, OSObject *iface, bool set, bool userioc )
{
	uint8* iobuf = iobuf_small;
	uint8* iobuf_large = NULL;
	int err = 0;
	int io_err = 0;

	if (req->req_len > WLC_IOCTL_SMLEN) {
		iobuf_large = (uint8*)MALLOC(osh, req->req_len);
		if (!iobuf_large)
			return ENOMEM;
		iobuf = iobuf_large;
	}

	/* If this ioctl did not come from userland application then
	 * avoid copyin as address-space does not match
	 */
	if (!userioc)
		bcopy((void*)req->req_data, iobuf, req->req_len);
	else {
		err = copyin((user_addr_t)req->req_data, iobuf, req->req_len);
		if (err) {
			WL_ERROR(("wl%d: %s: copyin err %d\n", unit, __FUNCTION__, err));
			if (iobuf_large)
				MFREE(osh, iobuf_large, req->req_len);
			return err;
		}
	}

	KERNEL_DEBUG_CONSTANT(WiFi_misc_ioctl | DBG_FUNC_NONE, req->req_val, 0, 0, 0, 0);

	WL_LOCK();
	io_err = wlIoctl(req->req_val, iobuf, req->req_len, set, iface);
	WL_UNLOCK();

	if (io_err) {
		io_err = osl_error(io_err);
	}
	else
	{
#ifdef AAPLPWROFF
		// Check if this is a wowl_test set request
		if( req->req_val == WLC_SET_VAR && !strncmp( (const char *)iobuf, "wowl_test", sizeof( "wowl_test" ) - 1 ) )
		{
			UInt32 varNameLen = strlen( (const char *)iobuf ) + 1;

			if( req->req_len > varNameLen && req->req_len - varNameLen == sizeof( int ) )
			{
				platformWoWEnable( *(int *)(iobuf + varNameLen ) ? true : false );
			}

		}
#endif
	}

	if (!userioc)
		bcopy(iobuf, (void *)req->req_data, req->req_len);
	else
		err = copyout(iobuf, (user_addr_t)req->req_data, req->req_len);

	if (iobuf_large)
		MFREE(osh, iobuf_large, req->req_len);

	// return the error from wlc_ioctl if there is one instead of a copyout error
	return io_err ? io_err : err;
}

SInt32
#if VERSION_MAJOR < 10
AirPort_Brcm43xx::apple80211_ioctl( IO80211Interface * interface, ifnet_t ifn, u_int32_t cmd, void * data )
#else
#if defined(IO80211P2P) || 0 || (VERSION_MAJOR > 10)
AirPort_Brcm43xx::apple80211_ioctl( IO80211Interface * interface, IO80211VirtualInterface * vif, ifnet_t ifn, unsigned long cmd, void * data )
#else
AirPort_Brcm43xx::apple80211_ioctl( IO80211Interface * interface, ifnet_t ifn, unsigned long cmd, void * data )
#endif /* IO80211P2P */
#endif
{
	struct apple80211req req = {};
	OSObject *iface = NULL;

#ifdef BCMDBG
	pid_t pid = proc_selfpid();
#endif

	char processName[128] = "<unknown>";

	proc_selfname( processName, sizeof( processName ) );

	if (cmd != SIOCSA80211 && cmd != SIOCGA80211) {
#if defined(IO80211P2P) || 0 || (VERSION_MAJOR > 10)
		return super::apple80211_ioctl(interface, vif, ifn, cmd, data);
#else
		return super::apple80211_ioctl(interface, ifn, cmd, data);
#endif
	}

	bcopy(data, &req, sizeof(struct apple80211req));
#ifdef MACOSX_DHD
	req.req_len = MIN(req.req_len, WLC_IOCTL_MAXLEN);
#endif /* MACOSX_DHD */
	WL_PORT(("wl%d: \"%.16s\" ioctl %s : %s req_type %d %s val 0x%x buf 0x%p len %u, pid[%d]'%s'\n",
	         unit, req.req_if_name,
	         (cmd==SIOCSA80211)?"SIOCSA80211":"SIOCGA80211",
	         (cmd==SIOCSA80211)?"set":"get",
	         req.req_type,
	         lookup_name(apple80211_ionames, req.req_type), (uint)req.req_val,
		 OSL_OBFUSCATE_BUF((void *)((uintptr)req.req_data)),
	         (uint)req.req_len, pid, processName));

	/* IO80211Family 200.7 headers changed the ioctl number from 255 to 0xfffffff
	 * Recognize 255 for a while (8/22/07) until old wl command line utils have been
	 * rebuilt and replaced.
	 */
	if (req.req_type != (int)APPLE80211_IOC_CARD_SPECIFIC &&
	    req.req_type != 255) {
#if defined(IO80211P2P) || 0 || (VERSION_MAJOR > 10)
		return super::apple80211_ioctl(interface, vif, ifn, cmd, data);
#else
		return super::apple80211_ioctl(interface, ifn, cmd, data);
#endif /* IO80211P2P */
	}

#if defined(IO80211P2P)
	if( OSDynamicCast( IO80211P2PInterface, vif ) ) {
		iface = vif;
		// During transition it may happen
		if (iface == NULL)
			return ENXIO;
	}
#endif /* IO80211P2P */

	return wlIocCommon(&req, iface, (cmd == SIOCSA80211));
}

// -----------------------------------------------------------------------------
// Purpose:
//	Returns the adapter's hardware address. This pure virtual method is called by
//  IOEthernetInterface right after the attachInterface() call.
IOReturn
AirPort_Brcm43xx::getHardwareAddress( IOEthernetAddress *addr )
{
#ifdef MACOSX_DHD
	int err = NULL;

	err = wlIovarOp("cur_etheraddr", NULL, 0, addr, ETHER_ADDR_LEN, IOV_GET, NULL);
	if (err) {
		WL_ERROR(("wl%d: getHardwareAddress: error \"%s\" (%d) getting MAC addr\n",
			unit, bcmerrorstr(err), err));
		return kIOReturnError;
	}
#else
	BCOPYADRS( &pub->cur_etheraddr, addr );
#endif /* MACOSX_DHD */
	return kIOReturnSuccess;
}

IOReturn
AirPort_Brcm43xx::setHardwareAddress(const IOEthernetAddress * addr)
{
	struct ether_addr ea = {};
	int err = BCME_OK;
	BCOPYADRS(addr->bytes, ea.octet);
#ifdef MACOSX_DHD
	bool isup = isUp();
	if (isup) {
		wl_down((struct wl_info*)this);
	}
#endif /* MACOSX_DHD */
	err = wlIovarOp("cur_etheraddr", NULL, 0, ea.octet, ETHER_ADDR_LEN, IOV_SET, NULL);
	if (err) {
		WL_ERROR(("wl%d: setHardwareAddress: error \"%s\" (%d) setting MAC addr override\n",
			unit, bcmerrorstr(err), err));
		return kIOReturnError;
	}
#ifdef MACOSX_DHD
	if (isup) {
		wl_up((struct wl_info*)this);
	}
#endif /* MACOSX_DHD */

	return kIOReturnSuccess;
}

// -----------------------------------------------------------------------------
IOReturn
AirPort_Brcm43xx::setPromiscuousMode( bool active )
{
	int err = BCME_OK;

	err = wlIoctlSetInt(WLC_SET_PROMISC, (int)active);

	return (err == 0) ? kIOReturnSuccess : kIOReturnError;
}

// -----------------------------------------------------------------------------

void
AirPort_Brcm43xx::populateMulticastMode()
{
#ifndef MACOSX_DHD
	wlc_bsscfg_t *bsscfg = NULL;
	int i = 0;
	FOREACH_BSS(pWLC, i, bsscfg) {
		bsscfg->allmulti = multicastEnabled;
	}
#endif /* !MACOSX_DHD */
	return;
}

IOReturn
AirPort_Brcm43xx::setMulticastMode( IOEnetMulticastMode mode )
{
	multicastEnabled = !(mode == false);
	populateMulticastMode( );
	return kIOReturnSuccess;
}

// -----------------------------------------------------------------------------

void
AirPort_Brcm43xx::populateMulticastList( )
{
#ifndef MACOSX_DHD
	wlc_bsscfg_t *bsscfg = NULL;
	int i = 0;
	FOREACH_BSS(pWLC, i, bsscfg) {
		bsscfg->nmulticast = nMcast;
		memcpy(bsscfg->multicast, mcastList, MAXMULTILIST);
	}
#endif /* !MACOSX_DHD */
	return;
}


IOReturn
AirPort_Brcm43xx::setMulticastList( IOEthernetAddress *addrs, UInt32 count )
{
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
#endif /* BCMDBG || BCMDBG_ERR */

	nMcast = 0;	// while we're mucking with list
	UInt32  uniqCnt = 0;
	for (UInt32 i = 0; i < count; ++i)
	{
		if (uniqCnt == MAXMULTILIST)
		{
			WL_ERROR(( "*** Too many MULTICASTS ***\n" ));
			break;
		}

			// search for dup's before adding to table (AppleTalk bug?)
		UInt32 j = 0;
		for (j = 0; j < uniqCnt; ++j )
			if (BCMPADRS( &addrs[i], &mcastList[j]) == 0) {
				WL_ERROR(("AirPort_Brcm43xx::setMulticastList: duplicate multicast %s\n",
					bcm_ether_ntoa((struct ether_addr*)(&addrs[i]), eabuf)));
				break;		// found a dup
			}
		if (j == uniqCnt)		// then this adrs is a new one
			BCOPYADRS( &addrs[i], mcastList + uniqCnt++ );
	}
	nMcast = uniqCnt;

	populateMulticastList();

	return kIOReturnSuccess;
}

// Coalesce a packet coming from the stack for better performance during DMA.
mbuf_t
AirPort_Brcm43xx::coalesce_output(mbuf_t m, bool coalesce, bool free_orig)
{
	mbuf_t tmp = NULL, new_m = NULL;
	uint len = 0;
	uint count = 0;
	ASSERT(m);

	// Calculate the total length and total segments
	for (tmp = m; tmp; tmp = mbuf_next(tmp)) {
		count++;
		len += mbuf_len(tmp);
	}

	// Allow for couple of more for addition HEADER PROC and TAIL
	if (count > (MAX_DMA_SEGS - 2))
		coalesce = TRUE;
	else
		coalesce = (coalesce || coalesce_packet);

#ifdef BCMDMASGLISTOSL
	// Just conver the existing packet w/o copying. Few environments still require packetcopying for
	// better throughput
	if (!coalesce)
		return osl_pkt_frmnative(osh, m);
#endif /* BCMDMASGLISTOSL */

	// Allocate a new mbuf to account for TXOFF, the original length, and possible tailroom
	new_m = osl_pktget(osh, len + TXOFF + TXTAIL);
	if (!new_m) {
		WL_ERROR(( "coalesce_packet: osk_pktget() failed, pkt dropped!\n" ));
		freePacket(m);
		return (NULL);
	}

	// Set the data pointer to start from TXOFF
	mbuf_setdata(new_m, ((uint8*)mbuf_datastart(new_m)) + TXOFF, len);

	// Walk the chain and Coalesce
	for (tmp = m, len = 0; tmp; tmp = mbuf_next(tmp)){
		mbuf_copydata(tmp, 0, mbuf_len(tmp), (void *)((uint8*)mbuf_data(new_m) + len));
		len += mbuf_len(tmp);
	}

#ifdef WME
	/* Move the AC to adjust the prio if WME is set */
	APPLE80211_MBUF_SET_WME_AC(new_m, APPLE80211_MBUF_WME_AC(m));
#else
	PKTSETPRIO(0);
#endif

	// Free the original
	if (free_orig) {
		freePacket(m);
		m = NULL;
	}

	return(new_m);
}


#ifdef MACOS_VIRTIF
struct wlc_if *
AirPort_Brcm43xx::apif_wlcif()
{
	return apif.wlcif;
}

wl_if_t *
AirPort_Brcm43xx::getapif()
{
	return (&apif);
}

IOReturn
AirPort_Brcm43xx::handleVirtIOC(void *target, void * param0, void *param1, void *param2, void *param3)
{
	/* target == NULL, param0 == apple80211req *req, param1 =userioc, 2, 3 = NULL */
	AirPort_Brcm43xx *wlObj = ((AirPort_Brcm43xx *)target);
	WL_PORT(("%s %d %d\n", __FUNCTION__, __LINE__, *((bool *)param1)));

	return wlObj->wlIocCommon((struct apple80211req *)param0, wlObj, *((bool *)param1));
}

UInt32
AirPort_Brcm43xx::outputPacketVirtIf( mbuf_t m, void *param )
{
#ifdef BCMDBG
	AirPort_Brcm43xx *myif = (AirPort_Brcm43xx *)param;
#endif
	WL_APSTA_TX(("%s %d %d\n", __FUNCTION__, __LINE__, myif == this));

	APPLE80211_MBUF_SET_WME_AC( m, 0);

	return getCommandGate()->runAction(AirPort_Brcm43xx::outputPacketGated, (void *)&m, (void *)apif.virtIf,
	                                   (void*)NULL, (void *)NULL);
}

IOReturn
AirPort_Brcm43xx::outputPacketGated( OSObject *owner, void * arg0, void * arg1, void * arg2, void * arg3 )
{
	((AirPort_Brcm43xx *)owner)->outputPacket( *(mbuf_t *)arg0, arg1 );
	return kIOReturnSuccess;
}
#endif /* MACOS_VIRTIF */

UInt32
AirPort_Brcm43xx::outputPacket(mbuf_t m, void * param )
{
	mbuf_t new_m = NULL;
	int prio = 0;
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( (OSObject *)param );
#else
	struct wlc_if * wlcif = NULL;
#endif /* !MACOSX_DHD */
	txflc_t *txflc = NULL;

	ASSERT(m);
	if (!m)
		return kIOReturnOutputSuccess;

	WL_LOCK();

#ifdef IO80211P2P
	IO80211VirtualInterface * vif = (IO80211VirtualInterface *)OSDynamicCast( IO80211VirtualInterface, (OSObject *)param );

	// If a packet comes through on an P2P interface that may not exist anymore
	// drop it!
	if( !wlcif && vif ) {
		WL_ERROR(( "outputPacket: discarded (p2p interface missing)!\n" ));
		freePacket(m);
		WL_UNLOCK();
		return kIOReturnOutputDropped;
	}
#endif

	if (!wlcif) {
		txflc = &intf_txflowcontrol;
	}
	else {
#ifdef MACOS_VIRTIF
		txflc = &wlcif->wlif->intf_txflowcontrol;
#endif
#ifdef IO80211P2P
		txflc = &((AirPort_Brcm43xxP2PInterface *)wlcif->wlif)->intf_txflowcontrol;
#endif
	}

	/*
	 * In NIC mode check the pub->up to take a decission on whether
	 * to send the packet down or not. In case of Full Dongle DHD we cannot
	 * call isUp since it will fire an IOVAR in data path. Now wlSendPkt
	 * has the logic to check  whether the Bus state is UP before taking
	 * the packet further.
	 */
#ifndef MACOSX_DHD
	if (!pub->up)
	{
		freePacket(m);
		WL_UNLOCK();
		return kIOReturnOutputDropped;
	}
#endif /* !MACOSX_DHD */

	prio = APPLE80211_MBUF_WME_AC(m);
	ASSERT(prio >= 0 && prio < OV_NUM_PRIOS);

	if (mboolisset(txflc->txFlowControl, NBITVAL(prio)) && (skip_flowcontrol == FALSE)) {
		WL_UNLOCK();
		return kIOReturnOutputStall;
	}
#ifndef MACOSX_DHD
	if ((new_m = coalesce_output(m)) == NULL)
#else
	if ((new_m = coalesce_output(m, TRUE, TRUE)) == NULL)
#endif /* !MACOSX_DHD */
	{
		WL_UNLOCK();
		return kIOReturnOutputDropped;
	}
#ifndef MACOSX_DHD
	bool discarded = wlc_sendpkt(pWLC, new_m, wlcif);
#else
	bool discarded = wlSendPkt(new_m, (OSObject *)param );
#endif /* !MACOSX_DHD */
	WL_UNLOCK();
	if (discarded) {
		WL_ERROR(( "outputPacket: discarded!\n" ));
		return kIOReturnOutputDropped;
	}
	skip_flowcontrol = FALSE;
	netStats->outputPackets++;
	return kIOReturnOutputSuccess;
}

#ifdef MACOS_AQM

int AirPort_Brcm43xx::oscallback_dotxstatus( struct wlc_info *wlc, void *scb, void *p, void *txs,  uint status)
{
	BCM_REFERENCE(p);
	BCM_REFERENCE(status);
	int err = BCME_OK;
	AirPort_Brcm43xx *controller = wlc ? (AirPort_Brcm43xx*)wlc->wl : NULL;
	AirPort_Brcm43xxInterface *io80211 = controller ? (AirPort_Brcm43xxInterface *)controller->getNetworkInterface() : NULL;
	BCM_REFERENCE(scb);
	BCM_REFERENCE(txs);

	require_action( wlc , exit, err = -1 );
	require_action( io80211 , exit, err = -1 );

	io80211->signalOutputThread();
	return( 0 );

exit:
	return( err );
}

// AQM support logic

IOReturn
AirPort_Brcm43xx::processServiceClassQueue( IONetworkInterface *interface,
												uint32_t *pktcount,  int reapcount, uint32_t *pktsent,
												int ac, int hmark, int sc )
{
	struct wlc_if *wlcif = wlLookupWLCIf( (OSObject *)interface );
	mbuf_t pkt = NULL; mbuf_t nextpkt = NULL;
	mbuf_t pkthead = NULL;
	int discards = 0;
	IOReturn ioret = kIOReturnNoResources;
	int available = 0;
	int txpending = 0;
	int nSent = 0, ndequeue = 0;
	AirPort_Brcm43xxInterface *io80211 = (AirPort_Brcm43xxInterface *)interface;
	BCM_REFERENCE(hmark);

	*pktcount	= 0;
	*pktsent	= 0;

	txpending = io80211->getTxPacketsPending( ac, &available );

	require_action( (txpending >= 0), noresources, ioret = kIOReturnNoResources );
	require_action( (available != 0), noresources, ioret = kIOReturnNoResources );

	// Calculate the number of packets to dequeue
	ndequeue = (available > 0 ? MIN( reapcount, available ) : reapcount);

	// dequeueOutputPacketsWithServiceClass, returns:
	//   kIOReturnSuccess:
	//		Dequeued N number of packets
	//   kIOReturnNoFrames:
	//		No packets dequeued
	//
	// ::processServiceClassQueue returns:
	//   kIOReturnSuccess:
	//	Dequeued >= 0 number of packets sent
	//   kIOReturnNoResources:
	//      No resources available to queue packet to.
	//
	if( (ioret = interface->dequeueOutputPacketsWithServiceClass(
				ndequeue,
				(IOMbufServiceClass)(sc),
				&pkthead, NULL,
				pktcount, NULL ) ) == kIOReturnNoFrames )
	{
		// Network stack has no more frames/rate limited
		return( kIOReturnNoFrames );
	}

	pkt   = pkthead;

	WL_LOCK();

	while( pkt )
	{
		// Get next packet in chain
		nextpkt = mbuf_nextpkt( pkt );

		// Driver does not expect chained packets
		mbuf_setnextpkt( pkt, NULL );

		// Packet already classified properly by IO80211 Family, get from the mbuf API
		ac = mbuf_get_traffic_class( pkt );
		APPLE80211_MBUF_SET_WME_AC( pkt, ac ); // Set AC, called before coalesce_output

		if( (pkt = coalesce_output(pkt)) == NULL )
		{
			// Failed coalescing, continue w/next packet
			discards++;
			pkt = nextpkt;
			continue;
		}


		if( !wlc_sendpkt(pWLC, pkt, wlcif) )
		{
			nSent++;
			netStats->outputPackets++;
		}
		else
		{
			discards++;
		}


		pkt = nextpkt;
	}

	WL_UNLOCK();

	ioret = ((discards > 0) ? kIOReturnNoResources : kIOReturnSuccess);
	*pktsent = nSent;

	if(ioret == kIOReturnNoResources)
	{
		goto noresources;
	}

	// Have available resources, potentially sent packets
	return ioret;

noresources:
	// No resources

#ifdef BCMDBG
	WL_PORT( ( "%s: No resources: AC[%d] hmark[%d] available[%d] txpending[%d] ndequeue[%d] nSent[%d] discards[%d] pktcount[%d] reapcount[%d]\n",
		   __FUNCTION__,
		   ac, hmark, available, txpending, ndequeue, nSent, discards, *pktcount, reapcount ) );
#endif /* BCMDBG */

	return( kIOReturnNoResources );
}


// static
IOReturn
AirPort_Brcm43xx::processAllServiceClassQueues( OSObject *owner, void *arg0, void *arg1, void *arg2, void *arg3 )
{
	IOReturn rc[4] = { kIOReturnSuccess, kIOReturnSuccess, kIOReturnSuccess, kIOReturnSuccess };
	AirPort_Brcm43xx *controller = ((AirPort_Brcm43xx *)owner);
	IONetworkInterface *interface = arg0 ? (IONetworkInterface *)arg0 : NULL;

	const int reapmin = controller->_aqmTunables.reapmin;
	int reapcount = 0;
	uint32_t pktcount = 0;
	uint32_t pktsent[4] = { 0, 0, 0, 0 };
	uint32_t pktsDequeued = 0;
	BCM_REFERENCE(arg1);
	BCM_REFERENCE(arg2);
	BCM_REFERENCE(arg3);

	if (interface == NULL)
		return kIOReturnError;

	do
	{

		///////////////////////////////////////////////////////////////////////////////////////////////////
		//
		//  NOTES:
		//
		//  When ::processServiceClassQueue returns these return values:
		//
		//  <-- kIOReturnNoResources
		//        MEANS
		//          Driver could not send the requested packets (possible discards), or driver has available space to send.
		//          Driver should still continue to process and dequeue network stack packets for queueing to the other AC queues
		//        ASSERT CONDITIONS
		//          pktsent >= 0
		//
		//  <-- kIOReturnSuccess
		//        MEANS
		//          Driver sent >= 0 frames, including network stack rate limited
		//        ASSERT CONDITIONS
		//          pktsent >= 0
		//


		// Reset
		pktsDequeued = 0;
		reapcount = controller->_aqmTunables.reapcount;

		// Process AC: VO
		pktsent[ APPLE80211_WME_AC_VO ] = 0;
		rc[ APPLE80211_WME_AC_VO ] = controller->processServiceClassQueue(
			interface,
			&pktcount, reapcount,
			&pktsent[ APPLE80211_WME_AC_VO ],
			APPLE80211_WME_AC_VO,
			0,
			kIOMbufServiceClassVO );
		pktsDequeued += pktsent[ APPLE80211_WME_AC_VO ];
		reapcount -= pktsent[ APPLE80211_WME_AC_VO ];

		// Process AC: VI
		reapcount  = (reapcount < reapmin) ? reapmin : reapcount;

		pktsent[ APPLE80211_WME_AC_VI ] = 0;
		rc[ APPLE80211_WME_AC_VI ] = controller->processServiceClassQueue(
			interface,
			&pktcount, reapcount,
			&pktsent[ APPLE80211_WME_AC_VI ],
			APPLE80211_WME_AC_VI,
			0,
			kIOMbufServiceClassVI );
		pktsDequeued += pktsent[ APPLE80211_WME_AC_VI ];
		reapcount -= pktsent[ APPLE80211_WME_AC_VI ];


		// Process AC: BE
		reapcount  = (reapcount < reapmin) ? reapmin : reapcount;

		pktsent[ APPLE80211_WME_AC_BE ] = 0;
		rc[ APPLE80211_WME_AC_BE ] = controller->processServiceClassQueue(
			interface,
			&pktcount, reapcount,
			&pktsent[ APPLE80211_WME_AC_BE ],
			APPLE80211_WME_AC_BE,
			0,
			kIOMbufServiceClassBE );
		pktsDequeued += pktsent[ APPLE80211_WME_AC_BE ];
		reapcount -= pktsent[ APPLE80211_WME_AC_BE ];


		// Process AC: BK
		reapcount  = (reapcount < reapmin) ? reapmin : reapcount;

		pktsent[ APPLE80211_WME_AC_BK ] = 0;
		rc[APPLE80211_WME_AC_BK] = controller->processServiceClassQueue(
			interface,
			&pktcount, reapcount,
			&pktsent[ APPLE80211_WME_AC_BK ],
			APPLE80211_WME_AC_BK,
			0,
			kIOMbufServiceClassBK );
		pktsDequeued += pktsent[ APPLE80211_WME_AC_BK ];


		// Check for unconditional break-out conditions ??
	}
	while( pktsDequeued ); // Continue until: No packets dequeued/rate limited


	// Check for no resources on AC queues
	if( (rc[ APPLE80211_WME_AC_VO ] == kIOReturnNoResources) ||
	    (rc[ APPLE80211_WME_AC_VI ] == kIOReturnNoResources) ||
	    (rc[ APPLE80211_WME_AC_BE ] == kIOReturnNoResources) ||
	    (rc[ APPLE80211_WME_AC_BK ] == kIOReturnNoResources) )
	{
		// If any AC queue have no resources
		return( kIOReturnNoResources );
	}

	return kIOReturnSuccess;
}

IOReturn
AirPort_Brcm43xx::outputStart( IONetworkInterface *    interface,
			       IOOptionBits            options )
{
	BCM_REFERENCE(options);
	IOReturn rc = kIOReturnSuccess;
	struct wlc_if *wlcif = interface ? wlLookupWLCIf( (OSObject *)interface ) : NULL;
	int sched = getOutputPacketSchedulingModel();
	AirPort_Brcm43xxInterface *io80211 = (AirPort_Brcm43xxInterface *)interface;
	require_action( pub->up, noresources, rc = kIOReturnNoResources );
	// Check if HW is up, HW should be enabled else return kIOReturnOffline

	if( sched == IONetworkInterface::kOutputPacketSchedulingModelDriverManaged )
	{
		// Driver managed scheduling

		rc = AirPort_Brcm43xx::processAllServiceClassQueues( this, interface, wlcif, NULL, NULL );
		if(rc == kIOReturnNoResources)
		{
			// No resources available
			goto noresources;
		}

		// Sent packets
	}

	// Driver has scheduled/pulled packets on one or more class queues, then signal the thread to wakeup to
	//   re-enter the outputStart, until the network stack has no more packets to dequeue, or one or more
	//   of the driver AC class queues has no available resources.
	// If the network thread is already scheduled for wakeup and the network stack has no packets,
	//   then the thread sleeps until a packet is sent to the network stack.
	//
	io80211->signalOutputThread();

	return( rc );

noresources:
	// No more driver resources are available to dequeue packets from the network stack.
	// Returning kIOReturnNoResources makes the network thread sleep until next network stack
	//   packet gets enqueued.
	//
	// The driver returns kIOReturnNoResources when an AC queue pending packets goes above a high water mark,
	//   or when sending packets to the driver and the packet gets discarded for some reason.
	// The assumption here is that there are additional outgoing network packets. On each sending of a packet, the network thread
	//   and eventually calls into ::outputStart, which then starts dequeueing of all the AC queues, and
	//   sending packets is resumed.  A improvement is to add a watchdog to do
	//   signalOutputThread to always make sure that this condition never happens.
	//
	return( kIOReturnNoResources );
}
#endif /* MACOS_AQM */


bool
AirPort_Brcm43xx::checkBoardId( const char *cmpboardid )
{
	OSReturn bresult = false;
	IOPlatformExpert *platformExpert = NULL;
	IOService *rootNode = NULL;
	OSObject *prop = NULL;
	OSData *boardID = NULL;
	unsigned int length = 0;
	int cplen = 0, cmpsz = 0;
	char boardstr[128];

	memset( &boardstr[0], 0, sizeof(boardstr) );

	require_action( cmpboardid, exit, bresult = false );
	require_action( (platformExpert = getPlatform()), exit, bresult = false );
	require_action( (rootNode = platformExpert->getProvider()), exit, bresult = false );
	require_action( (prop = rootNode->getProperty(kBoardId)), exit, bresult = false );
	require_action( (boardID = OSDynamicCast(OSData, prop)), exit, bresult = false );
	require_action( ((length = boardID->getLength()) > 0), exit, bresult = false );

	cplen = (length < sizeof(boardstr) ? length : sizeof(boardstr));
	bcopy(boardID->getBytesNoCopy(), &boardstr[0], cplen);
	cmpsz = strnlen( cmpboardid, sizeof(boardstr)-1 ) + 1;
	bresult = (boardID->isEqualTo( cmpboardid, cmpsz ));

	return bresult;

exit:
	return false;
}


// -----------------------------------------------------------------------------
// send a Mac OS native mbuf_t packet up the stack through the inputQueue
//
void
AirPort_Brcm43xx::inputPacketQueued(mbuf_t m, wl_if_t *wlif, int16 rssi)
{
	BCM_REFERENCE(rssi);
	if (!wlif)
		(void)inputQueue->enqueueWithDrop((mbuf_t)m);

	else {
#ifdef IO80211P2P
		((AirPort_Brcm43xxP2PInterface *)wlif)->inputPacket(m);
#else
#ifdef MACOS_VIRTIF
		ASSERT(wlif == &apif);
		mysecondIF->inputPacket(m, mbuf_len(m));
#else
		freePacket(m);
#endif /* MACOS_VIRTIF */
#endif /* IO80211P2P */
	}

}

// -----------------------------------------------------------------------------
IOReturn
AirPort_Brcm43xx::setCipherKey( UInt8 *keyData, int keylen, int keyIdx, bool txKey )
{
// 	if (keylen != 0)
// 		WL_ERROR(( "setCipherKey %c: len %d, idx %d\n", txKey ? 'T' : 'R', keylen, keyIdx ));

	wl_wsec_key_t	key = {};
	bzero( &key, sizeof(key) );		// zero out unused fields

	key.len = keylen;
	key.index = keyIdx;

	if (keylen == 5)
		key.algo = CRYPTO_ALGO_WEP1;
	else if (keylen == 13)
		key.algo = CRYPTO_ALGO_WEP128;
	else if (keylen != 0) {
		WL_ERROR(( "setCipherKey: bad keylen = %d\n", keylen ));
		return kIOReturnBadArgument;
	}

	if (keylen && txKey)
		key.flags = WL_PRIMARY_KEY;			// this is also the tx key

	if (keylen && keyData)
		bcopy( keyData, key.data, keylen );
	(void) wlIoctl(WLC_SET_KEY, &key, sizeof(key), TRUE, NULL );

	return kIOReturnSuccess;
}


// -----------------------------------------------------------------------------
OSString *
AirPort_Brcm43xx::getVersionString()
{
	return OSString::withCStringNoCopy( kVersionString );
}
#ifdef IO80211P2P
#ifdef FAST_ACTFRM_TX
void
AirPort_Brcm43xx::restartActFrmQueue( void )
{
	IOGatedOutputQueue *queue = NULL;
	struct virt_intf_entry *elt = NULL;

	// we can safely service queues, even if they are not stalled.

	actFrmPending = false;

	// restart the primary interface act frm queue first
	queue = myNetworkIF->getOutputQueueForDLT(APPLE80211_ACT_FRM_DLT);
	ASSERT(queue != NULL);
	queue->service(IOBasicOutputQueue::kServiceAsync);
	// restart the virtual interface act frm queues if they exist
	SLIST_FOREACH(elt, &virt_intf_list, entries) {
		queue = elt->obj->getOutputQueueForDLT(APPLE80211_ACT_FRM_DLT);
		ASSERT(queue != NULL);
		queue->service(IOBasicOutputQueue::kServiceAsync);
	}
}
#endif // FAST_ACTFRM_TX

UInt32
AirPort_Brcm43xx::outputActionFrame( OSObject * interface, mbuf_t m )
{
	wl_af_params_t params = {};
	mbuf_t temp = NULL;
	int err = BCME_OK;

	if( mbuf_pkthdr_len( m ) > ACTION_FRAME_SIZE + sizeof( struct apple80211_act_frm_hdr ) ||
	    ( mbuf_len( m ) < sizeof( struct apple80211_act_frm_hdr ) &&
	      mbuf_pullup( &m, sizeof( struct apple80211_act_frm_hdr ) ) ) )
	{
		return kIOReturnOutputDropped;
	}

#ifdef FAST_ACTFRM_TX
	if (actFrmPending) {
		return kIOReturnOutputStall;
	}
#endif

	struct apple80211_act_frm_hdr * appleHdr = (struct apple80211_act_frm_hdr *)mbuf_data( m );

	params.channel = appleHdr->chan.channel;
	params.dwell_time = appleHdr->dwell_ms;
#ifdef FAST_ACTFRM_TX
	if (params.dwell_time < 50) {
		// force minimum dwell time of 50mS for now
		// in order to give enough time for retries to finish
		WL_ERROR(("dwell_time %d too small, changed to 50\n", params.dwell_time));
		params.dwell_time = 50;
	}
	if (params.channel == 0) {
		// force channel to home channel if user passed in 0
		params.channel = CHSPEC_CHANNEL(pWLC->home_chanspec);
		WL_ERROR(("channel is 0, changed to home channel %d\n", params.channel));
	}
#endif
	memcpy( &params.action_frame.da, &appleHdr->addr, sizeof( params.action_frame.da ) );

	// Pass in other BSSID is ok
	memcpy( &params.BSSID, &appleHdr->addr, sizeof( params.BSSID ) );
	mbuf_setdata( m, (UInt8 *)mbuf_data( m ) + sizeof( *appleHdr ), mbuf_len( m  ) - sizeof( *appleHdr ) );

	params.action_frame.len = 0;

	for( temp = m; temp; temp = mbuf_next( temp ) )
	{
		memcpy( &params.action_frame.data[params.action_frame.len], mbuf_data( temp ), mbuf_len( temp ) );
		params.action_frame.len+=mbuf_len( temp );
	}

	params.action_frame.packetId = actFrmID++;

	WL_LOCK();
	err = wlIovarOp( "actframe",
	                    NULL,
	                    0,
	                    &params,
	                    sizeof( params ),
	                    IOV_SET,
	                    interface );
	WL_UNLOCK();

#ifdef FAST_ACTFRM_TX
	if (err == BCME_OK) {
		actFrmPending = true;
	}
	else {
		mbuf_freem( m );
		return kIOReturnOutputDropped;
	}
#endif

#if VERSION_MAJOR > 10
	// Apple change: Support for APPLE80211_M_ACT_FRM_TX_COMPLETE
	if( err == BCME_OK )
	{
		memcpy( &_actFrmCompleteData.dst, &appleHdr->addr, sizeof( appleHdr->addr ) );
		_actFrmCompleteData.token = appleHdr->token;
	}
#endif // VERSION_MAJOR > 10

	mbuf_freem( m );
	return kIOReturnOutputSuccess;
}

UInt32
AirPort_Brcm43xx::bpfOutputPacket( OSObject * interface, UInt32 dlt, mbuf_t m )
{
	UInt32 ret = 0;

	if( dlt == APPLE80211_ACT_FRM_DLT )
	{
		ret = outputActionFrame( interface, m );
	}
	else if ( dlt == DLT_IEEE802_11_RADIO || dlt == DLT_IEEE802_11 )
	{
		ret = bpfOutput80211Radio(interface, m);
	}
	else
	{
		mbuf_freem( m );
		ret = kIOReturnOutputDropped;
	}

	return ret;
}

UInt32
AirPort_Brcm43xx::bpfOutput80211Radio( OSObject * interface, mbuf_t m )
{
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
	struct ieee80211_radiotap_header rth = {};
	mbuf_t new_m = NULL;
	uint header_len = 0;
	size_t mlen = 0;
	uint8 flags = 0;
	uint8 *rtap_header = NULL;
	ratespec_t rspec = 0;
	int err = BCME_OK;

	mlen = mbuf_pkthdr_len(m);

	if (mlen < sizeof(rth)) {
		WL_ERROR(("wl%d: %s: mbuf too short %lu, expected at least %lu for radiotap header\n",
		          unit, __FUNCTION__, mlen, sizeof(rth)));
		goto drop;
	}

	err = mbuf_copydata(m, 0, sizeof(rth), &rth);
	if (err) {
		WL_ERROR(("wl%d: %s: mbuf_copydata(%lu) err %d\n",
		          unit, __FUNCTION__, sizeof(rth), err));
		goto drop;
	}

	header_len = load16_ua(&rth.it_len);

	if (mlen < header_len) {
		WL_ERROR(("wl%d: %s: mbuf too short %lu, expected at least %u based on radiotap.it_len\n",
		          unit, __FUNCTION__, mlen, header_len));
		goto drop;
	}

	/* copy out the rtap header */
	rtap_header = (uint8*)MALLOC(osh, header_len);
	if (rtap_header == NULL)
		goto drop;

	err = mbuf_copydata(m, 0, header_len, rtap_header);
	if (err) {
		WL_ERROR(("wl%d: %s: mbuf_copydata(%u) err %d\n",
		          unit, __FUNCTION__, header_len, err));
		goto drop;
	}

	/* strip off the rtap header */
	mbuf_adj(m, header_len);
	mlen -= header_len;

	// Check if we need to strip the FCS
	if (wl_rtapFlags(rtap_header, &flags) && (flags & IEEE80211_RADIOTAP_F_FCS))
	{
		mbuf_adj(m, -DOT11_FCS_LEN);
		mlen -= DOT11_FCS_LEN;
	}

	/* create our native packet with headroom */
	new_m = osl_pktget(osh, mlen + TXOFF + TXTAIL);
	if (!new_m) {
		WL_ERROR(( "%s: osl_pktget() failed, pkt dropped!\n", __FUNCTION__));
		goto drop;
	}

	// Set the data pointer to start from TXOFF
	mbuf_setdata(new_m, ((uint8*)mbuf_datastart(new_m)) + TXOFF, mlen);

	// Copy the payload from m to new_m
	err = mbuf_copydata(m, 0, mlen, mbuf_data(new_m));
	if (err) {
		WL_ERROR(("wl%d: %s: mbuf_copydata(%lu) err %d\n",
		          unit, __FUNCTION__, mlen, err));
		goto drop;
	}

	// Free the original
	freePacket(m);
	m = NULL;

	rspec = wl_calcRspecFromRTap(rtap_header);
	PKTSETPRIO(new_m, PRIO_8021D_BE); // until we parse the prio from frame

	wlc_send80211_specified(pWLC, new_m, rspec, wlcif);

	MFREE(osh, rtap_header, header_len);

	return 0;

drop:
	if (rtap_header != NULL)
		MFREE(osh, rtap_header, header_len);

	mbuf_freem(m);
	return kIOReturnOutputDropped;
}

// configure p2p features as specified by the following parameters
//	intf: interface to configure p2p features on
//	features: bit field containing the features to turn ON or OFF
//	enab: TRUE if turning ON, FALSE if turning OFF
int
AirPort_Brcm43xx::ConfigP2PFeatures(AirPort_Brcm43xxP2PInterface *intf, int features, bool enab)
{
	int curr_features = 0;
	int new_features = 0;
	int err = BCME_OK;

	// Get the current p2p_features value first
	(void) wlIovarOp( "p2p_features",
	              NULL,
	              0,
	              &curr_features,
	              sizeof( curr_features ),
	              IOV_GET,
	              intf );
	// get the new p2p_features value
	if (enab == TRUE) {
		// enable features
		new_features = curr_features | features;
	}
	else {
		// disable features
		new_features = curr_features & ~features;
	}
	// Update the p2p_features GO_NOLEGACY flag
	if (new_features == curr_features) {
		// features remain the same, don't do anything
		return err;
	}

	err = wlIovarOp( "p2p_features",
	                    NULL,
	                    0,
	                    &new_features,
	                    sizeof( new_features ),
	                    IOV_SET,
	                    intf );

	return (err);
}

#endif /* IO80211P2P */

UInt32
AirPort_Brcm43xx::monitorPacketHeaderLength( IO80211Interface * interface )
{
	BCM_REFERENCE(interface);
	return AVS_HEADER_LEN;
}

UInt32
AirPort_Brcm43xx::monitorDLT( IO80211Interface * interface )
{
	BCM_REFERENCE(interface);
	return DLT_IEEE802_11_RADIO_AVS;
}

SInt32
AirPort_Brcm43xx::monitorModeSetEnabled( IO80211Interface * interface, bool enabled, UInt32 dlt )
{

	IOLog( "AirPort_Brcm43xx::monitorModeSetEnabled\n" );
	if (dlt != DLT_IEEE802_11_RADIO_AVS &&
	    dlt != DLT_IEEE802_11 &&
	    dlt != DLT_IEEE802_11_RADIO &&
	    dlt != DLT_PPI)
		return EPROTONOSUPPORT;

	int on = enabled ? 1 : 0;
	int err = BCME_OK;
	int new_avs = avsEnabled;
	int new_ieee80211 = ieee80211Enabled;
	int new_bsd = bsdEnabled;
	int new_ppi = ppiEnabled;
	BCM_REFERENCE(interface);

	switch (dlt) {
	case DLT_IEEE802_11_RADIO_AVS:
		new_avs = on;
		break;

	case DLT_IEEE802_11:
		new_ieee80211 = on;
		break;

	case DLT_IEEE802_11_RADIO:
		new_bsd = on;
		break;

	case DLT_PPI:
		new_ppi = on;
		break;
	}


	// check for a transition from all off to something on
	if( (on == ON) && ( !avsEnabled && !ieee80211Enabled && !bsdEnabled && !ppiEnabled ) )
	{
		err = wlIoctlSetInt(WLC_SET_MONITOR, on );
		if( !err )
			IOLog( "Monitor mode on\n" );
	}
	// check for a transition from something on to all off
	else if( (on == OFF) && ( avsEnabled || ieee80211Enabled || bsdEnabled || ppiEnabled ) )
	{
		if( !new_avs && !new_ieee80211 && !new_bsd && !new_ppi)
		{
			err = wlIoctlSetInt(WLC_SET_MONITOR, on );
			if( !err )
				IOLog( "Monitor mode off\n" );
		}
	}

	// update the enables if there was no error changing the monitor mode (or if there was no change)
	if( !err )
	{
		avsEnabled  = (bool)new_avs;
		ieee80211Enabled = (bool)new_ieee80211;
		bsdEnabled = (bool)new_bsd;
		ppiEnabled = (bool)new_ppi;
	}

	return osl_error(err);
}

UInt32
AirPort_Brcm43xx::hardwareOutputQueueDepth( IO80211Interface * interface )
{
	BCM_REFERENCE(interface);
#ifndef MACOSX_DHD
	return (UInt32)(wlc_txpktcnt(pWLC));
#else
	return 0;
#endif /* !MACOSX_DHD */
}

#ifdef MACOSX_DHD
extern "C" {
static bool
isprint(int c)
{
	if( (c >= 0x20) && (c <= 0x7E) ) {
		return TRUE;
	}
	return FALSE;
}

static int
wl_format_ssid(char* buf, const uchar ssid[], int ssid_len)
{
	uint i, c;
	char *p = buf;
	char *endp = buf + SSID_FMT_BUF_LEN;

	if ((buf == NULL) || (ssid == NULL)) {
		WL_ERROR(("%s NULL input buf: %p ssid: %p\n", __func__, buf, ssid));
		ASSERT(0);
		return BCME_BADARG;
	}

	if (ssid_len > DOT11_MAX_SSID_LEN) {
		ssid_len = DOT11_MAX_SSID_LEN;
	}

	for (i = 0; i < ssid_len; i++) {
		c = (uint)ssid[i];
		if (c == '\\') {
			*p++ = '\\';
			*p++ = '\\';
		} else if (bcm_isprint((uchar)c)) {
			*p++ = (char)c;
		} else {
			p += snprintf(p, (endp - p), "\\x%02X", c);
		}
	}
	*p = '\0';
	ASSERT(p < endp);

	return (int)(p - buf);
}

static bool wl_down_for_mpc(AirPort_Brcm43xx *wlobj, bool wlc_up)
{
	int err;
	int radio_disabled;
	int down_override;
	err = wlobj->wlIoctlGetInt(WLC_GET_RADIO, &radio_disabled);
	if (err) {
		WL_ERROR(("%s: WLC_GET_RADIO returned %d\n", __FUNCTION__, err));
		return false;
	}
	err = wlobj->wlIovarGetInt("down_override", &down_override);
	if (err) {
		WL_ERROR(("%s: IOV_DOWN_OVERRIDE returned %d\n", __FUNCTION__, err));
		return false;
	}
	WL_IOCTL(("wlc_up: %d down_override: %d radio_disabled: 0x%x\n",
			wlc_up, down_override, radio_disabled));
	if (!wlc_up && !down_override &&
	    mboolisset(radio_disabled, WL_RADIO_MPC_DISABLE) &&
	    !mboolisset(radio_disabled, ~WL_RADIO_MPC_DISABLE))
		return TRUE;

	return FALSE;
}
} /* extern C */
#endif /* MACOSX_DHD */

#pragma mark -
#pragma mark IO80211Controller IOCTL Get Handlers

// -----------------------------------------------------------------------------
//
// IO80211Controller IOCTL Get Handlers
//
// -----------------------------------------------------------------------------

SInt32
AirPort_Brcm43xx::getSSID( OSObject * interface, struct apple80211_ssid_data * sd )
{
	wlc_ssid_t ssid = {};
#ifdef BCMDBG
	char ssidbuf[SSID_FMT_BUF_LEN] = {0};
#endif
	BCM_REFERENCE(interface);
	ASSERT(sd);
	if (!sd)
		return EINVAL;
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );

	bool associated = wlcif ? wlcif->u.bsscfg->associated : wl_find_bsscfg()->associated;
#else
	bool associated = isAssociated();
#endif /* !MACOSX_DHD */
	if (!associated || IFMTHD( interface, linkState ) != kIO80211NetworkLinkUp) {

		// Apple change: WPA2-PSK IBSS support
		if( ibssRunning )
		{
			sd->ssid_len = ibssSSIDLen;
			memcpy( sd->ssid_bytes, ibssSSID, ibssSSIDLen );
			return 0;
		}

		WL_IOCTL(("wl%d: getSSID: not associated\n", unit));
		return ENXIO;
	}
	// Apple change: Need to return the SSID when in STA, IBSS and AP mode.
	if( wlIoctl(WLC_GET_SSID, &ssid, sizeof(ssid), FALSE, interface ) != 0 )
		return ENXIO;

	if( ssid.SSID_len <= APPLE80211_MAX_SSID_LEN )
		sd->ssid_len = ssid.SSID_len;
	else
		sd->ssid_len = APPLE80211_MAX_SSID_LEN;

	memcpy(sd->ssid_bytes, ssid.SSID, sd->ssid_len);
#ifndef MACOSX_DHD
	WL_IOCTL(("wl%d: getSSID: \"%s\"\n", unit,
	          (wlc_format_ssid(ssidbuf, sd->ssid_bytes, sd->ssid_len), ssidbuf)));
#else
	WL_IOCTL(("wl%d: getSSID: \"%s\"\n", unit,
	          (wl_format_ssid(ssidbuf, sd->ssid_bytes, sd->ssid_len), ssidbuf)));
#endif /* !MACOSX_DHD */
	return 0;
}

OSData *
AirPort_Brcm43xx::getCIPHER_KEY( OSObject * interface, int index )
{
	wl_wsec_key_t	key = {};
	apple80211_key appleKey = {};
	int error = 0;
	OSData * keyData = NULL;
	BCM_REFERENCE(interface);

	WL_IOCTL(("wl%d: getCIPHER_KEY(index = %d)\n", unit, index));

	bzero( &key, sizeof( key ) );
	bzero( &appleKey, sizeof( appleKey ) );

	key.index = index;

#ifndef MACOSX_DHD
	if ((wl_find_bsscfg()->associated) &&
		(error = wlIoctl(WLC_GET_KEY, &key, sizeof( key ), FALSE, NULL )) == 0)
#else
	if (isAssociated() &&
		(error = wlIoctl(WLC_GET_KEY, &key, sizeof( key ), FALSE, NULL )) == 0)
#endif /* !MACOSX_DHD */
	{
		appleKey.version = APPLE80211_VERSION;
		appleKey.key_len = key.len;
		appleKey.key_index = (UInt16)key.index;

		switch( key.algo )
		{
			case CRYPTO_ALGO_OFF:
				appleKey.key_cipher_type = APPLE80211_CIPHER_NONE;
				break;
			case CRYPTO_ALGO_WEP1:
				appleKey.key_cipher_type = APPLE80211_CIPHER_WEP_40;
				break;
			case CRYPTO_ALGO_TKIP:
				appleKey.key_cipher_type = APPLE80211_CIPHER_TKIP;
				break;
			case CRYPTO_ALGO_WEP128:
				appleKey.key_cipher_type = APPLE80211_CIPHER_WEP_104;
				break;
			case CRYPTO_ALGO_AES_CCM:
				appleKey.key_cipher_type = APPLE80211_CIPHER_AES_CCM;
				break;
			case CRYPTO_ALGO_AES_OCB_MSDU:
			case CRYPTO_ALGO_AES_OCB_MPDU:
				appleKey.key_cipher_type = APPLE80211_CIPHER_AES_OCB;
				break;
		}

		if( appleKey.key_len > APPLE80211_KEY_BUFF_LEN )
			appleKey.key_len = APPLE80211_KEY_BUFF_LEN;

		bcopy( key.data, appleKey.key, appleKey.key_len );

		keyData = OSData::withBytes( &appleKey, sizeof( appleKey ) );
	}

	return keyData;
}

SInt32
AirPort_Brcm43xx::getCHANNEL( OSObject * interface, struct apple80211_channel_data * cd )
{
	uint32 chanspec = 0;
	int err = BCME_OK;
	bool up;
#ifdef MACOSX_DHD
	int isup = isUp();
#endif /* MACOSX_DHD */
	ASSERT(cd);
	if (!cd)
		return EINVAL;

#ifdef MACOSX_DHD
#ifdef AAPLPWROFF
	up = (_powerState == PS_INDEX_ON) && (isup || wl_down_for_mpc(this, isup));
#else
	up = (isup || wl_down_for_mpc(this, isup));
#endif /* AAPLPWROFF */
#else /* MACOSX_DHD */
#ifdef AAPLPWROFF
	up = (_powerState == PS_INDEX_ON) && (pub->up || wlc_down_for_mpc(pWLC));
#else
	up = (pub->up || wlc_down_for_mpc(pWLC));
#endif /* AAPLPWROFF */
#endif /* MACOSX_DHD */
	if (!up) {
		return ENXIO;
	}

	err = wlIovarOp( "chanspec", NULL, 0, &chanspec, sizeof( chanspec ), IOV_GET, interface );
	if (err) {
		WL_IOCTL(("wl%d: getCHANNEL: error \"%s\" (%d) from iovar \"chanspec\"\n",
			unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	cd->version = APPLE80211_VERSION;
	chanspec2applechannel(chanspec, &cd->channel);

	WL_IOCTL(("wl%d: getCHANNEL: channel %d%s (%s GHz)\n", unit,
	          cd->channel.channel,
	          ((cd->channel.flags & APPLE80211_C_FLAG_40MHZ) ?
	           ((cd->channel.flags & APPLE80211_C_FLAG_EXT_ABV) ? "l":"u") :
	           ""),
	          (cd->channel.flags & APPLE80211_C_FLAG_2GHZ)?"2.4":"5"));
	return 0;
}

SInt32
AirPort_Brcm43xx::getTXPOWER( OSObject * interface, struct apple80211_txpower_data * td )
{
	int qdbm = 0;
	int err = BCME_OK;
	BCM_REFERENCE(interface);
	BCM_REFERENCE(interface);

	ASSERT(td);
	if (!td)
		return EINVAL;

	err = wlIovarGetInt("qtxpower", &qdbm);
	if (err)
		return osl_error(err);

	qdbm &= ~WL_TXPWR_OVERRIDE;

	td->version = APPLE80211_VERSION;
	td->txpower_unit = APPLE80211_UNIT_MW;
	td->txpower = (int32_t)bcm_qdbm_to_mw((uint8)qdbm);

	WL_IOCTL(("wl%d: getTXPOWER: %d mW (%d dBm)\n", unit, td->txpower, qdbm / 4));

	return 0;
}

SInt32
AirPort_Brcm43xx::getBSSID( OSObject * interface, struct apple80211_bssid_data * bd )
{
	int err = BCME_OK;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
#endif
	ASSERT(bd);
	if (!bd)
		return EINVAL;
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );

	bool associated = wlcif ? wlcif->u.bsscfg->associated : wl_find_bsscfg()->associated;
#else
	bool associated = isAssociated();
#endif /* !MACOSX_DHD */
	if (!associated ||
	    (proc_selfpid() != 0 && IFMTHD( interface, linkState ) != kIO80211NetworkLinkUp))
		err = ENXIO;
	else {
		bd->version = APPLE80211_VERSION;
		err = wlIoctl(WLC_GET_BSSID, &bd->bssid, sizeof( bd->bssid ), FALSE, interface );
	}

	if (err) {
		WL_IOCTL(("wl%d: getBSSID: not associated\n", unit));
	} else {
		WL_IOCTL(("wl%d: getBSSID: \"%s\"\n", unit,
			bcm_ether_ntoa((struct ether_addr *)&bd->bssid, eabuf)));
	}

	if (err == ENXIO)
		return err;

	return osl_error(err);
}
#ifndef MACOSX_DHD
// Apple change
bool
AirPort_Brcm43xx::wowCapablePlatform()
{
	char model[32] = {0};

	// If an m93 or m35d card goes into a machine that does not support wake on wireless,
	// add its name to this list.
	if( !PEGetModelName( model, sizeof( model ) ) ||
	    ! strcmp( model, "MacPro3,1" )
	    )
	{
		return false;
	}

#ifdef SUPPORT_FEATURE_WOW
	return true; // SnowLeopard only
#else
	return false;
#endif
}
#endif /* !MACOSX_DHD */
void
AirPort_Brcm43xx::checkStatusCode17( )
{
	intf_info_blk_t *blk = IFINFOBLKP(myNetworkIF);

	WL_PORT(("%s: associatation_status 0x%x sc17_retry_cnt 0x%x\n",
	         __FUNCTION__, blk->association_status, sc17_retry_cnt));

	if (blk->association_status != DOT11_SC_ASSOC_BUSY_FAIL) {
		sc17_retry_cnt = 0;
		return;
	}

	if (sc17_retry_cnt > SC17_MAX_RETRY)
		return;

	sc17_retry_cnt++;
	setSSID(myNetworkIF , &sc17_data);
}

SInt32
AirPort_Brcm43xx::getCARD_CAPABILITIES( OSObject * interface, struct apple80211_capability_data * cd )
{
	ASSERT(cd);
	if (!cd)
		return EINVAL;

	cd->version = APPLE80211_VERSION;

	setbit( cd->capabilities, APPLE80211_CAP_WEP );
	setbit( cd->capabilities, APPLE80211_CAP_TKIP );
	setbit( cd->capabilities, APPLE80211_CAP_AES_CCM );
	setbit( cd->capabilities, APPLE80211_CAP_IBSS );
	setbit( cd->capabilities, APPLE80211_CAP_PMGT );
#ifdef AP
	setbit( cd->capabilities, APPLE80211_CAP_HOSTAP );
#endif
	setbit( cd->capabilities, APPLE80211_CAP_SHSLOT );
	setbit( cd->capabilities, APPLE80211_CAP_SHPREAMBLE );
#ifdef WL_MONITOR
	setbit( cd->capabilities, APPLE80211_CAP_MONITOR );
#endif
	setbit( cd->capabilities, APPLE80211_CAP_TKIPMIC );
	setbit( cd->capabilities, APPLE80211_CAP_WPA1 );
	setbit( cd->capabilities, APPLE80211_CAP_WPA2 );
	setbit( cd->capabilities, APPLE80211_CAP_BURST );
#ifdef WME
	setbit( cd->capabilities, APPLE80211_CAP_WME );
#endif
#ifdef SUPPORT_FEATURE_WOW
	if( wowl_cap && wowl_cap_platform )
		setbit( cd->capabilities, APPLE80211_CAP_WOW );
#endif
#ifndef MACOSX_DHD
#if VERSION_MAJOR > 8
	// Leopard or later only capabilities...
	setbit( cd->capabilities, APPLE80211_CAP_TSN );
#endif
	if (sgi_cap & WLC_N_SGI_20)
		setbit( cd->capabilities, APPLE80211_CAP_SHORT_GI_20MHZ );

	if (sgi_cap & WLC_N_SGI_40)
		setbit( cd->capabilities, APPLE80211_CAP_SHORT_GI_40MHZ );

#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	if (sgi_cap & WLC_VHT_SGI_80)
		setbit( cd->capabilities, APPLE80211_CAP_SHORT_GI_80MHZ );
#endif
#endif /* !MACOSX_DHD */
#if defined(IO80211P2P)
#if VERSION_MAJOR > 10
	setbit( cd->capabilities, APPLE80211_CAP_P2P );
#ifdef MACOSX_DHD
	setbit( cd->capabilities, APPLE80211_CAP_AWDL );
	setbit( cd->capabilities, APPLE80211_CAP_AWDL_SOLO );
#endif /* MACOSX_DHD */
#endif
#ifdef MACOSX_DHD
	setbit( cd->capabilities, APPLE80211_CAP_DUAL_BAND );
#endif /* MACOSX_DHD */
#ifdef WIFI_ACT_FRAME
	setbit( cd->capabilities, APPLE80211_CAP_ACT_FRM );
#endif /* WIFI_ACT_FRAME */
#endif  /* IO80211P2P */

#ifndef MACOSX_DHD
	// Apple change: Add BT coex capability
	setbit( cd->capabilities, APPLE80211_CAP_BT_COEX );
#ifdef APPLE80211_IOC_CHAIN_ACK
	if (revinfo.chipnum == BCM4331_CHIP_ID) {
		setbit( cd->capabilities, APPLE80211_CAP_PER_CHAIN_ACK );
	}
#endif

#ifdef MFP
	// TODO: Need to protect this inside OS Version flag, 10_9 does not have this
	// capability
	setbit( cd->capabilities, 48 /*APPLE80211_CAP_80211W_MFP*/);
#endif

	// add multi scan capability
	setbit( cd->capabilities, APPLE80211_CAP_MULTI_SCAN );

	// Do not use Opportunistic Roam if Roaming Profiles are enabled.
	// (Roaming Profiles are providing the same functionality).
#if defined(WLROAMPROF) && defined(APPLE80211_IOC_ROAM_PROFILE)
	setbit( cd->capabilities, APPLE80211_CAP_ROAM_PROFILES );
#elif defined(OPPORTUNISTIC_ROAM) && defined(APPLE80211_IOC_ROAM)
	// Otherwise add Opportunistic Roam capability
	setbit( cd->capabilities, APPLE80211_CAP_OPP_ROAM );
#endif /* WLROAMPROF || OPPORTUNISTIC_ROAM */
#endif /* !MACOSX_DHD */

#if defined(WLBTCPROF) && defined(APPLE80211_IOC_BTCOEX_PROFILES)
	setbit( cd->capabilities, APPLE80211_CAP_BTCOEX_PROFILES );
#endif /* WLBTCPROF && APPLE80211_IOC_BTCOEX_PROFILES */

#ifdef MACOS_AQM
	if( OSDynamicCast( IO80211Interface, interface )
		&& (getOutputPacketSchedulingModel() >= 0) ) /* schedule configured - AQM enabled */
	{
		// AQM enabled driver, primary interface only
		setbit( cd->capabilities, APPLE80211_CAP_AQM );
	}
#endif /* MACOS_AQM */

#if defined(CNTRY_DEFAULT)
	if (isCntryDefaultSupported(revinfo.boardid)) {
		setbit( cd->capabilities, APPLE80211_CAP_DYNAMIC_COUNTRY_CODE);
	}
#endif
	return 0;
}

SInt32
AirPort_Brcm43xx::getSTATE( OSObject * interface, struct apple80211_state_data * sd )
{
	ASSERT(sd);
	if (!sd)
		return EINVAL;

	sd->version = APPLE80211_VERSION;
	sd->state = APPLE80211_S_INIT;

	// Should APPLE80211_S_RUN be linked to pub->associated? When we tried
	// that the Airport Menu would show connection strength bars even when
	// we had lost beacons.
	if (IFMTHD( interface, linkState ) == kIO80211NetworkLinkUp)
		sd->state = APPLE80211_S_RUN;
	// Should probably track scans and auth attempts internally so we can be
	// more specific later

	WL_IOCTL(("wl%d: getSTATE: %s\n", unit,
	          lookup_name(apple80211_statenames, sd->state)));

	return 0;
}

SInt32
AirPort_Brcm43xx::getPHY_MODE( OSObject * interface, struct apple80211_phymode_data * pd )
{
	int bcmerr = 0;
	int band_support = 0;
	uint32 phy_mode = APPLE80211_MODE_AUTO;
	uint32 active_phy_mode = APPLE80211_MODE_UNKNOWN;
	int nmode = 0;
	int vhtmode = 0;
	wlc_bsscfg_t *bsscfg = NULL;

	BCM_REFERENCE(interface);

	ASSERT(pd);
	if (!pd)
		return EINVAL;

	if ((bcmerr = bandSupport(&band_support)))
		return ENXIO;
#ifndef MACOSX_DHD
	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);
#endif /* !MACOSX_DHD */
	if (band_support == WLC_BAND_ALL || band_support == WLC_BAND_2G)
		phy_mode |= APPLE80211_MODE_11B | APPLE80211_MODE_11G;

	if (band_support == WLC_BAND_ALL || band_support == WLC_BAND_5G)
		phy_mode |= APPLE80211_MODE_11A;

	if ((bcmerr = wlIovarGetInt("nmode", &nmode)))
		nmode = FALSE;

	if (nmode && PHYTYPE_11N_CAP(phytype))
		phy_mode |= APPLE80211_MODE_11N;

	if ((bcmerr = wlIovarGetInt("vhtmode", &vhtmode)))
		vhtmode = FALSE;

#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	if (vhtmode && PHYTYPE_VHT_CAP(phytype))
		phy_mode |= APPLE80211_MODE_11AC;
#endif
#ifndef MACOSX_DHD
	if (bsscfg->associated) {
		wlc_bss_info_t *current_bss = wlc_get_current_bss(bsscfg);
#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
		if (VHT_ENAB(pub) && (current_bss->flags2 & WLC_BSS_VHT))
			active_phy_mode = APPLE80211_MODE_11AC;
		else
#endif
	if (N_ENAB(pub) && (current_bss->flags & WLC_BSS_HT))

			active_phy_mode = APPLE80211_MODE_11N;
		else if (CHSPEC_IS5G(wlc_get_home_chanspec(bsscfg)))
			active_phy_mode = APPLE80211_MODE_11A;
		else {
			bool sup_cck = FALSE, sup_ofdm = FALSE, sup_other = FALSE;
			ratesetModulations(&current_bss->rateset, FALSE,
			                   &sup_cck, &sup_ofdm, &sup_other);
			if (sup_ofdm)
				active_phy_mode = APPLE80211_MODE_11G;
			else
				active_phy_mode = APPLE80211_MODE_11B;
		}
	}

	pd->version = APPLE80211_VERSION;
	pd->phy_mode = phy_mode;
	pd->active_phy_mode = active_phy_mode;
#endif /* !MACOSX_DHD */

	return 0;
}

SInt32
AirPort_Brcm43xx::getOP_MODE( OSObject * interface, struct apple80211_opmode_data * od )
{
#ifndef MACOSX_DHD
	int monitor = 0;
	wlc_bsscfg_t *bsscfg = NULL;
#endif /* !MACOSX_DHD */

	ASSERT(od);
	if (!od)
		return EINVAL;

#ifndef MACOSX_DHD
	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	od->version = APPLE80211_VERSION;
	od->op_mode = 0;

	if( wlIoctlGetInt(WLC_GET_MONITOR, &monitor) == 0 && monitor)
		od->op_mode |= APPLE80211_M_MONITOR;

	if (bsscfg->associated /* interface->linkState() == kIO80211NetworkLinkUp */) {
		if (BSSCFG_AP(bsscfg))
			od->op_mode |= APPLE80211_M_HOSTAP;
		else
			od->op_mode |= bsscfg->BSS ? APPLE80211_M_STA : APPLE80211_M_IBSS;
	}
	else if( ibssRunning )
	{
		// Apple change: WPA2-PSK IBSS support
		od->op_mode |= APPLE80211_M_IBSS;
	}

#if defined(IO80211P2P)
	IO80211P2PInterface * p2pIf = OSDynamicCast( IO80211P2PInterface, interface );

	if( p2pIf && p2pIf->getInterfaceRole() == APPLE80211_VIRT_IF_ROLE_P2P_DEVICE )
	{
		od->op_mode |= APPLE80211_M_P2P_DEV;
	}
#endif

	WL_IOCTL(("wl%d: getOP_MODE: %s%s\n", unit,
	          monitor?"MONITOR ":"",
	          (od->op_mode & APPLE80211_M_HOSTAP) ? "HOSTAP" : (bsscfg->BSS?"STA":"IBSS")));
#else
	od->version = APPLE80211_VERSION;
	od->op_mode = 0;

	if (isAssociated()) {
		od->op_mode |= APPLE80211_M_STA;
	}
	WL_IOCTL(("wl%d: getOP_MODE: %s\n", unit,
	          (od->op_mode & APPLE80211_M_STA) ? "STA" : "NONE"));
#endif /* !MACOSX_DHD */
	BCM_REFERENCE(interface);
	return 0;
}

SInt32
AirPort_Brcm43xx::getMCS_INDEX_SET( OSObject * interface, struct apple80211_mcs_index_set_data * misd )
{
	int err = 0;
	uint8 mcsset[MCSSET_LEN] = {0};
	int copy_len = 0;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_bss_info_t *current_bss = NULL;
	BCM_REFERENCE(interface);
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */
	ASSERT(misd);
	if (!misd)
		return EINVAL;
#ifndef MACOSX_DHD
	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	current_bss = wlc_get_current_bss(bsscfg);
	/* If not associated to 802.11n ap, return error */
	if (bsscfg->associated &&
	    (!N_ENAB(pub) || !(current_bss->flags & WLC_BSS_HT)))
		return ENXIO;
	if ((err = wlc_iovar_op(pWLC, "cur_mcsset", NULL, 0, &mcsset, sizeof(mcsset),
	                        FALSE, bsscfg->wlcif)))
#else
	/* If not associated to 802.11n ap, return error */
	if ((err = wlIovarGetInt("nmode", &nmode) != 0)) {
		return err;
	}

	if ((err = wlIovarOp("cur_mcsset", NULL, 0, &mcsset, sizeof(mcsset), FALSE, NULL)))
#endif /* !MACOSX_DHD */
	{
		WL_IOCTL(("wl%d: getMCS_INDEX_SET: error \"%s\" (%d) from iovar \"curr_mcsset\"\n",
					unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	copy_len = MIN(sizeof(mcsset), sizeof(misd->mcs_set_map));
	bzero(misd->mcs_set_map, sizeof(misd->mcs_set_map));
	memcpy( &misd->mcs_set_map[0], &mcsset[0], copy_len );

	WL_IOCTL(("wl%d: getMCS_INDEX_SET \n", unit));

	return 0;
}
#ifndef MACOSX_DHD
#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
SInt32
AirPort_Brcm43xx::getVHT_MCS_INDEX_SET( OSObject * interface, struct apple80211_vht_mcs_index_set_data * vmisd )
{
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_bss_info_t *current_bss = NULL;

	BCM_REFERENCE(interface);
	ASSERT(vmisd);
	if (!vmisd)
		return EINVAL;

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);
	current_bss = wlc_get_current_bss(bsscfg);

	if (bsscfg->associated &&
	    (!VHT_ENAB(pub) || !(current_bss->flags2 & WLC_BSS_VHT)))
		return ENXIO;

	if (bsscfg->associated)
		vmisd->vht_mcs_index_set = current_bss->rateset.vht_mcsmap;
	else
		vmisd->vht_mcs_index_set = pWLC->default_bss->rateset.vht_mcsmap;

	return 0;
}
#endif
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::getRSSI( OSObject * interface, struct apple80211_rssi_data * rd )
{
	uint i = 0;
	int err = 0;
	wl_rssi_ant_t rssi_ant = {};
	int rssi = 0;

	ASSERT(rd);
	if (!rd)
		return EINVAL;
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );

	if (BSSCFG_AP(wl_find_bsscfg(wlcif))) {
		return ENXIO;
	}

#if defined(IO80211P2P)
	IO80211P2PInterface * p2pIf = OSDynamicCast( IO80211P2PInterface, interface );

	if( p2pIf && p2pIf->getInterfaceRole() != APPLE80211_VIRT_IF_ROLE_P2P_CLIENT )
	    return ENXIO;
#endif

	bool associated = wlcif ? wlcif->u.bsscfg->associated : wl_find_bsscfg()->associated;
#else
	bool associated = isAssociated();
#endif /* !MACOSX_DHD */

	rssi = 0;
	bzero((void *)&rssi_ant, (sizeof(rssi_ant)));

	if (!associated || IFMTHD( interface, linkState ) != kIO80211NetworkLinkUp) {
		rssi = 0;
		rssi_ant.count = 0;
	}
#ifndef MACOSX_DHD
	else if (SCAN_IN_PROGRESS(pWLC->scan)) {
		rssi_ant = cached_rssi_ant;
	}
#endif /* !MACOSX_DHD */
	else {
		err = wlIovarOp( "phy_rssi_ant", NULL, 0, &rssi_ant, sizeof( rssi_ant ),
				FALSE, interface );
		if (err)
			return osl_error(err);
	}

	/* Cache RSSI for use when there is an ongoing scan */
	cached_rssi_ant = rssi_ant;

	int actual_cnt = 0;
	rd->version = APPLE80211_VERSION;
	rd->num_radios = rssi_ant.count;
	rd->rssi_unit = APPLE80211_UNIT_DBM;
	rd->aggregate_rssi = 0;

	for (i = 0; i < rssi_ant.count; i++) {
		rd->rssi[i] = rssi_ant.rssi_ant[i];
		if (rd->rssi[i] != 0)
			actual_cnt++;
		rd->aggregate_rssi +=  rssi_ant.rssi_ant[i];
	}

	if (actual_cnt > 0) {
		rd->aggregate_rssi = rd->aggregate_rssi / actual_cnt;
		WL_PORT(("wl%d: getRSSI: %d dBm\n", unit, rd->aggregate_rssi));
	} else
		WL_PORT(("wl%d: getRSSI: %d dBm\n", unit, rssi));

	return 0;
}

SInt32
AirPort_Brcm43xx::getNOISE( OSObject * interface, struct apple80211_noise_data * nd )
{
	int ret = ENXIO, val = 0;
	BCM_REFERENCE(interface);
	ASSERT(nd);
	if (!nd)
		return EINVAL;

	nd->version = APPLE80211_VERSION;
	nd->num_radios = 1;
	nd->noise_unit = APPLE80211_UNIT_DBM;
#ifndef MACOSX_DHD
	if(wl_find_bsscfg()->associated)
#else
	if (isAssociated())
#endif /* !MACOSX_DHD */
	{
		if( wlIoctl(WLC_GET_PHY_NOISE, &val, sizeof( val ), FALSE, NULL ) == 0 )
		{
			nd->noise[0] = val;
			ret = 0;
                }
                WL_IOCTL(("wl%d: getNOISE: %d dBm\n", unit, val));
	}

	return ret;
}

SInt32
AirPort_Brcm43xx::getINT_MIT( OSObject * interface, struct apple80211_intmit_data * imd )
{
//	#define	INTERFERE_NONE	0	/* off */
//	#define	NON_WLAN	1	/* foreign/non 802.11 interference, no auto detect */
//	#define	WLAN_MANUAL	2	/* ACI: no auto detection */
//	#define	WLAN_AUTO	3	/* ACI: auto - detect */
//	#define AUTO_ACTIVE	(1 << 7)/* Auto is currently active */

	int val = 0, err = BCME_OK;

	BCM_REFERENCE(interface);

	ASSERT(imd);
	if (!imd)
		return EINVAL;

	err = wlIoctlGetInt(WLC_GET_INTERFERENCE_MODE, &val);
	if (err)
		return osl_error(err);

	imd->version = APPLE80211_VERSION;
	imd->int_mit = (val == INTERFERE_NONE) ?
	        APPLE80211_INT_MIT_OFF : APPLE80211_INT_MIT_AUTO;


	return 0;
}

SInt32
AirPort_Brcm43xx::getPOWER( OSObject * interface, struct apple80211_power_data * pd )
{
	int up = 0;
	int bcmerr = BCME_OK;
#ifdef MACOSX_DHD
	int isup;
#endif /* MACOSX_DHD */
	BCM_REFERENCE(interface);
	ASSERT(pd);
	if (!pd)
		return EINVAL;

	pd->version = APPLE80211_VERSION;
	pd->num_radios = num_radios;

#ifdef MACOSX_DHD
	bcmerr = wlIoctlGetInt( WLC_GET_UP, &isup );

	if (bcmerr) {
		WL_IOCTL(("wl%d: %s error (%d) from ioctl WLC_GET_UP\n",
			unit, __FUNCTION__, bcmerr));
		return osl_error(bcmerr);
	}
#ifdef AAPLPWROFF
	up = (_powerState == PS_INDEX_ON) && (isup || wl_down_for_mpc(this, isup));
#else
	up = (isup || wl_down_for_mpc(this, isup));
#endif /* AAPLPWROFF */
#else
#ifdef AAPLPWROFF
	up = (_powerState == PS_INDEX_ON) && (pub->up || wlc_down_for_mpc(pWLC));
#else
	up = (pub->up || wlc_down_for_mpc(pWLC));
#endif /* AAPLPWROFF */
#endif /* MACOSX_DHD */
	for (uint i = 0; i < num_radios; i++)
		pd->power_state[i] = up ? APPLE80211_POWER_ON : APPLE80211_POWER_OFF;

#ifdef WL11N
	// check for a subset of chains active via iovars
	if (up && num_radios > 1) {
		int rxchain = 0, txchain = 0;
		int mask = 0;

		bcmerr = wlIovarGetInt("rxchain", &rxchain);
		if (bcmerr) {
			WL_ERROR(("wl%d: error %d from iovar \"rxchain\"\n",
			          unit, bcmerr));
		} else {
			bcmerr = wlIovarGetInt("txchain", &txchain);
			if (bcmerr)
				WL_ERROR(("wl%d: error %d from iovar \"txchain\"\n",
				          unit, bcmerr));
		}

		if (!bcmerr) {
			// mask with the hw_chains bitmask
			mask = (1 << num_radios) - 1;

			if (((rxchain | txchain) & mask) == 0) {
				WL_ERROR(("wl%d: with num_radios=%d, expected overlap of chain bits "
				          "tx 0x%x and rx 0x%x with mask of available chains 0x%x\n",
				          unit, num_radios, txchain, rxchain, mask));
			} else {
				for (uint i = 0; i < num_radios; i++) {
					if ((rxchain & txchain) & NBITVAL(i))
						pd->power_state[i] = APPLE80211_POWER_ON;
					else if (rxchain & NBITVAL(i))
						pd->power_state[i] = APPLE80211_POWER_RX;
					else if (txchain & NBITVAL(i))
						pd->power_state[i] = APPLE80211_POWER_TX;
					else
						pd->power_state[i] = APPLE80211_POWER_OFF;
				}
			}
		}
	}
#endif /* WL11N */

	WL_IOCTL(("wl%d: getPOWER:\n", unit));
	for (uint i = 0; i < pd->num_radios; i++) {
		WL_IOCTL(("\tpower_state[%d]: %s\n", i,
		          lookup_name(apple80211_power_state_names, pd->power_state[i])));
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::getASSOCIATION_STATUS(OSObject * interface, struct apple80211_assoc_status_data * asd)
{
	ASSERT(asd);
	if (!asd)
		return EINVAL;

	asd->version = APPLE80211_VERSION;
	asd->status = IFINFOBLKP(interface)->association_status;

	WL_IOCTL(("wl%d: getASSOCIATION_STATUS: %s\n", unit,
	          lookup_name(dot11_status_names, IFINFOBLKP(interface)->association_status)));

	return 0;
}

SInt32
AirPort_Brcm43xx::getSCAN_RESULT( OSObject * interface, struct apple80211_scan_result ** scan_result )
{
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
	char ssidbuf[SSID_FMT_BUF_LEN] = {0};
	char age_str[16] = "(age   --- )";
#endif
	BCM_REFERENCE(interface);
	if (!scan_done) {
		WL_IOCTL(("wl%d: getSCAN_RESULT: not done\n", unit));
		return EBUSY;
	}

	if (scan_done < 0) {
		WL_PORT(("wl%d: getSCAN_RESULT: ENOMEM\n", unit));
		return ENOMEM;
	}

	ASSERT(scan_result);
	if (!scan_result)
		return EINVAL;

	// return next scan result
	if (current_scan_result != NULL) {
		*scan_result = &current_scan_result->asr;

#ifdef BCMDBG
		if (WL_PORT_ON()) {
			struct apple80211_scan_result *sr = *scan_result;
#ifndef MACOSX_DHD
			wlc_format_ssid(ssidbuf, sr->asr_ssid, sr->asr_ssid_len);
#else
			wl_format_ssid(ssidbuf, sr->asr_ssid, sr->asr_ssid_len);
#endif /* !MACOSX_DHD */
			if (sr->asr_age != 0)
				snprintf(age_str, sizeof(age_str), "(age %2d.%03d)",
				         sr->asr_age/1000, sr->asr_age % 1000);

			WL_PORT(("wl%d: getSCAN_RESULT: %s: rssi[%4d] ch %3d %s \"%s\"\n", unit,
			         bcm_ether_ntoa((struct ether_addr*)sr->asr_bssid, eabuf),
			         sr->asr_rssi, sr->asr_channel.channel, age_str, ssidbuf));
		}
#endif /* BCMDBG */
		current_scan_result = SLIST_NEXT(current_scan_result, entries);
		return 0;
	} else {
		WL_PORT(("wl%d: getSCAN_RESULT: done\n", unit));
	}

	return EIO;	// no more scan results
}

SInt32
AirPort_Brcm43xx::getAUTH_TYPE( OSObject * interface, struct apple80211_authtype_data * ad )
{
	ASSERT(ad);
	if (!ad)
		return EINVAL;

	ad->version = APPLE80211_VERSION;
	ad->authtype_lower = IFINFOBLKP(interface)->authtype_lower;
	ad->authtype_upper = IFINFOBLKP(interface)->authtype_upper;

	WL_IOCTL(("wl%d: getAUTH_TYPE: auth_lower %s auth_upper %s\n", unit,
		lookup_name(apple80211_authtype_lower_names, ad->authtype_lower),
		lookup_name(apple80211_authtype_upper_names, ad->authtype_upper)));

	return 0;
}
#ifdef MACOSX_DHD
extern "C" {
static struct mcs_rate_info {
	uint8 constellation_bits;
	uint8 coding_q;
	uint8 coding_d;
} mcs_info[] = {
        { 1, 1, 2 }, /* MCS  0: MOD: BPSK,   CR 1/2 */
        { 2, 1, 2 }, /* MCS  1: MOD: QPSK,   CR 1/2 */
        { 2, 3, 4 }, /* MCS  2: MOD: QPSK,   CR 3/4 */
        { 4, 1, 2 }, /* MCS  3: MOD: 16QAM,  CR 1/2 */
        { 4, 3, 4 }, /* MCS  4: MOD: 16QAM,  CR 3/4 */
        { 6, 2, 3 }, /* MCS  5: MOD: 64QAM,  CR 2/3 */
        { 6, 3, 4 }, /* MCS  6: MOD: 64QAM,  CR 3/4 */
        { 6, 5, 6 }, /* MCS  7: MOD: 64QAM,  CR 5/6 */
        { 8, 3, 4 }, /* MCS  8: MOD: 256QAM, CR 3/4 */
        { 8, 5, 6 }  /* MCS  9: MOD: 256QAM, CR 5/6 */
};

static uint
mcs2rate(uint mcs, uint nss, uint bw, int sgi)
{
        const int ksps = 250; /* kilo symbols per sec, 4 us sym */
        const int Nsd_20MHz = 52;
        const int Nsd_40MHz = 108;
        const int Nsd_80MHz = 234;
        const int Nsd_160MHz = 468;
        uint rate;

        if (mcs == 32) {
                /* just return fixed values for mcs32 instead of trying to parametrize */
                rate = (sgi == 0) ? 6000 : 6700;
        } else if (mcs <= 9) {
                /* This calculation works for 11n HT and 11ac VHT if the HT mcs values
                 * are decomposed into a base MCS = MCS % 8, and Nss = 1 + MCS / 8.
                 * That is, HT MCS 23 is a base MCS = 7, Nss = 3
                 */

                /* find the number of complex numbers per symbol */
                if (bw == 20) {
                        rate = Nsd_20MHz;
                } else if (bw == 40) {
                        rate = Nsd_40MHz;
                } else if (bw == 80) {
                        rate = Nsd_80MHz;
                } else if (bw == 160) {
                        rate = Nsd_160MHz;
                } else {
                        rate = 1;
                }

                /* multiply by bits per number from the constellation in use */
                rate = rate * mcs_info[mcs].constellation_bits;

                /* adjust for the number of spatial streams */
                rate = rate * nss;

                /* adjust for the coding rate given as a quotient and divisor */
                rate = (rate * mcs_info[mcs].coding_q) / mcs_info[mcs].coding_d;

                /* multiply by Kilo symbols per sec to get Kbps */
                rate = rate * ksps;

                /* adjust the symbols per sec for SGI
                 * symbol duration is 4 us without SGI, and 3.6 us with SGI,
                 * so ratio is 10 / 9
                 */
                if (sgi) {
                        /* add 4 for rounding of division by 9 */
                        rate = ((rate * 10) + 4) / 9;
                }
        } else {
                rate = 0;
        }

        return rate;
}

static int
ratespec2rate(uint32 rspec)
{
        int rate = -1;
        int sgi = ((rspec & WL_RSPEC_SGI) != 0);

        if ((rspec & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_RATE) {
                rate = (rspec & WL_RSPEC_RATE_MASK);
        } else if ((rspec & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_HT) {
                uint mcs = (rspec & WL_RSPEC_RATE_MASK);

                if (mcs > 32) {
                        WL_ERROR(("%s: MCS %u out of range (>32) in ratespec 0x%X\n",
                                __FUNCTION__, mcs, rspec));
                } else if (mcs == 32) {
                        rate = mcs2rate(mcs, 1, 40, sgi) / 500;
                } else {
                        uint nss = 1 + (mcs / 8);
                        mcs = mcs % 8;

                        rate = mcs2rate(mcs, nss, 20, sgi) / 500;
                }
        } else if ((rspec & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_VHT) {
                uint mcs = (rspec & WL_RSPEC_VHT_MCS_MASK);
                uint nss = (rspec & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT;

                rate = mcs2rate(mcs, nss, 20, sgi) / 500;
        } else {
                WL_ERROR(("%s: missing rate encoding in ratespec 0x%X\n",
                        __FUNCTION__, (uint)rspec));
        }

        return rate;
}
} /* extern C */
#endif /* MACOSX_DHD */
// apple80211 interface definition of rate in Mbps
// rounds down for all legacy rates (5.5 -> 5, all others are integer Mbps),
// and rounds up for all HT rates
#ifndef MACOSX_DHD
#define RSPEC2MBPS(_rspec)			\
	(RSPEC_ISLEGACY(_rspec) ?		\
	RSPEC2KBPS(_rspec)/1000 :		\
	 (RSPEC2KBPS(_rspec) + 999)/1000)
#else
#define RSPEC2MBPS(_rspec) (ratespec2rate(_rspec) / 2)
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::getRATE( OSObject * interface, struct apple80211_rate_data * rd )
{
#ifndef MACOSX_DHD
	wlc_bsscfg_t *bsscfg = NULL;
	ASSERT(rd);
	if (!rd)
		return EINVAL;

	struct wlc_if * wlcif = wlLookupWLCIf( interface );

	bsscfg = wl_find_bsscfg( wlcif );
	ASSERT(bsscfg != NULL);
#else
	int err;
#endif /* !MACOSX_DHD */

#ifndef MACOSX_DHD
	if (!bsscfg->associated || IFMTHD( interface, linkState ) != kIO80211NetworkLinkUp)
#else
	if (IFMTHD(interface, linkState) != kIO80211NetworkLinkUp)
#endif /* !MACOSX_DHD */
	{
		WL_IOCTL(("wl%d: getRATE: not associated\n", unit));
		return ENXIO;
	}

	rd->version = APPLE80211_VERSION;
	rd->num_radios = 1;
#ifndef MACOSX_DHD
	ratespec_t rspec = wlc_get_rspec_history(bsscfg);
	rd->rate[0] = RSPEC2MBPS(rspec);
#else
	int rate;
	err = wlIoctl(WLC_GET_RATE, &rate, sizeof( rate ), FALSE, NULL);
	if (err) {
		return err;
	}

	rd->rate[0] = rate/2;
#endif /* !MACOSX_DHD */
	WL_IOCTL(("wl%d: getRATE: %d\n", unit, rd->rate[0]));

	return 0;
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::getMCS( OSObject * interface, struct apple80211_mcs_data * md )
{
	wlc_bsscfg_t *bsscfg = NULL;

	ASSERT(md);
	if (!md)
		return EINVAL;

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	if (!bsscfg->associated || IFMTHD( interface, linkState ) != kIO80211NetworkLinkUp) {
		WL_IOCTL(("wl%d: getMCS: not associated\n", unit));
		return ENXIO;
	}

	md->version = APPLE80211_VERSION;

	ratespec_t rspec = wlc_get_rspec_history(bsscfg);

	if (RSPEC_ISHT(rspec)) {
		md->index = (uint32)(rspec & RSPEC_RATE_MASK);
		WL_IOCTL(("wl%d: getMCS: ht index %u\n", unit, md->index));
	} else if ( RSPEC_ISVHT(rspec)) {
		md->index = (uint32)(rspec & RSPEC_VHT_MCS_MASK);
		WL_IOCTL(("wl%d: getMCS: vht index %u\n", unit, md->index));
	} else {
		WL_IOCTL(("wl%d: getMCS: rate not MCS index\n", unit));
		return ENXIO;
	}

	return 0;
}

#ifdef APPLE80211_IOC_MCS_VHT
SInt32
AirPort_Brcm43xx::getMCS_VHT( OSObject * interface, struct apple80211_mcs_vht_data * mvd )
{
	wlc_bsscfg_t *bsscfg = NULL;
	chanspec_t chspec = 0;

	BCM_REFERENCE(interface);
	ASSERT(mvd);
	if (!mvd)
		return EINVAL;

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	if (!bsscfg->associated) {
		WL_ERROR(("wl%d: getMCS_VHT: not associated\n", unit));
		return ENXIO;
	}

	if ((!VHT_ENAB(pub))) {
		WL_ERROR(("wl%d: getMCS_VHT: Not in VHT mode \n", unit));
		return ENXIO;
	}

	mvd->version = APPLE80211_VERSION;

	ratespec_t rspec = wlc_get_rspec_history(bsscfg);

	if ( RSPEC_ISVHT(rspec)) {
		mvd->index = (uint32)(rspec & RSPEC_VHT_MCS_MASK);
		WL_ERROR(("wl%d: getMCS_VHT: vht index %u\n", unit, mvd->index));
	} else {
		WL_ERROR(("wl%d: getMCS_VHT: rate not MCS index\n", unit));
		return ENXIO;
	}
    /* returns the number of spatial stream */
	mvd->nss = wlc_ratespec_nss(rspec);

    /* return the channel bw */
	chspec = bsscfg->current_bss->chanspec;
	mvd->bandwidth = CHSPEC_WLC_BW(chspec);

	/* return the guard interval */
	mvd->gi = APPLE80211_GI_LONG;
	if (WLC_HT_GET_SGI_TX(bsscfg->wlc->hti) == ON)
		mvd->gi = APPLE80211_GI_SHORT;

	return 0;
}
#endif /* APPLE80211_IOC_MCS_VHT */
#endif /* !MACOSX_DHD */

SInt32
AirPort_Brcm43xx::getLDPC( OSObject * interface, struct apple80211_ldpc_data * ld )
{
	int bcmerr = 0;
	int ldpc_cap = 0;
	BCM_REFERENCE(interface);

	ASSERT(ld);
	if (!ld)
		return EINVAL;

	if (!PHYTYPE_11N_CAP(phytype))
		return EOPNOTSUPP;

	ld->version = APPLE80211_VERSION;
	ld->enabled = 0;

	bcmerr = wlIovarGetInt("ldpc_cap", &ldpc_cap);
	if (!bcmerr) {
		ld->enabled = (ldpc_cap != OFF);
	}

	return 0;
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::getMULTICAST_RATE( OSObject * interface, struct apple80211_rate_data * rd )
{
	int rate = 0;
	const char* iov = NULL;
	wlc_bsscfg_t *bsscfg = NULL;

	BCM_REFERENCE(interface);
	ASSERT(rd);
	if (!rd)
		return EINVAL;

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	rd->version = APPLE80211_VERSION;
	rd->num_radios = 1;

	if( bsscfg->associated )
	{
		if (CHSPEC_IS2G(wlc_get_home_chanspec(bsscfg)))
			iov = "2g_mrate";
		else
			iov = "5g_mrate";

		if( wlIovarGetInt(iov, &rate) == 0 )
			rd->rate[0] = rate/2;	// rate reported in 500 kbps units
	}

	return 0;
}
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::getSTATUS_DEV( OSObject * interface, struct apple80211_status_dev_data * dd )
{
	ASSERT(dd);
	if (!dd)
		return EINVAL;

	BCM_REFERENCE(interface);
	dd->version = APPLE80211_VERSION;
	bzero(dd->dev_name, sizeof(dd->dev_name));
	strncpy( (char *)dd->dev_name, "appleAE", sizeof(dd->dev_name) );

	return 0;
}

SInt32
AirPort_Brcm43xx::getSUPPORTED_CHANNELS( OSObject * interface, struct apple80211_sup_channel_data  * sd )
{
	wl_uint32_list_t  *list = NULL;
	UInt32 i = 0;
	struct apple80211_channel * chan = NULL;
	int err = BCME_OK;
	UInt32 chanspec = 0, len = 0;

	BCM_REFERENCE(interface);

	ASSERT(sd);
	if (!sd)
		return EINVAL;

	len = sizeof(uint32) * (WL_NUMCHANSPECS + 1);

	list = (wl_uint32_list_t*)MALLOC(osh, len);

	if (list == NULL)
		return ENOMEM;

	list->count = WL_NUMCHANSPECS;

	/* Get all the supported chanspecs */
	err = wlIovarOp( "chanspecs", &chanspec, sizeof( chanspec ), list, len, FALSE,
	                    NULL );

	if (err) {
		WL_IOCTL(("wl%d: getSUPPORTED_CHANNELS: error \"%s\" (%d) from chanspecs\n",
			unit, bcmerrorstr(err), err));
		err = ENXIO;
		goto exit;
	}

	sd->version = APPLE80211_VERSION;
	sd->num_channels = MIN(list->count, APPLE80211_MAX_CHANNELS);

	for( i = 0; i < sd->num_channels; i++ )
	{
		chan = &sd->supported_channels[i];
		chanspec = list->element[i];

		chanspec2applechannel(chanspec, chan);

		/* Active scanning supported ? */
#ifndef MACOSX_DHD
		if (!wlc_quiet_chanspec(pWLC->cmi, chanspec))
#endif /* !MACOSX_DHD */
			chan->flags |= APPLE80211_C_FLAG_ACTIVE;
	}

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		WL_IOCTL(("wl%d: getSUPPORTED_CHANNELS: reporting %u channels\n", unit, sd->num_channels));
		dump_apple80211_channels(sd->num_channels, sd->supported_channels);
	}
#endif
exit:
	MFREE(osh, list, len);

	return err;
}

#ifdef APPLE80211_IOC_HW_SUPPORTED_CHANNELS
SInt32
AirPort_Brcm43xx::getHW_SUPPORTED_CHANNELS( OSObject * interface, struct apple80211_sup_channel_data  * sd )
{
	wl_uint32_list_t  *list = NULL;
	UInt32 i = 0;
	struct apple80211_channel * chan = NULL;
	int err = BCME_OK;
	UInt32 chanspec = 0, len = 0;

	BCM_REFERENCE(interface);

	ASSERT(sd);
	if (!sd)
		return EINVAL;

	len = sizeof(uint32) * (WL_NUMCHANSPECS + 1);

	list = (wl_uint32_list_t*)MALLOC(osh, len);

	if (list == NULL)
		return ENOMEM;

	list->count = WL_NUMCHANSPECS;

	/* Get all the supported chanspecs */
	err = wlIovarOp( "chanspecs", &chanspec, sizeof( chanspec ), list, len, FALSE,
					 NULL );

	if (err) {
		WL_IOCTL(("wl%d: getHW_SUPPORTED_CHANNELS: error \"%s\" (%d) from chanspecs\n",
				  unit, bcmerrorstr(err), err));
		err = ENXIO;
		goto exit;
	}
#ifndef MACOSX_DHD
	if (appendProhibited_channels(list) < 0) {
		WL_ERROR(("wl%d: %s: Channel List too long\n", unit, __FUNCTION__));
		err = ENXIO;
		goto exit;
	}
#endif /* !MACOSX_DHD */
	sd->version = APPLE80211_VERSION;
	sd->num_channels = MIN(list->count, APPLE80211_MAX_CHANNELS);

	for( i = 0; i < sd->num_channels; i++ )
	{
		chan = &sd->supported_channels[i];
		chanspec = list->element[i];

		chanspec2applechannel(chanspec, chan);
		/* Active scanning supported ? */
#ifndef MACOSX_DHD
		if (!wlc_quiet_chanspec(pWLC->cmi, chanspec))
#endif /* !MACOSX_DHD */
		{
			chan->flags |= APPLE80211_C_FLAG_ACTIVE;
		}
	}

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		WL_IOCTL(("wl%d: getHW_SUPPORTED_CHANNELS: reporting %u channels\n", unit, sd->num_channels));
		dump_apple80211_channels(sd->num_channels, sd->supported_channels);
	}
#endif
exit:
	MFREE(osh, list, len);

	return err;
}
#endif /* APPLE80211_IOC_HW_SUPPORTED_CHANNELS */

SInt32
AirPort_Brcm43xx::getLOCALE( OSObject * interface, struct apple80211_locale_data * ld )
{
	uint32 locale = 0;
	char country[WLC_CNTRY_BUF_SZ] = {};
	int err = BCME_OK;
	const struct {
		const char *cc;
		uint32 apple_locale;
	} cc_locales[] = {
		{"XB", APPLE80211_LOCALE_FCC},
		{"X0", APPLE80211_LOCALE_FCC},
		{"US", APPLE80211_LOCALE_FCC},
		{"XA", APPLE80211_LOCALE_ETSI},
		{"X3", APPLE80211_LOCALE_ETSI},
		{"FR", APPLE80211_LOCALE_ETSI},
		{"DE", APPLE80211_LOCALE_ETSI},
		{"JP", APPLE80211_LOCALE_JAPAN},
		{"KR", APPLE80211_LOCALE_KOREA},
		{"X1", APPLE80211_LOCALE_APAC},
		{"X2", APPLE80211_LOCALE_ROW},
#if VERSION_MAJOR > 12
		{"ID", APPLE80211_LOCALE_INDONESIA},
#endif
		{NULL, 0}
	};

	BCM_REFERENCE(interface);
	ASSERT(ld);
	if (!ld)
		return EINVAL;

	err = wlIoctl(WLC_GET_COUNTRY, country, WLC_CNTRY_BUF_SZ, FALSE, NULL);

	if (err)
		return ENXIO;

	locale = APPLE80211_LOCALE_UNKNOWN;
	for (int i = 0; cc_locales[i].cc != NULL; i++) {
		if (!strcmp(country, cc_locales[i].cc)) {
			locale = cc_locales[i].apple_locale;
			break;
		}
	}

	ld->version = APPLE80211_VERSION;
	ld->locale = locale;

	WL_IOCTL(("wl%d: getLOCALE: %s\n", unit,
	          lookup_name(apple80211_locale_names, ld->locale)));

	return 0;
}

SInt32
AirPort_Brcm43xx::getAP_MODE( OSObject * interface, struct apple80211_apmode_data * ad )
{
	int infra = 0;

	BCM_REFERENCE(interface);
	ASSERT(ad);
	if (!ad)
		return EINVAL;

	if( wlIoctlGetInt(WLC_GET_INFRA, &infra) != 0 )
		return ENXIO;

	ad->version = APPLE80211_VERSION;
	ad->apmode = infra ? APPLE80211_AP_MODE_INFRA : APPLE80211_AP_MODE_IBSS;

	return 0;
}

SInt32
AirPort_Brcm43xx::getPROTMODE( OSObject * interface, struct apple80211_protmode_data * pd )
{
	int val = 0;

	BCM_REFERENCE(interface);
	ASSERT(pd);
	if (!pd)
		return EINVAL;

	if( wlIoctlGetInt(WLC_GET_GMODE_PROTECTION_OVERRIDE, &val) != 0 )
		return ENXIO;

	pd->version = APPLE80211_VERSION;
	switch( val )
	{
		case WLC_PROTECTION_OFF:
			pd->protmode = APPLE80211_PROTMODE_OFF;
			break;
		case WLC_PROTECTION_ON:
			pd->protmode = APPLE80211_PROTMODE_CTS;
			break;
		case WLC_PROTECTION_AUTO:
			pd->protmode = APPLE80211_PROTMODE_AUTO;
			break;
	}

	pd->threshold = 0;

	return 0;
}

SInt32
AirPort_Brcm43xx::getFRAG_THRESHOLD( OSObject * interface, struct apple80211_frag_threshold_data * td )
{
	int err = BCME_OK;
	int fragthresh = 0;

	BCM_REFERENCE(interface);

	ASSERT(td);
	if (!td)
		return EINVAL;

	err = wlIovarGetInt("fragthresh", &fragthresh);
	if (err)
		return ENXIO;

	td->version = APPLE80211_VERSION;
	td->threshold = fragthresh;

	return 0;
}

SInt32
AirPort_Brcm43xx::getRATE_SET( OSObject * interface, struct apple80211_rate_set_data * rd )
{
	int i = 0;
	int err = BCME_OK;
	wl_rateset_t rateset = {};
#ifdef BCMDBG
	const uint ratestr_len = 5*APPLE80211_MAX_RATES + 1;
	char ratestr[ratestr_len];
	char *buf = ratestr;
	bool b = FALSE;
#endif

	BCM_REFERENCE(interface);
	ASSERT(rd);
	if (!rd)
		return EINVAL;

	rd->version = APPLE80211_VERSION;
	rd->num_rates = 0;
#ifndef MACOSX_DHD
	if( !wl_find_bsscfg()->associated ) {
		WL_IOCTL(("wl%d: getRATE_SET: not associated\n", unit));
		return 0;
	}
#endif /* !MACOSX_DHD */
	err = wlIoctl(WLC_GET_CURR_RATESET, &rateset, sizeof( rateset ), FALSE, NULL );
	if (err) {
		WL_IOCTL(("wl%d: getRATE_SET: error \"%s\" (%d) from WLC_GET_CURR_RATESET\n",
			unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	rd->num_rates = (UInt16)MIN(rateset.count, APPLE80211_MAX_RATES);

	for( i = 0; i < rd->num_rates; i++ ) {
		rd->rates[i].version = APPLE80211_VERSION;
		rd->rates[i].rate = (rateset.rates[i] & DOT11_RATE_MASK)/2;
		rd->rates[i].flags = ((rateset.rates[i] & DOT11_RATE_BASIC) ?
							  APPLE80211_RATE_FLAG_BASIC : APPLE80211_RATE_FLAG_NONE);
	}

#ifdef BCMDBG
	for (i = 0; i < rd->num_rates; i++) {
		b = (rd->rates[i].flags & APPLE80211_RATE_FLAG_BASIC) != 0;
		buf += snprintf(buf, ratestr_len - (buf - ratestr), "%d%s ",
			rd->rates[i].rate, b ? "b" : "");
	}
	WL_IOCTL(("wl%d: getRATE_SET: [ %s]\n", unit, ratestr));
#endif

	return 0;
}

SInt32
AirPort_Brcm43xx::getSHORT_SLOT( OSObject * interface, struct apple80211_short_slot_data * sd )
{
	int err = BCME_OK, val = 0;

	BCM_REFERENCE(interface);

	ASSERT(sd);
	if (!sd)
		return EINVAL;

	err = wlIoctlGetInt(WLC_GET_SHORTSLOT, &val);
	if( !err )
	{
		sd->version = APPLE80211_VERSION;

		switch( val )
		{
			case WLC_SHORTSLOT_AUTO:
				sd->mode = APPLE80211_SHORT_SLOT_MODE_AUTO;
				break;
			case WLC_SHORTSLOT_OFF:
				sd->mode = APPLE80211_SHORT_SLOT_MODE_LONG;
				break;
			case WLC_SHORTSLOT_ON:
				sd->mode = APPLE80211_SHORT_SLOT_MODE_SHORT;
				break;
			default:
				return ENXIO;	// ug
		}
	}

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::getSHORT_RETRY_LIMIT( OSObject * interface, struct apple80211_retry_limit_data * rld )
{
	int err = BCME_OK;
	uint val = 0;

	BCM_REFERENCE(interface);

	ASSERT(rld);
	if (!rld)
		return EINVAL;

	err = wlIoctlGetInt(WLC_GET_SRL, (int*)&val);
	if (err)
		return ENXIO;

	rld->version = APPLE80211_VERSION;
	rld->limit = val;

	return 0;
}

SInt32
AirPort_Brcm43xx::getLONG_RETRY_LIMIT( OSObject * interface, struct apple80211_retry_limit_data * rld )
{
	int err = 0;
	uint val = 0;

	BCM_REFERENCE(interface);

	ASSERT(rld);
	if (!rld)
		return EINVAL;

	err = wlIoctlGetInt(WLC_GET_LRL, (int*)&val);
	if (err)
		return ENXIO;

	rld->version = APPLE80211_VERSION;
	rld->limit = val;

	return 0;
}

SInt32
AirPort_Brcm43xx::getTX_ANTENNA( OSObject * interface, struct apple80211_antenna_data * ad )
{
	int err = 0;
	int val = 0;

	BCM_REFERENCE(interface);

	ASSERT(ad);
	if (!ad)
		return EINVAL;

	ad->num_radios = num_radios;

	if (num_radios > 1) {
		for (uint i = 0; i < num_radios; i++) {
			ad->antenna_index[i] = i;
		}
	} else {
		err = wlIoctlGetInt(WLC_GET_TXANT, &val);
		if (err)
			return ENXIO;

		// Apple docs say -1 for auto, WLC_GET_TXANT returns 3
		if (val == 3)
			val = -1;

		ad->antenna_index[0] = val;
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::getRX_ANTENNA( OSObject * interface, struct apple80211_antenna_data * ad )
{
	int err = 0;
	int val = 0;

	BCM_REFERENCE(interface);
	ASSERT(ad);
	if (!ad)
		return EINVAL;

	ad->num_radios = num_radios;

	if (num_radios > 1) {
		for (uint i = 0; i < num_radios; i++) {
			ad->antenna_index[i] = i;
		}
	} else {
		err = wlIoctlGetInt(WLC_GET_RX_ANT, &val);
		if (err)
			return ENXIO;

		ad->antenna_index[0] = val;
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::getANTENNA_DIVERSITY( OSObject * interface, struct apple80211_antenna_data * ad )
{
	int err = BCME_OK;
	int val = 0;

	BCM_REFERENCE(interface);
	ASSERT(ad);
	if (!ad)
		return EINVAL;

	ad->num_radios = num_radios;

	if (num_radios > 1) {
		for (uint i = 0; i < num_radios; i++) {
			ad->antenna_index[i] = i;
		}
	} else {
		err = wlIoctlGetInt(WLC_GET_ANTDIV, &val);
		if (err)
			return ENXIO;

		// Apple docs say -1 for auto, WLC_GET_ANTDIV returns 3
		if (val == 3)
			val = -1;

		ad->antenna_index[0] = val;
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::getDTIM_INT( OSObject * interface, apple80211_dtim_int_data * dd )
{
	int err = BCME_OK;
	int val = 0;

	BCM_REFERENCE(interface);

	ASSERT(dd);
	if (!dd)
		return EINVAL;

	err = wlIoctlGetInt(WLC_GET_DTIMPRD, &val);
	if (err)
		return ENXIO;

	dd->version = APPLE80211_VERSION;
	dd->interval = val;

	return 0;
}

SInt32
AirPort_Brcm43xx::getSTATION_LIST( OSObject * interface, struct apple80211_sta_data * sd )
{
	struct maclist* sta_list = NULL;
	int err = BCME_OK;
	int len = 0;

	BCM_REFERENCE(interface);

	ASSERT(sd);
	if (!sd)
		return EINVAL;

	len = sizeof(struct maclist) + (APPLE80211_MAX_STATIONS - 1) * sizeof(struct ether_addr);

	sta_list = (struct maclist*)MALLOC(osh, len);
	if (sta_list == NULL)
		return ENOMEM;

	sta_list->count = APPLE80211_MAX_STATIONS;

	err = wlIoctl(WLC_GET_ASSOCLIST, sta_list, len, FALSE, NULL);

	if( err == 0 )
	{
		int i = 0;
		sd->version = APPLE80211_VERSION;
		sd->num_stations = MIN(sta_list->count, APPLE80211_MAX_STATIONS);
		for( i = 0; i < (int)sd->num_stations; i++ ) {
			scb_val_t scb_val = {};

			/* Obtain per-sta RSSI */
			memcpy( &scb_val.ea, &sta_list->ea[i], sizeof( struct ether_addr ) );
			err = wlIoctl(WLC_GET_RSSI, &scb_val, sizeof(scb_val), FALSE, NULL);
			if ( err == 0)
				sd->station_list[i].sta_rssi = (int32_t)scb_val.val;
			else
				sd->station_list[i].sta_rssi = 0;
			memcpy( &sd->station_list[i].sta_mac, &sta_list->ea[i], sizeof( struct ether_addr ) );
		}
	}

	MFREE(osh, sta_list, len);

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::getDRIVER_VERSION( OSObject * interface, struct apple80211_version_data * vd )
{
	BCM_REFERENCE(interface);
	ASSERT(vd);
	if (!vd)
		return EINVAL;

	vd->version = APPLE80211_VERSION;
#ifdef BCMDBG
	memset(vd->string, 'X', APPLE80211_MAX_VERSION_LEN);
#endif
	vd->string_len = MIN(APPLE80211_MAX_VERSION_LEN, strlen(kVersionString));
	memcpy(vd->string, kVersionString, vd->string_len);

	return 0;
}

SInt32
AirPort_Brcm43xx::getHARDWARE_VERSION( OSObject * interface, struct apple80211_version_data * vd )
{
	char * temp = NULL;
	int len = 0;

	BCM_REFERENCE(interface);
	ASSERT(vd);
	if (!vd)
		return EINVAL;

	vd->version = APPLE80211_VERSION;
	temp = vd->string;
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "vendorid: 0x%x\n", revinfo.vendorid );
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "deviceid: 0x%x\n", revinfo.deviceid);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "radiorev: 0x%x\n", revinfo.radiorev);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "chipnum: 0x%x\n", revinfo.chipnum);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "chiprev: 0x%x\n", revinfo.chiprev);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "corerev: 0x%x\n", revinfo.corerev);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "boardid: 0x%x\n", revinfo.boardid);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "boardvendor: 0x%x\n", revinfo.boardvendor);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "boardrev: 0x%x\n", revinfo.boardrev);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "driverrev: %d.%d.%d.%d\n",
		revinfo.drvrev_major,
		revinfo.drvrev_minor,
		revinfo.drvrev_rc,
		revinfo.drvrev_rc_inc);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "ucoderev: 0x%x\n", revinfo.ucoderev);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;

	temp = vd->string + strlen( vd->string );
	len+=snprintf( temp, APPLE80211_MAX_VERSION_LEN - len, "bus: 0x%x\n", revinfo.bus);
	if( len >= APPLE80211_MAX_VERSION_LEN ) goto done;


	done:
	vd->string_len = (u_int16_t)len;
	return 0;
}

SInt32
AirPort_Brcm43xx::getPOWERSAVE(OSObject * interface, struct apple80211_powersave_data *pd)
{
	int err = BCME_OK, val = 0;
	int monitor = 0;

	BCM_REFERENCE(interface);

	ASSERT(pd);
	if (!pd)
		return EINVAL;

	err = wlIoctlGetInt(WLC_GET_PM, &val);

	if( err == 0 )
	{
		pd->version = APPLE80211_VERSION;

		// When in monitor mode, the driver is not in powersave
		// so reflect so
		if( wlIoctlGetInt(WLC_GET_MONITOR, &monitor) == 0 && monitor) {
			pd->powersave_level = APPLE80211_POWERSAVE_MODE_DISABLED;
			return 0;
		}

		switch( val )
		{
			case PM_OFF:
				pd->powersave_level = APPLE80211_POWERSAVE_MODE_DISABLED;
				break;
			case PM_MAX:
				pd->powersave_level = APPLE80211_POWERSAVE_MODE_80211;
				break;
			case PM_FAST:
				pd->powersave_level = APPLE80211_POWERSAVE_MODE_VENDOR;
				break;
			default:
				return ENXIO;	// what the
		}
	} else if (err == BCME_NOTSTA) {
		// Apple request to return MODE disabled instead of error for Not STA
		pd->powersave_level = APPLE80211_POWERSAVE_MODE_DISABLED;
		return 0;
	}

	return !err ? 0 : ENXIO;
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::getROM( OSObject * interface, struct apple80211_rom_data * rd )
{
	srom_rw_t *srom = NULL;
	const uint srom_read_bytes = 128;
	int err = BCME_OK;
	int len = 0;

	BCM_REFERENCE(interface);

	ASSERT(rd);
	if (!rd)
		return EINVAL;

	len = sizeof(srom_rw_t) - sizeof(uint16) + srom_read_bytes;

	srom = (srom_rw_t*)MALLOC(osh, len);
	if (srom == NULL)
		return ENOMEM;

	srom->byteoff = 0;
	srom->nbytes = srom_read_bytes;

	err = wlIoctl(WLC_GET_SROM, srom, len, FALSE, NULL);

	if (err == 0) {
		rd->version = APPLE80211_VERSION;
		rd->rom_len = MIN(rd->rom_len, srom_read_bytes);
		memcpy(rd->rom, srom->buf, rd->rom_len);
	}

	MFREE(osh, srom, len);

	return !err ? 0 : ENXIO;
}
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::getRAND( OSObject * interface, struct apple80211_rand_data * rd )
{
	uint rand = 0;
	int err = BCME_OK;

	BCM_REFERENCE(interface);

	ASSERT(rd);
	if (!rd)
		return EINVAL;

	err = wlIovarGetInt( "rand", (int *)&rand);
	if (err)
		return ENXIO;

	rd->rand = rand << 16;

	err = wlIovarGetInt( "rand", (int *)&rand);
	if (err)
		return ENXIO;

	rd->rand |= rand;

	rd->version = APPLE80211_VERSION;

	return 0;
}
#ifndef MACOSX_DHD
// Return the WPA/WPA2 IE if the interface is currently participating in a network using WPA or WPA2
SInt32
AirPort_Brcm43xx::getRSN_IE( OSObject * interface, struct apple80211_rsn_ie_data * rid )
{
	bcm_tlv_t *rsn_ie = NULL;
	bcm_tlv_t *req_ies = NULL;
	uint req_ies_len = 0;
	wlc_bsscfg_t *bsscfg = NULL;

	BCM_REFERENCE(interface);

	ASSERT(rid);
	if (!rid)
		return EINVAL;

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	// Only return the IE info if we are currently part of a network
	if (!bsscfg->associated) {
		WL_IOCTL(("wl%d: getRSN_IE: not associated, returning ENXIO %d\n",
		          unit, ENXIO));
		return ENXIO;
	}

	// If there is no association information saved, or it is too short to contain IEs
	// then no IEs to return
	if (!bsscfg->assoc->req || bsscfg->assoc->req_len <= sizeof(struct dot11_assoc_req)) {
		WL_IOCTL(("wl%d: getRSN_IE: associated but no saved association information, "
		          "returning ENXIO %d\n", unit, ENXIO));
		return ENXIO;
	}

	req_ies = (bcm_tlv_t*)(bsscfg->assoc->req+1);
	req_ies_len = bsscfg->assoc->req_len - sizeof(struct dot11_assoc_req);

	// Skip the previous BSSID field if this is a saved reassociation request
	// instead of an association request
	if (bsscfg->assoc->req_is_reassoc) {
		if (req_ies_len <= ETHER_ADDR_LEN)
			return ENXIO;
		req_ies_len -= ETHER_ADDR_LEN;
		req_ies = (bcm_tlv_t*)((int8*)req_ies + ETHER_ADDR_LEN);
	}

	// Look for the WPA or WPA2 IE informaiton, exiting if not found
	if ((rsn_ie = (bcm_tlv_t*)bcm_find_wpaie((uint8*)req_ies, req_ies_len)) == NULL &&
	    (rsn_ie = bcm_parse_tlvs(req_ies, req_ies_len, DOT11_MNG_RSN_ID)) == NULL) {
		WL_IOCTL(("wl%d: getRSN_IE: no WPA/RSN IEs found in saved association information, "
		          "returning ENXIO %d\n", unit, ENXIO));
		return ENXIO;
	}

	rid->version = APPLE80211_VERSION;
	rid->len = TLV_HDR_LEN + rsn_ie->len;
	ASSERT(rid->len <= APPLE80211_MAX_RSN_IE_LEN);
	if (rid->len > APPLE80211_MAX_RSN_IE_LEN)
		return EINVAL;
	bcopy(rsn_ie, rid->ie, rid->len);

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		WL_IOCTL(("wl%d: getRSN_IE: returning RSN IE len %d\n",
		          unit, (int)rid->len));
		prhex(NULL, rid->ie, rid->len);
	}
#endif

	return 0;
}
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::getAP_IE_LIST( OSObject * interface, struct apple80211_ap_ie_data * ied )
{
	int8 *ap_ies = NULL;
	uint ap_ies_len = 0;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_bss_info_t *current_bss = NULL;
#ifdef MACOSX_DHD
	char iovbuf[WLC_IOCTL_SMLEN];
	int bcmerr;
	wl_assoc_info_t	ai;
#endif /* MACOSX_DHD */

	BCM_REFERENCE(interface);

	ASSERT(ied);
	if (!ied)
		return EINVAL;
#ifdef MACOSX_DHD
	memset(iovbuf,0, sizeof(iovbuf));
	bcmerr = wlIovarOp( "probe_resp_ies", NULL, 0, iovbuf, sizeof(iovbuf), false, NULL );
	if (bcmerr) {
		WL_ERROR(("%s: probe_resp_ies returned error %d\n", __func__, bcmerr));
		return ENXIO;
	}
	ap_ies_len = *(int*)iovbuf;
#else
	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	// only return the IE info if we are currently part of a network
	if (!bsscfg->associated) {
		WL_IOCTL(("wl%d: getAP_IE_LIST: not associated, returning ENXIO %d\n",
		          unit, ENXIO));
		return ENXIO;
	}
	current_bss = wlc_get_current_bss(bsscfg);

	// make sure there are some IEs to return
	if (!(current_bss->bcn_prb && current_bss->bcn_prb_len > DOT11_BCN_PRB_LEN)) {
		WL_IOCTL(("wl%d: getAP_IE_LIST: associated but no saved association information, "
		          "returning ENXIO %d\n", unit, ENXIO));
		return ENXIO;
	}

	// IEs come after the fixed length portion of beacon or probe
	ap_ies = (int8*)current_bss->bcn_prb + DOT11_BCN_PRB_LEN;
	ap_ies_len = current_bss->bcn_prb_len - DOT11_BCN_PRB_LEN;
#endif /* MACOSX_DHD */
	WL_IOCTL(("wl%d: getAP_IE_LIST: incomming apple80211_ap_ie_data (%p):\n",
		unit, OSL_OBFUSCATE_BUF(ied)));
	WL_IOCTL(("wl%d: getAP_IE_LIST: version: %d\n", unit, ied->version));
	WL_IOCTL(("wl%d: getAP_IE_LIST: len: %d\n", unit, ied->len));
	WL_IOCTL(("wl%d: getAP_IE_LIST: ie_data: %p\n", unit, OSL_OBFUSCATE_BUF(ied->ie_data)));

	// make sure there is room in the ie buffer
	if (ied->len < ap_ies_len) {
		WL_IOCTL(("wl%d: getAP_IE_LIST: ie_data buffer to short (%d bytes) for %d bytes "
		          "of bcn/prb IE data\n", unit, ied->len, ap_ies_len));
		return ENXIO;
	}

	ied->version = APPLE80211_VERSION;
	ied->len = ap_ies_len;
#ifdef MACOSX_DHD
	bcopy((iovbuf+4), ied->ie_data, ap_ies_len);
#else
	bcopy(ap_ies, ied->ie_data, ap_ies_len);
#endif /* MACOSX_DHD */
	WL_IOCTL(("wl%d: getAP_IE_LIST: returning %d bytes of bcn/prb IE data\n",
	          unit, ied->len));

	return 0;
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::getSTATS( OSObject * interface, struct apple80211_stats_data *sd )
{
	BCM_REFERENCE(interface);
	ASSERT(sd);
	if (!sd)
		return EINVAL;

	sd->version		= APPLE80211_VERSION;
	sd->tx_frame_count	= WLCNTVAL(pub->_cnt->txframe);
	sd->tx_errors		= WLCNTVAL(pub->_cnt->txerror);
	sd->rx_frame_count	= WLCNTVAL(pub->_cnt->rxframe);
	sd->rx_errors		= WLCNTVAL(pub->_cnt->rxerror);

	return 0;
}
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::getDEAUTH( OSObject * interface, struct apple80211_deauth_data *dd )
{
	dd->version = APPLE80211_VERSION;
	dd->deauth_reason = IFINFOBLKP(interface)->deauth_reason;
	bcopy(IFINFOBLKP(interface)->deauth_ea.octet, dd->deauth_ea.octet, ETHER_ADDR_LEN);

	return 0;
}

SInt32
AirPort_Brcm43xx::getCOUNTRY_CODE( OSObject * interface, struct apple80211_country_code_data * ccd )
{
	char country[WLC_CNTRY_BUF_SZ] = {0};
	int err = BCME_OK;

	BCM_REFERENCE(interface);

	ASSERT(ccd);
	if (!ccd)
		return EINVAL;

	err = wlIoctl(WLC_GET_COUNTRY, country, WLC_CNTRY_BUF_SZ, FALSE, NULL);
	if (err)
		return osl_error(err);

#if APPLE80211_MAX_CC_LEN < 2
#error "Expecting space for 2 chars in cc field"
#endif
	// copy the 2 character country code with null termination
	ccd->version = APPLE80211_VERSION;
	bzero(ccd->cc, APPLE80211_MAX_CC_LEN);
	bcopy(country, ccd->cc, 2);

	return 0;
}

bool
AirPort_Brcm43xx::isCntryDefaultSupported(uint boardid)
{
#ifdef CNTRY_DEFAULT
	uint board_ids[] = {
		BCM943602X100GS, BCM943602X100P2,
		BCM94350X14P2
	};
	uint idx;

	for (idx = 0; idx < ARRAYSIZE(board_ids); idx++) {
		if (board_ids[idx] == boardid) {
			return TRUE;
		}
	}
#else
	BCM_REFERENCE(boardid);
#endif /* CNTRY_DEFAULT */
	return FALSE;
}

#ifdef CNTRY_DEFAULT
bool
AirPort_Brcm43xx::isCntryRestricted(char *country)
{
	const char *restrictedCntry[] = {"ID", 0};
	int i;

	if (country == NULL) {
		ASSERT(0 && "Country string is NULL");
	}

	for (i = 0; restrictedCntry[i] != 0; i++) {
		if (!bcmp(country, restrictedCntry[i], WLC_CNTRY_BUF_SZ-1)) {
			WL_INFORM(("%s: restricted Country %s\n", __FUNCTION__, country));
			return TRUE;
		}
	}
	WL_INFORM(("%s: non-restricted Country %s\n", __FUNCTION__, country));
	return FALSE;
}

SInt32
AirPort_Brcm43xx::resetAutoCountry(char *country)
{
	int err = BCME_OK;
	bool restrictedCntry = isCntryRestricted(country);

	// disabed ccode_pr_g country if restricted Country
	if ((err = wlIovarSetInt("ccode_pr_2g", !restrictedCntry))) {
		WL_ERROR(("%s: CC 2G priority set Failed %d\n", __FUNCTION__, err));
		return osl_error(err);
	}
	if (!restrictedCntry) {
		if ((err = setAutoCountry(TRUE))) {
			WL_ERROR(("%s: Auto Country set Failed %d\n", __FUNCTION__, err));
			return osl_error(err);
		}
	}
	return osl_error(err);
}

SInt32
AirPort_Brcm43xx::setRestrictedCntryCode(char *country, bool restricted)
{
	int err = BCME_OK;

	WL_TRACE(("%s: set restricted country %s\n", __FUNCTION__, country));
	if (country == NULL) {
		WL_ERROR(("%s: Country string is NULL\n", __FUNCTION__));
		return osl_error(EINVAL);
	}

	if (restricted) {
		if ((err = wlIovarSetInt("ccode_pr_2g", FALSE))) {
			WL_ERROR(("%s: CC 2G priority set Failed %d\n", __FUNCTION__, err));
			return osl_error(err);
		}

		if ((err = setAutoCountry(FALSE))) {
			WL_ERROR(("%s: set auto country Failed %d\n", __FUNCTION__, err));
			return osl_error(err);
		}

		if ((err = wlIovarSetInt("passive_on_restricted", TRUE))) {
			WL_ERROR(("%s: set passive on restricted cntry Failed %d\n",
				__FUNCTION__, err));
			return osl_error(err);
		}

		if ((err = wlIovarSetInt("country_default", FALSE))) {
			WL_ERROR(("%s: set country default Failed %d\n", __FUNCTION__, err));
			return osl_error(err);
		}
#if NOT_YET
		if (wlIoctl(WLC_SET_COUNTRY, country, WLC_CNTRY_BUF_SZ, TRUE, NULL)) {
			WL_ERROR(("%s: WLC_SET_COUNTRY %s Failed %d\n",
				__FUNCTION__, country, err));
			return osl_error(err);
		}
#endif
	} else {
		// disable passive_on_restricted
		if ((err = wlIovarSetInt("passive_on_restricted", FALSE))) {
			WL_ERROR(("%s: set passive on restricted cntry Failed %d\n",
				__FUNCTION__, err));
			return osl_error(err);
		}

		// enable country_default
		if ((err = wlIovarSetInt("country_default", TRUE))) {
			WL_ERROR(("%s: set country default Failed %d\n", __FUNCTION__, err));
			return osl_error(err);
		}
#if NOT_YET
		if (wlIoctl(WLC_SET_COUNTRY, country, WLC_CNTRY_BUF_SZ, TRUE, NULL)) {
			WL_ERROR(("%s: WLC_SET_COUNTRY %s Failed %d\n", __FUNCTION__,
				country, err));
			return osl_error(err);
		}
#endif

		if (!host_cc) {
			// disable country_default
			if ((err = wlIovarSetInt("country_default", FALSE))) {
				WL_ERROR(("%s: set country default Failed %d\n",
					__FUNCTION__, err));
				return osl_error(err);
			}

			// reset AutoCountry
			err = resetAutoCountry(country);
		}
        }
	return osl_error(err);
}
#endif /* CNTRY_DEFAULT */

SInt32
AirPort_Brcm43xx::setCOUNTRY_CODE( OSObject * interface, struct apple80211_country_code_data * ccd )
{
	int err = EINVAL;
#ifdef CNTRY_DEFAULT
	char country[WLC_CNTRY_BUF_SZ] = { 0 };

	BCM_REFERENCE(interface);

	ASSERT(ccd);
	if (!ccd)  {
		return osl_error(err);
	}

	/* setCOUNTRY_CODE should only be allowed for specific
	 * modules that supports Geo-Location as per Olympic
	 * requirement doc.
	 */
	if (!isCntryDefaultSupported(revinfo.boardid)) {
		WL_ERROR(("%s: Geo Location is Not Supported\n", __FUNCTION__));
		return osl_error(err);
	}

	/* cc == NULL or cc == X*, then no override; use 11d */
	if ((ccd->cc[0] == 0) || (ccd->cc[0] == 'X') || (ccd->cc[0] == 'x')) {
		// get default country code
		err = wlIovarOp("autocountry_default", NULL, 0, country, WLC_CNTRY_BUF_SZ,
			IOV_GET, interface);
		if (err) {
			WL_ERROR(("%s: Get autocountry default Failed %d\n", __FUNCTION__, err));
			return osl_error(err);
		}
		host_cc = FALSE;
	} else {
		strncpy(country, (const char *)ccd->cc, WLC_CNTRY_BUF_SZ);
		country[WLC_CNTRY_BUF_SZ - 1] = 0;
		host_cc = TRUE;
	}

	WL_ASSOC(("%s: set country=[%s] host_cc=%d\n", __FUNCTION__, country, host_cc));
	if (isCntryRestricted(country)) {
		if (pub->associated) {
			if ((err = setDISASSOCIATE(interface))) {
				WL_ERROR(("%s: setDISASSOCATED Failed %d\n", __FUNCTION__, err));
				return osl_error(err);
			}
			err = wlIovarOp("scancache_clear", NULL, 0, NULL, 0, IOV_SET, interface);
			if (err) {
				WL_ERROR(("%s: scam cache flush Failed %d\n", __FUNCTION__, err));
				return osl_error(err);
			}
		}
		err = setRestrictedCntryCode(country, TRUE);
		if (wlIoctl(WLC_SET_COUNTRY, country, WLC_CNTRY_BUF_SZ, TRUE, NULL)) {
			WL_ERROR(("%s: WLC_SET_COUNTRY %s Failed %d\n", __FUNCTION__, country, err));
			return osl_error(err);
		}
		WL_ERROR(("%s: Set Restricted country %s\n", __FUNCTION__, country));
	} else {
		if (!pub->associated) {
			if ((err = wlIovarSetInt("ccode_pr_2g", TRUE))) {
				WL_ERROR(("%s: set CC 2G priority Failed %d\n", __FUNCTION__, err));
				return osl_error(err);
			}
			if ((err = setAutoCountry(TRUE))) {
				WL_ERROR(("%s: set auto country Failed %d\n", __FUNCTION__, err));
				return osl_error(err);
			}
		}
		err = setRestrictedCntryCode(country, FALSE);
		if (wlIoctl(WLC_SET_COUNTRY, country, WLC_CNTRY_BUF_SZ, TRUE, NULL)) {
			WL_ERROR(("%s: WLC_SET_COUNTRY %s Failed %d\n", __FUNCTION__, country, err));
			return osl_error(err);
		}
		WL_ERROR(("%s: Set Non-Restricted country %s\n", __FUNCTION__, country));
	}
#else
	BCM_REFERENCE(interface);
	BCM_REFERENCE(ccd);
#endif /* CNTRY_DEFAULT */
	return osl_error(err);
}

SInt32
AirPort_Brcm43xx::getLAST_RX_PKT_DATA( OSObject * interface, struct apple80211_last_rx_pkt_data * pd )
{
	ASSERT(pd);

	if (!pd)
		return EINVAL;

	memcpy(pd, &IFINFOBLKP(interface)->rxpd, sizeof(struct apple80211_last_rx_pkt_data));

	return 0;
}

SInt32
AirPort_Brcm43xx::getRADIO_INFO( OSObject * interface, struct apple80211_radio_info_data * rid )
{
	BCM_REFERENCE(interface);
	ASSERT(rid);
	if (!rid)
		return EINVAL;

	rid->version = APPLE80211_VERSION;
	rid->count = num_radios;

	return 0;
}

SInt32
AirPort_Brcm43xx::getGUARD_INTERVAL(OSObject * interface, struct apple80211_guard_interval_data * gid)
{
	BCM_REFERENCE(interface);
	ASSERT(gid);
	if (!gid)
		return EINVAL;

	if (!PHYTYPE_11N_CAP(phytype)) {
		WL_IOCTL(("wl%d: getGUARD_INTERVAL: error, not NPHY\n", unit));
		return EOPNOTSUPP;
	}

	gid->version = APPLE80211_VERSION;
	gid->interval = APPLE80211_GI_LONG;

	WL_IOCTL(("wl%d: getGUARD_INTERVAL: returning %d ns\n", unit, (uint)gid->interval));

	return 0;
}

SInt32
AirPort_Brcm43xx::getMSDU(OSObject * interface, struct apple80211_msdu_data * md)
{
	int bcmerr = 0;
	int amsdu_aggbytes = 0;
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */

	BCM_REFERENCE(interface);
	ASSERT(md);
	if (!md)
		return EINVAL;
#ifndef MACOSX_DHD
	if (!N_ENAB(pub))
		return EOPNOTSUPP;
#else
	if ((bcmerr = wlIovarGetInt("nmode", &nmode) != 0)) {
		IOLog( "wlIovarGetInt(nmode) returned %d\n", bcmerr);
		return bcmerr;
	}

	if (!nmode) {
		return EOPNOTSUPP;
	}
#endif /* !MACOSX_DHD */

	bcmerr = wlIovarGetInt("amsdu_aggbytes", &amsdu_aggbytes);
	if (bcmerr)
		return ENXIO;

	md->version = APPLE80211_VERSION;
	md->max_length = (uint32)amsdu_aggbytes;

	return 0;
}

SInt32
AirPort_Brcm43xx::getMPDU(OSObject * interface, struct apple80211_mpdu_data * md)
{
	int bcmerr = 0;
	int density = 0;
	int factor = 0;
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */

	BCM_REFERENCE(interface);
	ASSERT(md);
	if (!md)
		return EINVAL;

#ifndef MACOSX_DHD
	if (!N_ENAB(pub)) {
		return EOPNOTSUPP;
	}
#else
	if (bcmerr = wlIovarGetInt("nmode", &nmode)) {
		return bcmerr;
	}

	if (!nmode) {
		return EOPNOTSUPP;
	}
#endif /* !MACOSX_DHD */

	bcmerr = wlIovarGetInt("ampdu_density", &density);
	if (bcmerr)
		return ENXIO;

	bcmerr = wlIovarGetInt("ampdu_rx_factor", &factor);
	if (bcmerr)
		return ENXIO;

	md->version = APPLE80211_VERSION;
	md->max_factor = factor;
	md->max_density = density;

	return 0;
}

SInt32
AirPort_Brcm43xx::getBLOCK_ACK( OSObject * interface, struct apple80211_block_ack_data * bad )
{
	int enabled = 0;
	int bcmerr = 0;
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */
	BCM_REFERENCE(interface);
	ASSERT(bad);
	if (!bad)
		return EINVAL;

#ifndef MACOSX_DHD
	if (!N_ENAB(pub))
		return EOPNOTSUPP;
#else
	if (bcmerr = wlIovarGetInt("nmode", &nmode)) {
		return bcmerr;
	}

	if (!nmode) {
		return EOPNOTSUPP;
	}
#endif /* !MACOSX_DHD */

	bcmerr = wlIovarGetInt("ampdu", &enabled);
	if (bcmerr)
		return ENXIO;

	if (enabled)
		enabled = 1;

	bad->version = APPLE80211_VERSION;
	bad->ba_enabled = (uint8)enabled;
	bad->immediate_ba_enabled = (uint8)enabled;
	bad->cbba_enabled = (uint8)enabled;
	bad->implicit_ba_enabled = (uint8)enabled;

	return 0;
}

SInt32
AirPort_Brcm43xx::getPHY_SUB_MODE( OSObject * interface, struct apple80211_physubmode_data * pd )
{
	BCM_REFERENCE(interface);
	ASSERT(pd);
	if (!pd)
		return EINVAL;

	if (pd->phy_mode != APPLE80211_MODE_11N) {
		WL_IOCTL(("wl%d: getPHY_SUB_MODE: request for phy_sub_mode for phy type %d. "
		          "Only support for APPLE80211_MODE_11N (%d)\n",
		          unit, pd->phy_mode, APPLE80211_MODE_11N));
		return EOPNOTSUPP;
	} else if (!PHYTYPE_11N_CAP(phytype)) {
		WL_IOCTL(("wl%d: getPHY_SUB_MODE: request for phy_sub_mode for 11N phy, "
		          "but device is not 11N.", unit));
		return EOPNOTSUPP;
	}

	pd->version = APPLE80211_VERSION;
	pd->phy_mode = APPLE80211_MODE_11N;
	pd->phy_submode = (phy_submode == 0) ? APPLE80211_SUBMODE_11N_AUTO : phy_submode;
	if (phy_submode_flags == 0) {
		pd->flags = APPLE80211_C_FLAG_20MHZ | APPLE80211_C_FLAG_40MHZ;
		pd->flags |= APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_5GHZ;
	} else {
		pd->flags = phy_submode_flags;
	}

	return 0;
}

#if VERSION_MAJOR > 9

SInt32
AirPort_Brcm43xx::getROAM_THRESH( OSObject * interface, struct apple80211_roam_threshold_data * rtd )
{
	BCM_REFERENCE(interface);
	ASSERT(rtd);
	if (!rtd)
		return EINVAL;

	rtd->version = APPLE80211_VERSION;

	struct {
		int val;
		int band;
	} x;
	uint32 chanspec =0;

	int err = wlIovarGetInt( "chanspec", (int *)&chanspec);
	if (err) {
		WL_IOCTL(("wl%d: getROAM_THRESH: error \"%s\" (%d) from iovar \"chanspec\"\n",
		          unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	x.band = CHSPEC2WLC_BAND(chanspec);
	int bcmerror = wlIoctl(WLC_GET_ROAM_TRIGGER, &x, sizeof( x ), FALSE, NULL );
	if (bcmerror)
		return ENXIO;

	rtd->rssi_thresh = x.val;
	rtd->rate_thresh = 0; // BRCM does not have rate based roaming

	return 0;

}


// Apple change: getSTA_IE_LIST, getKEY_RSC, and getSTA_STATS are for hostapd support
SInt32
AirPort_Brcm43xx::getSTA_IE_LIST( OSObject * interface, struct apple80211_sta_ie_data * sid )
{
	sta_ie_list_t *elt = NULL;
	uint8 *ie_data = NULL;
	uint8 *start = NULL;
	bcm_tlv_t *ie0 = NULL, *ie1 = NULL;
	bcm_tlv_t *tlv = NULL;
	UInt32 tlvlen = 0, sidlen = 0;
	UInt32 len = 0;

	BCM_REFERENCE(interface);
	ASSERT(sid);
	if (!sid) {
		return EINVAL;
	}

	if (sid->len == 0) {
		WL_ERROR(("%s: buffer length passed in is 0, INVALID!\n", __FUNCTION__));
		return EINVAL;
	}

	SLIST_FOREACH(elt, &assocStaIEList, entries) {
		if (BCMPADRS(&sid->mac, &elt->addr) == 0) {
			break;
		}
	}

	if (elt == NULL) {
		// no entry found in sta ie list, return 0 bytes
		sid->len = 0;
		return 0;
	}

	// check if we need to truncate ie list
	if (sid->len >= elt->tlvlen) {
		// we're good, just copy data over and return
		bcopy((void *)elt->tlv, (void *)sid->ie_data, elt->tlvlen);
		sid->len = elt->tlvlen;
		return 0;
	}

	WL_ERROR(("%s: buffer too small, need to truncate ie list from %d to %d bytes\n",
	          __FUNCTION__, (int)elt->tlvlen, (int)sid->len));

	// need to truncate list with preference given to wpa/wpa2 ie
	tlv = elt->tlv;
	tlvlen = elt->tlvlen;
	// fetch wpa/wpa2 ie and see if we need to rearrange the ie's
	ie0 = (bcm_tlv_t *)bcm_find_wpaie((uint8 *)tlv, tlvlen);
	ie1 = bcm_parse_tlvs(tlv, tlvlen, DOT11_MNG_RSN_ID);
	// use sidlen to keep track of how much room we have left
	sidlen = sid->len;

	if ((ie0 == NULL) && (ie1 == NULL)) {
		// no wpa/wpa2 ie, just copy what will fit
		bcopy((void *)elt->tlv, (void *)sid->ie_data, sidlen);
		return 0;
	}

	ie_data = (uint8 *)sid->ie_data;
	// more parsing with priority given to wpa/wpa2 ie
	// This means we will copy wpa/wpa2 ie to sid->ie_data first if we can.
	// If there is still room, then we copy over the rest of the IE's until
	// we fill up sid->ie_data buffer.
	if (ie0 > ie1) {
		bcm_tlv_t *tmp = ie0;
		// swap to make sure ie0 is the earlier IE
		ie0 = ie1;
		ie1 = tmp;
	}

	// copy ie0 and ie1 to ie_data
	if (ie0) {
		len = ie0->len + TLV_HDR_LEN;
		if (len > sidlen) {
			len = sidlen;
		}
		WL_PORT(("%s: copy ie0 len = %d, sidlen = %d\n",
		         __FUNCTION__, (int)len, (int)sidlen));
		bcopy((void *)ie0, (void *)ie_data, len);
		sidlen -= len;
		ie_data += len;
	}
	if (ie1) {
		len = ie1->len + TLV_HDR_LEN;
		if (len > sidlen) {
			len = sidlen;
		}
		WL_PORT(("%s: copy ie1 len = %d, sidlen = %d\n",
		         __FUNCTION__, (int)len, (int)sidlen));
		bcopy((void *)ie1, (void *)ie_data, len);
		sidlen -= len;
		ie_data += len;
	}
	// copy the rest of the ie's that will fit
	if (ie0) {
		// copy from start of ie list to ie0
		len = (UInt32)((uint8 *)ie0 - (uint8 *)tlv);
	} else {
		// copy from start of ie list to ie1
		len = (UInt32)((uint8 *)ie1 - (uint8 *)tlv);
	}
	if (len > sidlen) {
		len = sidlen;
	}
	WL_PORT(("%s: copy from start to %s len = %d, sidlen = %d\n",
	         __FUNCTION__, (ie0) ? "ie0" : "ie1", (int)len, (int)sidlen));
	bcopy((void *)tlv, (void *)ie_data, len);
	sidlen -= len;
	ie_data += len;

	if (ie0 && ie1) {
		// copy from ie0 to ie1, excluding ie0
		len = (UInt32)((uint8 *)ie1 - (uint8 *)ie0) - (ie0->len + TLV_HDR_LEN);
		if (len > sidlen) {
			len = sidlen;
		}
		WL_PORT(("%s: copy from ie0 to ie1 len = %d, sidlen = %d\n",
		         __FUNCTION__, (int)len, (int)sidlen));
		start = (uint8 *)ie0 + (ie0->len + TLV_HDR_LEN);
		bcopy((void *)start, (void *)ie_data, len);
		sidlen -= len;
		ie_data += len;
	}

	if (ie1) {
		// copy from ie1 to end, excluding ie1
		start = (uint8 *)ie1 + (ie1->len + TLV_HDR_LEN);
		len = tlvlen - (UInt32)(start - (uint8 *)tlv);
	}
	if (len > sidlen) {
		len = sidlen;
	}
	WL_PORT(("%s: copy from %s to end len = %d, sidlen = %d\n",
	         __FUNCTION__, (ie1) ? "ie1" : "ie0", (int)len, (int)sidlen));
	if (start != NULL) {
		bcopy((void *)start, (void *)ie_data, len);
	}
	return 0;
}

SInt32
AirPort_Brcm43xx::getKEY_RSC( OSObject * interface, struct apple80211_key * key )
{
	union {
		int32 index;
		uint8 tsc[EAPOL_WPA_KEY_RSC_LEN];
	} u;

	BCM_REFERENCE(interface);
	bzero( &u, sizeof( u ) );

	u.index = (int32)key->key_index;
	IOLog( "AirPort_Brcm43xx::getKEY_RSC: Requesting RSC for index %d.\n", u.index );

	int bcmerror = wlIoctl(WLC_GET_KEY_SEQ, &u, sizeof( u ), FALSE, NULL );
	IOLog( "wlIoctl(WLC_GET_KEY_SEQ) returned %d\n", bcmerror );
	if( !bcmerror )
	{
		memcpy( key->key_rsc, u.tsc, EAPOL_WPA_KEY_RSC_LEN );
		key->key_rsc_len = EAPOL_WPA_KEY_RSC_LEN;

		return 0;
	}

	return osl_error( bcmerror );
}

SInt32
AirPort_Brcm43xx::getSTA_STATS( OSObject * interface, struct apple80211_sta_stats_data * ssd )
{
#ifndef MACOSX_DHD
        struct ether_addr macaddr = {};
        SInt32 error = 0;
        sta_info_t sta = {};

        ASSERT(ssd);
        if (!ssd) {
                return EINVAL;
        }

        if(ETHER_ISNULLADDR(&ssd->mac)) {
                WL_IOCTL(("wl%d: getSTA_STATS: Null Mac Address\n", unit));
                return EINVAL;
        }

        bzero(&macaddr, ETHER_ADDR_LEN);
        bzero(&sta, sizeof(sta));

        /*Copy the required MAC address*/
        BCOPYADRS(&ssd->mac, &macaddr);

        /*Get driver sta info */
        error = wlIovarOp("sta_info", &macaddr, sizeof(macaddr), &sta, sizeof(sta),
                                                                IOV_GET, interface );

        if (error) {
                WL_IOCTL(("wl%d: getSTA_STATS: error \"%s\" (%d) from iovar \"sta_info\"\n",
                                        unit, bcmerrorstr(error), error));
                return osl_error(error);
        }

        ssd->version = APPLE80211_VERSION;
        ssd->rx_packets = sta.rx_tot_pkts;
        ssd->rx_bytes = sta.rx_tot_bytes;
        ssd->tx_packets = sta.tx_tot_pkts;

        return 0;
#else
	return EOPNOTSUPP;
#endif /* !MACOSX_DHD */
}

#endif /* VERSION_MAJOR > 9 */

#ifdef SUPPORT_FEATURE_WOW
SInt32
AirPort_Brcm43xx::getWOW_PARAMETERS( OSObject * interface, struct apple80211_wow_parameter_data * wpd )
{
	if( !wowl_cap )
		return EOPNOTSUPP;

	int param = 0, brcmerror = 0;
	BCM_REFERENCE(interface);

	brcmerror = wlIovarGetInt( "wowl_os", &param );
	if( brcmerror )
		return osl_error( brcmerror );

	if( param & WL_WOWL_MAGIC )
		setbit( wpd->wake_cond_map, APPLE80211_WAKE_COND_MAGIC_PATTERN );
	if( param & WL_WOWL_NET )
		setbit( wpd->wake_cond_map, APPLE80211_WAKE_COND_NET_PATTERN );
	if( param & WL_WOWL_DIS )
	{
		// No way to get one without the other
		setbit( wpd->wake_cond_map, APPLE80211_WAKE_COND_DISASSOCIATED );
		setbit( wpd->wake_cond_map, APPLE80211_WAKE_COND_DEAUTHED );
	}
	if( param & WL_WOWL_RETR )
		setbit( wpd->wake_cond_map, APPLE80211_WAKE_COND_RETROGRADE_TSF );
	if( param & WL_WOWL_BCN )
	{
		setbit( wpd->wake_cond_map, APPLE80211_WAKE_COND_BEACON_LOSS );

		brcmerror = wlIovarGetInt( "wowl_bcn_loss", &param );
		if( brcmerror )
			return osl_error( brcmerror );

		wpd->beacon_loss_time = (u_int32_t)param;
	}

	/* Pattern list exists independent of the actual setting of net pattern */
	{
		wl_wowl_pattern_list_t * plist = NULL;
		wl_wowl_pattern_t * brcmPat = NULL;
		UInt32 i = 0;

		plist = (wl_wowl_pattern_list_t *)MALLOCZ(osh, WLC_IOCTL_MAXLEN);
		if( !plist )
			return ENOMEM;

		brcmerror = wlIovarOp( "wowl_pattern",
		                          NULL,
		                          0,
		                          plist,
		                          WLC_IOCTL_MAXLEN,
		                          FALSE,
		                          NULL );
		if( brcmerror ) {
			MFREE(osh, plist, WLC_IOCTL_MAXLEN);
			return osl_error( brcmerror );
		}

		brcmPat = plist->pattern;
		for( i = 0; i < plist->count && i < APPLE80211_MAX_WOW_PATTERNS; i++ )
		{
			// This should never happen...
			if( brcmPat->patternsize > APPLE80211_MAX_WOW_PAT_LEN )
				brcmPat->patternsize = APPLE80211_MAX_WOW_PAT_LEN;

			wpd->patterns[i].len = brcmPat->patternsize;
			memcpy( wpd->patterns[i].pattern, (UInt8 *)brcmPat + brcmPat->patternoffset, brcmPat->patternsize );

			brcmPat = (wl_wowl_pattern_t *)( (UInt8 *)brcmPat + brcmPat->patternoffset + brcmPat->patternsize );
		}

		wpd->pattern_count = i;

		MFREE(osh, plist, WLC_IOCTL_MAXLEN);
	}

	return 0;
}
#endif /* SUPPORT_FEATURE_WOW */

#ifdef APPLE80211_IOC_CDD_MODE
SInt32
AirPort_Brcm43xx::getCDD_MODE( IO80211Interface * interface, struct apple80211_cdd_mode_data * cmd )
{
	struct wl_spatial_mode_data spatial_mode = {};
	int err = 0;

	ASSERT(cmd);
	if (!cmd)
		return EINVAL;

	err = wlIovarOp("spatial_policy", NULL, 0, &spatial_mode, sizeof(spatial_mode),
		IOV_GET, interface );

	if (err) {
		WL_IOCTL(("wl%d: getCDD_MODE: error \"%s\" (%d) from iovar \"spatial_mode\"\n",
			  unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	cmd->version = APPLE80211_VERSION;
	cmd->cdd_mode_24G = (spatial_mode.mode[SPATIAL_MODE_2G_IDX] == AUTO) ?
		APPLE80211_CDD_AUTO : spatial_mode.mode[SPATIAL_MODE_2G_IDX];
	cmd->cdd_mode_5G_lower = (spatial_mode.mode[SPATIAL_MODE_5G_LOW_IDX] == AUTO) ?
		APPLE80211_CDD_AUTO : spatial_mode.mode[SPATIAL_MODE_5G_LOW_IDX];
	cmd->cdd_mode_5G_middle = (spatial_mode.mode[SPATIAL_MODE_5G_MID_IDX] == AUTO) ?
		APPLE80211_CDD_AUTO : spatial_mode.mode[SPATIAL_MODE_5G_MID_IDX];
	cmd->cdd_mode_5G_H = (spatial_mode.mode[SPATIAL_MODE_5G_HIGH_IDX] == AUTO) ?
		APPLE80211_CDD_AUTO : spatial_mode.mode[SPATIAL_MODE_5G_HIGH_IDX];
	cmd->cdd_mode_5G_upper = (spatial_mode.mode[SPATIAL_MODE_5G_UPPER_IDX] == AUTO) ?
		APPLE80211_CDD_AUTO : spatial_mode.mode[SPATIAL_MODE_5G_UPPER_IDX];

	WL_IOCTL(("wl%d: getCDD_MODE:  2.4G %d 5GL %d 5GM %d 5GH %d 5GU %d\n", unit,
		  cmd->cdd_mode_24G, cmd->cdd_mode_5G_lower, cmd->cdd_mode_5G_middle,
		  cmd->cdd_mode_5G_H, cmd->cdd_mode_5G_upper));

	return 0;
}
#endif /* APPLE80211_IOC_CDD_MODE */

#ifdef APPLE80211_IOC_THERMAL_THROTTLING
SInt32
AirPort_Brcm43xx::getTHERMAL_THROTTLING( IO80211Interface * interface, struct apple80211_thermal_throttling * tt )
{
	int throttling, streams = 0;
	int err = BCME_OK;

	ASSERT(tt);
	if (!tt)
		return EINVAL;

	if ((revinfo.chipnum != BCM4331_CHIP_ID) &&
	    (revinfo.chipnum != BCM4360_CHIP_ID) &&
	    (revinfo.chipnum != BCM4350_CHIP_ID) &&
	    (revinfo.chipnum != BCM43602_CHIP_ID))
		return EOPNOTSUPP;

	err = wlIovarOp("pwrthrottle_mask", NULL, 0, &throttling, sizeof(throttling),
		IOV_GET, interface);

	if (err) {
		WL_IOCTL(("wl%d: getTHERMAL_THROTTLING: error \"%s\" (%d) from iovar \"pwrthrottl_core\"\n",
			  unit, bcmerrorstr(err), err));
		return osl_error(err);
	}
	err = wlIovarOp("txstreams", NULL, 0, &streams, sizeof(streams),
					  IOV_GET, interface);
	if (err) {
		WL_IOCTL(("wl%d: getTHERMAL_THROTTLING: error \"%s\" (%d) from iovar \"txstreams\"\n",
			  unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	memset(tt, 0, sizeof(struct apple80211_thermal_throttling));
	tt->version = APPLE80211_VERSION;
	tt->num_radios = streams;
	tt->thermal_state[0] = (throttling & 1) ? APPLE80211_THERMAL_THROTTLE_ON : APPLE80211_THERMAL_THROTTLE_OFF;
	tt->thermal_state[1] = (throttling & 2) ? APPLE80211_THERMAL_THROTTLE_ON : APPLE80211_THERMAL_THROTTLE_OFF;
	tt->thermal_state[2] = (throttling & 4) ? APPLE80211_THERMAL_THROTTLE_ON : APPLE80211_THERMAL_THROTTLE_OFF;

	WL_IOCTL(("wl%d: getTHERMAL_THROTTLING: # of radio %d state: %d %d %d\n", unit,
		  tt->num_radios, tt->thermal_state[0], tt->thermal_state[1], tt->thermal_state[2]));

	return 0;
}
#endif /* APPLE80211_IOC_THERMAL_THROTTLING */

#ifdef IO80211P2P

AirPort_Brcm43xxP2PInterface *
AirPort_Brcm43xx::getVirtIf( OSObject *interface) const
{
	AirPort_Brcm43xxP2PInterface *virtIf = NULL;
	AirPort_Brcm43xxP2PInterface *foundVirtIf = NULL;

	if (!OSDynamicCast( IO80211P2PInterface , interface ))
		return NULL;
	virtIf = (AirPort_Brcm43xxP2PInterface *)interface;

	struct virt_intf_entry *elt = NULL;
	SLIST_FOREACH(elt, &virt_intf_list, entries) {
		if (elt->obj == virtIf) {
			foundVirtIf = virtIf;
			break;
		}
	}

	return foundVirtIf;
}
#endif /* IO80211P2P */

struct wlc_if *
AirPort_Brcm43xx::wlLookupWLCIf( OSObject *interface) const
{
	BCM_REFERENCE(interface);
#ifdef MACOS_VIRTIF
	if (apif.virtIf == interface)
		return apif.wlcif;
#endif
#ifdef IO80211P2P
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);
	if (virtIf)
		return virtIf->wlcif;

	// There is some synchronization issue between deleting/detaching interface
	// so some packets/ioctls may bleed through
	// ASSERT(interface == ((IO80211Interface*)myNetworkIF));
#endif
	return NULL;
}
#ifndef MACOSX_DHD
#if VERSION_MAJOR > 9
SInt32
AirPort_Brcm43xx::getLINK_QUAL_EVENT_PARAMS( OSObject * interface, struct apple80211_link_qual_event_data * lqd )
{
	BCM_REFERENCE(interface);
	ASSERT(lqd);
	if (!lqd)
		return EINVAL;

	lqd->version = APPLE80211_VERSION;
	lqd->rssi_divisor = link_qual_config.rssi_divisor;
	lqd->tx_rate_divisor = link_qual_config.tx_rate_divisor;
	lqd->min_interval = link_qual_config.min_interval;
#if VERSION_MAJOR > 13
	lqd->hysteresis = link_qual_config.hysteresis;
#endif
	return 0;
}

SInt32
AirPort_Brcm43xx::getVENDOR_DBG_FLAGS( OSObject * interface, struct apple80211_vendor_dbg_data * vdd )
{
	BCM_REFERENCE(interface);
	ASSERT(vdd);
	if (!vdd)
		return EINVAL;

	vdd->version = APPLE80211_VERSION;
	bcopy(dbg_flags_map, vdd->dbg_flags_map, sizeof(dbg_flags_map));

	return 0;
}

// Apple change: Add get handler for APPLE80211_IOC_BTCOEX_MODE
SInt32
AirPort_Brcm43xx::getBTCOEX_MODE( OSObject * interface, struct apple80211_btc_mode_data * btcdata )
{
	BCM_REFERENCE(interface);
	ASSERT(btcdata);
	if (!btcdata)
		return EINVAL;

	int err = BCME_OK, brcmCoexMode = 0;

	err = wlIovarGetInt("btc_mode", &brcmCoexMode);
	if(err == 0)
	{
		switch( brcmCoexMode )
		{
			case WL_BTC_DISABLE:
				btcdata->mode = APPLE80211_BTCOEX_MODE_OFF;
				break;
/* APPLE MODIFICATION <cbz@apple.com> add new enumerated values; APPLE80211_BTCOEX_MODE_FULL_TDM - APPLE80211_BTCOEX_MODE_HYBRID */
#if VERSION_MAJOR > 10
			case WL_BTC_FULLTDM:
				btcdata->mode = APPLE80211_BTCOEX_MODE_FULL_TDM;
				break;
			case WL_BTC_PREMPT:
				btcdata->mode = APPLE80211_BTCOEX_MODE_FULL_TDM_PREEMPTION;
				break;
			case WL_BTC_LITE:
				btcdata->mode = APPLE80211_BTCOEX_MODE_LIGHTWEIGHT;
				break;
			case WL_BTC_PARALLEL:
				btcdata->mode = APPLE80211_BTCOEX_MODE_UNIQUE_ANTENNAE;
				break;
			case WL_BTC_HYBRID:
				btcdata->mode = APPLE80211_BTCOEX_MODE_HYBRID;
				break;
#else
			case WL_BTC_ENABLE:
			case WL_BTC_HYBRID:
				btcdata->mode = APPLE80211_BTCOEX_MODE_ON;
				break;
#endif
			default:
				btcdata->mode = APPLE80211_BTCOEX_MODE_ON;
		}
	}

	return osl_error(err);
}

#if defined(WLBTCPROF) && defined(APPLE80211_IOC_BTCOEX_PROFILES)
SInt32
AirPort_Brcm43xx::get_iovar_select_profile( wlc_btc_select_profile_t *btc_select_profile, int len )
{
	int err = EINVAL;

	if (btc_select_profile)
	{
		if (((size_t)len < sizeof(wlc_btc_select_profile_t)) ||
			((size_t)len > sizeof(wlc_btc_select_profile_t) * BTC_SUPPORT_BANDS))
			return err;

		err = wlIovarOp("btc_select_profile", NULL, 0, (void *) btc_select_profile, len, IOV_GET, NULL);
		err = osl_error(err);
	}

	return err;
}

SInt32
AirPort_Brcm43xx::set_iovar_select_profile( wlc_btc_select_profile_t *btc_select_profile, int len )
{
	int err = EINVAL;

	if (btc_select_profile)
	{
		if (((size_t)len < sizeof(wlc_btc_select_profile_t)) ||
			((size_t)len > sizeof(wlc_btc_select_profile_t) * BTC_SUPPORT_BANDS))
			return err;

		err = wlIovarOp("btc_select_profile", NULL, 0, (void *) btc_select_profile, len, IOV_SET, NULL);
		err = osl_error(err);
	}

	return err;
}

SInt32
AirPort_Brcm43xx::getBTCOEX_PROFILES( OSObject * interface, struct apple80211_btc_profiles_data *btc_data )
{
	int cur_band = 0;
	BCM_REFERENCE(interface);

	if (btc_data)
	{
		cur_band = btc_data->band;

		if (btc_data->band == APPLE80211_MODE_11A)
		{
			memcpy(btc_data, &btc_in_profile[BTC_PROFILE_5G], sizeof(struct apple80211_btc_profiles_data));
		}
		else
		{
			memcpy(btc_data, &btc_in_profile[BTC_PROFILE_2G], sizeof(struct apple80211_btc_profiles_data));
		}

		btc_data->band = cur_band;

		return 0;
	}

	return EINVAL;
}

SInt32
AirPort_Brcm43xx::setBTCOEX_PROFILES( OSObject * interface, struct apple80211_btc_profiles_data *btc_data )
{
	int err = EINVAL;
	BCM_REFERENCE(interface);

	if (btc_data)
	{
		if (btc_data->band == APPLE80211_MODE_11A)
		{
			memset(&btc_in_profile[BTC_PROFILE_5G], 0, sizeof(struct apple80211_btc_profiles_data));

			if (btc_data->num_profiles <= APPLE80211_MAX_NUM_BTC_PROFILES)
			{
				btc_in_profile[BTC_PROFILE_5G] = *btc_data;
				btc_in_profile[BTC_PROFILE_5G].band = WLC_BAND_5G;
				err = 0;
			}
		}
		else
		{
			// for APPLE80211_MODE_11G case
			memset(&btc_in_profile[BTC_PROFILE_2G], 0, sizeof(struct apple80211_btc_profiles_data));

			if (btc_data->num_profiles <= APPLE80211_MAX_NUM_BTC_PROFILES)
			{
				btc_in_profile[BTC_PROFILE_2G] = *btc_data;
				btc_in_profile[BTC_PROFILE_2G].band = WLC_BAND_2G;
				err = 0;
			}
		}
	}

	return err;
}

SInt32
AirPort_Brcm43xx::getBTCOEX_CONFIG( OSObject * interface, struct apple80211_btc_config_data *btc_data )
{
	BCM_REFERENCE(interface);
	if (btc_data)
	{
		memcpy(btc_data, &btc_in_config, sizeof(struct apple80211_btc_config_data));

		return 0;
	}

	return EINVAL;
}

SInt32
AirPort_Brcm43xx::setup_btc_mode( wlc_btc_profile_t *btc_select_profile , uint32 *btc_mode)
{
	int err = BCME_OK;

	switch( btc_select_profile->mode )
	{
		case APPLE80211_BTCOEX_MODE_OFF:
			*btc_mode = WL_BTC_DISABLE;
			break;
		case APPLE80211_BTCOEX_MODE_ON:
			*btc_mode = WL_BTC_ENABLE;
			break;
		case APPLE80211_BTCOEX_MODE_DEFAULT:
			// Could do CPU-specific check here
			*btc_mode = WL_BTC_DEFAULT;
			break;
#if VERSION_MAJOR > 10
		case APPLE80211_BTCOEX_MODE_FULL_TDM:
			*btc_mode = WL_BTC_FULLTDM;
			break;
		case APPLE80211_BTCOEX_MODE_FULL_TDM_PREEMPTION:
			*btc_mode = WL_BTC_PREMPT;
			break;
		case APPLE80211_BTCOEX_MODE_LIGHTWEIGHT:
			*btc_mode = WL_BTC_LITE;
			break;
		case APPLE80211_BTCOEX_MODE_UNIQUE_ANTENNAE:
			*btc_mode = WL_BTC_PARALLEL;
			break;
		case APPLE80211_BTCOEX_MODE_HYBRID:
			*btc_mode = WL_BTC_HYBRID;
			break;
#endif
		default:
			err = EINVAL;
	}

	return err;
}

void
AirPort_Brcm43xx::setup_btc_chain_ack( wlc_btc_profile_t *btc_select_profile )
{
	uint32 i = 0;
	uint32 temp = 0;

	for (i = 0; i < btc_select_profile->num_chains; i++) {
		if (btc_select_profile->chain_ack[i] == APPLE80211_CHAIN_ACK_ON)
		{
			temp |= 1 << i;
		}
	}

	btc_select_profile->chain_ack[0] = temp;
}

SInt32
AirPort_Brcm43xx::setup_btc_tx_power( wlc_btc_profile_t *btc_select_profile )
{
	uint i = 0;

	if (btc_select_profile->num_chains > num_radios)
		return EINVAL;

	for (i = 0; i < btc_select_profile->num_chains; i++) {
		if (btc_select_profile->chain_power_offset[i] > 0 ||
		    btc_select_profile->chain_power_offset[i] < (SCHAR_MIN / WLC_TXPWR_DB_FACTOR)) {
			WL_INFORM(("%s chain %d: power_offset %d out of range, should be from -32 to 0\n",
			         __FUNCTION__, i, btc_select_profile->chain_power_offset[i]));
			return EINVAL;
		}
	}

	for (; i < num_radios; i++)
	{
		btc_select_profile->chain_power_offset[i] = 0;
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::setup_btc_select_profile( wlc_btc_profile_t *btc_select_profile )
{
	int err = BCME_OK;

	err = setup_btc_mode(btc_select_profile, &btc_select_profile->mode);
	if (!err)
	{
		setup_btc_chain_ack(btc_select_profile);
		err = setup_btc_tx_power(btc_select_profile);
	}

	if (err)
	{
		memset(btc_select_profile, 0, sizeof(wlc_btc_profile_t));
	}

	return err;
}

SInt32
AirPort_Brcm43xx::btc_profile_copy( wlc_btc_select_profile_t *btc_profile, int band, int profile_index )
{
	int i = 0;
	int err = BCME_OK;

	/* original btc profile */
	btc_profile[band].select_profile.mode =
		btc_in_profile[band].profiles[profile_index].mode;
	btc_profile[band].select_profile.desense =
		btc_in_profile[band].profiles[profile_index].desense;
	btc_profile[band].select_profile.desense_level =
		btc_in_profile[band].profiles[profile_index].desense_level;
	btc_profile[band].select_profile.desense_thresh_high =
		btc_in_profile[band].profiles[profile_index].desense_thresh_high;
	btc_profile[band].select_profile.desense_thresh_low =
		btc_in_profile[band].profiles[profile_index].desense_thresh_low;
	btc_profile[band].select_profile.num_chains =
		btc_in_profile[band].profiles[profile_index].num_chains;
	for (i = 0; i < WL_NUM_TXCHAIN_MAX; i++) {
		btc_profile[band].select_profile.chain_ack[i] =
			btc_in_profile[band].profiles[profile_index].chain_ack[i];
		btc_profile[band].select_profile.chain_power_offset[i] =
			btc_in_profile[band].profiles[profile_index].chain_power_offset[i];
	}
#if defined(APPLE80211_MAX_BT_DESENSE_LEVELS)
	/* extended btc profile */
	btc_profile[band].select_profile.btc_wlrssi_thresh =
		(int8) btc_in_profile[band].profiles[profile_index].wlrssi_thresh;
	btc_profile[band].select_profile.btc_wlrssi_hyst =
		(uint8) btc_in_profile[band].profiles[profile_index].wlrssi_hyst;
	btc_profile[band].select_profile.btc_btrssi_thresh =
		(int8) btc_in_profile[band].profiles[profile_index].btrssi_thresh;
	btc_profile[band].select_profile.btc_btrssi_hyst =
		(uint8) btc_in_profile[band].profiles[profile_index].btrssi_hyst;
	btc_profile[band].select_profile.btc_num_desense_levels =
		(uint8) btc_in_profile[band].profiles[profile_index].num_desense_levels;
	for (i = 0; i < APPLE80211_MAX_BT_DESENSE_LEVELS; i++) {
		btc_profile[band].select_profile.btc_siso_resp_en[i] =
			(uint8) btc_in_profile[band].profiles[profile_index].siso_resp_en[i];
		btc_profile[band].select_profile.btc_max_siso_resp_power[i] =
			(int8) btc_in_profile[band].profiles[profile_index].max_siso_resp_power[i];
	}
#endif /* APPLE80211_MAX_BT_DESENSE_LEVELS */

	return err;
}

SInt32
AirPort_Brcm43xx::setBTCOEX_CONFIG( OSObject * interface, struct apple80211_btc_config_data *btc_data )
{
	int err = EINVAL;
	wlc_btc_select_profile_t btc_profile[BTC_SUPPORT_BANDS] = {};

	BCM_REFERENCE(interface);

	if (btc_data)
	{
		btc_in_config = *btc_data;

		memset(btc_profile, 0, sizeof(struct wlc_btc_select_profile) * BTC_SUPPORT_BANDS);

		if (btc_in_config.enable_2G && (btc_in_config.profile_2G < btc_in_profile[BTC_PROFILE_2G].num_profiles))
		{
			btc_profile[BTC_PROFILE_2G].enable = BTC_PROFILE_ENABLE;
			btc_profile_copy(&btc_profile[0], BTC_PROFILE_2G, btc_in_config.profile_2G);
			err = setup_btc_select_profile(&btc_profile[BTC_PROFILE_2G].select_profile);
		}
		else if (!btc_in_config.enable_2G)
		{
			btc_profile[BTC_PROFILE_2G].enable = BTC_PROFILE_DISABLE;
			err = 0;
		}

		if (err)
			return err;
		else
			err = EINVAL;

		if (btc_in_config.enable_5G && (btc_in_config.profile_5G < btc_in_profile[BTC_PROFILE_5G].num_profiles))
		{
			btc_profile[BTC_PROFILE_5G].enable = BTC_PROFILE_ENABLE;
			btc_profile_copy(&btc_profile[0], BTC_PROFILE_5G, btc_in_config.profile_5G);
			err = setup_btc_select_profile(&btc_profile[BTC_PROFILE_5G].select_profile);
		}
		else if (!btc_in_config.enable_5G)
		{
			btc_profile[BTC_PROFILE_5G].enable = BTC_PROFILE_DISABLE;
			err = 0;
		}

		if (!err)
		{
			set_iovar_select_profile(&btc_profile[0], sizeof(wlc_btc_select_profile_t) * BTC_SUPPORT_BANDS);
		}
	}

	return (err);
}
#endif /* WLBTCPROF && APPLE80211_IOC_BTCOEX_PROFILES */

#ifdef APPLE80211_IOC_BTCOEX_OPTIONS

SInt32
AirPort_Brcm43xx::setBTCOEX_OPTIONS(OSObject *  interface, struct apple80211_btc_options_data* btc_opt)
{

	int err = BCME_OK;
	int btc_val = 0;

	BCM_REFERENCE(interface);

	ASSERT(btc_opt);

	if (!btc_opt) {
		return EINVAL;
	}

	switch( btc_opt->flags ) {

		case APPLE80211_BTCOEX_OPTION_FLAG_WIFI_TX_FLAG_DISABLE:
			btc_val = 0;
			break;
		default:
			btc_val = 0x200;
	}

	err = wlc_btc_params_set(pWLC, M_BTCX_HOST_FLAGS_OFFSET(pWLC)/2, btc_val);

	return (err);
}

SInt32
AirPort_Brcm43xx::getBTCOEX_OPTIONS(OSObject *  interface, struct apple80211_btc_options_data* btc_opt)
{
	int err = EINVAL;
	int val = 0;

	BCM_REFERENCE(interface);

	ASSERT(btc_opt);

	if (!btc_opt) {
		return EINVAL;
	}
	btc_opt->flags = 0;
	val = wlc_btc_params_get(pWLC, M_BTCX_HOST_FLAGS_OFFSET(pWLC)/2);

	WL_ERROR((" BTCOEX shmem M_BTCX_HOST_FLAGS value %x \n", val));


	if (!val ) {
		btc_opt->flags |= APPLE80211_BTCOEX_OPTION_FLAG_WIFI_TX_FLAG_DISABLE;
	} else if (val == 0xbad) {
		return err;
	}

	return 0;
}

#endif /* APPLE80211_IOC_BTCOEX_OPTIONS */

#ifdef APPLEIOCIE
bool
AirPort_Brcm43xx::matchIESignature(uint8 *signature, u_int32_t signature_len, vndr_ie_t *ie)
{
	uint8 *ie_data = (uint8*)ie;

	if (ie->id != signature[0] || ie->len < (signature_len - 1))
		return FALSE;

	if (signature_len == 1)
		return TRUE;

	// Compare the rest of the signature skipping over the length filed
	return !bcmp(&signature[1], &ie_data[VNDR_IE_HDR_LEN], signature_len - 1);
}


void
AirPort_Brcm43xx::printApple80211IE(struct apple80211_ie_data * ied, bool get, bool req)
{
	if (!WL_PORT_ON())
		return;

	WL_PORT(("%s %s:\n", get? "Get": "Set", req? "Request": "Result"));
	if (!get || (get && !req)) {
		WL_PORT(("\tframe_type_flags:0x%x\n\t\t", ied->frame_type_flags));
		if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_BEACON)
			WL_PORT(("BEACON\t"));
		if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_PROBE_RESP)
			WL_PORT(("PRBRSP\t"));
		if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_ASSOC_RESP)
			WL_PORT(("ASSOCRSP\t"));
		if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_PROBE_REQ)
			WL_PORT(("PRBREQ_FLAG\t"));
		if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_ASSOC_REQ)
			WL_PORT(("ASSOCREQ"));
		WL_PORT(("\n"));
	}

	if (!get)
		WL_PORT(("%s\n", ied->add? "Add": "Del"));

	WL_PORT(("signature_len: %d\n", ied->signature_len));

	uint8* data = (uint8*)ied->ie_data;
	(void)data; // For Release builds
	uint len = 0, count = 0, col = 0;

	if ((!get && ied->add) || (get && !req)) {
		WL_PORT(("ie_len: %d\n", ied->ie_len));
		len = ied->ie_len;
	} else
		len = ied->signature_len;

	for (count = 0; count < len;) {
		for (col = 0; (col < 20) &&
		             (count < len); col++, count++) {
			WL_PORT(("%02x ", *data++));
		}
		WL_PORT(("\n"));
	}
}

SInt32
AirPort_Brcm43xx::getIE( OSObject * interface, struct apple80211_ie_data * ied )
{
	ASSERT(ied);
	if (!ied)
		return EINVAL;

	// First get all the IEs to do a signature match against
	uchar *iebuf = NULL;
	uchar *data = NULL;
	int tot_ie = 0, pktflag = 0, iecount = 0, datalen = 0;
	vndr_ie_buf_t *ie_getbuf = NULL;
	vndr_ie_info_t *ie_info = NULL;
	vndr_ie_t *ie = NULL;
	int err = ENOENT, bcmerror = 0;
	uint8 *signature = NULL;

	printApple80211IE(ied, TRUE, TRUE);

	ie_getbuf = (vndr_ie_buf_t*)MALLOCZ(osh,  WLC_IOCTL_MAXLEN);
	if (!ie_getbuf)
		return ENOMEM;

	bcmerror = wlIovarOp( "vndr_ie",
	                    NULL,
	                    0,
	                    ie_getbuf,
	                    WLC_IOCTL_MAXLEN,
	                    FALSE,
	                    interface );

	if (bcmerror)
		goto done;

	signature = (uint8*)ied->ie_data;
	tot_ie = ie_getbuf->iecount;
	iebuf = (uchar *)&ie_getbuf->vndr_ie_list[0];
	for (iecount = 0; iecount < tot_ie; iecount++) {
		ie_info = (vndr_ie_info_t *) iebuf;

		pktflag = ie_info->pktflag;

		// Skip over pktflag
		iebuf += sizeof(uint32);

		ie = &ie_info->vndr_ie_data;
		if (matchIESignature(signature, ied->signature_len, ie)) {
			data = (uint8*) ie;
			datalen = ie->len + VNDR_IE_HDR_LEN;

			memcpy( &signature[ied->signature_len], &data[ied->signature_len+1],
			        datalen - ( ied->signature_len + 1 ) );
			ied->ie_len = datalen - 1;
			ied->frame_type_flags = 0;

			if (pktflag & VNDR_IE_BEACON_FLAG)
				ied->frame_type_flags |= APPLE80211_FRAME_TYPE_F_BEACON;

			if (pktflag & VNDR_IE_PRBRSP_FLAG)
				ied->frame_type_flags |= APPLE80211_FRAME_TYPE_F_PROBE_RESP;

			if (pktflag & VNDR_IE_ASSOCRSP_FLAG)
				ied->frame_type_flags |= APPLE80211_FRAME_TYPE_F_ASSOC_RESP;

			if (pktflag & VNDR_IE_PRBREQ_FLAG)
				ied->frame_type_flags |= APPLE80211_FRAME_TYPE_F_PROBE_REQ;

			if (pktflag & VNDR_IE_ASSOCREQ_FLAG)
				ied->frame_type_flags |= APPLE80211_FRAME_TYPE_F_ASSOC_REQ;

			err =0;
			printApple80211IE(ied, TRUE, FALSE);

			// Assumes presence of ONLY ONE element matching the signature
			goto done;
		}

		iebuf += ie->len + VNDR_IE_HDR_LEN;
	}

	WL_ERROR(("%s: IE not found!\n", __FUNCTION__));
done:
	MFREE(osh, ie_getbuf, WLC_IOCTL_MAXLEN);

	if ( bcmerror )
		return osl_error( bcmerror );

	return err;
}
#endif /* APPLEIOCIE */

#endif /* VERSION_MAJOR > 9 */

#ifdef APPLE80211_IOC_TX_CHAIN_POWER
SInt32
AirPort_Brcm43xx::getTX_CHAIN_POWER( OSObject * interface, struct apple80211_chain_power_data_get * txcpd )
{
	wl_txchain_pwr_offsets_t offsets = {};
	uint txchain = 0, txchain_pwr = 0;
	int32_t i= 0;
	int32_t state = 0;
	int min_pwr = 0, max_pwr = 0;
	int err = BCME_OK;

	BCM_REFERENCE(interface);

	ASSERT(txcpd);
	if (!txcpd)
		return EINVAL;

	if (!PHYTYPE_HT_CAP(pWLC->band))
		return EOPNOTSUPP;

	txcpd->version = APPLE80211_VERSION;
	txcpd->num_chains = (int32_t)num_radios;

	err = wlIovarGetInt("txchain", (int *)&txchain);
	if (err != 0) {
		WL_ERROR(("wl%d: %s: error %d from IOVAR \"txchain\"\n",
		          unit, __FUNCTION__, err));
		return osl_error(err);
	}

	err = wlc_stf_txchain_subval_get(pWLC, WLC_TXCHAIN_ID_PWR_LIMIT, &txchain_pwr);
	if (err != 0) {
		WL_ERROR(("wl%d: %s: error %d from wlc_stf_txchain_subval_get()\n",
		          unit, __FUNCTION__, err));
		return osl_error(err);
	}

	err = wlIovarOp("txchain_pwr_offset", NULL, 0, &offsets, sizeof(wl_txchain_pwr_offsets_t),
	                   IOV_GET, NULL);
	if (err != 0) {
		WL_ERROR(("wl%d: %s: error %d from iovar \"txchain_pwr_offset\"\n",
		          unit, __FUNCTION__, err));
		return osl_error(err);
	}

	wlc_channel_tx_power_target_min_max(pWLC, pWLC->chanspec, &min_pwr, &max_pwr);

	wlc_txchain_pwr_t last_pwr = {};

	err = wlc_get_last_txpwr(pWLC, &last_pwr);

	if (err != 0) {
		WL_PORT(("wl%d: %s: error %d \"%s\" from wlc_get_last_txpwr(), "
		         "will not report current tx power\n",
		         unit, __FUNCTION__, err, bcmerrorstr(err)));
		return osl_error(err);
	}

	// Get the sar limits
	uint8 sarlimit[WLC_TXCORE_MAX] = {};
	wlc_channel_sarlimit_subband(pWLC->cmi, pWLC->chanspec, sarlimit);


	for (i = 0; i < txcpd->num_chains; i++) {
		state = (txchain & NBITVAL(i)) ? APPLE80211_CHAIN_POWER_ON : APPLE80211_CHAIN_POWER_OFF;
		if (state == APPLE80211_CHAIN_POWER_ON &&
		    (txchain_pwr & NBITVAL(i)) == 0)
			state = APPLE80211_CHAIN_POWER_DISABLED;
		txcpd->powers[i].state = state;
		{
			txcpd->powers[i].power_offset = offsets.offset[i];
			tx_chain_offset[i] = offsets.offset[i];
		}
		txcpd->powers[i].power_min = (int32_t)(min_pwr + offsets.offset[i]) / WLC_TXPWR_DB_FACTOR;

		if (txcpd->powers[i].power_min > sarlimit[i]) {
			txcpd->powers[i].power_min = sarlimit[i];
		}

		txcpd->powers[i].power_max = (int32_t)(max_pwr + offsets.offset[i]) / WLC_TXPWR_DB_FACTOR;

		sarlimit[i] /= WLC_TXPWR_DB_FACTOR;
		if (txcpd->powers[i].power_max > sarlimit[i]) {
			txcpd->powers[i].power_max = sarlimit[i];
		}

		txcpd->powers[i].power_current = last_pwr.chain[i] / WLC_TXPWR_DB_FACTOR;


		WL_PORT(("wl%d: %s: chain %d state %d offset %d min %d max %d current %d\n",
		         unit, __FUNCTION__,
		         i,
		         txcpd->powers[i].state,
		         txcpd->powers[i].power_offset,
		         txcpd->powers[i].power_min,
		         txcpd->powers[i].power_max,
		         txcpd->powers[i].power_current
		        ));
	}

	return 0;
}

struct txp_range {
	int start;
	int end;
};

#ifndef PPR_API
static const struct txp_range txp_ranges_20[] = {
	/* All 20MHz rates */
	{WL_TX_POWER_CCK_FIRST, (WL_TX_POWER_20_S3x3_FIRST + WL_NUM_RATES_MCS_1STREAM - 1)},
	{-1, -1}
};

static const struct txp_range txp_ranges_40[] = {
	/* All 40MHz rates */
	{WL_TX_POWER_OFDM40_FIRST, (WL_TX_POWER_40_S3x3_FIRST + WL_NUM_RATES_MCS_1STREAM - 1)},
	/* All 20 in 40MHz rates */
	{WL_TX_POWER_20UL_CCK_FIRST, (WL_TX_POWER_20UL_S3x3_FIRST + WL_NUM_RATES_MCS_1STREAM - 1)},
	{-1, -1}
};
#endif // !PPR_API
#endif /* APPLE80211_IOC_TX_CHAIN_POWER */


#ifdef FACTORY_MERGE
SInt32
AirPort_Brcm43xx::getFACTORY_MODE( IO80211Interface * interface, struct apple80211_factory_mode_data * fmd )
{
	BCM_REFERENCE(interface);
	ASSERT(fmd);
	if (!fmd)
		return EINVAL;

	int err = BCME_OK;

	fmd->powersave = (disableFactoryPowersave)? APPLE80211_FACTORY_MODE_OFF: APPLE80211_FACTORY_MODE_ON;
	fmd->countryadopt = (disableFactory11d)? APPLE80211_FACTORY_MODE_OFF: APPLE80211_FACTORY_MODE_ON;
	fmd->roam = (disableFactoryRoam)? APPLE80211_FACTORY_MODE_OFF: APPLE80211_FACTORY_MODE_ON;

	return (err);

}
#endif

#ifdef APPLE80211_IOC_INTERRUPT_STATS
#define MAX_NUM_INTR	32

#ifdef BCMDBG
static const struct {
	uint8 id;
	char name[20];
} intr_names[] = {
	{  0, "MI_MACSSPNDD" },		/* MAC has gracefully suspended */
	{  1, "MI_BCNTPL" },		/* beacon template available */
	{  2, "MI_TBTT" },		/* TBTT indication */
	{  3, "MI_BCNSUCCESS" },	/* beacon successfully tx'd */
	{  4, "MI_BCNCANCLD" },		/* beacon canceled (IBSS) */
	{  5, "MI_ATIMWINEND" },	/* end of ATIM-window (IBSS) */
	{  6, "MI_PMQ" },		/* PMQ entries available */
	{  7, "MI_NSPECGEN_0" },	/* non-specific gen-stat bits that are set by PSM */
	{  8, "MI_NSPECGEN_1" },	/* non-specific gen-stat bits that are set by PSM */
	{  9, "MI_MACTXERR" },		/* MAC level Tx error */
	{ 10, "MI_NSPECGEN_3" },	/* non-specific gen-stat bits that are set by PSM */
	{ 11, "MI_PHYTXERR" },		/* PHY Tx error */
	{ 12, "MI_PME" },		/* Power Management Event */
	{ 13, "MI_GP0" },		/* General-purpose timer0 */
	{ 14, "MI_GP1" },		/* General-purpose timer1 */
	{ 15, "MI_DMAINT" },		/* (ORed) DMA-interrupts */
	{ 16, "MI_TXSTOP" },		/* MAC has completed a TX FIFO Suspend/Flush */
	{ 17, "MI_CCA" },		/* MAC has completed a CCA measurement */
	{ 18, "MI_BG_NOISE" },		/* MAC has collected background noise samples */
	{ 19, "MI_DTIM_TBTT" },		/* MBSS DTIM TBTT indication */
	{ 20, "MI_PRQ" },		/* Probe response queue needs attention */
	{ 21, "MI_PWRUP" },		/* Radio/PHY has been powered back up. */
	{ 22, "MI_BT_RFACT_STUCK" },	/* MAC has detected invalid BT_RFACT pin */
	{ 23, "MI_BT_PRED_REQ" },	/* MAC requested driver BTCX predictor calc */
	{ 24, "MI_NOTUSED" },
	{ 25, "MI_P2P" },		/* WiFi P2P interrupt */
	{ 26, "MI_DMATX" },		/* MAC new frame ready */
	{ 27, "MI_TSSI_LIMIT" },	/* Tssi Limit Reach, TxIdx=0/127 Interrupt */
	{ 28, "MI_RFDISABLE" },		/* MAC detected a change on RF Disable input (corerev >= 10) */
	{ 29, "MI_TFS" },		/* MAC has completed a TX (corerev >= 5) */
	{ 30, "MI_PHYCHANGED" },	/* A phy status change wrt G mode */
	{ 31, "MI_TO" }			/* general purpose timeout (corerev >= 3) */
};
#endif

SInt32
AirPort_Brcm43xx::getINTERRUPT_STATS(OSObject *interface, struct apple80211_interrupt_stats_data *isd)
{
	int err = EOPNOTSUPP;
#ifdef WL_INTERRUPTSTATS
	struct apple80211_interrupt_stat *stats = NULL;
	uint count = 0;
	uint id = 0, n_rec = 0, len = 0;

	BCM_REFERENCE(interface);

	if (isd == NULL)
		return EINVAL;

	n_rec = sizeof(isd->stats)/sizeof(struct apple80211_interrupt_stat);
	WL_PORT(("%s: input buffersize hold upto %d records\n", __FUNCTION__, n_rec));
	if (n_rec < MAX_NUM_INTR)
		return ENOMEM;
	for (id = 0; id < n_rec && id < MAX_NUM_INTR; id++) {
		if (intr_cnt[id] == 0)
			continue;
		stats = &isd->stats[count++];
		len = MIN(strlen(intr_names[id].name), APPLE80211_PERF_STAT_DESC_LEN - 1);
		strncpy(stats->desc, intr_names[id].name, len);
		stats->desc[len] = 0;
		stats->interrupt_count = intr_cnt[id];
		WL_PORT(("intr %s: count: %d\n",
		       stats->desc, (uint)stats->interrupt_count));
	}
	isd->stat_count = count;
	err = 0;
#endif /* WL_INTERRUPTSTATS */
	return err;
}
#endif /* APPLE80211_IOC_INTERRUPT_STATS */

#ifdef APPLE80211_IOC_TIMER_STATS
SInt32
AirPort_Brcm43xx::getTIMER_STATS(OSObject *interface, struct apple80211_timer_stats_data *tsd)
{
	int err = EOPNOTSUPP;
	BCM_REFERENCE(interface);
#ifdef WL_TIMERSTATS
	AirPort_Brcm43xxTimer *t = timers;
	struct apple80211_timer_stat *stats = NULL;
	uint id = 0, count = 0;
	uint len = 0, n_rec = 0;

	if (tsd == NULL)
		return EINVAL;

	n_rec = sizeof(tsd->stats)/sizeof(struct apple80211_timer_stat);
	WL_PORT(("%s: input buffersize hold upto %d records\n", __FUNCTION__, n_rec));
	while(t != NULL && id < n_rec) {
		if (t->scheduled_count == 0) {
			t = t->next;
			id++;
			continue;
		}
		stats = &tsd->stats[count++];
		len = MIN(strlen(t->name), APPLE80211_PERF_STAT_DESC_LEN - 1);
		strncpy(stats->desc, t->name, len);
		stats->desc[len] = 0;
		stats->scheduled_count = t->scheduled_count;
		stats->canceled_count = t->canceled_count;
		stats->fired_count = t->fired_count;
		t = t->next;
		WL_PORT(("timer %s: scheduled: %d  canceled: %d  fired: %d\n",
		       stats->desc, (uint)stats->scheduled_count,
		       (uint)stats->canceled_count, (uint)stats->fired_count));
	}
	tsd->stat_count = count;
	err = 0;
	if (t != NULL)
		err = ENOMEM;
#endif /* WL_TIMERSTATS */
	return err;
}
#endif /* APPLE80211_IOC_TIMER_STATS */

#ifdef APPLE80211_IOC_OFFLOAD_STATS
SInt32
AirPort_Brcm43xx::getOFFLOAD_STATS(OSObject *interface, struct apple80211_offload_stats_data *osd)
{
	BCM_REFERENCE(interface);
	BCM_REFERENCE(osd);
	return 0;
}

SInt32
AirPort_Brcm43xx::setOFFLOAD_STATS_RESET(OSObject *interface)
{
	BCM_REFERENCE(interface);
	return 0;
}
#endif /* APPLE80211_IOC_OFFLOAD_STATS */
#if VERSION_MAJOR > 10
#ifdef APPLE80211_IOC_DESENSE_LEVEL
SInt32
AirPort_Brcm43xx::getDESENSE_LEVEL( OSObject * interface, struct apple80211_desense_level_data * btcdata )
{
	BCM_REFERENCE(interface);
    ASSERT(btcdata);
    if (!btcdata)
        return EINVAL;

	int err = BCME_OK, restage_rxgain_level = 0;

	err = wlIovarGetInt("btc_rxgain_level", &restage_rxgain_level);

	if(err == 0) {

		btcdata->level = restage_rxgain_level;
	}

	return osl_error(err);
}

SInt32
AirPort_Brcm43xx::setDESENSE_LEVEL( OSObject * interface, struct apple80211_desense_level_data * btcdata )
{
	BCM_REFERENCE(interface);
    ASSERT(btcdata);
    if (!btcdata)
        return EINVAL;

	int err = BCME_OK;

	err = wlIovarSetInt("btc_rxgain_level", btcdata->level);

	return osl_error(err);
}
#endif /* APPLE80211_IOC_DESENSE_LEVEL */
#endif /* MAJOR_VERSION > 10 */

#ifdef APPLE80211_IOC_OFFLOAD_ARP
SInt32
AirPort_Brcm43xx::getOFFLOAD_ARP( OSObject * interface, struct apple80211_offload_arp_data * oad )
{
	BCM_REFERENCE(interface);
	BCM_REFERENCE(oad);
	return 0;
}

SInt32
AirPort_Brcm43xx::setOFFLOAD_ARP( OSObject * interface, struct apple80211_offload_arp_data * oad )
{
	BCM_REFERENCE(interface);
	BCM_REFERENCE(oad);
	return 0;
}
#endif /* APPLE80211_IOC_OFFLOAD_ARP */

#ifdef APPLE80211_IOC_OFFLOAD_NDP
SInt32
AirPort_Brcm43xx::getOFFLOAD_NDP( OSObject * interface, struct apple80211_offload_ndp_data * ond )
{
	BCM_REFERENCE(interface);
	BCM_REFERENCE(ond);
	return 0;
}

SInt32
AirPort_Brcm43xx::setOFFLOAD_NDP( OSObject * interface, struct apple80211_offload_ndp_data * ond )
{
	BCM_REFERENCE(interface);
	BCM_REFERENCE(ond);
	return 0;
}

#endif /* APPLE80211_IOC_OFFLOAD_NDP */

#ifdef APPLE80211_IOC_OFFLOAD_SCAN

SInt32
AirPort_Brcm43xx::getOFFLOAD_SCAN( OSObject * interface, struct apple80211_offload_scan_data * osd )
{
	BCM_REFERENCE(interface);
	BCM_REFERENCE(osd);
	return 0;
}

SInt32
AirPort_Brcm43xx::setOFFLOAD_SCAN( OSObject * interface, struct apple80211_offload_scan_data * osd )
{
	BCM_REFERENCE(interface);
	BCM_REFERENCE(osd);
	return 0;
}
#endif /* APPLE80211_IOC_OFFLOAD_SCAN */
#ifdef APPLE80211_IOC_TX_NSS
SInt32
AirPort_Brcm43xx::getTX_NSS( OSObject * interface, struct apple80211_tx_nss_data * tnd )
{
	BCM_REFERENCE(interface);
	ASSERT(tnd);
	if (!tnd)
	{
		return EINVAL;
	}

	int txstream_value = 0;

	int err = wlc_iovar_getint(pWLC, "tx_nss", &txstream_value);

	if(err == 0)
	{
		tnd->txstream_value = txstream_value;
	}

	return osl_error(err);
}

SInt32
AirPort_Brcm43xx::setTX_NSS( OSObject * interface, struct apple80211_tx_nss_data * tnd )
{
	BCM_REFERENCE(interface);
	ASSERT(tnd);
	if (!tnd)
	{
		return EINVAL;
	}
	int err = wlc_iovar_setint(pWLC, "tx_nss", tnd->txstream_value);

	return osl_error(err);
}
#endif /* APPLE80211_IOC_TX_NSS */
#endif /* !MACOSX_DHD */
#ifdef APPLE80211_IOC_ROAM_PROFILE
SInt32
AirPort_Brcm43xx::getROAM_PROFILE( OSObject * interface, struct apple80211_roam_profile_band_data * rpbd )
{
	SInt32 err = BCME_OK;
	wl_roam_prof_band_t bcm_rpband;
	u_int32_t band = 0;
	uint i = 0;

	BCM_REFERENCE(interface);
	ASSERT(rpbd);
	if (!rpbd)
		return EINVAL;

	bzero(&bcm_rpband, sizeof(wl_roam_prof_band_t));
	bcm_rpband.ver = WL_MAX_ROAM_PROF_VER;

	band = rpbd->band;

	if ((band & APPLE80211_MODE_11A) && (band & (APPLE80211_MODE_11B | APPLE80211_MODE_11G)))
		/* Can be only one band at a time */
		return EINVAL;
	else if (band & APPLE80211_MODE_11A)
		bcm_rpband.band =  WLC_BAND_5G;
	else
		bcm_rpband.band =  WLC_BAND_2G;

	err = wlIovarOp("roam_prof", NULL, 0, &bcm_rpband, sizeof(wl_roam_prof_band_t), IOV_GET, interface);
	if (err) {
		WL_ROAM(("wl%d: getROAM_PROFILE: err from wlIovarOp(\"roam_prof\"), %d \"%s\"\n",
		          unit, err, bcmerrorstr(err)));
		return osl_error(err);
	}
	bzero(rpbd, sizeof(*rpbd));
	rpbd->version = APPLE80211_VERSION;
	rpbd->band = band;
	rpbd->prof_num = bcm_rpband.len / sizeof(wl_roam_prof_t);

	for (i = 0; i < rpbd->prof_num; ++i) {
		struct apple80211_roam_profile * current_rp = &rpbd->profile[i];
		wl_roam_prof_t * bcm_current_rp = &bcm_rpband.roam_prof[i];

		if ((i * sizeof(wl_roam_prof_t)) > bcm_rpband.len)
			break;

		/* The full scan period must be non-zero for valid roam profile */
		if (bcm_current_rp->fullscan_period == 0)
			break;

		current_rp->roam_flags = bcm_current_rp->roam_flags;
		current_rp->roam_trigger = bcm_current_rp->roam_trigger;
		current_rp->rssi_lower = bcm_current_rp->rssi_lower;
		current_rp->roam_delta = bcm_current_rp->roam_delta;
		current_rp->rssi_boost_thresh = bcm_current_rp->rssi_boost_thresh;
		current_rp->rssi_boost_delta = bcm_current_rp->rssi_boost_delta;
		current_rp->nfscan = bcm_current_rp->nfscan;
		current_rp->fullscan_period = bcm_current_rp->fullscan_period;
		current_rp->init_scan_period = bcm_current_rp->init_scan_period;
		current_rp->backoff_multiplier = bcm_current_rp->backoff_multiplier;
		current_rp->max_scan_period = bcm_current_rp->max_scan_period;

		WL_ROAM(("wl%d: getROAM_PROFILE: [%d] flags:%02x RSSI[%d,%d] delta:%d boost:%d.by.%d \
			 nfscan:%d period(full:%ds init:%ds x%d max:%ds)\n",
			 unit,
			 i,
			 current_rp->roam_flags,
			 current_rp->roam_trigger,
			 current_rp->rssi_lower,
			 current_rp->roam_delta,
			 current_rp->rssi_boost_thresh,
			 current_rp->rssi_boost_delta,
			 current_rp->nfscan,
			 current_rp->fullscan_period,
			 current_rp->init_scan_period,
			 current_rp->backoff_multiplier,
			 current_rp->max_scan_period));
	}

	if (i < rpbd->prof_num)
		rpbd->prof_num = i;

	return 0;
}

SInt32
AirPort_Brcm43xx::setROAM_PROFILE( OSObject * interface, struct apple80211_roam_profile_band_data *rpbd )
{
	SInt32 err = BCME_OK;
	wl_roam_prof_band_t bcm_rpband = {};
	uint i = 0;

	BCM_REFERENCE(interface);
	ASSERT(rpbd);
	if (!rpbd)
		return EINVAL;

	IOVERCHECK(rpbd->version, "setROAM_PROFILE");

	bzero(&bcm_rpband, sizeof(wl_roam_prof_band_t));
	bcm_rpband.ver = WL_MAX_ROAM_PROF_VER;

	if ((rpbd->band & APPLE80211_MODE_11A) && (rpbd->band & (APPLE80211_MODE_11B | APPLE80211_MODE_11G)))
		/* Can be only one band at a time */
		return EINVAL;
	else if (rpbd->band & APPLE80211_MODE_11A)
		bcm_rpband.band =  WLC_BAND_5G;
	else
		bcm_rpband.band =  WLC_BAND_2G;

	bcm_rpband.ver = WL_MAX_ROAM_PROF_VER;
	bcm_rpband.len = rpbd->prof_num * sizeof(wl_roam_prof_t);

	for (i = 0; i < rpbd->prof_num; ++i) {
		struct apple80211_roam_profile * current_rp = &rpbd->profile[i];
		wl_roam_prof_t * bcm_current_rp = &bcm_rpband.roam_prof[i];

		bcm_current_rp->roam_flags = current_rp->roam_flags;
		bcm_current_rp->roam_trigger = current_rp->roam_trigger;
		bcm_current_rp->rssi_lower = current_rp->rssi_lower;
		bcm_current_rp->roam_delta = current_rp->roam_delta;
		bcm_current_rp->rssi_boost_thresh = current_rp->rssi_boost_thresh;
		bcm_current_rp->rssi_boost_delta = current_rp->rssi_boost_delta;
		bcm_current_rp->nfscan = current_rp->nfscan;
		bcm_current_rp->fullscan_period = current_rp->fullscan_period;
		bcm_current_rp->init_scan_period = current_rp->init_scan_period;
		bcm_current_rp->backoff_multiplier = current_rp->backoff_multiplier;
		bcm_current_rp->max_scan_period = current_rp->max_scan_period;

		WL_ROAM(("wl%d: setROAM_PROFILE: [%d] flags:%02x RSSI[%d,%d] delta:%d boost:%d.by.%d \
			 nfscan:%d period(full:%ds init:%ds x%d max:%ds)\n",
			 unit,
			 (int)i,
			 current_rp->roam_flags,
			 current_rp->roam_trigger,
			 current_rp->rssi_lower,
			 current_rp->roam_delta,
			 current_rp->rssi_boost_thresh,
			 current_rp->rssi_boost_delta,
			 current_rp->nfscan,
			 current_rp->fullscan_period,
			 current_rp->init_scan_period,
			 current_rp->backoff_multiplier,
			 current_rp->max_scan_period));
	}

	err = wlIovarOp("roam_prof", NULL, 0, &bcm_rpband, sizeof(wl_roam_prof_band_t), IOV_SET, interface);
	if (err) {
		WL_ROAM(("wl%d: setROAM_PROFILE: err from wlIovarOp(\"roam_prof\"), %d \"%s\"\n",
		         unit, err, bcmerrorstr(err)));
	}

	return osl_error(err);

}
#endif /* APPLE80211_IOC_ROAM_PROFILE */

#pragma mark -
#pragma mark IO80211Controller IOCTL Set Handlers

// -----------------------------------------------------------------------------
//
// IO80211Controller IOCTL Set Handlers
//
// -----------------------------------------------------------------------------

SInt32
AirPort_Brcm43xx::setCHANNEL( OSObject * interface, struct apple80211_channel_data * cd )
{
	uint16 chanspec = 0;
	int channel = 0;
	uint32 flags = 0;
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
	wlc_bsscfg_t *bsscfg = wl_find_bsscfg(wlcif);
#endif /* !MACOSX_DHD */
	int err = BCME_OK;

	ASSERT(cd);
	if (!cd)
		return EINVAL;

	IOVERCHECK(cd->version, "setCHANNEL");
#ifndef MACOSX_DHD
	if (bsscfg == NULL) {
		WL_ERROR(("setCHANNEL: error, can't find bsscfg associated to wlcif %p\n", OSL_OBFUSCATE_BUF(wlcif)));
		return EINVAL;
	}
#endif /* !MACOSX_DHD */
	channel = (int)cd->channel.channel;
	flags = cd->channel.flags;
#ifndef MACOSX_DHD
	WL_IOCTL(("wl%d.%d: setCHANNEL: setting channel %u flags 0x%x\n",
	          unit, bsscfg->_idx, channel, flags));
#else
	WL_IOCTL(("wl%d: setCHANNEL: setting channel %u flags 0x%x\n",
	          unit, channel, flags));
#endif /* !MACOSX_DHD */

	if (flags & APPLE80211_C_FLAG_10MHZ)
		chanspec |= WL_CHANSPEC_BW_10;
	else if (flags & APPLE80211_C_FLAG_20MHZ)
		chanspec |= WL_CHANSPEC_BW_20;
	else if (flags & APPLE80211_C_FLAG_40MHZ)
		chanspec |= WL_CHANSPEC_BW_40;
#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	else if (flags & APPLE80211_C_FLAG_80MHZ)
		chanspec |= WL_CHANSPEC_BW_80;
#endif
	else
		return EINVAL;

	if (flags & APPLE80211_C_FLAG_2GHZ)
		chanspec |= WL_CHANSPEC_BAND_2G;
	else if (flags & APPLE80211_C_FLAG_5GHZ)
		chanspec |= WL_CHANSPEC_BAND_5G;
	else
		return EINVAL;

#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	if (CHSPEC_IS80(chanspec) || CHSPEC_IS160(chanspec)) {
		chanspec = wf_channel2chspec(channel, chanspec & WL_CHANSPEC_BW_MASK);
		if (chanspec <= 0) {
			return EINVAL;
		}
	} else {
#endif
	if (CHSPEC_IS40(chanspec)) {
		if (flags & APPLE80211_C_FLAG_EXT_ABV) {
			chanspec |= WL_CHANSPEC_CTL_SB_L;
			channel += 2;	/* center channel is above control */
		} else {
			chanspec |= WL_CHANSPEC_CTL_SB_U;
			channel -= 2;	/* center channel is below control */
		}
	}

	if (channel < 0 || channel > WL_CHANSPEC_CHAN_MASK)
		return EINVAL;

	chanspec |= channel;

#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	}
#endif
#ifndef MACOSX_DHD
	// for active ap that has stations associated to it, send out csa with channel switch
	if (BSSCFG_AP(bsscfg) && bsscfg->up) {
		wl_chan_switch_t csa = {};

		csa.mode = DOT11_CSA_MODE_ADVISORY;
		csa.count = bsscfg->current_bss->dtim_period + 1;
		csa.chspec = chanspec;
		csa.reg = wlc_get_regclass(pWLC->cmi, chanspec);
		err = wlIovarOp("csa", NULL, 0, &csa, sizeof(wl_chan_switch_t), IOV_SET, interface);
	} else
#endif /* !MACOSX_DHD */
	{
		int chanSpecInt = (int)chanspec;
		err = wlIovarOp("chanspec", NULL, 0, &chanSpecInt, sizeof( chanSpecInt ), IOV_SET, interface);
	}
	if (err)
		WL_IOCTL(("wl%d: setCHANNEL: error setting chanspec 0x%x err %d \"%s\"\n",
		          unit, chanspec, err, bcmerrorstr(err)));

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setPROTMODE( OSObject * interface, struct apple80211_protmode_data * pd )
{
	int val = WLC_PROTECTION_AUTO;

	BCM_REFERENCE(interface);

	ASSERT(pd);
	if (!pd)
		return EINVAL;

	IOVERCHECK(pd->version, "setPROTMODE");

	if( pd->threshold > DOT11_MAX_RTS_LEN )
		return EINVAL;

	switch( pd->protmode )
	{
		case APPLE80211_PROTMODE_OFF:
			val = WLC_PROTECTION_OFF;
			break;
		case APPLE80211_PROTMODE_CTS:
		case APPLE80211_PROTMODE_RTSCTS:
			val = WLC_PROTECTION_ON;
			break;
		case APPLE80211_PROTMODE_AUTO:
			val = WLC_PROTECTION_AUTO;

	}

	int result = wlIoctlSetInt(WLC_SET_GMODE_PROTECTION_OVERRIDE, val);

	return result == 0 ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setTXPOWER( OSObject * interface, struct apple80211_txpower_data * td )
{
	int err = BCME_OK;
	uint8 qdbm = WLC_TXPWR_MAX;
	int percent = 100;

	BCM_REFERENCE(interface);

	ASSERT(td);
	if (!td)
		return EINVAL;

	IOVERCHECK(td->version, "setTXPOWER");

	switch (td->txpower_unit) {
	case APPLE80211_UNIT_MW:
		qdbm = bcm_mw_to_qdbm((uint16)MIN(td->txpower, 0xffff));
		break;

	case APPLE80211_UNIT_DBM:
		if (td->txpower < 0)
			qdbm = 0;
		else
			qdbm = MIN(td->txpower * 4, WLC_TXPWR_MAX);
		break;

	case APPLE80211_UNIT_PERCENT:
		if (td->txpower < 0 || td->txpower > 100)
			err = EINVAL;
		percent = td->txpower;
		break;

	default:
		err = EINVAL;
		break;
	}

	WL_IOCTL(("wl%d: setTXPOWER: value %d units %s (#%u)\n",
	          unit, (int)td->txpower,
	          lookup_name(apple80211_unit_names, td->txpower_unit), td->txpower_unit));

	if (err)
		return err;

	err = wlIovarSetInt("qtxpower", qdbm);
	if (err)
		return osl_error(err);

	err = wlIoctlSetInt(WLC_SET_PWROUT_PERCENTAGE, percent);
	if (err)
		return osl_error(err);

	return 0;
}

SInt32
AirPort_Brcm43xx::setSCAN_REQ( OSObject * interface, struct apple80211_scan_data * sd )
{
	static struct ether_addr	anyBSSID = {{0xff,0xff,0xff,0xff,0xff,0xff}};
	struct ether_addr		*bssidPtr = NULL;
	int bssType = 0;
	int scanType = 0, activeTime = -1, passiveTime = -1, homeTime = -1;
	int bcmerr = 0;
	chanspec_t chanspec_list[MAXCHANNEL] = {};
	int chanspec_count = 0;
	wlc_ssid_t ssid = { 0, {0} };
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN] = {};
	char ssidbuf[SSID_FMT_BUF_LEN] = {};
	const int flagstr_size = 64;
	char flagstr[flagstr_size] = {};
#endif
	uint scanFlags = 0;
#ifdef MACOSX_DHD
	int params_size;
	wl_escan_params_t *escan;
	wl_scan_params_t *scan_p;
	int i;
#endif /* MACOSX_DHD */

	BCM_REFERENCE(interface);

	ASSERT(sd);
	if (!sd)
		return EINVAL;

	IOVERCHECK(sd->version, "setSCAN_REQ");

	KERNEL_DEBUG_CONSTANT(WiFi_apple80211Request_setSCAN_REQ | DBG_FUNC_NONE, 0, 0, 0, 0, 0);

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		WL_IOCTL(("wl%d: setSCAN_REQ:\n", unit));
		WL_IOCTL(("  %-18s%u (%s)\n", "bss_type", sd->bss_type,
		          lookup_name(apple80211_apmode_names, sd->bss_type)));
		WL_IOCTL(("  %-18s%s\n", "bssid", bcm_ether_ntoa(&sd->bssid, eabuf)));
#ifdef MACOSX_DHD
		wl_format_ssid(ssidbuf, sd->ssid, sd->ssid_len);
#else
		wlc_format_ssid(ssidbuf, sd->ssid, sd->ssid_len);
#endif /* MACOSX_DHD */
		WL_IOCTL(("  %-18s\"%s\"\n", "ssid", ssidbuf));
		WL_IOCTL(("  %-18s%u (%s)\n", "scan_type", sd->scan_type,
		          lookup_name(apple80211_scan_type_names, sd->scan_type)));
		bcm_format_flags(apple80211_phymode_flags, sd->phy_mode, flagstr, flagstr_size);
		WL_IOCTL(("  %-18s%s\n", "phy_mode", flagstr));
		WL_IOCTL(("  %-18s%u\n", "dwell_time", (uint)sd->dwell_time));
		WL_IOCTL(("  %-18s%u\n", "rest_time", (uint)sd->rest_time));
		WL_IOCTL(("  %-18s%u\n", "num_channels", (uint)sd->num_channels));
		dump_apple80211_channels(sd->num_channels, sd->channels);
	}
#endif

	if (scanreq_common(sd->bss_type, sd->scan_type, sd->dwell_time, sd->rest_time,
	                   sd->phy_mode, sd->num_channels, sd->channels,
	                   &bssType, &scanType, &activeTime, &passiveTime, &homeTime,
	                   chanspec_list, &chanspec_count, &scanFlags) == EINVAL) {
		return EINVAL;
	}

	// Get bssid
	if( ETHER_ISNULLADDR(&sd->bssid) )
		bssidPtr = &anyBSSID;
	else
		bssidPtr = &sd->bssid;

	// Copy specified ssid info into the local wlc_ssid_t struct
	if (sd->ssid_len > 0) {
		ssid.SSID_len = MIN(sd->ssid_len, DOT11_MAX_SSID_LEN);
		memcpy(ssid.SSID, sd->ssid, ssid.SSID_len);
	}
#ifndef MACOSX_DHD
	// If this is a fast scan then just return the current scan cache.
	if (sd->scan_type == APPLE80211_SCAN_TYPE_FAST) {
		wlc_bss_list_t bss_list;

		memset(&bss_list, 0, sizeof(bss_list));
		if (SCANCACHE_ENAB(pWLC->scan)) {
			wlc_scan_get_cache(pWLC->scan, bssidPtr, 1, &ssid, bssType,
			                   chanspec_list, chanspec_count, &bss_list);
		}

		scanComplete(&bss_list);
		return 0;
	}
#ifdef SCAN_JOIN_TIMEOUT
	pWLC->scan_timeout = SCAN_TIMEOUT_FAIL;
#endif
	bcmerr = wlc_scan_request_ex(	pWLC,
					bssType,				// bsstype
					bssidPtr,
	                                1, &ssid,
					scanType,				// scan_type,
					-1,					// nprobes,
					activeTime,				// active_time,
					passiveTime,				// passive_time,
					homeTime,    				// home_time,
					chanspec_list,				// chanspec_list,
					chanspec_count,				// channel_num,
					0,
					true,
					apple80211_scan_done,	// callback
					this,
					WLC_ACTION_SCAN,
					scanFlags,
					NULL,
					NULL,
					NULL );

	// save the last bcmerr in case queried
	if (bcmerr && VALID_BCMERROR(bcmerr))
		pub->bcmerror = bcmerr;
#else

	params_size = WL_SCAN_PARAMS_FIXED_SIZE + sizeof(wlc_ssid_t) + (chanspec_count * sizeof(uint16));
	escan = (wl_escan_params_t *)IOMalloc(params_size);
	if (!escan) {
		WL_ERROR(("%s: MALLOC(%d) failed\n", __FUNCTION__, params_size));
		return ENOMEM;
	}

	memset(escan, 0, params_size);
	escan->version = ESCAN_REQ_VERSION;
	escan->action = WL_SCAN_ACTION_START;
	escan->sync_id = 4321;

	scan_p = &escan->params;
	scan_p->ssid.SSID_len = ssid.SSID_len;
	memcpy(scan_p->ssid.SSID, ssid.SSID, ssid.SSID_len);
	memcpy(&scan_p->bssid, bssidPtr, sizeof(scan_p->bssid));
	scan_p->bss_type = bssType;
	scan_p->scan_type = scanType;
	scan_p->nprobes = -1;
	scan_p->active_time = activeTime;
	scan_p->passive_time = passiveTime;
	scan_p->home_time = homeTime;
	scan_p->channel_num = chanspec_count;

	for (i = 0; i < chanspec_count; i++) {
		scan_p->channel_list[i] = chanspec_list[i];
	}

	bcmerr = wlIovarOp( "escan", escan, params_size, NULL, 0, IOV_SET, NULL);
	if (bcmerr) {
		WL_ERROR(("%s: wlIovarOp failed for escan error:%d\n", __FUNCTION__, bcmerr));
	}
	IOFree(escan, params_size);
#endif /* !MACOSX_DHD */

	return osl_error(bcmerr);
}

#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::setSCAN_REQ_MULTIPLE( OSObject * interface, struct apple80211_scan_multiple_data * sd )
{
	static struct ether_addr	anyBSSID = {{0xff,0xff,0xff,0xff,0xff,0xff}};
	struct ether_addr		*bssidPtr = NULL;
	int bssType = 0;
	int scanType = 0, activeTime = -1, passiveTime = -1, homeTime = -1;
	int bcmerr = 0;
	chanspec_t chanspec_list[MAXCHANNEL] = {};
	int chanspec_count = 0;
	int i = 0;
	wlc_ssid_t ssidlist[APPLE80211_MAX_SCAN_SSID_CNT];
	uint32 nssid = 0;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
	char ssidbuf[SSID_FMT_BUF_LEN] = {0};
	const int flagstr_size = 64;
	char flagstr[flagstr_size] = {0};
#endif
	uint scanFlags = 0;

	BCM_REFERENCE(interface);
	ASSERT(sd);
	if (!sd)
		return EINVAL;

	IOVERCHECK(sd->version, "setSCAN_REQ_MULTIPLE");

	KERNEL_DEBUG_CONSTANT(WiFi_apple80211Request_setSCAN_REQ_MULTIPLE | DBG_FUNC_NONE, 0, 0, 0, 0, 0);

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		WL_IOCTL(("wl%d: setSCAN_REQ_MULTIPLE:\n", unit));
		WL_IOCTL(("  %-18s%d\n", "version", sd->version));
		WL_IOCTL(("  %-18s%u (%s)\n", "bss_type", sd->bss_type,
		          lookup_name(apple80211_apmode_names, sd->bss_type)));
		for (i = 0; i < (int)sd->bssid_list_count; i++) {
			WL_IOCTL(("  %-18s%s\n", "bssid", bcm_ether_ntoa(&sd->bssid_list[i], eabuf)));
		}
		for (i = 0; i < (int)sd->ssid_list_count; i++) {
			wlc_format_ssid(ssidbuf, sd->ssid_list[i].ssid_bytes, sd->ssid_list[i].ssid_len);
			WL_IOCTL(("  %-18s\"%s\"\n", "ssid", ssidbuf));
		}
		WL_IOCTL(("  %-18s%u (%s)\n", "scan_type", sd->scan_type,
		          lookup_name(apple80211_scan_type_names, sd->scan_type)));
		bcm_format_flags(apple80211_phymode_flags, sd->phy_mode, flagstr, flagstr_size);
		WL_IOCTL(("  %-18s%s\n", "phy_mode", flagstr));
		WL_IOCTL(("  %-18s%u\n", "dwell_time", (uint)sd->dwell_time));
		WL_IOCTL(("  %-18s%u\n", "rest_time", (uint)sd->rest_time));
		WL_IOCTL(("  %-18s%u\n", "num_channels", (uint)sd->num_channels));
		dump_apple80211_channels(sd->num_channels, sd->channels);
	}
#endif

	if (scanreq_common(sd->bss_type, sd->scan_type, sd->dwell_time, sd->rest_time,
	                   sd->phy_mode, sd->num_channels, sd->channels,
	                   &bssType, &scanType, &activeTime, &passiveTime, &homeTime,
	                   chanspec_list, &chanspec_count, &scanFlags) == EINVAL) {
		return EINVAL;
	}

	// zero out ssidlist
	bzero(ssidlist, (sizeof(wlc_ssid_t) * APPLE80211_MAX_SCAN_SSID_CNT));

	// indicate we're about to do a multiple scan
	inMultiScan = TRUE;

	// Get bssid list and set the maclist to it
	mscan_maclist_fill(sd->bssid_list, (uint8)sd->bssid_list_count);

	// bssidPtr is passed to scan engine, always set to anyBSSID
	bssidPtr = &anyBSSID;

	// Copy specified ssid info into the local wlc_ssid_t struct list
	for (i = 0; i < (int)sd->ssid_list_count; i++) {
		struct apple80211_ssid_data *currssid = &sd->ssid_list[i];

		if (currssid->ssid_len > 0) {
			ssidlist[nssid].SSID_len = MIN(currssid->ssid_len, DOT11_MAX_SSID_LEN);
			memcpy(ssidlist[nssid].SSID, currssid->ssid_bytes, ssidlist[nssid].SSID_len);
		}
		nssid++;
	}
	// Make sure if no ssid's were specified, we force set nssid to 1
	if (nssid == 0) {
		nssid = 1;
	}

	// If this is a fast scan then just return the current scan cache.
	if (sd->scan_type == APPLE80211_SCAN_TYPE_FAST) {
		wlc_bss_list_t bss_list = {};

		memset(&bss_list, 0, sizeof(bss_list));
		if (SCANCACHE_ENAB(pWLC->scan)) {
			wlc_scan_get_cache(pWLC->scan, bssidPtr, nssid, ssidlist, bssType,
			                   chanspec_list, chanspec_count, &bss_list);
		}

		scanComplete(&bss_list);
		return 0;
	}

	bcmerr = wlc_scan_request_ex(	pWLC,
					bssType,				// bsstype
					bssidPtr,
	                                nssid, ssidlist,
					scanType,				// scan_type,
					-1,					// nprobes,
					activeTime,				// active_time,
					passiveTime,				// passive_time,
					homeTime,    				// home_time,
					chanspec_list,				// chanspec_list,
					chanspec_count,				// channel_num,
					0,
					true,
					apple80211_scan_done,	// callback
					this,
					WLC_ACTION_SCAN,
					scanFlags,
					NULL,
					NULL,
					NULL );

	if (bcmerr != BCME_OK) {
		// scan returned error, clear the multiscan state and maclist
		inMultiScan = FALSE;
		mscan_maclist_clear();
	}

	// save the last bcmerr in case queried
	if (bcmerr && VALID_BCMERROR(bcmerr))
		pub->bcmerror = bcmerr;

	return osl_error(bcmerr);
}
#endif /* !MACOSX_DHD */

SInt32
AirPort_Brcm43xx::setASSOCIATE( OSObject * interface, struct apple80211_assoc_data * ad )
{
	SInt32	err = BCME_OK;
	struct apple80211_authtype_data authtype_data = { APPLE80211_VERSION, 0, 0 };
	struct apple80211_rsn_ie_data rid = {};
	struct apple80211_ssid_data sd = {};
	struct apple80211_apmode_data apmode = {};
#ifdef BCMDBG
	char ssidbuf[SSID_FMT_BUF_LEN] = {0};
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
#endif
#ifdef IO80211P2P
	AirPort_Brcm43xxP2PInterface *virtIf = NULL;
#endif
	memset(&rid, 0, sizeof(rid));
	rid.version = APPLE80211_VERSION;
	BCM_REFERENCE(interface);
	ASSERT(ad);
	if (!ad)
		return EINVAL;

	IOVERCHECK(ad->version, "setASSOCIATE");

	KERNEL_DEBUG_CONSTANT(WiFi_apple80211Request_setASSOCIATE | DBG_FUNC_NONE, 0, 0, 0, 0, 0);

#ifdef IO80211P2P
	if( OSDynamicCast( IO80211P2PInterface, interface ) ) {
		virtIf = (AirPort_Brcm43xxP2PInterface *)interface;
		if (ad->ad_mode == APPLE80211_AP_MODE_IBSS)
			return EINVAL;
	}
#endif

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		printf("wl%d: setASSOCIATE\n", unit);
		printf("apple80211_assoc_data:\n");
		printf("  %-18s%s\n", "mode", lookup_name(apple80211_apmode_names, ad->ad_mode));
		printf("  %-18s%s\n", "auth_lower",
		       lookup_name(apple80211_authtype_lower_names, ad->ad_auth_lower));
		printf("  %-18s%s\n", "auth_upper",
		       lookup_name(apple80211_authtype_upper_names, ad->ad_auth_upper));
		printf("  %-18s%s\n", "bssid", bcm_ether_ntoa(&ad->ad_bssid, eabuf));
#ifndef MACOSX_DHD
		wlc_format_ssid(ssidbuf, ad->ad_ssid, ad->ad_ssid_len);
#else
		wl_format_ssid(ssidbuf, ad->ad_ssid, ad->ad_ssid_len);
#endif /* !MACOSX_DHD */
		printf("  %-18s\"%s\"\n", "ssid", ssidbuf);
		printf("key:\n");
		dump_apple80211_key(&ad->ad_key);
		if (ad->ad_rsn_ie_len > 0)
			prhex("rsn ie", ad->ad_rsn_ie, ad->ad_rsn_ie_len);
	}
#endif

	intf_info_blk_t *blk = IFINFOBLKP(interface);

	blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
	blk->cached_assocstatus = APPLE80211_STATUS_UNAVAILABLE;

	blk->deauth_reason = 0;
	blk->link_deauth_reason = 0;
	bzero(&blk->deauth_ea, sizeof(blk->deauth_ea));

#ifndef IO80211P2P
	// Don't turn AP off for a virtual driver
	setAP(FALSE);
#endif

	// disassociate from current network
	err = setDISASSOCIATE( interface );
	if( err ) goto exit;

	// Copy to controller for maclist
	BCOPYADRS(&ad->ad_bssid, &configured_bssid);

	// Set auth mode
	authtype_data.authtype_lower = ad->ad_auth_lower;
	authtype_data.authtype_upper = ad->ad_auth_upper;
	err = setAUTH_TYPE( interface, &authtype_data );
	if( err ) goto exit;
#ifdef MACOSX_DHD
#if defined(MFP) && defined(APPLE80211_ASSOC_F_80211W_MFP)
	if (ad->ad_flags[0] & (1<<APPLE80211_ASSOC_F_80211W_MFP)) {
		err = wlIovarSetInt("mfp", 1);
		if (err) {
			WL_ERROR(("%s: call to set mfp to 1 failed! err:%d\n", __FUNCTION__, err));
			goto exit;
		}
	}
#endif /* MFP */
#endif /* MACOSX_DHD */
	// Set the RSN IE
	if (ad->ad_auth_upper != APPLE80211_AUTHTYPE_NONE && ad->ad_rsn_ie_len) {
		rid.len = ad->ad_rsn_ie_len;
		bcopy(ad->ad_rsn_ie, rid.ie, ad->ad_rsn_ie_len);
	}
	err = setRSN_IE(interface, &rid);
	if( err ) goto exit;
#ifndef MACOSX_DHD
#ifdef MFP
	// TODO: Need to protect this inside OS Version flag, 10_9 does not have this
	// capability
	if (ad->ad_flags[0] & (1<<5) /*APPLE80211_ASSOC_F_80211W_MFP*/) {
		err = wlIovarSetInt("mfp", 1);
		if (err) {
			WL_ERROR(("%s: call to set mfp to 1 failed\n", __FUNCTION__));
			goto exit;
		}
	}
#endif /* MFP */
#endif /* !MACOSX_DHD */
	// clear any existing keys
	err = setCIPHER_KEY(interface, NULL);
	if (err) goto exit;

	// install the provided key
	if (ad->ad_key.key_len) {
		struct apple80211_key * key = &ad->ad_key;

		// The emeded key in 'ad' may not have a version set, assume same as
		// 'ad' version.
		if (!key->version)
			key->version = ad->version;

		err = setCIPHER_KEY(interface, key);
		if (err) goto exit;
	}

	// Set IBSS or infrastructure mode
	apmode.version = APPLE80211_VERSION;
	apmode.apmode = ad->ad_mode;

#ifdef IO80211P2P
	ASSERT(!virtIf || apmode.apmode != APPLE80211_AP_MODE_IBSS);
#endif
	err = setAP_MODE( interface, &apmode );
	if( err ) goto exit;

	// Initiate the association
	memset(&sd, 0, sizeof(sd));
	sd.version = APPLE80211_VERSION;
	sd.ssid_len = ad->ad_ssid_len;
	memcpy( sd.ssid_bytes, ad->ad_ssid, ad->ad_ssid_len );
	err = setSSID( interface, &sd );

#ifdef IO80211P2P
	if (!virtIf) {
#endif
		sc17_retry_cnt = 0;
		bcopy(&sd, &sc17_data, sizeof(struct apple80211_ssid_data));
#ifdef IO80211P2P
	}
#endif

exit:
	return err;

}

uint
AirPort_Brcm43xx::wl_calc_wsec(OSObject * interface)
{
	uint wsec = 0;
	BCM_REFERENCE(interface);
#ifndef MACOSX_DHD
	int bcmerr = 0;
	wlc_bsscfg_t *bsscfg = wl_find_bsscfg(wlLookupWLCIf( interface ));

	ASSERT(bsscfg != NULL);

	// Get existing wsec configuration to avoid trampling it
	bcmerr = wlIovarGetInt("wsec", (int*)&wsec);
	ASSERT(bcmerr == 0);

	wsec &= ~(AES_ENABLED | TKIP_ENABLED | WEP_ENABLED);

	if (IFINFOBLKP(interface)->authtype_upper != APPLE80211_AUTHTYPE_NONE) {
		// Find the correct WSEC value from the WPA/RSN ie
		if (bsscfg->assoc->ie && bsscfg->assoc->ie_len) {
			wsec |= wl_calc_wsec_from_rsn((bcm_tlv_t*)bsscfg->assoc->ie);
			WL_IOCTL(("wl%d: wl_calc_wsec: calculated wsec %u from RSN IE\n",
			          unit, wsec));
		} else if (IFINFOBLKP(interface)->authtype_upper == APPLE80211_AUTHTYPE_WPS) {
			wsec |= WEP_ENABLED;
		}
	} else if (bsscfg != NULL) {
		wlc_key_id_t key_id;
		wlc_key_info_t key_info;

		for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
			wlc_key_t *key;
			key = wlc_keymgmt_get_bss_key(pWLC->keymgmt, bsscfg, key_id, &key_info);
			if (key_info.algo != CRYPTO_ALGO_OFF)
				break;
		}

		switch (key_info.algo) {
		case CRYPTO_ALGO_WEP1:
		case CRYPTO_ALGO_WEP128:
			wsec |= WEP_ENABLED;
			break;
		case CRYPTO_ALGO_TKIP:
			wsec |= TKIP_ENABLED;
			break;
		case CRYPTO_ALGO_AES_CCM:
			wsec |= AES_ENABLED;
			break;
		default:
			break;
		}
	}
	WL_IOCTL(("wl%d: wl_calc_wsec: wsec %u from configured key\n", unit, wsec));
#endif /* !MACOSX_DHD */
	return wsec;
}

uint
AirPort_Brcm43xx::wl_calc_wsec_from_rsn(bcm_tlv_t *rsn_ie)
{
	int err = BCME_OK;
	wpa_suite_t *mc_suite = NULL;
	wpa_suite_t *uc_suite = NULL;
	uint uc_count = 0;
	uint mc_wsec = 0;
	uint uc_wsec = 0;
	uint16 cipher = 0;

	err = wl_parse_wpa_ciphers(rsn_ie, &mc_suite, &uc_count, &uc_suite);
	if (err)
		return 0;

	if (rsn_ie->id == DOT11_MNG_RSN_ID) {
		if (!mc_suite)
			mc_wsec = AES_ENABLED;
		else if (wpa2_cipher(mc_suite, &cipher, TRUE))
			bcmwpa_cipher2wsec((uint8*)mc_suite, &mc_wsec);

		if (!uc_suite)
			uc_wsec = AES_ENABLED;
		else if (wpa2_cipher(uc_suite, &cipher, TRUE))
			bcmwpa_cipher2wsec((uint8*)uc_suite, &uc_wsec);
	} else {
		if (!mc_suite)
			mc_wsec = TKIP_ENABLED;
		else if (wpa_cipher(mc_suite, &cipher, TRUE))
			bcmwpa_cipher2wsec((uint8*)mc_suite, &mc_wsec);

		if (!uc_suite)
			uc_wsec = TKIP_ENABLED;
		else if (wpa_cipher(uc_suite, &cipher, TRUE))
			bcmwpa_cipher2wsec((uint8*)uc_suite, &uc_wsec);
	}

	return (uc_wsec | mc_wsec);
}

int
AirPort_Brcm43xx::wl_parse_wpa_ciphers(bcm_tlv_t *rsn_ie,
	wpa_suite_t **mc_suite, uint *uc_count, wpa_suite_t **uc_suite)
{
	uint8* rsn_body = NULL;
	uint rsn_body_len = 0;

	if(mc_suite==NULL || uc_suite==NULL || uc_count==NULL)
		return 0;

	*mc_suite = NULL;
	*uc_suite = NULL;
	*uc_count = 0;

	if (rsn_ie->id == DOT11_MNG_RSN_ID) {
		wpa_rsn_ie_fixed_t *rsn = NULL;
		rsn = (wpa_rsn_ie_fixed_t*)rsn_ie;
		if (rsn->length < WPA_RSN_IE_TAG_FIXED_LEN ||
		    (ltoh16_ua(&rsn->version) != WPA2_VERSION)) {
			WL_ERROR(("wl%d: unsupported WPA2 version %d\n", unit,
			ltoh16_ua(&rsn->version)));
			return -1;
		}

		if (rsn->length > WPA_RSN_IE_TAG_FIXED_LEN) {
			rsn_body = (uint8*)rsn + WPA_RSN_IE_FIXED_LEN;
			rsn_body_len = (uint)(rsn->length - WPA_RSN_IE_FIXED_LEN);
		}
	} else if (rsn_ie->id == DOT11_MNG_PROPR_ID) {
		wpa_ie_fixed_t *wpa = NULL;
		wpa = (wpa_ie_fixed_t*)rsn_ie;
		if (wpa->length < WPA_IE_TAG_FIXED_LEN) {
			WL_ERROR(("wl%d: WPA IE len %d too short (need %d)\n",
			          unit, (uint)wpa->length, (uint)WPA_IE_TAG_FIXED_LEN));
			return -1;
		}
		if (bcmp(wpa->oui, WPA_OUI, WPA_OUI_LEN)||(wpa->oui_type!=0x01)) {
			WL_ERROR(("wl%d: WPA OUI[%02X:%02X:%02X]/Type[%02X] match failed\n",
			          unit,
			          wpa->oui[0], wpa->oui[1], wpa->oui[2], wpa->oui_type));
			return -1;
		}
		if ((ltoh16_ua(&wpa->version) != WPA_VERSION)) {
			WL_ERROR(("wl%d: unsupported WPA version %d\n", unit,
			ltoh16_ua(&wpa->version)));
			return -1;
		}

		if (wpa->length > WPA_IE_FIXED_LEN) {
			rsn_body = (uint8*)wpa + WPA_IE_FIXED_LEN;
			rsn_body_len = (uint)(wpa->length - WPA_RSN_IE_FIXED_LEN);
		}
	} else {
		WL_ERROR(("wl%d: unsupported RSN IE\n", unit));
		return -1;
	}

	// check for a multicast suite
	if (rsn_body_len < WPA_SUITE_LEN)
		return 0;

	*mc_suite = (wpa_suite_t*)rsn_body;

	// advance past the multicast suite
	rsn_body += WPA_SUITE_LEN;
	rsn_body_len -= WPA_SUITE_LEN;

	// check for a unicastcast suite list (uint16 count and array of suites)
	if (rsn_body_len < sizeof(uint16) + WPA_SUITE_LEN)
		return 0;

	*uc_count = ltoh16_ua(rsn_body);
	if (*uc_count == 0)
		return 0;

	rsn_body += sizeof(uint16);
	rsn_body_len -= sizeof(uint16);

	*uc_suite = (wpa_suite_t*)rsn_body;

	if (rsn_body_len < (*uc_count * WPA_SUITE_LEN)) {
		*uc_suite = NULL;
		*uc_count = 0;
	}

	return 0;
}
SInt32
AirPort_Brcm43xx::setPOWER( OSObject * interface, struct apple80211_power_data * pd )
{
#ifndef MACOSX_DHD
	int bcmerr = 0;
	uint power_on_mask = 0;
	uint new_txchain = 0, new_rxchain = 0;
	uint constraint;
	IOReturn iorc = kIOReturnSuccess;

	BCM_REFERENCE(interface);
	ASSERT(pd);
	if (!pd)
		return EINVAL;

	IOVERCHECK(pd->version, "setPOWER");

	WL_IOCTL(("wl%d: setPOWER:\n", unit));
	for (uint i = 0; i < pd->num_radios; i++) {
		WL_IOCTL(("\tpower_state[%d]: %s\n", i,
		          lookup_name(apple80211_power_state_names, pd->power_state[i])));
	}

	if (pd->num_radios != num_radios) {
		WL_ERROR(("wl%d: setPOWER: num_radios specifed = %u, but device has %u\n",
		          unit, pd->num_radios, num_radios));
		return EINVAL;
	}

	// Check the power_state for correct off/on/tx/rx values
	// Take note of the new rxchain txchain masks
	new_txchain = new_rxchain = 0;
	for (uint i = 0; i < num_radios; i++) {
		if (pd->power_state[i] > APPLE80211_POWER_RX) {
			WL_ERROR(("wl%d: setPOWER: invalid power_state[%u] %u\n",
			          unit, i, (uint)pd->power_state[i]));
			return EINVAL;
		}
		// map ON, RX, TX, OFF to tx/rx chain bitmaps
		switch (pd->power_state[i]) {
		case APPLE80211_POWER_ON:
			new_txchain |= (1 << i);
			new_rxchain |= (1 << i);
			break;
		case APPLE80211_POWER_TX:
			new_txchain |= (1 << i);
			break;
		case APPLE80211_POWER_RX:
			new_rxchain |= (1 << i);
			break;
		case APPLE80211_POWER_OFF:
		default:
			break;
		}
	}

	// power_on_mask has a bit set for any chain that is on for tx or rx
	power_on_mask = new_txchain | new_rxchain;

#ifdef AAPLPWROFF
	WL_IOCTL((" powerstate %ld\n", _powerState));
#endif

	// bring the driver up if any RF chain is on
	if (power_on_mask != 0) {
#ifdef AAPLPWROFF

		// If in the process of powering on, return an error so that IO80211Controller::enable() doesn't post
		// a redundant power changed event when it requests the interface's radios be powered on.
		if( _powerSleep )
			return EBUSY;

		if( _powerState != PS_INDEX_ON || ( _lastUserRequestedPowerState != PS_INDEX_ON ) ) {
			kprintf( "AirPort_Brcm43xx::setPOWER(ON): _powerState[%ld] _lastUserRequestedPowerState[%ld], calling changePowerStateToPriv( PS_INDEX_ON )\n",
				_powerState, _lastUserRequestedPowerState );

			if( (iorc = changePowerStateToPriv( PS_INDEX_ON )) != kIOReturnSuccess ) {
				kprintf( "AirPort_Brcm43xx::setPOWER(ON): changePowerStateToPriv( PS_INDEX_ON ) failed, rc[0x%08x]\n",
					iorc );

				return ENXIO;
			}

			_lastUserRequestedPowerState = PS_INDEX_ON;

			// When a power change is requested using changePowerStateTo, the driver needs to
			// release the workloop lock and sleep this thread until a setPowerState request is
			// received. setPowerState will take the workloop lock, handle the request, and wake
			// this thread. The workloop lock is used to guarantee that the setPowerState call,
			// which is not made on this thread, cannot issue a wake request before this thread sleeps.
			// If that were to happen, this thread would block forever.

			_powerSleep = true;
			_up = true;
			_down = false;
			WL_UNLOCK();
			getCommandGate()->commandSleep( this, THREAD_UNINT );
			WL_LOCK();
			_powerSleep = false;
			pub->hw_off = false;
#ifdef APPLE80211_IOC_LINK_CHANGED_EVENT_DATA
			intf_info_blk_t *blk = IFINFOBLKP(interface);

	                if (blk) {
				blk->isLinkDown = TRUE;
				blk->link_changed_reason.link_down_reason.reason = 0;
			}
#endif
		}
#else
		_up = true;
		_down = false;
		// Enable radio to match RF chain
		arg = (WL_RADIO_SW_DISABLE << 16);
		(void) wlIoctl(WLC_SET_RADIO, &arg, sizeof(arg), TRUE, NULL);
#endif /* AAPLPWROFF */
	} else {

#ifdef AAPLPWROFF
		kprintf( "AirPort_Brcm43xx::setPOWER(OFF): _powerState[%ld] _lastUserRequestedPowerState[%ld] _systemSleeping[%d] _powerOffInProgress[%d] _powerOffThreadRequest[%d] selfpid[%d] inprogress[%d]\n",
			_powerState, _lastUserRequestedPowerState, _systemSleeping,
			 _powerOffInProgress, _powerOffThreadRequest,
			 proc_selfpid(), myNetworkIF->SIOCSIFFLAGS_InProgress );

		// If a transition to PS_INDEX_OFF/PS_INDEX_DOZE is already in progress, don't make a redundant
		// changePowerStateTo( DOZE ) request. This will happen when the system goes
		// to sleep and disable requests power off. If the system is on the way down to sleep,
		// but we have not received a powerStateWillChangeTo( OFF ) callback to tell us a power off
		// request is in progress, don't call changePowerStateToPriv( DOZE ) (<rdar://problem/6259758>).
		// If a disable request comes in, but it is not in the context of handling SIOCSIFFLAGS, ignore
		// it to avoid blocking the power management thread and changing the driver's requested power state
		// from on to off. This can happen when the device is powered off, and then quickly back on (<rdar://problem/6394678>).
		if( _powerOffInProgress ||
		    ( _powerState != PS_INDEX_ON && ( _lastUserRequestedPowerState != PS_INDEX_ON ) ) ||
		    _systemSleeping ||
		    ( proc_selfpid() == 0 && myNetworkIF->SIOCSIFFLAGS_InProgress == false && !_powerOffThreadRequest ) )
		{
			WL_ERROR(( "AirPort_Brcm43xx::setPOWER(OFF): Transition in progress ..."
				"_powerState[%ld] _lastUserRequestedPowerState[%ld] _systemSleeping[%d]"
				" _powerOffInProgress[%d] _powerOffThreadRequest[%d] selfpid[%d] inprogress[%d]\n",
				_powerState, _lastUserRequestedPowerState, _systemSleeping,
				_powerOffInProgress, _powerOffThreadRequest,
				proc_selfpid(), myNetworkIF->SIOCSIFFLAGS_InProgress ));
			kprintf( "AirPort_Brcm43xx::setPOWER(OFF): Transistion in progress, not calling changePowerStateToPriv()\n" );
			return 0;
		}
#endif

#ifdef SUPPORT_FEATURE_WOW
		int wowl_test_mode = 0;
		int wowl_user = 0; // User enabled WOWL

		(void) wlIovarGetInt( "wowl_test", &wowl_test_mode );
		(void) wlIovarGetInt( "wowl", &wowl_user );
#endif /* SUPPORT_FEATURE_WOW */
		pWLC->down_override = 1;
#ifdef SUPPORT_FEATURE_WOW
		/* If in Test mode, just enable WOWL regardless of other conditions */
		if ( (wowl_test_mode  || wowl_user) &&
		     wlc_wowl_enable( pWLC->wowl ) ) {
			/* Enable PME in the pci config */
			if (BUSTYPE(revinfo.bus) == PCI_BUS)
				si_pci_pmeen(pub->sih);
		} else {
			if (!pub->hw_off)
				si_pcie_prep_D3(pub->sih, TRUE);
			wl_down((struct wl_info *)this);
		}
#else
		wl_down((struct wl_info *)this);
#endif /* SUPPORT_FEATURE_WOW */

		// Return to default client STA mode
		setAP(FALSE);
		setIBSS(FALSE);

		pub->hw_up = false;

#ifdef AAPLPWROFF
		kprintf( "AirPort_Brcm43xx::setPOWER(OFF): calling changePowerStateToPriv( PS_INDEX_DOZE )\n" );
		if( (iorc = changePowerStateToPriv( PS_INDEX_DOZE )) != kIOReturnSuccess ) {
			kprintf( "AirPort_Brcm43xx::setPOWER(OFF): changePowerStateToPriv( PS_INDEX_DOZE ) failed, rc[0x%08x]\n",
				iorc );
			return ENXIO;
		}

		_lastUserRequestedPowerState = PS_INDEX_DOZE;

		// Reset flag before giving up the workloop lock and sleeping
		_powerOffThreadRequest = false;

		// See comment on workloop lock above
		_powerSleep = true;
		_up = false;
		_down = true;
		WL_UNLOCK();
		// WAR to avoid the thread getting stuck here
		if (getNetworkInterface()->enabledBySystem()) {
			getCommandGate()->commandSleep( this, THREAD_UNINT );
		}
		WL_LOCK();
		_powerSleep = false;
		pub->hw_off = true;
#ifdef APPLE80211_IOC_LINK_CHANGED_EVENT_DATA
		intf_info_blk_t *blk = IFINFOBLKP(interface);

		if (blk) {
			blk->isLinkDown = TRUE;
			blk->link_changed_reason.link_down_reason.reason = 0;
		}
#endif
		// Apple <rdar://problem/6181254> Always put link down when powering off.
		// Need to use reason code 8 as this was forced by the system and doesn't get recorded as unintended link drop.
		WL_UNLOCK();
		setLinkState( kIO80211NetworkLinkDown , DOT11_RC_DISASSOC_LEAVING);
		WL_LOCK();
#else
		_up = false;
		_down = true;
		arg = (WL_RADIO_SW_DISABLE << 16 | WL_RADIO_SW_DISABLE);
		(void) wlIoctl(WLC_SET_RADIO, &arg, sizeof(arg), TRUE, NULL);
#endif /* AAPLPWROFF */

	}

	if (num_radios == 1)
		return 0;

	// if we are not turning the hardware off, set the RF chains as specified
	if (pd->num_radios > 1 && power_on_mask != 0) {
		uint rxchain = 0, txchain = 0;
		int ioval = 0;

		WL_IOCTL(("wl%d: setPOWER: setting RF chain mask rx 0x%x tx 0x%x\n",
		          unit, new_rxchain, new_txchain));

		/* Get the current active rxchains */
		bcmerr = wlIovarGetInt("rxchain", &ioval);
		rxchain = (uint)ioval;

		if (bcmerr) {
			WL_ERROR(("wl%d: setPOWER: error %d from iovar get \"rxchain\", skipping config\n",
			          unit, bcmerr));
		}

		/* Only set rxchain if new mask is different from active rxchain */
		if (!bcmerr && new_rxchain != rxchain) {
			WL_IOCTL(("wl%d: setPOWER: setting RF RX chain mask 0x%x\n",
			          unit, new_rxchain));
			bcmerr = wlIovarSetInt("rxchain", new_rxchain);
			if (bcmerr)
			    WL_ERROR(("wl%d: setPOWER: err setting rxchain = 0x%x err = %d \"%s\"\n",
			              unit, new_rxchain, bcmerr, bcmerrorstr(bcmerr)));
		}

		/* Get the current active txchains */
		bcmerr = wlIovarGetInt("txchain", &ioval);
		txchain = (uint)ioval;

		if (bcmerr) {
			WL_ERROR(("wl%d: setPOWER: error %d from iovar get \"txchain\", skipping config\n",
			          unit, bcmerr));
		}

		/* Only set txchain if power_on_mask different from active txchain */
		if (!bcmerr && new_txchain != txchain) {
			WL_IOCTL(("wl%d: setPOWER: setting RF TX chain mask 0x%x\n",
			          unit, power_on_mask));
			bcmerr = wlIovarSetInt("txchain", new_txchain);
			if (new_txchain != 0) {
				constraint = wlc_tpc_get_local_constraint_qdbm(pWLC->tpc);
				wlc_channel_set_txpower_limit(pWLC->cmi, constraint);
			}
			if (bcmerr)
			    WL_ERROR(("wl%d: setPOWER: err setting txchain = 0x%x err = %d \"%s\"\n",
			              unit, new_txchain, bcmerr, bcmerrorstr(bcmerr)));
		}
	}

	return osl_error(bcmerr);
#else
	return BCME_OK;
#endif /* !MACOSX_DHD */
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::setIBSS_MODE( OSObject * interface, struct apple80211_network_data * nd)
{
	struct apple80211_authtype_data authtype_data = { APPLE80211_VERSION, 0, 0 };
	struct apple80211_ssid_data sd = {};
	SInt32 err = BCME_OK;
	int bcmerr = 0;
	char country[APPLE80211_MAX_CC_LEN] = {0};
	BCM_REFERENCE(interface);

	// NULL nd indicates IBSS mode should be disabled
	if (!nd) {
		WL_IOCTL(("wl%d: setIBSS_MODE: nd == NULL, disabling IBSS\n", unit));
		setIBSS(FALSE);
#if defined(MFP) && defined(APPLE80211_ASSOC_F_80211W_MFP)
		/* Enable MFP since we're exiting IBSS Mode*/
		err = wlIovarSetInt("mfp", 1);
		if (err) {
			WL_ERROR(("%s: Failed to enable MFP when exiting IBSS mode, err[%d]\n", __FUNCTION__, err));
		}
#endif /* MFP && defined(APPLE80211_ASSOC_F_80211W_MFP) */
		return 0;
	}

#if VERSION_MAJOR > 10
	if (is_channel_supported(nd->nd_channel.channel, &country[0]) == BCME_ERROR) {
		WL_ERROR(("%s: country %s channel %d is not supported\n", __FUNCTION__,
					country, nd->nd_channel.channel));
		return EINVAL;
	}
#endif /* VERSION_MAJOR > 10 */

	IOVERCHECK(nd->version, "setIBSS_MODE");

#if defined(IO80211P2P)
	if( OSDynamicCast( IO80211P2PInterface, interface ) ) {
		return ENXIO;
	}
#endif

	WL_IOCTL(("wl%d: setIBSS_MODE: enabling IBSS mode with following info:\n", unit));
#ifdef BCMDBG
	if (WL_IOCTL_ON())
		dump_apple80211_network_data(nd);
#endif

	if (nd->nd_mode != APPLE80211_AP_MODE_IBSS) {
		WL_ERROR(("wl%d: setIBSS_MODE: nd_mode %u was not APPLE80211_AP_MODE_IBSS %u\n",
		          unit, (uint)nd->nd_mode, APPLE80211_AP_MODE_IBSS));
		return EINVAL;
	}

	if (nd->nd_auth_lower != APPLE80211_AUTHTYPE_OPEN) {
		WL_ERROR(("wl%d: setIBSS_MODE: nd_auth_lower %u, was not APPLE80211_AUTHTYPE_OPEN %u, "
		          "ignoring nd_auth_lower.\n",
		          unit, nd->nd_auth_lower, APPLE80211_AUTHTYPE_OPEN));
		nd->nd_auth_lower = APPLE80211_AUTHTYPE_OPEN;
	}

	// Apple change: WPA2-PSK IBSS support
	if ( (nd->nd_auth_upper != APPLE80211_AUTHTYPE_NONE && nd->nd_auth_upper != APPLE80211_AUTHTYPE_WPA2_PSK) ||
	    nd->nd_auth_lower != APPLE80211_AUTHTYPE_OPEN) {
		WL_ERROR(("wl%d: setIBSS_MODE: nd_auth_upper %u lower %u, was not "
		          "APPLE80211_AUTHTYPE_NONE/APPLE80211_AUTHTYPE_WPA2_PSK %u/%u, APPLE80211_AUTHTYPE_OPEN %u\n",
		          unit, nd->nd_auth_upper, nd->nd_auth_lower,
		          APPLE80211_AUTHTYPE_NONE, APPLE80211_AUTHTYPE_WPA2_PSK, APPLE80211_AUTHTYPE_OPEN));
		return EINVAL;
	}

	// Bring the card down and back up
	wl_down( (struct wl_info *)this );

#if defined(MFP) && defined(APPLE80211_ASSOC_F_80211W_MFP)
	/* Disable MFP if enabled - we dont want it to interfere in IBSS Mode */
	/* Needs to be set in the "wl down" mode */
	err = wlIovarSetInt("mfp", 0);
	if (err) {
		WL_ERROR(("%s: Failed to disable MFP in IBSS mode, err[%d]\n", __FUNCTION__, err));
	}
#endif /* MFP */

	setIBSS(TRUE);

	// RDR:4614798 Need to set specific channel for factory diagnostics
	// Disable spectrum managemet when we set up an AP/IBSS on a 5GHz channel
	if (nd->nd_channel.flags & APPLE80211_C_FLAG_5GHZ) {
		bcmerr = wlIoctlSetInt(WLC_SET_SPECT_MANAGMENT, SPECT_MNGMT_OFF);
		if (bcmerr) {
			WL_ERROR(("wl%d: setIBSS_MODE: err setting ioctl "
			          "WLC_SET_SPECT_MANAGMENT to OFF (%d), err %d \"%s\"\n",
			          unit, SPECT_MNGMT_OFF, bcmerr, bcmerrorstr(bcmerr)));
		}
	}
	wl_up( (struct wl_info *)this );

	// Set auth mode
	authtype_data.authtype_lower = nd->nd_auth_lower;
	authtype_data.authtype_upper = nd->nd_auth_upper;
	err = setAUTH_TYPE(interface, &authtype_data);
	if (err)
		return EINVAL;

	// clear any existing keys
	err = setCIPHER_KEY(interface, NULL);
	if (err)
		return EINVAL;

	// install the provided key
	if (nd->nd_key.key_len && nd->nd_key.key_cipher_type != APPLE80211_CIPHER_PMK) {
		struct apple80211_key * key = &nd->nd_key;

		// The emeded key in 'nd' may not have a version set, assume same as
		// 'nd' version.
		if (!key->version)
			key->version = nd->version;

		// The embeded key may not have the flags set, assume tx key
		key->key_flags |= APPLE80211_KEY_FLAG_TX;

		err = setCIPHER_KEY(interface, key);
		if (err) {
			WL_ERROR(("wl%d: setIBSS_MODE: err %d from setCIPHER_KEY "
			          "cipher:0x%x len:%d\n",
			          unit, (int)err,
			          (uint)key->key_cipher_type, (int)key->key_len));
			return err;
		}
	}

	err = wlIoctlSetInt(WLC_SET_CHANNEL, nd->nd_channel.channel );

	if (err) {
		WL_ERROR(("%s: err %d from WLC_SET_CHANNEL\n", __FUNCTION__, err));
		return err;
	}

	// Create/Join the IBSS
	sd.version = APPLE80211_VERSION;
	sd.ssid_len = nd->nd_ssid_len;
	memcpy(sd.ssid_bytes, nd->nd_ssid, MIN(sd.ssid_len, sizeof(sd.ssid_bytes)));
	err = setSSID( interface, &sd );

	// Apple change: WPA2-PSK IBSS support
	if( IFINFOBLKP(interface)->authtype_upper == APPLE80211_AUTHTYPE_WPA2_PSK )
	{
		int bcmerror = 0;
		int wsec = 0;

		bcmerror = wlIovarSetInt( "auth", DOT11_OPEN_SYSTEM );
		if( bcmerror )
			return osl_error( bcmerror );

		bcmerror = wlIovarSetInt( "wpa_cap", 0 );
		if( bcmerror )
			return osl_error( bcmerror );

		// Get existing wsec configuration to avoid trampling it
		bcmerror = wlIovarGetInt("wsec", &wsec);
		if( bcmerror )
			return osl_error( bcmerror );

		wsec &= ~(AES_ENABLED | TKIP_ENABLED | WEP_ENABLED);
		wsec |= AES_ENABLED;

		bcmerror = wlIovarSetInt( "wsec", wsec);
		if( bcmerror )
			return osl_error( bcmerror );

		bcmerror = wlIovarSetInt( "wpa_auth", WPA2_AUTH_PSK );
		if( bcmerror )
			return osl_error( bcmerror );

		wlc_update_beacon( pWLC );
		wlc_update_probe_resp( pWLC, FALSE );
	}

	// The driver seems to take a while to report that it is in IBSS mode through
	// the pub->associated check, which can hold up hostapd initialization and cause
	// it to miss the IBSS peer arrived event for any stations already participating
	// in the network.
	memcpy( ibssSSID, nd->nd_ssid, nd->nd_ssid_len );
	ibssSSIDLen = (UInt32)nd->nd_ssid_len;
	ibssRunning = true;

	return 0;
}
#endif /* !MACOSX_DHD */

SInt32
AirPort_Brcm43xx::setHOST_AP_MODE( OSObject * interface, struct apple80211_network_data * nd)
{
	struct apple80211_authtype_data authtype_data = { APPLE80211_VERSION, 0, 0 };
	struct apple80211_ssid_data sd = {};
	struct apple80211_channel_data cd = {};
	wlc_ssid_t ssid = {};
	SInt32 err = BCME_OK;
	int bcmerr = 0;
	bool restore_ap, restore_mfp;
	int orig_ap, orig_mfp;
	restore_ap = FALSE;
	restore_mfp = FALSE;

#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char ssidbuf[SSID_FMT_BUF_LEN];
#endif /* BCMDBG || BCMDBG_ERR */
	struct wlc_if * wlcif = NULL;
	char country[APPLE80211_MAX_CC_LEN] = {0};

	// Need to deal with InternetSharing and P2P together
	if (!nd) {
		WL_IOCTL(("wl%d: setHOST_AP_MODE: nd == NULL, clear AP mode\n", unit));
		setAP(FALSE);
#if defined(MFP) && defined(APPLE80211_ASSOC_F_80211W_MFP)
		/* Enable MFP since we're exiting SWAP Mode*/
		err = wlIovarSetInt("mfp", 1);
		if (err) {
			WL_ERROR(("%s: Failed to enable MFP when exiting SWAP mode, err[%d]\n", __FUNCTION__, err));
		}
#endif /* MFP && defined(APPLE80211_ASSOC_F_80211W_MFP) */

		return 0;
	}
#ifndef MACOSX_DHD
#if VERSION_MAJOR > 10
	if (is_channel_supported(nd->nd_channel.channel, &country[0]) == BCME_ERROR) {
		WL_ERROR(("%s: country %s channel %d is not supported\n", __FUNCTION__,
			country, nd->nd_channel.channel));
		return EINVAL;
	}
#endif /* VERSION_MAJOR > 10 */
#endif /* MACOSX_DHD */

	IOVERCHECK(nd->version, "setHOST_AP_MODE");

	WL_IOCTL(("wl%d: setHOST_AP_MODE: enabling AP mode with following info:\n", unit));
#ifdef BCMDBG
	if (WL_IOCTL_ON())
		dump_apple80211_network_data(nd);
#endif

	// Bring the card down and back up
	wl_down( (struct wl_info *)this );

#if defined(MFP) && defined(APPLE80211_ASSOC_F_80211W_MFP)
	/* Disable MFP if enabled - we dont want it to interfere in HOST AP Mode */
	/* Needs to be setin the "wl down" mode */
	err = wlIovarGetInt("mfp", &orig_mfp);
	if (!err) {
		restore_mfp = TRUE;
	}
	err = wlIovarSetInt("mfp", 0);
	if (err) {
		WL_ERROR(("%s: Failed to disable MFP in SWAP mode, err[%d]\n", __FUNCTION__, err));
	}
#endif /* MFP && defined(APPLE80211_ASSOC_F_80211W_MFP) */

	bcmerr = wlIoctlGet(WLC_GET_AP, &orig_ap, sizeof(orig_ap), interface);
	if (!bcmerr) {
		restore_ap = TRUE;
	}
	setAP(TRUE);
	ssid.SSID_len = 0;
	bcmerr = wlIoctl(WLC_SET_SSID, &ssid, sizeof(ssid), TRUE, interface);
	if (bcmerr) {
		WL_ERROR(("wl%d: setHOST_AP_MODE: err from WLC_SET_SSID, %d \"%s\"\n",
		          unit, bcmerr, bcmerrorstr(bcmerr)));
		goto exit_bcmerr;
	}

	// Disable spectrum management and regulatory when we set up an AP to avoid adding Country IE with Aggregate
	// country codes
	bcmerr = wlIoctlSetInt(WLC_SET_SPECT_MANAGMENT, SPECT_MNGMT_OFF);
	if (bcmerr) {
		WL_ERROR(("wl%d: setHOST_AP_MODE: err setting ioctl "
		          "WLC_SET_SPECT_MANAGMENT to OFF (%d), err %d \"%s\"\n",
		          unit, SPECT_MNGMT_OFF, bcmerr, bcmerrorstr(bcmerr)));
		goto exit_bcmerr;
	}

	bcmerr = wlIoctlSetInt(WLC_SET_REGULATORY, 0);
	if (bcmerr) {
		WL_ERROR(("wl%d: setHOST_AP_MODE: err setting ioctl "
		          "WLC_SET_REGULATORY to OFF (%d), err %d \"%s\"\n",
		          unit, 0, bcmerr, bcmerrorstr(bcmerr)));
		goto exit_bcmerr;
	}

	bcmerr = wl_up( (struct wl_info *)this );
	if (bcmerr) {
		WL_ERROR(("wl%d: setHOST_AP_MODE: err from wl_up, %d \"%s\"\n",
		          unit, bcmerr, bcmerrorstr(bcmerr)));
		goto exit_bcmerr;
	}

	// Set auth mode
	authtype_data.authtype_lower = nd->nd_auth_lower;
	authtype_data.authtype_upper = nd->nd_auth_upper;
	err = setAUTH_TYPE(interface, &authtype_data);
	if (err)
		return EINVAL;

	// clear any existing keys
	err = setCIPHER_KEY(interface, NULL);
	if (err)
		return err;

	// install the provided key
	if (nd->nd_key.key_len) {
		struct apple80211_key * key = &nd->nd_key;

		// The emeded key in 'nd' may not have a version set, assume same as
		// 'nd' version.
		if (!key->version)
			key->version = nd->version;

		// The embeded key may not have the flags set, assume tx key
		key->key_flags |= APPLE80211_KEY_FLAG_TX;

		err = setCIPHER_KEY(interface, key);
		if (err) {
			WL_ERROR(("wl%d: setHOST_AP_MODE: err %d from setCIPHER_KEY "
			          "cipher:0x%x len:%d\n",
			          unit, (int)err,
			          (uint)key->key_cipher_type, (int)key->key_len));
			goto exit_bcmerr;
		}
	}

	cd.version = APPLE80211_VERSION;
	cd.channel = nd->nd_channel;
	err = setCHANNEL(interface, &cd);

	if (err) {
		WL_ERROR(("wl%d: setHOST_AP_MODE: err from WLC_SET_CHANNEL ch %d, %d\n",
				unit, nd->nd_channel.channel, (int)err));
			goto exit_bcmerr;
		}

	// Create the BSS
	sd.version = APPLE80211_VERSION;
	sd.ssid_len = nd->nd_ssid_len;
	memcpy(sd.ssid_bytes, nd->nd_ssid, MIN(sd.ssid_len, sizeof(sd.ssid_bytes)));
	err = setSSID( interface, &sd );
	if (err) {
#ifdef MACOSX_DHD
		WL_ERROR(("wl%d: setHOST_AP_MODE: err from setSSID %s, %d\n", unit,
			(wl_format_ssid(ssidbuf, sd.ssid_bytes, sd.ssid_len), ssidbuf),
			(int)err));
#else
		WL_ERROR(("wl%d: setHOST_AP_MODE: err from setSSID %s, %d\n", unit,
			(wlc_format_ssid(ssidbuf, sd.ssid_bytes, sd.ssid_len), ssidbuf),
			(int)err));
#endif
		goto exit_bcmerr;
	}

#ifndef MACOSX_DHD
	if (!wl_find_bsscfg(wlcif)->associated) {
		WL_ERROR(("wl%d: setHOST_AP_MODE: wl_find_bsscfg()->associated not true!\n", unit));
		return ENXIO;
	}
#endif /* MACOSX_DHD */

	return 0;

 exit_bcmerr:
	if (restore_ap == TRUE) {
		err = wlIoctlSetInt(WLC_SET_AP, orig_ap);
		if (err) {
			WL_ERROR(("%s: Failed to restore AP mode, err[%d]\n", __FUNCTION__, err));
		}
	}
#if defined(MFP) && defined(APPLE80211_ASSOC_F_80211W_MFP)
        /* restore mfp */
	if (restore_mfp) {
		err = wlIovarSetInt("mfp", orig_mfp);
		if (err) {
			WL_ERROR(("%s: Failed to restore MFP in SWAP mode, err[%d]\n", __FUNCTION__, err));
		}
	}
#endif /* MFP && defined(APPLE80211_ASSOC_F_80211W_MFP) */

	return osl_error(bcmerr);
}

SInt32
AirPort_Brcm43xx::setCIPHER_KEY( OSObject * interface, struct apple80211_key * key )
{
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
	wlc_bsscfg_t *bsscfg = wl_find_bsscfg(wlcif);
#endif /* !MACOSX_DHD */
	BCM_REFERENCE(interface);
	// Null key indicates that all keys should be cleared
	if (!key) {
		int bcmerr = 0;
		int wsec = 0;

		WL_IOCTL(("wl%d: setCIPHER_KEY: key == NULL, clear keys\n", unit));
#ifndef MACOSX_DHD
		wlc_keymgmt_reset(pWLC->keymgmt, bsscfg, NULL);
#endif /* !MACOSX_DHD */
		bcmerr = wlIovarGetInt("wsec", &wsec);
		if (bcmerr) {
			WL_ERROR(("%s: err from wlIovarGetInt(\"wsec\")\n", __FUNCTION__));
			return osl_error(bcmerr);
		}

		wsec &= ~(AES_ENABLED | TKIP_ENABLED | WEP_ENABLED);
		bcmerr = wlIovarOp("wsec", NULL, 0, &wsec, sizeof(wsec), IOV_SET, interface);
		if (bcmerr) {
#ifndef MACOSX_DHD
			WL_ERROR(("wl%d: setCIPHER_KEY: err from wlIovarOp(\"wsec\", %d, %p), %d \"%s\"\n",
			          unit, wsec, OSL_OBFUSCATE_BUF(wlcif),
			          bcmerr, bcmerrorstr(bcmerr)));
#endif /* !MACOSX_DHD */
		} else {
			WL_IOCTL(("wl%d: setCIPHER_KEY: configured wsec %d\n", unit, wsec));
		}
		return 0;
	}

	IOVERCHECK(key->version, "setCIPHER_KEY");
#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		printf("wl%d: setCIPHER_KEY:\n", unit);
		dump_apple80211_key(key);
	}
#endif

#if VERSION_MAJOR > 9
#ifndef MACOSX_DHD
	// Apple change: WPA2-PSK IBSS support
	if (bsscfg->associated || ibssRunning )
	{
		/*This Flag is used to bypass the flow control check
		* so in wlc_tx_enq 802.1x  priority can be changed
		*/
		skip_flowcontrol = TRUE;
		if( BSSCFG_AP(bsscfg) || ( !bsscfg->BSS || ibssRunning ) )
		{
			if( ( IFINFOBLKP(interface)->authtype_upper & APPLE80211_AUTHTYPE_WPA_PSK ) ||
			    ( IFINFOBLKP(interface)->authtype_upper & APPLE80211_AUTHTYPE_WPA2_PSK ) )
				return setSTA_WPA_Key( key );
		}
	}
#endif /* !MACOSX_DHD */
#endif /* VERSION_MAJOR > 9 */

#if defined(AWDL_FAMILY)
		/* MacOSX requirement: send ROAM_START/END event to OS when roam scan start/end.
			- sent ROAM_END when wsec keys are installed.
		*/
		if (!wlcif && !ETHER_ISNULLADDR(&key->key_ea) &&
			roam_start_set) {
#if defined(APPLE80211_M_ROAM_END)
			struct apple80211_roam_event_data event_data;

			memset(&event_data, 0, sizeof(event_data));
			postMessage(APPLE80211_M_ROAM_END, (void *)&event_data, sizeof(event_data));
#endif
			WL_ASSOC(("%s(): post APPLE80211_M_ROAM_END event.\n", __FUNCTION__));
			roam_start_set = FALSE;
		}
#endif /* AWDL_FAMILY */

	switch( key->key_cipher_type )
	{
	case APPLE80211_CIPHER_NONE:
		break;

	case APPLE80211_CIPHER_WEP_40:
	case APPLE80211_CIPHER_WEP_104:
		if (key->key_len != 5 && key->key_len != 13) {
			WL_ERROR(("wl%d: setCIPHER_KEY: cipher type WEP (%u) "
			          "but len was not 5 or 13 (%u)\n",
			          unit, key->key_cipher_type, key->key_len));
			return EINVAL;
		}

		setCipherKey(key->key, (int)key->key_len, (int)key->key_index,
		             (key->key_flags & APPLE80211_KEY_FLAG_TX) ? true : false );
		break;

	case APPLE80211_CIPHER_TKIP: // For WPA1/2 EAP only
	case APPLE80211_CIPHER_AES_CCM:
		if( key->key_len ) {
			SetCryptoKey( key->key,
			              key->key_len,
			              key->key_index,
			              key->key_rsc_len ? key->key_rsc : NULL,
			              key->key_flags & APPLE80211_KEY_FLAG_TX ? true : false,
			              ETHER_ISNULLADDR(&key->key_ea) ? NULL : &key->key_ea );
		}
#if VERSION_MAJOR > 11
#if defined WOWL_OS_OFFLOADS
		if (wlc_wowl_cap(pWLC) && key->kek_len && key->kck_len) {
			// For MacOS, the offload identifier is a don't care
			// set to '1'


			if (!wlc_wowl_set_key_info(pWLC->wowl, WOWL_GTK_OFFLOADID,
				key->kek,
				key->kek_len,
				key->kck,
				key->kck_len,
				key->key_rsc,
				EAPOL_KEY_REPLAY_LEN)) {
					WL_WOWL(("wl%d: setCIPHER_KEY: GTK Key Info programmed !\n", unit));
				} else {
					WL_ERROR(("wl%d: setCIPHER_KEY: failed to program GTK Key Info !\n", unit));
					return EINVAL;
				}
		}
#endif /* WOWL_OS_OFFLOADS */
#endif /* VERSION_MAJOR > 11 */
		break;

	case APPLE80211_CIPHER_PMK:
		break;

	default:
		WL_ERROR(("wl%d: setCIPHER_KEY: unknown cipher type %d",
		          unit, key->key_cipher_type));
		return EINVAL;
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::setAUTH_TYPE( OSObject * interface, struct apple80211_authtype_data * ad )
{
	int macAuth = DOT11_OPEN_SYSTEM;	// default to OpenSys mac-layer auth
	int wpaAuth = WPA_AUTH_DISABLED;	// default to no upper-layer auth
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
#endif /* !MACOSX_DHD */
	ASSERT(ad);
	if (!ad)
		return EINVAL;

	IOVERCHECK(ad->version, "setAUTH_TYPE");

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		printf("wl%d: setAUTH_TYPE\n", unit);
		printf("apple80211_authtype_data:\n");
		printf("  %-18s%s\n", "auth_lower",
		       lookup_name(apple80211_authtype_lower_names, ad->authtype_lower));
		printf("  %-18s%s\n", "auth_upper",
		       lookup_name(apple80211_authtype_upper_names, ad->authtype_upper));
	}
#endif

	// Select lower level auth
	switch( ad->authtype_lower ) {
		case APPLE80211_AUTHTYPE_OPEN:
			macAuth = DOT11_OPEN_SYSTEM;
			break;

		case APPLE80211_AUTHTYPE_SHARED:
			macAuth = DOT11_SHARED_KEY;
			break;

		case APPLE80211_AUTHTYPE_CISCO:
			macAuth = 0x80;
			break;

		default:
			return EINVAL;
	}

#if VERSION_MAJOR > 9
	// Apple change: hostapd support. authtype_upper is now a bit mask to support
	//				 configuring WPA-PSK and WPA2-PSK at the same time
	UInt32 validUpperAuth = ( APPLE80211_AUTHTYPE_WPA |
	                          APPLE80211_AUTHTYPE_WPA_PSK |
	                          APPLE80211_AUTHTYPE_WPA2 |
	                          APPLE80211_AUTHTYPE_WPA2_PSK |
	                          APPLE80211_AUTHTYPE_LEAP |
	                          APPLE80211_AUTHTYPE_8021X |
	                          APPLE80211_AUTHTYPE_WPS );

	if( ad->authtype_upper & ~validUpperAuth )
		return EINVAL;

	if( ad->authtype_upper == APPLE80211_AUTHTYPE_LEAP )
		macAuth = 0x80;
	else
	{
		if( ad->authtype_upper & APPLE80211_AUTHTYPE_WPA_PSK )
			wpaAuth |= WPA_AUTH_PSK;
		if( ad->authtype_upper & APPLE80211_AUTHTYPE_WPA )
			wpaAuth |= WPA_AUTH_UNSPECIFIED;
		if( ad->authtype_upper & APPLE80211_AUTHTYPE_WPA2_PSK )
			wpaAuth |= WPA2_AUTH_PSK;
		if( ad->authtype_upper & APPLE80211_AUTHTYPE_WPA2 )
			wpaAuth |= WPA2_AUTH_UNSPECIFIED;
	}
#else
	// Select upper level auth
	switch( ad->authtype_upper ) {
		case APPLE80211_AUTHTYPE_NONE:
			break;
		case APPLE80211_AUTHTYPE_WPA_PSK:
			wpaAuth = WPA_AUTH_PSK;
			break;
		case APPLE80211_AUTHTYPE_WPA:
			wpaAuth = WPA_AUTH_UNSPECIFIED;
			break;
		case APPLE80211_AUTHTYPE_WPA2_PSK:
			wpaAuth = WPA2_AUTH_PSK;
			break;
		case APPLE80211_AUTHTYPE_WPA2:
			wpaAuth = WPA2_AUTH_UNSPECIFIED;
			break;
		case APPLE80211_AUTHTYPE_LEAP:
			macAuth = 0x80;
			break;
		case APPLE80211_AUTHTYPE_8021X: // ??
		case APPLE80211_AUTHTYPE_WPS:
			break;
		default:
			return EINVAL;	// nothing else supported
	}
#endif /* VERSION_MAJOR > 9 */

	// Store the apple80211 interface values
	IFINFOBLKP(interface)->authtype_lower = ad->authtype_lower;
	IFINFOBLKP(interface)->authtype_upper = ad->authtype_upper;
#ifndef MACOSX_DHD
	wlc_bsscfg_t *bsscfg = wl_find_bsscfg(wlcif);
	bool isup = bsscfg ? bsscfg->up : FALSE;

	if( isup ) wlc_bsscfg_down( pWLC, bsscfg );
	(void) wlIoctl(WLC_SET_AUTH, &macAuth, sizeof(macAuth), TRUE, interface);	// set mac-layer auth
	(void) wlIoctl(WLC_SET_WPA_AUTH, &wpaAuth, sizeof(wpaAuth), TRUE, interface);	// set upper-layer auth
	if( isup ) wlc_bsscfg_up( pWLC, bsscfg );
#else
	(void) wlIoctl(WLC_SET_AUTH, &macAuth, sizeof(macAuth), TRUE, interface);	// set mac-layer auth
	(void) wlIoctl(WLC_SET_WPA_AUTH, &wpaAuth, sizeof(wpaAuth), TRUE, interface);	// set upper-layer auth
#endif /* !MACOSX_DHD */
	return 0;
}

SInt32
AirPort_Brcm43xx::setDISASSOCIATE( OSObject * interface )
{
	int disassoc = 0;
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
	wlc_bsscfg_t *cfg = wl_find_bsscfg(wlcif);

	WL_IOCTL(("wl%d: setDISASSOCIATE\n", unit));

	if (cfg->associated && !BSSCFG_AP(cfg))
#endif /* !MACOSX_DHD */
		(void) wlIoctl(WLC_DISASSOC, &disassoc, sizeof (disassoc) , FALSE, interface);

#ifndef IO80211P2P
	setIBSS(FALSE);
#endif

	intf_info_blk_t *blk = IFINFOBLKP(interface);

	blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
	blk->cached_assocstatus = APPLE80211_STATUS_UNAVAILABLE;
	blk->deauth_reason = 0;
	blk->link_deauth_reason = 0;
	bzero(&blk->deauth_ea, sizeof(blk->deauth_ea));

	return 0;
}

SInt32
AirPort_Brcm43xx::setSSID( OSObject * interface, struct apple80211_ssid_data * sd )
{
	wlc_ssid_t ssid = {};
	int bcmerr = 0;
	int wsec = 0, ap = 0, isup = 0;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char ssidbuf[SSID_FMT_BUF_LEN] = {0};
#endif /* BCMDBG || BCMDBG_ERR */
	intf_info_blk_t *blk = IFINFOBLKP(interface);

	ASSERT(sd);
	if (!sd)
		return EINVAL;

	IOVERCHECK(sd->version, "setSSID");
#ifndef MACOSX_DHD
	WL_IOCTL(("wl%d: setSSID: \"%s\"\n", unit,
		(wlc_format_ssid(ssidbuf, sd->ssid_bytes, sd->ssid_len), ssidbuf)));
#else
	WL_IOCTL(("wl%d: setSSID: \"%s\"\n", unit,
		(wl_format_ssid(ssidbuf, sd->ssid_bytes, sd->ssid_len), ssidbuf)));
#endif /* !MACOSX_DHD */

	wsec = wl_calc_wsec(interface);
	bcmerr = wlIovarOp("wsec", NULL, 0, &wsec, sizeof(wsec), IOV_SET, interface);
	if (bcmerr) {
		WL_ERROR(("wl%d: setSSID: err from wlIovarOp(\"wsec\", %d, %p), %d \"%s\"\n",
		          unit, wsec, OSL_OBFUSCATE_BUF(interface),
		          bcmerr, bcmerrorstr(bcmerr)));
	} else {
		WL_IOCTL(("wl%d: setSSID: configured wsec %d\n", unit, wsec));
	}

	(void) wlIovarGetInt("ap", &ap);
	if (ap) {
		int wep = 0;
		(void) wlIovarGetInt("wsec", &wep);
		if (WSEC_WEP_ENABLED(wep)) {
#ifndef MACOSX_DHD
			isup = pub->up;
#else
			isup = isUp();
#endif /* !MACOSX_DHD */
			if (isup)
				wl_down((struct wl_info *)this);
			// disable nmode
			(void) wlIovarSetInt("nmode", 0);
			if (isup)
				wl_up((struct wl_info *)this);
		}
	}

	// update the mac access list for the current mode
	wl_maclist_update();
#ifndef MACOSX_DHD
	ssid.SSID_len = sd->ssid_len;
	memcpy( ssid.SSID, sd->ssid_bytes, MIN(ssid.SSID_len, sizeof(ssid.SSID)) );
#ifdef SCAN_JOIN_TIMEOUT
	pWLC->join_timeout = JOIN_FAIL_TIMEOUT; /* set to 8 + 1  seconds */
#endif
	bcmerr = wlIoctl(WLC_SET_SSID, &ssid, sizeof( ssid ), TRUE, interface);

	if (bcmerr) {
		WL_ERROR(("wl%d: setSSID: err from WLC_SET_SSID %s, %d \"%s\"\n", unit,
		          (wlc_format_ssid(ssidbuf, ssid.SSID, ssid.SSID_len), ssidbuf),
		          bcmerr, bcmerrorstr(bcmerr)));
	} else
		blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
#else

	memset(&ssid, 0, sizeof(ssid));
	ssid.SSID_len = MIN(sd->ssid_len, sizeof(ssid.SSID));
	memcpy( ssid.SSID, sd->ssid_bytes, ssid.SSID_len);
	bcmerr = wlIoctl(WLC_SET_SSID, &ssid, sizeof(ssid), TRUE, interface);

	if (bcmerr) {
		WL_ERROR(("wl%d: setSSID: err from WLC_SET_SSID %s, %d \"%s\"\n", unit,
		          (wl_format_ssid(ssidbuf, ssid.SSID, ssid.SSID_len), ssidbuf),
		          bcmerr, bcmerrorstr(bcmerr)));
	} else {
		blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
	}
#endif /* !MACOSX_DHD */
	blk->deauth_reason = 0;
	blk->link_deauth_reason = 0;
	bzero(&blk->deauth_ea, sizeof(blk->deauth_ea));

	return osl_error(bcmerr);
}

// update the mac access list for the current mode
void
AirPort_Brcm43xx::wl_maclist_update()
{
	int err = BCME_OK;
	// Clear mac access list if configured_bssid is zero,
	// otherwise set up mac access list

#ifndef MACOSX_DHD
	if (AP_ENAB(pub) || ETHER_ISNULLADDR(&configured_bssid))
#else
	if (ETHER_ISNULLADDR(&configured_bssid))
#endif /* !MACOSX_DHD */
	{
		err = wlIoctlSetInt(WLC_SET_MACLIST, 0);
		if (err)
			WL_ERROR(("%s: WLC_SET_MACLIST returned err %d\n", __FUNCTION__, err));
		err = wlIoctlSetInt(WLC_SET_MACMODE, 0);
		if (err)
			WL_ERROR(("%s: WLC_SET_MACMODE returned err %d\n", __FUNCTION__, err));
	} else {
		struct maclist bssid_list = {};
		bssid_list.count = 1;
		BCOPYADRS(&configured_bssid, &bssid_list.ea);
		err = wlIoctl(WLC_SET_MACLIST, &bssid_list, sizeof(bssid_list), TRUE, NULL);
		if (err)
			WL_ERROR(("%s: WLC_SET_MACLIST returned err %d\n", __FUNCTION__, err));
		// mode 2: allow only this BSSID
		err = wlIoctlSetInt(WLC_SET_MACMODE, 2);
		if (err)
			WL_ERROR(("%s: WLC_SET_MACMODE returned err %d\n", __FUNCTION__, err));
	}
}

void
AirPort_Brcm43xx::mscan_maclist_fill(struct ether_addr *bssid_list, uint8 num_bssid)
{
	// make sure we don't overflow mscan_bssid_list
	if (num_bssid > APPLE80211_MAX_SCAN_BSSID_CNT) {
		WL_ERROR(("%s: num_bssid %d truncated to %d\n", __FUNCTION__, num_bssid,
		          APPLE80211_MAX_SCAN_BSSID_CNT));
		num_bssid = APPLE80211_MAX_SCAN_BSSID_CNT;
	}

	bcopy(bssid_list, mscan_bssid_list, ETHER_ADDR_LEN * num_bssid);
	num_mscan_bssid = num_bssid;
}

void
AirPort_Brcm43xx::mscan_maclist_clear()
{
	bzero(mscan_bssid_list, ETHER_ADDR_LEN * num_mscan_bssid);
	num_mscan_bssid = 0;
}

bool
AirPort_Brcm43xx::mscan_maclist_filter(struct ether_addr *bssid)
{
	uint8 i = 0;

	if (num_mscan_bssid == 0) {
		// nothing in mscan maclist, don't filter
		return FALSE;
	}

	for (i = 0; i < num_mscan_bssid; i++) {
		if (BCMPADRS(bssid, &mscan_bssid_list[i]) == 0) {
			// bssid matched entry in list, don't filter
			return FALSE;
		}
	}

	// not found in list, filter
	return TRUE;
}
#ifdef MACOSX_DHD
bool
AirPort_Brcm43xx::scanresult_list_filter(struct ether_addr *bssid)
{
	scan_result_t *pl;

	if (SLIST_EMPTY(&scan_result_list)) {
		// nothing in list, don't filter
		return FALSE;
	}

	SLIST_FOREACH(pl, &scan_result_list, entries) {
		if (BCMPADRS(bssid, pl->asr.asr_bssid) == 0) {
			// bssid matched entry in list, filter
			return TRUE;
		}
	}

	// not found in list, don't filter
	return FALSE;
}
#endif /* MACOSX_DHD */
SInt32
AirPort_Brcm43xx::setAP_MODE( OSObject * interface, struct apple80211_apmode_data * ad )
{
	int isInfra = 0, err = BCME_OK;
	BCM_REFERENCE(interface);
#ifdef IO80211P2P
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
#endif

	ASSERT(ad);
	if (!ad)
		return EINVAL;

	IOVERCHECK(ad->version, "setAP_MODE");

	switch( ad->apmode )
	{
		case APPLE80211_AP_MODE_IBSS:
#ifdef IO80211P2P
			// wlcif is non-null for virtual interface
			if (wlcif)
				return EINVAL;
#endif
			isInfra = 0;
			break;
		case APPLE80211_AP_MODE_INFRA:
			isInfra = 1;
			break;
		default:
			return EINVAL;
	}

	err = wlIoctlSetInt(WLC_SET_INFRA, isInfra );

	return !err ? 0 : ENXIO;
}

#define X0_REGREV_IDX	0
#define X1_REGREV_IDX	1
#define X2_REGREV_IDX	2
#define X3_REGREV_IDX	3
#define JP_REGREV_IDX	4
#define KR_REGREV_IDX	5
#define NUM_REGREV_IDX	6

#define MAXNUM_MODULE_ENTRY	(sizeof(module_regrev)/sizeof(module_regrev[0]))

const struct {
	uint16	boardtype;
	uint8	regrev[NUM_REGREV_IDX];
} module_regrev[] = {
	{BCM943224X21_FCC, {7, 6, 6, 6, 12, 10}},
	{BCM943224X21B, {7, 6, 6, 6, 12, 10}},
	{BCM94331X19, {8, 7, 7, 7, 13, 11}},
	{BCM94331X28, {9, 8, 8, 8, 18, 17}},
	{BCM94331X28B, {13, 12, 12, 12, 28, 31}},
	{BCM94331X29B, {10, 9, 9, 9, 20, 18}},
	{BCM94331X29D, {14, 13, 13, 13, 20, 18}},
	{BCM94331X19C, {11, 10, 10, 10, 23, 20}},
	{BCM94331X33, {12, 11, 11, 11, 26, 23}},
	{BCM94360X29C, {15, 0, 14, 14, 0, 0}},
	{BCM94360X29CP2, {15, 0, 14, 14, 0, 0}},
	{BCM94360X29CP3, {15, 0, 14, 14, 0, 0}},
	{BCM94360X51, {16, 0, 15, 15, 0, 0}},
	{BCM94360X51P2, {19, 0, 19, 19, 0, 0}},
	{BCM94360X51P3, {23, 0, 23, 23, 0, 0}},
	{BCM94360X51A, {19, 0, 19, 19, 0, 0}},
	{BCM94360X51B, {16, 0, 15, 15, 0, 0}},
	{BCM94360X52C, {18, 0, 17, 17, 0, 0}},
	{BCM94360X52D, {18, 0, 17, 17, 0, 0}},
	{BCM94350X52B, {17, 0, 16, 16, 0, 0}},
	{BCM943602X87, {204, 0, 204, 204, 0, 0}},
	{BCM943602X87P2, {204, 0, 204, 204, 0, 0}},
	{BCM943602X87P3, {204, 0, 204, 204, 0, 0}},
	{BCM943602X238, {21, 0, 21, 21, 0, 0}},
	{BCM943602X238D, {242, 0, 242, 242, 0, 0}},
	{BCM943602X238DP2, {242, 0, 242, 242, 0, 0}},
	{BCM94350X14, {219, 0, 219, 219, 0, 0}},
	{BCM94350X14P2, {219, 0, 219, 219, 0, 0}}
};

int
AirPort_Brcm43xx::get_regrev(int idx)
{
	uint i = 0;

	ASSERT(idx < NUM_REGREV_IDX);

	for (i = 0; i < MAXNUM_MODULE_ENTRY; i++) {
		if (module_regrev[i].boardtype == revinfo.boardid)
			return module_regrev[i].regrev[idx];
	}
	return -1;
}

SInt32
AirPort_Brcm43xx::setLOCALE( OSObject * interface, struct apple80211_locale_data * ld )
{
	int err = BCME_OK;
	const char *country_code = NULL;
	wl_country_t cspec = {{0}, -1, {0}};

	BCM_REFERENCE(interface);
	ASSERT(ld);
	if (!ld)
		return EINVAL;

	IOVERCHECK(ld->version, "setLOCALE");

	WL_IOCTL(("wl%d: setLOCALE: %s\n", unit,
	          lookup_name(apple80211_locale_names, ld->locale)));

	switch (ld->locale) {
		case APPLE80211_LOCALE_FCC:
			country_code = "X0";
			cspec.rev = get_regrev(X0_REGREV_IDX);
			break;
		case APPLE80211_LOCALE_ETSI:
			country_code = "X3";
			cspec.rev = get_regrev(X3_REGREV_IDX);
			break;
		case APPLE80211_LOCALE_JAPAN:
			if (D11REV_LT(revinfo.corerev, 42)) {
				err = 1;
			} else {
				country_code = "JP";
				cspec.rev = get_regrev(JP_REGREV_IDX);
			}
			break;
		case APPLE80211_LOCALE_KOREA:
			if (D11REV_LT(revinfo.corerev, 42)) {
				err = 1;
			} else {
				country_code = "KR";
				cspec.rev = get_regrev(KR_REGREV_IDX);
			}
			break;
		case APPLE80211_LOCALE_APAC:
			if (D11REV_LT(revinfo.corerev, 42)) {
				err = 1;
			} else {
				country_code = "X1";
				cspec.rev = get_regrev(X1_REGREV_IDX);
			}
			break;
		case APPLE80211_LOCALE_ROW:
			country_code = "X2";
			cspec.rev = get_regrev(X2_REGREV_IDX);
			break;
		default:
			err = 1;
	}

	/* Check for invalid parameter */
	if (err) {
		WL_IOCTL(("wl%d: setLOCALE: unknown locale %d\n", unit, ld->locale));
		return EINVAL;
	}

	snprintf(cspec.country_abbrev, sizeof(cspec.country_abbrev), "%s", country_code);
	if (cspec.rev != -1)
		snprintf(cspec.ccode, sizeof(cspec.ccode), "%s", country_code);

	if (cspec.rev == -1 && cspec.ccode[0] == '\0')
		err = wlIovarOp("country", NULL, 0, (void *)&cspec,
				   WLC_CNTRY_BUF_SZ, IOV_SET, NULL);
	else
		err = wlIovarOp("country", NULL, 0, (void *)&cspec,
				   sizeof(cspec), IOV_SET, NULL);

	if (err) {
		WL_IOCTL(("wl%d: setLOCALE: error \"%s\" (%d) from WLC_SET_COUNTRY\n",
		          unit, bcmerrorstr(err), err));
	}

	return osl_error(err);
}

SInt32
AirPort_Brcm43xx::setINT_MIT( OSObject * interface, struct apple80211_intmit_data * imd )
{
	int val = 0;

	BCM_REFERENCE(interface);
	ASSERT(imd);
	if (!imd)
		return EINVAL;

	IOVERCHECK(imd->version, "setINT_MIT");

	// if interference mitigation is forced on, just return without err
	if (forceInterferenceMitigation) {
		return 0;
	}

	switch( imd->int_mit )
	{
		case APPLE80211_INT_MIT_OFF:
			val = INTERFERE_NONE;
			break;
		case APPLE80211_INT_MIT_AUTO:
			val = WLAN_AUTO;
			break;
		default:
			return EINVAL;
	}

	if( wlIoctlSetInt(WLC_SET_INTERFERENCE_MODE, val ) != 0 )
		return ENXIO;

	return 0;
}

SInt32
AirPort_Brcm43xx::setBSSID(OSObject * interface, struct apple80211_bssid_data *bd)
{
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
#endif
	BCM_REFERENCE(interface);
	ASSERT(bd);
	if (!bd)
		return EINVAL;

	IOVERCHECK(bd->version, "setBSSID");

	WL_IOCTL(("wl%d: setBSSID: %s\n",
	          unit, bcm_ether_ntoa(&bd->bssid, eabuf)));

	BCOPYADRS(&bd->bssid, &configured_bssid);

	return 0;
}

SInt32
AirPort_Brcm43xx::setDEAUTH( OSObject * interface, struct apple80211_deauth_data * dd )
{
	int err = BCME_OK;
	scb_val_t scb = {};
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN] = {};
#endif

	ASSERT(dd);
	if (!dd)
		return EINVAL;

#if defined(AWDL_FAMILY)
         if (!wlcif && roam_start_set && dd->deauth_reason == DOT11_RC_AUTH_INVAL ) {
         /* Send ROAM_END when EAPOL failed.*/
#if defined(APPLE80211_M_ROAM_END)
                 struct apple80211_roam_event_data event_data;
                 memset(&event_data, 0, sizeof(event_data));
                 event_data.reason = WLC_E_STATUS_FAIL;
                 postMessage(APPLE80211_M_ROAM_END, (void *)&event_data, sizeof(event_data));
#endif
                 WL_ASSOC(("%s(): post APPLE80211_M_ROAM_END event.\n", __FUNCTION__));
                 roam_start_set = FALSE;
         }
#endif /* AWDL_FAMILY */

	IOVERCHECK(dd->version, "setDEAUTH");

	WL_IOCTL(("wl%d: setDEAUTH: reason %d MAC %s\n",
	          unit, dd->deauth_reason, bcm_ether_ntoa(&dd->deauth_ea, eabuf)));

	/* block deauth from Airport supplicant while MIC report is the queue.
	 * the callback fn "wlc_tkip_countermeasures" will deauth and clear
	 * the key on transmit complete.
	 */
	if (dd->deauth_reason == DOT11_RC_MIC_FAILURE)
		return 0;

	IFINFOBLKP(interface)->deauth_reason = dd->deauth_reason;
	IFINFOBLKP(interface)->link_deauth_reason = dd->deauth_reason;
	scb.val = dd->deauth_reason;
	memcpy( &scb.ea, &dd->deauth_ea, sizeof( scb.ea ) );

	err = wlIoctl(WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb, sizeof( scb ), TRUE, NULL);

	return !err ? 0 : ENXIO;
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::setCOUNTERMEASURES( OSObject * interface,
                                struct apple80211_countermeasures_data * cd )
{
	/* Driver does the right thing on detecting MIC Failures so that supplicant
	 * does not need to do anything
	 */
	BCM_REFERENCE(interface);
	BCM_REFERENCE(cd);
	return 0;
}
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::setFRAG_THRESHOLD( OSObject * interface, struct apple80211_frag_threshold_data * td )
{
	int err = BCME_OK;

	BCM_REFERENCE(interface);

	ASSERT(td);
	if (!td)
		return EINVAL;

	IOVERCHECK(td->version, "setFRAG_THRESHOLD");

	err = wlIovarSetInt( "frag", td->threshold );

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setRATE_SET(OSObject * interface, struct apple80211_rate_set_data *rd)
{
	wl_rateset_t rateset = {};
	uint i = 0;
	int err = BCME_OK;

	BCM_REFERENCE(interface);
	ASSERT(rd);
	if (!rd)
		return EINVAL;

	IOVERCHECK(rd->version, "setRATE_SET");

	if (rd->num_rates > APPLE80211_MAX_RATES)
		return EINVAL;

	rateset.count = rd->num_rates;

	for( i = 0; i < rd->num_rates; i++ )
	{
		rateset.rates[i] = apple2mac_rate(rd->rates[i].rate);

		if( rd->rates[i].flags & APPLE80211_RATE_FLAG_BASIC )
			rateset.rates[i] |= DOT11_RATE_BASIC;
	}

	err = wlIoctl(WLC_SET_RATESET, &rateset, sizeof( rateset ), TRUE, NULL );

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setSHORT_SLOT( OSObject * interface, struct apple80211_short_slot_data * sd )
{
	int err = BCME_OK, val = 0;

	BCM_REFERENCE(interface);

	ASSERT(sd);
	if (!sd)
		return EINVAL;

	IOVERCHECK(sd->version, "setSHORT_SLOT");

	switch( sd->mode )
	{
		case APPLE80211_SHORT_SLOT_MODE_AUTO:
			val = WLC_SHORTSLOT_AUTO;
			break;

		case APPLE80211_SHORT_SLOT_MODE_LONG:
			val = WLC_SHORTSLOT_OFF;
			break;

		case APPLE80211_SHORT_SLOT_MODE_SHORT:
			val = WLC_SHORTSLOT_ON;
			break;

		default:
			return EINVAL;
	}

	err = wlIoctlSetInt(WLC_SET_SHORTSLOT_OVERRIDE, val);

	if( !err )
	{
		val = ( val == WLC_SHORTSLOT_ON ? 1 : 0 );
		err = wlIoctlSetInt(WLC_SET_SHORTSLOT_RESTRICT, val);
		if( err )
			err = 0;	// not really an error, probably just not in AP mode
	}

	return !err ? 0 : ENXIO;
}

int
AirPort_Brcm43xx::rateOverride(uint32 nrate, bool multicast)
{
	bool found = FALSE;
	const char *iovarname = NULL;
	int mcs = 0;
	int rate = 0;
	uint i = 0;
	int bcmerr = 0;
#ifndef MACOSX_DHD
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_bss_info_t *current_bss = NULL;

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);
#else
	int nmode;
	chanspec_t chanspec;
	wl_rateset_t rateset;
	uint8 mcsset[MCSSET_LEN];
#endif /* !MACOSX_DHD */
	// Reset rate override if nrate is zero
	if (nrate == 0 && !multicast) {
		rate_override = 0;
		(void) wlIovarSetInt("5g_rate", 0);
		(void) wlIovarSetInt("2g_rate", 0);
		return 0;
	}
#ifndef MACOSX_DHD
	current_bss = wlc_get_current_bss(bsscfg);
#else
	bcmerr = wlIoctl(WLC_GET_CURR_RATESET, &rateset, sizeof( rateset ), FALSE, NULL);
	if (bcmerr) {
		WL_ERROR(("%s: ioctl WLC_GET_CURR_RATESET failed! error: %d\n", __FUNCTION__,
			bcmerr));
		return bcmerr;
	}

	bcmerr = wlIovarOp("cur_mcsset", NULL, 0, &mcsset, sizeof(mcsset), FALSE, NULL);
	if (bcmerr) {
		WL_ERROR(("%s: wlIovarOp cur_mcsset failed! error: %d\n", __FUNCTION__,
			bcmerr));
		return bcmerr;
	}
#endif /* !MACOSX_DHD */

	// check for a valid rate in the current BSS
	if ((nrate & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_RATE) {
		rate = nrate & WL_RSPEC_RATE_MASK;
#ifndef MACOSX_DHD
		for (i = 0; i < current_bss->rateset.count; i++)
			if (rate == (current_bss->rateset.rates[i] & RATE_MASK))
				found = TRUE;
#else
		for (i = 0; i < rateset.count; i++) {
			if (rate == (rateset.rates[i] & RATE_MASK)) {
				found = TRUE;
			}
		}
#endif /* !MACOSX_DHD */
		if (!found)
			return BCME_BADARG;
	} else if ((nrate & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_HT) {
		mcs = nrate & WL_RSPEC_RATE_MASK;
		if (mcs > WLC_MAXMCS)
			return BCME_BADARG;
#ifndef MACOSX_DHD
		if (!isset(current_bss->rateset.mcs, mcs))
#else
		if (!isset(mcsset, mcs))
#endif /* !MACOSX_DHD */
		{
			return BCME_BADARG;
		}

	} else if ((nrate & WL_RSPEC_ENCODING_MASK) == WL_RSPEC_ENCODE_VHT) {
	} else {
		return BCME_BADARG;
	}

	// Here if we have a valid rate for the current BSS
#ifndef MACOSX_DHD
	if (multicast)
		iovarname = CHSPEC_IS5G(wlc_get_home_chanspec(bsscfg)) ? "5g_mrate" : "2g_mrate";
	else if (!N_ENAB(pub))
		iovarname = CHSPEC_IS5G(wlc_get_home_chanspec(bsscfg)) ? "5g_rate" : "2g_rate";
	else
		iovarname = "nrate";
#else
	bcmerr = wlIovarGetInt("nmode", &nmode);
	if (bcmerr) {
		WL_ERROR(("%s: wlIovarGetInt nmode failed! error: %d\n", __FUNCTION__,
			bcmerr));
		return bcmerr;
	}

	/* MACOSX_DHD home chanspec? */
	if (bcmerr = wlIovarOp("chanspec", NULL, 0, &chanspec, sizeof(chanspec), FALSE, NULL)) {
		return bcmerr;
	}

	if (multicast) {
		iovarname = CHSPEC_IS5G(chanspec) ? "5g_mrate" : "2g_mrate";
	} else if (!nmode) {
			iovarname = CHSPEC_IS5G(chanspec) ? "5g_rate" : "2g_rate";
	} else {
		iovarname = "nrate";
	}
#endif /* !MACOSX_DHD */
	bcmerr = wlIovarSetInt(iovarname, nrate);
	if (bcmerr) {
		WL_IOCTL(("wl%d: rateOverride: bcmerror \"%s\" (%d), setting \"%s\" rate 0x%x\n",
		          unit, bcmerrorstr(bcmerr), bcmerr, iovarname, nrate));
		return bcmerr;
	}

	if (multicast)
		mrate_override = nrate;
	else
		rate_override = nrate;

	return 0;
}

void
AirPort_Brcm43xx::rateOverrideReset()
{
	if (mrate_override) {
		mrate_override = 0;
		(void) wlIovarSetInt("5g_mrate", 0);
		(void) wlIovarSetInt("2g_mrate", 0);
	}

	if (rate_override) {
		rate_override = 0;
		(void) wlIovarSetInt("5g_rate", 0);
		(void) wlIovarSetInt("2g_rate", 0);
	}
}

SInt32
AirPort_Brcm43xx::setRATE( OSObject * interface, struct apple80211_rate_data * rd )
{
	int err = BCME_OK;
	int bcmerr = BCME_OK;
	int rate = 0;
	BCM_REFERENCE(interface);

	ASSERT(rd);
	if (!rd)
		return EINVAL;

	IOVERCHECK(rd->version, "setRATE");

	if (rd->num_radios != 1)
		return EINVAL;
#ifndef MACOSX_DHD
	if (!wl_find_bsscfg()->associated) {
		WL_IOCTL(("wl%d: setRATE: not associated\n", unit));
		return ENXIO;
	}
#endif /* !MACOSX_DHD */
	rate = apple2mac_rate(rd->rate[0]);
	if (rate > WLC_RATE_54M) {
		WL_IOCTL(("wl%d: setRATE: bad legacy rate %d Mbps\n", unit, rd->rate[0]));
		return EINVAL;
	}

	// Common rate override handler
	bcmerr = rateOverride(rate, FALSE);

	if (bcmerr) {
		err = osl_error(bcmerr);
		WL_IOCTL(("wl%d: setRATE: rateOverride() returns bcmerror \"%s\" (%d), "
		          "returning err %d\n",
		          unit, bcmerrorstr(bcmerr), bcmerr, err));
	}

	return err;
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::setMULTICAST_RATE( OSObject * interface, struct apple80211_rate_data * rd )
{
	int err = 0;
	int bcmerr;
	int rate;
	BCM_REFERENCE(interface);

	ASSERT(rd);
	if (!rd)
		return EINVAL;

	IOVERCHECK(rd->version, "setMULTICAST_RATE");

	if (rd->num_radios != 1)
		return EINVAL;

	if (!wl_find_bsscfg()->associated) {
		WL_IOCTL(("wl%d: setMULTICAST_RATE: not associated\n", unit));
		return ENXIO;
	}

	rate = apple2mac_rate(rd->rate[0]);
	if (rate > WLC_RATE_54M) {
		WL_IOCTL(("wl%d: setMULTICAST_RATE: bad legacy rate %d Mbps\n", unit, rd->rate[0]));
		return EINVAL;
	}

	// Common rate override handler
	bcmerr = rateOverride(rate, TRUE);

	if (bcmerr) {
		err = osl_error(bcmerr);
		WL_IOCTL(("wl%d: setMULTICAST_RATE: rateOverride() returns bcmerror \"%s\" (%d), "
		          "returning err %d\n",
		          unit, bcmerrorstr(bcmerr), bcmerr, err));
	}

	return err;
}
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::setSHORT_RETRY_LIMIT( OSObject * interface, struct apple80211_retry_limit_data * rld )
{
	int err = BCME_OK;

	BCM_REFERENCE(interface);
	ASSERT(rld);
	if (!rld)
		return EINVAL;

	IOVERCHECK(rld->version, "setSHORT_RETRY_LIMIT");

	err = wlIoctlSetInt(WLC_SET_SRL, rld->limit);

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setLONG_RETRY_LIMIT( OSObject * interface, struct apple80211_retry_limit_data * rld )
{
	int err = BCME_OK;

	BCM_REFERENCE(interface);

	ASSERT(rld);
	if (!rld)
		return EINVAL;

	IOVERCHECK(rld->version, "setLONG_RETRY_LIMIT");

	err = wlIoctlSetInt(WLC_SET_LRL, rld->limit);

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setTX_ANTENNA( OSObject * interface, struct apple80211_antenna_data * ad )
{
#ifdef __ppc__
	return 0;
#else
	int err = BCME_OK;

	BCM_REFERENCE(interface);

	ASSERT(ad);
	if (!ad)
		return EINVAL;

	IOVERCHECK(ad->version, "setTX_ANTENNA");

	WL_IOCTL(("wl%d: setTX_ANTENNA: num_radios %d:", unit, ad->num_radios));
	for (int i = 0; i < (int)ad->num_radios; i++)
		WL_IOCTL((" %d", (int)ad->antenna_index[i]));
	WL_IOCTL(("\n"));

	int antenna = (int)ad->antenna_index[0];

	if( antenna == -1 )
		antenna = 3;

	err = wlIoctlSetInt(WLC_SET_TXANT, antenna);

	return !err ? 0 : ENXIO;
#endif
}

SInt32
AirPort_Brcm43xx::setANTENNA_DIVERSITY( OSObject * interface, struct apple80211_antenna_data * ad )
{
#ifdef __ppc__
	return 0;
#else
	int err = BCME_OK;
	uint i = 0;

	BCM_REFERENCE(interface);
	ASSERT(ad);
	if (!ad)
		return EINVAL;

	IOVERCHECK(ad->version, "setANTENNA_DIVERSITY");

	WL_IOCTL(("wl%d: setANTENNA_DIVERSITY: num_radios %d:", unit, ad->num_radios));
	for (i = 0; i < ad->num_radios; i++)
		WL_IOCTL((" %d", (int)ad->antenna_index[i]));
	WL_IOCTL(("\n"));

	if (ad->num_radios == 0)
		return EINVAL;

	// rx diversity handled differently for multi RF vs single RF chain systems
	if (num_radios > 1) {
		// If we are multi RF chain system, num_radios needs to match
		if (ad->num_radios != num_radios) {
			WL_ERROR(("wl%d: setANTENNA_DIVERSITY: num_radios %d specifed, but have %d\n",
			          unit, ad->num_radios, num_radios));
			return EINVAL;
		}

		// accept an AUTO (-1) setting or a matching antenna index for each antenna_index value
		for (i = 0; i < ad->num_radios; i++) {
			if (ad->antenna_index[i] != (int)i &&
			    ad->antenna_index[i] != -1) {
				WL_ERROR(("wl%d: setANTENNA_DIVERSITY: antenna_index[%u] = %d, expected %u or -1",
				          unit, i, (int)ad->antenna_index[i], i));
				return EINVAL;
			}
		}
	} else {
		int antenna = (int)ad->antenna_index[0];
		if( antenna == -1 )
			antenna = 3;

		err = wlIoctlSetInt(WLC_SET_ANTDIV, antenna);
	}

	return !err ? 0 : ENXIO;
#endif
}

SInt32
AirPort_Brcm43xx::setDTIM_INT( OSObject * interface, apple80211_dtim_int_data * dd )
{
	int err = BCME_OK;

	BCM_REFERENCE(interface);
	ASSERT(dd);
	if (!dd)
		return EINVAL;

	IOVERCHECK(dd->version, "setDTIM_INT");

	err = wlIoctlSetInt(WLC_SET_DTIMPRD, dd->interval);

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setPOWERSAVE(OSObject * interface, struct apple80211_powersave_data *pd)
{
	int err = BCME_OK, val = 0;

	BCM_REFERENCE(interface);
	ASSERT(pd);
	if (!pd)
		return EINVAL;

	IOVERCHECK(pd->version, "setPOWERSAVE");

	WL_IOCTL(("wl%d: setPOWERSAVE: %d %s\n", unit, pd->powersave_level,
	          (pd->powersave_level == APPLE80211_POWERSAVE_MODE_DISABLED)?
	          "DISABLED":
	          (pd->powersave_level == APPLE80211_POWERSAVE_MODE_80211)?
	          "80211":
	          (pd->powersave_level == APPLE80211_POWERSAVE_MODE_VENDOR)?
	          "VENDOR":
			  "<unrecognized>"
	          ));

	switch( pd->powersave_level )
	{
		case APPLE80211_POWERSAVE_MODE_DISABLED:
			val = PM_OFF;
			break;
		// Apple change: Hook APPLE80211_POWERSAVE_MODE_MAX_POWERSAVE and
		//				 APPLE80211_POWERSAVE_MODE_80211 to PM_MAX
#if VERSION_MAJOR > 8
		case APPLE80211_POWERSAVE_MODE_MAX_POWERSAVE:
#endif
		case APPLE80211_POWERSAVE_MODE_80211:
			val = PM_MAX;
			break;
		// Apple change: Hook APPLE80211_POWERSAVE_MODE_MAX_THROUGHPUT to
		//				 PM_FAST
#if VERSION_MAJOR > 8
		case APPLE80211_POWERSAVE_MODE_MAX_THROUGHPUT:
#endif
		case APPLE80211_POWERSAVE_MODE_VENDOR:
#ifndef MACOSX_DHD
			val = PM_FAST;
			WL_ERROR(("Setting PM Mode to %d \n", val));
			break;
#else
			return 0;
#endif /* !MACOSX_DHD */
#ifdef SUPPORT_FEATURE_WOW
		case APPLE80211_POWERSAVE_MODE_WOW:
			// wowl is enabled unconditionally in the sync power state code
			if( !wowl_cap )
				return EINVAL;

			enableDefaultWowlBits(pWLC);

			if( !wlc_wowl_enable( pWLC->wowl ) )
				return ENXIO;
			else {
				/* Enable PME in the pci config */
				if (BUSTYPE(revinfo.bus) == PCI_BUS)
					si_pci_pmeen(pub->sih);

#ifdef AAPLPWROFF
				platformWoWEnable( true );
#endif
			}

			return 0;
#endif
		default:
			return EINVAL;
	}

#ifdef FACTORY_MERGE
	// If in factory power save mode, then skip power saving
	if (disableFactoryPowersave) {
		return 0;
	}
#endif

#ifdef AAPLPWROFF
	// check for power off state.  Return error if power off
	if (_powerState == PS_INDEX_ON) {
		err = wlIoctlSetInt(WLC_SET_PM, val);
	} else {
		WL_ERROR(("wl%d: %s called while _powerState is %ld\n",
			unit, __FUNCTION__, _powerState));
		err = EINVAL;
	}
#else
	err = wlIoctlSetInt(WLC_SET_PM, val);
#endif

	// Update the cached value
	cached_PM_state = val;
	cached_Apple_PM_state = pd->powersave_level;

	return !err ? 0 : ENXIO;
}

SInt32
AirPort_Brcm43xx::setRSN_IE(OSObject * interface, struct apple80211_rsn_ie_data * rid)
{
#ifndef MACOSX_DHD
	struct wlc_if * wlcif = wlLookupWLCIf( interface );
	wlc_bsscfg_t *bsscfg = wl_find_bsscfg(wlcif);
	ASSERT(bsscfg != NULL);
#else
	int bcmerr;
#endif /* !MACOSX_DHD */
	BCM_REFERENCE(interface);

	ASSERT(rid);
	if (!rid)
		return EINVAL;

	IOVERCHECK(rid->version, "setRSN_IE");

#ifdef BCMDBG
	if (WL_IOCTL_ON()) {
		if (rid->len == 0)
			WL_IOCTL(("wl%d: setRSN_IE: clearing RSN IE\n", unit));
		else {
			WL_IOCTL(("wl%d: setRSN_IE: setting RSN IE len %d\n",
				unit, (int)rid->len));
			prhex(NULL, rid->ie, rid->len);
		}
	}
#endif
#ifndef MACOSX_DHD
	if (bsscfg->assoc->ie) {
		MFREE(osh, bsscfg->assoc->ie, bsscfg->assoc->ie_len);
		bsscfg->assoc->ie = NULL;
		bsscfg->assoc->ie_len = 0;
	}
#endif /* !MACOSX_DHD */
	if (rid->len == 0)
		return 0;
#ifndef MACOSX_DHD
	bsscfg->assoc->ie = (uint8*)MALLOC(osh, rid->len);
	if (!bsscfg->assoc->ie) {
		WL_ERROR(("wl%d: setRSN_IE: out of memory, malloced %d bytes\n",
			unit, MALLOCED(osh)));
		return ENOMEM;
	}

	bsscfg->assoc->ie_len = rid->len;
	bcopy(rid->ie, bsscfg->assoc->ie, rid->len);

	return 0;
#else
	bcmerr = wlIovarOp("assoc_req_ies", NULL, 0, rid->ie, rid->len, true, interface);
	if (bcmerr) {
		WL_ERROR(("%s: wlIovarOp failed for assoc_req_ies error:%d\n", __FUNCTION__,
			bcmerr));
	}
	return bcmerr;
#endif /* !MACOSX_DHD */
}

int
AirPort_Brcm43xx::bandSupport(int *band_support) const
{
	int bcmerr = 0;
	uint iobuf[10] = {0};

	bcmerr = wlIoctlGet(WLC_GET_BANDLIST, &iobuf, sizeof(iobuf), NULL);
	if (bcmerr) {
		WL_ERROR(("wl%d: bandSupport: err %d from wlIoctl()\n", unit, bcmerr));
		return bcmerr;
	}

	if (iobuf[0] > 1)
		*band_support = WLC_BAND_ALL;
	else if (iobuf[1] == WLC_BAND_2G || iobuf[1] == WLC_BAND_5G)
		*band_support = iobuf[1];
	else {
		ASSERT("WLC_GET_BANDLIST returned an unknown band");
		return BCME_ERROR;
	}

	return 0;
}

int
AirPort_Brcm43xx::bandsOperational(uint32 *bands)
{
	int bcmerr = BCME_OK;
	int band = 0;
	int band_support = 0;

	*bands = 0;

	if ((bcmerr = wlIoctlGetInt(WLC_GET_BAND, &band)))
		return ENXIO;

	if ((bcmerr = bandSupport(&band_support)))
		return ENXIO;

	if (band_support != WLC_BAND_ALL)
		band = band_support;

	if (band == WLC_BAND_AUTO)
		*bands = APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_5GHZ;
	else if (band == WLC_BAND_2G)
		*bands = APPLE80211_C_FLAG_2GHZ;
	else {
		ASSERT(band == WLC_BAND_5G);
		*bands = APPLE80211_C_FLAG_5GHZ;
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::setPHY_MODE(OSObject *interface, struct apple80211_phymode_data *pd)
{
	uint32 phy_mode = 0;
	const uint32 valid_opt  = (APPLE80211_MODE_AUTO
				   | APPLE80211_MODE_11A | APPLE80211_MODE_11B
				   | APPLE80211_MODE_11G | APPLE80211_MODE_11N
#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
				   | APPLE80211_MODE_11AC
#endif
				   );
	int band = 0, gmode = 0, nmode = 0, vhtmode = 0, band_support = 0;
	int cur_band = 0, cur_gmode = 0;
	int want_2g = 0, want_5g = 0;
	int isup = FALSE;
	int err = BCME_OK;
	int bcmerr = 0;
#ifdef MACOSX_DHD
	int nmode_g;
#endif /* MACOSX_DHD */
#ifdef BCMDBG
	const int flagstr_size = 64;
	char flagstr[flagstr_size] = {0};
#endif

	BCM_REFERENCE(interface);

	ASSERT(pd);
	if (!pd)
		return EINVAL;

	IOVERCHECK(pd->version, "setPHY_MODE");

#ifdef BCMDBG
	bcm_format_flags(apple80211_phymode_flags, pd->phy_mode, flagstr, flagstr_size);
	WL_IOCTL(("wl%d: setPHY_MODE: 0x%x %s\n", unit, pd->phy_mode, flagstr));
#endif

	phy_mode = pd->phy_mode;
	gmode = GMODE_AUTO;

	/* error for unhandled options */
	if ((phy_mode == APPLE80211_MODE_UNKNOWN) ||
	    (phy_mode & ~valid_opt)) {
		err = EINVAL;
		goto exit;
	}

	/* error if AUTO is set with anything else */
	if ((phy_mode & APPLE80211_MODE_AUTO) && (phy_mode & ~APPLE80211_MODE_AUTO)) {
		err = EINVAL;
		goto exit;
	}

	/* reduce the combination of G+B to just G */
	if (phy_mode & APPLE80211_MODE_11G)
		phy_mode &= ~APPLE80211_MODE_11B;

	/* see if legacy mode was requested */
	if (phy_mode & APPLE80211_MODE_11B)
		gmode = GMODE_LEGACY_B;

	/* we do not allow N and legacy B */
	if ((phy_mode & APPLE80211_MODE_11B) && (phy_mode & APPLE80211_MODE_11N)) {
		err = EINVAL;
		goto exit;
	}

#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	/* we do not allow AC and legacy B */
	if ((phy_mode & APPLE80211_MODE_11B) && (phy_mode & APPLE80211_MODE_11AC)) {
		err = EINVAL;
		goto exit;
	}
#endif

	/* determine the desired bands
	 * If neither band is mentioned, or both bands are mentioned then we are not band locked.
	 * If one band is mentioned but not the other we are band locked.
	 */
	want_2g = (phy_mode & (APPLE80211_MODE_11B | APPLE80211_MODE_11G));
	want_5g = (phy_mode & APPLE80211_MODE_11A);

	if ((want_2g && want_5g) || (!want_2g && !want_5g))
		band = WLC_BAND_AUTO;
	else if (want_2g)
		band = WLC_BAND_2G;
	else
		band = WLC_BAND_5G;

	/* determine nmode setting */
	if ((phy_mode & APPLE80211_MODE_11N) ||
	    ((phy_mode & APPLE80211_MODE_AUTO) && PHYTYPE_11N_CAP(phytype)))
		nmode = TRUE;
	else
		nmode = FALSE;

	cached_nmode_state = nmode ? 1 : 0;
	/* Check for supported mode for the current hardware */

	/* nphy specified but hardware is not nphy */
	if (nmode && !PHYTYPE_11N_CAP(phytype)) {
		err = EINVAL;
		goto exit;
	}

#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	/* determine vhtmode setting */
	if ((phy_mode & APPLE80211_MODE_11AC) ||
	    ((phy_mode & APPLE80211_MODE_AUTO) && PHYTYPE_VHT_CAP(phytype)))
		vhtmode = TRUE;
	else
#endif
		vhtmode = FALSE;

	cached_vhtmode_state = vhtmode ? 1 : 0;
	/* Check for supported mode for the current hardware */

	/* nphy specified but hardware is not nphy */
	if (vhtmode && !PHYTYPE_VHT_CAP(phytype)) {
		err = EINVAL;
		goto exit;
	}

	/* band specified is not supported */
	if ((bcmerr = bandSupport(&band_support)))
		goto exit;

	if ((band_support != WLC_BAND_ALL) &&
	    ((want_2g && band_support != WLC_BAND_2G) ||
	     (want_5g && band_support != WLC_BAND_5G))) {
		err = EINVAL;
		goto exit;
	}
#ifndef MACOSX_DHD
	isup = pub->up;
#else
	isup = isUp();
#endif /* !MACOSX_DHD */
	if (isup)
		wl_down((struct wl_info *)this);

	/* apply the mode settings */
#ifndef MACOSX_DHD
	if (N_ENAB(pub) != nmode && (bcmerr = wlIovarSetInt("nmode", nmode)))
		goto exit;
#else
	if ((err = wlIovarGetInt("nmode", &nmode_g)) != 0) {
		goto exit;
	}

	if ((nmode_g != nmode) && (bcmerr = wlIovarSetInt("nmode", nmode))) {
		goto exit;
	}
#endif /* !MACOSX_DHD */
#ifndef MACOSX_DHD
	if (VHT_ENAB(pub) != vhtmode &&
	    (bcmerr = wlIovarSetInt("vhtmode", vhtmode)))
		goto exit;
#endif /* !MACOSX_DHD */
	if ((bcmerr = wlIoctlGetInt(WLC_GET_GMODE, &cur_gmode)))
		goto exit;

	if (cur_gmode != gmode &&
	    (bcmerr = wlIoctlSetInt(WLC_SET_GMODE, gmode)))
		goto exit;

	if ((bcmerr = wlIoctlGetInt(WLC_GET_BAND, &cur_band)))
		goto exit;

	if (cur_band != band &&
	    (bcmerr = wlIoctlSetInt(WLC_SET_BAND, band)))
		goto exit;

 exit:
	if (bcmerr) {
		err = osl_error(bcmerr);
		WL_IOCTL(("wl%d: setPHY_MODE: bcmerror \"%s\" (%d), returning err %d\n",
			unit, bcmerrorstr(bcmerr), bcmerr, err));
	} else if (err) {
		WL_IOCTL(("wl%d: setPHY_MODE: returning error %d\n", unit, err));
	}

	if (isup)
		wl_up((struct wl_info *)this);

	return err;
}

SInt32
AirPort_Brcm43xx::setGUARD_INTERVAL(OSObject * interface, struct apple80211_guard_interval_data * gid)
{
	BCM_REFERENCE(interface);
	ASSERT(gid);
	if (!gid)
		return EINVAL;

	IOVERCHECK(gid->version, "setGUARD_INTERVAL");

	WL_IOCTL(("wl%d: setGUARD_INTERVAL: %u\n", unit, (uint)gid->interval));

	if (!PHYTYPE_11N_CAP(phytype))
		return EOPNOTSUPP;

	if (gid->interval != APPLE80211_GI_LONG)
		return EINVAL;

	return 0;
}

SInt32
AirPort_Brcm43xx::setMCS(OSObject * interface, struct apple80211_mcs_data * md)
{
	int err = BCME_OK;
	int bcmerr = 0;
	uint32 nrate = 0;
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */

	BCM_REFERENCE(interface);
	ASSERT(md);
	if (!md)
		return EINVAL;

	IOVERCHECK(md->version, "setMCS");

	WL_IOCTL(("wl%d: setMCS: index %u\n", unit, (uint)md->index));
#ifdef MACOSX_DHD
	err = wlIovarGetInt("nmode", &nmode);
	if (err) {
		WL_ERROR(("%s: wlIovarGetInt nmode failed! error: %d\n", __FUNCTION__, err));
		return err;
	}

	if (!nmode) {
		return EOPNOTSUPP;
	}
#else
	if (!N_ENAB(pub))
		return EOPNOTSUPP;

	if (!wl_find_bsscfg()->associated) {
		WL_IOCTL(("wl%d: setMCS: not associated\n", unit));
		return ENXIO;
	}
#endif /* !MACOSX_DHD */
	if ((md->index != APPLE80211_MCS_INDEX_AUTO) &&
	    (md->index > APPLE80211_MAX_MCS_INDEX)) {
		WL_IOCTL(("wl%d: setMCS: bad MCS index %u\n", unit, (uint)md->index));
		return EINVAL;
	}

	if (md->index == APPLE80211_MCS_INDEX_AUTO) {
		nrate = 0;
		WL_IOCTL(("wl%d: setMCS: index set to auto, reset rate override\n",
		          unit));
	}
	else {
		nrate = (uint32)(WL_RSPEC_ENCODE_HT | md->index);
	}

	// Common rate override handler
	bcmerr = rateOverride(nrate, FALSE);

	if (bcmerr) {
		err = osl_error(bcmerr);
		WL_IOCTL(("wl%d: setMCS: rateOverride() returns bcmerror \"%s\" (%d), "
		          "returning err %d\n",
		          unit, bcmerrorstr(bcmerr), bcmerr, err));
	}

	return err;
}

#ifndef MACOSX_DHD
#ifdef APPLE80211_IOC_MCS_VHT
SInt32
AirPort_Brcm43xx::setMCS_VHT( OSObject * interface, struct apple80211_mcs_vht_data * mvd)
{
	int err = BCME_OK;
	int bcmerr = BCME_OK;
	uint32 rate = 0;

	BCM_REFERENCE(interface);
	ASSERT(mvd);
	if (!mvd)
		return EINVAL;

	IOVERCHECK(mvd->version, "setMCS_VHT");

	WL_ERROR(("wl%d: setMCS_VHT: index %u\n", unit, (uint)mvd->index));

	if (!wl_find_bsscfg()->associated) {
		WL_IOCTL(("wl%d: setMCS_VHT: not associated\n", unit));
		return ENXIO;
	}

	if ((!VHT_ENAB(pub))){
		WL_ERROR(("wl%d: setMCS_VHT: not VHT mode\n", unit));
		return ENXIO;
	}

	if ((mvd->index != APPLE80211_MCS_INDEX_AUTO) &&
	    (mvd->index > APPLE80211_MAX_MCS_INDEX)) {
		WL_ERROR(("wl%d: setMCS_VHT: bad MCS index %u\n", unit, (uint)mvd->index));
		return EINVAL;
	}

	if (mvd->index == APPLE80211_MCS_INDEX_AUTO) {
		rate = 0;
		WL_ERROR(("wl%d: setMCS_VHT: index set to auto, reset rate override\n",
		          unit));
	}
	else {
		rate = (uint32)(WL_RSPEC_ENCODE_VHT | mvd->index);
		rate  = rate | (mvd->nss << WL_RSPEC_VHT_NSS_SHIFT);
		if (mvd->bandwidth == WLC_20_MHZ)
			rate = rate | WL_RSPEC_BW_20MHZ;
		else if (mvd->bandwidth == WLC_40_MHZ)
			rate = rate | WL_RSPEC_BW_40MHZ;
		else if (mvd->bandwidth == WLC_80_MHZ)
			rate = rate | WL_RSPEC_BW_80MHZ;
		else if (mvd->bandwidth == WLC_160_MHZ)
			rate = rate | WL_RSPEC_BW_160MHZ;

		if (mvd->gi == APPLE80211_GI_SHORT)
			rate = rate | WL_RSPEC_SGI;
	}

	// Common rate override handler
	bcmerr = rateOverride(rate, FALSE);

	if (bcmerr) {
		err = osl_error(bcmerr);
		WL_ERROR(("wl%d: setMCS_VHT: rateOverride() returns bcmerror \"%s\" (%d), "
		          "returning err %d\n",
		          unit, bcmerrorstr(bcmerr), bcmerr, err));
	}

	return err;
}
#endif
#endif /* !MACOSX_DHD */
SInt32
AirPort_Brcm43xx::setLDPC( OSObject * interface, struct apple80211_ldpc_data * ld )
{
	int bcmerr = BCME_OK;
	uint32 ldpc_cap = 0;

	BCM_REFERENCE(interface);
	ASSERT(ld);
	if (!ld)
		return EINVAL;

	IOVERCHECK(ld->version, "setLDPC");

	WL_IOCTL(("wl%d: setLDPC: enabled %u\n", unit, (uint)ld->enabled));

	if (!PHYTYPE_11N_CAP(phytype))
		return EOPNOTSUPP;

	if (ld->enabled != 0)
		ldpc_cap = AUTO;
	else
		ldpc_cap = OFF;

	bcmerr = wlIovarSetInt("ldpc_cap", ldpc_cap);
	if (bcmerr) {
		return EOPNOTSUPP;
	} else {
		return 0;
	}
}

SInt32
AirPort_Brcm43xx::setMSDU(OSObject * interface, struct apple80211_msdu_data * md)
{
	int bcmerr = 0;
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */
	ASSERT(md);

	BCM_REFERENCE(interface);
	if (!md)
		return EINVAL;

	IOVERCHECK(md->version, "setMSDU");

	WL_IOCTL(("wl%d: setMSDU: max_length %u\n", unit, (uint)md->max_length));
#ifndef MACOSX_DHD
	if (!N_ENAB(pub))
		return EOPNOTSUPP;
#else
	if ((bcmerr = wlIovarGetInt("nmode", &nmode))) {
		IOLog( "wlIovarGetInt(nmode) returned %d\n", bcmerr);
		return bcmerr;
	}

	if (!nmode) {
		return EOPNOTSUPP;
	}
#endif /* !MACOSX_DHD */

	bcmerr = wlIovarSetInt("amsdu_aggbytes", md->max_length);
	if (bcmerr)
		WL_IOCTL(("wl%d: setMSDU: bcmerror \"%s\" (%d), setting amsdu_aggbytes\n",
		          unit, bcmerrorstr(bcmerr), bcmerr));

	return osl_error(bcmerr);
}

SInt32
AirPort_Brcm43xx::setMPDU( OSObject * interface, struct apple80211_mpdu_data * md )
{
	int bcmerr = 0;
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */

	BCM_REFERENCE(interface);
	ASSERT(md);
	if (!md)
		return EINVAL;

	IOVERCHECK(md->version, "setMPDU");

	WL_IOCTL(("wl%d: setMPDU: density %u factor %u\n", unit,
	          (uint)md->max_factor, (uint)md->max_density));
#ifndef MACOSX_DHD
	if (!N_ENAB(pub))
		return EOPNOTSUPP;
#else
	if ((bcmerr = wlIovarGetInt("nmode", &nmode))) {
		IOLog( "wlIovarGetInt(nmode) returned %d\n", bcmerr);
		return bcmerr;
	}

	if (!&nmode) {
		return EOPNOTSUPP;
	}
#endif /* MACOSX_DHD */

	bcmerr = wlIovarSetInt("ampdu_density", md->max_density);
	if (bcmerr)
		WL_IOCTL(("wl%d: setMSDU: bcmerror \"%s\" (%d), setting ampdu_density\n",
		          unit, bcmerrorstr(bcmerr), bcmerr));

#ifndef MACOSX_DHD
	if (!bcmerr && !WLCISACPHY(pWLC->band) &&
		 (bcmerr = wlIovarSetInt("ampdu_rx_factor", md->max_factor)))
#else
	if (!bcmerr && (bcmerr = wlIovarSetInt("ampdu_rx_factor", md->max_factor)))
#endif /* !MACOSX_DHD */
	{
		WL_IOCTL(("wl%d: setMSDU: bcmerror \"%s\" (%d), setting ampdu_rx_factor\n",
			unit, bcmerrorstr(bcmerr), bcmerr));
	}

	return osl_error(bcmerr);
}

SInt32
AirPort_Brcm43xx::setBLOCK_ACK( OSObject * interface, struct apple80211_block_ack_data * bad )
{
	int bcmerr = 0;
	int enabled = 0;
	int isup = 0;
#ifdef MACOSX_DHD
	int nmode;
#endif /* MACOSX_DHD */
	BCM_REFERENCE(interface);
	ASSERT(bad);
	if (!bad)
		return EINVAL;

	IOVERCHECK(bad->version, "setBLOCK_ACK");

	WL_IOCTL(("wl%d: setBLOCK_ACK: enabled %d immed %d cbba %d implicit %d\n", unit,
	          (int)bad->ba_enabled, (int)bad->immediate_ba_enabled,
	          (int)bad->cbba_enabled, (int)bad->implicit_ba_enabled));
#ifndef MACOSX_DHD
	if (!N_ENAB(pub))
		return EOPNOTSUPP;
#else
	if ((bcmerr = wlIovarGetInt("nmode", &nmode))) {
		IOLog( "wlIovarGetInt(nmode) returned %d\n", bcmerr);
		return bcmerr;
	}

	if (!nmode) {
		return EOPNOTSUPP;
	}
#endif /* !MACOSX_DHD */

	if (bad->ba_enabled && bad->immediate_ba_enabled &&
	    bad->cbba_enabled && bad->implicit_ba_enabled)
		enabled = AMPDU_AGGMODE_AUTO;	/* ampdu auto on: driver decides ampdu agg mode */
	else if (!bad->ba_enabled && !bad->immediate_ba_enabled &&
	         !bad->cbba_enabled && !bad->implicit_ba_enabled)
		enabled = AMPDU_AGG_OFF;
	else
		return EINVAL;
#ifndef MACOSX_DHD
	isup = pub->up;
#else
	isup = isUp();
#endif /* !MACOSX_DHD */
	if (isup)
		wl_down((struct wl_info *)this);

	bcmerr = wlIovarSetInt("ampdu", enabled);

	if (bcmerr)
		WL_IOCTL(("wl%d: setBLOCK_ACK: bcmerror \"%s\" (%d), setting ampdu\n",
		          unit, bcmerrorstr(bcmerr), bcmerr));

	if (isup)
		wl_up((struct wl_info *)this);

	return osl_error(bcmerr);
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::setPHY_SUB_MODE( OSObject * interface, struct apple80211_physubmode_data * pd )
{
	int bcmerr = 0;
	int band_support = 0;
	int band = 0;
	int txbw = 0;

	BCM_REFERENCE(interface);
	ASSERT(pd);
	if (!pd)
		return EINVAL;

	IOVERCHECK(pd->version, "setPHY_SUB_MODE");

	WL_IOCTL(("wl%d: setPHY_SUB_MODE: request for phy_sub_mode 0x%x, channel flags 0x%x\n",
	          unit, pd->phy_submode, pd->flags));

	if (pd->phy_mode != APPLE80211_MODE_11N) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: set for phy_sub_mode for phy type %d. "
		          "Only support for APPLE80211_MODE_11N (%d)\n",
		          unit, pd->phy_mode, APPLE80211_MODE_11N));
		return EOPNOTSUPP;
	} else if (!PHYTYPE_11N_CAP(phytype)) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: set for phy_sub_mode for 11N phy, "
		          "but device is not 11N.", unit));
		return EOPNOTSUPP;
	}


	if (phy_submode == 0) {
		// default state
		phy_submode = APPLE80211_SUBMODE_11N_AUTO;
		phy_submode_flags = APPLE80211_C_FLAG_20MHZ | APPLE80211_C_FLAG_40MHZ;
		phy_submode_flags |= APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_5GHZ;
	}

	// Check for change in GF mode while up
	if (phy_submode != pd->phy_submode && phy_submode == APPLE80211_SUBMODE_11N_GF &&
	    pub->up) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: cannot change submode from GF to non-GF (%d) "
		          "while driver is up (radios on)\n",
		          unit, pd->phy_submode));
		return ENXIO;
	}
	if (phy_submode != pd->phy_submode && pd->phy_submode == APPLE80211_SUBMODE_11N_GF &&
	    pub->up) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: cannot change submode from non-GF (%d) to "
		          "GF while driver is up (radios on)\n",
		          unit, phy_submode));
		return ENXIO;
	}

	// Check for Legacy/LegacyDup/HT/HTDup change while not associated
	if (!wl_find_bsscfg()->associated &&
	    (pd->phy_submode & (APPLE80211_SUBMODE_11N_LEGACY | APPLE80211_SUBMODE_11N_LEGACY_DUP |
	                        APPLE80211_SUBMODE_11N_HT | APPLE80211_SUBMODE_11N_HT_DUP))) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: cannot change to a BSS specific mode (%d) "
		          "while not associated to a BSS\n",
		          unit, pd->phy_submode));
		return ENXIO;
	}

	// check HT BSS for HT modes

	// check 40 MHz BSS for DUP modes

	if ((pd->flags & (APPLE80211_C_FLAG_20MHZ | APPLE80211_C_FLAG_40MHZ)) !=
	    (APPLE80211_C_FLAG_20MHZ | APPLE80211_C_FLAG_40MHZ)) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: cannot set bandwidth specific flags, "
		          "must have 20/40 set but got 0x%x\n",
		          unit, pd->flags));
		return EOPNOTSUPP;
	}

	// Accept both band flags unset as indicating that both bands can be used.
	// Convert the both-unset case to a canonical both-set value
	if ((pd->flags & (APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_5GHZ)) == 0)
		pd->flags |= (APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_5GHZ);

	if ((pd->flags & APPLE80211_C_FLAG_2GHZ) &&
	    (pd->flags & APPLE80211_C_FLAG_5GHZ))
		band = WLC_BAND_AUTO;
	else if (pd->flags & APPLE80211_C_FLAG_2GHZ)
		band = WLC_BAND_2G;
	else
		band = WLC_BAND_5G;

	if ((bcmerr = bandSupport(&band_support)))
		return ENXIO;

	if (band_support == WLC_BAND_2G && band == WLC_BAND_5G) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: cannot set 5G band on this device\n", unit));
		return EOPNOTSUPP;
	}
	if (band_support == WLC_BAND_5G && band == WLC_BAND_2G) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: cannot set 2G band on this device\n", unit));
		return EOPNOTSUPP;
	}

	// Set the band
	if ((bcmerr = wlIoctlSetInt(WLC_SET_BAND, band))) {
		WL_IOCTL(("wl%d: setPHY_SUB_MODE: error %d setting band to %d\n",
		          unit, bcmerr, band));
		return ENXIO;
	}

	// Set the submode
	switch (pd->phy_submode) {
	case APPLE80211_SUBMODE_11N_AUTO:
		if (phy_submode == APPLE80211_SUBMODE_11N_LEGACY ||
		    phy_submode == APPLE80211_SUBMODE_11N_LEGACY_DUP ||
		    phy_submode == APPLE80211_SUBMODE_11N_HT ||
		    phy_submode == APPLE80211_SUBMODE_11N_HT_DUP) {
			rateOverrideReset();
			(void) wlIovarSetInt("ofdm_txbw", AUTO);
			(void) wlIovarSetInt("mimo_txbw", AUTO);
		} else if (phy_submode == APPLE80211_SUBMODE_11N_GF)
			(void) wlIovarSetInt("mimo_preamble", AUTO);
		break;

	case APPLE80211_SUBMODE_11N_LEGACY:
	case APPLE80211_SUBMODE_11N_LEGACY_DUP:
		bcmerr = rateOverride(WLC_RATE_6M, FALSE);
		bcmerr = rateOverride(WLC_RATE_6M, TRUE);
		txbw = (pd->phy_submode == APPLE80211_SUBMODE_11N_LEGACY_DUP) ?
		        PHY_TXC1_BW_40MHZ_DUP : AUTO;
		(void) wlIovarSetInt("mimo_txbw", AUTO);
		(void) wlIovarSetInt("ofdm_txbw", txbw);
		break;

	case APPLE80211_SUBMODE_11N_HT:
	case APPLE80211_SUBMODE_11N_HT_DUP:
		bcmerr = rateOverride((WL_RSPEC_ENCODE_HT | 0), FALSE);
		bcmerr = rateOverride((WL_RSPEC_ENCODE_HT | 0), TRUE);
		txbw = (pd->phy_submode == APPLE80211_SUBMODE_11N_HT_DUP) ?
		        PHY_TXC1_BW_40MHZ_DUP : AUTO;
		(void) wlIovarSetInt("mimo_txbw", txbw);
		(void) wlIovarSetInt("ofdm_txbw", AUTO);
		break;

	case APPLE80211_SUBMODE_11N_GF:
		(void) wlIovarSetInt("mimo_preamble", WLC_N_PREAMBLE_GF);
		break;

	default:
		break;
	}

	// Save the new submode
	phy_submode = pd->phy_submode;
	phy_submode_flags = pd->flags;

	return 0;
}

#if VERSION_MAJOR > 9

SInt32
AirPort_Brcm43xx::setROAM_THRESH( OSObject * interface, struct apple80211_roam_threshold_data * rtd )
{
	BCM_REFERENCE(interface);
	ASSERT(rtd);
	if (!rtd)
		return EINVAL;

	IOVERCHECK(rtd->version, "setROAM_THRESH");
	struct {
		int val;
		int band;
	} x;

	x.band = WLC_BAND_ALL;
	x.val = rtd->rssi_thresh;
	int bcmerror = wlIoctl(WLC_SET_ROAM_TRIGGER, &x, sizeof( x ), TRUE, NULL );
	if (bcmerror)
		return ENXIO;

	return 0;
}

// Apple change: setSTA_DEAUTH, setSTA_AUTHORIZE, wsecFlagForCipher, and setRSN_CONF are for hostapd support
SInt32
AirPort_Brcm43xx::setSTA_DEAUTH( OSObject * interface, struct apple80211_sta_deauth_data * sdd )
{
	scb_val_t scb_val = {};
	BCM_REFERENCE(interface);

	scb_val.val = sdd->reason;
	memcpy( &scb_val.ea, &sdd->mac, sizeof( sdd->mac ) );

	int bcmerr = wlIoctl(WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val, sizeof( scb_val ), TRUE, NULL );

	return bcmerr ? ENXIO : 0;
}

SInt32
AirPort_Brcm43xx::setSTA_AUTHORIZE( OSObject * interface, struct apple80211_sta_authorize_data * sad )
{
	BCM_REFERENCE(interface);
	int cmd = sad->authorize ? WLC_SCB_AUTHORIZE : WLC_SCB_DEAUTHORIZE;

	int bcmerr = wlIoctl(cmd, (void *)&sad->mac, ETHER_ADDR_LEN, TRUE, NULL );

	return osl_error( bcmerr );
}

static int
wsecFlagForCipher( UInt32 cipher )
{
	switch( cipher )
	{
		case APPLE80211_CIPHER_WEP_40:
		case APPLE80211_CIPHER_WEP_104:
			return WEP_ENABLED;

		case APPLE80211_CIPHER_TKIP:
			return TKIP_ENABLED;

		case APPLE80211_CIPHER_AES_CCM:
			return AES_ENABLED;
	}

	return 0;
}

SInt32
AirPort_Brcm43xx::setRSN_CONF( OSObject * interface, struct apple80211_rsn_conf_data * rcd )
{
	int wsec = 0, wpa_auth = 0, wpa_cap = 0, bcmerror;
	UInt32 i = 0;
	struct wlc_if * wlcif = wlLookupWLCIf( interface );

	// Get existing wsec configuration to avoid trampling it
	bcmerror = wlIovarGetInt("wsec", &wsec);
	if (bcmerror) {
		IOLog( "wlIovarGetInt(wsec) returned %d\n", bcmerror );
		return osl_error( bcmerror );
	}

	wsec &= ~(AES_ENABLED | TKIP_ENABLED | WEP_ENABLED);

	// Figure out wpa_auth
	for( i = 0; i < rcd->wpa.rsn_authselcnt && i < APPLE80211_RSN_MAX_AUTHSELS; i++ )
	{
		switch( rcd->wpa.rsn_authsels[i] )
		{
			case APPLE80211_AUTHTYPE_WPA:
				wpa_auth |= WPA_AUTH_UNSPECIFIED;
				break;

			case APPLE80211_AUTHTYPE_WPA_PSK:
				wpa_auth |= WPA_AUTH_PSK;
				break;
		}
	}

	for( i = 0; i < rcd->rsn.rsn_authselcnt && i < APPLE80211_RSN_MAX_AUTHSELS; i++ )
	{
		switch( rcd->rsn.rsn_authsels[i] )
		{
			case APPLE80211_AUTHTYPE_WPA2:
				wpa_auth |= WPA2_AUTH_UNSPECIFIED;
				break;

			case APPLE80211_AUTHTYPE_WPA2_PSK:
				wpa_auth |= WPA2_AUTH_PSK;
				break;
		}
	}

	// No way to specify ciphers as WPA or WPA2 only...
	if( rcd->wpa.rsn_authselcnt )
	{
		for( i = 0; i < rcd->wpa.rsn_uciphercnt && i < APPLE80211_RSN_MAX_UCIPHERS; i++ )
			wsec |= wsecFlagForCipher( rcd->wpa.rsn_uciphers[i] );
	}
	if( rcd->rsn.rsn_authselcnt )
	{
		for( i = 0; i < rcd->rsn.rsn_uciphercnt && i < APPLE80211_RSN_MAX_UCIPHERS; i++ )
			wsec |= wsecFlagForCipher( rcd->rsn.rsn_uciphers[i] );

		if( rcd->rsn.rsn_caps & APPLE80211_RSN_CAP_PREAUTH )
			wpa_cap = WPA_CAP_WPA2_PREAUTH;
	}

	wlc_bsscfg_t *bsscfg = wl_find_bsscfg(wlcif);
	bool isup = bsscfg ? bsscfg->up : FALSE;

	if( isup ) wlc_bsscfg_down( pWLC, bsscfg );

	bcmerror = wlIovarSetInt( "auth", DOT11_OPEN_SYSTEM );
	IOLog( "wlIovarSetInt(auth) returned %d\n", bcmerror );
	if( bcmerror )
		goto exit;

	bcmerror = wlIovarSetInt( "wpa_cap", wpa_cap );
	IOLog( "wlIovarSetInt(wpa_cap=0x%x) returned %d\n", wpa_cap, bcmerror );
	if( bcmerror )
		goto exit;

	bcmerror = wlIovarSetInt( "wsec", wsec );
	IOLog( "wlIovarSetInt(wsec=0x%x) returned %d\n", wsec, bcmerror );
	if( bcmerror )
		goto exit;

	bcmerror = wlIovarSetInt( "wpa_auth", wpa_auth );
	IOLog( "wlIovarSetInt(wpa_auth=0x%x) returned %d\n", wpa_auth, bcmerror );

	wlc_update_beacon( pWLC );
	wlc_update_probe_resp( pWLC, FALSE );

exit:
	if( isup ) wlc_bsscfg_up( pWLC, bsscfg );

	return osl_error( bcmerror );
}

#ifdef APPLEIOCIE
SInt32
AirPort_Brcm43xx::setIE( OSObject * interface, struct apple80211_ie_data * ied )
{
	// First get all the IEs to do a signature match against
	uchar *iebuf = NULL;
	uchar *data = NULL, *dst = NULL;
	int tot_ie = 0, pktflag = 0, iecount = 0;
	vndr_ie_buf_t *ie_getbuf = NULL;
	vndr_ie_setbuf_t *ie_setbuf = NULL;
	vndr_ie_info_t *ie_info = NULL;
	vndr_ie_info_t *mod_ie_info = NULL;
	uint iov_len = 0;

	vndr_ie_t *ie = NULL;
	int err = ENOENT, bcmerror = BCME_OK;
	uint8 *signature = NULL;

	BCM_REFERENCE(interface);

	printApple80211IE(ied, FALSE, TRUE);

	ie_getbuf = (vndr_ie_buf_t*)MALLOCZ(osh, WLC_IOCTL_MAXLEN);
	if (!ie_getbuf)
		return ENOMEM;

	ie_setbuf = (vndr_ie_setbuf_t*)MALLOCZ(osh, WLC_IOCTL_MAXLEN);

	if (!ie_setbuf) {
		err =  ENOMEM;
		goto done;
	}

	bcmerror = wlIovarOp( "vndr_ie",
	                    NULL,
	                    0,
	                    ie_getbuf,
	                    WLC_IOCTL_MAXLEN,
	                    FALSE,
	                    interface);

	if (bcmerror)
		goto done;

	signature = (uint8*)ied->ie_data;
	tot_ie = ie_getbuf->iecount;
	iebuf = (uchar *)&ie_getbuf->vndr_ie_list[0];

	ie_setbuf->vndr_ie_buffer.iecount = 1;
	mod_ie_info = &ie_setbuf->vndr_ie_buffer.vndr_ie_list[0];

	for (iecount = 0; iecount < tot_ie; iecount++) {
		ie_info = (vndr_ie_info_t *) iebuf;

		pktflag = ie_info->pktflag;
		iebuf += sizeof(uint32);

		ie = &ie_info->vndr_ie_data;

		if (matchIESignature(signature, ied->signature_len, ie)) {
			// Remove this IE first
			strncpy(ie_setbuf->cmd, "del", strlen("del"));
			ie_setbuf->cmd[strlen("del")] = '\0';

			// pktflag + IE Hdr + ie->len
			bcopy(ie_info, mod_ie_info, sizeof(uint32) + ie->len + VNDR_IE_HDR_LEN);
			iov_len = sizeof(vndr_ie_setbuf_t) + ie->len - sizeof(uint32);

			bcmerror = wlIovarOp( "vndr_ie",
			                         NULL,
			                         0,
			                         ie_setbuf,
			                         iov_len,
			                         IOV_SET,
			                         interface );

			if (bcmerror)
				goto done;

			// Instructed to only "add"
			if (ied->add == 0) {
				err = 0;
				goto done;
			}

			break;
		}

		iebuf += ie->len + VNDR_IE_HDR_LEN;
	}

	if (ied->add == 0) {
		WL_ERROR(("%s: Did not find the IE\n", __FUNCTION__));
		goto done;
	}

	// Now add the new one
	mod_ie_info->pktflag = 0;
	strncpy(ie_setbuf->cmd, "add", strlen("add"));
	ie_setbuf->cmd[strlen("add")] = '\0';

	// Set the pktflags
	if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_BEACON)
		mod_ie_info->pktflag |= (VNDR_IE_BEACON_FLAG | VNDR_IE_CUSTOM_FLAG);

	if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_PROBE_RESP)
		mod_ie_info->pktflag |= (VNDR_IE_PRBRSP_FLAG | VNDR_IE_CUSTOM_FLAG);

	if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_ASSOC_RESP)
		mod_ie_info->pktflag |= (VNDR_IE_ASSOCRSP_FLAG | VNDR_IE_CUSTOM_FLAG);

	if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_PROBE_REQ)
		mod_ie_info->pktflag |= (VNDR_IE_PRBREQ_FLAG | VNDR_IE_CUSTOM_FLAG);

	if (ied->frame_type_flags & APPLE80211_FRAME_TYPE_F_ASSOC_REQ)
		mod_ie_info->pktflag |= (VNDR_IE_ASSOCREQ_FLAG | VNDR_IE_CUSTOM_FLAG);

	data = (uchar*)ied->ie_data;
	dst = (uchar*)&mod_ie_info->vndr_ie_data;
	mod_ie_info->vndr_ie_data.id = data[0];

	bcopy((void *)&data[1], (void *)&dst[VNDR_IE_HDR_LEN], ied->ie_len - 1);

	// Subtract the ID portion
	mod_ie_info->vndr_ie_data.len = ied->ie_len - 1;
	// Precise Length of iovar - list count
	iov_len = sizeof(vndr_ie_setbuf_t) + mod_ie_info->vndr_ie_data.len - sizeof(uint32);

	bcmerror = wlIovarOp( "vndr_ie",
	                         NULL,
	                         0,
	                         ie_setbuf,
	                         iov_len,
	                         IOV_SET,
	                         interface );

done:
	if (ie_getbuf)
		MFREE(osh, ie_getbuf, WLC_IOCTL_MAXLEN);

	if (ie_setbuf)
		MFREE(osh, ie_setbuf, WLC_IOCTL_MAXLEN);

	if ( bcmerror )
		return osl_error( bcmerror );

	return 0;
}
#endif /* APPLEIOCIE */

#ifdef IO80211P2P
// max number of clientes allowed to associate to GO
#define MACOS_GO_MAX_ASSOC		1	// if 0 or < 0, means use default settings
// P2P GO specific scb timeout check period
#define MACOS_P2P_GO_SCB_TIMEOUT	10	// if 0, disable GO scb timeout and use global scb timeout

SInt32
AirPort_Brcm43xx::setP2P_LISTEN( OSObject * interface, struct apple80211_p2p_listen_data * pld )
{
	ASSERT(pld);
	if( !pld )
		return EINVAL;
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);

	if (!virtIf || !WLC_P2P_CAP(pWLC))
		return ENXIO;

#ifdef WLP2P
	wl_p2p_disc_st_t st = {};

	st.state = WL_P2P_DISC_ST_LISTEN;
	st.dwell = (uint16)pld->listen_time;
	st.chspec = CH20MHZ_CHSPEC( pld->channel.channel );

	int err = wlIovarOp( "p2p_state",
	                        NULL,
	                        0,
	                        &st,
	                        sizeof( st ),
	                        IOV_SET,
	                        virtIf );

	if (!err)
		scan_event_external = 1;

	return osl_error( err );
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::setP2P_SCAN( OSObject * interface, struct apple80211_scan_data * sd )
{
	BCM_REFERENCE(interface);
	ASSERT(sd);
	if (!sd || !WLC_P2P_CAP(pWLC))
		return EINVAL;

#ifdef WLP2P
	int err;
	UInt32 paramsSize = 0;
	wl_p2p_scan_t *p2pparams = NULL;
	wl_scan_params_t * params = NULL;
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);

#ifdef IO80211P2P
	require_action( virtIf , exit, err = ENXIO );
#endif
	require_action( sd->num_channels <= WL_NUMCHANNELS, exit, err = EINVAL );
	require_action( sd->ssid_len <= sizeof( params->ssid.SSID ), exit, err = EINVAL );

	paramsSize = sizeof(wl_p2p_scan_t);
	paramsSize += WL_SCAN_PARAMS_FIXED_SIZE + ( sd->num_channels * sizeof( UInt16 ) );

	p2pparams = (wl_p2p_scan_t *)MALLOC(osh, paramsSize);
	require_action( p2pparams, exit, err = ENOMEM );

	bzero( p2pparams, paramsSize );

	// Initialize defaults
	// Set the type to default scan method instead of ESCAN (event scan)
	p2pparams->type = 'S';

	// SCAN params follow the p2p scan structure
	params = (wl_scan_params_t *)(p2pparams + 1);

	memset( &params->bssid, 0xff, ETHER_ADDR_LEN );
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = -1;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	// BSS Type
	switch( sd->bss_type ) {
		case APPLE80211_AP_MODE_INFRA:
			params->bss_type = DOT11_BSSTYPE_INFRASTRUCTURE;
			break;
		case APPLE80211_AP_MODE_ANY:
			params->bss_type = DOT11_BSSTYPE_ANY;
			break;
		default:
			err = EINVAL;
			goto exit;
	}

	switch( sd->scan_type ) {
		case APPLE80211_SCAN_TYPE_ACTIVE:
			params->scan_type = DOT11_SCANTYPE_ACTIVE;
			if( sd->dwell_time )
				params->active_time = (int)sd->dwell_time;
			break;
		case APPLE80211_SCAN_TYPE_PASSIVE:
			params->scan_type = DOT11_SCANTYPE_PASSIVE;
			if( sd->dwell_time )
				params->passive_time = (int)sd->dwell_time;
			break;
		default:
			err = EINVAL;
			goto exit;
	}

	if( sd->rest_time > 20 )
		params->home_time = (int)sd->rest_time;

	if( !ETHER_ISNULLADDR( &sd->bssid ) )
		memcpy( &params->bssid, &sd->bssid, sizeof( sd->bssid ) );

	for( UInt32 i = 0; i < sd->num_channels; i++ )
		params->channel_list[i] = CH20MHZ_CHSPEC( sd->channels[i].channel );

	params->channel_num = sd->num_channels;

	if( sd->ssid_len )
	{
		memcpy( params->ssid.SSID, sd->ssid, sd->ssid_len );
		params->ssid.SSID_len = sd->ssid_len;
	}

	err = wlIovarOp( "p2p_scan",
	                    NULL,
	                    0,
	                    p2pparams,
	                    (int)paramsSize,
	                    IOV_SET,
	                    virtIf );

	// Mark this as external scan request
	if (!err)
		scan_event_external = 1;
	err = osl_error( err );

exit:

	if( p2pparams )
		MFREE(osh, p2pparams, paramsSize);

	return err;
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::setVIRTUAL_IF_CREATE( OSObject * interface, struct apple80211_virt_if_create_data * icd )
{
	BCM_REFERENCE(interface);
	if (!WLC_P2P_CAP(pWLC))
		return EINVAL;

#ifdef WLP2P
	int bsscfg_idx = 0;
	int err = EINVAL;

	if( icd->role == APPLE80211_VIRT_IF_ROLE_P2P_DEVICE )
	{
		struct ether_addr curr_etheraddr = {}, p2p_da = {};
		struct ether_addr *user_da = NULL;
		bool override_p2p_da = FALSE;

		err = wlIovarGetInt("p2p_dev", &bsscfg_idx);

		// IOVAR will error if a device exists and is already in discovery state
		if (!err) {
			// APPLE CHANGE: Enhanced failure condition logging for <rdar://problem/9339105>
			IOLog("setVIRTUAL_IF_CREATE: p2p_dev get(1) failed\n");
			return EALREADY;
		}

		// get the current ethernet address
		(void) wlIovarOp("cur_etheraddr", NULL, 0, &curr_etheraddr, ETHER_ADDR_LEN, IOV_GET, NULL);

		// Zero macaddr means use default p2p device addr
		if (ETHER_ISNULLADDR(&icd->mac)) {
			// get the current p2p_da
			(void) wlIovarOp("p2p_da_override", NULL, 0, &p2p_da, ETHER_ADDR_LEN, IOV_GET, NULL);

			// default p2p_da should be curr_etheraddr with the local bit set
			ETHER_SET_LOCALADDR(&curr_etheraddr);
			if (BCMPADRS(&curr_etheraddr, &p2p_da)) {
				// current p2p da not equal to default p2p_da, set to default p2p_da
				user_da = &curr_etheraddr;
				override_p2p_da = TRUE;
			}
		}
		// Non-zero macaddr means override with user defined addr
		else {
			// validate the mac address passed in.
			// 1. icd->mac is not multicast
			// 2. icd->mac is local address
			// 3. last 5 bytes of icd->mac and curr_etheraddr are the same
			if (ETHER_ISMULTI(&icd->mac) || !ETHER_IS_LOCALADDR(&icd->mac) ||
			    bcmp(&(icd->mac.octet[1]), &(curr_etheraddr.octet[1]), ETHER_ADDR_LEN-1)) {
                                // APPLE CHANGE: Enhanced failure condition logging for <rdar://problem/9339105>
				IOLog("setVIRTUAL_IF_CREATE: icd->mac passed in is invalid!\n");
				return EINVAL;
			}
			user_da = &icd->mac;
			override_p2p_da = TRUE;
		}

		// override the p2p_da if needed
		if (override_p2p_da == TRUE) {
			err = wlIovarOp("p2p_da_override", NULL, 0, user_da, ETHER_ADDR_LEN, IOV_SET, NULL);
			if (err) {
                                // APPLE CHANGE: Enhanced failure condition logging for <rdar://problem/9339105>
				IOLog("setVIRTUAL_IF_CREATE: p2p_da_override set failed: err = 0x%x\n", err);
				return osl_error( err );
			}
		}

		_attachingIfRole = icd->role;
		err = wlIovarSetInt( "p2p_disc", 1 );
		if( err == 0 )
		{
			AirPort_Brcm43xxP2PInterface *virtIf = NULL;

			if (SLIST_EMPTY(&virt_intf_list)) {
				// APPLE CHANGE: Enhanced failure condition logging for <rdar://problem/9339105>
				IOLog("setVIRTUAL_IF_CREATE: virtual interface list check failed\n");
				return ENXIO;
			}

			err = wlIovarGetInt("p2p_dev", &bsscfg_idx);
			if (err) {
				// APPLE CHANGE: Enhanced failure condition logging for <rdar://problem/9339105>
				IOLog("setVIRTUAL_IF_CREATE: p2p_dev get(2) failed (%d)\n",err);
				return osl_error ( err );
			}

			struct virt_intf_entry *elt = NULL;
			SLIST_FOREACH(elt, &virt_intf_list, entries) {
				if (elt->obj->unit == (uint)bsscfg_idx) {
					virtIf = elt->obj;
					break;
				}
			}

			ASSERT(virtIf);

			strncpy( icd->name, virtIf->getBSDName(), sizeof( icd->name ) );
			icd->name[sizeof( icd->name ) - 1] = 0;
			// Enable RESTRICT_DEV_RESP p2p feature
			ConfigP2PFeatures(virtIf, WL_P2P_FEAT_RESTRICT_DEV_RESP, TRUE);
		}
	}
	else if( icd->role == APPLE80211_VIRT_IF_ROLE_P2P_GO ||
	         icd->role == APPLE80211_VIRT_IF_ROLE_P2P_CLIENT)
	{
		struct wl_p2p_if ifData = {};

		memcpy( &ifData.addr, &icd->mac, sizeof( icd->mac ) );
		if (icd->role == APPLE80211_VIRT_IF_ROLE_P2P_GO)
		{
			ifData.type = WL_P2P_IF_GO;

			// Set default channel only if primary is unassociated for non-multi-channel
			if (!wl_find_bsscfg()->associated)
				ifData.chspec = CH20MHZ_CHSPEC( 1 );
			else
				ifData.chspec = 0; // Follow the Infra client
		} else {
			ifData.type = WL_P2P_IF_CLIENT;
			ifData.chspec = 0;
		}

		IOLog("setVIRTUAL_IF_CREATE: P2P interface role (%u)\n", icd->role);

		_attachingIfRole = icd->role;
		err = wlIovarOp("p2p_ifadd", NULL, 0, &ifData, sizeof( ifData ), IOV_SET, NULL );
		if( err == 0 )
		{
			AirPort_Brcm43xxP2PInterface * virtIf = NULL;

			if (SLIST_EMPTY(&virt_intf_list))
				return ENXIO;

			virtIf = SLIST_FIRST(&virt_intf_list)->obj;

			// Both are IFNAMSIZ buffers
			strncpy( icd->name, virtIf->getBSDName(), IFNAMSIZ - 1 );

			if( icd->role == APPLE80211_VIRT_IF_ROLE_P2P_GO )
			{
				ConfigP2PFeatures(virtIf, WL_P2P_FEAT_GO_NOLEGACY, TRUE);
			}
		}
	} else {
		// APPLE CHANGE: Enhanced failure condition logging for <rdar://problem/9339105>
		IOLog("setVIRTUAL_IF_CREATE: Unrecognized interface role (%u)\n", icd->role);
		return EINVAL;
	}

    // APPLE CHANGE: Enhanced failure condition logging for <rdar://problem/9339105>
    if( err )
        IOLog("setVIRTUAL_IF_CREATE: Failed (%d)\n",err);

	return osl_error( err );
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::setVIRTUAL_IF_DELETE( OSObject * interface, struct apple80211_virt_if_delete_data * idd )
{
	BCM_REFERENCE(interface);
	if (!WLC_P2P_CAP(pWLC))
		return EINVAL;

#ifdef WLP2P
	if( OSDynamicCast( IO80211VirtualInterface, interface ) )
	{
		// Can't ask an interface to delete itself
		if( !strncmp( ((IO80211VirtualInterface *)interface)->getBSDName(), idd->name, sizeof( idd->name ) ) )
			return EINVAL;
	}

	int err = 0;
	AirPort_Brcm43xxP2PInterface * delIf = NULL;

	// Locate virtual interface to be deleted
	struct virt_intf_entry *elt = NULL;
	SLIST_FOREACH(elt, &virt_intf_list, entries) {
		if (!strncmp( elt->obj->getBSDName(), idd->name, IFNAMSIZ )) {
			delIf = elt->obj;
			break;
		}
	}

	// Interface being deleted must be virtual
	if( !delIf )
		return EINVAL;

	switch( delIf->getInterfaceRole() )
	{
		case APPLE80211_VIRT_IF_ROLE_P2P_DEVICE:
			// Disable RESTRICT_DEV_RESP p2p feature
			ConfigP2PFeatures(delIf, WL_P2P_FEAT_RESTRICT_DEV_RESP, FALSE);
			err = wlIovarSetInt( "p2p_disc", 0 );
			break;
		case APPLE80211_VIRT_IF_ROLE_P2P_GO:
			// clear the GO scb_timeout period
			wlc_p2p_go_scb_timeout_set(pWLC->p2p, 0);
		case APPLE80211_VIRT_IF_ROLE_P2P_CLIENT:
		{
			err = wlIovarOp( "p2p_ifdel",
			                    NULL,
			                    0,
			                    &delIf->wlcif->u.bsscfg->cur_etheraddr,
			                    sizeof( struct ether_addr ),
			                    IOV_SET,
			                    interface );

			if (err == BCME_NOTFOUND) {
				if (delIf->wlcif->u.bsscfg->enable)
					wlc_bsscfg_disable(pWLC, delIf->wlcif->u.bsscfg);
				wlc_bsscfg_free(pWLC, delIf->wlcif->u.bsscfg);
				err = 0;
			}

			break;
		}
		default:
			return EINVAL;
	}

	return osl_error( err );
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::setP2P_GO_CONF( OSObject * interface, struct apple80211_p2p_go_conf_data * gcd )
{
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);
	char country[APPLE80211_MAX_CC_LEN] = {0};

	BCM_REFERENCE(interface);
	if (!virtIf || !WLC_P2P_CAP(pWLC))
		return ENXIO;

	WL_P2P(("%s():\n", __FUNCTION__));
#if VERSION_MAJOR > 10
	if (is_channel_supported(gcd->go_channel.channel, &country[0]) == BCME_ERROR) {
		WL_ERROR(("%s: country %s channel %d is not supported\n", __FUNCTION__,
			country, gcd->go_channel.channel));
		return EINVAL;
	}
#endif /* VERSION_MAJOR > 10 */
#ifdef WLP2P
	int bcmerr = 0;
	int wsec = 0;

	// For now just support open mode
	if( gcd->go_auth_lower != APPLE80211_AUTHTYPE_OPEN || gcd->go_auth_upper != APPLE80211_AUTHTYPE_NONE )
		return EINVAL;

	struct wlc_if * wlcif = wlLookupWLCIf( interface );

	if( !wlcif )
	{
		IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: wlLookupWLCIf failed\n" );
		return EINVAL;
	}

	bcmerr = wlc_bsscfg_down( pWLC, wlcif->u.bsscfg );
	if( bcmerr )
	{
		IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: wlc_bsscfg_down failed (%d)\n", bcmerr );
		return osl_error( bcmerr );
	}

	bcmerr = wlIovarGetInt("wsec", &wsec);
	if( bcmerr )
	{
		IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Failed to get security (%d)\n", bcmerr );
		return osl_error( bcmerr );
	}
	wsec &= ~(AES_ENABLED | TKIP_ENABLED | WEP_ENABLED);

	bcmerr = wlIovarOp("wsec", NULL, 0, &wsec, sizeof( wsec ), IOV_SET, interface);
	if( bcmerr )
	{
		IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Failed to set security (%d)\n", bcmerr );
		return osl_error( bcmerr );
	}

#ifndef WLMCHAN
	// Set channel only if primary sta is not associated.
	if (!wl_find_bsscfg()->associated) {

		struct apple80211_channel_data cd = { APPLE80211_VERSION, { 0, 0, 0} };

		memcpy( &cd.channel, &gcd->go_channel, sizeof( gcd->go_channel ) );

		SInt32 err = setCHANNEL( interface, &cd );
		if( err )
		{
			IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Unable to set channel (%d)\n", err );
			return err;
		}
	}
#else
	// Set channel for this interface
	struct apple80211_channel_data cd = { APPLE80211_VERSION, { 0, 0, 0} };

	memcpy( &cd.channel, &gcd->go_channel, sizeof( gcd->go_channel ) );

	SInt32 err = setCHANNEL( interface, &cd );
	if( err )
	{
		IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Unable to set channel (%d)\n", err );
		return err;
	}
#endif /* WLMCHAN */

	// Turn on Dynamic Beaconing BEFORE setting the SSID
	if (isset(gcd->go_flags, APPLE80211_P2P_GO_FLAG_NO_BEACON)) {
		uint dynbcn = 1;
		bcmerr = wlIovarOp("dynbcn", NULL, 0, &dynbcn, sizeof( dynbcn ), IOV_SET, interface);
		if( bcmerr )
		{
			IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Failed to set dynbcn (%d)\n", bcmerr );
			return osl_error( bcmerr );
		}
	}

	wlc_ssid_t ssid = {};

	ssid.SSID_len = gcd->go_ssid_len;
	memcpy( ssid.SSID, gcd->go_ssid, MIN(ssid.SSID_len, sizeof(ssid.SSID)) );

	bcmerr = wlIovarOp("ssid", NULL, 0, &ssid, sizeof( ssid ), IOV_SET, interface);
	if( bcmerr )
	{
		IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Failed to set SSID (%d)\n", bcmerr );
		return osl_error( bcmerr );
	}

	/* Force max association for GO to be MACOS_GO_MAX_ASSOC if it is non-zero */
	if (MACOS_GO_MAX_ASSOC > 0) {
		uint maxbss = MACOS_GO_MAX_ASSOC;

		bcmerr = wlIovarOp("bss_maxassoc", NULL, 0, &maxbss, sizeof( maxbss ), IOV_SET, interface);
		if( bcmerr )
		{
			IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Failed to set bss maxassoc (%d)\n", bcmerr );
			return osl_error( bcmerr );
		}
	}

	/* Force more frequent checks for STA timeout */
	wlc_p2p_go_scb_timeout_set(pWLC->p2p, MACOS_P2P_GO_SCB_TIMEOUT);

	/* APPLE MODIFICATION <cbz@apple.com> <rdar://problem/8913516> - hide the SSID in beacons */
	SInt32 isClosed = 1;
	bcmerr = wlIoctl(WLC_SET_CLOSED, &isClosed, sizeof( isClosed ), TRUE, interface);
	if( bcmerr ) {
		IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: Failed to close SSID (%d)\n", bcmerr );
		return osl_error( bcmerr );
	}

	if (virtIf->enabledBySystem()) {
		WL_P2P(("%s(): virtIf is enabled.\n", __FUNCTION__));
		bcmerr = wlc_bsscfg_up( pWLC, wlcif->u.bsscfg );
		if( bcmerr )
		{
			IO8Log( "AirPort_Brcm43xx::setP2P_GO_CONF: wlc_bsscfg_up failed (%d)\n", bcmerr );
			return osl_error( bcmerr );
		}
	}
	return 0;
#endif /* WLP2P */
}

#if VERSION_MAJOR > 10
SInt32
AirPort_Brcm43xx::setP2P_OPP_PS( OSObject * interface, struct apple80211_opp_ps_data * opd )
{
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);

	if (!virtIf || !WLC_P2P_CAP(pWLC))
		return ENXIO;

#ifdef WLP2P
	int bcmerr = BCME_OK;
	wl_p2p_ops_t wlops = {};

	IOVERCHECK(opd->version, "setP2P_OPP_PS");

	struct wlc_if *wlcif = wlLookupWLCIf( interface );

	if (!wlcif)
	{
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_OPP_PS: wlLookupWLCIf failed\n"));
		return EINVAL;
	}

	virtIf->opp_ps_enable = (opd->enabled == 0) ? FALSE : TRUE;

	if ((virtIf->opp_ps_enable == FALSE) ||
	    ((virtIf->opp_ps_enable == TRUE) && virtIf->ct_window)) {
		wlops.ops = virtIf->opp_ps_enable;
		wlops.ctw = (uint8)virtIf->ct_window;
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_OPP_PS: setting opp ps, enable 0x%x, ctw 0x%x\n",
		          wlops.ops, wlops.ctw));
		bcmerr = wlIovarOp("p2p_ops", NULL, 0, &wlops, sizeof( wlops ), IOV_SET, interface);
	}

	if (bcmerr)
	{
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_OPP_PS: Failed to set p2p opp ps (%d)\n", bcmerr));
	}

	return osl_error(bcmerr);
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::getP2P_OPP_PS( OSObject * interface, struct apple80211_opp_ps_data * opd )
{
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);

	if (!virtIf || !WLC_P2P_CAP(pWLC))
		return ENXIO;

#ifdef WLP2P
	int bcmerr = BCME_OK;
	wl_p2p_ops_t wlops = {};

	opd->version = APPLE80211_VERSION;

	struct wlc_if *wlcif = wlLookupWLCIf( interface );

	if (!wlcif)
	{
		WL_IOCTL(("AirPort_Brcm43xx::getP2P_OPP_PS: wlLookupWLCIf failed\n"));
		return EINVAL;
	}

	opd->enabled = (virtIf->opp_ps_enable == TRUE) ? 1 : 0;

	if ((virtIf->opp_ps_enable == FALSE) ||
	    ((virtIf->opp_ps_enable == TRUE) && virtIf->ct_window)) {
		bcmerr = wlIovarOp("p2p_ops", NULL, 0, &wlops, sizeof( wlops ), IOV_GET, interface);
		if (!bcmerr && ((virtIf->opp_ps_enable != wlops.ops) ||
		                (wlops.ops && (virtIf->ct_window != wlops.ctw)))) {
			WL_IOCTL(("AirPort_Brcm43xx::getP2P_OPP_PS: driver enable 0x%x, ctw 0x%x"
			          "does not match virtIf opp_ps_enable 0x%x, ct_window 0x%x\n",
			          wlops.ops, wlops.ctw, virtIf->opp_ps_enable, (uint)virtIf->ct_window));
		}
	}

	if (bcmerr)
	{
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_OPP_PS: Failed to set p2p opp ps (%d)\n", bcmerr));
	}

	return osl_error(bcmerr);
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::setP2P_CT_WINDOW( OSObject * interface, struct apple80211_ct_window_data * cwd )
{
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);

	if (!virtIf || !WLC_P2P_CAP(pWLC))
		return ENXIO;

#ifdef WLP2P
	int bcmerr = BCME_OK;
	wl_p2p_ops_t wlops = {};

	IOVERCHECK(cwd->version, "setP2P_CT_WINDOW");

	struct wlc_if *wlcif = wlLookupWLCIf( interface );

	if (!wlcif)
	{
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_CT_WINDOW: wlLookupWLCIf failed\n"));
		return EINVAL;
	}

	if (cwd->ct_window < 10) {
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_CT_WINDOW: ct_window 0x%x must be >= 10 TU\n",
		          cwd->ct_window));
		return EINVAL;
	}
	// save the ct_window value
	virtIf->ct_window = cwd->ct_window;

	if (virtIf->opp_ps_enable == TRUE) {
		wlops.ops = virtIf->opp_ps_enable;
		wlops.ctw = (uint8)virtIf->ct_window;
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_CT_WINDOW: setting opp ps, enable 0x%x, ctw 0x%x\n",
		          wlops.ops, wlops.ctw));
		bcmerr = wlIovarOp("p2p_ops", NULL, 0, &wlops, sizeof( wlops ), IOV_SET, interface);
	}

	if (bcmerr)
	{
		WL_IOCTL(("AirPort_Brcm43xx::setP2P_CT_WINDOW: Failed to set p2p opp ps (%d)\n", bcmerr));
	}

	return osl_error(bcmerr);
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::getP2P_CT_WINDOW( OSObject * interface, struct apple80211_ct_window_data * cwd )
{
	AirPort_Brcm43xxP2PInterface *virtIf = getVirtIf(interface);

	if (!virtIf || !WLC_P2P_CAP(pWLC))
		return ENXIO;

#ifdef WLP2P
	int bcmerr = BCME_OK;
	wl_p2p_ops_t wlops;
	cwd->version = APPLE80211_VERSION;

	struct wlc_if *wlcif = wlLookupWLCIf( interface );

	if (!wlcif)
	{
		WL_IOCTL(("AirPort_Brcm43xx::getP2P_CT_WINDOW: wlLookupWLCIf failed\n"));
		return EINVAL;
	}

	cwd->ct_window = (UInt32)virtIf->ct_window;

	if ((virtIf->opp_ps_enable == FALSE) ||
	    ((virtIf->opp_ps_enable == TRUE) && virtIf->ct_window)) {
		bcmerr = wlIovarOp("p2p_ops", NULL, 0, &wlops, sizeof( wlops ), IOV_GET, interface);
		if (!bcmerr && ((virtIf->opp_ps_enable != wlops.ops) ||
		                (wlops.ops && (virtIf->ct_window != wlops.ctw)))) {
			WL_IOCTL(("AirPort_Brcm43xx::getP2P_CT_WINDOW: driver enale 0x%x, ctw 0x%x"
			          "does not match virtIf opp_ps_enable 0x%x, ct_window 0x%x\n",
			          wlops.ops, wlops.ctw, virtIf->opp_ps_enable, (uint)virtIf->ct_window));
		}
	}

	if (bcmerr)
	{
		WL_IOCTL(("AirPort_Brcm43xx::getP2P_CT_WINDOW: Failed to set p2p opp ps (%d)\n", bcmerr));
	}

	return osl_error(bcmerr);
#endif /* WLP2P */
}

SInt32
AirPort_Brcm43xx::get_channels_by_country(struct channels_info *param)
{
	int err = BCME_OK;
	/* country string is 3 bytes + NULL */
	char country[APPLE80211_MAX_CC_LEN + 1];
	wl_uint32_list_t  *list = NULL;
	UInt32 i = 0, num_channels = 0;
	UInt32 chanspec = 0, len = 0;

	err = wlIoctl(WLC_GET_COUNTRY, country, (APPLE80211_MAX_CC_LEN + 1), FALSE, NULL);
	if (err)
		return osl_error(err);

#if APPLE80211_MAX_CC_LEN < 2
#error "Expecting space for 2 chars in cc field"
#endif
	// copy the 2 character country code with null termination
	param->version = APPLE80211_VERSION;
	bzero(param->countryCode, APPLE80211_MAX_CC_LEN);
	bcopy(country, param->countryCode, 2);


	/* MAX_NUM_CHANNELS is defined to be 64 in apple80211_ioctl.h */
	len = sizeof(uint32) * (WL_NUMCHANSPECS + 1);

	list = (wl_uint32_list_t*)MALLOC(osh, len);

	if (list == NULL)
		return ENOMEM;

	list->count = WL_NUMCHANSPECS;

	/* Get all the supported chanspecs */
	err = wlIovarOp( "chanspecs", &chanspec, sizeof( chanspec ), list, len, FALSE,
	                    NULL );
	if (err) {
		WL_ERROR(("wl%d: getSUPPORTED_CHANNELS: error \"%s\" (%d) from chanspecs\n",
			unit, bcmerrorstr(err), err));
		MFREE(osh, list, len);
		return ENXIO;
	}

	num_channels = MIN(list->count, APPLE80211_MAX_CHANNELS);
	param->numChanSpecs = num_channels;
	for( i = 0; i < num_channels; i++ )
	{
		chanspec = list->element[i];
		param->chanSpec[i] = chanspec;
		param->chanNum[i] = wf_chspec_ctlchan(chanspec);
		param->indoorRestricted[i] = (wlc_restricted_chanspec(pWLC->cmi, chanspec)) ? 1 : 0;
		// determine if  the chanspec is in a radar domain
		param->radarDFS[i] = (wlc_radar_chanspec(pWLC->cmi, chanspec)) ? 1 : 0;
		param->support40MHz[i] = (CHSPEC_IS40(chanspec)) ? 1 : 0;
		param->support80MHz[i] = (CHSPEC_IS80(chanspec)) ? 1 : 0;
		param->passive[i] = (wlc_quiet_chanspec(pWLC->cmi, chanspec)) ? 1 : 0;
	}

	if (list)
		MFREE(osh, list, len);
	return err;
}

SInt32
AirPort_Brcm43xx::is_channel_supported(UInt32 sel_channel, char *country)
{
	struct channels_info param;
	UInt32 i;
	int err = BCME_ERROR;

	if (get_channels_by_country(&param) != BCME_OK) {
		WL_ERROR(("%s Unable to read channels info\n", __FUNCTION__));
		return err;
	}

	bzero(country, APPLE80211_MAX_CC_LEN);
	bcopy(param.countryCode, country, 2);

	for( i = 0; i < param.numChanSpecs; i++ )
	{
		if ((sel_channel == param.chanNum[i]) && !param.radarDFS[i] && !param.passive[i]) {
			return BCME_OK;
		} else if (sel_channel == param.chanNum[i]) {
			break;
		}
	}

	return err;
}
#endif // VERSION_MAJOR > 10
#endif /* IO80211P2P */

#endif /* VERSION_MAJOR > 9 */

#ifdef SUPPORT_FEATURE_WOW
SInt32
AirPort_Brcm43xx::setWOW_PARAMETERS( OSObject * interface, struct apple80211_wow_parameter_data * wpd )
{
	BCM_REFERENCE(interface);
	if( !wowl_cap )
		return EOPNOTSUPP;

	int wowl = 0, beacon_loss_time = 0;
	uint8 *dst = NULL;
	int brcmerror;

	brcmerror = wlIovarGetInt("wowl_os", &wowl);
	if (brcmerror)
		goto exit;

	wowl &= ~(WL_WOWL_MAGIC | WL_WOWL_NET | WL_WOWL_DIS| WL_WOWL_RETR | WL_WOWL_BCN);
	// Wake condition flags
	if( isset( wpd->wake_cond_map, APPLE80211_WAKE_COND_MAGIC_PATTERN ) )
		wowl |= WL_WOWL_MAGIC;

	if( isset( wpd->wake_cond_map, APPLE80211_WAKE_COND_NET_PATTERN ) && wpd->pattern_count )
		wowl |= WL_WOWL_NET;

	if( isset( wpd->wake_cond_map, APPLE80211_WAKE_COND_DISASSOCIATED ) ||
	    isset(wpd->wake_cond_map, APPLE80211_WAKE_COND_DEAUTHED)) {
		wowl |= WL_WOWL_DIS;
	}

	if( isset( wpd->wake_cond_map, APPLE80211_WAKE_COND_RETROGRADE_TSF ) )
		wowl |= WL_WOWL_RETR;

	if (isset(wpd->wake_cond_map, APPLE80211_WAKE_COND_BEACON_LOSS) && wpd->beacon_loss_time) {
		wowl |= WL_WOWL_BCN;
		beacon_loss_time = (int)wpd->beacon_loss_time;
	}

	brcmerror = wlIovarSetInt( "wowl_os", wowl );
	if( brcmerror )
		goto exit;

	if (beacon_loss_time) {
		brcmerror = wlIovarSetInt( "wowl_bcn_loss", beacon_loss_time );
		if( brcmerror )
			goto exit;
	}

	if (wowl & WL_WOWL_NET)	{
		UInt32 i;

		dst = (uint8*)MALLOC(osh, WLC_IOCTL_MAXLEN);

		if (!dst)
			return ENOMEM;

		strncpy((char*)dst, "clr", strlen("clr"));
		dst[strlen("clr")] = '\0';

		/* Send 'clr' to clear the current list */
		brcmerror = wlIovarOp( "wowl_pattern",
		                          NULL,
		                          0,
		                          dst,
		                          (int)(strlen("clr")+1),
		                          TRUE,
		                          NULL );

		if (brcmerror)
			goto exit;

		if (wpd->pattern_count) {
			wl_wowl_pattern_t * brcmPat;
			uint iovar_len;

			strncpy((char*)dst, "add", strlen("add"));
			dst[strlen("add")] = '\0';

			brcmPat = (wl_wowl_pattern_t *)(dst + (strlen("add") + 1));

			for( i = 0; i < wpd->pattern_count; i++ )
				{
					/* Reinit in the loop */
					iovar_len = strlen("add") + 1;

					brcmPat->masksize = 0;
					brcmPat->offset = 0;
					brcmPat->patternoffset = sizeof( wl_wowl_pattern_t );
					brcmPat->patternsize = wpd->patterns[i].len;
					memcpy( brcmPat + 1, wpd->patterns[i].pattern, brcmPat->patternsize );
					iovar_len += sizeof(wl_wowl_pattern_t) + brcmPat->patternsize;

					brcmerror = wlIovarOp( "wowl_pattern",
					                          NULL,
					                          0,
					                          dst,
					                          (int)iovar_len,
					                          TRUE,
					                          NULL );

					if ( brcmerror )
						goto exit;
				}
		}
	}

exit:

	if( dst )
		MFREE(osh, dst, WLC_IOCTL_MAXLEN);

	// Apple change: <rdar://problem/6501147> Support dynamic interface wake flags

	// WoW config changed, tell network controller to query disabled filters again
	if( OSDynamicCast( IO80211Interface, interface ) )
		((IO80211Interface *)interface)->inputEvent( kIONetworkEventWakeOnLANSupportChanged, NULL );

	return osl_error( brcmerror );
}
#endif /* SUPPORT_FEATURE_WOW */

#ifdef APPLE80211_IOC_CDD_MODE
SInt32
AirPort_Brcm43xx::setCDD_MODE( IO80211Interface * interface, struct apple80211_cdd_mode_data * cmd )
{

	struct wl_spatial_mode_data spatial_mode;
	int err;

	BCM_REFERENCE(interface);
	ASSERT(cmd);
	if (!cmd)
		return EINVAL;

	IOVERCHECK(cmd->version, "setCDD_MODE");

	WL_IOCTL(("wl%d: setCDD_MODE: 2.4G %d 5GL %d 5GM %d 5GH %d 5GU %d\n",
		unit, cmd->cdd_mode_24G, cmd->cdd_mode_5G_lower,
		cmd->cdd_mode_5G_middle, cmd->cdd_mode_5G_H, cmd->cdd_mode_5G_upper));

	spatial_mode.mode[SPATIAL_MODE_2G_IDX] =
		(cmd->cdd_mode_24G == APPLE80211_CDD_AUTO) ? AUTO : cmd->cdd_mode_24G;
	spatial_mode.mode[SPATIAL_MODE_5G_LOW_IDX] =
		(cmd->cdd_mode_5G_lower == APPLE80211_CDD_AUTO) ? AUTO : cmd->cdd_mode_5G_lower;
	spatial_mode.mode[SPATIAL_MODE_5G_MID_IDX] =
		(cmd->cdd_mode_5G_middle == APPLE80211_CDD_AUTO) ? AUTO : cmd->cdd_mode_5G_middle;
	spatial_mode.mode[SPATIAL_MODE_5G_HIGH_IDX] =
		(cmd->cdd_mode_5G_H == APPLE80211_CDD_AUTO) ? AUTO : cmd->cdd_mode_5G_H;
	spatial_mode.mode[SPATIAL_MODE_5G_UPPER_IDX] =
		(cmd->cdd_mode_5G_upper == APPLE80211_CDD_AUTO) ? AUTO : cmd->cdd_mode_5G_upper;

	err = wlIovarOp("spatial_policy", NULL, 0, &spatial_mode, sizeof(spatial_mode),
		IOV_SET, interface);

	if (err) {
		WL_IOCTL(("wl%d: setCDD_MODE: error \"%s\" (%d) from iovar \"spatial_mode\"\n",
			  unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	return 0;
}
#endif /* APPLE80211_IOC_CDD_MODE */

#ifdef APPLE80211_IOC_THERMAL_THROTTLING
SInt32
AirPort_Brcm43xx::setTHERMAL_THROTTLING( IO80211Interface * interface, struct apple80211_thermal_throttling * tt )
{

	int throttling = 0;
	int err;

	BCM_REFERENCE(interface);
	ASSERT(tt);
	if (!tt)
		return EINVAL;

	if ((revinfo.chipnum != BCM4331_CHIP_ID) &&
	    (revinfo.chipnum != BCM4360_CHIP_ID) &&
	    (revinfo.chipnum != BCM4350_CHIP_ID) &&
	    (revinfo.chipnum != BCM43602_CHIP_ID))
		return EOPNOTSUPP;

	IOVERCHECK(tt->version, "setTHERMAL_THROTTLING");

	WL_IOCTL(("wl%d: setTHERMAL_THROTTLING: # of radio %d state %d %d %d\n", unit,
		tt->num_radios, tt->thermal_state[0], tt->thermal_state[0], tt->thermal_state[0]));

	if (tt->thermal_state[0])
		throttling |= 1;
	if (tt->thermal_state[1])
		throttling |= 2;
	if (tt->thermal_state[2])
		throttling |= 4;

	err = wlIovarOp("pwrthrottle_mask", NULL, 0, &throttling, sizeof(throttling),
		IOV_SET, interface);

	if (err) {
		WL_IOCTL(("wl%d: setTHERMAL_THROTTLING: error \"%s\" (%d) from iovar \"pwrthrottle_mask\"\n",
			unit, bcmerrorstr(err), err));
		return osl_error(err);
	}

	return 0;
}
#endif /* APPLE80211_IOC_THERMAL_THROTTLING */

#ifdef APPLE80211_IOC_REASSOCIATE
SInt32
AirPort_Brcm43xx::setREASSOCIATE( IO80211Interface * interface )
{
	wl_reassoc_params_t reassoc_params;
	wlc_bsscfg_t *cfg = wl_find_bsscfg();
	wlc_bss_info_t * current_bss;

	WL_PORT(("wl%d: %s: entry\n",
	         unit, __FUNCTION__));

	BCM_REFERENCE(interface);
	ASSERT(cfg);
	if (!cfg->associated || interface->linkState() != kIO80211NetworkLinkUp) {
		WL_ERROR(("wl%d: %s: not connected\n", unit, __FUNCTION__));
		return EBUSY;
	}

	current_bss = wlc_get_current_bss(cfg);

	if (current_bss->bss_type != DOT11_BSSTYPE_INFRASTRUCTURE) {
		WL_ERROR(("wl%d: %s: not Infrastructure STA\n", unit, __FUNCTION__));
		return ENXIO;
	}

	/* initialize the reassoc params with the current BSSID and chanspec */
	bzero(&reassoc_params, sizeof(wl_reassoc_params_t));
	BCOPYADRS(&cfg->BSSID, &reassoc_params.bssid);
	reassoc_params.chanspec_num = 1;
	reassoc_params.chanspec_list[0] = wlc_get_home_chanspec(cfg);

	if ( ETHER_ISNULLADDR( &reassoc_params.bssid ))   {
		WL_ERROR(("wl%d: %s: ROAM active current BSSID is NULL\n", unit, __FUNCTION__));
		return EBUSY;
	}


	// Reassoc
	if ( wlIoctl(WLC_REASSOC, &reassoc_params,
	                sizeof(reassoc_params), TRUE, NULL ) != 0 ) {
		WL_ERROR(("wl%d: %s: REASSOC ioctl failed\n", unit, __FUNCTION__));
		return ENXIO;
	}

	return 0;
}
#endif

#if VERSION_MAJOR > 9
SInt32
AirPort_Brcm43xx::setLINK_QUAL_EVENT_PARAMS( OSObject * interface, struct apple80211_link_qual_event_data * lqd )
{
	BCM_REFERENCE(interface);
	ASSERT(lqd);
	if (!lqd)
		return EINVAL;

	IOVERCHECK(lqd->version, "setLINK_QUAL_EVENT_PARAMS");

#if VERSION_MAJOR > 13
	WL_IOCTL(("wl%d: setLINK_QUAL_EVENT_PARAMS: request for link_qual indication "
	          "rssi_divisor:%d, tx_rate_divisor:%d, min_interval: %d, hysteresis:%d\n",
	          unit, lqd->rssi_divisor, lqd->tx_rate_divisor, lqd->min_interval, lqd->hysteresis));
#else
	WL_IOCTL(("wl%d: setLINK_QUAL_EVENT_PARAMS: request for link_qual indication "
	          "rssi_divisor:%d, tx_rate_divisor:%d, min_interval: %d\n",
	          unit, lqd->rssi_divisor, lqd->tx_rate_divisor, lqd->min_interval));
#endif

	if (lqd->rssi_divisor)
		link_qual_config.rssi_divisor = lqd->rssi_divisor;
	if (lqd->tx_rate_divisor)
		link_qual_config.tx_rate_divisor = lqd->tx_rate_divisor;
	if (lqd->min_interval)
		link_qual_config.min_interval = lqd->min_interval;
#if VERSION_MAJOR > 13
	if (lqd->hysteresis)
		link_qual_config.hysteresis = lqd->hysteresis;

	// Recalculate RSSI and Tx Rate hysteresis
	rssi_hysteresis = MAX_RSSI * link_qual_config.hysteresis / link_qual_config.rssi_divisor / 100;
	tx_rate_hysteresis = max_txrate * link_qual_config.hysteresis / link_qual_config.tx_rate_divisor / 100;
#endif

	// Clear the current history based on old bucket parameters
	linkQualHistoryReset();

	return 0;
}

SInt32
AirPort_Brcm43xx::setVENDOR_DBG_FLAGS( OSObject * interface, struct apple80211_vendor_dbg_data * vdd )
{
	BCM_REFERENCE(interface);
#ifdef WLEXTLOG
	wlc_extlog_cfg_t w_cfg = {};
	int err = 0;

	ASSERT(vdd);
	if (!vdd)
		return EINVAL;

	IOVERCHECK(vdd->version, "setVENDOR_DBG_FLAGS");

	WL_IOCTL(("wl%d: setVENDOR_DBG_FLAGS\n",
	          unit));

	bzero(&w_cfg, sizeof(wlc_extlog_cfg_t));

	// Right now both the flags are mapped to LOG_MODULE_ASSOC
	if (isset(vdd->dbg_flags_map, APPLE80211_V_DBG_FLAG_ASSOC) &&
	    !isset(dbg_flags_map, APPLE80211_V_DBG_FLAG_ASSOC)) {
		WL_IOCTL(("wl%d: ASSOC flag set\n",
		          unit));
		setbit(dbg_flags_map, APPLE80211_V_DBG_FLAG_ASSOC);
		w_cfg.module = LOG_MODULE_ASSOC;
	}

	if (isset(vdd->dbg_flags_map, APPLE80211_V_DBG_FLAG_CONNECTION) &&
	    !isset(dbg_flags_map, APPLE80211_V_DBG_FLAG_CONNECTION)) {
		WL_IOCTL(("wl%d: CONNECTION flag set\n",
		          unit));
		setbit(dbg_flags_map, APPLE80211_V_DBG_FLAG_CONNECTION);
		w_cfg.module = LOG_MODULE_ASSOC;
	}

	if (isclr(vdd->dbg_flags_map, APPLE80211_V_DBG_FLAG_ASSOC) &&
	    isset(dbg_flags_map, APPLE80211_V_DBG_FLAG_ASSOC)) {
		WL_IOCTL(("wl%d: ASSOC flag clear\n",
		          unit));
		clrbit(dbg_flags_map, APPLE80211_V_DBG_FLAG_ASSOC);
	}

	if (isclr(vdd->dbg_flags_map, APPLE80211_V_DBG_FLAG_CONNECTION) &&
	    isset(dbg_flags_map, APPLE80211_V_DBG_FLAG_CONNECTION)) {
		WL_IOCTL(("wl%d: CONNECTION flag clear\n",
		          unit));
		clrbit(dbg_flags_map, APPLE80211_V_DBG_FLAG_CONNECTION);
	}

	w_cfg.flag = LOG_FLAG_EVENT;
	w_cfg.level = WL_LOG_LEVEL_ERR;

	err = wlIovarOp("extlog_cfg", NULL, 0, &w_cfg, sizeof(wlc_extlog_cfg_t), IOV_SET, NULL);
	if (err) {
		WL_ERROR(("wl%d: extlog cfg failed: error \"%s\" (%d)\n",
		          unit, bcmerrorstr(err), err));
	}

	return osl_error( err );
#endif
	return 0;
}
#ifdef FACTORY_MERGE
SInt32
AirPort_Brcm43xx::setFACTORY_MODE( IO80211Interface * interface,struct apple80211_factory_mode_data * fmd )
{
	int err = BCME_OK;

	BCM_REFERENCE(interface);
	ASSERT(fmd);
	if (!fmd)
		return EINVAL;

	IOVERCHECK(fmd->version, "setFACTORY_MODE");

	WL_IOCTL(("wl%d: setFACTORY_MODE: PowerSave= %s CountryAdopt= %s Roam =%s\n", unit,
		  (fmd->powersave == APPLE80211_FACTORY_MODE_OFF)   ? "OFF":"ON",
		  (fmd->countryadopt == APPLE80211_FACTORY_MODE_OFF)? "OFF":"ON",
		  (fmd->roam == APPLE80211_FACTORY_MODE_OFF)        ? "OFF":"ON"));

	switch(fmd->powersave)
	{
		case APPLE80211_FACTORY_MODE_OFF:

			/// Check current state:
			/// If disableFactoryPowersave == FALSE (powersave on), and requested wants OFF, then
			///   disableFactoryPowersave = TRUE (powersave off)
			///
			if (!disableFactoryPowersave)
			{
				disableFactoryPowersave = TRUE;
				err |= wlIoctlSetInt(WLC_SET_PM, PM_OFF);

				//Don't update cached PM state here as we want to remember the previous setting before entering factory mode
			}

			break;
		case APPLE80211_FACTORY_MODE_ON:

			/// Check current state:
			/// If disableFactoryPowersave == TRUE (powersave off), and requested wants ON, then
			///   disableFactoryPowersave = FALSE (powersave on)
			///
			if (disableFactoryPowersave)
			{
				disableFactoryPowersave = FALSE;

				// Go back to previous powersave state before entering factory mode
				err |= wlIoctlSetInt(WLC_SET_PM, cached_PM_state);
			}

			break;
		default:
			err |=ENXIO;
			break;
	}
	switch(fmd->countryadopt)
	{
		case APPLE80211_FACTORY_MODE_OFF:

			/// Check current state:
			/// If disableFactory11d == FALSE (country adopt enabled), and requested wants OFF, then
			///   disableFactory11d = TRUE (country adopt disabled)
			///
			if (!disableFactory11d)
			{
				disableFactory11d = TRUE;

				//Toggle the interface so that this always takes into effect
				wl_down( (struct wl_info *)this );
				err |= setAutoCountry(FALSE);
				err |= wl_up( (struct wl_info *)this );

			}

			break;
		case APPLE80211_FACTORY_MODE_ON:

			/// Check current state:
			/// If disableFactory11d == TRUE (country adopt disabled), and requested wants ON, then
			///   disableFactory11d = FALSE (country adopt enabled)
			///
			if(disableFactory11d)
			{
				disableFactory11d = FALSE;

				//Toggle the interface so that this always takes into effect
				wl_down( (struct wl_info *)this );
				err |= setAutoCountry(TRUE);
				err |= wl_up( (struct wl_info *)this );
			}

			break;
		default:
			err |=ENXIO;
			break;
	}

	switch(fmd->roam)
	{
		case APPLE80211_FACTORY_MODE_OFF:

			/// Check current state:
			/// If disableFactoryRoam == FALSE (roam enabled), and requested wants OFF, then
			///   disableFactoryRoam = TRUE (roam disabled)
			///
			if (!disableFactoryRoam)
			{
				disableFactoryRoam = TRUE;
				err |= wlIovarSetInt("roam_off", TRUE);
			}
			break;

		case APPLE80211_FACTORY_MODE_ON:

			/// Check current state:
			/// If disableFactoryRoam == TRUE (roam disabled), and requested wants ON, then
			///   disableFactoryRoam = FALSE (roam enable)
			///
			if (disableFactoryRoam)
			{
				disableFactoryRoam = FALSE;
				err |= wlIovarSetInt("roam_off", FALSE);
			}
			break;

		default:
			err |=ENXIO;
			break;
	}


	return !err ? 0 : ENXIO;

}
#endif
// Apple change: Move setBTCOEX_MODE to be with the rest of the ioctl set handlers
SInt32
AirPort_Brcm43xx::setBTCOEX_MODE( OSObject * interface, struct apple80211_btc_mode_data * btcdata )
{
	int err;

	BCM_REFERENCE(interface);
	ASSERT(btcdata);
	if (!btcdata)
		return EINVAL;

	int brcmCoexMode;

	switch( btcdata->mode )
	{
		case APPLE80211_BTCOEX_MODE_OFF:
			brcmCoexMode = WL_BTC_DISABLE;
			break;
		case APPLE80211_BTCOEX_MODE_ON:
			brcmCoexMode = WL_BTC_ENABLE;
			break;
		case APPLE80211_BTCOEX_MODE_DEFAULT:
			// Could do CPU-specific check here
			brcmCoexMode = WL_BTC_DEFAULT;
			break;
/* APPLE MODIFICATION <cbz@apple.com> add new enumerated values; APPLE80211_BTCOEX_MODE_FULL_TDM - APPLE80211_BTCOEX_MODE_HYBRID */
#if VERSION_MAJOR > 10
		case APPLE80211_BTCOEX_MODE_FULL_TDM:
			brcmCoexMode = WL_BTC_FULLTDM;
			break;
		case APPLE80211_BTCOEX_MODE_FULL_TDM_PREEMPTION:
			brcmCoexMode = WL_BTC_PREMPT;
			break;
		case APPLE80211_BTCOEX_MODE_LIGHTWEIGHT:
			brcmCoexMode = WL_BTC_LITE;
			break;
		case APPLE80211_BTCOEX_MODE_UNIQUE_ANTENNAE:
			brcmCoexMode = WL_BTC_PARALLEL;
			break;
		case APPLE80211_BTCOEX_MODE_HYBRID:
			brcmCoexMode = WL_BTC_HYBRID;
			break;
#endif
		default:
			return EINVAL;
	}

/* APPLE MODIFICATION <rdar://problem/9127736> - <cbz@apple.com>  - seriously...set it the way I tell you to set it on Lion */
#if VERSION_MAJOR < 11
	/* Enabling BTC for X16 puts it in hybrid coex algorithm mode */
	if (brcmCoexMode == WL_BTC_ENABLE && revinfo.chipnum == BCM43224_CHIP_ID)
		brcmCoexMode = WL_BTC_DEFAULT;

	/* Enabling BTC for X19 in hybrid coex algorithm mode
	 * VERSION_MAJOR 10 or less is SL, this X28 is not supported
	 */
	if (revinfo.chipnum == BCM4331_CHIP_ID) {
		if (brcmCoexMode != WL_BTC_DISABLE) {
			if ((revinfo.boardid == BCM94331X28) ||
			    (revinfo.boardid == BCM94331X28B) ||
			    (revinfo.boardid == BCM94331X29B) ||
			    (revinfo.boardid == BCM94331X29D))
				brcmCoexMode = WL_BTC_LITE;
			else if ((revinfo.boardid == BCM94331X19) ||
				 (revinfo.boardid == BCM94331X19C) ||
				 (revinfo.boardid == BCM94331X33))
				brcmCoexMode = WL_BTC_HYBRID;
		} else {
			brcmCoexMode = WL_BTC_DISABLE;
		}
	}
#endif

#ifdef AAPLPWROFF
	if( _powerState == PS_INDEX_ON && !_powerSleep )
	{
		err = wlIovarSetInt("btc_mode", brcmCoexMode);
	}
	else
	{
		// Since the desired mode will be cached and applied later when the device
		// powers on, this is not an error condition.
		// Apple Change
		err = BCME_OK;
	}
#else
	err = wlIovarSetInt("btc_mode", brcmCoexMode);
#endif

	return osl_error(err);
}

#endif /* VERSION_MAJOR > 9 */
#endif /* !MACOSX_DHD */
void
AirPort_Brcm43xx::allocAssocStaIEList(struct ether_addr *addr, void *data, UInt32 datalen)
{
	sta_ie_list_t *elt = NULL;
	char eabuf[ETHER_ADDR_STR_LEN] = {0};

	if (datalen == 0) {
		return;
	}

	if ((addr == NULL) || (data == NULL)) {
		WL_ERROR(("%s: addr(%p) or data(%p) is null\n",
		          __FUNCTION__, OSL_OBFUSCATE_BUF(addr),
				OSL_OBFUSCATE_BUF(data)));
		return;
	}

	bcm_ether_ntoa(addr, eabuf);

	// remove duplicate entries
	freeAssocStaIEList(addr, FALSE);

	// allocate and add new entry to list
	if ((elt = (sta_ie_list_t *)MALLOC(osh, sizeof(sta_ie_list_t) + datalen)) == NULL) {
		WL_ERROR(("%s: MALLOC failed for %s\n", __FUNCTION__, eabuf));
		return;
	}

	// initialize newly allocated entry
	elt->tlv = (bcm_tlv_t *)((uint8 *)elt + sizeof(sta_ie_list_t));
	elt->tlvlen = datalen;
	BCOPYADRS(addr, &elt->addr);
	bcopy(data, (void *)elt->tlv, datalen);

	// insert entry into list
	SLIST_INSERT_HEAD(&assocStaIEList, elt, entries);
}

void
AirPort_Brcm43xx::freeAssocStaIEList(struct ether_addr *addr, bool deleteAll)
{
	sta_ie_list_t *elt = NULL;
	bool found = 0;
	char eabuf[ETHER_ADDR_STR_LEN] = {0};

	if ((addr == NULL) && !deleteAll) {
		WL_ERROR(("%s: addr is null\n", __FUNCTION__));
		return;
	}

	// remove duplicate entries
	do {
		found = FALSE;
		SLIST_FOREACH(elt, &assocStaIEList, entries) {
			if (deleteAll || (BCMPADRS(addr, &elt->addr) == 0)) {
				found = TRUE;
				break;
			}
		}
		if (found) {
			SLIST_REMOVE(&assocStaIEList, elt, sta_ie_list, entries);
			bcm_ether_ntoa(&elt->addr, eabuf);
			WL_PORT(("%s: entry %p found for %s, removed\n",
			         __FUNCTION__, OSL_OBFUSCATE_BUF(elt), eabuf));
			MFREE(osh, elt, sizeof(sta_ie_list_t) + elt->tlvlen);
		}
	} while (found);
}
#ifndef MACOSX_DHD
#ifdef APPLE80211_IOC_TX_CHAIN_POWER
SInt32
AirPort_Brcm43xx::setTX_CHAIN_POWER( OSObject * interface, struct apple80211_chain_power_data_set * txcpd )
{
	int i = 0;
	int txcpd_power_offset[WLC_CHAN_NUM_TXCHAIN] = {0};

	BCM_REFERENCE(interface);

	ASSERT(txcpd);
	if (!txcpd)
		return EINVAL;

	IOVERCHECK(txcpd->version, "setTX_CHAIN_POWER");

	if (!PHYTYPE_HT_CAP(pWLC->band))
		return EOPNOTSUPP;

	if (txcpd->num_chains != (int32_t)num_radios)
		return EINVAL;

	for (i = 0; i < txcpd->num_chains; i++) {
		if (txcpd->powers[i].power_offset > 0 ||
		    txcpd->powers[i].power_offset < (SCHAR_MIN / WLC_TXPWR_DB_FACTOR)) {
			WL_PORT(("wl%d: %s: power_offset %d out of range\n",
			         unit, __FUNCTION__, txcpd->powers[i].power_offset));
			return EINVAL;
		}
		txcpd_power_offset[i] = txcpd->powers[i].power_offset;
	}

	for (; i < WLC_CHAN_NUM_TXCHAIN; i++) {
		txcpd_power_offset[i] = 0;
	}

	/*
	*	Evaluate the need to do a olpc cal.
	*	Get the tssivisi threshold
	*/
	{
		int8 band_max_power = 0;
		int8 band_min_power = 0;
		int maxoffset = 0;
		int band, nBand, err, visi_thresh;

		err = wlIovarGetInt("tssivisi_thresh", &visi_thresh);
		if (err) {
			return EINVAL;
		}
		for (i = 0; i < txcpd->num_chains; i++) {
			maxoffset = MAX(maxoffset, txcpd_power_offset[i]);
		}
		for (nBand = 0; nBand < 2; nBand++) {
			band = (nBand == 0) ? WLC_BAND_2G : WLC_BAND_5G;
			band_max_power = wlc_channel_max_tx_power_for_band(pWLC, band, &band_min_power);
			if ((band_min_power - maxoffset - visi_thresh) < 0) {
			       /* start an olpc cal */
				wlIovarSetInt("olpc_chan_override", TRUE);
			} else {
				wlIovarSetInt("olpc_chan_override", FALSE);
			}
			wlc_channel_apply_band_chain_limits(pWLC, band, band_max_power,
			       txcpd->num_chains, &txcpd_power_offset[0], &tx_chain_offset[0]);
		}
	}

	return 0;
}
#endif /* APPLE80211_IOC_TX_CHAIN_POWER */

#if VERSION_MAJOR > 10
#ifdef APPLE80211_IOC_DESENSE
SInt32
AirPort_Brcm43xx::setDESENSE( OSObject * interface, struct apple80211_desense_data * dd )
{
	int err = BCME_OK;
	int32 flags = 0;
	uint32 threshold = 0;

	BCM_REFERENCE(interface);
	ASSERT(dd);
	if (!dd)
		return EINVAL;

	IOVERCHECK(dd->version, "setDESENSE");

	if (dd->flags & APPLE80211_DESENSE_ENABLE_2DOT4GHZ)
		flags |= BTC_RXGAIN_FORCE_2G_ON;
	if (dd->flags & APPLE80211_DESENSE_ENABLE_5GHZ)
		flags |= BTC_RXGAIN_FORCE_5G_ON;
	err |= wlIovarSetInt("btc_rxgain_force", flags);

	/* Set threshold */
	threshold = ((uint32)((dd->highRssi2dot4GHz * -1) & 0xFF)) |
	            ((uint32)((dd->lowRssi2dot4GHz * -1) & 0xFF) << 8) |
	            ((uint32)((dd->highRssi5GHz * -1) & 0xFF) << 16) |
	            ((uint32)((dd->lowRssi5GHz * -1) & 0xFF) << 24);
	err |= wlIovarSetInt("btc_rxgain_thresh", threshold);

	if (err)
		return EINVAL;

	return 0;
}

SInt32
AirPort_Brcm43xx::getDESENSE( OSObject * interface, struct apple80211_desense_data * dd )
{
	int32 flags = 0;
	int32 threshold = 0;

	BCM_REFERENCE(interface);
	ASSERT(dd);
	if (!dd)
		return EINVAL;

	IOVERCHECK(dd->version, "getDESENSE");

	dd->flags = 0;
	(void) wlIovarGetInt("btc_rxgain_force", &flags);
	if (flags & BTC_RXGAIN_FORCE_2G_ON)
		dd->flags |= APPLE80211_DESENSE_ENABLE_2DOT4GHZ;
	if (flags & BTC_RXGAIN_FORCE_5G_ON)
		dd->flags |= APPLE80211_DESENSE_ENABLE_5GHZ;

	/* Get threshold */
	(void) wlIovarGetInt("btc_rxgain_thresh", &threshold);
	dd->highRssi2dot4GHz = ((int16_t)(threshold & 0xFF)) * -1;
	dd->lowRssi2dot4GHz = ((int16_t)((threshold >> 8) & 0xFF)) * -1;
	dd->highRssi5GHz = ((int16_t)((threshold >> 16) & 0xFF)) * -1;
	dd->lowRssi5GHz = ((int16_t)((threshold >> 24) & 0xFF)) * -1;

	return 0;
}
#endif /* APPLE80211_IOC_DESENSE */

#ifdef APPLE80211_IOC_CHAIN_ACK
SInt32
AirPort_Brcm43xx::setCHAIN_ACK( OSObject * interface, struct apple80211_chain_ack * ca )
{
	int32 chain_ack = 0;
	uint32 i = 0;
	int err = BCME_OK;

	BCM_REFERENCE(interface);
	ASSERT(ca);
	if (!ca)
		return EINVAL;

	IOVERCHECK(ca->version, "setCHAIN_ACK");

	if (ca->num_radios > (u_int32_t)num_radios)
		return EINVAL;

	for (i = 0; i < ca->num_radios; i++) {
		if (ca->chain_ack[i] == APPLE80211_CHAIN_ACK_ON)
			chain_ack |= 1 << i;
	}

	if ((err = wlIovarSetInt("btc_siso_ack", chain_ack)) != 0)
		return EOPNOTSUPP;

	return 0;
}

SInt32
AirPort_Brcm43xx::getCHAIN_ACK( OSObject * interface, struct apple80211_chain_ack * ca )
{
	uint32 i = 0;
	int32 chain = 0;
	int32 chain_ack = 0;
	uint err = BCME_OK;
	BCM_REFERENCE(interface);
	ASSERT(ca);
	if (!ca)
		return EINVAL;

	IOVERCHECK(ca->version, "getCHAIN_ACK");

	if ((err = wlIovarGetInt("btc_siso_ack", &chain_ack)) != 0)
	        return EOPNOTSUPP;

	if ((err = wlIovarGetInt("hw_txchain", &chain)) != 0)
		return EOPNOTSUPP;

	chain_ack &= chain;

	ca->num_radios = num_radios;
	for (i = 0; i < ca->num_radios; i++) {
		ca->chain_ack[i] = APPLE80211_CHAIN_ACK_OFF;
		if (chain_ack & (1u << i))
			ca->chain_ack[i] = APPLE80211_CHAIN_ACK_ON;
	}
	return 0;
}
#endif /* APPLE80211_IOC_CHAIN_ACK */

#ifdef OPPORTUNISTIC_ROAM
SInt32
AirPort_Brcm43xx::setROAM( OSObject * interface, struct apple80211_sta_roam_data * rd )
{
	int err = BCME_OK;
	ASSERT(rd);
	if (!rd)
		return EINVAL;

	IOVERCHECK(rd->version, "setROAM");

	struct wlc_if * wlcif = wlLookupWLCIf( interface );
	wlc_bsscfg_t *bsscfg = wl_find_bsscfg(wlcif);
	wlc_roam_t *roam = bsscfg->roam;

	if (!memcmp(rd->bssid.octet, BSSID_INVALID, sizeof(rd->bssid.octet))) {
		WL_ASSOC(("%s(): INVALID target BSSID!\n", __FUNCTION__));
	}
	WL_ASSOC(("%s(): bsscfg%d oppr_roam_off %d\n", __FUNCTION__, WLC_BSSCFG_IDX(bsscfg), roam->oppr_roam_off));

	if (bsscfg->associated && !BSSCFG_AP(bsscfg) && !roam->oppr_roam_off) {
		if(bsscfg->assoc->type == AS_ROAM) {
			WL_ASSOC(("%s(): Roaming is already going on\n", __FUNCTION__));
			return EBUSY;
		}
		memcpy(bsscfg->join_bssid.octet, rd->bssid.octet, sizeof(bsscfg->join_bssid.octet));
		err = wlc_roamscan_start(bsscfg, WLC_E_REASON_BETTER_AP);
		if (err < 0)  {
			WL_ASSOC(("%s(): Cannot start roam scan at this time, %d\n", __FUNCTION__, err));
			return EBUSY;
		}
	} else {
		return EINVAL;
	}

	return 0;
}
#endif /* OPPORTUNISTIC_ROAM */

#ifdef IO80211P2P
SInt32
AirPort_Brcm43xx::setSCANCACHE_CLEAR( IO80211Interface * interface )
{
	(void) wlIovarOp("scancache_clear", NULL, 0, NULL, 0, IOV_SET, interface);
	return 0;
}
#endif // IO80211P2P
#endif // VERSION_MAJOR > 10

SInt32
AirPort_Brcm43xx::performCountryCodeOperation( IO80211Interface * interface, IO80211CountryCodeOp op )
{
	int autocountry = 0;
	int bcmerr = 0;

	BCM_REFERENCE(interface);
	WL_IOCTL(("wl%d: performCountryCodeOperation: op 0x%x\n",
	          unit, op));

#ifdef AAPLPWROFF
	// check for power off state.  Return error if power off
	if (_powerState != PS_INDEX_ON) {
		WL_IOCTL(("wl%d: %s called while _powerState is %ld\n",
		          unit, __FUNCTION__, _powerState));
		return EPWROFF;
	}
#endif

	if (op != kIO80211CountryCodeReset)
		return EOPNOTSUPP;

	// handle kIO80211CountryCodeReset

	(void) wlIovarGetInt("autocountry", &autocountry);
	if (autocountry == 0) {
		WL_IOCTL(("wl%d: performCountryCodeOperation: AutoCountry not enabled, "
		          "skipping CountryCodeReset operation\n",
		          unit));
	} else {

		/* Since driver is up right now, defer the operation */
		if (pWLC->pub->up) {
			AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx*)pWLC->wl;

			WL_PORT(("wl%d: performCountryCodeOperation: Not Down, "
			          "deferring CountryCodeReset operation\n",
			          unit));

			wlobj->resetCountryCode = TRUE;
			return 0;
		}
#ifdef FACTORY_BUILD
		bcmerr = setAutoCountry(FALSE);
#else
#ifdef FACTORY_MERGE
		if (disableFactory11d)
			bcmerr = setAutoCountry(FALSE);
		else
			bcmerr = setAutoCountry(TRUE);

#else
		bcmerr = setAutoCountry(TRUE);
#endif
#endif
	}

	if (bcmerr)
		WL_ERROR(("wl%d: performCountryCodeOperation: bcmerror \"%s\" (%d)\n",
		          unit, bcmerrorstr(bcmerr), bcmerr));

	return osl_error(bcmerr);
}
#endif /* !MACOSX_DHD */
void
AirPort_Brcm43xx::phySubmodeDisassocReset()
{
	if (phy_submode == APPLE80211_SUBMODE_11N_LEGACY ||
	    phy_submode == APPLE80211_SUBMODE_11N_LEGACY_DUP ||
	    phy_submode == APPLE80211_SUBMODE_11N_HT ||
	    phy_submode == APPLE80211_SUBMODE_11N_HT_DUP) {
		rateOverrideReset();
		(void) wlIovarSetInt("ofdm_txbw", AUTO);
		(void) wlIovarSetInt("mimo_txbw", AUTO);
		phy_submode = APPLE80211_SUBMODE_11N_AUTO;
	}
}

bool
AirPort_Brcm43xx::setLinkState( IO80211LinkState linkState , UInt32 reason)
{
	IO80211Interface * interface = getNetworkInterface();

	KERNEL_DEBUG_CONSTANT(WiFi_apple80211Request_setLinkState | DBG_FUNC_NONE, linkState, reason, 0, 0, 0);

	if( interface ) {
#if VERSION_MAJOR > 11
		// setLinkQualityMetric should be called in the code dynamically depending on the wireless link quality,
		//   put this here for now, but this should be fixed probably in a watchdog timer for link metric.
		interface->setLinkQualityMetric( (linkState == kIO80211NetworkLinkUp) ?
								kIO80211NetworkLinkQualityMetricGood : kIO80211NetworkLinkQualityMetricOff );
#endif /* VERSION_MAJOR > 11 */
#if VERSION_MAJOR > 9
		return interface->setLinkState( linkState , reason);
#else
		return interface->setLinkState( linkState );
#endif
	}
	return FALSE;
}
#ifndef MACOSX_DHD
SInt32
AirPort_Brcm43xx::appendProhibited_channels(wl_uint32_list_t *list)
{
	int chanspec_count = 0;
	int i;

	ASSERT(list);
	chanspec_t chanspec_list[MAXCHANNEL] = {};
	chanspec_count = wlc_scan_prohibited_channels_get(pWLC->scan, chanspec_list, MAXCHANNEL);
	if ((list->count+chanspec_count) > WL_NUMCHANSPECS) {
		WL_ERROR(("%s: Channel List too long (%d > %d[max])\n", __FUNCTION__,
				(list->count+chanspec_count), WL_NUMCHANSPECS));
		return -1;
	}

	for (i = 0; i < chanspec_count; i++) {
		list->element[list->count++] = chanspec_list[i];
	}
	return chanspec_count;
}
#endif /* !MACOSX_DHD */
// This is common scan code called by both setSCAN_REQ and setSCAN_REQ_MULTIPLE
SInt32
AirPort_Brcm43xx::scanreq_common(uint32 bss_type, uint32 scan_type, uint32 dwell_time, uint32 rest_time,
                                 uint32 phy_mode, uint32 num_channels, struct apple80211_channel *channels,
                                 int *bssType, int *scanType, int *activeTime, int *passiveTime, int *homeTime,
                                 chanspec_t *chanspec_list, int *chanspec_count, uint *scanFlags)
{
	int i = 0;
	uint16 restrict_band = 0;
	uint32 mode_2_4 = 0;
	uint32 mode_5 = 0;

	// APPLE: Issue with M93 scanning time so set Dwell time to low value
#define APPLE_PASSIVE_DWELL_TIME 110 // Apple can change this to 110 if needed
	*passiveTime = APPLE_PASSIVE_DWELL_TIME;

	// Figure out bssType
	switch( bss_type ) {
		case APPLE80211_AP_MODE_IBSS:
			*bssType = DOT11_BSSTYPE_INDEPENDENT;
			break;
		case APPLE80211_AP_MODE_INFRA:
			*bssType = DOT11_BSSTYPE_INFRASTRUCTURE;
			break;
		case APPLE80211_AP_MODE_ANY:
			*bssType = DOT11_BSSTYPE_ANY;
			break;
		default:
			return EINVAL;
	}

	switch( scan_type ) {
		case APPLE80211_SCAN_TYPE_FAST:
			break;
		case APPLE80211_SCAN_TYPE_ACTIVE:
			*scanType = DOT11_SCANTYPE_ACTIVE;
			if( dwell_time )
				*activeTime = (int)dwell_time;
			break;
		case APPLE80211_SCAN_TYPE_PASSIVE:
			*scanType = DOT11_SCANTYPE_PASSIVE;
			if( dwell_time )
				*passiveTime = (int)dwell_time;
			break;
		default:
			return EINVAL;
	}

	if( rest_time > 20 )
		*homeTime = (int)rest_time;

	// Convert the channel list from apple80211 form to chanspec list
	for (i = 0;
	     i < (int)num_channels && i <  min( APPLE80211_MAX_CHANNELS, MAXCHANNEL ); i++)
		chanspec_list[i] = CH20MHZ_CHSPEC(channels[i].channel);
	*chanspec_count = i;

	// Save phy mode for scan results filtering
	scan_phy_mode = phy_mode;

	scanFreeResults();
	scan_done = 0;
#ifndef MACOSX_DHD
	if (WLC_CNTRY_DEFAULT_ENAB(pWLC) && isCntryDefaultSupported(revinfo.boardid)) {
		*scanFlags |= WL_SCANFLAGS_PROHIBITED;
	}
#endif /* !MACOSX_DHD */
	// Check if scan phy_mode restricts the bands to scan.
	// There is a band restriction if the phy_mode has only band specific modes
	// of one band.
	restrict_band = 0;
	mode_2_4 = phy_mode & (APPLE80211_MODE_11B |
	                           APPLE80211_MODE_11G |
	                           APPLE80211_MODE_TURBO_G);
	mode_5 = phy_mode & (APPLE80211_MODE_11A |
	                         APPLE80211_MODE_TURBO_A);

	if (mode_2_4 && phy_mode == mode_2_4)
		restrict_band = WLC_BAND_2G;
	else if (mode_5 && phy_mode == mode_5)
		restrict_band = WLC_BAND_5G;
#ifndef MACOSX_DHD
	// Create a channel list for filtering if there is a band restriction
	// and no channel list has been specified.
	if (restrict_band && *chanspec_count == 0)
		wlc_scan_default_channels(pWLC->scan,
			wf_chspec_ctlchspec(pWLC->home_chanspec),
			restrict_band, chanspec_list, chanspec_count);

#endif /* !MACOSX_DHD */
	return 0;
}

#if VERSION_MAJOR > 8
void
AirPort_Brcm43xx::postMessage( int msg, void * data, size_t dataLen )
#else
void
AirPort_Brcm43xx::postMessage(int msg)
#endif
{
	IO80211Interface * interface = getNetworkInterface();

	if (interface) {
		WL_IOCTL(("wl%d: postMessage(%s)\n",
		          unit, lookup_name(apple80211_msgnames, msg)));
#if VERSION_MAJOR > 8
		interface->postMessage(msg, data, dataLen);
#else
		interface->postMessage(msg);
#endif
	} else {
		WL_IOCTL(("wl%d: postMessage(%s), ERROR interface = NULL\n",
		          unit, lookup_name(apple80211_msgnames, msg)));
	}
}

#if VERSION_MAJOR < 10
#define IO8Log printf
#endif
#ifndef MACOSX_DHD
void
AirPort_Brcm43xx::wlEvent( char *ifname, wlc_event_t *evt )
{
	uint event_type = evt->event.event_type;
	struct ether_addr* addr = evt->addr;
	uint reason = evt->event.reason;
	uint status = evt->event.status;
	wlc_bsscfg_t *bsscfg = NULL;
	struct wlc_if *wlcif = NULL;
	intf_info_blk_t *blk = IFINFOBLKP(myNetworkIF);
#if defined(MACOS_VIRTIF) || defined(IO80211P2P)
	int len = strlen(ifname);
#endif /* MACOS_VIRTIF || IO80211P2P */

#ifdef ENABLE_CORECAPTURE
	bool needCoreCapture = false;
	char reason_string[CC_REASON_STR_LEN];
#endif /* ENABLE_CORECAPTURE */

	WL_PORT(("%s: %s %s Event %d\n", __FUNCTION__, wl_ifname((wl_info*)this, NULL), ifname,
	         event_type));

#ifdef MACOS_VIRTIF
	bool virtif = FALSE;
	if (strncmp(wl_ifname((wl_info*)this, NULL), ifname, len) != 0) {
		if (apif.virtIf && strncmp(wl_ifname((wl_info*)this, &apif),
		                        ifname, len) == 0) {
			wlcif = apif.wlcif;
			virtif = TRUE;
		}
	}
#endif
#ifdef IO80211P2P
	AirPort_Brcm43xxP2PInterface *virtIf = NULL;

	/* IF event, though it contains name of the virtual if, is always meant for main if */
	if ((event_type != WLC_E_IF) &&
	    (strncmp(wl_ifname((wl_info*)this, NULL), ifname, len) != 0)) {
		struct virt_intf_entry *elt = NULL;
		SLIST_FOREACH(elt, &virt_intf_list, entries) {
			if (strncmp(wl_ifname((wl_info*)this, (struct wl_if *)elt->obj),
			         ifname, len) == 0)
			{
				virtIf = elt->obj;
				wlcif = virtIf->wlcif;
				blk = IFINFOBLKP(virtIf);
				break;
			}
		}
	}
#endif

	bsscfg = wl_find_bsscfg(wlcif);
	ASSERT(bsscfg != NULL);

	switch (event_type) {
	case WLC_E_LINK: {
		bool link_up = (evt->event.flags & WLC_EVENT_MSG_LINK) ? TRUE : FALSE;

#ifdef MACOS_VIRTIF
#if defined(BCMDBG)
		IOLog("%s: %s %s Link %s\n", __FUNCTION__, wl_ifname((wl_info*)this, NULL), ifname,
		      (evt->event.flags&WLC_EVENT_MSG_LINK)?"UP":"DOWN");
#endif /* BCMDBG */
		if (virtif) {
			WL_UNLOCK();
			apif.virtIf->inputEvent(link_up ?
			                        kIONetworkEventTypeLinkUp: kIONetworkEventTypeLinkDown, NULL);
			WL_LOCK();
			break;
		}
#endif
#ifdef IO80211P2P
#if defined(BCMDBG)
		IOLog("%s: %s %s Link %s\n", __FUNCTION__, wl_ifname((wl_info*)this, NULL), ifname,
		      (evt->event.flags&WLC_EVENT_MSG_LINK)?"UP":"DOWN");
#endif /* BCMDBG */
		if (virtIf) {
			WL_UNLOCK();
			virtIf->setLinkState((evt->event.flags&WLC_EVENT_MSG_LINK) ?
			                     kIO80211NetworkLinkUp: kIO80211NetworkLinkDown );
			WL_LOCK();
#if VERSION_MAJOR > 11
		// post a APPLE80211_M_CHANNEL_SWITCH event to Family so it re-enables the AWDL interface from a Channel
		// Switch Annoucement
		if (link_up) {
				apple80211_channel_switch_anoucement_t csa;
				bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
				csa.complete = APPLE80211_CSA_SUCCESS;
				postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
				// pWLC->csa_state_join=0;
		}
#endif

			break;
		}
#endif /* IO80211P2P */

		IO80211LinkState linkState = link_up ?
		        kIO80211NetworkLinkUp : kIO80211NetworkLinkDown;
		UInt32 reason = DOT11_RC_INACTIVITY; // Default RC code

		if ( linkState == kIO80211NetworkLinkUp )
		{
			// turn off framebursting for b-only
			UInt32  maxRate = 0;
			// turn off framebursting for b-only
                        if (bsscfg->current_bss) {
				maxRate = wlc_get_current_highest_rate(bsscfg);
			} else {
				maxRate = 0;
                        /* Should not be NULL. Crash debug images to investigate. */
                                ASSERT(0);
                        }
			blk->link_deauth_reason = 0;
			reason = 0;
			maxRate = RSPEC2MBPS(maxRate);
			if (maxRate != 0)		// don't change on error
				(void)wlIoctlSetInt(WLC_SET_FAKEFRAG, (maxRate > 108) ? 1 : 0 );
#if VERSION_MAJOR > 9
			linkQualHistoryUpd(maxRate);
#endif
		} else {
#if defined(STA) && !defined(WLTEST)
			/// Don't reset the country code if WLTEST is defined (MFG configuration)
			/* Reset the country to default for link down only if primary intf is STA*/
			if (WL11H_ENAB(pWLC) && !bsscfg->_ap && (virtIf == NULL) &&
#if defined(FACTORY_MERGE)
			    (disableFactory11d == FALSE) &&
#endif
			    TRUE) {
				if (!isCntryDefaultSupported(revinfo.boardid)) {
				wlc_set_countrycode(pWLC->cmi, wlc_channel_sromccode(pWLC->cmi));
				}
#ifdef CNTRY_DEFAULT
				else {
					char country[WLC_CNTRY_BUF_SZ] = {0};
					wlIoctl(WLC_GET_COUNTRY, country, WLC_CNTRY_BUF_SZ, FALSE, NULL);
					resetAutoCountry(country);
				}
#endif /* CNTRY_DEFAULT */
			}
#endif /* STA && !WLTEST */
			// Clear testing overrides when we lose our association
			rateOverrideReset();
			phySubmodeDisassocReset();
#if VERSION_MAJOR > 9
			// Pass the last deauth reason, if any */
			if (evt->event.reason == WLC_E_LINK_DISASSOC) {
				if (blk->link_deauth_reason == 0 )
					reason = DOT11_RC_DISASSOC_LEAVING;
				else
					reason = blk->link_deauth_reason;
			} else if (evt->event.reason == WLC_E_LINK_BCN_LOSS) {
				// print cached rssi value
				IOLog("wl%d: Beacon Loss Event, last RSSI[%d]\n", unit, link_event_rssi);
				if (blk->link_deauth_reason == 0)
					reason = DOT11_RC_INACTIVITY;
				else
					reason = blk->link_deauth_reason;
#if defined(STA)
				uint32 chanspec = 0;
				struct apple80211_channel channel = {};
				int rssi = 0;
				if (wlc_iovar_op( pWLC, "chanspec", NULL, 0, &chanspec, sizeof( chanspec ), IOV_GET, wlcif ) == BCME_OK) {
					chanspec2applechannel(chanspec, &channel);
#if defined(DBG_BCN_LOSS)
					struct wlc_scb_dbg_bcn dbg_bcn = {};

					if (wlc_get_bcn_dbg_info(bsscfg, addr, &dbg_bcn) == BCME_OK) {
						rssi = dbg_bcn.last_bcn_rssi;
					}
					else {
						WL_ERROR(("wl%d %s: error getting beacon debug info\n", unit, __FUNCTION__));
					}
#else
					rssi = link_event_rssi;
#endif /* defined(DBG_BCN_LOSS) */

					char channelStr[7] = {};

					if (channel.flags & APPLE80211_C_FLAG_40MHZ)
					{
						if (channel.flags & APPLE80211_C_FLAG_EXT_ABV)
						{
							snprintf(channelStr, sizeof(channelStr), "%u,+1", channel.channel);
						}
						else
						{
							snprintf(channelStr, sizeof(channelStr), "%u,-1", channel.channel);
						}
					}
					else
					{
						snprintf(channelStr, sizeof(channelStr), "%u", channel.channel);
					}

#if defined(MAC_OS_X_VERSION_10_9)
					char rssiStr[4] = {};
					snprintf(rssiStr, sizeof(rssiStr), "%i", rssi);

					UInt32 kvCount = 2;
					const char *keys[] = {"com.apple.message.last_rssi", "com.apple.message.last_channel"};
					const char *values[] = {rssiStr, channelStr};

					IO8LogMT("wifi.driver.broadcom.beacon_loss", keys, values, kvCount);
#else
					IO8LogMT((APPLE80211_MT_FLAG_DOMAIN | APPLE80211_MT_FLAG_SIG | APPLE80211_MT_FLAG_SIG2 | APPLE80211_MT_FLAG_VAL),
						 (char *)"wifi.droppedConnection.beaconLoss",
						 NULL,
						 (char *)"Link drop due to beacon loss event",
						 channelStr,
						 NULL,
						 0,
						 rssi,
						 0,
						 0,
						 NULL,
						 NULL,
						 NULL);
#endif
				}
				else {
					WL_ERROR(("wl%d %s: error getting chanspec\n", unit, __FUNCTION__));
				}
#endif /* defined(STA) */
			} else if (evt->event.reason == WLC_E_LINK_BSSCFG_DIS) {
				reason = DOT11_RC_DEAUTH_LEAVING;
			}
			else if (evt->event.reason == WLC_E_LINK_ASSOC_REC) {
				reason = DOT11_RC_DISASSOC_LEAVING;
			}

			linkQualHistoryReset();
			linkQualHistoryUpd(0);
#endif

			// Clear the association status
			blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
		}

#ifdef APPLE80211_IOC_LINK_CHANGED_EVENT_DATA
		/* cache link state/reason before setLinkState */
		blk->isLinkDown = !link_up;
		if (!link_up) {
			blk->link_changed_reason.link_down_reason.reason = evt->event.reason;
		}
#endif
		// Report link change
		WL_UNLOCK();

		setLinkState( linkState , reason);
		WL_LOCK();

#ifdef APPLE80211_POSTMESSAGE_TLV
		{
			struct apple80211_messasge_link_changed link_changed;

			link_changed.status = reason;
			link_changed.reason = APPLE80211_RETURN(kApple80211ReturnLink, (evt->event.reason - 1));

			WL_ASSOC(("%s(): WLC_E_LINK:\n", __FUNCTION__));
			wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_LINK_CHANGED,
				(uint8 *)&link_changed, NULL);
		}
#endif /* APPLE80211_POSTMESSAGE_TLV */

		break;
	}

	case WLC_E_MIC_ERROR: {
		int msg = (evt->event.flags&WLC_EVENT_MSG_GROUP) ?
		        APPLE80211_M_MIC_ERROR_MCAST : APPLE80211_M_MIC_ERROR_UCAST;
		postMessage(msg);
		break;
	}

	case WLC_E_SCAN_COMPLETE: {
#ifdef IO80211P2P
		// Post messages only for externally generated scan events
		if (virtIf) {
			if (scan_event_external) {
				if (status == WLC_E_STATUS_SUCCESS ) {
					if (virtIf->listen_complete)
						virtIf->listen_complete = FALSE;
					else
						virtIf->postMessage( APPLE80211_M_P2P_SCAN_COMPLETE );
				}
				scan_event_external = 0;
			}
		} else
#endif
		if (scan_event_external) {
			scan_event_external = 0;
		}
#if VERSION_MAJOR > 9
		// INTERNAL_SCAN_DONE makes sense only if it's successful and not on virtual interface
		else if (status == WLC_E_STATUS_SUCCESS) {
			wlc_bss_list_t bss_list = {};

			memset(&bss_list, 0, sizeof(bss_list));
			if (SCANCACHE_ENAB(pWLC->scan)) {
				wlc_scan_get_cache(pWLC->scan, NULL, 1, NULL, DOT11_BSSTYPE_ANY,
				                   NULL, 0, &bss_list);
			}

			scanComplete(&bss_list);
			// wlc_scan_get_cache() may call wlc_scan_build_cache_list(), which calls MALLOC()
			// need to make sure we clean up the memory that may be MALLOC'd
			wlc_bss_list_free(pWLC, &bss_list);
			postMessage(APPLE80211_M_INTERNAL_SCAN_DONE);
		}
#endif
		break;
	}
	case WLC_E_SET_SSID:
		KERNEL_DEBUG_CONSTANT(WiFi_events_APPLE80211_M_ASSOC_DONE | DBG_FUNC_NONE, status, 0, 0, 0, 0);
		if (status == WLC_E_STATUS_NO_NETWORKS) {
			WL_ERROR(( "directed SSID scan fail\n" ));
#ifdef RDR5858823
			blk->cached_assocstatus = APPLE80211_NO_NETWORKS;
#else
			blk->cached_assocstatus = APPLE80211_STATUS_TIMEOUT;
#endif
		}
#ifdef IO80211P2P
		// Handle virtual interface join
		if (virtIf) {
			blk->association_status = blk->cached_assocstatus;
			virtIf->postMessage( APPLE80211_M_ASSOC_DONE );
			break;
		} else
#endif
		// Handle IBSS create/join
		if (pWLC->default_bss->bss_type == DOT11_BSSTYPE_INDEPENDENT) {
			if (status == WLC_E_STATUS_SUCCESS)
				blk->association_status = DOT11_SC_SUCCESS;
			else {
				blk->association_status = DOT11_SC_FAILURE;
				setIBSS(FALSE);
			}
		} else {
			blk->association_status = blk->cached_assocstatus;
		}
		checkStatusCode17();
		postMessage(APPLE80211_M_ASSOC_DONE);

#ifdef APPLE80211_POSTMESSAGE_TLV
		{
			struct apple80211_message_assoc_done assoc_done;

			assoc_done.status = blk->association_status;
			assoc_done.reason = reason;

			WL_ASSOC(("%s(): WLC_E_SET_SSID:\n", __FUNCTION__));
			wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_SSID_CHANGED, (uint8*)&assoc_done,
				NULL);
		}
#endif /* APPLE80211_POSTMESSAGE_TLV */

		break;

	case WLC_E_JOIN:
		break;

#if VERSION_MAJOR > 9
	// Apple change: hostapd support
	case WLC_E_REASSOC_IND:
		if(BSSCFG_AP(bsscfg))
		{
			// save sta IE list when sta associates
			allocAssocStaIEList(evt->addr, evt->data, evt->event.datalen);
#if defined(IO80211P2P)
			if( virtIf )
				virtIf->postMessage(APPLE80211_M_STA_ARRIVE, (void *)evt->addr, sizeof(struct ether_addr));
			else
#endif
				postMessage(APPLE80211_M_STA_ARRIVE, (void *)evt->addr, sizeof(struct ether_addr));
		}
		break;

	case WLC_E_ASSOC_IND:
		// save sta IE list when sta associates
		allocAssocStaIEList(evt->addr, evt->data, evt->event.datalen);
#if defined(IO80211P2P)
		if( virtIf )
			virtIf->postMessage(APPLE80211_M_STA_ARRIVE, (void *)evt->addr, sizeof(struct ether_addr));
		else
#endif
			postMessage(APPLE80211_M_STA_ARRIVE, (void *)evt->addr, sizeof(struct ether_addr));
		break;

	case WLC_E_DEAUTH:
		if(BSSCFG_AP(bsscfg))
		{
			// free sta IE list when sta leaves
			freeAssocStaIEList(evt->addr, FALSE);
#if defined(IO80211P2P)
			if( virtIf )
				virtIf->postMessage(APPLE80211_M_STA_LEAVE, (void *)evt->addr, sizeof(struct ether_addr));
			else
#endif
				postMessage(APPLE80211_M_STA_LEAVE, (void *)evt->addr, sizeof(struct ether_addr));
		}
		break;

	// Apple change: WPA2-PSK IBSS support
	case WLC_E_IBSS_ASSOC:
	{
		struct apple80211_ibss_peer ibss_peer = {};

		if( evt->event.datalen <= sizeof( ibss_peer.rsn_ie ) )
		{
			// Copy WPA2 IE if present
			if( evt->event.datalen )
				memcpy( ibss_peer.rsn_ie, evt->data, evt->event.datalen );

			memcpy( &ibss_peer.ea, evt->addr, sizeof( struct ether_addr ) );

			postMessage( APPLE80211_M_IBSS_PEER_ARRIVED, &ibss_peer, sizeof( ibss_peer ) );
		}
		break;
	}

	case WLC_E_DISASSOC:
		if( evt->event.auth_type == DOT11_BSSTYPE_INDEPENDENT )
		{
			struct apple80211_ibss_peer ibss_peer = {};

			memcpy( &ibss_peer.ea, evt->addr, sizeof( struct ether_addr ) );

			postMessage( APPLE80211_M_IBSS_PEER_LEFT, &ibss_peer, sizeof( ibss_peer ) );
		}
		break;
#endif /* VERSION_MAJOR > 9 */

	case WLC_E_AUTH:
		MacAuthEvent( ifname, blk, reason, status, (UInt8*)addr );

#ifdef APPLE80211_POSTMESSAGE_TLV
		{
			struct apple80211_message_auth auth_msg;

			auth_msg.status = blk->cached_assocstatus;
			auth_msg.reason = reason;

			WL_ASSOC(("%s(): WLC_E_AUTH:\n", __FUNCTION__));
			wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_AUTH, (uint8*)&auth_msg,
			NULL);

		}
#endif /* APPLE80211_POSTMESSAGE_TLV */

		break;

	case WLC_E_REASSOC:
	case WLC_E_ASSOC:
		if (status == WLC_E_STATUS_SUCCESS || status == WLC_E_STATUS_FAIL) {
			blk->cached_assocstatus = (uint16)reason;
		} else if (status == WLC_E_STATUS_TIMEOUT) {
			blk->cached_assocstatus = DOT11_SC_AUTH_TIMEOUT;
		}

#ifdef ENABLE_CORECAPTURE
		switch (status) {
			case WLC_E_STATUS_FAIL:
				switch (reason) {
					case DOT11_SC_ASSOC_RATE_MISMATCH:
					case DOT11_SC_INVALID_PARAMS:
					case DOT11_SC_INVALID_RSNIE_CAP:
					case DOT11_SC_TIMEOUT:
					case DOT11_SC_UNEXP_MSG:
					case DOT11_SC_TRANSMIT_FAILURE:
						needCoreCapture = true;
						break;
				}
				break;

			case WLC_E_STATUS_TIMEOUT:
			case WLC_E_STATUS_NO_ACK:
			case WLC_E_STATUS_ATTEMPT:
			case WLC_E_STATUS_ERROR:
				needCoreCapture = true;
				break;
		}

		if (needCoreCapture) {
			snprintf(reason_string, sizeof(reason_string), "AssocFail:sts:%d_rsn:%d", status, reason);
			reason_string[sizeof(reason_string)-1] = 0;
			wl_log_system_state((void*)this, reason_string, TRUE);
		}
#endif /* ENABLE_CORECAPTURE */

#ifdef APPLE80211_POSTMESSAGE_TLV
		{
			struct apple80211_message_assoc_done assoc_msg;

			/* struct apple80211_message_assoc_done and struct apple80211_message_reassoc are the same */
			assoc_msg.status = blk->cached_assocstatus;
			assoc_msg.reason = reason;

			WL_ASSOC(("%s(): %s:\n", __FUNCTION__,
				(event_type == WLC_E_REASSOC) ? "WLC_E_REASSOC" : "WLC_E_ASSOC"));

			wl_compose_postmessage_tlv(evt,
				(event_type == WLC_E_REASSOC) ? APPLE80211_POSTMESSAGE_TYPE_REASSOC : APPLE80211_POSTMESSAGE_TYPE_ASSOC_DONE,
				(uint8 *)&assoc_msg, NULL);
		}
#endif /* APPLE80211_POSTMESSAGE_TLV */

		break;

	case WLC_E_DEAUTH_IND: {
		if(BSSCFG_AP(bsscfg))
		{
			// free sta IE list when sta leaves
			freeAssocStaIEList(evt->addr, FALSE);
#if VERSION_MAJOR > 9
#if defined(IO80211P2P)
			if( virtIf )
				virtIf->postMessage(APPLE80211_M_STA_LEAVE, (void *)evt->addr, sizeof(struct ether_addr));
			else
#endif
				postMessage(APPLE80211_M_STA_LEAVE, (void *)evt->addr, sizeof(struct ether_addr));
#endif
		}
		else
		{
			blk->deauth_reason = (uint16)reason;
			blk->link_deauth_reason = blk->deauth_reason;

			if (addr)
				bcopy(addr, &blk->deauth_ea, ETHER_ADDR_LEN);

			// Don't want to continue roam/assoc abort if deauth is received due to
			// #2 as this is considered as invalid passphrase
			if (reason == DOT11_RC_AUTH_INVAL && pWLC->pub->up) {
				wlc_assoc_abort(bsscfg);
			}

#ifdef BCMDBG
			char eabuf[ETHER_ADDR_STR_LEN] = "";
			if (addr)
				bcm_ether_ntoa(addr, eabuf);
			WL_ERROR(( "DeAuthenticated by %s, reason #%d\n", eabuf, reason));
#endif

#ifndef APPLE80211_M_DEAUTH_RECEIVED
#define APPLE80211_M_DEAUTH_RECEIVED 200
#endif

#ifdef APPLE80211_POSTMESSAGE_TLV
			{
				struct apple80211_message_deauth_received deauth_msg;
				deauth_msg.reason = blk->deauth_reason;
				deauth_msg.status = reason;

				WL_ASSOC(("%s(): WLC_E_DEAUTH_IND:\n", __FUNCTION__));
				wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_DEAUTH_RECEIVED,
					(uint8 *)&deauth_msg, virtIf);
			}
#endif /* APPLE80211_POSTMESSAGE_TLV */

#ifdef IO80211P2P
			if( virtIf )
				virtIf->postMessage(APPLE80211_M_DEAUTH_RECEIVED, &blk->deauth_reason, sizeof(blk->deauth_reason));
			else
#endif
				postMessage(APPLE80211_M_DEAUTH_RECEIVED, &blk->deauth_reason, sizeof(blk->deauth_reason));
		}

		break;
	}
	case WLC_E_DISASSOC_IND:
#if VERSION_MAJOR > 9
		// Apple change: hostapd support
		if(BSSCFG_AP(bsscfg))
		{
			// free sta IE list when sta leaves
			freeAssocStaIEList(evt->addr, FALSE);
#if defined(IO80211P2P)
			if( virtIf )
				virtIf->postMessage(APPLE80211_M_STA_LEAVE, (void *)evt->addr, sizeof(struct ether_addr));
			else
#endif
				postMessage(APPLE80211_M_STA_LEAVE, (void *)evt->addr, sizeof(struct ether_addr));
		}
#ifdef APPLE80211_POSTMESSAGE_TLV
		else {
			struct apple80211_message_dissasoc_received disassoc_msg;

			disassoc_msg.reason = reason;
			disassoc_msg.status = status;

			WL_ASSOC(("%s(): WLC_E_DISASSOC_IND:\n", __FUNCTION__));
			wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_DISSASOC_RECEIVED,
				(uint8 *)&disassoc_msg, NULL);
		}
#endif /* APPLE80211_POSTMESSAGE_TLV */
#endif /* VERSION_MAJOR > 9 */
		break;

	case WLC_E_START:
		break;

	case WLC_E_ROAM: {
		char eabuf[ETHER_ADDR_STR_LEN] = "";

#ifdef ENABLE_CORECAPTURE
		switch (status) {
			case WLC_E_STATUS_FAIL:
			case WLC_E_STATUS_TIMEOUT:
			case WLC_E_STATUS_NO_ACK:
			case WLC_E_STATUS_UNSOLICITED:
			case WLC_E_STATUS_ERROR:
				needCoreCapture = true;
				break;
		}

		if (needCoreCapture) {
			snprintf(reason_string, sizeof(reason_string), "RoamFail:sts:%d_rsn:%d", status, reason);
			reason_string[sizeof(reason_string)-1] = 0;
			wl_log_system_state((void*)this, reason_string, TRUE);
		}
#endif /* ENABLE_CORECAPTURE */

		// P2P Virtual driver consideration
		if (status != WLC_E_STATUS_SUCCESS) {
			break;
		}

		if (addr)
			bcm_ether_ntoa(addr, eabuf);
		IOLog("wl%d: Roamed or switched channel, reason #%d, bssid %s, last RSSI %d\n", unit, reason, eabuf, roam_start_rssi);

		blk->association_status = blk->cached_assocstatus;

		// Clear testing overrides when we change APs
		rateOverrideReset();
		phySubmodeDisassocReset();
#if VERSION_MAJOR > 9
		linkQualHistoryReset();
		{
			int maxRate = 0;
			if(wlIoctlGetInt(WLC_GET_MAX_RATE, &maxRate) != 0)
				WL_ERROR(("%s: WLC_GET_MAX_RATE failed\n", __FUNCTION__));

			// Convert maxrate from 500kbps to Mbps
			maxRate /= 2;
			linkQualHistoryUpd(maxRate);
		}
#endif

#ifdef SUPPORT_FEATURE_WOW
		// Apple Change: <rdar://problem/6578148> Ask networking family to re-query
		//				 magic packet capability after roaming
		getNetworkInterface()->inputEvent( kIONetworkEventWakeOnLANSupportChanged, NULL );
#endif

		postMessage(APPLE80211_M_BSSID_CHANGED);
		break;
	}

#if defined(AWDL_FAMILY) && defined(APPLE80211_M_ROAM_START)
	case WLC_E_ROAM_START: {
		struct apple80211_roam_event_data event_data = {};

		memset(&event_data, 0, sizeof(event_data));
		/* auth_type is the ROAM_START(1)/END(0) indication */
		if (evt->event.auth_type == ON) {
			event_data.reason = reason;
			postMessage(APPLE80211_M_ROAM_START, (void *)&event_data, sizeof(event_data));
			roam_start_set = TRUE;
			WL_ASSOC(("%s(): post APPLE80211_M_ROAM_START event.\n", __FUNCTION__));
		}
		else if (roam_start_set) {
			event_data.reason = status;
			postMessage(APPLE80211_M_ROAM_END, (void *)&event_data, sizeof(event_data));
			roam_start_set = FALSE;
			WL_ASSOC(("%s(): post APPLE80211_M_ROAM_END event.\n", __FUNCTION__));
		}
		break;
	}
#endif /* AWDL_FAMILY */

#if VERSION_MAJOR > 11
	case WLC_E_CSA_START_IND: {
		apple80211_channel_switch_anoucement_t csa;
		bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
		csa.complete = APPLE80211_CSA_START;
		postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
		break;
	}
	case WLC_E_CSA_DONE_IND: {
		apple80211_channel_switch_anoucement_t csa;
		bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
		csa.complete = APPLE80211_CSA_SUCCESS;
		postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
		break;
	}
	case WLC_E_CSA_FAILURE_IND: {
		apple80211_channel_switch_anoucement_t csa;
		bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
		csa.complete = APPLE80211_CSA_FAILURE;
		postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
		break;
	}
#endif

	case WLC_E_CSA_COMPLETE_IND:

		if (status != WLC_E_STATUS_SUCCESS)
			break;

		IOLog("wl%d: Roamed or switched channel, reason CSA\n", unit);
		break;

	case WLC_E_COUNTRY_CODE_CHANGED: {
#ifdef APPLE80211_IOC_TX_CHAIN_POWER
		// re-apply the per-chain offsets if the country changes since
		// it may change the per-band max transmit target
		struct apple80211_chain_power_data_set txcpd = {};
		bool have_offset = false;

		txcpd.version = APPLE80211_VERSION;
		txcpd.num_chains = (int32_t)num_radios;
		for (uint i = 0; i < num_radios; i++) {
			if (tx_chain_offset[i] != 0)
				have_offset = true;
			txcpd.powers[i].power_offset = tx_chain_offset[i];
		}
		if (have_offset)
			setTX_CHAIN_POWER(getNetworkInterface(), &txcpd);
#endif

		postMessage(APPLE80211_M_COUNTRY_CODE_CHANGED);
		break;
	}
#ifdef IO80211P2P
	case WLC_E_P2P_DISC_LISTEN_COMPLETE:
	{
		if( virtIf) {
			virtIf->listen_complete = TRUE;
			if (status == 0 && reason == 0 )
				virtIf->postMessage( APPLE80211_M_P2P_LISTEN_COMPLETE );
		}

		break;
	}
/* Radar12866312:  Apple requested disabling WLC_E_PROBRESP_MSG and
 * WLC_E_P2P_PROBREQ_MSG since Family no longer uses these events
 */
#endif /* IO80211P2P */
#ifdef WLEXTLOG
	case WLC_E_EXTLOG_MSG: {
		log_record_t *record = (log_record_t *)evt->data;
		ASSERT(evt->event.datalen == sizeof(log_record_t));
		{
			int j;
			for (j = 0; j < FMTSTR_MAX_ID; j++)
				if (record->id == extlog_fmt_str[j].id)
					break;

			if (j == FMTSTR_MAX_ID)
				break;

			switch (extlog_fmt_str[j].arg_type) {
			case LOG_ARGTYPE_NULL:
				IO8Log("%s", extlog_fmt_str[j].fmt_str);
				break;

			case LOG_ARGTYPE_STR:
				IO8Log(extlog_fmt_str[j].fmt_str, record->str);
				break;

			case LOG_ARGTYPE_INT:
				IO8Log(extlog_fmt_str[j].fmt_str, record->arg);
				break;

			case LOG_ARGTYPE_INT_STR:
				IO8Log(extlog_fmt_str[j].fmt_str, record->arg,
					record->str);
				break;

			case LOG_ARGTYPE_STR_INT:
				IO8Log(extlog_fmt_str[j].fmt_str, record->str,
					record->arg);
				break;
			}
		}
		break;
	}
#endif
#ifdef IO80211P2P
	case WLC_E_ACTION_FRAME_RX:
	{
		struct apple80211_act_frm_hdr appleHdr = {};
		wl_event_rx_frame_data_t * brcmHdr = (wl_event_rx_frame_data_t *)evt->data;
		bool ifUp = FALSE;

		ifUp = ((virtIf) ? virtIf->enabledBySystem() : myNetworkIF->enabledBySystem());

		if (!ifUp) {
			WL_IOCTL(("wl%d.%d: %s::WLC_E_ACTION_FRAME_RX, interface not marked up, drop frame\n",
			          unit, bsscfg->_idx, __FUNCTION__));
			break;
		}
		memcpy( &appleHdr.addr, evt->addr, sizeof( *evt->addr ) );
		chanspec2applechannel( ntoh16( brcmHdr->channel ), &appleHdr.chan );

		UInt32 dataLen = evt->event.datalen - sizeof( wl_event_rx_frame_data_t );

		mbuf_t m = allocatePacket( dataLen );

		if( m ) {
			memcpy( mbuf_data( m ), (uint8*)(evt->data) + sizeof(wl_event_rx_frame_data_t), dataLen );
			mbuf_setlen( m, dataLen );
			mbuf_pkthdr_setlen( m, dataLen );

			if( virtIf )
				virtIf->bpfTapInput( m, APPLE80211_ACT_FRM_DLT, &appleHdr, sizeof( appleHdr ) );
			else
				myNetworkIF->bpfTapInput( m, APPLE80211_ACT_FRM_DLT, &appleHdr, sizeof( appleHdr ) );

			freePacket( m );
		}
		break;
	}
#ifdef FAST_ACTFRM_TX
	case WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE:
	{
		restartActFrmQueue();
		break;
	}
#endif // FAST_ACTFRM_TX
#endif

#if VERSION_MAJOR > 10
	// Apple change: Support for APPLE80211_M_ACT_FRM_TX_COMPLETE
	case WLC_E_ACTION_FRAME_COMPLETE:
	{
#if defined(IO80211P2P)
		if( virtIf )
		{
			virtIf->postMessage( APPLE80211_M_ACT_FRM_TX_COMPLETE,
								 &_actFrmCompleteData,
								 sizeof( _actFrmCompleteData ) );
		}
		else
#endif // IO80211P2P
		{
			myNetworkIF->postMessage( APPLE80211_M_ACT_FRM_TX_COMPLETE,
									  &_actFrmCompleteData,
									  sizeof( _actFrmCompleteData ) );
		}

		break;
	}
#endif // VERSION_MAJOR > 10

	default:
		break;
	}
}
#else
/* TODO: should support all the events as above */
void
AirPort_Brcm43xx::wlEvent( char *ifname, wl_event_msg_t *evt, void *data )
{
	uint event_type = evt->event_type;
	struct ether_addr* addr = &evt->addr;
	uint reason = evt->reason;
	uint status = evt->status;
	struct wlc_if *wlcif = NULL;
	intf_info_blk_t *blk = IFINFOBLKP(myNetworkIF);
#if defined(MACOS_VIRTIF) || defined(IO80211P2P)
	int len = strlen(ifname);
#endif /* MACOS_VIRTIF || IO80211P2P */

	WL_PORT(("%s: %s %s Event %d\n", __FUNCTION__, wl_ifname((wl_info*)this, NULL), ifname,
	         event_type));

#ifdef MACOS_VIRTIF
	bool virtif = FALSE;
	if (strncmp(wl_ifname((wl_info*)this, NULL), ifname, len) != 0) {
		if (apif.virtIf && strncmp(wl_ifname((wl_info*)this, &apif),
		                        ifname, len) == 0) {
			wlcif = apif.wlcif;
			virtif = TRUE;
		}
	}
#endif
#ifdef IO80211P2P
	AirPort_Brcm43xxP2PInterface *virtIf = NULL;

	/* IF event, though it contains name of the virtual if, is always meant for main if */
	if ((event_type != WLC_E_IF) &&
	    (strncmp(wl_ifname((wl_info*)this, NULL), ifname, len) != 0)) {
		struct virt_intf_entry *elt;
		SLIST_FOREACH(elt, &virt_intf_list, entries) {
			if (strncmp(wl_ifname((wl_info*)this, (struct wl_if *)elt->obj),
			         ifname, len) == 0)
			{
				virtIf = elt->obj;
				wlcif = virtIf->wlcif;
				blk = IFINFOBLKP(virtIf);
				break;
			}
		}
	}
#endif
	switch (event_type) {
	case WLC_E_LINK: {

#ifdef MACOS_VIRTIF
#if defined(BCMDBG)
		IOLog("%s: %s %s Link %s\n", __FUNCTION__, wl_ifname((wl_info*)this, NULL), ifname,
		      (evt->event.flags&WLC_EVENT_MSG_LINK)?"UP":"DOWN");
#endif /* BCMDBG */
		if (virtif) {
			WL_UNLOCK();
			apif.virtIf->inputEvent((evt->event.flags&WLC_EVENT_MSG_LINK) ?
			                        kIONetworkEventTypeLinkUp: kIONetworkEventTypeLinkDown, NULL);
			WL_LOCK();
			break;
		}
#endif
#ifdef IO80211P2P
#if defined(BCMDBG)
		IOLog("%s: %s %s Link %s\n", __FUNCTION__, wl_ifname((wl_info*)this, NULL), ifname,
		      (evt->event.flags&WLC_EVENT_MSG_LINK)?"UP":"DOWN");
#endif /* BCMDBG */
		if (virtIf) {
			WL_UNLOCK();
			virtIf->setLinkState((evt->event.flags&WLC_EVENT_MSG_LINK) ?
			                     kIO80211NetworkLinkUp: kIO80211NetworkLinkDown );
			WL_LOCK();

#if VERSION_MAJOR > 11
		// post a APPLE80211_M_CHANNEL_SWITCH event to Family so it re-enables the AWDL interface from a Channel
		// Switch Annoucement
		if (evt->event.flags & WLC_EVENT_MSG_LINK) {
				apple80211_channel_switch_anoucement_t csa;
				bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
				csa.complete = APPLE80211_CSA_SUCCESS;
				postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
				// pWLC->csa_state_join=0;
		}
#endif

			break;
		}
#endif /* IO80211P2P */

		IO80211LinkState linkState = (evt->flags&WLC_EVENT_MSG_LINK) ?
		        kIO80211NetworkLinkUp : kIO80211NetworkLinkDown;
		UInt32 reason = DOT11_RC_INACTIVITY; // Default RC code

		if ( linkState == kIO80211NetworkLinkUp ) {
			// turn off framebursting for b-only
			int  maxRate = 0;
			if(wlIoctlGetInt(WLC_GET_MAX_RATE, &maxRate) != 0) {
				WL_ERROR(("%s: WLC_GET_MAX_RATE failed\n", __FUNCTION__));
			}
			blk->link_deauth_reason = 0;
			reason = 0;
			// WLC_GET_MAX_RATE returns value in units of 500kbps
			// Convert to MBPS
			maxRate = maxRate / 2;
			if (maxRate != 0) {		// don't change on error
				(void)wlIoctlSetInt(WLC_SET_FAKEFRAG, (maxRate > 108) ? 1 : 0 );
			}
#if VERSION_MAJOR > 9
			linkQualHistoryUpd(maxRate);
#endif
		} else {
			// Clear testing overrides when we lose our association
			rateOverrideReset();
			phySubmodeDisassocReset();
#if VERSION_MAJOR > 9
			// Pass the last deauth reason, if any */
			if (evt->reason == WLC_E_LINK_DISASSOC) {
				if (blk->link_deauth_reason == 0 ) {
					reason = DOT11_RC_DISASSOC_LEAVING;
				} else {
					reason = blk->link_deauth_reason;
				}
			} else if (evt->reason == WLC_E_LINK_BCN_LOSS) {
				// print cached rssi value
				if (blk->link_deauth_reason == 0) {
					reason = DOT11_RC_INACTIVITY;
				} else {
					reason = blk->link_deauth_reason;
				}
#if defined(STA)
				uint32 chanspec;
				struct apple80211_channel channel;
				int rssi = 0;
				int bcmerr;
				scb_val_t scb_val;
				if (wlIovarOp("chanspec", NULL, 0, &chanspec, sizeof( chanspec ), IOV_GET, NULL) == BCME_OK) {
					char channelStr[7] = {};

					chanspec2applechannel(chanspec, &channel);
					bzero( &scb_val.ea, sizeof( struct ether_addr ) );
					bcmerr = wlIoctl(WLC_GET_RSSI, &scb_val, sizeof(scb_val), FALSE, NULL);
					if (bcmerr != BCME_OK) {
						WL_ERROR(("wl%d: %s(): Error= %d getting RSSI (WLC_GET_RSSI)\n",
									pub->unit, __FUNCTION__, bcmerr));
						break;
					}
					rssi = scb_val.val;

					if (channel.flags & APPLE80211_C_FLAG_40MHZ) {
						if (channel.flags & APPLE80211_C_FLAG_EXT_ABV) {
							snprintf(channelStr, sizeof(channelStr), "%u,+1", channel.channel);
						} else {
							snprintf(channelStr, sizeof(channelStr), "%u,-1", channel.channel);
						}
					} else {
						snprintf(channelStr, sizeof(channelStr), "%u", channel.channel);
					}

#if defined(MAC_OS_X_VERSION_10_9)
					char rssiStr[4] = {};
					snprintf(rssiStr, sizeof(rssiStr), "%i", rssi);

					UInt32 kvCount = 2;
					const char *keys[] = {"com.apple.message.last_rssi", "com.apple.message.last_channel"};
					const char *values[] = {rssiStr, channelStr};

					IO8LogMT("wifi.driver.broadcom.beacon_loss", keys, values, kvCount);
#else
					IO8LogMT((APPLE80211_MT_FLAG_DOMAIN |
								APPLE80211_MT_FLAG_SIG |
								APPLE80211_MT_FLAG_SIG2 |
								APPLE80211_MT_FLAG_VAL),
							(char *)"wifi.droppedConnection.beaconLoss",
							NULL,
							(char *)"Link drop due to beacon loss event",
							channelStr,
							NULL,
							0,
							rssi,
							0,
							0,
							NULL,
							NULL,
							NULL);
#endif
				} else {
						WL_ERROR(("wl%d %s: error getting chanspec\n", unit, __FUNCTION__));
				}
#endif /* defined(STA) */
			} else if (evt->reason == WLC_E_LINK_BSSCFG_DIS) {
				reason = DOT11_RC_DEAUTH_LEAVING;
			}
			else if (evt->reason == WLC_E_LINK_ASSOC_REC) {
				reason = DOT11_RC_DISASSOC_LEAVING;
			}
			linkQualHistoryReset();
			linkQualHistoryUpd(0);
#endif

			// Clear the association status
			blk->association_status = APPLE80211_STATUS_UNAVAILABLE;
			}

			// Report link change
			WL_UNLOCK();

			setLinkState( linkState , reason);
			WL_LOCK();
			break;
		}

	case WLC_E_MIC_ERROR: {
		int msg = (evt->flags&WLC_EVENT_MSG_GROUP) ?
		APPLE80211_M_MIC_ERROR_MCAST : APPLE80211_M_MIC_ERROR_UCAST;
		postMessage(msg);
		break;
	}
	case WLC_E_ESCAN_RESULT: {
			wl_escan_result_t *escan_result = (wl_escan_result_t*) data;
			if (status == WLC_E_STATUS_ERROR) {
				WL_ERROR(("%s: for WLC_E_ESCAN_RESULT error is %d\n", status));
				break;
			}
			//scanComplete(escan_result->bss_info, escan_result->bss_count);
			scanAddResult(escan_result->bss_info, escan_result->bss_count);
			if (status == WLC_E_STATUS_SUCCESS) {
				scan_done = 1;
				current_scan_result = SLIST_FIRST(&scan_result_list);
				KERNEL_DEBUG_CONSTANT(WiFi_events_APPLE80211_M_SCAN_DONE | DBG_FUNC_NONE,
					status, 0, 0, 0, 0);
				postMessage(APPLE80211_M_SCAN_DONE);
			}
		}
		break;

	case WLC_E_SCAN_COMPLETE: {
		if (scan_event_external) {
			scan_event_external = 0;
		}
#if VERSION_MAJOR > 9
		// INTERNAL_SCAN_DONE makes sense only if it's successful and not on virtual interface
		else if (status == WLC_E_STATUS_SUCCESS) {
			wl_escan_result_t *escan_result = (wl_escan_result_t*) data;
			if(escan_result) {
				scanAddResult(escan_result->bss_info, escan_result->bss_count);
			}
			postMessage(APPLE80211_M_INTERNAL_SCAN_DONE);
		}
#endif
		break;
	}
	case WLC_E_SET_SSID:
		KERNEL_DEBUG_CONSTANT(WiFi_events_APPLE80211_M_ASSOC_DONE | DBG_FUNC_NONE, status, 0, 0, 0, 0);
		if (status == WLC_E_STATUS_NO_NETWORKS) {
			WL_ERROR(( "directed SSID scan fail\n" ));
#ifdef RDR5858823
			blk->cached_assocstatus = APPLE80211_NO_NETWORKS;
#else
			blk->cached_assocstatus = APPLE80211_STATUS_TIMEOUT;
#endif
		}
#ifdef IO80211P2P
		// Handle virtual interface join
		if (virtIf) {
			blk->association_status = blk->cached_assocstatus;
			virtIf->postMessage( APPLE80211_M_ASSOC_DONE );
			break;
		} else
#endif
		{
			// Handle IBSS create/join
			int infra = 0;
			if (wlIoctlGetInt( WLC_GET_INFRA, &infra) != 0) {
				WL_ERROR(("%s: Get infra ioctl failed\n", __FUNCTION__));
				break;
			}
			if (infra == 0) {
				if (status == WLC_E_STATUS_SUCCESS) {
					blk->association_status = DOT11_SC_SUCCESS;
				} else {
					blk->association_status = DOT11_SC_FAILURE;
					setIBSS(FALSE);
				}
			} else {
				blk->association_status = blk->cached_assocstatus;
			}
		}
		checkStatusCode17();
		postMessage(APPLE80211_M_ASSOC_DONE);
		break;

	case WLC_E_JOIN:
		break;

#if VERSION_MAJOR > 9
	case WLC_E_DISASSOC:
		if( evt->auth_type == DOT11_BSSTYPE_INDEPENDENT ) {
			struct apple80211_ibss_peer ibss_peer = {};

			memcpy( &ibss_peer.ea, &evt->addr, sizeof( struct ether_addr ) );

			postMessage( APPLE80211_M_IBSS_PEER_LEFT, &ibss_peer, sizeof( ibss_peer ) );
		}
		break;
#endif /* VERSION_MAJOR > 9 */

	case WLC_E_AUTH:
		MacAuthEvent( ifname, blk, reason, status, (UInt8*)addr );
		break;

	case WLC_E_REASSOC:
	case WLC_E_ASSOC:
		if (status == WLC_E_STATUS_SUCCESS || status == WLC_E_STATUS_FAIL) {
			blk->cached_assocstatus = (uint16)reason;
		} else if (status == WLC_E_STATUS_TIMEOUT) {
			blk->cached_assocstatus = DOT11_SC_AUTH_TIMEOUT;
		} else if (status == WLC_E_STATUS_NO_ACK) {
			blk->cached_assocstatus =  DOT11_SC_SERVER_UNREACHABLE;
		}
		break;

	case WLC_E_START:
		break;

	case WLC_E_ROAM: {
		char eabuf[ETHER_ADDR_STR_LEN] = "";

		// P2P Virtual driver consideration
		if(status != WLC_E_STATUS_SUCCESS) {
			break;
		}

		if(addr) {
			bcm_ether_ntoa(addr, eabuf);
		}
		blk->association_status = blk->cached_assocstatus;

		// Clear testing overrides when we change APs
		rateOverrideReset();
		phySubmodeDisassocReset();
#if VERSION_MAJOR > 9
		linkQualHistoryReset();
		{
			int maxRate = 0;
			if(wlIoctlGetInt(WLC_GET_MAX_RATE, &maxRate) != 0) {
				WL_ERROR(("%s: WLC_GET_MAX_RATE failed\n", __FUNCTION__));
			}

			// Convert maxrate from 500kbps to Mbps
			maxRate /= 2;
			linkQualHistoryUpd(maxRate);
		}
#endif

#ifdef SUPPORT_FEATURE_WOW
		// Apple Change: <rdar://problem/6578148> Ask networking family to re-query
		//				 magic packet capability after roaming
		getNetworkInterface()->inputEvent( kIONetworkEventWakeOnLANSupportChanged, NULL );
#endif

		postMessage(APPLE80211_M_BSSID_CHANGED);

#ifdef APPLE80211_POSTMESSAGE_TLV
		{
			struct apple80211_message_roamed roam_msg;

			roam_msg.status = reason;

			WL_ASSOC(("%s(): WLC_E_ROAM:\n", __FUNCTION__));
			wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_ROAMED, (uint8 *)&roam_msg,
				NULL);
		}
#endif /* APPLE80211_POSTMESSAGE_TLV */

		break;
	}
#if VERSION_MAJOR > 11
	case WLC_E_CSA_START_IND: {
		apple80211_channel_switch_anoucement_t csa;
		bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
		csa.complete = APPLE80211_CSA_START;
		postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
		break;
	}
	case WLC_E_CSA_DONE_IND: {
		apple80211_channel_switch_anoucement_t csa;
		bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
		csa.complete = APPLE80211_CSA_SUCCESS;
		postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
		break;
	}
	case WLC_E_CSA_FAILURE_IND: {
		apple80211_channel_switch_anoucement_t csa;
		bzero( &csa, sizeof(apple80211_channel_switch_anoucement_t) ); // zero out unused fields
		csa.complete = APPLE80211_CSA_FAILURE;
		postMessage(APPLE80211_M_CHANNEL_SWITCH, (void *)&csa, sizeof(apple80211_channel_switch_anoucement_t));
		break;
	}
#endif

	case WLC_E_CSA_COMPLETE_IND:

		if (status != WLC_E_STATUS_SUCCESS) {
			break;
		}

		IOLog("wl%d: Roamed or switched channel, reason CSA\n", unit);
		break;

#ifdef APPLE80211_POSTMESSAGE_TLV
#if NOT_YET
	/* no internal supplicant, no WLC_E_PSK_SUP event for now */
	case WLC_E_PSK_SUP:
	{
		struct apple80211_message_supplicant_event sup_msg;

		sup_msg.status = status;

		wl_compose_postmessage_tlv(evt, APPLE80211_M_SUPPLICANT_EVENT,
			(uint8 *)&sup_msg, NULL);

		break;
	}
	/* Need to sync up with iOS about WLC_E_PRUNE handling */
	case WLC_E_PRUNE:
	{
		struct apple80211_message_prune prune_msg;

		prune_msg.status = reason;

		WL_ASSOC(("%s(): WLC_E_PRUNE:\n", __FUNCTION__));
		wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_PRUNE,
			(uint8 *)&prune_msg, NULL);

		break;
	}
#endif /* NOT YET */

	case WLC_E_ICV_ERROR:
	{
		struct apple80211_message_decryption_error icv_msg;

		icv_msg.status = status;
		icv_msg.reason = reason;

		WL_ASSOC(("%s(): WLC_E_ICV_ERROR:\n", __FUNCTION__));
		wl_compose_postmessage_tlv(evt, APPLE80211_POSTMESSAGE_TYPE_DECRYPTION_ERROR,
			(uint8 *)&icv_msg, NULL);

		break;
	}
#endif /* APPLE80211_POSTMESSAGE_TLV */

#if (VERSION_MAJOR >= 15) && defined(CCA_STATS)
	case WLC_E_CCA_CHAN_QUAL:
	{
		cca_chan_qual_event_t *cca_event_data = NULL;
		chanspec_t chspec = 0;
		struct apple80211_stat_report reason = {};
		struct apple80211_channel channel = {};
		int val = 0;

		ASSERT(evt->datalen);
		cca_event_data = (cca_chan_qual_event_t *) (data);
		memset(&reason, 0, sizeof(reason));

		if (cca_event_data->id == WL_CHAN_QUAL_CCA) {
			chspec = cca_event_data->chanspec;
			chanspec2applechannel(chspec, &channel);
			// drop invalid result radar://13456025
			if (cca_event_data->cca_busy.duration >= cca_event_data->cca_busy.congest
				&& cca_event_data->cca_busy.duration <= 10000)
			{
				// update CCA result for this channel
				reason.reportReason = (enum apple80211_stat_report_reason)kStatReportReason_ChipEvent;
				reason.isChannelSupplied = TRUE;
				reason.currentChannel = channel;

				val = (int)(cca_event_data->cca_busy.congest * 100 / cca_event_data->cca_busy.duration);

				WL_PORT(("%s: chan:%x CCA %d%% duration: %d congest: %d ts: %d\n", __FUNCTION__, chspec, val,
					(unsigned int)cca_event_data->cca_busy.duration,
					(unsigned int)cca_event_data->cca_busy.congest,
					(unsigned int)cca_event_data->cca_busy.timestamp));

				setChanCCA(&reason, val);
			}
		} else if (cca_event_data->id == WL_CHAN_QUAL_NF) {
			chspec = cca_event_data->chanspec;
			chanspec2applechannel(chspec, &channel);

			// update NF result for this channel
			reason.reportReason = (enum apple80211_stat_report_reason)kStatReportReason_ChipEvent;
			reason.isChannelSupplied = TRUE;
			reason.currentChannel = channel;
			val = cca_event_data->noise;

			WL_PORT(("%s: WL_CHAN_QUAL_NF: chan:%x NoiseFloor: %d\n", __FUNCTION__, chspec, val));

			setChanNoiseFloor(&reason, val);
		}

		break;
	}
#endif /* (VERSION_MAJOR >= 15) && CCA_STATS */

	default:
		break;
	}
}
#endif /* !MACOSX_DHD */
#ifdef MACOSX_DHD
void
AirPort_Brcm43xx::scanAddResult(wl_bss_info_t *scan_results, uint count)
{
	scan_result_t *sr;
	scan_result_t *tail_sr = NULL;
	wl_bss_info_t *bi = scan_results;
#ifdef BCMDBG
	char ssidbuf[SSID_FMT_BUF_LEN];
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	for (uint i = 0; i < count; i++) {
		if (bi->RSSI == WLC_RSSI_INVALID) {
			continue;
		}

#ifndef MACOSX_DHD
		if (!scanPhyModeMatch(scan_phy_mode, bi)) {
			WL_IOCTL(("wl%d: %s: pruning for phy_mode match failure: "
			          "%s: ch %3d \"%s\"\n",
			          unit, __func__,
			          bcm_ether_ntoa(&bi->BSSID, eabuf),
			          CHSPEC_CHANNEL(bi->chanspec),
			          (wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf)));
			continue;
		}
#endif /* !MACOSX_DHD */

		// filter based on bssid list passed in from multi scan req
		if ((scanresult_list_filter(&bi->BSSID) == TRUE)) {
			// this scan result is filtered out
			WL_IOCTL(("wl%d: %s: pruning for bssid match failure: "
			          "%s: ch %3d \"%s\"\n",
			          unit, __func__,
			          bcm_ether_ntoa(&bi->BSSID, eabuf),
			          CHSPEC_CHANNEL(bi->chanspec),
			          (wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf)));
			continue;
		}

		sr = (scan_result_t*)IOMalloc(sizeof(scan_result_t));
		if (!sr) {
			break;
		}
		memset(sr, 0, sizeof(*sr));

		scanConvertResult(bi, &sr->asr);

		// Link this scan result to the list
		if (tail_sr) {
			SLIST_INSERT_AFTER(tail_sr, sr, entries);
		} else {
			SLIST_INSERT_HEAD(&scan_result_list, sr, entries);
		}

		tail_sr = sr;
	}
}
#endif /* MACOSX_DHD */

#ifndef MACOSX_DHD
void
AirPort_Brcm43xx::wlEventSync( char *ifname, wlc_event_t *evt )
{
	uint event_type = evt->event.event_type;
	struct ether_addr* addr = evt->addr;
	struct wl_info *wl = (struct wl_info *)this;
	wlc_bsscfg_t *bsscfg;

	WL_PORT(("%s: %s %s Event %d\n", __FUNCTION__, wl_ifname(wl, NULL), ifname, event_type));

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	switch (event_type) {
	case WLC_E_ROAM_START:
		roam_start_rssi = bsscfg->link->rssi;
		break;
	case WLC_E_LINK:
		/* TODO: Investigate moving WLC_E_LINK event from async wl_event()
		 * to sync wl_event_sync() processing. For now just cache the rssi.
		 * Also need to work out multiple interface/link case.
		 */
		link_event_rssi = bsscfg->link->rssi;
		break;
	case WLC_E_ROAM_PREP: {
		IO80211Interface * interface = getNetworkInterface();
#ifdef BCMDBG
			char eabuf[ETHER_ADDR_STR_LEN] = "";
			if (addr) {
				bcm_ether_ntoa(addr, eabuf);
			}
			WL_ASSOC(("Roaming to %s\n", eabuf));
#endif
			WL_UNLOCK();
			interface->willRoam(addr);
			WL_LOCK();
			break;
		}

		default:
			break;
	}
}
#endif /* !MACOSX_DHD */

void
AirPort_Brcm43xx::MacAuthEvent( char *ifname, intf_info_blk_t *blk, UInt32 dot11_status, UInt32 event_status, UInt8 *addr )
{
	char eabuf[ETHER_ADDR_STR_LEN] = {};

	WL_ERROR(( "MacAuthEvent %s ", ifname));
	if (addr) {
		bcm_ether_ntoa((const struct ether_addr *)addr, eabuf);
		WL_ERROR(( "  Auth result for: %s ", eabuf ));
	}

	if (event_status == WLC_E_STATUS_SUCCESS) {
		WL_ERROR(( " MAC AUTH succeeded\n" ));
	} else if (event_status == WLC_E_STATUS_UNSOLICITED) {
		WL_ERROR(( "Unsolicited  Auth\n" ));
	} else if (event_status == WLC_E_STATUS_ABORT) {
		WL_ERROR(( "Auth Aborted\n" ));
	} else if (event_status == WLC_E_STATUS_TIMEOUT) {
		WL_ERROR(( "Auth timed out\n" ));
		blk->cached_assocstatus = DOT11_SC_AUTH_TIMEOUT;
	} else if (event_status == WLC_E_STATUS_NO_ACK) {
		WL_ERROR(( "Auth request tx failed\n" ));
		blk->cached_assocstatus = DOT11_SC_AUTH_TIMEOUT;
	} else if (event_status == WLC_E_STATUS_FAIL) {
		blk->cached_assocstatus = dot11_status;

		switch( dot11_status )	// this is the failure code returned by AP
		{
			case DOT11_SC_FAILURE:
				if (addr && (addr[1] == 0x80) &&
				    (addr[2] == 0xc8) &&
				    (addr[0] == 0x00))
				{		// D-Link returns generic error on bad Auth
					WL_ERROR(( "D-LINK OUI Auth status 1\n" ));
					blk->cached_assocstatus = DOT11_SC_AUTH_MISMATCH;	// Framework will try OpenSys
				}
				else {
					WL_ERROR(( "Not Authenticated\n" ));
				}
				break;

			case DOT11_SC_ASSOC_FAIL:
				WL_ERROR(( "Auth denied for personal reasons\n" ));
				break;

			case DOT11_SC_AUTH_MISMATCH:
				WL_ERROR(( "Authentication Mismatch failure\n" ));
				break;

			case DOT11_SC_AUTH_SEQ:
				WL_ERROR(( "Auth fail code 14 (Cisco?), changing to Challenge Failure\n" ));
				blk->cached_assocstatus = DOT11_SC_AUTH_CHALLENGE_FAIL;
				break;

			case DOT11_SC_AUTH_CHALLENGE_FAIL:
				WL_ERROR(( "Challenge fail\n" ));
				break;
			case DOT11_SC_ASSOC_BUSY_FAIL:
				WL_ERROR(( "AP busy fail\n" ));
				break;
			default:
				WL_ERROR(( "  unknown auth fail = %d\n", (int)dot11_status ));
				// dot11_status could be set to 0 here, which will look like success
				// even though it is really an error
				if( dot11_status == DOT11_SC_SUCCESS )
					// Apple change: use this as a generic error code
					// since DOT11_SC_FAILURE forever means ACL
					// error due to old base station behavior
					blk->cached_assocstatus = DOT11_SC_AUTH_TIMEOUT;
				break;
		}
	} else {
		WL_ERROR(( "unknown event status %d\n", (int)event_status ));
	}
}

void
AirPort_Brcm43xx::txFlowControlClearAll()
{
	struct wl_info *wl = (struct wl_info *)this;

	// if we're still flow controlled, turn it off
	if (intf_txflowcontrol.txFlowControl) {
		WL_IOCTL(("tx flowcontrol primary still on after down, turn it off!\n"));
		wl_txflowcontrol(wl, NULL, OFF, ALLPRIO);
	}
#ifdef MACOS_VIRTIF
	if (apif.virtIf && apif.intf_txflowcontrol.txFlowControl) {
		WL_IOCTL(("tx flowcontrol virtual still on after down, turn it off!\n"));
		wl_txflowcontrol(wl, &apif, OFF, ALLPRIO);
	}
#endif
#ifdef IO80211P2P
	struct virt_intf_entry *elt = NULL;
	SLIST_FOREACH(elt, &virt_intf_list, entries) {
		WL_IOCTL(("tx flowcontrol virtual still on after down, turn it off!\n"));
		wl_txflowcontrol(wl, (wl_if_t *)elt->obj, OFF, ALLPRIO);
	}
#endif
}
#ifndef MACOSX_DHD
#if VERSION_MAJOR > 9

// Apple change: hostapd support
SInt32
AirPort_Brcm43xx::setSTA_WPA_Key( struct apple80211_key * key )
{
	wl_wsec_key_t brcm_key = {};

	bzero( &brcm_key, sizeof( brcm_key ) );

	if (key->key_index == APPLE80211_KEY_INDEX_NONE) {
		if( key->key_len && key->key_flags != ( APPLE80211_KEY_FLAG_TX | APPLE80211_KEY_FLAG_RX ) )
			return EINVAL;
	} else {
		brcm_key.index = (uint32)key->key_index;
	}

	memcpy( &brcm_key.ea, &key->key_ea, sizeof( struct ether_addr ) );
	brcm_key.len = key->key_len;

	if( brcm_key.len > sizeof( brcm_key.data ) )
		return EINVAL;


	UInt8 * ivptr = (UInt8 *)&key->key_rsc;
	brcm_key.rxiv.hi = (ivptr[5] << 24) | (ivptr[4] << 16) |
	(ivptr[3] << 8) | ivptr[2];
	brcm_key.rxiv.lo = (ivptr[1] << 8) | ivptr[0];
	brcm_key.iv_initialized = TRUE;

	if( key->key_len )
		memcpy( brcm_key.data, key->key, key->key_len );

	switch( key->key_cipher_type )
	{
		case APPLE80211_CIPHER_NONE:
			brcm_key.algo = CRYPTO_ALGO_OFF;
			break;
		case APPLE80211_CIPHER_WEP_40:
			brcm_key.algo = CRYPTO_ALGO_WEP1;
			break;
		case APPLE80211_CIPHER_WEP_104:
			brcm_key.algo = CRYPTO_ALGO_WEP128;
			break;
		case APPLE80211_CIPHER_TKIP:
			brcm_key.algo = CRYPTO_ALGO_TKIP;
			break;
		case APPLE80211_CIPHER_AES_CCM:
			brcm_key.algo = CRYPTO_ALGO_AES_CCM;
			break;
		default:
			return EINVAL;
	}

	if( key->key_flags & (APPLE80211_KEY_FLAG_DEFAULT | APPLE80211_KEY_FLAG_TX | APPLE80211_KEY_FLAG_RX ))
		brcm_key.flags |= WL_PRIMARY_KEY;
	if( key->key_flags & APPLE80211_KEY_FLAG_IBSS_PEER_GTK )
		brcm_key.flags |= WL_IBSS_PEER_GROUP_KEY;

#define ETHER_ISMULTI(ea) (((const uint8 *)(ea))[0] & 1)

//	IOLog( "AirPort_Brcm43xx::setSTA_WPA_Key: algo=%d, multi=%d, len=%d\n", brcm_key.algo, ETHER_ISMULTI( &brcm_key.ea ) ? 1 : 0, brcm_key.len );

	if( ETHER_ISMULTI( &brcm_key.ea ) )
		bzero( &brcm_key.ea, ETHER_ADDR_LEN );

#undef ETHER_ISMULTI

	int brcmerror = wlIovarOp( "wsec_key",
								 NULL,
								 0,
								 &brcm_key,
								 sizeof( brcm_key ),
								 IOV_SET,
								 NULL );

	return osl_error( brcmerror );
}
#endif /* VERSION_MAJOR > 9 */
#endif /* !MACOSX_DHD */

void
AirPort_Brcm43xx::SetCryptoKey( UInt8 *keyData, int keylen, int keyIdx, UInt8 *sc, bool txKey, struct ether_addr * ea )
{
//	ASSERT( keylen <= WLC_MAX_KEY_SIZE );
// 	if (keylen != 0)
// 		IOLog( "SetCryptoKey %c: len %d, idx %d\n", txKey ? 'T' : 'R', keylen, keyIdx );

	wl_wsec_key_t	key = {};
	bzero( &key, sizeof(key) );		// zero out unused fields

	key.len = keylen;
	key.index = keyIdx;

	if (keylen == 5)
		key.algo = CRYPTO_ALGO_WEP1;
	else if (keylen == 13)
		key.algo = CRYPTO_ALGO_WEP128;
	else if (keylen == kTkipGTKSize)
	{
		key.algo = CRYPTO_ALGO_TKIP;
//		if (txKey)
//		{
//			bcopy( authMacAdrs, &key.ea, 6 );
//		}
	}
	else if( keylen == 16 )
		key.algo = CRYPTO_ALGO_AES_CCM;
	else if (keylen != 0)
	{
		IOLog( "SetCryptoKey: bad keylen = %d\n", keylen );
		return;
	}
	else /* keylen == 0 */
	{
//		if (txKey)
//			pairwiseKeyPlumbed = false;		// pairwise key removed
	}

	if (keylen && txKey)
		key.flags = WL_PRIMARY_KEY;		// this is also the tx key

	if (sc) {
		key.iv_initialized = true;

#ifdef MFP
		if (IS_IGTK_INDEX(key.index)) {
			key.rxiv.lo = (sc[5] << 8) | sc[4];
			key.rxiv.hi = (sc[3] << 24) | (sc[2] << 16) | (sc[1] << 8) | sc[0];
		} else
#endif /* MFP */
		{
			key.rxiv.lo = (sc[1] << 8) | sc[0];
			key.rxiv.hi = (sc[5] << 24) | (sc[4] << 16) | (sc[3] << 8) | sc[2];
		}
	}

	if (keylen && keyData) {
		bcopy( keyData, key.data, keylen );
		memcpy(&key.data[16], &keyData[24], 8);
		memcpy(&key.data[24], &keyData[16], 8);
	}

	if (txKey && ea)
		memcpy(&key.ea, ea, sizeof( *ea ));

	(void) wlIoctl( WLC_SET_KEY, &key, sizeof(key), TRUE, NULL);
}

#ifdef APPLE80211_IOC_ACL_POLICY
SInt32
AirPort_Brcm43xx::setACL_POLICY( OSObject * interface, struct apple80211_acl_policy_data * pd )
{
	int	bcmerr = 0;
	uint32 macmode = 0;

	switch ( pd->policy )
	{
		case APPLE80211_ACL_POLICY_OFF:
			macmode = WLC_MACMODE_DISABLED;
			break;
		case APPLE80211_ACL_POLICY_WHITE_LIST:
			macmode = WLC_MACMODE_ALLOW;
			break;
		case APPLE80211_ACL_POLICY_BLACK_LIST:
			macmode = WLC_MACMODE_DENY;
			break;
		default:
			return EINVAL;
	}

	bcmerr = wlIoctl(WLC_SET_MACMODE, &macmode, sizeof( macmode ), TRUE, interface);

	return osl_error(bcmerr);
}

SInt32
AirPort_Brcm43xx::getACL_POLICY( OSObject * interface, struct apple80211_acl_policy_data * pd )
{
	int	bcmerr = 0;
	uint32 macmode = 0;

	bcmerr = wlIoctl(WLC_GET_MACMODE, &macmode, sizeof( macmode ), FALSE, interface);
	switch ( macmode )
	{
		case WLC_MACMODE_DISABLED:
			pd->policy = APPLE80211_ACL_POLICY_OFF;
			break;
		case WLC_MACMODE_ALLOW:
			pd->policy = APPLE80211_ACL_POLICY_WHITE_LIST;
			break;
		case WLC_MACMODE_DENY:
			pd->policy = APPLE80211_ACL_POLICY_BLACK_LIST;
			break;
		default:
			return EINVAL;
	}

	return osl_error(bcmerr);
}
#endif

#ifdef APPLE80211_IOC_ACL_ADD
SInt32
AirPort_Brcm43xx::setACL_ADD( OSObject * interface, struct apple80211_acl_entry_data * ad )
{
	struct maclist *list = NULL;
	size_t listSize = 0;
	int	bcmerr = 0;
	SInt32 err = BCME_OK;
	uint i = 0, j = 0;
	bool found = FALSE;

	listSize = OFFSETOF(struct maclist, ea) + ( MAXMACLIST * ETHER_ADDR_LEN );
	list = ( struct maclist * )MALLOCZ(osh, listSize);

	require_action( list , exit, err = ENOMEM );

	bcmerr = wlIoctl(WLC_GET_MACLIST, list, listSize, FALSE, interface);
	require_action( !bcmerr, exit, err = osl_error( bcmerr ) );

	/* merge dupes */
	for ( j = 0; j < ad->acl_list_count; j++ )
	{
		found = false;
		for( i = 0 ; i < list->count; i++ )
		{
			if ( !memcmp( &ad->acl_list[j], &list->ea[i], ETHER_ADDR_LEN ) )
			{
				found = true;
				break;
			}

		}
		if ( !found )
		{
			require_action( ( list->count < ( MAXMACLIST - 1 ) ), exit, err = ENOMEM );
			memcpy( &list->ea[i], &ad->acl_list[j], ETHER_ADDR_LEN );
			list->count++;
		}
	}
	bcmerr = wlIoctl(WLC_SET_MACLIST, list, listSize, TRUE, interface);
	require_action( !bcmerr, exit, err = osl_error( bcmerr ) );

exit:
	if ( list ) MFREE(osh, list, listSize);

	return err;
}
#endif

#ifdef APPLE80211_IOC_ACL_REMOVE
SInt32
AirPort_Brcm43xx::setACL_REMOVE( OSObject * interface, struct apple80211_acl_entry_data * ad )
{
	struct maclist *oldList = NULL, *newList = NULL;
	size_t listSize = 0;
	int	bcmerr = 0;
	SInt32 err = BCME_OK;
	uint i = 0, j = 0;
	bool found = 0;

	listSize = OFFSETOF(struct maclist, ea) + ( MAXMACLIST * ETHER_ADDR_LEN );
	oldList = ( struct maclist * )MALLOCZ(osh, listSize);
	newList = ( struct maclist * )MALLOCZ(osh, listSize);

	require_action( oldList && newList, exit, err = ENOMEM );

	newList->count = 0;

	bcmerr = wlIoctl(WLC_GET_MACLIST, oldList, listSize, FALSE, interface);
	require_action( !bcmerr, exit, err = osl_error( bcmerr ) );

	for( i = 0 ; i < oldList->count; i++ )
	{
		found = false;
		for ( j = 0 ; j < ad->acl_list_count; j++ )
		{
			if ( !memcmp( &ad->acl_list[j], &oldList->ea[i], ETHER_ADDR_LEN ) )
			{
				found = true;
				break;
			}

		}
		if ( !found )
		{
			memcpy( &newList->ea[newList->count], &oldList->ea[i], ETHER_ADDR_LEN );
			newList->count++;
		}
	}

	bcmerr = wlIoctl(WLC_SET_MACLIST, newList, listSize, TRUE, interface);
	require_action( !bcmerr, exit, err = osl_error( bcmerr ) );

exit:
	if ( oldList ) MFREE(osh, oldList, listSize);
	if ( newList ) MFREE(osh, newList, listSize);

	return err;
}
#endif /* APPLE80211_IOC_ACL_REMOVE */

#ifdef APPLE80211_IOC_ACL_FLUSH
SInt32
AirPort_Brcm43xx::setACL_FLUSH( OSObject * interface )
{
	struct maclist list = {};
	int	bcmerr = 0;

	list.count = 0;

	bcmerr = wlIoctl(WLC_SET_MACLIST, &list, sizeof( list ), TRUE, interface);

	return osl_error(bcmerr);
}
#endif /* APPLE80211_IOC_ACL_FLUSH */

#ifdef APPLE80211_IOC_ACL_LIST
SInt32
AirPort_Brcm43xx::getACL_LIST( OSObject * interface, struct apple80211_acl_entry_data * ad )
{
	struct maclist *list = NULL;
	size_t listSize = 0;
	int	bcmerr = 0;
	SInt32 err = BCME_OK;
	uint i = 0;

	listSize = OFFSETOF(struct maclist, ea) + ( MAXMACLIST * ETHER_ADDR_LEN );
	list = ( struct maclist * )MALLOCZ(osh, listSize);

	require_action( list , exit, err = ENOMEM );

	bcmerr = wlIoctl(WLC_GET_MACLIST, list, listSize, FALSE, interface);
	require_action( !bcmerr, exit, err = osl_error( bcmerr ) );

	for( i = 0 ; i < MIN( list->count, APPLE80211_MAX_ACL_ENTRIES ); i++ )
	{
		memcpy( &ad->acl_list[i], &list->ea[i], ETHER_ADDR_LEN );
	}
	ad->acl_list_count = MIN( list->count, APPLE80211_MAX_ACL_ENTRIES );

exit:
	if ( list ) MFREE(osh, list, listSize);

	return err;
}
#endif /* APPLE80211_IOC_ACL_LIST */

SInt32
AirPort_Brcm43xx::stopDMA()
{
#ifndef MACOSX_DHD
	if (pWLC == NULL)
		return 0;
#endif /* !MACOSX_DHD */

	struct apple80211_power_data pd = { APPLE80211_VERSION, 0, {0}};

	pd.num_radios = radioCountForInterface( myNetworkIF );

	for( UInt32 i = 0; i < pd.num_radios; i++ )
		pd.power_state[i] = APPLE80211_POWER_OFF;

	WL_LOCK();
	SInt32 result = setPOWER( myNetworkIF, &pd );
	WL_UNLOCK();

	return result;
}

// dynamic 802.11n
SInt32
AirPort_Brcm43xx::enableFeature( IO80211FeatureCode feature, void * refcon )
{
	SInt32 err = 0;
	BCM_REFERENCE(refcon);

	if (feature == kIO80211Feature80211n) {
		if (PHYTYPE_11N_CAP(phytype))
			enable11n = true;
		else
			err = EOPNOTSUPP;
	} else {
		err = EOPNOTSUPP;
	}

	return err;
}

#ifdef BCMDBG
void
AirPort_Brcm43xx::dump_apple80211_channels(uint count, struct apple80211_channel *channels)
{
	struct apple80211_channel * chan = NULL;
	const char *band = NULL;
	const char *bw = NULL;
	const char *extension = NULL;
	uint i = 0;

	for (i = 0; i < count; i++ ) {
		chan = &channels[i];
		if (chan->flags & APPLE80211_C_FLAG_2GHZ)
			band = "2.4 GHz";
		else if (chan->flags & APPLE80211_C_FLAG_5GHZ)
			band = "5 GHz";
		else
			band = "? GHz";

		bw = (chan->flags & APPLE80211_C_FLAG_10MHZ)?", 10 MHz":
		        ((chan->flags & APPLE80211_C_FLAG_40MHZ)?", 40 MHz": "");
		if (chan->flags & APPLE80211_C_FLAG_40MHZ)
			extension = (chan->flags & APPLE80211_C_FLAG_EXT_ABV) ?
			        " ext +1" : " ext -1";
		else
			extension = "";

		printf("%d (%s%s%s)\n", chan->channel, band, bw, extension);
	}
}

void
AirPort_Brcm43xx::dump_apple80211_network_data(struct apple80211_network_data * nd)
{
	char ssidbuf[SSID_FMT_BUF_LEN] = {};
	const char *bw = NULL;
	const char *band = NULL;

	switch (nd->nd_channel.flags & (APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_5GHZ)) {
	case APPLE80211_C_FLAG_2GHZ:
		band = "2.4 GHz";
		break;
	case APPLE80211_C_FLAG_5GHZ:
		band = "5 GHz";
		break;
	case 0:
		band = "bandflag=0";
		break;
	case (APPLE80211_C_FLAG_2GHZ | APPLE80211_C_FLAG_5GHZ):
		band = "bandflag=2+5";
		break;
	default:
		band = "<bandflag-unknown>";
		break;
	}

	switch (nd->nd_channel.flags &
	        (APPLE80211_C_FLAG_10MHZ | APPLE80211_C_FLAG_20MHZ | APPLE80211_C_FLAG_40MHZ)) {
	case APPLE80211_C_FLAG_40MHZ:
		bw = "40MHz";
		break;
	case APPLE80211_C_FLAG_20MHZ:
		bw = "20MHz";
		break;
	case APPLE80211_C_FLAG_10MHZ:
		bw = "10MHz";
		break;
	default:
		bw = "Multiple BW flags";
		break;
	}

	printf("  %-18s%s\n", "mode", lookup_name(apple80211_apmode_names, nd->nd_mode));
	printf("  %-18s%s\n", "auth_lower",
	       lookup_name(apple80211_authtype_lower_names, nd->nd_auth_lower));
	printf("  %-18s%s\n", "auth_upper",
	       lookup_name(apple80211_authtype_upper_names, nd->nd_auth_upper));
	printf("  %-18s%u (%s, %s)\n", "channel", nd->nd_channel.channel, band, bw);
#ifndef MACOSX_DHD
	wlc_format_ssid(ssidbuf, nd->nd_ssid, nd->nd_ssid_len);
#else
	wl_format_ssid(ssidbuf, nd->nd_ssid, nd->nd_ssid_len);
#endif /* !MACOSX_DHD */
	printf("  %-18s\"%s\"\n", "ssid", ssidbuf);
	dump_apple80211_key(&nd->nd_key);
}

void
AirPort_Brcm43xx::dump_apple80211_key(struct apple80211_key * key)
{
	uint i = 0;
	const bcm_bit_desc_t key_flags[] = {
		{APPLE80211_KEY_FLAG_UNICAST, "unicast"},
		{APPLE80211_KEY_FLAG_MULTICAST, "multicast"},
		{APPLE80211_KEY_FLAG_TX, "tx"},
		{APPLE80211_KEY_FLAG_RX, "rx"},
		{0, NULL}
	};
	char flagstr[64] = {0};
	char eabuf[ETHER_ADDR_STR_LEN] = {};
	char *data_str = NULL;
	uint data_str_len = 0;

	printf("  %-18s%s\n", "ea", bcm_ether_ntoa(&key->key_ea, eabuf));
	printf("  %-18s%u\n", "index", (uint)key->key_index);
	printf("  %-18s%s\n", "cipher",
	       lookup_name(apple80211_cipher_type_names, key->key_cipher_type));
	bcm_format_flags(key_flags, key->key_flags, flagstr, 64);
	printf("  %-18s%s\n", "flags",  flagstr);

	// Allocate a string buffer for both key data and rsc
	data_str_len = 1 + 2*MAX(key->key_len, key->key_rsc_len);
	data_str = (char*)MALLOC(osh, data_str_len);
	if (data_str == NULL)
		return;

	if (key->key_len == 0)
		printf("  data(len 0)\n");
	else {
		bcm_format_hex(data_str, key->key, key->key_len);
		printf("  data(len %2d)      0x%s\n", key->key_len, data_str);
		for (i = 0; i < key->key_len; i++)
			if (!bcm_isprint(key->key[i]))
				break;
		if (key->key_len && i == key->key_len)
			printf("                    (%.*s)\n", (int)key->key_len, key->key);
	}

	if (key->key_rsc_len == 0)
		printf("  rsc(len 0)\n");
	else {
		bcm_format_hex(data_str, key->key_rsc, key->key_rsc_len);
		printf("  rsc(len %2d)       0x%s\n", key->key_rsc_len, data_str);
	}

	MFREE(osh, data_str, data_str_len);
}
#endif /* BCMDBG */

#pragma mark -

#if VERSION_MAJOR > 8

SInt32
AirPort_Brcm43xx::requestCheck( UInt32 req, int type )
{
	pid_t pid = 0;
	static char processName[128] = "<unknown>"; /* static - only one entry, single threaded */
	pid = proc_selfpid();
	proc_selfname( processName, sizeof( processName ) );

	BCM_REFERENCE(req);
	BCM_REFERENCE(type);

#ifdef AAPLPWROFF
	switch (type) {
		/* Allow these requests always */
		case APPLE80211_IOC_CARD_CAPABILITIES:
		case APPLE80211_IOC_DRIVER_VERSION:
		case APPLE80211_IOC_BTCOEX_MODE:
		case APPLE80211_IOC_RADIO_INFO:
			return 0;
	}

	// Do not allow any ioctl request while setPOWER is sleeping
	// waiting for setPowerstate or recovery is in progress
	if( _powerSleep || wl_powercycle_inprogress((struct wl_info *)this))
	{
#if defined(BCMDBG) || defined(ENABLE_CORECAPTURE)
		DEBUGLOG(("apple80211Request %s \"%s\" rejected while _powerSleep:%d, powerCycleInProgress:%d, pid[%d]'%s'\n",
			(req == SIOCGA80211)? "Get":"Set",
			lookup_name(apple80211_ionames, type), _powerSleep,
			wl_powercycle_inprogress((struct wl_info *)this), pid, processName));
#endif /* BCMDBG || ENABLE_CORECAPTURE */
		return ENXIO;
	}

	if (_powerState != PS_INDEX_ON) {
		/* Following gets try to access the hardware */
		switch (type) {
			case APPLE80211_IOC_ROM:
			case APPLE80211_IOC_ANTENNA_DIVERSITY:
			case APPLE80211_IOC_RX_ANTENNA:
			case APPLE80211_IOC_RAND:
#if defined(BCMDBG) || defined(ENABLE_CORECAPTURE)
				DEBUGLOG(("apple80211Request %s \"%s\" rejected while _powerState:%d, pid[%d]'%s'\n",
					(req == SIOCGA80211)? "Get":"Set",
					lookup_name(apple80211_ionames, type), _powerState, pid, processName));
#endif /* BCMDBG || ENABLE_CORECAPTURE */
				return EPWROFF;
		}

		/* Don't allow ANY sets other than setPOWER and BTCOEX during power off */
		if (type != APPLE80211_IOC_POWER &&
			type != APPLE80211_IOC_POWERSAVE &&
			type != APPLE80211_IOC_BTCOEX_MODE &&
#ifdef IO80211P2P
			type != APPLE80211_IOC_VIRTUAL_IF_CREATE &&
			type != APPLE80211_IOC_VIRTUAL_IF_DELETE &&
#endif
			type !=	 APPLE80211_IOC_SUPPORTED_CHANNELS &&

			type != APPLE80211_IOC_VENDOR_DBG_FLAGS	 &&
#ifdef SUPPORT_FEATURE_WOW
			type != APPLE80211_IOC_WOW_PARAMETERS &&
#endif
			TRUE) { /* Do not allow all other GET requests when device is in D3,
				 * some of them may try to touch HW eg, getNOISE() radar://20956861 */
#if defined(BCMDBG) || defined(ENABLE_CORECAPTURE)
			DEBUGLOG(("apple80211Request %s \"%s\" rejected while _powerState:%d, pid[%d]'%s'\n",
				(req == SIOCGA80211)? "Get":"Set",
				lookup_name(apple80211_ionames, type), _powerState, pid, processName));
#endif /* BCMDBG || ENABLE_CORECAPTURE */
			return EPWROFF;
		}

#if defined(BCMDBG) || defined(ENABLE_CORECAPTURE)
		DEBUGLOG(("apple80211Request %s \"%s\" while off, pid[%d]'%s'\n",
		          (req == SIOCGA80211)? "Get":"Set",
		          lookup_name(apple80211_ionames, type), pid, processName));
#endif /* BCMDBG || ENABLE_CORECAPTURE */
	}
#endif /* AAPLPWROFF */

	return 0;
}

SInt32
AirPort_Brcm43xx::apple80211Request( UInt32 req, int type, IO80211Interface * intf, void * data )
{
	SInt32 ret = EOPNOTSUPP;

#if VERSION_MAJOR >= 15
	if(getWorkLoop()->onThread()) {
		WL_ERROR(("%s: onThread", __FUNCTION__));
	}
#endif /* VERSION_MAJOR >= 15 */

	if( req != SIOCGA80211 && req != SIOCSA80211 )
		return EINVAL;

	// Check if the device is in a state to handle this request
	if( ( ret = requestCheck( req, type ) ) != 0 )
		return ret;

	WL_LOCK();

	switch( type )
	{
		case APPLE80211_IOC_SSID:
			GETSET( SSID, apple80211_ssid_data );
		case APPLE80211_IOC_AUTH_TYPE:
			GETSET( AUTH_TYPE, apple80211_authtype_data );
		case APPLE80211_IOC_CIPHER_KEY:
			SET( CIPHER_KEY, apple80211_key );
		case APPLE80211_IOC_CHANNEL:
			GETSET( CHANNEL, apple80211_channel_data );
		case APPLE80211_IOC_POWERSAVE:
			GETSET( POWERSAVE, apple80211_powersave_data );
		case APPLE80211_IOC_PROTMODE:
			GETSET( PROTMODE, apple80211_protmode_data );
		case APPLE80211_IOC_TXPOWER:
			GETSET( TXPOWER, apple80211_txpower_data );
		case APPLE80211_IOC_RATE:
			GETSET( RATE, apple80211_rate_data );
		case APPLE80211_IOC_BSSID:
			GETSET( BSSID, apple80211_bssid_data );
		case APPLE80211_IOC_SCAN_REQ:
			SET( SCAN_REQ, apple80211_scan_data );
		case APPLE80211_IOC_SCAN_RESULT:
			GET( SCAN_RESULT, apple80211_scan_result* );
		case APPLE80211_IOC_CARD_CAPABILITIES:
			GET( CARD_CAPABILITIES, apple80211_capability_data );
		case APPLE80211_IOC_STATE:
			GET( STATE, apple80211_state_data );
		case APPLE80211_IOC_PHY_MODE:
			GETSET( PHY_MODE, apple80211_phymode_data );
		case APPLE80211_IOC_OP_MODE:
			GET( OP_MODE, apple80211_opmode_data );
		case APPLE80211_IOC_RSSI:
			GET( RSSI, apple80211_rssi_data );
		case APPLE80211_IOC_NOISE:
			GET( NOISE, apple80211_noise_data );
		case APPLE80211_IOC_INT_MIT:
			GETSET( INT_MIT, apple80211_intmit_data );
		case APPLE80211_IOC_POWER:
			GETSET( POWER, apple80211_power_data );
		case APPLE80211_IOC_ASSOCIATE:
			SET( ASSOCIATE, apple80211_assoc_data );
		case APPLE80211_IOC_DISASSOCIATE:
			if( req == SIOCSA80211 )
				ret = setDISASSOCIATE( intf );
			break;
		case APPLE80211_IOC_STATUS_DEV_NAME:
			GET( STATUS_DEV, apple80211_status_dev_data );
		case APPLE80211_IOC_SUPPORTED_CHANNELS:
			GET( SUPPORTED_CHANNELS, apple80211_sup_channel_data );
		case APPLE80211_IOC_LOCALE:
			GETSET( LOCALE, apple80211_locale_data );
		case APPLE80211_IOC_DEAUTH:
			GETSET( DEAUTH, apple80211_deauth_data );
		case APPLE80211_IOC_FRAG_THRESHOLD:
			GETSET( FRAG_THRESHOLD, apple80211_frag_threshold_data );
		case APPLE80211_IOC_RATE_SET:
			GETSET( RATE_SET, apple80211_rate_set_data );
		case APPLE80211_IOC_SHORT_SLOT:
			GETSET( SHORT_SLOT, apple80211_short_slot_data );
		case APPLE80211_IOC_SHORT_RETRY_LIMIT:
			GETSET( SHORT_RETRY_LIMIT, apple80211_retry_limit_data );
		case APPLE80211_IOC_LONG_RETRY_LIMIT:
			GETSET( LONG_RETRY_LIMIT, apple80211_retry_limit_data );
		case APPLE80211_IOC_TX_ANTENNA:
			GETSET( TX_ANTENNA, apple80211_antenna_data );
		case APPLE80211_IOC_RX_ANTENNA:
			GET( RX_ANTENNA, apple80211_antenna_data );
		case APPLE80211_IOC_ANTENNA_DIVERSITY:
			GETSET( ANTENNA_DIVERSITY, apple80211_antenna_data );
		case APPLE80211_IOC_DTIM_INT:
			GETSET( DTIM_INT, apple80211_dtim_int_data );
		case APPLE80211_IOC_STATION_LIST:
			GET( STATION_LIST, apple80211_sta_data );
		case APPLE80211_IOC_DRIVER_VERSION:
			GET( DRIVER_VERSION, apple80211_version_data );
		case APPLE80211_IOC_HARDWARE_VERSION:
			GET( HARDWARE_VERSION, apple80211_version_data );
		case APPLE80211_IOC_AP_IE_LIST:
			GET( AP_IE_LIST, apple80211_ap_ie_data );
		case APPLE80211_IOC_ASSOCIATION_STATUS:
			GET( ASSOCIATION_STATUS, apple80211_assoc_status_data );
		case APPLE80211_IOC_COUNTRY_CODE:
			GETSET( COUNTRY_CODE, apple80211_country_code_data );
		case APPLE80211_IOC_LAST_RX_PKT_DATA:
			GET( LAST_RX_PKT_DATA, apple80211_last_rx_pkt_data );
		case APPLE80211_IOC_RADIO_INFO:
			GET( RADIO_INFO, apple80211_radio_info_data );
		case APPLE80211_IOC_GUARD_INTERVAL:
			GETSET( GUARD_INTERVAL, apple80211_guard_interval_data );
		case APPLE80211_IOC_MSDU:
			GETSET( MSDU, apple80211_msdu_data );
		case APPLE80211_IOC_MPDU:
			GETSET( MPDU, apple80211_mpdu_data );
		case APPLE80211_IOC_BLOCK_ACK:
			GETSET( BLOCK_ACK, apple80211_block_ack_data );
#ifndef MACOSX_DHD
		case APPLE80211_IOC_STATS:
			GET( STATS, apple80211_stats_data );
		case APPLE80211_IOC_SCAN_REQ_MULTIPLE:
			SET( SCAN_REQ_MULTIPLE, apple80211_scan_multiple_data );
		case APPLE80211_IOC_IBSS_MODE:
			SET( IBSS_MODE, apple80211_network_data );
#endif /* MACOSX_DHD */
		case APPLE80211_IOC_HOST_AP_MODE:
			SET( HOST_AP_MODE, apple80211_network_data );
#ifndef MACOSX_DHD
		case APPLE80211_IOC_AP_MODE:
			GETSET( AP_MODE, apple80211_apmode_data );
		case APPLE80211_IOC_COUNTERMEASURES:
			SET( COUNTERMEASURES, apple80211_countermeasures_data );
		case APPLE80211_IOC_MULTICAST_RATE:
			GETSET( MULTICAST_RATE, apple80211_rate_data );
		case APPLE80211_IOC_ROM:
			GET( ROM, apple80211_rom_data );
		case APPLE80211_IOC_RAND:
			GET( RAND, apple80211_rand_data );
		case APPLE80211_IOC_RSN_IE:
			GETSET( RSN_IE, apple80211_rsn_ie_data );
		case APPLE80211_IOC_MIMO_POWERSAVE:
			GET( MIMO_POWERSAVE, apple80211_powersave_data );
		case APPLE80211_IOC_MCS:
			GETSET( MCS, apple80211_mcs_data );
		case APPLE80211_IOC_RIFS:
			GETSET( RIFS, apple80211_rifs_data );
		case APPLE80211_IOC_LDPC:
			GETSET( LDPC, apple80211_ldpc_data );
		case APPLE80211_IOC_PLS:
			GETSET( PLS, apple80211_pls_data );
		case APPLE80211_IOC_PSMP:
			GETSET( PSMP, apple80211_psmp_data );
		case APPLE80211_IOC_PHY_SUB_MODE:
			GETSET( PHY_SUB_MODE, apple80211_physubmode_data );
		case APPLE80211_IOC_MCS_INDEX_SET:
			GET( MCS_INDEX_SET, apple80211_mcs_index_set_data );
#ifdef SUPPORT_FEATURE_WOW
		case APPLE80211_IOC_WOW_PARAMETERS:
			GETSET( WOW_PARAMETERS, apple80211_wow_parameter_data );
#endif
#if VERSION_MAJOR > 9
		case APPLE80211_IOC_ROAM_THRESH:
			GETSET( ROAM_THRESH, apple80211_roam_threshold_data );
		case APPLE80211_IOC_LINK_QUAL_EVENT_PARAMS:
			GETSET( LINK_QUAL_EVENT_PARAMS, apple80211_link_qual_event_data);
		case APPLE80211_IOC_VENDOR_DBG_FLAGS:
			GETSET( VENDOR_DBG_FLAGS, apple80211_vendor_dbg_data);
		// Apple change: APPLE80211_IOC_STA_IE_LIST, APPLE80211_IOC_STA_DEAUTH, APPLE80211_IOC_KEY_RSC,
		//		 APPLE80211_IOC_STA_AUTHORIZE, APPLE80211_IOC_RSN_CONF, and APPLE80211_IOC_STA_STATS
		//				 are for hostapd support.
		case APPLE80211_IOC_STA_IE_LIST:
			GET( STA_IE_LIST, apple80211_sta_ie_data );
		case APPLE80211_IOC_STA_DEAUTH:
			SET( STA_DEAUTH, apple80211_sta_deauth_data );
		case APPLE80211_IOC_KEY_RSC:
			GET( KEY_RSC, apple80211_key );
		case APPLE80211_IOC_STA_AUTHORIZE:
			SET( STA_AUTHORIZE, apple80211_sta_authorize_data );
		case APPLE80211_IOC_RSN_CONF:
			SET( RSN_CONF, apple80211_rsn_conf_data );
		case APPLE80211_IOC_STA_STATS:
			GET( STA_STATS, apple80211_sta_stats_data );
// Apple change: Add get handler for APPLE80211_IOC_BTCOEX_MODE
		case APPLE80211_IOC_BTCOEX_MODE:
			GETSET(BTCOEX_MODE, apple80211_btc_mode_data);
#ifdef APPLEIOCIE
		case APPLE80211_IOC_IE:
			GETSET( IE, apple80211_ie_data);
#endif /* APPLEIOCIE */
#ifdef IO80211P2P
		case APPLE80211_IOC_P2P_LISTEN:
			SET( P2P_LISTEN, apple80211_p2p_listen_data );
		case APPLE80211_IOC_P2P_SCAN:
			SET( P2P_SCAN, apple80211_scan_data );
		case APPLE80211_IOC_VIRTUAL_IF_CREATE:
			SET( VIRTUAL_IF_CREATE, apple80211_virt_if_create_data );
		case APPLE80211_IOC_VIRTUAL_IF_DELETE:
			SET( VIRTUAL_IF_DELETE, apple80211_virt_if_delete_data );
#endif /* IO80211P2P */
#endif /* VERSION_MAJOR > 9 */

#if VERSION_MAJOR > 10
#ifdef APPLE80211_IOC_DESENSE
		case APPLE80211_IOC_DESENSE:
			GETSET( DESENSE, apple80211_desense_data );
#endif /* APPLE80211_IOC_DESENSE */
#ifdef APPLE80211_IOC_DESENSE_LEVEL
        case APPLE80211_IOC_DESENSE_LEVEL:
			GETSET( DESENSE_LEVEL, apple80211_desense_level_data );
#endif
#ifdef APPLE80211_IOC_CHAIN_ACK
		case APPLE80211_IOC_CHAIN_ACK:
			GETSET( CHAIN_ACK, apple80211_chain_ack );
#endif /* APPLE80211_IOC_CHAIN_ACK */
#ifdef OPPORTUNISTIC_ROAM
		case APPLE80211_IOC_ROAM:
			SET( ROAM, apple80211_sta_roam_data );
#endif /* OPPORTUNISTIC_ROAM */
#ifdef IO80211P2P
		case APPLE80211_IOC_SCANCACHE_CLEAR:
			if( req != SIOCGA80211 )
				ret = setSCANCACHE_CLEAR( intf );
			break;
#endif // IO80211P2P
#endif /* VERSION_MAJOR > 10 */
#ifdef APPLE80211_IOC_TX_CHAIN_POWER
		case APPLE80211_IOC_TX_CHAIN_POWER:
			if( req == SIOCGA80211 )
				ret = getTX_CHAIN_POWER(intf, (struct apple80211_chain_power_data_get *)data);
			else
				ret = setTX_CHAIN_POWER(intf, (struct apple80211_chain_power_data_set *)data);
		break;
#endif
#ifdef FACTORY_MERGE
		case APPLE80211_IOC_FACTORY_MODE:
			GETSET( FACTORY_MODE, apple80211_factory_mode_data );
#endif
#ifdef APPLE80211_IOC_CDD_MODE
		case APPLE80211_IOC_CDD_MODE:
			GETSET( CDD_MODE, apple80211_cdd_mode_data );
#endif
#ifdef APPLE80211_IOC_THERMAL_THROTTLING
		case APPLE80211_IOC_THERMAL_THROTTLING:
			GETSET( THERMAL_THROTTLING, apple80211_thermal_throttling );
#endif
#ifdef APPLE80211_IOC_REASSOCIATE
		case APPLE80211_IOC_REASSOCIATE:
			if( req == SIOCSA80211 )
				ret = setREASSOCIATE( intf );
			break;
#endif
#ifdef APPLE80211_IOC_ACL_POLICY
		case APPLE80211_IOC_ACL_POLICY:
			GETSET( ACL_POLICY, apple80211_acl_policy_data );
#endif
#ifdef APPLE80211_IOC_ACL_ADD
		case APPLE80211_IOC_ACL_ADD:
			if ( req == SIOCSA80211 )
				ret = setACL_ADD( intf, (struct apple80211_acl_entry_data *)data );
			break;
#endif
#ifdef APPLE80211_IOC_ACL_REMOVE
		case APPLE80211_IOC_ACL_REMOVE:
			if ( req == SIOCSA80211 )
				ret = setACL_REMOVE( intf, (struct apple80211_acl_entry_data *)data );
			break;
#endif
#ifdef APPLE80211_IOC_ACL_FLUSH
		case APPLE80211_IOC_ACL_FLUSH:
			if ( req == SIOCSA80211 )
				ret = setACL_FLUSH( intf );
			break;
#endif
#ifdef APPLE80211_IOC_ACL_LIST
		case APPLE80211_IOC_ACL_LIST:
			if ( req == SIOCGA80211 )
				ret = getACL_LIST( intf, (struct apple80211_acl_entry_data *)data );
			break;
#endif
#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
		case APPLE80211_IOC_VHT_MCS_INDEX_SET:
			GET( VHT_MCS_INDEX_SET, apple80211_vht_mcs_index_set_data );
#endif
#ifdef APPLE80211_IOC_MCS_VHT
		case APPLE80211_IOC_MCS_VHT:
			GETSET( MCS_VHT, apple80211_mcs_vht_data );
#endif
#ifdef APPLE80211_IOC_INTERRUPT_STATS
		case APPLE80211_IOC_INTERRUPT_STATS:
			GET( INTERRUPT_STATS, apple80211_interrupt_stats_data );
#endif
#ifdef APPLE80211_IOC_TIMER_STATS
		case APPLE80211_IOC_TIMER_STATS:
			GET( TIMER_STATS, apple80211_timer_stats_data );
#endif
#ifdef APPLE80211_IOC_OFFLOAD_STATS
		case APPLE80211_IOC_OFFLOAD_STATS:
			GET( OFFLOAD_STATS, apple80211_offload_stats_data );
		case APPLE80211_IOC_OFFLOAD_STATS_RESET:
			if( req == SIOCSA80211 )
				ret = setOFFLOAD_STATS_RESET( intf );
			break;
#endif
#ifdef APPLE80211_IOC_OFFLOAD_ARP
		case APPLE80211_IOC_OFFLOAD_ARP:
			GETSET( OFFLOAD_ARP, apple80211_offload_arp_data );
			break;
#endif
#ifdef APPLE80211_IOC_OFFLOAD_NDP
		case APPLE80211_IOC_OFFLOAD_NDP:
			GETSET( OFFLOAD_NDP, apple80211_offload_ndp_data );
			break;
#endif
#ifdef APPLE80211_IOC_OFFLOAD_SCAN
		case APPLE80211_IOC_OFFLOAD_SCAN:
			GETSET( OFFLOAD_SCAN, apple80211_offload_scan_data );
			break;
#endif
#ifdef APPLE80211_IOC_TX_NSS
		case APPLE80211_IOC_TX_NSS:
			GETSET( TX_NSS, apple80211_tx_nss_data );
			break;
#endif
#if defined(WLBTCPROF) && defined(APPLE80211_IOC_BTCOEX_PROFILES)
		case APPLE80211_IOC_BTCOEX_PROFILES:
			GETSET( BTCOEX_PROFILES, apple80211_btc_profiles_data );
			break;
		case APPLE80211_IOC_BTCOEX_CONFIG:
			GETSET( BTCOEX_CONFIG, apple80211_btc_config_data );
			break;
#endif /* WLBTCPROF && APPLE80211_IOC_BTCOEX_PROFILES */
#ifdef APPLE80211_IOC_ROAM_PROFILE
		case APPLE80211_IOC_ROAM_PROFILE:
			GETSET( ROAM_PROFILE, apple80211_roam_profile_band_data );
			break;
#endif
#ifdef APPLE80211_IOC_BTCOEX_OPTIONS
		case APPLE80211_IOC_BTCOEX_OPTIONS:
			GETSET( BTCOEX_OPTIONS, apple80211_btc_options_data );
			break;
#endif /* APPLE80211_IOC_BTCOEX_OPTIONS */
#ifdef APPLE80211_IOC_LINK_CHANGED_EVENT_DATA
		case APPLE80211_IOC_LINK_CHANGED_EVENT_DATA:
			GET( LINK_CHANGED_EVENT_DATA, apple80211_link_changed_event_data);
			break;
#endif
#endif /* !MACOSX_DHD */
#ifdef APPLE80211_IOC_SUPPORTED_COUNTRY_CODES
		case APPLE80211_IOC_SUPPORTED_COUNTRY_CODES:
			GET( SUPPORTED_COUNTRY_CODES, apple80211_supported_country_codes_data);
			break;
#endif /* APPLE80211_IOC_SUPPORTED_COUNTRY_CODES */
#ifdef APPLE80211_IOC_COUNTRY_TX_POWER_LIMITS
		case APPLE80211_IOC_COUNTRY_TX_POWER_LIMITS:
			GET( COUNTRY_TX_POWER_LIMITS, apple80211_country_tx_power_limits_data);
			break;
#endif /* APPLE80211_IOC_COUNTRY_TX_POWER_LIMITS */
#ifdef APPLE80211_IOC_HW_SUPPORTED_CHANNELS
		case APPLE80211_IOC_HW_SUPPORTED_CHANNELS:
			GET( HW_SUPPORTED_CHANNELS, apple80211_sup_channel_data );
			break;
#endif /* APPLE80211_IOC_HW_SUPPORTED_CHANNELS */

		default:
			ret = EOPNOTSUPP;
		break;
	}

	if (ret) {
		WL_PORT(("%s: ioctl %s: ret %d\n", __FUNCTION__,
		         lookup_name(apple80211_ionames, type), (int)ret));
	}

	WL_UNLOCK();

	return ret;
}

#ifdef IO80211P2P
SInt32
AirPort_Brcm43xx::apple80211VirtualRequest( UInt32 req, int type, IO80211VirtualInterface * intf, void * data )
{
	SInt32 ret = 0;

#if VERSION_MAJOR >= 15
	if(getWorkLoop()->onThread()) {
		WL_ERROR(("%s: onThread", __FUNCTION__));
	}
#endif /* VERSION_MAJOR >= 15 */

	if( req != SIOCGA80211 && req != SIOCSA80211 )
		return EINVAL;

	// Check if the device is in a state to handle this request
	if( ( ret = requestCheck( req, type ) ) != 0 )
		return ret;

	WL_LOCK();

	switch ( type )
	{
		case APPLE80211_IOC_DRIVER_VERSION:
			GET( DRIVER_VERSION, apple80211_version_data );
		case APPLE80211_IOC_CARD_CAPABILITIES:
			GET( CARD_CAPABILITIES, apple80211_capability_data );
		case APPLE80211_IOC_IE:
			GETSET( IE, apple80211_ie_data );
#ifdef IO80211P2P
		case APPLE80211_IOC_P2P_LISTEN:
			SET( P2P_LISTEN, apple80211_p2p_listen_data );
		case APPLE80211_IOC_P2P_SCAN:
			SET( P2P_SCAN, apple80211_scan_data );
#endif /* IO80211P2P */
		case APPLE80211_IOC_SUPPORTED_CHANNELS:
			GET( SUPPORTED_CHANNELS, apple80211_sup_channel_data );
#ifdef IO80211P2P
		case APPLE80211_IOC_P2P_GO_CONF:
			SET( P2P_GO_CONF, apple80211_p2p_go_conf_data );
#if VERSION_MAJOR > 10
		case APPLE80211_IOC_P2P_OPP_PS:
			GETSET( P2P_OPP_PS, apple80211_opp_ps_data );
		case APPLE80211_IOC_P2P_CT_WINDOW:
			GETSET( P2P_CT_WINDOW, apple80211_ct_window_data );
#endif /* VERSION_MAJOR > 10 */
#endif /* IO80211P2P */
		case APPLE80211_IOC_VIRTUAL_IF_CREATE:
			SET( VIRTUAL_IF_CREATE, apple80211_virt_if_create_data );
		case APPLE80211_IOC_VIRTUAL_IF_DELETE:
			SET( VIRTUAL_IF_DELETE, apple80211_virt_if_delete_data );
		case APPLE80211_IOC_ASSOCIATE:
			SET( ASSOCIATE, apple80211_assoc_data );
		case APPLE80211_IOC_ASSOCIATION_STATUS:
			GET( ASSOCIATION_STATUS, apple80211_assoc_status_data );
		case APPLE80211_IOC_POWER:
			GET( POWER, apple80211_power_data );
		case APPLE80211_IOC_STATE:
			GET( STATE, apple80211_state_data );
		case APPLE80211_IOC_OP_MODE:
			GET( OP_MODE, apple80211_opmode_data );
		case APPLE80211_IOC_AUTH_TYPE:
			GETSET( AUTH_TYPE, apple80211_authtype_data );
		case APPLE80211_IOC_DISASSOCIATE:
			if( req == SIOCSA80211 )
			    ret = setDISASSOCIATE( intf );
			break;
		case APPLE80211_IOC_SSID:
			GETSET( SSID, apple80211_ssid_data );
		case APPLE80211_IOC_CHANNEL:
			GETSET( CHANNEL, apple80211_channel_data );
		case APPLE80211_IOC_BSSID:
			GETSET( BSSID, apple80211_bssid_data );
		case APPLE80211_IOC_RSSI:
			GET( RSSI, apple80211_rssi_data );
		case APPLE80211_IOC_RATE:
			GET( RATE, apple80211_rate_data );
		case APPLE80211_IOC_CIPHER_KEY:
			SET( CIPHER_KEY, apple80211_key );
		case APPLE80211_IOC_AP_MODE:
			GETSET( AP_MODE, apple80211_apmode_data );
		default:
			ret = EOPNOTSUPP;
	}

	WL_UNLOCK();

	if (ret)
		WL_PORT(("%s: type %d ioctl %s: ret %d\n", __FUNCTION__, type, lookup_name(apple80211_ionames, type), (int)ret));

	return ret;
}
#endif /* IO80211P2P */
#undef GETSET
#undef GET
#undef SET

#endif /* VERSION_MAJOR > 8 */

// -----------------------------------------------------------------------------
static UInt32
WlPrio2MacOSPrio(int prio)
{
	int ac = 0;
	switch (prio) {
	case PRIO_8021D_NONE:
	case PRIO_8021D_BK:
		ac = APPLE80211_WME_AC_BK;
		break;
	case PRIO_8021D_BE:
	case PRIO_8021D_EE:
		ac = APPLE80211_WME_AC_BE;
		break;
	case PRIO_8021D_CL:
	case PRIO_8021D_VI:
		ac = APPLE80211_WME_AC_VI;
		break;
	case PRIO_8021D_VO:
	case PRIO_8021D_NC:
		ac = APPLE80211_WME_AC_VO;
		break;
	default:
		ASSERT(0);
	};
	return ac;
}

void
AirPort_Brcm43xx::chanspec2applechannel(chanspec_t chanspec, struct apple80211_channel *channel)
{
	channel->version = APPLE80211_VERSION;
	channel->channel = wf_chspec_ctlchan(chanspec);
	channel->flags = CHSPEC_IS2G(chanspec) ? APPLE80211_C_FLAG_2GHZ : APPLE80211_C_FLAG_5GHZ;

#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	if (CHSPEC_IS80(chanspec)) {
		channel->flags |= APPLE80211_C_FLAG_80MHZ;
	} else
#endif
	if (CHSPEC_IS40(chanspec)) {
		channel->flags |= APPLE80211_C_FLAG_40MHZ;
		if (CHSPEC_SB_LOWER(chanspec)) {
			channel->flags |= APPLE80211_C_FLAG_EXT_ABV;
		}
	} else if (CHSPEC_IS20(chanspec))
		channel->flags |= APPLE80211_C_FLAG_20MHZ;
	else if (CHSPEC_BW(chanspec) == WL_CHANSPEC_BW_10)
		channel->flags |= APPLE80211_C_FLAG_10MHZ;
	// determine if we need to add radar flag
#ifndef MACOSX_DHD
	if (wlc_radar_chanspec(pWLC->cmi, chanspec)) {
		channel->flags |= APPLE80211_C_FLAG_DFS;
	}
#endif /* !MACOSX_DHD */
}

int
AirPort_Brcm43xx::applechannel2chanspec(const struct apple80211_channel *cd, chanspec_t *chanspec)
{
	int channel = (int)cd->channel;
	u_int32_t flags = cd->flags;
	*chanspec = 0;

#ifdef WL_CHANSPEC_BW_10
	if (flags & APPLE80211_C_FLAG_10MHZ) {
		*chanspec |= WL_CHANSPEC_BW_10;
	} else
#endif /* WL_CHANSPEC_BW_10 */
	if (flags & APPLE80211_C_FLAG_20MHZ) {
		*chanspec |= WL_CHANSPEC_BW_20;
	} else if (flags & APPLE80211_C_FLAG_40MHZ) {
		*chanspec |= WL_CHANSPEC_BW_40;
#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	} else if (flags & APPLE80211_C_FLAG_80MHZ) {
		*chanspec |= WL_CHANSPEC_BW_80;
	} else if (flags & APPLE80211_C_FLAG_160MHZ) {
		*chanspec |= WL_CHANSPEC_BW_160;
#endif
	} else {
		return EINVAL;
	}

	if (flags & APPLE80211_C_FLAG_2GHZ) {
		*chanspec |= WL_CHANSPEC_BAND_2G;
	} else if (flags & APPLE80211_C_FLAG_5GHZ) {
		*chanspec |= WL_CHANSPEC_BAND_5G;
	} else {
		return EINVAL;
	}
	if (CHSPEC_IS80(*chanspec) || CHSPEC_IS160(*chanspec) || (flags & APPLE80211_C_FLAG_5GHZ))
	{
		*chanspec = wf_channel2chspec(channel, *chanspec & WL_CHANSPEC_BW_MASK);
		if (*chanspec == 0) {
			return EINVAL;
		}
	} else {
		if (CHSPEC_IS40(*chanspec)) {
			if (flags & APPLE80211_C_FLAG_EXT_ABV) {
				*chanspec |= WL_CHANSPEC_CTL_SB_L;
				channel += 2;	/* center channel is above control */
			} else {
				*chanspec |= WL_CHANSPEC_CTL_SB_U;
				channel -= 2;	/* center channel is below control */
			}
		}

		if (channel < 0 || channel > WL_CHANSPEC_CHAN_MASK) {
			return EINVAL;
		}

		*chanspec |= (chanspec_t)channel;

	}
	return 0;
}

#ifndef MACOSX_DHD
/* static */ void
AirPort_Brcm43xx::apple80211_scan_done( void *drv, int status, wlc_bsscfg_t *cfg)
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)drv;
	BCM_REFERENCE(cfg);

	KERNEL_DEBUG_CONSTANT(WiFi_events_APPLE80211_M_SCAN_DONE | DBG_FUNC_NONE, status, 0, 0, 0, 0);

	if (status == WLC_E_STATUS_ERROR)
		return;

	// call scanComplete even in case of a failed scan
	// to allow APPLE80211_M_SCAN_DONE to be signaled and so we do not report
	// our state as scanning from getSCAN_RESULT()

	wlobj->scanComplete(wlobj->pWLC->scan_results);
	wlobj->postMessage(APPLE80211_M_SCAN_DONE);
}
#endif /* !MACOSX_DHD */

void
AirPort_Brcm43xx::scanFreeResults()
{
	scan_result_t *sr = NULL;

	current_scan_result = NULL;

	while (!SLIST_EMPTY(&scan_result_list)) {
		sr = SLIST_FIRST(&scan_result_list);
		SLIST_REMOVE_HEAD(&scan_result_list, entries);

		if (sr->asr.asr_ie_data)
			MFREE(osh, sr->asr.asr_ie_data, sr->asr.asr_ie_len);
		MFREE(osh, sr, sizeof(scan_result_t));
	}
}
#ifndef MACOSX_DHD
void
AirPort_Brcm43xx::scanComplete(wlc_bss_list_t *scan_results)
{
	scan_result_t *sr = NULL;
	scan_result_t *tail_sr = NULL;
	wlc_bss_info_t *bi = NULL;
#ifdef BCMDBG
	char ssidbuf[SSID_FMT_BUF_LEN] = {0};
	char eabuf[ETHER_ADDR_STR_LEN] = {0};
#endif

	for (uint i = 0; i < scan_results->count; i++) {
		bi = scan_results->ptrs[i];

		// <rdar://problem/19966595> Wi-FI CoEx: J44 running Capitan GM C109 unable to scan reliably. (Regression from Cab)
		//if (bi->RSSI == WLC_RSSI_INVALID)
		//	continue;

		if (!scanPhyModeMatch(scan_phy_mode, bi)) {
			WL_IOCTL(("wl%d: scanComplete: pruning for phy_mode match failure: "
			          "%s: ch %3d \"%s\"\n",
			          unit,
			          bcm_ether_ntoa(&bi->BSSID, eabuf),
			          CHSPEC_CHANNEL(bi->chanspec),
			          (wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf)));
			continue;
		}

		// filter based on bssid list passed in from multi scan req
		if ((inMultiScan == TRUE) && (mscan_maclist_filter(&bi->BSSID) == TRUE)) {
			// this scan result is filtered out
			WL_IOCTL(("wl%d: scanComplete: pruning for bssid match failure: "
			          "%s: ch %3d \"%s\"\n",
			          unit,
			          bcm_ether_ntoa(&bi->BSSID, eabuf),
			          CHSPEC_CHANNEL(bi->chanspec),
			          (wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf)));
			continue;
		}

		sr = (scan_result_t*)MALLOCZ(osh, sizeof(scan_result_t));
		if (!sr)
			break;
		memset(sr, 0, sizeof(scan_result_t));

		scanConvertResult(bi, &sr->asr);

		// Link this scan result to the list
		if (tail_sr)
			SLIST_INSERT_AFTER(tail_sr, sr, entries);
		else
			SLIST_INSERT_HEAD(&scan_result_list, sr, entries);

		tail_sr = sr;
	}

	scan_done = 1;
	scan_event_external = 1;
	current_scan_result = SLIST_FIRST(&scan_result_list);
	if (inMultiScan == TRUE) {
		// clear the multiscan state and maclist
		inMultiScan = FALSE;
		mscan_maclist_clear();
	}
#ifdef SCAN_JOIN_TIMEOUT
	pWLC->scan_timeout = 0;
#endif
}
#else
void
AirPort_Brcm43xx::scanComplete(wl_bss_info_t *scan_results, uint count)
{
	scan_result_t *sr;
	scan_result_t *tail_sr = NULL;
#ifdef BCMDBG
	char ssidbuf[SSID_FMT_BUF_LEN];
	char eabuf[ETHER_ADDR_STR_LEN];
#endif

	for (uint i = 0; i < count; i++) {

		sr = (scan_result_t*)MALLOCZ(osh, sizeof(*sr));
		if (!sr) {
			WL_ERROR(("%s: MALLOC(%d) failed\n", __FUNCTION__, (int)sizeof(*sr)));
			break;
		}
		memset(sr, 0, sizeof(*sr));

		// Link this scan result to the list
		if (tail_sr) {
			SLIST_INSERT_AFTER(tail_sr, sr, entries);
		} else {
			SLIST_INSERT_HEAD(&scan_result_list, sr, entries);
		}

		tail_sr = sr;
	}
}
#endif /* MACOSX_DHD */

#ifndef MACOSX_DHD
bool
AirPort_Brcm43xx::scanPhyModeMatch(uint32 phy_mode, wlc_bss_info_t * bi)
{
	bool sup_cck = FALSE, sup_ofdm = FALSE, sup_other = FALSE;
	bool basic_cck = FALSE, basic_ofdm = FALSE, basic_other = FALSE;
#ifdef MACOSX_DHD
	wl_rateset rateset;
#endif /* MACOSX_DHD */
	uint known_modes = (APPLE80211_MODE_11A | APPLE80211_MODE_11B |
	                    APPLE80211_MODE_11G | APPLE80211_MODE_11N);

	// declare a match if no phy_mode specified or was AUTO,
	// or the phy_mode had modes we do not know about.
	if (phy_mode == 0 ||
	    (phy_mode & APPLE80211_MODE_AUTO) ||
	    (phy_mode & ~known_modes))
		return TRUE;

	// otherwise check if the BSS Info fits any of the specifed phy modes
#ifdef MACOSX_DHD
	rateset.count = bi->rateset.count;
	memcpy(rateset.rates, bi->rateset.rates, 16);

	ratesetModulations(&rateset, FALSE, &sup_cck, &sup_ofdm, &sup_other);
	ratesetModulations(&rateset, TRUE, &basic_cck, &basic_ofdm, &basic_other);
#else
	ratesetModulations(&bi->rateset, FALSE, &sup_cck, &sup_ofdm, &sup_other);
	ratesetModulations(&bi->rateset, TRUE, &basic_cck, &basic_ofdm, &basic_other);
#endif /* MACOSX_DHD */
	if (phy_mode & APPLE80211_MODE_11A) {
		// phy_mode 11a is supported if in the 5G band,
		// the supported rateset includes OFDM,
		// and the basic rateset does not include anything other than OFDM
		if (CHSPEC_BAND(bi->chanspec) == WL_CHANSPEC_BAND_5G &&
		    sup_ofdm && !(basic_cck || basic_other))
			return TRUE;
	}
	if (phy_mode & APPLE80211_MODE_11B) {
		// phy_mode 11b is supported if in the 2G band,
		// the supported rateset includes CCK,
		// and the basic rateset does not include anything other than CCK
		if (CHSPEC_BAND(bi->chanspec) == WL_CHANSPEC_BAND_2G &&
		    sup_cck && !(basic_ofdm || basic_other))
			return TRUE;
	}
	if (phy_mode & APPLE80211_MODE_11G) {
		// phy_mode 11g is supported if in the 2G band,
		// the supported rateset includes OFDM,
		// and the basic rateset does not include anything other than CCK or OFDM
		if (CHSPEC_BAND(bi->chanspec) == WL_CHANSPEC_BAND_2G &&
		    sup_ofdm && !basic_other)
			return TRUE;
	}
	if (phy_mode & APPLE80211_MODE_11N) {
		// phy_mode 11n is supported in either band if the BSS is 11n capable,
		// and the basic rateset does not include anything other than CCK or OFDM
		if (CHSPEC_BAND(bi->chanspec) == WL_CHANSPEC_BAND_2G &&
		    (bi->flags & WLC_BSS_HT) && !basic_other)
			return TRUE;
	}

	return FALSE;
}
#endif /* !MACOSX_DHD */

void
#ifdef MACOSX_DHD
AirPort_Brcm43xx::ratesetModulations(wl_rateset_t *rs, bool onlyBasic,
                                     bool *cck, bool *ofdm, bool *other)
#else
AirPort_Brcm43xx::ratesetModulations(wlc_rateset_t *rs, bool onlyBasic,
                                     bool *cck, bool *ofdm, bool *other)
#endif /* MACOSX_DHD */
{
	*cck = FALSE;
	*ofdm = FALSE;
	*other = FALSE;

	for (uint i = 0; i < rs->count; i++) {
		uint8 r = rs->rates[i];

		if (onlyBasic && !(r & DOT11_RATE_BASIC))
			continue;
		r = r & DOT11_RATE_MASK;

		if (r == WLC_RATE_1M ||
		    r == WLC_RATE_2M ||
		    r == WLC_RATE_5M5 ||
		    r == WLC_RATE_11M)
			*cck = TRUE;
		else if (r == WLC_RATE_6M ||
		         r == WLC_RATE_9M ||
		         r == WLC_RATE_12M ||
		         r == WLC_RATE_18M ||
		         r == WLC_RATE_24M ||
		         r == WLC_RATE_36M ||
		         r == WLC_RATE_48M ||
		         r == WLC_RATE_54M)
			*ofdm = TRUE;
		else
			*other = TRUE;

		if (*cck && *ofdm && *other)
			return;
	}
}

#ifdef MACOSX_DHD
void
AirPort_Brcm43xx::scanConvertResult(wl_bss_info_t * bi, struct apple80211_scan_result *asr)
#else
void
AirPort_Brcm43xx::scanConvertResult(wlc_bss_info_t * bi, struct apple80211_scan_result *asr)
#endif /* MACOSX_DHD */
{
	asr->version = APPLE80211_VERSION;

	chanspec2applechannel(bi->chanspec, &asr->asr_channel);
	asr->asr_noise = bi->phy_noise;
	asr->asr_rssi = bi->RSSI;
	asr->asr_beacon_int = bi->beacon_period;
	asr->asr_cap = bi->capability;
	memcpy(asr->asr_bssid, &bi->BSSID, APPLE80211_ADDR_LEN);

	/* brcm reports rates in 500kbps units w/ hi bit set if basic */
	asr->asr_nrates = (u_int8_t)MIN((int)bi->rateset.count, APPLE80211_MAX_RATES);
	for (int j = 0; j < asr->asr_nrates; j++)
		asr->asr_rates[j] = (UInt32)(( bi->rateset.rates[j] & 0x7f ) / 2);

	asr->asr_ssid_len = (u_int8_t)MIN((int)bi->SSID_len, APPLE80211_MAX_SSID_LEN);
	memcpy(asr->asr_ssid, bi->SSID, asr->asr_ssid_len);
#ifndef MACOSX_DHD
#ifdef WLSCANCACHE
	if ((bi->flags & WL_BSS_FLAGS_FROM_CACHE) && bi->timestamp != 0)
		asr->asr_age = OSL_SYSUPTIME() - bi->timestamp;
	else
#endif
#endif /* !MACOSX_DHD */
		asr->asr_age = 0;
#ifndef MACOSX_DHD
	if (bi->bcn_prb) {
		asr->asr_ie_len = bi->bcn_prb_len - DOT11_BCN_PRB_LEN;
		asr->asr_ie_data = MALLOCZ(osh, asr->asr_ie_len);

		if (asr->asr_ie_data == NULL)
			asr->asr_ie_len = 0;
		else
			memcpy(asr->asr_ie_data, (UInt8 *)bi->bcn_prb + DOT11_BCN_PRB_LEN,
			       asr->asr_ie_len);
	} else {
		asr->asr_ie_len = 0;
		asr->asr_ie_data = NULL;
	}
#else
	if (bi->ie_length > 0) {
		asr->asr_ie_len = bi->ie_length;
		asr->asr_ie_data = MALLOCZ(osh, asr->asr_ie_len);

		if (asr->asr_ie_data == NULL)
			asr->asr_ie_len = 0;
		else
			memcpy(asr->asr_ie_data, (UInt8 *)bi + bi->ie_offset,
			       asr->asr_ie_len);
	} else {
		asr->asr_ie_len = 0;
		asr->asr_ie_data = NULL;
	}
#endif /* !MACOSX_DHD */
}

int
AirPort_Brcm43xx::setAP(bool ap)
{
	int bcmerr = 0;
	int isup = 0;
	int default_spect = 0, spect = 0;
#ifdef MACOSX_DHD
	int isap;
#endif /* MACOSX_DHD */
	scb_val_t scb_val = {};
#ifndef MACOSX_DHD
#ifdef IO80211P2P
	if (ap)
		(void) wlIovarSetInt("apsta", 0);
#endif /* IO80211P2P */

	if (ap == AP_ENAB(pub))
		return 0;

	isup = pub->up;
#else
	bcmerr = wlIoctlGetInt( WLC_GET_AP, &isap );

	if (bcmerr) {
		WL_IOCTL(("wl%d: %s: error (%d) from ioctl WLC_GET_AP\n",
			unit, __FUNCTION__, bcmerr));
		return osl_error(bcmerr);
	}
	if (ap == isap) {
		return 0;
	}
	isup = isUp();
#endif /* !MACOSX_DHD */
	if (isup)
		wl_down((struct wl_info *)this);

	setIBSS(FALSE);

	if (ap) {
		(void) wlIovarGetInt("nmode", &cached_nmode_state);
		(void) wlIovarGetInt("vhtmode", &cached_vhtmode_state);
		// Cache PM mode
		(void)wlIoctlGetInt(WLC_GET_PM, &cached_PM_state);
		(void) wlIoctlSetInt(WLC_SET_PM, 0);
	}

	bcmerr = wlIoctlSetInt(WLC_SET_AP, ap);
	if (bcmerr) {
		WL_ERROR(("wl%d: setAP: err setting ioctl WLC_SET_AP to %d, err %d \"%s\"\n",
		          unit, ap, bcmerr, bcmerrorstr(bcmerr)));
		goto done;
	}

	if (!ap) {
		// Restore nmode
		(void) wlIovarSetInt("nmode", cached_nmode_state);
		(void) wlIovarSetInt("vhtmode", cached_vhtmode_state);
		// Restore PM mode
		(void)wlIoctlSetInt(WLC_SET_PM, cached_PM_state);
#ifdef IO80211P2P
		(void) wlIovarSetInt("apsta", 1);
#endif /* IO80211P2P */
	}

	/* Turn-on per-sta RSSI required later */

	bzero( &scb_val.ea, sizeof( struct ether_addr ) );
	(void) wlIoctl(WLC_GET_RSSI, &scb_val, sizeof(scb_val), FALSE, NULL);

	// RDR:4614798 Need to set specific channel for factory diagnostics
	// Reenable spectrum managemet when we leave AP or IBSS mode
	default_spect = SPECT_MNGMT_LOOSE_11H;
	bcmerr = wlIoctlGetInt(WLC_GET_SPECT_MANAGMENT, &spect);
	if (bcmerr) {
		WL_ERROR(("wl%d: setAP: err getting ioctl WLC_GET_SPECT_MANAGMENT, err %d \"%s\"\n",
		          unit, bcmerr, bcmerrorstr(bcmerr)));
		goto done;
	}
	if (!ap && default_spect != spect) {
		bcmerr = wlIoctlSetInt(WLC_SET_SPECT_MANAGMENT, default_spect);
		if (bcmerr) {
			WL_ERROR(("wl%d: setAP: err setting ioctl WLC_SET_SPECT_MANAGMENT to %u, "
			          "err %d \"%s\"\n",
			          unit, default_spect, bcmerr, bcmerrorstr(bcmerr)));
			goto done;
		}
	}

 done:
	if (isup)
		wl_up((struct wl_info *)this);

	return bcmerr;
}
int
AirPort_Brcm43xx::setIBSS(bool ibss)
{
	int bcmerr = 0;
	int isup = 0;
	int default_spect = 0, spect = 0;
#ifdef MACOSX_DHD
	int infra;
#endif /* MACOSX_DHD */
	if ( !ibss && !ibssRunning)
		return 0;

	// Apple change: WPA2-PSK IBSS support
	if( !ibss )
		ibssRunning = false;
#ifndef MACOSX_DHD
	if (ibss == (pWLC->default_bss->bss_type == DOT11_BSSTYPE_INDEPENDENT))
		return 0;

	isup = pub->up;
#else

	bcmerr = wlIoctlGetInt( WLC_GET_INFRA, &infra);
	if (bcmerr) {
		WL_ERROR(("%s: Get infra ioctl failed\n", __FUNCTION__));
		return bcmerr;
	}
	else if (ibss == (infra == 0))
		return 0;
	isup = isUp();
#endif /* !MACOSX_DHD */
	if (isup)
		wl_down((struct wl_info *)this);

	setAP(FALSE);

	bcmerr = wlIoctlSetInt(WLC_SET_INFRA, !ibss);
	if (bcmerr) {
		WL_ERROR(("wl%d: setIBSS: err setting ioctl WLC_SET_INFRA to %d, err %d \"%s\"\n",
		          unit, !ibss, bcmerr, bcmerrorstr(bcmerr)));
		goto done;
	}

	if (PHYTYPE_11N_CAP(phytype)) {
		int shortslot = ibss ? WLC_SHORTSLOT_ON : WLC_SHORTSLOT_AUTO;
		bcmerr = wlIoctlSetInt(WLC_SET_SHORTSLOT_OVERRIDE, shortslot);
		if (bcmerr) {
			WL_ERROR(("wl%d: setIBSS: err setting ioctl WLC_SET_SHORTSLOT_OVERRIDE "
			          "to %s, err %d \"%s\"\n",
			          unit, shortslot == WLC_SHORTSLOT_ON ? "ON" : "AUTO",
			          bcmerr, bcmerrorstr(bcmerr)));
		}
	}

	// RDR:4614798 Need to set specific channel for factory diagnostics
	// Reenable spectrum managemet when we leave AP or IBSS mode
	default_spect = SPECT_MNGMT_LOOSE_11H;
	bcmerr = wlIoctlGetInt(WLC_GET_SPECT_MANAGMENT, &spect);
	if (bcmerr) {
		WL_ERROR(("wl%d: setIBSS: err getting ioctl WLC_GET_SPECT_MANAGMENT, err %d \"%s\"\n",
		          unit, bcmerr, bcmerrorstr(bcmerr)));
		goto done;
	}

	if (!ibss && default_spect != spect) {
		bcmerr = wlIoctlSetInt(WLC_SET_SPECT_MANAGMENT, default_spect);
		if (bcmerr) {
			WL_ERROR(("wl%d: setIBSS: err setting ioctl WLC_SET_SPECT_MANAGMENT to %u, "
			          "err %d \"%s\"\n",
			          unit, default_spect, bcmerr, bcmerrorstr(bcmerr)));
			goto done;
		}
	}

 done:
	if (isup)
		wl_up((struct wl_info *)this);

	return bcmerr;
}
#ifndef MACOSX_DHD
wlc_bsscfg_t *
AirPort_Brcm43xx::wl_find_bsscfg(struct wlc_if * wlcif) const
{
	/* TODO: Save a wlcif pointer (or something from which wlcif pointer
	 * can be found) along with the wl struct and use that to find an appropriate
	 * bsscfg if we need to support multiple bsscfgs/interfaces.
	 */
	return wlc_bsscfg_find_by_wlcif(pWLC, wlcif);
}

// Apple Change: Support virtually, but not physically contiguous packets
UInt32
AirPort_Brcm43xx::getFeatures() const
{
	#if ( VERSION_MAJOR >= 9 )
		return kIONetworkFeatureMultiPages;
	#else
		return 0;
	#endif
}
#endif /* !MACOSX_DHD */
#if VERSION_MAJOR > 9
#ifdef MAX_RSSI
#undef MAX_RSSI
#endif /* MAX_RSSI */
#define MAX_RSSI 100
#ifndef MACOSX_DHD
static void
wl_approbe_complete(wlc_info_t *wlc, uint txstatus, void *ea_arg)
{
	/* no ack */
	BCM_REFERENCE(ea_arg);
	if (!(txstatus & TX_STATUS_ACK_RCV)) {
		WL_PS(("No ack received for Null data frame at %d\n", wlc->pub->now));
		return;
	}

	WL_PS(("Ack received for Null data frame at %d\n", wlc->pub->now));
}
extern "C" {
static int
macos_watchdog_nulldata_cb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *p, void *data)
{
	int err = BCME_OK;
	BCM_REFERENCE(data);
	ASSERT(wlc != NULL);
	ASSERT(cfg != NULL);
	if ((err = wlc_pcb_fn_register(wlc->pcb, wl_approbe_complete,
		(void*)&cfg->BSSID, p))) {
		WL_ERROR(("%s: Could not register callback at %d\n", __FUNCTION__,
			wlc->pub->now));
		return err;
	}

	return BCME_OK;
}
} /* extern "C" */
#endif /* !MACOSX_DHD */

void
AirPort_Brcm43xx::macosWatchdog()
{
	IO80211Interface * interface = getNetworkInterface();
	struct apple80211_link_quality sample;	// Sample that was 'sent' last time.

	int rssi_bucket = -1;
	int tx_rate_bucket = -1;
#ifndef MACOSX_DHD
	wlc_bsscfg_t *bsscfg = wl_find_bsscfg();
	int drv_pwr_throttle_state = 0;

	ASSERT(bsscfg != NULL);
#endif /* !MACOSX_DHD */

	_macosWatchdogTimeStamp = OSL_SYSUPTIME_US64();

	timeNow++;

#ifndef MACOSX_DHD
	LINKDEBUGPRINT( _linkdebug, (LINKDEBUGFLAGS_WATCHDOG|LINKDEBUGFLAGS_TXPACKET|LINKDEBUGFLAGS_RXPACKET),
			"associated[%d] BSSID[%02x:%02x:%02x:%02x:%02x:%02x] rssi[%d] inputPackets[%u] outputPackets[%u]\n",
			bsscfg->associated,
			bsscfg->BSSID.octet[0], bsscfg->BSSID.octet[1], bsscfg->BSSID.octet[2],
			bsscfg->BSSID.octet[3], bsscfg->BSSID.octet[4], bsscfg->BSSID.octet[5],
			(bsscfg->link ? bsscfg->link->rssi : -1),
			(netStats ? netStats->inputPackets : 0),
			(netStats ? netStats->outputPackets : 0) );

	// Check if associated
	if (!bsscfg->associated || interface->linkState() != kIO80211NetworkLinkUp)
		return;
#else
	if (!interface || interface->linkState() != kIO80211NetworkLinkUp) {
		return;
	}
#endif /* !MACOSX_DHD */

	/* Send a null data frame every minute */
#ifndef MACOSX_DHD
	if (((pWLC->pub->now % 60) == 0) && BSSCFG_STA(bsscfg))  {
		if (!wlc_sendnulldata(pWLC, bsscfg, &bsscfg->BSSID, 0, 0,
			PRIO_8021D_BE, macos_watchdog_nulldata_cb, NULL))
			WL_PS(("Sending Null data frame failed at %d\n", pWLC->pub->now));
		else
			WL_PS(("Sending Null data frame succeeded at %d\n", pWLC->pub->now));
	}

	if (revinfo.boardid == BCM943224X21 ||
		((revinfo.boardvendor == VENDOR_APPLE) &&
		 ((revinfo.boardid == BCM943224X21_FCC) ||
		  (revinfo.boardid == BCM943224X21B) ||
		  (revinfo.chipnum == BCM4331_CHIP_ID) ||
		  (revinfo.chipnum == BCM4360_CHIP_ID) ||
		  (revinfo.chipnum == BCM4350_CHIP_ID) ||
		  (revinfo.chipnum == BCM43602_CHIP_ID)))) {
		(void) wlIovarGetInt( "pwrthrottle_state", &drv_pwr_throttle_state );
		if (pwr_throttle_state != drv_pwr_throttle_state) {
			IOLog("New Power Throttle state:%d Old state:%d\n",
			      drv_pwr_throttle_state, pwr_throttle_state);
			pwr_throttle_state = drv_pwr_throttle_state;
		}
	}
#endif /* !MACOSX_DHD */
#ifndef MACOSX_DHD
	// If associated, see if the time has elapsed beyond min_interval
	if ((pWLC->pub->now - last_sample.now) < link_qual_config.min_interval)
		return;
#else
	if ((timeNow - last_sample.now) < link_qual_config.min_interval) {
		return;
	}
#endif /* !MACOSX_DHD */
	bzero(&sample, sizeof(struct apple80211_link_quality));
	// If beyond min_interval, check current RSSI and current txrate
	struct apple80211_rssi_data rssid;

	// A bit aggressive but should be rare to return on failure
#ifndef MACOSX_DHD
	if (!SCAN_IN_PROGRESS(pWLC->scan) && !getRSSI(interface, &rssid))
#else
	if (!getRSSI(interface, &rssid))
#endif /* !MACOSX_DHD */
	{
		int rssi = MAX(-MAX_RSSI, rssid.aggregate_rssi);
		rssi = MIN(0, rssi);
		sample.rssi = rssi;
		// The way this is setup is to make sure that we don't have a divide-by-zero
		// issue. After claming the RSSI between 0 and -100, we find the bucket
		// using the current divisor
		rssi_bucket = CEIL(((MAX_RSSI + sample.rssi) * link_qual_config.rssi_divisor), MAX_RSSI);
		WL_NONE(("RSSI Value: %d rssi_divisor: %d bucket: %d\n",
		          rssi, link_qual_config.rssi_divisor, rssi_bucket));
	} else
		return;

	struct apple80211_rate_data rated;
	if (!getRATE(interface, &rated)) {
		sample.tx_rate = rated.rate[0];
		// The way this is setup is to make sure that we don't have a divide-by-zero issue.
		if (max_txrate == 0)
			return;
		tx_rate_bucket = CEIL((sample.tx_rate * link_qual_config.tx_rate_divisor), max_txrate);
		WL_NONE(("TX rate Value: %d tx_rate_divisor: %d max_txrate:%d bucket: %d\n",
		          sample.tx_rate, link_qual_config.tx_rate_divisor,
		          max_txrate, tx_rate_bucket));
	} else
		return;

	u_int32_t rssi_delta = ABS(sample.rssi - last_sample.rssi);
	u_int32_t tx_rate_delta = ABS((int)sample.tx_rate - last_sample.tx_rate);

	// If either changed beyond divisor, post message
	if (((rssi_bucket != last_sample.rssi_bucket) && (rssi_delta > rssi_hysteresis)) ||
	    ((tx_rate_bucket != last_sample.tx_rate_bucket) && (tx_rate_delta > tx_rate_hysteresis))) {
		WL_PORT(("wl%d: wl_macos_watchdog, pub->up = %d\n"
		         "Posting sample: now:%d Rssi: %d bucket: %d Txrate: %d bucket %d\n"
		         "Previous sample: prev: %d bucket:%d bucket:%d \n",
#ifndef MACOSX_DHD
				unit, pub->up, pub->now,
#else
				unit, 1 , timeNow,
#endif /* !MACOSX_DHD */
		         sample.rssi, rssi_bucket, sample.tx_rate, tx_rate_bucket,
		         last_sample.now,
		         last_sample.rssi_bucket,
		         last_sample.tx_rate_bucket));

		last_sample.rssi_bucket = rssi_bucket;
		last_sample.tx_rate_bucket = tx_rate_bucket;
#ifndef MACOSX_DHD
		last_sample.now = pWLC->pub->now;
#else
		last_sample.now = timeNow;
#endif /* !MACOSX_DHD */
		last_sample.rssi = sample.rssi;
		last_sample.tx_rate = sample.tx_rate;
		postMessage(APPLE80211_M_LINK_QUALITY, &sample,
		            sizeof(struct apple80211_link_quality));
	}
}
#endif
// -----------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//
// Interface between C++ and C for wlc access to the services of AirPort_Brcm43xx
//
////////////////////////////////////////////////////////////////////////////////
// -----------------------------------------------------------------------------

extern "C" {
#ifndef MACOSX_DHD
// -----------------------------------------------------------------------------
void wl_init(struct wl_info *wl)
{
	AirPort_Brcm43xx *pciw = (AirPort_Brcm43xx*)wl;
#if VERSION_MAJOR > 10
	wme_tx_params_t txparams[AC_COUNT] = {};
#endif

	WL_TRACE(("wl%d: wl_init\n", pciw->unit));

	wl_reset(wl);		// reset the interface
	wlc_init(pciw->pWLC);	// initialize the driver and chip


#if VERSION_MAJOR > 10
	/*
	 * Use higher limits for VO class to reduce packet loss, as it's more susceptible
	 * to bursty noise.
	 */
#if !defined(WME_PER_AC_TUNING) || !defined(WME_PER_AC_TX_PARAMS)
	#error "WME_PER_AC_TUNING and WME_PER_AC_TX_PARAMS needs to be enabled"
#endif
	if (pciw->wlIovarOp("wme_tx_params", NULL, 0, &txparams, sizeof(txparams), IOV_GET, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: Cannot change tx params.\n", pciw->unit));
		return;
	}
	txparams[AC_VO].short_retry = RETRY_SHORT_AC_VO;
	(void) pciw->wlIovarOp("wme_tx_params", NULL, 0, &txparams, sizeof(txparams), IOV_SET, NULL);
#endif /* VERSION_MAJOR > 10 */
}

// -----------------------------------------------------------------------------
uint
wl_reset(struct wl_info *wl)
{
	AirPort_Brcm43xx *pciw = (AirPort_Brcm43xx*)wl;

	WL_TRACE(("wl%d: wl_reset\n", pciw->unit));

	wlc_reset(pciw->pWLC);

	return 0;
}
#endif /* !MACOSX_DHD */
/* IO80211_WME_QUEUES & WME are expected to always be on */
#if !defined(IO80211_WME_QUEUES) || !defined(WME)
#error "!IO80211_WME_QUEUES || !WME"
#endif

/* No longer needed, verify its no longer used */
#if defined(MACOS_VIRTIF)
#error "Unexpected usage of deprecated MACOS_VIRTIF"
#endif

/* Some early releases didn't have this */
#if !defined(APPLE80211_WME_NUM_AC)
#define APPLE80211_WME_NUM_AC 4
#endif

void
wl_txflowcontrol( struct wl_info *drv, struct wl_if *wlif, bool on, int wl_prio )
{
	AirPort_Brcm43xx *pciw = (AirPort_Brcm43xx*) drv;
	struct wlc_if *wlcif = NULL;
	txflc_t *txflc = NULL;
	uint macos_bm = 0;
	wlc_bsscfg_t *bsscfg = NULL;
	IOGatedOutputQueue *prio_queues[APPLE80211_WME_NUM_AC] = {};
#ifdef BCMDBG
	const char *prio_name[APPLE80211_WME_NUM_AC] = { "BE", "BK", "VI", "VO" };
#endif
#ifndef MACOSX_DHD
	ASSERT(PRIOFC_ENAB(pciw->pub));
#endif /* !MACOSX_DHD */

	if (!wlif) {
		txflc = &pciw->intf_txflowcontrol;
	} else {
#ifdef IO80211P2P
		wlcif = ((AirPort_Brcm43xxP2PInterface *)wlif)->wlcif;
		txflc = &((AirPort_Brcm43xxP2PInterface *)wlif)->intf_txflowcontrol;
#else
		ASSERT(!"Unknown interface type passed into wl_txflowcontrol");
		return;
#endif
	}

	/* Get the queue pointers. */
	if (wlcif == NULL) {
		IO80211Interface *intf = pciw->getNetworkInterface();
		prio_queues[APPLE80211_WME_AC_BE] = intf->getWMEBestEffortQueue();
		prio_queues[APPLE80211_WME_AC_BK] = intf->getWMEBackgroundQueue();
		prio_queues[APPLE80211_WME_AC_VI] = intf->getWMEVideoQueue();
		prio_queues[APPLE80211_WME_AC_VO] = intf->getWMEVoiceQueue();
	} else {
#ifdef IO80211P2P
		AirPort_Brcm43xxP2PInterface *intf = (AirPort_Brcm43xxP2PInterface *)wlif;
		prio_queues[APPLE80211_WME_AC_BE] = intf->getWMEBestEffortQueue();
		prio_queues[APPLE80211_WME_AC_BK] = intf->getWMEBackgroundQueue();
		prio_queues[APPLE80211_WME_AC_VI] = intf->getWMEVideoQueue();
		prio_queues[APPLE80211_WME_AC_VO] = intf->getWMEVoiceQueue();
#endif
	}

	/* Convert wl based prio's to bitmap of MacOS based bits */
	if (wl_prio == ALLPRIO) {
		mboolset(macos_bm, NBITVAL(APPLE80211_WME_AC_BE)); //0
		mboolset(macos_bm, NBITVAL(APPLE80211_WME_AC_BK)); //1
		mboolset(macos_bm, NBITVAL(APPLE80211_WME_AC_VI)); //2
		mboolset(macos_bm, NBITVAL(APPLE80211_WME_AC_VO)); //3
	} else {
		mboolset(macos_bm, NBITVAL(WlPrio2MacOSPrio(wl_prio)));
	}

	/*
	  Check each bit (prio) and take action. When enabling flowcontrol, just mark
	  the queue for now.  Later, when outputPacket is called with the next packet,
	  if the queue is marked we return 'Stall' to the OS, which triggers
	  OS flow control.
	*/
#ifndef MACOSX_DHD
	bsscfg = wlc_bsscfg_find_by_wlcif(pciw->pWLC, wlcif);
	ASSERT(bsscfg != NULL);
#endif /* !MACOSX_DHD */
	for (int prio = 0; prio < APPLE80211_WME_NUM_AC; prio++){
		if (macos_bm & NBITVAL(prio)) {
#ifndef MACOSX_DHD
			WL_PORT(("wl%d.%d: %s: %s %s\n", pciw->unit, bsscfg->_idx,
				__FUNCTION__, prio_name[prio], on ? "Stop" : "Resume"));
#endif /* !MACOSX_DHD */
			if (on) {
				mboolset(txflc->txFlowControl, NBITVAL(prio));
			} else {
				mboolclr(txflc->txFlowControl, NBITVAL(prio));
				if (prio_queues[prio])
					prio_queues[prio]->service( IOBasicOutputQueue::kServiceAsync );
			}

		}
	}
}

#ifdef AP
// -----------------------------------------------------------------------------
struct wl_wds *
wl_add_wds(struct wl_info *wlh, struct ether_addr *remote, uint unit)
{
	BCM_REFERENCE(wlh);
	BCM_REFERENCE(remote);
	BCM_REFERENCE(unit);
	WL_ERROR(( "wl_add_wds():\n" ));
	return NULL;
}

// -----------------------------------------------------------------------------
void
wl_del_wds(struct wl_info *wlh, struct wl_wds *wdsh)
{
	BCM_REFERENCE(wlh);
	BCM_REFERENCE(wdsh);
	WL_ERROR(( "wl_del_wds():\n" ));
}

#endif
#ifdef MACOSX_DHD
// -----------------------------------------------------------------------------
/* Number of spatial streams, Nss, specified by the ratespec */
uint
wlc_ratespec_nss(ratespec_t rspec)
{
	int Nss;

	if (RSPEC_ISVHT(rspec)) {

		/* VHT ratespec specifies Nss directly */
		Nss = (rspec & WL_RSPEC_VHT_NSS_MASK) >> WL_RSPEC_VHT_NSS_SHIFT;

	} else if (RSPEC_ISHT(rspec)) {
		uint mcs = rspec & RSPEC_RATE_MASK;

		/* HT MCS encodes Nss in MCS index */

		ASSERT(mcs <= 32);

		if (mcs == 32) {
			Nss = 1;
		} else {
			Nss = 1 + (mcs / 8);
		}
	} else {
		/* Legacy rates are all Nss = 1, just add the expansion */
		Nss = 1;
	}

	return Nss;
}
#endif /* MACOSX_DHD */
// -----------------------------------------------------------------------------
// Send a wl common-code driver event packet up the stack
//
void wl_sendup_event(struct wl_info *drv, struct wl_if *wlif, void *m)
{
	wl_sendup(drv, wlif, m, 1);
}
// -----------------------------------------------------------------------------
// Send a wl common-code driver packet up the stack
//
void wl_sendup(struct wl_info *drv, struct wl_if *wlif, void *m, int numpkt)
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx*)drv;
	wlc_pkttag_t *pkttag = NULL;
	ratespec_t rspec = 0;
	struct apple80211_last_rx_pkt_data *pd = NULL;
	struct ether_header *eh = NULL;

	BCM_REFERENCE(numpkt);
	ASSERT(m);

	/* Update last_rx_pkt_data before stripping off pkttag */
	pkttag = WLPKTTAG(m);
	rspec = pkttag->rspec;

	if (!wlif)
		pd = &IFINFOBLKP((AirPort_Brcm43xxInterface*)wlobj->getNetworkInterface())->rxpd;

#ifdef MACOS_VIRTIF
	if (wlif)
		goto skip_pd;
#endif

#ifdef IO80211P2P
	else
		pd = &IFINFOBLKP(((AirPort_Brcm43xxP2PInterface *)wlif))->rxpd;
#endif

	ASSERT(pd);
	pd->num_streams = wlc_ratespec_nss(rspec);
	pd->rate = RSPEC2MBPS(rspec);
	pd->rssi = pkttag->pktinfo.misc.rssi;

	eh = (struct ether_header*) PKTDATA(wlobj->osh, m);
	memcpy( &pd->sa, eh->ether_shost, ETHER_ADDR_LEN);

#ifdef MACOS_VIRTIF
skip_pd:
#endif
	if (m)
		m = PKTTONATIVE(wlobj->osh, m);
	if (m) {
		if (PKTLEN(wlobj->osh, m) < ETHER_HDR_LEN) {
			int i = 0;
			IOLog("%s: Packet Length too short (len %d). Drop Packet\n",
				__FUNCTION__, PKTLEN(wlobj->osh, m));
			if (eh) {
				IOLog("dump: ");
				for (i = 0; i < 20; i++)
					IOLog(" %02x", ((uint8 *)eh)[i]);
				IOLog("\n");
			}
			PKTFREE(wlobj->osh, m, FALSE);
		} else {
				wlobj->inputPacketQueued((mbuf_t)m, wlif, pd->rssi);
		}
	}
}

// -----------------------------------------------------------------------------
int wl_up( struct wl_info *wl )
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;
	int error = 0;
#ifndef MACOSX_DHD
	WL_IOCTL(("wl%d: wl_up(), pub->up = %d\n",
	          wlobj->unit, wlobj->pub->up));

	if (wlobj->pub->up)
		return (0);

#else
	int isup;
	error = wlobj->wlIoctlGetInt( WLC_GET_UP, &isup );

	if (error) {
		WL_IOCTL(("wl%d: %s: error (%d) from ioctl WLC_GET_UP\n",
			wlobj->unit, __FUNCTION__, error));
		return osl_error(error);
	}
	WL_IOCTL(("wl%d: wl_up(), pub->up = %d\n", wlobj->unit, isup));

	if (isup) {
		return (0);
	}
#endif /* !MACOSX_DHD */

	if (wlobj->resetCountryCode) {
		WL_PORT(("wl%d: wl_up:"
		         "performing CountryCodeReset operation\n",
		         wlobj->unit));
#ifdef FACTORY_BUILD
		wlobj->resetCountryCode = (wlobj->setAutoCountry(FALSE) != 0);
#else
#ifdef FACTORY_MERGE
		if (wlobj->disableFactory11d)
		{
			wlobj->resetCountryCode = (wlobj->setAutoCountry(FALSE) != 0);
		}
		else
		{
			wlobj->resetCountryCode = (wlobj->setAutoCountry(TRUE) != 0);
		}

#else
#ifndef MACOSX_DHD
		wlobj->resetCountryCode = (wlobj->setAutoCountry(TRUE) != 0);
#endif /* !MACOSX_DHD */
#endif
#endif

	}
#ifndef MACOSX_DHD
	error = wlc_up(wlobj->pWLC);
#else
	error = wlobj->wlIoctl(WLC_UP, NULL, 0, TRUE, NULL);
#endif /* !MACOSX_DHD */
	return error;
}

// -----------------------------------------------------------------------------
void wl_down( struct wl_info  *wl )
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;
#ifndef MACOSX_DHD
	struct wlc_info * wlc = wlobj->pWLC;

	WL_IOCTL(("wl%d: wl_down, pub->up = %d\n",
	          wlobj->unit, wlobj->pub->up));

	wlc_down(wlc);
#else
	int error;
	error = wlobj->wlIoctl(WLC_DOWN, NULL, 0, TRUE, NULL);
	if (error) {
		WL_IOCTL(("wl%d: %s: error (%d) from ioctl WLC_DOWN\n",
			wlobj->unit, __FUNCTION__, error));
	}
#endif /* !MACOSX_DHD */
}

// -----------------------------------------------------------------------------
// Create a new timer
struct wl_timer *
wl_init_timer(struct wl_info *drv, void (*fn)(void*), void *arg, const char *name)
{
	AirPort_Brcm43xxTimer *t = NULL;
	IOReturn err = BCME_OK;
	AirPort_Brcm43xx *wl = (AirPort_Brcm43xx*) drv;
	IOWorkLoop *workloop = wl->getWorkLoop();

	if (!(t = AirPort_Brcm43xxTimer::initWithParams(wl, fn, arg, name))) {
		WL_ERROR(("wl: wl_init_timer: could not init timer\n"));
		return NULL;
	}

	// Create/Register a new timer event source
	t->iotimer = IOTimerEventSource::timerEventSource((OSObject*)t, wl_timer_callback);
	if (!t->iotimer) {
		WL_ERROR(("wl: wl_init_timer: IOTimerEventSource creation failed\n"));
		t->freeTimerName();
		t->release();
		return NULL;
	}

	err = workloop->addEventSource(t->iotimer);
	if (err != kIOReturnSuccess) {
		WL_ERROR(("wl: wl_init_timer: failed to add event source to work loop, err %d\n",
		          err));
		t->iotimer->release();
		t->freeTimerName();
		t->release();
		return NULL;
	}

	t->next = wl->timers;
	wl->timers = t;

	return (struct wl_timer *)t;
}

// -----------------------------------------------------------------------------
// all done with this timer -- toss it
void
wl_free_timer(struct wl_info *drv, struct wl_timer *timer)
{
	AirPort_Brcm43xxTimer *timers = NULL;
	AirPort_Brcm43xx *wl = NULL;
	IOReturn err = BCME_OK;
	IOWorkLoop *workloop = NULL;
	AirPort_Brcm43xxTimer *t = (AirPort_Brcm43xxTimer *)timer;

	ASSERT(t);
	ASSERT(t->wl == (AirPort_Brcm43xx*)drv);
	ASSERT(t->iotimer != NULL);

	wl = t->wl;
	workloop = wl->getWorkLoop();

	err = workloop->removeEventSource(t->iotimer);
	if (err != kIOReturnSuccess)
		WL_ERROR(("wl%d: wl_free_timer: failed to remove event source from work loop, err %d\n",
		          wl->unit, err));

	t->iotimer->release();

	// remove the wl_timer from the driver's linked list
	if (wl->timers == t)
		wl->timers = t->next;
	else {
		timers = wl->timers;
		while (timers && timers->next != t)
			timers = timers->next;

		ASSERT(timers && (timers->next == t));

		if (timers)
			timers->next = t->next;
	}

	t->freeTimerName();
	t->release();
}

// -----------------------------------------------------------------------------
// start a timer running
void
wl_add_timer(struct wl_info *drv, struct wl_timer *timer, uint ms, int periodic)
{
	AirPort_Brcm43xxTimer *t = (AirPort_Brcm43xxTimer *)timer;

	ASSERT(t);
	ASSERT(t->wl == (AirPort_Brcm43xx*)drv);

	t->addTimer(ms, (bool)periodic);
}

// -----------------------------------------------------------------------------
// cancel an active timer
bool
wl_del_timer(struct wl_info *drv, struct wl_timer *timer)
{
	AirPort_Brcm43xxTimer *t = (AirPort_Brcm43xxTimer *)timer;

	ASSERT(t);
	ASSERT(t->wl == (AirPort_Brcm43xx*)drv);

	return t->delTimer();
}

// -----------------------------------------------------------------------------
static void
wl_timer_callback(OSObject *owner, IOTimerEventSource *sender)
{
	AirPort_Brcm43xxTimer *t = OSDynamicCast ( AirPort_Brcm43xxTimer, owner);
#ifdef MACOSX_DHD
	int isup, err;
#endif /* MACOSX_DHD */

	ASSERT(t);

	t->wl->WL_LOCK();

	ASSERT(t->iotimer == sender);
	// check to make sure device is present
#ifdef MACOSX_DHD
	if (t->wl->isBusUp() && !t->wl->checkCardPresent(FALSE, NULL))
#else
	if (t->wl->pub->up && !t->wl->checkCardPresent(FALSE, NULL))
#endif /* MACOSX_DHD */
	{
#ifdef BCMDBG
		WL_PORT(("%s: timer name %s: fired with no card present!\n", __FUNCTION__, t->name));

		OSL_LOG("timer_no_card",
			"wl_timer_callback: card is not present, restart timer\n");
#endif
		t->restartTimer();
		t->wl->WL_UNLOCK();
		return;
	}
	t->timerCallback();

	t->wl->WL_UNLOCK();
}

// -----------------------------------------------------------------------------
void
wl_dump_ver(struct wl_info *wl, struct bcmstrbuf *b)
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx*)wl;

	bcm_bprintf(b, "wl%d: %s %s version %s", wlobj->unit,
		__DATE__, __TIME__, EPI_VERSION_STR);
#ifdef MACOS_VIRTIF
	bcm_bprintf(b, "(VirtEther)");
#endif
#ifdef IO80211P2P
	bcm_bprintf(b, "(P2P)");
#endif
	bcm_bprintf(b, "\n");
}
#ifndef MACOSX_DHD
// -----------------------------------------------------------------------------
void
wl_event(struct wl_info *wl, char *ifname, wlc_event_t *e)
{
	AirPort_Brcm43xx *drv = (AirPort_Brcm43xx*) wl;

	drv->wlEvent(ifname, e);
}

// -----------------------------------------------------------------------------
void
wl_event_sync(struct wl_info *wl, char *ifname, wlc_event_t *e)
{
	AirPort_Brcm43xx *drv = (AirPort_Brcm43xx*) wl;

#if (defined(WL_EVENTQ) && defined(P2PO) && !defined(P2PO_DISABLED))
	/* duplicate event for local event q */
	wl_eventq_dup_event(drv->wlevtq, e);
#endif /* WL_EVENTQ && P@PO && !P2PO_DISABLED */
	drv->wlEventSync(ifname, e);
}

// -----------------------------------------------------------------------------
bool
wl_alloc_dma_resources(struct wl_info *wl, uint addrwidth)
{
	BCM_REFERENCE(wl);
	BCM_REFERENCE(addrwidth);
	return TRUE;
}

// -----------------------------------------------------------------------------
void
wl_intrson(struct wl_info *wl)
{
	wlc_intrson( ((AirPort_Brcm43xx*)wl)->pWLC );
}

// -----------------------------------------------------------------------------
uint32
wl_intrsoff(struct wl_info *wl)
{
	return wlc_intrsoff( ((AirPort_Brcm43xx*)wl)->pWLC );
}

// -----------------------------------------------------------------------------
void
wl_intrsrestore(struct wl_info *wl, uint32 macintmask)
{
	wlc_intrsrestore( ((AirPort_Brcm43xx*)wl)->pWLC, macintmask );
}
#endif /* !MACOSX_DHD */

// -----------------------------------------------------------------------------
char *
wl_ifname(struct wl_info *wl, struct wl_if* wlif)
{
	AirPort_Brcm43xx *pciw = (AirPort_Brcm43xx *)wl;
	if (!wlif)
		return (char*)pciw->getIFName();
	else
#ifdef MACOS_VIRTIF
		return (char*)pciw->getIFName(wlif->virtIf, wlif->ifname);
#endif
#ifdef IO80211P2P
		return (char*)pciw->getIFName((IONetworkInterface *)wlif,
		                              ((AirPort_Brcm43xxP2PInterface *)wlif)->ifname);
#else
		return (char *)"";
#endif
}
#ifndef MACOSX_DHD
struct wl_if *
wl_add_if(struct wl_info *wl, struct wlc_if* wlcif, uint unit, struct ether_addr *remote)
{
BCM_REFERENCE(remote);
#if defined(IO80211P2P) || defined(MACOS_VIRTIF)
	AirPort_Brcm43xx *wlObj = ((AirPort_Brcm43xx *)wl);
	return wlObj->createVirtIf(wlcif, unit);
#endif /* IO80211P2P */
	return NULL;
}

void
wl_del_if(struct wl_info *wl, struct wl_if *wlif)
{
#ifdef MACOS_VIRTIF
	AirPort_Brcm43xx *wlObj = ((AirPort_Brcm43xx *)wl);

	wlObj->destroyVirtIf(wlif);
#endif

#ifdef IO80211P2P
	AirPort_Brcm43xx *wlObj = ((AirPort_Brcm43xx *)wl);

	wlObj->destroyVirtIf((AirPort_Brcm43xxP2PInterface *)wlif);
#endif /* IO80211P2P */
}

int
wl_osl_pcie_rc(struct wl_info *wl, uint op, int param)
{
	BCM_REFERENCE(param);
#if VERSION_MAJOR > 10
	if (op == 0) {		/* return link capability in configuration space */
		return 2;	/* hardcode to pcie gen2 before getting function from apple*/
	} else if (op == 1) {	/* hot reset */
		AirPort_Brcm43xx *drv = (AirPort_Brcm43xx*) wl;
		if (drv && drv->parent_dev)
			drv->SecondaryBusReset(drv->parent_dev);
		return 0;
	}
#endif
	return 0;
}

#ifdef WL_MONITOR
// -----------------------------------------------------------------------------
// Monitor Mode
void
wl_monitor( struct wl_info *drv, wl_rxsts_t *rxsts, void *mbuf )
{
	AirPort_Brcm43xx * controller = (AirPort_Brcm43xx *)drv;
	//struct mbuf * p = (struct mbuf *)mbuf;
	mbuf_t p = (mbuf_t)mbuf;
	bool fixMbuf = false;

	if(controller->ieee80211Enabled ||
	   controller->avsEnabled ||
	   controller->bsdEnabled ||
	   controller->ppiEnabled) {
		mbuf_adj(p, D11_PHY_HDR_LEN);
		fixMbuf = true;
	}

	if( controller->ieee80211Enabled )
		controller->inputMonitorPacket( p, DLT_IEEE802_11, NULL, 0 );

	if( controller->avsEnabled ) {
		avs_radio_header radio_header;

		bzero( &radio_header, sizeof(avs_radio_header) );

		radio_header.version = htonl( P80211CAPTURE_VERSION );
		radio_header.length = htonl( sizeof( avs_radio_header ) );

		if(rxsts)
		{
			uint channel = wf_chspec_ctlchan(rxsts->chanspec);

			radio_header.channel = htonl(channel);

			// Time packet was received..not very useful
			radio_header.mactime = (UInt64)htonl( rxsts->mactime * 1000 );	// broadcom reports in us, AVS wants ns
			if (rxsts->datarate != 0)
				radio_header.datarate = htonl( ( rxsts->datarate * 500 )/100 );	// broadcom reports in 500kbps units, AVS wants 100 Kbps units
			else {
				ratespec_t rspec = 0;
				uint32 datarate = 0;

				rspec = RSPEC_ENCODE_HT | rxsts->mcs;

				if (rxsts->htflags & WL_RXS_HTF_40)
					rspec |= (PHY_TXC1_BW_40MHZ << RSPEC_BW_SHIFT);

				if (rxsts->htflags & WL_RXS_HTF_SGI)
					rspec |= RSPEC_SHORT_GI;

				datarate = RSPEC2RATE(rspec);
				radio_header.datarate = htonl( datarate/100 );	// broadcom reports in Kbps units, AVS wants 100 Kbps units
			}

			switch( rxsts->encoding )
			{
				case WL_RXS_ENCODING_DSSS_CCK:
					radio_header.encoding = htonl( encoding_type_cck );
				break;

				case WL_RXS_ENCODING_OFDM:
					radio_header.encoding = htonl( encoding_type_ofdm );
				break;

				default:
				break; // nothing

			};

			radio_header.ssi_signal = htonl( rxsts->signal );
			radio_header.ssi_type = htonl(ssi_type_dbm);
			radio_header.ssi_noise = htonl( rxsts->noise );

			radio_header.antenna = htonl( rxsts->antenna );

			switch( rxsts->preamble )
			{
				case WL_RXS_PREAMBLE_SHORT:
					radio_header.preamble = htonl( preamble_short );
				break;

				case WL_RXS_PREAMBLE_LONG:
					radio_header.preamble = htonl( preamble_long );

				/* ??? to add MIMO_MM and MIMO_GF */
				default:
				break; // do nothing

			};

			switch( rxsts->phytype )
			{
				case WL_RXS_PHY_A:
					radio_header.phytype = htonl( phytype_ofdm_dot11_a );	// just use the first type for A/B/C in the enum
				break;

				case WL_RXS_PHY_B:
					radio_header.phytype = htonl( phytype_dsss_dot11_b );
				break;

				case WL_RXS_PHY_G:
					radio_header.phytype = htonl( phytype_ofdm_dot11_g );
				break;

				default:
				break; // do nothing

			}

		}

		controller->inputMonitorPacket( (mbuf_t)p, DLT_IEEE802_11_RADIO_AVS, &radio_header, sizeof( radio_header ) );
	} else if(controller->bsdEnabled) {
		struct dot11_header *mac_header = NULL;
		uint bsd_header_len = 0;
		bsd_header_rx_t bsd_header = {};
			// Header length is complicated due to dynamic presence of signal and noise fields
		mac_header = (struct dot11_header *)mbuf_data( p );
		bsd_header_len = wl_radiotap_rx(mac_header, rxsts, &bsd_header);

		controller->inputMonitorPacket( (mbuf_t)p, DLT_IEEE802_11_RADIO, &bsd_header, bsd_header_len );
	} else if( controller->ppiEnabled ) {
		int channel_frequency = 0;
		u_int32_t channel_flags = 0;
		//		u_int8_t flags;
		//struct dot11_header * mac_header = (struct dot11_header *)mbuf_data( p );
		//u_int16_t fc = ltoh16( mac_header->fc );
		uint ppi_hdr_len = 0;
		ppi_packetheader_t ppi_hdr = {};
		char *hdr_bytes = NULL;

		ppi_hdr_len = PPI_HEADER_LEN;

		ppi_hdr.pph_version = 0;
		ppi_hdr.pph_flags = 0; /* non-aligned */
		ppi_hdr.pph_dlt = OSSwapHostToLittleInt32(DLT_IEEE802_11);

		if (CHSPEC_IS2G(rxsts->chanspec)) {
			channel_flags = PPI_80211_CHAN_2GHZ | PPI_80211_CHAN_DYN;
			channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec),
			                                   WF_CHAN_FACTOR_2_4_G);
		} else {
			channel_flags = PPI_80211_CHAN_5GHZ | PPI_80211_CHAN_OFDM;
			channel_frequency = wf_channel2mhz(wf_chspec_ctlchan(rxsts->chanspec),
			                                   WF_CHAN_FACTOR_5_G);
		}
// 		WL_TMP(("%s %d chanspec 0x%x ctlchan 0x%x freq: %d 0x%x \n", __FUNCTION__, __LINE__,
// 		        rxsts->chanspec, wf_chspec_ctlchan(rxsts->chanspec), channel_frequency, fc));

		ppi_80211_common_t ppi_80211_cmn;

		ppi_80211_cmn.fld_hdr.pfh_type = OSSwapHostToLittleInt16( PPI_80211_COMMON_TYPE);
		ppi_80211_cmn.fld_hdr.pfh_datalen = OSSwapHostToLittleInt16( PPI_80211_COMMON_LEN);
		ppi_80211_cmn.tsft = OSSwapHostToLittleInt64( (uint64)rxsts->mactime );
		ppi_80211_cmn.flags = OSSwapHostToLittleInt16(PPI_80211_COMMON_FCS);
		ppi_80211_cmn.rate = OSSwapHostToLittleInt16(rxsts->datarate);
		ppi_80211_cmn.channel_freq = OSSwapHostToLittleInt16( channel_frequency );
		ppi_80211_cmn.channel_flags = OSSwapHostToLittleInt16( channel_flags );
		ppi_80211_cmn.fhss_hopset = 0;
		ppi_80211_cmn.fhss_pattern = 0;
		if (rxsts->signal != 0)
			ppi_80211_cmn.dbm_antsignal = (int8) rxsts->signal;
		else
			ppi_80211_cmn.dbm_antsignal = -128;
		if (rxsts->noise != 0)
			ppi_80211_cmn.dbm_antnoise = (int8) rxsts->noise;
		else
			ppi_80211_cmn.dbm_antnoise = -128;

		ppi_hdr_len += sizeof(ppi_80211_cmn);
		ppi_hdr_len = ROUNDUP(ppi_hdr_len, 4);

		ppi_hdr.pph_len = OSSwapHostToLittleInt16( ppi_hdr_len );

		hdr_bytes = (char*)MALLOC(controller->osh, ppi_hdr_len);
		if (hdr_bytes) {
			bzero(hdr_bytes, ppi_hdr_len);
			bcopy(&ppi_hdr, hdr_bytes, PPI_HEADER_LEN);
			bcopy(&ppi_80211_cmn, hdr_bytes + PPI_HEADER_LEN, sizeof(ppi_80211_cmn));
			controller->inputMonitorPacket( (mbuf_t)p, DLT_PPI, hdr_bytes, ppi_hdr_len );
			MFREE(controller->osh, hdr_bytes, ppi_hdr_len);
			hdr_bytes = NULL;
		}
	}

	if( fixMbuf )
		mbuf_setdata( p, (int8*)mbuf_data(p) - D11_PHY_HDR_LEN, mbuf_len(p) + D11_PHY_HDR_LEN );
}

#ifdef WLTXMONITOR
void
wl_tx_monitor( struct wl_info *drv, wl_txsts_t *txsts, void *mbuf )
{
	AirPort_Brcm43xx * controller = (AirPort_Brcm43xx *)drv;
	mbuf_t p = (mbuf_t)mbuf;

	if( controller->ieee80211Enabled ||
		controller->avsEnabled ||
		controller->bsdEnabled ||
		controller->ppiEnabled )
		mbuf_adj( p, D11_PHY_HDR_LEN );

	if( controller->ieee80211Enabled ) {
		mbuf_t new_m = controller->coalesce_output((mbuf_t)p, TRUE, FALSE);

		controller->inputMonitorPacket( new_m, DLT_IEEE802_11, NULL, 0 );
		PKTFREE(controller->osh, new_m, TRUE);

	}

	if( controller->avsEnabled )
	{
		avs_radio_header radio_header = {};

		bzero( &radio_header, sizeof(avs_radio_header) );

		radio_header.version = htonl( P80211CAPTURE_VERSION );
		radio_header.length = htonl( sizeof( avs_radio_header ) );

		if(txsts)
		{
			uint channel = wf_chspec_ctlchan(txsts->chanspec);

			radio_header.channel = htonl(channel);

			// Time packet was received..not very useful
			radio_header.mactime = (UInt64)htonl( txsts->mactime * 1000 );	// broadcom reports in us, AVS wants ns

			if (txsts->datarate != 0)
				radio_header.datarate = htonl( ( txsts->datarate * 500 )/100 );	// broadcom reports in 500kbps units, AVS wants 100 Kbps units
			else {
				ratespec_t rspec = 0;
				uint32 datarate = 0;

				rspec = RSPEC_ENCODE_HT | txsts->mcs;

				if (txsts->htflags & WL_RXS_HTF_40)
					rspec |= (PHY_TXC1_BW_40MHZ << RSPEC_BW_SHIFT);

				if (txsts->htflags & WL_RXS_HTF_SGI)
					rspec |= RSPEC_SHORT_GI;

				datarate = RSPEC2RATE(rspec);
				radio_header.datarate = htonl( datarate/100 );	// broadcom reports in Kbps units, AVS wants 100 Kbps units
			}

			switch( txsts->encoding )
			{
				case WL_RXS_ENCODING_DSSS_CCK:
					radio_header.encoding = htonl( encoding_type_cck );
				break;

				case WL_RXS_ENCODING_OFDM:
					radio_header.encoding = htonl( encoding_type_ofdm );
				break;

				default:
				break; // nothing

			};

			//radio_header.antenna = htonl( txsts->antenna );

			switch( txsts->preamble )
			{
				case WL_RXS_PREAMBLE_SHORT:
					radio_header.preamble = htonl( preamble_short );
				break;

				case WL_RXS_PREAMBLE_LONG:
					radio_header.preamble = htonl( preamble_long );

				/* ??? to add MIMO_MM and MIMO_GF */
				default:
				break; // do nothing

			};

		}

		mbuf_t new_m = controller->coalesce_output((mbuf_t)p, TRUE, FALSE);
		controller->inputMonitorPacket( new_m, DLT_IEEE802_11_RADIO_AVS, &radio_header, sizeof( radio_header ) );
		PKTFREE(controller->osh, new_m, TRUE);
	}
	if(txsts &&  controller->bsdEnabled ) {
                 struct dot11_header * mac_header = (struct dot11_header *)mbuf_data( p );
                 uint bsd_header_len = 0;
                 bsd_header_tx_t bsd_header = {};

                 bsd_header_len = wl_radiotap_tx(mac_header, txsts, &bsd_header);
                 mbuf_t new_m = controller->coalesce_output((mbuf_t)p, TRUE, FALSE);
                 controller->inputMonitorPacket( new_m, DLT_IEEE802_11_RADIO, &bsd_header, bsd_header_len );
				 PKTFREE(controller->osh, new_m, TRUE);

	}
}
#endif // WLTXMONITOR

// -----------------------------------------------------------------------------
void
wl_set_monitor( struct wl_info *drv, int val )
{
	BCM_REFERENCE(drv);
	BCM_REFERENCE(val);
}
#endif /* WL_MONITOR */
#endif /* !MACOSX_DHD */

#if (VERSION_MAJOR > 11)
#define MAX_DUMPS 10

#define SOCRAMPATH "/Library/Logs/DiagnosticReports/AirPortBroadcom43XX_%lu.socram"
#define UCMPATH "/Library/Logs/DiagnosticReports/AirPortBroadcom43XX_%lu.ucm"
void wl_dump_mem(char *addr, int len, int type)
{
#if defined(ENABLE_CORECAPTURE)
	osl_wificc_save_snapshot(addr, len, type);
#else
	int error = 0;
	vfs_context_t ctx = 0;
	struct vnode *vp = NULL;
	clock_usec_t usecs = 0;
	clock_sec_t secs = 0;
	char file_path[128] = {};
	static int max_dumps = 0;

	/* Limit number of dumps per boot in case we get in a loop */
	if (max_dumps++ > MAX_DUMPS)
		return;

	clock_get_calendar_microtime(&secs, &usecs);
	if (type == WL_DUMP_MEM_SOCRAM) {
		snprintf(file_path, sizeof(file_path), SOCRAMPATH, secs);
	} else {
		snprintf(file_path, sizeof(file_path), UCMPATH, secs);
	}

	ctx = vfs_context_create(vfs_context_current());

	if((error = vnode_open(file_path, (O_CREAT | FWRITE), (0), 0, &vp, ctx)))
		goto err;

	vn_rdwr(UIO_WRITE, vp,
			addr, len, 0,
			UIO_SYSSPACE, IO_SYNC|IO_NODELOCKED|IO_UNIT,
			vfs_context_ucred(ctx), (int *) 0,
			vfs_context_proc(ctx));

err:
	if (vp) {
		error = vnode_close(vp, FWRITE, ctx);
	}
	vfs_context_rele(ctx);
#endif /* ENABLE_CORECAPTURE */

}
#endif /* (VERSION_MAJOR > 11) */

#ifdef BONJOUR_DATABASE_DEBUG
/*
 * Print the flattened database from Mac platform.
 */
void print_dbase(void *addr, int len) {
	uint8 *p = (uint8 *)addr;
	int i = 0;

	RELEASE_PRINT(("MDNS flattened dbase: %d\n", len));
	for (i = 0; i < len; i++) {
		if (i > 0 && i % 16  == 0) {
			RELEASE_PRINT(("\n"));
		}
		RELEASE_PRINT(("%02X", p[i]));
	}
	RELEASE_PRINT(("\n"));
}
#endif /* BONJOUR_DATABASE_DEBUG */

}

#if defined(BCMDBG) || defined(ENABLE_CORECAPTURE)
#define NAME_ENTRY(x) {x, #x}

const struct name_entry apple80211_ionames[] =
{
	NAME_ENTRY(APPLE80211_IOC_SSID),
	NAME_ENTRY(APPLE80211_IOC_AUTH_TYPE),
	NAME_ENTRY(APPLE80211_IOC_CIPHER_KEY),
	NAME_ENTRY(APPLE80211_IOC_CHANNEL),
	NAME_ENTRY(APPLE80211_IOC_POWERSAVE),
	NAME_ENTRY(APPLE80211_IOC_PROTMODE),
	NAME_ENTRY(APPLE80211_IOC_TXPOWER),
	NAME_ENTRY(APPLE80211_IOC_RATE),
	NAME_ENTRY(APPLE80211_IOC_BSSID),
	NAME_ENTRY(APPLE80211_IOC_SCAN_REQ),
	NAME_ENTRY(APPLE80211_IOC_SCAN_RESULT),
	NAME_ENTRY(APPLE80211_IOC_CARD_CAPABILITIES),
	NAME_ENTRY(APPLE80211_IOC_STATE),
	NAME_ENTRY(APPLE80211_IOC_PHY_MODE),
	NAME_ENTRY(APPLE80211_IOC_OP_MODE),
	NAME_ENTRY(APPLE80211_IOC_RSSI),
	NAME_ENTRY(APPLE80211_IOC_NOISE),
	NAME_ENTRY(APPLE80211_IOC_INT_MIT),
	NAME_ENTRY(APPLE80211_IOC_POWER),
	NAME_ENTRY(APPLE80211_IOC_ASSOCIATE),
	NAME_ENTRY(APPLE80211_IOC_ASSOCIATE_RESULT),
	NAME_ENTRY(APPLE80211_IOC_DISASSOCIATE),
	NAME_ENTRY(APPLE80211_IOC_STATUS_DEV_NAME),
	NAME_ENTRY(APPLE80211_IOC_IBSS_MODE),
	NAME_ENTRY(APPLE80211_IOC_HOST_AP_MODE),
	NAME_ENTRY(APPLE80211_IOC_AP_MODE),
	NAME_ENTRY(APPLE80211_IOC_SUPPORTED_CHANNELS),
	NAME_ENTRY(APPLE80211_IOC_LOCALE),
	NAME_ENTRY(APPLE80211_IOC_DEAUTH),
	NAME_ENTRY(APPLE80211_IOC_COUNTERMEASURES),
	NAME_ENTRY(APPLE80211_IOC_FRAG_THRESHOLD),
	NAME_ENTRY(APPLE80211_IOC_RATE_SET),
	NAME_ENTRY(APPLE80211_IOC_SHORT_SLOT),
	NAME_ENTRY(APPLE80211_IOC_MULTICAST_RATE),
	NAME_ENTRY(APPLE80211_IOC_SHORT_RETRY_LIMIT),
	NAME_ENTRY(APPLE80211_IOC_LONG_RETRY_LIMIT),
	NAME_ENTRY(APPLE80211_IOC_TX_ANTENNA),
	NAME_ENTRY(APPLE80211_IOC_RX_ANTENNA),
	NAME_ENTRY(APPLE80211_IOC_ANTENNA_DIVERSITY),
	NAME_ENTRY(APPLE80211_IOC_ROM),
	NAME_ENTRY(APPLE80211_IOC_DTIM_INT),
	NAME_ENTRY(APPLE80211_IOC_STATION_LIST),
	NAME_ENTRY(APPLE80211_IOC_DRIVER_VERSION),
	NAME_ENTRY(APPLE80211_IOC_HARDWARE_VERSION),
	NAME_ENTRY(APPLE80211_IOC_RAND),
	NAME_ENTRY(APPLE80211_IOC_RSN_IE),
	NAME_ENTRY(APPLE80211_IOC_BACKGROUND_SCAN),
	NAME_ENTRY(APPLE80211_IOC_AP_IE_LIST),
	NAME_ENTRY(APPLE80211_IOC_STATS),
	NAME_ENTRY(APPLE80211_IOC_ASSOCIATION_STATUS),
	NAME_ENTRY(APPLE80211_IOC_COUNTRY_CODE),
	NAME_ENTRY(APPLE80211_IOC_DEBUG_FLAGS),
	NAME_ENTRY(APPLE80211_IOC_LAST_RX_PKT_DATA),
	NAME_ENTRY(APPLE80211_IOC_RADIO_INFO),
	NAME_ENTRY(APPLE80211_IOC_GUARD_INTERVAL),
	NAME_ENTRY(APPLE80211_IOC_MIMO_POWERSAVE),
	NAME_ENTRY(APPLE80211_IOC_MCS),
	NAME_ENTRY(APPLE80211_IOC_RIFS),
	NAME_ENTRY(APPLE80211_IOC_LDPC),
	NAME_ENTRY(APPLE80211_IOC_MSDU),
	NAME_ENTRY(APPLE80211_IOC_MPDU),
	NAME_ENTRY(APPLE80211_IOC_BLOCK_ACK),
	NAME_ENTRY(APPLE80211_IOC_PLS),
	NAME_ENTRY(APPLE80211_IOC_PSMP),
	NAME_ENTRY(APPLE80211_IOC_PHY_SUB_MODE),
	NAME_ENTRY(APPLE80211_IOC_MCS_INDEX_SET),
	NAME_ENTRY(APPLE80211_IOC_CARD_SPECIFIC),
#if VERSION_MAJOR > 9
	NAME_ENTRY(APPLE80211_IOC_BTCOEX_MODE),
#endif
#ifdef APPLEIOCIE
	NAME_ENTRY(APPLE80211_IOC_IE),
#endif
#ifdef IO80211P2P
	NAME_ENTRY(APPLE80211_IOC_P2P_LISTEN),
	NAME_ENTRY(APPLE80211_IOC_P2P_SCAN),
	NAME_ENTRY(APPLE80211_IOC_P2P_GO_CONF),
#if VERSION_MAJOR > 10
	NAME_ENTRY(APPLE80211_IOC_P2P_OPP_PS),
	NAME_ENTRY(APPLE80211_IOC_P2P_CT_WINDOW),
#endif // VERSION_MAJOR > 10
	NAME_ENTRY(APPLE80211_IOC_VIRTUAL_IF_CREATE),
	NAME_ENTRY(APPLE80211_IOC_VIRTUAL_IF_DELETE),
#endif
#if VERSION_MAJOR > 10
#ifdef APPLE80211_IOC_DESENSE
	NAME_ENTRY(APPLE80211_IOC_DESENSE),
#endif /* APPLE80211_IOC_DESENSE */
#ifdef APPLE80211_IOC_DESENSE_LEVEL
	NAME_ENTRY(APPLE80211_IOC_DESENSE_LEVEL),
#endif /* APPLE80211_IOC_DESENSE_LEVEL */
#ifdef APPLE80211_IOC_CHAIN_ACK
	NAME_ENTRY(APPLE80211_IOC_CHAIN_ACK),
#endif /* APPLE80211_IOC_CHAIN_ACK */
#ifdef APPLE80211_IOC_VHT_MCS_INDEX_SET
	NAME_ENTRY(APPLE80211_IOC_VHT_MCS_INDEX_SET),
#endif /* APPLE80211_IOC_VHT_MCS_INDEX_SET */
#ifdef APPLE80211_IOC_MCS_VHT
	NAME_ENTRY(APPLE80211_IOC_MCS_VHT),
#endif /* APPLE80211_IOC_MCS_VHT */
#ifdef APPLE80211_IOC_OFFLOAD_SCANNING
	NAME_ENTRY(APPLE80211_IOC_OFFLOAD_SCANNING),
#endif /* APPLE80211_IOC_OFFLOAD_SCANNING */
#ifdef APPLE80211_IOC_OFFLOAD_ARP_NDP
	NAME_ENTRY(APPLE80211_IOC_OFFLOAD_ARP_NDP),
#endif /* APPLE80211_IOC_OFFLOAD_ARP_NDP */
#ifdef APPLE80211_IOC_OFFLOAD_BEACONS
	NAME_ENTRY(APPLE80211_IOC_OFFLOAD_BEACONS),
#endif /* APPLE80211_IOC_OFFLOAD_BEACONS */
#ifdef OPPORTUNISTIC_ROAM
	NAME_ENTRY(APPLE80211_IOC_ROAM),
#endif /* OPPORTUNISTIC_ROAM */
	NAME_ENTRY(APPLE80211_IOC_SCANCACHE_CLEAR),
#endif // VERSION_MAJOR > 10
#ifdef APPLE80211_IOC_TX_CHAIN_POWER
	NAME_ENTRY(APPLE80211_IOC_TX_CHAIN_POWER),
#endif
#ifdef APPLE80211_IOC_CDD_MODE
	NAME_ENTRY(APPLE80211_IOC_CDD_MODE),
#endif
#ifdef APPLE80211_IOC_THERMAL_THROTTLING
	NAME_ENTRY(APPLE80211_IOC_THERMAL_THROTTLING),
#endif
#ifdef SUPPORT_FEATURE_WOW
	NAME_ENTRY(APPLE80211_IOC_WOW_PARAMETERS),
#endif
#ifdef APPLE80211_IOC_REASSOCIATE
	NAME_ENTRY(APPLE80211_IOC_REASSOCIATE),
#endif
#ifdef APPLE80211_IOC_SUPPORTED_COUNTRY_CODES
	NAME_ENTRY(APPLE80211_IOC_SUPPORTED_COUNTRY_CODES),
#endif
#ifdef APPLE80211_IOC_COUNTRY_TX_POWER_LIMITS
	NAME_ENTRY(APPLE80211_IOC_COUNTRY_TX_POWER_LIMITS),
#endif
#ifdef APPLE80211_IOC_BTCOEX_PROFILES
	NAME_ENTRY(APPLE80211_IOC_BTCOEX_PROFILES),
	NAME_ENTRY(APPLE80211_IOC_BTCOEX_CONFIG),
#endif
#if defined(APPLE80211_IOC_HW_SUPPORTED_CHANNELS)
	NAME_ENTRY(APPLE80211_IOC_HW_SUPPORTED_CHANNELS),
#endif
	{0, NULL}
};

const struct name_entry apple80211_cipher_type_names[] =
{
	NAME_ENTRY(APPLE80211_CIPHER_NONE),
	NAME_ENTRY(APPLE80211_CIPHER_WEP_40),
	NAME_ENTRY(APPLE80211_CIPHER_WEP_104),
	NAME_ENTRY(APPLE80211_CIPHER_TKIP),
	NAME_ENTRY(APPLE80211_CIPHER_AES_OCB),
	NAME_ENTRY(APPLE80211_CIPHER_AES_CCM),
	NAME_ENTRY(APPLE80211_CIPHER_PMK),
	{0, NULL}
};

const struct name_entry apple80211_authtype_lower_names[] =
{
	NAME_ENTRY(APPLE80211_AUTHTYPE_OPEN),
	NAME_ENTRY(APPLE80211_AUTHTYPE_SHARED),
	NAME_ENTRY(APPLE80211_AUTHTYPE_CISCO),
	{0, NULL}
};

const struct name_entry apple80211_authtype_upper_names[] =
{
	NAME_ENTRY(APPLE80211_AUTHTYPE_NONE),
	NAME_ENTRY(APPLE80211_AUTHTYPE_WPA),
	NAME_ENTRY(APPLE80211_AUTHTYPE_WPA_PSK),
	NAME_ENTRY(APPLE80211_AUTHTYPE_WPA2),
	NAME_ENTRY(APPLE80211_AUTHTYPE_WPA2_PSK),
	NAME_ENTRY(APPLE80211_AUTHTYPE_LEAP),
	NAME_ENTRY(APPLE80211_AUTHTYPE_8021X),
	NAME_ENTRY(APPLE80211_AUTHTYPE_WPS),
	{0, NULL}
};

const struct name_entry apple80211_apmode_names[] =
{
	NAME_ENTRY(APPLE80211_AP_MODE_UNKNOWN),
	NAME_ENTRY(APPLE80211_AP_MODE_IBSS),
	NAME_ENTRY(APPLE80211_AP_MODE_INFRA),
	NAME_ENTRY(APPLE80211_AP_MODE_ANY),
	{0, NULL}
};

const struct name_entry apple80211_scan_type_names[] =
{
	NAME_ENTRY(APPLE80211_SCAN_TYPE_NONE),
	NAME_ENTRY(APPLE80211_SCAN_TYPE_ACTIVE),
	NAME_ENTRY(APPLE80211_SCAN_TYPE_PASSIVE),
	NAME_ENTRY(APPLE80211_SCAN_TYPE_FAST),
	NAME_ENTRY(APPLE80211_SCAN_TYPE_BACKGROUND),
	{0, NULL}
};

const struct name_entry apple80211_msgnames[] =
{
	NAME_ENTRY(APPLE80211_M_POWER_CHANGED),
	NAME_ENTRY(APPLE80211_M_SSID_CHANGED),
	NAME_ENTRY(APPLE80211_M_BSSID_CHANGED),
	NAME_ENTRY(APPLE80211_M_LINK_CHANGED),
	NAME_ENTRY(APPLE80211_M_MIC_ERROR_UCAST),
	NAME_ENTRY(APPLE80211_M_MIC_ERROR_MCAST),
	NAME_ENTRY(APPLE80211_M_INT_MIT_CHANGED),
	NAME_ENTRY(APPLE80211_M_MODE_CHANGED),
	NAME_ENTRY(APPLE80211_M_ASSOC_DONE),
	NAME_ENTRY(APPLE80211_M_SCAN_DONE),
	NAME_ENTRY(APPLE80211_M_COUNTRY_CODE_CHANGED),
	{0, NULL}
};

const struct name_entry apple80211_power_state_names[] =
{
	NAME_ENTRY(APPLE80211_POWER_OFF),
	NAME_ENTRY(APPLE80211_POWER_ON),
	NAME_ENTRY(APPLE80211_POWER_TX),
	NAME_ENTRY(APPLE80211_POWER_RX),
	{0, NULL}
};

const struct name_entry apple80211_statenames[] =
{
	NAME_ENTRY(APPLE80211_S_INIT),
	NAME_ENTRY(APPLE80211_S_SCAN),
	NAME_ENTRY(APPLE80211_S_AUTH),
	NAME_ENTRY(APPLE80211_S_ASSOC),
	NAME_ENTRY(APPLE80211_S_RUN),
	{0, NULL}
};

const struct name_entry apple80211_locale_names[] =
{
	NAME_ENTRY(APPLE80211_LOCALE_UNKNOWN),
	NAME_ENTRY(APPLE80211_LOCALE_FCC),
	NAME_ENTRY(APPLE80211_LOCALE_ETSI),
	NAME_ENTRY(APPLE80211_LOCALE_JAPAN),
	NAME_ENTRY(APPLE80211_LOCALE_KOREA),
	NAME_ENTRY(APPLE80211_LOCALE_APAC),
	NAME_ENTRY(APPLE80211_LOCALE_ROW),
#if VERSION_MAJOR > 12
	NAME_ENTRY(APPLE80211_LOCALE_INDONESIA),
#endif
	{0, NULL}
};

const struct name_entry apple80211_unit_names[] =
{
	NAME_ENTRY(APPLE80211_UNIT_DBM),
	NAME_ENTRY(APPLE80211_UNIT_MW),
	NAME_ENTRY(APPLE80211_UNIT_PERCENT),
	{0, NULL}
};

const struct name_entry dot11_status_names[] =
{
	NAME_ENTRY(DOT11_SC_SUCCESS),
	NAME_ENTRY(DOT11_SC_FAILURE),
	NAME_ENTRY(DOT11_SC_CAP_MISMATCH),
	NAME_ENTRY(DOT11_SC_REASSOC_FAIL),
	NAME_ENTRY(DOT11_SC_ASSOC_FAIL),
	NAME_ENTRY(DOT11_SC_AUTH_MISMATCH),
	NAME_ENTRY(DOT11_SC_AUTH_SEQ),
	NAME_ENTRY(DOT11_SC_AUTH_CHALLENGE_FAIL),
	NAME_ENTRY(DOT11_SC_AUTH_TIMEOUT),
	NAME_ENTRY(DOT11_SC_ASSOC_BUSY_FAIL),
	NAME_ENTRY(DOT11_SC_ASSOC_RATE_MISMATCH),
	NAME_ENTRY(DOT11_SC_ASSOC_SHORT_REQUIRED),
	NAME_ENTRY(DOT11_SC_ASSOC_PBCC_REQUIRED),
	NAME_ENTRY(DOT11_SC_ASSOC_AGILITY_REQUIRED),
	NAME_ENTRY(DOT11_SC_ASSOC_SPECTRUM_REQUIRED),
	NAME_ENTRY(DOT11_SC_ASSOC_BAD_POWER_CAP),
	NAME_ENTRY(DOT11_SC_ASSOC_BAD_SUP_CHANNELS),
	NAME_ENTRY(DOT11_SC_ASSOC_SHORTSLOT_REQUIRED),
	NAME_ENTRY(DOT11_SC_ASSOC_DSSSOFDM_REQUIRED),
	NAME_ENTRY(DOT11_SC_ASSOC_HT_REQUIRED),
	NAME_ENTRY(APPLE80211_RESULT_UNKNOWN),
	{0, NULL}
};

const struct name_entry IO80211LinkState_names[] =
{
	NAME_ENTRY(kIO80211NetworkLinkUndefined),
	NAME_ENTRY(kIO80211NetworkLinkDown),
	NAME_ENTRY(kIO80211NetworkLinkUp),
	{0, NULL}
};

const char* lookup_name(const name_entry* tbl, int id)
{
	const name_entry *elt = tbl;
	static char unknown[64] = {};

	for (elt = tbl; elt->name != NULL; elt++) {
		if (id == elt->id)
			return elt->name;
	}

	snprintf(unknown, sizeof(unknown), "ID:%d", id);

	return unknown;

}

const bcm_bit_desc_t apple80211_phymode_flags[] = {
	{APPLE80211_MODE_AUTO, "auto"},
	{APPLE80211_MODE_11A, "11a"},
	{APPLE80211_MODE_11B, "11b"},
	{APPLE80211_MODE_11G, "11g"},
	{APPLE80211_MODE_11N, "11n"},
	{APPLE80211_MODE_TURBO_A, "turbo_A"},
	{APPLE80211_MODE_TURBO_G, "turbo_G"},
	{0, NULL}
};
#endif /* BCMDBG || ENABLE_CORECAPTURE */

#if VERSION_MAJOR > 9
// Link quality indication feature
void
AirPort_Brcm43xx::linkQualHistoryReset()
{
	bzero(&last_sample, sizeof(link_quality_sample_t));
}

void
AirPort_Brcm43xx::linkQualHistoryUpd(UInt32 maxRate)
{
	max_txrate = maxRate;
#if VERSION_MAJOR > 13
	tx_rate_hysteresis = max_txrate * link_qual_config.hysteresis / link_qual_config.tx_rate_divisor / 100;
#endif
}

// Apple change: <rdar://problem/6501147> Support dynamic interface wake flags
struct apple_ie
{
	u_int8_t	apple_id;
	u_int8_t	apple_len;
	u_int8_t	apple_oui[3];
	u_int8_t	apple_version;
	u_int8_t	apple_prod_id;
	u_int16_t	apple_flags;	// BIG ENDIAN!
} __attribute__((packed));

#define APPLE_OUI_BYTES 0x00, 0x03, 0x93
#define APPLE_IE_FLAG_WOW_SUPPORTED 0x100

#ifdef SUPPORT_FEATURE_WOW
bool
AirPort_Brcm43xx::wowCapableAP() const
{
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_bss_info_t *current_bss = NULL;

	bsscfg = wl_find_bsscfg();
	ASSERT(bsscfg != NULL);

	current_bss = wlc_get_current_bss(bsscfg);

	if( !bsscfg->associated || !(current_bss->bcn_prb && current_bss->bcn_prb_len > DOT11_BCN_PRB_LEN) )
		return false;

	u_int32_t	authtype_lower = IFINFOBLKP(myNetworkIF)->authtype_lower;
	u_int32_t	authtype_upper = IFINFOBLKP(myNetworkIF)->authtype_upper;

	if( authtype_lower == APPLE80211_AUTHTYPE_SHARED ||
	    ( authtype_lower == APPLE80211_AUTHTYPE_OPEN &&
		  authtype_upper != APPLE80211_AUTHTYPE_WPA &&
		  authtype_upper != APPLE80211_AUTHTYPE_WPA_PSK &&
		  authtype_upper != APPLE80211_AUTHTYPE_WPA2 &&
		  authtype_upper != APPLE80211_AUTHTYPE_WPA2_PSK ) )
	{
		// If doing WEP or no security, always consider AP
		// as supporting WoW

		// radius server is configured to reauth.

		return true;
	}

	UInt8 appleOUI[] = { APPLE_OUI_BYTES };
	bcm_tlv_t * tlv = (bcm_tlv_t *)((uint8 *)current_bss->bcn_prb + DOT11_BCN_PRB_LEN);
	int tlv_list_len = (int)current_bss->bcn_prb_len - DOT11_BCN_PRB_LEN;

	while( ( tlv = bcm_next_tlv( tlv, &tlv_list_len ) ) != NULL )
	{
		if( tlv->id == DOT11_MNG_PROPR_ID &&
		    ( tlv->len + 2 ) >= (int)sizeof( struct apple_ie ) &&
		    !memcmp( tlv->data, appleOUI, sizeof( appleOUI ) ) )
		{
			struct apple_ie * appleIE = (struct apple_ie *)tlv;
			UInt16 ieFlags = OSSwapBigToHostInt16( appleIE->apple_flags );	// Flags are big endian

			if( ieFlags & APPLE_IE_FLAG_WOW_SUPPORTED )
				return true;
		}
	}

	return false;
}

IOReturn
AirPort_Brcm43xx::getPacketFilters( const OSSymbol * group, UInt32 * filters ) const
{
	if( group == gIOEthernetWakeOnLANFilterGroup &&
	    ( wowl_cap && wowl_cap_platform ) )
	{
		*filters = kIOEthernetWakeOnMagicPacket;
		return kIOReturnSuccess;
	}

	if( group == gIOEthernetDisabledWakeOnLANFilterGroup )
	{
		// If controller has multiple interfaces attached, use IONetworkController::getCommandClient() to determine
		// which interface is querying the controller

		if( wowl_cap && wowl_cap_platform )
		{
			int wowl_os = 0;
			int err = wlIovarGetInt( "wowl_os", &wowl_os );

			if( !err &&
			    wowl_os &&
			    wl_find_bsscfg()->associated &&
			    wl_find_bsscfg()->BSS &&
			    myNetworkIF->wowEnabled() &&
			    /* Only call wowCapableAP when not OFFLD capable */
			    wowCapableAP())
			{
				*filters = 0;
				return kIOReturnSuccess;
			}
		}

		*filters = kIOEthernetWakeOnMagicPacket;
		return kIOReturnSuccess;
	}

	return super::getPacketFilters( group, filters );
}
#endif /* SUPPORT_FEATURE_WOW */

extern "C" {
void
wl_macos_watchdog(void *hdl)
{
	AirPort_Brcm43xx *drv = (AirPort_Brcm43xx*) hdl;

	drv->macosWatchdog();
}
} /* extern "C" */
#endif

#ifndef MACOSX_DHD
#ifdef WL_BSSLISTSORT
bool
AirPort_Brcm43xx::wlSortBsslist(wlc_bss_info_t **bip, uint nbip)
{
	wlc_info_t *wlc = pWLC;
	BCM_REFERENCE(wlc);
	int i = 0, j = 0, k = 0;
	struct apple80211_scan_result **asr_list = NULL;
	struct apple80211_scan_result *asr = NULL, *tmp_asr = NULL;
	int total = (int)nbip;
	wlc_bss_info_t *tmp_bi = NULL;
	bool ret = TRUE;
	bool done = FALSE;
	IO80211Interface * interface = getNetworkInterface();

	if (total <= 1)
		return TRUE;

	WL_PORT(("wl%d: wlSortBsslist, total = %d\n",
	         unit, total));

	asr_list = (struct apple80211_scan_result **)MALLOCZ(osh, (sizeof(struct apple80211_scan_result *) *
	                                                      total));
	if (!asr_list)
		return FALSE;
	else
		memset(asr_list, 0, sizeof(struct apple80211_scan_result *) *
		       total);

	// Convert all to the format that API requires
	for (i = 0; i < (int)total; i++) {
		asr = (struct apple80211_scan_result *)MALLOCZ(osh, sizeof(struct apple80211_scan_result));
		if (!asr) {
			ret = FALSE;
			goto fail;
		}

		memset(asr, 0, sizeof(struct apple80211_scan_result));

		scanConvertResult(bip[i], asr);

		// Store the converted pointer
		asr_list[i] = asr;
	}


	/* sort join_targets by join preference score in increasing order */
	for (k = total; --k >= 0;) {
		done = TRUE;
		for (j = 0; j < k; j++) {
			struct apple80211_ap_cmp_data ap1 = {}, ap2 = {}, *ap_res = NULL;

			ap1.asr = asr_list[j];
			ap2.asr = asr_list[j+1];
			ap_res = interface->apCompare(&ap1, &ap2);
			if (ap_res == &ap1) {
				/* swap_asr */
				tmp_asr = asr_list[j];
				asr_list[j] = asr_list[j+1];
				asr_list[j+1] = tmp_asr;

				/* swap bip */
				tmp_bi = bip[j];
				bip[j] = bip[j+1];
				bip[j+1] = tmp_bi;
				done = FALSE;
			}
		}
		if (done)
			break;
	}

fail:
	if (asr_list) {
		for (i = 0; i < (int)total; i++) {
			struct apple80211_scan_result *asr = asr_list[i];
			if (asr) {
				if (asr->asr_ie_data)
					MFREE(osh, asr->asr_ie_data, asr->asr_ie_len);
				MFREE(osh, asr, sizeof(struct apple80211_scan_result));
				asr_list[i] = NULL;
			}
		}
		MFREE(osh, asr_list, sizeof(struct apple80211_scan_result*) * total);
	}
#ifdef BONJOUR_DATABASE_DEBUG
	print_dbase(fOffloadData, fOffloadLen);
#endif

	BCM_REFERENCE(interface);

	return ret;
}
#endif /* MACOSX_DHD */

extern "C" {
	bool
	wl_sort_bsslist(struct wl_info *wl, wlc_bss_info_t **bip, uint nbip)
	{
		return ((AirPort_Brcm43xx*)wl)->wlSortBsslist(bip, nbip);
	}
}

#endif /* !MACOSX_DHD */

#ifdef APPLE80211_POSTMESSAGE_TLV
static wl_bss_info_t *
find_bssinfo_from_edata(uchar *edata, uint32 len)
{
	bcm_tlv_t *tlv, *tlv_pre;
	brcm_prop_ie_t *prop_ie;
	uint32 tlv_len = len;

	tlv = (bcm_tlv_t *)edata;
	while(tlv && (tlv_len >= (BRCM_PROP_IE_LEN + sizeof(wl_bss_info_t)))) {
		do {
			tlv_pre = tlv;
			tlv = bcm_parse_tlvs(tlv, tlv_len, DOT11_MNG_PROPR_ID);
			if (tlv == NULL) {
				WL_ERROR(("%s(): no DOT11_MNG_PROPR_ID IE.\n", __FUNCTION__));
				break;
			}

			prop_ie = (brcm_prop_ie_t *)tlv;
			if (memcmp(prop_ie->oui, BRCM_PROP_OUI, DOT11_OUI_LEN)) {
				WL_ERROR(("%s(): not BRCM_PROP_OUI.\n", __FUNCTION__));
				break;
			}

			if (prop_ie->type != BRCM_EVT_WL_BSS_INFO) {
				WL_ERROR(("%s(): not BRCM_EVT_WL_BSS_INFO.\n", __FUNCTION__));
				break;
			}
			if (WL_ASSOC_ON()) {
				WL_ASSOC(("%s(): found wl_bss_info.\n", __FUNCTION__));
				prhex("wl_bss_info:", (uint8*)prop_ie + BRCM_PROP_IE_LEN, sizeof(wl_bss_info_t));
			}

			return (wl_bss_info_t *)((uint8*)prop_ie + BRCM_PROP_IE_LEN);
		} while (1);

		tlv_len -= tlv_pre->len;
		tlv = (bcm_tlv_t*)(tlv_pre->data + tlv_pre->len);
	}

	return NULL;
}

void
AirPort_Brcm43xx::bssinfoToScanresult(wl_bss_info_t * bi, struct apple80211_scan_result *asr)
{
	memset(asr, 0, sizeof(*asr));

	asr->version = APPLE80211_VERSION;

	if (bi->chanspec)
		chanspec2applechannel(bi->chanspec, &asr->asr_channel);
	else
		WL_ERROR(("%s(): chanspec 0!\n", __FUNCTION__));
	asr->asr_noise = bi->phy_noise;
	asr->asr_rssi = bi->RSSI;
	asr->asr_beacon_int = bi->beacon_period;
	asr->asr_cap = bi->capability;
	memcpy(asr->asr_bssid, &bi->BSSID, APPLE80211_ADDR_LEN);

	/* brcm reports rates in 500kbps units w/ hi bit set if basic */
	asr->asr_nrates = (u_int8_t)MIN((int)bi->rateset.count, APPLE80211_MAX_RATES);
	for (int j = 0; j < asr->asr_nrates; j++)
		asr->asr_rates[j] = (UInt32)(( bi->rateset.rates[j] & 0x7f ) / 2);

	asr->asr_ssid_len = (u_int8_t)MIN((int)bi->SSID_len, APPLE80211_MAX_SSID_LEN);
	memcpy(asr->asr_ssid, bi->SSID, asr->asr_ssid_len);

	/* wl_bss_info_t has no timestamp, so set asr_age = 0 */
	asr->asr_age = 0;

	/* no prob ies for no scan events */
}

/* APPLE80211_POSTMESSAGE_TYPE_SUPPLICANT_EVENT is the last enum , starting from 0.
   enum apple80211_postMessage_tlv_types {
    APPLE80211_POSTMESSAGE_TYPE_BSS_INFO = 0,           // apple80211_scan_result
    APPLE80211_POSTMESSAGE_TYPE_SSID_CHANGED,           // apple80211_message_ssid_changed
    APPLE80211_POSTMESSAGE_TYPE_DEAUTH_RECEIVED,        // apple80211_message_deauth_received
    APPLE80211_POSTMESSAGE_TYPE_DISSASOC_RECEIVED,      // apple80211_message_dissasoc_received
    APPLE80211_POSTMESSAGE_TYPE_LINK_CHANGED,           // apple80211_messasge_link_changed
    APPLE80211_POSTMESSAGE_TYPE_DECRYPTION_ERROR,       // apple80211_message_decryption_error
    APPLE80211_POSTMESSAGE_TYPE_REASSOC,                // apple80211_message_reassoc
    APPLE80211_POSTMESSAGE_TYPE_AUTH,                   // apple80211_message_auth
    APPLE80211_POSTMESSAGE_TYPE_ASSOC_DONE,             // apple80211_message_assoc_done
    APPLE80211_POSTMESSAGE_TYPE_ROAMED,                 // apple80211_message_roamed
    APPLE80211_POSTMESSAGE_TYPE_PRUNE,                  // apple80211_message_prune
    APPLE80211_POSTMESSAGE_TYPE_SUPPLICANT_EVENT,       // apple80211_message_supplicant_event
};
*/
uint32 postmsg_tlv_len[APPLE80211_POSTMESSAGE_TYPE_SUPPLICANT_EVENT + 1] = {
	sizeof(struct apple80211_scan_result),
	sizeof(struct apple80211_message_ssid_changed),
	sizeof(struct apple80211_message_deauth_received),
	sizeof(struct apple80211_message_dissasoc_received),
	sizeof(struct apple80211_messasge_link_changed),
	sizeof(struct apple80211_message_decryption_error),
	sizeof(struct apple80211_message_reassoc),
	sizeof(struct apple80211_message_auth),
	sizeof(struct apple80211_message_assoc_done),
	sizeof(struct apple80211_message_roamed),
	sizeof(struct apple80211_message_prune),
	sizeof(struct apple80211_message_supplicant_event)
};


void
AirPort_Brcm43xx::wl_compose_postmessage_tlv(wlc_event_t *evt,
	enum apple80211_postMessage_tlv_types tlv_type, uint8 *tlv_msg, void *virtIf)
{
	uint8 *msg = NULL;
	uint32 msg_len = 0;
	uint8 *msg_ptr = NULL;
	uint32 post_msg = 0;
	wl_bss_info_t *bss_info = NULL;
	struct apple80211_postMessage_TLV *tlv_ptr = NULL;

	switch (tlv_type) {
		case APPLE80211_POSTMESSAGE_TYPE_LINK_CHANGED: {
			bss_info = find_bssinfo_from_edata((uchar *)evt->data, evt->event.datalen);
			if (!bss_info) {
				WL_ERROR(("%s(): no bss_info for WLC_E_LINK!\n", __FUNCTION__));
				prhex("event data", (uchar*)evt->data, evt->event.datalen);
			}

			post_msg = APPLE80211_M_LINK_CHANGED;

			break;
		}

		case APPLE80211_POSTMESSAGE_TYPE_SSID_CHANGED: {
			uint32 offset = 0;
			uchar *edata_tlv = (uchar *)evt->data;

			/* evt->data starts with SSID */
			offset += strlen((char*)evt->data) + 1;
			edata_tlv += offset;

			bss_info = find_bssinfo_from_edata((uchar *)edata_tlv, evt->event.datalen - offset);
			if (!bss_info) {
				/* try to find it from back */
				edata_tlv = (uchar*)evt->data + evt->event.datalen - sizeof(wl_bss_info_t) - BRCM_PROP_IE_LEN;
					bss_info = find_bssinfo_from_edata((uchar *)edata_tlv, sizeof(wl_bss_info_t) + BRCM_PROP_IE_LEN);
				if (!bss_info) {
					WL_ERROR(("%s(): no bss_info for WLC_E_SET_SSID from back either!\n", __FUNCTION__));
					prhex("event data", (uchar*)evt->data, evt->event.datalen);
				}
			}

			post_msg = APPLE80211_M_SSID_CHANGED;

			break;
		}

		case APPLE80211_POSTMESSAGE_TYPE_AUTH: {
			bss_info = find_bssinfo_from_edata((uchar *)evt->data, evt->event.datalen);
			if (!bss_info) {
				WL_ERROR(("%s(): no bss_info for WLC_E_DISASSOC!\n", __FUNCTION__));
				prhex("event data", (uchar*)evt->data, evt->event.datalen);
			}

			post_msg = APPLE80211_M_AUTH;

			break;
		}

		case APPLE80211_POSTMESSAGE_TYPE_REASSOC:
		case APPLE80211_POSTMESSAGE_TYPE_ASSOC_DONE: {
			uint8 *tlv_data = (uint8*)evt->data;

			/* TLV starts after DOT11_ASSOC_RESP_FIXED_LEN */
			tlv_data += DOT11_ASSOC_RESP_FIXED_LEN;
			bss_info = find_bssinfo_from_edata(tlv_data, evt->event.datalen - DOT11_ASSOC_RESP_FIXED_LEN);
			if (!bss_info) {
				/* try to find it from back */
				tlv_data = (uchar*)evt->data + evt->event.datalen - sizeof(wl_bss_info_t) - BRCM_PROP_IE_LEN;
				bss_info = find_bssinfo_from_edata((uchar *)tlv_data, sizeof(wl_bss_info_t) + BRCM_PROP_IE_LEN);
				if (!bss_info) {
					WL_ERROR(("%s(): no bss_info for %s from back either!\n", __FUNCTION__,
						(tlv_type == APPLE80211_POSTMESSAGE_TYPE_REASSOC) ? "WLC_E_REASSOC" : "WLC_E_ASSOC"));
					prhex("event data", (uchar*)evt->data, evt->event.datalen);
				}
			}

			post_msg = (tlv_type == APPLE80211_POSTMESSAGE_TYPE_REASSOC) ?
				APPLE80211_M_REASSOC : APPLE80211_M_ASSOC_DONE;

			break;
		}

		case APPLE80211_POSTMESSAGE_TYPE_DEAUTH_RECEIVED: {
			uint8 *tlv_data = (uint8*)evt->data;

			/* TLV starts after 2-byte reason code */
			tlv_data += 2;
			bss_info = find_bssinfo_from_edata(tlv_data, evt->event.datalen - 2);
			if (!bss_info){
				WL_ERROR(("%s(): no bss_info for WLC_E_DEAUTH_IND!\n", __FUNCTION__));
				prhex("event data", (uchar*)evt->data, evt->event.datalen);
			}

			post_msg = APPLE80211_M_DEAUTH_RECEIVED;

			break;
		}

		case APPLE80211_POSTMESSAGE_TYPE_DISSASOC_RECEIVED: {
			uint8 *tlv_data = (uint8*)evt->data;

			/* TLV starts after 2-byte reason code */
			tlv_data += 2;
			bss_info = find_bssinfo_from_edata(tlv_data, evt->event.datalen - 2);
			if (!bss_info) {
				WL_ERROR(("%s(): no bss_info for WLC_E_DISASSOC_IND!\n", __FUNCTION__));
				prhex("event data", (uchar*)evt->data, evt->event.datalen);
			}

			post_msg = APPLE80211_M_DISSASOC_RECEIVED;

			break;

		}

		case APPLE80211_POSTMESSAGE_TYPE_ROAMED: {
			bss_info = find_bssinfo_from_edata((uchar *)evt->data, evt->event.datalen);
			if (!bss_info) {
				WL_ERROR(("%s(): no bss_info for WLC_E_ROAM!\n", __FUNCTION__));
				prhex("event data", (uchar*)evt->data, evt->event.datalen);
			}

			post_msg = APPLE80211_M_ROAMED;

			break;
		}

#if NOT_YET
		/* Need to sync up with iOS */
		case APPLE80211_POSTMESSAGE_TYPE_PRUNE: {
			bss_info = find_bssinfo_from_edata((uchar *)evt->data, evt->event.datalen);
			if (!bss_info) {
				WL_ERROR(("%s(): no bss_info for WLC_E_PRUNE!\n", __FUNCTION__));
				prhex("event data", (uchar*)evt->data, evt->event.datalen);
			}

			post_msg = APPLE80211_M_PRUNE;

			break;
		}
#endif
		case APPLE80211_POSTMESSAGE_TYPE_DECRYPTION_ERROR: {
			bss_info = find_bssinfo_from_edata((uchar *)evt->data, evt->event.datalen);
			if (!bss_info) {
				WL_ERROR(("%s(): no bss_info for WLC_E_ICV_ERROR!\n", __FUNCTION__));
				prhex("event data", (uchar*)evt->data, evt->event.datalen);
			}

			post_msg = APPLE80211_M_DECRYPTION_FAILURE;

			break;
		}

		default:
			break;

	}

	if (bss_info) {
		msg_len += sizeof(struct apple80211_postMessage_TLV) + sizeof(struct apple80211_scan_result);
	}
	msg_len += sizeof(struct apple80211_postMessage_TLV) + postmsg_tlv_len[tlv_type];

	msg = (uint8 *)MALLOCZ(osh, msg_len);
	if (msg == NULL) {
			WL_ERROR(("%s(): malloc for %d  failed.\n", __FUNCTION__, tlv_type));
			return;
	}
	msg_ptr = msg;

	if (bss_info) {
		/* always put struct apple80211_scan_result which contains bss_info first */
		tlv_ptr = (struct apple80211_postMessage_TLV *)msg_ptr;
		tlv_ptr->type = APPLE80211_POSTMESSAGE_TYPE_BSS_INFO;
		tlv_ptr->len = sizeof(struct apple80211_scan_result);

		msg_ptr += sizeof(struct apple80211_postMessage_TLV);
		bssinfoToScanresult(bss_info, (struct apple80211_scan_result *)msg_ptr);

		msg_ptr += tlv_ptr->len;
	}

	/* fill in the tlv for type TLV message */
	tlv_ptr = (struct apple80211_postMessage_TLV *)msg_ptr;
	tlv_ptr->type = tlv_type;
	tlv_ptr->len = postmsg_tlv_len[tlv_type];

	/* fill in the tlv_msg */
	msg_ptr += sizeof(struct apple80211_postMessage_TLV);
	memcpy(msg_ptr, tlv_msg, postmsg_tlv_len[tlv_type]);

	if (virtIf) {
#ifdef IO80211P2P
		AirPort_Brcm43xxP2PInterface *vif = (AirPort_Brcm43xxP2PInterface *)virtIf;
		vif->postMessage(post_msg, msg, msg_len);
#endif
	}
	else
		postMessage(post_msg, msg, msg_len);

	if (WL_ASSOC_ON()) {
		prhex("tlv_msg", msg, msg_len);
	}

	MFREE(osh, msg, msg_len);

	return;

}
#endif /* APPLE80211_POSTMESSAGE_TLV */


#if (VERSION_MAJOR >= 15)
// Log power information helper macro
#define GETLOGPOWER_KEYVALUE( _rc, _key, _kv, _kvlen, _buffer, _offset, _maxsz, _delim )	\
do {                                                                  \
	require( _buffer, exit );                                     \
                                                                      \
	if( (_rc = getIOPMPowerSourceKey( (_key), (_kv), (_kvlen) )) == 0 ) { \
		require( ((int)strnlen(_buffer, (_maxsz)) <= (int)(_maxsz)), exit ); \
                                                                      \
		_offset += strlcat( (_buffer), "'" _key "': ", ((_maxsz) - (_offset)) );        \
		require( (_offset <= (_maxsz)), exit );               \
                                                                      \
		_offset += strlcat( (_buffer), (_kv), ((_maxsz) - (_offset)) );                 \
		require( (_offset <= (_maxsz)), exit );               \
                                                                      \
		_offset += strlcat( (_buffer), _delim, ((_maxsz) - (_offset)) );                \
		require( (_offset <= (_maxsz)), exit );               \
	}                                                             \
} while(0);

// Log power information
int
AirPort_Brcm43xx::logIOPMPowerSourceInformation( char *prefix )
{
	int sz = 1024, offset = 0;
	char keyvalue[256] = {};
	char *buffer = NULL;
	int rc = 0;

	if( !prefix )
	  prefix = (char *)"";

	if( (buffer = (char*)IOMalloc( sz )) == NULL )
		return -ENOMEM;

	memset( buffer, 0, sz );

	// Get all information

	getIOPMSystemSleepType( buffer, sz );
	offset += strnlen(buffer, sz);
	require( (offset <= (sz)), exit );

	offset += strlcat( buffer, ",  ", (sz - offset) );
	require( (offset <= (sz)), exit );

	// get "ExternalConnected", which indicates AC adatper is connected or not
	GETLOGPOWER_KEYVALUE( rc, kIOPMPSExternalConnectedKey, keyvalue, sizeof(keyvalue), buffer, offset, sz, ", " );
	require( (rc != -ETIMEDOUT), exit ); /* bail if waitForMatchingService timeout */

	// get "TimeRemaining", which indicates minutes of time remaining
	GETLOGPOWER_KEYVALUE( rc, kIOPMPSTimeRemainingKey, keyvalue, sizeof(keyvalue), buffer, offset, sz, ", " );
	require( (rc != -ETIMEDOUT), exit ); /* bail if waitForMatchingService timeout */

	// get "AtCriticalLevel", which indicates power is at a critical level
	GETLOGPOWER_KEYVALUE( rc, kIOPMPSAtCriticalLevelKey, keyvalue, sizeof(keyvalue), buffer, offset, sz, ", " );
	require( (rc != -ETIMEDOUT), exit ); /* bail if waitForMatchingService timeout */

	// get "AtWarnLevel", which indicates power is at a critical level
	GETLOGPOWER_KEYVALUE( rc, kIOPMPSAtWarnLevelKey, keyvalue, sizeof(keyvalue), buffer, offset, sz, "  " );
	require( (rc != -ETIMEDOUT), exit ); /* bail if waitForMatchingService timeout */

exit:
	// Display via RELEASELOG
	RELEASELOG( ( "IOPMPowerSource Information: %s,  %s\n", prefix, buffer ) );

	IOFree(buffer, sz);

	return 0;
}

#define kIOServiceAppleSmartBatteryKey	"AppleSmartBattery"
// Some common keys, see <IOKit/pwr_mgt/IOPM.h>
// kIOPMPSExternalConnectedKey - "ExternalConnected"
//

int
AirPort_Brcm43xx::getIOPMPowerSourceKey( const char *key, char *buffer, int len, const char *fmt )
{
	int err = 0;
	IOService *service = NULL;
	OSObject *obj = NULL;
	uint64_t value = 0;
	OSDictionary *matching = NULL;

	require_action( key, exit, err = -EINVAL);
	require_action( (buffer && (len > 0)), exit, err = -EINVAL);

	buffer[0] = 0;

	matching = IOService::serviceMatching( kIOServiceAppleSmartBatteryKey );
	require_action( matching, exit, err = -EINVAL);

	service = waitForMatchingService( matching, (uint64_t)(500*1000*1000ULL));
	require_action( service, exit, err = -ETIMEDOUT);

	obj = OSDynamicCast ( OSObject, service->getProperty( key ) );
	require_action( obj, exit, err = -ENOENT);

	// Parse different objects
	if( (OSDynamicCast( OSString, obj) != NULL) )
	{
		// Is a string
		require_action( OSDynamicCast( OSString, obj )->getCStringNoCopy(), exit, err = 0);
		snprintf(buffer, len, "%s", OSDynamicCast( OSString, obj )->getCStringNoCopy() );
	}
	if( (OSDynamicCast( OSNumber, obj) != NULL) )
	{
		// Is a number
		value = OSDynamicCast( OSNumber, obj )->unsigned64BitValue();
		snprintf(buffer, len, fmt ? fmt : "%llu", value );
	}
	if( (OSDynamicCast( OSBoolean, obj) != NULL) )
	{
		// Is bool
		snprintf(buffer, len, "%s", OSDynamicCast( OSBoolean, obj )->isTrue() ? "Yes" : "No" );
	}

	// If not parsed
	require_action( (buffer[0] != 0), exit, err = -ENOENT);

exit:
	OSSafeReleaseNULL(service);
	OSSafeReleaseNULL(matching);

	return err;
}

int
AirPort_Brcm43xx::getIOPMSystemSleepType( char *buffer, int len )
{
	OSNumber *number = NULL;
	uint32_t sleepType = -1;
	int err = -1;

	// see <IOKit/pwr_mgt/IOPMPrivate.h>
	static const char *mapString[] = {
		"SleepType: Invalid",	   /* kIOPMSleepTypeInvalid      = 0 */
		"SleepType: Aborted Sleep",/* kIOPMSleepTypeAbortedSleep = 1 */
		"SleepType: Normal Sleep", /* kIOPMSleepTypeNormalSleep  = 2 */
		"SleepType: Safe Sleep",   /* kIOPMSleepTypeSafeSleep    = 3 */
		"SleepType: Hibernate",    /* kIOPMSleepTypeHibernate    = 4 */
		"SleepType: Standby",      /* kIOPMSleepTypeStandby      = 5 */
		"SleepType: Power Off",    /* kIOPMSleepTypePowerOff     = 6 */
		"SleepType: Deep Idle",    /* kIOPMSleepTypeDeepIdle     = 7 */
	};

	require_action( IOService::getPMRootDomain(), exit, err = -1 );

	number = OSDynamicCast ( OSNumber, (IOService::getPMRootDomain())->getProperty ( kIOPMSystemSleepTypeKey ) );
	require_action( number, exit, err = -1 );

	sleepType = number->unsigned32BitValue();
	require_action( (sleepType < kIOPMSleepTypeLast), exit, err = sleepType ); /* out of range - can't map, return sleepType */

	// User requested return reason string to be retrieved and returned
	require_action( (buffer && (len > 0)), exit, err = sleepType ); /* not user requested, return sleepType */

	strncpy( buffer, mapString[sleepType], len );
	err = sleepType;

exit:
	return err;
}
#endif /* VERSION_MAJOR >= 15 */


#if defined(ENABLE_CORECAPTURE)

// APPLE APPLE APPLE
// CoreCapture support

#define CCBUFLEN 2048
int
osl_wificc_logDebug(const char *fmt, ...)
{
	static char buf[CCBUFLEN] = {};
	int len = 0;
	va_list ap = {};
	AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)g_brcm_controller;
	IOWorkLoop *workloop = obj ? obj->getWorkLoop() : NULL;

	len = snprintf(buf, sizeof(buf), "[%p][%d][%d] ",
		(void*)OSL_OBFUSCATE_BUF(IOThreadSelf()),
		workloop ? workloop->inGate() : 0,
		workloop ? workloop->onThread() : 0 );

	va_start(ap, fmt);
	vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
	va_end(ap);


	require_action(obj,               exit, len = 0);
	require_action(obj->_ccLogPipe,   exit, len = 0);
	require_action(obj->_ccLogStream, exit, len = 0);

	obj->_ccLogStream->logDebug("%s", buf );

exit:
	return len;
}

int
osl_wificc_logDebugIf(uint64_t flags, const char *fmt, ...)
{
	static char buf[CCBUFLEN] = {};
	int len = 0;
	va_list ap = {};

	va_start(ap, fmt);
	vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
	va_end(ap);

	AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)g_brcm_controller;

	require_action(obj,               exit, len = 0);
	require_action(obj->_ccLogPipe,   exit, len = 0);
	require_action(obj->_ccLogStream, exit, len = 0);

	obj->_ccLogStream->logDebugIf((CCStreamLogFlag)flags, "%s", buf );

exit:
	return len;
}

#if !defined(DRIVER_BASENAME) /* Allow over-ride */
#define DRIVER_BASENAME         "com.apple.driver.AirPort.Brcm4360"
#endif

#define DRIVER_LOGS_NAME	"DriverLogs"
#define DRIVER_LOG_ID		"Brcm4360"
#define DRIVER_LOG_FILE		"AirPortBrcm4360_Logs"
#define SOC_RAM_DUMP_NAME	"StateSnapshots"
#define SOC_RAM_DUMP_FILE	"SoC_RAM.bin"
#define UCM_DUMP_FILE		"UCM.bin"
int
osl_wificc_capture(const char *reason)
{
	int rc = BCME_OK;
	CCTimestamp timestamp = {};
	CCDataSession *session = NULL;

	AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)g_brcm_controller;

	require_action(obj,               exit, rc = 0);
	require_action(obj->_ccLogPipe,   exit, rc = 0);
	require_action(obj->_ccLogStream, exit, rc = 0);

	require_action(obj->_ccDataPipe,   exit, rc = BCME_ERROR);
	require_action(obj->_ccDataStream, exit, rc = BCME_ERROR);

	session = obj->_ccDataStream->openSession(reason);
	require_action(session,            exit, rc = BCME_ERROR);

	// Get timestamp from session
	timestamp = session->timestamp;

	/* If FW Dump is available, then save in same directory */
	if (obj->_FWData) {
		obj->_ccDataStream->saveData(SOC_RAM_DUMP_FILE, obj->_FWData, NULL, NULL, session );
		obj->_FWData->release();
		obj->_FWData = NULL;
	}

	if (obj->_UCMData) {
		obj->_ccDataStream->saveData(UCM_DUMP_FILE, obj->_UCMData, NULL, NULL, session );
		obj->_UCMData->release();
		obj->_UCMData = NULL;
	}

	obj->_ccDataStream->closeSession(session);

	// Save driver logs in same location as that of FW & UCM snapshot
	CCPipe::capturePipesUnder( obj->getProvider(), timestamp, reason);

exit:
	return rc;
}


int
osl_wificc_save_snapshot(const void *buffer, const size_t len, int type )
{
	int rc = BCME_OK;
	OSData ** ppData;
	AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)g_brcm_controller;

	ppData = type == WL_DUMP_MEM_SOCRAM ? &obj->_FWData : &obj->_UCMData;

	if (*ppData) {
		WL_ERROR(("%s: skip repeated %s binary dump\n", __FUNCTION__,
			type == WL_DUMP_MEM_SOCRAM ? "SOCRAM" : "UCM"));
		return BCME_BUSY;
	}

	// Copy buffer into OSData object. _XYZData will be freed in osl_wificc_capture()
	*ppData = OSData::withBytes( buffer, len );
	require_action(*ppData,	exit, rc = BCME_ERROR);

exit:
	return rc;
}

bool
AirPort_Brcm43xx::initializeCoreCapture()
{
	CCStreamLogLevel requestedLevel = kCCLogLevelDebug;
	CCStreamLogFlag requestedFlags = (((uint64)wl_msg_level2 << 32) | wl_msg_level);
	const char *moduleID = "0";

	char owner[128] = {};
	snprintf(owner, sizeof(owner), "%s.%s", DRIVER_BASENAME, moduleID);
	_FWData = NULL;

	// Logging
	{
		// Pipe
		struct CCPipeOptions pipeOptions = {};

		pipeOptions.pipeType = kCCLogPipe;
		pipeOptions.logType = kCCLogTypeCircular;
		pipeOptions.logDataType = kCCLogDataText;
		pipeOptions.logPolicy = kCCLogPolicyCapture;

		pipeOptions.minLogSizeToNotify = 1024*4; // 4KB watermark
		pipeOptions.logSize = (4*1024*1024); // Kernel side RingBuffer Size
		pipeOptions.notifyTimeoutms = 1000; //ms

		strncpy(pipeOptions.fileName, DRIVER_LOG_FILE, kCCPipeMaxFileNameLength);
		strncpy(pipeOptions.logIdentifier, DRIVER_LOG_ID, kCCPipeMaxFileNameLength);

		pipeOptions.fileSize = 2*1024*1024; /*2Mb files*/
		pipeOptions.numberOfFiles = 8; /*16Mb total*/
		pipeOptions.callbacks.target = this;
		pipeOptions.callbacks.captureCallback = AirPort_Brcm43xx::cc_callback_capture;

		_ccLogPipe = OSDynamicCast(CCLogPipe, CCPipe::withOwnerNameCapacity(this, owner, DRIVER_LOGS_NAME, &pipeOptions));
		require(_ccLogPipe, failed);


		// Driver Stream
		struct CCStreamOptions streamOptions = {};

		streamOptions.type = kCCLogStream;
		streamOptions.logSettings.ccLevel = requestedLevel;
		streamOptions.logSettings.ccFlags = requestedFlags;
		streamOptions.logSettings.consoleLevel = kCCLogLevelNone;
		streamOptions.logSettings.consoleFlags = 0;

		streamOptions.callbacks.target = this;
		streamOptions.callbacks.setLogLevel = AirPort_Brcm43xx::cc_callback_setLogLevel;
		streamOptions.callbacks.setLogFlags = AirPort_Brcm43xx::cc_callback_setLogFlags;
		streamOptions.callbacks.setConsoleLogFlags = AirPort_Brcm43xx::cc_callback_setConsoleLogFlags;
		streamOptions.callbacks.setConsoleLogLevel = AirPort_Brcm43xx::cc_callback_setConsoleLogLevel;

		snprintf(streamOptions.logIdentifier,  kCCStreamMaxFileNameLength, DRIVER_LOG_ID);
		_ccLogStream = OSDynamicCast(CCLogStream, CCStream::withPipeAndName(_ccLogPipe, DRIVER_LOGS_NAME, &streamOptions));

		require(_ccLogStream, failed);
	}

	// Create CCDataPipe  -- files
	{
		struct CCPipeOptions pipeOptions = {};

		pipeOptions.pipeType = kCCDataPipe;
		pipeOptions.logDataType = kCCLogDataRaw;
		pipeOptions.logType = kCCLogTypeData;
		pipeOptions.logPolicy = kCCLogPolicyCapture;

		pipeOptions.minLogSizeToNotify = 1024*4; // 4KB watermark
		pipeOptions.logSize = (4*1024*1024); // Kernel side RingBuffer Size
		pipeOptions.notifyTimeoutms = 1000; //ms

		strncpy(pipeOptions.fileName, SOC_RAM_DUMP_NAME, sizeof(pipeOptions.fileName));
		strncpy(pipeOptions.logIdentifier, DRIVER_LOG_ID, kCCPipeMaxFileNameLength);

		pipeOptions.fileSize = 4*1024*1024; /*4Mb files*/
		pipeOptions.numberOfFiles = 8; /*32Mb total*/
		pipeOptions.callbacks.target = this;
		pipeOptions.callbacks.captureCallback = AirPort_Brcm43xx::cc_callback_capture;

		_ccDataPipe = OSDynamicCast(CCDataPipe, CCPipe::withOwnerNameCapacity(this, owner, SOC_RAM_DUMP_NAME, &pipeOptions));
		require(_ccDataPipe, failed);


		// Driver Stream
		struct CCStreamOptions streamOptions = {};

		streamOptions.type = kCCDataStream;
		streamOptions.logSettings.ccLevel = requestedLevel;
		streamOptions.logSettings.ccFlags = requestedFlags;
		streamOptions.logSettings.consoleLevel = kCCLogLevelNone;
		streamOptions.logSettings.consoleFlags = 0;

		streamOptions.callbacks.target = this;
		streamOptions.callbacks.setLogLevel = AirPort_Brcm43xx::cc_callback_setLogLevel;
		streamOptions.callbacks.setLogFlags = AirPort_Brcm43xx::cc_callback_setLogFlags;
		streamOptions.callbacks.setConsoleLogFlags = AirPort_Brcm43xx::cc_callback_setConsoleLogFlags;
		streamOptions.callbacks.setConsoleLogLevel = AirPort_Brcm43xx::cc_callback_setConsoleLogLevel;
		snprintf(streamOptions.logIdentifier,  kCCStreamMaxFileNameLength, DRIVER_LOG_ID);

		// Create CCDataStream -- files
		_ccDataStream = OSDynamicCast(CCDataStream, CCDataStream::withPipeAndName(_ccDataPipe, SOC_RAM_DUMP_NAME, &streamOptions) );
		require(_ccDataStream, failed);
	}

	_ccLogPipe->startPipe();
	_ccDataPipe->startPipe();

	return true;

failed:
	return false;
}


void
AirPort_Brcm43xx::cc_callback_capture(const OSObject *target, const char* reason, const CCTimestamp *timestamp)
{
	// AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)target;
	BCM_REFERENCE(target);
	BCM_REFERENCE(reason);
	BCM_REFERENCE(timestamp);
}

void
AirPort_Brcm43xx::cc_callback_setLogLevel(const OSObject *target, enum CCStreamLogLevel level)
{
	// AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)target;
	BCM_REFERENCE(target);
	BCM_REFERENCE(level);
}

void
AirPort_Brcm43xx::cc_callback_setLogFlags(const OSObject *target, CCStreamLogFlag flags)
{
	// AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)target;
	BCM_REFERENCE(target);
	BCM_REFERENCE(flags);
}

void
AirPort_Brcm43xx::cc_callback_setConsoleLogLevel(const OSObject *target, enum CCStreamLogLevel level)
{
	// AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)target;
	BCM_REFERENCE(target);
	BCM_REFERENCE(level);
}

void
AirPort_Brcm43xx::cc_callback_setConsoleLogFlags(const OSObject *target, CCStreamLogFlag flags)
{
	// AirPort_Brcm43xx *obj = (AirPort_Brcm43xx *)target;
	BCM_REFERENCE(target);
	BCM_REFERENCE(flags);
}

extern "C" {
int wl_log_system_state(void * wl, const char * reason, bool capture)
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;
	int err = BCME_OK;
	static int logging_in_progress = 0;
	char drv_version[128];
	uint i;

	if (logging_in_progress) {
		return BCME_BUSY;
	}

	logging_in_progress = TRUE;

	WL_ERROR(("Log system state... : %s\n", reason));

	struct bcmstrbuf b;
	bcm_binit(&b, drv_version, (uint)sizeof(drv_version));

	wl_dump_ver((struct wl_info *)wl, &b);
	WIFICC_LOGDEBUG(("%s\n", drv_version));

	WIFICC_LOGDEBUG(( "KMOD info(name: '%s' version['%s'] address[%p] size[0x%lx])\n",
		KMOD_INFO_NAME.name, KMOD_INFO_NAME.version,
		OSL_OBFUSCATE_BUF(KMOD_INFO_NAME.address), KMOD_INFO_NAME.size ));

	wl_print_backtrace(NULL, NULL, 0);

	osl_pci_access_display(wlobj->osh, 0);

#define WL_DUMP_BUFFER_SIZE (200*1024)
static const char * min_dump_list[] = {
		"sireg", "hw", " hrt", "bsscfg", "scb",
		"prot", "prot_g", "prot_n", "ap",
		"amsdu", "ratesel", "tbtt", "wowl",
		"scancache", "wowl_wake_pkt", "wowl_scan_pkt",
		"vhtcap", "lq", "phynoise", "mchan",
		"11h", "dfs", "tpc", "11d",
		"txc", "macfltr", "awdl", "stf",
		"wlc", "bssinfo", "stats", "obss",
		"phyaci", "mempool", "swkeys", "siid",
		"pm", "htcap", "chanswitch", "olpc",
		"rssi", "cca_stats", "txpwr_reg",
		"wme", "scan", "p2p", "locale",
		"txs_hist", "dma", "macsuspend", "btcx", "btc",
		"amt", "tsf", "pcieinfo", "mac"
	};

static const char * hw_dump_list[] = {
		"mcnx", "txbf", "ampdu", "ratestuff",
		"phycal", "hwkeys", "ucode_fatal", "bmc",
		"perf_stats", "ppr", "pcieregs"
	};

	for (i = 0; i < ARRAYSIZE(min_dump_list); i++) {
		WIFICC_LOGDEBUG(("\n"));
		WIFICC_LOGDEBUG(("%s:----\n", min_dump_list[i]));
		err = wlc_dump_local(wlobj->pWLC->dumpi, (char *)min_dump_list[i],
			WL_DUMP_BUFFER_SIZE);
	}

	if (wlobj->pWLC->clk) {
		for (i = 0; i < ARRAYSIZE(hw_dump_list); i++) {
			WIFICC_LOGDEBUG(("\n"));
			WIFICC_LOGDEBUG(("%s:----\n", hw_dump_list[i]));
			err = wlc_dump_local(wlobj->pWLC->dumpi, (char *)hw_dump_list[i],
				WL_DUMP_BUFFER_SIZE);
		}
	}

	WL_ASSOC(("\nDump Complete\n"));

	if (capture) {
		if (wl_powercycle_inprogress(wl)) {
			WIFICC_LOGDEBUG(("CoreCapture:%s\n", reason));
			WIFICC_LOGDEBUG(("Skip capture while power cycle in progress\n"));
		}
		else {
			WIFICC_CAPTURE(reason);
			wlobj->postMessage(APPLE80211_M_DUMP_LOGS);
		}
	} else {
		/* Store the reason string to be used to capture after power cycle completes */
		strncpy(wlobj->_ccReason, reason, sizeof(wlobj->_ccReason)-1);
		wlobj->_ccCapturePending = TRUE;
	}

	logging_in_progress = FALSE;

	return err;
}

int
wl_log_system_state_wrapper(const char *exp)
{
	int ret = BCME_EPERM;

	if (!wl_powercycle_inprogress((void*)g_brcm_controller)) {
		ret = wl_log_system_state((void*)g_brcm_controller, exp, TRUE);
	}

	return ret;
}

int
wl_print_backtrace(const char * prefix, void *i_backtrace, int i_backtrace_depth)
{
#define MAX_BACKTRACE_LEN   (MAX_BACKTRACE_DEPTH * 17 + 64)
	int i;
	int64 offset;
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)g_brcm_controller;
	void * backtrace[MAX_BACKTRACE_DEPTH];
	int depth;
	char * buf;
	char * temp_buf;
	int len = 0, rem_len = MAX_BACKTRACE_LEN;
	int ret = BCME_OK;

	if (i_backtrace == NULL) {
		depth = OSBacktrace(&backtrace[0], MAX_BACKTRACE_DEPTH);
	} else {
		memcpy(backtrace, i_backtrace, sizeof(void*) * i_backtrace_depth);
		depth = i_backtrace_depth;
	}

	require_action(wlobj, exit, ret = BCME_BADARG);
	require_action(wlobj->osh, exit, ret = BCME_ERROR);

	buf = (char *) MALLOC(wlobj->osh, MAX_BACKTRACE_LEN);
	require_action(buf, exit, ret = BCME_NOMEM);
	temp_buf = buf;

	for (i=0; i < depth; i++) {
		offset = (uint64)backtrace[i] - (uint64)KMOD_INFO_NAME.address;

		if ((offset < 0) || (offset > (int64)KMOD_INFO_NAME.size)) {
			offset = (uint64)OSL_OBFUSCATE_BUF(backtrace[i]);
		}

		len = snprintf(temp_buf, rem_len, "0x%llx ", offset);
		temp_buf += len;
		rem_len -= len;
		require_action(rem_len > 0, eob, ret = BCME_BUFTOOSHORT);
	}

eob:
	buf[MAX_BACKTRACE_LEN-1] = '\0';
	WIFICC_LOGDEBUG(("backtrace[%s]: %s\n", prefix ? prefix : "", buf));

	MFREE(wlobj->osh, buf, MAX_BACKTRACE_LEN);
exit:
	return ret;
}
}
#endif /* ENABLE_CORECAPTURE */

#if defined(PCIE_AER_EVT)
void AirPort_Brcm43xx::receivePCIEvent(IOPCIEventSource * es, const IOPCIEvent * event )
{
	BCM_REFERENCE(es);

	require(    es, exit);
	require( event, exit);

	WL_ERROR((" PCIE AER Event:%d, data[0]:%d, data[1]:%d, data[2]:%d, data[3]:%d, data[4]:%d\n",
		event->event, event->data[0], event->data[1], event->data[2], event->data[3], event->data[4]));
	if (event->event == kIOPCIEventFatalError) {
		wl_fatal_error((void*)this, WL_REINIT_RC_PCIE_FATAL_ERROR);
	}

exit:
	return;
}
#endif /* PCIE_AER_EVT */

#ifdef NEED_HARD_RESET
static void _powerCycleOffOnThread( thread_call_param_t param0, thread_call_param_t param1 )
{
	AirPort_Brcm43xx * controller = (AirPort_Brcm43xx *)param0;

	OSIncrementAtomic( &controller->_powerCycleOffOnThreadRunning );
	controller->getCommandGate()->runAction( AirPort_Brcm43xx::powerCycleOffOnThreadGated, param1 );
	OSDecrementAtomic( &controller->_powerCycleOffOnThreadRunning );
}

IOReturn
AirPort_Brcm43xx::powerCycleOffOnThreadGated( OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3 )
{
	((AirPort_Brcm43xx *)owner)->WL_LOCK();
	((AirPort_Brcm43xx *)owner)->powerCycleOffOnThread( arg0, arg1, arg2, arg3 );
	((AirPort_Brcm43xx *)owner)->WL_UNLOCK();

	return kIOReturnSuccess;
}

void
AirPort_Brcm43xx::powerCycleOffOnThread( void * arg0, void * arg1, void * arg2, void * arg3 )
{
	struct apple80211_power_data pd = { APPLE80211_VERSION, 0, {0} };
	SInt32 err = 0;
	uint64_t ms = (uint64_t)arg0;
	// keep the last user requested power state
	unsigned long lastPowerState = _lastUserRequestedPowerState; ;

#define PWR_CHANGE_RETRY 10
#define PWR_CHANGE_DELAY 200
	uint32 retry = PWR_CHANGE_RETRY;
	BCM_REFERENCE(arg1);
	BCM_REFERENCE(arg2);
	BCM_REFERENCE(arg3);

	IO80211WorkLoop * workLoop = (IO80211WorkLoop *)getWorkLoop();

	// keep the last user requested power state
	lastPowerState = _lastUserRequestedPowerState;

	pd.num_radios = num_radios;


	WL_PCIE(("%s - Start Powercycle...\n", __FUNCTION__));

	// power off
	for (uint i = 0; i < num_radios; i++)
		pd.power_state[i] = APPLE80211_POWER_OFF;

	struct apple80211_driver_availability msg;
	msg.version = 2;
	msg.flags = 0;
	msg.available = false;
	msg.reason = 0;

#if MAJOR_VERSION > 13
	msg.reason = kIO80211ReturnWatchdog;
	/* TODO: Use correct reason code once OS headers have them available */
	msg.sub_reason = kIO80211ReturnWDTriggered;
#endif
	postMessage(APPLE80211_M_DRIVER_AVAILABLE, &msg, sizeof(msg));

	// Apple Change: As a side effect of the fix for <rdar://problem/6394678>, this request
	//				 was not being honored. Adding the _powerOffThreadRequest flag fixes that.
	_powerOffThreadRequest = true;

	if (myNetworkIF)
	{
#if VERSION_MAJOR > 13
		struct apple80211_chip_reset reset = { .state = kChipResetInProgress };
		myNetworkIF->postMessage(APPLE80211_M_RESET_INTERFACE, &reset, sizeof(reset));

	#ifdef IO80211P2P
		struct virt_intf_entry *elt;
		SLIST_FOREACH(elt, &virt_intf_list, entries) {
			elt->obj->postMessage(APPLE80211_M_RESET_INTERFACE, &reset, sizeof(reset));
		}
	#endif /* IO80211P2P */

#endif /* VERSION_MAJOR > 13 */

#if VERSION_MAJOR >= 15
		uint32 reinit_reason = wlc_need_reinit(pWLC);
		uint32_t watchdogReason = reinit_reason < ARRAYSIZE(BcmAppleRetCodeMap) ?
			BcmAppleRetCodeMap[reinit_reason] : kAppleBCMWLANCoreReturnUnknown;
		struct apple80211_driver_availability driverAvailMsg = { 2, 0x0, false, kIO80211ReturnWatchdog, watchdogReason, 0, 0};
#elif VERSION_MAJOR > 13
		// Earlier versions do not have return codes defined
		struct apple80211_driver_availability driverAvailMsg = { 2, 0x0, false, 0, 0 };
#else
		// Earlier versions do not have the 5th parameter
		struct apple80211_driver_availability driverAvailMsg = { 2, 0x0, false, 0 };
#endif
		postMessage(APPLE80211_M_DRIVER_AVAILABLE, &driverAvailMsg, sizeof(driverAvailMsg));
	}

	WL_PCIE(("AirPort_Brcm43xx::powerCycleOffOnThread: calling setPOWER(OFF)\n"));
	while (((err = setPOWER( myNetworkIF, &pd)) == EBUSY) && retry--) {
		WL_ERROR(("setPOWER(OFF) Failed. Retry:%d\n",retry));
		WL_UNLOCK();
		workLoop->openGate();
		IOSleep(PWR_CHANGE_DELAY);
		workLoop->closeGate();
		WL_LOCK();
	}

	_powerOffThreadRequest = false;
	WL_PCIE(("AirPort_Brcm43xx::powerCycleOffOnThread: setPOWER(OFF) returned %d.\n", (int)err));

	/* This is best effort case, but not deterministic */
	WL_PCIE(("Sleep : %llu ms, CurrPowerState:%lu\n", ms, _powerState));
	WL_UNLOCK();
	workLoop->openGate();
	IOSleep(ms);
	workLoop->closeGate();
	WL_LOCK();

	if (_powerState == PS_INDEX_ON) {
		WL_ERROR(("PowerState did not change to OFF : %lu", _powerState));
	}

	// We turn on the power only when user asked to
	if ( lastPowerState == PS_INDEX_ON ) {
		// power on
		for (uint i = 0; i < num_radios; i++)
			pd.power_state[i] = APPLE80211_POWER_ON;

		WL_PCIE(("AirPort_Brcm43xx::powerCycleOffOnThread: calling setPOWER(ON)\n"));
		retry = PWR_CHANGE_RETRY;
		while (((err = setPOWER( myNetworkIF, &pd)) == EBUSY) && retry--) {
			WL_ERROR(("setPOWER(ON) Failed. Retry:%d\n",retry));
			WL_UNLOCK();
			workLoop->openGate();
			IOSleep(PWR_CHANGE_DELAY);
			workLoop->closeGate();
			WL_LOCK();
		}

		WL_PCIE(("Sleep : %d ms\n", WL_POWERCYCLE_ON_DURATION));
		WL_UNLOCK();
		workLoop->openGate();
		IOSleep( WL_POWERCYCLE_ON_DURATION );
		workLoop->closeGate();
		WL_LOCK();

		WL_PORT(( "AirPort_Brcm43xx::powerCycleOffOnThread: setPOWER(ON) returned %d.\n", (int)err));
		if (_powerState != PS_INDEX_ON) {
			WL_ERROR(("PowerState did not change to ON : %lu\n", _powerState));
		}

		WL_PCIE(("AirPort_Brcm43xx::powerCycleOffOnThread: setPOWER(ON) returned %d.\n", (int)err));

		if (myNetworkIF)
		{
#if VERSION_MAJOR >= 15
			struct apple80211_driver_availability driverAvailMsg = { 2, 0x0, true, kIO80211ReturnInitializing, 0, 0, 0 };
#elif VERSION_MAJOR > 13
			// Earlier versions do not have return codes defined
			struct apple80211_driver_availability driverAvailMsg = { 2, 0x0, false, 0, 0 };
#else
			// Earlier versions do not have a 5th parameter
			struct apple80211_driver_availability driverAvailMsg = { 2, 0x0, true, 0 };
#endif
			kprintf(" watchdog succeeded - sending driver available event\n");
			postMessage(APPLE80211_M_DRIVER_AVAILABLE, &driverAvailMsg, sizeof(driverAvailMsg));

#if VERSION_MAJOR > 13
			struct apple80211_chip_reset reset = { .state = kChipResetComplete };
			myNetworkIF->postMessage(APPLE80211_M_RESET_INTERFACE, &reset, sizeof(reset));

#ifdef IO80211P2P
			struct virt_intf_entry *elt;
			SLIST_FOREACH(elt, &virt_intf_list, entries) {
				elt->obj->postMessage(APPLE80211_M_RESET_INTERFACE, &reset, sizeof(reset));
			}
#endif /* IO80211P2P */

#endif /* VERSION_MAJOR > 13 */

		}
	} else {
		WL_ERROR(("lastPowerState:%lu Leave module powered off\n", lastPowerState));
	}

	_powerCycleInProgress = FALSE;

#ifdef ENABLE_CORECAPTURE
	if (_ccCapturePending) {
		WL_ERROR(("Power Cycle Completed\n"));
                WIFICC_CAPTURE(_ccReason);
		postMessage(APPLE80211_M_DUMP_LOGS);
		_ccCapturePending = FALSE;
        }
#endif /* ENABLE_CORECAPTURE */
}

/*
 * This function does a hard reset(powercycle) on the chip.
 * Sequence
 * Stop timer processing
 * Stop IOCTL Processing
 * Disable interrupts
 * setPOWER(OFF) -> wl_down()
 * changePowerStateToPriv()
 * setPOWER(ON) -> wl_up()
 * Restore interrupts
 * Restore IOCTL/timer processing
 */
int
AirPort_Brcm43xx::wl_fault_powercycle(void * wl)
{
	int err = BCME_OK;
	bool ret;

	BCM_REFERENCE(wl);

	if(_pwrCycleOffOnThreadCall && !_powerCycleOffOnThreadRunning) {
		ret = thread_call_enter1( _pwrCycleOffOnThreadCall, (void*)WL_POWERCYCLE_OFF_DURATION);
		WL_PCIE(("%s: thread_call_enter1 - Power Thread %s\n", __FUNCTION__,
			ret ? "already scheduled" : "scheduled"));
	}
	else {
		if (_pwrCycleOffOnThreadCall == NULL) {
			err = BCME_NOTREADY;
			WL_PCIE(("Power Thread not initialized\n"));
		} else if (_powerCycleOffOnThreadRunning) {
			WL_PCIE(("%s Power thread already running\n",
				__FUNCTION__));
		}
	}

	return err;
}

extern "C" {

int
wl_powercycle(void * wl)
{
        AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;

	WL_ERROR(("Fatal Error - Initiate powercycle \n"));
	return wlobj->wl_fault_powercycle(wl);
}

bool wl_powercycle_inprogress(void * wl)
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;

	return wlobj->_powerCycleInProgress;
}

}

#endif /* NEED_HARD_RESET */

#ifndef MACOSX_DHD
int wl_fatal_error(void * wl, int rc)
{
	AirPort_Brcm43xx *wlobj = (AirPort_Brcm43xx *)wl;
	char reason[64] = {0};
	bool hard_reset = TRUE;

	/* Test command for big hammer validation */
	if (rc == WL_REINIT_RC_USER_FORCED) {
		return FALSE;
	}

	/* Force panic on backplane timeout if boot-arg is set */
	if ((rc == WL_REINIT_RC_AXI_BUS_ERROR) && (wlobj->_panicOnBPT)) {
		osl_panic("BackPlane timeout");
	}

#ifdef BCM_BACKPLANE_TIMEOUT
	if (rc != WL_REINIT_RC_AXI_BUS_ERROR) {
		snprintf(reason, sizeof(reason)-1, "reinit:%d-%s", rc, wl_get_reinit_rc_name(rc));
	}
	else {
		const si_axi_error_info_t * err_info = si_get_axi_errlog_info(wlobj->pub->sih);
		if (err_info && err_info->count) {
			const si_axi_error_t * axi_error = &err_info->axi_error[0];
			const char * rsn_str;
			switch(axi_error->error) {
				case AXI_WRAP_STS_TIMEOUT:
					rsn_str = "AXITimeout";
					break;

				case AXI_WRAP_STS_SLAVE_ERR:
					rsn_str = "AXISlaveErr";
					break;

				case AXI_WRAP_STS_DECODE_ERR:
					rsn_str = "AXIDecodeErr";
					break;

				case AXI_WRAP_STS_PCI_RD_ERR:
					rsn_str = "PCICfgRdFail";
					break;

				case AXI_WRAP_STS_WRAP_RD_ERR:
					rsn_str = "AXIWrapRdFail";
					break;

				case AXI_WRAP_STS_SET_CORE_FAIL:
					rsn_str = "SetCoreFail";
					break;
				default:
					rsn_str = "UnKnown";
					break;
			}
			if (axi_error->error == AXI_WRAP_STS_PCI_RD_ERR) {
				snprintf(reason, sizeof(reason)-1, "reinit:%d:-%s-%s:0x%x",
					rc, rsn_str, "CfgReg", axi_error->errlog_lo);
			} else if (axi_error->error == AXI_WRAP_STS_SET_CORE_FAIL) {
				snprintf(reason, sizeof(reason)-1, "reinit:%d:-%s-Core:0x%x,Idx:%d",
					rc, rsn_str, axi_error->coreid, axi_error->errlog_lo);
			} else {
				snprintf(reason, sizeof(reason)-1, "reinit:%d:-%s-%s:0x%x-By:0x%.8x",
					rc, rsn_str,
					(axi_error->errlog_flags  & AXI_ERRLOG_FLAGS_WRITE_REQ) ? "WriteReg" : "ReadReg",
					axi_error->errlog_lo, axi_error->errlog_id);
			}
		} else {
			snprintf(reason, sizeof(reason)-1, "reinit:%d-%s", rc, wl_get_reinit_rc_name(rc));
		}
	}

	reason[sizeof(reason)-1] = '\0';
#else
	snprintf(reason, sizeof(reason)-1, "reinit:%d-%s", rc, wl_get_reinit_rc_name(rc));
#endif /* BCM_BACKPLANE_TIMEOUT */

#ifdef NEED_HARD_RESET
	if (!wlobj->_powerCycleInProgress) {
		if (hard_reset) {
			wlobj->_powerCycleInProgress = TRUE;
		}

		if (DEVICEREMOVED(wlobj->pWLC)) {
			/* Update the clock status, so the dump functions wil not access core regs */
			wlobj->pWLC->clk = false;
			si_set_device_removed(wlobj->pWLC->pub->sih, TRUE);
		}

		wl_log_system_state(wl, reason, FALSE);

		if (wlc_halt_hw(wlobj->pWLC) == BCME_NODEVICE) {
			/* We are in process of unloading, no need to reset */
			wlobj->_powerCycleInProgress = FALSE;
			goto skip_reset;
		}

		if (hard_reset) {
#ifdef ENABLE_CORECAPTURE
			/* Dump UCM memory and SOCRAM if not already captured */
			if (!wlobj->_UCMData) {
				wlc_dump_mem(wlobj->pWLC, WL_DUMP_MEM_UCM);
			}

#ifdef WLOFFLD
			if (!wlobj->_FWData) {
				wlc_dump_mem(wlobj->pWLC, WL_DUMP_MEM_SOCRAM);
			}
#endif /* WLOFFLD */
#endif /* ENABLE_CORECAPTURE */
			if (wl_powercycle(wl) != BCME_OK) {
				/* Failed to start the power cycle thread,
				 * let WLC_FATAL_ERROR try for core reset
				 */
				hard_reset = FALSE;
			}
		}
	}
#else
	wl_log_system_state(wl, reason, TRUE);
	hard_reset = FALSE;
#endif /* NEED_HARD_RESET */

skip_reset:
	return hard_reset;
}

#ifdef APPLE80211_IOC_LINK_CHANGED_EVENT_DATA
static apple80211LinkChangeReason
wl_map_bcmlrc_to_apple80211lrc(uint brcmlrc)
{
	apple80211LinkChangeReason a_lrc[WLC_E_LINK_BSSCFG_DIS + 1] =
		{APPLE80211_LINK_CHANGE_REASON_INTERNAL_ERROR, 	/* 0 */
		APPLE80211_LINK_CHANGE_REASON_BEACONLOST, 		/* WLC_E_LINK_BCN_LOSS		1 */
		APPLE80211_LINK_CHANGE_REASON_DEAUTH, 			/* WLC_E_LINK_DISASSOC		2 */
		APPLE80211_LINK_CHANGE_REASON_INTERNAL_ERROR,	/* WLC_E_LINK_ASSOC_REC	3 */
		APPLE80211_LINK_CHANGE_REASON_INTERNAL_ERROR 	/* WLC_E_LINK_BSSCFG_DIS 	4 */
		};

	if (brcmlrc > WLC_E_LINK_BSSCFG_DIS)
		return APPLE80211_LINK_CHANGE_REASON_INTERNAL_ERROR;

	return a_lrc[brcmlrc];
}

SInt32
AirPort_Brcm43xx::getLINK_CHANGED_EVENT_DATA(OSObject * interface, struct apple80211_link_changed_event_data * ed)
{
		BCM_REFERENCE(interface);

		ASSERT(ed);
		if (!ed)
			return EINVAL;

		WL_PORT(("%s():\n", __FUNCTION__));

		struct wlc_if * wlcif = wlLookupWLCIf( interface );

		if (wlcif) {
#ifdef IO80211P2P
			IO80211P2PInterface * p2pIf = OSDynamicCast( IO80211P2PInterface, interface );

			if( p2pIf && p2pIf->getInterfaceRole() != APPLE80211_VIRT_IF_ROLE_P2P_CLIENT ) {
				return ENXIO;
			}
#endif /* IO80211P2P  */
		}

		int err = EPERM;

#ifdef MAC_OS_X_VERSION_10_11
		/* get rssi: in dbm */
		struct apple80211_rssi_data rssi;

		err = getRSSI(interface, &rssi);

		if (err) {
			WL_ERROR(("%s(): get rssi failed: err = %d\n", __FUNCTION__, err));
			return ENXIO;
		}
		ed->rssi = rssi.aggregate_rssi;

		/* get snr: in db */
		int val;
		err = wlIovarOp("snr", NULL, 0, &val, sizeof(val), FALSE, interface);
		if (err) {
			WL_ERROR(("%s(): get snr failed: err = %d\n", __FUNCTION__, err));
			return ENXIO;
		}
		ed->snr = val;

		/* get nf: in dbm  */
		err = wlIoctl(WLC_GET_PHY_NOISE, &val, sizeof( val ), FALSE, NULL );
		if (err) {
			WL_ERROR(("%s(): get nf failed: err = %d\n", __FUNCTION__, err));
			return ENXIO;
		}
		ed->nf = val;

		/* get cca:  in % from 0 meaning free, and 100 meaning congested */
		cca_congest_channel_req_t cca_req;
		cca_congest_channel_req_t *cca_result = NULL;
		int cca_result_size = sizeof(cca_congest_channel_req_t) + (MAX_CCA_SECS - 1) * sizeof(cca_congest_t);
		int i;
		uint32 total_dur = 0, total_busy = 0;

		cca_result = (cca_congest_channel_req_t *)MALLOCZ(osh, cca_result_size);
		if (cca_result == NULL) {
			WL_ERROR(("%s(): failed to malloc %d bytes for cca_stats.\n", __FUNCTION__, cca_result_size));
			return ENOMEM;
		}
		if (wlcif) {
			cca_req.chanspec = wlcif->u.bsscfg->current_bss->chanspec;
		}
		else {
			cca_req.chanspec = pWLC->cfg->current_bss->chanspec;
		}
		cca_req.num_secs = MAX_CCA_SECS;
		err = wlIovarOp("cca_get_stats", &cca_req, sizeof(cca_req), cca_result, cca_result_size,
			FALSE, interface);
		if (err) {
			WL_ERROR(("%s(): get cca_stats failed: err = %d\n", __FUNCTION__, err));
			MFREE(osh, cca_result, cca_result_size);
			return ENXIO;
		}
		for (i = 0; i < cca_result->num_secs; i++) {
			total_dur += cca_result->secs[i].duration;
			/* Sanity filter the cca stats results */
			if (cca_result->secs[i].duration >= cca_result->secs[i].congest_ibss)
				total_busy += cca_result->secs[i].congest_ibss;
			if (cca_result->secs[i].duration >= cca_result->secs[i].congest_obss)
				total_busy += cca_result->secs[i].congest_obss;
			if (cca_result->secs[i].duration >= cca_result->secs[i].interference)
				total_busy += cca_result->secs[i].interference;
		}
		if (total_dur) {
			if (total_busy > total_dur) {
				WL_ERROR(("%s(): bad cca stats: num_secs %d, total_busy %d, total_dur %d\n",
					__FUNCTION__, cca_result->num_secs, total_busy, total_dur));
				ed->cca = 100;
			}
			else
				ed->cca = total_busy * 100 / total_dur;
		}
		else {
			ed->cca = 0;
			WL_ERROR(("%s: cca is not available.\n", __FUNCTION__));
		}
		MFREE(osh, cca_result, cca_result_size);

		/* link_changed_reason: */
		intf_info_blk_t *blk = IFINFOBLKP(interface);
		if (blk) {
			ed->isLinkDown = blk->isLinkDown;
			if (blk->isLinkDown) {
				ed->link_changed_reason.link_down_reason.reason =
					wl_map_bcmlrc_to_apple80211lrc(blk->link_changed_reason.link_down_reason.reason);
				ed->link_changed_reason.link_down_reason.isDisconnectInVoluntary = 0;
			}
		}

		WL_ASSOC(("%s(): apple80211_link_changed_event_data:\n", __FUNCTION__));
		WL_ASSOC(("\tisLinkDown: %s\n", ed->isLinkDown ? "TRUE" : "FALSE"));
		WL_ASSOC(("\trssi: %ddbm, snr: %ddb, nf: %ddbm, cca: %d%c\n",
			ed->rssi, ed->snr, ed->nf, ed->cca, '%'));
		if (ed->isLinkDown) {
			WL_ASSOC(("\tlink_down_reason: voluntary %d, reason:%d\n",
				ed->link_changed_reason.link_down_reason.isDisconnectInVoluntary,
				ed->link_changed_reason.link_down_reason.reason));
		}
#endif /* MAC_OS_X_VERSION_10_11 */
		return err;
}
#endif /* APPLE80211_IOC_LINK_CHANGED_EVENT_DATA */
#endif /* !MACOSX_DHD */

#ifdef APPLE80211_IOC_SUPPORTED_COUNTRY_CODES
SInt32
AirPort_Brcm43xx::getSUPPORTED_COUNTRY_CODES(OSObject *interface,
	struct apple80211_supported_country_codes_data *sccd)
{
	int err = 0;
	size_t cc_len = MIN(WLC_CNTRY_BUF_SZ, APPLE80211_MAX_CC_LEN);
	uint cc_idx;
	BCM_REFERENCE(interface);
	sccd->num_country_codes = 0;
	wl_country_list_t *cl = (wl_country_list_t *)MALLOCZ(osh, WLC_IOCTL_MAXLEN);
	struct apple80211_country_channel_data *ccd =
		(struct apple80211_country_channel_data *)
		MALLOC(osh, sizeof(struct apple80211_country_channel_data));
	struct apple80211_country_tx_power_limits_data *ctpld =
		(struct apple80211_country_tx_power_limits_data *)
		MALLOC(osh, sizeof(struct apple80211_country_tx_power_limits_data));
	if ((cl == NULL) || (ccd == NULL) || (ctpld == NULL)) {
		err = ENOMEM;
		goto cleanup;
	}
	cl->buflen = WLC_IOCTL_MAXLEN;
	cl->count = 0;
	err = wlIoctl(WLC_GET_COUNTRY_LIST, cl, (size_t)cl->buflen, FALSE, NULL);
	if (err != BCME_OK) {
		err = osl_error(err);
		goto cleanup;
	}
	sccd->version = APPLE80211_VERSION;
	/* From retrieved country list admit only genuine CCs and default (minimum) CC */
	for (cc_idx = 0; cc_idx < cl->count; ++cc_idx) {
		const char *country_code = cl->country_abbrev + cc_idx * WLC_CNTRY_BUF_SZ;
		if ((country_code - (const char *)cl) > WLC_IOCTL_MAXLEN) {
			WL_ERROR(("'count' in 'wl_country_list_t' exceeds buffer length\n"));
			err = EINVAL;
			goto cleanup;
		}
		if (sccd->num_country_codes == APPLE80211_MAX_COUNTRY_CODES) {
			WL_ERROR(("'clm_available_countries' returned too many countries\n"));
			err = EINVAL;
			goto cleanup;
		}
		/* Is CC genuine? */
		if ((country_code[0] < 'A') || (country_code[0] > 'Z') ||
			(country_code[1] < 'A') || (country_code[1] > 'Z') ||
			(country_code[0] == 'X') ||
			((country_code[0] == 'Q') && (country_code[1] >= 'M')) ||
			!bcmp (country_code, "AA", 2) || !bcmp (country_code, "ZZ", 2) ||
			!bcmp (country_code, "EU", 2))
		{
			/* CC is nongenuine. Is it default (minimum) CC? */
			/* This information may be obtained from getCOUNTRY_TX_POWER_LIMITS(),
			 * but it needs valid channel for a country - so first call
			 * getCOUNTRY_CHANNELS() to get it
			 */
			bzero (ccd, sizeof(*ccd));
			ccd->version = APPLE80211_VERSION;
			ccd->num_channels = 0;
			bcopy (country_code, ccd->cc, cc_len);
			if ((getCOUNTRY_CHANNELS (interface, ccd) != 0) || (ccd->num_channels == 0))
			{
				continue;
			}
			bzero (ctpld, sizeof(*ctpld));
			ctpld->version = APPLE80211_VERSION;
			ctpld->country_code.version = APPLE80211_VERSION;
			bcopy (country_code, ctpld->country_code.cc, cc_len);
			ctpld->channel = ccd->supported_channels[0];
			if ((getCOUNTRY_TX_POWER_LIMITS (interface, ctpld) != 0) ||
				((ctpld->country_power_flags & APPLE80211_COUNTRY_TX_POWER_TYPE_MASK)
				!= APPLE80211_COUNTRY_TX_POWER_TYPE_WORLDWIDE))
			{
				continue;
			}
			/* Yes, it was default (minimum) CC */
		}
		/* CC accepted. Will put it into buffer */
		struct apple80211_country_code_data *apple_cc_data =
			&(sccd->country_codes[sccd->num_country_codes++]);
		apple_cc_data->version = APPLE80211_VERSION;
		bzero(apple_cc_data->cc, sizeof(apple_cc_data->cc));
		bcopy(country_code, apple_cc_data->cc, cc_len);
	}
cleanup:
	if (cl != NULL) {
		MFREE(osh, cl, WLC_IOCTL_MAXLEN);
	}
	if (ccd != NULL) {
		MFREE(osh, ccd, sizeof(struct apple80211_country_channel_data));
	}
	if (ctpld != NULL) {
		MFREE(osh, ctpld, sizeof(struct apple80211_country_tx_power_limits_data));
	}
	if (err) {
		bzero(sccd, sizeof(*sccd));
	}
	return err;
}
#endif /* APPLE80211_IOC_SUPPORTED_COUNTRY_CODES */

#ifdef APPLE80211_IOC_COUNTRY_TX_POWER_LIMITS

SInt32
AirPort_Brcm43xx::getCOUNTRY_TX_POWER_LIMITS(OSObject *interface,
	struct apple80211_country_tx_power_limits_data *cpld)
{
	/* Failsafe output data */
	cpld->country_power_flags = 0;
	cpld->num_tx_power_limits = 0;

	IOVERCHECK(cpld->version, "getCOUNTRY_TX_POWER_LIMITS");

	int err = 0;
	size_t buflen = WLC_IOCTL_MAXLEN;
	wlc_clm_power_limits_req_t *plr = NULL;
	ppr_t *ppr = 0;
	{
		/* Checking 'power_unit' input parameter for correctness */
		if ((cpld->power_unit != APPLE80211_UNIT_DBM) &&
			(cpld->power_unit != APPLE80211_UNIT_MW))
		{
			WL_ERROR(("invalid power_unit: %d. Shall be APPLE80211_UNIT_DBM or APPLE80211_UNIT_MW\n",
				cpld->power_unit));
			err = EINVAL;
			goto cleanup;
		}

		plr = (wlc_clm_power_limits_req_t *)MALLOCZ(osh, buflen);
		if (!plr) {
			err = ENOMEM;
			goto cleanup;
		}
		plr->version = WLC_CLM_POWER_LIMITS_REQ_VERSION;
		plr->buflen = buflen;
		bcopy(cpld->country_code.cc, plr->cc, sizeof(ccode_t));

		/* Translating channel/bandwidth to chanspec */
		chanspec_t chanspec;
		err = applechannel2chanspec(&cpld->channel, &chanspec);
		if (err != BCME_OK) {
			WL_ERROR(("invalid channel specification: version=%d, channel=%d, flags=0x%0X\n",
				cpld->channel.version, cpld->channel.channel, cpld->channel.flags));
			goto cleanup;
		}
		plr->chanspec = chanspec;

		/* Translating subchannel index */
		static const struct {
			enum apple80211_subchannel apple_sc;
			clm_limits_type clm_sc;
		} sc_translate[] = {
			{APPLE80211_SUBCHANNEL_FULL, CLM_LIMITS_TYPE_CHANNEL},
			{APPLE80211_SUBCHANNEL_L, CLM_LIMITS_TYPE_SUBCHAN_L},
			{APPLE80211_SUBCHANNEL_U, CLM_LIMITS_TYPE_SUBCHAN_U},
#ifdef WL11AC
			{APPLE80211_SUBCHANNEL_LL, CLM_LIMITS_TYPE_SUBCHAN_LL},
			{APPLE80211_SUBCHANNEL_LU, CLM_LIMITS_TYPE_SUBCHAN_LU},
			{APPLE80211_SUBCHANNEL_UL, CLM_LIMITS_TYPE_SUBCHAN_UL},
			{APPLE80211_SUBCHANNEL_UU, CLM_LIMITS_TYPE_SUBCHAN_UU},
			{APPLE80211_SUBCHANNEL_LLL, CLM_LIMITS_TYPE_SUBCHAN_LLL},
			{APPLE80211_SUBCHANNEL_LLU, CLM_LIMITS_TYPE_SUBCHAN_LLU},
			{APPLE80211_SUBCHANNEL_LUL, CLM_LIMITS_TYPE_SUBCHAN_LUL},
			{APPLE80211_SUBCHANNEL_LUU, CLM_LIMITS_TYPE_SUBCHAN_LUU},
			{APPLE80211_SUBCHANNEL_ULL, CLM_LIMITS_TYPE_SUBCHAN_ULL},
			{APPLE80211_SUBCHANNEL_ULU, CLM_LIMITS_TYPE_SUBCHAN_ULU},
			{APPLE80211_SUBCHANNEL_UUL, CLM_LIMITS_TYPE_SUBCHAN_UUL},
			{APPLE80211_SUBCHANNEL_UUU, CLM_LIMITS_TYPE_SUBCHAN_UUU},
#endif /* WL11AC */
		};
		int i;
		for (i = 0; (i < (int)ARRAYSIZE(sc_translate)) &&
			(sc_translate[i].apple_sc != cpld->subchannel); ++i);
		if (i == ARRAYSIZE(sc_translate)) {
			WL_ERROR(("invalid subchannel: %d\n", cpld->subchannel));
			err = EINVAL;
			goto cleanup;
		}
		plr->clm_subchannel = sc_translate[i].clm_sc;
		/* Done with subchannel index */

		plr->antenna_idx = cpld->antenna_index;
		/* Input parameter fields filled in */

		/* Requesting driver iovar */
		err = wlIovarOp("clm_power_limits", NULL, 0, plr, buflen, IOV_GET, interface);
		if (err != BCME_OK) {
			err = osl_error(err);
			goto cleanup;
		}
		/* Iovar request succeeded. Now will interpret the results */

		/* Filling in country flags */
		switch (plr->clm_country_flags_5g & CLM_FLAG_DFS_MASK) {
		case CLM_FLAG_DFS_NONE:
			cpld->country_power_flags |= APPLE80211_COUNTRY_TX_POWER_DFS_NONE;
			break;
		case CLM_FLAG_DFS_EU:
			cpld->country_power_flags |= APPLE80211_COUNTRY_TX_POWER_DFS_EU;
			break;
		case CLM_FLAG_DFS_US:
		default:
			cpld->country_power_flags |= APPLE80211_COUNTRY_TX_POWER_DFS_US;
			break;
		}
		if (plr->output_flags & WLC_CLM_POWER_LIMITS_OUTPUT_FLAG_DEFAULT_COUNTRY_LIMITS) {
			cpld->country_power_flags
				|= APPLE80211_COUNTRY_TX_POWER_TYPE_COUNTRY_DEFAULT;
		} else if (plr->output_flags & WLC_CLM_POWER_LIMITS_OUTPUT_FLAG_WORLDWIDE_LIMITS) {
			cpld->country_power_flags |= APPLE80211_COUNTRY_TX_POWER_TYPE_WORLDWIDE;
		} else {
			cpld->country_power_flags |= APPLE80211_COUNTRY_TX_POWER_TYPE_NORMAL;
		}

		/* Translating bandwidth into various representations */
		static const struct {
			u_int32_t apple_bw;
			wl_tx_bw_t wl_bw;
			ratespec_t rspec_bw;
		} bw_translate [] = {
			{APPLE80211_C_FLAG_20MHZ, WL_TX_BW_20, RSPEC_BW_20MHZ},
			{APPLE80211_C_FLAG_40MHZ, WL_TX_BW_40, RSPEC_BW_40MHZ},
#if defined(WL11AC) && defined(APPLE80211_IOC_VHT_MCS_INDEX_SET)
#ifdef CHSPEC_IS80
			{APPLE80211_C_FLAG_80MHZ, WL_TX_BW_80, RSPEC_BW_80MHZ},
#ifdef CHSPEC_IS160
			{APPLE80211_C_FLAG_160MHZ, WL_TX_BW_160, RSPEC_BW_160MHZ},
#endif /* CHSPEC_IS160 */
#endif /* CHSPEC_IS80 */
#endif /* defined(WL11AC) && defined(APPLE80211_IOC_VHT_MCS_INDEX_SET) */
		};
		for (i = 0; (i < (int)ARRAYSIZE(bw_translate)) &&
			!(bw_translate[i].apple_bw & cpld->channel.flags); ++i);
		if (i == ARRAYSIZE(bw_translate)) {
			WL_ERROR(("invalid bandwidth. Channel flags: 0x%X\n", cpld->channel.flags));
			err = EINVAL;
			goto cleanup;
		}
		wl_tx_bw_t wl_bw = bw_translate[i].wl_bw;
		/* rspec_bw will be set to subchannel (inner) bandwidth */
		if (cpld->subchannel >= APPLE80211_SUBCHANNEL_L) {
			i -= 1;
		}
		if (cpld->subchannel >= APPLE80211_SUBCHANNEL_LL) {
			i -= 1;
		}
		if (cpld->subchannel >= APPLE80211_SUBCHANNEL_LLL) {
			i -= 1;
		}
		if (i < 0) {
			err = EINVAL;
			goto cleanup;
		}
		ratespec_t rspec_bw = bw_translate[i].rspec_bw;

		/* Deserialize received limits into PPR buffer */
		ppr = ppr_create(osh, wl_bw);
		if (!ppr) {
			err = ENOMEM;
			goto cleanup;
		}
		err = ppr_convert_from_tlv(ppr, (uint8 *)plr->ppr_tlv, plr->ppr_tlv_size);
		if (err != BCME_OK) {
			err = osl_error(err);
			goto cleanup;
		}
		/* Limits are in PPR buffer - now will translate its content */

		/* Defining descriptors for rate groups (PPR ratesets) */
		typedef struct rate_group {
			typedef enum rate_type {
				RT_DSSS,
				RT_OFDM,
				RT_HT,
				RT_VHT
			} rate_type_t;
			rate_type_t rt;			/* Rate type */
			wl_tx_chains_t chains;		/* Number of expanded chains */
			wl_tx_nss_t actual_nss;		/* Actual number of unexpanded chains
							 * (number of chains in unexpanded rate)
							 */
			wl_tx_nss_t nominal_nss;	/* Nominal number of unexpanded chains
							 * (number of chains to pass to PPR)
							 */
			wl_tx_mode_t mode;		/* Expansion method */
			u_int32_t apple_rspec_exp;	/* Expansion-related
							 * APPLE80211_TX_RATE_SPEC_...flags
							 */
			ratespec_t rspec_exp;		/* Expansion-related RSPEC_RATE_... flags
							 */
		} rate_group_t;
#if WL_NUMRATES == 100
	/* Original rate definitions - no VHT rates, i.e. VHT rateset has HT length */
	#define VHT_RT RT_HT
#else
	/* Modern rate definitions, VHT rateset (almost?) always has VHT length */
	#define VHT_RT RT_VHT
	#define HAS_VHT_RATES
#endif /* WL_NUMRATES == 100 */
#if WL_NUMRATES <= 178
	/* VHT TXBF0 rateset has HT length */
	#define VHT_TXBF0_RT RT_HT
#else
	/* VHT TXBF0 rateset has VHT length */
	#define VHT_TXBF0_RT RT_VHT
#endif /* WL_NUMRATES <= 178 */
#if WL_NUMRATES >= 178
	#define HAS_TXBF
#endif /* WL_NUMRATES >= 178 */
#if WL_NUMRATES >= 336
	#define HAS_4TX
#endif /* WL_NUMRATES >= 336 */
/* In subsequent macros 'chains' is number of expanded chains, nss is number of unexpanded chains
 */
#define RATE_GROUP_DSSS(chains) {rate_group::RT_DSSS, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_1, WL_TX_NSS_1, WL_TX_MODE_NONE, \
	(chains - 1) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT, \
	RSPEC_ENCODE_RATE | (chains - 1) << RSPEC_TXEXP_SHIFT}
#define RATE_GROUP_OFDM() {rate_group::RT_OFDM, WL_TX_CHAINS_1, \
	WL_TX_NSS_1, WL_TX_NSS_1, WL_TX_MODE_NONE, \
	0, \
	RSPEC_ENCODE_RATE}
#define RATE_GROUP_OFDM_CDD(chains) {rate_group::RT_OFDM, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_1, WL_TX_NSS_1, WL_TX_MODE_CDD, \
	(chains - 1) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT, \
	RSPEC_ENCODE_RATE | (chains - 1) << RSPEC_TXEXP_SHIFT}
#define RATE_GROUP_OFDM_TXBF(chains) {rate_group::RT_OFDM, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_1, WL_TX_NSS_1, WL_TX_MODE_TXBF, \
	((chains - 1) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT) | APPLE80211_TX_RATE_SPEC_EXP_TXBF, \
	RSPEC_ENCODE_RATE | ((chains - 1) << RSPEC_TXEXP_SHIFT) | RSPEC_TXBF}
#define RATE_GROUP_VHTSS1() {rate_group::VHT_RT, WL_TX_CHAINS_1, \
	WL_TX_NSS_1, WL_TX_NSS_1, WL_TX_MODE_NONE, \
	0, \
	RSPEC_ENCODE_VHT}
#define RATE_GROUP_VHT_SS1_CDD(chains) {rate_group::VHT_RT, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_1, WL_TX_NSS_1, WL_TX_MODE_CDD, \
	(chains - 1) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT, \
	RSPEC_ENCODE_VHT | (chains - 1) << RSPEC_TXEXP_SHIFT}
#define RATE_GROUP_VHT_SS1_STBC(chains) {rate_group::VHT_RT, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_1, WL_TX_NSS_2, WL_TX_MODE_STBC, \
	((chains - 1) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT) | APPLE80211_TX_RATE_SPEC_EXP_STBC, \
	RSPEC_ENCODE_VHT | ((chains - 1) << RSPEC_TXEXP_SHIFT) | RSPEC_STBC}
#define RATE_GROUP_VHT_SS1_TXBF(chains) {rate_group::VHT_RT, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_1, WL_TX_NSS_1, WL_TX_MODE_TXBF, \
	((chains - 1) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT) | APPLE80211_TX_RATE_SPEC_EXP_TXBF, \
	RSPEC_ENCODE_VHT | ((chains - 1) << RSPEC_TXEXP_SHIFT) | RSPEC_TXBF}
#define RATE_GROUP_VHT(chains, nss) {rate_group::VHT_RT, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_##nss, WL_TX_NSS_##nss, WL_TX_MODE_NONE, \
	(chains - nss) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT, \
	RSPEC_ENCODE_VHT | (chains - nss) << RSPEC_TXEXP_SHIFT}
#define RATE_GROUP_VHT_TXBF0(chains) {rate_group::VHT_TXBF0_RT, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_##chains, WL_TX_NSS_##chains, WL_TX_MODE_TXBF, \
	APPLE80211_TX_RATE_SPEC_EXP_TXBF, \
	RSPEC_ENCODE_VHT | RSPEC_TXBF}
#define RATE_GROUP_VHT_TXBF(chains, nss) {rate_group::RT_VHT, WL_TX_CHAINS_##chains, \
	WL_TX_NSS_##nss, WL_TX_NSS_##nss, WL_TX_MODE_TXBF, \
	((chains - nss) << APPLE80211_TX_RATE_SPEC_TXEXP_SHIFT) | APPLE80211_TX_RATE_SPEC_EXP_TXBF, \
	RSPEC_ENCODE_VHT | ((chains - nss) << RSPEC_TXEXP_SHIFT) | RSPEC_TXBF}
		static const rate_group_t rate_groups[] = {
			RATE_GROUP_DSSS(1),
			RATE_GROUP_OFDM(),
			RATE_GROUP_VHTSS1(),

			RATE_GROUP_DSSS(2),
			RATE_GROUP_OFDM_CDD(2),
			RATE_GROUP_VHT_SS1_CDD(2),
			RATE_GROUP_VHT_SS1_STBC(2),
			RATE_GROUP_VHT(2, 2),

			RATE_GROUP_DSSS(3),
			RATE_GROUP_OFDM_CDD(3),
			RATE_GROUP_VHT_SS1_CDD(3),
			RATE_GROUP_VHT_SS1_STBC(3),
			RATE_GROUP_VHT(3, 2),
			RATE_GROUP_VHT(3, 3),

#ifdef HAS_TXBF
			RATE_GROUP_OFDM_TXBF(2),
			RATE_GROUP_VHT_SS1_TXBF(2),
			RATE_GROUP_VHT_TXBF0(2),

			RATE_GROUP_OFDM_TXBF(3),
			RATE_GROUP_VHT_SS1_TXBF(3),
			RATE_GROUP_VHT_TXBF(3, 2),
			RATE_GROUP_VHT_TXBF0(3),
#ifdef HAS_4TX
			RATE_GROUP_DSSS(4),
			RATE_GROUP_OFDM_CDD(4),
			RATE_GROUP_VHT_SS1_CDD(4),
			RATE_GROUP_VHT_SS1_STBC(4),
			RATE_GROUP_VHT(4, 3),
			RATE_GROUP_VHT(4, 4),
			RATE_GROUP_VHT(4, 2),
			RATE_GROUP_OFDM_TXBF(4),
			RATE_GROUP_VHT_SS1_TXBF(4),
			RATE_GROUP_VHT_TXBF(4, 2),
			RATE_GROUP_VHT_TXBF(4, 3),
			RATE_GROUP_VHT_TXBF0(4),
#endif /* HAS_4TX */
#endif /* HAS_TXBF */
		};
#undef VHT_RT
#undef VHT_TXBF0_RT
#ifdef HAS_4TX
	#undef HAS_4TX
#endif /* HAS_4TX */
#ifdef HAS_VHT_RATES
	#undef HAS_VHT_RATES
#endif /* HAS_VHT_RATES */
#undef RATE_GROUP_DSSS
#undef RATE_GROUP_OFDM
#undef RATE_GROUP_OFDM_EXP
#undef RATE_GROUP_VHTSS1
#undef RATE_GROUP_VHT_SS1_CDD
#undef RATE_GROUP_VHT_SS1_STBC
#undef RATE_GROUP_VHT_SS1_TXBF
#undef RATE_GROUP_VHT
#undef RATE_GROUP_VHT_TXBF0
#undef RATE_GROUP_VHT_TXBF
		/* Rate codes in 500kbps units for 802.11a/b/g rates */
		static const uint dsss_rate_codes[] = {
			WLC_RATE_1M, WLC_RATE_2M, WLC_RATE_5M5, WLC_RATE_11M
		};
		static const uint ofdm_rate_codes[] = {
			WLC_RATE_6M, WLC_RATE_9M, WLC_RATE_12M, WLC_RATE_18M,
			WLC_RATE_24M, WLC_RATE_36M, WLC_RATE_48M, WLC_RATE_54M
		};

		/* Loop over rate groups */
		size_t gi;
		for (gi = 0; gi < ARRAYSIZE(rate_groups); ++gi) {
			/* Retrieved limits for rate group */
			ppr_dsss_rateset_t dsss_limits;
			ppr_ofdm_rateset_t ofdm_limits;
			ppr_ht_mcs_rateset_t mcs_limits;
			ppr_vht_mcs_rateset_t vht_limits;
			/* Limits for current group */
			const int8 *limits = NULL;
			/* Number of rates in group */
			size_t num_rates = 0;
			/* Rate codes for curernt rate group (if OFDM/DSSS) */
			const uint *rate_codes = NULL;

			switch (rate_groups[gi].rt) {
			case rate_group::RT_DSSS:
				ppr_get_dsss(ppr, wl_bw, rate_groups[gi].chains, &dsss_limits);
				limits = dsss_limits.pwr;
				num_rates = ARRAYSIZE(dsss_limits.pwr);
				rate_codes = dsss_rate_codes;
				break;
			case rate_group::RT_OFDM:
				ppr_get_ofdm(ppr, wl_bw, rate_groups[gi].mode,
					rate_groups[gi].chains, &ofdm_limits);
				limits = ofdm_limits.pwr;
				num_rates = ARRAYSIZE(ofdm_limits.pwr);
				rate_codes = ofdm_rate_codes;
				break;
			case rate_group::RT_HT:
				ppr_get_ht_mcs(ppr, wl_bw, rate_groups[gi].nominal_nss,
					rate_groups[gi].mode, rate_groups[gi].chains, &mcs_limits);
				limits = mcs_limits.pwr;
				num_rates = ARRAYSIZE(mcs_limits.pwr);
				break;
			case rate_group::RT_VHT:
				ppr_get_vht_mcs(ppr, wl_bw, rate_groups[gi].nominal_nss,
					rate_groups[gi].mode, rate_groups[gi].chains, &vht_limits);
				limits = vht_limits.pwr;
				num_rates = ARRAYSIZE(vht_limits.pwr);
				break;
			default:
				ASSERT(0);
				continue;
			}
			size_t ri;
			for (ri = 0; ri < num_rates; ++ri) {
				if (limits[ri] == WL_RATE_DISABLED) {
					continue;
				}
				if (cpld->num_tx_power_limits == ARRAYSIZE(cpld->tx_power_limits)) {
					continue; /* Output buffer capacity exceeded (improbable,
						   * but just in case) - skip it
						   */
				}
				/* Computing various rate characteristics */
				ratespec_t rspec = rate_groups[gi].rspec_exp | rspec_bw;
				u_int32_t apple_rspec = rate_groups[gi].apple_rspec_exp;
				u_int32_t apple_rate_flags = 0;
				switch (rate_groups[gi].rt) {
				case rate_group::RT_DSSS:
				case rate_group::RT_OFDM:
					rspec |= (ratespec_t)rate_codes[ri];
					apple_rspec |= APPLE80211_TX_RATE_SPEC_TYPE_80211 |
						rate_codes[ri];
					apple_rate_flags |= APPLE80211_RATE_FLAG_BASIC;
					break;
				case rate_group::RT_HT:
				case rate_group::RT_VHT:
					rspec |= (ratespec_t)ri |
						(ratespec_t)(rate_groups[gi].actual_nss <<
						RSPEC_VHT_NSS_SHIFT);
					if (ri < 8) {
						apple_rspec |= APPLE80211_TX_RATE_SPEC_TYPE_HT |
							(ratespec_t)ri |
							(ratespec_t)((rate_groups[gi].actual_nss -
							1) << 3);
						apple_rate_flags |= APPLE80211_RATE_FLAG_HT;
					} else {
						apple_rspec |= APPLE80211_TX_RATE_SPEC_TYPE_VHT |
							(ratespec_t)ri |
							(ratespec_t)(rate_groups[gi].actual_nss <<
							APPLE80211_TX_RATE_SPEC_VHT_NSS_SHIFT);
						apple_rate_flags |= APPLE80211_RATE_FLAG_VHT;
					}
					break;
				}
				/* Filling in data for rate */
				cpld->tx_power_limits[cpld->num_tx_power_limits].version =
					APPLE80211_VERSION;
				cpld->tx_power_limits[cpld->num_tx_power_limits].rate.version =
					APPLE80211_VERSION;
				cpld->tx_power_limits[cpld->num_tx_power_limits].rate.flags =
					apple_rate_flags;
				cpld->tx_power_limits[cpld->num_tx_power_limits].rate.rate =
					RSPEC2MBPS(rspec);
				cpld->tx_power_limits[cpld->num_tx_power_limits].tx_rate_spec =
					apple_rspec;
				cpld->tx_power_limits[cpld->num_tx_power_limits].power =
					(cpld->power_unit == APPLE80211_UNIT_DBM)
					? (limits[ri] / 4) : bcm_qdbm_to_mw(limits[ri]);
				cpld->num_tx_power_limits += 1;
				/* Data for current rate filled in */
			}

		}
	}
cleanup:
	if (plr) {
		MFREE (osh, plr, buflen);
	}
	if (ppr) {
		ppr_delete(osh, ppr);
	}
	if (err) {
		bzero(cpld, sizeof(cpld));
	}
	return err;
}
#endif /* APPLE80211_IOC_COUNTRY_TX_POWER_LIMITS */
