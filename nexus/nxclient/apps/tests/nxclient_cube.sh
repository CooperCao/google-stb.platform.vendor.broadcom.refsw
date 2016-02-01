#!/bin/bash

source nxclient_common.sh

nx_start_server

cube f=2000 &
NX_PID[$i]=$!

nx_wait_for_clients
nx_stop_server
