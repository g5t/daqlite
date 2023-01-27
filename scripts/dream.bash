#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/dream/dream.json &
../build/bin/daqlite $BROKER -f ../configs/dream/dream_bwe.json &
../build/bin/daqlite $BROKER -f ../configs/dream/dream_fwe.json
