Test/RvRNightly1.test -ap 4360d -sta 43458c0 -title "43458 SDIO"  -noapload -nostaload -nostareload -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -attnstep 2 -email hnd-utf-list 
sleep 5s;
Test/RvRNightly1.test -ap 4360d -sta 43458b0 -title "43458 PCIe"  -noapload -nostaload -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -attnstep 2 -email hnd-utf-list 
sleep 5s;
Test/RvRNightly1.test -ap 4360d -sta 43430b0 -title "43430"  -noapload -nostaload -nostareload -no2G40 -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -attnstep 2 -email hnd-utf-list
sleep 5s;
Test/StaNightly.test -ap 4360d -sta 43458b0 -noapload -nostaload -title "BLR16 43458 PCIe StaNightly" -email hnd-utf-list
sleep 4s;
Test/StaNightly.test -ap 4360d -sta 43458c0 -noapload -nostaload -nostareload -title "BLR16 43458 SDIO StaNightly" -email hnd-utf-list
sleep 4s;
Test/StaNightly.test -ap 4360d -sta 43430b0 -noapload -nostaload -nostareload -title "BLR16 43430 StaNightly" -email hnd-utf-list -notkip -noaes


