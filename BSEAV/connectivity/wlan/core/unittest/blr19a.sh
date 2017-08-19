Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 11
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test RSDB APCh=36l P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36l -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave TCP RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test RSDB APCh=3 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Slave UDP Video RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test VSDB APCh=3 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master TCP RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test VSDB APCh=3 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test RSDB APCh=3 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test RSDB APCh=36/80 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB' -sta_go '4355b3-Mst-RSDB' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Master UDP Video RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Direct Video bi-dir Long Test P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL Direct Video bi-dir Long Test P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 WLAN QoS' -ap '4360'  -sta '4355b3-Slv' -wlan_security aespsk2 -ap_chan 36/80 -wlan_bandwidth_BE 30M -wlan_bandwidth_BK 30M -wlan_bandwidth_VI 30M -wlan_bandwidth_VO 30M -run_qos -email hnd-utf-list -awdl
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 AWDL P2P QoS' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -wlan_security aespsk2 -nod -nos -ap_chan 36/80 -p2p_chan 161/80 -p2p_bandwidth_BE 30M -p2p_bandwidth_BK 30M -p2p_bandwidth_VI 30M -p2p_bandwidth_VO 30M -run_qos -email hnd-utf-list -awdl -qos_p2p_traffic
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 11
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave TCP RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Slave UDP Video RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test VSDB APCh=3 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master TCP RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test VSDB APCh=3 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Master UDP Video RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Direct Video bi-dir Long Test P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL Direct Video bi-dir Long Test P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO WLAN QoS' -ap '4360'  -sta '4355b3-Slv-RSDB-disable' -wlan_security aespsk2 -ap_chan 36/80 -wlan_bandwidth_BE 30M -wlan_bandwidth_BK 30M -wlan_bandwidth_VI 30M -wlan_bandwidth_VO 30M -run_qos -email hnd-utf-list -awdl
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 9_15 RSDB build string rsdb_config : MIMO AWDL P2P QoS' -ap '4360' -sta_gc '4355b3-Slv-RSDB-disable' -sta_go '4355b3-Mst-RSDB-disable' -wlan_security aespsk2 -nod -nos -ap_chan 36/80 -p2p_chan 161/80 -p2p_bandwidth_BE 30M -p2p_bandwidth_BK 30M -p2p_bandwidth_VI 30M -p2p_bandwidth_VO 30M -run_qos -email hnd-utf-list -awdl -qos_p2p_traffic
sleep 10

Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 11
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave TCP RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Slave UDP Video RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GC -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test VSDB APCh=3 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 3 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master TCP RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:300][P2P:TCP:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test SCC APCh=3 P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 3
sleep 10
#Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test VSDB APCh=3 P2PCh=11' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 11
#sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test VSDB APCh=3 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 3 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test SCC APCh=36l P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36l -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test VSDB APCh=36 P2PCh=149' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test VSDB APCh=48u P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36l' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36l
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test SCC APCh=36/80 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test VSDB APCh=36/80 P2PCh=161/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 161/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test SCC APCh=36 P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36 -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test VSDB APCh=48u P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 48u -p2p_chan 36/80
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test SCC APCh=36/80 P2PCh=36' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 36
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Master UDP Video RX Long Test VSDB APCh=36/80 P2PCh=48u' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -ap_connect GO -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:RX:0:300][P2P:VI:RX:0:300]' -ap_chan 36/80 -p2p_chan 48u
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Direct Video bi-dir Long Test P2PCh=36/80' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 149
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL Direct Video bi-dir Long Test P2PCh=3' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -nos -nom -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[P2P:VI:BI:0:300]' -p2p_chan 3
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 WLAN QoS' -ap '4360'  -sta '4355b3-Slv41' -wlan_security aespsk2 -ap_chan 36/80 -wlan_bandwidth_BE 30M -wlan_bandwidth_BK 30M -wlan_bandwidth_VI 30M -wlan_bandwidth_VO 30M -run_qos -email hnd-utf-list -awdl
sleep 10
Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DIN2915T165R6_BRANCH_9_41 AWDL P2P QoS' -ap '4360' -sta_gc '4355b3-Slv41' -sta_go '4355b3-Mst41' -wlan_security aespsk2 -nod -nos -ap_chan 36/80 -p2p_chan 161/80 -p2p_bandwidth_BE 30M -p2p_bandwidth_BK 30M -p2p_bandwidth_VI 30M -p2p_bandwidth_VO 30M -run_qos -email hnd-utf-list -awdl -qos_p2p_traffic
sleep 10

# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 TCP' -ap 'AP1-4331-4706 AP1-4360-4706' -apdate '2015.2.12.1' -sta '4355b3-Slv' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:5]' -ap_chan 36/80 -fb1 -attn_type 1
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 GO TCP' -ap 'AP3-4331-4706 AP3-4360-4706' -apdate '2015.2.12.1' -sta '4355b3-Mst' -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:RX:0:5]' -ap_chan 36/80 -fb1 -attn_type 1
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC TCP Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC TCP Same STA+GO' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC Video Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC Video Same STA+GO' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 1 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GO' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GO' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 1 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 1 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GO' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 1 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 1 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GO' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 1 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC TCP Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC TCP Same STA+GO' -ap '4360' -sta_gc '4355b3-Mst' -sta_go '4355b3-Slv' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC Video Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 SCC Video Same STA+GO' -ap '4360' -sta_gc '4355b3-Mst' -sta_go '4355b3-Slv' -nom -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 36/80 -fb1 -attn_type 2 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GO' -ap '4360' -sta_gc '4355b3-Mst' -sta_go '4355b3-Slv' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GO' -ap '4360' -sta_gc '4355b3-Mst' -sta_go '4355b3-Slv' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 161/80 -fb1 -attn_type 2 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 2 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB TCP Same STA+GO' -ap '4360' -sta_gc '4355b3-Mst' -sta_go '4355b3-Slv' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:TCP:BI:0:5][P2P:TCP:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 2 -ap_connect GO
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GC' -ap '4360' -sta_gc '4355b3-Slv' -sta_go '4355b3-Mst' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 2 -ap_connect GC
# sleep 10
# Test/P2PQoSNightly.test -title 'blr19a 4355 B3 Module DINGO2 VSDB Video Same STA+GO' -ap '4360' -sta_gc '4355b3-Mst' -sta_go '4355b3-Slv' -nos -nod -wlan_bandwidth_VI 40M -p2p_bandwidth_VI 40M -run_qos -email hnd-utf-list -awdl -qos_tests '[WLAN:VI:BI:0:5][P2P:VI:BI:0:5]' -ap_chan 36/80 -p2p_chan 11 -fb1 -attn_type 2 -ap_connect GO
# sleep 10