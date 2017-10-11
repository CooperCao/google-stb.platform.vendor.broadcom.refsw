#
#  $Copyright Broadcom Corporation$
#  $Id$ 2014/08/22 19:45:00 sotmishi Exp $
#
#  Testbed configuration file for MC39s Testbed Sheida Otmishi
#

source utfconf/mc39.tcl
set ::UTF::SummaryDir "/projects/hnd_sig_ext12/sotmishi/mc39s"

set ::UTF::SetupTestBed {
    foreach s {43224AP 43228AP 43228 43224} {
	catch {$s wl down}
	$s deinit
    }
    foreach s {43224AP 43224} {
	catch {$s iptables -F}
	$s deinit
    }
    return
}

UTF::Linux mc39tst3 -sta {43224AP enp12s0 43228AP enp13s0} \
    -power  "power5 1" \
    -console  "mc39end1:40551" \
    -datarate {-i 0.5} -udp 250m -slowassoc 5 \
    -wlinitcmds {wl msglevel +assoc; wl dtim 3; wl bw_cap 2g -1}

43224AP configure -ipaddr 192.168.21.31
43228AP configure -ipaddr 192.168.21.32

43224AP clone 43224eAP -tag  "EAGLE_BRANCH_10_10"
43228AP clone 43228eAP -tag  "EAGLE_BRANCH_10_10"

UTF::Linux mc39tst4 -sta {43228 enp11s0 43224 enp13s0} \
    -power  "power5 2" \
    -console  "mc39end1:40552" \
    -datarate {-i 0.5} -udp 250m -slowassoc 5 \
    -wlinitcmds {wl msglevel +assoc; wl bw_cap 2g -1}

43228 configure -ipaddr 192.168.21.41
43224 configure -ipaddr 192.168.21.42

43228 clone 43228e -tag  "EAGLE_BRANCH_10_10"
43224 clone 43224e -tag  "EAGLE_BRANCH_10_10"

43228 clone 43228d -tag  "DINGO_BRANCH_9_10"
43224 clone 43224d -tag  "DINGO_BRANCH_9_10"

UTF::Q mc39s mc39end1
