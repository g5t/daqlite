#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/cbm/cbm_2d.json $KAFKA_CONFIG &
