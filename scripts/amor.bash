#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

TOPIC="-t amor_detector"

../build/bin/daqlite $BROKER $TOPIC -f ../configs/amor/amor.json &
../build/bin/daqlite $BROKER $TOPIC -f ../configs/amor/amortof.json &
../build/bin/daqlite $BROKER $TOPIC -f ../configs/amor/amormon_ch0.json &
../build/bin/daqlite $BROKER $TOPIC -f ../configs/amor/amortof2d.json
