#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

../build/bin/daqlite $BROKER -f ../configs/loki/stfc/loki.json &
../build/bin/daqlite $BROKER -f ../configs/loki/stfc/lokimon_ch0.json &
../build/bin/daqlite $BROKER -f ../configs/loki/stfc/lokimon_ch1.json &
../build/bin/daqlite $BROKER -f ../configs/loki/stfc/lokitof.json
