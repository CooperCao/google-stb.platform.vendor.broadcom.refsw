1. Introduce:
     This doc includes all of home networking related APP programming based on DVR-Ext lib 2.0. The programming will access mediafile I/F to achieve
     the verification of  home networking related basic and advanced features.

2. Test programs:
   - hnstreaming.c:
     This APP covers most of home networking basic function, such as getting data from MF I/F write into local file, getting data from MF I/F and streaming out to client via UDP,
     supporting full mode and trick mode, supporting fast forward and reverse way. Its input parameters and usage refer to hnstreaming.txt.
 
   - hnrecord.c:
     Home networking recording will convert the linear file format to segmented file format in dvr-ext lib. Due dvr-ext lib API has issue now, this function cannot be achieved successfully. 
     Once API function issue will been fixed, this APP will be working fine.

   - hnmediadisplay.c
     This APP covers the media playback function via home networking I/F (mediafile I/F), it supports two functions, one is hnMediaPlayTest which supports only one playback 
     with recorded file, another one is hnMediaPlayTestMultiplePlay which supports multiple playback functions perform simultaneously with recorded file. Its input parameters and 
     usage refer to hnmediadisplay.txt.


   - hn1PFTo1StreamNet.c
     This APP will achieve server streaming function with only one VLC client, it means that this utility get data from a permanent file via media file I/F, and sends data via UDP
     socket to only one VLC client. Its input parameters and usage refer to hnpfstreaming.txt.


   - hnMulPFToMulStreamNet.c
     This APP will achieve server streaming function with multiple VLC clients, it means that this utility get data from multiple permanent files via media file I/F, and sends data 
     via UDP socket to up to 8 VLC clients. Its input parameters and usage refer to hnmulpfstreaming.txt.

3. Build programs:
     Under the directory ../rockford/unittests/applibs/ocap/dvr, to perform the command "make install", then the frame work and input parameters files will be located under nexus/bin
     directory automatically.

4. Run programs:
     Under nexus/bin, you could edit textlist.txt file to enable or disable function name that you favorite or not, then execute "./nexus dvr_auto_test".


Note:
Two more additional functions hn8TSBTo8StreamFile2PBInjectionTest and hn8TSBTo8StreamNet2PBInjectionTest will be supported soon once they are ready.



