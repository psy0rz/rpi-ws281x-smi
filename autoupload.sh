#!/bin/bash

IP=$1

[ "$IP" ] || exit 1

STUFF="*.js *.cpp smi *.json CMakeLists.txt *.c *.h"
TEST="node test.js"

#find $STUFF|entr -r -s "rsync -avx $STUFF root@$IP:rpi-ws281x-smi && ssh root@$IP 'killall node; cd rpi-ws281x-smi; cmake-js compile; $TEST '"
find $STUFF | entr -r -s "rsync -avx $STUFF root@$IP:rpi-ws281x-smi && ssh root@$IP 'cd ledder && rm -rf node_modules/rpi-ws281x-smi && npm install ../rpi-ws281x-smi && node ledder/server/server.js'"
