#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

# Not used with daqlite deployment - uncomment if needed
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/amor/amor_ibm.json $KAFKA_CONFIG
