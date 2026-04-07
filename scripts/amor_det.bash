#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

# Amor instrument
$DAQLITE_HOME/bin/daqlite $BROKER $KAFKA_CONFIG -f $DAQLITE_CONFIG/amor/amor.json &
$DAQLITE_HOME/bin/daqlite $BROKER $KAFKA_CONFIG -f $DAQLITE_CONFIG/amor/amortof.json &
$DAQLITE_HOME/bin/daqlite $BROKER $KAFKA_CONFIG -f $DAQLITE_CONFIG/amor/amortof2d.json
