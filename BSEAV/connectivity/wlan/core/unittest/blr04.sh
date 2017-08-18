Test/StaNightly.test -ap 4360a -sta "4355b3 4355b3.1 4355b3.2" -title "BLR04 StaNightly -> 4355b3 MIMO+RSDB $now" -email hnd-utf-list -ap2 4360b
sleep 30
Test/RvRNightly1.test -ap 4360a -sta 4355b3 -title "blr14 RvR -> 4355b3" -perftime 2.5 -pathloss 20 -attnstep 2 -nocompositemainpage -no2G40 -window 8m -cycle2G20AttnRange "0-95 95-0" -email hnd-utf-list -pathloss 20 -cycle2G40AttnRange "0-100 100-0" -cycle5G20AttnRange "0-100 100-0" -cycle5G40AttnRange "0-100 100-0" -cycle5G80AttnRange "0-100 100-0" -chan5G80 36/80
sleep 30
Test/StaNightly.test -ap 4360a -sta "4355b3d" -title "BLR04 StaNightly -> 4355b3 MIMO 9.41" -email hnd-utf-list
sleep 30

