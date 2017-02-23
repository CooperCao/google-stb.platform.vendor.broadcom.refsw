#!/bin/bash

# global variables
# NX_STREAMS[] - use directly, or with nx_random_stream
# NX_STARTTIME - set by nx_start_server, used by nx_elapsed_time
# NX_PID[] - if a script puts a pid here, nx_wait_for_clients will wait for it
# NX_SERVERPID - set by nx_start_server, used by nx_wait_for_server

NX_STREAMS=(
    'videos/cnnticker.mpg'
    'videos/t2-hd.mpg'
    'videos/ratatouille-clip_h720p.mkv'
    'videos/wild.wmv'
    'videos/battlestar.mp4'
    'videos/BRCM_Logo_7445C_HEVC_1920x1080p60.v1.ts'
    'videos/Star_Wars_Rebels_Extended_Trailer.1920x1080.webm'
);

NX_PCMSTREAMS=(
    'audio/California_Girl.pcm'
    'audio/africa-toto.wav'
);

function nx_random_stream {
    local index=$(($RANDOM % ${#NX_STREAMS[*]}))
    echo ${NX_STREAMS[$index]}
}

function nx_random_pcmstream {
    local index=$(($RANDOM % ${#NX_PCMSTREAMS[*]}))
    echo ${NX_PCMSTREAMS[$index]}
}

# $1: timeout, based on client
function nx_start_server {
    # clean up from previous runs
    killall nxserver
    sleep 5

    ulimit -c unlimited
    ulimit -n 10000

    # start server
    if [ _${NX_NEXUS_DIR} != _ ]; then
        ( cd ${NX_NEXUS_DIR} ; exec nexus nxserver $* ) &
    else
        nexus nxserver $* &
    fi
    NX_SERVERPID=$!
    sleep 1
    wait_for_server
    local RC=$?
    if [ $RC -ne "0" ]; then
        echo FAIL: wait_for_server failed with $RC
        exit 1
    fi

    # get global variables ready
    NX_STARTTIME=`date +%s`
    declare -a NX_PID
}

function nx_elapsed_time {
    local curtime=`date +%s`
    echo $(($curtime - $NX_STARTTIME));
}

function nx_wait_for_clients {
    local pid
    # wait for all clients to exit
    for pid in ${NX_PID[*]}; do
        wait $pid
        RC=$?
        if [ $RC -ne "0" ]; then
            echo FAIL: $pid failed with $RC
            exit 1
        fi
        echo $pid exited cleanly
    done
    unset NX_PID
}

function nx_stop_server {
    echo stopping nxserver
    kill -TERM $NX_SERVERPID
    wait $NX_SERVERPID
    local RC=$?
    if [ $RC -ne "0" ]; then
        echo FAIL: nxserver failed with $RC
        exit 1
    fi
    echo nxserver exited cleanly
}

# $1 = pid to wait for clean exit
function nx_wait {
    echo nx_wait on $1
    wait $1
    local RC=$?
    if [ $RC -ne "0" ]; then
        echo FAIL: pid $1 failed with $RC
        exit 1
    fi
}

source nxclient.sh
