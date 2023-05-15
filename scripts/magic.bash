#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/magic/magic.json &
../build/bin/daqlite $BROKER -f ../configs/magic/magic_frdet.json &
../build/bin/daqlite $BROKER -f ../configs/magic/magic_padet.json &
