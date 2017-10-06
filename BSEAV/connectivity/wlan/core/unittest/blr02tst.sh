Test/P2PQoSNightly.test -title "blr02 - BLR 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=36/80" -ap 4360softap -sta_gc 4359b1-FC19-DUT75 -sta_go 4359b1-FC19-GO75 -email hnd-utf-list -multicore_mode 0  -nom -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests [WLAN:VI:BI:0:300][P2P:VI:BI:0:300] -ap_chan 36/80 -p2p_chan 36/80 -nrate_check 2x9 -nostaload
sleep 4
Test/P2PQoSNightly.test -title "blr02 - BLR 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test RSDB APCh=11u P2PCh=36/80" -ap 4360softap -sta_gc 4359b1-FC19-DUT75 -sta_go 4359b1-FC19-GO75 -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests [WLAN:VI:BI:0:300][P2P:VI:BI:0:300] -ap_chan 11u -p2p_chan 36/80 -nrate_check 1x9 -nostaload
sleep 4 
Test/P2PQoSNightly.test -title "blr02 - BLR 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test RSDB APCh=36/80 P2PCh=11u" -ap 4360softap -sta_gc 4359b1-FC19-DUT75 -sta_go 4359b1-FC19-GO75 -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests [WLAN:VI:BI:0:300][P2P:VI:BI:0:300] -ap_chan 36/80 -p2p_chan 11u -nrate_check 1x9 -nostaload
sleep 4 
Test/P2PQoSNightly.test -title 'blr02 - BLR 4359b1 P2P Video VSDB RvR DINGO_BRANCH_9_slna Same APGO APCHAN 36/80 P2PCHAN=161/80' -ap_connect GO -ap 4360softap -sta_gc 4359b1-FC19-DUT75 -sta_go 4359b1-FC19-GO75 -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests [P2P:VI:BI:0:5][P2P:VI:BI:0:5] -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2 -nostaload
sleep 4
Test/P2PQoSNightly.test -title "blr02 - BLR 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test RSDB APCh=36/80 P2PCh=3" -ap 4360softap -sta_gc 4359b1-FC19-DUT75 -sta_go 4359b1-FC19-GO75 -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests [WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300] -ap_chan 36/80 -p2p_chan 3 -nrate_check 1x9 -ap_connect GC -nostaload
sleep 4
Test/P2PQoSNightly.test -title "blr02 - BLR 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36/80" -ap 4360softap -sta_gc 4359b1-FC19-DUT75 -sta_go 4359b1-FC19-GO75 -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests [WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300] -ap_chan 36 -p2p_chan 36/80 -nrate_check 2x9 -ap_connect GC -nostaload
sleep 4
Test/P2PQoSNightly.test -title "blr02 - BLR 4359b1 PCIe DIN07T48RC50_BRANCH_9_75 WLAN QoS" -ap 4360softap -sta 4359b1-FC19-DUT75 -wlan_security aespsk2 -ap_chan 36l -email hnd-utf-list -multicore_mode 0  -wlan_bandwidth_BE 3M -wlan_bandwidth_BK 30M -wlan_bandwidth_VI 40M -wlan_bandwidth_VO 40M -run_qos -nrate_check 2x9 -nostaload
sleep 4

