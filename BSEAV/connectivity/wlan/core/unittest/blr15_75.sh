Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 3 -p2p_chan 3 -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=11u P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 11u -p2p_chan 11u -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=3 P2PCh=11' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 3 -p2p_chan 11 -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=6u P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 6u -p2p_chan 11u -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=3 P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 3 -p2p_chan 11u -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36 -p2p_chan 36 -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36l P2PCh=36l' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36l -p2p_chan 36l -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36 P2PCh=149' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36 -p2p_chan 149 -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=48u P2PCh=36l' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 48u -p2p_chan 36l -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36l' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36 -p2p_chan 36l -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36/80 -p2p_chan 36/80 -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36/80 -p2p_chan 161/80 -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 3 -p2p_chan 36/80 -nrate_check "1x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test RSDB APCh=11u P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod-run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 11u -p2p_chan 36/80 -nrate_check "1x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36 -p2p_chan 36/80 -nrate_check "2x9"
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 48u -p2p_chan 36/80 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36/80 -p2p_chan 3 -nrate_check "1x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test RSDB APCh=36/80 P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36/80 -p2p_chan 11u -nrate_check "1x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=36' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36/80 -p2p_chan 36 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 TCP bi-dir Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36/80 -p2p_chan 48u -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 3 -p2p_chan 3 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=11u P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 11u -p2p_chan 11u -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=11' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 3 -p2p_chan 11 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=6u P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 6u -p2p_chan 11u -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 3 -p2p_chan 11u -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=36' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36 -p2p_chan 36 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36l P2PCh=36l' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36l -p2p_chan 36l -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=149' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36 -p2p_chan 149 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=48u P2PCh=36l' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 48u -p2p_chan 36l -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=36l' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36 -p2p_chan 36l -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nom -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36/80 -p2p_chan 36/80 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 80M -p2p_bandwidth_VI 80M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36/80 -p2p_chan 161/80 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 3 -p2p_chan 36/80 -nrate_check "1x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test RSDB APCh=11u P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 11u -p2p_chan 36/80 -nrate_check "1x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36 -p2p_chan 36/80 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 48u -p2p_chan 36/80 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36/80 -p2p_chan 3 -nrate_check "1x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test RSDB APCh=36/80 P2PCh=11u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36/80 -p2p_chan 11u -nrate_check "1x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=36' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36/80 -p2p_chan 36 -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 GC PCIe DIN07T48RC50_BRANCH_9_75 UDP Video bi-dir Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36/80 -p2p_chan 48u -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 PCIe DIN07T48RC50_BRANCH_9_75 Direct P2P Video bi-dir Long Test P2PCh=36/80' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 36/80

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 PCIe DIN07T48RC50_BRANCH_9_75 Direct P2P Video bi-dir Long Test P2PCh=3' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 3

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 PCIe DIN07T48RC50_BRANCH_9_75 WLAN QoS' -ap '4360softap' -sta '4359b1-FC19-DUT75' -wlan_security aespsk2 -ap_chan 36l -email hnd-utf-list -multicore_mode 1 -scan_flag  -wlan_bandwidth_BE 3M -wlan_bandwidth_BK 30M -wlan_bandwidth_VI 40M -wlan_bandwidth_VO 40M -run_qos -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 PCIe DIN07T48RC50_BRANCH_9_75 P2P QoS' -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -multicore_mode 1 -scan_flag  -wlan_security aespsk2 -p2p_security -ap_chan 36 -p2p_chan 36/80 -p2p_bandwidth_BE 30M -p2p_bandwidth_BK 30M -p2p_bandwidth_VI 30M -p2p_bandwidth_VO 30M -run_qos -qos_p2p_traffic -nrate_check "2x9"

sleep 4
Test/P2PQoSNightly.test -title "blr15 4359 B1 9_35 P2P QoS" -ap 4360softap -sta_gc 4359b1-FC19-DUT75 -sta_go 4359b1-FC19-GO75 -wlan_security aespsk2 -nod -nos -ap_chan 36/80 -p2p_chan 161/80 -p2p_bandwidth_BE 30M -p2p_bandwidth_BK 30M -p2p_bandwidth_VI 30M -p2p_bandwidth_VO 30M -run_qos -qos_p2p_traffic -multicore_mode 1 -scan_flag -nrate_check 2x9 -email hnd-utf-list 
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN TCP SCC RvR DINGO_BRANCH_9_slna Same APGC' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN TCP SCC RvR DINGO_BRANCH_9_slna Same APGO' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN Video SCC RvR DINGO_BRANCH_9_slna Same APGC' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN Video SCC RvR DINGO_BRANCH_9_slna Same APGO' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN TCP VSDB RvR DINGO_BRANCH_9_slna Same APGC' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN TCP VSDB RvR DINGO_BRANCH_9_slna Same APGO' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN Video VSDB RvR DINGO_BRANCH_9_slna Same APGC' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN Video VSDB RvR DINGO_BRANCH_9_slna Same APGO' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN TCP RSDB RvR DINGO_BRANCH_9_slna Same APGC' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN TCP RSDB RvR DINGO_BRANCH_9_slna Same APGO' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN Video RSDB RvR DINGO_BRANCH_9_slna Same APGC' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 WLAN Video RSDB RvR DINGO_BRANCH_9_slna Same APGO' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 1
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P TCP SCC RvR DINGO_BRANCH_9_slna Same APGC APCHAN 36/80 P2PCHAN=36/80' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P TCP SCC RvR DINGO_BRANCH_9_slna Same APGO APCHAN 36/80 P2PCHAN=36/80' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P Video SCC RvR DINGO_BRANCH_9_slna Same APGC APCHAN 36/80 P2PCHAN=36/80' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P Video SCC RvR DINGO_BRANCH_9_slna Same APGO APCHAN 36/80 P2PCHAN=36/80' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P TCP VSDB RvR DINGO_BRANCH_9_slna Same APGC APCHAN 36/80 P2PCHAN=161/80' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P TCP VSDB RvR DINGO_BRANCH_9_slna Same APGO APCHAN 36/80 P2PCHAN=161/80' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P Video VSDB RvR DINGO_BRANCH_9_slna Same APGC APCHAN 36/80 P2PCHAN=161/80' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P Video VSDB RvR DINGO_BRANCH_9_slna Same APGO APCHAN 36/80 P2PCHAN=161/80' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P TCP RSDB RvR DINGO_BRANCH_9_slna Same APGC APCHAN 36/80 P2PCHAN=3' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P TCP RSDB RvR DINGO_BRANCH_9_slna Same APGO APCHAN 36/80 P2PCHAN=3' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P Video RSDB RvR DINGO_BRANCH_9_slna Same APGC APCHAN 36/80 P2PCHAN=3' -ap_connect GC -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 2
sleep 4
Test/P2PQoSNightly.test -title 'blr15 4359b1 P2P Video RSDB RvR DINGO_BRANCH_9_slna Same APGO APCHAN 36/80 P2PCHAN=3' -ap_connect GO -ap '4360softap' -sta_gc '4359b1-FC19-DUT75' -sta_go '4359b1-FC19-GO75' -email hnd-utf-list -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos  -qos_tests '[P2P:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 3 -fb1 -attn_type 2
sleep 4
