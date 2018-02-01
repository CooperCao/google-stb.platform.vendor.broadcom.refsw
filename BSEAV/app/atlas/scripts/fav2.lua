--DEMO 2 Initialize
--  pip hidden
--  nba in main

atlas.debug(string.format("Demo 2 Setup - Please Wait"))

--hide pip
if (atlas.getPipState() == true) then
    atlas.showPip(false)
end

--tune to nba channel only if necessary
num = atlas.getCurrentChannelNumber()
if (num ~= "1.1") then
    atlas.channelTune(1.1)
    atlas.sleep(2000)
end

atlas.debug(string.format("Demo 2 Setup - Complete!"))
atlas.sleep(1500)
atlas.debug(string.format(""))
