# -*-tcl-*-
#
# $Copyright Broadcom Corporation$
# $Id$
#

######
#
# UTF Ramsey5 in SW Lab 5010
#

# Shared resources
source utfconf/ramsey_shared.tcl

set ::UTF::SummaryDir "/projects/hnd_software/utf/ramsey5"

R5 configure -default 10

set UTF::SetupTestBed {
    R5 attn default
    foreach S {43342a0o 4334b3o} {
	catch {$S unload}
	$S deinit
    }
}

# Relay
UTF::Linux UTFTestS -power {UTFPower6 4}

#####################
# 1st small Ramsey box

UTF::Linux UTFTestQ -sta {4360 enp1s0} \
    -tag BISON05T_BRANCH_7_35 \
    -console "UTFTestS:40001" -power {UTFPower6 7} -reloadoncrash 1 \
    -wlinitcmds {wl msglevel +assoc;wl dtim 3;wl bw_cap 2g -1}

4360 configure -ipaddr 192.168.1.50 -attngrp R5

#####################
# 2nd small Ramsey box

package require UTF::HSIC

UTF::HSIC UTFTestR -sta {4334b3h eth1} \
    -relay UTFTestF \
    -console "UTFTestF:40000" \
    -rteconsole "UTFTestF:40002" \
    -power {UTFPower6 3} \
    -nvram "src/shared/nvram/bcm94334OlympicCent_usi_st.txt" \
    -nvram_add "htagc=1" \
    -tag "PHOENIX2_BRANCH_6_10" \
    -type "4334b1-roml/usb-ag-p2p-pno-nocis-keepalive-aoe-oob-idsup-idauth-err-assert-wapi-srsusp-ve-awdl-ndoe-pf2-proptxstatus-wlfcts-cca-pwrstats-wnm-noclminc-clm_4334_centennial" \
    -brand "linux-external-dongle-usb" -customer "olympic" \
    -host_brand "linux26-internal-hsic-phoenix2" \
    -host_tag "RTRFALCON_REL_5_130_??" \
    -host_nvram {
	lan_gateway=10.19.7.1
	lan_stp=0
	lan1_stp=0
	watchdog=6000
    } \
    -postinstall {dhd hsicautosleep 1} \
    -wlinitcmds {wl mimo_bw_cap 1} \
    -nocal 1 -noframeburst 1 -slowassoc 5 \
    -datarate {-i 2} \
    -yart {-attn5g 27-95 -attn2g 36-95 -pad 3}

4334b3h configure -ipaddr 192.168.1.98

4334b3h clone 4334b3h.1 -sta {4334b3h.1 wl0.1} \
    -apmode 1 -nochannels 1 -perfchans 3 \
    -wlinitcmds {wl mimo_bw_cap 1; wl apsta 1; wl ssid -C 1 4334AP} \
    -noaes 1 -notkip 1 -nopm1 1 -nobighammer 1 -yart {}

4334b3h clone 4334b3hx -type "Innsbruck/centennial.trx" \
    -yart {} -perfonly 1 -noaes 1 -perfchans 36l

4334b3hx clone 4334b3ox -tag PHO2203RC1_BRANCH_6_25 -type "Okemo/4334b1/centennial.trx"

4334b3h clone 4334b3o -tag PHO2203RC1_BRANCH_6_25 \
    -type "4334b1-roml/usb-ag-p2p-pno-nocis-keepalive-aoe-oob-idsup-idauth-err-assert-wapi-srsusp-ve-awdl-proptxstatus-cca-noclminc/rtecdc.bin.trx" \
    -datarate {-skiptx 32}
4334b3h.1 clone 4334b3o.1 -tag PHO2203RC1_BRANCH_6_25 \
    -type "4334b1-roml/usb-ag-p2p-pno-nocis-keepalive-aoe-oob-idsup-idauth-err-assert-wapi-srsusp-ve-awdl-proptxstatus-cca-noclminc/rtecdc.bin.trx"

4334b3o clone 4334b3o_nohtagc -nvram_add  "htagc=0"

4334b3o clone 4334b3mfg \
    -brand "linux-mfgtest-dongle-usb" -customer bcm \
    -type "4334b1-roml/usb-ag-mfgtest-nodis-swdiv-oob-seqcmds-srsusp-idsup"

4334b3o clone 4334b3o65 -tag PHO2203RC1_TWIG_6_25_65
4334b3ox clone 4334b3o65x -tag PHO2203RC1_TWIG_6_25_65

# 3rd small ramsey box

UTF::HSIC UTFTestG -sta {43342a0o eth1} \
    -relay UTFTestF \
    -console "UTFTestF:40004" \
    -rteconsole "UTFTestF:40001" \
    -power {UTFPower6 1} \
    -nvram "src/shared/nvram/bcm943342ChardonnayMurataKK.txt" \
    -nvram_add "htagc=1" \
    -tag "PHO2203RC1_BRANCH_6_25" \
    -type "43342a0-roml/usb-ag-p2p-pno-nocis-keepalive-aoe-oob-idsup-err-assert-srsusp-ve-awdl-ndoe-pf2-proptxstatus-cca-pwrstats-wnm-noclminc-clm_43342_olympic-mpf" \
    -brand "linux-external-dongle-usb" -customer "olympic" \
    -host_brand "linux26-internal-hsic-phoenix2" \
    -host_tag "RTRFALCON_REL_5_130_??" \
    -host_nvram {
	lan_gateway=10.19.7.1
	lan_stp=0
	lan1_stp=0
	watchdog=6000
    } \
    -postinstall {dhd hsicautosleep 1} \
    -wlinitcmds {wl mimo_bw_cap 1} \
    -nocal 1 -noframeburst 1 -slowassoc 5 \
    -datarate {-skiptx 32} \
    -yart {-attn5g 24-95 -attn2g 39-95 -pad 3}

43342a0o configure -ipaddr 192.168.1.97

43342a0o clone 43342a0o.1 -sta {43342a0o.1 wl0.1} \
    -apmode 1 -nochannels 1 -slowassoc 5 -perfchans 3 \
    -wlinitcmds {wl mimo_bw_cap 1; wl apsta 1; wl ssid -C 1 43342AP} \
    -noaes 1 -notkip 1 -nopm1 1 -nobighammer 1 -yart {}

43342a0o clone 43342a0ox -type "Okemo/43342a0/chardonnay.trx" \
    -noaes 1 -notkip 1

43342a0ox clone 43342a0ox.1 -sta {43342a0ox.1 wl0.1} \
    -apmode 1 -nochannels 1 -slowassoc 5 -perfchans 3 \
    -wlinitcmds {wl mimo_bw_cap 1; wl apsta 1; wl ssid -C 1 43342AP} \
    -noaes 1 -notkip 1 -nopm1 1 -nobighammer 1 -yart {}

43342a0o clone 43342a0o105 -tag PHO2203RC1_TWIG_6_25_105
43342a0ox clone 43342a0o105x -tag PHO2203RC1_TWIG_6_25_105

#####
UTF::Q ramsey5 UTFTestC
