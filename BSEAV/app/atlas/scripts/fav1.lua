--DEMO 1 Initialize
--  pip shown
--  bikini in main
--  cnnticker in pip

atlas.debug(string.format("Demo 1 Setup - Please Wait"))

--tune to bikini channel only if necessary
num = atlas.getCurrentChannelNumber()
if (num ~= "4.1") then
    atlas.channelTune(4.1)
    atlas.sleep(2000)
end

--show pip
if (atlas.getPipState() == false) then
    atlas.showPip(true)
end

atlas.swapPip()

--tune to cnnticker channel only if necessary
num = atlas.getCurrentChannelNumber()
if (num ~= "3.1") then
    atlas.channelTune(3.1)
    atlas.sleep(2000)
end

atlas.swapPip()

atlas.debug(string.format("Demo 1 Setup - Complete!"))
atlas.sleep(1500);
atlas.debug(string.format(""))
