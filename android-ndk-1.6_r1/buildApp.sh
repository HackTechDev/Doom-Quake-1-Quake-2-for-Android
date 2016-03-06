#!/bin/sh

rm out/apps/ -rf
sh build/host-setup.sh --no-awk-check
make APP=glesquakejni
make APP=Doom
make APP=quake2
