#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/freia/freia.json $KAFKA_CONFIG
# Beam monitor setup not used in daqlite deployment
# $DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/freia/freiamon_ch0.json $KAFKA_CONFIG &
# $DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/freia/freiamon_ch1.json $KAFKA_CONFIG &