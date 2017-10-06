now=$(date +"%Y.%m.%d")
Test/RouterNightly.test -aps "53574b0 53574b0_5G 53574b0_2G" -stas "4360amb 4360bmb" -title "BLR18 RouterNightly 53574b0 MIMO + RSDB with 2x2 mesh txchain/rxchain 3 $now" -noapload -email hnd-utf-list
sleep 30
Test/RouterNightly.test -aps "47189MC5 47189MC2" -stas "4360amb 4360bmb" -title "BLR18 RouterNightly 47189 MIMO  with 2x2 mesh txchain/rxchain 3 $now" -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "47189MC5 47189MC2" -sta 4360amb -nocompositemainpage -title "RvR -> 47189 2x2 Mesh Rxchain/Txchain 3 $now" -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -chan5G80 36/80 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "53574b0" -sta 4360amb -nocompositemainpage -title "RvR -> 53573 MIMO 2x2 Mesh Rxchain/Txchain 3 $now" -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -chan5G80 36/80 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "53574b0_5G" -sta 4360amb -nocompositemainpage -title "RvR -> 53573 5G 2x2 Mesh Rxchain/Txchain 3 $now" -no2G20 -no2G40 -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -chan5G80 36/80 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "53574b0_2G" -sta 4360amb -nocompositemainpage -title "RvR -> 53573 2G 2x2 Mesh Rxchain/Txchain 3 $now" -no5G20 -no5G40 -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
Test/RouterNightly.test -aps "53574NR" -stas "4360am 4360bm" -title "blr18 RouterNightly 53574ACNR with 2x2 mesh txchain/rxchain 3" -noapload -email hnd-utf-list
sleep 30
Test/RvRNightly1.test -ap "53574NR" -sta 4360am -nocompositemainpage -title "blr18 RvR -> 53574ACNR 2x2 Mesh Rxchain/Txchain 3" -no2G20 -no2G40 -cycle2G20AttnRange "0-95 95-0" -cycle2G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G80AttnRange "0-95 95-0" -pathloss 20 -chan5G80 36/80 -attnstep 2 -perftime 2.5 -noapload -email hnd-utf-list
sleep 30
