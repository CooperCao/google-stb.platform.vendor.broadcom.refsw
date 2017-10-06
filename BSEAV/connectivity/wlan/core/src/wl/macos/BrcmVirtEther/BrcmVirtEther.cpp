/*
 * Mac OS X Virtual ethernet driver to go with
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
 * $Id: BrcmVirtEther.cpp,v 1.12 2009-09-29 21:15:15 $
 */

#include <IOKit/IOLib.h>
#include "BrcmVirtEther.h"
extern "C" {
#include <pexpert/pexpert.h>
}

#undef  super
#define super IOEthernetController
OSDefineMetaClassAndStructors(BrcmVirtEther, IOEthernetController)

BrcmVirtEther *g_BrcmVirtEther = NULL;

bool
BrcmVirtEther::start(IOService *provider)
{
	bool res = super::start(provider);

	g_BrcmVirtEther = this;
	IOLog("BrcmVirtEther started\n");

	return res;
}

void  BrcmVirtEther::stop(IOService *provider)
{
	if (m_etherIf)
		netifDetach();

	super::stop(provider);
	IOLog("BrcmVirtEther stopped\n");
	g_BrcmVirtEther = NULL;
}

bool
BrcmVirtEther::netifAttach()
{
	if (m_etherIf)
		return false;

	if (!attachInterface((IONetworkInterface **)&m_etherIf, false)) {
		IOLog("BrcmVirtEther: Failed to attach Interface\n");
		return false;
	}

	m_etherIf->registerService();

	/* publish for ioctl user clients */
	registerService();

	return true;
}

IOEthernetInterface *
BrcmVirtEther::netifAttach(IOEthernetController *hdl, IOEthernetAddress *addrs, IOOutputAction _outAction,
                           IONetworkController::Action _iocAction, uint unit)
{
	bcopy(addrs, &myaddrs, 6);
	if (netifAttach()) {
		wlhdl = hdl;
		index = unit;
		_outputAction = _outAction;
		_ioctlAction = _iocAction;

		return m_etherIf;
	}
	return NULL;
}

bool
BrcmVirtEther::netifDetach()
{
	if (!m_etherIf)
		return false;
	detachInterface((IONetworkInterface *)m_etherIf);
	m_etherIf->release();
	m_etherIf = NULL;
	return true;
}

bool
BrcmVirtEther::netifDetach(IOEthernetInterface *intf)
{
	if (m_etherIf && m_etherIf == intf)
		netifDetach();

	return true;
}

IOReturn
BrcmVirtEther::setMulticastMode(IOEnetMulticastMode mode)
{
	struct apple80211req multi_req;
	struct apple80211req *req = &multi_req;
	char iovar_buf[WLC_IOCTL_SMLEN*2];
	bool multi = !(mode == false);
	uint iovar_len = strlen("allmulti") + 1;

	bzero(iovar_buf, sizeof(iovar_buf));
	memmove(&iovar_buf[iovar_len], &multi, sizeof(multi));
	strncpy(iovar_buf, "allmulti", strlen("allmulti"));

	bzero(req, sizeof (struct apple80211req));
#if VERSION_MAJOR > 9
	req->req_data = CAST_USER_ADDR_T(iovar_buf);
#else
	req->req_data = (void *)iovar_buf;
#endif
	req->req_type = APPLE80211_IOC_CARD_SPECIFIC;
	req->req_val = WLC_SET_VAR;
	req->req_len = iovar_len + sizeof(uint);

	return handleIoctl(req, 0);
}

IOReturn
BrcmVirtEther::setPromiscuousMode(bool active)
{
	struct apple80211req promisc_req;
	struct apple80211req *req = &promisc_req;
	char iovar_buf[WLC_IOCTL_SMLEN];

	bzero(iovar_buf, sizeof(iovar_buf));
	memmove(iovar_buf, &active, sizeof(active));

	bzero(req, sizeof (struct apple80211req));

	req->req_type = APPLE80211_IOC_CARD_SPECIFIC;
	req->req_val = WLC_SET_PROMISC;
	req->req_len = sizeof(uint);
#if VERSION_MAJOR > 9
	req->req_data = CAST_USER_ADDR_T(iovar_buf);
#else
	req->req_data = (void*)iovar_buf;
#endif

	return handleIoctl(req, 0);
}

IOReturn
BrcmVirtEther::setMulticastList(IOEthernetAddress *addrs, UInt32 count)
{
	struct apple80211req multi_req;
	struct apple80211req *req = &multi_req;
	struct maclist *list;
	uint iovar_len = strlen("mcast_list") + 1 + sizeof(struct maclist) +
	        count * ETHER_ADDR_LEN;
	char *iovar_buf;
	IOReturn ret;

	iovar_buf = (char *)IOMalloc(iovar_len);
	if (!iovar_buf)
		return ENOMEM;

	bzero(iovar_buf, sizeof(iovar_buf));
	strncpy(iovar_buf, "mcast_list", strlen("mcast_list"));

	list = (struct maclist *)&iovar_buf[strlen("mcast_list") + 1];
	list->count = 0;
	
	UInt32  uniqCnt = 0;
	for (UInt32 i = 0; i < count; ++i)
	{
		// search for dup's before adding to table (AppleTalk bug?)
		UInt32 j;
		for (j = 0; j < uniqCnt; ++j )
			if (bcmp( &addrs[i], &list->ea[j], ETHER_ADDR_LEN) == 0) {
				break;		// found a dup
			}
		if (j == uniqCnt)		// then this adrs is a new one
			bcopy( &addrs[i], &list->ea[uniqCnt++] , ETHER_ADDR_LEN);
	}
	list->count = uniqCnt;

	bzero(req, sizeof (struct apple80211req));

#if VERSION_MAJOR > 9
	req->req_data = CAST_USER_ADDR_T(iovar_buf);
#else
	req->req_data = (void *)iovar_buf;
#endif
	req->req_type = APPLE80211_IOC_CARD_SPECIFIC;
	req->req_val = WLC_SET_VAR;
	req->req_len = iovar_len;

	ret=handleIoctl(req, 0);
	IOFree(iovar_buf, iovar_len);
	return ret;
}

IOReturn
BrcmVirtEther::setWakeOnMagicPacket(bool active)
{
	return kIOReturnSuccess;
}

IOReturn
BrcmVirtEther::selectMedium(const IONetworkMedium *medium)
{
	return kIOReturnSuccess;
}

IOReturn
BrcmVirtEther::getPacketFilters(const OSSymbol *group, UInt32 *filters) const
{
	IOReturn rtn = kIOReturnSuccess;

	if (group == gIONetworkFilterGroup) {
		*filters = kIOPacketFilterUnicast | kIOPacketFilterBroadcast |
			kIOPacketFilterMulticast | kIOPacketFilterMulticastAll |
			kIOPacketFilterPromiscuous;
	} else {
		rtn = super::getPacketFilters(group, filters);
	}

	return rtn;
}


extern "C" {

IOEthernetInterface *
brcmVirtEther_create_interface(IOEthernetController *hdl, IOEthernetAddress *addrs, IOOutputAction _outAction,
                               IONetworkController::Action _ioctlAction, uint unit)
{
	if (g_BrcmVirtEther)
		return g_BrcmVirtEther->netifAttach(hdl, addrs, _outAction, _ioctlAction, unit);
	IOLog("brcmVirtEther_create_interface: Failed to create interface %p\n", g_BrcmVirtEther);
	return NULL;
}


bool
brcmVirtEther_destroy_interface(IOEthernetInterface *intf)
{
	if (g_BrcmVirtEther)
		return g_BrcmVirtEther->netifDetach(intf);
	return false;
}

}

IOReturn
BrcmVirtEther::operateBSS(bool up)
{
	char iovar_buf[WLC_IOCTL_SMLEN];
	struct apple80211req up_req;
	struct apple80211req *req = &up_req;

	uint iovar_len = strlen("bss") + 1;

	bzero(iovar_buf, sizeof(iovar_buf));
	memmove(&iovar_buf[iovar_len], &index, sizeof(index));
	memmove(&iovar_buf[iovar_len+sizeof(index)], &up, sizeof(up));
	strncpy(iovar_buf, "bss", strlen("bss"));

	bzero(req, sizeof (struct apple80211req));
#if VERSION_MAJOR > 9
	req->req_data = CAST_USER_ADDR_T(iovar_buf);
#else
	req->req_data = (void *)iovar_buf;
#endif
	req->req_type = APPLE80211_IOC_CARD_SPECIFIC;
	req->req_val = WLC_SET_VAR;
	req->req_len = iovar_len + 2*sizeof(uint);

	return handleIoctl(req, 0);
}

IOReturn
BrcmVirtEther::enable(IONetworkInterface *netif)
{

	IOLog("enable");
	if (m_etherIfEnab == true)
		return kIOReturnSuccess;

	if (publishEtherMedium() == false)
		return kIOReturnError;

	m_etherIfEnab = true;

	return operateBSS(1);
}

IOReturn
BrcmVirtEther::disable()
{
	if (m_etherIf)
		return disable(m_etherIf);
	return kIOReturnSuccess;
}

IOReturn
BrcmVirtEther::disable(IONetworkInterface *netif)
{
	if (m_etherIfEnab == false)
		return kIOReturnSuccess;

	m_etherIfEnab = false;

	setLinkStatus(0, 0);
	return operateBSS(0);
}

bool
BrcmVirtEther::configureInterface(IONetworkInterface *netif)
{
	if (super::configureInterface(netif) == false)
		return false;

	/* Get network statistics objects */
	IONetworkParameter *param = netif->getParameter(kIONetworkStatsKey);
	if (!param || !(m_netStats = (IONetworkStats *) param->getBuffer()))
		return false;

	param = netif->getParameter(kIOEthernetStatsKey);
	if (!param || !(m_etherStats = (IOEthernetStats *) param->getBuffer()))
		return false;

	m_netStats->outputPackets = 0;
	m_netStats->outputErrors = 0;
	m_netStats->inputPackets = 0;
	m_netStats->inputErrors = 0;

	return true;
}

bool
BrcmVirtEther::publishEtherMedium()
{
	OSDictionary *mediumDict = 0;
	IONetworkMedium	*medium;

	mediumDict = OSDictionary::withCapacity(1);
	if (!mediumDict)
		return false;

	medium = IONetworkMedium::medium(kIOMediumEthernetAuto, 0);
	IONetworkMedium::addMedium(mediumDict, medium);
	medium->release();

	if (publishMediumDictionary(mediumDict) != true)
		return false;

	medium = IONetworkMedium::getMediumWithType(mediumDict, kIOMediumEthernetAuto);
	setCurrentMedium(medium);
	setLinkStatus(kIONetworkLinkActive | kIONetworkLinkValid, medium, 100 * 1000000);

	mediumDict->release();
	mediumDict = 0;
	return true;
}

IOReturn
BrcmVirtEther::getHardwareAddress(IOEthernetAddress *addr)
{
	bcopy(&myaddrs, addr,6);
	return kIOReturnSuccess;
}

IOReturn
BrcmVirtEther::setHardwareAddress(const IOEthernetAddress * addr)
{
	struct apple80211req hard_req;
	struct apple80211req *req = &hard_req;
	char iovar_buf[WLC_IOCTL_SMLEN*2];
	uint iovar_len = strlen("cur_etheraddr") + 1;

	bzero(iovar_buf, sizeof(iovar_buf));
	memmove(&iovar_buf[iovar_len], addr, sizeof(IOEthernetAddress));
	strncpy(iovar_buf, "cur_etheraddr", strlen("cur_etheraddr"));

	bzero(req, sizeof (struct apple80211req));
#if VERSION_MAJOR > 9
	req->req_data = CAST_USER_ADDR_T(iovar_buf);
#else
	req->req_data = (void *)iovar_buf;
#endif
	req->req_type = APPLE80211_IOC_CARD_SPECIFIC;
	req->req_val = WLC_SET_VAR;
	req->req_len = iovar_len + sizeof(IOEthernetAddress);

	return handleIoctl(req, 0);

}

UInt32
BrcmVirtEther::outputPacket( mbuf_t m, void *param)
{
	if (_outputAction)
		return (((OSObject*)wlhdl)->*(this->_outputAction))(m, param);
	else {
		freePacket(m);
		return kIOReturnOutputDropped;
	}
}

IONetworkInterface *
BrcmVirtEther::createInterface()
{
	BrcmVirtEtherInt *newif = new BrcmVirtEtherInt;

	if (newif && (newif->init(this) == false)) {
		newif->release();
		newif = 0;
	}
	return newif;
}

SInt32
BrcmVirtEther::handleIoctl(struct apple80211req *req, bool userioc)
{
	SInt32 ret;
	ret = wlhdl->executeCommand(wlhdl, (this->_ioctlAction),
	                             wlhdl, /* target */
	                             (void *)req, /* param 0 */
	                             &userioc, NULL, NULL);
	return ret;
}
#pragma mark -
#pragma mark BrcmVirtEtherInt Implementation

//=============================================================================
// BrcmVirtEtherInt
//=============================================================================
#undef  super
#define super IOEthernetInterface
OSDefineMetaClassAndStructors( BrcmVirtEtherInt, IOEthernetInterface )

SInt32
BrcmVirtEtherInt::performCommand( IONetworkController * ctr,
                                  unsigned long cmd,
                                  void *                arg0,
                                  void *                arg1 )
{
	struct apple80211req *req = (struct apple80211req*)arg1;
	BrcmVirtEther *_ctr = (BrcmVirtEther *)ctr;
	
	if (req) {
		if (req->req_type == APPLE80211_IOC_CARD_SPECIFIC)
			return _ctr->handleIoctl(req);
	}
	return super::performCommand(ctr, cmd, arg0, arg1);
}
