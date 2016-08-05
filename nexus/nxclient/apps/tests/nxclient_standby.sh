#!/bin/bash

NX_STANDBYMODE=(
    's0'
    's1'
    's2'
    's3'
);

function nx_random_standbymode {
    local index=$(($RANDOM % ${#NX_STANDBYMODE[*]}))
    echo ${NX_STANDBYMODE[$index]}
}

source nxclient_common.sh

nx_start_server
play videos/cnnticker.mpg&
play_pid=$!

while [ $(nx_elapsed_time) -lt 180 ]; do
    MODE=$(nx_random_standbymode)
    echo "Entering $MODE mode"
    standby -$MODE
    if [ $MODE = "s0" ]; then
        sleep 5
    fi
done

standby -s0
kill -TERM $play_pid
nx_stop_server
