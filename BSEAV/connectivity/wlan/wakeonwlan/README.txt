Run below commands/script on STB to configure it as SoftAP with WOWL capabilities. After this AP is configured and can be used to send magic packets 

	wlInstall wl.ko wlan0 nvram.txt
	wl down
	wl infra 1
	wl ap 1
	wl chanspec 100/80
	wl up
	wl ssid 5g
	ifconfig wlan0 192.168.1.1


Run below commands/script on STB to configure it as STA with WOWL capabilities. After thi STA is capable to go to S2 state and can wake up with magic packet 

	insmod wowl-plat.ko
	wlinstall wl.ko wlan0 nvram.txt
	wl down
	wl infra 1
	wl up
	wl scan 5g
	wl scanresults
	sleep 5
	wl join 5g
	sleep 10
	ifconfig wlan0 192.168.1.2
	wl wowl 0x01 
	wl wowl_activate
	wl wowl_wakeind
	cd obj.97271/nexus/bin
	./nexus standby
	### type s2 to move to S2 state


Now system is in S2, send below command on the softAP to send a magic packet and verify that STA can wakeup for reason WOWL 


	wl -i wlan0 wowl_pkt 104 ucast 00:10:18:D5:C3:10 magic
