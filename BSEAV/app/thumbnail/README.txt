thumbnail demo

This application is a simple demo of PVR with thumbnail navigation.

Decoding thumbnails
---------------------------------------
Thumbnails are extracted from the stream on a regular interval using the Media Framework. 
The Media Framework allows us to support a variety of stream formats including TS, ASF, AVI, MKV, etc.

The compressed picture is then decoded with the Nexus StillDecoder. This can be decoded concurrently with a full decode.
The AVD decoder can decode stills in any codec including MPEG, AVC, VC1 and DivX.

The AVD returns an uncompressed picture in striped, YCrCb 4:2:0 format which is the native format for video, not graphics.
The Nexus Graphics2D interface (using the M2MC blitter) then converts this to YCrCb 4:2:2 which is usable as graphics.
Next, we blit this into a thumbnail-sized ARGB8888 format for use in the application. 
All other intermediate surfaces can be deleted and only the small thumbnail is kept.

Application
---------------------------------------
The user can navigate the thumbnails with the LEFT/RIGHT arrows of an IR remote (GI mode). A front panel is required.
A fixed number of thumbnails are displayed on the screen. You can scroll the thumbnails to the left or right.
Press the "SELECT" button to jump to that location in the stream.
Run the application "nexus thumbnail" to learn command line parameters and options.
