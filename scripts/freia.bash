#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/freia/freia.json &
../build/bin/daqlite $BROKER -f ../configs/freia/freiamon_ch0.json &
#../build/bin/daqlite $BROKER -f ../configs/freia/freiamon_ch1.json &
../build/bin/daqlite $BROKER -f ../configs/freia/freiatof.json
