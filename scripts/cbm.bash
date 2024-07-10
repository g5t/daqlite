#!/bin/bash

BROKER=${DAQLITE_BROKER:-}

# Use this if script started from dev environment
# Add the build/bin directory to the PATH if it exists
if [ -d "../build/bin" ]; then
    export DAQLITE_HOME="../build"
    export DAQLITE_CONFIG="../configs"
fi

$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/cbm/ibm_tof.json