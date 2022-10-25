#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/miracles/miracles.json &
../build/bin/daqlite $BROKER -f ../configs/miracles/miraclestof.json
