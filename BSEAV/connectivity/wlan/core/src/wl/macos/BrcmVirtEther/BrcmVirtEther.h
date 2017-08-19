/*
 * Mac OS X Virtual ethernet driver header file to go with
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
 * <<Broadcom-WL-IPTag/Private1743:>>
 *
 * $Id$
 */

#ifndef _BRCMVIRTETHER_H_
#define _BRCMVIRTETHER_H_
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOOutputQueue.h>
#include <IOKit/apple80211/apple80211_var.h>
#include <IOKit/apple80211/apple80211_ioctl.h>
#include <IOKit/IOService.h>
#include <libkern/version.h>

#include "wlioctl.h"

//===========================================================================
// BrcmVirtEtherInt class
//===========================================================================

class BrcmVirtEtherInt : public IOEthernetInterface
{
	OSDeclareDefaultStructors(BrcmVirtEtherInt)

public:
	virtual SInt32	performCommand( IONetworkController	* controller,
	                                unsigned long cmd,
	                                void * arg0,
	                                void * arg1 );
};


class BrcmVirtEther : public IOEthernetController
{
	OSDeclareDefaultStructors(BrcmVirtEther);

protected:
	IOEthernetAddress myaddrs;
	BrcmVirtEtherInt	*m_etherIf;
	bool			m_etherIfEnab;
	IOEthernetController *wlhdl;
	uint index;

	IONetworkStats		*m_netStats;
	IOEthernetStats		*m_etherStats;
	IOOutputAction _outputAction;
	IONetworkController::Action _ioctlAction;

	bool publishEtherMedium(void);
	bool netifDetach();
	bool netifAttach();
	IONetworkInterface * createInterface();
	IOReturn operateBSS(bool up);

public:
	IOEthernetInterface *netifAttach(IOEthernetController *hdl, IOEthernetAddress *addrs,
	                                 IOOutputAction _outAction, IONetworkController::Action _ioctlAction,
	                                 uint unit);
	bool netifDetach(IOEthernetInterface *intf);

	virtual bool start(IOService *provider);
	virtual void stop(IOService *provider);

	/*
	 * IOEthernetController methods - must implement these
	 */
	virtual IOReturn enable(IONetworkInterface *netif);
	virtual IOReturn disable(IONetworkInterface *netif);
	virtual IOReturn disable();
	virtual bool configureInterface(IONetworkInterface *netif);
	virtual IOReturn getPacketFilters(const OSSymbol *group, UInt32 *filters) const;
	virtual IOReturn getHardwareAddress(IOEthernetAddress *addr);

        /*
	 * IOEthernetController methods - helper functions
	 */
	virtual IOReturn setMulticastMode(IOEnetMulticastMode mode);
	virtual IOReturn setMulticastList(IOEthernetAddress *addrs, UInt32 count);
	virtual IOReturn setWakeOnMagicPacket(bool active);
	virtual IOReturn selectMedium(const IONetworkMedium *medium);
	virtual IOReturn setPromiscuousMode(bool active);
	virtual IOReturn setHardwareAddress(const IOEthernetAddress *addr);

	UInt32 outputPacket( mbuf_t m, void *param);
	SInt32 handleIoctl(struct apple80211req *req, bool userioc = 1);

};

#endif /* _BRCMVIRTETHER_H_ */
