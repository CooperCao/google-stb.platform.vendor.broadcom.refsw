#!/bin/bash

source nxclient_common.sh

nx_start_server -frontend off
WINDOWS=`cap -video_windows`
FILENAME=$(nx_random_stream)

# Only attempt to play content we know the chip can support
while : ; do
    SUPPORTED=`cap -support_decode $FILENAME`

    if [[ $SUPPORTED -ge 1 ]]; then
        break;
    fi

    FILENAME=$(nx_random_stream)
done

while [ $(nx_elapsed_time) -lt 110 ]; do

    play $FILENAME -timeout 15 -audio_primers &
    NX_PID[0]=$!

    i=0
    while [ $i -lt 5 ] && [ $WINDOWS == 2 ]; do
        while : ; do
            PIP_FILENAME=$(nx_random_stream)
            PIP_SUPPORTED=`cap -support_decode $PIP_FILENAME`
            if [[ $PIP_SUPPORTED -ge 1 ]]; then
                break;
            fi
            PIP_FILENAME=$(nx_random_stream)
        done

        play $PIP_FILENAME -pip -timeout 2 &
        nx_wait $!
        sleep 2
        i=$(($i+1))
    done

    nx_wait_for_clients
done

nx_stop_server
