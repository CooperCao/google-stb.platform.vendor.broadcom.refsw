Test/P2PQoSNightly.test -title 'blr11 43012 GC PCIe DINGO07T_BRANCH_9_35 TCP bi-dir Long Test VSDB APCh=5 P2PCh=9' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 5 -p2p_chan 9 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 TCP bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 3 -p2p_chan 3 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 TCP bi-dir Long Test VSDB APCh=1 P2PCh=11' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 1 -p2p_chan 11 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 TCP bi-dir Long Test VSDB APCh=11 P2PCh=1' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 11 -p2p_chan 1 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 TCP bi-dir Long Test VSDB APCh=6 P2PCh=8' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 6 -p2p_chan 8 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 3 -p2p_chan 3 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 UDP Video bi-dir Long Test VSDB APCh=1 P2PCh=11' -ap '43430a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 1 -p2p_chan 11 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 UDP Video bi-dir Long Test VSDB APCh=11 P2PCh=1' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 11 -p2p_chan 1 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 UDP Video bi-dir Long Test VSDB APCh=6 P2PCh=8' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 6 -p2p_chan 8 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 GC PCIe DINGO07T_BRANCH_9_35 UDP Video bi-dir Long Test VSDB APCh=5 P2PCh=9' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 5 -p2p_chan 9 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43430 PCIe DINGO07T_BRANCH_9_35 Direct P2P Video bi-dir Long Test P2PCh=3' -ap '4360a' -sta_gc '43430-GC' -sta_go '43430-GO' -email hnd-utf-list -multicore_mode 0  -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 3
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 3 -p2p_chan 3 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=11 P2PCh=11' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 11 -p2p_chan 11 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=3 P2PCh=11' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod  -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 3 -p2p_chan 11 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=6 P2PCh=11' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 6 -p2p_chan 11 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=5 P2PCh=8' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 5 -p2p_chan 8 -pm_mode 0
sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=36 P2PCh=149' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36 -p2p_chan 149 -pm_mode 0
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=48 P2PCh=36' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 48 -p2p_chan 36 -pm_mode 0
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=36 P2PCh=36' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36 -p2p_chan 36 -pm_mode 0
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 TCP bi-dir Long Test VSDB APCh=36 P2PCh=48' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -run_qos -qos_tests '[WLAN:TCP:BI:0:300][P2P:TCP:BI:0:300]' -ap_chan 36 -p2p_chan 48 -pm_mode 0

Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=3' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 3 -p2p_chan 3 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=3 P2PCh=11' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 3 -p2p_chan 11 -pm_mode 0

# Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=36' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36 -p2p_chan 36 -pm_mode 0
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=36 P2PCh=149' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 36 -p2p_chan 149 -pm_mode 0 
# sleep 4
# Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=48 P2PCh=36' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 48 -p2p_chan 36 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=11 P2PCh=11' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 11 -p2p_chan 11 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=6 P2PCh=11' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 6 -p2p_chan 11 -pm_mode 0 
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h GC PCIe HORNET_BRANCH_12_10 UDP Video bi-dir Long Test VSDB APCh=5 P2PCh=8' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[WLAN:VI:BI:0:300][P2P:VI:BI:0:300]' -ap_chan 5 -p2p_chan 8 -pm_mode 0
sleep 4
Test/P2PQoSNightly.test -title 'blr11 43012h PCIe HORNET_BRANCH_12_10 Direct P2P Video bi-dir Long Test P2PCh=3' -ap '4360a' -sta_gc '43012h-GC' -sta_go '43012h-GO' -email hnd-utf-list -multicore_mode 0  -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 3 -pm_mode 0
sleep 4
