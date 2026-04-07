#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

CALIBRATION=""

$DAQLITE_HOME/bin/fylgje $BROKER -f $DAQLITE_CONFIG/bifrost/fylgje.json -c $DAQLITE_CONFIG/bifrost/calibration.json $KAFKA_CONFIG