for i = 1,100 do
    print("\n==================================> Channel Up", i, "of 100\n")
    atlas.channelUp()
    atlas.sleep(3000)
end

atlas.channelUp()
print("\n==================================> Starting record session 1\n")
atlas.recordStart("triple_rec1.mpg", "./videos")
atlas.channelUp()
print("\n==================================> Starting record session 2\n")
atlas.recordStart("triple_rec2.mpg", "./videos")
atlas.channelUp()
print("\n==================================> Starting record session 3\n")
atlas.recordStart("triple_rec3.mpg", "./videos")
atlas.playbackStart("cnnticker.mpg")
atlas.sleep(30000)
print("\n==================================> Stopping record session 3\n")
atlas.recordStop("triple_rec3.mpg")
print("\n==================================> Stopping record session 2\n")
atlas.recordStop("triple_rec2.mpg")
print("\n==================================> Stopping record session 1\n")
atlas.recordStop("triple_rec1.mpg")

for i = 1,10 do
    print("\n==================================> Playback test", i, "of 10\n")
    atlas.playbackStart("riddick_avc_720p.mpg");
    atlas.sleep(60000);
    atlas.playbackStart("cnnticker.mpg");
    atlas.sleep(20000);
    atlas.playbackStart("riddick_avc_720p.mpg");
    atlas.sleep(10000);
    atlas.playbackStop("riddick_avc_720p.mpg");
end

for i = 1,10 do
    print("\n==================================> PiP/Swap test", i, "of 10\n")
    atlas.showPip(true)
    atlas.sleep(3000)
    atlas.channelUp()
    atlas.sleep(2000)
    atlas.swapPip()
    atlas.sleep(3000)
    atlas.channelDown()
    atlas.sleep(2000)
    atlas.swapPip()
    atlas.sleep(3000)
    atlas.showPip(false)
    atlas.sleep(3000)
end
