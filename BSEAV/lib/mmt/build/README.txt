MMT Support in URSR
-------------------

mmt lib (/BSEAV/lib/mmt)
------------------------
Takes input as .tlv or .ts file and generates AV pes packets to be fed to AV decoders or to be written to a file by CPU

mmt playback (/BSEAV/lib/mmt/build/bmmt_playback.c)
-----------------------------------
Test app to playback .tlv or .ts files and to convert .tlv or .ts file to .pes file.
For mmt playback, PSI information needs to be extracted using offline tools.


TODO Items
-----------
1. live support (via extended TSMF QAM tuner)
2. recording
3. PSI parsing
4. security
5. nxclient example apps

Example platform environment setting
------------------------------------
export PATH=$PATH:/opt/toolchains/stbgcc-4.8-1.6/bin
export B_REFSW_ARCH=arm-linux
export LINUX=/opt/brcm/linux-4.1-1.10/arm
export NEXUS_PLATFORM=97278
export BCHP_VER=A0
unset CABLE_SUPPORT
export NEXUS_MODE=

Build instructions
-----------------
# cd $(URSR_TOP)/nexus/build
# make
# cd $(URSR_TOP)BSEAV/lib/mmt/build
# make install
# cd $(URSR_TOP)/nexus/utils
# make playback
# $(URSR_TOP)/obj.97278 directory should have all the binaries


Example PSI info for MMT streams:
--------------------------------

 Source       Destination    Port     Packet ID    Content
---------------------------------------------------------------
 2001::34     ff0e::1        3001     0x0000       Package List
 2001::34     ff0e::1        3001     0x0100       HEVC Video
 2001::34     ff0e::1        3001     0x0101       MPG4 AAC Audio
 2001::34     ff0e::1        3001     0x1000       Package Access
-----------------------------------------------------------------


Run instructions
----------------
a. playback .tlv file
# ./nexus mmt -input_format 2 -video_ip ff0e::1.3001 -video_id 0x100  -signalling_id 0x1000 /streams/example.tlv

b. playback  .ts file
# ./nexus mmt -input_format 1 -tlv_pid 0x2d  -video_ip ff0e::1.3001 -video_id 0x100  -signalling_id 0x1000 /streams/example.ts

c. convert .tlv file to .pes file
# ./nexus mmt -input_format 2 -video_ip ff0e::1.3001 -video_id 0x100  -audio_ip ff0e::1.3001 -audio_id 0x0101 -signalling_id 0x1000 /streams/example.tlv /streams/example_tlv.pes

d. convert .ts file to .pes file
#./nexus mmt -input_format 1 -tlv_pid 0x2d  -video_ip ff0e::1.3001 -video_id 0x100  -audio_ip ff0e::1.3001 -audio_id 0x0101 -signalling_id 0x1000 /streams/example.ts /streams/example_ts.pes

e. playback .pes file
# ./nexus playback /streams/tears_of_steel_4k_mmt_tlv.pes
# ./nexus playback /streams/tears_of_steel_4k_mmt_ts.pes

Special note
-----------
a. If AAC audio is supported by URSR, then audio can be also enabled in playback of MMT streams
b. TLV->IP->MMT encapsulation represents .tlv input file
c. MPEG2TS->TLV->IP->MMT encapsulation represents .ts input file
