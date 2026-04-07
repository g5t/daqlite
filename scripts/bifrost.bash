#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/bifrost/bifrost.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/bifrost/bifrost_cbm.json $KAFKA_CONFIG &
