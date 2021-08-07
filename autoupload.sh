#!/bin/bash

IP=$1

[ "$IP" ] || exit 1

STUFF="*.js *.cpp smi *.json CMakeLists.txt *.c *.h"

find $STUFF|entr -r -s "rsync -avx $STUFF root@$IP:rpi-ws281x-smi && ssh root@$IP 'killall node; cd rpi-ws281x-smi; cmake-js compile && node test.js'"

