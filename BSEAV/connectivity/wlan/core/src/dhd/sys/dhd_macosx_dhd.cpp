/*
 * Mac OS X specific portion of
 * Broadcom 802.11 Networking Device Driver
 * DHD specific version
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: $
 */
/* FILE-CSTYLED */
#include <libkern/libkern.h>
#include <proto/ethernet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/proc.h>

#include <IOKit/IOTypes.h>
#include <IOKit/IOLib.h>

#include <IOKit/network/IOEthernetController.h>
#include <IOKit/apple80211/apple80211_var.h>
#include <IOKit/apple80211/apple80211_ioctl.h>
#include <IOKit/apple80211/IO80211Controller.h>
#include <IOKit/apple80211/IO80211Interface.h>
#include <IOKit/apple80211/IO80211WorkLoop.h>


#include <typedefs.h>
#include <osl.h>
#include <bcmendian.h>

extern "C"
{
	#include <typedefs.h>
	#include <bcmendian.h>
	#include <bcmutils.h>
	#include <bcmdevs.h>
	#include <osl.h>
	#include <epivers.h>
	#include <siutils.h>
	#include <dngl_stats.h>
	#include <pcie_core.h>
	#include <dhd.h>
	#include <dhd_bus.h>
	#include <dhd_proto.h>
	#include <dhd_dbg.h>
	#include <dhd_pcie.h>
	#include <bcmmsgbuf.h>
	#include <pcicfg.h>
	#include <dhd_flowring.h>
	#include <d11.h>
	#include <wlc_cfg.h>
	#include <wlc_pub.h>
	#include <wl_export.h>
	#include <wl_dbg.h>
	#include <dhd_timesync.h> 

	void dhd_sendup(struct wl_info *drv, int ifidx, void *m, int numpkt);
}

#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>

#include <wl_macosx.h>
uint dhd_watchdog_ms = CUSTOM_DHD_WATCHDOG_MS;


/* TX FIFO number to WME/802.1E Access Category */

/* WME/802.1E Access Category to TX FIFO number */
const uint8 wme_fifo2ac[] = { AC_BK, AC_BE, AC_VI, AC_VO, AC_BE, AC_BE };
const uint8 prio2fifo[NUMPRIO] = {
	TX_AC_BE_FIFO,	/* 0	BE	AC_BE	Best Effort */
	TX_AC_BK_FIFO,	/* 1	BK	AC_BK	Background */
	TX_AC_BK_FIFO,	/* 2	--	AC_BK	Background */
	TX_AC_BE_FIFO,	/* 3	EE	AC_BE	Best Effort */
	TX_AC_VI_FIFO,	/* 4	CL	AC_VI	Video */
	TX_AC_VI_FIFO,	/* 5	VI	AC_VI	Video */
	TX_AC_VO_FIFO,	/* 6	VO	AC_VO	Voice */
	TX_AC_VO_FIFO	/* 7	NC	AC_VO	Voice */
};
#define WME_PRIO2AC(prio)  wme_fifo2ac[prio2fifo[(prio)]]

#define SAFE_RELEASE(p) if (p) {p->release(); p = NULL;}

static char pfw_path[] = "/bcm-wifi/rtecdc.bin";
static char pnv_path[] = "/bcm-wifi/nvram.txt";
static char pclm_path[] = "/bcm-wifi/bcmdhd_clm.blob";

static char ioctl_buf[WLC_IOCTL_MEDLEN];
uint wl_msg_level = WL_ERROR_VAL;
uint wl_msg_level2 = WL_PCIE_VAL;

typedef struct dhd_event {
	wl_event_msg_t event;
	void * data;
	struct dhd_event *next;
} dhd_event_t;

typedef struct eventq
{
	dhd_event_t *head;
	dhd_event_t *tail;
	struct wl_timer	*timer;
	bool tpending;
} eventq_t;

class Dhd_Iface
{
	public:
		int idx;
		OSObject *os_iface;
		uint8 mac_addr[ETHER_ADDR_LEN];
		uint8 bssidx;
		char name[IFNAMSIZ+1];		//OS specific interface name
		char wlname[IFNAMSIZ+1];	//wl specific interface name
		uint8 role;

		//constructor
		Dhd_Iface(OSObject *os_iface, int ifidx,
				char *name, uint8 *mac, uint8 bssidx, uint8 role);
};

class Dhd_Iface_List
{
	private:
		Dhd_Iface *iflist[DHD_MAX_IFS];

	public:
		Dhd_Iface_List();
		int Iface2Idx(OSObject * intf) const;
		int Idx2Role(int idx);
		OSObject *Idx2Iface(int ifidx);
		int Wlname2Idx(char *ifname);
		int AddIface(OSObject *os_iface, int ifidx,
				char *name, uint8 *mac, uint8 bssidx, uint8 role);
		int DeleteIface(int ifidx);
		int UpdateIface(int ifidx, uint8 *mac, char *name);
};


//===========================================================================
// AirPort_Brcm43xx_Dhd class
//===========================================================================
class AirPort_Brcm43xx_Dhd : public AirPort_Brcm43xx
{
	OSDeclareDefaultStructors(AirPort_Brcm43xx_Dhd)

public:
	//Public functions
	bool start( IOService *provider );
	IOReturn setHardwareAddress(const IOEthernetAddress * addr);
	bool init(OSDictionary * properties);
	IOService *probe(IOService * provider, SInt32 *score);
	bool wlcStart();
	void wlcStop();
	void free();

	int enqueueEvent(int ifidx, wl_event_msg_t *e, void *event_data);
	int dispatchEvent();
	void *getOSSpinLock() { return dhdOSSpinlock; }
	bool takeProtLock();
	bool releaseProtLock();

	void interruptDriverWL();
	void inputPacketQueued(mbuf_t m, int ifidx, int16 rssi);
	void wlEvent( char *ifname, wl_event_msg_t *evt, void *data );
	SInt32 apple80211Request( UInt32 req, int type, IO80211Interface * intf, void * data);

	/* Device Interrupt Management Functions */
	int disableDeviceInterrupt();
	int enableDeviceInterrupt();

	//Public data
	int rx_ifidx;
	Dhd_Iface_List iface_list;
	void *rx_pkt;
	void *rx_pkt_tail;
	void *workLoopSpinlock;
	osl_t	*osh;

private:
	// Private functions
	void macosWatchdog();
	int wlIoctl(uint cmd, void *arg, size_t len, bool set, OSObject *iface);
	int wlIoctlGet(uint cmd, void *arg, size_t len, OSObject *iface) const;
	int wlIovarOp(const char *name, void *params, size_t p_len, void *buf, size_t len, bool set, OSObject *iface);
	int wlIovarGetInt(const char *name, int *arg) const;
	int wlIovarGetInt(const char *name, int *arg, OSObject *iface);
	bool wlSendPkt( void *sdu, void *iface);
	bool isAssociated();
	bool isUp();
	bool isBusUp();
	int getBusState();
	SInt32	setPOWER( OSObject * interface, struct apple80211_power_data * pd );
	SInt32	getPOWER( OSObject * interface, struct apple80211_power_data * pd );

	bool enableInterrupt();
	static void interruptEntryPoint(OSObject*, IOInterruptEventSource *src, int count);
	void interruptEntryHandler(IOInterruptEventSource *src, int count);
	static void interruptDWL(OSObject*, IOInterruptEventSource *src, int count);

	bool chipMatch();

	int eventq_attach();
	void eventq_detach();
	dhd_event_t* event_alloc(wl_event_msg_t *msg, void *data);
	void eventq_enq(dhd_event_t *e);
	dhd_event_t* eventq_deq();
	void event_free(dhd_event_t *e);

	// Private data
	eventq_t *eventq;

	IOPCIDevice *pciDev;
	IOMemoryMap *ioMapBAR0;
	IOMemoryMap *ioMapBAR1;
	IOWorkLoop *intrWorkLoop;
	IOInterruptEventSource *intrEventSrcMSI;
	IOInterruptEventSource *intrEventSrcMSI_DWL;

	IOLock *eventLock;
	IOLock *dhdProtLock;
	void *dhdOSSpinlock;

	dhd_bus_t *bus;

	// The index of the Interrupt Source in the device.
	// Used to disable/enable the interrupts.
	// Populated from enableInterrupt, where the appropriate index is identified by calling getInterruptType
	int interrupt_index;
};

extern "C" {
static void
event_dispatch(void *h)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd *)h;
	pobj->dispatchEvent();

}
} /* extern "C" */


Dhd_Iface::Dhd_Iface(OSObject *os_iface, int ifidx,
				char *name, uint8 *mac, uint8 bssidx, uint8 role)
{
	this->idx = ifidx;
	this->bssidx = bssidx;
	this->os_iface = os_iface;
	this->role = role;

	memset(this->name, 0, sizeof(this->name));

	if (name)
		strncpy(this->wlname, name, IFNAMSIZ);
	else
		memset(this->wlname, 0, sizeof(this->name));

	if (mac)
		memcpy(this->mac_addr, mac, ETHER_ADDR_LEN);
	else
		memset(this->mac_addr, 0, sizeof(this->mac_addr));
}


Dhd_Iface_List::Dhd_Iface_List()
{
	memset(iflist, 0, DHD_MAX_IFS);
}


int
Dhd_Iface_List::AddIface(OSObject *os_iface, int ifidx,
			char *name, uint8 *mac, uint8 bssidx, uint8 role)
{
	int i = 0;

	if (!os_iface || ifidx < 0) {
		DHD_ERROR(("%s: bad ptr or bad ifidx...\n", __func__));
		return BCME_BADARG;
	}

	//find an empty slot
	for (i = 0; i < DHD_MAX_IFS; ++i) {
		if (!iflist[i])
			break;
	}

	if (i >= DHD_MAX_IFS) {
		DHD_ERROR(("%s: no free slots !\n", __func__));
		return BCME_NOMEM;
	}

	iflist[i] = new Dhd_Iface(os_iface, ifidx, name, mac, bssidx, role);
	if (!iflist[i]) {
		DHD_ERROR(("%s: out of mem !\n", __func__));
		return BCME_NOMEM;
	}

	if (name)
		DHD_ERROR(("%s: Successfully alloc'd new iface at slot %d: os_iface = %p; ifidx = %d; name = %s; role = %d "
			"bssidx=%d\n", __func__, i, os_iface, ifidx, name, role, bssidx));

	return BCME_OK;
}

int
Dhd_Iface_List::DeleteIface(int ifidx)
{
	int i = 0;

	if (ifidx < 0)
		return BCME_BADARG;

	//try to find the entry
	for (i = 0; i < DHD_MAX_IFS; ++i) {
		if (iflist[i] && iflist[i]->idx == ifidx) {
			DHD_ERROR(("%s: Removing iface at slot %d: ifidx = %d; \n", __func__, i, ifidx));
			delete (iflist[i]);
			iflist[i] = NULL;
			return BCME_OK;
		}
	}

	return BCME_NOTFOUND;
}

int
Dhd_Iface_List::UpdateIface(int ifidx, uint8 *mac, char *name)
{
	int i = 0;

	if (ifidx < 0)
		return BCME_BADARG;

	for (i = 0; i < DHD_MAX_IFS; ++i) {
		if ( iflist[i] && iflist[i]->idx == ifidx ) {
			memcpy(iflist[i]->mac_addr, mac, ETHER_ADDR_LEN);
			strncpy(iflist[i]->name, name, IFNAMSIZ);
			return 0;
		}
	}

	return BCME_NOTFOUND;
}

int
Dhd_Iface_List::Iface2Idx(OSObject * intf) const
{
	int i = 0;
	int ifidx = -1;

	if (!intf)
		return -1;

	for (i = 0; i < DHD_MAX_IFS; ++i) {
		if (iflist[i] && iflist[i]->os_iface == intf) {
			return iflist[i]->idx;
		}
	}

	return ifidx;
}

int
Dhd_Iface_List::Idx2Role(int ifidx)
{
	int i = 0;
	int role = -1;

	if (ifidx < 0)
		return -1;

	for (i = 0; i < DHD_MAX_IFS; ++i) {
		if (iflist[i] && iflist[i]->idx == ifidx) {
			return iflist[i]->role;
		}
	}

	return role;
}

OSObject *
Dhd_Iface_List::Idx2Iface(int ifidx)
{
	int i = 0;

	if (ifidx < 0)
		return NULL;

	for (i = 0; i < DHD_MAX_IFS; ++i) {
		if (iflist[i] && iflist[i]->idx == ifidx) {
			return iflist[i]->os_iface;
		}
	}

	return NULL;
}

int
Dhd_Iface_List::Wlname2Idx(char *ifname)
{
	int i = 0;

	if (!ifname)
		return 0;

	for (i = 0; i < DHD_MAX_IFS; ++i) {
		if (iflist[i] && strncmp(ifname, iflist[i]->wlname, IFNAMSIZ) == 0) {
			return iflist[i]->idx;
		}
	}

	return 0;
}

#pragma mark -
#pragma mark AirPort_Brcm43xx_Dhd Implementation
//===========================================================================
// AirPort_Brcm43xx_Dhd
//===========================================================================
#undef  super
#define super AirPort_Brcm43xx
OSDefineMetaClassAndStructors( AirPort_Brcm43xx_Dhd, AirPort_Brcm43xx )

bool
AirPort_Brcm43xx_Dhd::start( IOService *provider )
{
	bool ret = FALSE;
	int err = 0;
	char name[IFNAMSIZ];

	//all the driver, pcie and fw initialization is done by the base class
	ret = super::start(provider);

	getOsIfaceName(name);

	err = iface_list.AddIface(getPrimaryIface(),
				0, name, NULL, 0, WLC_E_IF_ROLE_STA
				);
	if (err)
		DHD_ERROR(("%s: AddIface ERROR !...\n", __func__));

	return ret;
}

IOReturn
AirPort_Brcm43xx_Dhd::setHardwareAddress(const IOEthernetAddress * addr)
{
	IOReturn ret = 0;
	char name[IFNAMSIZ];

	ret = super::setHardwareAddress(addr);

	if( getPrimaryIface() ) {
		getOsIfaceName(name);
		//update the interface data in the driver, the proper iface name would be available now
		if ( iface_list.UpdateIface(0, (uint8 *) addr->bytes, name) < 0 )
			DHD_ERROR(("%s: UpdateIface ERROR ! \n", __func__));
		else
			DHD_ERROR(("%s: updated iface %s, idx = 0, with hw addr = %x.%x.%x.%x.%x.%x \n",
				__func__, name, addr->bytes[0], addr->bytes[1], addr->bytes[2],
				addr->bytes[3], addr->bytes[4], addr->bytes[5]));
	}

	return ret;
}

bool
AirPort_Brcm43xx_Dhd::init( OSDictionary * properties )
{
	printf("Broadcom Dongle Host Driver (DHD) for Mac OSX, version %s\n", EPI_VERSION_STR);
	printf("Compiled from %s\n", __FILE__);
	printf("Compiled on %s at %s\n\n", __DATE__, __TIME__);

	if (!super::init(properties)) {
		return false;
	}

	eventLock = IOLockAlloc();
	dhdProtLock = IOLockAlloc();
	dhdOSSpinlock = osl_spin_alloc_init();
	workLoopSpinlock = osl_spin_alloc_init();
	rx_pkt = rx_pkt_tail = NULL;

	return true;

}

IOService*
AirPort_Brcm43xx_Dhd::probe(IOService *provider, SInt32 *score)
{
	if(super::probe( provider, score)) {
		WL_PORT(("%d\n", (int)*score));
	}

	return(this);
}

bool
AirPort_Brcm43xx_Dhd::wlcStart()
{
	pciDev = super::getPCIDevice();
	if (!pciDev) {
		DHD_ERROR(("%s OSDynamicCast(IOPCIDevice, provider) failed!\n", __func__));
		return false;
	}

	osh = osl_attach(pciDev);
	if (!osh) {
		DHD_ERROR(("%s osl_attach failed!\n", __func__));
		return false;
	}
	OSL_DMADDRWIDTH(osh, DMADDRWIDTH_64);
	eventq_attach();
	if (!chipMatch()) {
		DHD_ERROR(("%s chipMatch failed!\n", __func__));
		return false;
	}

	// Dongle side needs to be up before super::wlcStart() call.
	if (!super::wlcStart()) {
		kprintf("%s: base wlcStart failed\n", __func__);
		return false;
	}
	/* Add a timer to host the watchdog. */
	if (!(watchdog_timer = wl_init_timer((struct wl_info *)this, (void (*)(void*))wl_macos_watchdog,
					(void *)this, "macos_dhd_watchdog"))) {
		WL_ERROR(("wl_init_timer for watchdog failed\n"));
		return false;
	}

	/* start a timer to fire every dhd_watchdog_ms ms */
	wl_add_timer((struct wl_info *)this, watchdog_timer, dhd_watchdog_ms, true);

	return true;
}

void
AirPort_Brcm43xx_Dhd::free()
{
	WL_PORT(("%s\n", __func__));

	if (eventLock) {
		IOLockFree(eventLock);
		eventLock = NULL;
	}
	if (dhdProtLock) {
		IOLockFree(dhdProtLock);
		dhdProtLock = NULL;
	}
	if (dhdOSSpinlock) {
		osl_spin_free(dhdOSSpinlock);
		dhdOSSpinlock = NULL;
	}
	if (workLoopSpinlock) {
		osl_spin_free(workLoopSpinlock);
		workLoopSpinlock = NULL;
	}
	if (osh) {
		osl_detach(osh);
		osh = NULL;
	}
	super::free();
}

void
AirPort_Brcm43xx_Dhd::wlcStop()
{
	// Destroy spinlock by calling super::wlcStop()
	IOWorkLoop *driverWorkLoop;

	eventq_detach();

	/* In this case the base class relies upon the dongle to
	 * be present for clean stop()
	 */
	super::wlcStop();
	driverWorkLoop = getWorkLoop();

	if (intrEventSrcMSI) {
		intrEventSrcMSI->disable();
		if (intrWorkLoop) {
			intrWorkLoop->removeEventSource(intrEventSrcMSI);
		}
		SAFE_RELEASE(intrEventSrcMSI);
	}
	SAFE_RELEASE(intrWorkLoop);
	SAFE_RELEASE(ioMapBAR0);
	SAFE_RELEASE(ioMapBAR1);

	dhdpcie_bus_release(bus);
	WL_PORT(("%s\n", __func__));
}

bool
AirPort_Brcm43xx_Dhd::isAssociated()
{
	return dhd_is_associated(bus->dhd, NULL, NULL);
}

bool
AirPort_Brcm43xx_Dhd::isUp()
{
	int isup = 0;
	int err = 0;

	err = wlIoctlGetInt(WLC_GET_UP, &isup);

	if (err) {
		DHD_ERROR(("wl%d: %s error (%d) from ioctl WLC_GET_UP\n",
			unit, __FUNCTION__, err));
	}
	WL_IOCTL(("isUp= %d\n", isup));
	return isup;
}

int
AirPort_Brcm43xx_Dhd::getBusState()
{
	return bus->dhd->busstate;
}

bool
AirPort_Brcm43xx_Dhd::isBusUp()
{
	return (getBusState() != DHD_BUS_DOWN) ? TRUE : FALSE;
}

SInt32
AirPort_Brcm43xx_Dhd::setPOWER( OSObject * interface, struct apple80211_power_data * pd )
{
	int bcmerr = 0;
	uint power_on_mask = 0;
	ASSERT(pd);
	if (!pd)
		return EINVAL;

	IOVERCHECK(pd->version, "setPOWER");

	WL_IOCTL(("wl%d: AirPort_Brcm43xx_Dhd::setPOWER:\n", unit));
	for (uint i = 0; i < pd->num_radios; i++) {
		WL_IOCTL(("\tpower_state[%d]: %s\n", i,
		          lookup_name(apple80211_power_state_names, pd->power_state[i])));
	}

	switch (pd->power_state[0]) {
		case APPLE80211_POWER_ON:
		case APPLE80211_POWER_TX:
		case APPLE80211_POWER_RX:
			power_on_mask = true;
			break;
		case APPLE80211_POWER_OFF:
		default:
			break;
	}
	WL_IOCTL(("power_on_mask :%d\n", power_on_mask));

	if (power_on_mask != 0) {
		wl_up((struct wl_info *)this);
	} else {
		wl_down((struct wl_info *)this);
	}

	return osl_error(bcmerr);
}
/**
 * Since we are having local definitiaon for setPOWER we should also have for getPOWER,
 * because apple80211Request requests both with APPLE80211_IOC_POWER. The differentiation is
 * through SIOCGA80211/SIOCSA80211.
 */
SInt32
AirPort_Brcm43xx_Dhd::getPOWER( OSObject * interface, struct apple80211_power_data * pd )
{
	return super::getPOWER(interface, pd);
}
/**
 * apple80211Request overrides AirPort_Brcm43xx(base class) method.
 * Any request which needs to be handled differently for MACOS DHD are hanled in this method.
 * Otherwise invoke parent.
 */
SInt32
AirPort_Brcm43xx_Dhd::apple80211Request( UInt32 req, int type, IO80211Interface * intf, void * data )
{
	SInt32 ret = EOPNOTSUPP;

	if(req != SIOCGA80211 && req != SIOCSA80211)
		return EINVAL;

	switch (type)
	{
		case APPLE80211_IOC_POWER:
			GETSET( POWER, apple80211_power_data );
		default:
			ret = super::apple80211Request(req, type, intf, data);
	}

	return ret;
}

bool
AirPort_Brcm43xx_Dhd::wlSendPkt(void *sdu, void *iface)
{
	int ret;
	dhd_pub_t *dhd;
	int ifidx = 0;

	/* Check the status of the Bus. If its down, no need to proceed further */
	dhd = bus->dhd;
	if (getBusState() == DHD_BUS_DOWN) {
		DHD_ERROR(("%s: xmit rejected pub.up=%d busstate=%d \n",
			__FUNCTION__, dhd->up, dhd->busstate));
		PKTFREE(bus->osh, sdu, TRUE);
		return BCME_NOTUP;
	}

	if (iface) {
		ifidx = iface_list.Iface2Idx( (OSObject *)iface );
		ifidx = (ifidx < 0) ? 0 : ifidx;
	}
	ret = dhd_flowid_update(bus->dhd, ifidx, WME_PRIO2AC(PKTPRIO((mbuf_t)sdu)), sdu);
	if (ret != BCME_OK) {
		printf("%s: error from flowid_update %d\n", __func__, ret);
		PKTFREE(bus->osh, sdu, TRUE);
		return ret;
	}

	ret = dhd_bus_txdata(bus, sdu, ifidx);
	if (ret != BCME_OK) {
		printf("%s: error %d\n", __func__, ret);
	}

	return ret;
}

int
AirPort_Brcm43xx_Dhd::wlIovarOp(const char *name, void *params, size_t p_len, void *buf, size_t len, bool set, OSObject *iface)
{
	int ret;
	int iovar_len;
	uint buflen = sizeof(ioctl_buf);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	iovar_len = bcm_mkiovar((char *)name, (char*)params, p_len, ioctl_buf, buflen);
	if (iovar_len == 0) {
		DHD_ERROR(("%s: insufficient buffer space passed to bcm_mkiovar for name %s\n", __func__, name));
		ASSERT(0);
		return BCME_BUFTOOSHORT;
	}
	if (set) {
		if (len+iovar_len > buflen) {
			printf("%s: buffer too short\n", __func__);
			return BCME_BUFTOOSHORT;
		}
		memcpy(ioctl_buf + iovar_len, buf, len);
		iovar_len += len;
	}

	ret =  wlIoctl((set) ? WLC_SET_VAR : WLC_GET_VAR,
			ioctl_buf, (set) ? iovar_len : buflen, set, iface);
	if (ret < 0) {
		DHD_ERROR(("%s wlIoctl failed. name %s, ret %d\n",
			__func__, name, ret));
	}
	else if (!set) {
		memcpy(buf, ioctl_buf, len);
	}
	return ret;
}

int
AirPort_Brcm43xx_Dhd::wlIovarGetInt(const char *name, int *arg) const
{
	int ret;
	int iovar_len;
	uint buflen = sizeof(ioctl_buf);
	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	iovar_len = bcm_mkiovar((char *)name, NULL, 0, ioctl_buf, buflen);
	if (iovar_len == 0) {
		DHD_ERROR(("%s: insufficient buffer space passed to bcm_mkiovar for name %s\n", __func__, name));
		ASSERT(0);
	}

	ret = dhd_wl_ioctl_cmd(bus->dhd, WLC_GET_VAR, ioctl_buf, buflen, false, 0);
	if (ret < 0) {
		DHD_ERROR(("%s dhd_wl_ioctl_cmd failed. cmd %d, ret %d\n",
			__func__, WLC_GET_VAR, ret));
	} else {
		memcpy(arg, ioctl_buf, sizeof(int));
	}

	return ret;
}

int
AirPort_Brcm43xx_Dhd::wlIovarGetInt(const char *name, int *arg, OSObject *iface)
{
	int ret;
	int iovar_len;
	uint buflen = sizeof(ioctl_buf);
	int ifidx = 0;

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	iovar_len = bcm_mkiovar((char *)name, NULL, 0, ioctl_buf, buflen);
	if (iovar_len == 0) {
		DHD_ERROR(("%s: insufficient buffer space passed to bcm_mkiovar for name %s\n", __func__, name));
		ASSERT(0);
	}

	if (iface) {
		ifidx = iface_list.Iface2Idx(iface);
		//if no interface is found, use the default zero
		ifidx = (ifidx >= 0) ? ifidx : 0;
	}

	ret = dhd_wl_ioctl_cmd(bus->dhd, WLC_GET_VAR, ioctl_buf, buflen, false, ifidx);

	if (ret < 0) {
		DHD_ERROR(("%s dhd_wl_ioctl_cmd failed. cmd %d, ret %d, ifidx = %d\n",
			__func__, WLC_GET_VAR, ret, ifidx));
	} else {
		memcpy(arg, ioctl_buf, sizeof(int));
	}

	return ret;
}

int
AirPort_Brcm43xx_Dhd::wlIoctl(uint cmd, void *arg, size_t len, bool set, OSObject *iface)
{
	int ret = -1;
	dhd_ioctl_t *ioc = NULL;
	void *internal_buf = NULL;
	uint buflen = 0;
	int ifidx = 0;

	/*
	* Differentiate between dhd and wl ioctls
	*/
	if (DHD_IOCTL_MAGIC == cmd && arg) {
		ioc = (dhd_ioctl_t *)arg;

		if (ioc->buf && ioc->len > 0) {
			buflen = MIN(ioc->len, DHD_IOCTL_MAXLEN);
			internal_buf = MALLOC(osh, buflen);
			if (!internal_buf) {
				DHD_ERROR(("%s: out of memory !\n", __FUNCTION__));
				return BCME_NOMEM;
			}

			ret = copyin((user_addr_t)ioc->buf, internal_buf, buflen);
			if (ret) {
				DHD_ERROR(("%s: copy in error !\n", __FUNCTION__));
				MFREE(osh, internal_buf, buflen);
				return ret;
			}
		}

		ret = dhd_ioctl(bus->dhd, ioc, internal_buf, buflen);
		if (ret < 0) {
			DHD_ERROR(("%s: dhd_ioctl failed. cmd %d, ret %d\n",
			__func__, ioc->cmd, ret));
			if (internal_buf) {
				MFREE(osh, internal_buf, buflen);
			}
			return ret;
		}

		if (internal_buf) {
			ret = copyout(internal_buf, (user_addr_t)ioc->buf, buflen);
			MFREE(osh, internal_buf, buflen);
		}
	}
	else {
		if (iface) {
			ifidx = iface_list.Iface2Idx(iface);
			//if no interface is found, use the default zero
			ifidx = (ifidx >= 0) ? ifidx : 0;
		}
		ret = dhd_wl_ioctl_cmd(bus->dhd, cmd, arg, len, set, ifidx);
		if (ret < 0) {
			DHD_ERROR(("%s: dhd_wl_ioctl_cmd failed. cmd %d, ret %d, ifidx = %d\n",
				__func__, cmd, ret, ifidx));
		}
	}

	return ret;
}

int
AirPort_Brcm43xx_Dhd::wlIoctlGet(uint cmd, void *arg, size_t len, OSObject *iface) const
{
	int err;
	int ifidx = 0;

	if (iface) {
		ifidx = iface_list.Iface2Idx(iface);
		//if no interface is found, use the default zero idx
		ifidx = (ifidx >= 0) ? ifidx : 0;
	}

	err = dhd_wl_ioctl_cmd(bus->dhd, cmd, arg, len, false, ifidx);
	if (err) {
		DHD_ERROR(("%s: called with cmd %d returned error %d on ifidx %d\n",
			__func__, cmd, err, ifidx));
	}

	return err;
}

bool
AirPort_Brcm43xx_Dhd::enableInterrupt()
{
	int index  = 0;
	int msi_index = -1;
	IOWorkLoop *driverWorkLoop;
	IOReturn ret = kIOReturnSuccess;

	ASSERT(pciDev);

	if (!(driverWorkLoop = getWorkLoop())) {
		DHD_ERROR(("getWorkLoop returns NULL\n"));
		return false;
	}
#ifdef DHD_ENABLE_MSI_SUPPORT
	for (index = 0; ; index++) {
		int interruptType = 0;
		ret = pciDev->getInterruptType(index, &interruptType);
		if (ret != kIOReturnSuccess )
			break;
		WL_INFORM(("index %d flags 0x%x\n", index, interruptType));
		if (interruptType & kIOInterruptTypePCIMessaged)
			msi_index = index;
	}
#endif /* DHD_ENABLE_MSI_SUPPORT */

	/* Store the interrupt index of the device for future interrupt Management */
	interrupt_index = index;

	/* Workloop to handle interrupts from the dongle */
	intrWorkLoop = IOWorkLoop::workLoop();
#ifdef DHD_ENABLE_MSI_SUPPORT
	intrEventSrcMSI = IOInterruptEventSource::interruptEventSource(this, interruptEntryPoint, pciDev, msi_index);
#else
	/* Register with default interrupt provided by pciDev */
	intrEventSrcMSI = IOInterruptEventSource::interruptEventSource(this, interruptEntryPoint, pciDev);
#endif /* DHD_ENABLE_MSI_SUPPORT */
	if (intrEventSrcMSI && intrWorkLoop->addEventSource(intrEventSrcMSI) == kIOReturnSuccess) {
		intrEventSrcMSI->enable();
		WL_INFORM(("MSI interrupt enabled\n"));
	}

	intrEventSrcMSI_DWL = IOInterruptEventSource::interruptEventSource(this, interruptDWL);
	if (intrEventSrcMSI_DWL && driverWorkLoop->addEventSource(intrEventSrcMSI_DWL) == kIOReturnSuccess) {
		intrEventSrcMSI_DWL->enable();
		WL_INFORM(("Secondary MSI Driver WorkLoop interrupt enabled\n"));
	}

	return true;
}

void
AirPort_Brcm43xx_Dhd::interruptEntryPoint(OSObject *driver, IOInterruptEventSource *src, int count)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)driver;
	DHD_TRACE(("########## Interrupt #############\n"));
	pobj->interruptEntryHandler(src, count);
}

void
AirPort_Brcm43xx_Dhd::interruptDWL(OSObject *driver, IOInterruptEventSource *src, int count)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)driver;
	int ifidx = 0;

	void *pchain = NULL;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface, toss packet\n", __func__));
		return;
	}
	osl_spin_lock(pobj->workLoopSpinlock);
	if (pobj->rx_pkt) {
		pchain = pobj->rx_pkt;
		pobj->rx_pkt = pobj->rx_pkt_tail = NULL;
	}

	ifidx = pobj->rx_ifidx;

	osl_spin_unlock(pobj->workLoopSpinlock);
	if (pchain) {
		void *pnext;
		do {
			pnext = PKTNEXT(pobj->osh, pchain);
			PKTSETNEXT(pobj->osh, pchain, NULL);
			dhd_sendup((struct wl_info *)pobj, ifidx, pchain, 1);
		} while ((pchain = pnext));
	}
}

void
AirPort_Brcm43xx_Dhd::interruptEntryHandler(IOInterruptEventSource *src, int count)
{
	dhdpcie_bus_isr(bus);
}

bool
AirPort_Brcm43xx_Dhd::chipMatch()
{
	uint16 vendorID = pciDev->configRead16(kIOPCIConfigVendorID);
	uint16 deviceID = pciDev->configRead16(kIOPCIConfigDeviceID);
	uint32	bar0 = pciDev->configRead32(kIOPCIConfigBaseAddress0);
	DHD_ERROR(("Vendor ID: 0x%x, Device ID: 0x%x, BAR0 0x%x\n", vendorID, deviceID, bar0));

	if (vendorID != VENDOR_BROADCOM) {
		DHD_ERROR(("Vendor ID 0x%x is not valid\n", vendorID));
		return false;
	}

	if (deviceID == 0x0 || deviceID == 0xFFFF) {
		DHD_ERROR(("Device ID 0x%x is not valid\n", deviceID));
		return false;
	}

	// Initialize PCI configuration
	pciDev->configWrite32(kIOPCIConfigCommand,
		kIOPCICommandMemorySpace | kIOPCICommandBusMaster | kIOPCICommandMemWrInvalidate);

	// Map PCI register space
	ioMapBAR0 = pciDev->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0);
	ioMapBAR1 = pciDev->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress2);

	WL_INFORM(("ioMapBAR0: %p, ioMapBAR1: %p\n", ioMapBAR0, ioMapBAR1));

	if (!ioMapBAR0 || !ioMapBAR1) {
		DHD_ERROR(("No Map PCI register space\n"));
		return false;
	}
	void *bar0Regs = (void *)ioMapBAR0->getVirtualAddress();
	void *bar1Regs = (void *)ioMapBAR1->getVirtualAddress();
	IOByteCount bar0Size = ioMapBAR0->getLength();
	IOByteCount bar1Size = ioMapBAR1->getLength();

	WL_ERROR(("BAR0 Regs: %p, BAR0 Size: %d, BAR1 Regs: %p, BAR1 Size: %d\n",
		bar0Regs, bar0Size, bar1Regs, bar1Size));

	/* Set bar0 window to si_enum_base */
	pciDev->configWrite32(PCI_BAR0_WIN, si_enum_base(deviceID));
	bus = dhdpcie_bus_attach(osh, (volatile char*)bar0Regs, (volatile char*)bar1Regs, pciDev);
	if (!bus) {
		DHD_ERROR(("No bus allocated\n"));
		return false;
	}
	bus->tcm = (char *)bar1Regs;

	bus->dhd->info = (struct dhd_info*)this;
#ifdef DHD_FW_COREDUMP
	/* enable socram dump */
	bus->dhd->memdump_enabled = DUMP_MEMFILE;
#endif
	enableInterrupt();

	// Download firmware
	int ret;
	ret = dhd_bus_download_firmware(bus, osh, pfw_path, pnv_path);
	if (ret) {
		return false;
	}

	ret = dhd_prot_attach(bus->dhd);
	if (ret) {
		DHD_ERROR(("%s: protocol failed to attach, return %d, busstate %d\n", __func__,
			ret, bus->dhd->busstate));
		ASSERT(0);
		return false;
	}

	ret = dhd_bus_init(bus->dhd, false);
	if (ret) {
		DHD_ERROR(("%s: bus failed to initialize, return %d, busstate %d\n", __func__,
			ret, bus->dhd->busstate));
		return false;
	}

	int num_flowrings = dhd_bus_max_h2d_queues(bus);
	ret = dhd_flow_rings_init(bus->dhd, num_flowrings);
	if (ret) {
		ASSERT(0);
	}

	dhd_prot_init(bus->dhd);

	char eventmask[WL_EVENTING_MASK_LEN];
	/* Read event_msgs mask */
	wlIovarOp("event_msgs", NULL, 0, eventmask, WL_EVENTING_MASK_LEN, FALSE, NULL);

	/* Setup event_msgs */
	setbit(eventmask, WLC_E_SET_SSID);
	setbit(eventmask, WLC_E_PRUNE);
	setbit(eventmask, WLC_E_AUTH);
	setbit(eventmask, WLC_E_ASSOC);
	setbit(eventmask, WLC_E_REASSOC);
	setbit(eventmask, WLC_E_REASSOC_IND);
	setbit(eventmask, WLC_E_DEAUTH);
	setbit(eventmask, WLC_E_DEAUTH_IND);
	setbit(eventmask, WLC_E_DISASSOC_IND);
	setbit(eventmask, WLC_E_DISASSOC);
	setbit(eventmask, WLC_E_JOIN);
	setbit(eventmask, WLC_E_START);
	setbit(eventmask, WLC_E_ASSOC_IND);
	setbit(eventmask, WLC_E_PSK_SUP);
	setbit(eventmask, WLC_E_LINK);
	setbit(eventmask, WLC_E_NDIS_LINK);
	setbit(eventmask, WLC_E_MIC_ERROR);
	setbit(eventmask, WLC_E_ASSOC_REQ_IE);
	setbit(eventmask, WLC_E_ASSOC_RESP_IE);
	setbit(eventmask, WLC_E_PMKID_CACHE);
	setbit(eventmask, WLC_E_TXFAIL);
	setbit(eventmask, WLC_E_JOIN_START);
	setbit(eventmask, WLC_E_SCAN_COMPLETE);
	setbit(eventmask, WLC_E_HTSFSYNC);
	setbit(eventmask, WLC_E_PFN_NET_FOUND);
	setbit(eventmask, WLC_E_ROAM);
	setbit(eventmask, WLC_E_BSSID);
	setbit(eventmask, WLC_E_ADDTS_IND);
	setbit(eventmask, WLC_E_DELTS_IND);
	setbit(eventmask, WLC_E_TDLS_PEER_EVENT);
	setbit(eventmask, WLC_E_ESCAN_RESULT);
	setbit(eventmask, WLC_E_ACTION_FRAME_RX);
	setbit(eventmask, WLC_E_P2P_DISC_LISTEN_COMPLETE);
	setbit(eventmask, WLC_E_ACTION_FRAME_RX);

	clrbit(eventmask, WLC_E_PROBREQ_MSG);

	/* Write updated Event mask */
	wlIovarOp("event_msgs", NULL, 0, eventmask, WL_EVENTING_MASK_LEN, TRUE, NULL);

	/* Download CLM */
	if ((ret = dhd_apply_default_clm(bus->dhd, pclm_path)) < 0) {
		DHD_ERROR(("%s: Warning: CLM download failed\n", __FUNCTION__));
	}
	return true;
}

int
AirPort_Brcm43xx_Dhd::eventq_attach()
{

	eventq = (eventq_t*)MALLOC(osh, sizeof(*eventq));
	if (eventq == NULL) {
		DHD_ERROR(("%s: MALLOC(%d) failed, malloced %d bytes",
					__FUNCTION__, (int)sizeof(*eventq), MALLOCED(osh)));
		return BCME_NOMEM;
	}
	memset(eventq, 0, sizeof(*eventq));

	if (!(eventq->timer = wl_init_timer((struct wl_info*)this, event_dispatch, this,
			"eventq"))) {
		DHD_ERROR(("%s: eventq_attach: timer failed\n", __func__));
		MFREE(osh, eventq, sizeof(*eventq));
		return -BCME_ERROR;
	}

	return 0;
}

void
AirPort_Brcm43xx_Dhd::eventq_detach()
{
	dhd_event_t *e;
	if (eventq->timer) {
		if (eventq->tpending) {
			wl_del_timer((struct wl_info *)this, eventq->timer);
			eventq->tpending = FALSE;
		}
		wl_free_timer((struct wl_info*)this, eventq->timer);
		eventq->timer = NULL;
	}

	// this function gets called.
	/* Clean up pending events */
	while((e = eventq_deq())) {
		event_free(e);
	}

	MFREE(osh, eventq, sizeof(*eventq));
	eventq = NULL;
	return;
}

dhd_event_t*
AirPort_Brcm43xx_Dhd::event_alloc(wl_event_msg_t *msg, void *data)
{
	dhd_event_t *e;

	e = (dhd_event_t *)MALLOC(osh, sizeof(*e));
	if (e == NULL) {
		DHD_ERROR(("%s: MALLOC(%d) failed, malloced %d bytes",
					__FUNCTION__, (int)sizeof(*e), MALLOCED(osh)));
		return NULL;
	}

	e->next = NULL;
	memcpy( &e->event, msg, sizeof(*msg));
	if (msg->datalen > 0) {
		e->data = (void *)MALLOC(osh, msg->datalen);
		if (e->data  == NULL) {
			DHD_ERROR(("%s: MALLOC(%d) failed, malloced %d bytes",
						__FUNCTION__, msg->datalen, MALLOCED(osh)));
			return NULL;
		}
		memcpy(e->data, data, msg->datalen);
	} else {
		e->data = NULL;
	}

	return e;
}

void
AirPort_Brcm43xx_Dhd::event_free(dhd_event_t *e)
{
	ASSERT(e->next == NULL);
	if (e->data) {
		MFREE(osh, e->data, e->event.datalen);
	}
	MFREE(osh, e, sizeof(*e));
}

void
AirPort_Brcm43xx_Dhd::eventq_enq(dhd_event_t *e)
{
	ASSERT(e->next == NULL);
	e->next = NULL;

	IOTakeLock(eventLock);
	if (eventq->tail) {
		eventq->tail->next = e;
		eventq->tail = e;
	} else {
		eventq->head = eventq->tail = e;
	}

	if (!eventq->tpending) {
		eventq->tpending = TRUE;
		/* Use a zero-delay timer to trigger
		 * delayed processing of the event.
		 */
		wl_add_timer((struct wl_info *)this, eventq->timer, 0, 0);
	}
	IOUnlock(eventLock);
}

dhd_event_t*
AirPort_Brcm43xx_Dhd::eventq_deq()
{
	dhd_event_t *e;

	IOTakeLock(eventLock);
	e = eventq->head;
	if (e) {
		eventq->head = e->next;
		e->next = NULL;

		if (eventq->head == NULL) {
			eventq->tail = eventq->head;
		}
	}
	IOUnlock(eventLock);
	return e;
}

int
AirPort_Brcm43xx_Dhd::enqueueEvent(int ifidx, wl_event_msg_t *e, void *event_data)
{
	dhd_event_t *evt;
	evt = event_alloc(e, event_data);
	if (!evt) {
		DHD_ERROR(("%s: failed to allocate buffer for event\n", __func__));
		return BCME_NOMEM;
	}
	eventq_enq(evt);
	return BCME_OK;
}

int
AirPort_Brcm43xx_Dhd::dispatchEvent()
{
	dhd_event_t *e;
	IOTakeLock(eventLock);
	eventq->tpending = FALSE;
	IOUnlock(eventLock);
	while((e = eventq_deq())) {
		wlEvent(e->event.ifname, &e->event, e->data);
		event_free(e);
	}
	return 0;
}

void
AirPort_Brcm43xx_Dhd::macosWatchdog()
{
	dhd_bus_watchdog(bus->dhd);
}

bool
AirPort_Brcm43xx_Dhd::takeProtLock()
{
	ASSERT(dhdProtLock);
	if (dhdProtLock) {
		IOTakeLock(dhdProtLock);
		return true;
	}
	return false;
}

bool
AirPort_Brcm43xx_Dhd::releaseProtLock()
{
	ASSERT(dhdProtLock);
	if (dhdProtLock) {
		IOUnlock(dhdProtLock);
		return true;
	}
	return false;
}

void
AirPort_Brcm43xx_Dhd::interruptDriverWL()
{
	intrEventSrcMSI_DWL->interruptOccurred(this, pciDev, 0);
}

void
AirPort_Brcm43xx_Dhd::wlEvent( char *ifname, wl_event_msg_t *evt, void *data )
{
	uint event_type = evt->event_type;
	int err = 0;
	struct wl_event_data_if *ifevent = NULL;
	IO80211LinkState link_state = kIO80211NetworkLinkDown;

	//let the base class handle the events first
	super::wlEvent(ifname, evt, data);

	//events not handled in the base class/need-more-handling shall be handled here
	switch (event_type) {
		case WLC_E_IF:
			ifevent = (struct wl_event_data_if *)data;
			break;

		case WLC_E_LINK:
			link_state = ( evt->flags & WLC_EVENT_MSG_LINK) ? kIO80211NetworkLinkUp : kIO80211NetworkLinkDown;
			break;

		default:
			break;
	}
}

void
AirPort_Brcm43xx_Dhd::inputPacketQueued(mbuf_t m, int ifidx, int16 rssi)
{
	OSObject *iface = NULL;

	iface = iface_list.Idx2Iface(ifidx);
	if (!iface)
		return;

}


// disableDeviceInterrupt returns BCME_OK on success and BCME_ERROR on failure
int
AirPort_Brcm43xx_Dhd::disableDeviceInterrupt(void)
{
	IOReturn ioret;
	int ret = BCME_ERROR;

	/*
	 * AirPort_Brcm43xx_Dhd has the object IOPCIDevice that inherits IOService.
	 * The methods to disable, enable the interrupts is part of IOService.
	 * So we can invoke them using pobj->pciDev
	 * Also the interrupt index to operate upon is also a member of the DHD
	 */
	if (pciDev) {
		ioret = pciDev->disableInterrupt(interrupt_index);
		if (ioret == kIOReturnSuccess) {
			ret = BCME_OK;
			DHD_INFO(("%s: Disabled the Interrupt \r\n", __func__));
		} else {
			ret = BCME_ERROR;
			DHD_ERROR(("%s: Unable to Disable the Interrupt err %d \r\n", __func__, ioret));
		} /* ioret != kIOReturnSuccess */
	} else {
		DHD_ERROR(("%s: Invalid pciDev \r\n", __func__));
	} /* pcieDev == NULL */

	return ret;
}

// enableDeviceInterrupt returns BCME_OK on success and BCME_ERROR on failure
int
AirPort_Brcm43xx_Dhd::enableDeviceInterrupt(void)
{
	IOReturn ioret;
	int ret = BCME_ERROR;

	/*
	 * AirPort_Brcm43xx_Dhd has the object IOPCIDevice that inherits IOService.
	 * The methods to disable, enable the interrupts is part of IOService.
	 * So we can invoke them using pobj->pciDev
	 * Also the interrupt index to operate upon is also a member of the DHD
	 */
	if (pciDev) {
		ioret = pciDev->enableInterrupt(interrupt_index);
		if (ioret == kIOReturnSuccess) {
			ret = BCME_OK;
			DHD_INFO(("%s: Enabled the Interrupt \r\n", __func__));
		} else {
			ret = BCME_ERROR;
			DHD_ERROR(("%s: Unable to Enable the Interrupt err %d \r\n", __func__, ioret));
		} /* ioret != kIOReturnSuccess */
	} else {
		DHD_ERROR(("%s: Invalid pciDev \r\n", __func__));
	} /* pcieDev == NULL */

	return ret;
}



extern "C" {
int
dhd_os_proto_block(dhd_pub_t *pub)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)pub->info;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		return 0;
	}
	return pobj->takeProtLock();
}

int
dhd_os_proto_unblock(dhd_pub_t *pub)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)pub->info;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		return 0;
	}
	return pobj->releaseProtLock();
}

unsigned long
dhd_os_general_spin_lock(dhd_pub_t *pub)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)pub->info;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		return 0;
	}
	osl_spin_lock(pobj->getOSSpinLock());
	return 0;
}

void
dhd_os_general_spin_unlock(dhd_pub_t *pub, unsigned long flags)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)pub->info;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		return;
	}
	osl_spin_unlock(pobj->getOSSpinLock());
}

void
dhd_rx_event(dhd_pub_t *pub, int ifidx, void *pkt)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)pub->info;
	void *pktdata;
	wl_event_msg_t event;
	void *event_data = NULL;

	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		ASSERT(0);
		goto toss;
	}
	pktdata = PKTDATA(pub->osh, pkt);
	wl_host_event(pub, &ifidx, pktdata, 0xFFFF, &event, &event_data, NULL);
	wl_event_to_host_order(&event);
	if (event.event_type < WLC_E_LAST) {
		pobj->enqueueEvent(ifidx, &event, event_data);
	}

toss:
	PKTFREE(pub->osh, pkt, TRUE);

}

void
dhd_rx_frame(dhd_pub_t *dhdp, int ifidx, void *pktbuf, int numpkt, uint8 chan)
{
	int i;
	void *pnext = NULL;
	void *pinit = pktbuf;
	void *pprev = pktbuf;

	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)dhdp->info;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		return;
	}

	// TODO: send up all packets (the link list) to apple 80211 layer.

	for (i = 0; pktbuf && i < numpkt; i++, pktbuf = pnext) {
		struct ether_header *eh;

		pnext = PKTNEXT(dhdp->osh, pktbuf);
		eh = (struct ether_header *)PKTDATA(dhdp->osh, pktbuf);
		if (ntoh16(eh->ether_type) == ETHER_TYPE_BRCM) {
			if (pinit == pktbuf) {
				pinit = pnext;
			}
			PKTSETNEXT(dhdp->osh, pktbuf, NULL);
			if (pprev != pktbuf) {
				PKTSETNEXT(dhdp->osh, pprev, pnext);
			}
			dhd_rx_event(dhdp, ifidx, pktbuf);
		} else {
				pprev = pktbuf;
		}
	}

	if (pinit) {
		osl_spin_lock(pobj->workLoopSpinlock);
		if (pobj->rx_pkt) {
			PKTSETNEXT(dhdp->osh, pobj->rx_pkt_tail, pinit);
		} else {
			pobj->rx_pkt = pinit;
		}
		pobj->rx_pkt_tail = pprev;
		pobj->rx_ifidx = ifidx;
		osl_spin_unlock(pobj->workLoopSpinlock);
		pobj->interruptDriverWL();
	}

}

void dhd_sendup(struct wl_info *drv, int ifidx, void *m, int numpkt)
{
	AirPort_Brcm43xx_Dhd *wlobj = (AirPort_Brcm43xx_Dhd*)drv;
	struct ether_header *eh = NULL;

	ASSERT(m);

	if (!ifidx){
		wl_sendup(drv, NULL, m, numpkt);
		return;
	}

	if (m)
		m = PKTTONATIVE(wlobj->osh, m);
	if (m) {
		if (PKTLEN(wlobj->osh, m) < ETHER_HDR_LEN) {
			IOLog("%s: Packet Length too short (len %d). Drop Packet\n",
				__FUNCTION__, PKTLEN(wlobj->osh, m));
			PKTFREE(wlobj->osh, m, FALSE);
		} else {
			wlobj->inputPacketQueued((mbuf_t)m, ifidx, 0);
		}
	}
}

int dhd_get_ifidx(struct dhd_info *dhd_info, char *name)
{
	AirPort_Brcm43xx_Dhd *dhd_airport_obj = (AirPort_Brcm43xx_Dhd*)dhd_info;

	return dhd_airport_obj->iface_list.Wlname2Idx(name);
}

/* API to Disable the Device Interrupt, returns 0 - Success, -1 - Failure */
int
dhd_macos_disable_irq(dhd_pub_t *dhdp)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)dhdp->info;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		return BCME_ERROR;
	}

	return pobj->disableDeviceInterrupt();
}

/* API to Enable the Device Interrupt, returns 0 - Success, -1 - Failure */
int
dhd_macos_enable_irq(dhd_pub_t *dhdp)
{
	AirPort_Brcm43xx_Dhd *pobj = (AirPort_Brcm43xx_Dhd*)dhdp->info;
	if (!pobj) {
		DHD_ERROR(("%s: No Airport interface\n", __func__));
		return BCME_ERROR;
	}

	return pobj->enableDeviceInterrupt();
}

void
dhd_os_dhdiovar_lock(dhd_pub_t *pub)
{
	return;
}

void
dhd_os_dhdiovar_unlock(dhd_pub_t *pub)
{
	return;
}

} /* extern "C" */
