#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/amorb/amor.json &
../build/bin/daqlite $BROKER -f ../configs/amorb/amormon.json &
../build/bin/daqlite $BROKER -f ../configs/amorb/amortof.json &
../build/bin/daqlite $BROKER -f ../configs/amorb/amortof2d.json
