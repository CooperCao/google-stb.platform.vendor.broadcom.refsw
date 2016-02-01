NxClient Instructions

Read nexus/nxclient/docs/NxClient.pdf for an overview of NxClient.

Building:

    cd nexus/nxclient
    make

Running:

    # start server
    nexus nxserver &
    
    # start clients
    . nxclient.sh
    blit_client &
    play videos/mystream.mpg &

Command line help:

    nexus nxserver --help
    
    . nxclient.sh
    blit_client --help
    play --help
    etc.

Example uses cases:

    # run sample launcher app
    desktop

    # display graphics
    blit_client
    animation_client

    # DVR playback
    play videos/cnnticker.mpg
    play -pip videos/starwars.mp4
    playmosaic -n 3 videos/sample.asf videos/another.avi

    # show and change display format
    setdisplay
    setdisplay -format 720p
    setdisplay -format 1080i50 -sd -format pal
    setdisplay -3d ou

    # show and change audio configuration
    setaudio
    setaudio -hdmi pcm -spdif passthrough

    # transcode a file or HDMI input
    transcode -video_size 720,480 videos/mpegfile.ts videos/avcfile.ts
    transcode -rt videos/input.ts videos/output.ts
    transcode -hdmi_input

    # play PCM audio, mixed with decoded audio
    playpcm audio.pcm

    # record from frontend or streamer
    record -qam -freq 777 -program 1
    record -streamer 0 -program 0
    record -crypto aes videos/myfile.ts videos/myfile.nav

    # list and resize clients
    config
    config -c 1 -rect 100,100,400,300

    # show JPEG, PNG, GIF, BMP
    picviewer /mnt/hd/pictures/*.jpg /mnt/hd/pictures/*.bmp

    # show some graphics, take a screenshot, then display it
    blit_client &
    screenshot /mnt/hd/pictures/screenshot.bmp
    picviewer /mnt/hd/pictures/screenshot.bmp

    # IP client (must compiled with PLAYBACK_IP_SUPPORT=y)
    play http://www.yourserver.com/mystream.mpg
