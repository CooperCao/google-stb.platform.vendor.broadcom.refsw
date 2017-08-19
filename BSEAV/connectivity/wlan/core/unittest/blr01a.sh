Test/RouterNightly.test -aps "53574 53574_5G 53574_2G" -stas "4360am 4360bm" -title "BLR01 RouterNightly 53574b0 MIMO + RSDB without  mesh txchain/rxchain 3 " -noapload -email hnd-utf-list
sleep 30
Test/RouterNightly.test -aps "47189MC5 47189MC2" -stas "4360am 4360bm" -title "BLR01 RouterNightly 47189 MIMO  without  mesh txchain/rxchain 3 " -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "53574" -sta 4360am -nocompositemainpage -title "RvR -> 53574 MIMO without Mesh Rxchain/Txchain 3 " -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -chan5G80 36/80 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "53574_5G" -sta 4360am -nocompositemainpage -title "RvR -> 53574 5G without Mesh Rxchain/Txchain 3 " -no2G20 -no2G40 -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -chan5G80 36/80 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "53574_2G" -sta 4360am -nocompositemainpage -title "RvR -> 53574 2G without Mesh Rxchain/Txchain 3 " -no5G20 -no5G40 -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "47189MC5 47189MC2" -sta 4360am -nocompositemainpage -title "RvR -> 47189 without Mesh Rxchain/Txchain 3 " -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -chan5G80 36/80 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
