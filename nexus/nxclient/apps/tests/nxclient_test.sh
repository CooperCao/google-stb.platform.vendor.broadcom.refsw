#!/bin/sh

export LD_LIBRARY_PATH=.
export PATH=$PATH:.

PCMFILE=${PCMFILE:=audio/California_Girl.pcm}
PLAYFILE=${PLAYFILE:=videos/cnnticker.mpg}
PLAYFILE2=${PLAYFILE2:=videos/japan480p.mpg}
PLAYFILE3=${PLAYFILE3:=videos/riddick_avc_720p.mpg}
PLAYFILE4=${PLAYFILE4:=videos/discoveryAvcHD.mpg}
NUM_PCMPLAY=${NUM_PCMPLAY:=4}
PICTURES=${PICTURES:=pictures/*}

CLIENT_PID_LIST=`config -list pid`
if [ ! -z "$CLIENT_PID_LIST" ]; then kill $CLIENT_PID_LIST; fi

max_processes() {
    PROCESS_NAME=$1
    NUM_ALLOWED=$2
    while true; do
        NUM_PIDS=`ps h -C $PROCESS_NAME -o pid|wc -l`
        if [ $NUM_PIDS -gt $NUM_ALLOWED ]; then
            OLDEST_PID=`ps h -C $PROCESS_NAME kstime|grep -v defunct|cut -c 1-5|tail -1`
            echo killing client $OLDEST_PID
            kill -9 $OLDEST_PID
        else
            break
        fi
    done
}

case $1 in
0)
    # kill and recreate clients
    while [ 1 ]; do
        for x in 100 450 800 1150 1500; do
            for y in 50 400 750; do
                blit_client -rect $x,$y,300,300 &
            done
        done
        sleep 5
        killall blit_client
        sleep 1
    done
    ;;
1)
    COUNT=10
    while [ $COUNT != 0 ]; do
        animation_client -move &
        sleep 1
        blit_client -move &
        sleep 1
        COUNT=$(($COUNT-1))
    done
    ;;
2)
    video_as_graphics videos/t2-hd.mpg &
    video_as_graphics videos/herbie1AvcHD.mpg &
    video_as_graphics videos/spider_cc.mpg -max 720,480 &
    video_as_graphics videos/cnnticker.mpg -max 720,480 &
    sleep 1
    config -c 1 -vrect 1280,720:0,0,640,360
    config -c 2 -vrect 1280,720:640,0,640,360
    config -c 3 -vrect 1280,720:0,360,640,360
    config -c 4 -vrect 1280,720:640,360,640,360
    ;;
3)
    # endless cycle of play, playpcm and hdmi_input
    while [ 1 ]; do
        max_processes play 4
        max_processes playpcm $NUM_PCMPLAY
        max_processes hdmi_input 4
        max_processes live 4
        max_processes transcode 2
        let "x = $RANDOM % 920";
        let "y = $RANDOM % 520";
        let "test = $RANDOM % 7";
        case $test in
        0) play $PLAYFILE -vrect 1280,720:$x,$y,300,200 -audio_primers &
            ;;
        1) playpcm <$PCMFILE &
            ;;
        2) hdmi_input &
            ;;
        3) live -timeout 12 &
            ;;
        4) setaudio -hdmi pcm -spdif pcm &
            ;;
        5) setaudio -hdmi passthrough -spdif passthrough &
            ;;
        6) transcode -hdmi_input &
            ;;
        esac
        echo starting client $!

        sleep 1
    done
    ;;
4)
    # endless cycle of blit_clients
    while [ 1 ]; do
        max_processes blit_client 10
        let "x = $RANDOM % 920";
        let "y = $RANDOM % 520";
        blit_client -vrect 1280,720:$x,$y,300,200 &
        echo starting client $!

        usleep 10000
    done
    ;;
5)
    # multiple pcm playbacks
    play $PLAYFILE &
    sleep 1
    blit_client -vrect 720,480:360,240,330,210 &
    while [ 1 ]; do
        max_processes playpcm $NUM_PCMPLAY
        playpcm <$PCMFILE &
        echo starting client $!

        sleep 3
    done
    ;;
6)
    # suspend and resume decode
    # audio_primers are using when starting pip
    play $PLAYFILE2 -rect 50,270,960,540 -audio_primers &
    while [ 1 ]; do
        sleep 5
        echo starting second main
        play $PLAYFILE -timeout 5 -rect 960,0,960,540
        sleep 5
        echo starting pip
        play $PLAYFILE -timeout 5 -pip -rect 960,0,960,540
    done
    ;;
7)
    # display format change
    play $PLAYFILE2 &
    for x in 100 350 600 850 1100; do
        animation_client -rect $x,100,200,200 -zorder 1 &
    done
    for x in 100 350 600 850 1100; do
        blit_client -rect $x,400,200,200 -zorder 1 &
    done
    while [ 1 ]; do
        sleep 8
        setdisplay -format 720p
        sleep 8
        setdisplay -format 1080i
        sleep 8
        setdisplay -format 480p
        sleep 8
        setdisplay -format 480i
    done
    ;;

8)
    # start 3 clients and control with config
    # add primers for more test, but not really needed because video will resync
    play $PLAYFILE  -audio_primers &
    play $PLAYFILE2 -audio_primers &
    play $PLAYFILE3 -audio_primers &
    sleep 1
    config -c 1 -vrect 720,480:60,100,200,200
    config -c 2 -vrect 720,480:260,100,200,200
    config -c 3 -vrect 720,480:460,100,200,200
    let "count = 1"
    while [ 1 ]; do
        sleep 5
        config -c $count -refresh
        let "count = (count % 3) + 1"
    done
    ;;

9)
    while true; do
        picviewer $PICTURES -timeout 2000 &
        record -program 0&
        record -program 1&
        record -program 0&
        record -program 1&
        record -program 0&
        record -program 1&
        record -program 0&
        record -program 1&
        sleep 120
        killall record picviewer
    done
    ;;

10)
    while true; do
        picviewer $PICTURES -timeout 2000 &
        record -program 0&
        record -program 1&
        record -program 0&
        record -program 1&
        transcode $PLAYFILE2&
        transcode $PLAYFILE3
        killall transcode picviewer record
    done
    ;;

11)
    while true; do
        play $PLAYFILE &
        play $PLAYFILE2 -pip &
        record -program 0 -timeout 30 &
        record -program 1 -timeout 50 &
        record -program 0 -timeout 70 &
        record -program 1 -timeout 90 &
        transcode $PLAYFILE3&
        transcode $PLAYFILE4
        killall play record transcode
    done
    ;;

12)
    while true; do
        # first transcode has as size which is DSP-encoder friendly
        transcode $GUI $PLAYFILE  -video_size 416,224&
        transcode $GUI $PLAYFILE2 -video_size 352,288&
        transcode $GUI $PLAYFILE3 -video_size 352,288&
        transcode $GUI $PLAYFILE4 -video_size 352,288&
        sleep 30
        killall transcode
    done
    ;;

13)
    while true; do
        # encrypted record and playback
        record -crypto aes  -program 0 videos/stream0.aes.mpg  videos/stream0.aes.nav&
        record -crypto 3des -program 1 videos/stream1.3des.mpg videos/stream1.3des.nav&
        record -crypto aes  -program 0 videos/stream2.aes.mpg  videos/stream2.aes.nav&
        # not true timeshifting, so we must sleep to build up files
        sleep 10
        play -crypto aes  videos/stream0.aes.mpg videos/stream0.aes.nav -timeshift&
        play -crypto 3des videos/stream1.3des.mpg videos/stream1.3des.nav -pip -timeshift&
        sleep 60
        killall play record
    done
    ;;

14)
    while true; do
        mosaic_video_as_graphics videos/mosaic_6avc_6mp2_cif.ts -n 2 &
        sleep 1
        mosaic_video_as_graphics videos/mosaic_6avc_6mp2_cif.ts -n 3 &
        sleep 1
        mosaic_video_as_graphics videos/mosaic_6avc_6mp2_cif.ts -n 6 &
        sleep 1
        mosaic_video_as_graphics videos/mosaic_6avc_6mp2_cif.ts -n 1 &
        sleep 1
        config -c 1 -vrect 1280,720:0,0,640,360
        config -c 2 -vrect 1280,720:640,0,640,360
        config -c 3 -vrect 1280,720:0,360,640,360
        config -c 4 -vrect 1280,720:640,360,640,360
        sleep 60
        killall mosaic_video_as_graphics
    done
    ;;
esac
