#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/dream/dream.json &
../build/bin/daqlite $BROKER -f ../configs/dream/dream_bwe.json &
../build/bin/daqlite $BROKER -f ../configs/dream/dream_fwe.json &
../build/bin/daqlite $BROKER -f ../configs/dream/dream_mantle.json &
../build/bin/daqlite $BROKER -f ../configs/dream/dream_hr.json &
../build/bin/daqlite $BROKER -f ../configs/dream/dream_sans.json
