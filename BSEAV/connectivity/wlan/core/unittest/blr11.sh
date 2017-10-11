#Test/StaNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Max Throughput StaNightly' -sta '4355b2-GC' -ap '4360'  -email hnd-utf-list -awdl
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
#Test/StaNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Max Throughput GO StaNightly' -sta '4355b2-GO' -ap '4360'  -email hnd-utf-list -awdl
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Max Throughput TCP WLAN Only' -sta '4355b2-GC' -ap '4360' -ap_chan 36/80 -fb1  -email hnd-utf-list -awdl
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Max Throughput UDP WLAN Only' -sta '4355b2-GC' -ap '4360' -ap_chan 36/80  -email hnd-utf-list -awdl -fb1 -wlan_bandwidth_BE 1.2g -run_qos -qos_tests '[WLAN:BE:RX:0:25]\|[WLAN:BE:TX:0:25]\|[WLAN:BE:BI:0:25]'
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Max Throughput GO TCP WLAN Only' -sta '4355b2-GO' -ap '4360' -ap_chan 36/80 -fb1  -email hnd-utf-list -awdl
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Max Throughput GO UDP WLAN Only' -sta '4355b2-GO' -ap '4360' -ap_chan 36/80 -fb1  -email hnd-utf-list -awdl -wlan_bandwidth_BE 1.2g -run_qos -qos_tests '[WLAN:BE:RX:0:25]\|[WLAN:BE:TX:0:25]\|[WLAN:BE:BI:0:25]'
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4

#Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 5G Video Roaming' -sta '4355b2-GC' -roam_src_ap 'AP1-4331-476 AP1-4360-476' -roam_dst_ap 'AP3-4331-476 AP3-4360-476'  -email hnd-utf-list -awdl -no2G4roaming -no2G2roaming -no5G2roaming -sniffer 'snif' -run_qos -qos_tests '[WLAN:VI:RX:0:120]' -roam_5G8_src_ap_attn '14-31' -roam_5G8_dst_ap_attn '24-' -roam_5G8_sniffer_start_attn 26 -roam_5G4_src_ap_attn '16-33' -roam_5G4_dst_ap_attn '25-' -roam_5G4_sniffer_start_attn 28 
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 2G Video Roaming' -sta '4355b2-GC' -roam_src_ap 'AP1-4331-476 AP1-4360-476' -roam_dst_ap 'AP3-4331-476 AP3-4360-476'  -email hnd-utf-list -awdl -no5G4roaming -no5G8roaming -no5G2roaming -no2G4roaming -sniffer 'snif' -run_qos -qos_tests '[WLAN:VI:RX:0:120]' -roam_2G2_src_ap_attn "25-55" -roam_2G2_dst_ap_attn "22-" -roam_2G2_sniffer_start_attn 34 -wlan_bandwidth_VI 1M 
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP Roaming' -sta '4355b2-GC' -roam_src_ap 'AP1-4331-476 AP1-4360-476' -roam_dst_ap 'AP3-4331-476 AP3-4360-476'  -email hnd-utf-list -awdl -no2G4roaming -no5G2roaming -sniffer 'snif' -run_qos -qos_tests '[WLAN:TCP:RX:0:120]' -roam_5G8_src_ap_attn '14-31' -roam_5G8_dst_ap_attn '24-' -roam_5G8_sniffer_start_attn 26 -roam_5G4_src_ap_attn '16-33' -roam_5G4_dst_ap_attn '25-' -roam_5G4_sniffer_start_attn 28 -roam_2G2_src_ap_attn '3-5' -roam_2G2_dst_ap_attn '22-' -roam_2G2_sniffer_start_attn 35
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
#sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b2b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 3 -p2p_chan 3
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=11u P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 11u -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 3 -p2p_chan 11
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=6u P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 6u -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=3 P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 3 -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36 -p2p_chan 36
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36l -p2p_chan 36l
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36 -p2p_chan 149
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 48u -p2p_chan 36l
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36 -p2p_chan 36l
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 161/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 3 -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test RSDB APCh=11u P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 11u -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36 -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 48u -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 3
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test RSDB APCh=36/80 P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 48u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 3
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=11u P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 11u -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 11
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=6u P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 6u -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 36
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36l -p2p_chan 36l
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 149
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 48u -p2p_chan 36l
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 36l
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 161/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test RSDB APCh=11u P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 11u -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 48u -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 3
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test RSDB APCh=36/80 P2PCh=11u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 11u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 36
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 48u
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Direct P2P Video bi-dir Long Test P2PCh=36/80' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 36/80
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 Direct P2P Video bi-dir Long Test P2PCh=3' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 3
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 UDP Video bi-dir Long VSDB/RSDB Test APCh=36/80 P2PCh=7' -ap '4360' -sta_gc '4349-FC19-DUTx' -sta_go '4349-FC19-GOx'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:TX:0:120][WLAN:VI:TX:0:120]' -ap_chan 36/80 -p2p_chan 7 -rsdb_switch_test -p2p_connection_first
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 WLAN QoS' -ap '4360' -sta '4355b2-GC' -wlan_security aespsk2 -ap_chan 36l  -email hnd-utf-list -awdl -wlan_bandwidth_BE 3M -wlan_bandwidth_BK 30M -wlan_bandwidth_VI 40M -wlan_bandwidth_VO 40M -run_qos
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 4355b2 PCIe DINGO_BRANCH_9_10 P2P QoS' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -wlan_security aespsk2 -p2p_security -ap_chan 36 -p2p_chan 36/80 -p2p_bandwidth_BE 30M -p2p_bandwidth_BK 30M -p2p_bandwidth_VI 30M -p2p_bandwidth_VO 30M -run_qos -qos_p2p_traffic
#./UTF.tcl 4355b2-GC power cycle
#./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 WLAN TCP RvR Same APGC' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 WLAN TCP RvR Same APGO' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 WLAN Video RvR Same APGC' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 WLAN Video RvR Same APGO' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 P2P TCP RvR Same APGC' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 P2P TCP RvR Same APGO' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 P2P Video RvR Same APGC' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 4355b2 P2P Video RvR Same APGO' -ap '4360' -sta_gc '4355b2-GC' -sta_go '4355b2-GO'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
# sleep 4
# Test/RvRNightly1.test -ap '4360' -sta '4355b2-GC' -va af -cycle5G40AttnRange '1-7 7-1'  -email hnd-utf-list -awdl -cycle5G20AttnRange '1-7 7-1' -cycle2G40AttnRange '1-7 7-1' -cycle2G20ttnRange '1-7 7-1' -nocontrvr -no5G20 -no2G40 -title 'RvR Test' -perftime 2.5 -awdl
# #./UTF.tcl 4355b2-GC power cycle
# #./UTF.tcl 4355b2-GO power cycle
sleep 4
Test/StaNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Max Throughput StaNightly' -sta '43909-GC-b120' -ap '4360'  -email hnd-utf-list -awdl -perfonly
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
#Test/StaNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Max Throughput GO StaNightly' -sta '43909-GO-b120' -ap '4360'  -email hnd-utf-list -awdl
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Max Throughput TCP WLAN Only' -sta '43909-GC-b120' -ap '4360' -ap_chan 36/80 -fb1  -email hnd-utf-list -awdl
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Max Throughput UDP WLAN Only' -sta '43909-GC-b120' -ap '4360' -ap_chan 36/80  -email hnd-utf-list -awdl -fb1 -wlan_bandwidth_BE 1.2g -run_qos -qos_tests '[WLAN:BE:RX:0:25]\|[WLAN:BE:TX:0:25]\|[WLAN:BE:BI:0:25]'
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Max Throughput GO TCP WLAN Only' -sta '43909-GO-b120' -ap '4360' -ap_chan 36/80 -fb1  -email hnd-utf-list -awdl
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Max Throughput GO UDP WLAN Only' -sta '43909-GO-b120' -ap '4360' -ap_chan 36/80 -fb1  -email hnd-utf-list -awdl -wlan_bandwidth_BE 1.2g -run_qos -qos_tests '[WLAN:BE:RX:0:25]\|[WLAN:BE:TX:0:25]\|[WLAN:BE:BI:0:25]'
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4

#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 5G Video Roaming' -sta '43909-GC-b120' -roam_src_ap 'AP1-4331-476 AP1-4360-476' -roam_dst_ap 'AP3-4331-476 AP3-4360-476'  -email hnd-utf-list -awdl -no2G4roaming -no2G2roaming -no5G2roaming -sniffer 'snif' -run_qos -qos_tests '[WLAN:VI:RX:0:120]' -roam_5G8_src_ap_attn '14-31' -roam_5G8_dst_ap_attn '24-' -roam_5G8_sniffer_start_attn 26 -roam_5G4_src_ap_attn '16-33' -roam_5G4_dst_ap_attn '25-' -roam_5G4_sniffer_start_attn 28 
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 2G Video Roaming' -sta '43909-GC-b120' -roam_src_ap 'AP1-4331-476 AP1-4360-476' -roam_dst_ap 'AP3-4331-476 AP3-4360-476'  -email hnd-utf-list -awdl -no5G4roaming -no5G8roaming -no5G2roaming -no2G4roaming -sniffer 'snif' -run_qos -qos_tests '[WLAN:VI:RX:0:120]' -roam_2G2_src_ap_attn "25-55" -roam_2G2_dst_ap_attn "22-" -roam_2G2_sniffer_start_attn 34 -wlan_bandwidth_VI 1M 
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP Roaming' -sta '43909-GC-b120' -roam_src_ap 'AP1-4331-476 AP1-4360-476' -roam_dst_ap 'AP3-4331-476 AP3-4360-476'  -email hnd-utf-list -awdl -no2G4roaming -no5G2roaming -sniffer 'snif' -run_qos -qos_tests '[WLAN:TCP:RX:0:120]' -roam_5G8_src_ap_attn '14-31' -roam_5G8_dst_ap_attn '24-' -roam_5G8_sniffer_start_attn 26 -roam_5G4_src_ap_attn '16-33' -roam_5G4_dst_ap_attn '25-' -roam_5G4_sniffer_start_attn 28 -roam_2G2_src_ap_attn '3-5' -roam_2G2_dst_ap_attn '22-' -roam_2G2_sniffer_start_attn 35
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
#sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=3 P2PCh=3' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 3 -p2p_chan 3
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=11u P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 11u -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=3 P2PCh=11' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 3 -p2p_chan 11
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=6u P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 6u -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=3 P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 3 -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test  APCh=36 P2PCh=36' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36 -p2p_chan 36
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test  APCh=36l P2PCh=36l' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36l -p2p_chan 36l
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=36 P2PCh=149' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36 -p2p_chan 149
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=48u P2PCh=36l' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 48u -p2p_chan 36l
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test  APCh=36 P2PCh=36l' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36 -p2p_chan 36l
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test  APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36/80 -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36/80 -p2p_chan 161/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 3 -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=11u P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 11u -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36 -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 48u -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36/80 -p2p_chan 3
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=36/80 P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36/80 -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36/80 -p2p_chan 36
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 TCP bi-dir Long Test APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:120][P2P:TCP:BI:0:120]' -ap_chan 36/80 -p2p_chan 48u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=3 P2PCh=3' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 3
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=11u P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 11u -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=3 P2PCh=11' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 11
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=6u P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 6u -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=3 P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36 P2PCh=36' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 36
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36l P2PCh=36l' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36l -p2p_chan 36l
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36 P2PCh=149' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 149
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=48u P2PCh=36l' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 48u -p2p_chan 36l
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36 P2PCh=36l' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 36l
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 161/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 3 -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=11u P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 11u -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36 -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 48u -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 3
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36/80 P2PCh=11u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 11u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 36
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long Test APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:120][P2P:VI:BI:0:120]' -ap_chan 36/80 -p2p_chan 48u
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Direct P2P Video bi-dir Long Test P2PCh=36/80' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:120]' -p2p_chan 36/80
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 Direct P2P Video bi-dir Long Test P2PCh=3' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:120]' -p2p_chan 3
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
#Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 UDP Video bi-dir Long VSDB/Test APCh=36/80 P2PCh=7' -ap '4360' -sta_gc '4349-FC19-DUTx' -sta_go '4349-FC19-GOx'  -email hnd-utf-list -awdl -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:TX:0:120][WLAN:VI:TX:0:120]' -ap_chan 36/80 -p2p_chan 7 -rsdb_switch_test -p2p_connection_first
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 WLAN QoS' -ap '4360' -sta '43909-GC-b120' -wlan_security aespsk2 -ap_chan 36l  -email hnd-utf-list -awdl -wlan_bandwidth_BE 3M -wlan_bandwidth_BK 30M -wlan_bandwidth_VI 40M -wlan_bandwidth_VO 40M -run_qos
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43909 SDIO BIS120RC4_TWIG_7_15_168 P2P QoS' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -wlan_security aespsk2 -p2p_security -ap_chan 36 -p2p_chan 36/80 -p2p_bandwidth_BE 30M -p2p_bandwidth_BK 30M -p2p_bandwidth_VI 30M -p2p_bandwidth_VO 30M -run_qos -qos_p2p_traffic
#./UTF.tcl 43909-GC-b120 power cycle
#./UTF.tcl 43909-GO-b120 power cycle
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 WLAN TCP RvR Same APGC' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 WLAN TCP RvR Same APGO' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 WLAN Video RvR Same APGC' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 WLAN Video RvR Same APGO' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 P2P TCP RvR Same APGC' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 P2P TCP RvR Same APGO' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:TCP:BI:0:30][P2P:TCP:BI:0:30]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 P2P Video RvR Same APGC' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43909 P2P Video RvR Same APGO' -ap '4360' -sta_gc '43909-GC-b120' -sta_go '43909-GO-b120'  -email hnd-utf-list -awdl -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:1][P2P:VI:BI:0:1]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
# #Test/RvRNightly1.test -ap '4360' -sta '43909-GC-b120' -va af -cycle5G40AttnRange '1-7 7-1'  -email hnd-utf-list -awdl -cycle5G20AttnRange '1-7 7-1' -cycle2G40AttnRange '1-7 7-1' -cycle2G20ttnRange '1-7 7-1' -nocontrvr -no5G20 -no2G40 -title 'RvR Test' -perftime 2.5 -awdl
# #./UTF.tcl 43909-GC-b120 power cycle
# #./UTF.tcl 43909-GO-b120 power cycle
# sleep 4
