																					#file edited on 14 JULY 2015 




																					
Test/RvRNightly1.test  -title '[4331 ACI 2G Int.Rate 6]' -ap 4360ref -sta 4331t -nosniffer  -nostaload -noapload -intsta 4360aci -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '90-0' -intattn2G40 'ALL 25' -cycle2G40AttnRange '90-0'  -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 4' -loop 2 -intwl2G40 '0 4' -loop 2 -nocompositemainpage -no5G20 -no5G40  -intchan2G40 11  -downstreamonly -intrate 6   -perftime 20 -perfsize 10  -email hnd-utf-list
sleep 10
																					
Test/RvRNightly1.test  -title '[4331 ACI 2G Int.Rate MCS15]' -ap 4360ref -sta 4331t -nosniffer  -nostaload -noapload -intsta 4360aci -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '90-0'  -intattn2G40 'ALL 25' -cycle2G40AttnRange '90-0' -attnstep 2  -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 4' -loop 2 -intwl2G40 '0 4' -loop 2 -nocompositemainpage -no5G20 -no5G40  -intchan2G40 11  -downstreamonly -intnrate 15   -perftime 20 -perfsize 10  -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[43228 ACI 2G Int.Rate 6]' -ap 4360ref -sta 43228t -nosniffer  -nostaload -noapload -intsta 4360aci -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '90-0'  -intattn2G40 'ALL 25' -cycle2G40AttnRange '90-0' -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 3' -loop 2 -intwl2G40 '0 3' -loop 2 -nocompositemainpage -no5G20 -no5G40  -intchan2G40 11  -downstreamonly -intrate 6   -perftime 20 -perfsize 10  -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[43228 ACI 2G Int.mcsRate 15]' -ap 4360ref  -sta 43228t  -nosniffer   -nostaload -noapload -intsta 4360aci -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '90-0'  -intattn2G40 'ALL 25' -cycle2G40AttnRange '90-0' -intwl2G40 '0 3' -loop 2  -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 3' -loop 2 -nocompositemainpage -no5G20 -no5G40  -intchan2G40 11 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10  -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[43224 ACI 2G Int.Rate 6 ]' -ap 4360ref -sta 43224t -nostaload  -noapload -nosniffer  -intsta 4360aci -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '90-0'  -intattn2G40 'ALL 25' -cycle2G40AttnRange '90-0' -attnstep 2  -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 3' -loop 2 -intwl2G40 '0 3' -loop 2 -nocompositemainpage -no5G20 -no5G40  -intchan2G40 11 -downstreamonly -intrate 6 -perftime 20 -perfsize 10  -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[43224 ACI 2G Int.mcsRate 15]' -ap 4360ref -sta 43224t  -noapload -nosniffer -intsta 4360aci -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '90-0'  -intattn2G40 'ALL 25' -cycle2G40AttnRange '90-0' -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 3' -loop 2 -intwl2G40 '0 3' -loop 2 -nocompositemainpage -no5G20 -no5G40  -intchan2G40 11 -downstreamonly -intnrate 15 -perftime 20 -perfsize 10  -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[43217 ACI 2G Int.Rate 6]' -ap 4360ref -sta 43217t -nosniffer -nostaload -noapload -intsta 4360aci -attngrp G2  -intattn2G20 'ALL 25' -cycle2G20AttnRange '90-0'  -intattn2G40 'ALL 25'  -cycle2G40AttnRange '90-0' -attnstep 2 -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 4' -loop 2 -intwl2G40 '0 4' -nocompositemainpage   -no5G20 -no5G40 -intchan2G40 11 -downstreamonly -intrate 6  -perftime 20 -perfsize 10 -email hnd-utf-list
sleep 10

Test/RvRNightly1.test  -title '[43217 ACI 2G Int.Rate MCS15]' -ap 4360ref -sta 43217t -nosniffer  -nostaload -noapload -intsta 4360aci -attngrp G2   -intattn2G20 'ALL 25'  -cycle2G20AttnRange '90-0'   -intattn2G40 'ALL 25' -cycle2G40AttnRange '90-0' -attnstep 2  -fb1 -trenderrors -intquiet -pathloss 20 -intwl2G20 '0 4' -loop 2   -intwl2G40 '0 4' -nocompositemainpage   -no5G20 -no5G40 -intchan2G40 11 -downstreamonly -intnrate 15  -perftime 20 -perfsize 10  -email hnd-utf-list
sleep 10




