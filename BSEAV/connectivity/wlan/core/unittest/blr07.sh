                                                                          # created on 28th august 2015


#43602lb

Test/StaNightly.test -ap 4360ref -sta 43602lt -title '[staNightly -> 43602]' -nostaload -noapload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43602 ACI 2G20 Int.Rate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn2G20 'ALL 0' -cycle2G20AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl2G20 '0 7' -loop 2 -nocompositemainpage -no2G40 -no5G20 -no5G40 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43602 ACI 2G40 Int.Rate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn2G40 'ALL 0' -cycle2G40AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl2G40 '0 7' -loop 2 -nocompositemainpage -no2G20 -no5G20 -no5G40 -intchan2G40 11 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43602 ACI 5G20 Int.Rate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn5G20 'ALL 10' -cycle5G20AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl5G20 '0 7' -loop 2  -nocompositemainpage -no2G20 -no2G40 -no5G40 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43602 ACI 5G40 Int.Rate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn5G40 'ALL 10' -cycle5G40AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl5G40 '0 7' -loop 2  -nocompositemainpage -no2G20 -no2G40 -no5G20 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[43602 ACI 5G80 Int.Rate]' -ap 4360ref -sta 43602lt -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G80 'ALL 10' -cycle5G80AttnRange 50-0 -fb1 -trenderrors -intquiet -pathloss 30 -intwl5G80 '0 7' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -no5G40 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -chan5G80 36/80  -intchan5G80 52/80 -email hnd-utf-list
sleep 10

#43909b0

Test/StaNightly.test -ap 4360ref -sta 43909b0 -title '[staNightly -> 43909]' -nostaload -noapload -email hnd-utf-list
sleep 10

#43602lb

Test/RvRNightly1.test -title  '[43602 ACI 2G20 Int.mcsRate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn2G20 'ALL 10' -cycle2G20AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl2G20 '0 7' -loop 2 -nocompositemainpage -no2G40 -no5G20 -no5G40 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43602 ACI 2G40 Int.mcsRate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn2G40 'ALL 10' -cycle2G40AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl2G40 '0 7' -loop 2 -nocompositemainpage -no2G20 -no5G20 -no5G40 -intchan2G40 11 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43602 ACI 5G20 Int.mcsRate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn5G20 'ALL 10' -cycle5G20AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl5G20 '0 7' -loop 2  -nocompositemainpage -no2G20 -no2G40 -no5G40 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43602 ACI 5G40 Int.mcsRate]' -ap 4360ref -sta 43602lt -nosniffer -intsta 4360aci -attngrp G2 -intattn5G40 'ALL 10' -cycle5G40AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 30 -intwl5G20 '0 7' -loop 2  -nocompositemainpage -no2G20 -no2G40 -no5G20 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[43602 ACI 5G80 Int.mcsRate]' -ap 4360ref -sta 43602lt -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G80 'ALL 10' -cycle5G80AttnRange 50-0 -fb1 -trenderrors -intquiet -pathloss 30 -intwl5G80 '0 7' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -no5G40 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -chan5G80 36/80 -intchan5G80 52/80 -email hnd-utf-list
sleep 10



#43909b0


Test/RvRNightly1.test -title  '[43909b0 ACI 2G20 Int.Rate]' -ap 4360ref -sta 43909b0 -nosniffer -intsta 4360aci -attngrp G2 -intattn2G20 'ALL 25' -cycle2G20AttnRange 80-0  -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 25' -loop 2 -nocompositemainpage -no2G40 -no5G20 -no5G40 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43909b0 ACI 2G40 Int.Rate]' -ap 4360ref -sta 43909b0 -nosniffer -intsta 4360aci -attngrp G2 -intattn2G40 'ALL 25' -cycle2G40AttnRange 80-0  -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G40 '0 25' -loop 2 -nocompositemainpage -no2G20 -no5G20 -no5G40 -intchan2G40 11 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[43909b0 ACI 5G20 Int.Rate]' -ap 4360ref -sta 43909b0 -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G20 'ALL 0' -cycle5G20AttnRange 50-0 -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G20 '0 25' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G40 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[43909b0 ACI 5G40 Int.Rate]' -ap 4360ref -sta 43909b0 -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G40 'ALL 0' -cycle5G40AttnRange 50-0 -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G40 '0 25' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[43909b0 ACI 5G80 Int.Rate]' -ap 4360ref -sta 43909b0 -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G80 'ALL 0' -cycle5G80AttnRange 50-0  -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G80 '0 25' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -no5G40 -downstreamonly -intrate 6 -perftime 20 -perfsize 10 -chan5G80 36/80  -intchan5G80 52/80 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43909b0 ACI 2G20 Int.mcsRate]' -ap 4360ref -sta 43909b0 -nosniffer -intsta 4360aci -attngrp G2 -intattn2G20 'ALL 25' -cycle2G20AttnRange 80-0  -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 25' -loop 2 -nocompositemainpage -no2G40 -no5G20 -no5G40 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title  '[43909b0 ACI 2G40 Int.mcsRate]' -ap 4360ref -sta 43909b0 -nosniffer -intsta 4360aci -attngrp G2 -intattn2G40 'ALL 25' -cycle2G40AttnRange 80-0  -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G40 '0 25' -loop 2 -nocompositemainpage -no2G20 -no5G20 -no5G40 -intchan2G40 11 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -nostaload -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[43909b0 ACI 5G20 Int.mcsRate]' -ap 4360ref -sta 43909b0 -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G20 'ALL 0' -cycle5G20AttnRange 50-0 -attnstep 2  -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G20 '0 25' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G40 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[43909b0 ACI 5G40 Int.mcsRate]' -ap 4360ref -sta 43909b0 -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G40 'ALL 0' -cycle5G40AttnRange 50-0  -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G40 '0 25' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -email hnd-utf-list  
sleep 10

Test/RvRNightly1.test -title '[43909b0 ACI 5G80 Int.mcsRate]' -ap 4360ref -sta 43909b0 -nosniffer -nostaload -intsta 4360aci -attngrp G2 -intattn5G80 'ALL 0' -cycle5G80AttnRange 50-0 -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G80 '0 25' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -no5G40 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10 -chan5G80 36/80 -intchan5G80 52/80 -email hnd-utf-list
sleep 10





