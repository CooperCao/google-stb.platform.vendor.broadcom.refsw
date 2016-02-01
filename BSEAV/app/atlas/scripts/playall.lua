for fname in dir("./videos") do
    if ((nil ~= string.find(fname, ".mpg"))  or 
        (nil ~= string.find(fname, ".mp4"))  or
        (nil ~= string.find(fname, ".ts" ))) then

        print("LUA> Now Playing: " .. fname)
        atlas.playbackStart(fname)
        atlas.sleep(5000)
        atlas.playbackStop(fname)
    end
end

--[[
atlas.quit()
--]]
