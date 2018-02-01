--DEMO 2 Cycle AC-4 Presentations

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

atlas.nextPresentation(0)

--get new presentation name/language for screen printout
index, name, lang = atlas.showPresentation()
atlas.debug(string.format("AC-4 Presentation: %s(%s)", name, lang))

atlas.sleep(1500)
atlas.debug(string.format(""))
