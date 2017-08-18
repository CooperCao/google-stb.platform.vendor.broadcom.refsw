now=$(date +"%Y.%m.%d")
sleep 5
Test/StaNightly.test -ap 4360 -sta 4355b2 -title "BLR22 StaNightly -> 4355b2 MIMO $now" -email hnd-utf-list
sleep 5 
Test/RvRNightly1.test -ap 4360 -sta 43012a0h -title "BLR22 RvrNightly -> 43012a0 Hornet $now" -email hnd-utf-list -cycle2G20AttnRange "0-100 100-0" -no2G40 -no5G20 -no5G40
sleep 5
Test/StaNightly.test -ap 4360 -sta 43012a0h -title "BLR22 StaNightly -> 43012a0 Hornet $now" -email hnd-utf-list -noperf40 -nocal -nopm
