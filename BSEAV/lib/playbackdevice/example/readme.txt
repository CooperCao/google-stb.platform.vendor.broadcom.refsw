app_playback_device:

HOW TO BUILD:

   1. export the variables that would suite your needs. Minimal set of exports are under exports_935230.txt file example

   2. Go to <your_working_path>/vobs/AppLibs/broadcom/playbackdevice/example 
       and run build_playbackdevice_app script;

   Note: this will build libnexusMng.so, libPlaybackDevice.so, libcurl.so, libssl.so, libcrypto.so, libnexus.so, libb_os.so, bcmdriver.ko and app_playback_device application and will put it under <your_working_path>/vobs/AppLibs/broadcom/playbackdevice/bin directory

HOW TO RUN:

   1. Boot the target system from nfs as described in the install and build guide for ??? platform;

   2. Source the variables required to run the system on target

   3. At the prompt type: prompt> ./nexus app_card_playback_device -f http://www.and.broadcom.com/~mfalco/bolt_out.vob -vpesid 0xe0 -apesid 0xc0 to run application

RELEASE NOTES:

v2.0.0, August 16, 2011:

- open with parsing is not supported in this version;
- security (like macrovision) for analog outputs is not implemented yet;
