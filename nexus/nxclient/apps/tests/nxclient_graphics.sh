#!/bin/bash

source nxclient_common.sh

nx_start_server -frontend off

while [ $(nx_elapsed_time) -lt 60 ]; do
    INDEX=$(($RANDOM % 32));
    if [ -z ${NX_PID[$INDEX]} ]; then
        X=$(($RANDOM % 1920));
        Y=$(($RANDOM % 1080));
        if [ $((RANDOM % 2)) == "0" ]; then
            APP=blit_client
        else
            APP=animation_client
        fi
        $APP -rect $X,$Y,300,200 -timeout $(($RANDOM%10 + 1)) -move &
        NX_PID[$INDEX]=$!
        echo ${NX_PID[$INDEX]} $INDEX started
    else
        nx_wait ${NX_PID[$INDEX]}
        unset NX_PID[$INDEX]
    fi
done

nx_wait_for_clients
nx_stop_server
