#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/loki/loki.json &
../build/bin/daqlite $BROKER -f ../configs/loki/lokitof.json &
#../build/bin/daqlite $BROKER -f ../configs/loki/lokimon_ch0.json &
#../build/bin/daqlite $BROKER -f ../configs/loki/lokimon_ch1.json
