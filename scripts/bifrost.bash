#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/bifrost/bifrost.json &
../build/bin/daqlite $BROKER -f ../configs/bifrost/bifrosttof.json
