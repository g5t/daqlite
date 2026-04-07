#!/bin/bash

# Setup Kafka environment
source "$(dirname "$0")/common.sh"

$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/tbl/tbl_tpx3.json $KAFKA_CONFIG &
$DAQLITE_HOME/bin/daqlite $BROKER -f $DAQLITE_CONFIG/tbl/tbl_cbm.json $KAFKA_CONFIG &
