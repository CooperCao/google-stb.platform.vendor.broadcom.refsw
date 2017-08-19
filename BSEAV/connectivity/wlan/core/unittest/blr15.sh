#test
Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "4MUTXBF RSDB 1x1 UDP muonly TRY 6/5" -attnstep 4 -musta "4359b1x1 4359c1x1 4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1600 -no5G40 -no5G20 -downstreamonly
sleep 5
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "4MUTXBF RSDB 1x1 UDP TRY 6/5" -attnstep 4 -musta "4359b1x1 4359c1x1 4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1600 -no5G40 -no5G20 -downstreamonly
sleep 5
#2x2
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a2x2 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "2MUTXBF 2X2 UDP 5G TRY6/5" -attnstep 4 -musta "4359b2x2" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1200 -no5G40 -no5G20 -downstreamonly
sleep 5


#3MUTXBF
Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "3MUTXBF 1x1 UDP muonly ABC" -attnstep 4 -musta "4359b1x1 4359c1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1200 -no5G40 -no5G20 -downstreamonly
sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "3MUTXBF 1x1 UDP ABC" -attnstep 4 -musta "4359b1x1 4359c1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1200 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359b1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "3MUTXBF 1x1 UDP muonly BCD" -attnstep 4 -musta "4359c1x1 4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1200 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "3MUTXBF 1x1 UDP BCD" -attnstep 4 -musta "4359c1x1 4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1200 -no5G40 -no5G20 -downstreamonly
# sleep 5

#2MUTXBF-test
Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP AB muonly" -attnstep 4 -musta "4359b1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP AC muonly" -attnstep 4 -musta "4359c1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP AD muonly" -attnstep 4 -musta "4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359b1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP BA muonly" -attnstep 4 -musta "4359a1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359b1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP BC muonly" -attnstep 4 -musta "4359c1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359b1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP BD muonly" -attnstep 4 -musta "4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359c1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP CA muonly" -attnstep 4 -musta "4359a1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359c1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP CB muonly" -attnstep 4 -musta "4359b1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359c1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP CD muonly" -attnstep 4 -musta "4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359d1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP DA muonly" -attnstep 4 -musta "4359a1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
# Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359d1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP DB muonly" -attnstep 4 -musta "4359b1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
# sleep 5
Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359d1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "2MUTXBF 1x1 UDP DC muonly" -attnstep 4 -musta "4359c1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
sleep 5

#MUTXBF
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "4MUTXBF RSDB 1x1 UDP Run1 ADCB" -attnstep 4 -musta "4359d1x1 4359c1x1 4359b1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1600 -no5G40 -no5G20 -downstreamonly
sleep 5
Test/RvRNightly1.test -ap "4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -title "4MUTXBF RSDB 1x1 UDP Run 2" -attnstep 4 -musta "4359b1x1 4359c1x1 4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1600 -no5G40 -no5G20 -downstreamonly
sleep 5
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "2MUTXBF 1x1 UDP" -attnstep 4 -musta "4359d1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 800 -no5G40 -no5G20 -downstreamonly
sleep 5
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "3MUTXBF 1x1 UDP" -attnstep 4 -musta "4359b1x1 4359c1x1" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1200 -no5G40 -no5G20 -downstreamonly
sleep 5
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a2x2 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "3MUTXBF 2x2 UDP" -attnstep 4 -musta "4359b2x2 4359c2x2" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1600 -no5G40 -no5G20 -downstreamonly
sleep 5
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a2x2 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "4MUTXBF 2x2 UDP" -attnstep 4 -musta "4359b2x2 4359c2x2 4359d2x2" -cycle5G80AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1800 -no5G40 -no5G20 -downstreamonly
sleep 5
# MIX
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -no2G20 -no2G40 -no5G20 -no5G40 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list -compareap -downstreamonly -title "MIX 2mu-1su TCP" -attnstep 4 -musta "4359b1x1 4359d1x1su" -downstreamonly -udp 1200 -attngrp G5 -cycle5G80AttnRange "0-95 95-0"
sleep 4
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -no2G20 -no2G40 -no5G20 -no5G40 -chan5G80 36/80 -perftime 2.5 -fb1 -window 2m -email "hnd-utf-list" -compareap -downstreamonly -title "MIX 1mu-1su UDP" -cycle5G80AttnRange "0-95 95-0" -attnstep 4 -musta 4359b1x1su -attngrp G5 -window 2m -udp 800
sleep 4
#FULL
Test/RvRNightly1.test -ap "4366softap-mutx0 4366softap-mutx1" -sta 4359a1x1 -chan5G80 36/80 -chan5G40 36l -chan5G20 36 -perftime 2.5 -fb1 -window 2m -email hnd-utf-list.pdl -compareap -title "4MUTXBF 1x1 UDP 5G ALL" -attnstep 4 -musta "4359b1x1 4359c1x1 4359d1x1" -cycle5G80AttnRange "0-95 95-0" -cycle5G40AttnRange "0-95 95-0" -cycle5G20AttnRange "0-95 95-0" -no2G20 -no2G40 -attngrp G5 -udp 1600 -downstreamonly
sleep 5