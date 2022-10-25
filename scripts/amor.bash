#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/amor/amor.json &
../build/bin/daqlite $BROKER -f ../configs/amor/amortof.json &
../build/bin/daqlite $BROKER -f ../configs/amor/amormon_ch0.json
