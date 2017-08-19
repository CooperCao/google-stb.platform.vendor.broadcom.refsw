Test/RvRNightly1.test -ap 4360c -sta 4331t -title "4331"  -noapload -nostaload -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -attnstep 2 -email hnd-utf-list 
sleep 4s;
Test/RvRNightly1.test -ap 4360c -sta 43224t -title "43224"  -noapload -nostaload -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -attnstep 2 -email hnd-utf-list 
sleep 4s;
Test/RvRNightly1.test -ap 4360c -sta 43217t -title "43217"  -noapload -nostaload -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -attnstep 2 -email hnd-utf-list 
sleep 4s;
Test/StaNightly.test -ap 4360c -sta 4331t -noapload -nostaload  -title "BLR03 4331 StaNightly" -email hnd-utf-list -noaes -notkip

Test/StaNightly.test -ap 4360c -sta 43217t -noapload -nostaload  -title "BLR03 43217 StaNightly" -email hnd-utf-list -noaes -notkip
sleep 4s;
Test/StaNightly.test -ap 4360c -sta 43224t -noapload -nostaload  -title "BLR03 43224 StaNightly" -noaes -notkip
sleep 4s;
Test/StaNightly.test -ap 4360c -sta 43228t -noapload -nostaload  -title "BLR03 43228 StaNightly" -email hnd-utf-list -noaes -notkip
sleep 4s;
Test/RvRNightly1.test -ap 4360c -sta 43228t -title "43228"  -noapload -nostaload -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -attnstep 2 -email hnd-utf-list 