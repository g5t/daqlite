#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/vipdemo/freia.json &
../build/bin/daqlite $BROKER -f ../configs/vipdemo/nmx.json &
../build/bin/daqlite $BROKER -f ../configs/vipdemo/loki.json &
../build/bin/daqlite $BROKER -f ../configs/vipdemo/cspec3d.json &
../build/bin/daqlite $BROKER -f ../configs/vipdemo/dream.json
