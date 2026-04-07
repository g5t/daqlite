#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

# Not used with daqlite deployment - uncomment if needed
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_bwe.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_fwe.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_mantle.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_hr.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_sans.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/dream/dream_cbm.json $KAFKA_CONFIG &
