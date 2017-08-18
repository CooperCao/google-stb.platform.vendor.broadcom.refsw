/*
 * +----------------------------------------------------------------------------
 * HND Forwarder between GMAC and "HW switching capable WLAN interfaces".
 *
 * Northstar router includes 3 GMAC ports from the integrated switch to the Host
 * (single or dual core) CPU. The integrated switch provides advanced ethernet
 * hardware switching functions, similar to that of a Linux software bridge.
 * In Northstar, this integrated HW switch is responsible for bridging packets
 * between the 4 LAN ports. All LAN ports are seen by the ethernet network
 * device driver as a single "vlan1" interface. This single vlan1 interface
 * represents the collection of physical LAN ports on the switch, without
 * having to create a seperate interface per LAN port and  adding each one of
 * these LAN interfaces to the default Linux software LAN bridge "br0".
 * The hardware switch allows the LAN ports to be segregated into multiple
 * bridges using VLAN (Independent VLAN Learning Mode). Again, each subset of
 * physical LAN ports are represented by a single interface, namely "vlanX".
 *
 * The 3 GMAC configuration treats the primary WLAN interface as just another
 * LAN interface (albeit with a WLAN 802.11 MAC as opposed to an Ethernet 802.3
 * MAC). Two of the three GMACs are dedicated for binding the primary WLAN
 * interfaces to the HW switch which performs the LAN to WLAN bridging function.
 * These two GMACs are referred to as Forwarding GMACs.
 * The third GMAC is used to connect the switch to the Linux router network
 * stack, by making all LAN and WLAN ports appear as a single vlan1 interface
 * that is added to the software Linux bridge "br0". This GMAC is referred to
 * as the Networking GMAC.
 *
 * Similar to LAN to WAN routing, where LAN originated packets would be flooded
 * to the WAN port via the br0, likewise, WLAN originated packets would re-enter
 * Linux network stack via the 3rd GMAC. Software Cut-Through-Forwarding CTF
 * will accelerate WLAN <-> WAN traffic. When the hardware Flow Accelerator is
 * enabled, WLAN <-> WAN traffic need not re-enter the host CPU, other than the
 * first few packets that are needed to establish the flows in the FA, post
 * DPI or Security related flow classification functions.
 *
 * +----------------------------------------------------------------------------
 *
 * A typical GMAC configuration is:
 *   GMAC#0 - port#5 - fwd0 <---> wl0/eth1 (radio 0) on CPU core0
 *   GMAC#1 - port#7 - fwd1 <---> wl1/eth2 (radio 1) on CPU core1
 *
 *   GMAC#2 - port#8 - eth0 <--- vlan1 ---> br0
 *
 * Here, analogous to all physical LAN ports are managed by vlan1, likewise the
 * port#5 and port#7 are also treated as "LAN" ports hosting a "WiFI" MAC.
 *
 * In such a configuration, the primary wl interfaces wl0 and wl1 (i.e. eth1 and
 * eth2) are not added to the Linux bridge br0, as the integrated HW switch will
 * handle all LAN <---> WLAN bridging functions. All LAN ports and the WLAN
 * interface are represented by vlan1. vlan1 is added to the Linux bridge br0.
 *
 * The switch port#8, is IMP port capable (Integrated Management Port). All
 * routed LAN and WAN traffic are directed to this port, where the eth0 network
 * interface is responsible for directing to the network stack. The eth0 network
 * interface uses the presence of a VLAN=1 to differentiate from WAN
 * originating packets.
 *
 * The same ethernet driver is instantiated per GMAC, and is flavored as a
 * Forwarder (GMAC#0 and GMAC#1) or a Network interface (GMAC#2). The Forwarder
 * GMAC(s) are not added to any Linux SW bridge, and are responsible to
 * receive/transmit packets from/to the integrated switch on behalf of the
 * HW switching capable WLAN interfaces.
 *
 * hndfwd.h provides the API for the GMAC Fwder device driver and wl driver
 * to register their respective transmit bypass handlers.
 *
 * An upstream forwarder object forwards traffic received from a GMAC to a
 * WLAN intreface for transmission. Likewise a downstream forwarder object
 * forwards traffic received from a WLAN interface to the GMAC. The upstream
 * and downstream forwarder, together act as a bi-directional socket.
 *
 * In a single core Northstar (47081), a pair of bidirectional forwarder objects
 * are maintained, indexed by the GMAC and WLAN radio fwder_unit number.
 *
 * In a dual core SMP Northstar, the bidirectional forwarder objects are defined
 * as per CPU instances. Each GMAC forwarder driver and mated WLAN interface are
 * tied to the appropriate CPU core using their respective fwder_unit number
 * and they access the per CPU core forwarder object by using the fwder_unit
 * number to retrieve the per CPU instance.
 *
 * In Atlas, where multiple radios may share the same IRQ, the two radios must
 * be hosted on the same CPU core. A "fwder_cpumap" nvram variable defines
 * the mode, channel, band, irq, and cpucore assignment, for all (3) radios.
 * The nvram variable is parsed and a unique fwder_unit number is assigned to
 * each radio such that radios that shared IRQ will use the same GMAC fwder.
 *
 * MultiBSS Radio:
 * Packets forwarded by the integrated switch only identify a port on which a
 * radio resides. In a Multi BSS configuration, a radio may host multipe virtual
 * interfaces that may avail of HW switching ("br0"). Each primary or virtual
 * interface registers an explicit Linux netdevice and WLAN interface in which
 * context the transmission occurs. In such a MBSS configuration, a packet
 * forwarded by a GMAC for WLAN transmission needs to first identify the virtual
 * interface, detected by the Ethernet MAC DA.
 *
 * +----------------------------------------------------------------------------
 *
 * WLAN Offload Forwarding Assist (WOFA)
 * HND Forwarder provides a Mac address to interface mapping table using an
 * abstraction of a dictionary of symbols. A symbol is a 6 Byte Ethernet MAC
 * address. A single dictionary is instantiated per GMAC forwarder pair.
 *
 * In this use case, the dictionary is managed by the WLAN driver. Symbols are
 * added or deleted on Association/Reassociation and Deassociation events
 * respectively, representing the address associated with the station.
 * When a symbol is added to the WOFA dictionary an opaque metadata is also
 * added. In the NIC mode, this metadata is the network device associated with
 * the WLAN interface. In the DGL mode, the metadata may encode the flow ring.
 * The metadata may be used to locate the wl_info/dhd_info, the interface and
 * any other information such as the queue associated with the flow ring.
 *
 * In the GMAC to WLAN direction, a lookup of the MAC DstAddress is performed
 * within wl_forward(bound bypass WLAN forwarding handler). The lookup returns
 * the WLAN interface (from which the registered Linux net_device and wl_info
 * object is derived).
 *
 * In the wl_sendup, path a lookup is also performed to determine whether the
 * packet has to be locally bridged. WOFA's bloom filter is used to quickly
 * terminate the lookup, and proceed with forwarding to the GMAC fwder.
 *
 * Broadcast packets would be flooded to all bound wl interfaces.
 *
 * If deassociation events may get dropped, a periodic synchronization of the
 * dictionary with the list of associated stations needs to be performed.
 *
 * WOFA's uses a 3 step lookup:
 * Stage 1. WOFA caches the most recent hit entry. If this entry matches, then
 *          the hash table is directly looked up.
 * Stage 2. A single hash based bloom filter is looked up. A negative filter
 *          avoids the need to further search the hash table.
 * Stage 3. A search of the hash table using a 4bin per bucket search is used.
 *
 * In devices where the source port number can be determined (BRCM tag), the
 * stage 1 lookup can benefit from back-to-back packets arriving for the same
 * Mac DstAddress.
 *
 * The bloom filter uses a simple multi word bitmap and a single hash function
 * for filtering. The bloom filter mwbmap assumes a 32bit architecture. As the
 * bloom filter can have false positives, an exact match by searching the hash
 * table is performed. The bloom-filter serves in quickly determining whether
 * a further exact match lookup is necessary. This is done to limit CPU cycles
 * for packets that are not destined to WOFA managed interfaces.
 *
 * +----------------------------------------------------------------------------
 *
 * Copyright (C) 2017, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id$
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * +----------------------------------------------------------------------------
 */

#ifndef _hndfwd_h_
#define _hndfwd_h_

/* Standalone WOFA (mac address dictionary) */
struct wofa;                    /* WOFA Address Resolution Logic */
#define WOFA_NOOP               do { /* noop */ } while (0)

#define WOFA_NULL               ((struct wofa *)NULL)
#define WOFA_FAILURE            (-1)
#define WOFA_SUCCESS            (0)

#define WOFA_DATA_INVALID        ((uintptr_t)(~0)) /* symbol to data mapping */


struct fwder;                   /* Forwarder object */
#define FWDER_NOOP              do { /* noop */ } while (0)

#define FWDER_NULL              (NULL)
#define FWDER_FAILURE           (-1)
#define FWDER_SUCCESS           (0)

#define FWDER_WOFA_INVALID		(WOFA_DATA_INVALID) /* uintptr_t ~0 */


/** Non GMAC3:  WOFA not supported - stubs */
#define wofa_init()             ({  (WOFA_NULL); })
#define wofa_fini(wofa)			WOFA_NOOP
#define wofa_add(wofa, symbol, data)                                           \
	({ BCM_REFERENCE(wofa); BCM_REFERENCE(symbol); BCM_REFERENCE(data);        \
	   (WOFA_FAILURE); })
#define wofa_del(wofa, symbol, data)                                           \
	({ BCM_REFERENCE(wofa); BCM_REFERENCE(symbol); BCM_REFERENCE(data);        \
	   (WOFA_FAILURE); })
#define wofa_clr(wofa, data)                                                   \
	({ BCM_REFERENCE(wofa); BCM_REFERENCE(data); (WOFA_FAILURE); })
#define wofa_lkup(wofa, symbol, port)                                          \
	({ BCM_REFERENCE(wofa); BCM_REFERENCE(symbol); BCM_REFERENCE(port);        \
	   (WOFA_DATA_INVALID); })


/** Non GMAC3:  FWDER not supported - stubs */
#define fwder_init()            FWDER_NOOP
#define fwder_exit()            FWDER_NOOP

#define fwder_attach(dir, unit, mode, bypass_fn, dev, osh)                     \
	({ BCM_REFERENCE(dir); BCM_REFERENCE(unit); BCM_REFERENCE(mode);           \
	   BCM_REFERENCE(bypass_fn); BCM_REFERENCE(dev); BCM_REFERENCE(osh);       \
	   (FWDER_NULL); })

#define fwder_dettach(fwder, dir, unit)                                        \
	({ BCM_REFERENCE(fwder); (FWDER_NULL); })

#define fwder_bind(dir, unit, subunit, dev, attach)                            \
	({ BCM_REFERENCE(dir); BCM_REFERENCE(unit); BCM_REFERENCE(subunit);        \
	   BCM_REFERENCE(attach); (FWDER_NULL); })

#define fwder_transmit(skbs, skb_cnt, rxdev, fwder)                            \
	({ BCM_REFERENCE(skbs); BCM_REFERENCE(skb_cnt); BCM_REFERENCE(rxdev);      \
	   BCM_REFERENCE(fwder); (FWDER_FAILURE); })

#define fwder_flood(fwder, pkt, osh, clone)                                    \
	({ BCM_REFERENCE(fwder); BCM_REFERENCE(pkt);                               \
	   BCM_REFERENCE(osh); BCM_REFERENCE(clone); })

#define fwder_fixup(fwder, pkt, len)                                           \
	({ BCM_REFERENCE(fwder); BCM_REFERENCE(pkt); BCM_REFERENCE(len)})

#define fwder_discard(fwder, pkt)                                              \
	({ BCM_REFERENCE(fwder); BCM_REFERENCE(pkt); })

#endif /* _hndfwd_h_ */
