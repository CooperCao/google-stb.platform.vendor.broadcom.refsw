
This is a nexus based test app, can be used to run audio tests in live mode(streamer input) 

Following are the capbilities : 
Decode of most audio codecs over DAC, SPDIF and HDMI 
Dynamic switching of SPDIF and HDMI modes(compressed, PCM, PCM5.1).
Transcode modes for SPDIF and HDMI(AC3, DTS).
Post Processings like DolbyVolume, SRS-TruVolume. 
Audio control tests like mute-unmute
PCM capture of decoder output and storing into wave files.


Build and Usage details : 
a) file located at /rockford/unittests/nexus/audio/ directory 
	make
	make <app-name> (Ex- make sqa_audio_decode_test)
	the binary and related driver gets created in /nexus/bin directory
b) locally copy or mount to the /nexus/bin directory on the reference setup
	have the streamer connected to the reference setup
	run any test stream from the streamer
	run the test app using the command "nexus <app-name>"(Ex- nexus sqa_audio_decode_test) 
	audo pcm capture can be enabled by running the app with -c or -C commandline options
	after running, user will be prompted to enter few input parameters 
	decode starts after the input parameters are entered 
	using the '?' user can get a menu for usage-modes/options supported 
