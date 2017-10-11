#4355B3

Test/RvRNightly1.test  -title '[4355b3 ACI 2G20 Int.Rate 6]' -ap 4360a -sta 4355b3n -nosniffer  -nostaload -intsta 4360b -attngrp G2 -intattn2G20 'ALL 25' -cycle2G20AttnRange '50-0'   -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 25' -loop 2 -nocompositemainpage -no5G20 -no5G40 -no2G40 -chan2G20 1 -intchan2G20 6 -downstreamonly -intrate 6  -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[4355b3 ACI 2G40 Int.Rate 6 ]' -ap 4360a -sta 4355b3n -nosniffer -nostaload  -intsta 4360b -attngrp G2 -intattn2G40 'ALL 25' -cycle2G40AttnRange   '50-0' -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G40 '0 25' -loop 2 -nocompositemainpage -no5G20 -no5G40 -no2G20 -downstreamonly -intrate 6  -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[4355b3 ACI 5G20 Int.Rate 6 ]' -ap 4360a -sta 4355b3n -nosniffer -nostaload  -intsta 4360b -attngrp G2 -intattn5G20 'ALL 30' -cycle5G20AttnRange  '50-0' -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G20 '0 25 ' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G40 -downstreamonly -intrate 6  -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[4355b3 ACI 5G40 Int.Rate 6 ]' -ap 4360a -sta 4355b3n -nosniffer -nostaload  -intsta 4360b -attngrp G2 -intattn5G40 'ALL 30' -cycle5G40AttnRange '50-0' -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G40 '0 25 ' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -downstreamonly -intrate 6  -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[4355b3 ACI 5G80 Int.Rate 6 ]' -ap 4360a -sta 4355b3n -nosniffer -nostaload  -intsta 4360b -attngrp G2 -intattn5G80 'ALL 25' -cycle5G80AttnRange '50-0' -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G80 '0 25 ' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -no5G40  -downstreamonly -intrate 6  -perftime 20 -perfsize 10 -chan5G80 36/80  -intchan5G80 52/80 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[4355b3 ACI 2G20 Int.Rate mcs 15]' -ap 4360a -sta 4355b3n -nosniffer -nostaload -intsta 4360b -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '50-0' -intwl2G20 '0 25' -loop 2 -fb1 -trenderrors -intquiet -pathloss 20  -nocompositemainpage -no5G20 -no5G40 -no2G40  -downstreamonly -intnrate 15  -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[4355b3 ACI 2G40 Int.Rate mcs 15]' -ap 4360a -sta 4355b3n -nosniffer -nostaload -intsta 4360b -attngrp G2  -intattn2G40 'ALL 25' -cycle2G40AttnRange  '50-0' -intwl2G40 '0 25' -loop 2 -fb1 -trenderrors -intquiet -pathloss 20  -nocompositemainpage -no5G20 -no5G40 -no2G20 -intchan2G40 11 -downstreamonly -intnrate 15  -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[4355b3 ACI 5G20 Int.Rate mcs 15]' -ap 4360a -sta 4355b3n -nosniffer -nostaload -intsta 4360b -attngrp G2 -intattn5G20 'ALL 20' -cycle5G20AttnRange '50-0' -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G20 '0 25 ' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G40  -downstreamonly -intnrate 15  -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[4355b3 ACI 5G40 Int.Rate mcs 15]' -ap 4360a -sta 4355b3n -nosniffer -nostaload -intsta 4360b -attngrp G2 -intattn5G40 'ALL 25' -cycle5G40AttnRange  '50-0' -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G40 '0 25 ' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20  -downstreamonly -intnrate 15  -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test -title '[4355b3 ACI 5G80 Int.Rate mcs 15]' -ap 4360a -sta 4355b3n -nosniffer -nostaload  -intsta 4360b -attngrp G2 -intattn5G80 'ALL 25' -cycle5G80AttnRange  '50-0' -fb1 -trenderrors -intquiet -pathloss 20 -intwl5G80 '0 25 ' -loop 2 -nocompositemainpage -no2G20 -no2G40 -no5G20 -no5G40  -downstreamonly -intnrate 15  -perftime 20 -perfsize 10 -chan5G80 36/80 -intchan5G80 52/80 -email hnd-utf-list
sleep 10
