#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/odin/odin.json &
../build/bin/daqlite $BROKER -f ../configs/odin/odintof.json
