#!/bin/bash

nxclient_graphics.sh || exit 1
nxclient_decode.sh || exit 1
nxclient_record.sh || exit 1
echo PASS
