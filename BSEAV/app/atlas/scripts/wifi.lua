server_ip="192.168.1.104"
count=1
while true do
    repeat
        if ("CONNECTED" == atlas.wifiConnectState())
        then
            atlas.debug(count .. "-disconnect wifi")
            atlas.sleep(1000);
            atlas.wifiDisconnect();
            atlas.sleep(5000);
        end
        atlas.debug(count .. "-connecting to ASUS_10_5G...")
        while ("CONNECTED" ~= atlas.wifiConnectState())
        do
            atlas.wifiConnect("ASUS_10_5G","ShivaNotSacko")
        end

        discoveryCount = 0
        atlas.debug(count .. "-waiting for discovery")
        while(10 > discoveryCount)
        do
            if(-1 ~= atlas.playlistShow(server_ip))
            then
                break
            end
            atlas.sleep(1000)
            discoveryCount = discoveryCount + 1
        end
    until (10 > discoveryCount)

    atlas.debug(count .. "-97445 found!")
    atlas.sleep(1000);

    for i = 1, 30 do
        atlas.debug(count .. "-start streaming...abcmpeg")
        if (-1 == atlas.channelStream("http://" .. server_ip .. ":8089/AbcMpeg2HD.mpg?program=1"))
        then
            atlas.debug(count .. "-streaming failure")
            break
        end
        atlas.sleep(15000)
        atlas.channelUntune()
        atlas.debug(count .. "-start streaming...avatar")
        if (-1 == atlas.channelStream("http://" .. server_ip .. ":8089/avatar_AVC_15M.ts?program=1"))
        then
            atlas.debug(count .. "-streaming failure")
            break
        end
        atlas.sleep(15000)
        atlas.channelUntune()
        atlas.debug(count .. "-start streaming...bbc japan")
        if (-1 == atlas.channelStream("http://" .. server_ip .. ":8089/bbc-japan_1080p.mp4?program=0"))
        then
            atlas.debug(count .. "-streaming failure")
            break
        end
        atlas.sleep(15000)
        atlas.channelUntune()
        atlas.debug(count .. "-start streaming...bikini")
        if (-1 == atlas.channelStream("http://" .. server_ip .. ":8089/bikini.mpg?program=0"))
        then
            atlas.debug(count .. "-streaming failure")
            break
        end
        atlas.sleep(15000)
    end

    atlas.debug(count .. "-stop streaming")
    atlas.channelUntune()

    count=count+1
end
