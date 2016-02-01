#!/bin/bash

source nxclient_common.sh

nx_start_server -frontend off
WINDOWS=`cap -video_windows`

while [ $(nx_elapsed_time) -lt 110 ]; do
    play $(nx_random_stream) -timeout 15 -audio_primers &
    NX_PID[0]=$!

    i=0
    while [ $i -lt 5 ] && [ $WINDOWS == 2 ]; do
        play $(nx_random_stream) -pip -timeout 2 &
        nx_wait $!
        sleep 2
        i=$(($i+1))
    done

    nx_wait_for_clients
done

nx_stop_server
