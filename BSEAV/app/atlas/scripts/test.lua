tuneReps     = 5            --repeat Tuning test
playbackReps = 2            --repeat Playback test
pipReps      = 3            --repeat PiP test
numPlaylist  = 1            --playlist to look for stream in
strStream    = "bikini.mpg" --stream to search playlist for


function checkError(err)
    if -1 == err then
        atlas.debug("*** Lua Error! Stopping script ***")
        error()
    end
end

function checkErrorSleep(err, delay)
    checkError(err)
    atlas.sleep(delay)
end

::test_channel_up::
for i = 1,tuneReps do
    atlas.debug(string.format("Channel Up %d of %d", i, tuneReps))
    checkError(atlas.channelUp())
end

::test_channel_down::
for i = 1,tuneReps do
    atlas.debug(string.format("Channel Down %d of %d", i, tuneReps))
    checkError(atlas.channelDown())
end

::test_tune_untune::
atlas.debug("Channel Tune(1.1)")
checkErrorSleep(atlas.channelTune(1.1), 2000)
atlas.debug("Channel UNTune")
checkErrorSleep(atlas.channelUntune(), 2000)
atlas.debug("Channel Tune(1.2)")
checkErrorSleep(atlas.channelTune(1.2), 2000)
atlas.debug("Channel Tune(1.3)")
checkErrorSleep(atlas.channelTune(1.3), 2000)

::test_QAM_channel_scan::
atlas.debug("Scan Qam find 19 San Diego channels")
checkErrorSleep(atlas.scanQam(57000000,255000000,6000000,6000000,false,9,2), 3000)
atlas.debug("Channel Tune(0) - first channel in channel list")
checkError(atlas.channelTune(0))

::test_record::
atlas.debug("Record triple_rec1.mpg")
checkError(atlas.recordStart("triple_rec1.mpg", "./videos"))
checkError(atlas.channelUp())
atlas.debug("Record triple_rec2.mpg")
checkError(atlas.recordStart("triple_rec2.mpg", "./videos"))
checkError(atlas.channelUp())
atlas.debug("Record triple_rec3.mpg")
checkError(atlas.recordStart("triple_rec3.mpg", "./videos"))
atlas.debug("Record for 10 seconds")
atlas.sleep(10000)
atlas.debug("Channel Down to 2nd recording channel")
checkError(atlas.channelDown())
atlas.debug("Channel Down to 1st recording channel")
checkError(atlas.channelDown())
atlas.debug("Stop Records")
atlas.sleep(2000)
atlas.debug("Stop Record: triple_rec3.mpg")
checkError(atlas.recordStop("triple_rec3.mpg"))
atlas.debug("Stop Record: triple_rec2.mpg")
checkError(atlas.recordStop("triple_rec2.mpg"))
atlas.debug("Stop Record: triple_rec1.mpg")
checkError(atlas.recordStop("triple_rec1.mpg"))

::test_playback::
for i = 1,playbackReps do
    atlas.debug(string.format("Playback test %d of %d: riddick_avc_720p.mpg", i, playbackReps))
    checkErrorSleep(atlas.playbackStart("riddick_avc_720p.mpg"), 30000)
    atlas.debug(string.format("Playback test %d of %d: cnnticker.mpg", i, playbackReps))
    checkErrorSleep(atlas.playbackStart("cnnticker.mpg"), 15000)
    atlas.debug(string.format("Playback test %d of %d: AbcMpeg2HD.mpg", i, playbackReps))
    checkErrorSleep(atlas.playbackStart("AbcMpeg2HD.mpg"), 7500)
    atlas.debug(string.format("Playback test %d of %d: bikini.mpg", i, playbackReps))
    checkErrorSleep(atlas.playbackStart("bikini.mpg"), 5000)
    atlas.debug("Pause")
    checkErrorSleep(atlas.playbackTrickMode("pause"), 3000)
    atlas.debug("Fast Forward 15x")
    checkErrorSleep(atlas.playbackTrickMode("rate(15)"), 4000)
    atlas.debug("Fast Forward 128x")
    checkErrorSleep(atlas.playbackTrickMode("rate(128)"), 4000)
    atlas.debug("Rewind 15x")
    checkErrorSleep(atlas.playbackTrickMode("rate(-15)"), 3000)
    atlas.debug("Rewind 128x")
    checkErrorSleep(atlas.playbackTrickMode("rate(-128)"), 3000)
    atlas.debug("Play")
    checkErrorSleep(atlas.playbackTrickMode("play"), 3000)
    atlas.debug("Jump to 00:30")
    checkErrorSleep(atlas.playbackTrickMode("seek(30000)"), 2000)
    atlas.debug("Jump to 01:00")
    checkErrorSleep(atlas.playbackTrickMode("seek(60000)"), 2000)
    atlas.debug("Jump to 00:30")
    checkErrorSleep(atlas.playbackTrickMode("seek(30000)"), 2000)
    atlas.debug("Jump to 07:00")
    checkErrorSleep(atlas.playbackTrickMode("seek(480000)"), 2000)
    atlas.debug("Play")
    checkErrorSleep(atlas.playbackTrickMode("play"), 5000)
    atlas.debug("Fast Forward 256x until wrap around")
    checkErrorSleep(atlas.playbackTrickMode("rate(256)"), 8000)
    atlas.debug("Rewind 256x stop at beginning")
    checkErrorSleep(atlas.playbackTrickMode("rate(-256)"), 6000)
    atlas.debug("Play")
    checkErrorSleep(atlas.playbackTrickMode("play"), 5000)
    checkError(atlas.playbackStop("bikini.mpg"))
end

::test_content_modes::
atlas.debug("")
checkErrorSleep(atlas.playbackStart("cnnticker.mpg"), 4000)
atlas.debug("Content Mode: zoom")
checkErrorSleep(atlas.setContentMode(0), 4000)
atlas.debug("Content Mode: box")
checkErrorSleep(atlas.setContentMode(1), 4000)
atlas.debug("Content Mode: pan/scan")
checkErrorSleep(atlas.setContentMode(2), 4000)
atlas.debug("Content Mode: full")
checkErrorSleep(atlas.setContentMode(3), 4000)
atlas.debug("Content Mode: stretch")
checkErrorSleep(atlas.setContentMode(4), 4000)
atlas.debug("Content Mode: pan/scan no a/r")
checkErrorSleep(atlas.setContentMode(5), 4000)
atlas.debug("Content Mode: box")
checkErrorSleep(atlas.setContentMode(1), 4000)
checkError(atlas.playbackStop("cnnticker.mpg"))
atlas.debug("Channel Tune(0) - first channel in channel list")
checkError(atlas.channelTune(0))

::test_color_spaces::
atlas.debug("colorspace: RGB")
checkErrorSleep(atlas.setColorSpace(1), 7000)
atlas.debug("colorspace: 4:4:2")
checkErrorSleep(atlas.setColorSpace(2), 7000)
atlas.debug("colorspace: 4:4:4")
checkErrorSleep(atlas.setColorSpace(3), 7000)
atlas.debug("colorspace: 4:4:0")
checkErrorSleep(atlas.setColorSpace(4), 7000)
atlas.debug("colorspace: Auto")
checkErrorSleep(atlas.setColorSpace(0), 7000)

::test_deinterlacing::
checkError(atlas.setAutoVideoFormat(true))
atlas.debug("Playback batman_avc_480i.mpg")
checkErrorSleep(atlas.playbackStart("batman_avc_480i.mpg"), 7000)
checkError(atlas.setAutoVideoFormat(false))
atlas.debug("Change video format to 1080i")
checkErrorSleep(atlas.setVideoFormat("1080i"), 7000)
atlas.debug("Deinterlacer: off")
checkErrorSleep(atlas.setDeinterlacer(false), 3000)
atlas.debug("Deinterlacer: ON")
checkErrorSleep(atlas.setDeinterlacer(true), 3000)
atlas.debug("Deinterlacer: off")
checkErrorSleep(atlas.setDeinterlacer(false), 3000)
atlas.debug("Deinterlacer: ON")
checkErrorSleep(atlas.setDeinterlacer(true), 3000)
atlas.debug("Deinterlacer: off")
checkErrorSleep(atlas.setDeinterlacer(false), 3000)
atlas.debug("Deinterlacer: ON")
checkErrorSleep(atlas.setDeinterlacer(true), 3000)
checkError(atlas.playbackStop("batman_avc_480i.mpg"))

::test_aspect_ratio::
checkErrorSleep(atlas.playbackStart("cnnticker.mpg"), 3000)
atlas.debug("Aspect Ratio: 4x3")
checkErrorSleep(atlas.setAspectRatio(1), 3000)
atlas.debug("Aspect Ratio: 16x9")
checkErrorSleep(atlas.setAspectRatio(2), 3000)
atlas.debug("Aspect Ratio: Auto")
checkErrorSleep(atlas.setAspectRatio(0), 3000)
checkError(atlas.playbackStop("cnnticker.mpg"))

::test_video_format_change::
atlas.debug("Auto Video Format: ON")
checkError(atlas.setAutoVideoFormat(true))
checkErrorSleep(atlas.channelTune(0), 4000)
atlas.debug("Channel UP 4 times - video format will change to match source")
checkErrorSleep(atlas.channelUp(), 4000)
checkErrorSleep(atlas.channelUp(), 4000)
checkErrorSleep(atlas.channelUp(), 4000)
checkErrorSleep(atlas.channelUp(), 4000)
checkErrorSleep(atlas.channelTune(0), 4000)
atlas.debug("Auto Video Format: off")
checkError(atlas.setAutoVideoFormat(false))

::test_channel_list::
atlas.debug("Save channel list")
atlas.sleep(2000)
checkError(atlas.channelListSave("channels_test.xml"))
atlas.debug("Load channel list")
atlas.sleep(2000)
checkError(atlas.channelListLoad("channels_test.xml"))

::test_encode::
--[[
atlas.debug("Channel Tune(0) - first channel in channel list")
checkError(atlas.channelTune(0))
atlas.debug("Encode channel start")
checkErrorSleep(atlas.encodeStart(), 10000)
atlas.debug("Encode channel stop")
checkErrorSleep(atlas.encodeStop(), 3000)
]]--

atlas.debug("Refresh Playback List")
checkError(atlas.playbackListRefresh())

::test_audio::
atlas.debug("Play mediasession.mpg")
checkErrorSleep(atlas.playbackStart("mediasession.mpg"), 5000)
atlas.debug("Audio PID to 0x1A9A (2nd PID)")
checkErrorSleep(atlas.setAudioProgram(0x1a9a), 3000)
atlas.debug("Audio PID to 0x1A99 (1st PID)")
checkErrorSleep(atlas.setAudioProgram(0x1a99), 3000)
atlas.debug("S/PDIF to compressed")
checkErrorSleep(atlas.setSpdifType(1), 3000)
atlas.debug("S/PDIF to PCM")
checkErrorSleep(atlas.setSpdifType(0), 3000)
atlas.debug("HDMI to compressed")
checkErrorSleep(atlas.setHdmiAudioType(1), 3000)
atlas.debug("HDMI to PCM")
checkErrorSleep(atlas.setHdmiAudioType(0), 3000)
atlas.debug("Downmix to AC-3 Surround")
checkErrorSleep(atlas.setDownmix(9), 3000)
atlas.debug("Downmix to AC-3 Standard")
checkErrorSleep(atlas.setDownmix(8), 3000)
atlas.debug("Downmix to Auto")
checkErrorSleep(atlas.setDownmix(7), 3000)
atlas.debug("DualMono to Left")
checkErrorSleep(atlas.setDualMono(0), 3000)
atlas.debug("DualMono to Right")
checkErrorSleep(atlas.setDualMono(1), 3000)
atlas.debug("DualMono to Stereo")
checkErrorSleep(atlas.setDualMono(2), 3000)
atlas.debug("DualMono to MonoMix")
checkErrorSleep(atlas.setDualMono(3), 3000)
atlas.debug("Dolby DRC to Heavy")
checkErrorSleep(atlas.setDolbyDRC(3), 3000)
atlas.debug("Dolby DRC to Medium")
checkErrorSleep(atlas.setDolbyDRC(2), 3000)
atlas.debug("Dolby DRC to None")
checkErrorSleep(atlas.setDolbyDRC(0), 3000)
atlas.debug("Dolby Dialog Normalization: ON")
checkErrorSleep(atlas.setDolbyDialogNorm(true), 3000)
atlas.debug("Dolby Dialog Normalization: off")
checkErrorSleep(atlas.setDolbyDialogNorm(false), 3000)
atlas.debug("Adjust Volume")
for i = 1,100 do
    checkErrorSleep(atlas.setVolume(i), 100)
end
atlas.debug("Mute: ON")
checkErrorSleep(atlas.setMute(true), 2000)
atlas.debug("Mute: off")
checkErrorSleep(atlas.setMute(false), 2000)
atlas.debug("Auto Volume Level")
checkErrorSleep(atlas.setAudioProcessing(1), 2000)
atlas.debug("Dolby Volume")
checkErrorSleep(atlas.setAudioProcessing(2), 2000)
atlas.debug("SRS TruVolume")
checkErrorSleep(atlas.setAudioProcessing(3), 2000)
checkErrorSleep(atlas.setAudioProcessing(0), 2000)
checkError(atlas.playbackStop("mediasession.mpg"))

::test_pip::
for i = 1,pipReps do
    atlas.debug("Pip/Swap test %d of %d", i, pipReps)
    atlas.debug("Show PiP")
    atlas.sleep(1000)
    checkErrorSleep(atlas.showPip(true),2000)
    atlas.debug("Channel Up")
    atlas.sleep(1000)
    checkErrorSleep(atlas.channelUp(), 2000)
    atlas.debug("Swap PiP")
    atlas.sleep(1000)
    checkErrorSleep(atlas.swapPip(), 2000)
    atlas.debug("Channel Down")
    atlas.sleep(1000)
    checkErrorSleep(atlas.channelDown(), 2000)
    atlas.debug("Swap PiP Again")
    atlas.sleep(1000)
    checkErrorSleep(atlas.swapPip(), 2000)
    atlas.debug("Hide PiP")
    atlas.sleep(1000)
    checkErrorSleep(atlas.showPip(false), 2000)
end

::test_cc::
atlas.debug("Channel Tune(1.2 - tune to PBS for CC testing")
checkError(atlas.channelTune(1.2))
atlas.debug("DCC Enable")
checkErrorSleep(atlas.closedCaptionEnable(true), 9000)
atlas.debug("DCC Disable")
checkErrorSleep(atlas.closedCaptionEnable(false), 3000)
atlas.debug("VBI Closed Captions ON")
checkErrorSleep(atlas.setVbiClosedCaptions(true), 9000)
atlas.debug("VBI Closed Captions off")
checkErrorSleep(atlas.setVbiClosedCaptions(false), 3000)

::test_ip::
--get the ip of the playlist referenced by numPlaylist
ip, name, ret = atlas.playlistDiscovery(numPlaylist)
checkError(ret)

--iterate thru playlist searching for strStream name
atlas.debug(string.format("Find %s in playlist #%d", strStream, numPlaylist))
i = 1
repeat
    url, ret = atlas.playlistShow(ip, i)
    checkError(ret)
    if url == nil then
        --done searching all playlist entries
        break
    end
    first, last = string.find(url, strStream)
    if first ~= nil then
        --found matching stream name!
        break
    end
    i = i + 1
until url == nil

if url ~= nil then
    --stream found - start streaming
    checkErrorSleep(atlas.channelStream(url), 15000)
    atlas.debug("Stop streaming")
    checkError(atlas.channelTune(0))
else
    --stream NOT found
    atlas.debug(string.format("%s not found in playlist #%d", strStream, numPlaylist))
    atlas.sleep(3000)
end

::done::
atlas.debug("Test Complete!")
atlas.sleep(3000)
atlas.debug("")
do return end
