#!/bin/bash

source nxclient_common.sh

nx_start_server

i=0
while [ $i -lt 8 ]; do
    record -timeout 30 &
    NX_PID[$i]=$!
    i=$(($i+1))
done

nx_wait_for_clients
nx_stop_server
